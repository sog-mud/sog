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
 * $Id: info.c,v 1.36 2003-04-19 00:26:46 fjoe Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if defined(SUNOS) || defined(SVR4) || defined(LINUX)
#	include <crypt.h>
#endif

#include <merc.h>
#include <lang.h>
#include <colors.h>

#include <sog.h>

#include "comm.h"

static DECLARE_SERVICE_FUN(cmd_who);
static DECLARE_SERVICE_FUN(cmd_auth);
static DECLARE_SERVICE_FUN(cmd_help);
static DECLARE_SERVICE_FUN(cmd_show);
static DECLARE_SERVICE_FUN(cmd_setf);

/* Currently supported commands:
 * WHO <fmt>				- outputs list of visible players
 * AUTH <plr> <pwd>			- authentificate a player
 * HELP <fmt> <lang> <lev> <topic>	- show help
 * SHOW	<fmt> <plr>			- dump player's information
 * SETF <plr> <pwd> <flags>		- set www_show_flags equal to <flags>
 */

static service_cmd_t info_cmds[] =
{
	{ "WHO",	cmd_who			},
	{ "AUTH",	cmd_auth		},
	{ "HELP",	cmd_help		},
	{ "SHOW",	cmd_show		},
	{ "SETF",	cmd_setf		},
	{ NULL,		service_unimpl		},
};

void
handle_info(DESCRIPTOR_DATA *d)
{
	if (check_ban(d, BCL_INFO) == BAN_DENY)
		return;
	handle_service(d, info_cmds);
}

static
SERVICE_FUN(cmd_who)
{
	BUFFER *output;
	DESCRIPTOR_DATA *d2;
	int format;
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH * 4];

	one_argument(argument, arg, sizeof(arg));
	output = buf_new(-1);
	buf_printf(output, BUF_END, "%d\n", top_player);
	for (d2 = descriptor_list; d2 != NULL; d2 = d2->next) {
		CHAR_DATA *wch = d2->original ? d2->original : d2->character;

		if (d2->connected != CON_PLAYING
		||  wch->invis_level
		||  wch->incog_level
		||  HAS_INVIS(wch, ID_ALL_INVIS)
		||  is_sn_affected(wch, "vampire"))
			continue;

		do_who_raw(NULL, wch, output);
	}
	format = format_lookup(arg);
	parse_colors(buf_string(output), buf, sizeof(buf), format);
	write_to_buffer(d, buf, 0);
	buf_free(output);
}

static
SERVICE_FUN(cmd_auth)
{
	char arg1[MAX_INPUT_LENGTH],arg2[MAX_INPUT_LENGTH];
	CHAR_DATA *ch;

	argument = first_arg(argument, arg1, sizeof(arg1), FALSE);
	argument = first_arg(argument, arg2, sizeof(arg2), FALSE);

	ch = char_load(arg1, LOAD_F_NOCREATE);
	if (!ch || strcmp(crypt(arg2, ch->name), PC(ch)->pwd)) {
		write_to_buffer(d, "AUTH FAILED.\n", 0);
		if (ch)
			char_nuke(ch);
		return;
	}
	write_to_buffer(d, "AUTH OK.\n", 0);
	char_nuke(ch);
}

static
SERVICE_FUN(cmd_help)
{
	BUFFER *output;
	int format, lev;
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH * 4];
	lang_t *l;
	size_t lang;

	argument = one_argument(argument, arg, sizeof(arg));
	format = format_lookup(arg);

	argument = one_argument(argument, arg, sizeof(arg));
	if ((l = lang_lookup(arg)) == NULL)
		lang = 0;
	else
		lang = varr_index(&langs, l);

	argument = one_argument(argument, arg, sizeof(arg));
	lev = atoi(arg);

	output = buf_new(-1);
	help_show_raw(lev, lang, output, argument);
	parse_colors(buf_string(output), buf, sizeof(buf), format);
	write_to_buffer(d, buf, 0);
	buf_free(output);
}

