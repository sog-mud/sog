/*
 * $Id: affects.c,v 1.77 2001-12-03 22:28:39 fjoe Exp $
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
#include <string.h>

#include <merc.h>
#include <lang.h>

#include <sog.h>

static bool show_name(CHAR_DATA *ch, BUFFER *output,
		      AFFECT_DATA *paf, AFFECT_DATA *paf_last);
static void show_duration(BUFFER *output, AFFECT_DATA *paf);
static void show_loc_affect(CHAR_DATA *ch, BUFFER *output,
			    AFFECT_DATA *paf, AFFECT_DATA **ppaf);
static void show_bit_affect(CHAR_DATA *ch, BUFFER *output,
		            AFFECT_DATA *paf, AFFECT_DATA **ppaf);
static void show_obj_affects(BUFFER *output, AFFECT_DATA *paf);
static void strip_race_and_form_affects(CHAR_DATA *ch);
static void reset_affects(CHAR_DATA *ch);

/* enchanted stuff for eq */
void
affect_enchant(OBJ_DATA *obj)
{
	/* okay, move all the old flags into new vectors if we have to */
	if (!IS_OBJ_STAT(obj, ITEM_ENCHANTED)) {
		AFFECT_DATA *paf;
		SET_OBJ_STAT(obj, ITEM_ENCHANTED);

		for (paf = obj->pObjIndex->affected; paf != NULL; paf = paf->next) {
			AFFECT_DATA *af_new;
			af_new = aff_dup(paf);
			af_new->next = obj->affected;
			obj->affected = af_new;
		}
	}
}

/*
 * Apply or remove an affect to a character.
 */
