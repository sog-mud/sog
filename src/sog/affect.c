/*
 * $Id: affect.c,v 1.16 1999-12-14 05:25:26 fjoe Exp $
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

/**************************************************************************r
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

#include <stdio.h>
#include <stdlib.h>
#include "merc.h"
#include "db.h"

AFFECT_DATA *aff_new(void)
{
	top_affect++;
	return calloc(1, sizeof(AFFECT_DATA));
}

AFFECT_DATA *aff_dup(const AFFECT_DATA *paf)
{
	AFFECT_DATA *naf = aff_new();
	naf->where	= paf->where;
	naf->type	= str_dup(paf->type);
	naf->level	= paf->level;
	naf->duration	= paf->duration;
	switch (paf->where) {
	case TO_RACE:
	case TO_SKILLS:
		naf->location.s = str_dup(paf->location.s);
		break;
	default:
		INT(naf->location) = INT(paf->location);
		break;
	}
	naf->modifier	= paf->modifier;
	naf->bitvector	= paf->bitvector;
	naf->owner	= paf->owner;
	naf->events	= paf->events;
	return naf;
}

void aff_free(AFFECT_DATA *af)
{
	switch (af->where) {
	case TO_RACE:
	case TO_SKILLS:
		free_string(af->location.s);
		break;
	}
	free_string(af->type);
	free(af);
	top_affect--;
}

void aff_free_list(AFFECT_DATA *paf)
{
	AFFECT_DATA *paf_next;

	for (; paf != NULL; paf = paf_next) {
		paf_next = paf->next;
		aff_free(paf);
	}
}

void saff_init(saff_t *sa)
{
	sa->sn = str_empty;
	sa->type = str_empty;
	sa->mod = 0;
	sa->bit = 0;
}

void saff_destroy(saff_t *sa)
{
	free_string(sa->sn);
	free_string(sa->type);
}

where_t where_table[] =
{
	{ TO_AFFECTS,	affect_flags,	"'%s' affect"			},
	{ TO_SKILLS,	sk_aff_flags,	"'%s' skill by %d with flags %s"},
	{ TO_RACE,	NULL,		"changes race to '%s'"		},
	{ TO_DETECTS,	id_flags,	"detection of '%s'"		},
	{ TO_INVIS,	id_flags,	"'%s'"				},
	{ -1 }
};

where_t *where_lookup(flag_t where)
{
	where_t *wd;

	for (wd = where_table; wd->where != -1; wd++)
		if (wd->where == where)
			return wd;
	return NULL;
}

/* enchanted stuff for eq */
void affect_enchant(OBJ_DATA *obj)
{
	/* okay, move all the old flags into new vectors if we have to */
	if (!IS_OBJ_STAT(obj, ITEM_ENCHANTED)) {
		AFFECT_DATA *paf;
		SET_OBJ_STAT(obj, ITEM_ENCHANTED);

		for (paf = obj->pObjIndex->affected;
						paf != NULL; paf = paf->next) {
			AFFECT_DATA *af_new;
			af_new = aff_dup(paf);
			af_new->next = obj->affected;
			obj->affected = af_new;
		}
	}
}
 
static void *
remove_sa_cb(void *p, va_list ap)
{
	saff_t *sa = (saff_t *) p;

	varr *v = va_arg(ap, varr *);
	AFFECT_DATA *paf = va_arg(ap, AFFECT_DATA *);

	if (!IS_SKILL(sa->sn, paf->location.s)
	||  !IS_SKILL(sa->type, paf->type)
	||  sa->mod != paf->modifier
	||  sa->bit != paf->bitvector)
		return NULL;

	varr_edelete(v, p);

	/*
	 * restart from this place
	 */
	return p;
}

/*
 * Apply or remove an affect to a character.
 */
