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
 * $Id: olc.c,v 1.31 1998-10-17 11:45:42 fjoe Exp $
 */

/***************************************************************************
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
#include "interp.h"
#include "olc.h"

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
	const char *	id;
	const char *	name;
	OLC_CMD_DATA *	cmd_table;
};
typedef struct olced_data OLCED_DATA;	

const char ED_AREA[]	= "area";
const char ED_ROOM[]	= "room";
const char ED_OBJ[]	= "object";
const char ED_MOB[]	= "mobile";
const char ED_MPCODE[]	= "mpcode";
const char ED_HELP[]	= "help";
const char ED_CLAN[]	= "clan";
const char ED_MSG[]	= "msgdb";
const char ED_LANG[]	= "language";
const char ED_GENDER[]	= "word";
const char ED_CASE[]	= "word";
/*const char ED_CLASS[]	= "class";*/

OLCED_DATA olced_table[] = {
	{ ED_AREA,	"AreaEd",	olc_cmds_area	},
	{ ED_ROOM,	"RoomEd",	olc_cmds_room	},
	{ ED_OBJ,	"ObjEd",	olc_cmds_obj	},
	{ ED_MOB,	"MobEd",	olc_cmds_mob	},
	{ ED_MPCODE,	"MPEd",		olc_cmds_mpcode	},
	{ ED_HELP,	"HelpEd",	olc_cmds_help	},
	{ ED_MSG,	"MsgEd",	olc_cmds_msg	},
	{ ED_CLAN,	"ClanEd",	olc_cmds_clan	},
	{ ED_LANG,	"LangEd",	olc_cmds_lang	},
/*	{ ED_GENDER,	"WordEd",	olc_cmds_word	},
	{ ED_CASE,	"WordEd",	olc_cmds_word	}, */
/*	{ ED_CLASS,	"ClassEd",	olc_cmds_class	}, */
	{ NULL }
};

static OLCED_DATA *	olced_lookup	(const char * id);
static OLC_CMD_DATA *	cmd_lookup	(OLC_CMD_DATA *cmd_table, OLC_FUN *fun);
static OLC_CMD_DATA *	cmd_name_lookup	(OLC_CMD_DATA *cmd_table,
					 const char *name);

static void do_olc(CHAR_DATA *ch, const char *argument, int fun);

/* Executed from comm.c.  Minimizes compiling when changes are made. */
bool run_olc_editor(DESCRIPTOR_DATA *d)
{
	char command[MAX_INPUT_LENGTH];
	OLC_CMD_DATA *cmd;
	const char *argument;
	OLCED_DATA *olced = olced_lookup(d->editor);

	if ((olced = olced_lookup(d->editor)) == NULL)
		return FALSE;

	argument = one_argument(d->incomm, command);

	if (command[0] == '\0') {
		olced->cmd_table[FUN_SHOW].olc_fun(d->character, argument);
		return TRUE;
	}

	if (!str_cmp(command, "done")) {
		edit_done(d);
		return TRUE;
	}

	if ((cmd = cmd_name_lookup(olced->cmd_table+FUN_FIRST, command)) == NULL
	||  cmd->olc_fun == NULL)
		return FALSE;

	if (cmd->olc_fun(d->character, argument)
	&&  olced->cmd_table[FUN_TOUCH].olc_fun)
		olced->cmd_table[FUN_TOUCH].olc_fun(d->character, NULL);

	return TRUE;
}

void do_create(CHAR_DATA *ch, const char *argument)
{
	do_olc(ch, argument, FUN_CREATE);
}

void do_edit(CHAR_DATA *ch, const char *argument)
{
	do_olc(ch, argument, FUN_EDIT);
}

void do_alist(CHAR_DATA *ch, const char *argument)
{
	do_olc(ch, argument, FUN_LIST);
}

void do_ashow(CHAR_DATA *ch, const char *argument)
{
	do_olc(ch, argument, FUN_SHOW);
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
	char_puts("Ok.\n\r", ch);
	return TRUE;
}

bool olced_str(CHAR_DATA *ch, const char *argument,
	       OLC_FUN *fun, const char **pStr)
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
	char_puts("Ok.\n\r", ch);
	return TRUE;
}

bool olced_str_text(CHAR_DATA *ch, const char *argument,
		    OLC_FUN *fun, const char **pStr)
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
	char_puts("Ok.\n\r", ch);
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
	char_puts("Ok.\n\r", ch);
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

