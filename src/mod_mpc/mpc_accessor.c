/*
 * Copyright (c) 2002 SoG Development Team
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
 * $Id: mpc_accessor.c,v 1.5 2004-02-19 17:16:47 fjoe Exp $
 */

#include <stdio.h>
#include <setjmp.h>

#include <merc.h>
#include <dynafun.h>

#include <sog.h>

#include "mpc_impl.h"

/*
 * generic getters
 */
#define GET_NAME(prefix, name)	prefix##_get_##name
#define _GET_BEGIN(name)						\
		static vo_t name(vo_t *vo)				\
		{							\
			vo_t v;
#define GET_BEGIN(prefix, name)						\
		_GET_BEGIN(GET_NAME(prefix, name))
#define GET_END								\
			return v;					\
		}
#define GET(prefix, name, lvalue, val)					\
		GET_BEGIN(prefix, name)					\
			lvalue = val;					\
		GET_END
#define GET_INT(prefix, name, val)	GET(prefix, name, v.i, val)
#define GET_STR(prefix, name, val)	GET(prefix, name, v.s, val)
#define GET_CHAR(prefix, name, val)	GET(prefix, name, v.ch, val)
#define GET_OBJ(prefix, name, val)	GET(prefix, name, v.obj, val)
#define GET_ROOM(prefix, name, val)	GET(prefix, name, v.r, val)

/*
 * generic setters
 */
#define SET_NAME(prefix, name)	prefix##_set_##name
#define _SET_BEGIN(name)						\
		static vo_t name(vo_t *vo, vo_t nv)			\
		{
#define SET_BEGIN(prefix, name)						\
		_SET_BEGIN(SET_NAME(prefix, name))
#define SET_END								\
			return nv;					\
		}
#define SET(prefix, name, lvalue, val)					\
		SET_BEGIN(prefix, name)					\
			lvalue = val;					\
		SET_END
#define SET_INT(prefix, name, lvalue)	SET(prefix, name, lvalue, nv.i)
#define SET_STR(prefix, name, lvalue)					\
		SET_BEGIN(prefix, name)					\
			free_string(lvalue);				\
			lvalue = str_qdup(nv.s);			\
		SET_END

/*
 * accessors definitions
 */
