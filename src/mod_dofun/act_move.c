/*
 * $Id: act_move.c,v 1.50 1998-06-12 14:25:57 fjoe Exp $
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

   if (RIDDEN(ch) && !IS_NPC(ch->mount)) 
   {
	move_char(ch->mount,door,follow);
	return;
   }

   if (CAN_DETECT(ch,ADET_WEB) 
		|| (MOUNTED(ch) && CAN_DETECT(ch->mount,ADET_WEB)))
	{
		WAIT_STATE(ch,PULSE_VIOLENCE);
		if (number_percent() < str_app[IS_NPC(ch) ? 20 : get_curr_stat(ch,STAT_STR)].tohit * 5)
		{
		 affect_strip(ch,gsn_web);
		 REMOVE_BIT(ch->detection,ADET_WEB);
		 send_to_char(msg(MOVE_WHEN_YOU_ATTEMPT_YOU_BREAK_WEBS, ch), ch);
		 act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING, MOVE_N_BREAKS_THE_WEBS);
		}
		else 
		{
		 send_to_char(msg(MOVE_YOU_ATTEMPT_WEBS_HOLD_YOU, ch), ch);
		 act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING, MOVE_N_STRUGGLES_VAINLY_AGAINST_WEBS);
		 return; 
		}
	}

	if (door < 0 || door > 5)
	{
		bug("Do_move: bad door %d.", door);
		return;
	}

	if ((IS_AFFECTED(ch, AFF_HIDE) && !IS_AFFECTED(ch, AFF_SNEAK))  
		|| (IS_AFFECTED(ch, AFF_FADE) && !IS_AFFECTED(ch, AFF_SNEAK)) )
	{
		REMOVE_BIT(ch->affected_by, AFF_HIDE);
		send_to_char(msg(MOVE_YOU_STEP_OUT_SHADOWS, ch), ch);
		act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING, MOVE_N_STEPS_OUT_OF_SHADOWS);
	}
	if (IS_AFFECTED(ch, AFF_CAMOUFLAGE))  {
		int chance;

		if (IS_NPC(ch)
		||  (chance = get_skill(ch, gsn_move_camf)) == 0) {
			REMOVE_BIT(ch->affected_by, AFF_CAMOUFLAGE);
			send_to_char(msg(MOVE_YOU_STEP_OUT_COVER, ch), ch);
			act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING, MOVE_N_STEPS_OUT_COVER);
		}	    
		else if (number_percent() < chance)
			check_improve(ch,gsn_move_camf,TRUE,5);
		else {
			REMOVE_BIT(ch->affected_by, AFF_CAMOUFLAGE);
			send_to_char(msg(MOVE_YOU_STEP_OUT_COVER, ch), ch);
			act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING, MOVE_N_STEPS_OUT_COVER);
			check_improve(ch, gsn_move_camf, FALSE, 5);
		}	    
	}

	in_room = ch->in_room;
	if ((pexit   = in_room->exit[door]) == NULL
	||   (to_room = pexit->u1.to_room  ) == NULL 
	||	 !can_see_room(ch,pexit->u1.to_room))
	{
		send_to_char(msg(MOVE_ALAS_YOU_CANNOT_GO, ch), ch);
		return;
	}

	if (IS_ROOM_AFFECTED(in_room,AFF_ROOM_RANDOMIZER))
		{
		 int d0;
		 while (1)
		 {
		  d0 = number_range(0,5);
		    if ((pexit   = in_room->exit[d0]) == NULL
		     ||   (to_room = pexit->u1.to_room  ) == NULL 
		     ||	 !can_see_room(ch,pexit->u1.to_room))
		   continue;	  
		  d0 = door;
		  break;
		 }
		}

	if (IS_SET(pexit->exit_info, EX_CLOSED) 
		&& (!IS_AFFECTED(ch, AFF_PASS_DOOR) || IS_SET(pexit->exit_info,EX_NOPASS))
		&&   !IS_TRUSTED(ch,ANGEL))
	{
		if (IS_AFFECTED(ch, AFF_PASS_DOOR) && IS_SET(pexit->exit_info, EX_NOPASS))  {
  	  act_nprintf(ch, NULL, pexit->keyword, TO_CHAR, POS_RESTING, MOVE_YOU_FAILED_TO_PASS);
		  act_nprintf(ch, NULL, pexit->keyword, TO_ROOM, POS_RESTING, MOVE_N_TRIES_TO_PASS_FAILED);
		}
		else
		  act_nprintf(ch, NULL, pexit->keyword, TO_CHAR, POS_RESTING, MOVE_THE_D_IS_CLOSED);
		  return;
	}

	if (IS_AFFECTED(ch, AFF_CHARM)
	&&   ch->master != NULL
	&&   in_room == ch->master->in_room)
	{
		send_to_char(msg(MOVE_WHAT_YOU_LEAVE, ch), ch);
		return;
	}

/*    if (!is_room_owner(ch,to_room) && room_is_private(to_room))	*/
	if (room_is_private(to_room))
	{
		send_to_char(msg(MOVE_ROOM_IS_PRIVATE, ch), ch);
		return;
	}

	if (MOUNTED(ch)) 
	{
		  if (MOUNTED(ch)->position < POS_FIGHTING) 
		  {
		   send_to_char(msg(MOVE_MOUNT_MUST_STAND, ch), ch);
		   return; 
		  }
		  if (!mount_success(ch, MOUNTED(ch), FALSE)) 
		  {
		   send_to_char(msg(MOVE_MOUNT_REFUSES_GO_THAT_WAY, ch), ch);
		   return;
		  }
	}

	if (!IS_NPC(ch))
	{
		int iClass, iGuild;
		int move;


		for (iClass = 0; iClass < MAX_CLASS; iClass++)
		    for (iGuild = 0; iGuild < MAX_GUILD; iGuild ++)	
		    	if (to_room->vnum == class_table[iClass].guild[iGuild] 
			     && !IS_IMMORTAL(ch))
		    	{
			    if (iClass != ch->class)  {
			      send_to_char(msg(MOVE_YOU_ARENT_ALLOWED_THERE, ch), ch);
			      return;
			    }
		              if (ch->last_fight_time != -1 &&
		                current_time - ch->last_fight_time < FIGHT_DELAY_TIME)
		              {
		                send_to_char(msg(MOVE_YOU_FEEL_TOO_BLOODY, ch), ch);
		                return;
		              }
		        }

		if (in_room->sector_type == SECT_AIR
		||   to_room->sector_type == SECT_AIR)
		{
		      if (MOUNTED(ch)) 
		      {
		          if(!IS_AFFECTED(MOUNTED(ch), AFF_FLYING)) 
		          {
		              send_to_char(msg(MOVE_YOUR_MOUNT_CANT_FLY, ch), ch);
		              return;
		          }
		      } 
		     else if (!IS_AFFECTED(ch, AFF_FLYING) && !IS_IMMORTAL(ch))
		    {
		      send_to_char(msg(MOVE_YOU_CANT_FLY, ch), ch);
		      return;
		      } 
		}

		if ((in_room->sector_type == SECT_WATER_NOSWIM
		||    to_room->sector_type == SECT_WATER_NOSWIM)
		&&    (MOUNTED(ch) && !IS_AFFECTED(MOUNTED(ch),AFF_FLYING)))
		{
		    send_to_char(msg(MOVE_YOU_CANT_TAKE_MOUNT_THERE, ch),ch);
		    return; 
		  }  

		if ((in_room->sector_type == SECT_WATER_NOSWIM
		||    to_room->sector_type == SECT_WATER_NOSWIM)
  	&&    (!MOUNTED(ch) && !IS_AFFECTED(ch,AFF_FLYING)))
		{
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
			send_to_char(msg(MOVE_YOU_NEED_A_BOAT, ch), ch);
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
		    send_to_char(msg(MOVE_YOU_TOO_EXHAUSTED, ch), ch);
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
			act_nprintf(ch, NULL, dir_name[door], TO_ROOM, POS_RESTING, 
				MOVE_LEAVES_RIDING_ON, MOUNTED(ch)->short_descr);
		      else
			act_nprintf(ch, NULL, dir_name[door], TO_ROOM, POS_RESTING, 
				MOVE_LEAVES);
		      check_improve(ch,gsn_quiet_movement,TRUE,1);
		    }
		else 
		  {
		     if (MOUNTED(ch))
			act_nprintf(ch, NULL, dir_name[door], TO_ROOM, POS_RESTING, 
				MOVE_LEAVES_T_RIDING_ON, MOUNTED(ch)->short_descr);
  	   else
			act_nprintf(ch, NULL, dir_name[door], TO_ROOM, POS_RESTING, 
				MOVE_LEAVES_T);
		  }
	 }

	if (IS_AFFECTED(ch, AFF_CAMOUFLAGE) && to_room->sector_type != SECT_FIELD
		 && to_room->sector_type != SECT_FOREST &&
		     to_room->sector_type != SECT_MOUNTAIN &&
		     to_room->sector_type != SECT_HILLS) 
	 {
		REMOVE_BIT(ch->affected_by, AFF_CAMOUFLAGE);
		send_to_char(msg(MOVE_YOU_STEP_OUT_COVER, ch), ch);
		act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING, MOVE_N_STEPS_OUT_COVER);
	 }	    

	mount = MOUNTED(ch);
	char_from_room(ch);
	char_to_room(ch, to_room);

	/* room record for tracking */
	if (!IS_NPC(ch) && ch->in_room)
		room_record(ch->name,in_room, door);


	if (!IS_AFFECTED(ch, AFF_SNEAK)
	&&   ch->invis_level < LEVEL_HERO)
		{
		if (mount)
		     act_nprintf(ch, NULL, mount,TO_ROOM, POS_RESTING, MOVE_ARRIVED_RIDING);
		else act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING, MOVE_ARRIVED);
		}

	do_look(ch, "auto");

	if (mount)
		{
		 char_from_room(mount);
		 char_to_room(mount, to_room);
  	 ch->riding = TRUE;
  	 mount->riding = TRUE;
		}

	if (in_room == to_room) /* no circular follows */
		return;


	for (fch = to_room->people,room_has_pc = FALSE;fch != NULL; fch = fch_next)
		{
		  fch_next = fch->next_in_room;
		  if (!IS_NPC(fch))
		    room_has_pc = TRUE;
		}

	for (fch = to_room->people;fch!=NULL;fch = fch_next) {
		fch_next = fch->next_in_room;

		/* greet progs for items carried by people in room */
		for (obj = fch->carrying;room_has_pc && obj != NULL;
		     obj = obj->next_content)
		  {
		    if (IS_SET(obj->progtypes,OPROG_GREET))
		      (obj->pIndexData->oprogs->greet_prog) (obj,ch);
		  }

		/* greet programs for npcs  */
		if (room_has_pc && IS_SET(fch->progtypes,MPROG_GREET))
		  (fch->pIndexData->mprogs->greet_prog) (fch,ch);
	}

	/* entry programs for items */
	for (obj = ch->carrying;room_has_pc && obj!=NULL;obj=obj->next_content)
		{
		  if (IS_SET(obj->progtypes,OPROG_ENTRY))
		    (obj->pIndexData->oprogs->entry_prog) (obj);
		}


	for (fch = in_room->people; fch != NULL; fch = fch_next)
	{
		fch_next = fch->next_in_room;

		if (fch->master == ch && IS_AFFECTED(fch,AFF_CHARM) 
		&&   fch->position < POS_STANDING)
		    do_stand(fch,"");

		if (fch->master == ch && fch->position == POS_STANDING 
		&&   can_see_room(fch,to_room))
		{

		    if (IS_SET(ch->in_room->room_flags,ROOM_LAW)
		    &&  (IS_NPC(fch) && IS_SET(fch->act,ACT_AGGRESSIVE)))
		    {
			act_nprintf(ch,NULL,fch,TO_CHAR, POS_DEAD, MOVE_YOU_CANT_BRING_N_CITY);
			act_nprintf(fch,NULL,NULL,TO_CHAR, POS_RESTING, MOVE_YOU_ARENT_ALLOWED_CITY);
			continue;
		    }

		    act_nprintf(fch, NULL, ch, TO_CHAR, POS_DEAD, MOVE_YOU_FOLLOW_N);
		    move_char(fch, door, TRUE);
		}
	}

	for (obj = ch->in_room->contents;room_has_pc && obj != NULL;	
		   obj = obj->next_content)
		{
		  if (IS_SET(obj->progtypes,OPROG_GREET))
		    (obj->pIndexData->oprogs->greet_prog) (obj,ch);
		}

	if (IS_SET(ch->progtypes,MPROG_ENTRY))
		(ch->pIndexData->mprogs->entry_prog) (ch);

	return;
}



