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
 * $Id: db_spec.c,v 1.26 2001-09-13 16:22:00 fjoe Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include <merc.h>
#include <db.h>
#include <rwfile.h>

DECLARE_DBLOAD_FUN(load_spec);
DECLARE_DBLOAD_FUN(load_spec_skill);

DECLARE_DBINIT_FUN(init_specs);

DBFUN dbfun_specs[] =
{
	{ "SPEC",	load_spec,	NULL	},		// notrans
	{ "SKILL",	load_spec_skill,NULL	},		// notrans
	{ NULL, NULL, NULL }
};

DBDATA db_spec = { dbfun_specs, init_specs, 0 };

DBINIT_FUN(init_specs)
{
	if (DBDATA_VALID(dbdata)) {
		db_set_arg(dbdata, "SKILL", NULL);
	} else
		c_init(&specs, &c_info_specs);
}

DBLOAD_FUN(load_spec)
{
	spec_t *sp = NULL;

	for (;;) {
		bool fMatch = FALSE;

		fread_keyword(fp);
		switch (rfile_tokfl(fp)) {
		case 'E':
			CHECK_VAR(sp, "Name");

			if (IS_TOKEN(fp, "End")) {
				db_set_arg(dbdata, "SKILL", sp);
				return;
			}
			break;

		case 'C':
			CHECK_VAR(sp, "Name");

			KEY("Class", sp->spec_class,
			    fread_fword(spec_classes, fp));
			break;

		case 'N':
			SPKEY("Name", sp->spec_name, fread_string(fp),
			      &specs, sp);
			break;

		case 'T':
			CHECK_VAR(sp, "Name");

			SKEY("Trigger", sp->mp_trig.trig_prog,
			     fread_string(fp));
			break;
		}

		if (!fMatch) {
			log(LOG_ERROR, "%s: %s: Unknown keyword",
			    __FUNCTION__, rfile_tok(fp));
			fread_to_eol(fp);
		}
	}
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
			     fread_strkey(fp, &skills));
			break;
		}

		if (!fMatch) {
			log(LOG_ERROR, "%s: %s: Unknown keyword",
			    __FUNCTION__, rfile_tok(fp));
			fread_to_eol(fp);
		}
	}
}
