/*-
 * Copyright (c) 1999 arborn <avn@org.chem.msu.su>
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
 * $Id: olc_class.c,v 1.10 1999-11-18 18:41:32 fjoe Exp $
 */

#include "olc.h"
#include "string_edit.h"

#define EDIT_CLASS(ch, class)	(class = (class_t*) ch->desc->pEdit)

DECLARE_OLC_FUN(classed_create		);
DECLARE_OLC_FUN(classed_edit		);
DECLARE_OLC_FUN(classed_save		);
DECLARE_OLC_FUN(classed_touch		);
DECLARE_OLC_FUN(classed_show		);
DECLARE_OLC_FUN(classed_list		);

DECLARE_OLC_FUN(classed_whoname		);
DECLARE_OLC_FUN(classed_primary		);
DECLARE_OLC_FUN(classed_weapon		);
DECLARE_OLC_FUN(classed_thac00		);
DECLARE_OLC_FUN(classed_thac32		);
DECLARE_OLC_FUN(classed_hprate		);
DECLARE_OLC_FUN(classed_manarate	);
DECLARE_OLC_FUN(classed_points		);
DECLARE_OLC_FUN(classed_deaths		);
DECLARE_OLC_FUN(classed_flags		);
DECLARE_OLC_FUN(classed_align		);
DECLARE_OLC_FUN(classed_ethos		);
DECLARE_OLC_FUN(classed_sex		);
DECLARE_OLC_FUN(classed_stats		);
DECLARE_OLC_FUN(classed_poses		);
DECLARE_OLC_FUN(classed_skillspec	);
DECLARE_OLC_FUN(classed_guilds		);

DECLARE_OLC_FUN(olc_skill_update	);

static DECLARE_VALIDATE_FUN(validate_whoname);

olced_strkey_t strkey_classes = { &classes, CLASSES_PATH };

olc_cmd_t olc_cmds_class[] =
{
	{ "create",	classed_create					},
	{ "edit",	classed_edit					},
	{ "",		classed_save					},
	{ "touch",	classed_touch					},
	{ "show",	classed_show					},
	{ "list",	classed_list					},

	{ "name",	olced_strkey,		NULL,	&strkey_classes	},
	{ "whoname",	classed_whoname,	validate_whoname	},
	{ "primary",	classed_primary,	NULL,	stat_names	},
	{ "weapon",	classed_weapon					},
	{ "thac00",	classed_thac00					},
	{ "thac32",	classed_thac32					},
	{ "hprate",	classed_hprate					},
	{ "manarate",	classed_manarate				},
	{ "points",	classed_points					},
	{ "deaths",	classed_deaths					},
	{ "flags",	classed_flags,		NULL,	class_flags	},
	{ "align",	classed_align,		NULL,	ralign_names	},
	{ "ethos",	classed_ethos,		NULL,	ethos_table	},
	{ "sex",	classed_sex,		NULL,	sex_table	},
	{ "stats",	classed_stats					},
	{ "poses",	classed_poses					},
	{ "skillspec",	classed_skillspec,	validate_skill_spec	},
	{ "guilds",	classed_guilds					},

	{ "update",	olc_skill_update				},
	{ "commands",	show_commands					},
	{ NULL }
};

typedef struct _save_class_t {
	CHAR_DATA *	ch;
	bool		found;
} _save_class_t;
static void * save_class_cb(void *p, void *d);

OLC_FUN(classed_create)
{
	class_t *cl;
	class_t class;
	char arg[MAX_INPUT_LENGTH];

	if (PC(ch)->security < SECURITY_CLASS) {
		char_puts("ClassEd: Insufficient security for creating classes\n",
			  ch);
		return FALSE;
	}

	first_arg(argument, arg, sizeof(arg), FALSE);
	if (arg[0] == '\0') {
		dofun("help", ch, "'OLC CREATE'");
		return FALSE;
	}

	/*
	 * olced_busy check is not needed since hash_insert
	 * adds new elements to the end of varr
	 */

	class_init(&class);
	class.name	= str_dup(arg);
	cl = hash_insert(&classes, class.name, &class);
	class_destroy(&class);

	if (cl == NULL) {
		char_printf(ch, "ClassEd: %s: already exists.\n", arg);
		return FALSE;
	}

	ch->desc->pEdit	= cl;
	OLCED(ch)	= olced_lookup(ED_CLASS);
	touch_class(cl);
	char_puts("Class created.\n",ch);
	return FALSE;
}

