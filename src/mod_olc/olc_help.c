/*
 * $Id: olc_help.c,v 1.7 1998-09-04 05:27:55 fjoe Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "merc.h"
#include "olc.h"
#include "interp.h"

#define HEDIT(fun)		bool fun(CHAR_DATA *ch, const char *argument)
#define EDIT_HELP(ch, help)	(help = (HELP_DATA*) ch->desc->pEdit)

DECLARE_OLC_FUN(hedit_create		);
DECLARE_OLC_FUN(hedit_edit		);
DECLARE_OLC_FUN(hedit_show		);
DECLARE_OLC_FUN(hedit_level		);
DECLARE_OLC_FUN(hedit_keyword		);
DECLARE_OLC_FUN(hedit_text		);
DECLARE_OLC_FUN(hedit_del		);

DECLARE_VALIDATE_FUN(validate_keyword	);

OLC_CMD_DATA hedit_table[] =
{
	{ "create",	hedit_create				},
	{ "edit",	hedit_edit				},
	{ "show",	hedit_show				},
	{ "level",	hedit_level			 	},
	{ "keywords",	hedit_keyword,	validate_keyword	},
	{ "text",	hedit_text				},
	{ "del",	hedit_del				},

	{ "commands",	show_commands				},
	{ NULL }
};

void hedit(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	char command[MAX_INPUT_LENGTH];
	int cmd;

	strnzcpy(arg, argument, MAX_INPUT_LENGTH);
	argument = one_argument(argument, command);

	if (command[0] == '\0') {
		hedit_show(ch, argument);
		return;
	}

	if (!str_cmp(command, "done")) {
		edit_done(ch);
		return;
	}

	for (cmd = 0; hedit_table[cmd].name != NULL; cmd++)
		if (!str_prefix(command, hedit_table[cmd].name)) {
			if (hedit_table[cmd].olc_fun(ch, argument)) {
				HELP_DATA *pHelp;
				EDIT_HELP(ch, pHelp);
				SET_BIT(pHelp->area->flags, AREA_CHANGED);
			}
			return;
		}

	interpret(ch, arg);
}

void do_hedit(CHAR_DATA *ch, const char *argument)
{
	char command[MAX_INPUT_LENGTH];

	if (IS_NPC(ch)) {
		char_nputs(MSG_HUH, ch);
		return;
	}
		
	if (ch->pcdata->security < SECURITY_HELP) {
		char_puts("HEdit: Insufficient security for editing helps\n\r", ch);
		return;
	}

	argument = one_argument(argument, command);

	if (!str_cmp(command, "create")) {
		hedit_create(ch, argument);
		return;
	}

	if (!str_cmp(command, "edit")) {
		hedit_edit(ch, argument);
		return;
	}

	do_help(ch, "'OLC HEDIT'");
}

HEDIT(hedit_create)
{
	HELP_DATA *pHelp;
	AREA_DATA *pArea;

	if (argument[0] == '\0') {
		do_help(ch, "'OLC HEDIT'");
		return FALSE;
	}

	if ((pHelp = help_lookup(1, argument)) != NULL) {
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
	pHelp->keyword		= str_dup(argument);
	pHelp->text		= NULL;
	help_add(pArea, pHelp);

	ch->desc->pEdit		= (void *)pHelp;
	ch->desc->editor	= ED_HELP;

	send_to_char("Help created.\n\r",ch);

	return FALSE;
}

HEDIT(hedit_edit)
{
	int num;
	char keyword[MAX_STRING_LENGTH];
	HELP_DATA *pHelp;

	num = number_argument(argument, keyword);
	if ((pHelp = help_lookup(num, keyword)) != NULL) {
		ch->desc->pEdit		= (void *)pHelp;
		ch->desc->editor	= ED_HELP;
	}
	else
		do_help(ch, "'OLC HEDIT'");

	return FALSE;
}

HEDIT(hedit_show)
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

HEDIT(hedit_level)
{
	HELP_DATA *pHelp;
	EDIT_HELP(ch, pHelp);
	return olced_number(ch, argument, hedit_level, &pHelp->level);
}

HEDIT(hedit_keyword)
{
	HELP_DATA *pHelp;
	EDIT_HELP(ch, pHelp);
	return olced_str(ch, argument, hedit_keyword, &pHelp->keyword);
}
		
HEDIT(hedit_text)
{
	HELP_DATA *pHelp;
	EDIT_HELP(ch, pHelp);
	return olced_mlstr_text(ch, argument, hedit_text, &pHelp->text);
}

HEDIT(hedit_del)
{
	HELP_DATA *pHelp;
	EDIT_HELP(ch, pHelp);
	help_free(pHelp);
	SET_BIT(pHelp->area->flags, AREA_CHANGED);
	char_puts("Help deleted.\n\r", ch);
	edit_done(ch);
	return FALSE;
}

VALIDATOR(validate_keyword)
{
	HELP_DATA *pHelp;

	if ((pHelp = help_lookup(1, (char*) arg))) {
		char_printf(ch,
			    "HEdit: Help already exists in area %s (%s).\n\r",
			    pHelp->area->name, pHelp->area->file_name);
		return FALSE;
	}
	return TRUE;
}
