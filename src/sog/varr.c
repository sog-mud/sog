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
 * $Id: varr.c,v 1.30 2001-08-26 16:17:35 fjoe Exp $
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <typedef.h>
#include <varr.h>
#include <buffer.h>
#include <str.h>

/*
 * Variable size array implementation
 */

void varr_init(varr *v, varrdata_t *v_data)
{
	v->p = NULL;
	v->nused = 0;
	v->nalloc = 0;
	v->v_data = v_data;
}

static void *
varr_cpy_cb(void *p, va_list ap)
{
	varr *v = va_arg(ap, varr *);
	int *pi = va_arg(ap, int *);

	if (v->v_data->e_cpy)
		v->v_data->e_cpy(VARR_GET(v, (*pi)++), p);
	else
		memcpy(VARR_GET(v, (*pi)++), p, v->v_data->nsize);
	return NULL;
}

varr *
varr_cpy(varr *dst, const varr *src)
{
	dst->nused = src->nused;
	dst->nalloc = src->nalloc;
	dst->v_data = src->v_data;

	dst->p = malloc(dst->v_data->nsize * dst->nalloc);
	if (dst->v_data->e_cpy) {
		int i = 0;
		varr_foreach((varr *)(uintptr_t)src, varr_cpy_cb, dst, &i);
	} else
		memcpy(dst->p, src->p, dst->v_data->nsize * dst->nused);
	return dst;
}

static void *
varr_destroy_cb(void *p, va_list ap)
{
	varr *v = va_arg(ap, varr *);

	v->v_data->e_destroy(p);
	return NULL;
}

void
varr_destroy(varr *v)
{
	varr_erase(v);
	v->nalloc = 0;
	free(v->p);
}

void
varr_erase(varr *v)
{
	if (v->v_data->e_destroy)
		varr_foreach(v, varr_destroy_cb, v);
	v->nused = 0;
}

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
	void *p = VARR_GET(v, i);

	if (i >= v->nused)
		return varr_touch(v, i);
	varr_enew(v);
	memmove(VARR_GET(v, i+1), p, v->v_data->nsize*(v->nused-1 - i));
	if (v->v_data->e_init)
		v->v_data->e_init(p);
	else
		memset(p, 0, v->v_data->nsize);
	return p;
}

void
varr_delete(varr *v, size_t i)
{
	if (!v->nused || i >= v->nused)
		return;

	if (v->v_data->e_destroy)
		v->v_data->e_destroy(VARR_GET(v, i));

	if (i >= --v->nused)
		return;

	memmove(VARR_GET(v, i), VARR_GET(v, i+1), v->v_data->nsize*(v->nused - i));
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
	void *elem = varr_bsearch(v, e, cmpfun);
	size_t i;

	if (elem == NULL)
		return NULL;

	i = varr_index(v, elem);
	while (i) {
		if (!!cmpfun(VARR_GET(v, i), e))
			break;
		i--;
	}

	return VARR_GET(v, i);
}

void *
varr_foreach(const varr *v, foreach_cb_t cb, ...)
{
	void *rv;
	va_list ap;

	va_start(ap, cb);
	rv = varr_anforeach(v, 0, cb, ap);
	va_end(ap);

	return rv;
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

void *
vstr_lookup_cb(void *p, va_list ap)
{
	const char *name = va_arg(ap, const char *);

	if (!str_cmp(name, *(const char **) p))
		return p;

	return NULL;
}

void *
vstr_search_cb(void *p, va_list ap)
{
	const char *name = va_arg(ap, const char *);

	if (!str_prefix(name, *(const char **) p))
		return p;

	return NULL;
}

void *
vstr_lookup(varr *v, const char *name)
{
	if (IS_NULLSTR(name))
		return NULL;

	return varr_foreach(v, vstr_lookup_cb, name);
}

void *
vstr_search(varr *v, const char *name)
{
	void *p;

	if (IS_NULLSTR(name))
		return NULL;

	/*
	 * try exact match first
	 */
	if ((p = vstr_lookup(v, name)) != NULL)
		return p;

	return varr_foreach(v, vstr_search_cb, name);
}

#if !defined(HASHTEST) && !defined(MPC)
static void *
vstr_dump_cb(void *p, va_list ap)
{
	BUFFER *buf = va_arg(ap, BUFFER *);
	int *pcol = va_arg(ap, int *);

	buf_printf(buf, BUF_END, "%-19.18s",			// notrans
		   *(const char**) p);
	if (++(*pcol) % 4 == 0)
		buf_append(buf, "\n");
	return NULL;
}

void
vstr_dump(varr *v, BUFFER *buf)
{
	int col = 0;
	varr_foreach(v, vstr_dump_cb, buf, &col);
	if (col % 4 != 0)
		buf_append(buf, "\n");
}
#endif
