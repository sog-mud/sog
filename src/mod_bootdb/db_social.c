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
 * $Id: db_social.c,v 1.24 2003-09-30 00:31:04 fjoe Exp $
 */

#include <limits.h>
#include <stdio.h>
#include <ctype.h>

#include <merc.h>
#include <db.h>
#include <rwfile.h>

DECLARE_DBLOAD_FUN(load_social);

DECLARE_DBINIT_FUN(init_socials);

DBFUN dbfun_socials[] =
{
	{ "SOCIAL",	load_social,	NULL	},		// notrans
	{ NULL, NULL, NULL }
};

DBDATA db_socials = { dbfun_socials, init_socials, 0 };

DBINIT_FUN(init_socials)
{
	if (!DBDATA_VALID(dbdata))
		c_init(&socials, &c_info_socials);
}

DBLOAD_FUN(load_social)
{
	social_t *soc = varr_enew(&socials);

	for (;;) {
		bool fMatch = FALSE;

		fread_keyword(fp);
		switch (rfile_tokfl(fp)) {
		case 'E':
			if (IS_TOKEN(fp, "end")) {
				if (IS_NULLSTR(soc->name)) {
					printlog(LOG_ERROR, "load_social: social name not defined");
					varr_edelete(&socials, soc);
				}
				return;
			}
			break;

		case 'F':
			MLSKEY("found_char", soc->found_char);
			MLSKEY("found_vict", soc->found_vict);
			MLSKEY("found_notvict", soc->found_notvict);
			break;

		case 'N':
			KEY("name", soc->name, fread_sword(fp));
			MLSKEY("notfound_char", soc->notfound_char);
			MLSKEY("noarg_char", soc->noarg_char);
			MLSKEY("noarg_room", soc->noarg_room);
			break;

		case 'M':
			KEY("min_pos", soc->min_pos,
			    fread_fword(position_table, fp));
			break;

		case 'S':
			MLSKEY("self_char", soc->self_char);
			MLSKEY("self_room", soc->self_room);
			break;
		}

		if (!fMatch) {
			printlog(LOG_ERROR, "%s: %s: Unknown keyword",
			    __FUNCTION__, rfile_tok(fp));
			fread_to_eol(fp);
		}
	}
}
