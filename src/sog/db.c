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
 * $Id: db.c,v 1.2 2001-08-05 16:36:57 fjoe Exp $
 */

#include <errno.h>
#include <dirent.h>
#include <fnmatch.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(BSD44)
#	include <fnmatch.h>
#else
#	include <compat/fnmatch.h>
#endif

#ifdef SUNOS
#	include <compat/compat.h>
#	define d_namlen d_reclen
#endif

#ifdef SVR4
#	define d_namlen d_reclen
#endif

#include <typedef.h>
#include <log.h>
#include <rwfile.h>
#include <str.h>
#include <util.h>
#include <db.h>

static void dbdata_init(DBDATA *dbdata);
static DBFUN *dbfun_lookup(DBDATA *dbdata, const char *name);
static void db_parse_file(DBDATA *dbdata, const char *path, const char *file);
static void logger_db(const char *buf);

char bootdb_filename[PATH_MAX];
int bootdb_errors;

void
db_load_file(DBDATA *dbdata, const char *path, const char *file)
{
	if (!dbdata->tab_sz)
		dbdata_init(dbdata);
	if (dbdata->dbinit)
		dbdata->dbinit(dbdata);
	db_parse_file(dbdata, path, file);
}

void
db_load_dir(DBDATA *dbdata, const char *path, const char *ext)
{
	struct dirent *dp;
	DIR *dirp;
	char mask[PATH_MAX];

	if ((dirp = opendir(path)) == NULL) {
		log(LOG_ERROR, "db_load_dir: %s: %s", path, strerror(errno));
		return;
	}

	if (!dbdata->tab_sz)
		dbdata_init(dbdata);

	snprintf(mask, sizeof(mask), "*%s", ext);	// notrans

	for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
		if (dp->d_type != DT_REG)
			continue;

		if (fnmatch(mask, dp->d_name, 0) == FNM_NOMATCH)
			continue;

		if (dbdata->dbinit)
			dbdata->dbinit(dbdata);
		db_parse_file(dbdata, path, dp->d_name);
	}
	closedir(dirp);
}

void
db_load_list(DBDATA *dbdata, const char *path, const char *file)
{
	rfile_t *fp;

	if ((fp = rfile_open(path, file)) == NULL) {
		log(LOG_ERROR, "%s%c%s: %s",
		    path, PATH_SEPARATOR, file, strerror(errno));
		return;
	}

	if (!dbdata->tab_sz)
		dbdata_init(dbdata);
	for (; ;) {
		fread_word(fp);
		if (IS_TOKEN(fp, "$"))
			break;

		if (dbdata->dbinit)
			dbdata->dbinit(dbdata);
		db_parse_file(dbdata, path, rfile_tok(fp));
	}
	rfile_close(fp);
}

void
db_set_arg(DBDATA *dbdata, const char *name, void *arg)
{
	DBFUN *fun;

	if (!dbdata->tab_sz)
		dbdata_init(dbdata);

	if ((fun = dbfun_lookup(dbdata, name)) == NULL)
		return;

	fun->arg = arg;
}

/*--------------------------------------------------------------------
 * local functions
 */

static void
dbdata_init(DBDATA *dbdata)
{
	dbdata->tab_sz = 0;
	if (dbdata->dbinit)
		dbdata->dbinit(dbdata);
	while(dbdata->fun_tab[dbdata->tab_sz].name)
		dbdata->tab_sz++;
	qsort(dbdata->fun_tab, dbdata->tab_sz,
	      sizeof(*dbdata->fun_tab), cmpstr);
}

static DBFUN *
dbfun_lookup(DBDATA *dbdata, const char *name)
{
	return bsearch(&name, dbdata->fun_tab, dbdata->tab_sz,
		       sizeof(*dbdata->fun_tab), cmpstr);
}

/*
 * db_parse_file - parses file using dbdata
 * dbdata->tab_sz should be properly intialized
 */
static void
db_parse_file(DBDATA *dbdata, const char *path, const char *file)
{
	char buf[PATH_MAX];
	int linenum;
	rfile_t *fp;
	logger_t logger_old;

	strnzcpy(buf, sizeof(buf), bootdb_filename);
	linenum = line_number;
	line_number = 1;
	snprintf(bootdb_filename, sizeof(bootdb_filename), "%s%c%s",
		 path, PATH_SEPARATOR, file);

	if ((fp = rfile_open(path, file)) == NULL) {
		log(LOG_ERROR, "db_parse_file: %s", strerror(errno));
		goto bail_out;
		return;
	}

	logger_old = logger_set(LOG_ERROR, logger_db);
	for (; ;) {
		DBFUN *fn;

		if (fread_letter(fp) != '#') {
			log(LOG_ERROR, "db_parse_file: '#' not found");
			break;
		}

		fread_word(fp);
		if (IS_TOKEN(fp, "$"))
			break;

		fn = dbfun_lookup(dbdata, rfile_tok(fp));
		if (fn)
			fn->fun(dbdata, fp, fn->arg);
		else {
			log(LOG_ERROR, "db_parse_file: bad section name");
			break;
		}
	}
	logger_set(LOG_ERROR, logger_old);
	rfile_close(fp);

bail_out:
	strnzcpy(bootdb_filename, sizeof(bootdb_filename), buf);
	line_number = linenum;
}

static void
logger_db(const char *buf)
{
	char buf2[MAX_STRING_LENGTH];

	bootdb_errors++;
	snprintf(buf2, sizeof(buf2), "%s: line %d: %s",		// notrans
		 bootdb_filename, line_number, buf);
	logger_default(buf2);
}
