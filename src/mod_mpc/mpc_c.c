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
 * $Id: mpc_c.c,v 1.14 2001-08-05 16:36:48 fjoe Exp $
 */

#include <stdio.h>
#include <stdarg.h>
#include <setjmp.h>

#include <typedef.h>
#include <log.h>
#include <memalloc.h>
#include <varr.h>
#include <hash.h>
#include <dynafun.h>
#include <util.h>

#include "mpc_impl.h"
#include "mpc_iter.h"

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
	sym_t *sym;
	vo_t *v;
	int type_tag;

	TRACE;

	v = pop(prog);
	type_tag = (int) code_get(prog);

	sym = (sym_t *) hash_lookup(&prog->syms, "$_");
	mpc_assert(prog, __FUNCTION__,
	    sym != NULL, "$_: symbol not found");
	mpc_assert(prog, __FUNCTION__,
	    sym->type == SYM_VAR, "$_: not a %d (got symtype %d)",
	    SYM_VAR, sym->type);

	sym->s.var.type_tag = type_tag;
	sym->s.var.data = *v;

	switch (type_tag) {
	case MT_INT:
		/* fprintf(stderr, "got: %d\n", v->i); */
		break;

	default:
		/* fprintf(stderr, "got: (type '%s')\n",
			flag_string(mpc_types, type_tag)); */
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
	    prog->ip <= varr_size(&prog->code), "program code exhausted");

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
		    argtype[i] == d->argtype[i].type_tag,
		    "%s: invalid arg %d type (want %d, got %d)",
		    d->name, i+1, argtype[i], d->argtype[i].type_tag);
	}

	/*
	 * This code is highly non-portable (see dynafun.c/dynafun_build_args)
	 */
	args = (dynafun_args_t *) VARR_GET(
	    &prog->data, varr_size(&prog->data) - nargs);
	mpc_assert(prog, __FUNCTION__,
	    varr_size(&prog->data) >= nargs, "data stack underflow");
	varr_size(&prog->data) -= nargs;

	if (rv_tag == MT_VOID) {
		vo.s = NULL;
		d->fun(*args);
	} else
		vo.s = d->fun(*args);

	/*
	 * push the result
	 */
	push(prog, vo);
}

void
c_jmp(prog_t *prog)
{
	TRACE;

	prog->ip = (int) code_get(prog);
}

void
c_jmp_addr(prog_t *prog)
{
	int addr;
	int *jmp_addr;

	TRACE;

	addr = (int) code_get(prog);
	jmp_addr = varr_get(&prog->code, addr);
	mpc_assert(prog, __FUNCTION__, jmp_addr != NULL,
		   "program code exhausted");
	prog->ip = *jmp_addr;
}

void
c_if(prog_t *prog)
{
	int next_addr;
	int then_addr;
	int else_addr;
	vo_t *v;

	TRACE;

	next_addr = (int) code_get(prog);
	then_addr = (int) code_get(prog);
	else_addr = (int) code_get(prog);

	execute(prog, prog->ip);
	v = pop(prog);

	if (v->i) {
		/* execute then */
		prog->ip = then_addr;
	} else if (else_addr != INVALID_ADDR) {
		/* execute else */
		prog->ip = else_addr;
	} else
		prog->ip = next_addr;
}

void
c_switch(prog_t *prog)
{
	vo_t *v;

	int jt_offset;
	int next_addr;

	varr *jumptab;
	swjump_t *jump;
	swjump_t *default_jump;

	TRACE;

	next_addr = (int) code_get(prog);
	jt_offset = (int) code_get(prog);

	/*
	 * get jumptab and its size
	 */
	jumptab = varr_get(&prog->jumptabs, jt_offset);
	mpc_assert(prog, __FUNCTION__,
	    jumptab != NULL, "jt_offset outside jumptab");
	default_jump = varr_get(jumptab, 0);
	mpc_assert(prog, __FUNCTION__,
	    default_jump != NULL, "invalid jumptab");

	execute(prog, prog->ip);
	v = pop(prog);

	/*
	 * lookup value in jumptab
	 */
	jump = (swjump_t *) bsearch(
	    &v->i, ((char *) jumptab->p) + jumptab->v_data->nsize,
	    jumptab->nused - 1, jumptab->v_data->nsize, cmpint);
	prog->ip =
	    jump != NULL ? jump->addr :
	    default_jump->addr != INVALID_ADDR ? default_jump->addr :
	    next_addr;
}