#define DEF_INT(prefix, name, type_tag)				\
		{ #name, type_tag, MT_INT, GET_NAME(prefix, name), NULL }
#define DEF_STR(prefix, name, type_tag)				\
		{ #name, type_tag, MT_STR, GET_NAME(prefix, name), NULL }
#define DEF_CHAR(prefix, name, type_tag)				\
		{ #name, type_tag, MT_CHAR, GET_NAME(prefix, name), NULL }
#define DEF_OBJ(prefix, name, type_tag)				\
		{ #name, type_tag, MT_OBJ, GET_NAME(prefix, name), NULL }
#define DEF_ROOM(prefix, name, type_tag)				\
		{ #name, type_tag, MT_ROOM, GET_NAME(prefix, name), NULL }

#define DEF_MUTABLE_INT(prefix, name, type_tag)				\
		{ #name, type_tag, MT_INT,				\
		  GET_NAME(prefix, name), SET_NAME(prefix, name) }
#define DEF_MUTABLE_STR(prefix, name, type_tag)				\
		{ #name, type_tag, MT_STR,				\
		  GET_NAME(prefix, name), SET_NAME(prefix, name) }
/*
 * CHAR_DATA accessors
 */
#define CHAR_GET_INT(name)	GET_INT(char, name, vo->ch->name)
#define CHAR_GET_STR(name)	GET_STR(char, name, vo->ch->name)
#define CHAR_GET_CHAR(name)	GET_CHAR(char, name, vo->ch->name)
#define CHAR_GET_OBJ(name)	GET_OBJ(char, name, vo->ch->name)
#define CHAR_GET_ROOM(name)	GET_ROOM(char, name, vo->ch->name)

#define DEF_CHAR_INT(name)	DEF_INT(char, name, MT_CHAR)
#define DEF_CHAR_STR(name)	DEF_STR(char, name, MT_STR)
#define DEF_CHAR_CHAR(name)	DEF_CHAR(char, name, MT_CHAR)
#define DEF_CHAR_OBJ(name)	DEF_OBJ(char, name, MT_CHAR)
#define DEF_CHAR_ROOM(name)	DEF_ROOM(char, name, MT_CHAR)

#define CHAR_SET_INT(name)	SET_INT(char, name, vo->ch->name)

#define DEF_CHAR_MUTABLE_INT(name)	DEF_MUTABLE_INT(char, name, MT_CHAR)

GET_INT(char, stat_str, get_curr_stat(vo->ch, STAT_STR))
GET_INT(char, stat_int, get_curr_stat(vo->ch, STAT_INT))
GET_INT(char, stat_wis, get_curr_stat(vo->ch, STAT_WIS))
GET_INT(char, stat_dex, get_curr_stat(vo->ch, STAT_DEX))
GET_INT(char, stat_con, get_curr_stat(vo->ch, STAT_CON))
GET_INT(char, stat_cha, get_curr_stat(vo->ch, STAT_CHA))

CHAR_GET_INT(hit)
SET_BEGIN(char, hit)
	vo->ch->hit = nv.i;
	update_pos(vo->ch);
	nv.i = vo->ch->hit;
SET_END
CHAR_GET_INT(max_hit)
CHAR_GET_INT(mana)
CHAR_SET_INT(mana)
CHAR_GET_INT(max_mana)
CHAR_GET_INT(move)
CHAR_SET_INT(move)
CHAR_GET_INT(max_move)

CHAR_GET_INT(gold)
CHAR_SET_INT(gold)
CHAR_GET_INT(silver)
CHAR_SET_INT(silver)

GET_INT(char, level, LEVEL(vo->ch))
GET_INT(char, real_level, vo->ch->level)

GET_STR(char, clan, IS_NULLSTR(vo->ch->clan) ? NULL : vo->ch->clan)
GET_INT(char, ethos, IS_NPC(vo->ch) ? ETHOS_NONE : PC(vo->ch)->ethos)
CHAR_GET_CHAR(fighting)
GET_STR(char, hometown,
	!IS_NPC(vo->ch) ? hometown_name(PC(vo->ch)->hometown) :
	NPC(vo->ch)->zone != NULL ?  NPC(vo->ch)->zone->name :
	    vo->ch->in_room->area->name)
CHAR_GET_ROOM(in_room)
GET_INT(char, luck, GET_LUCK(vo->ch))
GET_CHAR(char, pet, GET_PET(vo->ch))
CHAR_GET_INT(position)
GET_INT(char, quest_time, IS_NPC(vo->ch) ? -1 : PC(vo->ch)->questtime)
CHAR_GET_STR(race)
GET_INT(char, sex, flag_value(sex_table, mlstr_mval(&vo->ch->gender)))
CHAR_GET_INT(size)
GET_INT(char, vnum, IS_NPC(vo->ch) ? vo->ch->pMobIndex->vnum : 0)

mpc_accessor_t char_acstab[] =
{
	DEF_CHAR_INT(stat_str),
	DEF_CHAR_INT(stat_int),
	DEF_CHAR_INT(stat_wis),
	DEF_CHAR_INT(stat_dex),
	DEF_CHAR_INT(stat_con),
	DEF_CHAR_INT(stat_cha),

	DEF_CHAR_MUTABLE_INT(hit),
	DEF_CHAR_INT(max_hit),
	DEF_CHAR_MUTABLE_INT(mana),
	DEF_CHAR_INT(max_mana),
	DEF_CHAR_MUTABLE_INT(move),
	DEF_CHAR_INT(max_move),

	DEF_CHAR_MUTABLE_INT(gold),
	DEF_CHAR_MUTABLE_INT(silver),

	DEF_CHAR_INT(level),
	DEF_CHAR_INT(real_level),

	DEF_CHAR_STR(clan),
	DEF_CHAR_INT(ethos),
	DEF_CHAR_CHAR(fighting),
	DEF_CHAR_STR(hometown),
	DEF_CHAR_ROOM(in_room),
	DEF_CHAR_INT(luck),
	DEF_CHAR_CHAR(pet),
	DEF_CHAR_INT(position),
	DEF_CHAR_INT(quest_time),
	DEF_CHAR_STR(race),
	DEF_CHAR_INT(sex),
	DEF_CHAR_INT(size),
	DEF_CHAR_INT(vnum),
	{ NULL, 0, 0, NULL, NULL }
};
#define CHAR_ACSTAB_SZ	(sizeof(char_acstab) / sizeof(*char_acstab))

/*
 * OBJ_DATA accessors
 */
#define OBJ_GET_INT(name)	GET_INT(obj, name, vo->obj->name)
#define OBJ_GET_STR(name)	GET_STR(obj, name, vo->obj->name)
#define OBJ_GET_CHAR(name)	GET_CHAR(obj, name, vo->obj->name)
#define OBJ_GET_OBJ(name)	GET_OBJ(obj, name, vo->obj->name)
#define OBJ_GET_ROOM(name)	GET_ROOM(obj, name, vo->obj->name)

#define DEF_OBJ_INT(name)	DEF_INT(obj, name, MT_OBJ)
#define DEF_OBJ_STR(name)	DEF_STR(obj, name, MT_OBJ)
#define DEF_OBJ_CHAR(name)	DEF_CHAR(obj, name, MT_OBJ)
#define DEF_OBJ_OBJ(name)	DEF_OBJ(obj, name, MT_OBJ)
#define DEF_OBJ_ROOM(name)	DEF_ROOM(obj, name, MT_OBJ)

#define OBJ_SET_INT(name)	SET_INT(obj, name, vo->obj->name)

#define DEF_OBJ_MUTABLE_INT(name)	DEF_MUTABLE_INT(obj, name, MT_OBJ)

OBJ_GET_CHAR(carried_by)
OBJ_GET_OBJ(in_obj)
OBJ_GET_ROOM(in_room)

OBJ_GET_INT(cost)
GET_INT(obj, item_type, vo->obj->pObjIndex->item_type)
OBJ_GET_INT(level)
OBJ_SET_INT(level)
OBJ_GET_INT(timer)
OBJ_SET_INT(timer)
GET_INT(obj, v_dice_type,
	vo->obj->pObjIndex->item_type == ITEM_WEAPON ?
	    INT(vo->obj->value[2]) : 0)
SET_BEGIN(obj, v_dice_type)
	if (vo->obj->pObjIndex->item_type == ITEM_WEAPON)
		INT(vo->obj->value[2]) = nv.i;
	else
		nv.i = 0;
SET_END
OBJ_GET_INT(wear_loc)
GET_INT(obj, vnum, vo->obj->pObjIndex->vnum)

mpc_accessor_t obj_acstab[] =
{
	DEF_OBJ_CHAR(carried_by),
	DEF_OBJ_OBJ(in_obj),
	DEF_OBJ_ROOM(in_room),

	DEF_OBJ_INT(cost),
	DEF_OBJ_INT(item_type),
	DEF_OBJ_MUTABLE_INT(level),
	DEF_OBJ_MUTABLE_INT(timer),
	DEF_OBJ_INT(wear_loc),
	DEF_OBJ_MUTABLE_INT(v_dice_type),
	DEF_OBJ_INT(vnum),
	{ NULL, 0, 0, NULL, NULL }
};
#define OBJ_ACSTAB_SZ	(sizeof(obj_acstab) / sizeof(*obj_acstab))

/*
 * ROOM_DATA accessors
 */
#define ROOM_GET_INT(name)	GET_INT(room, name, vo->r->name)
#define ROOM_GET_STR(name)	GET_STR(room, name, vo->r->name)
#define ROOM_GET_CHAR(name)	GET_CHAR(room, name, vo->r->name)
#define ROOM_GET_OBJ(name)	GET_OBJ(room, name, vo->r->name)
#define ROOM_GET_ROOM(name)	GET_ROOM(room, name, vo->r->name)

#define DEF_ROOM_INT(name)	DEF_INT(room, name, MT_ROOM)
#define DEF_ROOM_STR(name)	DEF_STR(room, name, MT_ROOM)
#define DEF_ROOM_CHAR(name)	DEF_CHAR(room, name, MT_ROOM)
#define DEF_ROOM_OBJ(name)	DEF_OBJ(room, name, MT_ROOM)
#define DEF_ROOM_ROOM(name)	DEF_ROOM(room, name, MT_ROOM)

ROOM_GET_INT(sector_type)
ROOM_GET_INT(vnum)

mpc_accessor_t room_acstab[] =
{
	DEF_ROOM_INT(sector_type),
	DEF_ROOM_INT(vnum),
	{ NULL, 0, 0, NULL, NULL }
};
#define ROOM_ACSTAB_SZ	(sizeof(room_acstab) / sizeof(*room_acstab))

/*
 * AFFECT_DATA accessors
 */
#define AFFECT_GET_INT(name)	GET_INT(affect, name, vo->aff->name)
#define AFFECT_GET_STR(name)	GET_STR(affect, name, vo->aff->name)
#define AFFECT_GET_CHAR(name)	GET_CHAR(affect, name, vo->aff->name)
#define AFFECT_GET_OBJ(name)	GET_OBJ(affect, name, vo->aff->name)
#define AFFECT_GET_AFFECT(name)	GET_AFFECT(affect, name, vo->aff->name)

#define DEF_AFFECT_INT(name)	DEF_INT(affect, name, MT_AFFECT)
#define DEF_AFFECT_STR(name)	DEF_STR(affect, name, MT_AFFECT)
#define DEF_AFFECT_CHAR(name)	DEF_CHAR(affect, name, MT_AFFECT)
#define DEF_AFFECT_OBJ(name)	DEF_OBJ(affect, name, MT_AFFECT)
#define DEF_AFFECT_AFFECT(name)	DEF_AFFECT(affect, name, MT_AFFECT)

#define AFFECT_SET_INT(name)	SET_INT(affect, name, vo->aff->name)
#define AFFECT_SET_STR(name)	SET_STR(affect, name, vo->aff->name)

#define DEF_AFFECT_MUTABLE_INT(name)	DEF_MUTABLE_INT(affect, name, MT_AFFECT)
#define DEF_AFFECT_MUTABLE_STR(name)	DEF_MUTABLE_STR(affect, name, MT_AFFECT)

AFFECT_GET_INT(where)
AFFECT_GET_STR(type)
AFFECT_GET_INT(level)
AFFECT_SET_INT(level)
AFFECT_GET_INT(duration)
AFFECT_SET_INT(duration)
GET_BEGIN(affect, location)
	if (!HAS_STR_LOCATION(vo->aff))
		v.i = INT(vo->aff->location);
	else
		v.i = -1;
GET_END
SET_BEGIN(affect, location)
	if (HAS_STR_LOCATION(vo->aff))
		INT(vo->aff->location) = nv.i;
	else
		nv.i = -1;
SET_END
GET_BEGIN(affect, str_location)
	if (HAS_STR_LOCATION(vo->aff))
		v.s = vo->aff->location.s;
	else
		v.s = str_empty;
GET_END
SET_BEGIN(affect, str_location)
	if (HAS_STR_LOCATION(vo->aff)) {
		free_string(vo->aff->location.s);
		vo->aff->location.s = str_qdup(nv.s);
	} else
		nv.s = str_empty;
SET_END
AFFECT_GET_INT(modifier)
AFFECT_SET_INT(modifier)
AFFECT_GET_INT(bitvector)
AFFECT_SET_INT(bitvector)
AFFECT_GET_CHAR(owner)

mpc_accessor_t affect_acstab[] =
{
	DEF_AFFECT_INT(where),
	DEF_AFFECT_STR(type),
	DEF_AFFECT_MUTABLE_INT(level),
	DEF_AFFECT_MUTABLE_INT(duration),
	DEF_MUTABLE_INT(affect, location, MT_AFFECT),
	DEF_MUTABLE_STR(affect, str_location, MT_AFFECT),
	DEF_AFFECT_MUTABLE_INT(modifier),
	DEF_AFFECT_MUTABLE_INT(bitvector),
	DEF_AFFECT_CHAR(owner),
	{ NULL, 0, 0, NULL, NULL }
};
#define AFFECT_ACSTAB_SZ	(sizeof(affect_acstab) / sizeof(*affect_acstab))

/*
 * Lookup accessor by type and name
 */
mpc_accessor_t *
mpc_accessor_lookup(int type_tag, const char *name)
{
	static bool acstab_initialized;
	size_t sz;
	mpc_accessor_t *tab;

	if (!acstab_initialized) {
		qsort(char_acstab, CHAR_ACSTAB_SZ, sizeof(mpc_accessor_t),
		      cmpstr);
		qsort(obj_acstab, OBJ_ACSTAB_SZ, sizeof(mpc_accessor_t),
		      cmpstr);
		qsort(room_acstab, ROOM_ACSTAB_SZ, sizeof(mpc_accessor_t),
		      cmpstr);
		qsort(affect_acstab, AFFECT_ACSTAB_SZ, sizeof(mpc_accessor_t),
		      cmpstr);
		acstab_initialized = TRUE;
	}

	switch (type_tag) {
	case MT_CHAR:
		tab = char_acstab;
		sz = CHAR_ACSTAB_SZ;
		break;
	case MT_OBJ:
		tab = obj_acstab;
		sz = OBJ_ACSTAB_SZ;
		break;
	case MT_ROOM:
		tab = room_acstab;
		sz = ROOM_ACSTAB_SZ;
		break;
	case MT_AFFECT:
		tab = affect_acstab;
		sz = AFFECT_ACSTAB_SZ;
		break;
	default:
		return NULL;
	}

	return (mpc_accessor_t *) bsearch(
	    &name, tab, sz, sizeof(mpc_accessor_t), cmpstr);
}
