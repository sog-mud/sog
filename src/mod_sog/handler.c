/*
 * $Id: handler.c,v 1.333 2001-09-19 08:38:53 kostik Exp $
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
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <merc.h>
#include <lang.h>

#include <magic.h>
#include <update.h>
#include <quest.h>

#include <sog.h>

#include "comm.h"
#include "handler_impl.h"

static const char *	format_hmv	(int hp, int mana, int move);

static int		max_hit_gain	(CHAR_DATA *ch, class_t *cl);
static int		min_hit_gain	(CHAR_DATA *ch, class_t *cl);
static int		max_mana_gain	(CHAR_DATA *ch, class_t *cl);
static int		min_mana_gain	(CHAR_DATA *ch, class_t *cl);
static int		max_move_gain	(CHAR_DATA *ch);
static int		min_move_gain	(CHAR_DATA *ch);

static void drop_objs(CHAR_DATA *ch, OBJ_DATA *obj_list);
static OBJ_DATA *get_obj_list_raw(CHAR_DATA *ch, const char *name, uint *number,
				  OBJ_DATA *list, int flags);
static OBJ_DATA *get_obj_here_raw(CHAR_DATA *ch, const char *name,
				  uint *number);
static CHAR_DATA *get_char_room_raw(CHAR_DATA *ch, const char *name,
				    uint *number, ROOM_INDEX_DATA *room);
static void strip_obj_affects(CHAR_DATA *ch, AFFECT_DATA *paf);
static OBJ_DATA *get_stuck_eq(CHAR_DATA *ch, int wtype);
static bool has_boat(CHAR_DATA *ch);
static bool has_key(CHAR_DATA *ch, int key);
static bool has_key_ground(CHAR_DATA *ch, int key);

static void char_from_room(CHAR_DATA *ch);

static void obj_from_xxx(OBJ_DATA *obj);
static void obj_from_room(OBJ_DATA *obj);
static void obj_from_obj(OBJ_DATA *obj);

/*
 * Move a char into a room.
 */
