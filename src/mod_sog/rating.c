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
 * $Id: rating.c,v 1.9 1998-09-17 15:51:22 fjoe Exp $
 */

#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include "merc.h"
#include "rating.h"

struct rating_data {
	char *name;
	int pc_killed;
};

#define RATING_TABLE_SIZE 20
struct rating_data rating_table[RATING_TABLE_SIZE];

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

	rating_add(ch->name, ++ch->pcdata->pc_killed);
}

/*
 * rating_add - add (or replace) `name/pc_killed' to (in) rating_table
 */
void rating_add(char* name, int pc_killed)
{
	int i;
	struct rating_data *p = rating_table;

	for (i = 0; i < RATING_TABLE_SIZE; i++) {
		if (rating_table[i].name != NULL
		&&  str_cmp(name, rating_table[i].name) == 0) {
			rating_table[i].pc_killed = pc_killed;
			return;
		}
		if (rating_table[i].pc_killed < p->pc_killed)
			p = rating_table + i;
	}

	if (p->pc_killed < pc_killed) {
		if (p->name != NULL)
			free_string(p->name);
		p->name = str_dup(name);
		p->pc_killed = pc_killed;
	} 	
}

static
int
rating_data_cmp(const void *a, const void *b)
{
	return ((struct rating_data*) b)->pc_killed 
		- ((struct rating_data*) a)->pc_killed;
}

void do_rating(CHAR_DATA *ch, const char *argument)
{
	int i;

	qsort(rating_table, RATING_TABLE_SIZE, sizeof(struct rating_data),
	      rating_data_cmp);

	send_to_char("Name                    | PC's killed\n\r", ch);
	send_to_char("------------------------+------------\n\r", ch);
	for (i = 0; i < RATING_TABLE_SIZE; i++) {
		if (rating_table[i].name == NULL)
			continue;
		char_printf(ch, "%-24s| %d\n\r",
			    rating_table[i].name, rating_table[i].pc_killed);
	}
	if (!ch->pcdata->pc_killed)
		send_to_char("\n\rDo you profess to be SO {Cpeaceful{x?\n\r"
			     "You have killed no one.\n\r", ch);
	else {
		if (!strcmp(rating_table[0].name, ch->name))
			send_to_char("\n\rI bet you are {Rawful{x. \n\r"
				     "You're at the top of this list!", ch);
		char_printf(ch, "\n\rYou have killed %s{R%d{x player%s.\n\r",
			    ch->pcdata->pc_killed == 1 ? "ONLY " : "",
			    ch->pcdata->pc_killed, 
			    ch->pcdata->pc_killed == 1 ? "" : "s");
	}
}

