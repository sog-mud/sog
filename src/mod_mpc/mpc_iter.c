/*-
 * Copyright (c) 2001 SoG Development Team
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
 * $Id: mpc_iter.c,v 1.1 2001-06-23 13:07:35 fjoe Exp $
 */

#include <stdarg.h>
#include <stdlib.h>

#include <typedef.h>
#include <memalloc.h>
#include <varr.h>
#include <hash.h>
#include <dynafun.h>
#include <str.h>

#include "mpc_iter.h"

static void	range_init(int, int, vo_t *v, iterdata_t *id);
static bool	range_cond(vo_t *v, iterdata_t *);
static void	range_next(vo_t *v, iterdata_t *);

iter_t itertab[] = {
	{ { "range",	MT_INT,	2,	{ MT_INT, MT_INT },
	  (dynafun_t) range_init },
	  range_cond, range_next },
#if !defined(MPC)
#endif
};

#define ITERTAB_SZ	(sizeof(itertab) / sizeof(iter_t))

iter_t *
iter_lookup(const char *name)
{
	static bool itertab_initialized;

	if (!itertab_initialized) {
		qsort(itertab, ITERTAB_SZ, sizeof(iter_t), cmpstr);
		itertab_initialized = TRUE;
	}

	return (iter_t *) bsearch(
	    &name, itertab, ITERTAB_SZ, sizeof(iter_t), cmpstr);
}

void
range_init(int from, int to, vo_t *v, iterdata_t *id)
{
	v->i = from;
	id->v.i = to;
}

bool
range_cond(vo_t *v, iterdata_t *id)
{
	return v->i <= id->v.i;
}

void
range_next(vo_t *v, iterdata_t *id)
{
	v->i++;
}
