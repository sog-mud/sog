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
 * $Id: olc_cmd.c,v 1.28 2002-11-21 09:40:54 fjoe Exp $
 */

#include "olc.h"

#include <module.h>

#define EDIT_CMD(ch, cmd)	(cmd = (cmd_t *) ch->desc->pEdit)

DECLARE_OLC_FUN(cmded_create		);
DECLARE_OLC_FUN(cmded_edit		);
DECLARE_OLC_FUN(cmded_save		);
DECLARE_OLC_FUN(cmded_touch		);
DECLARE_OLC_FUN(cmded_show		);
DECLARE_OLC_FUN(cmded_list		);

DECLARE_OLC_FUN(cmded_name		);
DECLARE_OLC_FUN(cmded_aliases		);
DECLARE_OLC_FUN(cmded_minpos		);
DECLARE_OLC_FUN(cmded_minlevel		);
DECLARE_OLC_FUN(cmded_dofun		);
DECLARE_OLC_FUN(cmded_flags		);
DECLARE_OLC_FUN(cmded_log		);
DECLARE_OLC_FUN(cmded_module		);
DECLARE_OLC_FUN(cmded_move		);
DECLARE_OLC_FUN(cmded_delete		);

static DECLARE_VALIDATE_FUN(validate_cmd_name);
static DECLARE_VALIDATE_FUN(validate_cmd_alias);

olc_cmd_t olc_cmds_cmd[] =
{
	{ "create",	cmded_create,	NULL,		NULL		},
	{ "edit",	cmded_edit,	NULL,		NULL		},
	{ "",		cmded_save,	NULL,		NULL		},
	{ "touch",	cmded_touch,	NULL,		NULL		},
	{ "show",	cmded_show,	NULL,		NULL		},
	{ "list",	cmded_list,	NULL,		NULL		},

	{ "name",	cmded_name,	validate_cmd_name, NULL		},
	{ "aliases",	cmded_aliases,	validate_cmd_alias, NULL	},
	{ "minpos",	cmded_minpos,	NULL,		position_table	},
	{ "minlevel",	cmded_minlevel, NULL,		level_table	},
	{ "dofun",	cmded_dofun,	validate_funname, NULL		},
	{ "flags",	cmded_flags,	NULL,		cmd_flags	},
	{ "log",	cmded_log,	NULL,		cmd_logtypes	},
	{ "module",	cmded_module,	NULL,		module_names	},
	{ "move",	cmded_move,	NULL,		NULL		},

	{ "delete_cm",	olced_spell_out, NULL,		NULL		},
	{ "delete_cmd",	cmded_delete,	NULL,		NULL		},

	{ "commands",	show_commands,	NULL,		NULL		},
	{ "version",	show_version,	NULL,		NULL		},

	{ NULL, NULL, NULL, NULL }
};

static void check_shadow(CHAR_DATA *ch, const char *name);

