/*
 * $Id: fight.c,v 1.48 1998-07-11 20:55:10 fjoe Exp $
 */

/***************************************************************************
 *     ANATOLIA 2.1 is copyright 1996-1997 Serdar BULUT, Ibrahim CANPUNAR  *
 *     ANATOLIA has been brought to you by ANATOLIA consortium		   *
 *	 Serdar BULUT {Chronos} 	bulut@rorqual.cc.metu.edu.tr	   *
 *	 Ibrahim Canpunar  {Asena}	canpunar@rorqual.cc.metu.edu.tr    *
 *	 Murat BICER  {KIO}		mbicer@rorqual.cc.metu.edu.tr	   *
 *	 D.Baris ACAR {Powerman}	dbacar@rorqual.cc.metu.edu.tr	   *
 *     By using this code, you have agreed to follow the terms of the	   *
 *     ANATOLIA license, in the file Anatolia/anatolia.licence		   *
 ***************************************************************************/

/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,	   *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *									   *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael	   *
 *  Chastain, Michael Quan, and Mitchell Tse.				   *
 *									   *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc	   *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.						   *
 *									   *
 *  Much time and thought has gone into this software and you are	   *
 *  benefitting.  We hope that you share your changes too.  What goes	   *
 *  around, comes around.						   *
 ***************************************************************************/

/***************************************************************************
*	ROM 2.4 is copyright 1993-1995 Russ Taylor			   *
*	ROM has been brought to you by the ROM consortium		   *
*	    Russ Taylor (rtaylor@pacinfo.com)				   *
*	    Gabrielle Taylor (gtaylor@pacinfo.com)			   *
*	    Brian Moore (rom@rom.efn.org)				   *
*	By using this code, you have agreed to follow the terms of the	   *
*	ROM license, in the file Rom24/doc/rom.license			   *
***************************************************************************/

#include <sys/types.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include "merc.h"
#include "act_obj.h"
#include "act_wiz.h"
#include "db.h"
#include "comm.h"
#include "hometown.h"
#include "act_comm.h"
#include "magic.h"
#include "resource.h"
#include "quest.h"
#include "fight.h"
#include "rating.h"
#include "update.h"
#include "util.h"
#include "log.h"
#include "act_move.h"
#include "mob_prog.h"
#include "obj_prog.h"

#define MAX_DAMAGE_MESSAGE 34

/* command procedures needed */
DECLARE_DO_FUN(do_emote 	);
DECLARE_DO_FUN(do_berserk	);
DECLARE_DO_FUN(do_bash		);
DECLARE_DO_FUN(do_trip		);
DECLARE_DO_FUN(do_dirt		);
DECLARE_DO_FUN(do_flee		);
DECLARE_DO_FUN(do_kick		);
DECLARE_DO_FUN(do_disarm	);
DECLARE_DO_FUN(do_get		);
DECLARE_DO_FUN(do_recall	);
DECLARE_DO_FUN(do_yell		);
DECLARE_DO_FUN(do_sacrifice	);
DECLARE_DO_FUN(do_quit_count	);
DECLARE_DO_FUN(do_sleep 	);
DECLARE_DO_FUN(do_tail		);
DECLARE_DO_FUN(do_crush 	);
DECLARE_DO_FUN(do_dismount	);

extern void do_visible(CHAR_DATA *ch, const char *argument);

/*
 * Local functions.
 */
void	check_assist	args((CHAR_DATA *ch, CHAR_DATA *victim));
bool	check_dodge	args((CHAR_DATA *ch, CHAR_DATA *victim));
bool	check_parry	args((CHAR_DATA *ch, CHAR_DATA *victim));
bool	check_block	args((CHAR_DATA *ch, CHAR_DATA *victim));
bool	check_blink	args((CHAR_DATA *ch, CHAR_DATA *victim));
void	dam_message	args((CHAR_DATA *ch, CHAR_DATA *victim, int dam,
				    int dt, bool immune ,int dam_type));
void	death_cry	args((CHAR_DATA *ch));
void	death_cry_org	args((CHAR_DATA *ch, int part));
void	group_gain	args((CHAR_DATA *ch, CHAR_DATA *victim));
int	xp_compute	args((CHAR_DATA *gch, CHAR_DATA *victim,
				    int total_levels,int members));
bool	is_safe 	args((CHAR_DATA *ch, CHAR_DATA *victim));
void	make_corpse	args((CHAR_DATA *ch));
void	one_hit 	args((CHAR_DATA *ch, CHAR_DATA *victim, int dt ,bool secondary));
void	mob_hit 	args((CHAR_DATA *ch, CHAR_DATA *victim, int dt));
void	set_fighting	args((CHAR_DATA *ch, CHAR_DATA *victim));
void	disarm		args((CHAR_DATA *ch, CHAR_DATA *victim ,int disarm_second));
void	check_weapon_destroy	args((CHAR_DATA *ch, CHAR_DATA *victim, bool second));
void	damage_to_object	args((CHAR_DATA *ch, OBJ_DATA *wield, OBJ_DATA *worn, int damage));
int	critical_strike args((CHAR_DATA *ch, CHAR_DATA *victim, int dam));
void	check_shield_destroyed	args((CHAR_DATA *ch, CHAR_DATA *victim, bool second));
void	check_weapon_destroyed	args((CHAR_DATA *ch, CHAR_DATA *victim, bool second));

/*
 * Gets all money from the corpse.
 */
void get_gold_corpse(CHAR_DATA *ch, OBJ_DATA *corpse)
{
	OBJ_DATA *tmp, *tmp_next;
	for (tmp = corpse->contains; tmp; tmp = tmp_next) {
		tmp_next = tmp->next_content;
		if (tmp->item_type == ITEM_MONEY)
			get_obj(ch, tmp, corpse);
	}
}

/*
 * Control the fights going on.
 * Called periodically by update_handler.
 */
void violence_update(void)
{
	CHAR_DATA *ch;
	CHAR_DATA *ch_next;
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;

	for (ch = char_list; ch != NULL; ch = ch_next)
	{
		ch_next = ch->next;

		if ((victim = ch->fighting) == NULL || ch->in_room == NULL)
		    continue;


		if (IS_AWAKE(ch) && ch->in_room == victim->in_room)
		    multi_hit(ch, victim, TYPE_UNDEFINED);
		else
		    stop_fighting(ch, FALSE);

		if ((victim = ch->fighting) == NULL)
		    continue;

		if (!IS_NPC(victim))
		  ch->last_fought = victim;

		SET_FIGHT_TIME(ch);

		for (obj = ch->carrying;obj != NULL; obj = obj_next) {
			obj_next = obj->next_content;
			oprog_call(OPROG_FIGHT, obj, ch, NULL);
		}

		/*
		 * Fun for the whole family!
		 */
		check_assist(ch,victim);
		if (IS_NPC(ch)) {
			if (HAS_TRIGGER(ch, TRIG_FIGHT))
				mp_percent_trigger(ch, victim, NULL, NULL,
						   TRIG_FIGHT);
			if (HAS_TRIGGER(ch, TRIG_HPCNT))
				mp_hprct_trigger(ch, victim);
		}
	}

	return;
}

/* for auto assisting */
void check_assist(CHAR_DATA *ch,CHAR_DATA *victim)
{
	CHAR_DATA *rch, *rch_next;

	for (rch = ch->in_room->people; rch != NULL; rch = rch_next)
	{
		rch_next = rch->next_in_room;

		if (IS_AWAKE(rch) && rch->fighting == NULL)
		{

		    /* quick check for ASSIST_PLAYER */
		    if (!IS_NPC(ch) && IS_NPC(rch)
		    && IS_SET(rch->off_flags,ASSIST_PLAYERS)
		    &&	rch->level + 6 > victim->level)
		    {
			do_emote(rch,"screams and attacks!");
			multi_hit(rch,victim,TYPE_UNDEFINED);
			continue;
		    }

		    /* PCs next */
		    if (!IS_NPC(rch) || IS_AFFECTED(rch,AFF_CHARM))
		    {
			if (((!IS_NPC(rch) && IS_SET(rch->act,PLR_AUTOASSIST))
			||     IS_AFFECTED(rch,AFF_CHARM))
			&&   is_same_group(ch,rch))
			    multi_hit (rch,victim,TYPE_UNDEFINED);

			continue;
		    }

		    if (!IS_NPC(ch) && RIDDEN(rch) == ch)
		    {
			multi_hit(rch,victim,TYPE_UNDEFINED);
			continue;
		    }

		    /* now check the NPC cases */

		    if (IS_NPC(ch))

		    {
			if ((IS_NPC(rch) && IS_SET(rch->off_flags,ASSIST_ALL))

			||   (IS_NPC(rch) && RACE(rch) == RACE(ch)
			   && IS_SET(rch->off_flags,ASSIST_RACE))

			||   (IS_NPC(rch) && IS_SET(rch->off_flags,ASSIST_ALIGN)
			   &&	((IS_GOOD(rch)	  && IS_GOOD(ch))
			     ||  (IS_EVIL(rch)	  && IS_EVIL(ch))
			     ||  (IS_NEUTRAL(rch) && IS_NEUTRAL(ch))))

			||   (rch->pIndexData == ch->pIndexData
			   && IS_SET(rch->off_flags,ASSIST_VNUM)))

			{
			    CHAR_DATA *vch;
			    CHAR_DATA *target;
			    int number;

			    if (number_bits(1) == 0)
				continue;

			    target = NULL;
			    number = 0;

			    for (vch = ch->in_room->people; vch; vch = vch->next_in_room)
			    {
				if (can_see(rch,vch)
				&&  is_same_group(vch,victim)
				&&  number_range(0,number) == 0)
				{
				    target = vch;
				    number++;
				}
			    }

			    if (target != NULL)
			    {
				do_emote(rch,"screams and attacks!");
				multi_hit(rch,target,TYPE_UNDEFINED);
			    }
			}
		    }
		}
	}
}


/*
 * Do one group of attacks.
 */
