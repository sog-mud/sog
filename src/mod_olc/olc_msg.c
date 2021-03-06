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
 * $Id: olc_msg.c,v 1.62 2004-06-28 19:21:04 tatyana Exp $
 */

#include "olc.h"

#include <lang.h>

#define EDIT_MSG(ch, mp)	(mp = (mlstring *) ch->desc->pEdit)

DECLARE_OLC_FUN(msged_create	);
DECLARE_OLC_FUN(msged_edit	);
DECLARE_OLC_FUN(msged_save	);
DECLARE_OLC_FUN(msged_touch	);
DECLARE_OLC_FUN(msged_show	);
DECLARE_OLC_FUN(msged_list	);

DECLARE_OLC_FUN(msged_msg	);
DECLARE_OLC_FUN(msged_del	);

DECLARE_VALIDATE_FUN(validate_msg);

olced_strkey_t strkey_msgdb = { &msgdb, NULL, NULL };

olc_cmd_t olc_cmds_msg[] =
{
	{ "create",	msged_create,	NULL,		NULL		},
	{ "edit",	msged_edit,	NULL,		NULL		},
	{ "",		msged_save,	NULL,		NULL		},
	{ "touch",	msged_touch,	NULL,		NULL		},
	{ "show",	msged_show,	NULL,		NULL		},
	{ "list",	msged_list,	NULL,		NULL		},

	{ "msg",	msged_msg,	validate_msg,	&strkey_msgdb	},
	{ "delete_ms",	olced_spell_out, NULL,		NULL		},
	{ "delete_msg",	msged_del,	NULL,		NULL		},

	{ "commands",	show_commands,	NULL,		NULL		},
	{ "version",	show_version,	NULL,		NULL		},

	{ NULL, NULL, NULL, NULL }
};

/* case-sensitive substring search with [num.]name syntax */
static mlstring *	msg_search(const char *argument);

static const char*	atomsg(const char *argument);

OLC_FUN(msged_create)
{
	mlstring *mlp;
	const char *arg;

	if (PC(ch)->security < SECURITY_MSGDB) {
		act_char("MsgEd: Insufficient security.", ch);
		return FALSE;
	}

	if (IS_NULLSTR(argument))
		OLC_ERROR("'OLC CREATE'");

	arg = atomsg(argument);
	if (!str_cmp(arg, "$")) {
		act_char("MsgEd: invalid value", ch);
		return FALSE;
	}

	if ((mlp = c_insert(&msgdb, arg)) == NULL) {
		char buf[MAX_INPUT_LENGTH];

		act_char("MsgEd: msg already exists:", ch);
		snprintf(buf, sizeof(buf), "msg %s", argument);
		dofun("ashow", ch, buf);
		return FALSE;
	}

	mlstr_init2(mlp, arg);
	ch->desc->pEdit	= mlp;
	OLCED(ch)	= olced_lookup(ED_MSG);
	SET_BIT(changed_flags, CF_MSGDB);
	act_char("Msg created.", ch);
	return FALSE;
}

OLC_FUN(msged_edit)
{
	mlstring *mlp;

	if (PC(ch)->security < SECURITY_MSGDB) {
		act_char("MsgEd: Insufficient security.", ch);
		return FALSE;
	}

	if (IS_NULLSTR(argument))
		OLC_ERROR("'OLC EDIT'");

	if ((mlp = msg_search(atomsg(argument))) == NULL) {
		act_char("MsgEd: msg not found.", ch);
		return FALSE;
	}

	ch->desc->pEdit	= (void *) mlp;
	OLCED(ch)	= olced_lookup(ED_MSG);
	return FALSE;
}

