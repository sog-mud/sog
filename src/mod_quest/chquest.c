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
 * $Id: chquest.c,v 1.11 1999-06-10 18:18:55 fjoe Exp $
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
#include "chquest.h"
#include "auction.h"

DECLARE_DO_FUN(do_help);

static void chquest_startq(chquest_t *q);
static void chquest_stopq(chquest_t *q);
static inline void chquest_status(CHAR_DATA *ch);

static chquest_t *chquest_lookup(OBJ_INDEX_DATA *obj_index);
static chquest_t *chquest_lookup_obj(OBJ_DATA *obj);

chquest_t *chquest_list;

void do_chquest(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		do_help(ch, "'WIZ CHQUEST'");
		return;
	}

	if (!str_prefix(arg, "restart")) {
		chquest_start(CHQUEST_F_NODELAY);
		char_puts("Challenge quest restarted.\n", ch);
		return;
	}

	if (!str_prefix(arg, "status")) {
		chquest_status(ch);
		return;
	}

	if (!str_prefix(arg, "start")
	||  !str_prefix(arg, "stop")
	||  !str_prefix(arg, "add")
	||  !str_prefix(arg, "delete")) {
		char arg2[MAX_INPUT_LENGTH];
		OBJ_INDEX_DATA *obj_index;
		chquest_t *q;

		one_argument(argument, arg2, sizeof(arg2));
		if (!is_number(arg2)) {
			do_chquest(ch, str_empty);
			return;
		}

		if ((obj_index = get_obj_index(atoi(arg2))) == NULL) {
			char_printf(ch, "do_chquest: %s: no object with that vnum.\n", arg2);
			return;
		}

		if (!str_prefix(arg, "delete")) {
			chquest_delete(ch, obj_index);
			return;
		}

		if (!str_prefix(arg, "add")) {
			chquest_add(obj_index);
			return;
		}

		if ((q = chquest_lookup(obj_index)) == NULL) {
			char_printf(ch, "do_chquest: %s: no chquests with that vnum.\n", arg2);
			return;
		}

		if (!str_prefix(arg, "start")) {
			if (IS_RUNNING(q)) {
				char_printf(ch, "do_chquest: quest vnum %d "
						"already running.\n",
						q->obj_index->vnum);
				return;
			}
			chquest_startq(q);
			return;
		}

		if (IS_RUNNING(q) && IS_AUCTIONED(q->obj)) {
			act("$p is on auction right now.",
			    ch, q->obj, NULL, TO_CHAR);
			return;
		}

		chquest_stopq(q);
		return;
	}

	do_chquest(ch, str_empty);
}

/*
 * chquest_start -- create all objects and drop them randomly
 */
void chquest_start(int flags)
{
	chquest_t *q;

	for (q = chquest_list; q; q = q->next) {
		if (IS_RUNNING(q)
		||  (!IS_SET(flags, CHQUEST_F_NODELAY) && IS_WAITING(q)))
			continue;

		chquest_startq(q);
	}
}

/*
 * chquest_update -- update and restart chquests (called every area tick)
 */
void chquest_update(void)
{
	chquest_t *q;

	for (q = chquest_list; q; q = q->next) {
		if (!IS_WAITING(q))
			continue;
		if (!--q->delay)
			chquest_startq(q);
	}
}

void chquest_add(OBJ_INDEX_DATA *obj_index)
{
	chquest_t *q;

	if ((q = chquest_lookup(obj_index)) != NULL)
		return;

	log("chquest_add: added '%s' (vnum %d)",
		   mlstr_mval(&obj_index->short_descr), obj_index->vnum);

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
bool chquest_delete(CHAR_DATA *ch, OBJ_INDEX_DATA *obj_index)
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

	log("chquest_delete: deleted '%s' (vnum %d)",
		   mlstr_mval(&obj_index->short_descr), obj_index->vnum);
	return TRUE;
}

