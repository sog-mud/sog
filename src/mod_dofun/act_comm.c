/*
 * $Id: act_comm.c,v 1.187.2.34 2002-10-18 09:12:59 tatyana Exp $
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
 **************************************************************************/

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
#include <sys/types.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>

#if	!defined (WIN32)
#include <unistd.h>
#endif

#include "merc.h"
#include "mob_prog.h"
#include "obj_prog.h"
#include "auction.h"
#include "lang.h"
#include "note.h"

/* command procedures needed */
DECLARE_DO_FUN(do_replay	);
DECLARE_DO_FUN(do_look		);

void do_afk(CHAR_DATA *ch, const char *argument)
{
	if (IS_SET(ch->comm, COMM_AFK))
		do_replay(ch, str_empty);
	else
		char_puts("You are now in AFK mode.\n", ch);
	TOGGLE_BIT(ch->comm, COMM_AFK);
}

/* RT code to delete yourself */

void do_delet(CHAR_DATA *ch, const char *argument)
{
	char_puts("You must type the full command to delete yourself.\n",
		     ch);
}

void do_delete(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch))
		return;
	
	if (IS_SET(PC(ch)->plr_flags, PLR_CONFIRM_DELETE)) {
		if (argument[0] != '\0') {
			char_puts("Delete status removed.\n",ch);
			REMOVE_BIT(PC(ch)->plr_flags, PLR_CONFIRM_DELETE);
			return;
		}

		wiznet("$N turns $Mself into line noise.", ch, NULL, 0, 0, 0);
		delete_player(ch, NULL);
		return;
	}

	if (argument[0] != '\0') {
		char_puts("Just type delete. No argument.\n",ch);
		return;
	}

	char_puts("Type delete again to confirm this command.\n"
		     "WARNING: this command is irreversible.\n"
		     "Typing delete with an argument will undo delete status.\n",
		     ch);
	SET_BIT(PC(ch)->plr_flags, PLR_CONFIRM_DELETE);
	wiznet("$N is contemplating deletion.", ch, NULL, 0, 0, ch->level);
}

/* RT code to display channel status */
void do_channels(CHAR_DATA *ch, const char *argument)
{
	/* lists all channels and their status */
	char_puts("   channel     status\n",ch);
	char_puts("---------------------\n",ch);
	
	char_puts("music          ", ch);
	if (!IS_SET(ch->comm, COMM_NOMUSIC))
		 char_puts("ON\n", ch);
	else
		 char_puts("OFF\n", ch);

	char_puts("shout          ", ch);
	if (!IS_SET(ch->comm, COMM_NOSHOUT))
		 char_puts("ON\n", ch);
	else
		 char_puts("OFF\n", ch);

	char_puts("ooc            ", ch);
	if (!IS_SET(ch->comm, COMM_NOOOC))
		 char_puts("ON\n", ch);
	else
		 char_puts("OFF\n", ch);

	char_puts("auction        ", ch);
	if (!IS_SET(ch->comm, COMM_NOAUCTION))
		 char_puts("ON\n", ch);
	else
		 char_puts("OFF\n", ch);

	if (IS_IMMORTAL(ch)) {
		char_puts("god channel    ", ch);
		if (!IS_SET(ch->comm,COMM_NOWIZ))
			char_puts("ON\n", ch);
		else
			char_puts("OFF\n", ch);
	}

	char_puts("tells          ",ch);
	if (!IS_SET(ch->comm, COMM_DEAF))
		char_puts("ON\n", ch);
	else
		char_puts("OFF\n", ch);

	char_puts("quiet mode     ", ch);
	if (IS_SET(ch->comm, COMM_QUIET))
		 char_puts("ON\n", ch);
	else
		 char_puts("OFF\n", ch);

	if (IS_SET(ch->comm, COMM_SNOOP_PROOF))
		char_puts("You are immune to snooping.\n", ch);
	
	if (IS_SET(ch->comm, COMM_NOTELL))
		 char_puts("You cannot use tell.\n", ch);
	
	if (IS_SET(ch->comm, COMM_NOCHANNELS))
		char_puts("You cannot use channels.\n", ch);

	if (IS_SET(ch->comm, COMM_NOEMOTE))
		 char_puts("You cannot show emotions.\n", ch);
}

void do_deaf(CHAR_DATA *ch, const char *argument)
{
	if (IS_SET(ch->comm,COMM_DEAF)) {
		char_puts("You can now hear tells again.\n",ch);
		REMOVE_BIT(ch->comm, COMM_DEAF);
	}
	else {
		char_puts("From now on, you won't hear tells.\n",ch);
		SET_BIT(ch->comm, COMM_DEAF);
	}
}

/* RT quiet blocks out all communication */
void do_quiet(CHAR_DATA *ch, const char *argument)
{
	if (IS_SET(ch->comm,COMM_QUIET)) {
		 char_puts("Quiet mode removed.\n",ch);
		 REMOVE_BIT(ch->comm, COMM_QUIET);
	}
	else {
		char_puts("From now on, you will only hear says and emotes.\n", ch);
		SET_BIT(ch->comm, COMM_QUIET);
	}
}

void do_replay(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch)) {
		char_puts("Huh?\n", ch);
		return;
	}

	page_to_char(buf_string(PC(ch)->buffer), ch);
	buf_clear(PC(ch)->buffer);
}

void do_say(CHAR_DATA *ch, const char *argument)
{
	OBJ_DATA *char_obj;
	OBJ_DATA *char_obj_next;
  
        if (IS_SET(ch->in_room->room_flags, ROOM_SILENT)
        &&  !IS_IMMORTAL(ch)) {
                char_puts("You are in silent room, you can't say anything.\n",
			  ch);
                return;
        }

	if (argument[0] == '\0') {
		char_puts("Say what?\n", ch);
		return;
	}

	argument = garble(ch, argument);
	act_say(ch, "$t", argument);

	if (!IS_NPC(ch)) {
 		CHAR_DATA *mob, *mob_next;
		for (mob = ch->in_room->people; mob != NULL; mob = mob_next) {
 			mob_next = mob->next_in_room;
 			if (IS_NPC(mob) && HAS_TRIGGER(mob, TRIG_SPEECH)
 			&&  mob->position == mob->pMobIndex->default_pos)
 			mp_act_trigger(argument, mob, ch, NULL, NULL,
				TRIG_SPEECH);
 		}
	}

	for (char_obj = ch->carrying; char_obj != NULL;
	     char_obj = char_obj_next) {
		char_obj_next = char_obj->next_content;
		oprog_call(OPROG_SPEECH, char_obj, ch, argument);
	}

	for (char_obj = ch->in_room->contents; char_obj != NULL;
	     char_obj = char_obj_next) {
		char_obj_next = char_obj->next_content;
		oprog_call(OPROG_SPEECH, char_obj, ch, argument);
	}
}

void do_tell(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];

        if (IS_SET(ch->in_room->room_flags, ROOM_SILENT)
        &&  !IS_IMMORTAL(ch)) {
                char_puts("You are in silent room, you can't tell.\n", ch);
                return;
        }

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0' || argument[0] == '\0') {
		char_puts("Tell whom what?\n", ch);
		return;
	}

	do_tell_raw(ch, get_char_world(ch, arg), argument);
}

void do_reply(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch)) {
		char_puts("Huh?\n", ch);
		return;
	}

        if (IS_SET(ch->in_room->room_flags, ROOM_SILENT)
        &&  !IS_IMMORTAL(ch)
        &&  !IS_IMMORTAL(PC(ch)->reply)) {
                char_puts("You are in silent room, you can't tell.\n", ch);
                return;
        }
	do_tell_raw(ch, PC(ch)->reply, argument);
}

void do_mtell(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *vch, *vch_next;

	char arg[MAX_INPUT_LENGTH];

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0' || argument[0] == '\0') {
		char_puts("Tell whom what?\n", ch);
		return;
	}

	do_tell_raw(ch, get_char_world(ch, arg), argument);

	if (IS_NPC(ch))
		return;

	if (IS_SET(ch->comm, COMM_NOCHANNELS)) {
		 char_puts("The gods refuse to listen to you right now.", ch);
		 return;
	}

	for (vch = char_list; vch != NULL && !IS_NPC(vch); vch = vch_next) {
		CHAR_DATA *victim = GET_ORIGINAL(vch);
		vch_next = vch->next;

		if (!IS_IMMORTAL(victim)
		||  IS_SET(victim->comm, COMM_NOWIZ))
			continue;

		act_puts3("{W$n{x tells $I '{G$t{x'",
			 ch, argument, vch, get_char_world(ch, arg),
			 TO_VICT | ACT_TOBUF, POS_DEAD);
	}
}