void do_north(CHAR_DATA *ch, char *argument)
{
	move_char(ch, DIR_NORTH, FALSE);
	return;
}



void do_east(CHAR_DATA *ch, char *argument)
{
	move_char(ch, DIR_EAST, FALSE);
	return;
}



void do_south(CHAR_DATA *ch, char *argument)
{
	move_char(ch, DIR_SOUTH, FALSE);
	return;
}



void do_west(CHAR_DATA *ch, char *argument)
{
	move_char(ch, DIR_WEST, FALSE);
	return;
}



void do_up(CHAR_DATA *ch, char *argument)
{
	move_char(ch, DIR_UP, FALSE);
	return;
}



void do_down(CHAR_DATA *ch, char *argument)
{
	move_char(ch, DIR_DOWN, FALSE);
	return;
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
	else
	{
		act_nprintf(ch, NULL, arg, TO_CHAR, POS_DEAD, MOVE_I_SEE_NO_EXIT_T_HERE);
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
	else
	{
		for (door = 0; door <= 5; door++)
		{
		    if ((pexit = ch->in_room->exit[door]) != NULL
		    &&   IS_SET(pexit->exit_info, EX_ISDOOR)
		    &&   pexit->keyword != NULL
		    &&   is_name(arg, pexit->keyword))
			return door;
		}
		act_nprintf(ch, NULL, arg, TO_CHAR, POS_DEAD, MOVE_I_SEE_NO_T_HERE);
		return -1;
	}

	if ((pexit = ch->in_room->exit[door]) == NULL)
	{
		act_nprintf(ch, NULL, arg, TO_CHAR, POS_DEAD, MOVE_I_SEE_NO_DOOR_T_HERE);
		return -1;
	}

	if (!IS_SET(pexit->exit_info, EX_ISDOOR))
	{
		send_to_char(msg(MOVE_YOU_CANT_DO_THAT, ch), ch);
		return -1;
	}

	return door;
}

/* scan.c */

void scan_list           args((ROOM_INDEX_DATA *scan_room, CHAR_DATA *ch,
		                         int depth, int door));

void do_scan2(CHAR_DATA *ch, char *argument)
{
	extern char *const dir_name[];
	EXIT_DATA *pExit;
	int door;

	act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING, 
			MOVE_N_LOOKS_ALL_AROUND);
	send_to_char(msg(MOVE_LOOKING_AROUND_YOU_SEE, ch), ch);
	send_to_char(msg(MOVE_SCAN_HERE, ch), ch);
	scan_list(ch->in_room, ch, 0, -1);
	for (door = 0; door < 6; door++) {
		if ((pExit = ch->in_room->exit[door]) == NULL
		|| !pExit->u1.to_room
		|| !can_see_room(ch,pExit->u1.to_room))
			continue;
		char_printf(ch, "{C%s{x:\n\r", dir_name[door]);
		if (IS_SET(pExit->exit_info, EX_CLOSED)) {
			send_to_char(msg(MOVE_SCAN_DOOR_CLOSED, ch), ch);
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

void do_open(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	int door;

	one_argument(argument, arg);

	if (arg[0] == '\0')
	{
		send_to_char(msg(MOVE_OPEN_WHAT, ch), ch);
		return;
	}



	if ((obj = get_obj_here(ch, arg)) != NULL)
	{
 	/* open portal */
		if (obj->item_type == ITEM_PORTAL)
		{
		    if (!IS_SET(obj->value[1], EX_ISDOOR))
		    {
			send_to_char(msg(MOVE_YOU_CANT_DO_THAT, ch),ch);
			return;
		    }

		    if (!IS_SET(obj->value[1], EX_CLOSED))
		    {
			send_to_char(msg(MOVE_ITS_ALREADY_OPEN, ch), ch);
			return;
		    }

		    if (IS_SET(obj->value[1], EX_LOCKED))
		    {
			send_to_char(msg(MOVE_ITS_LOCKED, ch), ch);
			return;
		    }

		    REMOVE_BIT(obj->value[1], EX_CLOSED);
		    act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD, MOVE_YOU_OPEN_P);
		    act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING, MOVE_N_OPENS_P);
		    return;
 	}

		/* 'open object' */
		if (obj->item_type != ITEM_CONTAINER)
		    { send_to_char(msg(MOVE_THATS_NOT_A_CONTAINER, ch), ch); return; }
		if (!IS_SET(obj->value[1], CONT_CLOSED))
		    { send_to_char(msg(MOVE_ITS_ALREADY_OPEN, ch), ch); return; }
		if (!IS_SET(obj->value[1], CONT_CLOSEABLE))
		    { send_to_char(msg(MOVE_YOU_CANT_DO_THAT, ch), ch); return; }
		if (IS_SET(obj->value[1], CONT_LOCKED))
		    { send_to_char(msg(MOVE_ITS_LOCKED, ch), ch); return; }

		REMOVE_BIT(obj->value[1], CONT_CLOSED);
		act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD, MOVE_YOU_OPEN_P);
		act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING, MOVE_N_OPENS_P);
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
		    { send_to_char(msg(MOVE_ITS_ALREADY_OPEN, ch), ch); return; }
		if ( IS_SET(pexit->exit_info, EX_LOCKED))
		    { send_to_char(msg(MOVE_ITS_LOCKED, ch), ch); return; }

		REMOVE_BIT(pexit->exit_info, EX_CLOSED);
		act_nprintf(ch, NULL, pexit->keyword, TO_ROOM, POS_RESTING, MOVE_N_OPENS_THE_D);
		send_to_char(msg(MOVE_OK, ch), ch);

		/* open the other side */
		if ((to_room   = pexit->u1.to_room           ) != NULL
		&&   (pexit_rev = to_room->exit[rev_dir[door]]) != NULL
		&&   pexit_rev->u1.to_room == ch->in_room)
		{
		    CHAR_DATA *rch;

		    REMOVE_BIT(pexit_rev->exit_info, EX_CLOSED);
		    for (rch = to_room->people; rch != NULL; rch = rch->next_in_room)
			act_nprintf(rch, NULL, pexit_rev->keyword, TO_CHAR, POS_RESTING, MOVE_THE_D_OPENS);
		}
		return;
	}
	return;
}



void do_close(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	int door;

	one_argument(argument, arg);

	if (arg[0] == '\0')
	{
		send_to_char(msg(MOVE_CLOSE_WHAT, ch), ch);
		return;
	}



	if ((obj = get_obj_here(ch, arg)) != NULL)
	{
		/* portal stuff */
		if (obj->item_type == ITEM_PORTAL)
		{

		    if (!IS_SET(obj->value[1],EX_ISDOOR)
		    ||   IS_SET(obj->value[1],EX_NOCLOSE))
		    {
			send_to_char(msg(MOVE_YOU_CANT_DO_THAT, ch), ch);
			return;
		    }

		    if (IS_SET(obj->value[1],EX_CLOSED))
		    {
			send_to_char(msg(MOVE_ITS_ALREADY_CLOSED, ch), ch);
			return;
		    }

		    SET_BIT(obj->value[1],EX_CLOSED);
		    act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD, MOVE_YOU_CLOSE_P);
		    act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING, MOVE_N_CLOSES_P);
		    return;
		}

		/* 'close object' */
		if (obj->item_type != ITEM_CONTAINER)
		    { send_to_char(msg(MOVE_THATS_NOT_A_CONTAINER, ch), ch); return; }
		if (IS_SET(obj->value[1], CONT_CLOSED))
		    { send_to_char(msg(MOVE_ITS_ALREADY_CLOSED, ch), ch); return; }
		if (!IS_SET(obj->value[1], CONT_CLOSEABLE))
		    { send_to_char(msg(MOVE_YOU_CANT_DO_THAT, ch), ch); return; }

		SET_BIT(obj->value[1], CONT_CLOSED);
		act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD, MOVE_YOU_CLOSE_P);
		act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING, MOVE_N_CLOSES_P);
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
		    { send_to_char(msg(MOVE_ITS_ALREADY_CLOSED, ch), ch); return; }

		SET_BIT(pexit->exit_info, EX_CLOSED);
		act_nprintf(ch, NULL, pexit->keyword, TO_ROOM, POS_RESTING, MOVE_N_CLOSES_THE_D);
		send_to_char(msg(MOVE_OK, ch), ch);

		/* close the other side */
		if ((to_room   = pexit->u1.to_room           ) != NULL
		&&   (pexit_rev = to_room->exit[rev_dir[door]]) != 0
		&&   pexit_rev->u1.to_room == ch->in_room)
		{
		    CHAR_DATA *rch;

		    SET_BIT(pexit_rev->exit_info, EX_CLOSED);
		    for (rch = to_room->people; rch != NULL; rch = rch->next_in_room)
			act_nprintf(rch, NULL, pexit_rev->keyword, TO_CHAR, POS_RESTING, MOVE_THE_D_CLOSES);
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



void do_lock(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	int door;
	CHAR_DATA *rch;

	one_argument(argument, arg);

	if (arg[0] == '\0')
	{
		send_to_char(msg(MOVE_LOCK_WHAT, ch), ch);
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
			send_to_char(msg(MOVE_YOU_CANT_DO_THAT, ch),ch);
			return;
		    }
		    if (!IS_SET(obj->value[1],EX_CLOSED))
		    {
			send_to_char(msg(MOVE_ITS_NOT_CLOSED, ch),ch);
		 	return;
		    }

		    if (obj->value[4] < 0 || IS_SET(obj->value[1],EX_NOLOCK))
		    {
			send_to_char(msg(MOVE_IT_CANT_BE_LOCKED, ch), ch);
			return;
		    }

		    if (!has_key(ch,obj->value[4]))
		    {
			send_to_char(msg(MOVE_YOU_LACK_THE_KEY, ch), ch);
			return;
		    }

		    if (IS_SET(obj->value[1],EX_LOCKED))
		    {
			send_to_char(msg(MOVE_ITS_ALREADY_LOCKED, ch), ch);
			return;
		    }

		    SET_BIT(obj->value[1],EX_LOCKED);
		    act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD, MOVE_YOU_LOCK_P);
		    act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING, MOVE_N_LOCKS_P);
		    return;
		}

		/* 'lock object' */
		if (obj->item_type != ITEM_CONTAINER)
		    { send_to_char(msg(MOVE_THATS_NOT_A_CONTAINER, ch), ch); return; }
		if (!IS_SET(obj->value[1], CONT_CLOSED))
		    { send_to_char(msg(MOVE_ITS_NOT_CLOSED, ch), ch); return; }
		if (obj->value[2] < 0)
		    { send_to_char(msg(MOVE_IT_CANT_BE_LOCKED, ch), ch); return; }
		if (!has_key(ch, obj->value[2]))
		    { send_to_char(msg(MOVE_YOU_LACK_THE_KEY, ch), ch); return; }
		if (IS_SET(obj->value[1], CONT_LOCKED))
		    { send_to_char(msg(MOVE_ITS_ALREADY_LOCKED, ch), ch); return; }

		SET_BIT(obj->value[1], CONT_LOCKED);
		act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD, MOVE_YOU_LOCK_P);
		act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING, MOVE_N_LOCKS_P);
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
		    { send_to_char(msg(MOVE_ITS_NOT_CLOSED, ch), ch); return; }
		if (pexit->key < 0)
		    { send_to_char(msg(MOVE_IT_CANT_BE_LOCKED, ch), ch); return; }
		if (!has_key(ch, pexit->key) && 
		       !has_key_ground(ch, pexit->key))
		    { send_to_char(msg(MOVE_YOU_LACK_THE_KEY, ch), ch); return; }
		if (IS_SET(pexit->exit_info, EX_LOCKED))
		    { send_to_char(msg(MOVE_ITS_ALREADY_LOCKED, ch), ch); return; }

		SET_BIT(pexit->exit_info, EX_LOCKED);
		send_to_char(msg(MOVE_CLICK, ch), ch);
		act_nprintf(ch, NULL, pexit->keyword, TO_ROOM, POS_RESTING, MOVE_N_LOCKS_THE_D);

		/* lock the other side */
		if ((to_room   = pexit->u1.to_room           ) != NULL
		&&   (pexit_rev = to_room->exit[rev_dir[door]]) != 0
		&&   pexit_rev->u1.to_room == ch->in_room)
		{
		    SET_BIT(pexit_rev->exit_info, EX_LOCKED);
		    for (rch = to_room->people; rch != NULL; rch = rch->next_in_room)
			act_nprintf(rch, NULL, pexit_rev->keyword, TO_CHAR, POS_RESTING, MOVE_THE_D_CLICKS);

		}
		  return;
	}
	return;
}



