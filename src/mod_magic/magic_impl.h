/*-
 * Copyright (c) 1999, 2000 SoG Development Team
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
 * $Id: magic_impl.h,v 1.3 2001-09-01 19:08:28 fjoe Exp $
 */

#ifndef _MAGIC_IMPL_H_
#define _MAGIC_IMPL_H_

extern const char *target_name;

/**
 * cast/pray data
 */
struct cpdata_t {
	const char *sn;
	skill_t *sk;
	int chance;
	int mana;
	bool shadow;
};
typedef struct cpdata_t cpdata_t;

/**
 * Spell/prayer target
 */
struct sptarget_t {
	void *vo;

	int door;
	bool cast_far;

	CHAR_DATA *bch;
	int bane_chance;
};
typedef struct sptarget_t sptarget_t;

bool saves_dispel(int dis_level, int spell_level, int duration);
bool check_trust(CHAR_DATA *ch, CHAR_DATA *victim);
bool spellbane(CHAR_DATA *bch, CHAR_DATA *ch, int bane_chance, int bane_damage);

void cpdata_init(cpdata_t *cp);
int get_cpdata(CHAR_DATA *ch, const char *argument,
	       int skill_type, cpdata_t *cp);

void sptarget_init(sptarget_t *spt);
int find_sptarget(CHAR_DATA *ch, skill_t *sk, sptarget_t *spt);

void cast_spell(CHAR_DATA *ch, cpdata_t *cp, sptarget_t *spt);

#endif /* _MAGIC_IMPL_H_ */
