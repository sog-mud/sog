/*
 * $Id: act_move.c,v 1.73 1998-08-03 00:22:29 efdi Exp $
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
#include "recycle.h"
#include "db.h"
#include "comm.h"
#include "resource.h"
#include "hometown.h"
#include "magic.h"
#include "update.h"
#include "util.h"
#include "log.h"
#include "act_move.h"
#include "mob_prog.h"
#include "obj_prog.h"
#include "mlstring.h"
#include "interp.h"
#include "fight.h"

/* command procedures needed */
DECLARE_DO_FUN(do_look		);
DECLARE_DO_FUN(do_recall	);
DECLARE_DO_FUN(do_stand		);
DECLARE_DO_FUN(do_yell		);
void	one_hit		args((CHAR_DATA *ch, CHAR_DATA *victim, int dt ,bool secondary)); 
int 	mount_success 	args((CHAR_DATA *ch, CHAR_DATA *mount, int canattack));
int 	find_path	args((int in_room_vnum, int out_room_vnum, CHAR_DATA *ch, int depth, int in_zone));
int	check_obj_dodge	args((CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj, int bonus)); 


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
int	find_door	args((CHAR_DATA *ch, char *arg));
int 	find_exit	args((CHAR_DATA *ch, char *arg));
bool	has_key		args((CHAR_DATA *ch, int key));



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

	if (RIDDEN(ch) && !IS_NPC(ch->mount)) {
		move_char(ch->mount,door,follow);
		return;
	}

	if (CAN_DETECT(ch, ADET_WEB) 
	|| (MOUNTED(ch) && CAN_DETECT(ch->mount, ADET_WEB))) {
		WAIT_STATE(ch, PULSE_VIOLENCE);
		if (number_percent() < str_app[IS_NPC(ch) ?
			20 : get_curr_stat(ch,STAT_STR)].tohit * 5) {
		 	affect_strip(ch, gsn_web);
		 	REMOVE_BIT(ch->detection, ADET_WEB);
		 	char_nputs(WHEN_YOU_ATTEMPT_YOU_BREAK_WEBS, ch);
		 	act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING,
				    N_BREAKS_THE_WEBS);
		} else {
			char_nputs(YOU_ATTEMPT_WEBS_HOLD_YOU, ch);
			act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING,
				    N_STRUGGLES_VAINLY_AGAINST_WEBS);
			return; 
		}
	}

	if (door < 0 || door > 5) {
		bug("Do_move: bad door %d.", door);
		return;
	}

	if ((IS_AFFECTED(ch, AFF_HIDE) && !IS_AFFECTED(ch, AFF_SNEAK))  
	|| (IS_AFFECTED(ch, AFF_FADE) && !IS_AFFECTED(ch, AFF_SNEAK)) ) {
		REMOVE_BIT(ch->affected_by, AFF_HIDE);
		char_nputs(YOU_STEP_OUT_SHADOWS, ch);
		act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING,
			    N_STEPS_OUT_OF_SHADOWS);
	}

	if (IS_AFFECTED(ch, AFF_CAMOUFLAGE))  {
		int chance;

		if (IS_NPC(ch)
		||  (chance = get_skill(ch, gsn_move_camf)) == 0) {
			REMOVE_BIT(ch->affected_by, AFF_CAMOUFLAGE);
			char_nputs(YOU_STEP_OUT_COVER, ch);
			act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING,
				    N_STEPS_OUT_COVER);
		}	    
		else if (number_percent() < chance)
			check_improve(ch, gsn_move_camf, TRUE, 5);
		else {
			REMOVE_BIT(ch->affected_by, AFF_CAMOUFLAGE);
			char_nputs(YOU_STEP_OUT_COVER, ch);
			act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING,
				    N_STEPS_OUT_COVER);
			check_improve(ch, gsn_move_camf, FALSE, 5);
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
	||  !can_see_room(ch,pexit->u1.to_room)) {
		char_nputs(ALAS_YOU_CANNOT_GO, ch);
		return;
	}

	if (IS_ROOM_AFFECTED(in_room, RAFF_RANDOMIZER)) {
		int d0;
		while (1) {
			d0 = number_range(0, 5);
			if ((pexit = in_room->exit[d0]) == NULL
			||  (to_room = pexit->u1.to_room) == NULL 
			||  !can_see_room(ch,pexit->u1.to_room))
				continue;	  
			d0 = door;
			break;
		}
	}

	if (IS_SET(pexit->exit_info, EX_CLOSED) 
	&& (!IS_AFFECTED(ch, AFF_PASS_DOOR) ||
	    IS_SET(pexit->exit_info, EX_NOPASS))
	&&  !IS_TRUSTED(ch, ANGEL)) {
		if (IS_AFFECTED(ch, AFF_PASS_DOOR)
		&&  IS_SET(pexit->exit_info, EX_NOPASS))  {
  			act_nprintf(ch, NULL, pexit->keyword, TO_CHAR,
				    POS_RESTING, YOU_FAILED_TO_PASS);
			act_nprintf(ch, NULL, pexit->keyword, TO_ROOM,
				    POS_RESTING, N_TRIES_TO_PASS_FAILED);
		}
		else
			act_nprintf(ch, NULL, pexit->keyword, TO_CHAR,
				    POS_RESTING, THE_D_IS_CLOSED);
		return;
	}

	if (IS_AFFECTED(ch, AFF_CHARM)
	&&  ch->master != NULL
	&&  in_room == ch->master->in_room) {
		char_nputs(WHAT_YOU_LEAVE, ch);
		return;
	}

/*    if (!is_room_owner(ch,to_room) && room_is_private(to_room))	*/
	if (room_is_private(to_room)) {
		char_nputs(ROOM_IS_PRIVATE, ch);
		return;
	}

	if (MOUNTED(ch)) {
		if (MOUNTED(ch)->position < POS_FIGHTING) {
			char_nputs(MOUNT_MUST_STAND, ch);
			return; 
		}
		if (!mount_success(ch, MOUNTED(ch), FALSE)) {
			char_nputs(MOUNT_REFUSES_GO_THAT_WAY, ch);
			return;
		}
	}

	if (!IS_NPC(ch)) {
		int move;
		int i;

		if ((i = guild_check(ch, to_room)) > 0) {
			if (IS_PUMPED(ch)) {
				char_nputs(YOU_FEEL_TOO_BLOODY, ch);
				return;
			}
		}
		else if (i < 0) {
			char_nputs(YOU_ARENT_ALLOWED_THERE, ch);
			return;
		}

		if (ch->level < LEVEL_IMMORTAL && IS_PUMPED(ch)
		&&  IS_SET(to_room->room_flags, ROOM_SAFE)) {
			char_nputs(YOU_FEEL_TOO_BLOODY, ch);
			return;
		}

		if (in_room->sector_type == SECT_AIR
		||  to_room->sector_type == SECT_AIR) {
			if (MOUNTED(ch)) {
		        	if(!IS_AFFECTED(MOUNTED(ch), AFF_FLYING)) {
		        		char_nputs(YOUR_MOUNT_CANT_FLY, ch);
						   return;
				}
			} 
			else if (!IS_AFFECTED(ch, AFF_FLYING)
			&& !IS_IMMORTAL(ch)) {
				char_nputs(YOU_CANT_FLY, ch);
				return;
			} 
		}

		if ((in_room->sector_type == SECT_WATER_NOSWIM ||
		     to_room->sector_type == SECT_WATER_NOSWIM)
		&&  (MOUNTED(ch) && !IS_AFFECTED(MOUNTED(ch),AFF_FLYING))) {
			char_nputs(YOU_CANT_TAKE_MOUNT_THERE, ch);
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
			if (obj->item_type == ITEM_BOAT)
			{
			    found = TRUE;
			    break;
			}
		    }
		    if (!found)
		    {
			char_nputs(YOU_NEED_A_BOAT, ch);
			return;
		    }
		}

		move = movement_loss[UMIN(SECT_MAX-1, in_room->sector_type)]
		     + movement_loss[UMIN(SECT_MAX-1, to_room->sector_type)]
		     ;

		  move /= 2;  /* i.e. the average */


		/* conditional effects */
		if (IS_AFFECTED(ch,AFF_FLYING) || IS_AFFECTED(ch,AFF_HASTE))
		    move /= 2;

		if (IS_AFFECTED(ch,AFF_SLOW))
		    move *= 2;

		if (!MOUNTED(ch) && ch->move < move)
		{
		    char_nputs(YOU_TOO_EXHAUSTED, ch);
		    return;
		}

		if (!MOUNTED(ch) && 
		(ch->in_room->sector_type == SECT_DESERT || IS_WATER(ch->in_room)))
		  WAIT_STATE(ch, 2);
		else
		  WAIT_STATE(ch, 1);
		
		if (!MOUNTED(ch))	ch->move -= move;
	}

	if (!IS_AFFECTED(ch, AFF_SNEAK) && !IS_AFFECTED(ch, AFF_CAMOUFLAGE)
	&&   ch->invis_level < LEVEL_HERO)
	 {
		  if (!IS_NPC(ch) && ch->in_room->sector_type != SECT_INSIDE &&
		      ch->in_room->sector_type != SECT_CITY &&
		      number_percent() < get_skill(ch,gsn_quiet_movement))
		    {
		    if (MOUNTED(ch))
			/* XXX */
			act_nprintf(ch, NULL, dir_name[door], TO_ROOM, POS_RESTING, 
				LEAVES_RIDING_ON,
				mlstr_cval(MOUNTED(ch)->short_descr, ch));
		      else
			act_nprintf(ch, NULL, dir_name[door], TO_ROOM, POS_RESTING, 
				LEAVES);
		      check_improve(ch,gsn_quiet_movement,TRUE,1);
		    }
		else 
		  {
		     if (MOUNTED(ch))
			/* XXX */
			act_nprintf(ch, NULL, dir_name[door], TO_ROOM, POS_RESTING, 
				LEAVES_T_RIDING_ON,
				mlstr_cval(MOUNTED(ch)->short_descr, ch));
  	   else
			act_nprintf(ch, NULL, dir_name[door], TO_ROOM, POS_RESTING, 
				LEAVES_T);
		  }
	 }

	if (IS_AFFECTED(ch, AFF_CAMOUFLAGE)
	&&  to_room->sector_type != SECT_FIELD
	&&  to_room->sector_type != SECT_FOREST
	&&  to_room->sector_type != SECT_MOUNTAIN
	&&  to_room->sector_type != SECT_HILLS) {
		REMOVE_BIT(ch->affected_by, AFF_CAMOUFLAGE);
		char_nputs(YOU_STEP_OUT_COVER, ch);
		act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING,
			    N_STEPS_OUT_COVER);
	}

	mount = MOUNTED(ch);
	char_from_room(ch);
	char_to_room(ch, to_room);

	/* room record for tracking */
	if (!IS_NPC(ch) && ch->in_room && !IS_AFFECTED(ch, AFF_FLYING))
		room_record(ch->name, in_room, door);


	if (!IS_AFFECTED(ch, AFF_SNEAK) && ch->invis_level < LEVEL_HERO) {
		if (mount)
			act_nprintf(ch, NULL, mount, TO_ROOM, POS_RESTING,
				    ARRIVED_RIDING);
		else
			act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING,
				    ARRIVED);
	}

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
			act_nprintf(ch, NULL, fch, TO_CHAR, POS_DEAD,
				    YOU_CANT_BRING_N_CITY);
			act_nprintf(fch, NULL, NULL, TO_CHAR,
				    POS_RESTING, YOU_ARENT_ALLOWED_CITY);
			continue;
		}

		act_nprintf(fch, NULL, ch, TO_CHAR, POS_DEAD, YOU_FOLLOW_N);
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
		act_nprintf(ch, NULL, arg, TO_CHAR, POS_DEAD,
			    I_SEE_NO_EXIT_T_HERE);
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
		act_nprintf(ch, NULL, arg, TO_CHAR, POS_DEAD, I_SEE_NO_T_HERE);
		return -1;
	}

	if ((pexit = ch->in_room->exit[door]) == NULL) {
		act_nprintf(ch, NULL, arg, TO_CHAR, POS_DEAD,
			    I_SEE_NO_DOOR_T_HERE);
		return -1;
	}

	if (!IS_SET(pexit->exit_info, EX_ISDOOR)) {
		char_nputs(YOU_CANT_DO_THAT, ch);
		return -1;
	}

	return door;
}

/* scan.c */

void scan_list           args((ROOM_INDEX_DATA *scan_room, CHAR_DATA *ch,
		                         int depth, int door));

void do_scan2(CHAR_DATA *ch, const char *argument)
{
	extern char *const dir_name[];
	EXIT_DATA *pExit;
	int door;

	act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING, 
			N_LOOKS_ALL_AROUND);
	char_nputs(LOOKING_AROUND_YOU_SEE, ch);
	char_nputs(SCAN_HERE, ch);
	scan_list(ch->in_room, ch, 0, -1);
	for (door = 0; door < 6; door++) {
		if ((pExit = ch->in_room->exit[door]) == NULL
		|| !pExit->u1.to_room
		|| !can_see_room(ch,pExit->u1.to_room))
			continue;
		char_printf(ch, "{C%s{x:\n\r", dir_name[door]);
		if (IS_SET(pExit->exit_info, EX_CLOSED)) {
			char_nputs(SCAN_DOOR_CLOSED, ch);
			continue;
		}
		scan_list(pExit->u1.to_room, ch, 1, door);
	}
	return;
}

void scan_list(ROOM_INDEX_DATA *scan_room, CHAR_DATA *ch, 
		int depth, int door)
{
	CHAR_DATA *rch;

	if (scan_room == NULL) 
		return;
	for (rch = scan_room->people; rch != NULL; rch = rch->next_in_room) {
		if (rch == ch || 
		    (!IS_NPC(rch) && rch->invis_level > get_trust(ch)))
			continue;
		if (can_see(ch, rch)) 
			char_printf(ch, "{W	%s{x.\n\r", PERS(rch, ch));
	}
	return;
}

