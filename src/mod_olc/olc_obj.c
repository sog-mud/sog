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
 * $Id: olc_obj.c,v 1.20 1998-10-26 08:40:09 fjoe Exp $
 */

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "interp.h"
#include "olc.h"
#include "db/db.h"

#define EDIT_OBJ(ch, obj)	(obj = (OBJ_INDEX_DATA*) ch->desc->pEdit)

DECLARE_OLC_FUN(objed_create		);
DECLARE_OLC_FUN(objed_edit		);
DECLARE_OLC_FUN(objed_touch		);
DECLARE_OLC_FUN(objed_show		);
DECLARE_OLC_FUN(objed_list		);

DECLARE_OLC_FUN(objed_name		);
DECLARE_OLC_FUN(objed_short		);
DECLARE_OLC_FUN(objed_long		);
DECLARE_OLC_FUN(objed_addaffect		);
DECLARE_OLC_FUN(objed_addapply		);
DECLARE_OLC_FUN(objed_delaffect		);
DECLARE_OLC_FUN(objed_value0		);
DECLARE_OLC_FUN(objed_value1		);
DECLARE_OLC_FUN(objed_value2		);
DECLARE_OLC_FUN(objed_value3		);
DECLARE_OLC_FUN(objed_value4		);
DECLARE_OLC_FUN(objed_weight		);
DECLARE_OLC_FUN(objed_limit		);
DECLARE_OLC_FUN(objed_cost		);
DECLARE_OLC_FUN(objed_exd		);

DECLARE_OLC_FUN(objed_extra		);
DECLARE_OLC_FUN(objed_wear		);
DECLARE_OLC_FUN(objed_type		);
DECLARE_OLC_FUN(objed_affect		);
DECLARE_OLC_FUN(objed_material		);
DECLARE_OLC_FUN(objed_level		);
DECLARE_OLC_FUN(objed_condition		);
DECLARE_OLC_FUN(objed_clan		);
DECLARE_OLC_FUN(objed_clone		);

DECLARE_VALIDATE_FUN(validate_condition);

OLC_CMD_DATA olc_cmds_obj[] =
{
/*	{ command	function		arg			}, */

	{ "create",	objed_create					},
	{ "edit",	objed_edit					},
	{ "touch",	objed_touch					},
	{ "show",	objed_show					},
	{ "list",	objed_list					},

	{ "addaffect",	objed_addaffect					},
	{ "addapply",	objed_addapply					},
	{ "cost",	objed_cost					},
	{ "delaffect",	objed_delaffect					},
	{ "exd",	objed_exd					},
	{ "long",	objed_long					},
	{ "name",	objed_name					},
	{ "short",	objed_short					},
	{ "v0",		objed_value0					},
	{ "v1",		objed_value1					},
	{ "v2",		objed_value2					},
	{ "v3",		objed_value3					},
	{ "v4",		objed_value4					},
	{ "weight",	objed_weight					},
	{ "limit",	objed_limit					},

	{ "extra",	objed_extra,		extra_flags		},
	{ "wear",	objed_wear,		wear_flags		},
	{ "type",	objed_type,		item_types		},
	{ "material",	objed_material 					},
	{ "level",	objed_level					},
	{ "condition",	objed_condition,	validate_condition	},
	{ "clan",	objed_clan					},
	{ "clone",	objed_clone					},

	{ "version",	show_version					},
	{ "commands",	show_commands					},

	{ NULL }
};

static void show_obj_values(BUFFER *output, OBJ_INDEX_DATA *obj);
static bool set_obj_values(CHAR_DATA *ch, OBJ_INDEX_DATA *pObj,
			   int value_num, const char *argument);
static void show_skill_cmds(CHAR_DATA *ch, int tar);

OLC_FUN(objed_create)
{
	OBJ_INDEX_DATA *pObj;
	AREA_DATA *pArea;
	int  value;
	int  iHash;
	char arg[MAX_STRING_LENGTH];

	one_argument(argument, arg);
	value = atoi(arg);
	if (!value) {
		do_help(ch, "'OLC CREATE'");
		return FALSE;
	}

	pArea = area_vnum_lookup(value);
	if (!pArea) {
		char_puts("OEdit: That vnum is not assigned an area.\n\r", ch);
		return FALSE;
	}

	if (!IS_BUILDER(ch, pArea)) {
		char_puts("OEdit: Insufficient security.\n\r", ch);
		return FALSE;
	}

	if (get_obj_index(value)) {
		char_puts("OEdit: Object vnum already exists.\n\r", ch);
		return FALSE;
	}
		 
	pObj			= new_obj_index();
	pObj->vnum		= value;
		 
	if (value > top_vnum_obj)
		top_vnum_obj = value;

	iHash			= value % MAX_KEY_HASH;
	pObj->next		= obj_index_hash[iHash];
	obj_index_hash[iHash]	= pObj;

	ch->desc->pEdit		= (void *)pObj;
	ch->desc->editor	= ED_OBJ;
	touch_area(pArea);
	char_puts("OEdit: Object created.\n\r", ch);
	return FALSE;
}

