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
 * $Id: skills.h,v 1.8 1998-11-11 10:34:02 fjoe Exp $
 */

#ifndef _SKILLS_H_
#define _SKILLS_H_

/*----------------------------------------------------------------------
 * skills stuff
 */

#define SKILL_CLAN	(A)
#define SKILL_RANGE	(B)

struct skill_data {
	const char *	name;			/* skill name */
	SPELL_FUN *	spell_fun;		/* spell function */
	sflag_t		target;			/* legal target */
	sflag_t		minimum_position;	/* position for caster */
	int *		pgsn;			/* pointer to gsn */
	int		slot;			/* slot for #OBJOLD loading */
	int		min_mana;		/* min mana used */
	int		beats;			/* waiting time after use */
	const char *	noun_damage;		/* damage message */
	const char *	msg_off;		/* wear off message */
	const char *	msg_obj;		/* wear off message for obj */
	int		flags;			/* skill flags */
	const char *	restrict_race;		/* race restrictions */
	sflag_t		group;			/* skill group */
};

extern varr skills;

#define HAS_SKILL(ch, sn)	(skill_level(ch, sn) < LEVEL_IMMORTAL)

#define SKILL(sn)		((SKILL_DATA*) VARR_GET(&skills, sn))
#define skill_lookup(sn)	((SKILL_DATA*) varr_get(&skills, sn))

const char *	skill_name	(int sn);
int		sn_lookup	(const char *name);

/* lookup skill by name in skill list */
void *		skill_vlookup	(varr *v, const char *name);

int		get_weapon_sn	(OBJ_DATA *obj);
int		get_weapon_skill(CHAR_DATA *ch, int sn);

int		get_skill	(CHAR_DATA *ch, int sn);
void		set_skill	(CHAR_DATA *ch, int sn, int value);
void		set_skill_raw	(CHAR_DATA *ch, int sn, int value, bool repl);
int		skill_level	(CHAR_DATA *ch, int sn);

void		update_skills	(CHAR_DATA *ch);

int		mana_cost	(CHAR_DATA *ch, int sn);
void		say_spell	(CHAR_DATA *ch, int sn);

int		base_exp	(CHAR_DATA *ch);
int		exp_to_level	(CHAR_DATA *ch);
int		exp_for_level	(CHAR_DATA *ch, int level);
void		check_improve	(CHAR_DATA *ch, int sn, bool success, int mult);
int		group_lookup	(const char *name);

#endif
