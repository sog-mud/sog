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
 * $Id: chquest_impl.h,v 1.1 2000-06-02 16:41:01 fjoe Exp $
 */

#ifndef __CHQUEST_H_
#define __CHQUEST_H_

#include "quest.h"

typedef struct chquest_t chquest_t;
struct chquest_t {
	OBJ_INDEX_DATA *obj_index;

	OBJ_DATA *obj;			/* NULL if this quest is not running */

	int delay;			/* > 0  - delay before next repop    */
					/*        (in area ticks)	     */
					/* == 0 - quest is running	     */
					/* < 0  - quest is stopped	     */
	chquest_t *next;
};

extern chquest_t *chquest_list;		/* global list of chquests	     */

#define IS_RUNNING(q)	(q->delay == 0)
#define IS_WAITING(q)	(q->delay > 0)
#define IS_STOPPED(q)	(q->delay < 0)

#define SET_STOPPED(q)		(q->delay = -1)
#define SET_RUNNING(q)		(q->delay = 0)
#define SET_WAITING(q, aticks)	(q->delay = aticks)

void chquest_startq(chquest_t *q);
void chquest_stopq(chquest_t *q);

chquest_t *chquest_lookup(OBJ_INDEX_DATA *obj_index);
chquest_t *chquest_lookup_obj(OBJ_DATA *obj);

#endif
