/*-
 * Copyright (c) 2000 SoG Development Team
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
 * $Id: init_update.c,v 1.12 2001-08-13 18:23:54 fjoe Exp $
 */

#include <stdarg.h>
#include <stdio.h>
#include <dlfcn.h>

#include <merc.h>
#include <module.h>
#include <db.h>
#include <rwfile.h>

#include <update.h>
#include "update_impl.h"

DECLARE_MODINIT_FUN(_module_load);
DECLARE_MODINIT_FUN(_module_unload);

static hashdata_t h_uhandlers = {
	sizeof(uhandler_t), 4,
	(e_init_t) uhandler_init,
	(e_destroy_t) uhandler_destroy,
	(e_cpy_t) uhandler_cpy,

	STRKEY_HASH_SIZE,
	k_hash_str,
	ke_cmp_str
};

DECLARE_DBLOAD_FUN(load_uhandler);

DBFUN dbfun_uhandlers[] =
{
	{ "UHANDLER",	load_uhandler, NULL	},		// notrans
	{ NULL, NULL, NULL }
};

DBDATA db_uhandlers = { dbfun_uhandlers, NULL, 0 };

MODINIT_FUN(_module_load, m)
{
	hash_init(&uhandlers, &h_uhandlers);
	db_load_file(&db_uhandlers, ETC_PATH, UHANDLERS_CONF);

	varr_foreach(&commands, cmd_load_cb, MODULE, m);
	uhandler_load(m->name);
	update_register(m);
	return 0;
}

MODINIT_FUN(_module_unload, m)
{
	update_unregister();
	uhandler_unload(m->name);
	varr_foreach(&commands, cmd_unload_cb, MODULE);
	hash_destroy(&uhandlers);
	return 0;
}

DBLOAD_FUN(load_uhandler)
{
	uhandler_t hdlr;

	uhandler_init(&hdlr);

	for (;;) {
		bool fMatch = FALSE;

		fread_keyword(fp);
		switch (rfile_tokfl(fp)) {
		case 'E':
			if (IS_TOKEN(fp, "End")) {
				hdlr.cnt = number_range(1, hdlr.ticks);
				if (IS_NULLSTR(hdlr.name))
					log(LOG_ERROR, "load_uhandler: name undefined");
				else if (IS_NULLSTR(hdlr.fun_name))
					log(LOG_ERROR, "load_uhandler: fun name undefined");
				else if (!hdlr.ticks)
					log(LOG_ERROR, "load_uhandler: ticks undefined");
				else if (!hash_insert(&uhandlers, hdlr.name, &hdlr))
					log(LOG_ERROR, "load_uhandler: %s: duplicate uhandler name", hdlr.name);
				uhandler_destroy(&hdlr);
				return;
			}
			break;

		case 'F':
			SKEY("Fun", hdlr.fun_name, fread_string(fp));
			break;

		case 'I':
			KEY("Iterator", hdlr.iter,
			    (vo_iter_t *) (uintptr_t) fread_fword(
				iterator_names, fp));
			break;

		case 'M':
			KEY("Module", hdlr.mod,
			    fread_fword(module_names, fp));
			break;

		case 'N':
			SKEY("Name", hdlr.name, fread_string(fp));
			SKEY("Notify", hdlr.notify, fread_string(fp));
			break;

		case 'T':
			KEY("Ticks", hdlr.ticks, fread_number(fp));
			break;
		}

		if (!fMatch) {
			log(LOG_ERROR, "load_uhandler: %s: Unknown keyword",
			    rfile_tok(fp));
			fread_to_eol(fp);
		}
	}
}
