/*-
 * Copyright (c) 1998 fjoe <fjoe@iclub.nsu.ru>
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
 * $Id: olc_area.c,v 1.31 1999-02-10 15:58:51 fjoe Exp $
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "olc.h"
#include "db/db.h"

#define EDIT_AREA(ch, area)	(area = (AREA_DATA*) ch->desc->pEdit)

DECLARE_OLC_FUN(areaed_create		);
DECLARE_OLC_FUN(areaed_edit		);
DECLARE_OLC_FUN(areaed_touch		);
DECLARE_OLC_FUN(areaed_show		);
DECLARE_OLC_FUN(areaed_list		);

DECLARE_OLC_FUN(areaed_name		);
DECLARE_OLC_FUN(areaed_file		);
DECLARE_OLC_FUN(areaed_flags		);
DECLARE_OLC_FUN(areaed_age		);
DECLARE_OLC_FUN(areaed_reset		);
DECLARE_OLC_FUN(areaed_security		);
DECLARE_OLC_FUN(areaed_builders		);
DECLARE_OLC_FUN(areaed_resetmsg		);
DECLARE_OLC_FUN(areaed_minvnum		);
DECLARE_OLC_FUN(areaed_maxvnum		);
DECLARE_OLC_FUN(areaed_move		);
DECLARE_OLC_FUN(areaed_credits		);
DECLARE_OLC_FUN(areaed_minlevel		);
DECLARE_OLC_FUN(areaed_maxlevel		);
DECLARE_OLC_FUN(areaed_clan		);

DECLARE_VALIDATE_FUN(validate_security	);
DECLARE_VALIDATE_FUN(validate_minvnum	);
DECLARE_VALIDATE_FUN(validate_maxvnum	);
DECLARE_VALIDATE_FUN(validate_move	);

OLC_CMD_DATA olc_cmds_area[] =
{
/*	{   command	function	arg			}, */

	{ "create",	areaed_create				},
	{ "edit",	areaed_edit				},
	{ "touch",	areaed_touch				},
	{ "show",	areaed_show				},
	{ "list",	areaed_list				},

	{ "name",	areaed_name				},
	{ "filename",	areaed_file,	validate_filename	},
	{ "area",	areaed_flags,	area_flags		},
	{ "age",	areaed_age				},
	{ "reset",	areaed_reset				},
	{ "security",	areaed_security, validate_security	},
	{ "builders",	areaed_builders				},
	{ "resetmsg",	areaed_resetmsg				},
	{ "minvnum",	areaed_minvnum,	validate_minvnum	},
	{ "maxvnum",	areaed_maxvnum,	validate_maxvnum	},
	{ "move",	areaed_move,	validate_move		},
	{ "credits",	areaed_credits				},
	{ "minlevel",	areaed_minlevel				},
	{ "maxlevel",	areaed_maxlevel				},
	{ "clan",	areaed_clan				},

	{ "commands",	show_commands				},
	{ "version",	show_version				},

	{ NULL }
};

static AREA_DATA *check_range(AREA_DATA *pArea, int ilower, int iupper);

/*
 * Area Editor Functions.
 */
OLC_FUN(areaed_create)
{
	AREA_DATA *pArea;

	if (ch->pcdata->security < SECURITY_AREA_CREATE) {
		char_puts("AreaEd: Insufficient security.\n", ch);
		return FALSE;
	}

	pArea			= new_area();
	area_last->next		= pArea;
	area_last		= pArea;	/* Thanks, Walker. */

	ch->desc->pEdit		= (void*) pArea;
	ch->desc->editor	= ED_AREA;
	touch_area(pArea);
	char_puts("AreaEd: Area created.\n", ch);
	return FALSE;
}

OLC_FUN(areaed_edit)
{
	AREA_DATA *pArea;
	char arg[MAX_STRING_LENGTH];

	one_argument(argument, arg);
	if (arg[0] == '\0')
		pArea = ch->in_room->area;
	else if (!is_number(arg) || (pArea = area_lookup(atoi(arg))) == NULL) {
		char_puts("AreaEd: That area vnum does not exist.\n", ch);
		return FALSE;
	}

	if (!IS_BUILDER(ch, pArea)) {
		char_puts("AreaEd: Insufficient security.\n", ch);
		return FALSE;
	}

	ch->desc->pEdit		= (void *) pArea;
	ch->desc->editor	= ED_AREA;
	return FALSE;
}