OLC_FUN(objed_edit)
{
	char arg[MAX_INPUT_LENGTH];
	int value;
	OBJ_INDEX_DATA *pObj;
	AREA_DATA *pArea;

	one_argument(argument, arg);
	if (arg[0] == '\0') {
		do_help(ch, "'OLC EDIT'");
		return FALSE;
	}

	value = atoi(arg);
	pObj = get_obj_index(value);
	if (!pObj) {
		char_puts("OEdit: Vnum does not exist.\n\r", ch);
		return FALSE;
	}

	pArea = area_vnum_lookup(pObj->vnum);
	if (!IS_BUILDER(ch, pArea)) {
		char_puts("OEdit: Insufficient security.\n\r", ch);
	       	return FALSE;
	}

	ch->desc->pEdit = (void*) pObj;
	ch->desc->editor = ED_OBJ;
	return FALSE;
}

OLC_FUN(objed_touch)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);
	return touch_vnum(pObj->vnum);
}

OLC_FUN(objed_show)
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_INDEX_DATA	*pObj;
	AREA_DATA	*pArea;
	AFFECT_DATA *paf;
	int cnt;
	BUFFER *output;
	CLAN_DATA *clan;

	one_argument(argument, arg);
	if (arg[0] == '\0') {
		if (ch->desc->editor == ED_OBJ)
			EDIT_OBJ(ch, pObj);
		else {
			do_help(ch, "'OLC EDIT'");
			return FALSE;
		}
	}
	else {
		int value = atoi(arg);
		pObj = get_obj_index(value);
		if (!pObj) {
			char_puts("OEdit: Vnum does not exist.\n\r", ch);
			return FALSE;
		}
	}

	pArea = area_vnum_lookup(pObj->vnum);

	output = buf_new(-1);
	buf_printf(output, "Name:        [%s]\n\rArea:        [%5d] %s\n\r",
		pObj->name, pArea->vnum, pArea->name);

	buf_printf(output, "Vnum:        [%5d]\n\rType:        [%s]\n\r",
		pObj->vnum,
		flag_string(item_types, pObj->item_type));

	if (pObj->clan && (clan = clan_lookup(pObj->clan))) 
		buf_printf(output, "Clan      : [%s]\n\r", clan->name);

	if (pObj->limit != -1)
		buf_printf(output, "Limit:       [%5d]\n\r", pObj->limit);
	else
		buf_add(output, "Limit:       [none]\n\r");

	buf_printf(output, "Level:       [%5d]\n\r", pObj->level);

	buf_printf(output, "Wear flags:  [%s]\n\r",
		flag_string(wear_flags, pObj->wear_flags));

	buf_printf(output, "Extra flags: [%s]\n\r",
		flag_string(extra_flags, pObj->extra_flags));

	buf_printf(output, "Material:    [%s]\n\r",                /* ROM */
		pObj->material);

	buf_printf(output, "Condition:   [%5d]\n\r",               /* ROM */
		pObj->condition);

	buf_printf(output, "Weight:      [%5d]\n\rCost:        [%5d]\n\r",
		pObj->weight, pObj->cost);

	if (pObj->ed) {
		ED_DATA *ed;

		buf_add(output, "Ex desc kwd: ");

		for (ed = pObj->ed; ed; ed = ed->next)
			buf_printf(output, "[%s]", ed->keyword);

		buf_add(output, "\n\r");
	}

	mlstr_dump(output, "Short desc: ", pObj->short_descr);
	mlstr_dump(output, "Long desc: ", pObj->description);

	for (cnt = 0, paf = pObj->affected; paf; paf = paf->next) {
		WHERE_DATA *w = where_lookup(paf->where);

		if (cnt == 0) {
			buf_add(output, "Number      Affects Modifier Affects Bitvector\n\r");
			buf_add(output, "------ ------------ -------- ------- -----------------------------------------\n\r");
		}
		buf_printf(output, "[%4d] %12.12s %8d %7.7s %s"
				   "\n\r",
			   cnt,
			   flag_string(apply_flags, paf->location),
			   paf->modifier,
			   flag_string(apply_types, paf->where),
			   w ? flag_string(w->table, paf->bitvector) : "none");
		cnt++;
	}

	show_obj_values(output, pObj);
	page_to_char(buf_string(output), ch);
	buf_free(output);

	return FALSE;
}

OLC_FUN(objed_list)
{
	OBJ_INDEX_DATA	*pObjIndex;
	AREA_DATA	*pArea;
	BUFFER		*buffer;
	char		arg  [MAX_INPUT_LENGTH];
	bool fAll, found;
	int vnum;
	int  col = 0;

	one_argument(argument, arg);
	if (arg[0] == '\0') {
		do_help(ch, "'OLC ALIST'");
		return FALSE;
	}

	if ((pArea = get_edited_area(ch)) == NULL)
		pArea = ch->in_room->area;

	buffer  = buf_new(-1);
	fAll    = !str_cmp(arg, "all");
	found   = FALSE;

	for (vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++) {
		if ((pObjIndex = get_obj_index(vnum))) {
			if (fAll || is_name(arg, pObjIndex->name)
			|| flag_value(item_types, arg) == pObjIndex->item_type) {
				found = TRUE;
				buf_printf(buffer, "[%5d] %-17.16s",
					   pObjIndex->vnum,
					   mlstr_mval(pObjIndex->short_descr));
				if (++col % 3 == 0)
					buf_add(buffer, "\n\r");
			}
		}
	}

	if (!found)
		char_puts("Object(s) not found in this area.\n\r", ch);
	else {
		if (col % 3 != 0)
			buf_add(buffer, "\n\r");

		page_to_char(buf_string(buffer), ch);
	}

	buf_free(buffer);
	return FALSE;
}

