/*-
 * Copyright (c) 2002 SoG Development Team
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
 * 3. Follow all guidelines in the diku license (contained in the file
 *    license.doc)
 * 4. Follow all guidelines in the Merc license (contained in the file
 *    license.txt)
 * 5. Follow all guidelines in the ROM license (contained in the file
 *    rom.license)
 * 6. Follow all guidelines in the Anatolia license (contained in the file
 *    anatolia.license)
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
 * $Id: roulette.c,v 1.1.2.2 2004-06-21 08:54:35 sg Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include "merc.h"
#include "games.h"

void
roulette_update(void)
{
	stake_t *list;
	int i;
	CHAR_DATA *vch;

	if (stake_list == NULL) {
		foreach (vch, char_in_world()) {
			if (IS_NPC(vch))
				continue;
			if (IS_SET(vch->in_room->room_flags, ROOM_CASINO)) {
				act_char("You hear strange whisper, 'Let's "
					 "play roulette!'", vch);
			}
		} end_foreach(vch);
		return;
	}

	i = number_range(0, 32);
	foreach (vch, char_in_world()) {
		if (IS_NPC(vch))
			continue;
		if (IS_SET(vch->in_room->room_flags, ROOM_CASINO)) {
			if (i == 0) {
				act("You glance at the roulette wheel. Zero!",
				    vch, NULL, NULL, TO_CHAR);
			} else {
				act("You glance at the roulette wheel. $j!",
					vch, (const void *) i, NULL, TO_CHAR);
			}
		}
	} end_foreach(vch);

	for (list = stake_list; list; list = list->next) {
		if (list->number == i)
			give_winnings(list->ch, list->stake, list->rating);
	}

	stake_list = NULL;
}

void
give_winnings(CHAR_DATA *ch, int stake, int rating)
{
	CHAR_DATA *croupier;

	if (ch == NULL) {
		printlog("roulette: null ch, skipping.");
		return;
	}

	croupier = croupier_lookup(ch);

	if (croupier == NULL)
		return;

	if (IS_NPC(ch))
		return;

	PC(ch)->bank_g = stake * rating + PC(ch)->bank_g;

	CROUPIER_SAYS(croupier, ch);
	act("    Congratulations, $n! Your winning is transfered into you bank "
	    "account.", ch, NULL, NULL, TO_CHAR);
};

CHAR_DATA *
croupier_lookup(CHAR_DATA *ch)
{
	CHAR_DATA *vch;
	CHAR_DATA *croupier = NULL;

	if (!IS_SET(ch->in_room->room_flags, ROOM_CASINO)) {
		act_char("Try to find casino!", ch);
		return NULL;
	}

	foreach(vch, char_in_room(ch->in_room)) {
		if (!IS_NPC(vch))
			continue;
		if (IS_SET(vch->pMobIndex->act, ACT_CROUPIER)) {
			croupier = vch;
			break;
		}
	} end_foreach (vch);

	if (croupier == NULL) {
		act_char("Try to find good casino.", ch);
		return NULL;
	}

	if (croupier->fighting != NULL) {
		act_char("Wait until the fighting stops.", ch);
		return NULL;
	}

	if (!can_see(croupier, ch)) {
		act_char("You need to be visible!", ch);
		return NULL;
	}

	return croupier;
}

