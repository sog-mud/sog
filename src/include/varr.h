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
 * $Id: varr.h,v 1.12 1999-11-23 12:14:30 fjoe Exp $
 */

#ifndef _VARR_H_
#define _VARR_H_

#include <stdarg.h>

typedef void (*varr_e_init_t)(void *);
typedef void *(*varr_e_cpy_t)(void *, void *);
typedef void (*varr_e_destroy_t)(void *);

typedef struct varr varr;
struct varr {
	size_t nsize;			/* size of elem */
	size_t nstep;			/* allocation step */

	void *p;

	size_t nused;			/* elems used */
	size_t nalloc;			/* elems allocated */

	varr_e_init_t e_init;		/* init elem */
	varr_e_cpy_t e_cpy;		/* copy elem */
	varr_e_destroy_t e_destroy;	/* destroy elem */
};

void	varr_init	(varr*, size_t nsize, size_t nstep);
varr *	varr_cpy	(varr* dst, varr *src);
void	varr_destroy	(varr*);

void *	varr_touch	(varr*, size_t i);
void *	varr_insert	(varr*, size_t i);
void	varr_delete	(varr*, size_t i);

void	varr_qsort	(varr*, int (*)(const void*, const void*));
void *	varr_bsearch	(varr*, const void *e,
			 int (*)(const void*, const void*));

typedef void *(*foreach_cb_t)(void *, va_list);

void *	varr_foreach	(varr *, foreach_cb_t, ...);
void *	varr_eforeach	(varr *, void *, foreach_cb_t, ...);
void *	varr_nforeach	(varr *, size_t i, foreach_cb_t, ...);
void *	varr_anforeach	(varr *, size_t i, foreach_cb_t, va_list ap);

#define varr_enew(v)	(varr_touch((v), (v)->nused))
#define VARR_GET(v, i)	((void*) (((char*) (v)->p) + (i)*(v)->nsize))
#define varr_get(v, i)	((i) < 0 || (i) >= (v)->nused ? \
			 NULL : VARR_GET((v), (i)))
#define varr_index(v, q) ((((char*) q) - ((char*) (v)->p)) / (v)->nsize)
#define varr_edelete(v, p) (varr_delete((v), varr_index((v), (p))))
#define varr_isempty(v)	(!(v)->nused)

#endif