/*
 * Need to issue warning if flag isn't valid. -- does so now -- Hugin.
 */
OLC_FUN(objed_addaffect)
{
	int location;
	int modifier;
	sflag_t where;
	flag_t bitvector;
	OBJ_INDEX_DATA *pObj;
	AFFECT_DATA *pAf;
	char loc[MAX_STRING_LENGTH];
	char mod[MAX_STRING_LENGTH];
	char wh[MAX_STRING_LENGTH];

	EDIT_OBJ(ch, pObj);

	argument = one_argument(argument, loc);
	argument = one_argument(argument, mod);
	argument = one_argument(argument, wh);

	if (loc[0] == '\0') {
		do_help(ch, "'OLC ADDAFFECT'");
		return FALSE;
	}

	if (!str_cmp(loc, "none")) {
		location = APPLY_NONE;
		modifier = 0;
	}
	else {
		if ((location = flag_value(apply_flags, loc)) < 0) {
			char_puts("Valid locations are:\n\r", ch);
			show_flags(ch, apply_flags);
			return FALSE;
		}

		if (!is_number(mod)) {
			do_help(ch, "'OLC ADDAFFECT'");
			return FALSE;
		}
		modifier = atoi(mod);
	}

	if (wh[0] == '\0') {
		where = -1;
		bitvector = 0;
	}
	else {
		WHERE_DATA *w;

		if ((where = flag_value(apply_types, wh)) < 0) {
			char_puts("Valid bitaffect locations are:\n\r", ch);
			show_flags(ch, apply_types);
			return FALSE;
		}

		if ((w = where_lookup(where)) == NULL) {
			char_printf(ch, "%s: not in where_table.\n\r",
				    flag_string(apply_types, where));
			return FALSE;
		}

		if ((bitvector = flag_value(w->table, argument)) == 0) {
			char_printf(ch, "Valid '%s' bitaffect flags are:\n\r",
				    flag_string(apply_types, where));
			show_flags(ch, w->table);
			return FALSE;
		}
	}

	pAf             = aff_new();
	pAf->location   = location;
	pAf->modifier   = modifier;
	pAf->where	= where;
	pAf->type       = -1;
	pAf->duration   = -1;
	pAf->bitvector  = bitvector;
	pAf->level      = pObj->level;
	pAf->next       = pObj->affected;
	pObj->affected  = pAf;

	char_puts("Affect added.\n\r", ch);
	return TRUE;
}

OLC_FUN(objed_addapply)
{
	int location, bv, where;
	OBJ_INDEX_DATA *pObj;
	AFFECT_DATA *pAf;
	WHERE_DATA *wd;
	char loc[MAX_STRING_LENGTH];
	char mod[MAX_STRING_LENGTH];
	char type[MAX_STRING_LENGTH];
	char bvector[MAX_STRING_LENGTH];

	EDIT_OBJ(ch, pObj);

	argument = one_argument(argument, type);
	argument = one_argument(argument, loc);
	argument = one_argument(argument, mod);
	one_argument(argument, bvector);

	if ((where = flag_value(apply_types, type)) < 0) {
		char_puts("Invalid apply type. Valid apply types are:\n\r", ch);
		show_flags(ch, apply_types);
		return FALSE;
	}

	if ((location = flag_value(apply_flags, loc)) < 0) {
		char_puts("Valid applies are:\n\r", ch);
		show_flags(ch, apply_flags);
		return FALSE;
	}

	if ((wd = where_lookup(where)) == NULL) {
		char_puts("OEdit: bit vector table undefined. "
			  "Report it to implementors.\n\r", ch);
		return FALSE;
	}

	if ((bv = flag_value(wd->table, bvector)) == 0) {
		char_puts("Valid bitvector types are:\n\r", ch);
		show_flags(ch, wd->table);
		return FALSE;
	}

	if (!is_number(mod)) {
		char_puts("Syntax: addapply type location "
			  "mod bitvector\n\r", ch);
		return FALSE;
	}

	pAf             = aff_new();
	pAf->location   = location;
	pAf->modifier   = atoi(mod);
	pAf->where	= where;
	pAf->type	= -1;
	pAf->duration   = -1;
	pAf->bitvector  = bv;
	pAf->level      = pObj->level;
	pAf->next       = pObj->affected;
	pObj->affected  = pAf;

	char_puts("Apply added.\n\r", ch);
	return TRUE;
}

/*
 * My thanks to Hans Hvidsten Birkeland and Noam Krendel(Walker)
 * for really teaching me how to manipulate pointers.
 */
