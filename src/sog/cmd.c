/*-
 * Copyright (c) 1998 fjoe <fjoe@iclub.nsu.ru>
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
 * $Id: cmd.c,v 1.3 1999-06-24 16:33:13 fjoe Exp $
 */

#include <stdarg.h>
#include <stdio.h>

#include "typedef.h"
#include "const.h"
#include "str.h"
#include "varr.h"
#include "cmd.h"
#include "log.h"

varr commands = { sizeof(cmd_t), 16 };

cmd_t *cmd_lookup(const char *name)
{
	int i;

	for (i = 0; i < commands.nused; i++) {
		cmd_t *cmd = VARR_GET(&commands, i);
		if (!str_cmp(cmd->name, name))
			return cmd;
	}

	return NULL;
}

void dofun(const char *name, CHAR_DATA *ch, const char *fmt, ...)
{
	cmd_t *cmd;
	char buf[MAX_STRING_LENGTH];
	va_list ap;

	if ((cmd = cmd_lookup(name)) == NULL) {
		bug("dofun: %s: unknown dofun", name);
		return;
	}

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	cmd->do_fun(ch, buf);
}