void do_gtell(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *gch;
	int i;
	int flags;

        if (IS_SET(ch->in_room->room_flags, ROOM_SILENT)
        &&  !IS_IMMORTAL(ch)) {
                char_puts("You are in silent room, you can't tell your group"
                         " anything.\n", ch);
                return;
        }

	if (argument[0] == '\0') {
		char_puts("Tell your group what?\n", ch);
		return;
	}

	if (IS_SET(ch->comm, COMM_NOTELL)) {
		char_puts("Your message didn't get through!\n", ch);
		return;
	}

	argument = garble(ch, argument);
	flags = TO_VICT | ACT_TOBUF | (ACT_SPEECH(ch) & ~ACT_NODEAF);
	for (i = 0, gch = char_list; gch; gch = gch->next) {
		if (IS_NPC(gch))
			break;

		i++;
		if (is_same_group(gch, ch) && !is_affected(gch, gsn_deafen))
			act_puts("$n tells the group '{G$t{x'",
				 ch, argument, gch, flags, POS_DEAD);
	}

	if (i > 1 && !is_affected(ch, gsn_deafen))
		act_puts("You tell your group '{G$t{x'",
			 ch, argument, NULL,
			 TO_CHAR | ACT_SPEECH(ch), POS_DEAD);
	else
		char_puts("Quit talking to yourself. You are all alone.\n", ch);
}

void do_emote(CHAR_DATA *ch, const char *argument)
{
	int flags;

	if (!IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE)) {
		char_puts("You can't show your emotions.\n", ch);
		return;
	}
	
	if (argument[0] == '\0') {
		char_puts("Emote what?\n", ch);
		return;
	}
	
	argument = garble(ch, argument);

	flags = ACT_NOTRIG | 
		(!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM) ? ACT_NOTRANS : 0);
	act("$n $T", ch, NULL, argument, TO_CHAR | flags);

	flags |= ACT_TOBUF | ACT_NOTWIT;
	act("$n $T", ch, NULL, argument, TO_ROOM | flags);
}

void do_pmote(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *vch;
	const char *letter,*name;
	char last[MAX_INPUT_LENGTH], temp[MAX_STRING_LENGTH];
	int matches = 0;
	int flags;

	if (!IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE)) {
		char_puts("You can't show your emotions.\n", ch);
		return;
	}
	
	if (argument[0] == '\0') {
		char_puts("Emote what?\n", ch);
		return;
	}
	
	argument = garble(ch, argument);

	flags = TO_CHAR | ACT_NOTRIG | ACT_NOFIXSH |
		(!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM) ? ACT_NOTRANS : 0);
	act("$n $t", ch, argument, NULL, flags);

	flags |= ACT_TOBUF | ACT_NOTWIT;
	for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room) {
		if (vch->desc == NULL || vch == ch)
			continue;

		if ((letter = strstr(argument, vch->name)) == NULL) {
			act("$N $t", vch, argument, ch, flags);
			continue;
		}

		strnzcpy(temp, sizeof(temp), argument);
		temp[strlen(argument) - strlen(letter)] = '\0';
		last[0] = '\0';
		name = vch->name;
	
		for (; *letter != '\0'; letter++) { 
			if (*letter == '\'' && matches == strlen(vch->name)) {
				strnzcat(temp, sizeof(temp), "r");
				continue;
			}

			if (*letter == 's' && matches == strlen(vch->name)) {
				matches = 0;
				continue;
			}
		
			if (matches == strlen(vch->name))
				matches = 0;

			if (*letter == *name) {
				matches++;
				name++;
				if (matches == strlen(vch->name)) {
					strnzcat(temp, sizeof(temp), "you");
					last[0] = '\0';
					name = vch->name;
					continue;
				}
				strnzncat(last, sizeof(last), letter, 1);
				continue;
			}

			matches = 0;
			strnzcat(temp, sizeof(temp), last);
			strnzncat(temp, sizeof(temp), letter, 1);
			last[0] = '\0';
			name = vch->name;
		}

		act("$N $t", vch, temp, ch, flags);
	}
}

void do_immtalk(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *vch, *vch_next;
	CHAR_DATA *orig;
	int flags;

	orig = GET_ORIGINAL(ch);
	if (argument[0] == '\0') {
		TOGGLE_BIT(orig->comm, COMM_NOWIZ);
		if (IS_SET(orig->comm, COMM_NOWIZ))
			char_puts("Immortal channel is now OFF\n", ch);
		else
			char_puts("Immortal channel is now ON\n", ch);
		return;
	}

	if (IS_SET(orig->comm, COMM_NOCHANNELS)) {
		char_puts("The gods have revoked your channel privileges.\n", ch);
		return;
	}
	
	if (IS_SET(orig->comm, COMM_NOWIZ))
		do_immtalk(ch, str_empty);

	flags = ACT_SPEECH(orig) & ~(ACT_STRANS | ACT_NODEAF);
	act_puts("[IMM] $n: {C$t{x",
		 orig, argument, NULL, TO_CHAR | flags, POS_DEAD);

	for (vch = char_list; vch != NULL && !IS_NPC(vch); vch = vch_next) {
		CHAR_DATA *victim = GET_ORIGINAL(vch);
		vch_next = vch->next;

		if (!IS_IMMORTAL(victim)
		||  IS_SET(victim->comm, COMM_NOWIZ))
			continue;

		act_puts("[IMM] $n: {C$t{x", orig, argument, vch,
			 TO_VICT | ACT_TOBUF | flags, POS_DEAD);
	}
}

void do_yell(CHAR_DATA *ch, const char *argument)
{
        if (IS_SET(ch->in_room->room_flags, ROOM_SILENT)
        &&  !IS_IMMORTAL(ch)) {
                char_puts("You are in silent room, you can't yell.\n", ch);
                return;
        }

	if (IS_SET(ch->comm, COMM_NOCHANNELS)) {
		 char_puts("The gods have revoked your channel privileges.\n", ch);
		 return;
	}
	
	if (argument[0] == '\0') {
		char_puts("Yell what?\n", ch);
		return;
	}

	argument = garble(ch, argument);
	act_puts("You yell '{M$t{x'",
		 ch, argument, NULL, TO_CHAR | ACT_SPEECH(ch), POS_DEAD);
	act_yell(ch, "$t", argument, NULL);
}

void do_shout(CHAR_DATA *ch, const char *argument)
{
	DESCRIPTOR_DATA *d;

	if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
		return;

        if (IS_SET(ch->in_room->room_flags, ROOM_SILENT)
        &&  !IS_IMMORTAL(ch)) {
                char_puts("You are in silent room, you can't shout.\n", ch);
                return;
        }

	if (argument[0] == '\0') {
		TOGGLE_BIT(ch->comm, COMM_NOSHOUT);
		if (IS_SET(ch->comm, COMM_NOSHOUT))
			char_puts("You will no longer hear shouts.\n",ch);
		else 
			char_puts("You will now hear shouts.\n", ch);
		return;
	}

	if (IS_SET(ch->comm, COMM_NOCHANNELS)) {
		 char_puts("The gods have revoked your channel privileges.\n", ch);
		 return;
	}
	
	if (IS_SET(ch->comm, COMM_NOSHOUT))
		do_shout(ch, str_empty);
	WAIT_STATE(ch, PULSE_VIOLENCE);

	argument = garble(ch, argument);
	act_puts("You shout '{Y$t{x'",
		 ch, argument, NULL, TO_CHAR | ACT_SPEECH(ch), POS_DEAD);

	for (d = descriptor_list; d; d = d->next) {
		if (d->connected == CON_PLAYING
		&&  d->character != ch
		&&  !IS_SET(d->character->comm, COMM_NOSHOUT)
		&&  (d->character->in_room == NULL ||
		     !IS_SET(d->character->in_room->room_flags, ROOM_SILENT) ||
		     IS_IMMORTAL(d->character))) {
			act_puts("$n shouts '{Y$t{x'",
				 ch, argument, d->character,
	    			 TO_VICT | ACT_NOTWIT | ACT_SPEECH(ch),
				 POS_DEAD);
		}
	}
}

void do_music(CHAR_DATA *ch, const char *argument)
{
	DESCRIPTOR_DATA *d;

	if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
		return;

	if (argument[0] == '\0') {
		TOGGLE_BIT(ch->comm, COMM_NOMUSIC);
		if (IS_SET(ch->comm, COMM_NOMUSIC))
			char_puts("You will no longer hear music.\n",ch);
		else 
			char_puts("You will now hear music.\n", ch);
		return;
	}

	if (IS_SET(ch->in_room->room_flags, ROOM_SILENT)
        &&  !IS_IMMORTAL(ch)) {
                char_puts("You are in silent room, you can't music.\n", ch);
                return;
        }

	if (IS_SET(ch->comm, COMM_NOCHANNELS)) {
		 char_puts("The gods have revoked your channel privileges.\n", ch);
		 return;
	}
	
	if (IS_SET(ch->comm, COMM_NOMUSIC))
		do_music(ch, str_empty);
	WAIT_STATE(ch, PULSE_VIOLENCE);

	argument = garble(ch, argument);
	act_puts("You music '{W$t{x'",
		 ch, argument, NULL, TO_CHAR | ACT_SPEECH(ch), POS_DEAD);

	for (d = descriptor_list; d; d = d->next) {
		if (d->connected == CON_PLAYING
		&&  d->character != ch
		&&  !IS_SET(d->character->comm, COMM_NOMUSIC)
		&&  (d->character->in_room == NULL ||
		     !IS_SET(d->character->in_room->room_flags, ROOM_SILENT) ||
		     IS_IMMORTAL(d->character))) {
			act_puts("$n musics '{W$t{x'",
		        	 ch, argument, d->character,
	    			 TO_VICT | ACT_NOTWIT | ACT_SPEECH(ch),
				 POS_DEAD);
		}
	}
}

