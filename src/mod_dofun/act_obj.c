/*
 * $Id: act_obj.c,v 1.174 1999-11-19 13:05:27 fjoe Exp $
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
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "mob_prog.h"
#include "obj_prog.h"
#include "fight.h"
#include "lang.h"
#include "auction.h"

DECLARE_DO_FUN(do_split		);
DECLARE_DO_FUN(do_say		);
DECLARE_DO_FUN(do_mount		);
DECLARE_DO_FUN(do_yell		);
DECLARE_DO_FUN(do_emote		);

/*
 * Local functions.
 */
static CHAR_DATA *	find_keeper	(CHAR_DATA * ch);
static uint		get_cost	(CHAR_DATA * keeper, OBJ_DATA * obj,
					 bool fBuy);
static void		obj_to_keeper	(OBJ_DATA * obj, CHAR_DATA * ch);
static OBJ_DATA *	get_obj_keeper	(CHAR_DATA * ch, CHAR_DATA * keeper,
					 const char *argument);

static void		sac_obj		(CHAR_DATA * ch, OBJ_DATA *obj);
static bool		put_obj		(CHAR_DATA *ch, OBJ_DATA *container,
					 OBJ_DATA *obj, int* count);
static void		drop_obj	(CHAR_DATA *ch, OBJ_DATA *obj);

void do_get(CHAR_DATA * ch, const char *argument)
{
	char            arg1[MAX_INPUT_LENGTH];
	char            arg2[MAX_INPUT_LENGTH];
	OBJ_DATA       *obj, *obj_next;
	OBJ_DATA       *container;
	bool            found;

	argument = one_argument(argument, arg1, sizeof(arg1));
	argument = one_argument(argument, arg2, sizeof(arg2));

	if (!str_cmp(arg2, "from"))
		argument = one_argument(argument, arg2, sizeof(arg2));

	/* Get type. */
	if (arg1[0] == '\0') {
		char_puts("Get what?\n", ch);
		return;
	}
	if (arg2[0] == '\0') {
		if (str_cmp(arg1, "all") && str_prefix("all.", arg1)) {
			/* 'get obj' */
			obj = get_obj_list(ch, arg1, ch->in_room->contents);
			if (obj == NULL) {
				act_puts("I see no $T here.",
					 ch, NULL, arg1, TO_CHAR, POS_DEAD);
				return;
			}

			get_obj(ch, obj, NULL, NULL);
		} else {
			/* 'get all' or 'get all.obj' */
			found = FALSE;
			for (obj = ch->in_room->contents; obj != NULL;
			     obj = obj_next) {
				obj_next = obj->next_content;
				if ((arg1[3] == '\0' || is_name(arg1+4, obj->name))
				&& can_see_obj(ch, obj)) {
					found = TRUE;
					get_obj(ch, obj, NULL, NULL);
				}
			}

			if (!found) {
				if (arg1[3] == '\0')
					char_puts("I see nothing here.\n", ch);
				else
					act_puts("I see no $T here.",
						 ch, NULL, arg1+4, TO_CHAR,
						 POS_DEAD);
			}
		}
		return;
	}

	/* 'get ... container' */
	if (!str_cmp(arg2, "all") || !str_prefix("all.", arg2)) {
		char_puts("You can't do that.\n", ch);
		return;
	}
	if ((container = get_obj_here(ch, arg2)) == NULL) {
		act_puts("I see no $T here.",
			 ch, NULL, arg2, TO_CHAR, POS_DEAD);
		return;
	}

	switch (container->pObjIndex->item_type) {
	default:
		char_puts("That is not a container.\n", ch);
		return;

	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
		break;

	case ITEM_CORPSE_PC:
		if (!can_loot(ch, container)) {
			char_puts("You can't do that.\n", ch);
			return;
		}
	}

	if (IS_SET(INT_VAL(container->value[1]), CONT_CLOSED)) {
		act_puts("The $d is closed.",
			 ch, NULL, container->name, TO_CHAR, POS_DEAD);
		return;
	}
	if (str_cmp(arg1, "all") && str_prefix("all.", arg1)) {
		/* 'get obj container' */
		obj = get_obj_list(ch, arg1, container->contains);
		if (obj == NULL) {
			act_puts("I see nothing like that in the $T.",
				 ch, NULL, arg2, TO_CHAR, POS_DEAD);
			return;
		}
		get_obj(ch, obj, container, NULL);
	} else {
		/* 'get all container' or 'get all.obj container' */
		found = FALSE;
		for (obj = container->contains; obj != NULL; obj = obj_next) {
			obj_next = obj->next_content;
			if ((arg1[3] == '\0' || is_name(&arg1[4], obj->name))
			    && can_see_obj(ch, obj)) {
				found = TRUE;
				if (IS_SET(container->pObjIndex->extra_flags,
					   ITEM_PIT)
				&&  !IS_IMMORTAL(ch)) {
					act_puts("Don't be so greedy!",
						 ch, NULL, NULL, TO_CHAR,
						 POS_DEAD);
					return;
				}
				get_obj(ch, obj, container, NULL);
			}
		}

		if (!found) {
			if (arg1[3] == '\0')
				act_puts("I see nothing in the $T.",
					 ch, NULL, arg2, TO_CHAR, POS_DEAD);
			else
				act_puts("I see nothing like that in the $T.",
					 ch, NULL, arg2, TO_CHAR, POS_DEAD);
		}
	}
}

void do_put(CHAR_DATA * ch, const char *argument)
{
	char		arg1[MAX_INPUT_LENGTH];
	char		arg2[MAX_INPUT_LENGTH];
	OBJ_DATA *	container;
	OBJ_DATA *	obj;
	OBJ_DATA *	obj_next;
	OBJ_DATA *	objc;
	int		count;
	clan_t *	clan;

	argument = one_argument(argument, arg1, sizeof(arg1));
	argument = one_argument(argument, arg2, sizeof(arg2));

	if (!str_cmp(arg2, "in") || !str_cmp(arg2, "on"))
		argument = one_argument(argument, arg2, sizeof(arg2));

	if (arg1[0] == '\0' || arg2[0] == '\0') {
		char_puts("Put what in what?\n", ch);
		return;
	}

	if (!str_cmp(arg2, "all") || !str_prefix("all.", arg2)) {
		char_puts("You can't do that.\n", ch);
		return;
	}

	if ((container = get_obj_here(ch, arg2)) == NULL) {
		act_puts("I see no $T here.",
			 ch, NULL, arg2, TO_CHAR, POS_DEAD);
		return;
	}

	if (container->pObjIndex->item_type != ITEM_CONTAINER) {
		char_puts("That is not a container.\n", ch);
		return;
	}

	/*
	 * clan members can put item into clan altar even if it is closed
	 * check that obj is clan item is done in put_obj
	 */
	if (IS_SET(INT_VAL(container->value[1]), CONT_CLOSED) 
	&&  ((clan = clan_lookup(ch->clan)) == NULL ||
	     clan->altar_ptr != container)) {
		act_puts("$P is closed.",
			 ch, NULL, container, TO_CHAR, POS_DEAD);
		return;
	}

	if (str_cmp(arg1, "all") && str_prefix("all.", arg1)) {
		/* 'put obj container' */
		if ((obj = get_obj_carry(ch, arg1)) == NULL) {
			char_puts("You do not have that item.\n", ch);
			return;
		}

		if (obj == container) {
			char_puts("You can't fold it into itself.\n", ch);
			return;
		}

		if (!can_drop_obj(ch, obj)) {
			char_puts("You can't let go of it.\n", ch);
			return;
		}

		if (WEIGHT_MULT(obj) != 100) {
			char_puts("You have a feeling that would be a bad idea.\n", ch);
			return;
		}

		if (obj->pObjIndex->limit != -1) {
			act_puts("This unworthy container won't hold $p.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			return;
		}

		if (get_obj_weight(obj) + get_true_weight(container) >
		    (INT_VAL(container->value[0]) * 10)
		||  get_obj_weight(obj) > (INT_VAL(container->value[3]) * 10)) {
			char_puts("It won't fit.\n", ch);
			return;
		}

		count = 0;
		for (objc = container->contains; objc; objc = objc->next_content)
			count++;

		put_obj(ch, container, obj, &count);
	} else {
		if (!IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM)) {
			do_say(ch, "Nah, I won't do that.");
			return;
		}

		count = 0;
		for (objc = container->contains; objc; objc = objc->next_content)
			count++;

		/* 'put all container' or 'put all.obj container' */
		for (obj = ch->carrying; obj != NULL; obj = obj_next) {
			obj_next = obj->next_content;

			if ((arg1[3] == '\0' || is_name(&arg1[4], obj->name))
			&&  can_see_obj(ch, obj)
			&&  WEIGHT_MULT(obj) == 100
			&&  obj->wear_loc == WEAR_NONE
			&&  obj != container
			&&  can_drop_obj(ch, obj)
			&&  get_obj_weight(obj) + get_true_weight(container) <=
			    INT_VAL(container->value[0]) * 10
			&&  get_obj_weight(obj) < INT_VAL(container->value[3]) * 10
			&&  !put_obj(ch, container, obj, &count))
				break;
		}
	}
}

void do_drop(CHAR_DATA * ch, const char *argument)
{
	char            arg[MAX_INPUT_LENGTH];
	OBJ_DATA       *obj;
	OBJ_DATA       *obj_next;
	bool            found;

	argument = one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		char_puts("Drop what?\n", ch);
		return;
	}
	if (is_number(arg)) {
		/* 'drop NNNN coins' */
		int             amount, gold = 0, silver = 0;
		amount = atoi(arg);
		argument = one_argument(argument, arg, sizeof(arg));
		if (amount <= 0
		    || (str_cmp(arg, "coins") && str_cmp(arg, "coin")
			&& str_cmp(arg, "gold") && str_cmp(arg, "silver"))) {
			char_puts("You can't do that.\n", ch);
			return;
		}
		if (!str_cmp(arg, "coins") || !str_cmp(arg, "coin")
		    || !str_cmp(arg, "silver")) {
			if (ch->silver < amount) {
				char_puts("You don't have that much silver.\n", ch);
				return;
			}
			ch->silver -= amount;
			silver = amount;
		} else {
			if (ch->gold < amount) {
				char_puts("You don't have that much gold.\n", ch);
				return;
			}
			ch->gold -= amount;
			gold = amount;
		}

		for (obj = ch->in_room->contents; obj != NULL; obj = obj_next) {
			obj_next = obj->next_content;

			switch (obj->pObjIndex->vnum) {
			case OBJ_VNUM_SILVER_ONE:
			case OBJ_VNUM_GOLD_ONE:
			case OBJ_VNUM_COINS:
			case OBJ_VNUM_GOLD_SOME:
			case OBJ_VNUM_SILVER_SOME:
				silver += INT_VAL(obj->value[0]);
				gold += INT_VAL(obj->value[1]);
				extract_obj(obj, 0);
				break;
			}
		}

		obj = create_money(gold, silver);
		obj_to_room(obj, ch->in_room);
		act("$n drops some coins.", ch, NULL, NULL,
		    TO_ROOM | (IS_AFFECTED(ch, AFF_SNEAK) ? ACT_NOMORTAL : 0));
		char_puts("Ok.\n", ch);
		if (IS_WATER(ch->in_room)) {
			extract_obj(obj, 0);
			act("The coins sink down, and disapear in the water.",
			    ch, NULL, NULL,
			    TO_ROOM | (IS_AFFECTED(ch, AFF_SNEAK) ? ACT_NOMORTAL : 0));
			act_puts("The coins sink down, and disapear in the water.",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		}
		return;
	}
	if (str_cmp(arg, "all") && str_prefix("all.", arg)) {
		/* 'drop obj' */
		if ((obj = get_obj_carry(ch, arg)) == NULL) {
			char_puts("You do not have that item.\n", ch);
			return;
		}
		if (!can_drop_obj(ch, obj)) {
			char_puts("You can't let go of it.\n", ch);
			return;
		}
		drop_obj(ch, obj);
	}
	else {
/* 'drop all' or 'drop all.obj' */

		if (!IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM)) {
			do_say(ch, "Nah, I won't do that.");
			return;
		}

		found = FALSE;
		for (obj = ch->carrying; obj != NULL; obj = obj_next) {
			obj_next = obj->next_content;

			if ((arg[3] == '\0' || is_name(&arg[4], obj->name))
			&&  can_see_obj(ch, obj)
			&&  obj->wear_loc == WEAR_NONE
			&&  can_drop_obj(ch, obj)) {
				found = TRUE;
				drop_obj(ch, obj);
			}
		}

		if (!found) {
			if (arg[3] == '\0')
				act_puts("You are not carrying anything.",
					 ch, NULL, arg, TO_CHAR, POS_DEAD);
			else
				act_puts("You are not carrying any $T.",
					 ch, NULL, &arg[4], TO_CHAR, POS_DEAD);
		}
	}
}

