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
 * $Id: skills.c,v 1.131 2001-09-17 18:42:32 fjoe Exp $
 */

#include <stdio.h>
#include <string.h>

#include <merc.h>

#include <sog.h>

#include "skills_impl.h"

/*
 * skill_level -- find min level of the skill for char
 */
int
skill_level(CHAR_DATA *ch, const char *sn)
{
	spec_skill_t spec_sk;

	if (IS_NPC(ch))
		return ch->level;

	spec_sk.sn = sn;
	spec_stats(ch, &spec_sk);
	return spec_sk.level;
}

/*
 * skill_beats -- return skill beats
 */
int
skill_beats(const char *sn)
{
	skill_t *sk;

	if ((sk = skill_lookup(sn)) == NULL) {
#ifdef C_STRKEY_STRICT_CHECKS
		log(LOG_BUG, "skill_beats: %s: unknown skill", sn);
#endif
		return 0;
	}

	return sk->beats;
}
/*
 * skill_mana -- return mana cost based on min_mana and ch->level
 */
int
skill_mana(CHAR_DATA *ch, const char *sn)
{
	skill_t *sk;

	if ((sk = skill_lookup(sn)) == NULL) {
#ifdef C_STRKEY_STRICT_CHECKS
		log(LOG_BUG, "skill_mana: %s: unknown skill", sn);
#endif
		return 0;
	}

	if (IS_NPC(ch))
		return sk->min_mana;

	return UMAX(sk->min_mana, 100 / (2 + UMAX(ch->level - skill_level(ch, sn), 0)));
}

gmlstr_t *
skill_noun(const char *sn)
{
	skill_t *sk;
	STRKEY_CHECK(&skills, sn);
	sk = skill_lookup(sn);
	if (sk != NULL)
		return &sk->noun_damage;
	else
		return glob_lookup("hit");
}

int
get_skill(CHAR_DATA *ch, const char *sn)
{
	int percent;
	skill_t *sk;

	if ((sk = skill_lookup(sn)) == NULL)
		return 0;

	if (ch->shapeform) {
		if (!IS_NULLSTR(ch->shapeform->index->skill_spec)) {
			spec_skill_t *sp_sk;
			spec_t * fsp;
			if (!(fsp=spec_lookup(ch->shapeform->index->skill_spec))) {
				log(LOG_BUG, "get_skill: bad form (%s) spec (%s).\n",
					ch->shapeform->index->name,
					ch->shapeform->index->skill_spec);
				return 0;
			}

			if ((sp_sk = spec_skill_lookup(fsp, sn)) != NULL)
				return sp_sk->adept;
			}
		if (sk->skill_type == ST_SKILL
		&&  !IS_SET(sk->skill_flags, SKILL_FORM))
			return 0;

	}

	if (!IS_NPC(ch)) {
		pc_skill_t *pc_sk;

		if ((IS_SET(sk->skill_flags, SKILL_CLAN)
		&& !clan_item_ok(ch->clan)))
			return 0;

		if ((pc_sk = pc_skill_lookup(ch, sn)) == NULL
		||  skill_level(ch, sn) > ch->level)
			percent = 0;
		else
			percent = pc_sk->percent;
	} else
		percent = get_mob_skill(ch, sk);

	if (ch->daze > 0) {
		if (sk->skill_type == ST_SPELL
		||  sk->skill_type == ST_PRAYER)
			percent /= 2;
		else
			percent = 2 * percent / 3;
	}

	if (!IS_NPC(ch) && PC(ch)->condition[COND_DRUNK]  > 10)
		percent = 9 * percent / 10;

	return UMAX(0, percent + get_skill_mod(ch, sk, percent));
}

/*
 * simply adds sn to ch's known skills (if skill is not already known).
 */
void
_set_skill(CHAR_DATA *ch, const char *sn, int percent, bool replace)
{
	pc_skill_t *pc_sk;

	if (IS_NULLSTR(sn))
		return;

	STRKEY_CHECK(&skills, sn);

	if ((pc_sk = pc_skill_lookup(ch, sn)) != NULL) {
		if (replace || pc_sk->percent < percent)
			pc_sk->percent = percent;
		return;
	}
	pc_sk = varr_enew(&PC(ch)->learned);
	pc_sk->sn = str_dup(sn);
	pc_sk->percent = percent;
	varr_qsort(&PC(ch)->learned, cmpstr);
}

void
set_skill(CHAR_DATA *ch, const char *sn, int percent)
{
	_set_skill(ch, sn, percent, TRUE);
}