void do_open(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	int door;

	one_argument(argument, arg);

	if (arg[0] == '\0') {
		char_nputs(OPEN_WHAT, ch);
		return;
	}



	if ((obj = get_obj_here(ch, arg)) != NULL) {
 	/* open portal */
		if (obj->item_type == ITEM_PORTAL)
		{
		    if (!IS_SET(obj->value[1], EX_ISDOOR))
		    {
			char_nputs(YOU_CANT_DO_THAT, ch);
			return;
		    }

		    if (!IS_SET(obj->value[1], EX_CLOSED)) {
			char_nputs(ITS_ALREADY_OPEN, ch);
			return;
		    }

		    if (IS_SET(obj->value[1], EX_LOCKED)) {
			char_nputs(ITS_LOCKED, ch);
			return;
		    }

		    REMOVE_BIT(obj->value[1], EX_CLOSED);
		    act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD, YOU_OPEN_P);
		    act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING, N_OPENS_P);
		    return;
 	}

		/* 'open object' */
		if (obj->item_type != ITEM_CONTAINER)
		    { char_nputs(THATS_NOT_A_CONTAINER, ch); return; }
		if (!IS_SET(obj->value[1], CONT_CLOSED))
		    { char_nputs(ITS_ALREADY_OPEN, ch); return; }
		if (!IS_SET(obj->value[1], CONT_CLOSEABLE))
		    { char_nputs(YOU_CANT_DO_THAT, ch); return; }
		if (IS_SET(obj->value[1], CONT_LOCKED))
		    { char_nputs(ITS_LOCKED, ch); return; }

		REMOVE_BIT(obj->value[1], CONT_CLOSED);
		act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD, YOU_OPEN_P);
		act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING, N_OPENS_P);
		return;
	}
	if ((door = find_door(ch, arg)) >= 0)
	{
		/* 'open door' */
		ROOM_INDEX_DATA *to_room;
		EXIT_DATA *pexit;
		EXIT_DATA *pexit_rev;

		pexit = ch->in_room->exit[door];
		if (!IS_SET(pexit->exit_info, EX_CLOSED))
		    { char_nputs(ITS_ALREADY_OPEN, ch); return; }
		if ( IS_SET(pexit->exit_info, EX_LOCKED))
		    { char_nputs(ITS_LOCKED, ch); return; }

		REMOVE_BIT(pexit->exit_info, EX_CLOSED);
		act_nprintf(ch, NULL, pexit->keyword, TO_ROOM, POS_RESTING, N_OPENS_THE_D);
		char_nputs(OK, ch);

		/* open the other side */
		if ((to_room   = pexit->u1.to_room           ) != NULL
		&&   (pexit_rev = to_room->exit[rev_dir[door]]) != NULL
		&&   pexit_rev->u1.to_room == ch->in_room)
		{
		    CHAR_DATA *rch;

		    REMOVE_BIT(pexit_rev->exit_info, EX_CLOSED);
		    for (rch = to_room->people; rch != NULL; rch = rch->next_in_room)
			act_nprintf(rch, NULL, pexit_rev->keyword, TO_CHAR, POS_RESTING, THE_D_OPENS);
		}
		return;
	}
	return;
}



void do_close(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	int door;

	one_argument(argument, arg);

	if (arg[0] == '\0') {
		char_nputs(CLOSE_WHAT, ch);
		return;
	}



	if ((obj = get_obj_here(ch, arg)) != NULL) {
		/* portal stuff */
		if (obj->item_type == ITEM_PORTAL)
		{

		    if (!IS_SET(obj->value[1],EX_ISDOOR)
		    ||   IS_SET(obj->value[1],EX_NOCLOSE))
		    {
			char_nputs(YOU_CANT_DO_THAT, ch);
			return;
		    }

		    if (IS_SET(obj->value[1],EX_CLOSED)) {
			char_nputs(ITS_ALREADY_CLOSED, ch);
			return;
		    }

		    SET_BIT(obj->value[1],EX_CLOSED);
		    act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD, YOU_CLOSE_P);
		    act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING, N_CLOSES_P);
		    return;
		}

		/* 'close object' */
		if (obj->item_type != ITEM_CONTAINER)
		    { char_nputs(THATS_NOT_A_CONTAINER, ch); return; }
		if (IS_SET(obj->value[1], CONT_CLOSED))
		    { char_nputs(ITS_ALREADY_CLOSED, ch); return; }
		if (!IS_SET(obj->value[1], CONT_CLOSEABLE))
		    { char_nputs(YOU_CANT_DO_THAT, ch); return; }

		SET_BIT(obj->value[1], CONT_CLOSED);
		act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD, YOU_CLOSE_P);
		act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING, N_CLOSES_P);
		return;
	}

	if ((door = find_door(ch, arg)) >= 0)
	{
		/* 'close door' */
		ROOM_INDEX_DATA *to_room;
		EXIT_DATA *pexit;
		EXIT_DATA *pexit_rev;

		pexit	= ch->in_room->exit[door];
		if (IS_SET(pexit->exit_info, EX_CLOSED))
		    { char_nputs(ITS_ALREADY_CLOSED, ch); return; }

		SET_BIT(pexit->exit_info, EX_CLOSED);
		act_nprintf(ch, NULL, pexit->keyword, TO_ROOM, POS_RESTING, N_CLOSES_THE_D);
		char_nputs(OK, ch);

		/* close the other side */
		if ((to_room   = pexit->u1.to_room           ) != NULL
		&&   (pexit_rev = to_room->exit[rev_dir[door]]) != 0
		&&   pexit_rev->u1.to_room == ch->in_room)
		{
		    CHAR_DATA *rch;

		    SET_BIT(pexit_rev->exit_info, EX_CLOSED);
		    for (rch = to_room->people; rch != NULL; rch = rch->next_in_room)
			act_nprintf(rch, NULL, pexit_rev->keyword, TO_CHAR, POS_RESTING, THE_D_CLOSES);
		}
		return;
	}

	return;
}


/* 
 * Added can_see check. Kio.
 */

bool has_key(CHAR_DATA *ch, int key)
{
	OBJ_DATA *obj;

	for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
	{
		if (obj->pIndexData->vnum == key)
		  if (can_see_obj(ch, obj))
		    return TRUE;
	}

	return FALSE;
}

bool has_key_ground(CHAR_DATA *ch, int key)
{
	OBJ_DATA *obj;

	for (obj = ch->in_room->contents; obj != NULL; obj = obj->next_content)
	{
		if (obj->pIndexData->vnum == key)
		    if (can_see_obj(ch, obj))
		    return TRUE;
	}

	return FALSE;
}



void do_lock(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	int door;
	CHAR_DATA *rch;

	one_argument(argument, arg);

	if (arg[0] == '\0')
	{
		char_nputs(LOCK_WHAT, ch);
		return;
	}


	if ((obj = get_obj_here(ch, arg)) != NULL)
	{
		/* portal stuff */
		if (obj->item_type == ITEM_PORTAL)
		{
		    if (!IS_SET(obj->value[1],EX_ISDOOR)
		    ||  IS_SET(obj->value[1],EX_NOCLOSE))
		    {
			char_nputs(YOU_CANT_DO_THAT, ch);
			return;
		    }
		    if (!IS_SET(obj->value[1],EX_CLOSED))
		    {
			char_nputs(ITS_NOT_CLOSED, ch);
		 	return;
		    }

		    if (obj->value[4] < 0 || IS_SET(obj->value[1],EX_NOLOCK))
		    {
			char_nputs(IT_CANT_BE_LOCKED, ch);
			return;
		    }

		    if (!has_key(ch,obj->value[4]))
		    {
			char_nputs(YOU_LACK_THE_KEY, ch);
			return;
		    }

		    if (IS_SET(obj->value[1],EX_LOCKED))
		    {
			char_nputs(ITS_ALREADY_LOCKED, ch);
			return;
		    }

		    SET_BIT(obj->value[1],EX_LOCKED);
		    act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD, YOU_LOCK_P);
		    act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING, N_LOCKS_P);
		    return;
		}

		/* 'lock object' */
		if (obj->item_type != ITEM_CONTAINER)
		    { char_nputs(THATS_NOT_A_CONTAINER, ch); return; }
		if (!IS_SET(obj->value[1], CONT_CLOSED))
		    { char_nputs(ITS_NOT_CLOSED, ch); return; }
		if (obj->value[2] < 0)
		    { char_nputs(IT_CANT_BE_LOCKED, ch); return; }
		if (!has_key(ch, obj->value[2]))
		    { char_nputs(YOU_LACK_THE_KEY, ch); return; }
		if (IS_SET(obj->value[1], CONT_LOCKED))
		    { char_nputs(ITS_ALREADY_LOCKED, ch); return; }

		SET_BIT(obj->value[1], CONT_LOCKED);
		act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD, YOU_LOCK_P);
		act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING, N_LOCKS_P);
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
		    { char_nputs(ITS_NOT_CLOSED, ch); return; }
		if (pexit->key < 0)
		    { char_nputs(IT_CANT_BE_LOCKED, ch); return; }
		if (!has_key(ch, pexit->key) && 
		       !has_key_ground(ch, pexit->key))
		    { char_nputs(YOU_LACK_THE_KEY, ch); return; }
		if (IS_SET(pexit->exit_info, EX_LOCKED))
		    { char_nputs(ITS_ALREADY_LOCKED, ch); return; }

		SET_BIT(pexit->exit_info, EX_LOCKED);
		char_nputs(CLICK, ch);
		act_nprintf(ch, NULL, pexit->keyword, TO_ROOM, POS_RESTING, N_LOCKS_THE_D);

		/* lock the other side */
		if ((to_room   = pexit->u1.to_room           ) != NULL
		&&   (pexit_rev = to_room->exit[rev_dir[door]]) != 0
		&&   pexit_rev->u1.to_room == ch->in_room)
		{
		    SET_BIT(pexit_rev->exit_info, EX_LOCKED);
		    for (rch = to_room->people; rch != NULL; rch = rch->next_in_room)
			act_nprintf(rch, NULL, pexit_rev->keyword, TO_CHAR, POS_RESTING, THE_D_CLICKS);

		}
		  return;
	}
	return;
}



void do_unlock(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	int door;
	CHAR_DATA *rch;

	one_argument(argument, arg);

	if (arg[0] == '\0') {
		char_nputs(UNLOCK_WHAT, ch);
		return;
	}


	if ((obj = get_obj_here(ch, arg)) != NULL) {
 	/* portal stuff */
		if (obj->item_type == ITEM_PORTAL) {
		    if (IS_SET(obj->value[1],EX_ISDOOR)) {
			char_nputs(YOU_CANT_DO_THAT, ch);
			return;
		    }

		    if (!IS_SET(obj->value[1],EX_CLOSED)) {
			char_nputs(ITS_NOT_CLOSED, ch);
			return;
		    }

		    if (obj->value[4] < 0) {
			char_nputs(IT_CANT_BE_UNLOCKED, ch);
			return;
		    }

		    if (!has_key(ch,obj->value[4])) {
			char_nputs(YOU_LACK_THE_KEY, ch);
			return;
		    }

		    if (!IS_SET(obj->value[1],EX_LOCKED)) {
			char_nputs(ITS_ALREADY_UNLOCKED, ch);
			return;
		    }

		    REMOVE_BIT(obj->value[1],EX_LOCKED);
		    act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD, YOU_UNLOCK_P);
		    act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING, N_UNLOCKS_P);
		    return;
		}

		/* 'unlock object' */
		if (obj->item_type != ITEM_CONTAINER)
		    { char_nputs(THATS_NOT_A_CONTAINER, ch); return; }
		if (!IS_SET(obj->value[1], CONT_CLOSED))
		    { char_nputs(ITS_NOT_CLOSED, ch); return; }
		if (obj->value[2] < 0)
		    { char_nputs(IT_CANT_BE_UNLOCKED, ch); return; }
		if (!has_key(ch, obj->value[2]))
		    { char_nputs(YOU_LACK_THE_KEY, ch); return; }
		if (!IS_SET(obj->value[1], CONT_LOCKED))
		    { char_nputs(ITS_ALREADY_UNLOCKED, ch); return; }

		REMOVE_BIT(obj->value[1], CONT_LOCKED);
		act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD, YOU_UNLOCK_P);
		act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING, N_UNLOCKS_P);
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
		    { char_nputs(ITS_NOT_CLOSED, ch); return; }
		if (pexit->key < 0)
		    { char_nputs(IT_CANT_BE_UNLOCKED, ch); return; }
		if (!has_key(ch, pexit->key) && 
		       !has_key_ground(ch, pexit->key))
		    { char_nputs(YOU_LACK_THE_KEY, ch); return; }
		if (!IS_SET(pexit->exit_info, EX_LOCKED))
		    { char_nputs(ITS_ALREADY_UNLOCKED, ch); return; }

		REMOVE_BIT(pexit->exit_info, EX_LOCKED);
		char_nputs(CLICK, ch);
		act_nprintf(ch, NULL, pexit->keyword, TO_ROOM, POS_RESTING, N_UNLOCKS_THE_D);

		/* unlock the other side */
		if ((to_room   = pexit->u1.to_room           ) != NULL
		&&   (pexit_rev = to_room->exit[rev_dir[door]]) != NULL
		&&   pexit_rev->u1.to_room == ch->in_room)
		{
		    REMOVE_BIT(pexit_rev->exit_info, EX_LOCKED);
		    for (rch = to_room->people; rch != NULL; rch = rch->next_in_room)
			act_nprintf(rch, NULL, pexit_rev->keyword, TO_CHAR, POS_RESTING, THE_D_CLICKS);
		}
		  return;
	}
	return;
}



