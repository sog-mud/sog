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
 * $Id: olc_form.c,v 1.16 1999-02-19 15:22:24 fjoe Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include "merc.h"
#include "olc.h"
#include "db/word.h"
#include "db/lang.h"

#define EDIT_WORD(ch, w)	(w = (WORD_DATA*) ch->desc->pEdit)
#define EDIT_LANG(ch, l)	(l = (LANG_DATA*) ch->desc->pEdit2)
#define EDIT_HASH(ch, l, hash)						\
	hash = (ch->desc->editor == ED_GENDER) ? l->hash_genders :	\
	       (ch->desc->editor == ED_CASE) ?	 l->hash_cases :	\
						 l->hash_qtys;

DECLARE_OLC_FUN(formed_create	);
DECLARE_OLC_FUN(formed_edit	);
DECLARE_OLC_FUN(formed_touch	);
DECLARE_OLC_FUN(formed_show	);
DECLARE_OLC_FUN(formed_list	);

DECLARE_OLC_FUN(formed_name	);
DECLARE_OLC_FUN(formed_baselen	);
DECLARE_OLC_FUN(formed_form	);
DECLARE_OLC_FUN(formed_del	);

OLC_CMD_DATA olc_cmds_form[] =
{
	{ "create",	formed_create	},
	{ "edit",	formed_edit	},
	{ "touch",	formed_touch	},
	{ "show",	formed_show	},
	{ "list",	formed_list	},

	{ "name",	formed_name	},
	{ "base",	formed_baselen	},
	{ "form",	formed_form	},
	{ "deletefor",	olced_spell_out	},
	{ "deleteform",	formed_del	},

	{ "commands",	show_commands	},
	{ NULL }
};

OLC_FUN(formed_create)
{
	WORD_DATA *w;
	LANG_DATA *l = NULL;
	char arg[MAX_STRING_LENGTH];
	const char *type;
	varr *hash;

	if (ch->pcdata->security < SECURITY_MSGDB) {
		char_puts("FormEd: Insufficient security.\n", ch);
		return FALSE;
	}

	argument = one_argument(argument, arg, sizeof(arg));
	if (argument[0] == '\0') {
		do_help(ch, "'OLC CREATE'");
		return FALSE;
	}

	if (ch->desc->editor == ED_LANG)
		l = ch->desc->pEdit;
	else if (ch->desc->editor == ED_GENDER ||
		 ch->desc->editor == ED_CASE ||
		 ch->desc->editor == ED_QTY)
		EDIT_LANG(ch, l);

	if (l == NULL) {
		char_puts("FormEd: You must be editing a language or another word.\n", ch);
		return FALSE;
	}

	if (!str_prefix(arg, "case")) {
		type = ED_CASE;
		hash = l->hash_cases;
	}
	else if (!str_prefix(arg, "gender")) {
		type = ED_GENDER;
		hash = l->hash_genders;
	}
	else if (!str_prefix(arg, "qtys")) {
		type = ED_QTY;
		hash = l->hash_qtys;
	}
	else {
		do_help(ch, "'OLC CREATE'");
		return FALSE;
	}

	if (word_lookup(hash, argument)) {
		char_printf(ch, "FormEd: %s: duplicate name.\n", argument);
		return FALSE;
	}

	w = word_new();
	w->name = str_dup(argument);
	ch->desc->editor = type;
	ch->desc->pEdit = word_add(hash, w);
	ch->desc->pEdit2 = l; 
	touch_lang(l, type);
	char_puts("FormEd: word created.\n", ch);
	return FALSE;
}

