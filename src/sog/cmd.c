/*-
 * Copyright (c) 1998 SoG Development Team
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
 * $Id: cmd.c,v 1.30 2003-10-10 16:15:10 fjoe Exp $
 */

#include <stdio.h>
#include <dlfcn.h>

#include <typedef.h>
#include <str.h>
#include <varr.h>
#include <container.h>
#include <cmd.h>
#include <log.h>
#include <module.h>

varr commands;

static void
cmd_init(cmd_t *cmd)
{
	cmd->name = str_empty;
	cmd->aliases = str_empty;
	cmd->dofun_name = str_empty;
	cmd->sn = str_empty;
	cmd->min_pos = 0;
	cmd->min_level = 0;
	cmd->cmd_log = LOG_NORMAL;
	cmd->cmd_flags = 0;
	cmd->cmd_mod = MOD_DOFUN;
	cmd->do_fun = NULL;
}

static void
cmd_destroy(cmd_t *cmd)
{
	free_string(cmd->name);
	free_string(cmd->aliases);
	free_string(cmd->dofun_name);
	free_string(cmd->sn);
}

varr_info_t c_info_commands =
{
	&varr_ops,

	(e_init_t) cmd_init,
	(e_destroy_t) cmd_destroy,

	sizeof(cmd_t), 16
};

void
cmd_mod_load(module_t *m)
{
	cmd_t *cmd;

	C_FOREACH(cmd, &commands) {
		if (m->mod_id != cmd->cmd_mod)
			continue;

		cmd->do_fun = dlsym(m->dlh, cmd->dofun_name);
		if (cmd->do_fun == NULL)
			printlog(LOG_INFO, "cmd_load: %s", dlerror());
	}
}

void
cmd_mod_unload(module_t *m)
{
	cmd_t *cmd;

	C_FOREACH(cmd, &commands) {
		if (m->mod_id == cmd->cmd_mod)
			cmd->do_fun = NULL;
	}
}

void
dofun(const char *name, CHAR_DATA *ch, const char *argument)
{
	cmd_t *cmd;

	if ((cmd = cmd_lookup(name)) == NULL) {
		printlog(LOG_BUG, "dofun: %s: unknown dofun", name);
		return;
	}

	if (cmd->do_fun == NULL)
		printlog(LOG_BUG, "dofun: %s: NULL do_fun", cmd->name);
	else
		cmd->do_fun(ch, argument);
}
