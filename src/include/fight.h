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
 * $Id: fight.h,v 1.27 2000-06-02 16:40:40 fjoe Exp $
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
#include "dynafun_decl.h"

__MODULE_START_DECL

DECLARE_FUN4(void, one_hit,
	     CHAR_DATA, ch, CHAR_DATA, victim, cchar_t, dt, bool, secondary)
DECLARE_FUN2(void, handle_death,
	     CHAR_DATA, ch, CHAR_DATA, victim)
DECLARE_FUN3(void, multi_hit,
	     CHAR_DATA, ch, CHAR_DATA, victim, cchar_t, dt)
DECLARE_FUN6(bool, damage,
	     CHAR_DATA, ch, CHAR_DATA, victim, int, dam,
	     cchar_t, dt, int, class, int, dam_flags)
DECLARE_FUN1(void, update_pos,
	     CHAR_DATA, victim)
DECLARE_FUN2(void, set_fighting,
	     CHAR_DATA, ch, CHAR_DATA, victim)
DECLARE_FUN2(void, stop_fighting,
	     CHAR_DATA, ch, bool, fBoth)
DECLARE_FUN2(OBJ_DATA, raw_kill,
	     CHAR_DATA, ch, CHAR_DATA, victim)
DECLARE_FUN1(cchar_t, vs_dam_alias,
	     int, dam)
DECLARE_FUN1(cchar_t, vp_dam_alias,
	     int, dam)
DECLARE_FUN4(bool, check_obj_dodge,
	     CHAR_DATA, ch, CHAR_DATA, victim,
	     OBJ_DATA, obj, int, bonus)
DECLARE_FUN2(bool, is_safe,
	     CHAR_DATA, ch, CHAR_DATA, victim)
DECLARE_FUN2(bool, is_safe_nomessage,
	     CHAR_DATA, ch, CHAR_DATA, victim)
DECLARE_FUN3(bool, is_safe_spell,
	     CHAR_DATA, ch, CHAR_DATA, victim, bool, area)
DECLARE_FUN2(bool, is_safe_rspell,
	     AFFECT_DATA, af, CHAR_DATA, victim)
DECLARE_FUN2(int, get_dam_class,
	     CHAR_DATA, ch, OBJ_DATA, wield)
DECLARE_FUN2(CHAR_DATA, check_guard,
	     CHAR_DATA, ch, CHAR_DATA, mob)
DECLARE_FUN2(bool, in_PK,
	     CHAR_DATA, ch, CHAR_DATA, victim)

__MODULE_END_DECL

#endif