void affect_modify(CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd)
{
	OBJ_DATA *wield, *obj2;
	int mod, i;

	if (paf->where == TO_SKILLS) {
		if (fAdd) {
			saff_t *sa = varr_enew(&ch->sk_affected);
			sa->sn = str_dup(paf->location.s);
			sa->type = str_dup(paf->type);
			sa->mod = paf->modifier;
			sa->bit =  paf->bitvector;
		} else {
			void *p = NULL;

			do {
				p = varr_eforeach(&ch->sk_affected, p,
						  remove_sa_cb,
						  &ch->sk_affected, paf);
			} while (p);
		}
		return;
	} else if (paf->where == TO_RACE) {
		free_string(ch->race);
		ch->race = str_dup(fAdd ? paf->location.s : ORG_RACE(ch));
		race_resetstats(ch);
		affect_check(ch, -1, -1);
		spec_update(ch);
		return;
	}

	mod = paf->modifier;
	if (fAdd) {
		switch (paf->where) {
		case TO_AFFECTS:
			SET_BIT(ch->affected_by, paf->bitvector);
			break;
		case TO_DETECTS:
			SET_DETECT(ch, paf->bitvector);
			break;
		case TO_INVIS:
			SET_INVIS(ch, paf->bitvector);
			break;
		}
	} else {
		switch (paf->where) {
		case TO_AFFECTS:
			REMOVE_BIT(ch->affected_by, paf->bitvector);
			break;
		case TO_DETECTS:
			REMOVE_DETECT(ch, paf->bitvector);
			break;
		case TO_INVIS:
			REMOVE_DETECT(ch, paf->bitvector);
			break;
		}
		mod = 0 - mod;
	}

	switch (INT(paf->location)) {
	case APPLY_NONE:
	case APPLY_HEIGHT:
	case APPLY_WEIGHT:
	case APPLY_GOLD:
	case APPLY_EXP:
	case APPLY_SEX:
		break;

	case APPLY_STR:		ch->mod_stat[STAT_STR]	+= mod; break;
	case APPLY_DEX:		ch->mod_stat[STAT_DEX]	+= mod;	break;
	case APPLY_INT:		ch->mod_stat[STAT_INT]	+= mod;	break;
	case APPLY_WIS:		ch->mod_stat[STAT_WIS]	+= mod;	break;
	case APPLY_CON:		ch->mod_stat[STAT_CON]	+= mod;	break;
	case APPLY_CHA:		ch->mod_stat[STAT_CHA]	+= mod; break;

	case APPLY_MANA:	ch->max_mana		+= mod;	break;
	case APPLY_HIT:		ch->max_hit		+= mod;	break;
	case APPLY_MOVE:	ch->max_move		+= mod;	break;

	case APPLY_HITROLL:	ch->hitroll		+= mod;	break;
	case APPLY_DAMROLL:	ch->damroll		+= mod;	break;
	case APPLY_LEVEL:	ch->add_level		+= mod; break;

	case APPLY_SIZE:	ch->size		+= mod;	break;
	case APPLY_AGE:	
		if (!IS_NPC(ch))
			PC(ch)->add_age += age_to_num(mod);
		break;

	case APPLY_AC:
		for (i = 0; i < 4; i ++)
			ch->armor[i] += mod;
		break;

	case APPLY_SAVES:		ch->saving_throw	+= mod;	break;
	case APPLY_SAVING_ROD:		ch->saving_throw	+= mod;	break;
	case APPLY_SAVING_PETRI:	ch->saving_throw	+= mod;	break;
	case APPLY_SAVING_BREATH:	ch->saving_throw	+= mod;	break;
	case APPLY_SAVING_SPELL:	ch->saving_throw	+= mod;	break;

	case APPLY_RESIST_BASH:
	case APPLY_RESIST_PIERCE:
	case APPLY_RESIST_SLASH:
	case APPLY_RESIST_FIRE:
	case APPLY_RESIST_COLD:
	case APPLY_RESIST_LIGHTNING:
	case APPLY_RESIST_ACID:
	case APPLY_RESIST_HOLY:
	case APPLY_RESIST_NEGATIVE:
	case APPLY_RESIST_ENERGY:
	case APPLY_RESIST_MENTAL:
	case APPLY_RESIST_SOUND:
	case APPLY_RESIST_DISEASE:
	case APPLY_RESIST_POISON:
	case APPLY_RESIST_CHARM:
	case APPLY_RESIST_HARM:
	case APPLY_RESIST_LIGHT:

		ch->resists[INT(paf->location)-APPLY_RESIST_BASH] += mod;
		break;
	default:
		if (IS_NPC(ch)) {
			log("affect_modify: vnum %d: in room %d: "
				   "unknown location %d, where: %d",
				   ch->pMobIndex->vnum,
				   ch->in_room ? ch->in_room->vnum : -1,
				   paf->location, paf->where);
		}
		else {
			log("affect_modify: %s: unknown location %d"
				"where: %d", ch->name, paf->location,
				paf->where);
		}
		return;

	}

	/*
	 * Check for weapon wielding.
	 * Guard against recursion (for weapons with affects).
	 * May be called from char_load (ch->in_room will be NULL)
	 */
	if (!IS_NPC(ch)
	&&  ch->in_room != NULL
	&&  (wield = get_eq_char(ch, WEAR_WIELD)) != NULL
	&&  get_obj_weight(wield) > (str_app[get_curr_stat(ch,STAT_STR)].wield*10))
	{
		static int depth;

		if (depth == 0) {
		    depth++;
		    act("You drop $p.", ch, wield, NULL, TO_CHAR);
		    act("$n drops $p.", ch, wield, NULL, TO_ROOM);
		    obj_from_char(wield);
		    obj_to_room(wield, ch->in_room);

		if ((obj2 = get_eq_char(ch, WEAR_SECOND_WIELD)) != NULL) {
			act("You wield your second weapon as your first!",  ch, NULL,NULL,TO_CHAR);
			act("$n wields $s second weapon as first!",  ch, NULL,NULL,TO_ROOM);
		unequip_char(ch, obj2);
		equip_char(ch, obj2 , WEAR_WIELD);
		}
		    depth--;
		}
	}
}

