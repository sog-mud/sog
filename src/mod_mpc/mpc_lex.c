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
 * $Id: mpc_lex.c,v 1.24 2003-04-25 12:49:32 fjoe Exp $
 */

#include <ctype.h>
#include <setjmp.h>
#include <stdio.h>
#include <string.h>

#include <typedef.h>
#include <varr.h>
#include <container.h>
#include <avltree.h>
#include <memalloc.h>
#include <dynafun.h>
#include <flag.h>
#include <mprog.h>

#include "mpc_impl.h"
#include "mpc.h"

/*--------------------------------------------------------------------
 * lexer stuff
 */

static int
mpc_getc(mpcode_t *mpc)
{
	if (*mpc->cp == '\0')
		return EOF;

	return *mpc->cp++;
}

static void
mpc_ungetc(int ch __attribute__((unused)), mpcode_t *mpc)
{
	if (mpc->cp > mpc->mp->text)
		mpc->cp--;
}

static bool
gobble(mpcode_t *mpc, int ch)
{
	int c = mpc_getc(mpc);
	if (c == ch)
		return TRUE;

	mpc_ungetc(c, mpc);
	return FALSE;
}

static void
skip_comment(mpcode_t *mpc)
{
	for (; ;) {
		switch (mpc_getc(mpc)) {
		case EOF:
			compile_error(mpc, "unterminated comment");
			return;
			/* NOTREACHED */

		case '\n':
			mpc->lineno++;
			break;

		case '*':
			if (gobble(mpc, '/'))
				return;
			break;
		}
	}
}

static void
skip_line(mpcode_t *mpc)
{
	for (; ;) {
		int ch;

		switch ((ch = mpc_getc(mpc))) {
		case EOF:
			return;
			/* NOTREACHED */

		case '\n':
			mpc_ungetc(ch, mpc);
			return;
			/* NOTREACHED */
		}
	}
}

#define TRY(c, rv)							\
	do {								\
		if (gobble(mpc, (c)))					\
			return (rv);					\
	} while (0)

#define STORE(c)							\
	do {								\
		if (yyp - yytext >= (int) sizeof(yytext)) {		\
			compile_error(mpc, "Line too long");		\
			goto nextch;					\
		}							\
									\
		*yyp++ = (c);						\
	} while (0)

#define IS_IDENT_CH(ch)		((isalpha(ch) && isascii(ch)) || ch == '_')

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
	{ "static",	L_STATIC	},
	{ "persistent",	L_PERSISTENT	},
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

struct flaginfo_t mpc_types[] =
{
	{ "",		TABLE_INTVAL,		FALSE	},

	{ "int",	MT_INT,			FALSE	},
	{ "string",	MT_STR,			FALSE	},
	{ "char",	MT_CHAR,		FALSE	},
	{ "obj",	MT_OBJ,			FALSE	},
	{ "room",	MT_ROOM,		FALSE	},

	{ NULL, 0, FALSE }
};

int
mpc_lex(mpcode_t *mpc)
{
	int ch;
	static char yytext[MAX_STRING_LENGTH];

	for (; ; ) {
		bool is_hex;
		char *yyp;
		keyword_t *k;
		mpc_iter_t *iter;
		int type_tag;

		switch ((ch = mpc_getc(mpc))) {
		case EOF:
			return -1;
			/* NOTREACHED */

		case '\n':
			mpc->lineno++;
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
			TRY('>', L_ARROW);
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
			if (gobble(mpc, '<')) {
				TRY('=', L_SHL_EQ);
				return L_SHL;
			}
			TRY('=', L_LE);
			return ch;
			/* NOTREACHED */

		case '>':
			if (gobble(mpc, '>')) {
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
			if (gobble(mpc, '*')) {
				skip_comment(mpc);
				break;
			} else if (gobble(mpc, '/')) {
				skip_line(mpc);
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
		case '[':
		case ']':
			return ch;
			/* NOTREACHED */

		case '.':
			goto badch;		/* maybe later */
			/* NOTREACHED */

		case '"':
			yyp = yytext;

			for (; ; ) {
				ch = mpc_getc(mpc);
				if (ch == EOF) {
					compile_error(mpc,
					    "EOF while parsing string");
					break;
				}

				if (ch == '"')
					break;

				if (ch == '\\') {
					ch = mpc_getc(mpc);
					if (ch == EOF) {
						compile_error(mpc,
						    "EOF while parsing string");
						break;
					}

					ch = backslash(ch);
				}

				STORE(ch);
			}

			STORE('\0');
			mpc_lval.string = alloc_string(mpc, yytext);
			return L_STRING;
			/* NOTREACHED */

		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			yyp = yytext;
			is_hex = FALSE;

			STORE(ch);
			if (ch == '0' && gobble(mpc, 'x')) {
				STORE('x');
				is_hex = TRUE;
			}

			for (; ;) {
				ch = mpc_getc(mpc);
				if (is_hex ? !isxdigit(ch) : !isdigit(ch))
					break;

				STORE(ch);
			}
			mpc_ungetc(ch, mpc);
			STORE('\0');

			mpc_lval.number = strtol(yytext, &yyp, 0);
			if (*yyp != '\0') {
				compile_error(mpc,
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
				ch = mpc_getc(mpc);
				if (!IS_IDENT_CH(ch) && !isdigit(ch))
					break;

				STORE(ch);
			}
			mpc_ungetc(ch, mpc);
			STORE('\0');

			if ((k = keyword_lookup(yytext)) != NULL)
				return k->lexval;

			if ((type_tag = flag_svalue(mpc_types, yytext)) >= 0) {
				mpc_lval.type_tag = type_tag;
				return L_TYPE;
			}

			if ((iter = mpc_iter_lookup(yytext)) != NULL) {
				mpc_lval.iter = iter;
				return L_ITER;
			}

			mpc_lval.string = alloc_string(mpc, yytext);
			return L_IDENT;
			/* NOTREACHED */
		}

nextch:
		;
	}

badch:
	compile_error(mpc,
	    "Illegal character (0x%02x) '%c'", (unsigned) ch, (char) ch);
	return ' ';
}

