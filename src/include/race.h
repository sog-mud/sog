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
 * $Id: race.h,v 1.10 1999-07-21 04:19:18 avn Exp $
 */

#ifndef _RACE_H_
#define _RACE_H_

#define RACE_NOCH		(A)	/* can't live in common hometowns */
#define RACE_CHANGED		(Z)	/* OLC internal flag */

struct race_t
{
	const char *	name;		/* call name of the race	*/
	const char *	file_name;	/* filename			*/
	flag64_t	act;		/* act bits			*/
	flag64_t	aff;		/* aff bits			*/
	flag32_t	off;		/* off bits			*/
	flag32_t	imm;		/* imm bits			*/
	flag32_t	res;		/* res bits			*/
	flag32_t	vuln;		/* vuln bits			*/
	flag32_t	form;		/* default form flag		*/
	flag32_t	parts;		/* default body parts		*/
	flag32_t	race_flags;	/* race flags			*/
	pcrace_t *	pcdata;		/* additional data for pc races */
};

/* additional data for pc races */
struct pcrace_t
{
	char 	who_name[6];		/* 5-letter who-name		*/
	int 	points; 		/* cost in exp of the race	*/
	varr	classes;		/* available classes		*/
	const char *	bonus_skills;	/* bonus skills for the race	*/
	varr 	skills;			/* race skills			*/
	int	stats[MAX_STATS];	/* starting stats		*/
	int	max_stats[MAX_STATS];	/* maximum stats		*/
	flag32_t size;			/* aff bits for the race	*/
	int 	hp_bonus;		/* initial hp bonus		*/
	int 	mana_bonus;		/* initial mana bonus		*/
	int 	prac_bonus;		/* initial practice bonus	*/
	int	slang;			/* spoken language		*/
	flag32_t	restrict_align;	/* alignment restrictions	*/
	flag32_t	restrict_ethos;	/* ethos restrictions		*/
};

/* additional data for available classes for race */
struct rclass_t {
	const char *	name;		/* class name */
	int		mult;		/* exp multiplier */
};

struct rskill_t {
	int	sn;
	int	level;
};

extern varr races;

#define RACE(i)		((race_t*) VARR_GET(&races, i))
#define race_lookup(i)	((race_t*) varr_get(&races, i))
#define rskill_lookup(race, sn) \
	((rskill_t*) varr_bsearch(&race->pcdata->skills, &sn, cmpint))
#define rclass_lookup(race, name) \
	((rclass_t*) varr_bsearch(&race->pcdata->classes, &name, cmpstr))

race_t *	race_new(void);
pcrace_t *	pcrace_new(void);
void		race_free(race_t*);
void		pcrace_free(pcrace_t*);

const char *	race_name(int);
int		rn_lookup(const char*);

#endif