void multi_hit(CHAR_DATA *ch, CHAR_DATA *victim, int dt)
{
	int     chance;

	/* decrement the wait */
	if (ch->desc == NULL)
		ch->wait = UMAX(0,ch->wait - PULSE_VIOLENCE);

	/* no attacks for stunnies -- just a check */
	if (ch->position < POS_RESTING)
		return;

#ifdef 0
	/* become CRIMINAL in Law rooms */
	if (!IS_NPC(ch) && !IS_NPC(victim)
	    && IS_SET(ch->in_room->room_flags, ROOM_LAW)
	    && !IS_SET(victim->act, PLR_WANTED)
	    && !IS_SET(ch->act, PLR_WANTED)) {
		send_to_char("This room is under supervision of the law! Now you're {RCRIMINAL{x!\n\r", ch);
		SET_BIT(ch->act, PLR_WANTED);		
	}
#endif

	/* ridden's adjustment */
	if (RIDDEN(victim) && !IS_NPC(victim->mount))
		{
		 if (victim->mount->fighting == NULL
		     || victim->mount->fighting == ch)
		 victim = victim->mount;
		 else do_dismount(victim->mount,"");
		}

	/* no attacks on ghosts or attacks by ghosts */
	if ((!IS_NPC(victim) && IS_SET(victim->act, PLR_GHOST)) ||
		(!IS_NPC(ch) && IS_SET(ch->act, PLR_GHOST)))
		return;

	if (IS_AFFECTED(ch,AFF_WEAK_STUN))
		{
 act_puts("You are too stunned to respond $N's attack.",
			ch,NULL,victim,TO_CHAR,POS_FIGHTING);
 act_puts("$n is too stunned to respond your attack.",
			ch,NULL,victim,TO_VICT,POS_FIGHTING);
 act_puts("$n seems to be stunned.",
			ch,NULL,victim,TO_NOTVICT,POS_FIGHTING);
		 REMOVE_BIT(ch->affected_by,AFF_WEAK_STUN);
		 return;
		}

	if (IS_AFFECTED(ch,AFF_STUN))
		{
 act_puts("You are too stunned to respond $N's attack.",
			ch,NULL,victim,TO_CHAR,POS_FIGHTING);
 act_puts("$n is too stunned to respond your attack.",
			ch,NULL,victim,TO_VICT,POS_FIGHTING);
 act_puts("$n seems to be stunned.",
			ch,NULL,victim,TO_NOTVICT,POS_FIGHTING);
		 affect_strip(ch,gsn_power_stun);
		 SET_BIT(ch->affected_by,AFF_WEAK_STUN);
		 return;
		}

	if (IS_NPC(ch))
	{
		mob_hit(ch,victim,dt);
		return;
	}

	one_hit(ch, victim, dt ,FALSE);

	if (ch->fighting != victim)
		return;

   if (ch->level > skill_table[gsn_area_attack].skill_level[ch->class]
			&& number_percent() < get_skill(ch,gsn_area_attack))
	{
		int count=0,max_count;
		CHAR_DATA *vch,*vch_next;

		check_improve(ch,gsn_area_attack, TRUE, 6);

		if (ch->level < 70)	max_count = 1;
		else if (ch->level < 80)	max_count = 2;
		else if (ch->level < 90)	max_count = 3;
		else max_count = 4;

		for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
		{
		    vch_next = vch->next_in_room;
		    if ((vch != victim && vch->fighting == ch))
			{
			 one_hit(ch,vch,dt,FALSE);
			 count++;
			}
		    if (count == max_count) break;
		}
	}

	if (IS_AFFECTED(ch,AFF_HASTE))
		one_hit(ch,victim,dt, FALSE);

	if (ch->fighting != victim || dt == gsn_backstab || dt == gsn_cleave
		|| dt == gsn_ambush || dt == gsn_dual_backstab || dt == gsn_circle
		|| dt == gsn_assassinate || dt == gsn_vampiric_bite)
		return;

	chance = get_skill(ch,gsn_second_attack)/2;
	if (number_percent() < chance)
	{
		one_hit(ch, victim, dt,FALSE);
		check_improve(ch,gsn_second_attack,TRUE,5);
		if (ch->fighting != victim)
		    return;
	}

	chance = get_skill(ch,gsn_third_attack)/3;
	if (number_percent() < chance)
	{
		one_hit(ch, victim, dt ,FALSE);
		check_improve(ch,gsn_third_attack,TRUE,6);
		if (ch->fighting != victim)
		    return;
	}


	chance = get_skill(ch,gsn_fourth_attack)/6;
	if (number_percent() < chance)
	{
		one_hit(ch, victim, dt ,FALSE);
		check_improve(ch,gsn_fourth_attack,TRUE,7);
		if (ch->fighting != victim)
		    return;
	}

	chance = get_skill(ch,gsn_fifth_attack)/8;
	if (number_percent() < chance)
	{
		one_hit(ch, victim, dt ,FALSE);
		check_improve(ch,gsn_fifth_attack,TRUE,8);
		if (ch->fighting != victim)
		    return;
	}

	chance = get_skill(ch,gsn_second_weapon) / 2;
	if (number_percent() < chance)
	{
	 if (get_eq_char(ch , WEAR_SECOND_WIELD))
		{
		 one_hit(ch , victim , dt, TRUE);
		 check_improve(ch, gsn_second_weapon,TRUE,2);
		 if (ch->fighting != victim)
			return;
		}
	}

	chance = get_skill(ch,gsn_secondary_attack) / 8;
	if (number_percent() < chance)
	{
	 if (get_eq_char(ch , WEAR_SECOND_WIELD))
		{
		 one_hit(ch , victim , dt, TRUE);
		 check_improve(ch, gsn_secondary_attack,TRUE,2);
		 if (ch->fighting != victim)
			return;
		}
	}

	return;
}

/* procedure for all mobile attacks */
void mob_hit (CHAR_DATA *ch, CHAR_DATA *victim, int dt)
{
	int chance,number;
	CHAR_DATA *vch, *vch_next;

	/* no attacks on ghosts */
	if (!IS_NPC(victim) && IS_SET(victim->act, PLR_GHOST))
		return;

	/* no attack by ridden mobiles except spec_casts */
	if (RIDDEN(ch))
		{
		 if (ch->fighting != victim) set_fighting(ch,victim);
		 return;
		}

	one_hit(ch,victim,dt,FALSE);

	if (ch->fighting != victim)
		return;


	/* Area attack -- BALLS nasty! */

	if (IS_SET(ch->off_flags,OFF_AREA_ATTACK))
	{
		for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
		{
		    vch_next = vch->next_in_room;
		    if ((vch != victim && vch->fighting == ch))
			one_hit(ch,vch,dt,FALSE);
		}
	}

	if (IS_AFFECTED(ch,AFF_HASTE) || IS_SET(ch->off_flags,OFF_FAST))
		one_hit(ch,victim,dt,FALSE);

	if (ch->fighting != victim || dt == gsn_backstab || dt == gsn_circle ||
		dt == gsn_dual_backstab || dt == gsn_cleave || dt == gsn_ambush
			|| dt == gsn_vampiric_bite)
		return;

	chance = get_skill(ch,gsn_second_attack)/2;
	if (number_percent() < chance)
	{
		one_hit(ch,victim,dt,FALSE);
		if (ch->fighting != victim)
		    return;
	}

	chance = get_skill(ch,gsn_third_attack)/4;
	if (number_percent() < chance)
	{
		one_hit(ch,victim,dt,FALSE);
		if (ch->fighting != victim)
		    return;
	}

	chance = get_skill(ch,gsn_fourth_attack)/6;
	if (number_percent() < chance)
	{
		one_hit(ch, victim, dt ,FALSE);
		if (ch->fighting != victim)
		    return;
	}

	/* PC waits */

	if (ch->wait > 0)
		return;

	number = number_range(0,2);

	if (number == 1 && IS_SET(ch->act,ACT_MAGE))
	{
		/*  { mob_cast_mage(ch,victim); return; } */;
	}


	if (number == 2 && IS_SET(ch->act,ACT_CLERIC))
	{
		/*  { mob_cast_cleric(ch,victim); return; } */;
	}

	/* now for the skills */

	number = number_range(0,7);

	switch(number)
	{
	case (0) :
		if (IS_SET(ch->off_flags,OFF_BASH))
		    do_bash(ch,"");
		break;

	case (1) :
		if (IS_SET(ch->off_flags,OFF_BERSERK) && !IS_AFFECTED(ch,AFF_BERSERK))
		    do_berserk(ch,"");
		break;


	case (2) :
		if (IS_SET(ch->off_flags,OFF_DISARM)
		|| (get_weapon_sn(ch, WEAR_WIELD) != gsn_hand_to_hand
		&& (IS_SET(ch->act,ACT_WARRIOR)
		||  IS_SET(ch->act,ACT_THIEF))))
		    do_disarm(ch,"");
		break;

	case (3) :
		if (IS_SET(ch->off_flags,OFF_KICK))
		    do_kick(ch,"");
		break;

	case (4) :
		if (IS_SET(ch->off_flags,OFF_DIRT_KICK))
		    do_dirt(ch,"");
		break;

	case (5) :
		if (IS_SET(ch->off_flags,OFF_TAIL))
		  do_tail(ch,"");
		break;

	case (6) :
		if (IS_SET(ch->off_flags,OFF_TRIP))
		    do_trip(ch,"");
		break;
	case (7) :
		if (IS_SET(ch->off_flags,OFF_CRUSH))
		    do_crush(ch,"");
		break;
	}
}

/*
 * Hit one guy once.
 */
