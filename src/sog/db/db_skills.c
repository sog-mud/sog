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
 * $Id: db_skills.c,v 1.28 2001-06-24 10:51:02 avn Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include "merc.h"
#include "db.h"

DECLARE_DBLOAD_FUN(load_skill);

DECLARE_DBINIT_FUN(init_skills);

DBFUN dbfun_skills[] =
{
	{ "SKILL",	load_skill,	NULL	},		// notrans
	{ NULL, NULL, NULL }
};

DBDATA db_skills = { dbfun_skills, init_skills, 0 };

static hashdata_t h_skills =
{
	sizeof(skill_t), 1,
	(e_init_t) skill_init,
	(e_destroy_t) skill_destroy,
	(e_cpy_t) skill_cpy,

	STRKEY_HASH_SIZE,
	k_hash_str,
	ke_cmp_mlstr
};

DBINIT_FUN(init_skills)
{
	if (!DBDATA_VALID(dbdata))
		hash_init(&skills, &h_skills);
}

DBLOAD_FUN(load_skill)
{
	skill_t sk;

	skill_init(&sk);

	for (;;) {
		bool fMatch = FALSE;

		fread_keyword(fp);
		switch (rfile_tokfl(fp)) {
		case 'B':
			KEY("Beats", sk.beats, fread_number(fp));
			break;
		case 'E':
			if (IS_TOKEN(fp, "End")) {
				const char *sn = gmlstr_mval(&sk.sk_name);

				if (IS_NULLSTR(sn)) {
					log(LOG_ERROR, "load_skill: skill name undefined");
				} else if (!hash_insert(&skills, sn, &sk)) {
					log(LOG_ERROR, "load_skill: duplicate skill name");
				}
				skill_destroy(&sk);
				return;
			}
			if (IS_TOKEN(fp, "Event")) {
				flag_t event = fread_fword(events_classes, fp);
				evf_t *evf;

				if (event < 0) {
					log(LOG_ERROR, "load_skill: unknown event");
					fread_to_eol(fp);
					break;
				}

				evf = varr_bsearch(&sk.events, &event, cmpint);
				if (evf != NULL) {
					log(LOG_ERROR, "load_skill: %s: duplicate event", flag_string(events_classes, event));
					fread_to_eol(fp);
					break;
				}

				evf = varr_enew(&sk.events);
				evf->event = event;
				evf->fun_name = fread_sword(fp);
				varr_qsort(&sk.events, cmpint);
				fMatch = TRUE;
				break;
			}
			break;
		case 'F':
			KEY("Flags", sk.skill_flags,
			    fread_fstring(skill_flags, fp));
			SKEY("Fun", sk.fun_name, fread_string(fp));
			break;
		case 'G':
			KEY("Group", sk.group,
			    fread_fword(skill_groups, fp));
			MLSKEY("Gender", sk.sk_name.gender);
			break;
		case 'M':
			KEY("MinMana", sk.min_mana, fread_number(fp));
			KEY("MinPos", sk.min_pos,
			    fread_fword(position_table, fp));
			break;
		case 'N':
			MLSKEY("Name", sk.sk_name.ml);
			MLSKEY("NounDamage", sk.noun_damage.ml);
			MLSKEY("NounGender", sk.noun_damage.gender);
			break;
		case 'O':
			MLSKEY("ObjWearOff", sk.msg_obj);
			break;
		case 'R':
			KEY("Rank", sk.rank, fread_number(fp));
			if (sk.rank < 0 || sk.rank > 7) {
				sk.rank = 0;
				log(LOG_ERROR,
				    "load_skill: rank should be beetwen 0..7");
			}
			break;
		case 'S':
			KEY("Slot", sk.slot, fread_number(fp));
			KEY("SpellFun", sk.fun_name, fread_sword(fp));
			break;
		case 'T':
			KEY("Type", sk.skill_type,
			    fread_fword(skill_types, fp));
			KEY("Target", sk.target,
			    fread_fword(skill_targets, fp));
			break;
		case 'W':
			MLSKEY("WearOff", sk.msg_off);
			break;
		}

		if (!fMatch) {
			log(LOG_ERROR, "load_skill: %s: Unknown keyword",
				 rfile_tok(fp));
			fread_to_eol(fp);
		}
	}
}

