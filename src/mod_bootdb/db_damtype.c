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
 * $Id: db_damtype.c,v 1.16 2001-08-05 16:36:27 fjoe Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include <merc.h>
#include <db.h>
#include <rwfile.h>

DECLARE_DBLOAD_FUN(load_damtype);

DECLARE_DBINIT_FUN(init_damtype);

DBFUN dbfun_damtype[] =
{
	{ "DAMTYPE",	load_damtype,	NULL	},		// notrans
	{ NULL, NULL, NULL }
};

DBDATA db_damtype = { dbfun_damtype, init_damtype, 0 };

DBINIT_FUN(init_damtype)
{
	if (!DBDATA_VALID(dbdata))
		hash_init(&damtypes, &h_damtypes);
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
				if (IS_NULLSTR(d.dam_name)) {
					log(LOG_ERROR, "load_damtype: damtype name undefined");
				} else if (!hash_insert(&damtypes, d.dam_name, &d)) {
					log(LOG_ERROR, "load_damtype: duplicate damtype name");
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
			log(LOG_ERROR, "load_damtype: %s: Unknown keyword",
				 rfile_tok(fp));
			fread_to_eol(fp);
		}
	}
}
