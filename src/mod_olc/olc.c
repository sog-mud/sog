/*
 * $Id: olc.c,v 1.13 1998-08-18 17:18:27 fjoe Exp $
 */

/***************************************************************************
 *  File: olc.c                                                            *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was freely distributed with the The Isles 1.1 source code,   *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 *                                                                         *
 ***************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "olc.h"
#include "comm.h"
#include "db.h"
#include "interp.h"
#include "tables.h"
#include "buffer.h"
#include "mlstring.h"
#include "util.h"
#include "recycle.h"
#include "magic.h"
#include "resource.h"
#include "string_edit.h"

/*
 * The version info.  Please use this info when reporting bugs.
 * It is displayed in the game by typing 'version' while editing.
 * Do not remove these from the code - by request of Jason Dinkel
 */
#define VERSION	"ILAB Online Creation [Beta 1.0, ROM 2.3 modified]\n\r" \
		"     Port a ROM 2.4 v1.7\n\r"
#define AUTHOR	"     By Jason(jdinkel@mines.colorado.edu)\n\r" \
                "     Modified for use with ROM 2.3\n\r"        \
                "     By Hans Birkeland (hansbi@ifi.uio.no)\n\r" \
                "     Modificado para uso en ROM 2.4b4a\n\r"	\
                "     Por Ivan Toledo (pvillanu@choapa.cic.userena.cl)\n\r" \
		"     Modified for use with Muddy\n\r" \
		"     Farmer Joe (fjoe@iclub.nsu.ru)\n\r"
#define DATE	"     (Apr. 7, 1995 - ROM mod, Apr 16, 1995)\n\r" \
		"     (Port a ROM 2.4 - Nov 2, 1996)\n\r" \
		"     Version actual : 1.71 - Mar 22, 1998\n\r"
#define CREDITS "     Original by Surreality(cxw197@psu.edu) and Locke(locke@lm.com)"

struct olced_data {
	int		id;
	char *		name;
	DO_FUN *	edit;
	OLC_CMD_DATA *	cmd_table;
	char *		do_name;
	DO_FUN *	do_fun;
};
typedef struct olced_data OLCED_DATA;	

OLCED_DATA olced_table[] = {
	{	ED_AREA,	"AEdit",	aedit,
		aedit_table,	"area",		do_aedit,	},
	{	ED_ROOM,	"REdit",	redit,
		redit_table,	"room",		do_redit,	},
	{	ED_OBJECT,	"OEdit",	oedit,
		oedit_table,	"object",	do_oedit,	},
	{	ED_MOBILE,	"MEdit",	medit,
		medit_table,	"mobile",	do_medit,	},
	{	ED_MPCODE,	"MPEdit",	mpedit,
		mpedit_table,	"mpcode",	do_mpedit,	},
	{	ED_HELP,	"HEdit",	hedit,
		hedit_table,	"help",		do_hedit,	},
	{ -1 }
};

static OLCED_DATA *	olced_lookup(int id);
static OLC_CMD_DATA *olced_cmd_lookup(CHAR_DATA *ch, OLC_FUN *fun,
				      OLCED_DATA *olced);
static void show_olc_cmds(CHAR_DATA *ch, OLC_CMD_DATA *olc_table);

/* Executed from comm.c.  Minimizes compiling when changes are made. */
bool run_olc_editor(DESCRIPTOR_DATA *d)
{
	OLCED_DATA *olced = olced_lookup(d->editor);
	if (olced == NULL)
		return FALSE;
	olced->edit(d->character, d->incomm);
	return TRUE;
}

/*
 * olc_ed_name - returns name of current OLC editor (if any).
 *		 Called by bust_a_prompt.
 */
const char *olc_ed_name(CHAR_DATA *ch)
{
	OLCED_DATA *olced;

	if (IS_NPC(ch))
		return str_empty;

	olced = olced_lookup(ch->desc->editor);
	if (olced == NULL)
		return str_empty;
	return olced->name;
}

