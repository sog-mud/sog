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
 * $Id: vo_iter.c,v 1.11 2003-04-24 12:42:20 fjoe Exp $
 */

#include <stdio.h>
#include <stdlib.h>

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
	return ((ROOM_INDEX_DATA *) vo)->people;
}

VO_ITER(next_char_room)
{
	return ((CHAR_DATA *) vo)->next_in_room;
}

VO_ITER(first_obj_world)
{
	return object_list;
}

VO_ITER(next_obj_world)
{
	return ((OBJ_DATA *) vo)->next;
}

VO_ITER(first_obj_room)
{
	if (vo == NULL)
		return NULL;
	return ((ROOM_INDEX_DATA *) vo)->contents;
}

VO_ITER(first_obj_char)
{
	if (vo == NULL)
		return NULL;
	return ((CHAR_DATA *) vo)->carrying;
}

VO_ITER(first_obj_obj)
{
	if (vo == NULL)
		return NULL;
	return ((OBJ_DATA *) vo)->contains;
}

VO_ITER(next_obj_list)
{
	return ((OBJ_DATA *) vo)->next_content;
}

VO_ITER(first_descriptor)
{
	return descriptor_list;
}

VO_ITER(next_descriptor)
{
	return ((DESCRIPTOR_DATA *) vo)->next;
}

vo_iter_class_t iter_char_world = {
	first_char_world,
	next_char_world,
	MT_CHAR
};

vo_iter_class_t iter_npc_world = {
	first_npc_world,
	next_char_world,
	MT_CHAR
};

vo_iter_class_t iter_char_room = {
	first_char_room,
	next_char_room,
	MT_CHAR
};

vo_iter_class_t iter_obj_world = {
	first_obj_world,
	next_obj_world,
	MT_OBJ
};

vo_iter_class_t iter_obj_room = {
	first_obj_room,
	next_obj_list,
	MT_OBJ
};

vo_iter_class_t iter_obj_char = {
	first_obj_char,
	next_obj_list,
	MT_OBJ
};

vo_iter_class_t iter_obj_obj = {
	first_obj_obj,
	next_obj_list,
	MT_OBJ
};

vo_iter_class_t iter_descriptor = {
	first_descriptor,
	next_descriptor,
	MT_DESCRIPTOR
};

vo_iter_t *free_iter_list;
vo_iter_t *descriptor_iter;
static int cnt;

static vo_iter_t *
iter_alloc(vo_iter_class_t *cl, vo_iter_t *next)
{
	vo_iter_t *i = malloc(sizeof(*i));
	i->mtag = 0;
	i->cl = cl;
	i->vo_cont.p = NULL;
	i->vo_next.p = NULL;
	i->u.next = next;
	return i;
}

void
init_iterators(void)
{
	int k;

	for (k = 0; k < 8; k++)
		free_iter_list = iter_alloc(NULL, free_iter_list);
	descriptor_iter = iter_alloc(&iter_descriptor, NULL);
}

vo_iter_t *
iter_new(vo_iter_class_t *cl, void *cont)
{
	vo_iter_t *i;

	if (free_iter_list == NULL) {
		log(LOG_ERROR, "iter_new: free_iter_list is empty");
		return NULL;
	}
	i = free_iter_list;
	free_iter_list = i->u.next;

	i->mtag = -1;
	i->cl = cl;
	i->vo_cont.p = cont;
	return i;
}

void
iter_free(vo_iter_t *i)
{
	iter_destroy(i);
	i->cl = NULL;
	i->vo_cont.p = NULL;
	i->vo_next.p = NULL;

	/* put on free list */
	i->u.next = free_iter_list;
	free_iter_list = i;
}

void *
iter_init(vo_iter_t *i)
{
	void *vo;

	if (i == NULL)
		return NULL;

	/*
	 * sanity check
	 */
	vo = iter_first(i);
	if (vo == NULL)
		return NULL;	/* short circuit */

	if (!mem_is(vo, i->cl->mem_type)) {
		log(LOG_BUG, "iter_init: bad mt (expect %d)", i->cl->mem_type);
		return NULL;
	}

	if (i->mtag < 0) {
		if (cnt < 0) {
			log(LOG_BUG, "iter_init: cnt underflow (%d)", cnt);
			abort();
			return NULL;
		} else if (cnt > 7) {
			log(LOG_BUG, "iter_init: cnt overflow (%d)", cnt);
			return NULL;
		}
		i->mtag = (1 << cnt++);
	}

	/*
	 * mark all the objects in list
	 */
	for (; vo != NULL; vo = i->cl->next(vo))
		mem_tag(vo, i->mtag ? i->mtag : 1);

	return iter_first(i);
}

void *
iter_cond(vo_iter_t *i, void *vo)
{
	if (i == NULL)
		return NULL;

	while (vo != NULL) {
		/*
		 * skip untagged (already processed) objects
		 */
		if (!mem_tagged(vo, i->mtag ? i->mtag : 1)) {
			vo = i->cl->next(vo);
			continue;
		}

		/*
		 * extracted object encountered -- restart
		 */
		if (!mem_is(vo, i->cl->mem_type)) {
			log(LOG_INFO, "vo_foreach: restarting (mt %d)\n",
			    i->cl->mem_type);
			vo = iter_first(i);
			continue;
		}

		/*
		 * untag object
		 */
		mem_untag(vo, i->mtag ? i->mtag : 1);

		iter_next(i) = i->cl->next(vo);
		return vo;
	}

	iter_next(i) = NULL;
	return NULL;
}

void
iter_destroy(vo_iter_t *i)
{
	void *vo;

	/*
	 * untag objects that were not processed
	 */
	for (vo = iter_next(i); (vo = iter_cond(i, vo)) != NULL; vo = iter_next(i))
		;

	if (i->mtag > 0) {
		cnt--;
		i->mtag = -1;
	}
}
