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
 * $Id: hash.c,v 1.8 1999-12-15 15:35:43 fjoe Exp $
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "const.h"
#include "typedef.h"
#include "varr.h"
#include "hash.h"

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
	h->v = malloc(hsize * sizeof(varr));

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

	varr_edelete(v, e);
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
void *hash_foreach(hash_t *h, foreach_cb_t cb, ...)
{
	int i;
	void *rv = NULL;
	va_list ap;
	
	va_start(ap, cb);
	for (i = 0; i < h->hsize; i++) {
		void *p;
		if ((p = varr_anforeach(h->v + i, 0, cb, ap)) != NULL) {
			rv = p;
			break;
		}
	}
	va_end(ap);

	return rv;
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

