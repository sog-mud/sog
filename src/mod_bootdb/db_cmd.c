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
 * $Id: db_cmd.c,v 1.20 2003-09-30 00:31:03 fjoe Exp $
 */

#include <stdio.h>

#include <merc.h>
#include <db.h>
#include <rwfile.h>

DECLARE_DBLOAD_FUN(load_cmd);

DECLARE_DBINIT_FUN(init_cmds);

DBFUN dbfun_cmd[] =
{
	{ "CMD",	load_cmd,	NULL	},		// notrans
	{ NULL, NULL, NULL }
};

DBDATA db_cmd = { dbfun_cmd, init_cmds, 0 };

DBINIT_FUN(init_cmds)
{
	if (!DBDATA_VALID(dbdata))
		c_init(&commands, &c_info_commands);
}

DBLOAD_FUN(load_cmd)
{
	cmd_t *cmd = varr_enew(&commands);

	for (;;) {
		bool fMatch = FALSE;

		fread_keyword(fp);
		switch(rfile_tokfl(fp)) {
		case 'A':
			SKEY("aliases", cmd->aliases, fread_string(fp));
			break;

		case 'D':
			SKEY("dofun", cmd->dofun_name, fread_string(fp));
			break;

		case 'E':
			if (IS_TOKEN(fp, "end")) {
				if (IS_NULLSTR(cmd->name)) {
					printlog(LOG_ERROR, "load_cmd: NULL name");
					varr_edelete(&commands, cmd);
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
			KEY("module", cmd->cmd_mod,
			    fread_fword(module_names, fp));
			break;

		case 'N':
			SKEY("name", cmd->name, fread_string(fp));
			break;

		case 'S':
			SKEY("skill", cmd->sn, fread_string(fp));
			break;
		}

		if (!fMatch) {
			printlog(LOG_ERROR, "%s: %s: Unknown keyword",
			    __FUNCTION__, rfile_tok(fp));
			fread_to_eol(fp);
		}
	}
}
