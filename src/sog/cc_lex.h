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
 * $Id: cc_lex.h,v 1.6 2001-06-24 10:50:46 avn Exp $
 */

#ifndef _CC_LEX_H_
#define _CC_LEX_H_

typedef struct ctx_t {
	bool val;
	struct cc_eclass_t *rcl;	/* rule class */
	va_list	ap;			/* arguments for predicate */
	const char *stmt;
} ctx_t;

extern ctx_t cc_ctx;

bool cc_fun_call(ctx_t *ctx, const char *rn, const char *arg);

extern jmp_buf cc_jmpbuf;

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

int cc_yylex(void);
void cc_yyerror(const char *s);
int cc_yyparse(void);

#endif

