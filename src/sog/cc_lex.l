alpha	[a-zA-Z]
alnum	[a-zA-Z0-9]

%{
#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>

#include "typedef.h"
#include "varr.h"
#include "cc_rule.h"
#include "str.h"

#include "y.tab.h"
#include "cc_lex.h"

jmp_buf cc_jmpbuf;

char buf[128];
char *s;

#define COPYOUT(c)					\
	do {						\
		if (s - buf > sizeof(buf) - 1)		\
			yyerror("string too long");	\
		*s++ = (c);				\
	} while (0);

#define YY_NO_UNPUT

extern int yyparse(ctx_t *);
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
				  fprintf(stderr, "STRING: '%s'\n", yylval.s);
				  return STRING;
				}
<X_STRING>.			{ *s++ = *yytext; }

"&&"				return AND;
"||"				return OR;

{alpha}{alnum}*			{ yylval.s = str_dup(yytext);
				  fprintf(stderr, "RULE: '%s'\n", yylval.s);
				  return RULE;
				}

[!()]				return *yytext;
[ \t\n]				;
.				yyerror("unknown character");

%%

void *
expr_match_cb(void *p, va_list ap)
{
	const char *expr = *(const char **) p;
	void *rv;
	ctx_t ctx;
	YY_BUFFER_STATE yy_buf;

	ctx.val = FALSE;
	ctx.rcl = va_arg(ap, cc_rulecl_t *);
	ctx.ap = va_arg(ap, va_list);

	yy_buf = yy_scan_string(expr);

	if (!setjmp(cc_jmpbuf)) {
		yyparse(&ctx);
		if (ctx.val)
			rv = (void *) expr;
		else
			rv = NULL;
	} else {
		rv = NULL;
	}

	yy_delete_buffer(yy_buf);

	return rv;
}

bool
rule_check(ctx_t *ctx, const char *rn, const char *arg)
{
	cc_rulefun_t *rfun;

	rfun = cc_rulefun_lookup(ctx->rcl, rn);
	if (rfun == NULL) {
		char buf[256];

		snprintf(buf, sizeof(buf),
			 "rule_check: %s: unknown rule in ruleclass '%s'\n",
			 rn, ctx->rcl->name);
		yyerror(buf);
	}

	if (rfun->fun == NULL)
		return FALSE;

	return rfun->fun(arg, ctx->ap);
}

