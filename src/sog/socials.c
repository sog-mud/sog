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
 * $Id: socials.c,v 1.5 1999-12-15 20:12:27 avn Exp $
 */

#include <stdarg.h>
#include <stdio.h>

#include "typedef.h"
#include "varr.h"
#include "socials.h"
#include "str.h"

varr socials = { sizeof(social_t), 8 };

social_t *social_new()
{
	return varr_enew(&socials);
}

void social_free(social_t *soc)
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

social_t *social_lookup(const char *name)
{
	int i;

	for (i = 0; i < socials.nused; i++) {
		social_t *soc = VARR_GET(&socials, i);
		if (!str_cmp(name, soc->name))
			return soc;
	}

	return NULL;
}

social_t *social_search(const char *name)
{
	int i;
	social_t *soc;

	if ((soc = social_lookup(name)))
		return soc;

	for (i = 0; i < socials.nused; i++) {
		soc = VARR_GET(&socials, i);
		if (!str_prefix(name, soc->name))
			return soc;
	}

	return NULL;
}
