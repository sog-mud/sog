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
 * $Id: magic_impl.c,v 1.16 2002-01-31 19:20:40 tatyana Exp $
 */

#include <stdio.h>

#include <merc.h>

#include <sog.h>

#include <magic.h>
#include "magic_impl.h"

static int allowed_other(CHAR_DATA *ch, skill_t *sk);

/*
 * The kludgy global is for spells who want more stuff from command line.
 */
const char *target_name;

bool
saves_dispel(int dis_level, int spell_level, int duration)
{
	int save;

	/* it's impossible to dispel permanent effects */
	if (duration == -2)
		return 1;

	if (duration == -1)
		spell_level += 5;

	save = 50 + (spell_level - dis_level) * 5;
	save = URANGE(5, save, 95);
	return number_percent() < save;
}

/*
 * spellbane - check if 'bch' deflects the spell of 'ch'
 */
bool
spellbane(CHAR_DATA *bch, CHAR_DATA *ch, int bane_chance, int bane_damage)
{
	if (IS_IMMORTAL(bch) || IS_IMMORTAL(ch))
		return FALSE;

	if (has_spec(bch, "clan_battleragers")
	&&  number_percent() < bane_chance) {
		if (ch == bch) {
			act_puts("Your spellbane deflects the spell!",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
			act("$n's spellbane deflects the spell!",
			    ch, NULL, NULL, TO_ROOM);
			damage(ch, ch, bane_damage, "spellbane",
			       DAM_NEGATIVE, DAM_F_SHOW);
		} else {
			check_improve(bch, "spellbane", TRUE, 8);
			act_puts("$N deflects your spell!",
				 ch, NULL, bch, TO_CHAR, POS_DEAD);
			act("You deflect $n's spell!",
			    ch, NULL, bch, TO_VICT);
			act("$N deflects $n's spell!",
			    ch, NULL, bch, TO_NOTVICT);
			if (!is_safe(bch, ch) && !is_safe(ch, bch)) {
				damage(bch, ch, bane_damage, "spellbane",
				       DAM_NEGATIVE, DAM_F_SHOW);
			}
	        }
	        return TRUE;
	}

	return FALSE;
}

bool
check_trust(CHAR_DATA *ch, CHAR_DATA *victim)
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

void
cpdata_init(cpdata_t *cp)
{
	cp->sn = str_empty;
	cp->sk = NULL;
	cp->chance = 0;
	cp->mana = 0;
	cp->shadow = FALSE;
}

bool
get_cpdata(CHAR_DATA *ch, const char *argument, int skill_type, cpdata_t *cp)
{
	char arg1[MAX_INPUT_LENGTH];

	cpdata_init(cp);

	/*
	 * lookup spell/prayer
	 */
	target_name = one_argument(argument, arg1, sizeof(arg1));
	if (arg1[0] == '\0') {
		if (skill_type == ST_SPELL)
			act_char("Cast which what where?", ch);
		else
			act_char("Pray for what?", ch);
		return FALSE;
	}

	if (skill_type == ST_SPELL
	&&  (cp->chance = get_skill(ch, "shadow magic"))
	&&  !str_cmp(arg1, "shadow")) {
		cp->shadow = TRUE;
		target_name = one_argument(target_name, arg1, sizeof(arg1));
		cp->sk = skill_search(arg1);
		if (cp->sk == NULL
		||  cp->sk->skill_type != ST_SPELL
		||  cp->sk->fun == NULL) {
			act("You have never heard about such a spell.",
				ch, NULL, NULL, TO_CHAR);
			return FALSE;
		}
		cp->sn = gmlstr_mval(&cp->sk->sk_name);
		if (!IS_SET(cp->sk->skill_flags, SKILL_SHADOW)) {
			act("You aren't able to imitate this spell.",
				ch, NULL, NULL, TO_CHAR);
			return FALSE;
		}
	} else {
		pc_skill_t *pc_sk = NULL;

		/* Allow some sugar */
		if (skill_type == ST_PRAYER && !str_cmp(arg1, "for"))
			target_name = one_argument(target_name, arg1,
			    sizeof(arg1));

		if (IS_NPC(ch)) {
			if (!str_cmp(arg1, "nowait")) {
				target_name = one_argument(target_name,
						   arg1, sizeof(arg1));
				if (ch->wait)
					ch->wait = 0;
			} else if (ch->wait)
				return FALSE;
		} else {
			pc_sk = (pc_skill_t *) c_strkey_search(
			    &PC(ch)->learned, arg1);
		}

		if (pc_sk != NULL)
			cp->sk = skill_lookup(pc_sk->sn);
		else
			cp->sk = skill_search(arg1);

		if (cp->sk == NULL
		||  (cp->chance = get_skill(ch, (cp->sn = gmlstr_mval(&cp->sk->sk_name)))) == 0) {
			if (skill_type == ST_SPELL)
				act_char("You don't know any spells of that name.", ch);
			else
				act_char("You don't know any prayers of that name.", ch);
			return FALSE;
		}
	}

	if (cp->sk->skill_type != skill_type
	||  cp->sk->fun == NULL) {
		if (skill_type == ST_SPELL)
			act_char("That's not a spell.", ch);
		else
			act_char("That's not a prayer.", ch);
		return FALSE;
	}

	cp->mana = cp->shadow ? cp->sk->min_mana: skill_mana(ch, cp->sn);

	return TRUE;
}

bool
casting_allowed(CHAR_DATA *ch, cpdata_t *cp)
{
	AFFECT_DATA *paf;
	if (has_spec(ch, "clan_battleragers") && !IS_IMMORTAL(ch)) {
		if (cp->sk->skill_type == ST_SPELL)
			act_char("You are Battle Rager, not a filthy magician!", ch);
		else
			act_char("You are Battle Rager, you prefer not to use prayers.", ch);
		return FALSE;
	}

	if (cp->sk->skill_type == ST_SPELL
	&&  is_sn_affected(ch, "shielding")) {
		act_char("You reach for the True Source and feel something stopping you.", ch);
		return FALSE;
	}

	if ((paf = affect_find(ch->affected, "hold")) != NULL) {
		if (paf->owner == NULL || paf->owner->in_room != ch->in_room)
			affect_strip(ch, "hold");

		if (cp->sk->skill_type == ST_SPELL
		&& cp->sk->rank > 2) {
			act_char(
			    "You cannot make right gestures to cast this spell",
			    ch);
			return FALSE;
		}
	}

	if (is_sn_affected(ch, "garble")
	||  is_sn_affected(ch, "deafen")
	||  (ch->shapeform != NULL &&
	     IS_SET(ch->shapeform->index->flags, FORM_NOCAST))) {
		act_char("You can't get the right intonations.", ch);
		return FALSE;
	}

	if (cp->sk->skill_type == ST_SPELL
	&&  IS_VAMPIRE(ch)
	&&  !IS_IMMORTAL(ch)
	&&  !is_sn_affected(ch, "vampire")
	&&  (IS_NPC(ch) || c_strkey_search(&PC(ch)->learned, cp->sn) != NULL)
	&&  !IS_SET(cp->sk->skill_flags, SKILL_CLAN)) {
		act_char("You must transform to vampire before casting!", ch);
		return FALSE;
	}

	if (ch->position < cp->sk->min_pos) {
		act_char("You can't concentrate enough.", ch);
		return FALSE;
	}

	if (cp->sk->skill_type == ST_SPELL
	&&  IS_SET(ch->in_room->room_flags, ROOM_NOMAGIC)) {
		act_char("Your spell fizzles out and fails.", ch);
		act("$n's spell fizzles out and fails.",
		    ch, NULL, NULL, TO_ROOM);
		return FALSE;
	}

	if (!IS_NPC(ch)
	&&  ch->mana < cp->mana) {
		act_char("You don't have enough mana.", ch);
		return FALSE;
	}

	return TRUE;
}

void
sptarget_init(sptarget_t *spt)
{
	spt->vo	= NULL;

	spt->door = -1;
	spt->cast_far = FALSE;

	spt->bch = NULL;
	spt->bane_chance = 100;
}

bool
find_sptarget(CHAR_DATA *ch, skill_t *sk, sptarget_t *spt)
{
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	int range;

	sptarget_init(spt);

	switch (sk->target) {
	default:
		log(LOG_BUG, "find_sptarget: %s: bad target %d",
		    gmlstr_mval(&sk->sk_name), sk->target);
		return FALSE;

	case TAR_IGNORE:
		spt->bch = ch;
		break;

	case TAR_CHAR_OFFENSIVE:
		if (target_name[0] == '\0') {
			if ((victim = ch->fighting) == NULL) {
				if (sk->skill_type == ST_SPELL)
					act_char("Cast the spell on whom?", ch);
				else
					act_char("Use this prayer on whom?", ch);
				return FALSE;
			}
		} else if ((range = allowed_other(ch, sk)) > 0) {
			if ((victim = get_char_spell(
			     ch, target_name, &spt->door, range)) == NULL) {
				act_char("They aren't here.", ch);
				WAIT_STATE(ch, MISSING_TARGET_DELAY);
				return FALSE;
			}

			if (victim->in_room != ch->in_room) {
				spt->cast_far = TRUE;
				if (IS_NPC(victim)) {
					if (room_is_private(ch->in_room)) {
						WAIT_STATE(ch, sk->beats);
						if (sk->skill_type == ST_SPELL)
							act_char("You can't cast this spell from private room right now.", ch);
						else
							act_char("You can't use this prayer from private room right now.", ch);
						return FALSE;
					}

					if (IS_SET(victim->pMobIndex->act,
						   ACT_NOTRACK)) {
						WAIT_STATE(ch, sk->beats);
						if (sk->skill_type == ST_SPELL)
							act_puts("You can't cast this spell to $N at this distance.", ch, NULL, victim, TO_CHAR, POS_DEAD);
						else
							act_puts("You can't use this prayer to $N at this distance.", ch, NULL, victim, TO_CHAR, POS_DEAD);
						return FALSE;
					}
				}
			}
		} else if ((victim = get_char_here(ch, target_name)) == NULL) {
			act_char("They aren't here.", ch);
			WAIT_STATE(ch, MISSING_TARGET_DELAY);
			return FALSE;
		}

		spt->vo = victim;
		spt->bch = victim;
		spt->bane_chance = 2 * get_skill(spt->bch, "spellbane") / 3;
		break;

	case TAR_CHAR_DEFENSIVE:
		if (target_name[0] == '\0')
			victim = ch;
		else if ((victim = get_char_here(ch, target_name)) == NULL) {
			act_char("They aren't here.", ch);
			return FALSE;
		}

		spt->vo = victim;
		spt->bch = victim;
		break;

	case TAR_CHAR_SELF:
		if (target_name[0] == '\0')
			victim = ch;
		else if ((victim = get_char_here(ch, target_name)) == NULL
		     ||  (!IS_NPC(ch) && victim != ch)) {
			if (sk->skill_type == ST_SPELL)
				act_char("You cannot cast this spell on another.", ch);
			else
				act_char("They should pray their gods themself.", ch);
			return FALSE;
		}

		spt->vo = victim;
		spt->bch = victim;
		break;

	case TAR_OBJ_INV:
		if (target_name[0] == '\0') {
			if (sk->skill_type == ST_SPELL)
				act_char("What should the spell be cast upon?", ch);
			else
				act_char("Use this prayer on what?", ch);
			return FALSE;
		}

		if ((obj = get_obj_carry(ch, ch, target_name)) == NULL) {
			act_char("You are not carrying that.", ch);
			return FALSE;
		}

		spt->vo = obj;
		spt->bch = ch;
		break;

	case TAR_OBJ_CHAR_OFF:
		if (target_name[0] == '\0') {
			if ((victim = ch->fighting) == NULL) {
				WAIT_STATE(ch, MISSING_TARGET_DELAY);
				if (sk->skill_type == ST_SPELL)
					act_char("Cast the spell on whom or what?", ch);
				else
					act_char("Use this prayer on whom or what?", ch);
				return FALSE;
			}
			spt->vo = victim;
		} else if ((victim = get_char_here(ch, target_name)) != NULL)
			spt->vo = victim;
		else if ((obj = get_obj_here(ch, target_name)) != NULL)
			spt->vo = obj;
		else {
			WAIT_STATE(ch, sk->beats);
			act_char("You don't see that here.", ch);
			return FALSE;
		}
		spt->bch = victim;
		break;

	case TAR_OBJ_CHAR_DEF:
		if (target_name[0] == '\0') {
			victim = ch;
			spt->vo = victim;
		} else if ((victim = get_char_here(ch, target_name)) != NULL)
			spt->vo = victim;
		else if ((obj = get_obj_carry(ch, ch, target_name)) != NULL)
			spt->vo = obj;
		else {
			act_char("You don't see that here.", ch);
			return FALSE;
		}
		spt->bch = victim;
		break;
	}

	return TRUE;
}

void
cast_spell(CHAR_DATA *ch, cpdata_t *cp, sptarget_t *spt)
{
	bool offensive = FALSE;
	CHAR_DATA *victim = NULL;
	OBJ_DATA *obj = NULL;
	int cha = 0;

	if (mem_is(spt->vo, MT_CHAR))
		victim = spt->vo;
	else if (mem_is(spt->vo, MT_OBJ))
		obj = spt->vo;
	else if (spt->vo != NULL) {
		log(LOG_BUG, "cast_spell: spt->vo is neither MT_CHAR nor MT_OBJ");
		return;
	}

	if ((obj != NULL || (victim != NULL && victim != ch))
	&&  ch->shapeform != NULL
	&&  IS_SET(ch->shapeform->index->flags, FORM_CASTSELF)) {
		act("You can only affect yourself in this form.",
		    ch, NULL, NULL, TO_CHAR);
		return;
	}

	if (!!str_cmp(cp->sn, "ventriloquate"))
		say_spell(ch, cp->sn);

	if (victim != NULL) {
		switch (cp->sk->target) {
		case TAR_CHAR_DEFENSIVE:
		case TAR_OBJ_CHAR_DEF:
			if (IS_SET(cp->sk->skill_flags, SKILL_QUESTIONABLE)
			&&  !check_trust(ch, victim)) {
				if (cp->sk->skill_type == ST_SPELL)
					act_char("They do not trust you enough for this spell.", ch);
				else
					act_char("They do not trust you enough for this prayer.", ch);
				return;
			}
			break;

		case TAR_CHAR_OFFENSIVE:
		case TAR_OBJ_CHAR_OFF:
			offensive = TRUE;
			if (IS_SET(cp->sk->skill_flags, SKILL_QUESTIONABLE))
				offensive = !check_trust(ch, victim);

			if (offensive) {
				if (is_safe(ch, victim))
					return;

				if (!IS_NPC(ch)
				&&  !IS_NPC(victim)
				&&  victim != ch
				&&  ch->fighting != victim
				&&  victim->fighting != ch
				&&  !is_same_group(ch, victim)) {
					if (cp->sk->skill_type == ST_SPELL) {
						yell(victim, ch,
						     "Die, $N, you sorcerous dog!");
					} else {
						yell(victim, ch,
						     "Help! I've been attacked by $N!");
					}
				}
				break;
			}
		}
	}

	WAIT_STATE(ch, cp->sk->beats);

	if (cp->sk->skill_type == ST_PRAYER) {
		if ((cha = get_curr_stat(ch, STAT_CHA)) < 12) {
			act("Your god doesn't wish to help you anymore.",
				ch, NULL, NULL, TO_CHAR);
			return;
		}

		cha -= cp->sk->rank;
		cp->chance -= (cha > 15)? 0 : (15 - cha) * 3;

		if (cp->sk->rank
		&&  cp->sk->rank > get_curr_stat(ch, STAT_WIS) - 12) {
			act("You aren't wise enough to use such a power.",
				ch, NULL, NULL, TO_CHAR);
			return;
		}
	}

	if (number_percent() > cp->chance) {
		if (cp->sk->skill_type == ST_SPELL) {
			act("You try to cast '$T', but lost your concentration.",
		 	    ch, NULL, cp->sn, TO_CHAR);
		} else {
			act("Your god doesn't hear your pray for '$T'.",
			    ch, NULL, cp->sn, TO_CHAR);
		}
		check_improve(ch, cp->sn, FALSE, 1);
		ch->mana -= cp->mana / 2;
		if (cp->shadow)
			check_improve(ch, "shadow magic", FALSE, 1);
		if (spt->cast_far)
			spt->cast_far = FALSE;
	} else {
		int slevel = LEVEL(ch);
		CHAR_DATA *gch;
		CHAR_DATA *familiar = NULL;
		int chance;

		if (!IS_NPC(ch)) {
			if (cp->sk->skill_type == ST_SPELL) {
				class_t *cl;

				if ((cl = class_lookup(ch->class)) != NULL
				&&  IS_SET(cl->class_flags, CLASS_MAGIC))
					slevel -= UMAX(0, (LEVEL(ch) / 20));
				else
					slevel -= UMAX(5, (LEVEL(ch) / 10));
			} else
				slevel += (cha - 20) / 2;
		}

		if (cp->sk->skill_type == ST_SPELL) {
			if ((chance = get_skill(ch, "spell craft"))) {
				if (number_percent() < chance) {
					slevel = LEVEL(ch);
					check_improve(
					    ch, "spell craft", TRUE, 1);
				} else {
					check_improve(
					    ch, "spell craft", FALSE, 1);
				}
			}

			if ((chance = get_skill(ch, "mastering spell"))
			&&  number_percent() < chance) {
				slevel += number_range(1, 4);
				check_improve(ch, "mastering spell", TRUE, 1);
			}

			for (gch = ch->in_room->people; gch; gch = gch->next_in_room) {
				if (IS_AFFECTED(gch, AFF_CHARM)
				&&  IS_NPC(gch)
				&&  IS_SET(gch->pMobIndex->act, ACT_FAMILIAR)
				&&  gch->master == ch)
					familiar = gch;
			}

			if (familiar != NULL
			&&  number_percent() < 20
			&&  familiar->mana > cp->mana) {
				act("You take some energy of your $N and power of your spell increases.",
				    ch, NULL, familiar, TO_CHAR);
				slevel += number_range(1, 3);
				familiar->mana -= cp->mana/2;
			}

			if (!IS_NPC(ch) && get_curr_stat(ch, STAT_INT) > 21)
				slevel += get_curr_stat(ch,STAT_INT) - 21;

			if (cp->sk->rank
			&&  cp->sk->rank > (get_curr_stat(ch, STAT_INT) - 12)) {
				act_char("You aren't intellegent enough to cast this spell.", ch);
				return;
			}

			if (familiar != NULL
			&&  number_percent() < 20
			&&  familiar->mana > cp->mana) {
				act("You take some energy of your $N to cast a spell.",
				    ch, NULL, familiar, TO_CHAR);
				slevel += number_range(1, 3);
				familiar->mana -= cp->mana/2;
				cp->mana -= cp->mana/2;
			}
		}

		slevel = UMAX(1, slevel);
		ch->mana -= cp->mana;

		if (IS_SET(cp->sk->skill_flags, SKILL_MISSILE)
		&&  victim != NULL
		&&  is_sn_affected(victim, "blur")
		&&  !HAS_DETECT(ch, ID_TRUESEEING)
		&&  (number_percent() < 50)) {
			act("You failed to focus your spell properly.",
				ch, NULL, NULL, TO_CHAR);
			act("$n fails to focus $s spell properly.",
				ch, NULL, NULL, TO_ROOM);
			return;
		}

		if (cp->sk->skill_type == ST_SPELL
		&&  victim != NULL
		&&  victim != ch
		&&  is_sn_affected(victim, "globe of invulnerability")) {
			act("Your spell cannot pass through the sphere "
			    "protecting $n.", ch, victim, NULL, TO_CHAR);
			act("Your globe protects you from $n's spell.",
			    ch, victim, NULL, TO_VICT);
			return;
		}

		check_improve(ch, cp->sn, TRUE, 1);

		if (cp->shadow)
			check_improve(ch, "shadow magic", TRUE, 1);

		if (spt->bch != NULL
		&&  spellbane(spt->bch, ch, spt->bane_chance, 3 * LEVEL(spt->bch)))
			return;

		if (cp->shadow) {
			AFFECT_DATA *paf;

			if (victim == ch) {
				act("You can't do that to yourself.",
					ch, NULL, NULL, TO_CHAR);
				return;
			}

			paf = aff_new(TO_AFFECTS, "shadow magic");
			paf->level	= slevel;
			affect_to_char(ch, paf);

			if (saves_spell(slevel, victim, DAM_MENTAL)) {
				/*
				 * check saves twice
				 */
				if (saves_spell(slevel, victim, DAM_MENTAL)) {
					act_char("Your imitation doesn't seem to have any effect.", ch);
					aff_free(paf);
					return;
				} else {
					affect_to_char(victim, paf);
				}
			}
			aff_free(paf);
		}

		cp->sk->fun(
		     cp->sn, IS_NPC(ch) ? ch->level : slevel, ch, spt->vo);
		if (cp->shadow) {
			if (!IS_EXTRACTED(ch))
				affect_strip(ch, "shadow magic");
		}
		if (victim != NULL && IS_EXTRACTED(victim))
			return;
		if (cp->shadow && is_sn_affected(victim, "shadow magic"))
			affect_strip(victim, "shadow magic");
	}

	if (spt->cast_far && spt->door != -1) {
		path_to_track(ch, victim, spt->door);
		return;
	}

	if (offensive
	&&  !IS_EXTRACTED(victim)
	&&  victim != ch
	&&  victim->master != ch
	&&  victim->fighting == NULL
	&&  victim->position != POS_SLEEPING)
		multi_hit(victim, ch, NULL);
}

/*-----------------------------------------------------------------------------
 * static functions
 */

/*
 * for casting different rooms
 * returned value is the range
 */
static int
allowed_other(CHAR_DATA *ch, skill_t *sk)
{
	if (IS_SET(sk->skill_flags, SKILL_RANGE))
		return LEVEL(ch) / 20 + 1;
	return 0;
}
