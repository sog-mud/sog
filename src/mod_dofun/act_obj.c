/*
 * $Id: act_obj.c,v 1.85 1998-10-21 05:00:27 fjoe Exp $
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
#include "quest.h"
#include "update.h"
#include "mob_prog.h"
#include "obj_prog.h"
#include "fight.h"
#include "interp.h"

#include "resource.h"

/*
 * Local functions.
 */
CHAR_DATA *	find_keeper	(CHAR_DATA * ch);
uint		get_cost	(CHAR_DATA * keeper, OBJ_DATA * obj, bool fBuy);
void		obj_to_keeper	(OBJ_DATA * obj, CHAR_DATA * ch);
OBJ_DATA *	get_obj_keeper	(CHAR_DATA * ch, CHAR_DATA * keeper,
				 const char *argument);
void		do_sacr		(CHAR_DATA * ch, const char *argument);
AFFECT_DATA *	affect_find	(AFFECT_DATA * paf, int sn);

/* RT part of the corpse looting code */
bool can_loot(CHAR_DATA * ch, OBJ_DATA * obj)
{
	if (IS_IMMORTAL(ch))
		return TRUE;

	/*
	 * PC corpses in the ROOM_BATTLE_ARENA rooms can be looted
	 * only by owners
	 */
	if (obj->in_room != NULL
	&&  IS_SET(obj->in_room->room_flags, ROOM_BATTLE_ARENA)
	&&  obj->owner != NULL && str_cmp(ch->name, obj->owner))
		return FALSE;

	return TRUE;
}

void get_obj(CHAR_DATA * ch, OBJ_DATA * obj, OBJ_DATA * container)
{
	/* variables for AUTOSPLIT */
	CHAR_DATA      *gch;
	int             members;

	if (!CAN_WEAR(obj, ITEM_TAKE)
	||  ((obj->pIndexData->item_type == ITEM_CORPSE_PC ||
	      obj->pIndexData->item_type == ITEM_CORPSE_NPC) &&
	     (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM)) &&
	     !IS_IMMORTAL(ch) &&
	     str_cmp(ch->name, obj->owner))) {
		char_puts("You can't take that.\n\r", ch);
		return;
	}

	/* can't even get limited eq which does not match alignment */
	if (obj->pIndexData->limit != -1) {
		if ((IS_OBJ_STAT(obj, ITEM_ANTI_EVIL) && IS_EVIL(ch))
		||  (IS_OBJ_STAT(obj, ITEM_ANTI_GOOD) && IS_GOOD(ch))
		||  (IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch))) {
			act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD,
				    MSG_YOU_ZAPPED_BY_P);
			act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING,
				    MSG_N_ZAPPED_BY_P);
			return;
		}
	}

	if (ch->carry_number + get_obj_number(obj) > can_carry_n(ch)) {
		act_puts("$d: you can't carry that many items.",
			 ch, NULL, obj->name, TO_CHAR, POS_DEAD);
		return;
	}

	if (get_carry_weight(ch) + get_obj_weight(obj) > can_carry_w(ch)) {
		act_puts("$d: you can't carry that much weight.",
			 ch, NULL, obj->name, TO_CHAR, POS_DEAD);
		return;
	}

	if (obj->in_room != NULL) {
		for (gch = obj->in_room->people; gch != NULL;
		     gch = gch->next_in_room)
			if (gch->on == obj) {
				act_puts("$N appears to be using $p.",
					 ch, obj, gch, TO_CHAR, POS_DEAD);
				return;
			}
	}
	if (container != NULL) {
		if (obj_is_pit(container)
		&&  !CAN_WEAR(container, ITEM_TAKE)
		&&  !IS_OBJ_STAT(obj, ITEM_HAD_TIMER))
			obj->timer = 0;
		act_puts("You get $p from $P.",
			 ch, obj, container, TO_CHAR, POS_DEAD);
		act("$n gets $p from $P.", ch, obj, container,
		    TO_ROOM | (IS_AFFECTED(ch, AFF_SNEAK) ? SKIP_MORTAL : 0));
		REMOVE_BIT(obj->extra_flags, ITEM_HAD_TIMER);
		obj_from_obj(obj);
	}
	else {
		act_puts("You get $p.", ch, obj, container, TO_CHAR, POS_DEAD);
		act("$n gets $p.", ch, obj, container,
		    TO_ROOM | (IS_AFFECTED(ch, AFF_SNEAK) ? SKIP_MORTAL : 0));
		obj_from_room(obj);
	}

	if (obj->pIndexData->item_type == ITEM_MONEY) {
		if (get_carry_weight(ch) + obj->value[0] / 10
		    + obj->value[1] * 2 / 5 > can_carry_w(ch)) {
			act_puts("$d: you can't carry that much weight.",
				 ch, NULL, obj->name, TO_CHAR, POS_DEAD);
			if (container)
				obj_to_obj(obj, container);
			return;
		}
		ch->silver += obj->value[0];
		ch->gold += obj->value[1];
		if (IS_SET(ch->act, PLR_AUTOSPLIT)) {
			/* AUTOSPLIT code */
			members = 0;
			for (gch = ch->in_room->people; gch != NULL;
			     gch = gch->next_in_room) {
				if (!IS_AFFECTED(gch, AFF_CHARM)
				    && is_same_group(gch, ch))
					members++;
			}

			if (members > 1 && (obj->value[0] > 1
					    || obj->value[1]))
				doprintf(do_split, ch, "%d %d", obj->value[0],
					 obj->value[1]);
		}
		extract_obj(obj);
	}
	else {
		obj_to_char(obj, ch);
		oprog_call(OPROG_GET, obj, ch, NULL);
	}
}

