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
 * $Id: olc_obj.c,v 1.78 1999-12-17 12:40:35 fjoe Exp $
 */

#include <sys/types.h>
#include <ctype.h>
#include <time.h>

#include "olc.h"
#include "chquest.h"

#define EDIT_OBJ(ch, obj)	(obj = (OBJ_INDEX_DATA*) ch->desc->pEdit)

DECLARE_OLC_FUN(objed_create		);
DECLARE_OLC_FUN(objed_edit		);
DECLARE_OLC_FUN(objed_touch		);
DECLARE_OLC_FUN(objed_show		);
DECLARE_OLC_FUN(objed_list		);
DECLARE_OLC_FUN(objed_del		);

DECLARE_OLC_FUN(objed_name		);
DECLARE_OLC_FUN(objed_short		);
DECLARE_OLC_FUN(objed_long		);
DECLARE_OLC_FUN(objed_addaffect		);
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

DECLARE_OLC_FUN(objed_stat		);
DECLARE_OLC_FUN(objed_obj		);
DECLARE_OLC_FUN(objed_wear		);
DECLARE_OLC_FUN(objed_type		);
DECLARE_OLC_FUN(objed_affect		);
DECLARE_OLC_FUN(objed_material		);
DECLARE_OLC_FUN(objed_level		);
DECLARE_OLC_FUN(objed_condition		);
DECLARE_OLC_FUN(objed_clone		);
DECLARE_OLC_FUN(objed_gender		);
DECLARE_OLC_FUN(objed_restrictions	);

DECLARE_VALIDATE_FUN(validate_condition);

olc_cmd_t olc_cmds_obj[] =
{
/*	{ command	function	validator	arg		}, */

	{ "create",	objed_create					},
	{ "edit",	objed_edit					},
	{ "",		NULL						},
	{ "touch",	objed_touch					},
	{ "show",	objed_show					},
	{ "list",	objed_list					},
	{ "delete_ob",	olced_spell_out					},
	{ "delete_obj",	objed_del					},

	{ "addaffect",	objed_addaffect					},
	{ "delaffect",	objed_delaffect					},
	{ "cost",	objed_cost					},
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

	{ "stat",	objed_stat,	NULL,		stat_flags	},
	{ "obj",	objed_obj,	NULL,		obj_flags	},
	{ "wear",	objed_wear,	NULL,		wear_flags	},
	{ "type",	objed_type,	NULL,		item_types	},
	{ "material",	objed_material, NULL,		&materials	},
	{ "level",	objed_level					},
	{ "condition",	objed_condition,validate_condition		},
	{ "clone",	objed_clone					},
	{ "gender",	objed_gender,	NULL,		gender_table	},
	{ "restrictions",objed_restrictions,				},	

	{ "version",	show_version					},
	{ "commands",	show_commands					},

	{ NULL }
};

OLC_FUN(objed_create)
{
	OBJ_INDEX_DATA *pObj;
	AREA_DATA *pArea;
	int  value;
	int  iHash;
	char arg[MAX_STRING_LENGTH];

	one_argument(argument, arg, sizeof(arg));
	value = atoi(arg);
	if (!value)
		OLC_ERROR("'OLC CREATE'");

	pArea = area_vnum_lookup(value);
	if (!pArea) {
		char_puts("ObjEd: That vnum is not assigned an area.\n", ch);
		return FALSE;
	}

	if (!IS_BUILDER(ch, pArea)) {
		char_puts("ObjEd: Insufficient security.\n", ch);
		return FALSE;
	}

	if (get_obj_index(value)) {
		char_puts("ObjEd: Object vnum already exists.\n", ch);
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
	OLCED(ch)		= olced_lookup(ED_OBJ);
	TOUCH_AREA(pArea);
	char_puts("ObjEd: Object created.\n", ch);
	return FALSE;
}

OLC_FUN(objed_edit)
{
	char arg[MAX_INPUT_LENGTH];
	int value;
	OBJ_INDEX_DATA *pObj;
	AREA_DATA *pArea;

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0')
		OLC_ERROR("'OLC EDIT'");

	value = atoi(arg);
	pObj = get_obj_index(value);
	if (!pObj) {
		char_puts("ObjEd: Vnum does not exist.\n", ch);
		return FALSE;
	}

	pArea = area_vnum_lookup(pObj->vnum);
	if (!IS_BUILDER(ch, pArea)) {
		char_puts("ObjEd: Insufficient security.\n", ch);
	       	return FALSE;
	}

	ch->desc->pEdit = (void*) pObj;
	OLCED(ch)	= olced_lookup(ED_OBJ);
	return FALSE;
}

OLC_FUN(objed_touch)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);
	TOUCH_VNUM(pObj->vnum);
	return FALSE;
}

