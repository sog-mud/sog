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
 * $Id: fight.h,v 1.31 2001-07-29 20:14:28 fjoe Exp $
 */

#ifndef _FIGHT_H_
#define _FIGHT_H_

/*
 * dam_flags
 */
#define DAMF_NONE	(0)
#define DAMF_SHOW	(A)	/* show dam message */
#define DAMF_SECOND	(B)	/* damage inflicted by second weapon */
#define DAMF_HIT	(C)	/* damage by hit */
#define DAMF_HUNGER	(D)	/* damage by hunger */
#define DAMF_NOREDUCE	(E)	/* damage should not be reduced */
#define DAMF_THIRST	(F)	/* damage by thirst */
#define DAMF_LIGHT_V	(G)	/* light in room */
#define DAMF_TRAP_ROOM	(H)	/* room trap damage */

#undef MODULE_NAME
#define MODULE_NAME MOD_FIGHT
#include <dynafun_decl.h>

__MODULE_START_DECL

DECLARE_PROC4(one_hit,
	      ARG(CHAR_DATA), ch, ARG(CHAR_DATA), victim, ARG(cchar_t), dt,
	      ARG(bool), secondary)
DECLARE_PROC2(handle_death,
	      ARG(CHAR_DATA), ch, ARG(CHAR_DATA), victim)
DECLARE_PROC3(multi_hit,
	      ARG(CHAR_DATA), ch, ARG(CHAR_DATA), victim, ARG(cchar_t), dt)
DECLARE_FUN6(bool, damage,
	     ARG(CHAR_DATA), ch, ARG(CHAR_DATA), victim, ARG(int), dam,
	     ARG(cchar_t), dt, ARG(int), class, ARG(int), dam_flags)
DECLARE_PROC1(update_pos,
	      ARG(CHAR_DATA), victim)
DECLARE_PROC2(set_fighting,
	      ARG(CHAR_DATA), ch, ARG(CHAR_DATA), victim)
DECLARE_PROC2(stop_fighting,
	      ARG(CHAR_DATA), ch, ARG(bool), fBoth)
DECLARE_FUN2(OBJ_DATA, raw_kill,
	     ARG(CHAR_DATA), ch, ARG(CHAR_DATA), victim)
DECLARE_FUN1(cchar_t, vs_dam_alias,
	     ARG(int), dam)
DECLARE_FUN1(cchar_t, vp_dam_alias,
	     ARG(int), dam)
DECLARE_FUN4(bool, check_obj_dodge,
	     ARG(CHAR_DATA), ch, ARG(CHAR_DATA), victim,
	     ARG(OBJ_DATA), obj, ARG(int), bonus)
DECLARE_FUN2(bool, is_safe,
	     ARG(CHAR_DATA), ch, ARG(CHAR_DATA), victim)
DECLARE_FUN2(bool, is_safe_nomessage,
	     ARG(CHAR_DATA), ch, ARG(CHAR_DATA), victim)
DECLARE_FUN3(bool, is_safe_spell,
	     ARG(CHAR_DATA), ch, ARG(CHAR_DATA), victim, ARG(bool), area)
DECLARE_FUN2(bool, is_safe_rspell,
	     ARG(AFFECT_DATA), af, ARG(CHAR_DATA), victim)
DECLARE_FUN2(int, get_dam_class,
	     ARG(CHAR_DATA), ch, ARG(OBJ_DATA), wield)
DECLARE_FUN2(CHAR_DATA, check_guard,
	     ARG(CHAR_DATA), ch, ARG(CHAR_DATA), mob)
DECLARE_FUN2(bool, in_PK,
	     ARG(CHAR_DATA), ch, ARG(CHAR_DATA), victim)

DECLARE_PROC4(focus_positive_energy,
	      ARG(CHAR_DATA), ch, ARG(CHAR_DATA), victim, ARG(cchar_t), sn,
	      ARG(int), amount)
DECLARE_PROC4(focus_negative_energy,
	      ARG(CHAR_DATA), ch, ARG(CHAR_DATA), victim, ARG(cchar_t), sn,
	      ARG(int), amount)

__MODULE_END_DECL

#endif