OLC_FUN(areaed_touch)
{
	AREA_DATA *pArea;
	EDIT_AREA(ch, pArea);
	return touch_area(pArea);
}

OLC_FUN(areaed_show)
{
	AREA_DATA *pArea;
	char arg[MAX_STRING_LENGTH];

	one_argument(argument, arg);
	if (arg[0] == '\0') {
		if (ch->desc->editor == ED_AREA)
			EDIT_AREA(ch, pArea);
		else
			pArea = ch->in_room->area;
	}
	else if (!is_number(arg) || (pArea = area_lookup(atoi(arg))) == NULL) {
		char_puts("AreaEd: That area vnum does not exist.\n", ch);
		return FALSE;
	}

	char_printf(ch, "Name:     [%5d] %s\n", pArea->vnum, pArea->name);
	char_printf(ch, "File:     %s\n", pArea->file_name);
	char_printf(ch, "Vnums:    [%d-%d]\n",
		    pArea->min_vnum, pArea->max_vnum);
	char_printf(ch, "Levels:   [%d-%d]\n",
		    pArea->min_level, pArea->max_level);
	if (pArea->clan)
		char_printf(ch, "Clan:     [%s]\n", clan_name(pArea->clan));
	char_printf(ch, "Age:      [%d]\n",	pArea->age);
	char_printf(ch, "Players:  [%d]\n", pArea->nplayer);
	char_printf(ch, "Security: [%d]\n", pArea->security);
	if (!IS_NULLSTR(pArea->builders))
		char_printf(ch, "Builders: [%s]\n", pArea->builders);
	char_printf(ch, "Credits:  [%s]\n", pArea->credits);
	char_printf(ch, "Flags:    [%s]\n",
			flag_string(area_flags, pArea->flags));
	return FALSE;
}

OLC_FUN(areaed_list)
{
	char arg[MAX_STRING_LENGTH];
	AREA_DATA *pArea;
	BUFFER *output = NULL;

	one_argument(argument, arg);

	for (pArea = area_first; pArea; pArea = pArea->next) {
		if (arg[0] != '\0' && !strstr(strlwr(pArea->name), arg))
			continue;

		if (output == NULL) {
			output = buf_new(-1);
    			buf_printf(output, "[%3s] [%-27s] (%-5s-%5s) [%-10s] %3s [%-10s]\n",
				   "Num", "Area Name", "lvnum", "uvnum",
				   "Filename", "Sec", "Builders");
		}

		buf_printf(output, "[%3d] %-29.29s (%-5d-%5d) %-12.12s [%d] [%-10.10s]\n",
			   pArea->vnum, pArea->name,
			   pArea->min_vnum, pArea->max_vnum,
			   pArea->file_name, pArea->security, pArea->builders);
    	}

	if (output != NULL) {
		send_to_char(buf_string(output), ch);
		buf_free(output);
	}
	else
		char_puts("No areas with that name found.\n", ch);
	return FALSE;
}

OLC_FUN(areaed_reset)
{
	AREA_DATA *pArea;
	EDIT_AREA(ch, pArea);
	reset_area(pArea);
	char_puts("Area reset.\n", ch);
	return FALSE;
}

OLC_FUN(areaed_name)
{
	AREA_DATA *pArea;
	EDIT_AREA(ch, pArea);
	return olced_str(ch, argument, areaed_name, &pArea->name);
}

OLC_FUN(areaed_credits)
{
	AREA_DATA *pArea;
	EDIT_AREA(ch, pArea);
	return olced_str(ch, argument, areaed_credits, &pArea->credits);
}

OLC_FUN(areaed_file)
{
	AREA_DATA *pArea;
	EDIT_AREA(ch, pArea);
	return olced_str(ch, argument, areaed_file, &pArea->file_name);
}

OLC_FUN(areaed_age)
{
	AREA_DATA *pArea;
	EDIT_AREA(ch, pArea);
	return olced_number(ch, argument, areaed_age, &pArea->age);
}

OLC_FUN(areaed_flags)
{
	AREA_DATA *pArea;
	EDIT_AREA(ch, pArea);
	return olced_flag(ch, argument, areaed_flags, &pArea->flags);
}