void
c_quecolon(prog_t *prog)
{
	vo_t *v0;
	int next_addr;
	int else_addr;

	TRACE;

	next_addr = (int) code_get(prog);
	else_addr = (int) code_get(prog);

	v0 = pop(prog);
	if (v0->i) {
		execute(prog, prog->ip);
		prog->ip = next_addr;
	} else
		prog->ip = else_addr;
}

void
c_foreach(prog_t *prog)
{
	int next_addr;
	int body_addr;
	sym_t *sym;
	iterdata_t *id;
	iter_t *iter;
	vo_t v;
	dynafun_args_t *args;

	TRACE;

	next_addr = (int) code_get(prog);
	body_addr = (int) code_get(prog);

	/* push iter args onto stack */
	execute(prog, prog->ip);

	/* c_cleanup_syms, block */
	code_get(prog);
	code_get(prog);

	/* c_foreach_next, next_addr */
	code_get(prog);
	code_get(prog);

	sym = sym_get(prog, SYM_VAR);
	id = (iterdata_t *) code_get(prog);
	iter = id->iter;

	/*
	 * This code is highly non-portable (see dynafun.c/dynafun_build_args)
	 */
	v.s = (const char *) &sym->s.var.data;
	push(prog, v);

	v.s = (const char *) id;
	push(prog, v);

	args = (dynafun_args_t *) VARR_GET(
	    &prog->data, varr_size(&prog->data) - (iter->init.nargs + 2));
	mpc_assert(prog, __FUNCTION__,
	    varr_size(&prog->data) >= iter->init.nargs + 2,
	    "data stack underflow");
	iter->init.fun(*args);
	varr_size(&prog->data) -= iter->init.nargs + 2;

	/* execute loop body */
	if (iter->cond(&sym->s.var.data, id))
		prog->ip = body_addr;
	else
		prog->ip = next_addr;
}

void
c_foreach_next(prog_t *prog)
{
	sym_t *sym;
	iterdata_t *id;
	iter_t *iter;
	int next_addr;

	TRACE;

	next_addr = (int) code_get(prog);
	sym = sym_get(prog, SYM_VAR);
	id = (iterdata_t *) code_get(prog);
	iter = id->iter;

	/* get next */
	iter->next(&sym->s.var.data, id);
	if (!iter->cond(&sym->s.var.data, id))
		prog->ip = next_addr;
}

void
c_declare(prog_t *prog)
{
	sym_t sym;
	void *p;

	TRACE;

	sym.name = str_dup(code_get(prog));
	sym.type = SYM_VAR;
	sym.s.var.type_tag = (int) code_get(prog);
	sym.s.var.is_const = FALSE;
	sym.s.var.block = (int) code_get(prog);

	switch (sym.s.var.type_tag) {
	case MT_STR:
		sym.s.var.data.s = NULL;
		break;

	case MT_INT:
	default:
		sym.s.var.data.i = 0;
		break;
	};

	p = hash_insert(&prog->syms, sym.name, &sym);
	if (p == NULL)
		sym_destroy(&sym);
	mpc_assert(prog, __FUNCTION__,
	    p != NULL, "%s: duplicate symbol", sym.name);
	sym_destroy(&sym);
}

void
c_declare_assign(prog_t *prog)
{
	sym_t sym;
	sym_t *s;
	vo_t *v;

	TRACE;

	sym.name = str_dup(code_get(prog));
	sym.type = SYM_VAR;
	sym.s.var.type_tag = (int) code_get(prog);
	sym.s.var.is_const = FALSE;
	sym.s.var.block = (int) code_get(prog);

	s = (sym_t *) hash_insert(&prog->syms, sym.name, &sym);
	if (s == NULL)
		sym_destroy(&sym);
	mpc_assert(prog, __FUNCTION__,
	    s != NULL, "%s: duplicate symbol", sym.name);
	sym_destroy(&sym);

	v = pop(prog);
	s->s.var.data = *v;
}

