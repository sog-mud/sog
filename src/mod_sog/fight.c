/*
 * $Id: fight.c,v 1.142 1999-02-20 16:44:22 fjoe Exp $
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

#if !defined (WIN32)
#	include <unistd.h>
#endif

#include "merc.h"
#include "hometown.h"
#include "quest.h"
#include "fight.h"
#include "rating.h"
#include "update.h"
#include "mob_prog.h"
#include "obj_prog.h"

DECLARE_DO_FUN(do_quit_count	);
DECLARE_DO_FUN(do_crush		);
DECLARE_DO_FUN(do_emote		);
DECLARE_DO_FUN(do_dismount	);
DECLARE_DO_FUN(do_bash		);
DECLARE_DO_FUN(do_berserk	);
DECLARE_DO_FUN(do_disarm	);
DECLARE_DO_FUN(do_kick		);
DECLARE_DO_FUN(do_dirt		);
DECLARE_DO_FUN(do_trip		);
DECLARE_DO_FUN(do_tail		);
DECLARE_DO_FUN(do_look_in	);
DECLARE_DO_FUN(do_get		);
DECLARE_DO_FUN(do_sacrifice	);
DECLARE_DO_FUN(do_visible	);
DECLARE_DO_FUN(do_recall	);
DECLARE_DO_FUN(do_flee		);
DECLARE_DO_FUN(do_clan		);

/*
 * Local functions.
 */
void	check_assist		(CHAR_DATA *ch, CHAR_DATA *victim);
bool	check_dodge		(CHAR_DATA *ch, CHAR_DATA *victim);
bool	check_parry		(CHAR_DATA *ch, CHAR_DATA *victim, int loc);
bool	check_block		(CHAR_DATA *ch, CHAR_DATA *victim, int loc);
bool	check_blink		(CHAR_DATA *ch, CHAR_DATA *victim);
void	dam_message		(CHAR_DATA *ch, CHAR_DATA *victim, int dam,
				 int dt, bool immune, int dam_type);
void	death_cry		(CHAR_DATA *ch);
void	death_cry_org		(CHAR_DATA *ch, int part);
void	group_gain		(CHAR_DATA *ch, CHAR_DATA *victim);
int	xp_compute		(CHAR_DATA *gch, CHAR_DATA *victim,
				 int total_levels, int members);
bool	is_safe 		(CHAR_DATA *ch, CHAR_DATA *victim);

void	make_corpse		(CHAR_DATA *ch);
void	one_hit 		(CHAR_DATA *ch, CHAR_DATA *victim, int dt,
				 int loc);
void	mob_hit 		(CHAR_DATA *ch, CHAR_DATA *victim, int dt);
void	set_fighting		(CHAR_DATA *ch, CHAR_DATA *victim);
void	disarm			(CHAR_DATA *ch, CHAR_DATA *victim,
				 int disarm_second);
int	critical_strike		(CHAR_DATA *ch, CHAR_DATA *victim, int dam);
void	check_eq_damage		(CHAR_DATA *ch, CHAR_DATA *victim, int loc);
void	check_shield_damage	(CHAR_DATA *ch, CHAR_DATA *victim, int loc);
void	check_weapon_damage	(CHAR_DATA *ch, CHAR_DATA *victim, int loc);

void	handle_death		(CHAR_DATA *ch, CHAR_DATA *victim);

/*
 * Gets all money from the corpse.
 */
