/*-
 * Copyright (c) 1998 SoG Development Team
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
 * $Id: olc_area.c,v 1.74 1999-12-20 12:40:34 fjoe Exp $
 */

#include "olc.h"
#include "obj_prog.h"

#define EDIT_AREA(ch, area)	(area = (AREA_DATA*) ch->desc->pEdit)

DECLARE_OLC_FUN(areaed_create		);
DECLARE_OLC_FUN(areaed_edit		);
DECLARE_OLC_FUN(areaed_save		);
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

olc_cmd_t olc_cmds_area[] =
{
/*	{   command	function	validator		arg	}, */

	{ "create",	areaed_create					},
	{ "edit",	areaed_edit					},
	{ "",		areaed_save					},
	{ "touch",	areaed_touch					},
	{ "show",	areaed_show					},
	{ "list",	areaed_list					},

	{ "name",	areaed_name					},
	{ "filename",	areaed_file,	validate_filename		},
	{ "area",	areaed_flags,	NULL,		area_flags	},
	{ "age",	areaed_age					},
	{ "reset",	areaed_reset					},
	{ "security",	areaed_security,validate_security		},
	{ "builders",	areaed_builders					},
	{ "resetmsg",	areaed_resetmsg					},
	{ "minvnum",	areaed_minvnum,	validate_minvnum		},
	{ "maxvnum",	areaed_maxvnum,	validate_maxvnum		},
	{ "move",	areaed_move,	validate_move			},
	{ "credits",	areaed_credits					},
	{ "minlevel",	areaed_minlevel					},
	{ "maxlevel",	areaed_maxlevel					},
	{ "clan",	areaed_clan,	NULL,		&clans		},

	{ "commands",	show_commands					},
	{ "version",	show_version					},

	{ NULL }
};

static AREA_DATA *check_range(AREA_DATA *pArea, int ilower, int iupper);

static void save_area_list(CHAR_DATA *ch);
static void save_area(CHAR_DATA *ch, AREA_DATA *pArea);

/*
 * Area Editor Functions.
 */
OLC_FUN(areaed_create)
{
	AREA_DATA *pArea;

	if (PC(ch)->security < SECURITY_AREA_CREATE) {
		char_puts("AreaEd: Insufficient security.\n", ch);
		return FALSE;
	}

	pArea		= new_area();
	area_last->next	= pArea;
	area_last	= pArea;	/* Thanks, Walker. */

	ch->desc->pEdit	= (void*) pArea;
	OLCED(ch)	= olced_lookup(ED_AREA);
	TOUCH_AREA(pArea);
	char_puts("AreaEd: Area created.\n", ch);
	return FALSE;
}

OLC_FUN(areaed_edit)
{
	AREA_DATA *pArea;
	char arg[MAX_STRING_LENGTH];

	one_argument(argument, arg, sizeof(arg));
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

	ch->desc->pEdit	= (void *) pArea;
	OLCED(ch)	= olced_lookup(ED_AREA);
	return FALSE;
}

#define SAVE_F_ALL	(A)
#define SAVE_F_FOUND	(B)

OLC_FUN(areaed_save)
{
	char arg[MAX_INPUT_LENGTH];
	AREA_DATA *pArea;
	int save_flags = 0;

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] != '\0') {
		if (!str_cmp(arg, "all"))
			SET_BIT(save_flags, SAVE_F_ALL);
		else {
			if (ch)
				dofun("help", ch, "'OLC ASAVE'");
			return FALSE;
		}
	}
		
	olc_printf(ch, "Saved zones:");
	for (pArea = area_first; pArea; pArea = pArea->next) {
		/* Builder must be assigned this area. */
		if (ch && !IS_BUILDER(ch, pArea))
			continue;

		if (!IS_SET(save_flags, SAVE_F_ALL)
		&&  !IS_SET(pArea->area_flags, AREA_CHANGED))
			continue;

		SET_BIT(save_flags, SAVE_F_FOUND);
		save_area(ch, pArea);

		olc_printf(ch, "    %s (%s)", pArea->name, pArea->file_name);
		REMOVE_BIT(pArea->area_flags, AREA_CHANGED);
	}

	if (!IS_SET(save_flags, SAVE_F_FOUND))
		olc_printf(ch, "    None.");
	else
		save_area_list(ch);

	/*
	 * can't get rid of such cool message :)
	 *					/fjoe
	 */
	if (IS_SET(save_flags, SAVE_F_ALL)) {
		if (ch)
			char_puts("You saved the world.\n", ch);
		else
			wizlog("The world is saved.");
	}

	return FALSE;
}

OLC_FUN(areaed_touch)
{
	AREA_DATA *pArea;
	EDIT_AREA(ch, pArea);
	TOUCH_AREA(pArea);
	return FALSE;
}

OLC_FUN(areaed_show)
{
	AREA_DATA *pArea;
	char arg[MAX_STRING_LENGTH];

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		if (IS_EDIT(ch, ED_AREA))
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
		char_printf(ch, "Clan:     [%s]\n", pArea->clan);
	char_printf(ch, "Age:      [%d]\n",	pArea->age);
	char_printf(ch, "Players:  [%d]\n", pArea->nplayer);
	char_printf(ch, "Security: [%d]\n", pArea->security);
	if (!IS_NULLSTR(pArea->builders))
		char_printf(ch, "Builders: [%s]\n", pArea->builders);
	char_printf(ch, "Credits:  [%s]\n", pArea->credits);
	char_printf(ch, "Flags:    [%s]\n",
			flag_string(area_flags, pArea->area_flags));
	return FALSE;
}