/* find an affect in an affect list */
AFFECT_DATA  *affect_find(AFFECT_DATA *paf, const char *sn)
{
	STRKEY_CHECK(&skills, sn, "affect_find");

	for (; paf != NULL; paf = paf->next) {
		if (IS_SKILL(paf->type, sn))
			return paf;
	}

	return NULL;
}

void affect_check_list(CHAR_DATA *ch, AFFECT_DATA *paf,
		       int where, flag_t vector)
{
	for (; paf; paf = paf->next) {
		if ((where < 0 || paf->where == where)
		&&  (paf->bitvector & vector)) {
			switch (paf->where) {
			case TO_AFFECTS:
				SET_BIT(ch->affected_by, paf->bitvector);
				break;
			case TO_DETECTS:
				SET_DETECT(ch, paf->bitvector);
				break;
			case TO_INVIS:
				SET_INVIS(ch, paf->bitvector);
				break;
			}
		}
	}
}

/* fix object affects when removing one */
void affect_check(CHAR_DATA *ch, int where, flag_t vector)
{
	OBJ_DATA *obj;

	if (where == TO_OBJECT || where == TO_WEAPON || vector == 0)
		return;

	affect_check_list(ch, ch->affected, where, vector);
	for (obj = ch->carrying; obj != NULL; obj = obj->next_content) {
		if (obj->wear_loc == WEAR_NONE
		||  obj->wear_loc == WEAR_STUCK_IN)
			continue;
		affect_check_list(ch, obj->affected, where, vector);

		if (IS_OBJ_STAT(obj, ITEM_ENCHANTED))
			continue;

		affect_check_list(ch, obj->pObjIndex->affected, where, vector);
	}
}

/*
 * Give an affect to a char.
 */
void affect_to_char(CHAR_DATA *ch, AFFECT_DATA *paf)
{
	AFFECT_DATA *paf_new = aff_dup(paf);

	STRKEY_CHECK(&skills, paf->type, "affect_to_char");

	paf_new->next = ch->affected;
	ch->affected = paf_new;
	affect_modify(ch, aff_dup(paf), TRUE);
}

/* give an affect to an object */
void affect_to_obj(OBJ_DATA *obj, AFFECT_DATA *paf)
{
	AFFECT_DATA *paf_new = aff_dup(paf);

	STRKEY_CHECK(&skills, paf->type, "affect_to_obj");

	paf_new->next	= obj->affected;
	obj->affected	= paf_new;

	if (paf->bitvector) {
		switch (paf->where) {
		case TO_OBJECT:
			SET_OBJ_STAT(obj, paf->bitvector);
			break;
		case TO_WEAPON:
			if (obj->pObjIndex->item_type == ITEM_WEAPON)
		        	SET_BIT(INT(obj->value[4]), paf->bitvector);
			break;
		}
	}
}

