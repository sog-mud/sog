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
 * $Id: db_class.c,v 1.21 1999-10-06 09:56:15 fjoe Exp $
 */

#include <stdio.h>
#include <string.h>

#include "merc.h"
#include "db.h"

DECLARE_DBLOAD_FUN(load_class);
DECLARE_DBLOAD_FUN(load_pose);

DECLARE_DBINIT_FUN(init_class);

DBFUN dbfun_classes[] =
{
	{ "CLASS",	load_class	},
	{ "POSE",	load_pose	},
	{ NULL }
};

DBDATA db_classes = { dbfun_classes, init_class };

DBINIT_FUN(init_class)
{
	if (DBDATA_VALID(dbdata))
		db_set_arg(dbdata, "POSE", NULL);
}

DBLOAD_FUN(load_class)
{
	int i;
	class_t *class = class_new();
	class->file_name = get_filename(filename);
	db_set_arg(dbdata, "POSE", class);

	for (i = 0; i < MAX_LEVEL+1; i++)
		class->titles[i][0] = class->titles[i][1] = str_empty;

	for (;;) {
		char *word = feof(fp) ? "End" : fread_word(fp);
		bool fMatch = FALSE;

		switch(UPPER(word[0])) {
		case 'A':
			KEY("AddExp", class->points, fread_number(fp));
			break;
		case 'D':
			KEY("DeathLimit", class->death_limit,
			    fread_number(fp));
			break;
		case 'E':
			if (!str_cmp(word, "End")) {
				if (IS_NULLSTR(class->name)) {
					db_error("load_class",
						 "class name undefined");
					class_free(class);
					classes.nused--;
				}
				varr_qsort(&class->guilds, cmpint);
				return;
			}
		case 'F':
			KEY("Flags", class->class_flags,
			    fread_fstring(class_flags, fp));
			break;
		case 'G':
			if (!str_cmp(word, "GuildRoom")) {
				int vnum = fread_number(fp);
				int *pvnum = varr_enew(&class->guilds);
				*pvnum = vnum;
				fMatch = TRUE;
			}
			break;
		case 'H':
			KEY("HPRate", class->hp_rate, fread_number(fp));
			break;
		case 'M':
			KEY("ManaRate", class->mana_rate, fread_number(fp));
			break;
		case 'N':
			SKEY("Name", class->name, fread_string(fp));
			break;
		case 'P':
			KEY("PrimeStat", class->attr_prime,
			    fread_fword(stat_names, fp));
			break;
		case 'R':
			KEY("RestrictAlign", class->restrict_align,
			    fread_fstring(ralign_names, fp));
			KEY("RestrictSex", class->restrict_sex,
			    fread_fstring(sex_table, fp));
			KEY("RestrictEthos", class->restrict_ethos,
			    fread_fstring(ethos_table, fp));
			break;
		case 'S':
			KEY("SchoolWeapon", class->weapon,
			    fread_number(fp));
			SKEY("SkillSpec", class->skill_spec,
			     fread_name(fp, &specs, "load_class"));
			if (!str_cmp(word, "ShortName")) {
				const char *p = fread_string(fp);
				strnzcpy(class->who_name,
					 sizeof(class->who_name), p);
				free_string(p);
				fMatch = TRUE;
			}
			if (!str_cmp(word, "StatMod")) {
				for (i = 0; i < MAX_STATS; i++)
					class->stats[i] = fread_number(fp);
				fMatch = TRUE;
			}
			break;
		case 'T':
			KEY("Thac0_00", class->thac0_00, fread_number(fp));
			KEY("Thac0_32", class->thac0_32, fread_number(fp));
			if (!str_cmp(word, "Title")) {
				int level;
				int sex;

				level = fread_number(fp);
				if (level < 0 || level > MAX_LEVEL) {
					db_error("load_class",
						 "invalid level %d", level);
					continue;
				}
				sex = fread_fword(sex_table, fp);
				if (sex != SEX_MALE && sex != SEX_FEMALE) {
					db_error("load_class", "invalid sex");
					continue;
				}
				class->titles[level][sex-1] =
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
	class_t *class = arg;
	pose_t *pose;

	if (!class) {
		db_error("load_pose", "No #CLASS seen yet");
		return;
	}

	pose = varr_enew(&class->poses);
	for (;;) {
		char *word = feof(fp) ? "End" : fread_word(fp);
		bool fMatch = FALSE;

		switch(UPPER(word[0])) {
		case 'E':
			if (!str_cmp(word, "End"))
				return;
			break;
		case 'O':
			SKEY("Others", pose->others, fread_string(fp));
			break;
		case 'S':
			SKEY("Self", pose->self, fread_string(fp));
			break;
		}

		if (!fMatch) 
			db_error("load_pose", "%s: Unknown keyword", word);
	}
}
