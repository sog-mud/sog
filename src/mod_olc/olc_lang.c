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
 * $Id: olc_lang.c,v 1.34 2001-08-25 04:46:53 fjoe Exp $
 */

#include "olc.h"

#include <lang.h>

#define EDIT_LANG(ch, l)	(l = (lang_t *) (ch->desc->pEdit))

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
	{ "create",	langed_create,	NULL,		NULL		},
	{ "edit",	langed_edit,	NULL,		NULL		},
	{ "",		langed_save,	NULL,		NULL		},
	{ "touch",	langed_touch,	NULL,		NULL		},
	{ "show",	langed_show,	NULL,		NULL		},
	{ "list",	langed_list,	NULL,		NULL		},

	{ "name",	langed_name,	validate_langname, NULL		},
	{ "flags",	langed_flags,	NULL,		lang_flags	},
	{ "slangof",	langed_slangof,	NULL,		NULL		},
	{ "filename",	langed_filename, validate_filename, NULL	},
	{ "ruleclass",	langed_rulecl,	validate_filename, rulecl_flags	},

	{ "commands",	show_commands,	NULL,		NULL		},
	{ "version",	show_version,	NULL,		NULL		},

	{ NULL, NULL, NULL, NULL }
};

static bool save_lang(CHAR_DATA *ch, lang_t *l);

OLC_FUN(langed_create)
{
	lang_t *l;
	char arg[MAX_INPUT_LENGTH];

	if (PC(ch)->security < SECURITY_LANG) {
		act_char("LangEd: Insufficient security.", ch);
		return FALSE;
	}

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0')
		OLC_ERROR("'OLC CREATE'");

	if (lang_lookup(arg) != NULL) {
		act_char("LangEd: lang already exists.", ch);
		return FALSE;
	}

	l = varr_enew(&langs);
	l->name = str_dup(arg);
	l->file_name = str_printf("lang%02d.lang", langs.nused-1);
	ch->desc->pEdit	= l;
	OLCED(ch)	= olced_lookup(ED_LANG);
	act_char("LangEd: lang created.", ch);
	SET_BIT(l->lang_flags, LANG_CHANGED);
	return FALSE;
}

OLC_FUN(langed_edit)
{
	lang_t *lang;
	char arg[MAX_INPUT_LENGTH];

	if (PC(ch)->security < SECURITY_LANG) {
		act_char("LangEd: Insufficient security", ch);
		return FALSE;
	}

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0')
		OLC_ERROR("'OLC EDIT'");

	if ((lang = lang_lookup(arg)) == NULL) {
		act_char("LangEd: language not found.", ch);
		return FALSE;
	}

	ch->desc->pEdit = lang;
	OLCED(ch)	= olced_lookup(ED_LANG);
	return FALSE;
}

OLC_FUN(langed_save)
{
	size_t lang;
	FILE *fp;

	fp = olc_fopen(LANG_PATH, LANG_LIST, ch, SECURITY_LANG);
	if (fp == NULL)
		return FALSE;

	for (lang = 0; lang < varr_size(&langs); lang++) {
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

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		if (IS_EDIT(ch, ED_LANG))
			EDIT_LANG(ch, l);
		else
			OLC_ERROR("'OLC ASHOW'");
	} else {
		if ((l = lang_lookup(arg)) == NULL) {
			act_char("LangEd: language not found.", ch);
			return FALSE;
		}
	}

	act_puts("Name:     [$t]",
		 ch, l->name, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
	act_puts("Filename: [$t]",
		 ch, l->file_name, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
	if (l->slang_of) {
		act_puts("Slang of: [$t]",
			 ch, l->slang_of->name, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
	}
	if (l->lang_flags) {
		act_puts("Flags:    [$t]",
			 ch, flag_string(lang_flags, l->lang_flags), NULL,
			 TO_CHAR | ACT_NOTRANS, POS_DEAD); 
	}

	for (i = 0; i < MAX_RULECL; i++) {
		rulecl_t *rcl = l->rules + i;

		if (IS_NULLSTR(rcl->file_expl)
		&&  IS_NULLSTR(rcl->file_impl))
			continue;

		act_puts("\nRule Class: [$t]",
			 ch, flag_string(rulecl_names, i), NULL,
			 TO_CHAR | ACT_NOTRANS, POS_DEAD);
		act_puts("Expl file: [$t]",
			 ch, rcl->file_expl, NULL,
			 TO_CHAR | ACT_NOTRANS, POS_DEAD);
		act_puts("Impl file: [$t]",
			 ch, rcl->file_impl, NULL,
			 TO_CHAR | ACT_NOTRANS, POS_DEAD);
		act_puts("Flags: [$t]",
			 ch, flag_string(rulecl_flags, rcl->rcl_flags), NULL,
			 TO_CHAR | ACT_NOTRANS, POS_DEAD);
	}

	return FALSE;
}

OLC_FUN(langed_list)
{
	size_t lang;

	for (lang = 0; lang < varr_size(&langs); lang++) {
		lang_t *l = VARR_GET(&langs, lang);
		act_puts("[$j] $T",
			 ch, (const void *) lang, l->name,
			 TO_CHAR | ACT_NOTRANS, POS_DEAD);
	}

	return FALSE;
}

OLC_FUN(langed_name)
{
	lang_t *l;

	if (PC(ch)->security < 9) {
		act_char("LangEd: Insufficient security", ch);
		return FALSE;
	}

	EDIT_LANG(ch, l);
	return olced_str(ch, argument, cmd, &l->name);
}

OLC_FUN(langed_flags)
{
	lang_t *l;
	EDIT_LANG(ch, l);
	return olced_flag(ch, argument, cmd, &l->lang_flags);
}

OLC_FUN(langed_slangof)
{
	char arg[MAX_STRING_LENGTH];
	lang_t *l;
	lang_t *lang;

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0')
		OLC_ERROR("'OLC LANG SLANG'");

	if (!strcmp(arg, "none"))
		lang = NULL;
	else if ((lang = lang_lookup(arg)) == NULL) {
		act_char("LangEd: language not found.", ch);
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

static
VALIDATE_FUN(validate_langname)
{
	if (lang_lookup(arg) == NULL) {
		act_puts("$t: language already exists.",
			 ch, OLCED(ch)->name, NULL,
			 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
		return FALSE;
	}
	return TRUE;
}

static bool
save_lang(CHAR_DATA *ch, lang_t *l)
{
	int i;
	FILE *fp;
	int flags;

	fp = olc_fopen(LANG_PATH, l->file_name, ch, -1);
	if (fp == NULL)
		return FALSE;

	fprintf(fp, "#LANG\n"
		    "Name %s\n", l->name);
	if (l->slang_of != NULL && l != l->slang_of)
		fprintf(fp, "SlangOf %s\n", l->slang_of->name);
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
