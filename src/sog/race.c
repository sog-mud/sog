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
 * $Id: race.c,v 1.16 1999-12-11 15:31:18 fjoe Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include "merc.h"

hash_t races;

void
race_init(race_t *r)
{
	int i;
	r->name = str_empty;
	r->act = 0;
	r->aff = 0;
	r->has_invis = 0;
	r->has_detect = 0;
	r->off = 0;
	r->form = 0;
	r->parts = 0;
	r->race_flags = 0;
	r->damtype = str_dup("punch");
	for (i = 0; i < MAX_RESIST; i++)
		r->resists[i] = 0;
	r->race_pcdata = NULL;
	r->affected = NULL;
}

/*
 * r->race_pcdata and r->affected are not copied intentionally
 */
race_t *
race_cpy(race_t *dst, race_t *src)
{
	int i;
	dst->name = str_qdup(src->name);
	dst->act = src->act;
	dst->aff = src->aff;
	dst->has_invis = src->has_invis;
	dst->has_detect = src->has_detect;
	dst->off = src->off;
	dst->form = src->form;
	dst->parts = src->parts;
	dst->race_flags = src->race_flags;
	dst->damtype = str_dup(src->damtype);
	for (i = 0; i < MAX_RESIST; i++)
		dst->resists[i] = src->resists[i];
	return dst;
}

void
race_destroy(race_t *r)
{
	free_string(r->name);
	free_string(r->damtype);
	aff_free_list(r->affected);
	if (r->race_pcdata)
		pcrace_free(r->race_pcdata);
}

pcrace_t *
pcrace_new(void)
{
	pcrace_t *pcr;
	pcr = calloc(1, sizeof(*pcr));
	pcr->skill_spec = str_empty;
	varr_init(&pcr->classes, sizeof(rclass_t), 4);
	pcr->classes.e_init = strkey_init;
	pcr->classes.e_destroy = strkey_destroy;
	return pcr;
}

void
pcrace_free(pcrace_t *pcr)
{
	varr_destroy(&pcr->classes);
	free_string(pcr->skill_spec);
	free_string(pcr->bonus_skills);
	free(pcr);
}

rclass_t *
rclass_lookup(race_t *r, const char *cn)
{
	return (rclass_t*) varr_bsearch(&r->race_pcdata->classes, &cn, cmpstr);
}

void race_resetstats(CHAR_DATA *ch)
{
	race_t *r;
	int i;

	if ((r = race_lookup(ch->race)) == NULL)
		return;

	if (r->race_pcdata != NULL)
		ch->size = r->race_pcdata->size;

	ch->damtype = str_dup(r->damtype);
	ch->affected_by = r->aff;
	ch->has_invis = r->has_invis;
	ch->has_detect = r->has_detect;
	ch->form = r->form;
	ch->parts = r->parts;

	for (i=0; i < MAX_RESIST; i++)
		ch->resists[i] = r->resists[i];
}