void do_gossip(CHAR_DATA *ch, const char *argument)
{
	DESCRIPTOR_DATA *d;

	if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
		return;

	if (IS_SET(ch->comm, COMM_NOCHANNELS)) {
		 char_puts("The gods have revoked your channel privileges.\n", ch);
		 return;
	}
	
	if (argument[0] == '\0') {
		char_puts("Gossip what?\n", ch);
		return;
	}

	WAIT_STATE(ch, PULSE_VIOLENCE);

	if (!IS_IMMORTAL(ch)) {
		if (ch->mana < ch->max_mana) {
			char_puts("You don't have enough mana.\n", ch);
			return;
		}
		ch->mana = 0;
	}

	argument = garble(ch, argument);
	act_puts("You gossip '{R$t{x'",
		 ch, argument, NULL, TO_CHAR | ACT_SPEECH(ch), POS_DEAD);

	for (d = descriptor_list; d; d = d->next) {
		if (d->connected == CON_PLAYING
		&&  d->character != ch) {
			act_puts("$n gossips '{R$t{x'",
				 ch, argument, d->character,
				 TO_VICT | ACT_SPEECH(ch), POS_DEAD);
		}
	}
}

void do_clan(CHAR_DATA *ch, const char *argument)
{
	clan_t *clan;

	if (!ch->clan) {
		char_puts("You are not in a clan.\n", ch);
		return;
	}

	clan = clan_lookup(ch->clan);
	if (clan == NULL) {
		char_puts("Your clan is closed.\n", ch);
		return;
	}

	if (argument[0] == '\0') {
		TOGGLE_BIT(ch->comm, COMM_NOCLAN);
		if (IS_SET(ch->comm, COMM_NOCLAN))
			char_puts("You will no longer hear clan talks.\n",ch);
		else 
			char_puts("You will now hear clan talks.\n", ch);
		return;
	}

        if (IS_SET(ch->in_room->room_flags, ROOM_SILENT)
        &&  !IS_IMMORTAL(ch)) {
                char_puts("You are in silent room, you can't tell to your "
                         "clan anything.\n", ch);
                return;
        }

	if (IS_SET(ch->comm, COMM_NOCLAN))
		do_clan(ch, str_empty);

	argument = garble(ch, argument);
	act_puts("[CLAN] $lu{$n}: {C$t{x",
		 ch, argument, NULL, TO_CHAR | ACT_SPEECH(ch), POS_DEAD);
	act_clan(ch, "$t", argument);
}

void do_pray(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *vch, *vch_next;

	if (IS_NPC(ch))
		return;

	if (IS_SET(ch->comm, COMM_NOCHANNELS)) {
		 char_puts("The gods refuse to listen to you right now.", ch);
		 return;
	}

	if (argument[0] == '\0')
		argument = "any god";

	act_puts("You PRAY for '$t'",
		 ch, argument, NULL, TO_CHAR, POS_DEAD);

	for (vch = char_list; vch != NULL && !IS_NPC(vch); vch = vch_next) {
		CHAR_DATA *victim = GET_ORIGINAL(vch);
		vch_next = vch->next;

		if (!IS_IMMORTAL(victim)
		||  IS_SET(victim->comm, COMM_NOWIZ))
			continue;

		act_puts("{W$n{x is PRAYING for '{G$t{x'",
			 ch, argument, vch,
			 TO_VICT | ACT_TOBUF, POS_DEAD);
	 }
}

void do_pose(CHAR_DATA *ch, const char *argument)
{
	class_t *cl;
	pose_t *pose;
	int maxnum;

	if (IS_NPC(ch)
	||  (cl = class_lookup(ch->class)) == NULL
	||  cl->poses.nused == 0)
		return;

	maxnum = UMIN(ch->level, cl->poses.nused-1);
	pose = VARR_GET(&cl->poses, number_range(0, maxnum));
	act(pose->self, ch, NULL, NULL, TO_CHAR);
	act(pose->others, ch, NULL, NULL, TO_ROOM | ACT_TOBUF);
}

void do_bug(CHAR_DATA *ch, const char *argument)
{
	append_file(ch, BUG_FILE, argument);
	char_puts("Bug logged.\n", ch);
	return;
}

void do_typo(CHAR_DATA *ch, const char *argument)
{
	append_file(ch, TYPO_FILE, argument);
	char_puts("Typo logged.\n", ch);
	return;
}

void do_rent(CHAR_DATA *ch, const char *argument)
{
	char_puts("There is no rent here.  Just save and quit.\n", ch);
	return;
}

void do_qui(CHAR_DATA *ch, const char *argument)
{
	char_puts("If you want to QUIT, you have to spell it out.\n", ch);
}

void do_quit(CHAR_DATA *ch, const char *argument)
{
	PC_DATA *pc;

	if (IS_NPC(ch)) {
		char_puts("Huh?\n", ch);
		return;
	}

	pc = PC(ch);
	if (pc->pnote) {
		act_puts("You have an unfinished $t in progress.",
			 ch, flag_string(note_types, pc->pnote->type), NULL,
			 TO_CHAR, POS_DEAD);
		return;
	}

	quit_char(ch, XC_F_NOCOUNT);
}

void do_save(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch))
		return;

	if (ch->level < 2) {
		char_puts("You must be at least level 2 for saving.\n",ch);
		return;
	}

	/*
	 * char_save intentionally omitted
	 */
	act_puts("Saving.", ch, NULL, NULL, TO_CHAR, POS_DEAD);

	WAIT_STATE(ch, PULSE_VIOLENCE);
}

void do_follow(CHAR_DATA *ch, const char *argument)
{
/* RT changed to allow unlimited following and follow the NOFOLLOW rules */
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		char_puts("Follow whom?\n", ch);
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	if (IS_AFFECTED(ch, AFF_CHARM) && ch->master != NULL) {
		act("But you'd rather follow $N!", ch, NULL,ch->master,TO_CHAR);
		return;
	}

	if (victim == ch) {
		if (ch->master == NULL) {
			char_puts("You already follow yourself.\n", ch);
			return;
		}
		stop_follower(ch);
		return;
	}

	if (!IS_NPC(victim)
	&&  IS_SET(PC(victim)->plr_flags, PLR_NOFOLLOW)
	&&  !IS_IMMORTAL(ch)) {
		act("$N doesn't seem to want any followers.",
		    ch, NULL, victim, TO_CHAR);
		return;
	}

	REMOVE_BIT(PC(ch)->plr_flags, PLR_NOFOLLOW);
	add_follower(ch, victim);
}

void do_order(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH],arg2[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	CHAR_DATA *och;
	CHAR_DATA *och_next;
	bool found;
	bool fAll;

	argument = one_argument(argument, arg, sizeof(arg));
	one_argument(argument, arg2, sizeof(arg2));

	if (arg[0] == '\0' || argument[0] == '\0') {
		char_puts("Order whom to do what?\n", ch);
		return;
	}

	if (IS_AFFECTED(ch, AFF_CHARM)) {
		char_puts("You feel like taking, not giving, orders.\n", ch);
		return;
	}

	if (!str_cmp(arg, "all")) {
		fAll   = TRUE;
		victim = NULL;
	} else {
		fAll   = FALSE;
		if ((victim = get_char_room(ch, arg)) == NULL) {
			char_puts("They aren't here.\n", ch);
			return;
		}

		if (victim == ch) {
			char_puts("Aye aye, right away!\n", ch);
			return;
		}

		if (!IS_AFFECTED(victim, AFF_CHARM) || victim->master != ch 
		||(IS_IMMORTAL(victim) && victim->level >= ch->level)) {
			char_puts("Do it yourself!\n", ch);
			return;
		}
	}

	found = FALSE;
	for (och = ch->in_room->people; och != NULL; och = och_next) {
		och_next = och->next_in_room;

		if (IS_AFFECTED(och, AFF_CHARM)
		&&   och->master == ch
		&& (fAll || och == victim)) {
			found = TRUE;
			act("$n orders you to '$t', you do.",
			    ch, argument, och, TO_VICT);
			interpret_raw(och, argument, TRUE);
		}
	}

	if (found) {
		WAIT_STATE(ch,PULSE_VIOLENCE);
		char_puts("Ok.\n", ch);
	}
	else
		char_puts("You have no followers here.\n", ch);
}

