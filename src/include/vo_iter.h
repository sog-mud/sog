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
 * $Id: vo_iter.h,v 1.2 2001-09-15 17:12:34 fjoe Exp $
 */

#ifndef _VO_ITER_H_
#define _VO_ITER_H_

/*
 * vo iterators
 */

typedef void *(*vo_foreach_cb_t)(void *, va_list);

struct vo_iter_t {
	void *(*first)(void *);
	void *(*next)(void *);
	int mem_type;
};

/*
 * vo_foreach iterators
 */
extern vo_iter_t iter_char_world;
extern vo_iter_t iter_npc_world;
extern vo_iter_t iter_char_room;
extern vo_iter_t iter_obj_world;
extern vo_iter_t iter_obj_room;
extern vo_iter_t iter_obj_char;
extern vo_iter_t iter_obj_obj;

void *	vo_foreach(void *cont, vo_iter_t *iter, vo_foreach_cb_t cb, ...);

void *	vo_foreach_init(void *cont, vo_iter_t *iter, int *pftag);
void	vo_foreach_destroy(void *cont, vo_iter_t *iter, int ftag, bool untag);
bool	vo_foreach_cond(void *cont, vo_iter_t *iter, int ftag,
			void **pvo, void **pvo_next);

#endif
