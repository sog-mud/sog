/*
 * $Id: act_move.c,v 1.121 1998-12-14 04:26:05 fjoe Exp $
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
#include "merc.h"
#include "hometown.h"
#include "update.h"
#include "mob_prog.h"
#include "obj_prog.h"
#include "interp.h"
#include "fight.h"

#include "resource.h"

char *	const	dir_name	[]		=
{
	"north", "east", "south", "west", "up", "down"
};

const	int	rev_dir		[]		=
{
	2, 3, 0, 1, 5, 4
};

const	int	movement_loss	[SECT_MAX]	=
{
	1, 2, 2, 3, 4, 6, 4, 1, 6, 10, 6
};



/*
 * Local functions.
 */
int	find_door	(CHAR_DATA *ch, char *arg);
int 	find_exit	(CHAR_DATA *ch, char *arg);
bool	has_key		(CHAR_DATA *ch, int key);
int	mount_success	(CHAR_DATA *ch, CHAR_DATA *mount, int canattack);

void move_char(CHAR_DATA *ch, int door, bool follow)
{
	CHAR_DATA *fch;
	CHAR_DATA *fch_next;
	CHAR_DATA *mount;
	ROOM_INDEX_DATA *in_room;
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	bool room_has_pc;
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;
	int act_flags;

	if (RIDDEN(ch) && !IS_NPC(ch->mount)) {
		move_char(ch->mount,door,follow);
		return;
	}

	if (IS_AFFECTED(ch, AFF_DETECT_WEB) 
	|| (MOUNTED(ch) && IS_AFFECTED(ch->mount, AFF_DETECT_WEB))) {
		WAIT_STATE(ch, PULSE_VIOLENCE);
		if (number_percent() < str_app[IS_NPC(ch) ?
			20 : get_curr_stat(ch,STAT_STR)].tohit * 5) {
		 	affect_strip(ch, gsn_web);
		 	REMOVE_BIT(ch->affected_by, AFF_DETECT_WEB);
		 	char_nputs(MSG_WHEN_YOU_ATTEMPT_YOU_BREAK_WEBS, ch);
		 	act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING,
				    MSG_N_BREAKS_THE_WEBS);
		} else {
			char_nputs(MSG_YOU_ATTEMPT_WEBS_HOLD_YOU, ch);
			act_printf(ch, NULL, NULL, TO_ROOM, POS_RESTING,
				   "$n struggles vainly against the webs which hold $m in place.");
			return; 
		}
	}

	if (door < 0 || door >= MAX_DIR) {
		bug("Do_move: bad door %d.", door);
		return;
	}

	if (IS_AFFECTED(ch, AFF_HIDE | AFF_FADE)
	&&  !IS_AFFECTED(ch, AFF_SNEAK)) {
		REMOVE_BIT(ch->affected_by, AFF_HIDE);
		char_nputs(MSG_YOU_STEP_OUT_SHADOWS, ch);
		act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING,
			    MSG_N_STEPS_OUT_OF_SHADOWS);
	}

	if (IS_AFFECTED(ch, AFF_CAMOUFLAGE))  {
		int chance;

		if ((chance = get_skill(ch, gsn_camouflage_move)) == 0) {
			REMOVE_BIT(ch->affected_by, AFF_CAMOUFLAGE);
			char_nputs(MSG_YOU_STEP_OUT_COVER, ch);
			act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING,
				    MSG_N_STEPS_OUT_COVER);
		}	    
		else if (number_percent() < chance)
			check_improve(ch, gsn_camouflage_move, TRUE, 5);
		else {
			REMOVE_BIT(ch->affected_by, AFF_CAMOUFLAGE);
			char_nputs(MSG_YOU_STEP_OUT_COVER, ch);
			act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING,
				    MSG_N_STEPS_OUT_COVER);
			check_improve(ch, gsn_camouflage_move, FALSE, 5);
		}	    
	}

	/*
	 * Exit trigger, if activated, bail out. Only PCs are triggered.
	 */
	if (!IS_NPC(ch) && mp_exit_trigger(ch, door))
		return;

	in_room = ch->in_room;
	if ((pexit = in_room->exit[door]) == NULL
	||  (to_room = pexit->u1.to_room) == NULL 
	||  !can_see_room(ch, pexit->u1.to_room)) {
		char_puts("Alas, you cannot go that way.\n", ch);
		return;
	}

	if (IS_ROOM_AFFECTED(in_room, RAFF_RANDOMIZER)) {
		int d0;
		while (1) {
			d0 = number_range(0, MAX_DIR-1);
			if ((pexit = in_room->exit[d0]) == NULL
			||  (to_room = pexit->u1.to_room) == NULL 
			||  !can_see_room(ch, pexit->u1.to_room))
				continue;	  
			door = d0;
			break;
		}
	}

	if (IS_SET(pexit->exit_info, EX_CLOSED) 
	&&  (!IS_AFFECTED(ch, AFF_PASS_DOOR) ||
	     IS_SET(pexit->exit_info, EX_NOPASS))
	&&  !IS_TRUSTED(ch, ANGEL)) {
		if (IS_AFFECTED(ch, AFF_PASS_DOOR)
		&&  IS_SET(pexit->exit_info, EX_NOPASS))  {
  			act_nprintf(ch, NULL, pexit->keyword, TO_CHAR,
				    POS_RESTING, MSG_YOU_FAILED_TO_PASS);
			act_nprintf(ch, NULL, pexit->keyword, TO_ROOM,
				    POS_RESTING, MSG_N_TRIES_TO_PASS_FAILED);
		}
		else
			act_printf(ch, NULL, pexit->keyword, TO_CHAR,
				   POS_RESTING, "The $d is closed.");
		return;
	}

	if (IS_AFFECTED(ch, AFF_CHARM)
	&&  ch->master != NULL
	&&  in_room == ch->master->in_room) {
		char_puts("What? And leave your beloved master?\n", ch);
		return;
	}

/*    if (!is_room_owner(ch,to_room) && room_is_private(to_room))	*/
	if (room_is_private(to_room)) {
		char_puts("That room is private right now.\n", ch);
		return;
	}

	if (MOUNTED(ch)) {
		if (MOUNTED(ch)->position < POS_FIGHTING) {
			char_puts("Your mount must be standing.\n", ch);
			return; 
		}
		if (!mount_success(ch, MOUNTED(ch), FALSE)) {
			char_puts("Your mount subbornly refuses to go that way.\n", ch);
			return;
		}
	}

	if (!IS_NPC(ch)) {
		int move;

		if (guild_check(ch, to_room) < 0) {
			char_puts("You aren't allowed there.\n", ch);
			return;
		}

		if (!IS_IMMORTAL(ch) && IS_PUMPED(ch)
		&&  IS_SET(to_room->room_flags, ROOM_SAFE)) {
			act_puts("You feel too bloody to go in there now.",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
			return;
		}

		if (in_room->sector_type == SECT_AIR
		||  to_room->sector_type == SECT_AIR) {
			if (MOUNTED(ch)) {
		        	if(!IS_AFFECTED(MOUNTED(ch), AFF_FLYING)) {
		        		char_puts("You mount can't fly.\n", ch);
						   return;
				}
			} 
			else if (!IS_AFFECTED(ch, AFF_FLYING)
			&& !IS_IMMORTAL(ch)) {
				char_puts("You can't fly.\n", ch);
				return;
			} 
		}

		if ((in_room->sector_type == SECT_WATER_NOSWIM ||
		     to_room->sector_type == SECT_WATER_NOSWIM)
		&&  (MOUNTED(ch) && !IS_AFFECTED(MOUNTED(ch),AFF_FLYING))) {
			char_puts("You can't take your mount there.\n", ch);
			return;
		}  

		if ((in_room->sector_type == SECT_WATER_NOSWIM ||
		     to_room->sector_type == SECT_WATER_NOSWIM)
		&&  (!MOUNTED(ch) && !IS_AFFECTED(ch, AFF_FLYING))) {
			OBJ_DATA *obj;
			bool found;

		    /*
		     * Look for a boat.
		     */
		    found = FALSE;

		    if (IS_IMMORTAL(ch))
			found = TRUE;

		    for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
		    {
			if (obj->pIndexData->item_type == ITEM_BOAT)
			{
			    found = TRUE;
			    break;
			}
		    }
		    if (!found)
		    {
			char_puts("You need a boat to go there.\n", ch);
			return;
		    }
		}

		move = (movement_loss[UMIN(SECT_MAX-1, in_room->sector_type)]
		     + movement_loss[UMIN(SECT_MAX-1, to_room->sector_type)])/2;

		if (is_affected(ch, gsn_thumbling))
			move *= 2;
		else {
			if (IS_AFFECTED(ch,AFF_FLYING)
			|| IS_AFFECTED(ch,AFF_HASTE))
				move /= 2;

			if (IS_AFFECTED(ch,AFF_SLOW))
				move *= 2;
		}

		if (!MOUNTED(ch) && ch->move < move) {
			char_nputs(MSG_YOU_TOO_EXHAUSTED, ch);
			return;
		}

		if (!MOUNTED(ch) && 
		(ch->in_room->sector_type == SECT_DESERT || IS_WATER(ch->in_room)))
		  WAIT_STATE(ch, 2);
		else
		  WAIT_STATE(ch, 1);
		
		if (!MOUNTED(ch))	ch->move -= move;
	}

	if (!IS_AFFECTED(ch, AFF_SNEAK)
	&&  !IS_AFFECTED(ch, AFF_CAMOUFLAGE)
	&&  ch->invis_level < LEVEL_HERO) 
		act_flags = TO_ROOM;
	else
		act_flags = TO_ROOM | ACT_NOMORTAL;

	if (!IS_NPC(ch)
	&&  ch->in_room->sector_type != SECT_INSIDE
	&&  ch->in_room->sector_type != SECT_CITY
	&&  number_percent() < get_skill(ch, gsn_quiet_movement)) {
		act(MOUNTED(ch) ? "$n leaves, riding on $N." : "$n leaves.",
		    ch, NULL, MOUNTED(ch), act_flags);
		check_improve(ch,gsn_quiet_movement,TRUE,1);
	}
	else {
		act(MOUNTED(ch) ? "$n leaves $t, riding on $N." :
				  "$n leaves $t.",
		    ch, dir_name[door], MOUNTED(ch), act_flags | ACT_TRANS);
	}

	if (IS_AFFECTED(ch, AFF_CAMOUFLAGE)
	&&  to_room->sector_type != SECT_FOREST
	&&  to_room->sector_type != SECT_MOUNTAIN
	&&  to_room->sector_type != SECT_HILLS) {
		REMOVE_BIT(ch->affected_by, AFF_CAMOUFLAGE);
		char_nputs(MSG_YOU_STEP_OUT_COVER, ch);
		act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING,
			    MSG_N_STEPS_OUT_COVER);
	}

	mount = MOUNTED(ch);
	char_from_room(ch);
	char_to_room(ch, to_room);

	/* room record for tracking */
	if (!IS_NPC(ch) && ch->in_room)
		room_record(ch->name, in_room, door);

	if (!IS_AFFECTED(ch, AFF_SNEAK) && ch->invis_level < LEVEL_HERO) 
		act_flags = TO_ROOM;
	else
		act_flags = TO_ROOM | ACT_NOMORTAL;

	act_nprintf(ch, NULL, mount, act_flags, POS_RESTING,
		    mount ? MSG_ARRIVED_RIDING : MSG_ARRIVED);

	do_look(ch, "auto");

	if (mount) {
		char_from_room(mount);
		char_to_room(mount, to_room);
  		ch->riding = TRUE;
  		mount->riding = TRUE;
	}

	if (in_room == to_room) /* no circular follows */
		return;

	room_has_pc = FALSE;
	for (fch = to_room->people; fch != NULL; fch = fch_next) {
		fch_next = fch->next_in_room;
		if (!IS_NPC(fch)) {
			room_has_pc = TRUE;
			break;
		}
	}

	for (fch = to_room->people; fch != NULL; fch = fch_next) {
		fch_next = fch->next_in_room;

		/* greet progs for items carried by people in room */
		if (room_has_pc)
			for (obj = fch->carrying; obj != NULL; obj = obj_next) {
				obj_next = obj->next_content;
				oprog_call(OPROG_GREET, obj, ch, NULL);
			}
	}

	/* entry programs for items */
	if (room_has_pc)
		for (obj = ch->carrying; obj != NULL; obj = obj_next) {
			obj_next = obj->next_content;
			oprog_call(OPROG_ENTRY, obj, NULL, NULL);
		}

	for (fch = in_room->people; fch != NULL; fch = fch_next) {
		fch_next = fch->next_in_room;

		if (fch->master != ch || fch->position != POS_STANDING
		||  !can_see_room(fch, to_room))
			continue;

		if (IS_SET(ch->in_room->room_flags, ROOM_LAW)
		&&  IS_NPC(fch)
		&&  IS_SET(fch->act,ACT_AGGRESSIVE)) {
			act_printf(ch, NULL, fch, TO_CHAR, POS_DEAD,
				   "You can't bring $N into the city.");
			act("You aren't allowed in the city.",
			    fch, NULL, NULL, TO_CHAR);
			continue;
		}

		act_puts("You follow $N.", fch, NULL, ch, TO_CHAR, POS_DEAD);
		move_char(fch, door, TRUE);
	}

	/* 
	 * If someone is following the char, these triggers get activated
	 * for the followers before the char, but it's safer this way...
	 */
	if (IS_NPC(ch) && HAS_TRIGGER(ch, TRIG_ENTRY))
		mp_percent_trigger(ch, NULL, NULL, NULL, TRIG_ENTRY);
	if (!IS_NPC(ch))
    		mp_greet_trigger(ch);

	if (room_has_pc)
		for (obj = ch->in_room->contents; obj != NULL; obj = obj_next) {
			obj_next = obj->next_content;
			oprog_call(OPROG_GREET, obj, ch, NULL);
		}
}



void do_north(CHAR_DATA *ch, const char *argument)
{
	move_char(ch, DIR_NORTH, FALSE);
}



void do_east(CHAR_DATA *ch, const char *argument)
{
	move_char(ch, DIR_EAST, FALSE);
}



void do_south(CHAR_DATA *ch, const char *argument)
{
	move_char(ch, DIR_SOUTH, FALSE);
}



void do_west(CHAR_DATA *ch, const char *argument)
{
	move_char(ch, DIR_WEST, FALSE);
}



void do_up(CHAR_DATA *ch, const char *argument)
{
	move_char(ch, DIR_UP, FALSE);
}



void do_down(CHAR_DATA *ch, const char *argument)
{
	move_char(ch, DIR_DOWN, FALSE);
}