void do_give(CHAR_DATA * ch, const char *argument)
{
	char            arg[MAX_INPUT_LENGTH];
	CHAR_DATA      *victim;
	OBJ_DATA       *obj;
	int carry_w, carry_n;

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		char_puts("Give what to whom?\n", ch);
		return;
	}

	if (is_number(arg)) {
		/* 'give NNNN coins victim' */
		int amount;
		bool silver;
		int carry_w;

		amount = atoi(arg);

		argument = one_argument(argument, arg, sizeof(arg));
		if (arg[0] == '\0') {
			do_give(ch, str_empty);
			return;
		}

		if (amount <= 0
		||  (str_cmp(arg, "coins") && str_cmp(arg, "coin") &&
		     str_cmp(arg, "gold") && str_cmp(arg, "silver"))) {
			char_puts("Sorry, you can't do that.\n", ch);
			return;
		}

		silver = str_cmp(arg, "gold");

		argument = one_argument(argument, arg, sizeof(arg));
		if (!str_cmp(arg, "to"))
			argument = one_argument(argument, arg, sizeof(arg));
		if (arg[0] == '\0') {
			do_give(ch, str_empty);
			return;
		}

		if ((victim = get_char_room(ch, arg)) == NULL) {
			char_puts("They aren't here.\n", ch);
			return;
		}

		if ((!silver && ch->gold < amount)
		||  (silver && ch->silver < amount)) {
			char_puts("You haven't got that much.\n", ch);
			return;
		}

		if ((carry_w = can_carry_w(victim)) >= 0
		&&  get_carry_weight(victim) +
		    COINS_WEIGHT(silver ? 0 : amount, silver ? amount : 0) >
								carry_w) {
			act("$N can't carry that much weight.",
			    ch, NULL, victim, TO_CHAR);
			return;
		}

		if (silver) {
			ch->silver -= amount;
			victim->silver += amount;
		}
		else {
			ch->gold -= amount;
			victim->gold += amount;
		}

		act_puts3("$n gives you $J $t.",
			  ch, silver ? "silver" : "gold", victim,
			  (const void*) amount,
			  TO_VICT, POS_RESTING);
		act("$n gives $N some coins.", ch, NULL, victim, TO_NOTVICT);
		act_puts3("You give $N $J $t.",
			  ch, silver ? "silver" : "gold", victim,
			  (const void*) amount,
			  TO_CHAR, POS_DEAD);

		/*
		 * Bribe trigger
		 */
		if (IS_NPC(victim) && HAS_TRIGGER(victim, TRIG_BRIBE))
			mp_bribe_trigger(victim, ch,
					 silver ? amount : amount * 100);

		if (IS_NPC(victim)
		&&  IS_SET(victim->pMobIndex->act, ACT_CHANGER)) {
			int             change;
			change = (silver ? 95 * amount / 100 / 100
				  : 95 * amount);


			if (!silver && change > victim->silver)
				victim->silver += change;

			if (silver && change > victim->gold)
				victim->gold += change;

			if (change < 1 && can_see(victim, ch)) {
				do_tell_raw(victim, ch, "I'm sorry, you did not give me enough to change.");
				dofun("give", victim, "%d %s %s",
				      amount, silver ? "silver" : "gold",
				      ch->name);
			}
			else if (can_see(victim, ch)) {
				dofun("give", victim, "%d %s %s",
				      change, silver ? "gold" : "silver",
				      ch->name);
				if (silver)
					dofun("give", victim,
					      "%d silver %s",
					      (95 * amount / 100 - change * 100), ch->name);
				do_tell_raw(victim, ch,
					    "Thank you, come again.");
			}
		}
		return;
	}

	if ((obj = get_obj_carry(ch, arg)) == NULL) {
		char_puts("You do not have that item.\n", ch);
		return;
	}

	argument = one_argument(argument, arg, sizeof(arg));
	if (!str_cmp(arg, "to"))
		argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		do_give(ch, str_empty);
		return;
	}

	if (obj->wear_loc != WEAR_NONE) {
		char_puts("You must remove it first.\n", ch);
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	if (IS_NPC(victim) && victim->pMobIndex->pShop != NULL
	&&  !HAS_TRIGGER(victim, TRIG_GIVE)) {
		do_tell_raw(victim, ch, "Sorry, you'll have to sell that.");
		return;
	}

	if (!can_drop_obj(ch, obj)) {
		char_puts("You can't let go of it.\n", ch);
		return;
	}

	if ((carry_n = can_carry_n(victim)) >= 0
	&&  victim->carry_number + get_obj_number(obj) > carry_n) {
		act("$N has $S hands full.", ch, NULL, victim, TO_CHAR);
		return;
	}

	if ((carry_w = can_carry_w(victim)) >= 0
	&&  get_carry_weight(victim) + get_obj_weight(obj) > carry_w) {
		act("$N can't carry that much weight.",
		    ch, NULL, victim, TO_CHAR);
		return;
	}

	if (IS_SET(obj->pObjIndex->extra_flags, ITEM_QUEST)
	&&  !IS_IMMORTAL(ch) && !IS_IMMORTAL(victim)) {
		act_puts("Even you are not that silly to give $p to $N.",
			 ch, obj, victim, TO_CHAR, POS_DEAD);
		return;
	}

	if (!can_see_obj(victim, obj)) {
		act("$N can't see it.", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (obj->pObjIndex->limit != -1) {
		if ((IS_OBJ_STAT(obj, ITEM_ANTI_EVIL) && IS_EVIL(victim))
		||  (IS_OBJ_STAT(obj, ITEM_ANTI_GOOD) && IS_GOOD(victim))
		||  (IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(victim))) {
			char_puts("Your victim's alignment doesn't match the objects align.", ch);
			return;
		}
	}

	obj_from_char(obj);
	obj_to_char(obj, victim);
	act("$n gives $p to $N.", ch, obj, victim, TO_NOTVICT | ACT_NOTRIG);
	act("$n gives you $p.", ch, obj, victim, TO_VICT | ACT_NOTRIG);
	act("You give $p to $N.", ch, obj, victim, TO_CHAR | ACT_NOTRIG);

	/*
	 * Give trigger
	*/
	if (IS_NPC(victim) && HAS_TRIGGER(victim, TRIG_GIVE))
		mp_give_trigger(victim, ch, obj);

	oprog_call(OPROG_GIVE, obj, ch, victim);
}

/* for poisoning weapons and food/drink */
void do_envenom(CHAR_DATA * ch, const char *argument)
{
	OBJ_DATA       *obj;
	AFFECT_DATA     af;
	int		percent, chance;

	/* find out what */
	if (argument == '\0') {
		char_puts("Envenom what item?\n", ch);
		return;
	}
	obj = get_obj_list(ch, argument, ch->carrying);

	if (obj == NULL) {
		char_puts("You don't have that item.\n", ch);
		return;
	}

	if ((chance = get_skill(ch, "envenom")) == 0) {
		char_puts("Are you crazy? You'd poison yourself!\n", ch);
		return;
	}

	if (obj->pObjIndex->item_type == ITEM_FOOD || obj->pObjIndex->item_type == ITEM_DRINK_CON) {
		if (IS_OBJ_STAT(obj, ITEM_BLESS)
		||  IS_OBJ_STAT(obj, ITEM_BURN_PROOF)) {
			act("You fail to poison $p.", ch, obj, NULL, TO_CHAR);
			return;
		}

		WAIT_STATE(ch, skill_beats("envenom"));
		if (number_percent() < chance) {	/* success! */
			act("$n treats $p with deadly poison.",
			    ch, obj, NULL, TO_ROOM);
			act("You treat $p with deadly poison.",
			    ch, obj, NULL, TO_CHAR);
			if (!INT_VAL(obj->value[3])) {
				INT_VAL(obj->value[3]) = 1;
				check_improve(ch, "envenom", TRUE, 4);
			}
			return;
		}
		act("You fail to poison $p.", ch, obj, NULL, TO_CHAR);
		if (!INT_VAL(obj->value[3]))
			check_improve(ch, "envenom", FALSE, 4);
		return;
	} else if (obj->pObjIndex->item_type == ITEM_WEAPON) {
		if (IS_WEAPON_STAT(obj, WEAPON_FLAMING)
		||  IS_WEAPON_STAT(obj, WEAPON_FROST)
		||  IS_WEAPON_STAT(obj, WEAPON_VAMPIRIC)
		||  IS_WEAPON_STAT(obj, WEAPON_SHARP)
		||  IS_WEAPON_STAT(obj, WEAPON_VORPAL)
		||  IS_WEAPON_STAT(obj, WEAPON_SHOCKING)) chance/=2;

		if (IS_WEAPON_STAT(obj, WEAPON_HOLY)
		||  IS_OBJ_STAT(obj, ITEM_BLESS)
		||  IS_OBJ_STAT(obj, ITEM_BURN_PROOF)) {
			act("You can't seem to envenom $p.",
			    ch, obj, NULL, TO_CHAR);
			return;
		}

		if (damtype_class(obj->value[3].s) == DAM_BASH) {
			char_puts("You can only envenom edged weapons.\n",
				  ch);
			return;
		}

		if (IS_WEAPON_STAT(obj, WEAPON_POISON)) {
			act("$p is already envenomed.", ch, obj, NULL, TO_CHAR);
			return;
		}

		WAIT_STATE(ch, skill_beats("envenom"));
		percent = number_percent();
		if (percent < chance) {
			af.where = TO_WEAPON;
			af.type = "poison";
			af.level = LEVEL(ch) * percent / 100;
			af.duration = LEVEL(ch) * percent / 100;
			af.location = 0;
			af.modifier = 0;
			af.bitvector = WEAPON_POISON;
			affect_to_obj(obj, &af);

			act("$n coats $p with deadly venom.", ch, obj, NULL,
			    TO_ROOM | (IS_AFFECTED(ch, AFF_SNEAK) ? ACT_NOMORTAL : 0));
			act("You coat $p with venom.", ch, obj, NULL, TO_CHAR);
			check_improve(ch, "envenom", TRUE, 3);
			return;
		}
		else {
			act("You fail to envenom $p.", ch, obj, NULL, TO_CHAR);
			check_improve(ch, "envenom", FALSE, 3);
			return;
		}
	}
	act("You can't poison $p.", ch, obj, NULL, TO_CHAR);
}

void do_feed(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *vch;
	OBJ_DATA *obj;
	AFFECT_DATA *paf;
	AFFECT_DATA af;

	if (get_skill(ch, "bone dragon") == 0) {
		char_puts("Huh?\n", ch);
		return;
	}

	if (is_affected(ch, "bone dragon")) {
		act("Your pet might get too fat and clumsy.", 
		    ch, NULL, NULL, TO_CHAR);
		return;
	}

	for (vch = ch->in_room->people; vch; vch = vch->next_in_room) {
		if (vch->master == ch && IS_NPC(vch)
		&&  (vch->pMobIndex->vnum == MOB_VNUM_COCOON ||
		     vch->pMobIndex->vnum == MOB_VNUM_BONE_DRAGON))
			break;
	}

	if (!vch) {
		act("Hmmm. Where did that dragon go?", ch, NULL, NULL, TO_CHAR);
		return;
	}

	for (obj = ch->carrying; obj; obj = obj->next_content)
		if (obj->pObjIndex->item_type == ITEM_CORPSE_NPC
		||  obj->pObjIndex->item_type == ITEM_CORPSE_PC)
			break;

	if (!obj) {
		act("Are you going to feed $M with blueberries?",
		    ch, NULL, vch, TO_CHAR);
		return;
	}

	if (vch->pMobIndex->vnum == MOB_VNUM_BONE_DRAGON) {
		int what;

		if (vch->position < POS_RESTING)
			return;

		if (vch->position == POS_FIGHTING) {
			do_say(vch, "Tasty. But let's finish that fight "
				    "before meal!");
			return;
		}

		what = number_percent();
		act("$n hungrily devours $P", vch, NULL, obj, TO_ROOM);
		af.type		= "bone dragon";
		af.level	= obj->level;
		af.modifier	= 0;
		af.location	= 0;

		if (what < 10) {
			af.where	= TO_IMMUNE;
			af.bitvector	= IMM_MENTAL; 
			af.duration	= obj->level/5; 
			do_emote(vch, "looks clever!");
		}
		else if (what < 40) {
			af.where	= TO_AFFECTS;
			af.bitvector	= AFF_HASTE; 
			af.duration	= obj->level/3; 
			do_emote(vch, "looks filled with energy!");
		}
		else if (what < 70) {
			af.where	= TO_AFFECTS;
			af.bitvector	= AFF_SLEEP;
			af.duration	= obj->level/20; 
			vch->position	= POS_SLEEPING;
			do_emote(vch, "yawns and goes to sleep.");
		}
		else {
			af.where	= TO_RESIST;
			af.bitvector	= RES_MAGIC | RES_WEAPON;
			af.duration	= 10; 
			do_emote(vch, "flexes his muscles, looking tough.");
		}

		affect_to_char(vch, &af);

		af.where	= TO_AFFECTS;
		af.bitvector	= 0;
		af.location	= APPLY_NONE;
		af.duration	= 3;
		affect_to_char(ch, &af);
		extract_obj(obj, 0);
		return;
	}

	for (paf = vch->affected; paf; paf = paf->next)
		if (paf->type == "bone dragon")
			break;

	if (!paf) {
		bug("Feed: bone dragon w/o affect", 0);
		return;
	}

	paf->level += obj->level;

	act("Cocoon opens, devours $P and then closes again.",
	    vch, NULL,obj,TO_ROOM);

	af.where	= TO_AFFECTS;
	af.type		= "bone dragon";
	af.level	= ch->level;
	af.duration	= 2;
	af.modifier	= 0;
	af.bitvector	= 0;
	af.location	= APPLY_NONE;
	affect_to_char(ch, &af);
	extract_obj(obj, 0);
}

void do_fill(CHAR_DATA * ch, const char *argument)
{
	char            arg[MAX_INPUT_LENGTH];
	OBJ_DATA       *obj;
	OBJ_DATA       *fountain;
	bool            found;
	liquid_t	*lq;

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		char_puts("Fill what?\n", ch);
		return;
	}
	if ((obj = get_obj_carry(ch, arg)) == NULL) {
		char_puts("You do not have that item.\n", ch);
		return;
	}
	found = FALSE;
	for (fountain = ch->in_room->contents; fountain != NULL;
	     fountain = fountain->next_content) {
		if (fountain->pObjIndex->item_type == ITEM_FOUNTAIN) {
			found = TRUE;
			break;
		}
	}

	if (!found) {
		char_puts("There is no fountain here!\n", ch);
		return;
	}

	if (obj->pObjIndex->item_type != ITEM_DRINK_CON) {
		char_puts("You can't fill that.\n", ch);
		return;
	}

	if (INT_VAL(obj->value[1]) != 0
	&&  str_cmp(STR_VAL(obj->value[2]), STR_VAL(fountain->value[2]))) {
		char_puts("There is already another liquid in it.\n", ch);
		return;
	}

	if (INT_VAL(obj->value[1]) >= INT_VAL(obj->value[0])
	&&  INT_VAL(obj->value[0]) >= 0) {
		char_puts("Your container is full.\n", ch);
		return;
	}
        if ((lq = liquid_lookup(STR_VAL(fountain->value[2]))) == NULL) {
		bug("Unknown liquid: %s", STR_VAL(fountain->value[2]));
		return;
	}
	act_puts3("You fill $p with $U from $P.",
		   ch, obj, fountain, lq->name, TO_CHAR, POS_DEAD);
	act_puts3("$n fills $p with $U from $P.",
		   ch, obj, fountain, lq->name, TO_ROOM, POS_RESTING);
	STR_VAL_ASSIGN(obj->value[2], str_qdup(fountain->value[2].s));
	INT_VAL(obj->value[1]) = INT_VAL(obj->value[0]);

}

void do_pour(CHAR_DATA * ch, const char *argument)
{
	char            arg[MAX_STRING_LENGTH];
	OBJ_DATA       *out, *in;
	CHAR_DATA      *vch = NULL;
	int             amount;
	liquid_t	*lq;
	argument = one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0' || argument[0] == '\0') {
		char_puts("Pour what into what?\n", ch);
		return;
	}
	if ((out = get_obj_carry(ch, arg)) == NULL) {
		char_puts("You don't have that item.\n", ch);
		return;
	}
	if (out->pObjIndex->item_type != ITEM_DRINK_CON) {
		char_puts("That's not a drink container.\n", ch);
		return;
	}
        if ((lq = liquid_lookup(STR_VAL(out->value[2]))) == NULL) {
		bug("Unknown liquid: %s", STR_VAL(out->value[2]));
		return;
	}
	if (!str_cmp(argument, "out")) {
		if (INT_VAL(out->value[1]) == 0) {
			char_puts("It is empty.\n", ch);
			return;
		}
		INT_VAL(out->value[1]) = 0;
		INT_VAL(out->value[3]) = 0;
		act_puts3("You invert $p, spilling $T $U.",
			  ch, out, lq->name,
			  IS_WATER(ch->in_room) ? "in to the water" :
						  "all over the ground",
			  TO_CHAR, POS_DEAD);
		act_puts3("$n inverts $p, spilling $T $U.",
			  ch, out, lq->name,
			  IS_WATER(ch->in_room) ? "in to the water" :
						  "all over the ground",
			  TO_ROOM, POS_RESTING);
		return;
	}
	if ((in = get_obj_here(ch, argument)) == NULL) {
		vch = get_char_room(ch, argument);

		if (vch == NULL) {
			char_puts("Pour into what?\n", ch);
			return;
		}
		in = get_eq_char(vch, WEAR_HOLD);

		if (in == NULL) {
			char_puts("They aren't holding anything.", ch);
			return;
		}
	}
	if (in->pObjIndex->item_type != ITEM_DRINK_CON) {
		char_puts("You can only pour into other drink containers.\n", ch);
		return;
	}
	if (in == out) {
		char_puts("You cannot change the laws of physics!\n", ch);
		return;
	}
	if (INT_VAL(in->value[1]) != 0
	&& str_cmp(STR_VAL(in->value[2]), STR_VAL(out->value[2]))) {
		char_puts("They don't hold the same liquid.\n", ch);
		return;
	}
	if (INT_VAL(out->value[1]) == 0) {
		act("There's nothing in $p to pour.", ch, out, NULL, TO_CHAR);
		return;
	}
	if (INT_VAL(in->value[0]) < 0) {
		act("You cannot fill $p.", ch, in, NULL, TO_CHAR);
		return;
	}
	if (INT_VAL(in->value[1]) >= INT_VAL(in->value[0])) {
		act("$p is already filled to the top.", ch, in, NULL, TO_CHAR);
		return;
	}
	amount = UMIN(INT_VAL(out->value[1]), INT_VAL(in->value[0]) - INT_VAL(in->value[1]));

	INT_VAL(in->value[1]) += amount;
	if (INT_VAL(out->value[0]) > 0)
		INT_VAL(out->value[1]) -= amount;
	INT_VAL(in->value[2]) = INT_VAL(out->value[2]);

	if (vch == NULL) {
		act_puts3("You pour $U from $p into $P.",
			  ch, out, in, lq->name, TO_CHAR, POS_DEAD);
		act_puts3("$n pours $U from $p into $P.",
			  ch, out, in, lq->name, TO_ROOM, POS_RESTING);
	}
	else {
		act_puts3("You pour some $U for $N.",
			  ch, NULL, vch, lq->name, TO_CHAR, POS_DEAD);
		act_puts3("$n pours you some $U.",
			  ch, NULL, vch, lq->name, TO_VICT, POS_RESTING);
		act_puts3("$n pours some $U for $N.",
			  ch, NULL, vch, lq->name, TO_NOTVICT, POS_RESTING);
	}

}

