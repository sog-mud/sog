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
 * $Id: db_skill.c,v 1.16 1999-10-26 13:52:58 fjoe Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include "merc.h"
#include "db.h"

DECLARE_DBLOAD_FUN(load_skill);

DECLARE_DBINIT_FUN(init_skills);

DBFUN dbfun_skills[] =
{
	{ "SKILL",	load_skill	},
	{ NULL }
};

DBDATA db_skills = { dbfun_skills, init_skills };

DBINIT_FUN(init_skills)
{
	if (!DBDATA_VALID(dbdata)) {
		hash_init(&skills, STRKEY_HASH_SIZE, sizeof(skill_t),
			  (varr_e_init_t) skill_init,
			  (varr_e_destroy_t) skill_destroy);
		skills.k_hash = strkey_hash;
		skills.ke_cmp = strkey_struct_cmp;
		skills.e_cpy = (hash_e_cpy_t) skill_cpy;
	}
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
				if (IS_NULLSTR(sk.name)) {
					db_error("load_skill",
						 "skill name undefined");
				} else if (!hash_insert(&skills, sk.name, &sk)) {
					db_error("load_skill",
						 "duplicate skill name");
				}
				skill_destroy(&sk);
				return;
			}
			if (IS_TOKEN(fp, "Event")) {
				flag32_t event = fread_fword(events_table, fp);
				const char *fun_name = fread_sword(fp);
				event_fun_t *evf;

				if (!event) {
					db_error("load_event", "unknown event");
				}
				for (evf = sk.eventlist; evf; evf = evf->next)
					if (!str_cmp(evf->fun_name, fun_name)) {
						db_error("load_event",
							 "duplicate fun name");
						return;
					}
				evf = calloc(1, sizeof(event_fun_t));
				evf->event = event;
				evf->fun_name = fun_name;
				evf->next = sk.eventlist;
				sk.eventlist = evf;
				fMatch = TRUE;
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
			break;
		case 'M':
			KEY("MinMana", sk.min_mana, fread_number(fp));
			KEY("MinPos", sk.min_pos,
			    fread_fword(position_table, fp));
			break;
		case 'N':
			SKEY("Name", sk.name, fread_string(fp));
			SKEY("NounDamage", sk.noun_damage, fread_string(fp));
			break;
		case 'O':
			SKEY("ObjWearOff", sk.msg_obj, fread_string(fp));
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
			SKEY("WearOff", sk.msg_off, fread_string(fp));
			break;
		}

		if (!fMatch) {
			db_error("load_skill", "%s: Unknown keyword",
				 rfile_tok(fp));
			fread_to_eol(fp);
		}
	}
}