OLC_FUN(areaed_list)
{
	char arg[MAX_STRING_LENGTH];
	AREA_DATA *pArea;
	BUFFER *output = NULL;

	one_argument(argument, arg, sizeof(arg));

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
		page_to_char(buf_string(output), ch);
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
	return olced_str(ch, argument, cmd, &pArea->name);
}

OLC_FUN(areaed_credits)
{
	AREA_DATA *pArea;
	EDIT_AREA(ch, pArea);
	return olced_str(ch, argument, cmd, &pArea->credits);
}

OLC_FUN(areaed_file)
{
	AREA_DATA *pArea;
	EDIT_AREA(ch, pArea);
	return olced_str(ch, argument, cmd, &pArea->file_name);
}

OLC_FUN(areaed_age)
{
	AREA_DATA *pArea;
	EDIT_AREA(ch, pArea);
	return olced_number(ch, argument, cmd, &pArea->age);
}

OLC_FUN(areaed_flags)
{
	AREA_DATA *pArea;
	EDIT_AREA(ch, pArea);
	return olced_flag(ch, argument, cmd, &pArea->area_flags);
}

OLC_FUN(areaed_security)
{
	AREA_DATA *pArea;
	EDIT_AREA(ch, pArea);
	return olced_number(ch, argument, cmd, &pArea->security);
}

OLC_FUN(areaed_minlevel)
{
	AREA_DATA *pArea;
	EDIT_AREA(ch, pArea);
	return olced_number(ch, argument, cmd, &pArea->min_level);
}

OLC_FUN(areaed_maxlevel)
{
	AREA_DATA *pArea;
	EDIT_AREA(ch, pArea);
	return olced_number(ch, argument, cmd, &pArea->max_level);
}

OLC_FUN(areaed_resetmsg)
{
	AREA_DATA *pArea;
	EDIT_AREA(ch, pArea);
	return olced_mlstr(ch, argument, cmd, &pArea->resetmsg);
}

OLC_FUN(areaed_builders)
{
	AREA_DATA *pArea;
	char name[MAX_STRING_LENGTH];
	const char *p;
	char filename[PATH_MAX];

	EDIT_AREA(ch, pArea);

	one_argument(argument, name, sizeof(name));
	if (name[0] == '\0') {
		char_printf(ch, "Syntax: builders <name list>\n");
		return FALSE;
	}

	p = capitalize(name);
	snprintf(filename, sizeof(filename), "%s.gz", p);
	if (!dfexist(PLAYER_PATH, filename)
	&&  !dfexist(PLAYER_PATH, p)) {
		char_printf(ch, "AreaEd: %s: no such player.\n", name);
		return FALSE;
	}

	name_toggle(&pArea->builders, name, ch, "AreaEd");
	return TRUE;
}

OLC_FUN(areaed_minvnum)
{
	AREA_DATA *pArea;
	EDIT_AREA(ch, pArea);
	return olced_number(ch, argument, cmd, &pArea->min_vnum);
}

OLC_FUN(areaed_maxvnum)
{
	AREA_DATA *pArea;
	EDIT_AREA(ch, pArea);
	return olced_number(ch, argument, cmd, &pArea->max_vnum);
}

OLC_FUN(areaed_move)
{
	AREA_DATA *pArea;
	EDIT_AREA(ch, pArea);
	return olced_number(ch, argument, cmd, &pArea->min_vnum);
}

OLC_FUN(areaed_clan)
{
	AREA_DATA *pArea;
	EDIT_AREA(ch, pArea);
	return olced_foreign_strkey(ch, argument, cmd, &pArea->clan);
}

/* Validators */

