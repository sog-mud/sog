/*
 * $Id: act_move.c,v 1.305 2004-05-26 16:28:50 tatyana Exp $
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

#include <sys/time.h>
#include <stdio.h>
#include <string.h>

#include <merc.h>
#include <lang.h>

#include <sog.h>

DECLARE_DO_FUN(do_north);
DECLARE_DO_FUN(do_east);
DECLARE_DO_FUN(do_south);
DECLARE_DO_FUN(do_west);
DECLARE_DO_FUN(do_up);
DECLARE_DO_FUN(do_down);
DECLARE_DO_FUN(do_open);
DECLARE_DO_FUN(do_close);
DECLARE_DO_FUN(do_lock);
DECLARE_DO_FUN(do_unlock);
DECLARE_DO_FUN(do_pick);
DECLARE_DO_FUN(do_stand);
DECLARE_DO_FUN(do_rest);
DECLARE_DO_FUN(do_sit);
DECLARE_DO_FUN(do_sleep);
DECLARE_DO_FUN(do_wake);
DECLARE_DO_FUN(do_sneak);
DECLARE_DO_FUN(do_hide);
DECLARE_DO_FUN(do_camouflage);
DECLARE_DO_FUN(do_blend);
DECLARE_DO_FUN(do_acute);
DECLARE_DO_FUN(do_visible);
DECLARE_DO_FUN(do_recall);
DECLARE_DO_FUN(do_train);
DECLARE_DO_FUN(do_track);
DECLARE_DO_FUN(do_vampire);
DECLARE_DO_FUN(do_vbite);
DECLARE_DO_FUN(do_bash_door);
DECLARE_DO_FUN(do_blink);
DECLARE_DO_FUN(do_vanish);
DECLARE_DO_FUN(do_kidnap);
DECLARE_DO_FUN(do_fade);
DECLARE_DO_FUN(do_vtouch);
DECLARE_DO_FUN(do_fly);
DECLARE_DO_FUN(do_push);
DECLARE_DO_FUN(do_crecall);
DECLARE_DO_FUN(do_escape);
DECLARE_DO_FUN(do_layhands);
DECLARE_DO_FUN(do_mount);
DECLARE_DO_FUN(do_dismount);
DECLARE_DO_FUN(do_charge);
DECLARE_DO_FUN(do_shoot);
DECLARE_DO_FUN(do_human);
DECLARE_DO_FUN(do_revert);
DECLARE_DO_FUN(do_throw_weapon);
DECLARE_DO_FUN(do_enter);
DECLARE_DO_FUN(do_settraps);
DECLARE_DO_FUN(do_forest);
DECLARE_DO_FUN(do_breathhold);
DECLARE_DO_FUN(do_cast);
DECLARE_DO_FUN(do_pray);

/* command procedures needed */
DECLARE_DO_FUN(do_look);
DECLARE_DO_FUN(do_yell);
DECLARE_DO_FUN(do_help);

/*
 * Local functions.
 */
static int find_exit(CHAR_DATA *ch, char *arg);

DO_FUN(do_north, ch, argument)
{
	move_char(ch, DIR_NORTH, 0);
}

DO_FUN(do_east, ch, argument)
{
	move_char(ch, DIR_EAST, 0);
}

DO_FUN(do_south, ch, argument)
{
	move_char(ch, DIR_SOUTH, 0);
}

DO_FUN(do_west, ch, argument)
{
	move_char(ch, DIR_WEST, 0);
}

DO_FUN(do_up, ch, argument)
{
	move_char(ch, DIR_UP, 0);
}

DO_FUN(do_down, ch, argument)
{
	move_char(ch, DIR_DOWN, 0);
}

DO_FUN(do_open, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		act_char("Open what?", ch);
		return;
	}

	if ((obj = get_obj_here(ch, arg)) != NULL) {
		open_obj(ch, obj);
		return;
	}

	open_door(ch, arg);
}

DO_FUN(do_close, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		act_char("Close what?", ch);
		return;
	}

	if ((obj = get_obj_here(ch, arg)) != NULL) {
		close_obj(ch, obj);
		return;
	}

	close_door(ch, arg);
}

DO_FUN(do_lock, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		act_char("Lock what?", ch);
		return;
	}

	if ((obj = get_obj_here(ch, arg)) != NULL) {
		lock_obj(ch, obj);
		return;
	}

	lock_door(ch, arg);
}

DO_FUN(do_unlock, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		act_char("Unlock what?", ch);
		return;
	}

	if ((obj = get_obj_here(ch, arg)) != NULL) {
		unlock_obj(ch, obj);
		return;
	}

	unlock_door(ch, arg);
}