void do_get(CHAR_DATA * ch, const char *argument)
{
	char            arg1[MAX_INPUT_LENGTH];
	char            arg2[MAX_INPUT_LENGTH];
	OBJ_DATA       *obj, *obj_next;
	OBJ_DATA       *container;
	bool            found;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (!str_cmp(arg2, "from"))
		argument = one_argument(argument, arg2);

	/* Get type. */
	if (arg1[0] == '\0') {
		char_puts("Get what?\n\r", ch);
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

			get_obj(ch, obj, NULL);
		} else {
			/* 'get all' or 'get all.obj' */
			found = FALSE;
			for (obj = ch->in_room->contents; obj != NULL;
			     obj = obj_next) {
				obj_next = obj->next_content;
				if ((arg1[3] == '\0' || is_name(arg1+4, obj->name))
				&& can_see_obj(ch, obj)) {
					found = TRUE;
					get_obj(ch, obj, NULL);
				}
			}

			if (!found) {
				if (arg1[3] == '\0')
					char_puts("I see nothing here.\n\r", ch);
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
		char_puts("You can't do that.\n\r", ch);
		return;
	}
	if ((container = get_obj_here(ch, arg2)) == NULL) {
		act_puts("I see no $T here.",
			 ch, NULL, arg2, TO_CHAR, POS_DEAD);
		return;
	}
	switch (container->pIndexData->item_type) {
	default:
		char_puts("That is not a container.\n\r", ch);
		return;

	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
		break;

	case ITEM_CORPSE_PC:
		if (!can_loot(ch, container)) {
			char_puts("You can't do that.\n\r", ch);
			return;
		}
	}

	if (IS_SET(container->value[1], CONT_CLOSED)) {
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
		get_obj(ch, obj, container);
	} else {
		/* 'get all container' or 'get all.obj container' */
		found = FALSE;
		for (obj = container->contains; obj != NULL; obj = obj_next) {
			obj_next = obj->next_content;
			if ((arg1[3] == '\0' || is_name(&arg1[4], obj->name))
			    && can_see_obj(ch, obj)) {
				found = TRUE;
				if (container->pIndexData->vnum == OBJ_VNUM_PIT
				    && !IS_IMMORTAL(ch)) {
					char_nputs(MSG_DONT_BE_SO_GREEDY, ch);
					return;
				}
				get_obj(ch, obj, container);
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

bool put_obj(CHAR_DATA *ch, OBJ_DATA *container, OBJ_DATA *obj, int* count)
{
	OBJ_DATA *	objc;

	if (IS_SET(container->value[1], CONT_FOR_ARROW)
	&&  (obj->pIndexData->item_type != ITEM_WEAPON || obj->value[0] != WEAPON_ARROW)) {
		act_puts("You can only put arrows in $p.",
			 ch, container, NULL, TO_CHAR, POS_DEAD);
		return FALSE;
	}

	if (container->pIndexData->vnum == OBJ_VNUM_PIT
	&&  !CAN_WEAR(obj, ITEM_TAKE))
		if (obj->timer)
			SET_BIT(obj->extra_flags, ITEM_HAD_TIMER);
		else
			obj->timer = number_range(100, 200);

	if (obj->pIndexData->limit != -1
	||  IS_SET(obj->pIndexData->extra_flags, ITEM_QUEST)) {
		act_puts("This unworthy container won't hold $p.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		return TRUE;
	}

	if (obj->pIndexData->item_type == ITEM_POTION
	&&  IS_SET(container->wear_flags, ITEM_TAKE)) {
		int pcount = 0;
		for (objc = container->contains; objc; objc = objc->next_content)
			if (objc->pIndexData->item_type == ITEM_POTION)
				pcount++;
		if (pcount > 15) {
			act_puts("It's not safe to put more potions into $p.",
				 ch, container, NULL, TO_CHAR, POS_DEAD);
			return FALSE;
		}
	}

	(*count)++;
	if (*count > container->value[0]) {
		act_puts("It's not safe to put that much items into $p.",
			 ch, container, NULL, TO_CHAR, POS_DEAD);
		return FALSE;
	}

	obj_from_char(obj);
	obj_to_obj(obj, container);

	if (IS_SET(container->value[1], CONT_PUT_ON)) {
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

void do_put(CHAR_DATA * ch, const char *argument)
{
	char		arg1[MAX_INPUT_LENGTH];
	char		arg2[MAX_INPUT_LENGTH];
	OBJ_DATA *	container;
	OBJ_DATA *	obj;
	OBJ_DATA *	obj_next;
	OBJ_DATA *	objc;
	int		count;

	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (!str_cmp(arg2, "in") || !str_cmp(arg2, "on"))
		argument = one_argument(argument, arg2);

	if (arg1[0] == '\0' || arg2[0] == '\0') {
		char_puts("Put what in what?\n\r", ch);
		return;
	}

	if (!str_cmp(arg2, "all") || !str_prefix("all.", arg2)) {
		char_puts("You can't do that.\n\r", ch);
		return;
	}

	if ((container = get_obj_here(ch, arg2)) == NULL) {
		act_puts("I see no $T here.",
			 ch, NULL, arg2, TO_CHAR, POS_DEAD);
		return;
	}

	if (container->pIndexData->item_type != ITEM_CONTAINER) {
		char_puts("That is not a container.\n\r", ch);
		return;
	}

	if (IS_SET(container->value[1], CONT_CLOSED)) {
		act_puts("The $d is closed.",
			 ch, NULL, container->name, TO_CHAR, POS_DEAD);
		return;
	}

	if (str_cmp(arg1, "all") && str_prefix("all.", arg1)) {
		/* 'put obj container' */
		if ((obj = get_obj_carry(ch, arg1)) == NULL) {
			char_puts("You do not have that item.\n\r", ch);
			return;
		}

		if (obj == container) {
			char_puts("You can't fold it into itself.\n\r", ch);
			return;
		}

		if (!can_drop_obj(ch, obj)) {
			char_puts("You can't let go of it.\n\r", ch);
			return;
		}

		if (WEIGHT_MULT(obj) != 100) {
			char_puts("You have a feeling that would be a bad idea.\n\r", ch);
			return;
		}

		if (obj->pIndexData->limit != -1) {
			act_puts("This unworthy container won't hold $p.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			return;
		}

		if (get_obj_weight(obj) + get_true_weight(container) >
		    (container->value[0] * 10)
		||  get_obj_weight(obj) > (container->value[3] * 10)) {
			char_puts("It won't fit.\n\r", ch);
			return;
		}

		count = 0;
		for (objc = container->contains; objc; objc = objc->next_content)
			count++;

		put_obj(ch, container, obj, &count);
	}
	else {
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
			    (container->value[0] * 10)
			&&  get_obj_weight(obj) < (container->value[3] * 10)
			&&  !put_obj(ch, container, obj, &count))
				break;
		}
	}
}

void drop_obj(CHAR_DATA *ch, OBJ_DATA *obj)
{
	obj_from_char(obj);
	obj_to_room(obj, ch->in_room);

	act("$n drops $p.", ch, obj, NULL,
	    TO_ROOM | (IS_AFFECTED(ch, AFF_SNEAK) ? SKIP_MORTAL : 0));
	act_puts("You drop $p.", ch, obj, NULL, TO_CHAR, POS_DEAD);

	if (obj->pIndexData->vnum == OBJ_VNUM_POTION_VIAL
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
			extract_obj(obj);
			return;
		}

	oprog_call(OPROG_DROP, obj, ch, NULL);

	if (!may_float(obj) && cant_float(obj) && IS_WATER(ch->in_room)) {
		act("$p sinks down the water.", ch, obj, NULL,
		    TO_ROOM | (IS_AFFECTED(ch, AFF_SNEAK) ? SKIP_MORTAL : 0));
		act("$p sinks down the water.", ch, obj, NULL, TO_CHAR);
		extract_obj(obj);
	}
	else if (IS_OBJ_STAT(obj, ITEM_MELT_DROP)) {
		act("$p dissolves into smoke.", ch, obj, NULL,
		    TO_ROOM | (IS_AFFECTED(ch, AFF_SNEAK) ? SKIP_MORTAL : 0));
		act("$p dissolves into smoke.", ch, obj, NULL, TO_CHAR);
		extract_obj(obj);
	}
}

void do_drop(CHAR_DATA * ch, const char *argument)
{
	char            arg[MAX_INPUT_LENGTH];
	OBJ_DATA       *obj;
	OBJ_DATA       *obj_next;
	bool            found;

	argument = one_argument(argument, arg);

	if (arg[0] == '\0') {
		char_puts("Drop what?\n\r", ch);
		return;
	}
	if (is_number(arg)) {
		/* 'drop NNNN coins' */
		int             amount, gold = 0, silver = 0;
		amount = atoi(arg);
		argument = one_argument(argument, arg);
		if (amount <= 0
		    || (str_cmp(arg, "coins") && str_cmp(arg, "coin")
			&& str_cmp(arg, "gold") && str_cmp(arg, "silver"))) {
			char_puts("You can't do that.\n\r", ch);
			return;
		}
		if (!str_cmp(arg, "coins") || !str_cmp(arg, "coin")
		    || !str_cmp(arg, "silver")) {
			if (ch->silver < amount) {
				char_puts("You don't have that much silver.\n\r", ch);
				return;
			}
			ch->silver -= amount;
			silver = amount;
		} else {
			if (ch->gold < amount) {
				char_puts("You don't have that much gold.\n\r", ch);
				return;
			}
			ch->gold -= amount;
			gold = amount;
		}

		for (obj = ch->in_room->contents; obj != NULL; obj = obj_next) {
			obj_next = obj->next_content;

			switch (obj->pIndexData->vnum) {
			case OBJ_VNUM_SILVER_ONE:
				silver += 1;
				extract_obj(obj);
				break;

			case OBJ_VNUM_GOLD_ONE:
				gold += 1;
				extract_obj(obj);
				break;

			case OBJ_VNUM_SILVER_SOME:
				silver += obj->value[0];
				extract_obj(obj);
				break;

			case OBJ_VNUM_GOLD_SOME:
				gold += obj->value[1];
				extract_obj(obj);
				break;

			case OBJ_VNUM_COINS:
				silver += obj->value[0];
				gold += obj->value[1];
				extract_obj(obj);
				break;
			}
		}

		obj = create_money(gold, silver);
		obj_to_room(obj, ch->in_room);
		act("$n drops some coins.", ch, NULL, NULL,
		    TO_ROOM | (IS_AFFECTED(ch, AFF_SNEAK) ? SKIP_MORTAL : 0));
		char_puts("Ok.\n\r", ch);
		if (IS_WATER(ch->in_room)) {
			extract_obj(obj);
			act("The coins sink down, and disapear in the water.",
			    ch, NULL, NULL,
			    TO_ROOM | (IS_AFFECTED(ch, AFF_SNEAK) ? SKIP_MORTAL : 0));
			act_puts("The coins sink down, and disapear in the water.",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		}
		return;
	}
	if (str_cmp(arg, "all") && str_prefix("all.", arg)) {
		/* 'drop obj' */
		if ((obj = get_obj_carry(ch, arg)) == NULL) {
			char_puts("You do not have that item.\n\r", ch);
			return;
		}
		if (!can_drop_obj(ch, obj)) {
			char_puts("You can't let go of it.\n\r", ch);
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

	argument = one_argument(argument, arg);
	if (arg[0] == '\0') {
		char_puts("Give what to whom?\n\r", ch);
		return;
	}

	if (is_number(arg)) {
		/* 'give NNNN coins victim' */
		int             amount;
		bool            silver;

		amount = atoi(arg);

		argument = one_argument(argument, arg);
		if (arg[0] == '\0') {
			do_give(ch, str_empty);
			return;
		}

		if (amount <= 0
		||  (str_cmp(arg, "coins") && str_cmp(arg, "coin") &&
		     str_cmp(arg, "gold") && str_cmp(arg, "silver"))) {
			char_puts("Sorry, you can't do that.\n\r", ch);
			return;
		}

		silver = str_cmp(arg, "gold");

		argument = one_argument(argument, arg);
		if (!str_cmp(arg, "to"))
			argument = one_argument(argument, arg);
		if (arg[0] == '\0') {
			do_give(ch, str_empty);
			return;
		}

		if ((victim = get_char_room(ch, arg)) == NULL) {
			char_puts("They aren't here.\n\r", ch);
			return;
		}

		if ((!silver && ch->gold < amount)
		||  (silver && ch->silver < amount)) {
			char_puts("You haven't got that much.\n\r", ch);
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

		act_printf(ch, silver ? "silver" : "gold", victim,
			   TO_VICT | TRANS_TEXT, POS_RESTING,
			   "$n gives you %d $t.", amount);
		act("$n gives $N some coins.", ch, NULL, victim, TO_NOTVICT);
		act_printf(ch, silver ? "silver" : "gold", victim,
			   TO_CHAR | TRANS_TEXT, POS_RESTING,
			   "You give $N %d $t.", amount);

		/*
		 * Bribe trigger
		 */
		if (IS_NPC(victim) && HAS_TRIGGER(victim, TRIG_BRIBE))
			mp_bribe_trigger(victim, ch,
					 silver ? amount : amount * 100);

		if (IS_NPC(victim) && IS_SET(victim->act, ACT_CHANGER)) {
			int             change;
			change = (silver ? 95 * amount / 100 / 100
				  : 95 * amount);


			if (!silver && change > victim->silver)
				victim->silver += change;

			if (silver && change > victim->gold)
				victim->gold += change;

			if (change < 1 && can_see(victim, ch)) {
				act("$n tells you '{GI'm sorry, you did not give me enough to change.{x'",
				    victim, NULL, ch, TO_VICT);
				ch->reply = victim;
				doprintf(do_give, victim, "%d %s %s",
				amount, silver ? "silver" : "gold", ch->name);
			}
			else if (can_see(victim, ch)) {
				doprintf(do_give, victim, "%d %s %s",
				change, silver ? "gold" : "silver", ch->name);
				if (silver)
					doprintf(do_give, victim, "%d silver %s",
						 (95 * amount / 100 - change * 100), ch->name);
				act("$n tells you '{GThank you, come again.{x'",
				    victim, NULL, ch, TO_VICT);
				ch->reply = victim;
			}
		}
		return;
	}

	if ((obj = get_obj_carry(ch, arg)) == NULL) {
		char_puts("You do not have that item.\n\r", ch);
		return;
	}

	argument = one_argument(argument, arg);
	if (!str_cmp(arg, "to"))
		argument = one_argument(argument, arg);
	if (arg[0] == '\0') {
		do_give(ch, str_empty);
		return;
	}

	if (obj->wear_loc != WEAR_NONE) {
		char_puts("You must remove it first.\n\r", ch);
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL) {
		char_puts("They aren't here.\n\r", ch);
		return;
	}

	if (IS_NPC(victim) && victim->pIndexData->pShop != NULL
	&&  !HAS_TRIGGER(victim, TRIG_GIVE)) {
		do_tell_raw(victim, ch, "Sorry, you'll have to sell that.");
		return;
	}

	if (!can_drop_obj(ch, obj)) {
		char_puts("You can't let go of it.\n\r", ch);
		return;
	}

	if (victim->carry_number + get_obj_number(obj) > can_carry_n(victim)) {
		act("$N has $S hands full.", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (get_carry_weight(victim) + get_obj_weight(obj) > can_carry_w(victim)) {
		act("$N can't carry that much weight.", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (IS_SET(obj->pIndexData->extra_flags, ITEM_QUEST)
	&&  !IS_IMMORTAL(ch) && !IS_IMMORTAL(victim)) {
		act_puts("Even you are not that silly to give $p to $N.",
			 ch, obj, victim, TO_CHAR, POS_DEAD);
		return;
	}

	if (!can_see_obj(victim, obj)) {
		act("$N can't see it.", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (obj->pIndexData->limit != -1) {
		if ((IS_OBJ_STAT(obj, ITEM_ANTI_EVIL) && IS_EVIL(victim))
		||  (IS_OBJ_STAT(obj, ITEM_ANTI_GOOD) && IS_GOOD(victim))
		||  (IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(victim))) {
			char_puts("Your victim's alignment doesn't match the objects align.", ch);
			return;
		}
	}

	obj_from_char(obj);
	obj_to_char(obj, victim);
	act("$n gives $p to $N.", ch, obj, victim, TO_NOTVICT | NO_TRIGGER);
	act("$n gives you $p.", ch, obj, victim, TO_VICT | NO_TRIGGER);
	act("You give $p to $N.", ch, obj, victim, TO_CHAR | NO_TRIGGER);

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
	int		sn;

	/* find out what */
	if (argument == '\0') {
		char_puts("Envenom what item?\n\r", ch);
		return;
	}
	obj = get_obj_list(ch, argument, ch->carrying);

	if (obj == NULL) {
		char_puts("You don't have that item.\n\r", ch);
		return;
	}

	if ((sn = sn_lookup("envenom")) < 0
	||  (chance = get_skill(ch, sn)) == 0) {
		char_puts("Are you crazy? You'd poison yourself!\n\r", ch);
		return;
	}

	if (obj->pIndexData->item_type == ITEM_FOOD || obj->pIndexData->item_type == ITEM_DRINK_CON) {
		if (IS_OBJ_STAT(obj, ITEM_BLESS)
		||  IS_OBJ_STAT(obj, ITEM_BURN_PROOF)) {
			act("You fail to poison $p.", ch, obj, NULL, TO_CHAR);
			return;
		}

		WAIT_STATE(ch, SKILL(sn)->beats);
		if (number_percent() < chance) {	/* success! */
			act("$n treats $p with deadly poison.",
			    ch, obj, NULL, TO_ROOM);
			act("You treat $p with deadly poison.",
			    ch, obj, NULL, TO_CHAR);
			if (!obj->value[3]) {
				obj->value[3] = 1;
				check_improve(ch, sn, TRUE, 4);
			}
			return;
		}
		act("You fail to poison $p.", ch, obj, NULL, TO_CHAR);
		if (!obj->value[3])
			check_improve(ch, sn, FALSE, 4);
		return;
	}
	else if (obj->pIndexData->item_type == ITEM_WEAPON) {
		if (IS_WEAPON_STAT(obj, WEAPON_FLAMING)
		||  IS_WEAPON_STAT(obj, WEAPON_FROST)
		||  IS_WEAPON_STAT(obj, WEAPON_VAMPIRIC)
		||  IS_WEAPON_STAT(obj, WEAPON_SHARP)
		||  IS_WEAPON_STAT(obj, WEAPON_VORPAL)
		||  IS_WEAPON_STAT(obj, WEAPON_SHOCKING)
		||  IS_WEAPON_STAT(obj, WEAPON_HOLY)
		||  IS_OBJ_STAT(obj, ITEM_BLESS)
		||  IS_OBJ_STAT(obj, ITEM_BURN_PROOF)) {
			act("You can't seem to envenom $p.",
			    ch, obj, NULL, TO_CHAR);
			return;
		}
		if (obj->value[3] < 0
		||  attack_table[obj->value[3]].damage == DAM_BASH) {
			char_puts("You can only envenom edged weapons.\n\r",
				  ch);
			return;
		}
		if (IS_WEAPON_STAT(obj, WEAPON_POISON)) {
			act("$p is already envenomed.", ch, obj, NULL, TO_CHAR);
			return;
		}

		WAIT_STATE(ch, SKILL(sn)->beats);
		percent = number_percent();
		if (percent < chance) {
			af.where = TO_WEAPON;
			af.type = gsn_poison;
			af.level = ch->level * percent / 100;
			af.duration = ch->level * percent / 100;
			af.location = 0;
			af.modifier = 0;
			af.bitvector = WEAPON_POISON;
			affect_to_obj(obj, &af);

			act("$n coats $p with deadly venom.", ch, obj, NULL,
			    TO_ROOM | (IS_AFFECTED(ch, AFF_SNEAK) ? SKIP_MORTAL : 0));
			act("You coat $p with venom.", ch, obj, NULL, TO_CHAR);
			check_improve(ch, sn, TRUE, 3);
			return;
		}
		else {
			act("You fail to envenom $p.", ch, obj, NULL, TO_CHAR);
			check_improve(ch, sn, FALSE, 3);
			return;
		}
	}
	act("You can't poison $p.", ch, obj, NULL, TO_CHAR);
}

void do_fill(CHAR_DATA * ch, const char *argument)
{
	char            arg[MAX_INPUT_LENGTH];
	OBJ_DATA       *obj;
	OBJ_DATA       *fountain;
	bool            found;
	one_argument(argument, arg);

	if (arg[0] == '\0') {
		char_puts("Fill what?\n\r", ch);
		return;
	}
	if ((obj = get_obj_carry(ch, arg)) == NULL) {
		char_puts("You do not have that item.\n\r", ch);
		return;
	}
	found = FALSE;
	for (fountain = ch->in_room->contents; fountain != NULL;
	     fountain = fountain->next_content) {
		if (fountain->pIndexData->item_type == ITEM_FOUNTAIN) {
			found = TRUE;
			break;
		}
	}

	if (!found) {
		char_puts("There is no fountain here!\n\r", ch);
		return;
	}
	if (obj->pIndexData->item_type != ITEM_DRINK_CON) {
		char_puts("You can't fill that.\n\r", ch);
		return;
	}
	if (obj->value[1] != 0 && obj->value[2] != fountain->value[2]) {
		char_puts("There is already another liquid in it.\n\r", ch);
		return;
	}
	if (obj->value[1] >= obj->value[0]) {
		char_puts("Your container is full.\n\r", ch);
		return;
	}
	act_printf(ch, obj, fountain, TO_CHAR, POS_RESTING,
		   "You fill $p with %s from $P.",
		   liq_table[fountain->value[2]].liq_name);
	act_printf(ch, obj, fountain, TO_ROOM, POS_RESTING,
		   "$n fills $p with %s from $P.",
		   liq_table[fountain->value[2]].liq_name);
	obj->value[2] = fountain->value[2];
	obj->value[1] = obj->value[0];

}

void do_pour(CHAR_DATA * ch, const char *argument)
{
	char            arg[MAX_STRING_LENGTH];
	OBJ_DATA       *out, *in;
	CHAR_DATA      *vch = NULL;
	int             amount;
	argument = one_argument(argument, arg);

	if (arg[0] == '\0' || argument[0] == '\0') {
		char_puts("Pour what into what?\n\r", ch);
		return;
	}
	if ((out = get_obj_carry(ch, arg)) == NULL) {
		char_puts("You don't have that item.\n\r", ch);
		return;
	}
	if (out->pIndexData->item_type != ITEM_DRINK_CON) {
		char_puts("That's not a drink container.\n\r", ch);
		return;
	}
	if (!str_cmp(argument, "out")) {
		if (out->value[1] == 0) {
			char_puts("It is empty.\n\r", ch);
			return;
		}
		out->value[1] = 0;
		out->value[3] = 0;
		act_printf(ch, out, NULL, TO_CHAR, POS_RESTING,
			   "You invert $p, spilling %s %s.",
			   liq_table[out->value[2]].liq_name,
			   IS_WATER(ch->in_room) ? "in to the water"
			   : "all over the ground");

		act_printf(ch, out, NULL, TO_ROOM, POS_RESTING,
			   "$n inverts $p, spilling %s %s.",
			   liq_table[out->value[2]].liq_name,
			   IS_WATER(ch->in_room) ? "in to the water"
			   : "all over the ground");
		return;
	}
	if ((in = get_obj_here(ch, argument)) == NULL) {
		vch = get_char_room(ch, argument);

		if (vch == NULL) {
			char_puts("Pour into what?\n\r", ch);
			return;
		}
		in = get_eq_char(vch, WEAR_HOLD);

		if (in == NULL) {
			char_puts("They aren't holding anything.", ch);
			return;
		}
	}
	if (in->pIndexData->item_type != ITEM_DRINK_CON) {
		char_puts("You can only pour into other drink containers.\n\r", ch);
		return;
	}
	if (in == out) {
		char_puts("You cannot change the laws of physics!\n\r", ch);
		return;
	}
	if (in->value[1] != 0 && in->value[2] != out->value[2]) {
		char_puts("They don't hold the same liquid.\n\r", ch);
		return;
	}
	if (out->value[1] == 0) {
		act("There's nothing in $p to pour.", ch, out, NULL, TO_CHAR);
		return;
	}
	if (in->value[1] >= in->value[0]) {
		act("$p is already filled to the top.", ch, in, NULL, TO_CHAR);
		return;
	}
	amount = UMIN(out->value[1], in->value[0] - in->value[1]);

	in->value[1] += amount;
	out->value[1] -= amount;
	in->value[2] = out->value[2];

	if (vch == NULL) {
		act_printf(ch, out, in, TO_CHAR, POS_RESTING,
			   "You pour %s from $p into $P.",
			   liq_table[out->value[2]].liq_name);
		act_printf(ch, out, in, TO_ROOM, POS_RESTING,
			   "$n pours %s from $p into $P.",
			   liq_table[out->value[2]].liq_name);
	} else {
		act_printf(ch, NULL, vch, TO_CHAR, POS_RESTING,
			   "You pour some %s for $N.",
			   liq_table[out->value[2]].liq_name);
		act_printf(ch, NULL, vch, TO_VICT, POS_RESTING,
			   "$n pours you some %s.",
			   liq_table[out->value[2]].liq_name);
		act_printf(ch, NULL, vch, TO_NOTVICT, POS_RESTING,
			   "$n pours some %s for $N.",
			   liq_table[out->value[2]].liq_name);
	}

}

void do_drink(CHAR_DATA * ch, const char *argument)
{
	char            arg[MAX_INPUT_LENGTH];
	OBJ_DATA       *obj;
	int             amount;
	int             liquid;
	one_argument(argument, arg);

	if (arg[0] == '\0') {
		for (obj = ch->in_room->contents; obj; obj= obj->next_content) {
			if (obj->pIndexData->item_type == ITEM_FOUNTAIN)
				break;
		}

		if (obj == NULL) {
			char_puts("Drink what?\n\r", ch);
			return;
		}
	} else {
		if ((obj = get_obj_here(ch, arg)) == NULL) {
			char_puts("You can't find it.\n\r", ch);
			return;
		}
	}

	if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10) {
		char_puts("You fail to reach your mouth.  *Hic*\n\r", ch);
		return;
	}
	switch (obj->pIndexData->item_type) {
	default:
		char_puts("You can't drink from that.\n\r", ch);
		return;

	case ITEM_FOUNTAIN:
		if ((liquid = obj->value[2]) < 0) {
			bug("Do_drink: bad liquid number %d.", liquid);
			liquid = obj->value[2] = 0;
		}
		amount = liq_table[liquid].liq_affect[4] * 3;
		break;

	case ITEM_DRINK_CON:
		if (obj->value[1] <= 0) {
			char_puts("It is empty.\n\r", ch);
			return;
		}
		if ((liquid = obj->value[2]) < 0) {
			bug("Do_drink: bad liquid number %d.", liquid);
			liquid = obj->value[2] = 0;
		}
		amount = liq_table[liquid].liq_affect[4];
		amount = UMIN(amount, obj->value[1]);
		break;
	}
	if (!IS_NPC(ch) && !IS_IMMORTAL(ch)
	    && ch->pcdata->condition[COND_FULL] > 80) {
		char_puts("You're too full to drink more.\n\r", ch);
		return;
	}
	act("$n drinks $T from $p.",
	    ch, obj, liq_table[liquid].liq_name, TO_ROOM);
	act_puts("You drink $T from $p.",
		 ch, obj, liq_table[liquid].liq_name, TO_CHAR, POS_DEAD);

	if (ch->fighting)
		WAIT_STATE(ch, 3 * PULSE_VIOLENCE);

	gain_condition(ch, COND_DRUNK,
		     amount * liq_table[liquid].liq_affect[COND_DRUNK] / 36);
	gain_condition(ch, COND_FULL,
		       amount * liq_table[liquid].liq_affect[COND_FULL] / 2);
	gain_condition(ch, COND_THIRST,
		     amount * liq_table[liquid].liq_affect[COND_THIRST] / 5);
	gain_condition(ch, COND_HUNGER,
		     amount * liq_table[liquid].liq_affect[COND_HUNGER] / 1);

	if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10)
		char_puts("You feel drunk.\n\r", ch);
	if (!IS_NPC(ch) && ch->pcdata->condition[COND_FULL] > 60)
		char_puts("You are full.\n\r", ch);
	if (!IS_NPC(ch) && ch->pcdata->condition[COND_THIRST] > 60)
		char_nputs(MSG_YOUR_THIRST_QUENCHED, ch);

	if (obj->value[3] != 0) {
		/* The drink was poisoned ! */
		AFFECT_DATA     af;

		act("$n chokes and gags.", ch, NULL, NULL, TO_ROOM);
		char_puts("You choke and gag.\n\r", ch);
		af.where = TO_AFFECTS;
		af.type = gsn_poison;
		af.level = number_fuzzy(amount);
		af.duration = 3 * amount;
		af.location = APPLY_NONE;
		af.modifier = 0;
		af.bitvector = AFF_POISON;
		affect_join(ch, &af);
	}
	if (obj->value[0] > 0)
		obj->value[1] -= amount;
	return;
}

void do_eat(CHAR_DATA * ch, const char *argument)
{
	char            arg[MAX_INPUT_LENGTH];
	OBJ_DATA       *obj;

	one_argument(argument, arg);
	if (arg[0] == '\0') {
		char_puts("Eat what?\n\r", ch);
		return;
	}
	if ((obj = get_obj_carry(ch, arg)) == NULL) {
		char_puts("You do not have that item.\n\r", ch);
		return;
	}
	if (!IS_IMMORTAL(ch)) {
		if ((obj->pIndexData->item_type != ITEM_FOOD ||
		     IS_SET(obj->extra_flags, ITEM_NOT_EDIBLE))
		&& obj->pIndexData->item_type != ITEM_PILL) {
			char_puts("That's not edible.\n\r", ch);
			return;
		}
		if (!IS_NPC(ch) && ch->pcdata->condition[COND_FULL] > 80) {
			char_puts("You are too full to eat more.\n\r", ch);
			return;
		}
	}
	act("$n eats $p.", ch, obj, NULL, TO_ROOM);
	act_puts("You eat $p.", ch, obj, NULL, TO_CHAR, POS_DEAD);
	if (ch->fighting != NULL)
		WAIT_STATE(ch, 3 * PULSE_VIOLENCE);

	switch (obj->pIndexData->item_type) {

	case ITEM_FOOD:
		if (!IS_NPC(ch)) {
			int             condition;
			condition = ch->pcdata->condition[COND_HUNGER];
			gain_condition(ch, COND_FULL, obj->value[0] * 2);
			gain_condition(ch, COND_HUNGER, obj->value[1] * 2);
			if (!condition
			&& ch->pcdata->condition[COND_HUNGER] > 0)
				char_puts("You are no longer hungry.\n\r", ch);
			else if (ch->pcdata->condition[COND_FULL] > 60)
				char_puts("You are full.\n\r", ch);
		}
		if (obj->value[3] != 0) {
			/* The food was poisoned! */
			AFFECT_DATA     af;

			act("$n chokes and gags.", ch, NULL, NULL, TO_ROOM);
			char_puts("You choke and gag.\n\r", ch);

			af.where = TO_AFFECTS;
			af.type = gsn_poison;
			af.level = number_fuzzy(obj->value[0]);
			af.duration = 2 * obj->value[0];
			af.location = APPLY_NONE;
			af.modifier = 0;
			af.bitvector = AFF_POISON;
			affect_join(ch, &af);
		}
		break;

	case ITEM_PILL:
		obj_cast_spell(obj->value[1], obj->value[0], ch, ch, NULL);
		obj_cast_spell(obj->value[2], obj->value[0], ch, ch, NULL);
		obj_cast_spell(obj->value[3], obj->value[0], ch, ch, NULL);
		obj_cast_spell(obj->value[4], obj->value[0], ch, ch, NULL);
		break;
	}

	extract_obj(obj);
	return;
}

/*
 * Remove an object.
 */
bool remove_obj(CHAR_DATA * ch, int iWear, bool fReplace)
{
	OBJ_DATA       *obj;
	if ((obj = get_eq_char(ch, iWear)) == NULL)
		return TRUE;

	if (!fReplace)
		return FALSE;

	if (IS_SET(obj->extra_flags, ITEM_NOREMOVE)) {
		act_puts("You can't remove $p.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		return FALSE;
	}
	if ((obj->pIndexData->item_type == ITEM_TATTOO) && (!IS_IMMORTAL(ch))) {
		act_puts("You must scratch it to remove $p.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		return FALSE;
	}
	if (iWear == WEAR_STUCK_IN) {
		unequip_char(ch, obj);

		if (get_eq_char(ch, WEAR_STUCK_IN) == NULL) {
			if (is_affected(ch, gsn_arrow))
				affect_strip(ch, gsn_arrow);
			if (is_affected(ch, gsn_spear))
				affect_strip(ch, gsn_spear);
		}
		act_puts("You remove $p, in pain.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		act("$n remove $p, in pain.", ch, obj, NULL, TO_ROOM);
		WAIT_STATE(ch, 4);
		return TRUE;
	}
	unequip_char(ch, obj);
	act("$n stops using $p.", ch, obj, NULL, TO_ROOM);
	act_puts("You stop using $p.", ch, obj, NULL, TO_CHAR, POS_DEAD);

	if (iWear == WEAR_WIELD
	    && (obj = get_eq_char(ch, WEAR_SECOND_WIELD)) != NULL) {
		unequip_char(ch, obj);
		equip_char(ch, obj, WEAR_WIELD);
	}
	return TRUE;
}

/*
 * Wear one object.
 * Optional replacement of existing objects.
 * Big repetitive code, ick.
 */
void wear_obj(CHAR_DATA * ch, OBJ_DATA * obj, bool fReplace)
{
	int wear_level = get_wear_level(ch, obj);

	if (wear_level < obj->level) {
		char_printf(ch, "You must be level %d to use this object.\n\r",
			    obj->level - wear_level + ch->level);
		act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING,
			    MSG_N_TRIES_TO_USE);
		return;
	}

	if (obj->pIndexData->item_type == ITEM_LIGHT) {
		if (!remove_obj(ch, WEAR_LIGHT, fReplace))
			return;
		act("$n lights $p and holds it.", ch, obj, NULL, TO_ROOM);
		act_puts("You light $p and hold it.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		equip_char(ch, obj, WEAR_LIGHT);
		return;
	}
	if (CAN_WEAR(obj, ITEM_WEAR_FINGER)) {
		if (get_eq_char(ch, WEAR_FINGER_L) != NULL
		&&  get_eq_char(ch, WEAR_FINGER_R) != NULL
		&&  !remove_obj(ch, WEAR_FINGER_L, fReplace)
		&&  !remove_obj(ch, WEAR_FINGER_R, fReplace))
			return;

		if (get_eq_char(ch, WEAR_FINGER_L) == NULL) {
			act("$n wears $p on $s left finger.",
			    ch, obj, NULL, TO_ROOM);
			act_puts("You wear $p on your left finger.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			equip_char(ch, obj, WEAR_FINGER_L);
			return;
		}
		if (get_eq_char(ch, WEAR_FINGER_R) == NULL) {
			act("$n wears $p on $s right finger.",
			    ch, obj, NULL, TO_ROOM);
			act_puts("You wear $p on your right finger.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			equip_char(ch, obj, WEAR_FINGER_R);
			return;
		}
		bug("Wear_obj: no free finger.", 0);
		char_puts("You already wear two rings.\n\r", ch);
		return;
	}
	if (CAN_WEAR(obj, ITEM_WEAR_NECK)) {
		if (get_eq_char(ch, WEAR_NECK_1) != NULL
		    && get_eq_char(ch, WEAR_NECK_2) != NULL
		    && !remove_obj(ch, WEAR_NECK_1, fReplace)
		    && !remove_obj(ch, WEAR_NECK_2, fReplace))
			return;

		if (get_eq_char(ch, WEAR_NECK_1) == NULL) {
			act("$n wears $p around $s neck.",
			    ch, obj, NULL, TO_ROOM);
			act_puts("You wear $p around your neck.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			equip_char(ch, obj, WEAR_NECK_1);
			return;
		}
		if (get_eq_char(ch, WEAR_NECK_2) == NULL) {
			act("$n wears $p around $s neck.",
			    ch, obj, NULL, TO_ROOM);
			act_puts("You wear $p around your neck.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			equip_char(ch, obj, WEAR_NECK_2);
			return;
		}
		bug("Wear_obj: no free neck.", 0);
		char_puts("You already wear two neck items.\n\r", ch);
		return;
	}
	if (CAN_WEAR(obj, ITEM_WEAR_BODY)) {
		if (!remove_obj(ch, WEAR_BODY, fReplace))
			return;
		act("$n wears $p on $s torso.", ch, obj, NULL, TO_ROOM);
		act_puts("You wear $p on your torso.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		equip_char(ch, obj, WEAR_BODY);
		return;
	}
	if (CAN_WEAR(obj, ITEM_WEAR_HEAD)) {
		if (!remove_obj(ch, WEAR_HEAD, fReplace))
			return;
		act("$n wears $p on $s head.", ch, obj, NULL, TO_ROOM);
		act_puts("You wear $p on your head.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		equip_char(ch, obj, WEAR_HEAD);
		return;
	}
	if (CAN_WEAR(obj, ITEM_WEAR_LEGS)) {
		if (!remove_obj(ch, WEAR_LEGS, fReplace))
			return;
		act("$n wears $p on $s legs.", ch, obj, NULL, TO_ROOM);
		act_puts("You wear $p on your legs.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		equip_char(ch, obj, WEAR_LEGS);
		return;
	}
	if (CAN_WEAR(obj, ITEM_WEAR_FEET)) {
		if (!remove_obj(ch, WEAR_FEET, fReplace))
			return;
		act("$n wears $p on $s feet.", ch, obj, NULL, TO_ROOM);
		act_puts("You wear $p on your feet.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		equip_char(ch, obj, WEAR_FEET);
		return;
	}
	if (CAN_WEAR(obj, ITEM_WEAR_HANDS)) {
		if (!remove_obj(ch, WEAR_HANDS, fReplace))
			return;
		act("$n wears $p on $s hands.", ch, obj, NULL, TO_ROOM);
		act_puts("You wear $p on your hands.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		equip_char(ch, obj, WEAR_HANDS);
		return;
	}
	if (CAN_WEAR(obj, ITEM_WEAR_ARMS)) {
		if (!remove_obj(ch, WEAR_ARMS, fReplace))
			return;
		act("$n wears $p on $s arms.", ch, obj, NULL, TO_ROOM);
		act_puts("You wear $p on your arms.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		equip_char(ch, obj, WEAR_ARMS);
		return;
	}
	if (CAN_WEAR(obj, ITEM_WEAR_ABOUT)) {
		if (!remove_obj(ch, WEAR_ABOUT, fReplace))
			return;
		act("$n wears $p on $s torso.", ch, obj, NULL, TO_ROOM);
		act_puts("You wear $p on your torso.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		equip_char(ch, obj, WEAR_ABOUT);
		return;
	}
	if (CAN_WEAR(obj, ITEM_WEAR_WAIST)) {
		if (!remove_obj(ch, WEAR_WAIST, fReplace))
			return;
		act("$n wears $p about $s waist.", ch, obj, NULL, TO_ROOM);
		act_puts("You wear $p about your waist.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		equip_char(ch, obj, WEAR_WAIST);
		return;
	}
	if (CAN_WEAR(obj, ITEM_WEAR_WRIST)) {
		if (get_eq_char(ch, WEAR_WRIST_L) != NULL
		    && get_eq_char(ch, WEAR_WRIST_R) != NULL
		    && !remove_obj(ch, WEAR_WRIST_L, fReplace)
		    && !remove_obj(ch, WEAR_WRIST_R, fReplace))
			return;

		if (get_eq_char(ch, WEAR_WRIST_L) == NULL) {
			act("$n wears $p around $s left wrist.",
			    ch, obj, NULL, TO_ROOM);
			act_puts("You wear $p around your left wrist.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			equip_char(ch, obj, WEAR_WRIST_L);
			return;
		}
		if (get_eq_char(ch, WEAR_WRIST_R) == NULL) {
			act("$n wears $p around $s right wrist.",
			    ch, obj, NULL, TO_ROOM);
			act_puts("You wear $p around your right wrist.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			equip_char(ch, obj, WEAR_WRIST_R);
			return;
		}
		bug("Wear_obj: no free wrist.", 0);
		char_puts("You already wear two wrist items.\n\r", ch);
		return;
	}
	if (CAN_WEAR(obj, ITEM_WEAR_SHIELD)) {
		OBJ_DATA       *weapon;
		if (get_eq_char(ch, WEAR_SECOND_WIELD) != NULL) {
			char_puts("You can't use a shield while using a second weapon.\n\r", ch);
			return;
		}
		if (!remove_obj(ch, WEAR_SHIELD, fReplace))
			return;

		weapon = get_eq_char(ch, WEAR_WIELD);
		if (weapon != NULL && ch->size < SIZE_LARGE
		&&  IS_WEAPON_STAT(weapon, WEAPON_TWO_HANDS)) {
			char_puts("Your hands are tied up with your weapon!\n\r", ch);
			return;
		}
		act("$n wears $p as a shield.", ch, obj, NULL, TO_ROOM);
		act_puts("You wear $p as a shield.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		equip_char(ch, obj, WEAR_SHIELD);
		return;
	}
	if (CAN_WEAR(obj, ITEM_WIELD)) {
		int             sn, skill;
		OBJ_DATA       *dual;
		if ((dual = get_eq_char(ch, WEAR_SECOND_WIELD)) != NULL)
			unequip_char(ch, dual);

		if (!remove_obj(ch, WEAR_WIELD, fReplace))
			return;

		if (!IS_NPC(ch)
		&& get_obj_weight(obj) >
			  str_app[get_curr_stat(ch, STAT_STR)].wield * 10) {
			char_puts("It is too heavy for you to wield.\n\r", ch);
			if (dual)
				equip_char(ch, dual, WEAR_SECOND_WIELD);
			return;
		}
		if (IS_WEAPON_STAT(obj, WEAPON_TWO_HANDS)
		&&  ((!IS_NPC(ch) && ch->size < SIZE_LARGE &&
		      get_eq_char(ch, WEAR_SHIELD) != NULL) ||
		     get_eq_char(ch, WEAR_SECOND_WIELD) != NULL)) {
			char_puts("You need two hands free for that weapon.\n\r", ch);
			if (dual)
				equip_char(ch, dual, WEAR_SECOND_WIELD);
			return;
		}
		act("$n wields $p.", ch, obj, NULL, TO_ROOM);
		act_puts("You wield $p", ch, obj, NULL, TO_CHAR, POS_DEAD);
		equip_char(ch, obj, WEAR_WIELD);
		if (dual)
			equip_char(ch, dual, WEAR_SECOND_WIELD);

		sn = get_weapon_sn(ch, WEAR_WIELD);

		if (sn == gsn_hand_to_hand)
			return;

		skill = get_weapon_skill(ch, sn);

		if (skill >= 100)
			act_puts("$p feels like a part of you!",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
		else if (skill > 85)
			act_puts("You feel quite confident with $p.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
		else if (skill > 70)
			act_puts("You are skilled with $p.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
		else if (skill > 50)
			act_puts("Your skill with $p is adequate.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
		else if (skill > 25)
			act_puts("$p feels a little clumsy in your hands.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
		else if (skill > 1)
			act_puts("You fumble and almost drop $p.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
		else
			act_puts("You don't even know which end is up on $p.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
		return;
	}
	if (CAN_WEAR(obj, ITEM_HOLD)) {
		if (get_eq_char(ch, WEAR_SECOND_WIELD) != NULL) {
			act_puts("You can't hold an item while using 2 weapons.",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
			return;
		}
		if (!remove_obj(ch, WEAR_HOLD, fReplace))
			return;
		act("$n holds $p in $s hand.", ch, obj, NULL, TO_ROOM);
		act_puts("You hold $p in your hand.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		equip_char(ch, obj, WEAR_HOLD);
		return;
	}
	if (CAN_WEAR(obj, ITEM_WEAR_FLOAT)) {
		if (!remove_obj(ch, WEAR_FLOAT, fReplace))
			return;
		act("$n releases $p to float next to $m.",
		    ch, obj, NULL, TO_ROOM);
		act_puts("You release $p and it floats next to you.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		equip_char(ch, obj, WEAR_FLOAT);
		return;
	}
	if (CAN_WEAR(obj, ITEM_WEAR_TATTOO) && IS_IMMORTAL(ch)) {
		if (!remove_obj(ch, WEAR_TATTOO, fReplace))
			return;
		act("$n now uses $p as tattoo of $s religion.",
		    ch, obj, NULL, TO_ROOM);
		act_puts("You now use $p as the tattoo of your religion.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		equip_char(ch, obj, WEAR_TATTOO);
		return;
	}
	if (fReplace)
		char_puts("You can't wear, wield, or hold that.\n\r", ch);
}

void do_wear(CHAR_DATA * ch, const char *argument)
{
	char            arg[MAX_INPUT_LENGTH];
	OBJ_DATA       *obj;
	OBJ_DATA       *obj_next;
	one_argument(argument, arg);

	if (arg[0] == '\0') {
		char_puts("Wear, wield, or hold what?\n\r", ch);
		return;
	}
	if (!str_cmp(arg, "all"))
		for (obj = ch->carrying; obj != NULL; obj = obj_next) {
			obj_next = obj->next_content;
			if (obj->wear_loc == WEAR_NONE && can_see_obj(ch, obj))
				wear_obj(ch, obj, FALSE);
		}
	else if ((obj = get_obj_carry(ch, arg)) == NULL) {
		char_puts("You do not have that item.\n\r", ch);
		return;
	} else
		wear_obj(ch, obj, TRUE);
}

void do_remove(CHAR_DATA * ch, const char *argument)
{
	char            arg[MAX_INPUT_LENGTH];
	OBJ_DATA       *obj;
	one_argument(argument, arg);

	if (arg[0] == '\0') {
		char_puts("Remove what?\n\r", ch);
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
		char_puts("You do not have that item.\n\r", ch);
		return;
	}
	remove_obj(ch, obj->wear_loc, TRUE);
	return;
}

void do_sacrifice(CHAR_DATA * ch, const char *argument)
{
	OBJ_DATA       *r_cont;
	OBJ_DATA       *r_next_cont;
	if (!strcmp(argument, "all"))
		for (r_cont = ch->in_room->contents; r_cont;
		     r_cont = r_next_cont) {
			r_next_cont = r_cont->next_content;
			do_sacr(ch, r_cont->name);
	} else
		do_sacr(ch, argument);
}

void do_sacr(CHAR_DATA * ch, const char *argument)
{
	char            arg[MAX_INPUT_LENGTH];
	char            buf[MAX_STRING_LENGTH];
	char            buf2[MAX_STRING_LENGTH];
	OBJ_DATA       *obj;
	OBJ_DATA       *obj_content;
	OBJ_DATA       *obj_next;
	OBJ_DATA       *two_objs[2];
	int             silver;
	int             iScatter;
	bool            fScatter;


	/* variables for AUTOSPLIT */
	CHAR_DATA      *gch;
	int             members;
	one_argument(argument, arg);

	if (arg[0] == '\0' || !str_cmp(arg, ch->name)) {
		act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING, MSG_N_SACS_SELF);
		char_puts("Gods appreciates your offer and may accept it later.\n\r", ch);
		return;
	}
	obj = get_obj_list(ch, arg, ch->in_room->contents);
	if (obj == NULL) {
		char_puts("You can't find it.\n\r", ch);
		return;
	}
	if ((obj->pIndexData->item_type == ITEM_CORPSE_PC && ch->level < MAX_LEVEL)
	    || (QUEST_OBJ_FIRST <= obj->pIndexData->vnum
		&& obj->pIndexData->vnum <= QUEST_OBJ_LAST)) {
		char_puts("Gods wouldn't like that.\n\r", ch);
		return;
	}
	if (!CAN_WEAR(obj, ITEM_TAKE) || CAN_WEAR(obj, ITEM_NO_SAC)) {
		act_puts("$p is not an acceptable sacrifice.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		return;
	}
	silver = UMAX(1, number_fuzzy(obj->level));

	if (obj->pIndexData->item_type != ITEM_CORPSE_NPC
	    && obj->pIndexData->item_type != ITEM_CORPSE_PC)
		silver = UMIN(silver, obj->cost);

	if (silver == 1)
		char_puts("Gods give you one silver coin for your sacrifice.\n\r", ch);
	else
		char_printf(ch, "Gods give you %d silver coins for your sacrifice.\n\r", silver);

	ch->silver += silver;

	if (IS_SET(ch->act, PLR_AUTOSPLIT)) {
		/* AUTOSPLIT code */
		members = 0;
		for (gch = ch->in_room->people; gch != NULL;
		     gch = gch->next_in_room)
			if (is_same_group(gch, ch))
				members++;

		if (members > 1 && silver > 1)
			doprintf(do_split, ch, "%d", silver);
	}
	act("$n sacrifices $p to gods.", ch, obj, NULL, TO_ROOM);

	if (oprog_call(OPROG_SAC, obj, ch, NULL))
		return;

	wiznet("$N sends up $p as a burnt offering.",
	       ch, obj, WIZ_SACCING, 0, 0);
	fScatter = TRUE;
	if (obj->pIndexData->item_type == ITEM_CORPSE_NPC
	    || obj->pIndexData->item_type == ITEM_CORPSE_PC) {
		iScatter = 0;
		for (obj_content = obj->contains; obj_content;
		     obj_content = obj_next) {
			obj_next = obj_content->next_content;
			two_objs[iScatter < 1 ? 0 : 1] = obj_content;
			obj_from_obj(obj_content);
			obj_to_room(obj_content, ch->in_room);
			iScatter++;
		}
		if (iScatter == 1) {
			act_puts("Your sacrifice reveals $p.",
				 ch, two_objs[0], NULL, TO_CHAR, POS_DEAD);
			act("$p is revealed by $n's sacrifice.",
			    ch, two_objs[0], NULL, TO_ROOM);
		}
		if (iScatter == 2) {
			act_puts("Your sacrifice reveals $p and $P.",
				 ch, two_objs[0], two_objs[1], TO_CHAR, POS_DEAD);
			act("$p and $P are revealed by $n's sacrifice.",
			    ch, two_objs[0], two_objs[1], TO_ROOM);
		}
		snprintf(buf, sizeof(buf), MSG("As you sacrifice the corpse, ", ch->lang));
		snprintf(buf2, sizeof(buf2), MSG("As $n sacrifices the corpse, ", ch->lang));
		if (iScatter < 3)
			fScatter = FALSE;
		else if (iScatter < 5) {
			strcat(buf, MSG("few things ", ch->lang));
			strcat(buf2, MSG("few things ", ch->lang));
		} else if (iScatter < 9) {
			strcat(buf, MSG("a bunch of objects ", ch->lang));
			strcat(buf2, MSG("a bunch of objects ", ch->lang));
		} else if (iScatter < 15) {
			strcat(buf, MSG("many things ", ch->lang));
			strcat(buf2, MSG("many things ", ch->lang));
		} else {
			strcat(buf, MSG("a lot of objects ", ch->lang));
			strcat(buf2, MSG("a lot of objects ", ch->lang));
		}
		strcat(buf, MSG("on it, ", ch->lang));
		strcat(buf2, MSG("on it, ", ch->lang));

		switch (ch->in_room->sector_type) {
		case SECT_FIELD:
		case SECT_FOREST:
			strcat(buf, MSG("scatter on the dirt.", ch->lang));
			strcat(buf2, MSG("scatter on the dirt.", ch->lang));
			break;
		case SECT_WATER_SWIM:
			strcat(buf, MSG("scatter over the water.", ch->lang));
			strcat(buf2, MSG("scatter over the water.", ch->lang));
			break;
		case SECT_WATER_NOSWIM:
			strcat(buf, MSG("scatter over the water.", ch->lang));
			strcat(buf2, MSG("scatter over the water.", ch->lang));
			break;
		default:
			strcat(buf, MSG("scatter around.", ch->lang));
			strcat(buf2, MSG("scatter around.", ch->lang));
			break;
		}
		if (fScatter) {
			act_puts(buf, ch, NULL, NULL, TO_CHAR, POS_DEAD);
			act(buf2, ch, NULL, NULL, TO_ROOM);
		}
	}
	extract_obj(obj);
	return;
}

void do_quaff(CHAR_DATA * ch, const char *argument)
{
	char            arg[MAX_INPUT_LENGTH];
	OBJ_DATA       *obj;
	one_argument(argument, arg);

	if (arg[0] == '\0') {
		char_puts("Quaff what?\n\r", ch);
		return;
	}
	if ((obj = get_obj_carry(ch, arg)) == NULL) {
		char_puts("You do not have that potion.\n\r", ch);
		return;
	}
	if (obj->pIndexData->item_type != ITEM_POTION) {
		char_puts("You can quaff only potions.\n\r", ch);
		return;
	}
	if (ch->level < obj->level) {
		char_puts("This liquid is too powerful for you to drink.\n\r", ch);
		return;
	}
	act("$n quaffs $p.", ch, obj, NULL, TO_ROOM);
	act_puts("You quaff $p.", ch, obj, NULL, TO_CHAR, POS_DEAD);

	obj_cast_spell(obj->value[1], obj->value[0], ch, ch, NULL);
	obj_cast_spell(obj->value[2], obj->value[0], ch, ch, NULL);
	obj_cast_spell(obj->value[3], obj->value[0], ch, ch, NULL);
	obj_cast_spell(obj->value[4], obj->value[0], ch, ch, NULL);

	if (IS_PUMPED(ch) || ch->fighting != NULL)
		WAIT_STATE(ch, 2 * PULSE_VIOLENCE);

	extract_obj(obj);
	obj_to_char(create_obj(get_obj_index(OBJ_VNUM_POTION_VIAL), 0), ch);
}

void do_recite(CHAR_DATA * ch, const char *argument)
{
	char            arg1[MAX_INPUT_LENGTH];
	char            arg2[MAX_INPUT_LENGTH];
	CHAR_DATA      *victim;
	OBJ_DATA       *scroll;
	OBJ_DATA       *obj;
	int		sn;

	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if ((scroll = get_obj_carry(ch, arg1)) == NULL) {
		char_puts("You do not have that scroll.\n\r", ch);
		return;
	}

	if (scroll->pIndexData->item_type != ITEM_SCROLL) {
		char_puts("You can recite only scrolls.\n\r", ch);
		return;
	}

	if (ch->level < scroll->level
	||  (sn = sn_lookup("scrolls")) < 0) {
		char_puts("This scroll is too complex for you to comprehend.\n\r", ch);
		return;
	}

	obj = NULL;
	if (arg2[0] == '\0')
		victim = ch;
	else if ((victim = get_char_room(ch, arg2)) == NULL
	&&       (obj = get_obj_here(ch, arg2)) == NULL) {
		char_puts("You can't find it.\n\r", ch);
		return;
	}

	act("$n recites $p.", ch, scroll, NULL, TO_ROOM);
	act_puts("You recite $p.", ch, scroll, NULL, TO_CHAR, POS_DEAD);

	if (number_percent() >= get_skill(ch, sn) * 4 / 5) {
		char_nputs(MSG_MISPRONOUNCE_SYLLABLE, ch);
		check_improve(ch, sn, FALSE, 2);
	} else {
		obj_cast_spell(scroll->value[1], scroll->value[0],
			       ch, victim, obj);
		obj_cast_spell(scroll->value[2], scroll->value[0],
			       ch, victim, obj);
		obj_cast_spell(scroll->value[3], scroll->value[0],
			       ch, victim, obj);
		obj_cast_spell(scroll->value[4], scroll->value[0],
			       ch, victim, obj);
		check_improve(ch, sn, TRUE, 2);

		if (IS_PUMPED(ch) || ch->fighting != NULL)
			WAIT_STATE(ch, 2 * PULSE_VIOLENCE);
	}

	extract_obj(scroll);
}

void do_brandish(CHAR_DATA * ch, const char *argument)
{
	CHAR_DATA      *vch;
	CHAR_DATA      *vch_next;
	OBJ_DATA       *staff;
	int             sn;
	SKILL_DATA *	sk;

	if ((staff = get_eq_char(ch, WEAR_HOLD)) == NULL) {
		char_puts("You hold nothing in your hand.\n\r", ch);
		return;
	}

	if (staff->pIndexData->item_type != ITEM_STAFF) {
		char_puts("You can brandish only with a staff.\n\r", ch);
		return;
	}

	if ((sk = skill_lookup(staff->value[3])) == NULL
	||  sk->spell_fun == NULL
	||  (sn = sn_lookup("staves")) < 0)
		return;

	WAIT_STATE(ch, 2 * PULSE_VIOLENCE);

	if (staff->value[2] > 0) {
		act("$n brandishes $p.", ch, staff, NULL, TO_ROOM);
		act("You brandish $p.", ch, staff, NULL, TO_CHAR);
		if (ch->level + 3 < staff->level
		|| number_percent() >= 10 + get_skill(ch, sn) * 4 / 5) {
			act("You fail to invoke $p.", ch, staff, NULL, TO_CHAR);
			act("...and nothing happens.", ch, NULL, NULL, TO_ROOM);
			check_improve(ch, sn, FALSE, 2);
		} else
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

				if (is_safe(ch, vch))
					continue;

				obj_cast_spell(staff->value[3],
					       staff->value[0], ch, vch, NULL);
				check_improve(ch, sn, TRUE, 2);
			}
	}
	if (--staff->value[2] <= 0) {
		act("$n's $p blazes bright and is gone.", ch, staff, NULL, TO_ROOM);
		act("Your $p blazes bright and is gone.", ch, staff, NULL, TO_CHAR);
		extract_obj(staff);
	}
}

void do_zap(CHAR_DATA * ch, const char *argument)
{
	char            arg[MAX_INPUT_LENGTH];
	CHAR_DATA      *victim;
	OBJ_DATA       *wand;
	OBJ_DATA       *obj;
	int		sn;

	one_argument(argument, arg);
	if (arg[0] == '\0' && ch->fighting == NULL) {
		char_puts("Zap whom or what?\n\r", ch);
		return;
	}
	if ((wand = get_eq_char(ch, WEAR_HOLD)) == NULL) {
		char_puts("You hold nothing in your hand.\n\r", ch);
		return;
	}
	if (wand->pIndexData->item_type != ITEM_WAND) {
		char_puts("You can zap only with a wand.\n\r", ch);
		return;
	}

	if ((sn = sn_lookup("wands")) < 0)
		return;

	obj = NULL;
	if (arg[0] == '\0') {
		if (ch->fighting != NULL) {
			victim = ch->fighting;
		} else {
			char_puts("Zap whom or what?\n\r", ch);
			return;
		}
	} else {
		if ((victim = get_char_room(ch, arg)) == NULL
		    && (obj = get_obj_here(ch, arg)) == NULL) {
			char_puts("You can't find it.\n\r", ch);
			return;
		}
	}

	if (victim && is_safe(ch, victim))
		return;

	WAIT_STATE(ch, 2 * PULSE_VIOLENCE);

	if (wand->value[2] > 0) {
		if (victim != NULL) {
			act("$n zaps $N with $p.", ch, wand, victim, TO_ROOM);
			act("You zap $N with $p.", ch, wand, victim, TO_CHAR);
		} else {
			act("$n zaps $P with $p.", ch, wand, obj, TO_ROOM);
			act("You zap $P with $p.", ch, wand, obj, TO_CHAR);
		}

		if (ch->level + 5 < wand->level
		|| number_percent() >= 20 + get_skill(ch, sn) * 4 / 5) {
			act("Your efforts with $p produce only smoke and sparks.",
			    ch, wand, NULL, TO_CHAR);
			act("$n's efforts with $p produce only smoke and sparks.",
			    ch, wand, NULL, TO_ROOM);
			check_improve(ch, sn, FALSE, 2);
		} else {
			obj_cast_spell(wand->value[3], wand->value[0],
				       ch, victim, obj);
			check_improve(ch, sn, TRUE, 2);
		}
	}
	if (--wand->value[2] <= 0) {
		act("$n's $p explodes into fragments.", ch, wand, NULL, TO_ROOM);
		act("Your $p explodes into fragments.", ch, wand, NULL, TO_CHAR);
		extract_obj(wand);
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
	int		sn;
	
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (arg1[0] == '\0' || arg2[0] == '\0') {
		char_puts("Steal what from whom?\n\r", ch);
		return;
	}

	if (IS_NPC(ch) && IS_SET(ch->affected_by, AFF_CHARM)
	    && (ch->master != NULL)) {
		char_puts("You are to dazed to steal anything.\n\r", ch);
		return;
	}

	if ((sn = sn_lookup("steal")) < 0)
		return;

	WAIT_STATE(ch, SKILL(sn)->beats);

	if ((victim = get_char_room(ch, arg2)) == NULL) {
		char_puts("They aren't here.\n\r", ch);
		return;
	}
	if (victim == ch) {
		char_puts("That's pointless.\n\r", ch);
		return;
	}

	if (victim->position == POS_FIGHTING) {
		char_puts("You'd better not -- you might get hit.\n\r", ch);
		return;
	}

	if (is_safe(ch, victim))
		return;
	
	percent = number_percent() +
		  (IS_AWAKE(victim) ? 10 : -50) +
		  (!can_see(victim, ch) ? -10 : 0);

	if ((!IS_NPC(ch) && percent > get_skill(ch, sn))
	||  IS_SET(victim->imm_flags, IMM_STEAL)
	||  IS_IMMORTAL(victim)
	||  (victim->in_room &&
	     IS_SET(victim->in_room->room_flags, ROOM_BATTLE_ARENA))) {
		/*
		 * Failure.
		 */

		char_puts("Oops.\n\r", ch);

		if (IS_AFFECTED(ch, AFF_HIDE | AFF_FADE) && !IS_NPC(ch)) {
			REMOVE_BIT(ch->affected_by, AFF_HIDE | AFF_FADE);
			char_nputs(MSG_YOU_STEP_OUT_SHADOWS, ch);
			act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING, 
				    MSG_N_STEPS_OUT_OF_SHADOWS);
        	}

		if (!IS_AFFECTED(victim, AFF_SLEEP)) {
			victim->position = victim->position == POS_SLEEPING ? POS_STANDING :
				victim->position;
			act("$n tried to steal from you.\n\r", ch, NULL, victim, TO_VICT);
		}
		act("$n tried to steal from $N.\n\r", ch, NULL, victim, TO_NOTVICT);

		if (IS_AWAKE(victim))
			switch (number_range(0, 3)) {
			case 0:
				doprintf(do_yell, victim, "%s is a lousy thief!", ch->name);
				break;
			case 1:
				doprintf(do_yell, victim, "%s couldn't rob %s way out of a paper bag!",
					ch->name,
					(ch->sex == SEX_FEMALE) ? "her" :
					(ch->sex == SEX_MALE) ? "his" :
					"its");
				break;
			case 2:
				doprintf(do_yell, victim, "%s tried to rob me!",
					 PERS(ch, victim));
				break;
			case 3:
				doprintf(do_yell, victim, "Keep your hands out of there, %s!",
					 PERS(ch, victim));
				break;
			}

		if (!IS_NPC(ch) && IS_NPC(victim)) {
			check_improve(ch, sn, FALSE, 2);
			multi_hit(victim, ch, TYPE_UNDEFINED);
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
			char_puts("You couldn't get any coins.\n\r", ch);
			return;
		}
		ch->gold += amount_g;
		victim->gold -= amount_g;
		ch->silver += amount_s;
		victim->silver -= amount_s;
		char_printf(ch, "Bingo!  You got %d %s coins.\n\r",
			    amount_s != 0 ? amount_s : amount_g,
			    amount_s != 0 ? "silver" : "gold");
		check_improve(ch, sn, TRUE, 2);
		return;
	}
	if ((obj = get_obj_carry(victim, arg1)) == NULL) {
		char_puts("You can't find it.\n\r", ch);
		return;
	}
	if (!can_drop_obj(ch, obj)
	/* ||   IS_SET(obj->extra_flags, ITEM_INVENTORY) */
	     /* ||  obj->level > ch->level */ ) {
		char_puts("You can't pry it away.\n\r", ch);
		return;
	}
	if (ch->carry_number + get_obj_number(obj) > can_carry_n(ch)) {
		char_puts("You have your hands full.\n\r", ch);
		return;
	}
	if (ch->carry_weight + get_obj_weight(obj) > can_carry_w(ch)) {
		char_puts("You can't carry that much weight.\n\r", ch);
		return;
	}
	if (!IS_SET(obj->extra_flags, ITEM_INVENTORY)) {
		obj_from_char(obj);
		obj_to_char(obj, ch);
		char_puts("You got it!\n\r", ch);
		check_improve(ch, sn, TRUE, 2);
	} else {
		obj_inve = NULL;
		obj_inve = create_obj(obj->pIndexData, 0);
		clone_obj(obj, obj_inve);
		REMOVE_BIT(obj_inve->extra_flags, ITEM_INVENTORY);
		obj_to_char(obj_inve, ch);
		char_puts("You got one of them!\n\r", ch);
		check_improve(ch, sn, TRUE, 1);
	}
}

/*
 * Shopping commands.
 */
CHAR_DATA * find_keeper(CHAR_DATA * ch)
{
	CHAR_DATA      *keeper;
	SHOP_DATA      *pShop = NULL;

	for (keeper = ch->in_room->people; keeper; keeper = keeper->next_in_room) {
		if (IS_NPC(keeper)
		&&  (pShop = keeper->pIndexData->pShop) != NULL)
			break;
	}

	if (pShop == NULL) {
		char_puts("You can't do that here.\n\r", ch);
		return NULL;
	}

	if (IS_SET(keeper->in_room->area->flags, AREA_HOMETOWN)
	    && !IS_NPC(ch) && IS_SET(ch->act, PLR_WANTED)) {
		do_say(keeper, "Criminals are not welcome!");
		doprintf(do_yell, keeper, "%s the CRIMINAL is over here!\n\r", ch->name);
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
		do_scan(keeper, str_empty);
		return NULL;
	}
	return keeper;
}

/* insert an object at the right spot for the keeper */
void obj_to_keeper(OBJ_DATA * obj, CHAR_DATA * ch)
{
	OBJ_DATA       *t_obj, *t_obj_next;
	/* see if any duplicates are found */
	for (t_obj = ch->carrying; t_obj != NULL; t_obj = t_obj_next) {
		t_obj_next = t_obj->next_content;

		if (obj->pIndexData == t_obj->pIndexData
		&& !mlstr_cmp(obj->short_descr, t_obj->short_descr)) {
			if (IS_OBJ_STAT(t_obj, ITEM_INVENTORY)) {
				extract_obj(obj);
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
OBJ_DATA * get_obj_keeper(CHAR_DATA * ch, CHAR_DATA * keeper, const char *argument)
{
	char            arg[MAX_INPUT_LENGTH];
	OBJ_DATA       *obj;
	int             number;
	int             count;
	number = number_argument(argument, arg);
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
			  && obj->pIndexData == obj->next_content->pIndexData
			  && !mlstr_cmp(obj->short_descr, obj->next_content->short_descr))
				obj = obj->next_content;
		}
	}

	return NULL;
}

uint get_cost(CHAR_DATA * keeper, OBJ_DATA * obj, bool fBuy)
{
	SHOP_DATA *	pShop;
	uint		cost;

	if (obj == NULL || (pShop = keeper->pIndexData->pShop) == NULL)
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
			if (obj->pIndexData->item_type == pShop->buy_type[itype]) {
				cost = obj->cost * pShop->profit_sell / 100;
				break;
			}
		}

		if (!IS_OBJ_STAT(obj, ITEM_SELL_EXTRACT))
			for (obj2 = keeper->carrying; obj2; obj2 = obj2->next_content) {
				if (obj->pIndexData == obj2->pIndexData
				&&  !mlstr_cmp(obj->short_descr,
					       obj2->short_descr))
					return 0;
/*
	 	    if (IS_OBJ_STAT(obj2,ITEM_INVENTORY))
				cost /= 2;
		    else
		              	cost = cost * 3 / 4;
*/
			}
	}

	if (obj->pIndexData->item_type == ITEM_STAFF
	||  obj->pIndexData->item_type == ITEM_WAND) {
		if (obj->value[1] == 0)
			cost /= 4;
		else
			cost = cost * obj->value[2] / obj->value[1];
	}
	return cost;
}

void do_buy_pet(CHAR_DATA * ch, const char *argument)
{
	uint		cost, roll;
	char            arg[MAX_INPUT_LENGTH];
	CHAR_DATA	*pet;
	ROOM_INDEX_DATA *pRoomIndexNext;
	ROOM_INDEX_DATA *in_room;

	if (IS_NPC(ch))
		return;

	argument = one_argument(argument, arg);
	if (arg[0] == '\0') {
		char_puts("Buy what?\n\r", ch);
		return;
	}
	smash_tilde(arg);

	pRoomIndexNext = get_room_index(ch->in_room->vnum + 1);
	if (pRoomIndexNext == NULL) {
		bug("Do_buy: bad pet shop at vnum %d.", ch->in_room->vnum);
		char_puts("Sorry, you can't buy that here.\n\r", ch);
		return;
	}
	in_room = ch->in_room;
	ch->in_room = pRoomIndexNext;
	pet = get_char_room(ch, arg);
	ch->in_room = in_room;

	if (pet == NULL || !IS_SET(pet->act, ACT_PET) || !IS_NPC(pet)) {
		char_puts("Sorry, you can't buy that here.\n\r", ch);
		return;
	}
	if (IS_SET(pet->act, ACT_RIDEABLE)
	&&  get_skill(ch, gsn_riding) && !MOUNTED(ch)) {
		cost = 10 * pet->level * pet->level;

		if ((ch->silver + 100 * ch->gold) < cost) {
			char_puts("You can't afford it.\n\r", ch);
			return;
		}
		if (ch->level < pet->level + 5) {
			char_puts("You're not powerful enough "
				     "to master this pet.\n\r", ch);
			return;
		}
		deduct_cost(ch, cost);
		pet = create_mob(pet->pIndexData);
		pet->comm = COMM_NOTELL | COMM_NOSHOUT | COMM_NOCHANNELS;

		char_to_room(pet, ch->in_room);
		do_mount(ch, pet->name);
		char_puts("Enjoy your mount.\n\r", ch);
		act("$n bought $N as a mount.", ch, NULL, pet, TO_ROOM);
		return;
	}
	if (ch->pet != NULL) {
		char_puts("You already own a pet.\n\r", ch);
		return;
	}
	cost = 10 * pet->level * pet->level;

	if ((ch->silver + 100 * ch->gold) < cost) {
		char_puts("You can't afford it.\n\r", ch);
		return;
	}
	if (ch->level < pet->level) {
		char_puts("You're not powerful enough "
			     "to master this pet.\n\r", ch);
		return;
	}
	/* haggle */
	roll = number_percent();
	if (roll < get_skill(ch, gsn_haggle)) {
		cost -= cost / 2 * roll / 100;
		char_printf(ch, "You haggle the price down to %d coins.\n\r",
			    cost);
		check_improve(ch, gsn_haggle, TRUE, 4);
	}
	deduct_cost(ch, cost);
	pet = create_mob(pet->pIndexData);
	SET_BIT(pet->affected_by, AFF_CHARM);
	pet->comm = COMM_NOTELL | COMM_NOSHOUT | COMM_NOCHANNELS;

	argument = one_argument(argument, arg);
	if (arg[0] != '\0')
		pet->name = str_printf(pet->pIndexData->name, arg);
	pet->description = mlstr_printf(pet->pIndexData->description, ch->name);

	char_to_room(pet, ch->in_room);
	add_follower(pet, ch);
	pet->leader = ch;
	ch->pet = pet;
	char_puts("Enjoy your pet.\n\r", ch);
	act("$n bought $N as a pet.", ch, NULL, pet, TO_ROOM);
}

void do_buy(CHAR_DATA * ch, const char *argument)
{
	uint		cost, roll;
	CHAR_DATA      *keeper;
	OBJ_DATA       *obj, *t_obj;
	char            arg[MAX_INPUT_LENGTH];
	uint		number, count = 1;

	if ((keeper = find_keeper(ch)) == NULL)
		return;

	if (IS_SET(ch->in_room->room_flags, ROOM_PET_SHOP)) {
		do_buy_pet(ch, argument);
		return;
	}

	number = mult_argument(argument, arg);
	if (arg[0] == '\0' || number == 0) {
		char_puts("Buy what?\n\r", ch);
		return;
	}

	if (number < 0) {
		char_puts("What? Try sell instead.\n\r", ch);
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
			if (t_obj->pIndexData == obj->pIndexData
			&& !mlstr_cmp(t_obj->short_descr, obj->short_descr))
				count++;
			else
				break;
		}

		if (count < number) {
			act("$n tells you '{GI don't have that many in stock.{x'",
			    keeper, NULL, ch, TO_VICT);
			ch->reply = keeper;
			return;
		}
	}
	if ((ch->silver + ch->gold * 100) < cost * number) {
		if (number > 1)
			act("$n tells you '{GYou can't afford to buy that many.{x'",
			    keeper, obj, ch, TO_VICT);
		else
			act("$n tells you '{GYou can't afford to buy $p.{x'",
			    keeper, obj, ch, TO_VICT);
		ch->reply = keeper;
		return;
	}
	if (obj->level > ch->level) {
		act("$n tells you '{GYou can't use $p yet.{x'",
		    keeper, obj, ch, TO_VICT);
		ch->reply = keeper;
		return;
	}
	if (ch->carry_number + number * get_obj_number(obj) > can_carry_n(ch)) {
		char_puts("You can't carry that many items.\n\r", ch);
		return;
	}
	if (ch->carry_weight + number * get_obj_weight(obj) > can_carry_w(ch)) {
		char_puts("You can't carry that much weight.\n\r", ch);
		return;
	}
	/* haggle */
	roll = number_percent();
	if (!IS_OBJ_STAT(obj, ITEM_SELL_EXTRACT)
	    && roll < get_skill(ch, gsn_haggle)) {
		cost -= obj->cost / 2 * roll / 100;
		act("You haggle with $N.", ch, NULL, keeper, TO_CHAR);
		check_improve(ch, gsn_haggle, TRUE, 4);
	}
	if (number > 1) {
		act_printf(ch, obj, NULL, TO_ROOM, POS_RESTING,
			   "$n buys $p[%d].", number);
		act_printf(ch, obj, NULL, TO_CHAR, POS_RESTING,
			   "You buy $p[%d] for %d silver.",
			   number, cost * number);
	} else {
		act("$n buys $p.", ch, obj, NULL, TO_ROOM);
		act_printf(ch, obj, NULL, TO_CHAR, POS_RESTING,
			   "You buy $p for %d silver.", cost);
	}

	deduct_cost(ch, cost * number);
	keeper->gold += cost * number / 100;
	keeper->silver += cost * number - (cost * number / 100) * 100;

	for (count = 0; count < number; count++) {
		if (IS_SET(obj->extra_flags, ITEM_INVENTORY))
			t_obj = create_obj(obj->pIndexData, obj->level);
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
			char_puts("You can't do that here.\n\r", ch);
			return;
		}
		found = FALSE;
		for (pet = pRoomIndexNext->people; pet; pet = pet->next_in_room) {
			if (!IS_NPC(pet))
				continue;	/* :) */
			if (IS_SET(pet->act, ACT_PET)) {
				if (!found) {
					found = TRUE;
					char_puts("Pets for sale:\n\r", ch);
				}
				char_printf(ch, "[%2d] %8d - %s\n\r",
					    pet->level,
					    10 * pet->level * pet->level,
					    mlstr_cval(pet->short_descr, ch));
			}
		}
		if (!found)
			char_puts("Sorry, we're out of pets right now.\n\r", ch);
		return;
	} else {
		CHAR_DATA      *keeper;
		OBJ_DATA       *obj;
		int             cost, count;
		bool            found;
		char            arg[MAX_INPUT_LENGTH];
		if ((keeper = find_keeper(ch)) == NULL)
			return;
		one_argument(argument, arg);

		found = FALSE;
		for (obj = keeper->carrying; obj; obj = obj->next_content) {
			if (obj->wear_loc == WEAR_NONE
			    && can_see_obj(ch, obj)
			    && (cost = get_cost(keeper, obj, TRUE)) > 0
			    && (arg[0] == '\0'
				|| is_name(arg, obj->name))) {
				if (!found) {
					found = TRUE;
					char_puts("[Lv Price Qty] Item\n\r", ch);
				}
				if (IS_OBJ_STAT(obj, ITEM_INVENTORY))
					char_printf(ch, "[%2d %5d -- ] %s\n\r",
					 obj->level, cost,
					mlstr_cval(obj->short_descr, ch));
				else {
					count = 1;

					while (obj->next_content != NULL
					       && obj->pIndexData == obj->next_content->pIndexData
					       && !mlstr_cmp(obj->short_descr,
					   obj->next_content->short_descr)) {
						obj = obj->next_content;
						count++;
					}
					char_printf(ch, "[%2d %5d %2d ] %s\n\r",
						    obj->level, cost, count,
						    mlstr_cval(obj->short_descr, ch));
				}
			}
		}

		if (!found)
			char_puts("You can't buy anything here.\n\r", ch);
		return;
	}
}

void do_sell(CHAR_DATA * ch, const char *argument)
{
	char            arg[MAX_INPUT_LENGTH];
	CHAR_DATA      *keeper;
	OBJ_DATA       *obj;
	uint		cost, roll;
	uint		gold, silver;

	one_argument(argument, arg);

	if (arg[0] == '\0') {
		char_puts("Sell what?\n\r", ch);
		return;
	}
	if ((keeper = find_keeper(ch)) == NULL)
		return;

	if ((obj = get_obj_carry(ch, arg)) == NULL) {
		act("$n tells you '{GYou don't have that item.{x'",
		    keeper, NULL, ch, TO_VICT);
		ch->reply = keeper;
		return;
	}
	if (!can_drop_obj(ch, obj)) {
		char_puts("You can't let go of it.\n\r", ch);
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
	if (!IS_OBJ_STAT(obj, ITEM_SELL_EXTRACT) && roll < get_skill(ch, gsn_haggle)) {
		roll = get_skill(ch, gsn_haggle) + number_range(1, 20) - 10;
		char_puts("You haggle with the shopkeeper.\n\r", ch);
		cost += obj->cost / 2 * roll / 100;
		cost = UMIN(cost, 95 * get_cost(keeper, obj, TRUE) / 100);
		cost = UMIN(cost, (keeper->silver + 100 * keeper->gold));
		check_improve(ch, gsn_haggle, TRUE, 4);
	}
	silver = cost - (cost / 100) * 100;
	gold = cost / 100;

	if (gold && silver)
		act_printf(ch, obj, NULL, TO_CHAR, POS_RESTING,
			   "You sell $p for %d gold and %d silver pieces.",
			   gold, silver);
	else if (gold)
		act_printf(ch, obj, NULL, TO_CHAR, POS_RESTING,
			   "You sell $p for %d gold pieces%s.",
			   gold, gold > 1 ? "s" : str_empty);
	else if (silver)
		act_printf(ch, obj, NULL, TO_CHAR, POS_RESTING,
			   "You sell $p for %d silver pieces%s.",
			   silver, silver > 1 ? "s" : str_empty);
	ch->gold += gold;
	ch->silver += silver;
	deduct_cost(keeper, cost);

	if (obj->pIndexData->item_type == ITEM_TRASH
	||  IS_OBJ_STAT(obj, ITEM_SELL_EXTRACT))
		extract_obj(obj);
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
	one_argument(argument, arg);

	if (arg[0] == '\0') {
		char_puts("Value what?\n\r", ch);
		return;
	}
	if ((keeper = find_keeper(ch)) == NULL)
		return;

	if ((obj = get_obj_carry(ch, arg)) == NULL) {
		act("$n tells you '{GYou don't have that item.{x'",
		    keeper, NULL, ch, TO_VICT);
		ch->reply = keeper;
		return;
	}
	if (!can_see_obj(keeper, obj)) {
		act("$n doesn't see what you are offering.", keeper, NULL, ch, TO_VICT);
		return;
	}
	if (!can_drop_obj(ch, obj)) {
		char_puts("You can't let go of it.\n\r", ch);
		return;
	}
	if ((cost = get_cost(keeper, obj, FALSE)) <= 0) {
		act("$n looks uninterested in $p.", keeper, obj, ch, TO_VICT);
		return;
	}
	act_printf(keeper, obj, ch, TO_VICT, POS_RESTING,
		   "$n tells you '{GI'll give you %d silver and %d gold coins for $p.{x'",
		   cost - (cost / 100) * 100, cost / 100);
	ch->reply = keeper;

	return;
}

void do_herbs(CHAR_DATA * ch, const char *argument)
{
	CHAR_DATA      *victim;
	char            arg[MAX_INPUT_LENGTH];
	int		sn;

	if (IS_NPC(ch)
	||  (sn = sn_lookup("herbs")) < 0)
		return;

	one_argument(argument, arg);

	if (is_affected(ch, sn)) {
		char_puts("You can't find any more herbs.\n\r", ch);
		return;
	}

	WAIT_STATE(ch, SKILL(sn)->beats);

	if (arg[0] == '\0')
		victim = ch;
	else if ((victim = get_char_room(ch, arg)) == NULL) {
		char_puts("They're not here.\n\r", ch);
		return;
	}

	if (ch->in_room->sector_type != SECT_INSIDE
	&&  ch->in_room->sector_type != SECT_CITY
	&&  number_percent() < get_skill(ch, sn)) {
		AFFECT_DATA     af;
		af.where = TO_AFFECTS;
		af.type = sn;
		af.level = ch->level;
		af.duration = 5;
		af.location = APPLY_NONE;
		af.modifier = 0;
		af.bitvector = 0;

		affect_to_char(ch, &af);

		char_puts("You gather some beneficial herbs.\n\r", ch);
		act("$n gathers some herbs.", ch, NULL, NULL, TO_ROOM);

		if (ch != victim) {
			act("$n gives you some herbs to eat.", ch, NULL, victim, TO_VICT);
			act("You give the herbs to $N.", ch, NULL, victim, TO_CHAR);
			act("$n gives the herbs to $N.", ch, NULL, victim, TO_NOTVICT);
		}
		if (victim->hit < victim->max_hit) {
			char_puts("You feel better.\n\r", victim);
			act("$n looks better.", victim, NULL, NULL, TO_ROOM);
		}
		victim->hit = UMIN(victim->max_hit, victim->hit + 5 * ch->level);
		check_improve(ch, sn, TRUE, 1);
		if (is_affected(victim, gsn_plague)) {
			if (check_dispel(ch->level, victim, gsn_plague)) {
				char_puts("Your sores vanish.\n\r", victim);
				act("$n looks relieved as $s sores vanish.",
				    victim, NULL, NULL, TO_ROOM);
			}
		}
	} else {
		char_puts("You search for herbs but find none here.\n\r", ch);
		act("$n looks around for herbs.", ch, NULL, NULL, TO_ROOM);
		check_improve(ch, sn, FALSE, 1);
	}
}

void do_lore_raw(CHAR_DATA *ch, OBJ_DATA *obj, BUFFER *output)
{
	int		chance;
	int		percent;
	int		value0, value1, value2, value3, value4;
	int		mana;
	int		max_skill;
	int		sn;

	if ((sn = sn_lookup("lore")) < 0
	||  (percent = get_skill(ch, sn)) < 10) {
		buf_add(output, "The meaning of this object escapes you for the moment.\n\r");
		return;
	}

	mana = SKILL(sn)->min_mana;
	if (ch->mana < mana) {
		buf_add(output, "You don't have enough mana.\n\r");
		return;
	}
	ch->mana -= mana;
	WAIT_STATE(ch, SKILL(sn)->beats);

	/* a random lore */
	chance = number_percent();

	if (percent < 20) {
		buf_printf(output, "Object '%s'.\n\r", obj->name);
		check_improve(ch, sn, TRUE, 8);
		return;
	}
	else if (percent < 40) {
		buf_printf(output,
			    "Object '%s'.  Weight is %d, value is %d.\n\r",
			    obj->name,
		chance < 60 ? obj->weight : number_range(1, 2 * obj->weight),
		     chance < 60 ? number_range(1, 2 * obj->cost) : obj->cost
			);
		if (str_cmp(obj->material, "oldstyle"))
			buf_printf(output, "Material is %s.\n\r", obj->material);
		check_improve(ch, sn, TRUE, 7);
		return;
	}
	else if (percent < 60) {
		buf_printf(output,
			    "Object '%s' has weight %d.\n\rValue is %d, level is %d.\n\rMaterial is %s.\n\r",
			    obj->name,
			    obj->weight,
		    chance < 60 ? number_range(1, 2 * obj->cost) : obj->cost,
		  chance < 60 ? obj->level : number_range(1, 2 * obj->level),
		str_cmp(obj->material, "oldstyle") ? obj->material : "unknown"
			);
		check_improve(ch, sn, TRUE, 6);
		return;
	}
	else if (percent < 80) {
		buf_printf(output,
			    "Object '%s' is type %s, extra flags %s.\n\rWeight is %d, value is %d, level is %d.\n\rMaterial is %s.\n\r",
			    obj->name,
			    flag_string(item_types, obj->pIndexData->item_type),
			    flag_string(extra_flags, obj->extra_flags),
			    obj->weight,
		    chance < 60 ? number_range(1, 2 * obj->cost) : obj->cost,
		  chance < 60 ? obj->level : number_range(1, 2 * obj->level),
		str_cmp(obj->material, "oldstyle") ? obj->material : "unknown"
			);
		check_improve(ch, sn, TRUE, 5);
		return;
	}
	else if (percent < 85) 
		buf_printf(output,
			    "Object '%s' is type %s, extra flags %s.\n\rWeight is %d, value is %d, level is %d.\n\rMaterial is %s.\n\r",
			    obj->name,
			    flag_string(item_types, obj->pIndexData->item_type),
			    flag_string(extra_flags, obj->extra_flags),
			    obj->weight,
			    obj->cost,
			    obj->level,
			    str_cmp(obj->material, "oldstyle") ?
				obj->material : "unknown"
			);
	else
		buf_printf(output,
			    "Object '%s' is type %s, extra flags %s.\n\rWeight is %d, value is %d, level is %d.\n\rMaterial is %s.\n\r",
			    obj->name,
			    flag_string(item_types, obj->pIndexData->item_type),
			    flag_string(extra_flags, obj->extra_flags),
			    obj->weight,
			    obj->cost,
			    obj->level,
			    str_cmp(obj->material, "oldstyle") ?
				obj->material : "unknown"
			);

	value0 = obj->value[0];
	value1 = obj->value[1];
	value2 = obj->value[2];
	value3 = obj->value[3];
	value4 = obj->value[4];

	max_skill = skills.nused;

	switch (obj->pIndexData->item_type) {
	case ITEM_SCROLL:
	case ITEM_POTION:
	case ITEM_PILL:
		if (percent < 85) {
			value0 = number_range(1, 60);
			if (chance > 40) {
				value1 = number_range(1, (max_skill - 1));
				if (chance > 60) {
					value2 = number_range(1, (max_skill - 1));
					if (chance > 80)
						value3 = number_range(1, (max_skill - 1));
				}
			}
		}
		else {
			if (chance > 60) {
				value1 = number_range(1, (max_skill - 1));
				if (chance > 80) {
					value2 = number_range(1, (max_skill - 1));
					if (chance > 95)
						value3 = number_range(1, (max_skill - 1));
				}
			}
		}

		buf_printf(output, "Level %d spells of:", obj->value[0]);
		if (value1 >= 0)
			buf_printf(output, " '%s'", skill_name(value1));
		if (value2 >= 0)
			buf_printf(output, " '%s'", skill_name(value2));
		if (value3 >= 0)
			buf_printf(output, " '%s'", skill_name(value3));
		if (value4 >= 0)
			buf_printf(output, " '%s'", skill_name(value4));
		buf_add(output, ".\n\r");
		break;

	case ITEM_WAND:
	case ITEM_STAFF:
		if (percent < 85) {
			value0 = number_range(1, 60);
			if (chance > 40) {
				value3 = number_range(1, (max_skill - 1));
				if (chance > 60) {
					value2 = number_range(0, 2 * obj->value[2]);
					if (chance > 80)
						value1 = number_range(0, value2);
				}
			}
		}
		else {
			if (chance > 60) {
				value3 = number_range(1, (max_skill - 1));
				if (chance > 80) {
					value2 = number_range(0, 2 * obj->value[2]);
					if (chance > 95)
						value1 = number_range(0, value2);
				}
			}
		}

		buf_printf(output, "Has %d(%d) charges of level %d '%s'.\n\r",
			    value1, value2, value0, skill_name(value3));
		break;

	case ITEM_WEAPON:
		buf_add(output, "Weapon type is ");
		if (percent < 85) {
			value0 = number_range(0, 8);
			if (chance > 33) {
				value1 = number_range(1, 2 * obj->value[1]);
				if (chance > 66)
					value2 = number_range(1, 2 * obj->value[2]);
			}
		}
		else {
			if (chance > 50) {
				value1 = number_range(1, 2 * obj->value[1]);
				if (chance > 75)
					value2 = number_range(1, 2 * obj->value[2]);
			}
		}

		buf_printf(output, "%s.\n\r", flag_string(weapon_class, value0));

		buf_printf(output, "Damage is %dd%d (average %d).\n\r",
			    value1, value2,
			    (1 + value2) * value1 / 2);
		break;

	case ITEM_ARMOR:
		if (percent < 85) {
			if (chance > 25) {
				value2 = number_range(0, 2 * obj->value[2]);
				if (chance > 45) {
					value0 = number_range(0, 2 * obj->value[0]);
					if (chance > 65) {
						value3 = number_range(0, 2 * obj->value[3]);
						if (chance > 85)
							value1 = number_range(0, 2 * obj->value[1]);
					}
				}
			}
		}
		else {
			if (chance > 45) {
				value2 = number_range(0, 2 * obj->value[2]);
				if (chance > 65) {
					value0 = number_range(0, 2 * obj->value[0]);
					if (chance > 85) {
						value3 = number_range(0, 2 * obj->value[3]);
						if (chance > 95)
							value1 = number_range(0, 2 * obj->value[1]);
					}
				}
			}
		}

		buf_printf(output,
			    "Armor class is %d pierce, %d bash, %d slash, and %d vs. magic.\n\r",
			    value0, value1, value2, value3);
		break;
	}

	if (percent < 87) {
		check_improve(ch, sn, TRUE, 5);
		return;
	}

	if (!IS_SET(obj->extra_flags, ITEM_ENCHANTED))
		format_obj_affects(output, obj->pIndexData->affected, TRUE);
	format_obj_affects(output, obj->affected, TRUE);
	check_improve(ch, sn, TRUE, 5);
}

void do_lore(CHAR_DATA *ch, const char *argument)
{
	char		arg[MAX_INPUT_LENGTH];
	BUFFER *	output;
	OBJ_DATA *	obj;

	argument = one_argument(argument, arg);
	if ((obj = get_obj_carry(ch, arg)) == NULL) {
		char_puts("You do not have that object.\n\r", ch);
		return;
	}

	output = buf_new(0);
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
	int		sn;
	int		chance;

	one_argument(argument, arg);
	if (arg[0] == '\0') {
		char_puts("Butcher what?\n\r", ch);
		return;
	}
	if ((obj = get_obj_here(ch, arg)) == NULL) {
		char_puts("You do not see that here.\n\r", ch);
		return;
	}
	if (obj->pIndexData->item_type != ITEM_CORPSE_PC
	&&  obj->pIndexData->item_type != ITEM_CORPSE_NPC) {
		char_puts("You can't butcher that.\n\r", ch);
		return;
	}
	if (obj->carried_by != NULL) {
		char_puts("Put it down first.\n\r", ch);
		return;
	}

	if ((sn = sn_lookup("butcher")) < 0
	||  (chance = get_skill(ch, sn)) == 0) {
		char_puts("You don't have the precision instruments "
			     "for that.", ch);
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

		if (numsteaks > 1) {
			act_printf(ch, obj, NULL, TO_ROOM, POS_RESTING,
				   "$n butchers $p and creates %i steaks.",
				   numsteaks);
			act_printf(ch, obj, NULL, TO_CHAR, POS_RESTING,
				   "You butcher $p and create %i steaks.",
				   numsteaks);
		} else {
			act("$n butchers $p and creates a steak.",
			    ch, obj, NULL, TO_ROOM);
			act("You butcher $p and create a steak.",
			    ch, obj, NULL, TO_CHAR);
		}
		check_improve(ch, sn, TRUE, 1);

		for (i = 0; i < numsteaks; i++) {
			steak = create_named_obj(get_obj_index(OBJ_VNUM_STEAK), 0, mlstr_mval(obj->short_descr));
			obj_to_room(steak, ch->in_room);
		}
	} else {
		act("You fail and destroy $p.", ch, obj, NULL, TO_CHAR);
		act("$n fails to butcher $p and destroys it.",
		    ch, obj, NULL, TO_ROOM);

		check_improve(ch, sn, FALSE, 1);
	}
	extract_obj(obj);
}

void do_balance(CHAR_DATA * ch, const char *argument)
{
	char            buf[160];
	uint            bank_g;
	uint            bank_s;

	if (IS_NPC(ch)) {
		char_puts("You don't have a bank account.\n\r", ch);
		return;
	}
	if (!IS_SET(ch->in_room->room_flags, ROOM_BANK)) {
		char_puts("You are not in a bank.\n\r", ch);
		return;
	}

	if (ch->pcdata->bank_s + ch->pcdata->bank_g == 0) {
		char_puts("You don't have any money in the bank.\n\r", ch);
		return;
	}

	bank_g = ch->pcdata->bank_g;
	bank_s = ch->pcdata->bank_s;
	snprintf(buf, sizeof(buf), "You have %s%s%s coin%s in the bank.\n\r",
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
	uint	fee;
	bool	silver = FALSE;
	char	arg[MAX_INPUT_LENGTH];

	if (IS_NPC(ch)) {
		char_puts("You don't have a bank account.\n\r", ch);
		return;
	}

	if (!IS_SET(ch->in_room->room_flags, ROOM_BANK)) {
		char_puts("The mosquito by your feet "
			  "will not give you any money.\n\r", ch);
		return;
	}

	argument = one_argument(argument, arg);
	if (arg[0] == '\0') {
		char_puts("Withdraw how much?\n\r", ch);
		return;
	}

	amount = atoi(arg);
	if (amount <= 0) {
		char_puts("What?\n\r", ch);
		return;
	}

	if (!str_cmp(argument, "silver"))
		silver = TRUE;
	else if (str_cmp(argument, "gold") && argument[0] != '\0') {
		char_puts("You can withdraw gold and silver coins only.", ch);
		return;
	}

	if ((silver && amount > ch->pcdata->bank_s)
	||  (!silver && amount > ch->pcdata->bank_g)) {
		char_puts("Sorry, we don't give loans.\n\r", ch);
		return;
	}

	fee = UMAX(1, amount * (silver ? 10 : 2) / 100);
	
	if (get_carry_weight(ch) + (amount - fee) * (silver ? 4 : 1) / 10 >
							can_carry_w(ch)) {
		char_puts("You can't carry that weight.\n\r", ch);
		return;
	}

	if (silver) {
		ch->silver += amount - fee;
		ch->pcdata->bank_s -= amount;
	}
	else {
		ch->gold += amount - fee;
		ch->pcdata->bank_g -= amount;
	}

	char_printf(ch,
		    "Here are your %d %s coin(s), "
		    "minus a %d coin(s) withdrawal fee.\n\r",
		    amount, silver ? "silver" : "gold", fee);
	act("$n steps up to the teller window.", ch, NULL, NULL, TO_ROOM);
}

void do_deposit(CHAR_DATA * ch, const char *argument)
{
	int	amount;
	bool	silver = FALSE;
	char	arg[MAX_INPUT_LENGTH];

	if (IS_NPC(ch)) {
		char_puts("You don't have a bank account.\n\r", ch);
		return;
	}

	if (!IS_SET(ch->in_room->room_flags, ROOM_BANK)) {
		char_puts("The ant by your feet can't carry your gold.\n\r", ch);
		return;
	}

	argument = one_argument(argument, arg);
	if (arg[0] == '\0') {
		char_puts("Deposit how much?\n\r", ch);
		return;
	}

	amount = atoi(arg);
	if (amount <= 0) {
		char_puts("What?\n\r", ch);
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
		char_puts("That's more than you've got.\n\r", ch);
		return;
	}

	if (silver) {
		ch->pcdata->bank_s += amount;
		ch->silver -= amount;
	}
	else {
		ch->pcdata->bank_g += amount;
		ch->gold -= amount;
	}

	if (amount == 1)
		char_printf(ch, "Oh boy! One %s coin!\n\r",
			    silver ? "silver" : "gold");
	else
		char_printf(ch, "%d %s coins deposited. Come again soon!\n\r",
			    amount, silver ? "silver" : "gold");
	act("$n steps up to the teller window.", ch, NULL, NULL, TO_ROOM);
}

/* wear object as a secondary weapon */
void do_second_wield(CHAR_DATA * ch, const char *argument)
{
	OBJ_DATA       *obj;
	int             sn, skill;
	int		wear_lev;

	if (get_skill(ch, gsn_second_weapon) == 0) {
		char_puts("You don't know how to wield a second weapon.\n\r", ch);
		return;
	}
	if (argument[0] == '\0') {
		char_puts("Wear which weapon in your off-hand?\n\r", ch);
		return;
	}
	obj = get_obj_carry(ch, argument);
	if (obj == NULL) {
		char_puts("You don't have that item.\n\r", ch);
		return;
	}
	if (!CAN_WEAR(obj, ITEM_WIELD)) {
		char_puts("You can't wield that as your second weapon.\n\r", ch);
		return;
	}
	if ((get_eq_char(ch, WEAR_SHIELD) != NULL) ||
	    (get_eq_char(ch, WEAR_HOLD) != NULL)) {
		char_puts("You cannot use a secondary weapon while using a shield or holding an item.\n\r", ch);
		return;
	}
	wear_lev = get_wear_level(ch, obj);
	if (wear_lev < obj->level) {
		char_printf(ch, "You must be level %d to use this object.\n\r",
			    wear_lev);
		act("$n tries to use $p, but is too inexperienced.",
		    ch, obj, NULL, TO_ROOM);
		return;
	}
	if (IS_WEAPON_STAT(obj, WEAPON_TWO_HANDS)) {
		char_puts("You can't dual wield two-handed weapon!\n\r", ch);
		return;
	}
	if (get_eq_char(ch, WEAR_WIELD) == NULL) {
		char_puts("You need to wield a primary weapon, before using a secondary one!\n\r", ch);
		return;
	}
	if (get_obj_weight(obj) > (str_app[get_curr_stat(ch, STAT_STR)].wield * 5)) {
		char_puts("This weapon is too heavy to be used as a secondary weapon by you.\n\r", ch);
		return;
	}
	if (!remove_obj(ch, WEAR_SECOND_WIELD, TRUE))
		return;

	act("$n wields $p in $s off-hand.", ch, obj, NULL, TO_ROOM);
	act("You wield $p in your off-hand.", ch, obj, NULL, TO_CHAR);
	equip_char(ch, obj, WEAR_SECOND_WIELD);

	sn = get_weapon_sn(ch, WEAR_SECOND_WIELD);
	if (sn) {
		skill = get_weapon_skill(ch, sn);

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
}

void do_enchant(CHAR_DATA * ch, const char *argument)
{
	OBJ_DATA *	obj;
	int		chance;
	int		sn;
	int		wear_level;

	if ((sn = sn_lookup("enchant sword")) < 0
	||  (chance = get_skill(ch, sn)) == 0) {
		char_puts("Huh?\n\r", ch);
		return;
	}

	if (argument[0] == '\0') {	/* empty */
		char_puts("Wear which weapon to enchant?\n\r", ch);
		return;
	}
	obj = get_obj_carry(ch, argument);

	if (obj == NULL) {
		char_puts("You don't have that item.\n\r", ch);
		return;
	}

	wear_level = get_wear_level(ch, obj);
	if (wear_level < obj->level) {
		char_printf(ch, "You must be level %d to be able to enchant this object.\n\r",
			    obj->level - wear_level + ch->level);
		act("$n tries to enchant $p, but is too inexperienced.",
		    ch, obj, NULL, TO_ROOM);
		return;
	}
	if (ch->mana < 100) {
		char_puts("You don't have enough mana.\n\r", ch);
		return;
	}

	WAIT_STATE(ch, SKILL(sn)->beats);
	if (number_percent() > chance) {
		char_puts("You lost your concentration.\n\r", ch);
		act("$n tries to enchant $p, but he forgets how for a moment.",
		    ch, obj, NULL, TO_ROOM);
		check_improve(ch, sn, FALSE, 6);
		ch->mana -= 50;
		return;
	}
	ch->mana -= 100;
	spell_enchant_weapon(24, ch->level, ch, obj, TARGET_OBJ);
	check_improve(ch, sn, TRUE, 2);
}
