/*
 * $Id: db_class.c,v 1.2 1998-09-15 02:51:38 fjoe Exp $
 */

#include <stdio.h>

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

void init_classes(void)
{
	classes = varr_new(sizeof(CLASS_DATA), 4);
}

DBINIT_FUN(init_class)
{
	class_curr = NULL;
}

DBLOAD_FUN(load_class)
{
	int i;

	class_curr = varr_enew(classes);
	class_curr->file_name = str_dup(filename);
	class_curr->skills = varr_new(sizeof(CLASS_SKILL), 8);
	class_curr->guild = varr_new(sizeof(int), 4);
	class_curr->poses = varr_new(sizeof(POSE_DATA), 4);

	for (i = 0; i < MAX_LEVEL+1; i++)
		class_curr->titles[i][0] = class_curr->titles[i][1] = str_empty;

	for (;;) {
		char *word = feof(fp) ? "End" : fread_word(fp);
		bool fMatch = FALSE;

		switch(UPPER(word[0])) {
		case 'A':
			KEY("AddExp", class_curr->points, fread_number(fp));
			break;

		case 'E':
			if (!str_cmp(word, "End")) {
				if (IS_NULLSTR(class_curr->name)) {
					db_error("load_class",
						 "class name undefined");
					varr_free(class_curr->skills);
					classes->nused--;
				}
				varr_qsort(class_curr->skills, cmpint);
				return;
			}

		case 'F':
			SKEY("Filename", class_curr->file_name);
			KEY("Flags", class_curr->flags,
			    fread_fstring(class_flags, fp));
			break;

		case 'G':
			if (!str_cmp(word, "GuildRoom")) {
				int vnum = fread_number(fp);
				int *pvnum = varr_enew(class_curr->guild);
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
			KEY("RestrictAlign", class_curr->align,
			    fread_fstring(align_names, fp));
			break;

		case 'S':
			if (!str_cmp(word, "ShortName")) {
				char *p = fread_string(fp);
				strnzcpy(class_curr->who_name, p,
					 sizeof(class_curr->who_name));
				free_string(p);
				fMatch = TRUE;
			}
			KEY("SchoolWeapon", class_curr->weapon, fread_number(fp));
			if (!str_cmp(word, "Skill")) {
				CLASS_SKILL *class_skill;

				class_skill = varr_enew(class_curr->skills);
				class_skill->sn = sn_lookup(fread_word(fp));
				class_skill->level = fread_number(fp);
				class_skill->rating = fread_number(fp);
				fMatch = TRUE;
			}
			KEY("SkillAdept", class_curr->skill_adept, fread_number(fp));
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
				class_curr->titles[level][sex-1] = fread_string(fp);
			}
			break;
		}
	}
}

DBLOAD_FUN(load_pose)
{
	POSE_DATA *pose;

	if (class_curr == NULL) {
		db_error("load_pose", "No #CLASS seen yet");
		return;
	}

	pose = varr_enew(class_curr->poses);
	pose->self = mlstr_fread(fp);
	pose->others = mlstr_fread(fp);
}