DO_FUN(do_pick, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *gch;
	OBJ_DATA *obj;
	int door;
	int chance;

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		act_char("Pick what?", ch);
		return;
	}

	if (MOUNTED(ch)) {
		  act_char("You can't pick while mounted.", ch);
		  return;
	}

	WAIT_STATE(ch, skill_beats("pick lock"));

	/* look for guards */
	for (gch = ch->in_room->people; gch; gch = gch->next_in_room) {
		if (IS_NPC(gch)
		&&  IS_AWAKE(gch)
		&&  ch->level + 5 < gch->level) {
			act("$N is standing too close to lock.",
			    ch, NULL, gch, TO_CHAR);
			return;
		}
	}

	chance = get_skill(ch, "pick lock");
	if ((obj = get_obj_here(ch, arg)) != NULL) {
		/* portal stuff */
		if (obj->pObjIndex->item_type == ITEM_PORTAL) {
			if (!IS_SET(INT(obj->value[1]), EX_ISDOOR)) {
				act_char("You can't do that.", ch);
				return;
			}

			if (!IS_SET(INT(obj->value[1]), EX_CLOSED)) {
				act_char("It's not closed.", ch);
				return;
			}

			if (INT(obj->value[4]) < 0) {
				act_char("It can't be unlocked.", ch);
				return;
			}

			if (IS_SET(INT(obj->value[1]), EX_PICKPROOF)) {
				act_char("You failed.", ch);
				return;
			}

			if (number_percent() > chance) {
				act_char("You failed.", ch);
				check_improve(ch, "pick lock", FALSE, 2);
				return;
			}

			REMOVE_BIT(INT(obj->value[1]), EX_LOCKED);
			act_puts("You pick the lock on $p.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			act("$n picks the lock on $p.", ch, obj, NULL, TO_ROOM);
			check_improve(ch, "pick lock", TRUE, 2);
			return;
		}

		/* 'pick object' */
		if (obj->pObjIndex->item_type != ITEM_CONTAINER) {
			act_char("That's not a container.", ch);
			return;
		}

		if (!IS_SET(INT(obj->value[1]), CONT_CLOSED)) {
			act_char("It's not closed.", ch);
			return;
		}

		if (INT(obj->value[2]) < 0) {
			act_char("It can't be unlocked.", ch);
			return;
		}

		if (!IS_SET(INT(obj->value[1]), CONT_LOCKED)) {
			act_char("It's already unlocked.", ch);
			return;
		}

		if (IS_SET(INT(obj->value[1]), CONT_PICKPROOF)) {
			act_char("You failed.", ch);
			return;
		}

		if (number_percent() > chance) {
			act_char("You failed.", ch);
			check_improve(ch, "pick lock", FALSE, 2);
			return;
		}

		REMOVE_BIT(INT(obj->value[1]), CONT_LOCKED);
		act_puts("You pick the lock on $p.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		act("$n picks the lock on $p.", ch, obj, NULL, TO_ROOM);
		check_improve(ch, "pick lock", TRUE, 2);
		return;
	}

	if ((door = find_door(ch, arg)) >= 0) {
		/* 'pick door' */
		ROOM_INDEX_DATA *to_room;
		EXIT_DATA *pexit;
		EXIT_DATA *pexit_rev;

		pexit = ch->in_room->exit[door];
		if (!IS_SET(pexit->exit_info, EX_CLOSED) && !IS_IMMORTAL(ch)) {
			act("It's not closed.",
			    ch, &pexit->short_descr, NULL, TO_CHAR);
			return;
		}

		if (pexit->key < 0 && !IS_IMMORTAL(ch)) {
			act("It can't be picked.",
			    ch, &pexit->short_descr, NULL, TO_CHAR);
			return;
		}

		if (!IS_SET(pexit->exit_info, EX_LOCKED)) {
			act("It's already unlocked.",
			    ch, &pexit->short_descr, NULL, TO_CHAR);
			return;
		}

		if (IS_SET(pexit->exit_info, EX_PICKPROOF)
		&&  !IS_IMMORTAL(ch)) {
			act_char("You failed.", ch);
			return;
		}

		if (number_percent() > chance) {
			act_char("You failed.", ch);
			check_improve(ch, "pick lock", FALSE, 2);
			return;
		}

		REMOVE_BIT(pexit->exit_info, EX_LOCKED);

		/*
		 * pick the other side
		 */
		if ((to_room   = pexit->to_room.r           ) != NULL
		&&   (pexit_rev = to_room->exit[rev_dir[door]]) != NULL
		&&   pexit_rev->to_room.r == ch->in_room)
			REMOVE_BIT(pexit_rev->exit_info, EX_LOCKED);

		act_char("*Click*", ch);
		act("$n picks $d.", ch, &pexit->short_descr, NULL, TO_ROOM);
		check_improve(ch, "pick lock", TRUE, 2);
	}
}

DO_FUN(do_stand, ch, argument)
{
	OBJ_DATA *obj = NULL;

	if (argument[0] != '\0') {
		if (ch->position == POS_FIGHTING) {
			act_char("Maybe you should finish fighting first?", ch);
			return;
		}

		obj = get_obj_list(ch, argument,
				   ch->in_room->contents, GETOBJ_F_ANY);
		if (obj == NULL) {
			act_char("You don't see that here.", ch);
			return;
		}

		if (obj->pObjIndex->item_type != ITEM_FURNITURE
		||  (!IS_SET(INT(obj->value[2]), STAND_AT) &&
		     !IS_SET(INT(obj->value[2]), STAND_ON) &&
		     !IS_SET(INT(obj->value[2]), STAND_IN))) {
			act_char("You can't seem to find a place to stand.", ch);
			return;
		}

		if (ch->on != obj && count_users(obj) >= INT(obj->value[0])) {
			act_puts("There's no room to stand on $p.",
				 ch, obj, NULL, TO_ROOM, POS_DEAD);
			return;
		}
	}

	switch (ch->position) {
	case POS_SLEEPING:
		if (IS_AFFECTED(ch, AFF_SLEEP)) {
			act_char("You can't wake up!", ch);
			return;
		}

		if (obj == NULL) {
			act_char("You wake and stand up.", ch);
			act("$n wakes and stands up.", ch, NULL, NULL, TO_ROOM);
			ch->on = NULL;
		} else if (IS_SET(INT(obj->value[2]), STAND_AT)) {
			act_puts("You wake and stand at $p.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			act("$n wakes and stands at $p.",
			    ch, obj, NULL, TO_ROOM);
		} else if (IS_SET(INT(obj->value[2]), STAND_ON)) {
			act_puts("You wake and stand on $p.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			act("$n wakes and stands on $p.",
			    ch, obj, NULL, TO_ROOM);
		} else {
			act_puts("You wake and stand in $p.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			act("$n wakes and stands in $p.",
			    ch, obj, NULL, TO_ROOM);
		}

		if (IS_HARA_KIRI(ch)) {
			act_char("You feel your blood heats your body.", ch);
			REMOVE_BIT(PC(ch)->plr_flags, PLR_HARA_KIRI);
		}

		ch->position = POS_STANDING;
		do_look(ch, "auto");
		break;

	case POS_RESTING:
	case POS_SITTING:
		if (obj == NULL) {
			act_char("You stand up.", ch);
			act("$n stands up.", ch, NULL, NULL, TO_ROOM);
			ch->on = NULL;
		} else if (IS_SET(INT(obj->value[2]), STAND_AT)) {
			act_puts("You stand at $p.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			act("$n stands at $p.", ch, obj, NULL, TO_ROOM);
		} else if (IS_SET(INT(obj->value[2]), STAND_ON)) {
			act_puts("You stand on $p.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			act("$n stands on $p.", ch, obj, NULL, TO_ROOM);
		} else {
			act_puts("You stand in $p.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			act("$n stands in $p.", ch, obj, NULL, TO_ROOM);
		}
		ch->position = POS_STANDING;
		break;

	case POS_STANDING:
		act_char("You are already standing.", ch);
		break;

	case POS_FIGHTING:
		act_char("You are already fighting!", ch);
		break;
	}
}

DO_FUN(do_rest, ch, argument)
{
	OBJ_DATA *obj = NULL;

	if (ch->position == POS_FIGHTING) {
		act_char("You are already fighting!", ch);
		return;
	}

	if (ch->shapeform != NULL
	&&  IS_SET(ch->shapeform->index->flags, SHAPEFORM_IMMOBILE)) {
		act_char("You cannot rest in this form.", ch);
		return;
	}

	if (MOUNTED(ch)) {
		  act_char("You can't rest while mounted.", ch);
		  return;
	}

	if (RIDDEN(ch)) {
		  act_char("You can't rest while being ridden.", ch);
		  return;
	}

	if (IS_AFFECTED(ch, AFF_SLEEP)) {
		act_char("You are already sleeping.", ch);
		return;
	}

	/* okay, now that we know we can rest, find an object to rest on */
	if (argument[0] != '\0') {
		obj = get_obj_list(ch, argument,
				   ch->in_room->contents, GETOBJ_F_ANY);
		if (obj == NULL) {
		    act_char("You don't see that here.", ch);
		    return;
		}
	}
	else obj = ch->on;

	if (obj != NULL) {
		if (obj->pObjIndex->item_type != ITEM_FURNITURE 
		||  (!IS_SET(INT(obj->value[2]), REST_ON) &&
		     !IS_SET(INT(obj->value[2]), REST_IN) &&
		     !IS_SET(INT(obj->value[2]), REST_AT))) {
		    act_char("You can't rest on that.", ch);
		    return;
		}

		if (obj != NULL && ch->on != obj
		&&  count_users(obj) >= INT(obj->value[0])) {
			act_puts("There's no more room on $p.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			return;
		}

		ch->on = obj;
	}

	switch (ch->position) {
	case POS_SLEEPING:
		if (obj == NULL) {
		    act_char("You wake up and start resting.", ch);
		    act("$n wakes up and starts resting.",
			ch, NULL, NULL, TO_ROOM);
		} else if (IS_SET(INT(obj->value[2]),REST_AT)) {
			act_puts("You wake up and rest at $p.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			act("$n wakes up and rests at $p.",
			    ch, obj, NULL, TO_ROOM);
		} else if (IS_SET(INT(obj->value[2]),REST_ON)) {
			act_puts("You wake up and rest on $p.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			act("$n wakes up and rests on $p.",
			    ch, obj, NULL, TO_ROOM);
		} else {
			act_puts("You wake up and rest in $p.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			act("$n wakes up and rests in $p.",
			    ch, obj, NULL, TO_ROOM);
		}
		ch->position = POS_RESTING;
		break;

	case POS_RESTING:
		act_char("You are already resting.", ch);
		break;

	case POS_STANDING:
		if (obj == NULL) {
			act_char("You rest.", ch);
			act("$n sits down and rests.",
			    ch, NULL, NULL, TO_ROOM);
		} else if (IS_SET(INT(obj->value[2]), REST_AT)) {
			act_puts("You sit down at $p and rest.",
			    ch, obj, NULL, TO_CHAR, POS_DEAD);
			act("$n sits down at $p and rests.",
			    ch, obj, NULL, TO_ROOM);
		} else if (IS_SET(INT(obj->value[2]), REST_ON)) {
			act_puts("You sit down on $p and rest.",
			    ch, obj, NULL, TO_CHAR, POS_DEAD);
			act("$n sits down on $p and rests.",
			    ch, obj, NULL, TO_ROOM);
		} else {
			act_puts("You sit down in $p and rest.",
			    ch, obj, NULL, TO_CHAR, POS_DEAD);
			act("$n sits down in $p and rests.",
			    ch, obj, NULL, TO_ROOM);
		}
		ch->position = POS_RESTING;
		break;

	case POS_SITTING:
		if (obj == NULL) {
			act_char("You rest.", ch);
			act("$n rests.", ch, NULL, NULL, TO_ROOM);
		} else if (IS_SET(INT(obj->value[2]),REST_AT)) {
			act_puts("You rest at $p.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			act("$n rests at $p.", ch, obj, NULL, TO_ROOM);
		} else if (IS_SET(INT(obj->value[2]),REST_ON)) {
			act_puts("You rest on $p.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			act("$n rests on $p.", ch, obj, NULL, TO_ROOM);
		} else {
			act_puts("You rest in $p.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			act("$n rests in $p.", ch, obj, NULL, TO_ROOM);
		}
		ch->position = POS_RESTING;

		if (IS_HARA_KIRI(ch)) {
			act_char("You feel your blood heats your body.", ch);
			REMOVE_BIT(PC(ch)->plr_flags, PLR_HARA_KIRI);
		}
		break;
	}
}

DO_FUN(do_sit, ch, argument)
{
	OBJ_DATA *obj = NULL;

	if (ch->position == POS_FIGHTING) {
		act_char("Maybe you should finish fighting first?", ch);
		return;
	}

	if (MOUNTED(ch)) {
		  act_char("You can't sit while mounted.", ch);
		  return;
	}

	if (RIDDEN(ch)) {
		  act_char("You can't sit while being ridden.", ch);
		  return;
	}

	if (IS_AFFECTED(ch, AFF_SLEEP)) {
		act_char("You are already sleeping.", ch);
		return;
	}

	/* okay, now that we know we can sit, find an object to sit on */
	if (argument[0] != '\0') {
		obj = get_obj_list(ch, argument,
				   ch->in_room->contents, GETOBJ_F_ANY);
		if (obj == NULL) {
			act_char("You don't see that here.", ch);
			return;
		}
	} else
		obj = ch->on;

	if (obj != NULL) {
		if (obj->pObjIndex->item_type != ITEM_FURNITURE
		||  (!IS_SET(INT(obj->value[2]), SIT_ON) &&
		     !IS_SET(INT(obj->value[2]), SIT_IN) &&
		     !IS_SET(INT(obj->value[2]), SIT_AT))) {
			act_char("You can't sit on that.", ch);
			return;
		}

		if (obj != NULL
		&&  ch->on != obj
		&&  count_users(obj) >= INT(obj->value[0])) {
			act_puts("There's no more room on $p.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			return;
		}

		ch->on = obj;
	}

	switch (ch->position) {
	case POS_SLEEPING:
		if (obj == NULL) {
			act_char("You wake and sit up.", ch);
			act("$n wakes and sits up.", ch, NULL, NULL, TO_ROOM);
		} else if (IS_SET(INT(obj->value[2]),SIT_AT)) {
			act_puts("You wake and sit at $p.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			act("$n wakes and sits at $p.", ch, obj, NULL, TO_ROOM);
		} else if (IS_SET(INT(obj->value[2]),SIT_ON)) {
			act_puts("You wake and sit on $p.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			act("$n wakes and sits on $p.", ch, obj, NULL, TO_ROOM);
		} else {
			act_puts("You wake and sit in $p.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			act("$n wakes and sits in $p.", ch, obj, NULL, TO_ROOM);
		}

		ch->position = POS_SITTING;
		break;

	case POS_RESTING:
		if (obj == NULL)
			act_char("You stop resting.", ch);
		else if (IS_SET(INT(obj->value[2]),SIT_AT)) {
			act_puts("You sit at $p.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			act("$n sits at $p.", ch, obj, NULL, TO_ROOM);
		} else if (IS_SET(INT(obj->value[2]),SIT_ON)) {
			act_puts("You sit on $p.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			act("$n sits on $p.", ch, obj, NULL, TO_ROOM);
		} else {
			act_puts("You sit in $p.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			act("$n sits in $p.", ch, obj, NULL, TO_ROOM);
		}
		ch->position = POS_SITTING;
		break;

	case POS_SITTING:
		act_char("You are already sitting down.", ch);
		break;

	case POS_STANDING:
		if (obj == NULL) {
			act_char("You sit down.", ch);
			act("$n sits down on the ground.",
			    ch, NULL, NULL, TO_ROOM);
		} else if (IS_SET(INT(obj->value[2]), SIT_AT)) {
			act_puts("You sit down at $p.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			act("$n sits down at $p.", ch, obj, NULL, TO_ROOM);
		} else if (IS_SET(INT(obj->value[2]),SIT_ON)) {
			act_puts("You sit down on $p.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			act("$n sits down on $p.", ch, obj, NULL, TO_ROOM);
		} else {
			act_puts("You sit down in $p.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			act("$n sits down in $p.", ch, obj, NULL, TO_ROOM);
		}
		ch->position = POS_SITTING;
		break;
	}

	if (IS_HARA_KIRI(ch)) {
		 act_char("You feel your blood heats your body.", ch);
		 REMOVE_BIT(PC(ch)->plr_flags, PLR_HARA_KIRI);
	}
}

DO_FUN(do_sleep, ch, argument)
{
	OBJ_DATA *obj = NULL;

	if (MOUNTED(ch)) {
		act_char("You can't sleep while mounted.", ch);
		return;
	}

	if (ch->shapeform != NULL
	&&  IS_SET(ch->shapeform->index->flags, SHAPEFORM_IMMOBILE)) {
		act_char("You cannot sleep in this form.", ch);
		return;
	}

	if (RIDDEN(ch)) {
		act_char("You can't sleep while being ridden.", ch);
		return;
	}

	switch (ch->position) {
	case POS_SLEEPING:
		act_char("You are already sleeping.", ch);
		break;

	case POS_RESTING:
	case POS_SITTING:
	case POS_STANDING:
		if (argument[0] == '\0' && ch->on == NULL) {
			act_char("You go to sleep.", ch);
			act("$n goes to sleep.", ch, NULL, NULL, TO_ROOM);
		}
		else { /* find an object and sleep on it */
			if (argument[0] == '\0')
				obj = ch->on;
			else
				obj = get_obj_list(ch, argument,
						   ch->in_room->contents,
						   GETOBJ_F_ANY);

			if (obj == NULL) {
				act_char("You don't see that here.", ch);
				return;
			}

			if (obj->pObjIndex->item_type != ITEM_FURNITURE
			||  (!IS_SET(INT(obj->value[2]), SLEEP_ON) &&
			     !IS_SET(INT(obj->value[2]), SLEEP_IN) &&
			     !IS_SET(INT(obj->value[2]), SLEEP_AT))) {
				act_char("You can't sleep on that.", ch);
				return;
			}

			if (ch->on != obj
			&&  count_users(obj) >= INT(obj->value[0])) {
				act_puts("There's no room on $p for you.",
					 ch, obj, NULL, TO_CHAR, POS_DEAD);
				return;
			}

			ch->on = obj;
			if (IS_SET(INT(obj->value[2]), SLEEP_AT)) {
				act_puts("You go to sleep at $p.",
					 ch, obj, NULL, TO_CHAR, POS_DEAD);
				act("$n goes to sleep at $p.",
				    ch, obj, NULL, TO_ROOM);
			} else if (IS_SET(INT(obj->value[2]), SLEEP_ON)) {
				act_puts("You go to sleep on $p.",
					 ch, obj, NULL, TO_CHAR, POS_DEAD);
				act("$n goes to sleep on $p.",
				    ch, obj, NULL, TO_ROOM);
			} else {
				act_puts("You go to sleep in $p.",
					 ch, obj, NULL, TO_CHAR, POS_DEAD);
				act("$n goes to sleep in $p.",
				    ch, obj, NULL, TO_ROOM);
			}
		}
		ch->position = POS_SLEEPING;
		break;

	case POS_FIGHTING:
		act_char("You are already fighting.", ch);
		break;
	}
}

DO_FUN(do_wake, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		do_stand(ch, argument);
		return;
	}

	if (!IS_AWAKE(ch)) {
		act_char("You are asleep yourself!", ch);
		return;
	}

	if ((victim = get_char_here(ch, arg)) == NULL) {
		act_char("They aren't here.", ch);
		return;
	}

	if (IS_AWAKE(victim)) {
		act_puts("$N is already awake.",
			 ch, NULL, victim, TO_CHAR, POS_DEAD);
		return;
	}

	if (IS_AFFECTED(victim, AFF_SLEEP)) {
		act_puts("You can't wake $M!",
			 victim, NULL, ch, TO_VICT, POS_DEAD);
		return;
	}

	act_puts("$n wakes you.", ch, NULL, victim, TO_VICT, POS_SLEEPING);
	do_stand(victim, str_empty);
}

DO_FUN(do_sneak, ch, argument)
{
	AFFECT_DATA *paf;

	if (MOUNTED(ch)) {
		act_char("You can't sneak while mounted.", ch);
		return;
	}

	if (HAS_INVIS(ch, ID_SNEAK)) {
		if (is_sn_affected(ch, "stealth")) {
			affect_strip(ch, "stealth");
			act_char("You trample around loudly again.", ch);
			return;
		} else {
			act_puts("You can't move less silently.",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
			return;
		}
	}

	act_puts("You attempt to move silently.",
		 ch, NULL, NULL, TO_CHAR, POS_DEAD);

	paf = aff_new(TO_INVIS, "stealth");
	paf->level     = LEVEL(ch);
	paf->duration  = -1;
	paf->bitvector = ID_SNEAK;
	affect_to_char(ch, paf);
	aff_free(paf);
}

DO_FUN(do_hide, ch, argument)
{
	int chance;
	flag_t sector;

	if (MOUNTED(ch)) {
		  act_char("You can't hide while mounted.", ch);
		  return;
	}

	if (RIDDEN(ch)) {
		  act_char("You can't hide while being ridden.", ch);
		  return;
	}

	if (IS_AFFECTED(ch, AFF_FAERIE_FIRE) )  {
		act_char("You cannot hide while glowing.", ch);
		return;
	}

	act_char("You attempt to hide.", ch);

	if ((chance = get_skill(ch, "hide")) == 0)
		return;

	sector = ch->in_room->sector_type;
	if (sector == SECT_FOREST
	||  sector == SECT_HILLS
	||  sector == SECT_SWAMP
	||  sector == SECT_MOUNTAIN)
		chance -= 15;
	else if (sector == SECT_CITY)
		chance += 15;

	if (number_percent() < chance) {
		SET_INVIS(ch, ID_HIDDEN);
		check_improve(ch, "hide", TRUE, 3);
	} else  {
		REMOVE_INVIS(ch, ID_HIDDEN);
		check_improve(ch, "hide", FALSE, 3);
	}
}

DO_FUN(do_camouflage, ch, argument)
{
	flag_t sector;

	if (MOUNTED(ch)) {
		act_char("You can't camouflage while mounted.", ch);
		return;
	}

	if (RIDDEN(ch)) {
		act_char("You can't camouflage while being ridden.", ch);
		return;
	}

	if (IS_AFFECTED(ch, AFF_FAERIE_FIRE))  {
		act_char("You can't camouflage yourself while glowing.", ch);
		return;
	}

	sector = ch->in_room->sector_type;
	if (sector != SECT_FOREST
	&&  sector != SECT_HILLS
	&&  sector != SECT_MOUNTAIN) {
		act_char("There's no cover here.", ch);
		act("$n tries to camouflage $mself against the lone leaf on the ground.",
		    ch, NULL, NULL, TO_ROOM);
		return;
	}

	act_char("You attempt to camouflage yourself.", ch);
	WAIT_STATE(ch, skill_beats("camouflage"));

	if (HAS_INVIS(ch, ID_CAMOUFLAGE))
		REMOVE_INVIS(ch, ID_CAMOUFLAGE);

	if (IS_NPC(ch) || number_percent() < get_skill(ch, "camouflage")) {
		SET_INVIS(ch, ID_CAMOUFLAGE);
		check_improve(ch, "camouflage", TRUE, 1);
	} else
		check_improve(ch, "camouflage", FALSE, 1);
}

DO_FUN(do_blend, ch, argument)
{
	flag_t sector;

	if (IS_AFFECTED(ch, AFF_FAERIE_FIRE)) {
		act_puts("You can't blend while glowing.",
			ch, NULL, NULL, TO_CHAR, POS_DEAD);
		return;
	}

	if (HAS_INVIS(ch, ID_BLEND)) {
		act_puts("You are already blending.",
			ch, NULL, NULL, TO_CHAR, POS_DEAD);
		return;
	}

	sector = ch->in_room->sector_type;
	if (sector != SECT_FOREST) {
		act_puts("You are not in the forest.",
			ch, NULL, NULL, TO_CHAR, POS_DEAD);
		return;
	}

	act_puts("You attempt to blend in the forest.",
		ch, NULL, NULL, TO_CHAR, POS_DEAD);
	act_puts("$n attempts to blend in the forest.",
		ch, NULL, NULL, TO_ROOM, POS_RESTING);
	if (number_percent() < get_skill(ch, "forest blending")) {
		AFFECT_DATA *paf;

		paf = aff_new(TO_INVIS, "forest blending");
		paf->level	= LEVEL(ch);
		paf->bitvector	= ID_BLEND;
		affect_to_char(ch, paf);
		aff_free(paf);

		check_improve(ch, "forest blending", TRUE, 2);
	} else
		check_improve(ch, "forest blending", FALSE, 2);
}

DO_FUN(do_acute, ch, argument)
{
	AFFECT_DATA *paf;
	int mana;

	if (HAS_DETECT(ch, ID_CAMOUFLAGE)) {
		act_char("Your vision is already acute.", ch);
		return;
	}

	mana = skill_mana(ch, "acute vision");
	if (ch->mana < mana) {
		act_char("You don't have enough mana.", ch);
		return;
	}
	ch->mana -= mana;
	WAIT_STATE(ch, skill_beats("acute vision"));

	if (number_percent() > get_skill(ch, "acute vision")) {
		act_char("You failed to sharpen your vision.", ch);
		check_improve(ch, "acute vision", FALSE, 3);
		return;
	}

	paf = aff_new(TO_DETECTS, "acute vision");
	paf->level	= LEVEL(ch);
	paf->duration	= 3 + LEVEL(ch) / 5;
	paf->bitvector	= ID_CAMOUFLAGE;
	affect_to_char(ch, paf);
	aff_free(paf);

	act_char("Your vision sharpens.", ch);
	check_improve(ch, "acute vision", TRUE, 3);
}

/*
 * Contributed by Alander
 */

DO_FUN(do_visible, ch, argument)
{
	make_visible(ch, TRUE);
}

DO_FUN(do_recall, ch, argument)
{
	ROOM_INDEX_DATA *location;
	CHAR_DATA *pet;

	if (IS_NPC(ch)) {
		act_char("Only players can recall.", ch);
		return;
	}

	if (ch->level >= 10 && !IS_IMMORTAL(ch)) {
		act_char("Recall is for only levels below 10.", ch);
		return;
	}

	if (ch->desc) {
		if (IS_PUMPED(ch)) {
			act_puts("You are too pumped to pray now.",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
			return;
		}
		location = get_recall(ch);
	} else
		location = get_random_recall();

	act("$n prays for transportation!", ch, NULL, NULL, TO_ROOM);

	if (ch->in_room == location)
		return;

	if (IS_SET(ch->in_room->room_flags, ROOM_NORECALL)
	||  IS_AFFECTED(ch, AFF_CURSE)
	||  IS_AFFECTED(ch->in_room, RAFF_CURSE)) {
		act_char("The gods have forsaken you.", ch);
		return;
	}

	pet = GET_PET(ch);
	ch->move /= 2;
	recall(ch, location);

	if (pet && !IS_AFFECTED(pet, AFF_SLEEP)) {
		if (pet->position != POS_STANDING)
			do_stand(pet, str_empty);
		recall(pet, location);
	}
}

DO_FUN(do_train, ch, argument)
{
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *mob;
	int stat;
	PC_DATA *pc;

	if (IS_NPC(ch))
		return;

	/*
	 * Check for trainer.
	 */
	for (mob = ch->in_room->people; mob; mob = mob->next_in_room) {
		if (IS_NPC(mob) && MOB_IS(mob, MOB_TRAIN | MOB_GAIN))
			break;
	}

	if (mob == NULL) {
		act_char("You can't do that here.", ch);
		return;
	}

	pc = PC(ch);
	stat = flag_svalue(stat_aliases, argument);
	if (stat < 0) {
		act_puts("You have $j training $qj{sessions}.",
			 ch, (const void *) pc->train, NULL,
			 TO_CHAR, POS_DEAD);
		snprintf(buf, sizeof(buf),
			 GETMSG("You can train:%s%s%s%s%s%s", GET_LANG(ch)),
			 ch->perm_stat[STAT_STR] < get_max_train(ch, STAT_STR) ?
				" str" : str_empty,		// notrans
			 ch->perm_stat[STAT_INT] < get_max_train(ch, STAT_INT) ?
				" int" : str_empty,		// notrans
			 ch->perm_stat[STAT_WIS] < get_max_train(ch, STAT_WIS) ?
				" wis" : str_empty,		// notrans
			 ch->perm_stat[STAT_DEX] < get_max_train(ch, STAT_DEX) ?
				" dex" : str_empty,		// notrans
			 ch->perm_stat[STAT_CON] < get_max_train(ch, STAT_CON) ?
				" con" : str_empty,		// notrans
			 ch->perm_stat[STAT_CHA] < get_max_train(ch, STAT_CHA) ?
				" cha" : str_empty);		// notrans

		if (buf[strlen(buf)-1] != ':') {
			act_puts("$t.", ch, buf, NULL,		// notrans
				 TO_CHAR | ACT_NOTRANS, POS_DEAD);
		} else {
			act("You have nothing left to train!",
			    ch, NULL, NULL, TO_CHAR);
		}
		return;
	}

	if (ch->perm_stat[stat] >= get_max_train(ch, stat)) {
		act_puts("Your $w is already at maximum.",
			 ch, flag_string(stat_names, stat), NULL,
			 TO_CHAR, POS_DEAD);
		return;
	}

	if (pc->train < 1) {
		act_char("You don't have enough training sessions.", ch);
		return;
	}

	pc->train--;
	ch->perm_stat[stat] += 1;
	act_puts("Your $w increases!",
		 ch, flag_string(stat_names, stat), NULL, TO_CHAR, POS_DEAD);
	act("$n's $w increases!",
	    ch, flag_string(stat_names, stat), NULL, TO_ROOM);
}

DO_FUN(do_track, ch, argument)
{
	ROOM_HISTORY_DATA *rh;
	EXIT_DATA *pexit;
	int d;

	WAIT_STATE(ch, skill_beats("track"));
	act("$n checks the ground for tracks.", ch, NULL, NULL, TO_ROOM);

	if (number_percent() < get_skill(ch, "track")) {
		/* success */
		for (rh = ch->in_room->history; rh != NULL; rh = rh->next)
			if (is_name(argument, rh->name)) {
				check_improve(ch, "track", TRUE, 1);
			if ((d = rh->went) == -1
			||  d >= MAX_DIR)
				continue;
			act_puts("$t's tracks lead $T.",
				 ch, rh->name, dir_name[d], TO_CHAR, POS_DEAD);
			if ((pexit = ch->in_room->exit[d]) != NULL
			&&  IS_SET(pexit->exit_info, EX_ISDOOR)
			&&  pexit->keyword != NULL)
				dofun("open", ch, dir_name[d]);
			move_char(ch, rh->went, 0);
			return;
		}
	}

	act_char("You don't see any tracks.", ch);
	check_improve(ch, "track", FALSE, 1);
}

DO_FUN(do_vampire, ch, argument)
{
	AFFECT_DATA *paf;

	if (is_sn_affected(ch, "vampire")) {
		act_char("But you are already vampire. Kill them! Kill them!", ch);
		return;
	}

	if (weather_info.sunlight == SUN_LIGHT
	||  weather_info.sunlight == SUN_RISE) {
		act_char("You should wait for the evening or night to transform to a vampire.", ch);
		return;
	}

	act_char("You feel yourself getting greater and greater.", ch);
	act("You cannot recognize $n anymore.", ch, NULL, NULL, TO_ROOM);

	paf = aff_new(TO_RESISTS, "vampire");
	paf->level     = LEVEL(ch);
	paf->duration  = LEVEL(ch) / 10 + 5;

/* negative immunity */
	INT(paf->location) = DAM_NEGATIVE;
	paf->modifier = 100;
	affect_to_char(ch, paf);

/* haste */
	paf->where     = TO_AFFECTS;
	INT(paf->location) = APPLY_DEX;
	paf->modifier  = 1 + (LEVEL(ch) /20);
	paf->bitvector = AFF_HASTE | AFF_BERSERK | AFF_FLYING | AFF_TURNED;
	affect_to_char(ch, paf);

/* giant strength + infrared */
	INT(paf->location) = APPLY_STR;
	paf->modifier  = 1 + (LEVEL(ch) / 20);
	paf->bitvector = 0;
	affect_to_char(ch, paf);

/* size */
	INT(paf->location) = APPLY_SIZE;
	paf->modifier  = 1 + (LEVEL(ch) / 50);
	affect_to_char(ch, paf);

/* damroll */
	INT(paf->location) = APPLY_DAMROLL;
	paf->modifier  = ch->damroll;
	affect_to_char(ch, paf);

/* infrared */
	paf->where     = TO_DETECTS;
	INT(paf->location) = 0;
	paf->modifier  = 0;
	paf->bitvector = ID_INFRARED;
	affect_to_char(ch, paf);

/* sneak */
	paf->where = TO_INVIS;
	paf->bitvector = ID_SNEAK;
	affect_to_char(ch, paf);
	aff_free(paf);

	free_string(PC(ch)->form_name);
	PC(ch)->form_name = str_dup("an ugly creature");
}

DO_FUN(do_vbite, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	if (!is_sn_affected(ch, "vampire")) {
		act_char("You must transform vampire before biting.", ch);
		return;
	}

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		act_char("Bite whom?", ch);
		return;
	}

	if ((victim = get_char_here(ch, arg)) == NULL) {
		WAIT_STATE(ch, MISSING_TARGET_DELAY);
		act_char("They aren't here.", ch);
		return;
	}

	if (victim->position != POS_SLEEPING) {
		act_char("They must be sleeping.", ch);
		return;
	}

	if (IS_NPC(ch) && !IS_NPC(victim))
		return;

	if (victim == ch) {
		act_char("How can you sneak upon yourself?", ch);
		return;
	}

	if (victim->fighting != NULL) {
		act_char("You can't bite a fighting person.", ch);
		return;
	}

	if (is_safe(ch, victim))
		return;

	if (victim->hit < (2 * victim->max_hit / 10) ) {
		act_puts("$N is hurt and suspicious ... doesn't worth up.",
			 ch, NULL, victim, TO_CHAR, POS_DEAD);
		return;
	}

	WAIT_STATE(ch, skill_beats("vampiric bite"));

	if (!IS_AWAKE(victim)
	&&  (IS_NPC(ch) ||
	     number_percent() < get_skill(ch, "vampiric bite") * 7 / 10 +
		2 * (LEVEL(ch) - LEVEL(victim)))) {
		one_hit(ch, victim, "vampiric bite", WEAR_WIELD);
		if (LEVEL(victim) > LEVEL(ch)
		&&  number_percent() < get_skill(ch, "resurrection") / 10 *
					(LEVEL(victim) - LEVEL(ch))) {
			AFFECT_DATA *paf;

			paf = aff_new(TO_AFFECTS, "resurrection");
			paf->level	= LEVEL(ch);
			paf->duration	= number_fuzzy(4);
			affect_join(ch, paf);
			aff_free(paf);

			act_char("You gain power of undead!", ch);
			check_improve(ch, "resurrection", TRUE, 1);
		}
	} else {
		check_improve(ch, "vampiric bite", FALSE, 1);
		damage(ch, victim, 0, "vampiric bite", DAM_F_SHOW);
	}
	if (!IS_NPC(victim) && victim->position == POS_FIGHTING)
		yell(victim, ch, "Help! $lu{$N} tried to bite me!");
}

DO_FUN(do_bash_door, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *gch;
	int chance;
	int damage_bash,door;
	int beats;

	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	if (MOUNTED(ch)) {
		act_char("You can't bash doors while mounted.", ch);
		return;
	}

	if (RIDDEN(ch)) {
		act_char("You can't bash doors while being ridden.", ch);
		return;
	}

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		act_char("Bash which door or direction?", ch);
		return;
	}

	if (ch->fighting) {
		act_char("Wait until the fight finishes.", ch);
		return;
	}

	/* look for guards */
	for (gch = ch->in_room->people; gch; gch = gch->next_in_room)
		if (IS_NPC(gch)
		&&  IS_AWAKE(gch) && ch->level + 5 < gch->level) {
			act_puts("$N is standing too close to door.",
				 ch, NULL, gch, TO_CHAR, POS_DEAD);
			return;
		}

	if ((door = find_door(ch, arg)) < 0)
		return;

	pexit = ch->in_room->exit[door];

	if (!IS_SET(pexit->exit_info, EX_CLOSED)) {
		act_char("It's already open.", ch);
		return;
	}

	if (!IS_SET(pexit->exit_info, EX_LOCKED)) {
		act_char("Just try to open it.", ch);
		return;
	}

	if (IS_SET(pexit->exit_info, EX_NOPASS)) {
		act_char("A mystical shield protects exit.", ch);
		return;
	}

	chance = get_skill(ch, "bash door");
	chance -= 90;

	/* modifiers */

	/* size and weight */
	chance += get_carry_weight(ch) / 100;
	chance += (ch->size - 2) * 20;

	/* stats */
	chance += get_curr_stat(ch, STAT_STR);

	if (IS_AFFECTED(ch,AFF_FLYING))
		chance -= 10;

	act_puts("You slam into $d, and try to break it!",
		 ch, &pexit->short_descr, NULL, TO_CHAR, POS_DEAD);
	act("$n slams into $d, and tries to break it!",
	    ch, &pexit->short_descr, NULL, TO_ROOM);

	if (char_in_dark_room(ch))
		chance /= 2;

	beats = skill_beats("bash door");
	/* now the attack */
	if (number_percent() < chance) {
		check_improve(ch, "bash door", TRUE, 1);

		REMOVE_BIT(pexit->exit_info, EX_LOCKED);
		REMOVE_BIT(pexit->exit_info, EX_CLOSED);
		act("$n bashes $d and breaks the lock.",
		    ch, &pexit->short_descr, NULL, TO_ROOM);
		act_puts("You successed to open $d.",
			 ch, &pexit->short_descr, NULL, TO_CHAR, POS_DEAD);

/* open the other side */
		if ((to_room = pexit->to_room.r) != NULL
		&&  (pexit_rev = to_room->exit[rev_dir[door]]) != NULL
		&&  pexit_rev->to_room.r == ch->in_room) {
			ROOM_INDEX_DATA *in_room;

			REMOVE_BIT(pexit_rev->exit_info, EX_CLOSED);
			REMOVE_BIT(pexit_rev->exit_info, EX_LOCKED);

			in_room = ch->in_room;
			ch->in_room = to_room;
			act("$n bashes $d and breaks the lock.",
			    ch, &pexit->short_descr, NULL, TO_ROOM);
			ch->in_room = in_room;
		}

		check_improve(ch, "bash door", TRUE, 1);
		WAIT_STATE(ch, beats);
	} else {
		act_puts("You fall flat on your face!",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		act_puts("$n falls flat on $s face.",
			 ch, NULL, NULL, TO_ROOM, POS_RESTING);
		check_improve(ch, "bash door", FALSE, 1);
		ch->position = POS_RESTING;
		WAIT_STATE(ch, beats * 3 / 2);
		damage_bash = ch->damroll +
			      number_range(4,4 + 4* ch->size + chance/5);
		damage(ch, ch, damage_bash, "bash door", DAM_F_SHOW);
	}
}

DO_FUN(do_blink, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] =='\0') {
		if (is_sn_affected(ch, "blink")) {
			act("You current blink status is now: ON.",
			    ch, NULL, NULL, TO_CHAR);
			return;
		} else {
			act("You current blink status is now: OFF.",
			    ch, NULL, NULL, TO_CHAR);
			return;
		}
	} else if (!str_cmp(arg,"on")) {
		if (is_sn_affected(ch, "blink")) {
			act("You are already blinking.",
			    ch, NULL, NULL, TO_CHAR);
			return;
		} else {
			AFFECT_DATA *paf;

			act("You start blinking.", ch, NULL, NULL, TO_CHAR);
			act("$n starts blinking.", ch, NULL, NULL, TO_ROOM);

			paf = aff_new(TO_AFFECTS, "blink");
			paf->level	= LEVEL(ch);
			paf->duration	= -1;
			affect_to_char(ch, paf);
			aff_free(paf);
			return;
		}
	} else if (!str_cmp(arg,"off")) {
		if (is_sn_affected(ch, "blink")) {
			act("You stop blinking.", ch, NULL, NULL, TO_CHAR);
			act("$n stops blinking.", ch, NULL, NULL, TO_ROOM);
			affect_strip(ch, "blink");
			return;
		} else {
			act("You are not blinking.", ch, NULL, NULL, TO_CHAR);
			return;
		}
	} else
		do_help(ch, "blink");
}

DO_FUN(do_vanish, ch, argument)
{
	int min_mana;

	if (ch->mana < (min_mana = skill_mana(ch, "vanish"))) {
		act_char("You don't have enough power.", ch);
		return;
	}
	ch->mana -= min_mana;
	WAIT_STATE(ch, skill_beats("vanish"));

	if (number_percent() > get_skill(ch, "vanish")) {
		act_char("You failed.", ch);
		check_improve(ch, "vanish", FALSE, 1);
		return;
	}

	if (IS_SET(ch->in_room->room_flags,
		   ROOM_NORECALL | ROOM_PEACE | ROOM_SAFE)) {
		act_char("You failed.", ch);
		return;
	}

	act("$n throws down a small globe.", ch, NULL, NULL, TO_ROOM);
	check_improve(ch, "vanish", TRUE, 1);

	if (!IS_NPC(ch) && ch->fighting && number_bits(1) == 1) {
		act_char("You failed.", ch);
		return;
	}

	stop_fighting(ch, TRUE);
	teleport_char(ch, NULL, get_random_room(ch, ch->in_room->area),
		      "$n is gone!", NULL, "$n appears from nowhere.");
}

DO_FUN(do_kidnap, ch, argument)
{
	CHAR_DATA * victim;
	char arg[MAX_INPUT_LENGTH];
	ROOM_INDEX_DATA* to_room;
	AFFECT_DATA *paf;
	int mana;
	int chance;

	if (is_sn_affected(ch, "kidnap")) {
		act("You feel too exhausted from previous kidnap attempt.",
		    ch, NULL, NULL, TO_CHAR);
		return;
	}

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] =='\0') {
		act("Kidnap whom?", ch, NULL, NULL, TO_CHAR);
		return;
	}

	if ((victim = get_char_here(ch, arg)) == NULL) {
		WAIT_STATE(ch, MISSING_TARGET_DELAY);
		act_char("They aren't here.", ch);
		return;
	}

	if (victim == ch) {
		act_char("Very funny.", ch);
		return;
	}

	if (victim->position == POS_FIGHTING) {
		act_char("You'd better not -- you might get hit.", ch);
		return;
	}

	if (is_safe(ch, victim))
		return;

	chance = get_skill(ch, "kidnap");
	chance -= get_curr_stat(victim, STAT_WIS) +
		  get_curr_stat(victim, STAT_DEX) -
		  get_curr_stat(ch, STAT_DEX);

	if (ch->mana < (mana = skill_mana(ch, "kidnap"))) {
		act_char("You don't have enough power.", ch);
		return;
	}
	ch->mana -= mana;

	paf = aff_new(TO_AFFECTS, "kidnap");
	paf->level	= ch->level;
	paf->duration	= 3;
	affect_to_char(ch, paf);
	aff_free(paf);

	if (IS_SET(ch->in_room->room_flags, ROOM_PEACE | ROOM_SAFE))
		chance = 0;

	if (IS_SET(ch->in_room->room_flags, ROOM_LAW))
		chance /= 2;

	WAIT_STATE(ch, skill_beats("kidnap"));

	if (number_percent() < chance) {
		to_room = get_random_room(ch, NULL);
		act("You grab $N and take $m away.",
		    ch, NULL, victim, TO_CHAR);
		act("$n grabs you and takes you away.",
		    ch, NULL, victim, TO_VICT);
		act("$n grabs $N and takes $m away.",
		    ch, NULL, victim, TO_NOTVICT);
		teleport_char(ch, NULL, to_room,
		    "$n disappears.", NULL, "$n appears from nowhere.");
		teleport_char(victim, NULL, to_room,
		    "$n disappears.", NULL, "$n appears from nowhere.");
		check_improve(ch, "kidnap", TRUE, 1);
		yell(victim, ch, "Help! $lu{$N} just kidnapped me!");
		multi_hit(victim, ch, NULL);
	} else {
		act("You grab $N, but $E escaped.",
			ch, NULL, victim, TO_CHAR);
		act("$n grabs you, but you manage to break free.",
			ch, NULL, victim, TO_VICT);
		act("$n grabs $N, but $E escaped.",
			ch, NULL, victim, TO_NOTVICT);
		yell(victim, ch, "Help! $lu{$N} tried to kidnap me!");
		check_improve(ch, "kidnap", FALSE, 1);
		multi_hit(victim, ch, NULL);
	}
}

DO_FUN(do_fade, ch, argument)
{
	if (MOUNTED(ch)) {
		  act_char("You can't fade while mounted.", ch);
		  return;
	}

	if (RIDDEN(ch)) {
		  act_char("You can't fade while being ridden.", ch);
		  return;
	}

	act_char("You attempt to fade.", ch);
	if (number_percent() < get_skill(ch, "fade")) {
		SET_INVIS(ch, ID_FADE);
		check_improve(ch, "fade", TRUE, 3);
	} else
		check_improve(ch, "fade", FALSE, 3);
}

DO_FUN(do_vtouch, ch, argument)
{
	CHAR_DATA *victim;
	int chance;

	if (!is_sn_affected(ch, "vampire")) {
		act_char("Let it be.", ch);
		return;
	}

	if (IS_AFFECTED(ch, AFF_CHARM))  {
		act_char("You don't want to drain your master.", ch);
		return;
	}

	if ((victim = get_char_here(ch,argument)) == NULL) {
		WAIT_STATE(ch, MISSING_TARGET_DELAY);
		act_char("They aren't here.", ch);
		return;
	}

	if (ch == victim) {
		act_char("Even you are not so stupid.", ch);
		return;
	}

	if (is_sn_affected(victim, "vampiric touch"))
		return;

	if (is_safe(ch,victim))
		return;

	if (is_sn_affected(victim, "free action"))
		chance -= 20;

	WAIT_STATE(ch, skill_beats("vampiric touch"));

	SET_FIGHT_TIME(victim);
	SET_FIGHT_TIME(ch);

	if (number_percent() < get_skill(ch, "vampiric touch") * 85 / 100
	&&  !IS_CLAN_GUARD(victim)
	&&  !IS_IMMORTAL(victim)) {
		AFFECT_DATA *paf;

		act_puts("You deadly touch $n's neck and put $m to nightmares.",
			 victim, NULL, ch, TO_VICT, POS_DEAD);
		act_puts("$N deadly touches your neck and puts you "
			 "to nightmares.", victim, NULL, ch, TO_CHAR, POS_DEAD);
		act("$N deadly touches $n's neck and puts $m to nightmares.",
		    victim, NULL, ch, TO_NOTVICT);

		check_improve(ch, "vampiric touch", TRUE, 1);

		paf = aff_new(TO_AFFECTS, "vampiric touch");
		paf->level = ch->level;
		paf->duration = LEVEL(ch) / 20 + 1;
		paf->bitvector = AFF_SLEEP;
		affect_join(victim, paf);
		aff_free(paf);

		if (IS_AWAKE(victim))
			victim->position = POS_SLEEPING;
	} else {
		damage(ch, victim, 0, "vampiric touch", DAM_F_SHOW);
		check_improve(ch, "vampiric touch", FALSE, 1);
	}
	yell(victim, ch, "Help! $lu{$N} tried to touch me!");
}

DO_FUN(do_fly, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];

	if (IS_NPC(ch))
		return;

	argument = one_argument(argument, arg, sizeof(arg));

	if (!str_cmp(arg, "up")) {
		race_t *r;

		if (IS_AFFECTED(ch, AFF_FLYING)) {
			act_char("You are already flying.", ch);
			return;
		}

		if (is_bit_affected(ch, TO_AFFECTS, AFF_FLYING)
		||  ((r = race_lookup(ch->race)) && (r->aff & AFF_FLYING))
		||  has_obj_affect(ch, AFF_FLYING)
		||  IS_SET(PC(ch)->plr_flags, PLR_GHOST)) {
			SET_BIT(ch->affected_by, AFF_FLYING);
			act_char("You start to fly.", ch);
		}
		else {
			act_char("To fly find potion or wings.", ch);
			return;
		}
	} else if (!str_cmp(arg,"down")) {
		if (IS_AFFECTED(ch, AFF_FLYING)) {
			REMOVE_BIT(ch->affected_by, AFF_FLYING);
			act_char("You slowly touch the ground.", ch);
		}
		else {
			act_char("You are already on the ground.", ch);
			return;
		}
	} else {
		act_char("Type fly with 'up' or 'down'.", ch);
		return;
	}

	WAIT_STATE(ch, skill_beats("fly"));
}

DO_FUN(do_push, ch, argument)
{
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	EXIT_DATA *pexit;
	int percent;
	int door;

	argument = one_argument(argument, arg1, sizeof(arg1));
	argument = one_argument(argument, arg2, sizeof(arg2));
	if (arg1[0] == '\0' || arg2[0] == '\0') {
		act_char("Push whom to what direction?", ch);
		return;
	}

	if (MOUNTED(ch)) {
		act_char("You can't push while mounted.", ch);
		return;
	}

	if (RIDDEN(ch)) {
		act_char("You can't push while being ridden.", ch);
		return;
	}

	if (IS_NPC(ch)
	&&  IS_SET(ch->affected_by, AFF_CHARM)
	&&  ch->master != NULL) {
		act_char("You are too dazed to push anyone.", ch);
		return;
	}

	if ((victim = get_char_here(ch, arg1)) == NULL) {
		WAIT_STATE(ch, MISSING_TARGET_DELAY);
		act_char("They aren't here.", ch);
		return;
	}

	if (!IS_NPC(victim) && victim->desc == NULL) {
		act_char("You can't do that.", ch);
		return;
	}

	if (victim == ch) {
		act_char("That's pointless.", ch);
		return;
	}

	if (victim->position == POS_FIGHTING) {
		act_char("Wait until the fight finishes.", ch);
		return;
	}

	if ((door = find_exit(ch, arg2)) < 0 || door >= MAX_DIR)
		return;

	WAIT_STATE(ch, skill_beats("push"));

	if ((pexit = ch->in_room->exit[door])
	&&  IS_SET(pexit->exit_info, EX_ISDOOR)) {
		if (IS_SET(pexit->exit_info, EX_CLOSED)) {
			act_char("The door is closed.", ch);
			return;
		}
		if (IS_SET(pexit->exit_info, EX_LOCKED)) {
			act_char("The door is locked.", ch);
			return;
		}
	}

	if (IS_SET(ch->in_room->room_flags, ROOM_BATTLE_ARENA)) {
		act_char("Some mystical force prevents you from doing that.", ch);
		return;
	}

	if (IS_AFFECTED(ch, AFF_WEB)) {
		act_char("You're webbed, and want to do WHAT?!?", ch);
		act("$n stupidly tries to push $N while webbed.",
		    ch, NULL, victim, TO_ROOM);
		return;
	}

	if (IS_AFFECTED(victim, AFF_WEB)) {
		act_puts("You attempt to push $N, but the webs hold $m "
			 "in place.", victim, NULL, ch, TO_VICT, POS_DEAD);
		act("$n attempts to push $n, but fails as the webs hold "
		    "$n in place.", victim, NULL, ch, TO_NOTVICT);
		return;
	}

	if (is_safe(ch,victim))
		return;

	percent  = number_percent() + (IS_AWAKE(victim) ? 10 : -50);
	percent += can_see(victim, ch) ? -10 : 0;

	if (victim->position == POS_FIGHTING
	||  (IS_NPC(victim) && IS_SET(victim->pMobIndex->act, ACT_NOTRACK))
	||  percent > get_skill(ch, "push")
	||  pexit == NULL
	||  pexit->to_room.r == NULL
	||  pexit->to_room.r->area != ch->in_room->area) {
		/*
		 * Failure.
		 */

		act_char("Oops.", ch);
		if (!IS_AFFECTED(victim, AFF_SLEEP)) {
			victim->position = victim->position == POS_SLEEPING ?
					   POS_STANDING : victim->position;
			act("$n tried to push you.",
			    ch, NULL, victim, TO_VICT);
		}
		act("$n tried to push $N.", ch, NULL, victim, TO_NOTVICT);

		if (IS_AWAKE(victim))
			act_yell(victim, "Keep your hands off of me, $N!",
				 ch, NULL);
		if (!IS_NPC(ch) && IS_NPC(victim)) {
			check_improve(ch, "push", FALSE, 2);
			multi_hit(victim, ch, NULL);
		}
		return;
	}

	act_puts("You push $N to $t.",
		 ch, dir_name[door], victim, TO_CHAR, POS_SLEEPING);
	act_puts("$n pushes you to $t.",
		 ch, dir_name[door], victim, TO_VICT, POS_SLEEPING);
	act("$n pushes $N to $t.",
	    ch, dir_name[door], victim, TO_NOTVICT);
	move_char(victim, door, 0);

	check_improve(ch, "push", TRUE, 1);
}

DO_FUN(do_crecall, ch, argument)
{
	ROOM_INDEX_DATA *location;
	clan_t *clan;
	CHAR_DATA *pet;
	AFFECT_DATA *paf;

	if ((clan = clan_lookup(ch->clan)) == NULL) {
		act_char("Huh?", ch);
		return;
	}

	if (is_sn_affected(ch, "clan recall")) {
		act_puts("You can't pray now.",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		return;
	}

	if (ch->desc && IS_PUMPED(ch)) {
		act_puts("You are too pumped to pray now.",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		return;
	}

	act("$n prays upper Lord of Battle Ragers for transportation.",
	    ch, NULL, NULL, TO_ROOM);

	if ((location = get_room_index(clan->recall_vnum)) == NULL) {
		act_char("You are completely lost.", ch);
		return;
	}

	if (ch->in_room == location)
		return;

	if (IS_SET(ch->in_room->room_flags, ROOM_NORECALL)
	||  IS_AFFECTED(ch, AFF_CURSE)
	||  IS_AFFECTED(ch->in_room, RAFF_CURSE)) {
		act_char("The gods have forsaken you.", ch);
		return;
	}

	ch->move /= 2;

	paf = aff_new(TO_AFFECTS, "clan recall");
	paf->level	= ch->level;
	paf->duration	= skill_beats("clan recall");
	affect_to_char(ch, paf);
	aff_free(paf);

	pet = GET_PET(ch);
	recall(ch, location);

	if (pet && !IS_AFFECTED(pet, AFF_SLEEP)) {
		if (pet->position != POS_STANDING)
			do_stand(pet, str_empty);
		recall(pet, location);
	}
}

DO_FUN(do_escape, ch, argument)
{
	ROOM_INDEX_DATA *was_in;
	ROOM_INDEX_DATA *now_in;
	EXIT_DATA *pexit;
	CHAR_DATA *victim;
	char arg[MAX_INPUT_LENGTH];
	int door;

	if ((victim = ch->fighting) == NULL) {
		if (ch->position == POS_FIGHTING)
			ch->position = POS_STANDING;
		act_char("You aren't fighting anyone.", ch);
		return;
	}

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		act_char("Escape to what direction?", ch);
		return;
	}

	if (MOUNTED(ch)) {
		  act_char("You can't escape while mounted.", ch);
		  return;
	}

	if (RIDDEN(ch)) {
		  act_char("You can't escape while being ridden.", ch);
		  return;
	}

	was_in = ch->in_room;
	if ((door = find_exit(ch, arg)) < 0 || door >= MAX_DIR) {
		act_char("PANIC! You couldn't escape!", ch);
		return;
	}

	if ((pexit = was_in->exit[door]) == 0
	||  pexit->to_room.r == NULL
	||  (IS_SET(pexit->exit_info, EX_CLOSED) &&
	     (!IS_TRANSLUCENT(ch) ||
	      IS_SET(pexit->exit_info, EX_NOPASS)) &&
	     !IS_TRUSTED(ch, LEVEL_IMMORTAL))
	||  IS_SET(pexit->exit_info, EX_NOFLEE)
	||  (IS_NPC(ch) &&
	     IS_SET(pexit->to_room.r->room_flags, ROOM_NOMOB))) {
		act_char("Something prevents you to escape that direction.", ch);
		return;
	}

	if (number_percent() >= get_skill(ch, "escape")) {
		act_puts("You failed to escape.",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		check_improve(ch, "escape", FALSE, 1);
		return;
	}

	check_improve(ch, "escape", TRUE, 1);
	move_char(ch, door, 0);
	if ((now_in = ch->in_room) == was_in) {
		act_char("It's pointless to escape there.", ch);
		return;
	}

	ch->in_room = was_in;
	act("$n has escaped!", ch, NULL, NULL, TO_ROOM);
	ch->in_room = now_in;

	if (!IS_NPC(ch)) {
		act_puts("You escaped from combat!",
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
}

DO_FUN(do_layhands, ch, argument)
{
	CHAR_DATA *victim;
	AFFECT_DATA *paf;

	WAIT_STATE(ch, skill_beats("lay hands"));

	if ((victim = get_char_here(ch,argument)) == NULL) {
		act_char("They aren't here.", ch);
		return;
	}

	if (is_sn_affected(ch, "lay hands")) {
		 act_char("You can't concentrate enough.", ch);
		 return;
	}

	paf = aff_new(TO_AFFECTS, "lay hands");
	paf->duration = 2;
	affect_to_char(ch, paf);
	aff_free(paf);

	victim->hit = UMIN(victim->hit + ch->level * 2, victim->max_hit);
	update_pos(victim);
	act_char("A warm feeling fills your body.", victim);

	if (IS_AFFECTED(victim, AFF_BLIND))
		spellfun("cure blindness", NULL, ch->level, ch, victim);
	if (IS_AFFECTED(victim, AFF_PLAGUE))
		spellfun("cure disease", NULL, ch->level, ch, victim);
	if (IS_AFFECTED(victim, AFF_POISON))
		spellfun("cure poison", NULL, ch->level, ch, victim);

	if (ch != victim)
		act_char("Ok.", ch);
	check_improve(ch, "lay hands", TRUE, 1);
}

/*
 * It is not finished yet to implement all.
 */
DO_FUN(do_mount, ch, argument)
{
	char		arg[MAX_INPUT_LENGTH];
	CHAR_DATA *	mount;

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		if (ch->mount && ch->mount->in_room == ch->in_room)
			mount = ch->mount;
		else {
			act_char("Mount what?", ch);
			return;
		}
	} else if ((mount = get_char_here(ch, arg)) == NULL) {
		act_char("You don't see that here.", ch);
		return;
	}

	if (!IS_NPC(mount)
	||  !IS_SET(mount->pMobIndex->act, ACT_RIDEABLE)
	||  IS_SET(mount->pMobIndex->act, ACT_NOTRACK)) {
		act("You can't ride that.", ch, NULL, mount, TO_CHAR);
		return;
	}

	if (LEVEL(ch) < LEVEL(mount)) {
		act_char("That beast is too powerful for you to ride.", ch);
		return;
	}

	if ((mount->mount) && (!mount->riding) && (mount->mount != ch)) {
		act_puts("$N belongs to $i, not you.",
			 ch, mount->mount, mount,
			 TO_CHAR, POS_DEAD);
		return;
	}

	if (mount->position < POS_STANDING) {
		act_char("Your mount must be standing.", ch);
		return;
	}

	if (RIDDEN(mount)) {
		act_char("This beast is already ridden.", ch);
		return;
	} else if (MOUNTED(ch)) {
		act_char("You are already riding.", ch);
		return;
	}

	if (!mount_success(ch, mount, TRUE)) {
		act_char("You fail to mount the beast.", ch);
		return;
	}

	act_puts("You hop on $N's back.", ch, NULL, mount, TO_CHAR, POS_DEAD);
	act("$n hops on $N's back.", ch, NULL, mount, TO_NOTVICT);
	act_puts("$n hops on your back!", ch, NULL, mount, TO_VICT, POS_SLEEPING);

	ch->mount = mount;
	ch->riding = TRUE;
	mount->mount = ch;
	mount->riding = TRUE;

	affect_bit_strip(ch, TO_INVIS, ID_ALL_INVIS | ID_SNEAK);
	REMOVE_INVIS(ch, ID_ALL_INVIS | ID_SNEAK);
}

DO_FUN(do_dismount, ch, argument)
{
	CHAR_DATA *mount;

	if ((mount = MOUNTED(ch))) {
		act_puts("You dismount from $N.",
			 ch, NULL, mount, TO_CHAR, POS_DEAD);
		act("$n dismounts from $N.", ch, NULL, mount, TO_NOTVICT);
		act_puts("$n dismounts from you.",
			 ch, NULL, mount, TO_VICT, POS_SLEEPING);

		ch->riding = FALSE;
		mount->riding = FALSE;
	} else {
		act_char("You aren't mounted.", ch);
		return;
	}
}

static OBJ_DATA *
find_arrow(CHAR_DATA *ch)
{
	OBJ_DATA *arrow;
	OBJ_DATA *obj;

	if ((arrow = get_eq_char(ch, WEAR_HOLD)))
		return arrow;

	for (obj = ch->carrying; obj; obj = obj->next_content) {
		if (obj->wear_loc == WEAR_NONE
		||  obj->pObjIndex->item_type != ITEM_CONTAINER
		||  !IS_SET(INT(obj->value[1]), CONT_QUIVER)
		||  !obj->contains)
			continue;
		return obj->contains;
	}

	return NULL;
}

DO_FUN(do_charge, ch, argument)
{
	CHAR_DATA *victim;
	OBJ_DATA *wield;
	int direction;
	int beats;
	int chance;
	EXIT_DATA *pexit;
	ROOM_INDEX_DATA *to_room;

	char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

	argument = one_argument(argument, arg1, sizeof(arg1));
	one_argument(argument, arg2, sizeof(arg2));

	if (arg1 == '\0' || arg2 == '\0') {
		act_char("Charge whom?", ch);
		return;
	}

	if ((wield = get_eq_char(ch, WEAR_WIELD)) == NULL) {
		act_char("You need a weapon to charge.", ch);
		return;
	}

	if (!WEAPON_IS(wield, WEAPON_LANCE)
	&&  !WEAPON_IS(wield, WEAPON_SPEAR)) {
		act_char("You need lance or spear to charge.", ch);
		return;
	}

	if ((direction = find_exit(ch, arg1)) < 0 || direction >= MAX_DIR) {
		act_char("Charge whom?", ch);
		return;
	}

	if ((victim = find_char(ch, arg2, direction, 1)) == NULL) {
		WAIT_STATE(ch, MISSING_TARGET_DELAY);
		return;
	}

	if (ch->in_room == victim->in_room) {
		act("$N is here. Just MURDER $M.", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (MOUNTED(ch) == NULL) {
		act_char("You have to be riding.", ch);
		return;
	}

	if (is_safe(ch, victim))
		return;

	if (victim->hit < victim->max_hit*9/10) {
		act("$N is already bleeding, your honour do not allow you attack $M.", ch, NULL, victim, TO_CHAR);
		return;
	}

	chance = get_skill(ch, "charge") * get_skill(ch, "riding") / 100;

	if (!move_char(ch, direction, MC_F_CHARGE))
		return;
	act("$n gallops from $t, charging you!",
	    ch, from_dir_name[rev_dir[direction]], victim, TO_VICT);
	act("$n gallops from $t, charging $N!",
	    ch, from_dir_name[rev_dir[direction]], victim, TO_NOTVICT);

	beats = skill_beats("charge");
	if (number_percent() < chance) {
		one_hit(ch, victim, "charge", WEAR_WIELD);
		WAIT_STATE(victim, beats * 2);
		WAIT_STATE(ch, beats);
	} else {
		damage(ch, victim, 0, "charge", DAM_F_SHOW);
		check_improve(ch, "charge", FALSE, 1);
		if (number_percent() > get_skill(ch, "riding")) {
			if ((pexit=ch->in_room->exit[direction]) == NULL
			|| (to_room = pexit->to_room.r) == NULL
			|| !can_see_room(ch, to_room)
			|| IS_AFFECTED(ch->in_room, RAFF_RANDOMIZER)
			|| IS_SET(pexit->exit_info, EX_CLOSED)) {
				WAIT_STATE(ch, skill_beats("charge") * 2);
				return;
			}
			else {
				act("$n cannot hold $s $N.",
				    ch, dir_name[direction], ch->mount, TO_NOTVICT);
				act("You cannot hold your $N.",
				    ch, NULL, ch->mount, TO_CHAR);
				move_char(ch, direction, 0);
				WAIT_STATE(ch, beats * 5);
				return;
			}
		}
		WAIT_STATE(ch, beats * 2);
	}
	yell(victim, ch, "Help! $lu{$N} is attacking me!");
}

DO_FUN(do_shoot, ch, argument)
{
	CHAR_DATA *victim;
	OBJ_DATA *wield;
	OBJ_DATA *arrow;
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	int chance, direction;
	int range = (LEVEL(ch) / 10) + 1;

	argument = one_argument(argument, arg1, sizeof(arg1));
	one_argument(argument, arg2, sizeof(arg2));
	if (arg1[0] == '\0' || arg2[0] == '\0') {
		act_char("Shoot what direction and whom?", ch);
		return;
	}

	if (ch->fighting != NULL) {
		CHAR_DATA *vch;

		for (vch = ch->in_room->people; vch; vch = vch->next_in_room)
			if (vch->fighting == ch)
				break;
		if (vch) {
			act_char("You cannot concentrate on shooting arrows.", ch);
			return;
		}
	}

	if ((direction = find_exit(ch, arg1)) < 0 || direction >= MAX_DIR) {
		act_char("Shoot which direction and whom?", ch);
		return;
	}

	if ((victim = find_char(ch, arg2, direction, range)) == NULL) {
		WAIT_STATE(ch, MISSING_TARGET_DELAY);
		act_char("They aren't there.", ch);
		return;
	}

	if (!IS_NPC(victim) && victim->desc == NULL) {
		act_char("You can't do that.", ch);
		return;
	}

	if (victim == ch) {
		act_char("That's pointless.", ch);
		return;
	}

	if (is_safe(ch, victim))
		return;

	wield = get_eq_char(ch, WEAR_WIELD);

	if (!wield
	||  wield->pObjIndex->item_type != ITEM_WEAPON
	||  !WEAPON_IS(wield, WEAPON_BOW)) {
		act_char("You need a bow to shoot!", ch);
		return;
	}

	if (get_eq_char(ch, WEAR_SECOND_WIELD)
	||  get_eq_char(ch, WEAR_SHIELD)) {
		act_char("Your second hand should be free!", ch);
		return;
	}

	if ((arrow = find_arrow(ch)) == NULL) {
		 act_char("You need an arrow to shoot!", ch);
		 return;
	}

	if (arrow->pObjIndex->item_type != ITEM_WEAPON
	||  !WEAPON_IS(arrow, WEAPON_ARROW)) {
		act_char("That's not the right kind of arrow!", ch);
		return;
	}

	WAIT_STATE(ch, skill_beats("bow"));

	chance = get_skill(ch, "bow");
	chance = (chance - 50) * 2;
	if (ch->position == POS_SLEEPING)
		chance += 20;
	if (ch->position == POS_RESTING)
		chance += 10;
	if (victim->position == POS_FIGHTING)
		chance -= 40;
	chance += GET_HITROLL(ch);

	act_puts("You shoot $p to $T.",
		 ch, arrow, dir_name[direction], TO_CHAR, POS_DEAD);
	act("$n shoots $p to $T.",
	    ch, arrow, dir_name[direction], TO_ROOM);

	if (obj_one_hit(ch, victim, wield, arrow, direction, chance))
		check_improve(ch, "bow", TRUE, 1);
	yell(victim, ch, "Help! $lu{$N} is trying to shoot me!");
}

DO_FUN(do_human, ch, argument)
{
	if (!is_sn_affected(ch, "vampire")) {
		act_char("You are already a human.", ch);
		return;
	}

	affect_strip(ch, "vampire");
	act_char("You return to your original size.", ch);
}

DO_FUN(do_revert, ch, argument)
{
	AFFECT_DATA *paf;
	AFFECT_DATA *paf_next;

	if (!ch->shapeform) {
		act_char("You aren't shapeshifted.", ch);
	}

	for (paf = ch->affected; paf; paf = paf_next) {
		paf_next = paf->next;

		if (paf->where == TO_FORM)
			affect_remove(ch, paf);
	}

	if (ch->shapeform)
		revert(ch);
}

DO_FUN(do_throw_weapon, ch, argument)
{
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	int chance, chance2, direction;
	int range = (LEVEL(ch) / 10) + 1;
	const char *sn;

	if ((chance = get_skill(ch, "throw weapon")) == 0) {
		act_char("You don't know how to use throwing weapons.", ch);
		return;
	}

	argument = one_argument(argument, arg1, sizeof(arg1));
	one_argument(argument, arg2, sizeof(arg2));
	if (arg1[0] == '\0' || arg2[0] == '\0') {
		act_char("Throw which direction and whom?", ch);
		return;
	}

	if (ch->fighting != NULL) {
		CHAR_DATA *vch;

		for (vch = ch->in_room->people; vch; vch = vch->next_in_room)
			if (vch->fighting == ch)
				break;
		if (vch) {
			act_char("You cannot concentrate on throwing weapons.", ch);
			return;
		}
	}

	if ((direction = find_exit(ch, arg1)) < 0 || direction >= MAX_DIR) {
		act_char("Throw which direction and whom?", ch);
		return;
	}

	if ((victim = find_char(ch, arg2, direction, range)) == NULL) {
		WAIT_STATE(ch, MISSING_TARGET_DELAY);
		act_char("They aren't there.", ch);
		return;
	}

	if (victim == ch) {
		act_char("That's pointless.", ch);
		return;
	}

	if ((obj = get_eq_char(ch, WEAR_WIELD)) == NULL) {
		act_char("Throwing your hands would not be a good idea.", ch);
		return;
	}

	if (obj->pObjIndex->item_type != ITEM_WEAPON) {
		act_char("Throwing cakes will not wound your foes. Try weapons.", ch);
		return;
	}

	sn = get_weapon_sn(obj);
	if ((chance2 = get_weapon_skill(ch, sn)) == 0) {
		act_char("Damn. It has just fallen from your hand!", ch);
		if (IS_OBJ_STAT(obj, ITEM_NODROP)
		||  IS_OBJ_STAT(obj, ITEM_INVENTORY))
			obj_to_char(obj, ch);
		else
			obj_to_room(obj, ch->in_room);
		return;
	}

	if (!IS_WEAPON_STAT(obj, WEAPON_THROW)) {
		act_char("It was never designed for that.", ch);
		return;
	}

	WAIT_STATE(ch, skill_beats("throw weapon"));

	chance = (chance + chance2 - 20) / 2;
	chance = (chance - 50) * 2;
	if (ch->position == POS_SLEEPING)
		chance += 20;
	if (ch->position == POS_RESTING)
		chance += 10;
	if (victim->position == POS_FIGHTING)
		chance -= 40;
	chance += GET_HITROLL(ch);

	act_puts("You throw $p to $T.",
		 ch, obj, dir_name[direction], TO_CHAR, POS_DEAD);
	act("$n throws $p to $T.",
	    ch, obj, dir_name[direction], TO_ROOM);

	if (obj_one_hit(ch, victim, NULL, obj, direction, chance))
		check_improve(ch, "throw weapon", TRUE, 1);
	yell(victim, ch, "Help! $lu{$N} is trying to throw something at me!");
}

/* RT Enter portals */
DO_FUN(do_enter, ch, argument)
{
	ROOM_INDEX_DATA *to_room;
	ROOM_INDEX_DATA *old_room;
	OBJ_DATA *portal;
	CHAR_DATA *mount;

	if (ch->fighting != NULL)
		return;

	/* nifty portal stuff */
	if (argument[0] == '\0') {
		act_char("Nope, can't do it.", ch);
		return;
	}

	old_room = ch->in_room;
	portal = get_obj_list(ch, argument,
			      ch->in_room->contents, GETOBJ_F_ANY);
	if (portal == NULL) {
		act_char("You don't see that here.", ch);
		return;
	}

	if (portal->pObjIndex->item_type != ITEM_PORTAL
	||  (IS_SET(INT(portal->value[1]), EX_CLOSED) &&
	     !IS_TRUSTED(ch, LEVEL_IMMORTAL))) {
		act_char("You can't seem to find a way in.", ch);
		return;
	}

	if (IS_SET(INT(portal->value[2]), GATE_NOCURSE)
	&&  !IS_TRUSTED(ch, LEVEL_IMMORTAL)
	&&  (IS_AFFECTED(ch, AFF_CURSE) ||
	     IS_SET(old_room->room_flags, ROOM_NORECALL) ||
	     IS_AFFECTED(old_room, RAFF_CURSE))) {
		act_char("Something prevents you from leaving...", ch);
		return;
	}

	if (IS_SET(INT(portal->value[2]), GATE_RANDOM)
	||  INT(portal->value[3]) == -1) {
		to_room = get_random_room(ch, NULL);
		INT(portal->value[3]) = to_room->vnum; /* keeps record */
	} else if (IS_SET(INT(portal->value[2]), GATE_BUGGY)
	       &&  (number_percent() < 5))
		to_room = get_random_room(ch, NULL);
	else
		to_room = get_room_index(INT(portal->value[3]));

	if (to_room == NULL
	||  to_room == old_room
	||  !can_see_room(ch, to_room)
	||  (room_is_private(to_room) && !IS_TRUSTED(ch, LEVEL_IMP))) {
		act("$p doesn't seem to go anywhere.", ch, portal,NULL,TO_CHAR);
		return;
	}

	if (IS_NPC(ch) && IS_SET(ch->pMobIndex->act, ACT_AGGRESSIVE)
	&&  IS_SET(to_room->room_flags, ROOM_LAW)) {
	        act_char("Something prevents you from leaving...", ch);
	        return;
	}

	if (pull_obj_trigger(TRIG_OBJ_ENTER, portal, ch, NULL) > 0
	||  !mem_is(portal, MT_OBJ)
	||  IS_EXTRACTED(ch))
		return;

	if (!pull_exit_triggers(ch, -1))
		return;

	act(MOUNTED(ch) ? "$n steps into $p, riding on $N." :
			  "$n steps into $p.",
	    ch, portal, MOUNTED(ch), TO_ROOM);

	act(IS_SET(INT(portal->value[2]), GATE_NORMAL_EXIT) ?
	    "You enter $p." :
	    "You walk through $p and find yourself somewhere else...",
	    ch, portal, NULL, TO_CHAR);

	mount = MOUNTED(ch);

	if (IS_SET(INT(portal->value[2]), GATE_GOWITH)) {
		/* take the gate along */
		obj_to_room(portal, to_room);
	}

	char_to_room(ch, to_room);
	if (mount) {
		char_to_room(mount, to_room);
		ch->riding = TRUE;
		mount->riding = TRUE;
	}

	if (!IS_EXTRACTED(ch)) {
		if (IS_SET(INT(portal->value[2]), GATE_NORMAL_EXIT)) {
			act(mount ? "$n has arrived, riding $N." :
				    "$n has arrived.",
			    ch, NULL, mount, TO_ROOM);
		} else {
			act(mount ? "$n has arrived through $p, riding $N." :
				    "$n has arrived through $p.",
			    ch, portal, mount, TO_ROOM);
		}
		do_look(ch, "auto");
	}


	/* charges */
	if (INT(portal->value[0]) > 0) {
		INT(portal->value[0])--;
		if (INT(portal->value[0]) == 0)
			INT(portal->value[0]) = -1;
	}

	/* protect against circular follows */
	if (old_room == to_room)
		return;

	if (!IS_EXTRACTED(ch)) {
		CHAR_DATA *vch;

		foreach (vch, char_in_room(old_room)) {
			/*
			 * no following through dead portals
			 */
			if (!mem_is(portal, MT_OBJ)
			||  INT(portal->value[0]) == -1)
				break;

			if (vch->master != ch || vch->position != POS_STANDING)
				continue;

			act("You follow $N.", vch, NULL, ch, TO_CHAR);
			do_enter(vch, argument);
		} end_foreach(vch);
	}

	if (mem_is(portal, MT_OBJ) && INT(portal->value[0]) == -1) {
		act("$p fades out of existence.", ch, portal, NULL, TO_CHAR);
		act("$p fades out of existence.",
		    old_room->people, portal, NULL, TO_CHAR);
		extract_obj(portal, 0);
	}

	if (IS_EXTRACTED(ch))
		return;

	pull_greet_entry_triggers(ch, to_room, -1);
}

DO_FUN(do_settraps, ch, argument)
{
	if (IS_SET(ch->in_room->room_flags, ROOM_LAW)) {
		act_char("A mystical power protects the room.", ch);
		return;
	}

	WAIT_STATE(ch, skill_beats("settraps"));

	if (IS_NPC(ch)
	||  number_percent() < get_skill(ch, "settraps") * 7 / 10) {
		AFFECT_DATA *paf;

		check_improve(ch, "settraps", TRUE, 1);

		if (is_sn_affected_room(ch->in_room, "settraps")) {
			act_char("This room has already trapped.", ch);
			return;
		}

		if (is_sn_affected(ch, "settraps")) {
			act_char("This skill is used too recently.", ch);
			return;
		}

		paf = aff_new(TO_AFFECTS, "settraps");
		paf->level	= ch->level;
		if (!IS_IMMORTAL(ch) && IS_PUMPED(ch))
			paf->duration = 1;
		else
			paf->duration = ch->level / 10;
		affect_to_char(ch, paf);

		paf->where	= TO_ROOM_AFFECTS;
		paf->duration	= ch->level / 40;
		paf->owner	= ch;
		affect_to_room(ch->in_room, paf);

		aff_free(paf);

		act_char("You set the room with your trap.", ch);
		act("$n set the room with $s trap.", ch, NULL, NULL, TO_ROOM);
		return;
	} else
		check_improve(ch, "settraps", FALSE, 1);
}

DO_FUN(do_forest, ch, argument)
{
	char arg[MAX_STRING_LENGTH];
	AFFECT_DATA *paf;
	bool attack;

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		act_char("Usage: forest {{ attack | defence | normal $}", ch);
		return;
	}

	if (!str_prefix(arg, "normal")) {
		if (!is_sn_affected(ch, "forest fighting")) {
			act_char("You do not use your knowledge of forest in fight.", ch);
			return;
		} else {
			act_char("You stop using your knowledge of forest in fight.", ch);
			affect_strip(ch, "forest fighting");
			return;
		}
	}

	if (!str_prefix(arg, "defence"))
		attack = FALSE;
	else if (!str_prefix(arg, "attack"))
		attack = TRUE;
	else {
		do_forest(ch, str_empty);
		return;
	}

	if (is_sn_affected(ch, "forest fighting"))
		affect_strip(ch, "forest fighting");

	paf = aff_new(TO_AFFECTS, "forest fighting");
	paf->level	= ch->level;
	paf->duration	= -1;

	if (attack) {
		paf->modifier	= ch->level/8;
		INT(paf->location)= APPLY_HITROLL;
		affect_to_char(ch, paf);

		INT(paf->location)= APPLY_DAMROLL;

		act_puts("You feel yourself wild.",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		act("$n looks wild.", ch, NULL, NULL, TO_ROOM);
	} else {
		paf->modifier	= -ch->level;
		INT(paf->location)= APPLY_AC;

		act_puts("You feel yourself protected.",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		act("$n looks protected.", ch, NULL, NULL, TO_ROOM);
	}

	affect_to_char(ch, paf);
	aff_free(paf);
}

DO_FUN(do_breathhold, ch, argument)
{
	if (is_sn_affected(ch, "water breathing")) {
		act_char("You already can breath under water.", ch);
		return;
	}

	if (is_sn_affected(ch, "breath under water")) {
		act_char("You already can breath under water.", ch);
		return;
	}

	if (ch->in_room && ch->in_room->sector_type == SECT_UNDERWATER) {
		act_char("You can't take a breath under water!", ch);
		return;
	}

	if (number_percent() < get_skill(ch, "hold breath")) {
		AFFECT_DATA *paf;

		act_char("You prepare yourself for swimming.", ch);
		check_improve(ch, "hold breath", TRUE, 1);

		paf = aff_new(TO_AFFECTS, "water breathing");
		paf->level	= LEVEL(ch);
		paf->duration	= LEVEL(ch) / 20;
		affect_to_char(ch, paf);
		aff_free(paf);
	} else {
		act_char("You took a deep breath but fail to concentrate.", ch);
		check_improve(ch, "hold breath", FALSE, 1);
	}
	WAIT_STATE(ch, skill_beats("hold breath"));
}

DO_FUN(do_cast, ch, argument)
{
	cast_spell_or_prayer(ch, argument, ST_SPELL);
}

DO_FUN(do_pray, ch, argument)
{
	cast_spell_or_prayer(ch, argument, ST_PRAYER);
}

/*--------------------------------------------------------------------
 * static functions
 */
static int
find_exit(CHAR_DATA *ch, char *arg)
{
	int door;

	     if (!str_cmp(arg, "n") || !str_cmp(arg, "north")) door = 0;
	else if (!str_cmp(arg, "e") || !str_cmp(arg, "east" )) door = 1;
	else if (!str_cmp(arg, "s") || !str_cmp(arg, "south")) door = 2;
	else if (!str_cmp(arg, "w") || !str_cmp(arg, "west" )) door = 3;
	else if (!str_cmp(arg, "u") || !str_cmp(arg, "up"   )) door = 4;
	else if (!str_cmp(arg, "d") || !str_cmp(arg, "down" )) door = 5;
	else {
		act_puts("I see no exit $T here.",
			 ch, NULL, arg, TO_CHAR, POS_DEAD);
		return -1;
	}

	return door;
}