void do_unlock(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	int door;
	CHAR_DATA *rch;

	one_argument(argument, arg);

	if (arg[0] == '\0')
	{
		send_to_char(msg(MOVE_UNLOCK_WHAT, ch), ch);
		return;
	}


	if ((obj = get_obj_here(ch, arg)) != NULL)
	{
 	/* portal stuff */
		if (obj->item_type == ITEM_PORTAL)
		{
		    if (IS_SET(obj->value[1],EX_ISDOOR))
		    {
			send_to_char(msg(MOVE_YOU_CANT_DO_THAT, ch), ch);
			return;
		    }

		    if (!IS_SET(obj->value[1],EX_CLOSED))
		    {
			send_to_char(msg(MOVE_ITS_NOT_CLOSED, ch),ch);
			return;
		    }

		    if (obj->value[4] < 0)
		    {
			send_to_char(msg(MOVE_IT_CANT_BE_UNLOCKED, ch), ch);
			return;
		    }

		    if (!has_key(ch,obj->value[4]))
		    {
			send_to_char(msg(MOVE_YOU_LACK_THE_KEY, ch), ch);
			return;
		    }

		    if (!IS_SET(obj->value[1],EX_LOCKED))
		    {
			send_to_char(msg(MOVE_ITS_ALREADY_UNLOCKED, ch), ch);
			return;
		    }

		    REMOVE_BIT(obj->value[1],EX_LOCKED);
		    act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD, MOVE_YOU_UNLOCK_P);
		    act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING, MOVE_N_UNLOCKS_P);
		    return;
		}

		/* 'unlock object' */
		if (obj->item_type != ITEM_CONTAINER)
		    { send_to_char(msg(MOVE_THATS_NOT_A_CONTAINER, ch), ch); return; }
		if (!IS_SET(obj->value[1], CONT_CLOSED))
		    { send_to_char(msg(MOVE_ITS_NOT_CLOSED, ch), ch); return; }
		if (obj->value[2] < 0)
		    { send_to_char(msg(MOVE_IT_CANT_BE_UNLOCKED, ch), ch); return; }
		if (!has_key(ch, obj->value[2]))
		    { send_to_char(msg(MOVE_YOU_LACK_THE_KEY, ch), ch); return; }
		if (!IS_SET(obj->value[1], CONT_LOCKED))
		    { send_to_char(msg(MOVE_ITS_ALREADY_UNLOCKED, ch), ch); return; }

		REMOVE_BIT(obj->value[1], CONT_LOCKED);
		act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD, MOVE_YOU_UNLOCK_P);
		act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING, MOVE_N_UNLOCKS_P);
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
		    { send_to_char(msg(MOVE_ITS_NOT_CLOSED, ch), ch); return; }
		if (pexit->key < 0)
		    { send_to_char(msg(MOVE_IT_CANT_BE_UNLOCKED, ch), ch); return; }
		if (!has_key(ch, pexit->key) && 
		       !has_key_ground(ch, pexit->key))
		    { send_to_char(msg(MOVE_YOU_LACK_THE_KEY, ch), ch); return; }
		if (!IS_SET(pexit->exit_info, EX_LOCKED))
		    { send_to_char(msg(MOVE_ITS_ALREADY_UNLOCKED, ch), ch); return; }

		REMOVE_BIT(pexit->exit_info, EX_LOCKED);
		send_to_char(msg(MOVE_CLICK, ch), ch);
		act_nprintf(ch, NULL, pexit->keyword, TO_ROOM, POS_RESTING, MOVE_N_UNLOCKS_THE_D);

		/* unlock the other side */
		if ((to_room   = pexit->u1.to_room           ) != NULL
		&&   (pexit_rev = to_room->exit[rev_dir[door]]) != NULL
		&&   pexit_rev->u1.to_room == ch->in_room)
		{
		    REMOVE_BIT(pexit_rev->exit_info, EX_LOCKED);
		    for (rch = to_room->people; rch != NULL; rch = rch->next_in_room)
			act_nprintf(rch, NULL, pexit_rev->keyword, TO_CHAR, POS_RESTING, MOVE_THE_D_CLICKS);
		}
		  return;
	}
	return;
}



void do_pick(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *gch;
	OBJ_DATA *obj;
	int door;

	one_argument(argument, arg);

	if (arg[0] == '\0')
	{
		send_to_char(msg(MOVE_PICK_WHAT, ch), ch);
		return;
	}

	if (MOUNTED(ch)) 
	{
		  send_to_char(msg(MOVE_CANT_PICK_MOUNTED, ch), ch);
		  return;
	}

	WAIT_STATE(ch, skill_table[gsn_pick_lock].beats);

	/* look for guards */
	for (gch = ch->in_room->people; gch; gch = gch->next_in_room)
	{
		if (IS_NPC(gch) && IS_AWAKE(gch) && ch->level + 5 < gch->level)
		{
		    act_nprintf(ch, NULL, gch, TO_CHAR, POS_RESTING, MOVE_N_IS_STANDING_TOO_CLOSE_TO_LOCK);
		    return;
		}
	}

	if (!IS_NPC(ch) && number_percent() > get_skill(ch,gsn_pick_lock))
	{
		send_to_char(msg(MOVE_YOU_FAILED, ch), ch);
		check_improve(ch,gsn_pick_lock,FALSE,2);
		return;
	}

	if ((obj = get_obj_here(ch, arg)) != NULL)
	{
		/* portal stuff */
		if (obj->item_type == ITEM_PORTAL)
		{
		    if (!IS_SET(obj->value[1],EX_ISDOOR))
		    {	
			send_to_char(msg(MOVE_YOU_CANT_DO_THAT, ch),ch);
			return;
		    }

		    if (!IS_SET(obj->value[1],EX_CLOSED))
		    {
			send_to_char(msg(MOVE_ITS_NOT_CLOSED, ch),ch);
			return;
		    }

		    if (obj->value[4] < 0)
		    {
			send_to_char(msg(MOVE_IT_CANT_BE_UNLOCKED, ch),ch);
			return;
		    }

		    if (IS_SET(obj->value[1],EX_PICKPROOF))
		    {
			send_to_char(msg(MOVE_YOU_FAILED, ch), ch);
			return;
		    }

		    REMOVE_BIT(obj->value[1],EX_LOCKED);
		    act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD, MOVE_YOU_PICK_THE_LOCK_ON_P);
		    act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING, MOVE_N_PICKS_THE_LOCK_ON_P);
		    check_improve(ch,gsn_pick_lock,TRUE,2);
		    return;
		}

		
		/* 'pick object' */
		if (obj->item_type != ITEM_CONTAINER)
		    { send_to_char(msg(MOVE_THATS_NOT_A_CONTAINER, ch), ch); return; }
		if (!IS_SET(obj->value[1], CONT_CLOSED))
		    { send_to_char(msg(MOVE_ITS_NOT_CLOSED, ch), ch); return; }
		if (obj->value[2] < 0)
		    { send_to_char(msg(MOVE_IT_CANT_BE_UNLOCKED, ch), ch); return; }
		if (!IS_SET(obj->value[1], CONT_LOCKED))
		    { send_to_char(msg(MOVE_ITS_ALREADY_UNLOCKED, ch), ch); return; }
		if (IS_SET(obj->value[1], CONT_PICKPROOF))
		    { send_to_char(msg(MOVE_YOU_FAILED, ch), ch); return; }

		REMOVE_BIT(obj->value[1], CONT_LOCKED);
		  act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD, MOVE_YOU_PICK_THE_LOCK_ON_P);
		  act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING, MOVE_N_PICKS_THE_LOCK_ON_P);
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
		    { send_to_char(msg(MOVE_ITS_NOT_CLOSED, ch), ch); return; }
		if (pexit->key < 0 && !IS_IMMORTAL(ch))
		    { send_to_char(msg(MOVE_IT_CANT_BE_PICKED, ch), ch); return; }
		if (!IS_SET(pexit->exit_info, EX_LOCKED))
		    { send_to_char(msg(MOVE_ITS_ALREADY_UNLOCKED, ch), ch); return; }
		if (IS_SET(pexit->exit_info, EX_PICKPROOF) && !IS_IMMORTAL(ch))
		    { send_to_char(msg(MOVE_YOU_FAILED, ch), ch); return; }

		REMOVE_BIT(pexit->exit_info, EX_LOCKED);
		send_to_char(msg(MOVE_CLICK, ch), ch);
		act_nprintf(ch, NULL, pexit->keyword, TO_ROOM, POS_RESTING, MOVE_N_PICKS_THE_D);
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




