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
 * $Id: skills.h,v 1.22 1999-11-19 09:07:06 fjoe Exp $
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

#define ST_SKILL	0
#define ST_SPELL	1
#define ST_PRAYER	2

/*
 * EVENTs for room affects
 */
#define EVENT_ROOM_ENTER		(A)
#define EVENT_ROOM_LEAVE		(B)
#define EVENT_ROOM_UPDATE		(C)
#define EVENT_ROOM_TIMEOUT		(D)
#define EVENT_CHAR_UPDATE		(E)
#define EVENT_CHAR_UPDFAST		(F)
#define	EVENT_CHAR_TIMEOUT		(G)

typedef struct skill_t skill_t;
struct skill_t {
	const char *	name;			/* skill name */
	const char *	fun_name;		/* skill function name */
	SPELL_FUN *	fun;			/* skill function */
	flag32_t	target;			/* legal target */
	flag32_t	min_pos;		/* position for caster */
	int		slot;			/* slot for #OBJOLD loading */
	int		min_mana;		/* min mana used */
	int		beats;			/* waiting time after use */
	const char *	noun_damage;		/* damage message */
	const char *	msg_off;		/* wear off message */
	const char *	msg_obj;		/* wear off message for obj */
	flag32_t	skill_flags;		/* skill flags */
	const char *	restrict_race;		/* race restrictions */
	flag32_t	group;			/* skill group */
	flag32_t	skill_type;		/* skill type */
	event_fun_t *	eventlist;		/* list of events */
};

struct event_fun_t {
	struct event_fun_t	*next;
	const char *		fun_name;
	EVENT_FUN *		fun;
	flag32_t		event;
};

extern hash_t skills;

#define HAS_SKILL(ch, sn)	(skill_level(ch, sn) < LEVEL_IMMORTAL)
#define IS_SKILL(sn1, sn2)	(!str_cmp((sn1), (sn2)))

void mob_skill_init(void);

void skill_init(skill_t *sk);
skill_t *skill_cpy(skill_t *dst, const skill_t *src);
void skill_destroy(skill_t *sk);

void		check_one_event		(CHAR_DATA *ch, AFFECT_DATA *af,
					flag32_t event);
void		check_events		(CHAR_DATA *ch, AFFECT_DATA *list,
					flag32_t event);

/*
 * misc skill lookup functions
 */

/* fast skill lookup by precise name */
#define skill_lookup(sn)	((skill_t*) strkey_lookup(&skills, (sn)))
#define skill_search(sn)	((skill_t*) strkey_search(&skills, (sn)))

/* lookup skill by prefix in skill list */
void *		skill_vsearch	(varr *v, const char *sn);

const char *	get_weapon_sn	(OBJ_DATA *obj);
int		get_weapon_skill(CHAR_DATA *ch, const char *sn);

int		skill_level	(CHAR_DATA *ch, const char *sn);
int		skill_beats	(const char *sn);
int		skill_mana	(CHAR_DATA *ch, const char *sn);

const char *	skill_slot_lookup(int slot);

typedef struct apply_sa_t {
	skill_t *	sk;
	int		skill;
	int		mod;
} apply_sa_t;

void *apply_sa_cb(void *p, void *d);

int		get_skill	(CHAR_DATA *ch, const char *sn);
void		set_skill	(CHAR_DATA *ch, const char *sn, int value);
void		_set_skill	(CHAR_DATA *ch, const char *sn,
				 int value, bool repl);

void		say_spell	(CHAR_DATA *ch, const char *sn);

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
