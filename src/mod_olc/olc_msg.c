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
 * $Id: olc_msg.c,v 1.36 1999-10-17 08:55:45 fjoe Exp $
 */

#include "olc.h"
#include "lang.h"

#define EDIT_MSG(ch, mp)	(mp = (msg_t*) ch->desc->pEdit)

DECLARE_OLC_FUN(msged_create	);
DECLARE_OLC_FUN(msged_edit	);
DECLARE_OLC_FUN(msged_save	);
DECLARE_OLC_FUN(msged_touch	);
DECLARE_OLC_FUN(msged_show	);
DECLARE_OLC_FUN(msged_list	);

DECLARE_OLC_FUN(msged_msg	);
DECLARE_OLC_FUN(msged_gender	);
DECLARE_OLC_FUN(msged_del	);

olc_cmd_t olc_cmds_msg[] =
{
	{ "create",	msged_create				},
	{ "edit",	msged_edit				},
	{ "",		msged_save				},
	{ "touch",	msged_touch				},
	{ "show",	msged_show				},
	{ "list",	msged_list				},

	{ "msg",	msged_msg				},
	{ "gender",	msged_gender,	NULL,	gender_table	},
	{ "delete_ms",	olced_spell_out				},
	{ "delete_msg",	msged_del				},

	{ "commands",	show_commands				},
	{ NULL }
};

/* case-sensitive substring search with [num.]name syntax */
static msg_t *		msg_search(const char *argument);

static const char*	atomsg(const char *argument);
static const char*	msgtoa(const char *argument);

static void		msg_dump(BUFFER *buf, msg_t *mp);

OLC_FUN(msged_create)
{
	msg_t m;

	if (PC(ch)->security < SECURITY_MSGDB) {
		char_puts("MsgEd: Insufficient security.\n", ch);
		return FALSE;
	}

	if (argument[0] == '\0') {
		dofun("help", ch, "'OLC CREATE'");
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

	mlstr_init(&m.ml, argument);
	m.gender = 0;
	ch->desc->pEdit	= (void*) msg_add(&m);
	OLCED(ch)	= olced_lookup(ED_MSG);
	SET_BIT(changed_flags, CF_MSGDB);
	char_puts("Msg created.\n", ch);
	return FALSE;
}

OLC_FUN(msged_edit)
{
	msg_t *mp;

	if (PC(ch)->security < SECURITY_MSGDB) {
		char_puts("MsgEd: Insufficient security.\n", ch);
		return FALSE;
	}

	if (argument[0] == '\0') {
		dofun("help", ch, "'OLC EDIT'");
		return FALSE;
	}

	if ((mp = msg_search(atomsg(argument))) == NULL) {
		char_puts("MsgEd: msg not found.\n", ch);
		return FALSE;
	}

	ch->desc->pEdit	= (void *) mp;
	OLCED(ch)	= olced_lookup(ED_MSG);
	return FALSE;
}

OLC_FUN(msged_save)
{
	int i;
	FILE *fp;

	if (!IS_SET(changed_flags, CF_MSGDB)) {
		olc_printf(ch, "Msgdb is not changed.");
		return FALSE;
	}

	if ((fp = olc_fopen(ETC_PATH, MSGDB_CONF, ch, SECURITY_MSGDB)) == NULL)
		return FALSE;

	for (i = 0; i < MAX_MSG_HASH; i++) {
		varr *v = msg_hash_table+i;
		int j;

		for (j = 0; j < v->nused; j++) {
			msg_t *mp = VARR_GET(v, j);

			if (IS_NULLSTR(mlstr_mval(&mp->ml)))
				continue;

			fprintf(fp, "#MSG\n");

			if (mp->gender) {
				fprintf(fp, "Gender %s\n",
					flag_string(gender_table, mp->gender));
			}
			
			mlstr_fwrite(fp, "Text", &mp->ml);
			fprintf(fp, "End\n\n");
		}
	}

	fprintf(fp, "#$\n");
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
	msg_t *mp;

	if (argument[0] == '\0') {
		if (IS_EDIT(ch, ED_MSG))
			EDIT_MSG(ch, mp);
		else {
			dofun("help", ch, "'OLC ASHOW'");
			return FALSE;
		}
	}
	else {
		if ((mp = msg_search(atomsg(argument))) == NULL) {
			char_puts("MsgEd: msg not found.\n", ch);
			return FALSE;
		}
	}

	output = buf_new(-1);
	msg_dump(output, mp);
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
		dofun("help", ch, "'OLC ALIST'");
		return FALSE;
	}
	
	argument = atomsg(argument);
	num = 0;
	for (i = 0; i < MAX_MSG_HASH; i++) {
		int j;
		varr *v = msg_hash_table+i;

		for (j = 0; j < v->nused; j++) {
			msg_t *mp = VARR_GET(v, j);
			const char *name = mlstr_mval(&mp->ml);

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
	msg_t m;

	msg_t *mp;
	EDIT_MSG(ch, mp);

	argument = one_argument(argument, arg, sizeof(arg));
	lang = lang_lookup(arg);
	if (lang < 0) {
		dofun("help", ch, "'OLC MSG'");
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

		m = msg_del(mlstr_mval(&mp->ml));
		mp = &m;
	}

	p = mlstr_convert(&mp->ml, lang);
	free_string(*p);
	*p = str_dup(argument);

	if (!lang) 
		ch->desc->pEdit = (void*) msg_add(mp);

	return TRUE;
}

OLC_FUN(msged_gender)
{
	msg_t *mp;
	EDIT_MSG(ch, mp);
	return olced_flag32(ch, argument, cmd, &mp->gender);
}

OLC_FUN(msged_del)
{
	msg_t *mp;
	msg_t m;

	if (olced_busy(ch, ED_MSG, NULL, NULL))
		return FALSE;

	EDIT_MSG(ch, mp);
	m = msg_del(mlstr_mval(&mp->ml));
	mlstr_destroy(&m.ml);
	SET_BIT(changed_flags, CF_MSGDB);
	edit_done(ch->desc);

	return FALSE;
}

/*
 * local functions
 */

/* case-sensitive substring search with [num.]name syntax */
static msg_t *msg_search(const char *argument)
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
			msg_t *mp = VARR_GET(v, j);

			if (strstr(mlstr_mval(&mp->ml), name) && !--num)
				return mp;
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

static void msg_dump(BUFFER *buf, msg_t *mp)
{
	int lang;
	int nlang = mlstr_nlang(&mp->ml);
	static char FORMAT[] = "[%s] [%s]\n";

	buf_printf(buf, "Gender: [%s]\n",
		   flag_string(gender_table, mp->gender));

	if (!nlang) {
		buf_printf(buf, FORMAT, "all", msgtoa(mlstr_mval(&mp->ml)));
		return;
	}

	for (lang = 0; lang < nlang; lang++) {
		lang_t *l = VARR_GET(&langs, lang);
		buf_printf(buf, FORMAT,
			   l->name, msgtoa(mlstr_val(&mp->ml, lang)));
	}
}

