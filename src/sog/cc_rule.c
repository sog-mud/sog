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
 * $Id: cc_rule.c,v 1.7 1999-12-13 14:10:39 avn Exp $
 */

#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "typedef.h"
#include "const.h"
#include "varr.h"
#include "log.h"
#include "str.h"
#include "rfile.h"
#include "cc_rule.h"
#include "tables.h"
#include "flag.h"
#include "db.h"
#include "buffer.h"
#include "hash.h"
#include "strkey_hash.h"

static void	cc_rulefun_init		(cc_rulefun_t *);
static void	cc_rulefun_destroy	(cc_rulefun_t *);

varr cc_rulecls;

void
cc_rulecl_init(cc_rulecl_t *rcl)
{
	rcl->name = str_empty;
	varr_init(&rcl->rulefuns, sizeof(cc_rulefun_t), 1);
	rcl->rulefuns.e_init = (varr_e_init_t) cc_rulefun_init;
	rcl->rulefuns.e_destroy = (varr_e_destroy_t) cc_rulefun_destroy;
}

void
cc_rulecl_destroy(cc_rulecl_t *rcl)
{
	free_string(rcl->name);
	varr_destroy(&rcl->rulefuns);
}

cc_rulecl_t *
cc_rulecl_lookup(const char *rcn)
{
	return (cc_rulecl_t *) varr_bsearch(&cc_rulecls, &rcn, cmpstr);
}

cc_rulefun_t *
cc_rulefun_lookup(cc_rulecl_t *rcl, const char *type)
{
	if (rcl == NULL)
		return NULL;

	return (cc_rulefun_t *) varr_bsearch(&rcl->rulefuns, &type, cmpstr);
}

void
cc_ruleset_init(cc_ruleset_t *rs)
{
	rs->order = CC_O_DENY_ALLOW;

	varr_init(&rs->allow, sizeof(const char *), 1);
	rs->allow.e_init = (varr_e_init_t) strkey_init;
	rs->allow.e_destroy = (varr_e_destroy_t) strkey_destroy;

	varr_init(&rs->deny, sizeof(const char *), 1);
	rs->deny.e_init = (varr_e_init_t) strkey_init;
	rs->deny.e_destroy = (varr_e_destroy_t) strkey_destroy;
}

void
cc_ruleset_destroy(cc_ruleset_t *rs)
{
	varr_destroy(&rs->allow);
	varr_destroy(&rs->deny);
}

void
fread_cc_ruleset(cc_ruleset_t *rs, const char *rcn, rfile_t *fp)
{
	cc_rulecl_t *rcl = cc_rulecl_lookup(rcn);

	if ((rcl = cc_rulecl_lookup(rcn)) == NULL) {
		db_error("fread_cc_ruleset: %s: unknown cc_rule class", rcn);
		fread_to_end(fp);
		return;
	}

	for (;;) {
		bool fMatch = FALSE;

		fread_keyword(fp);
		switch (rfile_tokfl(fp)) {
		case 'A':
			if (IS_TOKEN(fp, "Allow")) {
				const char **ps = varr_enew(&rs->allow);
				*ps = fread_string(fp);
				fMatch = TRUE;
			}
			break;
		case 'D':
			if (IS_TOKEN(fp, "Deny")) {
				const char **ps = varr_enew(&rs->deny);
				*ps = fread_string(fp);
				fMatch = TRUE;
			}
			break;
		case 'E':
			if (IS_TOKEN(fp, "End"))
				return;
			break;
		case 'O':
			KEY("Order", rs->order,
			    fread_fword(cc_order_types, fp));
			break;
		}

		if (!fMatch) {
			db_error("fread_cc_ruleset", "%s: Unknown keyword",
				 rfile_tok(fp));
			fread_to_eol(fp);
		}
	}
}

static void *
fwrite_arg_cb(void *p, va_list ap)
{
	const char **parg = (const char **) p;
	FILE *fp = va_arg(ap, FILE *);
	const char *rn = va_arg(ap, const char *);

	fprintf(fp, "%s %s~\n",
		rn, fix_string(*parg));
	return NULL;
}