void do_pick(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *gch;
	OBJ_DATA *obj;
	int door;

	one_argument(argument, arg);

	if (arg[0] == '\0') {
		char_nputs(PICK_WHAT, ch);
		return;
	}

	if (MOUNTED(ch)) {
		  char_nputs(CANT_PICK_MOUNTED, ch);
		  return;
	}

	WAIT_STATE(ch, skill_table[gsn_pick_lock].beats);

	/* look for guards */
	for (gch = ch->in_room->people; gch; gch = gch->next_in_room)
	{
		if (IS_NPC(gch) && IS_AWAKE(gch) && ch->level + 5 < gch->level)
		{
		    act_nprintf(ch, NULL, gch, TO_CHAR, POS_RESTING, N_IS_STANDING_TOO_CLOSE_TO_LOCK);
		    return;
		}
	}

	if (!IS_NPC(ch) && number_percent() > get_skill(ch,gsn_pick_lock))
	{
		char_nputs(YOU_FAILED, ch);
		check_improve(ch,gsn_pick_lock,FALSE,2);
		return;
	}

	if ((obj = get_obj_here(ch, arg)) != NULL) {
		/* portal stuff */
		if (obj->item_type == ITEM_PORTAL) {
		    if (!IS_SET(obj->value[1],EX_ISDOOR)) {	
			char_nputs(YOU_CANT_DO_THAT, ch);
			return;
		    }

		    if (!IS_SET(obj->value[1],EX_CLOSED)) {
			char_nputs(ITS_NOT_CLOSED, ch);
			return;
		    }

		    if (obj->value[4] < 0) {
			char_nputs(IT_CANT_BE_UNLOCKED, ch);
			return;
		    }

		    if (IS_SET(obj->value[1],EX_PICKPROOF)) {
			char_nputs(YOU_FAILED, ch);
			return;
		    }

		    REMOVE_BIT(obj->value[1],EX_LOCKED);
		    act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD, YOU_PICK_THE_LOCK_ON_P);
		    act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING, N_PICKS_THE_LOCK_ON_P);
		    check_improve(ch,gsn_pick_lock,TRUE,2);
		    return;
		}

		
		/* 'pick object' */
		if (obj->item_type != ITEM_CONTAINER)
		    { char_nputs(THATS_NOT_A_CONTAINER, ch); return; }
		if (!IS_SET(obj->value[1], CONT_CLOSED))
		    { char_nputs(ITS_NOT_CLOSED, ch); return; }
		if (obj->value[2] < 0)
		    { char_nputs(IT_CANT_BE_UNLOCKED, ch); return; }
		if (!IS_SET(obj->value[1], CONT_LOCKED))
		    { char_nputs(ITS_ALREADY_UNLOCKED, ch); return; }
		if (IS_SET(obj->value[1], CONT_PICKPROOF))
		    { char_nputs(YOU_FAILED, ch); return; }

		REMOVE_BIT(obj->value[1], CONT_LOCKED);
		  act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD, YOU_PICK_THE_LOCK_ON_P);
		  act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING, N_PICKS_THE_LOCK_ON_P);
		check_improve(ch,gsn_pick_lock,TRUE,2);
		return;
	}

	if ((door = find_door(ch, arg)) >= 0)
	{
		/* 'pick door' */
		ROOM_INDEX_DATA *to_room;
		EXIT_DATA *pexit;
		EXIT_DATA *pexit_rev;

		pexit = ch->in_room->exit[door];
		if (!IS_SET(pexit->exit_info, EX_CLOSED) && !IS_IMMORTAL(ch))
		    { char_nputs(ITS_NOT_CLOSED, ch); return; }
		if (pexit->key < 0 && !IS_IMMORTAL(ch))
		    { char_nputs(IT_CANT_BE_PICKED, ch); return; }
		if (!IS_SET(pexit->exit_info, EX_LOCKED))
		    { char_nputs(ITS_ALREADY_UNLOCKED, ch); return; }
		if (IS_SET(pexit->exit_info, EX_PICKPROOF) && !IS_IMMORTAL(ch))
		    { char_nputs(YOU_FAILED, ch); return; }

		REMOVE_BIT(pexit->exit_info, EX_LOCKED);
		char_nputs(CLICK, ch);
		act_nprintf(ch, NULL, pexit->keyword, TO_ROOM, POS_RESTING, N_PICKS_THE_D);
		check_improve(ch,gsn_pick_lock,TRUE,2);

		/* pick the other side */
		if ((to_room   = pexit->u1.to_room           ) != NULL
		&&   (pexit_rev = to_room->exit[rev_dir[door]]) != NULL
		&&   pexit_rev->u1.to_room == ch->in_room)
		{
		    REMOVE_BIT(pexit_rev->exit_info, EX_LOCKED);
		}
	}

	return;
}




void do_stand(CHAR_DATA *ch, const char *argument)
{
	OBJ_DATA *obj = NULL;

	if (argument[0] != '\0') {
		if (ch->position == POS_FIGHTING) {
		    char_nputs(MAYBE_YOU_SHOULD_FINISH_FIGHTING_FIRST, ch);
		    return;
		}
		obj = get_obj_list(ch,argument,ch->in_room->contents);
		if (obj == NULL)
		{
		    char_nputs(YOU_DONT_SEE_THAT, ch);
		    return;
		}
		if (obj->item_type != ITEM_FURNITURE
		||  (!IS_SET(obj->value[2],STAND_AT)
		&&   !IS_SET(obj->value[2],STAND_ON)
		&&   !IS_SET(obj->value[2],STAND_IN)))
		{
		    char_nputs(YOU_CANT_FIND_PLACE_STAND, ch);
		    return;
		}
		if (ch->on != obj && count_users(obj) >= obj->value[0])
		{
		    act_nprintf(ch, obj, NULL, TO_ROOM, POS_DEAD, THERES_NO_ROOM_TO_STAND_ON);
		    return;
		}
	}
	switch (ch->position)
	{
	case POS_SLEEPING:
		if (IS_AFFECTED(ch, AFF_SLEEP))
		    { char_nputs(YOU_CANT_WAKE_UP, ch); return; }
		
		if (obj == NULL)
		{
		    char_nputs(YOU_WAKE_AND_STAND_UP, ch);
		    act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING, 
				N_WAKES_AND_STANDS_UP);
		    ch->on = NULL;
		}
		else if (IS_SET(obj->value[2],STAND_AT))
		{
		   act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD, 
				YOU_WAKE_AND_STAND_AT);
		   act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING, 
				N_WAKES_AND_STANDS_AT);
		}
		else if (IS_SET(obj->value[2],STAND_ON))
		{
		   act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD, 
				YOU_WAKE_AND_STAND_ON);
		   act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING, 
				N_WAKES_AND_STANDS_ON);
		}
		else 
		{
		   act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD, 
				YOU_WAKE_AND_STAND_IN);
		   act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING, 
				N_WAKES_AND_STANDS_IN);
		}

		  if (IS_HARA_KIRI(ch)) 
		{
		 char_nputs(FEEL_BLOOD_HEATS, ch);
		 REMOVE_BIT(ch->act,PLR_HARA_KIRI);
		}

		ch->position = POS_STANDING;
		do_look(ch,"auto");
		break;

	case POS_RESTING: case POS_SITTING:
		if (obj == NULL)
		{
		    char_nputs(YOU_STAND_UP, ch);
		    act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING, N_STANDS_UP);
		    ch->on = NULL;
		}
		else if (IS_SET(obj->value[2],STAND_AT))
		{
		    act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD, YOU_STAND_AT);
		    act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING, N_STANDS_AT);
		}
		else if (IS_SET(obj->value[2],STAND_ON))
		{
		    act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD, YOU_STAND_ON);
		    act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING, N_STANDS_ON);
		}
		else
		{
		    act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD, YOU_STAND_IN);
		    act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING, N_STANDS_IN);
		}
		ch->position = POS_STANDING;
		break;

	case POS_STANDING:
		char_nputs(YOU_ARE_ALREADY_STANDING, ch);
		break;

	case POS_FIGHTING:
		char_nputs(YOU_ARE_ALREADY_FIGHTING, ch);
		break;
	}

	return;
}



void do_rest(CHAR_DATA *ch, const char *argument)
{
	OBJ_DATA *obj = NULL;

	if (ch->position == POS_FIGHTING)
	{
		char_nputs(YOU_ARE_ALREADY_FIGHTING, ch);
		return;
	}

	if (MOUNTED(ch)) 
	{
		  char_nputs(YOU_CANT_REST_MOUNTED, ch);
		  return;
	}
	if (RIDDEN(ch)) 
	{
		  char_nputs(YOU_CANT_REST_RIDDEN, ch);
		  return;
	}


	if (IS_AFFECTED(ch, AFF_SLEEP))
	{ char_nputs(YOU_ARE_ALREADY_SLEEPING, ch); return; }

	/* okay, now that we know we can rest, find an object to rest on */
	if (argument[0] != '\0')
	{
		obj = get_obj_list(ch,argument,ch->in_room->contents);
		if (obj == NULL)
		{
		    char_nputs(YOU_DONT_SEE_THAT, ch);
		    return;
		}
	}
	else obj = ch->on;

	if (obj != NULL)
	{
		  if (!IS_SET(obj->item_type,ITEM_FURNITURE) 
		||  (!IS_SET(obj->value[2],REST_ON)
		&&   !IS_SET(obj->value[2],REST_IN)
		&&   !IS_SET(obj->value[2],REST_AT)))
		{
		    char_nputs(YOU_CANT_REST_ON_THAT, ch);
		    return;
		}

		  if (obj != NULL && ch->on != obj && count_users(obj) >= obj->value[0])
		  {
		    act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD, 
				THERES_NO_MORE_ROOM_ON);
		    return;
		}
		
		ch->on = obj;
	}

	switch (ch->position)
	{
	case POS_SLEEPING:
		if (obj == NULL)
		{
		    char_nputs(YOU_WAKE_AND_REST, ch);
		    act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING, 
				N_WAKES_AND_RESTS);
		}
		else if (IS_SET(obj->value[2],REST_AT))
		{
		    act_nprintf(ch, obj, NULL, TO_CHAR, POS_SLEEPING,
				YOU_WAKE_UP_AND_REST_AT);
		    act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING,
				N_WAKES_UP_AND_RESTS_AT);
		}
		  else if (IS_SET(obj->value[2],REST_ON))
		  {
		    act_nprintf(ch, obj, NULL, TO_CHAR, POS_SLEEPING,
				YOU_WAKE_UP_AND_REST_ON);
		    act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING,
				N_WAKES_UP_AND_RESTS_ON);
		  }
		  else
		  {
		    act_nprintf(ch, obj, NULL, TO_CHAR, POS_SLEEPING,
				YOU_WAKE_UP_AND_REST_IN);
		    act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING,
				N_WAKES_UP_AND_RESTS_IN);
		  }
		ch->position = POS_RESTING;
		break;

	case POS_RESTING:
		char_nputs(YOU_ARE_ALREADY_RESTING, ch);
		break;

	case POS_STANDING:
		if (obj == NULL)
		{
		    char_nputs(YOU_REST, ch);
		    act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING,
				N_SITS_DOWN_AND_RESTS);
		}
		  else if (IS_SET(obj->value[2],REST_AT))
		  {
		    act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD,
				YOU_SIT_DOWN_AT_AND_REST);
		    act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING,
				N_SITS_DOWN_AT_AND_RESTS);
		  }
		  else if (IS_SET(obj->value[2],REST_ON))
		  {
		    act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD,
				YOU_SIT_DOWN_ON_AND_REST);
		    act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING,
				N_SITS_DOWN_ON_AND_RESTS);
		  }
		  else
		  {
		    act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD,
				YOU_SIT_DOWN_IN_AND_REST);
		    act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING,
				N_SITS_DOWN_IN_AND_RESTS);
		  }
		ch->position = POS_RESTING;
		break;

	case POS_SITTING:
		if (obj == NULL)
		{
		    char_nputs(YOU_REST, ch);
		    act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING, N_RESTS);
		}
		  else if (IS_SET(obj->value[2],REST_AT))
		  {
		    act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD, YOU_REST_AT);
		    act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING, N_RESTS_AT);
		  }
		  else if (IS_SET(obj->value[2],REST_ON))
		  {
		    act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD, YOU_REST_ON);
		    act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING, N_RESTS_ON);
		  }
		  else
		  {
		    act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD, YOU_REST_IN);
		    act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING, N_RESTS_IN);
		}
		ch->position = POS_RESTING;

		  if (IS_HARA_KIRI(ch)) 
		{
		 char_nputs(FEEL_BLOOD_HEATS, ch);
		 REMOVE_BIT(ch->act, PLR_HARA_KIRI);
		}
		break;
	}

	return;
}


