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
 * $Id: mpc_c.c,v 1.25 2001-09-12 19:42:55 fjoe Exp $
 */

#include <stdio.h>
#include <setjmp.h>

#include <typedef.h>
#include <log.h>
#include <memalloc.h>
#include <varr.h>
#include <hash.h>
#include <container.h>
#include <dynafun.h>
#include <util.h>
#include <flag.h>
#include <mprog.h>
#include <tables.h>

#include "mpc_impl.h"
#include "mpc_iter.h"

static void push(mpcode_t *mpc, vo_t vo);
static vo_t *pop(mpcode_t *mpc);
static vo_t *peek(mpcode_t *mpc, size_t depth);

static void *code_get(mpcode_t *mpc);
#define CODE_GET(t, mpc)	((t) (const void *) code_get(mpc))

static sym_t *sym_get(mpcode_t *mpc, symtype_t symtype);

static void mpc_assert(mpcode_t *mpc, const char *ctx, int e,
		       const char *fmt, ...)
	__attribute__ ((format(printf, 4, 5)));

static void dumpvar(const char *ctx, mpcode_t *mpc, sym_t *sym);

#define TRACE(a)							\
	do {								\
		if (IS_SET(mpc->mp->flags, MP_F_TRACE))			\
			log a;						\
	} while (0)

void
c_pop(mpcode_t *mpc)
{
	sym_t *sym;
	vo_t *v;
	int type_tag;

	TRACE((LOG_INFO, __FUNCTION__));

	v = pop(mpc);
	type_tag = CODE_GET(int, mpc);

	sym = (sym_t *) c_lookup(&mpc->syms, "$_");
	mpc_assert(mpc, __FUNCTION__,
	    sym != NULL, "$_: symbol not found");
	mpc_assert(mpc, __FUNCTION__,
	    sym->type == SYM_VAR, "$_: not a %d (got symtype %d)",
	    SYM_VAR, sym->type);

	sym->s.var.type_tag = type_tag;
	sym->s.var.data = *v;

	// dumpvar(__FUNCTION__, mpc, sym);
}

void
c_push_const(mpcode_t *mpc)
{
	vo_t vo;

	TRACE((LOG_INFO, __FUNCTION__));
	vo.s = code_get(mpc);
	push(mpc, vo);
}

void
c_push_var(mpcode_t *mpc)
{
	sym_t *sym;

	TRACE((LOG_INFO, __FUNCTION__));
	sym = sym_get(mpc, SYM_VAR);
	push(mpc, sym->s.var.data);

	if (IS_SET(mpc->mp->flags, MP_F_TRACE))
		dumpvar(__FUNCTION__, mpc, sym);
}

