/*-
 * Copyright (c) 2001 SoG Development Team
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
 * $Id: mpc_dynafun.c,v 1.10 2001-09-02 16:21:59 fjoe Exp $
 */

#include <stdio.h>

#if !defined(MPC)

#include <merc.h>

#include <sog.h>

#include "mpc_dynafun.h"

static
FOREACH_CB_FUN(has_sp_cb, p, ap)
{
	const char **pspn = (const char **) p;

	const char *spn = va_arg(ap, const char *);
	const char *spn_rm = va_arg(ap, const char *);

	if (!str_cmp(*pspn, spn_rm))
		return NULL;

	if (is_name_strict(*pspn, spn))
		return p;

	return NULL;
}

int
has_sp(CHAR_DATA *ch, const char *spn, const char *spn_rm, const char *spn_add)
{
	if (is_name_strict(spn_add, spn))
		return TRUE;

	return !!varr_foreach(&PC(ch)->specs, has_sp_cb, spn, spn_rm);
}

int
level(CHAR_DATA *ch)
{
	return ch->level;
}

static
FOREACH_CB_FUN(spclass_count_cb, p, ap)
{
	const char **pspn = (const char **) p;

	int spclass = va_arg(ap, int);
	const char *spn_rm = va_arg(ap, const char *);
	int *pcount = va_arg(ap, int *);

	spec_t *spec;

	if (!str_cmp(*pspn, spn_rm))
		return NULL;

	if ((spec = spec_lookup(*pspn)) != NULL
	&&  spec->spec_class == spclass)
		(*pcount)++;

	return NULL;
}

int
spclass_count(CHAR_DATA *ch, const char *spclass_name,
	      const char *spn_rm, const char *spn_add)
{
	int count = 0;
	int spclass;
	spec_t *spec;

	if ((spclass = flag_svalue(spec_classes, spclass_name)) < 0)
		return 0;

	if ((spec = spec_lookup(spn_add)) != NULL
	&&  spec->spec_class == spclass)
		count++;

	varr_foreach(&PC(ch)->specs, spclass_count_cb, spclass, spn_rm, &count);
	return count;
}

void
mob_interpret(CHAR_DATA *ch, const char *argument)
{
	interpret(ch, argument, FALSE);
}

bool
is_immortal(CHAR_DATA *ch)
{
	return IS_IMMORTAL(ch);
}

int
char_sex(CHAR_DATA *ch)
{
	return flag_value(sex_table, mlstr_mval(&ch->gender));
}

OBJ_DATA *
load_obj(CHAR_DATA *ch, int vnum)
{
	OBJ_DATA *obj;

	if ((obj = create_obj(vnum, 0)) == NULL)
		return NULL;

	obj_to_char(obj, ch);
	return obj;
}

#else /* !defined(MPC) */

void
print(int i)
{
	fprintf(stderr, "===> %s: %d\n", __FUNCTION__, i);
}

void
print2(int i, int j)
{
	fprintf(stderr, "===> %s: %d, %d\n", __FUNCTION__, i, j);
}

void
prints(const char *s)
{
	fprintf(stderr, "===> %s: '%s'\n", __FUNCTION__, s);
}

#endif /* !defined(MPC) */
