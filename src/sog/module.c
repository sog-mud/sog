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
 * $Id: module.c,v 1.1 1999-06-22 12:37:17 fjoe Exp $
 */

/*
 * .so libraries support functions
 */

#include <sys/param.h>
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>

#include "merc.h"
#include "dl.h"
#include "version.h"
#include "db/db.h"

static void load_spellfun(dl_t*);

dl_t dl_tab[] =
{
	{ "spellfun", load_spellfun },

	{ NULL }
};

int dl_load(dl_t *dl)
{
	void *dlh;
	int (*dl_version)(void);
	int ver;

	/*
	 * build .so filename if it was not done before
	 */
	if (dl->filename == NULL) {
		char filename[PATH_MAX];

		snprintf(filename, sizeof(filename), "%s%c%s.so",
			 DL_PATH, PATH_SEPARATOR, dl->name);
		dl->filename = str_dup(filename);
	}

	/*
	 * open .so and check its version
	 */
	dlh = dlopen(dl->filename, RTLD_LAZY);
	if (dlh == NULL) {
		db_error("dl_load", "%s: %s", dl->filename, dl_error(dlh));
		return -1;
	}

	dl_version = dlsym(dlh, "dl_version");
	if (dl_version == NULL) {
		dlclose(dlh);
		db_error("dl_load", "%s: %s", dl->filename, dl_error(dlh));
		return -1;
	}

	if ((ver = dl_version()) != DL_VERSION) {
		dl_close(dlh);
		db_error("dl_load: %s: incorrect version %d.%d, "
			 "current version %d.%d",
			 dl->filename,
			 VERSION_HI(ver), VERSION_LO(ver),
			 VERSION_HI(DL_VERSION), VERSION_LO(DL_VERSION));
		return -1;
	}

	if (dl->dlh != NULL)
		dl_close(dlh);
	dl->dlh = dlh;

	if (dl->load_callback)
		dl->load_callback(dl);
	return 0;
}

dl_t *dl_lookup(const char *name)
{
	dl_t *dl;

	for (dl = dl_tab; dl->name; dl++) {
		if (!str_prefix(name, dl->name))
			return dl;
	}

	return NULL;
}

/*
 * loader callbacks
 */
static void load_spellfun(dl_t* dl)
{
	int sn;

	for (sn = 0; sn < skills.nused; sn++) {
		skill_t *sk = SKILL(sn);

		if (sk->skill_type != ST_SPELL)
			continue;

		sk->fun = dl_sym(dl->dlh, sk->fun_name);
		if (sk->fun == NULL) {
			db_error("load_spellfun", "%s: %s",
				 sk->name, dl_error(dl->dlh));
		}
	}
}

