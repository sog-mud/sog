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
 * $Id: db_system.c,v 1.23 2001-09-12 19:42:45 fjoe Exp $
 */

#if !defined(WIN32)
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#else
#include <winsock.h>
#endif
#include <stdio.h>
#include <string.h>

#include <merc.h>
#include <module.h>
#include <db.h>
#include <rwfile.h>

DECLARE_DBLOAD_FUN(load_system);
DECLARE_DBLOAD_FUN(load_info);
DECLARE_DBLOAD_FUN(load_module);

DBFUN dbfun_system[] =
{
	{ "SYSTEM",	load_system,	NULL	},		// notrans
	{ "INFO",	load_info,	NULL	},		// notrans
	{ NULL, NULL, NULL }
};

DBDATA db_system = { dbfun_system, NULL, 0 };

static void fread_host(rfile_t *fp, varr *v);

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
			log(LOG_ERROR, "load_system: %s: Unknown keyword",
				 rfile_tok(fp));
			fread_to_eol(fp);
		}
	}
}

DBLOAD_FUN(load_info)
{
	bool fListen;

	/*
	 * command line parameters override
	 * configuration settings
	 */
	fListen = c_isempty(&info_sockets);

	for (;;) {
		bool fMatch = FALSE;

		fread_keyword(fp);
		switch(rfile_tokfl(fp)) {
		case 'A':
			if (IS_TOKEN(fp, "Allow")) {
				fread_host(fp, &info_trusted);
				fMatch = TRUE;
			}
			break;
		case 'E':
			if (IS_TOKEN(fp, "End"))
				return;
			break;
		case 'L':
			if (IS_TOKEN(fp, "Listen")) {
				int port;

				port = fread_number(fp);
				if (fListen) {
					int *p = varr_enew(&info_sockets);
					*p = port;
				}

				fMatch = TRUE;
			}
			break;
		}

		if (!fMatch) {
			log(LOG_ERROR, "load_info: %s: Unknown keyword",
				 rfile_tok(fp));
			fread_to_eol(fp);
		}
	}
}

static void
fread_host(rfile_t *fp, varr *v)
{
	const char *s = fread_string(fp);
	struct hostent *h = gethostbyname(s);

	free_string(s);
	if (!h)
		log(LOG_INFO, "load_info: gethostbyname: %s", hstrerror(h_errno));
	else {
		for (; *h->h_addr_list; h->h_addr_list++) {
			struct in_addr *in_addr;
			in_addr = varr_enew(v);
			memcpy(in_addr, *h->h_addr_list, sizeof(*in_addr));
			log(LOG_INFO, "load_info: added '%s' to trusted list",
				   inet_ntoa(*in_addr));
		}
	}
}
