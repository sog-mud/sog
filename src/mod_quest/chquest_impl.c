/*-
 * Copyright (c) 1999, 2000 SoG Development Team
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
 * $Id: chquest_impl.c,v 1.5 2001-08-02 18:20:13 fjoe Exp $
 */

#include <stdio.h>

#include <merc.h>

#include <handler.h>

#include <quest.h>
#include "chquest_impl.h"

chquest_t *chquest_list;		/* global list of chquests	     */

void
chquest_init(void)
{
	int i;

	for (i = 0; i < MAX_KEY_HASH; i++) {
		OBJ_INDEX_DATA *pObjIndex;

		for (pObjIndex = obj_index_hash[i]; pObjIndex != NULL;
		     pObjIndex = pObjIndex->next) {
			if (IS_SET(pObjIndex->obj_flags, OBJ_CHQUEST))
				chquest_add(pObjIndex);
		}
	}
}

void
chquest_update(void)
{
	chquest_t *q;
	static bool chquest_started;

	if (!chquest_started) {
		chquest_start(0);
		chquest_started = TRUE;
	}

	for (q = chquest_list; q; q = q->next) {
		if (!IS_WAITING(q))
			continue;
		if (!--q->delay)
			chquest_startq(q);
	}
}

/*
 * chquest_startq - start given chquest
 *
 * assumes that !IS_RUNNING(q) check is done
 */
void
chquest_startq(chquest_t *q)
{
	ROOM_INDEX_DATA *room;

#if CHQUEST_DEBUG
	log(LOG_INFO, "chquest_startq: started chquest for '%s' (vnum %d)",
	    mlstr_mval(&q->obj_index->short_descr), q->obj_index->vnum);
#endif

	SET_RUNNING(q);
	q->obj = create_obj(q->obj_index, 0);
	q->obj->timer = number_range(100, 150);

	do {
		room = get_random_room(NULL, NULL);
	} while (IS_SET(room->area->area_flags, AREA_NOQUEST));

	obj_to_room(q->obj, room);
}

void
chquest_stopq(chquest_t *q)
{
	if (IS_STOPPED(q))
		return;

#if CHQUEST_DEBUG
	log(LOG_INFO, "chquest_stopq: stopped quest for '%s' (vnum %d)",
		   mlstr_mval(&q->obj_index->short_descr), q->obj_index->vnum);
#endif

	if (IS_RUNNING(q))
		extract_obj(q->obj, XO_F_NOCHQUEST);
	SET_STOPPED(q);
}

chquest_t *
chquest_lookup(OBJ_INDEX_DATA *obj_index)
{
	chquest_t *q;

	for (q = chquest_list; q; q = q->next) {
		if (q->obj_index == obj_index)
			return q;
	}

	return NULL;
}

chquest_t *
chquest_lookup_obj(OBJ_DATA *obj)
{
	chquest_t *q = chquest_lookup(obj->pObjIndex);

	if (q == NULL || !IS_RUNNING(q) || q->obj != obj)
		return NULL;
	return q;
}
