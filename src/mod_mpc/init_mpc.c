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
 * $Id: init_mpc.c,v 1.1 2001-06-25 16:51:20 fjoe Exp $
 */

#include <setjmp.h>

#include <typedef.h>
#include <memalloc.h>
#include <log.h>

#include "_mpc.h"
#include "mpc_const.h"

#if !defined(MPC)
int
_module_load(module_t *m)
{
	mpc_init();
	return 0;
}

int
_module_unload(module_t *m)
{
	return 0;
}
#endif

const char *mpc_dynafuns[] = {
	"number_range",
	"print",
	"print2",
	"prints",
	"nonexistent",
	NULL
};

void
mpc_init()
{
	int_const_t *ic;
	const char **pp;
#if defined(MPC)
	module_t m;
#endif

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

	m.dlh = dlopen(NULL, 0);
	if (m.dlh == NULL) {
		fprintf(stderr, "dlopen: %s", dlerror());
		exit(1);
	}

	dynafun_tab_register(mpc_dynafun_tab, &m);
#endif
}

