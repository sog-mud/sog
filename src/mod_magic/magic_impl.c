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
 * $Id: magic_impl.c,v 1.4 2001-07-31 18:14:53 fjoe Exp $
 */

#include <stdio.h>

#include <merc.h>

#include <handler.h>

#include <magic.h>
#include "magic_impl.h"

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
		return FALSE;

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
			if (!is_safe(bch, ch) && !is_safe(ch, bch)) {
				damage(bch, ch, bane_damage, "spellbane",
				       DAM_NEGATIVE, DAMF_SHOW);
			}
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
