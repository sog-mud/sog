%{

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
 * $Id: mpc.y,v 1.37 2001-09-13 12:02:57 fjoe Exp $
 */

/*
 * Inspired by
 * - `hoc' from The Unix Programming Environment, by Brian Kernighan
 * and Rob Pike (Addison-Wesley, 1984).
 * - MudOS 0.9.19 copyright 1991-1992 by Erik Kay, Adam Beeman,
 * Stephan Iannce and John Garnett, LPmud copyright by Lars Pensj|, 1990, 1991.
 */

#include <sys/stat.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>

#if defined(MPC)
#include <dlfcn.h>
#endif

#include <typedef.h>
#include <varr.h>
#include <hash.h>
#include <container.h>
#include <avltree.h>
#include <dynafun.h>
#include <memalloc.h>
#include <buffer.h>
#include <strkey_hash.h>
#include <log.h>
#include <util.h>
#include <flag.h>
#include <mprog.h>

#if defined(MPC)
#include <module.h>
#endif

#include "mpc_impl.h"
#include "mpc_const.h"
#include "mpc_iter.h"

#define YYPARSE_PARAM mpc
#define YYPARSE_PARAM_TYPE mpcode_t *

#if 0
#if defined(MPC)
#define YYDEBUG 1
#endif
#endif

#undef yylex
#define yylex() mpc_lex(mpc)

#undef yyerror
#define yyerror(a) mpc_error(mpc, a)

#define yysslim mpc_sslim
#define yystacksize mpc_stacksize

int _mprog_compile(mprog_t *mp);
int _mprog_execute(mprog_t *mp,
		   void *arg1, void *arg2, void *arg3, void *arg4);

/*--------------------------------------------------------------------
 * argtype stack manipulation functions
 */

/**
 * Push argument type on argument type stack
 */
static void
argtype_push(mpcode_t *mpc, int type_tag)
{
	int *t = varr_enew(&mpc->args);
	*t = type_tag;
}

/**
 * Pop n arguments from argument type stack
 */
static void
argtype_popn(mpcode_t *mpc, size_t n)
{
	assert(c_size(&mpc->args) >= n);
	mpc->args.nused -= n;
}

/**
 * Get argument type from argument type stack
 */
static int
argtype_get(mpcode_t *mpc, int n, int idx)
{
	int *t = varr_get(&mpc->args, c_size(&mpc->args) - n + idx);
	if (t == NULL)
		return MT_NONE;
	return *t;
}

/*--------------------------------------------------------------------
 * code generation functions
 */

/**
 * Append opcode to program (compiled) text
 */
static int
code(mpcode_t *mpc, const void *opcode)
{
	int old_ip = c_size(&mpc->code);
	const void **o = varr_enew(&mpc->code);
	*o = opcode;
	return old_ip;
}

/**
 * Append two opcodes to program (compiled) text
 */
static inline void
code2(mpcode_t *mpc, const void *opcode1, const void *opcode2)
{
	code(mpc, opcode1);
	code(mpc, opcode2);
}

/**
 * Append three opcodes to program (compiled) text
 */
static inline void
code3(mpcode_t *mpc,
      const void *opcode1, const void *opcode2, const void *opcode3)
{
	code(mpc, opcode1);
	code(mpc, opcode2);
	code(mpc, opcode3);
}

#define SYM_LOOKUP(sym, ident, symtype)					\
	do {								\
		/*							\
		 * lookup symbol					\
		 */							\
		if ((sym = sym_lookup(mpc, (ident))) == NULL) {		\
			compile_error(mpc,				\
			    "%s: unknown identifier", (ident));		\
			YYERROR;					\
		}							\
									\
		if (sym->type != (symtype)) {				\
			compile_error(mpc,				\
			    "%s: not a %d", (ident), (symtype));	\
			YYERROR;					\
		}							\
	} while (0)

#define OP_INVALID_OPERAND(opname, type_tag)				\
	do {								\
		compile_error(mpc,					\
		    "invalid operand for '%s' (type '%s')",		\
		    (opname), flag_string(mpc_types, (type_tag)));	\
		YYERROR;						\
	} while (0)

#define INT_OP(opname, type_tag, c_fun)					\
	do {								\
		switch (type_tag) {					\
		case MT_INT:						\
			code(mpc, c_fun);				\
			break;						\
									\
		default:						\
			OP_INVALID_OPERAND((opname), (type_tag));	\
			/* NOTREACHED */				\
		}							\
	} while (0)

#define BOP_CHECK_TYPES(opname, type_tag1, type_tag2)			\
	do {								\
		if (!TYPE_IS((type_tag1), (type_tag2))			\
		&&  !TYPE_IS((type_tag2), (type_tag1)))	{		\
			compile_error(mpc,				\
			    "invalid operand types for '%s' ('%s' and '%s')",\
			    (opname),					\
			    flag_string(mpc_types, (type_tag1)),	\
			    flag_string(mpc_types, (type_tag2)));	\
			YYERROR;					\
		}							\
	} while (0)

#define INT_BOP(opname, c_bop_fun, type_tag1, type_tag2, rv)		\
	do {								\
		BOP_CHECK_TYPES((opname), (type_tag1), (type_tag2));	\
		INT_OP((opname), (type_tag1), (c_bop_fun));		\
		(rv) = MT_INT;						\
	} while (0)

#define INT_UOP(opname, c_uop_fun, type_tag, rv)			\
	do {								\
		INT_OP(opname, type_tag, c_uop_fun);			\
		(rv) = MT_INT;						\
	} while (0)

#define INCDEC_UOP(opname, c_uop_fun, ident, rv)			\
	do {								\
		sym_t *sym;						\
		SYM_LOOKUP(sym, (ident), SYM_VAR);			\
		INT_UOP((opname), c_uop_fun, sym->s.var.type_tag, (rv));\
		code(mpc, sym->name);					\
	} while (0)

#define PUSH_EXPLICIT(addr)						\
	do {								\
		int *p = (int *) varr_enew(&mpc->cstack);		\
		*p = addr;						\
	} while (0)

#define PUSH_ADDR()	PUSH_EXPLICIT(c_size(&mpc->code))

#define PEEK_ADDR(addr)							\
	do {								\
		int *p = (int *) varr_get(				\
		    &mpc->cstack, c_size(&mpc->cstack) - 1);		\
		if (p == NULL) {					\
			compile_error(mpc, "compiler stack underflow");	\
			YYERROR;					\
		}							\
		addr = *p;						\
	} while (0)

#define POP_ADDR(addr)							\
	do {								\
		PEEK_ADDR(addr);					\
		mpc->cstack.nused--;					\
	} while (0)

#define CODE(addr)	((int *) VARR_GET(&mpc->code, (addr)))

#define DELIMITER_ADDR	-2

%}

