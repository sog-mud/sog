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
 * $Id: db_rspells.c,v 1.1.2.1 1999-12-16 12:40:08 fjoe Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include "merc.h"
#include "db.h"

DECLARE_DBLOAD_FUN(load_rspell);

DBFUN dbfun_rspells[] =
{
	{ "RSPELL",	load_rspell	},
	{ NULL }
};

DBDATA db_rspells = { dbfun_rspells };

DBLOAD_FUN(load_rspell)
{
	rspell_t *rspell;

	rspell = varr_enew(&rspells);

	for (;;) {
		char *word = feof(fp) ? "End" : fread_word(fp);
		bool fMatch = FALSE;

		switch (UPPER(word[0])) {
		case 'E':
			if (!str_cmp(word, "End")) {
				if (IS_NULLSTR(rspell->name)) {
					db_error("load_rspell",
						 "skill name undefined");
					rspells.nused--;
				}
				if ((rspell->sn = sn_lookup(rspell->name)) == -1) {
					db_error("load_rspell",
						 "no matching skill");
					rspells.nused--;
				}
				return;
			}
			if (!str_cmp(word, "Enter")) rspell->events |= EVENT_ENTER;
			KEY("Enter", rspell->enter_fun_name, str_dup(fread_word(fp)));
			break;
		case 'L':
			if (!str_cmp(word, "Leave")) rspell->events |= EVENT_LEAVE;
			KEY("Leave", rspell->leave_fun_name, str_dup(fread_word(fp)));
			break;
		case 'N':
			SKEY("Name", rspell->name);
			break;
		case 'U':
			if (!str_cmp(word, "Update")) rspell->events |= EVENT_UPDATE;
			KEY("Update", rspell->update_fun_name, str_dup(fread_word(fp)));
			break;
		}
		if (!fMatch)
			db_error("load_rspell", "%s: Unknown keyword", word);
	}
}
