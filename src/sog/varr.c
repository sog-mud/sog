/*
 * $Id: varr.c,v 1.1 1998-09-01 18:38:02 fjoe Exp $
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