void
c_push_retval(mpcode_t *mpc)
{
	int i;
	sym_t *sym;
	vo_t vo;
	int rv_tag;
	int nargs;
	int *argtype;
	dynafun_args_t *args;
	dynafun_args_t dummy_args;
	dynafun_data_t *d;

	TRACE((LOG_INFO, __FUNCTION__));

	sym = sym_get(mpc, SYM_FUNC);

	TRACE((LOG_INFO, "%s: %s", __FUNCTION__, sym->name));

	/*
	 * get function info
	 */
	rv_tag = CODE_GET(int, mpc);
	nargs = CODE_GET(int, mpc);
	argtype = (int *) VARR_GET(&mpc->code, mpc->ip);
	mpc->ip += nargs;
	mpc_assert(mpc, __FUNCTION__,
	    (size_t) mpc->ip <= c_size(&mpc->code), "program code exhausted");

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
		log(LOG_INFO, "\t%d", peek(mpc, i)->i);
#endif

	/*
	 * lookup dynafun
	 */
	d = dynafun_data_lookup(sym->name);
	mpc_assert(mpc, __FUNCTION__, d != NULL, "%s: not found", sym->name);
	mpc_assert(mpc, __FUNCTION__, d->fun != NULL, "%s: NULL fun", d->name);

	/*
	 * check dynafun args and retval
	 */
	mpc_assert(mpc, __FUNCTION__,
	    nargs == d->nargs, "%s: called with %d args (expected %d)",
	    d->name, nargs, d->nargs);
	mpc_assert(mpc, __FUNCTION__,
	    d->rv_tag == rv_tag,
	    "%s: rv type mismatch (want %d, got %d)",
	    d->name, rv_tag, d->rv_tag);

	/*
	 * This code is highly non-portable (see dynafun.c/dynafun_build_args)
	 */
	if (nargs == 0)
		args = &dummy_args;
	else {
		mpc_assert(mpc, __FUNCTION__,
		    c_size(&mpc->data) >= (size_t) nargs,
		    "data stack underflow");
		args = (dynafun_args_t *) VARR_GET(
		    &mpc->data, c_size(&mpc->data) - nargs);
		mpc->data.nused -= nargs;
	}

	for (i = 0; i < nargs; i++) {
		if (d->argtype[i].type_tag != MT_PVOID
		&&  d->argtype[i].type_tag != MT_PCVOID
		&&  !TYPE_IS(d->argtype[i].type_tag, argtype[i])) {
			mpc_assert(mpc, __FUNCTION__,
			    FALSE,
			    "%s: invalid arg[%d] type (want %d, got %d)",
			    d->name, i+1, argtype[i], d->argtype[i].type_tag);
		}

		switch (d->argtype[i].type_tag) {
		case MT_INT:
		case MT_UINT:
		case MT_SIZE_T:
		case MT_BOOL:
		case MT_PVOID:
		case MT_PCVOID:
			break;

		case MT_STR:
		case MT_CHAR:
		case MT_OBJ:
		case MT_ROOM:
			mpc_assert(mpc, __FUNCTION__,
			    dynafun_check_arg(d, i, ((void **) args)[i]),
			    "dynafun_arg_check failed");
			break;

		default:
			mpc_assert(mpc, __FUNCTION__,
			    FALSE,
			    "dynafun_call: %s: invalid arg[%d] type %s (%d)",
			    d->name, i+1,
			    flag_string(mt_types, d->argtype[i].type_tag),
			    d->argtype[i].type_tag);
			break;
		}
	}

	if (rv_tag == MT_VOID) {
		vo.s = NULL;
		d->fun(*args);
	} else
		vo.s = d->fun(*args);

	/*
	 * push the result
	 */
	push(mpc, vo);
}

void
c_jmp(mpcode_t *mpc)
{
	TRACE((LOG_INFO, __FUNCTION__));

	mpc->ip = CODE_GET(int, mpc);
}

void
c_jmp_addr(mpcode_t *mpc)
{
	int addr;
	int *jmp_addr;

	TRACE((LOG_INFO, __FUNCTION__));

	addr = CODE_GET(int, mpc);
	jmp_addr = varr_get(&mpc->code, addr);
	mpc_assert(mpc, __FUNCTION__, jmp_addr != NULL,
		   "program code exhausted");
	mpc->ip = *jmp_addr;
}

void
c_if(mpcode_t *mpc)
{
	int next_addr;
	int then_addr;
	int else_addr;
	vo_t *v;

	TRACE((LOG_INFO, __FUNCTION__));

	next_addr = CODE_GET(int, mpc);
	then_addr = CODE_GET(int, mpc);
	else_addr = CODE_GET(int, mpc);

	execute(mpc, mpc->ip);
	v = pop(mpc);

	if (v->i) {
		/* execute then */
		mpc->ip = then_addr;
	} else if (else_addr != INVALID_ADDR) {
		/* execute else */
		mpc->ip = else_addr;
	} else
		mpc->ip = next_addr;
}

void
c_switch(mpcode_t *mpc)
{
	vo_t *v;

	int jt_offset;
	int next_addr;

	varr *jumptab;
	swjump_t *jump;
	swjump_t *default_jump;

	TRACE((LOG_INFO, __FUNCTION__));

	next_addr = CODE_GET(int, mpc);
	jt_offset = CODE_GET(int, mpc);

	/*
	 * get jumptab and its size
	 */
	jumptab = varr_get(&mpc->jumptabs, jt_offset);
	mpc_assert(mpc, __FUNCTION__,
	    jumptab != NULL, "jt_offset outside jumptab");
	default_jump = varr_get(jumptab, 0);
	mpc_assert(mpc, __FUNCTION__,
	    default_jump != NULL, "invalid jumptab");

	execute(mpc, mpc->ip);
	v = pop(mpc);

	/*
	 * lookup value in jumptab
	 */
	jump = (swjump_t *) bsearch(
	    &v->i, ((char *) jumptab->p) + jumptab->v_data->nsize,
	    jumptab->nused - 1, jumptab->v_data->nsize, cmpint);
	mpc->ip =
	    jump != NULL ? jump->addr :
	    default_jump->addr != INVALID_ADDR ? default_jump->addr :
	    next_addr;
}

