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
 * $Id: db_clan.c,v 1.23 1999-10-25 12:05:29 fjoe Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "merc.h"
#include "db.h"

DECLARE_DBLOAD_FUN(load_clan);

DECLARE_DBINIT_FUN(init_clans);

DBFUN dbfun_clans[] =
{
	{ "CLAN",	load_clan	},
	{ NULL }
};

DBDATA db_clans = { dbfun_clans, init_clans };

DECLARE_DBLOAD_FUN(load_plists);

DBFUN dbfun_plists[] =
{
	{ "PLISTS",	load_plists	},
	{ NULL }
};

DBDATA db_plists = { dbfun_plists };

DBINIT_FUN(init_clans)
{
	
	if (DBDATA_VALID(dbdata))
		db_set_arg(dbdata, "PLISTS", NULL);
	else {
		hash_init(&clans, STRKEY_HASH_SIZE, sizeof(clan_t),
			  (varr_e_init_t) clan_init,
			  (varr_e_destroy_t) clan_destroy);
		clans.k_hash = strkey_hash;
		clans.ke_cmp = strkey_struct_cmp;
		clans.e_cpy = (hash_e_cpy_t) clan_cpy;
	}
}

DBLOAD_FUN(load_clan)
{
	clan_t clan;

	clan_init(&clan);

	for (;;) {
		char *word = rfile_feof(fp) ? "End" : fread_word(fp);
		bool fMatch = FALSE;

		switch (UPPER(word[0])) {
		case 'A':
			KEY("Altar", clan.altar_vnum, fread_number(fp));
			break;
		case 'E':
			if (!str_cmp(word, "End")) {
				clan_t *pclan;
				if (IS_NULLSTR(clan.name)) {
					db_error("load_clan",
						 "clan name not defined");
				} else if ((pclan = hash_insert(&clans,
						clan.name, &clan)) != NULL) {
					const char *file_name =
						strkey_filename(clan.name);
					if (dfexist(PLISTS_PATH, file_name)) {
						db_set_arg(&db_plists, "PLISTS",
							   pclan);
						db_load_file(&db_plists,
							     PLISTS_PATH,
							     file_name);
					}
				}
				clan_destroy(&clan);
				return;
			}
			break;
		case 'F':
			KEY("Flags", clan.clan_flags,
			    fread_fstring(clan_flags, fp));
			break;
		case 'I':
			KEY("Item", clan.obj_vnum, fread_number(fp));
			break;
		case 'M':
			KEY("Mark", clan.mark_vnum, fread_number(fp));
			break;
		case 'N':
			SKEY("Name", clan.name, fread_string(fp));
			break;
		case 'R':
			KEY("Recall", clan.recall_vnum, fread_number(fp));
			break;
		case 'S':
			SKEY("SkillSpec", clan.skill_spec,
			     fread_strkey(fp, &specs, "load_clan"));
		}

		if (!fMatch) 
			db_error("load_clan", "%s: Unknown keyword", word);
	}
}

DBLOAD_FUN(load_plists)
{
	clan_t *clan = arg;

	for (;;) {
		char *word = rfile_feof(fp) ? "End" : fread_word(fp);
		bool fMatch = FALSE;

		switch (UPPER(word[0])) {
		case 'E':
			if (!str_cmp(word, "End"))
				return;
			break;
		case 'L':
			SKEY("Leaders", clan->leader_list, fread_string(fp));
			break;
		case 'M':
			SKEY("Members", clan->member_list, fread_string(fp));
			break;
		case 'S':
			SKEY("Seconds", clan->second_list, fread_string(fp));
			break;
		}

		if (!fMatch) 
			db_error("load_plists", "%s: Unknown keyword", word);
	}
}

