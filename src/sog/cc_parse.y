%{

/*-
 * Copyright (c) 1999 fjoe <fjoe@iclub.nsu.ru>
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
 * $Id: cc_parse.y,v 1.1 1999-12-11 15:56:35 fjoe Exp $
 */

#include <setjmp.h>
#include <stdarg.h>

#include "typedef.h"
#include "str.h"
#include "log.h"

#include "cc_lex.h"

#define YYPARSE_PARAM_TYPE ctx_t *
#define YYPARSE_PARAM ctx

extern int yylex(void);

%}

%union {
	bool val;	/* actual value */
	const char *s;	/* string argument or rule name */
}

%token <s> RULE
%token <s> STRING
%type <val> expr
%left OR
%left AND
%left '!'

%%

stmt:	expr			{ ctx->val = $1; }
	;

expr:	RULE '(' STRING ')'	{ $$ = rule_check(ctx, $1, $3);
				  free_string($1);
				  free_string($3);
				}
	| '(' expr ')'		{ $$ = $2; }
	| '!' expr 		{ $$ = !$2; }
	| expr AND expr		{ $$ = $1 && $3; }
	| expr OR expr		{ $$ = $1 || $3; }
	;

%%

int
yywrap(void)
{
	return 1;
}

#include <stdio.h>

void
yyerror(const char *s)
{
	log(s);
	longjmp(cc_jmpbuf, 1);
}