%union {
	int number;
	const char *string;
	int type_tag;
	c_fun cfun;
	iter_t *iter;
}

%token L_IDENT L_TYPE L_INT L_STRING L_ITER
%token L_IF L_SWITCH L_CASE L_DEFAULT L_ELSE L_FOREACH L_CONTINUE L_BREAK
%token L_RETURN
%token L_ADD_EQ L_SUB_EQ L_DIV_EQ L_MUL_EQ L_MOD_EQ L_AND_EQ L_OR_EQ L_XOR_EQ
%token L_SHL_EQ L_SHR_EQ

%right '='
%right '?'
%left L_LOR
%left L_LAND
%left '|'
%left '^'
%left '&'
%left L_EQ L_NE
%left '>' L_GE '<' L_LE
%left L_SHL L_SHR
%left '+' '-'
%left '*' '%' '/'
%right L_NOT '~'
%nonassoc L_INC L_DEC

%type <type_tag> expr comma_expr L_TYPE
%type <number> expr_list expr_list_ne L_INT int_const
%type <string> L_IDENT L_STRING
%type <cfun> assign
%type <iter> L_ITER

%%

stmt_list:	/* empty */
	| stmt_list stmt
	| stmt_list error ';'
	;

stmt:	';'
	| L_TYPE L_IDENT ';' {
		sym_t *s;

		if ((s = c_insert(&mpc->syms, $2)) == NULL) {
			compile_error(mpc, "%s: duplicate symbol", $2);
			YYERROR;
		}

		s->name = str_dup($2);
		s->type = SYM_VAR;
		s->s.var.type_tag = $1;
		s->s.var.is_const = FALSE;
		s->s.var.block = mpc->curr_block;

		switch ($1) {
		case MT_STR:
			s->s.var.data.s = NULL;
			break;

		case MT_INT:
		default:
			s->s.var.data.i = 0;
			break;
		};

		code3(mpc, c_declare, s->name, (void *) s->s.var.type_tag);
		code(mpc, (void *) s->s.var.block);
	}
	| L_TYPE L_IDENT '=' comma_expr ';' {
		sym_t *s;

		if ($1 != $4) {
			compile_error(mpc, "type mismatch (%d vs. %d)",
				      $1, $4);
			YYERROR;
		}

		if ((s = c_insert(&mpc->syms, $2)) == NULL) {
			compile_error(mpc, "%s: duplicate symbol", $2);
			YYERROR;
		}

		s->name = str_dup($2);
		s->type = SYM_VAR;
		s->s.var.type_tag = $1;
		s->s.var.is_const = FALSE;
		s->s.var.block = mpc->curr_block;

		code3(mpc,
		    c_declare_assign, s->name, (void *) s->s.var.type_tag);
		code(mpc, (void *) s->s.var.block);
	}
	| comma_expr ';' {
		/*
		 * pop calculated value from stack
		 */
		code2(mpc, c_pop, (void *) $1);
	}
	| label
	| if | switch | case_label | default | break | foreach | continue
	| return
	| '{' {
		mpc->curr_block++;
	} stmt_list '}' {
		code2(mpc, c_cleanup_syms, (void *) mpc->curr_block);
		cleanup_syms(mpc, mpc->curr_block--);
	}
	;

label:	L_IDENT ':' {
		sym_t *s;

		if ((s = c_insert(&mpc->syms, $1)) == NULL) {
			compile_error(mpc, "%s: duplicate symbol", $1);
			YYERROR;
		}

		s->name = str_dup($1);
		s->type = SYM_LABEL;
		s->s.label.addr = c_size(&mpc->code);
	}
	;

if:	L_IF {
		code(mpc, c_if);
		PUSH_ADDR();
		/* next stmt, then, else */
		code(mpc, (void *) INVALID_ADDR);
		code2(mpc, (void *) INVALID_ADDR, (void *) INVALID_ADDR);
	} '(' comma_expr ')' {
		int addr;

		code(mpc, c_stop);

		/* store then addr */
		PEEK_ADDR(addr);
		CODE(addr)[1] = c_size(&mpc->code);
	} stmt optional_else {
		int addr;

		/* store next stmt addr */
		POP_ADDR(addr);
		CODE(addr)[0] = c_size(&mpc->code);

		code2(mpc, c_cleanup_syms, (void *) (mpc->curr_block + 1));
	}
	;

optional_else: /* empty */
	| L_ELSE {
		int addr;

		PEEK_ADDR(addr);

		/* jmp to next stmt */
		code2(mpc, c_jmp_addr, (void *) addr);

		/* store else addr */
		CODE(addr)[2] = c_size(&mpc->code);
	} stmt
	;

switch:	L_SWITCH {
		varr *jumptab;
		swjump_t *jump;

		code(mpc, c_switch);

		PUSH_ADDR();
		PUSH_EXPLICIT(mpc->curr_break_addr);
		PUSH_EXPLICIT(mpc->curr_jumptab);

		/* jumptab[0] reserved for 'default:' */
		jumptab = (varr *) varr_get(
		    &mpc->jumptabs, ++mpc->curr_jumptab);
		if (jumptab == NULL)
			jumptab = varr_enew(&mpc->jumptabs);
		else
			c_erase(jumptab);
		jump = varr_enew(jumptab);
		jump->addr = INVALID_ADDR;

		/* next_addr, jt_offset */
		code(mpc, (void *) DELIMITER_ADDR);
		mpc->curr_break_addr = c_size(&mpc->code) - 1;
		code(mpc, (void *) mpc->curr_jumptab);
	} '(' expr ')' {
		code(mpc, c_stop);
	} stmt {
		int addr;
		int next_break_addr;
		varr *jumptab;

		/*
		 * emit `break'
		 * (if last case label/default does not have break)
		 */
		code(mpc, c_jmp);
		code(mpc, (void *) mpc->curr_break_addr);
		mpc->curr_break_addr = c_size(&mpc->code) - 1;

		/*
		 * traverse linked list of break addresses
		 * and emit address to jump
		 */
		for (; mpc->curr_break_addr != DELIMITER_ADDR;
		     mpc->curr_break_addr = next_break_addr) {
			next_break_addr = CODE(mpc->curr_break_addr)[0];
			CODE(mpc->curr_break_addr)[0] = c_size(&mpc->code);
		}

		/* qsort jumptab */
		jumptab = varr_get(&mpc->jumptabs, mpc->curr_jumptab);
		if (jumptab == NULL) {
			compile_error(mpc, "invalid jumptab");
			YYERROR;
		}

		if (jumptab->nused < 2) {
			compile_error(mpc,
			     "switch without case labels");
			YYERROR;
		}

		qsort(((char *) jumptab->p) + jumptab->v_data->nsize,
		    jumptab->nused - 1, jumptab->v_data->nsize, cmpint);

		POP_ADDR(mpc->curr_jumptab);
		POP_ADDR(mpc->curr_break_addr);
		POP_ADDR(addr);

		/* store next addr */
		CODE(addr)[0] = c_size(&mpc->code);
		code2(mpc, c_cleanup_syms, (void *) (mpc->curr_block + 1));
	}
	;

