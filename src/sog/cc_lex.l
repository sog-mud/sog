alpha	[a-zA-Z_]
alnum	[a-zA-Z0-9_]

%option noyywrap
%option prefix="cc_yy"

%{
#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>

#include "typedef.h"
#include "varr.h"
#include "cc_expr.h"
#include "str.h"

#include "cc_lex.h"
#include "cc_parse.h"

ctx_t cc_ctx;

jmp_buf cc_jmpbuf;

static char gbuf[128];
static char *s;

#define COPYOUT(c)					\
	do {						\
		if (s > gbuf + sizeof(gbuf) - 1)		\
			yyerror("string too long");	\
		*s++ = (c);				\
	} while (0);

#define YY_NO_UNPUT

%}

%x X_STRING

%%
\"				{ BEGIN X_STRING; s = gbuf; }
<X_STRING>\\n			{ COPYOUT('\n'); }
<X_STRING>\\r			{ COPYOUT('\r'); }
<X_STRING>\\t			{ COPYOUT('\t'); }
<X_STRING>\\a			{ COPYOUT('\a'); }
<X_STRING>\\\"			{ COPYOUT('\"'); }
<X_STRING>\"			{ COPYOUT('\0');
				  BEGIN 0;
				  yylval.s = str_dup(gbuf);
				  return STRING;
				}
<X_STRING>.			{ COPYOUT(*yytext); }

"&&"				return AND;
"||"				return OR;

{alpha}{alnum}*			{ yylval.s = str_dup(yytext);
				  return FUN;
				}

[!()]				return *yytext;
[ \t\n]				;
.				{ snprintf(gbuf, sizeof(gbuf),
					   "%c: unknown character", *yytext);
				  yyerror(gbuf);
				}
%%

const void *
expr_check_cb(void *p, va_list ap)
{
	cc_expr_t *e = (cc_expr_t *) p;
	const void *rv;
	YY_BUFFER_STATE yy_buf;

	cc_ctx.val = FALSE;
	cc_ctx.rcl = va_arg(ap, cc_eclass_t *);
	cc_ctx.ap = va_arg(ap, va_list);
	cc_ctx.stmt = e->expr;

	yy_buf = yy_scan_string(e->expr);

	if (!setjmp(cc_jmpbuf)) {
		yyparse();
		if (cc_ctx.val)
			rv = (const void *) e->mfun;
		else
			rv = NULL;
	} else {
		rv = NULL;
	}

	yy_delete_buffer(yy_buf);

	return rv;
}

bool
cc_fun_call(ctx_t *ctx, const char *en, const char *arg)
{
	cc_efun_t *efun;

	efun = cc_efun_lookup(ctx->rcl, en);
	if (efun == NULL) {
		char _buf[256];

		snprintf(_buf, sizeof(_buf),
			 "cc_fun_call: %s: unknown efun in ruleclass '%s'\n",
			 en, ctx->rcl->name);
		yyerror(_buf);
	}

	if (efun->fun == NULL)
		return FALSE;

	return efun->fun(arg, ctx->ap);
}

