/*-
 * Copyright (c) 1999, 2000 SoG Development Team
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
 * $Id: update_impl.c,v 1.9 2001-09-13 16:22:18 fjoe Exp $
 */

#include <stdlib.h>
#include <stdio.h>

#include <merc.h>

#include <sog.h>

#define MODULE_INIT MOD_UPDATE
#include <update.h>
#include "update_impl.h"

avltree_t uhandlers;

void
update_register(module_t *m)
{
	dynafun_tab_register(__mod_tab(MODULE), m);
}

void
update_unregister()
{
	dynafun_tab_unregister(__mod_tab(MODULE));
}

void
uhandler_update(uhandler_t *hdlr)
{
        if (hdlr->fun == NULL) {
		log(LOG_INFO, "uhandler_update: %s: NULL update fun",
		    hdlr->name);
		return;
	}

	if (hdlr->iter == NULL)
		((update_fun_t) hdlr->fun)();
	else
		vo_foreach(NULL, hdlr->iter, hdlr->fun);

	if (!IS_NULLSTR(hdlr->notify))
		wiznet(hdlr->notify, NULL, NULL, WIZ_TICKS, 0, 0);
}
