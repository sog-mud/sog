/*
 * $Id: db_skill.c,v 1.2 1998-09-15 02:51:38 fjoe Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include "merc.h"
#include "db.h"

DECLARE_DBLOAD_FUN(load_skill);

DBFUN db_load_skills[] =
{
	{ "SKILL",	load_skill	},
	{ NULL }
};

DBINIT_FUN(init_skills)
{
	skills = varr_new(sizeof(SKILL_DATA), 8);
}

DBLOAD_FUN(load_skill)
{
	SKILL_DATA *skill;

	skill = varr_enew(skills);

	for (;;) {
		char *word = feof(fp) ? "End" : fread_word(fp);
		bool fMatch = FALSE;

		switch(UPPER(word[0])) {
		case 'B':
			KEY("Beats", skill->beats, fread_number(fp));
			break;

		case 'E':
			if (!str_cmp(word, "End")) {
				if (IS_NULLSTR(skill->name)) {
					db_error("load_skill",
						 "skill name undefined");
					skills->nused--;
				}
				return;
			}
			break;

		case 'F':
			KEY("Flags", skill->flags,
			    fread_fstring(skill_flags, fp));
			break;

		case 'G':
			KEY("Group", skill->group,
			    fread_fword(skill_groups, fp));
			if (!str_cmp(word, "Gsn")) {
				skill->pgsn = fread_namedp(gsn_table, fp);
				*skill->pgsn = skills->nused - 1;
				fMatch = TRUE;
			}
			break;

		case 'M':
			KEY("MinMana", skill->min_mana, fread_number(fp));
			KEY("MinPos", skill->minimum_position,
			    fread_fword(position_table, fp));
			break;

		case 'N':
			SKEY("Name", skill->name);
			SKEY("NounDamage", skill->noun_damage);
			break;

		case 'S':
			KEY("Slot", skill->slot, fread_number(fp));
			KEY("SpellFun", skill->spell_fun,
			    fread_namedp(spellfn_table, fp));
			break;

		case 'T':
			KEY("Type", word, fread_word(fp));	/* just skip */
			KEY("Target", skill->target,
			    fread_fword(skill_targets, fp));
			break;
		case 'W':
			SKEY("WearOff", skill->msg_off);
			SKEY("ObjWearOff", skill->msg_obj);
			break;
		}
	}
}

