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
 * $Id: olc_class.c,v 1.3 1999-09-24 04:16:05 avn Exp $
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

DECLARE_OLC_FUN(classed_name		);
DECLARE_OLC_FUN(classed_filename	);
DECLARE_OLC_FUN(classed_whoname		);
DECLARE_OLC_FUN(classed_titles		);
DECLARE_OLC_FUN(classed_primary		);
DECLARE_OLC_FUN(classed_weapon		);
DECLARE_OLC_FUN(classed_adept		);
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
DECLARE_OLC_FUN(classed_skills		);
DECLARE_OLC_FUN(classed_guilds		);

DECLARE_OLC_FUN(olc_skill_update	);

static DECLARE_VALIDATE_FUN(validate_name);
static DECLARE_VALIDATE_FUN(validate_whoname);


olc_cmd_t olc_cmds_class[] =
{
	{ "create",	classed_create					},
	{ "edit",	classed_edit					},
	{ "",		classed_save					},
	{ "touch",	classed_touch					},
	{ "show",	classed_show					},
	{ "list",	classed_list					},

	{ "name",	classed_name,		validate_name		},
	{ "filename",	classed_filename,	validate_filename	},
	{ "whoname",	classed_whoname,	validate_whoname	},
	{ "primary",	classed_primary,	stat_names		},
	{ "weapon",	classed_weapon					},
	{ "adept",	classed_adept					},
	{ "thac00",	classed_thac00					},
	{ "thac32",	classed_thac32					},
	{ "hprate",	classed_hprate					},
	{ "manarate",	classed_manarate				},
	{ "points",	classed_points					},
	{ "deaths",	classed_deaths					},
	{ "flags",	classed_flags,		class_flags		},
	{ "align",	classed_align,		ralign_names		},
	{ "ethos",	classed_ethos,		ethos_table		},
	{ "sex",	classed_sex,		sex_table		},
	{ "stats",	classed_stats					},
	{ "titles",	classed_titles					},
	{ "poses",	classed_poses					},
	{ "skills",	classed_skills					},
	{ "guilds",	classed_guilds					},

	{ "update",	olc_skill_update				},
	{ "commands",	show_commands					},
	{ NULL }
};

static void save_class(CHAR_DATA *ch, class_t *class);

OLC_FUN(classed_create)
{
	int cn;
	class_t *class;
	char arg[MAX_STRING_LENGTH];

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

	if ((cn = cn_lookup(arg)) >= 0) {
		char_printf(ch, "ClassEd: %s: already exists.\n",
			    CLASS(cn)->name);
		return FALSE;
	}

	class		= class_new();
	class->name	= str_dup(arg);
	class->file_name= str_printf("class%02d.class", classes.nused-1);

	ch->desc->pEdit	= (void *)class;
	OLCED(ch)	= olced_lookup(ED_CLASS);
	touch_class(class);
	char_puts("Class created.\n",ch);
	return FALSE;
}

OLC_FUN(classed_edit)
{
	int cn;

	if (PC(ch)->security < SECURITY_CLASS) {
		char_puts("ClassEd: Insufficient security.\n", ch);
		return FALSE;
	}

	if (argument[0] == '\0') {
		dofun("help", ch, "'OLC EDIT'");
		return FALSE;
	}

	if ((cn = cn_lookup(argument)) < 0) {
		char_printf(ch, "ClassEd: %s: No such class.\n", argument);
		return FALSE;
	}

	ch->desc->pEdit	= CLASS(cn);
	OLCED(ch)	= olced_lookup(ED_CLASS);
	return FALSE;
}

OLC_FUN(classed_save)
{
	int i;
	FILE *fp;
	bool found = FALSE;

	fp = olc_fopen(CLASSES_PATH, CLASS_LIST, ch, SECURITY_CLASS);
	if (fp == NULL)
		return FALSE;

	olc_printf(ch, "Saved classes:");

	for (i = 0; i < classes.nused; i++) {
		fprintf(fp, "%s\n", CLASS(i)->file_name);
		if (IS_SET(CLASS(i)->class_flags, RACE_CHANGED)) {
			save_class(ch, CLASS(i));
			found = TRUE;
		}
	}

	fprintf(fp, "$\n");
	fclose(fp);

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
		else {
			dofun("help", ch, "'OLC ASHOW'");
			return FALSE;
		}
	}
	else {
		if ((i = cn_lookup(argument)) < 0) {
			char_printf(ch, "ClassEd: %s: No such class.\n", argument);
			return FALSE;
		}
		class = CLASS(i);
	}

	output = buf_new(-1);
	buf_printf(output, "Name:           [%3s] [%s]\n",
		   class->who_name, class->name);
	buf_printf(output, "Filename:       [%s]\n", class->file_name);
	buf_printf(output, "Primary attr:   [%s]\n",
		   flag_string(stat_names, class->attr_prime));
	if (class->weapon)
		buf_printf(output, "School weapon:  [%d]\n", class->weapon);
	if (class->skill_adept != 75)
		buf_printf(output, "Skill adept     [%d%%]\n",
			   class->skill_adept);
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
	int i;
	BUFFER	*buffer;

	buffer = buf_new(-1);
	for (i = 0; i < classes.nused; i++) {
		buf_printf(buffer, "[%2d] %s\n", i, CLASS(i)->name);
	}
	page_to_char(buf_string(buffer), ch);
	buf_free(buffer);
	return FALSE;
}

