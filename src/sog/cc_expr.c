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
 * $Id: cc_expr.c,v 1.6 1999-12-18 11:01:40 fjoe Exp $
 */

#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "typedef.h"
#include "const.h"
#include "varr.h"
#include "hash.h"
#include "log.h"
#include "str.h"
#include "mlstring.h"
#include "rfile.h"
#include "cc_expr.h"
#include "tables.h"
#include "flag.h"
#include "db.h"
#include "buffer.h"
#include "hash.h"
#include "strkey_hash.h"

static void	cc_efun_init		(cc_efun_t *);
static void	cc_efun_destroy	(cc_efun_t *);

static void		cc_expr_init	(cc_expr_t *e);
static cc_expr_t *	cc_expr_cpy	(cc_expr_t *dst, cc_expr_t *src);
static void		cc_expr_destroy	(cc_expr_t *e);

varr cc_eclasses;

static varrdata_t v_efuns =
{
	sizeof(cc_efun_t), 1,
	(e_init_t) cc_efun_init,
	(e_destroy_t) cc_efun_destroy,
	NULL
};

void
cc_eclass_init(cc_eclass_t *ecl)
{
	ecl->name = str_empty;
	varr_init(&ecl->efuns, &v_efuns);
}

void
cc_eclass_destroy(cc_eclass_t *ecl)
{
	free_string(ecl->name);
	varr_destroy(&ecl->efuns);
}

cc_eclass_t *
cc_eclass_lookup(const char *ecn)
{
	return (cc_eclass_t *) varr_bsearch(&cc_eclasses, &ecn, cmpstr);
}

cc_efun_t *
cc_efun_lookup(cc_eclass_t *ecl, const char *type)
{
	if (ecl == NULL)
		return NULL;

	return (cc_efun_t *) varr_bsearch(&ecl->efuns, &type, cmpstr);
}

static varrdata_t v_cc_expr =
{
	sizeof(cc_expr_t), 1,
	(e_init_t) cc_expr_init,
	(e_destroy_t) cc_expr_destroy,
	(e_cpy_t) cc_expr_cpy,
};

void
cc_vexpr_init(varr *v)
{
	varr_init(v, &v_cc_expr);
}

void
fread_cc_vexpr(varr *v, const char *ecn, rfile_t *fp)
{
	cc_eclass_t *ecl = cc_eclass_lookup(ecn);

	if ((ecl = cc_eclass_lookup(ecn)) == NULL) {
		db_error("fread_cc_exprset: %s: unknown cc_expr class", ecn);
		fread_to_end(fp);
		return;
	}

	for (;;) {
		bool fMatch = FALSE;

		fread_keyword(fp);
		switch (rfile_tokfl(fp)) {
		case 'D':
			if (IS_TOKEN(fp, "deny")) {
				cc_expr_t *e = varr_enew(v);
				e->mfun = fread_sword(fp);
				e->expr = fread_string(fp);
				fMatch = TRUE;
			}
			break;
		case 'E':
			if (IS_TOKEN(fp, "end"))
				return;
			break;
		}

		if (!fMatch) {
			db_error("fread_cc_expr", "%s: Unknown keyword",
				 rfile_tok(fp));
			fread_to_eol(fp);
		}
	}
}

static void *
fwrite_expr_cb(void *p, va_list ap)
{
	cc_expr_t *e = (cc_expr_t *) p;
	FILE *fp = va_arg(ap, FILE *);

	fprintf(fp, "deny %s %s~\n",
		fix_word(e->mfun), fix_string(e->expr));
	return NULL;
}

void
fwrite_cc_vexpr(varr *v, const char *pre, FILE *fp)
{
	if (varr_isempty(v))
		return;

	fprintf(fp, "%s\n", pre);
	varr_foreach(v, fwrite_expr_cb, fp);
	fprintf(fp, "end\n");
}

static void *
print_expr_cb(void *p, va_list ap)
{
	cc_expr_t *e = (cc_expr_t *) p;
	BUFFER *buf = va_arg(ap, BUFFER *);
	varr *v = va_arg(ap, varr *);

	buf_printf(buf, "  %2d) [%s] Deny %s\n",
		   varr_index(v, p), e->mfun, e->expr);
	return NULL;
}

void
print_cc_vexpr(varr *v, const char *pre, BUFFER *buf)
{
	if (varr_isempty(v))
		return;

	buf_printf(buf, "%s\n", pre);
	varr_foreach(v, print_expr_cb, buf, v);
}

extern void *expr_check_cb(void *, va_list);

const char *
cc_vexpr_check(varr *v, const char *ecn, ...)
{
	const char *rv;
	cc_eclass_t *ecl = cc_eclass_lookup(ecn);
	va_list ap;

	if (ecl == NULL) {
		wizlog("cc_exprset_ok: %s: unknown cc_expr class", ecn);
		return NULL;
	}

	va_start(ap, ecn);
	rv = varr_foreach(v, expr_check_cb, ecl, ap);
	va_end(ap);
	return rv;
}

/*-----------------------------------------------------------------------------
 * static functions
 */

static void
cc_efun_init(cc_efun_t *rfun)
{
	rfun->name = str_empty;
	rfun->fun_name = str_empty;
	rfun->fun = NULL;
}

static void
cc_efun_destroy(cc_efun_t *rfun)
{
	free_string(rfun->name);
	free_string(rfun->fun_name);
}

static void
cc_expr_init(cc_expr_t *e)
{
	e->mfun = str_empty;
	e->expr = str_empty;
}

static cc_expr_t *
cc_expr_cpy(cc_expr_t *dst, cc_expr_t *src)
{
	dst->mfun = str_qdup(src->mfun);
	dst->expr = str_qdup(src->expr);
	return dst;
}

static void
cc_expr_destroy(cc_expr_t *e)
{
	free_string(e->mfun);
	free_string(e->expr);
}