/*
 * Remove an affect from a char.
 */
void affect_remove(CHAR_DATA *ch, AFFECT_DATA *paf)
{
	int where;
	int vector;

	if (ch->affected == NULL) {
		bug("Affect_remove: no affect.", 0);
		return;
	}

	affect_modify(ch, paf, FALSE);
	where = paf->where;
	vector = paf->bitvector;

	if (paf == ch->affected)
		ch->affected	= paf->next;
	else {
		AFFECT_DATA *prev;

		for (prev = ch->affected; prev; prev = prev->next) {
			if (prev->next == paf) {
				prev->next = paf->next;
				break;
			}
		}

		if (prev == NULL) {
			bug("Affect_remove: cannot find paf.", 0);
			return;
		}
	}

	aff_free(paf);
	affect_check(ch, where, vector);
}

void affect_remove_obj(OBJ_DATA *obj, AFFECT_DATA *paf)
{
	int where, vector;

	if (obj->affected == NULL)
		return;

	if (obj->carried_by != NULL && obj->wear_loc != -1)
		affect_modify(obj->carried_by, paf, FALSE);

	where = paf->where;
	vector = paf->bitvector;

	/* remove flags from the object if needed */
	if (paf->bitvector)
		switch(paf->where) {
		case TO_OBJECT:
			REMOVE_OBJ_STAT(obj, paf->bitvector);
			break;
		case TO_WEAPON:
			if (obj->pObjIndex->item_type == ITEM_WEAPON)
				REMOVE_BIT(INT(obj->value[4]),
					   paf->bitvector);
			break;
		}

	if (paf == obj->affected)
	    obj->affected    = paf->next;
	else
	{
	    AFFECT_DATA *prev;

	    for (prev = obj->affected; prev != NULL; prev = prev->next)
	    {
	        if (prev->next == paf)
	        {
	            prev->next = paf->next;
	            break;
	        }
	    }

	    if (prev == NULL)
	    {
	        bug("Affect_remove_object: cannot find paf.", 0);
	        return;
	    }
	}

	aff_free(paf);

	if (obj->carried_by != NULL && obj->wear_loc != -1)
		affect_check(obj->carried_by, where, vector);
}

/*
 * Strip all affects of a given sn.
 */
void affect_strip(CHAR_DATA *ch, const char *sn)
{
	AFFECT_DATA *paf;
	AFFECT_DATA *paf_next;

	STRKEY_CHECK(&skills, sn, "affect_strip");

	for (paf = ch->affected; paf; paf = paf_next) {
		paf_next = paf->next;

		if (!IS_SKILL(paf->type, sn))
			continue;

		affect_remove(ch, paf);
	}
}

/*
 * strip all affects which affect given bitvector
 */
void affect_bit_strip(CHAR_DATA *ch, int where, flag_t bits)
{
	AFFECT_DATA *paf;
	AFFECT_DATA *paf_next;

	for (paf = ch->affected; paf; paf = paf_next) {
		paf_next = paf->next;
		if (paf->where == where && (paf->bitvector & bits))
			affect_remove(ch, paf);
	}
}

AFFECT_DATA *is_bit_affected(CHAR_DATA *ch, int where, flag_t bits)
{
	AFFECT_DATA *paf;

	for (paf = ch->affected; paf; paf = paf->next)
		if (paf->where == where && (paf->bitvector & bits))
			return paf;

	return NULL;
}

bool has_obj_affect(CHAR_DATA *ch, int vector)
{
	OBJ_DATA *obj;

	for (obj = ch->carrying; obj; obj = obj->next_content) {
		AFFECT_DATA *paf;

		if (obj->wear_loc == -1 || obj->wear_loc == WEAR_STUCK_IN)
			continue;

		for (paf = obj->affected; paf; paf = paf->next)
	        	if (paf->bitvector & vector)
				return TRUE;

		if (IS_OBJ_STAT(obj, ITEM_ENCHANTED))
			continue;

		for (paf = obj->pObjIndex->affected; paf; paf = paf->next)
			if (paf->bitvector & vector)
				return TRUE;
	}
	return FALSE;
}