OLC_FUN(classed_name)
{
	class_t *class;
	EDIT_CLASS(ch, class);
	return olced_str(ch, argument, cmd, &class->name);
}

OLC_FUN(classed_filename)
{
	class_t *class;
	EDIT_CLASS(ch, class);
	return olced_str(ch, argument, cmd, &class->file_name);
}

OLC_FUN(classed_whoname)
{
	int i;
	const char *str;
	class_t *class;
	EDIT_CLASS(ch, class);

	str = str_dup(class->who_name);
	if (olced_str(ch, argument, cmd, &str)) {
		for (i = 0; i < strlen(str); i++)
			class->who_name[i] = str[i];
		class->who_name[i] = '\0';
		free_string(str);
		return TRUE;
		}
	free_string(str);
	return FALSE;
}

OLC_FUN(classed_titles)
{
	class_t *class;
	char arg[MAX_STRING_LENGTH];
	int lev;
	char *endptr;
	const flag_t *sex;

	EDIT_CLASS(ch, class);

	if (argument[0] == '\0') {
		int i;
		BUFFER	*buffer;

		buffer = buf_new(-1);
		for (i = 0; i < MAX_LEVEL + 1; i++)
			buf_printf(buffer, "[%3d] %-30.29s %-30.29s\n",
				   i, class->titles[i][0], class->titles[i][1]);
		page_to_char(buf_string(buffer), ch);
		buf_free(buffer);
		return FALSE;
	}
	argument = one_argument(argument, arg, sizeof(arg));
	if (*arg == '\0') {
		char_puts("Syntax: titles level <male|female> title\n", ch);
		return FALSE;
	}
	lev = strtol(arg, &endptr, 0);
	if (*arg == '\0' || *endptr != '\0' || lev < 0 || lev > MAX_LEVEL) {
		char_puts("Syntax: titles level <male|female> title\n", ch);
		return FALSE;
	}
	argument = one_argument(argument, arg, sizeof(arg));
	if ((sex = flag_lookup(sex_table, arg)) == NULL
	|| (sex->bit != SEX_MALE && sex->bit != SEX_FEMALE)) {
		char_puts("Syntax: titles level <male|female> title\n", ch);
		return FALSE;
	}
	if (class->titles[lev][sex->bit - SEX_MALE])
		free_string(class->titles[lev][sex->bit - SEX_MALE]);
	class->titles[lev][sex->bit - SEX_MALE] = str_dup(argument);
	char_puts("Ok.\n", ch);
	return TRUE;
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

OLC_FUN(classed_adept		)
{
	class_t *class;
	EDIT_CLASS(ch, class);

	return olced_number(ch, argument, cmd, &class->skill_adept);
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
        char arg[MAX_STRING_LENGTH];
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
	&& class->restrict_sex != SEX_FEMALE)
		class->restrict_sex = -1;
	return TRUE;
}

