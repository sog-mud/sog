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
 * $Id: magic.c,v 1.37 2003-04-25 12:49:25 fjoe Exp $
 */

#include <stdio.h>

#include <merc.h>

#include <sog.h>

#include <magic.h>
#include "magic_impl.h"

static bool get_cpdata_bysn(CHAR_DATA *ch, const char *sn, cpdata_t *cp);

/*
 * Cast spells at targets using a magical object.
 */
void
obj_cast_spell(const char *sn, int level, CHAR_DATA *ch, void *vo)
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
		log(LOG_BUG, "obj_cast_spell: %s: bad target %d",
		    gmlstr_mval(&spell->sk_name), spell->target);
		return;

	case TAR_IGNORE:
		vo = NULL;
		bch = ch;
		bane_damage = 10 * bch->level;
		break;

	case TAR_CHAR_OFFENSIVE:
		if (vo == NULL)
			vo = ch->fighting;
		/* mem_is handles NULL vo properly (returns FALSE) */
		if (!mem_is(vo, MT_CHAR)) {
			act_char("You can't do that.", ch);
			return;
		}

		bch = vo;
		bane_damage = 10 * bch->level;
		bane_chance = 2 * get_skill(vo, "spellbane") / 3;
		break;

	case TAR_CHAR_DEFENSIVE:
	case TAR_CHAR_SELF:
		if (vo == NULL)
			vo = ch;
		/* mem_is handles NULL vo properly (returns FALSE) */
		if (!mem_is(vo, MT_CHAR)) {
			act_char("You can't do that.", ch);
			return;
		}
		bch = vo;
		bane_damage = 10 * bch->level;
		break;

	case TAR_OBJ_INV:
		/* mem_is handles NULL vo properly (returns FALSE) */
		if (!mem_is(vo, MT_OBJ)) {
			act_char("You can't do that.", ch);
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
				act_char("You can't do that.", ch);
				return;
			}
		}

		/* mem_is handles NULL vo properly (returns FALSE) */
		if (mem_is(vo, MT_CHAR)) {
			bch = vo;
			bane_damage = 3 * bch->level;
		} else if (!mem_is(vo, MT_OBJ)) {
			act_char("You can't do that.", ch);
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
			act_char("You can't do that.", ch);
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
				act_char("They do not trust you enough for this spell.", ch);
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
					act_char("Something isn't right...", ch);
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
		yell(victim, ch, "Help! $lu{$N} is attacking me!");
		if (victim->fighting == NULL)
			multi_hit(victim, ch, NULL);
	}
}

void
cast(const char *sn, CHAR_DATA *ch, const char *argument)
{
	cpdata_t cp;
	sptarget_t spt;

	if (!get_cpdata_bysn(ch, sn, &cp))
		return;

	if (!casting_allowed(ch, &cp))
		return;

	sptarget_init(&spt);

	switch (cp.sk->target) {
	default:
		log(LOG_BUG, "cast_mob: %s: bad target %d",
		    sn, cp.sk->target);
		return;

	case TAR_IGNORE:
		spt.bch = ch;
		break;

	case TAR_CHAR_OFFENSIVE:
	case TAR_CHAR_DEFENSIVE:
	case TAR_CHAR_SELF:
	case TAR_OBJ_INV:
	case TAR_OBJ_CHAR_OFF:
	case TAR_OBJ_CHAR_DEF:
		act_char("You can't cast this spell or use prayer on that.", ch);
		return;
	}

	target_name = argument;
	cast_spell(ch, &cp, &spt);
}

void
cast_char(const char *sn, CHAR_DATA *ch, CHAR_DATA *victim)
{
	cpdata_t cp;
	sptarget_t spt;

	if (!get_cpdata_bysn(ch, sn, &cp))
		return;

	if (!casting_allowed(ch, &cp))
		return;

	sptarget_init(&spt);

	switch (cp.sk->target) {
	default:
		log(LOG_BUG, "cast_mob: %s: bad target %d",
		    sn, cp.sk->target);
		return;

	case TAR_IGNORE:
	case TAR_OBJ_INV:
		act_char("You can't cast this spell or use prayer on that.", ch);
		return;

	case TAR_CHAR_OFFENSIVE:
		spt.bane_chance = 2 * get_skill(victim, "spellbane") / 3;
		/* FALLTHRU */

	case TAR_CHAR_DEFENSIVE:
	case TAR_OBJ_CHAR_OFF:
	case TAR_OBJ_CHAR_DEF:
		spt.vo = victim;
		spt.bch = victim;
		break;

	case TAR_CHAR_SELF:
		spt.vo = ch;
		spt.bch = ch;
		break;
	}

	target_name = victim->name;
	cast_spell(ch, &cp, &spt);
}

