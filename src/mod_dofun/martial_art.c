/*
 * $Id: martial_art.c,v 1.118 1999-10-18 18:08:02 avn Exp $
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
#include "mob_prog.h"

#ifdef SUNOS
#	include <stdarg.h>
#	include "compat/compat.h"
#endif

DECLARE_DO_FUN(do_yell		);
DECLARE_DO_FUN(do_sleep		);
DECLARE_DO_FUN(do_sit		);
DECLARE_DO_FUN(do_bash_door	);
DECLARE_DO_FUN(do_dismount	);

static inline bool	check_yell	(CHAR_DATA *ch, CHAR_DATA *victim,
					 bool fighting);

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

	if ((chance = get_skill(ch, "mortal strike"))
	&&  get_eq_char(ch, WEAR_WIELD)) {
		chance /= 30;
		chance += 1 + (LEVEL(ch) - LEVEL(victim)) / 2;
		if (number_percent() < chance) {
			act_puts("Your flash strike instantly slays $N!",
				 ch, NULL, victim, TO_CHAR, POS_RESTING);
			act_puts("$n flash strike instantly slays $N!",
				 ch, NULL, victim, TO_NOTVICT,
				 POS_RESTING);
			act_puts("$n flash strike instantly slays you!",
				 ch, NULL, victim, TO_VICT, POS_DEAD);
			damage(ch, victim, (victim->hit + 1),
			       "mortal strike", DAM_NONE, DAMF_SHOW);
			check_improve(ch, "mortal strike", TRUE, 1);
			return;
		} else
			check_improve(ch, "mortal strike", FALSE, 3);
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
	||  (IS_NPC(ch) && IS_SET(ch->pMobIndex->act, ACT_PET)))
		return;

	if ((victim = get_char_room(ch, arg)) == NULL) {
		WAIT_STATE(ch, MISSING_TARGET_DELAY);
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

	WAIT_STATE(ch, 1 * PULSE_VIOLENCE);

	if ((chance = get_skill(ch, "mortal strike"))
	&&  get_eq_char(ch, WEAR_WIELD)) {
		chance /= 30;
		chance += 1 + (LEVEL(ch) - LEVEL(victim)) / 2;
		if (number_percent() < chance) {
			act_puts("Your flash strike instantly slays $N!",
				 ch, NULL, victim, TO_CHAR, POS_RESTING);
			act_puts("$n flash strike instantly slays $N!",
				 ch, NULL, victim, TO_NOTVICT,
				 POS_RESTING);
			act_puts("$n flash strike instantly slays you!",
				 ch, NULL, victim, TO_VICT, POS_DEAD);
			damage(ch, victim, (victim->hit + 1),
			       "mortal strike", DAM_NONE, DAMF_SHOW);
			check_improve(ch, "mortal strike", TRUE, 1);
			return;
		} else
			check_improve(ch, "mortal strike", FALSE, 3);
	}

	multi_hit(ch, victim, TYPE_UNDEFINED);
	yell(victim, ch, "Help! $lu{$i} is attacking me!");
}

void do_flee(CHAR_DATA *ch, const char *argument)
{
	ROOM_INDEX_DATA *was_in;
	ROOM_INDEX_DATA *now_in;
	CHAR_DATA *victim;
	int attempt;

	if (RIDDEN(ch)) {
		char_puts("You should ask to your rider!\n", ch);
		return;
	}

	if (MOUNTED(ch))
		dofun("dismount", ch, str_empty);

	if ((victim = ch->fighting) == NULL) {
		if (ch->position == POS_FIGHTING)
			ch->position = POS_STANDING;
		char_puts("You aren't fighting anyone.\n", ch);
		return;
	}

	if (!can_flee(ch)) {
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
		     || pexit->to_room.r == NULL
		     || (IS_SET(pexit->exit_info, EX_CLOSED)
		         && (!IS_AFFECTED(ch, AFF_PASS_DOOR)
		             || IS_SET(pexit->exit_info,EX_NOPASS))
		             && !IS_TRUSTED(ch, LEVEL_IMMORTAL))
		         || (IS_SET(pexit->exit_info , EX_NOFLEE))
		         || (IS_NPC(ch)
		             && IS_SET(pexit->to_room.r->room_flags, ROOM_NOMOB)))
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
		} else {
			/* Once fled, the mob will not go after */
			NPC(ch)->last_fought = NULL;
		}

		stop_fighting(ch, TRUE);
		return;
	}

	char_puts("PANIC! You couldn't escape!\n", ch);
}

/*
 * Disarm a creature.
 * Caller must check for successful attack.
 */
void disarm(CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj)
{
	OBJ_DATA *obj2;
	int skill;

	if (IS_OBJ_STAT(obj, ITEM_NOREMOVE)) {
		act("$S weapon won't budge!", ch, NULL, victim, TO_CHAR);
		act("$n tries to disarm you, but your weapon won't budge!",
		    ch, NULL, victim, TO_VICT);
		act("$n tries to disarm $N, but fails.",
		    ch, NULL, victim, TO_NOTVICT);
		return;
	}

	if ((skill = get_skill(victim, "grip"))) {
		skill += (get_curr_stat(victim, STAT_STR) -
			  get_curr_stat(ch, STAT_STR)) * 5;
		if (number_percent() < skill) {
			act("$N grips and prevent you to disarm $M!",
			    ch, NULL, victim, TO_CHAR);
			act("$n tries to disarm you, but you grip and escape!",
			    ch, NULL, victim, TO_VICT);
			act("$n tries to disarm $N, but fails.",
			    ch, NULL, victim, TO_NOTVICT);
			check_improve(victim, "grip", TRUE, 1);
			return;
		}
		else
			check_improve(victim, "grip", FALSE, 1);
	}

	act_puts("$n DISARMS you and sends your weapon flying!", 
		 ch, NULL, victim, TO_VICT, POS_FIGHTING);
	act_puts("You disarm $N!", ch,NULL, victim, TO_CHAR, POS_FIGHTING);
	act_puts("$n disarms $N!", ch, NULL, victim, TO_NOTVICT, POS_FIGHTING);

	obj_from_char(obj);
	if (IS_OBJ_STAT(obj, ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_INVENTORY))
		obj_to_char(obj, victim);
	else {
		obj_to_room(obj, victim->in_room);
		if (IS_NPC(victim)
		&&  victim->wait == 0
		&&  can_see_obj(victim,obj))
			get_obj(victim, obj, NULL, NULL);
	}

	if ((obj2 = get_eq_char(victim, WEAR_SECOND_WIELD)) != NULL) {
		act_puts("You wield your second weapon as your first!.",
			 ch, NULL, victim, TO_VICT, POS_FIGHTING);
		act_puts("$N wields his second weapon as first!",
			 ch, NULL, victim, TO_CHAR, POS_FIGHTING);
		act_puts("$N wields his second weapon as first!",
			 ch, NULL, victim, TO_NOTVICT, POS_FIGHTING);
		unequip_char(victim, obj2);
		equip_char(victim, obj2, WEAR_WIELD);
	}
}

void do_berserk(CHAR_DATA *ch, const char *argument)
{
	int chance, hp_percent;

	if ((chance = get_skill(ch, "berserk")) == 0) {
		char_puts("You turn red in the face, but nothing happens.\n",
			  ch);
		return;
	}

	if (IS_AFFECTED(ch, AFF_BERSERK)
	||  is_affected(ch, "berserk")
	||  is_affected(ch, "frenzy")) {
		char_puts("You get a little madder.\n", ch);
		return;
	}

	if (IS_AFFECTED(ch, AFF_CALM)) {
		char_puts("You're feeling too mellow to berserk.\n", ch);
		return;
	}

	if (ch->mana < 50) {
		char_puts("You can't get up enough energy.\n", ch);
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
		ch->hit += LEVEL(ch) * 2;
		ch->hit = UMIN(ch->hit,ch->max_hit);

		char_puts("Your pulse races as you are consumned by rage!\n",
			  ch);
		act_puts("$n gets a wild look in $s eyes.",
			 ch, NULL, NULL, TO_ROOM, POS_FIGHTING);
		check_improve(ch, "berserk", TRUE, 2);

		af.where	= TO_AFFECTS;
		af.type		= "berserk";
		af.level	= ch->level;
		af.duration	= number_fuzzy(ch->level / 8);
		af.modifier	= UMAX(1,LEVEL(ch)/5);
		af.bitvector 	= AFF_BERSERK;

		af.location	= APPLY_HITROLL;
		affect_to_char(ch,&af);

		af.location	= APPLY_DAMROLL;
		affect_to_char(ch,&af);

		af.modifier	= UMAX(10,10 * (LEVEL(ch)/5));
		af.location	= APPLY_AC;
		affect_to_char(ch,&af);
	}
	else {
		WAIT_STATE(ch,2 * PULSE_VIOLENCE);
		ch->mana -= 25;
		ch->move /= 2;

		char_puts("Your pulse speeds up, but nothing happens.\n",
			  ch);
		check_improve(ch, "berserk", FALSE, 2);
	}
}

void do_breath(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *victim;
	int chance;
	char *spell;
	int mana;

	if ((chance = get_skill(ch, "breath")) == 0) {
		char_puts("Huh?", ch);
		return;
	}

	if ((victim = ch->fighting) == NULL) {
		char_puts("You aren't fighting anyone.", ch);
		return;
	}

	if (IS_RACE(ch->race, "blue dragon"))
		spell = "lightning breath";
	else if (IS_RACE(ch->race, "red dragon"))
		spell = "fire breath";
	else if (IS_RACE(ch->race, "white dragon"))
		spell = "frost breath";
	else if (IS_RACE(ch->race, "black dragon"))
		spell = "acid breath";
	else if (IS_RACE(ch->race, "green dragon"))
		spell = "gas breath";
	else {
		act("You are not a dragon.", ch, NULL, NULL, TO_CHAR);
		return;
	}
	
	mana = skill_mana(ch, "breath");
	if (ch->mana < mana) {
		act("You do not have enough energy.", ch, NULL, NULL, TO_CHAR);
		return;
	}

	if (number_percent() > chance) {
		char_puts("You failed.", ch);
		ch->mana -= mana/2;
		check_improve(ch, "breath", FALSE, 1);
		return;
	}
	ch->mana -= mana;
	spellfun_call(spell, NULL, LEVEL(ch), ch, victim);
	WAIT_STATE(ch, skill_beats("breath"));
}