int find_exit(CHAR_DATA *ch, char *arg)
{
	int door;

	     if (!str_cmp(arg, "n") || !str_cmp(arg, "north")) door = 0;
	else if (!str_cmp(arg, "e") || !str_cmp(arg, "east" )) door = 1;
	else if (!str_cmp(arg, "s") || !str_cmp(arg, "south")) door = 2;
	else if (!str_cmp(arg, "w") || !str_cmp(arg, "west" )) door = 3;
	else if (!str_cmp(arg, "u") || !str_cmp(arg, "up"   )) door = 4;
	else if (!str_cmp(arg, "d") || !str_cmp(arg, "down" )) door = 5;
	else {
		act_printf(ch, NULL, arg, TO_CHAR, POS_DEAD,
			   "I see no exit $T here.");
		return -1;
	}

	return door;
}


int find_door(CHAR_DATA *ch, char *arg)
{
	EXIT_DATA *pexit;
	int door;

	     if (!str_cmp(arg, "n") || !str_cmp(arg, "north")) door = 0;
	else if (!str_cmp(arg, "e") || !str_cmp(arg, "east" )) door = 1;
	else if (!str_cmp(arg, "s") || !str_cmp(arg, "south")) door = 2;
	else if (!str_cmp(arg, "w") || !str_cmp(arg, "west" )) door = 3;
	else if (!str_cmp(arg, "u") || !str_cmp(arg, "up"   )) door = 4;
	else if (!str_cmp(arg, "d") || !str_cmp(arg, "down" )) door = 5;
	else {
		for (door = 0; door <= 5; door++) {
		    if ((pexit = ch->in_room->exit[door]) != NULL
		    &&   IS_SET(pexit->exit_info, EX_ISDOOR)
		    &&   pexit->keyword != NULL
		    &&   is_name(arg, pexit->keyword))
			return door;
		}
		act_puts("I see no $T here.", ch, NULL, arg, TO_CHAR, POS_DEAD);
		return -1;
	}

	if ((pexit = ch->in_room->exit[door]) == NULL) {
		act_puts("I see no door $T here.",
			 ch, NULL, arg, TO_CHAR, POS_DEAD);
		return -1;
	}

	if (!IS_SET(pexit->exit_info, EX_ISDOOR)) {
		char_puts("You can't do that.\n", ch);
		return -1;
	}

	return door;
}

void do_open(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	int door;

	one_argument(argument, arg);

	if (arg[0] == '\0') {
		char_puts("Open what?\n", ch);
		return;
	}



	if ((obj = get_obj_here(ch, arg)) != NULL) {
 	/* open portal */
		if (obj->pIndexData->item_type == ITEM_PORTAL)
		{
		    if (!IS_SET(obj->value[1], EX_ISDOOR))
		    {
			char_puts("You can't do that.\n", ch);
			return;
		    }

		    if (!IS_SET(obj->value[1], EX_CLOSED)) {
			char_puts("It's already open.\n", ch);
			return;
		    }

		    if (IS_SET(obj->value[1], EX_LOCKED)) {
			char_puts("It's locked.\n", ch);
			return;
		    }

		    REMOVE_BIT(obj->value[1], EX_CLOSED);
		    act_puts("You open $p.", ch, obj, NULL, TO_CHAR, POS_DEAD);
		    act("$n opens $p.", ch, obj, NULL, TO_ROOM);
		    return;
 	}

		/* 'open object' */
		if (obj->pIndexData->item_type != ITEM_CONTAINER)
		    { char_puts("That's not a container.\n", ch); return; }
		if (!IS_SET(obj->value[1], CONT_CLOSED))
		    { char_puts("It's already open.\n", ch); return; }
		if (!IS_SET(obj->value[1], CONT_CLOSEABLE))
		    { char_puts("You can't do that.\n", ch); return; }
		if (IS_SET(obj->value[1], CONT_LOCKED))
		    { char_puts("It's locked.\n", ch); return; }

		REMOVE_BIT(obj->value[1], CONT_CLOSED);
		act_puts("You open $p.", ch, obj, NULL, TO_CHAR, POS_DEAD);
		act("$n opens $p.", ch, obj, NULL, TO_ROOM);
		return;
	}

	if ((door = find_door(ch, arg)) >= 0) {
		/* 'open door' */
		ROOM_INDEX_DATA *to_room;
		EXIT_DATA *pexit;
		EXIT_DATA *pexit_rev;

		pexit = ch->in_room->exit[door];
		if (!IS_SET(pexit->exit_info, EX_CLOSED))
		    { char_puts("It's already open.\n", ch); return; }
		if ( IS_SET(pexit->exit_info, EX_LOCKED))
		    { char_puts("It's locked.\n", ch); return; }

		REMOVE_BIT(pexit->exit_info, EX_CLOSED);
		act("$n opens the $d.", ch, NULL, pexit->keyword, TO_ROOM);
		char_puts("Ok.\n", ch);

		/* open the other side */
		if ((to_room   = pexit->u1.to_room           ) != NULL
		&&   (pexit_rev = to_room->exit[rev_dir[door]]) != NULL
		&&   pexit_rev->u1.to_room == ch->in_room) {
			ROOM_INDEX_DATA *in_room;

			REMOVE_BIT(pexit_rev->exit_info, EX_CLOSED);

			in_room = ch->in_room;
			ch->in_room = to_room;
			act("The $d opens.", ch, NULL, pexit_rev->keyword,
			    TO_ROOM);
			ch->in_room = in_room;
		}
		return;
	}
}

void do_close(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	int door;

	one_argument(argument, arg);

	if (arg[0] == '\0') {
		char_puts("Close what?\n", ch);
		return;
	}

	if ((obj = get_obj_here(ch, arg)) != NULL) {
		/* portal stuff */
		if (obj->pIndexData->item_type == ITEM_PORTAL)
		{

		    if (!IS_SET(obj->value[1],EX_ISDOOR)
		    ||   IS_SET(obj->value[1],EX_NOCLOSE))
		    {
			char_puts("You can't do that.\n", ch);
			return;
		    }

		    if (IS_SET(obj->value[1],EX_CLOSED)) {
			char_puts("It's already closed.\n", ch);
			return;
		    }

		    SET_BIT(obj->value[1],EX_CLOSED);
		    act_puts("You close $p.", ch, obj, NULL, TO_CHAR, POS_DEAD);
		    act("$n closes $p.", ch, obj, NULL, TO_ROOM);
		    return;
		}

		/* 'close object' */
		if (obj->pIndexData->item_type != ITEM_CONTAINER)
		    { char_puts("That's not a container.\n", ch); return; }
		if (IS_SET(obj->value[1], CONT_CLOSED))
		    { char_puts("It's already closed.\n", ch); return; }
		if (!IS_SET(obj->value[1], CONT_CLOSEABLE))
		    { char_puts("You can't do that.\n", ch); return; }

		SET_BIT(obj->value[1], CONT_CLOSED);
		act_puts("You close $p.", ch, obj, NULL, TO_CHAR, POS_DEAD);
		act("$n closes $p.", ch, obj, NULL, TO_ROOM);
		return;
	}

	if ((door = find_door(ch, arg)) >= 0) {
		/* 'close door' */
		ROOM_INDEX_DATA *to_room;
		EXIT_DATA *pexit;
		EXIT_DATA *pexit_rev;

		pexit	= ch->in_room->exit[door];
		if (IS_SET(pexit->exit_info, EX_CLOSED))
		    { char_puts("It's already closed.\n", ch); return; }

		SET_BIT(pexit->exit_info, EX_CLOSED);
		act("$n closes $d.", ch, NULL, pexit->keyword, TO_ROOM);
		char_puts("Ok.\n", ch);

		/* close the other side */
		if ((to_room   = pexit->u1.to_room           ) != NULL
		&&   (pexit_rev = to_room->exit[rev_dir[door]]) != 0
		&&   pexit_rev->u1.to_room == ch->in_room) {
			ROOM_INDEX_DATA *in_room;

			SET_BIT(pexit_rev->exit_info, EX_CLOSED);
			in_room = ch->in_room;
			ch->in_room = to_room;
			act("The $d closes.", ch, NULL, pexit_rev->keyword,
			    TO_ROOM);
			ch->in_room = in_room;
		}
		return;
	}
}

/* 
 * Added can_see check. Kio.
 */
bool has_key(CHAR_DATA *ch, int key)
{
	OBJ_DATA *obj;

	for (obj = ch->carrying; obj; obj = obj->next_content)
		if (obj->pIndexData->vnum == key
		&&  can_see_obj(ch, obj))
		    return TRUE;

	return FALSE;
}

bool has_key_ground(CHAR_DATA *ch, int key)
{
	OBJ_DATA *obj;

	for (obj = ch->in_room->contents; obj; obj = obj->next_content)
		if (obj->pIndexData->vnum == key
		&&  can_see_obj(ch, obj))
		    return TRUE;

	return FALSE;
}

void do_lock(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	int door;

	one_argument(argument, arg);

	if (arg[0] == '\0') {
		char_puts("Lock what?\n", ch);
		return;
	}

	if ((obj = get_obj_here(ch, arg)) != NULL) {
		/* portal stuff */
		if (obj->pIndexData->item_type == ITEM_PORTAL) {
		    if (!IS_SET(obj->value[1], EX_ISDOOR)
		    ||  IS_SET(obj->value[1], EX_NOCLOSE)) {
			char_puts("You can't do that.\n", ch);
			return;
		    }
		    if (!IS_SET(obj->value[1], EX_CLOSED)) {
			char_puts("It's not closed.\n", ch);
		 	return;
		    }

		    if (obj->value[4] < 0 || IS_SET(obj->value[1], EX_NOLOCK)) {
			char_puts("It can't be locked.\n", ch);
			return;
		    }

		    if (!has_key(ch, obj->value[4])) {
			char_puts("You lack the key.\n", ch);
			return;
		    }

		    if (IS_SET(obj->value[1], EX_LOCKED)) {
			char_puts("It's already locked.\n", ch);
			return;
		    }

		    SET_BIT(obj->value[1], EX_LOCKED);
		    act_puts("You lock $p.", ch, obj, NULL, TO_CHAR, POS_DEAD);
		    act("$n locks $p.", ch, obj, NULL, TO_ROOM);
		    return;
		}

		/* 'lock object' */
		if (obj->pIndexData->item_type != ITEM_CONTAINER)
		    { char_puts("That's not a container.\n", ch); return; }
		if (!IS_SET(obj->value[1], CONT_CLOSED))
		    { char_puts("It's not closed.\n", ch); return; }
		if (obj->value[2] < 0)
		    { char_puts("It can't be locked.\n", ch); return; }
		if (!has_key(ch, obj->value[2]))
		    { char_puts("You lack the key.\n", ch); return; }
		if (IS_SET(obj->value[1], CONT_LOCKED))
		    { char_puts("It's already locked.\n", ch); return; }

		SET_BIT(obj->value[1], CONT_LOCKED);
		act_puts("You lock $p.", ch, obj, NULL, TO_CHAR, POS_DEAD);
		act("$n locks $p.", ch, obj, NULL, TO_ROOM);
		return;
	}

	if ((door = find_door(ch, arg)) >= 0)
	{
		/* 'lock door' */
		ROOM_INDEX_DATA *to_room;
		EXIT_DATA *pexit;
		EXIT_DATA *pexit_rev;

		pexit	= ch->in_room->exit[door];
		if (!IS_SET(pexit->exit_info, EX_CLOSED))
		    { char_puts("It's not closed.\n", ch); return; }
		if (pexit->key < 0)
		    { char_puts("It can't be locked.\n", ch); return; }
		if (!has_key(ch, pexit->key) && 
		       !has_key_ground(ch, pexit->key))
		    { char_puts("You lack the key.\n", ch); return; }
		if (IS_SET(pexit->exit_info, EX_LOCKED))
		    { char_puts("It's already locked.\n", ch); return; }

		SET_BIT(pexit->exit_info, EX_LOCKED);
		char_puts("*Click*\n", ch);
		act("$n locks the $d.", ch, NULL, pexit->keyword, TO_ROOM);

		/* lock the other side */
		if ((to_room   = pexit->u1.to_room           ) != NULL
		&&   (pexit_rev = to_room->exit[rev_dir[door]]) != 0
		&&   pexit_rev->u1.to_room == ch->in_room) {
			ROOM_INDEX_DATA *in_room;

			SET_BIT(pexit_rev->exit_info, EX_LOCKED);

			in_room = ch->in_room;
			ch->in_room = to_room;
			act("The $d clicks.",
			    ch, NULL, pexit_rev->keyword, TO_ROOM);
			ch->in_room  = in_room;
		}
		return;
	}
}

void do_unlock(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	int door;

	one_argument(argument, arg);

	if (arg[0] == '\0') {
		char_puts("Unlock what?\n", ch);
		return;
	}

	if ((obj = get_obj_here(ch, arg)) != NULL) {
 	/* portal stuff */
		if (obj->pIndexData->item_type == ITEM_PORTAL) {
		    if (IS_SET(obj->value[1],EX_ISDOOR)) {
			char_puts("You can't do that.\n", ch);
			return;
		    }

		    if (!IS_SET(obj->value[1],EX_CLOSED)) {
			char_puts("It's not closed.\n", ch);
			return;
		    }

		    if (obj->value[4] < 0) {
			char_puts("It can't be unlocked.\n", ch);
			return;
		    }

		    if (!has_key(ch,obj->value[4])) {
			char_puts("You lack the key.\n", ch);
			return;
		    }

		    if (!IS_SET(obj->value[1],EX_LOCKED)) {
			char_puts("It's already unlocked.\n", ch);
			return;
		    }

		    REMOVE_BIT(obj->value[1],EX_LOCKED);
		    act_puts("You unlock $p.", ch, obj, NULL, TO_CHAR, POS_DEAD);
		    act("$n unlocks $p.", ch, obj, NULL, TO_ROOM);
		    return;
		}

		/* 'unlock object' */
		if (obj->pIndexData->item_type != ITEM_CONTAINER)
		    { char_puts("That's not a container.\n", ch); return; }
		if (!IS_SET(obj->value[1], CONT_CLOSED))
		    { char_puts("It's not closed.\n", ch); return; }
		if (obj->value[2] < 0)
		    { char_puts("It can't be unlocked.\n", ch); return; }
		if (!has_key(ch, obj->value[2]))
		    { char_puts("You lack the key.\n", ch); return; }
		if (!IS_SET(obj->value[1], CONT_LOCKED))
		    { char_puts("It's already unlocked.\n", ch); return; }

		REMOVE_BIT(obj->value[1], CONT_LOCKED);
		act_puts("You unlock $p.", ch, obj, NULL, TO_CHAR, POS_DEAD);
		act("$n unlocks $p.", ch, obj, NULL, TO_ROOM);
		return;
	}

	if ((door = find_door(ch, arg)) >= 0)
	{
		/* 'unlock door' */
		ROOM_INDEX_DATA *to_room;
		EXIT_DATA *pexit;
		EXIT_DATA *pexit_rev;

		pexit = ch->in_room->exit[door];

		if (!IS_SET(pexit->exit_info, EX_CLOSED))
		    { char_puts("It's not closed.\n", ch); return; }
		if (pexit->key < 0)
		    { char_puts("It can't be unlocked.\n", ch); return; }
		if (!has_key(ch, pexit->key) && 
		       !has_key_ground(ch, pexit->key))
		    { char_puts("You lack the key.\n", ch); return; }
		if (!IS_SET(pexit->exit_info, EX_LOCKED))
		    { char_puts("It's already unlocked.\n", ch); return; }

		REMOVE_BIT(pexit->exit_info, EX_LOCKED);
		char_puts("*Click*\n", ch);
		act("$n unlocks the $d.", ch, NULL, pexit->keyword, TO_ROOM);

		/* unlock the other side */
		if ((to_room   = pexit->u1.to_room           ) != NULL
		&&   (pexit_rev = to_room->exit[rev_dir[door]]) != NULL
		&&   pexit_rev->u1.to_room == ch->in_room) {
			ROOM_INDEX_DATA *in_room;

			REMOVE_BIT(pexit_rev->exit_info, EX_LOCKED);

			in_room = ch->in_room;
			ch->in_room = to_room;
			act("The $d clicks.",
			    ch, NULL, pexit_rev->keyword, TO_ROOM);
			ch->in_room = in_room;
		}
		return;
	}
}

