/*
 * $Id: martial_art.c,v 1.225 2003-10-10 14:28:08 fjoe Exp $
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

#include <stdio.h>

#include <merc.h>

#include <sog.h>
#include <update.h>

DECLARE_DO_FUN(do_kill);
DECLARE_DO_FUN(do_murder);
DECLARE_DO_FUN(do_flee);
DECLARE_DO_FUN(do_pound);
DECLARE_DO_FUN(do_entangle);
DECLARE_DO_FUN(do_gash);
DECLARE_DO_FUN(do_cut);
DECLARE_DO_FUN(do_hunger);
DECLARE_DO_FUN(do_whirl);
DECLARE_DO_FUN(do_feint);
DECLARE_DO_FUN(do_berserk);
DECLARE_DO_FUN(do_breath);
DECLARE_DO_FUN(do_bash);
DECLARE_DO_FUN(do_dirt);
DECLARE_DO_FUN(do_trip);
DECLARE_DO_FUN(do_backstab);
DECLARE_DO_FUN(do_knife);
DECLARE_DO_FUN(do_cleave);
DECLARE_DO_FUN(do_impale);
DECLARE_DO_FUN(do_ambush);
DECLARE_DO_FUN(do_rescue);
DECLARE_DO_FUN(do_kick);
DECLARE_DO_FUN(do_circle);
DECLARE_DO_FUN(do_disarm);
DECLARE_DO_FUN(do_strip);
DECLARE_DO_FUN(do_nerve);
DECLARE_DO_FUN(do_endure);
DECLARE_DO_FUN(do_tame);
DECLARE_DO_FUN(do_assassinate);
DECLARE_DO_FUN(do_caltrops);
DECLARE_DO_FUN(do_throw);
DECLARE_DO_FUN(do_strangle);
DECLARE_DO_FUN(do_headcrush);
DECLARE_DO_FUN(do_blackjack);
DECLARE_DO_FUN(do_bloodthirst);
DECLARE_DO_FUN(do_toughen);
DECLARE_DO_FUN(do_trophy);
DECLARE_DO_FUN(do_truesight);
DECLARE_DO_FUN(do_warcry);
DECLARE_DO_FUN(do_guard);
DECLARE_DO_FUN(do_explode);
DECLARE_DO_FUN(do_target);
DECLARE_DO_FUN(do_tiger);
DECLARE_DO_FUN(do_hara);
DECLARE_DO_FUN(do_shield);
DECLARE_DO_FUN(do_weapon);
DECLARE_DO_FUN(do_tail);
DECLARE_DO_FUN(do_concentrate);
DECLARE_DO_FUN(do_bandage);
DECLARE_DO_FUN(do_katana);
DECLARE_DO_FUN(do_crush);
DECLARE_DO_FUN(do_sense);
DECLARE_DO_FUN(do_poison_smoke);
DECLARE_DO_FUN(do_blindness_dust);
DECLARE_DO_FUN(do_dishonor);
DECLARE_DO_FUN(do_surrender);
DECLARE_DO_FUN(do_rake);

/* command procedures needed */
DECLARE_DO_FUN(do_yell);
DECLARE_DO_FUN(do_sleep);
DECLARE_DO_FUN(do_sit);
DECLARE_DO_FUN(do_bash_door);
DECLARE_DO_FUN(do_dismount);
DECLARE_DO_FUN(do_throw_weapon);

static bool
check_close_contact(CHAR_DATA *ch, CHAR_DATA *victim)
{
	OBJ_DATA *v_weapon;
	OBJ_DATA *v_weapon2;
	int chance;

	if (IS_EXTRACTED(ch) || IS_EXTRACTED(victim))
		return FALSE;

	v_weapon = get_eq_char(victim, WEAR_WIELD);
	v_weapon2 = get_eq_char(victim, WEAR_SECOND_WIELD);
	chance = get_skill(victim, "close contact");

	if (!chance || !v_weapon)
		return FALSE;

	if (!WEAPON_IS(v_weapon, WEAPON_DAGGER)
	&&  !(v_weapon2 && WEAPON_IS(v_weapon2, WEAPON_DAGGER)))
		return FALSE;

	chance += get_curr_stat(victim, STAT_DEX);

	if (number_percent() < chance / 9) {
		act("Too close.. $N turns you into bloody mess with rapid dagger blows.", ch, NULL, victim, TO_CHAR);
		act("$n gets too close to you. It's $s fault.",
		    ch, NULL, victim, TO_VICT);

		while (number_percent() < chance) {
			if (IS_EXTRACTED(ch))
				return TRUE;
			if(WEAPON_IS(v_weapon, WEAPON_DAGGER))
				one_hit(victim, ch, "close contact",
					WEAR_WIELD);
			if (IS_EXTRACTED(ch))
				return TRUE;
			if(v_weapon2 && WEAPON_IS(v_weapon2, WEAPON_DAGGER))
				one_hit(victim, ch, "close contact",
					WEAR_SECOND_WIELD);
			chance /= 3;
		}
		check_improve(victim, "close contact", TRUE, 2);
		return TRUE;
	}
	return FALSE;
}

static bool
check_reversal(CHAR_DATA *ch, CHAR_DATA *victim)
{
	int chance;

	if (IS_EXTRACTED(ch) || IS_EXTRACTED(victim))
		return FALSE;

	chance = get_skill(victim, "reversal");
	if (get_eq_char(victim, WEAR_WIELD))
		return FALSE;

	if (!chance || !free_hands(ch))
		return FALSE;

	chance += get_curr_stat(victim, STAT_DEX);

	if (number_percent() < chance / 7) {
		act("You make a mistake coming too close to $M.",
			ch, NULL, victim, TO_CHAR);
		act("$n gets too close to you. It's $s fault.",
		ch, NULL, victim, TO_VICT);
		switch(number_bits(2)) {
		case 1:
			act("You grab $n and throw $m to the ground.",
				ch, NULL, victim, TO_VICT);
			act("$N grabs you and throws to the ground.",
				ch, NULL, victim, TO_CHAR);
			WAIT_STATE(ch, 2 * get_pulse("violence"));

			damage(victim, ch,
			    (LEVEL(victim) + get_curr_stat(victim, STAT_STR)),
			    "throw", DAM_F_SHOW);
			return TRUE;
		default:
			act("You foresee $N's maneuver and meet $M well "
			"prepared.", ch, NULL, victim, TO_CHAR);
			act("$n foresees your maneuver. You should be faster "
			"next time.", ch, NULL, victim, TO_CHAR);

			while(number_percent() < chance) {
				if (IS_EXTRACTED(ch))
					return TRUE;
				one_hit(victim, ch, "reversal", WEAR_WIELD);
				if (IS_EXTRACTED(ch))
					return TRUE;
				if (free_hands(ch) > 1)
					one_hit(victim, ch, "reversal", WEAR_WIELD);
				chance /= 3;
			}
		}
		check_improve(victim, "reversal", TRUE, 2);
		return TRUE;
	}

	return FALSE;
}

static bool
check_close(CHAR_DATA* ch, CHAR_DATA* victim)
{
	if (check_close_contact(ch, victim))
		return TRUE;
	else
		return check_reversal(ch, victim);
}

static void
check_downstrike(CHAR_DATA *victim)
{
	CHAR_DATA *ch;

	foreach (ch, char_in_room(victim->in_room)) {
		OBJ_DATA *weapon;
		int chance;

		if (IS_EXTRACTED(victim))
			break;

		if (ch->fighting != victim
		||  (weapon = get_eq_char(ch, WEAR_WIELD)) == NULL
		||  !WEAPON_IS(weapon, WEAPON_DAGGER)
		||  (chance = get_skill(ch, "downstrike")) != 0)
			continue;

		chance += get_curr_stat(ch, STAT_DEX);
		chance -= get_curr_stat(victim, STAT_DEX);
		chance /= 3;

		if (number_percent() < chance) {
			act("You stab $N down as $E falls to the ground.",
			    ch, NULL, victim, TO_CHAR);
			act("$n takes advantage of your fall, stabbing you down.",
			    ch, NULL, victim, TO_VICT);
			act("$n takes advantage of $N's fall.",
			    ch, NULL, victim, TO_NOTVICT);
			one_hit(ch, victim, "downstrike", WEAR_WIELD);
			check_improve(ch, "downstrike", TRUE, 5);
		}
	} end_foreach (ch);
}

static bool
distance_check(CHAR_DATA *ch, CHAR_DATA *victim)
{
	OBJ_DATA * v_weapon;

	int chance;

	v_weapon = get_eq_char(victim, WEAR_WIELD);

	if (!v_weapon || !WEAPON_IS_LONG(v_weapon))
		return FALSE;

	if (!(chance = get_skill(victim, "distance")))
		return FALSE;

	chance += get_curr_stat(victim, STAT_DEX) - get_curr_stat(ch, STAT_DEX);

	if (number_percent() < chance / 4) {
		act("You fail to reach $N.", ch, NULL, victim, TO_CHAR);
		act("You stay out of $n's reach.", ch, NULL, victim, TO_VICT);
		check_improve(victim, "distance", TRUE, 5);
		return TRUE;
	}

	return FALSE;
}

DO_FUN(do_kill, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int chance;

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		act_char("Kill whom?", ch);
		return;
	}

	WAIT_STATE(ch, 1 * get_pulse("violence"));

	if ((victim = get_char_here(ch, arg)) == NULL) {
		act_char("They aren't here.", ch);
		return;
	}

	if (ch->position == POS_FIGHTING) {
		if (victim == ch->fighting)
			act_char("You do the best you can!", ch);
		else if (victim->fighting != ch)
			act_char("One battle at a time, please.", ch);
		else {
			act("You start aiming at $N.",ch,NULL,victim,TO_CHAR);
			ch->fighting = victim;
		}
		return;
	}

	if (!IS_NPC(victim)) {
		act_char("You must MURDER a player.", ch);
		return;
	}

	if (victim == ch) {
		act_char("You hit yourself.  Ouch!", ch);
		multi_hit(ch, ch, NULL);
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
			damage(ch, victim, victim->hit + 1,
			       "mortal strike", DAM_F_SHOW);
			check_improve(ch, "mortal strike", TRUE, 1);
			return;
		} else
			check_improve(ch, "mortal strike", FALSE, 3);
	}

	multi_hit(ch, victim, NULL);
}

DO_FUN(do_murder, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int chance;

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		act_char("Murder whom?", ch);
		return;
	}

	if (IS_AFFECTED(ch, AFF_CHARM)
	||  (IS_NPC(ch) && IS_SET(ch->pMobIndex->act, ACT_PET)))
		return;

	if ((victim = get_char_here(ch, arg)) == NULL) {
		WAIT_STATE(ch, MISSING_TARGET_DELAY);
		act_char("They aren't here.", ch);
		return;
	}

	if (victim == ch) {
		act_char("Suicide is a mortal sin.", ch);
		return;
	}

	if (IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim) {
		act("$N is your beloved master.", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (ch->position == POS_FIGHTING) {
		act_char("You do the best you can!", ch);
		return;
	}

	if (is_safe(ch, victim))
		return;

	WAIT_STATE(ch, 1 * get_pulse("violence"));

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
			       "mortal strike", DAM_F_SHOW);
			check_improve(ch, "mortal strike", TRUE, 1);
			return;
		} else
			check_improve(ch, "mortal strike", FALSE, 3);
	}

	multi_hit(ch, victim, NULL);
	yell(victim, ch, "Help! $lu{$N} is attacking me!");
}

DO_FUN(do_flee, ch, argument)
{
	ROOM_INDEX_DATA *was_in;
	ROOM_INDEX_DATA *now_in;
	CHAR_DATA *victim;
	int attempt;
	int num_attempts;

	if (RIDDEN(ch)) {
		act_char("You should ask to your rider!", ch);
		return;
	}

	if (MOUNTED(ch))
		dofun("dismount", ch, str_empty);

	if ((victim = ch->fighting) == NULL) {
		if (ch->position == POS_FIGHTING)
			ch->position = POS_STANDING;
		act_char("You aren't fighting anyone.", ch);
		return;
	}

	if (!can_flee(ch)) {
		 act_char("Your honour doesn't let you flee, try dishonoring yourself.", ch);
		 return;
	}

	was_in = ch->in_room;
	num_attempts = dice_wlb(3, 2, ch, NULL);
	for (attempt = 0; attempt < num_attempts; attempt++) {
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

		move_char(ch, door, 0);
		if ((now_in = ch->in_room) == was_in)
		    continue;

		ch->in_room = was_in;
		act("$n has fled!", ch, NULL, NULL, TO_ROOM);
		ch->in_room = now_in;

		if (!IS_NPC(ch)) {
			act_puts("You fled from combat!",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
			if (ch->level < LEVEL_HERO) {
				int xp = FLEE_EXP(ch);
				act_puts("You lose $j exp.",
					 ch, (const void *) xp, NULL,
					 TO_CHAR, POS_DEAD);
				gain_exp(ch, -xp);
			}
		} else {
			/* Once fled, the mob will not go after */
			NPC(ch)->last_fought = NULL;
		}

		stop_fighting(ch, TRUE);
		return;
	}
	WAIT_STATE(ch, get_pulse("violence"));
	act_char("PANIC! You couldn't escape!", ch);
}

DO_FUN(do_pound, ch, argument)
{
	CHAR_DATA *victim;
	OBJ_DATA *weapon;
	OBJ_DATA *second_weap;
	bool attack;
	char arg[MAX_INPUT_LENGTH];

	weapon = get_eq_char(ch, WEAR_WIELD);
	second_weap = get_eq_char(ch, WEAR_SECOND_WIELD);

	if (!weapon) {
		act("You need a weapon to do that.", ch, NULL, NULL, TO_CHAR);
		return;
	}

	if (!WEAPON_IS(weapon, WEAPON_MACE)
	&&  (!second_weap || !WEAPON_IS(second_weap, WEAPON_MACE))) {
		act("You need a mace to do that.", ch, NULL, NULL, TO_CHAR);
		return;
	}

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		victim = ch->fighting;
		if (victim == NULL) {
			act_char("But you aren't fighting anyone!", ch);
			return;
		}
	} else
		victim = get_char_here(ch, arg);

	if (!victim || victim->in_room != ch->in_room) {
		WAIT_STATE(ch, MISSING_TARGET_DELAY);
		act_char("They aren't here.", ch);
		return;
	}

	if (victim == ch) {
		act("You hit yourself with your mace. It really did hurt.",
			ch, NULL, NULL, TO_CHAR);
		act("$n hits $mself with $s mace.",
			ch, NULL, NULL, TO_ROOM);
		return;
	}

	WAIT_STATE(ch, skill_beats("pound"));

	if (is_safe(ch, victim))
		return;

	attack = (victim != ch->fighting) && (victim->fighting != ch);

	if (number_percent() > get_skill(ch, "pound")
	||  distance_check(ch, victim)) {
		damage(ch, victim, 0, "pound", DAM_F_SHOW);
		check_improve(ch, "pound", FALSE, 3);
	} else {
		int wear_loc;
		act("You swing at $N with your weapon.",
			ch, NULL, victim, TO_CHAR);
		act("$n swings at you with $s weapon.",
			ch, NULL, victim, TO_VICT);

		wear_loc = WEAPON_IS(weapon, WEAPON_MACE)? WEAR_WIELD : WEAR_SECOND_WIELD;

		one_hit(ch, victim, "pound", wear_loc);
		check_improve(ch, "pound", TRUE, 3);
	}

	if (attack)
		yell(victim, ch, "Help! $lu{$N} is attacking me!");
}

