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
 * $Id: db_liquid.c,v 1.12 2000-02-10 14:08:59 fjoe Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include "merc.h"
#include "db.h"

DECLARE_DBLOAD_FUN(load_liquid);

DECLARE_DBINIT_FUN(init_liquids);

DBFUN dbfun_liquids[] =
{
	{ "LIQUID",	load_liquid	},
	{ NULL }
};

DBDATA db_liquids = { dbfun_liquids, init_liquids };

static hashdata_t h_liquids =
{
	sizeof(liquid_t), 1,
	(e_init_t) liquid_init,
	(e_destroy_t) liquid_destroy,
	(e_cpy_t) liquid_cpy,

	STRKEY_HASH_SIZE,
	k_hash_str,
	ke_cmp_mlstr
};

DBINIT_FUN(init_liquids)
{
	if (DBDATA_VALID(dbdata))
		hash_init(&liquids, &h_liquids);
}

DBLOAD_FUN(load_liquid)
{
	liquid_t lq;

	liquid_init(&lq);

	for (;;) {
		bool fMatch = FALSE;

		fread_keyword(fp);
		switch (rfile_tokfl(fp)) {
		case 'A':
			if (IS_TOKEN(fp, "Affect")) {
				int i;

				for (i = 0; i < MAX_COND; i++)
					lq.affect[i] = fread_number(fp);
				fMatch = TRUE;
			}
			break;
		case 'C':
			MLSKEY("Color", lq.lq_color);
			break;
		case 'E':
			if (IS_TOKEN(fp, "End")) {
				const char *ln = gmlstr_mval(&lq.lq_name);

				if (IS_NULLSTR(ln))
					log(LOG_ERROR, "load_liquid: liquid name undefined");
				else if (!hash_insert(&liquids, ln, &lq))
					log(LOG_ERROR, "load_liquid: duplicate liquid name");
				liquid_destroy(&lq);
				return;
			}
			break;
		case 'G':
			MLSKEY("Gender", lq.lq_name.gender);
			break;
		case 'N':
			MLSKEY("Name", lq.lq_name.ml);
			break;
		case 'S':
			KEY("Sip", lq.sip, fread_number(fp));
			break;
		}

		if (!fMatch) {
			log(LOG_ERROR, "load_liquid: %s: Unknown keyword",
				 rfile_tok(fp));
			fread_to_eol(fp);
		}
	}
}