void
fwrite_cc_ruleset(cc_ruleset_t *rs, const char *pre, FILE *fp)
{
	if (cc_ruleset_isempty(rs))
		return;

	fprintf(fp, "%s\n"
		    "Order %s\n",
		pre, 
		fix_word(flag_string(cc_order_types, rs->order)));

	switch (rs->order) {
	case CC_O_DENY_ALLOW:
		varr_foreach(&rs->deny, fwrite_arg_cb, fp, "Deny");
		varr_foreach(&rs->allow, fwrite_arg_cb, fp, "Allow");
		break;

	case CC_O_ALLOW_DENY:
	default:
		varr_foreach(&rs->allow, fwrite_arg_cb, fp, "Allow");
		varr_foreach(&rs->deny, fwrite_arg_cb, fp, "Deny");
		break;
	}

	fprintf(fp, "End\n");
}

static void *
print_arg_cb(void *p, va_list ap)
{
	const char **parg = (const char **) p;
	BUFFER *buf = va_arg(ap, BUFFER *);
	const char *rn = va_arg(ap, const char *);
	varr *v = va_arg(ap, varr *);

	buf_printf(buf, "  (%2d) %s [%s]\n",
		   varr_index(v, parg), rn, *parg);
	return NULL;
}

void
print_cc_ruleset(cc_ruleset_t *rs, const char *pre, BUFFER *buf)
{
	if (cc_ruleset_isempty(rs))
		return;

	buf_printf(buf, "%s\n"
			"       Order [%s]\n",
		   pre,
		   fix_word(flag_string(cc_order_types, rs->order)));

	switch (rs->order) {
	case CC_O_DENY_ALLOW:
		varr_foreach(&rs->deny, print_arg_cb, buf, "Deny", &rs->deny);
		varr_foreach(&rs->allow, print_arg_cb, buf, "Allow", &rs->allow);
		break;

	case CC_O_ALLOW_DENY:
	default:
		varr_foreach(&rs->allow, print_arg_cb, buf, "Allow", &rs->allow);
		varr_foreach(&rs->deny, print_arg_cb, buf, "Deny", &rs->deny);
		break;
	}

	buf_add(buf, "\n");
}

extern void *expr_match_cb(void *, va_list);

bool
cc_ruleset_check(const char *rcn, cc_ruleset_t *rs, ...)
{
	bool rv;
	cc_rulecl_t *rcl = cc_rulecl_lookup(rcn);
	va_list ap;

	if (rcl == NULL) {
		wizlog("cc_ruleset_ok: %s: unknown cc_rule class", rcn);
		return NULL;
	}

	va_start(ap, rs);

	switch (rs->order) {
	case CC_O_DENY_ALLOW:
		rv = TRUE;
		if (varr_foreach(&rs->deny, expr_match_cb, rcl, ap))
			rv = FALSE;
		if (varr_foreach(&rs->allow, expr_match_cb, rcl, ap))
			rv = TRUE;
		break;
	case CC_O_ALLOW_DENY:
		rv = FALSE;
		if (varr_foreach(&rs->allow, expr_match_cb, rcl, ap))
			rv = TRUE;
		if (varr_foreach(&rs->deny, expr_match_cb, rcl, ap))
			rv = FALSE;
		break;
	case CC_O_MUTUAL_FAILURE:
		if (varr_foreach(&rs->allow, expr_match_cb, rcl, ap)
		&&  !varr_foreach(&rs->deny, expr_match_cb, rcl, ap))
			rv = TRUE;
		else
			rv = FALSE;
		break;
	default:
		wizlog("cc_ruleset_rv: unknown ruleset order");
		rv = FALSE;
	}

	va_end(ap);
	return rv;
}

/*-----------------------------------------------------------------------------
 * static functions
 */

static void
cc_rulefun_init(cc_rulefun_t *rfun)
{
	rfun->type = str_empty;
	rfun->fun_name = str_empty;
	rfun->fun = NULL;
}

static void
cc_rulefun_destroy(cc_rulefun_t *rfun)
{
	free_string(rfun->type);
	free_string(rfun->fun_name);
}