DO_FUN(do_entangle, ch, argument)
{
	CHAR_DATA *victim;
	int chance;
	OBJ_DATA *whip;

	if ((victim = ch->fighting) == NULL) {
		act_char("You aren't fighting anyone.", ch);
		return;
	}

	if (is_sn_affected(ch, "entanglement")
	||  is_sn_affected(victim, "entanglement"))
		return;

	if ((whip = get_eq_char(ch, WEAR_SECOND_WIELD)) == NULL
	||  !(WEAPON_IS(whip, WEAPON_WHIP) || WEAPON_IS(whip, WEAPON_FLAIL))) {
		act("You need to wield whip or flail in your off-hand.",
			ch, NULL, NULL, TO_CHAR);
		return;
	}

	chance = get_skill(ch, "entanglement");
	chance += get_curr_stat(ch, STAT_DEX) - get_curr_stat(victim, STAT_DEX);
	chance = chance * 2 / 3;

	WAIT_STATE(ch, skill_beats("entanglement"));

	if (number_percent() > chance) {
		AFFECT_DATA *paf;

		act("You entangle $N with your $p.",
			ch, whip, victim, TO_CHAR);
		act("$n entangles you with $s $p.",
			ch, whip, victim, TO_VICT);

		paf = aff_new(TO_AFFECTS, "entanglement");
		paf->level	= ch->level;
		paf->duration	= -1;
		paf->owner	= victim;
		affect_to_char(ch, paf);

		paf->owner	= ch;
		INT(paf->location)= APPLY_DEX;
		paf->modifier	= -5;
		affect_to_char(victim, paf);
		aff_free(paf);

		check_improve(ch, "entanglement", TRUE, 3);
	} else {
		act("You fail to entangle $N with your $p.",
			ch, whip, victim, TO_CHAR);
		act("$n tries to entangle you but fails.",
			ch, whip, victim, TO_VICT);
		check_improve(ch, "entanglement", FALSE, 3);
	}
}

static void
gash_drop(CHAR_DATA *ch, CHAR_DATA *victim, int loc)
{
	OBJ_DATA *item;

	if ((item = get_eq_char(victim, loc)) == NULL)
		return;

	if (IS_OBJ_STAT(item, ITEM_NOREMOVE)) {
		act("$N is unable to release $p and it pulls $S hand down.",
			ch, item, victim, TO_CHAR);
		act("You are unable to release $p and it pulls your hand down.",
			ch, item, victim, TO_VICT);
		act("$N is unable to release $p and it pulls $S hand down.",
			ch, item, victim, TO_NOTVICT);
	} else {
		act("$N drops $S $p, unable to hold it!",
			ch, item, victim, TO_CHAR);
		act("You drop your $p, unable to hold it!",
			ch, item, victim, TO_VICT);
		act("$N drops $S $p, unable to hold it!",
			ch, item, victim, TO_NOTVICT);

		if (IS_OBJ_STAT(item, ITEM_NODROP)
		||  IS_OBJ_STAT(item, ITEM_INVENTORY))
			obj_to_char(item, victim);
		else
			obj_to_room(item, victim->in_room);
	}
}

DO_FUN(do_gash, ch, argument)
{
	CHAR_DATA *victim;
	int chance, wear_loc;
	OBJ_DATA *weapon, *second_weap, *gauntlets, *dagger;
	bool attack;
	char arg[MAX_INPUT_LENGTH];

	weapon = get_eq_char(ch, WEAR_WIELD);
	second_weap = get_eq_char(ch, WEAR_SECOND_WIELD);
	if (weapon == NULL) {
		act("You need to wield a dagger first.",
		ch, NULL, NULL, TO_CHAR);
		return;
	}

	if (!WEAPON_IS(weapon, WEAPON_DAGGER)
	&&  (!second_weap || WEAPON_IS(second_weap, WEAPON_DAGGER))) {
		act("You need to wield a dagger first.",
		ch, NULL, NULL, TO_CHAR);
		return;
	}

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		if ((victim = ch->fighting) == NULL) {
			act("But you aren't fighting anyone!",
			ch, NULL, NULL, TO_CHAR);
			return;
		}
	} else {
		victim = get_char_here(ch, arg);
	}

	if (!victim || victim->in_room != ch->in_room) {
		act("They aren't here.", ch, NULL, NULL, TO_CHAR);
		WAIT_STATE(ch, MISSING_TARGET_DELAY);
		return;
	}

	if (victim->fighting == NULL) {
		act("They must be fighting.", ch, NULL, NULL, TO_CHAR);
		return;
	}

	if (victim == ch) {
		act("That would really hurt. Think twice more.",
		ch, NULL, NULL, TO_CHAR);
		return;
	}

	if (is_safe(ch, victim))
		return;

	if (is_sn_affected(victim, "crippled hands")) {
		act("$s hands are already crippled.",
		    ch, NULL, victim, TO_CHAR);
		return;
	}

	WAIT_STATE(ch, skill_beats("gash"));

	if (check_close(ch, victim))
		return;

	attack = (victim != ch->fighting) && (victim->fighting != ch);

	chance = get_skill(ch, "gash") / 2;
	chance += get_curr_stat(ch, STAT_DEX) - get_curr_stat(victim, STAT_DEX);
	chance += LEVEL(ch) - LEVEL(victim);

	wear_loc = WEAPON_IS(weapon, WEAPON_DAGGER) ? WEAR_WIELD : WEAR_SECOND_WIELD;
	dagger = WEAPON_IS(weapon, WEAPON_DAGGER) ? weapon : second_weap;

	if ((number_percent() > chance) || distance_check(ch, victim)) {
		act("$N notices your maneuver right in time. DOH!",
		ch, NULL, victim, TO_CHAR);
		act("You notice $n aiming $s weapon at your hands right in"
		"time to avoid the attack.", ch, NULL, victim, TO_VICT);
		check_improve(ch, "gash", FALSE, 3);
	} else {
		AFFECT_DATA *paf;

		gauntlets = get_eq_char(victim, WEAR_HANDS);
		if(gauntlets) {
			if((gauntlets->item_type != ITEM_ARMOR)
			|| !(material_is(gauntlets, MATERIAL_METAL)
			|| material_is(gauntlets, MATERIAL_INDESTRUCT))) {
				damage_to_obj(victim, dagger, gauntlets,
					number_range(30, 40));
			} else {
				switch(number_bits(1)) {
				case 0:
					act("You only damage your dagger "
					"against $N's $p.",
					ch, gauntlets, victim, TO_CHAR);
					act("$n's dagger slides off your $p.",
					ch, gauntlets, victim, TO_VICT);
					damage_to_obj(victim, gauntlets, dagger,
						number_range(30, 40));
					break;
				case 1:
					act("Your dagger only scratches $N's $p.",
					    ch, gauntlets, victim, TO_CHAR);
					act("$n's dagger leaves a few scratches on your $p.",
					    ch, gauntlets, victim, TO_VICT);
					damage_to_obj(victim, dagger, gauntlets,
						number_range(30, 40));
					break;
				}
				check_improve(ch, "gash", TRUE, 3);
				return;
			}
		}

		gash_drop(ch, victim, WEAR_WIELD);
		gash_drop(ch, victim, WEAR_SECOND_WIELD);
		gash_drop(ch, victim, WEAR_HOLD);

		act("{RYour gash at $N's hands, crippling it ruthlessly!{x",
			ch, NULL, victim, TO_CHAR);
		act("{R$n's dagger strikes for your hands, producing a "
			"terrible wound!{x", ch, NULL, victim, TO_VICT);
		act("{R$n gashes at $N's hands, producing a bleeding wound!{x",
			ch, NULL, victim, TO_ROOM);

		paf = aff_new(TO_AFFECTS, "crippled hands");
		paf->level = ch->level;
		INT(paf->location) = APPLY_HITROLL;
		paf->duration = ch->level/15;
		paf->modifier = -ch->level/5;
		affect_to_char(victim, paf);
		aff_free(paf);

		one_hit(ch, victim, "gash", wear_loc);
		check_improve(ch, "gash", TRUE, 3);
	}

	if (attack)
		yell(victim, ch, "Help! $lu{$N} is attacking me!");
}

DO_FUN(do_cut, ch, argument)
{
	CHAR_DATA *victim;
	int chance;
	OBJ_DATA *weapon;
	OBJ_DATA *second_weap;
	bool attack;
	char arg[MAX_INPUT_LENGTH];

	weapon = get_eq_char(ch, WEAR_WIELD);
	second_weap = get_eq_char(ch, WEAR_SECOND_WIELD);
	if (weapon == NULL) {
		act("You need a weapon to do that.", ch, NULL, NULL, TO_CHAR);
		return;
	}

	if (!WEAPON_IS(weapon, WEAPON_SWORD)
	&&  (!second_weap || !WEAPON_IS(second_weap, WEAPON_SWORD))) {
		act("You need a sword to do that.", ch, NULL, NULL, TO_CHAR);
		return;
	}

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		victim = ch->fighting;
		if (victim == NULL) {
			act_char("But you aren't fighting anyone!", ch);
			return;
		}
	} else
		victim = get_char_here(ch, arg);

	if (victim == NULL || victim->in_room != ch->in_room) {
		WAIT_STATE(ch, MISSING_TARGET_DELAY);
		act_char("They aren't here.", ch);
		return;
	}

	if (victim == ch) {
		act("You can't do that to yourself.",
			ch, NULL, NULL, TO_CHAR);
		return;
	}

	WAIT_STATE(ch, skill_beats("cut"));

	if (is_safe(ch, victim))
		return;

	attack = (victim != ch->fighting) && (victim->fighting != ch);

	chance = get_skill(ch, "cut");
	if (WEAPON_IS(weapon, WEAPON_SWORD)) {
		if ((number_percent() > chance) || distance_check(ch, victim)) {
			damage(ch, victim, 0, "cut", DAM_F_SHOW);
			check_improve(ch, "cut", FALSE, 3);
		} else {
			act("You attempt to cut $N with your weapon.",
				ch, NULL, victim, TO_CHAR);
			act("$n attempts to cut you with $s weapon.",
				ch, NULL, victim, TO_VICT);

			one_hit(ch, victim, "cut", WEAR_WIELD);
			check_improve(ch, "cut",TRUE, 3);
		}
	}

	if (IS_EXTRACTED(victim)
	||  !victim->in_room
	||  !ch->in_room
	||  victim->in_room != ch->in_room)
		return;

	if (second_weap && WEAPON_IS(second_weap, WEAPON_SWORD)) {
		if ((number_percent() > chance) || distance_check(ch, victim)) {
			damage(ch, victim, 0, "cut", DAM_F_SHOW);
			check_improve(ch, "cut", FALSE, 3);
		} else {
			act("You attempt to cut $N with your weapon.",
				ch, NULL, victim, TO_CHAR);
			act("$n attempts to cut you with $s weapon.",
				ch, NULL, victim, TO_VICT);

			one_hit(ch, victim, "cut", WEAR_SECOND_WIELD);
			check_improve(ch, "cut", TRUE, 3);
		}
	}

	if (attack)
		yell(victim, ch, "Help! $lu{$N} is attacking me!");
}

DO_FUN(do_hunger, ch, argument)
{
	AFFECT_DATA *paf;

	paf = aff_new(TO_FORMAFFECTS, "hungry rat");
	paf->level	= LEVEL(ch);
	paf->duration	= number_fuzzy(ch->level / 8);
	paf->modifier	= UMAX(1, LEVEL(ch)/5);
	INT(paf->location)= APPLY_HITROLL;
	affect_to_char(ch, paf);

	INT(paf->location)= APPLY_DAMROLL;
	paf->bitvector	= AFF_BERSERK;
	affect_to_char(ch, paf);
	aff_free(paf);

	act("You feel so hungry now.", ch, NULL, NULL, TO_CHAR);
}

DO_FUN(do_whirl, ch, argument)
{
	CHAR_DATA *victim;
	OBJ_DATA *weapon;
	OBJ_DATA *second_weap;
	bool attack;
	char arg[MAX_INPUT_LENGTH];

	weapon = get_eq_char(ch, WEAR_WIELD);
	second_weap = get_eq_char(ch, WEAR_SECOND_WIELD);
	if (weapon == NULL) {
		act("You need a weapon to do that.", ch, NULL, NULL, TO_CHAR);
		return;
	}

	if (!WEAPON_IS(weapon, WEAPON_AXE)
	&&  (!second_weap || !WEAPON_IS(second_weap, WEAPON_AXE))) {
		act("You need a axe to do that.", ch, NULL, NULL, TO_CHAR);
		return;
	}

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		victim = ch->fighting;
		if (victim == NULL) {
			act_char("But you aren't fighting anyone!", ch);
			return;
		}
	} else
		victim = get_char_here(ch, arg);

	if (victim == NULL || victim->in_room != ch->in_room) {
		WAIT_STATE(ch, MISSING_TARGET_DELAY);
		act_char("They aren't here.", ch);
		return;
	}

	if (victim == ch) {
		act("You can't do that to yourself.",
			ch, NULL, NULL, TO_CHAR);
		return;
	}

	WAIT_STATE(ch, skill_beats("whirl"));

	if (is_safe(ch, victim))
		return;

	attack = (victim != ch->fighting) && (victim->fighting != ch);

	if (number_percent() > get_skill(ch, "whirl")
	||  distance_check(ch, victim)) {
		damage(ch, victim, 0, "whirl", DAM_F_SHOW);
		check_improve(ch, "whirl", FALSE, 3);
	} else {
		int wear_loc;
		wear_loc = WEAPON_IS(weapon, WEAPON_AXE)? WEAR_WIELD : WEAR_SECOND_WIELD;

		one_hit(ch, victim, "whirl", wear_loc);
	}

	if (attack)
		yell(victim, ch, "Help! $lu{$N} is attacking me!");
}

