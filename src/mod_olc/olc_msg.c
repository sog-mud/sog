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
 * $Id: olc_msg.c,v 1.49 2001-02-11 14:35:42 fjoe Exp $
 */

#include "olc.h"
#include "lang.h"

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

olced_strkey_t strkey_msgdb = { &msgdb, NULL };

olc_cmd_t olc_cmds_msg[] =
{
	{ "create",	msged_create				},
	{ "edit",	msged_edit				},
	{ "",		msged_save				},
	{ "touch",	msged_touch				},
	{ "show",	msged_show				},
	{ "list",	msged_list				},

	{ "msg",	msged_msg, validate_msg, &strkey_msgdb	},
	{ "delete_ms",	olced_spell_out				},
	{ "delete_msg",	msged_del				},

	{ "commands",	show_commands				},
	{ NULL }
};

/* case-sensitive substring search with [num.]name syntax */
static mlstring *	msg_search(const char *argument);

static const char*	atomsg(const char *argument);

OLC_FUN(msged_create)
{
	mlstring ml;
	mlstring *mlp;
	const char *arg;

	if (PC(ch)->security < SECURITY_MSGDB) {
		act_char("MsgEd: Insufficient security.", ch);
		return FALSE;
	}

	if (argument[0] == '\0')
		OLC_ERROR("'OLC CREATE'");

	arg = atomsg(argument);
	if (!str_cmp(arg, "$")) {
		act_char("MsgEd: invalid value", ch);
		return FALSE;
	}

	mlstr_init2(&ml, arg);
	mlp = hash_insert(&msgdb, arg, &ml);
	mlstr_destroy(&ml);

	if (mlp == NULL) {
		act_char("MsgEd: msg already exists:", ch);
		dofun("ashow", ch, "msg %s", argument);
		return FALSE;
	}

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

	if (argument[0] == '\0')
		OLC_ERROR("'OLC EDIT'");

	if ((mlp = msg_search(atomsg(argument))) == NULL) {
		act_char("MsgEd: msg not found.", ch);
		return FALSE;
	}

	ch->desc->pEdit	= (void *) mlp;
	OLCED(ch)	= olced_lookup(ED_MSG);
	return FALSE;
}

static void *
msged_add_cb(void *p, va_list ap)
{
	const char *mval = mlstr_mval((mlstring *) p);
	varr *v = va_arg(ap, varr *);
	const char **str;

	if (IS_NULLSTR(mval))
		return NULL;
	str = varr_enew(v);
	*str = mval;
	return NULL;
}

static void *
msged_save_cb(void *p, va_list ap)
{
	FILE *fp = va_arg(ap, FILE *);
	mlstring *ml = msg_lookup(*(const char **) p);

	mlstr_fwrite(fp, NULL, ml);
	return NULL;
}

static varrdata_t v_msgdb = {
	sizeof(const char *), 64
};

OLC_FUN(msged_save)
{
	FILE *fp;
	varr v;

	if (!IS_SET(changed_flags, CF_MSGDB)) {
		olc_printf(ch, "Msgdb is not changed.");
		return FALSE;
	}

	if ((fp = olc_fopen(ETC_PATH, MSGDB_FILE, ch, SECURITY_MSGDB)) == NULL)
		return FALSE;

	varr_init(&v, &v_msgdb);
	hash_foreach(&msgdb, msged_add_cb, &v);
	varr_qsort(&v, cscmpstr);
	varr_foreach(&v, msged_save_cb, fp);
	varr_destroy(&v);

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

	if (argument[0] == '\0') {
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

	output = buf_new(-1);
	mlstr_dump(output, "Msg: ", mlp, DL_ALL);
	page_to_char(buf_string(output), ch);
	buf_free(output);
	return FALSE;
}

static void *
msged_list_cb(void *p, va_list ap)
{
	const char *arg = va_arg(ap, const char *);
	int *pnum = va_arg(ap, int *);
	BUFFER *output = va_arg(ap, BUFFER *);

	const char *name = mlstr_mval((mlstring *) p);

	if (IS_NULLSTR(name))
		return NULL;

	if (strstr(name, arg)) {
		buf_printf(output, BUF_END, "%2d. [%s]\n",
			   ++(*pnum), strdump(name, DL_ALL));
	}
	return NULL;
}

OLC_FUN(msged_list)
{
	int num = 0;
	BUFFER *output;

	if (argument[0] == '\0')
		OLC_ERROR("'OLC ALIST'");
	
	argument = atomsg(argument);
	output = buf_new(-1);
	hash_foreach(&msgdb, msged_list_cb, atomsg(argument), &num, output);
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
	if (argument[0] == '.')
		argument++;
	return _olced_mlstrkey(ch, arg, atomsg(argument), cmd);
}

OLC_FUN(msged_del)
{
	mlstring *mlp;

	if (olced_busy(ch, ED_MSG, NULL, NULL))
		return FALSE;

	EDIT_MSG(ch, mlp);
	hash_delete(&msgdb, mlstr_mval(mlp));
	SET_BIT(changed_flags, CF_MSGDB);
	edit_done(ch->desc);
	return FALSE;
}

/*
 * local functions
 */

/* case-sensitive substring search with [num.]name syntax */

static void *
msg_search_cb(void *p, va_list ap)
{
	int *pnum = va_arg(ap, int *);
	const char *arg = va_arg(ap, const char *);

	const char *name = mlstr_mval((mlstring *) p);

	if (IS_NULLSTR(name))
		return NULL;

	if (strstr(name, arg) && !--(*pnum))
		return p;

	return NULL;
}

static mlstring *
msg_search(const char *argument)
{
	char name[MAX_INPUT_LENGTH];
	int num;
	mlstring *mlp;

	num = number_argument(argument, name, sizeof(name));
	if (name[0] == '\0' || num <= 0)
		return NULL;

	if ((mlp = msg_lookup(argument)) != NULL)
		return mlp;

	return hash_foreach(&msgdb, msg_search_cb, &num, name);
}

static const char *atomsg(const char *argument)
{
	static char buf[MAX_STRING_LENGTH];
	const char *i;
	int o;

	if (argument[0] == '.')
		argument++;

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

VALIDATE_FUN(validate_msg)
{
	if (!strcmp(arg, "$")) {
		act_char("MsgEd: invalid value", ch);
		return FALSE;
	}

	return TRUE;
}