OLC_FUN(formed_edit)
{
	WORD_DATA *w;
	LANG_DATA *l = NULL;
	char arg[MAX_STRING_LENGTH];
	const char *type;
	varr *hash;

	if (ch->pcdata->security < SECURITY_MSGDB) {
		char_puts("FormEd: Insufficient security.\n", ch);
		return FALSE;
	}

	argument = one_argument(argument, arg, sizeof(arg));
	if (argument[0] == '\0') {
		do_help(ch, "'OLC EDIT'");
		return FALSE;
	}

	if (ch->desc->editor == ED_LANG)
		l = ch->desc->pEdit;
	else if (ch->desc->editor == ED_GENDER ||
		 ch->desc->editor == ED_CASE ||
		 ch->desc->editor == ED_QTY)
		EDIT_LANG(ch, l);

	if (l == NULL) {
		char_puts("FormEd: You must be editing a language or another word.\n", ch);
		return FALSE;
	}

	if (!str_prefix(arg, "cases")) {
		type = ED_CASE;
		hash = l->hash_cases;
	}
	else if (!str_prefix(arg, "genders")) {
		type = ED_GENDER;
		hash = l->hash_genders;
	}
	else if (!str_prefix(arg, "qtys")) {
		type = ED_QTY;
		hash = l->hash_qtys;
	}
	else {
		do_help(ch, "'OLC EDIT'");
		return FALSE;
	}

	if ((w = word_lookup(hash, argument)) == NULL) {
		char_printf(ch, "FormEd: %s: not found.\n", argument);
		return FALSE;
	}

	ch->desc->editor = type;
	ch->desc->pEdit = w;
	ch->desc->pEdit2 = l;
	return FALSE;
}

OLC_FUN(formed_touch)
{
	LANG_DATA *l;
	EDIT_LANG(ch, l);
	return touch_lang(l, ch->desc->editor);
}

OLC_FUN(formed_show)
{
	int i;
	char buf[MAX_STRING_LENGTH];
	WORD_DATA *w;
	LANG_DATA *l;

	if (ch->desc->editor == ED_LANG) {
		varr *hash;
		char arg[MAX_INPUT_LENGTH];

		argument = one_argument(argument, arg, sizeof(arg));
		if (argument[0] == '\0') {
			do_help(ch, "'OLC ASHOW'");
			return FALSE;
		}

		l = ch->desc->pEdit;
		if (!str_prefix(arg, "case"))
			hash = l->hash_cases;
		else if (!str_prefix(arg, "gender"))
			hash = l->hash_genders;
		else if (!str_prefix(arg, "qtys")) 
			hash = l->hash_qtys;
		else {
			do_help(ch, "'OLC ASHOW'");
			return FALSE;
		}

		if ((w = word_lookup(hash, argument)) == NULL) {
			char_printf(ch, "FormEd: %s: not found.\n", argument);
			return FALSE;
		}
	}
	else if (ch->desc->editor == ED_GENDER ||
		 ch->desc->editor == ED_CASE ||
		 ch->desc->editor == ED_QTY) {
		EDIT_LANG(ch, l);
		EDIT_WORD(ch, w);
	}
	else {
		char_puts("FormEd: You must be editing a language or another word.\n", ch);
		return FALSE;
	}

	char_printf(ch, "Name: [%s]\n"
			"Lang: [%s]\n"
			"Type: [%s]\n",
		    w->name,
		    l->name,
		    ch->desc->editor == ED_GENDER ?	"gender" :
		    ch->desc->editor == ED_CASE ?	"case" :
		    ch->desc->editor == ED_QTY ?	"qty" :
							"unknown");

	strnzcpy(buf, w->name, UMIN(w->base_len+1, sizeof(buf)));
	char_printf(ch, "Base: [%s] (%d)\n", buf, w->base_len);

	for (i = 0; i < w->f.nused; i++) {
		char **p = VARR_GET(&w->f, i);

		if (!IS_NULLSTR(*p))
			char_printf(ch, "Form: [%d] [%s]\n", i, *p);
	}

	return FALSE;
}