DO_FUN(do_feint, ch, argument)
{
	CHAR_DATA *victim;
	int chance;
	OBJ_DATA *weapon;

	if ((weapon = get_eq_char(ch, WEAR_WIELD)) == NULL
	||  !WEAPON_IS(weapon, WEAPON_SWORD)) {
		act("You need to wield a sword.", ch, NULL, NULL, TO_CHAR);
		return;
	}

	if ((victim = ch->fighting) == NULL) {
		act("You aren't fighting anyone.", ch, NULL, NULL, TO_CHAR);
		return;
	}

	WAIT_STATE(ch, skill_beats("feint"));

	chance = get_skill(ch, "feint");
	chance += get_curr_stat(ch, STAT_DEX);
	chance -= get_curr_stat(victim, STAT_INT);
	chance = chance * 2 / 3;

	if (number_percent() < chance) {
		switch(number_bits(1)) {
		case 0:
			act("You press at $N's defences and manage to slip in "
				"another attack!", ch, NULL, victim, TO_CHAR);
			act("$n maneuvers dextrously and you miss an incoming "
				"attack!", ch, NULL, victim, TO_VICT);
			multi_hit(ch, victim, NULL);
			break;
		case 1:
			act("You dazzle $N with your artful fencing.",
				ch, NULL, victim, TO_CHAR);
			act("You are unable to follow $n's maneuvers.",
				ch, NULL, victim, TO_VICT);
			SET_BIT(victim->affected_by, AFF_WEAK_STUN);
			break;
		}
		check_improve(ch, "feint", TRUE, 1);
	} else {
		act("You fail to achieve anything with your fencing.",
			ch, NULL, NULL, TO_CHAR);
		act("$n tries to trick you with $s feint, but you ignore it.",
			ch, NULL, NULL, TO_VICT);
		check_improve(ch, "feint", FALSE, 4);
	}
}

/*
 * Disarm a creature.
 * Caller must check for successful attack.
 */
static void
disarm(CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj)
{
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
			act("$N grips and prevents you to disarm $M!",
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

	if (IS_OBJ_STAT(obj, ITEM_NODROP)
	||  IS_OBJ_STAT(obj, ITEM_INVENTORY)
	||  IS_SET(victim->in_room->room_flags, ROOM_BATTLE_ARENA))
		obj_to_char(obj, victim);
	else {
		obj_to_room(obj, victim->in_room);
		if (IS_NPC(victim)
		&&  victim->wait == 0
		&&  can_see_obj(victim, obj))
			get_obj(victim, obj, NULL, NULL);
	}
}

DO_FUN(do_berserk, ch, argument)
{
	int hp_percent;
	int mana;
	int chance;

	if (IS_AFFECTED(ch, AFF_BERSERK)
	||  is_sn_affected(ch, "berserk")
	||  is_sn_affected(ch, "frenzy")) {
		act_char("You get a little madder.", ch);
		return;
	}

	if (IS_AFFECTED(ch, AFF_CALM)) {
		act_char("You're feeling too mellow to berserk.", ch);
		return;
	}

	mana = skill_mana(ch, "berserk");
	if (ch->mana < mana) {
		act_char("You can't get up enough energy.", ch);
		return;
	}

	/* modifiers */

	/* fighting */
	chance = get_skill(ch, "berkserk");
	if (ch->position == POS_FIGHTING)
		chance += 10;

	/* damage -- below 50% of hp helps, above hurts */
	hp_percent = 100 * ch->hit/ch->max_hit;
	chance += 25 - hp_percent/2;

	if (number_percent() < chance) {
		AFFECT_DATA *paf;

		WAIT_STATE(ch, get_pulse("violence"));
		ch->mana -= mana;
		ch->move /= 2;

		/* heal a little damage */
		ch->hit += LEVEL(ch) * 2;
		ch->hit = UMIN(ch->hit,ch->max_hit);

		act_char("Your pulse races as you are consumned by rage!", ch);
		act_puts("$n gets a wild look in $s eyes.",
			 ch, NULL, NULL, TO_ROOM, POS_FIGHTING);
		check_improve(ch, "berserk", TRUE, 2);

		paf = aff_new(TO_AFFECTS, "berserk");
		paf->level	= ch->level;
		paf->duration	= number_fuzzy(ch->level / 8);

		INT(paf->location)= APPLY_HITROLL;
		paf->modifier	= UMAX(1, LEVEL(ch)/5);
		affect_to_char(ch, paf);

		INT(paf->location)= APPLY_DAMROLL;
		affect_to_char(ch, paf);

		INT(paf->location)= APPLY_AC;
		paf->modifier	= UMAX(10, 10 * (LEVEL(ch)/5));
		paf->bitvector	= AFF_BERSERK;
		affect_to_char(ch, paf);
		aff_free(paf);
	} else {
		WAIT_STATE(ch,2 * get_pulse("violence"));
		ch->mana -= mana / 2;
		ch->move /= 2;

		act_char("Your pulse speeds up, but nothing happens.", ch);
		check_improve(ch, "berserk", FALSE, 2);
	}
}

DO_FUN(do_breath, ch, argument)
{
	CHAR_DATA *victim;
	const char *spell;
	int mana;

	if ((victim = ch->fighting) == NULL) {
		act_char("You aren't fighting anyone.", ch);
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

	if (number_percent() > get_skill(ch, "breath")) {
		act_char("You failed.", ch);
		ch->mana -= mana/2;
		check_improve(ch, "breath", FALSE, 1);
		return;
	}
	ch->mana -= mana;
	spellfun(spell, NULL, LEVEL(ch), ch, victim);
	WAIT_STATE(ch, skill_beats("breath"));
}

DO_FUN(do_bash, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int chance, wait;
	bool attack = FALSE;
	int damage_bash;

	if (MOUNTED(ch)) {
		act_char("You can't bash while riding!", ch);
		return;
	}

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] != '\0' && !str_cmp(arg, "door")) {
		do_bash_door(ch, argument);
		return;
	}

	if (arg[0] == '\0') {
		victim = ch->fighting;
		if (victim == NULL) {
			act_char("But you aren't fighting anyone!", ch);
			return;
		}
	} else
		victim = get_char_here(ch, arg);

	if (victim == NULL || victim->in_room != ch->in_room) {
		WAIT_STATE(ch, MISSING_TARGET_DELAY);
		act_char("They aren't here.", ch);
		return;
	}

	WAIT_STATE(ch, skill_beats("bash"));

	if (victim->position < POS_FIGHTING) {
		act("You'll have to let $M get back up first.",
			  ch, NULL, victim, TO_CHAR);
		return;
	}

	if (victim == ch) {
		act_char("You try to bash your brains out, but fail.", ch);
		return;
	}

	if (MOUNTED(victim)) {
		act_char("You can't bash a riding one!", ch);
		return;
	}

	if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim) {
		act("But $N is your friend!", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (is_sn_affected(victim, "protective shield")) {
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
	chance = get_skill(ch, "bash");
	chance += get_carry_weight(ch) / 25;
	chance -= get_carry_weight(victim) / 20;

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

	if (check_close(ch, victim)
	|| distance_check(ch, victim))
		return;

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

		WAIT_STATE(victim, wait * get_pulse("violence"));
		WAIT_STATE(ch, skill_beats("bash"));
		victim->position = POS_RESTING;
		damage_bash = (ch->damroll / 2) +
			      number_range(4, 4 + 4* ch->size + chance/10);
		damage(ch, victim, damage_bash, "bash", DAM_F_SHOW);
		check_downstrike(victim);
	} else {
		damage(ch, victim, 0, "bash", DAM_F_SHOW);
		act_puts("You fall flat on your face!",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		act("$n falls flat on $s face.",
		    ch, NULL, victim, TO_NOTVICT);
		act("You evade $n's bash, causing $m to fall flat on $s face.",
		    ch, NULL, victim, TO_VICT);
		check_improve(ch, "bash", FALSE, 1);
		ch->position = POS_RESTING;
		WAIT_STATE(ch, skill_beats("bash") * 3/2);
		check_downstrike(ch);
	}

	if (attack)
		yell(victim, ch, "Help! $lu{$N} is bashing me!");
}

DO_FUN(do_dirt, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	bool attack;
	int chance;

	if (MOUNTED(ch)) {
		act_char("You can't dirt while riding!", ch);
		return;
	}

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		victim = ch->fighting;
		if (victim == NULL) {
			act_char("But you aren't in combat!", ch);
			return;
		}
	} else
		victim = get_char_here(ch, arg);

	if (victim == NULL || victim->in_room != ch->in_room) {
		WAIT_STATE(ch, MISSING_TARGET_DELAY);
		act_char("They aren't here.", ch);
		return;
	}

	WAIT_STATE(ch, skill_beats("dirt kicking"));

	if (IS_AFFECTED(ch, AFF_FLYING)) {
		 act_char("While flying?", ch);
		 return;
	}

	if (IS_AFFECTED(victim, AFF_BLIND)) {
		act("$gN{He}'s already been blinded.",
		    ch, NULL, victim, TO_CHAR);
		return;
	}

	if (victim == ch) {
		act_char("Very funny.", ch);
		return;
	}

	if (MOUNTED(victim)) {
		act_char("You can't dirt a riding one!", ch);
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
	chance = get_skill(ch, "dirt kicking");
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
	case(SECT_AIR):			chance  =  0;	break;
	case(SECT_SWAMP):		chance  =  0;	break;
	case(SECT_DESERT):		chance += 10;   break;
	}

	if (chance == 0) {
		act_char("There isn't any dirt to kick.", ch);
		return;
	}

	attack = (ch->fighting != victim);

	/* now the attack */
	if (number_percent() < chance) {
		AFFECT_DATA *paf;

		act("$n is blinded by the dirt in $s eyes!",
		    victim, NULL, NULL, TO_ROOM);
		act_char("You can't see a thing!", victim);
		check_improve(ch, "dirt kicking", TRUE, 2);

		paf = aff_new(TO_AFFECTS, "dirt kicking");
		paf->level	= ch->level;
		INT(paf->location)= APPLY_HITROLL;
		paf->modifier	= -4;
		paf->bitvector	= AFF_BLIND;
		affect_to_char(victim, paf);
		aff_free(paf);

		damage(ch, victim, number_range(2, 5),
		       "dirt kicking", DAM_F_SHOW);
	} else {
		damage(ch, victim, 0, "dirt kicking", DAM_F_SHOW);
		check_improve(ch, "dirt kicking", FALSE, 2);
	}

	if (attack)
		yell(victim, ch, "Help! $lu{$N} just kicked dirt into my eyes!");
}

DO_FUN(do_trip, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	bool attack;
	int chance;
	int beats;

	if (MOUNTED(ch)) {
		act_char("You can't trip while riding!", ch);
		return;
	}

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		victim = ch->fighting;
		if (victim == NULL) {
			act_char("But you aren't fighting anyone!", ch);
			return;
		}
	} else
		victim = get_char_here(ch, arg);

	if (victim == NULL || victim->in_room != ch->in_room) {
		WAIT_STATE(ch, MISSING_TARGET_DELAY);
		act_char("They aren't here.", ch);
		return;
	}

	if (MOUNTED(victim)) {
		act_char("You can't trip a riding one!", ch);
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
		check_downstrike(ch);
		return;
	}

	if (IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim) {
		act("$N is your beloved master.", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (is_safe(ch, victim))
		return;

	/* modifiers */
	chance = get_skill(ch, "trip");

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

	if (check_close(ch, victim)
	||  distance_check(ch, victim))
		return;

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
		       "trip", DAM_F_SHOW);
		check_downstrike(victim);
	} else {
		damage(ch, victim, 0, "trip", DAM_F_SHOW);
		WAIT_STATE(ch, beats * 2 / 3);
		check_improve(ch, "trip", FALSE, 1);
		check_downstrike(ch);
	}

	if (attack)
		yell(victim, ch, "Help! $lu{$N} just tripped me!");
}

DO_FUN(do_backstab, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	if (!can_backstab(ch))
		return;

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		act_char("Backstab whom?", ch);
		return;
	}

	if ((victim = get_char_here(ch, arg)) == NULL) {
		WAIT_STATE(ch, MISSING_TARGET_DELAY);
		act_char("They aren't here.", ch);
		return;
	}

	backstab_char(ch, victim);
}

DO_FUN(do_knife, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA *knife;
	int chance;

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		act_char("Knife whom?", ch);
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

	if ((victim = get_char_here(ch, arg)) == NULL) {
		act_char("They aren't here.", ch);
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

	if (number_percent() < chance) {
		one_hit(ch, victim, "knife", WEAR_WIELD);
	} else {
		damage(ch, victim, 0, "knife", DAM_F_SHOW);
		check_improve(ch, "knife", FALSE, 1);
	}
	yell(victim, ch, "Die, $N! You're backstabbing scum!");
}

DO_FUN(do_cleave, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA *obj;

	if (MOUNTED(ch)) {
		act_char("You can't cleave while riding!", ch);
		return;
	}

	if (ch->master != NULL && IS_NPC(ch))
		return;

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		act_char("Cleave whom?", ch);
		return;
	}

	if ((victim = get_char_here(ch, arg)) == NULL) {
		WAIT_STATE(ch, MISSING_TARGET_DELAY);
		act_char("They aren't here.", ch);
		return;
	}

	if (victim == ch) {
		act_char("How can you sneak up on yourself?", ch);
		return;
	}

	if ((obj = get_eq_char(ch, WEAR_WIELD)) == NULL) {
		act_char("You need to wield a weapon to cleave.", ch);
		return;
	}

	if (!WEAPON_IS(obj, WEAPON_AXE)
	&&  !WEAPON_IS(obj, WEAPON_SWORD)) {
		act_char("You must wield axe or sword to cleave.", ch);
		return;
	}

	if (victim->fighting != NULL) {
		act_char("You can't cleave a fighting person.", ch);
		return;
	}

	if ((victim->hit < victim->max_hit * 9 / 10)
	&&  IS_AWAKE(victim)) {
		act("$N is hurt and suspicious ... you can't sneak up.",
		    ch, NULL, victim, TO_CHAR);
		return;
	}

	if (is_safe(ch, victim))
		return;

	WAIT_STATE(ch, skill_beats("cleave"));

	if (!IS_AWAKE(victim)
	||  IS_NPC(ch)
	||  number_percent() < get_skill(ch, "cleave")) {
		one_hit(ch, victim, "cleave", WEAR_WIELD);
	} else {
		check_improve(ch, "cleave", FALSE, 1);
		damage(ch, victim, 0, "cleave", DAM_F_SHOW);
	}
	yell(victim, ch, "Die, $N, you butchering fool!");
}

