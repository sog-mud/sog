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
 * $Id: clan.c,v 1.48 1999-09-08 10:40:07 fjoe Exp $
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

varr clans = { sizeof(clan_t), 4 };

clan_t *clan_new(void)
{
	clan_t *clan;

	clan = varr_enew(&clans);
	varr_init(&clan->skills, sizeof(clskill_t), 4);

	return clan;
}

void clan_free(clan_t *clan)
{
	varr_destroy(&clan->skills);
}

void clan_save(clan_t *clan)
{
	SET_BIT(clan->clan_flags, CLAN_CHANGED);
	dofun("asave", NULL, "clans");
}

int cln_lookup(const char *name)
{
	int cln;

	if (IS_NULLSTR(name))
		return -1;

	for (cln = 0; cln < clans.nused; cln++)
		if (!str_prefix(name, CLAN(cln)->name))
			return cln;

	return -1;
}

const char *clan_name(int cln)
{
	clan_t *clan = clan_lookup(cln);
	if (clan)
		return clan->name;
	return "None";
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

bool clan_item_ok(int cln)
{
	clan_t* clan;
	OBJ_DATA* obj;
	int room_in;
	int i;

	if (!(clan = clan_lookup(cln)) || !(clan->obj_ptr)) 
		return TRUE;

	for (obj = clan->obj_ptr; obj->in_obj != NULL; obj = obj->in_obj)
		;

	if (obj->in_room) 
		room_in=obj->in_room->vnum;
	else 
		return TRUE;

	if (room_in == clan->altar_vnum)
		return TRUE;

	for (i = 0; i < clans.nused; i++)
		if (room_in == clan_lookup(i)->altar_vnum)
			return FALSE;
	return TRUE;
}
