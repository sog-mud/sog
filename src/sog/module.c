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
 * $Id: module.c,v 1.35 2003-09-30 00:31:38 fjoe Exp $
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

static int	modset_add	(varr *v, module_t *m, time_t curr_time);
static module_t *modset_search	(varr *v, const char *name);
static int	modset_elem_cmp	(const void *, const void *);
static int	module_cmp	(const void *p, const void *q);

static varr_info_t c_info_modset = {
	&varr_ops, NULL, NULL,

	sizeof(module_t *), 4
};

extern bool do_longjmp;
extern jmp_buf game_loop_jmpbuf;

int
mod_reload(module_t *m, time_t curr_time)
{
	varr v;
	module_t **mp;

	c_init(&v, &c_info_modset);

	if (modset_add(&v, m, curr_time) < 0) {
		c_destroy(&v);
		return -1;
	}

	varr_qsort(&v, modset_elem_cmp);

	VARR_RFOREACH(mp, &v) {
		MODINIT_FUN *callback;

		if ((*mp)->dlh == NULL)
			continue;

		if ((callback = dlsym((*mp)->dlh, "_module_unload")) != NULL // notrans
		&&  callback(*mp) < 0)
			continue;

		dlclose((*mp)->dlh);
		(*mp)->dlh = NULL;
	}

	C_FOREACH(mp, &v) {
		MODINIT_FUN *callback;
		void *dlh;
		const char *_depend;

		if ((*mp)->dlh != NULL)
			continue;

		/*
		 * open .so
		 */
		dlh = dlopen((*mp)->file_name, RTLD_NOW);
		if (dlh == NULL) {
			printlog(LOG_ERROR, "mod_load: %s", dlerror());
			continue;
		}

		/*
		 * call on-load callback.
		 * (*mp)->dlh should be set before
		 */
		(*mp)->dlh = dlh;
		if ((callback = dlsym((*mp)->dlh, "_module_load")) != NULL // notrans
		&&  callback((*mp)) < 0) {
			dlclose((*mp)->dlh);
			(*mp)->dlh = NULL;
			continue;
		}

		/*
		 * update dependencies
		 */
		free_string((*mp)->mod_deps);
		_depend = dlsym((*mp)->dlh, "_depend");		// notrans
		(*mp)->mod_deps = str_dup(_depend);

		/*
		 * update `last_reload' time
		 */
		time(&(*mp)->last_reload);
		printlog(LOG_INFO, "module `%s' (%s) loaded",
		    (*mp)->name, (*mp)->file_name);
	}

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

	C_FOREACH(m_dep, &modules) {
		/*
		 * skip self and not loaded modules
		 */
		if (m_dep->dlh == NULL
		||  !str_cmp(m_dep->name, m->name))
			continue;

		if (is_name_strict(m->name, m_dep->mod_deps)) {
			printlog(LOG_ERROR, "module `%s' (%s) can't be unloaded: module `%s' (%s) depends on it",
			    m->name, m->file_name,
			    m_dep->name, m_dep->file_name);
			return -1;
		}
	}

	dlclose(m->dlh);
	m->dlh = NULL;
	printlog(LOG_INFO, "module `%s' (%s) unloaded", m->name, m->file_name);
	return 0;
}

module_t *
mod_lookup(const char *name)
{
	module_t *m;

	C_FOREACH(m, &modules) {
		if (!str_prefix(name, m->name))
			return m;
	}

	return NULL;
}

static varr_info_t c_info_modules = {
	&varr_ops, NULL, NULL,

	sizeof(module_t), 2
};