OLC_FUN(objed_delaffect)
{
	OBJ_INDEX_DATA *pObj;
	AFFECT_DATA *pAf;
	AFFECT_DATA *pAf_next;
	char affect[MAX_STRING_LENGTH];
	int  value;
	int  cnt = 0;

	EDIT_OBJ(ch, pObj);

	one_argument(argument, affect);

	if (!is_number(affect) || affect[0] == '\0')
	{
		char_puts("Syntax:  delaffect [#xaffect]\n\r", ch);
		return FALSE;
	}

	value = atoi(affect);

	if (value < 0)
	{
		char_puts("Only non-negative affect-numbers allowed.\n\r", ch);
		return FALSE;
	}

	if (!(pAf = pObj->affected))
	{
		char_puts("OEdit:  Non-existant affect.\n\r", ch);
		return FALSE;
	}

	if(value == 0)	/* First case: Remove first affect */
	{
		pAf = pObj->affected;
		pObj->affected = pAf->next;
		aff_free(pAf);
	}
	else		/* Affect to remove is not the first */
	{
		while ((pAf_next = pAf->next) && (++cnt < value))
			 pAf = pAf_next;

		if(pAf_next)		/* See if it's the next affect */
		{
			pAf->next = pAf_next->next;
			aff_free(pAf_next);
		}
		else                                 /* Doesn't exist */
		{
			 char_puts("No such affect.\n\r", ch);
			 return FALSE;
		}
	}

	char_puts("Affect removed.\n\r", ch);
	return TRUE;
}

OLC_FUN(objed_name)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);
	return olced_str(ch, argument, objed_name, &pObj->name);
}

OLC_FUN(objed_short)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);
	return olced_mlstr(ch, argument, objed_short, &pObj->short_descr);
}

OLC_FUN(objed_long)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);
	return olced_mlstr(ch, argument, objed_long, &pObj->description);
}

bool set_value(CHAR_DATA *ch, OBJ_INDEX_DATA *pObj, const char *argument, int value)
{
	if (argument[0] == '\0') {
		set_obj_values(ch, pObj, -1, str_empty);
		return FALSE;
	}

	if (set_obj_values(ch, pObj, value, argument))
		return TRUE;

	return FALSE;
}



/*****************************************************************************
 Name:		objed_values
 Purpose:	Finds the object and sets its value.
 Called by:	The four valueX functions below. (now five -- Hugin)
 ****************************************************************************/
bool objed_values(CHAR_DATA *ch, const char *argument, int value)
{
	OBJ_INDEX_DATA *pObj;

	EDIT_OBJ(ch, pObj);

	if (set_value(ch, pObj, argument, value))
		 return TRUE;

	return FALSE;
}

OLC_FUN(objed_value0)
{
	return objed_values(ch, argument, 0);
}

OLC_FUN(objed_value1)
{
	return objed_values(ch, argument, 1);
}

OLC_FUN(objed_value2)
{
	return objed_values(ch, argument, 2);
}

OLC_FUN(objed_value3)
{
	return objed_values(ch, argument, 3);
}

OLC_FUN(objed_value4)
{
	return objed_values(ch, argument, 4);
}

OLC_FUN(objed_weight)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);
	return olced_number(ch, argument, objed_weight, &pObj->weight);
}

OLC_FUN(objed_limit)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);
	return olced_number(ch, argument, objed_limit, &pObj->limit);
}

OLC_FUN(objed_cost)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);
	return olced_number(ch, argument, objed_cost, &pObj->cost);
}

OLC_FUN(objed_exd)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);
	return olced_exd(ch, argument, &pObj->ed);
}

OLC_FUN(objed_extra)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);
	return olced_flag(ch, argument, objed_extra, &pObj->extra_flags);
}

OLC_FUN(objed_wear)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);
	return olced_flag(ch, argument, objed_wear, &pObj->wear_flags);
}

OLC_FUN(objed_type)
{
	bool changed;
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);
	changed = olced_flag(ch, argument, objed_type, &pObj->item_type);
	if (changed) {
		pObj->value[0] = 0;
		pObj->value[1] = 0;
		pObj->value[2] = 0;
		pObj->value[3] = 0;
		pObj->value[4] = 0;     /* ROM */
	}
	return changed;
}

OLC_FUN(objed_material)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);
	return olced_str(ch, argument, objed_material, &pObj->material);
}

OLC_FUN(objed_level)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);
	return olced_number(ch, argument, objed_level, &pObj->level);
}

OLC_FUN(objed_condition)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);
	return olced_number(ch, argument, objed_condition, &pObj->condition);
}

OLC_FUN(objed_clan)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);
	return olced_clan(ch, argument, objed_clan, &pObj->clan);
}

