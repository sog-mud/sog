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
 * $Id: olc_msg.c,v 1.46 2000-10-07 18:15:00 fjoe Exp $
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
static const char*	msgtoa(const char *argument);

OLC_FUN(msged_create)
{
	mlstring ml;
	mlstring *mlp;

	if (PC(ch)->security < SECURITY_MSGDB) {
		act_char("MsgEd: Insufficient security.", ch);
		return FALSE;
	}

	if (argument[0] == '\0')
		OLC_ERROR("'OLC CREATE'");

	argument = atomsg(argument);
	if (!str_cmp(argument, "$")) {
		act_char("MsgEd: invalid value", ch);
		return FALSE;
	}

	mlstr_init2(&ml, argument);
	mlp = hash_insert(&msgdb, argument, &ml);
	mlstr_destroy(&ml);

	if (mlp == NULL) {
		act_char("MsgEd: msg already exists.", ch);
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
msged_save_cb(void *p, va_list ap)
{
	FILE *fp = va_arg(ap, FILE *);

	if (IS_NULLSTR(mlstr_mval((mlstring *) p)))
		return NULL;

	mlstr_fwrite(fp, NULL, (mlstring *) p);
	return NULL;
}

OLC_FUN(msged_save)
{
	FILE *fp;

	if (!IS_SET(changed_flags, CF_MSGDB)) {
		olc_printf(ch, "Msgdb is not changed.");
		return FALSE;
	}

	if ((fp = olc_fopen(ETC_PATH, MSGDB_FILE, ch, SECURITY_MSGDB)) == NULL)
		return FALSE;

	hash_foreach(&msgdb, msged_save_cb, fp);
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

static void
msg_dump(BUFFER *buf, const char *name, const mlstring *mlp)
{
	char space[MAX_STRING_LENGTH];
	size_t namelen;
	int lang;
	static char FORMAT[] = "%s[%s] [%s]\n";
	lang_t *l;

	if (mlp == NULL || mlp->nlang == 0) {
		buf_printf(buf, BUF_END, FORMAT, name, "all",
			   mlp == NULL ? "(null)" : msgtoa(mlp->u.str));
		return;
	}

	if (langs.nused == 0)
		return;

	l = VARR_GET(&langs, 0);
	buf_printf(buf, BUF_END, FORMAT, name, l->name, msgtoa(mlp->u.lstr[0]));

	if (langs.nused < 1)
		return;

	namelen = strlen(name);
	namelen = URANGE(0, namelen, sizeof(space)-1);
	memset(space, ' ', namelen);
	space[namelen] = '\0';

	for (lang = 1; lang < mlp->nlang && lang < langs.nused; lang++) {
		l = VARR_GET(&langs, lang);
		buf_printf(buf, BUF_END, FORMAT,
			   space, l->name, msgtoa(mlp->u.lstr[lang]));
	}
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
	msg_dump(output, "Msg: ", mlp);
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

	if (strstr(name, arg)) 
		buf_printf(output, BUF_END, "%2d. [%s]\n", ++(*pnum), msgtoa(name));
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

	if (!argument)
		return str_empty;

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

VALIDATE_FUN(validate_msg)
{
	if (!strcmp(arg, "$")) {
		act_char("MsgEd: invalid value", ch);
		return FALSE;
	}

	return TRUE;
}
