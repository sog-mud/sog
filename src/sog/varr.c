/*-
 * Copyright (c) 1998 SoG Development Team
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
 * $Id: varr.c,v 1.40 2001-11-30 21:18:04 fjoe Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <typedef.h>
#include <container.h>
#include <varr.h>
#include <buffer.h>
#include <str.h>
#include <util.h>

/*
 * Variable size array implementation
 */

void *
varr_touch(varr *v, size_t i)
{
	void *p;

	if (i >= v->nalloc) {
		int nalloc = (i/v->v_data->nstep + 1) * v->v_data->nstep;
		p = realloc(v->p, nalloc*v->v_data->nsize);
		if (p == NULL)
			return NULL;
		v->nalloc = nalloc;
		v->p = p;
	}

	p = VARR_GET(v, i);
	if (i >= v->nused) {
		size_t j;

		for (j = v->nused; j < i+1; j++) {
			if (v->v_data->e_init)
				v->v_data->e_init(VARR_GET(v, j));
			else
				memset(VARR_GET(v, j), 0, v->v_data->nsize);
		}
		v->nused = i+1;
	}
	return p;
}

void *
varr_insert(varr *v, size_t i)
{
	void *p;

	if (i >= v->nused)
		return varr_touch(v, i);

	varr_enew(v);
	p = VARR_GET(v, i);
	memmove(VARR_GET(v, i + 1), p, v->v_data->nsize * (v->nused - 1 - i));

	if (v->v_data->e_init)
		v->v_data->e_init(p);
	else
		memset(p, 0, v->v_data->nsize);

	return p;
}

void *
varr_enew(varr *v)
{
	return varr_touch((v), c_size(v));
}

void *
varr_get(varr *v, size_t i)
{
	return i >= c_size(v) ? NULL : VARR_GET(v, i);
}

void
varr_ndelete(varr *v, size_t i)
{
	if (!v->nused || i >= v->nused)
		return;

	if (v->v_data->e_destroy)
		v->v_data->e_destroy(VARR_GET(v, i));

	varr_ndelete_nd(v, i);
}

void
varr_ndelete_nd(varr *v, size_t i)
{
	if (!v->nused || i >= v->nused)
		return;

	if (i >= --v->nused)
		return;

	memmove(
	    VARR_GET(v, i), VARR_GET(v, i+1), v->v_data->nsize*(v->nused - i));
}

void
varr_destroy_nd(varr *v)
{
	free(v->p);
}

void
varr_qsort(const varr *v, int (*cmpfun)(const void *, const void *))
{
	if (v == NULL || v->nused == 0)
		return;
	qsort(v->p, v->nused, v->v_data->nsize, cmpfun);
}

void *
varr_bsearch(const varr *v, const void *e,
	     int (*cmpfun)(const void *, const void *))
{
	if (v == NULL || v->nused == 0)
		return NULL;
	return bsearch(e, v->p, v->nused, v->v_data->nsize, cmpfun);
}

void *
varr_bsearch_lower(const varr *v, const void *e,
		   int (*cmpfun)(const void *, const void *))
{
	if (v == NULL || v->nused == 0)
		return NULL;
	return bsearch_lower(e, v->p, v->nused, v->v_data->nsize, cmpfun);
}

void *
varr_eforeach(const varr *v, void *e, foreach_cb_t cb, ...)
{
	void *rv;
	va_list ap;

	va_start(ap, cb);
	rv = varr_anforeach(v, e ? varr_index(v, e) : 0, cb, ap);
	va_end(ap);

	return rv;
}

void *
varr_nforeach(const varr *v, size_t from, foreach_cb_t cb, ...)
{
	void *rv;
	va_list ap;

	va_start(ap, cb);
	rv = varr_anforeach(v, from, cb, ap);
	va_end(ap);

	return rv;
}

void *
varr_anforeach(const varr *v, size_t from, foreach_cb_t cb, va_list ap)
{
	size_t i;

	for (i = from; i < v->nused; i++) {
		void *p;
		if ((p = cb(VARR_GET(v, i), ap)) != NULL)
			return p;
	}

	return NULL;
}