OLC_FUN(objed_clone)
{
	OBJ_INDEX_DATA *pObj;
	OBJ_INDEX_DATA *pFrom;
	char arg[MAX_INPUT_LENGTH];
	int i;
	AFFECT_DATA *paf;
	AFFECT_DATA *paf_next;
	AFFECT_DATA **ppaf;
	ED_DATA *ed;
	ED_DATA *ed_next;
	ED_DATA **ped;

	one_argument(argument, arg);
	if (!is_number(arg)) {
		char_puts("Syntax: clone <vnum>\n\r", ch);
		return FALSE;
	}

	i = atoi(arg);
	if ((pFrom = get_obj_index(i)) == NULL) {
		char_printf(ch, "ObjEd: %d: Vnum does not exist.\n\r", i);
		return FALSE;
	}

	EDIT_OBJ(ch, pObj);
	if (pObj == pFrom)
		return FALSE;

	free_string(pObj->name);
	pObj->name		= str_dup(pFrom->name);
	free_string(pObj->material);
	pObj->material		= str_dup(pFrom->material);
	mlstr_free(pObj->short_descr);
	pObj->short_descr	= mlstr_dup(pFrom->short_descr);
	mlstr_free(pObj->description);
	pObj->description	= mlstr_dup(pFrom->description);

	pObj->item_type		= pFrom->item_type;
	pObj->extra_flags	= pFrom->extra_flags;
	pObj->wear_flags	= pFrom->wear_flags;
	pObj->level		= pFrom->level;
	pObj->condition		= pFrom->condition;
	pObj->weight		= pFrom->weight;
	pObj->cost		= pFrom->cost;
	pObj->limit		= pFrom->limit;
	pObj->clan		= pFrom->clan;

	for (i = 0; i < 5; i++)
		pObj->value[i]	= pFrom->value[i];

/* copy affects */
	for (paf = pObj->affected; paf; paf = paf_next) {
		paf_next = paf->next;
		aff_free(paf);
	}

	ppaf = &pObj->affected;
	for (paf = pFrom->affected; paf; paf = paf->next) {
		*ppaf = aff_dup(paf);
		ppaf = &(*ppaf)->next;
	}

/* copy extra descriptions */
	for (ed = pObj->ed; ed; ed = ed_next) {
		ed_next = ed->next;
		ed_free(ed);
	}

	ped = &pObj->ed;
	for (ed = pFrom->ed; ed; ed = ed->next) {
		*ped = ed_dup(ed);
		ped = &(*ped)->next;
	}

	return TRUE;
}

void show_obj_values(BUFFER *output, OBJ_INDEX_DATA *obj)
{
	switch(obj->item_type) {
	default:	/* No values. */
		break;
		     
	case ITEM_LIGHT:
		if (obj->value[2] == -1 || obj->value[2] == 999) /* ROM OLC */
			buf_printf(output, "[v2] Light:  Infinite[-1]\n\r");
		else
			buf_printf(output, "[v2] Light:  [%d]\n\r", obj->value[2]);
		break;

	case ITEM_WAND:
	case ITEM_STAFF:
		buf_printf(output,
			"[v0] Level:          [%d]\n\r"
			"[v1] Charges Total:  [%d]\n\r"
			"[v2] Charges Left:   [%d]\n\r"
			"[v3] Spell:          %s\n\r",
			obj->value[0],
			obj->value[1],
			obj->value[2],
			skill_name(obj->value[3]));
		break;

	case ITEM_PORTAL:
		buf_printf(output,
			    "[v0] Charges:        [%d]\n\r"
			    "[v1] Exit Flags:     %s\n\r"
			    "[v2] Portal Flags:   %s\n\r"
			    "[v3] Goes to (vnum): [%d]\n\r",
			    obj->value[0],
			    flag_string(exit_flags, obj->value[1]),
			    flag_string(portal_flags , obj->value[2]),
			    obj->value[3]);
		break;
			
	case ITEM_FURNITURE:          
		buf_printf(output,
			    "[v0] Max people:      [%d]\n\r"
			    "[v1] Max weight:      [%d]\n\r"
			    "[v2] Furniture Flags: %s\n\r"
			    "[v3] Heal bonus:      [%d]\n\r"
			    "[v4] Mana bonus:      [%d]\n\r",
			    obj->value[0],
			    obj->value[1],
			    flag_string(furniture_flags, obj->value[2]),
			    obj->value[3],
			    obj->value[4]);
		break;

	case ITEM_SCROLL:
	case ITEM_POTION:
	case ITEM_PILL:
		buf_printf(output,
			"[v0] Level:  [%d]\n\r"
			"[v1] Spell:  %s\n\r"
			"[v2] Spell:  %s\n\r"
			"[v3] Spell:  %s\n\r"
			"[v4] Spell:  %s\n\r",
			obj->value[0],
			skill_name(obj->value[1]),
			skill_name(obj->value[2]),
			skill_name(obj->value[3]),
			skill_name(obj->value[4]));
		break;

/* ARMOR for ROM */

	case ITEM_ARMOR:
		buf_printf(output,
			"[v0] Ac pierce       [%d]\n\r"
			"[v1] Ac bash         [%d]\n\r"
			"[v2] Ac slash        [%d]\n\r"
			"[v3] Ac exotic       [%d]\n\r",
			obj->value[0],
			obj->value[1],
			obj->value[2],
			obj->value[3]);
			break;

/* WEAPON changed in ROM: */
/* I had to split the output here, I have no idea why, but it helped -- Hugin */
/* It somehow fixed a bug in showing scroll/pill/potions too ?! */
	case ITEM_WEAPON:
		buf_printf(output, "[v0] Weapon class:   %s\n\r",
			    flag_string(weapon_class, obj->value[0]));
		buf_printf(output, "[v1] Number of dice: [%d]\n\r", obj->value[1]);
		buf_printf(output, "[v2] Type of dice:   [%d]\n\r", obj->value[2]);
		buf_printf(output, "[v3] Type:           %s\n\r",
			    attack_table[obj->value[3]].name);
		buf_printf(output, "[v4] Special type:   %s\n\r",
			    flag_string(weapon_type2,  obj->value[4]));
		break;

	case ITEM_CONTAINER:
		buf_printf(output,
			"[v0] Weight:     [%d kg]\n\r"
			"[v1] Flags:      [%s]\n\r"
			"[v2] Key:     %s [%d]\n\r"
			"[v3] Capacity    [%d]\n\r"
			"[v4] Weight Mult [%d]\n\r",
			obj->value[0],
			flag_string(cont_flags, obj->value[1]),
		        get_obj_index(obj->value[2]) ?
			mlstr_mval(get_obj_index(obj->value[2])->short_descr) :
			"none",
		        obj->value[2],
		        obj->value[3],
		        obj->value[4]);
		break;

	case ITEM_DRINK_CON:
		buf_printf(output,
			    "[v0] Liquid Total: [%d]\n\r"
			    "[v1] Liquid Left:  [%d]\n\r"
			    "[v2] Liquid:       %s\n\r"
			    "[v3] Poisoned:     %s\n\r",
			    obj->value[0],
			    obj->value[1],
			    liq_table[obj->value[2]].liq_name,
			    obj->value[3] != 0 ? "Yes" : "No");
		break;

	case ITEM_FOUNTAIN:
		buf_printf(output,
			    "[v0] Liquid Total: [%d]\n\r"
			    "[v1] Liquid Left:  [%d]\n\r"
			    "[v2] Liquid:	    %s\n\r",
			    obj->value[0],
			    obj->value[1],
			    liq_table[obj->value[2]].liq_name);
		break;
			    
	case ITEM_FOOD:
		buf_printf(output,
			"[v0] Food hours: [%d]\n\r"
			"[v1] Full hours: [%d]\n\r"
			"[v3] Poisoned:   %s\n\r",
			obj->value[0],
			obj->value[1],
			obj->value[3] != 0 ? "Yes" : "No");
		break;

	case ITEM_MONEY:
		buf_printf(output, "[v0] Gold:   [%d]\n\r", obj->value[0]);
		break;
	}
}



