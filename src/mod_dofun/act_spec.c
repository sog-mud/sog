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
 * $Id: act_spec.c,v 1.34 2004-02-19 20:55:50 fjoe Exp $
 */

#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#if !defined(WIN32)
#include <unistd.h>
#endif
#include <ctype.h>

#include <merc.h>

#include <sog.h>

DECLARE_DO_FUN(do_read);
DECLARE_DO_FUN(do_specialize);
DECLARE_DO_FUN(do_magicschool);

DO_FUN(do_read, ch, argument)
{
	OBJ_DATA *book;
	int chance;

	if (IS_NPC(ch)) {
		act("You don't know how to read.", ch, NULL, NULL, TO_CHAR);
		return;
	}

	if ((book = get_eq_char(ch, WEAR_HOLD)) == NULL
	||  (book->pObjIndex->item_type != ITEM_BOOK)) {
		act("You do not hold any book in your hands.",
			ch, NULL, NULL, TO_CHAR);
		return;
	}

	if (spec_lookup(STR(book->value[1])) == NULL) {
		act("This $t appears to be blank.",
			ch, flag_string(book_class, INT(book->value[0])),
			NULL, TO_CHAR);
		return;
	}

	if (has_spec(ch, STR(book->value[1]))) {
		act("You page through the $t, but haven't found anything new.",
			ch, flag_string(book_class, INT(book->value[0])),
			NULL, TO_CHAR);
		return;
	}

	if (pull_obj_trigger(TRIG_OBJ_USE, book, ch, NULL) > 0
	||  IS_EXTRACTED(ch) || !mem_is(book, MT_OBJ))
		return;

	chance = INT(book->value[2]);
	chance = (chance * get_curr_stat(ch, STAT_INT)) / 18;

	if (number_percent() > chance
	||  !spec_replace(ch, NULL, STR(book->value[1]))) {
		int eff = INT(book->value[3]);
		act("$p glows with strange red light, then disappears.",
			ch, book, NULL, TO_CHAR);

		extract_obj(book, 0);
		bad_effect(ch, eff);
	} else {
		act(STR(book->value[4]), ch, NULL, NULL, TO_CHAR);
		act("$p disappears suddenly.", ch, book, NULL, TO_CHAR);
		extract_obj(book, 0);
	}
}

DO_FUN(do_specialize, ch, argument)
{
	CHAR_DATA *mob;
	bool found = FALSE;
	const char *weapon;
	const char *output;
	int group = GROUP_WEAPONSMASTER;

	for (mob = ch->in_room->people; mob; mob = mob->next_in_room) {
		if (IS_NPC(mob) && MOB_PRACTICES(mob, group)) {
			found = TRUE;
			break;
		}
	}

	if (!found) {
		act("You can't do that here.", ch, NULL, NULL, TO_CHAR);
		return;
	}

	if (argument[0] == '\0') {
		dofun("help", ch, "'SPECIALIZE'");
		return;
	}

	if (!str_prefix(argument, "maces")) {
		weapon = "weapon_mace";
		output = "mace";				// notrans
	} else if (!str_prefix(argument, "swords")) {
		weapon = "weapon_sword";
		output = "sword";				// notrans
	} else if (!str_prefix(argument, "axes")) {
		weapon = "weapon_axe";
		output = "axe";					// notrans
	} else if (!str_prefix(argument, "hands")) {
		weapon = "weapon_hands";
		output = "hand to hand";			// notrans
	} else if (!str_prefix(argument, "daggers")) {
		weapon = "weapon_dagger";
		output = "dagger";				// notrans
	} else if (!str_prefix(argument, "whips")
		|| !str_prefix(argument, "flails")) {
		weapon = "weapon_whip";
		output = "whip/flail";				// notrans
	} else if (!str_prefix(argument, "staves")
		|| !str_prefix(argument, "staff")
		|| !str_prefix(argument, "polearms")
		|| !str_prefix(argument, "spears")) {
		weapon = "weapon_spear";
		output = "spear/staff/polearm";			// notrans
	} else {
		dofun("help", ch, "'SPECIALIZE'");
		return;
	}

	if (has_spec(ch, weapon)) {
		act_puts("You already specialize in $T.",
			ch, NULL, output, TO_CHAR, POS_DEAD);
		return;
	}

	if (spec_replace(ch, NULL, weapon)) {
		act_puts("You specialize in $T.",
			ch, NULL, output, TO_CHAR, POS_DEAD);
	}
}