/* checks for skill improvement */
void
check_improve(CHAR_DATA *ch, const char *sn, bool success, int multiplier)
{
	pc_skill_t *pc_sk;
	int chance;
	spec_skill_t spec_sk;
	skill_t *sk;

	if (IS_NPC(ch)
	||  (pc_sk = pc_skill_lookup(ch, sn)) == NULL
	||  (sk = skill_lookup(sn)) == NULL
	||  pc_sk->percent <= 0)
		return;

	spec_sk.sn = sn;
	spec_stats(ch, &spec_sk);
	if (spec_sk.level > ch->level
	||  pc_sk->percent >= spec_sk.max)
		return;

	/*
	 * check to see if the character has a chance to learn
	 */
	chance = 10 * int_app[get_curr_stat(ch, STAT_INT)].learn;
	chance /= (multiplier *	spec_sk.rating * 4);
	chance += ch->level;

	if (number_range(1, 1000) > chance)
		return;

/* now that the character has a CHANCE to learn, see if they really have */

	if (success) {
		chance = URANGE(5, spec_sk.max - pc_sk->percent, 95);
		if (number_percent() < chance) {
			pc_sk->percent++;
			gain_exp(ch, dice(spec_sk.rating, ch->level));
			if (pc_sk->percent == spec_sk.max) {
				act_puts("{gYou mastered {W$v{g!{x",
					 ch, &sk->sk_name, NULL,
					 TO_CHAR, POS_DEAD);
			} else {
				act_puts("{gYou have become better at {W$v{g!{x",
					 ch, &sk->sk_name, NULL,
					 TO_CHAR, POS_DEAD);
			}
		}
	} else {
		chance = URANGE(5, pc_sk->percent / 2, 30);
		if (number_percent() < chance) {
			pc_sk->percent += number_range(1, 3);
			pc_sk->percent = UMIN(pc_sk->percent, spec_sk.max);
			gain_exp(ch, dice(spec_sk.rating, ch->level));
			if (pc_sk->percent == spec_sk.max) {
				act_puts("{gYou learn from your mistakes and you manage to master {W$v{g!{x",
					 ch, &sk->sk_name, NULL,
					 TO_CHAR, POS_DEAD);
			} else {
				act_puts("{gYou learn from your mistakes and your {W$v{g skill improves!{x",
					 ch, &sk->sk_name, NULL,
					 TO_CHAR, POS_DEAD);
			}

		}
	}
}

const char *
get_weapon_sn(OBJ_DATA *wield)
{
	if (wield == NULL)
		return "hand to hand";

	if (wield->item_type != ITEM_WEAPON
	||  INT(wield->value[0]) == WEAPON_EXOTIC)
		return NULL;

	return flag_string(weapon_class, INT(wield->value[0]));
}

int
get_weapon_skill(CHAR_DATA *ch, const char *sn)
{
/* sn == NULL for exotic */
	if (sn == NULL)
		return 3 * ch->level;

	return get_skill(ch, sn);
}

void
show_weapon_skill(CHAR_DATA *ch, OBJ_DATA *obj)
{
	int skill = get_weapon_skill(ch, get_weapon_sn(obj));

	if (skill >= 100) {
		act("$p feels like a part of you!",
		    ch, obj, NULL, TO_CHAR);
	} else if (skill > 85) {
		act("You feel quite confident with $p.",
		    ch, obj, NULL, TO_CHAR);
	} else if (skill > 70) {
		act("You are skilled with $p.",
		    ch, obj, NULL, TO_CHAR);
	} else if (skill > 50) {
		act("Your skill with $p is adequate.",
		    ch, obj, NULL, TO_CHAR);
	} else if (skill > 25) {
		act("$p feels a little clumsy in your hands.",
		    ch, obj, NULL, TO_CHAR);
	} else if (skill > 1) {
		act("You fumble and almost drop $p.",
		    ch, obj, NULL, TO_CHAR);
	} else {
		act("You don't even know which end is up on $p.",
		    ch, obj, NULL, TO_CHAR);
	}
}

/*
 * Utter mystical words for an sn.
 */
