/*-
 * Copyright (c) 1999 fjoe <fjoe@iclub.nsu.ru>
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
 * $Id: cc_expr.c,v 1.1 1999-11-22 14:54:24 fjoe Exp $
 */

#include <stdio.h>
#include <dlfcn.h>

#include "typedef.h"
#include "varr.h"
#include "cc_rule.h"
#include "log.h"

#include "module.h"

static void *load_cb(void *p, va_list ap);
static void *unload_cb(void *p, va_list ap);

int _module_load(module_t* m)
{
	varr_foreach(&cc_rulecls, load_cb, m);
	return 0;
}

int _module_unload(module_t *m)
{
	varr_foreach(&cc_rulecls, unload_cb);
	return 0;
}

static void *
rfun_load_cb(void *p, va_list ap)
{
	cc_rulefun_t *rfun = (cc_rulefun_t *) p;

	module_t *m = va_arg(ap, module_t *);

	rfun->fun = dlsym(m->dlh, rfun->fun_name);
	if (rfun->fun == NULL) 
		wizlog("_module_load(mod_cc_rulefun): %s", dlerror());
	return NULL;
}

static void *
rfun_unload_cb(void *p, va_list ap)
{
	cc_rulefun_t *rfun = (cc_rulefun_t *) p;

	rfun->fun = NULL;
	return NULL;
}

static void *
load_cb(void *p, va_list ap)
{
	cc_rulecl_t *rcl = (cc_rulecl_t *) p;

	module_t *m = va_arg(ap, module_t *);

	varr_foreach(&rcl->rulefuns, rfun_load_cb, m);
	return NULL;
}

static void *
unload_cb(void *p, va_list ap)
{
	cc_rulecl_t *rcl = (cc_rulecl_t *) p;
	varr_foreach(&rcl->rulefuns, rfun_unload_cb);
	return NULL;
}