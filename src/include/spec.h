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
 * $Id: spec.h,v 1.11 1999-12-20 12:09:52 kostik Exp $
 */

#ifndef _SPEC_H_
#define _SPEC_H_

/*
 * spec classes
 */
enum {
	SPEC_CLASS,
	SPEC_RACE,
	SPEC_CLAN,
	SPEC_WEAPON,
	SPEC_FORM,
	SPEC_MAJORSCHOOL,
	SPEC_MINORSCHOOL
};

#define	SPF_CHANGED	(Z)

typedef struct spec_t spec_t;
struct spec_t {
	const char *spec_name;	/* spec name, also used as file name	*/
	flag_t spec_class;	/* spec class				*/
	varr spec_skills;	/* spec_skill_t				*/
	varr spec_deps;		/* cc_expr_t				*/
	flag_t spec_flags;	/* SPF_CHANGED - for OLC		*/
};

extern hash_t specs;

typedef struct spec_skill_t spec_skill_t;
struct spec_skill_t {
	const char *	sn;		/* skill name			*/
	int 		level;		/* level needed to gain skill	*/
	int 		rating;		/* how hard it is to learn	*/

	int		min;		/* min (initial) skill percents */
	int		adept;		/* adept percents		*/
	int		max;		/* max skill percents		*/
};

void	spec_init(spec_t *spec);
spec_t *spec_cpy(spec_t *dst, const spec_t *src);
void	spec_destroy(spec_t *spec);

spec_skill_t *spec_skill_lookup(spec_t *s, const char *sn);

/* fast spec lookup by precise name */
#define spec_lookup(spn)	((spec_t*) strkey_lookup(&specs, (spn)))
#define spec_search(spn)	((spec_t*) strkey_search(&specs, (spn)))

void spec_stats		(CHAR_DATA *ch, spec_skill_t *spec_sk);

bool has_spec		(CHAR_DATA *ch, const char *spn);
bool spec_add		(CHAR_DATA *ch, const char *spn);
bool spec_del		(CHAR_DATA *ch, const char *spn);

void update_skills	(CHAR_DATA *ch);
void spec_update	(CHAR_DATA *ch);

const char *spec_replace(CHAR_DATA *ch, const char *spn_rm, const char *spn_add);

#endif
