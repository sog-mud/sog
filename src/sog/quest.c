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
 * $Id: quest.c,v 1.1 1999-06-24 20:35:09 fjoe Exp $
 */

#include <stdio.h>

#include "merc.h"
#include "quest.h"

void quest_handle_death(CHAR_DATA *ch, CHAR_DATA *victim)
{
	if (IS_NPC(ch)
	&&  IS_SET(ch->pIndexData->act, ACT_SUMMONED)
	&&  ch->master != NULL)
		ch = ch->master;

	if (victim->hunter)
		if (victim->hunter == ch) {
			act_puts("You have almost completed your QUEST!\n"
				 "Return to questmaster before your time "
				 "runs out!",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
			ch->pcdata->questmob = -1;
		}
		else {
			act_puts("You have completed someone's quest.",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);

			ch = victim->hunter;
			act_puts("Someone has completed you quest.",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
			quest_cancel(ch);
			ch->pcdata->questtime = -number_range(5, 10);
		}
}

void quest_cancel(CHAR_DATA *ch)
{
	CHAR_DATA *fch;

	if (IS_NPC(ch)) {
		bug("quest_cancel: called for NPC", 0);
		return;
	}

	/*
	 * remove mob->hunter
	 */
	for (fch = npc_list; fch; fch = fch->next)
		if (fch->hunter == ch) {
			fch->hunter = NULL;
			break;
		}

	ch->pcdata->questtime = 0;
	ch->pcdata->questgiver = 0;
	ch->pcdata->questmob = 0;
	ch->pcdata->questobj = 0;
	ch->pcdata->questroom = NULL;
}

void quest_update(void)
{
	CHAR_DATA *ch, *ch_next;

	for (ch = char_list; ch && !IS_NPC(ch); ch = ch_next) {
		ch_next = ch->next;

		if (ch->pcdata->questtime < 0) {
			if (++ch->pcdata->questtime == 0) {
				char_puts("{*You may now quest again.\n", ch);
				return;
			}
		} else if (IS_ON_QUEST(ch)) {
			if (--ch->pcdata->questtime == 0) {
				char_puts("You have run out of time for your quest!\n", ch);
				quest_cancel(ch);
				ch->pcdata->questtime = -number_range(5, 10);
			} else if (ch->pcdata->questtime < 6) {
				char_puts("Better hurry, you're almost out of time for your quest!\n", ch);
				return;
			}
		}
	}
}

