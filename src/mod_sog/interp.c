/*
 * $Id: interp.c,v 1.209 2004-02-24 10:14:13 fjoe Exp $
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#include <merc.h>
#include <cmd.h>

#include <sog.h>

#include "handler_impl.h"
#include "comm.h"

#undef IMMORTALS_LOGS

static void interpret_social(social_t *soc,
			     CHAR_DATA *ch, const char *argument);

/*
 * Command logging types.
 */
#define LOG_NORMAL	0
#define LOG_ALWAYS	1
#define LOG_NEVER	2

#ifdef IMMORTALS_LOGS
/*
 * immortals log file
 */
FILE				*imm_log;
#endif

/*
 * The main entry point for executing commands.
 * Can be recursively called from 'at', 'order', 'force'.
 */
void
interpret(CHAR_DATA *ch, const char *argument, bool is_order)
{
	char command[MAX_INPUT_LENGTH];
	const char *save_argument;
	cmd_t *cmd = NULL;
	social_t *soc = NULL;
	int min_pos;
	flag_t cmd_flg;
	int cmd_log;
	const char *cmd_name;
	int cmd_level;
	bool found = FALSE;
	CHAR_DATA *vch;

	/*
	 * Strip leading spaces.
	 */
	while (isspace(*argument))
		argument++;

	if (argument[0] == '\0')
		return;

	save_argument = argument;

	/*
	 * Grab the command word.
	 * Special parsing so ' can be a command,
	 * also no spaces needed after punctuation.
	 */

#ifdef IMMORTALS_LOGS
	if (IS_IMMORTAL(ch)) {
		if ((imm_log = dfopen(GODS_PATH, IMMLOG_FILE, "a+"))) {
			fprintf(imm_log, "%s [%s] %s\n",
				strtime(time(NULL)), ch->name, save_argument);
			fprintf(imm_log, buf);
			fclose(imm_log);
		}
	}
#endif

	if (!isalpha((u_char) argument[0]) && !isdigit(argument[0])) {
		command[0] = argument[0];
		command[1] = '\0';
		argument++;
		while (isspace(*argument))
			argument++;
	} else
		argument = one_argument(argument, command, sizeof(command));

	vch = GET_ORIGINAL(ch);

	/*
	 * Look for command in command table.
	 */
	C_FOREACH(cmd, &commands) {
		if (!!str_prefix(command, cmd->name)
		&&  !is_name(command, cmd->aliases))
			continue;

		if (!IS_NULLSTR(cmd->sn)
		&&  get_skill(vch, cmd->sn) == 0)
			continue;

		if (cmd->min_level >= LEVEL_IMMORTAL) {
			if (IS_NPC(vch))
				continue;

			if (vch->level < LEVEL_IMP
			&&  !is_name(cmd->name, PC(vch)->granted))
				continue;
		} else if (cmd->min_level > ch->level)
			continue;

		found = TRUE;
		break;
	}

	/*
	 * snoop
	 */
	if (ch->desc && ch->desc->snoop_by) {
		char buf[MAX_INPUT_LENGTH];

		snprintf(buf, sizeof(buf), "# %s\n\r", save_argument); // notrans
		write_to_snoop(ch->desc, buf, 0);
	}

	if (!found) {
		if (!IS_NPC(ch)
		&&  IS_SET(PC(ch)->plr_flags, PLR_FREEZE)) {
			act_char("You're totally frozen!", ch);
			return;
		}

		/*
		 * Look for command in socials table.
		 */
		if ((soc = social_search(command)) != NULL) {
			if (!IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE)) {
				act_char("You are anti-social!", ch);
				return;
			}
			if (ch->shapeform != NULL
			&&  IS_SET(ch->shapeform->index->flags, SHAPEFORM_NOEMOTE)) {
				act_char(
				    "You can't do such a things in this form.",
				    ch);
				return;
			}

			found = TRUE;

			min_pos = soc->min_pos;
			/* socials are harmless unless ch is charmed */
			cmd_flg = CMD_HARMLESS | CMD_STRIP_HIDE;
			cmd_log = LOG_NORMAL;
			cmd_name = soc->name;
			cmd_level = 0;
		} else {
			min_pos = POS_RESTING;
			cmd_flg = 0;
			cmd_log = LOG_NEVER;
			cmd_name = command;
			cmd_level = 0;
		}
	} else {
		min_pos = cmd->min_pos;
		cmd_flg = cmd->cmd_flags;
		cmd_log = cmd->cmd_log;
		cmd_name = cmd->name;
		cmd_level = cmd->min_level;
	}

	if (IS_SET(cmd_flg, CMD_STRICT_MATCH)
	&&  !!str_cmp(command, cmd_name)) {
		act_puts("If you want to $t, spell it out.",
		    ch, strupr(cmd_name), NULL, TO_CHAR, POS_DEAD);
		return;
	}

	if (!IS_SET(cmd_flg, CMD_HARMLESS)
	&&  !IS_NPC(ch)
	&&  ch->desc != NULL
	&&  (ch->wait > 0 || !QBUF_IN_SYNC(ch->desc))) {
		append_to_qbuf(ch->desc, save_argument);
		return;
	}

	/*
	 * Implement freeze command.
	 */
	if (!IS_NPC(ch)
	&&  IS_SET(PC(ch)->plr_flags, PLR_FREEZE)
	&&  !IS_SET(cmd_flg, CMD_FROZEN_OK)) {
		act_char("You're totally frozen!", ch);
		return;
	}

	/*
	 * "edit" cannot be disabled, otherwise it would be impossible
	 * to enable it back
	 */
	if (IS_SET(cmd_flg, CMD_DISABLED)
	&&  !!str_cmp(cmd_name, "edit")) {
		act_char("Sorry, this command is disabled.", ch);
		return;
	}

	if (is_order) {
		if (IS_SET(cmd_flg, CMD_NOORDER)
		||  cmd_level >= LEVEL_IMMORTAL)
			return;
	} else if (IS_AFFECTED(ch, AFF_CHARM)
	       &&  !IS_SET(cmd_flg, CMD_HARMLESS)
	       &&  ch->master != NULL
	       &&  cmd_level < LEVEL_IMMORTAL
	       &&  !IS_IMMORTAL(ch)) {
		act("First ask your beloved master!",
		    ch, NULL, ch->master, TO_CHAR);
		return;
	}

	if (!IS_SET(cmd_flg, CMD_HARMLESS)
	&&  IS_AFFECTED(ch, AFF_STUN)) {
		act_char("You are STUNNED to do that.", ch);
		return;
	}

	/*
	 * Log
	 */
	if (((!IS_NPC(ch) && IS_SET(PC(ch)->plr_flags, PLR_LOG)) ||
	     fLogAll ||
	     cmd_log == LOG_ALWAYS)
	&&  cmd_log != LOG_NEVER
	&&  save_argument[0] != '\0')
		printlog(LOG_INFO, "Log %s: %s", vch->name, save_argument);

	if (!IS_NPC(ch)) {
		/* Come out of hiding for most commands */
		if (HAS_INVIS(ch, ID_HIDDEN | ID_FADE)
		&&  (!IS_SET(cmd_flg, CMD_HARMLESS | CMD_KEEP_HIDE) ||
		     IS_SET(cmd_flg, CMD_STRIP_HIDE))
		&&  cmd_level < LEVEL_IMMORTAL) {
			REMOVE_INVIS(ch, ID_HIDDEN | ID_FADE);
			act_puts("You step out of shadows.",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
			act("$n steps out of shadows.",
			    ch, NULL, NULL, TO_ROOM);
		}
	}

	/*
	 * Character not in position for command?
	 */
	if (ch->position < min_pos) {
		switch(ch->position) {
		case POS_DEAD:
			act_char("Lie still; You are DEAD.", ch);
			break;

		case POS_MORTAL:
		case POS_INCAP:
			act_char("You are hurt far too bad for that.", ch);
			break;

		case POS_STUNNED:
			act_char("You are too stunned to do that.", ch);
			break;

		case POS_SLEEPING:
			act_char("In your dreams, or what?", ch);
			break;

		case POS_RESTING:
			act_char("Nah... You feel too relaxed...", ch);
			break;

		case POS_SITTING:
			act_char("Better stand up first.", ch);
			break;

		case POS_FIGHTING:
			act_char("No way! You are still fighting!", ch);
			break;
		}
		return;
	}

	if (!found) {
		/*
		 * pull 'cmd' triggers
		 */
		if (!IS_NPC(ch)
		&&  (!IS_AFFECTED(ch, AFF_CHARM) || is_order)) {
			OBJ_DATA *obj;

			if (pull_room_trigger(TRIG_ROOM_CMD,
				ch->in_room, ch,
				CAST(void *, save_argument)) >= 0
			||  IS_EXTRACTED(ch))
				return;

			foreach (vch, char_in_room(ch->in_room)) {
				if (pull_mob_trigger(TRIG_MOB_CMD, vch, ch,
					CAST(void *, save_argument)) >= 0
				||  IS_EXTRACTED(ch)) {
					foreach_done(vch);
					return;
				}
			} end_foreach(vch);

			foreach (obj, obj_of_char(ch)) {
				if (pull_obj_trigger(TRIG_OBJ_CMD, obj, ch,
					 CAST(void *, save_argument)) >= 0
				||  IS_EXTRACTED(ch)) {
					foreach_done(obj);
					return;
				}
			} end_foreach(obj);

			foreach (obj, obj_in_room(ch->in_room)) {
				if (pull_obj_trigger(TRIG_OBJ_CMD, obj, ch,
					 CAST(void *, save_argument)) >= 0
				||  IS_EXTRACTED(ch)) {
					foreach_done(obj);
					return;
				}
			} end_foreach(obj);
		}

		act_char("Huh?", ch);
		return;
	}

	if (soc) {
		interpret_social(soc, ch, argument);
		return;
	}

	/*
	 * Dispatch the command.
	 */
	if (cmd->do_fun == NULL)
		printlog(LOG_BUG, "interpret: %s: NULL do_fun", cmd->name);
	else
		cmd->do_fun(ch, argument);
}