OLC_FUN(classed_edit)
{
	class_t *cl;

	if (PC(ch)->security < SECURITY_CLASS) {
		char_puts("ClassEd: Insufficient security.\n", ch);
		return FALSE;
	}

	if (argument[0] == '\0') {
		dofun("help", ch, "'OLC EDIT'");
		return FALSE;
	}

	if ((cl = class_search(argument)) == NULL) {
		char_printf(ch, "ClassEd: %s: No such class.\n", argument);
		return FALSE;
	}

	ch->desc->pEdit	= cl;
	OLCED(ch)	= olced_lookup(ED_CLASS);
	return FALSE;
}

OLC_FUN(classed_save)
{
	_save_class_t sc;

	olc_printf(ch, "Saved classes:");
	sc.ch = ch;
	sc.found = FALSE;
	hash_foreach(&classes, save_class_cb, &sc);
	if (!sc.found)
		olc_printf(ch, "    None.");
	return FALSE;
}

OLC_FUN(classed_touch)
{
	class_t *class;
	EDIT_CLASS(ch, class);

	return touch_class(class);
}

OLC_FUN(classed_show)
{
	int i;
	BUFFER *output;
	class_t *class;
	bool found;

	if (argument[0] == '\0') {
		if (IS_EDIT(ch, ED_CLASS))
			EDIT_CLASS(ch, class);
		else {
			dofun("help", ch, "'OLC ASHOW'");
			return FALSE;
		}
	} else {
		if ((class = class_search(argument)) == NULL) {
			char_printf(ch, "ClassEd: %s: No such class.\n",
				    argument);
			return FALSE;
		}
	}

	output = buf_new(-1);
	buf_printf(output, "Name:           [%3s] [%s]\n",
		   class->who_name, class->name);
	buf_printf(output, "Primary attr:   [%s]\n",
		   flag_string(stat_names, class->attr_prime));
	if (!IS_NULLSTR(class->skill_spec))
		buf_printf(output, "SkillSpec:      [%s]\n", class->skill_spec);
	if (class->weapon)
		buf_printf(output, "School weapon:  [%d]\n", class->weapon);
	buf_printf(output, "THAC0 (level 0) [%d] THAC0 (level 32) [%d]\n",
		   class->thac0_00, class->thac0_32);
	buf_printf(output, "HP rate:        [%d%%] Mana rate:      [%d%%]\n",
		   class->hp_rate, class->mana_rate);
	if (class->class_flags)
		buf_printf(output, "Class flags:    [%s]\n",
			   flag_string(class_flags, class->class_flags));
	if (class->points)
		buf_printf(output, "Exp points:     [%d]\n", class->points);

	for (i = 0, found = FALSE; i < MAX_STATS; i++)
		if (class->stats[i]) found = TRUE;
	if (found) {
		buf_add(output, "Stats mod:      [");
		for (i = 0; i < MAX_STATS; i++)
			buf_printf(output, "%s: %2d ",
				   flag_string(stat_names, i),
				   class->stats[i]);
		buf_add(output, "]\n");
	}
	if (class->restrict_align)
		buf_printf(output, "Align restrict: [%s]\n",
			   flag_string(ralign_names, class->restrict_align));
	if (class->restrict_ethos)
		buf_printf(output, "Ethos restrict: [%s]\n",
			   flag_string(ethos_table, class->restrict_ethos));
	if (class->restrict_sex != -1)
		buf_printf(output, "Sex restrict:   [%s]\n",
			   flag_string(sex_table, class->restrict_sex));
	if (class->death_limit != -1)
		buf_printf(output, "Death limit:    [%d]\n", class->death_limit);

	page_to_char(buf_string(output), ch);
	buf_free(output);

	return FALSE;
}

OLC_FUN(classed_list)
{
	BUFFER *buffer = buf_new(-1);
	strkey_printall(&classes, buffer);
	page_to_char(buf_string(buffer), ch);
	buf_free(buffer);
	return FALSE;
}

OLC_FUN(classed_whoname)
{
	const char *str;
	class_t *class;
	EDIT_CLASS(ch, class);

	str = str_dup(class->who_name);
	if (olced_str(ch, argument, cmd, &str)) {
		strnzcpy(class->who_name, sizeof(class->who_name), str);
		free_string(str);
		return TRUE;
	}
	free_string(str);
	return FALSE;
}

