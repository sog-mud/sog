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
 * $Id: olc_msg.c,v 1.2 1998-09-24 14:09:05 fjoe Exp $
 */

#include <stdio.h>
#include <string.h>

#include "merc.h"
#include "olc.h"
#include "interp.h"

#define EDIT_MSG(ch, mlp)	(mlp = (mlstring**) ch->desc->pEdit)

DECLARE_OLC_FUN(msged_create		);
DECLARE_OLC_FUN(msged_edit		);
DECLARE_OLC_FUN(msged_touch		);
DECLARE_OLC_FUN(msged_show		);

DECLARE_OLC_FUN(msged_msg		);
DECLARE_OLC_FUN(msged_list		);
DECLARE_OLC_FUN(msged_del		);

OLC_CMD_DATA olc_cmds_msg[] =
{
	{ "create",	msged_create	},
	{ "edit",	msged_edit	},
	{ "touch",	msged_touch	},
	{ "show",	msged_show	},

	{ "msg",	msged_msg	},
	{ "list",	msged_list	},
	{ "del",	msged_del	},

	{ NULL }
};

/* case-sensitive substring search with [num.]name syntax */
static mlstring **	msg_search(const char *argument);
static char*		fix_msg(const char *argument);

OLC_FUN(msged_create)
{
	if (ch->pcdata->security < SECURITY_MSGDB) {
		char_puts("MsgEd: Insufficient security.\n\r", ch);
		return FALSE;
	}

	if (argument[0] == '\0') {
		do_help(ch, "'OLC CREATE'");
		return FALSE;
	}

	if (!str_cmp(argument, "$")) {
		char_puts("MsgEd: invalid value.\n\r", ch);
		return FALSE;
	}

	if (msg_lookup(argument) != NULL) {
		char_puts("MsgEd: msg already exists.\n\r", ch);
		return FALSE;
	}

	ch->desc->pEdit	= (void*) msg_add(mlstr_new(fix_msg(argument)));
	ch->desc->editor = ED_MSG;
	char_puts("Msg created.\n\r", ch);
	return FALSE;
}

OLC_FUN(msged_edit)
{
	mlstring **mlp;

	if (ch->pcdata->security < SECURITY_MSGDB) {
		char_puts("MsgEd: Insufficient security.\n\r", ch);
		return FALSE;
	}

	if (argument[0] == '\0') {
		do_help(ch, "'OLC EDIT'");
		return FALSE;
	}

	if ((mlp = msg_search(argument)) == NULL) {
		char_puts("MsgEd: msg not found.\n\r", ch);
		return FALSE;
	}

	ch->desc->pEdit		= (void *) mlp;
	ch->desc->editor	= ED_MSG;
	return FALSE;
}

OLC_FUN(msged_touch)
{
	return FALSE;
}

OLC_FUN(msged_show)
{
	BUFFER *output;
	mlstring **mlp;

	EDIT_MSG(ch, mlp);
	output = buf_new(0);

	mlstr_dump(output, str_empty, *mlp);

	page_to_char(buf_string(output), ch);
	buf_free(output);
	return FALSE;
}

OLC_FUN(msged_msg)
{
	char arg[MAX_STRING_LENGTH];
	int lang;
	char **p;
	mlstring *ml;

	mlstring **mlp;
	EDIT_MSG(ch, mlp);

	argument = one_argument(argument, arg);
	lang = lang_lookup(arg);
	if (lang < 0 && str_cmp(arg, "all")) {
		do_help(ch, "'OLC MSG'");
		return FALSE;
	}

	if (lang <= 0) {
		/* gonna change name */
		mlstring **mlp2;

		if (!str_cmp(argument, "$")) {
			char_puts("MsgEd: invalid value.\n\r", ch);
			return FALSE;
		}

		mlp2 = msg_lookup(argument);
		if (mlp2 && mlp2 != mlp) {
			char_puts("MsgEd: duplicate name.\n\r", ch);
			return FALSE;
		}
		ml = msg_del(mlstr_mval(*mlp));
	}
	else
		ml = *mlp;

	p = mlstr_convert(&ml, lang);
	free_string(*p);
	*p = str_dup(fix_msg(argument));

	if (lang <= 0)
		ch->desc->pEdit = (void*) msg_add(ml);

	return TRUE;
}

OLC_FUN(msged_list)
{
	int i;
	BUFFER *output = NULL;

	if (argument[0] == '\0') {
		do_help(ch, "'OLC MSG'");
		return FALSE;
	}
		
	for (i = 0; i < MAX_MSG_HASH; i++) {
		int j;
		varr *v = msg_hash_table[i];

		if (v == NULL)
			continue;

		for (j = 0; j < v->nused; j++) {
			mlstring **mlp = VARR_GET(v, j);
			char *name = mlstr_mval(*mlp);

			if (strstr(name, argument)) {
				if (output == NULL)
					output = buf_new(0);
				buf_add(output, name);
				if (name[strlen(name)-1] != '\r')
					buf_add(output, "\n\r");
			}
		}
	}

	if (output) {
		page_to_char(buf_string(output), ch);
		buf_free(output);
	}
	else
		send_to_char("MsgEd: no messages found.\n\r", ch);

	return FALSE;
}

OLC_FUN(msged_del)
{
	mlstring *ml;
	mlstring **mlp;
	EDIT_MSG(ch, mlp);

	ml = msg_del(mlstr_mval(*mlp));
	mlstr_free(ml);
	edit_done(ch->desc);

	return FALSE;
}

/* case-sensitive substring search with [num.]name syntax */
static mlstring **msg_search(const char *argument)
{
	char name[MAX_INPUT_LENGTH];
	int i;
	int num;

	num = number_argument(argument, name);
	if (name[0] == '\0' || num <= 0)
		return NULL;

	for (i = 0; i < MAX_MSG_HASH; i++) {
		int j;
		varr *v = msg_hash_table[i];

		if (v == NULL)
			continue;

		for (j = 0; j < v->nused; j++) {
			mlstring **mlp = VARR_GET(v, j);

			if (strstr(mlstr_mval(*mlp), argument) && !--num)
				return mlp;
		}
	}

	return NULL;
}

static char *fix_msg(const char *argument)
{
	static char buf[MAX_STRING_LENGTH];
	int i;
	const char *o;

	for (i = 0, o = argument; i < sizeof(buf)-1 && *o; i++, o++) {
		if (*o == '\\' && *(o+1)) {
			switch (*++o) {
			case 'r':
				buf[i] = '\r';
				break;
			case 'n':
				buf[i] = '\n';
				break;
			default:
				buf[i] = *o;
				break;
			}
			continue;
		}
		buf[i] = *o;
	}
	buf[i] = '\0';

	return buf;
}

