/*-
 * Copyright (c) 2001 SoG Development Team
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
 * $Id: init_sog.c,v 1.6 2001-08-21 09:33:23 fjoe Exp $
 */

#include <stdio.h>
#include <stdarg.h>
#include <dlfcn.h>

#include <merc.h>

#include <module.h>
#define MODULE_INIT MOD_SOG
#include <sog.h>

#include "handler_impl.h"

DECLARE_MODINIT_FUN(_module_load);
DECLARE_MODINIT_FUN(_module_unload);
DECLARE_MODINIT_FUN(_module_boot);

static char_logger_t old_logger;

MODINIT_FUN(_module_load, m)
{
	run_game = dlsym(m->dlh, "_run_game");			// notrans
	if (run_game == NULL) {
		log(LOG_INFO, "_module_load(mod_sog): %s", dlerror());
		return -1;
	}

	run_game_bottom = dlsym(m->dlh, "_run_game_bottom");	// notrans
	if (run_game_bottom == NULL) {
		log(LOG_INFO, "_module_load(mod_sog): %s", dlerror());
		return -1;
	}

	dynafun_tab_register(__mod_tab(MODULE), m);
	old_logger = char_logger_set(act_char_logger);
	return 0;
}

extern bool do_longjmp;

MODINIT_FUN(_module_unload, m)
{
	char_logger_set(old_logger);
	dynafun_tab_unregister(__mod_tab(MODULE));

	do_longjmp = TRUE;
	run_game_bottom = NULL;
	run_game = NULL;
	return 0;
}

MODINIT_FUN(_module_boot, m)
{
	load_bans();
	return 0;
}
