/*-
 * Copyright (c) 1999 SoG Development Team
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
 * $Id: vo_iter.c,v 1.9 2001-09-15 17:12:55 fjoe Exp $
 */

#include <stdio.h>

#include <merc.h>

/*
 * vo iterators
 */
#define VO_ITER(fun) static void *fun(void *vo __attribute__((unused)))

VO_ITER(first_char_world)
{
	return char_list;
}

VO_ITER(first_npc_world)
{
	return npc_list;
}

VO_ITER(next_char_world)
{
	return ((CHAR_DATA *) vo)->next;
}

VO_ITER(first_char_room)
{
	if (vo == NULL)
		return NULL;
	return ((ROOM_INDEX_DATA *)vo)->people;
}

VO_ITER(next_char_room)
{
	return ((CHAR_DATA *)vo)->next_in_room;
}

VO_ITER(first_obj_world)
{
	return object_list;
}

VO_ITER(next_obj_world)
{
	return ((OBJ_DATA *)vo)->next;
}

VO_ITER(first_obj_room)
{
	if (vo == NULL)
		return NULL;
	return ((ROOM_INDEX_DATA *)vo)->contents;
}

VO_ITER(first_obj_char)
{
	if (vo == NULL)
		return NULL;
	return ((CHAR_DATA *)vo)->carrying;
}

VO_ITER(first_obj_obj)
{
	if (vo == NULL)
		return NULL;
	return ((OBJ_DATA *)vo)->contains;
}

VO_ITER(next_obj_list)
{
	return ((OBJ_DATA *) vo)->next_content;
}

vo_iter_t iter_char_world = {
	first_char_world,
	next_char_world,
	MT_CHAR
};

vo_iter_t iter_npc_world = {
	first_npc_world,
	next_char_world,
	MT_CHAR
};

vo_iter_t iter_char_room = {
	first_char_room,
	next_char_room,
	MT_CHAR
};

vo_iter_t iter_obj_world = {
	first_obj_world,
	next_obj_world,
	MT_OBJ
};

vo_iter_t iter_obj_room = {
	first_obj_room,
	next_obj_list,
	MT_OBJ
};

vo_iter_t iter_obj_char = {
	first_obj_char,
	next_obj_list,
	MT_OBJ
};

vo_iter_t iter_obj_obj = {
	first_obj_obj,
	next_obj_list,
	MT_OBJ
};

void *
vo_foreach(void *cont, vo_iter_t *iter, vo_foreach_cb_t cb, ...)
{
	void *rv = NULL;
	void *vo, *vo_next;
	va_list ap;
	int ftag;

	if ((vo = vo_foreach_init(cont, iter, &ftag)) == 0)
		return NULL;

	va_start(ap, cb);
	while (vo_foreach_cond(cont, iter, ftag, &vo, &vo_next)) {
		if ((rv = cb(vo, ap)) != NULL)
			break;

		vo = vo_next;
	}
	va_end(ap);

	vo_foreach_destroy(cont, iter, ftag, rv != NULL);

	return rv;
}

static int cnt;

void *
vo_foreach_init(void *cont, vo_iter_t *iter, int *pftag)
{
	void *vo;

	if (cnt < 0 || cnt > 7) {
		log(LOG_BUG, "vo_foreach: cnt overflow (%d)", cnt);
		return NULL;
	}

	/*
	 * sanity check
	 */
	vo = iter->first(cont);
	if (vo == NULL)
		return NULL;	/* short circuit */

	if (!mem_is(vo, iter->mem_type)) {
		log(LOG_BUG, "vo_foreach: bad mt (expect %d)", iter->mem_type);
		return NULL;
	}

	/*
	 * mark all the objects in list
	 */
	*pftag = (1 << cnt++);
	for (; vo != NULL; vo = iter->next(vo))
		mem_tag(vo, *pftag);

	return iter->first(cont);
}

void
vo_foreach_destroy(void *cont, vo_iter_t *iter, int ftag, bool untag)
{
	/*
	 * untag objects that were not processed
	 */
	if (untag) {
		void *vo;

		for (vo = iter->first(cont); vo != NULL; vo = iter->next(vo))
			mem_untag(vo, ftag);
	}

	cnt--;
}

bool
vo_foreach_cond(void *cont, vo_iter_t *iter, int ftag,
		void **pvo, void **pvo_next)
{
	while (*pvo != NULL) {
		/*
		 * skip untagged (already processed) objects
		 */
		if (!mem_tagged(*pvo, ftag)) {
			*pvo = iter->next(*pvo);
			continue;
		}

		/*
		 * extracted object encountered -- restart
		 */
		if (!mem_is(*pvo, iter->mem_type)) {
			log(LOG_INFO, "vo_foreach: restarting (mt %d)\n",
			    iter->mem_type);
			*pvo = iter->first(cont);
			continue;
		}

		/*
		 * untag object
		 */
		mem_untag(*pvo, ftag);

		*pvo_next = iter->next(*pvo);
		return TRUE;
	}

	return FALSE;
}
