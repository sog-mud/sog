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
 * $Id: olc_msg.c,v 1.28 1999-03-11 09:04:34 fjoe Exp $
 */

#include <stdio.h>
#include <string.h>

#include "merc.h"
#include "olc.h"
#include "db/lang.h"

#define EDIT_MSG(ch, mlp)	(mlp = (mlstring**) ch->desc->pEdit)

DECLARE_OLC_FUN(msged_create		);
DECLARE_OLC_FUN(msged_edit		);
DECLARE_OLC_FUN(msged_show		);
DECLARE_OLC_FUN(msged_list		);

DECLARE_OLC_FUN(msged_msg		);
DECLARE_OLC_FUN(msged_del		);

olc_cmd_t olc_cmds_msg[] =
{
	{ "create",	msged_create	},
	{ "edit",	msged_edit	},
	{ "touch",	olced_dummy	},
	{ "show",	msged_show	},
	{ "list",	msged_list	},

	{ "msg",	msged_msg	},
	{ "delete_ms",	olced_spell_out	},
	{ "delete_msg",	msged_del	},

	{ "commands",	show_commands	},
	{ NULL }
};

/* case-sensitive substring search with [num.]name syntax */
static mlstring **	msg_search(const char *argument);

static const char*	atomsg(const char *argument);
static const char*	msgtoa(const char *argument);

static void		msg_dump(BUFFER *buf, mlstring *ml);

OLC_FUN(msged_create)
{
	if (ch->pcdata->security < SECURITY_MSGDB) {
		char_puts("MsgEd: Insufficient security.\n", ch);
		return FALSE;
	}

	if (argument[0] == '\0') {
		do_help(ch, "'OLC CREATE'");
		return FALSE;
	}

	if (!str_cmp(argument, "$")) {
		char_puts("MsgEd: invalid value.\n", ch);
		return FALSE;
	}

	if (msg_lookup(argument = atomsg(argument))) {
		char_puts("MsgEd: msg already exists.\n", ch);
		return FALSE;
	}

	if (olced_busy(ch, ED_MSG, NULL, NULL))
		return FALSE;

	ch->desc->pEdit	= (void*) msg_add(mlstr_new(argument));
	OLCED(ch)	= olced_lookup(ED_MSG);
	char_puts("Msg created.\n", ch);
	return FALSE;
}

OLC_FUN(msged_edit)
{
	mlstring **mlp;

	if (ch->pcdata->security < SECURITY_MSGDB) {
		char_puts("MsgEd: Insufficient security.\n", ch);
		return FALSE;
	}

	if (argument[0] == '\0') {
		do_help(ch, "'OLC EDIT'");
		return FALSE;
	}

	if ((mlp = msg_search(atomsg(argument))) == NULL) {
		char_puts("MsgEd: msg not found.\n", ch);
		return FALSE;
	}

	ch->desc->pEdit	= (void *) mlp;
	OLCED(ch)	= olced_lookup(ED_MSG);
	return FALSE;
}

OLC_FUN(msged_show)
{
	BUFFER *output;
	mlstring **mlp;

	if (argument[0] == '\0') {
		if (IS_EDIT(ch, ED_MSG))
			EDIT_MSG(ch, mlp);
		else {
			do_help(ch, "'OLC ASHOW'");
			return FALSE;
		}
	}
	else {
		if ((mlp = msg_search(atomsg(argument))) == NULL) {
			char_puts("MsgEd: msg not found.\n", ch);
			return FALSE;
		}
	}

	output = buf_new(-1);
	msg_dump(output, *mlp);
	page_to_char(buf_string(output), ch);
	buf_free(output);
	return FALSE;
}

OLC_FUN(msged_list)
{
	int i;
	int num;
	BUFFER *output = NULL;

	if (argument[0] == '\0') {
		do_help(ch, "'OLC ALIST'");
		return FALSE;
	}
	
	argument = atomsg(argument);
	num = 0;
	for (i = 0; i < MAX_MSG_HASH; i++) {
		int j;
		varr *v = msg_hash_table+i;

		for (j = 0; j < v->nused; j++) {
			mlstring **mlp = VARR_GET(v, j);
			const char *name = mlstr_mval(*mlp);

			if (strstr(name, argument)) {
				if (output == NULL)
					output = buf_new(-1);
				buf_printf(output, "%2d. [%s]\n",
					   ++num, msgtoa(name));
			}
		}
	}

	if (output) {
		page_to_char(buf_string(output), ch);
		buf_free(output);
	}
	else
		char_puts("MsgEd: no messages found.\n", ch);

	return FALSE;
}

