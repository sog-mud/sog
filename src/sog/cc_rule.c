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
 * $Id: cc_rule.c,v 1.1 1999-11-22 14:54:25 fjoe Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>

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

static void	cc_rulefun_init		(cc_rulefun_t *);
static void	cc_rulefun_destroy	(cc_rulefun_t *);
static void	cc_rule_init		(cc_rule_t *);
static void	cc_rule_destroy		(cc_rule_t *);

static void	fread_cc_rule	(rfile_t *fp, cc_rulecl_t *rcl, varr *v);

static bool ruleset_match(cc_rulecl_t *rcl, varr *v, void *p);

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

void
cc_ruleset_init(cc_ruleset_t *rs)
{
	rs->order = RSO_DENY_ALLOW;

	varr_init(&rs->allow, sizeof(cc_rule_t), 1);
	rs->allow.e_init = (varr_e_init_t) cc_rule_init;
	rs->allow.e_destroy = (varr_e_destroy_t) cc_rule_destroy;

	varr_init(&rs->deny, sizeof(cc_rule_t), 1);
	rs->deny.e_init = (varr_e_init_t) cc_rule_init;
	rs->deny.e_destroy = (varr_e_destroy_t) cc_rule_destroy;
}

void
cc_ruleset_destroy(cc_ruleset_t *rs)
{
	varr_destroy(&rs->allow);
	varr_destroy(&rs->deny);
}

void
fread_cc_ruleset(rfile_t *fp, const char *rcn, cc_ruleset_t *rs)
{
	cc_rulecl_t *rcl = cc_rulecl_lookup(rcn);

	if (rcl == NULL) {
		db_error("fread_cc_ruleset: %s: unknown cc_rule class", rcn);
		return;
	}

	for (;;) {
		bool fMatch = FALSE;

		fread_keyword(fp);
		switch (rfile_tokfl(fp)) {
		case 'A':
			if (IS_TOKEN(fp, "Allow")) {
				fread_cc_rule(fp, rcl, &rs->allow);
				fMatch = TRUE;
			}
			break;
		case 'D':
			if (IS_TOKEN(fp, "Deny")) {
				fread_cc_rule(fp, rcl, &rs->deny);
				fMatch = TRUE;
			}
			break;
		case 'E':
			if (IS_TOKEN(fp, "End")) {
				return;
			}
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

typedef struct fwrite_ruleset_t {
	FILE *fp;
	const char *type;
} fwrite_ruleset_t;

static void *
fwrite_ruleset_cb(void *p, va_list ap)
{
	return NULL;
}

void
fwrite_cc_ruleset(FILE *fp, const char *rcn, cc_ruleset_t *rs)
{
	fwrite_ruleset_t frs;

	fprintf(fp, "order '%s'\n", flag_string(cc_order_types, rs->order));

	frs.fp = fp;
	frs.type = "allow";
	varr_foreach(&rs->allow, fwrite_ruleset_cb, &frs);

	frs.type = "deny";
	varr_foreach(&rs->deny, fwrite_ruleset_cb, &frs);

	fprintf(fp, "end");
}

int
cc_ruleset_ok(const char *rcn, cc_ruleset_t *rs, void *p)
{
	bool retval;
	cc_rulecl_t *rcl = cc_rulecl_lookup(rcn);

	if (rcl == NULL) {
		wizlog("cc_ruleset_ok: %s: unknown cc_rule class", rcn);
		return TRUE;
	}

	switch (rs->order) {
	case RSO_DENY_ALLOW:
		retval = TRUE;
		if (ruleset_match(rcl, &rs->deny, p))
			retval = FALSE;
		if (ruleset_match(rcl, &rs->allow, p))
			retval = TRUE;
		break;
	case RSO_ALLOW_DENY:
		retval = FALSE;
		if (ruleset_match(rcl, &rs->allow, p))
			retval = TRUE;
		if (ruleset_match(rcl, &rs->deny, p))
			retval = FALSE;
		break;
	default:
		wizlog("cc_ruleset_ok: unknown ruleset order");
		retval = FALSE;
	}

	return retval;
}

/*-----------------------------------------------------------------------------
 * static functions
 */

static void
cc_rulefun_init(cc_rulefun_t *rfun)
{
	rfun->keyword = str_empty;
	rfun->fun_name = str_empty;
	rfun->fun = NULL;
}

static void
cc_rulefun_destroy(cc_rulefun_t *rfun)
{
	free_string(rfun->keyword);
	free_string(rfun->fun_name);
}

static void
cc_rule_init(cc_rule_t *r)
{
	r->keyword = str_empty;
	r->arg = str_empty;
}

static void
cc_rule_destroy(cc_rule_t *r)
{
	free_string(r->keyword);
	free_string(r->arg);
}

static void
fread_cc_rule(rfile_t *fp, cc_rulecl_t *rcl, varr *v)
{
	cc_rule_t *r = varr_enew(v);
	r->keyword = fread_sword(fp);
	r->arg = fread_string(fp);

	if (cc_rulefun_lookup(rcl, r->keyword) == NULL) {
		log("fread_cc_rule: %s: unknown keyword for cc_rulecl '%s'",
		    r->keyword, rcl->name);
	}
}

static void *
ruleset_match_cb(void *p, va_list ap)
{
	cc_rule_t *r = (cc_rule_t *) p;

	bool *prv = va_arg(ap, bool *);
	void *arg = va_arg(ap, void *);
	cc_rulecl_t *rcl = va_arg(ap, cc_rulecl_t *);

	cc_rulefun_t *rfun = cc_rulefun_lookup(rcl, r->keyword);
	if (rfun != NULL
	&&  rfun->fun != NULL
	&&  rfun->fun(r->arg, arg))
		*prv = TRUE;
	return NULL;
}

static bool
ruleset_match(cc_rulecl_t *rcl, varr *v, void *p)
{
	bool rv = FALSE;
	varr_foreach(v, ruleset_match_cb, &rv, p, rcl);
	return rv;
}