OLC_FUN(formed_list)
{
	int i;
	BUFFER *output = NULL;
	varr *hash;
	LANG_DATA *l;
	char arg[MAX_STRING_LENGTH];

	if (ch->desc->editor == ED_LANG)
		l = ch->desc->pEdit;
	else if (ch->desc->editor == ED_GENDER ||
		 ch->desc->editor == ED_CASE ||
		 ch->desc->editor == ED_QTY)
		EDIT_LANG(ch, l);
	else {
		char_puts("FormEd: You must be editing a language or another word.\n", ch);
		return FALSE;
	}

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0' || argument[0] == '\0') {
		do_help(ch, "'OLC ALIST'");
		return FALSE;
	}
	
	if (!str_prefix(arg, "cases"))
		hash = l->hash_cases;
	else if (!str_prefix(arg, "genders"))
		hash = l->hash_genders;
	else if (!str_prefix(arg, "qtys"))
		hash = l->hash_qtys;
	else {
		do_help(ch, "'OLC ALIST'");
		return FALSE;
	}

	for (i = 0; i < MAX_WORD_HASH; i++) {
		int j;
		varr *v = hash+i;

		for (j = 0; j < v->nused; j++) {
			WORD_DATA *w = VARR_GET(v, j);

			if (!str_prefix(argument, w->name)) {
				if (output == NULL)
					output = buf_new(-1);
				buf_printf(output, "%s\n", w->name);
			}
		}
	}

	if (output) {
		page_to_char(buf_string(output), ch);
		buf_free(output);
	}
	else
		char_puts("FormEd: no messages found.\n", ch);

	return FALSE;
}

OLC_FUN(formed_name)
{
	WORD_DATA *w;
	LANG_DATA *l;
	varr *hash;

	if (argument[0] == '\0') {
		do_help(ch, "'OLC WORD'");
		return FALSE;
	}

	EDIT_WORD(ch, w);
	EDIT_LANG(ch, l);
	EDIT_HASH(ch, l, hash);

	if (word_lookup(hash, argument)) {
		char_printf(ch, "FormEd: %s: duplicate name.\n", argument);
		return FALSE;
	}

	word_del(hash, w->name);
	free_string(w->name);
	w->name = str_dup(argument);
	ch->desc->pEdit = word_add(hash, w);
	return TRUE;
}

OLC_FUN(formed_baselen)
{
	char arg[MAX_INPUT_LENGTH];
	WORD_DATA *w;
	EDIT_WORD(ch, w);

	one_argument(argument, arg, sizeof(arg));
	if (str_prefix(arg, w->name)) {
		char_printf(ch, "FormEd: %s: not prefix of name (%s).\n",
			    arg, w->name);
		return FALSE;
	}

	w->base_len = strlen(arg);
	return TRUE;
}

OLC_FUN(formed_form)
{
	WORD_DATA *w;
	bool add;
	int num;
	char arg[MAX_STRING_LENGTH];
	char arg2[MAX_STRING_LENGTH];

	argument = one_argument(argument, arg, sizeof(arg));
	argument = one_argument(argument, arg2, sizeof(arg2));

	if (!str_prefix(arg, "add"))
		add = TRUE;
	else if (!str_prefix(arg, "delete"))
		add = FALSE;
	else {
		do_help(ch, "'OLC FORM'");
		return FALSE;
	}

	if ((add && argument[0] == '\0') || !is_number(arg2)) {
		do_help(ch, "'OLC FORM'");
		return FALSE;
	}

	num = atoi(arg2);
	EDIT_WORD(ch, w);
	if (add)
		word_form_add(w, num, argument);
	else
		word_form_del(w, num);
	return TRUE;
}

OLC_FUN(formed_del)
{
	varr *hash;
	WORD_DATA *w;
	LANG_DATA *l;

	EDIT_WORD(ch, w);
	EDIT_LANG(ch, l);
	EDIT_HASH(ch, l, hash);

	word_del(hash, w->name);
	touch_lang(l, ch->desc->editor);
	edit_done(ch->desc);

	return FALSE;
}