/*
 * Add or enhance an affect.
 */
void affect_join(CHAR_DATA *ch, AFFECT_DATA *paf)
{
	AFFECT_DATA *paf_old;
	bool found;

	found = FALSE;
	for (paf_old = ch->affected; paf_old != NULL; paf_old = paf_old->next) {
		if (IS_SKILL(paf_old->type, paf->type)
		&&  paf_old->where == paf->where) {
			paf->level = (paf->level += paf_old->level) / 2;
			paf->duration += paf_old->duration;
			paf->modifier += paf_old->modifier;
			affect_remove(ch, paf_old);
			break;
		}
	}

	affect_to_char(ch, paf);
}

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

	switch (INT(paf->location))
	{
	default:
		bug("Affect_modify_room: unknown location %d.", paf->location);
		return;

	case APPLY_ROOM_NONE:					break;
	case APPLY_ROOM_HEAL:	room->heal_rate   += mod;	break;
	case APPLY_ROOM_MANA:	room->mana_rate   += mod;	break;
	case APPLY_ROOM_SECT:	room->sector_type += mod;	break;
	}
}

/*
 * Give an affect to a room.
 */
void affect_to_room(ROOM_INDEX_DATA *room, AFFECT_DATA *paf)
{
	AFFECT_DATA *paf_new;

	if (paf->owner == NULL) {
		log("[*****] BUG: affect_to_room: NULL owner");
		return;
	}

	if (!room->affected) {
		room->aff_next = top_affected_room;
		top_affected_room = room;
	}

	paf_new = aff_new();

	*paf_new	= *paf;
	paf_new->next	= room->affected;
	room->affected	= paf_new;

	affect_modify_room(room, paf_new, TRUE);
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

	if (!room->affected) {
		ROOM_INDEX_DATA *prev;

		if (top_affected_room  == room)
			top_affected_room = room->aff_next;
		else {
			for(prev = top_affected_room;
				prev->aff_next && prev->aff_next != room;
				prev = prev->aff_next);
			if (prev == NULL) {
				bug("Affect_remove_room: cannot find room.", 0);
				return;
			}
			prev->aff_next = room->aff_next;
		}
		room->aff_next = NULL;
	}

	aff_free(paf);

	affect_check_room(room,where,vector);
}

/*
 * Strip all affects of a given sn.
 */
void affect_strip_room(ROOM_INDEX_DATA *room, const char *sn)
{
	AFFECT_DATA *paf;
	AFFECT_DATA *paf_next;

	for (paf = room->affected; paf != NULL; paf = paf_next) {
		paf_next = paf->next;
		if (IS_SKILL(paf->type, sn))
			affect_remove_room(room, paf);
	}
}

/*
 * Return true if a room is affected by a spell.
 */
bool is_affected_room(ROOM_INDEX_DATA *room, const char *sn)
{
	AFFECT_DATA *paf;

	for (paf = room->affected; paf != NULL; paf = paf->next) {
		if (IS_SKILL(paf->type, sn))
			return TRUE;
	}

	return FALSE;
}

void strip_raff_owner(CHAR_DATA *ch)
{
	ROOM_INDEX_DATA *room, *room_next;
	AFFECT_DATA *af, *af_next;

	for (room = top_affected_room; room; room = room_next) {
		room_next = room->aff_next;

		for (af = room->affected; af; af = af_next) {
			af_next = af->next;
			if (af->owner == ch) affect_remove_room(room, af);
		}
	}
}
/*----------------------------------------------------------------------------
 * show affects stuff
 */

void show_name(CHAR_DATA *ch, BUFFER *output,
	       AFFECT_DATA *paf, AFFECT_DATA *paf_last)
{
	if (paf_last && paf->type == paf_last->type)
		if (ch && ch->level < 20)
			return;
		else
			buf_add(output, "                      ");
	else
		buf_printf(output, "Spell: {c%-15s{x", paf->type);
}

void show_duration(BUFFER *output, AFFECT_DATA *paf)
{
	if (paf->duration < 0)
		buf_add(output, "permanently.\n");
	else 
		buf_act(output, "for {c$j{x $qj{hours}.", NULL,
			(const void*) paf->duration, NULL, NULL, ACT_NOUCASE);
}