OLC_FUN(objed_show)
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_INDEX_DATA	*pObj;
	AREA_DATA	*pArea;
	AFFECT_DATA *paf;
	int cnt;
	BUFFER *output;

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		if (IS_EDIT(ch, ED_OBJ))
			EDIT_OBJ(ch, pObj);
		else
			OLC_ERROR("'OLC EDIT'");
	}
	else {
		int value = atoi(arg);
		pObj = get_obj_index(value);
		if (!pObj) {
			char_puts("ObjEd: Vnum does not exist.\n", ch);
			return FALSE;
		}
	}

	pArea = area_vnum_lookup(pObj->vnum);

	output = buf_new(-1);
	buf_printf(output, "Name:        [%s]\n"
			   "Area:        [%5d] %s\n",
		pObj->name, pArea->vnum, pArea->name);

	buf_printf(output, "Vnum:        [%5d]\n"
			   "Type:        [%s]\n",
		pObj->vnum,
		flag_string(item_types, pObj->item_type));

	mlstr_dump(output, "Gender:      ", &pObj->gender);

	if (pObj->limit != -1)
		buf_printf(output, "Limit:       [%5d]\n", pObj->limit);
	else
		buf_add(output, "Limit:       [none]\n");

	buf_printf(output, "Level:       [%5d]\n", pObj->level);

	buf_printf(output, "Wear flags:  [%s]\n",
		flag_string(wear_flags, pObj->wear_flags));

	buf_printf(output, "Stat flags:  [%s]\n",
		flag_string(stat_flags, pObj->stat_flags));

	buf_printf(output, "Obj flags:   [%s]\n",
		flag_string(obj_flags, pObj->obj_flags));

	buf_printf(output, "Material:    [%s]\n",                /* ROM */
		pObj->material);

	buf_printf(output, "Condition:   [%5d]\n",               /* ROM */
		pObj->condition);

	buf_printf(output, "Weight:      [%5d]\nCost:        [%5d]\n",
		pObj->weight, pObj->cost);

	if (pObj->ed) {
		ED_DATA *ed;

		buf_add(output, "Ex desc kwd: ");

		for (ed = pObj->ed; ed; ed = ed->next)
			buf_printf(output, "[%s]", ed->keyword);

		buf_add(output, "\n");
	}

	mlstr_dump(output, "Short desc: ", &pObj->short_descr);
	mlstr_dump(output, "Long desc: ", &pObj->description);

	for (cnt = 0, paf = pObj->affected; paf; paf = paf->next) {
		where_t *w = where_lookup(paf->where);

		if (cnt == 0) {
			buf_add(output, "Number      Affects Modifier Affects Bitvector\n");
			buf_add(output, "------ ------------ -------- ------- -----------------------------------------\n");
		}
		buf_printf(output, "[%4d] %12.12s %8d %7.7s %s"
				   "\n",
			   cnt,
			   paf->where == TO_SKILLS ?
				STR(paf->location) :
				SFLAGS(apply_flags, paf->location),
			   paf->modifier,
			   flag_string(apply_types, paf->where),
			   w ? flag_string(w->table, paf->bitvector) : "none");
		cnt++;
	}

	objval_show(output, pObj->item_type, pObj->value);
	print_cc_vexpr(&pObj->restrictions, "Restrictions:", output);
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

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0')
		OLC_ERROR("'OLC ALIST'");

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
					   mlstr_mval(&pObjIndex->short_descr));
				if (++col % 3 == 0)
					buf_add(buffer, "\n");
			}
		}
	}

	if (!found)
		char_puts("Object(s) not found in this area.\n", ch);
	else {
		if (col % 3 != 0)
			buf_add(buffer, "\n");

		page_to_char(buf_string(buffer), ch);
	}

	buf_free(buffer);
	return FALSE;
}