static bool
is_in_opposite_clans(CHAR_DATA *ch1, CHAR_DATA *ch2)
{
	return
	   (HAS_SKILL(ch1, gsn_enforcer_badge) && HAS_SKILL(ch2, gsn_disperse))
	|| (HAS_SKILL(ch1, gsn_disperse) && HAS_SKILL(ch2, gsn_enforcer_badge))
	|| (HAS_SKILL(ch1, gsn_evil_spirit) && HAS_SKILL(ch2, gsn_riding))
	|| (HAS_SKILL(ch1, gsn_riding) && HAS_SKILL(ch2, gsn_evil_spirit))
	|| (HAS_SKILL(ch1, gsn_mastering_spell) && HAS_SKILL(ch2, gsn_spellbane))
	|| (HAS_SKILL(ch1, gsn_spellbane) && HAS_SKILL(ch2, gsn_mastering_spell));
}

void do_group(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	CHAR_DATA *gch;

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {

		act_puts("$N's group:", ch, NULL, leader_lookup(ch),
			 TO_CHAR, POS_DEAD);

		for (gch = char_list; gch; gch = gch->next) {
			if (is_same_group(gch, ch)) {
				if (IS_NPC(gch)) {
					char_printf(ch,
					    "[%2d %s] %-16s %d/%d hp "
					    "%d/%d mana %d/%d mv\n",
					    gch->level,
					    class_who_name(gch),
					    PERS(gch, ch),
					    gch->hit,   gch->max_hit,
					    gch->mana,  gch->max_mana,
					    gch->move,  gch->max_move);
				} else {
					char_printf(ch,
					    "[%2d %s] %-16s %d/%d hp "
					    "%d/%d mana %d/%d mv %5d tnl\n",
					    gch->level,
					    class_who_name(gch),
					    PERS(gch, ch),
					    gch->hit,   gch->max_hit,
					    gch->mana,  gch->max_mana,
					    gch->move,  gch->max_move,
					    exp_to_level(gch));
				}
			}
		}
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	if (victim == ch) {
		char_puts("Huh? Grouping with yourself?!\n", ch);
		return;
	}

	if (ch->master != NULL || ch->leader != NULL) {
		char_puts("But you are following someone else!\n", ch);
		return;
	}

	if (victim->master != ch && ch != victim) {
		act("$N isn't following you.", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (IS_AFFECTED(victim, AFF_CHARM)) {
		char_puts("You can't remove charmed mobs from your group.\n",ch);
		return;
	}

	if (IS_AFFECTED(ch,AFF_CHARM)) {
		act("You like your master too much to leave $m!",ch,NULL,victim,TO_VICT);
		return;
	}

	if (is_same_group(victim, ch) && ch != victim) {
		CHAR_DATA *guard;

		if (!IS_NPC(ch) && !IS_NPC(victim)) {
			if (PC(ch)->guarding == victim
			||  PC(victim)->guarded_by == ch) {
				act("You stop guarding $N.",
				    ch, NULL, victim, TO_CHAR);
				act("$n stops guarding you.",
				    ch, NULL, victim, TO_VICT);
				act("$n stops guarding $N.",
				   ch, NULL, victim, TO_NOTVICT);
				PC(victim)->guarded_by = NULL;
				PC(ch)->guarding       = NULL;
			}
		}

		victim->leader = NULL;
		act_puts("$n removes $N from $s group.",   ch, NULL, victim,
			 TO_NOTVICT,POS_SLEEPING);
		act_puts("$n removes you from $s group.",  ch, NULL, victim,
			 TO_VICT,POS_SLEEPING);
		act_puts("You remove $N from your group.", ch, NULL, victim,
			 TO_CHAR,POS_SLEEPING);

		if (!IS_NPC(victim)
		&&  (guard = PC(victim)->guarded_by) != NULL
		&&  !is_same_group(victim, guard)) {
			act("You stop guarding $N.",
			    guard, NULL, victim, TO_CHAR);
			act("$n stops guarding you.",
			    guard, NULL, victim, TO_VICT);
			act("$n stops guarding $N.",
			    guard, NULL, victim, TO_NOTVICT);
			PC(guard)->guarding		= NULL;
			PC(victim)->guarded_by	= NULL;
		}
		return;
	}

	if (ch->level - victim->level < -8 || ch->level - victim->level > 8) {
		act_puts("$N cannot join $n's group.", ch, NULL, victim,
			 TO_NOTVICT,POS_SLEEPING);
		act_puts("You cannot join $n's group.", ch, NULL, victim,
			 TO_VICT,POS_SLEEPING);
		act_puts("$N cannot join your group.", ch, NULL, victim,
			 TO_CHAR ,POS_SLEEPING);
		return;
	}

	for (gch = char_list; gch; gch = gch->next) {
		if (is_same_group(gch, ch)) {
			if (IS_GOOD(gch) && IS_EVIL(victim)) {
				act_puts("You are too evil for $n's group.",
				    ch, NULL, victim, TO_VICT, POS_SLEEPING);
				act_puts("$N is too evil for your group!",
				    ch, NULL, victim, TO_CHAR, POS_SLEEPING);
				return;
			}
			if (IS_GOOD(victim) && IS_EVIL(ch)) {
				act_puts("You are too pure to join $n's group!",
				    ch, NULL, victim, TO_VICT, POS_SLEEPING);
				act_puts("$N is too pure for your group!",
				    ch, NULL, victim, TO_CHAR,POS_SLEEPING);
				return;
			}
			if (is_in_opposite_clans(ch, gch)) {
				act_puts("You hate $n's clan, how can you join $n's group?",
				    gch, NULL, victim, TO_VICT, POS_SLEEPING);
				if (gch == ch) {
					act_puts("You hate $N's clan, how can you want $N to join your group?",
					    ch, NULL, victim, TO_CHAR, POS_SLEEPING);
				} else {
					act_puts("There are $N clan enemies in your group, how can $E join you?", ch, NULL, victim, TO_CHAR, POS_SLEEPING);
				}
			return;
			}
		}
	}

	victim->leader = ch;
	act_puts("$N joins $n's group.", ch, NULL, victim, TO_NOTVICT,
		 POS_SLEEPING);
	act_puts("You join $n's group.", ch, NULL, victim, TO_VICT,
		 POS_SLEEPING);
	act_puts("$N joins your group.", ch, NULL, victim, TO_CHAR,
		 POS_SLEEPING);
}

/*
 * 'Split' originally by Gnort, God of Chaos.
 */
void do_split(CHAR_DATA *ch, const char *argument)
{
	char arg1[MAX_INPUT_LENGTH],arg2[MAX_INPUT_LENGTH];
	CHAR_DATA *gch;
	int members;
	int amount_gold = 0, amount_silver = 0;
	int share_gold, share_silver;
	int extra_gold, extra_silver;

	argument = one_argument(argument, arg1, sizeof(arg1));
		   one_argument(argument, arg2, sizeof(arg2));

	if (!is_number(arg1)) {
		act_puts("Split how much?", ch, NULL, NULL, TO_CHAR, POS_DEAD);
		return;
	}
	
	amount_silver = atoi(arg1);

	if (is_number(arg2))
		amount_gold = atoi(arg2);

	if (amount_gold < 0 || amount_silver < 0) {
		act_puts("Your group wouldn't like that.",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		return;
	}

	if (amount_gold == 0 && amount_silver == 0) {
		act_puts("You hand out zero coins, but no one notices.",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		return;
	}

	if (ch->gold < amount_gold || ch->silver < amount_silver) {
		act_puts("You don't have that much to split.",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		return;
	}
	
	members = 0;
	for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room) {
		if (is_same_group(gch, ch) && !IS_AFFECTED(gch, AFF_CHARM))
			members++;
	}

	if (members < 2) {
		act_puts("Just keep it all.",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		return;
	}
		
	share_silver = amount_silver / members;
	extra_silver = amount_silver % members;

	share_gold   = amount_gold / members;
	extra_gold   = amount_gold % members;

	if (share_gold == 0 && share_silver == 0) {
		act_puts("Don't even bother, cheapskate.",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		return;
	}

	ch->silver	-= amount_silver;
	ch->silver	+= share_silver + extra_silver;
	ch->gold 	-= amount_gold;
	ch->gold 	+= share_gold + extra_gold;

	if (share_silver > 0) {
		act_puts3("You split $j $qj{silver coins}. "
			  "Your share is $J silver.",
			  ch, (const void*) amount_silver, NULL,
			  (const void*) share_silver + extra_silver,
			  TO_CHAR, POS_DEAD);
	}

	if (share_gold > 0) {
		act_puts3("You split $j $qj{gold coins}. "
			  "Your share is $J gold.",
			  ch, (const void*) amount_gold, NULL,
			  (const void*) share_gold + extra_gold,
			  TO_CHAR, POS_DEAD);
	}

	for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room) {
		if (gch == ch
		||  !is_same_group(gch, ch)
		||  IS_AFFECTED(gch, AFF_CHARM))
			continue;

		if (share_gold == 0) {
			act_puts3("$n splits $j $qj{silver coins}. "
				  "Your share is $J silver.",
				  ch, (const void*) amount_silver, gch,
				  (const void*) share_silver,
				  TO_VICT, POS_DEAD);
		} else if (share_silver == 0) {
			act_puts3("$n splits $j $qj{gold coins}. "
				  "Your share is $J gold.",
				  ch, (const void*) amount_gold, gch,
				  (const void*) share_gold,
				  TO_VICT, POS_DEAD);
		} else {
			act_puts3("$n splits $j silver and $J $qJ{gold coins}",
				  ch, (const void*) amount_silver, gch,
				  (const void*) amount_gold,
				  TO_VICT | ACT_NOLF, POS_DEAD);
			act_puts3(", giving you $j silver and $J gold.",
				  ch, (const void*) share_silver, gch,
				  (const void*) share_gold,
				  TO_VICT, POS_DEAD);
		}

		gch->gold += share_gold;
		gch->silver += share_silver;
	}
}

void do_speak(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	int language;
	race_t *r;

	if (IS_NPC(ch)
	||  (r = race_lookup(PC(ch)->race)) == NULL
	||  !r->race_pcdata)
		return;

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		char_printf(ch, "You now speak %s.\n",
			flag_string(slang_table, ch->slang));
		char_puts("You can speak:\n", ch);
		char_printf(ch, "       common, %s\n",
			    flag_string(slang_table, r->race_pcdata->slang));
		return;
	}

	if ((language = flag_value(slang_table, arg)) < 0) {
		char_puts("You never heard of that language.\n", ch);
		return;
	}

	if (language != SLANG_COMMON
	&&  (language != r->race_pcdata->slang && !IS_IMMORTAL(ch))) {
		act("You do not know how to speak $t.",
		    ch, flag_string(slang_table, language), NULL,
		    TO_CHAR);
		return;
	}

	ch->slang = language;
	act("Now you speak $t.",
	    ch, flag_string(slang_table, ch->slang), NULL, TO_CHAR);
}

void do_twit(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_STRING_LENGTH];

	if (IS_NPC(ch)) {
		char_puts("Huh?\n", ch);
		return;
	}

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		char_printf(ch, "Current twitlist is [%s]\n",
			    PC(ch)->twitlist);
		return;
	}

	name_toggle(&PC(ch)->twitlist, arg, ch, "Twitlist");
}

void do_lang(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_STRING_LENGTH];
	int lang;
	lang_t *l;
	DESCRIPTOR_DATA *d;

	if (langs.nused == 0) {
		char_puts("No languages defined.\n", ch);
		return;
	}

	if ((d = ch->desc) == NULL)
		return;

	argument = one_argument(argument, arg, sizeof(arg));

	if (*arg == '\0') {
		l = varr_get(&langs, d->dvdata->lang);
		if (l == NULL) {
			log("do_lang: %s: lang == %d\n",
				   ch->name, d->dvdata->lang);
			l = VARR_GET(&langs, d->dvdata->lang = 0);
		}
		char_printf(ch, "Interface language is '%s'.\n", l->name);
		return;
	}

	lang = lang_lookup(arg);
	if (lang < 0) {
		char_puts("Usage: lang [ ", ch);
		for (lang = 0; lang < langs.nused; lang++) {
			l = VARR_GET(&langs, lang);
			if (IS_SET(l->lang_flags, LANG_HIDDEN))
				continue;
			char_printf(ch, "%s%s",
				    lang == 0 ? str_empty : " | ", l->name);
		}
		char_puts(" ]\n", ch);
		return;
	}

	d->dvdata->lang = lang;
	do_lang(ch, str_empty);
	do_look(ch, str_empty);
}

void do_judge(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	if (!HAS_SKILL(ch, gsn_enforcer_badge)) {
		char_puts("Huh?\n", ch);
		return;
	}

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		char_puts("Judge whom?\n", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg)) == NULL) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	if (IS_NPC(victim)) {
		char_puts("Not a mobile, of course.\n", ch);
		return;
	}

	if (IS_IMMORTAL(victim) && !IS_IMMORTAL(ch)) {
		char_puts("You do not have the power to judge Immortals.\n",
			  ch);
		return;
	}

	char_printf(ch, "%s is %s-%s.\n",
		    PERS(victim, ch),
		    flag_string(ethos_table, victim->ethos),
		    flag_string(align_names, NALIGN(victim)));
}

