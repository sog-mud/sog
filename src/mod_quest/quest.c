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
 * $Id: quest.c,v 1.7 2000-02-20 10:36:41 avn Exp $
 */

#include <stdio.h>

#include "merc.h"
#include "quest.h"

/*
 * assumes IS_NPC(victim)
 */
void quest_handle_death(CHAR_DATA *ch, CHAR_DATA *victim)
{
	CHAR_DATA *hunter;

	if (IS_NPC(ch)
	&&  IS_SET(ch->pMobIndex->act, ACT_SUMMONED)
	&&  ch->master != NULL)
		ch = ch->master;

	if ((hunter = NPC(victim)->hunter) != NULL) {
		if (hunter == ch) {
			act_puts("You have almost completed your QUEST!\n"
				 "Return to questmaster before your time "
				 "runs out!",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
			PC(ch)->questmob = -1;
		}
		else {
			act_puts("You have completed someone's quest.",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);

			act_puts("Someone has completed your quest.",
				 hunter, NULL, NULL, TO_CHAR, POS_DEAD);
			quest_cancel(hunter);
			PC(ch)->questtime = -number_range(5, 10);
		}
	}
}

void quest_cancel(CHAR_DATA *ch)
{
	CHAR_DATA *fch;

	if (IS_NPC(ch)) {
		log(LOG_ERROR, "quest_cancel: called for NPC");
		return;
	}

	/*
	 * remove NPC(mob)->hunter
	 */
	for (fch = npc_list; fch; fch = fch->next) {
		NPC_DATA *npc = NPC(fch);
		if (npc->hunter == ch) {
			npc->hunter = NULL;
			break;
		}
	}

	PC(ch)->questtime = 0;
	PC(ch)->questgiver = 0;
	PC(ch)->questmob = 0;
	PC(ch)->questobj = 0;
	PC(ch)->questroom = NULL;
}

