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
 * $Id: db_cc_expr.c,v 1.3 1999-12-18 11:01:43 fjoe Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "merc.h"
#include "db.h"
#include "lang.h"

DECLARE_DBLOAD_FUN(load_cc_eclass);
DECLARE_DBINIT_FUN(init_cc_eclass);

DBFUN dbfun_cc_eclass[] =
{
	{ "ECLASS",	load_cc_eclass	},
	{ NULL }
};

DBDATA db_cc_expr = { dbfun_cc_eclass, init_cc_eclass };

static varrdata_t v_cc_eclasses =
{
	sizeof(cc_eclass_t), 1,
	(e_init_t) cc_eclass_init,
	(e_destroy_t) cc_eclass_destroy
};

/*----------------------------------------------------------------------------
 * cc_eclass loader
 */
DBINIT_FUN(init_cc_eclass)
{
	if (!DBDATA_VALID(dbdata)) 
		varr_init(&cc_eclasses, &v_cc_eclasses);
}

DBLOAD_FUN(load_cc_eclass)
{
	cc_eclass_t *rcl = varr_enew(&cc_eclasses);

	for (;;) {
		bool fMatch = FALSE;

		fread_keyword(fp);
		switch (rfile_tokfl(fp)) {
		case 'E':
			if (IS_TOKEN(fp, "efun")) {
				cc_efun_t *rfun = varr_enew(&rcl->efuns);
				rfun->name = fread_sword(fp);
				rfun->fun_name = fread_sword(fp);
				fMatch = TRUE;
			}
			if (IS_TOKEN(fp, "end")) {
				if (IS_NULLSTR(rcl->name)) {
					db_error("load_cc_eclass",
						 "cc_eclass name undefined");
					varr_edelete(&cc_eclasses, rcl);
				} else {
					varr_qsort(&cc_eclasses, cmpstr);
				}
				varr_qsort(&rcl->efuns, cmpstr);
				return;
			}
			break;
		case 'N':
			KEY("name", rcl->name, fread_sword(fp));
			break;
		}

		if (!fMatch) {
			db_error("load_cc_eclass", "%s: Unknown keyword",
				 rfile_tok(fp));
			fread_to_eol(fp);
		}
	}
}
