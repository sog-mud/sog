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
 * $Id: olc_help.c,v 1.49 2004-02-10 14:15:52 fjoe Exp $
 */

#include "olc.h"

#define EDIT_HELP(ch, help)	(help = (HELP_DATA *) ch->desc->pEdit)

DECLARE_OLC_FUN(helped_create		);
DECLARE_OLC_FUN(helped_edit		);
DECLARE_OLC_FUN(helped_touch		);
DECLARE_OLC_FUN(helped_show		);
DECLARE_OLC_FUN(helped_list		);
DECLARE_OLC_FUN(helped_del		);

DECLARE_OLC_FUN(helped_level		);
DECLARE_OLC_FUN(helped_keyword		);
DECLARE_OLC_FUN(helped_text		);

olc_cmd_t olc_cmds_help[] =
{
	{ "create",	helped_create,	NULL,	NULL		},
	{ "edit",	helped_edit,	NULL,	NULL		},
	{ "",		NULL,		NULL,	NULL		},
	{ "touch",	helped_touch,	NULL,	NULL		},
	{ "show",	helped_show,	NULL,	NULL		},
	{ "list",	helped_list,	NULL,	NULL		},
	{ "delete_hel",	olced_spell_out, NULL,	NULL		},
	{ "delete_help",helped_del,	NULL,	NULL		},

	{ "level",	helped_level,	NULL,	level_table	},
	{ "keywords",	helped_keyword,	NULL,	NULL		},
	{ "text",	helped_text,	NULL,	NULL		},

	{ "commands",	show_commands,	NULL,	NULL		},
	{ "version",	show_version,	NULL,	NULL		},

	{ NULL, NULL, NULL, NULL }
};

OLC_FUN(helped_create)
{
	HELP_DATA *pHelp;
	AREA_DATA *pArea;

	if (PC(ch)->security < SECURITY_HELP) {
		act_char("HelpEd: Insufficient security.", ch);
		return FALSE;
	}

	if (argument[0] == '\0')
		OLC_ERROR("'OLC CREATE'");

	if (IS_EDIT(ch, ED_HELP))
		pArea = ((HELP_DATA*) ch->desc->pEdit)->area;
	else if (IS_EDIT(ch, ED_AREA))
		pArea = ((AREA_DATA*) ch->desc->pEdit);
	else { 
		act_char("You must be editing an area or another help to create helps.", ch);
		return FALSE;
	}

	pHelp		= help_new();
	pHelp->level	= 0;
	pHelp->keyword	= str_dup(argument);
	help_add(pArea, pHelp);

	ch->desc->pEdit	= (void*) pHelp;
	OLCED(ch)	= olced_lookup(ED_HELP);
	TOUCH_AREA(pArea);
	act_char("Help created.", ch);
	return FALSE;
}

OLC_FUN(helped_edit)
{
	int num;
	char keyword[MAX_STRING_LENGTH];
	HELP_DATA *pHelp;

	if (PC(ch)->security < SECURITY_HELP) {
		act_char("HelpEd: Insufficient security.", ch);
		return FALSE;
	}

	num = number_argument(argument, keyword, sizeof(keyword));
	if (keyword[0] == '\0')
		OLC_ERROR("'OLC EDIT'");

	if ((pHelp = help_lookup(num, keyword)) == NULL) {
		act_puts("HelpEd: $t: Help not found.",
			 ch, keyword, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	ch->desc->pEdit	= (void*) pHelp;
	OLCED(ch)	= olced_lookup(ED_HELP);
	return FALSE;
}

OLC_FUN(helped_touch)
{
	HELP_DATA *pHelp;
	EDIT_HELP(ch, pHelp);
	TOUCH_AREA(pHelp->area);
	return FALSE;
}

OLC_FUN(helped_show)
{
	BUFFER *output;
	HELP_DATA *pHelp;

	if (argument[0] == '\0') {
		if (IS_EDIT(ch, ED_HELP))
			EDIT_HELP(ch, pHelp);
		else
			OLC_ERROR("'OLC ASHOW'");
	}
	else {
		int num;
		char keyword[MAX_INPUT_LENGTH];

		num = number_argument(argument, keyword, sizeof(keyword));
		if (keyword[0] == '\0')
			OLC_ERROR(OLCED(ch) ?  "'OLC EDIT'" : "'OLC ASHOW'");

		if ((pHelp = help_lookup(num, keyword)) == NULL) {
			act_puts("HelpEd: $t: Help not found.",
				 ch, keyword, NULL,
				 TO_CHAR | ACT_NOTRANS, POS_DEAD);
			return FALSE;
		}
	}

	output = buf_new(0);
	buf_printf(output, BUF_END,
		   "Level:    [%s]\n"
		   "Keywords: [%s]\n",
		   flag_string_int(level_table, pHelp->level),
		   pHelp->keyword);
	mlstr_dump(output, "Text:     ", &pHelp->text, DUMP_LEVEL(ch));
	page_to_char(buf_string(output), ch);
	buf_free(output);

	return FALSE;
}

extern HELP_DATA *help_first;

OLC_FUN(helped_list)
{
	BUFFER *output;
	AREA_DATA *pArea = NULL;
	HELP_DATA *pHelp;
	char arg[MAX_INPUT_LENGTH];

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		if ((pArea = get_edited_area(ch)) == NULL)
			OLC_ERROR("'OLC ALIST'");
	}

	output = buf_new(0);

	if (pArea) {
		buf_printf(output, BUF_END, "Available help topics in area '%s' [%d]:\n",
			   pArea->name, pArea->vnum);
		for (pHelp = pArea->help_first; pHelp;
						pHelp = pHelp->next_in_area)
			buf_printf(output, BUF_END, "    o %s\n", pHelp->keyword);
	}
	else {
		buf_printf(output, BUF_END, "Available help topics on keyword '%s':\n",
			   arg);
		for (pHelp = help_first; pHelp; pHelp = pHelp->next)
			if (is_name(arg, pHelp->keyword))
				buf_printf(output, BUF_END, "    o %s\n",
					   pHelp->keyword);
	}

	page_to_char(buf_string(output), ch);
	buf_free(output);
	return FALSE;
}

OLC_FUN(helped_level)
{
	HELP_DATA *pHelp;
	EDIT_HELP(ch, pHelp);
	return olced_ival(ch, argument, cmd, &pHelp->level);
}

OLC_FUN(helped_keyword)
{
	HELP_DATA *pHelp;
	EDIT_HELP(ch, pHelp);
	return olced_str(ch, argument, cmd, &pHelp->keyword);
}
		
OLC_FUN(helped_text)
{
	HELP_DATA *pHelp;
	EDIT_HELP(ch, pHelp);
	return olced_mlstr_text(ch, argument, cmd, &pHelp->text);
}

OLC_FUN(helped_del)
{
	HELP_DATA *pHelp;
	EDIT_HELP(ch, pHelp);

	if (olced_busy(ch, ED_HELP, pHelp, NULL))
		return FALSE;

	TOUCH_AREA(pHelp->area);
	help_free(pHelp);
	act_char("HelpEd: Help deleted.", ch);
	edit_done(ch->desc);
	return FALSE;
}

