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
 * $Id: init_magic.c,v 1.13 2001-07-30 13:05:49 fjoe Exp $
 */

#include <stdarg.h>
#include <stdio.h>
#include <dlfcn.h>

#include "typedef.h"
#include "varr.h"
#include "hash.h"
#include "mlstring.h"
#include "skills.h"
#include "log.h"
#include "memalloc.h"
#include "cmd.h"

#include "module.h"
#define MODULE_INIT MOD_MAGIC
#include "magic.h"

static void *load_cb(void *p, va_list ap);
static void *unload_cb(void *p, va_list ap);

int
_module_load(module_t* m)
{
	varr_foreach(&commands, cmd_load_cb, MODULE, m);
	hash_foreach(&skills, load_cb, m);
	dynafun_tab_register(__mod_tab(MODULE), m);
	return 0;
}

int
_module_unload(module_t *m)
{
	dynafun_tab_unregister(__mod_tab(MODULE));
	hash_foreach(&skills, unload_cb, NULL);
	varr_foreach(&commands, cmd_unload_cb, MODULE);
	return 0;
}

static void *
load_cb(void *p, va_list ap)
{
	skill_t *sk = (skill_t*) p;

	module_t *m = va_arg(ap, module_t *);

	if (sk->skill_type == ST_SPELL
	||  sk->skill_type == ST_PRAYER) {
		sk->fun = dlsym(m->dlh, sk->fun_name);
		if (sk->fun == NULL)
			log(LOG_INFO, "_module_load(magic): %s", dlerror());
	}
	return NULL;
}

static void *
unload_cb(void *p, va_list ap)
{
	skill_t *sk = (skill_t*) p;

	if (sk->skill_type == ST_SPELL
	||  sk->skill_type == ST_PRAYER)
		sk->fun = NULL;
	return NULL;
}
