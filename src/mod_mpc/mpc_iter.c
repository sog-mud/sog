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
 * $Id: mpc_iter.c,v 1.9 2002-11-28 20:08:04 fjoe Exp $
 */

#include <stdlib.h>
#include <setjmp.h>

#include <typedef.h>
#include <memalloc.h>
#include <varr.h>
#include <container.h>
#include <avltree.h>
#include <dynafun.h>
#include <str.h>
#include <vo_iter.h>

#undef MODULE_NAME
#define MODULE_NAME MOD_MPC
#define MODULE_INIT MOD_MPC
#include <dynafun_decl.h>

#include "mpc_impl.h"

/* range */
void		range_init(int, int, iterdata_t *id, vo_t *v);
static void	range_destroy(iterdata_t *id);
static bool	range_cond(iterdata_t *id, vo_t *v);
static void	range_next(iterdata_t *id, vo_t *v);

/* char_room */
void		vo_iter_init(void *vo, iterdata_t *id, vo_t *v);
void		vo_void_iter_init(iterdata_t *id, vo_t *v);
static void	vo_iter_destroy(iterdata_t *id);
static bool	vo_iter_cond(iterdata_t *id, vo_t *v);
static void	vo_iter_next(iterdata_t *id, vo_t *v);

iter_t itertab[] = {
	{
	  DECLARE_FUN2(int, range,
		       ARG(int), from, ARG(int), to)
	  (dynafun_t) range_init, range_destroy, range_cond, range_next,
	  NULL
	},

#if !defined(MPC)
	{
	  DECLARE_FUN1(CHAR_DATA, char_in_room,
		       ARG(ROOM_INDEX_DATA), room)
	  (dynafun_t) vo_iter_init, vo_iter_destroy, vo_iter_cond, vo_iter_next,
	  &iter_char_room
	},

	{
	  DECLARE_FUN0(CHAR_DATA, char_in_world)
	  (dynafun_t) vo_void_iter_init, vo_iter_destroy,
	  vo_iter_cond, vo_iter_next,
	  &iter_char_world
	},

	{
	  DECLARE_FUN0(CHAR_DATA, npc_in_world)
	  (dynafun_t) vo_void_iter_init, vo_iter_destroy,
	  vo_iter_cond, vo_iter_next,
	  &iter_npc_world
	},

	{
	  DECLARE_FUN1(CHAR_DATA, obj_in_obj,
		       ARG(OBJ_DATA), obj)
	  (dynafun_t) vo_iter_init, vo_iter_destroy, vo_iter_cond, vo_iter_next,
	  &iter_obj_obj
	},

	{
	  DECLARE_FUN1(CHAR_DATA, obj_of_char,
		       ARG(CHAR_DATA), ch)
	  (dynafun_t) vo_iter_init, vo_iter_destroy, vo_iter_cond, vo_iter_next,
	  &iter_obj_char
	},

	{
	  DECLARE_FUN1(CHAR_DATA, obj_in_room,
		       ARG(ROOM_INDEX_DATA), room)
	  (dynafun_t) vo_iter_init, vo_iter_destroy, vo_iter_cond, vo_iter_next,
	  &iter_obj_room
	},

	{
	  DECLARE_FUN0(CHAR_DATA, obj_in_world)
	  (dynafun_t) vo_void_iter_init, vo_iter_destroy,
	  vo_iter_cond, vo_iter_next,
	  &iter_obj_world
	},
#endif
};

#define ITERTAB_SZ	(sizeof(itertab) / sizeof(iter_t))

iter_t *
mpc_iter_lookup(const char *name)
{
	static bool itertab_initialized;

	if (!itertab_initialized) {
		qsort(itertab, ITERTAB_SZ, sizeof(iter_t), cmpstr);
		itertab_initialized = TRUE;
	}

	return (iter_t *) bsearch(
	    &name, itertab, ITERTAB_SZ, sizeof(iter_t), cmpstr);
}

/*
 * range
 */

void
range_init(int from, int to, iterdata_t *id, vo_t *v)
{
	v->i = from;
	id->vo.i = to;
}

static void
range_destroy(iterdata_t *id)
{
	UNUSED_ARG(id);
}

static bool
range_cond(iterdata_t *id, vo_t *v)
{
	return v->i <= id->vo.i;
}

static void
range_next(iterdata_t *id, vo_t *v)
{
	UNUSED_ARG(id);
	v->i++;
}

/*
 * vo iterators
 */

void
vo_iter_init(void *vo, iterdata_t *id, vo_t *v)
{
	id->vo.p = vo;
	v->p = vo_foreach_init(id->vo.p, id->iter->vo_iter, &id->ftag);
}

void
vo_void_iter_init(iterdata_t *id, vo_t *v)
{
	id->vo.p = NULL;
	v->p = vo_foreach_init(id->vo.p, id->iter->vo_iter, &id->ftag);
}

static void
vo_iter_destroy(iterdata_t *id)
{
	vo_foreach_destroy(
	    id->vo.p, id->iter->vo_iter, &id->ftag, id->vo_next.p);
}

static bool
vo_iter_cond(iterdata_t *id, vo_t *v)
{
	v->p = vo_foreach_cond(
	    id->vo.p, id->iter->vo_iter, id->ftag, v->p, &id->vo_next.p);
	return v->p != NULL;
}

static void
vo_iter_next(iterdata_t *id, vo_t *v)
{
	v->p = id->vo_next.p;
}