void show_loc_affect(CHAR_DATA *ch, BUFFER *output,
		 AFFECT_DATA *paf, AFFECT_DATA **ppaf)
{
	if (INT(paf->location) == APPLY_NONE
	&&  paf->bitvector)
		return;

	show_name(ch, output, paf, *ppaf);
	if (paf->where == TO_SKILLS
	||  paf->where == TO_RACE)
		buf_add(output, ": ");
	else {
		buf_printf(output, ": modifies {c%s{x by {c%d{x ",
			   SFLAGS(apply_flags, paf->location),
			   paf->modifier);
	}
	show_duration(output, paf);
	*ppaf = paf;
}

void show_bit_affect(BUFFER *output, AFFECT_DATA *paf, AFFECT_DATA **ppaf)
{
	char buf[MAX_STRING_LENGTH];
	where_t *w;

	if (paf->where == TO_SKILLS
	||  paf->where == TO_RACE
	||  (w = where_lookup(paf->where)) == NULL
	||  !paf->bitvector)
		return;

	show_name(NULL, output, paf, *ppaf);
	snprintf(buf, sizeof(buf), ": adds %s ", w->format);
	buf_printf(output, buf, flag_string(w->table, paf->bitvector));
	show_duration(output, paf);
	*ppaf = paf;
}

void show_obj_affects(CHAR_DATA *ch, BUFFER *output, AFFECT_DATA *paf)
{
	AFFECT_DATA *paf_last = NULL;

	for (; paf; paf = paf->next) {
		if (paf->where == TO_SKILLS
		||  paf->where == TO_RACE)
			continue;
		show_bit_affect(output, paf, &paf_last);
	}
}

void show_affects(CHAR_DATA *ch, BUFFER *output)
{
	OBJ_DATA *obj;
	AFFECT_DATA *paf, *paf_last = NULL;

	buf_add(output, "You are affected by the following spells:\n");
	for (paf = ch->affected; paf; paf = paf->next) {
		if (ch->level < 20) {
			show_name(ch, output, paf, paf_last);
			if (paf_last && paf_last->type == paf->type)
				continue;
			buf_add(output, "\n");
			paf_last = paf;
			continue;
		}
		show_loc_affect(ch, output, paf, &paf_last);
		show_bit_affect(output, paf, &paf_last);
	}

	if (ch->level < 20)
		return;

	for (obj = ch->carrying; obj; obj = obj->next_content)
		if (obj->wear_loc != WEAR_NONE) {
			if (!IS_OBJ_STAT(obj, ITEM_ENCHANTED))
				show_obj_affects(ch, output,
						 obj->pObjIndex->affected);
			show_obj_affects(ch, output, obj->affected);
		}
}

void fwrite_affect(AFFECT_DATA *paf, FILE *fp)
{
	if (IS_SKILL(paf->type, "doppelganger"))
		return;

	switch (paf->where) {
	case TO_SKILLS:
	case TO_RACE:
		fprintf(fp, "'%s' %3d %3d %3d %3d '%s' %s\n",
			paf->type,
			paf->where, paf->level, paf->duration, paf->modifier,
			STR(paf->location), format_flags(paf->bitvector));
		break;
	default:
		fprintf(fp, "'%s' %3d %3d %3d %3d %3d %s\n",
			paf->type,
			paf->where, paf->level, paf->duration, paf->modifier,
			INT(paf->location), format_flags(paf->bitvector));
		break;
	}
}

AFFECT_DATA *fread_affect(rfile_t *fp)
{
	AFFECT_DATA *paf = aff_new();

	paf->type = fread_strkey(fp, &skills, "fread_affect");
	paf->where = fread_number(fp);
	paf->level = fread_number(fp);
	paf->duration = fread_number(fp);
	paf->modifier = fread_number(fp);
	switch (paf->where) {
	case TO_SKILLS:
		paf->location.s = fread_strkey(fp, &skills, "fread_affect");
		break;
	case TO_RACE:
		paf->location.s = fread_strkey(fp, &races, "fread_affect");
		break;
	default:
		INT(paf->location) = fread_number(fp);
		break;
	}
	paf->bitvector = fread_flags(fp);

	return paf;
}