void
c_quecolon(mpcode_t *mpc)
{
	vo_t *v0;
	int next_addr;
	int else_addr;

	TRACE((LOG_INFO, __FUNCTION__));

	next_addr = CODE_GET(int, mpc);
	else_addr = CODE_GET(int, mpc);

	v0 = pop(mpc);
	if (v0->i) {
		execute(mpc, mpc->ip);
		mpc->ip = next_addr;
	} else
		mpc->ip = else_addr;
}

void
c_op_foreach(mpcode_t *mpc)
{
	int next_addr;
	int body_addr;
	sym_t *sym;
	iterdata_t *id;
	iter_t *iter;
	vo_t v;
	dynafun_args_t *args;

	TRACE((LOG_INFO, __FUNCTION__));

	next_addr = CODE_GET(int, mpc);
	body_addr = CODE_GET(int, mpc);

	/* push iter args onto stack */
	execute(mpc, mpc->ip);

	/* c_cleanup_syms, block */
	code_get(mpc);
	code_get(mpc);

	/* c_foreach_next, next_addr */
	code_get(mpc);
	code_get(mpc);

	sym = sym_get(mpc, SYM_VAR);
	id = (iterdata_t *) code_get(mpc);
	iter = id->iter;

	/*
	 * This code is highly non-portable (see dynafun.c/dynafun_build_args)
	 */
	v.s = (const char *) &sym->s.var.data;
	push(mpc, v);

	v.s = (const char *) id;
	push(mpc, v);

	mpc_assert(mpc, __FUNCTION__,
	    c_size(&mpc->data) >= (size_t) (iter->init.nargs + 2),
	    "data stack underflow");
	args = (dynafun_args_t *) VARR_GET(
	    &mpc->data, c_size(&mpc->data) - (iter->init.nargs + 2));
	mpc->data.nused -= iter->init.nargs + 2;

	/* XXX check argtypes */
	iter->init.fun(*args);

	/* execute loop body */
	if (iter->cond(&sym->s.var.data, id))
		mpc->ip = body_addr;
	else
		mpc->ip = next_addr;
}

void
c_foreach_next(mpcode_t *mpc)
{
	sym_t *sym;
	iterdata_t *id;
	iter_t *iter;
	int next_addr;

	TRACE((LOG_INFO, __FUNCTION__));

	next_addr = CODE_GET(int, mpc);
	sym = sym_get(mpc, SYM_VAR);
	id = (iterdata_t *) code_get(mpc);
	iter = id->iter;

	/* get next */
	iter->next(&sym->s.var.data, id);
	if (!iter->cond(&sym->s.var.data, id))
		mpc->ip = next_addr;
}

void
c_declare(mpcode_t *mpc)
{
	sym_t sym;
	void *p;

	TRACE((LOG_INFO, __FUNCTION__));

	sym.name = str_dup(code_get(mpc));
	sym.type = SYM_VAR;
	sym.s.var.type_tag = CODE_GET(int, mpc);
	sym.s.var.is_const = FALSE;
	sym.s.var.block = CODE_GET(int, mpc);

	switch (sym.s.var.type_tag) {
	case MT_STR:
		sym.s.var.data.s = NULL;
		break;

	case MT_INT:
	default:
		sym.s.var.data.i = 0;
		break;
	};

	p = c_insert(&mpc->syms, sym.name, &sym);
	if (p == NULL)
		sym_destroy(&sym);
	mpc_assert(mpc, __FUNCTION__,
	    p != NULL, "%s: duplicate symbol", sym.name);
	sym_destroy(&sym);
}

void
c_declare_assign(mpcode_t *mpc)
{
	sym_t sym;
	sym_t *s;
	vo_t *v;

	TRACE((LOG_INFO, __FUNCTION__));

	sym.name = str_dup(code_get(mpc));
	sym.type = SYM_VAR;
	sym.s.var.type_tag = CODE_GET(int, mpc);
	sym.s.var.is_const = FALSE;
	sym.s.var.block = CODE_GET(int, mpc);
	TRACE((LOG_INFO, "%s: block %d", __FUNCTION__, sym.s.var.block));

	s = (sym_t *) c_insert(&mpc->syms, sym.name, &sym);
	if (s == NULL)
		sym_destroy(&sym);
	mpc_assert(mpc, __FUNCTION__,
	    s != NULL, "%s: duplicate symbol", sym.name);
	sym_destroy(&sym);

	v = pop(mpc);
	s->s.var.data = *v;
}