case_label: L_CASE int_const ':' {
		varr *jumptab;
		swjump_t *jump;

		if (mpc->curr_jumptab < 0) {
			compile_error(mpc,
			    "case label outside of switch");
			YYERROR;
		}

		jumptab = varr_get(&mpc->jumptabs, mpc->curr_jumptab);
		if (jumptab == NULL) {
			compile_error(mpc, "jumptab not found");
			YYERROR;
		}

		jump = (swjump_t *) varr_enew(jumptab);
		jump->val.i = $2;
		jump->addr = c_size(&mpc->code);
	}
	;

default: L_DEFAULT ':' {
		varr *jumptab;
		swjump_t *jump;

		if (mpc->curr_jumptab < 0) {
			compile_error(mpc,
			    "default: label outside of switch");
			YYERROR;
		}

		jumptab = varr_get(&mpc->jumptabs, mpc->curr_jumptab);
		if (jumptab == NULL) {
			compile_error(mpc, "jumptab not found");
			YYERROR;
		}

		jump = (swjump_t *) VARR_GET(jumptab, 0);
		jump->addr = c_size(&mpc->code);
	}
	;

break: L_BREAK ';' {
		if (mpc->curr_break_addr == INVALID_ADDR) {
			compile_error(mpc,
			    "break outside of switch or loop");
			YYERROR;
		}

		code2(mpc, c_jmp, (void *) mpc->curr_break_addr);
		mpc->curr_break_addr = c_size(&mpc->code) - 1;
	}
	| L_BREAK L_IDENT ';' {
		sym_t *sym;
		SYM_LOOKUP(sym, $2, SYM_LABEL);

		if (CODE(sym->s.label.addr)[0] != (int) c_if
		&&  CODE(sym->s.label.addr)[0] != (int) c_switch
		&&  CODE(sym->s.label.addr)[0] != (int) c_op_foreach) {
			compile_error(mpc,
			    "break outside of if or switch or loop");
			YYERROR;
		}

		code2(mpc, c_jmp_addr, (void *) (sym->s.label.addr + 1));
	}
	;

foreach: L_FOREACH {
		/* emit `foreach' */
		code(mpc, c_op_foreach);
		PUSH_ADDR();

		/* next addr, body */
		code2(mpc, (void *) INVALID_ADDR, (void *) INVALID_ADDR);
	 } '(' L_IDENT ',' L_ITER '(' expr_list ')' ')' {
		int i;
		sym_t *sym;
		iterdata_t *id;
		int addr;

		SYM_LOOKUP(sym, $4, SYM_VAR);

		/*
		 * check number of arguments
		 */
		if ($6->init.nargs != $8) {
			compile_error(mpc,
			    "%s: invalid number of arguments %d (%d expected)",
			    $6->init.name, $8, $6->init.nargs);
			YYERROR;
		}

		/*
		 * check argument types
		 */
		for (i = 0; i < $6->init.nargs; i++) {
			int got_type = argtype_get(mpc, $8, i);
			if (got_type != $6->init.argtype[i].type_tag) {
				compile_error(mpc,
				    "%s: invalid arg[%d] type '%s' (type '%s' (%d) expected)",
				    $6->init.name, i+1,
				    flag_string(mpc_types, got_type),
				    flag_string(mpc_types, $6->init.argtype[i].type_tag),
				    $6->init.argtype[i].type_tag);
				YYERROR;
			}
		}

		/* emit `foreach_next' */
		code(mpc, c_stop);

		PEEK_ADDR(addr);

		PUSH_EXPLICIT(mpc->curr_continue_addr);
		PUSH_EXPLICIT(mpc->curr_break_addr);
		PUSH_ADDR();

		mpc->curr_continue_addr = DELIMITER_ADDR;
		mpc->curr_break_addr = DELIMITER_ADDR;

		id = (iterdata_t *) varr_enew(&mpc->iterdata);
		id->iter = $6;
		code2(mpc, c_cleanup_syms, (void *) (mpc->curr_block + 1));
		code(mpc, c_foreach_next);
		code3(mpc, (void *) INVALID_ADDR, sym->name, id);

		/* body addr */
		CODE(addr)[1] = c_size(&mpc->code);
	} stmt {
		int addr;
		int body_addr;
		int next_continue_addr;

		/* emit `continue' */
		code(mpc, c_jmp);
		code(mpc, (void *) mpc->curr_continue_addr);
		mpc->curr_continue_addr = c_size(&mpc->code) - 1;

		POP_ADDR(body_addr);

		/*
		 * traverse linked list of continue addresses
		 * and emit address to jump
		 */
		for (; mpc->curr_continue_addr != DELIMITER_ADDR;
		     mpc->curr_continue_addr = next_continue_addr) {
			next_continue_addr = CODE(mpc->curr_continue_addr)[0];
			CODE(mpc->curr_continue_addr)[0] = body_addr;
		}

		POP_ADDR(mpc->curr_break_addr);
		POP_ADDR(mpc->curr_continue_addr);

		/* store next stmt addr */
		POP_ADDR(addr);
		CODE(addr)[0] = c_size(&mpc->code);
		CODE(body_addr)[3] = c_size(&mpc->code);

		code2(mpc, c_cleanup_syms, (void *) (mpc->curr_block + 1));
	}
	;

continue: L_CONTINUE ';' {
		if (mpc->curr_continue_addr == INVALID_ADDR) {
			compile_error(mpc, "continue outside of loop");
			YYERROR;
		}

		/* emit `continue' */
		code2(mpc, c_jmp, (void *) mpc->curr_continue_addr);
		mpc->curr_continue_addr = c_size(&mpc->code) - 1;
	}
	| L_CONTINUE L_IDENT ';' {
		sym_t *sym;
		SYM_LOOKUP(sym, $2, SYM_LABEL);

		if (CODE(sym->s.label.addr)[0] != (int) c_op_foreach) {
			compile_error(mpc,
			    "continue outside of loop");
			YYERROR;
		}

		code2(mpc, c_jmp, (void *) (sym->s.label.addr + 2));
	}
	;

