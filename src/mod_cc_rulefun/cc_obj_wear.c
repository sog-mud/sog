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
 * $Id: cc_obj_wear.c,v 1.3 1999-11-24 07:22:22 fjoe Exp $
 */

#include <stdio.h>
#include "merc.h"

bool
cc_obj_wear_race(const char *arg, va_list ap)
{
	CHAR_DATA *ch = va_arg(ap, CHAR_DATA *);
	return _is_name(ch->race, arg, str_cmp);
}

bool
cc_obj_wear_class(const char *arg, va_list ap)
{
	CHAR_DATA *ch = va_arg(ap, CHAR_DATA *);
	return _is_name(ch->class, arg, str_cmp);
}

bool
cc_obj_wear_minsize(const char *arg, va_list ap)
{
	CHAR_DATA *ch = va_arg(ap, CHAR_DATA *);

	if (!str_cmp(arg, "all"))
		return TRUE;

	return ch->size <= flag_value(size_table, arg);
}

