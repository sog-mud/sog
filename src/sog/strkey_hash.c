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
 * $Id: strkey_hash.c,v 1.15 2000-04-16 09:21:56 fjoe Exp $
 */

#include <limits.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "typedef.h"
#include "varr.h"
#include "hash.h"
#include "strkey_hash.h"
#include "buffer.h"
#include "str.h"
#include "mlstring.h"
#include "db.h"
#include "log.h"

void strkey_init(void *p)
{
	*(const char **) p = str_empty;
}

#if !defined(HASHTEST)
void strkey_destroy(void *p)
{
	free_string(*(const char **) p);
}
#endif

int k_hash_str(const void *k, size_t hsize)
{
	return hashcasestr((const char *) k, 32, hsize);
}

int ke_cmp_str(const void *k, const void *e)
{
	return str_cmp((const char *) k, *(const char**) e);
}

int k_hash_csstr(const void *k, size_t hsize)
{
	return hashstr((const char *) k, 32, hsize);
}

int ke_cmp_csstr(const void *k, const void *e)
{
	return str_cscmp((const char *) k, *(const char **) e);
}

#if !defined(HASHTEST)
int ke_cmp_mlstr(const void *k, const void *e)
{
	return str_cmp((const char *) k, mlstr_mval((mlstring *) e));
}

int ke_cmp_csmlstr(const void *k, const void *e)
{
	return str_cscmp((const char *) k, mlstr_mval((mlstring *) e));
}
#endif

void *
strkey_lookup(hash_t *h, const char *name)
{
	if (IS_NULLSTR(name))
		return NULL;
	return hash_lookup(h, name);
}

void *
strkey_search_cb(void *p, va_list ap)
{
	const char *key = va_arg(ap, const char *);
	if (!str_prefix(key, *(const char **) p))
		return p;
	return NULL;
}

/*
 * strkey_search -- lookup elem by prefix
 */
void *
strkey_search(hash_t *h, const char *name)
{
	void *p;

	if (IS_NULLSTR(name))
		return NULL;

	/*
	 * try exact match first
	 */
	if ((p = hash_lookup(h, name)) != NULL)
		return p;

	/*
	 * search by prefix
	 */
	return hash_foreach(h, vstr_search_cb, name);
}

#if !defined(HASHTEST)
void *
mlstrkey_search_cb(void *p, va_list ap)
{
	const char *key = va_arg(ap, const char *);
	if (!str_prefix(key, mlstr_mval((mlstring *) p)))
		return p;
	return NULL;
}

/*
 * mlstrkey_search -- lookup elem by prefix
 */
void *
mlstrkey_search(hash_t *h, const char *name)
{
	void *p;

	if (IS_NULLSTR(name))
		return NULL;

	/*
	 * try exact match first
	 */
	if ((p = hash_lookup(h, name)) != NULL)
		return p;

	/*
	 * search by prefix
	 */
	return hash_foreach(h, mlstrkey_search_cb, name);
}

const char *fread_strkey(rfile_t *fp, hash_t *h, const char *id)
{
	const char *name = fread_sword(fp);
	STRKEY_CHECK(h, name, id);
	return name;
}
#endif

char *
strkey_filename(const char *name, const char *ext)
{
	static char buf[2][MAX_STRING_LENGTH];
	static int ind = 0;
	char *p;

	if (IS_NULLSTR(name))
		return str_empty;

	ind = (ind + 1) % 2;
	for (p = buf[ind]; *name && p-buf[ind] < sizeof(buf[0])-1; p++, name++) {
		switch (*name) {
		case ' ':
		case '\t':
			*p = '_';
			break;

		default:
			*p = LOWER(*name);
			break;
		}
	}

	*p = '\0';
	if (!IS_NULLSTR(ext)) 
		strnzcat(buf[ind], sizeof(buf[ind]), ext);
	return buf[ind];
}

#if !defined(HASHTEST)
void *
add_strname_cb(void *p, va_list ap)
{
	varr *v = va_arg(ap, varr *);
	const char **q = varr_enew(v);
	*q = str_dup(*(const char **) p);
	return NULL;
}

void *
add_mlstrname_cb(void *p, va_list ap)
{
	varr *v = va_arg(ap, varr *);
	const char **q = varr_enew(v);
	*q = str_dup(mlstr_mval((mlstring *) p));
	return NULL;
}
#endif
