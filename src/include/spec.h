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
 * $Id: spec.h,v 1.2 1999-10-21 12:51:54 fjoe Exp $
 */

#ifndef _SPEC_H_
#define _SPEC_H_

/*
 * spec classes
 */
enum {
	SPEC_CLASS,
	SPEC_RACE,
	SPEC_CLAN
};

typedef struct spec_t spec_t;
struct spec_t {
	const char *spec_name;	/* spec name, also used as file name	*/
	flag32_t spec_class;	/* spec class				*/
	varr spec_skills;	/* spec_skill_t				*/
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

void spec_init(spec_t *spec);
spec_t *spec_cpy(spec_t *dst, const spec_t *src);
void spec_destroy(spec_t *spec);

void spec_skill_init(spec_skill_t *spec_sk);

/* fast spec lookup by precise name */
#define spec_lookup(spn)	((spec_t*) strkey_lookup(&specs, (spn)))

void spec_stats		(CHAR_DATA *ch, spec_skill_t *spec_sk);

bool spec_add		(CHAR_DATA *ch, const char *spn);
bool spec_del		(CHAR_DATA *ch, const char *spn);

void update_skills	(CHAR_DATA *ch);
void spec_update	(CHAR_DATA *ch);

bool spec_can_gain	(CHAR_DATA *ch, const char *spn);

#endif