/*
 * $Id: db_class.c,v 1.1 1998-09-01 18:37:57 fjoe Exp $
 */

#include <stdio.h>

#include "merc.h"
#include "db.h"

DECLARE_DBLOAD_FUN(load_class);

DBFUN db_load_classes[] =
{
	{ "CLASS",	load_class	},
	{ NULL }
};

DBINIT(init_classes)
{
	classes = varr_new(sizeof(CLASS_DATA), 4);
}

void load_class(FILE *fp)
{
	int i;
	CLASS_DATA *class;

	class = varr_enew(classes);
	class->file_name = str_dup(filename);
	class->skills = varr_new(sizeof(CLASS_SKILL), 8);
	class->guild = varr_new(sizeof(int), 4);
	for (i = 0; i < MAX_LEVEL+1; i++)
		class->titles[i][0] = class->titles[i][1] = str_empty;

	for (;;) {
		char *word = feof(fp) ? "End" : fread_word(fp);
		bool fMatch = FALSE;

		switch(UPPER(word[0])) {
		case 'A':
			KEY("AddExp", class->points, fread_number(fp));
			break;

		case 'E':
			if (!str_cmp(word, "End")) {
				if (IS_NULLSTR(class->name)) {
					db_error("load_class",
						 "class name undefined");
					varr_free(class->skills);
					classes->nused--;
				}
				varr_qsort(class->skills, cmpint);
				return;
			}

		case 'F':
			SKEY("Filename", class->file_name);
			KEY("Flags", class->flags,
			    fread_fstring(class_flags, fp));
			break;

		case 'G':
			if (!str_cmp(word, "GuildRoom")) {
				int vnum = fread_number(fp);
				int *pvnum = varr_enew(class->guild);
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
			SKEY("Name", class->name);
			break;

		case 'P':
			KEY("PrimeStat", class->attr_prime,
			    fread_fword(stat_names, fp));
			break;

		case 'R':
			KEY("RestrictAlign", class->align,
			    fread_fstring(align_names, fp));
			break;

		case 'S':
			if (!str_cmp(word, "ShortName")) {
				char *p = fread_string(fp);
				strnzcpy(class->who_name, p,
					 sizeof(class->who_name));
				free_string(p);
				fMatch = TRUE;
			}
			KEY("SchoolWeapon", class->weapon, fread_number(fp));
			if (!str_cmp(word, "Skill")) {
				CLASS_SKILL *class_skill;

				class_skill = varr_enew(class->skills);
				class_skill->sn = sn_lookup(fread_word(fp));
				class_skill->level = fread_number(fp);
				class_skill->rating = fread_number(fp);
				fMatch = TRUE;
			}
			KEY("SkillAdept", class->skill_adept, fread_number(fp));
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
				if (level < 0 || level > MAX_LEVEL)
					db_error("load_class",
						 "invalid level %d", level);
				sex = fread_fword(sex_table, fp);
				if (sex < 1 || sex > 2)
					db_error("load_class", "invalid sex");
				class->titles[level][sex-1] = fread_string(fp);
			}
			break;
		}
	}
}
