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
 * $Id: cc_rule.h,v 1.5 1999-11-24 07:22:22 fjoe Exp $
 */

#ifndef _CC_RULE_H_
#define _CC_RULE_H_

/*
 * Apache-style condition checking rules
 */

/*
 * condition checking ruleclass
 */
typedef struct cc_rulecl_t {
	const char *name;	/* rule class name		 */
	varr rulefuns;		/* varr of cc_rulefun_t	(sorted) */
} cc_rulecl_t;

extern varr cc_rulecls;		/* varr of cc_rulecl_t */

void cc_rulecl_init	(cc_rulecl_t *);
void cc_rulecl_destroy	(cc_rulecl_t *);

cc_rulecl_t *cc_rulecl_lookup(const char *rcn);

typedef bool (*cc_fun_t)(const char *arg, va_list ap);

/*
 * condition checking function
 */
typedef struct cc_rulefun_t {
	const char *type;			/* rule_fun type	*/
	const char *fun_name;			/* rule_fun name	*/
	cc_fun_t fun;				/* rule_fun 		*/
} cc_rulefun_t;

cc_rulefun_t *cc_rulefun_lookup(cc_rulecl_t *rcl, const char *type);

/*
 * order types
 */
enum {
	CC_O_DENY_ALLOW,	/* `deny' cc's are checked before `allow', */
				/* initial state is OK		 	   */
	CC_O_ALLOW_DENY,	/* `allow' check conditions are checked	   */
				/* before `deny', initial state is FORB	   */
	CC_O_MUTUAL_FAILURE,	/* OK only if matches `allow' cc's and	   */
				/* does not match `deny' cc's		   */
				/* initial state is irrelevant		   */
};

/*
 * a set of cc_rule's with checking order
 */
typedef struct cc_ruleset_t {
	const char *type;	/* type			*/
	flag32_t order;		/* order		*/
	varr allow;		/* allow arg's		*/
	varr deny;		/* deny arg's		*/
} cc_ruleset_t;

void	cc_ruleset_init		(cc_ruleset_t *);
void	cc_ruleset_destroy	(cc_ruleset_t *);

#define cc_ruleset_lookup(v, type) \
	((cc_ruleset_t *) varr_foreach((v), strkey_search_cb, (type)))

void	fread_cc_rules		(rfile_t *fp, const char *rcn, varr *v);
void *	fwrite_cc_ruleset_cb	(void *p, va_list ap);
void *	print_cc_ruleset_cb	(void *p, va_list ap);

#define cc_ruleset_isempty(rs)	\
		(varr_isempty(&(rs)->allow) && varr_isempty(&(rs)->deny))

const char *cc_rules_check(const char *rcn, varr *v, ...);

#endif

