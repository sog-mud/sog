/*
 * $Id: martial_art.c,v 1.43 1998-10-08 13:27:54 fjoe Exp $
 */

/***************************************************************************
 *     ANATOLIA 2.1 is copyright 1996-1997 Serdar BULUT, Ibrahim CANPUNAR  *	
 *     ANATOLIA has been brought to you by ANATOLIA consortium		   *
 *	 Serdar BULUT {Chronos}		bulut@rorqual.cc.metu.edu.tr       *
 *	 Ibrahim Canpunar  {Asena}	canpunar@rorqual.cc.metu.edu.tr    *	
 *	 Murat BICER  {KIO}		mbicer@rorqual.cc.metu.edu.tr	   *	
 *	 D.Baris ACAR {Powerman}	dbacar@rorqual.cc.metu.edu.tr	   *	
 *     By using this code, you have agreed to follow the terms of the      *
 *     ANATOLIA license, in the file Anatolia/anatolia.licence             *	
 ***************************************************************************/

/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
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
#include <stdio.h>
#include "merc.h"
#include "fight.h"
#include "interp.h"

#ifdef SUNOS
#	include <stdarg.h>
#	include "compat/compat.h"
#endif

void	one_hit		(CHAR_DATA *ch, CHAR_DATA *victim,
			 int dt, bool secondary); 
void	set_fighting	(CHAR_DATA *ch, CHAR_DATA *victim);

/*
 * Disarm a creature.
 * Caller must check for successful attack.
 */
void disarm(CHAR_DATA *ch, CHAR_DATA *victim ,int disarm_second)
{
	OBJ_DATA *obj;
	OBJ_DATA *obj2;
	int skill;

	if (disarm_second) {
		if ((obj = get_eq_char(victim, WEAR_SECOND_WIELD)) == NULL) {
			bug("Disarm second with NULL WEAR_SECOND_WIELD", 0);
			return;
		}
	}
	else {
		if ((obj = get_eq_char(victim, WEAR_WIELD)) == NULL) {
			bug("Disarm first with NULL WEAR_WIELD",0);
			return;
		}
	}

	if (IS_OBJ_STAT(obj, ITEM_NOREMOVE)) {
		act("$S weapon won't budge!", ch, NULL, victim, TO_CHAR);
		act("$n tries to disarm you, but your weapon won't budge!",
		    ch, NULL, victim, TO_VICT);
		act("$n tries to disarm $N, but fails.",
		    ch, NULL, victim, TO_NOTVICT);
		return;
	}

	if ((skill = get_skill(victim, gsn_grip)) != 0) {
		skill += (get_curr_stat(victim,STAT_STR) -
			  get_curr_stat(ch,STAT_STR)) * 5;
		if (number_percent() < skill) {
			act("$N grips and prevent you to disarm $S!",
			    ch, NULL, victim, TO_CHAR);
			act("$n tries to disarm you, but you grip and escape!",
			    ch, NULL, victim, TO_VICT);
			act("$n tries to disarm $N, but fails.",
			    ch, NULL, victim, TO_NOTVICT);
			check_improve(victim, gsn_grip, TRUE, 1);
			return;
		}
		else
			check_improve(victim,gsn_grip,FALSE,1);
	}

	act_puts("$n DISARMS you and sends your weapon flying!", 
		 ch, NULL, victim, TO_VICT, POS_FIGHTING);
	act_puts("You disarm $N!", ch,NULL, victim, TO_CHAR, POS_FIGHTING);
	act_puts("$n disarms $N!", ch, NULL, victim, TO_NOTVICT, POS_FIGHTING);

	obj_from_char(obj);
	if (IS_OBJ_STAT(obj,ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_INVENTORY))
		obj_to_char(obj, victim);
	else {
		obj_to_room(obj, victim->in_room);
		if (IS_NPC(victim)
		&&  victim->wait == 0
		&&  can_see_obj(victim,obj))
			get_obj(victim, obj, NULL);
	}

	if ((obj2 = get_eq_char(victim, WEAR_SECOND_WIELD)) != NULL) {
		act_puts("You wield your second weapon as your first!.",
			 ch, NULL, victim, TO_VICT, POS_FIGHTING);
		act_puts("$N wields his second weapon as first!",
			 ch, NULL, victim, TO_CHAR, POS_FIGHTING);
		act_puts("$N wields his second weapon as first!",
			 ch, NULL, victim, TO_NOTVICT, POS_FIGHTING);
		unequip_char(victim, obj2);
		equip_char(victim, obj2 , WEAR_WIELD);
	}
}

void do_berserk(CHAR_DATA *ch, const char *argument)
{
	int chance, hp_percent;

	if ((chance = get_skill(ch, gsn_berserk)) == 0) {
		char_puts("You turn red in the face, but nothing happens.\n\r",
			  ch);
		return;
	}

	if (IS_AFFECTED(ch, AFF_BERSERK)
	||  is_affected(ch, gsn_berserk)
	||  is_affected(ch, gsn_frenzy)) {
		char_puts("You get a little madder.\n\r", ch);
		return;
	}

	if (IS_AFFECTED(ch, AFF_CALM)) {
		char_puts("You're feeling too mellow to berserk.\n\r", ch);
		return;
	}

	if (ch->mana < 50) {
		char_puts("You can't get up enough energy.\n\r", ch);
		return;
	}

	/* modifiers */

	/* fighting */
	if (ch->position == POS_FIGHTING)
		chance += 10;

	/* damage -- below 50% of hp helps, above hurts */
	hp_percent = 100 * ch->hit/ch->max_hit;
	chance += 25 - hp_percent/2;

	if (number_percent() < chance) {
		AFFECT_DATA af;

		WAIT_STATE(ch,PULSE_VIOLENCE);
		ch->mana -= 50;
		ch->move /= 2;

		/* heal a little damage */
		ch->hit += ch->level * 2;
		ch->hit = UMIN(ch->hit,ch->max_hit);

		char_puts("Your pulse races as you are consumned by rage!\n\r",
			  ch);
		act_puts("$n gets a wild look in $s eyes.",
			 ch, NULL, NULL, TO_ROOM, POS_FIGHTING);
		check_improve(ch, gsn_berserk, TRUE, 2);

		af.where	= TO_AFFECTS;
		af.type		= gsn_berserk;
		af.level	= ch->level;
		af.duration	= number_fuzzy(ch->level / 8);
		af.modifier	= UMAX(1,ch->level/5);
		af.bitvector 	= AFF_BERSERK;

		af.location	= APPLY_HITROLL;
		affect_to_char(ch,&af);

		af.location	= APPLY_DAMROLL;
		affect_to_char(ch,&af);

		af.modifier	= UMAX(10,10 * (ch->level/5));
		af.location	= APPLY_AC;
		affect_to_char(ch,&af);
	}
	else {
		WAIT_STATE(ch,2 * PULSE_VIOLENCE);
		ch->mana -= 25;
		ch->move /= 2;

		char_puts("Your pulse speeds up, but nothing happens.\n\r",
			  ch);
		check_improve(ch, gsn_berserk, FALSE, 2);
	}
}

void do_bash(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int chance, wait;
	int damage_bash;

	if (MOUNTED(ch)) {
		char_puts("You can't bash while riding!\n\r", ch);
		return;
	}

	argument = one_argument(argument,arg);
 
	if ((chance = get_skill(ch, gsn_bash)) == 0) {
		char_puts("Bashing? What's that?\n\r", ch);
		return;
	}
 
	if (arg[0] != '\0' && !str_cmp(arg,"door")) {
		do_bash_door(ch, argument);
		return;
	}

	WAIT_STATE(ch, SKILL(gsn_bash)->beats);

	if (arg[0] == '\0') {
		victim = ch->fighting;
		if (victim == NULL) {
			char_puts("But you aren't fighting anyone!\n\r",ch);
			return;
		}
	}
	else if ((victim = get_char_room(ch, arg)) == NULL) {
		char_puts("They aren't here.\n\r",ch);
		return;
	}

	if (victim->position < POS_FIGHTING) {
		act("You'll have to let $M get back up first.",
			  ch, NULL, victim, TO_CHAR);
		return;
	} 

	if (victim == ch) {
		char_puts("You try to bash your brains out, but fail.\n\r",
			     ch);
		return;
	}

	if (MOUNTED(victim)) {
		char_puts("You can't bash a riding one!\n\r", ch);
		return;
	}

	if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim) {
		act("But $N is your friend!", ch, NULL, victim, TO_CHAR);
		return;
	}
		
	if (is_affected(victim, gsn_protective_shield)) {
		act_puts("Your bash seems to slide around $N.",
			 ch, NULL, victim, TO_CHAR, POS_FIGHTING);
		act_puts("$n's bash slides off your protective shield.",
			 ch, NULL, victim, TO_VICT, POS_FIGHTING);
		act_puts("$n's bash seems to slide around $N.",
			 ch, NULL, victim, TO_NOTVICT, POS_FIGHTING);
		return;
	}

	if (is_safe(ch, victim))
		return;

	/* modifiers */

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
	if (IS_SET(ch->off_flags, OFF_FAST))
		chance += 10;
	if (IS_SET(victim->off_flags, OFF_FAST))
		chance -= 20;

	/* level */
	chance += (ch->level - victim->level) * 2;

	RESET_WAIT_STATE(ch);

	/* now the attack */
	if (number_percent() < chance) {
		act("$n sends you sprawling with a powerful bash!",
		    ch, NULL, victim, TO_VICT);
		act("You slam into $N, and send $M flying!",
		    ch, NULL, victim, TO_CHAR);
		act("$n sends $N sprawling with a powerful bash.",
		    ch, NULL, victim, TO_NOTVICT);
		check_improve(ch, gsn_bash, TRUE, 1);

		wait = 3;

		switch(number_bits(2)) {
			case 0: wait = 1; break;
			case 1: wait = 2; break;
			case 2: wait = 4; break;
			case 3: wait = 3; break;
		}

		WAIT_STATE(victim, wait * PULSE_VIOLENCE);
		WAIT_STATE(ch, SKILL(gsn_bash)->beats);
		victim->position = POS_RESTING;
		damage_bash = (ch->damroll / 2) +
			      number_range(4, 4 + 4* ch->size + chance/10);
		damage(ch, victim, damage_bash, gsn_bash, DAM_BASH, TRUE);
	}
	else {
		damage(ch, victim, 0, gsn_bash, DAM_BASH, TRUE);
		act("You fall flat on your face!",
		    ch, NULL, victim, TO_CHAR);
		act("$n falls flat on $s face.",
		    ch, NULL, victim, TO_NOTVICT);
		act("You evade $n's bash, causing $m to fall flat on $s face.",
		    ch, NULL, victim, TO_VICT);
		check_improve(ch, gsn_bash, FALSE, 1);
		ch->position = POS_RESTING;
		WAIT_STATE(ch, SKILL(gsn_bash)->beats * 3/2); 
	}
}

