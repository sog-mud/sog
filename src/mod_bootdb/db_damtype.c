/*-
 * Copyright (c) 1999 SoG Development Team
 * All rights reservedt->
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
 * $Id: db_damtype.c,v 1.19 2001-09-13 16:21:58 fjoe Exp $
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
		c_init(&damtypes, &c_info_damtypes);
}

DBLOAD_FUN(load_damtype)
{
	damtype_t *dt = NULL;

	for (;;) {
		bool fMatch = FALSE;

		fread_keyword(fp);
		switch (rfile_tokfl(fp)) {
		case 'C':
			CHECK_VAR(dt, "Name");

			KEY("Class", dt->dam_class, fread_fword(dam_classes, fp));
			break;

		case 'E':
			CHECK_VAR(dt, "Name");

			if (IS_TOKEN(fp, "End"))
				return;
			break;

		case 'G':
			CHECK_VAR(dt, "Name");

			MLSKEY("Gender", dt->dam_noun.gender);
			break;

		case 'N':
			SPKEY("Name", dt->dam_name, fread_sword(fp),
			      &damtypes, dt);

			CHECK_VAR(dt, "Name");

			MLSKEY("Noun", dt->dam_noun.ml);
			break;

		case 'S':
			CHECK_VAR(dt, "Name");

			KEY("Slot", dt->dam_slot, fread_number(fp));
			break;
		}

		if (!fMatch) {
			log(LOG_ERROR, "%s: %s: Unknown keyword",
			    __FUNCTION__, rfile_tok(fp));
			fread_to_eol(fp);
		}
	}
}
