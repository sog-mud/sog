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
 * $Id: olc_form.c,v 1.5 1998-12-23 16:11:21 fjoe Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include "merc.h"
#include "olc.h"
#include "db/lang.h"
#include "db/word.h"

#define EDIT_WORD(ch, w)	(w = (WORD_DATA*) ch->desc->pEdit)
#define EDIT_HASH(ch, l, hashp)						\
	hashp = ch->desc->editor == ED_GENDER ? l->hash_genders :	\
						l->hash_cases;

DECLARE_OLC_FUN(worded_create	);
DECLARE_OLC_FUN(worded_edit	);
DECLARE_OLC_FUN(worded_touch	);
DECLARE_OLC_FUN(worded_show	);
DECLARE_OLC_FUN(worded_list	);

DECLARE_OLC_FUN(worded_name	);
DECLARE_OLC_FUN(worded_base	);
DECLARE_OLC_FUN(worded_form	);
DECLARE_OLC_FUN(worded_del	);

OLC_CMD_DATA olc_cmds_word[] =
{
	{ "create",	worded_create	},
	{ "edit",	worded_edit	},
	{ "touch",	worded_touch	},
	{ "show",	worded_show	},
	{ "list",	worded_list	},

	{ "name",	worded_name,	},
	{ "base",	worded_base	},
	{ "form",	worded_form	},
	{ "del",	worded_del	},

	{ "commands",	show_commands	},
	{ NULL }
};

OLC_FUN(worded_create)
{
	WORD_DATA *w;
	LANG_DATA *l = NULL;
	char arg[MAX_STRING_LENGTH];
	const char *type;
	varr **hashp;

	if (ch->pcdata->security < SECURITY_MSGDB) {
		char_puts("WordEd: Insufficient security.\n", ch);
		return FALSE;
	}

	argument = one_argument(argument, arg);
	if (argument[0] == '\0') {
		do_help(ch, "'OLC CREATE'");
		return FALSE;
	}

	if (ch->desc->editor == ED_LANG)
		l = ch->desc->pEdit;
	else if (ch->desc->editor == ED_GENDER || ch->desc->editor == ED_CASE) {
		w = ch->desc->pEdit;
		l = varr_get(&langs, w->lang);
	}

	if (l == NULL) {
		char_puts("WordEd: You must be editing a language or another word.\n", ch);
		return FALSE;
	}

	if (!str_prefix(arg, "case")) {
		type = ED_CASE;
		hashp = l->hash_cases;
	}
	else if (!str_prefix(arg, "gender")) {
		type = ED_GENDER;
		hashp = l->hash_genders;
	}
	else {
		do_help(ch, "'OLC CREATE'");
		return FALSE;
	}

	if (word_lookup(hashp, argument)) {
		char_printf(ch, "WordEd: %s: duplicate name.\n", argument);
		return FALSE;
	}

	w = word_new(l->vnum);
	w->name = str_dup(argument);
	word_add(hashp, w);
	ch->desc->editor = type;
	ch->desc->pEdit = w;
	char_puts("WordEd: word created.\n", ch);
	return FALSE;
}

OLC_FUN(worded_edit)
{
	WORD_DATA *w;
	LANG_DATA *l = NULL;
	char arg[MAX_STRING_LENGTH];
	const char *type;
	varr **hashp;

	if (ch->pcdata->security < SECURITY_MSGDB) {
		char_puts("WordEd: Insufficient security.\n", ch);
		return FALSE;
	}

	argument = one_argument(argument, arg);
	if (argument[0] == '\0') {
		do_help(ch, "'OLC EDIT'");
		return FALSE;
	}

	if (ch->desc->editor == ED_LANG)
		l = ch->desc->pEdit;
	else if (ch->desc->editor == ED_GENDER || ch->desc->editor == ED_CASE) {
		w = ch->desc->pEdit;
		l = varr_get(&langs, w->lang);
	}

	if (l == NULL) {
		char_puts("WordEd: You must be editing a language or another word.\n", ch);
		return FALSE;
	}

	if (!str_prefix(arg, "cases")) {
		type = ED_CASE;
		hashp = l->hash_cases;
	}
	else if (!str_prefix(arg, "genders")) {
		type = ED_GENDER;
		hashp = l->hash_genders;
	}
	else {
		do_help(ch, "'OLC EDIT'");
		return FALSE;
	}

	if ((w = word_lookup(hashp, argument)) == NULL) {
		char_printf(ch, "WordEd: %s: not found.\n", argument);
		return FALSE;
	}

	ch->desc->editor = type;
	ch->desc->pEdit = w;
	return FALSE;
}