/*
 * stop challenge quest if item is extracted. called from extract_obj
 * quest will be restarted automatically from chquest_update after
 * random delay (in area ticks)
 */
void chquest_extract(OBJ_DATA *obj)
{
	chquest_t *q;

	if ((q = chquest_lookup_obj(obj)) == NULL)
		return;

	log("chquest_extract: finished quest for '%s' (vnum %d)",
		   mlstr_mval(&q->obj_index->short_descr), q->obj_index->vnum);
	SET_WAITING(q, number_range(15, 20));
}

CHAR_DATA *chquest_carried_by(OBJ_DATA *obj)
{
	chquest_t *q;

	if ((q = chquest_lookup_obj(obj)) == NULL)
		return NULL;

	/* find the uppest obj container */
	while(obj->in_obj)
		obj = obj->in_obj;

	return obj->carried_by;
}

/*
 * static functions
 */

static chquest_t *chquest_lookup(OBJ_INDEX_DATA *obj_index)
{
	chquest_t *q;

	for (q = chquest_list; q; q = q->next) {
		if (q->obj_index == obj_index)
			return q;
	}

	return NULL;
}

static chquest_t *chquest_lookup_obj(OBJ_DATA *obj)
{
	chquest_t *q = chquest_lookup(obj->pIndexData);

	if (q == NULL || !IS_RUNNING(q) || q->obj != obj)
		return NULL;
	return q;
}

/*
 * chquest_startq - start given chquest
 *
 * assumes that !IS_RUNNING(q) check is done
 */
static void chquest_startq(chquest_t *q)
{
	ROOM_INDEX_DATA *room;

	log("chquest_startq: started chquest for '%s' (vnum %d)",
	   	   mlstr_mval(&q->obj_index->short_descr), q->obj_index->vnum);

	SET_RUNNING(q);
	q->obj = create_obj(q->obj_index, 0);
	q->obj->timer = number_range(100, 150);

	do {
		room = get_random_room(NULL, NULL);
	} while (IS_SET(room->area->area_flags, AREA_NOQUEST));

	obj_to_room(q->obj, room);
}

static void chquest_stopq(chquest_t *q)
{
	if (IS_STOPPED(q))
		return;

	log("chquest_stopq: stopped quest for '%s' (vnum %d)",
		   mlstr_mval(&q->obj_index->short_descr), q->obj_index->vnum);
	if (IS_RUNNING(q))
		extract_obj(q->obj, XO_F_NOCHQUEST);
	SET_STOPPED(q);
}

static inline void chquest_status(CHAR_DATA *ch)
{
	chquest_t *q;

	char_puts("Challenge quest items:\n", ch);
	for (q = chquest_list; q; q = q->next) {
		OBJ_DATA *obj;

		char_printf(ch, "- %s (vnum %d) - ",
			    mlstr_mval(&q->obj_index->short_descr),
			    q->obj_index->vnum);

		if (IS_STOPPED(q)) {
			char_puts("stopped.\n", ch);
			continue;
		} else if (IS_WAITING(q)) {
			char_printf(ch, "%d area ticks to start.\n",
				    q->delay);
			continue;
		}

		if ((obj = q->obj) == NULL) {
			char_puts("status unknown.\n", ch);
			continue;
		}

		char_printf(ch, "running (%d ticks left).\n",
			    q->obj->timer);

		while (obj->in_obj)
			obj = obj->in_obj;

		if (obj->carried_by) {
			act_puts3("        $r (vnum $J), carried by $N.",
				  ch, obj->carried_by->in_room, obj->carried_by,
				  (const void*) obj->carried_by->in_room->vnum,
				  TO_CHAR, POS_DEAD);
		} else if (obj->in_room) {
			act_puts3("         $r (vnum $J).",
				  ch, obj->in_room, NULL,
				  (const void*) obj->in_room->vnum,
				  TO_CHAR, POS_DEAD);
		}
	}
}

