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
 * $Id: hometown.c,v 1.1 1999-03-10 17:23:34 fjoe Exp $
 */

#include <stdio.h>

#include "merc.h"

static hometown_t *	get_hometown	(int htn);

varr hometowns = { sizeof(hometown_t), 4 };

/*
 * lookup hometown number by name
 */
int htn_lookup(const char *name)
{
	int i;

	for (i = 0; i < hometowns.nused; i++) {
		hometown_t *h = VARR_GET(&hometowns, i);
		if (!str_prefix(name, h->area))
			return i;
	}

	return -1;
}

/*
 * lookup hometown name by htn
 */
const char* hometown_name(int htn)
{
	return get_hometown(htn)->area;
}

bool hometown_restrict(hometown_t *h, CHAR_DATA *ch)
{
	RACE_DATA *r;
	CLASS_DATA *cl;

	if ((r = race_lookup(ORG_RACE(ch))) == NULL
	||  !r->pcdata
	||  (cl = class_lookup(ch->class)) == NULL)
		return TRUE;

	if (h->restrict_race && !is_name(r->name, h->restrict_race))
		return TRUE;

	if (h->restrict_class && !is_name(cl->name, h->restrict_class))
		return TRUE;

	if (h->restrict_align
	&&  (RALIGN(ch) & h->restrict_align) == 0)
		return TRUE;

	return FALSE;
}

/*
 * recall lookup by ch hometown and align (must always return non NULL)
 */
ROOM_INDEX_DATA *get_recall(CHAR_DATA *ch)
{
	if (!IS_NPC(ch) && ch->pcdata->homepoint)
		return ch->pcdata->homepoint;
	return get_hometown(ch->hometown)->recall[NALIGN(ch)];
}

ROOM_INDEX_DATA *get_random_recall(void)
{
	return get_hometown(number_range(1, hometowns.nused-1))->recall[number_range(0, MAX_ANUM-1)];
}

altar_t *get_altar(CHAR_DATA *ch)
{
	return get_hometown(ch->hometown)->altar + NALIGN(ch);
}

/*
 * just prints the list of availble hometowns
 */
void hometown_print_avail(CHAR_DATA *ch)
{
	int i;
	int col = 0;

	for (i = 0; i < hometowns.nused; i++) {
		hometown_t *h = VARR_GET(&hometowns, i);

		if (hometown_restrict(h, ch))
			continue;

		if (col > 60) {
			char_puts("\n", ch);
			col = 0;
		}

		if (col)
			char_puts(", ", ch);
		char_puts(h->area, ch);
		col += strlen(h->area) + 2;
	}
}

/*----------------------------------------------------------------------------
 * local functions
 */
static hometown_t* get_hometown(int htn)
{
	hometown_t *h = varr_get(&hometowns, htn);
	if (!h)
		h = VARR_GET(&hometowns, 0);
	return h;
}

