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
 * $Id: db_damtype.c,v 1.8 1999-12-16 12:24:54 fjoe Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include "merc.h"
#include "db.h"

DECLARE_DBLOAD_FUN(load_damtype);

DECLARE_DBINIT_FUN(init_damtype);

DBFUN dbfun_damtype[] =
{
	{ "DAMTYPE",	load_damtype	},
	{ NULL }
};

DBDATA db_damtype = { dbfun_damtype, init_damtype };

DBINIT_FUN(init_damtype)
{
	if (!DBDATA_VALID(dbdata)) {
		hash_init(&damtypes, STRKEY_HASH_SIZE, sizeof(damtype_t),
			  (varr_e_init_t) damtype_init,
			  (varr_e_destroy_t) damtype_destroy);
		damtypes.k_hash = strkey_hash;
		damtypes.ke_cmp = strkey_struct_cmp;
		damtypes.e_cpy = (hash_e_cpy_t) damtype_cpy;
	}
}

DBLOAD_FUN(load_damtype)
{
	damtype_t d;

	damtype_init(&d);

	for (;;) {
		bool fMatch = FALSE;

		fread_keyword(fp);
		switch (rfile_tokfl(fp)) {
		case 'C':
			KEY("Class", d.dam_class, fread_fword(dam_classes, fp));
			break;
		case 'E':
			if (IS_TOKEN(fp, "End")) {
				msg_t *m;
				const char *dn = gmlstr_mval(&d.dam_noun);

				if (mlstr_nlang(&d.dam_noun.ml) == 0
				&&  (m = msg_lookup(dn)) != NULL
				&&  str_cmp(dn, "silver")
				&&  str_cmp(dn, "gold")) {
					mlstr_cpy(&d.dam_noun.ml, &m->ml);
					if (m->gender) {
						const char **pp = mlstr_convert(&d.dam_noun.gender, 1);
						free_string(*pp);
						*pp = str_dup(flag_string(gender_table, m->gender));
					}
				}

				if (IS_NULLSTR(d.dam_name)) {
					db_error("load_damtype",
						 "damtype name undefined");
				} else if (!hash_insert(&damtypes, d.dam_name, &d)) {
					db_error("load_damtype",
						 "duplicate damtype name");
				}
				damtype_destroy(&d);
				return;
			}
			break;
		case 'G':
			MLSKEY("Gender", d.dam_noun.gender);
			break;
		case 'N':
			KEY("Name", d.dam_name, fread_sword(fp));
			MLSKEY("Noun", d.dam_noun.ml);
			break;
		case 'S':
			KEY("Slot", d.dam_slot, fread_number(fp));
			break;
		}

		if (!fMatch) {
			db_error("load_damtype", "%s: Unknown keyword",
				 rfile_tok(fp));
			fread_to_eol(fp);
		}
	}
}