void do_sit (CHAR_DATA *ch, const char *argument)
{
	OBJ_DATA *obj = NULL;

	if (ch->position == POS_FIGHTING)
	{
		char_nputs(MAYBE_YOU_SHOULD_FINISH_FIGHTING_FIRST, ch);
		return;
	}
	if (MOUNTED(ch)) 
	{
		  char_nputs(YOU_CANT_SIT_MOUNTED, ch);
		  return;
	}
	if (RIDDEN(ch)) 
	{
		  char_nputs(YOU_CANT_SIT_RIDDEN, ch);
		  return;
	}

	if (IS_AFFECTED(ch, AFF_SLEEP))
	{ char_nputs(YOU_ARE_ALREADY_SLEEPING, ch); return; }

	/* okay, now that we know we can sit, find an object to sit on */
	if (argument[0] != '\0')
	{
		obj = get_obj_list(ch,argument,ch->in_room->contents);
		if (obj == NULL)
		{
	if (IS_AFFECTED(ch, AFF_SLEEP))
	{ char_nputs(YOU_ARE_ALREADY_SLEEPING, ch); return; }
		    char_nputs(YOU_DONT_SEE_THAT, ch);
		    return;
		}
	}
	else obj = ch->on;

	if (obj != NULL)                                                              
	{
		if (!IS_SET(obj->item_type,ITEM_FURNITURE)
		||  (!IS_SET(obj->value[2],SIT_ON)
		&&   !IS_SET(obj->value[2],SIT_IN)
		&&   !IS_SET(obj->value[2],SIT_AT)))
		{
		    char_nputs(YOU_CANT_SIT_ON_THAT, ch);
		    return;
		}

		if (obj != NULL && ch->on != obj && count_users(obj) >= obj->value[0])
		{
		    act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD,
				THERES_NO_MORE_ROOM_ON);
		    return;
		}

		ch->on = obj;
	}
	switch (ch->position)
	{
		case POS_SLEEPING:
		      if (obj == NULL)
		      {
		      	char_nputs(YOU_WAKE_AND_SIT_UP, ch);
		      	act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING,
					N_WAKES_AND_SITS_UP);
		      }
		      else if (IS_SET(obj->value[2],SIT_AT))
		      {
		      	act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD,
					YOU_WAKE_AND_SIT_AT);
		      	act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING,
					N_WAKES_AND_SITS_AT);
		      }
		      else if (IS_SET(obj->value[2],SIT_ON))
		      {
		      	act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD,
					YOU_WAKE_AND_SIT_ON);
		      	act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING,
					N_WAKES_AND_SITS_ON);
		      }
		      else
		      {
		      	act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD,
					YOU_WAKE_AND_SIT_IN);
		      	act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING,
					N_WAKES_AND_SITS_IN);
		      }

		    ch->position = POS_SITTING;
		    break;
		case POS_RESTING:
		    if (obj == NULL)
			char_nputs(YOU_STOP_RESTING, ch);
		    else if (IS_SET(obj->value[2],SIT_AT))
		    {
			act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD,
					YOU_SIT_AT);
			act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING,
					N_SITS_AT);
		    }

		    else if (IS_SET(obj->value[2],SIT_ON))
		    {
			act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD,
					YOU_SIT_ON);
			act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING,
					N_SITS_ON);
		    }
		    ch->position = POS_SITTING;
		    break;
		case POS_SITTING:
		    char_nputs(YOU_ARE_ALREADY_SITTING_DOWN, ch);
		    break;
		case POS_STANDING:
		    if (obj == NULL)
		    {
			char_nputs(YOU_SIT_DOWN, ch);
		        act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING,
					N_SITS_DOWN_ON_THE_GROUND);
		    }
		    else if (IS_SET(obj->value[2],SIT_AT))
		    {
			act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD,
					YOU_SIT_DOWN_AT);
			act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING,
					N_SITS_DOWN_AT);
		    }
		    else if (IS_SET(obj->value[2],SIT_ON))
		    {
			act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD,
					YOU_SIT_ON);
			act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING,
					N_SITS_ON);
		    }
		    else
		    {
			act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD,
					YOU_SIT_DOWN_IN);
			act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING,
					N_SITS_DOWN_IN);
		    }
		    ch->position = POS_SITTING;
		    break;
	}
	if (IS_HARA_KIRI(ch)) 
		{
		 char_nputs(FEEL_BLOOD_HEATS, ch);
		 REMOVE_BIT(ch->act,PLR_HARA_KIRI);
		}
	return;
}


void do_sleep(CHAR_DATA *ch, const char *argument)
{
	OBJ_DATA *obj = NULL;

	if (MOUNTED(ch)) 
	{
		  char_nputs(YOU_CANT_SLEEP_MOUNTED, ch);
		  return;
	}
	if (RIDDEN(ch)) 
	{
		  char_nputs(YOU_CANT_SLEEP_RIDDEN, ch);
		  return;
	}

	switch (ch->position)
	{
	case POS_SLEEPING:
		char_nputs(YOU_ARE_ALREADY_SLEEPING, ch);
		break;

	case POS_RESTING:
	case POS_SITTING:
	case POS_STANDING: 
		if (argument[0] == '\0' && ch->on == NULL)
		{
		    char_nputs(YOU_GO_TO_SLEEP, ch);
		    act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING,
					N_GOES_TO_SLEEP);
		    ch->position = POS_SLEEPING;
		}
		else  /* find an object and sleep on it */
		{
		    if (argument[0] == '\0')
			obj = ch->on;
		    else
		    	obj = get_obj_list(ch, argument,  ch->in_room->contents);

		    if (obj == NULL)
		    {
			char_nputs(YOU_DONT_SEE_THAT, ch);
			return;
		    }
		    if (obj->item_type != ITEM_FURNITURE
		    ||  (!IS_SET(obj->value[2],SLEEP_ON) 
		    &&   !IS_SET(obj->value[2],SLEEP_IN)
		    &&	 !IS_SET(obj->value[2],SLEEP_AT)))
		    {
			char_nputs(YOU_CANT_SLEEP_ON_THAT, ch);
			return;
		    }

		    if (ch->on != obj && count_users(obj) >= obj->value[0])
		    {
			act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD,
					THERES_NO_ROOM_ON_P_FOR_YOU);
			return;
		    }

		    ch->on = obj;
		    if (IS_SET(obj->value[2],SLEEP_AT))
		    {
			act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD,
					YOU_GO_TO_SLEEP_AT);
			act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING,
					N_GOES_TO_SLEEP_AT);
		    }
		    else if (IS_SET(obj->value[2],SLEEP_ON))
		    {
			act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD,
					YOU_GO_TO_SLEEP_ON);
			act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING,
					N_GOES_TO_SLEEP_ON);
		    }
		    else
		    {
			act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD,
					YOU_GO_TO_SLEEP_IN);
			act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING,
					N_GOES_TO_SLEEP_IN);
		    }
		    ch->position = POS_SLEEPING;
		}
		break;

	case POS_FIGHTING:
		char_nputs(YOU_ARE_ALREADY_FIGHTING, ch);
		break;
	}

	return;
}


void do_wake(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	one_argument(argument, arg);
	if (arg[0] == '\0')
		{ do_stand(ch, argument); return; }

	if (!IS_AWAKE(ch))
		{ char_nputs(YOU_ARE_ASLEEP_YOURSELF, ch); return; }

	if ((victim = get_char_room(ch, arg)) == NULL)
		{ char_nputs(THEY_ARENT_HERE, ch); return; }

	if (IS_AWAKE(victim)) { 
		act_nprintf(ch, NULL, victim, TO_CHAR, POS_DEAD,
				N_IS_ALREADY_AWAKE); return; 
	}

	if (IS_AFFECTED(victim, AFF_SLEEP)) { 
		act_nprintf(ch, NULL, victim, TO_CHAR, POS_DEAD, YOU_CANT_WAKE_M);  
		return; 
	}

	act_nprintf(ch, NULL, victim, TO_VICT, POS_SLEEPING, N_WAKES_YOU);
	do_stand(victim,"");
	return;
}



void do_sneak(CHAR_DATA *ch, const char *argument)
{
	AFFECT_DATA af;

	if (MOUNTED(ch)) 
	{
		  char_nputs(YOU_CANT_SNEAK_MOUNTED, ch);
		  return;
	}

	char_nputs(YOU_ATTEMPT_TO_MOVE_SILENTLY, ch);
	affect_strip(ch, gsn_sneak);

	if (IS_AFFECTED(ch,AFF_SNEAK))
		return;

	if (number_percent() < get_skill(ch,gsn_sneak))
	{
		check_improve(ch,gsn_sneak,TRUE,3);
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
		check_improve(ch,gsn_sneak,FALSE,3);

	return;
}



void do_hide(CHAR_DATA *ch, const char *argument)
{
	int forest;

	if (MOUNTED(ch)) 
	{
		  char_nputs(YOU_CANT_HIDE_MOUNTED, ch);
		  return;
	}

	if (RIDDEN(ch)) 
	{
		  char_nputs(YOU_CANT_HIDE_RIDDEN, ch);
		  return;
	}

	if (IS_AFFECTED(ch, AFF_FAERIE_FIRE) )  {
		char_nputs(YOU_CANNOT_HIDE_GLOWING, ch);
		return;
	}
	forest = 0;
	forest += ch->in_room->sector_type == SECT_FOREST ? 60 : 0;
	forest += ch->in_room->sector_type == SECT_FIELD ? 60 : 0;

	char_nputs(YOU_ATTEMPT_TO_HIDE, ch);

	if (number_percent() < get_skill(ch,gsn_hide)-forest) {
		SET_BIT(ch->affected_by, AFF_HIDE);
		check_improve(ch,gsn_hide,TRUE,3);
	} else  {
		REMOVE_BIT(ch->affected_by, AFF_HIDE);
		check_improve(ch,gsn_hide,FALSE,3);
	}

	return;
}

void do_camouflage(CHAR_DATA *ch, const char *argument)
{
	int chance;

	if (MOUNTED(ch)) 
	{
		  char_nputs(YOU_CANT_CAMOUFLAGE_MOUNTED, ch);
		  return;
	}
	if (RIDDEN(ch)) 
	{
		  char_nputs(YOU_CANT_CAMOUFLAGE_RIDDEN, ch);
		  return;
	}

	if (IS_NPC(ch)
	||  (chance = get_skill(ch, gsn_camouflage)) == 0) {
		char_nputs(YOU_DONT_KNOW_CAMOUFLAGE, ch);
		return;
	}

	if (IS_AFFECTED(ch, AFF_FAERIE_FIRE))  {
		char_nputs(YOU_CANT_CAMOUFLAGE_GLOWING, ch);
		return;
	}

	if (ch->in_room->sector_type != SECT_FOREST &&
		ch->in_room->sector_type != SECT_HILLS  &&
		ch->in_room->sector_type != SECT_MOUNTAIN)
		{
		char_nputs(THERES_NO_COVER_HERE, ch);
		act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING,
				N_TRIES_TO_CAMOUFLAGE);
		return;
		}
	char_nputs(YOU_ATTEMPT_TO_CAMOUFLAGE, ch);
	WAIT_STATE(ch, skill_table[gsn_camouflage].beats);

	if (IS_AFFECTED(ch, AFF_CAMOUFLAGE))
		REMOVE_BIT(ch->affected_by, AFF_CAMOUFLAGE);


	if (IS_NPC(ch) || number_percent() < chance) {
		SET_BIT(ch->affected_by, AFF_CAMOUFLAGE);
		check_improve(ch, gsn_camouflage, TRUE, 1);
	}
	else
		check_improve(ch, gsn_camouflage, FALSE, 1);

	return;
}

/*
 * Contributed by Alander
 */
void do_visible(CHAR_DATA *ch, const char *argument)
{
	if (IS_SET(ch->affected_by, AFF_HIDE)) {
		char_nputs(YOU_STEP_OUT_SHADOWS, ch);
		REMOVE_BIT(ch->affected_by, AFF_HIDE);
		act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING,
				N_STEPS_OUT_OF_SHADOWS);
	}
	if (IS_SET(ch->affected_by, AFF_FADE)) {
		char_nputs(YOU_STEP_OUT_SHADOWS, ch);
		REMOVE_BIT(ch->affected_by, AFF_FADE);
		act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING,
				N_STEPS_OUT_OF_SHADOWS);
	}
	if (IS_SET(ch->affected_by, AFF_CAMOUFLAGE)) {
		char_nputs(YOU_STEP_OUT_COVER, ch);
		REMOVE_BIT(ch->affected_by,AFF_CAMOUFLAGE);
		act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING,
				N_STEPS_OUT_COVER);
	}
	if (IS_SET(ch->affected_by, AFF_INVISIBLE)) {
		char_nputs(YOU_FADE_INTO_EXIST, ch);
		affect_strip(ch, gsn_invis);
		affect_strip(ch, gsn_mass_invis);
		REMOVE_BIT(ch->affected_by, AFF_INVISIBLE);
		act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING,
				N_FADES_INTO_EXIST);
	}
	if (IS_SET(ch->affected_by, AFF_IMP_INVIS)) {
		char_nputs(YOU_FADE_INTO_EXIST, ch);
		affect_strip(ch, gsn_invis);
		affect_strip(ch, gsn_imp_invis);
		REMOVE_BIT(ch->affected_by, AFF_IMP_INVIS);
		act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING,
				N_FADES_INTO_EXIST);
	}
	if (IS_SET(ch->affected_by, AFF_SNEAK) 
	    && !IS_NPC(ch) && !IS_SET(race_table[RACE(ch)].aff,AFF_SNEAK)) {
		char_nputs(YOU_TRAMPLE_AROUND_LOUDLY, ch);
		affect_strip(ch, gsn_sneak);
		REMOVE_BIT(ch->affected_by, AFF_SNEAK);
	}

	affect_strip (ch, gsn_mass_invis);
	return;
}

