/*
 * $Id: raffect.c,v 1.5 1998-08-05 10:56:24 fjoe Exp $
 */

#include <sys/time.h>
#include <stdio.h>

#include "merc.h"
#include "db.h"
#include "log.h"
#include "comm.h"
#include "resource.h"
#include "recycle.h"
#include "interp.h"
#include "lookup.h"
#include "raffects.h"
#include "fight.h"

/* room affects by chronos */
void	affect_modify_room	args((ROOM_INDEX_DATA *room, AFFECT_DATA *paf, bool fAdd)); 

/*
 * Apply or remove an affect to a room.
 */
void affect_modify_room(ROOM_INDEX_DATA *room, AFFECT_DATA *paf, bool fAdd)
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

	return;
}

/*
 * Give an affect to a room.
 */
void affect_to_room(ROOM_INDEX_DATA *room, AFFECT_DATA *paf)
{
	AFFECT_DATA *paf_new;
	ROOM_INDEX_DATA *pRoomIndex;

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

	paf_new = new_affect();

	*paf_new		= *paf;
	paf_new->next	= room->affected;
	room->affected	= paf_new;

	affect_modify_room(room , paf_new, TRUE);
	return;
}

void affect_check_room(ROOM_INDEX_DATA *room,int where,int vector)
{
	AFFECT_DATA *paf;

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
void affect_remove_room(ROOM_INDEX_DATA *room, AFFECT_DATA *paf)
{
	int where;
	int vector;


	if (room->affected == NULL)
	{
		bug("Affect_remove_room: no affect.", 0);
		return;
	}

	affect_modify_room(room, paf, FALSE);
	where = paf->where;
	vector = paf->bitvector;

	if (paf == room->affected)
	{
		room->affected	= paf->next;
	}
	else
	{
		AFFECT_DATA *prev;

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

	free_affect(paf);

	affect_check_room(room,where,vector);
	return;
}

/*
 * Strip all affects of a given sn.
 */
void affect_strip_room(ROOM_INDEX_DATA *room, int sn)
{
	AFFECT_DATA *paf;
	AFFECT_DATA *paf_next;

	for (paf = room->affected; paf != NULL; paf = paf_next)
	{
		paf_next = paf->next;
		if (paf->type == sn)
		    affect_remove_room(room, paf);
	}

	return;
}



/*
 * Return true if a room is affected by a spell.
 */
bool is_affected_room(ROOM_INDEX_DATA *room, int sn)
{
	AFFECT_DATA *paf;

	for (paf = room->affected; paf != NULL; paf = paf->next)
	{
		if (paf->type == sn)
		    return TRUE;
	}

	return FALSE;
}



/*
 * Add or enhance an affect.
 */
void affect_join_room(ROOM_INDEX_DATA *room, AFFECT_DATA *paf)
{
	AFFECT_DATA *paf_old;
	bool found;

	found = FALSE;
	for (paf_old = room->affected; paf_old != NULL; paf_old = paf_old->next)
	{
		if (paf_old->type == paf->type)
		{
		    paf->level = (paf->level += paf_old->level) / 2;
		    paf->duration += paf_old->duration;
		    paf->modifier += paf_old->modifier;
		    affect_remove_room(room, paf_old);
		    break;
		}
	}

	affect_to_room(room, paf);
	return;
}


bool is_safe_rspell_nom(int level, CHAR_DATA *victim)
{
	/* ghosts are safe */
	if (!IS_NPC(victim) && IS_SET(victim->act, PLR_GHOST))
		return TRUE;
 
	/* link dead players who do not have rushing adrenalin are safe */
	if (!IS_NPC(victim) && !IS_PUMPED(victim) && victim->desc == NULL) 
		return TRUE;

	if (victim->level < 5 && !IS_NPC(victim))
		return TRUE;

	if (!IS_NPC(victim) && IS_SET(victim->act, PLR_GHOST))
		return TRUE;


	if (!IS_NPC(victim)
	&&  ((level >= victim->level + 5) || (victim->level >= level + 5)))
		return TRUE;

	return FALSE;
}


bool is_safe_rspell(int level, CHAR_DATA *victim)
{
  if (is_safe_rspell_nom(level,victim))
	{
	  act("The gods protect $n.",victim,NULL,NULL,TO_CHAR);
	  act("The gods protect $n from the spell of room.",victim,NULL,NULL,TO_ROOM);
	  return TRUE;
	}
  else return FALSE;
}
	  
		
void raffect_to_char(ROOM_INDEX_DATA *room, CHAR_DATA *ch)
{
  AFFECT_DATA *paf;

  if (IS_ROOM_AFFECTED(room, RAFF_LSHIELD))
  {
	 int sn;
	 CHAR_DATA *vch;

	 if ((sn = skill_lookup("lightning shield")) == -1)
		{ bug("Bad sn for lightning shield",0); return; }

	 for (vch=room->people;vch;vch=vch->next_in_room)
		{
		 if (is_room_owner(vch,room)) break;
		}

	if (!vch)
		{
		 bug("Owner of lightning shield left the room.",0);
		 free_string(room->owner);
		 room->owner = str_dup("");	 
		 affect_strip_room(room,sn); 
		}
	 else 
	 {
	  send_to_char("The protective shield of room blocks you.\n\r",ch);
	  act("$N has entered the room.",vch,NULL,ch,TO_CHAR);
	  do_wake(vch,"");

	  if ((paf = affect_find(room->affected,sn)) == NULL)
		 { bug("Bad paf for lightning shield",0); return; }

	  if (!is_safe_rspell(paf->level,ch)) 
		{
		 damage(vch,ch,dice(paf->level,4)+12,sn,DAM_LIGHTNING, TRUE);
		 free_string(room->owner);
		 room->owner = str_dup("");	 
		 affect_remove_room(room , paf);
		}
	 }
   }

  if (IS_ROOM_AFFECTED(room, RAFF_SHOCKING))
  {
	 int sn;

	 if ((sn = skill_lookup("shocking trap")) == -1)
		{ bug("Bad sn for shocking shield",0); return; }

	 send_to_char("The shocking waves of room shocks you.\n\r",ch);

	 if ((paf = affect_find(room->affected,sn)) == NULL)
		 { bug("Bad paf for shocking shield",0); return; }

	 if (!is_safe_rspell(paf->level,ch)) 
		{
		 if (check_immune(ch, DAM_LIGHTNING) != IS_IMMUNE)
		 damage(ch,ch,dice(paf->level,4)+12,TYPE_HUNGER,DAM_TRAP_ROOM, TRUE);
		 affect_remove_room(room , paf);
		}
   }

  if (IS_ROOM_AFFECTED(room, RAFF_THIEF_TRAP))
  {
	 send_to_char("The trap ,set by someone, blocks you.\n\r",ch);

	 if ((paf = affect_find(room->affected,gsn_settraps)) == NULL)
		 { bug("Bad paf for settraps",0); return; }

	 if (!is_safe_rspell(paf->level,ch)) 
		{
		 if (check_immune(ch, DAM_PIERCE) != IS_IMMUNE)
		 damage(ch,ch,dice(paf->level,5)+12,TYPE_HUNGER,DAM_TRAP_ROOM, TRUE);
		 affect_remove_room(room , paf);
		}
   }

  if (IS_ROOM_AFFECTED(room, RAFF_SLOW)
		|| IS_ROOM_AFFECTED(room, RAFF_SLEEP))
	 char_puts("There is some mist flowing in the air.\n\r",ch);

  return;
 }

void raffect_back_char(ROOM_INDEX_DATA *room, CHAR_DATA *ch)
{
	if (IS_ROOM_AFFECTED(room, RAFF_LSHIELD)) {
		int sn;

	if ((sn = skill_lookup("lightning shield")) == -1)
		{ bug("Bad sn for lightning shield",0); return; }
	if (is_room_owner(ch,room)) 
		{
		 free_string(room->owner);
		 room->owner = str_dup("");	 
		 affect_strip_room(room,sn); 
		}
   }
}


void do_raffects(CHAR_DATA *ch, const char *argument)
{
	AFFECT_DATA *paf, *paf_last = NULL;

	if (ch->in_room->affected == NULL) {
		send_to_char("The room is not affected by any spells.\n\r",ch);
		return;
	}

	send_to_char("The room is affected by the following spells:\n\r", ch);
	for (paf = ch->in_room->affected; paf != NULL; paf = paf->next) {
		if (paf_last != NULL && paf->type == paf_last->type)
			if (ch->level >= 20)
				char_puts("                      ", ch);
			else
				continue;
		else
			char_printf(ch, "%s {c%-15s{x", msg(AFF_SPELL, ch),
				    paf->type > 0 ?
				    skill_table[paf->type].name : "none");

		if (ch->level >= 20) {
			char_printf(ch, ": %s {c%s{x %s {c%d{x ",
				    msg(AFF_MODIFIES, ch),
				    raffect_loc_name(paf->location),
				    msg(AFF_BY, ch),
				    paf->modifier);
			if (paf->duration == -1 || paf->duration == -2)
				char_nputs(AFF_PERMANENTLY, ch);
			else
				char_printf(ch, msg(AFF_FOR_D_HOURS, ch),
					    paf->duration);
		}
		send_to_char("\n\r", ch);
		paf_last = paf;
	}
}

