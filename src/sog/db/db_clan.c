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
 * $Id: db_clan.c,v 1.5 1998-10-01 06:39:21 fjoe Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "merc.h"
#include "db/db.h"

DECLARE_DBLOAD_FUN(load_clan);

DBFUN db_load_clans[] =
{
	{ "CLAN",	load_clan	},
	{ NULL }
};

void init_clans(void)
{
	clans = varr_new(sizeof(CLAN_DATA), 4);
}

DBLOAD_FUN(load_clan)
{
	CLAN_DATA *	clan;
	char *		p;

	clan = varr_enew(clans);
	clan->skills = varr_new(sizeof(CLAN_SKILL), 8);
	if ((p = strrchr(filename, '/')))
		p++;
	else
		p = filename;
	clan->file_name = str_dup(p);

	for (;;) {
		char *word = feof(fp) ? "End" : fread_word(fp);
		bool fMatch = FALSE;

		switch (UPPER(word[0])) {
		case 'E':
			if (!str_cmp(word, "End")) {
				if (IS_NULLSTR(clan->name)) {
					db_error("load_clan",
						 "clan name not defined");
					varr_free(clan->skills);
					clans->nused--;
				}
				varr_qsort(clan->skills, cmpint);
				return;
			}
			break;
		case 'F':
			KEY("Flags", clan->flags,
			    fread_fstring(clan_flags, fp));
			SKEY("Filename", clan->file_name);
			break;
		case 'M':
			MLSKEY("MsgPrays", clan->msg_prays);
			MLSKEY("MsgVanishes", clan->msg_vanishes);
			break;
		case 'N':
			SKEY("Name", clan->name);
			break;
		case 'R':
			KEY("Recall", clan->recall_vnum, fread_number(fp));
			break;
		case 'S':
			if (!str_cmp(word, "Skill")) {
				CLAN_SKILL *sk = varr_enew(clan->skills);
				sk->sn = sn_lookup(fread_word(fp));	
				sk->level = fread_number(fp);
				fMatch = TRUE;
			}
			break;
		}
	}
}