void do_recall(CHAR_DATA *ch, const char *argument)
{
	ROOM_INDEX_DATA *location;
	int point;
 
	if (IS_NPC(ch) && !IS_SET(ch->act, ACT_PET)) {
		char_nputs(ONLY_PLAYERS_RECALL, ch);
		return;
	}

	if (is_affected(ch, gsn_holler)) {
		char_puts("Gods laugh under your pray.\n\r", ch);
		do_say(ch, "RiCFalsch...\n\r");
		return;
	}

	if (ch->level >= 11 && !IS_IMMORTAL(ch)) {
		char_nputs(RECALL_FOR_BELOW_10, ch);
		return;
	}

	if (ch->desc != NULL && IS_PUMPED(ch)) {
		char_nputs(TOO_PUMPED_TO_PRAY, ch);
		return;
	}

	if (IS_GOOD(ch)) 
		point = hometown_table[ch->hometown].recall[0];
	else if (IS_EVIL(ch))
		point = hometown_table[ch->hometown].recall[2];
	else
		point = hometown_table[ch->hometown].recall[1];

	if (ch->desc == NULL && !IS_NPC(ch)) 
		point =	hometown_table[number_range(0, 4)].recall[number_range(0,2)];

	act_nprintf(ch, 0, 0, TO_ROOM, POS_RESTING, N_PRAYS_FOR_TRANSPORTATION);
	
	if ((location = get_room_index(point))== NULL) {
		char_nputs(YOU_ARE_COMPLETELY_LOST, ch);
		return;
	}

	if (ch->in_room == location)
		return;

	if (IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
	||  IS_AFFECTED(ch, AFF_CURSE) 
	||  IS_RAFFECTED(ch->in_room, RAFF_CURSE)) {
		char_nputs(GODS_FORSAKEN_YOU, ch);
		return;
	}

#if 0
	if ((victim = ch->fighting) != NULL)
	{
		char_nputs(YOU_ARE_STILL_FIGHTING, ch);

		if (IS_NPC(ch))
		    skill = 40 + ch->level;
		else
		    skill = get_skill(ch,gsn_recall);

		if (number_percent() < 80 * skill / 100)
		{
		    check_improve(ch,gsn_recall,FALSE,6);
		    WAIT_STATE(ch, 4);
		    char_nputs(YOU_FAILED, ch);
		    return;
		}

		lose = 25;
		gain_exp(ch, 0 - lose);
		check_improve(ch,gsn_recall,TRUE,4);
		char_nprintf(ch, RECALL_FROM_COMBAT, lose);
		stop_fighting(ch, TRUE); 
	}
#endif

	ch->move /= 2;
	act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING,
				N_DISAPPEARS);
	char_from_room(ch);
	char_to_room(ch, location);
	act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING,
				N_APPEARS_IN_THE_ROOM);
	do_look(ch, "auto");
	
	if (ch->pet != NULL) {
 		char_from_room(ch->pet);
		char_to_room(ch->pet, location);
		do_look(ch->pet, "auto");
	}
}


void do_train(CHAR_DATA *ch, const char *argument)
{
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *mob;
	int stat = - 1;
	char *pOutput = NULL;
	int cost;

	if (IS_NPC(ch))
		return;

	/*
	 * Check for trainer.
	 */
	for (mob = ch->in_room->people; mob; mob = mob->next_in_room) {
		if (IS_NPC(mob) && (IS_SET(mob->act, ACT_PRACTICE) 
		|| IS_SET(mob->act,ACT_TRAIN) || IS_SET(mob->act,ACT_GAIN)))
		    break;
	}

	if (mob == NULL) {
		char_nputs(YOU_CANT_DO_THAT_HERE, ch);
		return;
	}

	if (argument[0] == '\0') {
		char_nprintf(ch, YOU_HAVE_D_TRAINING_SESSIONS, ch->train);
		argument = "foo";
	}

	cost = 1;

	if (!str_cmp(argument, "str"))
	{
		if (class_table[ch->class].attr_prime == STAT_STR)
		    cost    = 1;
		stat        = STAT_STR;
		pOutput     = "strength";
	}

	else if (!str_cmp(argument, "int"))
	{
		if (class_table[ch->class].attr_prime == STAT_INT)
		    cost    = 1;
		stat	    = STAT_INT;
		pOutput     = "intelligence";
	}

	else if (!str_cmp(argument, "wis"))
	{
		if (class_table[ch->class].attr_prime == STAT_WIS)
		    cost    = 1;
		stat	    = STAT_WIS;
		pOutput     = "wisdom";
	}

	else if (!str_cmp(argument, "dex"))
	{
		if (class_table[ch->class].attr_prime == STAT_DEX)
		    cost    = 1;
		stat  	    = STAT_DEX;
		pOutput     = "dexterity";
	}

	else if (!str_cmp(argument, "con"))
	{
		if (class_table[ch->class].attr_prime == STAT_CON)
		    cost    = 1;
		stat	    = STAT_CON;
		pOutput     = "constitution";
	}

	else if (!str_cmp(argument, "cha"))
	{
		if (class_table[ch->class].attr_prime == STAT_CHA)
		    cost    = 1;
		stat	    = STAT_CHA;
		pOutput     = "charisma";
	}

	else if (!str_cmp(argument, "hp"))
		cost = 1;

	else if (!str_cmp(argument, "mana"))
		cost = 1;

	else
	{
		strcpy(buf, msg(YOU_CAN_TRAIN, ch));
		if (ch->perm_stat[STAT_STR] < get_max_train2(ch,STAT_STR)) 
		    strcat(buf, " str");
		if (ch->perm_stat[STAT_INT] < get_max_train2(ch,STAT_INT))  
		    strcat(buf, " int");
		if (ch->perm_stat[STAT_WIS] < get_max_train2(ch,STAT_WIS)) 
		    strcat(buf, " wis");
		if (ch->perm_stat[STAT_DEX] < get_max_train2(ch,STAT_DEX))  
		    strcat(buf, " dex");
		if (ch->perm_stat[STAT_CON] < get_max_train2(ch,STAT_CON))  
		    strcat(buf, " con");
		if (ch->perm_stat[STAT_CHA] < get_max_train2(ch,STAT_CHA))  
		    strcat(buf, " cha");
		strcat(buf, " hp mana");

		if (buf[strlen(buf)-1] != ':')
		{
		    strcat(buf, ".\n\r");
		    send_to_char(buf, ch);
		}
		else
		{
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

	if (!str_cmp("hp",argument))
	{
		if (cost > ch->train)
		{
		 	    char_nputs(NOT_ENOUGH_TRAININGS, ch);
		      return;
		  }
 
		ch->train -= cost;
		  ch->pcdata->perm_hit += 10;
		  ch->max_hit += 10;
		  ch->hit +=10;
		  act_nprintf(ch, NULL, NULL, TO_CHAR, POS_DEAD,
				YOUR_DURABILITY_INCREASES);
		  act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING,
				N_DURABILITY_INCREASES);
		  return;
	}
 
	if (!str_cmp("mana",argument))
	{
		  if (cost > ch->train)
		  {
		 	    char_nputs(NOT_ENOUGH_TRAININGS, ch);
		      return;
		  }

		ch->train -= cost;
		  ch->pcdata->perm_mana += 10;
		  ch->max_mana += 10;
		  ch->mana += 10;
		  act_nprintf(ch, NULL, NULL, TO_CHAR, POS_DEAD,
				YOUR_POWER_INCREASES);
		  act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING,
				N_POWER_INCREASES);
		  return;
	}

	if (ch->perm_stat[stat]  >= get_max_train2(ch,stat))
	{
		act_nprintf(ch, NULL, pOutput, TO_CHAR, POS_DEAD, YOUR_T_IS_MAX);
		return;
	}

	if (cost > ch->train)
	{
		  char_nputs(NOT_ENOUGH_TRAININGS, ch);
		return;
	}

	ch->train		-= cost;
  
	ch->perm_stat[stat]		+= 1;
	act_nprintf(ch, NULL, pOutput, TO_CHAR, POS_DEAD, YOUR_T_INCREASES);
	act_nprintf(ch, NULL, pOutput, TO_ROOM, POS_RESTING, N_T_INCREASES);
	return;
}



void do_track(CHAR_DATA *ch, const char *argument)
{
  ROOM_HISTORY_DATA *rh;
  EXIT_DATA *pexit;
  static char *door[] = { "north","east","south","west","up","down",
		                      "that way" };
  int d;

  if (!IS_NPC(ch) && get_skill(ch,gsn_track) < 2) {
	char_nputs(THERE_ARE_NO_TRAIN_TRACKS_HERE, ch);
	return;
  }

  WAIT_STATE(ch, skill_table[gsn_track].beats);
  act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING, N_CHECKS_TRACKS);

  if (IS_NPC(ch) || number_percent() < get_skill(ch,gsn_track))
	{
		if (IS_NPC(ch))
		{
		   ch->status = 0;
		 if (ch->last_fought != NULL && !IS_SET(ch->act,ACT_NOTRACK))
			add_mind(ch,ch->last_fought->name);
		  }
		for (rh = ch->in_room->history;rh != NULL;rh = rh->next)
		  if (is_name(argument,rh->name)) 
		   {
		    check_improve(ch,gsn_track,TRUE,1);
		  if ((d = rh->went) == -1) continue;
		    char_nprintf(ch, TRACKS_LEAD_S, capitalize(rh->name),
				 door[d]);
		  if ((pexit = ch->in_room->exit[d]) != NULL
		    &&   IS_SET(pexit->exit_info, EX_ISDOOR)
		    &&   pexit->keyword != NULL)
		     doprintf(do_open, ch,"%s",door[d]);
		    move_char(ch,rh->went,FALSE);
		    return;
		}
	}

		char_nputs(DONT_SEE_TRACKS, ch);

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
		char_nputs(YOU_CANT_BE_MORE_VAMPIRE, ch);
		return;
	}

	if ((chance = get_skill(ch, gsn_vampire)) == 0) {
		char_nputs(YOU_SHOW_MORE_UGGLY, ch);
		return;
	}

	if (chance < 100) {
		char_nputs(GO_AND_ASK_QUESTOR, ch);
		return;
	}

	if (is_affected(ch, gsn_vampire)) {
		char_nputs(GO_KILL_PLAYER, ch);
		return;
	}


	if (weather_info.sunlight == SUN_LIGHT 
	||  weather_info.sunlight == SUN_RISE) {
		char_nputs(WAIT_NIGHT, ch);
		return;
	}

	level = ch->level;
	duration = level / 10 ;
	duration += 5;

/* haste */
	af.where     = TO_AFFECTS;
	af.type      = gsn_vampire;
	af.level     = level;
	af.duration  = duration;
	af.location  = APPLY_DEX;
	af.modifier  = 1 + (level /20);
	af.bitvector = AFF_HASTE;
	affect_to_char(ch, &af);

/* giant strength + infrared */
	af.where     = TO_AFFECTS;
	af.type      = gsn_vampire;
	af.level     = level;
	af.duration  = duration;
	af.location  = APPLY_STR;
	af.modifier  = 1 + (level / 20);
	af.bitvector = AFF_INFRARED;
	affect_to_char(ch, &af);

/* size */
	af.where     = TO_AFFECTS;
	af.type      = gsn_vampire;
	af.level     = level;
	af.duration  = duration;
	af.location  = APPLY_SIZE;
	af.modifier  = 1 + (level / 50);
	af.bitvector = AFF_SNEAK;
	affect_to_char(ch, &af);

/* damroll */
	af.where     = TO_AFFECTS;
	af.type      = gsn_vampire;
	af.level     = level;
	af.duration  = duration;
	af.location  = APPLY_DAMROLL;
	af.modifier  = ch->damroll;
	af.bitvector = AFF_BERSERK;
	affect_to_char(ch, &af);

/* negative immunity */
	af.where = TO_IMMUNE;
	af.type = gsn_vampire;
	af.duration = duration;
	af.level = level;
	af.location = APPLY_NONE;
	af.modifier = 0;
	af.bitvector = IMM_NEGATIVE;
	affect_to_char(ch, &af);

/* vampire flag */
	af.where     = TO_ACT_FLAG;
	af.type      = gsn_vampire;
	af.level     = level;
	af.duration  = duration;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = PLR_VAMPIRE;
	affect_to_char(ch, &af);

	char_nputs(FEEL_GREATER, ch);
	act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING, CANNOT_RECOGNIZE);
	return;
}

void do_vbite(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int chance;

	one_argument(argument, arg);

	if ((chance = get_skill(ch, gsn_vampiric_bite)) == 0) {
		char_nputs(DONT_KNOW_BITE, ch);
		return;
	}

	if (!IS_VAMPIRE(ch)) {
		char_nputs(MUST_TRANSFORM_VAMP, ch);
		return;
	}

	if (arg[0] == '\0') {
		char_nputs(BITE_WHOM, ch);
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL) {
		char_nputs(THEY_ARENT_HERE, ch);
		return;
	}

	if (victim->position != POS_SLEEPING) {
		char_nputs(THEY_MUST_BE_SLEEPING, ch);
		return;
	}

	if ((IS_NPC(ch)) && (!(IS_NPC(victim))))
		return;
		

	if (victim == ch) {
		char_nputs(HOW_CAN_YOU_SNEAK_YOU, ch);
		return;
	}

	if (is_safe(ch, victim))
		return;

	if (victim->fighting != NULL) {
		char_nputs(CANT_BITE_FIGHTING_PERS, ch);
		return;
	}


	WAIT_STATE(ch, skill_table[gsn_vampiric_bite].beats);

	if (victim->hit < (8 * victim->max_hit / 10) && (IS_AWAKE(victim))) {
		act_nprintf(ch, NULL, victim, TO_CHAR, POS_DEAD,
			   DOESNT_WORTH_UP);
		return;
	}

	if (current_time-victim->last_fight_time<300 && IS_AWAKE(victim)) {
		act_nprintf(ch, NULL, victim, TO_CHAR, POS_DEAD,
			   DOESNT_WORTH_TO_DO);
		return;       
	}

	if (!IS_AWAKE(victim)
	&& (IS_NPC(ch)
	||  number_percent() < ((chance * 7 / 10) +
		(2 * (ch->level - victim->level)) ))) {
		check_improve(ch,gsn_vampiric_bite,TRUE,1);
		one_hit(ch, victim, gsn_vampiric_bite,FALSE);
	}
	else {
		check_improve(ch,gsn_vampiric_bite,FALSE,1);
		damage(ch, victim, 0, gsn_vampiric_bite,DAM_NONE, TRUE);
	}

	/* Player shouts if he doesn't die */
	if (!(IS_NPC(victim)) && !(IS_NPC(ch)) 
		&& victim->position == POS_FIGHTING)
  	    doprintf(do_yell, victim, msg(HELP_TRIED_TO_BITE, victim));
	return;
}

