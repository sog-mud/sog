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
 * $Id: act_spec.c,v 1.7 1999-12-16 12:24:42 fjoe Exp $
 */

#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#if !defined(WIN32)
#	include <unistd.h>
#endif
#include <ctype.h>

#include "merc.h"
#include "db.h"
#include "quest.h"
#include "obj_prog.h"
#include "fight.h"
#include "socials.h"
#include "string_edit.h"
#include "spec.h"

void do_specialize(CHAR_DATA* ch, const char* argument) 
{
	CHAR_DATA *mob;
	bool found = FALSE;
	char *weapon;
	char *output;

	for (mob = ch->in_room->people; mob; mob = mob->next_in_room) {
		if (IS_NPC(mob)  
		&&  MOB_IS(mob, MOB_PRACTICE)
		&&  IS_SET(mob->pMobIndex->practicer, GROUP_WEAPONSMASTER)) {
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
		output = "mace";
	} else if (!str_prefix(argument, "swords")) {
		weapon = "weapon_sword";
		output = "sword";
	} else if (!str_prefix(argument, "axes")) {
		weapon = "weapon_axe";
		output = "axe";
	} else if (!str_prefix(argument, "hands")) {
		weapon = "weapon_hands";
		output = "hand to hand";
	} else if (!str_prefix(argument, "daggers")) {
		weapon = "weapon_dagger";
		output = "dagger";
	} else if (!str_prefix(argument, "whips")
		|| !str_prefix(argument, "flails")) {
		weapon = "weapon_whip";
		output = "whip/flail";
	} else if (!str_prefix(argument, "staves")
		|| !str_prefix(argument, "staff")
		|| !str_prefix(argument, "polearms")
		|| !str_prefix(argument, "spears")) {
		weapon = "weapon_spear";
		output = "spear/staff/polearm";
	} else {
		dofun("help", ch, "'SPECIALIZE'");
		return;
	}

	if (has_spec(ch, weapon)) {
		act_puts("You already specialize in $T.", 
			ch, NULL, output, TO_CHAR, POS_DEAD);
		return;
	}

	if (spec_replace(ch, NULL, weapon) == NULL) {
		act_puts("You specialize in $T.", 
			ch, NULL, output, TO_CHAR, POS_DEAD);
	} else {
		act_puts("You are not ready to specialize in $T yet.",
			 ch, NULL, output, TO_CHAR, POS_DEAD);
	}
}