void
c_cleanup_syms(prog_t *prog)
{
	int block;

	TRACE;

	block = (int) code_get(prog);
	cleanup_syms(prog, block);
}

void
c_return(prog_t *prog)
{
	TRACE;

	prog->ip = INVALID_ADDR;
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

/*
 * special case - '||' and '&&'
 */
void
c_bop_lor(prog_t *prog)
{
	vo_t *v1;
	vo_t v;
	int next_addr;

	next_addr = (int) code_get(prog);

	v1 = pop(prog);
	if (v1->i) {
		v.i = 1;
		prog->ip = next_addr;
	} else {
		vo_t *v2;

		execute(prog, prog->ip);
		v2 = pop(prog);
		v.i = v1->i || v2->i;
	}

	push(prog, v);
}

void
c_bop_land(prog_t *prog)
{
	vo_t *v1;
	vo_t v;
	int next_addr;

	next_addr = (int) code_get(prog);

	v1 = pop(prog);
	if (!v1->i) {
		v.i = 0;
		prog->ip = next_addr;
	} else {
		vo_t *v2;

		execute(prog, prog->ip);
		v2 = pop(prog);
		v.i = v1->i && v2->i;
	}

	push(prog, v);
}

INT_BOP(c_bop_or, |)
INT_BOP(c_bop_xor, ^)
INT_BOP(c_bop_and, &)

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
	vo_t *v1;
	vo_t *v2;
	vo_t v;

	TRACE;

	v2 = pop(prog);
	v1 = pop(prog);

	v.i = str_cscmp(v1->s, v2->s);
	push(prog, v);
}

void
c_bop_eq_string(prog_t *prog)
{
	vo_t *v1;
	vo_t *v2;
	vo_t v;

	TRACE;

	v2 = pop(prog);
	v1 = pop(prog);

	v.i = !str_cscmp(v1->s, v2->s);
	push(prog, v);
}

/*--------------------------------------------------------------------
 * unary operations
 */

#define INT_UOP(c_uop_fun, op)						\
	void								\
	c_uop_fun(prog_t *prog)						\
	{								\
		vo_t *v1;						\
		vo_t v;							\
									\
		TRACE;							\
									\
		v1 = pop(prog);						\
		v.i = op v1->i;						\
		push(prog, v);						\
	}

INT_UOP(c_uop_not, !)
INT_UOP(c_uop_compl, ~)
INT_UOP(c_uop_minus, -)

/*--------------------------------------------------------------------
 * incdec operations
 */

#define INT_INCDEC(c_incdec_fun, preop, postop)				\
	void								\
	c_incdec_fun(prog_t *prog)					\
	{								\
		sym_t *sym;						\
		vo_t v;							\
									\
		TRACE;							\
									\
		sym = sym_get(prog, SYM_VAR);				\
		v.i = preop sym->s.var.data.i postop;			\
		push(prog, v);						\
	}

INT_INCDEC(c_postinc, , ++)
INT_INCDEC(c_postdec, , --)
INT_INCDEC(c_preinc, ++, )
INT_INCDEC(c_predec, --, )

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
	varr_size(&prog->data)--;
	return vo;
}

static vo_t *
peek(prog_t *prog, size_t depth)
{
	vo_t *vo = (vo_t *) varr_get(
	    &prog->data, varr_size(&prog->data) - 1 - depth);
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
		char buf[MAX_STRING_LENGTH];
		va_list ap;

		va_start(ap, fmt);
		vsnprintf(buf, sizeof(buf), fmt, ap);
		va_end(ap);

		fprintf(stderr, "Runtime error: %s: %s\n", ctx, buf);

		longjmp(prog->jmpbuf, -1);
	}
}
