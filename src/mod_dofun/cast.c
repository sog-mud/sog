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
 * $Id: cast.c,v 1.4 1999-09-11 12:49:56 fjoe Exp $
 */

#include <stdio.h>
#include "merc.h"
#include "fight.h"

static int allowed_other(CHAR_DATA *ch, int sn);

void do_cast(CHAR_DATA *ch, const char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	void *vo;
	int mana;
	int sn = -1;
	int door, range;
	bool cast_far = FALSE;
	bool offensive = FALSE;
	bool has_skill = FALSE;
	int slevel;
	int chance = 0;
	skill_t *spell;
	class_t *cl;

	CHAR_DATA *bch;		/* char to check spellbane on */
	int bane_chance;	/* spellbane chance */

	if ((cl = class_lookup(ch->class)) == NULL)
		return;

	if (HAS_SKILL(ch, gsn_spellbane) && !IS_IMMORTAL(ch)) {
		char_puts("You are Battle Rager, not the filthy magician.\n",
			  ch);
		return;
	}

	if (is_affected(ch, gsn_shielding)) {
		char_puts("You reach for the True Source and feel something "
			  "stopping you.\n", ch);
		return;
	}

	if (is_affected(ch, gsn_garble) || is_affected(ch, gsn_deafen)) {
		char_puts("You can't get the right intonations.\n", ch);
		return;
	}

	target_name = one_argument(argument, arg1, sizeof(arg1));
	if (arg1[0] == '\0') {
		char_puts("Cast which what where?\n", ch);
		return;
	}

	if (IS_NPC(ch)) {
		if (!str_cmp(arg1, "nowait")) {
			target_name = one_argument(target_name,
						   arg1, sizeof(arg1));
			if (ch->wait)
				ch->wait = 0;
		}
		else if (ch->wait) 
			return;
		sn = sn_lookup(arg1);
	}
	else {
		pcskill_t *ps;
		ps = (pcskill_t*) skill_vlookup(&PC(ch)->learned, arg1);
		if (ps) {sn = ps->sn; has_skill = TRUE; }
		    else sn = sn_lookup(arg1);
	}

	if ((chance = get_skill(ch, sn)) == 0) {
		char_puts("You don't know any spells of that name.\n", ch);
		return;
	}
	spell = SKILL(sn);
	
	if (IS_VAMPIRE(ch)
	&&  !IS_IMMORTAL(ch)
	&&  !is_affected(ch, gsn_vampire)
	&&  has_skill
	&&  !IS_SET(spell->skill_flags, SKILL_CLAN)) {
		char_puts("You must transform to vampire before casting!\n",
			  ch);
		return;
	}

	if (spell->skill_type != ST_SPELL
	||  spell->fun == NULL) {
		char_puts("That's not a spell.\n", ch);
		return;
	}

	if (ch->position < spell->minimum_position) {
		char_puts("You can't concentrate enough.\n", ch);
		return;
	}

	if (IS_SET(ch->in_room->room_flags, ROOM_NOMAGIC)) {
		char_puts("Your spell fizzles out and fails.\n", ch);
		act("$n's spell fizzles out and fails.",
		    ch, NULL, NULL, TO_ROOM);
		return;
	}

	if (!IS_NPC(ch)) {
		mana = mana_cost(ch, sn);

		if (ch->mana < mana) {
			char_puts("You don't have enough mana.\n", ch);
			return;
		}
	}
	else
		mana = 0;

	/*
	 * Locate targets.
	 */
	victim		= NULL;
	obj		= NULL;
	vo		= NULL;
	bane_chance	= 100;
	bch		= NULL;

	switch (spell->target) {
	default:
		bug("do_cast: %s: bad target %d.", spell->name, spell->target);
		return;

	case TAR_IGNORE:
		bch = ch;
		break;

	case TAR_CHAR_OFFENSIVE:
		if (target_name[0] == '\0') {
			if ((victim = ch->fighting) == NULL) {
				char_puts("Cast the spell on whom?\n", ch);
				return;
			}
		}
		else if ((range = allowed_other(ch, sn)) > 0) {
			if ((victim = get_char_spell(ch, target_name,
						     &door, range)) == NULL) {
				WAIT_STATE(ch, MISSING_TARGET_DELAY);
				return;
			}

			if (victim->in_room != ch->in_room) {
				cast_far = TRUE;
				if (IS_NPC(victim)) {
					if (room_is_private(ch->in_room)) {
						WAIT_STATE(ch, spell->beats);
						char_puts("You can't cast this spell from private room right now.\n", ch);
						return;
					}

					if (IS_SET(victim->pMobIndex->act,
						   ACT_NOTRACK)) {
						WAIT_STATE(ch, spell->beats);
						act_puts("You can't cast this spell to $N at this distance.", ch, NULL, victim, TO_CHAR, POS_DEAD);
						return;
					}	
				}
			}
		}
		else if ((victim = get_char_room(ch, target_name)) == NULL) {
			WAIT_STATE(ch, MISSING_TARGET_DELAY);
			char_puts("They aren't here.\n", ch);
			return;
		}

		vo = victim;
		bch = victim;
		bane_chance = 2*get_skill(bch, gsn_spellbane)/3;
		break;

	case TAR_CHAR_DEFENSIVE:
		if (target_name[0] == '\0')
			victim = ch;
		else if ((victim = get_char_room(ch, target_name)) == NULL) {
			char_puts("They aren't here.\n", ch);
			return;
		}

		vo = victim;
		bch = victim;
		break;

	case TAR_CHAR_SELF:
		if (target_name[0] == '\0')
			victim = ch;
		else if ((victim = get_char_room(ch, target_name)) == NULL
		     ||  (!IS_NPC(ch) && victim != ch)) {
			char_puts("You cannot cast this spell "
				  "on another.\n", ch);
			return;
		}

		vo = victim;
		bch = victim;
		break;

	case TAR_OBJ_INV:
		if (target_name[0] == '\0') {
			char_puts("What should the spell be cast upon?\n",
				  ch);
			return;
		}

		if ((obj = get_obj_carry(ch, target_name)) == NULL) {
			char_puts("You are not carrying that.\n", ch);
			return;
		}

		vo = obj;
		bch = ch;
		break;

	case TAR_OBJ_CHAR_OFF:
		if (target_name[0] == '\0') {
			if ((victim = ch->fighting) == NULL) {
				WAIT_STATE(ch, MISSING_TARGET_DELAY);
				char_puts("Cast the spell on whom or what?\n",
					  ch);
				return;
			}
			vo = victim;
		}
		else if ((victim = get_char_room(ch, target_name)))
			vo = victim;
		else if ((obj = get_obj_here(ch, target_name)))
			vo = obj;
		else {
			WAIT_STATE(ch, spell->beats);
			char_puts("You don't see that here.\n",ch);
			return;
		}
		bch = victim;
		break;

	case TAR_OBJ_CHAR_DEF:
		if (target_name[0] == '\0') {
			victim = ch;
			vo = victim;
		}
		else if ((victim = get_char_room(ch, target_name)))
			vo = victim;
		else if ((obj = get_obj_carry(ch, target_name)))
			vo = obj;
		else {
			char_puts("You don't see that here.\n",ch);
			return;
		}
		bch = victim;
		break;
	}

	if (str_cmp(spell->name, "ventriloquate"))
		say_spell(ch, sn);

	if (mem_is(vo, MT_CHAR)) {
		vo = (void*) victim;

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
				if (is_safe(ch, victim))
					return;

				if (!IS_NPC(ch)
				&&  !IS_NPC(victim)
				&&  victim != ch
				&&  ch->fighting != victim
				&&  victim->fighting != ch
				&&  !is_same_group(ch, victim))
					yell(victim, ch,
					     "Die, $i, you sorcerous dog!");
				break;
			}
		}
	}

	WAIT_STATE(ch, spell->beats);

	if (number_percent() > chance) {
		char_puts("You lost your concentration.\n", ch);
		check_improve(ch, sn, FALSE, 1);
		ch->mana -= mana / 2;
		if (cast_far) cast_far = FALSE;
	}
	else {
		if (IS_SET(cl->class_flags, CLASS_MAGIC))
			slevel = LEVEL(ch) - UMAX(0, (LEVEL(ch) / 20));
		else
			slevel = LEVEL(ch) - UMAX(5, (LEVEL(ch) / 10));

		if ((chance = get_skill(ch, gsn_spell_craft))) {
			if (number_percent() < chance) {
				slevel = LEVEL(ch); 
				check_improve(ch, gsn_spell_craft, TRUE, 1);
			}
			else 
				check_improve(ch, gsn_spell_craft, FALSE, 1);
		}


		if (IS_SET(spell->group, GROUP_MALADICTIONS)
		&&  (chance = get_skill(ch, gsn_improved_maladiction))) {
			if (number_percent() < chance) {
				slevel = LEVEL(ch);
				slevel += chance/20;
				check_improve(ch, gsn_improved_maladiction,
					      TRUE, 1);
			}
			else
				check_improve(ch, gsn_improved_maladiction,
					      FALSE, 1);
		}

		if (IS_SET(spell->group, GROUP_BENEDICTIONS)
		&&  (chance = get_skill(ch, gsn_improved_benediction))) {
			if (number_percent() < chance) {
				slevel = LEVEL(ch);
				slevel += chance/10;
				check_improve(ch, gsn_improved_benediction,
					      TRUE, 1);
			}
			else 
				check_improve(ch, gsn_improved_benediction,
					      FALSE, 1);
		}

		if ((chance = get_skill(ch, gsn_mastering_spell))
		&&  number_percent() < chance) {
			slevel += number_range(1,4); 
			check_improve(ch, gsn_mastering_spell, TRUE, 1);
		}

		if (!IS_NPC(ch) && get_curr_stat(ch, STAT_INT) > 21)
			slevel += get_curr_stat(ch,STAT_INT) - 21;

		if (slevel < 1)
			slevel = 1;

		ch->mana -= mana;
		check_improve(ch, sn, TRUE, 1);
		if (bch && spellbane(bch, ch, bane_chance, 3 * LEVEL(bch)))
			return;
		spell->fun(sn, IS_NPC(ch) ? ch->level : slevel, ch, vo);
		if (victim && IS_EXTRACTED(victim))
			return;
	}
		
	if (cast_far && door != -1)
		path_to_track(ch, victim, door);
	else if (offensive && victim != ch && victim->master != ch) {
		CHAR_DATA *vch;
		CHAR_DATA *vch_next;

		for (vch = ch->in_room->people; vch; vch = vch_next) {
			vch_next = vch->next_in_room;
			if (victim == vch && victim->fighting == NULL) {
				if (victim->position != POS_SLEEPING)
					multi_hit(victim, ch, TYPE_UNDEFINED);
				break;
			}
		}
	}
}

/*-----------------------------------------------------------------------------
 * static functions
 */

/*
 * for casting different rooms 
 * returned value is the range 
 */
static int allowed_other(CHAR_DATA *ch, int sn)
{
	if (IS_SET(SKILL(sn)->skill_flags, SKILL_RANGE))
		return LEVEL(ch) / 20 + 1;
	return 0;
}