void
char_to_room(CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex)
{
	olced_t *olced;
	OBJ_DATA *obj;

	if (pRoomIndex == NULL) {
		ROOM_INDEX_DATA *room;

		log(LOG_BUG, "%s: NULL", __FUNCTION__);

		if ((room = get_room_index(ROOM_VNUM_TEMPLE)) != NULL)
			char_to_room(ch, room);

		return;
	}

	if (ch->in_room != NULL)
		char_from_room(ch);

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
 * Take an obj from its character.
 */
void
obj_from_char(OBJ_DATA *obj)
{
	CHAR_DATA *ch;

	if ((ch = obj->carried_by) == NULL) {
		log(LOG_BUG, "%s: null ch (obj->name  = '%s')",
		    __FUNCTION__, obj->pObjIndex->name);
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
			log(LOG_BUG, "%s: obj not in list", __FUNCTION__);
		else
			prev->next_content = obj->next_content;
	}

	obj->carried_by		= NULL;
	obj->next_content	= NULL;
	ch->carry_number	-= get_obj_number(obj);
	ch->carry_weight	-= get_obj_weight(obj);
}

/*
 * Give an obj to a char.
 */
void
obj_to_char(OBJ_DATA *obj, CHAR_DATA *ch)
{
	obj_from_xxx(obj);

	obj->next_content	= ch->carrying;
	ch->carrying		= obj;
	obj->carried_by		= ch;

	if (obj->last_owner && !IS_NPC(ch) && obj->last_owner != ch) {
		name_add(&PC(obj->last_owner)->enemy_list, ch->name, NULL,NULL);
		PC(ch)->last_offence_time = current_time;
	}

	if (!IS_NPC(ch))
		obj->last_owner		= ch;

	ch->carry_number	+= get_obj_number(obj);
	ch->carry_weight	+= get_obj_weight(obj);
}

/*
 * Move an obj into a room.
 */
void
obj_to_room(OBJ_DATA *obj, ROOM_INDEX_DATA *pRoomIndex)
{
	obj_from_xxx(obj);

	obj->next_content	= pRoomIndex->contents;
	pRoomIndex->contents	= obj;
	obj->in_room		= pRoomIndex;

	if (IS_WATER(pRoomIndex))
		obj->water_float = floating_time(obj);
}

/*
 * Move an object into an object.
 */
void
obj_to_obj(OBJ_DATA *obj, OBJ_DATA *obj_to)
{
	if (obj == obj_to) {
		log(LOG_INFO, "obj_to_obj: obj == obj_to (vnum %d)",
			   obj->pObjIndex->vnum);
		return;
	}

	obj_from_xxx(obj);

	obj->next_content	= obj_to->contains;
	obj_to->contains	= obj;
	obj->in_obj		= obj_to;

	if (OBJ_IS(obj_to, OBJ_PIT))
		obj->cost = 0;

	for (; obj_to != NULL; obj_to = obj_to->in_obj) {
		if (obj_to->carried_by != NULL) {
/*	    obj_to->carried_by->carry_number += get_obj_number(obj); */
		    obj_to->carried_by->carry_weight += get_obj_weight(obj)
			* WEIGHT_MULT(obj_to) / 100;
		}
	}
}

/*
 * Create an instance of a mobile.
 */
CHAR_DATA *
create_mob(int vnum, int flags)
{
	CHAR_DATA *mob;
	int i;
	race_t *r;
	AFFECT_DATA *paf;
	MOB_INDEX_DATA *pMobIndex;

	if ((pMobIndex = get_mob_index(vnum)) == NULL) {
		log(LOG_BUG, "create_mob: vnum %d: no such mob", vnum);
		return NULL;
	}

	mob = char_new(pMobIndex);

	mob->name	= str_qdup(pMobIndex->name);
	mlstr_cpy(&mob->short_descr, &pMobIndex->short_descr);
	mlstr_cpy(&mob->long_descr, &pMobIndex->long_descr);
	mlstr_cpy(&mob->description, &pMobIndex->description);
	mob->class = str_empty;

	if (pMobIndex->wealth) {
		long wealth;

		wealth = number_range(pMobIndex->wealth/2,
				      3 * pMobIndex->wealth/2);
		mob->gold = number_range(wealth/200,wealth/100);
		mob->silver = wealth - (mob->gold * 100);
	}

	mob->affected_by	= pMobIndex->affected_by;
	mob->has_invis		= pMobIndex->has_invis;
	mob->has_detect		= pMobIndex->has_detect;
	mob->alignment		= pMobIndex->alignment;
	mob->level		= pMobIndex->level;
	mob->position		= pMobIndex->start_pos;

	free_string(mob->race);
	mob->race		= str_qdup(pMobIndex->race);

	/*
	 * apply race modifiers
	 */
	if ((r = race_lookup(pMobIndex->race)) != NULL)
		mob->luck	+= r->luck_bonus;

	mob->form		= pMobIndex->form;
	mob->parts		= pMobIndex->parts;
	mob->size		= pMobIndex->size;
	free_string(mob->clan);
	mob->clan		= str_qdup(pMobIndex->clan);
	mob->invis_level	= pMobIndex->invis_level;
	mob->incog_level	= pMobIndex->incog_level;
	mob->material		= str_qdup(pMobIndex->material);

	mob->damtype		= str_qdup(pMobIndex->damtype);
	if (IS_NULLSTR(mob->damtype)) {
		switch (number_range(1, 3)) {
		case 1:
			mob->damtype = str_dup("slash");
			break;
		case 2:
			mob->damtype = str_dup("pound");
			break;
		case 3:
			mob->damtype = str_dup("pierce");
			break;
		}
	}

	if (flag_value(sex_table, mlstr_mval(&pMobIndex->gender)) == SEX_EITHER) {
		MOB_INDEX_DATA *fmob;
		int sex = number_range(SEX_MALE, SEX_FEMALE);

		mlstr_destroy(&mob->gender);
		mlstr_init2(&mob->gender, flag_string(gender_table, sex));

		if (sex == SEX_FEMALE
		&&  (fmob = get_mob_index(pMobIndex->fvnum))) {
			mob->name	= str_qdup(fmob->name);
			mlstr_cpy(&mob->short_descr, &fmob->short_descr);
			mlstr_cpy(&mob->long_descr, &fmob->long_descr);
			mlstr_cpy(&mob->description, &fmob->description);
		}
	} else {
		mlstr_cpy(&mob->gender, &pMobIndex->gender);
	}

	for (i = 0; i < MAX_STAT; i ++)
		mob->perm_stat[i] = UMIN(25, 11 + mob->level/4);

	mob->perm_stat[STAT_STR] += mob->size - SIZE_MEDIUM;
	mob->perm_stat[STAT_CON] += (mob->size - SIZE_MEDIUM) / 2;

	mob->hitroll		= (mob->level / 2) + pMobIndex->hitroll;
	mob->damroll		= pMobIndex->damage[DICE_BONUS];
	SET_HIT(mob, dice(pMobIndex->hit[DICE_NUMBER],
			  pMobIndex->hit[DICE_TYPE]) +
		     pMobIndex->hit[DICE_BONUS]);
	SET_MANA(mob, dice(pMobIndex->mana[DICE_NUMBER],
			   pMobIndex->mana[DICE_TYPE]) +
		      pMobIndex->mana[DICE_BONUS]);
	NPC(mob)->dam.dice_number = pMobIndex->damage[DICE_NUMBER];
	NPC(mob)->dam.dice_type = pMobIndex->damage[DICE_TYPE];
	for (i = 0; i < 4; i++)
		mob->armor[i]	= pMobIndex->ac[i];

	if (IS_SET(pMobIndex->act, ACT_WARRIOR)) {
		mob->perm_stat[STAT_STR] += 3;
		mob->perm_stat[STAT_INT] -= 1;
		mob->perm_stat[STAT_CON] += 2;
	}

	if (IS_SET(pMobIndex->act, ACT_THIEF)) {
		mob->perm_stat[STAT_DEX] += 3;
		mob->perm_stat[STAT_INT] += 1;
		mob->perm_stat[STAT_WIS] -= 1;
	}

	if (IS_SET(pMobIndex->act, ACT_CLERIC)) {
		mob->perm_stat[STAT_WIS] += 3;
		mob->perm_stat[STAT_DEX] -= 1;
		mob->perm_stat[STAT_STR] += 1;
	}

	if (IS_SET(pMobIndex->act, ACT_MAGE)) {
		mob->perm_stat[STAT_INT] += 3;
		mob->perm_stat[STAT_STR] -= 1;
		mob->perm_stat[STAT_DEX] += 1;
	}

	if (IS_SET(pMobIndex->off_flags, OFF_FAST))
		mob->perm_stat[STAT_DEX] += 2;

	/* let's get some spell action */
	if (IS_AFFECTED(mob, AFF_SANCTUARY)) {
		paf = aff_new(TO_AFFECTS, "sanctuary");
		paf->level	= mob->level;
		paf->duration	= -1;
		paf->bitvector	= AFF_SANCTUARY;
		affect_to_char(mob, paf);
		aff_free(paf);
	}

	if (IS_AFFECTED(mob, AFF_HASTE)) {
		paf = aff_new(TO_AFFECTS, "haste");
		paf->level	= mob->level;
		paf->duration	= -1;
		INT(paf->location)= APPLY_DEX;
		paf->modifier	= 1 + (mob->level >= 18) + (mob->level >= 25) +
				  (mob->level >= 32);
		paf->bitvector	= AFF_HASTE;
		affect_to_char(mob, paf);
		aff_free(paf);
	}

	if (IS_AFFECTED(mob, AFF_PROTECT_EVIL)) {
		paf = aff_new(TO_AFFECTS, "protection evil");
		paf->level	= mob->level;
		paf->duration	= -1;
		INT(paf->location)= APPLY_SAVES;
		paf->modifier	= -1;
		paf->bitvector	= AFF_PROTECT_EVIL;
		affect_to_char(mob, paf);
		aff_free(paf);
	}

	if (IS_AFFECTED(mob, AFF_PROTECT_GOOD)) {
		paf = aff_new(TO_AFFECTS, "protection good");
		paf->level	= mob->level;
		paf->duration	= -1;
		INT(paf->location)= APPLY_SAVES;
		paf->modifier	= -1;
		paf->bitvector	= AFF_PROTECT_GOOD;
		affect_to_char(mob, paf);
		aff_free(paf);
	}

	for (paf = pMobIndex->affected; paf != NULL; paf = paf->next)
		affect_to_char(mob, paf);

	/*
	 * link the mob to the world list
	 * if CM_F_NOLIST is not set
	 */
	if (!IS_SET(flags, CM_F_NOLIST)) {
		if (char_list_lastpc) {
			mob->next = char_list_lastpc->next;
			char_list_lastpc->next = mob;
		} else {
			mob->next = char_list;
			char_list = mob;
		}
	}

	pMobIndex->count++;
	return mob;
}

static
MLSTR_FOREACH_FUN(cb_xxx_of, lang, p, ap)
{
	mlstring *owner = va_arg(ap, mlstring *);
	const char *q;

	if (IS_NULLSTR(*p))
		return NULL;

	q = str_printf(*p, word_form(mlstr_val(owner, lang), 1,
				     lang, RULES_CASE));
	free_string(*p);
	*p = q;
	return NULL;
}

CHAR_DATA *
create_mob_of(int vnum, mlstring *owner)
{
	CHAR_DATA *mob;

	if ((mob = create_mob(vnum, 0)) != NULL) {
		mlstr_foreach(&mob->short_descr, cb_xxx_of, owner);
		mlstr_foreach(&mob->long_descr, cb_xxx_of, owner);
		mlstr_foreach(&mob->description, cb_xxx_of, owner);
	}

	return mob;
}

/* duplicate a mobile exactly -- except inventory */
CHAR_DATA *
clone_mob(CHAR_DATA *parent)
{
	int i;
	AFFECT_DATA *paf, *paf_next;
	CHAR_DATA *clone;

	/*
	 * create_mob can't return NULL because parent->pMobIndex is not NULL
	 */
	clone = create_mob(parent->pMobIndex->vnum, 0);

	/* start fixing values */
	free_string(clone->name);
	clone->name		= str_qdup(parent->name);
	mlstr_cpy(&clone->short_descr, &parent->short_descr);
	mlstr_cpy(&clone->long_descr, &parent->long_descr);
	mlstr_cpy(&clone->description, &parent->description);
	mlstr_cpy(&clone->gender, &parent->gender);
	free_string(clone->class);
	clone->class		= str_qdup(parent->class);
	free_string(clone->race);
	clone->race		= str_qdup(parent->race);
	clone->level		= parent->level;
	clone->wait		= parent->wait;

	clone->hit		= parent->hit;
	clone->max_hit		= parent->max_hit;
	clone->perm_hit		= parent->perm_hit;

	clone->mana		= parent->mana;
	clone->max_mana		= parent->max_mana;
	clone->perm_mana	= parent->perm_mana;

	clone->move		= parent->move;
	clone->max_move		= parent->max_move;
	clone->perm_move	= parent->perm_move;

	clone->gold		= parent->gold;
	clone->silver		= parent->silver;
	clone->comm		= parent->comm;
	clone->invis_level	= parent->invis_level;
	clone->incog_level	= parent->incog_level;
	clone->affected_by	= parent->affected_by;
	clone->has_invis	= parent->has_invis;
	clone->has_detect	= parent->has_detect;
	clone->position		= parent->position;
	clone->saving_throw	= parent->saving_throw;
	clone->alignment	= parent->alignment;
	clone->hitroll		= parent->hitroll;
	clone->damroll		= parent->damroll;
	clone->wimpy		= parent->wimpy;
	clone->form		= parent->form;
	clone->parts		= parent->parts;
	clone->size		= parent->size;
	free_string(clone->material);
	clone->material		= str_qdup(parent->material);
	free_string(clone->damtype);
	clone->damtype		= str_qdup(parent->damtype);
	clone->hunting		= NULL;
	free_string(clone->clan);
	clone->clan	= str_qdup(parent->clan);
	NPC(clone)->dam	= NPC(parent)->dam;

	for (i = 0; i < 4; i++)
		clone->armor[i]	= parent->armor[i];

	for (i = 0; i < MAX_STAT; i++) {
		clone->perm_stat[i]	= parent->perm_stat[i];
		clone->mod_stat[i]	= parent->mod_stat[i];
	}

	/*
	 * clone affects
	 */
	for (paf = clone->affected; paf != NULL; paf = paf_next) {
		paf_next = paf->next;
		affect_remove(clone, paf);
	}
	clone->affected = NULL;

	for (paf = parent->affected; paf != NULL; paf = paf->next)
		affect_to_char(clone, paf);

	return clone;
}

/*
 * Extract a char from the world.
 */
void
extract_char(CHAR_DATA *ch, int flags)
{
	CHAR_DATA *wch;
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;
	int extract_obj_flags;

	if (!mem_is(ch, MT_CHAR)) {
		log(LOG_BUG, "extract_char: ch is not MT_CHAR");
		return;
	}

	strip_raff_owner(ch);

	/* nuke pet */
	if (!IS_NPC(ch)
	&&  (wch = PC(ch)->pet) != NULL) {
		stop_follower(wch);
		act("$n slowly fades away.", wch, NULL, NULL, TO_ROOM);
		extract_char(wch, 0);
		PC(ch)->pet = NULL;
	}

	if (!IS_SET(flags, XC_F_INCOMPLETE))
		die_follower(ch);

	stop_fighting(ch, TRUE);

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

void
quit_char(CHAR_DATA *ch, int flags)
{
	DESCRIPTOR_DATA *d, *d_next;
	CHAR_DATA *vch, *vch_next;
	OBJ_DATA *obj;
	const char *name;

	if (IS_NPC(ch)) {
		log(LOG_BUG, "quit_char: IS_NPC");
		return;
	}

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

	if (ch->shapeform)
		revert(ch);

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

		if (is_sn_affected(ch, "witch curse")) {
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
		&&  is_sn_affected(ch, "evil spirit")) {
			act_char("Evil spirits in you prevents you from leaving.", ch);
			return;
		}

		if (PC(ch)->last_offence_time != -1
		&&  current_time - PC(ch)->last_offence_time < OFFENCE_DELAY_TIME
		&&  !IS_IMMORTAL(ch)) {
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
		if (is_sn_affected(vch, "doppelganger")
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
				act_clan(vch, "$N has left the realm, I have to leave too.", ch);
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

/*
 * Create an instance of an object.
 */
OBJ_DATA *
create_obj(int vnum, int flags)
{
	OBJ_DATA *obj;
	OBJ_INDEX_DATA *pObjIndex;
	int i;

	if ((pObjIndex = get_obj_index(vnum)) == NULL) {
		log(LOG_BUG, "create_obj: vnum %d: no such object", vnum);
		return NULL;
	}

	obj = new_obj();

	obj->pObjIndex	= pObjIndex;
	obj->level = pObjIndex->level;
	obj->wear_loc	= -1;

	mlstr_cpy(&obj->short_descr, &pObjIndex->short_descr);
	mlstr_cpy(&obj->description, &pObjIndex->description);
	obj->material		= str_qdup(pObjIndex->material);
	obj->stat_flags		= pObjIndex->stat_flags;
	obj->wear_flags		= pObjIndex->wear_flags;
	obj->weight		= pObjIndex->weight;
	obj->condition		= pObjIndex->condition;
	obj->cost		= pObjIndex->cost;

	/*
	 * objval_destroy is not needed since obj was just created
	 */
	obj->item_type = pObjIndex->item_type;
	objval_cpy(obj->item_type, obj->value, pObjIndex->value);

	/*
	 * Mess with object properties.
	 */
	switch (obj->item_type) {
	case ITEM_LIGHT:
		if (INT(obj->value[2]) == 999)
			INT(obj->value[2]) = -1;
		break;

	case ITEM_JUKEBOX:
		for (i = 0; i < 5; i++)
			INT(obj->value[i]) = -1;
		break;
	}

	obj->next	= object_list;
	object_list	= obj;
	if (!IS_SET(flags, CO_F_NOCOUNT))
		pObjIndex->count++;
	return obj;
}

OBJ_DATA *
create_obj_of(int vnum, mlstring *owner)
{
	OBJ_DATA *obj;

	if ((obj = create_obj(vnum, 0)) != NULL) {
		mlstr_foreach(&obj->short_descr, cb_xxx_of, owner);
		mlstr_foreach(&obj->description, cb_xxx_of, owner);
	}

	return obj;
}

/* duplicate an object exactly -- except contents */
OBJ_DATA *
clone_obj(OBJ_DATA *parent)
{
	AFFECT_DATA *paf;
	ED_DATA *ed, *ed2;
	OBJ_DATA *clone;

	/*
	 * create_obj can't return NULL becase parent->pObjIndex is not NULL
	 */
	clone = create_obj(parent->pObjIndex->vnum, 0);

	/* start copying the object */
	free_string(clone->label);
	clone->label		= str_qdup(parent->label);

	mlstr_cpy(&clone->short_descr, &parent->short_descr);
	mlstr_cpy(&clone->description, &parent->description);
	clone->stat_flags	= parent->stat_flags;
	clone->wear_flags	= parent->wear_flags;
	clone->weight		= parent->weight;
	clone->cost		= parent->cost;
	clone->level		= parent->level;
	clone->condition	= parent->condition;
	clone->material		= str_qdup(parent->material);
	clone->timer		= parent->timer;
	mlstr_cpy(&clone->owner, &parent->owner);

	/*
	 * obj values
	 */
	objval_destroy(parent->item_type, clone->value);
	objval_cpy(parent->item_type, clone->value, parent->value);

	/*
	 * affects
	 */
	for (paf = parent->affected; paf != NULL; paf = paf->next)
		affect_to_obj(clone, paf);

	/*
	 * extended desc
	 */
	for (ed = parent->ed; ed != NULL; ed = ed->next) {
		ed2		= ed_new();
		ed2->keyword	= str_qdup(ed->keyword);
		mlstr_cpy(&ed2->description, &ed->description);
		ed2->next	= clone->ed;
		clone->ed	= ed2;
	}

	return clone;
}

/*
 * Extract an obj from the world.
 */
void
extract_obj(OBJ_DATA *obj, int flags)
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

		if (!IS_SET(flags, XO_F_NORECURSE)) {
			extract_obj(obj_content, flags);
			continue;
		}

		if (obj->in_room)
			obj_to_room(obj_content, obj->in_room);
		else if (obj->carried_by)
			obj_to_char(obj_content, obj->carried_by);
		else if (obj->in_obj)
			obj_to_obj(obj_content, obj->in_obj);
		else
			extract_obj(obj_content, 0);
	}

	obj_from_xxx(obj);

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
 * True if char can see victim.
 */
bool
can_see(CHAR_DATA *ch, CHAR_DATA *victim)
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

	if (is_sn_affected(ch, "hallucination"))
		return (number_percent() < 70);

	if (char_in_dark_room(ch) && !HAS_DETECT(ch, ID_INFRARED))
		return FALSE;

	if ((victim->has_invis & ID_ALL_INVIS & ~ch_can_see) != 0)
		return FALSE;

	return TRUE;
}

/*
 * True if char can see obj.
 */
bool
can_see_obj(CHAR_DATA *ch, OBJ_DATA *obj)
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

	if (char_in_dark_room(ch) && !HAS_DETECT(ch, ID_INFRARED))
		return FALSE;

	return TRUE;
}

/* visibility on a room -- for entering and exits */
bool
can_see_room(CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex)
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

bool
check_blind(CHAR_DATA *ch)
{
	bool cansee = check_blind_nomessage(ch);

	if (!cansee)
		act_char("You can't see a thing!", ch);

	return cansee;
}

bool
check_blind_nomessage(CHAR_DATA *ch)
{
	if (!IS_NPC(ch) && IS_SET(PC(ch)->plr_flags, PLR_HOLYLIGHT))
		return TRUE;

	if (IS_AFFECTED(ch, AFF_BLIND))
		return FALSE;

	return TRUE;
}

CHAR_DATA *
nth_char(CHAR_DATA *ch, int n)
{
	CHAR_DATA *vch;
	int i = 0;

	for (vch = ch; i < n; vch = (vch->next) ? vch->next : char_list)
		i++;
	return vch;
}

OBJ_DATA *
nth_obj(OBJ_DATA *obj, int n)
{
	OBJ_DATA *nobj;
	int i = 0;

	for (nobj = obj; i < n; nobj = (nobj->next) ? nobj->next : object_list)
		i++;
	return nobj;
}

/*
 * Retrieve a character's carry capacity.
 */
int
can_carry_n(CHAR_DATA *ch)
{
	if (IS_IMMORTAL(ch))
		return -1;

	if (IS_NPC(ch)) {
		if (IS_SET(ch->pMobIndex->act, ACT_PET))
			return 0;
		if (MOB_IS(ch, MOB_JANITOR))
			return -1;
	}

	return MAX_WEAR + get_curr_stat(ch,STAT_DEX) - 10 + ch->size;
}

/*
 * Retrieve a character's carry capacity.
 */
int
can_carry_w(CHAR_DATA *ch)
{
	if (IS_IMMORTAL(ch))
		return -1;

	if (IS_NPC(ch)) {
		if (IS_SET(ch->pMobIndex->act, ACT_PET))
			return 0;
		if (MOB_IS(ch, MOB_JANITOR))
			return -1;
	}

	return (str_app[get_curr_stat(ch,STAT_STR)].carry + ch->level * 25) * 3
	    / 2;
}

/*
 * Return # of objects which an object counts as.
 * Thanks to Tony Chamberlain for the correct recursive code here.
 */
int
get_obj_number(OBJ_DATA *obj)
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

int
get_obj_realnumber(OBJ_DATA *obj)
{
	int number = 1;

	for (obj = obj->contains; obj != NULL; obj = obj->next_content)
	    number += get_obj_number(obj);

	return number;
}

/*
 * Return weight of an object, including weight of contents.
 */
int
get_obj_weight(OBJ_DATA *obj)
{
	int weight;
	OBJ_DATA *tobj;

	weight = obj->weight;
	for (tobj = obj->contains; tobj != NULL; tobj = tobj->next_content)
		weight += get_obj_weight(tobj) * WEIGHT_MULT(obj) / 100;

	return weight;
}

int
get_obj_realweight(OBJ_DATA *obj)
{
	int weight;

	weight = obj->weight;
	for (obj = obj->contains; obj != NULL; obj = obj->next_content)
	    weight += get_obj_weight(obj);

	return weight;
}

/*
 * Equip a char with an obj. Return obj on success. Otherwise returns NULL.
 */
void
equip_char(CHAR_DATA *ch, OBJ_DATA *obj, int iWear)
{
	int i;
	AFFECT_DATA *paf;

	/*
	 * special WEAR_STUCK_IN handling
	 */
	if (iWear == WEAR_STUCK_IN) {
		obj->wear_loc = iWear;
		return;
	}

	/*
	 * sanity check
	 */
	if (get_eq_char(ch, iWear)) {
		if (IS_NPC(ch)) {
			log(LOG_BUG,
			    "equip_char: vnum %d: in_room %d: obj vnum %d: location %s: already equipped.", // notrans
			    ch->pMobIndex->vnum,
			    ch->in_room ? ch->in_room->vnum : -1,
			    obj->pObjIndex->vnum,
			    flag_string(wear_loc_flags, iWear));
		} else {
			log(LOG_BUG,
			    "equip_char: %s: obj vnum %d: location %s: already equipped.", // notrans
			    ch->name, obj->pObjIndex->vnum,
			    flag_string(wear_loc_flags, iWear));
		}
		return;
	}

	obj->wear_loc = iWear;

	/*
	 * apply obj modifiers
	 */
	for (i = 0; i < 4; i++)
		ch->armor[i] -= apply_ac(obj, obj->wear_loc, i);

	if (!IS_OBJ_STAT(obj, ITEM_ENCHANTED)) {
		for (paf = obj->pObjIndex->affected; paf; paf = paf->next)
			affect_modify(ch, paf, TRUE);
	}

	for (paf = obj->affected; paf; paf = paf->next)
		affect_modify(ch, paf, TRUE);

	/*
	 * adjust light
	 */
	if (obj->item_type == ITEM_LIGHT
	&&  INT(obj->value[2]) != 0
	&&  ch->in_room != NULL)
		++ch->in_room->light;
}

static void
strip_obj_affects(CHAR_DATA *ch, AFFECT_DATA *paf)
{
	for (; paf != NULL; paf = paf->next) {
		affect_modify(ch, paf, FALSE);
		affect_check(ch, paf->where, paf->bitvector);
	}
}

/*
 * Unequip a char with an obj.
 */
void
unequip_char(CHAR_DATA *ch, OBJ_DATA *obj)
{
	int i;
	int wear_loc = obj->wear_loc;

	/*
	 * sanity check
	 */
	if (wear_loc == WEAR_NONE) {
		log(LOG_BUG, "unequip_char: already unequipped");
		return;
	}

	/*
	 * special WEAR_STUCK_IN handling
	 */
	if (wear_loc == WEAR_STUCK_IN) {
		obj->wear_loc = WEAR_NONE;
		return;
	}

	/*
	 * unapply obj modifiers
	 */
	for (i = 0; i < 4; i++)
		ch->armor[i] += apply_ac(obj, obj->wear_loc, i);

	if (!IS_OBJ_STAT(obj, ITEM_ENCHANTED))
		strip_obj_affects(ch, obj->pObjIndex->affected);
	strip_obj_affects(ch, obj->affected);

	obj->wear_loc = WEAR_NONE;

	/*
	 * adjust light
	 */
	if (obj->item_type == ITEM_LIGHT
	&&  INT(obj->value[2]) != 0
	&&  ch->in_room != NULL
	&&  ch->in_room->light > 0)
		--ch->in_room->light;

	if (wear_loc == WEAR_WIELD
	&&  (obj = get_eq_char(ch, WEAR_SECOND_WIELD)) != NULL) {
		act_puts("You wield your second weapon as your first!",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		act("$n wields $s second weapon as first!",
		    ch, NULL, NULL, TO_ROOM);
		obj->wear_loc = WEAR_WIELD;
	}
}

/*
 * Find a piece of eq on a character.
 */
OBJ_DATA *
get_eq_char(CHAR_DATA *ch, int iWear)
{
	OBJ_DATA *obj;

	if (ch == NULL)
		return NULL;

	if (ch->shapeform)
		return NULL;

	for (obj = ch->carrying; obj != NULL; obj = obj->next_content) {
		if (obj->wear_loc == iWear)
			return obj;
	}

	return NULL;
}

#define CAN_SWIM(ch)	(IS_IMMORTAL(ch) || number_percent() < get_skill(ch, "swimming"))
#define CAN_FLY(ch)	(IS_IMMORTAL(ch) || IS_AFFECTED(ch, AFF_FLYING))

static int movement_loss[MAX_SECT] =
{
	1, 2, 2, 3, 4, 6, 4, 1, 12, 10, 6
};

static
FOREACH_CB_FUN(pull_obj_trigger_cb, p, ap)
{
	OBJ_DATA *obj = (OBJ_DATA *) p;

	int trig_type = va_arg(ap, int);
	CHAR_DATA *ch = va_arg(ap, CHAR_DATA *);
	char *arg = va_arg(ap, char *);

	pull_obj_trigger(trig_type, obj, ch, arg);
	if (IS_EXTRACTED(ch))
		return p;

	return NULL;
}

static
FOREACH_CB_FUN(pull_mob_exit_cb, p, ap)
{
	CHAR_DATA *vch = (CHAR_DATA *) p;

	CHAR_DATA *ch = va_arg(ap, CHAR_DATA *);
	char *arg = va_arg(ap, char *);

	if (!can_see(vch, ch))
		return NULL;

	if (pull_mob_trigger(TRIG_MOB_EXIT, vch, ch, arg) > 0
	||  IS_EXTRACTED(ch))
		return p;

	return NULL;
}

static
FOREACH_CB_FUN(pull_mob_greet_cb, p, ap)
{
	CHAR_DATA *vch = (CHAR_DATA *) p;

	CHAR_DATA *ch = va_arg(ap, CHAR_DATA *);
	char *arg = va_arg(ap, char *);

	if (!can_see(vch, ch))
		return NULL;

	pull_mob_trigger(TRIG_MOB_GREET, vch, ch, arg);
	if (IS_EXTRACTED(ch))
		return p;

	if (vo_foreach(vch, &iter_obj_char, pull_obj_trigger_cb,
		       TRIG_OBJ_GREET, ch, arg) != NULL)
		return p;

	return NULL;
}

bool
pull_exit_triggers(CHAR_DATA *ch, int door)
{
	const char *dir = door == -1 ? "portal" : dir_name[door];

	if (!IS_NPC(ch)
	&&  vo_foreach(ch->in_room, &iter_char_room, pull_mob_exit_cb,
		       ch, dir) != NULL)
		return FALSE;

	return TRUE;
}

bool
pull_greet_entry_triggers(CHAR_DATA *ch, ROOM_INDEX_DATA *to_room, int door)
{
	const char *dir = door == -1 ? "portal" : dir_name[door];

	/*
	 * pull GREET and ENTRY triggers
	 *
	 * if someone is following the char, these triggers get activated
	 * for the followers before the char, but it's safer this way...
	 */
	if (!IS_NPC(ch)) {
		if (vo_foreach(to_room, &iter_char_room, pull_mob_greet_cb,
			       ch, dir))
			return FALSE;

		if (vo_foreach(to_room, &iter_obj_room, pull_obj_trigger_cb,
			       TRIG_OBJ_GREET, ch, dir))
			return FALSE;

		if (vo_foreach(ch, &iter_obj_char, pull_obj_trigger_cb,
			       TRIG_OBJ_ENTRY, ch, NULL))
			return FALSE;
	}

	pull_mob_trigger(TRIG_MOB_ENTRY, ch, NULL, NULL);
	if (IS_EXTRACTED(ch))
		return FALSE;

	return TRUE;
}

bool
pull_wear_triggers(CHAR_DATA *ch, OBJ_DATA *obj)
{
	if ((IS_OBJ_STAT(obj, ITEM_ANTI_EVIL)    && IS_EVIL(ch)   )
	||  (IS_OBJ_STAT(obj, ITEM_ANTI_GOOD)    && IS_GOOD(ch)   )
	||  (IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch))) {
		/*
		 * Thanks to Morgenes for the bug fix here!
		 */
		act("You try to use $p.",
		    ch, obj, NULL, TO_CHAR);
		act("You are zapped by $p and drop it.",
		    ch, obj, NULL, TO_CHAR);

		act("$n tries to use $p.",
		    ch, obj, NULL, TO_ROOM);
		act("$n is zapped by $p and drops it.",
		    ch, obj, NULL, TO_ROOM);

		obj_to_room(obj, ch->in_room);
		return FALSE;
	}

	if (pull_obj_trigger(TRIG_OBJ_WEAR, obj, ch, NULL) > 0
	||  !mem_is(obj, MT_OBJ)
	||  IS_EXTRACTED(ch))
		return FALSE;

	return TRUE;
}

bool
move_char(CHAR_DATA *ch, int door, flag_t flags)
{
	CHAR_DATA *fch;
	CHAR_DATA *fch_next;
	CHAR_DATA *mount;
	ROOM_INDEX_DATA *in_room;
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	int act_flags;
	AFFECT_DATA *paf;

	if (RIDDEN(ch) && !IS_NPC(ch->mount))
		return move_char(ch->mount, door, flags);

	if ((paf = affect_find(ch->affected, "entanglement")) != NULL
	&&  (INT(paf->location) == APPLY_DEX)) {
		act("You aren't able to move.", ch, NULL, NULL, TO_CHAR);
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
		log(LOG_BUG, "move_char: bad door %d.", door);
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

	if (is_sn_affected(ch, "globe of invulnerability")) {
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
		act("$n steps out from $s cover.",
			ch, NULL, NULL, TO_ROOM);
	}

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

	if (IS_AFFECTED(in_room, RAFF_RANDOMIZER)
	&&  !IS_SET(flags, MC_F_CHARGE)) {
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
			    || !is_sn_affected(MOUNTED(ch), "water breathing"))) {
				act_char("Your mount refuses to dive.", ch);
				return FALSE;
			}
			if (!CAN_SWIM(ch)) {
				act_char("You can't swim.", ch);
				return FALSE;
			}
		}

		move = (movement_loss[URANGE(0, in_room->sector_type, MAX_SECT-1)]
		  + movement_loss[URANGE(0, to_room->sector_type, MAX_SECT-1)])/2;

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

	/*
	 * Exit trigger, if activated, bail out. Only PCs are triggered.
	 */
	if (!pull_exit_triggers(ch, door))
		return FALSE;

	if (!HAS_INVIS(ch, ID_SNEAK | ID_CAMOUFLAGE)
	&&  ch->invis_level < LEVEL_HERO)
		act_flags = TO_ROOM;
	else
		act_flags = TO_ROOM | ACT_NOMORTAL;

	if (!IS_NPC(ch)
	&&  ch->in_room->sector_type != SECT_INSIDE
	&&  ch->in_room->sector_type != SECT_CITY
	&&  number_percent() < get_skill(ch, "quiet movement")
	&&  !IS_SET(flags, MC_F_CHARGE)) {
		act(MOUNTED(ch) ? "$n leaves, riding on $N." : "$n leaves.",
		    ch, NULL, MOUNTED(ch), act_flags);
		check_improve(ch, "quiet movement", TRUE, 1);
	} else if (IS_SET(flags, MC_F_CHARGE)) {
		act("$n spurs $s $N, leaving $t.", ch,
		dir_name[is_sn_affected(ch, "misleading")
			? number_range(0, 5) : door],
		ch->mount,  TO_ROOM);
	} else {
		act(MOUNTED(ch) ? "$n leaves $t, riding on $N." :
				  "$n leaves $t.", ch,
		dir_name[is_sn_affected(ch, "misleading") ?
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
		act("$n steps out from $s cover.",
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

	if (!HAS_INVIS(ch, ID_SNEAK) && ch->invis_level < LEVEL_HERO)
		act_flags = TO_ROOM;
	else
		act_flags = TO_ROOM | ACT_NOMORTAL;

	char_to_room(ch, to_room);
	if (mount) {
		char_to_room(mount, to_room);
		ch->riding = TRUE;
		mount->riding = TRUE;
	}

	if (!IS_EXTRACTED(ch)) {
		dofun("look", ch, "auto");

		if (!IS_SET(flags, MC_F_CHARGE)) {
			act(mount ? "$n has arrived, riding $N." :
				    "$n has arrived.",
			    ch, NULL, mount, act_flags);
		}
	}

	if (in_room == to_room) /* no circular follows */
		return !IS_EXTRACTED(ch);

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
		move_char(fch, door, 0);
	}

	if (IS_EXTRACTED(ch))
		return FALSE;

	if (!pull_greet_entry_triggers(ch, to_room, door))
		return FALSE;

	return TRUE;
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

int
get_age(CHAR_DATA *ch)
{
	return (17 + get_played(ch, TRUE) / 72000);
}

int
get_hours(CHAR_DATA *ch)
{
	return get_played(ch, FALSE) / 3600;
}

typedef struct damsubst_t damsubst_t;
struct damsubst_t {
	int dam_class;
	int dam_subst;
};

damsubst_t damsubst_tab[] = {
	/* DAM_WEAPON damtypes */
	{ DAM_BASH,		DAM_WEAPON },
	{ DAM_PIERCE,		DAM_WEAPON },
	{ DAM_SLASH,		DAM_WEAPON },

	/* DAM_MAGIC damtypes */
	{ DAM_FIRE,		DAM_MAGIC },
	{ DAM_COLD,		DAM_MAGIC },
	{ DAM_LIGHTNING,	DAM_MAGIC },
	{ DAM_ACID,		DAM_MAGIC },
	{ DAM_POISON,		DAM_MAGIC },
	{ DAM_NEGATIVE,		DAM_MAGIC },
	{ DAM_HOLY,		DAM_MAGIC },
	{ DAM_ENERGY,		DAM_MAGIC },
	{ DAM_MENTAL,		DAM_MAGIC },
	{ DAM_DISEASE,		DAM_MAGIC },
	{ DAM_LIGHT,		DAM_MAGIC },
	{ DAM_CHARM,		DAM_MAGIC },
	{ DAM_SOUND,		DAM_MAGIC },
	{ DAM_HARM,		DAM_MAGIC },
	{ DAM_OTHER,		DAM_MAGIC },
};

#define DAMSUBST_TAB_SZ	(sizeof(damsubst_tab) / sizeof(*damsubst_tab))

static int
get_res(CHAR_DATA *ch, race_t *r, int dam_class)
{
	if (ch->shapeform != NULL) {
		if (ch->shapeform->index->resists[dam_class] != RES_UNDEF)
			return ch->shapeform->index->resists[dam_class];
		return 0;
	}

	if (IS_NPC(ch) && ch->pMobIndex->resists[dam_class] != RES_UNDEF)
		return ch->pMobIndex->resists[dam_class];

	return r->resists[dam_class];
}

int
get_resist(CHAR_DATA *ch, int dam_class, bool default_mod)
{
	int res;
	int mod;

	int16_t *res_mod;
	race_t *r;

	damsubst_t *d;
	static bool damsubsts_initialized = FALSE;

	if ((r = race_lookup(ch->race)) == NULL)
		return 0;

	/*
	 * handle DAM_NONE and invalid damtypes
	 */
	if (dam_class == DAM_NONE)
		return 0;

	if (dam_class < 0) {
		log(LOG_BUG, "get_resist: dam_class %d < 0", dam_class);
		return 0;
	}

	if (dam_class >= MAX_RESIST) {
		log(LOG_BUG, "get_resist: dam_class %d >= MAX_RESIST",
		    dam_class);
		return 0;
	}

	/*
	 * find default damclass
	 */
	if (!damsubsts_initialized) {
		damsubsts_initialized = TRUE;
		qsort(damsubst_tab, DAMSUBST_TAB_SZ,
		     sizeof(damsubst_t), cmpint);
	}

	d = bsearch(
	    &dam_class, damsubst_tab, DAMSUBST_TAB_SZ,
	    sizeof(damsubst_t), cmpint);

	/*
	 * calculate resist modifier
	 */
	if (ch->shapeform != NULL)
		res_mod = ch->shapeform->res_mod;
	else
		res_mod = ch->res_mod;

	mod = res_mod[dam_class];
	if (d != NULL && default_mod)
		mod += res_mod[d->dam_subst];

	/* additional mods based on dam_class */
	switch (dam_class) {
	case DAM_POISON:
	case DAM_DISEASE:
		mod += get_curr_stat(ch, STAT_CON) - 18;
		break;
	case DAM_BASH:
		mod += (get_curr_stat(ch, STAT_CON) - 18) / 2;
		break;
	case DAM_MENTAL:
		if (IS_IMMORTAL(ch))
			return 100;
		mod += (get_curr_stat(ch, STAT_WIS) + get_curr_stat(ch, STAT_INT) - 36) / 2;
		break;
	case DAM_HOLY:
		mod += ch->alignment / 500;
		break;
	case DAM_NEGATIVE:
		mod -= ch->alignment / 500;
		break;
	case DAM_CHARM:
		if (IS_IMMORTAL(ch))
			return 100;
		break;
	}

	/*
	 * calculate base resist
	 */
	if ((res = get_res(ch, r, dam_class)) == RES_UNDEF) {
		if (d != NULL)
			res = get_res(ch, r, d->dam_subst);

		if (res == RES_UNDEF)
			res = 0;
	}

	return URANGE(-100, res + mod, 100);
}

/*
 * Find a char in the room.
 */
static CHAR_DATA *
get_char_room_raw(CHAR_DATA *ch, const char *name,
		  uint *number, ROOM_INDEX_DATA *room)
{
	CHAR_DATA *rch;
	bool ugly;
	int vnum = 0;

	if (!str_cmp(name, "self"))
		return ch;

	ugly = !str_cmp(name, "ugly");

	if ((IS_NPC(ch) || IS_IMMORTAL(ch))
	&&  is_number(name))
		vnum = atoi(name);

	for (rch = room->people; rch; rch = rch->next_in_room) {
		CHAR_DATA *vch;

		if (!can_see(ch, rch))
			continue;

		if (ugly
		&&  *number == 1
		&&  is_sn_affected(rch, "vampire"))
			return rch;

		vch = (is_sn_affected(rch, "doppelganger") &&
		       (IS_NPC(ch) || !IS_SET(PC(ch)->plr_flags, PLR_HOLYLIGHT))) ?
					rch->doppel : rch;
		if (vnum) {
			if (!IS_NPC(vch) || vnum != vch->pMobIndex->vnum)
				continue;
		} else if ((!IS_IMMORTAL(ch) || name[0])
		       &&  !is_name(name, vch->name))
			continue;

		if (!--(*number))
			return rch;
	}

	return NULL;
}

/*
 * Find a char in the room.
 */
CHAR_DATA *
get_char_here(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	uint number;

	number = number_argument(argument, arg, sizeof(arg));
	if (!number)
		return NULL;

	return get_char_room_raw(ch, arg, &number, ch->in_room);
}

CHAR_DATA *
get_char_room(CHAR_DATA *ch, ROOM_INDEX_DATA *room, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	uint number;

	number = number_argument(argument, arg, sizeof(arg));
	if (!number)
		return NULL;

	return get_char_room_raw(ch, arg, &number, room);
}

CHAR_DATA *
get_char_area(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *ach;
	uint number;
	int vnum = 0;

	number = number_argument(argument, arg, sizeof(arg));
	if (!number)
		return NULL;

	if ((ach = get_char_room_raw(ch, arg, &number, ch->in_room)))
		return ach;

	if (arg[0] == '\0')
		return NULL;

	if ((IS_NPC(ch) || IS_IMMORTAL(ch))
	&&  is_number(arg))
		vnum = atoi(arg);

	for (ach = char_list; ach; ach = ach->next) {
		if (!ach->in_room
		||  ach->in_room == ch->in_room)
			continue;

		if (ach->in_room->area != ch->in_room->area
		||  !can_see(ch, ach))
			continue;

		if (vnum) {
			if (!IS_NPC(ach) || vnum != ach->pMobIndex->vnum)
				continue;
		} else if (!is_name(arg, ach->name))
			continue;

		if (!--number)
			return ach;
	}
	return NULL;
}

/*
 * Find a char in the world.
 */
CHAR_DATA *
get_char_world(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *wch;
	uint number;
	int vnum = 0;

	number = number_argument(argument, arg, sizeof(arg));
	if (!number)
		return NULL;

	if ((wch = get_char_room_raw(ch, arg, &number, ch->in_room)))
		return wch;

	if (arg[0] == '\0')
		return NULL;

	if ((IS_NPC(ch) || IS_IMMORTAL(ch))
	&&  is_number(arg))
		vnum = atoi(arg);

	for (wch = char_list; wch; wch = wch->next) {
		if (!wch->in_room
		||  wch->in_room == ch->in_room
		||  !can_see(ch, wch))
			continue;

		if (vnum) {
			if (!IS_NPC(wch) || vnum != wch->pMobIndex->vnum)
				continue;
		} else if (!is_name(arg, wch->name))
			continue;

		if (!--number)
			return wch;
	}

	return NULL;
}

int
exp_to_level(CHAR_DATA *ch)
{
	if (IS_NPC(ch)) {
		log(LOG_BUG, "exp_to_level: IS_NPC");
		return 0;
	}

	return exp_for_level(ch, ch->level+1) - PC(ch)->exp;
}

int
exp_for_level(CHAR_DATA *ch, int level)
{
	if (IS_NPC(ch)) {
		log(LOG_BUG, "exp_for_level: IS_NPC");
		return 0;
	}

	level -= 1;
	/*
	 * Sum 0..n i*(i+1) = (n^3 + 3*n^2 + 2*n) / 3
	 */
	return ((level + 3) * level + 2) * level / 3 * 256;
}

void
gain_exp(CHAR_DATA *ch, int gain)
{
	if (IS_NPC(ch)) {
		log(LOG_BUG, "gain_exp: IS_NPC");
		return;
	}

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
		log(LOG_BUG, "advance_level: IS_NPC");
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
		log(LOG_BUG, "delevel: IS_NPC");
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
		char_delete(ch, "lack of hp");
	} else if (ch->perm_mana <= 0) {
		act("You've lost all your power.", ch, NULL, NULL, TO_CHAR);
		char_delete(ch, "lack of mana");
	} else if (ch->perm_move <= 0) {
		act("You've lost all your ability to move.",
		    ch, NULL, NULL, TO_CHAR);
		char_delete(ch, "lack of move");
	}
}

bool
room_is_dark(ROOM_INDEX_DATA *pRoomIndex)
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
bool
room_is_private(ROOM_INDEX_DATA *pRoomIndex)
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

ROOM_INDEX_DATA *
find_location(CHAR_DATA *ch, const char *argument)
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

static
FOREACH_CB_FUN(pull_mob_get_cb, p, ap)
{
	CHAR_DATA *vch = (CHAR_DATA *) p;

	CHAR_DATA *ch = va_arg(ap, CHAR_DATA *);
	OBJ_DATA *obj = va_arg(ap, OBJ_DATA *);

	pull_mob_trigger(TRIG_MOB_GET, vch, ch, obj);
	if (!mem_is(obj, MT_OBJ) || IS_EXTRACTED(ch))
		return p;

	return NULL;
}

bool
get_obj(CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *container,
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
		return FALSE;
	}

	if ((carry_n = can_carry_n(ch)) >= 0
	&&  ch->carry_number + get_obj_number(obj) > carry_n) {
		act_puts("$P: you can't carry that many items.",
			 ch, NULL, obj, TO_CHAR, POS_DEAD);
		return FALSE;
	}

	if ((carry_w = can_carry_w(ch)) >= 0
	&&  get_carry_weight(ch) + get_obj_weight(obj) > carry_w) {
		act_puts("$P: you can't carry that much weight.",
			 ch, NULL, obj, TO_CHAR, POS_DEAD);
		return FALSE;
	}

	if (obj->in_room != NULL) {
		for (gch = obj->in_room->people; gch != NULL;
		     gch = gch->next_in_room) {
			if (gch->on == obj) {
				act_puts("$N appears to be using $p.",
					 ch, obj, gch, TO_CHAR, POS_DEAD);
				return FALSE;
			}
		}
	}

	if (obj->item_type == ITEM_MONEY) {
		if (carry_w >= 0
		&&  get_carry_weight(ch) + MONEY_WEIGHT(obj) > carry_w) {
			act_puts("$P: you can't carry that much weight.",
				 ch, NULL, obj, TO_CHAR, POS_DEAD);
			return FALSE;
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
	} else {
		act_puts("You get $p.", ch, obj, container, TO_CHAR, POS_DEAD);
		act(msg_others == NULL ? "$n gets $p." : msg_others,
		    ch, obj, container,
		    TO_ROOM | (HAS_INVIS(ch, ID_SNEAK) ? ACT_NOMORTAL : 0));
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
				char buf[MAX_INPUT_LENGTH];

				snprintf(buf, sizeof(buf),
					 "%d %d",		// notrans
					 INT(obj->value[0]),
					 INT(obj->value[1]));
				dofun("split", ch, buf);
			}
		}
		extract_obj(obj, 0);
	} else {
		obj_to_char(obj, ch);

		if (!IS_NPC(ch)) {
			if (vo_foreach(ch->in_room, &iter_char_room,
				       pull_mob_get_cb, ch, obj))
				return FALSE;
		}

		pull_obj_trigger(TRIG_OBJ_GET, obj, ch, NULL);
		if (!mem_is(obj, MT_OBJ) || IS_EXTRACTED(ch))
			return FALSE;
	}

	return FALSE;
}

void
quaff_obj(CHAR_DATA *ch, OBJ_DATA *obj)
{
	OBJ_DATA *vial;

	act("$n quaffs $p.", ch, obj, NULL, TO_ROOM);
	act_puts("You quaff $p.", ch, obj, NULL, TO_CHAR, POS_DEAD);

	if (IS_PUMPED(ch) || ch->fighting != NULL)
		WAIT_STATE(ch, 2 * get_pulse("violence"));

	if ((vial = create_obj(OBJ_VNUM_POTION_VIAL, 0)) != NULL) {
		vial->label = str_qdup(obj->label);
		obj_to_char(vial, ch);
	}

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
 * Wear one object.
 * Optional replacement of existing objects.
 * Big repetitive code, ick.
 */
bool
wear_obj(CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace)
{
	if (IS_NPC(ch)
	&&  (!IS_SET(ch->form, FORM_BIPED) ||
	     !IS_SET(ch->form, FORM_SENTIENT))) {
		act("WEAR ?", ch, NULL, NULL, TO_CHAR);
		return FALSE;
	}

	if (ch->shapeform != NULL) {
		act("You cannot reach your items.",
			ch, NULL, NULL, TO_CHAR);
		return FALSE;
	}

	if (obj->item_type == ITEM_LIGHT) {
		if (!remove_obj(ch, WEAR_LIGHT, fReplace))
			return FALSE;

		if (!pull_wear_triggers(ch, obj))
			return FALSE;

		act("$n lights $p and holds it.", ch, obj, NULL, TO_ROOM);
		act_puts("You light $p and hold it.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		equip_char(ch, obj, WEAR_LIGHT);
		return TRUE;
	}

	if (CAN_WEAR(obj, ITEM_WEAR_FINGER)) {
		if (get_eq_char(ch, WEAR_FINGER_L) != NULL
		&&  get_eq_char(ch, WEAR_FINGER_R) != NULL
		&&  !remove_obj(ch, WEAR_FINGER_L, fReplace)
		&&  !remove_obj(ch, WEAR_FINGER_R, fReplace))
			return FALSE;

		if (get_eq_char(ch, WEAR_FINGER_L) == NULL) {
			if (!pull_wear_triggers(ch, obj))
				return FALSE;

			act("$n wears $p on $s left finger.",
			    ch, obj, NULL, TO_ROOM);
			act_puts("You wear $p on your left finger.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			equip_char(ch, obj, WEAR_FINGER_L);
			return TRUE;
		}

		if (get_eq_char(ch, WEAR_FINGER_R) == NULL) {
			if (!pull_wear_triggers(ch, obj))
				return FALSE;

			act("$n wears $p on $s right finger.",
			    ch, obj, NULL, TO_ROOM);
			act_puts("You wear $p on your right finger.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			equip_char(ch, obj, WEAR_FINGER_R);
			return TRUE;
		}

		log(LOG_BUG, "wear_obj: no free finger");
		act_char("You already wear two rings.", ch);
		return FALSE;
	}

	if (CAN_WEAR(obj, ITEM_WEAR_NECK)) {
		if (!remove_obj(ch, WEAR_NECK, fReplace))
			return FALSE;

		if (!pull_wear_triggers(ch, obj))
			return FALSE;

		act("$n wears $p around $s neck.",
		    ch, obj, NULL, TO_ROOM);
		act_puts("You wear $p around your neck.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		equip_char(ch, obj, WEAR_NECK);
		return TRUE;
	}

	if (CAN_WEAR(obj, ITEM_WEAR_FACE)) {
		if (!remove_obj(ch, WEAR_FACE, fReplace))
			return FALSE;

		if (!pull_wear_triggers(ch, obj))
			return FALSE;

		act("$n wears $p on $s face.", ch, obj, NULL, TO_ROOM);
		act_puts("You wear $p on your face.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		equip_char(ch, obj, WEAR_FACE);
		return TRUE;
	}

	if (CAN_WEAR(obj, ITEM_WEAR_BODY)) {
		if (!remove_obj(ch, WEAR_BODY, fReplace))
			return FALSE;

		if (!pull_wear_triggers(ch, obj))
			return FALSE;

		act("$n wears $p on $s torso.", ch, obj, NULL, TO_ROOM);
		act_puts("You wear $p on your torso.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		equip_char(ch, obj, WEAR_BODY);
		return TRUE;
	}

	if (CAN_WEAR(obj, ITEM_WEAR_HEAD)) {
		if (!remove_obj(ch, WEAR_HEAD, fReplace))
			return FALSE;

		if (!pull_wear_triggers(ch, obj))
			return FALSE;

		act("$n wears $p on $s head.", ch, obj, NULL, TO_ROOM);
		act_puts("You wear $p on your head.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		equip_char(ch, obj, WEAR_HEAD);
		return TRUE;
	}

	if (CAN_WEAR(obj, ITEM_WEAR_LEGS)) {
		if (!remove_obj(ch, WEAR_LEGS, fReplace))
			return FALSE;

		if (!pull_wear_triggers(ch, obj))
			return FALSE;

		act("$n wears $p on $s legs.", ch, obj, NULL, TO_ROOM);
		act_puts("You wear $p on your legs.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		equip_char(ch, obj, WEAR_LEGS);
		return TRUE;
	}

	if (CAN_WEAR(obj, ITEM_WEAR_FEET)) {
		if (!remove_obj(ch, WEAR_FEET, fReplace))
			return FALSE;

		if (!pull_wear_triggers(ch, obj))
			return FALSE;

		act("$n wears $p on $s feet.", ch, obj, NULL, TO_ROOM);
		act_puts("You wear $p on your feet.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		equip_char(ch, obj, WEAR_FEET);
		return TRUE;
	}

	if (CAN_WEAR(obj, ITEM_WEAR_HANDS)) {
		if (!remove_obj(ch, WEAR_HANDS, fReplace))
			return FALSE;

		if (!pull_wear_triggers(ch, obj))
			return FALSE;

		act("$n wears $p on $s hands.", ch, obj, NULL, TO_ROOM);
		act_puts("You wear $p on your hands.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		equip_char(ch, obj, WEAR_HANDS);
		return TRUE;
	}

	if (CAN_WEAR(obj, ITEM_WEAR_ARMS)) {
		if (!remove_obj(ch, WEAR_ARMS, fReplace))
			return FALSE;

		if (!pull_wear_triggers(ch, obj))
			return FALSE;

		act("$n wears $p on $s arms.", ch, obj, NULL, TO_ROOM);
		act_puts("You wear $p on your arms.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		equip_char(ch, obj, WEAR_ARMS);
		return TRUE;
	}

	if (CAN_WEAR(obj, ITEM_WEAR_ABOUT)) {
		if (!remove_obj(ch, WEAR_ABOUT, fReplace))
			return FALSE;

		if (!pull_wear_triggers(ch, obj))
			return FALSE;

		act("$n wears $p on $s torso.", ch, obj, NULL, TO_ROOM);
		act_puts("You wear $p on your torso.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		equip_char(ch, obj, WEAR_ABOUT);
		return TRUE;
	}

	if (CAN_WEAR(obj, ITEM_WEAR_WAIST)) {
		if (!remove_obj(ch, WEAR_WAIST, fReplace))
			return FALSE;

		if (!pull_wear_triggers(ch, obj))
			return FALSE;

		act("$n wears $p about $s waist.", ch, obj, NULL, TO_ROOM);
		act_puts("You wear $p about your waist.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		equip_char(ch, obj, WEAR_WAIST);
		return TRUE;
	}

	if (CAN_WEAR(obj, ITEM_WEAR_WRIST)) {
		if (get_eq_char(ch, WEAR_WRIST_L) != NULL
		&&  get_eq_char(ch, WEAR_WRIST_R) != NULL
		&&  !remove_obj(ch, WEAR_WRIST_L, fReplace)
		&&  !remove_obj(ch, WEAR_WRIST_R, fReplace))
			return FALSE;

		if (get_eq_char(ch, WEAR_WRIST_L) == NULL) {
			if (!pull_wear_triggers(ch, obj))
				return FALSE;

			act("$n wears $p around $s left wrist.",
			    ch, obj, NULL, TO_ROOM);
			act_puts("You wear $p around your left wrist.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			equip_char(ch, obj, WEAR_WRIST_L);
			return TRUE;
		}

		if (get_eq_char(ch, WEAR_WRIST_R) == NULL) {
			if (!pull_wear_triggers(ch, obj))
				return FALSE;

			act("$n wears $p around $s right wrist.",
			    ch, obj, NULL, TO_ROOM);
			act_puts("You wear $p around your right wrist.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
			equip_char(ch, obj, WEAR_WRIST_R);
			return TRUE;
		}

		log(LOG_BUG, "wear_obj: no free wrist");
		act_char("You already wear two wrist items.", ch);
		return FALSE;
	}

	if (CAN_WEAR(obj, ITEM_WEAR_SHIELD)) {
		if (!remove_obj(ch, WEAR_SHIELD, fReplace))
			return FALSE;

		if (!free_hands(ch)) {
			act_puts("Your hands are full.",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
			return FALSE;
		}

		if (!pull_wear_triggers(ch, obj))
			return FALSE;

		act("$n wears $p as a shield.", ch, obj, NULL, TO_ROOM);
		act_puts("You wear $p as a shield.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		equip_char(ch, obj, WEAR_SHIELD);
		return TRUE;
	}

	if (CAN_WEAR(obj, ITEM_WIELD)) {
		OBJ_DATA *weapon;

		if (!IS_NPC(ch)
		&&  get_obj_weight(obj) > str_app[get_curr_stat(ch, STAT_STR)].wield) {
			act_char("It is too heavy for you to wield.", ch);
			return FALSE;
		}

		if (is_sn_affected(ch, "crippled hands")) {
			act("Your crippled hands refuse to hold $p.",
				ch, obj, NULL, TO_CHAR);
			return FALSE;
		}

		if ((weapon = get_eq_char(ch, WEAR_WIELD))
		&&  free_hands(ch) - need_hands(ch, weapon) < 2) {
			act_char("You need two hands free for that weapon.", ch);
			return FALSE;
		}

		if ((weapon = get_eq_char(ch, WEAR_SECOND_WIELD)))
			weapon->wear_loc = WEAR_NONE;

		if (!remove_obj(ch, WEAR_WIELD, fReplace)) {
			if (weapon)
				weapon->wear_loc = WEAR_SECOND_WIELD;
			return FALSE;
		}

		if (!pull_wear_triggers(ch, obj)) {
			if (weapon)
				weapon->wear_loc = WEAR_SECOND_WIELD;
			return FALSE;
		}

		act("$n wields $p.", ch, obj, NULL, TO_ROOM);
		act_puts("You wield $p.", ch, obj, NULL, TO_CHAR, POS_DEAD);
		equip_char(ch, obj, WEAR_WIELD);
		if (weapon)
			weapon->wear_loc = WEAR_SECOND_WIELD;
		show_weapon_skill(ch, obj);
		return TRUE;
	}

	if (CAN_WEAR(obj, ITEM_HOLD)) {
		if (!remove_obj(ch, WEAR_HOLD, fReplace))
			return FALSE;

		if (!free_hands(ch)) {
			act_puts("Your hands are full.",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
			return FALSE;
		}

		if (is_sn_affected(ch, "crippled hands")) {
			act("Your crippled hands refuse to hold $p.",
				ch, obj, NULL, TO_CHAR);
			return FALSE;
		}

		if (!pull_wear_triggers(ch, obj))
			return FALSE;

		act("$n holds $p in $s hand.", ch, obj, NULL, TO_ROOM);
		act_puts("You hold $p in your hand.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		equip_char(ch, obj, WEAR_HOLD);
		return TRUE;
	}

	if (CAN_WEAR(obj, ITEM_WEAR_FLOAT)) {
		if (!remove_obj(ch, WEAR_FLOAT, fReplace))
			return FALSE;

		if (!pull_wear_triggers(ch, obj))
			return FALSE;

		act("$n releases $p to float next to $m.",
		    ch, obj, NULL, TO_ROOM);
		act_puts("You release $p and it floats next to you.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		equip_char(ch, obj, WEAR_FLOAT);
		return TRUE;
	}

	if (CAN_WEAR(obj, ITEM_WEAR_TATTOO) && IS_IMMORTAL(ch)) {
		if (!remove_obj(ch, WEAR_TATTOO, fReplace))
			return FALSE;

		if (!pull_wear_triggers(ch, obj))
			return FALSE;

		act("$n now uses $p as tattoo of $s religion.",
		    ch, obj, NULL, TO_ROOM);
		act_puts("You now use $p as the tattoo of your religion.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		equip_char(ch, obj, WEAR_TATTOO);
		return TRUE;
	}

	if (CAN_WEAR(obj, ITEM_WEAR_CLANMARK)) {
		if (!remove_obj(ch, WEAR_CLANMARK, fReplace))
			return FALSE;

		if (!pull_wear_triggers(ch, obj))
			return FALSE;

		act("$n now uses $p as $s clan mark.",
		    ch, obj, NULL, TO_ROOM);
		act_puts("You now use $p as your clan mark.",
		    ch, obj, NULL, TO_CHAR, POS_DEAD);
		equip_char(ch, obj, WEAR_CLANMARK);
		return TRUE;
	}

	if (fReplace) {
		act("You can't wear, wield or hold $p.",
		    ch, obj, NULL, TO_CHAR);
	}

	return FALSE;
}

/*
 * Remove an object.
 */
bool
remove_obj(CHAR_DATA *ch, int iWear, bool fReplace)
{
	OBJ_DATA *obj;

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

	if (iWear == WEAR_STUCK_IN) {
		const char *wsn = get_weapon_sn(obj);

		unequip_char(ch, obj);

		if (obj->pObjIndex->item_type == ITEM_WEAPON
		&&  get_stuck_eq(ch, INT(obj->value[0])) == NULL) {
			if (is_sn_affected(ch, wsn))
				affect_strip(ch, wsn);
		}

		act_puts("You remove $p, in pain.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
		act("$n removes $p, in pain.", ch, obj, NULL, TO_ROOM);
                damage(ch, ch, dice(obj->level, 12),
		       NULL, DAM_OTHER, DAMF_NONE);
                WAIT_STATE(ch, 4);
		return !IS_EXTRACTED(ch);
	}

	if (pull_obj_trigger(TRIG_OBJ_REMOVE, obj, ch, NULL) > 0
	||  !mem_is(obj, MT_OBJ) || IS_EXTRACTED(ch))
		return FALSE;

	act("$n stops using $p.", ch, obj, NULL, TO_ROOM);
	act_puts("You stop using $p.", ch, obj, NULL, TO_CHAR, POS_DEAD);

	unequip_char(ch, obj);
	return TRUE;
}

bool
give_obj(CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj)
{
	int carry_w, carry_n;

	if (obj->wear_loc != WEAR_NONE) {
		act_char("You must remove it first.", ch);
		return FALSE;
	}

	if (IS_NPC(victim) && victim->pMobIndex->pShop != NULL
	&&  !MOB_HAS_TRIGGER(victim, TRIG_OBJ_GIVE)) {
		tell_char(victim, ch, "Sorry, you'll have to sell that.");
		return FALSE;
	}

	if (!can_drop_obj(ch, obj)) {
		act_char("You can't let go of it.", ch);
		return FALSE;
	}

	if ((carry_n = can_carry_n(victim)) >= 0
	&&  victim->carry_number + get_obj_number(obj) > carry_n) {
		act("$N has $S hands full.", ch, NULL, victim, TO_CHAR);
		return FALSE;
	}

	if ((carry_w = can_carry_w(victim)) >= 0
	&&  get_carry_weight(victim) + get_obj_weight(obj) > carry_w) {
		act("$N can't carry that much weight.",
		    ch, NULL, victim, TO_CHAR);
		return FALSE;
	}

	if (OBJ_IS(obj, OBJ_QUEST)
	&&  !IS_IMMORTAL(ch) && !IS_IMMORTAL(victim)) {
		act_puts("Even you are not that silly to give $p to $N.",
			 ch, obj, victim, TO_CHAR, POS_DEAD);
		return FALSE;
	}

	if (!can_see_obj(victim, obj)) {
		act("$N can't see it.", ch, NULL, victim, TO_CHAR);
		return FALSE;
	}

	obj->last_owner = NULL;

	obj_to_char(obj, victim);
	act("$n gives $p to $N.", ch, obj, victim, TO_NOTVICT | ACT_NOTRIG);
	act("$n gives you $p.", ch, obj, victim, TO_VICT | ACT_NOTRIG);
	act("You give $p to $N.", ch, obj, victim, TO_CHAR | ACT_NOTRIG);

	if (can_see(victim, ch)) {
		pull_mob_trigger(TRIG_MOB_GIVE, victim, ch, obj);
		if (IS_EXTRACTED(victim)
		||  IS_EXTRACTED(ch)
		||  !mem_is(obj, MT_OBJ))
			return FALSE;
	}

	pull_obj_trigger(TRIG_OBJ_GIVE, obj, ch, victim);
	if (!mem_is(obj, MT_OBJ) || IS_EXTRACTED(ch) || IS_EXTRACTED(victim))
		return FALSE;

	return TRUE;
}

bool
drop_obj(CHAR_DATA *ch, OBJ_DATA *obj)
{
	obj_to_room(obj, ch->in_room);

	act("$n drops $p.", ch, obj, NULL,
	    TO_ROOM | (HAS_INVIS(ch, ID_SNEAK) ? ACT_NOMORTAL : 0));
	act_puts("You drop $p.", ch, obj, NULL, TO_CHAR, POS_DEAD);

	pull_obj_trigger(TRIG_OBJ_DROP, obj, ch, NULL);
	if (!mem_is(obj, MT_OBJ) || IS_EXTRACTED(ch))
		return FALSE;

	if (obj->in_room == NULL)
		return TRUE;

	if (obj->pObjIndex->vnum == OBJ_VNUM_POTION_VIAL
	&&  number_percent() < 51) {
		switch (ch->in_room->sector_type) {
		case SECT_FOREST:
		case SECT_DESERT:
		case SECT_AIR:
		case SECT_WATER_NOSWIM:
		case SECT_WATER_SWIM:
		case SECT_FIELD:
			break;
		default:
			act("$p cracks and shaters into tiny pieces.",
			    ch, obj, NULL, TO_ROOM);
			act("$p cracks and shaters into tiny pieces.",
			    ch, obj, NULL, TO_CHAR);
			extract_obj(obj, 0);
			return FALSE;
		}
	}

	if (!floating_time(obj) && IS_WATER(ch->in_room)) {
		act("$p sinks down the water.", ch, obj, NULL,
		    TO_ROOM | (HAS_INVIS(ch, ID_SNEAK) ? ACT_NOMORTAL : 0));
		act("$p sinks down the water.", ch, obj, NULL, TO_CHAR);
		extract_obj(obj, 0);
		return FALSE;
	} else if (IS_OBJ_STAT(obj, ITEM_MELT_DROP)) {
		act("$p dissolves into smoke.", ch, obj, NULL,
		    TO_ROOM | (HAS_INVIS(ch, ID_SNEAK) ? ACT_NOMORTAL : 0));
		act("$p dissolves into smoke.", ch, obj, NULL, TO_CHAR);
		extract_obj(obj, 0);
		return FALSE;
	}

	return TRUE;
}

static
FOREACH_CB_FUN(pull_mob_open_cb, p, ap)
{
	CHAR_DATA *vch = (CHAR_DATA *) p;

	CHAR_DATA *ch = va_arg(ap, CHAR_DATA *);
	OBJ_DATA *obj = va_arg(ap, OBJ_DATA *);

	if (pull_mob_trigger(TRIG_MOB_OPEN, vch, ch, obj) > 0
	||  !mem_is(obj, MT_OBJ) || IS_EXTRACTED(ch))
		return p;

	return NULL;
}

bool
open_obj(CHAR_DATA *ch, OBJ_DATA *obj)
{
	if (obj->item_type == ITEM_PORTAL) {
		/* open portal */
		if (!IS_SET(INT(obj->value[1]), EX_ISDOOR)) {
			act_char("You can't do that.", ch);
			return FALSE;
		}

		if (!IS_SET(INT(obj->value[1]), EX_CLOSED)) {
			act_char("It's already open.", ch);
			return FALSE;
		}

		if (IS_SET(INT(obj->value[1]), EX_LOCKED)) {
			act_char("It's locked.", ch);
			return FALSE;
		}
	} else {
		/* 'open object' */
		if (obj->item_type != ITEM_CONTAINER) {
			act_char("That's not a container.", ch);
			return FALSE;
		}

		if (!IS_SET(INT(obj->value[1]), CONT_CLOSED)) {
			act_char("It's already open.", ch);
			return FALSE;
		}

		if (!IS_SET(INT(obj->value[1]), CONT_CLOSEABLE)) {
			act_char("You can't do that.", ch);
			return FALSE;
		}

		if (IS_SET(INT(obj->value[1]), CONT_LOCKED)) {
			act_char("It's locked.", ch);
			return FALSE;
		}
	}

	if (!IS_NPC(ch)) {
		if (vo_foreach(ch->in_room, &iter_char_room,
			       pull_mob_open_cb, ch, obj))
			return FALSE;
	}

	if (pull_obj_trigger(TRIG_OBJ_OPEN, obj, ch, NULL) > 0
	||  IS_EXTRACTED(ch) || !mem_is(obj, MT_OBJ))
		return FALSE;

	act_puts("You open $p.", ch, obj, NULL, TO_CHAR, POS_DEAD);
	act("$n opens $p.", ch, obj, NULL, TO_ROOM);

	if (obj->item_type == ITEM_PORTAL)
		REMOVE_BIT(INT(obj->value[1]), EX_CLOSED);
	else
		REMOVE_BIT(INT(obj->value[1]), CONT_CLOSED);

	return TRUE;
}

bool
close_obj(CHAR_DATA *ch, OBJ_DATA *obj)
{
	if (obj->item_type == ITEM_PORTAL) {
		/* portal stuff */
		if (!IS_SET(INT(obj->value[1]), EX_ISDOOR)
		||  IS_SET(INT(obj->value[1]), EX_NOCLOSE)) {
			act_char("You can't do that.", ch);
			return FALSE;
		}

		if (IS_SET(INT(obj->value[1]), EX_CLOSED)) {
			act_char("It's already closed.", ch);
			return FALSE;
		}
	} else {
		/* 'close object' */
		if (obj->item_type != ITEM_CONTAINER) {
			act_char("That's not a container.", ch);
			return FALSE;
		}

		if (IS_SET(INT(obj->value[1]), CONT_CLOSED)) {
			act_char("It's already closed.", ch);
			return FALSE;
		}

		if (!IS_SET(INT(obj->value[1]), CONT_CLOSEABLE)) {
			act_char("You can't do that.", ch);
			return FALSE;
		}
	}

	if (pull_obj_trigger(TRIG_OBJ_CLOSE, obj, ch, NULL) > 0
	||  IS_EXTRACTED(ch) || !mem_is(obj, MT_OBJ))
		return FALSE;

	if (obj->item_type == ITEM_PORTAL)
		SET_BIT(INT(obj->value[1]), EX_CLOSED);
	else
		SET_BIT(INT(obj->value[1]), CONT_CLOSED);

	act_puts("You close $p.", ch, obj, NULL, TO_CHAR, POS_DEAD);
	act("$n closes $p.", ch, obj, NULL, TO_ROOM);

	return TRUE;
}

bool
lock_obj(CHAR_DATA *ch, OBJ_DATA *obj)
{
	if (obj->item_type == ITEM_PORTAL) {
		/* portal stuff */
		if (!IS_SET(INT(obj->value[1]), EX_ISDOOR)
		||  IS_SET(INT(obj->value[1]), EX_NOCLOSE)) {
			act_char("You can't do that.", ch);
			return FALSE;
		}

		if (!IS_SET(INT(obj->value[1]), EX_CLOSED)) {
			act_char("It's not closed.", ch);
			return FALSE;
		}

		if (INT(obj->value[4]) < 0
		||  IS_SET(INT(obj->value[1]), EX_NOLOCK)) {
			act_char("It can't be locked.", ch);
			return FALSE;
		}

		if (!has_key(ch, INT(obj->value[4]))) {
			act_char("You lack the key.", ch);
			return FALSE;
		}

		if (IS_SET(INT(obj->value[1]), EX_LOCKED)) {
			act_char("It's already locked.", ch);
			return FALSE;
		}
	} else {
		/* 'lock object' */
		if (obj->item_type != ITEM_CONTAINER) {
			act_char("That's not a container.", ch);
			return FALSE;
		}

		if (!IS_SET(INT(obj->value[1]), CONT_CLOSED)) {
			act_char("It's not closed.", ch);
			return FALSE;
		}

		if (INT(obj->value[2]) < 0) {
			act_char("It can't be locked.", ch);
			return FALSE;
		}

		if (!has_key(ch, INT(obj->value[2]))) {
			act_char("You lack the key.", ch);
			return FALSE;
		}

		if (IS_SET(INT(obj->value[1]), CONT_LOCKED)) {
			act_char("It's already locked.", ch);
			return FALSE;
		}
	}

	if (pull_obj_trigger(TRIG_OBJ_LOCK, obj, ch, NULL) > 0
	||  IS_EXTRACTED(ch) || !mem_is(obj, MT_OBJ))
		return FALSE;

	if (obj->item_type == ITEM_PORTAL)
		SET_BIT(INT(obj->value[1]), EX_LOCKED);
	else
		SET_BIT(INT(obj->value[1]), CONT_LOCKED);

	act_puts("You lock $p.", ch, obj, NULL, TO_CHAR, POS_DEAD);
	act("$n locks $p.", ch, obj, NULL, TO_ROOM);

	return TRUE;
}

bool
unlock_obj(CHAR_DATA *ch, OBJ_DATA *obj)
{
	if (obj->item_type == ITEM_PORTAL) {
		/* portal stuff */
		if (!IS_SET(INT(obj->value[1]), EX_ISDOOR)) {
			act_char("You can't do that.", ch);
			return FALSE;
		}

		if (!IS_SET(INT(obj->value[1]),EX_CLOSED)) {
			act_char("It's not closed.", ch);
			return FALSE;
		}

		if (INT(obj->value[4]) < 0) {
			act_char("It can't be unlocked.", ch);
			return FALSE;
		}

		if (!has_key(ch, INT(obj->value[4]))) {
			act_char("You lack the key.", ch);
			return FALSE;
		}

		if (!IS_SET(INT(obj->value[1]), EX_LOCKED)) {
			act_char("It's already unlocked.", ch);
			return FALSE;
		}
	} else {
		/* 'unlock object' */
		if (obj->item_type != ITEM_CONTAINER) {
			act_char("That's not a container.", ch);
			return FALSE;
		}

		if (!IS_SET(INT(obj->value[1]), CONT_CLOSED)) {
			act_char("It's not closed.", ch);
			return FALSE;
		}

		if (INT(obj->value[2]) < 0) {
			act_char("It can't be unlocked.", ch);
			return FALSE;
		}

		if (!has_key(ch, INT(obj->value[2]))) {
			act_char("You lack the key.", ch);
			return FALSE;
		}

		if (!IS_SET(INT(obj->value[1]), CONT_LOCKED)) {
			act_char("It's already unlocked.", ch);
			return FALSE;
		}
	}

	if (pull_obj_trigger(TRIG_OBJ_UNLOCK, obj, ch, NULL) > 0
	||  IS_EXTRACTED(ch) || !mem_is(obj, MT_OBJ))
		return FALSE;

	if (obj->item_type == ITEM_PORTAL)
		REMOVE_BIT(INT(obj->value[1]),EX_LOCKED);
	else
		REMOVE_BIT(INT(obj->value[1]), CONT_LOCKED);

	act_puts("You unlock $p.", ch, obj, NULL, TO_CHAR, POS_DEAD);
	act("$n unlocks $p.", ch, obj, NULL, TO_ROOM);

	return TRUE;
}

bool
open_door(CHAR_DATA *ch, const char *name)
{
	int door;
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	if ((door = find_door(ch, name)) < 0)
		return FALSE;

	pexit = ch->in_room->exit[door];
	if (!IS_SET(pexit->exit_info, EX_CLOSED)) {
		act("It's already open.",
		    ch, &pexit->short_descr, NULL, TO_CHAR);
		return FALSE;
	}

	if (IS_SET(pexit->exit_info, EX_LOCKED)) {
		act("It's locked.", ch, &pexit->short_descr, NULL, TO_CHAR);
		return FALSE;
	}

	REMOVE_BIT(pexit->exit_info, EX_CLOSED);
	act("$n opens $d.", ch, &pexit->short_descr, NULL, TO_ROOM);
	act_char("Ok.", ch);

	/* open the other side */
	if ((to_room = pexit->to_room.r) != NULL
	&&  (pexit_rev = to_room->exit[rev_dir[door]]) != NULL
	&&  pexit_rev->to_room.r == ch->in_room) {
		ROOM_INDEX_DATA *in_room;

		REMOVE_BIT(pexit_rev->exit_info, EX_CLOSED);

		in_room = ch->in_room;
		ch->in_room = to_room;
		act("$d opens.", ch, &pexit_rev->short_descr, NULL, TO_ROOM);
		ch->in_room = in_room;
	}

	return TRUE;
}

bool
close_door(CHAR_DATA *ch, const char *name)
{
	int door;
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	if ((door = find_door(ch, name)) < 0)
		return FALSE;

	pexit = ch->in_room->exit[door];
	if (IS_SET(pexit->exit_info, EX_CLOSED)) {
		act("It's already closed.",
		    ch, &pexit->short_descr, NULL, TO_CHAR);
		return FALSE;
	}

	SET_BIT(pexit->exit_info, EX_CLOSED);
	act("$n closes $d.", ch, &pexit->short_descr, NULL, TO_ROOM);
	act_char("Ok.", ch);

	/* close the other side */
	if ((to_room = pexit->to_room.r) != NULL
	&&  (pexit_rev = to_room->exit[rev_dir[door]]) != 0
	&&  pexit_rev->to_room.r == ch->in_room) {
		ROOM_INDEX_DATA *in_room;

		SET_BIT(pexit_rev->exit_info, EX_CLOSED);
		in_room = ch->in_room;
		ch->in_room = to_room;
		act("$d closes.", ch, &pexit_rev->short_descr, NULL, TO_ROOM);
		ch->in_room = in_room;
	}

	return TRUE;
}

bool
lock_door(CHAR_DATA *ch, const char *name)
{
	int door;
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	if ((door = find_door(ch, name)) < 0)
		return FALSE;

	pexit = ch->in_room->exit[door];
	if (!IS_SET(pexit->exit_info, EX_CLOSED)) {
		act("It's not closed.",
		    ch, &pexit->short_descr, NULL, TO_CHAR);
		return FALSE;
	}

	if (pexit->key < 0) {
		act("It can't be locked.",
		    ch, &pexit->short_descr, NULL, TO_CHAR);
		return FALSE;
	}

	if (!has_key(ch, pexit->key)
	&&  !has_key_ground(ch, pexit->key)) {
		act("You lack the key.", ch, NULL, NULL, TO_CHAR);
		return FALSE;
	}

	if (IS_SET(pexit->exit_info, EX_LOCKED)) {
		act("It's already locked.",
		    ch, &pexit->short_descr, NULL, TO_ROOM);
		return FALSE;
	}

	SET_BIT(pexit->exit_info, EX_LOCKED);
	act_char("*Click*", ch);
	act("$n locks $d.", ch, &pexit->short_descr, NULL, TO_ROOM);

	/* lock the other side */
	if ((to_room = pexit->to_room.r) != NULL
	&&  (pexit_rev = to_room->exit[rev_dir[door]]) != 0
	&&  pexit_rev->to_room.r == ch->in_room) {
		ROOM_INDEX_DATA *in_room;

		SET_BIT(pexit_rev->exit_info, EX_LOCKED);

		in_room = ch->in_room;
		ch->in_room = to_room;
		act("$d clicks.",
		    ch, &pexit_rev->short_descr, NULL, TO_ROOM);
		ch->in_room  = in_room;
	}

	return TRUE;
}

bool
unlock_door(CHAR_DATA *ch, const char *name)
{
	int door;
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;

	if ((door = find_door(ch, name)) < 0)
		return FALSE;

	pexit = ch->in_room->exit[door];
	if (!IS_SET(pexit->exit_info, EX_CLOSED)) {
		act("It's not closed.",
		    ch, &pexit->short_descr, NULL, TO_CHAR);
		return FALSE;
	}

	if (pexit->key < 0) {
		act("It can't be unlocked.",
		    ch, &pexit->short_descr, NULL, TO_CHAR);
		return FALSE;
	}

	if (!has_key(ch, pexit->key)
	&&  !has_key_ground(ch, pexit->key)) {
		act("You lack the key.", ch, NULL, NULL, TO_CHAR);
		return FALSE;
	}

	if (!IS_SET(pexit->exit_info, EX_LOCKED)) {
		act("It's already unlocked.",
		    ch, &pexit->short_descr, NULL, TO_CHAR);
		return FALSE;
	}

	REMOVE_BIT(pexit->exit_info, EX_LOCKED);
	act_char("*Click*", ch);
	act("$n unlocks $d.", ch, &pexit->short_descr, NULL, TO_ROOM);

	/* unlock the other side */
	if ((to_room = pexit->to_room.r) != NULL
	&&  (pexit_rev = to_room->exit[rev_dir[door]]) != NULL
	&&  pexit_rev->to_room.r == ch->in_room) {
		ROOM_INDEX_DATA *in_room;

		REMOVE_BIT(pexit_rev->exit_info, EX_LOCKED);

		in_room = ch->in_room;
		ch->in_room = to_room;
		act("$d clicks.",
		    ch, &pexit_rev->short_descr, NULL, TO_ROOM);
		ch->in_room = in_room;
	}

	return TRUE;
}

void
look_char(CHAR_DATA *ch, CHAR_DATA *victim)
{
	OBJ_DATA *obj;
	int i;
	int percent;
	bool found;
	const char *msg;
	const char *desc;
	CHAR_DATA *doppel = victim;
	CHAR_DATA *mirror = victim;
	char buf[MAX_STRING_LENGTH];

	if (is_sn_affected(victim, "doppelganger")) {
		if (IS_NPC(ch) || !IS_SET(PC(ch)->plr_flags, PLR_HOLYLIGHT)) {
			doppel = victim->doppel;
			if (is_sn_affected(victim, "mirror"))
				mirror = victim->doppel;
		}
	}

	if (can_see(victim, ch)) {
		if (ch == victim)
			act("$n looks at $mself.",
			    ch, NULL, NULL, TO_ROOM);
		else {
			act_puts("$n looks at you.",
				 ch, NULL, victim, TO_VICT, POS_RESTING);
			act("$n looks at $N.",
			    ch, NULL, victim, TO_NOTVICT);
		}
	}

	if (is_sn_affected(ch, "hallucination") && !IS_NPC(ch))
		doppel = nth_char(doppel, PC(ch)->random_value);

	if (doppel->shapeform)
		desc = mlstr_cval(&doppel->shapeform->index->description, ch);
	else if (IS_NPC(doppel))
		desc = mlstr_cval(&doppel->description, ch);
	else
		desc = mlstr_mval(&doppel->description);

	if (!IS_NULLSTR(desc)) {
		if (doppel->shapeform || IS_NPC(doppel)) {
			act_puts(desc, ch, NULL, NULL,
				 TO_CHAR | ACT_NOLF, POS_DEAD);
		} else {
			act_puts("$t{x", ch, desc, NULL,	// notrans
				 TO_CHAR | ACT_NOLF, POS_DEAD);
		}
	} else {
		act_puts("You see nothing special about $m.",
			 victim, NULL, ch, TO_VICT, POS_DEAD);
	}

	if (MOUNTED(victim)) {
		act_puts("$N is riding $i.",
			 ch, MOUNTED(victim), victim, TO_CHAR, POS_DEAD);
	}
	if (RIDDEN(victim)) {
		act_puts("$N is being ridden by $i.",
			 ch, RIDDEN(victim), victim, TO_CHAR, POS_DEAD);
	}

	if (victim->max_hit > 0)
		percent = (100 * victim->hit) / victim->max_hit;
	else
		percent = -1;

	if (percent >= 100)
		msg = "{Cis in perfect health{x.";
	else if (percent >= 90)
		msg = "{bhas a few scratches{x.";
	else if (percent >= 75)
		msg = "{Bhas some small but disgusting cuts{x.";
	else if (percent >= 50)
		msg = "{Gis covered with bleeding wounds{x.";
	else if (percent >= 30)
		msg = "{Yis gushing blood{x.";
	else if (percent >= 15)
		msg = "{Mis writhing in agony{x.";
	else if (percent >= 0)
		msg = "{Ris convulsing on the ground{x.";
	else
		msg = "{Ris nearly dead{x.";

	/* vampire ... */
	/* XXX should not be here, should be called from do_look or whatever */
	if (percent < 90 && IS_VAMPIRE(ch))
		gain_condition(ch, COND_BLOODLUST, -1);

	if (!IS_IMMORTAL(doppel)) {
		act_puts("($t) ", ch, doppel->race, NULL,	// notrans
			 TO_CHAR | ACT_NOTRANS | ACT_NOLF, POS_DEAD);
		if (!IS_NPC(doppel)) {
			act_puts("($t) ($T) ", ch,		// notrans
				 doppel->class, mlstr_mval(&doppel->gender),
				 TO_CHAR | ACT_NOTRANS | ACT_NOLF, POS_DEAD);
		}
	}

	strnzcpy(buf, sizeof(buf), PERS(victim, ch, GET_LANG(ch), ACT_FORMSH));
	cstrtoupper(buf);
	if (IS_IMMORTAL(victim))
		send_to_char("{W", ch);				// notrans
	act_puts("$N", ch, NULL, victim,			// notrans
		 TO_CHAR | ACT_NOLF | ACT_FORMSH, POS_DEAD);
	if (IS_IMMORTAL(victim))
		send_to_char("{x", ch);
	act_puts(" $t", ch, msg, NULL, TO_CHAR, POS_DEAD);	// notrans

	found = FALSE;
	for (i = 0; show_order[i] != -1; i++) {
		if ((obj = get_eq_char(mirror, show_order[i]))
		&&  can_see_obj(ch, obj)) {
			if (!found) {
				send_to_char("\n", ch);
				act("$N is using:", ch, NULL, victim, TO_CHAR);
				found = TRUE;
			}

			show_obj_to_char(ch, obj, show_order[i]);
		}
	}

	for (obj = mirror->carrying; obj; obj = obj->next_content) {
		if (obj->wear_loc == WEAR_STUCK_IN
		&&  can_see_obj(ch, obj)) {
			if (!found) {
				send_to_char("\n", ch);
				act("$N is using:", ch, NULL, victim, TO_CHAR);
				found = TRUE;
			}

			show_obj_to_char(ch, obj, WEAR_STUCK_IN);
		}
	}

	if (victim != ch
	&&  (!IS_IMMORTAL(victim) || IS_IMMORTAL(ch))
	&&  !IS_NPC(ch)
	&&  number_percent() < get_skill(ch, "peek")) {
		send_to_char("\n", ch);
		act_char("You peek at the inventory:", ch);
		check_improve(ch, "peek", TRUE, 4);
		show_list_to_char(
		    mirror->carrying, ch, FO_F_SHORT | FO_F_SHOW_NOTHING);
	}

	/* Love potion */
	if (is_sn_affected(ch, "love potion") && (victim != ch)) {
		AFFECT_DATA *paf;

		affect_strip(ch, "love potion");

		add_follower(ch, victim);
		set_leader(ch, victim);

		paf = aff_new(TO_AFFECTS, "charm person");
		paf->level = ch->level;
		paf->duration =  number_fuzzy(victim->level / 4);
		paf->bitvector = AFF_CHARM;
		affect_to_char(ch, paf);
		aff_free(paf);

		act("Isn't $n just so nice?",
		    victim, NULL, ch, TO_VICT);
		act("$N looks at you with adoring eyes.",
		    victim, NULL, ch, TO_CHAR);
		act("$N looks at $n with adoring eyes.",
		    victim, NULL, ch, TO_NOTVICT);
	}

	if (can_see(victim, ch))
		pull_mob_trigger(TRIG_MOB_LOOK, victim, ch, NULL);
}

bool
transfer_char(CHAR_DATA *ch, ROOM_INDEX_DATA *room)
{
	if (room_is_private(room))
		return FALSE;

	char_to_room(ch, room);
	if (!IS_EXTRACTED(ch))
		dofun("look", ch, "auto");
	return TRUE;
}

void
social_char(const char *socname, CHAR_DATA *ch, CHAR_DATA *victim)
{
	social_t *soc = social_lookup(socname);

	if (soc == NULL) {
		log(LOG_BUG, "%s: %s: unknown social", __FUNCTION__, socname);
		return;
	}

	interpret_social_char(soc, ch, victim);
}

/*
 * Find an obj in a list.
 */
static OBJ_DATA *
get_obj_list_raw(CHAR_DATA *ch, const char *name,
		 uint *number, OBJ_DATA *list, int flags)
{
	OBJ_DATA *obj;
	int vnum = 0;

	if ((IS_NPC(ch) || IS_IMMORTAL(ch))
	&&  is_number(name))
		vnum = atoi(name);

	for (obj = list; obj; obj = obj->next_content) {
		if (!can_see_obj(ch, obj))
			continue;

		if (vnum) {
			if (vnum != obj->pObjIndex->vnum)
				continue;
		} else if (!IS_OBJ_NAME(obj, name))
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
static OBJ_DATA *
get_obj_here_raw(CHAR_DATA *ch, const char *name, uint *number)
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
OBJ_DATA *
get_obj_carry(CHAR_DATA *ch, CHAR_DATA *victim, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	uint number;

	number = number_argument(argument, arg, sizeof(arg));
	if (!number || arg[0] == '\0')
		return NULL;

	return get_obj_list_raw(
	    ch, arg, &number, victim->carrying, GETOBJ_F_INV);
}

/*
 * Find an obj in player's equipment.
 */
OBJ_DATA *
get_obj_wear(CHAR_DATA *ch, CHAR_DATA *victim, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	uint number;

	number = number_argument(argument, arg, sizeof(arg));
	if (!number || arg[0] == '\0')
		return NULL;

	return get_obj_list_raw(
	    ch, arg, &number, victim->carrying, GETOBJ_F_WORN);
}

OBJ_DATA *
get_obj_obj(CHAR_DATA *ch, OBJ_DATA *obj, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	uint number;

	number = number_argument(argument, arg, sizeof(arg));
	if (!number || arg[0] == '\0')
		return NULL;

	return get_obj_list_raw(ch, arg, &number, obj->contains, 0);
}

/*
 * Find an obj in the room or in inventory.
 */
OBJ_DATA *
get_obj_here(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	uint number;

	number = number_argument(argument, arg, sizeof(arg));
	if (!number || arg[0] == '\0')
		return NULL;

	return get_obj_here_raw(ch, arg, &number);
}

OBJ_DATA *
get_obj_here_all(CHAR_DATA *ch, const char *argument)
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
		if (!can_see(ch, vch))
			continue;

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

OBJ_DATA *
get_obj_room(CHAR_DATA *ch, ROOM_INDEX_DATA *room, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	uint number;

	number = number_argument(argument, arg, sizeof(arg));
	if (!number || arg[0] == '\0')
		return NULL;

	return get_obj_list_raw(
	    ch, arg, &number, room->contents, GETOBJ_F_ANY);
}

/*
 * Find an obj in the world.
 */
OBJ_DATA *
get_obj_world(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	uint number;
	int vnum = 0;

	number = number_argument(argument, arg, sizeof(arg));
	if (!number || arg[0] == '\0')
		return NULL;

	if ((obj = get_obj_here_raw(ch, arg, &number)))
		return obj;

	if ((IS_NPC(ch) || IS_IMMORTAL(ch))
	&&  is_number(arg))
		vnum = atoi(arg);

	for (obj = object_list; obj; obj = obj->next) {
		if (vnum) {
			if (vnum != obj->pObjIndex->vnum)
				continue;
		} else if (!IS_OBJ_NAME(obj, arg))
			continue;

		if (can_see_obj(ch, obj)
		&&  obj->carried_by != ch
		&&  !--number)
			return obj;
	}

	return NULL;
}

/*
 * add_mind - remember 'str' in mind buffer of 'ch'
 *	      remember the place to return in mind buffer if it is empty
 */
void
add_mind(CHAR_DATA *ch, const char *str)
{
	NPC_DATA *npc;

	if (!IS_NPC(ch)) {
		log(LOG_BUG, "add_mind: !IS_NPC");
		return;
	}

	if ((npc = NPC(ch))->in_mind == NULL) {
		/* remember a place to return */
		npc->in_mind = str_printf("%d", ch->in_room->vnum);
	}

	name_add(&npc->in_mind, str, NULL, NULL);
}

/*
 * remove_mind - remove 'str' from mind buffer of 'ch'
 *		 if it was the last revenge - return home
 */
void
remove_mind(CHAR_DATA *ch, const char *str)
{
	NPC_DATA *npc;

	if (!IS_NPC(ch)) {
		log(LOG_BUG, "remove_mind: !IS_NPC");
		return;
	}

	if (!name_delete(&(npc = NPC(ch))->in_mind, str, NULL, NULL))
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

void
back_home(CHAR_DATA *ch)
{
	NPC_DATA *npc;
	ROOM_INDEX_DATA *location;
	char arg[MAX_INPUT_LENGTH];

	if (!IS_NPC(ch)) {
		log(LOG_BUG, "back_home: !IS_NPC");
		return;
	}

	if ((npc = NPC(ch))->in_mind == NULL
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

/*
 * returns TRUE if lch is one of the leaders of ch
 */
bool
is_leader(CHAR_DATA *ch, CHAR_DATA *lch)
{
	for (ch = ch->leader; ch; ch = ch->leader) {
		if (ch == lch)
			return TRUE;
	}

	return FALSE;
}

void
set_leader(CHAR_DATA *ch, CHAR_DATA *lch)
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

CHAR_DATA *
leader_lookup(CHAR_DATA* ch)
{
	CHAR_DATA* res;
	for (res = ch; res->leader != NULL; res = res->leader)
		;
	return res;
}

bool
is_same_group(CHAR_DATA *ach, CHAR_DATA *bch)
{
	return leader_lookup(ach) == leader_lookup(bch);
}

void
add_follower(CHAR_DATA *ch, CHAR_DATA *master)
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

void
stop_follower(CHAR_DATA *ch)
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

void
die_follower(CHAR_DATA *ch)
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

void
damage_to_obj(CHAR_DATA *ch, OBJ_DATA *wield, OBJ_DATA *worn, int dmg)
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

bool
make_eq_damage(CHAR_DATA *ch, CHAR_DATA *victim, int loc_wield, int loc_destroy)
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

bool
random_eq_damage(CHAR_DATA *ch, CHAR_DATA *victim, int loc_wield)
{
	int i;

	do {
		i = number_range(0, MAX_WEAR - 1);
	} while (i == WEAR_WIELD || i == WEAR_SECOND_WIELD ||
		 i == WEAR_TATTOO || i == WEAR_STUCK_IN ||
		 i == WEAR_SHIELD || i == WEAR_CLANMARK);

	return make_eq_damage(ch, victim, loc_wield, i);
}

int
need_hands(CHAR_DATA *ch, OBJ_DATA *weapon)
{
	int need = 1;

	if (WEAPON_IS_LONG(weapon)
	||  (IS_WEAPON_STAT(weapon, WEAPON_TWO_HANDS) &&
	     ch->size < SIZE_LARGE))
		need++;

	return need;
}

int
free_hands(CHAR_DATA *ch)
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

int
exit_lookup(const char *arg)
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

int
find_door_nomessage(CHAR_DATA *ch, const char *arg)
{
	int door = exit_lookup(arg);

	if (door >= 0)
		return door;

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

int
find_door(CHAR_DATA *ch, const char *arg)
{
	EXIT_DATA *pexit;
	int door;

	if ((door = find_door_nomessage(ch, arg)) < 0)
		return door;

	/*
	 * the following two if's can be TRUE only
	 * if direction was matched in `find_door_nomessage'
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

void
teleport_char(CHAR_DATA *ch, CHAR_DATA *vch, ROOM_INDEX_DATA *to_room,
	      const char *msg_out, const char *msg_travel, const char *msg_in)
{
	act(msg_out, ch, NULL, NULL, TO_ROOM);
	if (ch != vch)
		act_puts(msg_travel, ch, NULL, vch, TO_CHAR, POS_DEAD);

	char_to_room(ch, to_room);

	if (!IS_EXTRACTED(ch)) {
		act(msg_in, ch, NULL, NULL, TO_ROOM);
		dofun("look", ch, "auto");
	}
}

void
recall(CHAR_DATA *ch, ROOM_INDEX_DATA *location)
{
	teleport_char(ch, NULL, location,
		      "$n disappears.", NULL, "$n appears in the room.");
}

bool
can_gate(CHAR_DATA *ch, CHAR_DATA *victim)
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

/*
 * True if char can drop obj.
 */
bool
can_drop_obj(CHAR_DATA *ch, OBJ_DATA *obj)
{
	if (!IS_OBJ_STAT(obj, ITEM_NODROP))
		return TRUE;

	if (IS_IMMORTAL(ch))
		return TRUE;

	return FALSE;
}

/* RT part of the corpse looting code */
bool
can_loot(CHAR_DATA *ch, OBJ_DATA *obj)
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

bool
shapeshift(CHAR_DATA *ch, const char *shapeform)
{
	form_index_t *form_index;
	form_t *form;
	int i;

	if ((form_index = form_lookup(shapeform)) == NULL) {
		log(LOG_BUG, "shapeshift: unknown form %s.\n", shapeform);
		return FALSE;
	}

	form = (form_t *) calloc(1, sizeof(form_t));

	form->index = form_index;
	form->damroll = form_index->damage[DICE_BONUS];
	form->hitroll = form_index->hitroll;

	for (i = 0; i < MAX_RESIST; i++)
		form->res_mod[i] = 0;

	ch->shapeform = form;

	return TRUE;
}

bool
revert(CHAR_DATA *ch)
{
	AFFECT_DATA *paf;
	AFFECT_DATA *paf_next;

	for (paf = ch->affected; paf; paf = paf_next) {
		paf_next = paf->next;
		if (paf->where == TO_FORMAFFECTS)
			affect_remove(ch, paf);
	}

	if (!ch->shapeform) {
		log(LOG_BUG, "Revert: character is not shapeshifted.\n");
		return FALSE;
	}

	free(ch->shapeform);
	ch->shapeform = NULL;

	return TRUE;
}

void
clan_save(const char *cln)
{
	clan_t *clan;

	if ((clan = clan_lookup(cln)) == NULL) {
		log(LOG_BUG, "clan_save: %s: unknown clan", cln);
		return;
	}

	SET_BIT(clan->clan_flags, CLAN_CHANGED);
	dofun("asave", NULL, "clans");				// notrans
}

/*
 * clan_update_lists - remove 'victim' from leader and second lists of 'clan'
 *		       if memb is TRUE 'victim' will be deleted from members
 *		       list
 */
void
clan_update_lists(const char *cln, CHAR_DATA *victim, bool memb)
{
	const char **nl = NULL;
	clan_t* clan;

	if ((clan = clan_lookup(cln)) == NULL) {
		log(LOG_BUG, "clan_update_lists: %s: unknown clan", cln);
		return;
	}

	switch (PC(victim)->clan_status) {
	case CLAN_SECOND:
		nl = &clan->second_list;
		break;

	case CLAN_LEADER:
		nl = &clan->leader_list;
		break;
	}
	if (nl)
		name_delete(nl, victim->name, NULL, NULL);

	if (memb)
		name_delete(&clan->member_list, victim->name, NULL, NULL);
}

static void *
item_ok_cb(void *p, va_list ap)
{
	clan_t *clan = (clan_t *) p;

	int room_in_vnum = va_arg(ap, int);

	if (room_in_vnum == clan->altar_vnum)
		return p;
	return NULL;
}

bool
clan_item_ok(const char *cln)
{
	clan_t* clan;
	OBJ_DATA* obj;
	int room_in;

	if ((clan = clan_lookup(cln)) == NULL
	||  clan->obj_ptr == NULL)
		return TRUE;

	for (obj = clan->obj_ptr; obj->in_obj != NULL; obj = obj->in_obj)
		;

	if (obj->in_room)
		room_in=obj->in_room->vnum;
	else
		return TRUE;

	if (room_in == clan->altar_vnum)
		return TRUE;

	if (c_foreach(&clans, item_ok_cb, room_in) != NULL)
		return FALSE;

	return TRUE;
}

/*
 * Parse a name for acceptability.
 */
bool
pc_name_ok(const char *name)
{
	const u_char *pc;
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

bool
char_in_dark_room(CHAR_DATA *ch)
{
	ROOM_INDEX_DATA *pRoomIndex = ch->in_room;

	if (!IS_NPC(ch) && IS_SET(PC(ch)->plr_flags, PLR_HOLYLIGHT))
		return FALSE;

	if (is_sn_affected(ch, "vampire"))
		return FALSE;

	return room_is_dark(pRoomIndex);
}

void
do_who_raw(CHAR_DATA* ch, CHAR_DATA *wch, BUFFER* output)
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
		buf_append(output, "{c[AFK]{x ");	// notrans

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

int
isn_dark_safe(CHAR_DATA *ch)
{
	CHAR_DATA *rch;
	OBJ_DATA *light;
	int light_exist;

	if (!is_sn_affected(ch, "vampire")
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

void
format_obj(BUFFER *output, OBJ_DATA *obj)
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
			   "This equipment has been LIMITED by number %d.\n",
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
			buf_printf(output, BUF_END, "Weapon flags: %s.\n",
				   SFLAGS(weapon_type2, obj->value[4]));
		}
		break;

	case ITEM_ARMOR:
		buf_printf(output, BUF_END, "Armor class is %d pierce, "
				   "%d bash, %d slash, and %d vs. exotic.\n",
			   INT(obj->value[0]), INT(obj->value[1]),
			   INT(obj->value[2]), INT(obj->value[3]));
		break;
	}
}

/*
 * Count occurrences of an obj in a list.
 */
int
count_obj_list(OBJ_INDEX_DATA *pObjIndex, OBJ_DATA *list)
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

void
label_add(OBJ_DATA *obj, const char *label)
{
	const char *p = obj->label;
	obj->label = str_printf("%s %s", obj->label, label);
	free_string(p);
}

static inline void
money_form(size_t lang, char *buf, size_t len, int num, const char *name)
{
	char tmp[MAX_STRING_LENGTH];

	if (num < 0)
		return;

	strnzcpy(tmp, sizeof(tmp),
		 word_form(GETMSG(name, lang), 1, lang, RULES_CASE));
	strnzcpy(buf, len, word_form(tmp, (size_t) num, lang, RULES_QTY));
}

static MLSTR_FOREACH_FUN(money_descr_cb, lang, p, ap)
{
	int num1 = va_arg(ap, int);
	const char *name1 = va_arg(ap, const char *);
	int num2 = va_arg(ap, int);
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
OBJ_DATA *
create_money(int gold, int silver)
{
	OBJ_DATA *obj;

	if (gold < 0 || silver < 0 || (gold == 0 && silver == 0)) {
		log(LOG_INFO, "create_money: gold %d, silver %d",
			   gold, silver);
		gold = UMAX(1, gold);
		silver = UMAX(1, silver);
	}

	if (gold == 0 && silver == 1)
		obj = create_obj(OBJ_VNUM_SILVER_ONE, 0);
	else if (gold == 1 && silver == 0)
		obj = create_obj(OBJ_VNUM_GOLD_ONE, 0);
	else if (silver == 0) {
		if ((obj = create_obj(OBJ_VNUM_GOLD_SOME, 0)) != NULL) {
			mlstr_foreach(
			    &obj->short_descr, money_descr_cb,
			    gold, "gold coins", -1, NULL);
			INT(obj->value[1]) = gold;
			obj->cost	= 100*gold;
			obj->weight	= gold/5;
		}
	} else if (gold == 0) {
		if ((obj = create_obj(OBJ_VNUM_SILVER_ONE, 0)) != NULL) {
			mlstr_foreach(
			    &obj->short_descr, money_descr_cb,
			    silver, "silver coins", -1, NULL);
			INT(obj->value[0]) = silver;
			obj->cost	= silver;
			obj->weight	= silver/20;
		}
	} else {
		if ((obj = create_obj(OBJ_VNUM_COINS, 0)) != NULL) {
			mlstr_foreach(
			    &obj->short_descr, money_descr_cb,
			    silver, "silver coins", gold, "gold coins");
			INT(obj->value[0]) = silver;
			INT(obj->value[1]) = gold;
			obj->cost	= 100*gold + silver;
			obj->weight	= gold/5 + silver/20;
		}
	}

	return obj;
}

int
age_to_num(int age)
{
	return  age * 72000;
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
		act("$n steps out from $s cover.",
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

void
bad_effect(CHAR_DATA *ch, int effect)
{
	switch(effect) {
	case FAIL_NONE:
	default:
		act("Nothing happens.", ch, NULL, NULL, TO_CHAR);
		return;

	case FAIL_DEATH:
		raw_kill(ch, ch);
		return;

	case FAIL_HALLUCINATION:
		spellfun_call("hallucination", NULL, ch->level, ch, ch);
		return;
	}
}

int
trust_level(CHAR_DATA *ch)
{
	ch = GET_ORIGINAL(ch);
	return IS_NPC(ch) ? UMIN((ch)->level, LEVEL_HERO - 1) : ch->level;
}

/* object condition aliases */
const char *
get_cond_alias(OBJ_DATA *obj)
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

bool
mount_success(CHAR_DATA *ch, CHAR_DATA *mount, int canattack)
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

const char *stat_val_aliases[MAX_STAT][6] =
{
  { "Titanic", "Herculian", "Strong", "Average", "Poor", "Weak"	},  // notrans
  { "Genious", "Clever", "Good", "Average", "Poor", "Hopeless"	},  // notrans
  { "Excellent", "Wise", "Good", "Average", "Dim", "Fool"	},  // notrans
  { "Fast", "Quick", "Dextrous", "Average", "Clumsy", "Slow"	},  // notrans
  { "Iron", "Hearty", "Healthy", "Average", "Poor", "Fragile"	},  // notrans
  { "Charismatic", "Familier", "Good", "Average", "Poor", "Mongol"} // notrans
};

const char *
get_stat_alias(CHAR_DATA *ch, int stat)
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

void
set_title(CHAR_DATA *ch, const char *title)
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

void
look_at(CHAR_DATA *ch, ROOM_INDEX_DATA *room)
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
ROOM_INDEX_DATA *
get_random_room(CHAR_DATA *ch, AREA_DATA *area)
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

/*
 * deduct cost from a character
 */
void
deduct_cost(CHAR_DATA *ch, int cost)
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

int
count_charmed(CHAR_DATA *ch)
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

void
path_to_track(CHAR_DATA *ch, CHAR_DATA *victim, int door)
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
	opdoor = rev_dir[door];

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

/*
 * Room record:
 * For less than 5 people in room create a new record.
 * Else use the oldest one.
 */
void
room_record(const char *name, ROOM_INDEX_DATA *room, int door)
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
int
count_users(OBJ_DATA *obj)
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

/*
 * Find the ac value of an obj, including position effect.
 */
int
apply_ac(OBJ_DATA *obj, int iWear, int type)
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

/* find stuck-in objects of certain weapon type */
static OBJ_DATA *
get_stuck_eq(CHAR_DATA *ch, int wtype)
{
	OBJ_DATA *obj;

	if (!ch)
		return NULL;

	for (obj = ch->carrying; obj; obj = obj->next_content) {
		if (obj->wear_loc == WEAR_STUCK_IN
		&&  obj->pObjIndex->item_type == ITEM_WEAPON
		&&  INT(obj->value[0]) == wtype)
			return obj;
	}
	return NULL;
}

CHAR_DATA *
find_char(CHAR_DATA *ch, const char *argument, int door, int range)
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

	opdoor = rev_dir[door];
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

/*
 * Find a char for range casting.
 * argument must specify target in form '[d.][n.]name' where
 * 'd' - direction
 * 'n' - number
 */
CHAR_DATA *
get_char_spell(CHAR_DATA *ch, const char *argument, int *door, int range)
{
	char buf[MAX_INPUT_LENGTH];
	char *p;

	p = strchr(argument, '.');
	if (!p) {
		*door = -1;
		return get_char_here(ch, argument);
	}

	strnzncpy(buf, sizeof(buf), argument, (size_t) (p-argument));
	if ((*door = exit_lookup(buf)) < 0)
		return get_char_here(ch, argument);

	return find_char(ch, p+1, *door, range);
}

/*
 * just prints the list of available hometowns
 */
void
hometown_print_avail(CHAR_DATA *ch)
{
	size_t i;
	int col = 0;

	for (i = 0; i < hometowns.nused; i++) {
		hometown_t *h = VARR_GET(&hometowns, i);

		if (hometown_restrict(h, ch))
			continue;

		if (col > 60) {
			send_to_char("\n", ch);
			col = 0;
		}

		if (col)
			send_to_char(", ", ch);			// notrans
		act_puts("$t", ch, h->area, NULL,
			 TO_CHAR | ACT_NOLF | ACT_NOTRANS, POS_DEAD);
		col += strlen(h->area) + 2;
	}
}

void
reboot_mud(void)
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
			    "reboot_mud: unable to activate eqcheck");  // notrans
		} else {
			log(LOG_INFO, "reboot_mud: eqcheck activated"); // notrans
			fclose(fp);
		}
	}

	merc_down = TRUE;
}

/*****************************************************************************
 Name:		show_flags
 Purpose:	Displays settable flags and stats.
 ****************************************************************************/
void
show_flags(CHAR_DATA *ch, flaginfo_t *flag_table)
{
	BUFFER *output;

	output = buf_new(0);
	show_flags_buf(output, flag_table);
	page_to_char(buf_string(output), ch);
	buf_free(output);
}

char *
format_obj_to_char(OBJ_DATA *obj, CHAR_DATA *ch, int flags)
{
	static char buf[MAX_STRING_LENGTH];

	buf[0] = '\0';
	if ((IS_SET(flags, FO_F_SHORT) && mlstr_null(&obj->short_descr))
	||  mlstr_null(&obj->description))
		return str_empty;

	if (IS_SET(ch->comm, COMM_LONG)) {
		if (IS_OBJ_STAT(obj, ITEM_INVIS))
			strnzcat(buf, sizeof(buf),
				 GETMSG("({yInvis{x) ", GET_LANG(ch)));
		if (IS_OBJ_STAT(obj, ITEM_DARK))
			strnzcat(buf, sizeof(buf),
				 GETMSG("({DDark{x) ", GET_LANG(ch)));
		if (HAS_DETECT(ch, ID_EVIL)
		&&  IS_OBJ_STAT(obj, ITEM_EVIL))
			strnzcat(buf, sizeof(buf),
				 GETMSG("({RRed Aura{x) ", GET_LANG(ch)));
		if (HAS_DETECT(ch, ID_GOOD)
		&&  IS_OBJ_STAT(obj, ITEM_BLESS))
			strnzcat(buf, sizeof(buf),
				 GETMSG("({BBlue Aura{x) ", GET_LANG(ch)));
		if (HAS_DETECT(ch, ID_MAGIC)
		&&  IS_OBJ_STAT(obj, ITEM_MAGIC))
			strnzcat(buf, sizeof(buf),
				 GETMSG("({MMagical{x) ", GET_LANG(ch)));
		if (IS_OBJ_STAT(obj, ITEM_GLOW))
			strnzcat(buf, sizeof(buf),
				 GETMSG("({WGlowing{x) ", GET_LANG(ch)));
		if (IS_OBJ_STAT(obj, ITEM_HUM))
			strnzcat(buf, sizeof(buf),
				 GETMSG("({YHumming{x) ", GET_LANG(ch)));
	} else {
		static char FLAGS[] = "{x[{y.{D.{R.{B.{M.{W.{Y.{x] "; // notrans
		strnzcpy(buf, sizeof(buf), FLAGS);
		if (IS_OBJ_STAT(obj, ITEM_INVIS))
			buf[5] = 'I';
		if (IS_OBJ_STAT(obj, ITEM_DARK))
			buf[8] = 'D';
		if (HAS_DETECT(ch, ID_EVIL)
		&&  IS_OBJ_STAT(obj, ITEM_EVIL))
			buf[11] = 'E';
		if (HAS_DETECT(ch, ID_GOOD)
		&&  IS_OBJ_STAT(obj,ITEM_BLESS))
			buf[14] = 'B';
		if (HAS_DETECT(ch, ID_MAGIC)
		&&  IS_OBJ_STAT(obj, ITEM_MAGIC))
			buf[17] = 'M';
		if (IS_OBJ_STAT(obj, ITEM_GLOW))
			buf[20] = 'G';
		if (IS_OBJ_STAT(obj, ITEM_HUM))
			buf[23] = 'H';
		if (strcmp(buf, FLAGS) == 0)
			buf[0] = '\0';
	}

	if (IS_SET(flags, FO_F_SHORT)) {
		strnzcat(buf, sizeof(buf),
			 format_short(&obj->short_descr, obj->pObjIndex->name,
				      ch, GET_LANG(ch), 0));
		if (obj->pObjIndex->vnum > 5 /* not money, gold, etc */
		&&  (obj->condition < COND_EXCELLENT ||
		     !IS_SET(ch->comm, COMM_NOVERBOSE))) {
			char buf2[MAX_STRING_LENGTH];
			snprintf(buf2, sizeof(buf2), " [{g%s{x]",  // notrans
				 GETMSG(get_cond_alias(obj), GET_LANG(ch)));
			strnzcat(buf, sizeof(buf), buf2);
		}
		return buf;
	}

	if (obj->in_room && IS_WATER(obj->in_room)) {
		char *p;

		p = strchr(buf, '\0');
		strnzcat(buf, sizeof(buf),
			 format_short(&obj->short_descr, obj->pObjIndex->name,
				      ch, GET_LANG(ch), 0));
		cstrtoupper(p);
		switch(number_range(1, 3)) {
		case 1:
			strnzcat(buf, sizeof(buf),
				 " is floating gently on the water.");
			break;
		case 2:
			strnzcat(buf, sizeof(buf),
				 " is making it's way on the water.");
			break;
		case 3:
			strnzcat(buf, sizeof(buf),
				 " is getting wet by the water.");
			break;
		}
	} else {
		char tmp[MAX_STRING_LENGTH];
		actopt_t opt;

		opt.to_lang = GET_LANG(ch);
		opt.act_flags = ACT_NOUCASE | ACT_NOLF;

		act_buf(format_long(&obj->description, ch), ch, ch,
			NULL, NULL, NULL, &opt, tmp, sizeof(tmp));
		strnzcat(buf, sizeof(buf), tmp);
	}
	return buf;
}

/*
 * Show a list to a character.
 * Can coalesce duplicated items.
 */
void
show_list_to_char(OBJ_DATA *list, CHAR_DATA *ch, int flags)
{
	BUFFER *output;
	const char **prgpstrShow;
	int *prgnShow;
	char *pstrShow;
	OBJ_DATA *obj;
	int nShow;
	int iShow;
	int count;
	bool fCombine;

	if (ch->desc == NULL)
		return;

	/*
	 * Alloc space for output lines.
	 */
	output = buf_new(0);

	count = 0;
	for (obj = list; obj != NULL; obj = obj->next_content)
		count++;
	prgpstrShow = malloc(count * sizeof(char *));
	prgnShow    = malloc(count * sizeof(int)  );
	nShow	= 0;

	/*
	 * Format the list of objects.
	 */
	for (obj = list; obj != NULL; obj = obj->next_content) {
		if (obj->wear_loc == WEAR_NONE && can_see_obj(ch, obj)) {
			pstrShow = format_obj_to_char(obj, ch, flags);

			fCombine = FALSE;

			if (IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE)) {
				/*
				 * Look for duplicates, case sensitive.
				 * Matches tend to be near end so run loop
				 * backwords.
				 */
				for (iShow = nShow - 1; iShow >= 0; iShow--) {
					if (!strcmp(prgpstrShow[iShow],
						    pstrShow)) {
						prgnShow[iShow]++;
						fCombine = TRUE;
						break;
					}
				}
			}

			/*
			 * Couldn't combine, or didn't want to.
			 */
			if (!fCombine) {
				prgpstrShow [nShow] = str_dup(pstrShow);
				prgnShow    [nShow] = 1;
				nShow++;
			}
		}
	}

	/*
	 * Output the formatted list.
	 */
	for (iShow = 0; iShow < nShow; iShow++) {
		if (prgpstrShow[iShow][0] == '\0')
			continue;

		if (IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE)) {
			if (prgnShow[iShow] != 1) {
				buf_printf(output, BUF_END,
				    "(%2d) ",			// notrans
				    prgnShow[iShow]);
			} else
				buf_append(output,"     ");	// notrans
		}

		buf_append(output, prgpstrShow[iShow]);
		buf_append(output, "\n");
		free_string(prgpstrShow[iShow]);
	}

	if (IS_SET(flags, FO_F_SHOW_NOTHING) && nShow == 0) {
		if (IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE))
			send_to_char("     ", ch);		// notrans
		act_char("Nothing.", ch);
	}

	page_to_char(buf_string(output),ch);

	/*
	 * Clean up.
	 */
	buf_free(output);
	free(prgpstrShow);
	free(prgnShow);
}

static const char * wear_loc_names[] =
{
	"<used as light>     $t",
	"<worn on finger>    $t",
	"<worn on finger>    $t",
	"<worn around neck>  $t",
	"<worn on face>      $t",
	"<worn on torso>     $t",
	"<worn on head>      $t",
	"<worn on legs>      $t",
	"<worn on feet>      $t",
	"<worn on hands>     $t",
	"<worn on arms>      $t",
	"<worn as shield>    $t",
	"<worn about body>   $t",
	"<worn about waist>  $t",
	"<worn about wrist>  $t",
	"<worn about wrist>  $t",
	"<wielded>           $t",
	"<held>              $t",
	"<floating nearby>   $t",
	"<scratched tattoo>  $t",
	"<dual wielded>      $t",
	"<clan mark>         $t",
	"<stuck in>          $t",
};

void
show_obj_to_char(CHAR_DATA *ch, OBJ_DATA *obj, flag_t wear_loc)
{
	bool can_see_it;

	if (obj == NULL) {
		switch (wear_loc) {
		case WEAR_TATTOO:
		case WEAR_CLANMARK:
			return;
			/* NOTREACHED */

		case WEAR_SECOND_WIELD:
			if (get_skill(ch, "dual wield") == 0)
				return;
			/* FALLTHRU */

		case WEAR_SHIELD:
		case WEAR_HOLD:
		case WEAR_WIELD:
			if (!free_hands(ch))
				return;
			break;
		}
	}

	can_see_it = (obj == NULL) ? FALSE : can_see_obj(ch, obj);
	act(wear_loc_names[wear_loc], ch,
	    can_see_it ? format_obj_to_char(obj, ch, FO_F_SHORT) :
	    obj == NULL ? "nothing" : "something",
	    NULL, TO_CHAR | (can_see_it ? ACT_NOTRANS : 0));
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
	int gain = (con_app[get_max_train(ch, STAT_CON)].hitp + 2) *
		cl->hp_rate / 100;
	return UMAX(3, gain);
}

static int
min_hit_gain(CHAR_DATA *ch, class_t *cl)
{
	int gain = (con_app[get_curr_stat(ch, STAT_CON)].hitp - 3) *
		cl->hp_rate / 100;

	return UMAX(1, gain);
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

static void
drop_objs(CHAR_DATA *ch, OBJ_DATA *obj_list)
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

		if (obj->carried_by == NULL
		&&  obj->in_obj == NULL) {
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

		if (c_foreach(&clans, drop_objs_cb, obj) != NULL)
			continue;
		extract_obj(obj, 0);
	}
}

static bool
has_boat(CHAR_DATA *ch)
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

static bool
has_key(CHAR_DATA *ch, int key)
{
	OBJ_DATA *obj;

	for (obj = ch->carrying; obj; obj = obj->next_content) {
		if (obj->pObjIndex->vnum == key
		&&  can_see_obj(ch, obj))
		    return TRUE;
	}

	return FALSE;
}

static bool
has_key_ground(CHAR_DATA *ch, int key)
{
	OBJ_DATA *obj;

	for (obj = ch->in_room->contents; obj; obj = obj->next_content) {
		if (obj->pObjIndex->vnum == key
		&&  can_see_obj(ch, obj))
		    return TRUE;
	}

	return FALSE;
}

/*
 * Move a char out of a room.
 */
static void
char_from_room(CHAR_DATA *ch)
{
	OBJ_DATA *obj;
	CHAR_DATA *prev = NULL;
	CHAR_DATA *vch;

	if (ch->in_room == NULL) {
		log(LOG_BUG, "%s: NULL", __FUNCTION__);
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
		log(LOG_BUG, "%s: ch not found", __FUNCTION__);
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

static void
obj_from_xxx(OBJ_DATA *obj)
{
	if (obj->in_room)
		obj_from_room(obj);
	else if (obj->carried_by)
		obj_from_char(obj);
	else if (obj->in_obj)
		obj_from_obj(obj);
}

/*
 * Move an obj out of a room.
 */
static void
obj_from_room(OBJ_DATA *obj)
{
	ROOM_INDEX_DATA *in_room;
	CHAR_DATA *ch;

	if ((in_room = obj->in_room) == NULL) {
		log(LOG_INFO, "%s: NULL obj->in_room (vnum %d)",
		    __FUNCTION__, obj->pObjIndex->vnum);
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
			log(LOG_BUG, "%s: obj not found", __FUNCTION__);
		else
			prev->next_content = obj->next_content;
	}

	obj->in_room      = NULL;
	obj->next_content = NULL;
	return;
}

/*
 * Move an object out of an object.
 */
static void
obj_from_obj(OBJ_DATA *obj)
{
	OBJ_DATA *obj_from;

	if ((obj_from = obj->in_obj) == NULL) {
		log(LOG_BUG, "%s: null obj_from", __FUNCTION__);
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
			log(LOG_BUG, "%s: obj not found", __FUNCTION__);
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