bool olced_exd(CHAR_DATA *ch, const char* argument, ED_DATA **ped)
{
	ED_DATA *ed;
	char command[MAX_INPUT_LENGTH];
	char keyword[MAX_INPUT_LENGTH];
	char lang[MAX_INPUT_LENGTH];

	argument = one_argument(argument, command);
	argument = one_argument(argument, keyword);
	argument = one_argument(argument, lang);

	if (command[0] == '\0' || keyword[0] == '\0') {
		do_help(ch, "'OLC EXD'");
		return FALSE;
	}

	if (!str_cmp(command, "add")) {
		ed		= ed_new();
		ed->keyword	= str_dup(keyword);
		ed->next	= *ped;
		*ped		= ed;

		if (!mlstr_append(ch, &ed->description, lang)) {
			ed_free(ed);
			do_help(ch, "'OLC EXD'");
			return FALSE;
		}

		char_puts("Extra description added.\n\r", ch);
		return TRUE;
	}

	if (!str_cmp(command, "edit")) {
		ed = ed_lookup(keyword, *ped);
		if (ed == NULL) {
			char_puts("Extra description keyword not found.\n\r", ch);
			return FALSE;
		}

		if (!mlstr_append(ch, &ed->description, lang)) {
			do_help(ch, "'OLC EXD'");
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
			char_puts("Extra description keyword not found.\n\r", ch);
			return FALSE;
		}

		if (prev == NULL)
			*ped = ed->next;
		else
			prev->next = ed->next;

		ed_free(ed);

		char_puts("Extra description deleted.\n\r", ch);
		return TRUE;
	}


	if (!str_cmp(command, "format")) {
		ed = ed_lookup(keyword, *ped);
		if (ed == NULL) {
			char_puts("REdit:  Extra description keyword not found.\n\r", ch);
			return FALSE;
		}

		mlstr_format(&ed->description);
		char_puts("Extra description formatted.\n\r", ch);
		return TRUE;
	}

	do_help(ch, "'OLC EXD'");
	return FALSE;
}

