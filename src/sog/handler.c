/*
 * $Id: handler.c,v 1.286 2001-06-21 16:16:58 avn Exp $
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

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "merc.h"
#include "obj_prog.h"
#include "db.h"
#include "lang.h"
#include "mob_prog.h"
#include "auction.h"

#include "fight.h"
#include "magic.h"
#include "update.h"
#include "quest.h"

static const char *	format_hmv	(int hp, int mana, int move);

static int		max_hit_gain	(CHAR_DATA *ch, class_t *cl);
static int		min_hit_gain	(CHAR_DATA *ch, class_t *cl);
static int		max_mana_gain	(CHAR_DATA *ch, class_t *cl);
static int		min_mana_gain	(CHAR_DATA *ch, class_t *cl);
static int		max_move_gain	(CHAR_DATA *ch);
static int		min_move_gain	(CHAR_DATA *ch);

static OBJ_DATA *get_obj_list_raw(CHAR_DATA *ch, const char *name, uint *number,
			   OBJ_DATA *list, int flags);
static OBJ_DATA *get_obj_here_raw(CHAR_DATA *ch, const char *name,
				  uint *number);
static CHAR_DATA *get_char_room_raw(CHAR_DATA *ch, const char *name,
				    uint *number, ROOM_INDEX_DATA *room);
static void strip_obj_affects(CHAR_DATA *ch, AFFECT_DATA *paf);
#if 0
static OBJ_DATA *get_stuck_eq(CHAR_DATA *ch, int wtype);
#endif

/*
 * Room record:
 * For less than 5 people in room create a new record.
 * Else use the oldest one.
 */
void room_record(const char *name, ROOM_INDEX_DATA *room, int door)
{
	ROOM_HISTORY_DATA *rec;
	ROOM_HISTORY_DATA *prev = NULL;
	int i = 0;

	if (room->history) {
		for (rec = room->history; rec->next != NULL; rec = rec->next) {
			i++;
			prev = rec;
		}
	}

	if (prev == NULL || i < 4) 
		rec = malloc(sizeof(*rec)); 
	else { 
		rec = prev->next;
		prev->next = NULL;
		free_string(rec->name);
	}

	rec->next = room->history;
	room->history = rec; 

	rec->name = str_dup(name);
	rec->went = door;
}

/* returns number of people on an object */
int count_users(OBJ_DATA *obj)
{
	CHAR_DATA *fch;
	int count = 0;

	if (obj->in_room == NULL)
		return 0;

	for (fch = obj->in_room->people; fch != NULL; fch = fch->next_in_room) {
		if (fch->on == obj)
			count++;
	}

	return count;
}

int age_to_num(int age)
{
	return  age * 72000;
}

DECLARE_SPEC_FUN(spec_janitor);

/*
 * Retrieve a character's carry capacity.
 */
int can_carry_n(CHAR_DATA *ch)
{
	if (IS_IMMORTAL(ch))
		return -1;

	if (IS_NPC(ch)) {
		if (IS_SET(ch->pMobIndex->act, ACT_PET))
			return 0;
		if (ch->pMobIndex->spec_fun == spec_janitor)
			return -1;
	}

	return MAX_WEAR + get_curr_stat(ch,STAT_DEX) - 10 + ch->size;
}

/*
 * Retrieve a character's carry capacity.
 */
int can_carry_w(CHAR_DATA *ch)
{
	if (IS_IMMORTAL(ch))
		return -1;

	if (IS_NPC(ch)) {
		if (IS_SET(ch->pMobIndex->act, ACT_PET))
			return 0;
		if (ch->pMobIndex->spec_fun == spec_janitor)
			return -1;
		if (MOB_IS(ch, MOB_CHANGER))
			return -1;
	}

	return (str_app[get_curr_stat(ch,STAT_STR)].carry + ch->level * 25) * 3
	    / 2;
}

/*
 * Move a char out of a room.
 */
void char_from_room(CHAR_DATA *ch)
{
	OBJ_DATA *obj;
	CHAR_DATA *prev = NULL;
	CHAR_DATA *vch;

	if (ch->in_room == NULL) {
		log(LOG_BUG, "char_from_room: NULL");
		return;
	}

	if (ch->in_room->affected)
		  check_events(ch, ch->in_room->affected, EVENT_ROOM_LEAVE);

	if (!IS_NPC(ch))
		--ch->in_room->area->nplayer;

	if ((obj = get_eq_char(ch, WEAR_LIGHT)) != NULL
	&&   obj->item_type == ITEM_LIGHT
	&&   INT(obj->value[2]) != 0
	&&   ch->in_room->light > 0)
		--ch->in_room->light;

	for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room) {
		if (vch == ch)
			break;
		prev = vch;
	}

	if (vch == NULL)
		log(LOG_BUG, "char_from_room: ch not found");
	else if (prev == NULL)
		ch->in_room->people = ch->next_in_room;
	else 
		prev->next_in_room = ch->next_in_room;

	ch->in_room = NULL;
	ch->next_in_room = NULL;
	ch->on = NULL;  /* sanity check! */

	if (ch->mount != NULL) {
		ch->mount->riding = FALSE;
		ch->riding = FALSE;
	}
}

/*
 * Move a char into a room.
 */
void char_to_room(CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex)
{
	olced_t *olced;
	OBJ_DATA *obj;

	if (pRoomIndex == NULL) {
		ROOM_INDEX_DATA *room;

		log(LOG_BUG, "char_to_room: NULL");
		
		if ((room = get_room_index(ROOM_VNUM_TEMPLE)) != NULL)
			char_to_room(ch, room);
		
		return;
	}

	ch->in_room		= pRoomIndex;
	ch->next_in_room	= pRoomIndex->people;
	pRoomIndex->people	= ch;

	if (!IS_NPC(ch)) {
		if (ch->in_room->area->empty) {
			ch->in_room->area->empty = FALSE;
			ch->in_room->area->age = 0;
		}
		++ch->in_room->area->nplayer;
	}

	if ((obj = get_eq_char(ch, WEAR_LIGHT)) != NULL
	&&   obj->item_type == ITEM_LIGHT
	&&   INT(obj->value[2]) != 0)
		++ch->in_room->light;

	if (pRoomIndex->affected) {
		if (IS_IMMORTAL(ch))
			dofun("raffects", ch, str_empty);
		else {
			check_events(ch, ch->in_room->affected,
				EVENT_ROOM_ENTER);
			if (IS_EXTRACTED(ch))
				return;
		}
	}

	if (ch->desc != NULL
	&&  (olced = OLCED(ch)) != NULL
	&&  !str_cmp(olced->id, "rooms"))
		dofun("edit", ch, "rooms dropout");		// notrans
}

/*
 * Give an obj to a char.
 */
void obj_to_char(OBJ_DATA *obj, CHAR_DATA *ch)
{
	obj->next_content	= ch->carrying;
	ch->carrying		= obj;
	obj->carried_by		= ch;
	obj->in_room		= NULL;
	obj->in_obj		= NULL;

	if (obj->last_owner && !IS_NPC(ch) && obj->last_owner != ch) {
		name_add(&PC(obj->last_owner)->enemy_list, ch->name, NULL,NULL);
		PC(ch)->last_offence = current_time;
	}

	if (!IS_NPC(ch)) {
		obj->last_owner		= ch;
	}
	ch->carry_number	+= get_obj_number(obj);
	ch->carry_weight	+= get_obj_weight(obj);
}

/*
 * Take an obj from its character.
 */
void obj_from_char(OBJ_DATA *obj)
{
	CHAR_DATA *ch;

	if ((ch = obj->carried_by) == NULL) {
		log(LOG_BUG, "obj_from_char: null ch (obj->name  = '%s')",
		    obj->pObjIndex->name);
		return;
	}

	if (obj->wear_loc != WEAR_NONE)
		unequip_char(ch, obj);

	if (ch->carrying == obj)
		ch->carrying = obj->next_content;
	else {
		OBJ_DATA *prev;

		for (prev = ch->carrying; prev; prev = prev->next_content) {
			if (prev->next_content == obj)
				break;
		}

		if (prev == NULL)
			log(LOG_BUG, "obj_from_char: obj not in list");
		else
			prev->next_content = obj->next_content;
	}

	obj->carried_by		= NULL;
	obj->next_content	= NULL;
	ch->carry_number	-= get_obj_number(obj);
	ch->carry_weight	-= get_obj_weight(obj);
}

/*
 * Find the ac value of an obj, including position effect.
 */
int apply_ac(OBJ_DATA *obj, int iWear, int type)
{
	if (obj->item_type != ITEM_ARMOR)
		return 0;

	switch (iWear) {
	case WEAR_BODY:		return 3 * INT(obj->value[type]);
	case WEAR_HEAD:		return 2 * INT(obj->value[type]);
	case WEAR_LEGS:		return 2 * INT(obj->value[type]);
	case WEAR_FEET:		return	INT(obj->value[type]);
	case WEAR_HANDS:	return	INT(obj->value[type]);
	case WEAR_ARMS:		return	INT(obj->value[type]);
	case WEAR_SHIELD:	return	INT(obj->value[type]);
	case WEAR_FINGER_L:	return	INT(obj->value[type]) / 2;
	case WEAR_FINGER_R:	return	INT(obj->value[type]) / 2;
	case WEAR_NECK:		return	INT(obj->value[type]);
	case WEAR_FACE:		return	INT(obj->value[type]);
	case WEAR_ABOUT:	return 2 * INT(obj->value[type]);
	case WEAR_WAIST:	return	INT(obj->value[type]);
	case WEAR_WRIST_L:	return	INT(obj->value[type]);
	case WEAR_WRIST_R:	return	INT(obj->value[type]);
	case WEAR_HOLD:		return	INT(obj->value[type]);
	}

	return 0;
}

/* XXX is not used now */
#if 0
/* find stuck-in objects of certain weapon type */
static OBJ_DATA *get_stuck_eq(CHAR_DATA *ch, int wtype)
{
	OBJ_DATA *obj;

	if (!ch)
		return NULL;

	for (obj = ch->carrying; obj; obj = obj->next_content)
		if (obj->wear_loc == WEAR_STUCK_IN
		&&  obj->pObjIndex->item_type == ITEM_WEAPON
		&&  INT(obj->value[0]) == wtype)
			return obj;
	return NULL;
}
#endif

/*
 * Find a piece of eq on a character.
 */
OBJ_DATA *get_eq_char(CHAR_DATA *ch, int iWear)
{
	OBJ_DATA *obj;

	if (ch == NULL)
		return NULL;

	if (ch->shapeform)
		return NULL;

	for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
		if (obj->wear_loc == iWear)
			return obj;

	return NULL;
}

void _equip_char(CHAR_DATA *ch, OBJ_DATA *obj)
{
	int i;
	AFFECT_DATA *paf;

	if (obj->wear_loc == WEAR_STUCK_IN)
		return;

	for (i = 0; i < 4; i++)
		ch->armor[i] -= apply_ac(obj, obj->wear_loc, i);

	if (!IS_OBJ_STAT(obj, ITEM_ENCHANTED))
		for (paf = obj->pObjIndex->affected; paf; paf = paf->next)
			affect_modify(ch, paf, TRUE);

	for (paf = obj->affected; paf; paf = paf->next)
		affect_modify(ch, paf, TRUE);
}

/*
 * Equip a char with an obj. Return obj on success. Otherwise returns NULL.
 */
OBJ_DATA *
equip_char(CHAR_DATA *ch, OBJ_DATA *obj, int iWear)
{
	if (iWear == WEAR_STUCK_IN) {
		obj->wear_loc = iWear;
		return obj;
	}

	if (get_eq_char(ch, iWear)) {
		if (IS_NPC(ch)) {
			log(LOG_BUG, "equip_char: vnum %d: in_room %d: "
			   "obj vnum %d: location %s: "
			   "already equipped.",
			   ch->pMobIndex->vnum,
			   ch->in_room ? ch->in_room->vnum : -1,
			   obj->pObjIndex->vnum,
			   flag_string(wear_loc_flags, iWear));
		} else {
			log(LOG_BUG, "equip_char: %s: location %s: "
			   "already equipped.",
			   ch->name,
			   flag_string(wear_loc_flags, iWear));
		}
		return NULL;
	}

	if ((IS_OBJ_STAT(obj, ITEM_ANTI_EVIL)    && IS_EVIL(ch)   )
	||  (IS_OBJ_STAT(obj, ITEM_ANTI_GOOD)    && IS_GOOD(ch)   )
	||  (IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch))) {
		/*
		 * Thanks to Morgenes for the bug fix here!
		 */
		act("You are zapped by $p and drop it.", ch, obj, NULL, TO_CHAR);
		act("$n is zapped by $p and drops it.",  ch, obj, NULL, TO_ROOM);
		obj_from_char(obj);
		obj_to_room(obj, ch->in_room);
		return NULL;
	}

	obj->wear_loc = iWear;
	_equip_char(ch, obj);

	if (obj->item_type == ITEM_LIGHT
	&&  INT(obj->value[2]) != 0
	&&  ch->in_room != NULL)
		++ch->in_room->light;

	if (oprog_call(OPROG_WEAR, obj, ch, NULL))
		return NULL;
	return obj;
}

static void strip_obj_affects(CHAR_DATA *ch, AFFECT_DATA *paf)
{
	for (; paf != NULL; paf = paf->next) {
		affect_modify(ch, paf, FALSE);
		affect_check(ch, paf->where, paf->bitvector);
	}
}

/*
 * Unequip a char with an obj.
 */