void do_trust(CHAR_DATA *ch, const char *argument)
{	
	char arg[MAX_INPUT_LENGTH];
	PC_DATA *pc;
	
	if (IS_NPC(ch)) {
		char_puts("Huh?\n", ch);
		return;
	}

	pc = PC(ch);
	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		if (!pc->trust) {
			char_puts("You do not allow anyone to cast questionable spells on you.\n", ch);
			return;
		}

		if (IS_SET(pc->trust, TRUST_ALL)) {
			char_puts("You allow everyone to cast questionable spells on you.\n", ch);
			return;
		}
		if (IS_SET(pc->trust, TRUST_CLAN)) 
			char_puts("You trust your clan with questionable spells.\n", ch);
		if (IS_SET(pc->trust, TRUST_GROUP))
			char_puts("You trust your group with questionable spells.\n", ch);
		return;
	}

	if (!str_cmp(arg, "clan")) {
		if (ch->clan == 0) {
			char_puts("You are not in clan.\n", ch);
			return;
		};

		TOGGLE_BIT(pc->trust, TRUST_CLAN);
		if (IS_SET(pc->trust, TRUST_CLAN)) {
			REMOVE_BIT(pc->trust, TRUST_ALL);
			char_puts("You now trust your clan with questionable spells.\n", ch);
		}
		else 
			char_puts("You no longer trust your clan with questionable spells.\n", ch);
		return;
	}

	if (!str_cmp(arg, "group")) {
		TOGGLE_BIT(pc->trust, TRUST_GROUP);
		if (IS_SET(pc->trust, TRUST_GROUP)) {
			REMOVE_BIT(pc->trust, TRUST_ALL);
			char_puts("You allow your group to cast questionable spells on you.\n", ch);
		}
		else
			char_puts("You no longer trust your group with questionable spells.\n", ch);
		return;
	}

	if (!str_cmp(arg, "all")) {
		pc->trust = TRUST_ALL;
		char_puts("You allow everyone to cast questionable spells on you.\n", ch);
		return;
	}

	if (!str_cmp(arg, "none")) {
		pc->trust = 0;
		char_puts("You do not allow anyone to cast questionable spells on you.\n", ch);
		return;
	}

	char_puts("Syntax: trust {{ group | clan | all | none }\n", ch);
}

void do_wanted(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int sn_wanted;

	if ((sn_wanted = sn_lookup("wanted")) < 0
	||  get_skill(ch, sn_wanted) == 0) {
		char_puts("Huh?\n", ch);
		return;
	}

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		char_puts("Set wanted whom?\n", ch);
		return;
	}

	if ((victim = get_char_area(ch, arg)) == NULL) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	if (IS_NPC(victim)) {
		char_puts("Not a mobile, of course.\n", ch);
		return;
	}

	if (IS_IMMORTAL(victim) && ch->level < victim->level) {
		act("You do not have the power to arrest $N.",
		    ch, NULL, victim, TO_CHAR);
		return;
	}

	if (victim == ch) {
		char_puts( "You cannot do that to yourself.\n", ch);
		return;
	}

	if (IS_WANTED(victim)) {
		/*
		 * Commoner can remove wanted only if he/she set it before
		 */
		if (!IS_NPC(ch)
		&&  !IS_IMMORTAL(ch)
		&&  PC(ch)->clan_status == CLAN_COMMONER
		&&  str_cmp(PC(victim)->wanted_by, ch->name)) {
			act("You do not have enough privileges to remove "
			    "WANTED status from $N.",
			    ch, NULL, victim, TO_CHAR);
			return;
		}

		SET_WANTED(victim, NULL);
		act("You are no longer wanted.", victim, NULL, ch, TO_CHAR);
		act("$n is no longer wanted.", victim, NULL, ch, TO_VICT);
		act("$n is no longer wanted.", victim, NULL, ch, TO_NOTVICT);
		if (ch->in_room != victim->in_room)
			act("$N is no longer wanted.", ch, NULL, victim, TO_NOTVICT);
	} else {
		SET_WANTED(victim, ch->name);
		act("$N declares you WANTED!!!", victim, NULL, ch, TO_CHAR);
		act("You declare $n WANTED!!!", victim, NULL, ch, TO_VICT);
		act("$N declares $n WANTED!!!", victim, NULL, ch, TO_NOTVICT);
		if (ch->in_room != victim->in_room)
			act("$n declares $N WANTED!!!", ch, NULL, victim, TO_NOTVICT);
	}
}

