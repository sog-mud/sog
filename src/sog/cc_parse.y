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
 * $Id: cc_parse.y,v 1.8 2000-02-10 14:08:44 fjoe Exp $
 */

#include <setjmp.h>
#include <stdarg.h>

#include "typedef.h"
#include "str.h"
#include "log.h"

#include "cc_lex.h"

#define yychar cc_yychar
#define yycheck cc_yycheck
#define yydebug cc_yydebug
#define yydefred cc_yydefred
#define yydgoto cc_yydgoto
#define yyerrflag cc_yyerrflag
#define yyerror cc_yyerror
#define yygindex cc_yygindex
#define yylen cc_yylen
#define yylex cc_yylex
#define yylhs cc_yylhs
#define yylval cc_yylval
#define yynerrs cc_yynerrs
#define yyparse cc_yyparse
#define yyrindex cc_yyrindex
#define yysindex cc_yysindex
#define yyss cc_yyss
#define yyssp cc_yyssp
#define yytable cc_yytable
#define yyval cc_yyval
#define yyvs cc_yyvs
#define yyvsp cc_yyvsp

#define yysslim cc_yysslim
#define yystacksize cc_yystacksize

extern int yylex();

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
yyerror(const char *s)
{
	log(LOG_ERROR, "cc_yyerror: %s (%s)", s, cc_ctx.stmt);
	longjmp(cc_jmpbuf, 1);
}

