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
 * $Id: module.c,v 1.29 2001-09-12 19:43:18 fjoe Exp $
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
#include <setjmp.h>
#include <unistd.h>

#include <merc.h>
#include <module.h>
#include <log.h>
#include <abi_version.h>
#include <dynafun.h>

varr modules;

static DECLARE_FOREACH_CB_FUN(mod_load_cb);
static DECLARE_FOREACH_CB_FUN(mod_unload_cb);
static DECLARE_FOREACH_CB_FUN(mod_lookup_cb);

static DECLARE_FOREACH_CB_FUN(boot_load_cb);
static DECLARE_FOREACH_CB_FUN(boot_cb);

static DECLARE_FOREACH_CB_FUN(checkdep_cb);

static int	modset_add	(varr *v, module_t *m, time_t curr_time);
static module_t *modset_search	(varr *v, const char *name);
static int	modset_elem_cmp	(const void *, const void *);
static int	module_cmp	(const void *p, const void *q);

static varrdata_t v_modset = {
	&varr_ops,

	sizeof(module_t*), 4,

	NULL,
	NULL,
	NULL
};

extern bool do_longjmp;
extern jmp_buf game_loop_jmpbuf;

int
mod_reload(module_t* m, time_t curr_time)
{
	varr v;

	c_init(&v, &v_modset);

	if (modset_add(&v, m, curr_time) < 0) {
		c_destroy(&v);
		return -1;
	}

	varr_qsort(&v, modset_elem_cmp);
	varr_rforeach(&v, mod_unload_cb);
	c_foreach(&v, mod_load_cb);

	c_destroy(&v);

	if (do_longjmp) {
		do_longjmp = FALSE;
		log_unsetchar();

		longjmp(game_loop_jmpbuf, 1);
	}

	return 0;
}

int
mod_unload(module_t *m)
{
	module_t *m_dep;

	if ((m_dep = c_foreach(&modules, checkdep_cb, m->name)) != NULL) {
		log(LOG_ERROR, "modules `%s' (%s) can't be unloaded: module `%s' (%s) depends on it",
		    m->name, m->file_name, m_dep->name, m_dep->file_name);
		return -1;
	}

	dlclose(m->dlh);
	m->dlh = NULL;
	log(LOG_INFO, "module `%s' (%s) unloaded", m->name, m->file_name);
	return 0;
}

module_t *
mod_lookup(const char *name)
{
	return c_foreach(&modules, mod_lookup_cb, name);
}

static varrdata_t v_modules = {
	&varr_ops,

	sizeof(module_t), 2,

	NULL, NULL, NULL
};

void
boot_modules()
{
	time_t curr_time;
	FILE *fp;
	char buf[MAX_INPUT_LENGTH];

	c_init(&modules, &v_modules);

	/*
	 * read modules
	 */
	fp = dfopen(ETC_PATH, MODULES_CONF, "r");
	if (fp == NULL) {
		log(LOG_ERROR, "%s%c%s: %s",
		    ETC_PATH, PATH_SEPARATOR, MODULES_CONF, strerror(errno));
		exit(1);
	}

	while (fgets(buf, sizeof(buf), fp)) {
		const char *p;
		char arg[MAX_INPUT_LENGTH];
		int mod_prio;
		size_t len;
		module_t *m;

		/* strip trailing '\n' */
		len = strlen(buf);
		if (len == 0)
			continue;
		if (buf[len-1] != '\n') {
			log(LOG_INFO, "%s%c%s: line too long",
			    ETC_PATH, PATH_SEPARATOR, MODULES_CONF);
			continue;
		}

		buf[len-1] = '\0';

		/* read module prio, skip comments and empty lines */
		p = first_arg(buf, arg, sizeof(arg), FALSE);
		if (arg[0] == '#' || arg[0] == '\0')
			continue;

		if (!is_number(arg)) {
			log(LOG_ERROR, "%s%c%s: priority expected",
			    ETC_PATH, PATH_SEPARATOR, MODULES_CONF);
			exit(1);
		}

		mod_prio = atoi(arg);

		/* read module name */
		p = first_arg(p, arg, sizeof(arg), FALSE);
		if (arg[0] == '\0') {
			log(LOG_ERROR, "%s%c%s: module name expected",
			    ETC_PATH, PATH_SEPARATOR, MODULES_CONF);
			exit(1);
		}

		if (p[0] != '\0') {
			log(LOG_INFO, "%s%c%s: extra characters on line",
			    ETC_PATH, PATH_SEPARATOR, MODULES_CONF);
		}

		/* add module */
		m = varr_enew(&modules);
		m->mod_prio = mod_prio;
		m->name = str_dup(arg);
		m->mod_id = flag_value(module_names, m->name);
		if (m->mod_id < 0) {
			log(LOG_ERROR, "%s%c%s: %s: unknown module",
			    ETC_PATH, PATH_SEPARATOR, MODULES_CONF, m->name);
		}
		m->file_name = str_printf("%s%c%s.so.%d",
		    MODULES_PATH, PATH_SEPARATOR, m->name, ABI_VERSION);

		log(LOG_INFO, "module %s (%d)", m->name, m->mod_prio);
	}

	fclose(fp);

	/*
	 * load modules and call boot callbacks
	 */
	time(&curr_time);
	init_dynafuns();

	varr_qsort(&modules, module_cmp);
	c_foreach(&modules, boot_load_cb, curr_time);
	c_foreach(&modules, boot_cb);
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

	MODINIT_FUN *callback;
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
	log(LOG_INFO, "module `%s' (%s) loaded", m->name, m->file_name);
	return NULL;
}

/*
 * unload previously loaded module
 */
static
FOREACH_CB_FUN(mod_unload_cb, arg, ap)
{
	module_t *m = *(module_t **) arg;

	MODINIT_FUN *callback;

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

	if (mod_reload(m, curr_time) < 0)
		exit(1);

	return NULL;
}

static
FOREACH_CB_FUN(boot_cb, p, ap)
{
	module_t *m = (module_t *) p;

	MODINIT_FUN *callback;

	if (m->dlh == NULL
	||  (callback = dlsym(m->dlh, "_module_boot")) == NULL) // notrans
		return NULL;

	if (callback(m) < 0)
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
	if (c_foreach(&modules, modset_add_cb, v, m->name, curr_time))
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

static
FOREACH_CB_FUN(checkdep_cb, p, ap)
{
	module_t *m = (module_t *) p;

	const char *name = va_arg(ap, const char *);

	/*
	 * skip self and not loaded modules
	 */
	if (m->dlh == NULL
	||  !str_cmp(m->name, name))
		return NULL;

	if (is_name_strict(name, m->mod_deps))
		return m;

	return NULL;
}

static module_t *
modset_search(varr *v, const char *name)
{
	return c_foreach(v, modset_search_cb, name);
}

static int
modset_elem_cmp(const void *p, const void *q)
{
	const module_t *m1 = *(const module_t * const *) p;
	const module_t *m2 = *(const module_t * const *) q;

	return m2->mod_prio - m1->mod_prio;
}

static int
module_cmp(const void *p, const void *q)
{
	const module_t *m1 = (const module_t *) p;
	const module_t *m2 = (const module_t *) q;

	return m2->mod_prio - m1->mod_prio;
}