OLC_FUN(areaed_security)
{
	AREA_DATA *pArea;
	EDIT_AREA(ch, pArea);
	return olced_number(ch, argument, areaed_security, &pArea->security);
}

OLC_FUN(areaed_minlevel)
{
	AREA_DATA *pArea;
	EDIT_AREA(ch, pArea);
	return olced_number(ch, argument, areaed_minlevel, &pArea->min_level);
}

OLC_FUN(areaed_maxlevel)
{
	AREA_DATA *pArea;
	EDIT_AREA(ch, pArea);
	return olced_number(ch, argument, areaed_maxlevel, &pArea->max_level);
}

OLC_FUN(areaed_resetmsg)
{
	AREA_DATA *pArea;
	EDIT_AREA(ch, pArea);
	return olced_mlstr(ch, argument, areaed_resetmsg, &pArea->resetmsg);
}

OLC_FUN(areaed_builders)
{
	AREA_DATA *pArea;
	char name[MAX_STRING_LENGTH];

	EDIT_AREA(ch, pArea);

	one_argument(argument, name);
	if (name[0] == '\0') {
		do_help(ch, "'OLC AREA BUILDER'");
		return FALSE;
	}
	name_toggle(&pArea->builders, name, ch, "AreaEd");
	return TRUE;
}

OLC_FUN(areaed_minvnum)
{
	AREA_DATA *pArea;
	EDIT_AREA(ch, pArea);
	return olced_number(ch, argument, areaed_minvnum, &pArea->min_vnum);
}

OLC_FUN(areaed_maxvnum)
{
	AREA_DATA *pArea;
	EDIT_AREA(ch, pArea);
	return olced_number(ch, argument, areaed_maxvnum, &pArea->max_vnum);
}

OLC_FUN(areaed_move)
{
	AREA_DATA *pArea;
	EDIT_AREA(ch, pArea);
	return olced_number(ch, argument, areaed_move, &pArea->min_vnum);
}

OLC_FUN(areaed_clan)
{
	AREA_DATA *pArea;
	EDIT_AREA(ch, pArea);
	return olced_clan(ch, argument, areaed_clan, &pArea->clan);
}

/* Validators */

VALIDATE_FUN(validate_security)
{
	int sec = *(int*) arg;
	if (sec > ch->pcdata->security || sec < 0) {
		if (ch->pcdata->security != 0)
			char_printf(ch, "AreaEd: Valid security range is 0..%d.\n", ch->pcdata->security);
		else
			char_puts("AreaEd: Valid security is 0 only.\n", ch);
		return FALSE;
	}
	return TRUE;
}

VALIDATE_FUN(validate_minvnum)
{
	int min_vnum = *(int*) arg;
	AREA_DATA *pArea;
	EDIT_AREA(ch, pArea);

	if (min_vnum && pArea->max_vnum) {
		if (min_vnum > pArea->max_vnum) {
			char_puts("AreaEd: Min vnum must be less than max vnum.\n", ch);
			return FALSE;
		}
	
		if (check_range(pArea, min_vnum, pArea->max_vnum)) {
			char_puts("AreaEd: Range must include only this area.\n", ch);
			return FALSE;
		}
	}
	return TRUE;
}

VALIDATE_FUN(validate_maxvnum)
{
	int max_vnum = *(int*) arg;
	AREA_DATA *pArea;
	EDIT_AREA(ch, pArea);

	if (pArea->min_vnum && max_vnum) {
		if (max_vnum < pArea->min_vnum) {
			char_puts("AreaEd: Max vnum must be greater than min vnum.\n", ch);
			return FALSE;
		}
	
		if (check_range(pArea, pArea->min_vnum, max_vnum)) {
			char_puts("AreaEd: Range must include only this area.\n", ch);
			return FALSE;
		}
	}
	return TRUE;
}

#define IN_RANGE(i, l, u) ((l) <= (i) && (i) <= (u))
#define MOVE(i) if (IN_RANGE(i, pArea->min_vnum, pArea->max_vnum)) {	\
			i += delta;					\
			touched = TRUE;					\
		}

static void move_mob(MOB_INDEX_DATA *mob, AREA_DATA *pArea, int delta);
static void move_obj(OBJ_INDEX_DATA *obj, AREA_DATA *pArea, int delta);
static void move_room(ROOM_INDEX_DATA *room, AREA_DATA *pArea, int delta);

