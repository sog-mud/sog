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
 * $Id: db_cc_rulecl.c,v 1.2 1999-11-24 07:22:28 fjoe Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "merc.h"
#include "db.h"
#include "lang.h"

DECLARE_DBLOAD_FUN(load_cc_rulecl);
DECLARE_DBINIT_FUN(init_cc_rulecl);

DBFUN dbfun_cc_rulecl[] =
{
	{ "CC_RULECL",	load_cc_rulecl	},
	{ NULL }
};

DBDATA db_cc_rulecl = { dbfun_cc_rulecl, init_cc_rulecl };

/*----------------------------------------------------------------------------
 * cc_rulecl loader
 */
DBINIT_FUN(init_cc_rulecl)
{
	if (!DBDATA_VALID(dbdata)) {
		varr_init(&cc_rulecls, sizeof(cc_rulecl_t), 1);
		cc_rulecls.e_init = (varr_e_init_t) cc_rulecl_init;
		cc_rulecls.e_destroy = (varr_e_destroy_t) cc_rulecl_destroy;
	}
}

DBLOAD_FUN(load_cc_rulecl)
{
	cc_rulecl_t *rcl = varr_enew(&cc_rulecls);

	for (;;) {
		bool fMatch = FALSE;

		fread_keyword(fp);
		switch (rfile_tokfl(fp)) {
		case 'E':
			if (IS_TOKEN(fp, "End")) {
				if (IS_NULLSTR(rcl->name)) {
					db_error("load_cc_rulecl",
						 "cc_rulecl name undefined");
					varr_edelete(&cc_rulecls, rcl);
				} else {
					varr_qsort(&cc_rulecls, cmpstr);
				}
				return;
			}
			break;
		case 'N':
			KEY("Name", rcl->name, fread_sword(fp));
			break;
		case 'T':
			if (IS_TOKEN(fp, "Type")) {
				cc_rulefun_t *rfun = varr_enew(&rcl->rulefuns);
				rfun->type = fread_sword(fp);
				rfun->fun_name = fread_sword(fp);
				varr_qsort(&rcl->rulefuns, cmpstr);
				fMatch = TRUE;
			}
			break;
		}

		if (!fMatch) {
			db_error("load_cc_rulecl", "%s: Unknown keyword",
				 rfile_tok(fp));
			fread_to_eol(fp);
		}
	}
}
