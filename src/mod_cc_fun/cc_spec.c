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
 * $Id: cc_spec.c,v 1.4 2000-10-22 17:53:39 fjoe Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include "merc.h"

/*
 * varags for this cc_rulecl:
 *	CHAR_DATA * ch		- character to check (!IS_NPC assumed)
 *	const char *spn_rm	- spec to be removed
 *	const char *spn_add	- spec to be added
 */

static void *
has_spec_cb(void *p, va_list ap)
{
	const char **pspn = (const char **) p;

	const char *arg = va_arg(ap, const char *);
	const char *spn_rm = va_arg(ap, const char *);

	if (!str_cmp(*pspn, spn_rm))
		return NULL;

	if (is_name_strict(*pspn, arg))
		return p;

	return NULL;
}

/*
 * has spec
 *
 * arg format: <spec name>...
 * return values: TRUE is `ch' has one of the specs from specified list
 *		  otherwise FALSE
 */
bool
cc_spec_has_spec(const char *arg, va_list ap)
{
	CHAR_DATA *ch = va_arg(ap, CHAR_DATA *);
	const char *spn_rm = va_arg(ap, const char *);
	const char *spn_add = va_arg(ap, const char *);

	if (is_name_strict(spn_add, arg))
		return TRUE;

	return !!varr_foreach(&PC(ch)->specs, has_spec_cb, arg, spn_rm);
}

static void *
spclass_count_cb(void *p, va_list ap)
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

/*
 * spec class limit
 *
 * arg format: <num> <spclass>
 * return values: TRUE if current number of specs with spec class <spclass>
 *		  is not greater than <num>, otherwise FALSE
 */
bool
cc_spec_spclass_limit(const char *arg, va_list ap)
{
	CHAR_DATA *ch = va_arg(ap, CHAR_DATA *);
	const char *spn_rm = va_arg(ap, const char *);
	const char *spn_add = va_arg(ap, const char *);

	int count = 0;
	int limit;
	int spclass;
	spec_t *spec;
	char arg1[MAX_STRING_LENGTH];

	arg = one_argument(arg, arg1, sizeof(arg1));
	if ((spclass = flag_value(spec_classes, arg1)) < 0)
		return FALSE;

	if (!is_number(arg))
		return FALSE;
	limit = atoi(arg);
	
	if ((spec = spec_lookup(spn_add)) != NULL
	&&  spec->spec_class == spclass)
		count++;

	varr_foreach(&PC(ch)->specs, spclass_count_cb, spclass, spn_rm, &count);
	return count <= limit;
}

/*
 * level_lt
 *
 * arg format: <num>
 * return values: TRUE if ch's level is less than <num>, otherwise FALSE
 */
bool
cc_spec_level_lt(const char *arg, va_list ap)
{
	CHAR_DATA *ch = va_arg(ap, CHAR_DATA *);

	if (!is_number(arg))
		return FALSE;
	return ch->level < atoi(arg);
}

