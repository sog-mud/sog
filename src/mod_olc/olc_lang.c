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
 * $Id: olc_lang.c,v 1.20.2.1 1999-12-16 12:39:55 fjoe Exp $
 */

#include "olc.h"
#include "lang.h"

#define EDIT_LANG(ch, l)	(l = (lang_t*) (ch->desc->pEdit))

DECLARE_OLC_FUN(langed_create	);
DECLARE_OLC_FUN(langed_edit	);
DECLARE_OLC_FUN(langed_save	);
DECLARE_OLC_FUN(langed_touch	);
DECLARE_OLC_FUN(langed_show	);
DECLARE_OLC_FUN(langed_list	);

DECLARE_OLC_FUN(langed_name	);
DECLARE_OLC_FUN(langed_flags	);
DECLARE_OLC_FUN(langed_slangof	);
DECLARE_OLC_FUN(langed_filename	);
DECLARE_OLC_FUN(langed_rulecl	);

static DECLARE_VALIDATE_FUN(validate_langname);

olc_cmd_t olc_cmds_lang[] =
{
	{ "create",	langed_create					},
	{ "edit",	langed_edit					},
	{ "",		langed_save					},
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

static bool save_lang(CHAR_DATA *ch, lang_t *l);

OLC_FUN(langed_create)
{
	lang_t *l;
	char arg[MAX_INPUT_LENGTH];

	if (PC(ch)->security < 9) {
		char_puts("LangEd: Insufficient security.\n", ch);
		return FALSE;
	}

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		dofun("help", ch, "'OLC CREATE'");
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

	if (PC(ch)->security < SECURITY_MSGDB) {
		char_puts("LangEd: Insufficient security", ch);
		return FALSE;
	}

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		dofun("help", ch, "'OLC EDIT'");
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

OLC_FUN(langed_save)
{
	int lang;
	FILE *fp;

	fp = olc_fopen(LANG_PATH, LANG_LIST, ch, SECURITY_MSGDB);
	if (fp == NULL)
		return FALSE;

	for (lang = 0; lang < langs.nused; lang++) {
		lang_t *l = VARR_GET(&langs, lang);

		fprintf(fp, "%s\n", l->file_name);
		if (IS_SET(l->lang_flags, LANG_CHANGED)
		&&  save_lang(ch, l)) {
			olc_printf(ch, "Language '%s' saved (%s%c%s).",
				   l->name, LANG_PATH, PATH_SEPARATOR,
				   l->file_name);
			l->lang_flags &= ~LANG_CHANGED;
		}
	}

	fprintf(fp, "$\n");
	fclose(fp);
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
			dofun("help", ch, "'OLC ASHOW'");
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

	if (PC(ch)->security < 9) {
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
		dofun("help", ch, "'OLC LANG SLANG'");
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

static VALIDATE_FUN(validate_langname)
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

static bool save_lang(CHAR_DATA *ch, lang_t *l)
{
	int i;
	FILE *fp;
	lang_t *sl;
	int flags;

	fp = olc_fopen(LANG_PATH, l->file_name, ch, -1);
	if (fp == NULL)
		return FALSE;

	fprintf(fp, "#LANG\n"
		    "Name %s\n", l->name);
	if ((sl = varr_get(&langs, l->slang_of)))
		fprintf(fp, "SlangOf %s\n", sl->name);
	flags = l->lang_flags & ~LANG_CHANGED;
	if (flags)
		fprintf(fp, "Flags %s~\n", flag_string(lang_flags, flags));
	fprintf(fp, "End\n\n");

	for (i = 0; i < MAX_RULECL; i++) {
		rulecl_t *rcl = l->rules + i;

		if (!IS_NULLSTR(rcl->file_impl)
		||  !IS_NULLSTR(rcl->file_expl)) {
			fprintf(fp, "#RULECLASS\n"
				    "Class %s\n",
				flag_string(rulecl_names, i));
			fwrite_string(fp, "Impl", rcl->file_impl);
			fwrite_string(fp, "Expl", rcl->file_expl);
			fprintf(fp, "End\n\n");
		}
	}

	fprintf(fp, "#$\n");
	fclose(fp);
	return TRUE;
}