void do_stand(CHAR_DATA *ch, char *argument)
{
	OBJ_DATA *obj = NULL;

	if (argument[0] != '\0')
	{
		if (ch->position == POS_FIGHTING)
		{
		    send_to_char(msg(MOVE_MAYBE_YOU_SHOULD_FINISH_FIGHTING_FIRST, ch),ch);
		    return;
		}
		obj = get_obj_list(ch,argument,ch->in_room->contents);
		if (obj == NULL)
		{
		    send_to_char(msg(MOVE_YOU_DONT_SEE_THAT, ch),ch);
		    return;
		}
		if (obj->item_type != ITEM_FURNITURE
		||  (!IS_SET(obj->value[2],STAND_AT)
		&&   !IS_SET(obj->value[2],STAND_ON)
		&&   !IS_SET(obj->value[2],STAND_IN)))
		{
		    send_to_char(msg(MOVE_YOU_CANT_FIND_PLACE_STAND, ch),ch);
		    return;
		}
		if (ch->on != obj && count_users(obj) >= obj->value[0])
		{
		    act_nprintf(ch, obj, NULL, TO_ROOM, POS_DEAD, MOVE_THERES_NO_ROOM_TO_STAND_ON);
		    return;
		}
	}
	switch (ch->position)
	{
	case POS_SLEEPING:
		if (IS_AFFECTED(ch, AFF_SLEEP))
		    { send_to_char(msg(MOVE_YOU_CANT_WAKE_UP, ch), ch); return; }
		
		if (obj == NULL)
		{
		    send_to_char(msg(MOVE_YOU_WAKE_AND_STAND_UP, ch), ch);
		    act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING, 
				MOVE_N_WAKES_AND_STANDS_UP);
		    ch->on = NULL;
		}
		else if (IS_SET(obj->value[2],STAND_AT))
		{
		   act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD, 
				MOVE_YOU_WAKE_AND_STAND_AT);
		   act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING, 
				MOVE_N_WAKES_AND_STANDS_AT);
		}
		else if (IS_SET(obj->value[2],STAND_ON))
		{
		   act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD, 
				MOVE_YOU_WAKE_AND_STAND_ON);
		   act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING, 
				MOVE_N_WAKES_AND_STANDS_ON);
		}
		else 
		{
		   act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD, 
				MOVE_YOU_WAKE_AND_STAND_IN);
		   act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING, 
				MOVE_N_WAKES_AND_STANDS_IN);
		}

		  if (IS_HARA_KIRI(ch)) 
		{
		 send_to_char(msg(MOVE_FEEL_BLOOD_HEATS, ch), ch);
		 REMOVE_BIT(ch->act,PLR_HARA_KIRI);
		}

		ch->position = POS_STANDING;
		do_look(ch,"auto");
		break;

	case POS_RESTING: case POS_SITTING:
		if (obj == NULL)
		{
		    send_to_char(msg(MOVE_YOU_STAND_UP, ch), ch);
		    act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING, MOVE_N_STANDS_UP);
		    ch->on = NULL;
		}
		else if (IS_SET(obj->value[2],STAND_AT))
		{
		    act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD, MOVE_YOU_STAND_AT);
		    act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING, MOVE_N_STANDS_AT);
		}
		else if (IS_SET(obj->value[2],STAND_ON))
		{
		    act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD, MOVE_YOU_STAND_ON);
		    act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING, MOVE_N_STANDS_ON);
		}
		else
		{
		    act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD, MOVE_YOU_STAND_IN);
		    act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING, MOVE_N_STANDS_IN);
		}
		ch->position = POS_STANDING;
		break;

	case POS_STANDING:
		send_to_char(msg(MOVE_YOU_ARE_ALREADY_STANDING, ch), ch);
		break;

	case POS_FIGHTING:
		send_to_char(msg(MOVE_YOU_ARE_ALREADY_FIGHTING, ch), ch);
		break;
	}

	return;
}



void do_rest(CHAR_DATA *ch, char *argument)
{
	OBJ_DATA *obj = NULL;

	if (ch->position == POS_FIGHTING)
	{
		send_to_char(msg(MOVE_YOU_ARE_ALREADY_FIGHTING, ch), ch);
		return;
	}

	if (MOUNTED(ch)) 
	{
		  send_to_char(msg(MOVE_YOU_CANT_REST_MOUNTED, ch), ch);
		  return;
	}
	if (RIDDEN(ch)) 
	{
		  send_to_char(msg(MOVE_YOU_CANT_REST_RIDDEN, ch), ch);
		  return;
	}


	if (IS_AFFECTED(ch, AFF_SLEEP))
	{ send_to_char(msg(MOVE_YOU_ARE_ALREADY_SLEEPING, ch), ch); return; }

	/* okay, now that we know we can rest, find an object to rest on */
	if (argument[0] != '\0')
	{
		obj = get_obj_list(ch,argument,ch->in_room->contents);
		if (obj == NULL)
		{
		    send_to_char(msg(MOVE_YOU_DONT_SEE_THAT, ch), ch);
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
		    send_to_char(msg(MOVE_YOU_CANT_REST_ON_THAT, ch), ch);
		    return;
		}

		  if (obj != NULL && ch->on != obj && count_users(obj) >= obj->value[0])
		  {
		    act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD, 
				MOVE_THERES_NO_MORE_ROOM_ON);
		    return;
		}
		
		ch->on = obj;
	}

	switch (ch->position)
	{
	case POS_SLEEPING:
		if (obj == NULL)
		{
		    send_to_char(msg(MOVE_YOU_WAKE_AND_REST, ch), ch);
		    act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING, 
				MOVE_N_WAKES_AND_RESTS);
		}
		else if (IS_SET(obj->value[2],REST_AT))
		{
		    act_nprintf(ch, obj, NULL, TO_CHAR, POS_SLEEPING,
				MOVE_YOU_WAKE_UP_AND_REST_AT);
		    act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING,
				MOVE_N_WAKES_UP_AND_RESTS_AT);
		}
		  else if (IS_SET(obj->value[2],REST_ON))
		  {
		    act_nprintf(ch, obj, NULL, TO_CHAR, POS_SLEEPING,
				MOVE_YOU_WAKE_UP_AND_REST_ON);
		    act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING,
				MOVE_N_WAKES_UP_AND_RESTS_ON);
		  }
		  else
		  {
		    act_nprintf(ch, obj, NULL, TO_CHAR, POS_SLEEPING,
				MOVE_YOU_WAKE_UP_AND_REST_IN);
		    act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING,
				MOVE_N_WAKES_UP_AND_RESTS_IN);
		  }
		ch->position = POS_RESTING;
		break;

	case POS_RESTING:
		send_to_char(msg(MOVE_YOU_ARE_ALREADY_RESTING, ch), ch);
		break;

	case POS_STANDING:
		if (obj == NULL)
		{
		    send_to_char(msg(MOVE_YOU_REST, ch), ch);
		    act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING,
				MOVE_N_SITS_DOWN_AND_RESTS);
		}
		  else if (IS_SET(obj->value[2],REST_AT))
		  {
		    act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD,
				MOVE_YOU_SIT_DOWN_AT_AND_REST);
		    act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING,
				MOVE_N_SITS_DOWN_AT_AND_RESTS);
		  }
		  else if (IS_SET(obj->value[2],REST_ON))
		  {
		    act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD,
				MOVE_YOU_SIT_DOWN_ON_AND_REST);
		    act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING,
				MOVE_N_SITS_DOWN_ON_AND_RESTS);
		  }
		  else
		  {
		    act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD,
				MOVE_YOU_SIT_DOWN_IN_AND_REST);
		    act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING,
				MOVE_N_SITS_DOWN_IN_AND_RESTS);
		  }
		ch->position = POS_RESTING;
		break;

	case POS_SITTING:
		if (obj == NULL)
		{
		    send_to_char(msg(MOVE_YOU_REST, ch), ch);
		    act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING, MOVE_N_RESTS);
		}
		  else if (IS_SET(obj->value[2],REST_AT))
		  {
		    act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD, MOVE_YOU_REST_AT);
		    act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING, MOVE_N_RESTS_AT);
		  }
		  else if (IS_SET(obj->value[2],REST_ON))
		  {
		    act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD, MOVE_YOU_REST_ON);
		    act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING, MOVE_N_RESTS_ON);
		  }
		  else
		  {
		    act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD, MOVE_YOU_REST_IN);
		    act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING, MOVE_N_RESTS_IN);
		}
		ch->position = POS_RESTING;

		  if (IS_HARA_KIRI(ch)) 
		{
		 send_to_char(msg(MOVE_FEEL_BLOOD_HEATS, ch), ch);
		 REMOVE_BIT(ch->act, PLR_HARA_KIRI);
		}
		break;
	}

	return;
}


void do_sit (CHAR_DATA *ch, char *argument)
{
	OBJ_DATA *obj = NULL;

	if (ch->position == POS_FIGHTING)
	{
		send_to_char(msg(MOVE_MAYBE_YOU_SHOULD_FINISH_FIGHTING_FIRST, ch), ch);
		return;
	}
	if (MOUNTED(ch)) 
	{
		  send_to_char(msg(MOVE_YOU_CANT_SIT_MOUNTED, ch), ch);
		  return;
	}
	if (RIDDEN(ch)) 
	{
		  send_to_char(msg(MOVE_YOU_CANT_SIT_RIDDEN, ch), ch);
		  return;
	}

	if (IS_AFFECTED(ch, AFF_SLEEP))
	{ send_to_char(msg(MOVE_YOU_ARE_ALREADY_SLEEPING, ch), ch); return; }

	/* okay, now that we know we can sit, find an object to sit on */
	if (argument[0] != '\0')
	{
		obj = get_obj_list(ch,argument,ch->in_room->contents);
		if (obj == NULL)
		{
	if (IS_AFFECTED(ch, AFF_SLEEP))
	{ send_to_char(msg(MOVE_YOU_ARE_ALREADY_SLEEPING, ch), ch); return; }
		    send_to_char(msg(MOVE_YOU_DONT_SEE_THAT, ch), ch);
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
		    send_to_char(msg(MOVE_YOU_CANT_SIT_ON_THAT, ch), ch);
		    return;
		}

		if (obj != NULL && ch->on != obj && count_users(obj) >= obj->value[0])
		{
		    act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD,
				MOVE_THERES_NO_MORE_ROOM_ON);
		    return;
		}

		ch->on = obj;
	}
	switch (ch->position)
	{
		case POS_SLEEPING:
		      if (obj == NULL)
		      {
		      	send_to_char(msg(MOVE_YOU_WAKE_AND_SIT_UP,  ch), ch);
		      	act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING,
					MOVE_N_WAKES_AND_SITS_UP);
		      }
		      else if (IS_SET(obj->value[2],SIT_AT))
		      {
		      	act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD,
					MOVE_YOU_WAKE_AND_SIT_AT);
		      	act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING,
					MOVE_N_WAKES_AND_SITS_AT);
		      }
		      else if (IS_SET(obj->value[2],SIT_ON))
		      {
		      	act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD,
					MOVE_YOU_WAKE_AND_SIT_ON);
		      	act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING,
					MOVE_N_WAKES_AND_SITS_ON);
		      }
		      else
		      {
		      	act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD,
					MOVE_YOU_WAKE_AND_SIT_IN);
		      	act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING,
					MOVE_N_WAKES_AND_SITS_IN);
		      }

		    ch->position = POS_SITTING;
		    break;
		case POS_RESTING:
		    if (obj == NULL)
			send_to_char(msg(MOVE_YOU_STOP_RESTING, ch), ch);
		    else if (IS_SET(obj->value[2],SIT_AT))
		    {
			act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD,
					MOVE_YOU_SIT_AT);
			act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING,
					MOVE_N_SITS_AT);
		    }

		    else if (IS_SET(obj->value[2],SIT_ON))
		    {
			act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD,
					MOVE_YOU_SIT_ON);
			act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING,
					MOVE_N_SITS_ON);
		    }
		    ch->position = POS_SITTING;
		    break;
		case POS_SITTING:
		    send_to_char(msg(MOVE_YOU_ARE_ALREADY_SITTING_DOWN, ch), ch);
		    break;
		case POS_STANDING:
		    if (obj == NULL)
		    {
			send_to_char(msg(MOVE_YOU_SIT_DOWN,  ch), ch);
		        act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING,
					MOVE_N_SITS_DOWN_ON_THE_GROUND);
		    }
		    else if (IS_SET(obj->value[2],SIT_AT))
		    {
			act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD,
					MOVE_YOU_SIT_DOWN_AT);
			act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING,
					MOVE_N_SITS_DOWN_AT);
		    }
		    else if (IS_SET(obj->value[2],SIT_ON))
		    {
			act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD,
					MOVE_YOU_SIT_ON);
			act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING,
					MOVE_N_SITS_ON);
		    }
		    else
		    {
			act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD,
					MOVE_YOU_SIT_DOWN_IN);
			act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING,
					MOVE_N_SITS_DOWN_IN);
		    }
		    ch->position = POS_SITTING;
		    break;
	}
	if (IS_HARA_KIRI(ch)) 
		{
		 send_to_char(msg(MOVE_FEEL_BLOOD_HEATS, ch), ch);
		 REMOVE_BIT(ch->act,PLR_HARA_KIRI);
		}
	return;
}


