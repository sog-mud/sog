/*
 * $Id: raffect.c,v 1.27 1999-06-21 20:11:16 avn Exp $
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

#include "merc.h"
#include "fight.h"

/*
 * Apply or remove an affect to a room.
 */
void affect_modify_room(ROOM_INDEX_DATA *room, ROOM_AFFECT_DATA *paf, bool fAdd)
{
	int mod;

	mod = paf->modifier;

	if (fAdd)
	{
		switch (paf->where)
		{
		case TO_ROOM_AFFECTS:
		      SET_BIT(room->affected_by, paf->bitvector);
		    break;
		case TO_ROOM_FLAGS:
		      SET_BIT(room->room_flags, paf->bitvector);
		    break;
		case TO_ROOM_CONST:
		    break;
		}
	}
	else
	{
	    switch (paf->where)
	    {
	    case TO_ROOM_AFFECTS:
	          REMOVE_BIT(room->affected_by, paf->bitvector);
	        break;
		case TO_ROOM_FLAGS:
		      REMOVE_BIT(room->room_flags, paf->bitvector);
		    break;
	    case TO_ROOM_CONST:
	        break;
	    }
		mod = 0 - mod;
	}

	switch (paf->location)
	{
	default:
		bug("Affect_modify_room: unknown location %d.", paf->location);
		return;

	case APPLY_ROOM_NONE:					break;
	case APPLY_ROOM_HEAL:	room->heal_rate += mod;		break;
	case APPLY_ROOM_MANA:	room->mana_rate += mod;		break;
	}
}

/*
 * Give an affect to a room.
 */
void affect_to_room(ROOM_INDEX_DATA *room, ROOM_AFFECT_DATA *paf)
{
	ROOM_AFFECT_DATA *paf_new;
	ROOM_INDEX_DATA *pRoomIndex;

	if (paf->owner == NULL) {
		log("[*****] BUG: affect_to_room: NULL owner");
		return;
	}

	if (! room->affected)
	{
	 if (top_affected_room)
	 {
	  for (pRoomIndex  = top_affected_room;
		  pRoomIndex->aff_next != NULL;
		  pRoomIndex  = pRoomIndex->aff_next)
				continue;
	  pRoomIndex->aff_next = room;	
	 }
	 else top_affected_room = room;
	 room->aff_next = NULL;
	}

	paf_new = raff_new();

	*paf_new	= *paf;
	paf_new->next	= room->affected;
	room->affected	= paf_new;

	affect_modify_room(room , paf_new, TRUE);
}

void affect_check_room(ROOM_INDEX_DATA *room,int where,int vector)
{
	ROOM_AFFECT_DATA *paf;

	if (vector == 0)
		return;

	for (paf = room->affected; paf != NULL; paf = paf->next)
		if (paf->where == where && paf->bitvector == vector)
		{
		    switch (where)
		    {
		        case TO_ROOM_AFFECTS:
			      SET_BIT(room->affected_by,vector);
			    break;
			case TO_ROOM_FLAGS:
		      	      SET_BIT(room->room_flags, vector);
		    	    break;
		        case TO_ROOM_CONST:
			    break;
		    }
		    return;
		}
}

/*
 * Remove an affect from a room.
 */
void affect_remove_room(ROOM_INDEX_DATA *room, ROOM_AFFECT_DATA *paf)
{
	int where;
	int vector;


	if (room->affected == NULL) {
		bug("Affect_remove_room: no affect.", 0);
		return;
	}

	affect_modify_room(room, paf, FALSE);
	where = paf->where;
	vector = paf->bitvector;

	if (paf == room->affected)
		room->affected	= paf->next;
	else
	{
		ROOM_AFFECT_DATA *prev;

		for (prev = room->affected; prev != NULL; prev = prev->next)
		{
		    if (prev->next == paf)
		    {
			prev->next = paf->next;
			break;
		    }
		}

		if (prev == NULL)
		{
		    bug("Affect_remove_room: cannot find paf.", 0);
		    return;
		}
	}

	if (!room->affected)
	{
	 ROOM_INDEX_DATA *prev;

	 if (top_affected_room  == room)
		{
		 top_affected_room = room->aff_next;
		}
	 else
	    {
	     for(prev = top_affected_room; prev->aff_next; prev = prev->aff_next)
		  {
		    if (prev->aff_next == room)
		    {
			prev->aff_next = room->aff_next;
			break;
		    }
		  }
		 if (prev == NULL)
		  {
		    bug("Affect_remove_room: cannot find room.", 0);
		    return;
		  }
	    }
	  room->aff_next = NULL;

	 }

	raff_free(paf);

	affect_check_room(room,where,vector);
}

