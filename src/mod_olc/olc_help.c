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
 * $Id: olc_help.c,v 1.23 1998-12-07 05:34:12 fjoe Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "merc.h"
#include "interp.h"
#include "olc.h"

#define EDIT_HELP(ch, help)	(help = (HELP_DATA*) ch->desc->pEdit)

DECLARE_OLC_FUN(helped_create		);
DECLARE_OLC_FUN(helped_edit		);
DECLARE_OLC_FUN(helped_touch		);
DECLARE_OLC_FUN(helped_show		);
DECLARE_OLC_FUN(helped_list		);

DECLARE_OLC_FUN(helped_level		);
DECLARE_OLC_FUN(helped_keyword		);
DECLARE_OLC_FUN(helped_text		);
DECLARE_OLC_FUN(helped_del		);

OLC_CMD_DATA olc_cmds_help[] =
{
	{ "create",	helped_create	},
	{ "edit",	helped_edit	},
	{ "touch",	helped_touch	},
	{ "show",	helped_show	},
	{ "list",	helped_list	},

	{ "level",	helped_level	},
	{ "keywords",	helped_keyword	},
	{ "text",	helped_text	},
	{ "del",	helped_del	},

	{ "commands",	show_commands	},
	{ NULL }
};

OLC_FUN(helped_create)
{
	HELP_DATA *pHelp;
	AREA_DATA *pArea;

	if (ch->pcdata->security < SECURITY_HELP) {
		char_puts("HEdit: Insufficient security.\n", ch);
		return FALSE;
	}

	if (argument[0] == '\0') {
		do_help(ch, "'OLC CREATE'");
		return FALSE;
	}

	if ((pHelp = help_lookup(1, argument)) != NULL) {
		char_printf(ch,
			    "HEdit: Help already exists in area %s (%s).\n",
			    pHelp->area->name, pHelp->area->file_name);
		return FALSE;
	}

	if (ch->desc->editor == ED_HELP)
		pArea = ((HELP_DATA*) ch->desc->pEdit)->area;
	else if (ch->desc->editor == ED_AREA)
		pArea = ((AREA_DATA*) ch->desc->pEdit);
	else { 
		char_puts("You must be editing an area or another help to create helps.\n",
			  ch);
		return FALSE;
	}

	pHelp			= help_new();
	pHelp->level		= 0;
	pHelp->keyword		= str_dup(argument);
	pHelp->text		= NULL;
	help_add(pArea, pHelp);

	ch->desc->pEdit		= (void *)pHelp;
	ch->desc->editor	= ED_HELP;
	touch_area(pArea);
	char_puts("Help created.\n",ch);
	return FALSE;
}

OLC_FUN(helped_edit)
{
	int num;
	char keyword[MAX_STRING_LENGTH];
	HELP_DATA *pHelp;

	if (ch->pcdata->security < SECURITY_HELP) {
		char_puts("HEdit: Insufficient security.\n", ch);
		return FALSE;
	}

	num = number_argument(argument, keyword);
	if (keyword[0] == '\0') {
		do_help(ch, "'OLC EDIT'");
		return FALSE;
	}

	if ((pHelp = help_lookup(num, keyword)) == NULL) {
		char_printf(ch, "HEdit: %s: Help not found.\n",
			    keyword);
		return FALSE;
	}

	ch->desc->pEdit		= (void *)pHelp;
	ch->desc->editor	= ED_HELP;
	return FALSE;
}

OLC_FUN(helped_touch)
{
	HELP_DATA *pHelp;
	EDIT_HELP(ch, pHelp);
	return touch_area(pHelp->area);
}

OLC_FUN(helped_show)
{
	BUFFER *output;
	HELP_DATA *pHelp;

	if (argument[0] == '\0') {
		if (ch->desc->editor == ED_HELP)
			EDIT_HELP(ch, pHelp);
		else {
			do_help(ch, "'OLC ASHOW'");
			return FALSE;
		}
	}
	else {
		int num;
		char keyword[MAX_INPUT_LENGTH];

		num = number_argument(argument, keyword);
		if (keyword[0] == '\0') {
			if (ch->desc->editor)
			do_help(ch, ch->desc->editor ?
				    "'OLC EDIT'" : "'OLC ASHOW'");
			return FALSE;
		}

		if ((pHelp = help_lookup(num, keyword)) == NULL) {
			char_printf(ch, "HEdit: %s: Help not found.\n",
				    keyword);
			return FALSE;
		}
	}

	output = buf_new(-1);
	buf_printf(output,
		   "Level:    [%d]\n"
		   "Keywords: [%s]\n",
		   pHelp->level, pHelp->keyword);
	mlstr_dump(output, "Text:     ", pHelp->text);
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

	one_argument(argument, arg);
	if (arg[0] == '\0') {
		if ((pArea = get_edited_area(ch)) == NULL) {
			do_help(ch, "'OLC ALIST'");
			return FALSE;
		}
	}

	output = buf_new(-1);

	if (pArea) {
		buf_printf(output, "Available help topics in area '%s' [%d]:\n",
			   pArea->name, pArea->vnum);
		for (pHelp = pArea->help_first; pHelp;
						pHelp = pHelp->next_in_area)
			buf_printf(output, "    o %s\n", pHelp->keyword);
	}
	else {
		buf_printf(output, "Available help topics on keyword '%s':\n",
			   arg);
		for (pHelp = help_first; pHelp; pHelp = pHelp->next)
			if (is_name(arg, pHelp->keyword))
				buf_printf(output, "    o %s\n",
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
	return olced_number(ch, argument, helped_level, &pHelp->level);
}

OLC_FUN(helped_keyword)
{
	HELP_DATA *pHelp;
	EDIT_HELP(ch, pHelp);
	return olced_str(ch, argument, helped_keyword, &pHelp->keyword);
}
		
OLC_FUN(helped_text)
{
	HELP_DATA *pHelp;
	EDIT_HELP(ch, pHelp);
	return olced_mlstr_text(ch, argument, helped_text, &pHelp->text);
}

OLC_FUN(helped_del)
{
	HELP_DATA *pHelp;
	EDIT_HELP(ch, pHelp);
	touch_area(pHelp->area);
	help_free(pHelp);
	char_puts("HEdit: Help deleted.\n", ch);
	edit_done(ch->desc);
	return FALSE;
}