void do_sleep(CHAR_DATA *ch, char *argument)
{
	OBJ_DATA *obj = NULL;

	if (MOUNTED(ch)) 
	{
		  send_to_char(msg(MOVE_YOU_CANT_SLEEP_MOUNTED, ch), ch);
		  return;
	}
	if (RIDDEN(ch)) 
	{
		  send_to_char(msg(MOVE_YOU_CANT_SLEEP_RIDDEN, ch), ch);
		  return;
	}

	switch (ch->position)
	{
	case POS_SLEEPING:
		send_to_char(msg(MOVE_YOU_ARE_ALREADY_SLEEPING, ch), ch);
		break;

	case POS_RESTING:
	case POS_SITTING:
	case POS_STANDING: 
		if (argument[0] == '\0' && ch->on == NULL)
		{
		    send_to_char(msg(MOVE_YOU_GO_TO_SLEEP, ch), ch);
		    act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING,
					MOVE_N_GOES_TO_SLEEP);
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
			send_to_char(msg(MOVE_YOU_DONT_SEE_THAT, ch), ch);
			return;
		    }
		    if (obj->item_type != ITEM_FURNITURE
		    ||  (!IS_SET(obj->value[2],SLEEP_ON) 
		    &&   !IS_SET(obj->value[2],SLEEP_IN)
		    &&	 !IS_SET(obj->value[2],SLEEP_AT)))
		    {
			send_to_char(msg(MOVE_YOU_CANT_SLEEP_ON_THAT, ch), ch);
			return;
		    }

		    if (ch->on != obj && count_users(obj) >= obj->value[0])
		    {
			act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD,
					MOVE_THERES_NO_ROOM_ON_P_FOR_YOU);
			return;
		    }

		    ch->on = obj;
		    if (IS_SET(obj->value[2],SLEEP_AT))
		    {
			act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD,
					MOVE_YOU_GO_TO_SLEEP_AT);
			act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING,
					MOVE_N_GOES_TO_SLEEP_AT);
		    }
		    else if (IS_SET(obj->value[2],SLEEP_ON))
		    {
			act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD,
					MOVE_YOU_GO_TO_SLEEP_ON);
			act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING,
					MOVE_N_GOES_TO_SLEEP_ON);
		    }
		    else
		    {
			act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD,
					MOVE_YOU_GO_TO_SLEEP_IN);
			act_nprintf(ch, obj, NULL, TO_ROOM, POS_RESTING,
					MOVE_N_GOES_TO_SLEEP_IN);
		    }
		    ch->position = POS_SLEEPING;
		}
		break;

	case POS_FIGHTING:
		send_to_char(msg(MOVE_YOU_ARE_ALREADY_FIGHTING, ch), ch);
		break;
	}

	return;
}


void do_wake(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	one_argument(argument, arg);
	if (arg[0] == '\0')
		{ do_stand(ch, argument); return; }

	if (!IS_AWAKE(ch))
		{ send_to_char(msg(MOVE_YOU_ARE_ASLEEP_YOURSELF, ch), ch); return; }

	if ((victim = get_char_room(ch, arg)) == NULL)
		{ send_to_char(msg(MOVE_THEY_ARENT_HERE,  ch), ch); return; }

	if (IS_AWAKE(victim)) { 
		act_nprintf(ch, NULL, victim, TO_CHAR, POS_DEAD,
				MOVE_N_IS_ALREADY_AWAKE); return; 
	}

	if (IS_AFFECTED(victim, AFF_SLEEP)) { 
		act_nprintf(ch, NULL, victim, TO_CHAR, POS_DEAD, MOVE_YOU_CANT_WAKE_M);  
		return; 
	}

	act_nprintf(ch, NULL, victim, TO_VICT, POS_SLEEPING, MOVE_N_WAKES_YOU);
	do_stand(victim,"");
	return;
}



void do_sneak(CHAR_DATA *ch, char *argument)
{
	AFFECT_DATA af;

	if (MOUNTED(ch)) 
	{
		  send_to_char(msg(MOVE_YOU_CANT_SNEAK_MOUNTED, ch), ch);
		  return;
	}

	send_to_char(msg(MOVE_YOU_ATTEMPT_TO_MOVE_SILENTLY, ch), ch);
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



void do_hide(CHAR_DATA *ch, char *argument)
{
	int forest;

	if (MOUNTED(ch)) 
	{
		  send_to_char(msg(MOVE_YOU_CANT_HIDE_MOUNTED, ch), ch);
		  return;
	}

	if (RIDDEN(ch)) 
	{
		  send_to_char(msg(MOVE_YOU_CANT_HIDE_RIDDEN, ch), ch);
		  return;
	}

	if (IS_AFFECTED(ch, AFF_FAERIE_FIRE) )  {
		send_to_char(msg(MOVE_YOU_CANNOT_HIDE_GLOWING, ch), ch);
		return;
	}
	forest = 0;
	forest += ch->in_room->sector_type == SECT_FOREST ? 60 : 0;
	forest += ch->in_room->sector_type == SECT_FIELD ? 60 : 0;

	send_to_char(msg(MOVE_YOU_ATTEMPT_TO_HIDE, ch), ch);

	if (number_percent() < get_skill(ch,gsn_hide)-forest) {
		SET_BIT(ch->affected_by, AFF_HIDE);
		check_improve(ch,gsn_hide,TRUE,3);
	} else  {
		REMOVE_BIT(ch->affected_by, AFF_HIDE);
		check_improve(ch,gsn_hide,FALSE,3);
	}

	return;
}

void do_camouflage(CHAR_DATA *ch, char *argument)
{
	int chance;

	if (MOUNTED(ch)) 
	{
		  send_to_char(msg(MOVE_YOU_CANT_CAMOUFLAGE_MOUNTED, ch), ch);
		  return;
	}
	if (RIDDEN(ch)) 
	{
		  send_to_char(msg(MOVE_YOU_CANT_CAMOUFLAGE_RIDDEN, ch), ch);
		  return;
	}

	if (IS_NPC(ch)
	||  (chance = get_skill(ch, gsn_camouflage)) == 0) {
		send_to_char(msg(MOVE_YOU_DONT_KNOW_CAMOUFLAGE, ch), ch);
		return;
	}

	if (IS_AFFECTED(ch, AFF_FAERIE_FIRE))  {
		send_to_char(msg(MOVE_YOU_CANT_CAMOUFLAGE_GLOWING, ch), ch);
		return;
	}

	if (ch->in_room->sector_type != SECT_FOREST &&
		ch->in_room->sector_type != SECT_HILLS  &&
		ch->in_room->sector_type != SECT_MOUNTAIN)
		{
		send_to_char(msg(MOVE_THERES_NO_COVER_HERE, ch), ch);
		act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING,
				MOVE_N_TRIES_TO_CAMOUFLAGE);
		return;
		}
	send_to_char(msg(MOVE_YOU_ATTEMPT_TO_CAMOUFLAGE, ch), ch);
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
void do_visible(CHAR_DATA *ch, char *argument)
{
	if (IS_SET(ch->affected_by, AFF_HIDE)) {
		send_to_char(msg(MOVE_YOU_STEP_OUT_SHADOWS, ch), ch);
		REMOVE_BIT(ch->affected_by, AFF_HIDE);
		act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING,
				MOVE_N_STEPS_OUT_OF_SHADOWS);
	}
	if (IS_SET(ch->affected_by, AFF_FADE)) {
		send_to_char(msg(MOVE_YOU_STEP_OUT_SHADOWS, ch), ch);
		REMOVE_BIT(ch->affected_by, AFF_FADE);
		act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING,
				MOVE_N_STEPS_OUT_OF_SHADOWS);
	}
	if (IS_SET(ch->affected_by, AFF_CAMOUFLAGE)) {
		send_to_char(msg(MOVE_YOU_STEP_OUT_COVER, ch), ch);
		REMOVE_BIT(ch->affected_by,AFF_CAMOUFLAGE);
		act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING,
				MOVE_N_STEPS_OUT_COVER);
	}
	if (IS_SET(ch->affected_by, AFF_INVISIBLE)) {
		send_to_char(msg(MOVE_YOU_FADE_INTO_EXIST, ch), ch);
		affect_strip(ch, gsn_invis);
		affect_strip(ch, gsn_mass_invis);
		REMOVE_BIT(ch->affected_by, AFF_INVISIBLE);
		act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING,
				MOVE_N_FADES_INTO_EXIST);
	}
	if (IS_SET(ch->affected_by, AFF_IMP_INVIS)) {
		send_to_char(msg(MOVE_YOU_FADE_INTO_EXIST, ch), ch);
		affect_strip(ch, gsn_invis);
		affect_strip(ch, gsn_imp_invis);
		REMOVE_BIT(ch->affected_by, AFF_IMP_INVIS);
		act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING,
				MOVE_N_FADES_INTO_EXIST);
	}
	if (IS_SET(ch->affected_by, AFF_SNEAK) 
	    && !IS_NPC(ch) && !IS_SET(race_table[RACE(ch)].aff,AFF_SNEAK)) {
		send_to_char(msg(MOVE_YOU_TRAMPLE_AROUND_LOUDLY, ch), ch);
		affect_strip(ch, gsn_sneak);
		REMOVE_BIT(ch->affected_by, AFF_SNEAK);
	}

	affect_strip (ch, gsn_mass_invis);
	return;
}

void do_recall(CHAR_DATA *ch, char *argument)
{
	ROOM_INDEX_DATA *location;
	int point;
#ifdef 0
	CHAR_DATA *victim;
	int lose,skill;
#endif
 
	if (IS_GOOD(ch))      
		point = hometown_table[ch->hometown].recall[0];
	else if (IS_EVIL(ch))
		point = hometown_table[ch->hometown].recall[2];
	else
		point = hometown_table[ch->hometown].recall[1];

	if (IS_NPC(ch) && !IS_SET(ch->act,ACT_PET))
	{
		send_to_char(msg(MOVE_ONLY_PLAYERS_RECALL,  ch), ch);
		return;
	}

	if (ch->level >= 11 && !IS_IMMORTAL(ch))
		{
		send_to_char(msg(MOVE_RECALL_FOR_BELOW_10, ch), ch);
		return;
		}

	if (ch->desc != NULL && current_time - ch->last_fight_time
		< FIGHT_DELAY_TIME)
		{
		send_to_char(msg(MOVE_TOO_PUMPED_TO_PRAY, ch), ch);
		return;
		}

	if (ch->desc == NULL && !IS_NPC(ch))
		{
		point =	hometown_table[number_range(0, 4)].recall[number_range(0,2)];
		}

	act_nprintf(ch, 0, 0, TO_ROOM, POS_RESTING,
			MOVE_N_PRAYS_FOR_TRANSPORTATION);
	
	if ((location = get_room_index(point))== NULL)
	{
		send_to_char(msg(MOVE_YOU_ARE_COMPLETELY_LOST, ch), ch);
		return;
	}

	if (ch->in_room == location)
		return;

	if (IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
	||   IS_AFFECTED(ch, AFF_CURSE) 
	||   IS_RAFFECTED(ch->in_room, AFF_ROOM_CURSE))
	{
		send_to_char(msg(MOVE_GODS_FORSAKEN_YOU, ch), ch);
		return;
	}

#ifdef 0
	if ((victim = ch->fighting) != NULL)
	{
		send_to_char(msg(MOVE_YOU_ARE_STILL_FIGHTING, ch), ch);

		if (IS_NPC(ch))
		    skill = 40 + ch->level;
		else
		    skill = get_skill(ch,gsn_recall);

		if (number_percent() < 80 * skill / 100)
		{
		    check_improve(ch,gsn_recall,FALSE,6);
		    WAIT_STATE(ch, 4);
		    send_to_char(msg(MOVE_YOU_FAILED, ch), ch);
		    return;
		}

		lose = 25;
		gain_exp(ch, 0 - lose);
		check_improve(ch,gsn_recall,TRUE,4);
		char_printf(ch, msg(MOVE_RECALL_FROM_COMBAT, ch), lose);
		stop_fighting(ch, TRUE); 
	}
#endif

	ch->move /= 2;
	act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING,
				MOVE_N_DISAPPEARS);
	char_from_room(ch);
	char_to_room(ch, location);
	act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING,
				MOVE_N_APPEARS_IN_THE_ROOM);
	do_look(ch, "auto");
	
	if (ch->pet != NULL) {
 		char_from_room(ch->pet);
		char_to_room(ch->pet, location);
		do_look(ch->pet, "auto");
	}

	return;
}


