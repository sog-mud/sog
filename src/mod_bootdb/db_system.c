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
 * $Id: db_system.c,v 1.27 2003-09-30 00:31:04 fjoe Exp $
 */

#include <stdio.h>
#include <string.h>

#include <merc.h>
#include <module.h>
#include <db.h>
#include <rwfile.h>

static void load_service(varr *v, rfile_t *fp);

DECLARE_DBLOAD_FUN(load_system);
DECLARE_DBLOAD_FUN(load_info);
DECLARE_DBLOAD_FUN(load_mudftp);

DBFUN dbfun_system[] =
{
	{ "SYSTEM",	load_system,	NULL	},		// notrans
	{ "INFO",	load_info,	NULL	},		// notrans
	{ "MUDFTP",	load_mudftp,	NULL	},		// notrans
	{ NULL, NULL, NULL }
};

DBDATA db_system = { dbfun_system, NULL, 0 };

DBLOAD_FUN(load_system)
{
	bool fListen;

	/*
	 * command line parameters override
	 * configuration settings
	 */
	fListen = c_isempty(&control_sockets);

	for (;;) {
		bool fMatch = FALSE;

		fread_keyword(fp);
		switch(rfile_tokfl(fp)) {
		case 'E':
			if (IS_TOKEN(fp, "End"))
				return;
			break;

		case 'L':
			if (IS_TOKEN(fp, "Listen")) {
				int port;

				port = fread_number(fp);
				if (fListen) {
					int *p = varr_enew(&control_sockets);
					*p = port;
				}

				fMatch = TRUE;
			}
			break;

		case 'O':
			KEY("Options", mud_options,
			    fread_fstring(options_table, fp));
			break;
		}

		if (!fMatch) {
			printlog(LOG_ERROR, "%s: %s: Unknown keyword",
			    __FUNCTION__, rfile_tok(fp));
			fread_to_eol(fp);
		}
	}
}

DBLOAD_FUN(load_info)
{
	load_service(&info_sockets, fp);
}

DBLOAD_FUN(load_mudftp)
{
	load_service(&mudftp_sockets, fp);
}

static void
load_service(varr *v, rfile_t *fp)
{
	bool fListen;

	/*
	 * command line parameters override
	 * configuration settings
	 */
	fListen = c_isempty(v);

	for (;;) {
		bool fMatch = FALSE;

		fread_keyword(fp);
		switch(rfile_tokfl(fp)) {
		case 'E':
			if (IS_TOKEN(fp, "End"))
				return;
			break;

		case 'L':
			if (IS_TOKEN(fp, "Listen")) {
				int port;

				port = fread_number(fp);
				if (fListen) {
					int *p = varr_enew(v);
					*p = port;
				}

				fMatch = TRUE;
			}
			break;
		}

		if (!fMatch) {
			printlog(LOG_ERROR, "%s: %s: Unknown keyword",
			    __FUNCTION__, rfile_tok(fp));
			fread_to_eol(fp);
		}
	}
}
