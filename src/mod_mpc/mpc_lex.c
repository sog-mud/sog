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
 * $Id: mpc_lex.c,v 1.9 2001-07-31 18:14:58 fjoe Exp $
 */

#include <ctype.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <typedef.h>
#include <varr.h>
#include <hash.h>
#include <memalloc.h>
#include <dynafun.h>

#include "mpc_impl.h"
#include "mpc_iter.h"
#include "mpc.h"

/*--------------------------------------------------------------------
 * lexer stuff
 */

static int
mpc_getc(prog_t *prog)
{
	if (prog->cp - prog->text >= (int) prog->textlen)
		return EOF;

	return *prog->cp++;
}

static void
mpc_ungetc(int ch, prog_t *prog)
{
	if (prog->cp > prog->text)
		prog->cp--;
}

static bool
gobble(prog_t *prog, int ch)
{
	int c = mpc_getc(prog);
	if (c == ch)
		return TRUE;

	mpc_ungetc(c, prog);
	return FALSE;
}

static void
skip_comment(prog_t *prog)
{
	for (; ;) {
		switch (mpc_getc(prog)) {
		case EOF:
			compile_error(prog, "unterminated comment");
			return;
			/* NOTREACHED */

		case '\n':
			prog->lineno++;
			break;

		case '*':
			if (gobble(prog, '/'))
				return;
			break;
		}
	}
}

static void
skip_line(prog_t *prog)
{
	for (; ;) {
		int ch;

		switch ((ch = mpc_getc(prog))) {
		case EOF:
			return;
			/* NOTREACHED */

		case '\n':
			mpc_ungetc(ch, prog);
			return;
			/* NOTREACHED */
		}
	}
}

#define TRY(c, rv)							\
	do {								\
		if (gobble(prog, (c)))					\
			return (rv);					\
	} while (0)

#define STORE(c)							\
	do {								\
		if (yyp - yytext >= (int) sizeof(yytext)) {		\
			compile_error(prog, "Line too long");		\
			goto nextch;					\
		}							\
									\
		*yyp++ = (c);						\
	} while (0)

#define IS_IDENT_CH(ch)		((isalpha(ch) && isascii(ch)) || ch == '_')

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
	{ "break",	L_BREAK		},
	{ "switch",	L_SWITCH	},
	{ "case",	L_CASE		},
	{ "default",	L_DEFAULT	},
	{ "return",	L_RETURN	},
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
	{ "mob",	MT_CHAR },
	{ "obj",	MT_OBJ },
	{ "room",	MT_ROOM }
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
mpc_lex(prog_t *prog)
{
	int ch;
	static char yytext[MAX_IDENT_LEN];

	for (; ; ) {
		bool is_hex;
		char *yyp;
		keyword_t *k;
		type_t *t;
		iter_t *iter;

		switch ((ch = mpc_getc(prog))) {
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

			TRY('=', L_DIV_EQ);
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
		case ':':
		case '?':
			return ch;
			/* NOTREACHED */

		case '[':
		case ']':
		case '.':
			goto badch;		/* maybe later */
			/* NOTREACHED */

		case '"':
			yyp = yytext;

			for (; ; ) {
				ch = mpc_getc(prog);
				if (ch == EOF) {
					compile_error(prog,
					    "EOF while parsing string");
					break;
				}

				if (ch == '"')
					break;

				if (ch == '\\') {
					ch = mpc_getc(prog);
					if (ch == EOF) {
						compile_error(prog,
						    "EOF while parsing string");
						break;
					}

					ch = backslash(ch);
				}

				STORE(ch);
			}

			STORE('\0');
			mpc_lval.string = alloc_string(prog, yytext);
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
				ch = mpc_getc(prog);
				if (is_hex ? !isxdigit(ch) : !isdigit(ch))
					break;

				STORE(ch);
			}
			mpc_ungetc(ch, prog);
			STORE('\0');

			mpc_lval.number = strtol(yytext, &yyp, 0);
			if (*yyp != '\0') {
				compile_error(prog,
				    "Invalid number '%s'", yytext);
				return L_INT;
			}
			return L_INT;
			/* NOTREACHED */

		default:
			/* allow '$' to be first letter of identifier */
			if (!IS_IDENT_CH(ch) && ch != '$')
				goto badch;

			yyp = yytext;

			STORE(ch);
			for (; ;) {
				ch = mpc_getc(prog);
				if (!IS_IDENT_CH(ch) && !isnumber(ch))
					break;

				STORE(ch);
			}
			mpc_ungetc(ch, prog);
			STORE('\0');

			if ((k = keyword_lookup(yytext)) != NULL)
				return k->lexval;

			if ((t = type_lookup(yytext)) != NULL) {
				mpc_lval.type_tag = t->type_tag;
				return L_TYPE;
			}

			if ((iter = iter_lookup(yytext)) != NULL) {
				mpc_lval.iter = iter;
				return L_ITER;
			}

			mpc_lval.string = alloc_string(prog, yytext);
			return L_IDENT;
			/* NOTREACHED */
		}

nextch:
	}

badch:
	compile_error(prog,
	    "Illegal character (0x%02x) '%c'", (unsigned) ch, (char) ch);
	return ' ';
}