void one_hit(CHAR_DATA *ch, CHAR_DATA *victim, int dt ,bool secondary)
{
	OBJ_DATA *wield;
	int victim_ac;
	int thac0;
	int thac0_00;
	int thac0_32;
	int dam;
	int diceroll;
	int sn,skill;
	int dam_type;
	bool counter;
	bool yell;
	bool result;
	OBJ_DATA *corpse;
	int sercount;

	sn = -1;
	counter = FALSE;

	if (victim->fighting == ch)
	  yell = FALSE;
	else yell = TRUE;

	/* just in case */
	if (victim == ch || ch == NULL || victim == NULL)
		return;

	/* ghosts can't fight */
	if ((!IS_NPC(victim) && IS_SET(victim->act, PLR_GHOST)) ||
		(!IS_NPC(ch) && IS_SET(ch->act, PLR_GHOST)))
		return;

	/*
	 * Can't beat a dead char!
	 * Guard against weird room-leavings.
	 */
	if (victim->position == POS_DEAD || ch->in_room != victim->in_room)
		return;

	/*
	 * Figure out the type of damage message.
	 */

	if (secondary)
	wield = get_eq_char(ch, WEAR_SECOND_WIELD);
		else
	wield = get_eq_char(ch, WEAR_WIELD);

	if (dt == TYPE_UNDEFINED)
	{
		dt = TYPE_HIT;
		if (wield != NULL && wield->item_type == ITEM_WEAPON)
		    dt += wield->value[3];
		else
		    dt += ch->dam_type;
	}

	if (dt < TYPE_HIT)
		if (wield != NULL)
		    dam_type = attack_table[wield->value[3]].damage;
		else
		    dam_type = attack_table[ch->dam_type].damage;
	else
		dam_type = attack_table[dt - TYPE_HIT].damage;

	if (dam_type == -1)
		dam_type = DAM_BASH;

	/* get the weapon skill */
	sn = get_weapon_sn(ch, WEAR_WIELD);
	skill = 20 + get_weapon_skill(ch,sn);

	/*
	 * Calculate to-hit-armor-class-0 versus armor.
	 */
	if (IS_NPC(ch))
	{
		thac0_00 = 20;
		thac0_32 = -4;	 /* as good as a thief */
		if (IS_SET(ch->act,ACT_WARRIOR))
		    thac0_32 = -10;
		else if (IS_SET(ch->act,ACT_THIEF))
		    thac0_32 = -4;
		else if (IS_SET(ch->act,ACT_CLERIC))
		    thac0_32 = 2;
		else if (IS_SET(ch->act,ACT_MAGE))
		    thac0_32 = 6;
	}
	else
	{
		thac0_00 = class_table[ch->class].thac0_00;
		thac0_32 = class_table[ch->class].thac0_32;
	}

	thac0  = interpolate(ch->level, thac0_00, thac0_32);

	if (thac0 < 0)
		thac0 = thac0/2;

	if (thac0 < -5)
		thac0 = -5 + (thac0 + 5) / 2;

	thac0 -= GET_HITROLL(ch) * skill/100;
	thac0 += 5 * (100 - skill) / 100;

	if (dt == gsn_backstab)
		thac0 -= 10 * (100 - get_skill(ch,gsn_backstab));

	if (dt == gsn_dual_backstab)
		thac0 -= 10 * (100 - get_skill(ch,gsn_dual_backstab));

	if (dt == gsn_cleave)
		thac0 -= 10 * (100 - get_skill(ch,gsn_cleave));

	if (dt == gsn_ambush)
		thac0 -= 10 * (100 - get_skill(ch,gsn_ambush));

	if (dt == gsn_vampiric_bite)
		thac0 -= 10 * (100 - get_skill(ch,gsn_vampiric_bite));

	switch(dam_type)
	{
		case(DAM_PIERCE):victim_ac = GET_AC(victim,AC_PIERCE)/10;	break;
		case(DAM_BASH):  victim_ac = GET_AC(victim,AC_BASH)/10; 	break;
		case(DAM_SLASH): victim_ac = GET_AC(victim,AC_SLASH)/10;	break;
		default:	 victim_ac = GET_AC(victim,AC_EXOTIC)/10;	break;
	};

	if (victim_ac < -15)
		victim_ac = (victim_ac + 15) / 5 - 15;

	if (get_skill(victim,gsn_armor_use) > 70)
		{
		 check_improve(victim,gsn_armor_use,TRUE,8);
		 victim_ac -= (victim->level) / 2;
		}

	if (!can_see(ch, victim))
		{
		 if (ch->level > skill_table[gsn_blind_fighting].skill_level[ch->class]
			&& number_percent() < get_skill(ch,gsn_blind_fighting))
			{
			 check_improve(ch,gsn_blind_fighting,TRUE,16);
			}
		 else victim_ac -= 4;
		}

	if (victim->position < POS_FIGHTING)
		victim_ac += 4;

	if (victim->position < POS_RESTING)
		victim_ac += 6;

	/*
	 * The moment of excitement!
	 */
	while ((diceroll = number_bits(5)) >= 20)
		;

	if (diceroll == 0
	|| (diceroll != 19 && diceroll < thac0 - victim_ac))
	{
		/* Miss. */
		damage(ch, victim, 0, dt, dam_type, TRUE);
		tail_chain();
		return;
	}

	/*
	 * Hit.
	 * Calc damage.
	 */


	if (IS_NPC(ch) && (!ch->pIndexData->new_format || wield == NULL))
		if (!ch->pIndexData->new_format)
		{
		    dam = number_range(ch->level / 2, ch->level * 3 / 2);
		    if (wield != NULL)
			dam += dam / 2;
		}
		else
		    dam = dice(ch->damage[DICE_NUMBER],ch->damage[DICE_TYPE]);

	else
	{
		if (sn != -1)
		    check_improve(ch,sn,TRUE,5);
		if (wield != NULL)
		{
		    if (wield->pIndexData->new_format)
			dam = dice(wield->value[1],wield->value[2]) * skill/100;
		    else
			dam = number_range(wield->value[1] * skill/100,
					wield->value[2] * skill/100);

		    if (get_eq_char(ch,WEAR_SHIELD) == NULL)  /* no shield = more */
			dam = dam * 21/20;

		    /* sharpness! */
		    if (IS_WEAPON_STAT(wield,WEAPON_SHARP))
		    {
			int percent;

			if ((percent = number_percent()) <= (skill / 8))
			    dam = 2 * dam + (dam * 2 * percent / 100);
		    }
		    /* holy weapon */
		    if (IS_WEAPON_STAT(wield,WEAPON_HOLY) &&
			  IS_GOOD(ch) && IS_EVIL(victim) && number_percent() < 30)
		    {
			act_puts("$p shines with a holy area.",
				ch,wield,NULL,TO_CHAR,POS_DEAD);
			act_puts("$p shines with a holy area.",
				ch,wield,NULL,TO_ROOM,POS_DEAD);
			dam += dam * 120 / 100;
		    }
		}
		else {
		      dam =number_range(1 + 4 * skill/100, 2 * ch->level/3 * skill/100);
		      if (get_skill(ch,gsn_master_hand) > 0)
			  {
			   int d;
			   if ((d=number_percent()) <= get_skill(ch,gsn_master_hand))
				{
				 check_improve(ch,gsn_master_hand,TRUE,6);
				 dam += dam * 110 /100;
				 if (d < 20)
				  {
				   SET_BIT(victim->affected_by,AFF_WEAK_STUN);
		act_puts("You hit $N with a stunning force!",
			ch,NULL,victim,TO_CHAR,POS_DEAD);
		act_puts("$n hit you with a stunning force!",
			ch,NULL,victim,TO_VICT,POS_DEAD);
		act_puts("$n hits $N with a stunning force!",
			ch,NULL,victim,TO_NOTVICT,POS_DEAD);
				   check_improve(ch,gsn_master_hand,TRUE,6);
				  }
				}

			  }

		     }
	}

	/*
	 * Bonuses.
	 */
	if (get_skill(ch,gsn_enhanced_damage) > 0)
	{
		diceroll = number_percent();
		if (diceroll <= get_skill(ch,gsn_enhanced_damage))
		{
		    int div;
		    check_improve(ch,gsn_enhanced_damage,TRUE,6);
		    div = (ch->class == CLASS_WARRIOR) ? 100 :
				(ch->class == CLASS_CLERIC) ? 130 : 114;
		    dam += dam * diceroll/div;
		}
	}

	if (get_skill(ch,gsn_master_sword) > 0 && sn== gsn_sword)
	{
		if (number_percent() <= get_skill(ch,gsn_master_sword))
		{
		    OBJ_DATA *katana;

		    check_improve(ch,gsn_master_sword,TRUE,6);
		    dam += dam * 110 /100;

		    if ((katana = get_eq_char(ch,WEAR_WIELD)) != NULL)
			{
			 AFFECT_DATA *paf;

			 if (IS_WEAPON_STAT(katana,WEAPON_KATANA)
			    && strstr(katana->extra_descr->description,ch->name) != NULL)
			 {
			  katana->cost++;
			  if (katana->cost > 249)
			  {
			   paf =  affect_find(katana->affected,gsn_katana);
			   if (paf != NULL)
				{
				 int old_mod=paf->modifier;
				 paf->modifier = UMIN(paf->modifier+1,ch->level / 3);
				 if (paf->next != NULL) {
				     paf->next->modifier = paf->modifier;
				     ch->hitroll += paf->modifier - old_mod;
				     if (paf->next->next) {
					 paf->next->next->modifier 
						= paf->modifier;
					 ch->damroll += paf->modifier - old_mod;
				     }
				 }
				 act("$n's katana glows blue.\n\r",ch,NULL,NULL,TO_ROOM);
				 send_to_char("Your katana glows blue.\n\r",ch);
				}
			   katana->cost = 0;
			  }
			}
		       }
		    else if ((katana=get_eq_char(ch,WEAR_SECOND_WIELD)) != NULL)
			{
			 AFFECT_DATA *paf;

			 if (IS_WEAPON_STAT(katana,WEAPON_KATANA)
			    && strstr(katana->extra_descr->description,ch->name)!=NULL)
			 {
			  katana->cost++;
			  if (katana->cost > 249)
			  {
			   paf =  affect_find(katana->affected,gsn_katana);
			   if (paf != NULL)
				{
				 paf->modifier = UMIN(paf->modifier+1,ch->level / 3);
				 if (paf->next != NULL) {
					paf->next->modifier = paf->modifier;
					if (paf->next->next)
						paf->next->next->modifier 
							= paf->modifier;
				 }
				 act("$n's katana glows blue.\n\r",ch,NULL,NULL,TO_ROOM);
				 send_to_char("Your katana glows blue.\n\r",ch);
				}
			   katana->cost = 0;
			  }
			 }
			}
		}
	}

	if (!IS_AWAKE(victim))
		dam *= 2;
	 else if (victim->position < POS_FIGHTING)
		dam = dam * 3 / 2;

	sercount = number_percent();
	if (dt==gsn_backstab || dt==gsn_vampiric_bite)
		sercount += 40;
	if (!IS_IMMORTAL(ch) && IS_PUMPED(ch))
		sercount += 10;
	sercount *= 2;
	if (victim->fighting == NULL && !IS_NPC(victim) &&
		 !is_safe_nomessage(victim, ch) && !is_safe_nomessage(ch,victim) &&
		(victim->position == POS_SITTING || victim->position == POS_STANDING)
		&& dt != gsn_assassinate &&
		(sercount <= get_skill(victim,gsn_counter)))
	  {
		counter = TRUE;
		check_improve(victim,gsn_counter,TRUE,1);
		act("$N turns your attack against you!",ch,NULL,victim,TO_CHAR);
		act("You turn $n's attack against $m!",ch,NULL,victim,TO_VICT);
		act("$N turns $n's attack against $m!",ch,NULL,victim,TO_NOTVICT);
		ch->fighting = victim;
	  }
	else if (!victim->fighting) check_improve(victim,gsn_counter,FALSE,1);

	if (dt == gsn_backstab && wield != NULL)
	  dam = (ch->level < 50) ? (ch->level/10 + 1) * dam + ch->level
				     : (ch->level/10) * dam + ch->level;

	else if (dt == gsn_dual_backstab && wield != NULL)
	  dam = (ch->level < 56) ? (ch->level/14 + 1) * dam + ch->level
				     : (ch->level/14) * dam + ch->level;

	else if (dt == gsn_circle)
	  dam = (ch->level/40 + 1) * dam + ch->level;

	else if (dt == gsn_vampiric_bite && IS_VAMPIRE(ch))
	  dam = (ch->level/20 + 1) * dam + ch->level;

	else if (dt == gsn_cleave && wield != NULL) {
		if (number_percent() < URANGE(4, 5+(ch->level-victim->level),10) && !counter && !IS_IMMORTAL(victim)) {
		    act_puts("Your cleave chops $N IN HALF!",
			      ch,NULL,victim,TO_CHAR,POS_RESTING);
		    act_puts("$n's cleave chops you IN HALF!",
			      ch,NULL,victim,TO_VICT,POS_RESTING);
		    act_puts("$n's cleave chops $N IN HALF!",
			      ch,NULL,victim,TO_NOTVICT,POS_RESTING);
		    send_to_char("You have been KILLED!\n\r",victim);
		    act("$n is DEAD!",victim,NULL,NULL,TO_ROOM);
		    WAIT_STATE(ch, 2);
		    raw_kill(ch, victim);
		    if (!IS_NPC(ch) && IS_NPC(victim))
		      {
			corpse = get_obj_list(ch, "corpse", ch->in_room->contents);

			if (IS_SET(ch->act, PLR_AUTOLOOT) &&
			    corpse && corpse->contains) /* exists and not empty */
			  do_get(ch, "all corpse");

			if (IS_SET(ch->act,PLR_AUTOGOLD) &&
			    corpse && corpse->contains	&& /* exists and not empty */
			    !IS_SET(ch->act,PLR_AUTOLOOT))  {
				get_gold_corpse(ch, corpse);
			}

			if (IS_SET(ch->act, PLR_AUTOSAC))
/*			  if (IS_SET(ch->act,PLR_AUTOLOOT) && corpse
			      && corpse->contains)
			    return;
			  else
*/			    do_sacrifice(ch, "corpse");
		      }
		    return;
		  }
		else dam = (dam * 2 + ch->level);
	  }

	if (dt == gsn_assassinate)
	  {
		if (number_percent() <= URANGE(10, 20+(ch->level-victim->level)*2, 50) && !counter && !IS_IMMORTAL(victim)) {
		    act_puts("You {R+++ASSASSINATE+++{x $N!",ch,NULL,victim,TO_CHAR,
			      POS_RESTING);
		    act("$N is DEAD!",ch,NULL,victim,TO_CHAR);
		    act_puts("$n {R+++ASSASSINATES+++{x $N!",ch,NULL,victim,
			      TO_NOTVICT,POS_RESTING);
		    act("$N is DEAD!",ch,NULL,victim,TO_NOTVICT);
		    act_puts("$n {R+++ASSASSINATES+++{x you!",ch,NULL,victim,
			      TO_VICT,POS_DEAD);
		    send_to_char("You have been KILLED!\n\r",victim);
		    check_improve(ch,gsn_assassinate,TRUE,1);
		    raw_kill(ch, victim);
		    if (!IS_NPC(ch) && IS_NPC(victim))
		      {
			corpse = get_obj_list(ch, "corpse", ch->in_room->contents);

			if (IS_SET(ch->act, PLR_AUTOLOOT) &&
			    corpse && corpse->contains) /* exists and not empty */
			  do_get(ch, "all corpse");

			if (IS_SET(ch->act,PLR_AUTOGOLD) &&
			    corpse && corpse->contains	&& /* exists and not empty */
			    !IS_SET(ch->act,PLR_AUTOLOOT))
				get_gold_corpse(ch, corpse);

			if (IS_SET(ch->act, PLR_AUTOSAC))
/*			  if (IS_SET(ch->act,PLR_AUTOLOOT) && corpse
			      && corpse->contains)
			    return; 
			  else
*/			    do_sacrifice(ch, "corpse");
		      }
		    return;

		  }
		else
		  {
		    check_improve(ch,gsn_assassinate,FALSE,1);
		    dam *= 2;
		  }
	  }


	dam += GET_DAMROLL(ch) * UMIN(100,skill) /100;

	if (dt == gsn_ambush)
	  dam *= 3;

	if (!IS_NPC(ch) && get_skill(ch,gsn_deathblow) > 1 &&
		ch->level >= skill_table[gsn_deathblow].skill_level[ch->class])
	  {
		if (number_percent() < 0.125 * get_skill(ch,gsn_deathblow))
		  {
		    act("You deliver a blow of deadly force!",ch,NULL,NULL,TO_CHAR);
		    act("$n delivers a blow of deadly force!",ch,NULL,NULL,TO_ROOM);
		    if (clan_ok(ch,gsn_deathblow)) {
		      dam *= ((float)ch->level) / 20;
		      check_improve(ch,gsn_deathblow,TRUE,1);
		    }
		  }
		else check_improve(ch,gsn_deathblow,FALSE,3);
	  }

	if (dam <= 0)
		dam = 1;

	if (counter)
	  {
		result = damage(ch,ch,2*dam,dt,dam_type,TRUE);
		multi_hit(victim,ch,TYPE_UNDEFINED);
	  }
	else result = damage(ch, victim, dam, dt, dam_type, TRUE);

	/* vampiric bite gives hp to ch from victim */
		if (dt == gsn_vampiric_bite)
		{
		 int hit_ga = UMIN((dam / 2), victim->max_hit);

		 ch->hit += hit_ga;
		 ch->hit  = UMIN(ch->hit , ch->max_hit);
		 update_pos(ch);
	send_to_char("Your health increases as you suck your victim's blood.\n\r",ch);
		}

	/* but do we have a funky weapon? */
	if (result && wield != NULL)
	{
		int dam;

		if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_POISON))
		{
		    int level;
		    AFFECT_DATA *poison, af;

		    if ((poison = affect_find(wield->affected,gsn_poison)) == NULL)
			level = wield->level;
		    else
			level = poison->level;
		    if (!saves_spell(level / 2,victim,DAM_POISON))
		    {
			send_to_char("You feel poison coursing through your veins.",
			    victim);
			act("$n is poisoned by the venom on $p.",
			    victim,wield,NULL,TO_ROOM);

			af.where     = TO_AFFECTS;
			af.type      = gsn_poison;
			af.level     = level * 3/4;
			af.duration  = level / 2;
			af.location  = APPLY_STR;
			af.modifier  = -1;
			af.bitvector = AFF_POISON;
			affect_join(victim, &af);
		    }

		    /* weaken the poison if it's temporary */
		    if (poison != NULL)
		    {
			poison->level = UMAX(0,poison->level - 2);
			poison->duration = UMAX(0,poison->duration - 1);
			if (poison->level == 0 || poison->duration == 0)
			    act("The poison on $p has worn off.",ch,wield,NULL,TO_CHAR);
		    }
		}
		if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_VAMPIRIC))
		{
		    dam = number_range(1, wield->level / 5 + 1);
		    act("$p draws life from $n.",victim,wield,NULL,TO_ROOM);
		    act("You feel $p drawing your life away.",
			victim,wield,NULL,TO_CHAR);
		    damage(ch,victim,dam,0,DAM_NEGATIVE,FALSE);
		    ch->hit += dam/2;
		}
		if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_FLAMING))
		{
		    dam = number_range(1,wield->level / 4 + 1);
		    act("$n is burned by $p.",victim,wield,NULL,TO_ROOM);
		    act("$p sears your flesh.",victim,wield,NULL,TO_CHAR);
		    fire_effect((void *) victim,wield->level/2,dam,TARGET_CHAR);
		    damage(ch,victim,dam,0,DAM_FIRE,FALSE);
		}
		if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_FROST))
		{
		    dam = number_range(1,wield->level / 6 + 2);
		    act("$p freezes $n.",victim,wield,NULL,TO_ROOM);
		    act("The cold touch of $p surrounds you with ice.",
			victim,wield,NULL,TO_CHAR);
		    cold_effect(victim,wield->level/2,dam,TARGET_CHAR);
		    damage(ch,victim,dam,0,DAM_COLD,FALSE);
		}
		if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_SHOCKING))
		{
		    dam = number_range(1,wield->level/5 + 2);
		    act("$n is struck by lightning from $p.",victim,wield,NULL,TO_ROOM);
		    act("You are shocked by $p.",victim,wield,NULL,TO_CHAR);
		    shock_effect(victim,wield->level/2,dam,TARGET_CHAR);
		    damage(ch,victim,dam,0,DAM_LIGHTNING,FALSE);
		}
	}

	tail_chain();
	return;
}

