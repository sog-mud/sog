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
 * $Id: chquest.c,v 1.21 2001-07-31 14:56:18 fjoe Exp $
 */

/*
 * Challenge quests
 *
 * Challenge items have only one instance and are dropped randomly
 * in the start of the game (chquest_start) or by request of an immortal
 * ('chquest start'). Challenge items can be located only if they are
 * carried by someone. Challenge items are dropped when the char quits
 * the game. All questors can locate challenge items and
 * inform characters about current challenge items status ("carried by someone"
 * or "is somewhere (not found yet)").
 * If challenge items is extracted, the challenge is restarted automagically
 * (after some delay)
 */

#include <stdio.h>
#include <stdlib.h>

#include "merc.h"
#include "auction.h"

#include "_chquest.h"

#define CHQUEST_DEBUG 0

/*
 * chquest_start -- create all objects and drop them randomly
 */
void
chquest_start(int flags)
{
	chquest_t *q;

	for (q = chquest_list; q; q = q->next) {
		if (IS_RUNNING(q)
		||  (!IS_SET(flags, CHQUEST_F_NODELAY) && IS_WAITING(q)))
			continue;

		chquest_startq(q);
	}
}

void
chquest_add(OBJ_INDEX_DATA *obj_index)
{
	chquest_t *q;

	if ((q = chquest_lookup(obj_index)) != NULL)
		return;

#if CHQUEST_DEBUG
	log(LOG_INFO, "chquest_add: added '%s' (vnum %d)",
		   mlstr_mval(&obj_index->short_descr), obj_index->vnum);
#endif

	q = calloc(1, sizeof(*q));
	q->obj_index = obj_index;
	SET_STOPPED(q);
	q->next = chquest_list;
	chquest_list = q;
}

/*
 * chquest_delete -- delete obj_index from list of challenge quests
 *		     returns TRUE on success, FALSE on error
 */
bool
chquest_delete(CHAR_DATA *ch, OBJ_INDEX_DATA *obj_index)
{
	chquest_t *q;
	chquest_t *q_prev = NULL;

	for (q = chquest_list; q; q_prev = q, q = q->next) {
		if (q->obj_index == obj_index)
			break;
	}

	if (q == NULL)
		return TRUE;

	if (IS_RUNNING(q) && IS_AUCTIONED(q->obj)) {
		act("$p is on auction right now.",
		    ch, q->obj, NULL, TO_CHAR);
		return FALSE;
	}

	if (q_prev)
		q_prev->next = q->next;
	else
		chquest_list = chquest_list->next;

	chquest_stopq(q);
	free(q);

#if CHQUEST_DEBUG
	log(LOG_INFO, "chquest_delete: deleted '%s' (vnum %d)",
		   mlstr_mval(&obj_index->short_descr), obj_index->vnum);
#endif
	return TRUE;
}

/*
 * stop challenge quest if item is extracted. called from extract_obj
 * quest will be restarted automatically from chquest_update (mod_update)
 * after random delay
 */
void
chquest_extract(OBJ_DATA *obj)
{
	chquest_t *q;

	if ((q = chquest_lookup_obj(obj)) == NULL)
		return;

#if CHQUEST_DEBUG
	log(LOG_INFO, "chquest_extract: finished quest for '%s' (vnum %d)",
		   mlstr_mval(&q->obj_index->short_descr), q->obj_index->vnum);
#endif
	SET_WAITING(q, number_range(15, 20));

	return;
}

CHAR_DATA *
chquest_carried_by(OBJ_DATA *obj)
{
	chquest_t *q;

	if ((q = chquest_lookup_obj(obj)) == NULL)
		return NULL;

	/* find the uppest obj container */
	while(obj->in_obj)
		obj = obj->in_obj;

	return obj->carried_by;
}
