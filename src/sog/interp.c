/*
 * $Id: interp.c,v 1.158 1999-06-24 16:33:15 fjoe Exp $
 */

/***************************************************************************
 *     ANATOLIA 2.1 is copyright 1996-1997 Serdar BULUT, Ibrahim CANPUNAR  *	
 *     ANATOLIA has been brought to you by ANATOLIA consortium		   *
 *	 Serdar BULUT {Chronos}		bulut@rorqual.cc.metu.edu.tr       *
 *	 Ibrahim Canpunar  {Asena}	canpunar@rorqual.cc.metu.edu.tr    *	
 *	 Murat BICER  {KIO}		mbicer@rorqual.cc.metu.edu.tr	   *	
 *	 D.Baris ACAR {Powerman}	dbacar@rorqual.cc.metu.edu.tr	   *	
 *     By using this code, you have agreed to follow the terms of the      *
 *     ANATOLIA license, in the file Anatolia/anatolia.licence             *	
 ***************************************************************************/

/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
*	ROM 2.4 is copyright 1993-1995 Russ Taylor			   *
*	ROM has been brought to you by the ROM consortium		   *
*	    Russ Taylor (rtaylor@pacinfo.com)				   *
*	    Gabrielle Taylor (gtaylor@pacinfo.com)			   *
*	    Brian Moore (rom@rom.efn.org)				   *
*	By using this code, you have agreed to follow the terms of the	   *
*	ROM license, in the file Rom24/doc/rom.license			   *
***************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#include "merc.h"
#include "interp.h"
#include "olc.h"
#include "cmd.h"
#include "socials.h"

#undef IMMORTALS_LOGS

void interpret_social(social_t *soc, CHAR_DATA *ch, const char *argument);

/*
 * Command logging types.
 */
#define LOG_NORMAL	0
#define LOG_ALWAYS	1
#define LOG_NEVER	2

/*
 * Log-all switch.
 */
bool				fLogAll		= FALSE;

#ifdef IMMORTALS_LOGS
/*
 * immortals log file
 */
FILE				*imm_log;
#endif

void interpret(CHAR_DATA *ch, const char *argument)
{
	interpret_raw(ch, argument, FALSE);
}

/*
 * The main entry point for executing commands.
 * Can be recursively called from 'at', 'order', 'force'.
 */
