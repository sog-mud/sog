/*-
 * Copyright (c) 1998 SoG Development Team
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
 * $Id: race.c,v 1.37 2003-06-18 07:58:58 fjoe Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include <merc.h>

avltree_t races;

static void
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
	r->luck_bonus = 0;
	r->damtype = str_dup("+punch");
	for (i = 0; i < MAX_RESIST; i++)
		r->resists[i] = RES_UNDEF;
	r->race_pcdata = NULL;
	r->affected = NULL;
}

static void
race_destroy(race_t *r)
{
	free_string(r->name);
	free_string(r->damtype);
	aff_free_list(r->affected);
	if (r->race_pcdata)
		pcrace_free(r->race_pcdata);
}

avltree_info_t c_info_races =
{
	&avltree_ops,

	(e_init_t) race_init,
	(e_destroy_t) race_destroy,

	MT_PVOID, sizeof(race_t), ke_cmp_str
};

static varr_info_t c_info_rclasses =
{
	&varr_ops,

	strkey_init,
	strkey_destroy,

	sizeof(rclass_t), 4
};

pcrace_t *
pcrace_new(void)
{
	pcrace_t *pcr;
	pcr = calloc(1, sizeof(*pcr));
	pcr->skill_spec = str_empty;
	pcr->hunger_rate = 100;
	pcr->moves_rate	= 100;
	c_init(&pcr->classes, &c_info_rclasses);
	pcr->refcnt = 1;
	return pcr;
}

pcrace_t *
pcrace_dup(pcrace_t *pcr)
{
	if (pcr == NULL)
		return NULL;

	pcr->refcnt++;
	return pcr;
}

void
pcrace_free(pcrace_t *pcr)
{
	if (--pcr->refcnt > 0)
		return;
	c_destroy(&pcr->classes);
	free_string(pcr->skill_spec);
	free_string(pcr->bonus_skills);
	free(pcr);
}

rclass_t *
rclass_lookup(race_t *r, const char *cn)
{
	return (rclass_t*) varr_bsearch(&r->race_pcdata->classes, &cn, cmpstr);
}

void
race_resetstats(CHAR_DATA *ch)
{
	race_t *r;

	if ((r = race_lookup(ch->race)) == NULL)
		return;

	if (r->race_pcdata != NULL)
		ch->size = r->race_pcdata->size;

	free_string(ch->damtype);
	ch->damtype = str_dup(r->damtype);
	ch->affected_by = r->aff;
	ch->has_invis = r->has_invis;
	ch->has_detect = r->has_detect;
	ch->form = r->form;
	ch->parts = r->parts;
	ch->luck = r->luck_bonus;
}

/* return current stats */
int
get_curr_stat(CHAR_DATA *ch, int stat)
{
	int max;

	if (IS_NPC(ch) || ch->level >= LEVEL_IMMORTAL)
		max = 25;
	else
		max = get_max_train(ch, stat);

	if (!ch->shapeform)
		return URANGE(3, ch->perm_stat[stat] + ch->mod_stat[stat], max);
	else
		return URANGE(3, ch->shapeform->index->stats[stat], 25);
}

/* return max stats */
int
get_max_train(CHAR_DATA *ch, int stat)
{
	race_t *r;

	if (IS_NPC(ch) || ch->level >= LEVEL_IMMORTAL)
		return 25;

	if ((r = race_lookup(PC(ch)->race)) == NULL
	||  !r->race_pcdata)
		return 0;

	return UMIN(25, r->race_pcdata->max_stat[stat]);
}