static
SERVICE_FUN(cmd_show)
{
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH * 4];
	CHAR_DATA *ch;
	BUFFER *output;
	int format;

	argument = one_argument(argument, arg, sizeof(arg));
	format = format_lookup(arg);

	argument = first_arg(argument, arg, sizeof(arg), FALSE);
	ch = char_load(arg, LOAD_F_NOCREATE);
	if (!ch) {
		write_to_buffer(d, "This character has not born yet.\n", 0);
		return;
	}

	output = buf_new(-1);
	buf_printf(output, BUF_END, "%s\n%s\n%s %s\n%d\n%s\n%s\n%s\n%s\n%d\n%d %d\n%d\n%s\n%s\n%d\n%s\n",
		IS_SET(PC(ch)->www_show_flags, WSHOW_RACE) ? PC(ch)->race : "Unknown",
		IS_SET(PC(ch)->www_show_flags, WSHOW_CLASS) ? ch->class : "Unknown",
		(IS_SET(PC(ch)->www_show_flags, WSHOW_CLAN) && !IS_NULLSTR(ch->clan)) ? ch->clan : "Unknown",
		(IS_SET(PC(ch)->www_show_flags, WSHOW_CLAN) && !IS_NULLSTR(ch->clan)) ? flag_string(clan_status_table, PC(ch)->clan_status) : "",
		IS_SET(PC(ch)->www_show_flags, WSHOW_LEVEL) ? ch->level : -1,
		IS_SET(PC(ch)->www_show_flags, WSHOW_ALIGN) ? flag_string(align_names, NALIGN(ch)) : "Unknown",
		IS_SET(PC(ch)->www_show_flags, WSHOW_ETHOS) ? flag_string(ethos_table, PC(ch)->ethos) : "Unknown",
		IS_SET(PC(ch)->www_show_flags, WSHOW_SEX) ? mlstr_mval(&ch->gender) : "Unknown",
		IS_SET(PC(ch)->www_show_flags, WSHOW_SLANG) ? flag_string(slang_table, ch->slang) : "Unknown",
		IS_SET(PC(ch)->www_show_flags, WSHOW_DEATHS) ? PC(ch)->death : -1,
		IS_SET(PC(ch)->www_show_flags, WSHOW_KILLS) ? PC(ch)->has_killed : -1,
		IS_SET(PC(ch)->www_show_flags, WSHOW_KILLS) ? PC(ch)->anti_killed : -1,
		IS_SET(PC(ch)->www_show_flags, WSHOW_PCKILLS) ? PC(ch)->pc_killed : -1,
		IS_SET(PC(ch)->www_show_flags, WSHOW_TITLE) ? PC(ch)->title : "",
		IS_SET(PC(ch)->www_show_flags, WSHOW_HOMETOWN) ? hometown_name(PC(ch)->hometown) : "Unknown",
		IS_SET(PC(ch)->www_show_flags, WSHOW_AGE) ? get_age(ch) : -1,
		IS_WANTED(ch) ? "WANTED" : "");

	parse_colors(buf_string(output), buf, sizeof(buf), format);
	write_to_buffer(d, buf, 0);
	buf_free(output);
	char_nuke(ch);
}

static
SERVICE_FUN(cmd_setf)
{
	BUFFER *output;
	char arg1[MAX_INPUT_LENGTH],arg2[MAX_INPUT_LENGTH];
	CHAR_DATA *ch, *vch;

	argument = first_arg(argument, arg1, sizeof(arg1), FALSE);
	argument = first_arg(argument, arg2, sizeof(arg2), FALSE);

	ch = char_load(arg1, LOAD_F_NOCREATE);
	if (!ch || strcmp(crypt(arg2, ch->name), PC(ch)->pwd)) {
		write_to_buffer(d, "AUTH FAILED.\n", 0);
		if (ch)
			char_nuke(ch);
		return;
	}

	for (vch = char_list; vch && !IS_NPC(vch); vch = vch->next) {
		if (!str_cmp(ch->name, vch->name)) {
			write_to_buffer(d, "CHAR LOGGED IN.\n", 0);
			char_nuke(ch);
			return;
		}
	}

	PC(ch)->www_show_flags = flag_value(www_flags_table, argument);
	char_save(ch, SAVE_F_PSCAN);
	output = buf_new(-1);
	buf_printf(output, BUF_END, "FLAGS SET: %s",
	    flag_string(www_flags_table, PC(ch)->www_show_flags));
	write_to_buffer(d, buf_string(output), 0);
	buf_free(output);
	char_nuke(ch);
}
