/*-
 * Copyright (c) 1998 arborn <avn@org.chem.msu.su>
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
 * $Id: olc_cmd.c,v 1.3 1999-12-15 20:12:26 avn Exp $
 */

#include "olc.h"
#include "socials.h"

#define EDIT_CMD(ch, cmd)	(cmd = (cmd_t*) ch->desc->pEdit)

DECLARE_OLC_FUN(cmded_create		);
DECLARE_OLC_FUN(cmded_edit		);
DECLARE_OLC_FUN(cmded_save		);
DECLARE_OLC_FUN(cmded_touch		);
DECLARE_OLC_FUN(cmded_show		);
DECLARE_OLC_FUN(cmded_list		);

DECLARE_OLC_FUN(cmded_name		);
DECLARE_OLC_FUN(cmded_minpos		);
DECLARE_OLC_FUN(cmded_minlevel		);
DECLARE_OLC_FUN(cmded_dofun		);
DECLARE_OLC_FUN(cmded_flags		);
DECLARE_OLC_FUN(cmded_log		);
DECLARE_OLC_FUN(cmded_class		);
DECLARE_OLC_FUN(cmded_move		);

static DECLARE_VALIDATE_FUN(validate_cmd_name);

olc_cmd_t olc_cmds_cmd[] =
{
	{ "create",	cmded_create					},
	{ "edit",	cmded_edit					},
	{ "",		cmded_save					},
	{ "touch",	cmded_touch					},
	{ "show",	cmded_show					},
	{ "list",	cmded_list					},

	{ "name",	cmded_name,	validate_cmd_name 		},
	{ "minpos",	cmded_minpos,	NULL,		position_table	},
	{ "minlevel",	cmded_minlevel, NULL,		level_table	},
	{ "dofun",	cmded_dofun,	validate_funname				},
	{ "flags",	cmded_flags,	NULL,		cmd_flags	},
	{ "log",	cmded_log,	NULL,		cmd_logtypes	},
	{ "class",	cmded_class,	NULL,		cmd_classes	},
	{ "move",	cmded_move					},

	{ "commands",	show_commands					},
	{ NULL }
};

static void *save_cmd_cb(void *fp, va_list ap);
static void check_shadow(CHAR_DATA *ch, const char *name);


OLC_FUN(cmded_create)
{
	cmd_t *cmnd;
	char arg[MAX_STRING_LENGTH];

	if (PC(ch)->security < SECURITY_CMDS) {
		char_puts("CmdEd: Insufficient security for creating commands\n", ch);
		return FALSE;
	}

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0')
		OLC_ERROR("'OLC CREATE'");

	if ((cmnd = cmd_lookup(arg))) {
		char_printf(ch, "CmdEd: %s: already exists.\n", cmnd->name);
		return FALSE;
	}

	check_shadow(ch, arg);

	cmnd		= cmd_new();
	cmnd->name	= str_dup(arg);

	ch->desc->pEdit	= (void *) cmnd;
	OLCED(ch)	= olced_lookup(ED_CMD);
	SET_BIT(changed_flags, CF_CMD);
	char_puts("Command created.\n",ch);
	return FALSE;
}

OLC_FUN(cmded_edit)
{
	cmd_t *cmnd;
	char arg[MAX_STRING_LENGTH];

	if (PC(ch)->security < SECURITY_CMDS) {
		char_puts("CmdEd: Insufficient security.\n", ch);
		return FALSE;
	}

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0')
		OLC_ERROR("'OLC EDIT'");

	if (!(cmnd = cmd_lookup(arg))) {
		char_printf(ch, "CmdEd: %s: No such command.\n", arg);
		return FALSE;
	}

	ch->desc->pEdit	= cmnd;
	OLCED(ch)	= olced_lookup(ED_CMD);
	return FALSE;
}

OLC_FUN(cmded_save)
{
	FILE *fp;

	if (!IS_SET(changed_flags, CF_CMD)) {
		olc_printf(ch, "Commands are not changed.");
		return FALSE;
	}

	fp = olc_fopen(ETC_PATH, CMD_CONF, ch, SECURITY_CMDS);
	if (fp == NULL)
		return FALSE;

	varr_foreach(&commands, save_cmd_cb, fp);

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
	}
	else {
		if (!(cmnd = cmd_search(arg))) {
			char_printf(ch, "CmdEd: %s: No such command.\n", arg);
			return FALSE;
		}
	}

	output = buf_new(-1);

	buf_printf(output,
		   "[%3d]Name  [%s]\nDofun      [%s]\n",
		   varr_index(&commands, cmnd),
		   cmnd->name, cmnd->dofun_name);
	buf_printf(output,
		   "Min_pos    [%s]\n",
		   flag_string(position_table, cmnd->min_pos));
	if (cmnd->min_level)
		buf_printf(output, "Min_level  [%s]\n",
			flag_istring(level_table, cmnd->min_level));
	if (cmnd->cmd_log)
		buf_printf(output, "Log        [%s]\n",
			flag_string(cmd_logtypes, cmnd->cmd_log));
	if (cmnd->cmd_log)
		buf_printf(output, "Class      [%s]\n",
			flag_string(cmd_classes, cmnd->cmd_class));
	if (cmnd->cmd_flags)
		buf_printf(output, "Flags      [%s]\n",
			flag_string(cmd_flags, cmnd->cmd_flags));

	page_to_char(buf_string(output), ch);
	buf_free(output);

	return FALSE;
}