void do_bash_door(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *gch;
	int chance;
	int damage_bash,door;

	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	one_argument(argument,arg);
 
	if ((chance = get_skill(ch, gsn_bash_door)) == 0
	||  (IS_NPC(ch) && !IS_SET(ch->off_flags,OFF_BASH))) {	
		char_nputs(BASH_WHATS_THAT, ch);
		return;
	}
 
	if (MOUNTED(ch)) {
        	char_nputs(CANT_BASH_DOORS_MOUNTED, ch);
		return;
	}

	if (RIDDEN(ch)) {
		char_nputs(CANT_BASH_DOORS_RIDDEN, ch);
		return;
	}

	if (arg[0] == '\0') {
		char_nputs(BASH_WHICH_DOOR, ch);
		return;
	}

	if (ch->fighting) {	
		char_nputs(WAIT_FIGHT_FINISH, ch);
		return;
	}

	/* look for guards */
	for (gch = ch->in_room->people; gch; gch = gch->next_in_room)
		if (IS_NPC(gch)
		&&  IS_AWAKE(gch) && ch->level + 5 < gch->level) {
			act_nprintf(ch, NULL, gch, TO_CHAR, POS_DEAD, 
				    N_TOO_CLOSE_TO_DOOR);
			return;
		}

	if ((door = find_door(ch, arg)) < 0)
		return;

	pexit = ch->in_room->exit[door];

	if (!IS_SET(pexit->exit_info, EX_CLOSED)) {
		char_nputs(ITS_ALREADY_OPEN, ch);
		return;
	}

	if (!IS_SET(pexit->exit_info, EX_LOCKED)) {
		char_nputs(TRY_TO_OPEN, ch);
		return;
	}

	if (IS_SET(pexit->exit_info, EX_NOPASS)) {
		char_nputs(SHIELD_PROTECTS_EXIT, ch); 
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

	act_nprintf(ch, NULL, pexit->keyword, TO_CHAR, POS_DEAD, 
		    YOU_SLAM_TRY_BREAK);
	act_nprintf(ch, NULL, pexit->keyword, TO_ROOM, POS_RESTING,
		    N_SLAMS_TRY_BREAK);


	if (room_dark(ch->in_room))
		chance /= 2;

	/* now the attack */
	if (number_percent() < chance) {
		check_improve(ch,gsn_bash_door,TRUE,1);

		REMOVE_BIT(pexit->exit_info, EX_LOCKED);
		REMOVE_BIT(pexit->exit_info, EX_CLOSED);
		act_nprintf(ch, NULL, pexit->keyword, TO_ROOM, POS_RESTING, 
				N_BASHES_AND_BREAK);
		char_nputs(YOU_SUCCESSED_TO_OPEN_DOOR, ch);

		/* open the other side */
		if ((to_room = pexit->u1.to_room) != NULL
		&&  (pexit_rev = to_room->exit[rev_dir[door]]) != NULL
		&&  pexit_rev->u1.to_room == ch->in_room) {
			CHAR_DATA *rch;

			REMOVE_BIT(pexit_rev->exit_info, EX_CLOSED);
			REMOVE_BIT(pexit_rev->exit_info, EX_LOCKED);
			for (rch = to_room->people; rch != NULL;
							rch = rch->next_in_room)
				act_nprintf(rch, NULL, pexit_rev->keyword,
					    TO_CHAR, POS_DEAD,
					    THE_D_OPENS);
		}

		check_improve(ch, gsn_bash_door, TRUE, 1);
		WAIT_STATE(ch,skill_table[gsn_bash_door].beats);
	}
	else {
		char_nputs(YOU_FALL_ON_FACE, ch);
		act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING,
			    N_FALLS_ON_FACE);
		check_improve(ch, gsn_bash_door, FALSE, 1);
		ch->position = POS_RESTING;
		WAIT_STATE(ch,skill_table[gsn_bash_door].beats * 3/2); 
		damage_bash = ch->damroll +
			      number_range(4,4 + 4* ch->size + chance/5);
		damage(ch, ch, damage_bash, gsn_bash_door, DAM_BASH, TRUE);
	}

}

void do_blink(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];

	argument = one_argument(argument, arg);

	if (!IS_NPC(ch)
	&&  ch->level < skill_table[gsn_blink].skill_level[ch->class]) {
		char_nputs(HUH, ch);
		return;
	}

	if (arg[0] == '\0') {
		char_nprintf(ch, CURRENT_BLINK,
			     IS_SET(ch->act, PLR_BLINK) ? "ON" : "OFF");
		return;
	}

	if (!str_cmp(arg,"ON")) {
		SET_BIT(ch->act, PLR_BLINK);
		char_nputs(BLINK_ON, ch);
		return;
	}

	if (!str_cmp(arg,"OFF")) {
		REMOVE_BIT(ch->act, PLR_BLINK);
		char_nputs(BLINK_OFF, ch);
		return;
	}

	char_nprintf(ch, IS_S_A_STATUS, arg);
	return;
}

void do_vanish(CHAR_DATA *ch, const char *argument)
{
	ROOM_INDEX_DATA *pRoomIndex;

	if (!IS_NPC(ch)
	    && ch->level < skill_table[gsn_vanish].skill_level[ch->class]) {
		char_nputs(HUH, ch);
		return;
	}


	if (ch->mana < 25) {
		char_nputs(DONT_HAVE_POWER, ch);
		return;
	}
	ch->mana -= 25;

	if (number_percent() > get_skill(ch,gsn_vanish)) {
		char_nputs(YOU_FAILED, ch);
		WAIT_STATE(ch, skill_table[gsn_vanish].beats);
		check_improve(ch,gsn_vanish,FALSE,1);
		return;
	}

  WAIT_STATE(ch, skill_table[gsn_vanish].beats);

  if (ch->in_room == NULL
		||   IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL))
	{
		char_nputs(YOU_FAILED, ch);
		return;
	}

  for (; ;)
	{
		if ((pRoomIndex = get_room_index(number_range(0, 65535))) == NULL)
		continue;
		if ((ch->in_room->vnum < 500 || ch->in_room->vnum > 600)
		    && (pRoomIndex->vnum > 500 && pRoomIndex->vnum < 600))
		continue;
		if (can_see_room(ch,pRoomIndex) 
			&& !room_is_private(pRoomIndex)
			&& ch->in_room->area == pRoomIndex->area)
		  break;
	}

  
  act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING, N_THROWS_GLOBE);

  check_improve(ch,gsn_vanish,TRUE,1);

  if (!IS_NPC(ch) && ch->fighting != NULL && number_bits(1) == 1) 
  {
	char_nputs(YOU_FAILED, ch);
	return;
  }

  act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING, N_IS_GONE);

  char_from_room(ch);
  char_to_room(ch, pRoomIndex);
  act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING, N_APPEARS_FROM_NOWHERE);
  do_look(ch, "auto");
  stop_fighting(ch,TRUE);
  return;
}

void do_detect_sneak(CHAR_DATA *ch, const char *argument) 
{
	AFFECT_DATA af;

	if (is_affected(ch, gsn_detect_sneak))
	{
		char_nputs(YOU_CAN_ALREADY_DETECT_SNEAK, ch);
	}
	af.where	 = TO_DETECTS;
	af.type      = gsn_detect_sneak;
	af.level     = ch->level;
	af.duration  = ch->level / 10;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = DETECT_SNEAK;
	affect_to_char(ch, &af);
	char_nputs(YOU_CAN_DETECT_THE_SNEAK, ch);
	return;
}


void do_fade(CHAR_DATA *ch, const char *argument)
{
	if (ch_skill_nok(ch,gsn_fade)) return;

	if (MOUNTED(ch)) 
	{
		  char_nputs(CANT_FADE_MOUNTED, ch);
		  return;
	}
	if (RIDDEN(ch)) 
	{
		  char_nputs(CANT_FADE_RIDDEN, ch);
		  return;
	}

	if (!clan_ok(ch,gsn_fade)) return;
	char_nputs(YOU_ATTEMPT_TO_FADE, ch);

	SET_BIT(ch->affected_by, AFF_FADE);
	check_improve(ch,gsn_fade,TRUE,3);

	return;
}

void do_vtouch(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *victim;
	AFFECT_DATA af;

	if (IS_NPC(ch) ||
	    ch->level < skill_table[gsn_vampiric_touch].skill_level[ch->class])
	{
		char_nputs(LACK_SKILL_DRAIN_TOUCH, ch);
		return;
	}

	if (!IS_VAMPIRE(ch)) {
		char_nputs(LET_IT_BE, ch);
		return;
	}

	if (IS_AFFECTED(ch, AFF_CHARM))  {
		char_nputs(DONT_WANT_DRAIN_MASTER, ch);
		return;
	} 

	if ((victim = get_char_room(ch,argument)) == NULL) {
		char_nputs(THEY_ARENT_HERE, ch);
		return;
	}

	if (ch==victim) {
		char_nputs(EVEN_YOU_NOT_SO_STUPID, ch);
		return;
	}

	if (is_affected(victim,gsn_vampiric_touch))
		return;

	if (is_safe(ch,victim))
		return;

	SET_FIGHT_TIME(victim);
	SET_FIGHT_TIME(ch);

	WAIT_STATE(ch,skill_table[gsn_vampiric_touch].beats);

	if (IS_NPC(ch) || 
	    number_percent() < 0.85 * get_skill(ch,gsn_vampiric_touch)) {
		act_nprintf(ch, NULL, victim, TO_CHAR, POS_DEAD, 
				YOU_TOUCH_NS_NECK);
		act_nprintf(ch, NULL, victim, TO_VICT, POS_RESTING, 
				N_TOUCHES_YOUR_NECK);
		act_nprintf(ch, NULL, victim, TO_NOTVICT, POS_RESTING,
				N_TOUCHES_NS_NECK);
		check_improve(ch,gsn_vampiric_touch,TRUE,1);
		
		af.type = gsn_vampiric_touch;
		  af.where = TO_AFFECTS;
		af.level = ch->level;
		af.duration = ch->level / 20 + 1;
		af.location = APPLY_NONE;
		af.modifier = 0;
		af.bitvector = AFF_SLEEP;
		affect_join (victim,&af);

		if (IS_AWAKE(victim))
		  victim->position = POS_SLEEPING;
	} else {
		damage(ch, victim, 0, gsn_vampiric_touch, DAM_NONE, TRUE);
		check_improve(ch, gsn_vampiric_touch, FALSE, 1);
	}
}

void do_fly(CHAR_DATA *ch, const char *argument)
{
  char arg[MAX_INPUT_LENGTH];

	if (IS_NPC(ch)) return;

	argument = one_argument(argument,arg);
	if (!str_cmp(arg,"up"))
	{
	 if (IS_AFFECTED(ch,AFF_FLYING))
		{		       
		   char_nputs(YOU_ARE_ALREADY_FLYING, ch); 
		 return;
		}
	 if (is_affected(ch,gsn_fly) 
		 || (race_table[RACE(ch)].aff & AFF_FLYING) 
		 || affect_check_obj(ch,AFF_FLYING))
		{
   	 SET_BIT(ch->affected_by,AFF_FLYING);
		 REMOVE_BIT(ch->act,PLR_CHANGED_AFF);
   	 char_nputs(YOU_START_TO_FLY, ch);
		}
   else 
		{		       
		   char_nputs(FIND_POTION_OR_WINGS, ch); 
		}
	}
	else if (!str_cmp(arg,"down"))
	{
   if (IS_AFFECTED(ch,AFF_FLYING))
		{
   	 REMOVE_BIT(ch->affected_by,AFF_FLYING);
		 SET_BIT(ch->act,PLR_CHANGED_AFF);
   	 char_nputs(YOU_SLOWLY_TOUCH_GROUND, ch);
		}
   else 
		{		       
		   char_nputs(YOU_ARE_ALREADY_ON_GROUND, ch); 
		 return;
		}
	}
   else 
	{
		char_nputs(TYPE_WITH_UP_OR_DOWN, ch);
		return;
	}

	WAIT_STATE(ch, skill_table[gsn_fly].beats);   

   return;
}
		 