OLC_FUN(msged_save)
{
	FILE *fp;
	mlstring *ml;

	if (!IS_SET(changed_flags, CF_MSGDB)) {
		olc_printf(ch, "Msgdb is not changed.");
		return FALSE;
	}

	if ((fp = olc_fopen(ETC_PATH, MSGDB_FILE, ch, SECURITY_MSGDB)) == NULL)
		return FALSE;

	C_FOREACH (mlstring *, ml, &msgdb)
		mlstr_fwrite(fp, NULL, ml);

	fprintf(fp, "$~\n");
	fclose(fp);
	REMOVE_BIT(changed_flags, CF_MSGDB);
	olc_printf(ch, "Msgdb saved.");
	return FALSE;
}

OLC_FUN(msged_touch)
{
	SET_BIT(changed_flags, CF_MSGDB);
	return FALSE;
}

OLC_FUN(msged_show)
{
	BUFFER *output;
	mlstring *mlp;

	if (IS_NULLSTR(argument)) {
		if (IS_EDIT(ch, ED_MSG))
			EDIT_MSG(ch, mlp);
		else
			OLC_ERROR("'OLC ASHOW'");
	} else {
		if ((mlp = msg_search(atomsg(argument))) == NULL) {
			act_char("MsgEd: msg not found.", ch);
			return FALSE;
		}
	}

	output = buf_new(0);
	mlstr_dump(output, "Msg: ", mlp, DL_ALL);
	page_to_char(buf_string(output), ch);
	buf_free(output);
	return FALSE;
}

OLC_FUN(msged_list)
{
	int num = 0;
	BUFFER *output;
	mlstring *ml;

	if (IS_NULLSTR(argument))
		OLC_ERROR("'OLC ALIST'");

	argument = atomsg(argument);
	output = buf_new(0);

	C_FOREACH (mlstring *, ml, &msgdb) {
		const char *name = mlstr_mval(ml);

		if (IS_NULLSTR(name))
			continue;

		if (strstr(name, argument)) {
			buf_printf(output, BUF_END, "%2d. [%s]\n",
				   ++num, strdump(name, DL_ALL));
		}
	}

	if (num)
		page_to_char(buf_string(output), ch);
	else
		act_char("MsgEd: no messages found.", ch);
	buf_free(output);
	return FALSE;
}

OLC_FUN(msged_msg)
{
	char arg[MAX_INPUT_LENGTH];
	argument = one_argument(argument, arg, sizeof(arg));
	return _olced_mlstrkey(ch, arg, atomsg(argument), cmd);
}

OLC_FUN(msged_del)
{
	mlstring *mlp;

	EDIT_MSG(ch, mlp);

	if (olced_busy(ch, ED_MSG, mlp, NULL))
		return FALSE;

	c_delete(&msgdb, mlstr_mval(mlp));
	SET_BIT(changed_flags, CF_MSGDB);
	edit_done(ch->desc);
	return FALSE;
}

/*
 * local functions
 */

/* case-sensitive substring search with [num.]name syntax */

static mlstring *
msg_search(const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	int num;
	mlstring *ml;

	num = number_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0' || num <= 0)
		return NULL;

	if ((ml = msg_lookup(argument)) != NULL)
		return ml;

	C_FOREACH (mlstring *, ml, &msgdb) {
		const char *name = mlstr_mval(ml);

		if (IS_NULLSTR(name))
			continue;

		if (strstr(name, arg) && !--num)
			return ml;
	}

	return NULL;
}

static const char *
atomsg(const char *argument)
{
	static char buf[MAX_STRING_LENGTH];
	const char *i;
	size_t o;

	if (argument[0] == '.')
		argument++;

	for (o = 0, i = argument; o < sizeof(buf)-1 && *i; i++, o++) {
		if (*i == '\\' && *(i+1)) {
			buf[o] = backslash(*++i);
			continue;
		}
		buf[o] = *i;
	}
	buf[o] = '\0';

	return buf;
}

VALIDATE_FUN(validate_msg)
{
	if (!strcmp(arg, "$")) {
		act_char("MsgEd: invalid value", ch);
		return FALSE;
	}

	return TRUE;
}
