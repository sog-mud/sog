/*-
 * Copyright (c) 1998 fjoe <fjoe@iclub.nsu.ru>
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
 * $Id: race.c,v 1.1 1998-10-30 07:02:24 fjoe Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include "merc.h"

varr races = { sizeof(RACE_DATA), 8 };

RACE_DATA *race_new(void)
{
	return varr_enew(&races);
}

RACE_PCDATA *race_pcdata_new(void)
{
	RACE_PCDATA *pcr;

	pcr = calloc(1, sizeof(*pcr));

	pcr->skills.nsize = sizeof(RACE_SKILL);
	pcr->skills.nstep = 2;
	pcr->classes.nsize = sizeof(RACE_CLASS_DATA);
	pcr->classes.nstep = 4;

	return pcr;
}

void race_free(RACE_DATA *r)
{
	if (r->pcdata)
		race_pcdata_free(r->pcdata);
	free_string(r->name);
	free_string(r->file_name);
}

void race_pcdata_free(RACE_PCDATA *pcr)
{
	int i;

	for (i = 0; i < pcr->classes.nused; i++) {
		RACE_CLASS_DATA *rcl = VARR_GET(&pcr->classes, i);
		free_string(rcl->name);
	}
	varr_free(&pcr->classes);
	varr_free(&pcr->skills);
	free_string(pcr->bonus_skills);
	free(pcr);
}

const char *race_name(int i)
{
	RACE_DATA *r = race_lookup(i);

	if (r == NULL)
		return "unique";
	return r->name;
}

int rn_lookup(const char *name)
{
	int num;
 
	for (num = 0; num < races.nused; num++) {
		RACE_DATA *r = RACE(num);

		if (LOWER(name[0]) == LOWER(r->name[0])
		&&  !str_prefix(name, (r->name)))
			return num;
	}
 
	return 0;
}