void
say_spell(CHAR_DATA *ch, const char *sn)
{
	char buf  [MAX_STRING_LENGTH];
	CHAR_DATA *rch;
	const char *pName;
	int iSyl;
	int length;
	int skill;
	skill_t *spell;

	struct syl_type
	{
		const char *	old;
		const char *	new;
	};

	static const struct syl_type syl_table[] =
	{
		{ " ",		" "		},	// notrans
		{ "ar",		"abra"		},	// notrans
		{ "au",		"kada"		},	// notrans
		{ "bless",	"fido"		},	// notrans
		{ "blind",	"nose"		},	// notrans
		{ "bur",	"mosa"		},	// notrans
		{ "cu",		"judi"		},	// notrans
		{ "de",		"oculo"		},	// notrans
		{ "en",		"unso"		},	// notrans
		{ "light",	"dies"		},	// notrans
		{ "lo",		"hi"		},	// notrans
		{ "mor",	"zak"		},	// notrans
		{ "move",	"sido"		},	// notrans
		{ "ness",	"lacri"		},	// notrans
		{ "ning",	"illa"		},	// notrans
		{ "per",	"duda"		},	// notrans
		{ "ra",		"gru"		},	// notrans
		{ "fresh",	"ima"		},	// notrans
		{ "re",		"candus"	},	// notrans
		{ "son",	"sabru"		},	// notrans
		{ "tect",	"infra"		},	// notrans
		{ "tri",	"cula"		},	// notrans
		{ "ven",	"nofo"		},	// notrans
		{ "a", "a" }, { "b", "b" },		// notrans
		{ "c", "q" }, { "d", "e" },		// notrans
		{ "e", "z" }, { "f", "y" },		// notrans
		{ "g", "o" }, { "h", "p" },		// notrans
		{ "i", "u" }, { "j", "y" },		// notrans
		{ "k", "t" }, { "l", "r" },		// notrans
		{ "m", "w" }, { "n", "i" },		// notrans
		{ "o", "a" }, { "p", "s" },		// notrans
		{ "q", "d" }, { "r", "f" },		// notrans
		{ "s", "g" }, { "t", "h" },		// notrans
		{ "u", "j" }, { "v", "z" },		// notrans
		{ "w", "x" }, { "x", "n" },		// notrans
		{ "y", "l" }, { "z", "k" },		// notrans
		{ str_empty, str_empty }
	};

	if ((spell = skill_lookup(sn)) == NULL) {
		log(LOG_BUG, "say_spell: %s: unknown skill", sn);
		return;
	}

	if (spell->skill_type == ST_PRAYER) {
		if (!IS_EVIL(ch)) {
			act("$n raises $s hands to the sky in holy prayer.",
				ch, NULL, NULL, TO_ROOM);
		} else {
			act("$n utters the words of unholy power.",
				ch, NULL, NULL, TO_ROOM);
		}
		return;
	}

	buf[0]	= '\0';
	pName = gmlstr_mval(&spell->sk_name);
	for (; *pName != '\0'; pName += length) {
		for (iSyl = 0; (length = strlen(syl_table[iSyl].old)); iSyl++) {
			if (!str_prefix(syl_table[iSyl].old, pName)) {
				strnzcat(buf, sizeof(buf), syl_table[iSyl].new);
				break;
			}
		}
		if (length == 0)
			length = 1;
	}

	for (rch = ch->in_room->people; rch; rch = rch->next_in_room) {
		if (rch == ch)
			continue;

		skill = get_skill(rch, "spell craft") * 9 / 10;
		if (skill < number_percent()) {
			act("$n utters the words, '$t'.", ch, buf, rch, TO_VICT);
			check_improve(rch, "spell craft", FALSE, 5);
		} else  {
			act("$n utters the words, '$v'.",
			    ch, &spell->sk_name, rch, TO_VICT);
			check_improve(rch, "spell craft", TRUE, 5);
		}
	}
}

/*
 *  routine that checks for matching events and calls event function
 */
void
check_one_event(CHAR_DATA *ch, AFFECT_DATA *paf, flag_t event)
{
	skill_t *sk;
	evf_t *evf;

	if ((sk = skill_lookup(paf->type)) == NULL) {
		log(LOG_BUG, "check_one_event: %s: unknown skill", paf->type);
		return;
	}

	if ((evf = varr_bsearch(&sk->events, &event, cmpint)) != NULL
	&&  evf->fun != NULL)
		evf->fun(ch, paf);
}

void
check_events(CHAR_DATA *ch, AFFECT_DATA *paf, flag_t event)
{
	AFFECT_DATA *paf_next;

	for (; paf != NULL; paf = paf_next) {
		paf_next = paf->next;

		check_one_event(ch, paf, event);
		if (IS_EXTRACTED(ch))
			break;
	}
}