DO_FUN(do_impale, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA *obj;

	if (ch->master != NULL && IS_NPC(ch))
		return;

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		act_char("Impale whom?", ch);
		return;
	}

	if ((victim = get_char_here(ch, arg)) == NULL) {
		WAIT_STATE(ch, MISSING_TARGET_DELAY);
		act_char("They aren't here.", ch);
		return;
	}

	if (victim == ch) {
		act_char("How can you sneak up on yourself?", ch);
		return;
	}

	if ((obj = get_eq_char(ch, WEAR_WIELD)) == NULL) {
		act_char("You need to wield a weapon to impale.", ch);
		return;
	}

	if (!WEAPON_IS(obj, WEAPON_SPEAR)
	&&  !WEAPON_IS(obj, WEAPON_LANCE)) {
		act_char("You must wield spear or lance to impale.", ch);
		return;
	}

	if (victim->fighting != NULL) {
		act("$N is moving to quickly.", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (victim->hit < victim->max_hit * 9 / 10
	&&  IS_AWAKE(victim)) {
		act("$N is hurt and suspicious ... you can't sneak up.",
		    ch, NULL, victim, TO_CHAR);
		return;
	}

	if (is_safe(ch, victim))
		return;

	WAIT_STATE(ch, skill_beats("impale"));

	if (!IS_AWAKE(victim)
	||  IS_NPC(ch)
	||  number_percent() < get_skill(ch, "impale")) {
		one_hit(ch, victim, "impale", WEAR_WIELD);
	} else {
		check_improve(ch, "impale", FALSE, 1);
		damage(ch, victim, 0, "impale", DAM_F_SHOW);
	}
	yell(victim, ch, "Help! $lu{$N} just tried to impale me!");
}

DO_FUN(do_ambush, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	if (MOUNTED(ch)) {
		act_char("You can't ambush while riding!", ch);
		return;
	}

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		act_char("Ambush whom?", ch);
		return;
	}

	WAIT_STATE(ch, skill_beats("ambush"));

	if ((victim = get_char_here(ch, arg)) == NULL) {
		WAIT_STATE(ch, MISSING_TARGET_DELAY);
		act_char("They aren't here.", ch);
		return;
	}

	if (victim == ch) {
		act_char("How can you ambush yourself?", ch);
		return;
	}

	if (can_see(victim, ch)) {
		act_char("But they can see you.", ch);
		return;
	}

	if (!HAS_INVIS(ch, ID_CAMOUFLAGE)) {
		dofun("camouflage", ch, str_empty);
		if (!HAS_INVIS(ch, ID_CAMOUFLAGE)) {
			act_char("You failed.", ch);
			return;
		}
	}

	if (is_safe(ch, victim))
		return;

	if (!IS_AWAKE(victim)
	||  IS_NPC(ch)
	||  number_percent() < get_skill(ch, "ambush")) {
		check_improve(ch, "ambush", TRUE, 1);
		one_hit(ch, victim, "ambush", WEAR_WIELD);
	} else {
		check_improve(ch, "ambush", FALSE, 1);
		act("Your attack was not so much surprising.",
		    ch, NULL, NULL, TO_CHAR);
		damage(ch, victim, 0, "ambush", DAM_F_SHOW);
	}
	yell(victim, ch, "Help! I've been ambushed by $N!");
}

DO_FUN(do_rescue, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	CHAR_DATA *fch;

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		act_char("Rescue whom?", ch);
		return;
	}

	WAIT_STATE(ch, skill_beats("rescue"));

	if ((victim = get_char_here(ch, arg)) == NULL) {
		act_char("They aren't here.", ch);
		return;
	}

	if (victim == ch) {
		act_char("What about fleeing instead?", ch);
		return;
	}

	if (!IS_NPC(ch) && IS_NPC(victim)) {
		act_char("Doesn't need your help!", ch);
		return;
	}

	if (ch->fighting == victim) {
		act_char("Too late.", ch);
		return;
	}

	if ((fch = victim->fighting) == NULL) {
		act_char("That person is not fighting right now.", ch);
		return;
	}

	if (IS_NPC(ch) && ch->master != NULL && IS_NPC(victim))
		return;

	if (ch->master != NULL && is_safe(ch->master, fch))
		return;

	if (is_safe(ch, fch))
		return;

	if (number_percent() >= get_skill(ch, "rescue")) {
		act_char("You fail the rescue.", ch);
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

DO_FUN(do_kick, ch, argument)
{
	CHAR_DATA *victim;
	int kick_dam;
	int chance;

	if (MOUNTED(ch)) {
		act_char("You can't kick while riding!", ch);
		return;
	}

	if ((victim = ch->fighting) == NULL) {
		act_char("You aren't fighting anyone.", ch);
		return;
	}

	chance = get_skill(ch, "kick");
	if (IS_AFFECTED(ch, AFF_FLYING))
		chance = chance * 110 / 100;

	if (check_close(ch, victim)
	|| distance_check(ch, victim))
		return;

	WAIT_STATE(ch, skill_beats("kick"));
	if (IS_NPC(ch) || number_percent() < chance) {
		kick_dam = number_range(1, LEVEL(ch));
		if (has_spec(ch, "class_samurai")		// notrans
		&&  (get_eq_char(ch, WEAR_FEET) == NULL))
			kick_dam *= 2;
		kick_dam += ch->damroll / 2;
		damage(ch, victim, kick_dam, "kick", DAM_F_SHOW);
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
                                damage(ch, victim, kick_dam,
				       "kick", DAM_F_SHOW);
                                chance /= 5;
                             }
			check_improve(ch, "follow through", TRUE, 6);
		}
	} else {
		damage(ch, victim, 0, "kick", DAM_F_SHOW);
		check_improve(ch, "kick", FALSE, 1);
	}
}

DO_FUN(do_circle, ch, argument)
{
	CHAR_DATA *victim;
	CHAR_DATA *rch;
	OBJ_DATA *obj;

	if (MOUNTED(ch)) {
		act_char("You can't circle while riding!", ch);
		return;
	}

	if ((victim = ch->fighting) == NULL) {
		act_char("You aren't fighting anyone.", ch);
		return;
	}

	if ((obj = get_eq_char(ch, WEAR_WIELD)) == NULL
	||  skill_damclass(obj->value[3].s) != DAM_PIERCE) {
		 act_char("You must wield a piercing weapon to circle stab.", ch);
		 return;
	}

	if (is_safe(ch, victim))
		return;

	WAIT_STATE(ch, skill_beats("circle"));

	for (rch = ch->in_room->people; rch; rch = rch->next_in_room) {
		if (rch->fighting == ch) {
			act_char("You can't circle while defending yourself.", ch);
			return;
		}
	}

	if (number_percent() < get_skill(ch, "circle")) {
		one_hit(ch, victim, "circle", WEAR_WIELD);
	} else {
		damage(ch, victim, 0, "circle", DAM_F_SHOW);
		check_improve(ch, "circle", FALSE, 1);
	}
}

DO_FUN(do_disarm, ch, argument)
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

	if ((wield = get_eq_char(ch, WEAR_WIELD)) == NULL
	&&  (hth = get_skill(ch, "hand to hand")) == 0) {
		act_char("You must wield a weapon to disarm.", ch);
		return;
	}

	if ((victim = ch->fighting) == NULL) {
		act_char("You aren't fighting anyone.", ch);
		return;
	}

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] && !str_prefix(arg, "second"))
		loc = WEAR_SECOND_WIELD;

	if ((vwield = get_eq_char(victim, loc)) == NULL) {
		act_char("Your opponent is not wielding a weapon.", ch);
		return;
	}

	/* find weapon skills */
	ch_weapon = get_weapon_skill(ch, get_weapon_sn(wield));
	vict_weapon = get_weapon_skill(victim, get_weapon_sn(vwield));

	/* modifiers */

	/* skill */
	chance = get_skill(ch, "disarm");
	if (wield == NULL)
		chance = chance * hth / 150;
	else
		chance = chance * ch_weapon / 100;

	chance += (ch_weapon / 2 - vict_weapon) / 2;

	if (wield && WEAPON_IS_LONG(wield))
		chance += 30;
	if (WEAPON_IS_LONG(vwield))
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
	} else {
		act("You fail to disarm $N.", ch, NULL, victim, TO_CHAR);
		act("$n tries to disarm you, but fails.",
		    ch, NULL, victim, TO_VICT);
		act("$n tries to disarm $N, but fails.",
		    ch, NULL, victim, TO_NOTVICT);
		check_improve(ch, "disarm", FALSE, 1);
	}
}

DO_FUN(do_strip, ch, argument)
{
	CHAR_DATA *victim;
	OBJ_DATA *wield;
	OBJ_DATA *vwield;
	int chance, ch_weapon, vict_weapon;
	int loc = WEAR_WIELD;
	char arg[MAX_INPUT_LENGTH];

	if (ch->master != NULL && IS_NPC(ch))
		return;

	if ((wield = get_eq_char(ch, WEAR_WIELD)) == NULL
	||  !(WEAPON_IS(wield, WEAPON_WHIP) || WEAPON_IS(wield, WEAPON_FLAIL))) {
		act_char("You must wield a whip or flail to strip enemy weapon.", ch);
		return;
	}

	if ((victim = ch->fighting) == NULL) {
		act_char("You aren't fighting anyone.", ch);
		return;
	}

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] && !str_prefix(arg, "second"))
		loc = WEAR_SECOND_WIELD;

	if ((vwield = get_eq_char(victim, loc)) == NULL) {
		act_char("Your opponent is not wielding a weapon.", ch);
		return;
	}

	/* find weapon skills */
	ch_weapon = get_weapon_skill(ch, get_weapon_sn(wield));
	vict_weapon = get_weapon_skill(victim, get_weapon_sn(vwield));

	/* modifiers */
	chance = get_skill(ch, "weapon strip");

	/* skill */
	chance = chance * ch_weapon/100;

	chance += (ch_weapon/2 - vict_weapon) / 2;

	if (WEAPON_IS_LONG(vwield))
		chance += 30;

	/* dex vs. strength */
	chance += get_curr_stat(ch,STAT_DEX);
	chance -= 2 * get_curr_stat(victim,STAT_STR);

	/* level */
	chance += (LEVEL(ch) - LEVEL(victim)) * 2;

	/* and now the attack */
	WAIT_STATE(ch, skill_beats("weapon strip"));
	if (number_percent() < chance) {
		if (IS_OBJ_STAT(vwield, ITEM_NOREMOVE)) {
			act("$S weapon won't budge!",
				ch, NULL, victim, TO_CHAR);
			act("$n tries to disarm you, but your weapon won't budge!",
				ch, NULL, victim, TO_VICT);
			act("$n tries to disarm $N, but fails.",
				ch, NULL, victim, TO_NOTVICT);
			return;
		}

		act_puts("$n DISARMS you!",
			ch, NULL, victim, TO_VICT, POS_FIGHTING);
		act_puts("You disarm $N!",
			ch,NULL, victim, TO_CHAR, POS_FIGHTING);
		act_puts("$n disarms $N!",
			ch, NULL, victim, TO_NOTVICT, POS_FIGHTING);

		if (IS_OBJ_STAT(vwield, ITEM_NODROP))
			obj_to_char(vwield, victim);
		else
			obj_to_char(vwield, ch);

		check_improve(ch, "weapon strip", TRUE, 1);
	} else {
		act("You fail to disarm $N.", ch, NULL, victim, TO_CHAR);
		act("$n tries to disarm you, but fails.",
		    ch, NULL, victim, TO_VICT);
		act("$n tries to disarm $N, but fails.",
		    ch, NULL, victim, TO_NOTVICT);
		check_improve(ch, "weapon strip", FALSE, 1);
	}
}

DO_FUN(do_nerve, ch, argument)
{
	CHAR_DATA *victim;
	char arg[MAX_INPUT_LENGTH];
	bool attack;

	if (MOUNTED(ch)) {
		act_char("You can't nerve while riding!", ch);
		return;
	}

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] =='\0') {
		victim = ch->fighting;
		if (victim == NULL) {
			act_char("You aren't fighting anyone.", ch);
			return;
		}
	} else if ((victim = get_char_here(ch, arg)) == NULL) {
		act_char("They aren't here.", ch);
		return;
	}

	if (is_sn_affected(victim, "nerve")) {
		act_char("You cannot weaken that character any more.", ch);
		return;
	}

	if (is_safe(ch, victim))
		return;

	WAIT_STATE(ch, skill_beats("nerve"));

	attack = (ch->fighting != victim);

	if (check_close(ch, victim)
	|| distance_check(ch, victim))
		return;

	if (IS_NPC(ch)
	||  number_percent() < (get_skill(ch, "nerve") + LEVEL(ch) - LEVEL(victim) -
	    get_curr_stat(victim, STAT_CON) + get_curr_stat(ch, STAT_DEX) / 2)
	||  ch == victim) {
		AFFECT_DATA *paf;

		paf = aff_new(TO_AFFECTS, "nerve");
		paf->level	= ch->level;
		paf->duration	= LEVEL(ch) * get_pulse("violence") / UMAX(get_pulse("char"), 1);
		INT(paf->location)= APPLY_STR;
		paf->modifier	= -3;
		affect_to_char(victim, paf);
		aff_free(paf);

		if (ch == victim) {
			act_char("You can't resist your own nerve pressure.",
			    ch);
			return;
		}

		act("You weaken $N with your nerve pressure.",
		    ch, NULL, victim, TO_CHAR);
		act("$n weakens you with $s nerve pressure.",
		    ch, NULL, victim, TO_VICT);
		act("$n weakens $N with $s nerve pressure.",
		    ch, NULL, victim, TO_NOTVICT);
		check_improve(ch, "nerve", TRUE, 1);
	} else {
		act_char("You press the wrong points and fail.", ch);
		act("$n tries to weaken you with nerve pressure, but fails.",
		    ch, NULL, victim, TO_VICT);
		act("$n tries to weaken $N with nerve pressure, but fails.",
		    ch, NULL, victim, TO_NOTVICT);
		check_improve(ch, "nerve", FALSE, 1);
	}
	if (attack)
		yell(victim, ch, "Help! $lu{$N} is attacking me!");
	multi_hit(victim,ch,NULL);
}

DO_FUN(do_endure, ch, argument)
{
	AFFECT_DATA *paf;

	if (IS_NPC(ch)) {
		act_char("You have no endurance whatsoever.", ch);
		return;
	}

	if (is_sn_affected(ch, "endure")) {
		act_char("You cannot endure more concentration.", ch);
		return;
	}

	WAIT_STATE(ch, skill_beats("endure"));

	paf = aff_new(TO_AFFECTS, "endure");
	paf->level	= ch->level;
	paf->duration	= ch->level / 4;
	INT(paf->location)= APPLY_SAVING_SPELL;
	paf->modifier	= -get_skill(ch, "endure") / 10;
	affect_to_char(ch, paf);
	aff_free(paf);

	act_char("You prepare yourself for magical encounters.", ch);
	act("$n concentrates for a moment, then resumes $s position.",
	    ch, NULL, NULL, TO_ROOM);
	check_improve(ch, "endure", TRUE, 1);
}

