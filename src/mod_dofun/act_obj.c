/*
 * $Id: act_obj.c,v 1.52 1998-08-06 10:03:45 fjoe Exp $
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
#include "comm.h"
#include "db.h"
#include "act_wiz.h"
#include "act_comm.h"
#include "magic.h"
#include "quest.h"
#include "update.h"
#include "util.h"
#include "resource.h"
#include "act_obj.h"
#include "log.h"
#include "mob_prog.h"
#include "obj_prog.h"
#include "mlstring.h"
#include "fight.h"

/* command procedures needed */
DECLARE_DO_FUN(do_split);
DECLARE_DO_FUN(do_yell);
DECLARE_DO_FUN(do_say);
DECLARE_DO_FUN(do_mount);



/*
 * Local functions.
 */
#define CD CHAR_DATA
#define OD OBJ_DATA

DECLARE_SPELL_FUN(spell_enchant_weapon);

bool remove_obj args((CHAR_DATA * ch, int iWear, bool fReplace));
void wear_obj   args((CHAR_DATA * ch, OBJ_DATA * obj, bool fReplace));
CD             *find_keeper args((CHAR_DATA * ch));
int get_cost    args((CHAR_DATA * keeper, OBJ_DATA * obj, bool fBuy));
void obj_to_keeper args((OBJ_DATA * obj, CHAR_DATA * ch));
OD             *get_obj_keeper args((CHAR_DATA * ch, CHAR_DATA * keeper, const char *argument));
void do_sacr    args((CHAR_DATA * ch, const char *argument));
#undef OD
#undef CD

AFFECT_DATA    *affect_find(AFFECT_DATA * paf, int sn);


/* RT part of the corpse looting code */

bool 
can_loot(CHAR_DATA * ch, OBJ_DATA * obj)
{
	if (IS_IMMORTAL(ch))
		return TRUE;

	/*
	 * PC corpses in the ROOM_BATTLE_ARENA rooms can be looted
	 * only by owners
	 */
	if (obj->in_room != NULL
	&& IS_SET(obj->in_room->room_flags, ROOM_BATTLE_ARENA)
	&& obj->from != NULL && str_cmp(ch->name, obj->from))
		return FALSE;

	return TRUE;
}


void 
get_obj(CHAR_DATA * ch, OBJ_DATA * obj, OBJ_DATA * container)
{
	/* variables for AUTOSPLIT */
	CHAR_DATA      *gch;
	int             members;

	/* can't take corpses in ROOM_BATTLE_ARENA rooms */
	if (!CAN_WEAR(obj, ITEM_TAKE)
	||  (obj->item_type == ITEM_CORPSE_PC &&
	     obj->in_room != NULL &&
	     IS_SET(obj->in_room->room_flags, ROOM_BATTLE_ARENA) &&
	     obj->from != NULL &&
	     str_cmp(ch->name, obj->from))) {
		char_nputs(YOU_CANT_TAKE_THAT, ch);
		return;
	}

	/* can't even get limited eq which does not match alignment */
	if (obj->pIndexData->limit != -1) {
		if ((IS_OBJ_STAT(obj, ITEM_ANTI_EVIL) && IS_EVIL(ch))
		||  (IS_OBJ_STAT(obj, ITEM_ANTI_GOOD) && IS_GOOD(ch))
		||  (IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch))) {
			act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD,
				    YOU_ZAPPED_BY_P);
			act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING,
				    N_ZAPPED_BY_P);
			return;
		}
	}

	if (ch->carry_number + get_obj_number(obj) > can_carry_n(ch)) {
		act_nprintf(ch, NULL, obj->name, TO_CHAR, POS_DEAD,
			    CANT_CARRY_ITEMS);
		return;
	}

	if (get_carry_weight(ch) + get_obj_weight(obj) > can_carry_w(ch)) {
		act_nprintf(ch, NULL, obj->name, TO_CHAR, POS_DEAD,
			    CANT_CARRY_WEIGHT);
		return;
	}

	if (obj->in_room != NULL) {
		for (gch = obj->in_room->people; gch != NULL;
		     gch = gch->next_in_room)
			if (gch->on == obj) {
				act_nprintf(ch, obj, gch, TO_CHAR, POS_DEAD,
					    N_APPEARS_USING);
				return;
			}
	}
	if (container != NULL) {
		if (container->pIndexData->vnum == OBJ_VNUM_INVADER_SKULL
		    || container->pIndexData->vnum == OBJ_VNUM_RULER_STAND
		    || container->pIndexData->vnum == OBJ_VNUM_BATTLE_THRONE
		    || container->pIndexData->vnum == OBJ_VNUM_CHAOS_ALTAR
		    || container->pIndexData->vnum == OBJ_VNUM_SHALAFI_ALTAR
		    || container->pIndexData->vnum == OBJ_VNUM_KNIGHT_ALTAR
		    || container->pIndexData->vnum == OBJ_VNUM_LIONS_ALTAR
		    || container->pIndexData->vnum == OBJ_VNUM_HUNTER_ALTAR) {
			act_nprintf(ch, obj, container, TO_CHAR, POS_DEAD,
				    YOU_GET_P_FROM_P);
			if (!IS_AFFECTED(ch, AFF_SNEAK))
				act_nprintf(ch, obj, container, TO_ROOM,
					    POS_RESTING, N_GETS_P_FROM_P);
			obj_from_obj(obj);
			act("$p fades to black, then dissapears!", ch,
			    container, NULL, TO_ROOM);
			act("$p fades to black, then dissapears!", ch,
			    container, NULL, TO_CHAR);
			extract_obj(container);
			obj_to_char(obj, ch);

			oprog_call(OPROG_GET, obj, ch, NULL);
			return;
		}
		if (obj_is_pit(container)
		&&  !CAN_WEAR(container, ITEM_TAKE)
		&&  !IS_OBJ_STAT(obj, ITEM_HAD_TIMER))
			obj->timer = 0;
		act_nprintf(ch, obj, container, TO_CHAR, POS_DEAD,
			    YOU_GET_P_FROM_P);
		if (!IS_AFFECTED(ch, AFF_SNEAK))
			act_nprintf(ch, obj, container, TO_ROOM,
				    POS_RESTING, N_GETS_P_FROM_P);
		REMOVE_BIT(obj->extra_flags, ITEM_HAD_TIMER);
		obj_from_obj(obj);
	} else {
		act_nprintf(ch, obj, container, TO_CHAR, POS_DEAD, YOU_GET_P);
		if (!IS_AFFECTED(ch, AFF_SNEAK))
			act_nprintf(ch, obj, container, TO_ROOM, POS_RESTING,
				    N_GETS_P);
		obj_from_room(obj);
	}

	if (obj->item_type == ITEM_MONEY) {
		if (get_carry_weight(ch) + obj->value[0] / 10
		    + obj->value[1] * 2 / 5 > can_carry_w(ch)) {
			act_nprintf(ch, NULL, obj->name, TO_CHAR, POS_DEAD,
				    CANT_CARRY_WEIGHT);
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
	} else {
		obj_to_char(obj, ch);
		oprog_call(OPROG_GET, obj, ch, NULL);
	}
}



void 
do_get(CHAR_DATA * ch, const char *argument)
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
		char_nputs(GET_WHAT, ch);
		return;
	}
	if (arg2[0] == '\0') {
		if (str_cmp(arg1, "all") && str_prefix("all.", arg1)) {
			/* 'get obj' */
			obj = get_obj_list(ch, arg1, ch->in_room->contents);
			if (obj == NULL) {
				act_nprintf(ch, NULL, arg1, TO_CHAR, POS_DEAD,
					    I_SEE_NO_T_HERE);
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
					char_nputs(I_SEE_NOTHING_HERE, ch);
				else
					act_nprintf(ch, NULL, &arg1[4], TO_CHAR,
						  POS_DEAD, I_SEE_NO_T_HERE);
			}
		}
		return;
	}
	/* 'get ... container' */
	if (!str_cmp(arg2, "all") || !str_prefix("all.", arg2)) {
		char_nputs(YOU_CANT_DO_THAT, ch);
		return;
	}
	if ((container = get_obj_here(ch, arg2)) == NULL) {
		act_nprintf(ch, NULL, arg2, TO_CHAR, POS_DEAD, I_SEE_NO_T_HERE);
		return;
	}
	switch (container->item_type) {
	default:
		char_nputs(THATS_NOT_CONTAINER, ch);
		return;

	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
		break;

	case ITEM_CORPSE_PC:
		if (!can_loot(ch, container)) {
			char_nputs(YOU_CANT_DO_THAT, ch);
			return;
		}
	}

	if (IS_SET(container->value[1], CONT_CLOSED)) {
		act_nprintf(ch, NULL, container->name, TO_CHAR, POS_DEAD,
			    THE_D_IS_CLOSED);
		return;
	}
	if (str_cmp(arg1, "all") && str_prefix("all.", arg1)) {
		/* 'get obj container' */
		obj = get_obj_list(ch, arg1, container->contains);
		if (obj == NULL) {
			act_nprintf(ch, NULL, arg2, TO_CHAR, POS_DEAD,
				    DONT_SEE_ANYTHING_LIKE_IN_T);
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
					char_nputs(DONT_BE_SO_GREEDY, ch);
					return;
				}
				get_obj(ch, obj, container);
			}
		}

		if (!found) {
			if (arg1[3] == '\0')
				act_nprintf(ch, NULL, arg2, TO_CHAR, POS_DEAD,
					    I_SEE_NOTHING_IN_T);
			else
				act_nprintf(ch, NULL, arg2, TO_CHAR, POS_DEAD,
					    DONT_SEE_ANYTHING_LIKE_IN_T);
		}
	}
}