OLC_FUN(worded_touch)
{
	WORD_DATA *w;
	LANG_DATA *l;

	EDIT_WORD(ch, w);
	if ((l = varr_get(&langs, w->lang)) == NULL)
		return FALSE;

	if (ch->desc->editor == ED_GENDER)
		SET_BIT(l->flags, LANG_GENDERS_CHANGED);
	else if (ch->desc->editor == ED_CASE)
		SET_BIT(l->flags, LANG_CASES_CHANGED);

	return FALSE;
}

OLC_FUN(worded_show)
{
	int i;
	WORD_DATA *w;
	LANG_DATA *l;

	EDIT_WORD(ch, w);
	l = varr_get(&langs, w->lang);

	char_printf(ch, "Name: [%s]\n"
			"Lang: [%s]\n"
			"Type: [%s]\n",
		    w->name,
		    l->name,
		    ch->desc->editor == ED_GENDER ?	"gender" :
		    ch->desc->editor == ED_CASE ?	"case" :
							"unknown");

	if (!IS_NULLSTR(w->base))
		char_printf(ch, "Base: [%s]\n", w->base);

	for (i = 0; i < w->f.nused; i++) {
		char **p = VARR_GET(&w->f, i);

		if (!IS_NULLSTR(*p))
			char_printf(ch, "Form: [%d] [%s]\n", i, *p);
	}

	return FALSE;
}

OLC_FUN(worded_list)
{
	int i;
	BUFFER *output = NULL;
	varr **hashp;
	LANG_DATA *l;
	char arg[MAX_STRING_LENGTH];

	argument = one_argument(argument, arg);
	if (arg[0] == '\0' || argument[0] == '\0') {
		do_help(ch, "'OLC WORD LIST'");
		return FALSE;
	}
	
	if ((i = lang_lookup(arg)) < 0) {
		char_printf(ch, "WordEd: %s: unknown language.\n", arg);
		return FALSE;
	}

	l = VARR_GET(&langs, i);
	EDIT_HASH(ch, l, hashp);

	for (i = 0; i < MAX_WORD_HASH; i++) {
		int j;
		varr *v = hashp[i];

		if (v == NULL)
			continue;

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
		char_puts("WordEd: no messages found.\n", ch);

	return FALSE;
}

OLC_FUN(worded_name)
{
	WORD_DATA *w;
	LANG_DATA *l;
	varr **hashp;

	if (argument[0] == '\0') {
		do_help(ch, "'OLC WORD'");
		return FALSE;
	}

	EDIT_WORD(ch, w);
	l = varr_get(&langs, w->lang);
	if (l == NULL)
		return FALSE;
	EDIT_HASH(ch, l, hashp);

	if (word_lookup(hashp, argument)) {
		char_printf(ch, "WordEd: %s: duplicate name.\n", argument);
		return FALSE;
	}

	word_del(hashp, w->name);
	free_string(w->name);
	w->name = str_dup(argument);
	ch->desc->pEdit = word_add(hashp, w);
	return TRUE;
}

OLC_FUN(worded_base)
{
	WORD_DATA *w;
	EDIT_WORD(ch, w);
	return olced_str(ch, argument, worded_base, &w->base);
}

OLC_FUN(worded_form)
{
	WORD_DATA *w;
	bool add;
	int num;
	char arg[MAX_STRING_LENGTH];
	char arg2[MAX_STRING_LENGTH];

	argument = one_argument(argument, arg);
	argument = one_argument(argument, arg2);

	if (!str_prefix(arg, "add"))
		add = TRUE;
	else if (!str_prefix(arg, "delete"))
		add = FALSE;
	else {
		do_help(ch, "'OLC WORD FORM'");
		return FALSE;
	}

	if ((add && argument[0] == '\0') || !is_number(arg2)) {
		do_help(ch, "'OLC WORD FORM'");
		return FALSE;
	}

	num = atoi(arg2);
	EDIT_WORD(ch, w);
	if (add)
		word_form_add(w, num, argument);
	else
		word_form_del(w, num);
	return FALSE;
}

OLC_FUN(worded_del)
{
	varr **hashp;
	WORD_DATA *w;
	LANG_DATA *l;

	EDIT_WORD(ch, w);
	l = varr_get(&langs, w->lang);
	if (l == NULL)
		return FALSE;
	EDIT_HASH(ch, l, hashp);

	word_del(hashp, w->name);
	edit_done(ch->desc);

	return FALSE;
}

