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
 * $Id: hash.c,v 1.23 2001-09-13 16:08:58 fjoe Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <typedef.h>
#include <varr.h>
#include <hash.h>
#include <container.h>
#include <str.h>
#include <mlstring.h>
#include <util.h>

static void *hash_search(hash_t *h, const void *k, varr *v);

static DECLARE_FOREACH_CB_FUN(add_strkey_cb);
static DECLARE_FOREACH_CB_FUN(add_mlstrkey_cb);

static void hash_dump(hash_t *h, BUFFER *buf, foreach_cb_t addname_cb);

int
k_hash_vnum(const void *k, size_t hsize)
{
	return (*(const int *) k) * 17 % hsize;
}

int
k_hash_str(const void *k, size_t hsize)
{
	return hashcasestr((const char *) k, 32, hsize);
}

int
k_hash_csstr(const void *k, size_t hsize)
{
	return hashstr((const char *) k, 32, hsize);
}

void
hash_strkey_dump(hash_t *h, BUFFER *buf)
{
	hash_dump(h, buf, add_strkey_cb);
}

void
hash_mlstrkey_dump(hash_t *h, BUFFER *buf)
{
	hash_dump(h, buf, add_mlstrkey_cb);
}

/*-------------------------------------------------------------------
 * container ops
 */

DEFINE_C_OPS(hash);

static void
hash_init(void *c, void *info)
{
	hash_t *h = (hash_t *) c;
	hashdata_t *h_data = (hashdata_t *) info;
	size_t i;

	h->h_data = h_data;
	h->v = malloc(h_data->hsize * sizeof(varr));

	h->v[0].v_data = malloc(sizeof(varrdata_t));
	memcpy(h->v[0].v_data, h_data, sizeof(varrdata_t));
	h->v[0].v_data->ops = &varr_ops;

	for (i = 0; i < h_data->hsize; i++)
		c_init(h->v + i, h->v[0].v_data);
}

static void
hash_destroy(void *c)
{
	hash_t *h = (hash_t *) c;
	size_t i;
	varrdata_t *v_data;

	v_data = h->v[0].v_data;
	for (i = 0; i < h->h_data->hsize; i++)
		c_destroy(h->v + i);

	free(v_data);
	free(h->v);
}

static void
hash_erase(void *c)
{
	hash_t *h = (hash_t *) c;
	size_t i;

	for (i = 0; i < h->h_data->hsize; i++)
		c_erase(h->v + i);
}

static void *
hash_lookup(void *c, const void *k)
{
	hash_t *h = (hash_t *) c;
	return hash_search(h, k, h->v + h->h_data->k_hash(k, h->h_data->hsize));
}

static void *
hash_add(void *c, const void *k, int flags)
{
	hash_t *h = (hash_t *) c;
	varr *v = h->v + h->h_data->k_hash(k, h->h_data->hsize);
	void *elem = hash_search(h, k, v);	/* existing element */

	if (elem == NULL) {
		if (!IS_SET(flags, CA_F_INSERT))
			return NULL;
		elem = varr_enew(v);
	} else {
		if (!IS_SET(flags, CA_F_UPDATE))
			return NULL;
		if (v->v_data->e_destroy != NULL)
			v->v_data->e_destroy(elem);
		if (v->v_data->e_init != NULL)
			v->v_data->e_init(elem);
	}

	return elem;
}

static void
hash_delete(void *c, const void *k)
{
	hash_t *h = (hash_t *) c;
	varr *v = h->v + h->h_data->k_hash(k, h->h_data->hsize);
	void *e = hash_search(h, k, v);

	if (e == NULL)
		return;

	varr_edelete(v, e);
}

/*
 * hash_foreach -- call `cb' for each item in hash
 *		   if `cb' returns value < 0 then hash_foreach will
 *		   exit immediately
 */
void *
hash_foreach(void *c, foreach_cb_t cb, va_list ap)
{
	hash_t *h = (hash_t *) c;
	size_t i;
	void *rv = NULL;

	for (i = 0; i < h->h_data->hsize; i++) {
		if ((rv = varr_anforeach(h->v + i, 0, cb, ap)) != NULL)
			break;
	}

	return rv;
}

size_t
hash_size(void *c)
{
	hash_t *h = (hash_t *) c;
	size_t i;
	size_t size = 0;

	for (i = 0; i < h->h_data->hsize; i++)
		size += c_size(h->v + i);

	return size;
}

bool
hash_isempty(void *c)
{
	hash_t *h = (hash_t *) c;
	size_t i;

	for (i = 0; i < h->h_data->hsize; i++) {
		if (!c_isempty(h->v + i))
			return FALSE;
	}

	return TRUE;
}

#if !defined(HASHTEST) && !defined(MPC)
void *
hash_random_elem(void *c)
{
	hash_t *h = (hash_t *) c;

	if (hash_isempty(h))
		return NULL;

	for (; ;) {
		varr *v = h->v + number_range(0, h->h_data->hsize - 1);
		if (v->nused == 0)
			continue;
		return VARR_GET(v, number_range(0, c_size(v) - 1));
	}
}
#endif

/*-------------------------------------------------------------------
 * static functions
 */

static void *
hash_search(hash_t *h, const void *k, varr *v)
{
	size_t i;

	for (i = 0; i < v->nused; i++) {
		void *e = VARR_GET(v, i);
		if (!h->h_data->ke_cmp(k, e))
			return e;
	}

	return NULL;
}

static
FOREACH_CB_FUN(add_strkey_cb, p, ap)
{
	varr *v = va_arg(ap, varr *);
	const char **q = varr_enew(v);
	*q = *(const char **) p;
	return NULL;
}

static
FOREACH_CB_FUN(add_mlstrkey_cb, p, ap)
{
	varr *v = va_arg(ap, varr *);
	const char **q = varr_enew(v);
	*q = mlstr_mval((mlstring *) p);
	return NULL;
}

static varrdata_t v_print =
{
	&varr_ops, NULL, NULL,

	sizeof(const char *), 8
};

static void
hash_dump(hash_t *h, BUFFER *buf, foreach_cb_t addname_cb)
{
	varr v;

	c_init(&v, &v_print);
	c_foreach(h, addname_cb, &v);
	varr_qsort(&v, cmpstr);
	c_strkey_dump(&v, buf);
	c_destroy(&v);
}
