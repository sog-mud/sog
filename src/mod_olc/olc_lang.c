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
 * $Id: olc_lang.c,v 1.16 1999-06-10 14:33:36 fjoe Exp $
 */

#include <stdio.h>
#include <string.h>

#include "merc.h"
#include "olc.h"
#include "db/lang.h"

#define EDIT_LANG(ch, l)	(l = (lang_t*) (ch->desc->pEdit))

DECLARE_OLC_FUN(langed_create	);
DECLARE_OLC_FUN(langed_edit	);
DECLARE_OLC_FUN(langed_touch	);
DECLARE_OLC_FUN(langed_show	);
DECLARE_OLC_FUN(langed_list	);

DECLARE_OLC_FUN(langed_name	);
DECLARE_OLC_FUN(langed_flags	);
DECLARE_OLC_FUN(langed_slangof	);
DECLARE_OLC_FUN(langed_filename	);
DECLARE_OLC_FUN(langed_rulecl	);

DECLARE_VALIDATE_FUN(validate_langname);

olc_cmd_t olc_cmds_lang[] =
{
	{ "create",	langed_create					},
	{ "edit",	langed_edit					},
	{ "touch",	langed_touch					},
	{ "show",	langed_show					},
	{ "list",	langed_list					},

	{ "name",	langed_name,		validate_langname	},
	{ "flags",	langed_flags,		lang_flags		},
	{ "slangof",	langed_slangof					},
	{ "filename",	langed_filename,	validate_filename	},
	{ "ruleclass",	langed_rulecl					},

	{ "commands",	show_commands					},
	{ NULL }
};

OLC_FUN(langed_create)
{
	lang_t *l;
	char arg[MAX_INPUT_LENGTH];

	if (ch->pcdata->security < 9) {
		char_puts("LangEd: Insufficient security.\n", ch);
		return FALSE;
	}

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		do_help(ch, "'OLC CREATE'");
		return FALSE;
	}

	if (lang_lookup(arg) >= 0) {
		char_puts("LangEd: lang already exists.\n", ch);
		return FALSE;
	}

	l = lang_new();
	l->name = str_dup(arg);
	l->file_name = str_printf("lang%02d.lang", langs.nused-1);
	ch->desc->pEdit	= l;
	OLCED(ch)	= olced_lookup(ED_LANG);
	char_puts("LangEd: lang created.\n", ch);
	SET_BIT(l->lang_flags, LANG_CHANGED);
	return FALSE;
}

OLC_FUN(langed_edit)
{
	int lang;
	char arg[MAX_INPUT_LENGTH];

	if (ch->pcdata->security < SECURITY_MSGDB) {
		char_puts("LangEd: Insufficient security", ch);
		return FALSE;
	}

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		do_help(ch, "'OLC EDIT'");
		return FALSE;
	}

	if ((lang = lang_lookup(arg)) < 0) {
		char_puts("LangEd: language not found.\n", ch);
		return FALSE;
	}

	ch->desc->pEdit = VARR_GET(&langs, lang);
	OLCED(ch)	= olced_lookup(ED_LANG);
	return FALSE;
}

OLC_FUN(langed_touch)
{
	lang_t *l;
	EDIT_LANG(ch, l);
	SET_BIT(l->lang_flags, LANG_CHANGED);
	return FALSE;
}

OLC_FUN(langed_show)
{
	int i;
	char arg[MAX_INPUT_LENGTH];
	lang_t *l;
	lang_t *sl;

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		if (IS_EDIT(ch, ED_LANG))
			EDIT_LANG(ch, l);
		else {
			do_help(ch, "'OLC ASHOW'");
			return FALSE;
		}
	}
	else {
		int lang;

		if ((lang = lang_lookup(arg)) < 0) {
			char_puts("LangEd: language not found.\n", ch);
			return FALSE;
		}
		l = VARR_GET(&langs, lang);
	}

	char_printf(ch, "Name:     [%s]\n"
			"Filename: [%s]\n",
		    l->name, l->file_name);
	if ((sl = varr_get(&langs, l->slang_of)))
		char_printf(ch, "Slang of: [%s]\n", sl->name);
	if (l->lang_flags) {
		char_printf(ch, "Flags:    [%s]\n",
			    flag_string(lang_flags, l->lang_flags)); 
	}

	for (i = 0; i < MAX_RULECL; i++) {
		rulecl_t *rcl = l->rules + i;

		if (IS_NULLSTR(rcl->file_expl)
		&&  IS_NULLSTR(rcl->file_impl))
			continue;

		char_printf(ch, "\nRule Class: [%s]\n"
				"Expl file: [%s]\n" 
				"Impl file: [%s]\n"
				"Flags: [%s]\n",
			    flag_string(rulecl_names, i),
			    rcl->file_expl,
			    rcl->file_impl,
			    flag_string(rulecl_flags, rcl->rcl_flags));
	}

	return FALSE;
}

OLC_FUN(langed_list)
{
	int lang;

	for (lang = 0; lang < langs.nused; lang++) {
		lang_t *l = VARR_GET(&langs, lang);
		char_printf(ch, "[%d] %s\n", lang, l->name);
	}

	return FALSE;
}

OLC_FUN(langed_name)
{
	lang_t *l;

	if (ch->pcdata->security < 9) {
		char_puts("LangEd: Insufficient security", ch);
		return FALSE;
	}

	EDIT_LANG(ch, l);
	return olced_str(ch, argument, cmd, &l->name);
}

OLC_FUN(langed_flags)
{
	lang_t *l;
	EDIT_LANG(ch, l);
	return olced_flag32(ch, argument, cmd, &l->lang_flags);
}

OLC_FUN(langed_slangof)
{
	char arg[MAX_STRING_LENGTH];
	lang_t *l;
	int lang;

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		do_help(ch, "'OLC LANG SLANG'");
		return FALSE;
	}

	if ((lang = lang_lookup(arg)) < 0) {
		char_puts("LangEd: language not found.\n", ch);
		return FALSE;
	}

	EDIT_LANG(ch, l);
	l->slang_of = lang;
	return TRUE;
}

OLC_FUN(langed_filename)
{
	lang_t *l;
	EDIT_LANG(ch, l);
	return olced_str(ch, argument, cmd, &l->file_name);
}

OLC_FUN(langed_rulecl)
{
	lang_t *l;
	EDIT_LANG(ch, l);
	return olced_rulecl(ch, argument, cmd, l);
}

/* local functions */

VALIDATE_FUN(validate_langname)
{
	if (lang_lookup(arg) >= 0) {
		char_printf(ch, "%s: language already exists.\n",
			    OLCED(ch)->name);
		return FALSE;
	}
	return TRUE;
}

bool touch_lang(lang_t *l)
{
	return FALSE;
}

