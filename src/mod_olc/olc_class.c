/*-
 * Copyright (c) 1999 SoG Development Team
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
 * $Id: olc_class.c,v 1.38 2003-09-29 23:11:41 fjoe Exp $
 */

#include "olc.h"

#define EDIT_CLASS(ch, class)	(class = (class_t *) ch->desc->pEdit)

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
DECLARE_OLC_FUN(classed_luckbonus	);
DECLARE_OLC_FUN(classed_guilds		);

DECLARE_OLC_FUN(olc_skill_update	);

static DECLARE_VALIDATE_FUN(validate_whoname);

olced_strkey_t strkey_classes = { &classes, CLASSES_PATH, CLASS_EXT };

olc_cmd_t olc_cmds_class[] =
{
	{ "create",	classed_create,		NULL,	NULL		},
	{ "edit",	classed_edit,		NULL,	NULL		},
	{ "",		classed_save,		NULL,	NULL		},
	{ "touch",	classed_touch,		NULL,	NULL		},
	{ "show",	classed_show,		NULL,	NULL		},
	{ "list",	classed_list,		NULL,	NULL		},

	{ "name",	olced_strkey,		NULL,	&strkey_classes	},
	{ "whoname",	classed_whoname,	validate_whoname, NULL	},
	{ "primary",	classed_primary,	NULL,	stat_aliases	},
	{ "weapon",	classed_weapon,		NULL,	NULL		},
	{ "thac00",	classed_thac00,		NULL,	NULL		},
	{ "thac32",	classed_thac32,		NULL,	NULL		},
	{ "hprate",	classed_hprate,		NULL,	NULL		},
	{ "manarate",	classed_manarate,	NULL,	NULL		},
	{ "points",	classed_points,		NULL,	NULL		},
	{ "deaths",	classed_deaths,		NULL,	NULL		},
	{ "flags",	classed_flags,		NULL,	class_flags	},
	{ "align",	classed_align,		NULL,	ralign_names	},
	{ "ethos",	classed_ethos,		NULL,	ethos_table	},
	{ "sex",	classed_sex,		NULL,	sex_table	},
	{ "stats",	classed_stats,		NULL,	NULL		},
	{ "poses",	classed_poses,		NULL,	NULL		},
	{ "skillspec",	classed_skillspec,	validate_skill_spec, NULL },
	{ "guilds",	classed_guilds,		NULL,	NULL		},
	{ "luckbonus",	classed_luckbonus,	NULL,	NULL		},

	{ "update",	olc_skill_update,	NULL,	NULL		},

	{ "commands",	show_commands,		NULL,	NULL		},
	{ "version",	show_version,		NULL,	NULL		},

	{ NULL, NULL, NULL, NULL }
};

static bool touch_class(class_t *class);

