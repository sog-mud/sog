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
 * $Id: db_clan.c,v 1.40 2003-09-30 00:31:03 fjoe Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <merc.h>
#include <db.h>
#include <rwfile.h>

DECLARE_DBLOAD_FUN(load_clan);

DECLARE_DBINIT_FUN(init_clans);

DBFUN dbfun_clans[] =
{
	{ "CLAN",	load_clan,	NULL	},		// notrans
	{ NULL, NULL, NULL }
};

DBDATA db_clans = { dbfun_clans, init_clans, 0 };

DECLARE_DBLOAD_FUN(load_plists);

DBFUN dbfun_plists[] =
{
	{ "PLISTS",	load_plists,	NULL	},		// notrans
	{ NULL , NULL, NULL }
};

DBDATA db_plists = { dbfun_plists, NULL, 0 };

DBINIT_FUN(init_clans)
{
	if (DBDATA_VALID(dbdata))
		db_set_arg(dbdata, "PLISTS", NULL);
	else
		c_init(&clans, &c_info_clans);
}

DBLOAD_FUN(load_clan)
{
	clan_t *clan = NULL;

	for (;;) {
		bool fMatch = FALSE;

		fread_keyword(fp);
		switch (rfile_tokfl(fp)) {
		case 'A':
			CHECK_VAR(clan, "Name");

			KEY("Altar", clan->altar_vnum, fread_number(fp));
			break;

		case 'E':
			CHECK_VAR(clan, "Name");

			if (IS_TOKEN(fp, "End")) {
				const char *filename;

				filename = strkey_filename(
				    clan->name, CLAN_EXT);
				if (dfexist(PLISTS_PATH, filename)) {
					db_set_arg(&db_plists, "PLISTS", clan);
					db_load_file(
					    &db_plists, PLISTS_PATH, filename);
				}

				return;
			}
			break;

		case 'F':
			CHECK_VAR(clan, "Name");

			KEY("Flags", clan->clan_flags,
			    fread_fstring(clan_flags, fp));
			break;

		case 'I':
			CHECK_VAR(clan, "Name");

			KEY("Item", clan->obj_vnum, fread_number(fp));
			break;

		case 'M':
			CHECK_VAR(clan, "Name");

			KEY("Mark", clan->mark_vnum, fread_number(fp));
			break;

		case 'N':
			SPKEY("Name", clan->name, fread_string(fp),
			      &clans, clan);
			break;

		case 'R':
			CHECK_VAR(clan, "Name");

			KEY("Recall", clan->recall_vnum, fread_number(fp));
			break;

		case 'S':
			CHECK_VAR(clan, "Name");

			SKEY("SkillSpec", clan->skill_spec,
			     fread_strkey(fp, &specs));
			break;
		}

		if (!fMatch) {
			printlog(LOG_ERROR, "%s: %s: Unknown keyword",
			    __FUNCTION__, rfile_tok(fp));
			fread_to_eol(fp);
		}
	}
}

DBLOAD_FUN(load_plists)
{
	clan_t *clan = arg;

	for (;;) {
		bool fMatch = FALSE;

		fread_keyword(fp);
		switch (rfile_tokfl(fp)) {
		case 'E':
			if (IS_TOKEN(fp, "End"))
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

		if (!fMatch) {
			printlog(LOG_ERROR, "%s: %s: Unknown keyword",
			    __FUNCTION__, rfile_tok(fp));
			fread_to_eol(fp);
		}
	}
}