return:	L_RETURN comma_expr ';' {
		if ($2 != MT_INT) {
			compile_error(mpc,
			    "integer expression expected");
			YYERROR;
		}

		code3(mpc, c_pop, (void *) $2, c_return);
	}
	;

int_const: L_INT
	| int_const '|' int_const	{ $$ = $1 | $3; }
	| int_const '^' int_const	{ $$ = $1 ^ $3; }
	| int_const '&' int_const	{ $$ = $1 & $3; }
	| int_const L_EQ int_const	{ $$ = $1 == $3; }
	| int_const L_NE int_const	{ $$ = $1 != $3; }
	| int_const '>' int_const	{ $$ = $1 > $3; }
	| int_const L_GE int_const	{ $$ = $1 >= $3; }
	| int_const '<' int_const	{ $$ = $1 < $3; }
	| int_const L_LE int_const	{ $$ = $1 <= $3; }
	| int_const L_SHL int_const	{ $$ = $1 << $3; }
	| int_const L_SHR int_const	{ $$ = $1 >> $3; }
	| int_const '+' int_const	{ $$ = $1 + $3; }
	| int_const '-' int_const	{ $$ = $1 - $3; }
	| int_const '*' int_const	{ $$ = $1 * $3; }
	| int_const '%' int_const {
		if ($3)
			$$ = $1 % $3;
		else
			compile_error(mpc, "modulo by zero");
	}
	| int_const '/' int_const {
		if ($3)
			$$ = $1 / $3;
		else
			compile_error(mpc, "division by zero");
	}
	| int_const L_LOR int_const	{ $$ = $1 || $3; }
	| int_const L_LAND int_const	{ $$ = $1 && $3; }
	| int_const '?' int_const ':' int_const %prec '?' {
		$$ = $1 ? $3 : $5;
	}
	| '(' int_const ')'		{ $$ = $2; }
	| '-' int_const	%prec L_NOT	{ $$ = -$2; }
	| '+' int_const %prec L_NOT	{ $$ = $2; }
	| L_NOT int_const		{ $$ = !$2; }
	| '~' int_const			{ $$ = ~$2; }
	;

expr:	L_IDENT assign expr %prec '=' {
		sym_t *sym;

		SYM_LOOKUP(sym, $1, SYM_VAR);
		if (sym->s.var.type_tag != $3) {
			compile_error(mpc, "type mismatch ('%s' vs. '%s')",
			    flag_string(mpc_types, sym->s.var.type_tag),
			    flag_string(mpc_types, $3));
			YYERROR;
		}

		code2(mpc, $2, sym->name);
		$$ = sym->s.var.type_tag;
	}
	| L_IDENT '(' expr_list ')' {
		int i;
		sym_t *sym;
		dynafun_data_t *d;

		SYM_LOOKUP(sym, $1, SYM_FUNC);
		if ((d = dynafun_data_lookup(sym->name)) == NULL) {
			compile_error(mpc, "%s: no such dynafun", sym->name);
			YYERROR;
		}

		/*
		 * check number of arguments
		 */
		if (d->nargs != $3) {
			compile_error(mpc,
			    "%s: invalid number of arguments %d (%d expected)",
			    $1, $3, d->nargs);
			YYERROR;
		}

		/*
		 * check argument types
		 */
		code2(mpc, c_push_retval, sym->name);
		code2(mpc, (void *) d->rv_tag, (void *) d->nargs);

		for (i = 0; i < d->nargs; i++) {
			int got_type;

			got_type = argtype_get(mpc, $3, i);

			if (d->argtype[i].type_tag == MT_PVOID
			||  d->argtype[i].type_tag == MT_PCVOID
			||  TYPE_IS(d->argtype[i].type_tag, got_type)) {
				code(mpc, (void *) d->argtype[i].type_tag);
				continue;
			}

			compile_error(mpc,
			    "%s: invalid arg[%d] type '%s' (type '%s' (%d) expected)",
			    $1, i+1,
			    flag_string(mpc_types, got_type),
			    flag_string(mpc_types, d->argtype[i].type_tag),
			    d->argtype[i].type_tag);
			YYERROR;
		}

		$$ = d->rv_tag;
		if (IS_INT_TYPE($$))
			$$ = MT_INT;

		/*
		 * pop argtypes from stack
		 */
		argtype_popn(mpc, $3);
	}
	| L_IDENT {
		sym_t *sym;

		SYM_LOOKUP(sym, $1, SYM_VAR);
		code2(mpc, c_push_var, sym->name);
		$$ = sym->s.var.type_tag;
	}
	| L_STRING {
		code2(mpc, c_push_const, $1);
		$$ = MT_STR;
	}
	| L_INT {
		code2(mpc, c_push_const, (void *) $1);
		$$ = MT_INT;
	}
	| expr '?' {
		code(mpc, c_quecolon);

		PUSH_ADDR();
		/* next_addr, else_addr */
		code2(mpc, (void *) INVALID_ADDR, (void *) INVALID_ADDR);
	} expr {
		int addr;

		code(mpc, c_stop);

		/* else_addr */
		PEEK_ADDR(addr);
		CODE(addr)[1] = c_size(&mpc->code);
	} ':' expr %prec '?' {
		int addr;

		POP_ADDR(addr);

		if ($1 != $7) {
			compile_error(mpc,
			    "different operand types for '?:' (%d and %d)",
			    $1, $7);
			YYERROR;
		}

		/* next_addr */
		CODE(addr)[0] = c_size(&mpc->code);
	}
	| expr L_LOR {
		INT_OP("||", $1, c_bop_lor);
		PUSH_ADDR();
		code(mpc, (void *) INVALID_ADDR);
	} expr {
		int addr;

		BOP_CHECK_TYPES("||", $1, $4);

		code(mpc, c_stop);

		POP_ADDR(addr);
		CODE(addr)[0] = c_size(&mpc->code);

		$$ = MT_INT;
	}
	| expr L_LAND {
		INT_OP("&&", $1, c_bop_land);
		PUSH_ADDR();
		code(mpc, (void *) INVALID_ADDR);
	} expr {
		int addr;

		BOP_CHECK_TYPES("&&", $1, $4);

		code(mpc, c_stop);

		POP_ADDR(addr);
		CODE(addr)[0] = c_size(&mpc->code);

		$$ = MT_INT;
	}
	| expr '|' expr {
		INT_BOP("|", c_bop_or, $1, $3, $$);
	}
	| expr '^' expr {
		INT_BOP("^", c_bop_xor, $1, $3, $$);
	}
	| expr '&' expr {
		INT_BOP("&", c_bop_and, $1, $3, $$);
	}
	| expr L_EQ expr {
		BOP_CHECK_TYPES("==", $1, $3);

		switch ($1) {
		case MT_INT:
		case MT_CHAR:
		case MT_OBJ:
		case MT_ROOM:
			code(mpc, c_bop_eq);
			break;

		case MT_STR:
			code(mpc, c_bop_eq_string);
			break;

		default:
			OP_INVALID_OPERAND("==", $1);
		}

		$$ = MT_INT;
	}
	| expr L_NE expr {
		BOP_CHECK_TYPES("!=", $1, $3);

		switch ($1) {
		case MT_INT:
		case MT_CHAR:
		case MT_OBJ:
		case MT_ROOM:
			code(mpc, c_bop_ne);
			break;

		case MT_STR:
			code(mpc, c_bop_ne_string);
			break;

		default:
			OP_INVALID_OPERAND("!=", $1);
		}

		$$ = MT_INT;
	}
	| expr '>' expr {
		INT_BOP(">", c_bop_gt, $1, $3, $$);
	}
	| expr L_GE expr {
		INT_BOP(">=", c_bop_ge, $1, $3, $$);
	}
	| expr '<' expr {
		INT_BOP("<", c_bop_lt, $1, $3, $$);
	}
	| expr L_LE expr {
		INT_BOP("<=", c_bop_le, $1, $3, $$);
	}
	| expr L_SHL expr {
		INT_BOP("<<", c_bop_shl, $1, $3, $$);
	}
	| expr L_SHR expr {
		INT_BOP(">>", c_bop_shr, $1, $3, $$);
	}
	| expr '+' expr {
		INT_BOP("+", c_bop_add, $1, $3, $$);
	}
	| expr '-' expr {
		INT_BOP("-", c_bop_sub, $1, $3, $$);
	}
	| expr '*' expr {
		INT_BOP("*", c_bop_mul, $1, $3, $$);
	}
	| expr '%' expr {
		INT_BOP("%", c_bop_mod, $1, $3, $$);
	}
	| expr '/' expr {
		INT_BOP("/", c_bop_div, $1, $3, $$);
	}
	| L_NOT expr {
		INT_UOP("!", c_uop_not, $2, $$);
	}
	| '~' expr {
		INT_UOP("~", c_uop_compl, $2, $$);
	}
	| '(' comma_expr ')'		{ $$ = $2; }
	| '-' expr %prec L_NOT {
		INT_UOP("-", c_uop_minus, $2, $$);
	}
	| L_IDENT L_INC			/* normal precedence here */ {
		INCDEC_UOP("++", c_postinc, $1, $$);
	}
	| L_IDENT L_DEC			/* normal precedence here */ {
		INCDEC_UOP("--", c_postdec, $1, $$);
	}
	| L_INC L_IDENT %prec L_NOT	/* note lower precedence here */ {
		INCDEC_UOP("++", c_preinc, $2, $$);
	}
	| L_DEC L_IDENT %prec L_NOT	/* note lower precedence here */ {
		INCDEC_UOP("--", c_predec, $2, $$);
	}
	| '+' expr %prec L_NOT		{ $$ = $2; }
	;

