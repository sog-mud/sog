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
 * $Id: varr.h,v 1.31 2001-09-15 17:12:33 fjoe Exp $
 */

#ifndef _VARR_H_
#define _VARR_H_

extern c_ops_t varr_ops;

typedef struct varr_info_t {
	c_ops_t *ops;			/**< container ops		*/

	e_init_t e_init;		/**< init elem			*/
	e_destroy_t e_destroy;		/**< destroy elem		*/

	size_t nsize;			/**< size of elem		*/
	size_t nstep;			/**< allocation step		*/
} varr_info_t;

typedef struct varr varr;
struct varr {
	varr_info_t *v_data;

	void *p;

	size_t nused;			/* elems used */
	size_t nalloc;			/* elems allocated */
};

#define varr_esize(v)	((v)->v_data->nsize)
#define varr_index(v, q) ((((const char *) q) - ((const char *) (v)->p)) / (v)->v_data->nsize)

void *	varr_touch	(varr *, size_t i);
void *	varr_insert	(varr *, size_t i);

void *	varr_enew(varr *v);
void *	varr_get(varr *v, size_t i);
#define VARR_GET(v, i)	((void *) (((char *) (v)->p) + (i)*(v)->v_data->nsize))

void	varr_ndelete	(varr *, size_t i);
#define varr_edelete(v, p) (varr_ndelete((v), varr_index((v), (p))))

void	varr_ndelete_nd	(varr *, size_t i);
#define varr_edelete_nd(v, p) (varr_ndelete_nd((v), varr_index((v), (p))))

void	varr_destroy_nd	(varr *);

void	varr_qsort	(const varr *, int (*)(const void *, const void *));
void *	varr_bsearch	(const varr *, const void *e,
			 int (*)(const void *, const void *));
void *	varr_bsearch_lower(const varr *, const void *e,
			   int (*)(const void *, const void *));

/*
 * iterators
 */
void *	varr_eforeach	(const varr *, void *, foreach_cb_t *, ...);
void *	varr_nforeach	(const varr *, size_t i, foreach_cb_t *, ...);
void *	varr_anforeach	(const varr *, size_t i, foreach_cb_t *, va_list ap);

/*
 * reverse iterators
 */
void *	varr_rforeach	(const varr *, foreach_cb_t *, ...);
void *	varr_reforeach	(const varr *, void *, foreach_cb_t *, ...);
void *	varr_rnforeach	(const varr *, size_t i, foreach_cb_t *, ...);
void *	varr_arnforeach	(const varr *, size_t i, foreach_cb_t *, va_list ap);

#endif
