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
 * $Id: mpc_c.c,v 1.1 2001-06-16 18:20:38 fjoe Exp $
 */

#include <stdio.h>

#include <typedef.h>
#include <log.h>
#include <memalloc.h>

#include "_mpc.h"

#define INT_BOP(c_bop_fun, op)						\
	void								\
	c_bop_fun(prog_t *prog)						\
	{								\
		vo_t *v1, *v2;						\
		vo_t v;							\
									\
		log(LOG_INFO, __FUNCTION__);				\
									\
		v2 = pop(prog);						\
		if (v2 == NULL)						\
			; /* XXX longjmp */				\
									\
		v1 = pop(prog);						\
		if (v1 == NULL)						\
			; /* XXX longjmp */				\
									\
		v.i = v1->i op v2->i;					\
		push(prog, v);						\
	}

static void push(prog_t *prog, vo_t vo);
static vo_t *pop(prog_t *prog);
static vo_t *peek(prog_t *prog);

void
c_pop(prog_t *prog)
{
	vo_t *v;
	const void **d2;

	log(LOG_INFO, __FUNCTION__);

	v = pop(prog);
	if (v == NULL)
		; /* XXX longjmp */

	d2 = varr_get(&prog->code, prog->ip);
	if (d2 == NULL)
		; /* XXX longjmp */

	prog->ip++;

	switch ((int) *d2) {
	case MT_INT:
		fprintf(stderr, "got: %d\n", v->i);
		break;

	default:
		fprintf(stderr, "got: (type %d)\n", (int) *d2);
		break;
	}
}

void
c_push_const(prog_t *prog)
{
	const void **d;
	vo_t vo;

	log(LOG_INFO, __FUNCTION__);
	d = varr_get(&prog->code, prog->ip);
	if (d == NULL)
		; /* XXX longjmp */

	prog->ip++;

	vo.s = *d;
	push(prog, vo);
}

void
c_push_var(prog_t *prog)
{
	log(LOG_INFO, __FUNCTION__);
	prog->ip++;
	/* XXX */
}

void
c_push_retval(prog_t *prog)
{
	log(LOG_INFO, __FUNCTION__);
	/* XXX */
}

INT_BOP(c_bop_lor, ||)
INT_BOP(c_bop_land, &&)
INT_BOP(c_bop_or, ^)
INT_BOP(c_bop_xor, ^)
INT_BOP(c_bop_and, ^)

INT_BOP(c_bop_ne, !=)
INT_BOP(c_bop_eq, ==)

INT_BOP(c_bop_gt, >)
INT_BOP(c_bop_ge, >=)
INT_BOP(c_bop_lt, <)
INT_BOP(c_bop_le, <=)

INT_BOP(c_bop_shl, <<)
INT_BOP(c_bop_shr, >>)
INT_BOP(c_bop_add, +)
INT_BOP(c_bop_sub, -)
INT_BOP(c_bop_mul, *)
INT_BOP(c_bop_mod, %)
INT_BOP(c_bop_div, /)

void
c_bop_ne_string(prog_t *prog)
{
	log(LOG_INFO, __FUNCTION__);
	/* XXX */
}

void
c_bop_eq_string(prog_t *prog)
{
	log(LOG_INFO, __FUNCTION__);
	/* XXX */
}

static void
push(prog_t *prog, vo_t vo)
{
	vo_t *v = varr_enew(&prog->data);
	*v = vo;
}

static vo_t *
pop(prog_t *prog)
{
	vo_t *vo = peek(prog);
	if (vo == NULL)
		return vo;

	prog->data.nused--;
	return vo;
}

static vo_t *
peek(prog_t *prog)
{
	return varr_get(&prog->data, prog->data.nused - 1);
}
