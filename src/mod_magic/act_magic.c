/*-
 * Copyright (c) 1999 SoG Development Team
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
 * $Id: act_magic.c,v 1.41 2001-09-01 19:08:27 fjoe Exp $
 */

#include <stdio.h>
#include <string.h>

#include <merc.h>

#include <sog.h>

#include <magic.h>
#include "magic_impl.h"

DECLARE_DO_FUN(do_cast);
DECLARE_DO_FUN(do_pray);

DO_FUN(do_cast, ch, argument)
{
	cpdata_t cp;
	sptarget_t spt;

	if (get_cpdata(ch, argument, ST_SPELL, &cp) < 0)
		return;

	if (find_sptarget(ch, cp.sk, &spt) < 0)
		return;

	cast_spell(ch, &cp, &spt);
}

DO_FUN(do_pray, ch, argument)
{
	cpdata_t cp;
	sptarget_t spt;

	if (get_cpdata(ch, argument, ST_PRAYER, &cp) < 0)
		return;

	if (find_sptarget(ch, cp.sk, &spt) < 0)
		return;

	cast_spell(ch, &cp, &spt);
}