OLC_FUN(cmded_list)
{
	int i;
	int col = 0;
	char arg[MAX_STRING_LENGTH];
	BUFFER *output;

	one_argument(argument, arg, sizeof(arg));
	output = buf_new(-1);

	for (i = 0; i < commands.nused; i++) {
		cmd_t *cmnd = (cmd_t*) VARR_GET(&commands, i);

		if (arg[0] && str_prefix(arg, cmnd->name))
			continue;

		buf_printf(output, "[%3d] %-12s", i, cmnd->name);
		if (++col % 4 == 0)
			buf_add(output, "\n");
	}

	if (col % 4)
		buf_add(output, "\n");

	page_to_char(buf_string(output), ch);
	buf_free(output);

	return FALSE;
}

OLC_FUN(cmded_name)
{
	cmd_t *cmnd;
	EDIT_CMD(ch, cmnd);

	check_shadow(ch, argument);
	return olced_str(ch, argument, cmd, &cmnd->name);
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

OLC_FUN(cmded_class)
{
	cmd_t *cmnd;
	EDIT_CMD(ch, cmnd);
	return olced_flag(ch, argument, cmd, &cmnd->cmd_class);
}

OLC_FUN(cmded_minlevel)
{
	cmd_t *cmnd;
	EDIT_CMD(ch, cmnd);
	return olced_ival(ch, argument, cmd, &cmnd->min_level);
}

OLC_FUN(cmded_move)
{
	cmd_t *cmnd, *ncmnd;
	char arg[MAX_INPUT_LENGTH];
	int num, num2;

	EDIT_CMD(ch, cmnd);

	if (olced_busy(ch, ED_CMD, NULL, NULL))
		return FALSE;

	one_argument(argument, arg, sizeof(arg));
	if (!is_number(arg))
		OLC_ERROR("'OLC CMDS'");

	if ((num = atoi(arg)) > commands.nused)
		num = commands.nused;

	num2 = varr_index(&commands, cmnd);

	ncmnd = (cmd_t *)varr_insert(&commands, num);
	if (num <= num2)
		cmnd = (cmd_t *)varr_get(&commands, num2 + 1);

	ncmnd->name = cmnd->name;
	ncmnd->dofun_name = cmnd->dofun_name;
	ncmnd->do_fun = cmnd->do_fun;
	ncmnd->cmd_class = cmnd->cmd_class;
	ncmnd->cmd_log = cmnd->cmd_log;
	ncmnd->cmd_flags = cmnd->cmd_flags;
	ncmnd->min_pos = cmnd->min_pos;
	ncmnd->min_level = cmnd->min_level;
	
	varr_edelete(&commands, cmnd);
	ch->desc->pEdit	= ncmnd;
	char_printf(ch, "CmdEd: '%s' moved to %d position.\n",
		ncmnd->name, varr_index(&commands, ncmnd));
	return TRUE;
}

static VALIDATE_FUN(validate_cmd_name)
{
	const char *name = (const char*) arg;
	cmd_t *cmnd, *cmnd2;
	EDIT_CMD(ch, cmnd);

	if (strpbrk(name, " \t")) {
		char_printf(ch,
			    "CmdEd: %s: illegal character in command name.\n",
			    arg);
		return FALSE;
	}

	if ((cmnd2 = cmd_lookup(name))
	&&  cmnd2 != cmnd) {
		char_printf(ch, "CmdEd: %s: duplicate command name.\n", arg);
		return FALSE;
	}

	return TRUE;
}

static void *save_cmd_cb(void *p, va_list ap)
{
	cmd_t *cmnd = (cmd_t *) p;
	FILE *fp = va_arg(ap, FILE *);

	fprintf(fp, "#CMD\n");
	fprintf(fp, "name %s~\n", cmnd->name);
	fprintf(fp, "dofun %s~\n", cmnd->dofun_name);
	fprintf(fp, "min_pos %s\n",
		flag_string(position_table, cmnd->min_pos));
	if (cmnd->min_level)
		fprintf(fp, "min_level %s\n",
			fix_word(flag_istring(level_table, cmnd->min_level)));

	if (cmnd->cmd_log != LOG_NORMAL)
		fprintf(fp, "log %s\n",
			flag_string(cmd_logtypes, cmnd->cmd_log));

	if (cmnd->cmd_flags)
		fprintf(fp, "flags %s~\n",
			flag_string(cmd_flags, cmnd->cmd_flags));

	if (cmnd->cmd_class != CC_ORDINARY)
		fprintf(fp, "class %s\n",
			flag_string(cmd_classes, cmnd->cmd_class));

	fprintf(fp, "end\n\n");

	return NULL;
}

static void check_shadow(CHAR_DATA *ch, const char *name)
{
	BUFFER *output;
	social_t *soc;
	int i;
	bool found = FALSE;

	output = buf_new(-1);

	for (i = 0; i < socials.nused; i++) {
		soc = (social_t *)VARR_GET(&socials, i);
		if (str_prefix(soc->name, name))
			continue;
		if (!found)
			buf_add(output, "The following social(s) will be shadowed:\n");
		found = TRUE;
		buf_printf(output, "  [%s]\n", soc->name);
	}
	if (!found)
		buf_add(output, "This name will not shadow anything.\n");
	
	page_to_char(buf_string(output), ch);
	buf_free(output);
}