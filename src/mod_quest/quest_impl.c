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
 * $Id: quest_impl.c,v 1.5 2001-08-03 11:27:46 fjoe Exp $
 */

#include <stdio.h>

#include <merc.h>

#include <handler.h>
#include <quest.h>
#include "quest_impl.h"

void
quest_update(void)
{
	CHAR_DATA *ch, *ch_next;

	for (ch = char_list; ch && !IS_NPC(ch); ch = ch_next) {
		ch_next = ch->next;

		if (PC(ch)->questtime < 0) {
			if (++PC(ch)->questtime == 0) {
				act_char("{*You may now quest again.", ch);
				return;
			}
		} else if (IS_ON_QUEST(ch)) {
			if (--PC(ch)->questtime == 0) {
				act_char("You have run out of time for your quest!", ch);
				quest_cancel(ch);
				PC(ch)->questtime = -number_range(5, 10);
			} else if (PC(ch)->questtime < 6) {
				act_char("Better hurry, you're almost out of time for your quest!", ch);
				return;
			}
		}
	}
}

qtrouble_t *
qtrouble_lookup(CHAR_DATA *ch, int vnum)
{
	qtrouble_t *qt;

	for (qt = PC(ch)->qtrouble; qt != NULL; qt = qt->next)
		if (qt->vnum == vnum)
			return qt;

	return NULL;
}