bool set_obj_values(CHAR_DATA *ch, OBJ_INDEX_DATA *pObj,
		    int value_num, const char *argument)
{
	BUFFER *output;

	output = buf_new(-1);
	switch(pObj->item_type) {
	default:
		break;
		     
	case ITEM_LIGHT:
		switch (value_num) {
		default:
			do_help(ch, "ITEM_LIGHT");
			return FALSE;
		case 2:
			buf_add(output, "HOURS OF LIGHT SET.\n\r\n\r");
			pObj->value[2] = atoi(argument);
			break;
		}
		break;

	case ITEM_WAND:
	case ITEM_STAFF:
		switch (value_num) {
		default:
			do_help(ch, "ITEM_STAFF_WAND");
			return FALSE;
		case 0:
			buf_add(output, "SPELL LEVEL SET.\n\r\n\r");
			pObj->value[0] = atoi(argument);
			break;
		case 1:
			buf_add(output, "TOTAL NUMBER OF CHARGES SET.\n\r\n\r");
			pObj->value[1] = atoi(argument);
			break;
		case 2:
			buf_add(output, "CURRENT NUMBER OF CHARGES SET.\n\r\n\r");
			pObj->value[2] = atoi(argument);
			break;
		case 3:
			buf_add(output, "SPELL TYPE SET.\n\r");
			pObj->value[3] = sn_lookup(argument);
			break;
		}
		break;

	case ITEM_SCROLL:
	case ITEM_POTION:
	case ITEM_PILL:
		switch (value_num) {
		default:
			do_help(ch, "ITEM_SCROLL_POTION_PILL");
			return FALSE;
		case 0:
			buf_add(output, "SPELL LEVEL SET.\n\r\n\r");
			pObj->value[0] = atoi(argument);
			break;
		case 1:
			buf_add(output, "SPELL TYPE 1 SET.\n\r\n\r");
			pObj->value[1] = sn_lookup(argument);
			break;
		case 2:
			buf_add(output, "SPELL TYPE 2 SET.\n\r\n\r");
			pObj->value[2] = sn_lookup(argument);
			break;
		case 3:
			buf_add(output, "SPELL TYPE 3 SET.\n\r\n\r");
			pObj->value[3] = sn_lookup(argument);
			break;
		case 4:
			buf_add(output, "SPELL TYPE 4 SET.\n\r\n\r");
			pObj->value[4] = sn_lookup(argument);
			break;
 		}
		break;

/* ARMOR for ROM: */

	case ITEM_ARMOR:
		switch (value_num) {
		default:
			do_help(ch, "ITEM_ARMOR");
			return FALSE;
		case 0:
			buf_add(output, "AC PIERCE SET.\n\r\n\r");
			pObj->value[0] = atoi(argument);
			break;
		case 1:
			buf_add(output, "AC BASH SET.\n\r\n\r");
			pObj->value[1] = atoi(argument);
			break;
		case 2:
			buf_add(output, "AC SLASH SET.\n\r\n\r");
			pObj->value[2] = atoi(argument);
			break;
		case 3:
			buf_add(output, "AC EXOTIC SET.\n\r\n\r");
			pObj->value[3] = atoi(argument);
			break;
		}
		break;

/* WEAPONS changed in ROM */

	case ITEM_WEAPON:
		switch (value_num) {
		default:
			do_help(ch, "ITEM_WEAPON");
			return FALSE;
		case 0:
			buf_add(output, "WEAPON CLASS SET.\n\r\n\r");
			pObj->value[0] = flag_value(weapon_class, argument);
			break;
		case 1:
			buf_add(output, "NUMBER OF DICE SET.\n\r\n\r");
			pObj->value[1] = atoi(argument);
			break;
		case 2:
			buf_add(output, "TYPE OF DICE SET.\n\r\n\r");
			pObj->value[2] = atoi(argument);
			break;
		case 3:
			buf_add(output, "WEAPON TYPE SET.\n\r\n\r");
			pObj->value[3] = attack_lookup(argument);
			break;
		case 4:
			buf_add(output, "SPECIAL WEAPON TYPE TOGGLED.\n\r\n\r");
			pObj->value[4] ^= (flag_value(weapon_type2, argument) ?
			flag_value(weapon_type2, argument) : 0);
			break;
		}
		break;

	case ITEM_PORTAL:
		switch (value_num) {
		default:
			do_help(ch, "ITEM_PORTAL");
			return FALSE;
			        
		case 0:
			buf_add(output, "CHARGES SET.\n\r\n\r");
				     pObj->value[0] = atoi (argument);
			break;
		case 1:
			buf_add(output, "EXIT FLAGS SET.\n\r\n\r");
			pObj->value[1] = flag_value(exit_flags, argument);
			break;
		case 2:
			buf_add(output, "PORTAL FLAGS SET.\n\r\n\r");
			pObj->value[2] = flag_value(portal_flags, argument);
			break;
		case 3:
			buf_add(output, "EXIT VNUM SET.\n\r\n\r");
			pObj->value[3] = atoi (argument);
			break;
		}
		break;

	case ITEM_FURNITURE:
		switch (value_num) {
		default:
			do_help(ch, "ITEM_FURNITURE");
			return FALSE;
			        
		case 0:
			buf_add(output, "NUMBER OF PEOPLE SET.\n\r\n\r");
			pObj->value[0] = atoi (argument);
			break;
		case 1:
			buf_add(output, "MAX WEIGHT SET.\n\r\n\r");
			pObj->value[1] = atoi (argument);
			break;
		case 2:
		        buf_add(output, "FURNITURE FLAGS TOGGLED.\n\r\n\r");
			pObj->value[2] ^= (flag_value(furniture_flags, argument) ?
			flag_value(furniture_flags, argument) : 0);
			break;
		case 3:
			buf_add(output, "HEAL BONUS SET.\n\r\n\r");
			pObj->value[3] = atoi (argument);
			break;
		case 4:
			buf_add(output, "MANA BONUS SET.\n\r\n\r");
			pObj->value[4] = atoi (argument);
			break;
		}
		break;
		   
	case ITEM_CONTAINER:
		switch (value_num) {
			int value;
			
		default:
			do_help(ch, "ITEM_CONTAINER");
			return FALSE;
		case 0:
			buf_add(output, "WEIGHT CAPACITY SET.\n\r\n\r");
			pObj->value[0] = atoi(argument);
			break;
		case 1:
			if ((value = flag_value(cont_flags, argument)))
				TOGGLE_BIT(pObj->value[1], value);
			else {
				do_help (ch, "ITEM_CONTAINER");
				return FALSE;
			}
			buf_add(output, "CONTAINER TYPE SET.\n\r\n\r");
			break;
		case 2:
			if (atoi(argument) != 0) {
				if (!get_obj_index(atoi(argument))) {
					buf_add(output, "THERE IS NO SUCH ITEM.\n\r\n\r");
					return FALSE;
				}

				if (get_obj_index(atoi(argument))->item_type != ITEM_KEY) {
					buf_add(output, "THAT ITEM IS NOT A KEY.\n\r\n\r");
					return FALSE;
				}
			}
			buf_add(output, "CONTAINER KEY SET.\n\r\n\r");
			pObj->value[2] = atoi(argument);
			break;
		case 3:
			buf_add(output, "CONTAINER MAX WEIGHT SET.\n\r");
			pObj->value[3] = atoi(argument);
			break;
		case 4:
			buf_add(output, "WEIGHT MULTIPLIER SET.\n\r\n\r");
			pObj->value[4] = atoi (argument);
			break;
		}
		break;

	case ITEM_DRINK_CON:
		switch (value_num) {
		default:
			do_help(ch, "ITEM_DRINK");
/* OLC		    do_help(ch, "liquids");    */
			return FALSE;
		case 0:
			buf_add(output, "MAXIMUM AMOUT OF LIQUID HOURS SET.\n\r\n\r");
			pObj->value[0] = atoi(argument);
			break;
		case 1:
			buf_add(output, "CURRENT AMOUNT OF LIQUID HOURS SET.\n\r\n\r");
			pObj->value[1] = atoi(argument);
			break;
		case 2:
			buf_add(output, "LIQUID TYPE SET.\n\r\n\r");
			pObj->value[2] = (liq_lookup(argument) != -1 ?
			        		       liq_lookup(argument) : 0);
			break;
		case 3:
			buf_add(output, "POISON VALUE TOGGLED.\n\r\n\r");
			pObj->value[3] = (pObj->value[3] == 0) ? 1 : 0;
			break;
		}
		break;

	case ITEM_FOUNTAIN:
		switch (value_num) {
		default:
			do_help(ch, "ITEM_FOUNTAIN");
/* OLC		    do_help(ch, "liquids");    */
			return FALSE;
		case 0:
			buf_add(output, "MAXIMUM AMOUT OF LIQUID HOURS SET.\n\r\n\r");
			pObj->value[0] = atoi(argument);
			break;
		case 1:
			buf_add(output, "CURRENT AMOUNT OF LIQUID HOURS SET.\n\r\n\r");
			pObj->value[1] = atoi(argument);
			break;
		case 2:
			buf_add(output, "LIQUID TYPE SET.\n\r\n\r");
			pObj->value[2] = (liq_lookup(argument) != -1 ?
						liq_lookup(argument) : 0);
			break;
		}
		break;
			    	
	case ITEM_FOOD:
		switch (value_num) {
		default:
			do_help(ch, "ITEM_FOOD");
			return FALSE;
		case 0:
			buf_add(output, "HOURS OF FOOD SET.\n\r\n\r");
			pObj->value[0] = atoi(argument);
			break;
		case 1:
			buf_add(output, "HOURS OF FULL SET.\n\r\n\r");
			pObj->value[1] = atoi(argument);
			break;
		case 3:
			buf_add(output, "POISON VALUE TOGGLED.\n\r\n\r");
			pObj->value[3] = (pObj->value[3] == 0) ? 1 : 0;
			break;
		}
		break;

	case ITEM_MONEY:
		switch (value_num) {
		default:
			do_help(ch, "ITEM_MONEY");
			return FALSE;
		case 0:
			buf_add(output, "GOLD AMOUNT SET.\n\r\n\r");
			pObj->value[0] = atoi(argument);
			break;
		case 1:
			buf_add(output, "SILVER AMOUNT SET.\n\r\n\r");
			pObj->value[1] = atoi(argument);
			break;
		}
		break;
	}

	show_obj_values(output, pObj);

	send_to_char(buf_string(output), ch);
	buf_free(output);

	return TRUE;
}