OLC_FUN(classed_create)
{
	class_t *cl;
	char arg[MAX_INPUT_LENGTH];

	if (PC(ch)->security < SECURITY_CLASS) {
		act_char("ClassEd: Insufficient security for creating classes.", ch);
		return FALSE;
	}

	first_arg(argument, arg, sizeof(arg), FALSE);
	if (arg[0] == '\0')
		OLC_ERROR("'OLC CREATE'");

	if ((cl = c_insert(&classes, arg)) == NULL) {
		act_puts("ClassEd: $t: already exists.",
			 ch, arg, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	cl->name = str_dup(arg);
	ch->desc->pEdit	= cl;
	OLCED(ch)	= olced_lookup(ED_CLASS);
	touch_class(cl);
	act_char("Class created.", ch);
	return FALSE;
}

OLC_FUN(classed_edit)
{
	class_t *cl;

	if (PC(ch)->security < SECURITY_CLASS) {
		act_char("ClassEd: Insufficient security.", ch);
		return FALSE;
	}

	if (argument[0] == '\0')
		OLC_ERROR("'OLC EDIT'");

	if ((cl = class_search(argument)) == NULL) {
		act_puts("ClassEd: $t: No such class.",
			 ch, argument, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	ch->desc->pEdit	= cl;
	OLCED(ch)	= olced_lookup(ED_CLASS);
	return FALSE;
}

OLC_FUN(classed_save)
{
	bool found = FALSE;
	class_t *cl;

	olc_printf(ch, "Saved classes:");

	C_FOREACH(cl, &classes) {
		size_t i;
		FILE *fp;
		const char *filename;
		int *pvnum;
		pose_t *pose;

		if (!IS_SET(cl->class_flags, CLASS_CHANGED))
			continue;

		filename = strkey_filename(cl->name, CLASS_EXT);
		if ((fp = olc_fopen(CLASSES_PATH, filename, ch, -1)) == NULL)
			continue;

		REMOVE_BIT(cl->class_flags, CLASS_CHANGED);
		fprintf(fp, "#CLASS\n");
		fprintf(fp, "Name %s~\n", cl->name);
		fprintf(fp, "ShortName %s~\n", cl->who_name);
		fprintf(fp, "PrimeStat %s\n",
			flag_string(stat_aliases, cl->attr_prime));
		if (!IS_NULLSTR(cl->skill_spec))
			fprintf(fp, "SkillSpec '%s'\n", cl->skill_spec);
		fprintf(fp, "SchoolWeapon %d\n", cl->weapon);
		C_FOREACH(pvnum, &cl->guilds) {
			if (!*pvnum)
				continue;

			fprintf(fp, "GuildRoom %d\n", *pvnum);
		}
		fprintf(fp, "Thac0_00 %d\n", cl->thac0_00);
		fprintf(fp, "Thac0_32 %d\n", cl->thac0_32);
		fprintf(fp, "HPRate %d\n", cl->hp_rate);
		fprintf(fp, "LuckBonus %d\n", cl->luck_bonus);
		fprintf(fp, "ManaRate %d\n", cl->mana_rate);
		if (cl->class_flags) {
			fprintf(fp, "Flags %s~\n",
				flag_string(class_flags, cl->class_flags));
		}
		if (cl->points)
			fprintf(fp, "AddExp %d\n", cl->points);
		fprintf(fp, "StatMod");
		for (i = 0; i < MAX_STAT; i++)
			fprintf(fp, " %d", cl->mod_stat[i]);
		fprintf(fp, "\n");
		if (cl->restrict_align)
			fprintf(fp, "RestrictAlign %s~\n",
				flag_string(ralign_names, cl->restrict_align));
		fwrite_string(fp, "RestrictSex", cl->restrict_sex);
		if (cl->restrict_ethos)
			fprintf(fp, "RestrictEthos %s~\n",
				flag_string(ethos_table, cl->restrict_ethos));
		if (cl->death_limit != -1)
			fprintf(fp, "DeathLimit %d\n", cl->death_limit);
		fprintf(fp, "End\n\n");

		C_FOREACH(pose, &cl->poses) {
			if (IS_NULLSTR(pose->self) && IS_NULLSTR(pose->others))
				continue;

			fprintf(fp, "#POSE\n");
			fwrite_string(fp, "Self", pose->self);
			fwrite_string(fp, "Others", pose->others);
			fprintf(fp, "End\n\n");
		}

		fprintf(fp, "#$\n");
		fclose(fp);

		olc_printf(ch, "    %s (%s)", cl->name, filename);
		found = TRUE;
	}

	if (!found)
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
		else
			OLC_ERROR("'OLC ASHOW'");
	} else {
		if ((class = class_search(argument)) == NULL) {
			act_puts("ClassEd: $t: No such class.",
				 ch, argument, NULL,
				 TO_CHAR | ACT_NOTRANS, POS_DEAD);
			return FALSE;
		}
	}

	output = buf_new(0);
	buf_printf(output, BUF_END, "Name:           [%3s] [%s]\n",
		   class->who_name, class->name);
	buf_printf(output, BUF_END, "Primary attr:   [%s]\n",
		   flag_string(stat_aliases, class->attr_prime));
	if (!IS_NULLSTR(class->skill_spec))
		buf_printf(output, BUF_END, "SkillSpec:      [%s]\n", class->skill_spec);
	if (class->weapon)
		buf_printf(output, BUF_END, "School weapon:  [%d]\n", class->weapon);
	buf_printf(output, BUF_END, "THAC0 (level 0) [%d] THAC0 (level 32) [%d]\n",
		   class->thac0_00, class->thac0_32);
	buf_printf(output, BUF_END, "HP rate:        [%d%%] Mana rate:      [%d%%]\n",
		   class->hp_rate, class->mana_rate);
	if (class->class_flags) {
		buf_printf(output, BUF_END, "Class flags:    [%s]\n",
			   flag_string(class_flags, class->class_flags));
	}
	if (class->points) {
		buf_printf(output, BUF_END, "Exp points:     [%d]\n",
			   class->points);
	}

	buf_printf(output, BUF_END, "Luck bonus:     [%d]\n",
		   class->luck_bonus);

	for (i = 0, found = FALSE; i < MAX_STAT; i++) {
		if (class->mod_stat[i])
			found = TRUE;
	}
	if (found) {
		buf_append(output, "Stats mod:      [");
		for (i = 0; i < MAX_STAT; i++)
			buf_printf(output, BUF_END, "%s: %2d ",
				   flag_string(stat_aliases, i),
				   class->mod_stat[i]);
		buf_append(output, "]\n");
	}
	if (class->restrict_align)
		buf_printf(output, BUF_END, "Align restrict: [%s]\n",
			   flag_string(ralign_names, class->restrict_align));
	if (class->restrict_ethos)
		buf_printf(output, BUF_END, "Ethos restrict: [%s]\n",
			   flag_string(ethos_table, class->restrict_ethos));
	if (!IS_NULLSTR(class->restrict_sex))
		buf_printf(output, BUF_END, "Sex restrict:   [%s]\n",
			   class->restrict_sex);
	if (class->death_limit != -1)
		buf_printf(output, BUF_END, "Death limit:    [%d]\n", class->death_limit);

	page_to_char(buf_string(output), ch);
	buf_free(output);

	return FALSE;
}

OLC_FUN(classed_list)
{
	BUFFER *buffer = buf_new(0);
	c_strkey_dump(&classes, buffer);
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
		strlcpy(class->who_name, str, sizeof(class->who_name));
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

	return olced_flag(ch, argument, cmd, &class->attr_prime);
}

OLC_FUN(classed_weapon		)
{
	class_t *class;
	OBJ_INDEX_DATA *weap;
	EDIT_CLASS(ch, class);

	if ((weap = get_obj_index(atoi(argument))) == NULL) {
		act_char("ClassEd: Object doesn't exist.", ch);
		return FALSE;
	}
	if (weap->item_type != ITEM_WEAPON) {
		act_char("ClassEd: That obj is not a weapon.", ch);
		return FALSE;
	}
	return olced_number(ch, argument, cmd, &class->weapon);
}

OLC_FUN(classed_luckbonus	)
{
	class_t *class;
	EDIT_CLASS(ch, class);
	return olced_number(ch, argument, cmd, &class->luck_bonus);
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
	return olced_flag(ch, argument, cmd, &class->class_flags);
}

OLC_FUN(classed_stats)
{
	class_t *class;
        char arg[MAX_INPUT_LENGTH];
	char *endptr;
	int i, val;
	bool st = FALSE;

	EDIT_CLASS(ch, class);

	for (i = 0; i < MAX_STAT; i++) {
		argument = one_argument(argument, arg, sizeof(arg));
		if (*arg == '\0')
			break;
		val = strtol(arg, &endptr, 0);
		if (*arg == '\0' || *endptr != '\0')
			break;
		class->mod_stat[i] = val;
		st = TRUE;
	}

	if (!st) {
		act_puts("Syntax: $t <attr1> <attr2> ...",
			 ch, cmd->name, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
	} else
		act_char("Ok.", ch);
	return st;
}

OLC_FUN(classed_align		)
{
	class_t *class;
	EDIT_CLASS(ch, class);
	return olced_flag(ch, argument, cmd, &class->restrict_align);
}

OLC_FUN(classed_ethos		)
{
	class_t *class;
	EDIT_CLASS(ch, class);
	return olced_flag(ch, argument, cmd, &class->restrict_ethos);
}

OLC_FUN(classed_sex		)
{
	class_t *class;
	EDIT_CLASS(ch, class);
	return olced_name(ch, argument, cmd, &class->restrict_sex);
}

OLC_FUN(classed_poses)
{
	class_t *class;
	char arg[MAX_INPUT_LENGTH];
	pose_t *pose;

	EDIT_CLASS(ch, class);

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0' || !str_prefix(arg, "list")) {
		BUFFER	*buffer;
		bool st = FALSE;

		buffer = buf_new(0);

		C_FOREACH(pose, &class->poses) {
			if (IS_NULLSTR(pose->self) && IS_NULLSTR(pose->others))
				continue;
			st = TRUE;
			buf_printf(buffer, BUF_END,
			    "[%3d] Self:   %s\n      Others: %s\n",
			    varr_index(&class->poses, pose),
			    pose->self, pose->others);
		}

		if (!st) {
			buf_append(buffer,
			    "No poses have been defined for this class.\n");
		}

		page_to_char(buf_string(buffer), ch);
		buf_free(buffer);
		return FALSE;
	}

	if (!str_prefix(arg, "delete")) {
		argument = one_argument(argument, arg, sizeof(arg));
		pose = varr_get(&class->poses, atoi(arg));
		if (pose == NULL) {
			act_puts("ClassEd: no such pose: $t",
				 ch, arg, NULL,
				 TO_CHAR | ACT_NOTRANS, POS_DEAD);
			return FALSE;
		}

		varr_edelete(&class->poses, pose);
		act_char("Pose deleted.", ch);
		return TRUE;
	}

	if (!str_prefix(arg, "self")) {
		argument = one_argument(argument, arg, sizeof(arg));
		pose = varr_get(&class->poses, atoi(arg));
		if (pose == NULL) {
			act_puts("ClassEd: no such pose: $t",
				 ch, arg, NULL,
				 TO_CHAR | ACT_NOTRANS, POS_DEAD);
			return FALSE;
		}

		free_string(pose->self);
		pose->self = str_dup(argument);
		act_char("Ok.", ch);
		return TRUE;
	}

	if (!str_prefix(arg, "others")) {
		argument = one_argument(argument, arg, sizeof(arg));
		pose = varr_get(&class->poses, atoi(arg));
		if (pose == NULL) {
			act_puts("ClassEd: no such pose: $t",
				 ch, arg, NULL,
				 TO_CHAR | ACT_NOTRANS, POS_DEAD);
			return FALSE;
		}

		free_string(pose->others);
		pose->others = str_dup(argument);
		act_char("Ok.", ch);
		return TRUE;
	}

	if (!str_prefix(arg, "add")) {
		pose = varr_enew(&class->poses);
		pose->self = str_dup("#");
		pose->others = str_dup("#");
		act_char("Pose added.", ch);
		return TRUE;
	}

	if (!str_prefix(arg, "sort")) {
		varr_qsort(&class->poses, cmpstr);
		act_char("Ok.", ch);
		return TRUE;
	}

	OLC_ERROR("'OLC CLASS POSES'");
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
		BUFFER	*buffer;
		bool st = FALSE;
		int *pvnum;

		buffer = buf_new(0);

		C_FOREACH(pvnum, &class->guilds) {
			if (!*pvnum)
				continue;

			if ((room = get_room_index(*pvnum)) == NULL) {
				buf_printf(buffer, BUF_END,
				    "[%5d] Nonexistent.\n", *pvnum);
				continue;
			}

			st = TRUE;
			buf_printf(buffer, BUF_END, "[%5d] %-25.24s\n",
				   *pvnum, mlstr_mval(&room->name));
		}

		if (!st) {
			buf_append(buffer,
			    "No guild rooms have been defined for this class.\n");
		}

		page_to_char(buf_string(buffer), ch);
		buf_free(buffer);
		return FALSE;
	}

	if (!str_prefix(arg, "add")) {
		int *pvnum;

		argument = one_argument(argument, arg, sizeof(arg));
		vnum = atoi(arg);
		if ((room = get_room_index(vnum)) == NULL) {
			act_puts("ClassEd: $j: no such room.",
				 ch, (const void *) vnum, NULL,
				 TO_CHAR, POS_DEAD);
			return FALSE;
		}

		if (varr_bsearch(&class->guilds, &vnum, cmpint)) {
			act_puts("ClassEd: $j: already in list.",
				 ch, (const void *) vnum, NULL,
				 TO_CHAR, POS_DEAD);
			return FALSE;
		}

		pvnum = varr_enew(&class->guilds);
		*pvnum = vnum;
		varr_qsort(&class->guilds, cmpint);
		act_char("Guild added.", ch);
		return TRUE;
	}

	if (!str_prefix(arg, "delete")) {
		int *pvnum;

		argument = one_argument(argument, arg, sizeof(arg));
		vnum = atoi(arg);
		pvnum = varr_bsearch(&class->guilds, &vnum, cmpint);
		if (pvnum == NULL) {
			act_puts("ClassEd: $j: not in list.",
				 ch, (const void *) vnum, NULL,
				 TO_CHAR, POS_DEAD);
			return FALSE;
		}

		*pvnum = 0;
		varr_qsort(&class->guilds, cmpint);
		act_char("Guild deleted.", ch);
		return TRUE;
	}

	OLC_ERROR("'OLC CLASS GUILDS'");
}

/*--------------------------------------------------------------------
 * static functions
 */

static bool
touch_class(class_t *class)
{
	SET_BIT(class->class_flags, CLASS_CHANGED);
	return FALSE;
}

static
VALIDATE_FUN(validate_whoname)
{
	class_t *cl;
	class_t *cl2;

	EDIT_CLASS(ch, cl);

	if (strlen(arg) > 3 || strlen(arg) < 1) {
		act_char("ClassEd: whoname should be 1..3 symbols long.", ch);
		return FALSE;
	}

	C_FOREACH(cl2, &classes) {
		if (!str_cmp(cl2->who_name, arg)
		&&  cl2 != cl) {
			act_puts("ClassEd: $t: duplicate class whoname.",
				 ch, arg, NULL,
				 TO_CHAR | ACT_NOTRANS, POS_DEAD);
			return FALSE;
		}
	}

	return TRUE;
}
