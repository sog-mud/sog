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
 * $Id: race.c,v 1.5.2.1 1999-11-18 15:35:33 fjoe Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include "merc.h"

varr races = { sizeof(race_t), 8 };

race_t *race_new(void)
{
	return varr_enew(&races);
}

pcrace_t *pcrace_new(void)
{
	pcrace_t *pcr;

	pcr = calloc(1, sizeof(*pcr));

	varr_init(&pcr->skills, sizeof(rskill_t), 2);
	varr_init(&pcr->classes, sizeof(rclass_t), 4);

	return pcr;
}

void race_free(race_t *r)
{
	if (r->race_pcdata)
		pcrace_free(r->race_pcdata);
	free_string(r->name);
	free_string(r->file_name);
}

void pcrace_free(pcrace_t *pcr)
{
	int i;

	for (i = 0; i < pcr->classes.nused; i++) {
		rclass_t *rcl = VARR_GET(&pcr->classes, i);
		free_string(rcl->name);
	}
	varr_destroy(&pcr->classes);
	varr_destroy(&pcr->skills);
	free_string(pcr->bonus_skills);
	free(pcr);
}

const char *race_name(int i)
{
	race_t *r = race_lookup(i);

	if (r == NULL)
		return "unique";
	return r->name;
}

void race_setstats(CHAR_DATA *ch, int rn)
{
	race_t *r;

	if ((r = race_lookup(rn)) == NULL)
		return;

	if (r->race_pcdata != NULL)
		ch->size = r->race_pcdata->size;

	ch->dam_type = attack_lookup("punch");
	ch->affected_by = r->aff;
	ch->imm_flags = r->imm;
	ch->res_flags = r->res;
	ch->vuln_flags = r->vuln;
	ch->form = r->form;
	ch->parts = r->parts;
}

int rn_lookup(const char *name)
{
	int num;
 
	for (num = 0; num < races.nused; num++) {
		race_t *r = RACE(num);

		if (LOWER(name[0]) == LOWER(r->name[0])
		&&  !str_prefix(name, (r->name)))
			return num;
	}
 
	return -1;
}