void 
do_put(CHAR_DATA * ch, const char *argument)
{
	char            arg1[MAX_INPUT_LENGTH];
	char            arg2[MAX_INPUT_LENGTH];
	OBJ_DATA       *container;
	OBJ_DATA       *obj;
	OBJ_DATA       *obj_next;
	OBJ_DATA       *objc;
	int             pcount;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (!str_cmp(arg2, "in") || !str_cmp(arg2, "on"))
		argument = one_argument(argument, arg2);

	if (arg1[0] == '\0' || arg2[0] == '\0') {
		char_nputs(PUT_WHAT_IN_WHAT, ch);
		return;
	}
	if (!str_cmp(arg2, "all") || !str_prefix("all.", arg2)) {
		char_nputs(YOU_CANT_DO_THAT, ch);
		return;
	}
	if ((container = get_obj_here(ch, arg2)) == NULL) {
		act_nprintf(ch, NULL, arg2, TO_CHAR, POS_DEAD, I_SEE_NO_T_HERE);
		return;
	}
	if (container->item_type != ITEM_CONTAINER) {
		char_nputs(THATS_NOT_CONTAINER, ch);
		return;
	}
	if (IS_SET(container->value[1], CONT_CLOSED)) {
		act_nprintf(ch, NULL, container->name, TO_CHAR, POS_DEAD,
			    THE_D_IS_CLOSED);
		return;
	}
	if (str_cmp(arg1, "all") && str_prefix("all.", arg1)) {
		/* 'put obj container' */
		if ((obj = get_obj_carry(ch, arg1)) == NULL) {
			char_nputs(DONT_HAVE_ITEM, ch);
			return;
		}
		if (obj == container) {
			char_nputs(CANT_FOLD_INTO_SELF, ch);
			return;
		}
		if (!can_drop_obj(ch, obj)) {
			char_nputs(CANT_LET_GO_OF_IT, ch);
			return;
		}
		if (WEIGHT_MULT(obj) != 100) {
			char_nputs(THAT_WOULD_BE_BAD_IDEA, ch);
			return;
		}
		if (obj->pIndexData->limit != -1) {
			act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD,
				    UNWORTHY_CANT_HOLD);
			return;
		}
		if (IS_SET(container->value[1], CONT_FOR_ARROW)
		    && (obj->item_type != ITEM_WEAPON
			|| obj->value[0] != WEAPON_ARROW)) {
			act("You can only put arrows in $p.", ch, container, NULL, TO_CHAR);
			return;
		}
		if (get_obj_weight(obj) + get_true_weight(container)
		    > (container->value[0] * 10)
		    || get_obj_weight(obj) > (container->value[3] * 10)) {
			char_nputs(IT_WONT_FIT, ch);
			return;
		}
		if (obj->item_type == ITEM_POTION &&
		    IS_SET(container->wear_flags, ITEM_TAKE)) {
			pcount = 0;
			for (objc = container->contains; objc != NULL; objc = objc->next_content)
				if (objc->item_type == ITEM_POTION)
					pcount++;
			if (pcount > 15) {
				act("It's not safe to put more potions into $p.", ch, container, NULL, TO_CHAR);
				return;
			}
		}
		pcount = 0;
		for (objc = container->contains; objc != NULL; objc = objc->next_content)
			pcount++;
		if (pcount > container->value[0]) {
			act("It's not safe to put that much item into $p.", ch, container, NULL, TO_CHAR);
			return;
		}
		if (container->pIndexData->vnum == OBJ_VNUM_PIT
		    && !CAN_WEAR(container, ITEM_TAKE))
			if (obj->timer)
				SET_BIT(obj->extra_flags, ITEM_HAD_TIMER);
			else
				obj->timer = number_range(100, 200);

		obj_from_char(obj);
		obj_to_obj(obj, container);

		if (IS_SET(container->value[1], CONT_PUT_ON)) {
			act_nprintf(ch, obj, container, TO_ROOM, POS_RESTING,
				    N_PUTS_P_ON_P);
			act_nprintf(ch, obj, container, TO_CHAR, POS_DEAD,
				    YOU_PUT_P_ON_P);
		} else {
			act_nprintf(ch, obj, container, TO_ROOM, POS_RESTING,
				    N_PUTS_P_IN_P);
			act_nprintf(ch, obj, container, TO_CHAR, POS_DEAD,
				    YOU_PUT_P_IN_P);
		}
	} else {
		pcount = 0;
		for (objc = container->contains; objc != NULL; objc = objc->next_content)
			pcount++;

		/* 'put all container' or 'put all.obj container' */
		for (obj = ch->carrying; obj != NULL; obj = obj_next) {
			obj_next = obj->next_content;

			if ((arg1[3] == '\0' || is_name(&arg1[4], obj->name))
			    && can_see_obj(ch, obj)
			    && WEIGHT_MULT(obj) == 100
			    && obj->wear_loc == WEAR_NONE
			    && obj != container
			    && can_drop_obj(ch, obj)
			  && get_obj_weight(obj) + get_true_weight(container)
			    <= (container->value[0] * 10)
			&& get_obj_weight(obj) < (container->value[3] * 10)) {
				if (container->pIndexData->vnum == OBJ_VNUM_PIT
				    && !CAN_WEAR(obj, ITEM_TAKE))
					if (obj->timer)
						SET_BIT(obj->extra_flags, ITEM_HAD_TIMER);
					else
						obj->timer = number_range(100, 200);

				if (obj->pIndexData->limit != -1) {
					act_nprintf(ch, obj, NULL, TO_CHAR,
						  POS_DEAD, UNWORTHY_CANT_HOLD);
					continue;
				}
				if (obj->item_type == ITEM_POTION &&
				  IS_SET(container->wear_flags, ITEM_TAKE)) {
					pcount = 0;
					for (objc = container->contains; objc != NULL; objc = objc->next_content)
						if (objc->item_type == ITEM_POTION)
							pcount++;
					if (pcount > 15) {
						act("It's not safe to put more potions into $p.", ch, container, NULL, TO_CHAR);
						continue;
					}
				}
				pcount++;
				if (pcount > container->value[0]) {
					act("It's not safe to put that much item into $p.", ch, container, NULL, TO_CHAR);
					return;
				}
				obj_from_char(obj);
				obj_to_obj(obj, container);

				if (IS_SET(container->value[1], CONT_PUT_ON)) {
					act_nprintf(ch, obj, container, TO_ROOM,
						    POS_RESTING, N_PUTS_P_ON_P);
					act_nprintf(ch, obj, container, TO_CHAR,
						    POS_DEAD, YOU_PUT_P_ON_P);
				} else {
					act_nprintf(ch, obj, container, TO_ROOM,
						    POS_RESTING, N_PUTS_P_IN_P);
					act_nprintf(ch, obj, container, TO_CHAR,
						    POS_DEAD, YOU_PUT_P_IN_P);
				}
			}
		}
	}

	return;
}



void 
do_drop(CHAR_DATA * ch, const char *argument)
{
	char            arg[MAX_INPUT_LENGTH];
	OBJ_DATA       *obj;
	OBJ_DATA       *obj_next;
	bool            found;


	argument = one_argument(argument, arg);

	if (arg[0] == '\0') {
		char_nputs(DROP_WHAT, ch);
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
			char_nputs(YOU_CANT_DO_THAT, ch);
			return;
		}
		if (!str_cmp(arg, "coins") || !str_cmp(arg, "coin")
		    || !str_cmp(arg, "silver")) {
			if (ch->silver < amount) {
				char_nputs(DONT_HAVE_MUCH_SILVER, ch);
				return;
			}
			ch->silver -= amount;
			silver = amount;
		} else {
			if (ch->gold < amount) {
				char_nputs(DONT_HAVE_MUCH_GOLD, ch);
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
		if (!IS_AFFECTED(ch, AFF_SNEAK))
			act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING,
				    N_DROPS_SOME_COINS);
		char_nputs(OK, ch);
		if (IS_WATER(ch->in_room)) {
			extract_obj(obj);
			if (!IS_AFFECTED(ch, AFF_SNEAK))
				act("The coins sink down, and disapear in the water.", ch, NULL, NULL, TO_ROOM);
			act("The coins sink down, and disapear in the water.", ch, NULL, NULL, TO_CHAR);
		}
		return;
	}
	if (str_cmp(arg, "all") && str_prefix("all.", arg)) {
		/* 'drop obj' */
		if ((obj = get_obj_carry(ch, arg)) == NULL) {
			char_nputs(DONT_HAVE_ITEM, ch);
			return;
		}
		if (!can_drop_obj(ch, obj)) {
			char_nputs(CANT_LET_GO_OF_IT, ch);
			return;
		}
		obj_from_char(obj);
		obj_to_room(obj, ch->in_room);
		if (!IS_AFFECTED(ch, AFF_SNEAK))
			act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING, N_DROPS_P);
		act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD, YOU_DROP_P);
		if (obj->pIndexData->vnum == OBJ_VNUM_POTION_VIAL &&
		    number_percent() < 40)
			if (!IS_SET(ch->in_room->sector_type, SECT_FOREST) &&
			    !IS_SET(ch->in_room->sector_type, SECT_DESERT) &&
			    !IS_SET(ch->in_room->sector_type, SECT_AIR) &&
			    !IS_WATER(ch->in_room)) {
				act("$p cracks and shaters into tiny pieces.", ch, obj, NULL, TO_ROOM);
				act("$p cracks and shaters into tiny pieces.", ch, obj, NULL, TO_CHAR);
				extract_obj(obj);
				return;
			}

		oprog_call(OPROG_DROP, obj, ch, NULL);

		if (!may_float(obj) && cant_float(obj) && IS_WATER(ch->in_room)) {
			if (!IS_AFFECTED(ch, AFF_SNEAK))
				act("$p sinks down the water.", ch, obj, NULL, TO_ROOM);
			act("$p sinks down the water.", ch, obj, NULL, TO_CHAR);
			extract_obj(obj);
		} else if (IS_OBJ_STAT(obj, ITEM_MELT_DROP)) {
			if (!IS_AFFECTED(ch, AFF_SNEAK))
				act("$p dissolves into smoke.", ch, obj, NULL, TO_ROOM);
			act("$p dissolves into smoke.", ch, obj, NULL, TO_CHAR);
			extract_obj(obj);
		}
	} else {
		/* 'drop all' or 'drop all.obj' */
		found = FALSE;
		for (obj = ch->carrying; obj != NULL; obj = obj_next) {
			obj_next = obj->next_content;

			if ((arg[3] == '\0' || is_name(&arg[4], obj->name))
			    && can_see_obj(ch, obj)
			    && obj->wear_loc == WEAR_NONE
			    && can_drop_obj(ch, obj)) {
				found = TRUE;
				obj_from_char(obj);
				obj_to_room(obj, ch->in_room);
				if (!IS_AFFECTED(ch, AFF_SNEAK))
					act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING, N_DROPS_P);
				act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD, YOU_DROP_P);
				if (obj->pIndexData->vnum == OBJ_VNUM_POTION_VIAL &&
				    number_percent() < 70)
					if (!IS_SET(ch->in_room->sector_type, SECT_FOREST) &&
					    !IS_SET(ch->in_room->sector_type, SECT_DESERT) &&
					    !IS_SET(ch->in_room->sector_type, SECT_AIR) &&
					    !IS_WATER(ch->in_room)) {
						if (!IS_AFFECTED(ch, AFF_SNEAK))
							act("$p cracks and shaters into tiny pieces.", ch, obj, NULL, TO_ROOM);
						act("$p cracks and shaters into tiny pieces.", ch, obj, NULL, TO_CHAR);
						extract_obj(obj);
						continue;
					}

				oprog_call(OPROG_DROP, obj, ch, NULL);

				if (!may_float(obj) && cant_float(obj) && IS_WATER(ch->in_room)) {
					if (!IS_AFFECTED(ch, AFF_SNEAK))
						act("$p sinks down the water.", ch, obj, NULL, TO_ROOM);
					act("$p sinks down the water.", ch, obj, NULL, TO_CHAR);
					extract_obj(obj);
				} else if (IS_OBJ_STAT(obj, ITEM_MELT_DROP)) {
					if (!IS_AFFECTED(ch, AFF_SNEAK))
						act("$p dissolves into smoke.", ch, obj, NULL, TO_ROOM);
					act("$p dissolves into smoke.", ch, obj, NULL, TO_CHAR);
					extract_obj(obj);
				}
			}
		}

		if (!found) {
			if (arg[3] == '\0')
				act("You are not carrying anything.",
				    ch, NULL, arg, TO_CHAR);
			else
				act("You are not carrying any $T.",
				    ch, NULL, &arg[4], TO_CHAR);
		}
	}

	return;
}



void 
do_give(CHAR_DATA * ch, const char *argument)
{
	char            arg1[MAX_INPUT_LENGTH];
	char            arg2[MAX_INPUT_LENGTH];
	char            buf2[MAX_STRING_LENGTH];
	CHAR_DATA      *victim;
	OBJ_DATA       *obj;
	argument = one_argument(argument, arg1);
	strcpy(buf2, argument);
	argument = one_argument(argument, arg2);

	if (arg1[0] == '\0' || arg2[0] == '\0') {
		send_to_char("Give what to whom?\n\r", ch);
		return;
	}
	if (is_number(arg1)) {
		/* 'give NNNN coins victim' */
		int             amount;
		bool            silver;
		amount = atoi(arg1);
		if (amount <= 0
		    || (str_cmp(arg2, "coins") && str_cmp(arg2, "coin") &&
			str_cmp(arg2, "gold") && str_cmp(arg2, "silver"))) {
			send_to_char("Sorry, you can't do that.\n\r", ch);
			return;
		}
		silver = str_cmp(arg2, "gold");

		argument = one_argument(argument, arg2);
		if (arg2[0] == '\0') {
			send_to_char("Give what to whom?\n\r", ch);
			return;
		}
		if ((victim = get_char_room(ch, arg2)) == NULL) {
			send_to_char("They aren't here.\n\r", ch);
			return;
		}
		if ((!silver && ch->gold < amount) || (silver && ch->silver < amount)) {
			send_to_char("You haven't got that much.\n\r", ch);
			return;
		}
		if (silver) {
			ch->silver -= amount;
			victim->silver += amount;
		} else {
			ch->gold -= amount;
			victim->gold += amount;
		}

		act_printf(ch, NULL, victim, TO_VICT, POS_RESTING,
		  "$n gives you %d %s.", amount, silver ? "silver" : "gold");
		act("$n gives $N some coins.", ch, NULL, victim, TO_NOTVICT);
		act_printf(ch, NULL, victim, TO_CHAR, POS_RESTING,
		   "You give $N %d %s.", amount, silver ? "silver" : "gold");

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
				act(
				    "$n tells you 'I'm sorry, you did not give me enough to change.'"
				    ,victim, NULL, ch, TO_VICT);
				ch->reply = victim;
				doprintf(do_give, victim, "%d %s %s",
				amount, silver ? "silver" : "gold", ch->name);
			} else if (can_see(victim, ch)) {
				doprintf(do_give, victim, "%d %s %s",
				change, silver ? "gold" : "silver", ch->name);
				if (silver)
					doprintf(do_give, victim, "%d silver %s",
						 (95 * amount / 100 - change * 100), ch->name);
				act("$n tells you 'Thank you, come again.'",
				    victim, NULL, ch, TO_VICT);
				ch->reply = victim;
			}
		}
		return;
	}
	if ((obj = get_obj_carry(ch, arg1)) == NULL) {
		char_nputs(DONT_HAVE_ITEM, ch);
		return;
	}
	if (obj->wear_loc != WEAR_NONE) {
		send_to_char("You must remove it first.\n\r", ch);
		return;
	}
	if ((victim = get_char_room(ch, arg2)) == NULL) {
		send_to_char("They aren't here.\n\r", ch);
		return;
	}
	if (IS_NPC(victim) && victim->pIndexData->pShop != NULL
	&&  !HAS_TRIGGER(victim, TRIG_GIVE)) {
		do_tell_raw(victim, ch, "Sorry, you'll have to sell that.");
		return;
	}

	if (!can_drop_obj(ch, obj)) {
		send_to_char("You can't let go of it.\n\r", ch);
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
	if (!can_see_obj(victim, obj)) {
		act("$N can't see it.", ch, NULL, victim, TO_CHAR);
		return;
	}
	if (obj->pIndexData->limit != -1) {
		if ((IS_OBJ_STAT(obj, ITEM_ANTI_EVIL) && IS_EVIL(victim))
		    || (IS_OBJ_STAT(obj, ITEM_ANTI_GOOD) && IS_GOOD(victim))
		    || (IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(victim))) {
			send_to_char("Your victim's alignment doesn't match the objects align.", ch);
			return;
		}
	}
	obj_from_char(obj);
	obj_to_char(obj, victim);
	MOBtrigger = FALSE;
	act("$n gives $p to $N.", ch, obj, victim, TO_NOTVICT);
	act("$n gives you $p.", ch, obj, victim, TO_VICT);
	act("You give $p to $N.", ch, obj, victim, TO_CHAR);
	MOBtrigger = TRUE;

	/*
	 * Give trigger
	*/
	if (IS_NPC(victim) && HAS_TRIGGER(victim, TRIG_GIVE))
		mp_give_trigger(victim, ch, obj);

	oprog_call(OPROG_GIVE, obj, ch, victim);
}


