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
 * $Id: act_games.c,v 1.1.2.1 2004-06-10 09:20:16 tatyana Exp $
 */

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "const.h"
#include "merc.h"
#include "games.h"

void show_stakes_list(CHAR_DATA *ch);
void stake_list_update(CHAR_DATA *ch, int number, int stake, int rating);

void do_stake(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	int stake = 0;
	int number = 0;
	int rating;
	CHAR_DATA *croupier;

	if (IS_NPC(ch))
		return;

	if ((croupier = croupier_lookup(ch)) == NULL)
		return;

	if (ch->level < 20) {
		act_char("You need at least level 20 to play roulette.", ch);
		return;
	}

	argument = one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		CROUPIER_SAYS(croupier, ch);
		act("    $n, Specify your stake, please.",
		    ch, NULL, NULL, TO_ALL);
		return;
	}

	if (!str_cmp(arg, "list")) {
		show_stakes_list(ch);
		return;
	}

	if (!is_number(argument)) {
		dofun ("help", ch, "ROULETTE");
		return;
	} else {
		number = atoi(argument);
		if (number < 0 || number > 32) {
			CROUPIER_SAYS(croupier, ch);
			act("    Number range: 0 .. 32.",
			    ch, NULL, NULL, TO_ALL);
			return;
		}
	}

	if (is_number(arg)) {
		stake = atoi(arg);
		if (stake <= 0) {
			CROUPIER_SAYS(croupier, ch);
			act("    Is it your joke, $n?", ch, NULL, NULL, TO_ALL);
			return;
		}
		if (stake < 50) {
			CROUPIER_SAYS(croupier, ch);
			act("    Minimum stake is 50 gold coins.",
			    ch, NULL, NULL, TO_ALL);
			return;
		}
		if (ch->gold < stake) {
			CROUPIER_SAYS(croupier, ch);
			act("    You do not have enought money, $n!",
			    ch, NULL, NULL, TO_ALL);
			return;
		}
		ch->gold -= stake;
	}

	CROUPIER_SAYS(croupier, ch);
	if (!(number == 0)) {
		act_puts3("    Stake is $j gold $qj{coins} on $J, $T.",
			  ch, (const void *) stake,
			  (number % 2 == 0) ? "red" : "black",
			  (const void *) number,
			  TO_ALL | ACT_NOTRANS, POS_DEAD);
	} else {
		act("    Stake is $j gold $qj{coins} on zero, thanks.",
		    ch, (const void *) stake, NULL, TO_ALL);
	}

	rating = 32;
	stake_list_update(ch, number, stake, rating);

/*	if (!str_cmp(arg, "red")) {
		rating = 2;
		CROUPIER_SAYS(croupier, ch);
		act("    Stake is $j gold $qj{coins} on red.",
		    ch, (const void *) stake, NULL, TO_ALL);
		return;
	} else if (!str_cmp(arg, "black")) {
		rating = 2;
		CROUPIER_SAYS(croupier, ch);
		act("    Stake is $j gold $qj{coins} on black.",
		    ch, (const void *) stake, NULL, TO_ALL);
		return;
	} else if (!str_cmp(arg, "odd")) {
		rating = 2;
		CROUPIER_SAYS(croupier, ch);
		act("    Stake is $j gold $qj{coins} on odd.",
		    ch, (const void *) stake, NULL, TO_ALL);
		return;
	} else if (!str_cmp(arg, "even")) {
		rating = 2;
		CROUPIER_SAYS(croupier, ch);
		act("    Stake is $j gold $qj{coins} on even.",
		    ch, (const void *) stake, NULL, TO_ALL);
		return;
	}
*/
	return;
}

void
show_stakes_list(CHAR_DATA *ch)
{
	stake_t *list;

	if (stake_list == NULL) {
		act_char("You see no stakes on the table.", ch);
		return;
	}

	act_char("You see next stakes:", ch);
	for (list = stake_list; list; list = list->next) {
		act_puts3("    $N: $j gold on $J",
			  ch, (const void *) list->stake,
			  list->ch,
			  (const void *) list->number,
			  TO_CHAR | ACT_NOTRANS, POS_RESTING);
	}
	return;
}

void
stake_list_update(CHAR_DATA *ch, int number, int stake, int rating)
{
	stake_t *list;

	if (stake_list != NULL) {
		for (list = stake_list; list; list = list->next) {
			if (list->ch == ch && list->number == number) {
				list->stake += stake;
				return;
			}
		}
	}

	list = calloc(1, sizeof(*list));
	list->ch = ch;
	list->number = number;
	list->stake = stake;
	list->rating = rating;
	list->next = stake_list;

	stake_list = list;
}

