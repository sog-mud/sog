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
 * $Id: update.c,v 1.213 2004-06-28 19:21:08 tatyana Exp $
 */

#include <stdio.h>
#include <dlfcn.h>

#include <merc.h>
#include <module.h>

#include <sog.h>

static void uhandler_update(uhandler_t *hdlr);

void
update_handler(void)
{
	uhandler_t *hdlr;

	C_FOREACH (uhandler_t *, hdlr, &uhandlers) {
		if (--hdlr->cnt == 0) {
			hdlr->cnt = hdlr->ticks;
			uhandler_update(hdlr);
		}
	}
}

void *
update_one_handler(const char *hdlr_name)
{
	uhandler_t *hdlr = uhandler_search(hdlr_name);
	if (hdlr)
		uhandler_update(hdlr);
	return hdlr;
}

int
get_pulse(const char *hdlr_name)
{
	uhandler_t *hdlr = uhandler_lookup(hdlr_name);

	if (!hdlr) {
		printlog(LOG_BUG, "get_pulse: %s: unknown update handler",
		    hdlr_name);
		return 0;
	}

	return hdlr->ticks;
}

void
update_reset(const char *hdlr_name)
{
	uhandler_t *hdlr = uhandler_lookup(hdlr_name);
	if (!hdlr)
		return;
	hdlr->cnt = hdlr->ticks;
}

/*--------------------------------------------------------------------
 * local functions
 */

static void
uhandler_update(uhandler_t *hdlr)
{
        if (hdlr->fun == NULL) {
		printlog(LOG_INFO, "uhandler_update: %s: NULL update fun",
		    hdlr->name);
		return;
	}

	if (hdlr->iter_cl == NULL)
		((UPDATE_FUN *) hdlr->fun)();
	else {
		void *vo;

		foreach (vo, iter_new(hdlr->iter_cl, NULL)) {
			if (((UPDATE_FOREACH_FUN *) hdlr->fun)(vo))
				break;
		} end_foreach(vo);
	}

	if (!IS_NULLSTR(hdlr->notify))
		wiznet(hdlr->notify, NULL, NULL, WIZ_TICKS, 0, 0);
}