VALIDATE_FUN(validate_security)
{
	int sec = *(int*) arg;
	if (sec > PC(ch)->security || sec < 0) {
		if (PC(ch)->security != 0)
			char_printf(ch, "AreaEd: Valid security range is 0..%d.\n", PC(ch)->security);
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

static void *
move_clan_cb(void *p, va_list ap)
{
	clan_t *clan = (clan_t *) p;

	AREA_DATA *pArea = va_arg(ap, AREA_DATA *);
	int delta = va_arg(ap, int);
	bool *ptouched = va_arg(ap, bool *);

	bool touched = FALSE;

	MOVE(clan->altar_vnum);
	MOVE(clan->recall_vnum);
	MOVE(clan->obj_vnum);
	MOVE(clan->mark_vnum);

	if (touched) {
		touch_clan(clan);
		*ptouched = TRUE;
	}
	return NULL;
}

static void *
move_print_clan_cb(void *p, va_list ap)
{
	clan_t *clan = (clan_t *) p;

	CHAR_DATA *ch = va_arg(ap, CHAR_DATA *);

	if (IS_SET(clan->clan_flags, CLAN_CHANGED))
		char_printf(ch, "- %s\n", clan->name);
	return NULL;
}

VALIDATE_FUN(validate_move)
{
	int i;
	int new_min = *(int*) arg;
	int delta, oldmin, oldmax;
	bool touched = FALSE;
	AREA_DATA *pArea;
	MPCODE *mpc;
	EDIT_AREA(ch, pArea);

	if (PC(ch)->security < SECURITY_AREA_CREATE) {
		char_puts("AreaEd: Insufficient security.\n", ch);
		return FALSE;
	}

	if (!pArea->min_vnum || !pArea->max_vnum) {
		char_puts("AreaEd: Both min_vnum and max_vnum must be set "
			  "in order to perform area vnum move.\n", ch);
		return FALSE;
	}

	oldmin = pArea->min_vnum;
	oldmax = pArea->max_vnum;

/* check new region */
	delta = new_min - pArea->min_vnum;
	if (check_range(pArea, new_min, pArea->max_vnum+delta)) {
		char_puts("AreaEd: New vnum range overlaps other areas.\n", ch);
		return FALSE;
	}

/* everything is ok -- change vnums of all rooms, objs, mobs in area */

/* fix clan recall, item and altar vnums */
	hash_foreach(&clans, move_clan_cb, pArea, delta, &touched);
	if (touched) {
		char_puts("AreaEd: Changed clans:\n", ch);
		hash_foreach(&clans, move_print_clan_cb, ch);
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

	pArea->max_vnum += delta;
	TOUCH_AREA(pArea);

	char_puts("AreaEd: Changed areas:\n", ch);
	for (pArea = area_first; pArea; pArea = pArea->next)
		if (IS_SET(pArea->area_flags, AREA_CHANGED))
			char_printf(ch, "[%3d] %s (%s)\n",
				    pArea->vnum, pArea->name, pArea->file_name);

	move_pfiles(oldmin, oldmax, delta);

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
	bool touched = FALSE;
	MPTRIG *mp;
	int old_vnum = mob->vnum;

	MOVE(mob->vnum);

	if (mob->pShop) 
		MOVE(mob->pShop->keeper);

	for (mp = mob->mptrig_list; mp; mp = mp->next)
		MOVE(mp->vnum);

	MOVE(mob->fvnum);

/* touch area if it is not area being moved */
	if (touched && !IN_RANGE(old_vnum, pArea->min_vnum, pArea->max_vnum))
		TOUCH_VNUM(old_vnum);
}

static void move_obj(OBJ_INDEX_DATA *obj, AREA_DATA *pArea, int delta)
{
	OBJ_DATA *o;
	bool touched = FALSE;
	int old_vnum = obj->vnum;

	switch (obj->item_type) {
	case ITEM_CONTAINER:
		MOVE(INT(obj->value[2])); /* container key */
		if (touched) {
			for (o = object_list; o; o = o->next)
				if (o->pObjIndex == obj)
					INT(o->value[2]) += delta;
		}
		break;
	case ITEM_PORTAL:
		MOVE(INT(obj->value[3])); /* portal exit */
		MOVE(INT(obj->value[4])); /* portal key */
		if (touched) {
			for (o = object_list; o; o = o->next)
				if (o->pObjIndex == obj) {
					INT(o->value[3]) += delta;
					INT(o->value[4]) += delta;
				}
		}
		break;
	}

	MOVE(obj->vnum);

/* touch area if it is not area being moved */
	if (touched && !IN_RANGE(old_vnum, pArea->min_vnum, pArea->max_vnum))
		TOUCH_VNUM(old_vnum);
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

		if (!pExit || !pExit->to_room.r)
			continue;

		if (IN_RANGE(pExit->to_room.r->vnum, pArea->min_vnum+delta,
			     pArea->max_vnum+delta))
			touched = TRUE;

		if (IN_RANGE(pExit->to_room.r->vnum, pArea->min_vnum,
			     pArea->max_vnum))
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
		TOUCH_VNUM(old_vnum);
}

/*
 *  Verbose writes reset data in plain english into the comments
 *  section of the resets.  It makes areas considerably larger but
 *  may aid in debugging.
 */
#define VERBOSE

/*****************************************************************************
 Name:		save_area_list
 Purpose:	Saves the listing of files to be loaded at startup.
 ****************************************************************************/
static void save_area_list(CHAR_DATA *ch)
{
	FILE *fp;
	AREA_DATA *pArea;

	if ((fp = olc_fopen(AREA_PATH, AREA_LIST, ch, -1)) == NULL)
		return;

	for (pArea = area_first; pArea; pArea = pArea->next)
		fprintf(fp, "%s\n", pArea->file_name);

	fprintf(fp, "$\n");
	fclose(fp);
}

static void save_mobprogs(FILE *fp, AREA_DATA *pArea)
{
	MPCODE *mpcode;
        int i;
	bool found = FALSE;

	for (i = pArea->min_vnum; i <= pArea->max_vnum; i++) {
        	if ((mpcode = mpcode_lookup(i)) != NULL) {
			if (!found) {
        			fprintf(fp, "#MOBPROGS\n");
				found = TRUE;
			}
			fprintf(fp, "#%d\n", i);
			fwrite_string(fp, NULL, mpcode->code);
		}
        }

	if (found)
        	fprintf(fp,"#0\n\n");
}

/*****************************************************************************
 Name:		save_mobile
 Purpose:	Save one mobile to file, new format -- Hugin
 Called by:	save_mobiles (below).
 ****************************************************************************/
static void save_mobile(FILE *fp, MOB_INDEX_DATA *pMobIndex)
{
	race_t *r = race_lookup(pMobIndex->race);
	MPTRIG *mptrig;
	flag_t temp;
	const char *p;
	int i;

	if (r == NULL) {
		wizlog("save_mobile: vnum %d: %s: unknown race",
		       pMobIndex->vnum, pMobIndex->race);
		return;
	}

	fprintf(fp, "#%d\n",	pMobIndex->vnum);
	fwrite_string(fp, NULL,	pMobIndex->name);
	mlstr_fwrite(fp, NULL,	&pMobIndex->short_descr);
	mlstr_fwrite(fp, NULL,	&pMobIndex->long_descr);
	mlstr_fwrite(fp, NULL,	&pMobIndex->description);
	fwrite_string(fp, NULL,	r->name);
	fprintf(fp, "%s ",	format_flags(pMobIndex->act & ~r->act));
	fprintf(fp, "%s ",	format_flags(pMobIndex->mob_flags));
	fprintf(fp, "%s ",	format_flags(pMobIndex->affected_by & ~r->aff));
	fprintf(fp, "%s ",	format_flags(pMobIndex->has_invis & ~r->has_invis));
	fprintf(fp, "%s ",	format_flags(pMobIndex->has_detect & ~r->has_detect));
	fprintf(fp, "%d %d\n",	pMobIndex->alignment , pMobIndex->group);
	fprintf(fp, "%d ",	pMobIndex->level);
	fprintf(fp, "%d ",	pMobIndex->hitroll);
	fprintf(fp, "%dd%d+%d ",pMobIndex->hit[DICE_NUMBER], 
				pMobIndex->hit[DICE_TYPE], 
				pMobIndex->hit[DICE_BONUS]);
	fprintf(fp, "%dd%d+%d ",pMobIndex->mana[DICE_NUMBER], 
				pMobIndex->mana[DICE_TYPE], 
				pMobIndex->mana[DICE_BONUS]);
	fprintf(fp, "%dd%d+%d ",pMobIndex->damage[DICE_NUMBER], 
				pMobIndex->damage[DICE_TYPE], 
				pMobIndex->damage[DICE_BONUS]);
	fprintf(fp, "'%s'\n",	pMobIndex->damtype);
	fprintf(fp, "%d %d %d %d\n",
				pMobIndex->ac[AC_PIERCE] / 10, 
				pMobIndex->ac[AC_BASH]   / 10, 
				pMobIndex->ac[AC_SLASH]  / 10, 
				pMobIndex->ac[AC_EXOTIC] / 10);
	fprintf(fp, "%s\n",	format_flags(pMobIndex->off_flags & ~r->off));
	
	p = mlstr_mval(&pMobIndex->gender);
	if (IS_NULLSTR(p))
		p = flag_string(gender_table, SEX_NEUTRAL);
	fprintf(fp, "%s %s %s %d\n",
			flag_string(position_table, pMobIndex->start_pos),
			flag_string(position_table, pMobIndex->default_pos),
			p,
			pMobIndex->wealth);
	fprintf(fp, "%s ",	format_flags(pMobIndex->form & ~r->form));
	fprintf(fp, "%s ",	format_flags(pMobIndex->parts & ~r->parts));

	fprintf(fp, "%s ",	flag_string(size_table, pMobIndex->size));
	fprintf(fp, "%s\n",	IS_NULLSTR(pMobIndex->material) ? pMobIndex->material : "unknown");

	if (mlstr_nlang(&pMobIndex->gender) > 1)
		mlstr_fwrite(fp, "g", &pMobIndex->gender);

	/* save diffs */
	if ((temp = DIFF_BIT(r->act, pMobIndex->act)))
		fprintf(fp, "F act %s\n", format_flags(temp));

	if ((temp = DIFF_BIT(r->aff, pMobIndex->affected_by)))
		fprintf(fp, "F aff %s\n", format_flags(temp));

	if ((temp = DIFF_BIT(r->has_invis, pMobIndex->has_invis)))
		fprintf(fp, "F inv %s\n", format_flags(temp));

	if ((temp = DIFF_BIT(r->has_detect, pMobIndex->has_detect)))
		fprintf(fp, "F det %s\n", format_flags(temp));

	if ((temp = DIFF_BIT(r->off, pMobIndex->off_flags)))
		fprintf(fp, "F off %s\n", format_flags(temp));

	if ((temp = DIFF_BIT(r->form, pMobIndex->form)))
		fprintf(fp, "F for %s\n", format_flags(temp));

	if ((temp = DIFF_BIT(r->parts, pMobIndex->parts)))
		fprintf(fp, "F par %s\n", format_flags(temp));

	for (mptrig = pMobIndex->mptrig_list; mptrig; mptrig = mptrig->next) {
		fprintf(fp, "M %s %d %s~\n",
			flag_string(mptrig_types, mptrig->type), mptrig->vnum,
			fix_string(mptrig->phrase));
	}

	fwrite_word(fp, "C", pMobIndex->clan);
	if (pMobIndex->invis_level)
		fprintf(fp, "W %d\n", pMobIndex->invis_level);
	if (pMobIndex->incog_level)
		fprintf(fp, "I %d\n", pMobIndex->incog_level);
	if (pMobIndex->fvnum)
		fprintf(fp, "V %d\n", pMobIndex->fvnum);
	for (i = 0; i < MAX_RESIST; i++) {
		if (pMobIndex->resists[i] != r->resists[i]) {
			fprintf(fp, "r %s %d\n", 
				flag_string(resist_flags, i), 
				pMobIndex->resists[i]);
		}
	}
	aff_fwrite_list("a", pMobIndex->affected, fp);
}

/*****************************************************************************
 Name:		save_mobiles
 Purpose:	Save #MOBILES secion of an area file.
 Called by:	save_area(olc_save.c).
 Notes:         Changed for ROM OLC.
 ****************************************************************************/
static void save_mobiles(FILE *fp, AREA_DATA *pArea)
{
	int i;
	MOB_INDEX_DATA *pMob;
	bool found = FALSE;

	for (i = pArea->min_vnum; i <= pArea->max_vnum; i++)
		if ((pMob = get_mob_index(i))) {
			if (!found) {
				fprintf(fp, "#MOBILES\n");
				found = TRUE;
			}
			save_mobile(fp, pMob);
		}

	if (found)
		fprintf(fp, "#0\n\n");
}

/*****************************************************************************
 Name:		save_object
 Purpose:	Save one object to file.
                new ROM format saving -- Hugin
 Called by:	save_objects (below).
 ****************************************************************************/
static void save_object(FILE *fp, OBJ_INDEX_DATA *pObjIndex)
{
	char letter;
	ED_DATA *pEd;

	fprintf(fp, "#%d\n",	pObjIndex->vnum);
	fwrite_string(fp, NULL,	pObjIndex->name);
	mlstr_fwrite(fp, NULL,	&pObjIndex->short_descr);
	mlstr_fwrite(fp, NULL,	&pObjIndex->description);
	fwrite_string(fp, NULL,	pObjIndex->material);
	fprintf(fp, "%s ",	flag_string(item_types, pObjIndex->item_type));
	fprintf(fp, "%s ",	format_flags(pObjIndex->stat_flags));
	fprintf(fp, "%s ",	format_flags(pObjIndex->obj_flags));
	fprintf(fp, "%s\n",	format_flags(pObjIndex->wear_flags));
	fwrite_objval(pObjIndex->item_type, pObjIndex->value, fp);

	     if (pObjIndex->condition > 90) letter = 'P';
	else if (pObjIndex->condition > 75) letter = 'G';
	else if (pObjIndex->condition > 50) letter = 'A';
	else if (pObjIndex->condition > 25) letter = 'W';
	else if (pObjIndex->condition > 10) letter = 'D';
	else if (pObjIndex->condition >  0) letter = 'B';
	else				    letter = 'R';

	fprintf(fp, "%d %d %d %c\n",
		pObjIndex->level, pObjIndex->weight, pObjIndex->cost, letter);

	aff_fwrite_list("a", pObjIndex->affected, fp);

	for (pEd = pObjIndex->ed; pEd; pEd = pEd->next)
		ed_fwrite(fp, pEd);

	mlstr_fwrite(fp, "g", &pObjIndex->gender);
	fwrite_cc_vexpr(&pObjIndex->restrictions, "R", fp);
}

/*****************************************************************************
 Name:		save_objects
 Purpose:	Save #OBJECTS section of an area file.
 Called by:	save_area(olc_save.c).
 Notes:         Changed for ROM OLC.
 ****************************************************************************/
static void save_objects(FILE *fp, AREA_DATA *pArea)
{
	int i;
	OBJ_INDEX_DATA *pObj;
	bool found = FALSE;

	for (i = pArea->min_vnum; i <= pArea->max_vnum; i++)
		if ((pObj = get_obj_index(i))) {
			if (!found) {
    				fprintf(fp, "#OBJECTS\n");
				found = TRUE;
			}
			save_object(fp, pObj);
    		}

	if (found)
		fprintf(fp, "#0\n\n");
}

static int exitcmp(const void *p1, const void *p2)
{
	return (*(EXIT_DATA**)p1)->orig_door - (*(EXIT_DATA**)p2)->orig_door;
}

static void save_room(FILE *fp, ROOM_INDEX_DATA *pRoomIndex)
{
	int door;
	ED_DATA *pEd;
	EXIT_DATA *pExit;
	EXIT_DATA *exit[MAX_DIR];
	int max_door;

        fprintf(fp, "#%d\n",	pRoomIndex->vnum);
	mlstr_fwrite(fp, NULL,	&pRoomIndex->name);
	mlstr_fwrite(fp, NULL,	&pRoomIndex->description);
	fprintf(fp, "0 ");
        fprintf(fp, "%s ",	format_flags(pRoomIndex->room_flags));
        fprintf(fp, "%s\n",	flag_string(sector_types,
					    pRoomIndex->sector_type));

        for (pEd = pRoomIndex->ed; pEd; pEd = pEd->next)
		ed_fwrite(fp, pEd);

	/* sort exits (to minimize diffs) */
	for (max_door = 0, door = 0; door < MAX_DIR; door++)
		if ((pExit = pRoomIndex->exit[door]))
			exit[max_door++] = pExit;
	qsort(exit, max_door, sizeof(*exit), exitcmp);

	for (door = 0; door < max_door; door++) {
		pExit = exit[door];
		if (pExit->to_room.r) {
 
	 		/* HACK : TO PREVENT EX_LOCKED etc without EX_ISDOOR
 			   to stop booting the mud */
 			if (IS_SET(pExit->rs_flags, EX_CLOSED)
 			||  IS_SET(pExit->rs_flags, EX_LOCKED)
 			||  IS_SET(pExit->rs_flags, EX_PICKPROOF)
 			||  IS_SET(pExit->rs_flags, EX_NOPASS)
 			||  IS_SET(pExit->rs_flags, EX_EASY)
 			||  IS_SET(pExit->rs_flags, EX_HARD)
 			||  IS_SET(pExit->rs_flags, EX_INFURIATING)
 			||  IS_SET(pExit->rs_flags, EX_NOCLOSE)
 			||  IS_SET(pExit->rs_flags, EX_NOLOCK) )
 				SET_BIT(pExit->rs_flags, EX_ISDOOR);
 			else
 				REMOVE_BIT(pExit->rs_flags, EX_ISDOOR);
 
			if (pExit->size == SIZE_GARGANTUAN)
				fprintf(fp, "D%d\n",      pExit->orig_door);
			else
				fprintf(fp, "D%d %s\n", 
					pExit->orig_door,
					flag_string(size_table, pExit->size));
			mlstr_fwrite(fp, NULL,	  &pExit->description);
			fprintf(fp, "%s~\n",      pExit->keyword);
			mlstr_fwrite(fp, NULL, &pExit->short_descr.ml);
			mlstr_fwrite(fp, NULL, &pExit->short_descr.gender);
			fprintf(fp, "%s %d %d\n",
				format_flags(pExit->rs_flags | EX_BITVAL),
				pExit->key,
				pExit->to_room.r->vnum);
		}
	}

	if (pRoomIndex->mana_rate != 100 || pRoomIndex->heal_rate != 100)
		fprintf (fp, "M %d H %d\n", pRoomIndex->mana_rate,
					    pRoomIndex->heal_rate);
		 			     
	fprintf(fp, "S\n");
}

/*****************************************************************************
 Name:		save_rooms
 Purpose:	Save #ROOMS section of an area file.
 Called by:	save_area(olc_save.c).
 ****************************************************************************/
static void save_rooms(FILE *fp, AREA_DATA *pArea)
{
	ROOM_INDEX_DATA *pRoomIndex;
	bool found = FALSE;
	int i;

	for (i = pArea->min_vnum; i <= pArea->max_vnum; i++)
		if ((pRoomIndex = get_room_index(i))) {
			if (!found) {
				fprintf(fp, "#ROOMS\n");
				found = TRUE;
			}
			save_room(fp, pRoomIndex);
		}

	if (found)
		fprintf(fp, "#0\n\n");
}

static void save_special(FILE *fp, MOB_INDEX_DATA *pMobIndex)
{
#if defined(VERBOSE)
	fprintf(fp, "M %d %s\t* %s\n",
		pMobIndex->vnum,
		mob_spec_name(pMobIndex->spec_fun),
		mlstr_mval(&pMobIndex->short_descr));
#else
	fprintf(fp, "M %d %s\n",
		pMobIndex->vnum, mob_spec_name(pMobIndex->spec_fun));
#endif
}

/*****************************************************************************
 Name:		save_specials
 Purpose:	Save #SPECIALS section of area file.
 Called by:	save_area(olc_save.c).
 ****************************************************************************/
static void save_specials(FILE *fp, AREA_DATA *pArea)
{
	int i;
	MOB_INDEX_DATA *pMobIndex;
	bool found = FALSE;
    
	for (i = pArea->min_vnum; i <= pArea->max_vnum; i++)
		if ((pMobIndex = get_mob_index(i))
		&&  pMobIndex->spec_fun) {
			if (!found) {
				fprintf(fp, "#SPECIALS\n");
				found = TRUE;
			}
			save_special(fp, pMobIndex);
		}

	if (found)
		fprintf(fp, "S\n\n");
}

/*
 * This function is obsolete.  It it not needed but has been left here
 * for historical reasons.  It is used currently for the same reason.
 *
 * I don't think it's obsolete in ROM -- Hugin.
 */
static void save_door_reset(FILE *fp,
			    ROOM_INDEX_DATA *pRoomIndex, EXIT_DATA *pExit)
{
#if defined(VERBOSE)
	fprintf(fp,
		"D 0 %d %d %d\t* %s: door to the %s: %s\n", 
		pRoomIndex->vnum,
		pExit->orig_door,
		IS_SET(pExit->rs_flags, EX_LOCKED) ? 2 : 1,
		mlstr_mval(&pRoomIndex->name),
		dir_name[pExit->orig_door],
		IS_SET(pExit->rs_flags, EX_LOCKED) ?
			"closed and locked" : "closed");
#else
	fprintf(fp, "D 0 %d %d %d\n", 
		pRoomIndex->vnum,
		pExit->orig_door,
		IS_SET(pExit->rs_flags, EX_LOCKED) ? 2 : 1);
#endif
}

static void save_reset(FILE *fp, AREA_DATA *pArea,
		       ROOM_INDEX_DATA *pRoomIndex, RESET_DATA *pReset)
{
	switch (pReset->command) {
	default:
		bug("Save_resets: bad command %c.", pReset->command);
		break;

#if defined(VERBOSE)
	case 'M':
		fprintf(fp, "M 0 %d %d %d %d\t* %s (%s)\n", 
			pReset->arg1,
			pReset->arg2,
			pReset->arg3,
			pReset->arg4,
			mlstr_mval(&get_mob_index(pReset->arg1)->short_descr),
			mlstr_mval(&get_room_index(pReset->arg3)->name));
		break;

	case 'O':
		fprintf(fp, "O 0 %d 0 %d\t* %s (%s)\n", 
			pReset->arg1,
			pReset->arg3,
			mlstr_mval(&get_obj_index(pReset->arg1)->short_descr),
			mlstr_mval(&get_room_index(pReset->arg3)->name));
		break;

	case 'P':
		fprintf(fp, "P 0 %d %d %d %d\t* %s: %s\n", 
			pReset->arg1,
			pReset->arg2,
			pReset->arg3,
			pReset->arg4,
			mlstr_mval(&get_obj_index(pReset->arg3)->short_descr),
			mlstr_mval(&get_obj_index(pReset->arg1)->short_descr));
		break;

	case 'G':
		fprintf(fp, "G 0 %d 0\t\t*\t%s\n",
			pReset->arg1,
			mlstr_mval(&get_obj_index(pReset->arg1)->short_descr));
		break;

	case 'E':
		fprintf(fp, "E 0 %d 0 %d\t\t*\t%s: %s\n",
			pReset->arg1,
			pReset->arg3,
			mlstr_mval(&get_obj_index(pReset->arg1)->short_descr),
			flag_string(wear_loc_strings, pReset->arg3));
		break;

	case 'D':
		break;

	case 'R':
		pRoomIndex = get_room_index(pReset->arg1);
		fprintf(fp, "R 0 %d %d\t* %s: randomize\n", 
			pReset->arg1,
			pReset->arg2,
			mlstr_mval(&pRoomIndex->name));
		break;
#else
	case 'M':
		fprintf(fp, "M 0 %d %d %d %d\n", 
			pReset->arg1,
			pReset->arg2,
			pReset->arg3,
			pReset->arg4);
		break;

	case 'O':
		fprintf(fp, "O 0 %d 0 %d\n", 
			pReset->arg1,
			pReset->arg3);
		break;

	case 'P':
		fprintf(fp, "P 0 %d %d %d %d\n", 
			pReset->arg1,
			pReset->arg2,
			pReset->arg3,
			pReset->arg4);
		break;

	case 'G':
		fprintf(fp, "G 0 %d 0\n", pReset->arg1);
		break;

	case 'E':
		fprintf(fp, "E 0 %d 0 %d\n",
			pReset->arg1,
			pReset->arg3);
		break;

	case 'D':
		break;

	case 'R':
		fprintf(fp, "R 0 %d %d\n", 
			pReset->arg1,
			pReset->arg2);
		break;
#endif
	}
}

/*****************************************************************************
 Name:		save_resets
 Purpose:	Saves the #RESETS section of an area file.
 Called by:	save_area(olc_save.c)
 ****************************************************************************/
static void save_resets(FILE *fp, AREA_DATA *pArea)
{
	ROOM_INDEX_DATA *pRoomIndex;
	RESET_DATA *pReset;
	EXIT_DATA *pExit;
	int door;
	bool found = FALSE;
	int i;

	for (i = pArea->min_vnum; i <= pArea->max_vnum; i++)
		if ((pRoomIndex = get_room_index(i)))
			for (door = 0; door < MAX_DIR; door++)
				if ((pExit = pRoomIndex->exit[door])
				&&  pExit->to_room.r 
				&&  (IS_SET(pExit->rs_flags, EX_CLOSED) ||
				     IS_SET(pExit->rs_flags, EX_LOCKED))) {
					if (!found) {
						fprintf(fp, "#RESETS\n");
						found = TRUE;
					}
    					save_door_reset(fp, pRoomIndex, pExit);
				}

	for (i = pArea->min_vnum; i <= pArea->max_vnum; i++)
		if ((pRoomIndex = get_room_index(i)))
    			for (pReset = pRoomIndex->reset_first; pReset; pReset = pReset->next) {
				if (!found) {
					fprintf(fp, "#RESETS\n");
					found = TRUE;
				}
				save_reset(fp, pArea, pRoomIndex, pReset);
			}

	if (found)
		fprintf(fp, "S\n\n");
}

static void save_shop(FILE *fp, MOB_INDEX_DATA *pMobIndex)
{
	SHOP_DATA *pShopIndex;
	int iTrade;

	pShopIndex = pMobIndex->pShop;

	fprintf(fp, "%d ", pShopIndex->keeper);
	for (iTrade = 0; iTrade < MAX_TRADE; iTrade++)
		fprintf(fp, "%d ", pShopIndex->buy_type[iTrade]);
	fprintf(fp, "%d %d ", pShopIndex->profit_buy, pShopIndex->profit_sell);
	fprintf(fp, "%d %d\n", pShopIndex->open_hour, pShopIndex->close_hour);
}

/*****************************************************************************
 Name:		save_shops
 Purpose:	Saves the #SHOPS section of an area file.
 Called by:	save_area(olc_save.c)
 ****************************************************************************/
static void save_shops(FILE *fp, AREA_DATA *pArea)
{
	MOB_INDEX_DATA *pMobIndex;
	int i;
	bool found = FALSE;
    
	for (i = pArea->min_vnum; i <= pArea->max_vnum; i++)
		if ((pMobIndex = get_mob_index(i))
		&&  pMobIndex->pShop) {
			if (!found) {
				fprintf(fp, "#SHOPS\n");
				found = TRUE;
			}
			save_shop(fp, pMobIndex);
		}

	if (found)
		fprintf(fp, "0\n\n");
}

static void save_olimits(FILE *fp, AREA_DATA *pArea)
{
	int i;
	OBJ_INDEX_DATA *pObj;
	bool found = FALSE;

	for (i = pArea->min_vnum; i <= pArea->max_vnum; i++)
		if ((pObj = get_obj_index(i)) != NULL
		&&  pObj->limit != -1) {
			if (!found) {
				fprintf(fp, "#OLIMITS\n");
				found = TRUE;
			}
			fprintf(fp, "O %d %d\t* %s\n",
				i, pObj->limit, mlstr_mval(&pObj->short_descr));
		}

	if (found)
		fprintf(fp, "S\n\n");
}

static void save_omprog(FILE *fp, OBJ_INDEX_DATA *pObjIndex)
{
	int i;

	for (i = 0; i < OPROG_MAX; i++)
		if (pObjIndex->oprogs[i] != NULL)
			fprintf(fp, "O %d %s %s\t* `%s'\n",
				pObjIndex->vnum,
				optype_table[i],
				oprog_name_lookup(pObjIndex->oprogs[i]),
				mlstr_mval(&pObjIndex->short_descr));
}

static void save_omprogs(FILE *fp, AREA_DATA *pArea)
{
	int i;
	OBJ_INDEX_DATA *pObjIndex;
	bool found = FALSE;

	for (i = pArea->min_vnum; i <= pArea->max_vnum; i++)
		if ((pObjIndex = get_obj_index(i)) != NULL
		&&  pObjIndex->oprogs) {
			if (!found) {
				fprintf(fp, "#OMPROGS\n");
				found = TRUE;
			}
			save_omprog(fp, pObjIndex);
		}

	if (found)
		fprintf(fp, "S\n\n");
}

static void save_practicers(FILE *fp, AREA_DATA *pArea)
{
	int i;
	MOB_INDEX_DATA *pMobIndex;
	bool found = FALSE;

	for (i = pArea->min_vnum; i <= pArea->max_vnum; i++)
		if ((pMobIndex = get_mob_index(i)) != NULL
		&&  pMobIndex->practicer != 0) {
			if (!found) {
				fprintf(fp, "#PRACTICERS\n");
				found = TRUE;
			}
    			fprintf(fp, "M %d %s~\t* %s\n",
				pMobIndex->vnum,
				flag_string(skill_groups, pMobIndex->practicer),
				mlstr_mval(&pMobIndex->short_descr));
		}

	if (found)
		fprintf(fp, "S\n\n");
}

static void save_helps(FILE *fp, AREA_DATA *pArea)
{
	HELP_DATA *pHelp = pArea->help_first;

	if (pHelp == NULL)
		return;
		
	fprintf(fp, "#HELPS\n");

	for (; pHelp; pHelp = pHelp->next_in_area) {
		fprintf(fp, "%s %s~\n",
			fix_word(flag_istring(level_table, pHelp->level)),
			fix_string(pHelp->keyword));
		mlstr_fwrite(fp, NULL, &pHelp->text);
	}

	fprintf(fp, "-1 $~\n\n");
}

/*****************************************************************************
 Name:		save_area
 Purpose:	Save an area, note that this format is new.
 ****************************************************************************/
static void save_area(CHAR_DATA *ch, AREA_DATA *pArea)
{
	FILE *fp;
	int flags;

	if ((fp = olc_fopen(AREA_PATH, pArea->file_name, ch, -1)) == NULL)
		return;

	fprintf(fp, "#AREADATA\n");
	fprintf(fp, "Ver %d\n", AREA_VERSION);
	fprintf(fp, "Name %s~\n",	pArea->name);
	fwrite_string(fp, "Builders", pArea->builders);
	fprintf(fp, "VNUMs %d %d\n",	pArea->min_vnum, pArea->max_vnum);
	fwrite_string(fp, "Credits", pArea->credits);
	fprintf(fp, "Security %d\n",	pArea->security);
	fprintf(fp, "LevelRange %d %d\n",
		pArea->min_level, pArea->max_level);
	mlstr_fwrite(fp, "ResetMessage", &pArea->resetmsg);
	flags = pArea->area_flags & ~AREA_CHANGED;
	if (flags)
		fwrite_string(fp, "Flags", flag_string(area_flags, flags));
	fwrite_word(fp, "Clan", pArea->clan);
	fprintf(fp, "End\n\n");

	if (pArea->min_vnum && pArea->max_vnum) {
		save_mobiles(fp, pArea);
		save_objects(fp, pArea);
		save_rooms(fp, pArea);
		save_specials(fp, pArea);
		save_resets(fp, pArea);
		save_shops(fp, pArea);
		save_olimits(fp, pArea);
		save_mobprogs(fp, pArea);
		save_practicers(fp, pArea);
		save_omprogs(fp, pArea);
	}
	save_helps(fp, pArea);

	fprintf(fp, "#$\n");

	fclose(fp);
}