OLC_FUN(cmded_create)
{
	cmd_t *cmnd;
	char arg[MAX_STRING_LENGTH];

	if (PC(ch)->security < SECURITY_CMDS) {
		act_char("CmdEd: Insufficient security for creating commands", ch);
		return FALSE;
	}

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0')
		OLC_ERROR("'OLC CREATE'");

	if ((cmnd = cmd_lookup(arg))) {
		act_puts("CmdEd: $t: already exists.",
			 ch, cmnd->name, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	check_shadow(ch, arg);

	cmnd		= varr_enew(&commands);
	cmnd->name	= str_dup(arg);

	ch->desc->pEdit	= (void *) cmnd;
	OLCED(ch)	= olced_lookup(ED_CMD);
	SET_BIT(changed_flags, CF_CMD);
	act_char("Command created.", ch);
	return FALSE;
}

OLC_FUN(cmded_edit)
{
	cmd_t *cmnd;
	char arg[MAX_STRING_LENGTH];

	if (PC(ch)->security < SECURITY_CMDS) {
		act_char("CmdEd: Insufficient security.", ch);
		return FALSE;
	}

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0')
		OLC_ERROR("'OLC EDIT'");

	if (!(cmnd = cmd_search(arg))) {
		act_puts("CmdEd: $t: No such command.",
			 ch, arg, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	ch->desc->pEdit	= cmnd;
	OLCED(ch)	= olced_lookup(ED_CMD);
	return FALSE;
}

OLC_FUN(cmded_save)
{
	FILE *fp;
	cmd_t *cmnd;

	if (!IS_SET(changed_flags, CF_CMD)) {
		olc_printf(ch, "Commands are not changed.");
		return FALSE;
	}

	fp = olc_fopen(ETC_PATH, CMD_CONF, ch, SECURITY_CMDS);
	if (fp == NULL)
		return FALSE;

	C_FOREACH(cmnd, &commands) {
		fprintf(fp, "#CMD\n");
		fwrite_string(fp, "name", cmnd->name);
		fwrite_string(fp, "aliases", cmnd->aliases);
		fwrite_string(fp, "dofun", cmnd->dofun_name);
		fprintf(fp, "min_pos %s\n",
			flag_string(position_table, cmnd->min_pos));
		if (cmnd->min_level) {
			fprintf(fp, "min_level %s\n",
				fix_word(flag_istring(level_table, cmnd->min_level)));
		}

		if (cmnd->cmd_log != LOG_NORMAL) {
			fprintf(fp, "log %s\n",
				flag_string(cmd_logtypes, cmnd->cmd_log));
		}

		if (cmnd->cmd_flags) {
			fprintf(fp, "flags %s~\n",
				flag_string(cmd_flags, cmnd->cmd_flags));
		}

		if (cmnd->cmd_mod != MOD_DOFUN) {
			fprintf(fp, "module %s\n",
				flag_string(module_names, cmnd->cmd_mod));
		}

		fprintf(fp, "end\n\n");
	}

	fprintf(fp, "#$\n");
	fclose(fp);

	REMOVE_BIT(changed_flags, CF_CMD);
	olc_printf(ch, "Commands saved.");
	return FALSE;
}

OLC_FUN(cmded_touch)
{
	SET_BIT(changed_flags, CF_CMD);
	return FALSE;
}

OLC_FUN(cmded_show)
{
	char arg[MAX_STRING_LENGTH];
	BUFFER *output;
	cmd_t *cmnd;

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		if (IS_EDIT(ch, ED_CMD))
			EDIT_CMD(ch, cmnd);
		else
			OLC_ERROR("'OLC ASHOW'");
	} else {
		if (!(cmnd = cmd_search(arg))) {
			act_puts("CmdEd: $t: No such command.",
				 ch, arg, NULL,
				 TO_CHAR | ACT_NOTRANS, POS_DEAD);
			return FALSE;
		}
	}

	output = buf_new(0);

	buf_printf(output, BUF_END,
		   "[%3d] Name [%s]\n"
		   "Aliases    [%s]\n"
		   "Dofun      [%s]\n",
		   varr_index(&commands, cmnd),
		   cmnd->name,
		   cmnd->aliases,
		   cmnd->dofun_name);
	buf_printf(output, BUF_END,
		   "Min pos    [%s]\n",
		   flag_string(position_table, cmnd->min_pos));
	if (cmnd->min_level)
		buf_printf(output, BUF_END, "Min level  [%s]\n",
			flag_istring(level_table, cmnd->min_level));
	if (cmnd->cmd_log)
		buf_printf(output, BUF_END, "Log        [%s]\n",
			flag_string(cmd_logtypes, cmnd->cmd_log));
	if (cmnd->cmd_log)
		buf_printf(output, BUF_END, "Module     [%s]\n",
			flag_string(module_names, cmnd->cmd_mod));
	if (cmnd->cmd_flags)
		buf_printf(output, BUF_END, "Flags      [%s]\n",
			flag_string(cmd_flags, cmnd->cmd_flags));

	page_to_char(buf_string(output), ch);
	buf_free(output);

	return FALSE;
}

OLC_FUN(cmded_list)
{
	int col = 0;
	char arg[MAX_STRING_LENGTH];
	BUFFER *output;
	cmd_t *cmnd;

	one_argument(argument, arg, sizeof(arg));
	output = buf_new(0);

	C_FOREACH(cmnd, &commands) {
		if (arg[0] && str_prefix(arg, cmnd->name))
			continue;

		buf_printf(output, BUF_END, "[%3d] %-12s",
		    varr_index(&commands, cmnd), cmnd->name);
		if (++col % 4 == 0)
			buf_append(output, "\n");
	}

	if (col % 4)
		buf_append(output, "\n");

	page_to_char(buf_string(output), ch);
	buf_free(output);

	return FALSE;
}

OLC_FUN(cmded_name)
{
	cmd_t *cmnd;
	EDIT_CMD(ch, cmnd);
	return olced_str(ch, argument, cmd, &cmnd->name);
}

OLC_FUN(cmded_aliases)
{
	cmd_t *cmnd;
	EDIT_CMD(ch, cmnd);
	return olced_name(ch, argument, cmd, &cmnd->aliases);
}

OLC_FUN(cmded_dofun)
{
	cmd_t *cmnd;
	EDIT_CMD(ch, cmnd);
	return olced_str(ch, argument, cmd, &cmnd->dofun_name);
}

OLC_FUN(cmded_minpos)
{
	cmd_t *cmnd;
	EDIT_CMD(ch, cmnd);
	return olced_flag(ch, argument, cmd, &cmnd->min_pos);
}

OLC_FUN(cmded_flags)
{
	cmd_t *cmnd;
	EDIT_CMD(ch, cmnd);
	return olced_flag(ch, argument, cmd, &cmnd->cmd_flags);
}

OLC_FUN(cmded_log)
{
	cmd_t *cmnd;
	EDIT_CMD(ch, cmnd);
	return olced_flag(ch, argument, cmd, &cmnd->cmd_log);
}

OLC_FUN(cmded_module)
{
	cmd_t *cmnd;
	EDIT_CMD(ch, cmnd);
	return olced_flag(ch, argument, cmd, &cmnd->cmd_mod);
}

OLC_FUN(cmded_minlevel)
{
	cmd_t *cmnd;
	EDIT_CMD(ch, cmnd);
	return olced_ival(ch, argument, cmd, &cmnd->min_level);
}

OLC_FUN(cmded_move)
{
	cmd_t *cmnd;
	cmd_t ncmd;
	char arg[MAX_INPUT_LENGTH];
	int num, num2;

	EDIT_CMD(ch, cmnd);

	if (olced_busy(ch, ED_CMD, NULL, NULL))
		return FALSE;

	one_argument(argument, arg, sizeof(arg));
	if (!is_number(arg))
		OLC_ERROR("'OLC CMDS'");

	if ((num = atoi(arg)) >= (int) c_size(&commands))
		num = c_size(&commands) - 1;
	if (num < 0) {
		act_char("CmdEd: move: num should be >= 0.", ch);
		return FALSE;
	}

	num2 = varr_index(&commands, cmnd);
	if (num2 == num) {
		act_char("CmdEd: move: already there.", ch);
		return FALSE;
	}

	ncmd = *cmnd;
	varr_edelete_nd(&commands, cmnd);
	cmnd = (cmd_t *) varr_insert(&commands, num);
	*cmnd = ncmd;

	ch->desc->pEdit	= cmnd;
	act_puts("CmdEd: '$T' moved to $j position.",
		 ch, (const void *) varr_index(&commands, cmnd), cmnd->name,
		 TO_CHAR | ACT_NOTRANS, POS_DEAD);
	return TRUE;
}

OLC_FUN(cmded_delete)
{
	cmd_t *cmnd;
	EDIT_CMD(ch, cmnd);

	if (olced_busy(ch, ED_CMD, NULL, NULL))
		return FALSE;

	varr_edelete(&commands, cmnd);
	edit_done(ch->desc);
	return TRUE;
}

static VALIDATE_FUN(validate_cmd_name)
{
	const char *name = (const char*) arg;
	cmd_t *cmnd, *cmnd2;
	EDIT_CMD(ch, cmnd);

	if (strpbrk(name, " \t")) {
		act_puts("CmdEd: $t: illegal character in command name.",
			 ch, name, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	if ((cmnd2 = cmd_lookup(name))
	&&  cmnd2 != cmnd) {
		act_puts("CmdEd: $t: duplicate command name.",
			 ch, name, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	check_shadow(ch, name);
	return TRUE;
}

static VALIDATE_FUN(validate_cmd_alias)
{
	const char *alias = (const char*) arg;
	cmd_t *cmnd, *cmnd2;
	EDIT_CMD(ch, cmnd);

	if (strpbrk(alias, " \t")) {
		act_puts("CmdEd: $t: illegal character in alias name.",
			 ch, alias, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	if ((cmnd2 = cmd_lookup(alias))) {
		act_puts("CmdEd: $t: command with such name already exists.",
			 ch, alias, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	C_FOREACH(cmnd2, &commands) {
		if (is_name_strict(alias, cmnd2->aliases)) {
			act_puts("CmdEd: $t: command '$T' already have such alias.",
			    ch, alias, cmnd2->name,
			    TO_CHAR | ACT_NOTRANS, POS_DEAD);
			return FALSE;
		}
	}

	check_shadow(ch, alias);
	return TRUE;
}

static void
check_shadow(CHAR_DATA *ch, const char *name)
{
	BUFFER *output;
	social_t *soc;
	bool found = FALSE;

	output = buf_new(0);

	C_FOREACH(soc, &socials) {
		if (str_prefix(soc->name, name))
			continue;

		if (!found) {
			buf_append(output,
			    "The following social(s) will be shadowed:\n");
		}

		found = TRUE;
		buf_printf(output, BUF_END, "  [%s]\n", soc->name);
	}

	if (!found)
		buf_append(output, "This name will not shadow anything.\n");

	page_to_char(buf_string(output), ch);
	buf_free(output);
}