void
boot_modules()
{
	time_t curr_time;
	FILE *fp;
	char buf[MAX_INPUT_LENGTH];
	module_t *m;

	c_init(&modules, &c_info_modules);

	/*
	 * read modules
	 */
	fp = dfopen(ETC_PATH, MODULES_CONF, "r");
	if (fp == NULL) {
		printlog(LOG_ERROR, "%s%c%s: %s",
		    ETC_PATH, PATH_SEPARATOR, MODULES_CONF, strerror(errno));
		exit(1);
	}

	while (fgets(buf, sizeof(buf), fp)) {
		const char *p;
		char arg[MAX_INPUT_LENGTH];
		int mod_prio;
		size_t len;

		/* strip trailing '\n' */
		len = strlen(buf);
		if (len == 0)
			continue;
		if (buf[len-1] != '\n') {
			printlog(LOG_INFO, "%s%c%s: line too long",
			    ETC_PATH, PATH_SEPARATOR, MODULES_CONF);
			continue;
		}

		buf[len-1] = '\0';

		/* read module prio, skip comments and empty lines */
		p = first_arg(buf, arg, sizeof(arg), FALSE);
		if (arg[0] == '#' || arg[0] == '\0')
			continue;

		if (!is_number(arg)) {
			printlog(LOG_ERROR, "%s%c%s: priority expected",
			    ETC_PATH, PATH_SEPARATOR, MODULES_CONF);
			exit(1);
		}

		mod_prio = atoi(arg);

		/* read module name */
		p = first_arg(p, arg, sizeof(arg), FALSE);
		if (arg[0] == '\0') {
			printlog(LOG_ERROR, "%s%c%s: module name expected",
			    ETC_PATH, PATH_SEPARATOR, MODULES_CONF);
			exit(1);
		}

		if (p[0] != '\0') {
			printlog(LOG_INFO, "%s%c%s: extra characters on line",
			    ETC_PATH, PATH_SEPARATOR, MODULES_CONF);
		}

		/* add module */
		m = varr_enew(&modules);
		m->mod_prio = mod_prio;
		m->name = str_dup(arg);
		m->mod_id = flag_value(module_names, m->name);
		if (m->mod_id < 0) {
			printlog(LOG_ERROR, "%s%c%s: %s: unknown module",
			    ETC_PATH, PATH_SEPARATOR, MODULES_CONF, m->name);
		}
		m->file_name = str_printf("%s%c%s.so.%d",
		    MODULES_PATH, PATH_SEPARATOR, m->name, ABI_VERSION);

		printlog(LOG_INFO, "module %s (%d)", m->name, m->mod_prio);
	}

	fclose(fp);

	/*
	 * load modules and call boot callbacks
	 */
	time(&curr_time);
	init_dynafuns();

	varr_qsort(&modules, module_cmp);

	/*
	 * load all modules
	 */
	C_FOREACH(m, &modules) {
		if (mod_reload(m, curr_time) < 0)
			exit(1);
	}

	/*
	 * call module initializers
	 */
	C_FOREACH(m, &modules) {
		MODINIT_FUN *callback;

		if (m->dlh == NULL
		||  (callback = dlsym(m->dlh, "_module_boot")) == NULL) // notrans
			continue;

		if (callback(m) < 0)
			exit(1);
	}
}

/*--------------------------------------------------------------------
 * static functions
 */

static int
modset_add(varr *v, module_t *m, time_t curr_time)
{
	struct stat s;
	void *dlh;
	module_t **mp;
	module_t *m_dep;
	char buf[PATH_MAX];

	/*
	 * sanity checking
	 */
	if (stat(m->file_name, &s) < 0) {
		printlog(LOG_ERROR, "mod_load: %s: %s", m->file_name, strerror(errno));
		return -1;
	}

	snprintf(buf, sizeof(buf), "%s~", m->file_name);	// notrans
	if (!stat(buf, &s))
		unlink(buf);
	if (link(m->file_name, buf) < 0) {
		printlog(LOG_ERROR, "mod_load: %s: %s", buf, strerror(errno));
		return -1;
	}

	dlh = dlopen(buf, RTLD_NOW);
	unlink(buf);
	if (dlh == NULL) {
		printlog(LOG_ERROR, "mod_load: %s", dlerror());
		return -1;
	}

	if (dlsym(dlh, "_depend") == NULL) {			// notrans
		dlclose(dlh);
		printlog(LOG_ERROR, "mod_load: %s", dlerror());
		return -1;
	}

	dlclose(dlh);

	mp = (module_t **) varr_enew(v);
	*mp = m;

	C_FOREACH(m_dep, &modules) {
		if (is_name_strict(m->name, m_dep->mod_deps)
		&&  !modset_search(v, m_dep->name)
		&&  m_dep->dlh != NULL
		&&  m_dep->last_reload < curr_time
		&&  modset_add(v, m_dep, curr_time) < 0)
			return -1;
	}

	return 0;
}

static module_t *
modset_search(varr *v, const char *name)
{
	module_t **mp;

	C_FOREACH(mp, v) {
		if (!str_cmp(name, (*mp)->name))
			return *mp;
	}

	return NULL;
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