OLC_FUN(classed_primary		)
{
	class_t *class;
	EDIT_CLASS(ch, class);

	return olced_flag32(ch, argument, cmd, &class->attr_prime);
}

OLC_FUN(classed_weapon		)
{
	class_t *class;
	OBJ_INDEX_DATA *weap;
	EDIT_CLASS(ch, class);

	if ((weap = get_obj_index(atoi(argument))) == NULL) {
		char_puts("ClassEd: Object doesn't exist.\n", ch);
		return FALSE;
	}
	if (weap->item_type != ITEM_WEAPON) {
		char_puts("ClassEd: That obj is not a weapon.\n", ch);
		return FALSE;
	}
	return olced_number(ch, argument, cmd, &class->weapon);
}

OLC_FUN(classed_thac00		)
{
	class_t *class;
	EDIT_CLASS(ch, class);
	return olced_number(ch, argument, cmd, &class->thac0_00);
}

OLC_FUN(classed_thac32		)
{
	class_t *class;
	EDIT_CLASS(ch, class);
	return olced_number(ch, argument, cmd, &class->thac0_32);
}

OLC_FUN(classed_hprate		)
{
	class_t *class;
	EDIT_CLASS(ch, class);
	return olced_number(ch, argument, cmd, &class->hp_rate);
}

OLC_FUN(classed_manarate	)
{
	class_t *class;
	EDIT_CLASS(ch, class);
	return olced_number(ch, argument, cmd, &class->mana_rate);
}

OLC_FUN(classed_points		)
{
	class_t *class;
	EDIT_CLASS(ch, class);
	return olced_number(ch, argument, cmd, &class->points);
}

OLC_FUN(classed_deaths		)
{
	class_t *class;
	EDIT_CLASS(ch, class);
	return olced_number(ch, argument, cmd, &class->death_limit);
}

OLC_FUN(classed_flags		)
{
	class_t *class;
	EDIT_CLASS(ch, class);
	return olced_flag32(ch, argument, cmd, &class->class_flags);
}

OLC_FUN(classed_stats)
{
	class_t *class;
        char arg[MAX_INPUT_LENGTH];
	char *endptr;
	int i, val;
	bool st = FALSE;

	EDIT_CLASS(ch, class);

	for (i = 0; i < MAX_STATS; i++) {
		argument = one_argument(argument, arg, sizeof(arg));
		if (*arg == '\0') break;
		val = strtol(arg, &endptr, 0);
		if (*arg == '\0' || *endptr != '\0') break;
		class->stats[i] = val;
		st = TRUE;
	}
	
	if (!st) char_printf(ch, "Syntax: %s attr1 attr2 ...\n", cmd->name);
		else char_puts("Ok.\n", ch);
	return st;
}

OLC_FUN(classed_align		)
{
	class_t *class;
	EDIT_CLASS(ch, class);
	return olced_flag32(ch, argument, cmd, &class->restrict_align);
}

OLC_FUN(classed_ethos		)
{
	class_t *class;
	EDIT_CLASS(ch, class);
	return olced_flag32(ch, argument, cmd, &class->restrict_ethos);
}

OLC_FUN(classed_sex		)
{
	class_t *class;
	EDIT_CLASS(ch, class);

	if (!olced_flag32(ch, argument, cmd, &class->restrict_sex))
		return FALSE;
	if (class->restrict_sex != SEX_MALE
	&&  class->restrict_sex != SEX_FEMALE)
		class->restrict_sex = -1;
	return TRUE;
}