void delete_player(CHAR_DATA *victim, char* msg)
{
	char strsave[PATH_MAX];

	send_to_char("You became a ghost permanently "
		     "and leave the earth realm.\n\r", victim);
	act("$n is dead, and will not rise again.\n\r",
	    victim, NULL, NULL, TO_ROOM);
	victim->hit = 1;
	victim->position = POS_STANDING;
	snprintf(strsave, sizeof(strsave),
		 "%s%s", PLAYER_DIR, capitalize(victim->name));
	wiznet_printf(victim, NULL, 0, 0, 0, "$N is deleted due to %s.", msg);
	RESET_FIGHT_TIME(victim);
	do_quit_count(victim, "");
	unlink(strsave);
}

/*
 * handle_pc_death - called from `handle_death'
 * Returns TRUE if victim `became a ghost permanently' (is deleted permanently)
 */
static inline
bool
handle_pc_death(CHAR_DATA *ch, CHAR_DATA *victim)
{
	int epl;
	int lost_exp;

	/*
	 * Dying penalty: 2/3 way back.
	 */
	epl = exp_per_level(victim, victim->pcdata->points);
	if (victim->exp > epl * victim->level) {
		lost_exp = 2*(epl*victim->level - victim->exp)/3 + 50;
		gain_exp(victim, lost_exp);
	}

	if ((++victim->pcdata->death % 3) != 2)
		return FALSE;

	/*
	 *  Die too much and is deleted ... :(
	 */
	if (victim->class == CLASS_SAMURAI) {
		victim->perm_stat[STAT_CHA]--;
		if (victim->pcdata->death > 10) {
			delete_player(victim, "10 deaths limit of Samurai");
			return TRUE;
		}
	}
	else {
		if (--victim->perm_stat[STAT_CON] < 3) {
			delete_player(victim, "lack of CON");
			return TRUE;
		}
		else
			send_to_char("You feel your life power has decreased "
				     "with this death.\n\r", victim);
	}

	return FALSE;
}

/*
 * handle_death - called from `damage' if `ch' has killed `victim'
 */
static inline
void
handle_death(CHAR_DATA *ch, CHAR_DATA *victim)
{
	OBJ_DATA *corpse;

	group_gain(ch, victim);

	if (IS_NPC(ch) && ch->pIndexData->vnum == MOB_VNUM_STALKER)
		ch->status = 10;

	/*
	 * Death trigger
	 */
	if (IS_NPC(victim) && HAS_TRIGGER(victim, TRIG_DEATH)) {
		victim->position = POS_STANDING;
		mp_percent_trigger(victim, ch, NULL, NULL, TRIG_DEATH);
	}

	raw_kill(ch, victim);

	/*
	 * handle PC death (from NPC or being WANTED)
	 */
	if (!IS_NPC(victim)
	&&  (victim == ch ||
	     (IS_NPC(ch) && ch->master == NULL && ch->leader == NULL) ||
	     IS_SET(victim->act, PLR_WANTED))
	&&  handle_pc_death(ch, victim))
		return;

	/* don't remember killed victims anymore */
	if (IS_NPC(ch))
		remove_mind(ch, victim->name);

	/* RT new auto commands */
	if (!IS_NPC(ch) && IS_NPC(victim)
	&&  (corpse = get_obj_list(ch, "corpse", ch->in_room->contents)) != NULL) {
		if (ch->class == CLASS_VAMPIRE && ch->level > 10) {
			act_puts("$n suck {Rblood{x from $N's corpse!!",
				 ch, NULL,victim,TO_ROOM,POS_SLEEPING);
			send_to_char("You suck {Rblood{x "
				     "from the corpse!!\n\r\n\r", ch);
			gain_condition(ch, COND_BLOODLUST, 3);
		}

		if (corpse->contains)
			/* corpse exists and not empty */
			if (IS_SET(ch->act, PLR_AUTOLOOT))
				do_get(ch, "all corpse");
			else if (IS_SET(ch->act, PLR_AUTOGOLD))
				get_gold_corpse(ch, corpse);

		if (IS_SET(ch->act, PLR_AUTOSAC))
			do_sacrifice(ch, "corpse");
	}
}


/*
 * Inflict damage from a hit.
 */
bool damage(CHAR_DATA *ch, CHAR_DATA *victim,
	    int dam, int dt, int dam_type, bool show)
{
	bool immune;
	int dam2;

	if (victim->position == POS_DEAD)
		return FALSE;

	if (victim != ch) {
		/*
		 * Certain attacks are forbidden.
		 * Most other attacks are returned.
		 */
		if (is_safe(ch, victim))
			return FALSE;

		if (victim->position > POS_STUNNED) {
			if (victim->fighting == NULL) {
				set_fighting(victim, ch);
				if (IS_NPC(victim)
				&&  HAS_TRIGGER(victim, TRIG_KILL))
					mp_percent_trigger(victim, ch, NULL,
							   NULL, TRIG_KILL);
			}
			if (victim->timer <= 4)
				victim->position = POS_FIGHTING;
		}

		if (victim->position > POS_STUNNED) {
			if (ch->fighting == NULL)
				set_fighting(ch, victim);

			/*
			 * If victim is charmed, ch might attack
			 * victim's master.
			 */
			if (IS_NPC(ch)
			&&  IS_NPC(victim)
			&&  IS_AFFECTED(victim, AFF_CHARM)
			&&  victim->master != NULL
			&&  victim->master->in_room == ch->in_room
			&&  number_bits(3) == 0) {
				stop_fighting(ch, FALSE);
				multi_hit(ch, victim->master, TYPE_UNDEFINED);
				return FALSE;
			}
		}

		/*
		 * More charm and group stuff.
		 */
		if (victim->master == ch)
			stop_follower(victim);

		if (MOUNTED(victim) == ch || RIDDEN(victim) == ch)
			victim->riding = ch->riding = FALSE;
	}

	/*
	 * No one in combat can sneak, hide, or be invis or camoed.
	 */
	if (IS_SET(ch->affected_by, AFF_HIDE)
	||  IS_SET(ch->affected_by, AFF_INVISIBLE)
	||  IS_SET(ch->affected_by, AFF_SNEAK)
	||  IS_SET(ch->affected_by, AFF_FADE)
	||  IS_SET(ch->affected_by, AFF_CAMOUFLAGE)
	||  IS_SET(ch->affected_by, AFF_IMP_INVIS))
		do_visible(ch, "");

	/*
	 * Damage modifiers.
	 */
	if (IS_AFFECTED(victim, AFF_SANCTUARY)
	&&  !((dt == gsn_cleave) && (number_percent() < 50)))
		dam /= 2;

	if (IS_AFFECTED(victim, AFF_PROTECT_EVIL) && IS_EVIL(ch))
		dam -= dam / 4;

	if (IS_AFFECTED(victim, AFF_PROTECT_GOOD) && IS_GOOD(ch))
		dam -= dam / 4;

	if (is_affected(victim,gsn_resistance))
		dam = (3 * dam)/5;

	if (is_affected(victim, gsn_protection_heat) && (dam_type == DAM_FIRE))
		dam -= dam / 4;
	if (is_affected(victim, gsn_protection_cold) && (dam_type == DAM_COLD))
		dam -= dam / 4;

	immune = FALSE;


	/*
	 * Check for parry, and dodge.
	 */
	if (dt >= TYPE_HIT && ch != victim) {
		if (check_parry(ch, victim))
			return FALSE;
		if (check_block(ch, victim))
			return FALSE;
		if (check_dodge(ch, victim))
			return FALSE;
		if (check_blink(ch, victim))
			return FALSE;
	}

	switch(check_immune(victim, dam_type)) {
	case IS_IMMUNE:
		immune = TRUE;
		dam = 0;
		break;

	case IS_RESISTANT:
		dam -= dam/3;
		break;

	case IS_VULNERABLE:
		dam += dam/2;
		break;
	}

	if (dt != TYPE_HUNGER &&  dt >= TYPE_HIT && ch != victim) {
		if ((dam2 = critical_strike(ch,victim,dam)) != 0)
			dam = dam2;
	}

	if (show)
		dam_message(ch, victim, dam, dt, immune ,dam_type);

	if (dam == 0)
		return FALSE;

	/* temporarily second wield doesn't inflict damage */

	if (dt != TYPE_HUNGER && (dt >= TYPE_HIT && ch != victim))
		check_weapon_destroy(ch, victim , FALSE);

	/*
	 * Hurt the victim.
	 * Inform the victim of his new state.
	 */
	victim->hit -= dam;
	if (!IS_NPC(victim)
	&&  victim->level >= LEVEL_IMMORTAL &&  victim->hit < 1)
		victim->hit = 1;

	update_pos(victim);

	switch(victim->position) {
	case POS_MORTAL:
		if (dam_type == DAM_HUNGER || dam_type == DAM_THIRST) break;
		act("$n is mortally wounded, and will die soon, if not aided.",
		    victim, NULL, NULL, TO_ROOM);
		send_to_char( "You are mortally wounded, and will die soon, if not aided.\n\r", victim);
		break;

	case POS_INCAP:
		if (dam_type == DAM_HUNGER || dam_type == DAM_THIRST) break;
		act("$n is incapacitated and will slowly die, if not aided.",
		    victim, NULL, NULL, TO_ROOM);
		send_to_char( "You are incapacitated and will slowly die, if not aided.\n\r", victim);
		break;

	case POS_STUNNED:
		if (dam_type == DAM_HUNGER || dam_type == DAM_THIRST) break;
		act("$n is stunned, but will probably recover.",
		    victim, NULL, NULL, TO_ROOM);
		send_to_char("You are stunned, but will probably recover.\n\r",
			     victim);
		break;

	case POS_DEAD:
		act("$n is DEAD!!", victim, 0, 0, TO_ROOM);
		send_to_char("You have been KILLED!!\n\r\n\r", victim);
		break;

	default:
		if (dam_type == DAM_HUNGER || dam_type == DAM_THIRST) break;
		if (dam > victim->max_hit / 4)
			send_to_char("That really did HURT!\n\r", victim);
		if (victim->hit < victim->max_hit / 4)
			send_to_char("You sure are BLEEDING!\n\r", victim);
		break;
	}

	/*
	 * Sleep spells and extremely wounded folks.
	 */
	if (!IS_AWAKE(victim))
		stop_fighting(victim, FALSE);

	/*
	 * Payoff for killing things.
	 */
	if (victim->position == POS_DEAD) {
		handle_death(ch, victim);
		return TRUE;
	}

	if (victim == ch)
		return TRUE;

	/*
	 * Take care of link dead people.
	 */
	if (!IS_NPC(victim) && victim->desc == NULL) {
		if (number_range(0, victim->wait) == 0) {
			if (victim->level < 11)
				do_recall(victim, "");
			else
				do_flee(victim, "");
			return TRUE;
		}
	}

	/*
	 * Wimp out?
	 */
	if (IS_NPC(victim) && dam > 0 && victim->wait < PULSE_VIOLENCE / 2) {
		if ((IS_SET(victim->act, ACT_WIMPY) && number_bits(2) == 0 &&
		     victim->hit < victim->max_hit / 5)
		||  (IS_AFFECTED(victim, AFF_CHARM) &&
		     victim->master != NULL &&
		     victim->master->in_room != victim->in_room)
		||  (CAN_DETECT(victim, ADET_FEAR) &&
		     !IS_SET(victim->act, ACT_NOTRACK))) {
			do_flee(victim, "");
			victim->last_fought = NULL;
		}
	}

	if (!IS_NPC(victim)
	&&  victim->hit > 0
	&&  (victim->hit <= victim->wimpy || CAN_DETECT(victim, ADET_FEAR))
	&&  victim->wait < PULSE_VIOLENCE / 2)
		do_flee(victim, "");

	tail_chain();
	return TRUE;
}

