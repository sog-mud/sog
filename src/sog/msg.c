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
 * $Id: msg.c,v 1.1 1998-09-20 17:01:27 fjoe Exp $
 */

#include <stdio.h>

#include "const.h"
#include "typedef.h"

#include "db/db.h"
#include "db/msg.h"
#include "mlstring.h"
#include "util.h"
#include "varr.h"

varr *msg_hash_table[MAX_MSG_HASH];

#define msghash(s) hashstr(s, 20, MAX_MSG_HASH)
static int cmpmsg(const void*, const void*);
static int cmpmlstr(const void*, const void*);

void load_msgdb()
{
	FILE *fp;
	mlstring *ml;

	fp = dfopen(ETC_PATH, MSG_FILE, "r");
	if (fp == NULL) {
		perror(MSG_FILE);
		return;
	}

	while (1) {
		ml = mlstr_fread(fp);

		if (mlstr_null(ml)) {
			db_error("msgdb_load", "no '$' found");
			return;
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
	int hash;
	char *name = mlstr_mval(ml);

	if (IS_NULLSTR(name))
		return NULL;

	hash = msghash(name);
	v = msg_hash_table[hash];
	if (v == NULL)
		v = msg_hash_table[hash] = varr_new(sizeof(mlstring*), 4);

	if (varr_bsearch(v, name, cmpmsg)) {
		db_error("msg_add", "%s: duplicate entry", name);
		return NULL;
	}

	mlp = varr_enew(v);
	*mlp = ml;
	varr_qsort(v, cmpmlstr);
	return mlp;
}

mlstring **msg_lookup(const char *name)
{
	if (IS_NULLSTR(name))
		return NULL;

	return varr_bsearch(msg_hash_table[msghash(name)], name, cmpmsg);
}

mlstring *msg_del(const char *name)
{
	int hash;
	varr *v;
	mlstring **mlp;
	mlstring *ml;

	hash = msghash(name);
	v = msg_hash_table[hash];
	if (v == NULL)
		return NULL;

	mlp = varr_bsearch(v, name, cmpmsg);
	if (mlp == NULL)
		return NULL;
	ml = *mlp;
	*mlp = NULL;
	varr_qsort(v, cmpmlstr);
	v->nused--;
	return ml;
}

const char *MSG(const char *msg, int lang)
{
	mlstring **mlp = msg_lookup(msg);
	if (mlp == NULL)
		return msg;
	return mlstr_val(*mlp, lang);
}

static int cmpmsg(const void* p1, const void* p2)
{
	return strcmp((char*)p1, mlstr_mval(*(mlstring**)p2));
}

static int cmpmlstr(const void* p1, const void* p2)
{
	return strcmp(mlstr_mval(*(mlstring**)p1), mlstr_mval(*(mlstring**)p2));
}