void do_push(CHAR_DATA *ch, const char *argument)
{
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int percent;
	int door;

	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (arg1[0] == '\0' || arg2[0] == '\0') {
		char_nputs(PUSH_WHOM_WHERE, ch);
		return;
	}

	if (MOUNTED(ch)) {
		  char_nputs(CANT_PUSH_MOUNTED, ch);
		  return;
	}
	if (RIDDEN(ch)) {
		  char_nputs(CANT_PUSH_RIDDEN, ch);
		  return;
	}

	if (IS_NPC(ch) && IS_SET(ch->affected_by, AFF_CHARM) 
		&& (ch->master != NULL)) {
		char_nputs(TOO_DAZED_TO_PUSH, ch);
		return;
	}

	if ((victim = get_char_room(ch, arg1)) == NULL)
	{
		char_nputs(THEY_ARENT_HERE, ch);
		return;
	}

	if (!IS_NPC(victim) && victim->desc == NULL)
	{
		char_nputs(YOU_CANT_DO_THAT, ch);
		return;
	}

	if (victim == ch)
	{
		char_nputs(THATS_POINTLESS, ch);
		return;
	}

	if (is_safe(ch,victim))
		return;

	if (victim->position == POS_FIGHTING)
	{
		char_nputs(WAIT_FIGHT_FINISH, ch);
		return;
	}

if ((door = find_exit(ch, arg2)) >= 0)
 {
		/* 'push' */
	EXIT_DATA *pexit;

	if ((pexit = ch->in_room->exit[door]) != NULL)
	{
	 if (IS_SET(pexit->exit_info, EX_ISDOOR)) 
		{
		    if (IS_SET(pexit->exit_info, EX_CLOSED))
		    char_nputs(DIR_IS_CLOSED, ch); 
		    else if (IS_SET(pexit->exit_info, EX_LOCKED))
		     char_nputs(DIR_IS_LOCKED, ch); 
	 	  return;
		}
	}

	if (CAN_DETECT(ch,ADET_WEB))
	{
		char_nputs(YOU_WEBBED_WANT_WHAT, ch);
		act_nprintf(ch, NULL, victim, TO_ROOM, POS_RESTING, 
				N_TRIES_PUSH_WEBBED);
		return; 
	}

	if (CAN_DETECT(victim,ADET_WEB))
	{
		act_nprintf(ch, NULL, victim, TO_CHAR, POS_DEAD, 
				PUSH_VICT_WEBBED);
		act_nprintf(ch, NULL, victim, TO_ROOM, POS_RESTING,
				N_PUSHES_VICT_WEBBED);
		return; 
	}

	REMOVE_BIT(ch->act, PLR_GHOST);

	WAIT_STATE(ch, skill_table[gsn_push].beats);
	percent  = number_percent() + (IS_AWAKE(victim) ? 10 : -50);
	percent += can_see(victim, ch) ? -10 : 0;

	if (victim->position == POS_FIGHTING
	||  (!IS_NPC(ch) && percent > get_skill(ch,gsn_push))) {
		/*
		 * Failure.
		 */

		char_nputs(OOPS, ch);
		if (!IS_AFFECTED(victim, AFF_SLEEP)) {
		   victim->position = victim->position == POS_SLEEPING ? 
					POS_STANDING : victim->position;
		   act_nprintf(ch, NULL, victim, TO_VICT, POS_RESTING,
				N_TRIED_PUSH_YOU);
		}
		act_nprintf(ch, NULL, victim, TO_NOTVICT, POS_RESTING, 
				N_TRIED_PUSH_N);


		if (IS_AWAKE(victim))
			doprintf(do_yell, victim,
				 msg(KEEP_HANDS_OUT, ch), ch->name);
		if (!IS_NPC(ch))
		{
		    if (IS_NPC(victim))
		    {
		        check_improve(ch,gsn_push,FALSE,2);
			multi_hit(victim, ch, TYPE_UNDEFINED, MSTRIKE);
		    }
		}

		return;
	}


	act_nprintf(ch, NULL, victim, TO_CHAR, POS_SLEEPING,
		       COMM_YOU_PUSH, dir_name[door]);
	act_nprintf(ch, NULL, victim, TO_VICT, POS_SLEEPING,
		       COMM_PUSHES_YOU, dir_name[door]);
	act_nprintf(ch, NULL, victim, TO_NOTVICT, POS_SLEEPING,
		       COMM_PUSHES_N_TO, dir_name[door]);
	move_char(victim, door, FALSE);

	check_improve(ch,gsn_push,TRUE,1);
 }
   return;
}


void do_crecall(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *victim;
	ROOM_INDEX_DATA *location;
	int point = clan_table[ch->clan].room_vnum;
	AFFECT_DATA af;

	if (ch_skill_nok(ch,gsn_clan_recall))
		return;

	if (!clan_ok(ch,gsn_clan_recall))
		return;

	if (is_affected(ch, gsn_clan_recall))
		char_nputs(CANT_PRAY_NOW, ch);

	if (ch->desc != NULL && IS_PUMPED(ch)) {
		char_nputs(TOO_PUMPED_TO_PRAY, ch);
		return;
	}

	act_nprintf(ch, 0, 0, TO_ROOM, POS_RESTING,
		    clan_table[ch->clan].string_prays);
	
	if ((location = get_room_index(point))== NULL) {
		char_nputs(YOU_ARE_COMPLETELY_LOST, ch);
		return;
	}

	if (ch->in_room == location)
		return;

	if (IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
	||   IS_AFFECTED(ch, AFF_CURSE) 
	||   IS_RAFFECTED(ch->in_room, RAFF_CURSE)) {
		char_nputs(GODS_FORSAKEN_YOU, ch);
		return;
	}

	if ((victim = ch->fighting) != NULL) {
		char_nputs(YOU_ARE_STILL_FIGHTING, ch);
		return;
	}

	ch->move /= 2;
	act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING,
		    clan_table[ch->clan].string_vanishes);
	char_from_room(ch);
	char_to_room(ch, location);
	act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING, 
			N_APPEARS_IN_THE_ROOM);
	do_look(ch, "auto");
	
	if (ch->pet != NULL) {
	 	char_from_room(ch->pet);
		char_to_room(ch->pet, location);
		do_look(ch->pet, "auto");
	}

	af.type      = gsn_clan_recall;
	af.level     = ch->level;
	af.duration  = 5;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = 0;
	affect_to_char(ch, &af);

	return;
}

void do_escape(CHAR_DATA *ch, const char *argument)
{
	ROOM_INDEX_DATA *was_in;
	ROOM_INDEX_DATA *now_in;
	CHAR_DATA *victim;
	char arg[MAX_INPUT_LENGTH];
	int door;


	if ((victim = ch->fighting) == NULL) {
		  if (ch->position == POS_FIGHTING)
		      ch->position = POS_STANDING;
		char_nputs(ARENT_FIGHTING, ch);
		return;
	}

	argument = one_argument(argument, arg);

	if (arg[0] == '\0') {
		char_nputs(ESCAPE_WHAT_DIR, ch);
		return;
	}

	if (MOUNTED(ch)) {
		  char_nputs(CANT_ESCAPE_MOUNTED, ch);
		  return;
	}
	if (RIDDEN(ch)) {
		  char_nputs(CANT_ESCAPE_RIDDEN, ch);
		  return;
	}

	if (!IS_NPC(ch) && ch->level <
			skill_table[gsn_escape].skill_level[ch->class]) {
		char_nputs(TRY_FLEE, ch);
		return;
	}

	was_in = ch->in_room;
	while (1) {
	 if ((door = find_exit(ch, arg)) >= 0) {
		EXIT_DATA *pexit;

		if ((pexit = was_in->exit[door]) == 0
		||   pexit->u1.to_room == NULL
		|| (IS_SET(pexit->exit_info, EX_CLOSED) 
		&& (!IS_AFFECTED(ch, AFF_PASS_DOOR) 
		|| IS_SET(pexit->exit_info,EX_NOPASS))
		&&   !IS_TRUSTED(ch,ANGEL))
		|| (IS_SET(pexit->exit_info , EX_NOFLEE))
		|| (IS_NPC(ch)
		&&   IS_SET(pexit->u1.to_room->room_flags, ROOM_NOMOB))) {
			char_nputs(SOMETHING_PREVENTS_ESCAPE, ch); 
			return;
		}

		if (number_percent() > get_skill(ch,gsn_escape)) {
			char_nputs(ESCAPE_FAILED, ch);
			check_improve(ch,gsn_escape,FALSE,1);	
			return;
		}

		check_improve(ch,gsn_escape,TRUE,1);	
		move_char(ch, door, FALSE);
		if ((now_in = ch->in_room) == was_in)
			continue;

		ch->in_room = was_in;
		act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING,
				N_ESCAPED);
		ch->in_room = now_in;

		if (!IS_NPC(ch)) {
			char_nputs(YOU_ESCAPED_FROM_COMBAT, ch);
			if (ch->level < LEVEL_HERO) {
				char_nprintf(ch, YOU_LOSE_D_EXPS, 10);
				gain_exp(ch, -10);
			}
		}
		else
			ch->last_fought = NULL;  /* Once fled, 
						    the mob will not go after */

		stop_fighting(ch, TRUE);
		return;
	 } else 
		char_nputs(WRONG_DIRECTION, ch);
		break;
	}
	char_nputs(COULDNT_ESCAPE, ch);
	return;
}

void do_layhands(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *victim;
	AFFECT_DATA af;

	if (IS_NPC(ch) ||
	    ch->level < skill_table[gsn_lay_hands].skill_level[ch->class]) {
		char_nputs(CANT_LAY_HANDS, ch);
		return;
	}

	if ((victim = get_char_room(ch,argument)) == NULL) {
		char_nputs(THEY_ARENT_HERE, ch);
		return;
	}

	if (is_affected(ch, gsn_lay_hands)) {
		 char_nputs(CANT_CONCENTRATE_ENOUGH, ch);
		 return;
	}
	WAIT_STATE(ch,skill_table[gsn_lay_hands].beats);

	af.type = gsn_lay_hands;
	af.where = TO_AFFECTS;
	af.level = ch->level;
	af.duration = 2;
	af.location = APPLY_NONE;
	af.modifier = 0;
	af.bitvector = 0;
	affect_to_char (ch, &af);

	victim->hit = UMIN(victim->hit + ch->level * 2, victim->max_hit);
	update_pos(victim);
	char_nputs(WARM_FEELING, victim);
	if (ch != victim)
		char_nputs(OK, ch);
	check_improve(ch,gsn_lay_hands,TRUE,1);

}

int mount_success(CHAR_DATA *ch, CHAR_DATA *mount, int canattack)
{
  int percent;
  int success;

  percent = number_percent() + (ch->level < mount->level ? 
		      (mount->level - ch->level) * 3 : 
		      (mount->level - ch->level) * 2);

  if (!ch->fighting)
	percent -= 25;

  if (!IS_NPC(ch) && IS_DRUNK(ch)) {
	percent += get_skill(ch,gsn_riding) / 2;
	char_nputs(MOUNT_DRUNKEN, ch);
  }

  success = percent - get_skill(ch,gsn_riding);
  if(success <= 0) { /* Success */
	check_improve(ch, gsn_riding, TRUE, 1);
	return(1);
  } else {
	check_improve(ch, gsn_riding, FALSE, 1);
	if (success >= 10 && MOUNTED(ch) == mount) {
		act_nprintf(ch, NULL, mount, TO_CHAR, POS_DEAD, 
				YOU_FALL_OFF_N);
		act_nprintf(ch, NULL, mount, TO_ROOM, POS_RESTING, 
				N_FALLS_OFF_N);
		act_nprintf(ch, NULL, mount, TO_VICT, POS_SLEEPING, 
				N_FALLS_OFF_YOU);

		ch->riding = FALSE;
		mount->riding = FALSE;
		if (ch->position > POS_STUNNED) 
			ch->position=POS_SITTING;
	
	/*  if (ch->hit > 2) { */
		  ch->hit -= 5;
		  update_pos(ch);
		
	}
	if (success >= 40 && canattack) {
		act_nprintf(ch, NULL, mount, TO_CHAR, POS_DEAD,
				N_DOESNT_LIKE_YOU);
		act_nprintf(ch, NULL, mount, TO_ROOM, POS_RESTING,
				N_DOESNT_LIKE_N);
		act_nprintf(ch, NULL, mount, TO_VICT, POS_SLEEPING,
				YOU_DONT_LIKE_N);

		act_nprintf(ch, NULL, mount, TO_CHAR, POS_DEAD, 
				N_SNARLS_YOU);
		act_nprintf(ch, NULL, mount, TO_ROOM, POS_RESTING,
				N_SNARLS_N);
		act_nprintf(ch, NULL, mount, TO_VICT, POS_SLEEPING,
				YOU_SNARL_N);  

		damage(mount, ch, number_range(1, mount->level),
			gsn_kick, DAM_BASH, TRUE);
	}
  }
  return 0;
}

/*
 * It is not finished yet to implement all.
 */
void do_mount(CHAR_DATA *ch, const char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  struct char_data *mount;

  argument = one_argument(argument, arg);

  if (IS_NPC(ch)) 
	return;

  if (arg[0] == '\0' && ch->mount && ch->mount->in_room == ch->in_room) {
	mount = ch->mount;
  } else if (arg[0] == '\0') {
	char_nputs(MOUNT_WHAT, ch);
	return;
  }

 if (!(mount = get_char_room(ch, arg))) {
	char_nputs(YOU_DONT_SEE_THAT, ch);
	return;
  }
 
  if (!SKILL_OK(ch,gsn_riding)) {
	char_nputs(DONT_KNOW_RIDE, ch);
	return;
  } 

  if (!IS_NPC(mount) 
		|| !IS_SET(mount->act,ACT_RIDEABLE)
		  || IS_SET(mount->act,ACT_NOTRACK)) { 
	char_nputs(CANT_RIDE_THAT, ch); 
	return;
  }
  
  if (mount->level - 5 > ch->level) {
	char_nputs(BEAST_TOO_POWERFUL, ch);
	return;
  }    

  if((mount->mount) && (!mount->riding) && (mount->mount != ch)) {
	char_nprintf(ch, S_BELONGS_TO_S, mlstr_cval(mount->short_descr, ch),
		             mount->mount->name);
	return;
  } 

  if (mount->position < POS_STANDING) {
	char_nputs(MOUNT_MUST_STAND, ch);
	return;
  }

  if (RIDDEN(mount)) {
	char_nputs(ALREADY_RIDDEN, ch);
	return;
  } else if (MOUNTED(ch)) {
	char_nputs(ALREADY_RIDING, ch);
	return;
  }

  if(!mount_success(ch, mount, TRUE)) {
	char_nputs(FAIL_TO_MOUNT, ch);  
	return; 
  }

  act_nprintf(ch, NULL, mount, TO_CHAR, POS_DEAD, YOU_HOP_ON_N);
  act_nprintf(ch, NULL, mount, TO_NOTVICT, POS_RESTING, N_HOPS_ON_N);
  act_nprintf(ch, NULL, mount, TO_VICT, POS_SLEEPING, N_HOPS_ON_YOU);
 
  ch->mount = mount;
  ch->riding = TRUE;
  mount->mount = ch;
  mount->riding = TRUE;
  
  /* No sneaky people on mounts */
  affect_strip(ch, gsn_sneak);
  REMOVE_BIT(ch->affected_by, AFF_SNEAK);
  REMOVE_BIT(ch->affected_by, AFF_HIDE);
  affect_strip(ch, gsn_fade);
  REMOVE_BIT(ch->affected_by, AFF_FADE);
  affect_strip(ch, gsn_imp_invis);
  REMOVE_BIT(ch->affected_by, AFF_IMP_INVIS);
}