bool is_safe(CHAR_DATA *ch, CHAR_DATA *victim)
{
	if (is_safe_nomessage(ch,victim)) {
		act("The gods protect $N.",ch,NULL,victim,TO_CHAR);
		act("The gods protect $N from $n.",ch,NULL,victim,TO_ROOM);
		return TRUE;
	}
	return FALSE;
}



bool is_safe_nomessage(CHAR_DATA *ch, CHAR_DATA *victim)
{
	if (victim->fighting == ch || ch==victim)
		return FALSE;

	/* Ghosts are safe */
	if ((!IS_NPC(victim) && IS_SET(victim->act, PLR_GHOST))
	||  (!IS_NPC(ch) && IS_SET(ch->act, PLR_GHOST)))
		return TRUE;

	/* handle ROOM_SAFE flags */
	if (victim->in_room != NULL
	&&  IS_SET(victim->in_room->room_flags, ROOM_SAFE))
		return TRUE;

	/* Experimental vampires' coffins handle */
	if (victim->on && IS_SET(victim->on->value[2], SLEEP_IN)
	&&  victim->on->pIndexData->vnum == 1200
	&&  time_info.hour >= 6 && time_info.hour <= 18
	&&  victim->class == CLASS_VAMPIRE)
		return TRUE;


	/* link dead players whose adrenalin is not gushing are safe */
	if (!IS_NPC(victim) && !IS_PUMPED(victim) && victim->desc == NULL)
		return TRUE;

	/* newly death staff */
	if (!IS_IMMORTAL(ch) && !IS_NPC(victim)
	&&  ch->last_death_time != -1
	&&  (current_time - ch->last_death_time < 600 ||
	     current_time - victim->last_death_time < 600))
		return TRUE;

	return !in_PK(ch, victim);
}



bool is_safe_spell(CHAR_DATA *ch, CHAR_DATA *victim, bool area)
{
	if (ch == victim && !area)
		return TRUE;

	if (IS_IMMORTAL(victim) &&	area)
		return TRUE;

	if (is_same_group(ch,victim) && area)
		return TRUE;

	if (ch == victim && area && ch->in_room->sector_type == SECT_INSIDE)
		return TRUE;

	if ((RIDDEN(ch) == victim || MOUNTED(ch) == victim) && area)
		return TRUE;

	return is_safe(ch,victim);
}


/*
 * Check for parry.
 */
bool check_parry(CHAR_DATA *ch, CHAR_DATA *victim)
{
	int chance;

	if (!IS_AWAKE(victim))
		return FALSE;

	if (IS_NPC(victim))
	{
		chance	= UMIN(35, victim->level);
	}
	else
	{
		if (get_eq_char(victim, WEAR_WIELD) == NULL)
		    return FALSE;
		chance	= get_skill(victim,gsn_parry) / 2;
		if (victim->class == CLASS_WARRIOR || victim->class == CLASS_SAMURAI)
		chance *= 1.2;
	}


	if (number_percent() >= chance + victim->level - ch->level)
		return FALSE;

	act("You parry $n's attack.",  ch, NULL, victim, TO_VICT	);
	act("$N parries your attack.", ch, NULL, victim, TO_CHAR	);
	check_weapon_destroyed(ch, victim, FALSE);
	if (number_percent() >  get_skill(victim,gsn_parry))
	{
	 /* size  and weight */
	 chance += ch->carry_weight / 25;
	 chance -= victim->carry_weight / 20;

	 if (ch->size < victim->size)
		chance += (ch->size - victim->size) * 25;
	 else
		chance += (ch->size - victim->size) * 10;


	 /* stats */
	 chance += get_curr_stat(ch,STAT_STR);
	 chance -= get_curr_stat(victim,STAT_DEX) * 4/3;

	 if (IS_AFFECTED(ch,AFF_FLYING))
		chance -= 10;

	 /* speed */
	 if (IS_SET(ch->off_flags,OFF_FAST))
		chance += 10;
	 if (IS_SET(victim->off_flags,OFF_FAST))
		chance -= 20;

	 /* level */
	 chance += (ch->level - victim->level) * 2;

	 /* now the attack */
	 if (number_percent() < (chance / 20 ))
	 {
		act("You couldn't manage to keep your position!",
			ch,NULL,victim,TO_VICT);
		act("You fall down!",ch,NULL,victim,TO_VICT);
		act("$N couldn't manage to hold your attack and falls down!",
			ch,NULL,victim,TO_CHAR);
		act("$n stunning force makes $N falling down.",
			ch,NULL,victim,TO_NOTVICT);

		WAIT_STATE(victim,skill_table[gsn_bash].beats);
		victim->position = POS_RESTING;
	 }
	}
	check_improve(victim,gsn_parry,TRUE,6);
	return TRUE;
}

/*
 * check blink
 */
bool check_blink(CHAR_DATA *ch, CHAR_DATA *victim)
{
	int chance;

	if (!IS_BLINK_ON(victim))
		return FALSE;

	if (IS_NPC(victim))
		return FALSE;
	else
		chance	= victim->pcdata->learned[gsn_blink] / 2;

	if ((number_percent() >= chance + victim->level - ch->level)
		|| (number_percent() < 50)
		|| (victim->mana < 10))
		return FALSE;

	victim->mana -= UMAX(victim->level / 10,1);

	act("You blink out $n's attack.", ch, NULL, victim, TO_VICT   );
	act("$N blinks out your attack.", ch, NULL, victim, TO_CHAR   );
	check_improve(victim,gsn_blink,TRUE,6);
	return TRUE;
}


/*
 * Check for shield block.
 */

bool check_block(CHAR_DATA *ch, CHAR_DATA *victim)
{
	int chance;

	if (!IS_AWAKE(victim))
		return FALSE;

	if (get_eq_char(victim, WEAR_SHIELD) == NULL)
	  return FALSE;

	if (IS_NPC(victim))
	{
	  chance = 10;
	}
	else
	{
	 if (get_skill(victim,gsn_shield_block) <= 1)
		return FALSE;
	 chance = get_skill(victim,gsn_shield_block) / 2;
	 chance -= (victim->class == CLASS_WARRIOR) ? 0 : 10;
	}

	if (number_percent() >= chance + victim->level - ch->level)
		return FALSE;

	act("Your shield blocks $n's attack.",  ch, NULL, victim, TO_VICT	 );
	act("$N deflects your attack with $S shield.", ch,NULL,victim,TO_CHAR);
	check_shield_destroyed(ch, victim, FALSE);
	check_improve(victim,gsn_shield_block,TRUE,6);
	return TRUE;
}



/*
 * Check for dodge.
 */
bool check_dodge(CHAR_DATA *ch, CHAR_DATA *victim)
{
	int chance;

	if (!IS_AWAKE(victim))
		return FALSE;

	if (MOUNTED(victim))
		return FALSE;

	if (IS_NPC(victim))
		 chance  = UMIN(30, victim->level);
	else {
		  chance  = get_skill(victim,gsn_dodge) / 2;
		  /* chance for high dex. */
		  chance += 2 * (get_curr_stat(victim,STAT_DEX) - 20);
		 if (victim->class == CLASS_WARRIOR || victim->class == CLASS_SAMURAI)
		    chance *= 1.2;
		if (victim->class == CLASS_THIEF || victim->class ==CLASS_NINJA)
		    chance *= 1.1;
		 }

	if (number_percent() >= chance + (victim->level - ch->level) / 2)
		return FALSE;

	act("You dodge $n's attack.", ch, NULL, victim, TO_VICT	);
	act("$N dodges your attack.", ch, NULL, victim, TO_CHAR	);
   if (number_percent() < (get_skill(victim,gsn_dodge) / 20)
		&& !(IS_AFFECTED(ch,AFF_FLYING) || ch->position < POS_FIGHTING))
   {
	 /* size */
	 if (victim->size < ch->size)
		chance += (victim->size - ch->size) * 10;  /* bigger = harder to trip */

	 /* dex */
	 chance += get_curr_stat(victim,STAT_DEX);
	 chance -= get_curr_stat(ch,STAT_DEX) * 3 / 2;

	 if (IS_AFFECTED(victim,AFF_FLYING))
		chance -= 10;

	 /* speed */
	 if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
		chance += 10;
	 if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
		chance -= 20;

	 /* level */
	 chance += (victim->level - ch->level) * 2;


	 /* now the attack */
	 if (number_percent() < (chance / 20))
	 {
		act("$n lost his postion and fall down!",ch,NULL,victim,TO_VICT);
		act("As $N moves you lost your position fall down!",ch,NULL,victim,TO_CHAR);
		act("As $N dodges $N's attack ,$N lost his position and falls down.",ch,NULL,victim,TO_NOTVICT);

		WAIT_STATE(ch,skill_table[gsn_trip].beats);
		ch->position = POS_RESTING;
	 }
	}
	check_improve(victim,gsn_dodge,TRUE,6);
	return TRUE;
}



/*
 * Set position of a victim.
 */
void update_pos(CHAR_DATA *victim)
{
	if (victim->hit > 0)
	{
		if (victim->position <= POS_STUNNED)
		    victim->position = POS_STANDING;
		return;
	}

	if (IS_NPC(victim) && victim->hit < 1)
	{
		victim->position = POS_DEAD;
		return;
	}

	if (victim->hit <= -11)
	{
		victim->position = POS_DEAD;
		return;
	}

		 if (victim->hit <= -6) victim->position = POS_MORTAL;
	else if (victim->hit <= -3) victim->position = POS_INCAP;
	else			  victim->position = POS_STUNNED;

	return;
}



/*
 * Start fights.
 */
void set_fighting(CHAR_DATA *ch, CHAR_DATA *victim)
{
	if (ch->fighting != NULL)
	{
		bug("Set_fighting: already fighting", 0);
		return;
	}

	if (IS_AFFECTED(ch, AFF_SLEEP))
		affect_strip(ch, gsn_sleep);

	ch->fighting = victim;
	ch->position = POS_FIGHTING;

	return;
}



/*
 * Stop fights.
 */
void stop_fighting(CHAR_DATA *ch, bool fBoth)
{
	CHAR_DATA *fch;

	for (fch = char_list; fch != NULL; fch = fch->next)
	{
		if (fch == ch || (fBoth && fch->fighting == ch))
		{
		    fch->fighting	= NULL;
		    fch->position	= IS_NPC(fch) ? ch->default_pos : POS_STANDING;
		    update_pos(fch);
		}
	}

	return;
}



/*
 * Make a corpse out of a character.
 */