void interpret_raw(CHAR_DATA *ch, const char *argument, bool is_order)
{
	char command[MAX_INPUT_LENGTH];
	const char *logline;
	cmd_t *cmd;
	bool found;
	social_t *soc = NULL;
	int min_pos;
	flag64_t cmd_flags;

	/*
	 * Strip leading spaces.
	 */
	while (isspace(*argument))
		argument++;

	if (argument[0] == '\0')
		return;

	logline = argument;

	/*
	 * Grab the command word.
	 * Special parsing so ' can be a command,
	 * also no spaces needed after punctuation.
	 */

#ifdef IMMORTALS_LOGS
 	if (IS_IMMORTAL(ch)) {
		if ((imm_log = dfopen(GODS_PATH, IMMLOG_FILE, "a+"))) {
			fprintf(imm_log, "%s [%s] %s\n",
				strtime(time(NULL)), ch->name, logline);
			fprintf(imm_log, buf);
			fclose(imm_log);
		}
	}
#endif

	if (!isalpha(argument[0]) && !isdigit(argument[0])) {
		command[0] = argument[0];
		command[1] = '\0';
		argument++;
		while (isspace(*argument))
			argument++;
	} else
		argument = one_argument(argument, command, sizeof(command));

	/*
	 * Look for command in command table.
	 */
	found = FALSE;
	for (cmd = cmd_table; cmd->name; cmd++) {
		if (str_prefix(command, cmd->name))
			continue;

		/*
		 * Implement freeze command.
		 */
		if (!IS_NPC(ch)
		&&  IS_SET(ch->plr_flags, PLR_FREEZE)
		&&  !IS_SET(cmd->cmd_flags, CMD_FROZEN_OK))
			continue;

		if (IS_SET(cmd->cmd_flags, CMD_DISABLED)) {
			char_puts("Sorry, this command is temporarily disabled.\n", ch);
			return;
		}

		if (cmd->level >= LEVEL_IMMORTAL) {
			if (IS_NPC(ch))
				continue;

			if (ch->level < IMPLEMENTOR
			&&  !is_name(cmd->name, ch->pcdata->granted))
				continue;
		}
		else if (cmd->level > ch->level)
			continue;

		if (is_order) {
			if (IS_SET(cmd->cmd_flags, CMD_NOORDER)
			||  cmd->level >= LEVEL_IMMORTAL)
				return;
		}
		else {
			if (IS_AFFECTED(ch, AFF_CHARM)
			&&  !IS_SET(cmd->cmd_flags, CMD_CHARMED_OK)
			&&  cmd->level < LEVEL_IMMORTAL 
			&&  !IS_IMMORTAL(ch)) {
				char_puts("First ask your beloved master!\n",
					  ch);
				return;
			}
		}

		if (IS_AFFECTED(ch, AFF_STUN) 
		&&  !(cmd->cmd_flags & CMD_KEEP_HIDE)) {
			char_puts("You are STUNNED to do that.\n", ch);
			return;
		}

		found = TRUE;
		break;
	}

	/*
	 * Log and snoop.
	 */
	if (((!IS_NPC(ch) && IS_SET(ch->plr_flags, PLR_LOG)) ||
	     fLogAll ||
	     cmd->log == LOG_ALWAYS)
	&&  cmd->log != LOG_NEVER
	&&  logline[0] != '\0') {
		log("Log %s: %s", ch->name, logline);
		wiznet("Log $N: $t", ch, logline, WIZ_SECURE, 0, ch->level);
	}

	if (ch->desc && ch->desc->snoop_by) {
		char buf[MAX_INPUT_LENGTH];

		snprintf(buf, sizeof(buf), "# %s\n\r", logline);
		write_to_snoop(ch->desc, buf, 0);
	}

	if (!found) {
		if (!IS_NPC(ch)
		&&  IS_SET(ch->plr_flags, PLR_FREEZE)) {
			char_puts("You're totally frozen!\n", ch);
			return;
		}

		/*
		 * Look for command in socials table.
		 */
		if ((soc = social_lookup(command, str_prefix)) == NULL) {
			char_puts("Huh?\n", ch);
			return;
		}

		if (!IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE)) {
			char_puts("You are anti-social!\n", ch);
			return;
		}

		min_pos = soc->min_pos;
		cmd_flags = 0;
	}
	else {
		min_pos = cmd->position;
		cmd_flags = cmd->cmd_flags;
	}

	if (!IS_NPC(ch)) {
		/* Come out of hiding for most commands */
		if (IS_AFFECTED(ch, AFF_HIDE | AFF_FADE)
		&&  !IS_SET(cmd_flags, CMD_KEEP_HIDE)) {
			REMOVE_BIT(ch->affected_by, AFF_HIDE | AFF_FADE);
			act_puts("You step out of shadows.",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
			act("$n steps out of shadows.",
			    ch, NULL, NULL, TO_ROOM);
		}

		if (IS_AFFECTED(ch, AFF_IMP_INVIS)
		&&  min_pos == POS_FIGHTING) {
			affect_bit_strip(ch, TO_AFFECTS, AFF_IMP_INVIS);
			act_puts("You fade into existence.",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
			act("$n fades into existence.",
			    ch, NULL, NULL, TO_ROOM);
		}
	}

	/*
	 * Character not in position for command?
	 */
	if (ch->position < min_pos) {
		switch(ch->position) {
			case POS_DEAD:
				char_puts("Lie still; You are DEAD.\n", ch);
				break;

			case POS_MORTAL:
			case POS_INCAP:
				char_puts("You are hurt far too bad "
					  "for that.\n", ch);
				break;

			case POS_STUNNED:
				char_puts("You are too stunned to do that.\n",
					  ch);
				break;

			case POS_SLEEPING:
				char_puts("In your dreams, or what?\n", ch);
				break;

			case POS_RESTING:
				char_puts("Nah... You feel too relaxed...\n",
					  ch);
				break;

			case POS_SITTING:
				char_puts("Better stand up first.\n", ch);
				break;

			case POS_FIGHTING:
				char_puts("No way! You are still fighting!\n",
					  ch);
				break;

		}
		return;
	}

	if (soc) {
		interpret_social(soc, ch, argument);
		return;
	}

	/*
	 * Dispatch the command.
	 */
	cmd->do_fun(ch, argument);

	tail_chain();
}

void interpret_social(social_t *soc, CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	ROOM_INDEX_DATA *victim_room;

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		act(soc->noarg_char, ch, NULL, NULL, TO_CHAR);
		act(soc->noarg_room,
		    ch, NULL, NULL, TO_ROOM | ACT_TOBUF | ACT_NOTWIT);
		return;
	}

	if ((victim = get_char_world(ch, arg)) == NULL
	||  (IS_NPC(victim) && victim->in_room != ch->in_room)) {
		act(soc->notfound_char, ch, NULL, NULL, TO_CHAR);
		return;
	}

	if (victim == ch) {
		act(soc->self_char, ch, NULL, victim, TO_CHAR);
		act(soc->self_room,
		    ch, NULL, victim, TO_ROOM | ACT_TOBUF | ACT_NOTWIT);
		return;
	}

	victim_room = victim->in_room;
	victim->in_room = ch->in_room;

	act(soc->found_char, ch, NULL, victim, TO_CHAR);
	act(soc->found_vict,
	    ch, NULL, victim, TO_VICT | ACT_TOBUF | ACT_NOTWIT);
	act(soc->found_notvict,
	    ch, NULL, victim, TO_NOTVICT | ACT_TOBUF | ACT_NOTWIT);

	victim->in_room = victim_room;

	if (!IS_NPC(ch) && IS_NPC(victim) 
	&&  !IS_AFFECTED(victim, AFF_CHARM)
	&&  IS_AWAKE(victim) && !victim->desc) {
		switch (number_bits(4)) {
			case 0:

			case 1: case 2: case 3: case 4:
			case 5: case 6: case 7: case 8:
				act(soc->found_char,
				    victim, NULL, ch, TO_CHAR);
				act(soc->found_vict,
				    victim, NULL, ch, TO_VICT | ACT_TOBUF);
				act(soc->found_notvict,
				    victim, NULL, ch,
				    TO_NOTVICT | ACT_TOBUF | ACT_NOTWIT);
				break;

			case 9: case 10: case 11: case 12:
				act("$n slaps $N.", victim, NULL, ch, 
				    TO_NOTVICT | ACT_TOBUF | ACT_NOTWIT);
				act("You slap $N.", victim, NULL, ch, TO_CHAR);
				act("$n slaps you.", victim, NULL, ch, 
				    TO_VICT | ACT_TOBUF);
				break;
		}
	}
}

