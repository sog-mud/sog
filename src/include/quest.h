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
 * $Id: quest.h,v 1.15 2001-07-08 17:18:47 fjoe Exp $
 */

#ifndef _QUEST_H_
#define _QUEST_H_

#define IS_ON_QUEST(ch)	(PC(ch)->questtime > 0)

#undef MODULE_NAME
#define MODULE_NAME MOD_QUEST
#include <dynafun_decl.h>

/* flags for chquest_start */
#define CHQUEST_F_NODELAY	(A)

__MODULE_START_DECL

DECLARE_PROC2(quest_handle_death,
	     CHAR_DATA, ch, CHAR_DATA, victim)
DECLARE_PROC1(quest_cancel,
	      CHAR_DATA, ch)

DECLARE_PROC3(qtrouble_set,
	      CHAR_DATA, ch, int, vnum, int, count)
DECLARE_PROC2(qtrouble_dump,
	      BUFFER, buf, CHAR_DATA, ch)

DECLARE_PROC1(chquest_start,
	      int, flags)

DECLARE_PROC1(chquest_add,
	      OBJ_INDEX_DATA, obj_index)
DECLARE_FUN2(bool, chquest_delete,
	     CHAR_DATA, ch, OBJ_INDEX_DATA, obj_index)

DECLARE_PROC1(chquest_extract,
	      OBJ_DATA, obj)
DECLARE_FUN1(CHAR_DATA, chquest_carried_by,
	     OBJ_DATA, obj)

__MODULE_END_DECL

#endif