OLC_FUN(classed_poses)
{
	class_t *class;
	char arg[MAX_INPUT_LENGTH];
	pose_t *pose;

	EDIT_CLASS(ch, class);

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0' || !str_prefix(arg, "list")) {
		int i;
		BUFFER	*buffer;
		bool st = FALSE;

		buffer = buf_new(-1);
		for (i = 0; i < class->poses.nused; i++) {
			pose = VARR_GET(&class->poses, i);
			if (IS_NULLSTR(pose->self) && IS_NULLSTR(pose->others))
				continue;
			st = TRUE;
			buf_printf(buffer, "[%3d] Self:   %s\n      Others: %s\n",
				   i, pose->self, pose->others);
		}
		if (!st)
			buf_add(buffer, "No poses have been defined"
					" for this class.\n");
		page_to_char(buf_string(buffer), ch);
		buf_free(buffer);
		return FALSE;
	}

	if (!str_prefix(arg, "delete")) {
		argument = one_argument(argument, arg, sizeof(arg));
		pose = varr_get(&class->poses, atoi(arg));
		if (pose == NULL) {
			char_printf(ch, "ClassEd: no such pose: %s\n", arg);
			return FALSE;
		}

		varr_edelete(&class->poses, pose);
		char_puts("Pose deleted.\n", ch);
		return TRUE;
	}

	if (!str_prefix(arg, "self")) {
		argument = one_argument(argument, arg, sizeof(arg));
		pose = varr_get(&class->poses, atoi(arg));
		if (pose == NULL) {
			char_printf(ch, "ClassEd: no such pose: %s\n", arg);
			return FALSE;
		}

		free_string(pose->self);
		pose->self = str_dup(argument);
		char_puts("Ok.\n", ch);
		return TRUE;
	}

	if (!str_prefix(arg, "others")) {
		argument = one_argument(argument, arg, sizeof(arg));
		pose = varr_get(&class->poses, atoi(arg));
		if (pose == NULL) {
			char_printf(ch, "ClassEd: no such pose: %s\n", arg);
			return FALSE;
		}

		free_string(pose->others);
		pose->others = str_dup(argument);
		char_puts("Ok.\n", ch);
		return TRUE;
	}

	if (!str_prefix(arg, "add")) {
		pose = varr_enew(&class->poses);
		pose->self = str_dup("#");
		pose->others = str_dup("#");
		char_puts("Pose added.\n", ch);
		return TRUE;
	}

	if (!str_prefix(arg, "sort")) {
		varr_qsort(&class->poses, cmpstr);
		char_puts("Ok.\n", ch);
		return TRUE;
	}

	dofun("help", ch, "'OLC CLASS POSES'");
	return FALSE;
}

OLC_FUN(classed_skillspec)
{
	class_t *class;
	EDIT_CLASS(ch, class);
	return olced_str(ch, argument, cmd, &class->skill_spec);
}

OLC_FUN(classed_guilds)
{
	class_t *class;
	char arg[MAX_INPUT_LENGTH];
	int vnum;
	ROOM_INDEX_DATA *room;

	EDIT_CLASS(ch, class);

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0' || !str_prefix(arg, "list")) {
		int i;
		BUFFER	*buffer;
		bool st = FALSE;

		buffer = buf_new(-1);
		for (i = 0; i < class->guilds.nused; i++) {
			vnum = *(int*) VARR_GET(&class->guilds, i);
			if (!vnum)
				continue;
			if ((room = get_room_index(vnum)) == NULL) {
				buf_printf(buffer, "[%5d] Nonexistant.\n",
					   vnum);
				continue;
			}
			st = TRUE;
			buf_printf(buffer, "[%5d] %-25.24s\n",
				   vnum, mlstr_mval(&room->name));
		}

		if (!st)
			buf_add(buffer, "No guild rooms have been defined"
					" for this class.\n");
		page_to_char(buf_string(buffer), ch);
		buf_free(buffer);
		return FALSE;
	}

	if (!str_prefix(arg, "add")) {
		int *pvnum;

		argument = one_argument(argument, arg, sizeof(arg));
		vnum = atoi(arg);
		if ((room = get_room_index(vnum)) == NULL) {
			char_printf(ch, "ClassEd: %d: no such room.\n", vnum);
			return FALSE;
		}

		if (varr_bsearch(&class->guilds, &vnum, cmpint)) {
			char_printf(ch, "ClassEd: %d: already in list.\n",
				    vnum);
			return FALSE;
		}

		pvnum = varr_enew(&class->guilds);
		*pvnum = vnum;
		varr_qsort(&class->guilds, cmpint);
		char_puts("Guild added.\n", ch);
		return TRUE;
	}

	if (!str_prefix(arg, "delete")) {
		int *pvnum;

		argument = one_argument(argument, arg, sizeof(arg));
		vnum = atoi(arg);
		pvnum = varr_bsearch(&class->guilds, &vnum, cmpint);
		if (pvnum == NULL) {
			char_printf(ch, "ClassEd: %d: not in list.\n", vnum);
			return FALSE;
		}

		*pvnum = 0;
		varr_qsort(&class->guilds, cmpint);
		char_puts("Guild deleted.\n", ch);
		return TRUE;
	}

	dofun("help", ch, "'OLC CLASS SKILLS'");
	return FALSE;
}