/*--------------------------------------------------------------------
 * semi-locals
 */

/* does aliasing and other fun stuff */
void
substitute_alias(DESCRIPTOR_DATA *d, const char *argument)
{
	char buf[MAX_STRING_LENGTH];
	char prefix[MAX_INPUT_LENGTH];

	/* check for prefix */
	if (d->dvdata->prefix[0] != '\0' && str_prefix("prefix", argument)) {
		if (strlen(d->dvdata->prefix) + strlen(argument) + 2 >
							MAX_INPUT_LENGTH) {
			act_char("Line too long, prefix not processed.", d->character);
		} else {
			snprintf(prefix, sizeof(prefix),
				 "%s %s",			// notrans
				 d->dvdata->prefix, argument);
			argument = prefix;
		}
	}

	if (d->dvdata->alias[0] != NULL
	&&  str_prefix("alias", argument)
	&&  str_prefix("unalias", argument)
	&&  str_prefix("prefix", argument)) {
		int i;

		/* go through the aliases */
		for (i = 0; i < MAX_ALIAS; i++) {
			const char *point;

			if (d->dvdata->alias[i] == NULL)
				break;

			if (str_prefix(d->dvdata->alias[i], argument))
				continue;

			point = one_argument(argument, buf, sizeof(buf));
			if (strcmp(d->dvdata->alias[i], buf))
				continue;

			/*
			 * found an alias
			 */
			if (!IS_NULLSTR(point)) {
				snprintf(buf, sizeof(buf),
					 "%s %s",		// notrans
					 d->dvdata->alias_sub[i], point);
			} else {
				strlcpy(buf, d->dvdata->alias_sub[i],
				    sizeof(buf));
			}

			if (strlen(buf) > MAX_INPUT_LENGTH) {
				act_char("Alias substitution too long. Truncated.", d->character);
				buf[MAX_INPUT_LENGTH -1] = '\0';
			}
			argument = buf;
			break;
		}
	}

	if (argument[0] == '!') {
		interpret(d->character, argument+1, FALSE);
		return;
	}

	if (olc_interpret == NULL || !olc_interpret(d, argument))
		interpret(d->character, argument, FALSE);
}

