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
 * $Id: msg.c,v 1.22 1999-12-16 07:06:56 fjoe Exp $
 */

#if	defined (LINUX) || defined (WIN32)
#include <string.h>
#endif

#include <limits.h>
#include <stdarg.h>
#include <stdio.h>

#include "const.h"
#include "typedef.h"

#include "mlstring.h"
#include "varr.h"
#include "hash.h"
#include "db.h"
#include "msg.h"
#include "str.h"
#include "util.h"

varr msg_hash_table[MAX_MSG_HASH];

#define msghash(s) hashstr(s, 32, MAX_MSG_HASH)
static int cmpmsgname(const void*, const void*);
static int cmpmsg(const void*, const void*);

msg_t *msg_add(msg_t *m)
{
	msg_t *m2;
	varr *v;
	const char *name = mlstr_mval(&m->ml);

	if (IS_NULLSTR(name))
		return NULL;

	v = msg_hash_table+msghash(name);

	if (varr_bsearch(v, name, cmpmsgname)) {
		db_error("msg_add", "%s: duplicate entry", name);
		return NULL;
	}

	m2 = varr_enew(v);
	*m2 = *m;
	varr_qsort(v, cmpmsg);
	return varr_bsearch(v, name, cmpmsgname);
}

msg_t *msg_lookup(const char *name)
{
	if (IS_NULLSTR(name))
		return NULL;

	return varr_bsearch(msg_hash_table+msghash(name), name, cmpmsgname);
}

int msg_gender(const char *name)
{
	msg_t *mp = msg_lookup(name);
	if (mp == NULL)
		return SEX_MALE;
	return mp->gender;
}

msg_t msg_del(const char *name)
{
	varr *v;
	msg_t *mp;
	msg_t m;

	v = msg_hash_table+msghash(name);

	mp = varr_bsearch(v, name, cmpmsgname);
	if (mp == NULL) {
		mlstr_init(&m.ml, NULL);
		return m;
	}
	m = *mp;
	mlstr_init(&mp->ml, NULL);
	varr_qsort(v, cmpmsg);
	v->nused--;
	return m;
}

const char *GETMSG(const char *msg, int lang)
{
	msg_t *m = msg_lookup(msg);
	if (m == NULL)
		return msg;
	return mlstr_val(&m->ml, lang);
}

/* reverse order (otherwise msg_del will not work) */
static int cmpmsgname(const void* p1, const void* p2)
{
	return -strcmp((char*)p1, mlstr_mval(&((msg_t*) p2)->ml));
}

/* reverse order (otherwise msg_del will not work) */
static int cmpmsg(const void* p1, const void* p2)
{
	return -strcmp(mlstr_mval(&((msg_t*) p1)->ml),
		       mlstr_mval(&((msg_t*) p2)->ml));
}

