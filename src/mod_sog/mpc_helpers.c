/*-
 * Copyright (c) 2001 SoG Development Team
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
 * $Id: mpc_helpers.c,v 1.3 2001-09-23 16:24:23 fjoe Exp $
 */

#include <stdio.h>

#include <merc.h>

#include <sog.h>

void
mob_interpret(CHAR_DATA *ch, const char *argument)
{
	interpret(ch, argument, FALSE);
}

CHAR_DATA *
load_mob(CHAR_DATA *ch, int vnum)
{
	CHAR_DATA *mob;

	if ((mob = create_mob(vnum, 0)) == NULL)
		return NULL;

	char_to_room(mob, ch->in_room);
	return mob;
}

OBJ_DATA *
load_obj(CHAR_DATA *ch, int vnum)
{
	OBJ_DATA *obj;

	if ((obj = create_obj(vnum, 0)) == NULL)
		return NULL;

	obj_to_char(obj, ch);
	return obj;
}

bool
transfer_group(CHAR_DATA *ch, ROOM_INDEX_DATA *room)
{
	CHAR_DATA *victim, *victim_next;
	bool found = FALSE;

	for (victim = ch->in_room->people; victim != NULL; victim = victim_next) {
		victim_next = victim->next_in_room;

		if (is_same_group(ch, victim)
		&&  transfer_char(victim, room)
		&&  !found)
			found = TRUE;
	}

	return found;
}

void
affect_char(CHAR_DATA *ch, int where, const char *sn,
	    int level, int duration, int loc, int mod, int bits)
{
	AFFECT_DATA *paf;

	if (!IS_APPLY_WHERE(where))
		return;

	paf = aff_new(where, sn);

	paf->level = level;
	paf->duration = duration;

	INT(paf->location) = loc;
	paf->modifier = mod;
	paf->bitvector = bits;

	affect_to_char(ch, paf);
	aff_free(paf);
}

void
set_char_hit(CHAR_DATA *ch, int hit)
{
	ch->hit = hit;
	update_pos(ch);
}