VALIDATE_FUN(validate_move)
{
	int i;
	int new_min = *(int*) arg;
	int delta;
	bool touched;
	AREA_DATA *pArea;
	MPCODE *mpc;
	CLAN_DATA *clan;
	EDIT_AREA(ch, pArea);

	if (ch->pcdata->security < SECURITY_AREA_CREATE) {
		char_puts("AreaEd: Insufficient security.\n", ch);
		return FALSE;
	}

	if (!pArea->min_vnum || !pArea->max_vnum) {
		char_puts("AreaEd: Both min_vnum and max_vnum must be set "
			  "in order to perform area vnum move.\n", ch);
		return FALSE;
	}

/* check new region */
	delta = new_min - pArea->min_vnum;
	if (check_range(pArea, new_min, pArea->max_vnum+delta)) {
		char_puts("AreaEd: New vnum range overlaps other areas.\n", ch);
		return FALSE;
	}

/* everything is ok -- change vnums of all rooms, objs, mobs in area */

/* fix clan recall, item and altar vnums */
	touched = FALSE;
	for (i = 0; i < clans.nused; i++) {
		bool touched2 = touched;
		touched = FALSE;
		clan = CLAN(i);
		MOVE(clan->altar_vnum);
		MOVE(clan->recall_vnum);
		MOVE(clan->obj_vnum);
		if (touched)
			touch_clan(clan);
		else
			touched = touched2;
	}
	if (touched) {
		char_puts("AreaEd: Changed clans:\n", ch);
		for (i = 0; i < clans.nused; i++) {
			clan = CLAN(i);
			if (IS_SET(clan->flags, CLAN_CHANGED))
				char_printf(ch, "- %s\n", clan->name);
		}
	}

/* XXX fix mprogs */
	for (mpc = mpcode_list; mpc; mpc = mpc->next)
		MOVE(mpc->vnum);

/* fix mobs */
	for (i = 0; i < MAX_KEY_HASH; i++) {
		MOB_INDEX_DATA *mob;

		for (mob = mob_index_hash[i]; mob; mob = mob->next)
			move_mob(mob, pArea, delta);
	}

/* fix objs */
	for (i = 0; i < MAX_KEY_HASH; i++) {
		OBJ_INDEX_DATA *obj;

		for (obj = obj_index_hash[i]; obj; obj = obj->next)
			move_obj(obj, pArea, delta);
	}

/* fix rooms */
	for (i = 0; i < MAX_KEY_HASH; i++) {
		ROOM_INDEX_DATA *room;

		for (room = room_index_hash[i]; room; room = room->next)
			move_room(room, pArea, delta);
	}

/* rebuild mob index hash */
	top_vnum_mob = 0;
	for (i = 0; i < MAX_KEY_HASH; i++) {
		MOB_INDEX_DATA *mob, *mob_next, *mob_prev = NULL;

		for (mob = mob_index_hash[i]; mob; mob = mob_next) {
			int mob_hash = mob->vnum % MAX_KEY_HASH;
			mob_next = mob->next;

			if (top_vnum_mob < mob->vnum)
				top_vnum_mob = mob->vnum;

			if (mob_hash != i) {
				if (!mob_prev)
					mob_index_hash[i] = mob->next;
				else
					mob_prev->next = mob->next;
				mob->next = mob_index_hash[mob_hash];
				mob_index_hash[mob_hash] = mob;
			}
			else
				mob_prev = mob;
		}
	}

/* rebuild obj index hash */
	top_vnum_obj = 0;
	for (i = 0; i < MAX_KEY_HASH; i++) {
		OBJ_INDEX_DATA *obj, *obj_next, *obj_prev = NULL;

		for (obj = obj_index_hash[i]; obj; obj = obj_next) {
			int obj_hash = obj->vnum % MAX_KEY_HASH;
			obj_next = obj->next;

			if (top_vnum_obj < obj->vnum)
				top_vnum_obj = obj->vnum;

			if (obj_hash != i) {
				if (!obj_prev)
					obj_index_hash[i] = obj->next;
				else
					obj_prev->next = obj->next;
				obj->next = obj_index_hash[obj_hash];
				obj_index_hash[obj_hash] = obj;
			}
			else
				obj_prev = obj;
		}
	}

/* rebuild room index hash */
	top_vnum_room = 0;
	for (i = 0; i < MAX_KEY_HASH; i++) {
		ROOM_INDEX_DATA *room, *room_next, *room_prev = NULL;

		for (room = room_index_hash[i]; room; room = room_next) {
			int room_hash = room->vnum % MAX_KEY_HASH;
			room_next = room->next;

			if (top_vnum_room < room->vnum)
				top_vnum_room = room->vnum;

			if (room_hash != i) {
				if (!room_prev)
					room_index_hash[i] = room->next;
				else
					room_prev->next = room->next;
				room->next = room_index_hash[room_hash];
				room_index_hash[room_hash] = room;
			}
			else
				room_prev = room;
		}
	}

	pArea->min_vnum += delta;
	pArea->max_vnum += delta;
	touch_area(pArea);

	char_puts("AreaEd: Changed areas:\n", ch);
	for (pArea = area_first; pArea; pArea = pArea->next)
		if (IS_SET(pArea->flags, AREA_CHANGED))
			char_printf(ch, "[%3d] %s (%s)\n",
				    pArea->vnum, pArea->name, pArea->file_name);
	return TRUE;
}