void do_bash(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int chance, wait;
	bool attack = FALSE;
	int damage_bash;

	if (MOUNTED(ch)) {
		char_puts("You can't bash while riding!\n", ch);
		return;
	}
	
	argument = one_argument(argument, arg, sizeof(arg));
 
	if ((chance = get_skill(ch, "bash")) == 0) {
		char_puts("Bashing? What's that?\n", ch);
		return;
	}
 
	if (arg[0] != '\0' && !str_cmp(arg, "door")) {
		do_bash_door(ch, argument);
		return;
	}

	if (arg[0] == '\0') {
		victim = ch->fighting;
		if (victim == NULL) {
			char_puts("But you aren't fighting anyone!\n", ch);
			return;
		}
	}
	else 
		victim = get_char_room(ch, arg);

	if (!victim || victim->in_room != ch->in_room) {
		WAIT_STATE(ch, MISSING_TARGET_DELAY);
		char_puts("They aren't here.\n", ch);
		return;
	}

	WAIT_STATE(ch, skill_beats("bash"));

	if (victim->position < POS_FIGHTING) {
		act("You'll have to let $M get back up first.",
			  ch, NULL, victim, TO_CHAR);
		return;
	} 

	if (victim == ch) {
		char_puts("You try to bash your brains out, but fail.\n",
			     ch);
		return;
	}

	if (MOUNTED(victim)) {
		char_puts("You can't bash a riding one!\n", ch);
		return;
	}

	if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim) {
		act("But $N is your friend!", ch, NULL, victim, TO_CHAR);
		return;
	}
		
	if (is_affected(victim, "protective shield")) {
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
	chance += get_curr_stat(ch, STAT_STR);
	chance -= get_curr_stat(victim, STAT_DEX) * 4/3;

	if (IS_AFFECTED(ch, AFF_FLYING))
		chance -= 10;

	/* speed */
	if (IS_NPC(ch) && IS_SET(ch->pMobIndex->off_flags, OFF_FAST))
		chance += 10;
	if (IS_NPC(victim) && IS_SET(victim->pMobIndex->off_flags, OFF_FAST))
		chance -= 20;

	/* level */
	chance += (LEVEL(ch) - LEVEL(victim)) * 2;

	RESET_WAIT_STATE(ch);
	attack = !(ch->fighting == victim);

	/* now the attack */
	if (number_percent() < chance) {
		act("$n sends you sprawling with a powerful bash!",
		    ch, NULL, victim, TO_VICT);
		act("You slam into $N, and send $M flying!",
		    ch, NULL, victim, TO_CHAR);
		act("$n sends $N sprawling with a powerful bash.",
		    ch, NULL, victim, TO_NOTVICT);
		check_improve(ch, "bash", TRUE, 1);

		wait = 3;

		switch(number_bits(2)) {
			case 0: wait = 1; break;
			case 1: wait = 2; break;
			case 2: wait = 4; break;
			case 3: wait = 3; break;
		}

		WAIT_STATE(victim, wait * PULSE_VIOLENCE);
		WAIT_STATE(ch, skill_beats("bash"));
		victim->position = POS_RESTING;
		damage_bash = (ch->damroll / 2) +
			      number_range(4, 4 + 4* ch->size + chance/10);
		damage(ch, victim, damage_bash, "bash", DAM_BASH, TRUE);
	}
	else {
		damage(ch, victim, 0, "bash", DAM_BASH, TRUE);
		act_puts("You fall flat on your face!",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		act("$n falls flat on $s face.",
		    ch, NULL, victim, TO_NOTVICT);
		act("You evade $n's bash, causing $m to fall flat on $s face.",
		    ch, NULL, victim, TO_VICT);
		check_improve(ch, "bash", FALSE, 1);
		ch->position = POS_RESTING;
		WAIT_STATE(ch, skill_beats("bash") * 3/2); 
	}

	if (attack)
		yell(victim, ch, "Help! $lu{$i} is bashing me!");
}

void do_dirt(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	bool attack;
	int chance;

	if (MOUNTED(ch)) {
		char_puts("You can't dirt while riding!\n", ch);
		return;
	}

	one_argument(argument, arg, sizeof(arg));

	if ((chance = get_skill(ch, "dirt kicking")) == 0) {
		char_puts("You get your feet dirty.\n", ch);
		return;
	}

	if (arg[0] == '\0') {
		victim = ch->fighting;
		if (victim == NULL) {
			char_puts("But you aren't in combat!\n", ch);
			return;
		}
	}
	else 
		victim = get_char_room(ch, arg);

	if (!victim || victim->in_room != ch->in_room) {
		WAIT_STATE(ch, MISSING_TARGET_DELAY);
		char_puts("They aren't here.\n", ch);
		return;
	}

	WAIT_STATE(ch, skill_beats("dirt kicking"));

	if (IS_AFFECTED(ch, AFF_FLYING)) {
		 char_puts("While flying?\n", ch);
		 return;
	}

	if (IS_AFFECTED(victim, AFF_BLIND)) {
		act("$e's already been blinded.", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (victim == ch) {
		char_puts("Very funny.\n", ch);
		return;
	}

	if (MOUNTED(victim)) {
		char_puts("You can't dirt a riding one!\n", ch);
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
	if ((IS_NPC(ch) && IS_SET(ch->pMobIndex->off_flags, OFF_FAST))
	||  IS_AFFECTED(ch, AFF_HASTE))
		chance += 10;
	if ((IS_NPC(victim) && IS_SET(victim->pMobIndex->off_flags, OFF_FAST))
	||  IS_AFFECTED(victim, AFF_HASTE))
		chance -= 25;

	/* level */
	chance += (LEVEL(ch) - LEVEL(victim)) * 2;

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
		char_puts("There isn't any dirt to kick.\n",ch);
		return;
	}

	attack = (ch->fighting != victim);

	/* now the attack */
	if (number_percent() < chance) {
		AFFECT_DATA af;
		act("$n is blinded by the dirt in $s eyes!",
		    victim, NULL, NULL, TO_ROOM);
		char_puts("You can't see a thing!\n", victim);
		check_improve(ch, "dirt kicking", TRUE, 2);

		af.where	= TO_AFFECTS;
		af.type 	= "dirt kicking";
		af.level 	= ch->level;
		af.duration	= 0;
		af.location	= APPLY_HITROLL;
		af.modifier	= -4;
		af.bitvector 	= AFF_BLIND;

		affect_to_char(victim, &af);
		damage(ch, victim, number_range(2, 5),
		       "dirt kicking", DAM_NONE, FALSE);
	}
	else {
		damage(ch, victim, 0, "dirt kicking", DAM_NONE, TRUE);
		check_improve(ch, "dirt kicking", FALSE, 2);
	}

	if (attack)
		yell(victim, ch, "Help! $lu{$i} just kicked dirt into my eyes!");
}

void do_trip(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	bool attack;
	int chance;
	int beats;

	if (MOUNTED(ch)) {
		char_puts("You can't trip while riding!\n", ch);
		return;
	}

	one_argument(argument, arg, sizeof(arg));

	if ((chance = get_skill(ch, "trip")) == 0) {
		char_puts("Tripping? What's that?\n", ch);
		return;
	}

	if (arg[0] == '\0') {
		victim = ch->fighting;
		if (victim == NULL) {
			char_puts("But you aren't fighting anyone!\n",ch);
			return;
 		}
	}
	else 
		victim = get_char_room(ch, arg);

	if (!victim || victim->in_room != ch->in_room) {
		WAIT_STATE(ch, MISSING_TARGET_DELAY);
		char_puts("They aren't here.\n",ch);
		return;
	}

	if (MOUNTED(victim)) {
		char_puts("You can't trip a riding one!\n", ch);
		return;
	}

	if (IS_AFFECTED(victim, AFF_FLYING)) {
		act("$S feet aren't on the ground.", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (victim->position < POS_FIGHTING) {
		act("$N is already down.", ch, NULL, victim, TO_CHAR);
		return;
	}

	beats = skill_beats("trip");
	if (victim == ch) {
		act_puts("You fall flat on your face!",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		WAIT_STATE(ch, beats);
		act("$n trips over $s own feet!", ch, NULL, NULL, TO_ROOM);
		return;
	}

	if (IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim) {
		act("$N is your beloved master.", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (is_safe(ch, victim))
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
	if ((IS_NPC(ch) && IS_SET(ch->pMobIndex->off_flags, OFF_FAST))
	||  IS_AFFECTED(ch, AFF_HASTE))
		chance += 10;
	if ((IS_NPC(victim) && IS_SET(victim->pMobIndex->off_flags, OFF_FAST))
	||  IS_AFFECTED(victim, AFF_HASTE))
		chance -= 20;

	/* level */
	chance += (LEVEL(ch) - LEVEL(victim)) * 2;

	RESET_WAIT_STATE(ch);
	attack = (ch->fighting != victim);

	/* now the attack */
	if (number_percent() < chance) {
		act("$n trips you and you go down!", ch, NULL, victim, TO_VICT);
		act("You trip $N and $N goes down!",
		    ch, NULL, victim, TO_CHAR);
		act("$n trips $N, sending $M to the ground.",
		    ch, NULL, victim, TO_NOTVICT);
		check_improve(ch, "trip", TRUE, 1);

		WAIT_STATE(victim, beats);
		WAIT_STATE(ch, beats);
		victim->position = POS_RESTING;
		damage(ch, victim, number_range(2, 2 + 2*victim->size),
		       "trip", DAM_BASH, TRUE);
	}
	else {
		damage(ch, victim, 0, "trip", DAM_BASH, TRUE);
		WAIT_STATE(ch, beats * 2 / 3);
		check_improve(ch, "trip", FALSE, 1);
	}

	if (attack)
		yell(victim, ch, "Help! $lu{$i} just tripped me!");
}

void do_backstab(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int foo;
	int chance;
	const char *dt = TYPE_UNDEFINED;

	one_argument(argument, arg, sizeof(arg));

	if (MOUNTED(ch)) {
		char_puts("You can't backstab while riding!\n", ch);
		return;
	}

	if ((chance = get_skill(ch, "backstab")) == 0) {
		char_puts("You don't know how to backstab.\n",ch);
		return;
	}

	if (get_dam_class(ch, get_eq_char(ch, WEAR_WIELD),
			 &dt, &foo) != DAM_PIERCE) {
		char_puts("You need piercing weapon to backstab.\n", ch);
		return;
	}

	if (arg[0] == '\0') {
		char_puts("Backstab whom?\n", ch);
		return;
	}

	WAIT_STATE(ch, skill_beats("backstab"));

	if ((victim = get_char_room(ch, arg)) == NULL) {
		WAIT_STATE(ch, MISSING_TARGET_DELAY);
		char_puts("They aren't here.\n", ch);
		return;
	}

	if (victim == ch) {
		char_puts("How can you sneak up on yourself?\n", ch);
		return;
	}

	if (is_safe(ch, victim))
		return;

	if (victim->fighting) {
		if (ch)
			char_puts("You can't backstab a fighting person.\n",
				  ch);
		return;
	}

	if (victim->hit < 7 * victim->max_hit / 10) {
		if (ch)	act("$N is hurt and suspicious... "
			    "you couldn't sneak up.",
			    ch, NULL, victim, TO_CHAR);
		return;
	}

	if (!IS_AWAKE(victim) ||  number_percent() < chance) {
		check_improve(ch, "backstab", TRUE, 1);
		one_hit(ch, victim, "backstab", WEAR_WIELD);
		if (number_percent() < get_skill(ch, "dual backstab") * 8 / 10) {
			check_improve(ch, "dual backstab", TRUE, 1);
			one_hit(ch, victim, "dual backstab", WEAR_WIELD);
		} else 
			check_improve(ch, "dual backstab", FALSE, 1);
	} else {
		check_improve(ch, "backstab", FALSE, 1);
		damage(ch, victim, 0, "backstab", DAM_NONE, TRUE);
	}

	yell(victim, ch, "Die, $i! You are backstabbing scum!");
}

void do_knife(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA *knife;
	int chance;

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		char_puts("Knife whom?\n", ch);
		return;
	}
	
	if ((chance = get_skill(ch, "knife")) == 0) {
		act("You don't know how to knife.", ch, NULL, NULL, TO_CHAR);
		return;
	}

	if ((knife = get_eq_char(ch, WEAR_WIELD)) == NULL) {
		act("You need a weapon.", ch, NULL, NULL, TO_CHAR);
		return;
	}

	if (!WEAPON_IS(knife, WEAPON_DAGGER)) {
		act("Your weapon must be dagger.", ch, NULL, NULL, TO_CHAR);
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL) {
		char_puts("They aren't here.\n", ch);
		WAIT_STATE(ch, MISSING_TARGET_DELAY);
		return;
	}

	if (victim->fighting != NULL) {
		act("$N is fighting.", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (is_safe(ch, victim))
		return;

	if (ch == victim) {
		act("Suicide is a mortal sin.", ch, NULL, NULL, TO_CHAR);
		return;
	}

	WAIT_STATE(ch, skill_beats("knife"));

	if (number_percent()<chance) {
		one_hit(ch, victim, "knife", WEAR_WIELD);
		check_improve(ch, "knife", TRUE, 1);
	}
	else {
		damage(ch, victim, 0, "knife",  DAM_NONE, TRUE);
		check_improve(ch, "knife", FALSE, 1);
	}
	yell(victim, ch, "Die, $i! You're backstabbing scum!");
}

void do_cleave(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	int chance;

	if (MOUNTED(ch)) {
		char_puts("You can't cleave while riding!\n", ch);
		return;
	}

	one_argument(argument, arg, sizeof(arg));

	if (ch->master != NULL && IS_NPC(ch))
		return;

	if ((chance = get_skill(ch, "cleave")) == 0) {
		char_puts("You don't know how to cleave.\n",ch);
		return;
	}

	if (arg[0] == '\0') {
		char_puts("Cleave whom?\n", ch);
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL) {
		WAIT_STATE(ch, MISSING_TARGET_DELAY);
		char_puts("They aren't here.\n", ch);
		return;
	}

	if (victim == ch) {
		char_puts("How can you sneak up on yourself?\n", ch);
		return;
	}

	if ((obj = get_eq_char(ch, WEAR_WIELD)) == NULL) {
		char_puts("You need to wield a weapon to cleave.\n", ch);
		return;
	}

	if (!WEAPON_IS(obj, WEAPON_AXE)
	&&  !WEAPON_IS(obj, WEAPON_SWORD)) {
		char_puts("You must wield axe or sword to cleave.\n", ch);
		return;
	}

	if (victim->fighting != NULL) {
		char_puts("You can't cleave a fighting person.\n", ch);
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

	WAIT_STATE(ch, skill_beats("cleave"));

	if (!IS_AWAKE(victim)
	||  IS_NPC(ch)
	||  number_percent() < chance) {
		check_improve(ch, "cleave", TRUE, 1);
		one_hit(ch, victim, "cleave", WEAR_WIELD);
	}
	else {
		check_improve(ch, "cleave", FALSE, 1);
		damage(ch, victim, 0, "cleave", DAM_NONE, TRUE);
	}
	yell(victim, ch, "Die, $i, you butchering fool!");
}

void do_ambush(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int chance;

	if (MOUNTED(ch)) {
		char_puts("You can't ambush while riding!\n", ch);
		return;
	}

	one_argument(argument, arg, sizeof(arg));

	if ((chance = get_skill(ch, "ambush")) == 0) {
		char_puts("You don't know how to ambush.\n", ch);
		return;
	}

	if (arg[0] == '\0') {
		char_puts("Ambush whom?\n", ch);
		return;
	}
	
	WAIT_STATE(ch, skill_beats("ambush"));

	if ((victim = get_char_room(ch, arg)) == NULL) {
		WAIT_STATE(ch, MISSING_TARGET_DELAY);
		char_puts("They aren't here.\n", ch);
		return;
	}

	if (victim == ch) {
		char_puts("How can you ambush yourself?\n", ch);
		return;
	}

	if (!IS_AFFECTED(ch, AFF_CAMOUFLAGE) || can_see(victim, ch)) {
		char_puts("But they can see you.\n", ch);
		return;
	}

	if (is_safe(ch, victim))
		return;

	if (!IS_AWAKE(victim)
	||  IS_NPC(ch)
	||  number_percent() < chance) {
		check_improve(ch, "ambush", TRUE, 1);
		one_hit(ch, victim, "ambush", WEAR_WIELD);
	}
	else {
		check_improve(ch, "ambush", FALSE, 1);
		damage(ch, victim, 0, "ambush", DAM_NONE, TRUE);
	}
	yell(victim, ch, "Help! I've been ambushed by $i!");
}

void do_rescue(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	CHAR_DATA *fch;

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		char_puts("Rescue whom?\n", ch);
		return;
	}

	WAIT_STATE(ch, skill_beats("rescue"));

	if ((victim = get_char_room(ch, arg)) == NULL) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	if (victim == ch) {
		char_puts("What about fleeing instead?\n", ch);
		return;
	}

	if (!IS_NPC(ch) && IS_NPC(victim)) {
		char_puts("Doesn't need your help!\n", ch);
		return;
	}

	if (ch->fighting == victim) {
		char_puts("Too late.\n", ch);
		return;
	}

	if ((fch = victim->fighting) == NULL) {
		char_puts("That person is not fighting right now.\n", ch);
		return;
	}

	if (IS_NPC(ch) && ch->master != NULL && IS_NPC(victim))
		return;

	if (ch->master != NULL && is_safe(ch->master, fch))
		return;

	if (is_safe(ch, fch))
		return;

	if (number_percent() > get_skill(ch, "rescue")) {
		char_puts("You fail the rescue.\n", ch);
		check_improve(ch, "rescue", FALSE, 1);
		return;
	}

	act("You rescue $N!",  ch, NULL, victim, TO_CHAR   );
	act("$n rescues you!", ch, NULL, victim, TO_VICT   );
	act("$n rescues $N!",  ch, NULL, victim, TO_NOTVICT);
	check_improve(ch, "rescue", TRUE, 1);

	stop_fighting(ch, FALSE);
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
		char_puts("You can't kick while riding!\n", ch);
		return;
	}

	if ((chance = get_skill(ch, "kick")) == 0) {
		char_puts("You better leave the martial arts to fighters.\n",
			  ch);
		return;
	}

	if ((victim = ch->fighting) == NULL) {
		char_puts("You aren't fighting anyone.\n", ch);
		return;
	}

	if (IS_AFFECTED(ch, AFF_FLYING))
		chance = chance * 110 / 100;

	WAIT_STATE(ch, skill_beats("kick"));
	if (IS_NPC(ch) || number_percent() < chance) {
		kick_dam = number_range(1, LEVEL(ch));
		if (HAS_SKILL(ch, "katana")
		&&  (get_eq_char(ch, WEAR_FEET) == NULL)) 
			kick_dam *= 2;
		kick_dam += ch->damroll / 2;
		damage(ch, victim, kick_dam, "kick", DAM_BASH, TRUE);
		check_improve(ch, "kick", TRUE, 1);
                if((chance = get_skill(ch, "follow through")) != 0){
			chance += get_curr_stat(ch, STAT_DEX); 
			while( number_percent() <= chance){
				if (IS_EXTRACTED(victim)) break;
                                act_puts("You follow through kick strikes $N.", ch,  NULL, victim, TO_CHAR, POS_FIGHTING);
                                act_puts("$n's follow through kick strikes you.", ch,  NULL, victim, TO_VICT, POS_FIGHTING);
                                act_puts("$n's follow through kick strikes $N.", ch,  NULL, victim, TO_NOTVICT, POS_FIGHTING);

                                kick_dam /= 2;
				kick_dam += number_range(1, LEVEL(ch))/2;
                                damage(ch, victim, kick_dam, "kick", DAM_BASH, TRUE);
                                chance /= 5;
                             }
			check_improve(ch, "follow through", TRUE, 6);
		}
	} else {
		damage(ch, victim, 0, "kick", DAM_BASH, TRUE);
		check_improve(ch, "kick", FALSE, 1);
	}
}

void do_circle(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *victim;
	CHAR_DATA *rch;
	int chance;
	OBJ_DATA *obj;

	if (MOUNTED(ch)) {
		char_puts("You can't circle while riding!\n", ch);
		return;
	}

	if ((chance = get_skill(ch, "circle")) == 0) {
		char_puts("You don't know how to circle.\n", ch);
		return;
	}

	if ((victim = ch->fighting) == NULL) {
		char_puts("You aren't fighting anyone.\n", ch);
		return;
	}

	if ((obj = get_eq_char(ch, WEAR_WIELD)) == NULL
	||  damtype_class(obj->value[3].s) != DAM_PIERCE) {
		 char_puts("You must wield a piercing weapon to circle stab.\n",
			   ch);
		 return;
	}

	if (is_safe(ch, victim))
		return;

	WAIT_STATE(ch, skill_beats("circle"));

	for (rch = ch->in_room->people; rch; rch = rch->next_in_room) {
		if (rch->fighting == ch) {
			char_puts("You can't circle while defending yourself.\n",
				  ch);
			return;
		}
	}

	if (number_percent() < chance) {
		one_hit(ch, victim, "circle", WEAR_WIELD);
		check_improve(ch, "circle", TRUE, 1);
	} else {
		damage(ch, victim, 0, "circle", DAM_NONE, TRUE);
		check_improve(ch, "circle", FALSE, 1);
	}
}

void do_disarm(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *victim;
	OBJ_DATA *wield;
	OBJ_DATA *vwield;
	int chance, ch_weapon, vict_weapon;
	int loc = WEAR_WIELD;
	char arg[MAX_INPUT_LENGTH];
	int hth = 0;

	if (ch->master != NULL && IS_NPC(ch))
		return;

	if ((chance = get_skill(ch, "disarm")) == 0) {
		char_puts("You don't know how to disarm opponents.\n", ch);
		return;
	}

	if ((wield = get_eq_char(ch, WEAR_WIELD)) == NULL 
	&&  (hth = get_skill(ch, "hand to hand")) == 0) {
		char_puts("You must wield a weapon to disarm.\n", ch);
		return;
	}

	if ((victim = ch->fighting) == NULL) {
		char_puts("You aren't fighting anyone.\n", ch);
		return;
	}

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] && !str_prefix(arg, "second"))
		loc = WEAR_SECOND_WIELD;

	if ((vwield = get_eq_char(victim, loc)) == NULL) {
		char_puts("Your opponent is not wielding a weapon.\n", ch);
		return;
	}

	/* find weapon skills */
	ch_weapon = get_weapon_skill(ch, get_weapon_sn(wield));
	vict_weapon = get_weapon_skill(victim, get_weapon_sn(vwield));

	/* modifiers */

	/* skill */
	if (wield == NULL)
		chance = chance * hth/150;
	else
		chance = chance * ch_weapon/100;

	chance += (ch_weapon/2 - vict_weapon) / 2; 

	if (wield && WEAPON_IS(wield, WEAPON_STAFF))
		chance += 30;
	if (WEAPON_IS(vwield, WEAPON_STAFF))
		chance -= 30;

	/* dex vs. strength */
	chance += get_curr_stat(ch,STAT_DEX);
	chance -= 2 * get_curr_stat(victim,STAT_STR);

	/* level */
	chance += (LEVEL(ch) - LEVEL(victim)) * 2;
 
	/* and now the attack */
	WAIT_STATE(ch, skill_beats("disarm"));
	if (number_percent() < chance) {
		disarm(ch, victim, vwield);
		check_improve(ch, "disarm", TRUE, 1);
	}
	else {
		act("You fail to disarm $N.", ch, NULL, victim, TO_CHAR);
		act("$n tries to disarm you, but fails.",
		    ch, NULL, victim, TO_VICT);
		act("$n tries to disarm $N, but fails.",
		    ch, NULL, victim, TO_NOTVICT);
		check_improve(ch, "disarm", FALSE, 1);
	}
}

void do_nerve(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *victim;
	char arg[MAX_INPUT_LENGTH];
	bool attack;
	int chance;

	if (MOUNTED(ch)) {
		char_puts("You can't nerve while riding!\n", ch);
		return;
	}

	one_argument(argument, arg, sizeof(arg));

	if ((chance = get_skill(ch, "nerve")) == 0) {
		char_puts("Huh?\n", ch);
		return;
	}
	
	if (arg[0] =='\0') {
		victim = ch->fighting;
		if (victim == NULL) {
			char_puts("You aren't fighting anyone.\n", ch);
			return;
		}
	}
	else if ((victim=get_char_room(ch, arg)) == NULL) {
			char_puts("They aren't here.\n", ch);
			return;
	}


	if (is_affected(victim, "nerve")) {
		char_puts("You cannot weaken that character any more.\n",
			  ch);
		return;
	}

	if (is_safe(ch,victim))
		return;

	WAIT_STATE(ch, skill_beats("nerve"));

	attack = (ch->fighting != victim);


	if (IS_NPC(ch)
	||  number_percent() < (chance + ch->level 
			                 + get_curr_stat(ch,STAT_DEX))/2) {
		AFFECT_DATA af;
		af.where	= TO_AFFECTS;
		af.type 	= "nerve";
		af.level 	= ch->level;
		af.duration	= LEVEL(ch) * PULSE_VIOLENCE/PULSE_TICK;
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
		check_improve(ch, "nerve", TRUE, 1);
	}
	else {
		char_puts("You press the wrong points and fail.\n",ch);
		act("$n tries to weaken you with nerve pressure, but fails.",
		    ch, NULL, victim, TO_VICT);
		act("$n tries to weaken $N with nerve pressure, but fails.",
		    ch, NULL, victim, TO_NOTVICT);
		check_improve(ch, "nerve", FALSE, 1);
	}
	if (attack)
		yell(victim, ch, "Help! $lu{$i} is attacking me!");
	multi_hit(victim,ch,TYPE_UNDEFINED);
}

void do_endure(CHAR_DATA *ch, const char *argument)
{
	AFFECT_DATA af;
	int chance;

	if (IS_NPC(ch)) {
		char_puts("You have no endurance whatsoever.\n", ch);
		return;
	}

	if ((chance = get_skill(ch, "endure")) == 0) {
		char_puts("You lack the concentration.\n", ch);
		return;
	}
		 
	if (is_affected(ch, "endure")) {
		char_puts("You cannot endure more concentration.\n", ch);
		return;
	}
	
	WAIT_STATE(ch, skill_beats("endure"));

	af.where 	= TO_AFFECTS;
	af.type 	= "endure";
	af.level 	= ch->level;
	af.duration	= ch->level / 4;
	af.location	= APPLY_SAVING_SPELL;
	af.modifier	= - chance / 10; 
	af.bitvector	= 0;

	affect_to_char(ch, &af);

	char_puts("You prepare yourself for magical encounters.\n", ch);
	act("$n concentrates for a moment, then resumes $s position.",
	    ch, NULL, NULL, TO_ROOM);
	check_improve(ch, "endure", TRUE, 1);
}
 
void do_tame(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *victim;
	char arg[MAX_INPUT_LENGTH];
	int chance;

	if ((chance = get_skill(ch, "tame")) == 0) {
		char_puts("Huh?\n", ch);
		return;
	}

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		char_puts("You are beyond taming.\n", ch);
		act("$n tries to tame $mself but fails miserably.",
		    ch, NULL, NULL, TO_ROOM);
		return;
	}

	WAIT_STATE(ch, skill_beats("tame"));
	
	if ((victim = get_char_room(ch,arg)) == NULL) {
		char_puts("They're not here.\n", ch);
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

	if (!IS_SET(victim->pMobIndex->act, ACT_AGGRESSIVE)) {
		act("$N is not usually aggressive.", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (number_percent() < chance + 15 + 4*(LEVEL(ch) - LEVEL(victim))) {
		SET_BIT(victim->affected_by, AFF_CALM);
		char_puts("You calm down.\n", victim);
		act("You calm $N down.", ch, NULL, victim, TO_CHAR);
		act("$n calms $N down.", ch, NULL, victim, TO_NOTVICT);
		stop_fighting(victim, TRUE);
		check_improve(ch, "tame", TRUE, 1);
	}
	else {
		char_puts("You failed.\n",ch);
		act("$n tries to calm down $N but fails.",
		    ch, NULL, victim, TO_NOTVICT);
		act("$n tries to calm you down but fails.",
		    ch, NULL, victim, TO_VICT);
		check_improve(ch, "tame", FALSE, 1);
	}
}

void do_assassinate(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int chance;

	if (MOUNTED(ch)) {
		char_puts("You can't assassinate while riding!\n", ch);
		return;
	}

	one_argument(argument, arg, sizeof(arg));

	if (ch->master != NULL && IS_NPC(ch))
		return;

	if ((chance = get_skill(ch, "assassinate")) == 0) {
		char_puts("You don't know how to assassinate.\n", ch);
		return;
	}
	
	if (IS_AFFECTED(ch, AFF_CHARM))  {
		char_puts("You don't want to kill your beloved master.\n", ch);
		return;
	} 

	if (arg[0] == '\0') {
		char_puts("Assassinate whom?\n", ch);
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL) {
		WAIT_STATE(ch, MISSING_TARGET_DELAY);
		char_puts("They aren't here.\n", ch);
		return;
	}

	if (victim == ch) {
		char_puts("Suicide is against your way.\n", ch);
		return;
	}

	if (IS_IMMORTAL(victim) && !IS_NPC(victim)) {
		char_puts("Your hands pass through.\n", ch);
		return;
	}

	if (victim->fighting != NULL) {
		char_puts("You can't assassinate a fighting person.\n", ch);
		return;
	}
 
	if ((get_eq_char(ch, WEAR_WIELD) != NULL)
	||  (get_eq_char(ch, WEAR_HOLD ) != NULL))  {
		char_puts("You need both hands free to assassinate somebody.\n",
			  ch);
		return;
	}
 
	if (victim->hit < victim->max_hit*0.9) {
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

	WAIT_STATE(ch, skill_beats("assassinate"));

	if (number_percent() < chance
	&&  !IS_CLAN_GUARD(victim)
	&&  !IS_IMMORTAL(victim))
		one_hit(ch, victim, "assassinate", WEAR_WIELD);
	else {
		check_improve(ch, "assassinate", FALSE, 1);
		damage(ch, victim, 0, "assassinate", DAM_NONE, TRUE);
	}
	yell(victim, ch, "Help! $lu{$i} tries to assassinate me!");
}

void do_caltrops(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *victim = ch->fighting;
	int chance;

	if ((chance = get_skill(ch, "caltrops")) == 0) {
		char_puts("Caltrops? Is that a dance step?\n", ch);
		return;
	}

	if (victim == NULL) {
		char_puts("You must be in combat.\n",ch);
		return;
	}

	if (is_safe(ch,victim))
		return;
		
	act("You throw a handful of sharp spikes at the feet of $N.",
	    ch, NULL, victim, TO_CHAR);
	act("$n throws a handful of sharp spikes at your feet!",
	    ch, NULL, victim, TO_VICT);

	WAIT_STATE(ch, skill_beats("caltrops"));

	if (!IS_NPC(ch) && number_percent() >= chance) {
		damage(ch, victim, 0, "caltrops", DAM_PIERCE, TRUE);
		check_improve(ch, "caltrops", FALSE, 1);
		return;
	}

	damage(ch, victim, LEVEL(ch), "caltrops", DAM_PIERCE, TRUE);
	if (IS_EXTRACTED(victim))
		return;

	if (!is_affected(victim, "caltrops")) {
		AFFECT_DATA tohit, todam, todex;

		tohit.where     = TO_AFFECTS;
		tohit.type      = "caltrops";
		tohit.level     = ch->level;
		tohit.duration  = -1; 
		tohit.location  = APPLY_HITROLL;
		tohit.modifier  = -5;
		tohit.bitvector = 0;
		affect_to_char(victim, &tohit);

		todam.where = TO_AFFECTS;
		todam.type = "caltrops";
		todam.level = ch->level;
		todam.duration = -1;
		todam.location = APPLY_DAMROLL;
		todam.modifier = -5;
		todam.bitvector = 0;
		affect_to_char(victim, &todam);

		todex.type = "caltrops";
		todex.level = ch->level;
		todex.duration = -1;
		todex.location = APPLY_DEX;
		todex.modifier = -5;
		todex.bitvector = 0;
		affect_to_char(victim, &todex);

		act("$N starts limping.", ch, NULL, victim, TO_CHAR);
		act("You start to limp.", ch, NULL, victim, TO_VICT);
		check_improve(ch, "caltrops", TRUE, 1);
	}
}

DECLARE_DO_FUN(do_throw_weapon);

void do_throw(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *victim;
	char arg[MAX_INPUT_LENGTH];
	int chance;

	argument = one_argument(argument, arg, sizeof(arg));

	if (!str_cmp(arg, "weapon")) {
		do_throw_weapon(ch, argument);
		return;
	}

	if (MOUNTED(ch)) {
		char_puts("You can't throw while riding!\n", ch);
		return;
	}

	if ((chance = get_skill(ch, "throw")) == 0) {
		char_puts("A clutz like you couldn't throw down a worm.\n",
			  ch);
		return;
	}

	if (IS_AFFECTED(ch, AFF_FLYING)) {
		char_puts("Your feet should touch the ground to balance\n",
			     ch);
		return;
	}

	if ((victim = ch->fighting) == NULL) {
		char_puts("You aren't fighting anyone.\n", ch);
		return;
	}

	if (victim->in_room != ch->in_room) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim) {
		act("But $N is your friend!", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (is_safe(ch,victim))
		return;

	WAIT_STATE(ch, skill_beats("throw"));

	if (is_affected(victim, "protective shield")) {
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
	if (IS_NPC(ch) && IS_SET(ch->pMobIndex->off_flags, OFF_FAST))
		chance += 10;
	if (IS_NPC(victim) && IS_SET(victim->pMobIndex->off_flags, OFF_FAST))
		chance -= 20;

	/* level */
	chance += (LEVEL(ch) - LEVEL(victim)) * 2;

	if (number_percent() < chance) {
		act("You throw $N to the ground with stunning force.",
		    ch, NULL, victim, TO_CHAR);
		act("$n throws you to the ground with stunning force.",
		    ch, NULL, victim, TO_VICT);
		act("$n throws $N to the ground with stunning force.",
		    ch, NULL, victim, TO_NOTVICT);
		WAIT_STATE(victim,2 * PULSE_VIOLENCE);

		damage(ch, victim, LEVEL(ch) + get_curr_stat(ch, STAT_STR), 
		       "throw", DAM_BASH, TRUE);
		check_improve(ch, "throw", TRUE, 1);
	}
	else {
		act("You fail to grab your opponent.",
		    ch, NULL, NULL, TO_CHAR);
		act("$N tries to throw you, but fails.",
		    victim, NULL, ch, TO_CHAR);
		act("$n tries to grab $N's arm.",
		    ch, NULL, victim, TO_NOTVICT);
		check_improve(ch, "throw", FALSE, 1);
	}
}

void do_strangle(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *victim;
	AFFECT_DATA af;
	int chance;

	if (MOUNTED(ch)) {
		char_puts("You can't strangle while riding!\n", ch);
		return;
	}

	if ((chance = get_skill(ch, "strangle")) == 0) {
		char_puts("You lack the skill to strangle.\n", ch);
		return;
	}


	if (IS_AFFECTED(ch, AFF_CHARM))  {
		char_puts("You don't want to grap your beloved masters' neck.\n",
			  ch);
		return;
	} 


	if ((victim = get_char_room(ch,argument)) == NULL) {
		WAIT_STATE(ch, MISSING_TARGET_DELAY);
		char_puts("You do not see that person here.\n", ch);
		return;
	}

	if (ch == victim) {
		char_puts("Even you are not that stupid.\n", ch);
		return;
	}

	if (is_affected(victim, "strangle"))
		return;

	if (MOUNTED(victim)) {
		char_puts("You can't strangle a riding one.\n", ch);
		return;
	}

	if (is_affected(victim, "free action"))
		chance -= 15;
	if (is_safe(ch, victim))
		return;

	SET_FIGHT_TIME(victim);
	SET_FIGHT_TIME(ch);
	
	WAIT_STATE(ch, skill_beats("strangle"));

	if (number_percent() < chance * 6 /10
	&&  !IS_CLAN_GUARD(victim)
	&&  !IS_IMMORTAL(victim)) {
		act("You grab hold of $N's neck and put $M to sleep.",
		    ch, NULL, victim, TO_CHAR);
		act("$n grabs hold of your neck and puts you to sleep.",
		    ch, NULL, victim, TO_VICT);
		act("$n grabs hold of $N's neck and puts $M to sleep.",
		    ch, NULL, victim, TO_NOTVICT);
		check_improve(ch, "strangle", TRUE, 1);
		
		af.type = "strangle";
		af.where = TO_AFFECTS;
		af.level = ch->level;
		af.duration = LEVEL(ch) / 20 + 1;
		af.location = APPLY_NONE;
		af.modifier = 0;
		af.bitvector = AFF_SLEEP;
		affect_join (victim,&af);

		if (IS_AWAKE(victim))
			victim->position = POS_SLEEPING;
		if (RIDDEN(victim)) 
			do_dismount(RIDDEN(victim), str_empty);
	}
	else {
		damage(ch,victim, 0, "strangle", DAM_NONE, TRUE);
		check_improve(ch, "strangle", FALSE, 1);
		yell(victim, ch, "Help! I'm being strangled by $i!");
	}
}

void do_blackjack(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *victim;
	AFFECT_DATA af;
	int chance;

	if (MOUNTED(ch)) {
		char_puts("You can't blackjack while riding!\n", ch);
		return;
	}

	if ((chance = get_skill(ch, "blackjack")) == 0) {
		char_puts("Huh?\n", ch);
		return;
	}

	if ((victim = get_char_room(ch,argument)) == NULL) {
		WAIT_STATE(ch, MISSING_TARGET_DELAY);
		char_puts("You do not see that person here.\n", ch);
		return;
	}

	if (ch == victim) {
		char_puts("You idiot?! Blackjack yourself?!\n", ch);
		return;
	}

	if (MOUNTED(victim)) {
		act("You can't blackjack a riding person.\n", 
			ch, NULL, NULL, TO_CHAR);
		return;
	}

	if (IS_AFFECTED(ch, AFF_CHARM))  {
		char_puts("You don't want to hit your beloved masters' head with a full filled jack.\n",
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
	if (IS_NPC(victim) && victim->pMobIndex->pShop != NULL)
		chance -= 40;
	if (is_affected(victim, "free action"))
		chance -= 15;

	WAIT_STATE(ch, skill_beats("blackjack"));
 
	if (number_percent() < chance
	&&  !IS_CLAN_GUARD(victim)
	&&  !IS_IMMORTAL(victim)) {
		act("You hit $N's head with a lead filled sack.",
		    ch, NULL, victim, TO_CHAR);
		act("You feel a sudden pain erupts through your skull!",
		    ch, NULL, victim, TO_VICT);
		act("$n whacks $N at the back of $S head with a heavy looking sack!  *OUCH*",
		    ch, NULL, victim, TO_NOTVICT);
		check_improve(ch, "blackjack", TRUE, 1);
		
		af.type		= "blackjack";
		af.where	= TO_AFFECTS;
		af.level	= ch->level;
		af.duration	= LEVEL(ch) / 15 + 1;
		af.location	= APPLY_NONE;
		af.modifier	= 0;
		af.bitvector	= AFF_SLEEP;
		affect_join (victim,&af);

		if (IS_AWAKE(victim))
			victim->position = POS_SLEEPING;
		if (RIDDEN(victim))
			do_dismount(RIDDEN(victim), str_empty);
	}
	else {
		damage(ch, victim, LEVEL(ch)/2, "blackjack", DAM_NONE, TRUE);
		check_improve(ch, "blackjack", FALSE, 1);
		yell(victim, ch, "Help! I'm being blackjacked by $i!");
	}
}

void do_bloodthirst(CHAR_DATA *ch, const char *argument)
{
	int chance, hp_percent;

	if ((chance = get_skill(ch, "bloodthirst")) == 0) {
		char_puts("You're not that thirsty.\n", ch);
		return;
	}

	if (IS_AFFECTED(ch, AFF_BLOODTHIRST)) {
		char_puts("Your thirst for blood continues.\n", ch);
		return;
	}

	if (IS_AFFECTED(ch, AFF_CALM)) {
		char_puts("You're feeling to mellow to be bloodthirsty.\n",
			  ch);
		return;
	}

	if (ch->fighting == NULL) {
		char_puts("You need to be fighting.\n", ch);
		return;
	}

	/* modifiers */

	hp_percent = 100 * ch->hit/ch->max_hit;
	chance += 25 - hp_percent/2;

	if (number_percent() < chance) {
		AFFECT_DATA af;

		WAIT_STATE(ch, PULSE_VIOLENCE);
	
		char_puts("You hunger for blood!\n", ch);
		act("$n gets a bloodthirsty look in $s eyes.",
		    ch, NULL, NULL, TO_ROOM);
		check_improve(ch, "bloodthirst", TRUE, 2);

		af.where	= TO_AFFECTS;
		af.type		= "bloodthirst";
		af.level	= ch->level;
		af.duration	= 2 + LEVEL(ch) / 18;
		af.modifier	= 5 + LEVEL(ch) / 4;
		af.bitvector 	= AFF_BLOODTHIRST;

		af.location	= APPLY_HITROLL;
		affect_to_char(ch, &af);

		af.location	= APPLY_DAMROLL;
		affect_to_char(ch, &af);

		af.modifier	= - UMIN(LEVEL(ch) - 5, 35);
		af.location	= APPLY_AC;
		affect_to_char(ch, &af);
	}
	else {
		WAIT_STATE(ch,3 * PULSE_VIOLENCE);
		char_puts("You feel bloodthirsty for a moment, but it passes.\n",
			  ch);
		check_improve(ch, "bloodthirst", FALSE, 2);
	}
}

void do_resistance(CHAR_DATA *ch, const char *argument)
{
	int chance;
	int mana;

	if ((chance = get_skill(ch, "resistance")) == 0) {
		char_puts("Huh?\n", ch);
		return;
	}

	if (is_affected(ch, "resistance")) {
		char_puts("You are as resistant as you will get.\n", ch);
		return;
	}

	mana = skill_mana(ch, "resistance");
	if (ch->mana < mana) {
		char_puts("You cannot muster up the energy.\n", ch);
		return;
	}
	WAIT_STATE(ch, skill_beats("resistance"));

	if (number_percent() < chance) {
		AFFECT_DATA af;
		
		af.where	= TO_AFFECTS;
		af.type 	= "resistance";
		af.level 	= ch->level;
		af.duration	= LEVEL(ch) / 6;
		af.location	= APPLY_SAVES;
		af.modifier	= -LEVEL(ch)/4;
		af.bitvector	= 0;

		affect_to_char(ch, &af);
		ch->mana -= mana;

		act("You feel tough!", ch, NULL, NULL, TO_CHAR);
		act("$n looks tougher.", ch, NULL, NULL, TO_ROOM);
		check_improve(ch, "resistance", TRUE, 1);
	} else {
		ch->mana -= mana/2;

		char_puts("You flex your muscles, "
			  "but you don't feel tougher.\n", ch);
		act("$n flexes $s muscles, trying to look tough.",
		    ch, NULL, NULL, TO_ROOM);
		check_improve(ch, "resistance", FALSE, 1);
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
	
	if ((chance = get_skill(ch, "trophy")) == 0) {
		char_puts("Huh?\n", ch);
		return;
	}
	
	if (is_affected(ch, "trophy")) {
		char_puts("But you've already got one trophy!\n", ch);
		return;
	}
	
	mana = skill_mana(ch, "trophy");
	if (ch->mana < mana) {
		char_puts("You feel too weak to concentrate on a trophy.\n",
			  ch);
		return;
	}
	
	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		char_puts("Make a trophy of what?\n", ch);
		return;
	}
	
	if ((part = get_obj_carry(ch, arg)) == NULL) {
		char_puts("You do not have that body part.\n", ch);
		return;
	}

	if (number_percent() < chance * 2 / 3) {
		char_puts("You failed and destroyed it.\n", ch);
		extract_obj(part, 0);
		return;
	} 

	WAIT_STATE(ch, skill_beats("trophy"));

	if (part->pObjIndex->vnum == OBJ_VNUM_SLICED_ARM
	||  part->pObjIndex->vnum == OBJ_VNUM_SLICED_LEG
	||  part->pObjIndex->vnum == OBJ_VNUM_SEVERED_HEAD
	||  part->pObjIndex->vnum == OBJ_VNUM_TORN_HEART
	||  part->pObjIndex->vnum == OBJ_VNUM_GUTS)
		trophy_vnum = OBJ_VNUM_BATTLE_PONCHO;
	else if (part->pObjIndex->vnum == OBJ_VNUM_BRAINS) {
		char_puts("Why don't you just eat those instead?\n", ch);
		return;
	}
	else {
		char_puts("You can't make a trophy out of that!\n", ch);
		return;
	}

	if (mlstr_null(&part->owner)) {
		char_puts("Invalid body part.\n", ch);
		return;
	}

	if (!IS_NPC(ch) && number_percent() < chance) {
		af.where	= TO_AFFECTS;
		af.type		= "trophy";
		af.level	= ch->level;
		af.duration	= ch->level/2;
		af.modifier	= 0;
		af.bitvector 	= 0;
		
		af.location	= 0;
		affect_to_char(ch,&af);
	
		if (trophy_vnum != 0) {
			level = UMIN(part->level + 5, MAX_LEVEL);
 
			trophy = create_obj_of(get_obj_index(trophy_vnum),
					       &part->owner);
			trophy->level	= ch->level;
			trophy->timer	= ch->level * 2;
			trophy->cost	= 0;
			ch->mana	-= mana;
			af.where	= TO_OBJECT;
			af.type 	= "trophy";
			af.level	= level;
			af.duration	= -1;
			af.location	= APPLY_DAMROLL;
			af.modifier  = LEVEL(ch) / 5;
			af.bitvector	= 0;
			affect_to_obj(trophy, &af);

			af.location	= APPLY_HITROLL;
			af.modifier  = LEVEL(ch) / 5;
			af.bitvector	= 0;
			affect_to_obj(trophy, &af);

			af.location	= APPLY_INT;
			af.modifier	= level>20?-2:-1;
			affect_to_obj(trophy, &af);

			af.location	= APPLY_STR;
			af.modifier	= level>20?2:1;
			affect_to_obj(trophy, &af);

			INT_VAL(trophy->value[0]) = LEVEL(ch);
			INT_VAL(trophy->value[1]) = LEVEL(ch);
			INT_VAL(trophy->value[2]) = LEVEL(ch);
			INT_VAL(trophy->value[3]) = LEVEL(ch);
			
			obj_to_char(trophy, ch);
			  check_improve(ch, "trophy", TRUE, 1);
			
			act("You make a poncho from $p!",
			    ch, part, NULL, TO_CHAR);
			act("$n makes a poncho from $p!",
			    ch, part, NULL, TO_ROOM);
			
			extract_obj(part, 0);
			return;
		}
	}
	else {
		char_puts("You destroyed it.\n", ch);
		extract_obj(part, 0);
		ch->mana -= mana/2;
		check_improve(ch, "trophy", FALSE, 1);
	}
}
	
void do_truesight(CHAR_DATA *ch, const char *argument)
{
	int chance;

	if ((chance = get_skill(ch, "truesight")) == 0) {
		char_puts("Huh?\n", ch);
		return;
	}

	if (is_affected(ch, "truesight")) {
		char_puts("Your eyes are as sharp as they can get.\n", ch);
		return;
	}

	WAIT_STATE(ch, skill_beats("truesight"));

	if (number_percent() < chance) {
		AFFECT_DATA af;
		
		af.where    = TO_AFFECTS;
		af.type     = "truesight";
		af.level    = ch->level;
		af.duration = LEVEL(ch)/2 + 5;
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

		act("You look around sharply!", ch, NULL, NULL, TO_CHAR);
		act("$n looks more enlightened.", ch, NULL, NULL, TO_ROOM);
		check_improve(ch, "truesight", TRUE, 1);
	}
	else {
		char_puts("You look about sharply, but you don't see "
			     "anything new.\n" ,ch);
		act("$n looks around sharply but doesn't seem enlightened.",
			ch,NULL,NULL,TO_ROOM);
		check_improve(ch, "truesight", FALSE, 1);
	}
}

void do_warcry(CHAR_DATA *ch, const char *argument)
{
	AFFECT_DATA af;
	int chance;
	int mana;
	
	if ((chance = get_skill(ch, "warcry")) == 0) {
		char_puts("Huh?\n", ch);
		return;
	}
	
	if (is_affected(ch, "bless") || is_affected(ch, "warcry")) {
		char_puts("You are already blessed.\n",ch);
		return;
	}
	
	mana = skill_mana(ch, "warcry");
	if (ch->mana < mana) {
		char_puts("You can't concentrate enough right now.\n",ch);
		return;
	}
	WAIT_STATE(ch, skill_beats("warcry"));
	
	if (number_percent() > chance) {
		char_puts("You grunt softly.\n", ch);
		act("$n makes some soft grunting noises.",
		    ch, NULL, NULL, TO_ROOM);
		ch->mana -= mana/2;
		check_improve(ch, "warcry", FALSE, 1);
		return;
	}
	
	ch->mana -= mana;
	check_improve(ch, "warcry", TRUE, 1);
 
	af.where	= TO_AFFECTS;
	af.type		= "warcry";
	af.level	= ch->level;
	af.duration	= 6 + ch->level;
	af.location	= APPLY_HITROLL;
	af.modifier	= LEVEL(ch) / 8;
	af.bitvector	= 0;
	affect_to_char(ch, &af);
	
	af.location	= APPLY_SAVING_SPELL;
	af.modifier	= 0 - LEVEL(ch) / 8;
	affect_to_char(ch, &af);
	char_puts("You feel righteous as you yell out your warcry.\n", ch);
}

void do_guard(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	CHAR_DATA *guarding;
	int chance;
	
	if ((chance = get_skill(ch, "guard")) == 0) {
		char_puts("Huh?\n", ch);
		return;
	}

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		char_puts("Guard whom?\n", ch);
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	if (IS_NPC(victim)) {
		act("$N doesn't need any of your help!",
		    ch, NULL, victim, TO_CHAR);
		return;
	}

	guarding = PC(ch)->guarding;

	if (!str_cmp(arg, "none") || !str_cmp(arg, "self") || victim == ch) {
		if (guarding == NULL) {
			char_puts("You can't guard yourself!\n", ch);
			return;
		}
		act("You stop guarding $N.", ch, NULL, guarding, TO_CHAR);
		act("$n stops guarding you.", ch, NULL, guarding, TO_VICT);
		act("$n stops guarding $N.", ch, NULL, guarding, TO_NOTVICT);
		PC(guarding)->guarded_by	= NULL;
		PC(ch)->guarding		= NULL;
		return;
	}

	if (guarding == victim) {
		act("You're already guarding $N!", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (guarding != NULL) {
		char_puts("But you're already guarding someone else!\n", ch);
		return;
	}

	if (PC(victim)->guarded_by != NULL) {
		act("$N is already being guarded by someone.",
		    ch, NULL, victim, TO_CHAR);
		return;
	}

	if (PC(victim)->guarding == ch) {
		act("But $N is guarding you!", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (!is_same_group(victim, ch)) {
		act("But you aren't in the same group as $N.",
		    ch, NULL, victim, TO_CHAR);
		return;
	}

	if (IS_AFFECTED(ch, AFF_CHARM)) {
		act("You like your master too much to bother guarding $N!",
		    ch, NULL, victim, TO_VICT);
		return;
	}

	if (victim->fighting != NULL) {
		char_puts("Why don't you let them stop fighting first?\n",
			     ch);
		return;
	}
	
	if (ch->fighting != NULL) {
		char_puts("Better finish your own battle before you "
			     "worry about guarding someone else.\n", ch);
		return;
	}

	act("You are now guarding $N.", ch, NULL, victim, TO_CHAR);
	act("You are being guarded by $n.", ch, NULL, victim, TO_VICT);
	act("$n is now guarding $N.", ch, NULL, victim, TO_NOTVICT);

	PC(ch)->guarding = victim;
	PC(victim)->guarded_by = ch;
}

void do_explode(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *victim = ch->fighting;
	CHAR_DATA *vch, *vch_next;
	int dam=0,hp_dam,dice_dam,mana;
	int hpch,level= LEVEL(ch);
	bool attack;
	char arg[MAX_INPUT_LENGTH];
	int chance;

	if ((chance = get_skill(ch, "explode")) == 0) {
		char_puts("Flame? What is that?\n", ch);
		return;
	}

	if (ch->fighting)
		victim = ch->fighting;
	else {
		one_argument(argument, arg, sizeof(arg));
		if (arg[0] == '\0') { 
			char_puts("You play with the exploding material.\n",
				  ch);
			return;
		}
	}

	if (!victim || victim->in_room != ch->in_room) {
		WAIT_STATE(ch, MISSING_TARGET_DELAY);
		char_puts("They aren't here.\n", ch);
		return;
	}

	mana = skill_mana(ch, "explode");
	if (ch->mana < mana) {
		char_puts("You can't find that much energy to fire\n", ch);
		return;
	}
	ch->mana -= mana;
	WAIT_STATE(ch, skill_beats("explode"));

	act("$n burns something.", ch, NULL, victim, TO_NOTVICT);
	act("$n burns a cone of exploding material over you!",
	    ch, NULL, victim, TO_VICT);
	act("Burn them all!.", ch, NULL, NULL, TO_CHAR);

	if (number_percent() >= chance) {
		damage(ch, victim, 0, "explode", DAM_FIRE, TRUE);
		check_improve(ch, "explode", FALSE, 1);
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

		attack = (ch->fighting != vch);

		if (is_safe_spell(ch,vch,TRUE)
		||  (IS_NPC(vch) && IS_NPC(ch)
		&&   (ch->fighting != vch || vch->fighting != ch)))
			  continue;

		if (vch == victim) { /* full damage */
			fire_effect(vch, level, dam, TARGET_CHAR);
			damage(ch, vch, dam, "explode", DAM_FIRE, TRUE);
		} else { /* partial damage */
			fire_effect(vch, level/2, dam/4, TARGET_CHAR);
			damage(ch, vch, dam/2, "explode", DAM_FIRE,TRUE);
		}
		yell(vch, ch, "Help! $lu{$i} tries to burn me!");
	}

	if (number_percent() >= chance) {
		fire_effect(ch, level/4, dam/10, TARGET_CHAR);
		damage(ch, ch, (ch->hit / 10), "explode", DAM_FIRE, TRUE);
	}
}

void do_target(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *victim;
	int chance;

	if ((chance = get_skill(ch, "target")) == 0) {
		char_puts("You don't know how to change the target "
			  "while fighting a group.\n" ,ch);
		return;
	}

	if (ch->fighting == NULL) {
		char_puts("You aren't fighting yet.\n",ch);
		return;
	}

	if (argument[0] == '\0') {
		char_puts("Change target to whom?\n",ch);
		return;
	}

	if ((victim = get_char_room(ch, argument)) == NULL) {
		char_puts("You don't see that item.\n",ch);
		return;
	}

	/* check victim is fighting with him */

	if (victim->fighting != ch) {
		char_puts ("Target is not fighting with you.\n",ch);
		return;
	}

	WAIT_STATE(ch, skill_beats("target"));

	if (number_percent() < chance / 2) {
		check_improve(ch, "target", TRUE, 1);

		ch->fighting = victim;

		act("$n changes $s target to $N!",
		    ch, NULL, victim, TO_NOTVICT);
		act("You change your target to $N!",
		    ch, NULL, victim, TO_CHAR);
		act("$n changes target to you!", ch, NULL, victim, TO_VICT);
		return;
	}

	char_puts("You tried, but you couldn't. "
		  "But for honour try again!.\n", ch);
	check_improve(ch, "target", FALSE, 1);
}

void do_tiger(CHAR_DATA *ch, const char *argument)
{
	int chance, hp_percent;
	int mana;

	if ((chance = get_skill(ch, "tiger power")) == 0) {
		char_puts("Huh?\n", ch);
		return;
	}
	act("$n calls the power of 10 tigers!.", ch, NULL, NULL, TO_ROOM);

	if (IS_AFFECTED(ch, AFF_BERSERK) || is_affected(ch, "berserk")
	||  is_affected(ch, "tiger power") || is_affected(ch, "frenzy")) {
		char_puts("You get a little madder.\n", ch);
		return;
	}

	if (IS_AFFECTED(ch,AFF_CALM)) {
		char_puts("You're feeling too mellow to call 10 tigers.\n",
			  ch);
		return;
	}

	if (ch->in_room->sector_type != SECT_FIELD
	&&  ch->in_room->sector_type != SECT_FOREST
	&&  ch->in_room->sector_type != SECT_MOUNTAIN
	&&  ch->in_room->sector_type != SECT_HILLS) {
		char_puts("No tigers can hear your call.\n", ch);
		return;
	}

	mana = skill_mana(ch, "tiger power");
	if (ch->mana < mana) {
		char_puts("You can't get up enough energy.\n", ch);
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

		WAIT_STATE(ch, skill_beats("tiger power"));
		ch->mana -= mana;

		/* heal a little damage */
		ch->hit += LEVEL(ch) * 2;
		ch->hit = UMIN(ch->hit, ch->max_hit);

		char_puts("10 tigers come for your call, as you call them!\n",
			  ch);
		act("10 tigers come across $n, and connect with $n.",
		    ch, NULL, NULL, TO_ROOM);
		check_improve(ch, "tiger power", TRUE, 2);

		af.where	= TO_AFFECTS;
		af.type		= "tiger power";
		af.level	= ch->level;
		af.duration	= number_fuzzy(ch->level / 8);
		af.modifier	= UMAX(1, LEVEL(ch)/5);
		af.bitvector 	= AFF_BERSERK;

		af.location	= APPLY_HITROLL;
		affect_to_char(ch,&af);

		af.location	= APPLY_DAMROLL;
		affect_to_char(ch,&af);

		af.modifier	= UMAX(10,10 * (LEVEL(ch)/5));
		af.location	= APPLY_AC;
		affect_to_char(ch,&af);
	}
	else {
		WAIT_STATE(ch, 2 * skill_beats("tiger power"));
		ch->mana -= mana/2;
		char_puts("Your feel stregthen up, but nothing happens.\n",
			  ch);
		check_improve(ch, "tiger power", FALSE, 2);
	}
}

void do_hara(CHAR_DATA *ch, const char *argument)
{
	int chance;
	AFFECT_DATA  af;

	if (MOUNTED(ch)) {
		char_puts("You can't harakiri while riding!\n", ch);
		return;
	}

	if ((chance = get_skill(ch, "hara kiri")) == 0) {
		char_puts("You try to kill yourself, but you can't resist this ache.\n",ch);
		return;
	}

	if (is_affected(ch, "hara kiri")) {
		char_puts("If you want to kill yourself go and try to kill He-Man.\n",ch);
		return;
	}

	/* fighting */
	if (ch->position == POS_FIGHTING) {
		char_puts("Try your chance during fighting.\n", ch);
		return;
	}

	if (number_percent() < chance) {
		AFFECT_DATA af;

		WAIT_STATE(ch, skill_beats("hara kiri"));

		ch->hit = 1;
		ch->mana = 1;
		ch->move = 1;

		if (PC(ch)->condition[COND_HUNGER] < 40) 
			PC(ch)->condition[COND_HUNGER] = 40; 
		if (PC(ch)->condition[COND_THIRST] < 40) 
			PC(ch)->condition[COND_THIRST] = 40; 

		char_puts("Yo cut your finger and wait till all your blood "
			  "finishes.\n",ch);
		act_puts("$n cuts his body and look in a deadly figure.",
			 ch, NULL, NULL, TO_ROOM, POS_FIGHTING);
		check_improve(ch, "hara kiri", TRUE, 2);
		do_sleep(ch, str_empty);
		SET_BIT(PC(ch)->plr_flags,PLR_HARA_KIRI);

		af.where     = TO_AFFECTS;
		af.type      = "hara kiri";
		af.level     = ch->level;
		af.duration  = 10;
		af.location  = APPLY_NONE;
		af.modifier  = 0;
		af.bitvector = 0;
		affect_to_char(ch, &af);
	}
	else {
		WAIT_STATE(ch, 2 * skill_beats("hara kiri"));

		af.where     = TO_AFFECTS;
		af.type      = "hara kiri";
		af.level     = ch->level;
		af.duration  = 0;
		af.location  = APPLY_NONE;
		af.modifier  = 0;
		af.bitvector = 0;
		affect_to_char(ch, &af);

		char_puts("You couldn't cut your finger. "
			  "It is not so easy as you know.\n", ch);
		check_improve(ch, "hara kiri", FALSE, 2);
	}
}

void do_shield(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *victim;
	int chance,ch_weapon,vict_shield;
	OBJ_DATA *shield,*axe;

	if (IS_NPC(ch))
		return;

	if ((victim = ch->fighting) == NULL) {
		char_puts("You aren't fighting anyone.\n", ch);
		return;
	}
		
	if (victim->in_room != ch->in_room) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	if ((axe = get_eq_char(ch,WEAR_WIELD)) == NULL) {
		char_puts("You must be wielding a weapon.\n", ch);
		return;
	}

	if ((chance = get_skill(ch, "shield cleave")) == 0) {
		char_puts("You don't know how to cleave opponents's shield.\n",
			  ch);
		return;
	}

	if ((shield = get_eq_char(victim, WEAR_SHIELD)) == NULL) {
		char_puts("Your opponent must wield a shield.\n", ch);
		return;
	}

	if (material_is(shield, MATERIAL_INDESTRUCT))
		return;

	if (WEAPON_IS(axe, WEAPON_AXE))
		chance *= 1.2;
	else if (!WEAPON_IS(axe, WEAPON_SWORD)) {
		char_puts("Your weapon must be an axe or a sword.\n", ch);
		return;
	}

	/* find weapon skills */
	ch_weapon = get_weapon_skill(ch, get_weapon_sn(axe));
	vict_shield = get_skill(ch, "shield block");
	/* modifiers */

	/* skill */
	chance = chance * ch_weapon / 200;
	if (vict_shield)
		chance = chance * 100 / vict_shield;

	/* dex vs. strength */
	chance += get_curr_stat(ch, STAT_DEX);
	chance -= 2 * get_curr_stat(victim, STAT_STR);

	/* level */
/*	chance += (ch->level - victim->level) * 2; */
	chance += LEVEL(ch) - LEVEL(victim);
	chance += axe->level - shield->level;
 
	/* and now the attack */
	SET_BIT(ch->affected_by, AFF_WEAK_STUN);
	WAIT_STATE(ch, skill_beats("shield cleave"));
	if (number_percent() < chance) {
		act("You cleaved $N's shield into two.",
		    ch, NULL, victim, TO_CHAR);
		act("$n cleaved your shield into two.",
		    ch, NULL, victim, TO_VICT);
		act("$n cleaved $N's shield into two.",
		    ch, NULL, victim, TO_NOTVICT);
		check_improve(ch, "shield cleave", TRUE, 1);
		extract_obj(get_eq_char(victim, WEAR_SHIELD), 0);
	}
	else {
		act("You fail to cleave $N's shield.",
		    ch, NULL, victim, TO_CHAR);
		act("$n tries to cleave your shield, but fails.",
		    ch, NULL, victim, TO_VICT);
		act("$n tries to cleave $N's shield, but fails.",
		    ch, NULL, victim, TO_NOTVICT);
		check_improve(ch, "shield cleave", FALSE, 1);
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
		char_puts("You aren't fighting anyone.\n", ch);
		return;
	}

	if ((axe = get_eq_char(ch, WEAR_WIELD)) == NULL) {
		char_puts("You must be wielding a weapon.\n",ch);
		return;
	}

	if ((chance = get_skill(ch, "weapon cleave")) == 0) {
		char_puts("You don't know how to cleave opponents's weapon.\n",ch);
		return;
	}

	if ((wield = get_eq_char(victim, WEAR_WIELD)) == NULL) {
		char_puts("Your opponent must wield a weapon.\n", ch);
		return;
	}

	if (material_is(wield, MATERIAL_INDESTRUCT))
		return;

	if (WEAPON_IS(axe, WEAPON_AXE))
		chance *= 1.2;
	else if (!WEAPON_IS(axe, WEAPON_SWORD)) {
		char_puts("Your weapon must be an axe or a sword.\n",ch);
		return;
	}

	/* staves are easy to break */
	if (WEAPON_IS(wield, WEAPON_STAFF))
		chance += 50;

	/* find weapon skills */
	ch_weapon = get_weapon_skill(ch, get_weapon_sn(axe));
	vict_weapon = get_weapon_skill(victim, get_weapon_sn(wield));
	/* modifiers */

	/* skill */
	chance = chance * ch_weapon / 200;
	chance = chance * 101 / (vict_weapon+1);

	/* dex vs. strength */
	chance += get_curr_stat(ch, STAT_DEX) + get_curr_stat(ch, STAT_STR);
	chance -= get_curr_stat(victim, STAT_STR) +
				2 * get_curr_stat(victim, STAT_DEX);

	chance += LEVEL(ch) - LEVEL(victim);
	chance += axe->level - wield->level;
 
	/* and now the attack */
	SET_BIT(ch->affected_by, AFF_WEAK_STUN);
	WAIT_STATE(ch, skill_beats("weapon cleave"));
	if (number_percent() < chance) {
		act("You cleaved $N's weapon into two.",
		    ch, NULL, victim, TO_CHAR);
		act("$n cleaved your weapon into two.",
		    ch, NULL, victim, TO_VICT);
		act("$n cleaved $N's weapon into two.",
		    ch, NULL, victim, TO_NOTVICT);
		check_improve(ch, "weapon cleave", TRUE, 1);
		extract_obj(get_eq_char(victim, WEAR_WIELD), 0);
	}
	else {
		act("You fail to cleave $N's weapon.",
		    ch, NULL, victim, TO_CHAR);
		act("$n tries to cleave your weapon, but fails.",
		    ch, NULL, victim, TO_VICT);
		act("$n tries to cleave $N's weapon, but fails.",
		    ch, NULL, victim, TO_NOTVICT);
		check_improve(ch, "weapon cleave", FALSE, 1);
	}
}

void do_tail(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int chance, wait;
	int damage_tail;
	bool attack;

	if (MOUNTED(ch)) {
		char_puts("You can't tail while riding!\n", ch);
		return;
	}

	one_argument(argument, arg, sizeof(arg));
 
	if ((chance = get_skill(ch, "tail")) == 0) {
		char_puts("Huh?\n", ch);
		return;
	}
 
	if (arg[0] == '\0') {
		victim = ch->fighting;
		if (victim == NULL) {
			char_puts("But you aren't fighting anyone!\n", ch);
			return;
		}
	}
	else
		victim = get_char_room(ch, arg);

	if (!victim || victim->in_room != ch->in_room) {
		char_puts("They aren't here.\n", ch);
		WAIT_STATE(ch, MISSING_TARGET_DELAY);
		return;
	}

	WAIT_STATE(ch, skill_beats("tail"));

/*
	if (victim->position < POS_FIGHTING) {
		act("You'll have to let $M get back up first.",
		    ch, NULL, victim, TO_CHAR);
		return;
	} 
*/
	if (victim == ch) {
		char_puts("You try to hit yourself by your tail, but failed.\n",
			  ch);
		return;
	}

	if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim) {
		act("But $N is your friend!", ch, NULL, victim, TO_CHAR);
		return;
	}
		
	if (is_affected(victim, "protective shield")) {
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
	if (IS_NPC(ch) && IS_SET(ch->pMobIndex->off_flags, OFF_FAST))
		chance += 20;
	if (IS_NPC(victim) && IS_SET(victim->pMobIndex->off_flags, OFF_FAST))
		chance -= 30;

	/* level */
	chance += (LEVEL(ch) - LEVEL(victim)) * 2;

	RESET_WAIT_STATE(ch);
	attack = (ch->fighting != victim);

	/* now the attack */
	if (number_percent() < (chance / 4)) {
		act("$n sends you sprawling with a powerful tail!",
		    ch, NULL, victim, TO_VICT);
		act("You sprawle $N with your tail , and send $M flying!",
		    ch, NULL, victim, TO_CHAR);
		act("$n sends $N sprawling with a powerful tail.",
		    ch, NULL, victim, TO_NOTVICT);
		check_improve(ch, "tail", TRUE, 1);

		wait = 3;

		switch(number_bits(2)) {
		case 0: wait = 1; break;
		case 1: wait = 2; break;
		case 2: wait = 4; break;
		case 3: wait = 3; break;
		}

		WAIT_STATE(victim, wait * PULSE_VIOLENCE);
		WAIT_STATE(ch, skill_beats("tail"));
		victim->position = POS_RESTING;
		damage_tail = ch->damroll + 
			(2 * number_range(4,4 + 10* ch->size + chance/10));
		damage(ch, victim, damage_tail, "tail", DAM_BASH, TRUE);
	} else {
		damage(ch, victim, 0, "tail", DAM_BASH, TRUE);
		act("You lost your position and fall down!",
		    ch, NULL, victim, TO_CHAR);
		act("$n lost $s position and fall down!.",
		    ch, NULL, victim, TO_NOTVICT);
		act("You evade $n's tail, causing $m to fall down.",
		    ch, NULL, victim, TO_VICT);
		check_improve(ch, "tail", FALSE, 1);
		ch->position = POS_RESTING;
		WAIT_STATE(ch, skill_beats("tail") * 3 / 2); 
	}
	if (attack)
		yell(victim, ch, "Help! $lu{$i} tried to hit me with his tail!");
}

void do_concentrate(CHAR_DATA *ch, const char *argument)
{
	int chance;
	int mana;

	if (MOUNTED(ch)) {
		char_puts("You can't concentrate while riding!\n", ch);
		return;
	}

	if ((chance = get_skill(ch, "concentrate")) == 0) {
		char_puts("You try to concentrate on what is going on.\n",
			  ch);
		return;
	}

	if (is_affected(ch, "concentrate")) {
		char_puts("You are already concentrated for the fight.\n",
			  ch);
		return;
	}
		
	mana = skill_mana(ch, "concentrate");
	if (ch->mana < mana) {
		char_puts("You can't get up enough energy.\n", ch);
		return;
	}

	/* fighting */
	if (ch->fighting) {
		char_puts("Concentrate on your fighting!\n", ch);
		return;
	}

	WAIT_STATE(ch, skill_beats("concentrate"));
	if (number_percent() < chance) {
		AFFECT_DATA af;

		ch->mana -= mana;
		ch->move /= 2;

		do_sit(ch,str_empty);
		char_puts("You sit down and relax, "
			  "concentrating on the next fight.!\n", ch);
		act_puts("$n concentrates for the next fight.",
			 ch, NULL, NULL, TO_ROOM, POS_FIGHTING);
		check_improve(ch, "concentrate", TRUE, 2);

		af.where	= TO_AFFECTS;
		af.type		= "concentrate";
		af.level	= ch->level;
		af.duration	= number_fuzzy(ch->level / 8);
		af.modifier	= UMAX(1, LEVEL(ch)/8);
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
		char_puts("You try to concentrate for the next fight but fail.\n",
			  ch);
		check_improve(ch, "concentrate", FALSE, 2);
	}
}

void do_bandage(CHAR_DATA *ch, const char *argument)
{
	int heal;
	int chance;

	if ((chance = get_skill(ch, "bandage")) == 0) {
		char_puts("Huh?\n", ch);
		return;
	}

	if (IS_AFFECTED(ch, AFF_REGENERATION)) {
		char_puts("You have already using your bandage.\n", ch);
		return;
	}

	WAIT_STATE(ch, skill_beats("bandage"));
	if (number_percent() < chance) {
		AFFECT_DATA af;

		char_puts("You place your bandage to your shoulder!\n", ch);
		act("$n places a bandage to $s shoulder.",
		    ch, NULL, NULL, TO_ROOM);
		check_improve(ch, "bandage", TRUE, 2);

		heal = dice(4, 8) + LEVEL(ch) / 2;
		ch->hit = UMIN(ch->hit + heal, ch->max_hit);
		update_pos(ch);
		char_puts("You feel better!\n", ch);

		af.where	= TO_AFFECTS;
		af.type		= "bandage";
		af.level	= ch->level;
		af.duration	= ch->level / 10;
		af.modifier	= UMIN(15,ch->level/2);
		af.bitvector 	= AFF_REGENERATION;
		af.location	= 0;
		affect_to_char(ch,&af);
	}
	else {
		char_puts("You failed to place your bandage to your shoulder.\n",
			  ch);
		check_improve(ch, "bandage", FALSE, 2);
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

	one_argument(argument, arg, sizeof(arg));
	
	if ((chance = get_skill(ch, "katana")) == 0) {
		char_puts("Huh?\n", ch);
		return;
	}
	
	if (is_affected(ch, "katana")) {
		char_puts("But you've already got one katana!\n", ch);
		return;
	}
	
	mana = skill_mana(ch, "katana");
	if (ch->mana < mana) {
		char_puts("You feel too weak to concentrate on a katana.\n",
			  ch);
		return;
	}
	
	if (arg[0] == '\0') {
		char_puts("Make a katana from what?\n", ch);
		return;
	}
	
	if ((part = get_obj_carry(ch, arg)) == NULL) {
		char_puts("You do not have chunk of iron.\n", ch);
		return;
	}

	if (part->pObjIndex->vnum != OBJ_VNUM_CHUNK_IRON) {
		char_puts("You do not have the right material.\n", ch);
		return;
	}

	if (number_percent() < chance / 3 * 2) {
		char_puts("You failed and destroyed it.\n", ch);
		extract_obj(part, 0);
		return;
	} 

	WAIT_STATE(ch, skill_beats("katana"));

	if (number_percent() < chance) {
		af.where	= TO_AFFECTS;
		af.type		= "katana";
		af.level	= ch->level;
		af.duration	= ch->level;
		af.modifier	= 0;
		af.bitvector 	= 0;      
		af.location	= 0;
		affect_to_char(ch,&af);
	
		katana = create_obj(get_obj_index(OBJ_VNUM_KATANA_SWORD), 0);
		katana->level = ch->level;
		mlstr_cpy(&katana->owner, &ch->short_descr);
		katana->cost  = 0;
		ch->mana -= mana;

		af.where	= TO_OBJECT;
		af.type 	= "katana";
		af.level	= ch->level;
		af.duration	= -1;
		af.location	= APPLY_DAMROLL;
		af.modifier	= ch->level / 10;
		af.bitvector	= 0;
		affect_to_obj(katana, &af);

		af.location	= APPLY_HITROLL;
		affect_to_obj(katana, &af);

		INT_VAL(katana->value[2]) = ch->level / 10;
		katana->ed = ed_new2(katana->pObjIndex->ed, ch->name);
			
		obj_to_char(katana, ch);
		check_improve(ch, "katana", TRUE, 1);
			
		act("You make a katana from $p!",ch,part,NULL,TO_CHAR);
		act("$n makes a katana from $p!",ch,part,NULL,TO_ROOM);
			
		extract_obj(part, 0);
		return;
	} else {
		char_puts("You destroyed it.\n", ch);
		extract_obj(part, 0);
		ch->mana -= mana/2;
		check_improve(ch, "katana", FALSE, 1);
	}
}

void do_crush(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *victim;
	int chance, wait;
	int damage_crush;

	if (MOUNTED(ch)) 
		return;

	if ((chance = get_skill(ch, "crush")) == 0) {
		char_puts("You don't know how to crush.\n", ch);
		return;
	}
 
	if ((victim = ch->fighting) == NULL || victim->in_room != ch->in_room)
		return;

	if (victim->position < POS_FIGHTING)
		return;

	if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
		return;
		
	if (is_affected(victim, "protective shield")) {
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
	if (IS_NPC(ch) && IS_SET(ch->pMobIndex->off_flags, OFF_FAST))
		chance += 10;
	if (IS_NPC(victim) && IS_SET(victim->pMobIndex->off_flags, OFF_FAST))
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
		WAIT_STATE(ch, skill_beats("crush"));
		victim->position = POS_RESTING;
		damage_crush = (ch->damroll / 2) +
				number_range(4, 4 + 4*ch->size + chance/10);
		damage(ch, victim, damage_crush, "crush", DAM_BASH, TRUE);
	}
	else {
		damage(ch, victim, 0, "crush", DAM_BASH, TRUE);
		act_puts("You fall flat on your face!",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		act("$n falls flat on $s face.",
		    ch, NULL, victim, TO_NOTVICT);
		act("You evade $n's crush, causing $m to fall flat on $s face.",
		    ch, NULL, victim, TO_VICT);
		ch->position = POS_RESTING;
		WAIT_STATE(ch, skill_beats("crush") * 3 / 2); 
	}
}

void do_sense(CHAR_DATA *ch, const char *argument)
{
	int chance;
	int mana;

	if ((chance = get_skill(ch, "sense life")) == 0) {
		char_puts("Huh?\n", ch);
		return;
	}

	if (is_affected(ch, "sense life")) {
		char_puts("You can already feel life forms.\n", ch);
		return;
	}

	mana = skill_mana(ch, "sense life");
	if (ch->mana < mana) {
		char_puts("You cannot seem to concentrate enough.\n", ch);
		return;
	}

	WAIT_STATE(ch, skill_beats("sense life"));

	if (number_percent() < chance) {
		AFFECT_DATA af;
		
		af.where	= TO_AFFECTS;
		af.type 	= "sense life";
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
		check_improve(ch, "sense life", TRUE, 1);
	}
	else {
		ch->mana -= mana/2;
		char_puts("You failed.\n" ,ch);
		check_improve(ch, "sense life", FALSE, 1);
	}
}

void do_poison_smoke(CHAR_DATA *ch, const char *argument) 
{
	int chance;
	int mana;
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;
	bool attack;

	if ((chance = get_skill(ch, "poison smoke")) == 0) {
		char_puts("Huh?\n", ch);
		return;
	}

	mana = skill_mana(ch, "poison smoke");
	if (ch->mana < mana) {
		char_puts("You can't get up enough energy.\n", ch);
		return;
	}
	ch->mana -= mana;
	WAIT_STATE(ch, skill_beats("poison smoke"));

	if (number_percent() > chance) {
		char_puts("You failed.\n", ch);
		check_improve(ch, "poison smoke", FALSE, 1);
		return;
	}

	char_puts("A cloud of poison smoke fills the room.\n", ch);
	act("A cloud of poison smoke fills the room.", ch, NULL, NULL, TO_ROOM);

	check_improve(ch, "poison smoke", TRUE, 1);

	for (vch = ch->in_room->people; vch; vch = vch_next) {
		vch_next = vch->next_in_room;

		if (is_safe_spell(ch, vch, TRUE))
			continue;
		attack = (ch->fighting != vch);

		spellfun_call("poison", NULL, LEVEL(ch), ch, vch);
		if (vch != ch) {
			if (attack) 
				yell(vch, ch, "$i tries to poison me!");
			multi_hit(vch, ch, TYPE_UNDEFINED);
		}
	}
}

void do_blindness_dust(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;
	int chance;
	char arg[MAX_INPUT_LENGTH];
	bool attack;
	int mana;

	if ((chance = get_skill(ch, "blindness dust")) == 0) {
		char_puts("Huh?\n", ch);
		return;
	}

	mana = skill_mana(ch, "blindness dust");
	if (ch->mana < mana) {
		char_puts("You can't get up enough energy.\n", ch);
		return;
	}
	ch->mana -= mana;
	WAIT_STATE(ch, skill_beats("blindness dust"));

	if (number_percent() > chance) {
		char_puts("You failed.\n",ch);
		check_improve(ch, "blindness dust", FALSE, 1);
		return;
	}

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		char_puts("A cloud of dust fills in the room.\n", ch);
		act("A cloud of dust fills the room.", ch, NULL, NULL, TO_ROOM);

		check_improve(ch, "blindness dust", TRUE, 1);

		for (vch = ch->in_room->people; vch; vch = vch_next) {
			vch_next = vch->next_in_room;
			
			attack = (ch->fighting != vch);

			if (is_safe_spell(ch, vch, TRUE)) 
				continue;

			spellfun_call("blindness", NULL, LEVEL(ch), ch, vch);
			if (attack)
				yell(vch, ch, "Help! $lu{$i} just threw dust into my eyes!");
			if (vch != ch)
				multi_hit(vch, ch, TYPE_UNDEFINED);
		}
	}
	else {
		if ((vch = get_char_room(ch, arg)) == NULL) {
			char_puts("They aren't here.\n", ch);
			return;
		}
		if (is_safe(ch, vch))
			return;
		act("You throw some dust into $N's eyes.", ch, NULL, vch, TO_CHAR);
		act("$n throws some dust into $N's eyes.", ch, NULL, vch, TO_ROOM);
		act("$n throws some dust into your eyes.", ch, NULL, vch, TO_VICT);
		spellfun_call("blindness", NULL, LEVEL(ch), ch, vch);
		if (vch != ch)
			multi_hit(vch, ch, TYPE_UNDEFINED);
	}
}

void do_dishonor(CHAR_DATA *ch, const char *argument)
{
	ROOM_INDEX_DATA *was_in;
	ROOM_INDEX_DATA *now_in;
	CHAR_DATA *gch;
	int attempt, level = 0;
	int chance;

	if (RIDDEN(ch)) {
		char_puts("You should ask to your rider!\n", ch);
		return;
	}

	if (!HAS_SKILL(ch, "dishonor")) {
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
	chance = get_skill(ch, "dishonor");
	for (attempt = 0; attempt < 6; attempt++) {
		EXIT_DATA *pexit;
		int door;

		if (number_percent() >= chance)
			continue;

		door = number_door();
		if ((pexit = was_in->exit[door]) == 0
		||  pexit->to_room.r == NULL
		||  (IS_SET(pexit->exit_info, EX_CLOSED) &&
		     (!IS_AFFECTED(ch, AFF_PASS_DOOR) ||
		      IS_SET(pexit->exit_info,EX_NOPASS)) &&
		     !IS_TRUSTED(ch, LEVEL_IMMORTAL))
		|| IS_SET(pexit->exit_info, EX_NOFLEE)
		|| (IS_NPC(ch) &&
		    IS_SET(pexit->to_room.r->room_flags, ROOM_NOMOB)))
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
		else {
			/* Once fled, the mob will not go after */
			NPC(ch)->last_fought = NULL;
		}

		stop_fighting(ch, TRUE);
		if (MOUNTED(ch))
			dofun("dismount", ch, str_empty);

		check_improve(ch, "dishonor", TRUE, 1);
		return;
	}

	char_puts("PANIC! You couldn't escape!\n", ch);
	check_improve(ch, "dishonor", FALSE, 1);
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

static inline bool check_yell(CHAR_DATA *ch, CHAR_DATA *victim, bool fighting)
{
	return (!IS_NPC(ch) && !IS_NPC(victim) &&
		victim->position > POS_STUNNED && !fighting);
}

