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
 * $Id: mpc.y,v 1.3 2001-06-18 15:05:35 fjoe Exp $
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
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

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

#if defined(MPC)
#include <module.h>
#endif

#include "_mpc.h"
#include "mpc_const.h"

#define YYPARSE_PARAM prog
#define YYPARSE_PARAM_TYPE prog_t *

#if 0
#if defined(MPC)
#define YYDEBUG 1
#endif
#endif

#undef yylex
#define yylex() mp_lex(prog)

#undef yyerror
#define yyerror(a) mp_error(prog, a)

#define yysslim mp_sslim
#define yystacksize mp_stacksize

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
argtype_popn(prog_t *prog, int n)
{
	assert(prog->args.nused >= n);
	prog->args.nused -= n;
}

/**
 * Get argument type from argument type stack
 */
static int
argtype_get(prog_t *prog, int n, int index)
{
	int *t = varr_get(&prog->args, prog->args.nused - n + index);
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
static void
code(prog_t *prog, const void *opcode)
{
	const void **o = varr_enew(&prog->code);
	*o = opcode;
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

/*--------------------------------------------------------------------
 * string space manipulation functions
 */

/**
 * Make sure the string is allocated in program string space
 */
extern inline const char *
alloc_string(prog_t *prog, const char *s)
{
	const char **p = hash_lookup(&prog->strings, s);
	if (p == NULL)
		p = hash_insert(&prog->strings, s, s);
	return *p;
}

#define BOP_CHECK_TYPES(opname, op1, op2)				\
	do {								\
		if (op1 != op2) {					\
			compile_error(prog,				\
			    "Invalid operand types for '%s' (%d vs. %d)",\
			    opname, op1, op2);				\
			YYERROR;					\
		}							\
	} while (0)

#define OP_INVALID_OPERAND(opname, op)					\
	do {								\
		compile_error(prog,					\
		    "Invalid operand for '%s' (%d)",			\
		    (opname), (op));					\
		YYERROR;						\
	} while (0)

#define INT_BOP(opname, c_bop_fun, op1, op2, rv)			\
	do {								\
		BOP_CHECK_TYPES(opname, op1, op2);			\
									\
		switch (op1) {						\
		case MT_INT:						\
			code(prog, c_bop_fun);				\
			break;						\
									\
		default:						\
			OP_INVALID_OPERAND(opname, op1);		\
		}							\
									\
		rv = MT_INT;						\
	} while (0)

%}

%union {
	int number;
	const char *string;
	int type_tag;
	c_fun cfun;
}

%token L_IDENT L_TYPE L_INT L_STRING
%token L_IF L_ELSE L_FOREACH L_CONTINUE L_BREAK
%token L_ADD_EQ L_SUB_EQ L_DIV_EQ L_MUL_EQ L_MOD_EQ L_AND_EQ L_OR_EQ L_XOR_EQ
%token L_SHL_EQ L_SHR_EQ

%right '='
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

%type <type_tag> expr L_TYPE
%type <number> expr_list expr_list_ne L_INT
%type <string> L_IDENT L_STRING
%type <cfun> assign

%%

stmt_list:	/* empty */
	| stmt_list stmt
	| stmt_list error ';'
	;

stmt:	';'
	| L_TYPE L_IDENT ';' {
		const void *p;
		sym_t sym;

		/*
		 * simply copy $2 here -- no need for str_dup($2) and
		 * free_string($2)
		 */
		sym.type = SYM_VAR;
		sym.name = $2;
		sym.s.var.type_tag = $1;
		sym.s.var.is_const = FALSE;

		switch ($1) {
		case MT_STR:
			sym.s.var.data.s = 0;
			break;

		case MT_INT:
		default:
			sym.s.var.data.i = 0;
			break;
		};

		if ((p = hash_insert(&glob_syms, sym.name, &sym)) == NULL) {
			sym_destroy(&sym);
			compile_error(prog, "%s: duplicate symbol", sym.name);
			YYERROR;
		}
		sym_destroy(&sym);
	}
	| L_TYPE L_IDENT '=' expr ';' {
		const void *p;
		sym_t sym;

		if ($1 != $4) {
			compile_error(prog, "type mismatch (%d vs. %d)",
				      $1, $4);
			YYERROR;
		}

		/*
		 * simply copy $2 here -- no need for str_dup($2) and
		 * free_string($2)
		 */
		sym.type = SYM_VAR;
		sym.name = $2;
		sym.s.var.type_tag = $1;
		sym.s.var.is_const = FALSE;

		if ((p = hash_insert(&glob_syms, sym.name, &sym)) == NULL) {
			sym_destroy(&sym);
			compile_error(prog, "%s: duplicate symbol", sym.name);
			YYERROR;
		}

		code2(prog, c_assign, sym.name);
		sym_destroy(&sym);
	}
	| expr ';' {
		/*
		 * pop calculated value from stack
		 */
		code2(prog, c_pop, (const void *) $1);
	}
	;

expr:	  L_IDENT assign expr %prec '=' {
		sym_t *sym;

		/*
		 * lookup symbol
		 */
		if ((sym = sym_lookup(prog, $1)) == 0) {
			free_string($1);
			compile_error(prog, "%s: unknown identifier", $1);
			YYERROR;
		}
		free_string($1);

		if (sym->type != SYM_VAR) {
			compile_error(prog, "%s: not a variable", $1);
			YYERROR;
		}

		if (sym->s.var.type_tag != $3) {
			compile_error(prog, "type mismatch (%d vs. %d",
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

		/*
		 * lookup symbol
		 */
		if ((sym = sym_lookup(prog, $1)) == 0) {
			free_string($1);
			compile_error(prog, "%s: unknown identifier", $1);
			YYERROR;
		}
		free_string($1);

		if (sym->type != SYM_FUNC) {
			compile_error(prog, "%s: not a function", $1);
			YYERROR;
		}
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
		code2(prog, (const void *) d->rv_tag, (const void *) d->nargs);

		for (i = 0; i < d->nargs; i++) {
			int got_type = argtype_get(prog, $3, i);
			if (got_type != d->argtype[i]) {
				compile_error(prog,
				    "%s: invalid arg %d type %d (%d expected)",
				    $1, i+1, got_type, d->argtype[i]);
				YYERROR;
			}
			code(prog, (const void *) d->argtype[i]);
		}

		$$ = d->rv_tag;

		/*
		 * pop argtypes from stack
		 */
		argtype_popn(prog, $3);
	}
	| L_IDENT {
		sym_t *sym;

		/*
		 * lookup symbol
		 */
		if ((sym = sym_lookup(prog, $1)) == 0) {
			free_string($1);
			compile_error(prog, "%s: unknown identifier", $1);
			YYERROR;
		}
		free_string($1);

		if (sym->type != SYM_VAR) {
			compile_error(prog, "%s: not a variable", $1);
			YYERROR;
		}

		code2(prog, c_push_var, sym->name);
		$$ = sym->s.var.type_tag;
	}
	| L_STRING {
		code2(prog, c_push_const, $1);
		$$ = MT_STR;
	}
	| L_INT {
		code2(prog, c_push_const, (const void *) $1);
		$$ = MT_INT;
	}
	| expr L_LOR expr {
		INT_BOP("||", c_bop_lor, $1, $3, $$);
	}
	| expr L_LAND expr {
		INT_BOP("&&", c_bop_land, $1, $3, $$);
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
		/* XXX c_op_not */
	}
	| '~' expr {
		/* XXX c_op_compl */
	}
	| '(' expr ')'		{ $$ = $2; }
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

expr_list_ne: expr		{ argtype_push(prog, $1); $$ = 1; }
	| expr_list_ne ',' expr	{ argtype_push(prog, $3); $$ = $1 + 1; }

%%

int	mp_parse(prog_t *prog);

int
mp_error(prog_t *prog, const char *errmsg)
{
	compile_error(prog, "%s", errmsg);
	return 1;
}

/*--------------------------------------------------------------------
 * lexer stuff
 */

static int
mp_getc(prog_t *prog)
{
	if (prog->cp - prog->text >= prog->textlen)
		return EOF;

	return *prog->cp++;
}

static void
mp_ungetc(int ch, prog_t *prog)
{
	if (prog->cp > prog->text)
		prog->cp--;
}

static void
skip_comment(prog_t *prog)
{
	/* XXX */
}

static void
skip_line(prog_t *prog)
{
	/* XXX */
}

static bool
gobble(prog_t *prog, int ch)
{
	int c = mp_getc(prog);
	if (c == ch)
		return TRUE;

	mp_ungetc(c, prog);
	return FALSE;
}

#define TRY(c, rv)							\
	do {								\
		if (gobble(prog, (c)))					\
			return (rv);					\
	} while (0)

#define STORE(c)							\
	do {								\
		if (yyp - yytext >= sizeof(yytext)) {			\
			compile_error(prog, "Line too long");		\
			return ' ';					\
		}							\
									\
		*yyp++ = (c);						\
	} while (0)

#define IS_IDENT_CH(ch)							\
	((isalpha(ch) && isascii(ch)) || ch == '$' || ch == '_')

#define MAX_IDENT_LEN	64

struct keyword_t {
	const char *keyword;
	int lexval;
};
typedef struct keyword_t keyword_t;

keyword_t ktab[] = {
	{ "if",		L_IF		},
	{ "else",	L_ELSE		},
	{ "foreach",	L_FOREACH	},
	{ "continue",	L_CONTINUE	},
	{ "break",	L_BREAK		}
};
#define KTAB_SIZE	(sizeof(ktab) / sizeof(keyword_t))

static keyword_t *
keyword_lookup(const char *keyword)
{
	static bool ktab_initialized;

	if (!ktab_initialized) {
		qsort(ktab, KTAB_SIZE, sizeof(keyword_t), cmpstr);
		ktab_initialized = TRUE;
	}

	return bsearch(&keyword, ktab, KTAB_SIZE, sizeof(keyword_t), cmpstr);
}

struct type_t {
	const char *name;
	int type_tag;
};

typedef struct type_t type_t;

static type_t ttab[] = {
	{ "int",	MT_INT },
	{ "string",	MT_STR },
};

#define TTAB_SIZE (sizeof(ttab) / sizeof(type_t))

static type_t *
type_lookup(const char *typename)
{
	static bool ttab_initialized;

	if (!ttab_initialized) {
		qsort(ttab, TTAB_SIZE, sizeof(type_t), cmpstr);
		ttab_initialized = TRUE;
	}

	return bsearch(&typename, ttab, TTAB_SIZE, sizeof(type_t), cmpstr);
}

int
mp_lex(prog_t *prog)
{
	int ch;
	static char yytext[MAX_IDENT_LEN];

	for (; ; ) {
		bool is_hex;
		char *yyp;
		keyword_t *k;
		type_t *t;

		switch ((ch = mp_getc(prog))) {
		case EOF:
			return -1;
			/* NOTREACHED */

		case '\n':
			prog->lineno++;
			break;

		case ' ':
		case '\t':
		case '\f':
		case '\v':
			break;

		case '+':
			TRY('+', L_INC);
			TRY('=', L_ADD_EQ);
			return ch;
			/* NOTREACHED */

		case '-':
			TRY('-', L_DEC);
			TRY('=', L_SUB_EQ);
			return ch;
			/* NOTREACHED */

		case '&':
			TRY('&', L_LAND);
			TRY('=', L_AND_EQ);
			return ch;
			/* NOTREACHED */

		case '|':
			TRY('|', L_LOR);
			TRY('=', L_OR_EQ);
			return ch;
			/* NOTREACHED */

		case '^':
			TRY('=', L_XOR_EQ);
			return ch;
			/* NOTREACHED */

		case '<':
			if (gobble(prog, '<')) {
				TRY('=', L_SHL_EQ);
				return L_SHL;
			}
			TRY('=', L_LE);
			return ch;
			/* NOTREACHED */

		case '>':
			if (gobble(prog, '>')) {
				TRY('=', L_SHR_EQ);
				return L_SHR;
			}
			TRY('=', L_GE);
			return ch;
			/* NOTREACHED */

		case '*':
			TRY('=', L_MUL_EQ);
			return ch;
			/* NOTREACHED */

		case '%':
			TRY('=', L_MOD_EQ);
			return ch;
			/* NOTREACHED */

		case '/':
			if (gobble(prog, '*')) {
				skip_comment(prog);
				break;
			} else if (gobble(prog, '/')) {
				skip_line(prog);
				break;
			}

			TRY('/', L_DIV_EQ);
			return ch;
			/* NOTREACHED */

		case '=':
			TRY('=', L_EQ);
			return ch;
			/* NOTREACHED */

		case '!':
			TRY('=', L_NE);
			return L_NOT;
			/* NOTREACHED */

		case ';':
		case '(':
		case ')':
		case '{':
		case '}':
		case '~':
		case ',':
			return ch;
			/* NOTREACHED */

		case '[':
		case ']':
		case '?':
		case ':':
		case '.':
			goto badch;		/* maybe later */
			/* NOTREACHED */

		case '"':
			/* XXX */
			yyp = yytext;

			STORE('\0');
			yylval.string = alloc_string(prog, yytext);
			return L_STRING;
			/* NOTREACHED */

		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			yyp = yytext;
			is_hex = FALSE;

			STORE(ch);
			if (ch == '0' && gobble(prog, 'x')) {
				STORE('x');
				is_hex = TRUE;
			}

			for (; ;) {
				ch = mp_getc(prog);
				if (is_hex ? !isxdigit(ch) : !isdigit(ch))
					break;

				STORE(ch);
			}
			mp_ungetc(ch, prog);
			STORE('\0');

			yylval.number = strtol(yytext, &yyp, 0);
			if (*yyp != '\0') {
				compile_error(prog,
				    "Invalid number '%s'", yytext);
				return ' ';
			}
			return L_INT;
			/* NOTREACHED */

		default:
			if (!IS_IDENT_CH(ch))
				goto badch;

			yyp = yytext;

			STORE(ch);
			for (; ;) {
				ch = mp_getc(prog);
				if (!IS_IDENT_CH(ch) && !isnumber(ch))
					break;

				STORE(ch);
			}
			mp_ungetc(ch, prog);
			STORE('\0');

			if ((k = keyword_lookup(yytext)) != NULL)
				return k->lexval;

			if ((t = type_lookup(yytext)) != NULL) {
				yylval.type_tag = t->type_tag;
				return L_TYPE;
			}

			yylval.string = str_dup(yytext);
			return L_IDENT;
			/* NOTREACHED */
		}
	}

badch:
	compile_error(prog,
	    "Illegal character (0x%02x) '%c'", (unsigned) ch, (char) ch);
	return ' ';
}

static void
sym_init(sym_t *sym)
{
	sym->name = NULL;
	sym->type = SYM_KEYWORD;
}

static
void
sym_destroy(sym_t *sym)
{
	free_string(sym->name);

	switch (sym->type) {
	case SYM_KEYWORD:
	case SYM_FUNC:
	case SYM_VAR:
		/* do nothing */
		break;
	};
}

static sym_t *
sym_cpy(sym_t *dst, const sym_t *src)
{
	dst->name = str_qdup(src->name);
	dst->type = src->type;

	switch (src->type) {
	case SYM_KEYWORD:
	case SYM_FUNC:
		/* do nothing */
		break;

	case SYM_VAR:
		dst->s.var = src->s.var;
		break;
	}

	return dst;
}

sym_t *
sym_lookup(prog_t *prog, const char *name)
{
	sym_t *sym;

	if ((sym = (sym_t *) hash_lookup(&glob_syms, name)) != NULL)
		return sym;

	return (sym_t *) hash_lookup(&prog->syms, name);
}

varrdata_t v_ints = {
	sizeof(int), 8
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

	prog->text = NULL;
	prog->textlen = 0;

	prog->cp = prog->text;
	prog->errbuf = buf_new(-1);
	prog->lineno = 0;

	varr_init(&prog->code, &v_ints);
	hash_init(&prog->strings, &h_strings);
	hash_init(&prog->syms, &h_syms);
	varr_init(&prog->args, &v_ints);
	varr_init(&prog->data, &v_vos);
}

void
prog_destroy(prog_t *prog)
{
	free_string(prog->name);
	free((void *) prog->text);
	buf_free(prog->errbuf);
	varr_destroy(&prog->code);
	hash_destroy(&prog->strings);
	hash_destroy(&prog->syms);
	varr_destroy(&prog->args);
	varr_destroy(&prog->data);
}

int
prog_compile(prog_t *prog)
{
	prog->cp = prog->text;
	buf_clear(prog->errbuf);
	prog->lineno = 1;
	varr_erase(&prog->code);
	hash_erase(&prog->strings);
	hash_erase(&prog->syms);
	varr_erase(&prog->args);

	if (mp_parse(prog) < 0
	||  !IS_NULLSTR(buf_string(prog->errbuf)))
		return -1;

	return 0;
}

int
prog_execute(prog_t *prog)
{
	prog->ip = 0;
	while (prog->ip < prog->code.nused) {
		c_fun *c = (c_fun *) VARR_GET(&prog->code, prog->ip++);
		(*c)(prog);
	}

	return 0;
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

hash_t glob_syms;

#if defined(MPC)
void
print(int i)
{
	fprintf(stderr, "===> %s: %d\n", __FUNCTION__, i);
}

void
print2(int i, int j)
{
	fprintf(stderr, "===> %s: %d, %d\n", __FUNCTION__, i, j);
}

static dynafun_data_t mpc_dynafun_tab[] = {
	{ "number_range",	MT_INT, 2,
		{ MT_INT, MT_INT } },
	{ "print",		MT_VOID, 1,
		{ MT_INT } },
	{ "print2",		MT_VOID, 2,
		{ MT_INT, MT_INT } },
	{ NULL }
};

const char *mpc_dynafuns[] = {
	"number_range",
	"print",
	"print2",
	NULL
};
#endif

void
mpc_init()
{
	int_const_t *ic;
	const char **pp;
#if defined(MPC)
	module_t m;
#endif

	hash_init(&glob_syms, &h_syms);

	/*
	 * add consts to global symbol table
	 */
	for (ic = ic_tab; ic->name != NULL; ic++) {
		const void *p;
		sym_t sym;

		sym.type = SYM_VAR;
		sym.name = str_dup(ic->name);
		sym.s.var.type_tag = MT_INT;
		sym.s.var.data.i = ic->value;
		sym.s.var.is_const = TRUE;

		if ((p = hash_insert(&glob_syms, sym.name, &sym)) == NULL) {
			log(LOG_ERROR, "%s: duplicate symbol (const)",
			    sym.name);
		}
		sym_destroy(&sym);
	}

	/*
	 * add dynafuns to global symbol table
	 */
	for (pp = mpc_dynafuns; *pp != NULL; pp++) {
		const void *p;
		sym_t sym;

		sym.type = SYM_FUNC;
		sym.name = str_dup(*pp);

		if ((p = hash_insert(&glob_syms, sym.name, &sym)) == NULL) {
			log(LOG_ERROR, "%s: duplicate symbol (func)",
			    sym.name);
		}
		sym_destroy(&sym);
	}

#if defined(MPC)
	init_dynafuns();

	m.dlh = dlopen(NULL, 0);
	if (m.dlh == NULL) {
		fprintf(stderr, "dlopen: %s", dlerror());
		exit(1);
	}

	dynafun_tab_register(mpc_dynafun_tab, &m);
#endif
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
		exitcode = 0;

		rv = prog_execute(&prog);
		fprintf(stderr, "prog_execute: rv = %d\n", rv);
	}

	prog_destroy(&prog);

	return exitcode;
}
#endif
