/*
 * $Id: olc_help.c,v 1.13 1998-09-17 05:49:22 fjoe Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "merc.h"
#include "olc.h"
#include "interp.h"

#define EDIT_HELP(ch, help)	(help = (HELP_DATA*) ch->desc->pEdit)

DECLARE_OLC_FUN(hedit_create		);
DECLARE_OLC_FUN(hedit_edit		);
DECLARE_OLC_FUN(hedit_touch		);
DECLARE_OLC_FUN(hedit_show		);

DECLARE_OLC_FUN(hedit_level		);
DECLARE_OLC_FUN(hedit_keyword		);
DECLARE_OLC_FUN(hedit_text		);
DECLARE_OLC_FUN(hedit_del		);

OLC_CMD_DATA olc_cmds_help[] =
{
	{ "create",	hedit_create	},
	{ "edit",	hedit_edit	},
	{ "touch",	hedit_touch	},
	{ "show",	hedit_show	},

	{ "level",	hedit_level	},
	{ "keywords",	hedit_keyword	},
	{ "text",	hedit_text	},
	{ "del",	hedit_del	},

	{ "commands",	show_commands	},
	{ NULL }
};

OLC_FUN(hedit_create)
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
	send_to_char("Help created.\n\r",ch);
	return FALSE;
}

OLC_FUN(hedit_edit)
{
	int num;
	char keyword[MAX_STRING_LENGTH];
	HELP_DATA *pHelp;

	if (ch->pcdata->security < SECURITY_HELP) {
		char_puts("HEdit: Insufficient security.\n\r", ch);
		return FALSE;
	}

	num = number_argument(argument, keyword);
	if ((pHelp = help_lookup(num, keyword)) == NULL) {
		char_printf(ch, "HEdit: %s: Help keyword not found.\n\r",
			    keyword);
		return FALSE;
	}

	ch->desc->pEdit		= (void *)pHelp;
	ch->desc->editor	= ED_HELP;
	return FALSE;
}

OLC_FUN(hedit_touch)
{
	HELP_DATA *pHelp;
	EDIT_HELP(ch, pHelp);
	return touch_area(pHelp->area);
}

OLC_FUN(hedit_show)
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

OLC_FUN(hedit_level)
{
	HELP_DATA *pHelp;
	EDIT_HELP(ch, pHelp);
	return olced_number(ch, argument, hedit_level, &pHelp->level);
}

OLC_FUN(hedit_keyword)
{
	HELP_DATA *pHelp;
	EDIT_HELP(ch, pHelp);
	return olced_str(ch, argument, hedit_keyword, &pHelp->keyword);
}
		
OLC_FUN(hedit_text)
{
	HELP_DATA *pHelp;
	EDIT_HELP(ch, pHelp);
	return olced_mlstr_text(ch, argument, hedit_text, &pHelp->text);
}

OLC_FUN(hedit_del)
{
	HELP_DATA *pHelp;
	EDIT_HELP(ch, pHelp);
	touch_area(pHelp->area);
	help_free(pHelp);
	char_puts("HEdit: Help deleted.\n\r", ch);
	edit_done(ch->desc);
	return FALSE;
}