/*
 * Generic OLC editor functions.
 * All functions assume !IS_NPC(ch).
 */

bool olced_number(CHAR_DATA *ch, const char *argument, OLC_FUN* fun, int *pInt)
{
	int val;
	char *endptr;
	char arg[MAX_STRING_LENGTH];
	OLC_CMD_DATA *cmd;
	VALIDATE_FUN *validator;

	if ((cmd = olc_cmd_lookup(ch, fun)) == NULL)
		return FALSE;

	one_argument(argument, arg);
	val = strtol(arg, &endptr, 0);
	if (*arg == '\0' || *endptr != '\0') {
		char_printf(ch, "Syntax: %s number\n\r", cmd->name);
		return FALSE;
	}

	if ((validator = cmd->arg1) && !validator(ch, &val))
		return FALSE;

	*pInt = val;
	char_nputs(MSG_OK, ch);
	return TRUE;
}

bool olced_str(CHAR_DATA *ch, const char *argument, OLC_FUN *fun, char **pStr)
{
	OLC_CMD_DATA *cmd;
	VALIDATE_FUN *validator;

	if ((cmd = olc_cmd_lookup(ch, fun)) == NULL)
		return FALSE;

	if (IS_NULLSTR(argument)) {
		char_printf(ch, "Syntax: %s string\n\r", cmd->name);
		return FALSE;
	}

	if ((validator = cmd->arg1) && !validator(ch, argument))
		return FALSE;

	free_string(*pStr);
	*pStr = str_dup(argument);
	char_nputs(MSG_OK, ch);
	return TRUE;
}

bool olced_str_text(CHAR_DATA *ch, const char *argument,
		    OLC_FUN *fun, char **pStr)
{
	OLC_CMD_DATA *cmd;

	if ((cmd = olc_cmd_lookup(ch, fun)) == NULL)
		return FALSE;

	if (argument[0] =='\0') {
		string_append(ch, pStr);
		return TRUE;
	}

	char_printf(ch, "Syntax: %s\n\r", cmd->name);
	return FALSE;
}

bool olced_mlstr(CHAR_DATA *ch, const char *argument,
		 OLC_FUN *fun, mlstring **pmlstr)
{
	OLC_CMD_DATA *cmd;

	if ((cmd = olc_cmd_lookup(ch, fun)) == NULL)
		return FALSE;

	if (!mlstr_edit(pmlstr, argument)) {
		char_printf(ch, "Syntax: %s lang string\n\r", cmd->name);
		return FALSE;
	}
	char_nputs(MSG_OK, ch);
	return TRUE;
}

bool olced_mlstrnl(CHAR_DATA *ch, const char *argument,
		   OLC_FUN *fun, mlstring **pmlstr)
{
	OLC_CMD_DATA *cmd;

	if ((cmd = olc_cmd_lookup(ch, fun)) == NULL)
		return FALSE;

	if (!mlstr_editnl(pmlstr, argument)) {
		char_printf(ch, "Syntax: %s lang string\n\r", cmd->name);
		return FALSE;
	}
	char_nputs(MSG_OK, ch);
	return TRUE;
}

bool olced_mlstr_text(CHAR_DATA *ch, const char *argument,
		      OLC_FUN *fun, mlstring **pmlstr)
{
	OLC_CMD_DATA *cmd;

	if ((cmd = olc_cmd_lookup(ch, fun)) == NULL)
		return FALSE;

	if (!mlstr_append(ch, pmlstr, argument)) {
		char_printf(ch, "Syntax: %s lang\n\r", cmd->name);
		return FALSE;
	}
	return TRUE;
}

