/*-
 * Copyright (c) 1998 efdi <efdi@iclub.nsu.ru>
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
 * $Id: rating.c,v 1.16 1999-06-28 09:04:18 fjoe Exp $
 */

#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "merc.h"
#include "rating.h"

struct rating_t rating_table[RATING_TABLE_SIZE];

static int rating_cmp(const void *a, const void *b);

/*
 * Updates player's rating.
 * Should be called every death.
 */
void rating_update(CHAR_DATA *ch, CHAR_DATA *victim)
{
	if (IS_NPC(ch) || IS_NPC(victim) || ch == victim || IS_IMMORTAL(ch)
	||  (victim->in_room &&
	     IS_SET(victim->in_room->room_flags, ROOM_BATTLE_ARENA)))
		return;

	++ch->pcdata->pc_killed;
	rating_add(ch);
}

/*
 * rating_add - add (or replace) `name/pc_killed' to (in) rating_table
 */
void rating_add(CHAR_DATA *ch)
{
	int i;
	rating_t *p = rating_table;

	/*
	 * find the minimal entry in rating_table
	 */
	for (i = 0; i < RATING_TABLE_SIZE; i++) {
		/*
		 * if there is already entry for this player
		 */
		if (!IS_NULLSTR(rating_table[i].name)
		&&  !str_cmp(ch->name, rating_table[i].name)) {
			p = rating_table + i;
			if (p->pc_killed < ch->pcdata->pc_killed) {
				p->pc_killed = ch->pcdata->pc_killed;
				qsort(rating_table, RATING_TABLE_SIZE,
				      sizeof(rating_t), rating_cmp);
			}
			return;
		}

		if (rating_table[i].pc_killed < p->pc_killed)
			p = rating_table + i;
	}

	if (p->pc_killed < ch->pcdata->pc_killed) {
		free_string(p->name);
		p->name = str_qdup(ch->name);
		p->pc_killed = ch->pcdata->pc_killed;
		qsort(rating_table, RATING_TABLE_SIZE, sizeof(rating_t),
		      rating_cmp);
	} 	
}

static int rating_cmp(const void *a, const void *b)
{
	return ((rating_t*) b)->pc_killed - ((rating_t*) a)->pc_killed;
}