DO_FUN(do_tame, ch, argument)
{
	CHAR_DATA *victim;
	char arg[MAX_INPUT_LENGTH];

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		act_char("You are beyond taming.", ch);
		act("$n tries to tame $mself but fails miserably.",
		    ch, NULL, NULL, TO_ROOM);
		return;
	}

	WAIT_STATE(ch, skill_beats("tame"));

	if ((victim = get_char_here(ch,arg)) == NULL) {
		act_char("They're not here.", ch);
		return;
	}

	if (IS_NPC(ch)) {
		act_char("Why don't you tame yourself first?", ch);
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

	if (number_percent() < get_skill(ch, "tame") + 15 + 4 * (LEVEL(ch) - LEVEL(victim))) {
		SET_BIT(victim->affected_by, AFF_CALM);
		act_char("You calm down.", victim);
		act("You calm $N down.", ch, NULL, victim, TO_CHAR);
		act("$n calms $N down.", ch, NULL, victim, TO_NOTVICT);
		stop_fighting(victim, TRUE);
		check_improve(ch, "tame", TRUE, 1);
	} else {
		act_char("You failed.", ch);
		act("$n tries to calm down $N but fails.",
		    ch, NULL, victim, TO_NOTVICT);
		act("$n tries to calm you down but fails.",
		    ch, NULL, victim, TO_VICT);
		check_improve(ch, "tame", FALSE, 1);
	}
}

DO_FUN(do_assassinate, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	if (MOUNTED(ch)) {
		act_char("You can't assassinate while riding!", ch);
		return;
	}

	if (ch->master != NULL && IS_NPC(ch))
		return;

	if (IS_AFFECTED(ch, AFF_CHARM) && ch->master != NULL)  {
		act("You don't want to kill your beloved master.",
		    ch, NULL, ch->master, TO_CHAR);
		return;
	}

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		act_char("Assassinate whom?", ch);
		return;
	}

	if ((victim = get_char_here(ch, arg)) == NULL) {
		WAIT_STATE(ch, MISSING_TARGET_DELAY);
		act_char("They aren't here.", ch);
		return;
	}

	if (victim == ch) {
		act_char("Suicide is against your way.", ch);
		return;
	}

	if (IS_IMMORTAL(victim) && !IS_NPC(victim)) {
		act_char("Your hands pass through.", ch);
		return;
	}

	if (victim->fighting != NULL) {
		act_char("You can't assassinate a fighting person.", ch);
		return;
	}

	if ((get_eq_char(ch, WEAR_WIELD) != NULL)
	||  (get_eq_char(ch, WEAR_SHIELD) != NULL)
	||  (get_eq_char(ch, WEAR_HOLD ) != NULL))  {
		act_char("You need both hands free to assassinate somebody.", ch);
		return;
	}

	if (victim->hit < victim->max_hit * 9 / 10) {
		act("$N is hurt and suspicious ... you can't sneak up.",
		    ch, NULL, victim, TO_CHAR);
		return;
	}

	if (is_safe(ch, victim))
		return;

	WAIT_STATE(ch, skill_beats("assassinate"));

	if (number_percent() < get_skill(ch, "assassinate")
	&&  !IS_CLAN_GUARD(victim)
	&&  !IS_IMMORTAL(victim))
		one_hit(ch, victim, "assassinate", WEAR_WIELD);
	else {
		check_improve(ch, "assassinate", FALSE, 1);
		damage(ch, victim, 0, "assassinate", DAM_F_SHOW);
	}
	yell(victim, ch, "Help! $lu{$N} tries to assassinate me!");
}

DO_FUN(do_caltrops, ch, argument)
{
	CHAR_DATA *victim = ch->fighting;

	if (victim == NULL) {
		act_char("You must be in combat.", ch);
		return;
	}

	if (is_safe(ch, victim))
		return;

	act("You throw a handful of sharp spikes at the feet of $N.",
	    ch, NULL, victim, TO_CHAR);
	act("$n throws a handful of sharp spikes at your feet!",
	    ch, NULL, victim, TO_VICT);

	WAIT_STATE(ch, skill_beats("caltrops"));

	if (!IS_NPC(ch) && number_percent() >= get_skill(ch, "caltrops")) {
		damage(ch, victim, 0, "caltrops", DAM_F_SHOW);
		check_improve(ch, "caltrops", FALSE, 1);
		return;
	}

	damage(ch, victim, LEVEL(ch), "caltrops", DAM_F_SHOW);
	if (IS_EXTRACTED(victim))
		return;

	if (!is_sn_affected(victim, "caltrops")) {
		AFFECT_DATA *paf;

		paf = aff_new(TO_AFFECTS, "caltrops");
		paf->level	= ch->level;
		paf->duration	= -1;
		paf->modifier	= -5;
		INT(paf->location)= APPLY_HITROLL;
		affect_to_char(victim, paf);

		INT(paf->location)= APPLY_DAMROLL;
		affect_to_char(victim, paf);

		INT(paf->location)= APPLY_DEX;
		affect_to_char(victim, paf);
		aff_free(paf);

		act("$N starts limping.", ch, NULL, victim, TO_CHAR);
		act("You start to limp.", ch, NULL, victim, TO_VICT);
		check_improve(ch, "caltrops", TRUE, 1);
	}
}

DO_FUN(do_throw, ch, argument)
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
		act_char("You can't throw while riding!", ch);
		return;
	}

	if ((chance = get_skill(ch, "throw")) == 0) {
		act_char("A clutz like you couldn't throw down a worm.", ch);
		return;
	}
#if 0
	if (IS_AFFECTED(ch, AFF_FLYING)) {
		act_char("Your feet must touch the ground to balance.", ch);
		return;
	}
