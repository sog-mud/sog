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
 * $Id: mpc.y,v 1.19 2001-07-31 18:14:57 fjoe Exp $
 */

/*
 * Inspired by
 * - `hoc' from The Unix Programming Environment, by Brian Kernighan
 * and Rob Pike (Addison-Wesley, 1984).
 * - MudOS 0.9.19 copyright 1991-1992 by Erik Kay, Adam Beeman,
 * Stephan Iannce and John Garnett, LPmud copyright by Lars Pensj|, 1990, 1991.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>

#if defined(MPC)
#include <dlfcn.h>
#endif

#include <typedef.h>
#include <varr.h>
#include <hash.h>
#include <dynafun.h>
#include <memalloc.h>
#include <buffer.h>
#include <strkey_hash.h>
#include <log.h>
#include <util.h>

#if defined(MPC)
#include <module.h>
#endif

#include "mpc_impl.h"
#include "mpc_const.h"
#include "mpc_iter.h"

#define YYPARSE_PARAM prog
#define YYPARSE_PARAM_TYPE prog_t *

#if 0
#if defined(MPC)
#define YYDEBUG 1
#endif
#endif

#undef yylex
#define yylex() mpc_lex(prog)

#undef yyerror
#define yyerror(a) mpc_error(prog, a)

#define yysslim mpc_sslim
#define yystacksize mpc_stacksize

/*--------------------------------------------------------------------
 * argtype stack manipulation functions
 */

/**
 * Push argument type on argument type stack
 */
static void
argtype_push(prog_t *prog, int type_tag)
{
	int *t = varr_enew(&prog->args);
	*t = type_tag;
}

/**
 * Pop n arguments from argument type stack
 */
static void
argtype_popn(prog_t *prog, size_t n)
{
	assert(varr_size(&prog->args) >= n);
	varr_size(&prog->args) -= n;
}

/**
 * Get argument type from argument type stack
 */
