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
 * $Id: dynafun.h,v 1.6 2001-06-16 18:40:08 fjoe Exp $
 */

#ifndef _DYNAFUN_H_
#define _DYNAFUN_H_

#define DYNAFUN_NARGS 6

typedef struct dynafun_args_t {
	char p[DYNAFUN_NARGS * sizeof(void *)];
} dynafun_args_t;
typedef void *(*dynafun_t)(dynafun_args_t);

typedef struct dynafun_data_t dynafun_data_t;
struct dynafun_data_t {
	const char *name;
	int rv_tag;
	int nargs;
	int argtype[DYNAFUN_NARGS];
	dynafun_t fun;
};

extern hash_t dynafuns;		/* (dynafun_data_t) */

extern inline dynafun_data_t *
dynafun_data_lookup(const char *name)
{
	return (dynafun_data_t *) hash_lookup(&dynafuns, name);
}

void	init_dynafuns(void);

void *	dynafun_call(int rv_tag, cchar_t name, int nargs, ...);
void	dynaproc_call(cchar_t name, int nargs, ...);

void	dynafun_tab_register(dynafun_data_t *, module_t *m);
void	dynafun_tab_unregister(dynafun_data_t *);

#endif
