/*-
 * Copyright (c) 1999 fjoe <fjoe@iclub.nsu.ru>
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
 * $Id: db_cmd.c,v 1.2 1999-10-06 09:56:15 fjoe Exp $
 */

#include <stdio.h>
#include "merc.h"
#include "db.h"

DECLARE_DBLOAD_FUN(load_cmd);

DBFUN dbfun_cmd[] =
{
	{ "CMD",	load_cmd	},
	{ NULL }
};

DBDATA db_cmd = { dbfun_cmd };

DBLOAD_FUN(load_cmd)
{
	cmd_t *cmd = cmd_new();

	for (;;) {
		char *word = feof(fp) ? "End" : fread_word(fp);
		bool fMatch = FALSE;

		switch(UPPER(word[0])) {
		case 'C':
			KEY("class", cmd->cmd_class,
			    fread_fword(cmd_classes, fp));
			break;
		case 'D':
			SKEY("dofun", cmd->dofun_name, fread_string(fp));
			break;
		case 'E':
			if (!str_cmp(word, "end")) {
				if (IS_NULLSTR(cmd->name)) {
					db_error("load_cmd", "NULL name");
					cmd_free(cmd);
					commands.nused--;
				}
				return;
			}
			break;
		case 'F':
			KEY("flags", cmd->cmd_flags,
			    fread_fstring(cmd_flags, fp));
			break;
		case 'L':
			KEY("log", cmd->cmd_log,
			    fread_fword(cmd_logtypes, fp));
			break;
		case 'M':
			KEY("min_pos", cmd->min_pos,
			    fread_fword(position_table, fp));
			KEY("min_level", cmd->min_level,
			    fread_fword(level_table, fp));
			break;
		case 'N':
			SKEY("name", cmd->name, fread_string(fp));
			break;
		}

		if (!fMatch) 
			db_error("load_cmd", "%s: Unknown keyword", word);
	}
}
