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
 * $Id: init_quest.c,v 1.6 2001-08-13 18:23:52 fjoe Exp $
 */

#include <stdio.h>
#include <stdarg.h>

#include <typedef.h>
#include <memalloc.h>
#include <varr.h>
#include <hash.h>
#include <cmd.h>

#include <update.h>

#include <module.h>
#define MODULE_INIT MOD_QUEST
#include <quest.h>

#include "chquest_impl.h"

DECLARE_MODINIT_FUN(_module_load);
DECLARE_MODINIT_FUN(_module_unload);

MODINIT_FUN(_module_load, m)
{
	varr_foreach(&commands, cmd_load_cb, MODULE, m);
	dynafun_tab_register(__mod_tab(MODULE), m);
	uhandler_load(m->name);
	chquest_init();
	return 0;
}

MODINIT_FUN(_module_unload, m)
{
	uhandler_unload(m->name);
	dynafun_tab_unregister(__mod_tab(MODULE));
	varr_foreach(&commands, cmd_unload_cb, MODULE);
	return 0;
}
