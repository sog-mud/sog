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
 * $Id: init_cc_fun.c,v 1.4 2000-02-10 14:08:42 fjoe Exp $
 */

#include <stdarg.h>
#include <stdio.h>
#include <dlfcn.h>

#include "typedef.h"
#include "varr.h"
#include "cc_expr.h"
#include "log.h"

#include "module.h"

static void *load_cb(void *p, va_list ap);
static void *unload_cb(void *p, va_list ap);

int _module_load(module_t* m)
{
	varr_foreach(&cc_eclasses, load_cb, m);
	return 0;
}

int _module_unload(module_t *m)
{
	varr_foreach(&cc_eclasses, unload_cb);
	return 0;
}

static void *
efun_load_cb(void *p, va_list ap)
{
	cc_efun_t *efun = (cc_efun_t *) p;

	module_t *m = va_arg(ap, module_t *);

	efun->fun = dlsym(m->dlh, efun->fun_name);
	if (efun->fun == NULL) 
		log(LOG_ERROR, "_module_load(mod_cc_efun): %s", dlerror());
	return NULL;
}

static void *
efun_unload_cb(void *p, va_list ap)
{
	cc_efun_t *efun = (cc_efun_t *) p;

	efun->fun = NULL;
	return NULL;
}

static void *
load_cb(void *p, va_list ap)
{
	cc_eclass_t *rcl = (cc_eclass_t *) p;

	module_t *m = va_arg(ap, module_t *);

	varr_foreach(&rcl->efuns, efun_load_cb, m);
	return NULL;
}

static void *
unload_cb(void *p, va_list ap)
{
	cc_eclass_t *rcl = (cc_eclass_t *) p;
	varr_foreach(&rcl->efuns, efun_unload_cb);
	return NULL;
}