/*****************************************************************************
 Name:		show_skill_cmds
 Purpose:	Displays all skill functions.
 		Does remove those damn immortal commands from the list.
 		Could be improved by:
 		(1) Adding a check for a particular class.
 		(2) Adding a check for a level range.
 ***************************************************************************/
static void show_skill_cmds(CHAR_DATA *ch, int tar)
{
	int  sn;
	int  col;
	BUFFER *output;
 
	output = buf_new(-1);
	col = 0;
	for (sn = 0; sn < skills.nused; sn++) {
		SKILL_DATA *sk = SKILL(sn);

		if (!str_cmp(sk->name, "reserved") || sk->spell_fun == NULL)
			continue;

		if (tar == -1 || sk->target == tar) {
			buf_printf(output, "%-19.18s", sk->name);
			if (++col % 4 == 0)
				buf_add(output, "\n\r");
		}
	}
 
	if (col % 4 != 0)
		buf_add(output, "\n\r");

	char_puts(buf_string(output), ch);
	buf_free(output);
}

void show_liqlist(CHAR_DATA *ch)
{
	int liq;
	BUFFER *buffer;
	
	buffer = buf_new(-1);
	
	for (liq = 0; liq_table[liq].liq_name != NULL; liq++) {
		if ((liq % 21) == 0)
			buf_add(buffer,"Name                 Color          Proof Full Thirst Food Ssize\n\r");

		buf_printf(buffer, "%-20s %-14s %5d %4d %6d %4d %5d\n\r",
			liq_table[liq].liq_name,liq_table[liq].liq_color,
			liq_table[liq].liq_affect[0],liq_table[liq].liq_affect[1],
			liq_table[liq].liq_affect[2],liq_table[liq].liq_affect[3],
			liq_table[liq].liq_affect[4]);
	}

	page_to_char(buf_string(buffer), ch);
	buf_free(buffer);
}

