/*-
 * Copyright (c) 1999 SoG Development Team
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
 * $Id: db_spec.c,v 1.15 2000-02-10 14:09:00 fjoe Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include "merc.h"
#include "db.h"

DECLARE_DBLOAD_FUN(load_spec);
DECLARE_DBLOAD_FUN(load_spec_r);
DECLARE_DBLOAD_FUN(load_spec_skill);

DECLARE_DBINIT_FUN(init_specs);

DBFUN dbfun_specs[] =
{
	{ "SPEC",	load_spec	},
	{ "R",		load_spec_r	},
	{ "SKILL",	load_spec_skill	},
	{ NULL }
};

DBDATA db_spec = { dbfun_specs, init_specs };

static hashdata_t h_specs =
{
	sizeof(spec_t), 1,
	(e_init_t) spec_init,
	(e_destroy_t) spec_destroy,
	(e_cpy_t) spec_cpy,

	STRKEY_HASH_SIZE,
	k_hash_str,
	ke_cmp_str
};

DBINIT_FUN(init_specs)
{
	if (DBDATA_VALID(dbdata)) {
		db_set_arg(dbdata, "R", NULL);
		db_set_arg(dbdata, "SKILL", NULL);
	} else 
		hash_init(&specs, &h_specs);
}

DBLOAD_FUN(load_spec)
{
	spec_t sp;

	spec_init(&sp);

	for (;;) {
		bool fMatch = FALSE;

		fread_keyword(fp);
		switch (rfile_tokfl(fp)) {
		case 'E':
			if (IS_TOKEN(fp, "End")) {
				race_t *psp;

				if (IS_NULLSTR(sp.spec_name)) {
					log(LOG_ERROR, "load_spec: spec name undefined");
				} else if ((psp = hash_insert(&specs,
						sp.spec_name, &sp)) == NULL) {
					log(LOG_ERROR, "load_spec: duplicate spec name");
				} else {
					db_set_arg(dbdata, "R", psp);
					db_set_arg(dbdata, "SKILL", psp);
				}
				spec_destroy(&sp);
				return;
			}
			break;
		case 'C':
			KEY("Class", sp.spec_class,
			    fread_fword(spec_classes, fp));
			break;
		case 'N':
			SKEY("Name", sp.spec_name, fread_string(fp));
			break;
		}

		if (!fMatch) {
			log(LOG_ERROR, "load_spec: %s: Unknown keyword",
				 rfile_tok(fp));
			fread_to_eol(fp);
		}
	}
}

DBLOAD_FUN(load_spec_r)
{
	spec_t *spec = arg;

	if (!spec) {
		log(LOG_ERROR, "load_spec_r: No #SPEC seen yet");
		fread_to_end(fp);
		return;
	}

	fread_cc_vexpr(&spec->spec_deps, "spec", fp);
}

DBLOAD_FUN(load_spec_skill)
{
	spec_t *spec = arg;
	spec_skill_t *spec_sk;

	if (!spec) {
		log(LOG_ERROR, "load_spec_skill: No #SPEC seen yet");
		fread_to_end(fp);
		return;
	}

	spec_sk = varr_enew(&spec->spec_skills);
	for (;;) {
		bool fMatch = FALSE;

		fread_keyword(fp);
		switch(rfile_tokfl(fp)) {
		case 'A':
			KEY("Adept", spec_sk->adept, fread_number(fp));
			break;
		case 'E':
			if (IS_TOKEN(fp, "End")) {
				if (IS_NULLSTR(spec_sk->sn)) {
					log(LOG_ERROR, "load_spec_skill: skill name undefined");
					spec->spec_skills.nused--;
				} else {
					varr_qsort(&spec->spec_skills, cmpstr);
				}
				return;
			}
			break;
		case 'L':
			KEY("Level", spec_sk->level, fread_number(fp));
			break;
		case 'M':
			KEY("Min", spec_sk->min, fread_number(fp));
			KEY("Max", spec_sk->max, fread_number(fp));
			break;
		case 'R':
			KEY("Rating", spec_sk->rating, fread_number(fp));
			break;
		case 'S':
			SKEY("Skill", spec_sk->sn,
			     fread_strkey(fp, &skills, "load_spec_skill"));
			break;
		}

		if (!fMatch) {
			log(LOG_ERROR, "load_spec_skill: %s: Unknown keyword",
				 rfile_tok(fp));
			fread_to_eol(fp);
		}
	}
}