void
interpret_social_char(social_t *soc, CHAR_DATA *ch, CHAR_DATA *victim)
{
	ROOM_INDEX_DATA *victim_room;

	if (victim == NULL
	||  (IS_NPC(victim) && victim->in_room != ch->in_room)) {
		act_mlputs(&soc->notfound_char, ch, NULL, NULL, TO_CHAR, POS_RESTING);
		return;
	}

	if (victim == ch) {
		act_mlputs(&soc->self_char, ch, NULL, victim, TO_CHAR, POS_RESTING);
		act_mlputs(&soc->self_room,
		    ch, NULL, victim, TO_ROOM | ACT_TOBUF | ACT_NOTWIT, POS_RESTING);
		return;
	}

	victim_room = victim->in_room;
	victim->in_room = ch->in_room;

	act_mlputs(&soc->found_char, ch, NULL, victim, TO_CHAR, POS_RESTING);
	act_mlputs(&soc->found_vict,
	    ch, NULL, victim, TO_VICT | ACT_TOBUF | ACT_NOTWIT, POS_RESTING);
	act_mlputs(&soc->found_notvict,
	    ch, NULL, victim, TO_NOTVICT | ACT_TOBUF | ACT_NOTWIT, POS_RESTING);

	victim->in_room = victim_room;

	if (!IS_NPC(ch) && IS_NPC(victim)
	&&  !IS_AFFECTED(victim, AFF_CHARM)
	&&  IS_AWAKE(victim) && !victim->desc) {
		switch (number_bits(4)) {
			case 0:

			case 1: case 2: case 3: case 4:
			case 5: case 6: case 7: case 8:
				act_mlputs(&soc->found_char,
				    victim, NULL, ch, TO_CHAR, POS_RESTING);
				act_mlputs(&soc->found_vict,
				    victim, NULL, ch, TO_VICT | ACT_TOBUF, POS_RESTING);
				act_mlputs(&soc->found_notvict,
				    victim, NULL, ch,
				    TO_NOTVICT | ACT_TOBUF | ACT_NOTWIT, POS_RESTING);
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

/*--------------------------------------------------------------------
 * local functions
 */

static void
interpret_social(social_t *soc, CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		act_mlputs(&soc->noarg_char, ch, NULL, NULL, TO_CHAR, POS_RESTING);
		act_mlputs(&soc->noarg_room,
		    ch, NULL, NULL, TO_ROOM | ACT_TOBUF | ACT_NOTWIT, POS_RESTING);
		return;
	}

	interpret_social_char(soc, ch, get_char_world(ch, arg));
}