void do_train(CHAR_DATA *ch, char *argument)
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
	for (mob = ch->in_room->people; mob; mob = mob->next_in_room)
	{
		if (IS_NPC(mob) && (IS_SET(mob->act, ACT_PRACTICE) 
		|| IS_SET(mob->act,ACT_TRAIN) || IS_SET(mob->act,ACT_GAIN)))
		    break;
	}

	if (mob == NULL)
	{
		send_to_char(msg(MOVE_YOU_CANT_DO_THAT_HERE, ch), ch);
		return;
	}

	if (argument[0] == '\0')
	{
		char_printf(ch, msg(MOVE_YOU_HAVE_D_TRAINING_SESSIONS, ch), ch->train);
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
/*
		char_printf(ch, "You can't train charisma. That is about your behaviour.\n\r");
		return;
*/
	}

	else if (!str_cmp(argument, "hp"))
		cost = 1;

	else if (!str_cmp(argument, "mana"))
		cost = 1;

	else
	{
		strcpy(buf, msg(MOVE_YOU_CAN_TRAIN, ch));
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
		 	    send_to_char(msg(MOVE_NOT_ENOUGH_TRAININGS, ch), ch);
		      return;
		  }
 
		ch->train -= cost;
		  ch->pcdata->perm_hit += 10;
		  ch->max_hit += 10;
		  ch->hit +=10;
		  act_nprintf(ch, NULL, NULL, TO_CHAR, POS_DEAD,
				MOVE_YOUR_DURABILITY_INCREASES);
		  act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING,
				MOVE_N_DURABILITY_INCREASES);
		  return;
	}
 
	if (!str_cmp("mana",argument))
	{
		  if (cost > ch->train)
		  {
		 	    send_to_char(msg(MOVE_NOT_ENOUGH_TRAININGS, ch), ch);
		      return;
		  }

		ch->train -= cost;
		  ch->pcdata->perm_mana += 10;
		  ch->max_mana += 10;
		  ch->mana += 10;
		  act_nprintf(ch, NULL, NULL, TO_CHAR, POS_DEAD,
				MOVE_YOUR_POWER_INCREASES);
		  act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING,
				MOVE_N_POWER_INCREASES);
		  return;
	}

	if (ch->perm_stat[stat]  >= get_max_train2(ch,stat))
	{
		act_nprintf(ch, NULL, pOutput, TO_CHAR, POS_DEAD, MOVE_YOUR_T_IS_MAX);
		return;
	}

	if (cost > ch->train)
	{
		  send_to_char(msg(MOVE_NOT_ENOUGH_TRAININGS, ch), ch);
		return;
	}

	ch->train		-= cost;
  
	ch->perm_stat[stat]		+= 1;
	act_nprintf(ch, NULL, pOutput, TO_CHAR, POS_DEAD, MOVE_YOUR_T_INCREASES);
	act_nprintf(ch, NULL, pOutput, TO_ROOM, POS_RESTING, MOVE_N_T_INCREASES);
	return;
}



void do_track(CHAR_DATA *ch, char *argument)
{
  ROOM_HISTORY_DATA *rh;
  EXIT_DATA *pexit;
  static char *door[] = { "north","east","south","west","up","down",
		                      "that way" };
  int d;

  if (!IS_NPC(ch) && get_skill(ch,gsn_track) < 2) {
	send_to_char(msg(MOVE_THERE_ARE_NO_TRAIN_TRACKS_HERE, ch), ch);
	return;
  }

  WAIT_STATE(ch, skill_table[gsn_track].beats);
  act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING, MOVE_N_CHECKS_TRACKS);

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
		    char_printf(ch, msg(MOVE_TRACKS_LEAD_S, ch), 
					capitalize(rh->name), door[d]);
		  if ((pexit = ch->in_room->exit[d]) != NULL
		    &&   IS_SET(pexit->exit_info, EX_ISDOOR)
		    &&   pexit->keyword != NULL)
		     doprintf(do_open, ch,"%s",door[d]);
		    move_char(ch,rh->went,FALSE);
		    return;
		}
	}

		send_to_char(msg(MOVE_DONT_SEE_TRACKS, ch), ch);

		if (IS_NPC(ch))
			ch->status = 5; /* for stalker */
		check_improve(ch, gsn_track, FALSE, 1);
}


void do_vampire(CHAR_DATA *ch, char *argument)
{
	AFFECT_DATA af;
	int level, duration;
	int chance;
 
	if (is_affected(ch, gsn_vampire)) {
		send_to_char(msg(MOVE_YOU_CANT_BE_MORE_VAMPIRE, ch), ch);
		return;
	}

	if ((chance = get_skill(ch, gsn_vampire)) == 0) {
		send_to_char(msg(MOVE_YOU_SHOW_MORE_UGGLY, ch), ch);
		return;
	}

	if (chance < 100) {
		send_to_char(msg(MOVE_GO_AND_ASK_QUESTOR, ch), ch);
		return;
	}

	if (is_affected(ch, gsn_vampire)) {
		send_to_char(msg(MOVE_GO_KILL_PLAYER, ch), ch);
		return;
	}


	if (weather_info.sunlight == SUN_LIGHT 
	||  weather_info.sunlight == SUN_RISE) {
		send_to_char(msg(MOVE_WAIT_NIGHT, ch), ch);
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

	send_to_char(msg(MOVE_FEEL_GREATER, ch), ch);
	act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING, MOVE_CANNOT_RECOGNIZE);
	return;
}

void do_vbite(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int chance;

	one_argument(argument, arg);

	if ((chance = get_skill(ch, gsn_vampiric_bite)) == 0) {
		send_to_char(msg(MOVE_DONT_KNOW_BITE, ch), ch);
		return;
	}

	if (!IS_VAMPIRE(ch)) {
		send_to_char(msg(MOVE_MUST_TRANSFORM_VAMP, ch), ch);
		return;
	}

	if (arg[0] == '\0') {
		send_to_char(msg(MOVE_BITE_WHOM, ch), ch);
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL) {
		send_to_char(msg(MOVE_THEY_ARENT_HERE, ch), ch);
		return;
	}

	if (victim->position != POS_SLEEPING) {
		send_to_char(msg(MOVE_THEY_MUST_BE_SLEEPING, ch), ch);
		return;
	}

	if ((IS_NPC(ch)) && (!(IS_NPC(victim))))
		return;
		

	if (victim == ch) {
		send_to_char(msg(MOVE_HOW_CAN_YOU_SNEAK_YOU, ch), ch);
		return;
	}

	if (is_safe(ch, victim))
		return;

	if (victim->fighting != NULL) {
		send_to_char(msg(MOVE_CANT_BITE_FIGHTING_PERS, ch), ch);
		return;
	}


	WAIT_STATE(ch, skill_table[gsn_vampiric_bite].beats);

	if (victim->hit < (8 * victim->max_hit / 10) && (IS_AWAKE(victim))) {
		act_nprintf(ch, NULL, victim, TO_CHAR, POS_DEAD,
			   MOVE_DOESNT_WORTH_UP);
		return;
	}

	if (current_time-victim->last_fight_time<300 && IS_AWAKE(victim)) {
		act_nprintf(ch, NULL, victim, TO_CHAR, POS_DEAD,
			   MOVE_DOESNT_WORTH_TO_DO);
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
  	    doprintf(do_yell, victim, msg(MOVE_HELP_TRIED_TO_BITE, victim));
	return;
}

void do_bash_door(CHAR_DATA *ch, char *argument)
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
		send_to_char(msg(MOVE_BASH_WHATS_THAT, ch), ch);
		return;
	}
 
	if (MOUNTED(ch)) {
        	send_to_char(msg(MOVE_CANT_BASH_DOORS_MOUNTED, ch), ch);
		return;
	}

	if (RIDDEN(ch)) {
		send_to_char(msg(MOVE_CANT_BASH_DOORS_RIDDEN, ch), ch);
		return;
	}

	if (arg[0] == '\0') {
		send_to_char(msg(MOVE_BASH_WHICH_DOOR, ch), ch);
		return;
	}

	if (ch->fighting) {	
		send_to_char(msg(MOVE_WAIT_FIGHT_FINISH, ch), ch);
		return;
	}

	/* look for guards */
	for (gch = ch->in_room->people; gch; gch = gch->next_in_room)
		if (IS_NPC(gch)
		&&  IS_AWAKE(gch) && ch->level + 5 < gch->level) {
			act_nprintf(ch, NULL, gch, TO_CHAR, POS_DEAD, 
				    MOVE_N_TOO_CLOSE_TO_DOOR);
			return;
		}

	if ((door = find_door(ch, arg)) < 0)
		return;

	pexit = ch->in_room->exit[door];

	if (!IS_SET(pexit->exit_info, EX_CLOSED)) {
		send_to_char(msg(MOVE_ITS_ALREADY_OPEN, ch), ch);
		return;
	}

	if (!IS_SET(pexit->exit_info, EX_LOCKED)) {
		send_to_char(msg(MOVE_TRY_TO_OPEN, ch), ch);
		return;
	}

	if (IS_SET(pexit->exit_info, EX_NOPASS)) {
		send_to_char(msg(MOVE_SHIELD_PROTECTS_EXIT, ch), ch); 
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
		    MOVE_YOU_SLAM_TRY_BREAK);
	act_nprintf(ch, NULL, pexit->keyword, TO_ROOM, POS_RESTING,
		    MOVE_N_SLAMS_TRY_BREAK);


	if (room_dark(ch->in_room))
		chance /= 2;

	/* now the attack */
	if (number_percent() < chance) {
		check_improve(ch,gsn_bash_door,TRUE,1);

		REMOVE_BIT(pexit->exit_info, EX_LOCKED);
		REMOVE_BIT(pexit->exit_info, EX_CLOSED);
		act_nprintf(ch, NULL, pexit->keyword, TO_ROOM, POS_RESTING, 
				MOVE_N_BASHES_AND_BREAK);
		send_to_char(msg(MOVE_YOU_SUCCESSED_TO_OPEN_DOOR, ch), ch);

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
					    MOVE_THE_D_OPENS);
		}

		check_improve(ch, gsn_bash_door, TRUE, 1);
		WAIT_STATE(ch,skill_table[gsn_bash_door].beats);
	}
	else {
		send_to_char(msg(MOVE_YOU_FALL_ON_FACE, ch), ch);
		act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING,
			    MOVE_N_FALLS_ON_FACE);
		check_improve(ch, gsn_bash_door, FALSE, 1);
		ch->position = POS_RESTING;
		WAIT_STATE(ch,skill_table[gsn_bash_door].beats * 3/2); 
		damage_bash = ch->damroll +
			      number_range(4,4 + 4* ch->size + chance/5);
		damage(ch, ch, damage_bash, gsn_bash_door, DAM_BASH, TRUE);
	}

}

void do_blink(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];

	argument = one_argument(argument , arg);

	if (!IS_NPC(ch)
	&&   ch->level < skill_table[gsn_blink].skill_level[ch->class])
	{
		send_to_char("Huh?.\n\r", ch);
		return;
	}

	if (arg[0] == '\0')
	{
	 char_printf(ch, msg(MOVE_CURRENT_BLINK, ch), IS_BLINK_ON(ch) ? 
							"ON" : "OFF");
	 return;
	}

	if (!str_cmp(arg,"ON"))
		{
		 SET_BIT(ch->act,PLR_BLINK_ON);
		 send_to_char(msg(MOVE_BLINK_ON, ch), ch);
		 return;
		}

	if (!str_cmp(arg,"OFF"))
		{
		 REMOVE_BIT(ch->act,PLR_BLINK_ON);
		 send_to_char(msg(MOVE_BLINK_OFF, ch), ch);
		 return;
		}

   char_printf(ch, msg(MOVE_IS_S_A_STATUS, ch), arg);
   return;
}

