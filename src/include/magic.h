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
 * $Id: magic.h,v 1.2 2000-06-02 16:40:40 fjoe Exp $
 */

#ifndef _MAGIC_H_
#define _MAGIC_H_

#undef MODULE_NAME
#define MODULE_NAME MOD_MAGIC
#include "dynafun_decl.h"

__MODULE_START_DECL

DECLARE_FUN4(void, obj_cast_spell,
	     cchar_t, sn, int, level, CHAR_DATA, ch, void, vo)
DECLARE_FUN5(void, spellfun_call,
	     cchar_t, sn_fun, cchar_t, sn, int, level, CHAR_DATA, ch, void, vo)
DECLARE_FUN3(bool, saves_spell,
	     int, level, CHAR_DATA, victim, int, dam_class)
DECLARE_FUN3(bool, check_dispel,
	     int, dis_level, CHAR_DATA, victim, cchar_t, sn)

__MODULE_END_DECL

#undef MODULE_NAME

#endif