void do_drink(CHAR_DATA * ch, const char *argument)
{
	char            arg[MAX_INPUT_LENGTH];
	OBJ_DATA       *obj;
	int             amount;
	int		i;
	liquid_t	*lq;

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		for (obj = ch->in_room->contents; obj; obj= obj->next_content) {
			if (obj->pObjIndex->item_type == ITEM_FOUNTAIN)
				break;
		}

		if (obj == NULL) {
			char_puts("Drink what?\n", ch);
			return;
		}
	} else {
		if ((obj = get_obj_here(ch, arg)) == NULL) {
			char_puts("You can't find it.\n", ch);
			return;
		}
	}

	if (!IS_NPC(ch) && PC(ch)->condition[COND_DRUNK] > 10) {
		char_puts("You fail to reach your mouth.  *Hic*\n", ch);
		return;
	}
	if ((lq = liquid_lookup(STR_VAL(obj->value[2]))) == NULL) {
		bug("Do_drink: bad liquid %s.", STR_VAL(obj->value[2]));
		return;
	}
	switch (obj->pObjIndex->item_type) {
	default:
		char_puts("You can't drink from that.\n", ch);
		return;

	case ITEM_FOUNTAIN:
		amount = 3;
		break;

	case ITEM_DRINK_CON:
		if (INT_VAL(obj->value[1]) == 0) {
			char_puts("It is empty.\n", ch);
			return;
		}
		amount = 1;
		break;
	}
	if (!IS_NPC(ch) && !IS_IMMORTAL(ch)
	    && PC(ch)->condition[COND_FULL] > 80) {
		char_puts("You're too full to drink more.\n", ch);
		return;
	}
	act("$n drinks $T from $p.", ch, obj, lq->name, TO_ROOM);
	act_puts("You drink $T from $p.", ch, obj, lq->name, TO_CHAR, POS_DEAD);

	if (ch->fighting)
		WAIT_STATE(ch, 3 * PULSE_VIOLENCE);

	for (i = 0; i < MAX_COND; i++)
		gain_condition(ch, i, lq->affect[i]);

	if (!IS_NPC(ch) && PC(ch)->condition[COND_DRUNK] > 10)
		char_puts("You feel drunk.\n", ch);
	if (!IS_NPC(ch) && PC(ch)->condition[COND_FULL] > 60)
		char_puts("You are full.\n", ch);
	if (!IS_NPC(ch) && PC(ch)->condition[COND_THIRST] > 60)
		char_puts("Your thirst is quenched.\n", ch);

	if (INT_VAL(obj->value[3]) != 0) {
		/* The drink was poisoned ! */
		AFFECT_DATA     af;

		act("$n chokes and gags.", ch, NULL, NULL, TO_ROOM);
		char_puts("You choke and gag.\n", ch);
		af.where = TO_AFFECTS;
		af.type = "poison";
		af.level = number_fuzzy(lq->sip);
		af.duration = 3 * amount;
		af.location = APPLY_NONE;
		af.modifier = 0;
		af.bitvector = AFF_POISON;
		af.events    = EVENT_CHAR_UPDATE;
		affect_join(ch, &af);
	}
	if (INT_VAL(obj->value[0]) > 0)
		INT_VAL(obj->value[1]) = UMAX(INT_VAL(obj->value[1]) - lq->sip, 0);
}

void do_eat(CHAR_DATA * ch, const char *argument)
{
	char            arg[MAX_INPUT_LENGTH];
	OBJ_DATA       *obj;

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		char_puts("Eat what?\n", ch);
		return;
	}
	if ((obj = get_obj_carry(ch, arg)) == NULL) {
		char_puts("You do not have that item.\n", ch);
		return;
	}
	if (!IS_IMMORTAL(ch)) {
		if ((obj->pObjIndex->item_type != ITEM_FOOD ||
		     IS_SET(obj->extra_flags, ITEM_NOT_EDIBLE))
		&& obj->pObjIndex->item_type != ITEM_PILL) {
			char_puts("That's not edible.\n", ch);
			return;
		}
		if (!IS_NPC(ch) && PC(ch)->condition[COND_FULL] > 80) {
			char_puts("You are too full to eat more.\n", ch);
			return;
		}
	}
	act("$n eats $p.", ch, obj, NULL, TO_ROOM);
	act_puts("You eat $p.", ch, obj, NULL, TO_CHAR, POS_DEAD);
	if (ch->fighting != NULL)
		WAIT_STATE(ch, 3 * PULSE_VIOLENCE);

	switch (obj->pObjIndex->item_type) {

	case ITEM_FOOD:
		if (!IS_NPC(ch)) {
			int             condition;
			condition = PC(ch)->condition[COND_HUNGER];
			gain_condition(ch, COND_FULL, INT_VAL(obj->value[0]) * 2);
			gain_condition(ch, COND_HUNGER, INT_VAL(obj->value[1]) * 2);
			if (!condition
			&& PC(ch)->condition[COND_HUNGER] > 0)
				char_puts("You are no longer hungry.\n", ch);
			else if (PC(ch)->condition[COND_FULL] > 60)
				char_puts("You are full.\n", ch);
			if (obj->pObjIndex->vnum == OBJ_VNUM_TORN_HEART) {
				ch->hit = UMIN(ch->max_hit, ch->hit+obj->level+number_range(1,5));
				char_puts("You feel empowered from the blood of your foe.\n",ch);
			};
		}
		if (INT_VAL(obj->value[3]) != 0) {
			/* The food was poisoned! */
			AFFECT_DATA     af;

			act("$n chokes and gags.", ch, NULL, NULL, TO_ROOM);
			char_puts("You choke and gag.\n", ch);

			af.where = TO_AFFECTS;
			af.type = "poison";
			af.level = number_fuzzy(INT_VAL(obj->value[0]));
			af.duration = 2 * INT_VAL(obj->value[0]);
			af.location = APPLY_NONE;
			af.modifier = 0;
			af.bitvector = AFF_POISON;
			af.events    = EVENT_CHAR_UPDATE;
			affect_join(ch, &af);
		}
		if (IS_OBJ_STAT(obj, ITEM_MAGIC)) {
			char_puts("Magical heat flows through your blood.\n", ch);
			ch->mana = UMIN(ch->max_mana, ch->mana + obj->level + number_range(1, 5));
			if (number_percent() < 2) {
				AFFECT_DATA af;

				do_yell(ch, "Oh no! My eyes!");

				af.where = TO_AFFECTS;
				af.type = "blindness";
				af.level = number_fuzzy(obj->level);
				af.duration = obj->level / 25 + 1;
				af.location = APPLY_HITROLL;
				af.modifier = -4;
				af.bitvector = AFF_BLIND;
				affect_join(ch, &af);
			}
		}
		break;

	case ITEM_PILL:
		obj_cast_spell(obj->value[1].s, INT_VAL(obj->value[0]), ch, ch);
		obj_cast_spell(obj->value[2].s, INT_VAL(obj->value[0]), ch, ch);
		obj_cast_spell(obj->value[3].s, INT_VAL(obj->value[0]), ch, ch);
		obj_cast_spell(obj->value[4].s, INT_VAL(obj->value[0]), ch, ch);
		break;
	}

	extract_obj(obj, 0);
}

void do_wear(CHAR_DATA * ch, const char *argument)
{
	char            arg[MAX_INPUT_LENGTH];
	OBJ_DATA       *obj;
	OBJ_DATA       *obj_next;
	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		char_puts("Wear, wield, or hold what?\n", ch);
		return;
	}
	if (!str_cmp(arg, "all"))
		for (obj = ch->carrying; obj != NULL; obj = obj_next) {
			obj_next = obj->next_content;
			if (obj->wear_loc == WEAR_NONE && can_see_obj(ch, obj))
				wear_obj(ch, obj, FALSE);
		}
	else if ((obj = get_obj_carry(ch, arg)) == NULL) {
		char_puts("You do not have that item.\n", ch);
		return;
	} else
		wear_obj(ch, obj, TRUE);
}

void do_remove(CHAR_DATA * ch, const char *argument)
{
	char            arg[MAX_INPUT_LENGTH];
	OBJ_DATA       *obj;
	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		char_puts("Remove what?\n", ch);
		return;
	}
	if (!str_cmp(arg, "all")) {
		OBJ_DATA       *obj_next;

		if (!IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM)) {
			do_say(ch, "Nah, I won't do that.");
			return;
		}

		for (obj = ch->carrying; obj != NULL; obj = obj_next) {
			obj_next = obj->next_content;
			if (obj->wear_loc != WEAR_NONE && can_see_obj(ch, obj))
				remove_obj(ch, obj->wear_loc, TRUE);
		}
		return;
	}
	if ((obj = get_obj_wear(ch, arg)) == NULL) {
		char_puts("You do not have that item.\n", ch);
		return;
	}
	remove_obj(ch, obj->wear_loc, TRUE);
	return;
}

void do_sacrifice(CHAR_DATA * ch, const char *argument)
{
	char            arg[MAX_INPUT_LENGTH];
	OBJ_DATA       *obj;
	OBJ_DATA       *r_next_cont;

	if (!strcmp(argument, "all")) {
		for (obj = ch->in_room->contents; obj; obj = r_next_cont) {
			r_next_cont = obj->next_content;
			sac_obj(ch, obj);
		}
		return;
	}

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0' || !str_cmp(arg, ch->name)) {
		act("$n offers $mself to gods, who graciously declines.",
		    ch, NULL, NULL, TO_ROOM);
		char_puts("Gods appreciates your offer "
			  "and may accept it later.\n", ch);
		return;
	}

	obj = get_obj_list(ch, arg, ch->in_room->contents);
	if (obj == NULL) {
		char_puts("You can't find it.\n", ch);
		return;
	}

	sac_obj(ch, obj);
}

void do_quaff(CHAR_DATA * ch, const char *argument)
{
	char            arg[MAX_INPUT_LENGTH];
	OBJ_DATA       *obj;
	one_argument(argument, arg, sizeof(arg));
	
	if (HAS_SKILL(ch, "spellbane")) {
		char_puts("You are Battle Rager, not filthy magician!\n",ch);
		return;
	}

	if (arg[0] == '\0') {
		char_puts("Quaff what?\n", ch);
		return;
	}

	if ((obj = get_obj_carry(ch, arg)) == NULL) {
		char_puts("You do not have that potion.\n", ch);
		return;
	}

	if (obj->pObjIndex->item_type != ITEM_POTION) {
		char_puts("You can quaff only potions.\n", ch);
		return;
	}

	if (ch->level < obj->level) {
		char_puts("This liquid is too powerful for you to drink.\n", ch);
		return;
	}
	quaff_obj(ch, obj);
}