void do_dirt(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int chance;

	if (MOUNTED(ch)) {
		char_puts("You can't dirt while riding!\n\r", ch);
		return;
	}

	one_argument(argument,arg);

	if ((chance = get_skill(ch, gsn_dirt)) == 0) {
		char_puts("You get your feet dirty.\n\r", ch);
		return;
	}

	WAIT_STATE(ch, SKILL(gsn_dirt)->beats);

	if (arg[0] == '\0') {
		victim = ch->fighting;
		if (victim == NULL) {
			char_puts("But you aren't in combat!\n\r", ch);
			return;
		}
	}
	else if ((victim = get_char_room(ch, arg)) == NULL) {
		char_puts("They aren't here.\n\r", ch);
		return;
	}

	if (IS_AFFECTED(ch, AFF_FLYING)) {
		 char_puts("While flying?\n\r", ch);
		 return;
	}

	if (IS_AFFECTED(victim, AFF_BLIND)) {
		act("$e's already been blinded.", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (victim == ch) {
		char_puts("Very funny.\n\r", ch);
		return;
	}

	if (MOUNTED(victim)) {
		char_puts("You can't dirt a riding one!\n\r", ch);
		return;
	}

	if (IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim) {
		act("But $N is such a good friend!", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (is_safe(ch, victim))
		return;

	/* modifiers */

	/* dexterity */
	chance += get_curr_stat(ch, STAT_DEX);
	chance -= 2 * get_curr_stat(victim, STAT_DEX);

	/* speed  */
	if (IS_SET(ch->off_flags, OFF_FAST) || IS_AFFECTED(ch, AFF_HASTE))
		chance += 10;
	if (IS_SET(victim->off_flags, OFF_FAST)
	||  IS_AFFECTED(victim, AFF_HASTE))
		chance -= 25;

	/* level */
	chance += (ch->level - victim->level) * 2;

	if (chance % 5 == 0)
		chance += 1;

	/* terrain */

	switch(ch->in_room->sector_type) {
	case(SECT_INSIDE):		chance -= 20;	break;
	case(SECT_CITY):		chance -= 10;	break;
	case(SECT_FIELD):		chance +=  5;	break;
	case(SECT_FOREST):				break;
	case(SECT_HILLS):				break;
	case(SECT_MOUNTAIN):		chance -= 10;	break;
	case(SECT_WATER_SWIM):		chance  =  0;	break;
	case(SECT_WATER_NOSWIM):	chance  =  0;	break;
	case(SECT_AIR):			chance  =  0;  	break;
	case(SECT_DESERT):		chance += 10;   break;
	}

	if (chance == 0) {
		char_puts("There isn't any dirt to kick.\n\r",ch);
		return;
	}

	/* now the attack */
	if (number_percent() < chance) {
		AFFECT_DATA af;
		act("$n is blinded by the dirt in $s eyes!",
		    victim, NULL, NULL, TO_ROOM);
		damage(ch, victim, number_range(2, 5),
		       gsn_dirt, DAM_NONE, FALSE);
		char_puts("You can't see a thing!\n\r", victim);
		check_improve(ch, gsn_dirt, TRUE, 2);

		af.where	= TO_AFFECTS;
		af.type 	= gsn_dirt;
		af.level 	= ch->level;
		af.duration	= 0;
		af.location	= APPLY_HITROLL;
		af.modifier	= -4;
		af.bitvector 	= AFF_BLIND;

		affect_to_char(victim, &af);
	}
	else {
		damage(ch, victim, 0, gsn_dirt, DAM_NONE, TRUE);
		check_improve(ch, gsn_dirt, FALSE, 2);
	}
}

void do_trip(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int chance;

	if (MOUNTED(ch)) {
		char_puts("You can't trip while riding!\n\r", ch);
		return;
	}

	one_argument(argument,arg);

	if ((chance = get_skill(ch,gsn_trip)) == 0) {
		char_puts("Tripping? What's that?\n\r", ch);
		return;
	}

	if (arg[0] == '\0') {
		victim = ch->fighting;
		if (victim == NULL) {
			char_puts("But you aren't fighting anyone!\n\r",ch);
			return;
 		}
	}
	else if ((victim = get_char_room(ch, arg)) == NULL) {
		char_puts("They aren't here.\n\r",ch);
		return;
	}

	if (MOUNTED(victim)) {
		char_puts("You can't trip a riding one!\n\r", ch);
		return;
	}

	if (IS_AFFECTED(victim,AFF_FLYING)) {
		act("$S feet aren't on the ground.", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (victim->position < POS_FIGHTING) {
		act("$N is already down.", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (victim == ch) {
		char_puts("You fall flat on your face!\n\r", ch);
		WAIT_STATE(ch, 2 * SKILL(gsn_trip)->beats);
		act("$n trips over $s own feet!", ch, NULL, NULL, TO_ROOM);
		return;
	}

	if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim) {
		act("$N is your beloved master.", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (is_safe(ch,victim))
		return;

	/* modifiers */

	/* size */
	if (ch->size < victim->size)
		/* bigger = harder to trip */
		chance += (ch->size - victim->size) * 10;

	/* dex */
	chance += get_curr_stat(ch, STAT_DEX);
	chance -= get_curr_stat(victim, STAT_DEX) * 3 / 2;

	if (IS_AFFECTED(ch,AFF_FLYING))
		chance -= 10;

	/* speed */
	if (IS_SET(ch->off_flags, OFF_FAST) || IS_AFFECTED(ch, AFF_HASTE))
		chance += 10;
	if (IS_SET(victim->off_flags,OFF_FAST)
	||  IS_AFFECTED(victim, AFF_HASTE))
		chance -= 20;

	/* level */
	chance += (ch->level - victim->level) * 2;

	RESET_WAIT_STATE(ch);

	/* now the attack */
	if (number_percent() < chance) {
		act("$n trips you and you go down!", ch, NULL, victim, TO_VICT);
		act("You trip $N and $N goes down!",
		    ch, NULL, victim, TO_CHAR);
		act("$n trips $N, sending $M to the ground.",
		    ch, NULL, victim, TO_NOTVICT);
		check_improve(ch, gsn_trip, TRUE, 1);

		WAIT_STATE(victim, 2 * PULSE_VIOLENCE);
		WAIT_STATE(ch, SKILL(gsn_trip)->beats);
		victim->position = POS_RESTING;
		damage(ch, victim, number_range(2, 2 + 2*victim->size),
		       gsn_trip, DAM_BASH, TRUE);
	}
	else {
		damage(ch, victim, 0, gsn_trip, DAM_BASH, TRUE);
		WAIT_STATE(ch, SKILL(gsn_trip)->beats*2/3);
		check_improve(ch, gsn_trip, FALSE, 1);
	} 
}

void do_backstab(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	int chance;

	one_argument(argument, arg);

	if (MOUNTED(ch)) {
		char_puts("You can't backstab while riding!\n\r", ch);
		return;
	}

	if ((chance = get_skill(ch, gsn_backstab)) == 0) {
		char_puts("You don't know how to backstab.\n\r",ch);
		return;
	}

	if (arg[0] == '\0') {
		char_puts("Backstab whom?\n\r", ch);
		return;
	}

	WAIT_STATE(ch, SKILL(gsn_backstab)->beats);

	if ((victim = get_char_room(ch, arg)) == NULL) {
		char_puts("They aren't here.\n\r", ch);
		return;
	}

	if (IS_NPC(ch) && !IS_NPC(victim))
		return;

	if (victim == ch) {
		char_puts("How can you sneak up on yourself?\n\r", ch);
		return;
	}

	if ((obj = get_eq_char(ch, WEAR_WIELD)) == NULL) {
		char_puts("You need to wield a weapon to backstab.\n\r", ch);
		return;
	}

	if (attack_table[obj->value[3]].damage != DAM_PIERCE) {
		char_puts("You need to wield a piercing weapon to backstab.\n\r",
			  ch);
		return;
	}

	if (victim->fighting != NULL) {
		char_puts("You can't backstab a fighting person.\n\r", ch);
		return;
	}

	if (is_safe(ch, victim))
		return;

	if (victim->hit < (0.7 * victim->max_hit) && (IS_AWAKE(victim))) {
		act("$N is hurt and suspicious ... you couldn't sneak up.",
		    ch, NULL, victim, TO_CHAR);
		return;
	}

	if (current_time-victim->last_fight_time < 300 && IS_AWAKE(victim)) {
		act("$N is suspicious ... you couldn't sneak up.",
		    ch, NULL, victim, TO_CHAR);
		return;       
	}

	if (!IS_AWAKE(victim)
	||  IS_NPC(ch)
	||  number_percent() < chance) {
		check_improve(ch, gsn_backstab, TRUE, 1);
		if (!IS_NPC(ch)
		&&  number_percent() <
				(get_skill(ch, gsn_dual_backstab)/10)*8) {
			check_improve(ch, gsn_dual_backstab, TRUE, 1);
			one_hit(ch, victim, gsn_backstab, FALSE);
			one_hit(ch, victim, gsn_dual_backstab, FALSE);
		}
		else {
			check_improve(ch, gsn_dual_backstab, FALSE, 1);
			multi_hit(ch, victim, gsn_backstab);
		}
	}
	else {
		check_improve(ch, gsn_backstab, FALSE, 1);
		damage(ch, victim, 0, gsn_backstab, DAM_NONE, TRUE);
	}
}

void do_cleave(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	int chance;

	if (MOUNTED(ch)) {
		char_puts("You can't cleave while riding!\n\r", ch);
		return;
	}

	one_argument(argument, arg);

	if (ch->master != NULL && IS_NPC(ch))
		return;

	if ((chance = get_skill(ch, gsn_cleave)) == 0) {
		char_puts("You don't know how to cleave.\n\r",ch);
		return;
	}

	if (arg[0] == '\0') {
		char_puts("Cleave whom?\n\r", ch);
		return;
	}

	WAIT_STATE(ch, SKILL(gsn_cleave)->beats);

	if ((victim = get_char_room(ch, arg)) == NULL) {
		char_puts("They aren't here.\n\r", ch);
		return;
	}

	if (victim == ch) {
		char_puts("How can you sneak up on yourself?\n\r", ch);
		return;
	}

	if ((obj = get_eq_char(ch, WEAR_WIELD)) == NULL) {
		char_puts("You need to wield a weapon to cleave.\n\r", ch);
		return;
	}

	if (victim->fighting != NULL) {
		char_puts("You can't cleave a fighting person.\n\r", ch);
		return;
	}

	if ((victim->hit < (0.9 * victim->max_hit))
	&&  (IS_AWAKE(victim))) {
		act("$N is hurt and suspicious ... you can't sneak up.",
		    ch, NULL, victim, TO_CHAR);
		return;
	}

	if (is_safe(ch, victim))
		return;

	if (!IS_AWAKE(victim)
	||  IS_NPC(ch)
	||  number_percent() < chance) {
		check_improve(ch, gsn_cleave, TRUE, 1);
		multi_hit(ch, victim, gsn_cleave);
	}
	else {
		check_improve(ch, gsn_cleave, FALSE, 1);
		damage(ch, victim, 0, gsn_cleave, DAM_NONE, TRUE);
	}
}

void do_ambush(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int chance;

	if (MOUNTED(ch)) {
		char_puts("You can't ambush while riding!\n\r", ch);
		return;
	}

	one_argument(argument, arg);

	if ((chance = get_skill(ch, gsn_ambush)) == 0) {
		char_puts("You don't know how to ambush.\n\r", ch);
		return;
	}

	if (arg[0] == '\0') {
		char_puts("Ambush whom?\n\r", ch);
		return;
	}
	
	WAIT_STATE(ch, SKILL(gsn_ambush)->beats);

	if ((victim = get_char_room(ch, arg)) == NULL) {
		char_puts("They aren't here.\n\r", ch);
		return;
	}

	if (victim == ch) {
		char_puts("How can you ambush yourself?\n\r", ch);
		return;
	}

	if (!IS_AFFECTED(ch, AFF_CAMOUFLAGE) || can_see(victim, ch)) {
		char_puts("But they can see you.\n\r", ch);
		return;
	}

	if (is_safe(ch, victim))
		return;

	if (!IS_AWAKE(victim)
	||  IS_NPC(ch)
	||  number_percent() < chance) {
		check_improve(ch, gsn_ambush, TRUE, 1);
		multi_hit(ch, victim, gsn_ambush);
	}
	else {
		check_improve(ch, gsn_ambush, FALSE, 1);
		damage(ch, victim, 0, gsn_ambush, DAM_NONE, TRUE);
	}
}

void do_rescue(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	CHAR_DATA *fch;

	one_argument(argument, arg);
	if (arg[0] == '\0') {
		char_puts("Rescue whom?\n\r", ch);
		return;
	}

	WAIT_STATE(ch, SKILL(gsn_rescue)->beats);

	if ((victim = get_char_room(ch, arg)) == NULL) {
		char_puts("They aren't here.\n\r", ch);
		return;
	}

	if (victim == ch) {
		char_puts("What about fleeing instead?\n\r", ch);
		return;
	}

	if (!IS_NPC(ch) && IS_NPC(victim)) {
		char_puts("Doesn't need your help!\n\r", ch);
		return;
	}

	if (ch->fighting == victim) {
		char_puts("Too late.\n\r", ch);
		return;
	}

	if ((fch = victim->fighting) == NULL) {
		char_puts("That person is not fighting right now.\n\r", ch);
		return;
	}

	if (IS_NPC(ch) && ch->master != NULL && IS_NPC(victim))
		return;

	if (ch->master != NULL && is_safe(ch->master, fch))
		return;

	if (is_safe(ch, fch))
		return;

	if (number_percent() > get_skill(ch,gsn_rescue)) {
		char_puts("You fail the rescue.\n\r", ch);
		check_improve(ch, gsn_rescue, FALSE, 1);
		return;
	}

	act("You rescue $N!",  ch, NULL, victim, TO_CHAR   );
	act("$n rescues you!", ch, NULL, victim, TO_VICT   );
	act("$n rescues $N!",  ch, NULL, victim, TO_NOTVICT);
	check_improve(ch, gsn_rescue, TRUE, 1);

	stop_fighting(fch, FALSE);
	stop_fighting(victim, FALSE);

	set_fighting(ch, fch);
	set_fighting(fch, ch);
}

void do_kick(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *victim;
	int kick_dam;
	int chance;

	if (MOUNTED(ch)) {
		char_puts("You can't kick while riding!\n\r", ch);
		return;
	}

	if ((chance = get_skill(ch, gsn_kick)) == 0) {
		char_puts("You better leave the martial arts to fighters.\n\r",
			  ch);
		return;
	}

	if (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_KICK))
		return;

	if ((victim = ch->fighting) == NULL) {
		char_puts("You aren't fighting anyone.\n\r", ch);
		return;
	}

	if (IS_AFFECTED(ch,AFF_FLYING))
		chance = chance * 110 / 100;

	WAIT_STATE(ch, SKILL(gsn_kick)->beats);
	if (IS_NPC(ch) || number_percent() < chance) {
		kick_dam = number_range(1, ch->level);
		if ((ch->class == CLASS_SAMURAI)
		&& (get_eq_char(ch, WEAR_FEET) == NULL))
			kick_dam *= 2;
		kick_dam += ch->damroll / 2;
		damage(ch, victim, kick_dam, gsn_kick, DAM_BASH, TRUE);
		check_improve(ch, gsn_kick, TRUE, 1);
	}
	else {
		damage(ch, victim, 0, gsn_kick, DAM_BASH, TRUE);
		check_improve(ch, gsn_kick, FALSE, 1);
	}
}

void do_circle(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *victim;
	CHAR_DATA *rch;
	int chance;
	OBJ_DATA *obj;

	if (MOUNTED(ch)) {
		char_puts("You can't circle while riding!\n\r", ch);
		return;
	}

	if ((chance = get_skill(ch, gsn_circle)) == 0) {
		char_puts("You don't know how to circle.\n\r", ch);
		return;
	}

	if ((victim = ch->fighting) == NULL) {
		char_puts("You aren't fighting anyone.\n\r", ch);
		return;
	}

	if ((obj = get_eq_char(ch, WEAR_WIELD)) == NULL
	||  attack_table[obj->value[3]].damage != DAM_PIERCE) {
		 char_puts("You must wield a piercing weapon to circle stab.\n\r",
			   ch);
		 return;
	}

	if (is_safe(ch, victim))
		return;

	WAIT_STATE(ch, SKILL(gsn_circle)->beats);

	for (rch = ch->in_room->people; rch; rch = rch->next_in_room) {
		if (rch->fighting == ch) {
			char_puts("You can't circle while defending yourself.\n\r",
				  ch);
			return;
		}
	}

	if (number_percent() < chance) {
		one_hit(ch, victim, gsn_circle, FALSE);
		check_improve(ch, gsn_circle, TRUE, 1);
	}
	else {
		damage(ch, victim, 0, gsn_circle, TYPE_UNDEFINED, TRUE);
		check_improve(ch, gsn_circle, FALSE, 1);
	}
}

void do_disarm(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	int chance, hth, ch_weapon, vict_weapon, ch_vict_weapon;
	int disarm_second = 0;
	char arg[MAX_INPUT_LENGTH];
/*
	if (MOUNTED(ch)) {
			char_puts("You can't disarm while riding!\n\r", ch);
			return;
	}
*/
	hth = 0;

	if (ch->master != NULL && IS_NPC(ch))
		return;

	if ((chance = get_skill(ch, gsn_disarm)) == 0) {
		char_puts("You don't know how to disarm opponents.\n\r", ch);
		return;
	}

	if (get_eq_char(ch, WEAR_WIELD) == NULL 
	&&  (hth = get_skill(ch, gsn_hand_to_hand)) == 0) {
		char_puts("You must wield a weapon to disarm.\n\r", ch);
		return;
	}

	if ((victim = ch->fighting) == NULL) {
		char_puts("You aren't fighting anyone.\n\r", ch);
		return;
	}

	if ((obj = get_eq_char(victim, WEAR_WIELD)) == NULL) {
		char_puts("Your opponent is not wielding a weapon.\n\r", ch);
		return;
	}

	argument = one_argument(argument, arg);
	if (!IS_NPC(ch) && arg[0] != '\0') {
		if (!str_prefix(arg, "second"))
			disarm_second = 1;
		else
			disarm_second = 0;    
	}
	/* find weapon skills */
	ch_weapon = get_weapon_skill(ch,get_weapon_sn(ch, WEAR_WIELD));

	if (disarm_second) {
		vict_weapon = get_weapon_skill(victim,
				      get_weapon_sn(victim, WEAR_SECOND_WIELD));
		ch_vict_weapon = get_weapon_skill(ch,
				      get_weapon_sn(victim, WEAR_SECOND_WIELD));
	}
	else {
		vict_weapon = get_weapon_skill(victim,
					get_weapon_sn(victim, WEAR_WIELD));
		ch_vict_weapon = get_weapon_skill(ch,
					get_weapon_sn(victim, WEAR_WIELD));
	}
	/* modifiers */

	/* skill */
	if (get_eq_char(ch,WEAR_WIELD) == NULL)
		chance = chance * hth/150;
	else
		chance = chance * ch_weapon/100;

	chance += (ch_vict_weapon/2 - vict_weapon) / 2; 

	/* dex vs. strength */
	chance += get_curr_stat(ch,STAT_DEX);
	chance -= 2 * get_curr_stat(victim,STAT_STR);

	/* level */
	chance += (ch->level - victim->level) * 2;
 
	/* and now the attack */
	WAIT_STATE(ch, SKILL(gsn_disarm)->beats);
	if (number_percent() < chance) {
		disarm(ch, victim, disarm_second);
		check_improve(ch, gsn_disarm, TRUE, 1);
	}
	else {
		act("You fail to disarm $N.", ch, NULL, victim, TO_CHAR);
		act("$n tries to disarm you, but fails.",
		    ch, NULL, victim, TO_VICT);
		act("$n tries to disarm $N, but fails.",
		    ch, NULL, victim, TO_NOTVICT);
		check_improve(ch,gsn_disarm,FALSE,1);
	}
}

void do_nerve(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *victim;
	char arg[MAX_INPUT_LENGTH];
	int chance;

	if (MOUNTED(ch)) {
		char_puts("You can't nerve while riding!\n\r", ch);
		return;
	}

	one_argument(argument,arg);

	if ((chance = get_skill(ch, gsn_nerve)) == 0) {
		char_puts("Huh?\n\r", ch);
		return;
	}
	if (ch->fighting == NULL) {
		char_puts("You aren't fighting anyone.\n\r", ch);
		return;
	}

	victim = ch->fighting;

	if (is_affected(ch,gsn_nerve)) {
		char_puts("You cannot weaken that character any more.\n\r",
			  ch);
		return;
	}

	if (is_safe(ch,victim))
		return;

	WAIT_STATE(ch, SKILL(gsn_nerve)->beats);

	if (IS_NPC(ch)
	||  number_percent() < (chance + ch->level 
			                 + get_curr_stat(ch,STAT_DEX))/2) {
		AFFECT_DATA af;
		af.where	= TO_AFFECTS;
		af.type 	= gsn_nerve;
		af.level 	= ch->level;
		af.duration	= ch->level * PULSE_VIOLENCE/PULSE_TICK;
		af.location	= APPLY_STR;
		af.modifier	= -3;
		af.bitvector	= 0;

		affect_to_char(victim,&af);
		act("You weaken $N with your nerve pressure.",
		    ch, NULL, victim, TO_CHAR);
		act("$n weakens you with $s nerve pressure.",
		    ch, NULL, victim, TO_VICT);
		act("$n weakens $N with $s nerve pressure.",
		    ch, NULL, victim, TO_NOTVICT);
		check_improve(ch, gsn_nerve, TRUE, 1);
	}
	else {
		char_puts("You press the wrong points and fail.\n\r",ch);
		act("$n tries to weaken you with nerve pressure, but fails.",
		    ch, NULL, victim, TO_VICT);
		act("$n tries to weaken $N with nerve pressure, but fails.",
		    ch, NULL, victim, TO_NOTVICT);
		check_improve(ch, gsn_nerve, FALSE, 1);
	}

	multi_hit(victim,ch,TYPE_UNDEFINED);
}

void do_endure(CHAR_DATA *ch, const char *argument)
{
	AFFECT_DATA af;
	int chance;

	if (IS_NPC(ch)) {
		char_puts("You have no endurance whatsoever.\n\r", ch);
		return;
	}

	if ((chance = get_skill(ch, gsn_endure)) == 0) {
		char_puts("You lack the concentration.\n\r", ch);
		return;
	}
		 
	if (is_affected(ch,gsn_endure)) {
		char_puts("You cannot endure more concentration.\n\r", ch);
		return;
	}
	
	WAIT_STATE(ch, SKILL(gsn_endure)->beats);

	af.where 	= TO_AFFECTS;
	af.type 	= gsn_endure;
	af.level 	= ch->level;
	af.duration	= ch->level / 4;
	af.location	= APPLY_SAVING_SPELL;
	af.modifier	= - chance / 10; 
	af.bitvector	= 0;

	affect_to_char(ch, &af);

	char_puts("You prepare yourself for magical encounters.\n\r", ch);
	act("$n concentrates for a moment, then resumes $s position.",
	    ch, NULL, NULL, TO_ROOM);
	check_improve(ch, gsn_endure, TRUE, 1);
}
 
void do_tame(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *victim;
	char arg[MAX_INPUT_LENGTH];
	int chance;

	if ((chance = get_skill(ch, gsn_tame)) == 0) {
		char_puts("Huh?\n\r", ch);
		return;
	}

	one_argument(argument, arg);

	if (arg[0] == '\0') {
		char_puts("You are beyond taming.\n\r", ch);
		act("$n tries to tame $mself but fails miserably.",
		    ch, NULL, NULL, TO_ROOM);
		return;
	}

	WAIT_STATE(ch, SKILL(gsn_tame)->beats);
	
	if ((victim = get_char_room(ch,arg)) == NULL) {
		char_puts("They're not here.\n\r", ch);
		return;
	}

	if (IS_NPC(ch)) {
		char_puts("Why don't you tame yourself first?", ch);
		return;
	}

	if (!IS_NPC(victim)) {
		act("$N is beyond taming.", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (!IS_SET(victim->act,ACT_AGGRESSIVE)) {
		act("$N is not usually aggressive.", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (number_percent() < chance + 15 + 4*(ch->level - victim->level)) {
		REMOVE_BIT(victim->act, ACT_AGGRESSIVE);
		SET_BIT(victim->affected_by, AFF_CALM);
		char_puts("You calm down.\n\r", victim);
		act("You calm $N down.", ch, NULL, victim, TO_CHAR);
		act("$n calms $N down.", ch, NULL, victim, TO_NOTVICT);
		stop_fighting(victim, TRUE);
		check_improve(ch, gsn_tame, TRUE, 1);
	}
	else {
		char_puts("You failed.\n\r",ch);
		act("$n tries to calm down $N but fails.",
		    ch, NULL, victim, TO_NOTVICT);
		act("$n tries to calm you down but fails.",
		    ch, NULL, victim, TO_VICT);
		check_improve(ch, gsn_tame, FALSE, 1);
	}
}

void do_assassinate(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int chance;

	if (MOUNTED(ch)) {
		char_puts("You can't assassinate while riding!\n\r", ch);
		return;
	}

	one_argument(argument, arg);

	if (ch->master != NULL && IS_NPC(ch))
		return;

	if ((chance = get_skill(ch, gsn_assassinate)) == 0) {
		char_puts("You don't know how to assassinate.\n\r", ch);
		return;
	}
	
	if (IS_AFFECTED(ch, AFF_CHARM))  {
		char_puts("You don't want to kill your beloved master.\n\r", ch);
		return;
	} 

	if (arg[0] == '\0') {
		char_puts("Assassinate whom?\n\r", ch);
		return;
	}

	WAIT_STATE(ch, SKILL(gsn_assassinate)->beats);

	if ((victim = get_char_room(ch, arg)) == NULL) {
		char_puts("They aren't here.\n\r", ch);
		return;
	}

	if (victim == ch) {
		char_puts("Suicide is against your way.\n\r", ch);
		return;
	}

	if (IS_IMMORTAL(victim) && !IS_NPC(victim)) {
		char_puts("Your hands pass through.\n\r", ch);
		return;
	}

	if (victim->fighting != NULL) {
		char_puts("You can't assassinate a fighting person.\n\r", ch);
		return;
	}
 
	if ((get_eq_char(ch, WEAR_WIELD) != NULL)
	||  (get_eq_char(ch, WEAR_HOLD ) != NULL))  {
		char_puts("You need both hands free to assassinate somebody.\n\r",
			  ch);
		return;
	}
 
	if (victim->hit < victim->max_hit
	&&  can_see(victim, ch) && IS_AWAKE(victim)) {
		act("$N is hurt and suspicious ... you can't sneak up.",
		    ch, NULL, victim, TO_CHAR);
		return;
	}
	
/*
	if (IS_SET(victim->imm_flags, IMM_WEAPON)) {
		act("$N seems immune to your assassination attempt.",
		    ch, NULL, victim, TO_CHAR);
		act("$N seems immune to $n's assassination attempt.",
		    ch, NULL, victim, TO_ROOM);
		return;
	}
*/

	if (is_safe(ch, victim))
		return;

	if (number_percent() < chance)
		multi_hit(ch,victim,gsn_assassinate);
	else {
		check_improve(ch, gsn_assassinate, FALSE, 1);
		damage(ch, victim, 0, gsn_assassinate, DAM_NONE, TRUE);
	}
}

void do_caltrops(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *victim = ch->fighting;
	int chance;

	if ((chance = get_skill(ch, gsn_caltrops)) == 0) {
		char_puts("Caltrops? Is that a dance step?\n\r", ch);
		return;
	}

	if (victim == NULL) {
		char_puts("You must be in combat.\n\r",ch);
		return;
	}

	if (is_safe(ch,victim))
		return;
		
	act("You throw a handful of sharp spikes at the feet of $N.",
	    ch, NULL, victim, TO_CHAR);
	act("$n throws a handful of sharp spikes at your feet!",
	    ch, NULL, victim, TO_VICT);

	WAIT_STATE(ch, SKILL(gsn_caltrops)->beats);

	if (!IS_NPC(ch) && number_percent() >= chance) {
		damage(ch, victim, 0, gsn_caltrops, DAM_PIERCE, TRUE);
		check_improve(ch, gsn_caltrops, FALSE, 1);
		return;
	}

	damage(ch, victim, ch->level, gsn_caltrops, DAM_PIERCE, TRUE);

	if (!is_affected(victim, gsn_caltrops)) {
		AFFECT_DATA tohit, todam, todex;

		tohit.where     = TO_AFFECTS;
		tohit.type      = gsn_caltrops;
		tohit.level     = ch->level;
		tohit.duration  = -1; 
		tohit.location  = APPLY_HITROLL;
		tohit.modifier  = -5;
		tohit.bitvector = 0;
		affect_to_char(victim, &tohit);

		todam.where = TO_AFFECTS;
		todam.type = gsn_caltrops;
		todam.level = ch->level;
		todam.duration = -1;
		todam.location = APPLY_DAMROLL;
		todam.modifier = -5;
		todam.bitvector = 0;
		affect_to_char(victim, &todam);

		todex.type = gsn_caltrops;
		todex.level = ch->level;
		todex.duration = -1;
		todex.location = APPLY_DEX;
		todex.modifier = -5;
		todex.bitvector = 0;
		affect_to_char(victim, &todex);

		act("$N starts limping.", ch, NULL, victim, TO_CHAR);
		act("You start to limp.", ch, NULL, victim, TO_VICT);
		check_improve(ch, gsn_caltrops, TRUE, 1);
	}
}

DECLARE_DO_FUN(do_throw_spear);

void do_throw(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *victim;
	char arg[MAX_INPUT_LENGTH];
	int chance;

	if (MOUNTED(ch)) {
		char_puts("You can't throw while riding!\n\r", ch);
		return;
	}

	argument = one_argument(argument,arg);

	if (!str_cmp(arg, "spear")) {
		do_throw_spear(ch, argument);
		return;
	}

	if ((chance = get_skill(ch, gsn_throw)) == 0) {
		char_puts("A clutz like you couldn't throw down a worm.\n\r",
			  ch);
		return;
	}

	if (IS_AFFECTED(ch, AFF_FLYING)) {
		char_puts("Your feet should touch the ground to balance\n\r",
			     ch);
		return;
	}

	if ((victim = ch->fighting) == NULL) {
		char_puts("You aren't fighting anyone.\n\r", ch);
		return;
	}

	if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim) {
		act("But $N is your friend!", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (is_safe(ch,victim))
		return;

	WAIT_STATE(ch, SKILL(gsn_throw)->beats);

	if (is_affected(victim, gsn_protective_shield)) {
		act_puts("You fail to reach $s arm.",ch,NULL,victim, TO_CHAR,
			 POS_FIGHTING);
		act_puts("$n fails to throw you.", ch, NULL, victim, TO_VICT,
			 POS_FIGHTING);
		act_puts("$n fails to throw $N.", ch, NULL, victim, TO_NOTVICT,
			 POS_FIGHTING);
		return;
	}

	if (ch->size < victim->size)
		chance += (ch->size - victim->size) * 10;
	else
		chance += (ch->size - victim->size) * 25; 

	/* stats */
	chance += get_curr_stat(ch,STAT_STR);
	chance -= get_curr_stat(victim,STAT_DEX) * 4/3;

	if (IS_AFFECTED(victim, AFF_FLYING))
		chance += 10;

	/* speed */
	if (IS_SET(ch->off_flags,OFF_FAST))
		chance += 10;
	if (IS_SET(victim->off_flags,OFF_FAST))
		chance -= 20;

	/* level */
	chance += (ch->level - victim->level) * 2;

	if (number_percent() < chance) {
		act("You throw $N to the ground with stunning force.",
		    ch, NULL, victim, TO_CHAR);
		act("$n throws you to the ground with stunning force.",
		    ch, NULL, victim, TO_VICT);
		act("$n throws $N to the ground with stunning force.",
		    ch, NULL, victim, TO_NOTVICT);
		WAIT_STATE(victim,2 * PULSE_VIOLENCE);

		damage(ch, victim,ch->level + get_curr_stat(ch, STAT_STR), 
		       gsn_throw, DAM_BASH, TRUE);
		check_improve(ch, gsn_throw, TRUE, 1);
	}
	else {
		act("You fail to grab your opponent.",
		    ch, NULL, NULL, TO_CHAR);
		act("$N tries to throw you, but fails.",
		    victim, NULL, ch, TO_CHAR);
		act("$n tries to grab $N's arm.",
		    ch, NULL, victim, TO_NOTVICT);
		check_improve(ch, gsn_throw, FALSE, 1);
	}
}

void do_strangle(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *victim;
	AFFECT_DATA af;
	int chance;

	if (MOUNTED(ch)) {
		char_puts("You can't strangle while riding!\n\r", ch);
		return;
	}

	if ((chance = get_skill(ch, gsn_strangle)) == 0) {
		char_puts("You lack the skill to strangle.\n\r", ch);
		return;
	}

	if (IS_AFFECTED(ch, AFF_CHARM))  {
		char_puts("You don't want to grap your beloved masters' neck.\n\r",
			  ch);
		return;
	} 

	WAIT_STATE(ch, SKILL(gsn_strangle)->beats);

	if ((victim = get_char_room(ch,argument)) == NULL) {
		char_puts("You do not see that person here.\n\r", ch);
		return;
	}

	if (ch == victim) {
		char_puts("Even you are not that stupid.\n\r", ch);
		return;
	}

	if (is_affected(victim, gsn_strangle))
		return;

	if (is_safe(ch, victim))
		return;

	SET_FIGHT_TIME(victim);
	SET_FIGHT_TIME(ch);

	if (number_percent() < chance * 6 /10) {
		act("You grab hold of $N's neck and put $M to sleep.",
		    ch, NULL, victim, TO_CHAR);
		act("$n grabs hold of your neck and puts you to sleep.",
		    ch, NULL, victim, TO_VICT);
		act("$n grabs hold of $N's neck and puts $M to sleep.",
		    ch, NULL, victim, TO_NOTVICT);
		check_improve(ch, gsn_strangle, TRUE, 1);
		
		af.type = gsn_strangle;
		af.where = TO_AFFECTS;
		af.level = ch->level;
		af.duration = ch->level / 20 + 1;
		af.location = APPLY_NONE;
		af.modifier = 0;
		af.bitvector = AFF_SLEEP;
		affect_join (victim,&af);

		if (IS_AWAKE(victim))
			victim->position = POS_SLEEPING;
	}
	else {
		damage(ch,victim, 0, gsn_strangle, DAM_NONE, TRUE);
		check_improve(ch, gsn_strangle, FALSE, 1);
	}
}

void do_blackjack(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *victim;
	AFFECT_DATA af;
	int chance;

	if (MOUNTED(ch)) {
		char_puts("You can't blackjack while riding!\n\r", ch);
		return;
	}

	if ((chance = get_skill(ch, gsn_blackjack)) == 0) {
		char_puts("Huh?\n\r", ch);
		return;
	}

	WAIT_STATE(ch, SKILL(gsn_blackjack)->beats);

	if ((victim = get_char_room(ch,argument)) == NULL) {
		char_puts("You do not see that person here.\n\r", ch);
		return;
	}

	if (ch == victim) {
		char_puts("You idiot?! Blackjack your self?!\n\r", ch);
		return;
	}

	if (IS_AFFECTED(ch, AFF_CHARM))  {
		char_puts("You don't want to hit your beloved masters' head with a full filled jack.\n\r",
			  ch);
		return;
	} 

	if (IS_AFFECTED(victim,AFF_SLEEP))  {
		act("$E is already asleep.", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (is_safe(ch,victim))
		return;

	SET_FIGHT_TIME(victim);
	SET_FIGHT_TIME(ch);

	chance /= 2;
	chance += URANGE(0, (get_curr_stat(ch, STAT_DEX)-20)*2, 10);
	chance += can_see(victim, ch) ? 0 : 5;
	if (IS_NPC(victim) && victim->pIndexData->pShop != NULL)
		chance -= 40;
 
	if (number_percent() < chance) {
		act("You hit $N's head with a lead filled sack.",
		    ch, NULL, victim, TO_CHAR);
		act("You feel a sudden pain erupts through your skull!",
		    ch, NULL, victim, TO_VICT);
		act("$n whacks $N at the back of $S head with a heavy looking sack!  *OUCH*",
		    ch, NULL, victim, TO_NOTVICT);
		check_improve(ch, gsn_blackjack, TRUE, 1);
		
		af.type		= gsn_blackjack;
		af.where	= TO_AFFECTS;
		af.level	= ch->level;
		af.duration	= ch->level / 15 + 1;
		af.location	= APPLY_NONE;
		af.modifier	= 0;
		af.bitvector	= AFF_SLEEP;
		affect_join (victim,&af);

		if (IS_AWAKE(victim))
			victim->position = POS_SLEEPING;
	}
	else {
		damage(ch, victim, ch->level/2, gsn_blackjack, DAM_NONE, TRUE);
		check_improve(ch, gsn_blackjack, FALSE, 1);
	}
}

void do_bloodthirst(CHAR_DATA *ch, const char *argument)
{
	int chance, hp_percent;

	if ((chance = get_skill(ch, gsn_bloodthirst)) == 0) {
		char_puts("You're not that thirsty.\n\r", ch);
		return;
	}

	if (IS_AFFECTED(ch, AFF_BLOODTHIRST)) {
		char_puts("Your thirst for blood continues.\n\r", ch);
		return;
	}

	if (IS_AFFECTED(ch, AFF_CALM)) {
		char_puts("You're feeling to mellow to be bloodthirsty.\n\r",
			  ch);
		return;
	}

	if (ch->fighting == NULL) {
		char_puts("You need to be fighting.\n\r", ch);
		return;
	}

	/* modifiers */

	hp_percent = 100 * ch->hit/ch->max_hit;
	chance += 25 - hp_percent/2;

	if (number_percent() < chance) {
		AFFECT_DATA af;

		WAIT_STATE(ch, PULSE_VIOLENCE);
	
		char_puts("You hunger for blood!\n\r", ch);
		act("$n gets a bloodthirsty look in $s eyes.",
		    ch, NULL, NULL, TO_ROOM);
		check_improve(ch, gsn_bloodthirst, TRUE, 2);

		af.where	= TO_AFFECTS;
		af.type		= gsn_bloodthirst;
		af.level	= ch->level;
		af.duration	= 2 + ch->level / 18;
		af.modifier	= 5 + ch->level / 4;
		af.bitvector 	= AFF_BLOODTHIRST;

		af.location	= APPLY_HITROLL;
		affect_to_char(ch, &af);

		af.location	= APPLY_DAMROLL;
		affect_to_char(ch, &af);

		af.modifier	= - UMIN(ch->level - 5,35);
		af.location	= APPLY_AC;
		affect_to_char(ch, &af);
	}
	else {
		WAIT_STATE(ch,3 * PULSE_VIOLENCE);
		char_puts("You feel bloodthirsty for a moment, but it passes.\n\r",
			  ch);
		check_improve(ch, gsn_bloodthirst, FALSE, 2);
	}
}


void do_spellbane(CHAR_DATA *ch, const char *argument)
{
	AFFECT_DATA af;
	int chance;

	if ((chance = get_skill(ch, gsn_spellbane)) == 0) {
		char_puts("Huh?\n\r", ch);
		return;
	}

	if (is_affected(ch,gsn_spellbane)) {
		char_puts("You are already deflecting spells.\n\r", ch);
		return;
	}

	WAIT_STATE(ch, SKILL(gsn_spellbane)->beats);

	af.where	= TO_AFFECTS;
	af.type 	= gsn_spellbane;
	af.level 	= ch->level;
	af.duration	= ch->level / 3;
	af.location	= APPLY_SAVING_SPELL;
	af.modifier	= -ch->level/4;
	af.bitvector	= 0;

	affect_to_char(ch,&af);

	act("Your hatred of magic surrounds you.", ch, NULL, NULL, TO_CHAR);
	act("$n fills the air with $s hatred of magic.",
	    ch, NULL, NULL, TO_ROOM);
	check_improve(ch, gsn_spellbane, TRUE, 1);
}

void do_resistance(CHAR_DATA *ch, const char *argument)
{
	int chance;
	int mana;

	if ((chance = get_skill(ch, gsn_resistance)) == 0) {
		char_puts("Huh?\n\r", ch);
		return;
	}

	if (is_affected(ch, gsn_resistance)) {
		char_puts("You are as resistant as you will get.\n\r", ch);
		return;
	}

	mana = SKILL(gsn_resistance)->min_mana;
	if (ch->mana < mana) {
		char_puts("You cannot muster up the energy.\n\r", ch);
		return;
	}

	WAIT_STATE(ch, SKILL(gsn_resistance)->beats);

	if (number_percent() < chance) {
		AFFECT_DATA af;
		
		af.where	= TO_AFFECTS;
		af.type 	= gsn_resistance;
		af.level 	= ch->level;
		af.duration	= ch->level / 6;
		af.location	= APPLY_NONE;
		af.modifier	= 0;
		af.bitvector	= 0;

		affect_to_char(ch, &af);
		ch->mana -= mana;

		act("You feel tough!", ch, NULL, NULL, TO_CHAR);
		act("$n looks tougher.", ch, NULL, NULL, TO_ROOM);
		check_improve(ch, gsn_resistance, TRUE, 1);
	}
	else {
		ch->mana -= mana/2;

		char_puts("You flex your muscles, "
			  "but you don't feel tougher.\n\r", ch);
		act("$n flexes $s muscles, trying to look tough.",
		    ch, NULL, NULL, TO_ROOM);
		check_improve(ch, gsn_resistance, FALSE, 1);
	}
}

void do_trophy(CHAR_DATA *ch, const char *argument)
{
	int trophy_vnum;
	OBJ_DATA *trophy;
	AFFECT_DATA af;
	OBJ_DATA *part;
	char arg[MAX_INPUT_LENGTH];
	int level;
	int chance;
	int mana;
	
	if ((chance = get_skill(ch, gsn_trophy)) == 0) {
		char_puts("Huh?\n\r", ch);
		return;
	}
	
	if (is_affected(ch, gsn_trophy)) {
		char_puts("But you've already got one trophy!\n\r", ch);
		return;
	}
	
	mana = SKILL(gsn_trophy)->min_mana;
	if (ch->mana < mana) {
		char_puts("You feel too weak to concentrate on a trophy.\n\r",
			  ch);
		return;
	}
	
	one_argument(argument, arg);
	if (arg[0] == '\0') {
		char_puts("Make a trophy of what?\n\r", ch);
		return;
	}
	
	if ((part = get_obj_carry(ch, arg)) == NULL) {
		char_puts("You do not have that body part.\n\r", ch);
		return;
	}

	if (number_percent() < chance * 2 / 3) {
		char_puts("You failed and destroyed it.\n\r", ch);
		extract_obj(part);
		return;
	} 

	WAIT_STATE(ch, SKILL(gsn_trophy)->beats);

	if (part->pIndexData->vnum == OBJ_VNUM_SLICED_ARM
	||  part->pIndexData->vnum == OBJ_VNUM_SLICED_LEG
	||  part->pIndexData->vnum == OBJ_VNUM_SEVERED_HEAD
	||  part->pIndexData->vnum == OBJ_VNUM_TORN_HEART
	||  part->pIndexData->vnum == OBJ_VNUM_GUTS)
		trophy_vnum = OBJ_VNUM_BATTLE_PONCHO;
	else if (part->pIndexData->vnum == OBJ_VNUM_BRAINS) {
		char_puts("Why don't you just eat those instead?\n\r", ch);
		return;
	}
	else {
		char_puts("You can't make a trophy out of that!\n\r", ch);
		return;
	}

	if (IS_NULLSTR(part->from)) {
		char_puts("Invalid body part.\n\r", ch);
		return;
	}

	if (!IS_NPC(ch) && number_percent() < chance) {
		af.where  = TO_AFFECTS;
		af.type	= gsn_trophy;
		af.level	= ch->level;
		af.duration	= ch->level/2;
		af.modifier	= 0;
		af.bitvector 	= 0;
		
		af.location	= 0;
		affect_to_char(ch,&af);
	
		if (trophy_vnum != 0) {
			level = UMIN(part->level + 5, MAX_LEVEL);
 
			trophy = create_named_obj(get_obj_index(trophy_vnum),
						     level, part->from);
			trophy->timer = ch->level * 2;
			trophy->cost  = 0;
			trophy->level = ch->level;
			ch->mana -= mana;
			af.where	= TO_OBJECT;
			af.type 	= gsn_trophy;
			af.level	= level;
			af.duration	= -1;
			af.location	= APPLY_DAMROLL;
			af.modifier  = ch->level / 5;
			af.bitvector	= 0;
			affect_to_obj(trophy, &af);

			af.location	= APPLY_HITROLL;
			af.modifier  = ch->level / 5;
			af.bitvector	= 0;
			affect_to_obj(trophy, &af);

			af.location	= APPLY_INT;
			af.modifier	= level>20?-2:-1;
			affect_to_obj(trophy, &af);

			af.location	= APPLY_STR;
			af.modifier	= level>20?2:1;
			affect_to_obj(trophy, &af);

			trophy->value[0] = ch->level;
			trophy->value[1] = ch->level;
			trophy->value[2] = ch->level;
			trophy->value[3] = ch->level;

			
			obj_to_char(trophy, ch);
			  check_improve(ch, gsn_trophy, TRUE, 1);
			
			act("You make a poncho from $p!",
			    ch, part, NULL, TO_CHAR);
			act("$n makes a poncho from $p!",
			    ch, part, NULL, TO_ROOM);
			
			extract_obj(part);
			return;
		}
	}
	else {
		char_puts("You destroyed it.\n\r", ch);
		extract_obj(part);
		ch->mana -= mana/2;
		check_improve(ch, gsn_trophy, FALSE, 1);
	}
}
	
void do_truesight(CHAR_DATA *ch, const char *argument)
{
	int chance;

	if ((chance = get_skill(ch, gsn_truesight)) == 0) {
		char_puts("Huh?\n\r", ch);
		return;
	}

	if (is_affected(ch, gsn_truesight)) {
		char_puts("Your eyes are as sharp as they can get.\n\r", ch);
		return;
	}

	WAIT_STATE(ch, SKILL(gsn_truesight)->beats);

	if (number_percent() < chance) {
		AFFECT_DATA af;
		
		af.where    = TO_AFFECTS;
		af.type     = gsn_truesight;
		af.level    = ch->level;
		af.duration = ch->level/2 + 5;
		af.location = APPLY_NONE;
		af.modifier = 0;
		af.bitvector = AFF_DETECT_HIDDEN;
		affect_to_char(ch, &af);

		af.bitvector = AFF_DETECT_INVIS;
		affect_to_char(ch, &af);

		af.bitvector = AFF_DETECT_IMP_INVIS;
		affect_to_char(ch,&af);

		af.bitvector = AFF_ACUTE_VISION;
		affect_to_char(ch,&af);

		af.bitvector = AFF_DETECT_MAGIC;
		affect_to_char(ch,&af);

		act("You look around sharply!",ch,NULL,NULL,TO_CHAR);
		act("$n looks more enlightened.",ch,NULL,NULL,TO_ROOM);
		check_improve(ch,gsn_truesight,TRUE,1);
	}
	else {
		char_puts("You look about sharply, but you don't see "
			     "anything new.\n\r" ,ch);
		act("$n looks around sharply but doesn't seem enlightened.",
			ch,NULL,NULL,TO_ROOM);
		check_improve(ch, gsn_truesight, FALSE, 1);
	}
}

void do_warcry(CHAR_DATA *ch, const char *argument)
{
	AFFECT_DATA af;
	int chance;
	int mana;
	
	if ((chance = get_skill(ch, gsn_warcry)) == 0) {
		char_puts("Huh?\n\r", ch);
		return;
	}
	
	if (is_affected(ch,gsn_bless) || is_affected(ch, gsn_warcry)) {
		char_puts("You are already blessed.\n\r",ch);
		return;
	}
	
	mana = SKILL(gsn_warcry)->min_mana;
	if (ch->mana < mana) {
		char_puts("You can't concentrate enough right now.\n\r",ch);
		return;
	}
	
	WAIT_STATE(ch, SKILL(gsn_warcry)->beats);
	
	if (number_percent() > chance) {
		char_puts("You grunt softly.\n\r", ch);
		act("$n makes some soft grunting noises.",
		    ch, NULL, NULL, TO_ROOM);
		ch->mana -= mana/2;
		check_improve(ch, gsn_warcry, FALSE, 1);
		return;
	}
	
	ch->mana -= mana;
	check_improve(ch, gsn_warcry, TRUE, 1);
 
	af.where	= TO_AFFECTS;
	af.type      = gsn_warcry;
	af.level	 = ch->level;
	af.duration  = 6+ch->level;
	af.location  = APPLY_HITROLL;
	af.modifier  = ch->level / 8;
	af.bitvector = 0;
	affect_to_char(ch, &af);
	
	af.location  = APPLY_SAVING_SPELL;
	af.modifier  = 0 - ch->level / 8;
	affect_to_char(ch, &af);
	char_puts("You feel righteous as you yell out your warcry.\n\r", ch);
}

void do_guard(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int chance;
	
	if ((chance = get_skill(ch, gsn_guard)) == 0) {
		char_puts("Huh?\n\r", ch);
		return;
	}

	one_argument(argument, arg);
	if (arg[0] == '\0') {
		char_puts("Guard whom?\n\r", ch);
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL) {
		char_puts("They aren't here.\n\r", ch);
		return;
	}

	if (IS_NPC(victim)) {
		act("$N doesn't need any of your help!", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (!str_cmp(arg, "none") || !str_cmp(arg, "self") || victim == ch) {
		if (ch->guarding == NULL) {
			char_puts("You can't guard yourself!\n\r", ch);
			return;
		}
		act("You stop guarding $N.", ch, NULL, ch->guarding, TO_CHAR);
		act("$n stops guarding you.", ch, NULL, ch->guarding, TO_VICT);
		act("$n stops guarding $N.", ch, NULL, ch->guarding, TO_NOTVICT);
		ch->guarding->guarded_by = NULL;
		ch->guarding             = NULL;
		return;
	}

	if (ch->guarding == victim) {
		act("You're already guarding $N!", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (ch->guarding != NULL) {
		char_puts("But you're already guarding someone else!\n\r", ch);
		return;
	}

	if (victim->guarded_by != NULL) {
		act("$N is already being guarded by someone.",
		    ch, NULL, victim, TO_CHAR);
		return;
	}

	if (victim->guarding == ch) {
		act("But $N is guarding you!", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (!is_same_group(victim, ch)) {
		act("But you aren't in the same group as $N.",
		    ch, NULL, victim, TO_CHAR);
		return;
	}

	if (IS_AFFECTED(ch,AFF_CHARM)) {
		act("You like your master too much to bother guarding $N!",
		    ch, NULL, victim, TO_VICT);
		return;
	}

	if (victim->fighting != NULL) {
		char_puts("Why don't you let them stop fighting first?\n\r",
			     ch);
		return;
	}
	
	if (ch->fighting != NULL) {
		char_puts("Better finish your own battle before you "
			     "worry about guarding someone else.\n\r", ch);
		return;
	}

	act("You are now guarding $N.", ch, NULL, victim, TO_CHAR);
	act("You are being guarded by $n.", ch, NULL, victim, TO_VICT);
	act("$n is now guarding $N.", ch, NULL, victim, TO_NOTVICT);

	ch->guarding = victim;
	victim->guarded_by = ch;
}

CHAR_DATA *check_guard(CHAR_DATA *ch, CHAR_DATA *mob)
{
	int chance;

	if (ch->guarded_by == NULL ||
		get_char_room(ch,ch->guarded_by->name) == NULL)
		return ch;
	else {
		chance = (get_skill(ch->guarded_by,gsn_guard) - 
			(1.5 * (ch->level - mob->level)));
		if (number_percent() < chance) {
			act("$n jumps in front of $N!",
			    ch->guarded_by, NULL, ch, TO_NOTVICT);
			act("$n jumps in front of you!",
			    ch->guarded_by, NULL, ch, TO_VICT);
			act("You jump in front of $N!",
			    ch->guarded_by, NULL, ch, TO_CHAR);
			check_improve(ch->guarded_by, gsn_guard, TRUE, 3);
			return ch->guarded_by;
		}
		else {
			check_improve(ch->guarded_by, gsn_guard, FALSE, 3);
			return ch;
		}
	}
}

void do_explode(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *victim = ch->fighting;
	CHAR_DATA *vch, *vch_next;
	int dam=0,hp_dam,dice_dam,mana;
	int hpch,level= ch->level;
	char arg[MAX_INPUT_LENGTH];
	int chance;

	if ((chance = get_skill(ch, gsn_explode)) == 0) {
		char_puts("Flame? What is that?\n\r", ch);
		return;
	}

	WAIT_STATE(ch, SKILL(gsn_explode)->beats);

	if (victim == NULL) {
		one_argument(argument, arg);
		if (arg == NULL) { 
			char_puts("You play with the exploding material.\n\r",
				  ch);
			return;
		}
		if ((victim = get_char_room(ch,arg)) == NULL) {
			char_puts("They aren't here.\n\r", ch);
			return;
		}
	}

	mana = SKILL(gsn_explode)->min_mana;
	if (ch->mana < mana) {
		char_puts("You can't find that much energy to fire\n\r", ch);
		return;
	}
	ch->mana -= mana;
 
	act("$n burns something.", ch, NULL, victim, TO_NOTVICT);
	act("$n burns a cone of exploding material over you!",
	    ch, NULL, victim, TO_VICT);
	act("Burn them all!.", ch, NULL, NULL, TO_CHAR);

	if (number_percent() >= chance) {
		damage(ch, victim, 0, gsn_explode, DAM_FIRE, TRUE);
		check_improve(ch, gsn_explode, FALSE, 1);
		return;
	}

	hpch = UMAX(10, ch->hit);
	hp_dam  = number_range(hpch/9+1, hpch/5);
	dice_dam = dice(level,20);

	if (!is_safe(ch,victim)) {
		dam = UMAX(hp_dam + dice_dam /10, dice_dam + hp_dam / 10);
		fire_effect(victim->in_room, level, dam/2, TARGET_ROOM);
	}

	for (vch = victim->in_room->people; vch != NULL; vch = vch_next) {
		vch_next = vch->next_in_room;

		if (is_safe_spell(ch,vch,TRUE)
		||  (IS_NPC(vch) && IS_NPC(ch)
		&&   (ch->fighting != vch || vch->fighting != ch)))
			  continue;

		if (vch == victim) { /* full damage */
			fire_effect(vch, level, dam, TARGET_CHAR);
			damage(ch, vch, dam, gsn_explode, DAM_FIRE, TRUE);
		}
		else { /* partial damage */
			fire_effect(vch, level/2, dam/4, TARGET_CHAR);
			damage(ch, vch, dam/2, gsn_explode, DAM_FIRE,TRUE);
		}
	}

	if (number_percent() >= chance) {
		fire_effect(ch, level/4, dam/10, TARGET_CHAR);
		damage(ch, ch, (ch->hit / 10), gsn_explode, DAM_FIRE, TRUE);
	}
}

void do_target(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *victim;
	int chance;

	if ((chance = get_skill(ch, gsn_target)) == 0) {
		char_puts("You don't know how to change the target "
			  "while fighting a group.\n\r" ,ch);
		return;
	}

	if (ch->fighting == NULL) {
		char_puts("You aren't fighting yet.\n\r",ch);
		return;
	}

	if (argument[0] == '\0') {
		char_puts("Change target to whom?\n\r",ch);
		return;
	}

	if ((victim = get_char_room(ch, argument)) == NULL) {
		char_puts("You don't see that item.\n\r",ch);
		return;
	}

	/* check victim is fighting with him */

	if (victim->fighting != ch) {
		char_puts ("Target is not fighting with you.\n\r",ch);
		return;
	}

	WAIT_STATE(ch, SKILL(gsn_target)->beats);

	if (number_percent() < chance / 2) {
		check_improve(ch, gsn_target, TRUE, 1);

		ch->fighting = victim;

		act("$n changes $s target to $N!",
		    ch, NULL, victim, TO_NOTVICT);
		act("You change your target to $N!",
		    ch, NULL, victim, TO_CHAR);
		act("$n changes target to you!", ch, NULL, victim, TO_VICT);
		return;
	}

	char_puts("You tried, but you couldn't. "
		  "But for honour try again!.\n\r", ch);
	check_improve(ch, gsn_target, FALSE, 1);
}

void do_tiger(CHAR_DATA *ch, const char *argument)
{
	int chance, hp_percent;
	int mana;

	if ((chance = get_skill(ch, gsn_tiger_power)) == 0) {
		char_puts("Huh?\n\r", ch);
		return;
	}
	act("$n calls the power of 10 tigers!.", ch, NULL, NULL, TO_ROOM);

	if (IS_AFFECTED(ch, AFF_BERSERK) || is_affected(ch, gsn_berserk)
	||  is_affected(ch, gsn_tiger_power) || is_affected(ch, gsn_frenzy)) {
		char_puts("You get a little madder.\n\r", ch);
		return;
	}

	if (IS_AFFECTED(ch,AFF_CALM)) {
		char_puts("You're feeling too mellow to call 10 tigers.\n\r",
			  ch);
		return;
	}

	if (ch->in_room->sector_type != SECT_FIELD
	&&  ch->in_room->sector_type != SECT_FOREST
	&&  ch->in_room->sector_type != SECT_MOUNTAIN
	&&  ch->in_room->sector_type != SECT_HILLS) {
		char_puts("No tigers can hear your call.\n\r", ch);
		return;
	}

	mana = SKILL(gsn_tiger_power)->min_mana;
	if (ch->mana < mana) {
		char_puts("You can't get up enough energy.\n\r", ch);
		return;
	}

	/* modifiers */

	/* fighting */
	if (ch->position == POS_FIGHTING)
		chance += 10;

	hp_percent = 100 * ch->hit/ch->max_hit;
	chance += 25 - hp_percent/2;
	ch->move /= 2;

	if (number_percent() < chance) {
		AFFECT_DATA af;

		WAIT_STATE(ch, SKILL(gsn_tiger_power)->beats);
		ch->mana -= mana;

		/* heal a little damage */
		ch->hit += ch->level * 2;
		ch->hit = UMIN(ch->hit, ch->max_hit);

		char_puts("10 tigers come for your call, as you call them!\n\r",
			  ch);
		act("10 tigers come across $n, and connect with $n.",
		    ch, NULL, NULL, TO_ROOM);
		check_improve(ch, gsn_tiger_power, TRUE, 2);

		af.where	= TO_AFFECTS;
		af.type		= gsn_tiger_power;
		af.level	= ch->level;
		af.duration	= number_fuzzy(ch->level / 8);
		af.modifier	= UMAX(1,ch->level/5);
		af.bitvector 	= AFF_BERSERK;

		af.location	= APPLY_HITROLL;
		affect_to_char(ch,&af);

		af.location	= APPLY_DAMROLL;
		affect_to_char(ch,&af);

		af.modifier	= UMAX(10,10 * (ch->level/5));
		af.location	= APPLY_AC;
		affect_to_char(ch,&af);
	}
	else {
		WAIT_STATE(ch, 2 * SKILL(gsn_tiger_power)->beats);
		ch->mana -= mana/2;
		char_puts("Your feel stregthen up, but nothing happens.\n\r",
			  ch);
		check_improve(ch, gsn_tiger_power, FALSE, 2);
	}
}

void do_hara(CHAR_DATA *ch, const char *argument)
{
	int chance;
	AFFECT_DATA  af;

	if (MOUNTED(ch)) {
		char_puts("You can't harakiri while riding!\n\r", ch);
		return;
	}

	if ((chance = get_skill(ch,gsn_hara_kiri)) == 0) {
		char_puts("You try to kill yourself, but you can't resist this ache.\n\r",ch);
		return;
	}

	if (is_affected(ch, gsn_hara_kiri)) {
		char_puts("If you want to kill yourself go and try to kill He-Man.\n\r",ch);
		return;
	}

	/* fighting */
	if (ch->position == POS_FIGHTING) {
		char_puts("Try your chance during fighting.\n\r", ch);
		return;
	}

	if (number_percent() < chance) {
		AFFECT_DATA af;

		WAIT_STATE(ch, SKILL(gsn_hara_kiri)->beats);

		ch->hit = 1;
		ch->mana = 1;
		ch->move = 1;

		if (ch->pcdata->condition[COND_HUNGER] < 40) 
			ch->pcdata->condition[COND_HUNGER] = 40; 
		if (ch->pcdata->condition[COND_THIRST] < 40) 
			ch->pcdata->condition[COND_THIRST] = 40; 

		char_puts("Yo cut your finger and wait till all your blood "
			  "finishes.\n\r",ch);
		act_puts("$n cuts his body and look in a deadly figure.",
			 ch, NULL, NULL, TO_ROOM, POS_FIGHTING);
		check_improve(ch, gsn_hara_kiri, TRUE, 2);
		do_sleep(ch, str_empty);
		SET_BIT(ch->act,PLR_HARA_KIRI);

		af.where     = TO_AFFECTS;
		af.type      = gsn_hara_kiri;
		af.level     = ch->level;
		af.duration  = 10;
		af.location  = APPLY_NONE;
		af.modifier  = 0;
		af.bitvector = 0;
		affect_to_char(ch, &af);
	}
	else {
		WAIT_STATE(ch, 2 * SKILL(gsn_hara_kiri)->beats);

		af.where     = TO_AFFECTS;
		af.type      = gsn_hara_kiri;
		af.level     = ch->level;
		af.duration  = 0;
		af.location  = APPLY_NONE;
		af.modifier  = 0;
		af.bitvector = 0;
		affect_to_char(ch, &af);

		char_puts("You couldn't cut your finger. "
			  "It is not so easy as you know.\n\r", ch);
		check_improve(ch, gsn_hara_kiri, FALSE, 2);
	}
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

	if ((chance = get_skill(ch, gsn_critical)) == 0)
		return dam;
	
	diceroll = number_range(0, 100);
	if (victim -> level > ch -> level)
		diceroll += (victim->level - ch->level) * 2;
	if (victim -> level < ch -> level)
		diceroll -= (ch->level - victim->level);
 
	if (diceroll <= chance /2)  {  
		check_improve(ch, gsn_critical, TRUE, 2);
		dam += dam * diceroll/200;
	}  

	if (diceroll > chance / 13)
		return dam;

	diceroll = number_percent();
	if (diceroll <= 75) {  
		act_puts("$n takes you down with a weird judo move!", 
		         ch, NULL, victim, TO_VICT, POS_RESTING);
		act_puts("You take $N down with a weird judo move!", 
			 ch, NULL, victim, TO_CHAR, POS_RESTING);
		check_improve(ch, gsn_critical, TRUE, 3);
		WAIT_STATE(victim, 2 * PULSE_VIOLENCE);
		dam += (dam * number_range(2, 5)) / 5;
		return dam;
	}   
	else if (diceroll > 75 && diceroll < 95) {   
		act_puts("You are blinded by $n's attack!", ch, NULL, victim, 
			TO_VICT ,POS_RESTING);
		act_puts("You blind $N with your attack!", ch, NULL, victim, 
			TO_CHAR,POS_RESTING);
		check_improve(ch, gsn_critical, TRUE, 4);
		if (!IS_AFFECTED(victim,AFF_BLIND)) {
			baf.type = gsn_dirt;
			baf.level = ch->level; 
			baf.location = APPLY_HITROLL; 
			baf.modifier = -4;
			baf.duration = number_range(1,5); 
			baf.bitvector = AFF_BLIND;
			affect_to_char(victim, &baf);
		}  
		dam += dam * number_range(1, 2);			
		return dam;
	} 

	act_puts("$n cuts out your heart! OUCH!!",  
		 ch, NULL, victim, TO_VICT ,POS_RESTING); 
	act_puts("You cut out $N's heart!  I bet that hurt!",  
		 ch, NULL, victim, TO_CHAR ,POS_RESTING);
	check_improve(ch, gsn_critical, TRUE, 5);
	dam += dam * number_range(2, 5);			
	return dam;
}  

void do_shield(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *victim;
	int chance,ch_weapon,vict_shield;
	OBJ_DATA *shield,*axe;

	if (IS_NPC(ch))
		return;

	if ((victim = ch->fighting) == NULL) {
		char_puts("You aren't fighting anyone.\n\r", ch);
		return;
	}
		
	if ((axe = get_eq_char(ch,WEAR_WIELD)) == NULL) {
		char_puts("You must be wielding a weapon.\n\r", ch);
		return;
	}

	if ((chance = get_skill(ch,gsn_shield_cleave)) == 0) {
		char_puts("You don't know how to cleave opponents's shield.\n\r",
			  ch);
		return;
	}

	if ((shield = get_eq_char(victim, WEAR_SHIELD)) == NULL) {
		char_puts("Your opponent must wield a shield.\n\r", ch);
		return;
	}

	if (check_material(shield,"platinum")
	||  shield->pIndexData->limit != -1)
		return;

	if (axe->value[0] == WEAPON_AXE)
		chance *= 1.2;
	else if (axe->value[0] != WEAPON_SWORD) {
		char_puts("Your weapon must be an axe or a sword.\n\r", ch);
		return;
	}

	/* find weapon skills */
	ch_weapon = get_weapon_skill(ch, get_weapon_sn(ch, WEAR_WIELD));
	vict_shield = get_skill(ch, gsn_shield_block);
	/* modifiers */

	/* skill */
	chance = chance * ch_weapon / 200;
	chance = chance * 100 / vict_shield;

	/* dex vs. strength */
	chance += get_curr_stat(ch, STAT_DEX);
	chance -= 2 * get_curr_stat(victim, STAT_STR);

	/* level */
/*	chance += (ch->level - victim->level) * 2; */
	chance += ch->level - victim->level;
	chance += axe->level - shield->level;
 
	/* and now the attack */
	SET_BIT(ch->affected_by, AFF_WEAK_STUN);
	WAIT_STATE(ch, SKILL(gsn_shield_cleave)->beats);
	if (number_percent() < chance) {
		act("You cleaved $N's shield into two.",
		    ch, NULL, victim, TO_CHAR);
		act("$n cleaved your shield into two.",
		    ch, NULL, victim, TO_VICT);
		act("$n cleaved $N's shield into two.",
		    ch, NULL, victim, TO_NOTVICT);
		check_improve(ch, gsn_shield_cleave, TRUE, 1);
		extract_obj(get_eq_char(victim, WEAR_SHIELD));
	}
	else {
		act("You fail to cleave $N's shield.",
		    ch, NULL, victim, TO_CHAR);
		act("$n tries to cleave your shield, but fails.",
		    ch, NULL, victim, TO_VICT);
		act("$n tries to cleave $N's shield, but fails.",
		    ch, NULL, victim, TO_NOTVICT);
		check_improve(ch, gsn_shield_cleave, FALSE, 1);
	}
}

void do_weapon(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *victim;
	OBJ_DATA *wield,*axe;
	int chance,ch_weapon,vict_weapon;

	if (IS_NPC(ch))
		return;

	if ((victim = ch->fighting) == NULL) {
		char_puts("You aren't fighting anyone.\n\r", ch);
		return;
	}

	if ((axe = get_eq_char(ch, WEAR_WIELD)) == NULL) {
		char_puts("You must be wielding a weapon.\n\r",ch);
		return;
	}

	if ((chance = get_skill(ch, gsn_weapon_cleave)) == 0) {
		char_puts("You don't know how to cleave opponents's weapon.\n\r",ch);
		return;
	}

	if ((wield = get_eq_char(victim, WEAR_WIELD)) == NULL) {
		char_puts("Your opponent must wield a weapon.\n\r", ch);
		return;
	}

	if (check_material(wield,"platinum") || wield->pIndexData->limit != -1)
		return;


	if (axe->value[0] == WEAPON_AXE)
		chance *= 1.2;
	else if (axe->value[0] != WEAPON_SWORD) {
		char_puts("Your weapon must be an axe or a sword.\n\r",ch);
		return;
	}

	/* find weapon skills */
	ch_weapon = get_weapon_skill(ch, get_weapon_sn(ch, WEAR_WIELD));
	vict_weapon = get_weapon_skill(victim, get_weapon_sn(victim, WEAR_WIELD));
	/* modifiers */

	/* skill */
	chance = chance * ch_weapon / 200;
	chance = chance * 100 / vict_weapon;

	/* dex vs. strength */
	chance += get_curr_stat(ch, STAT_DEX) + get_curr_stat(ch, STAT_STR);
	chance -= get_curr_stat(victim, STAT_STR) +
				2 * get_curr_stat(victim, STAT_DEX);

	chance += ch->level - victim->level;
	chance += axe->level - wield->level;
 
	/* and now the attack */
	SET_BIT(ch->affected_by,AFF_WEAK_STUN);
	WAIT_STATE(ch, SKILL(gsn_weapon_cleave)->beats);
	if (number_percent() < chance) {
		act("You cleaved $N's weapon into two.",
		    ch, NULL, victim, TO_CHAR);
		act("$n cleaved your weapon into two.",
		    ch, NULL, victim, TO_VICT);
		act("$n cleaved $N's weapon into two.",
		    ch, NULL, victim, TO_NOTVICT);
		check_improve(ch, gsn_weapon_cleave, TRUE, 1);
		extract_obj(get_eq_char(victim, WEAR_WIELD));
	}
	else {
		act("You fail to cleave $N's weapon.",
		    ch, NULL, victim, TO_CHAR);
		act("$n tries to cleave your weapon, but fails.",
		    ch, NULL, victim, TO_VICT);
		act("$n tries to cleave $N's weapon, but fails.",
		    ch, NULL, victim, TO_NOTVICT);
		check_improve(ch, gsn_weapon_cleave, FALSE, 1);
	}
}

void do_tail(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int chance, wait;
	int damage_tail;

	if (MOUNTED(ch)) {
		char_puts("You can't tail while riding!\n\r", ch);
		return;
	}

	one_argument(argument, arg);
 
	if ((chance = get_skill(ch, gsn_tail)) == 0) {
		char_puts("Huh?\n\r", ch);
		return;
	}
 
	WAIT_STATE(ch, SKILL(gsn_cleave)->beats);

	if (arg[0] == '\0') {
		victim = ch->fighting;
		if (victim == NULL) {
			char_puts("But you aren't fighting anyone!\n\r", ch);
			return;
		}
	}
	else if ((victim = get_char_room(ch,arg)) == NULL) {
		char_puts("They aren't here.\n\r", ch);
		return;
	}
/*
	if (victim->position < POS_FIGHTING) {
		act("You'll have to let $M get back up first.",
		    ch, NULL, victim, TO_CHAR);
		return;
	} 
*/
	if (victim == ch) {
		char_puts("You try to hit yourself by your tail, but failed.\n\r",
			  ch);
		return;
	}

	if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim) {
		act("But $N is your friend!", ch, NULL, victim, TO_CHAR);
		return;
	}
		
	if (is_affected(victim, gsn_protective_shield)) {
		act_puts("Your tail seems to slide around $N.",
			 ch, NULL, victim, TO_CHAR, POS_FIGHTING);
		act_puts("$n's tail slides off your protective shield.",
			 ch, NULL, victim, TO_VICT, POS_FIGHTING);
		act_puts("$n's tail seems to slide around $N.",
			 ch, NULL, victim, TO_NOTVICT, POS_FIGHTING);
		return;
	}

	if (is_safe(ch, victim))
		return;

	/* modifiers */

	/* size  and weight */
	chance -= ch->carry_weight / 20;
	chance += victim->carry_weight / 25;

	if (ch->size < victim->size)
		chance += (ch->size - victim->size) * 25;
	else
		chance += (ch->size - victim->size) * 10; 

	/* stats */
	chance += get_curr_stat(ch, STAT_STR) + get_curr_stat(ch, STAT_DEX);
	chance -= get_curr_stat(victim, STAT_DEX) * 2;

	if (IS_AFFECTED(ch, AFF_FLYING))
		chance -= 10;

	/* speed */
	if (IS_SET(ch->off_flags, OFF_FAST))
		chance += 20;
	if (IS_SET(victim->off_flags, OFF_FAST))
		chance -= 30;

	/* level */
	chance += (ch->level - victim->level) * 2;

	RESET_WAIT_STATE(ch);

	/* now the attack */
	if (number_percent() < (chance / 4)) {
		act("$n sends you sprawling with a powerful tail!",
		    ch, NULL, victim, TO_VICT);
		act("You sprawle $N with your tail , and send $M flying!",
		    ch, NULL, victim, TO_CHAR);
		act("$n sends $N sprawling with a powerful tail.",
		    ch, NULL, victim, TO_NOTVICT);
		check_improve(ch, gsn_tail, TRUE, 1);

		wait = 3;

		switch(number_bits(2)) {
		case 0: wait = 1; break;
		case 1: wait = 2; break;
		case 2: wait = 4; break;
		case 3: wait = 3; break;
		}

		WAIT_STATE(victim, wait * PULSE_VIOLENCE);
		WAIT_STATE(ch, SKILL(gsn_tail)->beats);
		victim->position = POS_RESTING;
		damage_tail = ch->damroll + 
			(2 * number_range(4,4 + 10* ch->size + chance/10));
		damage(ch, victim, damage_tail, gsn_tail, DAM_BASH, TRUE);
	}
	else {
		damage(ch,victim,0,gsn_tail,DAM_BASH, TRUE);
		act("You lost your position and fall down!",
		    ch, NULL, victim, TO_CHAR);
		act("$n lost $s position and fall down!.",
		    ch, NULL, victim, TO_NOTVICT);
		act("You evade $n's tail, causing $m to fall down.",
		    ch, NULL, victim, TO_VICT);
		check_improve(ch,gsn_tail,FALSE,1);
		ch->position = POS_RESTING;
		WAIT_STATE(ch, SKILL(gsn_tail)->beats * 3/2); 
	}
}

void do_concentrate(CHAR_DATA *ch, const char *argument)
{
	int chance;
	int mana;

	if (MOUNTED(ch)) {
		char_puts("You can't concentrate while riding!\n\r", ch);
		return;
	}

	if ((chance = get_skill(ch,gsn_concentrate)) == 0) {
		char_puts("You try to concentrate on what is going on.\n\r",
			  ch);
		return;
	}

	if (is_affected(ch,gsn_concentrate)) {
		char_puts("You are already concentrated for the fight.\n\r",
			  ch);
		return;
	}
		
	mana = SKILL(gsn_concentrate)->min_mana;
	if (ch->mana < mana) {
		char_puts("You can't get up enough energy.\n\r", ch);
		return;
	}

	/* fighting */
	if (ch->fighting) {
		char_puts("Concentrate on your fighting!\n\r", ch);
		return;
	}

	WAIT_STATE(ch, SKILL(gsn_concentrate)->beats);
	if (number_percent() < chance) {
		AFFECT_DATA af;

		ch->mana -= mana;
		ch->move /= 2;

		do_sit(ch,str_empty);
		char_puts("You sit down and relax, "
			  "concentrating on the next fight.!\n\r", ch);
		act_puts("$n concentrates for the next fight.",
			 ch, NULL, NULL, TO_ROOM, POS_FIGHTING);
		check_improve(ch,gsn_concentrate,TRUE,2);

		af.where	= TO_AFFECTS;
		af.type		= gsn_concentrate;
		af.level	= ch->level;
		af.duration	= number_fuzzy(ch->level / 8);
		af.modifier	= UMAX(1,ch->level/8);
		af.bitvector 	= 0;

		af.location	= APPLY_HITROLL;
		affect_to_char(ch,&af);

		af.location	= APPLY_DAMROLL;
		affect_to_char(ch,&af);

		af.modifier	= UMAX(1,ch->level/10);
		af.location	= APPLY_AC;
		affect_to_char(ch,&af);
	}
	else {
		ch->mana -= mana/2;
		char_puts("You try to concentrate for the next fight but fail.\n\r",
			  ch);
		check_improve(ch, gsn_concentrate, FALSE, 2);
	}
}

void do_bandage(CHAR_DATA *ch, const char *argument)
{
	int heal;
	int chance;

	if ((chance = get_skill(ch,gsn_bandage)) == 0) {
		char_puts("Huh?\n\r", ch);
		return;
	}

	if (IS_AFFECTED(ch, AFF_REGENERATION)) {
		char_puts("You have already using your bandage.\n\r", ch);
		return;
	}

	WAIT_STATE(ch, SKILL(gsn_bandage)->beats);
	if (number_percent() < chance) {
		AFFECT_DATA af;

		char_puts("You place your bandage to your shoulder!\n\r", ch);
		act("$n places a bandage to $s shulder.",
		    ch, NULL, NULL, TO_ROOM);
		check_improve(ch, gsn_bandage, TRUE, 2);

		heal = dice(4, 8) + ch->level / 2;
		ch->hit = UMIN(ch->hit + heal, ch->max_hit);
		update_pos(ch);
		char_puts("You feel better!\n\r", ch);

		af.where	= TO_AFFECTS;
		af.type		= gsn_bandage;
		af.level	= ch->level;
		af.duration	= ch->level / 10;
		af.modifier	= UMIN(15,ch->level/2);
		af.bitvector 	= AFF_REGENERATION;
		af.location	= 0;
		affect_to_char(ch,&af);
	}
	else {
		char_puts("You failed to place your bandage to your shoulder.\n\r",
			  ch);
		check_improve(ch, gsn_bandage, FALSE, 2);
	}
}

void do_katana(CHAR_DATA *ch, const char *argument)
{
	OBJ_DATA *katana;
	AFFECT_DATA af;
	OBJ_DATA *part;
	char arg[MAX_INPUT_LENGTH];
	int chance;
	int mana;

	one_argument(argument, arg);
	
	if ((chance = get_skill(ch, gsn_katana)) == 0) {
		char_puts("Huh?\n\r", ch);
		return;
	}
	
	if (is_affected(ch, gsn_katana)) {
		char_puts("But you've already got one katana!\n\r", ch);
		return;
	}
	
	mana = SKILL(gsn_katana)->min_mana;
	if (ch->mana < mana) {
		char_puts("You feel too weak to concentrate on a katana.\n\r",
			  ch);
		return;
	}
	
	if (arg[0] == '\0') {
		char_puts("Make a katana from what?\n\r", ch);
		return;
	}
	
	if ((part = get_obj_carry(ch, arg)) == NULL) {
		char_puts("You do not have chunk of iron.\n\r", ch);
		return;
	}

	if (part->pIndexData->vnum != OBJ_VNUM_CHUNK_IRON) {
		char_puts("You do not have the right material.\n\r", ch);
		return;
	}

	if (number_percent() < chance / 3 * 2) {
		char_puts("You failed and destroyed it.\n\r", ch);
		extract_obj(part);
		return;
	} 

	WAIT_STATE(ch, SKILL(gsn_katana)->beats);

	if (number_percent() < chance) {
		af.where	= TO_AFFECTS;
		af.type		= gsn_katana;
		af.level	= ch->level;
		af.duration	= ch->level;
		af.modifier	= 0;
		af.bitvector 	= 0;      
		af.location	= 0;
		affect_to_char(ch,&af);
	
		katana = create_obj(get_obj_index(OBJ_VNUM_KATANA_SWORD),
				    ch->level);
		katana->cost  = 0;
		katana->level = ch->level;
		ch->mana -= mana;

		af.where	= TO_OBJECT;
		af.type 	= gsn_katana;
		af.level	= ch->level;
		af.duration	= -1;
		af.location	= APPLY_DAMROLL;
		af.modifier	= ch->level / 10;
		af.bitvector	= 0;
		affect_to_obj(katana, &af);

		af.location	= APPLY_HITROLL;
		affect_to_obj(katana, &af);

		katana->value[2] = ch->level / 10;
		katana->ed = ed_new2(katana->pIndexData->ed, ch->name);
			
		obj_to_char(katana, ch);
		check_improve(ch, gsn_katana, TRUE, 1);
			
		act("You make a katana from $p!",ch,part,NULL,TO_CHAR);
		act("$n makes a katana from $p!",ch,part,NULL,TO_ROOM);
			
		extract_obj(part);
		return;
	}
	else {
		char_puts("You destroyed it.\n\r", ch);
		extract_obj(part);
		ch->mana -= mana/2;
		check_improve(ch, gsn_katana, FALSE, 1);
	}
}

void do_crush(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *victim;
	int chance, wait;
	int damage_crush;

	if (MOUNTED(ch)) 
		return;

	if ((chance = get_skill(ch, gsn_crush)) == 0) {
		char_puts("You don't know how to crush.\n\r", ch);
		return;
	}
 
	if ((victim = ch->fighting)== NULL)
		return;

	if (victim->position < POS_FIGHTING)
		return;

	if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
		return;
		
	if (is_affected(victim, gsn_protective_shield)) {
		act_puts("Your crush seems to slide around $N.",
			 ch, NULL, victim, TO_CHAR, POS_FIGHTING);
		act_puts("$n's crush slides off your protective shield.",
			 ch, NULL, victim, TO_VICT, POS_FIGHTING);
		act_puts("$n's crush seems to slide around $N.",
			 ch, NULL, victim, TO_NOTVICT, POS_FIGHTING);
		return;
	}
		
	if (is_safe(ch, victim))
		return;

	/* modifiers */

	/* size  and weight */
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
	if (IS_SET(ch->off_flags, OFF_FAST))
		chance += 10;
	if (IS_SET(victim->off_flags, OFF_FAST))
		chance -= 20;

	/* level */
	chance += (ch->level - victim->level) * 2;

	/* now the attack */
	if (number_percent() < chance) {
		act("$n squezes you with a powerful crush!",
		    ch, NULL, victim, TO_VICT);
		act("You slam into $N, and crushes $M!",
		    ch, NULL, victim, TO_CHAR);
		act("$n squezes $N with a powerful crush.",
		    ch, NULL, victim, TO_NOTVICT);

		wait = 3;

		switch(number_bits(2)) {
		case 0: wait = 1; break;
		case 1: wait = 2; break;
		case 2: wait = 4; break;
		case 3: wait = 3; break;
		}

		WAIT_STATE(victim, wait * PULSE_VIOLENCE);
		WAIT_STATE(ch, SKILL(gsn_crush)->beats);
		victim->position = POS_RESTING;
		damage_crush = (ch->damroll / 2) +
				number_range(4, 4 + 4*ch->size + chance/10);
		damage(ch, victim, damage_crush, gsn_crush, DAM_BASH, TRUE);
	}
	else {
		damage(ch, victim, 0, gsn_crush, DAM_BASH, TRUE);
		act("You fall flat on your face!",
		    ch, NULL, victim, TO_CHAR);
		act("$n falls flat on $s face.",
		    ch, NULL, victim, TO_NOTVICT);
		act("You evade $n's crush, causing $m to fall flat on $s face.",
		    ch, NULL, victim, TO_VICT);
		ch->position = POS_RESTING;
		WAIT_STATE(ch, SKILL(gsn_crush)->beats * 3/2); 
	}
}

void do_sense(CHAR_DATA *ch, const char *argument)
{
	int chance;
	int mana;

	if ((chance = get_skill(ch, gsn_sense_life)) == 0) {
		char_puts("Huh?\n\r", ch);
		return;
	}

	if (is_affected(ch,gsn_sense_life)) {
		char_puts("You can already feel life forms.\n\r", ch);
		return;
	}

	mana = SKILL(gsn_sense_life)->min_mana;
	if (ch->mana < mana) {
		char_puts("You cannot seem to concentrate enough.\n\r", ch);
		return;
	}

	WAIT_STATE(ch, SKILL(gsn_sense_life)->beats);

	if (number_percent() < chance) {
		AFFECT_DATA af;
		
		af.where	= TO_AFFECTS;
		af.type 	= gsn_sense_life;
		af.level 	= ch->level;
		af.duration	= ch->level;
		af.location	= APPLY_NONE;
		af.modifier	= 0;
		af.bitvector	= AFF_DETECT_LIFE;
		affect_to_char(ch, &af);

		ch->mana -= mana;

		act("You start to sense life forms in the room!",
		    ch, NULL, NULL, TO_CHAR);
		act("$n looks more sensitive.", ch, NULL, NULL, TO_ROOM);
		check_improve(ch, gsn_sense_life, TRUE, 1);
	}
	else {
		ch->mana -= mana/2;
		char_puts("You failed.\n\r" ,ch);
		check_improve(ch, gsn_sense_life, FALSE, 1);
	}
}

void do_poison_smoke(CHAR_DATA *ch, const char *argument) 
{
	int chance;
	int mana;
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;

	if ((chance = get_skill(ch, gsn_poison_smoke)) == 0) {
		char_puts("Huh?\n\r", ch);
		return;
	}

	mana = SKILL(gsn_poison_smoke)->min_mana;
	if (ch->mana < mana) {
		char_puts("You can't get up enough energy.\n\r", ch);
		return;
	}
	ch->mana -= mana;
	WAIT_STATE(ch, SKILL(gsn_poison_smoke)->beats);

	if (number_percent() > chance) {
		char_puts("You failed.\n\r", ch);
		check_improve(ch, gsn_poison_smoke, FALSE, 1);
		return;
	}

	char_puts("A cloud of poison smoke fills the room.\n\r", ch);
	act("A cloud of poison smoke fills the room.", ch, NULL, NULL, TO_ROOM);

	check_improve(ch, gsn_poison_smoke, TRUE, 1);

	for (vch = ch->in_room->people; vch; vch = vch_next) {
		vch_next = vch->next_in_room;

		if (is_safe_spell(ch, vch, TRUE))
			continue;

		spell_poison(gsn_poison, ch->level, ch, vch, TARGET_CHAR);
		if (vch != ch)
			multi_hit(vch, ch, TYPE_UNDEFINED);
	}
}

void do_blindness_dust(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;
	int chance;
	int mana;

	if ((chance = get_skill(ch, gsn_blindness_dust)) == 0) {
		char_puts("Huh?\n\r", ch);
		return;
	}

	mana = SKILL(gsn_blindness_dust)->min_mana;
	if (ch->mana < mana) {
		char_puts("You can't get up enough energy.\n\r", ch);
		return;
	}

	ch->mana -= mana;
	WAIT_STATE(ch, SKILL(gsn_blindness_dust)->beats);
	if (number_percent() > chance) {
		char_puts("You failed.\n\r",ch);
		check_improve(ch,gsn_blindness_dust,FALSE,1);
		return;
	}

	char_puts("A cloud of dust fills in the room.\n\r", ch);
	act("A cloud of dust fills the room.", ch, NULL, NULL, TO_ROOM);

	check_improve(ch, gsn_blindness_dust, TRUE, 1);

	for (vch = ch->in_room->people; vch; vch = vch_next) {
		vch_next = vch->next_in_room;

		if (is_safe_spell(ch, vch, TRUE)) 
			continue;

		spell_blindness(gsn_blindness, ch->level, ch, vch, TARGET_CHAR);
		if (vch != ch)
			multi_hit(vch, ch, TYPE_UNDEFINED);
	}
}
