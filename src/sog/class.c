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
 * $Id: class.c,v 1.23 1999-12-03 11:57:16 fjoe Exp $
 */

#include <stdio.h>

#include "merc.h"

hash_t classes;

static void	pose_init(pose_t *p);
static void	pose_destroy(pose_t *p);

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
	cl->restrict_sex = str_empty;
	cl->restrict_ethos = -1;
	cl->death_limit = -1;
	cl->skill_spec = str_empty;

	varr_init(&cl->guilds, sizeof(int), 4);
	varr_init(&cl->poses, sizeof(pose_t), 4);
	cl->poses.e_init = (varr_e_init_t) pose_init;
	cl->poses.e_destroy = (varr_e_destroy_t) pose_destroy;
	for (i = 0; i < MAX_STAT; i++)
		cl->stats[i] = 0;
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
	dst->restrict_sex = str_qdup(src->restrict_sex);
	dst->restrict_ethos = src->restrict_ethos;
	dst->death_limit = src->death_limit;
	dst->skill_spec = str_qdup(src->skill_spec);

	varr_cpy(&dst->guilds, &src->guilds);

	for (i = 0; i < MAX_STAT; i++)
		dst->stats[i] = src->stats[i];

	return dst;
}

void
class_destroy(class_t *cl)
{
	free_string(cl->restrict_sex);
	free_string(cl->skill_spec);
	varr_destroy(&cl->poses);
	varr_destroy(&cl->guilds);
}

static void *
guild_ok_cb(void *p, va_list ap)
{
	class_t *cl = (class_t *) p;

	CHAR_DATA *ch = va_arg(ap, CHAR_DATA *);
	int vnum = va_arg(ap, int);
	const char **cn_found = va_arg(ap, const char **);
	int iGuild;

	for (iGuild = 0; iGuild < cl->guilds.nused; iGuild++) {
	    	if (vnum == *(int *) VARR_GET(&cl->guilds, iGuild)) {
			if (IS_CLASS(cl->name, ch->class))
				return p;
			*cn_found = cl->name;
		}
	}

	return NULL;
}

/*
 * guild_ok - check if ch allowed in the room (if the room is guild)
 */
int guild_ok(CHAR_DATA *ch, ROOM_INDEX_DATA *room)
{
	const char *cn_found;

	if (!IS_SET(room->room_flags, ROOM_GUILD)
	||  IS_IMMORTAL(ch))
		return TRUE;

	if (hash_foreach(&classes, guild_ok_cb, ch, room->vnum, &cn_found))
		return TRUE;

	if (IS_NULLSTR(cn_found)) {
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

bool can_flee(CHAR_DATA *ch)
{
	class_t *cl;

	if (ch->level < LEVEL_PK
	||  (cl = class_lookup(ch->class)) == NULL
	||  cl->death_limit < 0)
		return TRUE;

	return FALSE;
}

static void
pose_init(pose_t *p)
{
	p->self = str_empty;
	p->others = str_empty;
}

static void
pose_destroy(pose_t *p)
{
	free_string(p->self);
	free_string(p->others);
}