/*-----------------------------------------------------------------------------
 * clan stuff
 */
void do_mark(CHAR_DATA *ch, const char *argument)
{
	OBJ_DATA *mark;
	clan_t *clan = NULL;

	if ((ch->clan == 0) || ((clan = clan_lookup(ch->clan)) == NULL)) {
		char_puts("You are not in clan.\n", ch);
		return;
	}

	if (!clan->mark_vnum) {
		char_puts ("Your clan do not have any mark.\n", ch);
		return;
	}

	if ((mark = get_eq_char(ch, WEAR_CLANMARK)) != NULL) {
		obj_from_char(mark);
		extract_obj(mark, 0);
	}

	mark = create_obj(get_obj_index(clan->mark_vnum), 0);
	obj_to_char(mark, ch);
	equip_char(ch, mark, WEAR_CLANMARK);
}

void do_petitio(CHAR_DATA *ch, const char *argument)
{
	char_puts("You must enter full command to petition.\n",ch);
}

void do_petition(CHAR_DATA *ch, const char *argument)
{
	bool accept;
	int cln = 0;
	clan_t *clan = NULL;
	char arg1[MAX_STRING_LENGTH];
	OBJ_DATA *mark;
	PC_DATA *pc;

	if (IS_NPC(ch))
		return;	

	argument = one_argument(argument, arg1, sizeof(arg1));
	pc = PC(ch);

	if (IS_NULLSTR(arg1)) {
		if (IS_IMMORTAL(ch)
		||  (ch->clan && (pc->clan_status == CLAN_LEADER ||
				  pc->clan_status == CLAN_SECOND))) {
			char_printf(ch,
				    "Usage: petition %s<accept | reject> "
				    "<char name>\n",
				    IS_IMMORTAL(ch) ? "<clan name> " : str_empty);
		}
		if (IS_IMMORTAL(ch) || !ch->clan)
			char_puts("Usage: petition <clan name>\n", ch);
		return;
	}

	if (IS_IMMORTAL(ch)) {
		cln = cln_lookup(arg1);
		if (cln <= 0) {
			char_printf(ch, "%s: unknown clan\n", arg1);
			do_petition(ch, str_empty);
			return;
		}
		argument = one_argument(argument, arg1, sizeof(arg1));
		if (IS_NULLSTR(arg1)) {
			do_petition(ch, str_empty);
			return;
		}
		clan = CLAN(cln);
	}

	if ((accept = !str_prefix(arg1, "accept"))
	||  !str_prefix(arg1, "reject")) {
		CHAR_DATA *victim;
		PC_DATA *vpc;
		char arg2[MAX_STRING_LENGTH];
		bool loaded = FALSE;
		bool changed;

		if (!IS_IMMORTAL(ch)) {
			if (ch->clan == 0
			||  (clan = clan_lookup(ch->clan)) == NULL) {
				do_petition(ch, str_empty);
				return;
			}
			cln = ch->clan;
		}

		argument = one_argument(argument, arg2, sizeof(arg2));
		if (IS_NULLSTR(arg2)) {
			do_petition(ch, str_empty);
			return;
		}

		if (pc->clan_status != CLAN_LEADER
		&&  pc->clan_status != CLAN_SECOND
		&&  !IS_IMMORTAL(ch)) {
			char_puts("You don't have enough power to "
				  "accept/reject petitions.\n", ch);
			return;
		}

		if ((victim = get_char_world(ch, arg2)) == NULL
		||  IS_NPC(victim)) {
			victim = char_load(arg2, LOAD_F_NOCREATE);
			if (victim == NULL) {
				char_puts("Can't find them.\n", ch);
				return;
			}
			loaded = TRUE;
		}

		vpc = PC(victim);

		if (accept) {
			if (vpc->petition != cln) {
				char_puts("They didn't petition.\n", ch);
				goto cleanup;
			}

			victim->clan = cln;
			vpc->clan_status = CLAN_COMMONER;
			vpc->petition = CLAN_NONE;
			update_skills(victim);

			name_add(&clan->member_list, victim->name, NULL, NULL);
			clan_save(clan);

			char_puts("Greet new member!\n", ch);
			if (!loaded)
				act("Your petition to $t has been accepted.",
				    victim, clan->name, NULL, TO_CHAR);
			if ((mark = get_eq_char(victim, WEAR_CLANMARK)) != NULL) {
				obj_from_char(mark);
				extract_obj(mark, 0);
			}
			if (clan->mark_vnum) {
				mark = create_obj(get_obj_index(clan->mark_vnum), 0);
				obj_to_char(mark, victim);
				equip_char(victim, mark, WEAR_CLANMARK);
			};

			goto cleanup;
		}

/* handle 'petition reject' */
		if (victim->clan == cln) {
			if (vpc->clan_status == CLAN_LEADER
			&&  !IS_IMMORTAL(ch)) {
				char_puts("You don't have enough power "
					  "to do that.\n", ch);
				return;
			}

			clan_update_lists(clan, victim, TRUE);
			clan_save(clan);

			victim->clan = CLAN_NONE;
			REMOVE_BIT(vpc->trust, TRUST_CLAN);
			update_skills(victim);
			set_title(victim, str_empty);

			act("They are not a member of $t anymore.",
			    ch, clan->name, NULL, TO_CHAR);
			if (!loaded) {
				act("You are not a member of $t anymore.",
				    victim, clan->name, NULL, TO_CHAR);
			}

			if ((mark = get_eq_char(victim, WEAR_CLANMARK))) {
				obj_from_char(mark);
				extract_obj(mark, 0);
				mark = create_obj(get_obj_index(OBJ_VNUM_RENEGADE_MARK), 0);
				obj_to_char(mark, victim);
				equip_char(victim, mark, WEAR_CLANMARK);
			}

			goto cleanup;
		}

		if (vpc->petition == cln) {
			vpc->petition = CLAN_NONE;
			char_puts("Petition was rejected.\n", ch);
			if (!loaded)
				act("Your petition to $t was rejected.",
				    victim, clan->name, NULL, TO_CHAR);
			goto cleanup;
		}

		changed = FALSE;
		if (is_name(victim->name, clan->member_list)) {
			name_delete(&clan->member_list, victim->name,
				    NULL, NULL);
			changed = TRUE;
		}

		if (is_name(victim->name, clan->second_list)) {
			name_delete(&clan->second_list, victim->name,
				    NULL, NULL);
			changed = TRUE;
		}

		if (is_name(victim->name, clan->leader_list)) {
			name_delete(&clan->leader_list, victim->name,
				    NULL, NULL);
			changed = TRUE;
		}

		if (changed)
			clan_save(clan);

		char_puts("They didn't petition.\n", ch);

	cleanup:
		if (loaded) {
			char_save(victim, SAVE_F_PSCAN);
			char_nuke(victim);
		}
		else
			dofun("save", victim, str_empty);
				
		return;
	}

	if (IS_IMMORTAL(ch)
	||  (ch->clan && (pc->clan_status == CLAN_LEADER ||
			  pc->clan_status == CLAN_SECOND))) {
		DESCRIPTOR_DATA *d;
		bool found = FALSE;

		if (IS_IMMORTAL(ch)) {
			if ((cln = cln_lookup(arg1)) <= 0) {
				char_puts("No such clan.\n", ch);
				return;
			}
		}
		else
			cln = ch->clan;
			
		for (d = descriptor_list; d; d = d->next) {
			CHAR_DATA *vch = d->original ? d->original :
						       d->character;

			if (!vch
			||  vch->clan
			||  PC(vch)->petition != cln)
				continue;

			if (!found) {
				found = TRUE;
				char_puts("List of players petitioned to "
					  "your clan:\n", ch);
			}
			char_printf(ch, "%s\n", vch->name);
		}

		if (!found) 
			char_puts("Noone has petitioned to your clan.\n", ch);
		return;
	}

	if (ch->level < LEVEL_PK) {
		act("You are not ready to join clans.",
		    ch, NULL, NULL, TO_CHAR);
		return;
	}

	if ((cln = cln_lookup(arg1)) <= 0) {
		char_puts("No such clan.\n", ch);
		return;
	}

	if (ch->clan) {
		act_puts("You cannot leave your clan this way.",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		return;
	}

	if ((mark = get_eq_char(ch, WEAR_CLANMARK)) != NULL
	&&  mark->pObjIndex->vnum == OBJ_VNUM_RENEGADE_MARK) {
		act_puts("You are renegade and can't join clans anymore.",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		return;
	}

	pc->petition = cln;
	char_puts("Petition sent.\n", ch);
}

void do_promote(CHAR_DATA *ch, const char *argument)
{
	char arg1[MAX_STRING_LENGTH];
	char arg2[MAX_STRING_LENGTH];
	CHAR_DATA *victim;
	clan_t *clan;
	PC_DATA *vpc;
	bool loaded = FALSE, changed = FALSE;

	if (IS_NPC(ch)
	||  (!IS_IMMORTAL(ch) && PC(ch)->clan_status != CLAN_LEADER)) {
		char_puts("Huh?\n", ch);
		return;
	}

	argument = one_argument(argument, arg1, sizeof(arg1));
	argument = one_argument(argument, arg2, sizeof(arg2));

	if (!*arg1 || !*arg2) {
		char_puts("Usage: promote <char name> <commoner | secondary>\n",
			  ch);
		if (IS_IMMORTAL(ch)) {
			char_puts("    or: promote <char name> <leader>\n",
				  ch);
		}
		return;
	}

	if ((victim = get_char_world(ch, arg1)) == NULL) {
		if ((victim = char_load(arg1, LOAD_F_NOCREATE)) == NULL) {
			act("No such player.", ch, NULL, NULL, TO_CHAR);
			return;
		}
		loaded = TRUE;
	}

	if (victim->clan == 0 || (clan = clan_lookup(victim->clan)) == NULL
	||  (victim->clan != ch->clan && !IS_IMMORTAL(ch))) {
		char_puts("They are not an a clan.\n", ch);
		if (loaded)
			char_nuke(victim);
		return;
	}

	if (IS_NPC(victim)) {
		act("Not on mobiles.", ch, NULL, NULL, TO_CHAR);
		return;
	}

	vpc = PC(victim);

	if (!IS_IMMORTAL(ch) && vpc->clan_status == CLAN_LEADER) {
		char_puts("You don't have enough power to promote them.\n", ch);
		if (loaded)
			char_nuke(victim);
		return;
	}

	if (!str_prefix(arg2, "leader") && IS_IMMORTAL(ch)) {
		if (vpc->clan_status == CLAN_LEADER) {
			char_puts("They are already leader in a clan.\n", ch);
			if (loaded)
				char_nuke(victim);
			return;
		}

		clan_update_lists(clan, victim, FALSE);
		name_add(&clan->leader_list, victim->name, NULL, NULL);
		clan_save(clan);

		vpc->clan_status = CLAN_LEADER;
		char_puts("Ok.\n", ch);
		if (ch != victim)
			char_puts("They are now leader in their clan.\n", ch);
		char_puts("You are now leader in your clan.\n", victim);
		changed = TRUE;
		goto cleanup;
	}

	if (!str_prefix(arg2, "secondary")) {
		if (vpc->clan_status == CLAN_SECOND) {
			char_puts("They are already second in a clan.\n", ch);
			if (loaded)
				char_nuke(victim);
			return;
		}

		clan_update_lists(clan, victim, FALSE);
		name_add(&clan->second_list, victim->name, NULL, NULL);
		clan_save(clan);

		vpc->clan_status = CLAN_SECOND;
		if (ch != victim)
			char_puts("They are now second in the clan.\n", ch);
		char_puts("You are now second in the clan.\n", victim);
		changed = TRUE;
		goto cleanup;
	}

	if (!str_prefix(arg2, "commoner")) {
		if (vpc->clan_status == CLAN_COMMONER) {
			char_puts("They are already commoner in a clan.\n",
				  ch);
			if (loaded)
				char_nuke(victim);
			return;
		}

		clan_update_lists(clan, victim, FALSE);
		clan_save(clan);

		vpc->clan_status = CLAN_COMMONER;
		if (ch != victim)
			char_puts("They are now commoner in the clan.\n", ch);
		char_puts("You are now commoner in the clan.\n", victim);
		changed = TRUE;
		goto cleanup;
	}

	do_promote(ch, str_empty);
	return;

cleanup:
	if (changed)
		char_save(victim, loaded ? SAVE_F_PSCAN : 0);
	if (loaded)
		char_nuke(victim);
}

/*-----------------------------------------------------------------------------
 * alias/unalias stuff
 */

void do_alia(CHAR_DATA *ch, const char *argument)
{
	char_puts("I'm sorry, alias must be entered in full.\n", ch);
}

void do_alias(CHAR_DATA *ch, const char *argument)
{
	DESCRIPTOR_DATA *d;
	char arg[MAX_INPUT_LENGTH];
	int pos;

	if ((d = ch->desc) == NULL)
		return;

	argument = one_argument(argument, arg, sizeof(arg));
    
	if (arg[0] == '\0') {
		if (d->dvdata->alias[0] == NULL) {
			char_puts("You have no aliases defined.\n",ch);
			return;
		}

		char_puts("Your current aliases are:\n",ch);
		for (pos = 0; pos < MAX_ALIAS; pos++) {
			if (d->dvdata->alias[pos] == NULL
			||  d->dvdata->alias_sub[pos] == NULL)
				break;

			char_printf(ch,"    %s:  %s\n",
				    d->dvdata->alias[pos],
				    d->dvdata->alias_sub[pos]);
		}
		return;
	}

	if (!str_cmp(arg, "unalias") || !str_cmp(arg, "alias")) {
		char_puts("Sorry, that word is reserved.\n",ch);
		return;
	}

	if (argument[0] == '\0') {
		for (pos = 0; pos < MAX_ALIAS; pos++) {
			if (d->dvdata->alias[pos] == NULL
			||  d->dvdata->alias_sub[pos] == NULL)
				break;

			if (!str_cmp(arg, d->dvdata->alias[pos])) {
				char_printf(ch, "%s aliases to '%s'.\n",
					    d->dvdata->alias[pos],
					    d->dvdata->alias_sub[pos]);
				return;
			}
		}

		char_puts("That alias is not defined.\n",ch);
		return;
    	}

	if (!str_prefix(argument, "delete")
	||  !str_prefix(argument, "prefix")) {
		char_puts("That shall not be done!\n",ch);
		return;
	}

	for (pos = 0; pos < MAX_ALIAS; pos++) {
		if (d->dvdata->alias[pos] == NULL)
			break;

		if (!str_cmp(arg, d->dvdata->alias[pos])) {
			/*
			 * redefine an alias
			 */
			free_string(d->dvdata->alias_sub[pos]);
			d->dvdata->alias_sub[pos] = str_dup(argument);

			char_printf(ch, "%s is now realiased to '%s'.\n",
				    arg, argument);
			return;
		}
	}

	if (pos >= MAX_ALIAS) {
		char_puts("Sorry, you have reached the alias limit.\n", ch);
		return;
	}
  
	/*
	 * make a new alias
	 */
	d->dvdata->alias[pos]		= str_dup(arg);
	d->dvdata->alias_sub[pos]	= str_dup(argument);

	char_printf(ch, "%s is now aliased to '%s'.\n", arg, argument);
}

void do_unalia(CHAR_DATA *ch, const char *argument)
{
	char_puts("I'm sorry, unalias must be entered in full.\n", ch);
}

void do_unalias(CHAR_DATA *ch, const char *argument)
{
	DESCRIPTOR_DATA *d;
	char arg[MAX_INPUT_LENGTH];
	int pos;
	bool found = FALSE;
 
	if ((d = ch->desc) == NULL)
		return;
 
	argument = one_argument(argument, arg, sizeof(arg));
	if (arg == '\0') {
		char_puts("Unalias what?\n",ch);
		return;
	}

	for (pos = 0; pos < MAX_ALIAS; pos++) {
		if (d->dvdata->alias[pos] == NULL)
			break;

		if (found) {
			d->dvdata->alias[pos-1] = d->dvdata->alias[pos];
	    		d->dvdata->alias_sub[pos-1] = d->dvdata->alias_sub[pos];
			d->dvdata->alias[pos] = NULL;
			d->dvdata->alias_sub[pos] = NULL;
			continue;
		}

		if (!strcmp(arg, d->dvdata->alias[pos])) {
			char_puts("Alias removed.\n", ch);
			free_string(d->dvdata->alias[pos]);
			free_string(d->dvdata->alias_sub[pos]);
			d->dvdata->alias[pos] = NULL;
			d->dvdata->alias_sub[pos] = NULL;
			found = TRUE;
		}
	}

	if (!found)
		char_puts("No alias of that name to remove.\n", ch);
}

/*-----------------------------------------------------------------------------
 * toggle bit stuff
 */
typedef struct toggle_t toggle_t;

struct toggle_t {
	const char *name;	/* flag name				*/
	const char *desc;	/* toggle description			*/
	flag_t *f;		/* flag table				*/
	flag64_t bit;		/* flag bit				*/
	const char *msg_on;	/* msg to print when flag toggled on	*/
	const char *msg_off;	/* ---//--- off				*/
};

static toggle_t *toggle_lookup(const char *name);
static void toggle_print(CHAR_DATA *ch, toggle_t *t);
static flag64_t* toggle_bits(CHAR_DATA *ch, toggle_t *t);

/*
 * alphabetize these table by name if you are adding new entries
 */
toggle_t toggle_table[] =
{
	{ "affects",		"show affects in score",
	  comm_flags,	COMM_SHOWAFF,
	  "Affects will now be shown in score.",
	  "Affects will no longer be shown in score."
	},

	{ "brief",		"brief descriptions",
	  comm_flags,	COMM_BRIEF,
	  "Short descriptions activated.",
	  "Full descriptions activated."
	},

	{ "color",		"ANSI colors",
	  comm_flags,	COMM_COLOR,
	  "{BC{Ro{Yl{Co{Gr{x is now {RON{x, Way Cool!",
	  "Color is now OFF, *sigh*"
	},

	{ "compact",		"compact mode",
	  comm_flags,	COMM_COMPACT,
	  "$t set.",
	  "$t removed."
	},

	{ "combine",		"combined items in inventory list",
	  comm_flags,	COMM_COMBINE,
	  "Combined inventory selected.",
	  "Long inventory selected."
	},

	{ "long flags",		"long flags mode",
	  comm_flags,	COMM_LONG,
	  "$t set.",
	  "$t removed."
	},

	{ "nobust",		"do not bust prompt if hp/mana/move changed",
	  comm_flags,	COMM_NOBUST,
	  "$t set.",
	  "$t removed."
	},

	{ "noeng",		"do not display english obj/mob names",
	  comm_flags,	COMM_NOENG,
	  "You will not see english obj/mob names anymore.",
	  "You will now see english obj/mob names."
	},

	{ "noflee",		"do not flee from combat in lost-link",
	  comm_flags,	COMM_NOFLEE,
	  "You will not flee automagically from combat in lost-link anymore.",
	  "You will flee automagically from combat in lost-link."
	},

	{ "notelnet",		"no telnet parser",
	  comm_flags,	COMM_NOTELNET,
	  "Telnet parser is OFF.",
	  "Telnet parser is ON."
	},

	{ "noiac",		"no IACs in output",
	  comm_flags,	COMM_NOIAC,
	  "IACs will not be sent to you anymore.",
	  "Text will be sent to you unmodified."
	},

	{ "noverbose",		"no verbose messages",
	  comm_flags,	COMM_NOVERBOSE,
	  "You will no longer see verbose messages.",
	  "Now you will see verbose messages."
	},

	{ "prompt",		"show prompt",
	  comm_flags,	COMM_PROMPT,
	  "You will now see prompts.",
	  "You will no longer see prompts."
	},

	{ "telnet GA",		"send IAC GA (goahead) after each prompt",
	  comm_flags,	COMM_TELNET_GA,
	  "IAC GA will be sent after each prompt.",
	  "IAC GA will not be sent after prompts."
	},

	{ "quiet edit",		"quiet mode in string editor",
	  comm_flags,	COMM_QUIET_EDITOR,
	  "$t set.",
	  "$t removed."
	},

	{ "showrace",		"show mobiles and chars race in long desc",
	  comm_flags,	COMM_SHOW_RACE,
	  "You will now see race in long desc.",
	  "You will no longer see race in long desc."
	},

	{ "short equipment",	"short equipment mode",
	  comm_flags,	COMM_SHORT_EQ,
	  "You will no longer see empty equipment slots.",
	  "You will now see empty equipment slots."
	},

	{ "short aff",		"short affects mode",
	  comm_flags,	COMM_SHORT_AFF,
	  "Short affects mode on.",
	  "Short affects mode off."
	},

	{ "battle",		"do not show battle prompt",
	  comm_flags,	COMM_NO_BATTLE_PROMPT,
	  "You will no longer see battle prompt when fighting.",
	  "You will now see battle prompt when fighting."
	},

	{ NULL }
};

void do_toggle(CHAR_DATA *ch, const char *argument)
{
	toggle_t *t;
	char arg[MAX_INPUT_LENGTH];

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		char_puts("Your current settings are:\n", ch);
		for (t = toggle_table; t->name; t++)
			toggle_print(ch, t);
		return;
	}

	for (; arg[0]; argument = one_argument(argument, arg, sizeof(arg))) {
		flag64_t* bits;
		const char *p;

		if ((t = toggle_lookup(arg)) == NULL
		||  (bits = toggle_bits(ch, t)) == NULL) {
			char_printf(ch, "%s: no such toggle.\n", arg);
			continue;
		}

		p = one_argument(argument, arg, sizeof(arg));
		if (!str_cmp(arg, "on")) {
			SET_BIT(*bits, t->bit);
			argument = p;
		} else if (!str_cmp(arg, "off")) {
			REMOVE_BIT(*bits, t->bit);
			argument = p;
		} else {
			TOGGLE_BIT(*bits, t->bit);
		}
		act_puts(IS_SET(*bits, t->bit) ? t->msg_on : t->msg_off,
			 ch, t->desc, NULL, TO_CHAR, POS_DEAD);
	}
}
void do_ooc(CHAR_DATA *ch, const char *argument)
{
	DESCRIPTOR_DATA *d;

	if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
		return;

        if (IS_SET(ch->in_room->room_flags, ROOM_SILENT)
        &&  !IS_IMMORTAL(ch)) {
                char_puts("You are in silent room.\n", ch);
                return;
        }

	if (argument[0] == '\0') {
		TOGGLE_BIT(ch->comm, COMM_NOOOC);
		if (IS_SET(ch->comm, COMM_NOOOC)) {
			char_puts("You will no longer hear general "
				  "Shades of Gray line.\n", ch);
		} else {
			char_puts("You will now hear general "
				  "Shades of Gray line.\n", ch);
		}
		return;
	}

	if (IS_SET(ch->comm, COMM_NOCHANNELS)) {
		 char_puts("The gods have revoked your channel privileges.\n",
			   ch);
		 return;
	}

	if (IS_SET(ch->comm, COMM_NOOOC))
		do_ooc(ch, str_empty);
	WAIT_STATE(ch, PULSE_VIOLENCE);

	argument = garble(ch, argument);
	act_puts("[Shades of Gray] $lu{$n}: {C$t{x",
		 ch, argument, NULL,
		 TO_CHAR | (ACT_SPEECH(ch) & ~ACT_STRANS), POS_DEAD);

	for (d = descriptor_list; d; d = d->next) {
		if (d->connected == CON_PLAYING
		&&  d->character != ch
		&&  !IS_SET(d->character->comm, COMM_NOOOC)
		&&  (d->character->in_room == NULL ||
		     !IS_SET(d->character->in_room->room_flags, ROOM_SILENT) ||
		     IS_IMMORTAL(d->character))) {
			act_puts("[Shades of Gray] $lu{$n}: {C$t{x",
				 ch, argument, d->character,
				 TO_VICT | ACT_NOTWIT |
				 (ACT_SPEECH(ch) & ~ACT_STRANS),
				 POS_DEAD);
		}
	}
}
void do_oocme(CHAR_DATA *ch, const char *argument)
{
	DESCRIPTOR_DATA *d;

	if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
		return;

        if (IS_SET(ch->in_room->room_flags, ROOM_SILENT)
        &&  !IS_IMMORTAL(ch)) {
                char_puts("You are in silent room.\n", ch);
                return;
        }

	if (argument[0] == '\0')
		return;

	if (IS_SET(ch->comm, COMM_NOCHANNELS)) {
		 char_puts("The gods have revoked your channel privileges.\n",
			   ch);
		 return;
	}

	WAIT_STATE(ch, PULSE_VIOLENCE);

	if (is_affected(ch, gsn_garble))
		return;

	act_puts("[Shades of Gray] {C$lu{$n} $t{x",
		 ch, argument, NULL,
		 TO_CHAR | (ACT_SPEECH(ch) & ~ACT_STRANS), POS_DEAD);

	for (d = descriptor_list; d; d = d->next) {
		if (d->connected == CON_PLAYING
		&&  d->character != ch
		&&  !IS_SET(d->character->comm, COMM_NOOOC)
		&&  (d->character->in_room == NULL ||
		     !IS_SET(d->character->in_room->room_flags, ROOM_SILENT) ||
		     IS_IMMORTAL(d->character))) {
			act_puts("[Shades of Gray] {C$lu{$n} $t{x",
				 ch, argument, d->character,
				 TO_VICT | ACT_NOTWIT |
				 (ACT_SPEECH(ch) & ~ACT_STRANS),
				 POS_DEAD);
		}
	}
}

static toggle_t *toggle_lookup(const char *name)
{
	toggle_t *t;

	for (t = toggle_table; t->name; t++)
		if (!str_prefix(name, t->name))
			break;
	return t;
}

static void toggle_print(CHAR_DATA *ch, toggle_t *t)
{
	char buf[MAX_STRING_LENGTH];
	flag64_t *bits;

	if ((bits = toggle_bits(ch, t)) == NULL)
		return;

	snprintf(buf, sizeof(buf), "  %-11.11s - %-3.3s ($t)",
		 t->name, IS_SET(*bits, t->bit) ? "ON" : "OFF");
	act_puts(buf, ch, t->desc, NULL, TO_CHAR, POS_DEAD);
}

static flag64_t* toggle_bits(CHAR_DATA *ch, toggle_t *t)
{
	if (t->f == comm_flags)
		return &ch->comm;
	return NULL;
}