comma_expr: expr	{ $$ = $1; }
	| comma_expr	{ code2(mpc, c_pop, (void *) $1);
	} ',' expr	{ $$ = $4; }
	;

assign:	'='		{ $$ = c_assign; }
	| L_ADD_EQ	{ $$ = c_add_eq; }
	| L_SUB_EQ	{ $$ = c_sub_eq; }
	| L_DIV_EQ	{ $$ = c_div_eq; }
	| L_MUL_EQ	{ $$ = c_mul_eq; }
	| L_MOD_EQ	{ $$ = c_mod_eq; }
	| L_AND_EQ	{ $$ = c_and_eq; }
	| L_OR_EQ	{ $$ = c_or_eq; }
	| L_XOR_EQ	{ $$ = c_xor_eq; }
	| L_SHL_EQ	{ $$ = c_shl_eq; }
	| L_SHR_EQ	{ $$ = c_shr_eq; }
	;

expr_list:	/* empty */	{ $$ = 0; }
	| expr_list_ne		{ $$ = $1; }
	;

expr_list_ne: expr		{ argtype_push(mpc, $1); $$ = 1; }
	| expr_list_ne ',' expr	{ argtype_push(mpc, $3); $$ = $1 + 1; }
	;

%%

struct codeinfo_t {
	void *c_fun;
	const char *name;
	int gobble;
};
typedef struct codeinfo_t codeinfo_t;

struct codeinfo_t codetab[] = {
	{ c_pop,		"pop",			1 },
	{ c_push_const,		"push_const",		1 },
	{ c_push_var,		"push_var",		1 },
	{ c_push_retval,	"push_retval",		3 },
	{ c_stop,		"stop",			0 },
	{ c_jmp,		"jmp",			1 },
	{ c_jmp_addr,		"jmp_addr",		1 },
	{ c_if,			"if",			3 },
	{ c_switch,		"switch",		2 },
	{ c_quecolon,		"quecolon",		2 },
	{ c_op_foreach,		"foreach",		2 },
	{ c_foreach_next,	"foreach_next",		3 },
	{ c_declare,		"declare",		3 },
	{ c_declare_assign,	"declare_assign",	3 },
	{ c_cleanup_syms,	"cleanup_syms",		1 },
	{ c_return,		"return",		0 },
	{ c_bop_lor,		"bop_lor",		1 },
	{ c_bop_land,		"bop_land",		1 },
	{ c_bop_or,		"bop_or",		0 },
	{ c_bop_xor,		"bop_xor",		0 },
	{ c_bop_and,		"bop_and",		0 },
	{ c_bop_ne,		"bop_ne",		0 },
	{ c_bop_eq,		"bop_eq",		0 },
	{ c_bop_gt,		"bop_gt",		0 },
	{ c_bop_ge,		"bop_ge",		0 },
	{ c_bop_lt,		"bop_lt",		0 },
	{ c_bop_le,		"bop_le",		0 },
	{ c_bop_shl,		"bop_shl",		0 },
	{ c_bop_shr,		"bop_shr",		0 },
	{ c_bop_add,		"bop_add",		0 },
	{ c_bop_sub,		"bop_sub",		0 },
	{ c_bop_mul,		"bop_mul",		0 },
	{ c_bop_mod,		"bop_mod",		0 },
	{ c_bop_div,		"bop_div",		0 },
	{ c_bop_ne_string,	"bop_ne_string",	0 },
	{ c_bop_eq_string,	"bop_eq_string",	0 },
	{ c_uop_not,		"uop_not",		0 },
	{ c_uop_compl,		"uop_compl",		0 },
	{ c_uop_minus,		"uop_minus",		0 },
	{ c_postinc,		"postinc",		1 },
	{ c_postdec,		"postdec",		1 },
	{ c_preinc,		"preinc",		1 },
	{ c_predec,		"predec",		1 },
	{ c_assign,		"assign",		1 },
	{ c_add_eq,		"add_eq",		1 },
	{ c_sub_eq,		"sub_eq",		1 },
	{ c_div_eq,		"div_eq",		1 },
	{ c_mul_eq,		"mul_eq",		1 },
	{ c_mod_eq,		"mod_eq",		1 },
	{ c_and_eq,		"and_eq",		1 },
	{ c_or_eq,		"or_eq",		1 },
	{ c_xor_eq,		"xor_eq",		1 },
	{ c_shl_eq,		"shl_eq",		1 },
	{ c_shr_eq,		"shr_eq",		1 },
};

