/*
 * Copyright (c) 1999 Arborn <avn@iclub.nsu.ru>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id: raffect.c,v 1.31 1999-10-06 09:56:09 fjoe Exp $
 */

#include <sys/time.h>
#include <stdio.h>

#include "merc.h"
#include "fight.h"

hash_t rspells;

void rspell_init(rspell_t *rsp)
{
	rsp->sn = str_empty;
	rsp->enter_fun_name = str_empty;
	rsp->update_fun_name = str_empty;
	rsp->leave_fun_name = str_empty;
	rsp->enter_fun = NULL;
	rsp->update_fun = NULL;
	rsp->leave_fun = NULL;
	rsp->revents = 0;
}

rspell_t *rspell_cpy(rspell_t *dst, const rspell_t *src)
{
	dst->sn = str_qdup(src->sn);
	dst->enter_fun_name = str_qdup(src->enter_fun_name);
	dst->update_fun_name = str_qdup(src->update_fun_name);
	dst->leave_fun_name = str_qdup(src->leave_fun_name);
	dst->enter_fun = src->enter_fun;
	dst->update_fun = src->update_fun;
	dst->leave_fun = src->leave_fun;
	dst->revents = src->revents;
	return dst;
}

void rspell_destroy(rspell_t *rsp)
{
	free_string(rsp->sn);
	free_string(rsp->enter_fun_name);
	free_string(rsp->update_fun_name);
	free_string(rsp->leave_fun_name);
}

/*
 * Update room affect events
 */
void check_room_events(ROOM_INDEX_DATA *room)
{
	ROOM_AFFECT_DATA *raf;

	if (!room->affected) return;
	room->revents = 0;
	for (raf = room->affected; raf; raf = raf->next)
		room->revents |= raf->revents;
}

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
	case APPLY_ROOM_HEAL:	room->heal_rate   += mod;	break;
	case APPLY_ROOM_MANA:	room->mana_rate   += mod;	break;
	case APPLY_ROOM_SECT:	room->sector_type += mod;	break;
	}
	check_room_events(room);
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

	affect_modify_room(room, paf_new, TRUE);
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
	check_room_events(room);
}

/*
 * Strip all affects of a given sn.
 */
void affect_strip_room(ROOM_INDEX_DATA *room, const char *sn)
{
	ROOM_AFFECT_DATA *paf;
	ROOM_AFFECT_DATA *paf_next;

	for (paf = room->affected; paf != NULL; paf = paf_next) {
		paf_next = paf->next;
		if (SKILL_IS(paf->type, sn))
			affect_remove_room(room, paf);
	}
}

/*
 * Return true if a room is affected by a spell.
 */
bool is_affected_room(ROOM_INDEX_DATA *room, const char *sn)
{
	ROOM_AFFECT_DATA *paf;

	for (paf = room->affected; paf != NULL; paf = paf->next) {
		if (SKILL_IS(paf->type, sn))
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
	  act("The gods protect you from the spell of room.",
	      victim, NULL, NULL, TO_CHAR);
	  return TRUE;
	}
  else return FALSE;
}

#define SAFE_RSPELL_CALL(fun)				\
	do {						\
		if (fun)				\
			(fun)(room, ch, raf);		\
		else					\
			continue;			\
	} while(0);

void check_room_affects(CHAR_DATA *ch, ROOM_INDEX_DATA *room, flag32_t event)
{
	ROOM_AFFECT_DATA *raf, *raf_next;

	if (!IS_SET(room->revents, event))
		return;

	for (raf = room->affected; raf != NULL; raf = raf_next) {
		rspell_t *rsp;
		raf_next = raf->next;

		if (!IS_SET(raf->revents, event))
			continue;

		if (!raf->owner) {
			affect_remove_room(room, raf);
			continue;
		}

		if ((rsp = rspell_lookup(raf->type)) == NULL)
			continue;

		switch (event) {
		default:
			bug("check_room_affects: %d: no such event %d", event);
			return;
		case REVENT_ENTER:
			SAFE_RSPELL_CALL(rsp->enter_fun);
			break;
		case REVENT_LEAVE:
			SAFE_RSPELL_CALL(rsp->leave_fun);
			break;
		case REVENT_UPDATE:
			SAFE_RSPELL_CALL(rsp->update_fun);
			break;
		}
		if (IS_EXTRACTED(ch))
			break;
	}
}
