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
 * $Id: init_mpc.c,v 1.3 2001-07-04 19:21:18 fjoe Exp $
 */

#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <dlfcn.h>

#include <typedef.h>
#include <memalloc.h>
#include <log.h>
#include <varr.h>
#include <hash.h>
#include <dynafun.h>
#include <module.h>

#include "_mpc.h"
#include "mpc_const.h"

#if !defined(MPC)
static dynafun_data_t local_dynafun_tab[] = {
	{ "act_char",		MT_VOID, 2,	{ MT_CHAR, MT_STR }	},
	{ "has_spec",		MT_INT, 4,
	  { MT_CHAR, MT_STR, MT_STR, MT_STR }				},
	{ "level",		MT_INT, 1,	{ MT_CHAR }		},
	{ "spclass_count",	MT_INT, 4,
	  { MT_CHAR, MT_STR, MT_STR, MT_STR }				},
	{ NULL }
};
#endif

static dynafun_data_t core_dynafun_tab[] = {
	{ "number_range",	MT_INT, 2,	{ MT_INT, MT_INT }	},
#if defined(MPC)
	{ "print",		MT_VOID, 1,	{ MT_INT }		},
	{ "print2",		MT_VOID, 2,	{ MT_INT, MT_INT }	},
	{ "prints",		MT_VOID, 1,	{ MT_STR }		},
	{ "nonexistent",	MT_VOID, 0				},
#endif
	{ NULL }
};

#if !defined(MPC)
int
_module_load(module_t *m)
{
	mpc_init();
	dynafun_tab_register(local_dynafun_tab, m);
	return 0;
}

int
_module_unload(module_t *m)
{
	dynafun_tab_unregister(local_dynafun_tab);
	mpc_fini();
	return 0;
}
#endif

/*
 * keep this alphabetically sorted
 */
const char *mpc_dynafuns[] = {
#if !defined(MPC)
	"act_char",
	"has_spec",
	"level",
	"number_range",
	"spclass_count",
#else
	"nonexistent",
	"number_range",
	"print",
	"print2",
	"prints",
#endif
	NULL
};

void
mpc_init()
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
		sym.s.var.type_tag = MT_INT;
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
		return;
#endif

	}

	dynafun_tab_register(core_dynafun_tab, &m);
}

void
mpc_fini()
{
	dynafun_tab_unregister(core_dynafun_tab);
}