void do_recite(CHAR_DATA * ch, const char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	void *vo;
	OBJ_DATA *scroll;

	if (HAS_SKILL(ch, "spellbane")) {
		char_puts ("RECITE? You are Battle Rager!\n", ch);
		return;
	}

	argument = one_argument(argument, arg1, sizeof(arg1));
	argument = one_argument(argument, arg2, sizeof(arg2));

	if ((scroll = get_obj_carry(ch, arg1)) == NULL) {
		char_puts("You do not have that scroll.\n", ch);
		return;
	}

	if (scroll->pObjIndex->item_type != ITEM_SCROLL) {
		char_puts("You can recite only scrolls.\n", ch);
		return;
	}

	if (ch->level < scroll->level) {
		char_puts("This scroll is too complex for you to comprehend.\n", ch);
		return;
	}

	if (arg2[0] == '\0')
		vo = ch;
	else if ((vo = get_char_room(ch, arg2)) == NULL
	&&       (vo = get_obj_here(ch, arg2)) == NULL) {
		WAIT_STATE(ch, MISSING_TARGET_DELAY);
		char_puts("You can't find it.\n", ch);
		return;
	}

	act("$n recites $p.", ch, scroll, NULL, TO_ROOM);
	act_puts("You recite $p.", ch, scroll, NULL, TO_CHAR, POS_DEAD);

	if (number_percent() >= get_skill(ch, "scrolls") * 4 / 5) {
		act_puts("You mispronounce a syllable.",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		check_improve(ch, "scrolls", FALSE, 2);
	} else {
		obj_cast_spell(scroll->value[1].s, INT_VAL(scroll->value[0]), ch, vo);
		obj_cast_spell(scroll->value[2].s, INT_VAL(scroll->value[0]), ch, vo);
		obj_cast_spell(scroll->value[3].s, INT_VAL(scroll->value[0]), ch, vo);
		obj_cast_spell(scroll->value[4].s, INT_VAL(scroll->value[0]), ch, vo);
		check_improve(ch, "scrolls", TRUE, 2);

		if (IS_PUMPED(ch) || ch->fighting != NULL)
			WAIT_STATE(ch, 2 * PULSE_VIOLENCE);
	}

	extract_obj(scroll, 0);
}

void do_brandish(CHAR_DATA * ch, const char *argument)
{
	CHAR_DATA      *vch;
	CHAR_DATA      *vch_next;
	OBJ_DATA       *staff;
	skill_t *	sk;

	if (HAS_SKILL(ch, "spellbane")) {
		char_puts("BRANDISH? You are not a filthy magician!\n",ch);
		return;
	}

	if ((staff = get_eq_char(ch, WEAR_HOLD)) == NULL) {
		char_puts("You hold nothing in your hand.\n", ch);
		return;
	}

	if (staff->pObjIndex->item_type != ITEM_STAFF) {
		char_puts("You can brandish only with a staff.\n", ch);
		return;
	}

	if ((sk = skill_lookup(staff->value[3].s)) == NULL)
		return;

	WAIT_STATE(ch, 2 * PULSE_VIOLENCE);

	if (INT_VAL(staff->value[2]) > 0) {
		act("$n brandishes $p.", ch, staff, NULL, TO_ROOM);
		act("You brandish $p.", ch, staff, NULL, TO_CHAR);
		if (ch->level + 3 < staff->level
		|| number_percent() >= 10 + get_skill(ch, "staves") * 4 / 5) {
			act("You fail to invoke $p.", ch, staff, NULL, TO_CHAR);
			act("...and nothing happens.", ch, NULL, NULL, TO_ROOM);
			check_improve(ch, "staves", FALSE, 2);
		} else {
			if (!sk)
				return;

			for (vch = ch->in_room->people; vch; vch = vch_next) {
				vch_next = vch->next_in_room;

				switch (sk->target) {
				default:
					return;

				case TAR_IGNORE:
					if (vch != ch)
						continue;
					break;

				case TAR_CHAR_OFFENSIVE:
					if (IS_NPC(ch) ? IS_NPC(vch) : !IS_NPC(vch))
						continue;
					break;

				case TAR_CHAR_DEFENSIVE:
					if (IS_NPC(ch) ? !IS_NPC(vch) : IS_NPC(vch))
						continue;
					break;

				case TAR_CHAR_SELF:
					if (vch != ch)
						continue;
					break;
				}

				obj_cast_spell(staff->value[3].s,
					       INT_VAL(staff->value[0]), ch, vch);
				if (IS_SET(sk->skill_flags, SKILL_AREA_ATTACK))
					break;
			}
			check_improve(ch, "staves", TRUE, 2);
		}
	}

	if (--INT_VAL(staff->value[2]) <= 0) {
		act("$n's $p blazes bright and is gone.",
		    ch, staff, NULL, TO_ROOM);
		act("Your $p blazes bright and is gone.",
		    ch, staff, NULL, TO_CHAR);
		extract_obj(staff, 0);
	}
}

void do_zap(CHAR_DATA * ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *wand;
	void *vo;
	
	if (HAS_SKILL(ch, "spellbane")) {
		char_puts("You'd destroy magic, not use it!\n",ch);
		return;
	}

	one_argument(argument, arg, sizeof(arg));
	if ((wand = get_eq_char(ch, WEAR_HOLD)) == NULL) {
		char_puts("You hold nothing in your hand.\n", ch);
		return;
	}
	if (wand->pObjIndex->item_type != ITEM_WAND) {
		char_puts("You can zap only with a wand.\n", ch);
		return;
	}

	if (arg[0] == '\0') {
		if (ch->fighting && ch->fighting->in_room == ch->in_room)
			vo = ch->fighting;
		else {
			char_puts("Zap whom or what?\n", ch);
			return;
		}
	} else if ((vo = get_char_room(ch, arg)) == NULL
	&&       (vo = get_obj_here(ch, arg)) == NULL) {
		WAIT_STATE(ch, MISSING_TARGET_DELAY);
		char_puts("You can't find it.\n", ch);
		return;
	}

	WAIT_STATE(ch, 2 * PULSE_VIOLENCE);

	if (INT_VAL(wand->value[2]) > 0) {
		if (mem_is(vo, MT_CHAR)) {
			if (vo == ch) {
				act("$n zaps $mself with $p.",
				    ch, wand, vo, TO_ROOM);
				act("You zap yourself with $p.",
				    ch, wand, vo, TO_CHAR);
			} else {
				act("$n zaps $N with $p.",
				    ch, wand, vo, TO_NOTVICT);
				act("$n zaps you with $p.",
				    ch, wand, vo, TO_VICT);
				act("You zap $N with $p.",
				    ch, wand, vo, TO_CHAR);
			}
		} else {
			act("$n zaps $P with $p.", ch, wand, vo, TO_ROOM);
			act("You zap $P with $p.", ch, wand, vo, TO_CHAR);
		}

		if (ch->level + 5 < wand->level
		|| number_percent() >= 20 + get_skill(ch, "wands") * 4 / 5) {
			act("Your efforts with $p produce only smoke and sparks.",
			    ch, wand, NULL, TO_CHAR);
			act("$n's efforts with $p produce only smoke and sparks.",
			    ch, wand, NULL, TO_ROOM);
			check_improve(ch, "wands", FALSE, 2);
		} else {
			obj_cast_spell(wand->value[3].s, INT_VAL(wand->value[0]), ch, vo);
			check_improve(ch, "wands", TRUE, 2);
		}
	}
	if (--INT_VAL(wand->value[2]) <= 0) {
		act("$n's $p explodes into fragments.",
		    ch, wand, NULL, TO_ROOM);
		act("Your $p explodes into fragments.",
		    ch, wand, NULL, TO_CHAR);
		extract_obj(wand, 0);
	}
}

void do_steal(CHAR_DATA * ch, const char *argument)
{
	char            arg1[MAX_INPUT_LENGTH];
	char            arg2[MAX_INPUT_LENGTH];
	CHAR_DATA      *victim;
	OBJ_DATA       *obj;
	OBJ_DATA       *obj_inve;
	int             percent;
	int		carry_n, carry_w;
	
	argument = one_argument(argument, arg1, sizeof(arg1));
	argument = one_argument(argument, arg2, sizeof(arg2));

	if (arg1[0] == '\0' || arg2[0] == '\0') {
		char_puts("Steal what from whom?\n", ch);
		return;
	}

	if (IS_NPC(ch) && IS_SET(ch->affected_by, AFF_CHARM)
	    && (ch->master != NULL)) {
		char_puts("You are too dazed to steal anything.\n", ch);
		return;
	}

	WAIT_STATE(ch, skill_beats("steal"));

	if ((victim = get_char_room(ch, arg2)) == NULL) {
		WAIT_STATE(ch, MISSING_TARGET_DELAY);
		char_puts("They aren't here.\n", ch);
		return;
	}
	if (victim == ch) {
		char_puts("That's pointless.\n", ch);
		return;
	}

	if (victim->position == POS_FIGHTING) {
		char_puts("You'd better not -- you might get hit.\n", ch);
		return;
	}

	if (is_safe(ch, victim))
		return;
	
	percent = number_percent() +
		  (IS_AWAKE(victim) ? 10 : -50) +
		  (!can_see(victim, ch) ? -10 : 0);

	if ((!IS_NPC(ch) && percent > get_skill(ch, "steal"))
	||  IS_SET(victim->imm_flags, IMM_STEAL)
	||  IS_IMMORTAL(victim)
	||  (victim->in_room &&
	     IS_SET(victim->in_room->room_flags, ROOM_BATTLE_ARENA))) {
		/*
		 * Failure.
		 */

		char_puts("Oops.\n", ch);

		if (IS_AFFECTED(ch, AFF_HIDE | AFF_FADE) && !IS_NPC(ch)) {
			REMOVE_BIT(ch->affected_by, AFF_HIDE | AFF_FADE);
			act_puts("You step out of shadows.",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
			act("$n steps out of shadows.",
			    ch, NULL, NULL, TO_ROOM);
        	}

		if (!IS_AFFECTED(victim, AFF_SLEEP)) {
			victim->position = victim->position == POS_SLEEPING ? POS_STANDING :
				victim->position;
			act("$n tried to steal from you.\n", ch, NULL, victim, TO_VICT);
		}
		act("$n tried to steal from $N.\n", ch, NULL, victim, TO_NOTVICT);

		if (IS_AWAKE(victim)) {
			const char *msg;

			switch (number_range(0, 3)) {
			case 0:
				msg = "$i is a lousy thief!";
				break;
			case 1:
				msg = "$i couldn't rob $gi{his} way "
				      "out of a paper bag!";
				break;
			case 2:
				msg = "$i tried to rob me!";
				break;
			default:
				msg = "Keep your hands out of there, $i!";
				break;
			}

			act_yell(victim, msg, ch, NULL);
		}

		if (!IS_NPC(ch) && IS_NPC(victim)) {
			check_improve(ch, "steal", FALSE, 2);
			multi_hit(victim, ch, NULL);
		}
		return;
	}
	if (!str_cmp(arg1, "coin")
	    || !str_cmp(arg1, "coins")
	    || !str_cmp(arg1, "silver")
	    || !str_cmp(arg1, "gold")) {
		int             amount_s = 0;
		int             amount_g = 0;
		if (!str_cmp(arg1, "silver") ||
		    !str_cmp(arg1, "coin") ||
		    !str_cmp(arg1, "coins"))
			amount_s = victim->silver * number_range(1, 20) / 100;
		else if (!str_cmp(arg1, "gold"))
			amount_g = victim->gold * number_range(1, 7) / 100;

		if (amount_s <= 0 && amount_g <= 0) {
			char_puts("You couldn't get any coins.\n", ch);
			return;
		}
		ch->gold += amount_g;
		victim->gold -= amount_g;
		ch->silver += amount_s;
		victim->silver -= amount_s;
		char_printf(ch, "Bingo!  You got %d %s coins.\n",
			    amount_s != 0 ? amount_s : amount_g,
			    amount_s != 0 ? "silver" : "gold");
		check_improve(ch, "steal", TRUE, 2);
		return;
	}
	if ((obj = get_obj_carry(victim, arg1)) == NULL) {
		char_puts("You can't find it.\n", ch);
		return;
	}
	if (!can_drop_obj(ch, obj)
	/* ||   IS_SET(obj->extra_flags, ITEM_INVENTORY) */
	     /* ||  obj->level > ch->level */ ) {
		char_puts("You can't pry it away.\n", ch);
		return;
	}

	if ((carry_n = can_carry_n(ch)) >= 0
	&&  ch->carry_number + get_obj_number(obj) > carry_n) {
		char_puts("You have your hands full.\n", ch);
		return;
	}

	if ((carry_w = can_carry_w(ch)) >= 0
	&&  ch->carry_weight + get_obj_weight(obj) > carry_w) {
		char_puts("You can't carry that much weight.\n", ch);
		return;
	}

	if (!IS_SET(obj->extra_flags, ITEM_INVENTORY)) {
		obj_from_char(obj);
		obj_to_char(obj, ch);
		char_puts("You got it!\n", ch);
		check_improve(ch, "steal", TRUE, 2);
	} else {
		obj_inve = NULL;
		obj_inve = create_obj(obj->pObjIndex, 0);
		clone_obj(obj, obj_inve);
		REMOVE_BIT(obj_inve->extra_flags, ITEM_INVENTORY);
		obj_to_char(obj_inve, ch);
		char_puts("You got one of them!\n", ch);
		check_improve(ch, "steal", TRUE, 1);
	}
	oprog_call(OPROG_GET, obj, ch, NULL);
}

/*
 * Shopping commands.
 */
void do_buy_pet(CHAR_DATA * ch, const char *argument)
{
	int		cost, roll;
	char            arg[MAX_INPUT_LENGTH];
	CHAR_DATA	*pet;
	flag64_t		act;
	ROOM_INDEX_DATA *pRoomIndexNext;
	ROOM_INDEX_DATA *in_room;

	if (IS_NPC(ch))
		return;

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		char_puts("Buy what?\n", ch);
		return;
	}

	pRoomIndexNext = get_room_index(ch->in_room->vnum + 1);
	if (pRoomIndexNext == NULL) {
		bug("Do_buy: bad pet shop at vnum %d.", ch->in_room->vnum);
		char_puts("Sorry, you can't buy that here.\n", ch);
		return;
	}
	in_room = ch->in_room;
	ch->in_room = pRoomIndexNext;
	pet = get_char_room(ch, arg);
	ch->in_room = in_room;

	if (!pet
	||  !IS_NPC(pet)
	||  !IS_SET(act = pet->pMobIndex->act, ACT_PET)) {
		char_puts("Sorry, you can't buy that here.\n", ch);
		return;
	}

	
	if (IS_SET(act, ACT_RIDEABLE)
	&&  get_skill(ch, "riding") && !MOUNTED(ch)) {
		cost = 10 * pet->level * pet->level;

		if ((ch->silver + 100 * ch->gold) < cost) {
			char_puts("You can't afford it.\n", ch);
			return;
		}
		if (ch->level < pet->level + 5) {
			char_puts("You're not powerful enough "
				     "to master this pet.\n", ch);
			return;
		}
		deduct_cost(ch, cost);
		pet = create_mob(pet->pMobIndex);
		pet->comm = COMM_NOTELL | COMM_NOSHOUT | COMM_NOCHANNELS;

		char_to_room(pet, ch->in_room);
		if (IS_EXTRACTED(pet))
			return;

		do_mount(ch, pet->name);
		char_puts("Enjoy your mount.\n", ch);
		act("$n bought $N as a mount.", ch, NULL, pet, TO_ROOM);
		return;
	}
	if (GET_PET(ch) != NULL) {
		char_puts("You already own a pet.\n", ch);
		return;
	}
	cost = 10 * pet->level * pet->level;

	if ((ch->silver + 100 * ch->gold) < cost) {
		char_puts("You can't afford it.\n", ch);
		return;
	}
	if (LEVEL(ch) < LEVEL(pet)) {
		char_puts("You're not powerful enough "
			     "to master this pet.\n", ch);
		return;
	}
	/* haggle */
	roll = number_percent();
	if (roll < get_skill(ch, "haggle")) {
		cost -= cost / 2 * roll / 100;
		char_printf(ch, "You haggle the price down to %d coins.\n",
			    cost);
		check_improve(ch, "haggle", TRUE, 4);
	}
	deduct_cost(ch, cost);
	pet = create_mob(pet->pMobIndex);
	SET_BIT(pet->affected_by, AFF_CHARM);
	pet->comm = COMM_NOTELL | COMM_NOSHOUT | COMM_NOCHANNELS;

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] != '\0')
		pet->name = str_printf(pet->pMobIndex->name, arg);
	mlstr_printf(&pet->description,
		     &pet->pMobIndex->description, ch->name);

	char_to_room(pet, ch->in_room);
	if (IS_EXTRACTED(pet))
		return;

	add_follower(pet, ch);
	pet->leader = ch;
	PC(ch)->pet = pet;
	char_puts("Enjoy your pet.\n", ch);
	act("$n bought $N as a pet.", ch, NULL, pet, TO_ROOM);
}

void do_buy(CHAR_DATA * ch, const char *argument)
{
	int		cost, roll;
	CHAR_DATA      *keeper;
	OBJ_DATA       *obj, *t_obj;
	char            arg[MAX_INPUT_LENGTH];
	uint		number, count = 1;
	int		carry_w, carry_n;

	if ((keeper = find_keeper(ch)) == NULL)
		return;

	if (IS_SET(ch->in_room->room_flags, ROOM_PET_SHOP)) {
		do_buy_pet(ch, argument);
		return;
	}

	number = mult_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0' || number == 0) {
		char_puts("Buy what?\n", ch);
		return;
	}

	if (number < 0) {
		char_puts("What? Try sell instead.\n", ch);
		return;
	}

	obj = get_obj_keeper(ch, keeper, arg);
	cost = get_cost(keeper, obj, TRUE);

	if (cost <= 0 || !can_see_obj(ch, obj)) {
		do_tell_raw(keeper, ch, "I don't sell that -- try 'list'");
		return;
	}
	if (!IS_OBJ_STAT(obj, ITEM_INVENTORY)) {
		for (t_obj = obj->next_content; count < number && t_obj != NULL;
		     t_obj = t_obj->next_content) {
			if (t_obj->pObjIndex == obj->pObjIndex
			&& !mlstr_cmp(&t_obj->short_descr, &obj->short_descr))
				count++;
			else
				break;
		}

		if (count < number) {
			do_tell_raw(keeper, ch,
				    "I don't have that many in stock.'");
			return;
		}
	}
	if ((ch->silver + ch->gold * 100) < cost * number) {
		if (number > 1) {
			do_tell_raw(keeper, ch,
				    "You can't afford to buy that many.'");
		} else {
			act("$n tells you '{GYou can't afford to buy $p.{x'",
			    keeper, obj, ch, TO_VICT);
		}
		return;
	}
	if (obj->level > ch->level) {
		act("$n tells you '{GYou can't use $p yet.{x'",
		    keeper, obj, ch, TO_VICT);
		return;
	}

	if ((carry_n = can_carry_n(ch)) >= 0
	&&  ch->carry_number + number * get_obj_number(obj) > carry_n) {
		char_puts("You can't carry that many items.\n", ch);
		return;
	}

	if ((carry_w = can_carry_w(ch)) >= 0
	&&  ch->carry_weight + number * get_obj_weight(obj) > carry_w) {
		char_puts("You can't carry that much weight.\n", ch);
		return;
	}

	/* haggle */
	roll = number_percent();
	if (!IS_OBJ_STAT(obj, ITEM_SELL_EXTRACT)
	    && roll < get_skill(ch, "haggle")) {
		cost -= obj->cost / 2 * roll / 100;
		act("You haggle with $N.", ch, NULL, keeper, TO_CHAR);
		check_improve(ch, "haggle", TRUE, 4);
	}
	if (number > 1) {
		act_puts("$n buys $P[$j].",
			 ch, (const void*) number, obj, TO_ROOM, POS_RESTING);
		act_puts3("You buy $P[$j] for $J silver.",
			  ch, (const void*) number,
			  obj, (const void*) (cost*number),
			  TO_CHAR, POS_DEAD);
	} else {
		act("$n buys $P.", ch, NULL, obj, TO_ROOM);
		act_puts("You buy $P for $j silver.",
			 ch, (const void*) cost, obj, TO_CHAR, POS_DEAD);
	}

	deduct_cost(ch, cost * number);
	keeper->gold += cost * number / 100;
	keeper->silver += cost * number - (cost * number / 100) * 100;

	for (count = 0; count < number; count++) {
		if (IS_SET(obj->extra_flags, ITEM_INVENTORY))
			t_obj = create_obj(obj->pObjIndex, 0);
		else {
			t_obj = obj;
			obj = obj->next_content;
			obj_from_char(t_obj);
		}

		if (t_obj->timer > 0 && !IS_OBJ_STAT(t_obj, ITEM_HAD_TIMER))
			t_obj->timer = 0;
		REMOVE_BIT(t_obj->extra_flags, ITEM_HAD_TIMER);
		obj_to_char(t_obj, ch);
		if (cost < t_obj->cost)
			t_obj->cost = cost;
	}
}