OLC_FUN(objed_del)
{
	OBJ_INDEX_DATA *pObj;
	OBJ_DATA *obj, *obj_next;
	AREA_DATA *area;
	int i;
	bool error = FALSE;

	EDIT_OBJ(ch, pObj);

	if (olced_busy(ch, ED_OBJ, pObj, NULL))
		return FALSE;

/* check that pObj is not in resets */
	for (i = 0; i < MAX_KEY_HASH; i++) {
		ROOM_INDEX_DATA *room;

		for (room = room_index_hash[i]; room; room = room->next) {
			int j = 0;
			RESET_DATA *reset;

			for (reset = room->reset_first; reset;
							reset = reset->next) {
				bool found = FALSE;

				j++;
				switch (reset->command) {
				case 'P':
					if (reset->arg3 == pObj->vnum)
						found = TRUE;

					/* FALLTHRU */

				case 'O':
				case 'G':
				case 'E':
					if (reset->arg1 == pObj->vnum)
						found = TRUE;
					break;
				}

				if (!found)
					continue;

				if (!error) {
					error = TRUE;
					char_puts("ObjEd: can't delete obj "
						  "index: delete the "
						  "following resets:\n", ch);
				}

				char_printf(ch, "ObjEd: room %d, reset %d\n",
					    room->vnum, j);
			}
		}
	}

	if (error)
		return FALSE;

/* delete all the instances of obj index */
	if (!chquest_delete(ch, pObj))
		return FALSE;

	for (obj = object_list; obj; obj = obj_next) {
		obj_next = obj->next;

		if (obj->pObjIndex == pObj)
			extract_obj(obj, XO_F_NORECURSE);
	}

	if ((area = area_vnum_lookup(pObj->vnum)))
		TOUCH_AREA(area);

/* delete obj index itself */
	i = pObj->vnum % MAX_KEY_HASH;
	if (pObj == obj_index_hash[i])
		obj_index_hash[i] = pObj->next;
	else {
		OBJ_INDEX_DATA *prev;

		for (prev = obj_index_hash[i]; prev; prev = prev->next)
			if (prev->next == pObj)
				break;

		if (prev)
			prev->next = pObj->next;
	}

	free_obj_index(pObj);
	char_puts("ObjEd: Obj index deleted.\n", ch);
	edit_done(ch->desc);
	return FALSE;
}

/*
 * Need to issue warning if flag isn't valid. -- does so now -- Hugin.
 */
