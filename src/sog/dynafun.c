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
 * $Id: dynafun.c,v 1.5 2000-06-08 18:09:23 fjoe Exp $
 */

#include <stdlib.h>
#include <stdarg.h>
#include <dlfcn.h>

#include "typedef.h"
#include "dynafun.h"
#include "memalloc.h"
#include "log.h"
#include "str.h"
#include "varr.h"
#include "hash.h"
#include "strkey_hash.h"
#include "module.h"

/*
 * dynafun_call is highly arch-dependent
 *
 * currently it works for FreeBSD/i386 and Linux/i386
 * if you wish to add support for other platforms look into
 * valops.c in gdb source tree
 */

/*
 * Twas brillig, and the slithy toves
 *	Did gyre and gimble in the wabe;
 * All mimsy were the borogoves,
 *	And the mome raths outgrabe.
 * 
 * - Beware the Jabberwock, my son!
 *	The jaws that bite, the claws that catch!
 * Beware the Jujub bird, and shun
 *	The frumious Bandersnatch! 
 * 
 * He took his vorpal sword in hand:
 *	Long time the manxome foe he sought
 * So rested he by the Tumtum gree,
 *	And stood awhile in thought.
 * 
 * And as in uffish thought he stood
 *	The Jabberwock, with eyes of flame,
 * Came whiffling through the tulgey wook,
 *	And burbled as it came!
 * 
 * One, two! One, two! And through and through
 *	The vorpal blade went snicker-snack!
 * He left it dead, and with its head
 *	He went galumphing back.
 * 
 * - And has thou slain the Jabberwock?
 *	Come to my arms, my beamish boy!
 * O frabjous day! Calloh! Callay!
 *	He chortled in his joy.
 *
 *				.....
 */

static void		dynafun_init(dynafun_data_t *d);
static dynafun_data_t *	dynafun_cpy(dynafun_data_t *d1, const dynafun_data_t *d2);

typedef void (*dynafun_cb)(dynafun_data_t *d, void *arg);
static void dynafun_foreach(dynafun_data_t *, dynafun_cb cb, void *arg);
static void dynafun_register(dynafun_data_t *d, void *arg);
static void dynafun_unregister(dynafun_data_t *d, void *arg);

static hashdata_t h_dynafuns = {
	sizeof(dynafun_data_t), 8,
	(e_init_t) dynafun_init,
	strkey_destroy,
	(e_cpy_t) dynafun_cpy,

	STRKEY_HASH_SIZE,
	k_hash_str,
	ke_cmp_str
};

static hash_t dynafuns;

void
init_dynafuns(void)
{
	hash_init(&dynafuns, &h_dynafuns);
}

void *
dynafun_call(int rv_tag, cchar_t name, int nargs, ...)
{
	dynafun_data_t *d;
	dynafun_args_t args;
	int i;
	va_list ap;
	va_list args_ap = args.p;

	if ((d = (dynafun_data_t *) hash_lookup(&dynafuns, name)) == NULL) {
		log(LOG_BUG, "dynafun_call: %s: not found", name);
		return NULL;
	}

	if (rv_tag != d->rv_tag) {
		log(LOG_BUG, "dynafun_call: %s: rv type %d does not match "
			     "requested rv type %d",
		    d->name, d->rv_tag, rv_tag);
		return NULL;
	}

	if (d->fun == NULL) {
		log(LOG_BUG, "dynafun_call: %s: NULL fun", d->name);
		return NULL;
	}

	if (nargs != d->nargs) {
		log(LOG_BUG, "dynafun_call: %s: called with %d args (expected %d)", d->name, nargs, d->nargs);
		return NULL;
	}

	va_start(ap, nargs);
	for (i = 0; i < nargs; i++) {
		void *arg;

		switch (d->argtype[i]) {
		case MT_VOID:
			*(void **) args_ap = va_arg(ap, void *);
			arg = va_arg(args_ap, void *);
			continue;
			/* NOTREACHED */

		case MT_VA_LIST:
			*(va_list *) args_ap = va_arg(ap, va_list);
			arg = (void *) va_arg(args_ap, va_list);
			continue;
			/* NOTREACHED */

		case MT_INT:
			*(int *) args_ap = va_arg(ap, int);
			arg = (void *) va_arg(args_ap, int);
			continue;
			/* NOTREACHED */

		case MT_STR:
			*(cchar_t *) args_ap = va_arg(ap, cchar_t);
			arg = (void *) va_arg(args_ap, cchar_t);
			continue;
			/* NOTREACHED */

		case MT_CHAR:
		case MT_OBJ:
		case MT_ROOM:
		case MT_AFFECT:
		case MT_BUFFER:
		case MT_OBJ_INDEX:
		case MT_MOB_INDEX:
			break;

		default:
			va_end(ap);
			log(LOG_BUG, "dynafun_call: %s: %d: unknown type in arg list", d->name, d->argtype[i]);
			return NULL;
		}

		arg = va_arg(ap, void *);
		if (!mem_is(arg, d->argtype[i])) {
			va_end(ap);
			log(LOG_BUG, "dynafun_call: %s: arg[%d] type is not %d", d->name, i, d->argtype[i]);
			return NULL;
		}

		*(void **) args_ap = arg;
		arg = va_arg(args_ap, void *);
	}
	va_end(ap);

	return d->fun(args);
}

void
dynafun_tab_register(dynafun_data_t *dtab, module_t *m)
{
	dynafun_foreach(dtab, dynafun_register, m);
}

void
dynafun_tab_unregister(dynafun_data_t *dtab)
{
	dynafun_foreach(dtab, dynafun_unregister, NULL);
}

/*--------------------------------------------------------------------
 * static functions
 */

static void
dynafun_init(dynafun_data_t *d)
{
	int i;

	d->name = str_empty;
	d->rv_tag = MT_VOID;
	d->nargs = 0;
	for (i = 0; i < DYNAFUN_NARGS; i++)
		d->argtype[i] = MT_VOID;
	d->fun = NULL;
}

static dynafun_data_t *
dynafun_cpy(dynafun_data_t *d1, const dynafun_data_t *d2)
{
	int i;

	/*
	 * do not use `str_qdup' here because *d2 can be (and is)
	 * statically allocated
	 */
	free_string(d1->name);
	d1->name = str_dup(d2->name);

	d1->rv_tag = d2->rv_tag;
	d1->nargs = d2->nargs;
	for (i = 0; i < DYNAFUN_NARGS; i++)
		d1->argtype[i] = d2->argtype[i];
	d1->fun = d2->fun;

	return d1;
}

static void
dynafun_foreach(dynafun_data_t *dtab, dynafun_cb cb, void *arg)
{
	while (dtab->name != NULL)
		cb(dtab++, arg);
}

static void
dynafun_register(dynafun_data_t *d, void *arg)
{
	module_t *m = (module_t *) arg;

	if ((d = hash_insert(&dynafuns, d->name, d)) == NULL) {
		log(LOG_BUG, "dynafun_register: %s: duplicate dynafun name",
		    d->name);
	}

	if ((d->fun = dlsym(m->dlh, d->name)) == NULL) {
		log(LOG_BUG, "dynafun_register: %s: not found",
		    d->name);
	}
}

static void
dynafun_unregister(dynafun_data_t *d, void *arg)
{
	hash_delete(&dynafuns, d->name);
}