/*
 * Return true if an argument is completely numeric.
 */
bool is_number(const char *argument)
{
	if (IS_NULLSTR(argument))
    		return FALSE;
 
	if (*argument == '+' || *argument == '-')
    		argument++;
 
	for (; *argument != '\0'; argument++) {
    		if (!isdigit(*argument))
        		return FALSE;
	}
 
	return TRUE;
}

static uint x_argument(const char *argument, char c, char *arg, size_t len)
{
	char *p;
	char *q;
	int number;
    
	if (IS_NULLSTR(argument)) {
		arg[0] = '\0';
		return 0;
	}

	p = strchr(argument, c);
	if (p == NULL) {
		strnzcpy(arg, len, argument);
		return 1;
	}

	number = strtoul(argument, &q, 0);
	if (q == p)
		argument = p+1;
	else 
		number = 1;
	strnzcpy(arg, len, argument);
	return number;
}

/*
 * Given a string like 14.foo, return 14 and 'foo'
 */
uint number_argument(const char *argument, char *arg, size_t len)
{
	return x_argument(argument, '.', arg, len);
}

/* 
 * Given a string like 14*foo, return 14 and 'foo'
 */
uint mult_argument(const char *argument, char *arg, size_t len)
{
	return x_argument(argument, '*', arg, len);
}

/*
 * Pick off one argument from a string and return the rest.
 * Understands quotes.
 */
const char *one_argument(const char *argument, char *arg_first, size_t len)
{
	return first_arg(argument, arg_first, len, TRUE);
}

/*****************************************************************************
 Name:		first_arg
 Purpose:	Pick off one argument from a string and return the rest.
 		Understands quotes, if fCase then arg_first will be lowercased
 Called by:	string_add(string.c)
 ****************************************************************************/
