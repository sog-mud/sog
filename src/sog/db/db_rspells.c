/*-
 * Copyright (c) 1998 Arborn <avn@org.chem.msu.su>
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
 * $Id: db_rspells.c,v 1.2 1999-10-06 09:56:15 fjoe Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include "merc.h"
#include "db.h"

DECLARE_DBLOAD_FUN(load_rspell);

DECLARE_DBINIT_FUN(init_rspells);

DBFUN dbfun_rspells[] =
{
	{ "RSPELL",	load_rspell	},
	{ NULL }
};

DBDATA db_rspells = { dbfun_rspells, init_rspells };

DBINIT_FUN(init_rspells)
{
	if (DBDATA_VALID(dbdata)) {
		hash_init(&rspells, NAME_HASH_SIZE, sizeof(rspell_t),
			  (varr_e_init_t) rspell_init,
			  (varr_e_destroy_t) rspell_destroy);
		rspells.k_hash = name_hash;
		rspells.ke_cmp = name_struct_cmp;
		rspells.e_cpy = (hash_e_cpy_t) rspell_cpy;
	}
}

DBLOAD_FUN(load_rspell)
{
	rspell_t rsp;

	rspell_init(&rsp);

	for (;;) {
		char *word = feof(fp) ? "End" : fread_word(fp);
		bool fMatch = FALSE;

		switch (UPPER(word[0])) {
		case 'E':
			if (!str_cmp(word, "End")) {
				if (IS_NULLSTR(rsp.sn)) {
					db_error("load_rspell",
						 "skill name undefined");
				} else if (!hash_insert(&rspells, rsp.sn, &rsp)) {
					db_error("load_rspell",
						 "duplicate rspell name");
				}
				rspell_destroy(&rsp);
				return;
			}

			if (!str_cmp(word, "Event")) {
				flag32_t event = fread_fword(rspell_events, fp);
				const char *fun_name = str_dup(fread_word(fp));

				rsp.revents |= event;
				switch (event) {
				case REVENT_ENTER:
					free_string(rsp.enter_fun_name);
					rsp.enter_fun_name = fun_name;
					break;
				case REVENT_UPDATE:
					free_string(rsp.update_fun_name);
					rsp.update_fun_name = fun_name;
					break;
				case REVENT_LEAVE:
					free_string(rsp.leave_fun_name);
					rsp.leave_fun_name = fun_name;
					break;
				}
				fMatch = TRUE;
			}
			break;
		case 'S':
			SKEY("Skill", rsp.sn,
			     fread_name(fp, &skills, "load_rspell"));
			break;
		}

		if (!fMatch)
			db_error("load_rspell", "%s: Unknown keyword", word);
	}
}