/*
 * Strip all affects of a given sn.
 */
void affect_strip_room(ROOM_INDEX_DATA *room, int sn)
{
	ROOM_AFFECT_DATA *paf;
	ROOM_AFFECT_DATA *paf_next;

	for (paf = room->affected; paf != NULL; paf = paf_next) {
		paf_next = paf->next;
		if (paf->type == sn)
			affect_remove_room(room, paf);
	}
}

/*
 * Return true if a room is affected by a spell.
 */
bool is_affected_room(ROOM_INDEX_DATA *room, int sn)
{
	ROOM_AFFECT_DATA *paf;

	for (paf = room->affected; paf != NULL; paf = paf->next) {
		if (paf->type == sn)
			return TRUE;
	}

	return FALSE;
}

/*
 * Add or enhance an affect.
 */
void affect_join_room(ROOM_INDEX_DATA *room, ROOM_AFFECT_DATA *paf)
{
	ROOM_AFFECT_DATA *paf_old;
	bool found;

	found = FALSE;
	for (paf_old = room->affected; paf_old != NULL; paf_old = paf_old->next) {
		if (paf_old->type == paf->type) {
			paf->level = (paf->level += paf_old->level) / 2;
			paf->duration += paf_old->duration;
			paf->modifier += paf_old->modifier;
			affect_remove_room(room, paf_old);
			break;
		}
	}

	affect_to_room(room, paf);
}

bool is_safe_rspell_nom(ROOM_AFFECT_DATA *raf, CHAR_DATA *victim)
{
	if (raf->owner) return is_safe_nomessage(victim, raf->owner);
	bug("is_safe_rspell_nom: no affect owner", 0);
	affect_remove_room(victim->in_room, raf);
	return TRUE; /* protected from broken raffs */ 
}


bool is_safe_rspell(ROOM_AFFECT_DATA *raf, CHAR_DATA *victim)
{
  if (is_safe_rspell_nom(raf,victim))
	{
	  act("The gods protect you from the spell of room.",victim,NULL,NULL,TO_CHAR);
	  return TRUE;
	}
  else return FALSE;
}

void check_room_affects(CHAR_DATA *ch, ROOM_INDEX_DATA *room, int event)
{
	ROOM_AFFECT_DATA *raf, *raf_next;

	if (!room->affected || event == EVENT_NONE) return;

	for (raf = room->affected; raf != NULL; raf = raf_next) {
		raf_next = raf->next;

		if (raf->event != event)
			continue;

		if (!raf->owner) {
			affect_remove_room(room, raf);
			continue;
		}

		if (raf->event_fun == NULL) {
			log("[*****] BUG: check_room_affects: "
				   "no event_fun for %d event", event);
			continue;
		}

		raf->event_fun(room, ch, raf);
		if (IS_EXTRACTED(ch))
			break;
	}
}
	  
void do_raffects(CHAR_DATA *ch, const char *argument)
{
	ROOM_AFFECT_DATA *paf, *paf_last = NULL;

	if (ch->in_room->affected == NULL) {
		char_puts("The room is not affected by any spells.\n",ch);
		return;
	}

	char_puts("The room is affected by the following spells:\n", ch);
	for (paf = ch->in_room->affected; paf != NULL; paf = paf->next) {
		if (paf_last != NULL && paf->type == paf_last->type)
			if (ch->level >= 20)
				char_puts("                      ", ch);
			else
				continue;
		else
			char_printf(ch, "Spell: {c%-15s{x",
				    skill_name(paf->type));

		if (ch->level >= 20) {
			char_printf(ch, ": modifies {c%s{x by {c%d{x ",
				    flag_string(rapply_flags, paf->location),
				    paf->modifier);
			if (paf->duration == -1 || paf->duration == -2)
				char_puts("permanently.", ch);
			else
				char_printf(ch, "for {c%d{x hours.",
					    paf->duration);
		}
		char_puts("\n", ch);
		paf_last = paf;
	}
}

