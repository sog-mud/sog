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
 * $Id: varr.c,v 1.2 1998-09-17 15:51:24 fjoe Exp $
 */

#include <stdlib.h>
#include <string.h>

#include "typedef.h"
#include "varr.h"

/*
 * Variable size array implementation
 */

varr *varr_new(size_t size, size_t step)
{
	varr *v;
	v = calloc(1, sizeof(*v));
	v->nsize = size;
	v->nstep = step;
	return v;
}

void varr_free(varr *v)
{
	free(v->p);
	free(v);
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
		memset(VARR_GET(v, i), 0, v->nsize*(i+1 - v->nused));
		v->nused = i+1;
	}
	return p;
}

void varr_qsort(varr* v, int (*cmpfun)(const void*, const void*))
{
	if (v == NULL || v->nused == 0)
		return;
	qsort(v->p, v->nused, v->nsize, cmpfun);
}

void *varr_bsearch(varr* v, void *e, int (*cmpfun)(const void*, const void*))
{
	if (v == NULL || v->nused == 0)
		return NULL;
	return bsearch(e, v->p, v->nused, v->nsize, cmpfun);
}


