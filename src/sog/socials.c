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
 * $Id: socials.c,v 1.8 1999-12-21 00:27:51 avn Exp $
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

	mlstr_init(&soc->found_char);
	mlstr_init(&soc->found_vict);
	mlstr_init(&soc->found_notvict);

	mlstr_init(&soc->noarg_char);
	mlstr_init(&soc->noarg_room);

	mlstr_init(&soc->self_char);
	mlstr_init(&soc->self_room);

	mlstr_init(&soc->notfound_char);
}

void
social_destroy(social_t *soc)
{
	free_string(soc->name);

	mlstr_destroy(&soc->found_char);
	mlstr_destroy(&soc->found_vict);
	mlstr_destroy(&soc->found_notvict);

	mlstr_destroy(&soc->noarg_char);
	mlstr_destroy(&soc->noarg_room);

	mlstr_destroy(&soc->self_char);
	mlstr_destroy(&soc->self_room);

	mlstr_destroy(&soc->notfound_char);
}