#define CODETAB_SZ (sizeof(codetab) / sizeof(codeinfo_t))

static codeinfo_t *
codeinfo_lookup(void *p)
{
	static bool codetab_initialized = FALSE;

	if (!codetab_initialized) {
		qsort(codetab, CODETAB_SZ, sizeof(codeinfo_t), cmpint);
		codetab_initialized = TRUE;
	}

	return (codeinfo_t *) bsearch(
	    &p, codetab, CODETAB_SZ, sizeof(codeinfo_t), cmpint);
}

int	mpc_parse(mpcode_t *mpc);

static int
mpc_error(mpcode_t *mpc, const char *errmsg)
{
	compile_error(mpc, "%s", errmsg);
	return 1;
}

avltree_t glob_syms;

void
sym_init(sym_t *sym)
{
	sym->name = NULL;
	sym->type = SYM_KEYWORD;
}

void
sym_destroy(sym_t *sym)
{
	free_string(sym->name);
}

sym_t *
sym_cpy(sym_t *dst, const sym_t *src)
{
	dst->name = str_qdup(src->name);
	dst->type = src->type;
	dst->s = src->s;
	return dst;
}

varrdata_t v_swjumps = {
	&varr_ops,

	sizeof(swjump_t), 4,

	NULL, NULL, NULL
};

static void
jumptab_init(varr *v)
{
	c_init(v, &v_swjumps);
}

varrdata_t v_ints = {
	&varr_ops,

	sizeof(int), 8,

	NULL, NULL, NULL
};

varrdata_t v_jumptabs = {
	&varr_ops,

	sizeof(varr), 4,

	(e_init_t) jumptab_init,
	(e_destroy_t) varr_destroy,
	(e_cpy_t ) varr_cpy
};

varrdata_t v_iterdata = {
	&varr_ops,

	sizeof(iterdata_t), 4,

	NULL, NULL, NULL
};

avltree_info_t avltree_info_strings = {
	&avltree_ops,

	MT_PVOID, sizeof(char *), ke_cmp_csstr,

	strkey_init,
	strkey_destroy,
	strkey_cpy
};

avltree_info_t avltree_info_syms = {
	&avltree_ops,

	MT_PVOID, sizeof(sym_t), ke_cmp_csstr,

	(e_init_t) sym_init,
	(e_destroy_t) sym_destroy,
	(e_cpy_t) sym_cpy
};

varrdata_t v_vos = {
	&varr_ops,

	sizeof(vo_t), 4,

	NULL, NULL, NULL
};

void
mpcode_init(mpcode_t *mpc)
{
	mpc->name = str_empty;
	mpc->mp = NULL;
	mpc->lineno = 0;

	c_init(&mpc->strings, &avltree_info_strings);
	c_init(&mpc->syms, &avltree_info_syms);

	c_init(&mpc->cstack, &v_ints);
	c_init(&mpc->args, &v_ints);
	mpc->curr_block = 0;

	mpc->curr_jumptab = -1;
	mpc->curr_break_addr = INVALID_ADDR;
	mpc->curr_continue_addr = INVALID_ADDR;

	mpc->ip = 0;
	c_init(&mpc->code, &v_ints);

	c_init(&mpc->jumptabs, &v_jumptabs);
	c_init(&mpc->iterdata, &v_iterdata);

	c_init(&mpc->data, &v_vos);
}

void
mpcode_destroy(mpcode_t *mpc)
{
	free_string(mpc->name);

	c_destroy(&mpc->strings);
	c_destroy(&mpc->syms);

	c_destroy(&mpc->cstack);
	c_destroy(&mpc->args);

	c_destroy(&mpc->code);

	c_destroy(&mpc->jumptabs);
	c_destroy(&mpc->iterdata);

	c_destroy(&mpc->data);
}

static
FOREACH_CB_FUN(print_swjump_cb, p, ap)
{
	swjump_t *sw = (swjump_t *) p;

	fprintf(stderr, "\n\t%d:\t0x%08x", sw->val.i, sw->addr);
	return NULL;
}

void
mpcode_dump(mpcode_t *mpc)
{
	size_t ip;

	for (ip = 0; ip < c_size(&mpc->code); ip++) {
		void *p = (void *) CODE(ip)[0];
		codeinfo_t *ci;

		if ((ci = codeinfo_lookup(p)) == NULL) {
			fprintf(stderr, "0x%08x: 0x%08x\n", ip, (int) p);
			continue;
		}

		fprintf(stderr, "0x%08x: %s", ip, ci->name);
		if (p == c_push_const) {
			fprintf(stderr, " %d", CODE(ip)[1]);
		} else if (p == c_push_var) {
			fprintf(stderr, " %s", (const char *) CODE(ip)[1]);
		} else if (p == c_push_retval) {
			fprintf(stderr, " %s", (const char *) CODE(ip)[1]);
			ip += CODE(ip)[3];
		} else if (p == c_jmp || p == c_bop_lor || p == c_bop_land) {
			fprintf(stderr, " 0x%08x", CODE(ip)[1]);
		} else if (p == c_jmp_addr) {
			int addr = CODE(ip)[1];
			fprintf(stderr, " (addr: 0x%08x, jmp_addr: 0x%08x)",
				addr, CODE(addr)[0]);
		} else if (p == c_switch) {
			int jt_offset = CODE(ip)[2];
			varr *jumptab;

			jumptab = varr_get(&mpc->jumptabs, jt_offset);
			if (jumptab != NULL)
				c_foreach(jumptab, print_swjump_cb);
		} else if (p == c_quecolon) {
			fprintf(stderr, " (next: 0x%08x else: 0x%08x)",
				CODE(ip)[1], CODE(ip)[2]);
		} else if (p == c_if) {
			fprintf(stderr,
				" (next: 0x%08x, then: 0x%08x, else: 0x%08x)",
				CODE(ip)[1], CODE(ip)[2], CODE(ip)[3]);
		} else if (p == c_op_foreach) {
			fprintf(stderr,
				" (next: 0x%08x, body: 0x%08x)",
				CODE(ip)[1], CODE(ip)[2]);
		} else if (p == c_foreach_next) {
			fprintf(stderr, " (next: 0x%08x, %s, iter: %s)",
				CODE(ip)[1],
				(const char *) CODE(ip)[2],
				((iterdata_t *) CODE(ip)[3])->iter->init.name);
		} else if (p == c_declare || p == c_declare_assign) {
			fprintf(stderr, " (%d %s, block: %d)",
				CODE(ip)[2],
				(const char *) CODE(ip)[1],
				CODE(ip)[3]);
		} else if (p == c_cleanup_syms) {
			fprintf(stderr, " (block: %d)", CODE(ip)[1]);
		}

		ip += ci->gobble;
		fprintf(stderr, "\n");
	}
}