void get_gold_corpse(CHAR_DATA *ch, OBJ_DATA *corpse)
{
	OBJ_DATA *tmp, *tmp_next;
	for (tmp = corpse->contains; tmp; tmp = tmp_next) {
		tmp_next = tmp->next_content;
		if (tmp->pIndexData->item_type == ITEM_MONEY)
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

	for (ch = char_list; ch != NULL; ch = ch_next) {
		ch_next = ch->next;

		/* decrement the wait */
		if (ch->desc == NULL)
			ch->wait = UMAX(0, ch->wait - PULSE_VIOLENCE);

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

		for (obj = ch->carrying; obj; obj = obj_next) {
			obj_next = obj->next_content;
			if (ch->fighting == NULL)
				break;
			oprog_call(OPROG_FIGHT, obj, ch, NULL);
		}

		if ((victim = ch->fighting) == NULL)
			continue;

		/*
		 * Fun for the whole family!
		 */
		check_assist(ch, victim);
		if (IS_NPC(ch)) {
			if (HAS_TRIGGER(ch, TRIG_FIGHT))
				mp_percent_trigger(ch, victim, NULL, NULL,
						   TRIG_FIGHT);
			if (HAS_TRIGGER(ch, TRIG_HPCNT))
				mp_hprct_trigger(ch, victim);
		}
	}
}

/* for auto assisting */
void check_assist(CHAR_DATA *ch,CHAR_DATA *victim)
{
	CHAR_DATA *rch, *rch_next;

	for (rch = ch->in_room->people; rch != NULL; rch = rch_next) {
		rch_next = rch->next_in_room;

		if (IS_AWAKE(rch) && rch->fighting == NULL) {
		    /* quick check for ASSIST_PLAYER */
		    if (!IS_NPC(ch) && IS_NPC(rch)
		    &&  IS_SET(rch->pIndexData->off_flags, ASSIST_PLAYERS)
		    &&	rch->level + 6 > victim->level) {
			do_emote(rch, "screams and attacks!");
			multi_hit(rch,victim,TYPE_UNDEFINED);
			continue;
		    }

		    /* PCs next */
		    if (!IS_NPC(rch) || IS_AFFECTED(rch, AFF_CHARM)) {
			if (((!IS_NPC(rch) &&
			      IS_SET(rch->plr_flags, PLR_AUTOASSIST)) ||
			     IS_AFFECTED(rch, AFF_CHARM))
			&&  is_same_group(ch,rch)
			&&  !is_safe_nomessage(rch, victim))
			    multi_hit (rch,victim,TYPE_UNDEFINED);
				continue;
		    }

		    if (!IS_NPC(ch) && RIDDEN(rch) == ch)
		    {
			multi_hit(rch,victim,TYPE_UNDEFINED);
			continue;
		    }

		    /* now check the NPC cases */

		    if (IS_NPC(ch)) {
			if ((IS_NPC(rch) && IS_SET(rch->pIndexData->off_flags,ASSIST_ALL))
			||   (IS_NPC(rch) && rch->race == ch->race
			   && IS_SET(rch->pIndexData->off_flags,ASSIST_RACE))
			||   (IS_NPC(rch) && IS_SET(rch->pIndexData->off_flags,ASSIST_ALIGN)
			   &&	((IS_GOOD(rch)	  && IS_GOOD(ch))
			     ||  (IS_EVIL(rch)	  && IS_EVIL(ch))
			     ||  (IS_NEUTRAL(rch) && IS_NEUTRAL(ch))))
			||   (rch->pIndexData == ch->pIndexData
			   && IS_SET(rch->pIndexData->off_flags,ASSIST_VNUM))) {
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

	/* no attacks for stunnies -- just a check */
	if (ch->position < POS_RESTING)
		return;

#if 0
	/* become CRIMINAL in Law rooms */
	if (!IS_NPC(ch) && !IS_NPC(victim)
	&& IS_SET(ch->in_room->room_flags, ROOM_LAW)
	&& !IS_SET(victim->plr_flags, PLR_WANTED)
	&& !IS_SET(ch->plr_flags, PLR_WANTED)) {
		char_puts("This room is under supervision of the law! "
			  "Now you're {RCRIMINAL{x!\n", ch);
		SET_BIT(ch->plr_flags, PLR_WANTED);		
	}
#endif

	/* ridden's adjustment */
	if (RIDDEN(victim) && !IS_NPC(victim->mount)) {
		if (victim->mount->fighting == NULL
		|| victim->mount->fighting == ch)
			victim = victim->mount;
		else
			do_dismount(victim->mount, str_empty);
	}

	if (IS_AFFECTED(ch,AFF_WEAK_STUN)) {
		act_puts("You are too stunned to respond $N's attack.",
			 ch, NULL, victim, TO_CHAR, POS_FIGHTING);
		act_puts("$n is too stunned to respond your attack.",
			 ch, NULL, victim, TO_VICT, POS_FIGHTING);
		REMOVE_BIT(ch->affected_by, AFF_WEAK_STUN);
		return;
	}

	if (IS_AFFECTED(ch,AFF_STUN)) {
		act_puts("You are too stunned to respond $N's attack.",
			 ch, NULL, victim, TO_CHAR, POS_FIGHTING);
		act_puts("$n is too stunned to respond your attack.",
			 ch, NULL, victim, TO_VICT, POS_FIGHTING);
		act_puts("$n seems to be stunned.",
			 ch, NULL, victim, TO_NOTVICT, POS_FIGHTING);
		affect_bit_strip(ch, TO_AFFECTS, AFF_STUN);
		SET_BIT(ch->affected_by, AFF_WEAK_STUN);
		return;
	}

	if (IS_NPC(ch)) {
		mob_hit(ch, victim, dt);
		return;
	}

	one_hit(ch, victim, dt, WEAR_WIELD);

	if (ch->fighting != victim)
		return;

	if ((chance = get_skill(ch, gsn_area_attack))
	&&  number_percent() < chance) {
		int count = 0, max_count;
		CHAR_DATA *vch, *vch_next;

		check_improve(ch, gsn_area_attack, TRUE, 6);

		if (ch->level < 70)
			max_count = 1;
		else if (ch->level < 80)
			max_count = 2;
		else if (ch->level < 90)
			max_count = 3;
		else
			max_count = 4;

		for (vch = ch->in_room->people; vch; vch = vch_next) {
			vch_next = vch->next_in_room;
			if (vch != victim && vch->fighting == ch) {
				one_hit(ch, vch, dt, WEAR_WIELD);
				if (++count == max_count)
					break;
			}
		}
	}

	if (IS_AFFECTED(ch, AFF_HASTE))
		one_hit(ch, victim, dt, WEAR_WIELD);

	if (ch->fighting != victim || dt == gsn_backstab || dt == gsn_cleave
	|| dt == gsn_ambush || dt == gsn_dual_backstab || dt == gsn_circle
	|| dt == gsn_assassinate || dt == gsn_vampiric_bite)
		return;

	chance = get_skill(ch, gsn_second_attack) / 2;
	if (number_percent() < chance) {
		one_hit(ch, victim, dt, WEAR_WIELD);
		check_improve(ch, gsn_second_attack, TRUE, 5);
		if (ch->fighting != victim)
			return;
	}

	chance = get_skill(ch,gsn_third_attack)/3;
	if (number_percent() < chance) {
		one_hit(ch, victim, dt, WEAR_WIELD);
		check_improve(ch, gsn_third_attack, TRUE, 6);
		if (ch->fighting != victim)
			return;
	}


	chance = get_skill(ch,gsn_fourth_attack)/6;
	if (number_percent() < chance) {
		one_hit(ch, victim, dt, WEAR_WIELD);
		check_improve(ch, gsn_fourth_attack, TRUE, 7);
		if (ch->fighting != victim)
			return;
	}

	chance = get_skill(ch,gsn_fifth_attack)/8;
	if (number_percent() < chance) {
		one_hit(ch, victim, dt, WEAR_WIELD);
		check_improve(ch,gsn_fifth_attack,TRUE,8);
		if (ch->fighting != victim)
		    return;
	}

	chance = get_skill(ch, gsn_second_weapon) / 2;
	if (number_percent() < chance)
		if (get_eq_char(ch, WEAR_SECOND_WIELD)) {
			one_hit(ch, victim, dt, WEAR_SECOND_WIELD);
			check_improve(ch, gsn_second_weapon, TRUE, 2);
			if (ch->fighting != victim)
				return;
		}

	chance = get_skill(ch,gsn_secondary_attack) / 8;
	if (number_percent() < chance)
		if (get_eq_char(ch, WEAR_SECOND_WIELD)) {
			one_hit(ch, victim, dt, WEAR_SECOND_WIELD);
			check_improve(ch, gsn_secondary_attack, TRUE, 2);
			if (ch->fighting != victim)
				return;
		}
}

/* procedure for all mobile attacks */
void mob_hit(CHAR_DATA *ch, CHAR_DATA *victim, int dt)
{
	CHAR_DATA *vch, *vch_next;
	flag64_t act = ch->pIndexData->act;
	flag64_t off = ch->pIndexData->off_flags;

	/* no attack by ridden mobiles except spec_casts */
	if (RIDDEN(ch)) {
		 if (ch->fighting != victim)
			set_fighting(ch, victim);
		 return;
	}

	one_hit(ch, victim, dt, WEAR_WIELD);

	if (ch->fighting != victim)
		return;

	/* Area attack -- BALLS nasty! */

	if (IS_SET(off, OFF_AREA_ATTACK)) {
		for (vch = ch->in_room->people; vch != NULL; vch = vch_next) {
			vch_next = vch->next_in_room;
			if ((vch != victim && vch->fighting == ch))
				one_hit(ch, vch, dt, WEAR_WIELD);
		}
	}

	if (IS_AFFECTED(ch, AFF_HASTE) || IS_SET(off, OFF_FAST))
		one_hit(ch, victim, dt, WEAR_WIELD);

	if (ch->fighting != victim || dt == gsn_backstab || dt == gsn_circle ||
		dt == gsn_dual_backstab || dt == gsn_cleave || dt == gsn_ambush
			|| dt == gsn_vampiric_bite)
		return;

	if (number_percent() < get_skill(ch, gsn_second_attack) / 2) {
		one_hit(ch, victim, dt, WEAR_WIELD);
		if (ch->fighting != victim)
			return;
	}

	if (number_percent() < get_skill(ch, gsn_third_attack) / 4) {
		one_hit(ch, victim, dt, WEAR_WIELD);
		if (ch->fighting != victim)
			return;
	}

	if (number_percent() < get_skill(ch, gsn_fourth_attack) / 6) {
		one_hit(ch, victim, dt, WEAR_WIELD);
		if (ch->fighting != victim)
			return;
	}

	/* PC waits */

	if (ch->wait > 0)
		return;

#if 0
	switch (number_range(0, 2)) {
	case 1:
		if (IS_SET(act, ACT_MAGE)) {
			mob_cast_mage(ch, victim);
			return;
		}
		break;
	case 2:
		if (IS_SET(act, ACT_CLERIC)) {
			mob_cast_cleric(ch, victim);
			return;
		}
		break;
	}
#endif

	/* now for the skills */

	switch (number_range(0, 7)) {
	case 0:
		if (IS_SET(off, OFF_BASH))
			do_bash(ch, str_empty);
		break;

	case 1:
		if (IS_SET(off, OFF_BERSERK)
		&&  !IS_AFFECTED(ch, AFF_BERSERK))
			do_berserk(ch, str_empty);
		break;


	case 2:
		if (IS_SET(off, OFF_DISARM)
		||  IS_SET(act, ACT_WARRIOR | ACT_THIEF)) {
			if (number_range(0, 1)
			&&  get_eq_char(victim, WEAR_SECOND_WIELD))
				do_disarm(ch, "second");
			else if (get_eq_char(victim, WEAR_WIELD))
				do_disarm(ch, str_empty);
		}
		break;

	case 3:
		if (IS_SET(off, OFF_KICK))
			do_kick(ch, str_empty);
		break;

	case 4:
		if (IS_SET(off, OFF_DIRT_KICK))
			do_dirt(ch, str_empty);
		break;

	case 5:
		if (IS_SET(off, OFF_TAIL))
			do_tail(ch, str_empty);
		break;

	case 6:
		if (IS_SET(off, OFF_TRIP))
			do_trip(ch, str_empty);
		break;
	case 7:
		if (IS_SET(off, OFF_CRUSH))
			do_crush(ch, str_empty);
		break;
	}
}

int get_dam_type(CHAR_DATA *ch, OBJ_DATA *wield, int *dt)
{
	int dam_type;

	if (*dt == TYPE_UNDEFINED) {
		*dt = TYPE_HIT;
		if (wield &&  wield->pIndexData->item_type == ITEM_WEAPON)
			*dt += wield->value[3];
		else
			*dt += ch->dam_type;
	}

	if (*dt < TYPE_HIT)
		if (wield)
			dam_type = attack_table[wield->value[3]].damage;
		else
			dam_type = attack_table[ch->dam_type].damage;
	else
		dam_type = attack_table[*dt - TYPE_HIT].damage;

	if (dam_type == TYPE_UNDEFINED)
		dam_type = DAM_BASH;

	return dam_type;
}

/*
 * Hit one guy once.
 */
void one_hit(CHAR_DATA *ch, CHAR_DATA *victim, int dt, int loc)
{
	OBJ_DATA *wield;
	int victim_ac;
	int thac0;
	int thac0_00;
	int thac0_32;
	int dam;
	int diceroll;
	int sn, sk, sk2;
	int dam_type;
	bool counter;
	bool result;
	int sercount;
	int dam_flags;

	sn = -1;
	counter = FALSE;

	/* just in case */
	if (victim == ch || ch == NULL || victim == NULL)
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
	wield = get_eq_char(ch, loc);
	dam_flags = DAMF_SHOW;
	if (loc == WEAR_SECOND_WIELD)
		dam_flags |= DAMF_SECOND;
	dam_type = get_dam_type(ch, wield, &dt);

	/* get the weapon skill */
	sn = get_weapon_sn(wield);
	sk = 20 + get_weapon_skill(ch, sn);

	/*
	 * Calculate to-hit-armor-class-0 versus armor.
	 */
	if (IS_NPC(ch)) {
		flag64_t act = ch->pIndexData->act;

		thac0_00 = 20;
		thac0_32 = -4;	 /* as good as a thief */
		if (IS_SET(act, ACT_WARRIOR))
			thac0_32 = -10;
		else if (IS_SET(act, ACT_THIEF))
			thac0_32 = -4;
		else if (IS_SET(act, ACT_CLERIC))
			thac0_32 = 2;
		else if (IS_SET(act, ACT_MAGE))
			thac0_32 = 6;
	}
	else {
		CLASS_DATA *cl;

		if ((cl = class_lookup(ch->class)) == NULL)
			return;

		thac0_00 = cl->thac0_00;
		thac0_32 = cl->thac0_32;
	}

	thac0  = interpolate(ch->level, thac0_00, thac0_32);

	if (thac0 < 0)
		thac0 = thac0/2;

	if (thac0 < -5)
		thac0 = -5 + (thac0 + 5) / 2;

	thac0 -= GET_HITROLL(ch) * sk / 100;
	thac0 += 5 * (100 - sk) / 100;

	if (dt == gsn_backstab)
		thac0 -= 10 * (100 - get_skill(ch, gsn_backstab));
	else if (dt == gsn_dual_backstab)
		thac0 -= 10 * (100 - get_skill(ch, gsn_dual_backstab));
	else if (dt == gsn_cleave)
		thac0 -= 10 * (100 - get_skill(ch, gsn_cleave));
	else if (dt == gsn_ambush)
		thac0 -= 10 * (100 - get_skill(ch, gsn_ambush));
	else if (dt == gsn_vampiric_bite)
		thac0 -= 10 * (100 - get_skill(ch, gsn_vampiric_bite));
	else if (dt == gsn_charge)
		thac0 -= 10 * (100 - get_skill(ch, gsn_charge));

	switch(dam_type) {
	case DAM_PIERCE:victim_ac = GET_AC(victim,AC_PIERCE)/10; break;
	case DAM_BASH:  victim_ac = GET_AC(victim,AC_BASH)/10; 	 break;
	case DAM_SLASH: victim_ac = GET_AC(victim,AC_SLASH)/10;	 break;
	default:	victim_ac = GET_AC(victim,AC_EXOTIC)/10; break;
	}

	if (victim_ac < -15)
		victim_ac = (victim_ac + 15) / 5 - 15;

	if (get_skill(victim, gsn_armor_use) > 70) {
		check_improve(victim, gsn_armor_use, TRUE, 8);
		victim_ac -= (victim->level) / 2;
	}

	if (!can_see(ch, victim)) {
		 if ((sk2 = get_skill(ch, gsn_blind_fighting))
		 &&  number_percent() < sk2)
			check_improve(ch,gsn_blind_fighting,TRUE,16);
		 else
			victim_ac -= 4;
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
	|| (diceroll != 19 && diceroll < thac0 - victim_ac)) {
		/* Miss. */
		damage(ch, victim, 0, dt, dam_type, dam_flags);
		tail_chain();
		return;
	}

	/*
	 * Hit.
	 * Calc damage.
	 */

	if (IS_NPC(ch) && wield == NULL)
		dam = dice(ch->damage[DICE_NUMBER], ch->damage[DICE_TYPE]);
	else {
		if (sn != -1)
			check_improve(ch, sn, TRUE, 5);
		if (wield != NULL) {
			dam = dice(wield->value[1],
				   wield->value[2]) * sk / 100;

/* no shield = more */
			if (get_eq_char(ch, WEAR_SHIELD) == NULL)
				dam = dam * 21/20;

/* sharpness! */
			if (IS_WEAPON_STAT(wield, WEAPON_SHARP)) {
				int percent;

				if ((percent = number_percent()) <= (sk / 8))
					dam = 2 * dam + (dam * 2 * percent / 100);
			}

/* holy weapon */
			if (IS_WEAPON_STAT(wield, WEAPON_HOLY)
			&&  IS_GOOD(ch) && IS_EVIL(victim)
			&&  number_percent() < 30) {
				act("$n's flesh is burned with the holy aura of $p.", victim, wield, NULL, TO_ROOM);
				act_puts("Your flesh is burned with the holy aura of $p.", victim, wield, NULL, TO_CHAR, POS_DEAD);
				dam += dam * 120 / 100;
			}
		}
		else {
#if 0
			int d;
#endif

			dam = number_range(1 + 4 * sk / 100,
					   2 * ch->level / 3 * sk / 100);
#if 0
/* we do not have 'master hand' skill */
			if ((sk2 = get_skill(ch, gsn_master_hand))
			&&  (d = number_percent()) <= sk2) {
				check_improve(ch, gsn_master_hand, TRUE, 6);
				dam += dam * 110 /100;
				if (d < 20) {
					SET_BIT(victim->affected_by,
						AFF_WEAK_STUN);
					act_puts("You hit $N with a stunning "
						 "force!", ch, NULL, victim,
						 TO_CHAR, POS_DEAD);
					act_puts("$n hit you with a stunning "
						 "force!", ch, NULL, victim,
						 TO_VICT, POS_DEAD);
					act_puts("$n hits $N with a stunning "
						 "force!", ch, NULL, victim,
						 TO_NOTVICT, POS_DEAD);
					check_improve(ch, gsn_master_hand,
						      TRUE, 6);
				}
			}
#endif
		}
	}

	/*
	 * Bonuses.
	 */
	if ((sk2 = get_skill(ch, gsn_enhanced_damage))
	&&  (diceroll = number_percent()) <= sk2) {
		int div;
		check_improve(ch, gsn_enhanced_damage, TRUE, 6);
		div = (ch->class == CLASS_WARRIOR) ?
		      100 : (ch->class == CLASS_CLERIC) ? 130 : 114;
		dam += dam * diceroll/div;
	}

	if (sn == gsn_sword
	&&  (sk2 = get_skill(ch, gsn_mastering_sword))
	&&  number_percent() <= sk2) {
		OBJ_DATA *katana;

		check_improve(ch, gsn_mastering_sword, TRUE, 6);
		dam += dam * 110 /100;

		if (((katana = get_eq_char(ch,WEAR_WIELD)) ||
		     (katana = get_eq_char(ch, WEAR_SECOND_WIELD)))
		&&  IS_WEAPON_STAT(katana, WEAPON_KATANA)
		&&  strstr(mlstr_mval(katana->ed->description), ch->name)) {
			AFFECT_DATA *paf;

			if ((katana->cost = ++katana->cost % 250) == 0
			&&  (paf = affect_find(katana->affected, gsn_katana))) {
				int old_mod = paf->modifier;
				paf->modifier = UMIN(paf->modifier+1,
						     ch->level / 3);
				if (paf->next != NULL) {
					paf->next->modifier = paf->modifier;
					ch->hitroll += paf->modifier - old_mod;
					if (paf->next->next) {
						paf->next->next->modifier =
							paf->modifier;
						ch->damroll +=
							paf->modifier - old_mod;
					}
				}
				act("$n's katana glows blue.\n",
				    ch, NULL, NULL, TO_ROOM);
				char_puts("Your katana glows blue.\n",ch);
			}
		}
	}

	if (!IS_AWAKE(victim))
		dam *= 2;
	else if (victim->position < POS_FIGHTING)
		dam = dam * 3 / 2;

	sercount = number_percent();
	if (dt == gsn_backstab || dt == gsn_vampiric_bite)
		sercount += 40;
	if (!IS_IMMORTAL(ch) && IS_PUMPED(ch))
		sercount += 10;
	sercount *= 2;
	if (victim->fighting == NULL && !IS_NPC(victim)
	&&  !is_safe_nomessage(victim, ch)
	&&  !is_safe_nomessage(ch,victim)
	&&  (victim->position == POS_SITTING ||
	     victim->position == POS_STANDING)
	&&  dt != gsn_assassinate
	&&  (sercount <= get_skill(victim, gsn_counter))) {
		counter = TRUE;
		check_improve(victim,gsn_counter,TRUE,1);
		act("$N turns your attack against you!",
		    ch, NULL, victim, TO_CHAR);
		act("You turn $n's attack against $m!",
		    ch, NULL, victim, TO_VICT);
		act("$N turns $n's attack against $m!",
		    ch, NULL, victim, TO_NOTVICT);
		ch->fighting = victim;
	}
	else if (!victim->fighting)
		check_improve(victim, gsn_counter, FALSE, 1);

	if (dt == gsn_backstab && (IS_NPC(ch) || wield))
		dam = ch->level / 10 * dam + ch->level;
	else if (dt == gsn_dual_backstab && (IS_NPC(ch) || wield))
		dam = ch->level / 14 * dam + ch->level;
	else if (dt == gsn_circle)
		dam = (ch->level/40 + 1) * dam + ch->level;
	else if (dt == gsn_vampiric_bite && is_affected(ch, gsn_vampire))
		dam = (ch->level/20 + 1) * dam + ch->level;
	else if (dt == gsn_cleave && wield != NULL) {
		if (number_percent() <
				(URANGE(4, 5+ch->level-victim->level, 10)
				+ (wield->value[0]==WEAPON_AXE) ? 2:0 +
				(get_curr_stat(ch,STAT_STR)-21)/2)
		&&  !counter && !IS_IMMORTAL(victim)) {
			act_puts("Your cleave chops $N IN HALF!",
				 ch, NULL, victim, TO_CHAR, POS_RESTING);
			act_puts("$n's cleave chops you IN HALF!",
				 ch, NULL, victim, TO_VICT, POS_RESTING);
			act_puts("$n's cleave chops $N IN HALF!",
				 ch, NULL, victim, TO_NOTVICT, POS_RESTING);
			char_puts("You have been KILLED!\n", victim);
			act("$n is DEAD!", victim, NULL, NULL, TO_ROOM);
			WAIT_STATE(ch, 2);
			victim->position = POS_DEAD;
			handle_death(ch, victim);
			return;
		}
		else
			dam = (dam * 2 + ch->level);
	}

	if (dt == gsn_assassinate) {
		if (number_percent() <=
				URANGE(10, 20+(ch->level-victim->level)*2, 50)
		&& !counter && !IS_IMMORTAL(victim)) {
			act_puts("You {R+++ASSASSINATE+++{x $N!",
				 ch, NULL, victim, TO_CHAR, POS_RESTING);
			act_puts("$n {R+++ASSASSINATES+++{x $N!",
				 ch, NULL, victim, TO_NOTVICT, POS_RESTING);
			act_puts("$n {R+++ASSASSINATES+++{x you!",
				 ch, NULL, victim, TO_VICT, POS_DEAD);
			char_puts("You have been KILLED!\n", victim);
			act("$n is DEAD!", victim, NULL, victim, TO_ROOM);
			check_improve(ch, gsn_assassinate, TRUE, 1);
			victim->position = POS_DEAD;
			handle_death(ch, victim);
			return;
		}
		else {
			check_improve(ch, gsn_assassinate, FALSE, 1);
			dam *= 2;
		}
	}
	if (dt == gsn_charge)
		dam *= ch->level/15;

	dam += GET_DAMROLL(ch) * UMIN(100, sk) / 100;

	if (dt == gsn_ambush)
		dam *= 3;

	if ((sk2 = get_skill(ch, gsn_deathblow)) > 1) {
		if (number_percent() <  (sk2/8)) {
			act("You deliver a blow of deadly force!",
			    ch, NULL, NULL, TO_CHAR);
			act("$n delivers a blow of deadly force!",
			    ch, NULL, NULL, TO_ROOM);
			dam = ch->level*dam/20;
			check_improve(ch, gsn_deathblow, TRUE, 1);
		}
		else
			check_improve(ch, gsn_deathblow, FALSE, 3);
	}

	if (dam <= 0)
		dam = 1;

	if (counter) {
		result = damage(ch, ch, 2*dam, dt, dam_type, DAMF_SHOW);
		multi_hit(victim, ch, TYPE_UNDEFINED);
	}
	else
		result = damage(ch, victim, dam, dt, dam_type, dam_flags);

	/* vampiric bite gives hp to ch from victim */
	if (dt == gsn_vampiric_bite) {
		int hit_ga = UMIN((dam / 2), victim->max_hit);

		ch->hit += hit_ga;
		ch->hit  = UMIN(ch->hit, ch->max_hit);
		update_pos(ch);
		char_puts("Your health increases as you suck "
			  "your victim's blood.\n", ch);
	}

	/* but do we have a funky weapon? */
	if (result && wield != NULL && ch->fighting == victim) {
		int dam;

		if (IS_WEAPON_STAT(wield, WEAPON_VORPAL)) {
			int chance;

			chance = get_skill(ch, get_weapon_sn(wield)) +
				 get_curr_stat(ch, STAT_STR) * 4;

			if (chance > number_range(1, 200000)) {
				act("$p makes an huge arc in the air, "
				    "chopping $n's head OFF!",
				     victim, wield, NULL, TO_ROOM);
				act("$p whistles in the air, "
				    "chopping your head OFF!",
				    victim, wield, NULL, TO_CHAR);
				act("$n is DEAD!", victim, NULL, NULL, TO_ROOM);
				char_puts("You have been KILLED!\n", victim);
				victim->position = POS_DEAD;
				handle_death(ch, victim);
				return;
			}
		}

		if (IS_WEAPON_STAT(wield, WEAPON_POISON)) {
			int level;
			AFFECT_DATA *poison, af;

			if ((poison = affect_find(wield->affected,
							gsn_poison)) == NULL)
				level = wield->level;
			else
				level = poison->level;

			if (!saves_spell(level / 2,victim, DAM_POISON)) {
				char_puts("You feel poison coursing "
					  "through your veins.", victim);
				act("$n is poisoned by the venom on $p.",
				    victim, wield, NULL, TO_ROOM);

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
			if (poison != NULL) {
				poison->level = UMAX(0,poison->level - 2);
				poison->duration = UMAX(0,poison->duration - 1);
				if (poison->level == 0
				||  poison->duration == 0)
					act("The poison on $p has worn off.",
					    ch, wield, NULL, TO_CHAR);
			}
		}

		if (IS_WEAPON_STAT(wield, WEAPON_VAMPIRIC)) {
			dam = number_range(1, wield->level / 5 + 1);
			act("$p draws life from $n.",
			    victim, wield, NULL, TO_ROOM);
			act("You feel $p drawing your life away.",
			    victim, wield, NULL, TO_CHAR);
			damage(ch, victim, dam, 0, DAM_NEGATIVE, DAMF_NONE);
			ch->hit += dam/2;
		}

		if (IS_WEAPON_STAT(wield, WEAPON_FLAMING)) {
			dam = number_range(1,wield->level / 4 + 1);
			act("$n is burned by $p.", victim, wield, NULL, TO_ROOM);
			act("$p sears your flesh.",
			    victim, wield, NULL, TO_CHAR);
			fire_effect((void *) victim, wield->level/2, dam,
				    TARGET_CHAR);
			damage(ch, victim, dam, 0, DAM_FIRE, DAMF_NONE);
		}

		if (IS_WEAPON_STAT(wield, WEAPON_FROST)) {
			dam = number_range(1,wield->level / 6 + 2);
			act("$p freezes $n.", victim, wield, NULL, TO_ROOM);
			act("The cold touch of $p surrounds you with ice.",
			    victim, wield, NULL, TO_CHAR);
			cold_effect(victim, wield->level/2, dam, TARGET_CHAR);
			damage(ch, victim, dam, 0, DAM_COLD, DAMF_NONE);
		}

		if (IS_WEAPON_STAT(wield, WEAPON_SHOCKING)) {
			dam = number_range(1, wield->level/5 + 2);
			act("$n is struck by lightning from $p.",
			    victim, wield, NULL, TO_ROOM);
			act("You are shocked by $p.",
			    victim, wield, NULL, TO_CHAR);
			shock_effect(victim, wield->level/2, dam, TARGET_CHAR);
			damage(ch, victim, dam, 0, DAM_LIGHTNING, DAMF_NONE);
		}
	}

	tail_chain();
}

/*
 * handle_death - called from `damage' if `ch' has killed `victim'
 */
void handle_death(CHAR_DATA *ch, CHAR_DATA *victim)
{
	bool vnpc = IS_NPC(victim);
	ROOM_INDEX_DATA *vroom = victim->in_room;
	bool is_duel = !IS_NPC(victim) 
		&& (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM)) 
		&& IS_SET(victim->in_room->room_flags, ROOM_BATTLE_ARENA);
	OBJ_DATA *corpse;
	CLASS_DATA *cl;

	group_gain(ch, victim);

	/*
	 * Death trigger
	 */
	if (vnpc && HAS_TRIGGER(victim, TRIG_DEATH)) {
		victim->position = POS_STANDING;
		mp_percent_trigger(victim, ch, NULL, NULL, TRIG_DEATH);
	}

	raw_kill(ch, victim);

	/* RT new auto commands */
	if (!IS_NPC(ch) && vnpc && vroom == ch->in_room
	&&  (corpse = get_obj_list(ch, "corpse", ch->in_room->contents))) {
		if (HAS_SKILL(ch, gsn_vampire)) {
			act_puts("$n suck {Rblood{x from $N's corpse!!",
				 ch, NULL,victim,TO_ROOM,POS_SLEEPING);
			char_puts("You suck {Rblood{x "
				  "from the corpse!!\n\n", ch);
			gain_condition(ch, COND_BLOODLUST, 3);
		}

		if (IS_SET(ch->plr_flags, PLR_AUTOLOOK))
			do_look_in(ch, "corpse");
		if (corpse->contains)
			/* corpse exists and not empty */
			if (IS_SET(ch->plr_flags, PLR_AUTOLOOT))
				do_get(ch, "all corpse");
			else if (IS_SET(ch->plr_flags, PLR_AUTOGOLD))
				get_gold_corpse(ch, corpse);

		if (IS_SET(ch->plr_flags, PLR_AUTOSAC))
			do_sacrifice(ch, "corpse");
	}

	if (vnpc || victim->position == POS_STANDING)
		return;

	if (is_duel)
		return;

	/* Dying penalty: 2/3 way back. */
	if (IS_SET(victim->plr_flags, PLR_WANTED)
	&&  victim->level > 1
	&&  !is_duel) {
		REMOVE_BIT(victim->plr_flags, PLR_WANTED);
		victim->level--;
		victim->pcdata->plevels++;
		victim->exp = exp_for_level(victim, victim->level);
		victim->exp_tl = 0;
	}
	else 
		if (victim->exp_tl > 0)
			gain_exp(victim, -victim->exp_tl*2/3);

	if ((++victim->pcdata->death % 3) != 2)
		return;

	/* Die too much and is deleted ... :( */
	if ((cl = class_lookup(victim->class))
	&&  !CAN_FLEE(ch, cl)) {
		victim->perm_stat[STAT_CHA]--;
		if (victim->pcdata->death > cl->death_limit) {
			char msg[MAX_STRING_LENGTH];

			snprintf(msg, sizeof(msg),
				 "%d deaths limit for %s",
				 cl->death_limit, cl->name);
			delete_player(victim, msg);
			return;
		}
	}
	else if (--victim->perm_stat[STAT_CON] < 3) {
		delete_player(victim, "lack of CON");
		return;
	}
	else
		char_puts("You feel your life power has decreased "
			  "with this death.\n", victim);
}

/*
 * Inflict damage from a hit.
 */
bool damage(CHAR_DATA *ch, CHAR_DATA *victim,
	    int dam, int dt, int dam_type, int dam_flags)
{
	bool immune;
	int dam2;
	int loc;

	if (JUST_KILLED(victim))
		return FALSE;

	if (victim != ch) {
		/*
		 * Certain attacks are forbidden.
		 * Most other attacks are returned.
		 */

#if 0
		if (cant_kill(ch, victim, F_QUIET))
			return FALSE;
#endif

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
			&&  victim->master
			&&  victim->master->in_room == ch->in_room
			&&  !victim->master->fighting
			&&  number_bits(2) == 0) {
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
			victim->riding = ch->riding = 0;
	}

	/*
	 * No one in combat can hide, be invis or camoed.
	 */
	do_visible(ch, str_empty);

	/*
	 * Damage modifiers.
	 */
	if (IS_AFFECTED(victim, AFF_SANCTUARY)
	&&  !((dt == gsn_cleave) && (number_percent() < 50)))
		dam /= 2;

	if (IS_AFFECTED(victim, AFF_BLACK_SHROUD))
		dam = (7*dam)/15;

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
	loc = IS_SET(dam_flags, DAMF_SECOND) ? WEAR_SECOND_WIELD : WEAR_WIELD;

	/*
	 * Check for parry, and dodge.
	 */
	if (dt >= TYPE_HIT && ch != victim) {
		/*
		 * some funny stuff
		 */
		if (is_affected(victim, gsn_mirror)) {
			act("$n shatters into tiny fragments of glass.",
			    victim, NULL, NULL, TO_ROOM);
			extract_char(victim, TRUE);
			return FALSE;
		}

		if (check_parry(ch, victim, loc))
			return FALSE;
		if (check_block(ch, victim, loc))
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

	if (dt >= TYPE_HIT && ch != victim) {
		if ((dam2 = critical_strike(ch, victim, dam)) != 0)
			dam = dam2;
	}

	if (IS_SET(dam_flags, DAMF_SHOW))
		dam_message(ch, victim, dam, dt, immune, dam_type);

	if (dam == 0)
		return FALSE;

	if (dt >= TYPE_HIT && ch != victim)
		check_eq_damage(ch, victim, loc);

	/*
	 * Hurt the victim.
	 * Inform the victim of his new state.
	 */
	victim->hit -= dam;
	if (IS_IMMORTAL(victim) && victim->hit < 1)
		victim->hit = 1;

	update_pos(victim);

	switch(victim->position) {
	case POS_MORTAL:
		if (dam_type == DAM_HUNGER || dam_type == DAM_THIRST) break;
		act("$n is mortally wounded, and will die soon, if not aided.",
		    victim, NULL, NULL, TO_ROOM);
		char_puts( "You are mortally wounded, and will die soon, if not aided.\n", victim);
		break;

	case POS_INCAP:
		if (dam_type == DAM_HUNGER || dam_type == DAM_THIRST) break;
		act("$n is incapacitated and will slowly die, if not aided.",
		    victim, NULL, NULL, TO_ROOM);
		char_puts( "You are incapacitated and will slowly die, if not aided.\n", victim);
		break;

	case POS_STUNNED:
		if (dam_type == DAM_HUNGER || dam_type == DAM_THIRST) break;
		act("$n is stunned, but will probably recover.",
		    victim, NULL, NULL, TO_ROOM);
		char_puts("You are stunned, but will probably recover.\n",
			     victim);
		break;

	case POS_DEAD:
		act("$n is DEAD!!", victim, 0, 0, TO_ROOM);
		char_puts("You have been KILLED!!\n\n", victim);
		break;

	default:
		if (dam_type == DAM_HUNGER || dam_type == DAM_THIRST) break;
		if (dam > victim->max_hit / 4)
			char_puts("That really did HURT!\n", victim);
		if (victim->hit < victim->max_hit / 4)
			char_puts("You sure are BLEEDING!\n", victim);
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
			if (victim->level < 10)
				do_recall(victim, str_empty);
			else
				do_flee(victim, str_empty);
			return TRUE;
		}
	}

	/*
	 * Wimp out?
	 */
	if (IS_NPC(victim) && dam > 0 && victim->wait < PULSE_VIOLENCE / 2) {
		flag64_t act = victim->pIndexData->act;
		if ((IS_SET(act, ACT_WIMPY) && number_bits(2) == 0 &&
		     victim->hit < victim->max_hit / 5)
		||  (IS_AFFECTED(victim, AFF_CHARM) &&
		     victim->master != NULL &&
		     victim->master->in_room != victim->in_room)
		||  (IS_AFFECTED(victim, AFF_DETECT_FEAR) &&
		     !IS_SET(act, ACT_NOTRACK))) {
			do_flee(victim, str_empty);
			victim->last_fought = NULL;
		}
	}

	if (!IS_NPC(victim)
	&&  victim->hit > 0
	&&  (victim->hit <= victim->wimpy || IS_AFFECTED(victim, AFF_DETECT_FEAR))
	&&  victim->wait < PULSE_VIOLENCE / 2)
		do_flee(victim, str_empty);

	tail_chain();
	return TRUE;
}

bool cant_kill(CHAR_DATA *ch, CHAR_DATA *victim)
{
	/*
	 * ghosts are safe
	 * this check must be done first to avoid
	 * suicyco muttafuckas who recite 'leather-bound book' (#5743)
	 * without any target specified
	 * extracted NPCs are safe too
	 */
	if (!IS_NPC(victim)) {
		if (IS_SET(victim->plr_flags, PLR_GHOST))
			return TRUE;
		if (ch != victim
		&&  !IS_NPC(ch)
		&&  IS_SET(ch->plr_flags, PLR_GHOST))
			return TRUE;
	}
	else if (victim->extracted)
		return TRUE;

	if (victim->fighting == ch
	||  ch == victim
	||  IS_IMMORTAL(ch))
		return FALSE;

	/* handle ROOM_PEACE flags */
	if ((victim->in_room &&
	     IS_SET(victim->in_room->room_flags, ROOM_PEACE))
	||  (ch->in_room &&
	     IS_SET(ch->in_room->room_flags, ROOM_PEACE)))
		return TRUE;

	/* link dead players whose adrenalin is not gushing are safe */
	if (!IS_NPC(victim) && !IS_PUMPED(victim) && victim->desc == NULL)
		return TRUE;

	return !in_PK(ch, victim);
}

bool is_safe_nomessage(CHAR_DATA *ch, CHAR_DATA *victim)
{
	bool safe;
	CHAR_DATA *mount;

	if (IS_NPC(ch)
	&&  IS_AFFECTED(ch, AFF_CHARM)
	&&  ch->master)
		return is_safe_nomessage(ch->master, victim);
	
	if (IS_NPC(victim)
	&&  IS_AFFECTED(victim, AFF_CHARM)
	&&  victim->master)
		return is_safe_nomessage(ch, victim->master);

	if ((mount = RIDDEN(victim)))
		return is_safe_nomessage(ch, mount);

	if (victim->in_room != NULL 
		&& victim->in_room->area->clan != 0
		&& victim->clan != victim->in_room->area->clan
		&& ch->clan == victim->in_room->area->clan)
		return FALSE;

	if ((safe = cant_kill(ch, victim)) || IS_NPC(ch))
		return safe;

	if (victim != ch && IS_SET(ch->plr_flags, PLR_GHOST)) {
		char_puts("You return to your normal form.\n", ch);
		REMOVE_BIT(ch->plr_flags, PLR_GHOST);
	}

	return safe;
}

bool is_safe(CHAR_DATA *ch, CHAR_DATA *victim)
{
	if (is_safe_nomessage(ch, victim)) {
		act("The gods protect $N.",ch,NULL,victim,TO_CHAR);
		act("The gods protect $N from $n.",ch,NULL,victim,TO_ROOM);
		return TRUE;
	}
	return FALSE;
}

bool is_safe_spell(CHAR_DATA *ch, CHAR_DATA *victim, bool area)
{
#if 0
	if (ch == victim && !area)
		return TRUE;
#endif
	if (area) {
		if (IS_IMMORTAL(victim)
		||  is_same_group(ch, victim)
		||  ch == victim
		||  RIDDEN(ch) == victim
		||  MOUNTED(ch) == victim)
			return TRUE;
	}

	return is_safe(ch, victim);
}

/*
 * Check for parry.
 */
bool check_parry(CHAR_DATA *ch, CHAR_DATA *victim, int loc)
{
	int chance;

	if (!IS_AWAKE(victim))
		return FALSE;

	if (IS_NPC(victim))
		chance	= UMIN(35, victim->level);
	else {
		if (get_eq_char(victim, WEAR_WIELD) == NULL)
			return FALSE;
		chance = get_skill(victim, gsn_parry) / 2;
		if (victim->class == CLASS_WARRIOR
		||  victim->class == CLASS_SAMURAI)
			chance *= 1.2;
	}

	if (number_percent() >= chance + victim->level - ch->level)
		return FALSE;

	act("You parry $n's attack.", ch, NULL, victim, TO_VICT | ACT_VERBOSE);
	act("$N parries your attack.", ch, NULL, victim, TO_CHAR | ACT_VERBOSE);

	check_weapon_damage(ch, victim, loc);

	if (number_percent() >  get_skill(victim,gsn_parry)) {
		/* size and weight */
		chance += ch->carry_weight / 25;
		chance -= victim->carry_weight / 20;

		if (ch->size < victim->size)
			chance += (ch->size - victim->size) * 25;
		else
			chance += (ch->size - victim->size) * 10;

		/* stats */
		chance += get_curr_stat(ch, STAT_STR);
		chance -= get_curr_stat(victim, STAT_DEX) * 4/3;

		if (IS_AFFECTED(ch, AFF_FLYING))
			chance -= 10;

		/* speed */
		if (IS_NPC(ch) && IS_SET(ch->pIndexData->off_flags, OFF_FAST))
			chance += 10;
		if (IS_NPC(victim) && IS_SET(victim->pIndexData->off_flags,
					     OFF_FAST))
			chance -= 20;

		/* level */
		chance += (ch->level - victim->level) * 2;

		/* now the attack */
		if (number_percent() < (chance / 20 )) {
			act("You couldn't manage to keep your position!",
			    ch, NULL, victim, TO_VICT);
			act("You fall down!", ch, NULL, victim, TO_VICT);
			act("$N couldn't manage to hold your attack "
			    "and falls down!",
			    ch, NULL, victim, TO_CHAR);
			act("$n stunning force makes $N falling down.",
			    ch, NULL, victim, TO_NOTVICT);

			WAIT_STATE(victim, SKILL(gsn_bash)->beats);
			victim->position = POS_RESTING;
		}
	}

	check_improve(victim, gsn_parry, TRUE, 6);
	return TRUE;
}

/*
 * check blink
 */
bool check_blink(CHAR_DATA *ch, CHAR_DATA *victim)
{
	int chance;

	if (!IS_SET(victim->plr_flags, PLR_BLINK))
		return FALSE;

	if (IS_NPC(victim))
		return FALSE;
	else
		chance	= get_skill(victim, gsn_blink) / 2;

	if (number_percent() >= chance + victim->level - ch->level
	||  number_percent() < 50
	||  victim->mana < 10)
		return FALSE;

	victim->mana -= UMAX(victim->level / 10, 1);

	act("You blink out $n's attack.",
	    ch, NULL, victim, TO_VICT | ACT_VERBOSE);
	act("$N blinks out your attack.",
	    ch, NULL, victim, TO_CHAR | ACT_VERBOSE);
	check_improve(victim, gsn_blink, TRUE, 6);
	return TRUE;
}

/*
 * Check for shield block.
 */
bool check_block(CHAR_DATA *ch, CHAR_DATA *victim, int loc)
{
	int chance;

	if (!IS_AWAKE(victim))
		return FALSE;

	if (get_eq_char(victim, WEAR_SHIELD) == NULL)
		return FALSE;

	if (IS_NPC(victim))
		chance = 10;
	else {
		if (get_skill(victim,gsn_shield_block) <= 1)
			return FALSE;
		chance = get_skill(victim,gsn_shield_block) / 2;
		chance -= (victim->class == CLASS_WARRIOR) ? 0 : 10;
	}

	if (number_percent() >= chance + victim->level - ch->level)
		return FALSE;

	act("Your shield blocks $n's attack.",
	    ch, NULL, victim, TO_VICT | ACT_VERBOSE);
	act("$N deflects your attack with $S shield.",
	    ch, NULL, victim, TO_CHAR | ACT_VERBOSE);
	check_shield_damage(ch, victim, loc);
	check_improve(victim, gsn_shield_block, TRUE, 6);
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

	act("You dodge $n's attack.", ch, NULL, victim, TO_VICT | ACT_VERBOSE);
	act("$N dodges your attack.", ch, NULL, victim, TO_CHAR	| ACT_VERBOSE);

	if (number_percent() < get_skill(victim,gsn_dodge) / 20
	&&  !(IS_AFFECTED(ch, AFF_FLYING) || ch->position < POS_FIGHTING)) {
		/* size */
		if (victim->size < ch->size)
			/* bigger = harder to trip */
			chance += (victim->size - ch->size) * 10;

		/* dex */
		chance += get_curr_stat(victim, STAT_DEX);
		chance -= get_curr_stat(ch, STAT_DEX) * 3 / 2;

		if (IS_AFFECTED(victim, AFF_FLYING))
			chance -= 10;

		/* speed */
		if ((IS_NPC(victim) && IS_SET(victim->pIndexData->off_flags,
					      OFF_FAST))
		||  IS_AFFECTED(victim, AFF_HASTE))
			chance += 10;
		if ((IS_NPC(ch) && IS_SET(ch->pIndexData->off_flags, OFF_FAST))
		||  IS_AFFECTED(ch, AFF_HASTE))
			chance -= 20;

		/* level */
		chance += (victim->level - ch->level) * 2;

		/* now the attack */
		if (number_percent() < (chance / 20)) {
			act("$n lost his postion and fall down!",
			    ch, NULL, victim, TO_VICT);
			act("As $N moves you lost your position fall down!",
			    ch, NULL, victim, TO_CHAR);
			act("As $N dodges $N's attack, $N lost his position "
			    "and falls down.", ch, NULL, victim, TO_NOTVICT);

			WAIT_STATE(ch, SKILL(gsn_trip)->beats);
			ch->position = POS_RESTING;
		}
	}
	check_improve(victim, gsn_dodge, TRUE, 6);
	return TRUE;
}

/*
 * Set position of a victim.
 */
void update_pos(CHAR_DATA *victim)
{
	if (victim->hit > 0) {
		if (victim->position <= POS_STUNNED)
			victim->position = POS_STANDING;
		return;
	}

	if (IS_NPC(victim) && victim->hit < 1) {
		victim->position = POS_DEAD;
		return;
	}

	if (victim->hit <= -11) {
		victim->position = POS_DEAD;
		return;
	}

	if (victim->hit <= -6)
		victim->position = POS_MORTAL;
	else if (victim->hit <= -3)
		victim->position = POS_INCAP;
	else
		victim->position = POS_STUNNED;
}

/*
 * Start fights.
 */
void set_fighting(CHAR_DATA *ch, CHAR_DATA *victim)
{
	if (ch->fighting != NULL) {
		bug("Set_fighting: already fighting", 0);
		return;
	}

	if (IS_AFFECTED(ch, AFF_SLEEP))
		affect_bit_strip(ch, TO_AFFECTS, AFF_SLEEP);

	ch->fighting = victim;
	ch->position = POS_FIGHTING;
}

/*
 * Stop fights.
 */
void stop_fighting(CHAR_DATA *ch, bool fBoth)
{
	CHAR_DATA *fch;

	for (fch = char_list; fch; fch = fch->next) {
		if (fch == ch || (fBoth && fch->fighting == ch)) {
			fch->fighting = NULL;
			fch->position = IS_NPC(fch) ? ch->default_pos :
						      POS_STANDING;
			update_pos(fch);
		}
	}
}

/*
 * Make a corpse out of a character.
 */
void make_corpse(CHAR_DATA *ch)
{
	OBJ_DATA *corpse;
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;
	int i;

	corpse	= create_obj_of(get_obj_index(OBJ_VNUM_CORPSE_NPC),
				ch->short_descr);
	corpse->owner = mlstr_dup(ch->short_descr);

	if (IS_NPC(ch)) {
		corpse->timer	= number_range(3, 6);
		if (ch->gold > 0 || ch->silver > 0)
		  {
		    if (IS_SET(ch->form,FORM_INSTANT_DECAY))
		      obj_to_room(create_money(ch->gold, ch->silver), ch->in_room);
		    else
		      obj_to_obj(create_money(ch->gold, ch->silver), corpse);
		    ch->gold = 0;
		}
		corpse->cost = 0;
		corpse->level = ch->level;
	}
	else
	  {
		if (IS_GOOD(ch))
		  i = 0;
		if (IS_EVIL(ch))
		  i = 2;
		else
		  i = 1;

		corpse->timer	= number_range(25, 40);
		REMOVE_BIT(ch->plr_flags, PLR_CANLOOT);
		corpse->altar = hometown_table[ch->hometown].altar[i];
		corpse->pit = hometown_table[ch->hometown].pit[i];
		corpse->level = ch->level;

		if (ch->gold > 0 || ch->silver > 0) {
		    obj_to_obj(create_money(ch->gold, ch->silver), corpse);
		    ch->gold = 0;
		    ch->silver = 0;
		}
		corpse->cost = 0;
	}

	for (obj = ch->carrying; obj != NULL; obj = obj_next) {
		obj_next = obj->next_content;
		obj_from_char(obj);
		if (obj->pIndexData->item_type == ITEM_POTION)
		    obj->timer = number_range(500,1000);
		if (obj->pIndexData->item_type == ITEM_SCROLL)
		    obj->timer = number_range(1000,2500);
		if (IS_SET(obj->extra_flags,ITEM_ROT_DEATH))  {
		    obj->timer = number_range(5,10);
		    if (obj->pIndexData->item_type == ITEM_POTION)
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
		if (IS_SET(ch->parts, PART_GUTS)) {
			msg = "$n spills $s guts all over the floor.";
			vnum = OBJ_VNUM_GUTS;
		}
		break;
	case  3:
		if (IS_SET(ch->parts, PART_HEAD)) {
			msg  = "$n's severed head plops on the ground.";
			vnum = OBJ_VNUM_SEVERED_HEAD;
		}
		break;
	case  4:
		if (IS_SET(ch->parts, PART_HEART)) {
			msg  = "$n's heart is torn from $s chest.";
			vnum = OBJ_VNUM_TORN_HEART;
		}
		break;
	case  5:
		if (IS_SET(ch->parts, PART_ARMS)) {
			msg  = "$n's arm is sliced from $s dead body.";
			vnum = OBJ_VNUM_SLICED_ARM;
		}
		break;
	case  6:
		if (IS_SET(ch->parts, PART_LEGS)) {
			msg  = "$n's leg is sliced from $s dead body.";
			vnum = OBJ_VNUM_SLICED_LEG;
		}
		break;
	case 7:
		if (IS_SET(ch->parts, PART_BRAINS)) {
			msg = "$n's head is shattered, and $s brains splash all over you.";
			vnum = OBJ_VNUM_BRAINS;
		}
		break;
	}

	act(msg, ch, NULL, NULL, TO_ROOM);

	if (vnum) {
		OBJ_DATA *obj;

		obj = create_obj_of(get_obj_index(vnum), ch->short_descr);
		obj->owner = mlstr_dup(ch->short_descr);
		obj->timer = number_range(4, 7);
		obj->level = ch->level;

		if (obj->pIndexData->item_type == ITEM_FOOD) {
			if (IS_SET(ch->form,FORM_POISON))
				obj->value[3] = 1;
			else if (!IS_SET(ch->form,FORM_EDIBLE))
				SET_BIT(obj->extra_flags, ITEM_NOT_EDIBLE);
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
}

void raw_kill_org(CHAR_DATA *ch, CHAR_DATA *victim, int part)
{
	CHAR_DATA *tmp_ch, *tmp_ch_next;
	OBJ_DATA *obj,*obj_next;
	int i;
	OBJ_DATA *tattoo, *clanmark;

	for (obj = victim->carrying;obj != NULL;obj = obj_next) {
		obj_next = obj->next_content;
		if (obj->wear_loc != WEAR_NONE
		&&  oprog_call(OPROG_DEATH, obj, victim, NULL)) {
			victim->position = POS_STANDING;
			return;
		}
	}

	/* don't remember killed victims anymore */
	if (IS_NPC(ch))
		remove_mind(ch, victim->name);

	stop_fighting(victim, TRUE);
	rating_update(ch, victim);
	quest_handle_death(ch, victim);
	RESET_FIGHT_TIME(victim);
	victim->last_death_time = current_time;
	death_cry_org(victim, part);

	tattoo = get_eq_char(victim, WEAR_TATTOO);
	clanmark = get_eq_char(victim, WEAR_CLANMARK);
	if (tattoo != NULL)
		obj_from_char(tattoo);
	if (clanmark != NULL)
		obj_from_char(clanmark);
	make_corpse(victim);

	if (IS_NPC(victim)) {
		victim->pIndexData->killed++;
		kill_table[URANGE(0, victim->level, MAX_LEVEL-1)].killed++;
		extract_char(victim, TRUE);
		return;
	}

	SET_BIT(victim->plr_flags, PLR_GHOST);
	char_puts("You turn into an invincible ghost for a few minutes.\n"
		  "As long as you don't attack anything.\n",
		  victim);

	extract_char(victim, FALSE);

	while (victim->affected)
		affect_remove(victim, victim->affected);
	victim->affected_by	= 0;
	for (i = 0; i < 4; i++)
		victim->armor[i] = 100;
	victim->position	= POS_RESTING;
	victim->hit		= victim->max_hit / 10;
	victim->mana		= victim->max_mana / 10;
	victim->move		= victim->max_move;

	/* RT added to prevent infinite deaths */
	REMOVE_BIT(victim->plr_flags, PLR_BOUGHT_PET);

	victim->pcdata->condition[COND_THIRST] = 40;
	victim->pcdata->condition[COND_HUNGER] = 40;
	victim->pcdata->condition[COND_FULL] = 40;
	victim->pcdata->condition[COND_BLOODLUST] = 40;
	victim->pcdata->condition[COND_DESIRE] = 40;

	if (tattoo != NULL) {
		obj_to_char(tattoo, victim);
		equip_char(victim, tattoo, WEAR_TATTOO);
	}

	if (clanmark != NULL) {
		obj_to_char(clanmark, victim);
		equip_char(victim, clanmark, WEAR_CLANMARK);
	}

	if (victim->level > 1)
		save_char_obj(victim, FALSE);

	/*
	 * Calm down the tracking mobiles
	 */
	for (tmp_ch = char_list; tmp_ch != NULL; tmp_ch = tmp_ch_next) {
		tmp_ch_next = tmp_ch->next;
		if (tmp_ch->last_fought == victim)
			tmp_ch->last_fought = NULL;
		remove_mind(tmp_ch, victim->name);
		if (tmp_ch->target == victim 
		   && IS_NPC(tmp_ch)
		   && tmp_ch->pIndexData->vnum == MOB_VNUM_STALKER) {
			doprintf(do_clan, tmp_ch,
				"%s is dead and I can leave the realm.",
				PERS(victim, tmp_ch));
			extract_char(tmp_ch, TRUE);
		   }
	}
}

void group_gain(CHAR_DATA *ch, CHAR_DATA *victim)
{
	CHAR_DATA *lch;
	CHAR_DATA *gch;
	int xp;
	int members;
	int group_levels;

	if (!IS_NPC(victim) || victim == ch)
		return;

	if (IS_SET(victim->pIndexData->act, ACT_PET)
	||  victim->pIndexData->vnum < 100
	||  victim->master
	||  victim->leader)
		return;

	lch = ch->leader ? ch->leader : ch;

	members = 0;
	group_levels = 0;
	for (gch = ch->in_room->people; gch; gch = gch->next_in_room) {
		if (is_same_group(gch, ch)) {
			if (IS_NPC(gch)
			&&  (gch->pIndexData->vnum == MOB_VNUM_LESSER_GOLEM ||
			     gch->pIndexData->vnum == MOB_VNUM_STONE_GOLEM ||
			     gch->pIndexData->vnum == MOB_VNUM_IRON_GOLEM ||
			     gch->pIndexData->vnum == MOB_VNUM_ADAMANTITE_GOLEM))
				continue;
			if (!IS_NPC(gch)
			&&  abs(gch->level - lch->level) <= 8)
				members++;
			group_levels += gch->level;
		}
	}

	for (gch = ch->in_room->people; gch; gch = gch->next_in_room) {
		if (!is_same_group(gch, ch) || IS_NPC(gch))
			continue;

		if (gch->level - lch->level > 8) {
			char_puts("You are too high for this group.\n", gch);
			continue;
		}

		if (gch->level - lch->level < -8) {
			char_puts("You are too low for this group.\n", gch);
			continue;
		}

		xp = xp_compute(gch, victim, group_levels, members);
		char_printf(gch, "You receive %d experience points.\n", xp);
		gain_exp(gch, xp);
	}
}

/*
 * Compute xp for a kill.
 * Also adjust alignment of killer.
 * Edit this function to change xp computations.
 */
int xp_compute(CHAR_DATA *gch, CHAR_DATA *victim, int total_levels, int members)
{
	int xp;
	int base_exp;
	int level_range = victim->level - gch->level;
	int neg_cha = 0, pos_cha = 0;

/* base exp */
	switch (level_range) {
	case -9:	base_exp =   1; 	break;
	case -8:	base_exp =   2; 	break;
	case -7:	base_exp =   5; 	break;
	case -6:	base_exp =   9; 	break;
	case -5:	base_exp =  11; 	break;
	case -4:	base_exp =  22; 	break;
	case -3:	base_exp =  33; 	break;
	case -2:	base_exp =  43; 	break;
	case -1:	base_exp =  60; 	break;
	case  0:	base_exp =  74; 	break;
	case  1:	base_exp =  84; 	break;
	case  2:	base_exp =  99; 	break;
	case  3:	base_exp = 121; 	break;
	case  4:	base_exp = 143; 	break;
	default:
		if (level_range > 4)
			base_exp = 140 + 20 * (level_range - 4);
		else
			base_exp = 0;
	}

/* calculate exp multiplier */
#if 0
	if (IS_NPC(victim) && IS_SET(victim->pIndexData->act, ACT_NOALIGN))
		xp = base_exp;
	else
#endif
	if ((IS_EVIL(gch) && IS_GOOD(victim))
	||  (IS_EVIL(victim) && IS_GOOD(gch)))
		xp = base_exp * 8/5;
	else if (IS_GOOD(gch) && IS_GOOD(victim))
		xp = 0;
	else if (!IS_NEUTRAL(gch) && IS_NEUTRAL(victim))
		xp = base_exp * 1.1;
	else if (IS_NEUTRAL(gch) && !IS_NEUTRAL(victim))
		xp = base_exp * 1.3;
	else
		xp = base_exp;

	/* more exp at the low levels */
	if (gch->level < 6)
		xp = 15 * xp / (gch->level + 4);

	/* randomize the rewards */
	xp = number_range(xp * 3/4, xp * 5/4);

/* adjust for grouping */
	xp = xp * gch->level/total_levels;
	if (members == 2 || members == 3) {
		xp *= members;
		if (members == 2)
			xp = xp * 120 / 100;
		else
			xp = xp * 125 / 100;
	}

#if 0
	xp += (xp * (gch->max_hit - gch->hit)) / (gch->max_hit * 5);
#endif

	if (IS_GOOD(gch)) {
		if (IS_GOOD(victim)) {
			gch->pcdata->anti_killed++;
			neg_cha = 1;
		}
		else if (IS_NEUTRAL(victim)) {
			gch->pcdata->has_killed++;
			pos_cha = 1;
		}
		else if (IS_EVIL(victim)) {
			gch->pcdata->has_killed++;
			pos_cha = 1;
		}
	}
	else if (IS_NEUTRAL(gch)) {
		if (IS_GOOD(victim)) {
			gch->pcdata->has_killed++;
			pos_cha = 1;
		}
		else if (IS_NEUTRAL(victim)) {
			gch->pcdata->anti_killed++;
			neg_cha = 1;
		}
		else if (IS_EVIL(victim)) {
			gch->pcdata->has_killed++;
			pos_cha =1;
		}
	}
	else if (IS_EVIL(gch)) {
		if (IS_GOOD(victim)) {
			gch->pcdata->has_killed++;
			pos_cha = 1;
		}
		else if (IS_NEUTRAL(victim)) {
			gch->pcdata->has_killed++;
			pos_cha = 1;
		}
		else if (IS_EVIL(victim)) {
			gch->pcdata->anti_killed++;
			neg_cha = 1;
		}
	}

	if (neg_cha) {
		if ((gch->pcdata->anti_killed % 100) == 99) {
			char_printf(gch, "You have killed %d %s up to now.\n",
				    gch->pcdata->anti_killed,
				    IS_GOOD(gch) ?	"goods" :
				    IS_EVIL(gch) ?	"evils" :
							"neutrals");
			if (gch->perm_stat[STAT_CHA] > 3 && IS_GOOD(gch)) {
				char_puts("So your charisma "
					  "has reduced by one.\n", gch);
				gch->perm_stat[STAT_CHA] -= 1;
			}
		}
	}
	else if (pos_cha) {
		if ((gch->pcdata->has_killed % 200) == 199) {
			char_printf(gch, "You have killed %d %s up to now.\n",
				    gch->pcdata->anti_killed,
				    IS_GOOD(gch) ?	"anti-goods" :
				    IS_EVIL(gch) ?	"anti-evils" :
							"anti-neutrals");
			if (gch->perm_stat[STAT_CHA] <
						get_max_train(gch, STAT_CHA)
			&&  IS_GOOD(gch)) {
				char_puts("So your charisma "
					  "has increased by one.\n", gch);
				gch->perm_stat[STAT_CHA] += 1;
			}
		}
	}
	return xp;
}

void dam_message(CHAR_DATA *ch, CHAR_DATA *victim,
		 int dam, int dt, bool immune, int dam_type)
{
	const char *vs;
	const char *vp;
	const char *msg_char;
	const char *msg_vict = NULL;
	const char *msg_notvict;
	const char *attack = str_empty;

	if (dam == 0) {
		vs = "miss";
		vp = "misses";
	}
	else if (dam <= 4) {
		vs = "{cscratch{x";
		vp = "{cscratches{x";
	}
	else if (dam <= 8) {
		vs = "{cgraze{x";
		vp = "{cgrazes{x";
	}
	else if (dam <= 12) {
		vs = "{chit{x";
		vp = "{chits{x";
	}
	else if (dam <= 16) {
		vs = "{cinjure{x";
		vp = "{cinjures{x";
	}
	else if (dam <= 20) {
		vs = "{cwound{x";
		vp = "{cwounds{x";
	}
	else if (dam <= 24) {
		vs = "{cmaul{x";
		vp = "{cmauls{x";
	}
	else if (dam <= 28) {
		vs = "{cdecimate{x";
		vp = "{cdecimates{x";
	}
	else if (dam <= 32) {
		vs = "{cdevastate{x";
		vp = "{cdevastates{x";
	}
	else if (dam <= 36) {
		vs = "{cmaim{x";
		vp = "{cmaims{x";
	}
	else if (dam <= 42) {
		vs = "{MMUTILATE{x";
		vp = "{MMUTILATES{x";
	}
	else if (dam <= 52) {
		vs = "{MDISEMBOWEL{x";
		vp = "{MDISEMBOWELS{x";
	}
	else if (dam <= 65) {
		vs = "{MDISMEMBER{x";
		vp = "{MDISMEMBERS{x";
	}
	else if (dam <= 80) {
		vs = "{MMASSACRE{x";
		vp = "{MMASSACRES{x";
	}
	else if (dam <= 100) {
		vs = "{MMANGLE{x";
		vp = "{MMANGLES{x";
	}
	else if (dam <= 130) {
		vs = "{y*** DEMOLISH ***{x";
		vp = "{y*** DEMOLISHES ***{x";
	}
	else if (dam <= 175) {
		vs = "{y*** DEVASTATE ***{x";
		vp = "{y*** DEVASTATES ***{x";
	}
	else if (dam <= 250) {
		vs = "{y=== OBLITERATE ==={x";
		vp = "{y=== OBLITERATES ==={x";
	}
	else if (dam <= 325) {
		vs = "{y=== ATOMIZE ==={x";
		vp = "{y=== ATOMIZES ==={x";
	}
	else if (dam <= 400) {
		vs = "{R>>> ANNIHILATE <<<{x";
		vp = "{R>>> ANNIHILATES <<<{x";
	}
	else if (dam <= 500) {
		vs = "{R>>> ERADICATE <<<{x";
		vp = "{R>>> ERADICATES <<<{x";
	}
	else if (dam <= 650) {
		vs = "{R-==> ELECTRONIZE <==-{x";
		vp = "{R-==> ELECTRONIZES <==-{x";
	}
	else if (dam <= 800) {
		vs = "{R-==> SKELETONIZE <==-{x";
		vp = "{R-==> SKELETONIZES <==-{x";
	}
	else if (dam <= 1000) {
		vs = "{R### NUKE ###{x";
		vp = "{R### NUKES ###{x";
	}
	else if (dam <= 1250) {
		vs = "{R### TERMINATE ###{x";
		vp = "{R### TERMINATES ###{x";
	}
	else if (dam <= 1500) {
		vs = "{R[*] TEAR UP [*]{x";
		vp = "{R[*] TEARS UP [*]{x";
	}
	else {
		vs = "{*{R[*] POWER HIT [*]{x";
		vp = "{*{R[*] POWER HITS [*]{x";
	}

	if (dt == TYPE_HIT || dt == TYPE_HUNGER) {
		if (ch == victim) {
			switch (dam_type) {
			case DAM_HUNGER:
				msg_notvict = "$n's hunger $u $mself!";
				msg_char = "Your hunger $u yourself!";
				break;

			case DAM_THIRST:
				msg_notvict = "$n's thirst $u $mself!";
				msg_char = "Your thirst $u yourself!";
				break;

			case DAM_LIGHT_V:
				msg_notvict = "The light of room $u $n!";
				msg_char = "The light of room $u you!";
				break;

			case DAM_TRAP_ROOM:
				msg_notvict = "The trap at room $u $n!";
				msg_char = "The trap at room $u you!";
				break;

			default:
				msg_notvict = "$n $u $mself!";
				msg_char = "You $u yourself!";
				break;
			}
		}
		else {
			msg_notvict = "$n $u $N.";
			msg_char = "You $u $N.";
			msg_vict = "$n $u you.";
		}
	}
	else {
		SKILL_DATA *sk;

/* XXX */
#define MAX_DAMAGE_MESSAGE 40
		if ((sk = skill_lookup(dt)))
			attack	= sk->noun_damage;
		else if (dt >= TYPE_HIT && dt <= TYPE_HIT + MAX_DAMAGE_MESSAGE)
			attack	= attack_table[dt - TYPE_HIT].noun;
		else {
			bug("Dam_message: bad dt %d.", dt);
			dt = TYPE_HIT;
			attack = attack_table[0].name;
		}

		if (immune) {
			if (ch == victim) {
				msg_notvict = "$n is unaffected by $s own $U.";
				msg_char = "Luckily, you are immune to that.";
			}
			else {
				msg_notvict = "$N is unaffected by $n's $U!";
				msg_char = "$N is unaffected by your $U!";
				msg_vict = "$n's $U is powerless against you.";
			}
		}
		else {
			vs = vp;

			if (ch == victim) {
				msg_notvict = "$n's $U $u $m.";
				msg_char = "Your $U $u you.";
			}
			else {
				msg_notvict = "$n's $U $u $N.";
				msg_char = "Your $U $u $N.";
				msg_vict = "$n's $U $u you.";
			}
		}
	}

	if (ch == victim) {
		act_puts3(msg_notvict, ch, vp, NULL, attack,
			  TO_ROOM | ACT_TRANS, POS_RESTING);
		act_puts3(msg_char, ch, vs, NULL, attack,
			  TO_CHAR | ACT_TRANS, POS_RESTING);
	}
	else {
		act_puts3(msg_notvict, ch, vp, victim, attack,
			  TO_NOTVICT | ACT_TRANS, POS_RESTING);
		act_puts3(msg_char, ch, vs, victim, attack,
			  TO_CHAR | ACT_TRANS, POS_RESTING);
		act_puts3(msg_vict, ch, vp, victim, attack,
			  TO_VICT | ACT_TRANS, POS_RESTING);
	}
}

void do_kill(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int chance;

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		char_puts("Kill whom?\n", ch);
		return;
	}

	WAIT_STATE(ch, 1 * PULSE_VIOLENCE);

	if ((victim = get_char_room(ch, arg)) == NULL) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	if (ch->position == POS_FIGHTING) {
		if (victim == ch->fighting)
			char_puts("You do the best you can!\n", ch);
		else if (victim->fighting != ch)
			char_puts("One battle at a time, please.\n",ch);
		else {
			act("You start aiming at $N.",ch,NULL,victim,TO_CHAR);
			ch->fighting = victim;
		}
		return;
	}

	if (!IS_NPC(victim)) {
		char_puts("You must MURDER a player.\n", ch);
		return;
	}

	if (victim == ch) {
		char_puts("You hit yourself.  Ouch!\n", ch);
		multi_hit(ch, ch, TYPE_UNDEFINED);
		return;
	}

	if (IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim) {
		act("$N is your beloved master.", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (is_safe(ch, victim))
		return;

	if ((chance = get_skill(ch, gsn_mortal_strike))
	&&  get_eq_char(ch, WEAR_WIELD)
	&&  ch->level > (victim->level - 5)) {
		chance /= 30;
		chance += 1 + (ch->level - victim->level) / 2;
		if (number_percent() < chance) {
			act_puts("Your flash strike instantly slays $N!",
				 ch, NULL, victim, TO_CHAR, POS_RESTING);
			act_puts("$n flash strike instantly slays $N!",
				 ch, NULL, victim, TO_NOTVICT,
				 POS_RESTING);
			act_puts("$n flash strike instantly slays you!",
				 ch, NULL, victim, TO_VICT, POS_DEAD);
			damage(ch, victim, (victim->hit + 1),
			       gsn_mortal_strike, DAM_NONE, DAMF_SHOW);
			check_improve(ch, gsn_mortal_strike, TRUE, 1);
			return;
		} else
			check_improve(ch, gsn_mortal_strike, FALSE, 3);
	}

	multi_hit(ch, victim, TYPE_UNDEFINED);
}

void do_murde(CHAR_DATA *ch, const char *argument)
{
	char_puts("If you want to MURDER, spell it out.\n", ch);
	return;
}

void do_murder(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int chance;

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		char_puts("Murder whom?\n", ch);
		return;
	}

	if (IS_AFFECTED(ch, AFF_CHARM)
	||  (IS_NPC(ch) && IS_SET(ch->pIndexData->act, ACT_PET)))
		return;

	WAIT_STATE(ch, 1 * PULSE_VIOLENCE);

	if ((victim = get_char_room(ch, arg)) == NULL) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	if (victim == ch) {
		char_puts("Suicide is a mortal sin.\n", ch);
		return;
	}

	if (IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim) {
		act("$N is your beloved master.", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (ch->position == POS_FIGHTING) {
		char_puts("You do the best you can!\n", ch);
		return;
	}

	if (is_safe(ch, victim))
		return;

	if ((chance = get_skill(ch, gsn_mortal_strike))
	&&  get_eq_char(ch, WEAR_WIELD)
	&&  ch->level > (victim->level - 5)) {
		chance /= 30;
		chance += 1 + (ch->level - victim->level) / 2;
		if (number_percent() < chance) {
			act_puts("Your flash strike instantly slays $N!",
				 ch, NULL, victim, TO_CHAR, POS_RESTING);
			act_puts("$n flash strike instantly slays $N!",
				 ch, NULL, victim, TO_NOTVICT,
				 POS_RESTING);
			act_puts("$n flash strike instantly slays you!",
				 ch, NULL, victim, TO_VICT, POS_DEAD);
			damage(ch, victim, (victim->hit + 1),
			       gsn_mortal_strike, DAM_NONE, DAMF_SHOW);
			check_improve(ch, gsn_mortal_strike, TRUE, 1);
			return;
		} else
			check_improve(ch, gsn_mortal_strike, FALSE, 3);
	}

	multi_hit(ch, victim, TYPE_UNDEFINED);
}

void do_flee(CHAR_DATA *ch, const char *argument)
{
	ROOM_INDEX_DATA *was_in;
	ROOM_INDEX_DATA *now_in;
	CHAR_DATA *victim;
	int attempt;
	CLASS_DATA *cl;

	if (RIDDEN(ch)) {
		char_puts("You should ask to your rider!\n", ch);
		return;
	}

	if (MOUNTED(ch))
		do_dismount(ch, str_empty);

	if ((victim = ch->fighting) == NULL) {
		if (ch->position == POS_FIGHTING)
			ch->position = POS_STANDING;
		char_puts("You aren't fighting anyone.\n", ch);
		return;
	}

	if ((cl = class_lookup(ch->class))
	&&  !CAN_FLEE(ch, cl)) {
		 char_puts("Your honour doesn't let you flee, "
			   "try dishonoring yourself.\n", ch);
		 return;
	}

	was_in = ch->in_room;
	for (attempt = 0; attempt < 6; attempt++) {
		EXIT_DATA *pexit;
		int door;

		door = number_door();
		if ((pexit = was_in->exit[door]) == 0
		     || pexit->u1.to_room == NULL
		     || (IS_SET(pexit->exit_info, EX_CLOSED)
		         && (!IS_AFFECTED(ch, AFF_PASS_DOOR)
		             || IS_SET(pexit->exit_info,EX_NOPASS))
		             && !IS_TRUSTED(ch,ANGEL))
		         || (IS_SET(pexit->exit_info , EX_NOFLEE))
		         || (IS_NPC(ch)
		             && IS_SET(pexit->u1.to_room->room_flags, ROOM_NOMOB)))
			continue;

		move_char(ch, door, FALSE);
		if ((now_in = ch->in_room) == was_in)
		    continue;

		ch->in_room = was_in;
		act("$n has fled!", ch, NULL, NULL, TO_ROOM);
		ch->in_room = now_in;

		if (!IS_NPC(ch)) {
			act_puts("You fled from combat!",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
			if (ch->level < LEVEL_HERO) {
				char_printf(ch, "You lose %d exps.\n", 10);
				gain_exp(ch, -10);
			}
		} else
			ch->last_fought = NULL;

		stop_fighting(ch, TRUE);
		return;
	}

	char_puts("PANIC! You couldn't escape!\n", ch);
	return;
}

void do_sla(CHAR_DATA *ch, const char *argument)
{
	char_puts("If you want to SLAY, spell it out.\n", ch);
	return;
}

void do_slay(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *victim;
	char arg[MAX_INPUT_LENGTH];

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		char_puts("Slay whom?\n", ch);
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	if (ch == victim) {
		char_puts("Suicide is a mortal sin.\n", ch);
		return;
	}

	if (IS_IMMORTAL(victim)) {
		char_puts("You failed.\n", ch);
		return;
	}

	act("You slay $M in cold blood!", ch, NULL, victim, TO_CHAR);
	act("$n slays you in cold blood!", ch, NULL, victim, TO_VICT);
	act("$n slays $N in cold blood!", ch, NULL, victim, TO_NOTVICT);
	raw_kill(ch, victim);
}

/*
 * Check for obj dodge.
 */
bool check_obj_dodge(CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj, int bonus)
{
	int chance;

	if (!IS_AWAKE(victim) || MOUNTED(victim))
		return FALSE;

	if (!IS_NPC(victim) && HAS_SKILL(victim, gsn_spellbane)) {
		if (victim->pcdata->clan_status) {
			act("You catch $p that had been shot to you.",
			    ch, obj, victim, TO_VICT);
			act("$N catches $p that had been shot to $M.",
			    ch, obj, victim, TO_CHAR);
			act("$n catches $p that had been shot to $m.",
			    victim, obj, ch, TO_NOTVICT);
			obj_to_char(obj, victim);
		}
		return TRUE;
	}

	if (IS_NPC(victim))
		 chance  = UMIN(30, victim->level);
	else {
		chance  = get_skill(victim, gsn_dodge) / 2;
		/* chance for high dex. */
		chance += 2 * (get_curr_stat(victim, STAT_DEX) - 20);
		if (victim->class == CLASS_WARRIOR
		||  victim->class == CLASS_SAMURAI)
			chance *= 1.2;
		if (victim->class == CLASS_THIEF
		||  victim->class ==CLASS_NINJA)
			chance *= 1.1;
	}

	chance -= (bonus - 90);
	chance /= 2;
	if (number_percent() >= chance)
		return FALSE;

	act("You dodge $p that had been shot to you.",
	    ch, obj, victim, TO_VICT);
	act("$N dodges $p that had been shot to $M.",
	    ch, obj, victim, TO_CHAR);
	act("$n dodges $p that had been shot to $m.",
	    victim, obj, ch, TO_NOTVICT);
	obj_to_room(obj, victim->in_room);
	check_improve(victim, gsn_dodge, TRUE, 6);

	return TRUE;
}

void do_dishonor(CHAR_DATA *ch, const char *argument)
{
	ROOM_INDEX_DATA *was_in;
	ROOM_INDEX_DATA *now_in;
	CHAR_DATA *gch;
	int attempt, level = 0;
	int sn_dishonor;
	int chance;

	if (RIDDEN(ch)) {
		char_puts("You should ask to your rider!\n", ch);
		return;
	}

	if ((sn_dishonor = sn_lookup("dishonor")) < 0
	||  !HAS_SKILL(ch, sn_dishonor)) {
		char_puts("Which honor?\n", ch);
		return;
	}

	if (ch->fighting == NULL) {
		if (ch->position == POS_FIGHTING)
			ch->position = POS_STANDING;
		char_puts("You aren't fighting anyone.\n", ch);
		return;
	}

	for (gch = char_list; gch; gch = gch->next)
		  if (is_same_group(gch, ch->fighting) || gch->fighting == ch)
			level += gch->level;

	if ((ch->fighting->level - ch->level) < 5 && ch->level > (level / 3)) {
		 char_puts("Your fighting doesn't worth "
			   "to dishonor yourself.\n", ch);
		 return;
	}

	was_in = ch->in_room;
	chance = get_skill(ch, sn_dishonor);
	for (attempt = 0; attempt < 6; attempt++) {
		EXIT_DATA *pexit;
		int door;

		if (number_percent() >= chance)
			continue;

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
		act("$n has dishonored $mself!",
		    ch, NULL, NULL, TO_ROOM);
		ch->in_room = now_in;

		if (!IS_NPC(ch)) {
			char_puts("You dishonored yourself "
				     "and flee from combat.\n",ch);
			if (ch->level < LEVEL_HERO) {
				char_printf(ch, "You lose %d exps.\n",
					    ch->level);
				gain_exp(ch, -(ch->level));
			}
		}
		else
			ch->last_fought = NULL;

		stop_fighting(ch, TRUE);
		if (MOUNTED(ch))
			do_dismount(ch,str_empty);

		check_improve(ch, sn_dishonor, TRUE, 1);
		return;
	}

	char_puts("PANIC! You couldn't escape!\n", ch);
	check_improve(ch, sn_dishonor, FALSE, 1);
}

void do_surrender(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *mob;

	if (!IS_NPC(ch)) {
		char_puts("Huh?\n", ch);
		return;
	}

	if ((mob = ch->fighting) == NULL) {
		char_puts("But you're not fighting!\n", ch);
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
