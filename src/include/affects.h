/*-
 * Copyright (c) 2001 SoG Development Team
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
 * $Id: affects.h,v 1.18 2001-07-29 20:14:27 fjoe Exp $
 */

#ifndef _AFFECTS_H_
#define _AFFECTS_H_

#undef MODULE_NAME
#define MODULE_NAME MOD_AFFECTS
#include <dynafun_decl.h>

__MODULE_START_DECL

DECLARE_PROC3(affect_modify,
	      ARG(CHAR_DATA), ch, ARG(AFFECT_DATA), paf, ARG(bool), fAdd)
DECLARE_PROC3(affect_modify_room,
	      ARG(ROOM_INDEX_DATA), room, ARG(AFFECT_DATA), paf,
	      ARG(bool), fAdd)
DECLARE_FUN2(AFFECT_DATA, affect_find,
	     NULLABLE_ARG(AFFECT_DATA), paf, ARG(cchar_t), sn)
DECLARE_PROC3(affect_check,
	      ARG(CHAR_DATA), ch, ARG(int), where, ARG(flag_t), vector)
DECLARE_PROC4(affect_check_list,
	      ARG(CHAR_DATA), ch, ARG(AFFECT_DATA), paf, ARG(int), where,
	      ARG(flag_t), vector)
DECLARE_PROC1(affect_enchant,
	      ARG(OBJ_DATA), obj)

DECLARE_PROC2(affect_to_char,
	      ARG(CHAR_DATA), ch, ARG(AFFECT_DATA), paf)
DECLARE_PROC2(affect_to_obj,
	      ARG(OBJ_DATA), obj, ARG(AFFECT_DATA), paf)
DECLARE_PROC2(affect_remove,
	      ARG(CHAR_DATA), ch, ARG(AFFECT_DATA), paf)
DECLARE_PROC2(affect_remove_obj,
	      ARG(OBJ_DATA), obj, ARG(AFFECT_DATA), paf)
DECLARE_PROC2(affect_strip,
	      ARG(CHAR_DATA), ch, ARG(cchar_t), sn)
DECLARE_PROC3(affect_bit_strip,
	      ARG(CHAR_DATA), ch, ARG(int), where, ARG(flag_t), bits)
DECLARE_PROC2(affect_join,
	      ARG(CHAR_DATA), ch, ARG(AFFECT_DATA), paf)

DECLARE_FUN2(bool, is_affected,
	     ARG(CHAR_DATA), ch, ARG(cchar_t), sn)
DECLARE_FUN3(AFFECT_DATA, is_bit_affected,
	     ARG(CHAR_DATA), ch, ARG(int), where, ARG(flag_t), bits)
DECLARE_FUN2(bool, has_obj_affect,
	     ARG(CHAR_DATA), ch, ARG(flag_t), vector)

/* room affects */
DECLARE_PROC2(affect_to_room,
	      ARG(ROOM_INDEX_DATA), room, ARG(AFFECT_DATA), paf)
DECLARE_PROC2(affect_remove_room,
	      ARG(ROOM_INDEX_DATA), room, ARG(AFFECT_DATA), paf)
DECLARE_PROC2(affect_strip_room,
	      ARG(ROOM_INDEX_DATA), ch, ARG(cchar_t), sn)
DECLARE_FUN2(bool, is_affected_room,
	     ARG(ROOM_INDEX_DATA), ch, ARG(cchar_t), sn)
DECLARE_PROC1(strip_raff_owner,
	      ARG(CHAR_DATA), ch)

DECLARE_PROC2(aff_dump_list,
	      ARG(AFFECT_DATA), paf, ARG(BUFFER), output)
DECLARE_PROC3(show_affects,
	      ARG(CHAR_DATA), ch, ARG(CHAR_DATA), vch, ARG(BUFFER), output)

/* format_obj_affects flags */
#define FOA_F_NODURATION	(A)	/* do not show duration		*/
#define FOA_F_NOAFFECTS		(B)	/* do not show bit affects	*/

DECLARE_PROC3(format_obj_affects,
	      ARG(BUFFER), output, ARG(AFFECT_DATA), paf, ARG(int), flags)

__MODULE_END_DECL

#endif /* _AFFECTS_H_ */