#endif
	if ((victim = ch->fighting) == NULL) {
		act_char("You aren't fighting anyone.", ch);
		return;
	}

	if (free_hands(ch) < 1) {
		act("You should have a free hand to catch your victim.",
			ch, NULL, NULL, TO_CHAR);
		return;
	}

	if (victim->in_room != ch->in_room) {
		act_char("They aren't here.", ch);
		return;
	}

	if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim) {
		act("But $N is your friend!", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (is_safe(ch,victim))
		return;

	WAIT_STATE(ch, skill_beats("throw"));

	if (is_sn_affected(victim, "protective shield")) {
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

	if (free_hands(ch)<2)
		chance -= 20;

	/* stats */
	chance += get_curr_stat(ch, STAT_STR);
	chance -= get_curr_stat(victim, STAT_DEX) * 4/3;

	if (IS_AFFECTED(victim, AFF_FLYING))
		chance += 10;

	/* speed */
	if (IS_NPC(ch) && IS_SET(ch->pMobIndex->off_flags, OFF_FAST))
		chance += 10;
	if (IS_NPC(victim) && IS_SET(victim->pMobIndex->off_flags, OFF_FAST))
		chance -= 20;

	/* level */
	chance += (LEVEL(ch) - LEVEL(victim)) * 2;

	if (check_close(ch, victim)
	|| distance_check(ch, victim))
		return;

	if (number_percent() < chance) {
		act("You throw $N to the ground with stunning force.",
		    ch, NULL, victim, TO_CHAR);
		act("$n throws you to the ground with stunning force.",
		    ch, NULL, victim, TO_VICT);
		act("$n throws $N to the ground with stunning force.",
		    ch, NULL, victim, TO_NOTVICT);
		WAIT_STATE(victim, 2 * get_pulse("violence"));

		damage(ch, victim,
		    (LEVEL(ch) + get_curr_stat(ch, STAT_STR)) * free_hands(ch),
		    "throw", DAM_F_SHOW);
		check_improve(ch, "throw", TRUE, 1);
		check_downstrike(victim);
	} else {
		act("You fail to grab your opponent.",
		    ch, NULL, NULL, TO_CHAR);
		act("$N tries to throw you, but fails.",
		    victim, NULL, ch, TO_CHAR);
		act("$n tries to grab $N's arm.",
		    ch, NULL, victim, TO_NOTVICT);
		check_improve(ch, "throw", FALSE, 1);
	}
}

DO_FUN(do_strangle, ch, argument)
{
	CHAR_DATA *victim;
	int chance;

	if (MOUNTED(ch)) {
		act_char("You can't strangle while riding!", ch);
		return;
	}

	if (IS_AFFECTED(ch, AFF_CHARM))  {
		act("You don't want to grap your beloved masters' neck.", ch, NULL, ch->master, TO_CHAR);
		return;
	}

	if ((victim = get_char_here(ch,argument)) == NULL) {
		WAIT_STATE(ch, MISSING_TARGET_DELAY);
		act_char("You do not see that person here.", ch);
		return;
	}

	if (ch == victim) {
		act_char("Even you are not that stupid.", ch);
		return;
	}

	if (is_sn_affected(victim, "strangle"))
		return;

	if (MOUNTED(victim)) {
		act_char("You can't strangle a riding one.", ch);
		return;
	}

	if (IS_AFFECTED(victim, AFF_SLEEP))  {
		act("$E is already asleep.", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (is_safe(ch, victim))
		return;

	chance = get_skill(ch, "strangle");
	if (is_sn_affected(victim, "free action"))
		chance -= 15;

	SET_FIGHT_TIME(victim);
	SET_FIGHT_TIME(ch);

	WAIT_STATE(ch, skill_beats("strangle"));

	if (number_percent() < chance * 6 /10
	&&  !IS_CLAN_GUARD(victim)
	&&  !IS_IMMORTAL(victim)) {
		AFFECT_DATA *paf;

		act("You grab hold of $N's neck and put $M to sleep.",
		    ch, NULL, victim, TO_CHAR);
		act("$n grabs hold of your neck and puts you to sleep.",
		    ch, NULL, victim, TO_VICT);
		act("$n grabs hold of $N's neck and puts $M to sleep.",
		    ch, NULL, victim, TO_NOTVICT);
		check_improve(ch, "strangle", TRUE, 1);

		paf = aff_new(TO_AFFECTS, "strangle");
		paf->level = ch->level;
		paf->duration = LEVEL(ch) / 20 + 1;
		paf->bitvector = AFF_SLEEP;
		affect_join(victim, paf);
		aff_free(paf);

		if (IS_AWAKE(victim))
			victim->position = POS_SLEEPING;
		if (RIDDEN(victim))
			do_dismount(RIDDEN(victim), str_empty);
	} else {
		damage(ch,victim, 0, "strangle", DAM_F_SHOW);
		check_improve(ch, "strangle", FALSE, 1);
		yell(victim, ch, "Help! I'm being strangled by $N!");
	}
}

DO_FUN(do_headcrush, ch, argument)
{
	CHAR_DATA *victim;
	OBJ_DATA *wield;
	int chance;

	if ((victim = get_char_here(ch,argument)) == NULL) {
		WAIT_STATE(ch, MISSING_TARGET_DELAY);
		act_char("You do not see that person here.", ch);
		return;
	}

	if ((wield = get_eq_char(ch, WEAR_WIELD)) == NULL
	||  !WEAPON_IS(wield, WEAPON_MACE)) {
		act("You need to wield a mace for that.",
			ch, NULL, NULL, TO_CHAR);
		return;
	}

	if (victim == ch) {
		act("You try to shatter your head into small pieces, but it's too hard.",
			ch, NULL, NULL, TO_CHAR);
		return;
	}


	if (victim->position == POS_SLEEPING) {
		act("There are many ways to kill a sleeping one, and crushing his head is not the best of them.",
			ch, NULL, NULL, TO_CHAR);
		return;
	}

	if (victim->fighting) {
		act("$E is moving too quickly.", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (victim->hit < 7 * victim->max_hit / 10) {
		act("$N is hurt and suspicious... you couldn't sneak up.",
			    ch, NULL, victim, TO_CHAR);
		return;
	}

	WAIT_STATE(ch, skill_beats("head crush"));

	chance = get_skill(ch, "head crush");
	chance += get_curr_stat(victim, STAT_INT);
	chance += get_curr_stat(victim, STAT_WIS);
	chance -= get_curr_stat(victim, STAT_DEX);
	chance += get_curr_stat(ch, STAT_STR);
	chance /= 2;

	if (number_percent() > chance
	|| IS_IMMORTAL(victim)
	|| IS_CLAN_GUARD(victim)) {
		damage(ch, victim, 0, "head crush", DAM_F_SHOW);
		check_improve(ch, "head crush", FALSE, 2);
		yell(victim, ch, "Help! $lu{$N} just tried to shatter my skull!");
		return;
	}

	chance /= 2;

	if (number_percent() > chance) {
		one_hit(ch, victim, "head crush", WEAR_WIELD);
		yell(victim, ch, "Help! $lu{$N} just tried to shatter my skull!");
		return;
	}

	chance /= 2;

	if (number_percent() > chance) {
		AFFECT_DATA *paf;

		act("You hit $N's head with your mace and $E falls asleep.",
			ch, NULL, victim, TO_CHAR);
		act("You feel a sudden pain erupts through your skull!",
			ch, NULL, victim, TO_VICT);
		act("$n hits $N's head with $s mace and $N falls asleep.",
			ch, NULL, victim, TO_NOTVICT);

		paf = aff_new(TO_AFFECTS, "head crush");
		paf->level	= ch->level;
		paf->duration	= LEVEL(ch) / 20 + 1;
		paf->bitvector	= AFF_SLEEP;
		affect_join(victim, paf);
		aff_free(paf);

		check_improve(ch, "head crush", TRUE, 3);

		if (IS_AWAKE(victim))
			victim->position = POS_SLEEPING;
		if (RIDDEN(victim))
			do_dismount(RIDDEN(victim), str_empty);
		return;
	}

	act("You shatter $N's skull into small pieces.",
		ch, NULL, victim, TO_CHAR);
	act("$n shatters your skull into small pieces.",
		ch, NULL, victim, TO_VICT);
	act("$n shatters $N's skull into small pieces.",
		ch, NULL, victim, TO_NOTVICT);
	raw_kill(ch, victim);
}


DO_FUN(do_blackjack, ch, argument)
{
	CHAR_DATA *victim;
	int chance;

	if (MOUNTED(ch)) {
		act_char("You can't blackjack while riding!", ch);
		return;
	}

	if ((victim = get_char_here(ch, argument)) == NULL) {
		WAIT_STATE(ch, MISSING_TARGET_DELAY);
		act_char("You do not see that person here.", ch);
		return;
	}

	if (ch == victim) {
		act_char("You idiot?! Blackjack yourself?!", ch);
		return;
	}

	if (MOUNTED(victim)) {
		act("You can't blackjack a riding person.",
		    ch, NULL, NULL, TO_CHAR);
		return;
	}

	if (IS_AFFECTED(ch, AFF_CHARM))  {
		act("You don't want to hit your beloved masters' head with a full filled jack.", ch, NULL, ch->master, TO_CHAR);
		return;
	}

	if (IS_AFFECTED(victim, AFF_SLEEP))  {
		act("$E is already asleep.", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (is_safe(ch,victim))
		return;

	SET_FIGHT_TIME(victim);
	SET_FIGHT_TIME(ch);

	chance = get_skill(ch, "blackjack") / 2;
	chance += URANGE(0, (get_curr_stat(ch, STAT_DEX)-20)*2, 10);
	chance += can_see(victim, ch) ? 0 : 5;
	if (IS_NPC(victim) && victim->pMobIndex->pShop != NULL)
		chance -= 40;
	if (is_sn_affected(victim, "free action"))
		chance -= 15;

	WAIT_STATE(ch, skill_beats("blackjack"));

	if (number_percent() < chance
	&&  !IS_CLAN_GUARD(victim)
	&&  !IS_IMMORTAL(victim)) {
		AFFECT_DATA *paf;

		act("You hit $N's head with a lead filled sack.",
		    ch, NULL, victim, TO_CHAR);
		act("You feel a sudden pain erupts through your skull!",
		    ch, NULL, victim, TO_VICT);
		act("$n whacks $N at the back of $S head with a heavy looking sack!  *OUCH*",
		    ch, NULL, victim, TO_NOTVICT);
		check_improve(ch, "blackjack", TRUE, 1);

		paf = aff_new(TO_AFFECTS, "blackjack");
		paf->level	= ch->level;
		paf->duration	= LEVEL(ch) / 15 + 1;
		paf->bitvector	= AFF_SLEEP;
		affect_join(victim, paf);
		aff_free(paf);

		if (IS_AWAKE(victim))
			victim->position = POS_SLEEPING;
		if (RIDDEN(victim))
			do_dismount(RIDDEN(victim), str_empty);
	} else {
		damage(ch, victim, LEVEL(ch)/2, "blackjack", DAM_F_SHOW);
		check_improve(ch, "blackjack", FALSE, 1);
		yell(victim, ch, "Help! I'm being blackjacked by $N!");
	}
}

DO_FUN(do_bloodthirst, ch, argument)
{
	int hp_percent;
	int chance;

	if (IS_AFFECTED(ch, AFF_BLOODTHIRST)) {
		act_char("Your thirst for blood continues.", ch);
		return;
	}

	if (IS_AFFECTED(ch, AFF_CALM)) {
		act_char("You're feeling to mellow to be bloodthirsty.", ch);
		return;
	}

	if (ch->fighting == NULL) {
		act_char("You need to be fighting.", ch);
		return;
	}

	/* modifiers */

	hp_percent = 100 * ch->hit / ch->max_hit;
	chance = get_skill(ch, "bloodthirst");
	chance += 25 - hp_percent/2;

	if (number_percent() < chance) {
		AFFECT_DATA *paf;

		WAIT_STATE(ch, get_pulse("violence"));

		act_char("You hunger for blood!", ch);
		act("$n gets a bloodthirsty look in $s eyes.",
		    ch, NULL, NULL, TO_ROOM);
		check_improve(ch, "bloodthirst", TRUE, 2);

		paf = aff_new(TO_AFFECTS, "bloodthirst");
		paf->level	= ch->level;
		paf->duration	= 2 + LEVEL(ch) / 18;
		paf->modifier	= 5 + LEVEL(ch) / 4;
		paf->bitvector	= AFF_BLOODTHIRST;

		INT(paf->location)= APPLY_HITROLL;
		affect_to_char(ch, paf);

		INT(paf->location)= APPLY_DAMROLL;
		affect_to_char(ch, paf);

		INT(paf->location)= APPLY_AC;
		paf->modifier	= - UMIN(LEVEL(ch) - 5, 35);
		affect_to_char(ch, paf);
		aff_free(paf);
	} else {
		WAIT_STATE(ch,3 * get_pulse("violence"));
		act_char("You feel bloodthirsty for a moment, but it passes.", ch);
		check_improve(ch, "bloodthirst", FALSE, 2);
	}
}

DO_FUN(do_toughen, ch, argument)
{
	int mana;

	if (is_sn_affected(ch, "toughen")) {
		act_char("You are as resistant as you will get.", ch);
		return;
	}

	mana = skill_mana(ch, "toughen");
	if (ch->mana < mana) {
		act_char("You cannot muster up the energy.", ch);
		return;
	}
	WAIT_STATE(ch, skill_beats("toughen"));

	if (number_percent() < get_skill(ch, "toughen")) {
		AFFECT_DATA *paf;

		paf = aff_new(TO_AFFECTS, "toughen");
		paf->level	= ch->level;
		paf->duration	= LEVEL(ch) / 6;
		INT(paf->location)= APPLY_SAVES;
		paf->modifier	= -LEVEL(ch)/4;
		affect_to_char(ch, paf);
		aff_free(paf);

		ch->mana -= mana;

		act("You feel tough!", ch, NULL, NULL, TO_CHAR);
		act("$n looks tougher.", ch, NULL, NULL, TO_ROOM);
		check_improve(ch, "toughen", TRUE, 1);
	} else {
		ch->mana -= mana/2;

		act_char("You flex your muscles, but you don't feel tougher.", ch);
		act("$n flexes $s muscles, trying to look tough.",
		    ch, NULL, NULL, TO_ROOM);
		check_improve(ch, "toughen", FALSE, 1);
	}
}

#define OBJ_VNUM_BATTLE_PONCHO		26

DO_FUN(do_trophy, ch, argument)
{
	int trophy_vnum;
	OBJ_DATA *trophy;
	OBJ_DATA *part;
	char arg[MAX_INPUT_LENGTH];
	int level;
	int mana;
	int chance;

	if (is_sn_affected(ch, "trophy")) {
		act_char("But you've already got one trophy!", ch);
		return;
	}

	mana = skill_mana(ch, "trophy");
	if (ch->mana < mana) {
		act_char("You feel too weak to concentrate on a trophy.", ch);
		return;
	}

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		act_char("Make a trophy of what?", ch);
		return;
	}

	if ((part = get_obj_carry(ch, ch, arg)) == NULL) {
		act_char("You do not have that body part.", ch);
		return;
	}

	chance = get_skill(ch, "trophy");
	if (number_percent() < chance * 2 / 3) {
		act_char("You destroyed it.", ch);
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
		act_char("Why don't you just eat those instead?", ch);
		return;
	} else {
		act_char("You can't make a trophy out of that!", ch);
		return;
	}

	if (mlstr_null(&part->owner)) {
		act_char("Invalid body part.", ch);
		return;
	}

	if (!IS_NPC(ch) && number_percent() < chance) {
		AFFECT_DATA *paf;

		paf = aff_new(TO_AFFECTS, "trophy");
		paf->level	= ch->level;
		paf->duration	= ch->level/2;
		affect_to_char(ch, paf);
		aff_free(paf);

		if (trophy_vnum != 0
		&&  (trophy = create_obj_of(trophy_vnum, &part->owner)) != NULL) {
			level = UMIN(part->level + 5, MAX_LEVEL);

			trophy->level	= ch->level;
			trophy->timer	= ch->level * 2;
			trophy->cost	= 0;
			ch->mana	-= mana;

			paf = aff_new(TO_OBJECT, "trophy");
			paf->level	= level;
			paf->duration	= -1;
			INT(paf->location)= APPLY_DAMROLL;
			paf->modifier  = LEVEL(ch) / 5;
			affect_to_obj(trophy, paf);

			INT(paf->location)= APPLY_HITROLL;
			paf->modifier  = LEVEL(ch) / 5;
			affect_to_obj(trophy, paf);

			INT(paf->location)= APPLY_INT;
			paf->modifier	= level > 20 ? -2 : -1;
			affect_to_obj(trophy, paf);

			INT(paf->location)= APPLY_STR;
			paf->modifier	= level > 20 ? 2 : 1;
			affect_to_obj(trophy, paf);
			aff_free(paf);

			INT(trophy->value[0]) = LEVEL(ch);
			INT(trophy->value[1]) = LEVEL(ch);
			INT(trophy->value[2]) = LEVEL(ch);
			INT(trophy->value[3]) = LEVEL(ch);

			obj_to_char(trophy, ch);
			check_improve(ch, "trophy", TRUE, 1);

			act("You make a poncho from $p!",
			    ch, part, NULL, TO_CHAR);
			act("$n makes a poncho from $p!",
			    ch, part, NULL, TO_ROOM);

			extract_obj(part, 0);
			return;
		}
	} else {
		act_char("You destroyed it.", ch);
		extract_obj(part, 0);
		ch->mana -= mana / 2;
		check_improve(ch, "trophy", FALSE, 1);
	}
}

DO_FUN(do_truesight, ch, argument)
{
	if (is_sn_affected(ch, "truesight")) {
		act_char("Your eyes are as sharp as they can get.", ch);
		return;
	}

	WAIT_STATE(ch, skill_beats("truesight"));

	if (number_percent() < get_skill(ch, "truesight")) {
		AFFECT_DATA *paf;

		paf = aff_new(TO_DETECTS, "truesight");
		paf->level    = ch->level;
		paf->duration = LEVEL(ch)/2 + 5;
		paf->bitvector = ID_INVIS | ID_MAGIC;
		affect_to_char(ch, paf);
		aff_free(paf);

		act("You look around sharply!", ch, NULL, NULL, TO_CHAR);
		act("$n looks more enlightened.", ch, NULL, NULL, TO_ROOM);
		check_improve(ch, "truesight", TRUE, 1);
	} else {
		act_char("You look about sharply, but you don't see anything new.", ch);
		act("$n looks around sharply but doesn't seem enlightened.",
		    ch, NULL, NULL, TO_ROOM);
		check_improve(ch, "truesight", FALSE, 1);
	}
}

DO_FUN(do_warcry, ch, argument)
{
	AFFECT_DATA *paf;
	int mana;

	if (is_sn_affected(ch, "warcry")) {
		act_char("The fighting zeal is already with you.", ch);
		return;
	}

	mana = skill_mana(ch, "warcry");
	if (ch->mana < mana) {
		act_char("You can't concentrate enough right now.", ch);
		return;
	}
	WAIT_STATE(ch, skill_beats("warcry"));

	if (number_percent() > get_skill(ch, "warcry")) {
		act_char("You grunt softly.", ch);
		act("$n makes some soft grunting noises.",
		    ch, NULL, NULL, TO_ROOM);
		ch->mana -= mana/2;
		check_improve(ch, "warcry", FALSE, 1);
		return;
	}

	ch->mana -= mana;
	check_improve(ch, "warcry", TRUE, 1);

	paf = aff_new(TO_AFFECTS, "warcry");
	paf->level	= ch->level;
	paf->duration	= 6 + ch->level;
	INT(paf->location)= APPLY_HITROLL;
	paf->modifier	= UMAX(1, LEVEL(ch) / 8);
	affect_to_char(ch, paf);

	INT(paf->location)= APPLY_SAVING_SPELL;
	paf->modifier	= 0 - UMAX(1, LEVEL(ch) / 8);
	affect_to_char(ch, paf);
	aff_free(paf);

	act_char("You feel righteous as you yell out your warcry.", ch);
}

DO_FUN(do_guard, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	CHAR_DATA *guarding;

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		act_char("Guard whom?", ch);
		return;
	}

	if ((victim = get_char_here(ch, arg)) == NULL) {
		act_char("They aren't here.", ch);
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
			act_char("You can't guard yourself!", ch);
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
		act_char("But you're already guarding someone else!", ch);
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
		act_char("Why don't you let them stop fighting first?", ch);
		return;
	}

	if (ch->fighting != NULL) {
		act_char("Better finish your own battle before you worry about guarding someone else.", ch);
		return;
	}

	act("You are now guarding $N.", ch, NULL, victim, TO_CHAR);
	act("You are being guarded by $n.", ch, NULL, victim, TO_VICT);
	act("$n is now guarding $N.", ch, NULL, victim, TO_NOTVICT);

	PC(ch)->guarding = victim;
	PC(victim)->guarded_by = ch;
}

DO_FUN(do_explode, ch, argument)
{
	CHAR_DATA *victim = ch->fighting;
	CHAR_DATA *vch;
	int dam, hp_dam, hpch, dice_dam, mana;
	char arg[MAX_INPUT_LENGTH];

	if (ch->fighting)
		victim = ch->fighting;
	else {
		one_argument(argument, arg, sizeof(arg));
		if (arg[0] == '\0') {
			act_char("You play with the exploding material.", ch);
			return;
		}
	}

	if (victim == NULL || victim->in_room != ch->in_room) {
		WAIT_STATE(ch, MISSING_TARGET_DELAY);
		act_char("They aren't here.", ch);
		return;
	}

	mana = skill_mana(ch, "explode");
	if (ch->mana < mana) {
		act_char("You can't find that much energy to fire.", ch);
		return;
	}
	ch->mana -= mana;
	WAIT_STATE(ch, skill_beats("explode"));

	act("$n burns something.", ch, NULL, victim, TO_NOTVICT);
	act("$n burns a cone of exploding material over you!",
	    ch, NULL, victim, TO_VICT);
	act("Burn them all!", ch, NULL, NULL, TO_CHAR);

	if (number_percent() >= get_skill(ch, "explode")) {
		damage(ch, victim, 0, "explode", DAM_F_SHOW);
		check_improve(ch, "explode", FALSE, 1);
		return;
	}

	hpch = UMAX(10, ch->hit);
	hp_dam = number_range(hpch/9 + 1, hpch/5);
	dice_dam = dice(LEVEL(ch), 20);
	dam = UMAX(hp_dam + dice_dam /10, dice_dam + hp_dam / 10);

	inflict_effect("fire", victim->in_room, LEVEL(ch), dam/2);
	foreach (vch, char_in_room(victim->in_room)) {
		bool attack = (ch->fighting != vch);

		if (is_safe_spell(ch, vch, TRUE)
		||  (IS_NPC(vch) && IS_NPC(ch) &&
		     (ch->fighting != vch || vch->fighting != ch)))
			continue;

		if (vch == victim) { /* full damage */
			inflict_effect("fire", vch, LEVEL(ch), dam);
			damage(ch, vch, dam, "explode", DAM_F_SHOW);
		} else { /* partial damage */
			inflict_effect("fire", vch, LEVEL(ch)/2, dam/4);
			damage(ch, vch, dam/2, "explode", DAM_F_SHOW);
		}

		if (attack)
			yell(vch, ch, "Help! $lu{$N} tries to burn me!");
	} end_foreach (vch);

	if (number_percent() >= get_skill(ch, "explode")) {
		inflict_effect("fire", ch, LEVEL(ch)/4, dam/10);
		damage(ch, ch, (ch->hit / 10), "explode", DAM_F_SHOW);
	}
}

DO_FUN(do_target, ch, argument)
{
	CHAR_DATA *victim;

	if (ch->fighting == NULL) {
		act_char("You aren't fighting yet.", ch);
		return;
	}

	if (argument[0] == '\0') {
		act_char("Change target to whom?", ch);
		return;
	}

	if ((victim = get_char_here(ch, argument)) == NULL) {
		act_char("You don't see that item.", ch);
		return;
	}

	/* check victim is fighting with him */

	if (victim->fighting != ch) {
		act_char("Target is not fighting with you.", ch);
		return;
	}

	WAIT_STATE(ch, skill_beats("target"));

	if (number_percent() < get_skill(ch, "target") / 2) {
		check_improve(ch, "target", TRUE, 1);

		ch->fighting = victim;

		act("$n changes $s target to $N!",
		    ch, NULL, victim, TO_NOTVICT);
		act("You change your target to $N!",
		    ch, NULL, victim, TO_CHAR);
		act("$n changes target to you!", ch, NULL, victim, TO_VICT);
		return;
	}

	act_char("You tried, but you couldn't. But for honour try again!", ch);
	check_improve(ch, "target", FALSE, 1);
}

DO_FUN(do_tiger, ch, argument)
{
	int hp_percent;
	int mana;
	int chance;

	act("$n calls the power of ten tigers!", ch, NULL, NULL, TO_ROOM);

	if (IS_AFFECTED(ch, AFF_BERSERK) || is_sn_affected(ch, "berserk")
	||  is_sn_affected(ch, "tiger power") || is_sn_affected(ch, "frenzy")) {
		act_char("You get a little madder.", ch);
		return;
	}

	if (IS_AFFECTED(ch,AFF_CALM)) {
		act_char("You're feeling too mellow to call ten tigers.", ch);
		return;
	}

	if (ch->in_room->sector_type != SECT_FIELD
	&&  ch->in_room->sector_type != SECT_FOREST
	&&  ch->in_room->sector_type != SECT_MOUNTAIN
	&&  ch->in_room->sector_type != SECT_HILLS) {
		act_char("No tigers can hear your call.", ch);
		return;
	}

	mana = skill_mana(ch, "tiger power");
	if (ch->mana < mana) {
		act_char("You can't get up enough energy.", ch);
		return;
	}

	/* modifiers */

	/* fighting */
	chance = get_skill(ch, "tiger power");
	if (ch->position == POS_FIGHTING)
		chance += 10;

	hp_percent = 100 * ch->hit/ch->max_hit;
	chance += 25 - hp_percent/2;
	ch->move /= 2;

	if (number_percent() < chance) {
		AFFECT_DATA *paf;

		WAIT_STATE(ch, skill_beats("tiger power"));
		ch->mana -= mana;

		/* heal a little damage */
		ch->hit += LEVEL(ch) * 2;
		ch->hit = UMIN(ch->hit, ch->max_hit);

		act_char("Ten tigers come for your call, as you call them!", ch);
		act("Ten tigers come across $n, and connect with $n.",
		    ch, NULL, NULL, TO_ROOM);
		check_improve(ch, "tiger power", TRUE, 2);

		paf = aff_new(TO_AFFECTS, "tiger power");
		paf->level	= ch->level;
		paf->duration	= number_fuzzy(ch->level / 8);
		paf->modifier	= UMAX(1, LEVEL(ch)/5);

		INT(paf->location)= APPLY_HITROLL;
		affect_to_char(ch, paf);

		INT(paf->location)= APPLY_DAMROLL;
		affect_to_char(ch, paf);

		paf->modifier	= UMAX(10,10 * (LEVEL(ch)/5));
		INT(paf->location)= APPLY_AC;
		paf->bitvector	= AFF_BERSERK;
		affect_to_char(ch, paf);
		aff_free(paf);
	} else {
		WAIT_STATE(ch, 2 * skill_beats("tiger power"));
		ch->mana -= mana/2;
		act_char("Your feel stregthen up, but nothing happens.", ch);
		check_improve(ch, "tiger power", FALSE, 2);
	}
}

DO_FUN(do_hara, ch, argument)
{
	if (MOUNTED(ch)) {
		act_char("You can't harakiri while riding!", ch);
		return;
	}

	if (is_sn_affected(ch, "hara kiri")) {
		act_char("If you want to kill yourself go and try to kill He-Man.", ch);
		return;
	}

	/* fighting */
	if (ch->position == POS_FIGHTING) {
		act_char("Try your chance during fighting.", ch);
		return;
	}

	if (number_percent() < get_skill(ch, "hara kiri")) {
		AFFECT_DATA *paf;

		WAIT_STATE(ch, skill_beats("hara kiri"));

		ch->hit = 1;
		ch->mana = 1;
		ch->move = 1;

		if (PC(ch)->condition[COND_HUNGER] < 40)
			PC(ch)->condition[COND_HUNGER] = 40;
		if (PC(ch)->condition[COND_THIRST] < 40)
			PC(ch)->condition[COND_THIRST] = 40;

		act_char("You cut your finger and wait till all your blood finishes.",ch);
		act_puts("$n cuts his body and looks in a deadly figure.",
			 ch, NULL, NULL, TO_ROOM, POS_FIGHTING);
		check_improve(ch, "hara kiri", TRUE, 2);
		do_sleep(ch, str_empty);
		SET_BIT(PC(ch)->plr_flags,PLR_HARA_KIRI);

		paf = aff_new(TO_AFFECTS, "hara kiri");
		paf->level     = ch->level;
		paf->duration  = 10;
		affect_to_char(ch, paf);
		aff_free(paf);
	} else {
		AFFECT_DATA *paf;

		WAIT_STATE(ch, 2 * skill_beats("hara kiri"));

		paf = aff_new(TO_AFFECTS, "hara kiri");
		paf->level     = ch->level;
		affect_to_char(ch, paf);
		aff_free(paf);

		act_char("You couldn't cut your finger. It is not so easy, you know.", ch);
		check_improve(ch, "hara kiri", FALSE, 2);
	}
}

DO_FUN(do_shield, ch, argument)
{
	CHAR_DATA *victim;
	int chance,ch_weapon,vict_shield;
	OBJ_DATA *shield,*axe;

	if (IS_NPC(ch))
		return;

	if ((victim = ch->fighting) == NULL) {
		act_char("You aren't fighting anyone.", ch);
		return;
	}

	if (victim->in_room != ch->in_room) {
		act_char("They aren't here.", ch);
		return;
	}

	if ((axe = get_eq_char(ch, WEAR_WIELD)) == NULL) {
		act_char("You must be wielding a weapon.", ch);
		return;
	}

	if ((shield = get_eq_char(victim, WEAR_SHIELD)) == NULL) {
		act_char("Your opponent must wear a shield.", ch);
		return;
	}

	if (material_is(shield, MATERIAL_INDESTRUCT))
		return;

	if (WEAPON_IS(axe, WEAPON_AXE))
		chance = chance * 12 / 10;
	else if (!WEAPON_IS(axe, WEAPON_SWORD)) {
		act_char("Your weapon must be an axe or a sword.", ch);
		return;
	}

	/* find weapon skills */
	ch_weapon = get_weapon_skill(ch, get_weapon_sn(axe));
	vict_shield = get_skill(ch, "shield block");
	/* modifiers */

	/* skill */
	chance = get_skill(ch, "shield cleave") * ch_weapon / 200;
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
	} else {
		act("You fail to cleave $N's shield.",
		    ch, NULL, victim, TO_CHAR);
		act("$n tries to cleave your shield, but fails.",
		    ch, NULL, victim, TO_VICT);
		act("$n tries to cleave $N's shield, but fails.",
		    ch, NULL, victim, TO_NOTVICT);
		check_improve(ch, "shield cleave", FALSE, 1);
	}
}

DO_FUN(do_weapon, ch, argument)
{
	CHAR_DATA *victim;
	OBJ_DATA *wield, *axe;
	int chance, ch_weapon, vict_weapon;

	if (IS_NPC(ch))
		return;

	if ((victim = ch->fighting) == NULL) {
		act_char("You aren't fighting anyone.", ch);
		return;
	}

	if ((axe = get_eq_char(ch, WEAR_WIELD)) == NULL) {
		act_char("You must be wielding a weapon.", ch);
		return;
	}

	if ((wield = get_eq_char(victim, WEAR_WIELD)) == NULL) {
		act_char("Your opponent must wield a weapon.", ch);
		return;
	}

	if (material_is(wield, MATERIAL_INDESTRUCT))
		return;

	chance = get_skill(ch, "weapon cleave");
	if (WEAPON_IS(axe, WEAPON_AXE))
		chance = chance * 12 / 10;
	else if (!WEAPON_IS(axe, WEAPON_SWORD)) {
		act_char("Your weapon must be an axe or a sword.", ch);
		return;
	}

	/* find weapon skills */
	ch_weapon = get_weapon_skill(ch, get_weapon_sn(axe));
	vict_weapon = get_weapon_skill(victim, get_weapon_sn(wield));
	/* modifiers */

	/* skill */
	chance = chance * ch_weapon / 200;
	chance = chance * 101 / (vict_weapon + 1);

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
	} else {
		act("You fail to cleave $N's weapon.",
		    ch, NULL, victim, TO_CHAR);
		act("$n tries to cleave your weapon, but fails.",
		    ch, NULL, victim, TO_VICT);
		act("$n tries to cleave $N's weapon, but fails.",
		    ch, NULL, victim, TO_NOTVICT);
		check_improve(ch, "weapon cleave", FALSE, 1);
	}
}

DO_FUN(do_tail, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int chance;
	int wait;
	int damage_tail;
	bool attack;

	if (MOUNTED(ch)) {
		act_char("You can't tail while riding!", ch);
		return;
	}

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		victim = ch->fighting;
		if (victim == NULL) {
			act_char("But you aren't fighting anyone!", ch);
			return;
		}
	} else
		victim = get_char_here(ch, arg);

	if (!victim || victim->in_room != ch->in_room) {
		act_char("They aren't here.", ch);
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
		act_char("You try to hit yourself by your tail, but failed.", ch);
		return;
	}

	if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim) {
		act("But $N is your friend!", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (is_sn_affected(victim, "protective shield")) {
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
	chance = get_skill(ch, "tail");
	chance -= get_carry_weight(ch) / 20;
	chance += get_carry_weight(victim) / 25;

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
		act("You sprawle $N with your tail, and send $M flying!",
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

		WAIT_STATE(victim, wait * get_pulse("violence"));
		WAIT_STATE(ch, skill_beats("tail"));
		victim->position = POS_RESTING;
		damage_tail = ch->damroll +
			(2 * number_range(4,4 + 10* ch->size + chance/10));
		damage(ch, victim, damage_tail, "tail", DAM_F_SHOW);
		check_downstrike(victim);
	} else {
		damage(ch, victim, 0, "tail", DAM_F_SHOW);
		act("You lost your position and fall down!",
		    ch, NULL, victim, TO_CHAR);
		act("$n lost $gn{his} position and fall down!",
		    ch, NULL, victim, TO_NOTVICT);
		act("You evade $n's tail, causing $m to fall down.",
		    ch, NULL, victim, TO_VICT);
		check_improve(ch, "tail", FALSE, 1);
		ch->position = POS_RESTING;
		WAIT_STATE(ch, skill_beats("tail") * 3 / 2);
		check_downstrike(ch);
	}
	if (attack)
		yell(victim, ch, "Help! $lu{$N} tried to hit me with his tail!");
}

DO_FUN(do_concentrate, ch, argument)
{
	int mana;

	if (MOUNTED(ch)) {
		act_char("You can't concentrate while riding!", ch);
		return;
	}

	if (is_sn_affected(ch, "concentrate")) {
		act_char("You are already concentrated for the fight.", ch);
		return;
	}

	mana = skill_mana(ch, "concentrate");
	if (ch->mana < mana) {
		act_char("You can't get up enough energy.", ch);
		return;
	}

	/* fighting */
	if (ch->fighting != NULL) {
		act_char("Concentrate on your fighting!", ch);
		return;
	}

	WAIT_STATE(ch, skill_beats("concentrate"));
	if (number_percent() < get_skill(ch, "concentrate")) {
		AFFECT_DATA *paf;

		ch->mana -= mana;
		ch->move /= 2;

		do_sit(ch,str_empty);
		act_char("You sit down and relax, concentrating on the next fight!", ch);
		act_puts("$n concentrates for the next fight.",
			 ch, NULL, NULL, TO_ROOM, POS_FIGHTING);
		check_improve(ch, "concentrate", TRUE, 2);

		paf = aff_new(TO_AFFECTS, "concentrate");
		paf->level	= ch->level;
		paf->duration	= number_fuzzy(ch->level / 8);
		paf->modifier	= UMAX(1, LEVEL(ch)/8);

		INT(paf->location)= APPLY_HITROLL;
		affect_to_char(ch, paf);

		INT(paf->location)= APPLY_DAMROLL;
		affect_to_char(ch, paf);

		INT(paf->location)= APPLY_AC;
		paf->modifier	= UMAX(1,ch->level/10);
		affect_to_char(ch, paf);
		aff_free(paf);
	} else {
		ch->mana -= mana/2;
		act_char("You try to concentrate for the next fight but fail.", ch);
		check_improve(ch, "concentrate", FALSE, 2);
	}
}

DO_FUN(do_bandage, ch, argument)
{
	int heal;

	if (IS_AFFECTED(ch, AFF_REGENERATION)) {
		act_char("You are already using your bandage.", ch);
		return;
	}

	WAIT_STATE(ch, skill_beats("bandage"));
	if (number_percent() < get_skill(ch, "bandage")) {
		AFFECT_DATA *paf;

		act_char("You place your bandage to your shoulder!", ch);
		act("$n places a bandage to $s shoulder.",
		    ch, NULL, NULL, TO_ROOM);
		check_improve(ch, "bandage", TRUE, 2);

		heal = dice(4, 8) + LEVEL(ch) / 2;
		ch->hit = UMIN(ch->hit + heal, ch->max_hit);
		update_pos(ch);
		act_char("You feel better!", ch);

		paf = aff_new(TO_AFFECTS, "bandage");
		paf->level	= ch->level;
		paf->duration	= ch->level / 10;
		paf->modifier	= UMIN(15, ch->level/2);
		paf->bitvector	= AFF_REGENERATION;
		affect_to_char(ch, paf);
		aff_free(paf);
	} else {
		act_char("You failed to place your bandage to your shoulder.", ch);
		check_improve(ch, "bandage", FALSE, 2);
	}
}

#define OBJ_VNUM_CHUNK_IRON		6521
#define OBJ_VNUM_KATANA_SWORD		98

DO_FUN(do_katana, ch, argument)
{
	OBJ_DATA *katana;
	OBJ_DATA *part;
	char arg[MAX_INPUT_LENGTH];
	int chance;
	int mana;

	one_argument(argument, arg, sizeof(arg));
	if (is_sn_affected(ch, "katana")) {
		act_char("But you've already got one katana!", ch);
		return;
	}

	mana = skill_mana(ch, "katana");
	if (ch->mana < mana) {
		act_char("You feel too weak to concentrate on a katana.", ch);
		return;
	}

	if (arg[0] == '\0') {
		act_char("Make a katana from what?", ch);
		return;
	}

	if ((part = get_obj_carry(ch, ch, arg)) == NULL) {
		act_char("You do not have chunk of iron.", ch);
		return;
	}

	if (part->pObjIndex->vnum != OBJ_VNUM_CHUNK_IRON) {
		act_char("You do not have the right material.", ch);
		return;
	}

	chance = get_skill(ch, "katana");
	if (number_percent() < chance / 3 * 2) {
		act_char("You failed and destroyed it.", ch);
		extract_obj(part, 0);
		return;
	}

	WAIT_STATE(ch, skill_beats("katana"));

	if (number_percent() < chance) {
		AFFECT_DATA *paf;

		katana = create_obj(OBJ_VNUM_KATANA_SWORD, 0);
		if (katana == NULL)
			return;
		katana->level = ch->level;
		mlstr_cpy(&katana->owner, &ch->short_descr);
		katana->cost  = 0;
		ch->mana -= mana;

		paf = aff_new(TO_AFFECTS, "katana");
		paf->level	= ch->level;
		paf->duration	= ch->level;
		affect_to_char(ch, paf);

		paf->where	= TO_OBJECT;
		paf->level	= ch->level;
		paf->duration	= -1;
		INT(paf->location)= APPLY_DAMROLL;
		paf->modifier	= ch->level / 10;
		affect_to_obj(katana, paf);

		INT(paf->location)= APPLY_HITROLL;
		affect_to_obj(katana, paf);
		aff_free(paf);

		INT(katana->value[2]) = ch->level / 10;
		katana->ed = ed_new2(katana->pObjIndex->ed, ch->name);

		obj_to_char(katana, ch);
		check_improve(ch, "katana", TRUE, 1);

		act("You make a katana from $p!",ch,part,NULL,TO_CHAR);
		act("$n makes a katana from $p!",ch,part,NULL,TO_ROOM);

		extract_obj(part, 0);
		return;
	} else {
		act_char("You destroyed it.", ch);
		extract_obj(part, 0);
		ch->mana -= mana/2;
		check_improve(ch, "katana", FALSE, 1);
	}
}

DO_FUN(do_crush, ch, argument)
{
	CHAR_DATA *victim;
	int chance, wait;
	int damage_crush;

	if (MOUNTED(ch))
		return;

	if ((victim = ch->fighting) == NULL || victim->in_room != ch->in_room)
		return;

	if (victim->position < POS_FIGHTING)
		return;

	if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim)
		return;

	if (is_sn_affected(victim, "protective shield")) {
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

	if (check_close(ch, victim)
	||  distance_check(ch, victim))
		return;

	/* modifiers */

	/* size  and weight */
	chance = get_skill(ch, "crush");
	chance += get_carry_weight(ch) / 25;
	chance -= get_carry_weight(victim) / 20;

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
		act("$n squeezes you with a powerful crush!",
		    ch, NULL, victim, TO_VICT);
		act("You slam into $N, and crush $M!",
		    ch, NULL, victim, TO_CHAR);
		act("$n squeezes $N with a powerful crush.",
		    ch, NULL, victim, TO_NOTVICT);

		wait = 3;

		switch(number_bits(2)) {
		case 0: wait = 1; break;
		case 1: wait = 2; break;
		case 2: wait = 4; break;
		case 3: wait = 3; break;
		}

		WAIT_STATE(victim, wait * get_pulse("violence"));
		WAIT_STATE(ch, skill_beats("crush"));
		victim->position = POS_RESTING;
		damage_crush = (ch->damroll / 2) +
				number_range(4, 4 + 4*ch->size + chance/10);
		damage(ch, victim, damage_crush, "crush", DAM_F_SHOW);
		check_downstrike(victim);
	} else {
		damage(ch, victim, 0, "crush", DAM_F_SHOW);
		act_puts("You fall flat on your face!",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		act("$n falls flat on $s face.",
		    ch, NULL, victim, TO_NOTVICT);
		act("You evade $n's crush, causing $m to fall flat on $s face.",
		    ch, NULL, victim, TO_VICT);
		ch->position = POS_RESTING;
		WAIT_STATE(ch, skill_beats("crush") * 3 / 2);
		check_downstrike(ch);
	}
}

DO_FUN(do_sense, ch, argument)
{
	int mana;

	if (is_sn_affected(ch, "sense life")) {
		act_char("You can already feel life forms.", ch);
		return;
	}

	mana = skill_mana(ch, "sense life");
	if (ch->mana < mana) {
		act_char("You cannot seem to concentrate enough.", ch);
		return;
	}
	WAIT_STATE(ch, skill_beats("sense life"));

	if (number_percent() < get_skill(ch, "sense life")) {
		AFFECT_DATA *paf;

		paf = aff_new(TO_DETECTS, "sense life");
		paf->level	= ch->level;
		paf->duration	= ch->level;
		paf->bitvector	= ID_LIFE;
		affect_to_char(ch, paf);
		aff_free(paf);

		ch->mana -= mana;

		act("You start to sense life forms in the room!",
		    ch, NULL, NULL, TO_CHAR);
		act("$n looks more sensitive.", ch, NULL, NULL, TO_ROOM);
		check_improve(ch, "sense life", TRUE, 1);
	} else {
		ch->mana -= mana/2;
		act_char("You failed.", ch);
		check_improve(ch, "sense life", FALSE, 1);
	}
}

DO_FUN(do_poison_smoke, ch, argument)
{
	int mana;
	CHAR_DATA *vch;

	mana = skill_mana(ch, "poison smoke");
	if (ch->mana < mana) {
		act_char("You can't get up enough energy.", ch);
		return;
	}
	ch->mana -= mana;
	WAIT_STATE(ch, skill_beats("poison smoke"));

	if (number_percent() > get_skill(ch, "poison smoke")) {
		act_char("You failed.", ch);
		check_improve(ch, "poison smoke", FALSE, 1);
		return;
	}

	act("A cloud of poison smoke fills the room.", ch, NULL, NULL, TO_ALL);
	check_improve(ch, "poison smoke", TRUE, 1);
	foreach (vch, char_in_room(ch->in_room)) {
		bool attack = (ch->fighting != vch);

		if (is_safe_spell(ch, vch, TRUE))
			continue;

		spellfun("poison", NULL, LEVEL(ch), ch, vch);
		if (vch != ch) {
			if (attack)
				yell(vch, ch, "$lu{$N} tries to poison me!");
			multi_hit(vch, ch, NULL);
		}
	} end_foreach(vch);
}

DO_FUN(do_blindness_dust, ch, argument)
{
	CHAR_DATA *vch;
	char arg[MAX_INPUT_LENGTH];
	int mana;

	mana = skill_mana(ch, "blindness dust");
	if (ch->mana < mana) {
		act_char("You can't get up enough energy.", ch);
		return;
	}
	ch->mana -= mana;
	WAIT_STATE(ch, skill_beats("blindness dust"));

	if (number_percent() > get_skill(ch, "blindness dust")) {
		act_char("You failed.", ch);
		check_improve(ch, "blindness dust", FALSE, 1);
		return;
	}

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		act("A cloud of dust fills the room.", ch, NULL, NULL, TO_ALL);
		check_improve(ch, "blindness dust", TRUE, 1);

		foreach (vch, char_in_room(ch->in_room)) {
			bool attack = (ch->fighting != vch);

			if (is_safe_spell(ch, vch, TRUE))
				continue;

			spellfun("blindness", NULL, LEVEL(ch), ch, vch);
			if (attack) {
				yell(vch, ch,
				    "Help! $lu{$N} just threw dust into my eyes!");
			}
			if (vch != ch)
				multi_hit(vch, ch, NULL);
		} end_foreach (vch);
		return;
	}

	if ((vch = get_char_here(ch, arg)) == NULL) {
		act_char("They aren't here.", ch);
		return;
	}

	if (is_safe(ch, vch))
		return;

	act("You throw some dust into $N's eyes.", ch, NULL, vch, TO_CHAR);
	act("$n throws some dust into $N's eyes.", ch, NULL, vch, TO_ROOM);
	act("$n throws some dust into your eyes.", ch, NULL, vch, TO_VICT);
	spellfun("blindness", NULL, LEVEL(ch), ch, vch);
	if (vch != ch)
		multi_hit(vch, ch, NULL);
}

DO_FUN(do_dishonor, ch, argument)
{
	ROOM_INDEX_DATA *was_in;
	ROOM_INDEX_DATA *now_in;
	CHAR_DATA *gch;
	int attempt, level = 0;

	if (RIDDEN(ch)) {
		act_char("You should ask to your rider!", ch);
		return;
	}

	if (ch->fighting == NULL) {
		if (ch->position == POS_FIGHTING)
			ch->position = POS_STANDING;
		act_char("You aren't fighting anyone.", ch);
		return;
	}

	for (gch = char_list; gch; gch = gch->next)
		  if (is_same_group(gch, ch->fighting) || gch->fighting == ch)
			level += gch->level;

	if ((ch->fighting->level - ch->level) < 5 && ch->level > (level / 3)) {
		 act_char("Your fighting doesn't worth to dishonor yourself.", ch);
		 return;
	}

	was_in = ch->in_room;
	for (attempt = 0; attempt < 6; attempt++) {
		EXIT_DATA *pexit;
		int door;

		if (number_percent() >= get_skill(ch, "dishonor"))
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

		move_char(ch, door, 0);
		if ((now_in = ch->in_room) == was_in)
			continue;

		ch->in_room = was_in;
		act("$n has dishonored $mself!",
		    ch, NULL, NULL, TO_ROOM);
		ch->in_room = now_in;

		if (!IS_NPC(ch)) {
			act_char("You dishonored yourself and flee from combat.",ch);
			if (ch->level < LEVEL_HERO) {
				int xp = FLEE_EXP(ch) * 3;
				act_puts("You lose $j exp.",
					 ch, (const void *) xp, NULL,
					 TO_CHAR, POS_DEAD);
				gain_exp(ch, -xp);
			}
		} else {
			/* Once fled, the mob will not go after */
			NPC(ch)->last_fought = NULL;
		}

		stop_fighting(ch, TRUE);
		if (MOUNTED(ch))
			dofun("dismount", ch, str_empty);

		check_improve(ch, "dishonor", TRUE, 1);
		return;
	}

	act_char("PANIC! You couldn't escape!", ch);
	check_improve(ch, "dishonor", FALSE, 1);
}

DO_FUN(do_rake, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int chance, wait;
	bool attack = FALSE;

	if (MOUNTED(ch)) {
		act_char("You can't rake while riding!", ch);
		return;
	}

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		victim = ch->fighting;
		if ((victim = ch->fighting) == NULL) {
			act_char("But you aren't fighting anyone!", ch);
			return;
		}
	} else
		victim = get_char_here(ch, arg);

	if (victim == NULL || victim->in_room != ch->in_room) {
		WAIT_STATE(ch, MISSING_TARGET_DELAY);
		act_char("They aren't here.", ch);
		return;
	}

	WAIT_STATE(ch, skill_beats("rake"));

	if (victim->position < POS_FIGHTING) {
		act("You'll have to let $M get back up first.",
			  ch, NULL, victim, TO_CHAR);
		return;
	}

	if (victim == ch) {
		act_char("You grab yourself.", ch);
		WAIT_STATE(ch, skill_beats("rake"));
		return;
	}

	if (MOUNTED(victim)) {
		act_char("You can't rake a riding one!", ch);
		return;
	}

	if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim) {
		act("But $N is your friend!", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (is_safe(ch, victim))
		return;

	/* modifiers */
	chance = get_skill(ch, "rake");

	/* size  and weight */

	if (ch->size < victim->size)
		chance += (ch->size - victim->size) * 25;
	else
		chance += (ch->size - victim->size) * 10;

	/* stats */
	chance += get_curr_stat(ch, STAT_STR);
	chance -= get_curr_stat(victim, STAT_DEX) * 4/3;

	if (is_sn_affected(victim, "protective shield"))
		chance /= 2;

	/* speed */
	if (IS_NPC(ch) && IS_SET(ch->pMobIndex->off_flags, OFF_FAST))
		chance += 10;
	if (IS_NPC(victim) && IS_SET(victim->pMobIndex->off_flags, OFF_FAST))
		chance -= 20;

	/* level */
	chance += (LEVEL(ch) - LEVEL(victim)) * 2;

	RESET_WAIT_STATE(ch);
	attack = !(ch->fighting == victim);

	if (check_close(ch, victim)
	|| distance_check(ch, victim))
		return;

	/* now the attack */
	if (number_percent() < chance) {
		check_improve(ch, "rake", TRUE, 1);

		wait = 3;

		switch(number_bits(2)) {
			case 0: wait = 1; break;
			case 1: wait = 2; break;
			case 2: wait = 4; break;
			case 3: wait = 3; break;
		}

		act("You rake $N and throw $M to the ground.", ch,
		    NULL, victim, TO_CHAR);
		act("$n rakes you and throws to the ground.", ch,
		    NULL, victim, TO_VICT);
		WAIT_STATE(victim, wait * get_pulse("violence"));
		WAIT_STATE(ch, skill_beats("rake"));
		victim->position = POS_RESTING;
		damage(ch, victim, get_curr_stat(ch, STAT_STR) +
		    number_range(1, GET_DAMROLL(ch)), "rake", DAM_F_SHOW);
		check_downstrike(victim);
	} else {
		damage(ch, victim, 0, "rake", DAM_F_SHOW);
		check_improve(ch, "rake", FALSE, 1);
		WAIT_STATE(ch, skill_beats("rake"));
	}

	if (attack)
		yell(victim, ch, "Help! $lu{$N} is raking me!");
}
