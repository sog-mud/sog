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
 * $Id: class.c,v 1.19 1999-10-17 08:55:46 fjoe Exp $
 */

#include <stdio.h>

#include "merc.h"

hash_t classes;

void
class_init(class_t *cl)
{
	int i;

	cl->name = str_empty;
	cl->who_name[0] = '\0';
	cl->attr_prime = 0;
	cl->weapon = 0;
	cl->thac0_00 = 0;
	cl->thac0_32 = 0;
	cl->hp_rate = 100;
	cl->mana_rate = 100;
	cl->class_flags = 0;
	cl->points = 0;
	cl->restrict_align = -1;
	cl->restrict_sex = -1;
	cl->restrict_ethos = -1;
	cl->death_limit = -1;
	cl->skill_spec = str_empty;

	varr_init(&cl->guilds, sizeof(int), 4);
	varr_init(&cl->poses, sizeof(pose_t), 4);

	for (i = 0; i < MAX_STAT; i++)
		cl->stats[i] = 0;

	for (i = 0; i < MAX_LEVEL+1; i++)
		cl->titles[i][0] = cl->titles[i][1] = str_empty;
}

/*
 * poses are not copied intentionally
 */
class_t *
class_cpy(class_t *dst, class_t *src)
{
	int i;

	dst->name = str_dup(src->name);
	strnzcpy(dst->who_name, sizeof(dst->who_name), src->who_name);
	dst->attr_prime = src->attr_prime;
	dst->weapon = src->weapon;
	dst->thac0_00 = src->thac0_00;
	dst->thac0_32 = src->thac0_32;
	dst->hp_rate = src->hp_rate;
	dst->mana_rate = src->mana_rate;
	dst->class_flags = src->class_flags;
	dst->points = src->points;
	dst->restrict_align = src->restrict_align;
	dst->restrict_sex = src->restrict_sex;
	dst->restrict_ethos = src->restrict_ethos;
	dst->death_limit = src->death_limit;
	dst->skill_spec = str_qdup(src->skill_spec);

	varr_cpy(&dst->guilds, &src->guilds);

	for (i = 0; i < MAX_STAT; i++)
		dst->stats[i] = src->stats[i];

	for (i = 0; i < MAX_LEVEL+1; i++) {
		dst->titles[i][0] = str_qdup(src->titles[i][0]);
		dst->titles[i][1] = str_qdup(src->titles[i][1]);
	}

	return dst;
}

void
class_destroy(class_t *cl)
{
	int i;

	free_string(cl->skill_spec);
	varr_destroy(&cl->poses);
	varr_destroy(&cl->guilds);

	for (i = 0; i < MAX_LEVEL+1; i++) {
		free_string(cl->titles[i][0]);
		free_string(cl->titles[i][1]);
	}
}

typedef struct _guild_ok_t {
	int vnum;
	const char *	cn;
	const char *	cn_found;
} _guild_ok_t;

static void *
guild_ok_cb(void *p, void *d)
{
	class_t *cl = (class_t *) p;
	_guild_ok_t *g = (_guild_ok_t *) d;

	int iGuild;

	for (iGuild = 0; iGuild < cl->guilds.nused; iGuild++) {
	    	if (g->vnum == *(int *) VARR_GET(&cl->guilds, iGuild)) {
			if (IS_CLASS(cl->name, g->cn))
				return p;
			g->cn_found = cl->name;
		}
	}

	return NULL;
}

/*
 * guild_ok - check if ch allowed in the room (if the room is guild)
 */
int guild_ok(CHAR_DATA *ch, ROOM_INDEX_DATA *room)
{
	_guild_ok_t g;

	if (!IS_SET(room->room_flags, ROOM_GUILD)
	||  IS_IMMORTAL(ch))
		return TRUE;

	g.vnum = room->vnum;
	g.cn = ch->class;
	g.cn_found = str_empty;
	if (hash_foreach(&classes, guild_ok_cb, &g))
		return TRUE;

	if (IS_NULLSTR(g.cn_found)) {
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

const char *class_who_name(CHAR_DATA *ch)
{
	class_t *cl;

	if (IS_NPC(ch) || (cl = class_lookup(ch->class)) == NULL)
		return "Mob";
	return cl->who_name;
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

const char *title_lookup(CHAR_DATA *ch)
{
	class_t *cl;

	if ((cl = class_lookup(ch->class)) == NULL
	||  (ch->level < 0 || ch->level > MAX_LEVEL))
		return str_empty;

	return cl->titles[ch->level][URANGE(1, ch->sex, 2)-1];
}

bool can_flee(CHAR_DATA *ch)
{
	class_t *cl;

	if (ch->level < LEVEL_PK
	||  (cl = class_lookup(ch->class)) == NULL
	||  cl->death_limit < 0)
		return TRUE;

	return FALSE;
}

