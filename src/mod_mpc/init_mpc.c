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
 * $Id: init_mpc.c,v 1.12 2001-08-14 16:06:58 fjoe Exp $
 */

#include <sys/stat.h>
#include <dlfcn.h>
#include <errno.h>
#include <dirent.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#if defined(BSD44)
#	include <fnmatch.h>
#else
#	include <compat/fnmatch.h>
#endif

#include <merc.h>
#include <dynafun.h>
#include <module.h>
#include <rwfile.h>

#undef MODULE_NAME
#define MODULE_NAME MOD_MPC
#define MODULE_INIT MOD_MPC
#include <dynafun_decl.h>

#include "mpc_impl.h"
#include "mpc_const.h"
#include "mpc_iter.h"

#if !defined(MPC)
DECLARE_MODINIT_FUN(_module_load);
DECLARE_MODINIT_FUN(_module_unload);

static void load_mudprogs(void);

static dynafun_data_t local_dynafun_tab[] = {
	DECLARE_FUN4(int, has_sp,
		     ARG(CHAR_DATA), ch, ARG(cchar_t), spn,
		     ARG(cchar_t), spn_add, ARG(cchar_t), spn_rm)
	DECLARE_FUN1(int, level,
		     ARG(CHAR_DATA), ch)
	DECLARE_FUN4(int, spclass_count,
		     ARG(CHAR_DATA), ch, ARG(cchar_t), spn,
		     ARG(cchar_t), spn_add, ARG(cchar_t), spn_rm)
	NULL_DYNAFUN_DATA
};
#endif

static dynafun_data_t core_dynafun_tab[] = {
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
hashdata_t h_progs = {
	sizeof(prog_t), 4,

	(e_init_t) prog_init,
	(e_destroy_t) prog_destroy,
	NULL,

	STRKEY_HASH_SIZE,
	k_hash_str,
	ke_cmp_str
};

hash_t progs;

MODINIT_FUN(_module_load, m)
{
	if (iter_init(m) < 0)
		return -1;

	if (mpc_init() < 0)
		return -1;

	dynafun_tab_register(local_dynafun_tab, m);

	hash_init(&progs, &h_progs);
	load_mudprogs();
	return 0;
}

MODINIT_FUN(_module_unload, m)
{
	hash_destroy(&progs);

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
	"has_sp",
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

#if !defined(MPC)
static void load_mp(const char *name);

static void
load_mudprogs()
{
	struct dirent *dp;
	DIR *dirp;
	char mask[PATH_MAX];

	if ((dirp = opendir(MPC_PATH)) == NULL) {
		log(LOG_ERROR, "load_mudprogs: %s: %s",
		    MPC_PATH, strerror(errno));
		return;
	}

	snprintf(mask, sizeof(mask), "*%s", MPC_EXT);		// notrans

	for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
		if (dp->d_type != DT_REG)
			continue;

		if (fnmatch(mask, dp->d_name, 0) == FNM_NOMATCH)
			continue;

		load_mp(dp->d_name);
	}

	closedir(dirp);
}

flaginfo_t mp_types[] =
{
	{ "",			TABLE_INTVAL,		FALSE	},

	{ "mob",		MP_T_MOB,		TRUE	},
	{ "obj",		MP_T_OBJ,		TRUE	},
	{ "room",		MP_T_ROOM,		TRUE	},
	{ "spec",		MP_T_SPEC,		TRUE	},

	{ NULL, 0, FALSE }
};

static void
load_mp(const char *name)
{
	char *q;
	rfile_t *fp;
	prog_t prog;
	prog_t *p;
	struct stat s;

	if (dstat(MPC_PATH, name, &s) < 0) {
		log(LOG_ERROR, "load_mp: stat: %s: %s", name, strerror(errno));
		return;
	}

	fp = rfile_open(MPC_PATH, name);
	if (fp == NULL) {
		log(LOG_ERROR, "load_mp: fopen: %s: %s", name, strerror(errno));
		return;
	}

	/*
	 * find rightmost '.'
	 */
	q = strrchr(name, '.');
	if (q == NULL)
		q = strchr(name, '\0');

	prog_init(&prog);
	prog.name = str_ndup(name, q - name);

	/*
	 * try to find '#type'
	 */
	fread_word(fp);
	if (!!strcmp(rfile_tok(fp), "#type")) {
		log(LOG_ERROR, "load_mp: %s: missing #type directive", name);
		prog_destroy(&prog);
		goto bailout;
	}

	prog.type = fread_fword(mp_types, fp);
	fread_to_eol(fp);

	prog.textlen = s.st_size - rfile_ftello(fp);
	prog.text = malloc(s.st_size + 1);
	if (prog.text == NULL) {
		fprintf(stderr, "load_mp: malloc: %s\n", strerror(errno));
		prog_destroy(&prog);
		goto bailout;
	}

	rfile_fread((void *) (uintptr_t) prog.text, prog.textlen, 1, fp);
	((char *) (uintptr_t) prog.text)[prog.textlen] = '\0';

	if ((p = (prog_t *) hash_insert(&progs, prog.name, &prog)) == NULL) {
		fprintf(stderr, "load_mp: %s: duplicate mp", name);
		prog_destroy(&prog);
		goto bailout;
	}

	if (prog_compile(p) < 0) {
		log(LOG_INFO, "load_mp: %s (%s)",
		    p->name, flag_string(mp_types, p->type));
		fprintf(stderr, "%s", buf_string(prog.errbuf));
	}

bailout:
	rfile_close(fp);
}

#endif
