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
 * $Id: varr.c,v 1.42 2003-04-24 13:37:01 fjoe Exp $
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
	return varr_touch((v), v->nused);
}

void *
varr_get(varr *v, size_t i)
{
	return i >= v->nused ? NULL : VARR_GET(v, i);
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

#if !defined(TEST)
void *
varr_bsearch_lower(const varr *v, const void *e,
		   int (*cmpfun)(const void *, const void *))
{
	if (v == NULL || v->nused == 0)
		return NULL;
	return bsearch_lower(e, v->p, v->nused, v->v_data->nsize, cmpfun);
}
#endif

void *
varr_eforeach_first(varr *v, void *start_elem)
{
	return varr_get(v, start_elem ? varr_index(v, start_elem) : 0);
}

void *
varr_nforeach_first(varr *v, size_t pos)
{
	return varr_get(v, pos);
}

void *
varr_rforeach_first(varr *v)
{
	if (!v->nused)
		return NULL;

	return varr_get(v, v->nused - 1);
}

void *
varr_reforeach_first(varr *v, void *start_elem)
{
	if (!v->nused)
		return NULL;

	return varr_get(v, start_elem ? varr_index(v, start_elem) : v->nused - 1);
}

void *
varr_rforeach_next(varr *v, void *elem)
{
	size_t pos = varr_index(v, elem);

	if (!pos)
		return NULL;

	return varr_get(v, pos - 1);
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
varr_first(void *c)
{
	varr *v = (varr *) c;

	return varr_get(v, 0);
}

static bool
varr_cond(void *c, void *elem)
{
	UNUSED_ARG(c);

	return elem != NULL;
}

static void *
varr_next(void *c, void *elem)
{
	varr *v = (varr *) c;

	return varr_get(v, varr_index(v, elem) + 1);
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
	varr *v = (varr *) c;

	return v->nused == 0;
}

static void *
varr_random_elem(void *c)
{
#if !defined(TEST)
	varr *v = (varr *) c;
	size_t size = v->nused;

	if (!size)
		return NULL;

	return VARR_GET(v, number_range(1, size) - 1);
#else
	UNUSED_ARG(c);
	return NULL;
#endif
}
