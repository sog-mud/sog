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
 * $Id: varr.h,v 1.21 2001-06-24 10:50:41 avn Exp $
 */

#ifndef _VARR_H_
#define _VARR_H_

typedef void (*e_init_t)(void *);
typedef void *(*e_cpy_t)(void *, const void *);
typedef void (*e_destroy_t)(void *);

typedef struct varrdata_t {
	size_t nsize;			/* size of elem */
	size_t nstep;			/* allocation step */

	e_init_t e_init;		/* init elem */
	e_destroy_t e_destroy;		/* destroy elem */
	e_cpy_t e_cpy;			/* copy elem */
} varrdata_t;

typedef struct varr varr;
struct varr {
	void *p;

	size_t nused;			/* elems used */
	size_t nalloc;			/* elems allocated */

	varrdata_t *v_data;
};

void	varr_init	(varr *, varrdata_t *v_data);
varr *	varr_cpy	(varr *dst, const varr *src);
void	varr_destroy	(varr *);

void	varr_erase	(varr *);
void *	varr_touch	(varr *, size_t i);
void *	varr_insert	(varr *, size_t i);
void	varr_delete	(varr *, size_t i);

void	varr_qsort	(const varr *, int (*)(const void *, const void *));
void *	varr_bsearch	(const varr *, const void *e,
			 int (*)(const void *, const void *));

typedef void *(*foreach_cb_t)(void *, va_list);

/*
 * iterators
 */
void *	varr_foreach	(const varr *, foreach_cb_t, ...);
void *	varr_eforeach	(const varr *, void *, foreach_cb_t, ...);
void *	varr_nforeach	(const varr *, size_t i, foreach_cb_t, ...);
void *	varr_anforeach	(const varr *, size_t i, foreach_cb_t, va_list ap);

/*
 * reverse iterators
 */
void *	varr_rforeach	(const varr *, foreach_cb_t, ...);
void *	varr_reforeach	(const varr *, void *, foreach_cb_t, ...);
void *	varr_rnforeach	(const varr *, size_t i, foreach_cb_t, ...);
void *	varr_arnforeach	(const varr *, size_t i, foreach_cb_t, va_list ap);

#define varr_size(v)	((v)->nused)
#define varr_enew(v)	(varr_touch((v), varr_size(v)))
#define VARR_GET(v, i)	((void *) (((char *) (v)->p) + (i)*(v)->v_data->nsize))

void *varr_get(varr *v, size_t i);
extern inline void *varr_get(varr *v, size_t i)
{
	return i >= varr_size(v) ? NULL : VARR_GET(v, i);
}

#define varr_index(v, q) ((((const char*) q) - ((const char*) (v)->p)) / varr_size(v))
#define varr_edelete(v, p) (varr_delete((v), varr_index((v), (p))))
#define varr_isempty(v)	(!varr_size(v))

/*
 * `vstr_lookup' does precise search of name (str_cmp)
 * `vstr_search' does `vstr_lookup', if not found prefix search (str_prefix)
 * is performed
 */
void *	vstr_lookup_cb	(void *p, va_list ap);
void *	vstr_search_cb	(void *p, va_list ap);

void *	vstr_lookup	(varr *v, const char *name);
void *	vstr_search	(varr *v, const char *name);

void	vstr_dump	(varr *v, BUFFER *buf);

#endif