void make_corpse(CHAR_DATA *ch)
{
	OBJ_DATA *corpse;
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;
	char *name;
	int i;

	if (IS_NPC(ch))
	{
		name		= ch->short_descr;
		corpse		= create_object(get_obj_index(OBJ_VNUM_CORPSE_NPC), 0);
		corpse->timer	= number_range(3, 6);
		if (ch->gold > 0 || ch->silver > 0)
		  {
		    if (IS_SET(ch->form,FORM_INSTANT_DECAY))
		      obj_to_room(create_money(ch->gold, ch->silver), ch->in_room);
		    else
		      obj_to_obj(create_money(ch->gold, ch->silver), corpse);
		    ch->gold = 0;
		}
		corpse->from = str_dup(ch->short_descr);
		corpse->cost = 0;
	}
	else
	  {
		if (IS_GOOD(ch))
		  i = 0;
		if (IS_EVIL(ch))
		  i = 2;
		else
		  i = 1;

		name		= ch->name;
		corpse		= create_object(get_obj_index(OBJ_VNUM_CORPSE_PC), 0);
		corpse->timer	= number_range(25, 40);
		REMOVE_BIT(ch->act,PLR_CANLOOT);
		corpse->owner = str_dup(ch->name);
		corpse->from = str_dup(ch->name);
		corpse->altar = hometown_table[ch->hometown].altar[i];
		corpse->pit = hometown_table[ch->hometown].pit[i];

		if (ch->gold > 0 || ch->silver > 0) {
		    obj_to_obj(create_money(ch->gold, ch->silver), corpse);
		    ch->gold = 0;
		    ch->silver = 0;
		}
		corpse->cost = 0;
	}

	corpse->level = ch->level;

	str_printf(&corpse->short_descr, name);
	str_printf(&corpse->description, name);

	for (obj = ch->carrying; obj != NULL; obj = obj_next)
	{
		obj_next = obj->next_content;
		obj_from_char(obj);
		if (obj->item_type == ITEM_POTION)
		    obj->timer = number_range(500,1000);
		if (obj->item_type == ITEM_SCROLL)
		    obj->timer = number_range(1000,2500);
		if (IS_SET(obj->extra_flags,ITEM_ROT_DEATH))  {
		    obj->timer = number_range(5,10);
		    if (obj->item_type == ITEM_POTION)
		       obj->timer += obj->level * 20;
		}
		REMOVE_BIT(obj->extra_flags,ITEM_VIS_DEATH);
		REMOVE_BIT(obj->extra_flags,ITEM_ROT_DEATH);

		if (IS_SET(obj->extra_flags, ITEM_INVENTORY)  ||
		    (obj->pIndexData->limit != -1 &&
			(obj->pIndexData->count > obj->pIndexData->limit)))
		  {
		    extract_obj(obj);
		    continue;
		  }
		else if (IS_SET(ch->form,FORM_INSTANT_DECAY))
		  obj_to_room(obj, ch->in_room);

		else
		  obj_to_obj(obj, corpse);
	}

	obj_to_room(corpse, ch->in_room);
	return;
}


void death_cry(CHAR_DATA *ch)
{
  death_cry_org(ch, -1);
}

/*
 * Improved Death_cry contributed by Diavolo.
 */
void death_cry_org(CHAR_DATA *ch, int part)
{
	ROOM_INDEX_DATA *was_in_room;
	char *msg;
	int door;
	int vnum;

	vnum = 0;
	msg = "You hear $n's death cry.";

	if (part == -1)
	  part = number_bits(4);

	switch (part) {
	case  0:
		msg  = "$n hits the ground ... DEAD.";
		break;
	case  1:
		if (ch->material == 0) {
		    msg  = "$n splatters blood on your armor.";
		    break;
		}
		/* FALLTHRU */
	case  2:
		if (IS_SET(ch->parts,PART_GUTS)) {
			msg = "$n spills $s guts all over the floor.";
			vnum = OBJ_VNUM_GUTS;
		}
		break;
	case  3:
		if (IS_SET(ch->parts,PART_HEAD)) {
			msg  = "$n's severed head plops on the ground.";
			vnum = OBJ_VNUM_SEVERED_HEAD;
		}
		break;
	case  4:
		if (IS_SET(ch->parts,PART_HEART)) {
			msg  = "$n's heart is torn from $s chest.";
			vnum = OBJ_VNUM_TORN_HEART;
		}
		break;
	case  5:
		if (IS_SET(ch->parts,PART_ARMS)) {
			msg  = "$n's arm is sliced from $s dead body.";
			vnum = OBJ_VNUM_SLICED_ARM;
		}
		break;
	case  6:
		if (IS_SET(ch->parts,PART_LEGS)) {
			msg  = "$n's leg is sliced from $s dead body.";
			vnum = OBJ_VNUM_SLICED_LEG;
		}
		break;
	case 7:
		if (IS_SET(ch->parts,PART_BRAINS)) {
			msg = "$n's head is shattered, and $s brains splash all over you.";
			vnum = OBJ_VNUM_BRAINS;
		}
		break;
	}

	act(msg, ch, NULL, NULL, TO_ROOM);

	if (vnum != 0) {
		OBJ_DATA *obj;
		char *name;

		name		= IS_NPC(ch) ? ch->short_descr : ch->name;
		obj		= create_object(get_obj_index(vnum), 0);
		obj->timer	= number_range(4, 7);

		str_printf(&obj->short_descr, obj->short_descr, name);
		str_printf(&obj->description, obj->description, name);

		obj->from = str_dup(name);

		if (obj->item_type == ITEM_FOOD) {
			if (IS_SET(ch->form,FORM_POISON))
				obj->value[3] = 1;
			else if (!IS_SET(ch->form,FORM_EDIBLE))
				obj->item_type = ITEM_TRASH;
		}

		obj_to_room(obj, ch->in_room);
	}

	if (IS_NPC(ch))
		msg = "You hear something's death cry.";
	else
		msg = "You hear someone's death cry.";

	was_in_room = ch->in_room;
	for (door = 0; door <= 5; door++) {
		EXIT_DATA *pexit;

		if ((pexit = was_in_room->exit[door]) != NULL
		&&   pexit->u1.to_room != NULL
		&&   pexit->u1.to_room != was_in_room) {
			ch->in_room = pexit->u1.to_room;
			act(msg, ch, NULL, NULL, TO_ROOM);
		}
	}
	ch->in_room = was_in_room;

	return;
}


void raw_kill_org(CHAR_DATA *ch, CHAR_DATA *victim, int part)
{
	CHAR_DATA *tmp_ch;
	OBJ_DATA *obj,*obj_next;
	int i;
	OBJ_DATA *tattoo;

	for (obj = victim->carrying;obj != NULL;obj = obj_next) {
		obj_next = obj->next_content;
		if (obj->wear_loc != WEAR_NONE
		&&  oprog_call(OPROG_DEATH, obj, victim, NULL)) {
			victim->position = POS_STANDING;
			return;
		}
	}

	stop_fighting(victim, TRUE);
	rating_update(ch, victim);

	quest_handle_death(ch, victim);

	RESET_FIGHT_TIME(victim);
	victim->last_death_time = current_time;

	tattoo = get_eq_char(victim, WEAR_TATTOO);
	if (tattoo != NULL)
		obj_from_char(tattoo);

	death_cry_org(victim, part);
	make_corpse(victim);


	if (IS_NPC(victim)) {
		victim->pIndexData->killed++;
		kill_table[URANGE(0, victim->level, MAX_LEVEL-1)].killed++;
		extract_char(victim, TRUE);
		return;
	}

	char_puts("You turn into an invincible ghost for a few minutes.\n\r"
		  "As long as you don't attack anything.\n\r",
		  victim);

	extract_char(victim, FALSE);

	while (victim->affected)
		affect_remove(victim, victim->affected);
	victim->affected_by	= 0;
	victim->detection	= 0;
	for (i = 0; i < 4; i++)
		victim->armor[i] = 100;
	victim->position	= POS_RESTING;
	victim->hit		= victim->max_hit / 10;
	victim->mana		= victim->max_mana / 10;
	victim->move		= victim->max_move;

	/* RT added to prevent infinite deaths */
	REMOVE_BIT(victim->act, PLR_WANTED);
	REMOVE_BIT(victim->act, PLR_BOUGHT_PET);
	/*  SET_BIT(victim->act, PLR_GHOST);	*/

	victim->pcdata->condition[COND_THIRST] = 40;
	victim->pcdata->condition[COND_HUNGER] = 40;
	victim->pcdata->condition[COND_FULL] = 40;
	victim->pcdata->condition[COND_BLOODLUST] = 40;
	victim->pcdata->condition[COND_DESIRE] = 40;

	if (tattoo != NULL) {
		obj_to_char(tattoo, victim);
		equip_char(victim, tattoo, WEAR_TATTOO);
	}
	save_char_obj(victim);

	/*
	 * Calm down the tracking mobiles
	 */
	for (tmp_ch = char_list; tmp_ch != NULL; tmp_ch = tmp_ch->next) {
		if (tmp_ch->last_fought == victim)
			tmp_ch->last_fought = NULL;
		remove_mind(tmp_ch, victim->name);
	}
}



void group_gain(CHAR_DATA *ch, CHAR_DATA *victim)
{
	CHAR_DATA *lch;
	CHAR_DATA *gch;
	int xp;
	int members;
	int group_levels;

	if (victim == ch
	||  (IS_NPC(victim) && victim->pIndexData->vnum < 100))
		return;

	if (!IS_NPC(victim))
		return;

	if (IS_NPC(victim)
	&&  (victim->master != NULL || victim->leader != NULL))
		return;

	members = 1;
	group_levels = 0;
	for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room)
	{
		if (is_same_group(gch, ch))
		{
		    if (!IS_NPC(gch) && gch != ch)
		      members++;
		    group_levels += gch->level;
		}
	}

	lch = (ch->leader != NULL) ? ch->leader : ch;

	for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room)
	{
		OBJ_DATA *obj;
		OBJ_DATA *obj_next;

		if (!is_same_group(gch, ch) || IS_NPC(gch))
		    continue;


		if (gch->level - lch->level > 8)
		{
		    send_to_char("You are too high for this group.\n\r", gch);
		    continue;
		}

		if (gch->level - lch->level < -8)
		{
		    send_to_char("You are too low for this group.\n\r", gch);
		    continue;
		}


		xp = xp_compute(gch, victim, group_levels,members);
		char_printf(gch, "You receive %d experience points.\n\r", xp);
		gain_exp(gch, xp);

		for (obj = ch->carrying; obj != NULL; obj = obj_next)
		{
		    obj_next = obj->next_content;
		    if (obj->wear_loc == WEAR_NONE)
			continue;

		    if ((IS_OBJ_STAT(obj, ITEM_ANTI_EVIL)    && IS_EVIL(ch)	)
		    ||	 (IS_OBJ_STAT(obj, ITEM_ANTI_GOOD)    && IS_GOOD(ch)	)
		    ||	 (IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch)))
		    {
			act("You are zapped by $p.", ch, obj, NULL, TO_CHAR);
			act("$n is zapped by $p.",   ch, obj, NULL, TO_ROOM);
			obj_from_char(obj);
			obj_to_room(obj, ch->in_room);
		    }
		}
	}

	return;
}



/*
 * Compute xp for a kill.
 * Also adjust alignment of killer.
 * Edit this function to change xp computations.
 */