void
execute(mpcode_t *mpc, int ip)
{
	c_fun *c;

	mpc->ip = ip;
	while ((c = (c_fun *) varr_get(&mpc->code, mpc->ip++)) != NULL) {
		if (*c == c_stop)
			break;
		(*c)(mpc);
	}
}

sym_t *
sym_lookup(mpcode_t *mpc, const char *name)
{
	sym_t *sym;

	if ((sym = (sym_t *) c_lookup(&glob_syms, name)) != NULL)
		return sym;

	return (sym_t *) c_lookup(&mpc->syms, name);
}

static void *
find_block_cb(void *p, va_list ap)
{
	sym_t *sym = (sym_t *) p;
	int block = va_arg(ap, int);

	if (sym->type == SYM_VAR && sym->s.var.block >= block)
		return sym;

	return NULL;
}

void
cleanup_syms(mpcode_t *mpc, int block)
{
	for (; ;) {
		sym_t *sym;

		sym = (sym_t *) c_foreach(&mpc->syms, find_block_cb, block);
		if (sym == NULL)
			break;

		if (IS_SET(mpc->mp->flags, MP_F_TRACE)) {
			log(LOG_INFO, "%s: %s (%d)",
			    __FUNCTION__, sym->name, sym->s.var.block);
		}
		c_delete(&mpc->syms, sym->name);
	}
}

void
compile_error(mpcode_t *mpc, const char *fmt, ...)
{
	char buf[MAX_STRING_LENGTH];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	buf_printf(mpc->mp->errbuf, BUF_END, "%s:%d: %s\n",
		   mpc->name, mpc->lineno, buf);
}

const char *
alloc_string(mpcode_t *mpc, const char *s)
{
	const char **p;

	if (IS_NULLSTR(s))
		return str_empty;

	if ((p = c_lookup(&mpc->strings, s)) == NULL) {
		p = c_insert(&mpc->strings, s);
		*p = str_dup(s);
	}

	return *p;
}

static int
var_add(mpcode_t *mpc, const char *name, int type_tag)
{
	sym_t *s;

	if ((s = (sym_t *) c_insert(&mpc->syms, name)) == NULL) {
		compile_error(mpc, "%s: duplicate symbol", name);
		return -1;
	}

	s->name = str_dup(name);
	s->type = SYM_VAR;
	s->s.var.type_tag = type_tag;
	s->s.var.is_const = FALSE;
	s->s.var.block = -1;
	s->s.var.data.i = 0;

	return 0;
}

static sym_t *
var_lookup(mpcode_t *mpc, const char *name, int type_tag)
{
	sym_t *sym;

	if ((sym = sym_lookup(mpc, name)) == NULL) {
		fprintf(stderr, "Runtime error: %s: %s: symbol not found\n",
			mpc->name, name);
		return NULL;
	}

	if (sym->type != SYM_VAR) {
		fprintf(stderr, "%s: not a %d", name, SYM_VAR);
		return NULL;
	}

	if (sym->s.var.type_tag != type_tag) {
		fprintf(stderr, "%s: type mismatch (got %d, expected %d)\n",
			name, sym->s.var.type_tag, type_tag);
		return NULL;
	}

	return sym;
}

static int
var_assign(mpcode_t *mpc, const char *name, int type_tag, void *vo)
{
	sym_t *sym;

	if ((sym = var_lookup(mpc, name, type_tag)) == NULL)
		return -1;

	if (type_tag != MT_STR
	&&  vo != NULL
	&&  !mem_is(vo, type_tag)) {
		log(LOG_BUG, "%s: vo is not of type '%s'",
		    __FUNCTION__, flag_string(mpc_types, type_tag));
	}

	if (IS_SET(mpc->mp->flags, MP_F_TRACE)) {
		log(LOG_INFO, "%s: %s: %p (type '%s')",
		    __FUNCTION__, name, vo,
		    flag_string(mpc_types, type_tag));
	}

	sym->s.var.data.p = vo;
	return 0;
}

