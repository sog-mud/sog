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
 * $Id: socials.c,v 1.7 1999-12-18 11:01:41 fjoe Exp $
 */

#include <stdarg.h>
#include <stdio.h>

#include "typedef.h"
#include "varr.h"
#include "socials.h"
#include "str.h"

varr socials;

void
social_init(social_t *soc)
{
	soc->name = str_empty;
	soc->min_pos = 0;

	soc->found_char = str_empty;
	soc->found_vict = str_empty;
	soc->found_notvict = str_empty;

	soc->noarg_char = str_empty;
	soc->noarg_room = str_empty;

	soc->self_char = str_empty;
	soc->self_room = str_empty;

	soc->notfound_char = str_empty;
}

void
social_destroy(social_t *soc)
{
	free_string(soc->name);

	free_string(soc->found_char);
	free_string(soc->found_vict);
	free_string(soc->found_notvict);

	free_string(soc->noarg_char);
	free_string(soc->noarg_room);

	free_string(soc->self_char);
	free_string(soc->self_room);

	free_string(soc->notfound_char);
}
