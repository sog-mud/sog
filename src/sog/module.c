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
 * $Id: module.c,v 1.22 2001-07-31 14:56:24 fjoe Exp $
 */

/*
 * .so modules support functions
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

static DECLARE_FOREACH_CB(mod_load_cb);
static DECLARE_FOREACH_CB(mod_unload_cb);
static DECLARE_FOREACH_CB(mod_lookup_cb);

static DECLARE_FOREACH_CB(boot_load_cb);
static DECLARE_FOREACH_CB(boot_cb);

static int	modset_add	(varr *v, module_t *m, time_t curr_time);
static module_t *modset_search	(varr *v, const char *name);
static int	modset_elem_cmp	(const void *, const void *);
static int	module_cmp	(const void *p, const void *q);

static varrdata_t v_modset = {
	sizeof(module_t*), 4,
	NULL,
	NULL,
	NULL
};

int
mod_load(module_t* m, time_t curr_time)
{
	varr v;

	varr_init(&v, &v_modset);

	if (modset_add(&v, m, curr_time) < 0) {
		varr_destroy(&v);
		return -1;
	}

	varr_qsort(&v, modset_elem_cmp);
	varr_rforeach(&v, mod_unload_cb);
	varr_foreach(&v, mod_load_cb);

	varr_destroy(&v);
	return 0;
}

module_t *
mod_lookup(const char *name)
{
	return varr_foreach(&modules, mod_lookup_cb, name);
}

void
boot_modules()
{
	time_t curr_time;

	time(&curr_time);

	varr_qsort(&modules, module_cmp);
	varr_foreach(&modules, boot_load_cb, curr_time);
	varr_foreach(&modules, boot_cb);
}

/*--------------------------------------------------------------------
 * static functions
 */

/*
 * load module
 */
static
FOREACH_CB_FUN(mod_load_cb, arg, ap)
{
	module_t *m = *(module_t **) arg;

	int (*callback)(module_t *);
	void *dlh;
	const char *_depend;

	if (m->dlh != NULL)
		return NULL;

	/*
	 * open .so
	 */
	dlh = dlopen(m->file_name, RTLD_NOW);
	if (dlh == NULL) {
		log(LOG_ERROR, "mod_load: %s", dlerror());
		return NULL;
	}

	/*
	 * call on-load callback.
	 * m->dlh should be set before
	 */
	m->dlh = dlh;

	if ((callback = dlsym(m->dlh, "_module_load")) != NULL // notrans
	&&  callback(m) < 0) {
		dlclose(m->dlh);
		m->dlh = NULL;
		return NULL;
	}

	/*
	 * update dependencies
	 */
	free_string(m->mod_deps);
	_depend = dlsym(m->dlh, "_depend");			// notrans
	m->mod_deps = str_dup(_depend);

	/*
	 * update `last_reload' time
	 */
	time(&m->last_reload);
	log(LOG_INFO, "loaded module `%s' (%s)", m->name, m->file_name);
	return NULL;
}

/*
 * unload previously loaded module
 */
static
FOREACH_CB_FUN(mod_unload_cb, arg, ap)
{
	module_t *m = *(module_t **) arg;

	int (*callback)(module_t *);

	if (m->dlh == NULL)
		return NULL;

	if ((callback = dlsym(m->dlh, "_module_unload")) != NULL // notrans
	&&  callback(m) < 0)
		return NULL;

	dlclose(m->dlh);
	m->dlh = NULL;

	return NULL;
}

static
FOREACH_CB_FUN(mod_lookup_cb, p, ap)
{
	module_t *m = (module_t *) p;

	const char *name = va_arg(ap, const char *);

	if (!str_prefix(name, m->name))
		return m;

	return NULL;
}

static
FOREACH_CB_FUN(boot_load_cb, p, ap)
{
	module_t *m = (module_t *) p;

	time_t curr_time = va_arg(ap, time_t);

	if (mod_load(m, curr_time) < 0)
		exit(1);

	return NULL;
}

static
FOREACH_CB_FUN(boot_cb, p, ap)
{
	module_t *m = (module_t *) p;

	int (*cb)(void);

	if (m->dlh == NULL
	||  (cb = dlsym(m->dlh, "_module_boot")) == NULL) // notrans
		return NULL;

	if (cb() < 0)
		exit(1);

	return NULL;
}

static
FOREACH_CB_FUN(modset_add_cb, p, ap)
{
	module_t *m = (module_t *) p;

	varr *v = va_arg(ap, varr *);
	const char *name = va_arg(ap, const char *);
	time_t curr_time = va_arg(ap, time_t);

	if (is_name_strict(name, m->mod_deps)
	&&  !modset_search(v, m->name)
	&&  m->dlh != NULL
	&&  m->last_reload < curr_time
	&&  modset_add(v, m, curr_time) < 0)
		return m;

	return NULL;
}

static int
modset_add(varr *v, module_t *m, time_t curr_time)
{
	struct stat s;
	void *dlh;
	module_t **mp;
	char buf[PATH_MAX];

	/*
	 * sanity checking
	 */
	if (stat(m->file_name, &s) < 0) {
		log(LOG_ERROR, "mod_load: %s: %s", m->file_name, strerror(errno));
		return -1;
	}

	snprintf(buf, sizeof(buf), "%s~", m->file_name);	// notrans
	if (!stat(buf, &s))
		unlink(buf);
	if (link(m->file_name, buf) < 0) {
		log(LOG_ERROR, "mod_load: %s: %s", buf, strerror(errno));
		return -1;
	}

	dlh = dlopen(buf, RTLD_NOW);
	unlink(buf);
	if (dlh == NULL) {
		log(LOG_ERROR, "mod_load: %s", dlerror());
		return -1;
	}

	if (dlsym(dlh, "_depend") == NULL) {			// notrans
		dlclose(dlh);
		log(LOG_ERROR, "mod_load: %s", dlerror());
		return -1;
	}

	dlclose(dlh);

	mp = (module_t **) varr_enew(v);
	*mp = m;
	if (varr_foreach(&modules, modset_add_cb, v, m->name, curr_time))
		return -1;
	return 0;
}

static
FOREACH_CB_FUN(modset_search_cb, p, ap)
{
	module_t *m = *(module_t **) p;

	const char *name = va_arg(ap, const char *);

	if (!str_cmp(name, m->name))
		return m;

	return NULL;
}

static module_t *
modset_search(varr *v, const char *name)
{
	return varr_foreach(v, modset_search_cb, name);
}

static int
modset_elem_cmp(const void *p, const void *q)
{
	const module_t *m1 = *(const module_t **) p;
	const module_t *m2 = *(const module_t **) q;

	return m2->mod_prio - m1->mod_prio;
}

static int
module_cmp(const void *p, const void *q)
{
	const module_t *m1 = (const module_t *) p;
	const module_t *m2 = (const module_t *) q;

	return m2->mod_prio - m1->mod_prio;
}
