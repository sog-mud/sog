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
 * $Id: olc_hint.c,v 1.11 2004-06-28 19:21:04 tatyana Exp $
 */

#include "olc.h"

#include <lang.h>

#define EDIT_HINT(ch, hint)	(hint = (hint_t *) ch->desc->pEdit)

DECLARE_OLC_FUN(hinted_create		);
DECLARE_OLC_FUN(hinted_edit		);
DECLARE_OLC_FUN(hinted_save		);
DECLARE_OLC_FUN(hinted_touch		);
DECLARE_OLC_FUN(hinted_show		);
DECLARE_OLC_FUN(hinted_list		);

DECLARE_OLC_FUN(hinted_phrase		);
DECLARE_OLC_FUN(hinted_mask		);
DECLARE_OLC_FUN(hinted_delete		);

olc_cmd_t olc_cmds_hint[] =
{
	{ "create",	hinted_create,		NULL,	NULL		},
	{ "edit",	hinted_edit,		NULL,	NULL		},
	{ "",		hinted_save,		NULL,	NULL		},
	{ "touch",	hinted_touch,		NULL,	NULL		},
	{ "show",	hinted_show,		NULL,	NULL		},
	{ "list",	hinted_list,		NULL,	NULL		},

	{ "phrase",	hinted_phrase,		NULL,	NULL		},
	{ "mask",	hinted_mask,		NULL,	hint_levels	},
	{ "delete_hin",	olced_spell_out,	NULL,	NULL		},
	{ "delete_hint",hinted_delete,		NULL,	NULL		},

	{ "commands",	show_commands,		NULL,	NULL		},
	{ "version",	show_version,		NULL,	NULL		},

	{ NULL, NULL, NULL, NULL }
};

OLC_FUN(hinted_create)
{
	hint_t *hint;

	if (PC(ch)->security < SECURITY_HELP) {
		act_char("HintEd: Insufficient security for creating hints.", ch);
		return FALSE;
	}

	hint		= varr_enew(&hints);
	mlstr_init(&hint->phrase);
	hint->hint_level= HINT_ALL;

	ch->desc->pEdit	= (void *) hint;
	OLCED(ch)	= olced_lookup(ED_HINT);
	SET_BIT(changed_flags, CF_HINT);
	act_char("Hint created.", ch);
	return FALSE;
}

OLC_FUN(hinted_edit)
{
	hint_t *hint = NULL;

	if (PC(ch)->security < SECURITY_HELP) {
		act_char("HintEd: Insufficient security.", ch);
		return FALSE;
	}

	if (argument[0] == '\0')
		OLC_ERROR("'OLC EDIT'");

	if ((hint = hint_search(argument)) == NULL) {
		act_char("HintEd: no such hint.", ch);
		return FALSE;
	}

	ch->desc->pEdit	= hint;
	OLCED(ch)	= olced_lookup(ED_HINT);
	return FALSE;
}

OLC_FUN(hinted_save)
{
	FILE *fp;
	hint_t *hint;

	if (!IS_SET(changed_flags, CF_HINT)) {
		olc_printf(ch, "Hints are not changed.");
		return FALSE;
	}

	fp = olc_fopen(ETC_PATH, HINTS_FILE, ch, SECURITY_HELP);
	if (fp == NULL)
		return FALSE;

	C_FOREACH (hint_t *, hint, &hints) {
		if (mlstr_null(&hint->phrase))
			continue;

		fprintf(fp, "%s ", flag_string(hint_levels, hint->hint_level));
		mlstr_fwrite(fp, str_empty, &hint->phrase);
	}

	fprintf(fp, "0\n");
	fclose(fp);

	REMOVE_BIT(changed_flags, CF_HINT);
	olc_printf(ch, "Hints saved.");
	return FALSE;
}

OLC_FUN(hinted_touch)
{
	SET_BIT(changed_flags, CF_HINT);
	return FALSE;
}

OLC_FUN(hinted_show)
{
	BUFFER *output;
	hint_t *hint;

	if (argument[0] == '\0') {
		if (IS_EDIT(ch, ED_HINT))
			EDIT_HINT(ch, hint);
		else
			OLC_ERROR("'OLC ASHOW'");
	} else {
		if ((hint = hint_search(argument)) == NULL) {
			act_char("HintEd: No such hint.", ch);
			return FALSE;
		}
	}

	output = buf_new(0);

	buf_printf(output, BUF_END,
		   "Level: [%s]\n",
		   flag_string(hint_levels, hint->hint_level));
	mlstr_dump(output, "Hint:  ", &hint->phrase, DUMP_LEVEL(ch));

	page_to_char(buf_string(output), ch);
	buf_free(output);

	return FALSE;
}

OLC_FUN(hinted_list)
{
	BUFFER *output;
	hint_t *hint;

	output = buf_new(0);

	C_FOREACH (hint_t *, hint, &hints) {
		const char *name = mlstr_mval(&hint->phrase);

		if (IS_NULLSTR(name))
			continue;

		if (argument[0] == '\0' || strstr(name, argument)) {
			buf_printf(output, BUF_END, "[%3s] %s\n",
			    flag_string(hint_levels, hint->hint_level), name);
		}
	}

	page_to_char(buf_string(output), ch);
	buf_free(output);

	return FALSE;
}

OLC_FUN(hinted_mask)
{
	hint_t *hint;
	EDIT_HINT(ch, hint);
	return olced_flag(ch, argument, cmd, &hint->hint_level);
}

OLC_FUN(hinted_phrase)
{
	hint_t *hint;
	EDIT_HINT(ch, hint);
	return olced_mlstr(ch, argument, cmd, &hint->phrase);
}

OLC_FUN(hinted_delete)
{
	hint_t *hint;
	EDIT_HINT(ch, hint);

	if (olced_busy(ch, ED_HINT, NULL, NULL))
		return FALSE;

	varr_edelete(&hints, hint);
	edit_done(ch->desc);
	return TRUE;
}
