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
 * $Id: update_impl.h,v 1.1 2000-06-01 17:57:50 fjoe Exp $
 */

#ifndef __UPDATE_H_
#define __UPDATE_H_

#include "update.h"

extern hash_t	uhandlers;

typedef void (*update_fun_t)(void);

typedef struct uhandler_t uhandler_t;
struct uhandler_t {
	const char *name;	/* update handler name */
	const char *fun_name;	/* update handler function name */

	const char *notify;	/* update handler notification message */
				/* (printed to wiznet when handler */
				/*  is triggered) */

	int ticks;		/* ticks between uhandlers */
	vo_iter_t *iter;	/* update handler iterator */
	int mod;		/* module where update handler */
				/* implementation resides */

	int cnt;		/* current tick counter */
	void *fun;		/* update handler function */
};

#define uhandler_lookup(ln)	((uhandler_t*) strkey_lookup(&uhandlers, (ln)))
#define uhandler_search(ln)	((uhandler_t*) mlstrkey_search(&uhandlers, (ln)))

void		uhandler_init(uhandler_t *hdlr);
void		uhandler_destroy(uhandler_t *hdlr);
uhandler_t *	uhandler_cpy(uhandler_t *dest, uhandler_t *src);

void		update_register(module_t *m);
void		update_unregister(void);

void		uhandler_update(uhandler_t *hdlr);

void *		bloodthirst_cb(void *vo, va_list ap);

#endif
