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
 * $Id: module.c,v 1.5 1999-06-29 10:57:07 fjoe Exp $
 */

/*
 * .so libraries support functions
 */

#include <sys/param.h>
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>

#include "merc.h"
#include "module.h"
#include "version.h"
#include "log.h"

varr modules = { sizeof(module_t), 2 };

int mod_load(module_t* m)
{
	void *dlh;
	int* abi_ver;
	int (*callback)(module_t*);

	/*
	 * unload previously loaded module
	 */
	if (m->dlh != NULL) {
		if ((callback = dlsym(m->dlh, "_module_unload")) != NULL
		&&  callback(m) < 0)
			return -1;

		dlclose(m->dlh);
		m->dlh = NULL;
	}

	/*
	 * open .so and check its version
	 */
	dlh = dlopen(m->file_name, RTLD_NOW);
	if (dlh == NULL) {
		wizlog("mod_load: %s", dlerror());
		return -1;
	}

	abi_ver = dlsym(dlh, "_abi_version");
	if (abi_ver == NULL) {
		wizlog("mod_load: %s: %s", m->file_name, dlerror());
		dlclose(dlh);
		return -1;
	}

	if (*abi_ver != ABI_VERSION) {
		wizlog("mod_load: %s: incorrect version %d.%d, "
		       "current version %d.%d",
		       m->file_name,
		       VERSION_HI(*abi_ver), VERSION_LO(*abi_ver),
		       VERSION_HI(ABI_VERSION), VERSION_LO(ABI_VERSION));
		dlclose(dlh);
		return -1;
	}

	m->dlh = dlh;

	if ((callback = dlsym(m->dlh, "_module_load")) != NULL
	&&  callback(m) < 0) {
		dlclose(dlh);
		m->dlh = NULL;
		return -1;
	}

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