int xp_compute(CHAR_DATA *gch, CHAR_DATA *victim, int total_levels,int members)
{
  int xp;
  int base_exp;
  int level_range;
  int neg_cha=0, pos_cha=0;

  level_range = victim->level - gch->level;

  switch (level_range)
	{
	default :	base_exp =   0; 	break;
  case -9 :	base_exp =   1; 	break;
  case -8 :	base_exp =   2; 	break;
  case -7 :	base_exp =   5; 	break;
  case -6 :	base_exp =   9; 	break;
  case -5 :	base_exp =  11; 	break;
  case -4 :	base_exp =  22; 	break;
  case -3 :	base_exp =  33; 	break;
  case -2 :	base_exp =  43; 	break;
  case -1 :	base_exp =  60; 	break;
  case	0 :	base_exp =  74; 	break;
  case	1 :	base_exp =  84; 	break;
  case	2 :	base_exp =  99; 	break;
  case	3 :	base_exp = 121; 	break;
  case	4 :	base_exp = 143; 	break;
  }

  if (level_range > 4)
	base_exp = 140 + 20 * (level_range - 4);


  /* calculate exp multiplier */
  if (IS_SET(victim->act,ACT_NOALIGN))
	xp = base_exp;

  /* alignment */
  else if ((IS_EVIL(gch) && IS_GOOD(victim)) || (IS_EVIL(victim) && IS_GOOD(gch)))
	xp = base_exp * 8/5;

  else if (IS_GOOD(gch) && IS_GOOD(victim))
	xp = 0;

  else if (!IS_NEUTRAL(gch) && IS_NEUTRAL(victim))
	xp = base_exp * 1.1;

  else if (IS_NEUTRAL(gch) && !IS_NEUTRAL(victim))
	xp = base_exp * 1.3;

  else xp = base_exp;

	/* more exp at the low levels */
	if (gch->level < 6)
		xp = 15 * xp / (gch->level + 4);

	/* randomize the rewards */
	xp = number_range (xp * 3/4, xp * 5/4);

	/* adjust for grouping */
	xp = xp * gch->level/total_levels;

	if (members == 2 || members == 3)
	  xp *= (3 / 2);

	if (gch->level < 15)
		 xp = UMIN((250 + dice(1,25)),xp);
	else if (gch->level < 40)
		xp = UMIN((225 + dice(1,20)),xp);
	else if (gch->level < 60)
		xp = UMIN((200 + dice(1,20)),xp);
	else xp = UMIN((180 + dice(1,20)),xp);

	xp += (xp * (gch->max_hit - gch->hit)) / (gch->max_hit * 5);

	if (IS_GOOD(gch))
		{
		 if (IS_GOOD(victim)) { gch->pcdata->anti_killed++; neg_cha = 1; }
		 else if (IS_NEUTRAL(victim)) {gch->pcdata->has_killed++; pos_cha =1;}
		 else if (IS_EVIL(victim)) {gch->pcdata->has_killed++; pos_cha = 1;}
		}

	if (IS_NEUTRAL(gch))
		{
		 if (xp > 0)
		 {
		  if (IS_GOOD(victim)) {gch->pcdata->has_killed++; pos_cha = 1;}
		  else if (IS_NEUTRAL(victim)) {gch->pcdata->anti_killed++; neg_cha = 1;}
		  else if (IS_EVIL(victim)) {gch->pcdata->has_killed++; pos_cha =1;}
		 }
		}

	if (IS_EVIL(gch))
		{
		 if (xp > 0)
		 {
		  if (IS_GOOD(victim)) {gch->pcdata->has_killed++; pos_cha = 1;}
		  else if (IS_NEUTRAL(victim)) {gch->pcdata->has_killed++; pos_cha = 1;}
		  else if (IS_EVIL(victim)) {gch->pcdata->anti_killed++; neg_cha = 1;}
		 }
		}

 if (neg_cha)
  {
	if ((gch->pcdata->anti_killed % 100) == 99)
		{
		 char_printf(gch,"You have killed %d %s up to now.\n\r",
				gch->pcdata->anti_killed,
			IS_GOOD(gch) ? "goods" :
			IS_NEUTRAL(gch) ? "neutrals" :
			IS_EVIL(gch) ? "evils" : "nones");
		 if (gch->perm_stat[STAT_CHA] > 3 && IS_GOOD(gch))
		 {
		  send_to_char("So your charisma has reduced by one.\n\r", gch);
		  gch->perm_stat[STAT_CHA] -= 1;
		 }
		}
   }
  else if (pos_cha)
   {
	if ((gch->pcdata->has_killed % 200) == 199)
		{
		 char_printf(gch,"You have killed %d %s up to now.\n\r",
				gch->pcdata->anti_killed,
			IS_GOOD(gch) ? "anti-goods" :
			IS_NEUTRAL(gch) ? "anti-neutrals" :
			IS_EVIL(gch) ? "anti-evils" : "nones");
		  if (gch->perm_stat[STAT_CHA] < get_max_train(gch, STAT_CHA)
			&& IS_GOOD(gch))
		  {
		   send_to_char("So your charisma has increased by one.\n\r", gch);
		   gch->perm_stat[STAT_CHA] += 1;
		  }
		 }
   }
	return xp;
}


void dam_message(CHAR_DATA *ch, CHAR_DATA *victim,int dam,int dt,bool immune ,int dam_type)
{
	char buf1[256], buf2[256], buf3[256];
	const char *vs;
	const char *vp;
	const char *attack;
	char punct;

		 if (dam ==   0) { vs = "{cmiss{x"; vp = "{cmisses{x";}
	else if (dam <=   4) { vs = "{cscratch{x";  vp = "{cscratches{x";}
	else if (dam <=   8) { vs = "{cgraze{x";    vp = "{cgrazes{x";}
	else if (dam <=  12) { vs = "{chit{x";      vp = "{chits{x";}
	else if (dam <=  16) { vs = "{cinjure{x";   vp = "{cinjures{x"; }
	else if (dam <=  20) { vs = "{cwound{x";    vp = "{cwounds{x";}
	else if (dam <=  24) { vs = "{cmaul{x";     vp = "{cmauls{x";}
	else if (dam <=  28) { vs = "{cdecimate{x"; vp = "{cdecimates{x";}
	else if (dam <=  32) { vs = "{cdevastate{x";vp = "{cdevastates{x";}
	else if (dam <=  36) { vs = "{cmaim{x";	    vp = "{cmaims{x";}
	else if (dam <=  42) { vs = "{MMUTILATE{x";  vp = "{MMUTILATES{x"; }
	else if (dam <=  52) { vs = "{MDISEMBOWEL{x";vp = "{MDISEMBOWELS{x"; }
	else if (dam <=  65) { vs = "{MDISMEMBER{x"; vp = "{MDISMEMBERS{x";  }
	else if (dam <=  80) { vs = "{MMASSACRE{x";  vp = "{MMASSACRES{x"; }
	else if (dam <=  100) { vs = "{MMANGLE{x";   vp = "{MMANGLES{x"; }
	else if (dam <=  130) { vs = "{y*** DEMOLISH ***{x";
				     vp = "{y*** DEMOLISHES ***{x"; 		}
	else if (dam <= 175) { vs = "{y*** DEVASTATE ***{x";
				     vp = "{y*** DEVASTATES ***{x"; 		}
	else if (dam <= 250)  { vs = "{y=== OBLITERATE ==={x";
				     vp = "{y=== OBLITERATES ==={x";		}
	else if (dam <= 325)  { vs = "{y==== ATOMIZE ===={x";
				     vp = "{y==== ATOMIZES ===={x"; }
	else if (dam <= 400)  { vs = "{R<*> <*> ANNIHILATE <*> <*>{x";
				     vp = "{R<*> <*> ANNIHILATES <*> <*>{x";	}
	else if (dam <= 500)  { vs = "{R<*>!<*> ERADICATE <*>!<*>{x";
				     vp = "{R<*>!<*> ERADICATES <*>!<*>{x"; }
	else if (dam <= 650)  { vs = "{R<*><*><*> ELECTRONIZE <*><*><*>{x";
				     vp = "{R<*><*><*> ELECTRONIZES <*><*><*>{x";	}
	else if (dam <= 800)  { vs = "{R(<*>)!(<*>) SKELETONIZE (<*>)!(<*>){x";
				     vp = "{R(<*>)!(<*>) SKELETONIZES (<*>)!(<*>){x";}
	else if (dam <= 1000)  { vs = "{R(*)!(*)!(*) NUKE (*)!(*)!(*){x";
				     vp = "{R(*)!(*)!(*) NUKES (*)!(*)!(*){x";	}
	else if (dam <= 1250)  { vs = "{R(*)!<*>!(*) TERMINATE (*)!<*>!(*){x";
				     vp = "{R(*)!<*>!(*) TERMINATES (*)!<*>!(*){x"; }
	else if (dam <= 1500)  { vs = "{R<*>!(*)!<*>> TEAR UP <<*)!(*)!<*>{x";
				     vp = "{R<*>!(*)!<*>> TEARS UP <<*)!(*)!<*>{x"; }
	else		   { vs = "\007{R=<*) (*>= ! POWER HIT ! =<*) (*>={x\007";
				     vp = "\007{R=<*) (*>= ! POWER HITS ! =<*) (*>={x\007";}

	if (victim->level < 20)	punct	= (dam <= 24) ? '.' : '!';
	else if (victim->level < 50)  punct = (dam <= 50) ? '.' : '!';
	else punct = (dam <= 75) ? '.' : '!';

	if ((dt == TYPE_HIT)  || (dt == TYPE_HUNGER))
	{
		if (ch	== victim)
		{
		  if (dam_type == DAM_HUNGER)
		  {
		    sprintf(buf1, "$n's hunger %s $mself%c",vp,punct);
		    sprintf(buf2, "Your hunger %s yourself%c",vs,punct);
		  }

		  else if (dam_type == DAM_THIRST)
		  {
		    sprintf(buf1, "$n's thirst %s $mself%c",vp,punct);
		    sprintf(buf2, "Your thirst %s yourself%c",vs,punct);
		  }
		  else if (dam_type == DAM_LIGHT_V)
		  {
		    sprintf(buf1, "The light of room %s $n!%c",vp,punct);
		    sprintf(buf2, "The light of room %s you!%c",vs,punct);
		  }
		  else if (dam_type == DAM_TRAP_ROOM)
		  {
		    sprintf(buf1, "The trap at room %s $n!%c",vp,punct);
		    sprintf(buf2, "The trap at room %s you!%c",vs,punct);
		  }
		  else {
		    sprintf(buf1, "$n %s $mself%c",vp,punct);
		    sprintf(buf2, "You %s yourself%c",vs,punct);
			}
		}
		else
		{
		    sprintf(buf1, "$n %s $N%c",  vp, punct);
		    sprintf(buf2, "You %s $N%c", vs, punct);
		    sprintf(buf3, "$n %s you%c", vp, punct);
		}
	}

	else
	{
		if (dt >= 0 && dt < MAX_SKILL)
		    attack	= skill_table[dt].noun_damage;
		else if (dt >= TYPE_HIT
		&& dt <= TYPE_HIT + MAX_DAMAGE_MESSAGE)
		    attack	= attack_table[dt - TYPE_HIT].noun;
		else
		{
		    bug("Dam_message: bad dt %d.", dt);
		    dt	= TYPE_HIT;
		    attack  = attack_table[0].name;
		}

		if (immune)
		{
		    if (ch == victim)
		    {
			sprintf(buf1,"$n is unaffected by $s own %s.",attack);
			sprintf(buf2,"Luckily, you are immune to that.");
		    }
		    else
		    {
			sprintf(buf1,"$N is unaffected by $n's %s!",attack);
			sprintf(buf2,"$N is unaffected by your %s!",attack);
			sprintf(buf3,"$n's %s is powerless against you.",attack);
		    }
		}
		else
		{
		    if (ch == victim)
		    {
			sprintf(buf1, "$n's %s %s $m%c",attack,vp,punct);
			sprintf(buf2, "Your %s %s you%c",attack,vp,punct);
		    }
		    else
		    {
			sprintf(buf1, "$n's %s %s $N%c",  attack, vp, punct);
			sprintf(buf2, "Your %s %s $N%c",  attack, vp, punct);
			sprintf(buf3, "$n's %s %s you%c", attack, vp, punct);
		    }
		}
	}

	if (ch == victim)
	{
		if (dam <=36)
		{
		act_puts(buf1,ch,NULL,NULL,TO_ROOM,POS_RESTING);
		act_puts(buf2,ch,NULL,NULL,TO_CHAR,POS_RESTING);
		}
		else if (dam <=100)
		{
		act_puts(buf1,ch,NULL,NULL,TO_ROOM,POS_RESTING);
		act_puts(buf2,ch,NULL,NULL,TO_CHAR,POS_RESTING);
		}
		else if (dam <=325)
		{
		act_puts(buf1,ch,NULL,NULL,TO_ROOM,POS_RESTING);
		act_puts(buf2,ch,NULL,NULL,TO_CHAR,POS_RESTING);
		}
		else
		{
		act_puts(buf1,ch,NULL,NULL,TO_ROOM,POS_RESTING);
		act_puts(buf2,ch,NULL,NULL,TO_CHAR,POS_RESTING);
		}
	}
	else
	{
		if (dam <= 36)
		{
		act_puts(buf1, ch, NULL, victim, TO_NOTVICT,POS_RESTING);
		act_puts(buf2, ch, NULL, victim, TO_CHAR,POS_RESTING);
		act_puts(buf3, ch, NULL, victim, TO_VICT,POS_RESTING);
		}
		else if (dam <= 100)
		{
		act_puts(buf1, ch, NULL,victim,TO_NOTVICT,POS_RESTING);
		act_puts(buf2, ch, NULL,victim,TO_CHAR,POS_RESTING);
		act_puts(buf3, ch, NULL,victim,TO_VICT,POS_RESTING);
		}
		else if (dam <= 325)
		{
		act_puts(buf1, ch, NULL, victim, TO_NOTVICT,POS_RESTING);
		act_puts(buf2, ch, NULL, victim, TO_CHAR,POS_RESTING);
		act_puts(buf3, ch, NULL, victim, TO_VICT,POS_RESTING);
		}
		else
		{
		act_puts(buf1, ch, NULL, victim, TO_NOTVICT,POS_RESTING);
		act_puts(buf2, ch, NULL, victim, TO_CHAR,POS_RESTING);
		act_puts(buf3, ch, NULL, victim, TO_VICT,POS_RESTING);
		}

	}

	return;
}