void *
varr_rforeach(const varr *v, foreach_cb_t cb, ...)
{
	void *rv;
	va_list ap;

	if (!v->nused)
		return NULL;

	va_start(ap, cb);
	rv = varr_arnforeach(v, v->nused - 1, cb, ap);
	va_end(ap);

	return rv;
}

void *
varr_reforeach(const varr *v, void *e, foreach_cb_t cb, ...)
{
	void *rv;
	va_list ap;

	if (!v->nused)
		return NULL;

	va_start(ap, cb);
	rv = varr_arnforeach(v, e ? varr_index(v, e) : v->nused - 1, cb, ap);
	va_end(ap);

	return rv;
}

void *
varr_rnforeach(const varr *v, size_t from, foreach_cb_t cb, ...)
{
	void *rv;
	va_list ap;

	va_start(ap, cb);
	rv = varr_arnforeach(v, from, cb, ap);
	va_end(ap);

	return rv;
}

void *
varr_arnforeach(const varr *v, size_t from, foreach_cb_t cb, va_list ap)
{
	size_t i;

	if (from > v->nused)
		from = v->nused;

	i = UMIN(from, v->nused);
	for (;;) {
		void *p;
		if ((p = cb(VARR_GET(v, i), ap)) != NULL)
			return p;
		if (!i--)
			break;
	}

	return NULL;
}

/*-------------------------------------------------------------------
 * container ops
 */

#define varr_delete varr_ops_delete

DEFINE_C_OPS(varr);

static void
varr_init(void *c, void *v_data)
{
	varr *v = (varr *) c;

	v->p = NULL;
	v->nused = 0;
	v->nalloc = 0;
	v->v_data = v_data;
}

static void
varr_destroy(void *c)
{
	varr *v = (varr *) c;

	varr_erase(v);
	varr_destroy_nd(v);
}

static void
varr_erase(void *c)
{
	varr *v = (varr *) c;

	if (v->v_data->e_destroy) {
		void *elem;

		C_FOREACH(elem, v)
			v->v_data->e_destroy(elem);
	}

	v->nused = 0;
}

static void *
varr_lookup(void *c, const void *k)
{
	varr *v = (varr *) c;
	const char *name = (const char *) k;
	void *elem;

	if (IS_NULLSTR(name))
		return NULL;

	C_FOREACH(elem, v) {
		if (!str_cmp(name, *(const char **) elem))
			return elem;
	}

	return NULL;
}

static void *
varr_add(void *c, const void *k, int flags)
{
	UNUSED_ARG(c);
	UNUSED_ARG(k);
	UNUSED_ARG(flags);
	return NULL;
}

static void
varr_delete(void *c, const void *k)
{
	UNUSED_ARG(c);
	UNUSED_ARG(k);
}

static void
varr_move(void *c, const void *k, const void *k_new)
{
	UNUSED_ARG(c);
	UNUSED_ARG(k);
	UNUSED_ARG(k_new);
}

static void *
varr_foreach(void *c, foreach_cb_t cb, va_list ap)
{
	return varr_anforeach(c, 0, cb, ap);
}

static void *
varr_first(void *c)
{
	varr *v = (varr *) c;

	return VARR_GET(v, 0);
}

static bool
varr_cond(void *c, void *elem)
{
	varr *v = (varr *) c;

	return varr_index(v, elem) < varr_size(v);
}

static void *
varr_next(void *c, void *elem)
{
	varr *v = (varr *) c;

	return VARR_GET(v, varr_index(v, elem) + 1);
}

static size_t
varr_size(void *c)
{
	varr *v = (varr *) c;

	return v->nused;
}

static bool
varr_isempty(void *c)
{
	return c_size(c) == 0;
}

static void *
varr_random_elem(void *c)
{
	varr *v = (varr *) c;
	size_t size = c_size(c);

	if (!size)
		return NULL;

	return VARR_GET(v, number_range(1, size) - 1);
}
