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
 * $Id: init_mpc.c,v 1.19 2001-08-30 18:50:12 fjoe Exp $
 */

#include <dlfcn.h>
#include <errno.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#include <merc.h>
#include <dynafun.h>
#include <module.h>
#include <rwfile.h>
#include <mprog.h>

#include "mpc_impl.h"
#include "mpc_const.h"
#include "mpc_iter.h"

#if !defined(MPC)
#define MODULE_INIT MOD_MPC
#include "mpc_dynafun.h"

DECLARE_MODINIT_FUN(_module_load);
DECLARE_MODINIT_FUN(_module_unload);

#endif

static dynafun_data_t core_dynafun_tab[] = {
	DECLARE_PROC3(dofun,
		      ARG(cchar_t), name, ARG(CHAR_DATA), ch,
		      ARG(cchar_t), argument)
	DECLARE_FUN1(int, number_bits,
		     ARG(int), width)
	DECLARE_FUN0(int, number_percent)
	DECLARE_FUN2(int, number_range,
		     ARG(int), from, ARG(int), to)
#if defined(MPC)
	{ "print",		MT_VOID, 1,
	  { { MT_INT } }						},
	{ "print2",		MT_VOID, 2,
	  { { MT_INT }, { MT_INT } }					},
	{ "prints",		MT_VOID, 1,
	  { { MT_STR } }						},
	{ "nonexistent",	MT_VOID, 0				},
#endif
	NULL_DYNAFUN_DATA
};

#if !defined(MPC)
hash_t mpcodes;

hashdata_t h_mpcodes = {
	sizeof(mpcode_t), 4,

	(e_init_t) mpcode_init,
	(e_destroy_t) mpcode_destroy,
	NULL,

	STRKEY_HASH_SIZE,
	k_hash_csstr,
	ke_cmp_csstr
};

static
FOREACH_CB_FUN(compile_mprog_cb, p, ap)
{
	mprog_t *mp = (mprog_t *) p;

	if (mprog_compile(mp) < 0) {
		log(LOG_INFO, "load_mprog: %s (%s)",
		    mp->name, flag_string(mprog_types, mp->type));
		fprintf(stderr, "%s", buf_string(mp->errbuf));
	}

	return NULL;
}

MODINIT_FUN(_module_load, m)
{
	mprog_compile = dlsym(m->dlh, "_mprog_compile");	// notrans
	if (mprog_compile == NULL) {
		log(LOG_INFO, "_module_load(mod_mpc): %s", dlerror());
		return -1;
	}

	mprog_execute = dlsym(m->dlh, "_mprog_execute");	// notrans
	if (mprog_execute == NULL) {
		log(LOG_INFO, "_module_load(mod_mpc): %s", dlerror());
		return -1;
	}

	if (iter_init(m) < 0)
		return -1;

	if (mpc_init() < 0)
		return -1;

	dynafun_tab_register(__mod_tab(MODULE), m);

	hash_init(&mpcodes, &h_mpcodes);
	hash_foreach(&mprogs, compile_mprog_cb);

	return 0;
}

MODINIT_FUN(_module_unload, m)
{
	mprog_compile = NULL;
	mprog_execute = NULL;

	hash_destroy(&mpcodes);

	dynafun_tab_unregister(__mod_tab(MODULE));
	mpc_fini();
	return 0;
}
#endif

/*
 * keep this alphabetically sorted
 */
const char *mpc_dynafuns[] = {
#if !defined(MPC)
	"act",
	"act_char",
	"act_yell",
	"can_see",
	"dofun",
	"has_sp",
	"is_immortal",
	"level",
	"mob_interpret",
	"number_bits",
	"number_percent",
	"number_range",
	"spclass_count",
	"tell_char",
#else
	"nonexistent",
	"number_range",
	"print",
	"print2",
	"prints",
#endif
	NULL
};

int
mpc_init(void)
{
	int_const_t *ic;
	const char **pp;
	module_t m;

	hash_init(&glob_syms, &h_syms);

	/*
	 * add consts to global symbol table
	 */
	for (ic = ic_tab; ic->name != NULL; ic++) {
		const void *p;
		sym_t sym;

		sym.name = str_dup(ic->name);
		sym.type = SYM_VAR;
		sym.s.var.type_tag = ic->type_tag;
		sym.s.var.data.i = ic->value;
		sym.s.var.is_const = TRUE;

		if ((p = hash_insert(&glob_syms, sym.name, &sym)) == NULL) {
			log(LOG_ERROR, "%s: duplicate symbol (const)",
			    sym.name);
		}
		sym_destroy(&sym);
	}

	/*
	 * add dynafuns to global symbol table
	 */
	for (pp = mpc_dynafuns; *pp != NULL; pp++) {
		const void *p;
		sym_t sym;

		sym.name = str_dup(*pp);
		sym.type = SYM_FUNC;

		if ((p = hash_insert(&glob_syms, sym.name, &sym)) == NULL) {
			log(LOG_ERROR, "%s: duplicate symbol (func)",
			    sym.name);
		}
		sym_destroy(&sym);
	}

#if defined(MPC)
	init_dynafuns();
#endif

	m.dlh = dlopen(NULL, 0);
	if (m.dlh == NULL) {
#if defined(MPC)
		fprintf(stderr, "dlopen: %s", dlerror());
		exit(1);
#else
		log(LOG_ERROR, "%s: dlopen: %s", __FUNCTION__, dlerror());
		return -1;
#endif
	}

	dynafun_tab_register(core_dynafun_tab, &m);
	return 0;
}

void
mpc_fini()
{
	dynafun_tab_unregister(core_dynafun_tab);
}
