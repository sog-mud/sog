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
 * $Id: chquest.h,v 1.3 1999-05-26 12:44:49 fjoe Exp $
 */

#ifndef _CHQUEST_H_
#define _CHQUEST_H_

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

/* flags for chquest_start */
#define CHQUEST_F_NODELAY	(A)

void chquest_start(int flags);
void chquest_update(void);

void chquest_add(OBJ_INDEX_DATA *obj_index);
bool chquest_delete(CHAR_DATA *ch, OBJ_INDEX_DATA *obj_index);

void chquest_extract(OBJ_DATA *obj);

CHAR_DATA *chquest_carried_by(OBJ_DATA *obj);

#endif

