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
 * $Id: db.h,v 1.88 2003-10-10 16:14:09 fjoe Exp $
 */

#ifndef _BOOTDB_H_
#define _BOOTDB_H_

typedef struct dbdata DBDATA;

typedef void DBLOAD_FUN(DBDATA *dbdata, rfile_t *fp, void *arg);
#define DECLARE_DBLOAD_FUN(fun) DBLOAD_FUN fun
#define DBLOAD_FUN(fun)							\
	void fun(DBDATA *dbdata __attribute__((unused)),		\
		 rfile_t *fp, void *arg __attribute__((unused)))

typedef void DBINIT_FUN(DBDATA *dbdata);
#define DECLARE_DBINIT_FUN(fun) DBINIT_FUN fun
#define DBINIT_FUN(fun) void fun(DBDATA *dbdata)

struct dbfun {
	const char *	name;
	DBLOAD_FUN *	fun;
	void *		arg;
};
typedef struct dbfun DBFUN;

struct dbdata {
	DBFUN *		fun_tab;	/* table of parsing functions	*/
	DBINIT_FUN *	dbinit;		/* init function (called before	*/
					/* parsing each directive and	*/
					/* before parsing)		*/
	size_t		tab_sz;		/* table size			*/
};

#define DBDATA_VALID(dbdata) (!!(dbdata)->tab_sz)

void db_load_file(DBDATA *, const char *path, const char *file);
void db_load_dir(DBDATA *dbdata, const char *path, const char *ext);
void db_load_list(DBDATA *dbdata, const char *path, const char *file);
void db_set_arg(DBDATA *, const char* name, void *arg);

extern char bootdb_filename[PATH_MAX];
extern int bootdb_errors;

extern DBDATA db_areas;
extern DBDATA db_clans;
extern DBDATA db_classes;
extern DBDATA db_cmd;
extern DBDATA db_damtype;
extern DBDATA db_glob_gmlstr;
extern DBDATA db_hometowns;
extern DBDATA db_langs;
extern DBDATA db_liquids;
extern DBDATA db_materials;
extern DBDATA db_races;
extern DBDATA db_skills;
extern DBDATA db_events;
extern DBDATA db_socials;
extern DBDATA db_spec;
extern DBDATA db_system;
extern DBDATA db_forms;
extern DBDATA db_effects;
extern DBDATA db_uhandlers;

#endif /* _BOOTDB_H_ */