void do_pick(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *gch;
	OBJ_DATA *obj;
	int door;
	int chance;

	if ((chance = get_skill(ch, gsn_pick)) == 0) {
		char_puts("Huh?\n", ch);
		return;
	}

	one_argument(argument, arg);

	if (arg[0] == '\0') {
		char_puts("Pick what?\n", ch);
		return;
	}

	if (MOUNTED(ch)) {
		  char_puts("You can't pick while mounted.\n", ch);
		  return;
	}

	WAIT_STATE(ch, SKILL(gsn_pick)->beats);

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

	if (!IS_NPC(ch) && number_percent() > chance) {
		char_puts("You failed.\n", ch);
		check_improve(ch, gsn_pick, FALSE, 2);
		return;
	}

	if ((obj = get_obj_here(ch, arg)) != NULL) {
		/* portal stuff */
		if (obj->pIndexData->item_type == ITEM_PORTAL) {
		    if (!IS_SET(obj->value[1],EX_ISDOOR)) {	
			char_puts("You can't do that.\n", ch);
			return;
		    }

		    if (!IS_SET(obj->value[1],EX_CLOSED)) {
			char_puts("It's not closed.\n", ch);
			return;
		    }

		    if (obj->value[4] < 0) {
			char_puts("It can't be unlocked.\n", ch);
			return;
		    }

		    if (IS_SET(obj->value[1],EX_PICKPROOF)) {
			char_puts("You failed.\n", ch);
			return;
		    }

		    REMOVE_BIT(obj->value[1],EX_LOCKED);
		    act_puts("You pick the lock on $p.", ch, obj, NULL, TO_CHAR, POS_DEAD);
		    act("$n picks the lock on $p.", ch, obj, NULL, TO_ROOM);
		    check_improve(ch, gsn_pick, TRUE, 2);
		    return;
		}

		
		/* 'pick object' */
		if (obj->pIndexData->item_type != ITEM_CONTAINER)
		    { char_puts("That's not a container.\n", ch); return; }
		if (!IS_SET(obj->value[1], CONT_CLOSED))
		    { char_puts("It's not closed.\n", ch); return; }
		if (obj->value[2] < 0)
		    { char_puts("It can't be unlocked.\n", ch); return; }
		if (!IS_SET(obj->value[1], CONT_LOCKED))
		    { char_puts("It's already unlocked.\n", ch); return; }
		if (IS_SET(obj->value[1], CONT_PICKPROOF))
		    { char_puts("You failed.\n", ch); return; }

		REMOVE_BIT(obj->value[1], CONT_LOCKED);
		act_puts("You pick the lock on $p.", ch, obj, NULL, TO_CHAR, POS_DEAD);
		act("$n picks the lock on $p.", ch, obj, NULL, TO_ROOM);
		check_improve(ch, gsn_pick, TRUE, 2);
		return;
	}

	if ((door = find_door(ch, arg)) >= 0) {
		/* 'pick door' */
		ROOM_INDEX_DATA *to_room;
		EXIT_DATA *pexit;
		EXIT_DATA *pexit_rev;

		pexit = ch->in_room->exit[door];
		if (!IS_SET(pexit->exit_info, EX_CLOSED) && !IS_IMMORTAL(ch))
		    { char_puts("It's not closed.\n", ch); return; }
		if (pexit->key < 0 && !IS_IMMORTAL(ch))
		    { char_puts("It can't be picked.\n", ch); return; }
		if (!IS_SET(pexit->exit_info, EX_LOCKED))
		    { char_puts("It's already unlocked.\n", ch); return; }
		if (IS_SET(pexit->exit_info, EX_PICKPROOF) && !IS_IMMORTAL(ch))
		    { char_puts("You failed.\n", ch); return; }

		REMOVE_BIT(pexit->exit_info, EX_LOCKED);
		char_puts("*Click*\n", ch);
		act("$n picks the $d.", ch, NULL, pexit->keyword, TO_ROOM);
		check_improve(ch, gsn_pick, TRUE, 2);

		/* pick the other side */
		if ((to_room   = pexit->u1.to_room           ) != NULL
		&&   (pexit_rev = to_room->exit[rev_dir[door]]) != NULL
		&&   pexit_rev->u1.to_room == ch->in_room)
			REMOVE_BIT(pexit_rev->exit_info, EX_LOCKED);
	}
}

void do_stand(CHAR_DATA *ch, const char *argument)
{
	OBJ_DATA *obj = NULL;

	if (argument[0] != '\0') {
		if (ch->position == POS_FIGHTING) {
		    char_puts("Maybe you should finish fighting first?\n", ch);
		    return;
		}
		obj = get_obj_list(ch,argument,ch->in_room->contents);
		if (obj == NULL)
		{
		    char_puts("You don't see that here.\n", ch);
		    return;
		}
		if (obj->pIndexData->item_type != ITEM_FURNITURE
		||  (!IS_SET(obj->value[2],STAND_AT)
		&&   !IS_SET(obj->value[2],STAND_ON)
		&&   !IS_SET(obj->value[2],STAND_IN)))
		{
		    char_puts("You can't seem to find a place to stand.\n", ch);
		    return;
		}
		if (ch->on != obj && count_users(obj) >= obj->value[0])
		{
		    act_puts("There's no room to stand on $p.", ch, obj, NULL, TO_ROOM, POS_DEAD);
		    return;
		}
	}
	switch (ch->position) {
	case POS_SLEEPING:
		if (IS_AFFECTED(ch, AFF_SLEEP))
		    { char_puts("You can't wake up!\n", ch); return; }
		
		if (obj == NULL) {
		    char_puts("You wake and stand up.\n", ch);
		    act("$n wakes and stands up.", ch, NULL, NULL, TO_ROOM);
				
		    ch->on = NULL;
		}
		else if (IS_SET(obj->value[2],STAND_AT))
		{
		   act_puts("You wake and stand at $p.",
			    ch, obj, NULL, TO_CHAR, POS_DEAD);
		   act("$n wakes and stands at $p.", ch, obj, NULL, TO_ROOM);
				
		}
		else if (IS_SET(obj->value[2],STAND_ON))
		{
		   act_puts("You wake and stand on $p.",
			    ch, obj, NULL, TO_CHAR, POS_DEAD);
		   act("$n wakes and stands on $p.", ch, obj, NULL, TO_ROOM);
		}
		else 
		{
		   act_puts("You wake and stand in $p.",
			    ch, obj, NULL, TO_CHAR, POS_DEAD);
		   act("$n wakes and stands in $p.", ch, obj, NULL, TO_ROOM);
		}

		if (IS_HARA_KIRI(ch)) {
		 char_puts("You feel your blood heats your body.\n", ch);
		 REMOVE_BIT(ch->act,PLR_HARA_KIRI);
		}

		ch->position = POS_STANDING;
		do_look(ch,"auto");
		break;

	case POS_RESTING: case POS_SITTING:
		if (obj == NULL)
		{
		    char_puts("You stand up.\n", ch);
		    act("$n stands up.", ch, NULL, NULL, TO_ROOM);
		    ch->on = NULL;
		}
		else if (IS_SET(obj->value[2],STAND_AT))
		{
		    act_puts("You stand at $p.", ch, obj, NULL, TO_CHAR, POS_DEAD);
		    act("$n stands at $p.", ch, obj, NULL, TO_ROOM);
		}
		else if (IS_SET(obj->value[2],STAND_ON))
		{
		    act_puts("You stand on $p.", ch, obj, NULL, TO_CHAR, POS_DEAD);
		    act("$n stands on $p.", ch, obj, NULL, TO_ROOM);
		}
		else
		{
		    act_puts("You stand in $p.", ch, obj, NULL, TO_CHAR, POS_DEAD);
		    act("$n stands in $p.", ch, obj, NULL, TO_ROOM);
		}
		ch->position = POS_STANDING;
		break;

	case POS_STANDING:
		char_puts("You are already standing.\n", ch);
		break;

	case POS_FIGHTING:
		char_puts("You are already fighting!\n", ch);
		break;
	}
}