/* for poisoning weapons and food/drink */
void 
do_envenom(CHAR_DATA * ch, const char *argument)
{
	OBJ_DATA       *obj;
	AFFECT_DATA     af;
	int             percent, skill;
	/* find out what */
	if (argument == '\0') {
		send_to_char("Envenom what item?\n\r", ch);
		return;
	}
	obj = get_obj_list(ch, argument, ch->carrying);

	if (obj == NULL) {
		send_to_char("You don't have that item.\n\r", ch);
		return;
	}
	if ((skill = get_skill(ch, gsn_envenom)) < 1) {
		send_to_char("Are you crazy? You'd poison yourself!\n\r", ch);
		return;
	}
	if (obj->item_type == ITEM_FOOD || obj->item_type == ITEM_DRINK_CON) {
		if (IS_OBJ_STAT(obj, ITEM_BLESS) || IS_OBJ_STAT(obj, ITEM_BURN_PROOF)) {
			act("You fail to poison $p.", ch, obj, NULL, TO_CHAR);
			return;
		}
		if (number_percent() < skill) {	/* success! */
			act("$n treats $p with deadly poison.", ch, obj, NULL, TO_ROOM);
			act("You treat $p with deadly poison.", ch, obj, NULL, TO_CHAR);
			if (!obj->value[3]) {
				obj->value[3] = 1;
				check_improve(ch, gsn_envenom, TRUE, 4);
			}
			WAIT_STATE(ch, skill_table[gsn_envenom].beats);
			return;
		}
		act("You fail to poison $p.", ch, obj, NULL, TO_CHAR);
		if (!obj->value[3])
			check_improve(ch, gsn_envenom, FALSE, 4);
		WAIT_STATE(ch, skill_table[gsn_envenom].beats);
		return;
	}
	if (obj->item_type == ITEM_WEAPON) {
		if (IS_WEAPON_STAT(obj, WEAPON_FLAMING)
		    || IS_WEAPON_STAT(obj, WEAPON_FROST)
		    || IS_WEAPON_STAT(obj, WEAPON_VAMPIRIC)
		    || IS_WEAPON_STAT(obj, WEAPON_SHARP)
		    || IS_WEAPON_STAT(obj, WEAPON_VORPAL)
		    || IS_WEAPON_STAT(obj, WEAPON_SHOCKING)
		    || IS_WEAPON_STAT(obj, WEAPON_HOLY)
		    || IS_OBJ_STAT(obj, ITEM_BLESS) || IS_OBJ_STAT(obj, ITEM_BURN_PROOF)) {
			act("You can't seem to envenom $p.", ch, obj, NULL, TO_CHAR);
			return;
		}
		if (obj->value[3] < 0
		    || attack_table[obj->value[3]].damage == DAM_BASH) {
			send_to_char("You can only envenom edged weapons.\n\r", ch);
			return;
		}
		if (IS_WEAPON_STAT(obj, WEAPON_POISON)) {
			act("$p is already envenomed.", ch, obj, NULL, TO_CHAR);
			return;
		}
		percent = number_percent();
		if (percent < skill) {

			af.where = TO_WEAPON;
			af.type = gsn_poison;
			af.level = ch->level * percent / 100;
			af.duration = ch->level * percent / 100;
			af.location = 0;
			af.modifier = 0;
			af.bitvector = WEAPON_POISON;
			affect_to_obj(obj, &af);

			if (!IS_AFFECTED(ch, AFF_SNEAK))
				act("$n coats $p with deadly venom.", ch, obj, NULL, TO_ROOM);
			act("You coat $p with venom.", ch, obj, NULL, TO_CHAR);
			check_improve(ch, gsn_envenom, TRUE, 3);
			WAIT_STATE(ch, skill_table[gsn_envenom].beats);
			return;
		} else {
			act("You fail to envenom $p.", ch, obj, NULL, TO_CHAR);
			check_improve(ch, gsn_envenom, FALSE, 3);
			WAIT_STATE(ch, skill_table[gsn_envenom].beats);
			return;
		}
	}
	act("You can't poison $p.", ch, obj, NULL, TO_CHAR);
	return;
}

