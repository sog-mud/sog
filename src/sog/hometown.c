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
 * $Id: hometown.c,v 1.25 2003-09-29 23:11:54 fjoe Exp $
 */

#include <stdio.h>

#include <merc.h>

static hometown_t *	get_hometown	(int htn);

varr hometowns;

varr_info_t c_info_hometowns = {
	&varr_ops, NULL, NULL,

	sizeof(hometown_t), 4
};

/*
 * lookup hometown number by name
 */
int
htn_lookup(const char *name)
{
	hometown_t *h;

	C_FOREACH(h, &hometowns) {
		if (!str_prefix(name, h->area))
			return varr_index(&hometowns, h);
	}

	return -1;
}

/*
 * lookup hometown name by htn
 */
const char *
hometown_name(int htn)
{
	return get_hometown(htn)->area;
}

bool
hometown_restrict(hometown_t *h, CHAR_DATA *ch)
{
	race_t *r;
	class_t *cl;

	if ((r = race_lookup(ORG_RACE(ch))) == NULL
	||  !r->race_pcdata
	||  (cl = class_lookup(ch->class)) == NULL)
		return TRUE;

	if (IS_SET(r->race_flags, RACE_NOCH))
		return !is_name(r->name, h->restrict_race);

	if (IS_SET(cl->class_flags, CLASS_NOCH))
		return !is_name(cl->name, h->restrict_class);

	if (h->restrict_race
	&&  !is_name(r->name, h->restrict_race))
		return TRUE;

	if (h->restrict_class
	&&  !is_name(cl->name, h->restrict_class))
		return TRUE;

	if (h->restrict_align
	&&  (RALIGN(ch) & h->restrict_align) == 0)
		return TRUE;

	return FALSE;
}

int
hometown_permanent(CHAR_DATA *ch)
{
	int htn_perm = -1;
	hometown_t *h;

	C_FOREACH(h, &hometowns) {
		if (hometown_restrict(h, ch))
			continue;

		if (htn_perm < 0)
			htn_perm = varr_index(&hometowns, h);
		else
			return -1;	/* more than one hometown */
	}

	return htn_perm;
}

/*
 * recall lookup ch hometown and align
 * must always return non NULL, assumes !IS_NPC(ch)
 */
ROOM_INDEX_DATA *
get_recall(CHAR_DATA *ch)
{
	if (PC(ch)->homepoint)
		return PC(ch)->homepoint;
	return get_hometown(PC(ch)->hometown)->recall[NALIGN(ch)];
}

/*
 * map of hometown by hometown and align lookup, assumes !IS_NPC(ch)
 */
OBJ_INDEX_DATA *
get_map(CHAR_DATA *ch)
{
	return get_hometown(PC(ch)->hometown)->map[NALIGN(ch)];
}

ROOM_INDEX_DATA *
get_random_recall(void)
{
	int i = number_range(0, MAX_AN-1);
	return ((hometown_t *) c_random_elem(&hometowns))->recall[i];
}

/*
 * altar by hometown and align lookup, assumes !IS_NPC(ch)
 */
altar_t *
get_altar(CHAR_DATA *ch)
{
	return get_hometown(PC(ch)->hometown)->altar + NALIGN(ch);
}

/*----------------------------------------------------------------------------
 * local functions
 */
static hometown_t *
get_hometown(int htn)
{
	hometown_t *h;

	if (htn < 0)
		htn = 0;
	h = varr_get(&hometowns, htn);
	if (!h)
		h = VARR_GET(&hometowns, 0);
	return h;
}
