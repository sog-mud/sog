/*
 * $Id: fight.c,v 1.205 1999-10-11 11:52:17 kostik Exp $
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

#if !defined (WIN32)
#	include <unistd.h>
#endif

#include "merc.h"
#include "quest.h"
#include "fight.h"
#include "rating.h"
#include "mob_prog.h"
#include "obj_prog.h"

/*
 * Local functions.
 */
void	check_assist		(CHAR_DATA *ch, CHAR_DATA *victim);
bool	check_dodge		(CHAR_DATA *ch, CHAR_DATA *victim);
bool	check_parry		(CHAR_DATA *ch, CHAR_DATA *victim, int loc);
bool	check_block		(CHAR_DATA *ch, CHAR_DATA *victim, int loc);
bool	check_blink		(CHAR_DATA *ch, CHAR_DATA *victim);
bool	check_hand_block	(CHAR_DATA *ch, CHAR_DATA *victim);
void	dam_message		(CHAR_DATA *ch, CHAR_DATA *victim, int dam,
				 const char *dt, bool immune, int dam_class,
				 int dam_flags);
void	death_cry		(CHAR_DATA *ch);
void	group_gain		(CHAR_DATA *ch, CHAR_DATA *victim);
int	xp_compute		(CHAR_DATA *gch, CHAR_DATA *victim,
				 int total_levels, int members);
bool	is_safe 		(CHAR_DATA *ch, CHAR_DATA *victim);

void	make_corpse		(CHAR_DATA *ch);
void	mob_hit 		(CHAR_DATA *ch, CHAR_DATA *victim,
				 const char *dt);
void	disarm			(CHAR_DATA *ch, CHAR_DATA *victim,
				 int disarm_second);
int	critical_strike		(CHAR_DATA *ch, CHAR_DATA *victim, int dam);
void	check_eq_damage		(CHAR_DATA *ch, CHAR_DATA *victim, int loc);
void	check_shield_damage	(CHAR_DATA *ch, CHAR_DATA *victim, int loc);
void	check_weapon_damage	(CHAR_DATA *ch, CHAR_DATA *victim, int loc);
int 	check_forest		(CHAR_DATA *ch);

#define FOREST_ATTACK 1
#define FOREST_DEFENCE 2
#define FOREST_NONE 0

void	handle_death		(CHAR_DATA *ch, CHAR_DATA *victim);

/*
 * Gets all money from the corpse.
 */
void get_gold_corpse(CHAR_DATA *ch, OBJ_DATA *corpse)
{
	OBJ_DATA *tmp, *tmp_next;
	for (tmp = corpse->contains; tmp; tmp = tmp_next) {
		tmp_next = tmp->next_content;
		if (tmp->pObjIndex->item_type == ITEM_MONEY)
			get_obj(ch, tmp, corpse, NULL);
	}
}

