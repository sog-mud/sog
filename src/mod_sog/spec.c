/*-
 * Copyright (c) 2001 SoG Development Team
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
 * $Id: spec.c,v 1.35 2003-09-30 00:31:30 fjoe Exp $
 */

#include <stdio.h>

#include <merc.h>

#include <sog.h>

#include "skills_impl.h"

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

/*
 * spec_stats -- find spec stats of the skill for char,
 */
void
spec_stats(CHAR_DATA *ch, spec_skill_t *spec_sk)
{
	race_t *r;
	skill_t *sk;
	const char *bonus_skills;
	const char **pspn;

	spec_sk->level = LEVEL_IMMORTAL;	/* will find min */
	spec_sk->rating = 0;			/* will find min */
	spec_sk->min = 0;			/* will find max */
	spec_sk->adept = 0;			/* will find max */
	spec_sk->max = IS_IMMORTAL(ch) ? 1 : 0;	/* will find max */

/* noone can use ill-defined skills */
	if ((sk = skill_lookup(spec_sk->sn)) == NULL) {
#ifdef C_STRKEY_STRICT_CHECKS
		printlog(LOG_BUG, "spec_stats: %s: unknown skill", spec_sk->sn);
#endif
		goto bailout;
	}

/* check specs */
	if ((r = race_lookup(ch->race)) != NULL
	&&  r->race_pcdata != NULL)
		bonus_skills = r->race_pcdata->bonus_skills;
	else
		bonus_skills = NULL;

	C_FOREACH(pspn, &PC(ch)->specs) {
		spec_t *spec;
		spec_skill_t *spec_sk2;

/* lookup spec */
		spec = spec_lookup(*pspn);
		if (spec == NULL) {
#ifdef C_STRKEY_STRICT_CHECKS
			printlog(LOG_INFO, "spec_stats: %s: unknown spec", *pspn);
#endif
			continue;
		}

/* lookup skill in the given spec */
		spec_sk2 = varr_bsearch(&spec->spec_skills,
					&spec_sk->sn, cmpstr);
		if (spec_sk2 == NULL)
			continue;

/* apply spec skill */
		spec_apply(spec_sk, spec_sk2);

/* apply bonus skills for race */
		if (spec->spec_class == SPEC_CLASS
		&&  is_name(spec_sk->sn, bonus_skills))
			spec_sk->level = 1;
	}

/* check skill affects */
	if (get_skill_mod(ch, sk, 1))
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
 * update_skills stuff
 */

/*
 * use for adding/updating all skills available for `ch'
 */
void
update_skills(CHAR_DATA *ch)
{
	race_t *r;
	const char *bonus_skills;
	const char **pspn;
	pc_skill_t *pc_sk;

/* NPCs do not have skills */
	if (IS_NPC(ch))
		return;

/* check all the specs */
	if ((r = race_lookup(ch->race)) != NULL
	&&  r->race_pcdata != NULL)
		bonus_skills = r->race_pcdata->bonus_skills;
	else
		bonus_skills = NULL;

	C_FOREACH(pspn, &PC(ch)->specs) {
		spec_t *spec = spec_lookup(*pspn);
		spec_skill_t *spec_sk;

		if (spec == NULL) {
#ifdef C_STRKEY_STRICT_CHECKS
			printlog(LOG_INFO, "update_skills: %s: %s: unknown spec",
			    ch->name, *pspn);
#endif
			continue;
		}

		C_FOREACH(spec_sk, &spec->spec_skills) {
			int percent;
			int level;

			if (spec->spec_class == SPEC_CLASS
			&&  is_name(spec_sk->sn, bonus_skills)) {
				percent = spec_sk->max;
				level = 1;
			} else {
				percent = spec_sk->min;
				level = spec_sk->level;
			}

			if (level <= ch->level)
				_set_skill(ch, spec_sk->sn, percent, FALSE);
		}
	}

/* remove not matched skills */
	if (IS_IMMORTAL(ch))
		return;

	C_FOREACH(pc_sk, &PC(ch)->learned) {
		spec_skill_t spec_sk;

		spec_sk.sn = pc_sk->sn;
		spec_stats(ch, &spec_sk);
		pc_sk->percent = UMIN(pc_sk->percent, spec_sk.max);
	}
}

/*-------------------------------------------------------------------
 * adding/deleting and other spec handling
 *
 * the caller should call update_skills him/her(oh, no:)self
 */

#define SU_F_SEEN_RACE	(A)
#define SU_F_SEEN_CLASS	(B)
#define SU_F_SEEN_CLAN	(C)
#define SU_F_ALTERED	(D)

/*
 * spec_update -- update race, class and clan specs for character
 * Expects: !IS_NPC(ch)
 */
void
spec_update(CHAR_DATA *ch)
{
	int flags = 0;
	race_t *r = race_lookup(ch->race);
	class_t *cl = class_lookup(ch->class);
	clan_t *clan = clan_lookup(ch->clan);
	const char **pspn = NULL;

	do {
		VARR_EFOREACH(pspn, pspn, &PC(ch)->specs) {
			const char *new_spn = NULL;
			spec_t *spec = spec_lookup(*pspn);

			if (spec == NULL) {
				printlog(LOG_INFO,
				    "spec_update: %s: %s: unknown spec",
				    ch->name, *pspn);
				continue;
			}

			switch (spec->spec_class) {
			case SPEC_RACE:
				if (!IS_SET(flags, SU_F_SEEN_RACE)
				&&  r != NULL
				&&  r->race_pcdata != NULL) {
					SET_BIT(flags, SU_F_SEEN_RACE);
					if (!str_cmp(r->race_pcdata->skill_spec, *pspn))
						continue;	/* all ok */
					new_spn = r->race_pcdata->skill_spec;
				} else
					new_spn = str_empty;
				break;

			case SPEC_CLASS:
				if (!IS_SET(flags, SU_F_SEEN_CLASS)
				&&  cl != NULL) {
					SET_BIT(flags, SU_F_SEEN_CLASS);
					if (!str_cmp(cl->skill_spec, *pspn))
						continue;	/* all ok */
					new_spn = cl->skill_spec;
				} else
					new_spn = str_empty;
				break;

			case SPEC_CLAN:
				if (!IS_SET(flags, SU_F_SEEN_CLAN)
				&&  clan != NULL) {
					SET_BIT(flags, SU_F_SEEN_CLAN);
					if (!str_cmp(clan->skill_spec, *pspn))
						continue;	/* all ok */
					new_spn = clan->skill_spec;
				} else
					new_spn = str_empty;
				break;
			}

			if (new_spn != NULL) {
				SET_BIT(flags, SU_F_ALTERED);

				if (IS_NULLSTR(new_spn)) {
					varr_edelete(&PC(ch)->specs, pspn);

					/*
					 * restart from here
					 */
					break;
				}

				free_string(*pspn);
				*pspn = str_qdup(new_spn);
			}
		}
	} while (pspn != NULL);

	if (IS_SET(flags, SU_F_ALTERED))
		varr_qsort(&PC(ch)->specs, cmpstr);

	if (!IS_SET(flags, SU_F_SEEN_RACE)
	&&  r != NULL
	&&  r->race_pcdata != NULL
	&&  spec_add(ch, r->race_pcdata->skill_spec))
		SET_BIT(flags, SU_F_ALTERED);

	if (!IS_SET(flags, SU_F_SEEN_CLASS)
	&&  cl != NULL
	&&  spec_add(ch, cl->skill_spec))
		SET_BIT(flags, SU_F_ALTERED);

	if (!IS_SET(flags, SU_F_SEEN_CLAN)
	&&  clan != NULL
	&&  spec_add(ch, clan->skill_spec))
		SET_BIT(flags, SU_F_ALTERED);

	if (IS_SET(flags, SU_F_ALTERED))
		update_skills(ch);
}

/*
 * has_spec -- true, if `ch' has `spn'
 */
bool
has_spec(CHAR_DATA *ch, const char *spn)
{
	if (IS_NPC(ch) || IS_NULLSTR(spn))
		return FALSE;

	STRKEY_CHECK(&specs, spn);

	return varr_bsearch(&PC(ch)->specs, &spn, cmpstr) != NULL;
}

/*
 * spec_add -- add spec `spn' to `ch'
 */
bool
spec_add(CHAR_DATA *ch, const char *spn)
{
	const char **pspn;

	if (IS_NULLSTR(spn))
		return TRUE;

	STRKEY_CHECK(&specs, spn);

	pspn = varr_bsearch(&PC(ch)->specs, &spn, cmpstr);
	if (pspn != NULL)
		return FALSE;

	pspn = varr_enew(&PC(ch)->specs);
	*pspn = str_dup(spn);
	varr_qsort(&PC(ch)->specs, cmpstr);
	return TRUE;
}

bool
spec_del(CHAR_DATA *ch, const char *spn)
{
	const char **pspn;

	if (IS_NULLSTR(spn))
		return TRUE;

	STRKEY_CHECK(&specs, spn);

	pspn = varr_bsearch(&PC(ch)->specs, &spn, cmpstr);
	if (pspn == NULL)
		return FALSE;

	varr_edelete(&PC(ch)->specs, pspn);
	return TRUE;
}

bool
spec_replace(CHAR_DATA *ch, const char *spn_rm, const char *spn_add)
{
	spec_t *spec;
	const char **pspn;

	if (IS_NPC(ch))
		return FALSE;

	if ((spec = spec_lookup(spn_add)) != NULL
	&&  pull_spec_trigger(spec, ch, spn_rm, spn_add) > 0)
		return FALSE;

	C_FOREACH(pspn, &PC(ch)->specs) {
		if (!str_cmp(*pspn, spn_rm))
			continue;

		if ((spec = spec_lookup(*pspn)) != NULL
		&&  pull_spec_trigger(spec, ch, spn_rm, spn_add) > 0)
			return FALSE;
	}

	if (!spec_del(ch, spn_rm))
		return FALSE;

	if (!spec_add(ch, spn_add)) {
		spec_add(ch, spn_rm);
		return FALSE;
	}

	return TRUE;
}