const char *first_arg(const char *argument, char *arg_first, size_t len,
		      bool fCase)
{
	char *q;
	char cEnd = '\0';

	if (IS_NULLSTR(argument)) {
		arg_first[0] = '\0';
		return argument;
	}

/* skip leading spaces */
	while (isspace(*argument))
		argument++;

/* check quotes */
	if (*argument == '\'' || *argument == '"')
        	cEnd = *argument++;

	for (q = arg_first; *argument && q - arg_first + 1 < len; argument++) {
		if ((!cEnd && isspace(*argument)) || *argument == cEnd) {
			argument++;
			break;
		}
		*q++ = fCase ? LOWER(*argument) : *argument;
	}
	*q = '\0';

	while (isspace(*argument))
		argument++;

	return argument;
}

/*
 * Contributed by Alander.
 */
void do_commands(CHAR_DATA *ch, const char *argument)
{
	cmd_t *cmd;
	int col;
 
	col = 0;
	for (cmd = cmd_table; cmd->name; cmd++) {
		if (cmd->level < LEVEL_HERO
		&&  cmd->level <= ch->level 
		&&  !IS_SET(cmd->cmd_flags, CMD_HIDDEN)) {
			char_printf(ch, "%-12s", cmd->name);
			if (++col % 6 == 0)
				char_puts("\n", ch);
		}
	}
 
	if (col % 6 != 0)
		char_puts("\n", ch);
}

void do_wizhelp(CHAR_DATA *ch, const char *argument)
{
	cmd_t *cmd;
	int col;
 
	if (IS_NPC(ch)) {
		char_puts("Huh?\n", ch);
		return;
	}

	col = 0;
	for (cmd = cmd_table; cmd->name; cmd++) {
		if (cmd->level < LEVEL_IMMORTAL)
			continue;

		if (ch->level < IMPLEMENTOR
		&&  !is_name(cmd->name, ch->pcdata->granted))
			continue;

		char_printf(ch, "%-12s", cmd->name);
		if (++col % 6 == 0)
			char_puts("\n", ch);
	}
 
	if (col % 6 != 0)
		char_puts("\n", ch);
}

/*********** alias.c **************/

/* does aliasing and other fun stuff */
void substitute_alias(DESCRIPTOR_DATA *d, const char *argument)
{
    CHAR_DATA *ch;
    char buf[MAX_STRING_LENGTH],prefix[MAX_INPUT_LENGTH],name[MAX_INPUT_LENGTH];
    const char *point;
    int alias;

    ch = d->original ? d->original : d->character;

    /* check for prefix */
    if (ch->prefix[0] != '\0' && str_prefix("prefix",argument))
    {
	if (strlen(ch->prefix) + strlen(argument) > MAX_INPUT_LENGTH)
	    char_puts("Line to long, prefix not processed.\n",ch);
	else
	{
	    snprintf(prefix, sizeof(prefix), "%s %s",ch->prefix,argument);
	    argument = prefix;
	}
    }

    if (IS_NPC(ch) || ch->pcdata->alias[0] == NULL
    ||	!str_prefix("alias",argument) || !str_prefix("una",argument) 
    ||  !str_prefix("prefix",argument)) {
	interpret(d->character, argument);
	return;
    }

    strnzcpy(buf, sizeof(buf), argument);

    for (alias = 0; alias < MAX_ALIAS; alias++)	 /* go through the aliases */
    {
	if (ch->pcdata->alias[alias] == NULL)
	    break;

	if (!str_prefix(ch->pcdata->alias[alias],argument))
	{
	    point = one_argument(argument, name, sizeof(name));
	    if (!strcmp(ch->pcdata->alias[alias],name)) {
		snprintf(buf, sizeof(buf), "%s %s",
			 ch->pcdata->alias_sub[alias], point);
		break;
	    }
	    if (strlen(buf) > MAX_INPUT_LENGTH)
	    {
		char_puts("Alias substitution too long. Truncated.\n",ch);
		buf[MAX_INPUT_LENGTH -1] = '\0';
	    }
	}
    }
    interpret(d->character, buf);
}

void do_alia(CHAR_DATA *ch, const char *argument)
{
    char_puts("I'm sorry, alias must be entered in full.\n",ch);
    return;
}

