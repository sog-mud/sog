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
 * $Id: db_class.c,v 1.11 1999-02-09 10:19:14 fjoe Exp $
 */

#include <stdio.h>
#include <string.h>

#include "merc.h"
#include "db.h"

DECLARE_DBLOAD_FUN(load_class);
DECLARE_DBLOAD_FUN(load_pose);

DBFUN db_load_classes[] =
{
	{ "CLASS",	load_class	},
	{ "POSE",	load_pose	},
	{ NULL }
};

static CLASS_DATA *class_curr;

DBINIT_FUN(init_class)
{
	class_curr = NULL;
}

DBLOAD_FUN(load_class)
{
	int	i;

	class_curr = class_new();
	class_curr->file_name = get_filename(filename);

	for (i = 0; i < MAX_LEVEL+1; i++)
		class_curr->titles[i][0] = class_curr->titles[i][1] = str_empty;

	for (;;) {
		char *word = feof(fp) ? "End" : fread_word(fp);
		bool fMatch = FALSE;

		switch(UPPER(word[0])) {
		case 'A':
			KEY("AddExp", class_curr->points, fread_number(fp));
			break;

		case 'D':
			KEY("DeathLimit", class_curr->death_limit,
			    fread_number(fp));
			break;

		case 'E':
			if (!str_cmp(word, "End")) {
				if (IS_NULLSTR(class_curr->name)) {
					db_error("load_class",
						 "class name undefined");
					class_free(class_curr);
					classes.nused--;
				}
				varr_qsort(&class_curr->skills, cmpint);
				return;
			}

		case 'F':
			KEY("Flags", class_curr->flags,
			    fread_fstring(class_flags, fp));
			break;

		case 'G':
			if (!str_cmp(word, "GuildRoom")) {
				int vnum = fread_number(fp);
				int *pvnum = varr_enew(&class_curr->guild);
				*pvnum = vnum;
				fMatch = TRUE;
			}
			break;

		case 'H':
			KEY("HPRate", class_curr->hp_rate, fread_number(fp));
			break;

		case 'M':
			KEY("ManaRate", class_curr->mana_rate, fread_number(fp));
			break;

		case 'N':
			SKEY("Name", class_curr->name);
			break;

		case 'P':
			KEY("PrimeStat", class_curr->attr_prime,
			    fread_fword(stat_names, fp));
			break;

		case 'R':
			KEY("RestrictAlign", class_curr->restrict_align,
			    fread_fword(align_names, fp));
			KEY("RestrictSex", class_curr->restrict_sex,
			    fread_fword(sex_table, fp));
			SKEY("RestrictHometown", class_curr->restrict_hometown);
			break;

		case 'S':
			KEY("SkillAdept", class_curr->skill_adept,
			    fread_number(fp));
			KEY("SchoolWeapon", class_curr->weapon,
			    fread_number(fp));
			if (!str_cmp(word, "ShortName")) {
				const char *p = fread_string(fp);
				strnzcpy(class_curr->who_name, p,
					 sizeof(class_curr->who_name));
				free_string(p);
				fMatch = TRUE;
			}
			if (!str_cmp(word, "Skill")) {
				CLASS_SKILL *class_skill;

				class_skill = varr_enew(&class_curr->skills);
				class_skill->sn = sn_lookup(fread_word(fp));
				class_skill->level = fread_number(fp);
				class_skill->rating = fread_number(fp);
				fMatch = TRUE;
			}
			if (!str_cmp(word, "StatMod")) {
				for (i = 0; i < MAX_STATS; i++)
					class_curr->stats[i] = fread_number(fp);
				fMatch = TRUE;
			}
			break;

		case 'T':
			KEY("Thac0_00", class_curr->thac0_00, fread_number(fp));
			KEY("Thac0_32", class_curr->thac0_32, fread_number(fp));
			if (!str_cmp(word, "Title")) {
				int level;
				int sex;

				level = fread_number(fp);
				if (level < 0 || level > MAX_LEVEL)
					db_error("load_class",
						 "invalid level %d", level);
				sex = fread_fword(sex_table, fp);
				if (sex < 1 || sex > 2)
					db_error("load_class", "invalid sex");
				class_curr->titles[level][sex-1] =
							fread_string(fp);
				fMatch = TRUE;
			}
			break;
		}

		if (!fMatch) 
			db_error("load_class", "%s: Unknown keyword", word);
	}
}

DBLOAD_FUN(load_pose)
{
	POSE_DATA *pose;

	if (class_curr == NULL) {
		db_error("load_pose", "No #CLASS seen yet");
		return;
	}

	pose = varr_enew(&class_curr->poses);
	pose->self = mlstr_fread(fp);
	pose->others = mlstr_fread(fp);
}
