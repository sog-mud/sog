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
 * $Id: clan.c,v 1.52 1999-12-16 12:24:50 fjoe Exp $
 */

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if	defined (WIN32)
#	include <compat/compat.h>
#else
#	include <dirent.h>
#endif

#include "merc.h"

hash_t clans;

void
clan_init(clan_t *clan)
{
	clan->name = str_empty;
	clan->recall_vnum = 0;
	clan->skill_spec = str_empty;
	clan->clan_flags = 0;
	clan->altar_vnum = 0;
	clan->obj_vnum = 0;
	clan->mark_vnum = 0;
	clan->obj_ptr = NULL;
	clan->altar_ptr = NULL;
	clan->leader_list = str_empty;
	clan->member_list = str_empty;
	clan->second_list = str_empty;
}

clan_t *
clan_cpy(clan_t *dst, clan_t *src)
{
	dst->name = str_qdup(src->name);
	dst->recall_vnum = src->recall_vnum;
	dst->skill_spec = str_qdup(src->skill_spec);
	dst->clan_flags = src->clan_flags;
	dst->altar_vnum = src->altar_vnum;
	dst->obj_vnum = src->obj_vnum;
	dst->mark_vnum = src->mark_vnum;
	dst->obj_ptr = src->obj_ptr;
	dst->altar_ptr = src->altar_ptr;
	dst->leader_list = str_qdup(src->leader_list);
	dst->member_list = str_qdup(src->member_list);
	dst->second_list = str_qdup(src->second_list);
	return dst;
}

void
clan_destroy(clan_t *clan)
{
	free_string(clan->name);
	free_string(clan->skill_spec);
	free_string(clan->leader_list);
	free_string(clan->member_list);
	free_string(clan->second_list);
}

void clan_save(clan_t *clan)
{
	SET_BIT(clan->clan_flags, CLAN_CHANGED);
	dofun("asave", NULL, "clans");
}

/*
 * clan_update_lists - remove 'victim' from leader and second lists of 'clan'
 *		       if memb is TRUE 'victim' will be deleted from members
 *		       list
 */
void clan_update_lists(clan_t *clan, CHAR_DATA *victim, bool memb)
{
	const char **nl = NULL;

	switch (PC(victim)->clan_status) {
	case CLAN_SECOND:
		nl = &clan->second_list;
		break;

	case CLAN_LEADER:
		nl = &clan->leader_list;
		break;
	}
	if (nl)
		name_delete(nl, victim->name, NULL, NULL);

	if (memb)
		name_delete(&clan->member_list, victim->name, NULL, NULL);
}

static void *
item_ok_cb(void *p, va_list ap)
{
	clan_t *clan = (clan_t *) p;

	int room_in_vnum = va_arg(ap, int);

	if (room_in_vnum == clan->altar_vnum)
		return p;
	return NULL;
}

bool clan_item_ok(const char *cln)
{
	clan_t* clan;
	OBJ_DATA* obj;
	int room_in;

	if ((clan = clan_lookup(cln)) == NULL
	||  clan->obj_ptr == NULL) 
		return TRUE;

	for (obj = clan->obj_ptr; obj->in_obj != NULL; obj = obj->in_obj)
		;

	if (obj->in_room) 
		room_in=obj->in_room->vnum;
	else 
		return TRUE;

	if (room_in == clan->altar_vnum)
		return TRUE;

	if (hash_foreach(&clans, item_ok_cb, room_in) != NULL)
		return FALSE;

	return TRUE;
}
