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
 * $Id: skills.h,v 1.35 2000-01-05 12:01:02 kostik Exp $
 */

#ifndef _SKILLS_H_
#define _SKILLS_H_

/*----------------------------------------------------------------------
 * skills stuff
 */

#define SKILL_CLAN		(A)
#define SKILL_RANGE		(B)
#define SKILL_AREA_ATTACK	(C)
#define SKILL_QUESTIONABLE	(D)
#define SKILL_FORM		(E)
#define SKILL_MISSILE		(F)

#define ST_SKILL	0
#define ST_SPELL	1
#define ST_PRAYER	2

/*
 * EVENTs for room affects
 */
enum {
	EVENT_ROOM_ENTER,
	EVENT_ROOM_LEAVE,
	EVENT_ROOM_UPDATE,
	EVENT_ROOM_TIMEOUT,
	EVENT_CHAR_UPDATE,
	EVENT_CHAR_UPDFAST,
	EVENT_CHAR_TIMEOUT,
};

typedef struct skill_t skill_t;
struct skill_t {
	gmlstr_t	sk_name;		/* skill name */
	const char *	fun_name;		/* skill function name */
	SPELL_FUN *	fun;			/* skill function */
	flag_t		target;			/* legal target */
	flag_t		min_pos;		/* position for caster */
	int		slot;			/* slot for #OBJOLD loading */
	int		min_mana;		/* min mana used */
	int		beats;			/* waiting time after use */
	gmlstr_t	noun_damage;		/* damage message */
	mlstring	msg_off;		/* wear off message */
	mlstring 	msg_obj;		/* wear off message for obj */
	flag_t		skill_flags;		/* skill flags */
	const char *	restrict_race;		/* race restrictions */
	flag_t		group;			/* skill group */
	flag_t		skill_type;		/* skill type */
	varr		events;			/* evf_t, sorted by event */
};

typedef struct evf_t {
	flag_t			event;
	const char *		fun_name;
	EVENT_FUN *		fun;
} evf_t;

extern hash_t skills;

#define IS_SKILL(sn1, sn2)	(!str_cmp((sn1), (sn2)))

void mob_skill_init(void);

void skill_init(skill_t *sk);
skill_t *skill_cpy(skill_t *dst, const skill_t *src);
void skill_destroy(skill_t *sk);

void	check_one_event	(CHAR_DATA *ch, AFFECT_DATA *af,
			 flag_t event);
void	check_events	(CHAR_DATA *ch, AFFECT_DATA *list,
			 flag_t event);

/*
 * misc skill lookup functions
 */

/* fast skill lookup by precise name */
#define skill_lookup(sn)	((skill_t*) strkey_lookup(&skills, (sn)))
#define skill_search(sn)	((skill_t*) mlstrkey_search(&skills, (sn)))

const char *	get_weapon_sn	(OBJ_DATA *obj);
int		get_weapon_skill(CHAR_DATA *ch, const char *sn);

int		skill_level	(CHAR_DATA *ch, const char *sn);
int		skill_beats	(const char *sn);
int		skill_mana	(CHAR_DATA *ch, const char *sn);
gmlstr_t *	skill_noun	(const char *sn);

void		skills_dump	(BUFFER *output, int skill_type);

const char *	skill_slot_lookup(int slot);

int		get_skill_mod	(CHAR_DATA *ch, skill_t *sk, int percent);
int		get_skill	(CHAR_DATA *ch, const char *sn);
void		set_skill	(CHAR_DATA *ch, const char *sn, int value);
void		_set_skill	(CHAR_DATA *ch, const char *sn,
				 int value, bool repl);

void		say_spell	(CHAR_DATA *ch, const skill_t *spell);

/*
 * exp_to_level assumes !IS_NPC(ch)
 */
int		base_exp	(CHAR_DATA *ch);
int		exp_to_level	(CHAR_DATA *ch);
int		exp_for_level	(CHAR_DATA *ch, int level);
void		check_improve	(CHAR_DATA *ch, const char *sn,
				 bool success, int mult);
int		group_lookup	(const char *name);

#endif