DO_FUN(do_magicschool, ch, argument)
{
	const char *major_school = NULL;
	const char *minor_school;
	const char *school_name;

	bool major = FALSE;

	char arg[MAX_INPUT_LENGTH];

	if (argument[0] == '\0') {
		act("Usage: school {{major|minor} <school name>.",
		    ch, NULL, NULL, TO_CHAR);
		return;
	}

	argument = one_argument(argument, arg, sizeof(arg));

	if (argument[0] == '\0') {
		act("Usage: school {{major|minor} <school name>.",
		    ch, NULL, NULL, TO_CHAR);
		return;
	}

	if (!str_prefix(arg, "major"))
		major = TRUE;
	else if (!!str_prefix(arg, "minor")) {
		act("Usage: school {{major|minor} <school name>.",
		    ch, NULL, NULL, TO_CHAR);
		return;
	}

	if (!str_prefix(argument, "abjuration")) {
		major_school	= "major_abjuration";
		minor_school	= "minor_abjuration";
		school_name	= "abjuration";			// notrans
	} else if (!str_prefix(argument, "alteration")
		|| !str_prefix(argument, "transmutation")) {
		major_school	= "major_alteration";
		minor_school	= "minor_alteration";
		school_name	= "alteration";			// notrans
	} else if (!str_prefix(argument, "divination")) {
		minor_school	= "minor_divination";
		if (major) {
			act("You cannot choose divination as major school.",
				ch, NULL, NULL, TO_CHAR);
			return;
		}
		school_name	= "divination";			// notrans
	} else if (!str_prefix(argument, "charm")
		|| !str_prefix(argument, "enchantment")) {
		minor_school	= "minor_charm";
		major_school	= "major_charm";
		school_name	= "enchantment/charm";		// notrans
	} else if (!str_prefix(argument, "conjuration")
		|| !str_prefix(argument, "summonning")) {
		minor_school	= "minor_summonning";
		major_school	= "major_summonning";
		school_name	= "conjuration/summonning";	// notrans
	} else if (!str_prefix(argument, "illusion")) {
		minor_school	= "minor_illusion";
		major_school	= "major_illusion";
		school_name	= "illusion";			// notrans
	} else if (!str_prefix(argument, "necromancy")) {
		minor_school	= "minor_necromancy";
		major_school	= "major_necromancy";
		school_name	= "necromancy";			// notrans
	} else {
		act("No such magic school.", ch, NULL, NULL, TO_CHAR);
		return;
	}

	if (major) {
		const char *repl =
		    has_spec(ch, minor_school) ? minor_school : NULL;
		if (has_spec(ch, major_school)) {
			act("$T is already your major school.",
				ch, NULL, school_name, TO_CHAR);
			return;
		}

		if (spec_replace(ch, repl, major_school)) {
			act("You have chosen $T as your major magic school.",
				ch, NULL, school_name, TO_CHAR);
		}
	} else {
		if (has_spec(ch, major_school)) {
			act("$T is already your major school.",
				ch, NULL, school_name, TO_CHAR);
			return;
		}

		if (has_spec(ch, minor_school)) {
			act("$T is already your minor school.",
				ch, NULL, school_name, TO_CHAR);
			return;
		}

		if (spec_replace(ch, NULL, minor_school)) {
			act("You have chosen $T as your minor magic school.",
				ch, NULL, school_name, TO_CHAR);
		}
	}
}