bool olced_ed(CHAR_DATA *ch, const char* argument, ED_DATA **ped)
{
	ED_DATA *ed;
	char command[MAX_INPUT_LENGTH];
	char keyword[MAX_INPUT_LENGTH];
	char lang[MAX_INPUT_LENGTH];

	argument = one_argument(argument, command);
	argument = one_argument(argument, keyword);
	argument = one_argument(argument, lang);

	if (command[0] == '\0' || keyword[0] == '\0') {
		do_help(ch, "'OLC ED'");
		return FALSE;
	}

	if (!str_cmp(command, "add")) {
		ed		= ed_new();
		ed->keyword	= str_dup(keyword);
		ed->next	= *ped;
		*ped		= ed;

		if (!mlstr_append(ch, &ed->description, lang)) {
			ed_free(ed);
			do_help(ch, "'OLC ED'");
			return FALSE;
		}

		send_to_char("Extra description added.\n\r", ch);
		return TRUE;
	}

	if (!str_cmp(command, "edit")) {
		ed = ed_lookup(keyword, *ped);
		if (ed == NULL) {
			send_to_char("Extra description keyword not found.\n\r", ch);
			return FALSE;
		}

		if (!mlstr_append(ch, &ed->description, lang)) {
			do_help(ch, "'OLC ED'");
			return FALSE;
		}
		return TRUE;
	}

	if (!str_cmp(command, "delete")) {
		ED_DATA *prev = NULL;

		for (ed = *ped; ed; ed = ed->next) {
			if (is_name(keyword, ed->keyword))
				break;
			prev = ed;
		}

		if (ed == NULL) {
			send_to_char("Extra description keyword not found.\n\r", ch);
			return FALSE;
		}

		if (prev == NULL)
			*ped = ed->next;
		else
			prev->next = ed->next;

		ed_free(ed);

		send_to_char("Extra description deleted.\n\r", ch);
		return TRUE;
	}


	if (!str_cmp(command, "format")) {
		ed = ed_lookup(keyword, *ped);
		if (ed == NULL) {
			send_to_char("REdit:  Extra description keyword not found.\n\r", ch);
			return FALSE;
		}

		mlstr_format(&ed->description);
		send_to_char("Extra description formatted.\n\r", ch);
		return TRUE;
	}

	do_help(ch, "'OLC ED'");
	return FALSE;
}

bool olced_flag(CHAR_DATA *ch, const char *argument, OLC_FUN* fun, int *pInt)
{
	int stat;
	OLC_CMD_DATA *cmd;
	OLCED_DATA *olced;
	const FLAG *f;
	int marked;

	if ((olced = olced_lookup(ch->desc->editor)) == NULL
	||  (cmd = olced_cmd_lookup(ch, fun, olced)) == NULL)
		return FALSE;

	if (!cmd->arg1) {
		char_printf(ch, "%s: %s: Table of values undefined (report it to implementors).\n\r", olced->name, cmd->name);
		return FALSE;
	}

	if ((stat = is_stat(cmd->arg1)) < 0) {
		char_printf(ch, "%s: %s: Unknown table of values (report it to implementors).\n\r", olced->name, cmd->name);
		return FALSE;
	}

	if (!str_cmp(argument, "?")) {
		show_flag_cmds(ch, cmd->arg1);
		return FALSE;
	}

	if (stat) {
		if ((f = flag_lookup(cmd->arg1, argument)) == NULL) {
			char_printf(ch, "Syntax: %s value\n\r"
					"Type '%s ?' for a list of "
					"acceptable values.\n\r",
					cmd->name, cmd->name);
			return FALSE;
		}
		if (!f->settable) {
			char_printf(ch, "%s: %s: '%s': value is not settable.\n\r",
				    olced->name, cmd->name, f->name);
			return FALSE;
		}
		*pInt = f->bit;
		char_printf(ch, "%s: %s: '%s': Ok.\n\r",
			    olced->name, cmd->name, f->name);
		return TRUE;
	}

	marked = 0;

	/*
	 * Accept multiple flags.
	 */
	for (;;) {
		char word[MAX_INPUT_LENGTH];

		argument = one_argument(argument, word);

		if (word[0] == '\0')
			break;

		if ((f = flag_lookup(cmd->arg1, word)) == NULL) {
			char_printf(ch, "Syntax: %s flag...\n\r"
					"Type '%s ?' for a list of "
					"acceptable flags.\n\r",
					cmd->name, cmd->name);
			return FALSE;
		}
		if (!f->settable) {
			char_printf(ch, "%s: %s: '%s': flag is not settable.\n\r",
				    olced->name, cmd->name, f->name);
			continue;
		}
		SET_BIT(marked, f->bit);
	}

	if (marked) {
		TOGGLE_BIT(*pInt, marked);
		char_printf(ch, "%s: %s: '%s': flag(s) toggled.\n\r",
			    olced->name, cmd->name,
			    flag_string(cmd->arg1, marked));
		return TRUE;
	}
	return FALSE;
}