OLC_FUN(msged_msg)
{
	char arg[MAX_STRING_LENGTH];
	int lang;
	const char **p;
	mlstring *ml;

	mlstring **mlp;
	EDIT_MSG(ch, mlp);

	argument = one_argument(argument, arg, sizeof(arg));
	lang = lang_lookup(arg);
	if (lang < 0) {
		do_help(ch, "'OLC MSG'");
		return FALSE;
	}

	argument = atomsg(argument);
	if (!lang) {
		/* gonna change name */

		if (!str_cmp(argument, "$")) {
			char_puts("MsgEd: invalid value.\n", ch);
			return FALSE;
		}

		if (msg_lookup(argument)) {
			char_puts("MsgEd: duplicate name.\n", ch);
			return FALSE;
		}

		if (olced_busy(ch, ED_MSG, NULL, NULL))
			return FALSE;

		ml = msg_del(mlstr_mval(*mlp));
		mlp = &ml;
	}

	p = mlstr_convert(mlp, lang);
	free_string(*p);
	*p = str_dup(argument);

	if (!lang) 
		ch->desc->pEdit = (void*) msg_add(ml);

	return TRUE;
}

OLC_FUN(msged_del)
{
	mlstring *ml;
	mlstring **mlp;

	if (olced_busy(ch, ED_MSG, NULL, NULL))
		return FALSE;

	EDIT_MSG(ch, mlp);
	ml = msg_del(mlstr_mval(*mlp));
	mlstr_free(ml);
	edit_done(ch->desc);

	return FALSE;
}

/*
 * local functions
 */

/* case-sensitive substring search with [num.]name syntax */
static mlstring **msg_search(const char *argument)
{
	char name[MAX_INPUT_LENGTH];
	int i;
	int num;

	num = number_argument(argument, name, sizeof(name));
	if (name[0] == '\0' || num <= 0)
		return NULL;

	for (i = 0; i < MAX_MSG_HASH; i++) {
		int j;
		varr *v = msg_hash_table+i;

		for (j = 0; j < v->nused; j++) {
			mlstring **mlp = VARR_GET(v, j);

			if (strstr(mlstr_mval(*mlp), name) && !--num)
				return mlp;
		}
	}

	return NULL;
}

static const char *atomsg(const char *argument)
{
	static char buf[MAX_STRING_LENGTH];
	const char *i;
	int o;

	for (o = 0, i = argument; o < sizeof(buf)-1 && *i; i++, o++) {
		if (*i == '\\' && *(i+1)) {
			switch (*++i) {
			case 'a':
				buf[o] = '\r';
				break;
			case 'r':
				buf[o] = '\r';
				break;
			case 'n':
				buf[o] = '\n';
				break;
			default:
				buf[o] = *i;
				break;
			}
			continue;
		}
		buf[o] = *i;
	}
	buf[o] = '\0';

	return buf;
}

static const char* msgtoa(const char *argument)
{
	static char buf[MAX_STRING_LENGTH];
	const char *i;
	int o;

	for (o = 0, i = argument; o < sizeof(buf)-2 && *i; i++, o++) {
		switch (*i) {
		case '\a':
			buf[o++] = '\\';
			buf[o] = 'a';
			continue;
		case '\n':
			buf[o++] = '\\';
			buf[o] = 'n';
			continue;
		case '\r':
			buf[o++] = '\\';
			buf[o] = 'r';
			continue;
		case '\\':
			buf[o++] = '\\';
			break;
		case '{':
			buf[o++] = *i;
			break;
		}
		buf[o] = *i;
	}
	buf[o] = '\0';

	return buf;
}

static void msg_dump(BUFFER *buf, mlstring *ml)
{
	int lang;
	int nlang = mlstr_nlang(ml);
	static char FORMAT[] = "[%s] [%s]\n";

	if (!nlang) {
		buf_printf(buf, FORMAT, "all", msgtoa(mlstr_mval(ml)));
		return;
	}

	for (lang = 0; lang < nlang; lang++) {
		LANG_DATA *l = VARR_GET(&langs, lang);
		buf_printf(buf, FORMAT, l->name, msgtoa(mlstr_val(ml, lang)));
	}
}

