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
 * $Id: db_skill.c,v 1.5 1998-10-02 04:48:41 fjoe Exp $
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

DBLOAD_FUN(load_skill)
{
	SKILL_DATA *skill;

	skill = varr_enew(&skills);

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
					skills.nused--;
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
				*skill->pgsn = skills.nused - 1;
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

