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
 * $Id: vo_iter.h,v 1.4 2003-04-24 12:41:49 fjoe Exp $
 */

#ifndef _VO_ITER_H_
#define _VO_ITER_H_

/*
 * vo iterators
 */

struct vo_iter_class_t {
	void *(*first)(void *);
	void *(*next)(void *);
	int mem_type;
};

struct vo_iter_t {
	int mtag;		/* memory tag		*/
	vo_iter_class_t *cl;	/* iterator class	*/
	vo_t vo_cont;		/* container		*/
	vo_t vo_next;		/* next value		*/

	union {
		vo_iter_t *next;
		struct {
			mpc_iter_t *iter;
			int block;
		} mpc;
	} u;
};

void init_iterators(void);

vo_iter_t *iter_new(vo_iter_class_t *cl, void *cont);
void	iter_free(vo_iter_t *iter);

void *	iter_init(vo_iter_t *iter);
#define iter_first(i)	((i)->cl->first((i)->vo_cont.p))
void *	iter_cond(vo_iter_t *iter, void *vo);
#define iter_next(i)	((i)->vo_next.p)
void	iter_destroy(vo_iter_t *iter);

/*
 * vo_foreach iterator classes
 */
extern vo_iter_class_t iter_char_world;
extern vo_iter_class_t iter_npc_world;
extern vo_iter_class_t iter_char_room;
extern vo_iter_class_t iter_obj_world;
extern vo_iter_class_t iter_obj_room;
extern vo_iter_class_t iter_obj_char;
extern vo_iter_class_t iter_obj_obj;
extern vo_iter_class_t iter_descriptor;

#define char_in_world()		iter_new(&iter_char_world, NULL)
#define npc_in_world()		iter_new(&iter_npc_world, NULL)
#define char_in_room(room)	iter_new(&iter_char_room, (room))
#define obj_in_world()		iter_new(&iter_obj_world, NULL)
#define obj_in_room(room)	iter_new(&iter_obj_room, (room))
#define obj_of_char(ch)		iter_new(&iter_obj_char, (ch))
#define obj_in_obj(obj)		iter_new(&iter_obj_obj, (obj))
#define descriptor_in_world()	iter_new(&iter_descriptor, NULL)

#define foreach(elem, iter)						\
	{								\
		vo_iter_t *elem##_i = iter;				\
		for (elem = iter_init(elem##_i);			\
		     (elem = iter_cond(elem##_i, elem)) != NULL;	\
		     elem = iter_next(elem##_i))
#define foreach_done(elem)	iter_free(elem##_i)
#define end_foreach(elem)						\
		foreach_done(elem);					\
	}

#endif
