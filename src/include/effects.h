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
 * $Id: effects.h,v 1.5 2001-07-08 17:18:43 fjoe Exp $
 */

#ifndef _EFFECTS_H_
#define _EFFECTS_H_

#undef MODULE_NAME
#define MODULE_NAME MOD_EFFECTS
#include <dynafun_decl.h>

__MODULE_START_DECL

DECLARE_PROC3(acid_effect,
	      pvoid_t, vo, int, level, int, dam)
DECLARE_PROC3(cold_effect,
	      pvoid_t, vo, int, level, int, dam)
DECLARE_PROC3(fire_effect,
	      pvoid_t, vo, int, level, int, dam)
DECLARE_PROC3(poison_effect,
	      pvoid_t, vo, int, level, int, dam)
DECLARE_PROC3(shock_effect,
	      pvoid_t, vo, int, level, int, dam)
DECLARE_PROC3(sand_effect,
	      pvoid_t, vo, int, level, int, dam)
DECLARE_PROC3(scream_effect,
	      pvoid_t, vo, int, level, int, dam)

__MODULE_END_DECL

#endif