void
affect_modify(CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd)
{
	OBJ_DATA *wield;
	int mod, i;

	mod = paf->modifier;
	if (!fAdd)
		mod = 0 - mod;

	if (paf->where == TO_SKILLS) {
		if (fAdd) {
			saff_t *sa = varr_enew(&ch->sk_affected);
			sa->sn = str_qdup(paf->location.s);
			sa->type = str_qdup(paf->type);
			sa->mod = paf->modifier;
			sa->bit =  paf->bitvector;
		} else {
			saff_t *sa = NULL;

			do {
				VARR_EFOREACH(sa, sa, &ch->sk_affected) {
					if (!IS_SKILL(sa->sn, paf->location.s)
					||  !IS_SKILL(sa->type, paf->type)
					||  sa->mod != paf->modifier
					||  sa->bit != paf->bitvector)
						continue;

					varr_edelete(&ch->sk_affected, sa);

					/*
					 * restart from this place
					 */
					break;
				}
			} while (sa != NULL);
		}
		return;
	} else if (paf->where == TO_RACE) {
		free_string(ch->race);
		ch->race = str_qdup(fAdd ? paf->location.s : ORG_RACE(ch));
		reset_affects(ch);
		if (!IS_NPC(ch))
			spec_update(ch);
		return;
	} else if (IS_RESIST_AFFECT(paf)) {
		int res = INT(paf->location);

		if (res < 0)
			log(LOG_BUG, "affect_modify: res %d < 0", res);
		else if (res >= MAX_RESIST) {
			log(LOG_BUG, "affect_modify: res %d >= MAX_RESIST",
			    res);
		} else {
			if (ch->shapeform && paf->where == TO_FORMRESISTS)
				ch->shapeform->res_mod[res] += mod;
			ch->res_mod[res] += mod;
		}
		return;
	}

	if (fAdd) {
		switch (paf->where) {
		case TO_AFFECTS:
		case TO_FORMAFFECTS:
			SET_BIT(ch->affected_by, paf->bitvector);
			break;
		case TO_DETECTS:
			SET_DETECT(ch, paf->bitvector);
			break;
		case TO_INVIS:
			SET_INVIS(ch, paf->bitvector);
			break;
		case TO_FORM:
			shapeshift(ch, paf->location.s);
			reset_affects(ch);
			return;
		}
	} else {
		switch (paf->where) {
		case TO_AFFECTS:
		case TO_FORMAFFECTS:
			REMOVE_BIT(ch->affected_by, paf->bitvector);
			break;
		case TO_DETECTS:
			REMOVE_DETECT(ch, paf->bitvector);
			break;
		case TO_INVIS:
			REMOVE_INVIS(ch, paf->bitvector);
			break;
		case TO_FORM:
			revert(ch);
			reset_affects(ch);
			return;
		}
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

	case APPLY_HITROLL:
				if ((paf->where == TO_FORMAFFECTS)
				&& ch->shapeform)
					ch->shapeform->hitroll += mod;
				ch->hitroll		+= mod;	break;
	case APPLY_DAMROLL:
				if ((paf->where == TO_FORMAFFECTS)
				&& ch->shapeform)
					ch->shapeform->damroll += mod;
				ch->damroll		+= mod;	break;
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
	case APPLY_LUCK:		ch->luck_mod		+= mod; break;

	default:
		if (IS_NPC(ch)) {
			log(LOG_INFO, "affect_modify: vnum %d: in room %d: unknown location %d, where: %d",
				   ch->pMobIndex->vnum,
				   ch->in_room ? ch->in_room->vnum : -1,
				   INT(paf->location), paf->where);
		} else {
			log(LOG_INFO, "affect_modify: %s: unknown location %d where: %d",
			    ch->name, INT(paf->location), paf->where);
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
	&&  get_obj_weight(wield) > str_app[get_curr_stat(ch,STAT_STR)].wield) {
		static int depth;

		if (depth == 0) {
		    depth++;
		    act("You drop $p.", ch, wield, NULL, TO_CHAR);
		    act("$n drops $p.", ch, wield, NULL, TO_ROOM);
		    obj_to_room(wield, ch->in_room);
		    depth--;
		}
	}
}

/* find an affect in an affect list */
AFFECT_DATA  *
affect_find(AFFECT_DATA *paf, const char *sn)
{
	STRKEY_CHECK(&skills, sn);

	for (; paf != NULL; paf = paf->next) {
		if (IS_SKILL(paf->type, sn))
			return paf;
	}

	return NULL;
}

void
affect_check_list(CHAR_DATA *ch, AFFECT_DATA *paf, int where, flag_t vector)
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
void
affect_check(CHAR_DATA *ch, int where, flag_t vector)
{
	OBJ_DATA *obj;

	if (where == TO_OBJECT
	||  where == TO_WEAPON
	||  vector == 0)
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
void
affect_to_char(CHAR_DATA *ch, AFFECT_DATA *paf)
{
	AFFECT_DATA *paf_new, *paf2;

	STRKEY_CHECK(&skills, paf->type);

	if (paf->where == TO_RACE || paf->where == TO_FORM)
		strip_race_and_form_affects(ch);

	if (paf->owner != NULL) {
		for (paf2 = ch->affected; paf2 != NULL; paf2 = paf2->next) {
			if (paf2->owner != NULL)
				break;
		}

		if (paf2 == NULL) {
			ch->aff_next = top_affected_char;
			top_affected_char = ch;
		}
	}

	paf_new = aff_dup(paf);
	paf_new->next = ch->affected;
	ch->affected = paf_new;

	affect_modify(ch, paf_new, TRUE);
}

/* give an affect to an object */
void
affect_to_obj(OBJ_DATA *obj, AFFECT_DATA *paf)
{
	AFFECT_DATA *paf_new, *paf2;

	STRKEY_CHECK(&skills, paf->type);

	if (paf->owner != NULL) {
		for (paf2 = obj->affected; paf2 != NULL; paf2 = paf2->next) {
			if (paf2->owner != NULL)
				break;
		}

		if (paf2 == NULL) {
			obj->aff_next = top_affected_obj;
			top_affected_obj = obj;
		}
	}

	paf_new		= aff_dup(paf);
	paf_new->next	= obj->affected;
	obj->affected	= paf_new;

	if (paf->bitvector) {
		switch (paf->where) {
		case TO_OBJECT:
			SET_OBJ_STAT(obj, paf->bitvector);
			break;
		case TO_WEAPON:
			if (obj->item_type == ITEM_WEAPON)
				SET_BIT(INT(obj->value[4]), paf->bitvector);
			break;
		}
	}
}

/*
 * Remove an affect from a char.
 */
void
affect_remove(CHAR_DATA *ch, AFFECT_DATA *paf)
{
	int where;
	int vector;
	AFFECT_DATA *paf2;
	bool hadowner;

	if (ch->affected == NULL) {
		log(LOG_BUG, "affect_remove: no affect");
		return;
	}

	affect_modify(ch, paf, FALSE);
	where = paf->where;
	vector = paf->bitvector;
	hadowner = (paf->owner != NULL);

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
			log(LOG_BUG, "affect_remove: cannot find paf");
			return;
		}
	}

	if (hadowner) {
		for (paf2 = ch->affected; paf2; paf2 = paf2->next)
			if (paf2->owner)
				break;

		if (!paf2) {
			CHAR_DATA *prev;

			if (top_affected_char  == ch)
				top_affected_char = ch->aff_next;
			else {
				for(prev = top_affected_char;
					prev->aff_next && prev->aff_next != ch;
					prev = prev->aff_next);
				if (prev == NULL) {
					log(LOG_BUG, "affect_remove: cannot find char");
					return;
				}
				prev->aff_next = ch->aff_next;
			}
			ch->aff_next = NULL;
		}
	}

	aff_free(paf);
	affect_check(ch, where, vector);
}

