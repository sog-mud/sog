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
 * $Id: init_magic.c,v 1.19 2001-12-03 22:28:26 fjoe Exp $
 */

#include <stdio.h>
#include <dlfcn.h>

#include <merc.h>

#include <module.h>
#define MODULE_INIT MOD_MAGIC
#include <magic.h>

DECLARE_MODINIT_FUN(_module_load);
DECLARE_MODINIT_FUN(_module_unload);

MODINIT_FUN(_module_load, m)
{
	cmd_t *cmd;
	skill_t *sk;

	C_FOREACH(cmd, &commands)
		cmd_load(cmd, MODULE, m);
	C_FOREACH(sk, &skills) {
		if (sk->skill_type == ST_SPELL
		||  sk->skill_type == ST_PRAYER) {
			sk->fun = dlsym(m->dlh, sk->fun_name);
			if (sk->fun == NULL) {
				log(LOG_INFO, "_module_load(magic): %s",
				    dlerror());
			}
		}
	}
	dynafun_tab_register(__mod_tab(MODULE), m);
	return 0;
}

MODINIT_FUN(_module_unload, m)
{
	cmd_t *cmd;
	skill_t *sk;

	dynafun_tab_unregister(__mod_tab(MODULE));
	C_FOREACH(sk, &skills) {
		if (sk->skill_type == ST_SPELL
		||  sk->skill_type == ST_PRAYER)
			sk->fun = NULL;
	}
	C_FOREACH(cmd, &commands)
		cmd_unload(cmd, MODULE);
	return 0;
}