void do_dismount(CHAR_DATA *ch, const char *argument)
{
  struct char_data *mount;

  if(MOUNTED(ch)) {
	mount = MOUNTED(ch);

	act_nprintf(ch, NULL, mount, TO_CHAR, POS_DEAD, YOU_DISMOUNT_N);
	act_nprintf(ch, NULL, mount, TO_NOTVICT, POS_RESTING, 
				N_DISMOUNTS_N);
	act_nprintf(ch, NULL, mount, TO_VICT, POS_SLEEPING, 
				N_DISMOUNTS_YOU);

	ch->riding = FALSE;
	mount->riding = FALSE;
  } else {
	char_nputs(YOU_ARENT_MOUNTED, ch);
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
				act_nprintf(victim, arrow, NULL, TO_CHAR, 
					    POS_RESTING, P_STRIKES_YOU);
				act_nprintf(ch, arrow, victim, TO_CHAR, 
					    POS_DEAD, YOU_P_STRIKES_N);
				if (ch->in_room == victim->in_room)
				    act_nprintf(ch, arrow, victim, TO_NOTVICT, 
					       POS_RESTING, N_P_STRIKES_N);
				else {
					act_nprintf(ch, arrow, victim, TO_ROOM, 
						   POS_RESTING, NS_P_STRIKES_N);
					act_nprintf(victim, arrow, NULL, 
					     TO_ROOM, POS_RESTING, P_STRIKES_N);
				}
				if (is_safe(ch, victim) || (IS_NPC(victim) 
				    && IS_SET(victim->act, ACT_NOTRACK))) {
					act("$p falls from $n doing no visible damage...",victim,arrow,NULL,TO_ALL);
					act("$p falls from $n doing no visible damage...",ch,arrow,NULL,TO_CHAR);
					obj_to_room(arrow, victim->in_room);
				} else {
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
		            send_to_char("You feel poison coursing through your veins.",
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

			damage(ch, victim,dam,sn,DAM_PIERCE,TRUE);
			path_to_track(ch,victim,door);

		    }
		    return 1;
		  }
		  else {
		 	  obj_to_room(arrow,victim->in_room);
		          act("$p sticks in the ground at your feet!",victim,arrow,NULL, TO_ALL);
		          return 0;
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
 return 0;
}
		

void do_shoot(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *victim;
	OBJ_DATA *wield;
	OBJ_DATA *arrow; 
	char arg1[512],arg2[512];
	bool success;
	int chance,direction;
	int range = (ch->level / 10) + 1;
	
   if (IS_NPC(ch)) return; /* Mobs can't use bows */

   if (!SKILL_OK(ch,gsn_bow))
		{
		  send_to_char("You don't know how to shoot.\n\r",ch);
		  return;
		}

   argument=one_argument(argument, arg1);
   one_argument(argument, arg2);

   if (arg1[0] == '\0' || arg2[0] == '\0')
	{
		send_to_char("Shoot what diretion and whom?\n\r", ch);
		return;
	}

	if (ch->fighting)
	{
		send_to_char("You cannot concentrate on shooting arrows.\n\r",ch);
		return;
	}

   direction = find_exit(ch, arg1);

   if (direction<0 || direction > 5) 
		{
		 send_to_char("Shoot which direction and whom?\n\r",ch);
		 return;
		}
		
	if ((victim = find_char(ch, arg2, direction, range)) == NULL)
		return;

	if (!IS_NPC(victim) && victim->desc == NULL)
	{
		send_to_char("You can't do that.\n\r", ch);
		return;
	}

	if (victim == ch)
	{
		send_to_char("That's pointless.\n\r", ch);
		return;
	}

	if (is_safe(ch,victim))
	{
		char_printf(ch,"Gods protect %s.\n\r",victim->name);
		return;
	}

   wield = get_eq_char(ch, WEAR_WIELD);
   arrow = get_eq_char(ch, WEAR_HOLD);    

   if (!wield || wield->item_type!=ITEM_WEAPON || wield->value[0]!=WEAPON_BOW)
		{
		 send_to_char("You need a bow to shoot!\n\r",ch);
		 return;    	
		}

   if (get_eq_char(ch,WEAR_SECOND_WIELD) || get_eq_char(ch,WEAR_SHIELD))
		{
		 send_to_char("Your second hand should be free!\n\r",ch);
		 return;    	
		}

   if (!arrow)
		{
		 send_to_char("You need an arrow holding for your ammunition!\n\r",ch);
		 return;    	
		}	
		
	if (arrow->item_type!=ITEM_WEAPON || arrow->value[0] != WEAPON_ARROW) 
		{
		 send_to_char("That's not the right kind of arrow!\n\r",ch);
		 return;
		}
		
	
	WAIT_STATE(ch, skill_table[gsn_bow].beats);
   
	chance = (get_skill(ch,gsn_bow) - 50) * 2;
	if (ch->position == POS_SLEEPING)
		chance += 40;
	if (ch->position == POS_RESTING)
		chance += 10;
	if (victim->position == POS_FIGHTING)
		chance -= 40;
	chance += GET_HITROLL(ch);

	act_printf(ch, arrow, NULL, TO_CHAR, POS_RESTING,
		   "You shoot $p to %s.", dir_name[ direction ]);
	act_printf(ch, arrow, NULL, TO_ROOM, POS_RESTING,
		   "$n shoots $p to %s.", dir_name[ direction ]);

	obj_from_char(arrow);
	success = send_arrow(ch,victim,arrow,direction,chance,
			dice(wield->value[1],wield->value[2]));
	check_improve(ch,gsn_bow,TRUE,1);
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
	if (ch->class != CLASS_VAMPIRE)
	{
	 char_nputs(HUH, ch);
	 return;
	}
	 
	if (!IS_VAMPIRE(ch))
	{
	 char_nputs(ALREADY_HUMAN, ch);
	 return;
	}

   affect_strip(ch, gsn_vampire);
   REMOVE_BIT(ch->act,PLR_VAMPIRE);
   char_nputs(RETURN_TO_SIZE, ch);
   return;
}


void do_throw_spear(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *victim;
	OBJ_DATA *spear;
	char arg1[512],arg2[512];
	bool success;
	int chance,direction;
	int range = (ch->level / 10) + 1;
	
   if (IS_NPC(ch)) return; /* Mobs can't shoot spears */

   if (!SKILL_OK(ch,gsn_spear))
		{
		  send_to_char("You don't know how to throw a spear.\n\r",ch);
		  return;
		}

   argument=one_argument(argument, arg1);
   one_argument(argument, arg2);

   if (arg1[0] == '\0' || arg2[0] == '\0')
	{
		send_to_char("Throw spear what diretion and whom?\n\r", ch);
		return;
	}

	if (ch->fighting)
	{
		send_to_char("You cannot concentrate on throwing spear.\n\r",ch);
		return;
	}

   direction = find_exit(ch, arg1);

   if (direction<0 || direction > 5) 
		{
		 send_to_char("Throw which direction and whom?\n\r",ch);
		 return;
		}
		
	if ((victim = find_char(ch, arg2, direction, range)) == NULL)
		return;

	if (!IS_NPC(victim) && victim->desc == NULL)
	{
		send_to_char("You can't do that.\n\r", ch);
		return;
	}

	if (victim == ch)
	{
		send_to_char("That's pointless.\n\r", ch);
		return;
	}

	if (is_safe(ch,victim))
	{
		char_printf(ch,"Gods protect %s.\n\r",victim->name);
		return;
	}

   spear = get_eq_char(ch, WEAR_WIELD);

   if (!spear || spear->item_type!=ITEM_WEAPON || spear->value[0]!=WEAPON_SPEAR)
		{
		 send_to_char("You need a spear to throw!\n\r",ch);
		 return;    	
		}

   if (get_eq_char(ch,WEAR_SECOND_WIELD) || get_eq_char(ch,WEAR_SHIELD))
		{
		 send_to_char("Your second hand should be free!\n\r",ch);
		 return;    	
		}

	
	WAIT_STATE(ch, skill_table[gsn_spear].beats);
   
	chance = (get_skill(ch,gsn_spear) - 50) * 2;
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
	check_improve(ch,gsn_spear,TRUE,1);
}


/*
 * guild_check - == 0 - the room is not a guild
 *		  > 0 - the room is guild and ch is allowed there
 *		  < 0 - the room is guild and ch is not allowed there
 */
int guild_check(CHAR_DATA *ch, ROOM_INDEX_DATA *room)
{
	int class = CLASS_NONE;
	int iClass, iGuild;

	for (iClass = 0; iClass < MAX_CLASS; iClass++)
		for (iGuild = 0; class_table[iClass].guild[iGuild]; iGuild++)	
		    	if (room->vnum == class_table[iClass].guild[iGuild]) {
				if (iClass == ch->class)
					return 1;
				class = iClass;
			}

	if (class == CLASS_NONE)
		return 0;

	if (IS_IMMORTAL(ch))
		return 1;

	return -1;
}


/* random room generation procedure */
ROOM_INDEX_DATA  *get_random_room(CHAR_DATA *ch)
{
	ROOM_INDEX_DATA *room;

	for (; ;)
	{
	    room = get_room_index(number_range(0, 65535));
	    if (room != NULL)
	    if (can_see_room(ch,room)
	&&   !room_is_private(room)
	    &&   !IS_SET(room->room_flags, ROOM_PRIVATE)
	    &&   !IS_SET(room->room_flags, ROOM_SOLITARY) 
	&&   !IS_SET(room->room_flags, ROOM_SAFE) 
	&&   (IS_NPC(ch) || IS_SET(ch->act,ACT_AGGRESSIVE) 
	||   !IS_SET(room->room_flags,ROOM_LAW | ROOM_SAFE)))
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
		send_to_char("Nope, can't do it.\n\r",ch);
		return;
	}


	old_room = ch->in_room;
	portal = get_obj_list(ch, argument, ch->in_room->contents);
	
	if (portal == NULL) {
		send_to_char("You don't see that here.\n\r",ch);
		return;
	}

	if (portal->item_type != ITEM_PORTAL 
	||  (IS_SET(portal->value[1],EX_CLOSED) && !IS_TRUSTED(ch, ANGEL))) {
		send_to_char("You can't seem to find a way in.\n\r", ch);
		return;
	}

	if (!IS_TRUSTED(ch,ANGEL) && !IS_SET(portal->value[2],GATE_NOCURSE)
	&&  (IS_AFFECTED(ch,AFF_CURSE) 
	||  IS_SET(old_room->room_flags,ROOM_NO_RECALL) 
	||  IS_RAFFECTED(old_room,RAFF_CURSE))) {
		send_to_char("Something prevents you from leaving...\n\r",ch);
		return;
	}

	if (IS_SET(portal->value[2],GATE_RANDOM) || portal->value[3] == -1) {
		location = get_random_room(ch);
		portal->value[3] = location->vnum; /* keeps record */
	}
	else if (IS_SET(portal->value[2], GATE_BUGGY) && (number_percent() < 5))
		location = get_random_room(ch);
	else
		location = get_room_index(portal->value[3]);

	if (location == NULL
	||  location == old_room
	||  !can_see_room(ch,location) 
	||  (room_is_private(location) && !IS_TRUSTED(ch,IMPLEMENTOR))) {
		act("$p doesn't seem to go anywhere.", ch, portal,NULL,TO_CHAR);
		return;
	}

	if (IS_NPC(ch) && IS_SET(ch->act,ACT_AGGRESSIVE)
	&&  IS_SET(location->room_flags, ROOM_LAW | ROOM_SAFE)) {
	        send_to_char("Something prevents you from leaving...\n\r",ch);
	        return;
	}

	/* XXX */
	act_nprintf(ch, portal, NULL, TO_ROOM, POS_RESTING,
		   MOUNTED(ch) ? HERA_STEPS_INTO_RIDING_ON :
				 HERA_STEPS_INTO,
		   MOUNTED(ch) ? mlstr_cval(MOUNTED(ch)->short_descr, ch) :
				 NULL);
	
	act(IS_SET(portal->value[2], GATE_NORMAL_EXIT) ?
	    "You enter $p." :
	    "You walk through $p and find yourself somewhere else...",
	    ch, portal, NULL, TO_CHAR); 

	mount = MOUNTED(ch);
	char_from_room(ch);
	char_to_room(ch, location);

	if (IS_SET(portal->value[2], GATE_GOWITH)) {/* take the gate along */
		obj_from_room(portal);
		obj_to_room(portal,location);
	}

	if (IS_SET(portal->value[2],GATE_NORMAL_EXIT))
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
		send_to_char("You don't know how to set traps.\n\r",ch);
		return;
	}

	if (!ch->in_room)	return;

	if (IS_SET(ch->in_room->room_flags, ROOM_LAW))
	{
	 send_to_char("A mystical power protects the room.\n\r",ch);
	 return;
	}

	WAIT_STATE(ch, skill_table[gsn_settraps].beats);

	if (IS_NPC(ch) || number_percent() <  chance * 7 / 10) {
	  AFFECT_DATA af,af2;

	  check_improve(ch,gsn_settraps,TRUE,1);

	  if (is_affected_room(ch->in_room, gsn_settraps))
	  {
	send_to_char("This room has already trapped.\n\r",ch);
	return;
	   }

	  if (is_affected(ch,gsn_settraps))
	  {
	send_to_char("This skill is used too recently.\n\r",ch);
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
	  send_to_char("You set the room with your trap.\n\r", ch);
	  act("$n set the room with $s trap.",ch,NULL,NULL,TO_ROOM);
	  return;
	}
	else check_improve(ch,gsn_settraps,FALSE,1);

   return;
}

