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
 * $Id: mpc_dynafun.h,v 1.15 2001-09-23 16:24:17 fjoe Exp $
 */

#ifndef _MPC_DYNAFUN_H_
#define _MPC_DYNAFUN_H_

#undef MODULE_NAME
#define MODULE_NAME MOD_MPC
#include <dynafun_decl.h>

__MODULE_START_DECL

DECLARE_FUN4(int, has_sp,
	     ARG(CHAR_DATA), ch, ARG(cchar_t), spn,
	     ARG(cchar_t), spn_add, ARG(cchar_t), spn_rm)
DECLARE_FUN4(int, spclass_count,
	     ARG(CHAR_DATA), ch, ARG(cchar_t), spn,
	     ARG(cchar_t), spn_add, ARG(cchar_t), spn_rm)
DECLARE_FUN1(bool, is_immortal,
	     ARG(CHAR_DATA), ch)
DECLARE_FUN1(int, char_sex,
	     ARG(CHAR_DATA), ch)
DECLARE_FUN1(bool, is_ghost,
	     ARG(CHAR_DATA), ch)
DECLARE_FUN1(bool, is_good,
	     ARG(CHAR_DATA), ch)
DECLARE_FUN1(bool, is_neutral,
	     ARG(CHAR_DATA), ch)
DECLARE_FUN1(bool, is_evil,
	     ARG(CHAR_DATA), ch)
DECLARE_FUN1(ROOM_INDEX_DATA, char_room,
	     ARG(CHAR_DATA), ch)
DECLARE_FUN1(int, char_level,
	     ARG(CHAR_DATA), ch)
DECLARE_FUN1(int, real_char_level,
	     ARG(CHAR_DATA), ch)
DECLARE_PROC2(set_weapon_dice_type,
	      ARG(OBJ_DATA), weapon, ARG(int), dice_type)
DECLARE_FUN1(CHAR_DATA, char_fighting,
	     ARG(CHAR_DATA), ch)
DECLARE_PROC2(set_obj_level,
	      ARG(OBJ_DATA), obj, ARG(int), level)
DECLARE_FUN2(bool, is_owner,
	     ARG(CHAR_DATA), ch, ARG(OBJ_DATA), obj)
DECLARE_FUN2(bool, is_class,
	     ARG(CHAR_DATA), ch, ARG(cchar_t), cl)
DECLARE_FUN1(int, obj_level,
	     ARG(OBJ_DATA), obj)
DECLARE_FUN1(int, room_sector,
	     ARG(ROOM_INDEX_DATA), room)
DECLARE_FUN1(int, char_max_hit,
	     ARG(CHAR_DATA), ch)
DECLARE_FUN1(int, char_hit,
	     ARG(CHAR_DATA), ch)
DECLARE_FUN1(int, char_max_mana,
	     ARG(CHAR_DATA), ch)
DECLARE_FUN1(int, char_mana,
	     ARG(CHAR_DATA), ch)
DECLARE_PROC2(set_char_mana,
	     ARG(CHAR_DATA), ch, ARG(int), mana)
DECLARE_FUN1(int, char_max_moves,
	     ARG(CHAR_DATA), ch)
DECLARE_FUN1(int, char_moves,
	     ARG(CHAR_DATA), ch)
DECLARE_PROC2(set_char_moves,
	     ARG(CHAR_DATA), ch, ARG(int), moves)
DECLARE_FUN1(int, obj_timer,
	     ARG(OBJ_DATA), obj)
DECLARE_PROC2(set_obj_timer,
	      ARG(OBJ_DATA), obj, ARG(int), timer)
DECLARE_PROC2(set_char_gold,
	      ARG(CHAR_DATA), ch, ARG(int), gold)
DECLARE_PROC2(set_char_silver,
	      ARG(CHAR_DATA), ch, ARG(int), silver)
DECLARE_PROC2(wait_state,
	      ARG(CHAR_DATA), ch, ARG(int), ws)
DECLARE_FUN2(bool, is_affected,
	     ARG(CHAR_DATA), ch, ARG(int), aff)
DECLARE_FUN1(int, obj_wear_loc,
	     ARG(OBJ_DATA), obj)
DECLARE_FUN1(bool, is_npc,
	     ARG(CHAR_DATA), ch)
DECLARE_FUN2(int, umin,
	     ARG(int), i1, ARG(int), i2)
DECLARE_FUN2(int, umax,
	     ARG(int), i1, ARG(int), i2)
DECLARE_FUN1(int, char_position,
	     ARG(CHAR_DATA), ch)
DECLARE_FUN1(bool, is_wanted,
	     ARG(CHAR_DATA), ch)
DECLARE_FUN1(bool, is_awake,
	     ARG(CHAR_DATA), ch)
DECLARE_FUN1(int, obj_item_type,
	     ARG(OBJ_DATA), obj)
DECLARE_FUN2(bool, weapon_is,
	     ARG(OBJ_DATA), obj, ARG(int), wclass)
DECLARE_FUN1(cchar_t, char_clan,
	     ARG(CHAR_DATA), ch)
DECLARE_FUN0(int, time_hour)
DECLARE_FUN1(CHAR_DATA, get_random_fighting,
	     ARG(CHAR_DATA), ch)
DECLARE_FUN1(int, char_gold,
	     ARG(CHAR_DATA), ch)
DECLARE_FUN1(int, char_silver,
	     ARG(CHAR_DATA), ch)
DECLARE_FUN1(cchar_t, char_race,
	     ARG(CHAR_DATA), ch)
DECLARE_FUN1(int, obj_vnum,
	     ARG(OBJ_DATA), obj)
DECLARE_FUN1(int, char_size,
	     ARG(CHAR_DATA), ch)
DECLARE_FUN1(int, char_quest_time,
	     ARG(CHAR_DATA), ch)
DECLARE_FUN2(bool, can_wear,
	     ARG(OBJ_DATA), obj, ARG(int), wear)
DECLARE_FUN1(int, obj_cost,
	     ARG(OBJ_DATA), obj)
DECLARE_FUN1(int, char_vnum,
	     ARG(CHAR_DATA), ch)
DECLARE_FUN1(cchar_t, char_hometown,
	     ARG(CHAR_DATA), ch)
DECLARE_FUN2(bool, is_owner_name,
	     ARG(OBJ_DATA), obj, ARG(cchar_t), name)
DECLARE_FUN1(int, char_ethos,
	     ARG(CHAR_DATA), ch)
DECLARE_FUN1(CHAR_DATA, obj_carried_by,
	     ARG(OBJ_DATA), obj)
DECLARE_FUN1(ROOM_INDEX_DATA, obj_room,
	     ARG(OBJ_DATA), obj)
DECLARE_FUN1(OBJ_DATA, obj_in,
	     ARG(OBJ_DATA), obj)
DECLARE_FUN1(int, room_vnum,
	     ARG(ROOM_INDEX_DATA), room)
DECLARE_FUN1(bool, is_pumped,
	     ARG(CHAR_DATA), ch)
DECLARE_FUN2(bool, is_mount,
	     ARG(CHAR_DATA), ch, ARG(CHAR_DATA), mob)
DECLARE_FUN2(bool, is_act,
	     ARG(CHAR_DATA), ch, ARG(int), act_flags)

__MODULE_END_DECL

#endif /* _MPC_DYNAFUN_H_ */
