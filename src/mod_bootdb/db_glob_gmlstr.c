/*-
 * Copyright (c) 2001 SoG Development Team
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
 * $Id: db_glob_gmlstr.c,v 1.1 2001-08-02 18:21:31 fjoe Exp $
 */

#include <stdio.h>

#include <merc.h>
#include <bootdb.h>
#include <rwfile.h>

DECLARE_DBLOAD_FUN(load_glob_gmlstr);
DECLARE_DBINIT_FUN(init_glob_gmlstr);

DBFUN dbfun_glob_gmlstr[] = {
	{ "GLOB",	load_glob_gmlstr,	NULL	},	// notrans
	{ NULL, NULL, NULL }
};

DBDATA db_glob_gmlstr = { dbfun_glob_gmlstr, init_glob_gmlstr, 0 };

static hashdata_t h_glob_gmlstr =
{
	sizeof(gmlstr_t), 1,
	(e_init_t) gmlstr_init,
	(e_destroy_t) gmlstr_destroy,
	(e_cpy_t) gmlstr_cpy,

	STRKEY_HASH_SIZE,
	k_hash_str,
	ke_cmp_mlstr
};

DBINIT_FUN(init_glob_gmlstr)
{
	if (!DBDATA_VALID(dbdata))
		hash_init(&glob_gmlstr, &h_glob_gmlstr);
}

DBLOAD_FUN(load_glob_gmlstr)
{
	gmlstr_t gml;

	gmlstr_init(&gml);
	mlstr_fread(fp, &gml.ml);
	if (mlstr_null(&gml.ml)) {
		gmlstr_destroy(&gml);
		log(LOG_ERROR, "load_glob_gmlstr: null gmlstr");
		fread_to_end(fp);
		return;
	}

	for (;;) {
		bool fMatch = FALSE;

		fread_keyword(fp);
		switch (rfile_tokfl(fp)) {
		case 'E':
			if (IS_TOKEN(fp, "End")) {
				if (hash_insert(&glob_gmlstr, gmlstr_mval(&gml), &gml) == NULL) {
					log(LOG_ERROR, "load_gmlstr: duplicate gmlstr");
				}
				gmlstr_destroy(&gml);
				return;
			}
			break;
		case 'G':
			MLSKEY("gender", gml.gender);
			break;
		}

		if (!fMatch) {
			log(LOG_ERROR, "load_glob_gmlstr: %s: Unknown keyword",
				 rfile_tok(fp));
			fread_to_eol(fp);
		}
	}
}