void
c_cleanup_syms(mpcode_t *mpc)
{
	int block;

	TRACE((LOG_INFO, __FUNCTION__));

	block = CODE_GET(int, mpc);
	cleanup_syms(mpc, block);
}

void
c_return(mpcode_t *mpc)
{
	TRACE((LOG_INFO, __FUNCTION__));

	mpc->ip = INVALID_ADDR;
}

void
c_return_0(mpcode_t *mpc)
{
	sym_t *sym;

	TRACE((LOG_INFO, __FUNCTION__));

	sym = (sym_t *) c_lookup(&mpc->syms, "$_");
	mpc_assert(mpc, __FUNCTION__,
	    sym != NULL, "$_: symbol not found");
	mpc_assert(mpc, __FUNCTION__,
	    sym->type == SYM_VAR, "$_: not a %d (got symtype %d)",
	    SYM_VAR, sym->type);

	sym->s.var.type_tag = MT_INT;
	sym->s.var.data.i = 0;

	mpc->ip = INVALID_ADDR;
}

/*--------------------------------------------------------------------
 * binary operations
 */

#define INT_BOP_HEAD							\
	vo_t *v1, *v2;							\
	vo_t v;								\
									\
	TRACE((LOG_INFO, __FUNCTION__));				\
									\
	v2 = pop(mpc);							\
	v1 = pop(mpc)

#define INT_BOP_TAIL(op)						\
	v.i = v1->i op v2->i;						\
	push(mpc, v)

#define INT_BOP(c_bop_fun, op)						\
	void								\
	c_bop_fun(mpcode_t *mpc)						\
	{								\
		INT_BOP_HEAD;						\
		INT_BOP_TAIL(op);					\
	}

/*
 * special case - '||' and '&&'
 */
void
c_bop_lor(mpcode_t *mpc)
{
	vo_t *v1;
	vo_t v;
	int next_addr;

	next_addr = CODE_GET(int, mpc);

	v1 = pop(mpc);
	if (v1->i) {
		v.i = 1;
		mpc->ip = next_addr;
	} else {
		vo_t *v2;

		execute(mpc, mpc->ip);
		v2 = pop(mpc);
		v.i = v1->i || v2->i;
	}

	push(mpc, v);
}

