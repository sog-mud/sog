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
 * $Id: mpc_dynafun.h,v 1.23 2004-02-13 14:48:15 fjoe Exp $
 */

#ifndef _MPC_DYNAFUN_H_
#define _MPC_DYNAFUN_H_

#undef MODULE_NAME
#define MODULE_NAME MOD_MPC
#include <dynafun_decl.h>

__MODULE_START_DECL

DECLARE_FUN4(bool, has_sp,
	     ARG(CHAR_DATA), ch, ARG(cchar_t), spn,
	     ARG(cchar_t), spn_add, ARG(cchar_t), spn_rm)
DECLARE_FUN4(int, spclass_count,
	     ARG(CHAR_DATA), ch, ARG(cchar_t), spn,
	     ARG(cchar_t), spn_add, ARG(cchar_t), spn_rm)
DECLARE_FUN1(bool, is_immortal,
	     ARG(CHAR_DATA), ch)
DECLARE_FUN1(bool, is_ghost,
	     ARG(CHAR_DATA), ch)
DECLARE_FUN1(bool, is_good,
	     ARG(CHAR_DATA), ch)
DECLARE_FUN1(bool, is_neutral,
	     ARG(CHAR_DATA), ch)
DECLARE_FUN1(bool, is_evil,
	     ARG(CHAR_DATA), ch)
DECLARE_FUN2(bool, is_owner,
	     ARG(OBJ_DATA), obj, ARG(CHAR_DATA), ch)
DECLARE_PROC2(wait_state,
	      ARG(CHAR_DATA), ch, ARG(int), ws)
DECLARE_FUN2(bool, is_affected,
	     ARG(CHAR_DATA), ch, ARG(int), aff)
DECLARE_FUN1(bool, is_npc,
	     ARG(CHAR_DATA), ch)
DECLARE_FUN2(int, umin,
	     ARG(int), i1, ARG(int), i2)
DECLARE_FUN2(int, umax,
	     ARG(int), i1, ARG(int), i2)
DECLARE_FUN1(bool, is_wanted,
	     ARG(CHAR_DATA), ch)
DECLARE_FUN1(bool, is_awake,
	     ARG(CHAR_DATA), ch)
DECLARE_FUN2(bool, weapon_is,
	     ARG(OBJ_DATA), obj, ARG(int), wclass)
DECLARE_FUN0(int, time_hour)
DECLARE_FUN1(CHAR_DATA, get_random_fighting,
	     ARG(CHAR_DATA), ch)
DECLARE_FUN2(bool, can_wear,
	     ARG(OBJ_DATA), obj, ARG(int), wear)
DECLARE_FUN2(bool, is_owner_name,
	     ARG(OBJ_DATA), obj, ARG(cchar_t), name)
DECLARE_FUN1(bool, is_pumped,
	     ARG(CHAR_DATA), ch)
DECLARE_FUN2(bool, is_mount,
	     ARG(CHAR_DATA), ch, ARG(CHAR_DATA), mob)
DECLARE_FUN2(bool, is_act,
	     ARG(CHAR_DATA), ch, ARG(int), act_flags)
DECLARE_FUN2(bool, char_name_is,
	     ARG(CHAR_DATA), ch, ARG(cchar_t), nl)
DECLARE_FUN2(bool, has_detect,
	     ARG(CHAR_DATA), ch, ARG(int), f)
DECLARE_FUN2(bool, has_invis,
	     ARG(CHAR_DATA), ch, ARG(int), f)
DECLARE_FUN2(bool, room_is,
	     ARG(ROOM_INDEX_DATA), r, ARG(int), f)
DECLARE_FUN2(bool, char_form_is,
	     ARG(CHAR_DATA), ch, ARG(int), form)
DECLARE_PROC2(set_obj_owner,
	      ARG(OBJ_DATA), obj, ARG(CHAR_DATA), ch)
DECLARE_PROC2(mob_interpret,
	      ARG(CHAR_DATA), ch, ARG(cchar_t), argument)
DECLARE_FUN2(CHAR_DATA, load_mob,
	     ARG(CHAR_DATA), ch, ARG(int), vnum)
DECLARE_FUN2(OBJ_DATA, load_obj,
	     ARG(CHAR_DATA), ch, ARG(int), vnum)
DECLARE_FUN2(bool, transfer_group,
	     ARG(CHAR_DATA), ch, ARG(ROOM_INDEX_DATA), room)
DECLARE_FUN2(AFFECT_DATA, mpc_aff_new,
	     ARG(int), where, ARG(cchar_t), sn)

__MODULE_END_DECL

#endif /* _MPC_DYNAFUN_H_ */