int check_forest(CHAR_DATA* ch)
{
	AFFECT_DATA* paf;

	if (ch->in_room->sector_type != SECT_FOREST
	&&  ch->in_room->sector_type != SECT_HILLS
	&&  ch->in_room->sector_type != SECT_MOUNTAIN) 
		return FOREST_NONE;

	if ((paf = is_affected(ch, "forest fighting")) == NULL)
		return FOREST_NONE;

	if (INT_VAL(paf->location) == APPLY_AC) 
		return FOREST_DEFENCE;
	else 
		return FOREST_ATTACK;
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

	for (ch = char_list; ch; ch = ch_next) {
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

		if (IS_NPC(ch) && !IS_NPC(victim))
			NPC(ch)->last_fought = victim;

		SET_FIGHT_TIME(ch);

		if (victim->in_room != ch->in_room)
			continue;

		for (obj = ch->carrying; obj; obj = obj_next) {
			obj_next = obj->next_content;
			if (ch->fighting == NULL)
				break;
			oprog_call(OPROG_FIGHT, obj, ch, NULL);
		}

		if ((victim = ch->fighting) == NULL
		||  victim->in_room != ch->in_room)
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
void check_assist(CHAR_DATA *ch, CHAR_DATA *victim)
{
	CHAR_DATA *rch, *rch_next;

	for (rch = ch->in_room->people; rch != NULL; rch = rch_next) {
		rch_next = rch->next_in_room;

		if (!IS_AWAKE(rch) || rch->fighting != NULL)
			continue;

		/*
		 * ASSIST_PLAYERS mobs
		 */
		if (IS_NPC(rch) && !IS_NPC(ch)
		&&  IS_SET(rch->pMobIndex->off_flags, ASSIST_PLAYERS)
		&&  rch->level + 6 > victim->level) {
			dofun("emote", rch, "screams and attacks!");
			multi_hit(rch, victim, TYPE_UNDEFINED);
			continue;
		}

		/*
		 * charmed chars or PCs with PLR_AUTOASSIST
		 */
		if (((!IS_NPC(rch) && IS_SET(PC(rch)->plr_flags,
					     PLR_AUTOASSIST)) ||
		     IS_AFFECTED(rch, AFF_CHARM))
		&&  is_same_group(ch, rch)
		&&  !is_safe_nomessage(rch, victim)) {
			multi_hit (rch, victim, TYPE_UNDEFINED);
			continue;
		}

		if (!IS_NPC(ch)) {
			if (RIDDEN(rch) == ch)
				multi_hit(rch, victim, TYPE_UNDEFINED);
			continue;
		}

		/* that's all for !IS_NPC */
		if (!IS_NPC(rch))
			continue;

		if (IS_SET(rch->pMobIndex->off_flags, ASSIST_ALL)
		||  (rch->race == ch->race &&
		     IS_SET(rch->pMobIndex->off_flags, ASSIST_RACE))
		||  (rch->pMobIndex == ch->pMobIndex &&
		     IS_SET(rch->pMobIndex->off_flags, ASSIST_VNUM))
		||  (IS_SET(rch->pMobIndex->off_flags, ASSIST_ALIGN) &&
		     NALIGN(rch) == NALIGN(ch))) {
			CHAR_DATA *vch;
			CHAR_DATA *target;
			int number;

			if (number_bits(1) == 0)
				continue;

			target = NULL;
			number = 0;

			for (vch = ch->in_room->people; vch;
						vch = vch->next_in_room) {
				if (can_see(rch, vch)
				&&  is_same_group(vch, victim)
				&&  number_range(0, number) == 0) {
					target = vch;
					number++;
				}
			}

			if (target != NULL) {
				dofun("emote", rch, "screams and attacks!");
				multi_hit(rch, target, TYPE_UNDEFINED);
			}
		}
	}
}

void secondary_hit(CHAR_DATA *ch, CHAR_DATA *victim, const char *dt) 
{
	int chance;
	
	if (get_eq_char(ch, WEAR_SECOND_WIELD) != NULL) {
		chance = get_skill(ch, "dual wield") / 2;
		if (number_percent() < chance) {
			one_hit(ch, victim, dt, WEAR_SECOND_WIELD);
			check_improve(ch, "dual wield", TRUE, 2);
		}
	}
	
	if (free_hands(ch)>=2) {
		chance = get_skill(ch, "hand to hand") / 2;
		if (number_percent() < chance) 
			one_hit(ch, victim, dt, WEAR_WIELD);
	}
}
/*
 * Do one group of attacks.
 */
void multi_hit(CHAR_DATA *ch, CHAR_DATA *victim, const char *dt)
{
	OBJ_DATA *wield;
	int     chance;

	/* no attacks for stunnies -- just a check */
	if (ch->position < POS_RESTING)
		return;

	/* ridden's adjustment */
	if (RIDDEN(victim) && !IS_NPC(victim->mount)) {
		if (victim->mount->fighting == NULL
		|| victim->mount->fighting == ch)
			victim = victim->mount;
		else
			dofun("dismount", victim->mount, str_empty);
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
		REMOVE_BIT(ch->affected_by, AFF_STUN);
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

	if ((chance = get_skill(ch, "area attack"))
	&&  number_percent() < chance) {
		int count = 0, max_count;
		CHAR_DATA *vch, *vch_next;

		check_improve(ch, "area attack", TRUE, 6);

		if (LEVEL(ch) < 70)
			max_count = 1;
		else if (LEVEL(ch) < 80)
			max_count = 2;
		else if (LEVEL(ch) < 90)
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

	wield = get_eq_char(ch, WEAR_WIELD);
	if (wield && wield->pObjIndex->item_type == ITEM_WEAPON
	&&  WEAPON_IS(wield, WEAPON_STAFF)
	&&  number_percent() < get_skill(ch, "staff") / 3)
		one_hit(ch, victim, dt, WEAR_WIELD);

	if (ch->fighting != victim
	||  SKILL_IS(dt, "backstab")
	||  SKILL_IS(dt, "cleave")
	||  SKILL_IS(dt, "ambush")
	||  SKILL_IS(dt, "dual backstab")
	||  SKILL_IS(dt, "circle")
	||  SKILL_IS(dt, "assassinate")
	||  SKILL_IS(dt, "vampiric bite")
	||  SKILL_IS(dt, "knife"))
		return;

	secondary_hit(ch, victim, dt);

	if(ch->fighting != victim) 
		return;

	chance = get_skill(ch, "second attack") / 2;
	if (IS_AFFECTED(ch, AFF_SLOW))
		chance /= 2;
	if (IS_AFFECTED(ch, AFF_HASTE))
		chance += 50;
	if (number_percent() < chance) {
		one_hit(ch, victim, dt, WEAR_WIELD);
		check_improve(ch, "second attack", TRUE, 5);
		if (ch->fighting != victim)
			return;
		secondary_hit(ch, victim, dt);
	}

	chance = get_skill(ch, "third attack") / 3;
	if (IS_AFFECTED(ch, AFF_SLOW))
		chance /= 2;
	if (IS_AFFECTED(ch, AFF_HASTE))
		chance += 25;
	if (number_percent() < chance) {
		one_hit(ch, victim, dt, WEAR_WIELD);
		check_improve(ch, "third attack", TRUE, 6);
		if (ch->fighting != victim)
			return;

		secondary_hit(ch, victim, dt);
		if (ch->fighting != victim)
			return;
	}

	chance = get_skill(ch, "fourth attack") / 4;
	if (IS_AFFECTED(ch, AFF_SLOW))
		chance /= 2;
	if (IS_AFFECTED(ch, AFF_HASTE))
		chance += 15;
	if (number_percent() < chance) {
		one_hit(ch, victim, dt, WEAR_WIELD);
		check_improve(ch, "fourth attack", TRUE, 7);
		if (ch->fighting != victim)
			return;

		secondary_hit(ch, victim, dt);
		if (ch->fighting != victim)
			return;
	}

	chance = get_skill(ch, "fifth attack") / 5;
	if (IS_AFFECTED(ch, AFF_SLOW))
		chance /= 2;
	if (IS_AFFECTED(ch, AFF_HASTE))
		chance +=7;
	if (number_percent() < chance) {
		one_hit(ch, victim, dt, WEAR_WIELD);
		check_improve(ch, "fifth attack", TRUE, 8);
		if (ch->fighting != victim)
			return;

		secondary_hit(ch, victim, dt);
		if (ch->fighting != victim)
			return;

	}

	if (check_forest(ch) == FOREST_ATTACK) {
		chance = get_skill(ch, "forest fighting");
		while (number_percent() < chance) {
			one_hit(ch, victim, dt, WEAR_WIELD);
			check_improve (ch, "forest fighting", TRUE, 8);
			if (ch->fighting != victim)
				return;
			secondary_hit(ch, victim, dt);
			if (ch->fighting != victim)
				return;
			chance /= 3;
		}
	}

}

/* procedure for all mobile attacks */
void mob_hit(CHAR_DATA *ch, CHAR_DATA *victim, const char *dt)
{
	CHAR_DATA *vch, *vch_next;
	flag64_t act = ch->pMobIndex->act;
	flag64_t off = ch->pMobIndex->off_flags;

	/* no attack by ridden mobiles except spec_casts */
	if (RIDDEN(ch)) {
		if (ch->fighting != victim) {
			stop_fighting(ch, FALSE);
			set_fighting(ch, victim);
		}
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

	if (ch->fighting != victim
	||  SKILL_IS(dt, "backstab")
	||  SKILL_IS(dt, "cleave")
	||  SKILL_IS(dt, "ambush")
	||  SKILL_IS(dt, "dual backstab")
	||  SKILL_IS(dt, "circle")
	||  SKILL_IS(dt, "assassinate")
	||  SKILL_IS(dt, "vampiric bite")
	||  SKILL_IS(dt, "knife"))
		return;

	if (number_percent() < get_skill(ch, "second attack") / 2) {
		one_hit(ch, victim, dt, WEAR_WIELD);
		if (ch->fighting != victim)
			return;
	}

	if (number_percent() < get_skill(ch, "third attack") / 4) {
		one_hit(ch, victim, dt, WEAR_WIELD);
		if (ch->fighting != victim)
			return;
	}

	if (number_percent() < get_skill(ch, "fourth attack") / 6) {
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
			dofun("bash", ch, str_empty);
		break;

	case 1:
		if (IS_SET(off, OFF_BERSERK)
		&&  !IS_AFFECTED(ch, AFF_BERSERK))
			dofun("berserk", ch, str_empty);
		break;


	case 2:
		if (IS_SET(off, OFF_DISARM)
		||  IS_SET(act, ACT_WARRIOR | ACT_THIEF)) {
			if (number_range(0, 1)
			&&  get_eq_char(victim, WEAR_SECOND_WIELD))
				dofun("disarm", ch, "second");
			else if (get_eq_char(victim, WEAR_WIELD))
				dofun("disarm", ch, str_empty);
		}
		break;

	case 3:
		if (IS_SET(off, OFF_KICK))
			dofun("kick", ch, str_empty);
		break;

	case 4:
		if (IS_SET(off, OFF_DIRT_KICK))
			dofun("dirt", ch, str_empty);
		break;

	case 5:
		if (IS_SET(off, OFF_TAIL))
			dofun("tail", ch, str_empty);
		break;

	case 6:
		if (IS_SET(off, OFF_TRIP))
			dofun("trip", ch, str_empty);
		break;
	case 7:
		if (IS_SET(off, OFF_CRUSH))
			dofun("crush", ch, str_empty);
		break;
	}
}

int get_dam_class(CHAR_DATA *ch, OBJ_DATA *wield,
		  const char **dt, int *dam_flags)
{
	if (IS_NULLSTR(*dt)) {
		SET_BIT(*dam_flags, DAMF_HIT);
		if (wield && wield->pObjIndex->item_type == ITEM_WEAPON)
			*dt = wield->value[3].s;
		else
			*dt = ch->damtype;
	}

	if (wield && wield->pObjIndex->item_type == ITEM_WEAPON)
		return damtype_class(wield->value[3].s);
	else
		return damtype_class(ch->damtype);
}

/*
 * Hit one guy once.
 */
void one_hit(CHAR_DATA *ch, CHAR_DATA *victim, const char *dt, int loc)
{
	OBJ_DATA *wield;
	int victim_ac;
	int thac0;
	int thac0_00;
	int thac0_32;
	int dam;
	int diceroll;
	const char *weapon_sn = NULL;
	int sk, sk2;
	int dam_class;
	bool counter;
	bool result;
	int sercount;
	int dam_flags;

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
	dam_class = get_dam_class(ch, wield, &dt, &dam_flags);

	/* get the weapon skill */
	weapon_sn = get_weapon_sn(wield);
	sk = 20 + get_weapon_skill(ch, weapon_sn);

	/*
	 * Calculate to-hit-armor-class-0 versus armor.
	 */
	if (IS_NPC(ch)) {
		flag64_t act = ch->pMobIndex->act;

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
		class_t *cl;

		if ((cl = class_lookup(ch->class)) == NULL)
			return;

		thac0_00 = cl->thac0_00;
		thac0_32 = cl->thac0_32;
	}

	thac0  = interpolate(LEVEL(ch), thac0_00, thac0_32);

	if (thac0 < 0)
		thac0 = thac0/2;

	if (thac0 < -5)
		thac0 = -5 + (thac0 + 5) / 2;

	thac0 -= GET_HITROLL(ch) * sk / 100;
	thac0 += 5 * (100 - sk) / 100;

	if (SKILL_IS(dt, "backstab"))
		thac0 -= 10 * (100 - get_skill(ch, "backstab"));
	else if (SKILL_IS(dt, "dual_backstab"))
		thac0 -= 10 * (100 - get_skill(ch, "dual backstab"));
	else if (SKILL_IS(dt, "cleave"))
		thac0 -= 10 * (100 - get_skill(ch, "cleave"));
	else if (SKILL_IS(dt, "ambush"))
		thac0 -= 10 * (100 - get_skill(ch, "ambush"));
	else if (SKILL_IS(dt, "vampiric bite"))
		thac0 -= 10 * (100 - get_skill(ch, "vampiric bite"));
	else if (SKILL_IS(dt, "charge"))
		thac0 -= 10 * (100 - get_skill(ch, "charge"));

	switch(dam_class) {
	case DAM_PIERCE:victim_ac = GET_AC(victim,AC_PIERCE)/10; break;
	case DAM_BASH:  victim_ac = GET_AC(victim,AC_BASH)/10; 	 break;
	case DAM_SLASH: victim_ac = GET_AC(victim,AC_SLASH)/10;	 break;
	default:	victim_ac = GET_AC(victim,AC_EXOTIC)/10; break;
	}

	if (victim_ac < -15)
		victim_ac = (victim_ac + 15) / 5 - 15;

	if (get_skill(victim, "armor use") > 70) {
		check_improve(victim, "armor use", TRUE, 8);
		victim_ac -= (victim->level) / 2;
	}

	if (!can_see(ch, victim)) {
		 if ((sk2 = get_skill(ch, "blind fighting"))
		 &&  number_percent() < sk2)
			check_improve(ch, "blind fighting", TRUE, 16);
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
		damage(ch, victim, 0, dt, dam_class, dam_flags);
		tail_chain();
		return;
	}

	/*
	 * Hit.
	 * Calc damage.
	 */

	if (IS_NPC(ch) && wield == NULL) {
		NPC_DATA *npc = NPC(ch);
		dam = dice(npc->dam.dice_number, npc->dam.dice_type);
	} else {
		if (weapon_sn != NULL)
			check_improve(ch, weapon_sn, TRUE, 5);
		if (wield != NULL) {
			dam = dice(INT_VAL(wield->value[1]),
				   INT_VAL(wield->value[2])) * sk / 100;

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
		} else {
			dam = number_range(1 + 4 * sk / 100,
					   2 * LEVEL(ch) / 3 * sk / 100);
			if ((sk2 = get_skill(ch, "master hand"))
			&& number_percent() <= sk2) {
				check_improve(ch, "master hand", TRUE, 6);
				dam += dam * 110 /100;
				if (number_percent() < sk2/5+LEVEL(ch)-LEVEL(victim)) {
					SET_BIT(victim->affected_by,
						AFF_WEAK_STUN);
					act_puts("You hit $N with a stunning "
						 "force!", ch, NULL, victim,
						 TO_CHAR, POS_DEAD);
					act_puts("$n hits you with a stunning "
						 "force!", ch, NULL, victim,
						 TO_VICT, POS_DEAD);
					act_puts("$n hits $N with a stunning "
						 "force!", ch, NULL, victim,
						 TO_NOTVICT, POS_RESTING);
					check_improve(ch, "master hand",
						      TRUE, 6);
				}
			}
		}
	}

	/*
	 * Bonuses.
	 */
	if ((sk2 = get_skill(ch, "enhanced damage"))
	&&  (diceroll = number_percent()) <= sk2) {
		check_improve(ch, "enhanced damage", TRUE, 6);
		dam += dam * diceroll * sk2 / 10000;
	}

	if (SKILL_IS(weapon_sn, "sword")
	&&  (sk2 = get_skill(ch, "mastering sword"))
	&&  number_percent() <= sk2) {
		OBJ_DATA *katana;

		check_improve(ch, "mastering sword", TRUE, 6);
		dam += dam * 110 /100;

		if (((katana = get_eq_char(ch,WEAR_WIELD)) ||
		     (katana = get_eq_char(ch, WEAR_SECOND_WIELD)))
		&&  IS_WEAPON_STAT(katana, WEAPON_KATANA)
		&&  strstr(mlstr_mval(&katana->ed->description), ch->name)) {
			AFFECT_DATA *paf;

			if ((katana->cost = ++katana->cost % 250) == 0
			&&  (paf = affect_find(katana->affected, "katana"))) {
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
	if (SKILL_IS(dt, "backstab")
	||  SKILL_IS(dt, "vampiric bite"))
		sercount += 40;
	if (!IS_IMMORTAL(ch) && IS_PUMPED(ch))
		sercount += 10;
	sercount *= 2;
	if (victim->fighting == NULL && !IS_NPC(victim)
	&&  !is_safe_nomessage(victim, ch)
	&&  !is_safe_nomessage(ch,victim)
	&&  (victim->position == POS_SITTING ||
	     victim->position == POS_STANDING)
	&&  !SKILL_IS(dt, "assassinate")
	&&  (sercount <= get_skill(victim, "counter"))) {
		counter = TRUE;
		check_improve(victim, "counter", TRUE, 1);
		act("$N turns your attack against you!",
		    ch, NULL, victim, TO_CHAR);
		act("You turn $n's attack against $m!",
		    ch, NULL, victim, TO_VICT);
		act("$N turns $n's attack against $m!",
		    ch, NULL, victim, TO_NOTVICT);
		ch->fighting = victim;
	}
	else if (!victim->fighting)
		check_improve(victim, "counter", FALSE, 1);

	if (SKILL_IS(dt, "backstab") && (IS_NPC(ch) || wield))
		dam = LEVEL(ch) / 11 * dam + LEVEL(ch);
	else if (SKILL_IS(dt, "dual backstab") && (IS_NPC(ch) || wield))
		dam = LEVEL(ch) / 14 * dam + LEVEL(ch);
	else if (SKILL_IS(dt, "circle"))
		dam = (LEVEL(ch)/40 + 1) * dam + LEVEL(ch);
	else if (SKILL_IS(dt, "knife"))
		dam = (LEVEL(ch)/28 + 1) * dam + LEVEL(ch);
	else if (SKILL_IS(dt, "vampiric bite"))
		dam = (LEVEL(ch)/13 + 1) * dam + LEVEL(ch);
	else if (SKILL_IS(dt, "charge"))
		dam = LEVEL(ch)/12 * dam + LEVEL(ch);
	else if (SKILL_IS(dt, "cleave") && wield != NULL) {
		if (number_percent() <
				(URANGE(4, 5 + LEVEL(ch) - LEVEL(victim), 10)
				+ (WEAPON_IS(wield, WEAPON_AXE)) ? 2 : 0 +
				(get_curr_stat(ch, STAT_STR) - 21) / 2)
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
		} else
			dam = (dam * 2 + ch->level);
	}

	if (SKILL_IS(dt, "assassinate")) {
		if (number_percent() <=
				URANGE(10, 20+(LEVEL(ch) - LEVEL(victim))*2, 50)
		&& !counter && !IS_IMMORTAL(victim)) {
			act_puts("You {R+++ASSASSINATE+++{x $N!",
				 ch, NULL, victim, TO_CHAR, POS_RESTING);
			act_puts("$n {R+++ASSASSINATES+++{x $N!",
				 ch, NULL, victim, TO_NOTVICT, POS_RESTING);
			act_puts("$n {R+++ASSASSINATES+++{x you!",
				 ch, NULL, victim, TO_VICT, POS_DEAD);
			char_puts("You have been KILLED!\n", victim);
			act("$n is DEAD!", victim, NULL, victim, TO_ROOM);
			check_improve(ch, "assassinate", TRUE, 1);
			victim->position = POS_DEAD;
			handle_death(ch, victim);
			return;
		} else {
			check_improve(ch, "assassinate", FALSE, 1);
			dam *= 2;
		}
	}

	dam += GET_DAMROLL(ch) * UMIN(100, sk) / 100;

	if (SKILL_IS(dt, "ambush"))
		dam *= UMAX(3, LEVEL(ch)/12);

	if ((sk2 = get_skill(ch, "deathblow")) > 1
	&&  !SKILL_IS(dt, "backstab")
	&&  !SKILL_IS(dt, "dual backstab")
	&&  !SKILL_IS(dt, "cleave")
	&&  !SKILL_IS(dt, "assassinate")
	&&  !SKILL_IS(dt, "ambush")
	&&  !SKILL_IS(dt, "vampiric bite")
	&&  !SKILL_IS(dt, "knife")) {
		if (number_percent() <  (sk2/8)) {
			act("You deliver a blow of deadly force!",
			    ch, NULL, NULL, TO_CHAR);
			act("$n delivers a blow of deadly force!",
			    ch, NULL, NULL, TO_ROOM);
			dam = LEVEL(ch)*dam/20;
			check_improve(ch, "deathblow", TRUE, 1);
		} else
			check_improve(ch, "deathblow", FALSE, 3);
	}

	if (dam <= 0)
		dam = 1;

	if (counter) {
		result = damage(ch, ch, 2*dam, dt, dam_class, DAMF_SHOW);
		multi_hit(victim, ch, TYPE_UNDEFINED);
	} else
		result = damage(ch, victim, dam, dt, dam_class, dam_flags);

	/* vampiric bite gives hp to ch from victim */
	if (dt == "vampiric bite") {
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

			if (chance > number_range(1, 200000)
			&&  !IS_IMMORTAL(victim)) {
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

			if ((poison = affect_find(wield->affected, "poison"))
								== NULL)
				level = wield->level;
			else
				level = poison->level;

			if (!saves_spell(level / 2,victim, DAM_POISON)) {
				act("You feel poison coursing through "
				    "your veins.", victim, NULL, NULL, TO_CHAR);
				act("$n is poisoned by the venom on $p.",
				    victim, wield, NULL, TO_ROOM);

				af.where     = TO_AFFECTS;
				af.type      = "poison";
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
	PC_DATA *vpc;
	bool vnpc = IS_NPC(victim);
	ROOM_INDEX_DATA *vroom = victim->in_room;
	bool is_duel = !IS_NPC(victim) 
		&& (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM)) 
		&& IS_SET(victim->in_room->room_flags, ROOM_BATTLE_ARENA);
	OBJ_DATA *corpse;
	class_t *cl;

	if (is_affected(victim, "resurrection")) {
		raw_kill(ch, victim);
		return;
	} 

	group_gain(ch, victim);

	/*
	 * Death trigger
	 */
	if (vnpc && HAS_TRIGGER(victim, TRIG_DEATH)) {
		victim->position = POS_STANDING;
		mp_percent_trigger(victim, ch, NULL, NULL, TRIG_DEATH);
	}

	/*
	 * IS_NPC victim is not valid after raw_kill
	 */
	raw_kill(ch, victim);

	if (!IS_NPC(ch) && vnpc && vroom == ch->in_room
	&&  (corpse = get_obj_list(ch, "corpse", ch->in_room->contents))) {
		flag32_t plr_flags = PC(ch)->plr_flags;

		if (IS_VAMPIRE(ch) && !IS_IMMORTAL(ch)) {
			act_puts("$n sucks {Rblood{x from $p!",
				 ch, corpse, NULL, TO_ROOM, POS_RESTING);
			act_puts("You suck {Rblood{x from $p!",
				 ch, corpse, NULL, TO_CHAR, POS_DEAD);
			if (vnpc) 
				gain_condition(ch, COND_BLOODLUST, 3);
			else
				gain_condition(ch, COND_BLOODLUST, 10);
		}	

		if (IS_SET(plr_flags, PLR_AUTOLOOK))
			dofun("examine", ch, "corpse");
		if (corpse->contains)
			/* corpse exists and not empty */
			if (IS_SET(plr_flags, PLR_AUTOLOOT))
				dofun("get", ch, "all corpse");
			else if (IS_SET(plr_flags, PLR_AUTOGOLD))
				get_gold_corpse(ch, corpse);

		if (IS_SET(plr_flags, PLR_AUTOSAC))
			dofun("sacrifice", ch, "corpse");
	}

	/*
	 * that's all for NPC victims
	 */
	if (vnpc || victim->position == POS_STANDING)
		return;

	if (is_duel)
		return;

	vpc = PC(victim);

	/* Dying penalty: 2/3 way back. */
	if (IS_WANTED(victim) && victim->level > 2) {
		SET_WANTED(victim, NULL);
		victim->level--;
		PC(victim)->plevels++;
		vpc->exp = exp_for_level(victim, victim->level);
		vpc->exp_tl = 0;
		wiznet("$N has died wanted, demoted to level $j.",
			victim, (const void *)victim->level, WIZ_LEVELS, 0, 0);
	}
	else  {
		if (vpc->exp_tl > 0)
			gain_exp(victim, -vpc->exp_tl*2/3);
	}

	if ((++vpc->death % 3) != 2)
		return;

	/* Die too much and is deleted ... :( */
	if ((cl = class_lookup(victim->class))
	&&  !CAN_FLEE(ch, cl)) {
		victim->perm_stat[STAT_CHA]--;
		if (vpc->death > cl->death_limit) {
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
	    int dam, const char *dt, int dam_class, int dam_flags)
{
	bool immune;
	int dam2;
	int loc;

	if (IS_EXTRACTED(victim))
		return FALSE;

	if (victim != ch) {
		/*
		 * Certain attacks are forbidden.
		 * Most other attacks are returned.
		 */
		if (victim->position > POS_STUNNED) {
			if (victim->fighting == NULL) {
				set_fighting(victim, ch);
				if (IS_NPC(victim)
				&&  HAS_TRIGGER(victim, TRIG_KILL))
					mp_percent_trigger(victim, ch, NULL,
							   NULL, TRIG_KILL);
			}
			if (!IS_NPC(victim) && PC(victim)->idle_timer <= 4)
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
	if (IS_AFFECTED(ch, AFF_HIDE | AFF_FADE | AFF_CAMOUFLAGE | AFF_BLEND |
			    AFF_INVIS | AFF_IMP_INVIS))
		dofun("visible", ch, str_empty);

	/*
	 * Damage modifiers.
	 */
	if (IS_AFFECTED(victim, AFF_SANCTUARY)
	&&  !(SKILL_IS(dt, "cleave") && (number_percent() < 50)))
		dam /= 2;

	if (IS_AFFECTED(victim, AFF_BLACK_SHROUD)) 
		dam = (4*dam)/7;

	if (IS_AFFECTED(victim, AFF_PROTECT_EVIL) && IS_EVIL(ch))
		dam -= dam / 4;

	if (IS_AFFECTED(victim, AFF_PROTECT_GOOD) && IS_GOOD(ch))
		dam -= dam / 4;

	if (is_affected(victim, "resistance"))
		dam = (3 * dam)/5;

	if (is_affected(victim, "protection heat") && (dam_class == DAM_FIRE))
		dam -= dam / 4;
	if (is_affected(victim, "protection cold") && (dam_class == DAM_COLD))
		dam -= dam / 4;

	immune = FALSE;
	loc = IS_SET(dam_flags, DAMF_SECOND) ? WEAR_SECOND_WIELD : WEAR_WIELD;

	/*
	 * Check for parry, and dodge.
	 */
	if (IS_SET(dam_flags, DAMF_HIT) && ch != victim) {
		/*
		 * some funny stuff
		 */
		if (is_affected(victim, "mirror")) {
			act("$n shatters into tiny fragments of glass.",
			    victim, NULL, NULL, TO_ROOM);
			extract_char(victim, 0);
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
		if (check_hand_block(ch, victim))
			return FALSE;
	}

	switch(check_immune(victim, dam_class)) {
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

	if (IS_SET(dam_flags, DAMF_HIT) && ch != victim) {
		if ((dam2 = critical_strike(ch, victim, dam)) != 0)
			dam = dam2;
	}

	if (IS_SET(dam_flags, DAMF_SHOW))
		dam_message(ch, victim, dam, dt, immune, dam_class, dam_flags);

	if (dam == 0)
		return FALSE;

	if (IS_SET(dam_flags, DAMF_HIT) && ch != victim)
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
		if (dam_class == DAM_HUNGER || dam_class == DAM_THIRST) break;
		act("$n is mortally wounded, and will die soon, if not aided.",
		    victim, NULL, NULL, TO_ROOM);
		char_puts( "You are mortally wounded, and will die soon, if not aided.\n", victim);
		break;

	case POS_INCAP:
		if (dam_class == DAM_HUNGER || dam_class == DAM_THIRST) break;
		act("$n is incapacitated and will slowly die, if not aided.",
		    victim, NULL, NULL, TO_ROOM);
		char_puts( "You are incapacitated and will slowly die, if not aided.\n", victim);
		break;

	case POS_STUNNED:
		if (dam_class == DAM_HUNGER || dam_class == DAM_THIRST) break;
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
		if (dam_class == DAM_HUNGER || dam_class == DAM_THIRST) break;
		if (dam > victim->max_hit / 4)
			char_puts("That really did HURT!\n", victim);
		if (victim->hit < victim->max_hit / 4)
			char_puts("You sure are BLEEDING!\n", victim);
		break;
	}

	/*
	 * Sleep spells and extremely wounded folks.
	 */
	if (!IS_AWAKE(victim) && victim->fighting)
		victim->fighting = NULL;

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
	if (!IS_NPC(victim)
	&&  victim->desc == NULL
	&&  !IS_SET(victim->comm, COMM_NOFLEE)) {
		if (number_range(0, victim->wait) == 0) {
			dofun("flee", victim, str_empty);
			return TRUE;
		}
	}

	/*
	 * Wimp out?
	 */
	if (IS_NPC(victim) && dam > 0 && victim->wait < PULSE_VIOLENCE / 2) {
		flag64_t act = victim->pMobIndex->act;
		if ((IS_SET(act, ACT_WIMPY) && number_bits(2) == 0 &&
		     victim->hit < victim->max_hit / 5)
		||  (IS_AFFECTED(victim, AFF_CHARM) &&
		     victim->master != NULL &&
		     victim->master->in_room != victim->in_room)
		||  (IS_AFFECTED(victim, AFF_DETECT_FEAR) &&
		     !IS_SET(act, ACT_NOTRACK)))
			dofun("flee", victim, str_empty);
	}

	if (!IS_NPC(victim)
	&&  victim->hit > 0
	&&  (victim->hit <= victim->wimpy || IS_AFFECTED(victim, AFF_DETECT_FEAR))
	&&  victim->wait < PULSE_VIOLENCE / 2)
		dofun("flee", victim, str_empty);

	tail_chain();
	return TRUE;
}

static bool inline
is_safe_raw(CHAR_DATA *ch, CHAR_DATA *victim)
{
	/*
	 * ghosts are safe
	 * this check must be done first to avoid
	 * suicyco muttafuckas who recite 'leather-bound book' (#5743)
	 * without any target specified
	 * extracted NPCs are safe too
	 */
	if (!IS_NPC(victim)) {
		int clan;

		/* ghost cannot attack !IS_NPC */
		if (ch != victim
		&&  !IS_NPC(ch)
		&&  IS_SET(PC(ch)->plr_flags, PLR_GHOST))
			return TRUE;

		/* clan defenders can attack anyone in their clan */
		if (victim->in_room
		&&  (clan = victim->in_room->area->clan)
		&&  victim->clan != clan
		&&  ch->clan == clan)
			return FALSE;

		/* otherwise ghosts are safe */
		if (IS_SET(PC(victim)->plr_flags, PLR_GHOST))
			return TRUE;
	}
	else if (IS_EXTRACTED(victim))
		return TRUE;

	if (victim->fighting == ch
	||  ch == victim
	||  IS_IMMORTAL(ch))
		return FALSE;

	/* handle ROOM_PEACE flags */
	if ((victim->in_room && IS_SET(victim->in_room->room_flags, ROOM_PEACE))
	||  (ch->in_room && IS_SET(ch->in_room->room_flags, ROOM_PEACE)))
		return TRUE;

	/* link dead players whose adrenalin is not gushing are safe */
	if (!IS_NPC(victim) && !IS_PUMPED(victim) && victim->desc == NULL)
		return TRUE;

	return !in_PK(ch, victim);
}

/*
 * generic safe-checking function wrapper
 *
 * all the checks are done is_safe_raw to properly strip PLR_GHOST
 * flag if victim is not safe. add you checks there
 */
bool is_safe_nomessage(CHAR_DATA *ch, CHAR_DATA *victim)
{
	bool safe;
	CHAR_DATA *mount;

	if (IS_NPC(ch)
	&&  IS_AFFECTED(ch, AFF_CHARM)
	&&  ch->master
	&&  ch->in_room == ch->master->in_room)
		return is_safe_nomessage(ch->master, victim);
	
	if (IS_NPC(victim)
	&&  IS_AFFECTED(victim, AFF_CHARM)
	&&  victim->master)
		return is_safe_nomessage(ch, victim->master);

	if (IS_NPC(victim)
	&&  IS_AFFECTED(victim, AFF_QUESTTARGET)) {
		AFFECT_DATA *paf;

		for (paf = victim->affected; paf; paf = paf->next)
			if (SKILL_IS(paf->type, "qtarget")
			&&  (ch->level > paf->modifier ||
			     ch->level < paf->level))
				return TRUE;
	}

	if (IS_NPC(ch)
	&&  IS_AFFECTED(ch, AFF_QUESTTARGET)) {
		AFFECT_DATA *paf;

		for (paf = ch->affected; paf; paf = paf->next)
			if (SKILL_IS(paf->type, "qtarget")
			&&  (victim->level < paf->modifier ||
			     victim->level < paf->level))
				return TRUE;
	}

	if ((mount = RIDDEN(victim)))
		return is_safe_nomessage(ch, mount);

	if ((safe = is_safe_raw(ch, victim)) || IS_NPC(ch))
		return safe;

	if (victim != ch
	&&  !IS_NPC(ch)
	&&  IS_SET(PC(ch)->plr_flags, PLR_GHOST)) {
		char_puts("You return to your normal form.\n", ch);
		REMOVE_BIT(PC(ch)->plr_flags, PLR_GHOST);
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
	OBJ_DATA *v_weapon;
	OBJ_DATA *ch_weapon;

	if (!IS_AWAKE(victim))
		return FALSE;
	
	v_weapon = get_eq_char(victim, WEAR_WIELD);
	if (IS_NPC(victim))
		chance	= UMIN(35, victim->level);
	else {
		if (v_weapon == NULL)
			return FALSE;

		chance = get_skill(victim, "parry") / 2;
	}
	
	ch_weapon = get_eq_char(ch, loc);
	
	if (v_weapon) {
		switch (INT_VAL(v_weapon->value[1])) {
		case WEAPON_WHIP:
		case WEAPON_FLAIL:
			chance /= 2;
			break;

		case WEAPON_SWORD:
			if (number_percent() < get_skill(victim, "fence") / 2) {
				chance = chance * 3 / 2;
				check_improve(victim, "fence", TRUE, 4);
			}
			break;
		}
	}

	if (ch_weapon && WEAPON_IS(ch_weapon, WEAPON_SWORD)) {
		if (number_percent() < get_skill(ch, "fence")) {
			chance /= 2;
			check_improve(ch, "fence", TRUE, 7);
		}
	}

	if (check_forest(victim) == FOREST_DEFENCE
	&&  (number_percent() < get_skill(victim, "forest fighting"))) {
		chance = chance * 120 / 100;
		check_improve (victim, "forest fighting", TRUE, 7);
	}

	if (number_percent() >= chance + LEVEL(victim) - LEVEL(ch))
		return FALSE;

	act("You parry $n's attack.", ch, NULL, victim, TO_VICT | ACT_VERBOSE);
	act("$N parries your attack.", ch, NULL, victim, TO_CHAR | ACT_VERBOSE);

	check_weapon_damage(ch, victim, loc);

	if (number_percent() > chance) {
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
		if (IS_NPC(ch) && IS_SET(ch->pMobIndex->off_flags, OFF_FAST))
			chance += 10;
		if (IS_NPC(victim) && IS_SET(victim->pMobIndex->off_flags,
					     OFF_FAST))
			chance -= 20;

		/* level */
		chance += (LEVEL(ch) - LEVEL(victim)) * 2;

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

			WAIT_STATE(victim, skill_beats("bash"));
			victim->position = POS_RESTING;
		}
	}

	check_improve(victim, "parry", TRUE, 6);
	return TRUE;
}

/*
 * check blink
 */
bool check_blink(CHAR_DATA *ch, CHAR_DATA *victim)
{
	int chance;

	if (!is_affected(victim, "blink"))
		return FALSE;

	if (IS_NPC(victim))
		return FALSE;
	else
		chance	= get_skill(victim, "blink") / 2;

	if (number_percent() >= chance + LEVEL(victim) - LEVEL(ch)
	||  number_percent() < 50
	||  victim->mana < 10)
		return FALSE;

	victim->mana -= UMAX(victim->level / 10, 1);

	act("You blink out $n's attack.",
	    ch, NULL, victim, TO_VICT | ACT_VERBOSE);
	act("$N blinks out your attack.",
	    ch, NULL, victim, TO_CHAR | ACT_VERBOSE);
	check_improve(victim, "blink", TRUE, 6);
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
		chance = get_skill(victim, "shield block") / 2;
		if (chance <= 1)
			return FALSE;
	}	

	if (check_forest(victim) == FOREST_DEFENCE 
	&& (number_percent() < get_skill(victim, "forest fighting"))) {
		chance *= 1.2;
		check_improve (victim, "forest fighting", TRUE, 7);
	}

	if (MOUNTED(victim))
		chance *= 1.2;


	if (number_percent() >= chance + LEVEL(victim) - LEVEL(ch))
		return FALSE;

	act("Your shield blocks $n's attack.",
	    ch, NULL, victim, TO_VICT | ACT_VERBOSE);
	act("$N deflects your attack with $S shield.",
	    ch, NULL, victim, TO_CHAR | ACT_VERBOSE);
	check_shield_damage(ch, victim, loc);
	check_improve(victim, "shield block", TRUE, 6);
	return TRUE;
}

/* 
 * Check for hand block
 */

bool check_hand_block(CHAR_DATA *ch, CHAR_DATA *victim)
{
	int chance;

	if (!IS_AWAKE(victim) 
	|| IS_NPC(victim) 
	|| get_eq_char(victim, WEAR_WIELD)
	|| get_eq_char(victim, WEAR_SECOND_WIELD)
	|| (chance = get_skill(victim, "hand block") == 0)) 
		return FALSE;

	chance = URANGE(5, chance*2/5 + LEVEL(victim) - LEVEL(ch), 55);

	if (number_percent() < chance) {
		act("Your hand blocks $n's attack.", 
			ch, NULL, victim, TO_VICT|ACT_VERBOSE);
		act("$N blocks your attack with $S hand.",
			ch, NULL, victim, TO_CHAR|ACT_VERBOSE);
		check_improve(victim, "hand block", TRUE, 5);
		return TRUE;
	}
	return FALSE;
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
		chance  = get_skill(victim, "dodge") / 2;
		/* chance for high dex. */
		chance += 2 * (get_curr_stat(victim,STAT_DEX) - 20);
	}
	
	if (check_forest(victim) == FOREST_DEFENCE 
	  && (get_skill(victim, "forest fighting") > number_percent())) {
		chance *= 1.2;
		check_improve (victim, "forest fighting", TRUE, 7);
	}

	if (number_percent() >= chance + (victim->level - ch->level) / 2)
		return FALSE;

	act("You dodge $n's attack.", ch, NULL, victim, TO_VICT | ACT_VERBOSE);
	act("$N dodges your attack.", ch, NULL, victim, TO_CHAR	| ACT_VERBOSE);

	if (number_percent() < get_skill(victim, "dodge") / 20
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
		if ((IS_NPC(victim) && IS_SET(victim->pMobIndex->off_flags,
					      OFF_FAST))
		||  IS_AFFECTED(victim, AFF_HASTE))
			chance += 10;
		if ((IS_NPC(ch) && IS_SET(ch->pMobIndex->off_flags, OFF_FAST))
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

			WAIT_STATE(ch, skill_beats("trip"));
			ch->position = POS_RESTING;
		}
	}
	check_improve(victim, "dodge", TRUE, 6);
	return TRUE;
}

/*
 * Set position of a victim.
 */
void update_pos(CHAR_DATA *victim)
{
	if (victim->hit > 0) {
		if (victim->position <= POS_STUNNED) {
			if (IS_AFFECTED(victim, AFF_SLEEP)) {
				REMOVE_BIT(victim->affected_by, AFF_SLEEP);
				affect_bit_strip(victim, TO_AFFECTS, AFF_SLEEP);
			}

			victim->position = POS_STANDING;
		}
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

	if (IS_AFFECTED(ch, AFF_SLEEP)) {
		REMOVE_BIT(ch->affected_by, AFF_SLEEP);
		affect_bit_strip(ch, TO_AFFECTS, AFF_SLEEP);
	}

	ch->fighting = victim;
	ch->position = POS_FIGHTING;
}

static void STOP_FIGHTING(CHAR_DATA *ch)
{
	ch->fighting = NULL;
	ch->position = IS_NPC(ch) ? ch->pMobIndex->default_pos : POS_STANDING;
	update_pos(ch);
}

/*
 * Stop fights.
 */
void stop_fighting(CHAR_DATA *ch, bool fBoth)
{
	CHAR_DATA *fch;

	STOP_FIGHTING(ch);
	if (!fBoth)
		return;

	for (fch = char_list; fch; fch = fch->next) {
		if (fch->fighting == ch)
			STOP_FIGHTING(fch);
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

	if (IS_NPC(ch)) {
		corpse	= create_obj_of(get_obj_index(OBJ_VNUM_CORPSE_NPC),
					&ch->short_descr);
		corpse->timer	= number_range(3, 6);
		if (ch->gold > 0 || ch->silver > 0) {
			OBJ_DATA *money = create_money(ch->gold, ch->silver);
			if (IS_SET(ch->form,FORM_INSTANT_DECAY))
				obj_to_room(money, ch->in_room);
			else
				obj_to_obj(money, corpse);
		}
	}
	else {
		corpse	= create_obj_of(get_obj_index(OBJ_VNUM_CORPSE_PC),
					&ch->short_descr);

		corpse->timer= number_range(25, 40);
		corpse->altar = get_altar(ch);

		if (ch->gold > 0 || ch->silver > 0)
			obj_to_obj(create_money(ch->gold, ch->silver), corpse);
	}

	mlstr_cpy(&corpse->owner, &ch->short_descr);
	corpse->level = ch->level;

	ch->gold = 0;
	ch->silver = 0;

	for (obj = ch->carrying; obj != NULL; obj = obj_next) {
		obj_next = obj->next_content;
		obj_from_char(obj);
		if (obj->pObjIndex->item_type == ITEM_POTION)
		    obj->timer = number_range(500,1000);
		if (obj->pObjIndex->item_type == ITEM_SCROLL)
		    obj->timer = number_range(1000,2500);
		if (IS_SET(obj->extra_flags,ITEM_ROT_DEATH))  {
		    obj->timer = number_range(5,10);
		    if (obj->pObjIndex->item_type == ITEM_POTION)
		       obj->timer += obj->level * 20;
		}
		REMOVE_BIT(obj->extra_flags,ITEM_VIS_DEATH);
		REMOVE_BIT(obj->extra_flags,ITEM_ROT_DEATH);

		if (IS_SET(obj->extra_flags, ITEM_INVENTORY)  ||
		    (obj->pObjIndex->limit != -1 &&
			(obj->pObjIndex->count > obj->pObjIndex->limit)))
		  {
		    extract_obj(obj, 0);
		    continue;
		  }
		else if (IS_SET(ch->form,FORM_INSTANT_DECAY))
		  obj_to_room(obj, ch->in_room);

		else
		  obj_to_obj(obj, corpse);
	}

	obj_to_room(corpse, ch->in_room);
}


/*
 * Improved Death_cry contributed by Diavolo.
 */
void death_cry(CHAR_DATA *ch)
{
	ROOM_INDEX_DATA *was_in_room;
	char *msg;
	int door;
	int vnum;

	vnum = 0;
	msg = "You hear $n's death cry.";


	switch (number_bits(4)) {
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

		obj = create_obj_of(get_obj_index(vnum), &ch->short_descr);
		obj->level = ch->level;
		mlstr_cpy(&obj->owner, &ch->short_descr);
		obj->timer = number_range(4, 7);

		if (obj->pObjIndex->item_type == ITEM_FOOD) {
			if (IS_SET(ch->form,FORM_POISON))
				obj->value[3] = 1;
			if (IS_SET(ch->form, FORM_MAGICAL))
				SET_BIT(obj->extra_flags, ITEM_MAGIC);
			if (!IS_SET(ch->form,FORM_EDIBLE))
				SET_BIT(obj->extra_flags, ITEM_NOT_EDIBLE);
		}

		obj_to_room(obj, ch->in_room);
	}

	if (IS_NPC(ch))
		msg = "You hear something's death cry.";
	else
		msg = "You hear someone's death cry.";

	if ((was_in_room = ch->in_room)) {
		for (door = 0; door <= 5; door++) {
			EXIT_DATA *pexit;

			if ((pexit = was_in_room->exit[door]) != NULL
			&&   pexit->to_room.r != NULL
			&&   pexit->to_room.r != was_in_room) {
				ch->in_room = pexit->to_room.r;
				act(msg, ch, NULL, NULL, TO_ROOM);
			}
		}
		ch->in_room = was_in_room;
	}
}

void raw_kill(CHAR_DATA *ch, CHAR_DATA *victim)
{
	CHAR_DATA *vch, *vch_next;
	OBJ_DATA *obj, *obj_next;
	int i;
	OBJ_DATA *tattoo, *clanmark;

	if (is_affected(victim, "resurrection")) {
		act_puts("Yess! Your Great Master resurrects you!",
			 victim, NULL, NULL, TO_CHAR, POS_DEAD);
		act("Ouch! Beast stands and fight again, with new power!",
		    victim, NULL, NULL, TO_ROOM);
		act("$n giggles.", victim, NULL, NULL, TO_ROOM);
		gain_condition(ch, COND_BLOODLUST, 20);
		affect_strip(victim, "resurrection");
		if (victim->perm_stat[STAT_CHA] > 3)
			victim->perm_stat[STAT_CHA]--;
		victim->hit	= victim->max_hit;
		victim->mana	= victim->max_mana;
		victim->move	= victim->max_move;
		update_pos(victim);
	        if (!saves_spell(victim->level,ch,DAM_NEGATIVE)) {
			char_puts("Your muscles stop responding.\n", ch);
			DAZE_STATE(ch, victim->level);
		}
		return;
	}

	for (obj = victim->carrying; obj != NULL; obj = obj_next) {
		obj_next = obj->next_content;
		if (obj->wear_loc != WEAR_NONE
		&&  oprog_call(OPROG_DEATH, obj, victim, NULL)) {
			victim->position = POS_STANDING;
			return;
		}
	}

	if (IS_NPC(victim))
		quest_handle_death(ch, victim);
	else
		rating_update(ch, victim);

	stop_fighting(victim, TRUE);
	RESET_FIGHT_TIME(victim);
	victim->last_death_time = current_time;
	death_cry(victim);

	tattoo = get_eq_char(victim, WEAR_TATTOO);
	clanmark = get_eq_char(victim, WEAR_CLANMARK);
	if (tattoo != NULL)
		obj_from_char(tattoo);
	if (clanmark != NULL)
		obj_from_char(clanmark);
	make_corpse(victim);

	/*
	 * don't remember killed victims anymore
	 * `ch' might be extracted and should not be used further
	 */
	if (IS_NPC(ch))
		remove_mind(ch, victim->name);

	/*
	 * that's all for NPC victims
	 */
	if (IS_NPC(victim)) {
		if (tattoo != NULL)
			extract_obj(tattoo, 0);
		if (clanmark != NULL)
			extract_obj(clanmark, 0);
		victim->pMobIndex->killed++;
		extract_char(victim, 0);
		return;
	}

	SET_BIT(PC(victim)->plr_flags, PLR_GHOST);
	char_puts("You turn into an invincible ghost for a few minutes.\n"
		  "As long as you don't attack anything.\n",
		  victim);

	extract_char(victim, XC_F_INCOMPLETE);

	while (victim->affected)
		affect_remove(victim, victim->affected);
	victim->affected_by	= 0;
	for (i = 0; i < 4; i++)
		victim->armor[i] = 100;
	victim->position	= POS_RESTING;
	victim->hit		= victim->max_hit / 10;
	victim->mana		= victim->max_mana / 10;
	victim->move		= victim->max_move;
	update_pos(victim);

	/* RT added to prevent infinite deaths */
	REMOVE_BIT(PC(victim)->plr_flags, PLR_BOUGHT_PET);

	PC(victim)->condition[COND_THIRST] = 40;
	PC(victim)->condition[COND_HUNGER] = 40;
	PC(victim)->condition[COND_FULL] = 40;
	PC(victim)->condition[COND_BLOODLUST] = 40;
	PC(victim)->condition[COND_DESIRE] = 40;

	if (tattoo != NULL) {
		obj_to_char(tattoo, victim);
		equip_char(victim, tattoo, WEAR_TATTOO);
	}

	if (clanmark != NULL) {
		obj_to_char(clanmark, victim);
		equip_char(victim, clanmark, WEAR_CLANMARK);
	}

	char_save(victim, 0);

	/*
	 * Calm down the tracking mobiles
	 */
	for (vch = npc_list; vch; vch = vch_next) {
		vch_next = vch->next;

		if (NPC(vch)->target == victim) {
			if (vch->pMobIndex->vnum == MOB_VNUM_SHADOW) {
				act("$n slowly fades away.",
				    vch, NULL, NULL, TO_ROOM);
				extract_char(vch, 0);
				continue;
			}

			if (vch->pMobIndex->vnum == MOB_VNUM_STALKER) {
				act_clan(vch, "$i is dead and I can leave the realm.", victim);
				act("$n slowly fades away.",
				    vch, NULL, NULL, TO_ROOM);
				extract_char(vch, 0);
				continue;
			}
		}

		if (NPC(vch)->last_fought == victim)
			NPC(vch)->last_fought = NULL;
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

	if (IS_SET(victim->pMobIndex->act, ACT_PET)
	||  victim->pMobIndex->vnum < 100
	||  victim->master
	||  victim->leader)
		return;

	lch = leader_lookup(ch);

	members = 0;
	group_levels = 0;
	for (gch = ch->in_room->people; gch; gch = gch->next_in_room) {
		if (is_same_group(gch, ch)) {
			if (IS_NPC(gch)) {
				if (IS_SET(gch->pMobIndex->act, ACT_SUMMONED))
					continue;
			}
			else {
				if (abs(gch->level - lch->level) <= 8)
					members++;
			}
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
		if (gch->level < LEVEL_HERO) {
			char_printf(gch, "You receive %d experience points.\n", xp);
			gain_exp(gch, xp);
		}
	}
}

/*
 * Compute xp for a kill.
 * Also adjust alignment of killer.
 * Edit this function to change xp computations.
 *
 * gch is assumed to be !IS_NPC
 */
int xp_compute(CHAR_DATA *gch, CHAR_DATA *victim, int total_levels, int members)
{
	PC_DATA *pc = PC(gch);
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
	if (IS_NPC(victim) && IS_SET(victim->pMobIndex->act, ACT_NOALIGN))
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
			pc->anti_killed++;
			neg_cha = 1;
		}
		else if (IS_NEUTRAL(victim)) {
			pc->has_killed++;
			pos_cha = 1;
		}
		else if (IS_EVIL(victim)) {
			pc->has_killed++;
			pos_cha = 1;
		}
	}
	else if (IS_NEUTRAL(gch)) {
		if (IS_GOOD(victim)) {
			pc->has_killed++;
			pos_cha = 1;
		}
		else if (IS_NEUTRAL(victim)) {
			pc->anti_killed++;
			neg_cha = 1;
		}
		else if (IS_EVIL(victim)) {
			pc->has_killed++;
			pos_cha =1;
		}
	}
	else if (IS_EVIL(gch)) {
		if (IS_GOOD(victim)) {
			pc->has_killed++;
			pos_cha = 1;
		}
		else if (IS_NEUTRAL(victim)) {
			pc->has_killed++;
			pos_cha = 1;
		}
		else if (IS_EVIL(victim)) {
			pc->anti_killed++;
			neg_cha = 1;
		}
	}

	if (neg_cha) {
		if ((pc->anti_killed % 100) == 99) {
			char_printf(gch, "You have killed %d %s up to now.\n",
				    pc->anti_killed,
				    IS_GOOD(gch) ?	"goods" :
				    IS_EVIL(gch) ?	"evils" :
							"neutrals");
			if (gch->perm_stat[STAT_CHA] > 3 && IS_GOOD(gch)) {
				char_puts("So your charisma "
					  "has reduced by one.\n", gch);
				gch->perm_stat[STAT_CHA] -= 1;
			}
		}
	} else if (pos_cha) {
		if ((PC(gch)->has_killed % 200) == 199) {
			char_printf(gch, "You have killed %d %s up to now.\n",
				    pc->anti_killed,
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

void dam_message(CHAR_DATA *ch, CHAR_DATA *victim, int dam,
		 const char *dt, bool immune, int dam_class, int dam_flags)
{
	const char *vs;
	const char *vp;
	const char *msg_char;
	const char *msg_vict = NULL;
	const char *msg_notvict;
	const char *dam_noun = str_empty;

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

	if ((IS_SET(dam_flags, DAMF_HIT) && dam_class == DAM_NONE)
	||  IS_SET(dam_flags, DAMF_HUNGER)) {
		if (ch == victim) {
			switch (dam_class) {
			case DAM_HUNGER:
				vs = vp;
				msg_notvict = "$n's hunger $u $mself!";
				msg_char = "Your hunger $u yourself!";
				break;

			case DAM_THIRST:
				vs = vp;
				msg_notvict = "$n's thirst $u $mself!";
				msg_char = "Your thirst $u yourself!";
				break;

			case DAM_LIGHT_V:
				vs = vp;
				msg_notvict = "The light of room $u $n!";
				msg_char = "The light of room $u you!";
				break;

			case DAM_TRAP_ROOM:
				vs = vp;
				msg_notvict = "The trap at room $u $n!";
				msg_char = "The trap at room $u you!";
				break;

			default:
				msg_notvict = "$n $u $mself!";
				msg_char = "You $u yourself!";
				break;
			}
		} else {
			msg_notvict = "$n $u $N.";
			msg_char = "You $u $N.";
			msg_vict = "$n $u you.";
		}
	} else {
		if (IS_SET(dam_flags, DAMF_HIT))
			dam_noun = damtype_noun(dt);
		else {
			skill_t *sk = skill_lookup(dt);
			dam_noun = sk ? sk->noun_damage : "hit";
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
		} else {
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
		act_puts3(msg_notvict, ch, vp, NULL, dam_noun,
			  TO_ROOM, POS_RESTING);
		act_puts3(msg_char, ch, vs, NULL, dam_noun,
			  TO_CHAR, POS_RESTING);
	} else {
		act_puts3(msg_notvict, ch, vp, victim, dam_noun,
			  TO_NOTVICT, POS_RESTING);
		act_puts3(msg_char, ch, vs, victim, dam_noun,
			  TO_CHAR, POS_RESTING);
		act_puts3(msg_vict, ch, vp, victim, dam_noun,
			  TO_VICT, POS_RESTING);
	}
}

/*
 * Check for obj dodge.
 */
bool check_obj_dodge(CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj, int bonus)
{
	int chance;

	if (!IS_AWAKE(victim) || MOUNTED(victim))
		return FALSE;

	if (!IS_NPC(victim) && HAS_SKILL(victim, "spellbane")) {
		if (PC(victim)->clan_status) {
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
		chance  = get_skill(victim, "dodge") / 2;
		/* chance for high dex. */
		chance += 2 * (get_curr_stat(victim, STAT_DEX) - 20);
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
	check_improve(victim, "dodge", TRUE, 6);

	return TRUE;
}

/*  
 * critical strike
 */
int critical_strike(CHAR_DATA *ch, CHAR_DATA *victim, int dam)
{
	int diceroll;
	AFFECT_DATA baf;
	int chance;

	if (get_eq_char(ch, WEAR_WIELD) != NULL 
	&&  get_eq_char(ch, WEAR_SECOND_WIELD) != NULL
	&&  number_percent() > ((ch->hit * 100) / ch->max_hit)) 
		return 0;

	if ((chance = get_skill(ch, "critical strike")) == 0)
		return dam;
	
	diceroll = number_range(0, 100);
	if (LEVEL(victim) > LEVEL(ch))
		diceroll += (LEVEL(victim) - LEVEL(ch)) * 2;
	else
		diceroll -= (LEVEL(ch) - LEVEL(victim));
 
	if (diceroll <= chance /2)  {  
		check_improve(ch, "critical strike", TRUE, 2);
		dam += dam * diceroll/200;
	}  

	if (diceroll > chance / 13)
		return dam;

	diceroll = number_percent();
	if (diceroll <= 75) {  
		act_puts("You take $N down with a weird judo move!", 
			 ch, NULL, victim, TO_CHAR, POS_DEAD);
		act("$n takes you down with a weird judo move!", 
		    ch, NULL, victim, TO_VICT);
		act("$n takes $N down with a weird judo move!", 
		    ch, NULL, victim, TO_NOTVICT);
		check_improve(ch, "critical strike", TRUE, 3);
		WAIT_STATE(victim, 2 * PULSE_VIOLENCE);
		dam += (dam * number_range(2, 5)) / 5;
		return dam;
	} else if (diceroll > 75 && diceroll < 95) {   
		act_puts("You blind $N with your attack!",
			 ch, NULL, victim, TO_CHAR, POS_DEAD);
		act("You are blinded by $n's attack!",
		    ch, NULL, victim, TO_VICT);
		act("$N is blinded by $n's attack!",
		    ch, NULL, victim, TO_NOTVICT);
		check_improve(ch, "critical strike", TRUE, 4);
		if (!IS_AFFECTED(victim, AFF_BLIND)) {
			baf.where = TO_AFFECTS;
			baf.type = "critical strike";
			baf.level = ch->level; 
			baf.location = APPLY_HITROLL; 
			baf.modifier = -4;
			baf.duration = number_range(1, 3); 
			baf.bitvector = AFF_BLIND;
			affect_to_char(victim, &baf);
		}  
		dam += dam * number_range(1, 2);			
		return dam;
	} 

	act_puts("You cut out $N's {Rheart{x! I bet that hurt!",  
		 ch, NULL, victim, TO_CHAR ,POS_RESTING);
	act_puts("$n cuts out your {Rheart{x! OUCH!!",  
		 ch, NULL, victim, TO_VICT ,POS_RESTING); 
	act_puts("$n cuts out $N's {Rheart{x! I bet that hurt!",  
		 ch, NULL, victim, TO_NOTVICT ,POS_RESTING); 
	check_improve(ch, "critical strike", TRUE, 5);
	dam += dam * number_range(2, 5);			
	return dam;
}  