static int
argtype_get(prog_t *prog, int n, int index)
{
	int *t = varr_get(&prog->args, varr_size(&prog->args) - n + index);
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
code(prog_t *prog, const void *opcode)
{
	int old_ip = varr_size(&prog->code);
	const void **o = varr_enew(&prog->code);
	*o = opcode;
	return old_ip;
}

/**
 * Append two opcodes to program (compiled) text
 */
static inline void
code2(prog_t *prog, const void *opcode1, const void *opcode2)
{
	code(prog, opcode1);
	code(prog, opcode2);
}

/**
 * Append three opcodes to program (compiled) text
 */
static inline void
code3(prog_t *prog,
      const void *opcode1, const void *opcode2, const void *opcode3)
{
	code(prog, opcode1);
	code(prog, opcode2);
	code(prog, opcode3);
}

#define SYM_LOOKUP(sym, ident, symtype)					\
	do {								\
		/*							\
		 * lookup symbol					\
		 */							\
		if ((sym = sym_lookup(prog, (ident))) == 0) {		\
			compile_error(prog,				\
			    "%s: unknown identifier", (ident));		\
			YYERROR;					\
		}							\
									\
		if (sym->type != (symtype)) {				\
			compile_error(prog,				\
			    "%s: not a %d", (ident), (symtype));	\
			YYERROR;					\
		}							\
	} while (0)

#define OP_INVALID_OPERAND(opname, type_tag)				\
	do {								\
		compile_error(prog,					\
		    "invalid operand for '%s' (%d)",			\
		    (opname), (type_tag));				\
		YYERROR;						\
	} while (0)

#define INT_OP(opname, type_tag, c_fun)					\
	do {								\
		switch (type_tag) {					\
		case MT_INT:						\
			code(prog, c_fun);				\
			break;						\
									\
		default:						\
			OP_INVALID_OPERAND((opname), (type_tag));	\
			/* NOTREACHED */				\
		}							\
	} while (0)

#define BOP_CHECK_TYPES(opname, type_tag1, type_tag2)			\
	do {								\
		if ((type_tag1) != (type_tag2)) {			\
			compile_error(prog,				\
			    "invalid operand types for '%s' (%d vs. %d)",\
			    (opname), (type_tag1), (type_tag2));	\
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
		code(prog, sym->name);					\
	} while (0)

#define PUSH_EXPLICIT(addr)						\
	do {								\
		int *p = (int *) varr_enew(&prog->cstack);		\
		*p = addr;						\
	} while (0)

#define PUSH_ADDR()	PUSH_EXPLICIT(varr_size(&prog->code))

#define PEEK_ADDR(addr)						\
	do {								\
		int *p = (int *) varr_get(				\
		    &prog->cstack, varr_size(&prog->cstack) - 1);	\
		if (p == NULL) {					\
			compile_error(prog, "compiler stack underflow");\
			YYERROR;					\
		}							\
		addr = *p;						\
	} while (0)

#define POP_ADDR(addr)						\
	do {								\
		PEEK_ADDR(addr);					\
		varr_size(&prog->cstack)--;				\
	} while (0)

#define CODE(addr)	((int *) VARR_GET(&prog->code, (addr)))

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
		const void *p;
		sym_t sym;

		sym.name = str_dup($2);
		sym.type = SYM_VAR;
		sym.s.var.type_tag = $1;
		sym.s.var.is_const = FALSE;
		sym.s.var.block = prog->curr_block;

		switch ($1) {
		case MT_STR:
			sym.s.var.data.s = NULL;
			break;

		case MT_INT:
		default:
			sym.s.var.data.i = 0;
			break;
		};

		if ((p = hash_insert(&prog->syms, sym.name, &sym)) == NULL) {
			sym_destroy(&sym);
			compile_error(prog, "%s: duplicate symbol", sym.name);
			YYERROR;
		}

		code3(prog, c_declare, sym.name, (void *) sym.s.var.type_tag);
		code(prog, (void *) sym.s.var.block);

		sym_destroy(&sym);
	}
	| L_TYPE L_IDENT '=' comma_expr ';' {
		const void *p;
		sym_t sym;

		if ($1 != $4) {
			compile_error(prog, "type mismatch (%d vs. %d)",
				      $1, $4);
			YYERROR;
		}

		sym.name = str_dup($2);
		sym.type = SYM_VAR;
		sym.s.var.type_tag = $1;
		sym.s.var.is_const = FALSE;
		sym.s.var.block = prog->curr_block;

		if ((p = hash_insert(&prog->syms, sym.name, &sym)) == NULL) {
			sym_destroy(&sym);
			compile_error(prog, "%s: duplicate symbol", sym.name);
			YYERROR;
		}

		code3(prog,
		    c_declare_assign, sym.name, (void *) sym.s.var.type_tag);
		code(prog, (void *) sym.s.var.block);

		sym_destroy(&sym);
	}
	| comma_expr ';' {
		/*
		 * pop calculated value from stack
		 */
		code2(prog, c_pop, (void *) $1);
	}
	| label
	| if | switch | case_label | default | break | foreach | continue
	| return
	| '{' {
		prog->curr_block++;
	} stmt_list '}' {
		code2(prog, c_cleanup_syms, (void *) prog->curr_block);
		cleanup_syms(prog, prog->curr_block--);
	}
	;

label:	L_IDENT ':' {
		sym_t sym;
		void *p;

		sym.name = str_dup($1);
		sym.type = SYM_LABEL;
		sym.s.label.addr = varr_size(&prog->code);

		if ((p = hash_insert(&prog->syms, sym.name, &sym)) == NULL) {
			sym_destroy(&sym);
			compile_error(prog, "%s: duplicate symbol", sym.name);
			YYERROR;
		}
		sym_destroy(&sym);
	}
	;

if:	L_IF {
		code(prog, c_if);
		PUSH_ADDR();
		/* next stmt, then, else */
		code(prog, (void *) INVALID_ADDR);
		code2(prog, (void *) INVALID_ADDR, (void *) INVALID_ADDR);
	} '(' comma_expr ')' {
		int addr;

		code(prog, c_stop);

		/* store then addr */
		PEEK_ADDR(addr);
		CODE(addr)[1] = varr_size(&prog->code);
	} stmt optional_else {
		int addr;

		/* store next stmt addr */
		POP_ADDR(addr);
		CODE(addr)[0] = varr_size(&prog->code);

		code2(prog, c_cleanup_syms, (void *) prog->curr_block + 1);
	}
	;

optional_else: /* empty */
	| L_ELSE {
		int addr;

		PEEK_ADDR(addr);

		/* jmp to next stmt */
		code2(prog, c_jmp_addr, (void *) addr);

		/* store else addr */
		CODE(addr)[2] = varr_size(&prog->code);
	} stmt
	;

switch:	L_SWITCH {
		varr *jumptab;
		swjump_t *jump;

		code(prog, c_switch);

		PUSH_ADDR();
		PUSH_EXPLICIT(prog->curr_break_addr);
		PUSH_EXPLICIT(prog->curr_jumptab);

		/* jumptab[0] reserved for 'default:' */
		jumptab = (varr *) varr_get(
		    &prog->jumptabs, ++prog->curr_jumptab);
		if (jumptab == NULL)
			jumptab = varr_enew(&prog->jumptabs);
		else
			varr_erase(jumptab);
		jump = varr_enew(jumptab);
		jump->addr = INVALID_ADDR;

		/* next_addr, jt_offset */
		code(prog, (void *) DELIMITER_ADDR);
		prog->curr_break_addr = varr_size(&prog->code) - 1;
		code(prog, (void *) prog->curr_jumptab);
	} '(' expr ')' {
		code(prog, c_stop);
	} stmt {
		int addr;
		int next_break_addr;
		varr *jumptab;

		/*
		 * emit `break'
		 * (if last case label/default does not have break)
		 */
		code(prog, c_jmp);
		code(prog, (void *) prog->curr_break_addr);
		prog->curr_break_addr = varr_size(&prog->code) - 1;

		/*
		 * traverse linked list of break addresses
		 * and emit address to jump
		 */
		for (; prog->curr_break_addr != DELIMITER_ADDR;
		     prog->curr_break_addr = next_break_addr) {
			next_break_addr = CODE(prog->curr_break_addr)[0];
			CODE(prog->curr_break_addr)[0] = varr_size(&prog->code);
		}

		/* qsort jumptab */
		jumptab = varr_get(&prog->jumptabs, prog->curr_jumptab);
		if (jumptab == NULL) {
			compile_error(prog, "invalid jumptab");
			YYERROR;
		}

		if (jumptab->nused < 2) {
			compile_error(prog,
			     "switch without case labels");
			YYERROR;
		}

		qsort(((char *) jumptab->p) + jumptab->v_data->nsize,
		    jumptab->nused - 1, jumptab->v_data->nsize, cmpint);

		POP_ADDR(prog->curr_jumptab);
		POP_ADDR(prog->curr_break_addr);
		POP_ADDR(addr);

		/* store next addr */
		CODE(addr)[0] = varr_size(&prog->code);
		code2(prog, c_cleanup_syms, (void *) prog->curr_block + 1);
	}
	;

case_label: L_CASE int_const ':' {
		varr *jumptab;
		swjump_t *jump;

		if (prog->curr_jumptab < 0) {
			compile_error(prog,
			    "case label outside of switch");
			YYERROR;
		}

		jumptab = varr_get(&prog->jumptabs, prog->curr_jumptab);
		if (jumptab == NULL) {
			compile_error(prog, "jumptab not found");
			YYERROR;
		}

		jump = (swjump_t *) varr_enew(jumptab);
		jump->val.i = $2;
		jump->addr = varr_size(&prog->code);
	}
	;

default: L_DEFAULT ':' {
		varr *jumptab;
		swjump_t *jump;

		if (prog->curr_jumptab < 0) {
			compile_error(prog,
			    "default: label outside of switch");
			YYERROR;
		}

		jumptab = varr_get(&prog->jumptabs, prog->curr_jumptab);
		if (jumptab == NULL) {
			compile_error(prog, "jumptab not found");
			YYERROR;
		}

		jump = (swjump_t *) VARR_GET(jumptab, 0);
		jump->addr = varr_size(&prog->code);
	}
	;

break: L_BREAK ';' {
		if (prog->curr_break_addr == INVALID_ADDR) {
			compile_error(prog,
			    "break outside of switch or loop");
			YYERROR;
		}

		code2(prog, c_jmp, (void *) prog->curr_break_addr);
		prog->curr_break_addr = varr_size(&prog->code) - 1;
	}
	| L_BREAK L_IDENT ';' {
		sym_t *sym;
		SYM_LOOKUP(sym, $2, SYM_LABEL);

		if (CODE(sym->s.label.addr)[0] != (int) c_if
		&&  CODE(sym->s.label.addr)[0] != (int) c_switch
		&&  CODE(sym->s.label.addr)[0] != (int) c_foreach) {
			compile_error(prog,
			    "break outside of if or switch or loop");
			YYERROR;
		}

		code2(prog, c_jmp_addr, (void *) (sym->s.label.addr + 1));
	}
	;

foreach: L_FOREACH {
		/* emit `foreach' */
		code(prog, c_foreach);
		PUSH_ADDR();

		/* next addr, body */
		code2(prog, (void *) INVALID_ADDR, (void *) INVALID_ADDR);
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
			compile_error(prog,
			    "%s: invalid number of arguments %d (%d expected)",
			    $6->init.name, $8, $6->init.nargs);
			YYERROR;
		}

		/*
		 * check argument types
		 */
		for (i = 0; i < $6->init.nargs; i++) {
			int got_type = argtype_get(prog, $8, i);
			if (got_type != $6->init.argtype[i].type_tag) {
				compile_error(prog,
				    "%s: invalid arg %d type %d (%d expected)",
				    $6->init.name, i+1, got_type,
				    $6->init.argtype[i].type_tag);
				YYERROR;
			}
		}

		/* emit `foreach_next' */
		code(prog, c_stop);

		PEEK_ADDR(addr);

		PUSH_EXPLICIT(prog->curr_continue_addr);
		PUSH_EXPLICIT(prog->curr_break_addr);
		PUSH_ADDR();

		prog->curr_continue_addr = DELIMITER_ADDR;
		prog->curr_break_addr = DELIMITER_ADDR;

		id = (iterdata_t *) varr_enew(&prog->iterdata);
		id->iter = $6;
		code2(prog, c_cleanup_syms, (void *) prog->curr_block + 1);
		code(prog, c_foreach_next);
		code3(prog, (void *) INVALID_ADDR, sym->name, id);

		/* body addr */
		CODE(addr)[1] = varr_size(&prog->code);
	} stmt {
		int addr;
		int body_addr;
		int next_continue_addr;

		/* emit `continue' */
		code(prog, c_jmp);
		code(prog, (void *) prog->curr_continue_addr);
		prog->curr_continue_addr = varr_size(&prog->code) - 1;

		POP_ADDR(body_addr);

		/*
		 * traverse linked list of continue addresses
		 * and emit address to jump
		 */
		for (; prog->curr_continue_addr != DELIMITER_ADDR;
		     prog->curr_continue_addr = next_continue_addr) {
			next_continue_addr = CODE(prog->curr_continue_addr)[0];
			CODE(prog->curr_continue_addr)[0] = body_addr;
		}

		POP_ADDR(prog->curr_break_addr);
		POP_ADDR(prog->curr_continue_addr);

		/* store next stmt addr */
		POP_ADDR(addr);
		CODE(addr)[0] = varr_size(&prog->code);
		CODE(body_addr)[3] = varr_size(&prog->code);

		code2(prog, c_cleanup_syms, (void *) prog->curr_block + 1);
	}
	;