void do_vanish(CHAR_DATA *ch, char *argument)
{
  ROOM_INDEX_DATA *pRoomIndex;

  if (!IS_NPC(ch)
		&& ch->level < skill_table[gsn_vanish].skill_level[ch->class])
		{
		 send_to_char(msg(MOVE_HUH, ch), ch);
		 return;
		}


  if (ch->mana < 25)
		{
		 send_to_char(msg(MOVE_DONT_HAVE_POWER, ch), ch);
		 return;
		}

  ch->mana -= 25;

  if (number_percent() > get_skill(ch,gsn_vanish))
	{
		send_to_char(msg(MOVE_YOU_FAILED, ch), ch);
		WAIT_STATE(ch, skill_table[gsn_vanish].beats);
		check_improve(ch,gsn_vanish,FALSE,1);
		return;
	}

  WAIT_STATE(ch, skill_table[gsn_vanish].beats);

  if (ch->in_room == NULL
		||   IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL))
	{
		send_to_char(msg(MOVE_YOU_FAILED, ch), ch);
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

  
  act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING, MOVE_N_THROWS_GLOBE);

  check_improve(ch,gsn_vanish,TRUE,1);

  if (!IS_NPC(ch) && ch->fighting != NULL && number_bits(1) == 1) 
  {
	send_to_char(msg(MOVE_YOU_FAILED, ch), ch);
	return;
  }

  act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING, MOVE_N_IS_GONE);

  char_from_room(ch);
  char_to_room(ch, pRoomIndex);
  act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING, MOVE_N_APPEARS_FROM_NOWHERE);
  do_look(ch, "auto");
  stop_fighting(ch,TRUE);
  return;
}

void do_detect_sneak(CHAR_DATA *ch, char *argument) 
{
	AFFECT_DATA af;

	if (is_affected(ch, gsn_detect_sneak))
	{
		send_to_char(msg(MOVE_YOU_CAN_ALREADY_DETECT_SNEAK, ch), ch);
	}
	af.where	 = TO_DETECTS;
	af.type      = gsn_detect_sneak;
	af.level     = ch->level;
	af.duration  = ch->level / 10;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = DETECT_SNEAK;
	affect_to_char(ch, &af);
	send_to_char(msg(MOVE_YOU_CAN_DETECT_THE_SNEAK, ch), ch);
	return;
}


void do_fade(CHAR_DATA *ch, char *argument)
{
	if (ch_skill_nok(ch,gsn_fade)) return;

	if (MOUNTED(ch)) 
	{
		  send_to_char(msg(MOVE_CANT_FADE_MOUNTED, ch), ch);
		  return;
	}
	if (RIDDEN(ch)) 
	{
		  send_to_char(msg(MOVE_CANT_FADE_RIDDEN, ch), ch);
		  return;
	}

	if (!clan_ok(ch,gsn_fade)) return;
	send_to_char(msg(MOVE_YOU_ATTEMPT_TO_FADE, ch), ch);

	SET_BIT(ch->affected_by, AFF_FADE);
	check_improve(ch,gsn_fade,TRUE,3);

	return;
}

void do_vtouch(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;
	AFFECT_DATA af;

	if (IS_NPC(ch) ||
	    ch->level < skill_table[gsn_vampiric_touch].skill_level[ch->class])
	{
		send_to_char(msg(MOVE_LACK_SKILL_DRAIN_TOUCH, ch), ch);
		return;
	}

	if (!IS_VAMPIRE(ch)) {
		send_to_char(msg(MOVE_LET_IT_BE, ch), ch);
		return;
	}

	if (IS_AFFECTED(ch, AFF_CHARM))  {
		send_to_char(msg(MOVE_DONT_WANT_DRAIN_MASTER, ch), ch);
		return;
	} 

	if ((victim = get_char_room(ch,argument)) == NULL) {
		send_to_char(msg(MOVE_THEY_ARENT_HERE, ch), ch);
		return;
	}

	if (ch==victim) {
		send_to_char(msg(MOVE_EVEN_YOU_NOT_SO_STUPID, ch), ch);
		return;
	}

	if (is_affected(victim,gsn_vampiric_touch))
		return;

	if (is_safe(ch,victim))
		return;

	victim->last_fight_time = current_time;
	ch->last_fight_time = current_time;

	WAIT_STATE(ch,skill_table[gsn_vampiric_touch].beats);

	if (IS_NPC(ch) || 
	    number_percent() < 0.85 * get_skill(ch,gsn_vampiric_touch)) {
		act_nprintf(ch, NULL, victim, TO_CHAR, POS_DEAD, 
				MOVE_YOU_TOUCH_NS_NECK);
		act_nprintf(ch, NULL, victim, TO_VICT, POS_RESTING, 
				MOVE_N_TOUCHES_YOUR_NECK);
		act_nprintf(ch, NULL, victim, TO_NOTVICT, POS_RESTING,
				MOVE_N_TOUCHES_NS_NECK);
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

void do_fly(CHAR_DATA *ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];

	if (IS_NPC(ch)) return;

	argument = one_argument(argument,arg);
	if (!str_cmp(arg,"up"))
	{
	 if (IS_AFFECTED(ch,AFF_FLYING))
		{		       
		   send_to_char(msg(MOVE_YOU_ARE_ALREADY_FLYING, ch), ch); 
		 return;
		}
	 if (is_affected(ch,gsn_fly) 
		 || (race_table[RACE(ch)].aff & AFF_FLYING) 
		 || affect_check_obj(ch,AFF_FLYING))
		{
   	 SET_BIT(ch->affected_by,AFF_FLYING);
		 REMOVE_BIT(ch->act,PLR_CHANGED_AFF);
   	 send_to_char(msg(MOVE_YOU_START_TO_FLY, ch), ch);
		}
   else 
		{		       
		   send_to_char(msg(MOVE_FIND_POTION_OR_WINGS, ch), ch); 
		}
	}
	else if (!str_cmp(arg,"down"))
	{
   if (IS_AFFECTED(ch,AFF_FLYING))
		{
   	 REMOVE_BIT(ch->affected_by,AFF_FLYING);
		 SET_BIT(ch->act,PLR_CHANGED_AFF);
   	 send_to_char(msg(MOVE_YOU_SLOWLY_TOUCH_GROUND, ch), ch);
		}
   else 
		{		       
		   send_to_char(msg(MOVE_YOU_ARE_ALREADY_ON_GROUND, ch), ch); 
		 return;
		}
	}
   else 
	{
		send_to_char(msg(MOVE_TYPE_WITH_UP_OR_DOWN, ch), ch);
		return;
	}

	WAIT_STATE(ch, skill_table[gsn_fly].beats);   

   return;
}
		 

void do_push(CHAR_DATA *ch, char *argument)
{
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int percent;
	int door;

	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (arg1[0] == '\0' || arg2[0] == '\0') {
		send_to_char(msg(MOVE_PUSH_WHOM_WHERE, ch), ch);
		return;
	}

	if (MOUNTED(ch)) {
		  send_to_char(msg(MOVE_CANT_PUSH_MOUNTED, ch), ch);
		  return;
	}
	if (RIDDEN(ch)) {
		  send_to_char(msg(MOVE_CANT_PUSH_RIDDEN, ch), ch);
		  return;
	}

	if (IS_NPC(ch) && IS_SET(ch->affected_by, AFF_CHARM) 
		&& (ch->master != NULL)) {
		send_to_char(msg(MOVE_TOO_DAZED_TO_PUSH, ch), ch);
		return;
	}

	if ((victim = get_char_room(ch, arg1)) == NULL)
	{
		send_to_char(msg(MOVE_THEY_ARENT_HERE, ch), ch);
		return;
	}

	if (!IS_NPC(victim) && victim->desc == NULL)
	{
		send_to_char(msg(MOVE_YOU_CANT_DO_THAT, ch), ch);
		return;
	}

	if (victim == ch)
	{
		send_to_char(msg(MOVE_THATS_POINTLESS, ch), ch);
		return;
	}

	if (is_safe(ch,victim))
		return;

	if (victim->position == POS_FIGHTING)
	{
		send_to_char(msg(MOVE_WAIT_FIGHT_FINISH, ch), ch);
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
		    send_to_char(msg(MOVE_DIR_IS_CLOSED, ch), ch); 
		    else if (IS_SET(pexit->exit_info, EX_LOCKED))
		     send_to_char(msg(MOVE_DIR_IS_LOCKED, ch), ch); 
	 	  return;
		}
	}

	if (CAN_DETECT(ch,ADET_WEB))
	{
		send_to_char(msg(MOVE_YOU_WEBBED_WANT_WHAT, ch), ch);
		act_nprintf(ch, NULL, victim, TO_ROOM, POS_RESTING, 
				MOVE_N_TRIES_PUSH_WEBBED);
		return; 
	}

	if (CAN_DETECT(victim,ADET_WEB))
	{
		act_nprintf(ch, NULL, victim, TO_CHAR, POS_DEAD, 
				MOVE_PUSH_VICT_WEBBED);
		act_nprintf(ch, NULL, victim, TO_ROOM, POS_RESTING,
				MOVE_N_PUSHES_VICT_WEBBED);
		return; 
	}

	ch->last_death_time = -1;

	WAIT_STATE(ch, skill_table[gsn_push].beats);
	percent  = number_percent() + (IS_AWAKE(victim) ? 10 : -50);
	percent += can_see(victim, ch) ? -10 : 0;

	if (/* ch->level + 5 < victim->level || */
		   victim->position == POS_FIGHTING
	|| (!IS_NPC(ch) && percent > get_skill(ch,gsn_push)))
	{
		/*
		 * Failure.
		 */

		send_to_char(msg(MOVE_OOPS, ch), ch);
		if (!IS_AFFECTED(victim, AFF_SLEEP)) {
		   victim->position = victim->position == POS_SLEEPING ? 
					POS_STANDING : victim->position;
		   act_nprintf(ch, NULL, victim, TO_VICT, POS_RESTING,
				MOVE_N_TRIED_PUSH_YOU);
		}
		act_nprintf(ch, NULL, victim, TO_NOTVICT, POS_RESTING, 
				MOVE_N_TRIED_PUSH_N);


		if (IS_AWAKE(victim))
			doprintf(do_yell, victim,
				 msg(MOVE_KEEP_HANDS_OUT, ch), ch->name);
		if (!IS_NPC(ch))
		{
		    if (IS_NPC(victim))
		    {
		        check_improve(ch,gsn_push,FALSE,2);
			multi_hit(victim, ch, TYPE_UNDEFINED);
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


void do_crecall(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;
	ROOM_INDEX_DATA *location;
	int point = ROOM_VNUM_BATTLE;
	AFFECT_DATA af;

	if (ch_skill_nok(ch,gsn_clan_recall))
		return;

	if (!clan_ok(ch,gsn_clan_recall))
		return;

	if (is_affected(ch, gsn_clan_recall)) {
		send_to_char(msg(MOVE_CANT_PRAY_NOW, ch), ch);
	}

	if (ch->desc != NULL && current_time - ch->last_fight_time
		< FIGHT_DELAY_TIME) {
		send_to_char(msg(MOVE_TOO_PUMPED_TO_PRAY, ch), ch);
		return;
	}

	if (ch->desc == NULL && !IS_NPC(ch)) {
		point =	ROOM_VNUM_BATTLE;
	}

	act_nprintf(ch, 0, 0, TO_ROOM, POS_RESTING, MOVE_PRAYS_UPPER_LORD);
	
	if ((location = get_room_index(point))== NULL) {
		send_to_char(msg(MOVE_YOU_ARE_COMPLETELY_LOST, ch), ch);
		return;
	}

	if (ch->in_room == location)
		return;

	if (IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
	||   IS_AFFECTED(ch, AFF_CURSE) 
	||   IS_RAFFECTED(ch->in_room, AFF_ROOM_CURSE)) {
		send_to_char(msg(MOVE_GODS_FORSAKEN_YOU, ch), ch);
		return;
	}

	if ((victim = ch->fighting) != NULL) {
		send_to_char(msg(MOVE_YOU_ARE_STILL_FIGHTING, ch),ch);
		return;
	}

	if (ch->mana < (ch->max_mana * 0.3)) {
		send_to_char(msg(MOVE_DONT_HAVE_POWER, ch), ch);
		return;
	}

	ch->move /= 2;
	ch->mana /= 10;
	act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING, MOVE_N_DISAPPEARS);
	char_from_room(ch);
	char_to_room(ch, location);
	act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING, 
			MOVE_N_APPEARS_IN_THE_ROOM);
	do_look(ch, "auto");
	
	if (ch->pet != NULL) {
	 	char_from_room(ch->pet);
		char_to_room(ch->pet, location);
		do_look(ch->pet, "auto");
	}

	af.type      = gsn_clan_recall;
	af.level     = ch->level;
	af.duration  = ch->level / 6 + 15;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = 0;
	affect_to_char(ch, &af);

	return;
}

void do_escape(CHAR_DATA *ch, char *argument)
{
	ROOM_INDEX_DATA *was_in;
	ROOM_INDEX_DATA *now_in;
	CHAR_DATA *victim;
	char arg[MAX_INPUT_LENGTH];
	int door;


	if ((victim = ch->fighting) == NULL) {
		  if (ch->position == POS_FIGHTING)
		      ch->position = POS_STANDING;
		send_to_char(msg(MOVE_ARENT_FIGHTING, ch), ch);
		return;
	}

	argument = one_argument(argument, arg);

	if (arg[0] == '\0') {
		send_to_char(msg(MOVE_ESCAPE_WHAT_DIR, ch), ch);
		return;
	}

	if (MOUNTED(ch)) {
		  send_to_char(msg(MOVE_CANT_ESCAPE_MOUNTED, ch), ch);
		  return;
	}
	if (RIDDEN(ch)) {
		  send_to_char(msg(MOVE_CANT_ESCAPE_RIDDEN, ch), ch);
		  return;
	}

	if (!IS_NPC(ch) && ch->level <
			skill_table[gsn_escape].skill_level[ch->class]) {
		send_to_char(msg(MOVE_TRY_FLEE, ch), ch);
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
		&&   IS_SET(pexit->u1.to_room->room_flags, ROOM_NO_MOB))) {
			send_to_char(msg(MOVE_SOMETHING_PREVENTS_ESCAPE, ch), 
					ch);
			return;
		}

		if (number_percent() > get_skill(ch,gsn_escape)) {
			send_to_char(msg(MOVE_ESCAPE_FAILED, ch), ch);
			check_improve(ch,gsn_escape,FALSE,1);	
			return;
		}

		check_improve(ch,gsn_escape,TRUE,1);	
		move_char(ch, door, FALSE);
		if ((now_in = ch->in_room) == was_in)
			continue;

		ch->in_room = was_in;
		act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING,
				MOVE_N_ESCAPED);
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
		send_to_char(msg(MOVE_WRONG_DIRECTION, ch),ch);
		break;
	}
	send_to_char(msg(MOVE_COULDNT_ESCAPE, ch), ch);
	return;
}

