/*-
 * Copyright (c) 1998 SoG Development Team
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
 * $Id: class.c,v 1.37 2001-09-15 17:12:53 fjoe Exp $
 */

#include <stdio.h>

#include <merc.h>

avltree_t classes;

static varr_info_t c_info_guilds = {
	&varr_ops, NULL, NULL,

	sizeof(int), 4
};

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

static varr_info_t c_info_poses =
{
	&varr_ops,

	(e_init_t) pose_init,
	(e_destroy_t) pose_destroy,

	sizeof(pose_t), 4
};

static void
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
	cl->restrict_align = 0;
	cl->restrict_sex = str_empty;
	cl->restrict_ethos = 0;
	cl->death_limit = -1;
	cl->luck_bonus = 0;
	cl->skill_spec = str_empty;

	c_init(&cl->guilds, &c_info_guilds);
	c_init(&cl->poses, &c_info_poses);
	for (i = 0; i < MAX_STAT; i++)
		cl->mod_stat[i] = 0;
}

static void
class_destroy(class_t *cl)
{
	free_string(cl->restrict_sex);
	free_string(cl->skill_spec);
	c_destroy(&cl->poses);
	c_destroy(&cl->guilds);
}

avltree_info_t c_info_classes =
{
	&avltree_ops,

	(e_init_t) class_init,
	(e_destroy_t) class_destroy,

	MT_PVOID, sizeof(class_t), ke_cmp_str,
};

static void *
guild_ok_cb(void *p, va_list ap)
{
	class_t *cl = (class_t *) p;

	CHAR_DATA *ch = va_arg(ap, CHAR_DATA *);
	int vnum = va_arg(ap, int);
	const char **cn_found = va_arg(ap, const char **);
	size_t iGuild;

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
int
guild_ok(CHAR_DATA *ch, ROOM_INDEX_DATA *room)
{
	const char *cn_found;

	if (!IS_SET(room->room_flags, ROOM_GUILD)
	||  IS_IMMORTAL(ch))
		return TRUE;

	if (c_foreach(&classes, guild_ok_cb, ch, room->vnum, &cn_found))
		return TRUE;

	if (IS_NULLSTR(cn_found)) {
		/*
		 * room was not found in the list of guild rooms
		 * of all classes
		 */
		log(LOG_INFO, "guild_ok: room %d: is not in guild rooms list",
			   room->vnum);
		return TRUE;
	}

	return FALSE;
}

const char *
class_who_name(CHAR_DATA *ch)
{
	class_t *cl;

	if (IS_NPC(ch) || (cl = class_lookup(ch->class)) == NULL)
		return "Mob";					// notrans
	return cl->who_name;
}

bool
can_flee(CHAR_DATA *ch)
{
	class_t *cl;

	if (ch->level < LEVEL_PK
	||  (cl = class_lookup(ch->class)) == NULL
	||  cl->death_limit < 0)
		return TRUE;

	return FALSE;
}