void do_list(CHAR_DATA * ch, const char *argument)
{
	if (IS_SET(ch->in_room->room_flags, ROOM_PET_SHOP)) {
		ROOM_INDEX_DATA *pRoomIndexNext;
		CHAR_DATA      *pet;
		bool            found;

		pRoomIndexNext = get_room_index(ch->in_room->vnum + 1);
		if (pRoomIndexNext == NULL) {
			bug("Do_list: bad pet shop at vnum %d.", ch->in_room->vnum);
			char_puts("You can't do that here.\n", ch);
			return;
		}
		found = FALSE;
		for (pet = pRoomIndexNext->people; pet; pet = pet->next_in_room) {
			if (!IS_NPC(pet))
				continue;	/* :) */
			if (IS_SET(pet->pMobIndex->act, ACT_PET)) {
				if (!found) {
					found = TRUE;
					char_puts("Pets for sale:\n", ch);
				}
				char_printf(ch, "[%2d] %8d - %s\n",
					    pet->level,
					    10 * pet->level * pet->level,
					    PERS2(pet, ch, ACT_FORMSH));
			}
		}
		if (!found)
			char_puts("Sorry, we're out of pets right now.\n", ch);
		return;
	} else {
		CHAR_DATA      *keeper;
		OBJ_DATA       *obj;
		int             cost, count;
		bool            found;
		char            arg[MAX_INPUT_LENGTH];
		if ((keeper = find_keeper(ch)) == NULL)
			return;
		one_argument(argument, arg, sizeof(arg));

		found = FALSE;
		for (obj = keeper->carrying; obj; obj = obj->next_content) {
			if (obj->wear_loc == WEAR_NONE
			    && can_see_obj(ch, obj)
			    && (cost = get_cost(keeper, obj, TRUE)) > 0
			    && (arg[0] == '\0'
				|| is_name(arg, obj->name))) {
				if (!found) {
					found = TRUE;
					char_puts("[Lv Price Qty] Item\n", ch);
				}
				if (IS_OBJ_STAT(obj, ITEM_INVENTORY))
					char_printf(ch, "[%2d %5d -- ] %s\n",
						obj->level, cost,
						format_short(&obj->short_descr,
							     obj->name, ch, 0));
				else {
					count = 1;

					while (obj->next_content != NULL
					       && obj->pObjIndex == obj->next_content->pObjIndex
					       && !mlstr_cmp(&obj->short_descr,
					   &obj->next_content->short_descr)) {
						obj = obj->next_content;
						count++;
					}
					char_printf(ch, "[%2d %5d %2d ] %s\n",
						obj->level, cost, count,
						format_short(&obj->short_descr,
							     obj->name, ch, 0));
				}
			}
		}

		if (!found)
			char_puts("You can't buy anything here.\n", ch);
		return;
	}
}

void do_sell(CHAR_DATA * ch, const char *argument)
{
	char            arg[MAX_INPUT_LENGTH];
	CHAR_DATA      *keeper;
	OBJ_DATA       *obj;
	int		cost, roll;
	uint		gold, silver;

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		char_puts("Sell what?\n", ch);
		return;
	}
	if ((keeper = find_keeper(ch)) == NULL)
		return;

	if ((obj = get_obj_carry(ch, arg)) == NULL) {
		do_tell_raw(keeper, ch, "You don't have that item.");
		return;
	}
	if (!can_drop_obj(ch, obj)) {
		char_puts("You can't let go of it.\n", ch);
		return;
	}
	if (!can_see_obj(keeper, obj)) {
		act("$n doesn't see what you are offering.", keeper, NULL, ch, TO_VICT);
		return;
	}
	if ((cost = get_cost(keeper, obj, FALSE)) <= 0) {
		act("$n looks uninterested in $p.", keeper, obj, ch, TO_VICT);
		return;
	}
	if (cost > (keeper->silver + 100 * keeper->gold)) {
		act("$n tells you '{GI'm afraid I don't have enough wealth to buy $p.{x'",
		    keeper, obj, ch, TO_VICT);
		return;
	}
	act("$n sells $p.", ch, obj, NULL, TO_ROOM);
	/* haggle */
	roll = number_percent();
	if (!IS_OBJ_STAT(obj, ITEM_SELL_EXTRACT) && roll < get_skill(ch, "haggle")) {
		roll = get_skill(ch, "haggle") + number_range(1, 20) - 10;
		char_puts("You haggle with the shopkeeper.\n", ch);
		cost += obj->cost / 2 * roll / 100;
		cost = UMIN(cost, 95 * get_cost(keeper, obj, TRUE) / 100);
		cost = UMIN(cost, (keeper->silver + 100 * keeper->gold));
		check_improve(ch, "haggle", TRUE, 4);
	}
	silver = cost - (cost / 100) * 100;
	gold = cost / 100;

	if (gold && silver) {
		act_puts3("You sell $P for $j gold and $J silver $qJ{pieces}.",
			  ch, (const void*) gold, obj, (const void*) silver,
			  TO_CHAR, POS_DEAD);
	}
	else if (gold) {
		act_puts("You sell $P for $j gold $qj{pieces}.",
			 ch, (const void*) gold, obj, TO_CHAR, POS_DEAD);
	}
	else if (silver) {
		act_puts("You sell $P for $j silver $qj{pieces}.",
			 ch, (const void*) silver, obj, TO_CHAR, POS_DEAD);
	}
	ch->gold += gold;
	ch->silver += silver;
	deduct_cost(keeper, cost);

	if (obj->pObjIndex->item_type == ITEM_TRASH
	||  IS_OBJ_STAT(obj, ITEM_SELL_EXTRACT))
		extract_obj(obj, 0);
	else {
		obj_from_char(obj);
		if (obj->timer)
			SET_BIT(obj->extra_flags, ITEM_HAD_TIMER);
		else
			obj->timer = number_range(50, 100);
		obj_to_keeper(obj, keeper);
	}
}

void do_value(CHAR_DATA * ch, const char *argument)
{
	char            arg[MAX_INPUT_LENGTH];
	CHAR_DATA      *keeper;
	OBJ_DATA       *obj;
	int             cost;
	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		char_puts("Value what?\n", ch);
		return;
	}
	if ((keeper = find_keeper(ch)) == NULL)
		return;

	if ((obj = get_obj_carry(ch, arg)) == NULL) {
		do_tell_raw(keeper, ch, "You don't have that item.");
		return;
	}
	if (!can_see_obj(keeper, obj)) {
		act("$n doesn't see what you are offering.", keeper, NULL, ch, TO_VICT);
		return;
	}
	if (!can_drop_obj(ch, obj)) {
		char_puts("You can't let go of it.\n", ch);
		return;
	}
	if ((cost = get_cost(keeper, obj, FALSE)) <= 0) {
		act("$n looks uninterested in $p.", keeper, obj, ch, TO_VICT);
		return;
	}

	act_puts("$N tells you '{G", ch, NULL, keeper,
		 TO_CHAR | ACT_NOLF, POS_DEAD);
	act_puts3("I'll give you $j silver and $J gold $qJ{coins} for $P.",
		  ch, (const void*) (cost%100), obj, (const void*) (cost/100),
		  TO_CHAR | ACT_NOLF, POS_DEAD);
	act_puts("{x'", ch, NULL, NULL, TO_CHAR, POS_DEAD);
}

void do_herbs(CHAR_DATA * ch, const char *argument)
{
	CHAR_DATA      *victim;
	char            arg[MAX_INPUT_LENGTH];

	one_argument(argument, arg, sizeof(arg));

	if (!IS_NPC(ch) && is_affected(ch, "herbs")) {
		char_puts("You can't find any more herbs.\n", ch);
		return;
	}

	WAIT_STATE(ch, skill_beats("herbs"));

	if (arg[0] == '\0')
		victim = ch;
	else if ((victim = get_char_room(ch, arg)) == NULL) {
		char_puts("They're not here.\n", ch);
		return;
	}

	if ((ch->in_room->sector_type != SECT_INSIDE
	  &&  ch->in_room->sector_type != SECT_CITY
	  &&  number_percent() < get_skill(ch, "herbs"))
	|| (IS_NPC(ch) && IS_SET(ch->pMobIndex->act, ACT_HEALER))) {
		AFFECT_DATA     af;
		af.where = TO_AFFECTS;
		af.type = "herbs";
		af.level = ch->level;
		af.duration = 5;
		af.location = APPLY_NONE;
		af.modifier = 0;
		af.bitvector = 0;

		if (!IS_NPC(ch)) affect_to_char(ch, &af);

		char_puts("You gather some beneficial herbs.\n", ch);
		act("$n gathers some herbs.", ch, NULL, NULL, TO_ROOM);

		if (ch != victim) {
			act("$n gives you some herbs to eat.", ch, NULL, victim, TO_VICT);
			act("You give the herbs to $N.", ch, NULL, victim, TO_CHAR);
			act("$n gives the herbs to $N.", ch, NULL, victim, TO_NOTVICT);
		}
		if (victim->hit < victim->max_hit) {
			char_puts("You feel better.\n", victim);
			act("$n looks better.", victim, NULL, NULL, TO_ROOM);
		}
		victim->hit = UMIN(victim->max_hit, victim->hit + 5 * LEVEL(ch));
		check_improve(ch, "herbs", TRUE, 1);
		if (is_affected(victim, "plague"))
			if (check_dispel(LEVEL(ch), victim, "plague")) {
				act("$n looks relieved as $s sores vanish.",
				    victim, NULL, NULL, TO_ROOM);
			}
		if (is_affected(victim, "poison"))
			if (check_dispel(LEVEL(ch), victim, "poison")) {
				act("$n does not look so green anymore.",
				    victim, NULL, NULL, TO_ROOM);
			}
	} else {
		char_puts("You search for herbs but find none here.\n", ch);
		act("$n looks around for herbs.", ch, NULL, NULL, TO_ROOM);
		check_improve(ch, "herbs", FALSE, 1);
	}
}

static const char *
random_spell(void)
{
	skill_t *sk;

	for (;;) {
		sk = (skill_t*) hash_random_item(&skills);
		if (sk == NULL)
			return "agle-bargle";
		if (sk->skill_type == ST_SPELL
		||  sk->skill_type == ST_PRAYER)
			break;
	}

	return sk->name;
}

void do_lore_raw(CHAR_DATA *ch, OBJ_DATA *obj, BUFFER *output)
{
	int chance;
	int percent;
	vo_t v0, v1, v2, v3, v4;
	int mana;

	if ((percent = get_skill(ch, "lore")) < 10) {
		buf_add(output, "The meaning of this object escapes you for the moment.\n");
		return;
	}

	mana = skill_mana(ch, "lore");
	if (ch->mana < mana) {
		buf_add(output, "You don't have enough mana.\n");
		return;
	}
	ch->mana -= mana;
	WAIT_STATE(ch, skill_beats("beats"));

	/* a random lore */
	chance = number_percent();

	if (percent < 20) {
		buf_printf(output, "Object '%s'.\n", obj->name);
		check_improve(ch, "lore", TRUE, 8);
		return;
	} else if (percent < 40) {
		buf_printf(output,
			    "Object '%s'.  Weight is %d, value is %d.\n",
			    obj->name,
		chance < 60 ? obj->weight : number_range(1, 2 * obj->weight),
		     chance < 60 ? number_range(1, 2 * obj->cost) : obj->cost
			);
		if (str_cmp(obj->material, "oldstyle"))
			buf_printf(output, "Material is %s.\n", obj->material);
		check_improve(ch, "lore", TRUE, 7);
		return;
	} else if (percent < 60) {
		buf_printf(output,
			    "Object '%s' has weight %d.\nValue is %d, level is %d.\nMaterial is %s.\n",
			    obj->name,
			    obj->weight,
		    chance < 60 ? number_range(1, 2 * obj->cost) : obj->cost,
		  chance < 60 ? obj->level : number_range(1, 2 * obj->level),
		str_cmp(obj->material, "oldstyle") ? obj->material : "unknown"
			);
		check_improve(ch, "lore", TRUE, 6);
		return;
	} else if (percent < 80) {
		buf_printf(output,
			    "Object '%s' is type %s, extra flags %s.\nWeight is %d, value is %d, level is %d.\nMaterial is %s.\n",
			    obj->name,
			    flag_string(item_types, obj->pObjIndex->item_type),
			    flag_string(extra_flags, obj->extra_flags),
			    obj->weight,
		    chance < 60 ? number_range(1, 2 * obj->cost) : obj->cost,
		  chance < 60 ? obj->level : number_range(1, 2 * obj->level),
		str_cmp(obj->material, "oldstyle") ? obj->material : "unknown"
			);
		check_improve(ch, "lore", TRUE, 5);
		return;
	} else if (percent < 85) 
		buf_printf(output,
			    "Object '%s' is type %s, extra flags %s.\nWeight is %d, value is %d, level is %d.\nMaterial is %s.\n",
			    obj->name,
			    flag_string(item_types, obj->pObjIndex->item_type),
			    flag_string(extra_flags, obj->extra_flags),
			    obj->weight,
			    obj->cost,
			    obj->level,
			    str_cmp(obj->material, "oldstyle") ?
				obj->material : "unknown"
			);
	else
		buf_printf(output,
			    "Object '%s' is type %s, extra flags %s.\nWeight is %d, value is %d, level is %d.\nMaterial is %s.\n",
			    obj->name,
			    flag_string(item_types, obj->pObjIndex->item_type),
			    flag_string(extra_flags, obj->extra_flags),
			    obj->weight,
			    obj->cost,
			    obj->level,
			    str_cmp(obj->material, "oldstyle") ?
				obj->material : "unknown"
			);

	v0 = obj->value[0];
	v1 = obj->value[1];
	v2 = obj->value[2];
	v3 = obj->value[3];
	v4 = obj->value[4];

	switch (obj->pObjIndex->item_type) {
	case ITEM_SCROLL:
	case ITEM_POTION:
	case ITEM_PILL:
		if (percent < 85) {
			INT_VAL(v0) = number_range(1, 60);
			if (chance > 40) {
				v1.s = random_spell();
				if (chance > 60) {
					v2.s = random_spell();
					if (chance > 80)
						v3.s = random_spell();
				}
			}
		} else {
			if (chance > 60) {
				v1.s = random_spell();
				if (chance > 80) {
					v2.s = random_spell();
					if (chance > 95)
						v3.s = random_spell();
				}
			}
		}

		buf_printf(output, "Level %d spells of:", INT_VAL(v0));
		if (!IS_NULLSTR(v1.s))
			buf_printf(output, " '%s'", v1.s);
		if (!IS_NULLSTR(v2.s))
			buf_printf(output, " '%s'", v2.s);
		if (!IS_NULLSTR(v3.s))
			buf_printf(output, " '%s'", v3.s);
		if (!IS_NULLSTR(v4.s))
			buf_printf(output, " '%s'", v4.s);
		buf_add(output, ".\n");
		break;

	case ITEM_WAND:
	case ITEM_STAFF:
		if (percent < 85) {
			INT_VAL(v0) = number_range(1, 60);
			if (chance > 40) {
				v3.s = random_spell();
				if (chance > 60) {
					INT_VAL(v2) = number_range(0, 2 * INT_VAL(v2));
					if (chance > 80)
						INT_VAL(v1) = number_range(0, INT_VAL(v2));
				}
			}
		} else {
			if (chance > 60) {
				v3.s = random_spell();
				if (chance > 80) {
					INT_VAL(v2) = number_range(0, 2 * INT_VAL(v2));
					if (chance > 95)
						INT_VAL(v1) = number_range(0, INT_VAL(v2));
				}
			}
		}

		buf_printf(output, "Has %d(%d) charges of level %d '%s'.\n",
			    INT_VAL(v1), INT_VAL(v2), INT_VAL(v0), v3.s);
		break;

	case ITEM_WEAPON:
		buf_add(output, "Weapon type is ");
		if (percent < 85) {
			INT_VAL(v0) = number_range(0, 8);
			if (chance > 33) {
				INT_VAL(v1) = number_range(1, 2 * INT_VAL(v1));
				if (chance > 66)
					INT_VAL(v2) = number_range(1, 2 * INT_VAL(v2));
			}
		} else {
			if (chance > 50) {
				INT_VAL(v1) = number_range(1, 2 * INT_VAL(v1));
				if (chance > 75)
					INT_VAL(v2) = number_range(1, 2 * INT_VAL(v2));
			}
		}

		buf_printf(output, "%s.\n", flag_string(weapon_class, INT_VAL(v0)));

		buf_printf(output, "Damage is %dd%d (average %d).\n",
			    INT_VAL(v1), INT_VAL(v2), (1 + INT_VAL(v2)) * INT_VAL(v1) / 2);
		break;

	case ITEM_ARMOR:
		if (percent < 85) {
			if (chance > 25) {
				INT_VAL(v2) = number_range(0, 2 * INT_VAL(v2));
				if (chance > 45) {
					INT_VAL(v0) = number_range(0, 2 * INT_VAL(v0));
					if (chance > 65) {
						INT_VAL(v3) = number_range(0, 2 * INT_VAL(v3));
						if (chance > 85)
							INT_VAL(v1) = number_range(0, 2 * INT_VAL(v1));
					}
				}
			}
		} else {
			if (chance > 45) {
				INT_VAL(v2) = number_range(0, 2 * INT_VAL(v2));
				if (chance > 65) {
					INT_VAL(v0) = number_range(0, 2 * INT_VAL(v0));
					if (chance > 85) {
						INT_VAL(v3) = number_range(0, 2 * INT_VAL(v3));
						if (chance > 95)
							INT_VAL(v1) = number_range(0, 2 * INT_VAL(v1));
					}
				}
			}
		}

		buf_printf(output,
			    "Armor class is %d pierce, %d bash, %d slash, and %d vs. magic.\n",
			    INT_VAL(v0), INT_VAL(v1), INT_VAL(v2), INT_VAL(v3));
		break;
	}

	if (percent < 87) {
		check_improve(ch, "lore", FALSE, 5);
		return;
	}

	if (!IS_SET(obj->extra_flags, ITEM_ENCHANTED))
		format_obj_affects(output, obj->pObjIndex->affected,
				   FOA_F_NODURATION);
	format_obj_affects(output, obj->affected, 0);
	check_improve(ch, "lore", TRUE, 5);
}