OLC_FUN(objed_addaffect)
{
	where_t *w;
	vo_t location;
	int modifier = 0;
	flag_t where;
	flag_t bitvector = 0;
	OBJ_INDEX_DATA *pObj;
	AFFECT_DATA *paf;
	char arg1[MAX_STRING_LENGTH];
	char arg2[MAX_STRING_LENGTH];

	EDIT_OBJ(ch, pObj);

	argument = one_argument(argument, arg1, sizeof(arg1));
	argument = one_argument(argument, arg2, sizeof(arg2));

	if (arg1[0] == '\0')
		OLC_ERROR("'OLC ADDAFFECT'");

	/*
	 * set `w' and `where'
	 */
	if (!str_cmp(arg1, "none")) {
		w = NULL;
		where = -1;
	} else {
		if ((where = flag_value(apply_types, arg1)) < 0) {
			char_puts("Valid bitaffect locations are:\n", ch);
			show_flags(ch, apply_types);
			return FALSE;
		}

		if ((w = where_lookup(where)) == NULL) {
			char_printf(ch, "%s: not in where_table.\n",
				    flag_string(apply_types, where));
			return FALSE;
		}
	}

	/*
	 * set `location' and initialize `modifier'
	 */
	switch (where) {
	case TO_SKILLS: {
		skill_t *sk;

		if ((sk = skill_lookup(arg2)) == NULL) {
			BUFFER *output = buf_new(-1);
			buf_add(output, "Valid skills are:\n");
			skills_dump(output, -1);
			page_to_char(buf_string(output), ch);
			buf_free(output);
			return FALSE;
		}

		location.s = gmlstr_mval(&sk->sk_name);
		argument = one_argument(argument, arg2, sizeof(arg2));
		break;
	}
	default:
		if (!str_cmp(arg2, "none")) {
			INT(location) = APPLY_NONE;
			modifier = -1;
		} else {
			if ((INT(location) = flag_value(apply_flags, arg2)) < 0) {
				char_puts("Valid locations are:\n", ch);
				show_flags(ch, apply_flags);
				return FALSE;
			}
			argument = one_argument(argument, arg2, sizeof(arg2));
		}
		break;
	}

	/*
	 * set `modifier'
	 */
	if (modifier < 0) {
		/*
		 * do not want modifier to be set
		 */
		modifier = 0;
	} else {
		if (!is_number(arg2))
			OLC_ERROR("'OLC ADDAFFECT'");

		modifier = atoi(arg2);
	}

	/*
	 * set `bitvector'
	 */
	if (w
	&&  argument[0] != '\0'
	&&  (bitvector = flag_value(w->table, argument)) == 0) {
		char_printf(ch, "Valid '%s' bitaffect flags are:\n",
			    flag_string(apply_types, where));
		show_flags(ch, w->table);
		return FALSE;
	}

	paf             = aff_new();
	switch (where) {
	case TO_SKILLS:
		paf->location.s = str_dup(location.s);
		break;
	default:
		paf->location = location;
		break;
	}
	paf->modifier   = modifier;
	paf->where	= where;
	paf->type       = str_empty;
	paf->duration   = -1;
	paf->bitvector  = bitvector;
	paf->level      = pObj->level;
	paf->next       = pObj->affected;
	pObj->affected  = paf;

	char_puts("Affect added.\n", ch);
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

	one_argument(argument, affect, sizeof(affect));

	if (!is_number(affect) || affect[0] == '\0')
	{
		char_puts("Syntax:  delaffect [#xaffect]\n", ch);
		return FALSE;
	}

	value = atoi(affect);

	if (value < 0)
	{
		char_puts("Only non-negative affect-numbers allowed.\n", ch);
		return FALSE;
	}

	if (!(pAf = pObj->affected))
	{
		char_puts("ObjEd:  Non-existant affect.\n", ch);
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
			 char_puts("No such affect.\n", ch);
			 return FALSE;
		}
	}

	char_puts("Affect removed.\n", ch);
	return TRUE;
}

OLC_FUN(objed_name)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);
	return olced_name(ch, argument, cmd, &pObj->name);
}

OLC_FUN(objed_short)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);
	return olced_mlstr(ch, argument, cmd, &pObj->short_descr);
}

OLC_FUN(objed_long)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);
	return olced_mlstr(ch, argument, cmd, &pObj->description);
}

/*****************************************************************************
 Name:		objed_values
 Purpose:	Finds the object and sets its value.
 Called by:	The four valueX functions below. (now five -- Hugin)
 ****************************************************************************/
bool objed_values(CHAR_DATA *ch, const char *argument, int val_num)
{
	BUFFER *output;
	int errcode = 1;
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);

	output = buf_new(-1);
	if (argument[0] == '\0'
	||  (errcode = objval_set(output, pObj->item_type, pObj->value,
				  val_num, argument)) < 2)
		objval_show(output, pObj->item_type, pObj->value);
	page_to_char(buf_string(output), ch);
	buf_free(output);
	return !errcode;
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
	return olced_number(ch, argument, cmd, &pObj->weight);
}