void do_alias(CHAR_DATA *ch, const char *argument)
{
    CHAR_DATA *rch;
    char arg[MAX_INPUT_LENGTH];
    int pos;

    if (ch->desc == NULL)
	rch = ch;
    else
	rch = ch->desc->original ? ch->desc->original : ch;

    if (IS_NPC(rch))
	return;

    argument = one_argument(argument, arg, sizeof(arg));
    

    if (arg[0] == '\0')
    {
	if (rch->pcdata->alias[0] == NULL)
	{
	    char_puts("You have no aliases defined.\n",ch);
	    return;
	}
	char_puts("Your current aliases are:\n",ch);

	for (pos = 0; pos < MAX_ALIAS; pos++)
	{
	    if (rch->pcdata->alias[pos] == NULL
	    ||	rch->pcdata->alias_sub[pos] == NULL)
		break;

	    char_printf(ch,"    %s:  %s\n",rch->pcdata->alias[pos],
		    rch->pcdata->alias_sub[pos]);
	}
	return;
    }

    if (!str_prefix("una",arg) || !str_cmp("alias",arg))
    {
	char_puts("Sorry, that word is reserved.\n",ch);
	return;
    }

    if (argument[0] == '\0')
    {
	for (pos = 0; pos < MAX_ALIAS; pos++)
	{
	    if (rch->pcdata->alias[pos] == NULL
	    ||	rch->pcdata->alias_sub[pos] == NULL)
		break;

	    if (!str_cmp(arg,rch->pcdata->alias[pos])) {
		char_printf(ch, "%s aliases to '%s'.\n",
			    rch->pcdata->alias[pos],
			    rch->pcdata->alias_sub[pos]);
		return;
	    }
	}

	char_puts("That alias is not defined.\n",ch);
	return;
    }

    if (!str_prefix(argument,"delete") || !str_prefix(argument,"prefix"))
    {
	char_puts("That shall not be done!\n",ch);
	return;
    }

    for (pos = 0; pos < MAX_ALIAS; pos++)
    {
	if (rch->pcdata->alias[pos] == NULL)
	    break;

	if (!str_cmp(arg,rch->pcdata->alias[pos])) /* redefine an alias */
	{
	    free_string(rch->pcdata->alias_sub[pos]);
	    rch->pcdata->alias_sub[pos] = str_dup(argument);
	    char_printf(ch,"%s is now realiased to '%s'.\n",arg,argument);
	    return;
	}
     }

     if (pos >= MAX_ALIAS)
     {
	char_puts("Sorry, you have reached the alias limit.\n",ch);
	return;
     }
  
     /* make a new alias */
     rch->pcdata->alias[pos]		= str_dup(arg);
     rch->pcdata->alias_sub[pos]	= str_dup(argument);
     char_printf(ch,"%s is now aliased to '%s'.\n",arg,argument);
}


void do_unalias(CHAR_DATA *ch, const char *argument)
{
    CHAR_DATA *rch;
    char arg[MAX_INPUT_LENGTH];
    int pos;
    bool found = FALSE;
 
    if (ch->desc == NULL)
	rch = ch;
    else
	rch = ch->desc->original ? ch->desc->original : ch;
 
    if (IS_NPC(rch))
	return;
 
    argument = one_argument(argument, arg, sizeof(arg));

    if (arg == '\0')
    {
	char_puts("Unalias what?\n",ch);
	return;
    }

    for (pos = 0; pos < MAX_ALIAS; pos++)
    {
	if (rch->pcdata->alias[pos] == NULL)
	    break;

	if (found)
	{
	    rch->pcdata->alias[pos-1]		= rch->pcdata->alias[pos];
	    rch->pcdata->alias_sub[pos-1]	= rch->pcdata->alias_sub[pos];
	    rch->pcdata->alias[pos]		= NULL;
	    rch->pcdata->alias_sub[pos]		= NULL;
	    continue;
	}

	if(!strcmp(arg,rch->pcdata->alias[pos]))
	{
	    char_puts("Alias removed.\n",ch);
	    free_string(rch->pcdata->alias[pos]);
	    free_string(rch->pcdata->alias_sub[pos]);
	    rch->pcdata->alias[pos] = NULL;
	    rch->pcdata->alias_sub[pos] = NULL;
	    found = TRUE;
	}
    }

    if (!found)
	char_puts("No alias of that name to remove.\n",ch);
}