void do_rest(CHAR_DATA *ch, const char *argument)
{
	OBJ_DATA *obj = NULL;

	if (ch->position == POS_FIGHTING) {
		char_puts("You are already fighting!\n", ch);
		return;
	}

	if (MOUNTED(ch)) {
		  char_puts("You can't rest while mounted.\n", ch);
		  return;
	}

	if (RIDDEN(ch)) {
		  char_puts("You can't rest while being ridden.\n", ch);
		  return;
	}

	if (IS_AFFECTED(ch, AFF_SLEEP))
	{ char_puts("You are already sleeping.\n", ch); return; }

	/* okay, now that we know we can rest, find an object to rest on */
	if (argument[0] != '\0') {
		obj = get_obj_list(ch,argument,ch->in_room->contents);
		if (obj == NULL) {
		    char_puts("You don't see that here.\n", ch);
		    return;
		}
	}
	else obj = ch->on;

	if (obj != NULL) {
		if (!IS_SET(obj->pIndexData->item_type,ITEM_FURNITURE) 
		||  (!IS_SET(obj->value[2],REST_ON) &&
		     !IS_SET(obj->value[2],REST_IN) &&
		     !IS_SET(obj->value[2],REST_AT))) {
		    char_puts("You can't rest on that.\n", ch);
		    return;
		}

		if (obj != NULL && ch->on != obj
		&&  count_users(obj) >= obj->value[0]) {
			act_puts("There's no more room on $p.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			return;
		}
		
		ch->on = obj;
	}

	switch (ch->position) {
	case POS_SLEEPING:
		if (obj == NULL) {
		    char_puts("You wake up and start resting.\n", ch);
		    act("$n wakes up and starts resting.",
			ch, NULL, NULL, TO_ROOM);
				
		}
		else if (IS_SET(obj->value[2],REST_AT)) {
			act_puts("You wake up and rest at $p.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			act("$n wakes up and rests at $p.",
			    ch, obj, NULL, TO_ROOM);
		}
		else if (IS_SET(obj->value[2],REST_ON)) {
			act_puts("You wake up and rest on $p.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			act("$n wakes up and rests on $p.",
			    ch, obj, NULL, TO_ROOM);
		}
		else {
			act_puts("You wake up and rest in $p.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			act("$n wakes up and rests in $p.",
			    ch, obj, NULL, TO_ROOM);
		}
		ch->position = POS_RESTING;
		break;

	case POS_RESTING:
		char_puts("You are already resting.\n", ch);
		break;

	case POS_STANDING:
		if (obj == NULL) {
			char_puts("You rest.\n", ch);
			act("$n sits down and rests.",
			    ch, NULL, NULL, TO_ROOM);
		}
		else if (IS_SET(obj->value[2], REST_AT)) {
			act_puts("You sit down at $p and rest.",
			    ch, obj, NULL, TO_CHAR, POS_DEAD);
			act("$n sits down at $p and rests.",
			    ch, obj, NULL, TO_ROOM);
		}
		else if (IS_SET(obj->value[2], REST_ON)) {
			act_puts("You sit down on $p and rest.",
			    ch, obj, NULL, TO_CHAR, POS_DEAD);
			act("$n sits down on $p and rests.",
			    ch, obj, NULL, TO_ROOM);
		}
		else {
			act_puts("You sit down in $p and rest.",
			    ch, obj, NULL, TO_CHAR, POS_DEAD);
			act("$n sits down in $p and rests.",
			    ch, obj, NULL, TO_ROOM);
		}
		ch->position = POS_RESTING;
		break;

	case POS_SITTING:
		if (obj == NULL) {
			char_puts("You rest.\n", ch);
			act("$n rests.", ch, NULL, NULL, TO_ROOM);
		}
		else if (IS_SET(obj->value[2],REST_AT)) {
			act_puts("You rest at $p.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			act("$n rests at $p.", ch, obj, NULL, TO_ROOM);
		} 
		else if (IS_SET(obj->value[2],REST_ON)) {
			act_puts("You rest on $p.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			act("$n rests on $p.", ch, obj, NULL, TO_ROOM);
		}
		else {
			act_puts("You rest in $p.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			act("$n rests in $p.", ch, obj, NULL, TO_ROOM);
		}
		ch->position = POS_RESTING;

		if (IS_HARA_KIRI(ch)) {
			char_puts("You feel your blood heats your body.\n",
				  ch);
			REMOVE_BIT(ch->act, PLR_HARA_KIRI);
		}
		break;
	}
}

void do_sit(CHAR_DATA *ch, const char *argument)
{
	OBJ_DATA *obj = NULL;

	if (ch->position == POS_FIGHTING) {
		char_puts("Maybe you should finish fighting first?\n", ch);
		return;
	}

	if (MOUNTED(ch)) {
		  char_puts("You can't sit while mounted.\n", ch);
		  return;
	}

	if (RIDDEN(ch)) {
		  char_puts("You can't sit while being ridden.\n", ch);
		  return;
	}

	if (IS_AFFECTED(ch, AFF_SLEEP))
	{ char_puts("You are already sleeping.\n", ch); return; }

	/* okay, now that we know we can sit, find an object to sit on */
	if (argument[0] != '\0') {
		obj = get_obj_list(ch,argument,ch->in_room->contents);
		if (obj == NULL) {
			char_puts("You don't see that here.\n", ch);
			return;
		}
	}
	else
		obj = ch->on;

	if (obj != NULL) {
		if (!IS_SET(obj->pIndexData->item_type,ITEM_FURNITURE)
		||  (!IS_SET(obj->value[2],SIT_ON)
		&&   !IS_SET(obj->value[2],SIT_IN)
		&&   !IS_SET(obj->value[2],SIT_AT))) {
			char_puts("You can't sit on that.\n", ch);
			return;
		}

		if (obj != NULL
		&&  ch->on != obj
		&&  count_users(obj) >= obj->value[0]) {
			act_puts("There's no more room on $p.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			return;
		}

		ch->on = obj;
	}

	switch (ch->position) {
	case POS_SLEEPING:
		if (obj == NULL) {
			char_puts("You wake and sit up.\n", ch);
			act("$n wakes and sits up.", ch, NULL, NULL, TO_ROOM);
		}
		else if (IS_SET(obj->value[2],SIT_AT)) {
			act_puts("You wake and sit at $p.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			act("$n wakes and sits at $p.", ch, obj, NULL, TO_ROOM);
		}
		else if (IS_SET(obj->value[2],SIT_ON)) {
			act_puts("You wake and sit on $p.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			act("$n wakes and sits on $p.", ch, obj, NULL, TO_ROOM);
		}
		else {
			act_puts("You wake and sit in $p.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			act("$n wakes and sits in $p.", ch, obj, NULL, TO_ROOM);
		}

		ch->position = POS_SITTING;
		break;

	case POS_RESTING:
		if (obj == NULL)
			char_puts("You stop resting.\n", ch);
		else if (IS_SET(obj->value[2],SIT_AT)) {
			act_puts("You sit at $p.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			act("$n sits at $p.", ch, obj, NULL, TO_ROOM);
		}
		else if (IS_SET(obj->value[2],SIT_ON)) {
			act_puts("You sit on $p.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			act("$n sits on $p.", ch, obj, NULL, TO_ROOM);
		}
		else {
			act_puts("You sit in $p.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			act("$n sits in $p.", ch, obj, NULL, TO_ROOM);
		}
		ch->position = POS_SITTING;
		break;

	case POS_SITTING:
		char_puts("You are already sitting down.\n", ch);
		break;

	case POS_STANDING:
		if (obj == NULL) {
			char_puts("You sit down.\n", ch);
			act("$n sits down on the ground.",
			    ch, NULL, NULL, TO_ROOM);
		}
		else if (IS_SET(obj->value[2], SIT_AT)) {
			act_puts("You sit down at $p.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			act("$n sits down at $p.", ch, obj, NULL, TO_ROOM);
		}
		else if (IS_SET(obj->value[2],SIT_ON)) {
			act_puts("You sit down on $p.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			act("$n sits down on $p.", ch, obj, NULL, TO_ROOM);
		}
		else {
			act_puts("You sit down in $p.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			act("$n sits down in $p.", ch, obj, NULL, TO_ROOM);
		}
		ch->position = POS_SITTING;
		break;
	}

	if (IS_HARA_KIRI(ch)) {
		 char_puts("You feel your blood heats your body.\n", ch);
		 REMOVE_BIT(ch->act,PLR_HARA_KIRI);
	}
}

void do_sleep(CHAR_DATA *ch, const char *argument)
{
	OBJ_DATA *obj = NULL;

	if (MOUNTED(ch)) {
		char_puts("You can't sleep while mounted.\n", ch);
		return;
	}

	if (RIDDEN(ch)) {
		char_puts("You can't sleep while being ridden.\n", ch);
		return;
	}

	switch (ch->position) {
	case POS_SLEEPING:
		char_puts("You are already sleeping.\n", ch);
		break;

	case POS_RESTING:
	case POS_SITTING:
	case POS_STANDING: 
		if (argument[0] == '\0' && ch->on == NULL) {
			char_puts("You go to sleep.\n", ch);
			act("$n goes to sleep.", ch, NULL, NULL, TO_ROOM);
		}
		else { /* find an object and sleep on it */
			if (argument[0] == '\0')
				obj = ch->on;
			else
				obj = get_obj_list(ch, argument,
						   ch->in_room->contents);

			if (obj == NULL) {
				char_puts("You don't see that here.\n", ch);
				return;
			}

			if (obj->pIndexData->item_type != ITEM_FURNITURE
			||  (!IS_SET(obj->value[2], SLEEP_ON) &&
			     !IS_SET(obj->value[2],SLEEP_IN) &&
			     !IS_SET(obj->value[2],SLEEP_AT))) {
				char_puts("You can't sleep on that.\n", ch);
				return;
			}

			if (ch->on != obj
			&&  count_users(obj) >= obj->value[0]) {
				act_puts("There's no room on $p for you.",
					 ch, obj, NULL, TO_CHAR, POS_DEAD);
				return;
			}

			ch->on = obj;
			if (IS_SET(obj->value[2], SLEEP_AT)) {
				act_puts("You go to sleep at $p.",
					 ch, obj, NULL, TO_CHAR, POS_DEAD);
				act("$n goes to sleep at $p.",
				    ch, obj, NULL, TO_ROOM);
			}
			else if (IS_SET(obj->value[2], SLEEP_ON)) {
				act_puts("You go to sleep on $p.",
					 ch, obj, NULL, TO_CHAR, POS_DEAD);
				act("$n goes to sleep on $p.",
				    ch, obj, NULL, TO_ROOM);
			}
			else {
				act_puts("You go to sleep in $p.",
					 ch, obj, NULL, TO_CHAR, POS_DEAD);
				act("$n goes to sleep in $p.",
				    ch, obj, NULL, TO_ROOM);
			}
		}
		ch->position = POS_SLEEPING;
		break;

	case POS_FIGHTING:
		char_puts("You are already fighting.\n", ch);
		break;
	}
}

void do_wake(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	one_argument(argument, arg);
	if (arg[0] == '\0')
		{ do_stand(ch, argument); return; }

	if (!IS_AWAKE(ch))
		{ char_puts("You are asleep yourself!\n", ch); return; }

	if ((victim = get_char_room(ch, arg)) == NULL)
		{ char_puts("They aren't here.\n", ch); return; }

	if (IS_AWAKE(victim)) { 
		act_puts("$N is already awake.",
			 ch, NULL, victim, TO_CHAR, POS_DEAD);
		return;
	}

	if (IS_AFFECTED(victim, AFF_SLEEP)) { 
		act_nprintf(victim, NULL, ch, TO_VICT, POS_DEAD, MSG_YOU_CANT_WAKE_M); 
		return; 
	}

	act_puts("$n wakes you", ch, NULL, victim, TO_VICT, POS_SLEEPING);
	do_stand(victim, str_empty);
	return;
}

void do_sneak(CHAR_DATA *ch, const char *argument)
{
	AFFECT_DATA	af;
	int		chance;

	if ((chance = get_skill(ch, gsn_sneak)) == 0)
		return;

	if (MOUNTED(ch)) {
		  char_puts("You can't sneak while mounted.\n", ch);
		  return;
	}

	char_puts("You attempt to move silently.\n", ch);
	affect_strip(ch, gsn_sneak);

	if (IS_AFFECTED(ch, AFF_SNEAK))
		return;

	if (number_percent() < chance) {
		check_improve(ch, gsn_sneak, TRUE, 3);
		af.where     = TO_AFFECTS;
		af.type      = gsn_sneak;
		af.level     = ch->level; 
		af.duration  = ch->level;
		af.location  = APPLY_NONE;
		af.modifier  = 0;
		af.bitvector = AFF_SNEAK;
		affect_to_char(ch, &af);
	}
	else
		check_improve(ch, gsn_sneak, FALSE, 3);
}

void do_hide(CHAR_DATA *ch, const char *argument)
{
	int chance;
	sflag_t sector;

	if (MOUNTED(ch)) {
		  char_puts("You can't hide while mounted.\n", ch);
		  return;
	}

	if (RIDDEN(ch)) {
		  char_puts("You can't hide while being ridden.\n", ch);
		  return;
	}

	if (IS_AFFECTED(ch, AFF_FAERIE_FIRE) )  {
		char_puts("You cannot hide while glowing.\n", ch);
		return;
	}

	char_puts("You attempt to hide.\n", ch);

	if ((chance = get_skill(ch, gsn_hide)) == 0) {
		char_puts("Huh?\n", ch);
		return;
	}

	sector = ch->in_room->sector_type;
	if (sector == SECT_FOREST
	||  sector == SECT_HILLS
	||  sector == SECT_MOUNTAIN)
		chance += 15;
	else if (sector == SECT_CITY)
		chance -= 15;
	
	if (number_percent() < chance) {
		SET_BIT(ch->affected_by, AFF_HIDE);
		check_improve(ch, gsn_hide, TRUE, 3);
	}
	else  {
		REMOVE_BIT(ch->affected_by, AFF_HIDE);
		check_improve(ch, gsn_hide, FALSE, 3);
	}
}

void do_camouflage(CHAR_DATA *ch, const char *argument)
{
	int		sn;
	int		chance;
	sflag_t		sector;

	if (MOUNTED(ch)) {
		char_puts("You can't camouflage while mounted.\n", ch);
		return;
	}

	if (RIDDEN(ch)) {
		char_puts("You can't camouflage while being ridden.\n", ch);
		return;
	}

	if (IS_AFFECTED(ch, AFF_FAERIE_FIRE))  {
		char_puts("You can't camouflage yourself while glowing.\n", ch);
		return;
	}

	if ((sn = sn_lookup("camouflage")) < 0
	||  (chance = get_skill(ch, sn)) == 0) {
		char_puts("You don't know how to camouflage yourself.\n", ch);
		return;
	}

	sector = ch->in_room->sector_type;
	if (sector != SECT_FOREST
	&&  sector != SECT_HILLS
	&&  sector != SECT_MOUNTAIN) {
		char_puts("There's no cover here.\n", ch);
		act("$n tries to camouflage $mself against the lone leaf on the ground.",
		    ch, NULL, NULL, TO_ROOM);
		return;
	}

	char_puts("You attempt to camouflage yourself.\n", ch);
	WAIT_STATE(ch, SKILL(sn)->beats);

	if (IS_AFFECTED(ch, AFF_CAMOUFLAGE))
		REMOVE_BIT(ch->affected_by, AFF_CAMOUFLAGE);

	if (IS_NPC(ch) || number_percent() < chance) {
		SET_BIT(ch->affected_by, AFF_CAMOUFLAGE);
		check_improve(ch, sn, TRUE, 1);
	}
	else
		check_improve(ch, sn, FALSE, 1);
}

/*
 * Contributed by Alander
 */
void do_visible(CHAR_DATA *ch, const char *argument)
{
	if (IS_AFFECTED(ch, AFF_HIDE | AFF_FADE)) {
		char_nputs(MSG_YOU_STEP_OUT_SHADOWS, ch);
		REMOVE_BIT(ch->affected_by, AFF_HIDE | AFF_FADE);
		act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING,
				MSG_N_STEPS_OUT_OF_SHADOWS);
	}

	if (IS_AFFECTED(ch, AFF_CAMOUFLAGE)) {
		char_nputs(MSG_YOU_STEP_OUT_COVER, ch);
		REMOVE_BIT(ch->affected_by, AFF_CAMOUFLAGE);
		act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING,
				MSG_N_STEPS_OUT_COVER);
	}

	if (IS_AFFECTED(ch, AFF_INVIS | AFF_IMP_INVIS)) {
		char_puts("You fade into existence.\n", ch);
		affect_bit_strip(ch, TO_AFFECTS, AFF_INVIS | AFF_IMP_INVIS);
		REMOVE_BIT(ch->affected_by, AFF_INVIS | AFF_IMP_INVIS);
		act("$n fades into existence.", ch, NULL, NULL, TO_ROOM);
	}
}

void do_recall(CHAR_DATA *ch, const char *argument)
{
	ROOM_INDEX_DATA *location;
	int point;
 
	if (IS_NPC(ch)) {
		char_puts("Only players can recall.\n", ch);
		return;
	}

	if (ch->level >= 10 && !IS_IMMORTAL(ch)) {
		char_puts("Recall is for only levels below 10.\n", ch);
		return;
	}

	if (ch->desc) {
		if (IS_PUMPED(ch)) {
			char_nputs(MSG_TOO_PUMPED_TO_PRAY, ch);
			return;
		}
		point = get_recall(ch);
	}
	else 
		point =	hometown_table[number_range(0, 4)].recall[number_range(0,2)];

	act("$n prays for transportation!", ch, NULL, NULL, TO_ROOM);
	
	if ((location = get_room_index(point))== NULL) {
		char_puts("You are completely lost.\n", ch);
		return;
	}

	if (ch->in_room == location)
		return;

	if (IS_SET(ch->in_room->room_flags, ROOM_NORECALL)
	||  IS_AFFECTED(ch, AFF_CURSE) 
	||  IS_RAFFECTED(ch->in_room, RAFF_CURSE)) {
		char_puts("The gods have forsaken you.\n", ch);
		return;
	}

	ch->move /= 2;
	act("$n disappears.", ch, NULL, NULL, TO_ROOM);
	char_from_room(ch);
	char_to_room(ch, location);
	act("$n appears in the room.", ch, NULL, NULL, TO_ROOM);
	do_look(ch, "auto");
	
	if (ch->pet != NULL) {
		act("$n disappears.", ch->pet, NULL, NULL, TO_ROOM);
 		char_from_room(ch->pet);
		char_to_room(ch->pet, location);
		act("$n appears in the room.", ch->pet, NULL, NULL, TO_ROOM);
		do_look(ch->pet, "auto");
	}
}

void do_train(CHAR_DATA *ch, const char *argument)
{
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *mob;
	int stat = - 1;
	char *pOutput = NULL;

	if (IS_NPC(ch))
		return;

	/*
	 * Check for trainer.
	 */
	for (mob = ch->in_room->people; mob; mob = mob->next_in_room)
		if (IS_NPC(mob)
		&&  (IS_SET(mob->act, ACT_PRACTICE) ||
		     IS_SET(mob->act, ACT_TRAIN) ||
		     IS_SET(mob->act, ACT_GAIN)))
			break;

	if (mob == NULL) {
		char_puts("You can't do that here.\n", ch);
		return;
	}

	if (argument[0] == '\0') {
		char_printf(ch, "You have %d training sessions.\n",
			    ch->train);
		argument = "foo";
	}

	if (!str_cmp(argument, "str")) {
		stat        = STAT_STR;
		pOutput     = "strength";
	}
	else if (!str_cmp(argument, "int")) {
		stat	    = STAT_INT;
		pOutput     = "intelligence";
	}
	else if (!str_cmp(argument, "wis")) {
		stat	    = STAT_WIS;
		pOutput     = "wisdom";
	}
	else if (!str_cmp(argument, "dex")) {
		stat  	    = STAT_DEX;
		pOutput     = "dexterity";
	}
	else if (!str_cmp(argument, "con")) {
		stat	    = STAT_CON;
		pOutput     = "constitution";
	}
	else if (!str_cmp(argument, "cha")) {
		stat	    = STAT_CHA;
		pOutput     = "charisma";
	}
	else {
		strcpy(buf, GETMSG("You can train:", ch->lang));
		if (ch->perm_stat[STAT_STR] < get_max_train(ch,STAT_STR)) 
		    strcat(buf, " str");
		if (ch->perm_stat[STAT_INT] < get_max_train(ch,STAT_INT))  
		    strcat(buf, " int");
		if (ch->perm_stat[STAT_WIS] < get_max_train(ch,STAT_WIS)) 
		    strcat(buf, " wis");
		if (ch->perm_stat[STAT_DEX] < get_max_train(ch,STAT_DEX))  
		    strcat(buf, " dex");
		if (ch->perm_stat[STAT_CON] < get_max_train(ch,STAT_CON))  
		    strcat(buf, " con");
		if (ch->perm_stat[STAT_CHA] < get_max_train(ch,STAT_CHA))  
		    strcat(buf, " cha");

		if (buf[strlen(buf)-1] != ':') {
			strcat(buf, ".\n");
			char_puts(buf, ch);
		}
		else {
			/*
			 * This message dedicated to Jordan ... you big stud!
			 */
			act("You have nothing left to train, you $T!",
			    ch, NULL,
			    ch->sex == SEX_MALE   ? "big stud" :
			    ch->sex == SEX_FEMALE ? "hot babe" :
						    "wild thing",
			    TO_CHAR);
		}
		return;
	}

	if (ch->perm_stat[stat] >= get_max_train(ch,stat)) {
		act_puts("Your $T is already at maximum.",
			 ch, NULL, pOutput, TO_CHAR, POS_DEAD);
		return;
	}

	if (ch->train < 1) {
		char_puts("You don't have enough training sessions.\n", ch);
		return;
	}

	ch->train--;
	ch->perm_stat[stat] += 1;
	act_puts("Your $T increases!", ch, NULL, pOutput, TO_CHAR, POS_DEAD);
	act("$n's $T increases!", ch, NULL, pOutput, TO_ROOM);
}

void do_track(CHAR_DATA *ch, const char *argument)
{
	ROOM_HISTORY_DATA *rh;
	EXIT_DATA *pexit;
	static char *door[] = { "north","east","south","west","up","down",
		                      "that way" };
	int d;
	int chance;

	if ((chance = get_skill(ch, gsn_track)) == 0) {
		char_puts("There are no train tracks here.\n", ch);
		return;
	}

	WAIT_STATE(ch, SKILL(gsn_track)->beats);
	act("$n checks the ground for tracks.", ch, NULL, NULL, TO_ROOM);

	if (number_percent() < chance) {
		if (IS_NPC(ch)) {
			ch->status = 0;
			if (ch->last_fought != NULL
			&&  !IS_SET(ch->act, ACT_NOTRACK))
				add_mind(ch, ch->last_fought->name);
		}

		for (rh = ch->in_room->history; rh != NULL; rh = rh->next)
			if (is_name(argument, rh->name)) {
				check_improve(ch, gsn_track, TRUE, 1);
			if ((d = rh->went) == -1)
				continue;
			char_printf(ch, "%s's tracks lead %s.\n",
				     rh->name, door[d]);
			if ((pexit = ch->in_room->exit[d]) != NULL
			&&  IS_SET(pexit->exit_info, EX_ISDOOR)
			&&  pexit->keyword != NULL)
				doprintf(do_open, ch, "%s", door[d]);
			move_char(ch, rh->went, FALSE);
			return;
		}
	}

	char_puts("You don't see any tracks.\n", ch);

	if (IS_NPC(ch))
		ch->status = 5; /* for stalker */

	check_improve(ch, gsn_track, FALSE, 1);
}

void do_vampire(CHAR_DATA *ch, const char *argument)
{
	AFFECT_DATA af;
	int level, duration;
	int chance;
 
	if (is_affected(ch, gsn_vampire)) {
		char_puts("But you are already vampire. Kill them! Kill them!\n", ch);
		return;
	}

	if ((chance = get_skill(ch, gsn_vampire)) == 0) {
		char_puts("You try to show yourself even more ugly.\n", ch);
		return;
	}

	if (chance < 100) {
		char_puts("Go and ask the questor. He'll help you.\n", ch);
		return;
	}

	if (weather_info.sunlight == SUN_LIGHT 
	||  weather_info.sunlight == SUN_RISE) {
		char_puts("You should wait for the evening or night to transform to a vampire.\n", ch);
		return;
	}

	level = ch->level;
	duration = level / 10 + 5;

	af.type      = gsn_vampire;
	af.level     = level;
	af.duration  = duration;

/* negative immunity */
	af.where = TO_IMMUNE;
	af.location = APPLY_NONE;
	af.modifier = 0;
	af.bitvector = IMM_NEGATIVE;
	affect_to_char(ch, &af);

/* haste */
	af.where     = TO_AFFECTS;
	af.location  = APPLY_DEX;
	af.modifier  = 1 + (level /20);
	af.bitvector = AFF_HASTE;
	affect_to_char(ch, &af);

/* giant strength + infrared */
	af.location  = APPLY_STR;
	af.modifier  = 1 + (level / 20);
	af.bitvector = 0;
	affect_to_char(ch, &af);

/* size */
	af.location  = APPLY_SIZE;
	af.modifier  = 1 + (level / 50);
	affect_to_char(ch, &af);

/* damroll */
	af.where     = TO_AFFECTS;
	af.location  = APPLY_DAMROLL;
	af.modifier  = ch->damroll;
	af.bitvector = AFF_BERSERK;
	affect_to_char(ch, &af);

/* flying, infrared */
	af.where     = TO_AFFECTS;
	af.location  = 0;
	af.modifier  = 0;
	af.bitvector = AFF_SNEAK | AFF_FLYING | AFF_INFRARED;
	affect_to_char(ch, &af);

	char_puts("You feel yourself getting greater and greater.\n", ch);
	act("You cannot recognize $n anymore.", ch, NULL, NULL, TO_ROOM);
}

void do_vbite(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int chance;

	one_argument(argument, arg);

	if ((chance = get_skill(ch, gsn_vampiric_bite)) == 0) {
		char_puts("You don't know how to bite creatures.\n", ch);
		return;
	}

	if (!is_affected(ch, gsn_vampire)) {
		char_puts("You must transform vampire before biting.\n", ch);
		return;
	}

	if (arg[0] == '\0') {
		char_puts("Bite whom?\n", ch);
		return;
	}

	WAIT_STATE(ch, SKILL(gsn_vampiric_bite)->beats);

	if ((victim = get_char_room(ch, arg)) == NULL) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	if (victim->position != POS_SLEEPING) {
		char_puts("They must be sleeping.\n", ch);
		return;
	}

	if (IS_NPC(ch) && !IS_NPC(victim))
		return;

	if (victim == ch) {
		char_puts("How can you sneak upon yourself?\n", ch);
		return;
	}

	if (victim->fighting != NULL) {
		char_puts("You can't bite a fighting person.\n", ch);
		return;
	}

	if (is_safe(ch, victim))
		return;

	if (victim->hit < (8 * victim->max_hit / 10) && (IS_AWAKE(victim))) {
		act_puts("$N is hurt and suspicious ... doesn't worth up.",
			 ch, NULL, victim, TO_CHAR, POS_DEAD);
		return;
	}

	if (current_time-victim->last_fight_time<300 && IS_AWAKE(victim)) {
		act_puts("$N is hurt and suspicious ... doesn't worth to do.",
			 ch, NULL, victim, TO_CHAR, POS_DEAD);
		return;       
	}

	if (!IS_AWAKE(victim)
	&&  (IS_NPC(ch) ||
	     number_percent() < ((chance * 7 / 10) +
		(2 * (ch->level - victim->level)) ))) {
		check_improve(ch,gsn_vampiric_bite,TRUE,1);
		one_hit(ch, victim, gsn_vampiric_bite, WEAR_WIELD);
	}
	else {
		check_improve(ch, gsn_vampiric_bite, FALSE, 1);
		damage(ch, victim, 0, gsn_vampiric_bite, DAM_NONE, DAMF_SHOW);
	}
}

void do_bash_door(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *gch;
	int chance;
	int damage_bash,door;
	int beats;

	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	one_argument(argument,arg);
 
	if ((chance = get_skill(ch, gsn_bash_door)) == 0) {
		char_puts("Bashing? What's that?\n", ch);
		return;
	}
 
	if (MOUNTED(ch)) {
        	char_puts("You can't bash doors while mounted.\n", ch);
		return;
	}

	if (RIDDEN(ch)) {
		char_puts("You can't bash doors while being ridden.\n", ch);
		return;
	}

	if (arg[0] == '\0') {
		char_puts("Bash wich door or direction?\n", ch);
		return;
	}

	if (ch->fighting) {	
		char_puts("Wait until the fight finishes.\n", ch);
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
		char_puts("It's already open.\n", ch);
		return;
	}

	if (!IS_SET(pexit->exit_info, EX_LOCKED)) {
		char_puts("Just try to open it.\n", ch);
		return;
	}

	if (IS_SET(pexit->exit_info, EX_NOPASS)) {
		char_puts("A mystical shield protects exit.\n", ch); 
		return;
	}

	chance -= 90;

	/* modifiers */

	/* size and weight */
	chance += get_carry_weight(ch) / 100;
	chance += (ch->size - 2) * 20;

	/* stats */
	chance += get_curr_stat(ch, STAT_STR);

	if (IS_AFFECTED(ch,AFF_FLYING))
		chance -= 10;

	act_puts("You slam into $d, and try to break $d!",
		 ch, NULL, pexit->keyword, TO_CHAR, POS_DEAD);
	act("$n slams into $d, and tries to break it!",
	    ch, NULL, pexit->keyword, TO_ROOM);

	if (room_dark(ch->in_room))
		chance /= 2;

	beats = SKILL(gsn_bash_door)->beats;
	/* now the attack */
	if (number_percent() < chance) {
		check_improve(ch, gsn_bash_door, TRUE, 1);

		REMOVE_BIT(pexit->exit_info, EX_LOCKED);
		REMOVE_BIT(pexit->exit_info, EX_CLOSED);
		act("$n bashes the $d and breaks the lock.",
		    ch, NULL, pexit->keyword, TO_ROOM);
		char_nputs(MSG_YOU_SUCCESSED_TO_OPEN_DOOR, ch);

/* open the other side */
		if ((to_room = pexit->u1.to_room) != NULL
		&&  (pexit_rev = to_room->exit[rev_dir[door]]) != NULL
		&&  pexit_rev->u1.to_room == ch->in_room) {
			ROOM_INDEX_DATA *in_room;

			REMOVE_BIT(pexit_rev->exit_info, EX_CLOSED);
			REMOVE_BIT(pexit_rev->exit_info, EX_LOCKED);

			in_room = ch->in_room;
			ch->in_room = to_room;
			act("$n bashes the $d and breaks the lock.",
			    ch, NULL, pexit->keyword, TO_ROOM);
			ch->in_room = in_room;
		}

		check_improve(ch, gsn_bash_door, TRUE, 1);
		WAIT_STATE(ch, beats);
	}
	else {
		char_nputs(MSG_YOU_FALL_ON_FACE, ch);
		act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING,
			    MSG_N_FALLS_ON_FACE);
		check_improve(ch, gsn_bash_door, FALSE, 1);
		ch->position = POS_RESTING;
		WAIT_STATE(ch, beats * 3 / 2); 
		damage_bash = ch->damroll +
			      number_range(4,4 + 4* ch->size + chance/5);
		damage(ch, ch, damage_bash, gsn_bash_door, DAM_BASH, DAMF_SHOW);
	}
}

void do_blink(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];

	if (get_skill(ch, gsn_blink) == 0) {
		char_puts("Huh?\n", ch);
		return;
	}

	argument = one_argument(argument, arg);

	if (arg[0] == '\0') {
		char_printf(ch, "Your current blink status: %s.\n",
			    IS_SET(ch->act, PLR_BLINK) ? "ON" : "OFF");
		return;
	}

	if (!str_cmp(arg, "ON")) {
		SET_BIT(ch->act, PLR_BLINK);
		char_puts("Now, your current blink status is ON.\n", ch);
		return;
	}

	if (!str_cmp(arg, "OFF")) {
		REMOVE_BIT(ch->act, PLR_BLINK);
		char_puts("Now, your current blink status is OFF.\n", ch);
		return;
	}

	char_printf(ch, "What's that? Is %s a status?\n", arg);
}

void do_vanish(CHAR_DATA *ch, const char *argument)
{
	ROOM_INDEX_DATA *pRoomIndex;
	int chance;
	int sn;

	if ((sn = sn_lookup("vanish")) < 0
	||  (chance = get_skill(ch, sn)) == 0) {
		char_puts("Huh?\n", ch);
		return;
	}

	if (ch->mana < 25) {
		char_puts("You don't have enough power.\n", ch);
		return;
	}
	ch->mana -= 25;
	WAIT_STATE(ch, SKILL(sn)->beats);

	if (number_percent() > chance) {
		char_puts("You failed.\n", ch);
		check_improve(ch, sn, FALSE, 1);
		return;
	}

	if (ch->in_room == NULL
	||  IS_SET(ch->in_room->room_flags, ROOM_NORECALL)) {
		char_puts("You failed.\n", ch);
		return;
	}

	for (; ;) {
		if ((pRoomIndex = get_room_index(number_range(0, 65535))) == NULL)
			continue;
		if ((ch->in_room->vnum < 500 || ch->in_room->vnum > 600)
		&&  (pRoomIndex->vnum > 500 && pRoomIndex->vnum < 600))
			continue;
		if (can_see_room(ch,pRoomIndex) 
		&&  !room_is_private(pRoomIndex)
		&&  ch->in_room->area == pRoomIndex->area)
			break;
	}

	act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING, MSG_N_THROWS_GLOBE);
	check_improve(ch, sn, TRUE, 1);

  	if (!IS_NPC(ch) && ch->fighting != NULL && number_bits(1) == 1) {
		char_puts("You failed.\n", ch);
		return;
	}

	act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING, MSG_N_IS_GONE);

	char_from_room(ch);
	char_to_room(ch, pRoomIndex);
	act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING,
		    MSG_N_APPEARS_FROM_NOWHERE);
	do_look(ch, "auto");
	stop_fighting(ch, TRUE);
}

void do_fade(CHAR_DATA *ch, const char *argument)
{
	int chance;
	if ((chance=get_skill(ch, gsn_fade)) == 0)
		return;

	if (MOUNTED(ch)) {
		  char_puts("You can't fade while mounted.\n", ch);
		  return;
	}

	if (RIDDEN(ch)) {
		  char_puts("You can't fade while being ridden.\n", ch);
		  return;
	}

	char_puts("You attempt to fade.\n", ch);
	if (number_percent()<=chance) {
		SET_BIT(ch->affected_by, AFF_FADE);
		check_improve(ch, gsn_fade, TRUE, 3);
	}
	else check_improve(ch, gsn_fade, FALSE, 3);
}

void do_vtouch(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *victim;
	AFFECT_DATA af;
	int chance;
	int sn;

	if ((sn = sn_lookup("vampiric touch")) < 0
	||  (chance = get_skill(ch, sn)) == 0) {
		char_puts("You lack the skill to draining touch.\n", ch);
		return;
	}

	if (!is_affected(ch, gsn_vampire)) {
		char_puts("Let it be.\n", ch);
		return;
	}

	WAIT_STATE(ch, SKILL(sn)->beats);

	if (IS_AFFECTED(ch, AFF_CHARM))  {
		char_puts("You don't want to drain your master.\n", ch);
		return;
	} 

	if ((victim = get_char_room(ch,argument)) == NULL) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	if (ch == victim) {
		char_puts("Even you are not so stupid.\n", ch);
		return;
	}

	if (is_affected(victim, sn))
		return;

	if (is_safe(ch,victim))
		return;

	SET_FIGHT_TIME(victim);
	SET_FIGHT_TIME(ch);

	if (number_percent() < 0.85 * chance
	&&  !IS_CLAN_GUARD(victim)
	&&  !IS_IMMORTAL(victim)) {
		act_nprintf(victim, NULL, ch, TO_VICT, POS_DEAD, 
				MSG_YOU_TOUCH_NS_NECK);
		act("$N deadly touches your neck and puts you to nightmares.",
		    victim, NULL, ch, TO_CHAR);
		act_nprintf(victim, NULL, ch, TO_NOTVICT, POS_RESTING,
				MSG_N_TOUCHES_NS_NECK);
		check_improve(ch, sn, TRUE, 1);
		
		af.type = sn;
		af.where = TO_AFFECTS;
		af.level = ch->level;
		af.duration = ch->level / 20 + 1;
		af.location = APPLY_NONE;
		af.modifier = 0;
		af.bitvector = AFF_SLEEP;
		affect_join(victim,&af);

		if (IS_AWAKE(victim))
			victim->position = POS_SLEEPING;
	} else {
		damage(ch, victim, 0, sn, DAM_NONE, DAMF_SHOW);
		check_improve(ch, sn, FALSE, 1);
	}
}

void do_fly(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];

	if (IS_NPC(ch))
		return;

	argument = one_argument(argument,arg);

	if (!str_cmp(arg,"up")) {
		RACE_DATA *r;

		if (is_affected(ch, gsn_thumbling)) {
			char_puts("Stop jumping like a crazy rabbit first.\n",
				  ch);
			return;
		}

		if (IS_AFFECTED(ch, AFF_FLYING)) {		       
			char_puts("You are already flying.\n", ch); 
			return;
		}

		if (is_bit_affected(ch, TO_AFFECTS, AFF_FLYING)
		||  ((r = race_lookup(ch->race)) && (r->aff & AFF_FLYING))
		||  has_obj_affect(ch, AFF_FLYING)) {
			SET_BIT(ch->affected_by, AFF_FLYING);
			char_puts("You start to fly.\n", ch);
		}
		else {
			char_puts("To fly find potion or wings.\n", ch); 
			return;
		}
	}
	else if (!str_cmp(arg,"down")) {
		if (IS_AFFECTED(ch,AFF_FLYING)) {
			REMOVE_BIT(ch->affected_by, AFF_FLYING);
			char_puts("You slowly touch the ground.\n", ch);
		}
		else {		       
			char_puts("You are already on the ground.\n", ch); 
			return;
		}
	}
 	else {
		char_puts("Type fly with 'up' or 'down'.\n", ch);
		return;
	}

	WAIT_STATE(ch, SKILL(gsn_fly)->beats);   
}
		 
void do_push(CHAR_DATA *ch, const char *argument)
{
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	EXIT_DATA *pexit;
	int percent;
	int door;
	int sn;

	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (arg1[0] == '\0' || arg2[0] == '\0') {
		char_puts("Push whom to what direction?\n", ch);
		return;
	}

	if (MOUNTED(ch)) {
		char_puts("You can't push while mounted.\n", ch);
		return;
	}

	if (RIDDEN(ch)) {
		char_puts("You can't push while being ridden.\n", ch);
		return;
	}

	if (IS_NPC(ch) && IS_SET(ch->affected_by, AFF_CHARM) 
		&& (ch->master != NULL)) {
		char_puts("You are too dazed to push anyone.\n", ch);
		return;
	}

	if ((sn = sn_lookup("push")) < 0)
		return;

	WAIT_STATE(ch, SKILL(sn)->beats);

	if ((victim = get_char_room(ch, arg1)) == NULL) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	if (!IS_NPC(victim) && victim->desc == NULL) {
		char_puts("You can't do that.\n", ch);
		return;
	}

	if (victim == ch) {
		char_puts("That's pointless.\n", ch);
		return;
	}

	if (victim->position == POS_FIGHTING) {
		char_puts("Wait until the fight finishes.\n", ch);
		return;
	}

	if ((door = find_exit(ch, arg2)) < 0)
		return;

	if ((pexit = ch->in_room->exit[door])
	&&  IS_SET(pexit->exit_info, EX_ISDOOR)) {
		if (IS_SET(pexit->exit_info, EX_CLOSED)) {
			char_puts("The door is closed.\n", ch); 
			return;
		}
		if (IS_SET(pexit->exit_info, EX_LOCKED)) {
			char_puts("The door is locked.\n", ch); 
			return;
		}
	}

	if (IS_AFFECTED(ch, AFF_DETECT_WEB)) {
		char_puts("You're webbed, and want to do WHAT?!?\n", ch);
		act("$n stupidly tries to push $N while webbed.",
		    ch, NULL, victim, TO_ROOM);
		return; 
	}

	if (IS_AFFECTED(victim, AFF_DETECT_WEB)) {
		act_nprintf(victim, NULL, ch, TO_VICT, POS_DEAD, 
				MSG_PUSH_VICT_WEBBED);
		act_nprintf(victim, NULL, ch, TO_NOTVICT, POS_RESTING,
				MSG_N_PUSHES_VICT_WEBBED);
		return; 
	}

	if (is_safe(ch,victim))
		return;

	percent  = number_percent() + (IS_AWAKE(victim) ? 10 : -50);
	percent += can_see(victim, ch) ? -10 : 0;

	if (victim->position == POS_FIGHTING
	||  (IS_NPC(victim) && IS_SET(victim->act, ACT_NOTRACK))
	||  (!IS_NPC(ch) && percent > get_skill(ch, sn))) {
		/*
		 * Failure.
		 */

		char_puts("Oops.\n", ch);
		if (!IS_AFFECTED(victim, AFF_SLEEP)) {
			victim->position = victim->position == POS_SLEEPING ? 
					   POS_STANDING : victim->position;
			act_nprintf(ch, NULL, victim, TO_VICT, POS_RESTING,
				    MSG_N_TRIED_PUSH_YOU);
		}
		act_nprintf(ch, NULL, victim, TO_NOTVICT, POS_RESTING, 
				MSG_N_TRIED_PUSH_N);

		if (IS_AWAKE(victim))
			doprintf(do_yell, victim,
				 "Keep your hands out of me, %s!",
				 ch->name);
		if (!IS_NPC(ch) && IS_NPC(victim)) {
			check_improve(ch, sn, FALSE, 2);
			multi_hit(victim, ch, TYPE_UNDEFINED);
		}
		return;
	}

	act_printf(ch, NULL, victim, TO_CHAR, POS_SLEEPING,
		   "You push $N to %s.", dir_name[door]);
	act_printf(ch, NULL, victim, TO_VICT, POS_SLEEPING,
		   "$n pushes you to %s.", dir_name[door]);
	act_printf(ch, NULL, victim, TO_NOTVICT, POS_SLEEPING,
		   "$n pushes $N to %s.", dir_name[door]);
	move_char(victim, door, FALSE);

	check_improve(ch, sn, TRUE, 1);
}

void do_crecall(CHAR_DATA *ch, const char *argument)
{
	ROOM_INDEX_DATA *location;
	CLAN_DATA *clan;
	AFFECT_DATA af;
	int sn;

	if ((sn = sn_lookup("clan recall")) < 0
	||  get_skill(ch, sn) == 0
	||  (clan = clan_lookup(ch->clan)) == NULL) {
		char_puts("Huh?\n", ch);
		return;
	}

	if (is_affected(ch, sn)) {
		char_nputs(MSG_CANT_PRAY_NOW, ch);
		return;
	}

	if (ch->desc && IS_PUMPED(ch)) {
		char_nputs(MSG_TOO_PUMPED_TO_PRAY, ch);
		return;
	}

	act(clan->msg_prays, ch, NULL, NULL, TO_ROOM);
	
	if ((location = get_room_index(clan->recall_vnum)) == NULL) {
		char_puts("You are completely lost.\n", ch);
		return;
	}

	if (ch->in_room == location)
		return;

	if (IS_SET(ch->in_room->room_flags, ROOM_NORECALL)
	||  IS_AFFECTED(ch, AFF_CURSE) 
	||  IS_RAFFECTED(ch->in_room, RAFF_CURSE)) {
		char_puts("The gods have forsaken you.\n", ch);
		return;
	}

	ch->move /= 2;
	act(clan->msg_vanishes, ch, NULL, NULL, TO_ROOM);
	char_from_room(ch);
	char_to_room(ch, location);
	act("$n appears in the room.", ch, NULL, NULL, TO_ROOM);
	do_look(ch, "auto");
	
	if (ch->pet != NULL) {
	 	char_from_room(ch->pet);
		char_to_room(ch->pet, location);
		do_look(ch->pet, "auto");
	}

	af.type      = sn;
	af.level     = ch->level;
	af.duration  = SKILL(sn)->beats;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = 0;
	affect_to_char(ch, &af);
}

void do_escape(CHAR_DATA *ch, const char *argument)
{
	ROOM_INDEX_DATA *was_in;
	ROOM_INDEX_DATA *now_in;
	EXIT_DATA *pexit;
	CHAR_DATA *victim;
	char arg[MAX_INPUT_LENGTH];
	int door;
	int chance;
	int sn;

	if ((victim = ch->fighting) == NULL) {
		if (ch->position == POS_FIGHTING)
			ch->position = POS_STANDING;
		char_puts("You aren't fighting anyone.\n", ch);
		return;
	}

	argument = one_argument(argument, arg);

	if (arg[0] == '\0') {
		char_puts("Escape to what direction?\n", ch);
		return;
	}

	if (MOUNTED(ch)) {
		  char_puts("You can't escape while mounted.\n", ch);
		  return;
	}

	if (RIDDEN(ch)) {
		  char_puts("You can't escape while being ridden.\n", ch);
		  return;
	}

	if ((sn = sn_lookup("escape")) < 0
	||  (chance = get_skill(ch, sn)) == 0) {
		char_puts("Try flee. It may fit better to you.\n", ch);
		return;
	}

	was_in = ch->in_room;

	if ((door = find_exit(ch, arg)) < 0) {
		char_puts("PANIC! You couldn't escape!\n", ch);
		return;
	}

	if ((pexit = was_in->exit[door]) == 0
	||  pexit->u1.to_room == NULL
	||  (IS_SET(pexit->exit_info, EX_CLOSED) &&
	    (!IS_AFFECTED(ch, AFF_PASS_DOOR) 
	||  IS_SET(pexit->exit_info, EX_NOPASS)) &&
	    !IS_TRUSTED(ch, ANGEL))
	||  (IS_SET(pexit->exit_info, EX_NOFLEE))
	||  (IS_NPC(ch) &&
	     IS_SET(pexit->u1.to_room->room_flags, ROOM_NOMOB))) {
		char_puts("Something prevents you to escape that direction.\n", ch); 
		return;
	}

	if (number_percent() > chance) {
		char_nputs(MSG_ESCAPE_FAILED, ch);
		check_improve(ch, sn, FALSE, 1);	
		return;
	}

	check_improve(ch, sn, TRUE, 1);	
	move_char(ch, door, FALSE);
	if ((now_in = ch->in_room) == was_in) {
		char_puts("It's pointless to escape there.\n", ch);
		return;
	}

	ch->in_room = was_in;
	act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING, MSG_N_ESCAPED);
	ch->in_room = now_in;

	if (!IS_NPC(ch)) {
		char_nputs(MSG_YOU_ESCAPED_FROM_COMBAT, ch);
		if (ch->level < LEVEL_HERO) {
			char_printf(ch, "You lose %d exps.\n", 10);
			gain_exp(ch, -10);
		}
	}
	else
		/* Once fled, the mob will not go after */
		ch->last_fought = NULL;

	stop_fighting(ch, TRUE);
}

void do_layhands(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *victim;
	AFFECT_DATA af;
	int sn;

	if ((sn = sn_lookup("lay hands")) < 0
	||  get_skill(ch, sn) == 0) {
		char_puts("You lack the skill to heal others with touching.\n", ch);
		return;
	}

	WAIT_STATE(ch, SKILL(sn)->beats);

	if ((victim = get_char_room(ch,argument)) == NULL) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	if (is_affected(ch, sn)) {
		 char_puts("You can't concentrate enough.\n", ch);
		 return;
	}

	af.type = sn;
	af.where = TO_AFFECTS;
	af.level = ch->level;
	af.duration = 2;
	af.location = APPLY_NONE;
	af.modifier = 0;
	af.bitvector = 0;
	affect_to_char (ch, &af);

	victim->hit = UMIN(victim->hit + ch->level * 2, victim->max_hit);
	update_pos(victim);
	char_puts("A warm feeling fills your body.\n", victim);
	if (ch != victim)
		char_puts("Ok.\n", ch);
	check_improve(ch, sn, TRUE, 1);
}

int mount_success(CHAR_DATA *ch, CHAR_DATA *mount, int canattack)
{
	int	percent;
	int	success;
	int	chance;

	if ((chance = get_skill(ch, gsn_riding)) == 0)
		return FALSE;

	percent = number_percent() + (ch->level < mount->level ? 
		  (mount->level - ch->level) * 3 : 
		  (mount->level - ch->level) * 2);

	if (!ch->fighting)
		percent -= 25;

	if (!IS_NPC(ch) && IS_DRUNK(ch)) {
		percent += chance / 2;
		char_puts("Due to your being under the influence, riding seems a bit harder...\n", ch);
	}

	success = percent - chance;

	if (success <= 0) { /* Success */
		check_improve(ch, gsn_riding, TRUE, 1);
		return TRUE;
	}

	check_improve(ch, gsn_riding, FALSE, 1);
	if (success >= 10 && MOUNTED(ch) == mount) {
		act_nprintf(ch, NULL, mount, TO_CHAR, POS_DEAD, 
				MSG_YOU_FALL_OFF_N);
		act_nprintf(ch, NULL, mount, TO_NOTVICT, POS_RESTING, 
				MSG_N_FALLS_OFF_N);
		act_nprintf(ch, NULL, mount, TO_VICT, POS_SLEEPING, 
				MSG_N_FALLS_OFF_YOU);

		ch->riding = FALSE;
		mount->riding = FALSE;
		if (ch->position > POS_STUNNED) 
			ch->position=POS_SITTING;
	
		ch->hit -= 5;
		update_pos(ch);
	}
	if (success >= 40 && canattack) {
		act_puts("$N doesn't like the way you've been treating $M.",
			 ch, NULL, mount, TO_CHAR, POS_DEAD);
		act("$N doesn't like the way $n has been treating $M.",
		    ch, NULL, mount, TO_NOTVICT);
		act_puts("You don't like the way $n has been treating you.",
			 ch, NULL, mount, TO_VICT, POS_SLEEPING);

		act_nprintf(mount, NULL, ch, TO_VICT, POS_DEAD, 
				MSG_N_SNARLS_YOU);
		act_nprintf(mount, NULL, ch, TO_NOTVICT, POS_RESTING,
				MSG_N_SNARLS_N);
		act_nprintf(mount, NULL, ch, TO_CHAR, POS_SLEEPING,
				MSG_YOU_SNARL_N);  

		damage(mount, ch, number_range(1, mount->level),
			gsn_kick, DAM_BASH, DAMF_SHOW);
	}
	return FALSE;
}

/*
 * It is not finished yet to implement all.
 */
void do_mount(CHAR_DATA *ch, const char *argument)
{
	char 		arg[MAX_INPUT_LENGTH];
	CHAR_DATA *	mount;

	argument = one_argument(argument, arg);

	if (arg[0] == '\0') {
		if (ch->mount && ch->mount->in_room == ch->in_room)
			mount = ch->mount;
		else {
			char_puts("Mount what?\n", ch);
			return;
		}
	}
	else if ((mount = get_char_room(ch, arg)) == NULL) {
		char_puts("You don't see that here.\n", ch);
		return;
  	}
 
	if (!IS_NPC(mount) || !IS_SET(mount->act, ACT_RIDEABLE)
	||  IS_SET(mount->act, ACT_NOTRACK)) { 
		char_puts("You can't ride that.\n", ch); 
		return;
	}
  
	if (mount->level - 5 > ch->level) {
		char_puts("That beast is too powerful for you to ride.\n", ch);
		return;
	}

	if ((mount->mount) && (!mount->riding) && (mount->mount != ch)) {
		char_printf(ch, "%s belongs to %s, not you.\n",
			    mlstr_cval(mount->short_descr, ch),
		            mount->mount->name);
		return;
	} 

	if (mount->position < POS_STANDING) {
		char_puts("Your mount must be standing.\n", ch);
		return;
	}

	if (RIDDEN(mount)) {
		char_puts("This beast is already ridden.\n", ch);
		return;
	} else if (MOUNTED(ch)) {
		char_puts("You are already riding.\n", ch);
		return;
	}

	if(!mount_success(ch, mount, TRUE)) {
		char_nputs(MSG_FAIL_TO_MOUNT, ch);  
		return; 
	}

	act_puts("You hop on $N's back.", ch, NULL, mount, TO_CHAR, POS_DEAD);
	act("$n hops on $N's back.", ch, NULL, mount, TO_NOTVICT);
	act_puts("$n hops on your back!", ch, NULL, mount, TO_VICT, POS_SLEEPING);
 
	ch->mount = mount;
	ch->riding = TRUE;
	mount->mount = ch;
	mount->riding = TRUE;
  
	affect_bit_strip(ch, TO_AFFECTS, AFF_INVIS | AFF_IMP_INVIS | AFF_SNEAK);
	REMOVE_BIT(ch->affected_by, AFF_HIDE | AFF_FADE | AFF_CAMOUFLAGE |
				    AFF_INVIS | AFF_IMP_INVIS | AFF_SNEAK);
}

void do_dismount(CHAR_DATA *ch, const char *argument)
{
	struct char_data *mount;

	if (MOUNTED(ch)) {
		mount = MOUNTED(ch);

		act_puts("You dismount from $N.",
			 ch, NULL, mount, TO_CHAR, POS_DEAD); 
		act("$n dismounts from $N.", ch, NULL, mount, TO_NOTVICT);
		act_puts("$n dismounts from you.",
			 ch, NULL, mount, TO_VICT, POS_SLEEPING);

		ch->riding = FALSE;
		mount->riding = FALSE;
	}
	else {
		char_puts("You aren't mounted.\n", ch);
		return;
	}
} 

int send_arrow(CHAR_DATA *ch, CHAR_DATA *victim,OBJ_DATA *arrow, 
	       int door, int chance ,int bonus) 
{
	EXIT_DATA *pExit;
	ROOM_INDEX_DATA *dest_room;
	AFFECT_DATA *paf;
	int damroll = 0, hitroll = 0, sn;
	AFFECT_DATA af;

	if (arrow->value[0] == WEAPON_SPEAR)  
		sn = gsn_spear;
	else 
		sn = gsn_arrow;

	for (paf = arrow->affected; paf != NULL; paf = paf->next) {
		if (paf->location == APPLY_DAMROLL)
			damroll += paf->modifier;
		if (paf->location == APPLY_HITROLL)
			hitroll += paf->modifier;
	}

	dest_room = ch->in_room;
	chance += (hitroll + str_app[get_curr_stat(ch,STAT_STR)].tohit
		   + (get_curr_stat(ch,STAT_DEX) - 18)) * 2;
	damroll *= 10;
	while (1) {
		chance -= 10;
		if (victim->in_room == dest_room) {
			if (number_percent() < chance) { 
				if (check_obj_dodge(ch, victim, arrow, chance))
					return 0;
				act("$p strikes you!",
				    victim, arrow, NULL, TO_CHAR);
				act_puts("Your $p strikes $N!",
					 ch, arrow, victim, TO_CHAR, POS_DEAD);
				if (ch->in_room == victim->in_room)
					act("$n's $p strikes $N!",
					    ch, arrow, victim, TO_NOTVICT);
				else {
					act("$n's $p strikes $N!",
					    ch, arrow, victim, TO_ROOM);
					act("$p strikes $n!",
					    victim, arrow, NULL, TO_ROOM);
				}
				if (is_safe(ch, victim) || (IS_NPC(victim) 
				    && IS_SET(victim->act, ACT_NOTRACK))) {
					act("$p falls from $n doing no visible damage...",
					    victim, arrow, NULL, TO_ALL);
					act("$p falls from $n doing no visible damage...",
					    ch, arrow, NULL, TO_CHAR);
					obj_to_room(arrow, victim->in_room);
				}
				else {
					int dam;

					dam = dice(arrow->value[1],
						   arrow->value[2]);
					dam = number_range(dam, 2 * dam);
					dam += damroll + bonus + (10 * str_app[get_curr_stat(ch, STAT_STR)].todam);
					if (IS_WEAPON_STAT(arrow,
							   WEAPON_POISON)) {
						int level;
						AFFECT_DATA *poison, af;

		      	 if ((poison = affect_find(arrow->affected,gsn_poison)) == NULL)
		          	level = arrow->level;
		      	 else
		          	level = poison->level;
		      	 if (!saves_spell(level,victim,DAM_POISON))
		      	 {
		            char_puts("You feel poison coursing through your veins.",
		              victim);
		            act("$n is poisoned by the venom on $p.",
				victim,arrow,NULL,TO_ROOM);

		            af.where     = TO_AFFECTS;
		            af.type      = gsn_poison;
		            af.level     = level * 3/4;
		            af.duration  = level / 2;
		            af.location  = APPLY_STR;
		            af.modifier  = -1;
		            af.bitvector = AFF_POISON;
		            affect_join(victim, &af);
		      	 }

		  	}
		  	if (IS_WEAPON_STAT(arrow,WEAPON_FLAMING))
		  	{
		      	 act("$n is burned by $p.",victim,arrow,NULL,TO_ROOM);
		      	 act("$p sears your flesh.",victim,arrow,NULL,TO_CHAR);
		      	 fire_effect((void *) victim,arrow->level,dam,TARGET_CHAR);
		        }
		  	if (IS_WEAPON_STAT(arrow,WEAPON_FROST))
		        {
		            act("$p freezes $n.",victim,arrow,NULL,TO_ROOM);
		            act("The cold touch of $p surrounds you with ice.",
		                victim,arrow,NULL,TO_CHAR);
		            cold_effect(victim,arrow->level,dam,TARGET_CHAR);
		        }
		        if (IS_WEAPON_STAT(arrow,WEAPON_SHOCKING))
		        {
		            act("$n is struck by lightning from $p.",victim,arrow,NULL,TO_ROOM);
		            act("You are shocked by $p.",victim,arrow,NULL,TO_CHAR);
		            shock_effect(victim,arrow->level,dam,TARGET_CHAR);
		        }

			if (dam > victim->max_hit / 10 
				&& number_percent() < 50)
			{
			  af.where     = TO_AFFECTS;
			  af.type      = sn;
			  af.level     = ch->level; 
			  af.duration  = -1;
			  af.location  = APPLY_HITROLL;
			  af.modifier  = - (dam / 20);
			  if (IS_NPC(victim)) af.bitvector = 0;
				else af.bitvector = AFF_CORRUPTION;

			  affect_join(victim, &af);

			  obj_to_char(arrow,victim);
			  equip_char(victim,arrow,WEAR_STUCK_IN);
			}
		        else obj_to_room(arrow,victim->in_room); 

			damage(ch, victim, dam, sn, DAM_PIERCE, DAMF_SHOW);
			path_to_track(ch,victim,door);

		    }
		    return TRUE;
		  }
		  else {
		 	  obj_to_room(arrow,victim->in_room);
		          act("$p sticks in the ground at your feet!",victim,arrow,NULL, TO_ALL);
		          return FALSE;
		        }
		 }
 	 pExit = dest_room->exit[ door ];
		 if (!pExit) break;
		 else 
		 {
		  dest_room = pExit->u1.to_room;
		  if (dest_room->people)
			 act_printf(dest_room->people, arrow, NULL, TO_ALL,
				    POS_RESTING,
			 		"$p sails into the room from the %s!",
					dir_name[rev_dir[door]]);
		 }
		}
	return FALSE;
}

void do_shoot(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *victim;
	OBJ_DATA *wield;
	OBJ_DATA *arrow; 
	char arg1[512],arg2[512];
	bool success;
	int chance, direction;
	int range = (ch->level / 10) + 1;
	
	if (IS_NPC(ch))
		return; /* Mobs can't use bows */

	if (IS_NPC(ch) || (chance = get_skill(ch, gsn_bow)) == 0) {
		char_puts("You don't know how to shoot.\n",ch);
		return;
	}

	argument = one_argument(argument, arg1);
	one_argument(argument, arg2);

	if (arg1[0] == '\0' || arg2[0] == '\0') {
		char_puts("Shoot what direction and whom?\n", ch);
		return;
	}

	if (ch->fighting) {
		char_puts("You cannot concentrate on shooting arrows.\n", ch);
		return;
	}

	direction = find_exit(ch, arg1);

	if (direction < 0 || direction >= MAX_DIR) {
		char_puts("Shoot which direction and whom?\n",ch);
		return;
	}
		
	WAIT_STATE(ch, SKILL(gsn_bow)->beats);
   
	if ((victim = find_char(ch, arg2, direction, range)) == NULL) {
		char_puts("They aren't there.\n", ch);
		return;
	}

	if (!IS_NPC(victim) && victim->desc == NULL) {
		char_puts("You can't do that.\n", ch);
		return;
	}

	if (victim == ch) {
		char_puts("That's pointless.\n", ch);
		return;
	}

	wield = get_eq_char(ch, WEAR_WIELD);
	arrow = get_eq_char(ch, WEAR_HOLD);    

	if (!wield || wield->pIndexData->item_type != ITEM_WEAPON
	||  wield->value[0]!=WEAPON_BOW) {
		char_puts("You need a bow to shoot!\n",ch);
		return;    	
	}

	if (get_eq_char(ch, WEAR_SECOND_WIELD)
	||  get_eq_char(ch, WEAR_SHIELD)) {
		char_puts("Your second hand should be free!\n",ch);
		return;    	
	}

	if (!arrow) {
		 char_puts("You need an arrow holding for your ammunition!\n",
			   ch);
		 return;    	
	}
		
	if (arrow->pIndexData->item_type != ITEM_WEAPON
	||  arrow->value[0] != WEAPON_ARROW) {
		char_puts("That's not the right kind of arrow!\n",ch);
		return;
	}
		
	if (is_safe(ch, victim))
		return;

	chance = (chance - 50) * 2;
	if (ch->position == POS_SLEEPING)
		chance += 40;
	if (ch->position == POS_RESTING)
		chance += 10;
	if (victim->position == POS_FIGHTING)
		chance -= 40;
	chance += GET_HITROLL(ch);

	act_printf(ch, arrow, NULL, TO_CHAR, POS_RESTING,
		   "You shoot $p to %s.", dir_name[direction]);
	act_printf(ch, arrow, NULL, TO_ROOM, POS_RESTING,
		   "$n shoots $p to %s.", dir_name[direction]);

	obj_from_char(arrow);
	success = send_arrow(ch, victim, arrow, direction, chance,
			     dice(wield->value[1],wield->value[2]));
	check_improve(ch, gsn_bow, TRUE, 1);
}


char *find_way(CHAR_DATA *ch,ROOM_INDEX_DATA *rstart, ROOM_INDEX_DATA *rend) 
{
 int direction;
 static char buf[1024];
 EXIT_DATA *pExit;
 char buf2[2];

 snprintf(buf, sizeof(buf), "Bul: ");
 while (1)
 {
 if ((rend == rstart))
		 return buf;
  if ((direction = find_path(rstart->vnum,rend->vnum,ch,-40000,0)) == -1)
		{
		 strcat(buf," BUGGY");
		 return buf;
		}
  if (direction < 0 || direction > 5)
		{
		 strcat(buf," VERY BUGGY");
		 return buf;
		}
   buf2[0] = dir_name[direction][0];
   buf2[1] = '\0';
   strcat(buf,buf2);
   /* find target room */
   pExit = rstart->exit[ direction ];
   if (!pExit)  
		{
		 strcat(buf," VERY VERY BUGGY");
		 return buf;
		}
   else rstart = pExit->u1.to_room;
 }
}	

void do_human(CHAR_DATA *ch, const char *argument)
{
	if (!is_affected(ch, gsn_vampire)) {
		char_puts("You are already a human.\n", ch);
		return;
	}

	affect_strip(ch, gsn_vampire);
	char_puts("You return to your original size.\n", ch);
}

void do_throw_spear(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *victim;
	OBJ_DATA *spear;
	char arg1[512],arg2[512];
	bool success;
	int chance,direction;
	int range = (ch->level / 10) + 1;

	if (IS_NPC(ch) || (chance = get_skill(ch, gsn_spear)) == 0) {
		char_puts("You don't know how to throw a spear.\n",ch);
		return;
	}

	argument = one_argument(argument, arg1);
	one_argument(argument, arg2);

  	if (arg1[0] == '\0' || arg2[0] == '\0') {
		char_puts("Throw spear what direction and whom?\n", ch);
		return;
	}

	if (ch->fighting) {
		char_puts("You cannot concentrate on throwing spear.\n", ch);
		return;
	}

	direction = find_exit(ch, arg1);
	if (direction < 0 || direction >= MAX_DIR) {
		char_puts("Throw which direction and whom?\n",ch);
		return;
	}
		
	WAIT_STATE(ch, SKILL(gsn_spear)->beats);
   
	if ((victim = find_char(ch, arg2, direction, range)) == NULL) {
		char_puts("They aren't there.\n", ch);
		return;
	}

	if (!IS_NPC(victim) && victim->desc == NULL) {
		char_puts("You can't do that.\n", ch);
		return;
	}

	if (victim == ch) {
		char_puts("That's pointless.\n", ch);
		return;
	}

	spear = get_eq_char(ch, WEAR_WIELD);
	if (!spear || spear->pIndexData->item_type != ITEM_WEAPON
	||  spear->value[0] != WEAPON_SPEAR) {
		char_puts("You need a spear to throw!\n",ch);
		return;    	
	}

	if (get_eq_char(ch,WEAR_SECOND_WIELD) || get_eq_char(ch,WEAR_SHIELD)) {
		char_puts("Your second hand should be free!\n",ch);
		return;    	
	}

	if (is_safe(ch,victim))
		return;

	chance = (chance - 50) * 2;
	if (ch->position == POS_SLEEPING)
		chance += 40;
	if (ch->position == POS_RESTING)
		chance += 10;
	if (victim->position == POS_FIGHTING)
		chance -= 40;
	chance += GET_HITROLL(ch);

	act_printf(ch, spear, NULL, TO_CHAR, POS_RESTING,
			"You throw $p to %s.", dir_name[ direction ]);
	act_printf(ch, spear, NULL, TO_ROOM, POS_RESTING,
			"$n throws $p to %s.", dir_name[ direction ]);

	obj_from_char(spear);
	success = send_arrow(ch,victim,spear,direction,chance,
			dice(spear->value[1],spear->value[2]));
	check_improve(ch, gsn_spear, TRUE, 1);
}


/* random room generation procedure */
ROOM_INDEX_DATA  *get_random_room(CHAR_DATA *ch)
{
	ROOM_INDEX_DATA *room;

	for (; ;) {
		room = get_room_index(number_range(0, 65535));

		if (!room)
			continue;

		if (can_see_room(ch,room)
		&&  !room_is_private(room)
		&&  !IS_SET(room->room_flags, ROOM_SAFE) 
		&&  !IS_SET(room->area->flags, AREA_UNDER_CONSTRUCTION)
		&&  (!IS_NPC(ch) || !IS_SET(ch->act, ACT_AGGRESSIVE) ||
		     !IS_SET(room->room_flags, ROOM_LAW)))
			break;
	}

	return room;
}

/* RT Enter portals */
void do_enter(CHAR_DATA *ch, const char *argument)
{    
	ROOM_INDEX_DATA *location; 
	ROOM_INDEX_DATA *old_room;
	OBJ_DATA *portal;
	CHAR_DATA *fch, *fch_next, *mount;

	if (ch->fighting != NULL) 
		return;

	/* nifty portal stuff */
	if (argument[0] == '\0') {
		char_puts("Nope, can't do it.\n",ch);
		return;
	}

	old_room = ch->in_room;
	portal = get_obj_list(ch, argument, ch->in_room->contents);
	
	if (portal == NULL) {
		char_puts("You don't see that here.\n",ch);
		return;
	}

	if (portal->pIndexData->item_type != ITEM_PORTAL 
	||  (IS_SET(portal->value[1], EX_CLOSED) && !IS_TRUSTED(ch, ANGEL))) {
		char_puts("You can't seem to find a way in.\n", ch);
		return;
	}

	if (IS_SET(portal->value[2], GATE_NOCURSE)
	&&  !IS_TRUSTED(ch, ANGEL)
	&&  (IS_AFFECTED(ch, AFF_CURSE) ||
	     IS_SET(old_room->room_flags, ROOM_NORECALL) ||
	     IS_RAFFECTED(old_room, RAFF_CURSE))) {
		char_puts("Something prevents you from leaving...\n",ch);
		return;
	}

	if (IS_SET(portal->value[2], GATE_RANDOM) || portal->value[3] == -1) {
		location = get_random_room(ch);
		portal->value[3] = location->vnum; /* keeps record */
	}
	else if (IS_SET(portal->value[2], GATE_BUGGY) && (number_percent() < 5))
		location = get_random_room(ch);
	else
		location = get_room_index(portal->value[3]);

	if (location == NULL
	||  location == old_room
	||  !can_see_room(ch, location) 
	||  (room_is_private(location) && !IS_TRUSTED(ch, IMPLEMENTOR))) {
		act("$p doesn't seem to go anywhere.", ch, portal,NULL,TO_CHAR);
		return;
	}

	if (IS_NPC(ch) && IS_SET(ch->act, ACT_AGGRESSIVE)
	&&  IS_SET(location->room_flags, ROOM_LAW | ROOM_SAFE)) {
	        char_puts("Something prevents you from leaving...\n",ch);
	        return;
	}

	act(MOUNTED(ch) ? "$n steps into $p, riding on $N." :
			  "$n steps into $p.",
	    ch, portal, MOUNTED(ch), TO_ROOM);
	
	act(IS_SET(portal->value[2], GATE_NORMAL_EXIT) ?
	    "You enter $p." :
	    "You walk through $p and find yourself somewhere else...",
	    ch, portal, NULL, TO_CHAR); 

	mount = MOUNTED(ch);
	char_from_room(ch);
	char_to_room(ch, location);

	if (IS_SET(portal->value[2], GATE_GOWITH)) {/* take the gate along */
		obj_from_room(portal);
		obj_to_room(portal, location);
	}

	if (IS_SET(portal->value[2], GATE_NORMAL_EXIT))
		act(mount ? "$n has arrived, riding $N" : "$n has arrived.",
		    ch, portal, mount, TO_ROOM);
	else
		act(mount ? "$n has arrived through $p, riding $N." :
	        	    "$n has arrived through $p.",
		    ch, portal, mount, TO_ROOM);

	do_look(ch,"auto");

	if (mount) {
		char_from_room(mount);
		char_to_room(mount, location);
  		ch->riding = TRUE;
  		mount->riding = TRUE;
	}

	/* charges */
	if (portal->value[0] > 0) {
		portal->value[0]--;
		if (portal->value[0] == 0)
			portal->value[0] = -1;
	}

	/* protect against circular follows */
	if (old_room == location)
		return;

	for (fch = old_room->people; fch != NULL; fch = fch_next) {
	        fch_next = fch->next_in_room;

		/* no following through dead portals */
	        if (portal == NULL || portal->value[0] == -1) 
	        	continue;
 
	        if (fch->master != ch || fch->position != POS_STANDING)
			continue;

	        if (IS_SET(ch->in_room->room_flags,ROOM_LAW)
	        &&  (IS_NPC(fch) && IS_SET(fch->act,ACT_AGGRESSIVE))) {
	        	act("You can't bring $N into the city.",
	                    ch, NULL, fch, TO_CHAR);
			act("You aren't allowed in the city.",
			    fch, NULL, NULL, TO_CHAR);
			continue;
	        }
 
	        act("You follow $N.", fch, NULL, ch, TO_CHAR);
		do_enter(fch,argument);
	}

 	if (portal != NULL && portal->value[0] == -1) {
		act("$p fades out of existence.", ch, portal, NULL, TO_CHAR);
		if (ch->in_room == old_room)
			act("$p fades out of existence.",
			    ch, portal, NULL, TO_ROOM);
		else if (old_room->people != NULL) {
			act("$p fades out of existence.", 
			    old_room->people, portal, NULL, TO_CHAR);
			act("$p fades out of existence.",
			    old_room->people,portal,NULL,TO_ROOM);
		}
		extract_obj(portal);
	}

	/* 
	 * If someone is following the char, these triggers get
	 * activated for the followers before the char,
	 * but it's safer this way...
	 */
	if (IS_NPC(ch) && HAS_TRIGGER(ch, TRIG_ENTRY))
		mp_percent_trigger(ch, NULL, NULL, NULL, TRIG_ENTRY);
	if (!IS_NPC(ch))
		mp_greet_trigger(ch);
}

void do_settraps(CHAR_DATA *ch, const char *argument)
{
	int chance;

	if ((chance = get_skill(ch, gsn_settraps)) == 0) {
		char_puts("You don't know how to set traps.\n",ch);
		return;
	}

	if (!ch->in_room)
		return;

	if (IS_SET(ch->in_room->room_flags, ROOM_LAW)) {
		char_puts("A mystical power protects the room.\n",ch);
		return;
	}

	WAIT_STATE(ch, SKILL(gsn_settraps)->beats);

	if (IS_NPC(ch) || number_percent() <  chance * 7 / 10) {
	  AFFECT_DATA af,af2;

	  check_improve(ch,gsn_settraps,TRUE,1);

	  if (is_affected_room(ch->in_room, gsn_settraps))
	  {
	char_puts("This room has already trapped.\n",ch);
	return;
	   }

	  if (is_affected(ch,gsn_settraps))
	  {
	char_puts("This skill is used too recently.\n",ch);
	return;
	  }
   
	  af.where     = TO_ROOM_AFFECTS;
	  af.type      = gsn_settraps;
	  af.level     = ch->level;
	  af.duration  = ch->level / 40;
	  af.location  = APPLY_NONE;
	  af.modifier  = 0;
	  af.bitvector = RAFF_THIEF_TRAP;
	  affect_to_room(ch->in_room, &af);

	  af2.where     = TO_AFFECTS;
	  af2.type      = gsn_settraps;
	  af2.level	    = ch->level;
	
	  if (!IS_IMMORTAL(ch) && IS_PUMPED(ch))
	     af2.duration  = 1;
	  else af2.duration = ch->level / 10;

	  af2.modifier  = 0;
	  af2.location  = APPLY_NONE;
	  af2.bitvector = 0;
	  affect_to_char(ch, &af2);
	  char_puts("You set the room with your trap.\n", ch);
	  act("$n set the room with $s trap.",ch,NULL,NULL,TO_ROOM);
	  return;
	}
	else check_improve(ch,gsn_settraps,FALSE,1);

   return;
}

void do_thumbling(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_STRING_LENGTH];
	int chance;
	bool attack;
	AFFECT_DATA af;

	if (IS_NPC(ch) || (chance = get_skill(ch, gsn_thumbling)) == 0) {
		char_puts("You don't know how to do that.\n", ch);
		return;
	}

	one_argument(argument, arg);

	if (arg[0] == '\0') {
		if (is_affected(ch, gsn_thumbling)) {
			affect_strip(ch, gsn_thumbling);
			char_puts("Ok.\n", ch);
		}
		return;
	}

	if (!str_prefix(arg, "attack"))
		attack = TRUE;
	else if (!str_prefix(arg, "defense"))
		attack = FALSE;
	else {
		char_puts("Aglebargle, glip-glop?\n", ch);
		return;
	}

	WAIT_STATE(ch, SKILL(gsn_thumbling)->beats);

	if (is_affected(ch, gsn_thumbling)) {
		char_puts("You do the best you can.\n", ch);
		return;
	}

	if (IS_AFFECTED(ch, AFF_FLYING)) {
		char_puts("Touch the ground first.\n", ch);
		return;
	}

	if (number_percent() > chance) {
		act("You failed to reach the true source of tennis ball power.", ch, NULL, NULL, TO_CHAR);
		act("$n falls to the ground flat on $s face.", ch, NULL, NULL, TO_ROOM);
		check_improve(ch, gsn_thumbling, FALSE, 3);
		return;
	}

	af.where	= TO_AFFECTS;
	af.type		= gsn_thumbling;
	af.level	= ch->level;
	af.duration	= -1;
	af.bitvector	= 0;

	if (attack) {
		af.modifier	= ch->level / 3;
		af.location	= APPLY_HITROLL;
		affect_to_char(ch, &af);
		af.location	= APPLY_DAMROLL;
	}
	else {
		af.modifier	= - ch->level * 2;
		af.location	= APPLY_AC;
	}
	affect_to_char(ch, &af);

	act("You start to jump like a tennis ball!", ch, NULL, NULL, TO_CHAR);
	act("$n starts to jump like a tennis ball!", ch, NULL, NULL, TO_ROOM);

	check_improve(ch, gsn_thumbling, TRUE, 3);
}

