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
 * $Id: module.c,v 1.13 2000-01-06 02:45:38 fjoe Exp $
 */

/*
 * .so libraries support functions
 */

#include <sys/param.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <time.h>
#include <unistd.h>
#include "merc.h"
#include "module.h"
#include "log.h"

varr modules;

int mod_load(module_t* m)
{
	struct stat s;
	void *dlh;
	int (*callback)(module_t*);
	char buf[PATH_MAX];

	/*
	 * sanity checking
	 */
	if (stat(m->file_name, &s) < 0) {
		wizlog("mod_load: %s: %s", m->file_name, strerror(errno));
		return -1;
	}

	snprintf(buf, sizeof(buf), "%s~", m->file_name);
	if (!stat(buf, &s))
		unlink(buf);
	if (link(m->file_name, buf) < 0) {
		wizlog("mod_load: %s: %s", buf, strerror(errno));
		return -1;
	}

	dlh = dlopen(buf, RTLD_NOW);
	unlink(buf);
	if (dlh == NULL) {
		wizlog("mod_load: %s", dlerror());
		return -1;
	}
	dlclose(dlh);

	/*
	 * try to unload previously loaded module
	 */
	if (m->dlh != NULL) {
		if ((callback = dlsym(m->dlh, "_module_unload")) != NULL
		&&  callback(m) < 0)
			return -1;

		dlclose(m->dlh);
		m->dlh = NULL;
	}

	/*
	 * open .so
	 */
	dlh = dlopen(m->file_name, RTLD_NOW);
	if (dlh == NULL) {
		wizlog("mod_load: %s", dlerror());
		return -1;
	}

	/*
	 * call on-load callback.
	 * note that m->dlh should be set before
	 */
	m->dlh = dlh;

	if ((callback = dlsym(m->dlh, "_module_load")) != NULL
	&&  callback(m) < 0) {
		dlclose(m->dlh);
		m->dlh = NULL;
		return -1;
	}

	time(&m->load_time);
	wizlog("mod_load: loaded module `%s' (%s)", m->name, m->file_name);
	return 0;
}

module_t *mod_lookup(const char *name)
{
	int i;

	for (i = 0; i < modules.nused; i++) {
		module_t *m = VARR_GET(&modules, i);
		if (!str_prefix(name, m->name))
			return m;
	}

	return NULL;
}