void
affect_remove_obj(OBJ_DATA *obj, AFFECT_DATA *paf)
{
	int where, vector;
	AFFECT_DATA *paf2;
	bool hadowner;

	if (obj->affected == NULL)
		return;

	if (obj->carried_by != NULL && obj->wear_loc != -1)
		affect_modify(obj->carried_by, paf, FALSE);

	where = paf->where;
	vector = paf->bitvector;
	hadowner = (paf->owner != NULL);

	/* remove flags from the object if needed */
	if (paf->bitvector) {
		switch(paf->where) {
		case TO_OBJECT:
			REMOVE_OBJ_STAT(obj, paf->bitvector);
			break;
		case TO_WEAPON:
			if (obj->item_type == ITEM_WEAPON)
				REMOVE_BIT(INT(obj->value[4]),
					   paf->bitvector);
			break;
		}
	}

	if (paf == obj->affected)
		obj->affected = paf->next;
	else {
		AFFECT_DATA *prev;

		for (prev = obj->affected; prev != NULL; prev = prev->next) {
			if (prev->next == paf) {
				prev->next = paf->next;
				break;
			}
		}

		if (prev == NULL) {
			log(LOG_BUG, "affect_remove_obj: cannot find paf");
			return;
		}
	}

	if (hadowner) {
		for (paf2 = obj->affected; paf2; paf2 = paf2->next)
			if (paf2->owner)
				break;

		if (!paf2) {
			OBJ_DATA *prev;

			if (top_affected_obj  == obj)
				top_affected_obj = obj->aff_next;
			else {
				for (prev = top_affected_obj;
					prev->aff_next && prev->aff_next != obj;
					prev = prev->aff_next);
				if (prev == NULL) {
					log(LOG_BUG, "affect_remove_obj: cannot find obj");
					return;
				}
				prev->aff_next = obj->aff_next;
			}
			obj->aff_next = NULL;
		}
	}

	aff_free(paf);

	if (obj->carried_by != NULL && obj->wear_loc != -1)
		affect_check(obj->carried_by, where, vector);
}

/*
 * Strip all affects of a given sn.
 */
