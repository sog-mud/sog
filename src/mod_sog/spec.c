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
 * $Id: spec.c,v 1.1 1999-10-06 09:56:11 fjoe Exp $
 */

#include <stdio.h>
#include "merc.h"

/*
 *	Simple Simon met a pieman
 *	Going to the fair
 *
 *	Says Simple Simon to the pieman:
 *	'Let me taste your ware'
 *
 *	Says the pieman to Simple Simon:
 *	'Show me first your penny'
 *
 *	Says Simple Simon to the pieman:
 *	'Indeed I have not any'
 *
 */

/*-------------------------------------------------------------------
 * funcs for hashing
 *
 * spec->spec_skills is not initialized/copied/destroyed
 * this behavior is intentional
 */

hash_t specs;

void
spec_init(spec_t *spec)
{
	spec->spec_name = str_empty;
	spec->spec_class = 0;
}

spec_t *
spec_cpy(spec_t *dst, const spec_t *src)
{
	dst->spec_name = str_qdup(src->spec_name);
	dst->spec_class = src->spec_class;
	return dst;
}

void
spec_destroy(spec_t *spec)
{
	free_string(spec->spec_name);
}

void spec_skill_init(spec_skill_t *spec_sk)
{
	spec_sk->sn = str_empty;
	spec_sk->level = 1;
	spec_sk->rating = 1;
	spec_sk->min = 1;
	spec_sk->adept = 75;
	spec_sk->max = 100;
}

/*-------------------------------------------------------------------
 * update_skills stuff
 */

typedef struct _update_skills_t _update_skills_t;
struct _update_skills_t {
	CHAR_DATA *ch;
	const char *bonus_skills;
	const char *matched_skills;
	spec_t *spec;
};

static void *
add_one_skill_cb(void *p, void *d)
{
	spec_skill_t *spec_sk = (spec_skill_t *) p;
	_update_skills_t *u = (_update_skills_t *) d;

	int percent;
	int level;

	if (u->spec->spec_class == SPEC_CLASS
	&&  is_name(spec_sk->sn, u->bonus_skills)) {
		percent = spec_sk->max;
		level = 1;
	} else {
		percent = spec_sk->min;
		level = spec_sk->level;
	}

	if (level <= u->ch->level) {
		set_skill_raw(u->ch, spec_sk->sn, percent, FALSE);
		name_add(&u->matched_skills, spec_sk->sn, NULL, NULL);
	}
	return NULL;
}

static void *
add_skills_cb(void *p, void *d)
{
	const char *spn = *(const char **) p;
	_update_skills_t *u = (_update_skills_t *) d;

	spec_t *spec = spec_lookup(spn);
	if (spec == NULL) {
#ifdef NAME_STRICT_CHECKS
		wizlog("update_skills: %s: %s: unknown spec", u->ch->name, spn);
#endif
		return NULL;
	}

	u->spec = spec;
	varr_foreach(&spec->spec_skills, add_one_skill_cb, u);
	return NULL;
}

static void *
check_one_skill_cb(void *p, void *d)
{
	pc_skill_t *pc_sk = (pc_skill_t *) p;
	_update_skills_t *u = (_update_skills_t *) d;

	if (!is_name(pc_sk->sn, u->matched_skills))
		pc_sk->percent = 0;
	return NULL;
}

/*
 * use for adding/updating all skills available for `ch'
 */
void update_skills(CHAR_DATA *ch)
{
	race_t *r;
	_update_skills_t u;

/* NPCs do not have skills */
	if (IS_NPC(ch))
		return;

/* check all the specs */
	if ((r = race_lookup(ch->race)) != NULL
	&&  r->race_pcdata != NULL)
		u.bonus_skills = r->race_pcdata->bonus_skills;
	else
		u.bonus_skills = NULL;
	u.ch = ch;
	u.matched_skills = NULL;
	varr_foreach(&PC(ch)->specs, add_skills_cb, &u);

/* remove not matched skills */
	if (!IS_IMMORTAL(ch)) 
		varr_foreach(&PC(ch)->learned, check_one_skill_cb, &u);
	free_string(u.matched_skills);
}

/*-------------------------------------------------------------------
 * spec_stats stuff
 */