void unequip_char(CHAR_DATA *ch, OBJ_DATA *obj)
{
	int i;

	if (obj->wear_loc == WEAR_NONE) {
		log(LOG_BUG, "unequip_char: already unequipped");
		return;
	}

	if (obj->wear_loc == WEAR_STUCK_IN) {
		obj->wear_loc = WEAR_NONE;
		return;
	}

	for (i = 0; i < 4; i++)
		ch->armor[i] += apply_ac(obj, obj->wear_loc,i);
	obj->wear_loc = WEAR_NONE;

	if (!IS_OBJ_STAT(obj, ITEM_ENCHANTED))
		strip_obj_affects(ch, obj->pObjIndex->affected);
	strip_obj_affects(ch, obj->affected);

	if (obj->item_type == ITEM_LIGHT
	&&  INT(obj->value[2]) != 0
	&&  ch->in_room != NULL
	&&  ch->in_room->light > 0)
		--ch->in_room->light;

	oprog_call(OPROG_REMOVE, obj, ch, NULL);

	if ((obj = get_eq_char(ch, WEAR_SECOND_WIELD)) != NULL) {
		act_puts("You wield your second weapon as your first!",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		act("$n wields $s second weapon as first!",
		    ch, NULL, NULL, TO_ROOM);
		unequip_char(ch, obj);
		equip_char(ch, obj, WEAR_WIELD);
	}
}

/*
 * Count occurrences of an obj in a list.
 */
int count_obj_list(OBJ_INDEX_DATA *pObjIndex, OBJ_DATA *list)
{
	OBJ_DATA *obj;
	int nMatch;

	nMatch = 0;
	for (obj = list; obj != NULL; obj = obj->next_content) {
		if (obj->pObjIndex == pObjIndex)
		    nMatch++;
	}

	return nMatch;
}

/*
 * Move an obj out of a room.
 */
void obj_from_room(OBJ_DATA *obj)
{
	ROOM_INDEX_DATA *in_room;
	CHAR_DATA *ch;

	if ((in_room = obj->in_room) == NULL) {
		log(LOG_INFO, "obj_from_room: NULL obj->in_room (vnum %d)",
			   obj->pObjIndex->vnum);
		return;
	}

	for (ch = in_room->people; ch != NULL; ch = ch->next_in_room) {
		if (ch->on == obj)
			ch->on = NULL;
	}

	if (obj == in_room->contents)
		in_room->contents = obj->next_content;
	else {
		OBJ_DATA *prev;

		for (prev = in_room->contents; prev; prev = prev->next_content) {
			if (prev->next_content == obj)
				break;
		}

		if (prev == NULL)
			log(LOG_BUG, "obj_from_room: obj not found");
		else
			prev->next_content = obj->next_content;
	}

	obj->in_room      = NULL;
	obj->next_content = NULL;
	return;
}

/*
 * Move an obj into a room.
 */
void obj_to_room(OBJ_DATA *obj, ROOM_INDEX_DATA *pRoomIndex)
{
	obj->next_content	= pRoomIndex->contents;
	pRoomIndex->contents	= obj;
	obj->in_room		= pRoomIndex;
	obj->carried_by		= NULL;
	obj->in_obj		= NULL;

	if (IS_WATER(pRoomIndex))
		obj->water_float = floating_time(obj);
}

/*
 * Move an object into an object.
 */
void obj_to_obj(OBJ_DATA *obj, OBJ_DATA *obj_to)
{
	if (obj == obj_to) {
		log(LOG_INFO, "obj_to_obj: obj == obj_to (vnum %d)",
			   obj->pObjIndex->vnum);
		return;
	}

	obj->next_content	= obj_to->contains;
	obj_to->contains	= obj;
	obj->in_obj		= obj_to;
	obj->in_room		= NULL;
	obj->carried_by		= NULL;
	if (OBJ_IS(obj_to, OBJ_PIT))
		obj->cost = 0; 

	for (; obj_to != NULL; obj_to = obj_to->in_obj) {
		if (obj_to->carried_by != NULL)
		{
/*	    obj_to->carried_by->carry_number += get_obj_number(obj); */
		    obj_to->carried_by->carry_weight += get_obj_weight(obj)
			* WEIGHT_MULT(obj_to) / 100;
		}
	}
}

/*
 * Move an object out of an object.
 */
void obj_from_obj(OBJ_DATA *obj)
{
	OBJ_DATA *obj_from;

	if ((obj_from = obj->in_obj) == NULL) {
		log(LOG_BUG, "obj_from_obj: null obj_from");
		return;
	}

	if (obj == obj_from->contains)
		obj_from->contains = obj->next_content;
	else {
		OBJ_DATA *prev;

		for (prev = obj_from->contains; prev; prev = prev->next_content) {
			if (prev->next_content == obj) 
				break;
		}

		if (prev == NULL)
			log(LOG_BUG, "obj_from_obj: obj not found");
		else
			prev->next_content = obj->next_content;
	}

	obj->next_content = NULL;
	obj->in_obj       = NULL;

	for (; obj_from != NULL; obj_from = obj_from->in_obj) {
		if (obj_from->carried_by != NULL)
/*	    obj_from->carried_by->carry_number -= get_obj_number(obj); */
			obj_from->carried_by->carry_weight -= get_obj_weight(obj) * WEIGHT_MULT(obj_from) / 100;
	}
}

/*
 * Extract an obj from the world.
 */
void extract_obj(OBJ_DATA *obj, int flags)
{
	OBJ_DATA *obj_content;
	OBJ_DATA *obj_next;

	if (!mem_is(obj, MT_OBJ)) {
		log(LOG_BUG, "extract_obj: obj is not MT_OBJ");
		return;
	}

	if (OBJ_IS(obj, OBJ_CLAN))
		return;

	if (OBJ_IS(obj, OBJ_CHQUEST)) {
		if (!IS_SET(flags, XO_F_NOCHQUEST))
			chquest_extract(obj);
		flags |= XO_F_NORECURSE;
	}

	for (obj_content = obj->contains; obj_content; obj_content = obj_next) {
		obj_next = obj_content->next_content;

		if (!IS_SET(flags, XO_F_NORECURSE)
		||  IS_SET(flags, XO_F_NUKE)) {
			extract_obj(obj_content, flags);
			continue;
		}

		obj_from_obj(obj_content);
		if (obj->in_room)
			obj_to_room(obj_content, obj->in_room);
		else if (obj->carried_by)
			obj_to_char(obj_content, obj->carried_by);
		else if (obj->in_obj)
			obj_to_obj(obj_content, obj->in_obj);
		else
			extract_obj(obj_content, 0);
	}

	if (!IS_SET(flags, XO_F_NUKE)) {
		if (obj->in_room)
			obj_from_room(obj);
		else if (obj->carried_by)
			obj_from_char(obj);
		else if (obj->in_obj)
			obj_from_obj(obj);
	}

	if (obj->pObjIndex->vnum == OBJ_VNUM_MAGIC_JAR) {
		 CHAR_DATA *wch;
		 
		 for (wch = char_list; wch && !IS_NPC(wch); wch = wch->next) {
		 	if (!mlstr_cmp(&obj->owner, &wch->short_descr)) {
				REMOVE_BIT(PC(wch)->plr_flags, PLR_NOEXP);
				act_char("Now you catch your spirit.", wch);
				break;
			}
		}
	}

	if (object_list == obj)
		object_list = obj->next;
	else {
		OBJ_DATA *prev;

		for (prev = object_list; prev != NULL; prev = prev->next) {
			if (prev->next == obj) {
				prev->next = obj->next;
				break;
			}
		}

		if (prev == NULL) {
			log(LOG_BUG, "extract_obj: obj %d not found.",
			    obj->pObjIndex->vnum);
			return;
		}
	}

	if (obj == top_affected_obj)
		top_affected_obj = obj->aff_next;
	else {
		OBJ_DATA *prev;

		for (prev = top_affected_obj; prev; prev = prev->aff_next)
			if (prev->aff_next == obj)
				break;

		if (prev != NULL)
			prev->aff_next = obj->aff_next;
	}

	if (!IS_SET(flags, XO_F_NOCOUNT))
		--obj->pObjIndex->count;

	/*
	 * untag memory
	 */
	mem_untag(obj, -1);
	free_obj(obj);
}

/*
 * Extract a char from the world.
 */
void extract_char(CHAR_DATA *ch, int flags)
{
	CHAR_DATA *wch;
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;
	OBJ_DATA *wield;
	int extract_obj_flags;

	if (!mem_is(ch, MT_CHAR)) {
		log(LOG_BUG, "extract_char: ch is not MT_CHAR");
		return;
	}
	
	strip_raff_owner(ch);
	if (!IS_NPC(ch))
		nuke_pets(ch);

	if (!IS_SET(flags, XC_F_INCOMPLETE))
		die_follower(ch);
	
	stop_fighting(ch, TRUE);

	if ((wield = get_eq_char(ch, WEAR_WIELD)) != NULL)
		unequip_char(ch, wield); 

	extract_obj_flags = (IS_SET(flags, XC_F_NOCOUNT) ? XO_F_NOCOUNT : 0);
	for (obj = ch->carrying; obj != NULL; obj = obj_next) {
		obj_next = obj->next_content;
		extract_obj(obj, extract_obj_flags);
	}
	
	for (wch = char_list; wch && !IS_NPC(wch); wch = wch->next) {
		if (PC(wch)->reply == ch)
			PC(wch)->reply = NULL;
	}

	/*
	 * untag memory
	 */
	mem_untag(ch, -1);

	if (IS_SET(flags, XC_F_INCOMPLETE)) {
		char_from_room(ch);
		char_to_room(ch, get_altar(ch)->room);
		return;
	}

	if (ch->desc != NULL && ch->desc->original != NULL) {
		dofun("return", ch, str_empty);
		ch->desc = NULL;
	}

	char_from_room(ch);

	if (ch->mount != NULL) {
		ch->mount->mount = NULL;
		ch->mount = NULL;
	}

	if (IS_NPC(ch))
		--ch->pMobIndex->count;

	for (wch = npc_list; wch; wch = wch->next) {
		if (NPC(wch)->mprog_target == ch)
			NPC(wch)->mprog_target = NULL;
	}

	if (ch == char_list) {
		char_list = ch->next;
		if (ch == char_list_lastpc)
			char_list_lastpc = NULL;
	} else {
		CHAR_DATA *prev;

		for (prev = char_list; prev; prev = prev->next) {
			if (prev->next == ch)
				break;
		}

		if (prev == NULL)
			log(LOG_BUG, "Extract_char: char not found");
		else
			prev->next = ch->next;
		if (ch == char_list_lastpc)
			char_list_lastpc = prev;
	}

	if (ch == top_affected_char)
		top_affected_char = ch->aff_next;
	else {
		CHAR_DATA *prev;

		for (prev = top_affected_char; prev; prev = prev->aff_next)
			if (prev->aff_next == ch)
				break;

		if (prev != NULL)
			prev->aff_next = ch->aff_next;
	}

	if (ch->desc)
		ch->desc->character = NULL;

	char_free(ch);
}

/*
 * Find a char in the room.
 */
static CHAR_DATA *get_char_room_raw(CHAR_DATA *ch, const char *name,
				    uint *number, ROOM_INDEX_DATA *room)
{
	CHAR_DATA *rch;
	bool ugly;

	if (!str_cmp(name, "self"))
		return ch;

	ugly = !str_cmp(name, "ugly");
	for (rch = room->people; rch; rch = rch->next_in_room) {
		CHAR_DATA *vch;

		if (!can_see(ch, rch))
			continue;

		if (ugly
		&&  *number == 1
		&&  is_affected(rch, "vampire"))
			return rch;

		vch = (is_affected(rch, "doppelganger") &&
		       (IS_NPC(ch) || !IS_SET(PC(ch)->plr_flags, PLR_HOLYLIGHT))) ?
					rch->doppel : rch;
		if ((!IS_IMMORTAL(ch) || name[0]) && !is_name(name, vch->name))
			continue;

		if (!--(*number))
			return rch;
	}

	return NULL;
}

/*
 * Find a char in the room.
 */
CHAR_DATA *get_char_room(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	uint number;

	number = number_argument(argument, arg, sizeof(arg));
	if (!number)
		return NULL;

	return get_char_room_raw(ch, arg, &number, ch->in_room);
}

CHAR_DATA *get_char_area(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *ach;
	uint number;

	number = number_argument(argument, arg, sizeof(arg));
	if (!number)
		return NULL;

	if ((ach = get_char_room_raw(ch, arg, &number, ch->in_room)))
		return ach;

	if (arg[0] == '\0')
		return NULL;

	for (ach = char_list; ach; ach = ach->next) { 
		if (!ach->in_room
		||  ach->in_room == ch->in_room)
			continue;

		if (ach->in_room->area != ch->in_room->area
		||  !can_see(ch, ach)
		||  !is_name(arg, ach->name))
			continue;

		if (!--number)
			return ach;
	}
	return NULL;
}

/*
 * Find a char in the world.
 */
CHAR_DATA *get_char_world(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *wch;
	uint number;

	number = number_argument(argument, arg, sizeof(arg));
	if (!number)
		return NULL;

	if ((wch = get_char_room_raw(ch, arg, &number, ch->in_room)))
		return wch;

	if (arg[0] == '\0')
		return NULL;

	for (wch = char_list; wch; wch = wch->next) {
		if (!wch->in_room
		||  wch->in_room == ch->in_room
		||  !can_see(ch, wch) 
		||  !is_name(arg, wch->name))
			continue;

		if (!--number)
			return wch;
	}

	return NULL;
}

int opposite_door(int door)
{
	int opdoor;

	switch (door) {
	case 0: opdoor=2;	break;
	case 1: opdoor=3;	break;
	case 2: opdoor=0;	break;
	case 3: opdoor=1;	break;
	case 4: opdoor=5;	break;
	case 5: opdoor=4;	break;
	default: opdoor=-1;	break;
	}

	return opdoor;
}

CHAR_DATA *find_char(CHAR_DATA *ch, const char *argument, int door, int range) 
{
	EXIT_DATA *pExit, *bExit;
	ROOM_INDEX_DATA *dest_room = ch->in_room;
	ROOM_INDEX_DATA *back_room;
	CHAR_DATA *target;
	uint number;
	int opdoor;
	char arg[MAX_INPUT_LENGTH];

	number = number_argument(argument, arg, sizeof(arg));
	if (!number)
		return NULL;

	if ((target = get_char_room_raw(ch, arg, &number, dest_room)))
		return target;

	if ((opdoor = opposite_door(door)) == -1) {
		log(LOG_BUG, "In find_char wrong door: %d", door);
		act_char("You don't see that there.", ch);
		return NULL;
	}

	while (range > 0) {
		range--;

		/* find target room */
		back_room = dest_room;
		if ((pExit = dest_room->exit[door]) == NULL
		||  (dest_room = pExit->to_room.r) == NULL
		||  IS_SET(pExit->exit_info, EX_CLOSED))
			break;

		if ((bExit = dest_room->exit[opdoor]) == NULL
		||  bExit->to_room.r != back_room) {
			act_char("The path you choose prevents your power to pass.", ch);
			return NULL;
		}
		if ((target = get_char_room_raw(ch, arg, &number, dest_room))) 
			return target;
	}

	act_char("You don't see that there.", ch);
	return NULL;
}

int check_exit(const char *arg)
{
	int door = -1;

	     if (!str_cmp(arg, "n") || !str_cmp(arg, "north")) door = 0;
	else if (!str_cmp(arg, "e") || !str_cmp(arg, "east" )) door = 1;
	else if (!str_cmp(arg, "s") || !str_cmp(arg, "south")) door = 2;
	else if (!str_cmp(arg, "w") || !str_cmp(arg, "west" )) door = 3;
	else if (!str_cmp(arg, "u") || !str_cmp(arg, "up"   )) door = 4;
	else if (!str_cmp(arg, "d") || !str_cmp(arg, "down" )) door = 5;

	return door;
}

/*
 * Find a char for range casting.
 * argument must specify target in form '[d.][n.]name' where
 * 'd' - direction
 * 'n' - number
 */
CHAR_DATA *get_char_spell(CHAR_DATA *ch, const char *argument,
			  int *door, int range)
{
	char buf[MAX_INPUT_LENGTH];
	char *p;

	p = strchr(argument, '.');
	if (!p) {
		*door = -1;
		return get_char_room(ch, argument);
	}

	strnzncpy(buf, sizeof(buf), argument, (unsigned)(p-argument));
	if ((*door = check_exit(buf)) < 0)
		return get_char_room(ch, argument);

	return find_char(ch, p+1, *door, range);
}

/*
 * Find an obj in a list.
 */
static OBJ_DATA *get_obj_list_raw(CHAR_DATA *ch, const char *name,
				  uint *number, OBJ_DATA *list, int flags)
{
	OBJ_DATA *obj;

	for (obj = list; obj; obj = obj->next_content) {
		if (!can_see_obj(ch, obj)
		||  !IS_OBJ_NAME(obj, name))
			continue;

		switch (flags) {
		case GETOBJ_F_INV:
			if (obj->wear_loc != WEAR_NONE)
				continue;
			break;

		case GETOBJ_F_WORN:
			if (obj->wear_loc == WEAR_NONE)
				continue;
			break;
		}

		if (!--(*number))
			return obj;
	}

	return NULL;
}

/*
 * Find an obj in the room or in eq/inventory.
 */
static OBJ_DATA *get_obj_here_raw(CHAR_DATA *ch, const char *name,
				  uint *number)
{
	OBJ_DATA *obj;

/* search in player's inventory */
	obj = get_obj_list_raw(ch, name, number, ch->carrying,
			       GETOBJ_F_INV);
	if (obj)
		return obj;

/* search in player's eq */
	obj = get_obj_list_raw(ch, name, number, ch->carrying, GETOBJ_F_WORN);
	if (obj)
		return obj;

/* search in room contents */
	obj = get_obj_list_raw(ch, name, number, ch->in_room->contents,
			       GETOBJ_F_ANY);
	if (obj)
		return obj;

	return NULL;
}

/*
 * Find an obj in a list.
 */
OBJ_DATA *
get_obj_list(CHAR_DATA *ch, const char *argument, OBJ_DATA *list, int flags)
{
	char arg[MAX_INPUT_LENGTH];
	uint number;

	number = number_argument(argument, arg, sizeof(arg));
	if (!number || arg[0] == '\0')
		return NULL;

	return get_obj_list_raw(ch, arg, &number, list, flags);
}

/*
 * Find an obj in player's inventory.
 */
OBJ_DATA *get_obj_carry(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	uint number;

	number = number_argument(argument, arg, sizeof(arg));
	if (!number || arg[0] == '\0')
		return NULL;

	return get_obj_list_raw(ch, arg, &number, ch->carrying,
				GETOBJ_F_INV);
}

/*
 * Find an obj in player's equipment.
 */
OBJ_DATA *get_obj_wear(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	uint number;

	number = number_argument(argument, arg, sizeof(arg));
	if (!number || arg[0] == '\0')
		return NULL;

	return get_obj_list_raw(ch, arg, &number, ch->carrying, GETOBJ_F_WORN);
}

/*
 * Find an obj in the room or in inventory.
 */
OBJ_DATA *get_obj_here(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	uint number;

	number = number_argument(argument, arg, sizeof(arg));
	if (!number || arg[0] == '\0')
		return NULL;

	return get_obj_here_raw(ch, arg, &number);
}

OBJ_DATA *get_obj_room(CHAR_DATA *ch, const char *argument)
{
	OBJ_DATA *obj;
	CHAR_DATA *vch;
	char arg[MAX_INPUT_LENGTH];
	uint number;

	number = number_argument(argument, arg, sizeof(arg));
	if (!number || arg[0] == '\0')
		return NULL;

	if ((obj = get_obj_here_raw(ch, arg, &number)))
		return obj;

	for (vch = ch->in_room->people; vch; vch = vch->next_in_room) {
		/*
		 * search in the vch's inventory
		 */
		obj = get_obj_list_raw(ch, arg, &number, vch->carrying,
				       GETOBJ_F_INV);
		if (obj)
			return obj;

		/*
		 * search in the vch's eq
		 */
		obj = get_obj_list_raw(ch, arg, &number, vch->carrying,
				       GETOBJ_F_WORN);
		if (obj)
			return obj;
	}

	return NULL;
}

/*
 * Find an obj in the world.
 */
OBJ_DATA *get_obj_world(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	uint number;

	number = number_argument(argument, arg, sizeof(arg));
	if (!number || arg[0] == '\0')
		return NULL;

	if ((obj = get_obj_here_raw(ch, arg, &number)))
		return obj;

	for (obj = object_list; obj; obj = obj->next) {
		if (can_see_obj(ch, obj)
		&&  obj->carried_by != ch
		&&  IS_OBJ_NAME(obj, arg)
		&&  !--number)
			return obj;
	}

	return NULL;
}

/*
 * deduct cost from a character
 */
void deduct_cost(CHAR_DATA *ch, int cost)
{
	/*
	 * price in silver. MUST BE signed for proper exchange operations
	 */
	int silver = UMIN(ch->silver, cost); 
	int gold = 0;

	if (silver < cost) {
		gold = (cost - silver + 99) / 100;
		silver = cost - 100 * gold;
	}

	if (ch->gold < gold) {
		log(LOG_INFO, "deduct cost: %s: ch->gold (%d) < gold (%d)",
			   ch->name, ch->gold, gold);
		ch->gold = gold;
	}

	if (ch->silver < silver) {
		log(LOG_INFO, "deduct cost: %s: ch->silver (%d) < silver (%d)",
			   ch->name, ch->silver, silver);
		ch->silver = silver;
	}

	ch->gold -= gold;
	ch->silver -= silver;
} 

static inline void
money_form(int lang, char *buf, size_t len, uint num, const char *name)
{
	char tmp[MAX_STRING_LENGTH];

	strnzcpy(tmp, sizeof(tmp),
		 word_form(GETMSG(name, lang), 1, lang, RULES_CASE));
	strnzcpy(buf, len, word_form(tmp, num, lang, RULES_QTY));
}

static const char *
money_descr_cb(int lang, const char **p, va_list ap)
{
	uint num1 = va_arg(ap, uint);
	const char *name1 = va_arg(ap, const char *);
	uint num2 = va_arg(ap, uint);
	const char *name2 = va_arg(ap, const char *);

	char buf1[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];

	const char *q;

	if (IS_NULLSTR(*p))
		return NULL;

	money_form(lang, buf1, sizeof(buf1), num1, name1);
	money_form(lang, buf2, sizeof(buf2), num2, name2);

	q = str_printf(*p, num1, buf1, num2, buf2);
	free_string(*p);
	*p = q;
	return NULL;
}

/*
 * Create a 'money' obj.
 */
OBJ_DATA *create_money(int gold, int silver)
{
	OBJ_INDEX_DATA *pObjIndex;
	OBJ_DATA *obj;

	if (gold < 0 || silver < 0 || (gold == 0 && silver == 0)) {
		log(LOG_INFO, "create_money: gold %d, silver %d",
			   gold, silver);
		gold = UMAX(1, gold);
		silver = UMAX(1, silver);
	}

	if (gold == 0 && silver == 1)
		obj = create_obj(get_obj_index(OBJ_VNUM_SILVER_ONE), 0);
	else if (gold == 1 && silver == 0)
		obj = create_obj(get_obj_index(OBJ_VNUM_GOLD_ONE), 0);
	else if (silver == 0) {
		pObjIndex = get_obj_index(OBJ_VNUM_GOLD_SOME);
		obj = create_obj(pObjIndex, 0);
		mlstr_foreach(&obj->short_descr, money_descr_cb,
			      (unsigned)gold, "gold coins", -1, NULL);
		INT(obj->value[1]) = gold;
		obj->cost	= 100*gold;
		obj->weight	= gold/5;
	} else if (gold == 0) {
		pObjIndex = get_obj_index(OBJ_VNUM_SILVER_SOME);
		obj = create_obj(pObjIndex, 0);
		mlstr_foreach(&obj->short_descr, money_descr_cb,
			      (unsigned)silver, "silver coins", -1, NULL);
		INT(obj->value[0]) = silver;
		obj->cost	= silver;
		obj->weight	= silver/20;
	} else {
		pObjIndex = get_obj_index(OBJ_VNUM_COINS);
		obj = create_obj(pObjIndex, 0);
		mlstr_foreach(&obj->short_descr, money_descr_cb,
			      (unsigned)silver, "silver coins",
			      (unsigned)gold, "gold coins");
		INT(obj->value[0]) = silver;
		INT(obj->value[1]) = gold;
		obj->cost	= 100*gold + silver;
		obj->weight	= gold/5 + silver/20;
	}

	return obj;
}

/*
 * Return # of objects which an object counts as.
 * Thanks to Tony Chamberlain for the correct recursive code here.
 */
int get_obj_number(OBJ_DATA *obj)
{
	int number;
/* 
	if (obj->item_type == ITEM_CONTAINER || obj->item_type == ITEM_MONEY
	||  obj->item_type == ITEM_GEM || obj->item_type == ITEM_JEWELRY)
	    number = 0;
*/
	if (obj->item_type == ITEM_MONEY)
		number = 0;
	else
	    number = 1;

/* 
	for (obj = obj->contains; obj != NULL; obj = obj->next_content)
	    number += get_obj_number(obj);
*/ 
	return number;
}

int get_obj_realnumber(OBJ_DATA *obj)
{
	int number = 1;

	for (obj = obj->contains; obj != NULL; obj = obj->next_content)
	    number += get_obj_number(obj);

	return number;
}

/*
 * Return weight of an object, including weight of contents.
 */
int get_obj_weight(OBJ_DATA *obj)
{
	int weight;
	OBJ_DATA *tobj;

	weight = obj->weight;
	for (tobj = obj->contains; tobj != NULL; tobj = tobj->next_content)
		weight += get_obj_weight(tobj) * WEIGHT_MULT(obj) / 100;

	return weight;
}

int get_true_weight(OBJ_DATA *obj)
{
	int weight;
 
	weight = obj->weight;
	for (obj = obj->contains; obj != NULL; obj = obj->next_content)
	    weight += get_obj_weight(obj);
 
	return weight;
}

/*
 * True if room is dark.
 */
bool room_is_dark(CHAR_DATA *ch)
{
	ROOM_INDEX_DATA * pRoomIndex = ch->in_room;

	if (!IS_NPC(ch) && IS_SET(PC(ch)->plr_flags, PLR_HOLYLIGHT))
		return FALSE;

	if (is_affected(ch, "vampire"))
		return FALSE;

	return room_dark(pRoomIndex);
}

bool room_dark(ROOM_INDEX_DATA *pRoomIndex)
{
	if (pRoomIndex->light > 0)
		return FALSE;

	if (IS_AFFECTED(pRoomIndex, RAFF_LIGHT))
		return FALSE;

	if (IS_SET(pRoomIndex->room_flags, ROOM_DARK))
		return TRUE;

	if (pRoomIndex->sector_type == SECT_INSIDE
	||   pRoomIndex->sector_type == SECT_CITY)
		return FALSE;

	if (weather_info.sunlight == SUN_SET
	||  weather_info.sunlight == SUN_DARK)
		return TRUE;

	return FALSE;
}


/*
 * True if room is private.
 */
bool room_is_private(ROOM_INDEX_DATA *pRoomIndex)
{
	CHAR_DATA *rch;
	int count;

/*
	if (pRoomIndex->owner != NULL && pRoomIndex->owner[0] != '\0')
		return TRUE;
*/
	count = 0;
	for (rch = pRoomIndex->people; rch != NULL; rch = rch->next_in_room)
		count++;

	if (IS_SET(pRoomIndex->room_flags, ROOM_PRIVATE)  && count >= 2)
		return TRUE;

	if (IS_SET(pRoomIndex->room_flags, ROOM_SOLITARY) && count >= 1)
		return TRUE;
	
	if (IS_SET(pRoomIndex->room_flags, ROOM_IMP_ONLY))
		return TRUE;

	return FALSE;
}

/* visibility on a room -- for entering and exits */
bool can_see_room(CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex)
{
	if (IS_SET(pRoomIndex->room_flags, ROOM_IMP_ONLY) 
	&&  !IS_TRUSTED(ch, LEVEL_IMP))
		return FALSE;

	if (IS_SET(pRoomIndex->room_flags, ROOM_GODS_ONLY)
	&&  !IS_TRUSTED(ch, LEVEL_GOD))
		return FALSE;

	if (IS_SET(pRoomIndex->room_flags, ROOM_HEROES_ONLY)
	&&  !IS_TRUSTED(ch, LEVEL_HERO))
		return FALSE;

	if (IS_SET(pRoomIndex->room_flags, ROOM_NEWBIES_ONLY)
	&&  ch->level > LEVEL_NEWBIE && !IS_IMMORTAL(ch))
		return FALSE;

	return TRUE;
}

/*
 * True if char can see victim.
 */
bool can_see(CHAR_DATA *ch, CHAR_DATA *victim)
{
	CHAR_DATA *vch;
	flag_t ch_can_see = ch->has_detect;
	if (ch_can_see & ID_INVIS)
		ch_can_see |= ID_IMP_INVIS;

	if (ch == NULL || victim == NULL) {
		log(LOG_BUG, "can_see: ch = %p, victim = %p", ch, victim);
		return FALSE;
	}

	vch = GET_ORIGINAL(ch);
	if (ch == victim || vch == victim)
		return TRUE;

	if (IS_CLAN_GUARD(ch))
		return TRUE;

	/*
	 * wizi check for PC victim
	 */
	if (!IS_NPC(victim) && !IS_TRUSTED(ch, victim->invis_level))
		return FALSE;

	if (!IS_TRUSTED(ch, victim->incog_level)
	&&  ch->in_room != victim->in_room)
		return FALSE;

	if (!IS_NPC(vch) && IS_SET(PC(vch)->plr_flags, PLR_HOLYLIGHT))
		return TRUE;

	/*
	 * wizi check for mob victim
	 */
	if (IS_NPC(victim) && !IS_TRUSTED(ch, victim->invis_level)) {
		AREA_DATA *pArea = area_vnum_lookup(victim->pMobIndex->vnum);
		if (pArea == NULL
		||  !IS_BUILDER(vch, pArea))
			return FALSE;
	}

	if (HAS_DETECT(ch, ID_TRUESEEING))
		return TRUE;

	if (IS_AFFECTED(ch, AFF_BLIND))
		return FALSE;

	if (is_affected(ch, "hallucination"))
		return (number_percent() < 70);

	if (room_is_dark(ch) && !HAS_DETECT(ch, ID_INFRARED))
		return FALSE;

	if ((victim->has_invis & ID_ALL_INVIS & ~ch_can_see) != 0)
		return FALSE;

	return TRUE;
}

/*
 * True if char can see obj.
 */
bool can_see_obj(CHAR_DATA *ch, OBJ_DATA *obj)
{
	if (!IS_NPC(ch) && IS_SET(PC(ch)->plr_flags, PLR_HOLYLIGHT))
		return TRUE;

	if (IS_OBJ_STAT(obj, ITEM_VIS_DEATH))
		return FALSE;

	if (IS_AFFECTED(ch, AFF_BLIND) && obj->item_type != ITEM_POTION)
		return FALSE;

	if (obj->item_type == ITEM_LIGHT
	&&  INT(obj->value[2]) != 0)
		return TRUE;

	if (IS_OBJ_STAT(obj, ITEM_INVIS) && !HAS_DETECT(ch, ID_INVIS))
		return FALSE;

	if (IS_OBJ_STAT(obj, ITEM_GLOW))
		return TRUE;

	if (room_is_dark(ch) && !HAS_DETECT(ch, ID_INFRARED))
		return FALSE;

	return TRUE;
}

/*
 * True if char can drop obj.
 */
bool can_drop_obj(CHAR_DATA *ch, OBJ_DATA *obj)
{
	if (!IS_OBJ_STAT(obj, ITEM_NODROP))
		return TRUE;

	if (IS_IMMORTAL(ch))
		return TRUE;

	return FALSE;
}

int isn_dark_safe(CHAR_DATA *ch)
{
	CHAR_DATA *rch;
	OBJ_DATA *light;
	int light_exist;

	if (!is_affected(ch, "vampire")
	||  IS_SET(ch->in_room->room_flags, ROOM_DARK))
		return 0;

	if (weather_info.sunlight == SUN_LIGHT
	||  weather_info.sunlight == SUN_RISE)
		return 2;

	light_exist = 0;
	for (rch = ch->in_room->people; rch; rch = rch->next_in_room) {
		if ((light = get_eq_char(rch, WEAR_LIGHT))
		&&  INT(light->value[2]) != 0) {
			light_exist = 1;
			break;
		}
	}

	return light_exist;
}

int count_charmed(CHAR_DATA *ch)	
{
	CHAR_DATA *gch;
	int count = 0;

	for (gch = char_list; gch != NULL; gch = gch->next) {
		if (IS_AFFECTED(gch, AFF_CHARM) && gch->master == ch)
			count++;
	}

	if (count >= MAX_CHARM(ch)) {
		act_char("You are already controlling as many charmed mobs as you can!", ch);
		return count;
	}
	return 0;
}

/*
 * add_mind - remember 'str' in mind buffer of 'ch'
 *	      remember the place to return in mind buffer if it is empty
 *	      ch is assumed to be IS_NPC
 */
void add_mind(CHAR_DATA *ch, const char *str)
{
	NPC_DATA *npc = NPC(ch);

	if (npc->in_mind == NULL) {
		/* remember a place to return */
		npc->in_mind = str_printf("%d", ch->in_room->vnum);
	}

	name_add(&npc->in_mind, str, NULL, NULL);
}

/*
 * remove_mind - remove 'str' from mind buffer of 'ch'
 *		 if it was the last revenge - return home
 *		 ch is assumed to be IS_NPC
 */
void remove_mind(CHAR_DATA *ch, const char *str)
{
	NPC_DATA *npc = NPC(ch);

	if (!name_delete(&npc->in_mind, str, NULL, NULL))
		return;

	if (IS_NULLSTR(npc->in_mind) || is_number(npc->in_mind)) {
		dofun("say", ch, "At last, I took my revenge!"); 
		back_home(ch);
		if (!IS_EXTRACTED(ch)) {
			free_string(npc->in_mind);
			npc->in_mind = NULL;
		}
	}
}

/*
 * ch is assumed to be IS_NPC
 */
void back_home(CHAR_DATA *ch)
{
	NPC_DATA *npc = NPC(ch);
	ROOM_INDEX_DATA *location;
	char arg[MAX_INPUT_LENGTH];

	if (npc->in_mind == NULL
	&&  ch->pMobIndex->vnum < 100) {
		act("$n wanders on home.", ch, NULL, NULL, TO_ROOM);
		extract_char(ch, 0);
	}

	one_argument(npc->in_mind, arg, sizeof(arg));
	if ((location = find_location(ch, arg)) == NULL) {
		act("$n wanders on home.", ch, NULL, NULL, TO_ROOM);
		extract_char(ch, 0);
		return;
	}

	if (ch->fighting == NULL && location != ch->in_room) {
		act("$n prays for transportation.", ch, NULL, NULL, TO_ROOM);
		recall(ch, location);
	}
}

void path_to_track(CHAR_DATA *ch, CHAR_DATA *victim, int door)
{
	ROOM_INDEX_DATA *temp;
	EXIT_DATA *pExit;
	int opdoor;
	int range = 0;

	SET_FIGHT_TIME(ch);
  	if (!IS_NPC(victim)) {
		SET_FIGHT_TIME(victim);
		return;
	}

	if (victim->position == POS_DEAD)
		return;

	NPC(victim)->last_fought = ch;

	if ((opdoor = opposite_door(door)) == -1) {
		log(LOG_BUG, "In path_to_track wrong door: %d",door);
		return;
	}

	temp = ch->in_room;
	while (1) {
		range++;
		if (victim->in_room == temp)
			break;
		if ((pExit = temp->exit[ door ]) == NULL
		||  (temp = pExit->to_room.r) == NULL) {
			log(LOG_BUG, "path_to_track: couldn't calculate range %d",
			    range);
			return;
		}

		if (range > 100) {
			log(LOG_BUG, "path_to_track: range exceeded 100");
			return;
		}
	}

	temp = victim->in_room;
	while (--range > 0) {
		room_record(ch->name, temp, opdoor);
		if ((pExit = temp->exit[opdoor]) == NULL
		||  (temp = pExit->to_room.r) == NULL) {
			log(LOG_BUG, "path_to_track: Range: %d Room: %d opdoor:%d",
			     range, temp->vnum, opdoor); 
			return;
		}
	}
	add_mind(victim, ch->name);
	dofun("track", victim, str_empty);
}

bool can_gate(CHAR_DATA *ch, CHAR_DATA *victim)
{
	if (victim == ch
	||  ch->fighting != NULL
	||  victim->in_room == NULL
	||  !can_see_room(ch, victim->in_room)
	||  IS_SET(ch->in_room->room_flags, ROOM_NORECALL |
					    ROOM_PEACE | ROOM_NOSUMMON)
	||  IS_SET(victim->in_room->room_flags, ROOM_SAFE | ROOM_NORECALL |
						ROOM_PEACE | ROOM_NOSUMMON)
	||  IS_SET(ch->in_room->area->area_flags, AREA_CLOSED)
	||  room_is_private(victim->in_room))
		return FALSE;

	if (IS_NPC(victim)) {
		if (IS_SET(victim->pMobIndex->act, ACT_IMMSUMMON))
			return FALSE;
		if (NPC(victim)->hunter)
			return FALSE;
		return TRUE;
	}

	if (((!in_PK(ch, victim) ||
	      ch->in_room->area != victim->in_room->area) &&
	     IS_SET(PC(victim)->plr_flags, PLR_NOSUMMON))
	||  victim->level >= LEVEL_HERO
	||  !guild_ok(ch, victim->in_room))
		return FALSE;

	return TRUE;
}

void transfer_char(CHAR_DATA *ch, CHAR_DATA *vch, ROOM_INDEX_DATA *to_room,
		   const char *msg_out,
		   const char *msg_travel,
		   const char *msg_in)
{
	ROOM_INDEX_DATA *was_in = ch->in_room;

	if (ch != vch)
		act_puts(msg_travel, vch, NULL, ch, TO_VICT, POS_DEAD);

	char_from_room(ch);

	act(msg_out, was_in->people, NULL, ch, TO_ALL);
	act(msg_in, to_room->people, NULL, ch, TO_ALL);

	char_to_room(ch, to_room);

	if (!IS_EXTRACTED(ch))
		dofun("look", ch, "auto");
}

void
recall(CHAR_DATA *ch, ROOM_INDEX_DATA *location)
{
	transfer_char(ch, NULL, location,
		      "$N disappears.", NULL, "$N appears in the room.");
}

void look_at(CHAR_DATA *ch, ROOM_INDEX_DATA *room)
{
	ROOM_INDEX_DATA *was_in = ch->in_room;
	OBJ_DATA *obj;
	bool adjust_light = FALSE;

	if ((obj = get_eq_char(ch, WEAR_LIGHT))
	&&  obj->item_type == ITEM_LIGHT
	&&  INT(obj->value[2])) {
		adjust_light = TRUE;
		room->light++;
	}
		
	ch->in_room = room;
	dofun("look", ch, str_empty);
	ch->in_room = was_in;

	if (adjust_light)
		room->light--;
}

/* random room generation procedure */
ROOM_INDEX_DATA  *get_random_room(CHAR_DATA *ch, AREA_DATA *area)
{
	int min_vnum;
	int max_vnum;
	ROOM_INDEX_DATA *room;

	if (!area) {
		min_vnum = 1;
		max_vnum = top_vnum_room;
	}
	else {
		min_vnum = area->min_vnum;
		max_vnum = area->max_vnum;
	}

	for (; ;) {
		room = get_room_index(number_range(min_vnum, max_vnum));
		if (!room)
			continue;

		if (ch) {
			if (!can_see_room(ch, room)
			||  (IS_NPC(ch) &&
			     IS_SET(ch->pMobIndex->act, ACT_AGGRESSIVE) &&
			     IS_SET(room->room_flags, ROOM_LAW)))
				continue;
		}

		if (!room_is_private(room)
		&&  !IS_SET(room->room_flags, ROOM_SAFE | ROOM_PEACE) 
		&&  (area || !IS_SET(room->area->area_flags, AREA_CLOSED)))
			break;
	}

	return room;
}

CHAR_DATA *nth_char(CHAR_DATA *ch, int n)
{
	CHAR_DATA *vch;

	int i = 0;

	for (vch = ch; i < n; vch = (vch->next) ? vch->next : char_list)
		i++;
	return vch;
}

OBJ_DATA *nth_obj(OBJ_DATA *obj, int n)
{
	OBJ_DATA *nobj;

	int i = 0;

	for (nobj = obj; i < n; nobj = (nobj->next) ? nobj->next : object_list)
		i++;
	return nobj;
}

/* Random character generation procedure */

CHAR_DATA *random_char(ROOM_INDEX_DATA *room)
{
    	CHAR_DATA *vch, *victim = NULL;
    	int now = 0, highest = 0;

	if (room != NULL) {
    		for (vch = room->people; vch; vch = vch->next_in_room) {
        		if ((now = number_percent()) > highest) {
            			victim = vch;
            			highest = now;
        		}
    		}
		return victim;
	} else {
		for (vch = char_list; vch; vch = vch->next)  {
        		if ((now = number_range(1, 20000)) > highest) {
            			victim = vch;
            			highest = now;
        		}
    		}
		return victim;
	}
}

OBJ_DATA *random_obj()
{
	OBJ_DATA *obj;
	OBJ_DATA *rand_obj;
	int highest = 0, now = 0;

	for (rand_obj = obj = object_list; obj; obj = obj->next)  {
       		if ((now = number_range(1, 20000)) > highest) {
       			rand_obj = obj;
       			highest = now;
       		}
    	}
	return rand_obj;
}

void format_obj(BUFFER *output, OBJ_DATA *obj)
{
	int i;
	liquid_t *lq;

	buf_printf(output, BUF_END,
		"Object '%s%s' is type %s, stat flags %s.\n"
		"Weight is %d, value is %d, level is %d.\n",
		obj->pObjIndex->name, obj->label,
		flag_string(item_types, obj->item_type),
		flag_string(stat_flags, obj->stat_flags & ~ITEM_ENCHANTED),
		obj->weight,
		obj->cost,
		obj->level);

	if (obj->pObjIndex->limit != -1)
		buf_printf(output, BUF_END,
			   "This equipment has been LIMITED by number %d \n",
			   obj->pObjIndex->limit);

	switch (obj->item_type) {
	case ITEM_SCROLL:
	case ITEM_POTION:
	case ITEM_PILL:
		buf_printf(output, BUF_END, "Level %d spells of:", INT(obj->value[0]));

		for (i = 1; i < 5; i++) {
			if (!IS_NULLSTR(obj->value[i].s))
				buf_printf(output, BUF_END, " '%s'", obj->value[i].s);
		}

		buf_append(output, ".\n");
		break;

	case ITEM_WAND: 
	case ITEM_STAFF: 
		buf_printf(output, BUF_END, "Has %d charges of level %d",
			   INT(obj->value[2]), INT(obj->value[0]));
	  
		if (!IS_NULLSTR(obj->value[3].s))
			buf_printf(output, BUF_END, " '%s'", obj->value[3].s);

		buf_append(output, ".\n");
		break;

	case ITEM_DRINK_CON:
		if ((lq = liquid_lookup(STR(obj->value[2]))) == NULL)
			break;
		buf_printf(output, BUF_END, "It holds %s-colored %s.\n",
			   mlstr_mval(&lq->lq_color),
			   gmlstr_mval(&lq->lq_name));
		break;

	case ITEM_CONTAINER:
		buf_printf(output, BUF_END,
			   "Capacity: %d#  Maximum weight: %d#  flags: %s\n",
			   INT(obj->value[0]), INT(obj->value[3]),
			   SFLAGS(cont_flags, obj->value[1]));
		if (INT(obj->value[4]) != 100)
			buf_printf(output, BUF_END, "Weight multiplier: %d%%\n",
				   INT(obj->value[4]));
		break;
			
	case ITEM_WEAPON:
		buf_printf(output, BUF_END, "Weapon type is %s.\n",
			   SFLAGS(weapon_class, obj->value[0]));
		buf_printf(output, BUF_END, "Damage is %dd%d (average %d).\n",
			   INT(obj->value[1]), INT(obj->value[2]),
			   GET_WEAPON_AVE(obj));
		if (INT(obj->value[4])) {
	        	buf_printf(output, BUF_END, "Weapon flags: %s\n",
				   SFLAGS(weapon_type2, obj->value[4]));
		}
		break;

	case ITEM_ARMOR:
		buf_printf(output, BUF_END, "Armor class is %d pierce, "
				   "%d bash, %d slash, and %d vs. magic.\n", 
			   INT(obj->value[0]), INT(obj->value[1]),
			   INT(obj->value[2]), INT(obj->value[3]));
		break;
	}
}

void format_obj_affects(BUFFER *output, AFFECT_DATA *paf, int flags)
{
	for (; paf; paf = paf->next) {
		where_t *w;

		if ((w = where_lookup(paf->where)) == NULL)
			continue;

		if (!IS_NULLSTR(w->loc_format)
		&& (paf->where == TO_RESIST
			|| paf->where == TO_FORMRESIST
			|| INT(paf->location) != APPLY_NONE)
		&&  paf->modifier) { 
			buf_printf(output, BUF_END, w->loc_format,
				paf->where != TO_RACE ?
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

bool check_blind_raw(CHAR_DATA *ch)
{
	if (!IS_NPC(ch) && IS_SET(PC(ch)->plr_flags, PLR_HOLYLIGHT))
		return TRUE;

	if (IS_AFFECTED(ch, AFF_BLIND))
		return FALSE;

	return TRUE;
}

bool check_blind(CHAR_DATA *ch)
{
	bool cansee = check_blind_raw(ch);

	if (!cansee)
		act_char("You can't see a thing!", ch);

	return cansee;
}

/*
 * Parse a name for acceptability.
 */
bool pc_name_ok(const char *name)
{
	const unsigned char *pc;
	bool fIll,adjcaps = FALSE,cleancaps = FALSE;
 	uint total_caps = 0;

	/*
	 * Reserved words.
	 */
	if (is_name(name, "chronos all auto immortals self someone something the you demise balance circle loner honor none clan"))		// notrans
		return FALSE;
	
	/*
	 * Length restrictions.
	 */
	 
	if (strlen(name) < 2)
		return FALSE;

	if (strlen(name) > MAX_CHAR_NAME)
		return FALSE;

	/*
	 * Alphanumerics only.
	 * Lock out IllIll twits.
	 */
	fIll = TRUE;
	for (pc = name; *pc != '\0'; pc++) {
		if (IS_SET(mud_options, OPT_ASCII_ONLY_NAMES) && !isascii(*pc))
			return FALSE;

		if (!isalpha(*pc))
			return FALSE;

		if (isupper(*pc)) { /* ugly anti-caps hack */
			if (adjcaps)
				cleancaps = TRUE;
			total_caps++;
			adjcaps = TRUE;
		}
		else
			adjcaps = FALSE;

		if (LOWER(*pc) != 'i' && LOWER(*pc) != 'l')
			fIll = FALSE;
	}

	if (fIll)
		return FALSE;

	if (total_caps > strlen(name) / 2)
		return FALSE;

	/*
	 * Prevent players from naming themselves after mobs.
	 */
	{
		MOB_INDEX_DATA *pMobIndex;
		int iHash;

		for (iHash = 0; iHash < MAX_KEY_HASH; iHash++) {
			for (pMobIndex  = mob_index_hash[iHash];
			     pMobIndex != NULL; pMobIndex  = pMobIndex->next) 
				if (is_name(name, pMobIndex->name))
					return FALSE;
		}
	}

	if (clan_lookup(name) != NULL)
		return FALSE;

	return TRUE;
}

const char *stat_val_aliases[MAX_STAT][6] =
{
  { "Titanic", "Herculian", "Strong", "Average", "Poor", "Weak"	},  // notrans
  { "Genious", "Clever", "Good", "Average", "Poor", "Hopeless"	},  // notrans
  { "Excellent", "Wise", "Good", "Average", "Dim", "Fool"	},  // notrans
  { "Fast", "Quick", "Dextrous", "Average", "Clumsy", "Slow"	},  // notrans
  { "Iron", "Hearty", "Healthy", "Average", "Poor", "Fragile"	},  // notrans
  { "Charismatic", "Familier", "Good", "Average", "Poor", "Mongol"} // notrans
};

const char *get_stat_alias(CHAR_DATA *ch, int stat)
{
	int val;
	int i;

	if (stat >= MAX_STAT)
		return "Unknown";				// notrans

	val = get_curr_stat(ch, stat);
	     if (val >  22)	i = 0;
	else if (val >= 20)	i = 1;
	else if (val >= 18)	i = 2;
	else if (val >= 14)	i = 3;
	else if (val >= 10)	i = 4;
	else			i = 5;
	return stat_val_aliases[stat][i];
}

/*
 * returns TRUE if lch is one of the leaders of ch
 */
bool is_leader(CHAR_DATA *ch, CHAR_DATA *lch)
{
	for (ch = ch->leader; ch; ch = ch->leader) {
		if (ch == lch)
			return TRUE;
	}

	return FALSE;
}

void set_leader(CHAR_DATA *ch, CHAR_DATA *lch)
{
	/*
	 * This code must prevent ->leader cycling
	 */
	if (is_leader(lch, ch)) {
		CHAR_DATA *tch;
		CHAR_DATA *tch_next;

		for (tch = lch; tch && tch != ch; tch = tch_next) {
			tch_next = tch->leader;
			log(LOG_INFO, "set_leader: removing cycle: %s", tch->name);
			tch->leader = NULL;
			stop_follower(tch);
		}
	}
	ch->leader = lch;
}

void set_title(CHAR_DATA *ch, const char *title)
{
	char buf[MAX_TITLE_LENGTH];
	static char nospace[] = "-.,!?':";		// notrans

	buf[0] = '\0';

	if (title) {
		if (strchr(nospace, *cstrfirst(title)) == NULL) {
			buf[0] = ' ';
			buf[1] = '\0';
		}

		strnzcat(buf, sizeof(buf), title);
	}

	free_string(PC(ch)->title);
	PC(ch)->title = str_dup(buf);
}

const char *garble(CHAR_DATA *ch, const char *i)
{
	static char not_garbled[] = "?!()[]{},.:;'\" ";		// notrans
	static char buf[MAX_STRING_LENGTH];
	char *o;

	if (!is_affected(ch, "garble"))
		return i;

	for (o = buf; *i && o < buf + sizeof(buf) - 1; i++, o++) {
		if (strchr(not_garbled, *i))
			*o = *i;
		else
			*o = number_range(' ', 254);
	}
	*o = '\0';
	return buf;
}

void do_tell_raw(CHAR_DATA *ch, CHAR_DATA *victim, const char *msg)
{
	if (ch == victim) {
		act_char("Talking to yourself, eh?", ch);
		return;
	}

	if (ch->shapeform 
	&& IS_SET(ch->shapeform->index->flags, FORM_NOSPEAK)) {
		act("You can't speak in this form.", ch, NULL, NULL, TO_CHAR);
		return;
	}

	if (IS_SET(ch->comm, COMM_NOTELL)) {
		act_char("Your message didn't get through.", ch);
		return;
	}

	if (victim == NULL 
	|| (IS_NPC(victim) && victim->in_room != ch->in_room)) {
		act_char("They aren't here.", ch);
		return;
	}

	if (IS_SET(victim->comm, (COMM_QUIET | COMM_DEAF))
	&&  !IS_IMMORTAL(ch) && !IS_IMMORTAL(victim)) {
		act_puts("$E is not receiving tells.", ch, 0, victim,
			 TO_CHAR, POS_DEAD);
		return;
	}

	msg = garble(ch, msg);
	act_puts("You tell $N '{G$t{x'",
		 ch, msg, victim, TO_CHAR | ACT_SPEECH(ch), POS_DEAD);
	act_puts("$n tells you '{G$t{x'",
		 ch, msg, victim,
		 TO_VICT | ACT_TOBUF | ACT_NOTWIT | ACT_SPEECH(ch),
		 POS_SLEEPING);

	if (IS_NPC(ch))
		return;

	if (IS_NPC(victim)) {
		if (HAS_TRIGGER(victim, TRIG_SPEECH))
			mp_act_trigger(msg, victim, ch, NULL, NULL, TRIG_SPEECH);
	}
	else {
		if (!IS_IMMORTAL(victim)
		&&  !IS_IMMORTAL(ch)
		&&  is_name(ch->name, PC(victim)->twitlist))
			return;

		if (victim->desc == NULL)
			act_puts("$N seems to have misplaced $S link but "
				 "your tell will go through if $E returns.",
				 ch, NULL, victim, TO_CHAR, POS_DEAD);
		else if (IS_SET(victim->comm, COMM_AFK))
			act_puts("$E is AFK, but your tell will go through "
				 "when $E returns.",
				 ch, NULL, victim, TO_CHAR, POS_DEAD);
		PC(victim)->reply = ch;
	}
}

void yell(CHAR_DATA *victim, CHAR_DATA* ch, const char* text)
{
	if (IS_NPC(victim)
	||  IS_IMMORTAL(victim)
	||  victim->in_room == NULL
	||  victim->position <= POS_SLEEPING
	||  IS_EXTRACTED(victim)
	||  IS_SET(PC(victim)->plr_flags, PLR_GHOST))
		return;

	act_puts("You yell '{M$t{x'", victim,
		 act_speech(victim, victim, text, ch), NULL,
		 TO_CHAR | ACT_SPEECH(ch), POS_DEAD);
	act_yell(victim, text, ch, "$n yells in panic '{M$t{x'");
}

static void *
drop_objs_cb(void *p, va_list ap)
{
	clan_t *clan = (clan_t *) p;

	OBJ_DATA *obj = va_arg(ap, OBJ_DATA *);

	if (obj == clan->obj_ptr) {
		obj_to_room(obj, get_room_index(clan->altar_vnum));
		return p;
	}

	return NULL;
}

static void drop_objs(CHAR_DATA *ch, OBJ_DATA *obj_list)
{
	OBJ_DATA *obj, *obj_next;

	/*
	 * drop OBJ_QUIT_DROP/OBJ_CHQUEST/OBJ_CLAN items
	 */
	for (obj = obj_list; obj != NULL; obj = obj_next) {
		obj_next = obj->next_content;

		if (obj->contains)
			drop_objs(ch, obj->contains);

		if (!OBJ_IS(obj, OBJ_CLAN | OBJ_QUIT_DROP | OBJ_CHQUEST))
			continue;

		obj->last_owner = NULL;

		if (obj->carried_by)
			obj_from_char(obj);
		else if (obj->in_obj)
			obj_from_obj(obj);
		else {
			extract_obj(obj, 0);
			continue;
		}

		if (!OBJ_IS(obj, OBJ_CLAN)) {
			if (ch->in_room != NULL)
				obj_to_room(obj, ch->in_room);
			else
				extract_obj(obj, 0);
			continue;
		}

		if (hash_foreach(&clans, drop_objs_cb, obj) != NULL)
			continue;
		extract_obj(obj, 0);
	}
}

/*
 * ch is assumed to be !IS_NPC
 */
void quit_char(CHAR_DATA *ch, int flags)
{
	DESCRIPTOR_DATA *d, *d_next;
	CHAR_DATA *vch, *vch_next;
	OBJ_DATA *obj;
	const char *name;

	if (ch->position == POS_FIGHTING) {
		act_char("No way! You are fighting.", ch);
		return;
	}

	if (ch->position < POS_STUNNED ) {
		act_char("You're not DEAD yet.", ch);
		return;
	}

	if (IS_AFFECTED(ch, AFF_CHARM)) {
		act("You don't want to leave your master.",
		    ch, NULL, ch->master, TO_CHAR);
		return;
	}

	if (IS_SET(PC(ch)->plr_flags, PLR_NOEXP)) {
		act_char("You don't want to lose your spirit.", ch);
		return;
	}

	if (IS_AFFECTED(ch, AFF_SLEEP)) {
		act_char("You cannot quit, you are in deep sleep.", ch);
		return;
	}

	if (ch->shapeform) {
		AFFECT_DATA *paf;
		AFFECT_DATA *paf_next;
		for (paf = ch->affected; paf; paf = paf_next) {
			paf_next = paf->next;
			if (paf->where == TO_FORM)
				affect_remove (ch, paf);
		}
	}

	for (vch = npc_list; vch; vch = vch->next) {
		if (IS_AFFECTED(vch, AFF_CHARM)
		&&  IS_NPC(vch)
		&&  IS_SET(vch->pMobIndex->act, ACT_FAMILIAR)
		&&  vch->master == ch
		&&  vch->in_room != ch->in_room) {
			act("You cannot quit and leave your $N alone.", 
				ch, NULL, vch, TO_CHAR);
			return;
		}
	}

	if (auction.item != NULL
	&&  ((ch == auction.buyer) || (ch == auction.seller))) {
		act_char("Wait till you have sold/bought the item on auction.",ch);
		return;
	}

	if (!IS_IMMORTAL(ch)) {
		if (IS_PUMPED(ch)) {
			act_char("Your adrenalin is gushing! You can't quit yet.", ch);
			return;
		}

		if (is_affected(ch, "witch curse")) {
			act_char("You are cursed. Wait till you DIE!", ch);
			return;
		}

		if (!IS_NULLSTR(ch->in_room->area->clan)
		&&  !IS_CLAN(ch->in_room->area->clan, ch->clan)) {
			act_char("You can't quit here.", ch);
			return;
		}

		if (ch->in_room && IS_AFFECTED(ch->in_room, RAFF_ESPIRIT)) {
			act_char("Evil spirits in the area prevents you from leaving.", ch);
			return;
		}

		if (!get_skill(ch, "evil spirit")
		&&  is_affected(ch, "evil spirit")) {
			act_char("Evil spirits in you prevents you from leaving.", ch);
			return;
		}

		if (current_time - PC(ch)->last_offence < OFFENCE_DELAY_TIME 
		&& !IS_IMMORTAL(ch)) {
			act_char("You cannot quit yet.", ch);
			return;
		}
	}

	drop_objs(ch, ch->carrying);

	if (!IS_NPC(ch)) {
		if (ch->mount != NULL)
			dofun("dismount", ch, str_empty);

		if ((vch = PC(ch)->guarding) != NULL) {
			PC(ch)->guarding = NULL;
			PC(vch)->guarded_by = NULL;
			act("You stop guarding $N.",
			    ch, NULL, vch, TO_CHAR);
			act("$n stops guarding you.",
			    ch, NULL, vch, TO_VICT);
			act("$n stops guarding $N.",
			    ch, NULL, vch, TO_NOTVICT);
			if (ch->in_room != vch->in_room) {
				act("$N stops guarding $n.",
				    vch, NULL, ch, TO_NOTVICT);
			}
		}

		if ((vch = PC(ch)->guarded_by) != NULL) {
			PC(vch)->guarding = NULL;
			PC(ch)->guarded_by = NULL;
			act("You stop guarding $N.",
			    vch, NULL, ch, TO_CHAR);
			act("$n stops guarding you.",
			    vch, NULL, ch, TO_VICT);
			act("$n stops guarding $N.",
			    vch, NULL, ch, TO_NOTVICT);
			if (ch->in_room != vch->in_room) {
				act("$N stops guarding $n.",
				    ch, NULL, vch, TO_NOTVICT);
			}
		}
	}

	act_char("Alas, all good things must come to an end.", ch);
	act_char("You hit reality hard. Reality truth does unspeakable things to you.", ch);
	act_puts("$n has left the game.", ch, NULL, NULL, TO_ROOM, POS_RESTING);
	log(LOG_INFO, "%s has quit.", ch->name);
	wiznet("{W$N{x rejoins the real world.",
		ch, NULL, WIZ_LOGINS, 0, ch->level);

	for (vch = char_list; vch; vch = vch_next) {
		NPC_DATA *vnpc;

		vch_next = vch->next;
		if (is_affected(vch, "doppelganger")
		&&  vch->doppel == ch) {
			act_char("You shift to your true form as your victim leaves.", vch);
			affect_strip(vch, "doppelganger");
		}

		if (vch->hunting == ch)
			vch->hunting = NULL;

		if (!IS_NPC(vch))
			continue;

		vnpc = NPC(vch);
		if (vnpc->hunter == ch)
			vnpc->hunter = NULL;

		if (vnpc->target == ch) {
			if (vch->pMobIndex->vnum == MOB_VNUM_SHADOW) {
				act("$n slowly fades away.",
				    vch, NULL, NULL, TO_ROOM);
				extract_char(vch, 0);
				continue;
			}

			if (vch->pMobIndex->vnum == MOB_VNUM_STALKER) {
				act_clan(vch, "$i has left the realm, I have to leave too.", ch);
				act("$n slowly fades away.",
				    vch, NULL, NULL, TO_ROOM);
				extract_char(vch, 0);
				continue;
			}
		}

		if (vnpc->last_fought == ch)
			vnpc->last_fought = NULL;
	}

	for (obj = object_list; obj->next; obj = obj->next) {
		if (obj->last_owner == ch)
			obj->last_owner = NULL;
	}

	/*
	 * After extract_char the ch is no longer valid!
	 */

	char_save(ch, 0);
	name = str_qdup(ch->name);
	d = ch->desc;
	extract_char(ch, flags);

	if (d)
		close_descriptor(d, SAVE_F_NONE);

	/*
	 * toast evil cheating bastards 
	 *
	 * Workaround against clone cheat --
	 * Log in once, connect a second time and enter only name,
	 * drop all and quit with first character, finish login
	 * with second. This clones the player's inventory.
	 */
	for (d = descriptor_list; d; d = d_next) {
		CHAR_DATA *tch;

		d_next = d->next;
		tch = d->original ? d->original : d->character;
		if (tch && !str_cmp(name, tch->name)) {
			if (d->connected == CON_PLAYING)
				extract_char(tch, XC_F_NOCOUNT);
			close_descriptor(d, SAVE_F_NONE);
		} 
	}

	free_string(name);
}

void add_follower(CHAR_DATA *ch, CHAR_DATA *master)
{
	if (ch->master)
		stop_follower(ch);
	ch->master = master;
	ch->leader = NULL;

	if (can_see(master, ch))
		act_puts("$n now follows you.", ch, NULL, master, 
			 TO_VICT, POS_RESTING);
	act_puts("You now follow $N.", ch, NULL, master, 
		 TO_CHAR, POS_RESTING);
}

void stop_follower(CHAR_DATA *ch)
{
	if (ch->master == NULL) {
		log(LOG_BUG, "stop_follower: null master");
		return;
	}

	if (IS_AFFECTED(ch, AFF_CHARM)) {
		REMOVE_BIT(ch->affected_by, AFF_CHARM);
		affect_bit_strip(ch, TO_AFFECTS, AFF_CHARM);
	}

	if (can_see(ch->master, ch) && ch->in_room != NULL) {
		act_puts("$n stops following you.",ch, NULL, ch->master,
			 TO_VICT, POS_RESTING);
		act_puts("You stop following $N.", ch, NULL, ch->master,
			 TO_CHAR, POS_RESTING);
	}

	if (!IS_NPC(ch->master)) {
		PC_DATA *pc = PC(ch->master);
		if (pc->pet == ch)
			pc->pet = NULL;
	}

	if (ch->leader && ch->fighting == ch->leader->fighting)
		stop_fighting(ch, TRUE);
	ch->master = NULL;
	ch->leader = NULL;
}

/*
 * nuke pet
 * ch is assumed to be !IS_NPC and be properly inserted in room
 * (extract_char() already  assumes it)
 */
void nuke_pets(CHAR_DATA *ch)
{
	CHAR_DATA *pet;

	if ((pet = PC(ch)->pet) != NULL) {
		stop_follower(pet);
		act("$n slowly fades away.", pet, NULL, NULL, TO_ROOM);
		extract_char(pet, 0);
		PC(ch)->pet = NULL;
	}
}

void die_follower(CHAR_DATA *ch)
{
	CHAR_DATA *fch;
	CHAR_DATA *fch_next;

	if (ch->master != NULL)
		stop_follower(ch);

	ch->leader = NULL;

	for (fch = char_list; fch != NULL; fch = fch_next) {
		fch_next = fch->next;
		if (IS_NPC(fch) && IS_SET(fch->pMobIndex->act, ACT_FAMILIAR))
			continue;
		if (fch->master == ch)
			stop_follower(fch);
		if (fch->leader == ch)
			fch->leader = NULL;
	}
}

CHAR_DATA* leader_lookup(CHAR_DATA* ch)
{
	CHAR_DATA* res;
	for (res = ch; res->leader != NULL; res = res->leader)
		;
	return res;
}

void do_who_raw(CHAR_DATA* ch, CHAR_DATA *wch, BUFFER* output)
{
	clan_t *clan;
	class_t *cl;
	race_t *r;

	if ((cl = class_lookup(wch->class)) == NULL
	||  (r = race_lookup(wch->race)) == NULL
	||  !r->race_pcdata)
		return;

	buf_append(output, "{x");
	if (ch) {
		if (IS_IMMORTAL(ch)) {
			buf_printf(output, BUF_END, 
				   "[%3d %5.5s %3.3s] ",	// notrans
				   wch->level,
				   r->race_pcdata->who_name,
				   cl->who_name);
		} else {
			if (!IS_IMMORTAL(wch) && in_PK(ch, wch))
				buf_append(output, "{r[{RPK{r]{x ");
			else
				buf_append(output, "     ");	// notrans
		}
	}

	if (wch->level >= LEVEL_HERO) {
		buf_append(output, "[{G");		// notrans
		switch (wch->level) {
		case LEVEL_IMP:
			buf_append(output, "IMP");	// notrans
			break;
		case LEVEL_CRE:
			buf_append(output, "CRE");	// notrans
			break;
		case LEVEL_DEI:
			buf_append(output, "DEI");	// notrans
			break;
		case LEVEL_GOD:
			buf_append(output, "GOD");	// notrans
			break;
		case LEVEL_AVA:
			buf_append(output, "AVA");	// notrans
			break;
		case LEVEL_HERO:
			buf_append(output, "HERO");	// notrans
			break;
		}
		buf_append(output, "{x] ");		// notrans
	}

	if ((clan = clan_lookup(wch->clan)) != NULL
	&&  (!IS_SET(clan->clan_flags, CLAN_HIDDEN) ||
	     (ch && (IS_CLAN(wch->clan, ch->clan) || IS_IMMORTAL(ch)))))
		buf_printf(output, BUF_END, "[{c%s{x] ", clan->name); // notrans

	if (IS_SET(wch->comm, COMM_AFK))
		buf_append(output, "{c[AFK]{x ");

	if (wch->invis_level)
		buf_append(output, "[{WWizi{x] ");
	if (wch->incog_level)
		buf_append(output, "[{DIncog{x] ");

	if (IS_WANTED(wch))
		buf_append(output, "{R(WANTED){x ");

	if (IS_IMMORTAL(wch))
		buf_printf(output, BUF_END, "{W%s{x", wch->name);  // notrans
	else
		buf_append(output, wch->name);

	buf_append(output, PC(wch)->title);
	buf_append(output, "\n");
}

static int movement_loss[MAX_SECT+1] =
{
	1, 2, 2, 3, 4, 6, 4, 1, 12, 10, 6
};

int mount_success(CHAR_DATA *ch, CHAR_DATA *mount, int canattack)
{
	int	percent;
	int	success;
	int	chance;

	if ((chance = get_skill(ch, "riding")) == 0)
		return FALSE;

	percent = number_percent() + (ch->level < mount->level ?
		  (mount->level - ch->level) * 3 :
		  (mount->level - ch->level) * 2);

	if (!ch->fighting)
		percent -= 25;

	if (!IS_NPC(ch) && IS_DRUNK(ch)) {
		percent += chance / 2;
		act_char("Due to your being under the influence, riding seems a bit harder...", ch);
	}

	success = percent - chance;

	if (success <= 0) { /* Success */
		check_improve(ch, "riding", TRUE, 1);
		return TRUE;
	}

	check_improve(ch, "riding", FALSE, 1);
	if (success >= 10 && MOUNTED(ch) == mount) {
		act_puts("You lose control and fall off of $N.",
			 ch, NULL, mount, TO_CHAR, POS_DEAD);
		act("$n loses control and falls off of $N.",
		    ch, NULL, mount, TO_NOTVICT);
		act_puts("$n loses control and falls off of you.",
			 ch, NULL, mount, TO_VICT, POS_SLEEPING);

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

		act_puts("$N snarls and attacks you!",
			 mount, NULL, ch, TO_VICT, POS_DEAD);
		act("$N snarls and attacks $n!",
		    mount, NULL, ch, TO_NOTVICT);
		act_puts("You snarl and attack $n!",
			 mount, NULL, ch, TO_CHAR, POS_SLEEPING);

		damage(mount, ch, number_range(1, mount->level),
			"kick", DAM_BASH, DAMF_SHOW);
	}
	return FALSE;
}

void move_char(CHAR_DATA *ch, int door, bool follow)
{
	move_char_org(ch, door, follow, FALSE);
}

inline bool has_boat(CHAR_DATA *ch)
{
	OBJ_DATA *obj;
	bool found;

	found = FALSE;

	if (IS_IMMORTAL(ch))
		return TRUE;

	for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
		if (obj->item_type == ITEM_BOAT) {
			found = TRUE;
			break;
		}

	return found;
}

#define CAN_SWIM(ch)	(IS_IMMORTAL(ch) || number_percent() < get_skill(ch, "swimming"))
#define CAN_FLY(ch)	(IS_IMMORTAL(ch) || IS_AFFECTED(ch, AFF_FLYING))

bool move_char_org(CHAR_DATA *ch, int door, bool follow, bool is_charge)
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
	AFFECT_DATA *paf;

	if (RIDDEN(ch) && !IS_NPC(ch->mount))
		return move_char_org(ch->mount,door,follow,is_charge);

	if ((paf = is_affected(ch, "entanglement"))
	&& (INT(paf->location) == APPLY_DEX)) {
		act("You aren't able to move.",
			ch, NULL, NULL, TO_CHAR);
		return FALSE;
	}

	if (IS_AFFECTED(ch, AFF_WEB)
	|| (MOUNTED(ch) && IS_AFFECTED(ch->mount, AFF_WEB))) {
		WAIT_STATE(ch, get_pulse("violence"));
		if (number_percent() < str_app[IS_NPC(ch) ?
			20 : get_curr_stat(ch,STAT_STR)].tohit * 5) {
			affect_strip(ch, "web");
			act_puts("When you attempt to leave the room, you "
				 "break the webs holding you tight.",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
			act_puts("$n struggles against the webs which hold $m "
				 "in place, and break it.",
				 ch, NULL, NULL, TO_ROOM, POS_RESTING);
		} else {
			act_puts("You attempt to leave the room, but the webs "
				 "hold you tight.",
				 ch, NULL, NULL, TO_ROOM, POS_DEAD);
			act("$n struggles vainly against the webs which "
			    "hold $m in place.",
			    ch, NULL, NULL, TO_ROOM);
			return FALSE;
		}
	}

	for (fch = ch->in_room->people; fch; fch = fch->next_in_room) {
		if (IS_NPC(fch)
		&&  NPC(fch)->target == ch
		&&  fch->pMobIndex->vnum == MOB_VNUM_SHADOW) {
			act_puts("You attempt to leave your shadow alone,"
				 " but fail.",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
			return FALSE;
		}
	}

	if (door < 0 || door >= MAX_DIR) {
		log(LOG_BUG, "move_char_org: bad door %d.", door);
		return FALSE;
	}

	if (HAS_INVIS(ch, ID_HIDDEN | ID_FADE)
	&&  !HAS_INVIS(ch, ID_SNEAK)) {
		REMOVE_INVIS(ch, ID_HIDDEN | ID_FADE);
		act_puts("You step out of shadows.",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		act_puts("$n steps out of shadows.",
			 ch, NULL, NULL, TO_ROOM, POS_RESTING);
	}

	if (is_affected(ch, "globe of invulnerability")) {
		affect_strip(ch, "globe of invulnerability");
		act_char("You destroy your globe of invulnerability.", ch);
	}

	if (HAS_INVIS(ch, ID_CAMOUFLAGE))  {
		if (number_percent() < get_skill(ch, "camouflage move"))
			check_improve(ch, "camouflage move", TRUE, 5);
		else {
			REMOVE_INVIS(ch, ID_CAMOUFLAGE);
			act_puts("You step out from your cover.",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
			act("$n steps out from $s cover.",
			    ch, NULL, NULL, TO_ROOM);
			check_improve(ch, "camouflage move", FALSE, 5);
		}
	}

	if (HAS_INVIS(ch, ID_BLEND)) {
		REMOVE_INVIS(ch, ID_BLEND);
		affect_bit_strip(ch, TO_INVIS, ID_BLEND);
		act_puts("You step out from your cover.",
			ch, NULL, NULL, TO_CHAR, POS_DEAD);
		act("$n steps out from $m's cover.",
			ch, NULL, NULL, TO_ROOM);
	}

	/*
	 * Exit trigger, if activated, bail out. Only PCs are triggered.
	 */
	if (!IS_NPC(ch) && mp_exit_trigger(ch, door))
		return FALSE;

	in_room = ch->in_room;
	if ((pexit = in_room->exit[door]) == NULL
	||  (to_room = pexit->to_room.r) == NULL
	||  !can_see_room(ch, pexit->to_room.r)) {
		act_char("Alas, you cannot go that way.", ch);
		return FALSE;
	}

	if (ch->size > pexit->size && !IS_IMMORTAL(ch)) {
		act_puts("$d is too narrow for you to pass.",
			ch, &pexit->short_descr, NULL, TO_CHAR, POS_DEAD);
		act("$n tries to leave through $d, but almost stucks there.",
		    ch, &pexit->short_descr, NULL, TO_ROOM);
		return FALSE;
	}

	if (IS_AFFECTED(in_room, RAFF_RANDOMIZER) && !is_charge) {
		int d0;
		while (1) {
			d0 = number_range(0, MAX_DIR-1);
			if ((pexit = in_room->exit[d0]) == NULL
			||  (to_room = pexit->to_room.r) == NULL
			||  !can_see_room(ch, pexit->to_room.r))
				continue;
			door = d0;
			break;
		}
	}

	if (IS_SET(pexit->exit_info, EX_CLOSED)
	&&  (!IS_AFFECTED(ch, AFF_PASS_DOOR) ||
	     IS_SET(pexit->exit_info, EX_NOPASS))
	&&  !IS_TRUSTED(ch, LEVEL_IMMORTAL)) {
		if (IS_AFFECTED(ch, AFF_PASS_DOOR)
		&&  IS_SET(pexit->exit_info, EX_NOPASS)) {
			act_puts("You failed to pass through $d.",
				 ch, &pexit->short_descr, NULL,
				 TO_CHAR, POS_DEAD);
			act("$n tries to pass through $d, but $e fails.",
			    ch, &pexit->short_descr, NULL, TO_ROOM);
		} else {
			act_puts("$d is closed.",
				 ch, &pexit->short_descr, NULL,
				 TO_CHAR, POS_DEAD);
		}
		return FALSE;
	}

	if (IS_AFFECTED(ch, AFF_CHARM)
	&&  ch->master != NULL
	&&  in_room == ch->master->in_room) {
		act("What? And leave your beloved master?",
		    ch, NULL, ch->master, TO_CHAR);
		return FALSE;
	}

	if (room_is_private(to_room)) {
		act_char("That room is private right now.", ch);
		return FALSE;
	}

	if (MOUNTED(ch)) {
		if (MOUNTED(ch)->position < POS_FIGHTING) {
			act_char("Your mount must be standing.", ch);
			return FALSE;
		}
		if (!mount_success(ch, MOUNTED(ch), FALSE)) {
			act_char("Your mount subbornly refuses to go that way.", ch);
			return FALSE;
		}
	}

	if (!IS_NPC(ch)) {
		int move;
		int can_carry;

		if (!IS_IMMORTAL(ch)) {
			if (IS_SET(to_room->room_flags, ROOM_GUILD)
			&&  !guild_ok(ch, to_room)) {
				act_char("You aren't allowed in there.", ch);
				return FALSE;
			}

			if (IS_PUMPED(ch)
			&&  IS_SET(to_room->room_flags, ROOM_PEACE | ROOM_GUILD)
			&&  !IS_SET(in_room->room_flags,
				    ROOM_PEACE | ROOM_GUILD)) {
				act_puts("You feel too bloody to go in there now.",
					 ch, NULL, NULL, TO_CHAR, POS_DEAD);
				return FALSE;
			}
		}

		if (in_room->sector_type == SECT_AIR ||
		    to_room->sector_type == SECT_AIR) {
			if (MOUNTED(ch)) {
				if (!CAN_FLY(MOUNTED(ch))) {
					act_char("You mount can't fly.", ch);
					return FALSE;
				}
			} else {
				if (!CAN_FLY(ch)) {
					act_char("You can't fly.", ch);
					return FALSE;
				}
			}
		}

		if (in_room->sector_type == SECT_WATER_SWIM ||
		    to_room->sector_type == SECT_WATER_SWIM) {
			if (MOUNTED(ch)) {
				if (!CAN_FLY(MOUNTED(ch)) && !CAN_SWIM(MOUNTED(ch))) {
					act_char("Your mount can neither fly nor swim.", ch);
					return FALSE;
				}
			} else {
				if (!CAN_FLY(ch) && !CAN_SWIM(ch) && !has_boat(ch)) {
					act_char("Learn to swim or buy a boat.", ch);
					return FALSE;
				}
			}
		}

		if (in_room->sector_type == SECT_WATER_NOSWIM ||
		    to_room->sector_type == SECT_WATER_NOSWIM) {
			if (MOUNTED(ch)) {
				if (!CAN_FLY(MOUNTED(ch))) {
					act_char("Your mount can't fly.", ch);
					return FALSE;
				}
			} else {
				if (!CAN_FLY(ch) && !has_boat(ch)) {
					act_char("Learn to fly or buy a boat.", ch);
					return FALSE;
				}
			}
		}

		if (in_room->sector_type == SECT_UNDERWATER
		||  to_room->sector_type == SECT_UNDERWATER) {
			if (MOUNTED(ch)
			&& (!CAN_SWIM(MOUNTED(ch))
			    || !is_affected(MOUNTED(ch), "water breathing"))) {
				act_char("Your mount refuses to dive.", ch);
				return FALSE;
			}
			if (!CAN_SWIM(ch)) {
				act_char("You can't swim.", ch);
				return FALSE;
			}
		}

		move = (movement_loss[URANGE(0, in_room->sector_type, MAX_SECT)]
		  + movement_loss[URANGE(0, to_room->sector_type, MAX_SECT)])/2;

		if (IS_AFFECTED(ch, AFF_FLYING)
		||  IS_AFFECTED(ch, AFF_HASTE))
			move /= 2;

		if ((can_carry = can_carry_w(ch)) >= 0) {
			if (ch->carry_weight > can_carry * 3 / 4) {
				act_puts("You're carrying too much to go further.",
				    ch, NULL, NULL, TO_CHAR, POS_DEAD);
				return FALSE;
			} else if (ch->carry_weight > can_carry * 2 / 3) {
				/* Overburdened much */
				move *= 2;
			} else if (ch->carry_weight > can_carry / 2) {
				/* slightly overburdened */
				move = move * 3 / 2;
			}
		}

		if (IS_AFFECTED(ch, AFF_SLOW))
			move *= 2;

		if (!MOUNTED(ch)) {
			int wait;

			if (ch->move < move) {
				act_puts("You are too exhausted.",
					 ch, NULL, NULL, TO_CHAR, POS_DEAD);
				return FALSE;
			}

			ch->move -= move;

			if (!IS_NPC(ch))
				PC(ch)->move_used += move;

			if (ch->in_room->sector_type == SECT_DESERT
			||  IS_WATER(ch->in_room))
				wait = 2;
			else
				wait = 1;

			if (IS_AFFECTED(ch, AFF_SLOW)) wait *= 2;
			if (ch->carry_weight > can_carry * 2 / 3)
				wait *= 2;

			WAIT_STATE(ch, wait);
		}
	}

	if (!HAS_INVIS(ch, ID_SNEAK | ID_CAMOUFLAGE)
	&&  ch->invis_level < LEVEL_HERO)
		act_flags = TO_ROOM;
	else
		act_flags = TO_ROOM | ACT_NOMORTAL;

	if (!IS_NPC(ch)
	&&  ch->in_room->sector_type != SECT_INSIDE
	&&  ch->in_room->sector_type != SECT_CITY
	&&  number_percent() < get_skill(ch, "quiet movement")
	&&  !is_charge) {
		act(MOUNTED(ch) ? "$n leaves, riding on $N." : "$n leaves.",
		    ch, NULL, MOUNTED(ch), act_flags);
		check_improve(ch, "quiet movement", TRUE, 1);
	}
	else if (is_charge) {
		act("$n spurs $s $N, leaving $t.", ch,
		dir_name[is_affected(ch, "misleading")
			? number_range(0, 5) : door],
		ch->mount,  TO_ROOM);
	}
	else {
		act(MOUNTED(ch) ? "$n leaves $t, riding on $N." :
				  "$n leaves $t.", ch,
		dir_name[is_affected(ch, "misleading") ?
			number_range(0, 5) : door],
		MOUNTED(ch), act_flags);
	}

	if (HAS_INVIS(ch, ID_CAMOUFLAGE)
	&&  to_room->sector_type != SECT_FOREST
	&&  to_room->sector_type != SECT_MOUNTAIN
	&&  to_room->sector_type != SECT_HILLS) {
		REMOVE_INVIS(ch, ID_CAMOUFLAGE);
		act_puts("You step out from your cover.",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		act("$n steps out from $m's cover.",
		    ch, NULL, NULL, TO_ROOM);
	}

	/* room record for tracking */
	if (!IS_NPC(ch))
		room_record(ch->name, in_room, door);

	/*
	 * now, after all the checks are done we should
	 * - take the char from the room
	 * - print the message to chars in to_room about ch arrival
	 * - put the char to to_room
	 * - CHECK THAT CHAR IS NOT DEAD after char_to_room
	 * - move all the followers and pull all the triggers
	 */
	mount = MOUNTED(ch);
	char_from_room(ch);

	if (!HAS_INVIS(ch, ID_SNEAK) && ch->invis_level < LEVEL_HERO)
		act_flags = TO_ALL;
	else
		act_flags = TO_ALL | ACT_NOMORTAL;

	if (!is_charge)
		act(mount ? "$i has arrived, riding $N." : "$i has arrived.",
		    to_room->people, ch, mount, act_flags);

	char_to_room(ch, to_room);

	if (mount) {
		char_from_room(mount);
		char_to_room(mount, to_room);
		ch->riding = TRUE;
		mount->riding = TRUE;
	}

	if (!IS_EXTRACTED(ch))
		dofun("look", ch, "auto");

	if (in_room == to_room) /* no circular follows */
		return TRUE;

	/*
	 * move all the followers
	 */
	for (fch = in_room->people; fch; fch = fch_next) {
		fch_next = fch->next_in_room;

		if (fch->master != ch || fch->position != POS_STANDING
		||  !can_see_room(fch, to_room))
			continue;

		if (IS_SET(to_room->room_flags, ROOM_LAW)
		&&  IS_NPC(fch)
		&&  IS_SET(fch->pMobIndex->act, ACT_AGGRESSIVE)) {
			act_puts("You can't bring $N into the city.",
				 ch, NULL, fch, TO_CHAR, POS_DEAD);
			act("You aren't allowed in the city.",
			    fch, NULL, NULL, TO_CHAR);
			continue;
		}

		act_puts("You follow $N.", fch, NULL, ch, TO_CHAR, POS_DEAD);
		move_char(fch, door, TRUE);
	}

	if (IS_EXTRACTED(ch))
		return TRUE;

	room_has_pc = FALSE;
	for (fch = to_room->people; fch != NULL; fch = fch_next) {
		fch_next = fch->next_in_room;
		if (!IS_NPC(fch)) {
			room_has_pc = TRUE;
			break;
		}
	}

	if (!room_has_pc)
		return TRUE;

	/*
	 * pull GREET and ENTRY triggers
	 *
	 * if someone is following the char, these triggers get activated
	 * for the followers before the char, but it's safer this way...
	 */
	for (fch = to_room->people; fch; fch = fch_next) {
		fch_next = fch->next_in_room;

		/* greet progs for items carried by people in room */
		for (obj = fch->carrying; obj; obj = obj_next) {
			obj_next = obj->next_content;
			oprog_call(OPROG_GREET, obj, ch, NULL);
		}
	}

	for (obj = ch->in_room->contents; obj != NULL; obj = obj_next) {
		obj_next = obj->next_content;
		oprog_call(OPROG_GREET, obj, ch, NULL);
	}

	if (!IS_NPC(ch))
		mp_greet_trigger(ch);

	for (obj = ch->carrying; obj; obj = obj_next) {
		obj_next = obj->next_content;
		oprog_call(OPROG_ENTRY, obj, NULL, NULL);
	}

	if (IS_NPC(ch) && HAS_TRIGGER(ch, TRIG_ENTRY))
		mp_percent_trigger(ch, NULL, NULL, NULL, TRIG_ENTRY);

	return TRUE;
}

int
door_lookup(CHAR_DATA *ch, const char *arg)
{
	int door;

	     if (!str_cmp(arg, "n") || !str_cmp(arg, "north")) return 0;
	else if (!str_cmp(arg, "e") || !str_cmp(arg, "east" )) return 1;
	else if (!str_cmp(arg, "s") || !str_cmp(arg, "south")) return 2;
	else if (!str_cmp(arg, "w") || !str_cmp(arg, "west" )) return 3;
	else if (!str_cmp(arg, "u") || !str_cmp(arg, "up"   )) return 4;
	else if (!str_cmp(arg, "d") || !str_cmp(arg, "down" )) return 5;

	for (door = 0; door < MAX_DIR; door++) {
		EXIT_DATA *pexit;

		if ((pexit = ch->in_room->exit[door]) != NULL
		&&  IS_SET(pexit->exit_info, EX_ISDOOR)
		&&  is_name(arg, pexit->keyword))
			return door;
	}

	act_char("You don't see that here.", ch);
	return -1;
}

int find_door(CHAR_DATA *ch, const char *arg)
{
	EXIT_DATA *pexit;
	int door;

	if ((door = door_lookup(ch, arg)) < 0)
		return door;

	/*
	 * the following two if's can be TRUE only
	 * if direction was matched in `door_lookup'
	 */

	if ((pexit = ch->in_room->exit[door]) == NULL) {
		act_puts("I see no door $T here.",
			 ch, NULL, dir_name[door], TO_CHAR, POS_DEAD);
		return -1;
	}

	/* 'look direction' */
	if (!IS_SET(pexit->exit_info, EX_ISDOOR)) {
		act_char("You can't do that.", ch);
		return -1;
	}

	return door;
}

/* RT part of the corpse looting code */
bool can_loot(CHAR_DATA * ch, OBJ_DATA * obj)
{
	if (IS_IMMORTAL(ch))
		return TRUE;

	/*
	 * PC corpses in the ROOM_BATTLE_ARENA rooms can be looted
	 * only by owners
	 */
	if (obj->in_room
	&&  IS_SET(obj->in_room->room_flags, ROOM_BATTLE_ARENA)
	&&  !IS_OWNER(ch, obj))
		return FALSE;

	return TRUE;
}

int need_hands(CHAR_DATA *ch, OBJ_DATA *weapon)
{
	int need = 1;

	if (WEAPON_IS_LONG(weapon)
	||  (IS_WEAPON_STAT(weapon, WEAPON_TWO_HANDS) &&
	     ch->size < SIZE_LARGE))
		need++;

	return need;
}

int free_hands(CHAR_DATA *ch)
{
	int hands = 2;
	OBJ_DATA *weapon;

	weapon = get_eq_char(ch, WEAR_WIELD);
	if (weapon)
		hands -= need_hands(ch, weapon);

	if (get_eq_char(ch, WEAR_SECOND_WIELD))
		hands--;

	if (get_eq_char(ch, WEAR_SHIELD))
		hands--;

	if (get_eq_char(ch, WEAR_HOLD))
		hands--;

	return UMAX(0, hands);
}

void get_obj(CHAR_DATA * ch, OBJ_DATA * obj, OBJ_DATA * container,
	     const char *msg_others)
{
	/* variables for AUTOSPLIT */
	CHAR_DATA      *gch;
	int             members;
	int		carry_w, carry_n;

	if (!CAN_WEAR(obj, ITEM_TAKE)
	||  (obj->item_type == ITEM_CORPSE_PC &&
	     obj->in_room &&
	     IS_SET(obj->in_room->room_flags, ROOM_BATTLE_ARENA) &&
	     !IS_OWNER(ch, obj))) {
		act("You can't take $p.", ch, obj, NULL, TO_CHAR);
		return;
	}

	if ((carry_n = can_carry_n(ch)) >= 0
	&&  ch->carry_number + get_obj_number(obj) > carry_n) {
		act_puts("$P: you can't carry that many items.",
			 ch, NULL, obj, TO_CHAR, POS_DEAD);
		return;
	}

	if ((carry_w = can_carry_w(ch)) >= 0
	&&  get_carry_weight(ch) + get_obj_weight(obj) > carry_w) {
		act_puts("$P: you can't carry that much weight.",
			 ch, NULL, obj, TO_CHAR, POS_DEAD);
		return;
	}

	if (obj->in_room != NULL) {
		for (gch = obj->in_room->people; gch != NULL;
		     gch = gch->next_in_room) {
			if (gch->on == obj) {
				act_puts("$N appears to be using $p.",
					 ch, obj, gch, TO_CHAR, POS_DEAD);
				return;
			}
		}
	}

	if (obj->item_type == ITEM_MONEY) {
		if (carry_w >= 0
		&&  get_carry_weight(ch) + MONEY_WEIGHT(obj) > carry_w) {
			act_puts("$P: you can't carry that much weight.",
				 ch, NULL, obj, TO_CHAR, POS_DEAD);
			return;
		}
	}

	if (container) {
		if (OBJ_IS(container, OBJ_PIT)
		&&  !IS_OBJ_STAT(obj, ITEM_HAD_TIMER))
			obj->timer = 0;
		REMOVE_OBJ_STAT(obj, ITEM_HAD_TIMER);

		/*
		 * this cases differ only in order of parameters
		 * this is needed for more correct translation
		 */
		if (IS_SET(INT(container->value[1]), CONT_PUT_ON)) {
			act_puts("You get $P from $p.",
				 ch, container, obj, TO_CHAR, POS_DEAD);
			act(msg_others == NULL ?
			    "$n gets $P from $p." : msg_others,
			    ch, container, obj,
			    TO_ROOM | (HAS_INVIS(ch, ID_SNEAK) ? ACT_NOMORTAL : 0));
		} else {
			act_puts("You get $p from $P.",
				 ch, obj, container, TO_CHAR, POS_DEAD);
			act(msg_others == NULL ?
			    "$n gets $p from $P." : msg_others,
			    ch, obj, container,
			    TO_ROOM | (HAS_INVIS(ch, ID_SNEAK) ? ACT_NOMORTAL : 0));
		}

		obj_from_obj(obj);
	} else {
		act_puts("You get $p.", ch, obj, container, TO_CHAR, POS_DEAD);
		act(msg_others == NULL ? "$n gets $p." : msg_others,
		    ch, obj, container,
		    TO_ROOM | (HAS_INVIS(ch, ID_SNEAK) ? ACT_NOMORTAL : 0));

		obj_from_room(obj);
	}

	if (obj->item_type == ITEM_MONEY) {
		ch->silver += INT(obj->value[0]);
		ch->gold += INT(obj->value[1]);
		if (!IS_NPC(ch)
		&&  IS_SET(PC(ch)->plr_flags, PLR_AUTOSPLIT)) {
			/* AUTOSPLIT code */
			members = 0;
			for (gch = ch->in_room->people; gch != NULL;
			     gch = gch->next_in_room) {
				if (!IS_AFFECTED(gch, AFF_CHARM)
				&&  is_same_group(gch, ch))
					members++;
			}

			if (members > 1
			&&  (INT(obj->value[0]) > 1 ||
			     INT(obj->value[1]))) {
				dofun("split", ch,
				      "%d %d", obj->value[0],	// notrans
				      obj->value[1]);
			}
		}
		extract_obj(obj, 0);
	} else {
		obj_to_char(obj, ch);
		oprog_call(OPROG_GET, obj, ch, NULL);
	}
}

void quaff_obj(CHAR_DATA *ch, OBJ_DATA *obj)
{
	OBJ_DATA *vial;

	act("$n quaffs $p.", ch, obj, NULL, TO_ROOM);
	act_puts("You quaff $p.", ch, obj, NULL, TO_CHAR, POS_DEAD);

	if (IS_PUMPED(ch) || ch->fighting != NULL)
		WAIT_STATE(ch, 2 * get_pulse("violence"));

	vial = create_obj(get_obj_index(OBJ_VNUM_POTION_VIAL), 0);
	vial->label = str_qdup(obj->label);
	obj_to_char(vial, ch);

	obj_cast_spell(obj->value[1].s, INT(obj->value[0]), ch, ch);

	if (!IS_EXTRACTED(ch))
		obj_cast_spell(obj->value[2].s, INT(obj->value[0]), ch, ch);

	if (!IS_EXTRACTED(ch))
		obj_cast_spell(obj->value[3].s, INT(obj->value[0]), ch, ch);

	if (!IS_EXTRACTED(ch))
		obj_cast_spell(obj->value[4].s, INT(obj->value[0]), ch, ch);

	extract_obj(obj, 0);
}

/*
 * Remove an object.
 */
bool remove_obj(CHAR_DATA * ch, int iWear, bool fReplace)
{
	OBJ_DATA       *obj;
	if ((obj = get_eq_char(ch, iWear)) == NULL)
		return TRUE;

	if (!fReplace)
		return FALSE;

	if (ch->shapeform) {
		act("You cannot reach your items.", ch, NULL, NULL, TO_CHAR);
		return FALSE;
	}

	if (IS_OBJ_STAT(obj, ITEM_NOREMOVE)) {
		act_puts("You can't remove $p.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		return FALSE;
	}

	if (obj->item_type == ITEM_TATTOO && !IS_IMMORTAL(ch)) {
		act_puts("You must scratch it to remove $p.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		return FALSE;
	}

#if 0
	/* XXX should be moved to do_remove */
	if (iWear == WEAR_STUCK_IN) {
		const char *wsn = get_weapon_sn(obj);
		unequip_char(ch, obj);

		if (obj->pObjIndex->item_type == ITEM_WEAPON
		&&  get_stuck_eq(ch, INT(obj->value[0])) == NULL)
			if (is_affected(ch, wsn))
				affect_strip(ch, wsn);

		act_puts("You remove $p, in pain.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		act("$n removes $p, in pain.", ch, obj, NULL, TO_ROOM);
                damage(ch, ch, dice(obj->level, 12),
		       NULL, DAM_OTHER, DAMF_NONE);
                WAIT_STATE(ch, 4);
		return TRUE;
	}
#endif

	unequip_char(ch, obj);
	act("$n stops using $p.", ch, obj, NULL, TO_ROOM);
	act_puts("You stop using $p.", ch, obj, NULL, TO_CHAR, POS_DEAD);

	if (iWear == WEAR_WIELD
	&&  (obj = get_eq_char(ch, WEAR_SECOND_WIELD)) != NULL) {
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
void wear_obj(CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace)
{
	if (cc_vexpr_check(&obj->pObjIndex->restrictions,
	    "obj_wear", ch)) {					// notrans
		act("You can't use $p.", ch, obj, NULL, TO_CHAR);
		return;
	}

	if (IS_NPC(ch)
	&&  (!IS_SET(ch->form, FORM_BIPED) ||
	     !IS_SET(ch->form, FORM_SENTIENT))) {
		act("WEAR ?", ch, NULL, NULL, TO_CHAR);
		return;
	}

	if (ch->shapeform) {
		act("You cannot reach your items.",
			ch, NULL, NULL, TO_CHAR);
		return;
	}

	if (obj->item_type == ITEM_LIGHT) {
		if (!remove_obj(ch, WEAR_LIGHT, fReplace))
			return;
		act("$n lights $p and holds it.", ch, obj, NULL, TO_ROOM);
		act_puts("You light $p and hold it.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		equip_char(ch, obj, WEAR_LIGHT);
		return;
	}

	if (CAN_WEAR(obj, ITEM_WEAR_FINGER)) {
		if (get_eq_char(ch, WEAR_FINGER_L) != NULL
		&&  get_eq_char(ch, WEAR_FINGER_R) != NULL
		&&  !remove_obj(ch, WEAR_FINGER_L, fReplace)
		&&  !remove_obj(ch, WEAR_FINGER_R, fReplace))
			return;

		if (get_eq_char(ch, WEAR_FINGER_L) == NULL) {
			act("$n wears $p on $s left finger.",
			    ch, obj, NULL, TO_ROOM);
			act_puts("You wear $p on your left finger.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			equip_char(ch, obj, WEAR_FINGER_L);
			return;
		}

		if (get_eq_char(ch, WEAR_FINGER_R) == NULL) {
			act("$n wears $p on $s right finger.",
			    ch, obj, NULL, TO_ROOM);
			act_puts("You wear $p on your right finger.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			equip_char(ch, obj, WEAR_FINGER_R);
			return;
		}

		log(LOG_BUG, "wear_obj: no free finger");
		act_char("You already wear two rings.", ch);
		return;
	}

	if (CAN_WEAR(obj, ITEM_WEAR_NECK)) {
		if (get_eq_char(ch, WEAR_NECK) != NULL
		&&  !remove_obj(ch, WEAR_NECK, fReplace))
			return;

		if (get_eq_char(ch, WEAR_NECK) == NULL) {
			act("$n wears $p around $s neck.",
			    ch, obj, NULL, TO_ROOM);
			act_puts("You wear $p around your neck.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			equip_char(ch, obj, WEAR_NECK);
			return;
		}

	}

	if (CAN_WEAR(obj, ITEM_WEAR_FACE)) {
		if (!remove_obj(ch, WEAR_FACE, fReplace))
			return;

		act("$n wears $p on $s face.", ch, obj, NULL, TO_ROOM);
		act_puts("You wear $p on your face.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		equip_char(ch, obj, WEAR_FACE);
		return;
	}

	if (CAN_WEAR(obj, ITEM_WEAR_BODY)) {
		if (!remove_obj(ch, WEAR_BODY, fReplace))
			return;

		act("$n wears $p on $s torso.", ch, obj, NULL, TO_ROOM);
		act_puts("You wear $p on your torso.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		equip_char(ch, obj, WEAR_BODY);
		return;
	}

	if (CAN_WEAR(obj, ITEM_WEAR_HEAD)) {
		if (!remove_obj(ch, WEAR_HEAD, fReplace))
			return;

		act("$n wears $p on $s head.", ch, obj, NULL, TO_ROOM);
		act_puts("You wear $p on your head.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		equip_char(ch, obj, WEAR_HEAD);
		return;
	}

	if (CAN_WEAR(obj, ITEM_WEAR_LEGS)) {
		if (!remove_obj(ch, WEAR_LEGS, fReplace))
			return;

		act("$n wears $p on $s legs.", ch, obj, NULL, TO_ROOM);
		act_puts("You wear $p on your legs.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		equip_char(ch, obj, WEAR_LEGS);
		return;
	}

	if (CAN_WEAR(obj, ITEM_WEAR_FEET)) {
		if (!remove_obj(ch, WEAR_FEET, fReplace))
			return;

		act("$n wears $p on $s feet.", ch, obj, NULL, TO_ROOM);
		act_puts("You wear $p on your feet.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		equip_char(ch, obj, WEAR_FEET);
		return;
	}

	if (CAN_WEAR(obj, ITEM_WEAR_HANDS)) {
		if (!remove_obj(ch, WEAR_HANDS, fReplace))
			return;

		act("$n wears $p on $s hands.", ch, obj, NULL, TO_ROOM);
		act_puts("You wear $p on your hands.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		equip_char(ch, obj, WEAR_HANDS);
		return;
	}

	if (CAN_WEAR(obj, ITEM_WEAR_ARMS)) {
		if (!remove_obj(ch, WEAR_ARMS, fReplace))
			return;

		act("$n wears $p on $s arms.", ch, obj, NULL, TO_ROOM);
		act_puts("You wear $p on your arms.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		equip_char(ch, obj, WEAR_ARMS);
		return;
	}

	if (CAN_WEAR(obj, ITEM_WEAR_ABOUT)) {
		if (!remove_obj(ch, WEAR_ABOUT, fReplace))
			return;

		act("$n wears $p on $s torso.", ch, obj, NULL, TO_ROOM);
		act_puts("You wear $p on your torso.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		equip_char(ch, obj, WEAR_ABOUT);
		return;
	}

	if (CAN_WEAR(obj, ITEM_WEAR_WAIST)) {
		if (!remove_obj(ch, WEAR_WAIST, fReplace))
			return;

		act("$n wears $p about $s waist.", ch, obj, NULL, TO_ROOM);
		act_puts("You wear $p about your waist.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		equip_char(ch, obj, WEAR_WAIST);
		return;
	}

	if (CAN_WEAR(obj, ITEM_WEAR_WRIST)) {
		if (get_eq_char(ch, WEAR_WRIST_L) != NULL
		&&  get_eq_char(ch, WEAR_WRIST_R) != NULL
		&&  !remove_obj(ch, WEAR_WRIST_L, fReplace)
		&&  !remove_obj(ch, WEAR_WRIST_R, fReplace))
			return;

		if (get_eq_char(ch, WEAR_WRIST_L) == NULL) {
			act("$n wears $p around $s left wrist.",
			    ch, obj, NULL, TO_ROOM);
			act_puts("You wear $p around your left wrist.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			equip_char(ch, obj, WEAR_WRIST_L);
			return;
		}

		if (get_eq_char(ch, WEAR_WRIST_R) == NULL) {
			act("$n wears $p around $s right wrist.",
			    ch, obj, NULL, TO_ROOM);
			act_puts("You wear $p around your right wrist.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			equip_char(ch, obj, WEAR_WRIST_R);
			return;
		}

		log(LOG_BUG, "wear_obj: no free wrist");
		act_char("You already wear two wrist items.", ch);
		return;
	}

	if (CAN_WEAR(obj, ITEM_WEAR_SHIELD)) {
		if (!remove_obj(ch, WEAR_SHIELD, fReplace))
			return;

		if (!free_hands(ch)) {
			act_puts("Your hands are full.",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
			return;
		}

		act("$n wears $p as a shield.", ch, obj, NULL, TO_ROOM);
		act_puts("You wear $p as a shield.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		equip_char(ch, obj, WEAR_SHIELD);
		return;
	}

	if (CAN_WEAR(obj, ITEM_WIELD)) {
		int skill;
		OBJ_DATA *weapon;

		if (!IS_NPC(ch)
		&&  get_obj_weight(obj) > str_app[get_curr_stat(ch, STAT_STR)].wield) {
			act_char("It is too heavy for you to wield.", ch);
			return;
		}

		if (is_affected(ch, "crippled hands")) {
			act("Your crippled hands refuse to hold $p.",
				ch, obj, NULL, TO_CHAR);
			return;
		}

		if ((weapon = get_eq_char(ch, WEAR_WIELD))
		&&  free_hands(ch) - need_hands(ch, weapon) < 2) {
			act_char("You need two hands free for that weapon.", ch);
			return;
		}

		if ((weapon = get_eq_char(ch, WEAR_SECOND_WIELD)))
			unequip_char(ch, weapon);

		if (!remove_obj(ch, WEAR_WIELD, fReplace)) {
			if (weapon)
				equip_char(ch, weapon, WEAR_SECOND_WIELD);
			return;
		}

		act("$n wields $p.", ch, obj, NULL, TO_ROOM);
		act_puts("You wield $p.", ch, obj, NULL, TO_CHAR, POS_DEAD);
		obj = equip_char(ch, obj, WEAR_WIELD);
		if (weapon)
			equip_char(ch, weapon, WEAR_SECOND_WIELD);
		if (obj == NULL)
			return;

		skill = get_weapon_skill(ch, get_weapon_sn(obj));

		if (skill >= 100) {
			act_puts("$p feels like a part of you!",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
		} else if (skill > 85) {
			act_puts("You feel quite confident with $p.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
		} else if (skill > 70) {
			act_puts("You are skilled with $p.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
		} else if (skill > 50) {
			act_puts("Your skill with $p is adequate.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
		} else if (skill > 25) {
			act_puts("$p feels a little clumsy in your hands.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
		} else if (skill > 1) {
			act_puts("You fumble and almost drop $p.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
		} else {
			act_puts("You don't even know which end is up on $p.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
		}
		return;
	}

	if (CAN_WEAR(obj, ITEM_HOLD)) {
		if (!remove_obj(ch, WEAR_HOLD, fReplace))
			return;

		if (!free_hands(ch)) {
			act_puts("Your hands are full.",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
			return;
		}

		if (is_affected(ch, "crippled hands")) {
			act("Your crippled hands refuse to hold $p.",
				ch, obj, NULL, TO_CHAR);
			return;
		}

		act("$n holds $p in $s hand.", ch, obj, NULL, TO_ROOM);
		act_puts("You hold $p in your hand.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		equip_char(ch, obj, WEAR_HOLD);
		return;
	}

	if (CAN_WEAR(obj, ITEM_WEAR_FLOAT)) {
		if (!remove_obj(ch, WEAR_FLOAT, fReplace))
			return;

		act("$n releases $p to float next to $m.",
		    ch, obj, NULL, TO_ROOM);
		act_puts("You release $p and it floats next to you.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		equip_char(ch, obj, WEAR_FLOAT);
		return;
	}

	if (CAN_WEAR(obj, ITEM_WEAR_TATTOO) && IS_IMMORTAL(ch)) {
		if (!remove_obj(ch, WEAR_TATTOO, fReplace))
			return;

		act("$n now uses $p as tattoo of $s religion.",
		    ch, obj, NULL, TO_ROOM);
		act_puts("You now use $p as the tattoo of your religion.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		equip_char(ch, obj, WEAR_TATTOO);
		return;
	}

	if (CAN_WEAR(obj, ITEM_WEAR_CLANMARK)) {
		if (!remove_obj(ch, WEAR_CLANMARK, fReplace))
			return;

		act("$n now uses $p as $s clan mark.",
		    ch, obj, NULL, TO_ROOM);
		act_puts("You now use $p as your clan mark.",
		    ch, obj, NULL, TO_CHAR, POS_DEAD);
		equip_char(ch, obj, WEAR_CLANMARK);
		return;
	}

	if (fReplace) {
		act("You can't wear, wield or hold $p.",
		    ch, obj, NULL, TO_CHAR);
	}
}

void wiznet(const char *msg, CHAR_DATA *ch, const void *arg,
	    flag_t flag, flag_t flag_skip, int min_level)
{
	DESCRIPTOR_DATA *d;

	for (d = descriptor_list; d != NULL; d = d->next) {
		CHAR_DATA *vch = d->original ? d->original : d->character;

		if (d->connected != CON_PLAYING
		||  !vch
		||  vch->level < LEVEL_IMMORTAL
		||  !IS_SET(PC(vch)->wiznet, WIZ_ON)
		||  (flag && !IS_SET(PC(vch)->wiznet, flag))
		||  (flag_skip && IS_SET(PC(vch)->wiznet, flag_skip))
		||  vch->level < min_level
		||  vch == ch)
			continue;

		if (IS_SET(PC(vch)->wiznet, WIZ_PREFIX)) {
			act_puts("--> ", d->character,		// notrans
				 NULL, NULL, TO_CHAR | ACT_NOLF,
				 POS_DEAD);
		}
		act_puts(msg, d->character, arg, ch,
			 TO_CHAR | ACT_NOUCASE, POS_DEAD);
	}
}

ROOM_INDEX_DATA *find_location(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	clan_t *clan;
	ROOM_INDEX_DATA *room;

	if (is_number(argument))
		return get_room_index(atoi(argument));

	if ((clan = clan_search(argument)) != NULL
	&&  (room = get_room_index(clan->altar_vnum)) != NULL)
		return room;

	if ((victim = get_char_world(ch, argument)) != NULL)
		return victim->in_room;

	if ((obj = get_obj_world(ch, argument)) != NULL)
		return obj->in_room;

	return NULL;
}

void reboot_mud(void)
{
	DESCRIPTOR_DATA *d,*d_next;

	log(LOG_INFO, "Rebooting SoG");
	for (d = descriptor_list; d != NULL; d = d_next) {
		d_next = d->next;
		write_to_buffer(d,"SoG is going down for rebooting NOW!\n\r",0);
		close_descriptor(d, SAVE_F_REBOOT);
	}

	/*
	 * activate eqcheck on next boot
	 */
	if (!rebooter) {
		FILE *fp = dfopen(TMP_PATH, EQCHECK_FILE, "w");
		if (!fp) {
			log(LOG_ERROR,
			    "reboot_mud: unable to activate eqcheck");
		} else {
			log(LOG_INFO, "reboot_mud: eqcheck activated");
			fclose(fp);
		}
	}

	merc_down = TRUE;
}

/* object condition aliases */
const char *get_cond_alias(OBJ_DATA *obj)
{
	const char *stat;
	int istat = obj->condition;

	     if	(istat >= COND_EXCELLENT)	stat = "excellent";
	else if (istat >= COND_FINE)		stat = "fine";
	else if (istat >= COND_GOOD)		stat = "good";
	else if (istat >= COND_AVERAGE)		stat = "average";
	else if (istat >= COND_POOR)		stat = "poor";
	else					stat = "fragile";

	return stat;
}

void damage_to_obj(CHAR_DATA *ch, OBJ_DATA *wield, OBJ_DATA *worn, int dmg)
{
	if (dmg == 0)
		return;

	worn->condition -= dmg;

	if (wield != NULL) {
		act_puts("{g$p inflicts damage on {r$P{g.{x",
			 ch, wield, worn, TO_ROOM, POS_RESTING);
	} else {
		act_puts("{gYou inflict damage on {r$P{g.{x",
			 ch, NULL, worn, TO_CHAR, POS_DEAD);
		act_puts("{g$n inflicts damage on {r$P{g.{x",
			 ch, NULL, worn, TO_ROOM, POS_DEAD);
	}

	if (worn->condition < 1) {
		act_puts("{r$P{g breaks into pieces.{x",
			 ch, wield, worn, TO_ROOM, POS_RESTING);
		extract_obj(worn, 0);
		return;
	}

	if (wield == NULL
	||  !IS_OBJ_STAT(wield, ITEM_MAGIC))
		return;

	if (IS_OBJ_STAT(wield, ITEM_ANTI_EVIL)
	&&  IS_OBJ_STAT(wield, ITEM_ANTI_NEUTRAL)
	&&  IS_OBJ_STAT(worn, ITEM_ANTI_EVIL)
	&&  IS_OBJ_STAT(worn, ITEM_ANTI_NEUTRAL)) {
		act_puts("$p doesn't want to fight against $P.",
			 ch, wield, worn, TO_ROOM, POS_RESTING);
		act_puts("$p removes itself from you!",
			 ch, wield, worn, TO_CHAR, POS_RESTING);
		act_puts("$p removes itself from $n.",
			 ch, wield, worn, TO_ROOM, POS_RESTING);
		unequip_char(ch, wield);
		return;
	}

	if (IS_OBJ_STAT(wield, ITEM_ANTI_EVIL)
	&&  IS_OBJ_STAT(worn, ITEM_ANTI_EVIL)) {
		act_puts("$p worries for the damage to $P.",
			 ch, wield, worn, TO_ROOM, POS_RESTING);
		return;
	}
}

bool make_eq_damage(CHAR_DATA *ch, CHAR_DATA *victim,
		    int loc_wield, int loc_destroy)
{
	OBJ_DATA *wield, *destroy;
	const char *sn;
	int skill, dam, chance = 0;
	flag_t wflags, dflags;

	if ((wield = get_eq_char(ch, loc_wield)) == NULL
	||  (destroy = get_eq_char(victim, loc_destroy)) == NULL
	||  material_is(destroy, MATERIAL_INDESTRUCT))
		return FALSE;

	sn = get_weapon_sn(wield);
	skill = get_skill(ch, sn);

	if (number_percent() > skill)
		return FALSE;

	wflags = get_mat_flags(wield);
	dflags = get_mat_flags(destroy);

	chance = IS_SET(wflags, MATERIAL_METAL) +
		 IS_SET(wflags, MATERIAL_INDESTRUCT) -
		 IS_SET(wflags, MATERIAL_FRAGILE) -
		 IS_SET(dflags, MATERIAL_METAL) +
		 IS_SET(dflags, MATERIAL_FRAGILE);

	chance *= 15;

	chance += (LEVEL(ch) - LEVEL(victim)) / 3;
	chance += (wield->level - destroy->level) / 2;

	if (IS_WEAPON_STAT(wield, WEAPON_SHARP))
		chance += 10;

	if (IS_SKILL(sn, "axe"))
		chance += 10;

	if (IS_OBJ_STAT(destroy, ITEM_BLESS))
		chance -= 20;

	if (IS_OBJ_STAT(destroy, ITEM_MAGIC))
		chance -= 10;

	chance += skill - 85;

	dam = number_range(chance / 10, 3 * chance / 10) +
	      get_curr_stat(ch, STAT_STR) / 5;

	if (IS_SET(dflags, MATERIAL_FRAGILE))
		dam += number_range(0, 30);

	chance += get_curr_stat(ch, STAT_DEX) - get_curr_stat(victim, STAT_DEX);

	if (number_percent() < chance) {
		damage_to_obj(ch, wield, destroy, dam);
		return TRUE;
	}

	return FALSE;
}

bool random_eq_damage(CHAR_DATA *ch, CHAR_DATA *victim, int loc_wield)
{
	int i;

	do {
		i = number_range(0, MAX_WEAR - 1);
	} while (i == WEAR_WIELD || i == WEAR_SECOND_WIELD ||
		 i == WEAR_TATTOO || i == WEAR_STUCK_IN ||
		 i == WEAR_SHIELD || i == WEAR_CLANMARK);

	return make_eq_damage(ch, victim, loc_wield, i);
}

static inline int
get_played(CHAR_DATA *ch, bool add_age)
{
	int pl;

	if (IS_NPC(ch))
		return 0;

	pl = current_time - PC(ch)->logon + PC(ch)->played;
	if (add_age)
		pl += PC(ch)->add_age;
	return pl;
}

int get_age(CHAR_DATA *ch)
{
	return (17 + get_played(ch, TRUE) / 72000);
}

int get_hours(CHAR_DATA *ch)
{
	return get_played(ch, FALSE) / 3600;
}

int trust_level(CHAR_DATA *ch)
{
	ch = GET_ORIGINAL(ch);
	return IS_NPC(ch) ? UMIN((ch)->level, LEVEL_HERO - 1) : ch->level;
}

/* returns a flag for wiznet */
flag_t wiznet_lookup(const char *name)
{
	int flag;

	for (flag = 0; wiznet_table[flag].name; flag++)
	{
		if (LOWER(name[0]) == LOWER(wiznet_table[flag].name[0])
		&& !str_prefix(name,wiznet_table[flag].name))
		    return flag;
	}

	return -1;
}

/*
 * assumes !IS_NPC(ch)
 */
void
advance_level(CHAR_DATA *ch)
{
	int add_hp;
	int add_mana;
	int add_move;
	int add_prac=0;
	class_t *cl;

	if (IS_NPC(ch)) {
		log(LOG_BUG, "advance_level: a mob to advance");
		return;
	}

	if ((cl = class_lookup(ch->class)) == NULL) {
		log(LOG_INFO, "advance_level: %s: unknown class %s",
		    ch->name, ch->class);
		return;
	}

	add_hp = number_range(min_hit_gain(ch, cl), max_hit_gain(ch, cl));
	add_mana = number_range(min_mana_gain(ch, cl), max_mana_gain(ch, cl));
	add_move = number_range(min_move_gain(ch), max_move_gain(ch));

	ch->max_hit += add_hp;
	ch->max_mana += add_mana;
	ch->max_move += add_move;

	ch->perm_hit += add_hp;
	ch->perm_mana += add_mana;
	ch->perm_move += add_move;

	if (PC(ch)->plevels > 0) {
		PC(ch)->plevels--;
	} else {
		PC(ch)->train += ch->level % 5 ? 0 : 1;
		add_prac = wis_app[get_curr_stat(ch,STAT_WIS)].practice;
		PC(ch)->practice += add_prac;
	}

	act_puts("Your gain is $T {C$j{x prac.",
		 ch, (const void *) add_prac,
		 format_hmv(add_hp, add_mana, add_move),
		 TO_CHAR, POS_DEAD);
}

void
delevel(CHAR_DATA *ch)
{
	int lost_hitp;
	int lost_mana;
	int lost_move;
	class_t *cl;

	if (IS_NPC(ch)) {
		log(LOG_BUG, "delevel: a mob to delevel");
		return;
	}

	if ((cl = class_lookup(ch->class)) == NULL) {
		log(LOG_INFO, "delevel: %s: unknown class %s",
		    ch->name, ch->class);
		return;
	}

	act("You loose a level!", ch, NULL, NULL, TO_CHAR);
	ch->level--;
	update_skills(ch);

	lost_hitp = max_hit_gain(ch, cl);
	lost_mana = max_mana_gain(ch, cl);
	lost_move = max_move_gain(ch);

	ch->max_hit  -= lost_hitp;
	ch->max_mana -= lost_mana;
	ch->max_move -= lost_move;

	ch->perm_hit  -= lost_hitp;
	ch->perm_mana -= lost_mana;
	ch->perm_move -= lost_move;

	PC(ch)->plevels++;

	act_puts("You loose $t.",
		 ch, format_hmv(lost_hitp, lost_mana, lost_move), NULL,
		 TO_CHAR, POS_DEAD);

	if(ch->perm_hit <= 0) {
		act("You've lost all your life power.", ch, NULL, NULL,
		    TO_CHAR);
		delete_player(ch, "lack of hp");
	} else if (ch->perm_mana <= 0) {
		act("You've lost all your power.", ch, NULL, NULL, TO_CHAR);
		delete_player(ch, "lack of mana");
	} else if (ch->perm_move <= 0) {
		act("You've lost all your ability to move.",
		    ch, NULL, NULL, TO_CHAR);
		delete_player(ch, "lack of move");
	}
}

/*
 * assumes !IS_NPC(ch)
 */
void gain_exp(CHAR_DATA *ch, int gain)
{
	if (ch->level >= LEVEL_HERO)
		return;

	if (IS_SET(PC(ch)->plr_flags, PLR_NOEXP) && gain > 0) {
		act_char("You can't gain exp without your spirit.", ch);
		return;
	}

	PC(ch)->exp += gain;

	while (ch->level < LEVEL_HERO && exp_to_level(ch) <= 0) {
		class_t *cl;

		act_puts("{CYou raise a level!!{x ",
			 ch, NULL, NULL, TO_CHAR | ACT_NOLF, POS_DEAD);
		ch->level++;

		if ((cl = class_lookup(ch->class)) != NULL
		&&  cl->death_limit != 0
		&&  ch->level == LEVEL_PK)
			ch->wimpy = 0;

		if (ch->level == LEVEL_HERO)
			log(LOG_INFO, "%s made a hero level.", ch->name);

		wiznet("$N has attained level $j!",
			ch, (const void*) ch->level, WIZ_LEVELS, 0, 0);
		advance_level(ch);
		update_skills(ch);
		char_save(ch, 0);
	}
}

void bad_effect(CHAR_DATA* ch, int effect)
{
	switch(effect) {
	case FAIL_NONE:
	default:
		act("Nothing happens.", ch, NULL, NULL, TO_CHAR);
		return;
	case FAIL_DEATH:
		act("You die..", ch, NULL, NULL, TO_CHAR);
		raw_kill(ch, ch);
		return;
	case FAIL_HALLUCINATION:
		spellfun_call("hallucination", NULL, ch->level, ch, ch);
		return;
	}
}

int get_resist(CHAR_DATA *ch, int dam_class)
{
	int16_t *resists;
	int16_t bonus=0;

	if (ch->shapeform)
		resists = ch->shapeform->resists;
	else
		resists = ch->resists;

	switch(dam_class) {
	case DAM_POISON:
	case DAM_DISEASE:
		bonus = get_curr_stat(ch, STAT_CON) - 18;
		break;
	case DAM_BASH:
		bonus = (get_curr_stat(ch, STAT_CON) - 18) / 2;
		break;
	case DAM_MENTAL:
		bonus = (get_curr_stat(ch, STAT_WIS) + get_curr_stat(ch, STAT_INT) - 36) / 2;
		break;
	case DAM_HOLY:
		bonus = ch->alignment / 500;
		break;
	case DAM_NEGATIVE:
		bonus = - ch->alignment / 500;
		break;
	}

	if (dam_class != DAM_NONE)
		return URANGE(-100, resists[dam_class] + bonus, 100);

	return IS_IMMORTAL(ch)? 100 : 0;
}

void
label_add(OBJ_DATA *obj, const char *label)
{
	const char *p = obj->label;
	obj->label = str_printf("%s %s", obj->label, label);
	free_string(p);
}

void
make_visible(CHAR_DATA *ch, bool at_will)
{
	if (HAS_INVIS(ch, ID_HIDDEN | ID_FADE)) {
		REMOVE_INVIS(ch, ID_HIDDEN | ID_FADE);
		act_puts("You step out of shadows.",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		act_puts("$n steps out of shadows.",
			 ch, NULL, NULL, TO_ROOM, POS_RESTING);
	}

	if (HAS_INVIS(ch, ID_CAMOUFLAGE | ID_BLEND)) {
		REMOVE_INVIS(ch, ID_CAMOUFLAGE | ID_BLEND);
		affect_bit_strip(ch, TO_INVIS, ID_BLEND);
		act_puts("You step out from your cover.",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		act("$n steps out from $m's cover.",
		    ch, NULL, NULL, TO_ROOM);
	}

	if (HAS_INVIS(ch, ID_INVIS)) {
		REMOVE_INVIS(ch, ID_INVIS);
		affect_bit_strip(ch, TO_INVIS, ID_INVIS);
		act_puts("You fade into existence.",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		act("$n fades into existence.", ch, NULL, NULL, TO_ROOM);
	}

	if (!at_will)
		return;

	if (HAS_INVIS(ch, ID_IMP_INVIS)) {
		REMOVE_INVIS(ch, ID_IMP_INVIS);
		affect_bit_strip(ch, TO_INVIS, ID_IMP_INVIS);
		act_puts("You fade into existence.",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		act("$n fades into existence.", ch, NULL, NULL, TO_ROOM);
	}
}

/*--------------------------------------------------------------------
 * static functions
 */

static const char *
format_hmv(int hp, int mana, int move)
{
	static char buf[MAX_STRING_LENGTH];
	snprintf(buf, sizeof(buf), "{C%d{x hp, {C%d{x mana, {C%d{x mv",
		 hp, mana, move);
	return buf;
}

/*
 * Following functions assume !IS_NPC(ch).
 */
static int
max_hit_gain(CHAR_DATA *ch, class_t *cl)
{
	return (con_app[get_max_train(ch, STAT_CON)].hitp + 2) *
		cl->hp_rate / 100;
}

static int
min_hit_gain(CHAR_DATA *ch, class_t *cl)
{
	return (con_app[get_curr_stat(ch, STAT_CON)].hitp - 3) *
		cl->hp_rate / 100;
}

static int
max_mana_gain(CHAR_DATA *ch, class_t *cl)
{
	return (get_max_train(ch, STAT_WIS) + get_max_train(ch, STAT_INT) + 5) *
		cl->mana_rate / 200;
}

static int
min_mana_gain(CHAR_DATA *ch, class_t *cl)
{
	return (get_curr_stat(ch, STAT_WIS) + get_curr_stat(ch, STAT_INT) - 3) *
		cl->mana_rate / 200;
}

static int
max_move_gain(CHAR_DATA *ch)
{
	return UMAX(6, get_max_train(ch, STAT_DEX)/4+get_max_train(ch, STAT_CON)/6);
}

static int
min_move_gain(CHAR_DATA *ch)
{
	return UMAX(6, get_curr_stat(ch, STAT_DEX)/5 + get_curr_stat(ch, STAT_CON)/7);
}
