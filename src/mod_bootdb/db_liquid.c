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
 * $Id: db_liquid.c,v 1.20 2003-09-30 00:31:04 fjoe Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include <merc.h>
#include <db.h>
#include <rwfile.h>

DECLARE_DBLOAD_FUN(load_liquid);

DECLARE_DBINIT_FUN(init_liquids);

DBFUN dbfun_liquids[] =
{
	{ "LIQUID",	load_liquid,	NULL	},		// notrans
	{ NULL, NULL, NULL }
};

DBDATA db_liquids = { dbfun_liquids, init_liquids, 0 };

DBINIT_FUN(init_liquids)
{
	if (DBDATA_VALID(dbdata))
		c_init(&liquids, &c_info_liquids);
}

DBLOAD_FUN(load_liquid)
{
	liquid_t *lq = NULL;

	for (;;) {
		bool fMatch = FALSE;

		fread_keyword(fp);
		switch (rfile_tokfl(fp)) {
		case 'A':
			CHECK_VAR(lq, "Name");

			if (IS_TOKEN(fp, "Affect")) {
				int i;

				for (i = 0; i < MAX_COND; i++)
					lq->affect[i] = fread_number(fp);
				fMatch = TRUE;
			}
			break;

		case 'C':
			CHECK_VAR(lq, "Name");

			MLSKEY("Color", lq->lq_color);
			break;

		case 'E':
			CHECK_VAR(lq, "Name");

			if (IS_TOKEN(fp, "End"))
				return;
			break;

		case 'G':
			CHECK_VAR(lq, "Name");

			MLSKEY("Gender", lq->lq_name.gender);
			break;

		case 'N':
			MLSPKEY("Name", lq->lq_name.ml, &liquids, lq);
			break;

		case 'S':
			CHECK_VAR(lq, "Name");

			KEY("Sip", lq->sip, fread_number(fp));
			break;
		}

		if (!fMatch) {
			printlog(LOG_ERROR, "%s: %s: Unknown keyword",
			    __FUNCTION__, rfile_tok(fp));
			fread_to_eol(fp);
		}
	}
}
