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
 * $Id: db_race.c,v 1.21 1999-12-16 12:24:55 fjoe Exp $
 */

#include <stdio.h>

#include "merc.h"
#include "db.h"

DECLARE_DBLOAD_FUN(load_race);
DECLARE_DBLOAD_FUN(load_pcrace);

DECLARE_DBINIT_FUN(init_race);

DBFUN dbfun_races[] =
{
	{ "RACE",	load_race	},
	{ "PCRACE",	load_pcrace	},
	{ NULL }
};

DBDATA db_races = { dbfun_races, init_race };

DBINIT_FUN(init_race)
{
	if (DBDATA_VALID(dbdata))
		db_set_arg(dbdata, "PCRACE", NULL);
	else {
		hash_init(&races, STRKEY_HASH_SIZE, sizeof(race_t),
			  (varr_e_init_t) race_init,
			  (varr_e_destroy_t) race_destroy);
		races.k_hash = strkey_hash;
		races.ke_cmp = strkey_struct_cmp;
		races.e_cpy = (hash_e_cpy_t) race_cpy;
	}
}

DBLOAD_FUN(load_race)
{
	race_t r;

	race_init(&r);
	for (;;) {
		bool fMatch = FALSE;
		
		fread_keyword(fp);
		switch(rfile_tokfl(fp)) {
		case 'A':
			KEY("Aff", r.aff, fread_fstring(affect_flags, fp));
			KEY("Act", r.act, fread_fstring(act_flags, fp));
			break;
		case 'D':
			KEY("Det", r.has_detect, fread_fstring(id_flags, fp));
			KEY("Damtype", r.damtype, 
			    fread_strkey(fp, &damtypes, "load_mobiles"));
			break;
		case 'E':
			if (IS_TOKEN(fp, "End")) {
				race_t *pr;

				if (IS_NULLSTR(r.name)) {
					db_error("load_race",
						 "race name undefined");
					race_destroy(&r);
					return;
				}

				if ((pr = hash_insert(&races, r.name,
							     &r)) == NULL) {
					db_error("load_race",
						 "duplicate race name");
				} else {
					db_set_arg(dbdata, "PCRACE", pr);
				}

				race_destroy(&r);
				return;
			}
			break;
		case 'F':
			KEY("Form", r.form, fread_fstring(form_flags, fp));
			KEY("Flags", r.race_flags, fread_fstring(race_flags, fp));
			break;
		case 'I':
			KEY("Inv", r.has_invis, fread_fstring(id_flags, fp));
			break;
		case 'N':
			SKEY("Name", r.name, fread_string(fp));
			break;
		case 'O':
			KEY("Off", r.off, fread_fstring(off_flags, fp));
			break;
		case 'P':
			KEY("Parts", r.parts, fread_fstring(part_flags, fp));
			break;
		case 'R':
			if (IS_TOKEN(fp, "Resist")) {
				int res = fread_fword(resist_flags, fp);
				if (res < 0) {
					db_error("load_race", "unknown resistance name");
					fread_number(fp);
				} else {
					r.resists[res] = fread_number(fp);
				}
				fMatch = TRUE;
				break;
			};
			break;
		}

		if (!fMatch) {
			db_error("load_race", "%s: Unknown keyword",
				 rfile_tok(fp));
			fread_to_eol(fp);
		}
	}
}

DBLOAD_FUN(load_pcrace)
{
	race_t *r = arg;
	pcrace_t *pcr;

	if (!r) {
		db_error("load_pcrace", "#PCRACE before #RACE");
		return;
	}

	pcr = r->race_pcdata = pcrace_new();

	for (;;) {
		int i;
		bool fMatch = FALSE;

		fread_keyword(fp);
		switch(rfile_tokfl(fp)) {
		case 'B':
			SKEY("BonusSkills", pcr->bonus_skills, fread_string(fp));
			break;

		case 'C':
			if (IS_TOKEN(fp, "Class")) {
				rclass_t *rcl;

				rcl = varr_enew(&pcr->classes);
				rcl->name = fread_sword(fp);
				rcl->mult = fread_number(fp);
				fMatch = TRUE;
				break;
			}
			break;

		case 'E':
			if (IS_TOKEN(fp, "End")) {
				if (pcr->who_name[0] == '\0') {
					db_error("load_pcrace",
						 "race who_name undefined");
					pcrace_free(pcr);
					r->race_pcdata = NULL;
				}
				varr_qsort(&pcr->classes, cmpstr);
				return;
			}
			break;

		case 'H':
			KEY("HPBonus", pcr->hp_bonus, fread_number(fp));
			break;

		case 'M':
			KEY("ManaBonus", pcr->mana_bonus, fread_number(fp));
			if (IS_TOKEN(fp, "MaxStats")) {
				for (i = 0; i < MAX_STATS; i++)
					pcr->max_stat[i] = fread_number(fp);
				fMatch = TRUE;
				break;
			}
			break;

		case 'P':
			KEY("Points", pcr->points, fread_number(fp));
			KEY("PracBonus", pcr->prac_bonus, fread_number(fp));
			break;

		case 'R':
			KEY("RestrictAlign", pcr->restrict_align,
			    fread_fstring(ralign_names, fp));
			KEY("RestrictEthos", pcr->restrict_ethos,
			    fread_fstring(ethos_table, fp));
			break;

		case 'S':
			KEY("Size", pcr->size, fread_fword(size_table, fp));
			KEY("Slang", pcr->slang, fread_fword(slang_table, fp));
			SKEY("SkillSpec", pcr->skill_spec,
			     fread_strkey(fp, &specs, "load_pcrace"));
			if (IS_TOKEN(fp, "ShortName")) {
				const char *p = fread_string(fp);
				strnzcpy(pcr->who_name, sizeof(pcr->who_name),
					 p);
				free_string(p);
				fMatch = TRUE;
				break;
			}
			if (IS_TOKEN(fp, "Stats")) {
				for (i = 0; i < MAX_STATS; i++)
					pcr->mod_stat[i] = fread_number(fp);
				fMatch = TRUE;
				break;
			}
			break;
		}

		if (!fMatch) {
			db_error("load_pcrace", "%s: Unknown keyword",
				 rfile_tok(fp));
			fread_to_eol(fp);
		}
	}
}