void do_lore(CHAR_DATA *ch, const char *argument)
{
	char		arg[MAX_INPUT_LENGTH];
	BUFFER *	output;
	OBJ_DATA *	obj;

	argument = one_argument(argument, arg, sizeof(arg));
	if ((obj = get_obj_carry(ch, arg)) == NULL) {
		char_puts("You do not have that object.\n", ch);
		return;
	}

	output = buf_new(-1);
	do_lore_raw(ch, obj, output);
	page_to_char(buf_string(output), ch);
	buf_free(output);
}

void do_butcher(CHAR_DATA * ch, const char *argument)
{
	OBJ_DATA       *obj;
	char            arg[MAX_STRING_LENGTH];
	OBJ_DATA       *tmp_obj;
	OBJ_DATA       *tmp_next;
	int		chance;

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		char_puts("Butcher what?\n", ch);
		return;
	}
	if ((obj = get_obj_here(ch, arg)) == NULL) {
		char_puts("You do not see that here.\n", ch);
		return;
	}
	if (obj->pObjIndex->item_type != ITEM_CORPSE_PC
	&&  obj->pObjIndex->item_type != ITEM_CORPSE_NPC) {
		char_puts("You can't butcher that.\n", ch);
		return;
	}
	if (obj->carried_by != NULL) {
		char_puts("Put it down first.\n", ch);
		return;
	}

	if ((chance = get_skill(ch, "butcher")) == 0) {
		char_puts("You don't have the instruments for that.", ch);
		return;
	}

	obj_from_room(obj);

	for (tmp_obj = obj->contains; tmp_obj != NULL; tmp_obj = tmp_next) {
		tmp_next = tmp_obj->next_content;
		obj_from_obj(tmp_obj);
		obj_to_room(tmp_obj, ch->in_room);
	}

	if (number_percent() < chance) {
		int             numsteaks;
		int             i;
		OBJ_DATA       *steak;
		numsteaks = number_bits(2) + 1;

		act("$n butchers $P and creates $j $qj{steaks}.",
		    ch, (const void*) numsteaks, obj, TO_ROOM);
		act_puts("You butcher $P and create $j $qj{steaks}.",
			 ch, (const void*) numsteaks, obj,
			 TO_CHAR, POS_DEAD);
		check_improve(ch, "butcher", TRUE, 1);

		for (i = 0; i < numsteaks; i++) {
			steak = create_obj_of(get_obj_index(OBJ_VNUM_STEAK),
					      &obj->short_descr);
			obj_to_room(steak, ch->in_room);
		}
	} else {
		act("You fail and destroy $p.", ch, obj, NULL, TO_CHAR);
		act("$n fails to butcher $p and destroys it.",
		    ch, obj, NULL, TO_ROOM);

		check_improve(ch, "butcher", FALSE, 1);
	}
	extract_obj(obj, 0);
}

void do_crucify(CHAR_DATA *ch, const char *argument)
{	
	OBJ_DATA *obj;
	char arg[MAX_STRING_LENGTH];
	OBJ_DATA *obj2, *next;
	OBJ_DATA *cross;
	int chance;
	AFFECT_DATA af;

	if ((chance = get_skill(ch, "crucify")) == 0) {
		char_puts("Oh no, you can't do that.\n", ch);
		return;
	}

	if (is_affected(ch, "crucify")) {
		char_puts("You are not yet ready to make a sacrifice.\n", ch);
		return;
	}

	if (IS_AFFECTED(ch, AFF_BERSERK) 
	|| is_affected(ch, "frenzy")) {
		char_puts("Calm down first.\n", ch);
		return;
	}

	one_argument(argument, arg, sizeof(arg));

	if (arg == '\0') {
		char_puts("Crucify what?\n", ch);
		return;
	}

	if ((obj = get_obj_here(ch, arg)) == NULL) {
		char_puts("You do not see that here.", ch);
		return;
	}

	if (obj->pObjIndex->item_type != ITEM_CORPSE_PC
	 && obj->pObjIndex->item_type != ITEM_CORPSE_NPC) {
		char_puts("You cannot crucify that.\n", ch);
		return;
	 }

	if (obj->carried_by != NULL) {
		char_puts("Put it down first.\n", ch);
		return;
	}

	obj_from_room(obj);
	for(obj2 = obj->contains; obj2; obj2 = next) {
		next = obj2->next_content;
		obj_from_obj(obj2);
		obj_to_room(obj2, ch->in_room);
	}
	
	if (number_percent() > chance) {
		act("You attempt a ritual crucification of $p, "
		   "but fail and ruin it.", ch, obj, NULL, TO_CHAR);
		act("$n attempts to crucify $p, but fails and ruins it.",
		    ch, obj, NULL, TO_ROOM);
		extract_obj(obj, 0);
		check_improve(ch, "crucify", FALSE, 1);
	} else {
		cross = create_obj_of(get_obj_index(OBJ_VNUM_CROSS),
				      &obj->owner);
		obj_to_room(cross, ch->in_room);
		act("With a crunch of bone and splash of blood you nail "
		    "$p to a sacrificial cross.", ch, obj, NULL, TO_CHAR);
		act("With a crunch of bone and splash of blood $n nails "
		    "$p to a sacrificial cross.", ch, obj, NULL, TO_ROOM);
		char_puts("You are filled with a dark energy.\n", ch);
		ch->hit += obj->level*3/2;
		af.where 	= TO_AFFECTS;
		af.type  	= "crucify";
		af.level	= ch->level;
		af.duration	= 15;
		af.modifier	= UMAX(1, obj->level/8);
		af.bitvector	= AFF_BERSERK;

		af.location	= APPLY_HITROLL;
		affect_to_char(ch, &af);

		af.location	= APPLY_DAMROLL;
		affect_to_char(ch, &af);

		af.modifier 	= UMAX(10, obj->level);
		af.location	= APPLY_AC;

		affect_to_char(ch, &af);
		extract_obj(obj, 0);
		check_improve(ch, "crucify", TRUE, 1);
	}
}

void do_balance(CHAR_DATA * ch, const char *argument)
{
	char buf[160];
	int bank_g;
	int bank_s;

	if (IS_NPC(ch)) {
		char_puts("You don't have a bank account.\n", ch);
		return;
	}
	if (!IS_SET(ch->in_room->room_flags, ROOM_BANK)) {
		char_puts("You are not in a bank.\n", ch);
		return;
	}

	if (PC(ch)->bank_s + PC(ch)->bank_g == 0) {
		char_puts("You don't have any money in the bank.\n", ch);
		return;
	}

	bank_g = PC(ch)->bank_g;
	bank_s = PC(ch)->bank_s;
	snprintf(buf, sizeof(buf), "You have %s%s%s coin%s in the bank.\n",
		bank_g ? "%ld gold" : str_empty,
		(bank_g) && (bank_s) ? " and " : str_empty,
		bank_s ? "%ld silver" : str_empty,
		bank_s + bank_g > 1 ? "s" : str_empty);
	if (bank_g == 0)
		char_printf(ch, buf, bank_s);
	else
		char_printf(ch, buf, bank_g, bank_s);
}

void do_withdraw(CHAR_DATA * ch, const char *argument)
{
	int	amount;
	int	fee;
	bool	silver = FALSE;
	char	arg[MAX_INPUT_LENGTH];
	int carry_w;

	if (IS_NPC(ch)) {
		char_puts("You don't have a bank account.\n", ch);
		return;
	}

	if (!IS_SET(ch->in_room->room_flags, ROOM_BANK)) {
		char_puts("The mosquito by your feet "
			  "will not give you any money.\n", ch);
		return;
	}

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		char_puts("Withdraw how much?\n", ch);
		return;
	}

	amount = atoi(arg);
	if (amount <= 0) {
		char_puts("What?\n", ch);
		return;
	}

	if (!str_cmp(argument, "silver"))
		silver = TRUE;
	else if (str_cmp(argument, "gold") && argument[0] != '\0') {
		char_puts("You can withdraw gold and silver coins only.", ch);
		return;
	}

	if ((silver && amount > PC(ch)->bank_s)
	||  (!silver && amount > PC(ch)->bank_g)) {
		char_puts("Sorry, we don't give loans.\n", ch);
		return;
	}

	fee = UMAX(1, amount * (silver ? 10 : 2) / 100);
	
	if ((carry_w = can_carry_w(ch)) >= 0
	&&  get_carry_weight(ch) + (amount - fee) * (silver ? 4 : 1) / 10 >
							carry_w) {
		char_puts("You can't carry that weight.\n", ch);
		return;
	}

	if (silver) {
		ch->silver += amount - fee;
		PC(ch)->bank_s -= amount;
	}
	else {
		ch->gold += amount - fee;
		PC(ch)->bank_g -= amount;
	}

	char_printf(ch,
		    "Here are your %d %s coin(s), "
		    "minus a %d coin(s) withdrawal fee.\n",
		    amount, silver ? "silver" : "gold", fee);
	act("$n steps up to the teller window.", ch, NULL, NULL, TO_ROOM);
}

void do_deposit(CHAR_DATA * ch, const char *argument)
{
	int	amount;
	bool	silver = FALSE;
	char	arg[MAX_INPUT_LENGTH];

	if (IS_NPC(ch)) {
		char_puts("You don't have a bank account.\n", ch);
		return;
	}

	if (!IS_SET(ch->in_room->room_flags, ROOM_BANK)) {
		char_puts("The ant by your feet can't carry your gold.\n", ch);
		return;
	}

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		char_puts("Deposit how much?\n", ch);
		return;
	}

	amount = atoi(arg);
	if (amount <= 0) {
		char_puts("What?\n", ch);
		return;
	}

	if (!str_cmp(argument, "silver"))
		silver = TRUE;
	else if (str_cmp(argument, "gold") && argument[0] != '\0') {
		char_puts("You can deposit gold and silver coins only.", ch);
		return;
	}

	if ((silver && amount > ch->silver)
	||  (!silver && amount > ch->gold)) {
		char_puts("That's more than you've got.\n", ch);
		return;
	}

	if (silver) {
		PC(ch)->bank_s += amount;
		ch->silver -= amount;
	}
	else {
		PC(ch)->bank_g += amount;
		ch->gold -= amount;
	}

	if (amount == 1)
		char_printf(ch, "Oh boy! One %s coin!\n",
			    silver ? "silver" : "gold");
	else
		char_printf(ch, "%d %s coins deposited. Come again soon!\n",
			    amount, silver ? "silver" : "gold");
	act("$n steps up to the teller window.", ch, NULL, NULL, TO_ROOM);
}

/* wear object as a secondary weapon */
void do_second_wield(CHAR_DATA * ch, const char *argument)
{
	OBJ_DATA *	obj;
	OBJ_DATA *	wield;
	int		skill;
	int		wear_lev;

	if (get_skill(ch, "dual wield") == 0) {
		char_puts("You don't know how to wield a second weapon.\n", ch);
		return;
	}
	if (argument[0] == '\0') {
		char_puts("Wear which weapon in your off-hand?\n", ch);
		return;
	}
	obj = get_obj_carry(ch, argument);
	if (obj == NULL) {
		char_puts("You don't have that item.\n", ch);
		return;
	}
	if (!CAN_WEAR(obj, ITEM_WIELD)) {
		char_puts("You can't wield that as your second weapon.\n", ch);
		return;
	}
	if ((get_eq_char(ch, WEAR_SHIELD) != NULL) ||
	    (get_eq_char(ch, WEAR_HOLD) != NULL)) {
		char_puts("You cannot use a secondary weapon while using a shield or holding an item.\n", ch);
		return;
	}
	wear_lev = get_wear_level(ch, obj);
	if (wear_lev < obj->level) {
		char_printf(ch, "You must be level %d to use this object.\n",
			    wear_lev);
		act("$n tries to use $p, but is too inexperienced.",
		    ch, obj, NULL, TO_ROOM);
		return;
	}
	if (IS_WEAPON_STAT(obj, WEAPON_TWO_HANDS)) {
		char_puts("You can't dual wield two-handed weapon!\n", ch);
		return;
	}
	if ((wield = get_eq_char(ch, WEAR_WIELD)) == NULL) {
		char_puts("You need to wield a primary weapon, before using a secondary one!\n", ch);
		return;
	}
	if (WEAPON_IS(wield, WEAPON_STAFF)) {
		char_puts("You can't use second weapon while wielding a staff.\n", ch);
		return;
	}
	if (get_obj_weight(obj) > (str_app[get_curr_stat(ch, STAT_STR)].wield * 5)) {
		char_puts("This weapon is too heavy to be used as a secondary weapon by you.\n", ch);
		return;
	}
	if (!remove_obj(ch, WEAR_SECOND_WIELD, TRUE))
		return;

	act("$n wields $p in $s off-hand.", ch, obj, NULL, TO_ROOM);
	act("You wield $p in your off-hand.", ch, obj, NULL, TO_CHAR);
	obj = equip_char(ch, obj, WEAR_SECOND_WIELD);
	if (obj == NULL)
		return;

	skill = get_weapon_skill(ch, get_weapon_sn(obj));

	if (skill >= 100)
		act("$p feels like a part of you!", ch, obj, NULL, TO_CHAR);
	else if (skill > 85)
		act("You feel quite confident with $p.", ch, obj, NULL, TO_CHAR);
	else if (skill > 70)
		act("You are skilled with $p.", ch, obj, NULL, TO_CHAR);
	else if (skill > 50)
		act("Your skill with $p is adequate.", ch, obj, NULL, TO_CHAR);
	else if (skill > 25)
		act("$p feels a little clumsy in your hands.", ch, obj, NULL, TO_CHAR);
	else if (skill > 1)
		act("You fumble and almost drop $p.", ch, obj, NULL, TO_CHAR);
	else
		act("You don't even know which end is up on $p.",
		    ch, obj, NULL, TO_CHAR);
}

