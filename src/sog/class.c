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
 * $Id: class.c,v 1.17 1999-10-06 09:56:04 fjoe Exp $
 */

#include <stdio.h>

#include "merc.h"

varr	classes = { sizeof(class_t), 4 };

class_t *class_new(void)
{
	class_t *class;

	class = varr_enew(&classes);
	varr_init(&class->poses, sizeof(pose_t), 4);
	varr_init(&class->guilds, sizeof(int), 4);
	class->skill_spec = str_empty;
	class->hp_rate = 100;
	class->mana_rate = 100;
	class->restrict_sex = -1;
	class->death_limit = -1;

	return class;
}

void class_free(class_t *class)
{
	free_string(class->skill_spec);
	varr_destroy(&class->poses);
	varr_destroy(&class->guilds);
}

/*
 * guild_ok - check if ch allowed in the room (if the room is guild)
 */
int guild_ok(CHAR_DATA *ch, ROOM_INDEX_DATA *room)
{
	int class = -1;
	int iClass, iGuild;

	if (!IS_SET(room->room_flags, ROOM_GUILD)
	||  IS_IMMORTAL(ch))
		return TRUE;

	for (iClass = 0; iClass < classes.nused; iClass++) {
		class_t *cl = CLASS(iClass);
		for (iGuild = 0; iGuild < cl->guilds.nused; iGuild++) {
		    	if (room->vnum == *(int*)VARR_GET(&cl->guilds, iGuild)) {
				if (iClass == ch->class)
					return TRUE;
				class = iClass;
			}
		}
	}

	if (class < 0) {
		/*
		 * room was not found in the list of guild rooms
		 * of all classes
		 */
		log("guild_ok: room %d: is not in guild rooms list",
			   room->vnum);
		return TRUE;
	}

	return FALSE;
}

const char *class_name(CHAR_DATA *ch)
{
	class_t *cl;

	if (IS_NPC(ch) || (cl = class_lookup(ch->class)) == NULL)
		return "Mobile";
	return cl->name;
}

const char *class_who_name(CHAR_DATA *ch)
{
	class_t *cl;

	if (IS_NPC(ch) || (cl = class_lookup(ch->class)) == NULL)
		return "Mob";
	return cl->who_name;
}

/* returns class number */
int cn_lookup(const char *name)
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
	class_t *cl;
	race_t *r;

	if (IS_NPC(ch) || ch->level >= LEVEL_IMMORTAL)
		return 25;

	if ((cl = class_lookup(ch->class)) == NULL
	||  (r = race_lookup(PC(ch)->race)) == NULL
	||  !r->race_pcdata)
		return 0;

/* ORG_RACE && RACE serdar*/
	return UMIN(25, 20 + r->race_pcdata->stats[stat] + cl->stats[stat]);
}

bool clan_ok(CHAR_DATA *ch, int sn) 
{
	return TRUE;
}

const char *title_lookup(CHAR_DATA *ch)
{
	class_t *class;

	if ((class = class_lookup(ch->class)) == NULL
	||  (ch->level < 0 || ch->level > MAX_LEVEL))
		return str_empty;

	return class->titles[ch->level][URANGE(1, ch->sex, 2)-1];
}
