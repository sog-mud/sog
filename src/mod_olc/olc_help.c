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
 * $Id: olc_help.c,v 1.17 1998-10-02 04:48:47 fjoe Exp $
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
	char arg[MAX_STRING_LENGTH];

	if (ch->pcdata->security < SECURITY_HELP) {
		char_puts("HEdit: Insufficient security.\n\r", ch);
		return FALSE;
	}

	first_arg(argument, arg, FALSE);
	if (arg[0] == '\0') {
		do_help(ch, "'OLC CREATE'");
		return FALSE;
	}

	if ((pHelp = help_lookup(1, arg)) != NULL) {
		char_printf(ch,
			    "HEdit: Help already exists in area %s (%s).\n\r",
			    pHelp->area->name, pHelp->area->file_name);
		return FALSE;
	}

	if (ch->desc->editor == ED_HELP)
		pArea = ((HELP_DATA*) ch->desc->pEdit)->area;
	else if (ch->desc->editor == ED_AREA)
		pArea = ((AREA_DATA*) ch->desc->pEdit);
	else { 
		char_puts("You must be editing an area or another help to create helps.\n\r",
			  ch);
		return FALSE;
	}

	pHelp			= help_new();
	pHelp->level		= 0;
	pHelp->keyword		= str_dup(arg);
	pHelp->text		= NULL;
	help_add(pArea, pHelp);

	ch->desc->pEdit		= (void *)pHelp;
	ch->desc->editor	= ED_HELP;
	touch_area(pArea);
	char_puts("Help created.\n\r",ch);
	return FALSE;
}

OLC_FUN(helped_edit)
{
	int num;
	char keyword[MAX_STRING_LENGTH];
	HELP_DATA *pHelp;

	if (ch->pcdata->security < SECURITY_HELP) {
		char_puts("HEdit: Insufficient security.\n\r", ch);
		return FALSE;
	}

	num = number_argument(argument, keyword);
	if (keyword[0] == '\0') {
		do_help(ch, "'OLC EDIT'");
		return FALSE;
	}

	if ((pHelp = help_lookup(num, keyword)) == NULL) {
		char_printf(ch, "HEdit: %s: Help keyword not found.\n\r",
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
	EDIT_HELP(ch, pHelp);

	output = buf_new(0);
	buf_printf(output,
		   "Level:    [%d]\n\r"
		   "Keywords: [%s]\n\r",
		   pHelp->level, pHelp->keyword);
	mlstr_dump(output, "Text:     ", pHelp->text);
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
	char_puts("HEdit: Help deleted.\n\r", ch);
	edit_done(ch->desc);
	return FALSE;
}