void 
do_fill(CHAR_DATA * ch, const char *argument)
{
	char            arg[MAX_INPUT_LENGTH];
	OBJ_DATA       *obj;
	OBJ_DATA       *fountain;
	bool            found;
	one_argument(argument, arg);

	if (arg[0] == '\0') {
		send_to_char("Fill what?\n\r", ch);
		return;
	}
	if ((obj = get_obj_carry(ch, arg)) == NULL) {
		send_to_char("You do not have that item.\n\r", ch);
		return;
	}
	found = FALSE;
	for (fountain = ch->in_room->contents; fountain != NULL;
	     fountain = fountain->next_content) {
		if (fountain->item_type == ITEM_FOUNTAIN) {
			found = TRUE;
			break;
		}
	}

	if (!found) {
		send_to_char("There is no fountain here!\n\r", ch);
		return;
	}
	if (obj->item_type != ITEM_DRINK_CON) {
		send_to_char("You can't fill that.\n\r", ch);
		return;
	}
	if (obj->value[1] != 0 && obj->value[2] != fountain->value[2]) {
		send_to_char("There is already another liquid in it.\n\r", ch);
		return;
	}
	if (obj->value[1] >= obj->value[0]) {
		send_to_char("Your container is full.\n\r", ch);
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

void 
do_pour(CHAR_DATA * ch, const char *argument)
{
	char            arg[MAX_STRING_LENGTH];
	OBJ_DATA       *out, *in;
	CHAR_DATA      *vch = NULL;
	int             amount;
	argument = one_argument(argument, arg);

	if (arg[0] == '\0' || argument[0] == '\0') {
		send_to_char("Pour what into what?\n\r", ch);
		return;
	}
	if ((out = get_obj_carry(ch, arg)) == NULL) {
		send_to_char("You don't have that item.\n\r", ch);
		return;
	}
	if (out->item_type != ITEM_DRINK_CON) {
		send_to_char("That's not a drink container.\n\r", ch);
		return;
	}
	if (!str_cmp(argument, "out")) {
		if (out->value[1] == 0) {
			char_nputs(IT_IS_EMPTY, ch);
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
			send_to_char("Pour into what?\n\r", ch);
			return;
		}
		in = get_eq_char(vch, WEAR_HOLD);

		if (in == NULL) {
			send_to_char("They aren't holding anything.", ch);
			return;
		}
	}
	if (in->item_type != ITEM_DRINK_CON) {
		send_to_char("You can only pour into other drink containers.\n\r", ch);
		return;
	}
	if (in == out) {
		send_to_char("You cannot change the laws of physics!\n\r", ch);
		return;
	}
	if (in->value[1] != 0 && in->value[2] != out->value[2]) {
		send_to_char("They don't hold the same liquid.\n\r", ch);
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

void 
do_drink(CHAR_DATA * ch, const char *argument)
{
	char            arg[MAX_INPUT_LENGTH];
	OBJ_DATA       *obj;
	int             amount;
	int             liquid;
	one_argument(argument, arg);

	if (arg[0] == '\0') {
		for (obj = ch->in_room->contents; obj; obj= obj->next_content) {
			if (obj->item_type == ITEM_FOUNTAIN)
				break;
		}

		if (obj == NULL) {
			char_nputs(DRINK_WHAT, ch);
			return;
		}
	} else {
		if ((obj = get_obj_here(ch, arg)) == NULL) {
			char_nputs(CANT_FIND_IT, ch);
			return;
		}
	}

	if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10) {
		char_nputs(FAIL_TO_REACH_MOUTH, ch);
		return;
	}
	switch (obj->item_type) {
	default:
		char_nputs(CANT_DRINK_FROM_THAT, ch);
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
			char_nputs(IT_IS_EMPTY, ch);
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
		char_nputs(TOO_FULL_TO_DRINK, ch);
		return;
	}
	act_nprintf(ch, obj, liq_table[liquid].liq_name, TO_ROOM, POS_RESTING,
		    N_DRINKS_T_FROM_P);
	act_nprintf(ch, obj, liq_table[liquid].liq_name, TO_CHAR, POS_DEAD,
		    YOU_DRINK_T_FROM_P);

	if (ch->fighting != NULL)
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
		char_nputs(YOU_FEEL_DRUNK, ch);
	if (!IS_NPC(ch) && ch->pcdata->condition[COND_FULL] > 60)
		char_nputs(YOU_ARE_FULL, ch);
	if (!IS_NPC(ch) && ch->pcdata->condition[COND_THIRST] > 60)
		char_nputs(YOUR_THIRST_QUENCHED, ch);

	if (obj->value[3] != 0) {
		/* The drink was poisoned ! */
		AFFECT_DATA     af;
		act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING,
			    N_CHOKES_GAGS);
		char_nputs(YOU_CHOKE_GAG, ch);
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



void 
do_eat(CHAR_DATA * ch, const char *argument)
{
	char            arg[MAX_INPUT_LENGTH];
	OBJ_DATA       *obj;


	one_argument(argument, arg);
	if (arg[0] == '\0') {
		char_nputs(EAT_WHAT, ch);
		return;
	}
	if ((obj = get_obj_carry(ch, arg)) == NULL) {
		char_nputs(DONT_HAVE_ITEM, ch);
		return;
	}
	if (!IS_IMMORTAL(ch)) {
		if (obj->item_type != ITEM_FOOD
		&& obj->item_type != ITEM_PILL) {
			char_nputs(NOT_EDIBLE, ch);
			return;
		}
		if (!IS_NPC(ch) && ch->pcdata->condition[COND_FULL] > 80) {
			char_nputs(TOO_FULL_TO_EAT, ch);
			return;
		}
	}
	act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING, N_EATS_P);
	act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD, YOU_EAT_P);
	if (ch->fighting != NULL)
		WAIT_STATE(ch, 3 * PULSE_VIOLENCE);

	switch (obj->item_type) {

	case ITEM_FOOD:
		if (!IS_NPC(ch)) {
			int             condition;
			condition = ch->pcdata->condition[COND_HUNGER];
			gain_condition(ch, COND_FULL, obj->value[0] * 2);
			gain_condition(ch, COND_HUNGER, obj->value[1] * 2);
			if (!condition
			&& ch->pcdata->condition[COND_HUNGER] > 0)
				char_nputs(NO_LONGER_HUNGRY, ch);
			else if (ch->pcdata->condition[COND_FULL] > 60)
				char_nputs(YOU_ARE_FULL, ch);
		}
		if (obj->value[3] != 0) {
			/* The food was poisoned! */
			AFFECT_DATA     af;
			act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING,
				    N_CHOKES_GAGS);
			char_nputs(YOU_CHOKE_GAG, ch);

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
bool 
remove_obj(CHAR_DATA * ch, int iWear, bool fReplace)
{
	OBJ_DATA       *obj;
	if ((obj = get_eq_char(ch, iWear)) == NULL)
		return TRUE;

	if (!fReplace)
		return FALSE;

	if (IS_SET(obj->extra_flags, ITEM_NOREMOVE)) {
		act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD, CANT_REMOVE_IT);
		return FALSE;
	}
	if ((obj->item_type == ITEM_TATTOO) && (!IS_IMMORTAL(ch))) {
		act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD,
			    YOU_MUST_SCRATCH_TO_REMOVE);
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
		act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD,
			    YOU_REMOVE_P_IN_PAIN);
		act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING,
			    N_REMOVES_P_IN_PAIN);
		WAIT_STATE(ch, 4);
		return TRUE;
	}
	unequip_char(ch, obj);
	act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING, N_STOPS_USING_P);
	act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD, YOU_STOP_USING_P);

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
void 
wear_obj(CHAR_DATA * ch, OBJ_DATA * obj, bool fReplace)
{
	int             wear_level = ch->level;
	if ((class_table[ch->class].fMana && obj->item_type == ITEM_ARMOR)
	 || (!class_table[ch->class].fMana && obj->item_type == ITEM_WEAPON))
		wear_level += 3;

	if (wear_level < obj->level) {
		char_printf(ch, msg(MUST_BE_LEVEL_TO_USE, ch), obj->level);
		act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING,
			    N_TRIES_TO_USE);
		return;
	}
	if (obj->item_type == ITEM_LIGHT) {
		if (!remove_obj(ch, WEAR_LIGHT, fReplace))
			return;
		act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING,
			    N_LIGHTS_P_HOLD);
		act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD,
			    YOU_LIGHT_P_HOLD);
		equip_char(ch, obj, WEAR_LIGHT);
		return;
	}
	if (CAN_WEAR(obj, ITEM_WEAR_FINGER)) {
		if (get_eq_char(ch, WEAR_FINGER_L) != NULL
		    && get_eq_char(ch, WEAR_FINGER_R) != NULL
		    && !remove_obj(ch, WEAR_FINGER_L, fReplace)
		    && !remove_obj(ch, WEAR_FINGER_R, fReplace))
			return;

		if (get_eq_char(ch, WEAR_FINGER_L) == NULL) {
			act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING,
				    N_WEARS_P_LEFT_FINGER);
			act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD,
				    YOU_WEAR_P_LEFT_FINGER);
			equip_char(ch, obj, WEAR_FINGER_L);
			return;
		}
		if (get_eq_char(ch, WEAR_FINGER_R) == NULL) {
			act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING,
				    N_WEARS_P_RIGHT_FINGER);
			act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD,
				    YOU_WEAR_P_RIGHT_FINGER);
			equip_char(ch, obj, WEAR_FINGER_R);
			return;
		}
		bug("Wear_obj: no free finger.", 0);
		char_nputs(ALREADY_WEAR_TWO_RINGS, ch);
		return;
	}
	if (CAN_WEAR(obj, ITEM_WEAR_NECK)) {
		if (get_eq_char(ch, WEAR_NECK_1) != NULL
		    && get_eq_char(ch, WEAR_NECK_2) != NULL
		    && !remove_obj(ch, WEAR_NECK_1, fReplace)
		    && !remove_obj(ch, WEAR_NECK_2, fReplace))
			return;

		if (get_eq_char(ch, WEAR_NECK_1) == NULL) {
			act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING,
				    N_WEARS_P_NECK);
			act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD,
				    YOU_WEAR_P_NECK);
			equip_char(ch, obj, WEAR_NECK_1);
			return;
		}
		if (get_eq_char(ch, WEAR_NECK_2) == NULL) {
			act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING,
				    N_WEARS_P_NECK);
			act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD,
				    YOU_WEAR_P_NECK);
			equip_char(ch, obj, WEAR_NECK_2);
			return;
		}
		bug("Wear_obj: no free neck.", 0);
		char_nputs(ALREADY_WEAR_TWO_NECK, ch);
		return;
	}
	if (CAN_WEAR(obj, ITEM_WEAR_BODY)) {
		if (!remove_obj(ch, WEAR_BODY, fReplace))
			return;
		act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING,
			    N_WEARS_P_TORSO);
		act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD, YOU_WEAR_P_TORSO);
		equip_char(ch, obj, WEAR_BODY);
		return;
	}
	if (CAN_WEAR(obj, ITEM_WEAR_HEAD)) {
		if (!remove_obj(ch, WEAR_HEAD, fReplace))
			return;
		act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING,
			    N_WEARS_P_HEAD);
		act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD, YOU_WEAR_P_HEAD);
		equip_char(ch, obj, WEAR_HEAD);
		return;
	}
	if (CAN_WEAR(obj, ITEM_WEAR_LEGS)) {
		if (!remove_obj(ch, WEAR_LEGS, fReplace))
			return;
		act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING,
			    N_WEARS_P_LEGS);
		act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD, YOU_WEAR_P_LEGS);
		equip_char(ch, obj, WEAR_LEGS);
		return;
	}
	if (CAN_WEAR(obj, ITEM_WEAR_FEET)) {
		if (!remove_obj(ch, WEAR_FEET, fReplace))
			return;
		act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING,
			    N_WEARS_P_FEET);
		act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD, YOU_WEAR_P_FEET);
		equip_char(ch, obj, WEAR_FEET);
		return;
	}
	if (CAN_WEAR(obj, ITEM_WEAR_HANDS)) {
		if (!remove_obj(ch, WEAR_HANDS, fReplace))
			return;
		act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING,
			    N_WEARS_P_HANDS);
		act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD, YOU_WEAR_P_HANDS);
		equip_char(ch, obj, WEAR_HANDS);
		return;
	}
	if (CAN_WEAR(obj, ITEM_WEAR_ARMS)) {
		if (!remove_obj(ch, WEAR_ARMS, fReplace))
			return;
		act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING,
			    N_WEARS_P_ARMS);
		act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD, YOU_WEAR_P_ARMS);
		equip_char(ch, obj, WEAR_ARMS);
		return;
	}
	if (CAN_WEAR(obj, ITEM_WEAR_ABOUT)) {
		if (!remove_obj(ch, WEAR_ABOUT, fReplace))
			return;
		act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING,
			    N_WEARS_P_TORSO);
		act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD, YOU_WEAR_P_TORSO);
		equip_char(ch, obj, WEAR_ABOUT);
		return;
	}
	if (CAN_WEAR(obj, ITEM_WEAR_WAIST)) {
		if (!remove_obj(ch, WEAR_WAIST, fReplace))
			return;
		act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING,
			    N_WEARS_P_WAIST);
		act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD, YOU_WEAR_P_WAIST);
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
			act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING,
				    N_WEARS_P_RIGHT_WRIST);
			act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD,
				    YOU_WEAR_P_LEFT_WRIST);
			equip_char(ch, obj, WEAR_WRIST_L);
			return;
		}
		if (get_eq_char(ch, WEAR_WRIST_R) == NULL) {
			act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING,
				    N_WEARS_P_RIGHT_WRIST);
			act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD,
				    YOU_WEAR_P_RIGHT_WRIST);
			equip_char(ch, obj, WEAR_WRIST_R);
			return;
		}
		bug("Wear_obj: no free wrist.", 0);
		char_nputs(ALREADY_WEAR_TWO_WRIST, ch);
		return;
	}
	if (CAN_WEAR(obj, ITEM_WEAR_SHIELD)) {
		OBJ_DATA       *weapon;
		if (get_eq_char(ch, WEAR_SECOND_WIELD) != NULL) {
			char_nputs(CANT_USE_SHIELD_SECOND_WEAPON, ch);
			return;
		}
		if (!remove_obj(ch, WEAR_SHIELD, fReplace))
			return;

		weapon = get_eq_char(ch, WEAR_WIELD);
		if (weapon != NULL && ch->size < SIZE_LARGE
		    && IS_WEAPON_STAT(weapon, WEAPON_TWO_HANDS)) {
			char_nputs(YOUR_HANDS_TIRED_WEAPON, ch);
			return;
		}
		act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING,
			    N_WEARS_P_SHIELD);
		act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD,
			    YOU_WEAR_P_SHIELD);
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

		if (!IS_NPC(ch) && get_obj_weight(obj)
		    > (str_app[get_curr_stat(ch, STAT_STR)].wield * 10)) {
			char_nputs(TOO_HEAVY_WIELD, ch);
			if (dual)
				equip_char(ch, dual, WEAR_SECOND_WIELD);
			return;
		}
		if (IS_WEAPON_STAT(obj, WEAPON_TWO_HANDS)
		    && ((!IS_NPC(ch) && ch->size < SIZE_LARGE
			 && get_eq_char(ch, WEAR_SHIELD) != NULL)
			|| get_eq_char(ch, WEAR_SECOND_WIELD) != NULL)) {
			char_nputs(NEED_TWO_HANDS, ch);
			if (dual)
				equip_char(ch, dual, WEAR_SECOND_WIELD);
			return;
		}
		act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING, N_WIELDS_P);
		act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD, YOU_WIELD_P);
		equip_char(ch, obj, WEAR_WIELD);
		if (dual)
			equip_char(ch, dual, WEAR_SECOND_WIELD);

		sn = get_weapon_sn(ch, WEAR_WIELD);

		if (sn == gsn_hand_to_hand)
			return;

		skill = get_weapon_skill(ch, sn);

		if (skill >= 100)
			act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD,
				    P_LIKE_PART_OF_YOU);
		else if (skill > 85)
			act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD,
				    QUITE_CONFIDENT_P);
		else if (skill > 70)
			act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD,
				    SKILLED_WITH_P);
		else if (skill > 50)
			act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD,
				    SKILL_P_ADEQUATE);
		else if (skill > 25)
			act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD,
				    P_FEELS_CLUMSY);
		else if (skill > 1)
			act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD,
				    ALMOST_DROP_P);
		else
			act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD,
				    DONT_KNOW_THE_END);

		return;
	}
	if (CAN_WEAR(obj, ITEM_HOLD)) {
		if (get_eq_char(ch, WEAR_SECOND_WIELD) != NULL) {
			char_nputs(CANT_HOLD_WHILE_2_WEAPONS, ch);
			return;
		}
		if (!remove_obj(ch, WEAR_HOLD, fReplace))
			return;
		act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING,
			    N_HOLDS_P_HAND);
		act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD, YOU_HOLD_P_HAND);
		equip_char(ch, obj, WEAR_HOLD);
		return;
	}
	if (CAN_WEAR(obj, ITEM_WEAR_FLOAT)) {
		if (!remove_obj(ch, WEAR_FLOAT, fReplace))
			return;
		act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING, N_FLOAT_P);
		act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD, YOU_FLOAT_P);
		equip_char(ch, obj, WEAR_FLOAT);
		return;
	}
	if (CAN_WEAR(obj, ITEM_WEAR_TATTOO) && IS_IMMORTAL(ch)) {
		if (!remove_obj(ch, WEAR_TATTOO, fReplace))
			return;
		act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING, N_USES_TATTOO);
		act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD, YOU_USE_TATTOO);
		equip_char(ch, obj, WEAR_TATTOO);
		return;
	}
	if (fReplace)
		char_nputs(CANT_WEAR_IT, ch);

	return;
}



void 
do_wear(CHAR_DATA * ch, const char *argument)
{
	char            arg[MAX_INPUT_LENGTH];
	OBJ_DATA       *obj;
	OBJ_DATA       *obj_next;
	one_argument(argument, arg);

	if (arg[0] == '\0') {
		char_nputs(WEAR_WHAT, ch);
		return;
	}
	if (!str_cmp(arg, "all"))
		for (obj = ch->carrying; obj != NULL; obj = obj_next) {
			obj_next = obj->next_content;
			if (obj->wear_loc == WEAR_NONE && can_see_obj(ch, obj))
				wear_obj(ch, obj, FALSE);
		}
	else if ((obj = get_obj_carry(ch, arg)) == NULL) {
		char_nputs(DONT_HAVE_ITEM, ch);
		return;
	} else
		wear_obj(ch, obj, TRUE);

	return;
}



void 
do_remove(CHAR_DATA * ch, const char *argument)
{
	char            arg[MAX_INPUT_LENGTH];
	OBJ_DATA       *obj;
	one_argument(argument, arg);

	if (arg[0] == '\0') {
		char_nputs(REMOVE_WHAT, ch);
		return;
	}
	if (!str_cmp(arg, "all")) {
		OBJ_DATA       *obj_next;
		for (obj = ch->carrying; obj != NULL; obj = obj_next) {
			obj_next = obj->next_content;
			if (obj->wear_loc != WEAR_NONE && can_see_obj(ch, obj))
				remove_obj(ch, obj->wear_loc, TRUE);
		}
		return;
	}
	if ((obj = get_obj_wear(ch, arg)) == NULL) {
		char_nputs(DONT_HAVE_ITEM, ch);
		return;
	}
	remove_obj(ch, obj->wear_loc, TRUE);
	return;
}

void 
do_sacrifice(CHAR_DATA * ch, const char *argument)
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

