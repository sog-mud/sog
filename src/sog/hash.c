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
 * $Id: hash.c,v 1.3 1999-10-20 11:10:44 fjoe Exp $
 */

#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "const.h"
#include "typedef.h"
#include "varr.h"
#include "hash.h"
#include "buffer.h"
#include "str.h"
#include "db.h"
#include "log.h"

/*
 * hash_add flags
 */
#define HA_INSERT	(A)	/* insert element if does not exist */
#define HA_REPLACE	(B)	/* replace element if exists */

static void *hash_search(hash_t *h, const void *k, varr *v);
static void *hash_add(hash_t *h, const void *k, const void *e, int flags);

void
hash_init(hash_t *h, size_t hsize, size_t nsize,
	  varr_e_init_t e_init, varr_e_destroy_t e_destroy)
{
	int i;

	h->hsize = hsize;
	h->v = calloc(hsize, sizeof(varr));

	for (i = 0; i < hsize; i++) {
		varr_init(h->v + i, nsize, 1);
		h->v[i].e_init = e_init;
		h->v[i].e_destroy = e_destroy;
	}

	h->k_hash = NULL;
	h->ke_cmp = NULL;
	h->e_cpy = NULL;
}

void hash_destroy(hash_t *h)
{
	int i;

	for (i = 0; i < h->hsize; i++)
		varr_destroy(h->v + i);

	free(h->v);
}

void *
hash_lookup(hash_t *h, const void *k)
{
	return hash_search(h, k, h->v + h->k_hash(k, h->hsize));
}

void
hash_delete(hash_t *h, const void *k)
{
	varr *v = h->v + h->k_hash(k, h->hsize);
	void *e = hash_search(h, k, v);

	if (e == NULL)
		return;

	varr_delete(v, varr_index(v, e));
}

void *
hash_insert(hash_t *h, const void *k, const void *e)
{
	return hash_add(h, k, e, HA_INSERT);
}

void *
hash_replace(hash_t *h, const void *k, const void *e)
{
	return hash_add(h, k, e, HA_REPLACE);
}

void *
hash_update(hash_t *h, const void *k, const void *e)
{
	return hash_add(h, k, e, HA_INSERT | HA_REPLACE);
}

bool
hash_isempty(hash_t *h)
{
	int i;

	for (i = 0; i < h->hsize; i++) {
		if (h->v[i].nused != 0)
			return FALSE;
	}

	return TRUE;
}

void *
hash_random_item(hash_t *h)
{
	extern int number_range(int min, int max);

	if (hash_isempty(h))
		return NULL;

	for (;;) {
		varr *v = h->v + number_range(0, h->hsize - 1);
		if (v->nused == 0)
			continue;
		return VARR_GET(v, number_range(0, v->nused - 1));
	}
}

/*
 * hash_foreach -- call `cb' for each item in hash
 *		   if `cb' returns value < 0 then hash_foreach will
 *		   exit immediately
 */
void *hash_foreach(hash_t *h, void *(*cb)(void *p, void *d), void *d)
{
	int i;

	for (i = 0; i < h->hsize; i++) {
		void *p;
		if ((p = varr_foreach(h->v + i, cb, d)) != NULL)
			return p;
	}

	return NULL;
}

typedef struct _bufout_t _bufout_t;
struct _bufout_t {
	BUFFER *buf;
	int col;
};

static void *
print_name_cb(void *p, void *d)
{
	_bufout_t *_b = (_bufout_t*) d;

	if (++_b->col % 4 == 0)
		buf_add(_b->buf, "\n");
	buf_printf(_b->buf, "%-19.18s", *(const char**) p);
	return NULL;
}

void hash_print_names(hash_t *h, BUFFER *buf)
{
	_bufout_t _b = { buf, 0 };
	hash_foreach(h, print_name_cb, &_b);
	if (_b.col % 4)
		buf_add(buf, "\n");
}

void name_init(void *p)
{
	*(const char **) p = str_empty;
}

void name_destroy(void *p)
{
	free_string(*(const char **) p);
}

int name_hash(const void *k, size_t hsize)
{
	return hashistr((const char*) k, 32, hsize);
}

int name_struct_cmp(const void *k, const void *e)
{
	return str_cmp((const char*) k, *(const char**) e);
}

const char *fread_name(FILE *fp, hash_t *h, const char *id)
{
	const char *name = str_dup(fread_word(fp));
	NAME_CHECK(h, name, id);
	return name;
}

void *
name_search_cb(void *p, void *d)
{
	if (!str_prefix((const char *) d, *(const char **) p))
		return p;
	return NULL;
}

/*
 * skill_search -- lookup skill by prefix
 */
void *
name_search(hash_t *h, const char *name)
{
	if (IS_NULLSTR(name))
		return NULL;
	return hash_foreach(h, name_search_cb, (void*) name);
}

char *
name_filename(const char *name)
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
	return buf[ind];
}

/*-------------------------------------------------------------------
 * static functions
 */
static void *
hash_search(hash_t *h, const void *k, varr *v)
{
	int i;

	for (i = 0; i < v->nused; i++) {
		void *e = VARR_GET(v, i);
		if (!h->ke_cmp(k, e))
			return e;
	}

	return NULL;
}

static void *
hash_add(hash_t *h, const void *k, const void *e, int flags)
{
	varr *v = h->v + h->k_hash(k, h->hsize);
	void *elem = hash_search(h, k, v);	/* existing element */

	if (elem == NULL) {
		if (!IS_SET(flags, HA_INSERT))
			return NULL;
		elem = varr_enew(v);
	} else {
		if (!IS_SET(flags, HA_REPLACE))
			return NULL;
		if (v->e_destroy)
			v->e_destroy(elem);
	}

	return h->e_cpy(elem, e);
}

