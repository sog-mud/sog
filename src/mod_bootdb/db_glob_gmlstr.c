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
 * $Id: db_glob_gmlstr.c,v 1.4 2001-09-13 12:02:51 fjoe Exp $
 */

#include <stdio.h>

#include <merc.h>
#include <db.h>
#include <rwfile.h>

DECLARE_DBLOAD_FUN(load_glob_gmlstr);
DECLARE_DBINIT_FUN(init_glob_gmlstr);

DBFUN dbfun_glob_gmlstr[] = {
	{ "GLOB",	load_glob_gmlstr,	NULL	},	// notrans
	{ NULL, NULL, NULL }
};

DBDATA db_glob_gmlstr = { dbfun_glob_gmlstr, init_glob_gmlstr, 0 };

DBINIT_FUN(init_glob_gmlstr)
{
	if (!DBDATA_VALID(dbdata))
		c_init(&glob_gmlstr, &h_glob_gmlstr);
}

DBLOAD_FUN(load_glob_gmlstr)
{
	gmlstr_t gml;
	gmlstr_t *gmlp;

	gmlstr_init(&gml);
	mlstr_fread(fp, &gml.ml);
	if (mlstr_null(&gml.ml)) {
		gmlstr_destroy(&gml);
		log(LOG_ERROR, "%s: null gmlstr", __FUNCTION__);
		fread_to_end(fp);
		return;
	}

	if ((gmlp = c_insert(&glob_gmlstr, gmlstr_mval(&gml))) == NULL) {
		log(LOG_ERROR, "%s: %s: duplicate gmlstr",
		    __FUNCTION__, gmlstr_mval(&gml));
		fread_to_end(fp);
		return;
	}
	gmlstr_cpy(gmlp, &gml);
	gmlstr_destroy(&gml);

	for (;;) {
		bool fMatch = FALSE;

		fread_keyword(fp);
		switch (rfile_tokfl(fp)) {
		case 'E':
			if (IS_TOKEN(fp, "End"))
				return;
			break;
		case 'G':
			MLSKEY("gender", gmlp->gender);
			break;
		}

		if (!fMatch) {
			log(LOG_ERROR, "%s: %s: Unknown keyword",
			    __FUNCTION__, rfile_tok(fp));
			fread_to_eol(fp);
		}
	}
}