OLC_FUN(objed_limit)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);
	return olced_number(ch, argument, cmd, &pObj->limit);
}

OLC_FUN(objed_cost)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);
	return olced_number(ch, argument, cmd, &pObj->cost);
}

OLC_FUN(objed_exd)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);
	return olced_exd(ch, argument, cmd, &pObj->ed);
}

OLC_FUN(objed_stat)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);
	return olced_flag(ch, argument, cmd, &pObj->stat_flags);
}

OLC_FUN(objed_obj)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);
	return olced_flag(ch, argument, cmd, &pObj->obj_flags);
}

OLC_FUN(objed_wear)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);
	return olced_flag(ch, argument, cmd, &pObj->wear_flags);
}

OLC_FUN(objed_type)
{
	bool changed;
	flag_t old_item_type;
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);
	old_item_type = pObj->item_type;
	changed = olced_flag(ch, argument, cmd, &pObj->item_type);
	if (changed) {
		objval_destroy(old_item_type, pObj->value);
		objval_init(pObj->item_type, pObj->value);
	}
	return changed;
}

OLC_FUN(objed_material)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);
	return olced_foreign_strkey(ch, argument, cmd, &pObj->material);
}

OLC_FUN(objed_level)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);
	return olced_number(ch, argument, cmd, &pObj->level);
}

OLC_FUN(objed_condition)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);
	return olced_number(ch, argument, cmd, &pObj->condition);
}

OLC_FUN(objed_clone)
{
	OBJ_INDEX_DATA *pObj;
	OBJ_INDEX_DATA *pFrom;
	char arg[MAX_INPUT_LENGTH];
	int i;

	one_argument(argument, arg, sizeof(arg));
	if (!is_number(arg))
		OLC_ERROR("'OLC CLONE'");

	i = atoi(arg);
	if ((pFrom = get_obj_index(i)) == NULL) {
		char_printf(ch, "ObjEd: %d: Vnum does not exist.\n", i);
		return FALSE;
	}

	EDIT_OBJ(ch, pObj);
	if (pObj == pFrom)
		return FALSE;

	free_string(pObj->name);
	pObj->name		= str_qdup(pFrom->name);
	free_string(pObj->material);
	pObj->material		= str_qdup(pFrom->material);
	mlstr_cpy(&pObj->short_descr, &pFrom->short_descr);
	mlstr_cpy(&pObj->description, &pFrom->description);

	objval_destroy(pObj->item_type, pObj->value);
	pObj->item_type		= pFrom->item_type;
	objval_cpy(pFrom->item_type, pObj->value, pFrom->value);

	pObj->stat_flags	= pFrom->stat_flags;
	pObj->obj_flags		= pFrom->obj_flags;
	pObj->wear_flags	= pFrom->wear_flags;
	pObj->level		= pFrom->level;
	pObj->condition		= pFrom->condition;
	pObj->weight		= pFrom->weight;
	pObj->cost		= pFrom->cost;
	pObj->limit		= pFrom->limit;

/* copy affects */
	aff_free_list(pObj->affected);
	pObj->affected = aff_dup_list(pFrom->affected);

/* copy extra descriptions */
	ed_free(pObj->ed);
	pObj->ed = ed_dup(pFrom->ed);

	return TRUE;
}

OLC_FUN(objed_gender)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);
	return olced_gender(ch, argument, cmd, &pObj->gender);
}

OLC_FUN(objed_restrictions)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);
	return olced_cc_vexpr(ch, argument, cmd, &pObj->restrictions, "obj_wear");
}

VALIDATE_FUN(validate_condition)
{
	int val = *(int*) arg;

	if (val < 0 || val > 100) {
		char_puts("ObjEd: condition can range from 0 (ruined) "
			  "to 100 (perfect).\n", ch);
		return FALSE;
	}
	return TRUE;
}
