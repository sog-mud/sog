alpha	[a-zA-Z]
alnum	[a-zA-Z0-9]

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

#define yylval cc_yylval
#define yyerror cc_yyerror
#define yyparse cc_yyparse
#define yylval cc_yylval
#define yy_scan_string cc_yy_scan_string
#define yy_delete_buffer cc_yy_delete_buffer

#include "y.tab.h"
#include "cc_lex.h"

ctx_t cc_ctx;

jmp_buf cc_jmpbuf;

static char buf[128];
static char *s;

#define COPYOUT(c)					\
	do {						\
		if (s - buf > sizeof(buf) - 1)		\
			yyerror("string too long");	\
		*s++ = (c);				\
	} while (0);

#define YY_NO_UNPUT

extern int yyparse();
extern void yyerror(const char *);

%}

%x X_STRING

%%
\"				{ BEGIN X_STRING; s = buf; }
<X_STRING>\\n			{ COPYOUT('\n'); }
<X_STRING>\\r			{ COPYOUT('\r'); }
<X_STRING>\\t			{ COPYOUT('\t'); }
<X_STRING>\\a			{ COPYOUT('\a'); }
<X_STRING>\\\"			{ COPYOUT('\"'); }
<X_STRING>\"			{ COPYOUT('\0');
				  BEGIN 0;
				  yylval.s = str_dup(buf);
				  return STRING;
				}
<X_STRING>.			{ *s++ = *yytext; }

"&&"				return AND;
"||"				return OR;

{alpha}{alnum}*			{ yylval.s = str_dup(yytext);
				  return FUN;
				}

[!()]				return *yytext;
[ \t\n]				;
.				yyerror("unknown character");

%%

void *
expr_check_cb(void *p, va_list ap)
{
	cc_expr_t *e = (cc_expr_t *) p;
	void *rv;
	YY_BUFFER_STATE yy_buf;

	cc_ctx.val = FALSE;
	cc_ctx.rcl = va_arg(ap, cc_eclass_t *);
	cc_ctx.ap = va_arg(ap, va_list);

	yy_buf = yy_scan_string(e->expr);

	if (!setjmp(cc_jmpbuf)) {
		yyparse();
		if (cc_ctx.val)
			rv = (void *) e->mfun;
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
		char buf[256];

		snprintf(buf, sizeof(buf),
			 "cc_fun_call: %s: unknown efun in ruleclass '%s'\n",
			 en, ctx->rcl->name);
		yyerror(buf);
	}

	if (efun->fun == NULL)
		return FALSE;

	return efun->fun(arg, ctx->ap);
}