void do_layhands(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;
	AFFECT_DATA af;

	if (IS_NPC(ch) ||
	    ch->level < skill_table[gsn_lay_hands].skill_level[ch->class]) {
		send_to_char(msg(MOVE_CANT_LAY_HANDS, ch), ch);
		return;
	}

	if ((victim = get_char_room(ch,argument)) == NULL) {
		send_to_char(msg(MOVE_THEY_ARENT_HERE, ch), ch);
		return;
	}

	if (is_affected(ch, gsn_lay_hands)) {
		 send_to_char(msg(MOVE_CANT_CONCENTRATE_ENOUGH, ch), ch);
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
	send_to_char(msg(MOVE_WARM_FEELING, victim), victim);
	if (ch != victim)
		send_to_char(msg(MOVE_OK, ch), ch);
	check_improve(ch,gsn_lay_hands,TRUE,1);

}

int mount_success (CHAR_DATA *ch, CHAR_DATA *mount, int canattack)
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
	send_to_char(msg(MOVE_MOUNT_DRUNKEN, ch), ch);
  }

  success = percent - get_skill(ch,gsn_riding);
  if(success <= 0) { /* Success */
	check_improve(ch, gsn_riding, TRUE, 1);
	return(1);
  } else {
	check_improve(ch, gsn_riding, FALSE, 1);
	if (success >= 10 && MOUNTED(ch) == mount) {
		act_nprintf(ch, NULL, mount, TO_CHAR, POS_DEAD, 
				MOVE_YOU_FALL_OFF_N);
		act_nprintf(ch, NULL, mount, TO_ROOM, POS_RESTING, 
				MOVE_N_FALLS_OFF_N);
		act_nprintf(ch, NULL, mount, TO_VICT, POS_SLEEPING, 
				MOVE_N_FALLS_OFF_YOU);

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
				MOVE_N_DOESNT_LIKE_YOU);
		act_nprintf(ch, NULL, mount, TO_ROOM, POS_RESTING,
				MOVE_N_DOESNT_LIKE_N);
		act_nprintf(ch, NULL, mount, TO_VICT, POS_SLEEPING,
				MOVE_YOU_DONT_LIKE_N);

		act_nprintf(ch, NULL, mount, TO_CHAR, POS_DEAD, 
				MOVE_N_SNARLS_YOU);
		act_nprintf(ch, NULL, mount, TO_ROOM, POS_RESTING,
				MOVE_N_SNARLS_N);
		act_nprintf(ch, NULL, mount, TO_VICT, POS_SLEEPING,
				MOVE_YOU_SNARL_N);  

		damage(mount, ch, number_range(1, mount->level), gsn_kick,DAM_BASH,TRUE);

/*      multi_hit(mount, ch, TYPE_UNDEFINED); */
	}
  }
  return(0);
}

/*
 * It is not finished yet to implement all.
 */
void do_mount(CHAR_DATA *ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  struct char_data *mount;

  argument = one_argument(argument, arg);

  if (IS_NPC(ch)) 
	return;

  if (arg[0] == '\0' && ch->mount && ch->mount->in_room == ch->in_room) {
	mount = ch->mount;
  } else if (arg[0] == '\0') {
	send_to_char(msg(MOVE_MOUNT_WHAT, ch), ch);
	return;
  }

 if (!(mount = get_char_room(ch, arg))) {
	send_to_char(msg(MOVE_YOU_DONT_SEE_THAT, ch), ch);
	return;
  }
 
  if (!SKILL_OK(ch,gsn_riding)) {
	send_to_char(msg(MOVE_DONT_KNOW_RIDE, ch), ch);
	return;
  } 

  if (!IS_NPC(mount) 
		|| !IS_SET(mount->act,ACT_RIDEABLE)
		  || IS_SET(mount->act,ACT_NOTRACK)) { 
	send_to_char(msg(MOVE_CANT_RIDE_THAT, ch), ch); 
	return;
  }
  
  if (mount->level - 5 > ch->level) {
	send_to_char(msg(MOVE_BEAST_TOO_POWERFUL, ch), ch);
	return;
  }    

  if((mount->mount) && (!mount->riding) && (mount->mount != ch)) {
	char_printf(ch, msg(MOVE_S_BELONGS_TO_S, ch), mount->short_descr,
		             mount->mount->name);
	return;
  } 

  if (mount->position < POS_STANDING) {
	send_to_char(msg(MOVE_MOUNT_MUST_STAND, ch), ch);
	return;
  }

  if (RIDDEN(mount)) {
	send_to_char(msg(MOVE_ALREADY_RIDDEN, ch), ch);
	return;
  } else if (MOUNTED(ch)) {
	send_to_char(msg(MOVE_ALREADY_RIDING, ch), ch);
	return;
  }

  if(!mount_success(ch, mount, TRUE)) {
	send_to_char(msg(MOVE_FAIL_TO_MOUNT, ch), ch);  
	return; 
  }

  act_nprintf(ch, NULL, mount, TO_CHAR, POS_DEAD, MOVE_YOU_HOP_ON_N);
  act_nprintf(ch, NULL, mount, TO_NOTVICT, POS_RESTING, MOVE_N_HOPS_ON_N);
  act_nprintf(ch, NULL, mount, TO_VICT, POS_SLEEPING, MOVE_N_HOPS_ON_YOU);
 
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

void do_dismount(CHAR_DATA *ch, char *argument)
{
  struct char_data *mount;

  if(MOUNTED(ch)) {
	mount = MOUNTED(ch);

	act_nprintf(ch, NULL, mount, TO_CHAR, POS_DEAD, MOVE_YOU_DISMOUNT_N);
	act_nprintf(ch, NULL, mount, TO_NOTVICT, POS_RESTING, 
				MOVE_N_DISMOUNTS_N);
	act_nprintf(ch, NULL, mount, TO_VICT, POS_SLEEPING, 
				MOVE_N_DISMOUNTS_YOU);

	ch->riding = FALSE;
	mount->riding = FALSE;
  } 
  else 
  {
	send_to_char(msg(MOVE_YOU_ARENT_MOUNTED, ch), ch);
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
				if (check_obj_dodge(ch,victim,arrow,chance))
					return 0;
				act_nprintf(victim, arrow, NULL, TO_CHAR, 
					    POS_RESTING, MOVE_P_STRIKES_YOU);
				act_nprintf(ch, arrow, victim, TO_CHAR, 
					    POS_DEAD, MOVE_YOU_P_STRIKES_N);
				if (ch->in_room == victim->in_room)
				    act_nprintf(ch, arrow, victim, TO_NOTVICT, 
					       POS_RESTING, MOVE_N_P_STRIKES_N);
		   else 
		   {
		     act("$n's $p strikes $N!", ch, arrow, victim, TO_ROOM);
		     act("$p strikes $n!", victim, arrow, NULL, TO_ROOM);
		   }
		   if (is_safe(ch,victim) || 
			(IS_NPC(victim) && IS_SET(victim->act,ACT_NOTRACK)))
		    {
		 	act("$p falls from $n doing no visible damage...",victim,arrow,NULL,TO_ALL);
		 	act("$p falls from $n doing no visible damage...",ch,arrow,NULL,TO_CHAR);
		 	obj_to_room(arrow,victim->in_room);
		    }
		   else
		    {
		        int dam;

			dam = dice(arrow->value[1],arrow->value[2]);
			dam = number_range(dam,2*dam);
			dam += damroll + bonus + 
				(10 * str_app[get_curr_stat(ch,STAT_STR)].todam);

		  	if (IS_WEAPON_STAT(arrow,WEAPON_POISON))
		  	{
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
		

void do_shoot(CHAR_DATA *ch, char *argument)
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

void do_human(CHAR_DATA *ch, char *argument)
{
	if (ch->class != CLASS_VAMPIRE)
	{
	 send_to_char(msg(MOVE_HUH, ch), ch);
	 return;
	}
	 
	if (!IS_VAMPIRE(ch))
	{
	 send_to_char(msg(MOVE_ALREADY_HUMAN, ch), ch);
	 return;
	}

   affect_strip(ch, gsn_vampire);
   REMOVE_BIT(ch->act,PLR_VAMPIRE);
   send_to_char(msg(MOVE_RETURN_TO_SIZE, ch), ch);
   return;
}


void do_throw_spear(CHAR_DATA *ch, char *argument)
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