bool touch_class(class_t *class)
{
	SET_BIT(class->class_flags, CLASS_CHANGED);
	return FALSE;
}

static void *
whoname_cb(void *p, void *d)
{
	class_t *cl = (class_t *) p;

	if (!str_cmp(cl->who_name, d))
		return p;

	return NULL;
}

static VALIDATE_FUN(validate_whoname)
{
	class_t *cl;
	class_t *cl2;

	EDIT_CLASS(ch, cl);

	if (strlen(arg) > 3 || strlen(arg) < 1) {
		char_puts("ClassEd: whoname should be 1..3 symbols long.\n", ch);
		return FALSE;
	}

	if ((cl2 = hash_foreach(&classes, whoname_cb, (void*) arg)) != NULL
	&&  cl2 != cl) {
		char_printf(ch, "ClassEd: %s: duplicate class whoname.\n", arg);
		return FALSE;
	}

	return TRUE;
}

#define PROC_STR(s)	((s) ? (s) : (str_empty))

static void *
save_class_cb(void *p, void *d)
{
	class_t *cl = (class_t *) p;
	_save_class_t *sc = (_save_class_t *) d;

	int i;
	FILE *fp;
	char buf[PATH_MAX];

	snprintf(buf, sizeof(buf), "%s.%s", strkey_filename(cl->name), CLASS_EXT);
	if ((fp = olc_fopen(CLASSES_PATH, buf, sc->ch, -1)) == NULL)
		return NULL;

	REMOVE_BIT(cl->class_flags, RACE_CHANGED);
	fprintf(fp, "#CLASS\n");
	fprintf(fp, "Name %s~\n", cl->name);
	fprintf(fp, "ShortName %s~\n", cl->who_name);
	fprintf(fp, "PrimeStat %s\n", flag_string(stat_names, cl->attr_prime));
	if (!IS_NULLSTR(cl->skill_spec))
		fprintf(fp, "SkillSpec '%s'\n", cl->skill_spec);
	fprintf(fp, "SchoolWeapon %d\n", cl->weapon);
	for (i = 0; i < cl->guilds.nused; i++)
		if (*(int*)VARR_GET(&cl->guilds, i) != 0)
			fprintf(fp, "GuildRoom %d\n", *(int*)VARR_GET(&cl->guilds, i));
	fprintf(fp, "Thac0_00 %d\n", cl->thac0_00);
	fprintf(fp, "Thac0_32 %d\n", cl->thac0_32);
	fprintf(fp, "HPRate %d\n", cl->hp_rate);
	fprintf(fp, "ManaRate %d\n", cl->mana_rate);
	if (cl->class_flags)
		fprintf(fp, "Flags %s~\n",
			flag_string(class_flags, cl->class_flags));
	if (cl->points)
		fprintf(fp, "AddExp %d\n", cl->points);
	fprintf(fp, "StatMod");
	for (i = 0; i < MAX_STATS; i++)
		fprintf(fp, " %d", cl->stats[i]);
	fprintf(fp, "\n");
	if (cl->restrict_align)
		fprintf(fp, "RestrictAlign %s~\n",
			flag_string(ralign_names, cl->restrict_align));
	if (cl->restrict_sex != -1)
		fprintf(fp, "RestrictSex %s~\n",
			flag_string(sex_table, cl->restrict_sex));
	if (cl->restrict_ethos)
		fprintf(fp, "RestrictEthos %s~\n",
			flag_string(ethos_table, cl->restrict_ethos));
	if (cl->death_limit != -1)
		fprintf(fp, "DeathLimit %d\n", cl->death_limit);
	fprintf(fp, "End\n\n");

	for (i = 0; i < cl->poses.nused; i++) {
		pose_t *pose = VARR_GET(&cl->poses, i);
		if (IS_NULLSTR(pose->self) && IS_NULLSTR(pose->others))
			continue;
		fprintf(fp, "#POSE\n");
		fwrite_string(fp, "Self", pose->self);
		fwrite_string(fp, "Others", pose->others);
		fprintf(fp, "End\n\n");
	}

	fprintf(fp, "#$\n");
	fclose(fp);

	sc->found = TRUE;
	olc_printf(sc->ch, "    %s (%s)", cl->name, buf);
	return NULL;
}
