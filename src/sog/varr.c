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
 * $Id: varr.c,v 1.9 1999-06-24 06:36:32 fjoe Exp $
 */

#include <stdlib.h>
#include <string.h>

#include "typedef.h"
#include "varr.h"

/*
 * Variable size array implementation
 */

void varr_init(varr *v, size_t nsize, size_t nstep)
{
	v->nsize = nsize;
	v->nstep = nstep;
}

void varr_destroy(varr *v)
{
	v->nalloc = v->nused = 0;
	free(v->p);
}
	
void *varr_touch(varr *v, size_t i)
{
	void *p;

	if (i >= v->nalloc) {
		int nalloc = (i/v->nstep + 1) * v->nstep;
		p = realloc(v->p, nalloc*v->nsize);
		if (p == NULL)
			return NULL;
		v->nalloc = nalloc;
		v->p = p;
	}

	p = VARR_GET(v, i);
	if (i >= v->nused) {
		memset(VARR_GET(v, v->nused), 0, v->nsize*(i+1 - v->nused));
		v->nused = i+1;
	}
	return p;
}

void *varr_insert(varr *v, size_t i)
{
	if (i >= v->nused)
		return varr_touch(v, i);
	varr_enew(v);
	memmove(VARR_GET(v, i+1), VARR_GET(v, i), v->nsize*(v->nused-1 - i));
	return memset(VARR_GET(v, i), 0, v->nsize);
}

void varr_del(varr *v, void *p)
{
	size_t i = (((char*) p) - ((char*) v->p)) / v->nsize;

	if (!v->nused || i >= v->nused || i >= --v->nused)
		return;

	memmove(VARR_GET(v, i), VARR_GET(v, i+1), v->nsize*(v->nused - i));
}

void varr_qsort(varr* v, int (*cmpfun)(const void*, const void*))
{
	if (v == NULL || v->nused == 0)
		return;
	qsort(v->p, v->nused, v->nsize, cmpfun);
}

void *varr_bsearch(varr* v, const void *e,
		   int (*cmpfun)(const void*, const void*))
{
	if (v == NULL || v->nused == 0)
		return NULL;
	return bsearch(e, v->p, v->nused, v->nsize, cmpfun);
}