static void
spec_apply(spec_skill_t *spec_sk, spec_skill_t *spec_sk2)
{
	spec_sk->level = UMIN(spec_sk->level, spec_sk2->level);
	if (spec_sk->rating == 0)
		spec_sk->rating = spec_sk2->rating;
	else
		spec_sk->rating = UMIN(spec_sk->rating,
					  spec_sk2->rating);
	spec_sk->min = UMAX(spec_sk->min, spec_sk2->min);
	spec_sk->adept = UMAX(spec_sk->adept, spec_sk2->adept);
	spec_sk->max = UMAX(spec_sk->max, spec_sk2->max);
}

typedef struct _spec_stats_t _spec_stats_t;
struct _spec_stats_t
{
	const char	*bonus_skills;
	spec_skill_t	*spec_sk;
};

static void *
spec_stats_cb(void *p, void *d)
{
	const char *spn = *(const char **) p;
	_spec_stats_t *stats = (_spec_stats_t *) d;

	spec_t *spec;
	spec_skill_t *spec_sk;

/* lookup spec */
	spec = spec_lookup(spn);
	if (spec == NULL) {
#ifdef NAME_STRICT_CHECKS
		wizlog("spec_stats: %s: unknown spec", spn);
#endif
		return NULL;
	}

/* lookup skill in the given spec */
	spec_sk = varr_bsearch(&spec->spec_skills,
			       &stats->spec_sk->sn, cmpstr);
	if (spec_sk == NULL)
		return NULL;

/* apply spec skill */
	spec_apply(stats->spec_sk, spec_sk);

/* apply bonus skills for race */
	if (spec->spec_class == SPEC_CLASS
	&&  is_name(stats->spec_sk->sn, stats->bonus_skills))
		stats->spec_sk->level = 1;

	return NULL;
}

/*
 * spec_stats -- find spec stats of the skill for char,
 */
void spec_stats(CHAR_DATA *ch, spec_skill_t *spec_sk)
{
	race_t *r;
	skill_t *sk;
	AFFECT_DATA *paf;
	_spec_stats_t stats;

	spec_sk->level = LEVEL_IMMORTAL;	/* will find min */
	spec_sk->rating = 0;			/* will find min */
	spec_sk->min = 0;			/* will find max */
	spec_sk->adept = 0;			/* will find max */
	spec_sk->max = 0;			/* will find max */

/* noone can use ill-defined skills */
	if ((sk = skill_lookup(spec_sk->sn)) == NULL) {
#ifdef NAME_STRICT_CHECKS
		bug("spec_stats: %s: unknown skill", spec_sk->sn);
#endif
		goto bailout;
	}

/* check specs */
	if ((r = race_lookup(ch->race)) != NULL
	&&  r->race_pcdata != NULL)
		stats.bonus_skills = r->race_pcdata->bonus_skills;
	else
		stats.bonus_skills = NULL;
	stats.spec_sk = spec_sk;
	varr_foreach(&PC(ch)->specs, spec_stats_cb, &stats);

/* check skill affects */
	for (paf = ch->affected; paf; paf = paf->next)
		if (paf->where == TO_SKILLS
		&&  SKILL_IS(paf->location.s, spec_sk->sn))
			spec_sk->level = 1;

bailout:
	/*
	 * sanity checks
	 */
	spec_sk->level = URANGE(1, spec_sk->level, MAX_LEVEL);
	spec_sk->rating = UMAX(1, spec_sk->rating);
	spec_sk->max = UMAX(0, spec_sk->max);
	spec_sk->min = URANGE(0, spec_sk->min, spec_sk->max);
	spec_sk->adept = URANGE(spec_sk->min, spec_sk->adept,
				   spec_sk->max);
}

/*-------------------------------------------------------------------
 * adding/deleting and other spec handling
 *
 * the caller should call update_skills him/her(oh, no:)self
 */

/*
 * spec_add -- add spec `spn' to `ch'
 */
bool spec_add(CHAR_DATA *ch, const char *spn)
{
	const char **pspn;

	if (IS_NULLSTR(spn))
		return FALSE;

	NAME_CHECK(&specs, spn, "spec_add");

	pspn = varr_bsearch(&PC(ch)->specs, &spn, cmpstr);
	if (pspn)
		return FALSE;

	pspn = varr_enew(&PC(ch)->specs);
	*pspn = str_dup(spn);
	varr_qsort(&PC(ch)->specs, cmpstr);
	return TRUE;
}

