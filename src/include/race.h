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
 * $Id: race.h,v 1.31 2001-06-20 06:37:38 avn Exp $
 */

#ifndef _RACE_H_
#define _RACE_H_

#include "damtype.h"

#define RACE_NOCH		(A)	/* can't live in common hometowns */
#define RACE_CHANGED		(Z)	/* OLC internal flag */

struct race_t
{
	const char *	name;		/* name of the race		*/
	flag_t		act;		/* act bits			*/
	flag_t		aff;		/* aff bits			*/
	flag_t		has_invis;	/* invisibilities		*/
	flag_t		has_detect;	/* detects			*/
	flag_t		off;		/* off bits			*/
	flag_t		form;		/* default form flag		*/
	flag_t		parts;		/* default body parts		*/
	flag_t		race_flags;	/* race flags			*/
	const char *	damtype;	/* default dam_type for race	*/
	int16_t		resists[MAX_RESIST];
	int		luck_bonus;	/* Luck bonus for the race	*/
					/* initial resistances		*/
	pcrace_t *	race_pcdata;	/* additional data for pc races */
	AFFECT_DATA *	affected;	/* initial affected by		*/
};

/* additional data for pc races */
struct pcrace_t
{
	char	who_name[6];		/* 5-letter who-name		*/
	int	points;			/* cost in exp of the race	*/
	varr	classes;		/* available classes		*/
	const char *	bonus_skills;	/* bonus skills for the race	*/
	const char *	skill_spec;	/* skill spec for this race	*/
	int	mod_stat[MAX_STAT];	/* starting stat mod		*/
	int	max_stat[MAX_STAT];	/* maximum stat			*/
	flag_t	size;			/* aff bits for the race	*/
	int	hp_bonus;		/* initial hp bonus		*/
	int	mana_bonus;		/* initial mana bonus		*/
	int	prac_bonus;		/* initial practice bonus	*/
	int	slang;			/* spoken language		*/
	flag_t	restrict_align;		/* alignment restrictions	*/
	flag_t	restrict_ethos;		/* ethos restrictions		*/
	int	refcnt;
	int	hunger_rate;
};

/* additional data for available classes for race */
struct rclass_t {
	const char *	name;		/* class name */
	int		mult;		/* exp multiplier */
};

extern hash_t races;

#define race_lookup(rn)	((race_t*) strkey_lookup(&races, (rn)))
#define race_search(rn) ((race_t*) strkey_search(&races, (rn)))

#define IS_RACE(r1, r2)		(!str_cmp((r1), (r2)))

void	race_init	(race_t *r);
race_t *race_cpy	(race_t *dst, const race_t *src);
void	race_destroy	(race_t *r);

pcrace_t *	pcrace_new(void);
pcrace_t *	pcrace_dup(pcrace_t*);
void		pcrace_free(pcrace_t*);

rclass_t *rclass_lookup(race_t *r, const char *cn);

void	race_resetstats(CHAR_DATA *ch);

#endif