int
_mprog_compile(mprog_t *mp)
{
	mpcode_t *mpc;

	mp->status = MP_S_DIRTY;
	if (mp->errbuf == NULL)
		mp->errbuf = buf_new(-1);
	else
		buf_clear(mp->errbuf);

	if ((mpc = mpcode_lookup(mp->name)) == NULL) {
		if ((mpc = (mpcode_t *) c_insert(&mpcodes, mp->name)) == NULL) {
			log(LOG_ERROR, "compile_mprog: %s: mpcode already exists",
			    mp->name);
			return MPC_ERR_INTERNAL;
		}

		mpc->name = str_qdup(mp->name);
	}

	mpc->mp = mp;
	mpc->cp = mpc->mp->text;

	mpc->lineno = 1;

	c_erase(&mpc->strings);
	c_erase(&mpc->syms);

	c_erase(&mpc->cstack);
	c_erase(&mpc->args);
	mpc->curr_block = 0;

	c_erase(&mpc->code);

	c_erase(&mpc->jumptabs);
	c_erase(&mpc->iterdata);

	mpc->curr_jumptab = -1;
	mpc->curr_break_addr = INVALID_ADDR;
	mpc->curr_continue_addr = INVALID_ADDR;

	if (var_add(mpc, "$_", MT_INT) < 0)
		return MPC_ERR_COMPILE;

	switch (mpc->mp->type) {
	case MP_T_MOB:
		if (var_add(mpc, "$n", MT_CHAR) < 0)
			return MPC_ERR_COMPILE;
		if (var_add(mpc, "$N", MT_CHAR) < 0)
			return MPC_ERR_COMPILE;
		if (var_add(mpc, "$p", MT_OBJ) < 0)
			return MPC_ERR_COMPILE;
		if (var_add(mpc, "$t", MT_STR) < 0)
			return MPC_ERR_COMPILE;
		break;

	case MP_T_OBJ:
		if (var_add(mpc, "$p", MT_OBJ) < 0)
			return MPC_ERR_COMPILE;
		if (var_add(mpc, "$n", MT_CHAR) < 0)
			return MPC_ERR_COMPILE;
		break;

	case MP_T_ROOM:
		break;

	case MP_T_SPEC:
		if (var_add(mpc, "$n", MT_CHAR) < 0)
			return MPC_ERR_COMPILE;
		if (var_add(mpc, "$rm", MT_STR) < 0)
			return MPC_ERR_COMPILE;
		if (var_add(mpc, "$add", MT_STR) < 0)
			return MPC_ERR_COMPILE;
		break;
	}

	if (mpc_parse(mpc) < 0
	||  !IS_NULLSTR(buf_string(mpc->mp->errbuf)))
		return MPC_ERR_COMPILE;

	if (IS_SET(mpc->mp->flags, MP_F_TRACE))
		mpcode_dump(mpc);

	buf_free(mpc->mp->errbuf);
	mpc->mp->errbuf = NULL;

	cleanup_syms(mpc, 0);
	mpc->mp->status = MP_S_READY;
	mpc->cp = str_empty;
	mpc->mp = NULL;

	return 0;
}

#define execerr(errcode)				\
	do {						\
		rv = errcode;				\
		goto err;				\
	} while (0)

int
_mprog_execute(mprog_t *mp, void *arg1, void *arg2, void *arg3, void *arg4)
{
	sym_t *sym;
	mpcode_t *mpc;
	int rv;

	if ((mpc = mpcode_lookup(mp->name)) == NULL) {
		fprintf(stderr, "Runtime error: %s: mpcode not found\n",
			mp->name);
		return MPC_ERR_RUNTIME;
	}

	mpc->mp = mp;
	if (mpc->mp->status != MP_S_READY)
		execerr(MPC_ERR_DIRTY);

	switch (mpc->mp->type) {
	case MP_T_MOB:
		if (var_assign(mpc, "$n", MT_CHAR, arg1) < 0)
			execerr(MPC_ERR_RUNTIME);
		if (var_assign(mpc, "$N", MT_CHAR, arg2) < 0)
			execerr(MPC_ERR_RUNTIME);
		if (var_assign(mpc, "$p", MT_OBJ, arg3) < 0)
			execerr(MPC_ERR_RUNTIME);
		if (var_assign(mpc, "$t", MT_STR, arg4) < 0)
			execerr(MPC_ERR_RUNTIME);
		break;

	case MP_T_OBJ:
		if (var_assign(mpc, "$p", MT_OBJ, arg1) < 0)
			execerr(MPC_ERR_RUNTIME);
		if (var_assign(mpc, "$n", MT_CHAR, arg2) < 0)
			execerr(MPC_ERR_RUNTIME);
		break;

	case MP_T_ROOM:
		break;

	case MP_T_SPEC:
		if (var_assign(mpc, "$n", MT_CHAR, arg1) < 0)
			execerr(MPC_ERR_RUNTIME);

		if (var_assign(mpc, "$rm", MT_STR, arg2) < 0)
			execerr(MPC_ERR_RUNTIME);

		if (var_assign(mpc, "$add", MT_STR, arg3) < 0)
			execerr(MPC_ERR_RUNTIME);

		break;
	}

	if ((rv = setjmp(mpc->jmpbuf)) == 0)
		execute(mpc, 0);
	else if (rv < 0)
		goto err;

	if ((sym = (sym_t *) c_lookup(&mpc->syms, "$_")) == NULL) {
		fprintf(stderr, "Runtime error: %s: %s: symbol not found\n",
			__FUNCTION__, "$_");
		execerr(MPC_ERR_RUNTIME);
	}

	if (sym->type != SYM_VAR) {
		fprintf(stderr, "Runtime error: %s: not a %d (got symtype %d)\n",
			sym->name, SYM_VAR, sym->type);
		execerr(MPC_ERR_RUNTIME);
	}

	if (sym->s.var.data.i < 0) {
		fprintf(stderr, "Error: %s: %s: return value < 0",
			__FUNCTION__, mpc->name);
		execerr(MPC_ERR_RUNTIME);
	}

	rv = sym->s.var.data.i;

err:
	cleanup_syms(mpc, 0);
	mpc->mp = NULL;
	return rv;
}

#if defined(MPC)
static void
usage()
{
	fprintf(stderr, "Usage: mpc <filename>\n");
	exit(1);
}

int
main(int argc, char *argv[])
{
	prog_t prog;
	int exitcode;
	FILE *fp;
	struct stat s;
	int rv;

	if (argc != 2)
		usage();

	/*
	 * initialize mpc
	 */
	if (mpc_init() < 0) {
		fprintf(stderr, "mpc_init() failed\n");
		exit(2);
	}

	prog_init(&prog);
	prog.name = argv[1];
	if (stat(prog.name, &s) < 0) {
		fprintf(stderr, "stat: %s: %s\n", prog.name, strerror(errno));
		exit(2);
	}

	fp = fopen(prog.name, "r");
	if (fp == NULL) {
		fprintf(stderr, "fopen: %s: %s\n", prog.name, strerror(errno));
		exit(2);
	}

	prog.textlen = s.st_size;
	prog.text = malloc(s.st_size + 1);
	if (prog.text == NULL) {
		fprintf(stderr, "malloc: %s\n", strerror(errno));
		exit(2);
	}

	fread((void *) prog.text, prog.textlen, 1, fp);
	((char *) prog.text)[prog.textlen] = '\0';

	fclose(fp);

	yydebug = 1;
	if (prog_compile(&prog) < 0) {
		fprintf(stderr, "%s", buf_string(prog.errbuf));
		exitcode = 3;
	} else {
		int errcode;
		prog_dump(&prog);

		exitcode = 0;

		rv = prog_execute(&prog, &errcode);
		fprintf(stderr, "prog_execute: rv = %d (errcode %d)\n",
			rv, errcode);
	}

	prog_destroy(&prog);

	return exitcode;
}
#endif