void
c_bop_land(mpcode_t *mpc)
{
	vo_t *v1;
	vo_t v;
	int next_addr;

	next_addr = CODE_GET(int, mpc);

	v1 = pop(mpc);
	if (!v1->i) {
		v.i = 0;
		mpc->ip = next_addr;
	} else {
		vo_t *v2;

		execute(mpc, mpc->ip);
		v2 = pop(mpc);
		v.i = v1->i && v2->i;
	}

	push(mpc, v);
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
c_bop_mod(mpcode_t *mpc)
{
	INT_BOP_HEAD;
	mpc_assert(mpc, __FUNCTION__, v2->i != 0, "division by zero");
	INT_BOP_TAIL(%);
}

void
c_bop_div(mpcode_t *mpc)
{
	INT_BOP_HEAD;
	mpc_assert(mpc, __FUNCTION__, v2->i != 0, "division by zero");
	INT_BOP_TAIL(/);
}

void
c_bop_ne_string(mpcode_t *mpc)
{
	vo_t *v1;
	vo_t *v2;
	vo_t v;

	TRACE((LOG_INFO, __FUNCTION__));

	v2 = pop(mpc);
	v1 = pop(mpc);

	v.i = str_cscmp(v1->s, v2->s);
	push(mpc, v);
}

void
c_bop_eq_string(mpcode_t *mpc)
{
	vo_t *v1;
	vo_t *v2;
	vo_t v;

	TRACE((LOG_INFO, __FUNCTION__));

	v2 = pop(mpc);
	v1 = pop(mpc);

	v.i = !str_cscmp(v1->s, v2->s);
	push(mpc, v);
}

/*--------------------------------------------------------------------
 * unary operations
 */

#define INT_UOP(c_uop_fun, op)						\
	void								\
	c_uop_fun(mpcode_t *mpc)					\
	{								\
		vo_t *v1;						\
		vo_t v;							\
									\
		TRACE((LOG_INFO, __FUNCTION__));			\
									\
		v1 = pop(mpc);						\
		v.i = op v1->i;						\
		push(mpc, v);						\
	}

INT_UOP(c_uop_not, !)
INT_UOP(c_uop_compl, ~)
INT_UOP(c_uop_minus, -)

/*--------------------------------------------------------------------
 * incdec operations
 */

#define INT_INCDEC(c_incdec_fun, preop, postop)				\
	void								\
	c_incdec_fun(mpcode_t *mpc)					\
	{								\
		sym_t *sym;						\
		vo_t v;							\
									\
		TRACE((LOG_INFO, __FUNCTION__));			\
									\
		sym = sym_get(mpc, SYM_VAR);				\
		v.i = preop sym->s.var.data.i postop;			\
		push(mpc, v);						\
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
	TRACE((LOG_INFO, __FUNCTION__));				\
									\
	sym = sym_get(mpc, SYM_VAR);					\
	vo = pop(mpc)

#define INT_ASSIGN_TAIL(op)						\
	sym->s.var.data.i op vo->i;					\
	push(mpc, sym->s.var.data)

#define INT_ASSIGN(c_assign_fun, op)					\
	void								\
	c_assign_fun(mpcode_t *mpc)					\
	{								\
		ASSIGN_HEAD;						\
		INT_ASSIGN_TAIL(op);					\
	}

void
c_assign(mpcode_t *mpc)
{
	ASSIGN_HEAD;
	sym->s.var.data = *vo;
	push(mpc, sym->s.var.data);
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
c_div_eq(mpcode_t *mpc)
{
	ASSIGN_HEAD;
	mpc_assert(mpc, __FUNCTION__, vo->i != 0, "division by zero");
	INT_ASSIGN_TAIL(/=);
}

void
c_mod_eq(mpcode_t *mpc)
{
	ASSIGN_HEAD;
	mpc_assert(mpc, __FUNCTION__, vo->i != 0, "division by zero");
	INT_ASSIGN_TAIL(%=);
}

/*--------------------------------------------------------------------
 * static functions
 */

static void
push(mpcode_t *mpc, vo_t vo)
{
	vo_t *v = varr_enew(&mpc->data);
	*v = vo;
}

static vo_t *
pop(mpcode_t *mpc)
{
	vo_t *vo = peek(mpc, 0);
	mpc->data.nused--;
	return vo;
}

static vo_t *
peek(mpcode_t *mpc, size_t depth)
{
	vo_t *vo = (vo_t *) varr_get(
	    &mpc->data, c_size(&mpc->data) - 1 - depth);
	mpc_assert(mpc, __FUNCTION__, vo != NULL, "data stack underflow");
	return vo;
}

static void *
code_get(mpcode_t *mpc)
{
	void **p = varr_get(&mpc->code, mpc->ip);
	mpc_assert(mpc, __FUNCTION__, p != NULL, "program code exhausted");
	mpc->ip++;
	return *p;
}

static sym_t *
sym_get(mpcode_t *mpc, symtype_t symtype)
{
	const char *symname;
	sym_t *sym;

	symname = code_get(mpc);
/*	log(LOG_INFO, "%s: '%s'", __FUNCTION__, symname); */

	sym = sym_lookup(mpc, symname);
	mpc_assert(mpc, __FUNCTION__,
	    sym != NULL, "%s: symbol not found", symname);
	mpc_assert(mpc, __FUNCTION__,
	    sym->type == symtype, "symbol type mismatch (want %d, got %d)",
	    symtype, sym->type);

	return sym;
}

static void
mpc_assert(mpcode_t *mpc, const char *ctx, int e, const char *fmt, ...)
{
	if (!e) {
		char buf[MAX_STRING_LENGTH];
		va_list ap;

		va_start(ap, fmt);
		vsnprintf(buf, sizeof(buf), fmt, ap);
		va_end(ap);

		fprintf(stderr, "Runtime error: %s: %s\n", ctx, buf);

		longjmp(mpc->jmpbuf, -1);
	}
}

static void
dumpvar(const char *ctx, mpcode_t *mpc, sym_t *sym)
{
	UNUSED_ARG(mpc);

	switch (sym->s.var.type_tag) {
	case MT_INT:
		log(LOG_INFO, "%s: %s: (int) %d",
		    ctx, sym->name, sym->s.var.data.i);
		break;

	case MT_STR:
		log(LOG_INFO, "%s: %s: (string) '%s'",
		    ctx, sym->name, sym->s.var.data.s);
		break;

	default:
		log(LOG_INFO, "%s: %s: (%s) %p",
		    ctx, sym->name,
		    flag_string(mpc_types, sym->s.var.type_tag),
		    sym->s.var.data.ch);
		break;
	}
}