void do_enchant(CHAR_DATA * ch, const char *argument)
{
	OBJ_DATA *	obj;
	int		chance;
	int		wear_level;

	if ((chance = get_skill(ch, "enchant sword")) == 0) {
		char_puts("Huh?\n", ch);
		return;
	}

	if (argument[0] == '\0') {	/* empty */
		char_puts("Wear which weapon to enchant?\n", ch);
		return;
	}
	obj = get_obj_carry(ch, argument);

	if (obj == NULL) {
		char_puts("You don't have that item.\n", ch);
		return;
	}

	wear_level = get_wear_level(ch, obj);
	if (wear_level < obj->level) {
		char_printf(ch, "You must be level %d to be able to enchant this object.\n",
			    obj->level - wear_level + ch->level);
		act("$n tries to enchant $p, but is too inexperienced.",
		    ch, obj, NULL, TO_ROOM);
		return;
	}
	if (ch->mana < 100) {
		char_puts("You don't have enough mana.\n", ch);
		return;
	}

	WAIT_STATE(ch, skill_beats("enchant sword"));
	if (number_percent() > chance) {
		char_puts("You lost your concentration.\n", ch);
		act("$n tries to enchant $p, but he forgets how for a moment.",
		    ch, obj, NULL, TO_ROOM);
		check_improve(ch, "enchant sword", FALSE, 6);
		ch->mana -= 50;
		return;
	}
	ch->mana -= 100;
	spellfun_call("enchant weapon", NULL, LEVEL(ch), ch, obj);
	check_improve(ch, "enchant sword", TRUE, 2);
}

void do_label(CHAR_DATA* ch, const char *argument)
{
	OBJ_DATA *obj;
	const char *p;
	char obj_name[MAX_INPUT_LENGTH];
	char label[MAX_INPUT_LENGTH];
	
	if (IS_NPC(ch))
		return;

	argument = one_argument(argument, obj_name, sizeof(obj_name));
	first_arg(argument, label, sizeof(label), FALSE);
	
	if (!obj_name[0]) {
		char_puts("Label what?\n",ch);
		return;
	}

	if (!label[0]) {
		char_puts("How do you want to label it?\n",ch);
		return;
	}

	if ((obj = get_obj_carry(ch, obj_name)) == NULL) {
		char_puts("You don't have that object.\n", ch);
		return;
	}
	
	if (PC(ch)->questpoints < 10) {
		char_puts("You do not have enough questpoints for labeling.\n",
			  ch);
		return;
	}

	if ((strlen(obj->name) + strlen(label) + 2) >= MAX_STRING_LENGTH) {
		char_puts("You can't label this object with this label.\n", ch);
		return;
	}

	p = obj->name;
	obj->name = str_printf("%s %s", obj->name, label);
	free_string(p);
	
	PC(ch)->questpoints -= 10;
	char_puts("Ok.\n",ch);
}

void do_repair(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *mob;
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	int cost;

	for (mob = ch->in_room->people; mob; mob = mob->next_in_room) {
		if (IS_NPC(mob) && IS_SET(mob->pMobIndex->act, ACT_REPAIRMAN))
			break;
	}
 
	if (mob == NULL) {
		char_puts("You can't do that here.\n", ch);
		return;
	}

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
	do_say(mob,"I will repair a weapon for you, for a price.");
	char_puts("Type estimate <weapon> to be assessed for damage.\n",ch);
	return;
	}
	if ((obj = get_obj_carry(ch, arg)) == NULL)
	{
	do_say(mob,"You don't have that item");
	return;
	}

	if (obj->pObjIndex->vnum == OBJ_VNUM_HAMMER)
	{
	 do_say(mob,"That hammer is beyond my power.");
	 return;
	}

	if (obj->condition >= 100) {
	do_say(mob,"But that item is not broken.");
	    return;
	}

	if (obj->cost == 0) {
		act_say(mob, "$p is beyond repair.", obj);
   		return;
	}

	cost = ((obj->level * 10) +
		((obj->cost * (100 - obj->condition)) /100)   );
	cost /= 100;

	if (cost > ch->gold) {
		do_say(mob,"You do not have enough gold for my services.");
		return;
	}

	WAIT_STATE(ch,PULSE_VIOLENCE);

	ch->gold -= cost;
	mob->gold += cost;
	act_puts("$n takes $p from $N, repairs it, and returns it to $N",
		 mob, obj, ch, TO_ROOM, POS_RESTING);
	obj->condition = 100;
}

void do_estimate(CHAR_DATA *ch, const char *argument)
{
	OBJ_DATA *obj;
	CHAR_DATA *mob; 
	char arg[MAX_INPUT_LENGTH];
	int cost;
	
	for (mob = ch->in_room->people; mob; mob = mob->next_in_room) {
		if (IS_NPC(mob) && IS_SET(mob->pMobIndex->act, ACT_REPAIRMAN))
			break;
	}
 
	if (mob == NULL)
	{
	    char_puts("You can't do that here.\n", ch);
	    return;
	}
	
	one_argument(argument, arg, sizeof(arg));
	
	if (arg[0] == '\0')
	{
	do_say(mob,"Try estimate <item>");
   	return; 
	} 
	if ((obj = (get_obj_carry(ch, arg))) == NULL)
	{
	do_say(mob,"You don't have that item");
	return;
	}
	if (obj->pObjIndex->vnum == OBJ_VNUM_HAMMER)
	{
	    do_say(mob,"That hammer is beyond my power.");
	    return;
	}
	if (obj->condition >= 100)
	{
	do_say(mob,"But that item's not broken");
	return;
	}
	if (obj->cost == 0)
	{
	do_say(mob,"That item is beyond repair");
		return;
	} 
	
	cost = ((obj->level * 10) +
		((obj->cost * (100 - obj->condition)) /100)   );
	cost /= 100;

	act_say(mob, "It will cost you $j gold to fix that item.",
		(const void*) cost);
}

void do_restring(CHAR_DATA *ch, const char *argument)
{
#if 0
	CHAR_DATA *mob;
	char arg  [MAX_INPUT_LENGTH];
	char arg1 [MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	int cost = 2000;

	for (mob = ch->in_room->people; mob; mob = mob->next_in_room)
	{
	    if (IS_NPC(mob) && IS_SET(mob->pMobIndex->act, ACT_HEALER))
	        break;
	}
 
	if (mob == NULL) {
#endif
	    char_puts("You can't do that here.\n", ch);
	    return;
#if 0
	/* XXX */
	}

	argument = one_argument(argument, arg, sizeof(arg));
	argument = one_argument(argument, arg1, sizeof(arg1));

	if (arg[0] == '\0' || arg1[0] == '\0' || argument[0] == '\0')
	{
		char_puts("Syntax:\n",ch);
		char_puts("  restring <obj> <field> <string>\n",ch);
		char_puts("    fields: name short long\n",ch);
		return;
	}

	if ((obj = (get_obj_carry(ch, arg))) == NULL) {
		do_say(mob, "You don't have that item.");
		return;
	}

	cost += (obj->level * 1500);

	if (cost > ch->gold) {
		act("$N says 'You do not have enough gold for my services.'",
		  ch,NULL,mob,TO_CHAR);
		return;
	}
	
	if (!str_prefix(arg1, "name")) {
		free_string(obj->name);
		obj->name = str_dup(argument);
	}
	else
	if (!str_prefix(arg1, "short")) {
		free_string(obj->short_descr);
	    obj->short_descr = str_dup(argument);
	}
	else
	if (!str_prefix(arg1, "long")) {
		free_string(obj->description);
		obj->description = str_dup(argument);
	}
	else {
		char_puts("That's not a valid Field.\n",ch);
		return;
	}
	
	WAIT_STATE(ch, PULSE_VIOLENCE);

	ch->gold -= cost;
	mob->gold += cost;
	act("$N takes $n's item, tinkers with it, and returns it to $n.",
	    ch, NULL, mob, TO_ROOM);
	act_puts("$N takes $p, tinkers with it and returns it to you.\n",
		 ch, obj, mob, TO_CHAR, POS_DEAD);
	act_puts("Remember, if we find your new string offensive, "
		 "we will not be happy.",
		 chi, NULL, NULL, TO_CHAR, POS_DEAD);
	act_puts("This is your ONE AND ONLY warning.",
		 ch, NULL, NULL, TO_CHAR, POS_DEAD);
#endif
}

void do_smithing(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	OBJ_DATA *hammer;
	int chance;

	if ((chance = get_skill(ch, "smithing")) == 0) {
		char_puts("Huh?\n", ch);
		return;
	}

	if (ch->fighting) {
		char_puts("Wait until the fight finishes.\n", ch);
		return;
	}

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		char_puts("Which object do you want to repair.\n",ch);
		return;
	}

	if ((obj = get_obj_carry(ch, arg)) == NULL) {
		char_puts("You are not carrying that.\n",ch);
		return;
	}

	if (obj->condition >= 100) {
		char_puts("But that item is not broken.\n",ch);
		return;
	}

	if ((hammer = get_eq_char(ch, WEAR_HOLD)) == NULL) {
		char_puts("You are not holding a hammer.\n",ch);
		return;
	}

	if (hammer->pObjIndex->vnum != OBJ_VNUM_HAMMER) {
		char_puts("That is not the correct hammer.\n",ch);
		return;
	}

	WAIT_STATE(ch, skill_beats("smithing"));
	if (number_percent() > chance) {
		check_improve(ch, "smithing", FALSE, 8);
		act_puts("$n tries to repair $p with the hammer but fails.",
			 ch, obj, NULL, TO_ROOM, POS_RESTING);
		act_puts("You failed to repair $p.",
			 ch, obj, NULL, TO_CHAR, POS_RESTING);
		hammer->condition -= 25;
	}
	else {
		check_improve(ch, "smithing", TRUE, 4);
		act_puts("$n repairs $p with the hammer.",
			 ch, obj, NULL, TO_ROOM, POS_RESTING);
		act_puts("You repair $p.",
			 ch, obj, NULL, TO_CHAR, POS_RESTING);
		obj->condition = UMAX(100, obj->condition + (chance / 2));
		hammer->condition -= 25;
	}

	if (hammer->condition < 1)
		extract_obj(hammer, 0);
}

/*
 * static functions
 */
static CHAR_DATA *find_keeper(CHAR_DATA *ch)
{
	CHAR_DATA      *keeper;
	SHOP_DATA      *pShop = NULL;

	for (keeper = ch->in_room->people; keeper; keeper = keeper->next_in_room) {
		if (IS_NPC(keeper)
		&&  (pShop = keeper->pMobIndex->pShop) != NULL)
			break;
	}

	if (pShop == NULL) {
		char_puts("You can't do that here.\n", ch);
		return NULL;
	}

	if (IS_WANTED(ch)) {
		do_say(keeper, "Criminals are not welcome!");
		act_yell(keeper, "$i the CRIMINAL is over here!", ch, NULL);
		return NULL;
	}

	/*
	 * Shop hours.
	 */
	if (time_info.hour < pShop->open_hour) {
		do_say(keeper, "Sorry, I am closed. Come back later.");
		return NULL;
	}
	if (time_info.hour > pShop->close_hour) {
		do_say(keeper, "Sorry, I am closed. Come back tomorrow.");
		return NULL;
	}
	/*
	 * Invisible or hidden people.
	 */
	if (!can_see(keeper, ch) && !IS_IMMORTAL(ch)) {
		do_say(keeper, "I don't trade with folks I can't see.");
		do_emote(keeper, "looks all around.");
		return NULL;
	}
	return keeper;
}

/* insert an object at the right spot for the keeper */
static void obj_to_keeper(OBJ_DATA *obj, CHAR_DATA *ch)
{
	OBJ_DATA       *t_obj, *t_obj_next;
	/* see if any duplicates are found */
	for (t_obj = ch->carrying; t_obj != NULL; t_obj = t_obj_next) {
		t_obj_next = t_obj->next_content;

		if (obj->pObjIndex == t_obj->pObjIndex
		&&  !mlstr_cmp(&obj->short_descr, &t_obj->short_descr)) {
			if (IS_OBJ_STAT(t_obj, ITEM_INVENTORY)) {
				extract_obj(obj, 0);
				return;
			}
			obj->cost = t_obj->cost;	/* keep it standard */
			break;
		}
	}

	if (t_obj == NULL) {
		obj->next_content = ch->carrying;
		ch->carrying = obj;
	} else {
		obj->next_content = t_obj->next_content;
		t_obj->next_content = obj;
	}

	obj->carried_by = ch;
	obj->in_room = NULL;
	obj->in_obj = NULL;
	ch->carry_number += get_obj_number(obj);
	ch->carry_weight += get_obj_weight(obj);
}

/* get an object from a shopkeeper's list */
static OBJ_DATA *get_obj_keeper(CHAR_DATA *ch, CHAR_DATA *keeper,
				const char *argument)
{
	char            arg[MAX_INPUT_LENGTH];
	OBJ_DATA       *obj;
	int             number;
	int             count;
	number = number_argument(argument, arg, sizeof(arg));
	count = 0;
	for (obj = keeper->carrying; obj != NULL; obj = obj->next_content) {
		if (obj->wear_loc == WEAR_NONE
		    && can_see_obj(keeper, obj)
		    && can_see_obj(ch, obj)
		    && is_name(arg, obj->name)) {
			if (++count == number)
				return obj;

			/* skip other objects of the same name */
			while (obj->next_content != NULL
			  && obj->pObjIndex == obj->next_content->pObjIndex
			  && !mlstr_cmp(&obj->short_descr, &obj->next_content->short_descr))
				obj = obj->next_content;
		}
	}

	return NULL;
}

static uint get_cost(CHAR_DATA *keeper, OBJ_DATA *obj, bool fBuy)
{
	SHOP_DATA *	pShop;
	uint		cost;

	if (obj == NULL || (pShop = keeper->pMobIndex->pShop) == NULL)
		return 0;

	if (IS_OBJ_STAT(obj, ITEM_NOSELL))
		return 0;

	if (fBuy)
		cost = obj->cost * pShop->profit_buy / 100;
	else {
		OBJ_DATA       *obj2;
		int             itype;
		cost = 0;
		for (itype = 0; itype < MAX_TRADE; itype++) {
			if (obj->pObjIndex->item_type == pShop->buy_type[itype]) {
				cost = obj->cost * pShop->profit_sell / 100;
				break;
			}
		}

		if (!IS_OBJ_STAT(obj, ITEM_SELL_EXTRACT))
			for (obj2 = keeper->carrying; obj2; obj2 = obj2->next_content) {
				if (obj->pObjIndex == obj2->pObjIndex
				&&  !mlstr_cmp(&obj->short_descr,
					       &obj2->short_descr))
					return 0;
/*
	 	    if (IS_OBJ_STAT(obj2,ITEM_INVENTORY))
				cost /= 2;
		    else
		              	cost = cost * 3 / 4;
*/
			}
	}

	if (obj->pObjIndex->item_type == ITEM_STAFF
	||  obj->pObjIndex->item_type == ITEM_WAND) {
		if (INT_VAL(obj->value[1]) == 0)
			cost /= 4;
		else
			cost = cost * INT_VAL(obj->value[2]) / INT_VAL(obj->value[1]);
	}
	return cost;
}