continue: L_CONTINUE ';' {
		if (prog->curr_continue_addr == INVALID_ADDR) {
			compile_error(prog, "continue outside of loop");
			YYERROR;
		}

		/* emit `continue' */
		code2(prog, c_jmp, (void *) prog->curr_continue_addr);
		prog->curr_continue_addr = varr_size(&prog->code) - 1;
	}
	| L_CONTINUE L_IDENT ';' {
		sym_t *sym;
		SYM_LOOKUP(sym, $2, SYM_LABEL);

		if (CODE(sym->s.label.addr)[0] != (int) c_foreach) {
			compile_error(prog,
			    "continue outside of loop");
			YYERROR;
		}

		code2(prog, c_jmp, (void *) (sym->s.label.addr + 2));
	}
	;

return:	L_RETURN comma_expr ';' {
		if ($2 != MT_INT) {
			compile_error(prog,
			    "integer expression expected");
			YYERROR;
		}

		code3(prog, c_pop, (void *) $2, c_return);
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
			compile_error(prog, "modulo by zero");
	}
	| int_const '/' int_const {
		if ($3)
			$$ = $1 / $3;
		else
			compile_error(prog, "division by zero");
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
			compile_error(prog, "type mismatch (%d vs. %d)",
			    sym->s.var.type_tag, $3);
			YYERROR;
		}

		code2(prog, $2, sym->name);
		$$ = sym->s.var.type_tag;
	}
	| L_IDENT '(' expr_list ')' {
		int i;
		sym_t *sym;
		dynafun_data_t *d;

		SYM_LOOKUP(sym, $1, SYM_FUNC);
		if ((d = dynafun_data_lookup(sym->name)) == NULL) {
			compile_error(prog, "%s: no such dynafun", sym->name);
			YYERROR;
		}

		/*
		 * check number of arguments
		 */
		if (d->nargs != $3) {
			compile_error(prog,
			    "%s: invalid number of arguments %d (%d expected)",
			    $1, $3, d->nargs);
			YYERROR;
		}

		/*
		 * check argument types
		 */
		code2(prog, c_push_retval, sym->name);
		code2(prog, (void *) d->rv_tag, (void *) d->nargs);

		for (i = 0; i < d->nargs; i++) {
			int got_type = argtype_get(prog, $3, i);
			if (got_type != d->argtype[i].type_tag) {
				compile_error(prog,
				    "%s: invalid arg %d type %d (%d expected)",
				    $1, i+1, got_type, d->argtype[i].type_tag);
				YYERROR;
			}
			code(prog, (void *) d->argtype[i].type_tag);
		}

		$$ = d->rv_tag;

		/*
		 * pop argtypes from stack
		 */
		argtype_popn(prog, $3);
	}
	| L_IDENT {
		sym_t *sym;

		SYM_LOOKUP(sym, $1, SYM_VAR);
		code2(prog, c_push_var, sym->name);
		$$ = sym->s.var.type_tag;
	}
	| L_STRING {
		code2(prog, c_push_const, $1);
		$$ = MT_STR;
	}
	| L_INT {
		code2(prog, c_push_const, (void *) $1);
		$$ = MT_INT;
	}
	| expr '?' {
		code(prog, c_quecolon);

		PUSH_ADDR();
		/* next_addr, else_addr */
		code2(prog, (void *) INVALID_ADDR, (void *) INVALID_ADDR);
	} expr {
		int addr;

		code(prog, c_stop);

		/* else_addr */
		PEEK_ADDR(addr);
		CODE(addr)[1] = varr_size(&prog->code);
	} ':' expr %prec '?' {
		int addr;

		POP_ADDR(addr);

		if ($1 != $7) {
			compile_error(prog,
			    "different operand types for '?:' (%d and %d)",
			    $1, $7);
			YYERROR;
		}

		/* next_addr */
		CODE(addr)[0] = varr_size(&prog->code);
	}
	| expr L_LOR {
		INT_OP("||", $1, c_bop_lor);
		PUSH_ADDR();
		code(prog, (void *) INVALID_ADDR);
	} expr {
		int addr;

		POP_ADDR(addr);
		CODE(addr)[0] = varr_size(&prog->code);

		BOP_CHECK_TYPES("||", $1, $4);

		code(prog, c_stop);
		$$ = MT_INT;
	}
	| expr L_LAND {
		INT_OP("&&", $1, c_bop_land);
		PUSH_ADDR();
		code(prog, (void *) INVALID_ADDR);
	} expr {
		int addr;

		POP_ADDR(addr);
		CODE(addr)[0] = varr_size(&prog->code);

		BOP_CHECK_TYPES("&&", $1, $4);

		code(prog, c_stop);
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
			code(prog, c_bop_eq);
			break;

		case MT_STR:
			code(prog, c_bop_eq_string);
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
			code(prog, c_bop_ne);
			break;

		case MT_STR:
			code(prog, c_bop_ne_string);
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
	| comma_expr	{ code2(prog, c_pop, (void *) $1);
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

expr_list_ne: expr		{ argtype_push(prog, $1); $$ = 1; }
	| expr_list_ne ',' expr	{ argtype_push(prog, $3); $$ = $1 + 1; }
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
	{ c_foreach,		"foreach",		2 },
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

codeinfo_t *
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

int	mpc_parse(prog_t *prog);

int
mpc_error(prog_t *prog, const char *errmsg)
{
	compile_error(prog, "%s", errmsg);
	return 1;
}

hash_t glob_syms;

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
	sizeof(swjump_t), 4
};

static void
jumptab_init(varr *v)
{
	varr_init(v, &v_swjumps);
}

varrdata_t v_ints = {
	sizeof(int), 8
};

varrdata_t v_jumptabs = {
	sizeof(varr), 4,

	(e_init_t) jumptab_init,
	(e_destroy_t) varr_destroy,
	(e_cpy_t ) varr_cpy
};

varrdata_t v_iterdata = {
	sizeof(iterdata_t), 4
};

hashdata_t h_strings = {
	sizeof(char *), 4,

	strkey_init,
	strkey_destroy,
	strkey_cpy,

	STRKEY_HASH_SIZE,
	k_hash_str,
	ke_cmp_str
};

hashdata_t h_syms = {
	sizeof(sym_t), 4,

	(e_init_t) sym_init,
	(e_destroy_t) sym_destroy,
	(e_cpy_t) sym_cpy,

	STRKEY_HASH_SIZE,
	k_hash_str,
	ke_cmp_str
};

varrdata_t v_vos = {
	sizeof(vo_t), 4
};

void
prog_init(prog_t *prog)
{
	prog->name = NULL;
	prog->type = MP_T_MOB;
	prog->status = MP_S_DIRTY;

	prog->text = NULL;
	prog->textlen = 0;

	prog->cp = prog->text;
	prog->errbuf = buf_new(0);
	prog->lineno = 0;

	hash_init(&prog->strings, &h_strings);
	hash_init(&prog->syms, &h_syms);

	varr_init(&prog->cstack, &v_ints);
	varr_init(&prog->args, &v_ints);
	prog->curr_block = 0;

	prog->curr_jumptab = -1;
	prog->curr_break_addr = INVALID_ADDR;
	prog->curr_continue_addr = INVALID_ADDR;

	prog->ip = 0;
	varr_init(&prog->code, &v_ints);

	varr_init(&prog->jumptabs, &v_jumptabs);
	varr_init(&prog->iterdata, &v_iterdata);

	varr_init(&prog->data, &v_vos);
}

void
prog_destroy(prog_t *prog)
{
	free_string(prog->name);

	free((void *) prog->text);

	buf_free(prog->errbuf);

	hash_destroy(&prog->strings);
	hash_destroy(&prog->syms);

	varr_destroy(&prog->cstack);
	varr_destroy(&prog->args);

	varr_destroy(&prog->code);

	varr_destroy(&prog->jumptabs);
	varr_destroy(&prog->iterdata);

	varr_destroy(&prog->data);
}

static void
var_add(prog_t *prog, const char *name, int type_tag)
{
	sym_t sym;
	sym_t *s;

	sym.name = str_dup(name);
	sym.type = SYM_VAR;
	sym.s.var.type_tag = type_tag;
	sym.s.var.is_const = FALSE;
	sym.s.var.block = -1;
	sym.s.var.data.i = 0;

	if ((s = (sym_t *) hash_insert(&prog->syms, sym.name, &sym)) == NULL) {
		sym_destroy(&sym);
		compile_error(prog, "%s: duplicate symbol", sym.name);
		return;
	}
	sym_destroy(&sym);
}

int
prog_compile(prog_t *prog)
{
	prog->status = MP_S_DIRTY;
	prog->cp = prog->text;

	buf_clear(prog->errbuf);
	prog->lineno = 1;

	hash_erase(&prog->strings);
	hash_erase(&prog->syms);

	varr_erase(&prog->cstack);
	varr_erase(&prog->args);
	prog->curr_block = 0;

	varr_erase(&prog->code);

	varr_erase(&prog->jumptabs);
	varr_erase(&prog->iterdata);

	prog->curr_jumptab = -1;
	prog->curr_break_addr = INVALID_ADDR;
	prog->curr_continue_addr = INVALID_ADDR;

	var_add(prog, "$_", MT_INT);
	switch (prog->type) {
	case MP_T_MOB:
	case MP_T_OBJ:
	case MP_T_ROOM:
		break;

	case MP_T_SPEC:
		var_add(prog, "$n", MT_CHAR);
		var_add(prog, "$rm", MT_STR);
		var_add(prog, "$add", MT_STR);
		break;
	}

	if (mpc_parse(prog) < 0
	||  !IS_NULLSTR(buf_string(prog->errbuf)))
		return -1;

	cleanup_syms(prog, 0);
	prog->status = MP_S_READY;
	return 0;
}

int
prog_execute(prog_t *prog, int *errcode)
{
	sym_t *sym;

	if (prog->status != MP_S_READY) {
		*errcode = -2;
		return 0;
	}

	if ((*errcode = setjmp(prog->jmpbuf)) == 0)
		execute(prog, 0);

	if ((sym = (sym_t *) hash_lookup(&prog->syms, "$_")) == NULL) {
		fprintf(stderr, "Runtime error: %s: %s: symbol not found\n",
			__FUNCTION__, "$_");
		*errcode = -1;
		return 0;
	}
	if (sym->type != SYM_VAR) {
		fprintf(stderr, "Runtime error: %s: not a %d (got symtype %d)\n",
			sym->name, SYM_VAR, sym->type);
	}
	return sym->s.var.data.i;
}

void *
print_swjump_cb(void *p, va_list ap)
{
	swjump_t *sw = (swjump_t *) p;

	fprintf(stderr, "\n\t%d:\t0x%08x", sw->val.i, sw->addr);
	return NULL;
}

void
prog_dump(prog_t *prog)
{
	int ip;

	for (ip = 0; ip < varr_size(&prog->code); ip++) {
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

			jumptab = varr_get(&prog->jumptabs, jt_offset);
			if (jumptab != NULL)
				varr_foreach(jumptab, print_swjump_cb);
		} else if (p == c_quecolon) {
			fprintf(stderr, " (next: 0x%08x else: 0x%08x)",
				CODE(ip)[1], CODE(ip)[2]);
		} else if (p == c_if) {
			fprintf(stderr,
				" (next: 0x%08x, then: 0x%08x, else: 0x%08x)",
				CODE(ip)[1], CODE(ip)[2], CODE(ip)[3]);
		} else if (p == c_foreach) {
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
execute(prog_t *prog, int ip)
{
	c_fun *c;

	prog->ip = ip;
	while ((c = (c_fun *) varr_get(&prog->code, prog->ip++)) != NULL) {
		if (*c == c_stop)
			break;
		(*c)(prog);
	}
}

sym_t *
sym_lookup(prog_t *prog, const char *name)
{
	sym_t *sym;

	if ((sym = (sym_t *) hash_lookup(&glob_syms, name)) != NULL)
		return sym;

	return (sym_t *) hash_lookup(&prog->syms, name);
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
cleanup_syms(prog_t *prog, int block)
{
	for (; ;) {
		sym_t *sym;

		sym = (sym_t *) hash_foreach(&prog->syms, find_block_cb, block);
		if (sym == NULL)
			break;

#if 0
		log(LOG_INFO, "%s: %s (%d)",
		    __FUNCTION__, sym->name, sym->s.var.block);
#endif
		hash_delete(&prog->syms, sym->name);
	}
}

void
compile_error(prog_t *prog, const char *fmt, ...)
{
	char buf[MAX_STRING_LENGTH];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	buf_printf(prog->errbuf, BUF_END, "%s:%d: %s\n",
		   prog->name, prog->lineno, buf);
}

const char *
alloc_string(prog_t *prog, const char *s)
{
	const char **p = hash_lookup(&prog->strings, s);
	if (p == NULL)
		p = hash_insert(&prog->strings, s, &s);
	return *p;
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
	mpc_init();

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