OLC_FUN(classed_poses)
{
	class_t *class;
	char arg[MAX_STRING_LENGTH];
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
		pose = VARR_GET(&class->poses, atoi(arg));
		pose->self = str_dup(str_empty);
		pose->others = str_dup(str_empty);
		varr_qsort(&class->poses, cmpstr);
		char_puts("Pose deleted.\n", ch);
		return TRUE;
	}
	if (!str_prefix(arg, "self")) {
		argument = one_argument(argument, arg, sizeof(arg));
		pose = VARR_GET(&class->poses, atoi(arg));
		free_string(pose->self);
		pose->self = str_dup(argument);
		char_puts("Ok.\n", ch);
		return TRUE;
	}
	if (!str_prefix(arg, "others")) {
		argument = one_argument(argument, arg, sizeof(arg));
		pose = VARR_GET(&class->poses, atoi(arg));
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

OLC_FUN(classed_skills)
{
	class_t *class;
	cskill_t *csk;
	skill_t *sk;
	char arg[MAX_STRING_LENGTH];

	EDIT_CLASS(ch, class);

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0' || !str_prefix(arg, "list")) {
		int i;
		BUFFER	*buffer;
		bool st = FALSE;

		buffer = buf_new(-1);
		for (i = 0; i < class->skills.nused; i++) {
			csk = VARR_GET(&class->skills, i);
			if (csk->sn <= 0 || (sk = skill_lookup(csk->sn)) == NULL)
				continue;
			st = TRUE;
			buf_printf(buffer, "Skill: %-16.15s (level %3d, "
					   "rating %2d, modifier %3d)\n",
				   sk->name, csk->level, csk->rating, csk->mod);
		}
		if (!st)
			buf_add(buffer, "No skills have been defined"
					" for this class.\n");
		page_to_char(buf_string(buffer), ch);
		buf_free(buffer);
		return FALSE;
	}
	if (!str_prefix(arg, "add")) {
		int sn;

		argument = one_argument(argument, arg, sizeof(arg));
		if ((sn = sn_lookup(arg)) == -1) {
			char_printf(ch, "ClassEd: %s: no such skill.\n", arg);
			return FALSE;
		}
		if ((csk = cskill_lookup(class, sn)) != NULL) {
			char_printf(ch, "ClassEd: %s: already in list.\n",
				    SKILL(sn)->name);
			return FALSE;
		}
		csk = varr_enew(&class->skills);
		csk->sn = sn;
		argument = one_argument(argument, arg, sizeof(arg));
		csk->level = atoi(arg);
		if (!csk->level) csk->level = 1;
		argument = one_argument(argument, arg, sizeof(arg));
		csk->rating = atoi(arg);
		if (!csk->rating) csk->rating = 1;
		argument = one_argument(argument, arg, sizeof(arg));
		csk->mod = atoi(arg);
		varr_qsort(&class->skills, cmpint);
		char_puts("Skill added.\n", ch);
		return TRUE;
	}
	if (!str_prefix(arg, "delete")) {
		int sn;

		argument = one_argument(argument, arg, sizeof(arg));
		if ((sn = sn_lookup(arg)) == -1) {
			char_printf(ch, "ClassEd: %s: no such skill.\n", arg);
			return FALSE;
		}
		if ((csk = cskill_lookup(class, sn)) == NULL) {
			char_printf(ch, "ClassEd: %s: not in list.\n",
				    SKILL(sn)->name);
			return FALSE;
		}
		csk->sn = 0;
		varr_qsort(&class->skills, cmpint);
		char_puts("Skill deleted.\n", ch);
		return TRUE;
	}
	dofun("help", ch, "'OLC CLASS SKILLS'");
	return FALSE;
}

OLC_FUN(classed_guilds)
{
	class_t *class;
	char arg[MAX_STRING_LENGTH];
	int vnum;
	ROOM_INDEX_DATA *room;

	EDIT_CLASS(ch, class);

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0' || !str_prefix(arg, "list")) {
		int i;
		BUFFER	*buffer;
		bool st = FALSE;

		buffer = buf_new(-1);
		for (i = 0; i < class->guild.nused; i++) {
			vnum = *(int*)VARR_GET(&class->guild, i);
			if (!vnum) continue;
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
		int i;
		int *pvnum;

		argument = one_argument(argument, arg, sizeof(arg));
		vnum = atoi(arg);
		if ((room = get_room_index(vnum)) == NULL) {
			char_printf(ch, "ClassEd: %d: no such room.\n", vnum);
			return FALSE;
		}
		for (i = 0; i < class->guild.nused; i++)
			if (vnum == *(int*)VARR_GET(&class->guild, i)) {
			char_printf(ch, "ClassEd: %d: already in list.\n",
				    vnum);
			return FALSE;
			}
		pvnum = varr_enew(&class->guild);
		*pvnum = vnum;
		varr_qsort(&class->guild, cmpint);
		char_puts("Guild added.\n", ch);
		return TRUE;
	}
	if (!str_prefix(arg, "delete")) {
		int i;

		argument = one_argument(argument, arg, sizeof(arg));
		vnum = atoi(arg);
		for (i = 0; i < class->guild.nused; i++) {
			if (vnum != *(int*)VARR_GET(&class->guild, i))
				continue;
			*(int*)VARR_GET(&class->guild, i) = 0;
			varr_qsort(&class->guild, cmpint);
			char_puts("Guild deleted.\n", ch);
			return TRUE;
		}
		char_printf(ch, "ClassEd: %d: not in list.\n", vnum);
		return FALSE;
	}
	dofun("help", ch, "'OLC CLASS SKILLS'");
	return FALSE;
}

bool touch_class(class_t *class)
{
	SET_BIT(class->class_flags, CLASS_CHANGED);
	return FALSE;
}

