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
 * $Id: cc_obj_wear.c,v 1.3 2000-10-22 17:53:39 fjoe Exp $
 */

#include <stdio.h>
#include "merc.h"

/*
 * varags for this cc_rulecl:
 *	CHAR_DATA * ch		- character to check
 */

/*
 * race is in list
 *
 * arg format: <race name>...
 * return values: TRUE is ch's race is in specified list
 *		  otherwise FALSE
 */
bool
cc_obj_wear_race(const char *arg, va_list ap)
{
	CHAR_DATA *ch = va_arg(ap, CHAR_DATA *);
	return is_name_strict(ch->race, arg);
}

/*
 * class is in list
 *
 * arg format: <race name>...
 * return values: TRUE is ch's class is in specified list
 *		  otherwise FALSE
 */
bool
cc_obj_wear_class(const char *arg, va_list ap)
{
	CHAR_DATA *ch = va_arg(ap, CHAR_DATA *);
	return is_name_strict(ch->class, arg);
}

/*
 * min size
 *
 * arg format: <size> (see size_table[] for valid values)
 * return values: TRUE is ch's size is not greater then <size>
 */
bool
cc_obj_wear_minsize(const char *arg, va_list ap)
{
	CHAR_DATA *ch = va_arg(ap, CHAR_DATA *);

	if (!str_cmp(arg, "all"))
		return TRUE;

	return ch->size <= flag_value(size_table, arg);
}