void 
do_sacr(CHAR_DATA * ch, const char *argument)
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
		act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING, N_SACS_SELF);
		char_nputs(YOU_SAC_SELF, ch);
		return;
	}
	obj = get_obj_list(ch, arg, ch->in_room->contents);
	if (obj == NULL) {
		char_nputs(CANT_FIND_IT, ch);
		return;
	}
	if ((obj->item_type == ITEM_CORPSE_PC && ch->level < MAX_LEVEL)
	    || (QUEST_OBJ_FIRST <= obj->pIndexData->vnum
		&& obj->pIndexData->vnum <= QUEST_OBJ_LAST)) {
		char_nputs(GODS_WOUDLNT_LIKE_THAT, ch);
		return;
	}
	if (!CAN_WEAR(obj, ITEM_TAKE) || CAN_WEAR(obj, ITEM_NO_SAC)) {
		act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD,
			    P_NOT_ACCEPTABLE_SAC);
		return;
	}
	silver = UMAX(1, number_fuzzy(obj->level));

	if (obj->item_type != ITEM_CORPSE_NPC
	    && obj->item_type != ITEM_CORPSE_PC)
		silver = UMIN(silver, obj->cost);

	if (silver == 1)
		char_nputs(SAC_GET_ONE_SILVER, ch);
	else
		char_nprintf(ch, SAC_GET_D_SILVER, silver);

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
	act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING, N_SACS_P);

	if (oprog_call(OPROG_SAC, obj, ch, NULL))
		return;

	wiznet("$N sends up $p as a burnt offering.",
	       ch, obj, WIZ_SACCING, 0, 0);
	fScatter = TRUE;
	if (obj->item_type == ITEM_CORPSE_NPC
	    || obj->item_type == ITEM_CORPSE_PC) {
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
			act_nprintf(ch, two_objs[0], NULL, TO_CHAR, POS_DEAD,
				    YOUR_SAC_REVEALS_P);
			act_nprintf(ch, two_objs[0], NULL, TO_ROOM, POS_RESTING,
				    NS_SAC_REVEALS_P);
		}
		if (iScatter == 2) {
			act_nprintf(ch, two_objs[0], two_objs[1], TO_CHAR,
				    POS_DEAD, YOUR_SAC_REVEALS_P_P);
			act_nprintf(ch, two_objs[0], two_objs[1], TO_ROOM,
				    POS_RESTING, NS_SAC_REVEALS_P);
		}
		snprintf(buf, sizeof(buf), msg(AS_YOU_SAC, ch));
		snprintf(buf2, sizeof(buf2), msg(AS_N_SACS, ch));
		if (iScatter < 3)
			fScatter = FALSE;
		else if (iScatter < 5) {
			strcat(buf, msg(FEW_THINGS, ch));
			strcat(buf2, msg(FEW_THINGS, ch));
		} else if (iScatter < 9) {
			strcat(buf, msg(BUNCH_OF_OBJECTS, ch));
			strcat(buf2, msg(BUNCH_OF_OBJECTS, ch));
		} else if (iScatter < 15) {
			strcat(buf, msg(MANY_THINGS, ch));
			strcat(buf2, msg(MANY_THINGS, ch));
		} else {
			strcat(buf, msg(LOT_OF_OBJECTS, ch));
			strcat(buf2, msg(LOT_OF_OBJECTS, ch));
		}
		strcat(buf, msg(ON_IT, ch));
		strcat(buf2, msg(ON_IT, ch));

		switch (ch->in_room->sector_type) {
		case SECT_FIELD:
			strcat(buf, msg(SCATTER_ON_DIRT, ch));
			strcat(buf2, msg(SCATTER_ON_DIRT, ch));
			break;
		case SECT_FOREST:
			strcat(buf, msg(SCATTER_ON_DIRT, ch));
			strcat(buf2, msg(SCATTER_ON_DIRT, ch));
			break;
		case SECT_WATER_SWIM:
			strcat(buf, msg(SCATTER_OVER_WATER, ch));
			strcat(buf2, msg(SCATTER_OVER_WATER, ch));
			break;
		case SECT_WATER_NOSWIM:
			strcat(buf, msg(SCATTER_OVER_WATER, ch));
			strcat(buf2, msg(SCATTER_OVER_WATER, ch));
			break;
		default:
			strcat(buf, msg(SCATTER_AROUND, ch));
			strcat(buf2, msg(SCATTER_AROUND, ch));
			break;
		}
		if (fScatter) {
			act(buf, ch, NULL, NULL, TO_CHAR);
			act(buf2, ch, NULL, NULL, TO_ROOM);
		}
	}
	extract_obj(obj);
	return;
}



void 
do_quaff(CHAR_DATA * ch, const char *argument)
{
	char            arg[MAX_INPUT_LENGTH];
	OBJ_DATA       *obj;
	one_argument(argument, arg);

	if (ch->clan == CLAN_BATTLE && !IS_IMMORTAL(ch)) {
		send_to_char("You are a BattleRager, not a filthy magician!\n\r", ch);
		return;
	}
	if (arg[0] == '\0') {
		char_nputs(QUAFF_WHAT, ch);
		return;
	}
	if ((obj = get_obj_carry(ch, arg)) == NULL) {
		char_nputs(DONT_HAVE_POTION, ch);
		return;
	}
	if (obj->item_type != ITEM_POTION) {
		char_nputs(CAN_QUAFF_ONLY_POTIONS, ch);
		return;
	}
	if (ch->level < obj->level) {
		char_nputs(TOO_POWERFUL_LIQUID, ch);
		return;
	}
	act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING, N_QUAFFS_P);
	act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD, YOU_QUAFF_P);

	obj_cast_spell(obj->value[1], obj->value[0], ch, ch, NULL);
	obj_cast_spell(obj->value[2], obj->value[0], ch, ch, NULL);
	obj_cast_spell(obj->value[3], obj->value[0], ch, ch, NULL);
	obj_cast_spell(obj->value[4], obj->value[0], ch, ch, NULL);

	if (IS_PUMPED(ch) || ch->fighting != NULL)
		WAIT_STATE(ch, 2 * PULSE_VIOLENCE);

	extract_obj(obj);
	obj_to_char(create_object(get_obj_index(OBJ_VNUM_POTION_VIAL), 0), ch);
}



void 
do_recite(CHAR_DATA * ch, const char *argument)
{
	char            arg1[MAX_INPUT_LENGTH];
	char            arg2[MAX_INPUT_LENGTH];
	CHAR_DATA      *victim;
	OBJ_DATA       *scroll;
	OBJ_DATA       *obj;
	if (ch->clan == CLAN_BATTLE) {
		send_to_char(
			     "RECITE?!  You are a battle rager, not a filthy magician!\n\r", ch);
		return;
	}
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if ((scroll = get_obj_carry(ch, arg1)) == NULL) {
		char_nputs(DONT_HAVE_SCROLL, ch);
		return;
	}
	if (scroll->item_type != ITEM_SCROLL) {
		char_nputs(CAN_RECITE_ONLY_SCROLLS, ch);
		return;
	}
	if (ch->level < scroll->level) {
		char_nputs(SCROLL_TOO_COMPLEX, ch);
		return;
	}
	obj = NULL;
	if (arg2[0] == '\0')
		victim = ch;
	else if ((victim = get_char_room(ch, arg2)) == NULL
		 && (obj = get_obj_here(ch, arg2)) == NULL) {
		char_nputs(CANT_FIND_IT, ch);
		return;
	}
	act_nprintf(ch, scroll, NULL, TO_ROOM, POS_RESTING, N_RECITES_P);
	act_nprintf(ch, scroll, NULL, TO_CHAR, POS_DEAD, YOU_RECITE_P);

	if (number_percent() >= get_skill(ch, gsn_scrolls) * 4 / 5) {
		char_nputs(MISPRONOUNCE_SYLLABLE, ch);
		check_improve(ch, gsn_scrolls, FALSE, 2);
	} else {
		obj_cast_spell(scroll->value[1], scroll->value[0],
			       ch, victim, obj);
		obj_cast_spell(scroll->value[2], scroll->value[0],
			       ch, victim, obj);
		obj_cast_spell(scroll->value[3], scroll->value[0],
			       ch, victim, obj);
		obj_cast_spell(scroll->value[4], scroll->value[0],
			       ch, victim, obj);
		check_improve(ch, gsn_scrolls, TRUE, 2);

		if (IS_PUMPED(ch) || ch->fighting != NULL)
			WAIT_STATE(ch, 2 * PULSE_VIOLENCE);
	}

	extract_obj(scroll);
}



void 
do_brandish(CHAR_DATA * ch, const char *argument)
{
	CHAR_DATA      *vch;
	CHAR_DATA      *vch_next;
	OBJ_DATA       *staff;
	int             sn;
	if (IS_SET(ch->act, PLR_GHOST)) {
		send_to_char("You can't do it while being a ghost.\n\r", ch);
	}
	if (ch->clan == CLAN_BATTLE) {
		send_to_char("You are not a filthy magician!\n\r", ch);
		return;
	}
	if ((staff = get_eq_char(ch, WEAR_HOLD)) == NULL) {
		send_to_char("You hold nothing in your hand.\n\r", ch);
		return;
	}
	if (staff->item_type != ITEM_STAFF) {
		send_to_char("You can brandish only with a staff.\n\r", ch);
		return;
	}
	if ((sn = staff->value[3]) < 0
	    || sn >= MAX_SKILL
	    || skill_table[sn].spell_fun == 0) {
		bug("Do_brandish: bad sn %d.", sn);
		return;
	}
	WAIT_STATE(ch, 2 * PULSE_VIOLENCE);

	if (staff->value[2] > 0) {
		act("$n brandishes $p.", ch, staff, NULL, TO_ROOM);
		act("You brandish $p.", ch, staff, NULL, TO_CHAR);
		if (ch->level + 3 < staff->level
		    || number_percent() >= 10 + get_skill(ch, gsn_staves) * 4 / 5) {
			act("You fail to invoke $p.", ch, staff, NULL, TO_CHAR);
			act("...and nothing happens.", ch, NULL, NULL, TO_ROOM);
			check_improve(ch, gsn_staves, FALSE, 2);
		} else
			for (vch = ch->in_room->people; vch; vch = vch_next) {
				vch_next = vch->next_in_room;

				switch (skill_table[sn].target) {
				default:
					bug("Do_brandish: bad target for sn %d.", sn);
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

				obj_cast_spell(staff->value[3], staff->value[0], ch, vch, NULL);
				check_improve(ch, gsn_staves, TRUE, 2);
			}
	}
	if (--staff->value[2] <= 0) {
		act("$n's $p blazes bright and is gone.", ch, staff, NULL, TO_ROOM);
		act("Your $p blazes bright and is gone.", ch, staff, NULL, TO_CHAR);
		extract_obj(staff);
	}
	return;
}



void 
do_zap(CHAR_DATA * ch, const char *argument)
{
	char            arg[MAX_INPUT_LENGTH];
	CHAR_DATA      *victim;
	OBJ_DATA       *wand;
	OBJ_DATA       *obj;
	if (ch->clan == CLAN_BATTLE) {
		send_to_char("You'd destroy the magic, not use it!\n\r", ch);
		return;
	}
	one_argument(argument, arg);
	if (arg[0] == '\0' && ch->fighting == NULL) {
		send_to_char("Zap whom or what?\n\r", ch);
		return;
	}
	if ((wand = get_eq_char(ch, WEAR_HOLD)) == NULL) {
		send_to_char("You hold nothing in your hand.\n\r", ch);
		return;
	}
	if (wand->item_type != ITEM_WAND) {
		send_to_char("You can zap only with a wand.\n\r", ch);
		return;
	}
	obj = NULL;
	if (arg[0] == '\0') {
		if (ch->fighting != NULL) {
			victim = ch->fighting;
		} else {
			send_to_char("Zap whom or what?\n\r", ch);
			return;
		}
	} else {
		if ((victim = get_char_room(ch, arg)) == NULL
		    && (obj = get_obj_here(ch, arg)) == NULL) {
			send_to_char("You can't find it.\n\r", ch);
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
		    || number_percent() >= 20 + get_skill(ch, gsn_wands) * 4 / 5) {
			act("Your efforts with $p produce only smoke and sparks.",
			    ch, wand, NULL, TO_CHAR);
			act("$n's efforts with $p produce only smoke and sparks.",
			    ch, wand, NULL, TO_ROOM);
			check_improve(ch, gsn_wands, FALSE, 2);
		} else {
			obj_cast_spell(wand->value[3], wand->value[0], ch, victim, obj);
			check_improve(ch, gsn_wands, TRUE, 2);
		}
	}
	if (--wand->value[2] <= 0) {
		act("$n's $p explodes into fragments.", ch, wand, NULL, TO_ROOM);
		act("Your $p explodes into fragments.", ch, wand, NULL, TO_CHAR);
		extract_obj(wand);
	}
	return;
}



void 
do_steal(CHAR_DATA * ch, const char *argument)
{
	char            arg1[MAX_INPUT_LENGTH];
	char            arg2[MAX_INPUT_LENGTH];
	CHAR_DATA      *victim;
	OBJ_DATA       *obj;
	OBJ_DATA       *obj_inve;
	int             percent;
	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (arg1[0] == '\0' || arg2[0] == '\0') {
		send_to_char("Steal what from whom?\n\r", ch);
		return;
	}
	if (IS_NPC(ch) && IS_SET(ch->affected_by, AFF_CHARM)
	    && (ch->master != NULL)) {
		send_to_char("You are to dazed to steal anything.\n\r", ch);
		return;
	}
	if ((victim = get_char_room(ch, arg2)) == NULL) {
		send_to_char("They aren't here.\n\r", ch);
		return;
	}
	if (victim == ch) {
		send_to_char("That's pointless.\n\r", ch);
		return;
	}

	if (victim->position == POS_FIGHTING) {
		send_to_char("You'd better not -- you might get hit.\n\r", ch);
		return;
	}

	if (is_safe(ch, victim))
		return;
	
	WAIT_STATE(ch, skill_table[gsn_steal].beats);
	percent = number_percent() + (IS_AWAKE(victim) ? 10 : -50);
	percent += can_see(victim, ch) ? -10 : 0;
	percent += (victim->level - ch->level)/2;

	if (victim->position == POS_FIGHTING
	||  (!IS_NPC(ch) && percent > get_skill(ch, gsn_steal))) {
		/*
		 * Failure.
		 */

		send_to_char("Oops.\n\r", ch);

		if (IS_AFFECTED(ch, AFF_HIDE | AFF_FADE) && !IS_NPC(ch)) {
			REMOVE_BIT(ch->affected_by, AFF_HIDE | AFF_FADE);
			char_nputs(YOU_STEP_OUT_SHADOWS, ch);
			act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING, 
				    N_STEPS_OUT_OF_SHADOWS);
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
				doprintf(do_yell, victim, "%s tried to rob me!", ch->name);
				break;
			case 3:
				doprintf(do_yell, victim, "Keep your hands out of there, %s!",
					 ch->name);
				break;
			}

		if (!IS_NPC(ch) && IS_NPC(victim)) {
			check_improve(ch, gsn_steal, FALSE, 2);
			multi_hit(victim, ch, TYPE_UNDEFINED, MSTRIKE);
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
			send_to_char("You couldn't get any coins.\n\r", ch);
			return;
		}
		ch->gold += amount_g;
		victim->gold -= amount_g;
		ch->silver += amount_s;
		victim->silver -= amount_s;
		char_printf(ch, "Bingo!  You got %d %s coins.\n\r",
			    amount_s != 0 ? amount_s : amount_g,
			    amount_s != 0 ? "silver" : "gold");
		check_improve(ch, gsn_steal, TRUE, 2);
		return;
	}
	if ((obj = get_obj_carry(victim, arg1)) == NULL) {
		send_to_char("You can't find it.\n\r", ch);
		return;
	}
	if (!can_drop_obj(ch, obj)
	/* ||   IS_SET(obj->extra_flags, ITEM_INVENTORY) */
	     /* ||  obj->level > ch->level */ ) {
		send_to_char("You can't pry it away.\n\r", ch);
		return;
	}
	if (ch->carry_number + get_obj_number(obj) > can_carry_n(ch)) {
		send_to_char("You have your hands full.\n\r", ch);
		return;
	}
	if (ch->carry_weight + get_obj_weight(obj) > can_carry_w(ch)) {
		send_to_char("You can't carry that much weight.\n\r", ch);
		return;
	}
	if (!IS_SET(obj->extra_flags, ITEM_INVENTORY)) {
		obj_from_char(obj);
		obj_to_char(obj, ch);
		send_to_char("You got it!\n\r", ch);
		check_improve(ch, gsn_steal, TRUE, 2);
	} else {
		obj_inve = NULL;
		obj_inve = create_object(obj->pIndexData, 0);
		clone_object(obj, obj_inve);
		REMOVE_BIT(obj_inve->extra_flags, ITEM_INVENTORY);
		obj_to_char(obj_inve, ch);
		send_to_char("You got one of them!\n\r", ch);
		check_improve(ch, gsn_steal, TRUE, 1);
	}
}


