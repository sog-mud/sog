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
 * $Id: mpc_dynafun.h,v 1.3 2001-09-02 16:22:00 fjoe Exp $
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
DECLARE_FUN1(int, level,
	     ARG(CHAR_DATA), ch)
DECLARE_FUN4(int, spclass_count,
	     ARG(CHAR_DATA), ch, ARG(cchar_t), spn,
	     ARG(cchar_t), spn_add, ARG(cchar_t), spn_rm)
DECLARE_PROC2(mob_interpret,
	      ARG(CHAR_DATA), ch, ARG(cchar_t), argument)
DECLARE_FUN1(bool, is_immortal,
	     ARG(CHAR_DATA), ch)
DECLARE_FUN1(int, char_sex,
	     ARG(CHAR_DATA), ch)
DECLARE_FUN2(OBJ_DATA, load_obj,
	     ARG(CHAR_DATA), ch, ARG(int), vnum)

__MODULE_END_DECL

#endif /* _MPC_DYNAFUN_H_ */
