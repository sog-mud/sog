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
 * $Id: olc_lang.c,v 1.9 1999-02-17 04:25:25 fjoe Exp $
 */

#include <stdio.h>
#include <string.h>

#include "merc.h"
#include "olc.h"
#include "db/lang.h"

#define EDIT_LANG(ch, l)	(l = (LANG_DATA*) (ch->desc->pEdit))

DECLARE_OLC_FUN(langed_create	);
DECLARE_OLC_FUN(langed_edit	);
DECLARE_OLC_FUN(langed_touch	);
DECLARE_OLC_FUN(langed_show	);
DECLARE_OLC_FUN(langed_list	);

DECLARE_OLC_FUN(langed_name	);
DECLARE_OLC_FUN(langed_flags	);
DECLARE_OLC_FUN(langed_slangof	);
DECLARE_OLC_FUN(langed_filename	);
DECLARE_OLC_FUN(langed_genders	);
DECLARE_OLC_FUN(langed_cases	);
DECLARE_OLC_FUN(langed_qtys	);

DECLARE_VALIDATE_FUN(validate_langname);

OLC_CMD_DATA olc_cmds_lang[] =
{
	{ "create",	langed_create				},
	{ "edit",	langed_edit				},
	{ "touch",	langed_touch				},
	{ "show",	langed_show				},
	{ "list",	langed_list				},

	{ "name",	langed_name,	validate_langname	},
	{ "flags",	langed_flags,	lang_flags		},
	{ "slangof",	langed_slangof				},
	{ "filename",	langed_filename,validate_filename	},
	{ "genders",	langed_genders,	validate_filename	},
	{ "cases",	langed_cases,	validate_filename	},
	{ "qtys",	langed_qtys,	validate_filename	},

	{ "commands",	show_commands				},
	{ NULL }
};

OLC_FUN(langed_create)
{
	LANG_DATA *l;
	char arg[MAX_INPUT_LENGTH];

	if (ch->pcdata->security < 9) {
		char_puts("LangEd: Insufficient security.\n", ch);
		return FALSE;
	}

	one_argument(argument, arg);
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
	ch->desc->pEdit = l;
	ch->desc->editor = ED_LANG;
	char_puts("LangEd: lang created.\n", ch);
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

	one_argument(argument, arg);
	if (arg[0] == '\0') {
		do_help(ch, "'OLC EDIT'");
		return FALSE;
	}

	if ((lang = lang_lookup(arg)) < 0) {
		char_puts("LangEd: language not found.\n", ch);
		return FALSE;
	}

	ch->desc->pEdit = VARR_GET(&langs, lang);
	ch->desc->editor = ED_LANG;
	return FALSE;
}

OLC_FUN(langed_touch)
{
	LANG_DATA *l;
	EDIT_LANG(ch, l);
	return touch_lang(l, ch->desc->editor);
}

OLC_FUN(langed_show)
{
	char arg[MAX_INPUT_LENGTH];
	LANG_DATA *l;
	LANG_DATA *sl;

	one_argument(argument, arg);
	if (arg[0] == '\0') {
		if (ch->desc->editor == ED_LANG)
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
	if (!IS_NULLSTR(l->file_genders))
		char_printf(ch, "Genders:  [%s]\n", l->file_genders);
	if (!IS_NULLSTR(l->file_cases))
		char_printf(ch, "Cases:    [%s]\n", l->file_cases);
	if (l->flags)
		char_printf(ch, "Flags:    [%s]\n",
			    flag_string(lang_flags, l->flags)); 
	return FALSE;
}

OLC_FUN(langed_list)
{
	int lang;

	for (lang = 0; lang < langs.nused; lang++) {
		LANG_DATA *l = VARR_GET(&langs, lang);
		char_printf(ch, "[%d] %s\n", lang, l->name);
	}

	return FALSE;
}

OLC_FUN(langed_name)
{
	LANG_DATA *l;

	if (ch->pcdata->security < 9) {
		char_puts("LangEd: Insufficient security", ch);
		return FALSE;
	}

	EDIT_LANG(ch, l);
	return olced_str(ch, argument, langed_name, &l->name);
}

OLC_FUN(langed_flags)
{
	LANG_DATA *l;
	EDIT_LANG(ch, l);
	return olced_flag32(ch, argument, langed_flags, &l->flags);
}

OLC_FUN(langed_slangof)
{
	char arg[MAX_STRING_LENGTH];
	LANG_DATA *l;
	int lang;

	one_argument(argument, arg);
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
	LANG_DATA *l;
	EDIT_LANG(ch, l);
	return olced_str(ch, argument, langed_filename, &l->file_name);
}

OLC_FUN(langed_genders)
{
	LANG_DATA *l;
	EDIT_LANG(ch, l);
	return olced_str(ch, argument, langed_genders, &l->file_genders);
}

OLC_FUN(langed_cases)
{
	LANG_DATA *l;
	EDIT_LANG(ch, l);
	return olced_str(ch, argument, langed_cases, &l->file_cases);
}

OLC_FUN(langed_qtys)
{
	LANG_DATA *l;
	EDIT_LANG(ch, l);
	return olced_str(ch, argument, langed_qtys, &l->file_qtys);
}

bool touch_lang(LANG_DATA *l, const char *editor)
{
	if (editor == ED_GENDER)
		SET_BIT(l->flags, LANG_GENDERS_CHANGED);
	else if (editor == ED_CASE)
		SET_BIT(l->flags, LANG_CASES_CHANGED);
	else if (editor == ED_QTY)
		SET_BIT(l->flags, LANG_QTYS_CHANGED);
	else
		SET_BIT(l->flags, LANG_CHANGED);
	return FALSE;
}

/* local functions */

VALIDATE_FUN(validate_langname)
{
	if (lang_lookup(arg) >= 0) {
		char_printf(ch, "%s: language already exists.\n",
			    olc_ed_name(ch));
		return FALSE;
	}
	return TRUE;
}