bool olced_dice(CHAR_DATA *ch, const char *argument, OLC_FUN *fun, int *dice)
{
	int num, type, bonus;
	char* p;
	OLC_CMD_DATA *cmd;

	if ((cmd = olc_cmd_lookup(ch, fun)) == NULL)
		return FALSE;

	if (argument[0] == '\0')
		goto bail_out;
	
	num = strtod(argument, &p);
	if (num < 1 || *p != 'd')
		goto bail_out;

	type = strtod(p+1, &p);
	if (type < 1 || *p != '+')
		goto bail_out;
	
	bonus = strtod(p+1, &p);
	if (bonus < 0 || *p != '\0')
		goto bail_out;

	dice[DICE_NUMBER] = num;
	dice[DICE_TYPE]   = type;
	dice[DICE_BONUS]  = bonus;

	char_printf(ch, "%s set to %dd%d+%d.\n\r", cmd->name, num, type, bonus);
	return TRUE;

bail_out:
	char_printf(ch, "Syntax: %s <number>d<type>+<bonus>\n\r", cmd->name);
	return FALSE;
}

/*****************************************************************************
 Name:		show_commands
 Purpose:	Display all olc commands.
 Called by:	olc interpreters.
 ****************************************************************************/
bool show_commands(CHAR_DATA *ch, const char *argument)
{
	OLCED_DATA *olced;

	olced = olced_lookup(ch->desc->editor);
	if (olced == NULL)
		return FALSE;

	show_olc_cmds(ch, olced->cmd_table);
	return FALSE;
}

bool show_version(CHAR_DATA *ch, const char *argument)
{
	send_to_char(VERSION, ch);
	send_to_char("\n\r", ch);
	send_to_char(AUTHOR, ch);
	send_to_char("\n\r", ch);
	send_to_char(DATE, ch);
	send_to_char("\n\r", ch);
	send_to_char(CREDITS, ch);
	send_to_char("\n\r", ch);

	return FALSE;
}    

/*****************************************************************************
 Name:		show_flag_cmds
 Purpose:	Displays settable flags and stats.
 ****************************************************************************/
void show_flag_cmds(CHAR_DATA *ch, const FLAG *flag_table)
{
	BUFFER *output;
	int  flag;
	int  col;
 
	output = buf_new(0);
	col = 0;
	for (flag = 0; flag_table[flag].name != NULL; flag++) {
		if (flag_table[flag].settable) {
			buf_printf(output, "%-19.18s", flag_table[flag].name);
			if (++col % 4 == 0)
				buf_add(output, "\n\r");
		}
	}
 
	if (col % 4 != 0)
		buf_add(output, "\n\r");

	page_to_char(buf_string(output), ch);
	buf_free(output);
}

/*****************************************************************************
 Name:		edit_done
 Purpose:	Resets builder information on completion.
 Called by:	aedit, redit, oedit, medit(olc.c)
 ****************************************************************************/
bool edit_done(CHAR_DATA *ch)
{
	ch->desc->pEdit = NULL;
	ch->desc->editor = 0;
	return FALSE;
}