void
affect_strip(CHAR_DATA *ch, const char *sn)
{
	AFFECT_DATA *paf;
	AFFECT_DATA *paf_next;

	STRKEY_CHECK(&skills, sn);

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
void
affect_bit_strip(CHAR_DATA *ch, int where, flag_t bits)
{
	AFFECT_DATA *paf;
	AFFECT_DATA *paf_next;

	for (paf = ch->affected; paf; paf = paf_next) {
		paf_next = paf->next;
		if (paf->where == where && (paf->bitvector & bits))
			affect_remove(ch, paf);
	}
}

bool
is_sn_affected(CHAR_DATA *ch, const char *sn)
{
	return affect_find(ch->affected, sn) != NULL;
}

AFFECT_DATA *
is_bit_affected(CHAR_DATA *ch, int where, flag_t bits)
{
	AFFECT_DATA *paf;

	for (paf = ch->affected; paf; paf = paf->next)
		if (paf->where == where && (paf->bitvector & bits))
			return paf;

	return NULL;
}

bool
has_obj_affect(CHAR_DATA *ch, flag_t vector)
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
void
affect_join(CHAR_DATA *ch, AFFECT_DATA *paf)
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

/*----------------------------------------------------------------------------
 * room affects stuff
 */

/*
 * Apply or remove an affect to a room.
 */
void
affect_modify_room(ROOM_INDEX_DATA *room, AFFECT_DATA *paf, bool fAdd)
{
	int mod;

	if (fAdd) {
		switch (paf->where) {
		case TO_ROOM_AFFECTS:
			SET_BIT(room->affected_by, paf->bitvector);
			break;
		}
		mod = paf->modifier;
	} else {
		switch (paf->where) {
		case TO_ROOM_AFFECTS:
			REMOVE_BIT(room->affected_by, paf->bitvector);
			break;
		}
		mod = -paf->modifier;
	}

	switch (INT(paf->location)) {
	default:
		log(LOG_BUG, "affect_modify_room: unknown location %d",
		    INT(paf->location));
		break;
	case APPLY_ROOM_NONE:
		break;
	case APPLY_ROOM_HEAL:
		room->heal_rate_mod += mod;
		break;
	case APPLY_ROOM_MANA:
		room->mana_rate_mod += mod;
		break;
	}
}

/*
 * Give an affect to a room.
 */
void
affect_to_room(ROOM_INDEX_DATA *room, AFFECT_DATA *paf)
{
	AFFECT_DATA *paf_new;

	if (paf->owner == NULL) {
		log(LOG_BUG, "affect_to_room: NULL owner");
		return;
	}

	paf_new		= aff_dup(paf);
	paf_new->next	= room->affected;
	room->affected	= paf_new;
	affect_modify_room(room, paf_new, TRUE);

	x_room_add(room);
}

static void
affect_check_room(ROOM_INDEX_DATA *room, int where)
{
	AFFECT_DATA *paf;

	for (paf = room->affected; paf != NULL; paf = paf->next) {
		if (paf->where == where) {
			switch (where) {
			case TO_ROOM_AFFECTS:
				SET_BIT(room->affected_by, paf->where);
				break;
			}
		}
	}
}

/*
 * Remove an affect from a room.
 */
void
affect_remove_room(ROOM_INDEX_DATA *room, AFFECT_DATA *paf)
{
	AFFECT_DATA *af;
	AFFECT_DATA *af_prev = NULL;

/* remove `paf' from this room affects */
	for (af = room->affected; af != NULL; af = af->next) {
		if (af == paf)
			break;
		af_prev = af;
	}

	if (af == NULL) {
		log(LOG_BUG, "affect_remove_room: cannot find paf");
		return;
	}

	if (af_prev == NULL)
		room->affected = paf->next;
	else
		af_prev->next = paf->next;

	affect_modify_room(room, paf, FALSE);
	affect_check_room(room, paf->where);
	aff_free(paf);

	x_room_del(room);
}

/*
 * Strip all affects of a given sn.
 */
void
affect_strip_room(ROOM_INDEX_DATA *room, const char *sn)
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
bool
is_sn_affected_room(ROOM_INDEX_DATA *room, const char *sn)
{
	AFFECT_DATA *paf;

	for (paf = room->affected; paf != NULL; paf = paf->next) {
		if (IS_SKILL(paf->type, sn))
			return TRUE;
	}

	return FALSE;
}

typedef void (*aff_remove_t)(void *, AFFECT_DATA *);

static void
strip_raff_list(AFFECT_DATA *af, aff_remove_t aff_remove,
		CHAR_DATA *ch, void *vo)
{
	while (af != NULL) {
		AFFECT_DATA *af_next = af->next;

		if (af->owner == ch)
			aff_remove(vo, af);

		af = af_next;
	}
}

void
strip_raff_owner(CHAR_DATA *ch)
{
	ROOM_INDEX_DATA *room;
	CHAR_DATA *rch, *rch_next;
	OBJ_DATA *obj, *obj_next;

	for (room = x_room_list; room != NULL; room = room->x_next) {
		strip_raff_list(
		    room->affected, (aff_remove_t) affect_remove_room,
		    ch, room);
	}

	for (rch = top_affected_char; rch != NULL; rch = rch_next) {
		rch_next = rch->aff_next;

		strip_raff_list(
		    rch->affected, (aff_remove_t) affect_remove,
		    ch, rch);
	}

	for (obj = top_affected_obj; obj != NULL; obj = obj_next) {
		obj_next = obj->aff_next;

		strip_raff_list(
		    obj->affected, (aff_remove_t) affect_remove_obj,
		    ch, obj);
	}
}
/*
 * Strip all affects of a given bitvector
 */
void
affect_bit_strip_room(ROOM_INDEX_DATA *room, flag_t bits)
{
	AFFECT_DATA *paf;
	AFFECT_DATA *paf_next;

	for (paf = room->affected; paf != NULL; paf = paf_next) {
		paf_next = paf->next;
		if (paf->where == TO_ROOM_AFFECTS && (paf->bitvector & bits))
			affect_remove_room(room, paf);
	}
}

void
show_affects(CHAR_DATA *ch, CHAR_DATA *vch, BUFFER *output)
{
	OBJ_DATA *obj;
	AFFECT_DATA *paf, *paf_last = NULL;
	bool found = FALSE;

	for (paf = vch->affected; paf; paf = paf->next) {
		if (!found) {
			if (ch == vch)
				buf_append(output, "You are affected by:\n");
			else {
				buf_act(output, BUF_END, "$N is affected by:",
					ch, NULL, vch, 0);
			}

			found = TRUE;
		}

		if (ch->level < MAX_LEVEL / 3) {
			show_name(ch, output, paf, paf_last);
			if (paf_last && paf_last->type == paf->type)
				continue;
			buf_append(output, "\n");
			paf_last = paf;
			continue;
		}
		show_loc_affect(ch, output, paf, &paf_last);
		show_bit_affect(ch, output, paf, &paf_last);
	}

	if (!found) {
		if (ch == vch) {
			buf_append(output,
			    "You are not affected by any spells.\n");
		} else {
			buf_act(output, BUF_END,
				"$N is not affected by any spells.",
				ch, NULL, vch, 0);
		}
	}

	if (ch->level < MAX_LEVEL / 3)
		return;

	for (obj = vch->carrying; obj; obj = obj->next_content) {
		if (obj->wear_loc != WEAR_NONE) {
			if (!IS_OBJ_STAT(obj, ITEM_ENCHANTED)) {
				show_obj_affects(output,
						 obj->pObjIndex->affected);
			}
			show_obj_affects(output, obj->affected);
		}
	}
}

void
aff_dump_list(AFFECT_DATA *paf, BUFFER *output)
{
	int cnt = 0;

	for (; paf != NULL; paf = paf->next) {
		where_t *w = where_lookup(paf->where);

		if (cnt == 0) {
			buf_append(output, "Number Skill     Affects Applies to Modifier Bitvector\n");					// notrans
			buf_append(output, "------ --------- ------- ---------- -------- ----------------------------------\n");		// notrans
		}
		buf_printf(output, BUF_END,
		    "[%4d] %-9.9s %-7.7s %-10.10s %8d %s\n",	// notrans
		    cnt, paf->type,
		    flag_string(affect_where_types, paf->where),
		    HAS_INT_LOCATION(paf) ?
			(w != NULL && w->loc_table) ?
				SFLAGS(w->loc_table, paf->location) : "none" :
			STR(paf->location),
		    paf->modifier,
		    (w != NULL && w->bit_table) ?
			flag_string(w->bit_table, paf->bitvector) : "none");
		cnt++;
	}
}

void
format_obj_affects(BUFFER *output, AFFECT_DATA *paf, int flags)
{
	for (; paf; paf = paf->next) {
		where_t *w;

		if ((w = where_lookup(paf->where)) == NULL)
			continue;

		if (!IS_NULLSTR(w->loc_format)
		&&  (IS_RESIST_AFFECT(paf) ||
		     INT(paf->location) != APPLY_NONE)
		&&   paf->modifier) {
			buf_printf(output, BUF_END, w->loc_format,
			    HAS_INT_LOCATION(paf) ?
				SFLAGS(w->loc_table, paf->location) :
				STR(paf->location),
			    paf->modifier);
			if (!IS_SET(flags, FOA_F_NODURATION)
			&&  paf->duration > -1)
				buf_printf(output, BUF_END, " for %d hours",
					   paf->duration);
			buf_append(output, ".\n");
		}

		if (IS_SET(flags, FOA_F_NOAFFECTS))
			continue;

		if (paf->bitvector
		&& !IS_NULLSTR(w->bit_format)) {
			buf_printf(output, BUF_END, w->bit_format,
					flag_string(w->bit_table, paf->bitvector));
			if (!IS_SET(flags, FOA_F_NODURATION)
			&&  paf->duration > -1)
				buf_printf(output, BUF_END, " for %d hours",
					   paf->duration);
			buf_append(output, ".\n");
		}
	}
}

/*----------------------------------------------------------------------------
 * show affects stuff - local functions
 */

static bool
show_name(CHAR_DATA *ch, BUFFER *output,
	  AFFECT_DATA *paf, AFFECT_DATA *paf_last)
{
	skill_t *aff;
	const char *aff_type;

	if (ch != NULL
	&&  (ch->level < MAX_LEVEL / 3 || IS_SET(ch->comm, COMM_SHORT_AFF))
	&&  paf_last != NULL
	&&  IS_SKILL(paf->type, paf_last->type))
		return FALSE;

	if (IS_NULLSTR(paf->type))
		aff_type = "Item:";
	else if ((aff = skill_lookup(paf->type)) != NULL) {
		switch(aff->skill_type) {
		case ST_SPELL:
			aff_type = "Spell:";
			break;
		case ST_SKILL:
			aff_type = "Skill:";
			break;
		case ST_PRAYER:
			aff_type = "Prayer:";
			break;
		default:
			aff_type = "???:";			// notrans
		}
	} else
		aff_type = "???:";				// notrans

	if (paf_last && IS_SKILL(paf->type, paf_last->type)) {
		buf_append(output, "                        ");
	} else {
		buf_printf(output, BUF_END, "%-7s {c%-16s{x",
			   GETMSG(aff_type, buf_lang(output)), paf->type);
	}

	return TRUE;
}

static void
show_duration(BUFFER *output, AFFECT_DATA *paf)
{
	if (paf->duration < 0)
		buf_append(output, " permanently.\n");
	else  {
		buf_act(output, BUF_END, " for {c$j{x $qj{hours}.", NULL,
			(const void *) paf->duration, NULL, 0);
	}
}

static void
show_loc_affect(CHAR_DATA *ch, BUFFER *output,
		AFFECT_DATA *paf, AFFECT_DATA **ppaf)
{
	where_t *w;

	if ((w = where_lookup(paf->where)) == NULL
	||  IS_NULLSTR(w->loc_format))
		return;

	if ((INT(paf->location) == APPLY_NONE || paf->modifier == 0)
	&&  paf->bitvector)
		return;

	if (!show_name(ch, output, paf, *ppaf))
		return;

	if (IS_SET(ch->comm, COMM_SHORT_AFF))
		buf_append(output, ":");			// notrans
	else {
		buf_append(output, ": ");			// notrans
		buf_printf(output, BUF_END, w->loc_format,
		   w->loc_table ?
			SFLAGS(w->loc_table, paf->location) :
			STR(paf->location),
		   paf->modifier);
	}
	show_duration(output, paf);
	*ppaf = paf;
}

static void
show_bit_affect(CHAR_DATA *ch, BUFFER *output,
		AFFECT_DATA *paf, AFFECT_DATA **ppaf)
{
	where_t *w;

	if ((w = where_lookup(paf->where)) == NULL
	||  !paf->bitvector
	||  IS_NULLSTR(w->bit_format))
		return;

	if (!show_name(NULL, output, paf, *ppaf))
		return;

	if (ch != NULL && IS_SET(ch->comm, COMM_SHORT_AFF))
		buf_append(output, ":");			// notrans
	else {
		buf_append(output, ": ");			// notrans
		buf_printf(output, BUF_END, w->bit_format,
			flag_string(w->bit_table, paf->bitvector));
	}
	show_duration(output, paf);
	*ppaf = paf;
}

static void
show_obj_affects(BUFFER *output, AFFECT_DATA *paf)
{
	AFFECT_DATA *paf_last = NULL;

	for (; paf; paf = paf->next)
		show_bit_affect(NULL, output, paf, &paf_last);
}

static void
strip_race_and_form_affects(CHAR_DATA *ch)
{
	AFFECT_DATA *paf;

	for (; ;) {
		for (paf = ch->affected; paf; paf = paf->next) {
			if (paf->where == TO_RACE || paf->where == TO_FORM)
				break;
		}

		if (paf != NULL)
			affect_strip(ch, paf->type);
		else
			break;
	}
}

static void
reset_affects(CHAR_DATA *ch)
{
	if (ch->shapeform) {
		ch->affected_by	= ch->shapeform->index->affected_by;
		ch->has_invis	= ch->shapeform->index->has_invis;
		ch->has_detect	= ch->shapeform->index->has_detect;
	} else
		race_resetstats(ch);

	affect_check(ch, -1, -1);
}