void do_kill(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA *wield;

	one_argument(argument, arg);

	if (arg[0] == '\0')
	{
		send_to_char("Kill whom?\n\r", ch);
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		return;
	}

	if (is_safe(ch, victim))
		return;

	if (ch->position == POS_FIGHTING)
	{
	  if (victim == ch->fighting)
		send_to_char("You do the best you can!\n\r", ch);
	  else if (victim->fighting != ch)
		send_to_char("One battle at a time, please.\n\r",ch);
	  else
		{
		  act("You start aiming at $N.",ch,NULL,victim,TO_CHAR);
		  ch->fighting = victim;
		}
		return;
	}

	if (!IS_NPC(victim))
	{
		send_to_char("You must MURDER a player.\n\r", ch);
		return;
	}

	if (victim == ch)
	{
		send_to_char("You hit yourself.  Ouch!\n\r", ch);
		multi_hit(ch, ch, TYPE_UNDEFINED);
		return;
	}


	if (IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim)
	{
		act("$N is your beloved master.", ch, NULL, victim, TO_CHAR);
		return;
	}


	WAIT_STATE(ch, 1 * PULSE_VIOLENCE);

	if (SKILL_OK(ch, gsn_mortal_strike)
	&&  (wield = get_eq_char(ch,WEAR_WIELD)) != NULL
	&&  wield->level > (victim->level - 5)
	&&  clan_ok(ch,gsn_mortal_strike)) {
	  int chance = 1 + get_skill(ch,gsn_mortal_strike) / 30;
	  chance += (ch->level - victim->level) / 2;
	  if (number_percent() < chance)
		{
		 act_puts("Your flash strike instantly slays $N!",
			ch,NULL,victim,TO_CHAR,POS_RESTING);
		 act_puts("$n flash strike instantly slays $N!",
			ch,NULL,victim,TO_NOTVICT,POS_RESTING);
		 act_puts("$n flash strike instantly slays you!",
			ch,NULL,victim,TO_VICT,POS_DEAD);
		 damage(ch,victim,(victim->hit + 1),gsn_mortal_strike,DAM_NONE, TRUE);
		 check_improve(ch,gsn_mortal_strike, TRUE, 1);
		 return;
		}
	  else check_improve(ch,gsn_mortal_strike, FALSE, 3);
	 }

	multi_hit(ch, victim, TYPE_UNDEFINED);
	return;
}



void do_murde(CHAR_DATA *ch, const char *argument)
{
	send_to_char("If you want to MURDER, spell it out.\n\r", ch);
	return;
}



void do_murder(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA *wield;

	one_argument(argument, arg);

	if (arg[0] == '\0') {
		send_to_char("Murder whom?\n\r", ch);
		return;
	}

	if (IS_AFFECTED(ch, AFF_CHARM)
	||  (IS_NPC(ch) && IS_SET(ch->act,ACT_PET)))
		return;

	if ((victim = get_char_room(ch, arg)) == NULL) {
		send_to_char("They aren't here.\n\r", ch);
		return;
	}

	if (victim == ch) {
		send_to_char("Suicide is a mortal sin.\n\r", ch);
		return;
	}

	if (is_safe(ch, victim))
		return;

	if (IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim) {
		act("$N is your beloved master.", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (ch->position == POS_FIGHTING) {
		send_to_char("You do the best you can!\n\r", ch);
		return;
	}

	WAIT_STATE(ch, 1 * PULSE_VIOLENCE);
	if (!can_see(victim, ch))
		do_yell(victim, "Help! I am being attacked by someone!");
	else {
		if (IS_NPC(ch))
			doprintf(do_yell, victim,
				"Help! I am being attacked by %s!",
				ch->short_descr);
		else
			doprintf(do_yell, victim,
				 "Help! I am being attacked by %s!",
				 ch->name);
	}

	if (SKILL_OK(ch, gsn_mortal_strike)
	&&  (wield = get_eq_char(ch,WEAR_WIELD)) != NULL
	&&  wield->level > (victim->level - 5)
	&&  clan_ok(ch,gsn_mortal_strike)) {
		int chance = 1 + get_skill(ch,gsn_mortal_strike) / 30;
		chance += (ch->level - victim->level) / 2;
		if (number_percent() < chance) {
			act_puts("Your flash strike instantly slays $N!",
				 ch,NULL,victim,TO_CHAR,POS_RESTING);
			act_puts("$n flash strike instantly slays $N!",
				 ch,NULL,victim,TO_NOTVICT,POS_RESTING);
			act_puts("$n flash strike instantly slays you!",
				 ch,NULL,victim,TO_VICT,POS_DEAD);
			damage(ch, victim, (victim->hit + 1),gsn_mortal_strike,
			       DAM_NONE, TRUE);
			check_improve(ch, gsn_mortal_strike, TRUE, 1);
			return;
		}
	 	else
			check_improve(ch,gsn_mortal_strike, FALSE, 3);
	}

	multi_hit(ch, victim, TYPE_UNDEFINED);
	return;
}


void do_flee(CHAR_DATA *ch, const char *argument)
{
	ROOM_INDEX_DATA *was_in;
	ROOM_INDEX_DATA *now_in;
	CHAR_DATA *victim;
	int attempt;

	if (RIDDEN(ch))
	{
		send_to_char("You should ask to your rider!\n\r", ch);
		return;
	}

	if (MOUNTED(ch))
		do_dismount(ch,"");

	if ((victim = ch->fighting) == NULL)
	{
		if (ch->position == POS_FIGHTING)
		    ch->position = POS_STANDING;
		send_to_char("You aren't fighting anyone.\n\r", ch);
		return;
	}

	if ((ch->class == CLASS_SAMURAI) && (ch->level >= 10)) {
		 send_to_char("Your honour doesn't let you flee, try dishonoring yourself.\n\r", ch);
		 return;
	}

	was_in = ch->in_room;
	for (attempt = 0; attempt < 6; attempt++) {
		EXIT_DATA *pexit;
		int door;

		door = number_door();
		if ((pexit = was_in->exit[door]) == 0
		||   pexit->u1.to_room == NULL
		|| (IS_SET(pexit->exit_info, EX_CLOSED)
	  && (!IS_AFFECTED(ch, AFF_PASS_DOOR) || IS_SET(pexit->exit_info,EX_NOPASS))
	  &&   !IS_TRUSTED(ch,ANGEL))
		|| (IS_SET(pexit->exit_info , EX_NOFLEE))
		|| (IS_NPC(ch)
		&&   IS_SET(pexit->u1.to_room->room_flags, ROOM_NOMOB)))
		    continue;

		move_char(ch, door, FALSE);
		if ((now_in = ch->in_room) == was_in)
		    continue;

		ch->in_room = was_in;
		act("$n has fled!", ch, NULL, NULL, TO_ROOM);
		ch->in_room = now_in;

		if (!IS_NPC(ch)) {
			char_nputs(YOU_FLED_FROM_COMBAT, ch);
			if (ch->level < LEVEL_HERO) {
				char_nprintf(ch, YOU_LOSE_D_EXPS, 10);
				gain_exp(ch, -10);
			}
		}
		else
			ch->last_fought = NULL;

		stop_fighting(ch, TRUE);
		return;
	}

	send_to_char("PANIC! You couldn't escape!\n\r", ch);
	return;
}



void do_sla(CHAR_DATA *ch, const char *argument)
{
	send_to_char("If you want to SLAY, spell it out.\n\r", ch);
	return;
}



void do_slay(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *victim;
	char arg[MAX_INPUT_LENGTH];

	one_argument(argument, arg);
	if (arg[0] == '\0') {
		send_to_char("Slay whom?\n\r", ch);
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL) {
		send_to_char("They aren't here.\n\r", ch);
		return;
	}

	if (ch == victim) {
		send_to_char("Suicide is a mortal sin.\n\r", ch);
		return;
	}

	if ((!IS_NPC(victim) && victim->level >= get_trust(ch))
	||  IS_IMMORTAL(victim)) {
		send_to_char("You failed.\n\r", ch);
		return;
	}

	act("You slay $M in cold blood!", ch, NULL, victim, TO_CHAR);
	act("$n slays you in cold blood!", ch, NULL, victim, TO_VICT);
	act("$n slays $N in cold blood!", ch, NULL, victim, TO_NOTVICT);
	raw_kill(ch, victim);
	return;
}

/*
 * Check for obj dodge.
 */
bool check_obj_dodge(CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj, int bonus)
{
	int chance;

	if (!IS_AWAKE(victim) || MOUNTED(victim))
		return FALSE;

	if (IS_NPC(victim))
		 chance  = UMIN(30, victim->level);
	else {
		  chance  = get_skill(victim,gsn_dodge) / 2;
		  /* chance for high dex. */
		  chance += 2 * (get_curr_stat(victim,STAT_DEX) - 20);
		 if (victim->class == CLASS_WARRIOR || victim->class == CLASS_SAMURAI)
		    chance *= 1.2;
		if (victim->class == CLASS_THIEF || victim->class ==CLASS_NINJA)
		    chance *= 1.1;
		 }

	chance -= (bonus - 90);
	chance /= 2;
	if (number_percent() >= chance &&
		 (IS_NPC(victim) || victim->clan != CLAN_BATTLE))
		return FALSE;

	act("You dodge $p that had been shot to you.",ch,obj,victim,TO_VICT);
	act("$N dodges $p that had been shot to $M.",ch,obj,victim,TO_CHAR);
	act("$n dodges $p that had been shot to $m.",victim,obj,ch,TO_NOTVICT);
	obj_to_room(obj,victim->in_room);
	check_improve(victim,gsn_dodge,TRUE,6);

	return TRUE;
}



void do_dishonor(CHAR_DATA *ch, const char *argument)
{
	ROOM_INDEX_DATA *was_in;
	ROOM_INDEX_DATA *now_in;
	CHAR_DATA *gch;
	int attempt,level = 0;

	if (RIDDEN(ch)) {
		send_to_char("You should ask to your rider!\n\r", ch);
		return;
	}

	if ((ch->class != CLASS_SAMURAI) || (ch->level <10)) {
		send_to_char("Which honor?.\n\r", ch);
		return;
	}

	if (ch->fighting == NULL) {
		if (ch->position == POS_FIGHTING)
			ch->position = POS_STANDING;
		send_to_char("You aren't fighting anyone.\n\r", ch);
		return;
	}

	for (gch = char_list; gch != NULL; gch = gch->next)
		  if (is_same_group(gch, ch->fighting) || gch->fighting == ch)
			level += gch->level;

	if ((ch->fighting->level - ch->level) < 5 && ch->level > (level / 3)) {
		 send_to_char("Your fighting doesn't worth "
			      "to dishonor yourself.\n\r", ch);
		 return;
	}

	was_in = ch->in_room;
	for (attempt = 0; attempt < 6; attempt++) {
		EXIT_DATA *pexit;
		int door;

		door = number_door();
		if ((pexit = was_in->exit[door]) == 0
		||  pexit->u1.to_room == NULL
		||  (IS_SET(pexit->exit_info, EX_CLOSED) &&
		     (!IS_AFFECTED(ch, AFF_PASS_DOOR) ||
		      IS_SET(pexit->exit_info,EX_NOPASS)) &&
		     !IS_TRUSTED(ch,ANGEL))
		|| IS_SET(pexit->exit_info, EX_NOFLEE)
		|| (IS_NPC(ch) &&
		    IS_SET(pexit->u1.to_room->room_flags, ROOM_NOMOB)))
			continue;

		move_char(ch, door, FALSE);
		if ((now_in = ch->in_room) == was_in)
			continue;

		ch->in_room = was_in;
		act("$n has dishonored $mself!", ch, NULL, NULL, TO_ROOM);
		ch->in_room = now_in;

		if (!IS_NPC(ch)) {
			send_to_char("You dishonored yourself "
				     "and flee from combat.\n\r",ch);
			if (ch->level < LEVEL_HERO) {
				char_nprintf(ch, YOU_LOSE_D_EXPS, ch->level);
				gain_exp(ch, -(ch->level));
			}
		}
		else
			ch->last_fought = NULL;

		stop_fighting(ch, TRUE);
		if (MOUNTED(ch))
			do_dismount(ch,"");

		return;
	}

	send_to_char("PANIC! You couldn't escape!\n\r", ch);
	return;
}


void do_surrender(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *mob;
	if ((mob = ch->fighting) == NULL) {
		send_to_char("But you're not fighting!\n\r", ch);
		return;
	}
	act("You surrender to $N!", ch, NULL, mob, TO_CHAR);
	act("$n surrenders to you!", ch, NULL, mob, TO_VICT);
	act("$n tries to surrender to $N!", ch, NULL, mob, TO_NOTVICT);
	stop_fighting(ch, TRUE);
 
	if (!IS_NPC(ch) && IS_NPC(mob) 
	&&  (!HAS_TRIGGER(mob, TRIG_SURR) ||
	     !mp_percent_trigger(mob, ch, NULL, NULL, TRIG_SURR))) {
		act("$N seems to ignore your cowardly act!",
		    ch, NULL, mob, TO_CHAR);
		multi_hit(mob, ch, TYPE_UNDEFINED);
	}
}