/* Entry point for all editors. */
void do_olc(CHAR_DATA *ch, const char *argument)
{
	char command[MAX_INPUT_LENGTH];
	int i;

	if (IS_NPC(ch))
		return;

	argument = one_argument(argument, command);

	if (command[0] == '\0') {
        	do_help(ch, "'OLC INTRO'");
        	return;
	}
 
 	/* Search Table and Dispatch Command. */
	for (i = 0; olced_table[i].name != NULL; i++) {
		if (!str_prefix(command, olced_table[i].do_name)) {
			olced_table[i].do_fun(ch, argument);
			return;
		}
	}

	/* Invalid command, send help. */
	do_help(ch, "'OLC INTRO'");
}

/*****************************************************************************
 Name:		do_alist
 Purpose:	Normal command to list areas and display area information.
 Called by:	interpreter(interp.c)
 ****************************************************************************/
void do_alist(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_STRING_LENGTH];
	AREA_DATA *pArea;
	BUFFER *output = NULL;

	one_argument(argument, arg);

	for (pArea = area_first; pArea; pArea = pArea->next) {
		if (arg[0] != '\0') {
			char *lowered;
			bool match;

			lowered = str_dup(pArea->name);
			strlwr(lowered);
			match = strstr(lowered, arg) != NULL;
			free(lowered);

			if (!match)
				continue;
		}

		if (output == NULL) {
			output = buf_new(0);
    			buf_printf(output, "[%3s] [%-27s] (%-5s-%5s) [%-10s] %3s [%-10s]\n\r",
				   "Num", "Area Name", "lvnum", "uvnum",
				   "Filename", "Sec", "Builders");
		}

		buf_printf(output, "[%3d] %-29.29s (%-5d-%5d) %-12.12s [%d] [%-10.10s]\n\r",
			   pArea->vnum, pArea->name,
			   pArea->min_vnum, pArea->max_vnum,
			   pArea->file_name, pArea->security, pArea->builders);
    	}

	if (output != NULL) {
		char_puts(buf_string(output), ch);
		buf_free(output);
	}
	else
		char_puts("No areas with that name found.\n\r", ch);
}

OLC_CMD_DATA *olc_cmd_lookup(CHAR_DATA *ch, OLC_FUN *fun)
{
	OLCED_DATA *olced = olced_lookup(ch->desc->editor);

	if ((olced = olced_lookup(ch->desc->editor)) == NULL)
		return NULL;

	return olced_cmd_lookup(ch, fun, olced);
}

/* Local functions */

static OLCED_DATA *olced_lookup(int id)
{
	int i;
	for (i = 0; olced_table[i].id != -1; i++)
		if (olced_table[i].id == id)
			return olced_table+i;
	return NULL;
}

static OLC_CMD_DATA *olced_cmd_lookup(CHAR_DATA *ch, OLC_FUN *fun,
				      OLCED_DATA *olced)
{
	OLC_CMD_DATA *cmd;
	for (cmd = olced->cmd_table; cmd->name; cmd++)
		if (cmd->olc_fun == fun)
			return cmd;
	return NULL;
}

/*****************************************************************************
 Name:		show_olc_cmds
 Purpose:	Format up the commands from given table.
 Called by:	show_commands(olc_act.c).
 ****************************************************************************/
void show_olc_cmds(CHAR_DATA *ch, OLC_CMD_DATA *olc_table)
{
    char buf  [ MAX_STRING_LENGTH ];
    char buf1 [ MAX_STRING_LENGTH ];
    int  cmd;
    int  col;
 
    buf1[0] = '\0';
    col = 0;
    for (cmd = 0; olc_table[cmd].name != NULL; cmd++)
    {
	sprintf(buf, "%-15.15s", olc_table[cmd].name);
	strcat(buf1, buf);
	if (++col % 5 == 0)
	    strcat(buf1, "\n\r");
    }
 
    if (col % 5 != 0)
	strcat(buf1, "\n\r");

    send_to_char(buf1, ch);
    return;
}