/*
 * Shopping commands.
 */
CHAR_DATA      *
find_keeper(CHAR_DATA * ch)
{
	CHAR_DATA      *keeper;
	SHOP_DATA      *pShop;
	pShop = NULL;
	for (keeper = ch->in_room->people; keeper; keeper = keeper->next_in_room) {
		if (IS_NPC(keeper) && (pShop = keeper->pIndexData->pShop) != NULL)
			break;
	}

	if (pShop == NULL) {
		send_to_char("You can't do that here.\n\r", ch);
		return NULL;
	}
	if (IS_SET(keeper->in_room->area->area_flag, AREA_HOMETOWN)
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
		return NULL;
	}
	return keeper;
}

/* insert an object at the right spot for the keeper */
void 
obj_to_keeper(OBJ_DATA * obj, CHAR_DATA * ch)
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
OBJ_DATA       *
get_obj_keeper(CHAR_DATA * ch, CHAR_DATA * keeper, const char *argument)
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

int 
get_cost(CHAR_DATA * keeper, OBJ_DATA * obj, bool fBuy)
{
	SHOP_DATA      *pShop;
	int             cost;
	if (obj == NULL || (pShop = keeper->pIndexData->pShop) == NULL)
		return 0;

	if (IS_OBJ_STAT(obj, ITEM_NOSELL))
		return 0;

	if (fBuy) {
		cost = obj->cost * pShop->profit_buy / 100;
	} else {
		OBJ_DATA       *obj2;
		int             itype;
		cost = 0;
		for (itype = 0; itype < MAX_TRADE; itype++) {
			if (obj->item_type == pShop->buy_type[itype]) {
				cost = obj->cost * pShop->profit_sell / 100;
				break;
			}
		}

		if (!IS_OBJ_STAT(obj, ITEM_SELL_EXTRACT))
			for (obj2 = keeper->carrying; obj2; obj2 = obj2->next_content) {
				if (obj->pIndexData == obj2->pIndexData
				&& !mlstr_cmp(obj->short_descr, obj2->short_descr))
					return 0;
/*
	 	    if (IS_OBJ_STAT(obj2,ITEM_INVENTORY))
			cost /= 2;
		    else
		              	cost = cost * 3 / 4;
*/
			}
	}

	if (obj->item_type == ITEM_STAFF || obj->item_type == ITEM_WAND) {
		if (obj->value[1] == 0)
			cost /= 4;
		else
			cost = cost * obj->value[2] / obj->value[1];
	}
	return cost;
}


