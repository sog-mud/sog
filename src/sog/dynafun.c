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
 * $Id: dynafun.c,v 1.22 2003-04-24 12:42:19 fjoe Exp $
 */

#include <stdlib.h>
#include <dlfcn.h>

#include <typedef.h>
#include <memalloc.h>
#include <log.h>
#include <str.h>
#include <varr.h>
#include <container.h>
#include <dynafun.h>
#include <module.h>
#include <flag.h>
#include <tables.h>
#include <avltree.h>

/*
 * dynafun_build_args is highly arch-dependent
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
 * Twas brillig, and the slithy toves
 *	Did gyre and gimble in the wabe;
 * All mimsy were the borogoves,
 *	And the mome raths outgrabe.
 */

static dynafun_data_t *dynafun_build_args(
    const char *name, dynafun_args_t *args, int nargs, va_list ap);

#define FOREACH_DYNAFUN(dtab)	for (; dtab->name != NULL; dtab++)
static void dynafun_register(dynafun_data_t *d, module_t *m);
static void dynafun_unregister(dynafun_data_t *d);

static void
dynafun_init(dynafun_data_t *d)
{
	int i;

	d->name = str_empty;
	d->rv_tag = MT_VOID;
	d->nargs = 0;
	for (i = 0; i < DYNAFUN_NARGS; i++) {
		d->argtype[i].type_tag = MT_PVOID;
		d->argtype[i].nullable = FALSE;
	}
	d->fun = NULL;
}

static avltree_info_t c_info_dynafuns = {
	&avltree_ops,

	(e_init_t) dynafun_init,
	strkey_destroy,

	MT_PVOID, sizeof(dynafun_data_t), ke_cmp_str,
};

avltree_t dynafuns;

void
init_dynafuns(void)
{
	c_init(&dynafuns, &c_info_dynafuns);
}

void *
dynafun_call(int rv_tag, const char *name, int nargs, ...)
{
	dynafun_data_t *d;
	dynafun_args_t args;
	va_list ap;

	va_start(ap, nargs);
	d = dynafun_build_args(name, &args, nargs, ap);
	va_end(ap);

	if (d == NULL)
		return NULL;

	if (rv_tag != d->rv_tag) {
		log(LOG_BUG, "dynafun_call: %s: rv type %d does not match requested rv type %d",
		    d->name, d->rv_tag, rv_tag);
		return NULL;
	}

	return d->fun(args);
}

void
dynaproc_call(const char *name, int nargs, ...)
{
	dynafun_data_t *d;
	dynafun_args_t args;
	va_list ap;

	va_start(ap, nargs);
	d = dynafun_build_args(name, &args, nargs, ap);
	va_end(ap);

	if (d == NULL)
		return;

	d->fun(args);
}

void
dynafun_tab_register(dynafun_data_t *dtab, module_t *m)
{
	FOREACH_DYNAFUN(dtab)
		dynafun_register(dtab, m);
}

void
dynafun_tab_unregister(dynafun_data_t *dtab)
{
	FOREACH_DYNAFUN(dtab)
		dynafun_unregister(dtab);
}

bool
dynafun_check_arg(dynafun_data_t *d, int i, const void *arg)
{
	if (arg == NULL) {
		if (!d->argtype[i].nullable) {
			log(LOG_BUG, "%s: %s: arg[%d] type is not nullable",
			    __FUNCTION__, d->name, i+1);
			return FALSE;
		}
	} else if (d->argtype[i].type_tag != MT_STR
	       &&  d->argtype[i].type_tag != MT_MLSTR
	       &&  !mem_is(arg, d->argtype[i].type_tag)) {
		log(LOG_BUG, "%s: %s: invalid arg[%d] type '%s' ('%s' expected)",
		    __FUNCTION__,
		    d->name, i+1,
		    flag_string(mt_types, mem_type(arg)),
		    flag_string(mt_types, d->argtype[i].type_tag));
		return FALSE;
	}

	return TRUE;
}

/*--------------------------------------------------------------------
 * static functions
 */

static dynafun_data_t *
dynafun_build_args(const char *name, dynafun_args_t *args, int nargs, va_list ap)
{
	dynafun_data_t *d;
	int i;
	va_list args_ap = args->p;

	if ((d = dynafun_data_lookup(name)) == NULL) {
		log(LOG_BUG, "dynafun_call: %s: not found", name);
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

	for (i = 0; i < nargs; i++) {
		const void *arg;

		switch (d->argtype[i].type_tag) {
		case MT_PVOID:
		case MT_PCVOID:
		case MT_ACTOPT:
		case MT_PCCHAR:
		case MT_PCHAR:
		case MT_PINT:
		case MT_SPEC_SKILL:
		case MT_FLAGINFO:
			*(void **) args_ap = va_arg(ap, void *);
			arg = va_arg(args_ap, void *);
			break;

		case MT_VA_LIST:
			*(va_list *) args_ap = va_arg(ap, va_list);
			arg = (const void *) va_arg(args_ap, va_list);
			break;

		case MT_INT:
		case MT_UINT:
			*(int *) args_ap = va_arg(ap, int);
			arg = (const void *) va_arg(args_ap, int);
			break;

		case MT_SIZE_T:
			*(size_t *) args_ap = va_arg(ap, size_t);
			arg = (const void *) va_arg(args_ap, size_t);
			break;

		case MT_BOOL:
			*(bool *) args_ap = va_arg(ap, bool);
			arg = (const void *) va_arg(args_ap, bool);
			break;

		case MT_STR:
		case MT_CHAR:
		case MT_OBJ:
		case MT_ROOM:
		case MT_AREA:
		case MT_AFFECT:
		case MT_BUFFER:
		case MT_OBJ_INDEX:
		case MT_MOB_INDEX:
		case MT_DESCRIPTOR:
		case MT_SKILL:
		case MT_MLSTR:
		case MT_GMLSTR:
			arg = va_arg(ap, void *);
			*(const void **) args_ap = arg;
			arg = va_arg(args_ap, void *);

			if (!dynafun_check_arg(d, i, arg)) {
				va_end(ap);
				return NULL;
			}
			break;

		default:
			va_end(ap);
			log(LOG_BUG, "dynafun_call: %s: invalid arg[%d] type %s (%d)",
			    d->name, i+1,
			    flag_string(mt_types, d->argtype[i].type_tag),
			    d->argtype[i].type_tag);
			return NULL;
			/* NOTREACHED */
		}
	}

	return d;
}

static void
dynafun_register(dynafun_data_t *d, module_t *m)
{
	dynafun_data_t *d2;
	int i;

	if ((d2 = c_insert(&dynafuns, d->name)) == NULL) {
		log(LOG_BUG, "dynafun_register: %s: duplicate dynafun name",
		    d->name);
	}

	/*
	 * do not use `str_qdup' here because *d2 can be (and is)
	 * statically allocated
	 */
	d2->name = str_dup(d->name);

	d2->rv_tag = d->rv_tag;
	d2->nargs = d->nargs;
	for (i = 0; i < DYNAFUN_NARGS; i++)
		d2->argtype[i] = d->argtype[i];

	if ((d2->fun = dlsym(m->dlh, d2->name)) == NULL)
		log(LOG_BUG, "dynafun_register: %s: not found", d2->name);
}

static void
dynafun_unregister(dynafun_data_t *d)
{
	c_delete(&dynafuns, d->name);
}
