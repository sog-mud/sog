/*-
 * Copyright (c) 1998 fjoe <fjoe@iclub.nsu.ru>
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
 * $Id: msg.c,v 1.14 1999-02-11 16:40:32 fjoe Exp $
 */

#if	defined (LINUX) || defined (WIN32)
#include <string.h>
#endif

#include <limits.h>
#include <stdio.h>

#include "const.h"
#include "typedef.h"

#include "db.h"
#include "msg.h"
#include "str.h"
#include "mlstring.h"
#include "util.h"
#include "varr.h"

varr msg_hash_table[MAX_MSG_HASH];

#define msghash(s) hashstr(s, 32, MAX_MSG_HASH)
static int cmpmsg(const void*, const void*);
static int cmpmlstr(const void*, const void*);

void load_msgdb(void)
{
	int i;
	FILE *fp;
	mlstring *ml;

	snprintf(filename, sizeof(filename), "%s%c%s", ETC_PATH, PATH_SEPARATOR, MSG_FILE);
	line_number = 0;

	fp = fopen(filename, "r");
	if (fp == NULL) {
		perror(filename);
		exit(1);
	}

	for (i = 0; i < MAX_MSG_HASH; i++) {
		varr *v = msg_hash_table+i;
		v->nsize = sizeof(mlstring*);
		v->nstep = 4;
	}

	for (;;) {
		ml = mlstr_fread(fp);

		if (mlstr_null(ml)) {
			db_error("msgdb_load", "no '$' found");
			exit(1);
		}

		if (!strcmp(mlstr_mval(ml), "$")) {
			mlstr_free(ml);
			break;
		}
		msg_add(ml);
	}
}

mlstring **msg_add(mlstring *ml)
{
	mlstring **mlp;
	varr *v;
	const char *name = mlstr_mval(ml);

	if (IS_NULLSTR(name))
		return NULL;

	v = msg_hash_table+msghash(name);

	if (varr_bsearch(v, name, cmpmsg)) {
		db_error("msg_add", "%s: duplicate entry", name);
		return NULL;
	}

	mlp = varr_enew(v);
	*mlp = ml;
	varr_qsort(v, cmpmlstr);
	return varr_bsearch(v, name, cmpmsg);
}

mlstring **msg_lookup(const char *name)
{
	if (IS_NULLSTR(name))
		return NULL;

	return varr_bsearch(msg_hash_table+msghash(name), name, cmpmsg);
}

mlstring *msg_del(const char *name)
{
	varr *v;
	mlstring **mlp;
	mlstring *ml;

	v = msg_hash_table+msghash(name);

	mlp = varr_bsearch(v, name, cmpmsg);
	if (mlp == NULL)
		return NULL;
	ml = *mlp;
	*mlp = NULL;
	varr_qsort(v, cmpmlstr);
	v->nused--;
	return ml;
}

const char *GETMSG(const char *msg, int lang)
{
	mlstring **mlp = msg_lookup(msg);
	if (mlp == NULL)
		return msg;
	return mlstr_val(*mlp, lang);
}

/* reverse order (otherwise msg_del will not work) */
static int cmpmsg(const void* p1, const void* p2)
{
	return -strcmp((char*)p1, mlstr_mval(*(mlstring**)p2));
}

/* reverse order (otherwise msg_del will not work) */
static int cmpmlstr(const void* p1, const void* p2)
{
	return -strcmp(mlstr_mval(*(mlstring**)p1), mlstr_mval(*(mlstring**)p2));
}