void show_damlist(CHAR_DATA *ch)
{
	int att;
	BUFFER *buffer;
	
	buffer = buf_new(-1);
	
	for (att = 0; attack_table[att].name != NULL; att++) {
		if ((att % 21) == 0)
			buf_add(buffer,"Name                 Noun\n\r");

		buf_printf(buffer, "%-20s %-20s\n\r",
			attack_table[att].name,attack_table[att].noun);
	}

	page_to_char(buf_string(buffer),ch);
	buf_free(buffer);
}

#if 0
	{ "type",	item_types,	"Types of objects."		},
	{ "extra",	extra_flags,	"Object attributes."		},
	{ "wear",	wear_flags,	"Where to wear object."		},
	{ "wear-loc",	wear_loc_flags,	"Where mobile wears object."	},
	{ "container",	cont_flags,"Container status."		},

/* ROM specific bits: */

	{ "armor",	ac_type,	"Ac for different attacks."	},
	{ "apply",	apply_flags,	"Apply flags"			},
	{ "wclass",     weapon_class,   "Weapon class."                }, 
	{ "wtype",      weapon_type2,   "Special weapon type."         },
	{ "portal",	portal_flags,	"Portal types."		},
	{ "furniture",	furniture_flags,"Furniture types."		},
	{ "liquid",	liq_table,	"Liquid types."		},
	{ "apptype",	apply_types,	"Apply types."			},
	{ "weapon",	attack_table,	"Weapon types."		},
	{ NULL,		NULL,		 NULL				}
};
#endif

VALIDATE_FUN(validate_condition)
{
	int val = *(int*) arg;

	if (val < 0 || val > 100) {
		char_puts("OEdit: condition can range from 0 (ruined) "
			  "to 100 (perfect).\n\r", ch);
		return FALSE;
	}
	return TRUE;
}

