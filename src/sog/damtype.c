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
 * $Id: damtype.c,v 1.4 1999-12-15 15:35:41 fjoe Exp $
 */

#include <string.h>
#include <stdio.h>
#include <limits.h>
#include "merc.h"
#include "db.h"

hash_t damtypes;

void damtype_init(damtype_t *d)
{
	d->dam_name = str_empty;
	mlstr_init(&d->dam_noun, str_empty);
	mlstr_init(&d->dam_gender, str_empty);
	d->dam_class = 0;
	d->dam_slot = -1;
}

damtype_t *damtype_cpy(damtype_t *dst, damtype_t *src)
{
	dst->dam_name = str_qdup(src->dam_name);
	mlstr_cpy(&dst->dam_noun, &src->dam_noun);
	mlstr_cpy(&dst->dam_gender, &src->dam_gender);
	dst->dam_class = src->dam_class;
	dst->dam_slot = src->dam_slot;
	return dst;
}

void damtype_destroy(damtype_t *d)
{
	free_string(d->dam_name);
	mlstr_destroy(&d->dam_noun);
	mlstr_destroy(&d->dam_gender);
}

static void *
damtype_slot_cb(void *p, va_list ap)
{
	damtype_t *d = (damtype_t *) p;

	int slot = va_arg(ap, int);

	if (d->dam_slot == slot)
		return (void*) d->dam_name;
	return NULL;
}

/*
 * Lookup a damtype by slot number.
 * Used for old object loading.
 */
const char *damtype_slot_lookup(int slot)
{
	const char *dn;

	if (slot < 0)
		return NULL;

	dn = hash_foreach(&damtypes, damtype_slot_cb, slot);
	if (IS_NULLSTR(dn))
		db_error("damtype_slot_lookup", "unknown slot %d", slot);
	return str_qdup(dn);
}

mlstring *
damtype_noun(const char *dn)
{
	damtype_t *d;
	STRKEY_CHECK(&damtypes, dn, "damtype_noun");
	d = damtype_lookup(dn);
	if (d != NULL)
		return &d->dam_noun;
	else
		return NULL;
}

int
damtype_class(const char *dn)
{
	damtype_t *d;
	STRKEY_CHECK(&damtypes, dn, "damtype_noun");
	d = damtype_lookup(dn);
	if (d != NULL)
		return d->dam_class;
	else
		return DAM_NONE;
}

