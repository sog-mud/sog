%{

/*-
 * Copyright (c) 1999 SoG Development Team
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
 * $Id: cc_parse.y,v 1.9 2001-06-24 10:50:46 avn Exp $
 */

#include <setjmp.h>
#include <stdarg.h>

#include "typedef.h"
#include "str.h"
#include "log.h"

#include "cc_lex.h"

%}

%union {
	bool val;	/* actual value */
	const char *s;	/* string argument or rule name */
}

%token <s> FUN
%token <s> STRING
%type <val> expr
%left OR
%left AND
%left '!'

%%

stmt:	expr			{ cc_ctx.val = $1; }
	;

expr:	FUN '(' STRING ')'	{ $$ = cc_fun_call(&cc_ctx, $1, $3);
				  free_string($1);
				  free_string($3);
				}
	| '(' expr ')'		{ $$ = $2; }
	| '!' expr 		{ $$ = !$2; }
	| expr AND expr		{ $$ = $1 && $3; }
	| expr OR expr		{ $$ = $1 || $3; }
	;

%%

void
cc_yyerror(const char *s)
{
	log(LOG_ERROR, "cc_yyerror: %s (%s)", s, cc_ctx.stmt);
	longjmp(cc_jmpbuf, 1);
}

