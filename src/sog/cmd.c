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
 * $Id: cmd.c,v 1.13 2000-04-16 09:21:53 fjoe Exp $
 */

#include <stdarg.h>
#include <stdio.h>
#include <dlfcn.h>

#include "typedef.h"
#include "str.h"
#include "varr.h"
#include "cmd.h"
#include "log.h"
#include "module.h"

varr commands;

void
cmd_init(cmd_t *cmd)
{
	cmd->name = str_empty;
	cmd->dofun_name = str_empty;
	cmd->min_pos = 0;
	cmd->min_level = 0;
	cmd->cmd_log = LOG_NORMAL;
	cmd->cmd_flags = 0;
	cmd->cmd_class = CC_ORDINARY;
	cmd->do_fun = NULL;
}

void
cmd_destroy(cmd_t *cmd)
{
	free_string(cmd->name);
	free_string(cmd->dofun_name);
}

void *
cmd_load_cb(void *p, va_list ap)
{
	cmd_t *cmd = (cmd_t *) p;

	int cmd_class = va_arg(ap, int);
	module_t *m = va_arg(ap, module_t*);

	if (cmd_class < 0
	||  cmd_class == cmd->cmd_class) {
		cmd->do_fun = dlsym(m->dlh, cmd->dofun_name);
		if (cmd->do_fun == NULL)
			log(LOG_INFO, "cmd_load: %s", dlerror());
	}

	return NULL;
}

void *
cmd_unload_cb(void *p, va_list ap)
{
	cmd_t *cmd = (cmd_t *) p;

	int cmd_class = va_arg(ap, int);

	if (cmd_class < 0
	||  cmd_class == cmd->cmd_class)
		cmd->do_fun = NULL;

	return NULL;
}

void
dofun(const char *name, CHAR_DATA *ch, const char *fmt, ...)
{
	cmd_t *cmd;
	char buf[MAX_STRING_LENGTH];
	va_list ap;

	if ((cmd = cmd_lookup(name)) == NULL) {
		log(LOG_ERROR, "dofun: %s: unknown dofun", name);
		return;
	}

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	if (cmd->do_fun == NULL)
		log(LOG_ERROR, "dofun: %s: NULL do_fun", cmd->name);
	else
		cmd->do_fun(ch, buf);
}