bool spec_del(CHAR_DATA *ch, const char *spn)
{
	const char **pspn;

	if (IS_NULLSTR(spn))
		return FALSE;

	NAME_CHECK(&specs, spn, "spec_add");

	pspn = varr_bsearch(&PC(ch)->specs, &spn, cmpstr);
	if (pspn == NULL)
		return FALSE;

	varr_delete(&PC(ch)->specs, varr_index(&PC(ch)->specs, pspn));
	return TRUE;
}

#define SU_F_SEEN_RACE	(A)
#define SU_F_SEEN_CLASS	(B)
#define SU_F_SEEN_CLAN	(C)
#define SU_F_ALTERED	(D)

typedef struct _spec_update_t _spec_update_t;
struct _spec_update_t {
	CHAR_DATA *ch;
	race_t *r;
	class_t *cl;
	clan_t *clan;
	int flags;
};

static void *
spec_update_cb(void *p, void *d)
{
	const char **pspn = (const char **) p;
	_spec_update_t *u = (_spec_update_t *) d;

	const char *new_spn = NULL;
	spec_t *spec = spec_lookup(*pspn);

	if (spec == NULL) {
		wizlog("spec_update: %s: %s: unknown spec", u->ch, *pspn);
		return NULL;
	}

	switch (spec->spec_class) {
	case SPEC_RACE:
		if (!IS_SET(u->flags, SU_F_SEEN_RACE)
		&&  u->r != NULL
		&&  u->r->race_pcdata != NULL) {
			SET_BIT(u->flags, SU_F_SEEN_RACE);
			if (!str_cmp(u->r->race_pcdata->skill_spec, *pspn))
				return NULL;			/* all ok */
			new_spn = u->r->race_pcdata->skill_spec;
		} else 
			new_spn = str_empty;
		break;

	case SPEC_CLASS:
		if (!IS_SET(u->flags, SU_F_SEEN_CLASS)
		&&  u->cl != NULL) {
			SET_BIT(u->flags, SU_F_SEEN_CLASS);
			if (!str_cmp(u->cl->skill_spec, *pspn))
				return NULL;			/* all ok */
			new_spn = u->cl->skill_spec;
		} else
			new_spn = str_empty;
		break;
	case SPEC_CLAN:
		if (!IS_SET(u->flags, SU_F_SEEN_CLAN)
		&&  u->clan != NULL) {
			SET_BIT(u->flags, SU_F_SEEN_CLAN);
			if (!str_cmp(u->clan->skill_spec, *pspn))
				return NULL;			/* all ok */
			new_spn = u->clan->skill_spec;
		} else
			new_spn = str_empty;
		break;
	}

	if (new_spn != NULL) {
		free_string(*pspn);
		*pspn = str_qdup(new_spn);
		SET_BIT(u->flags, SU_F_ALTERED);
	}
	return NULL;
}

void spec_update(CHAR_DATA *ch)
{
	_spec_update_t u;

	u.ch = ch;
	u.r = race_lookup(ch->race);
	u.cl = class_lookup(ch->class);
	if (ch->clan)
		u.clan = clan_lookup(ch->clan);
	else
		u.clan = NULL;
	u.flags = 0;
	varr_foreach(&PC(ch)->specs, spec_update_cb, &u);

	if (IS_SET(u.flags, SU_F_ALTERED))
		varr_qsort(&PC(ch)->specs, cmpstr);

	if (!IS_SET(u.flags, SU_F_SEEN_RACE)
	&&  u.r != NULL
	&&  u.r->race_pcdata != NULL
	&&  spec_add(ch, u.r->race_pcdata->skill_spec))
		SET_BIT(u.flags, SU_F_ALTERED);

	if (!IS_SET(u.flags, SU_F_SEEN_CLASS)
	&&  u.cl != NULL
	&&  spec_add(ch, u.cl->skill_spec))
		SET_BIT(u.flags, SU_F_ALTERED);

	if (!IS_SET(u.flags, SU_F_SEEN_CLAN)
	&&  u.clan != NULL
	&&  spec_add(ch, u.clan->skill_spec))
		SET_BIT(u.flags, SU_F_ALTERED);

	if (IS_SET(u.flags, SU_F_ALTERED))
		update_skills(ch);
}

bool spec_can_gain(CHAR_DATA *ch, const char *spn)
{
	return FALSE;
}
