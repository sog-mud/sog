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
 * $Id: mpc_c.c,v 1.3 2001-06-18 17:11:52 fjoe Exp $
 */

#include <stdio.h>
#include <stdarg.h>

#include <typedef.h>
#include <log.h>
#include <memalloc.h>
#include <varr.h>
#include <hash.h>
#include <dynafun.h>
#include <setjmp.h>

#include "_mpc.h"

static void push(prog_t *prog, vo_t vo);
static vo_t *pop(prog_t *prog);
static vo_t *peek(prog_t *prog, size_t depth);

static void *code_get(prog_t *prog);
static sym_t *sym_get(prog_t *prog, symtype_t symtype);

static void mpc_assert(prog_t *prog, const char *ctx, int e,
		       const char *fmt, ...)
	__attribute__ ((format(printf, 4, 5)));

#if 0
#define TRACE log(LOG_INFO, __FUNCTION__)
#else
#define TRACE
#endif

void
c_pop(prog_t *prog)
{
	vo_t *v;
	int type_tag;

	TRACE;

	v = pop(prog);
	type_tag = (int) code_get(prog);

	switch (type_tag) {
	case MT_INT:
		fprintf(stderr, "got: %d\n", v->i);
		break;

	default:
		fprintf(stderr, "got: (type %d)\n", type_tag);
		break;
	}
}

void
c_push_const(prog_t *prog)
{
	vo_t vo;

	TRACE;
	vo.s = code_get(prog);
	push(prog, vo);
}

void
c_push_var(prog_t *prog)
{
	sym_t *sym;

	TRACE;
	sym = sym_get(prog, SYM_VAR);
	push(prog, sym->s.var.data);
}

void
c_push_retval(prog_t *prog)
{
	int i;
	sym_t *sym;
	vo_t vo;
	int rv_tag;
	int nargs;
	int *argtype;
	dynafun_args_t *args;
	dynafun_data_t *d;

	TRACE;

	sym = sym_get(prog, SYM_FUNC);

	/*
	 * get function info
	 */
	rv_tag = (int) code_get(prog);
	nargs = (int) code_get(prog);
	argtype = (int *) VARR_GET(&prog->code, prog->ip);
	prog->ip += nargs;
	mpc_assert(prog, __FUNCTION__,
	    prog->ip <= prog->code.nused, "program code exhausted");

#if 0
	/*
	 * dump function info
	 */
	log(LOG_INFO, "%s: %d %s (%d args)",
	    __FUNCTION__, rv_tag, sym->name, nargs);
	for (i = 0; i < nargs; i++)
		log(LOG_INFO, "\t%d", argtype[i]);

	/*
	 * dump arguments
	 */
	log(LOG_INFO, "%s: dumping arguments", __FUNCTION__);
	for (i = 0; i < nargs; i++)
		log(LOG_INFO, "\t%d", peek(prog, i)->i);
#endif

	/*
	 * lookup dynafun
	 */
	d = dynafun_data_lookup(sym->name);
	mpc_assert(prog, __FUNCTION__, d != NULL, "%s: not found", sym->name);
	mpc_assert(prog, __FUNCTION__, d->fun != NULL, "%s: NULL fun", d->name);

	/*
	 * check dynafun args and retval
	 */
	mpc_assert(prog, __FUNCTION__,
	    nargs == d->nargs, "%s: called with %d args (expected %d)",
	    d->name, nargs, d->nargs);
	mpc_assert(prog, __FUNCTION__,
	    d->rv_tag == rv_tag,
	    "%s: rv type mismatch (want %d, got %d)",
	    d->name, rv_tag, d->rv_tag);
	for (i = 0; i < nargs; i++) {
		mpc_assert(prog, __FUNCTION__,
		    argtype[i] == d->argtype[i],
		    "%s: invalid arg %d type (want %d, got %d)",
		    d->name, i+1, argtype[i], d->argtype[i]);
	}

	/*
	 * This code is highly non-portable (see dynafun.c/dynafun_build_args)
	 */
	args = (dynafun_args_t *) VARR_GET(
	    &prog->data, prog->data.nused - nargs);
	mpc_assert(prog, __FUNCTION__,
	    prog->data.nused >= nargs, "data stack underflow");
	prog->data.nused -= nargs;

	if (rv_tag == MT_VOID) {
		vo.s = 0;
		d->fun(*args);
	} else
		vo.s = d->fun(*args);

	/*
	 * push the result
	 */
	push(prog, vo);
}

/*--------------------------------------------------------------------
 * binary operations
 */