void
cast_obj(const char *sn, CHAR_DATA *ch, OBJ_DATA *obj)
{
	cpdata_t cp;
	sptarget_t spt;

	if (!get_cpdata_bysn(ch, sn, &cp))
		return;

	if (!casting_allowed(ch, &cp))
		return;

	sptarget_init(&spt);

	switch (cp.sk->target) {
	default:
		log(LOG_BUG, "cast_obj: %s: bad target %d",
		    sn, cp.sk->target);
		return;

	case TAR_IGNORE:
	case TAR_CHAR_OFFENSIVE:
	case TAR_CHAR_DEFENSIVE:
	case TAR_CHAR_SELF:
		act_char("You can't cast this spell or use prayer on that.", ch);
		return;

	case TAR_OBJ_INV:
	case TAR_OBJ_CHAR_OFF:
	case TAR_OBJ_CHAR_DEF:
		spt.vo = obj;
		break;
	}

	target_name = obj->pObjIndex->name;
	cast_spell(ch, &cp, &spt);
}

/*
 * spellfun -- calls spellfun for spell `sn_fun' with
 *		    args (sn, level, ch, vo)
 *		    If `sn' is NULL it will be the same as `sn_fun'
 */
void
spellfun(const char *sn_fun, const char *sn, int level,
	      CHAR_DATA *ch, void *vo)
{
	skill_t *sk;

	if ((sk = skill_lookup(sn_fun)) == NULL) {
		log(LOG_BUG, "spellfun: %s (name): unknown or reserved spell",
		    sn_fun);
		return;
	}

	if (sn == NULL)
		sn = sn_fun;
	else if (skill_lookup(sn) == NULL) {
		log(LOG_BUG, "spellfun: %s (sn): unknown or reserved spell", sn);
		return;
	}

	if (sk->skill_type != ST_SPELL
	&&  sk->skill_type != ST_PRAYER) {
		log(LOG_BUG, "spellfun: %s: not a spell or prayer",
		    gmlstr_mval(&sk->sk_name));
		return;
	}

	if (sk->fun == NULL) {
		log(LOG_BUG, "spellfun: %s: NULL skill function",
		    gmlstr_mval(&sk->sk_name));
		return;
	}

	sk->fun(sn, level, ch, vo);
}

/*
 * Compute a saving throw.
 * Negative apply's make saving throw better.
 */
bool
saves_spell(int level, CHAR_DATA *victim, int dam_type)
{
	class_t *vcl;
	int save;

	save = (LEVEL(victim) - level) * 4 - victim->saving_throw;

	if (IS_NPC(victim))
		save += 40;

	if (IS_AFFECTED(victim, AFF_BERSERK))
		save += victim->level / 5;

	if (dam_type == DAM_MENTAL) {
		save += get_curr_stat(victim, STAT_WIS) - 18;
		save += get_curr_stat(victim, STAT_INT) - 18;
	}

	if (get_resist(victim, dam_type, TRUE) == 100)
		return TRUE;

	save += get_resist(victim, dam_type, TRUE) / 7;

	if (!IS_NPC(victim) && (vcl = class_lookup(victim->class))
	&&  IS_SET(vcl->class_flags, CLASS_MAGIC))
		save = 9 * save / 10;
	save = URANGE(5, save, 95);
	return dice_wlb(1, 100, NULL, victim) < save;
}

/*
 * co-routine for spell_dispel_magic and do_herbs
 */
bool
check_dispel(int dis_level, CHAR_DATA *victim, const char *sn)
{
	AFFECT_DATA *af;

	if (is_sn_affected(victim, sn)) {
	    for (af = victim->affected; af != NULL; af = af->next) {
	        if (IS_SKILL(af->type, sn)) {
	            if (!saves_dispel(dis_level,af->level,af->duration)) {
			skill_t *sk;

	                affect_strip(victim, sn);
			if ((sk = skill_lookup(sn)) != NULL
			&&  !mlstr_null(&sk->msg_off)) {
				act_mlputs(&sk->msg_off, victim, NULL, NULL,
					   TO_CHAR, POS_DEAD);
			}
			return TRUE;
		    } else
			af->level--;
	        }
	    }
	}
	return FALSE;
}

static int
get_cpdata_bysn(CHAR_DATA *ch, const char *sn, cpdata_t *cp)
{
	cpdata_init(cp);

	cp->sn = sn;
	if ((cp->sk = skill_lookup(sn)) == NULL
	||  (cp->chance = get_skill(ch, sn)) == 0) {
		act_char("You don't know any spells or prayers of that name.", ch);
		return FALSE;
	}

	if (cp->sk->skill_type != ST_SPELL && cp->sk->skill_type != ST_PRAYER) {
		act_char("That's not a spell or prayer.", ch);
		return FALSE;
	}

	cp->mana = skill_mana(ch, sn);

	return TRUE;
}
