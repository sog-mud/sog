/*-
 * Copyright (c) 1999 arborn <avn@org.chem.msu.su>
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
 * $Id: db_liquid.c,v 1.3 1999-10-21 14:27:55 fjoe Exp $
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

DBINIT_FUN(init_liquids)
{
	if (DBDATA_VALID(dbdata)) {
		hash_init(&liquids, STRKEY_HASH_SIZE, sizeof(liquid_t),
			  (varr_e_init_t) liquid_init,
			  (varr_e_destroy_t) liquid_destroy);
		liquids.k_hash = strkey_hash;
		liquids.ke_cmp = strkey_struct_cmp;
		liquids.e_cpy = (hash_e_cpy_t) liquid_cpy;
	}
}

DBLOAD_FUN(load_liquid)
{
	liquid_t lq;

	liquid_init(&lq);

	for (;;) {
		char *word = feof(fp) ? "End" : fread_word(fp);
		bool fMatch = FALSE;

		switch (UPPER(word[0])) {
		case 'A':
			if (!str_cmp(word, "Affect")) {
				int i;

				for (i = 0; i < MAX_COND; i++)
					lq.affect[i] = fread_number(fp);
				fMatch = TRUE;
			}
			break;
		case 'C':
			SKEY("Color", lq.color, fread_string(fp));
			break;
		case 'E':
			if (!str_cmp(word, "End")) {
				if (IS_NULLSTR(lq.name))
					db_error("load_liquid",
						 "liquid name undefined");
				else if (!hash_insert(&liquids, lq.name, &lq))
					db_error("load_liquid",
						 "duplicate liquid name");
				liquid_destroy(&lq);
				return;
			}
			break;
		case 'N':
			SKEY("Name", lq.name, fread_string(fp));
			break;
		case 'S':
			KEY("Sip", lq.sip, fread_number(fp));
			break;
		}

		if (!fMatch)
			db_error("load_liquid", "%s: Unknown keyword", word);
	}
}