#define INT_BOP_HEAD							\
	vo_t *v1, *v2;							\
	vo_t v;								\
									\
	TRACE;								\
									\
	v2 = pop(prog);							\
	v1 = pop(prog)

#define INT_BOP_TAIL(op)						\
	v.i = v1->i op v2->i;						\
	push(prog, v)

#define INT_BOP(c_bop_fun, op)						\
	void								\
	c_bop_fun(prog_t *prog)						\
	{								\
		INT_BOP_HEAD;						\
		INT_BOP_TAIL(op);					\
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

/*
 * '/' and '%' require additional checks
 */
void
c_bop_mod(prog_t *prog)
{
	INT_BOP_HEAD;
	mpc_assert(prog, __FUNCTION__, v2->i != 0, "division by zero");
	INT_BOP_TAIL(%);
}

void
c_bop_div(prog_t *prog)
{
	INT_BOP_HEAD;
	mpc_assert(prog, __FUNCTION__, v2->i != 0, "division by zero");
	INT_BOP_TAIL(/);
}

void
c_bop_ne_string(prog_t *prog)
{
	TRACE;
	/* XXX */
}

void
c_bop_eq_string(prog_t *prog)
{
	TRACE;
	/* XXX */
}

/*--------------------------------------------------------------------
 * assign operations
 */

#define ASSIGN_HEAD							\
	sym_t *sym;							\
	vo_t *vo;							\
									\
	TRACE;								\
									\
	sym = sym_get(prog, SYM_VAR);					\
	vo = pop(prog)

#define INT_ASSIGN_TAIL(op)						\
	sym->s.var.data.i op vo->i;					\
	push(prog, sym->s.var.data)

#define INT_ASSIGN(c_assign_fun, op)					\
	void								\
	c_assign_fun(prog_t *prog)					\
	{								\
		ASSIGN_HEAD;						\
		INT_ASSIGN_TAIL(op);					\
	}

void
c_assign(prog_t *prog)
{
	ASSIGN_HEAD;
	sym->s.var.data = *vo;
	push(prog, sym->s.var.data);
}

INT_ASSIGN(c_add_eq, +=)
INT_ASSIGN(c_sub_eq, -=)
INT_ASSIGN(c_mul_eq, *=)
INT_ASSIGN(c_and_eq, &=)
INT_ASSIGN(c_or_eq, |=)
INT_ASSIGN(c_xor_eq, ^=)
INT_ASSIGN(c_shl_eq, <<=)
INT_ASSIGN(c_shr_eq, >>=)

/*
 * /= and %= require additional checks
 */
void
c_div_eq(prog_t *prog)
{
	ASSIGN_HEAD;
	mpc_assert(prog, __FUNCTION__, vo->i != 0, "division by zero");
	INT_ASSIGN_TAIL(/=);
}

void
c_mod_eq(prog_t *prog)
{
	ASSIGN_HEAD;
	mpc_assert(prog, __FUNCTION__, vo->i != 0, "division by zero");
	INT_ASSIGN_TAIL(%=);
}

/*--------------------------------------------------------------------
 * static functions
 */

static void
push(prog_t *prog, vo_t vo)
{
	vo_t *v = varr_enew(&prog->data);
	*v = vo;
}

static vo_t *
pop(prog_t *prog)
{
	vo_t *vo = peek(prog, 0);
	prog->data.nused--;
	return vo;
}

static vo_t *
peek(prog_t *prog, size_t depth)
{
	vo_t *vo = (vo_t *) varr_get(
	    &prog->data, prog->data.nused - 1 - depth);
	mpc_assert(prog, __FUNCTION__, vo != NULL, "data stack underflow");
	return vo;
}

static void *
code_get(prog_t *prog)
{
	void **p = varr_get(&prog->code, prog->ip);
	mpc_assert(prog, __FUNCTION__, p != NULL, "program code exhausted");
	prog->ip++;
	return *p;
}

static sym_t *
sym_get(prog_t *prog, symtype_t symtype)
{
	const char *symname;
	sym_t *sym;

	symname = code_get(prog);
/*	log(LOG_INFO, "%s: '%s'", __FUNCTION__, symname); */

	sym = sym_lookup(prog, symname);
	mpc_assert(prog, __FUNCTION__,
	    sym != NULL, "%s: symbol not found", symname);
	mpc_assert(prog, __FUNCTION__,
	    sym->type == symtype, "symbol type mismatch (want %d, got %d)",
	    symtype, sym->type);

	return sym;
}

static void
mpc_assert(prog_t *prog, const char *ctx, int e, const char *fmt, ...)
{
	if (!e) {
		// XXX log error message
		longjmp(prog->jmpbuf, -1);
	}
}