static void sac_obj(CHAR_DATA * ch, OBJ_DATA *obj)
{
	int             silver;
	CHAR_DATA      *gch;
	int             members;

	if (!CAN_WEAR(obj, ITEM_TAKE) || CAN_WEAR(obj, ITEM_NO_SAC)) {
		act_puts("$p is not an acceptable sacrifice.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		return;
	}
	silver = UMAX(1, number_fuzzy(obj->level));

	if (obj->pObjIndex->item_type != ITEM_CORPSE_NPC
	&&  obj->pObjIndex->item_type != ITEM_CORPSE_PC)
		silver = UMIN(silver, obj->cost);

	act_puts("Gods give you $j silver $qj{coins} for your sacrifice.",
		 ch, (const void*) silver, NULL, TO_CHAR, POS_DEAD);

	ch->silver += silver;

	if (!IS_NPC(ch) && IS_SET(PC(ch)->plr_flags, PLR_AUTOSPLIT)) {
		/* AUTOSPLIT code */
		members = 0;
		for (gch = ch->in_room->people; gch != NULL;
		     gch = gch->next_in_room)
			if (is_same_group(gch, ch))
				members++;

		if (members > 1 && silver > 1)
			dofun("split", ch, "%d", silver);
	}
	act("$n sacrifices $p to gods.", ch, obj, NULL, TO_ROOM);

	if (oprog_call(OPROG_SAC, obj, ch, NULL))
		return;

	wiznet("$N sends up $p as a burnt offering.",
	       ch, obj, WIZ_SACCING, 0, 0);
	if (obj->pObjIndex->item_type == ITEM_CORPSE_NPC
	||  obj->pObjIndex->item_type == ITEM_CORPSE_PC) {
		OBJ_DATA       *obj_content;
		OBJ_DATA       *obj_next;
		OBJ_DATA       *two_objs[2];

		int	iScatter = 0;

		const char *qty;
		const char *where;

		for (obj_content = obj->contains; obj_content;
		     obj_content = obj_next) {
			obj_next = obj_content->next_content;
			two_objs[iScatter < 1 ? 0 : 1] = obj_content;
			obj_from_obj(obj_content);
			obj_to_room(obj_content, ch->in_room);
			iScatter++;
		}

		switch (iScatter) {
		case 0:
			break;

		case 1:
			act_puts("Your sacrifice reveals $p.",
				 ch, two_objs[0], NULL, TO_CHAR, POS_DEAD);
			act("$p is revealed by $n's sacrifice.",
			    ch, two_objs[0], NULL, TO_ROOM);
			break;

		case 2:
			act_puts("Your sacrifice reveals $p and $P.",
				 ch, two_objs[0], two_objs[1],
				 TO_CHAR, POS_DEAD);
			act("$p and $P are revealed by $n's sacrifice.",
			    ch, two_objs[0], two_objs[1], TO_ROOM);
			break;

		default: 
			if (iScatter < 5)
				qty = "few things";
			else if (iScatter < 9)
				qty = "a bunch of objects";
			else if (iScatter < 15)
				qty = "many things";
			else
				qty = "a lot of objects";

			switch (ch->in_room->sector_type) {
			case SECT_FIELD:
			case SECT_FOREST:
				where = "on the dirt";
				break;

			case SECT_WATER_SWIM:
			case SECT_WATER_NOSWIM:
				where = "over the water";
				break;

			default:
				where = "around";
				break;
			}
			act_puts("As you sacrifice the corpse, $t on it "
				 "scatter $T.",
				 ch, qty, where, TO_CHAR, POS_DEAD);
			act("As $n sacrifices the corpse, $t on it scatter $T.",
			    ch, qty, where, TO_ROOM);
			break;
		}
	}
	extract_obj(obj, 0);
}

static bool put_obj(CHAR_DATA *ch, OBJ_DATA *container,
		    OBJ_DATA *obj, int* count)
{
	OBJ_DATA *	objc;
	clan_t *clan;

	/*
	 * clan members can put item into clan altar even if it is closed
	 * check that container is clan altar is done in do_put
	 */
	if (IS_SET(INT_VAL(container->value[1]), CONT_CLOSED) 
	&&  ((clan = clan_lookup(ch->clan)) == NULL ||
	     clan->obj_ptr != obj)) {
		act_puts("$P is closed.",
			 ch, NULL, container, TO_CHAR, POS_DEAD);
		return FALSE;
	}

	if (IS_SET(INT_VAL(container->value[1]), CONT_QUIVER)
	&&  (obj->pObjIndex->item_type != ITEM_WEAPON ||
	     !WEAPON_IS(obj, WEAPON_ARROW))) {
		act_puts("You can only put arrows in $p.",
			 ch, container, NULL, TO_CHAR, POS_DEAD);
		return FALSE;
	}

	if (IS_SET(container->pObjIndex->extra_flags, ITEM_PIT)
	&&  !CAN_WEAR(obj, ITEM_TAKE)) {
		if (obj->timer)
			SET_BIT(obj->extra_flags, ITEM_HAD_TIMER);
		else
			obj->timer = number_range(100, 200);
	}

	if (obj->pObjIndex->limit != -1
	||  IS_SET(obj->pObjIndex->extra_flags, ITEM_QUEST)) {
		act_puts("This unworthy container won't hold $p.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		return TRUE;
	}

	if (obj->pObjIndex->item_type == ITEM_POTION
	&&  IS_SET(container->wear_flags, ITEM_TAKE)) {
		int pcount = 0;
		for (objc = container->contains; objc; objc = objc->next_content)
			if (objc->pObjIndex->item_type == ITEM_POTION)
				pcount++;
		if (pcount > 15) {
			act_puts("It's not safe to put more potions into $p.",
				 ch, container, NULL, TO_CHAR, POS_DEAD);
			return FALSE;
		}
	}

	(*count)++;
	if (*count > INT_VAL(container->value[0])) {
		act_puts("It's not safe to put that much items into $p.",
			 ch, container, NULL, TO_CHAR, POS_DEAD);
		return FALSE;
	}

	obj_from_char(obj);
	obj_to_obj(obj, container);

	if (IS_SET(INT_VAL(container->value[1]), CONT_PUT_ON)) {
		act("$n puts $p on $P.", ch, obj, container, TO_ROOM);
		act_puts("You put $p on $P.",
			 ch, obj, container, TO_CHAR, POS_DEAD);
	}
	else {
		act("$n puts $p in $P.", ch, obj, container, TO_ROOM);
		act_puts("You put $p in $P.",
			 ch, obj, container, TO_CHAR, POS_DEAD);
	}

	return TRUE;
}

static void drop_obj(CHAR_DATA *ch, OBJ_DATA *obj)
{
	obj_from_char(obj);
	obj_to_room(obj, ch->in_room);

	act("$n drops $p.", ch, obj, NULL,
	    TO_ROOM | (IS_AFFECTED(ch, AFF_SNEAK) ? ACT_NOMORTAL : 0));
	act_puts("You drop $p.", ch, obj, NULL, TO_CHAR, POS_DEAD);

	if (obj->pObjIndex->vnum == OBJ_VNUM_POTION_VIAL
	&&  number_percent() < 51)
		switch (ch->in_room->sector_type) {
		case SECT_FOREST:
		case SECT_DESERT:
		case SECT_AIR:
		case SECT_WATER_NOSWIM:
		case SECT_WATER_SWIM:
		case SECT_FIELD:
			break;
		default:
			act("$p cracks and shaters into tiny pieces.",
			    ch, obj, NULL, TO_ROOM);
			act("$p cracks and shaters into tiny pieces.",
			    ch, obj, NULL, TO_CHAR);
			extract_obj(obj, 0);
			return;
		}

	oprog_call(OPROG_DROP, obj, ch, NULL);

	if (!floating_time(obj) && IS_WATER(ch->in_room)) {
		act("$p sinks down the water.", ch, obj, NULL,
		    TO_ROOM | (IS_AFFECTED(ch, AFF_SNEAK) ? ACT_NOMORTAL : 0));
		act("$p sinks down the water.", ch, obj, NULL, TO_CHAR);
		extract_obj(obj, 0);
	}
	else if (IS_OBJ_STAT(obj, ITEM_MELT_DROP)) {
		act("$p dissolves into smoke.", ch, obj, NULL,
		    TO_ROOM | (IS_AFFECTED(ch, AFF_SNEAK) ? ACT_NOMORTAL : 0));
		act("$p dissolves into smoke.", ch, obj, NULL, TO_CHAR);
		extract_obj(obj, 0);
	}
}

/* equips a character */
void do_outfit(CHAR_DATA *ch, const char *argument)
{
	OBJ_DATA *obj;
	class_t *cl = class_lookup(ch->class);
	int vnum;

	if ((ch->level > 5 && !IS_IMMORTAL(ch))
	||  IS_NPC(ch) || cl == NULL) {
		char_puts("Find it yourself!\n",ch);
		return;
	}

	if ((obj = get_eq_char(ch, WEAR_LIGHT)) == NULL)
	{
	    obj = create_obj(get_obj_index(OBJ_VNUM_SCHOOL_BANNER), 0);
		obj->cost = 0;
		obj->condition = 100;
	    obj_to_char(obj, ch);
	    equip_char(ch, obj, WEAR_LIGHT);
	}
	
	if ((obj = get_eq_char(ch, WEAR_BODY)) == NULL)
	{
		obj = create_obj(get_obj_index(OBJ_VNUM_SCHOOL_VEST), 0);
		obj->cost = 0;
		obj->condition = 100;
	    obj_to_char(obj, ch);
	    equip_char(ch, obj, WEAR_BODY);
	}

	/* do the weapon thing */
	if ((obj = get_eq_char(ch,WEAR_WIELD)) == NULL) {
		vnum = cl->weapon;
		obj = create_obj(get_obj_index(vnum),0);
		obj->condition = 100;
	 	obj_to_char(obj,ch);
		equip_char(ch,obj,WEAR_WIELD);
	}

	if (((obj = get_eq_char(ch,WEAR_WIELD)) == NULL 
	||   !IS_WEAPON_STAT(obj,WEAPON_TWO_HANDS)) 
	&&  (obj = get_eq_char(ch, WEAR_SHIELD)) == NULL)
	{
	    obj = create_obj(get_obj_index(OBJ_VNUM_SCHOOL_SHIELD), 0);
		obj->cost = 0;
		obj->condition = 100;
	    obj_to_char(obj, ch);
	    equip_char(ch, obj, WEAR_SHIELD);
	}

	char_puts("You have been equipped by gods.\n",ch);
}

void do_auction(CHAR_DATA *ch, const char *argument)
{
	int tax;
	OBJ_DATA *obj;
	char arg1[MAX_INPUT_LENGTH];
	char starting[MAX_INPUT_LENGTH];

	argument = one_argument(argument, arg1, sizeof(arg1));

	if (IS_NPC(ch))    /* NPC can't auction cos it can be extracted! */
		return;

	if (!str_cmp(arg1, "off")) {
		char_puts("Auction channel is now OFF.\n",ch);
		SET_BIT(ch->comm,COMM_NOAUCTION);
		return;
	}

	if (IS_SET(ch->comm, COMM_NOAUCTION)) {
		char_puts("Auction channel is now ON.\n",ch);
		REMOVE_BIT(ch->comm, COMM_NOAUCTION);
	}

	if (arg1[0] == '\0') {
		if (auction.item != NULL) {
			/* show item data here */
			if (IS_IMMORTAL(ch)) {
				act_puts("Sold by: $i. Last bet by: $N.", 
					 ch, auction.seller, auction.buyer,
					 TO_CHAR, POS_DEAD);
			}
			if (auction.bet > 0) {
				act_puts("Current bet on this item is "
					 "$j gold.", ch,
					 (const void*) auction.bet, NULL,
					 TO_CHAR, POS_DEAD);
			} else {
				act_puts("Starting price for this item is "
					 "$j gold.", ch,
					 (const void*) auction.starting, NULL,
					 TO_CHAR, POS_DEAD);
				act_puts("No bets on this item have been "
					 "received.", ch, NULL, NULL,
					 TO_CHAR, POS_DEAD);
			}
			spellfun_call("identify", NULL, 0, ch, auction.item);
			return;
		}
		else {	
			char_puts("Auction WHAT?\n", ch);
			return;
		}
	}

	if (IS_IMMORTAL(ch) && !str_cmp(arg1, "stop"))
		if (auction.item == NULL) {
			char_puts("There is no auction going on "
				  "you can stop.\n",ch);
			return;
		}
		else { /* stop the auction */
			act_auction("Sale of $p has been stopped by $N.",
				    auction.item, ch, NULL,
				    ACT_FORMSH, POS_DEAD);
			auction_give_obj(auction.seller);

			/* return money to the buyer */
			if (auction.buyer != NULL) {
				auction.buyer->gold += auction.bet;
				act_puts("Your money has been returned.",
					 auction.buyer, NULL, NULL,
					 TO_CHAR, POS_DEAD);
	    		}

			/* return money to the seller */
			if (auction.seller != NULL) {
				auction.seller->gold +=
					(auction.starting * 20) / 100;
				act_puts("Your money has been returned.",
					 auction.seller, NULL, NULL,
					 TO_CHAR, POS_DEAD);
			}
	    		return;
		}

	if (!str_cmp(arg1, "bet")) {
	        int newbet;

		if (auction.item == NULL) {
	        	act_puts("There isn't anything being auctioned "
				 "right now.",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
	        	return;
		}

		if (ch == auction.seller) {
			act_puts("You cannot bet on your own equipment...:)",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
			return;
		}

	        /* make - perhaps - a bet now */
	        if (argument[0] == '\0') {
			act_puts("Bet how much?",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
			return;
	        }

		newbet = parsebet(auction.bet, argument);

	        if (newbet > ch->gold) {
	        	act_puts("You don't have that much money!",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
	        	return;
	        }

		if (auction.bet > 0) {
			if (newbet < auction.bet + 1) {
				act_puts("You must bid at least 1 gold "
					 "over the current bet.",
					 ch, NULL, NULL, TO_CHAR, POS_DEAD);
	        		return;
	        	}
		}
		else {
			if (newbet < auction.starting) {
				act_puts("You cannot bid less than the "
					 "starting price.",
					 ch, NULL, NULL, TO_CHAR, POS_DEAD);
				return;
			}
		}

	        /* the actual bet is OK! */

	        /* return the gold to the last buyer, if one exists */
	        if (auction.buyer != NULL)
	        	auction.buyer->gold += auction.bet;

	        ch->gold -= newbet; /* substract the gold - important :) */
	        auction.buyer = ch;
	        auction.bet   = newbet;
	        auction.going = 0;
	        auction.pulse = PULSE_AUCTION; /* start the auction over again */

	        act_auction("A bet of $J gold has been received on $p.",
			    auction.item, NULL, (const void*) newbet,
			    ACT_FORMSH, POS_RESTING);
	        return;
	}

	/* finally... */

	obj = get_obj_carry (ch, arg1); /* does char have the item ? */ 

	if (obj == NULL) {
		act_puts("You aren't carrying that.",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		return;
	}

	if (auction.item != NULL) {
		act_puts("Try again later - $p is being auctioned right now!",
			 ch, auction.item, NULL, TO_CHAR, POS_DEAD);
		return;
	}

	argument = one_argument(argument, starting, sizeof(starting));
	if (starting[0] == '\0')
		auction.starting = MIN_START_PRICE;
	else if ((auction.starting = atoi(starting)) < MIN_START_PRICE) {
		act_puts("You must specify the starting price "
			 "(at least $j gold).\n",
			 ch, (const void*) MIN_START_PRICE, NULL,
			 TO_CHAR, POS_DEAD);
		return;
	}

	switch (obj->pObjIndex->item_type) {
	default:
		if (!IS_SET(obj->pObjIndex->extra_flags, ITEM_CHQUEST)) {
			act_puts("You cannot auction $T.",
				 ch, NULL,
				 flag_string(item_types,
					     obj->pObjIndex->item_type),
				 TO_CHAR, POS_SLEEPING);
			break;
		}

		if (obj->contains) {
			act_puts("You can auction only empty containers.",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
			break;
		}

		/* FALLTHRU */

	case ITEM_LIGHT:
	case ITEM_WEAPON:
	case ITEM_ARMOR:
	case ITEM_STAFF:
	case ITEM_WAND:
	case ITEM_GEM:
	case ITEM_TREASURE:
	case ITEM_JEWELRY:
	case ITEM_FURNITURE:
	case ITEM_FOOD:
		tax = (auction.starting * 20) / 100;
		if (ch->gold < tax) {
			act_puts("You do not have enough gold to pay "
				 "an auction fee of $j gold.",
				 ch, (const void*) tax, NULL,
				 TO_CHAR, POS_DEAD);
			return;
		}

		act_puts("The auctioneer charges you an auction fee "
			 "of $j gold.",
			 ch, (const void*) tax, NULL,
			 TO_CHAR, POS_DEAD);
		ch->gold -= tax;

		obj_from_char(obj);
		auction.item = obj;
		auction.bet = 0; 	/* obj->cost / 100 */
		auction.buyer = NULL;
		auction.seller = ch;
		auction.pulse = PULSE_AUCTION;
		auction.going = 0;

		act_auction("A new item has been received: {Y$p{x.",
			    obj, NULL, NULL, ACT_FORMSH, POS_RESTING);
		break;
	} /* switch */
}