void 
do_buy_pet(CHAR_DATA * ch, const char *argument)
{
	int             cost, roll;
	char            arg[MAX_INPUT_LENGTH];
	CHAR_DATA      *pet;
	ROOM_INDEX_DATA *pRoomIndexNext;
	ROOM_INDEX_DATA *in_room;
	if (IS_NPC(ch))
		return;

	argument = one_argument(argument, arg);
	smash_tilde(arg);

	/* hack to make new thalos pets work */
	if (ch->in_room->vnum == 9621)
		pRoomIndexNext = get_room_index(9706);
	else
		pRoomIndexNext = get_room_index(ch->in_room->vnum + 1);

	if (pRoomIndexNext == NULL) {
		bug("Do_buy: bad pet shop at vnum %d.", ch->in_room->vnum);
		send_to_char("Sorry, you can't buy that here.\n\r", ch);
		return;
	}
	in_room = ch->in_room;
	ch->in_room = pRoomIndexNext;
	pet = get_char_room(ch, arg);
	ch->in_room = in_room;

	if (pet == NULL || !IS_SET(pet->act, ACT_PET) || !IS_NPC(pet)) {
		send_to_char("Sorry, you can't buy that here.\n\r", ch);
		return;
	}
	if (IS_SET(pet->act, ACT_RIDEABLE)
	    && ch->clan == CLAN_KNIGHT && !MOUNTED(ch)) {
		cost = 10 * pet->level * pet->level;

		if ((ch->silver + 100 * ch->gold) < cost) {
			send_to_char("You can't afford it.\n\r", ch);
			return;
		}
		if (ch->level < pet->level + 5) {
			send_to_char("You're not powerful enough "
				     "to master this pet.\n\r", ch);
			return;
		}
		deduct_cost(ch, cost);
		pet = create_mobile(pet->pIndexData);
		pet->comm = COMM_NOTELL | COMM_NOSHOUT | COMM_NOCHANNELS;

		char_to_room(pet, ch->in_room);
		do_mount(ch, pet->name);
		send_to_char("Enjoy your mount.\n\r", ch);
		act("$n bought $N as a mount.", ch, NULL, pet, TO_ROOM);
		return;
	}
	if (ch->pet != NULL) {
		send_to_char("You already own a pet.\n\r", ch);
		return;
	}
	cost = 10 * pet->level * pet->level;

	if ((ch->silver + 100 * ch->gold) < cost) {
		send_to_char("You can't afford it.\n\r", ch);
		return;
	}
	if (ch->level < pet->level) {
		send_to_char("You're not powerful enough "
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
	pet = create_mobile(pet->pIndexData);
	SET_BIT(pet->act, ACT_PET);
	SET_BIT(pet->affected_by, AFF_CHARM);
	pet->comm = COMM_NOTELL | COMM_NOSHOUT | COMM_NOCHANNELS;

	argument = one_argument(argument, arg);
	if (arg[0] != '\0')
		str_printf(&pet->name, arg);

	mlstr_printf(pet->description, ch->name);

	char_to_room(pet, ch->in_room);
	add_follower(pet, ch);
	pet->leader = ch;
	ch->pet = pet;
	send_to_char("Enjoy your pet.\n\r", ch);
	act("$n bought $N as a pet.", ch, NULL, pet, TO_ROOM);
	return;
}

void 
do_buy(CHAR_DATA * ch, const char *argument)
{
	int             cost, roll;
	CHAR_DATA      *keeper;
	OBJ_DATA       *obj, *t_obj;
	char            arg[MAX_INPUT_LENGTH];
	int             number, count = 1;
	if (argument[0] == '\0') {
		send_to_char("Buy what?\n\r", ch);
		return;
	}
	if (IS_SET(ch->in_room->room_flags, ROOM_PET_SHOP)) {
		do_buy_pet(ch, argument);
		return;
	}
	if ((keeper = find_keeper(ch)) == NULL)
		return;

	number = mult_argument(argument, arg);
	obj = get_obj_keeper(ch, keeper, arg);
	cost = get_cost(keeper, obj, TRUE);

	if (cost <= 0 || !can_see_obj(ch, obj)) {
		act("$n tells you '{GI don't sell that -- try 'list'{x'.",
		    keeper, NULL, ch, TO_VICT);
		ch->reply = keeper;
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
			act("$n tells you '{GI don't have that many in stock{x.",
			    keeper, NULL, ch, TO_VICT);
			ch->reply = keeper;
			return;
		}
	}
	if ((ch->silver + ch->gold * 100) < cost * number) {
		if (number > 1)
			act("$n tells you '{GYou can't afford to buy that many.{x",
			    keeper, obj, ch, TO_VICT);
		else
			act("$n tells you '{GYou can't afford to buy $p{x'.",
			    keeper, obj, ch, TO_VICT);
		ch->reply = keeper;
		return;
	}
	if (obj->level > ch->level) {
		act("$n tells you '{GYou can't use $p yet{x'.",
		    keeper, obj, ch, TO_VICT);
		ch->reply = keeper;
		return;
	}
	if (ch->carry_number + number * get_obj_number(obj) > can_carry_n(ch)) {
		send_to_char("You can't carry that many items.\n\r", ch);
		return;
	}
	if (ch->carry_weight + number * get_obj_weight(obj) > can_carry_w(ch)) {
		send_to_char("You can't carry that much weight.\n\r", ch);
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
			t_obj = create_object(obj->pIndexData, obj->level);
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



void 
do_list(CHAR_DATA * ch, const char *argument)
{
	if (IS_SET(ch->in_room->room_flags, ROOM_PET_SHOP)) {
		ROOM_INDEX_DATA *pRoomIndexNext;
		CHAR_DATA      *pet;
		bool            found;
		/* hack to make new thalos pets work */

		if (ch->in_room->vnum == 9621)
			pRoomIndexNext = get_room_index(9706);
		else
			pRoomIndexNext = get_room_index(ch->in_room->vnum + 1);

		if (pRoomIndexNext == NULL) {
			bug("Do_list: bad pet shop at vnum %d.", ch->in_room->vnum);
			send_to_char("You can't do that here.\n\r", ch);
			return;
		}
		found = FALSE;
		for (pet = pRoomIndexNext->people; pet; pet = pet->next_in_room) {
			if (!IS_NPC(pet))
				continue;	/* :) */
			if (IS_SET(pet->act, ACT_PET)) {
				if (!found) {
					found = TRUE;
					send_to_char("Pets for sale:\n\r", ch);
				}
				char_printf(ch, "[%2d] %8d - %s\n\r",
					    pet->level,
					    10 * pet->level * pet->level,
					    mlstr_cval(pet->short_descr, ch));
			}
		}
		if (!found)
			send_to_char("Sorry, we're out of pets right now.\n\r", ch);
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
					send_to_char("[Lv Price Qty] Item\n\r", ch);
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
			send_to_char("You can't buy anything here.\n\r", ch);
		return;
	}
}



void 
do_sell(CHAR_DATA * ch, const char *argument)
{
	char            arg[MAX_INPUT_LENGTH];
	CHAR_DATA      *keeper;
	OBJ_DATA       *obj;
	int             cost, roll;
	int             gold, silver;
	one_argument(argument, arg);

	if (arg[0] == '\0') {
		send_to_char("Sell what?\n\r", ch);
		return;
	}
	if ((keeper = find_keeper(ch)) == NULL)
		return;

	if ((obj = get_obj_carry(ch, arg)) == NULL) {
		act("$n tells you 'You don't have that item'.",
		    keeper, NULL, ch, TO_VICT);
		ch->reply = keeper;
		return;
	}
	if (!can_drop_obj(ch, obj)) {
		send_to_char("You can't let go of it.\n\r", ch);
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
		act("$n tells you 'I'm afraid I don't have enough wealth to buy $p.",
		    keeper, obj, ch, TO_VICT);
		return;
	}
	act("$n sells $p.", ch, obj, NULL, TO_ROOM);
	/* haggle */
	roll = number_percent();
	if (!IS_OBJ_STAT(obj, ITEM_SELL_EXTRACT) && roll < get_skill(ch, gsn_haggle)) {
		roll = get_skill(ch, gsn_haggle) + number_range(1, 20) - 10;
		send_to_char("You haggle with the shopkeeper.\n\r", ch);
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
			   gold, gold > 1 ? "s" : "");
	else if (silver)
		act_printf(ch, obj, NULL, TO_CHAR, POS_RESTING,
			   "You sell $p for %d silver pieces%s.",
			   silver, silver > 1 ? "s" : "");
	ch->gold += gold;
	ch->silver += silver;
	deduct_cost(keeper, cost);
	if (keeper->gold < 0)
		keeper->gold = 0;
	if (keeper->silver < 0)
		keeper->silver = 0;

	if (obj->item_type == ITEM_TRASH || IS_OBJ_STAT(obj, ITEM_SELL_EXTRACT)) {
		extract_obj(obj);
	} else {
		obj_from_char(obj);
		if (obj->timer)
			SET_BIT(obj->extra_flags, ITEM_HAD_TIMER);
		else
			obj->timer = number_range(50, 100);
		obj_to_keeper(obj, keeper);
	}

	return;
}



void 
do_value(CHAR_DATA * ch, const char *argument)
{
	char            arg[MAX_INPUT_LENGTH];
	CHAR_DATA      *keeper;
	OBJ_DATA       *obj;
	int             cost;
	one_argument(argument, arg);

	if (arg[0] == '\0') {
		send_to_char("Value what?\n\r", ch);
		return;
	}
	if ((keeper = find_keeper(ch)) == NULL)
		return;

	if ((obj = get_obj_carry(ch, arg)) == NULL) {
		act("$n tells you 'You don't have that item'.",
		    keeper, NULL, ch, TO_VICT);
		ch->reply = keeper;
		return;
	}
	if (!can_see_obj(keeper, obj)) {
		act("$n doesn't see what you are offering.", keeper, NULL, ch, TO_VICT);
		return;
	}
	if (!can_drop_obj(ch, obj)) {
		send_to_char("You can't let go of it.\n\r", ch);
		return;
	}
	if ((cost = get_cost(keeper, obj, FALSE)) <= 0) {
		act("$n looks uninterested in $p.", keeper, obj, ch, TO_VICT);
		return;
	}
	act_printf(keeper, obj, ch, TO_VICT, POS_RESTING,
		   "$n tells you '{GI'll give you %d silver and %d gold coins for $p{x'.",
		   cost - (cost / 100) * 100, cost / 100);
	ch->reply = keeper;

	return;
}


void 
do_herbs(CHAR_DATA * ch, const char *argument)
{
	CHAR_DATA      *victim;
	char            arg[MAX_INPUT_LENGTH];
	one_argument(argument, arg);

	if (IS_NPC(ch))
		return;

	if (is_affected(ch, gsn_herbs)) {
		send_to_char("You can't find any more herbs.\n\r", ch);
		return;
	}
	if (arg[0] == '\0')
		victim = ch;
	else if ((victim = get_char_room(ch, arg)) == NULL) {
		send_to_char("They're not here.\n\r", ch);
		return;
	}
	WAIT_STATE(ch, skill_table[gsn_herbs].beats);

	if (ch->in_room->sector_type != SECT_INSIDE &&
	    ch->in_room->sector_type != SECT_CITY &&
	    (IS_NPC(ch) || number_percent() < get_skill(ch, gsn_herbs))) {
		AFFECT_DATA     af;
		af.where = TO_AFFECTS;
		af.type = gsn_herbs;
		af.level = ch->level;
		af.duration = 5;
		af.location = APPLY_NONE;
		af.modifier = 0;
		af.bitvector = 0;

		affect_to_char(ch, &af);

		send_to_char("You gather some beneficial herbs.\n\r", ch);
		act("$n gathers some herbs.", ch, NULL, NULL, TO_ROOM);

		if (ch != victim) {
			act("$n gives you some herbs to eat.", ch, NULL, victim, TO_VICT);
			act("You give the herbs to $N.", ch, NULL, victim, TO_CHAR);
			act("$n gives the herbs to $N.", ch, NULL, victim, TO_NOTVICT);
		}
		if (victim->hit < victim->max_hit) {
			send_to_char("You feel better.\n\r", victim);
			act("$n looks better.", victim, NULL, NULL, TO_ROOM);
		}
		victim->hit = UMIN(victim->max_hit, victim->hit + 5 * ch->level);
		check_improve(ch, gsn_herbs, TRUE, 1);
		if (is_affected(victim, gsn_plague)) {
			if (check_dispel(ch->level, victim, gsn_plague)) {
				send_to_char("Your sores vanish.\n\r", victim);
				act("$n looks relieved as $s sores vanish.", victim, NULL, NULL, TO_ROOM);
			}
		}
	} else {
		send_to_char("You search for herbs but find none here.\n\r", ch);
		act("$n looks around for herbs.", ch, NULL, NULL, TO_ROOM);
		check_improve(ch, gsn_herbs, FALSE, 1);
	}
}

void 
do_lore(CHAR_DATA * ch, const char *argument)
{
	char            arg1[MAX_INPUT_LENGTH];
	OBJ_DATA       *obj;
	AFFECT_DATA    *paf;
	int             chance;
	int             skill;
	int             value0, value1, value2, value3, value4;
	argument = one_argument(argument, arg1);

	if ((obj = get_obj_carry(ch, arg1)) == NULL) {
		send_to_char("You do not have that object.\n\r", ch);
		return;
	}
	if (ch->mana < 30) {
		send_to_char("You don't have enough mana.\n\r", ch);
		return;
	}
	if (IS_NPC(ch) || (skill = get_skill(ch, gsn_lore)) < 10) {
		send_to_char("The meaning of this object escapes you for the moment.\n\r", ch);
		return;
	}
	/* a random lore */
	chance = number_percent();

	if (skill < 20) {
		char_printf(ch, "Object '%s'.\n\r", obj->name);
		ch->mana -= 30;
		check_improve(ch, gsn_lore, TRUE, 8);
		return;
	} else if (get_skill(ch, gsn_lore) < 40) {
		char_printf(ch,
			    "Object '%s'.  Weight is %d, value is %d.\n\r",
			    obj->name,
		chance < 60 ? obj->weight : number_range(1, 2 * obj->weight),
		     chance < 60 ? number_range(1, 2 * obj->cost) : obj->cost
			);
		if (str_cmp(obj->material, "oldstyle"))
			char_printf(ch, "Material is %s.\n\r", obj->material);
		ch->mana -= 30;
		check_improve(ch, gsn_lore, TRUE, 7);
		return;
	} else if (get_skill(ch, gsn_lore) < 60) {
		char_printf(ch,
			    "Object '%s' has weight %d.\n\rValue is %d, level is %d.\n\rMaterial is %s.\n\r",
			    obj->name,
			    obj->weight,
		    chance < 60 ? number_range(1, 2 * obj->cost) : obj->cost,
		  chance < 60 ? obj->level : number_range(1, 2 * obj->level),
		str_cmp(obj->material, "oldstyle") ? obj->material : "unknown"
			);
		ch->mana -= 30;
		check_improve(ch, gsn_lore, TRUE, 6);
		return;
	} else if (get_skill(ch, gsn_lore) < 80) {
		char_printf(ch,
			    "Object '%s' is type %s, extra flags %s.\n\rWeight is %d, value is %d, level is %d.\n\rMaterial is %s.\n\r",
			    obj->name,
			    item_type_name(obj),
			    extra_bit_name(obj->extra_flags),
			    obj->weight,
		    chance < 60 ? number_range(1, 2 * obj->cost) : obj->cost,
		  chance < 60 ? obj->level : number_range(1, 2 * obj->level),
		str_cmp(obj->material, "oldstyle") ? obj->material : "unknown"
			);
		ch->mana -= 30;
		check_improve(ch, gsn_lore, TRUE, 5);
		return;
	} else if (get_skill(ch, gsn_lore) < 85) 
		char_printf(ch,
			    "Object '%s' is type %s, extra flags %s.\n\rWeight is %d, value is %d, level is %d.\n\rMaterial is %s.\n\r",
			    obj->name,
			    item_type_name(obj),
			    extra_bit_name(obj->extra_flags),
			    obj->weight,
			    obj->cost,
			    obj->level,
		str_cmp(obj->material, "oldstyle") ? obj->material : "unknown"
			);
	else
		char_printf(ch,
			    "Object '%s' is type %s, extra flags %s.\n\rWeight is %d, value is %d, level is %d.\n\rMaterial is %s.\n\r",
			    obj->name,
			    item_type_name(obj),
			    extra_bit_name(obj->extra_flags),
			    obj->weight,
			    obj->cost,
			    obj->level,
		str_cmp(obj->material, "oldstyle") ? obj->material : "unknown"
			);

	ch->mana -= 30;

	value0 = obj->value[0];
	value1 = obj->value[1];
	value2 = obj->value[2];
	value3 = obj->value[3];
	value4 = obj->value[4];

	switch (obj->item_type) {
	case ITEM_SCROLL:
	case ITEM_POTION:
	case ITEM_PILL:
		if (get_skill(ch, gsn_lore) < 85) {
			value0 = number_range(1, 60);
			if (chance > 40) {
				value1 = number_range(1, (MAX_SKILL - 1));
				if (chance > 60) {
					value2 = number_range(1, (MAX_SKILL - 1));
					if (chance > 80)
						value3 = number_range(1, (MAX_SKILL - 1));
				}
			}
		} else {
			if (chance > 60) {
				value1 = number_range(1, (MAX_SKILL - 1));
				if (chance > 80) {
					value2 = number_range(1, (MAX_SKILL - 1));
					if (chance > 95)
						value3 = number_range(1, (MAX_SKILL - 1));
				}
			}
		}

		char_printf(ch, "Level %d spells of:", obj->value[0]);

		if (value1 >= 0 && value1 < MAX_SKILL)
			char_printf(ch, " '%s'", skill_table[value1].name);

		if (value2 >= 0 && value2 < MAX_SKILL)
			char_printf(ch, " '%s'", skill_table[value2].name);

		if (value3 >= 0 && value3 < MAX_SKILL)
			char_printf(ch, " '%s'", skill_table[value3].name);

		if (value4 >= 0 && value4 < MAX_SKILL)
			char_printf(ch, " '%s'", skill_table[value4].name);

		send_to_char(".\n\r", ch);
		break;

	case ITEM_WAND:
	case ITEM_STAFF:
		if (get_skill(ch, gsn_lore) < 85) {
			value0 = number_range(1, 60);
			if (chance > 40) {
				value3 = number_range(1, (MAX_SKILL - 1));
				if (chance > 60) {
					value2 = number_range(0, 2 * obj->value[2]);
					if (chance > 80)
						value1 = number_range(0, value2);
				}
			}
		} else {
			if (chance > 60) {
				value3 = number_range(1, (MAX_SKILL - 1));
				if (chance > 80) {
					value2 = number_range(0, 2 * obj->value[2]);
					if (chance > 95)
						value1 = number_range(0, value2);
				}
			}
		}

		char_printf(ch, "Has %d(%d) charges of level %d",
			    value1, value2, value0);

		if (value3 >= 0 && value3 < MAX_SKILL) {
			send_to_char(" '", ch);
			send_to_char(skill_table[value3].name, ch);
			send_to_char("'", ch);
		}
		send_to_char(".\n\r", ch);
		break;

	case ITEM_WEAPON:
		send_to_char("Weapon type is ", ch);
		if (get_skill(ch, gsn_lore) < 85) {
			value0 = number_range(0, 8);
			if (chance > 33) {
				value1 = number_range(1, 2 * obj->value[1]);
				if (chance > 66)
					value2 = number_range(1, 2 * obj->value[2]);
			}
		} else {
			if (chance > 50) {
				value1 = number_range(1, 2 * obj->value[1]);
				if (chance > 75)
					value2 = number_range(1, 2 * obj->value[2]);
			}
		}

		switch (value0) {
		case (WEAPON_EXOTIC):
			send_to_char("exotic.\n\r", ch);
			break;
		case (WEAPON_SWORD):
			send_to_char("sword.\n\r", ch);
			break;
		case (WEAPON_DAGGER):
			send_to_char("dagger.\n\r", ch);
			break;
		case (WEAPON_SPEAR):
			send_to_char("spear/staff.\n\r", ch);
			break;
		case (WEAPON_MACE):
			send_to_char("mace/club.\n\r", ch);
			break;
		case (WEAPON_AXE):
			send_to_char("axe.\n\r", ch);
			break;
		case (WEAPON_FLAIL):
			send_to_char("flail.\n\r", ch);
			break;
		case (WEAPON_WHIP):
			send_to_char("whip.\n\r", ch);
			break;
		case (WEAPON_POLEARM):
			send_to_char("polearm.\n\r", ch);
			break;
		case (WEAPON_BOW):
			send_to_char("bow.\n\r", ch);
			break;
		case (WEAPON_ARROW):
			send_to_char("arrow.\n\r", ch);
			break;
		case (WEAPON_LANCE):
			send_to_char("lance.\n\r", ch);
			break;
		default:
			send_to_char("unknown.\n\r", ch);
			break;
		}
		if (obj->pIndexData->new_format)
			char_printf(ch, "Damage is %dd%d (average %d).\n\r",
				    value1, value2,
				    (1 + value2) * value1 / 2);
		else
			char_printf(ch, "Damage is %d to %d (average %d).\n\r",
				    value1, value2,
				    (value1 + value2) / 2);
		break;

	case ITEM_ARMOR:
		if (get_skill(ch, gsn_lore) < 85) {
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
		} else {
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

		char_printf(ch,
			    "Armor class is %d pierce, %d bash, %d slash, and %d vs. magic.\n\r",
			    value0, value1, value2, value3);
		break;
	}

	if (get_skill(ch, gsn_lore) < 87) {
		check_improve(ch, gsn_lore, TRUE, 5);
		return;
	}

	if (!obj->enchanted)
		for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next)
			if (paf->location != APPLY_NONE && paf->modifier != 0)
				char_printf(ch, "Affects %s by %d.\n\r",
					    affect_loc_name(paf->location), paf->modifier);

	for (paf = obj->affected; paf != NULL; paf = paf->next)
		if (paf->location != APPLY_NONE && paf->modifier != 0)
			char_printf(ch, "Affects %s by %d.\n\r",
			      affect_loc_name(paf->location), paf->modifier);
	check_improve(ch, gsn_lore, TRUE, 5);
	return;
}



void 
do_butcher(CHAR_DATA * ch, const char *argument)
{
	OBJ_DATA       *obj;
	char            arg[MAX_STRING_LENGTH];
	OBJ_DATA       *tmp_obj;
	OBJ_DATA       *tmp_next;
	if (IS_NPC(ch))
		return;

	one_argument(argument, arg);
	if (arg[0] == '\0') {
		send_to_char("Butcher what?\n\r", ch);
		return;
	}
	if ((obj = get_obj_here(ch, arg)) == NULL) {
		send_to_char("You do not see that here.\n\r", ch);
		return;
	}
	if (obj->item_type != ITEM_CORPSE_PC
	    && obj->item_type != ITEM_CORPSE_NPC) {
		send_to_char("You can't butcher that.\n\r", ch);
		return;
	}
	if (obj->carried_by != NULL) {
		send_to_char("Put it down first.\n\r", ch);
		return;
	}
	if (!IS_NPC(ch) && get_skill(ch, gsn_butcher) < 1) {
		send_to_char("You don't have the precision instruments "
			     "for that.", ch);
		return;
	}
	obj_from_room(obj);

	for (tmp_obj = obj->contains; tmp_obj != NULL; tmp_obj = tmp_next) {
		tmp_next = tmp_obj->next_content;
		obj_from_obj(tmp_obj);
		obj_to_room(tmp_obj, ch->in_room);
	}


	if (IS_NPC(ch) || number_percent() < get_skill(ch, gsn_butcher)) {
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
		check_improve(ch, gsn_butcher, TRUE, 1);

		for (i = 0; i < numsteaks; i++) {
			steak = create_object(get_obj_index(OBJ_VNUM_STEAK), 0);

			mlstr_printf(steak->short_descr,
					mlstr_mval(obj->short_descr));
			mlstr_printf(steak->description,
					mlstr_mval(obj->short_descr));
			obj_to_room(steak, ch->in_room);
		}
	} else {
		act("You fail and destroy $p.", ch, obj, NULL, TO_CHAR);
		act("$n fails to butcher $p and destroys it.",
		    ch, obj, NULL, TO_ROOM);

		check_improve(ch, gsn_butcher, FALSE, 1);
	}
	extract_obj(obj);
}


void 
do_balance(CHAR_DATA * ch, const char *argument)
{
	char            buf[160];
	char            buf2[100];
	long            bank_g;
	long            bank_s;
	if (IS_NPC(ch)) {
		send_to_char("You don't have a bank account.\n\r", ch);
		return;
	}
	if (!IS_SET(ch->in_room->room_flags, ROOM_BANK)) {
		send_to_char("You are not in a bank.\n\r", ch);
		return;
	}
	if (ch->pcdata->bank_s + ch->pcdata->bank_g == 0) {
		send_to_char("You don't have any money in the bank.\n\r", ch);
		return;
	}
	bank_g = ch->pcdata->bank_g;
	bank_s = ch->pcdata->bank_s;
	sprintf(buf, "You have %s%s%s coin%s in the bank.\n\r",
		bank_g != 0 ? "%ld gold" : "",
		(bank_g != 0) && (bank_s != 0) ? " and " : "",
		bank_s != 0 ? "%ld silver" : "",
		bank_s + bank_g > 1 ? "s" : "");
	if (bank_g == 0)
		sprintf(buf2, buf, bank_s);
	else
		sprintf(buf2, buf, bank_g, bank_s);

	send_to_char(buf2, ch);
}

void 
do_withdraw(CHAR_DATA * ch, const char *argument)
{
	long            amount_s;
	long            amount_g;
	char            arg[MAX_INPUT_LENGTH];
	if (IS_NPC(ch)) {
		send_to_char("You don't have a bank account.\n\r", ch);
		return;
	}
	if (!IS_SET(ch->in_room->room_flags, ROOM_BANK)) {
		send_to_char("The mosquito by your feet will not give you any money.\n\r", ch);
		return;
	}
	argument = one_argument(argument, arg);
	if (arg[0] == '\0') {
		send_to_char("Withdraw how much?\n\r", ch);
		return;
	}
	amount_s = labs(atol(arg));
	if (!str_cmp(argument, "silver") || argument[0] == '\0')
		amount_g = 0;
	else if (!str_cmp(argument, "gold")) {
		amount_g = amount_s;
		amount_s = 0;
	} else {
		send_to_char("You can withdraw gold and silver coins only.", ch);
		return;
	}

	if (amount_g > ch->pcdata->bank_g) {
		send_to_char("Sorry, we don't give loans.\n\r", ch);
		return;
	}
	if (amount_s > ch->pcdata->bank_s) {
		send_to_char("Sorry, we don't give loans.\n\r", ch);
		return;
	}
	ch->pcdata->bank_g -= amount_g;
	ch->pcdata->bank_s -= amount_s;
	ch->gold += 0.98 * amount_g;
	ch->silver += 0.90 * amount_s;
	if (amount_s > 0 && amount_s < 10) {
		if (amount_s == 1)
			char_printf(ch, "One coin??!!! You cheapskate!\n\r");
		else
			char_printf(ch, "%ld coins??!!! You cheapskate!\n\r", amount_s);
	} else
		char_printf(ch,
			    "Here are your %ld %s coins, minus a %ld coin withdrawal fee.\n\r",
			    amount_s != 0 ? amount_s : amount_g,
			    amount_s != 0 ? "silver" : "gold",
			  amount_s != 0 ? (long) UMAX(1, (0.10 * amount_s)) :
			    (long) UMAX(1, (0.02 * amount_g)));
	act("$n steps up to the teller window.", ch, NULL, NULL, TO_ROOM);
}

void 
do_deposit(CHAR_DATA * ch, const char *argument)
{
	long            amount_s;
	long            amount_g;
	char            arg[200];
	if (IS_NPC(ch)) {
		send_to_char("You don't have a bank account.\n\r", ch);
		return;
	}
	if (!IS_SET(ch->in_room->room_flags, ROOM_BANK)) {
		send_to_char("The ant by your feet can't carry your gold.\n\r", ch);
		return;
	}
	argument = one_argument(argument, arg);
	if (arg[0] == '\0') {
		send_to_char("Deposit how much?\n\r", ch);
		return;
	}
	amount_s = labs(atol(arg));
	if (!str_cmp(argument, "silver") || argument[0] == '\0')
		amount_g = 0;
	else if (!str_cmp(argument, "gold")) {
		amount_g = amount_s;
		amount_s = 0;
	} else {
		send_to_char("You can deposit gold and silver coins only.", ch);
		return;
	}

	if (amount_g > ch->gold) {
		send_to_char("That's more than you've got.\n\r", ch);
		return;
	}
	if (amount_s > ch->silver) {
		send_to_char("That's more than you've got.\n\r", ch);
		return;
	}
	ch->pcdata->bank_s += amount_s;
	ch->pcdata->bank_g += amount_g;
	ch->gold -= amount_g;
	ch->silver -= amount_s;

	if (amount_s == 1)
		char_printf(ch, "Oh boy! One gold coin!\n\r");
	else
		char_printf(ch, "%ld %s coins deposited. Come again soon!\n\r",
			    amount_s != 0 ? amount_s : amount_g,
			    amount_s != 0 ? "silver" : "gold");
	act("$n steps up to the teller window.", ch, NULL, NULL, TO_ROOM);
}



/* wear object as a secondary weapon */

void 
do_second_wield(CHAR_DATA * ch, const char *argument)
{
	OBJ_DATA       *obj;
	int             sn, skill;
	if (get_skill(ch, gsn_second_weapon) == 0) {
		send_to_char("You don't know how to wield a second weapon.\n\r", ch);
		return;
	}
	if (argument[0] == '\0') {
		send_to_char("Wear which weapon in your off-hand?\n\r", ch);
		return;
	}
	obj = get_obj_carry(ch, argument);
	if (obj == NULL) {
		send_to_char("You don't have that item.\n\r", ch);
		return;
	}
	if (!CAN_WEAR(obj, ITEM_WIELD)) {
		send_to_char("You can't wield that as your second weapon.\n\r", ch);
		return;
	}
	if ((get_eq_char(ch, WEAR_SHIELD) != NULL) ||
	    (get_eq_char(ch, WEAR_HOLD) != NULL)) {
		send_to_char("You cannot use a secondary weapon while using a shield or holding an item\n\r", ch);
		return;
	}
	if (ch->level < (obj->level - 3)) {
		char_printf(ch, "You must be level %d to use this object.\n\r",
			    obj->level);
		act("$n tries to use $p, but is too inexperienced.",
		    ch, obj, NULL, TO_ROOM);
		return;
	}
	if (IS_WEAPON_STAT(obj, WEAPON_TWO_HANDS)) {
		send_to_char("It mustn't be a two-handed weapon!\n\r", ch);
		return;
	}
	if (get_eq_char(ch, WEAR_WIELD) == NULL) {
		send_to_char("You need to wield a primary weapon, before using a secondary one!\n\r", ch);
		return;
	}
	if (get_obj_weight(obj) > (str_app[get_curr_stat(ch, STAT_STR)].wield * 5)) {
		send_to_char("This weapon is too heavy to be used as a secondary weapon by you.\n\r", ch);
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
	return;
}


void 
do_enchant(CHAR_DATA * ch, const char *argument)
{
	OBJ_DATA       *obj;
	int             wear_level;
	int             chance;
	if ((chance = get_skill(ch, gsn_enchant_sword)) == 0) {
		send_to_char("Huh?.\n\r", ch);
		return;
	}
	if (argument[0] == '\0') {	/* empty */
		send_to_char("Wear which weapon to enchant?\n\r", ch);
		return;
	}
	obj = get_obj_carry(ch, argument);

	if (obj == NULL) {
		send_to_char("You don't have that item.\n\r", ch);
		return;
	}
	wear_level = ch->level;
	if ((class_table[ch->class].fMana && obj->item_type == ITEM_ARMOR)
	 || (!class_table[ch->class].fMana && obj->item_type == ITEM_WEAPON))
		wear_level += 3;

	if (wear_level < obj->level) {
		char_printf(ch, "You must be level %d to be able to enchant this object.\n\r", obj->level);
		act("$n tries to enchant $p, but is too inexperienced.",
		    ch, obj, NULL, TO_ROOM);
		return;
	}
	if (ch->mana < 100) {
		send_to_char("You don't have enough mana.\n\r", ch);
		return;
	}
	if (number_percent() > chance) {
		send_to_char("You lost your concentration.\n\r", ch);
		act("$n tries to enchant $p, but he forgets how for a moment.",
		    ch, obj, NULL, TO_ROOM);
		WAIT_STATE(ch, skill_table[gsn_enchant_sword].beats);
		check_improve(ch, gsn_enchant_sword, FALSE, 6);
		ch->mana -= 50;
		return;
	}
	ch->mana -= 100;
	spell_enchant_weapon(24, ch->level, ch, obj, TARGET_OBJ);
	check_improve(ch, gsn_enchant_sword, TRUE, 2);
	WAIT_STATE(ch, skill_table[gsn_enchant_sword].beats);
	return;
}
