/*-
 * Copyright (c) 1998 fjoe <fjoe@iclub.nsu.ru>
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
 * $Id: class.c,v 1.5 1998-10-30 06:56:32 fjoe Exp $
 */

#include <stdio.h>

#include "merc.h"

varr	classes = { sizeof(CLASS_DATA), 4 };

CLASS_DATA *class_new(void)
{
	CLASS_DATA *class;

	class = varr_enew(&classes);
	class->skills.nsize = sizeof(CLASS_SKILL);
	class->skills.nstep = 8;
	class->restrict_sex = -1;
	class->poses.nsize = sizeof(POSE_DATA);
	class->poses.nstep = 4;

	return class;
}

void class_free(CLASS_DATA *class)
{
	varr_free(&class->skills);
	varr_free(&class->poses);
}

/*
 * guild_check - == 0 - the room is not a guild
 *		  > 0 - the room is guild and ch is allowed there
 *		  < 0 - the room is guild and ch is not allowed there
 */
int guild_check(CHAR_DATA *ch, ROOM_INDEX_DATA *room)
{
	int class = -1;
	int iClass, iGuild;

	for (iClass = 0; iClass < classes.nused; iClass++) {
		CLASS_DATA *cl = CLASS(iClass);
		for (iGuild = 0; iGuild < cl->guild->nused; iGuild++) {
		    	if (room->vnum == *(int*) VARR_GET(cl->guild, iGuild)) {
				if (iClass == ch->class)
					return 1;
				class = iClass;
			}
		}
	}

	if (class == -1)
		return 0;

	if (IS_IMMORTAL(ch))
		return 1;

	return -1;
}

const char *class_name(int vnum)
{
	CLASS_DATA *cl;

	if ((cl = class_lookup(vnum)) == NULL)
		return "Mobile";
	return cl->name;
}

const char *class_who_name(int vnum)
{
	CLASS_DATA *cl;

	if ((cl = class_lookup(vnum)) == NULL)
		return "Mob";
	return cl->who_name;
}

/* returns class number */
int cln_lookup(const char *name)
{
	int num;
 
	for (num = 0; num < classes.nused; num++) {
		if (LOWER(name[0]) == LOWER(CLASS(num)->name[0])
		&&  !str_prefix(name, (CLASS(num)->name)))
			return num;
	}
 
	return -1;
}

/* command for retrieving stats */
int get_curr_stat(CHAR_DATA *ch, int stat)
{
	int max;

	if (IS_NPC(ch) || ch->level >= LEVEL_IMMORTAL)
		max = 25;
	else 
		max = UMIN(get_max_train(ch, stat), 25);
  
	return URANGE(3, ch->perm_stat[stat] + ch->mod_stat[stat], max);
}

/* command for returning max training score */
int get_max_train(CHAR_DATA *ch, int stat)
{
	CLASS_DATA *cl;
	RACE_DATA *r;

	if (IS_NPC(ch) || ch->level >= LEVEL_IMMORTAL)
		return 25;

	if ((cl = class_lookup(ch->class)) == NULL
	||  (r = race_lookup(ch->race)) == NULL
	||  !r->pcdata)
		return 0;

/* ORG_RACE && RACE serdar*/
	return UMIN(25, 20 + r->pcdata->stats[stat] + cl->stats[stat]);
}

bool clan_ok(CHAR_DATA *ch, int sn) 
{
	return TRUE;
}

const char *title_lookup(CHAR_DATA *ch)
{
	CLASS_DATA *class;

	if ((class = class_lookup(ch->class)) == NULL
	||  (ch->level < 0 || ch->level > MAX_LEVEL))
		return str_empty;

	return class->titles[ch->level][URANGE(1, ch->sex, 2)-1];
}