bool olced_flag(CHAR_DATA *ch, const char *argument,
		OLC_FUN* fun, flag_t *pflag)
{
	int stat;
	OLC_CMD_DATA *cmd;
	OLCED_DATA *olced;
	const FLAG *f;
	flag_t marked;

	if ((olced = olced_lookup(ch->desc->editor)) == NULL
	||  (cmd = cmd_lookup(olced->cmd_table, fun)) == NULL)
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
		show_flags(ch, cmd->arg1);
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
		*pflag = f->bit;
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
		TOGGLE_BIT(*pflag, marked);
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
	
	num = strtol(argument, &p, 0);
	if (num < 1 || *p != 'd')
		goto bail_out;

	type = strtol(p+1, &p, 0);
	if (type < 1 || *p != '+')
		goto bail_out;
	
	bonus = strtol(p+1, &p, 0);
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

bool olced_clan(CHAR_DATA *ch, const char *argument, OLC_FUN *fun, int *vnum)
{
	OLC_CMD_DATA *cmd;
	int cn;

	if ((cmd = olc_cmd_lookup(ch, fun)) == NULL)
		return FALSE;

	if (IS_NULLSTR(argument)) {
		char_printf(ch, "Syntax: %s clan\n\r"
				"Use 'clan ?' for list of valid clans.\n\r"
				"Use 'clan none' to reset clan.\n\r",
			    cmd->name);
		return FALSE;
	}

	if (!str_cmp(argument, "none")) {
		*vnum = 0;
		return TRUE;
	}

	if ((cn = cn_lookup(argument)) < 0) {
		char_printf(ch, "'%s': unknown clan.\n\r", argument);
		return FALSE;
	}

	*vnum = cn;
	return TRUE;
}

VALIDATE_FUN(validate_filename)
{
	if (strpbrk(arg, "/")) {
		char_printf(ch, "%s: Invalid characters in file name.\n\r",
			    olc_ed_name(ch));
		return FALSE;
	}
	return TRUE;
}

VALIDATE_FUN(validate_room_vnum)
{
	int vnum = *(int*) arg;

	if (vnum && get_room_index(vnum) == NULL) {
		char_printf(ch, "OLC: %d: no such room.\n\r", vnum);
		return FALSE;
	}

	return TRUE;
}

/*****************************************************************************
 Name:		show_commands
 Purpose:	Display all olc commands.
 Called by:	olc interpreters.
 ****************************************************************************/
bool show_commands(CHAR_DATA *ch, const char *argument)
{
	OLCED_DATA *	olced;
	OLC_CMD_DATA *	cmd;
	BUFFER *	output;
	int		col;

	olced = olced_lookup(ch->desc->editor);
	if (olced == NULL)
		return FALSE;

	output = buf_new(0); 

	for (col = 0, cmd = olced->cmd_table+FUN_FIRST; cmd->name; cmd++) {
		buf_printf(output, "%-15.15s", cmd->name);
		if (++col % 5 == 0)
			buf_add(output, "\n\r");
	}
	if (col % 5 != 0)
		buf_add(output, "\n\r");

	page_to_char(buf_string(output), ch);
	buf_free(output);

	return FALSE;
}

bool show_version(CHAR_DATA *ch, const char *argument)
{
	char_puts(VERSION	"\n\r"
		  AUTHOR	"\n\r"
		  DATE		"\n\r"
		  CREDITS	"\n\r", ch);

	return FALSE;
}    

AREA_DATA *get_edited_area(CHAR_DATA *ch)
{
	int vnum;
	const char *id = ch->desc->editor;
	void *p = ch->desc->pEdit;

	if (id == ED_AREA)
		return p;

	if (id == ED_HELP)
		return ((HELP_DATA*) p)->area;

	if (id == ED_ROOM)
		return ch->in_room->area;

	if (id == ED_OBJ)
		vnum = ((OBJ_INDEX_DATA*) p)->vnum;
	else if (id == ED_MOB)
		vnum = ((MOB_INDEX_DATA*) p)->vnum;
	else if (id == ED_MPCODE)
		vnum = ((MPCODE*) p)->vnum;
	else
		return NULL;
		
	return area_vnum_lookup(vnum);
}

bool touch_area(AREA_DATA *pArea)
{
	if (pArea)
		SET_BIT(pArea->flags, AREA_CHANGED);
	return FALSE;
}

bool touch_vnum(int vnum)
{
	return touch_area(area_vnum_lookup(vnum));
}

void edit_done(DESCRIPTOR_DATA *d)
{
	d->pEdit = NULL;
	d->editor = NULL;
}

OLC_CMD_DATA *olc_cmd_lookup(CHAR_DATA *ch, OLC_FUN *fun)
{
	OLCED_DATA *olced = olced_lookup(ch->desc->editor);

	if ((olced = olced_lookup(ch->desc->editor)) == NULL)
		return NULL;

	return cmd_lookup(olced->cmd_table, fun);
}

/* Local functions */

/* lookup OLC editor by id */
static OLCED_DATA *olced_lookup(const char * id)
{
	OLCED_DATA *olced;

	if (IS_NULLSTR(id))
		return NULL;

	for (olced = olced_table; olced->id; olced++)
		if (!str_prefix(id, olced->id))
			return olced;
	return NULL;
}

/* lookup cmd function by pointer */
static OLC_CMD_DATA *cmd_lookup(OLC_CMD_DATA *cmd_table, OLC_FUN *fun)
{
	for (; cmd_table->name; cmd_table++)
		if (cmd_table->olc_fun == fun)
			return cmd_table;
	return NULL;
}

/* lookup cmd function by name */
static OLC_CMD_DATA *cmd_name_lookup(OLC_CMD_DATA *cmd_table, const char *name)
{
	for (; cmd_table->name; cmd_table++)
		if (!str_prefix(name, cmd_table->name))
			return cmd_table;
	return NULL;
}

char* help_topics[FUN_MAX] =
{
	"'OLC CREATE'",
	"'OLC EDIT'",
	NULL,
	"'OLC ASHOW'",
	"'OLC ALIST'"
};

static void do_olc(CHAR_DATA *ch, const char *argument, int fun)
{
	char command[MAX_INPUT_LENGTH];
	OLCED_DATA *olced;
	OLC_FUN *olcfun;

	if (IS_NPC(ch))
		return;

	argument = one_argument(argument, command);
	if ((olced = olced_lookup(command)) == NULL
	||  (olcfun = olced->cmd_table[fun].olc_fun) == NULL) {
        	do_help(ch, help_topics[fun]);
        	return;
	}

	olcfun(ch, argument);
}

