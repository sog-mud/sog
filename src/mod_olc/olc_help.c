/*
 * $Id: olc_help.c,v 1.1 1998-08-14 22:33:06 fjoe Exp $
 */
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

#include "merc.h"
#include "olc.h"
#include "db.h"
#include "resource.h"
#include "comm.h"
#include "recycle.h"
#include "mlstring.h"
#include "string_edit.h"
#include "buffer.h"
#include "util.h"

#define HEDIT(fun)	bool fun(CHAR_DATA *ch, const char *argument)

const struct olc_cmd_type hedit_table[] =
{
	{ "commands",	show_commands	},
	{ "?",		show_help	},
	{ "create",	hedit_create	},
	{ "show",	hedit_show	},
	{ "level",	hedit_level 	},
	{ "keyword",	hedit_keyword	},
	{ "text",	hedit_text	},
	{ NULL }
};

HELP_DATA *help_lookup(const char *keyword);

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
				SET_BIT(pHelp->area->area_flags, AREA_CHANGED);
			}
			return;
		}

	interpret(ch, arg);
}

void do_hedit(CHAR_DATA *ch, const char *argument)
{
	HELP_DATA *pHelp;
	char command[MAX_INPUT_LENGTH];

	if (IS_NPC(ch)) {
		char_nputs(MSG_HUH, ch);
		return;
	}
		
	argument = one_argument(argument, command);

	if (!str_cmp(command, "create")) {
		if (argument[0] == '\0') {
			char_puts("Syntax: hedit create keyword\n\r", ch);
			return;
		}

		hedit_create(ch, argument);
		return;
	}

	if ((pHelp = help_lookup(command)) != NULL) {
		if (ch->pcdata->security < 9) {
			char_puts("HEdit: Insufficient security for editing helps\n\r", ch);
			return;
		}

		ch->desc->pEdit		= (void *)pHelp;
		ch->desc->editor	= ED_HELP;

		return;
	}

	char_puts("Syntax: hedit keyword\n\r"
		  "        hedit create keyword\n\r", ch);
}

HEDIT(hedit_create)
{
	HELP_DATA *pHelp;
	AREA_DATA *pArea;

	if (ch->pcdata->security < 9) {
		char_puts("HEdit: Insufficient security for editing helps\n\r", ch);
		return FALSE;
	}

	if (argument[0] == '\0') {
		char_puts("Syntax: hedit create keyword\n\r", ch);
		return FALSE;
	}

	if ((pHelp = help_lookup(argument)) != NULL) {
		if (!str_cmp(argument, "create")) {
			/* hedit create create */
			ch->desc->pEdit		= (void *)pHelp;
			ch->desc->editor	= ED_HELP;
			return FALSE;
		}
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
		char_puts("You must be editing an area to create helps.\n\r",
			  ch);
		return FALSE;
	}

	pHelp			= new_help();
	pHelp->level		= 0;
	pHelp->keyword		= str_dup(argument);
	pHelp->text		= &mlstr_empty;
	help_add(pArea, pHelp);

	ch->desc->pEdit		= (void *)pHelp;
	ch->desc->editor	= ED_HELP;

	send_to_char("Help created.\n\r",ch);

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
	char_puts(buf_string(output), ch);
	buf_free(output);

	return FALSE;
}

HEDIT(hedit_level)
{
	int level;
	char *endptr;
	char arg[MAX_STRING_LENGTH];
	HELP_DATA *pHelp;
	EDIT_HELP(ch, pHelp);

	one_argument(argument, arg);
	level = strtol(arg, &endptr, 0);
	if (*arg == '\0' || *endptr != '\0') {
		char_puts("Syntax: level num\n\r", ch);
		return FALSE;
	}
	pHelp->level = level;
	return TRUE;
}

HEDIT(hedit_keyword)
{
	HELP_DATA *pHelp;
	EDIT_HELP(ch, pHelp);

	if (argument[0] == '\0') {
		char_puts("Syntax: keyword string\n\r", ch);
		return FALSE;
	}

	free_string(pHelp->keyword);
	pHelp->keyword = str_dup(argument);
	char_puts("Help keywords set.\n\r", ch);
	return TRUE;
}
		
HEDIT(hedit_text)
{
	HELP_DATA *pHelp;
	EDIT_HELP(ch, pHelp);

	if (argument[0] != '\0') {
		mlstr_append(ch, &pHelp->text, argument);
		return TRUE;
	}

	char_puts("Syntax: text lang\n\r", ch);
	return FALSE;
}

HELP_DATA *help_lookup(const char *keyword)
{
	HELP_DATA *res;

	if (IS_NULLSTR(keyword))
		return NULL;

	for (res = help_first; res != NULL; res = res->next)
		if (is_name(keyword, res->keyword))
			return res;
	return NULL;
}
