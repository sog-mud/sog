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
 * $Id: cc_expr.h,v 1.3 2001-06-24 10:50:38 avn Exp $
 */

#ifndef _CC_EXPR_H
#define _CC_EXPR_H_

/*
 * condition checking expr class
 */
typedef struct cc_eclass_t {
	const char *name;	/* expr class name		 */
	varr efuns;		/* varr of cc_efun_t	(sorted) */
} cc_eclass_t;

extern varr cc_eclasses;	/* varr of cc_eclass_t */

void cc_eclass_init	(cc_eclass_t *);
void cc_eclass_destroy	(cc_eclass_t *);

cc_eclass_t *cc_eclass_lookup(const char *rcn);

typedef bool (*cc_fun_t)(const char *arg, va_list ap);

/*
 * condition checking function
 */
typedef struct cc_efun_t {
	const char *name;			/* efun name		*/
	const char *fun_name;			/* efun cc_fun_t name	*/
	cc_fun_t fun;				/* efun fun 		*/
} cc_efun_t;

cc_efun_t *cc_efun_lookup(cc_eclass_t *ecl, const char *type);

typedef struct cc_expr_t {
	const char *mfun;	/* main fun	*/
	const char *expr;
} cc_expr_t;

void	cc_vexpr_init	(varr *v);
void	fread_cc_vexpr	(varr *v, const char *rcn, rfile_t *fp);
void	fwrite_cc_vexpr	(varr *v, const char *pre, FILE *fp);
void	print_cc_vexpr	(varr *v, const char *pre, BUFFER *buf);

const char *cc_vexpr_check(varr *v, const char *rcn, ...);
const void *expr_check_cb(void *p, va_list ap);

#endif