/* Local functions */

/*****************************************************************************
 Name:		check_range(lower vnum, upper vnum)
 Purpose:	Ensures the range spans only one area.
 Called by:	areaed_vnum(olc_act.c).
 ****************************************************************************/
static AREA_DATA *check_range(AREA_DATA *this, int ilower, int iupper)
{
	AREA_DATA *pArea;

	for (pArea = area_first; pArea; pArea = pArea->next) {
		if (pArea == this || !pArea->min_vnum || !pArea->max_vnum)
			continue;
		if (IN_RANGE(ilower, pArea->min_vnum, pArea->max_vnum)
		||  IN_RANGE(iupper, pArea->min_vnum, pArea->max_vnum)
		||  IN_RANGE(pArea->min_vnum, ilower, iupper)
		||  IN_RANGE(pArea->max_vnum, ilower, iupper))
			return pArea;
	}
	return NULL;
}

static void move_mob(MOB_INDEX_DATA *mob, AREA_DATA *pArea, int delta)
{
	bool touched;
	MPTRIG *mp;

	MOVE(mob->vnum);

	if (mob->pShop) 
		MOVE(mob->pShop->keeper);

	for (mp = mob->mptrig_list; mp; mp = mp->next)
		MOVE(mp->vnum);
}

static void move_obj(OBJ_INDEX_DATA *obj, AREA_DATA *pArea, int delta)
{
	bool touched = FALSE;
	int old_vnum = obj->vnum;

/* fix containers */
	switch (obj->item_type) {
	case ITEM_CONTAINER:
		MOVE(obj->value[2]);
		if (touched) {
			OBJ_DATA *o;

			for (o = object_list; o; o = o->next)
				if (o->pIndexData == obj)
					o->value[2] += delta;
		}
	}

	MOVE(obj->vnum);

/* touch area if it is not area being moved */
	if (touched && !IN_RANGE(old_vnum, pArea->min_vnum, pArea->max_vnum))
		touch_vnum(old_vnum);
}

static void move_room(ROOM_INDEX_DATA *room, AREA_DATA *pArea, int delta)
{
	int i;
	bool touched = FALSE;
	int old_vnum = room->vnum;
	RESET_DATA *r;

	MOVE(room->vnum);

	for (i = 0; i < MAX_DIR; i++) {
		EXIT_DATA *pExit = room->exit[i];

		if (!pExit || !pExit->u1.to_room)
			continue;

		if (IN_RANGE(pExit->u1.to_room->vnum, pArea->min_vnum+delta,
			     pArea->max_vnum+delta))
			touched = TRUE;
	}

	for (r = room->reset_first; r; r = r->next) {
		switch (r->command) {
		case 'M':
		case 'O':
		case 'P':
			MOVE(r->arg1);
			MOVE(r->arg3);
			break;

		case 'G':
		case 'E':
		case 'D':
		case 'R':
			MOVE(r->arg1);
			break;
		}
	}

/* touch area if it is not area being moved */
	if (touched && !IN_RANGE(old_vnum, pArea->min_vnum, pArea->max_vnum))
		touch_vnum(old_vnum);
}

