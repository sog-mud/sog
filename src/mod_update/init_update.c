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
 * $Id: init_update.c,v 1.19 2003-09-30 00:31:35 fjoe Exp $
 */

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

static void
uhandler_init(uhandler_t *hdlr)
{
	hdlr->name = str_empty;
	hdlr->fun_name = str_empty;
	hdlr->notify = str_empty;
	hdlr->ticks = 0;
	hdlr->iter_cl = NULL;
	hdlr->mod = MOD_UPDATE;
	hdlr->cnt = 0;
	hdlr->fun = NULL;
}

static void
uhandler_destroy(uhandler_t *hdlr)
{
	free_string(hdlr->name);
	free_string(hdlr->fun_name);
	free_string(hdlr->notify);
}

static avltree_info_t c_info_uhandlers = {
	&avltree_ops,

	(e_init_t) uhandler_init,
	(e_destroy_t) uhandler_destroy,

	MT_PVOID, sizeof(uhandler_t), ke_cmp_str
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
	cmd_t *cmd;

	c_init(&uhandlers, &c_info_uhandlers);
	db_load_file(&db_uhandlers, ETC_PATH, UHANDLERS_CONF);

	C_FOREACH(cmd, &commands)
		cmd_load(cmd, MODULE, m);
	uhandler_load(m->name);
	update_register(m);
	return 0;
}

MODINIT_FUN(_module_unload, m)
{
	cmd_t *cmd;

	update_unregister();
	uhandler_unload(m->name);
	C_FOREACH(cmd, &commands)
		cmd_unload(cmd, MODULE);
	c_destroy(&uhandlers);
	return 0;
}

DBLOAD_FUN(load_uhandler)
{
	uhandler_t *hdlr = NULL;

	for (;;) {
		bool fMatch = FALSE;

		fread_keyword(fp);
		switch (rfile_tokfl(fp)) {
		case 'E':
			CHECK_VAR(hdlr, "Name");

			if (IS_TOKEN(fp, "End")) {
				hdlr->cnt = number_range(1, hdlr->ticks);
				if (IS_NULLSTR(hdlr->fun_name))
					printlog(LOG_ERROR, "load_uhandler: fun name undefined");
				else if (!hdlr->ticks) {
					printlog(LOG_ERROR, "load_uhandler: ticks undefined");
					hdlr->ticks = 1;
				}
				return;
			}
			break;

		case 'F':
			CHECK_VAR(hdlr, "Name");

			SKEY("Fun", hdlr->fun_name, fread_string(fp));
			break;

		case 'I':
			CHECK_VAR(hdlr, "Name");

			KEY("IteratorClass", hdlr->iter_cl,
			    (vo_iter_class_t *) (uintptr_t) fread_fword(
				iterator_classes, fp));
			break;

		case 'M':
			CHECK_VAR(hdlr, "Name");

			KEY("Module", hdlr->mod,
			    fread_fword(module_names, fp));
			break;

		case 'N':
			SPKEY("Name", hdlr->name, fread_string(fp),
			      &uhandlers, hdlr);

			CHECK_VAR(hdlr, "Name");

			SKEY("Notify", hdlr->notify, fread_string(fp));
			break;

		case 'T':
			CHECK_VAR(hdlr, "Name");

			KEY("Ticks", hdlr->ticks, fread_number(fp));
			break;
		}

		if (!fMatch) {
			printlog(LOG_ERROR, "%s: %s: Unknown keyword",
			    __FUNCTION__, rfile_tok(fp));
			fread_to_eol(fp);
		}
	}
}
