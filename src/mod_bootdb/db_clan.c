/*
 * $Id: db_clan.c,v 1.1 1998-09-01 18:37:57 fjoe Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include "merc.h"
#include "db.h"

DECLARE_DBLOAD_FUN(load_clan);

DBFUN db_load_clans[] =
{
	{ "CLAN",	load_clan	},
	{ NULL }
};

DBINIT(init_clans)
{
	clans = varr_new(sizeof(CLAN_DATA), 4);
}

void load_clan(FILE *fp)
{
	CLAN_DATA *clan;

	clan = varr_enew(clans);
	clan->skills = varr_new(sizeof(CLAN_SKILL), 8);
	clan->file_name = str_dup(filename);

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

