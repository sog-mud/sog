#ifndef _VARR_H_
#define _VARR_H_

/*
 * $Id: varr.h,v 1.1 1998-09-01 18:38:02 fjoe Exp $
 */

struct varr {
	void *p;
	size_t nsize;		/* size of elem */

	size_t nused;		/* elems used */
	size_t nalloc;		/* elems allocated */
	size_t nstep;		/* allocation step */
};

varr *	varr_new	(size_t nsize, size_t nstep);
void	varr_free	(varr*);
void *	varr_touch	(varr*, size_t i);
void	varr_qsort	(varr*, int (*)(const void*, const void*));
void *	varr_bsearch	(varr*, void *e, int (*)(const void*, const void*));

#define varr_enew(v)	(varr_touch((v), (v)->nused))
#define VARR_GET(v, i)	((void*) (((char*) (v)->p) + (i)*(v)->nsize))
#define varr_get(v, i)	((i) < 0 || (i) >= (v)->nused ? \
			 NULL : VARR_GET((v), (i)))

#endif