static VALIDATE_FUN(validate_name)
{
	int i;
	class_t *class;
	EDIT_CLASS(ch, class);

	for (i = 0; i < classes.nused; i++)
		if (CLASS(i) != class
		&&  !str_cmp(CLASS(i)->name, arg)) {
			char_printf(ch, "ClassEd: %s: duplicate class name.\n",
				    arg);
			return FALSE;
		}

	return TRUE;
}

static VALIDATE_FUN(validate_whoname)
{
	int i;
	class_t *class;
	EDIT_CLASS(ch, class);

	if (strlen(arg) > 3 || strlen(arg) < 1) {
		char_puts("ClassEd: whoname should be 1..3 symbols long.\n", ch);
		return FALSE;
	}
	for (i = 0; i < classes.nused; i++)
		if (CLASS(i) != class
		&&  !str_cmp(CLASS(i)->who_name, arg)) {
			char_printf(ch, "ClassEd: %s: duplicate class whoname.\n",
				    arg);
			return FALSE;
		}
	return TRUE;
}

#define PROC_STR(s)	((s) ? (s) : (str_empty))
static void save_class(CHAR_DATA *ch, class_t *class)
{
	int i;
	FILE *fp;

	if ((fp = olc_fopen(CLASSES_PATH, class->file_name, ch, -1)) == NULL)
		return;

	REMOVE_BIT(class->class_flags, RACE_CHANGED);
	fprintf(fp, "#CLASS\n");
	fprintf(fp, "Name %s~\n", class->name);
	fprintf(fp, "ShortName %s~\n", class->who_name);
	fprintf(fp, "PrimeStat %s\n", flag_string(stat_names, class->attr_prime));
	fprintf(fp, "SchoolWeapon %d\n", class->weapon);
	for (i = 0; i < class->guild.nused; i++)
		if (*(int*)VARR_GET(&class->guild, i) != 0)
			fprintf(fp, "GuildRoom %d\n", *(int*)VARR_GET(&class->guild, i));
	fprintf(fp, "SkillAdept %d\n", class->skill_adept);
	fprintf(fp, "Thac0_00 %d\n", class->thac0_00);
	fprintf(fp, "Thac0_32 %d\n", class->thac0_32);
	fprintf(fp, "HPRate %d\n", class->hp_rate);
	fprintf(fp, "ManaRate %d\n", class->mana_rate);
	if (class->class_flags)
		fprintf(fp, "Flags %s~\n",
			flag_string(class_flags, class->class_flags));
	if (class->points)
		fprintf(fp, "AddExp %d\n", class->points);
	fprintf(fp, "StatMod");
	for (i = 0; i < MAX_STATS; i++)
		fprintf(fp, " %d",
			   class->stats[i]);
	fprintf(fp, "\n");
	if (class->restrict_align)
		fprintf(fp, "RestrictAlign %s~\n",
			flag_string(ralign_names, class->restrict_align));
	if (class->restrict_sex != -1)
		fprintf(fp, "RestrictSex %s~\n",
			flag_string(sex_table, class->restrict_sex));
	if (class->restrict_ethos)
		fprintf(fp, "RestrictEthos %s~\n",
			flag_string(ethos_table, class->restrict_ethos));
	if (class->death_limit != -1)
		fprintf(fp, "DeathLimit %d\n", class->death_limit);
	for (i = 0; i < class->skills.nused; i++) {
		cskill_t *csk = VARR_GET(&class->skills, i);
		skill_t *sk;

		if (!csk->sn) continue;
		if ((sk = skill_lookup(csk->sn)) == NULL) continue;
		fprintf(fp, "Skill '%s' %d %d %d\n", skill_name(csk->sn),
			csk->level, csk->rating, csk->mod);
	}
	for (i = 0; i < MAX_LEVEL + 1; i++) {
		fprintf(fp, "Title %d male %s~\n",
			i, PROC_STR(class->titles[i][0]));
		fprintf(fp, "Title %d female %s~\n",
			i, PROC_STR(class->titles[i][1]));
	}
	fprintf(fp, "End\n\n");

	for (i = 0; i < class->poses.nused; i++) {
		pose_t *pose = VARR_GET(&class->poses, i);
		if (IS_NULLSTR(pose->self) && IS_NULLSTR(pose->others))
			continue;
		fprintf(fp, "#POSE\n");
		fprintf(fp, "Self %s~\n", pose->self);
		fprintf(fp, "Others %s~\n", pose->others);
		fprintf(fp, "End\n\n");
	}

	fprintf(fp, "#$\n");
	fclose(fp);
	olc_printf(ch, "    %s (%s)", class->name, class->file_name);
}
