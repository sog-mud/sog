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
 * $Id: magic.c,v 1.19 2000-01-19 06:51:48 fjoe Exp $
 */

#include <stdio.h>
#include "merc.h"
#include "fight.h"

/*
 * The kludgy global is for spells who want more stuff from command line.
 */
const char *target_name;

/*
 * spellbane - check if 'bch' deflects the spell of 'ch'
 */
bool spellbane(CHAR_DATA *bch, CHAR_DATA *ch, int bane_chance, int bane_damage)
{
	if (IS_IMMORTAL(bch) || IS_IMMORTAL(ch))
		bane_chance = 0;

	if (has_spec(bch, "clan_battleragers")
	&&  number_percent() < bane_chance) {
		if (ch == bch) {
	        	act_puts("Your spellbane deflects the spell!",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
			act("$n's spellbane deflects the spell!",
			    ch, NULL, NULL, TO_ROOM);
			damage(ch, ch, bane_damage, "spellbane",
			       DAM_NEGATIVE, DAMF_SHOW);
		} else {
			check_improve(bch, "spellbane", TRUE, 8);
			act_puts("$N deflects your spell!",
				 ch, NULL, bch, TO_CHAR, POS_DEAD);
			act("You deflect $n's spell!",
			    ch, NULL, bch, TO_VICT);
			act("$N deflects $n's spell!",
			    ch, NULL, bch, TO_NOTVICT);
			if (!is_safe(bch, ch))
				damage(bch, ch, bane_damage, "spellbane",
				       DAM_NEGATIVE, DAMF_SHOW);
	        }
	        return TRUE;
	}

	return FALSE;
}

bool check_trust(CHAR_DATA *ch, CHAR_DATA *victim)
{
	if (ch == victim)
		return TRUE;

	if (IS_NPC(victim))
		return is_same_group(ch, victim);

	if (IS_SET(PC(victim)->trust, TRUST_ALL)) 
		return TRUE;

	return (IS_SET(PC(victim)->trust, TRUST_GROUP) &&
		is_same_group(ch, victim))
	    || (!IS_NULLSTR(ch->clan) && IS_SET(PC(victim)->trust, TRUST_CLAN) &&
		IS_CLAN(ch->clan, victim->clan));
}

/*
 * Cast spells at targets using a magical object.
 */
void obj_cast_spell(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	skill_t *spell;
	CHAR_DATA *bch = NULL;
	CHAR_DATA *victim = NULL;
	int bane_chance = 100;
	int bane_damage = 0;
	bool offensive = FALSE;

	if ((spell = skill_lookup(sn)) == NULL
	||  (spell->skill_type != ST_SPELL &&
	     spell->skill_type != ST_PRAYER)
	||  spell->fun == NULL)
		return;

	switch (spell->target) {
	default:
		bug("obj_cast_spell: %s: bad target %d",
		    gmlstr_mval(&spell->sk_name), spell->target);
		return;

	case TAR_IGNORE:
		vo = NULL;
		bch = ch;
		bane_damage = 10*bch->level;
		break;

	case TAR_CHAR_OFFENSIVE:
		if (vo == NULL)
			vo = ch->fighting;
		/* mem_is handles NULL vo properly (returns FALSE) */
		if (!mem_is(vo, MT_CHAR)) {
			char_puts("You can't do that.\n", ch);
			return;
		}

		bch = vo;
		bane_damage = 10*bch->level;
		bane_chance = 2 * get_skill(vo, "spellbane") / 3;
		break;

	case TAR_CHAR_DEFENSIVE:
	case TAR_CHAR_SELF:
		if (vo == NULL)
			vo = ch;
		/* mem_is handles NULL vo properly (returns FALSE) */
		if (!mem_is(vo, MT_CHAR)) {
			char_puts("You can't do that.\n", ch);
			return;
		}
		bch = vo;
		bane_damage = 10*bch->level;
		break;

	case TAR_OBJ_INV:
		/* mem_is handles NULL vo properly (returns FALSE) */
		if (!mem_is(vo, MT_OBJ)) {
			char_puts("You can't do that.\n", ch);
			return;
		}
		bch = ch;
		bane_damage = 3*bch->level;
		break;

	case TAR_OBJ_CHAR_OFF:
		if (vo == NULL) {
			if (ch->fighting)
				vo = ch->fighting;
			else {
				char_puts("You can't do that.\n", ch);
				return;
			}
		}

		/* mem_is handles NULL vo properly (returns FALSE) */
		if (mem_is(vo, MT_CHAR)) {
			bch = vo;
			bane_damage = 3*bch->level;
		} else if (!mem_is(vo, MT_OBJ)) {
			char_puts("You can't do that.\n", ch);
			return;
		}
		break;

	case TAR_OBJ_CHAR_DEF:
		if (vo == NULL)
			vo = ch;

		/* mem_is handles NULL vo properly (returns FALSE) */
		if (mem_is(vo, MT_CHAR)) {
			bch = vo;
			bane_damage = 3*bch->level;
		} else if (!mem_is(vo, MT_OBJ)) {
			char_puts("You can't do that.\n", ch);
			return;
		}
		break;
	}

	if (mem_is(vo, MT_CHAR)) {
		victim = (CHAR_DATA*) vo;

		switch (spell->target) {
		case TAR_CHAR_DEFENSIVE:
		case TAR_OBJ_CHAR_DEF:
			if (IS_SET(spell->skill_flags, SKILL_QUESTIONABLE)
			&&  !check_trust(ch, victim)) {
				char_puts("They do not trust you enough "
					  "for this spell.\n", ch);
				return;
			}
			break;

		case TAR_CHAR_OFFENSIVE:
		case TAR_OBJ_CHAR_OFF:
			offensive = TRUE;
			if (IS_SET(spell->skill_flags, SKILL_QUESTIONABLE))
				offensive = !check_trust(ch, victim);

			if (offensive) {
				if (is_safe(ch, victim)) {
					char_puts("Something isn't right...\n",
						  ch);
					return;
				}
			}
			break;
		}
	}

	if (bch && spellbane(bch, ch, bane_chance, bane_damage))
		return;

	target_name = str_empty;
	spell->fun(sn, level, ch, vo);

	if (offensive
	&&  !IS_EXTRACTED(victim)
	&&  victim != ch
	&&  victim->master != ch
	&&  victim->in_room == ch->in_room
	&&  victim->position != POS_SLEEPING) {
		yell(victim, ch, "Help! $lu{$i} is attacking me!");
		if (victim->fighting == NULL)
			multi_hit(victim, ch, NULL);
	}
}

/*
 * spellfun_call -- calls spellfun for spell `sn_fun' with
 *		    args (sn, level, ch, vo)
 *		    If `sn' is NULL it will be the same as `sn_fun'
 */
void spellfun_call(const char *sn_fun, const char *sn, int level,
		   CHAR_DATA *ch, void *vo)
{
	skill_t *sk;

	if ((sk = skill_lookup(sn_fun)) == NULL) {
		bug("spellfun_call: %s (name): unknown or reserved spell",
		    sn_fun);
		return;
	}

	if (sn == NULL)
		sn = sn_fun;
	else if (skill_lookup(sn) == NULL) {
		bug("spellfun_call: %s (sn): unknown or reserved spell", sn);
		return;
	}

	if (sk->skill_type != ST_SPELL
	&&  sk->skill_type != ST_PRAYER) {
		bug("spellfun_call: %s: not a spell or prayer",
		    gmlstr_mval(&sk->sk_name));
		return;
	}

	if (sk->fun == NULL) {
		bug("spellfun_call: %s: NULL skill function",
		    gmlstr_mval(&sk->sk_name));
		return;
	}

	sk->fun(sn, level, ch, vo);
}
