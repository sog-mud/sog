/*
 * $Id: act_comm.c,v 1.146 1999-02-19 15:21:39 fjoe Exp $
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
#include "quest.h"
#include "mob_prog.h"
#include "obj_prog.h"
#include "auction.h"
#include "db/lang.h"
#include "db/gsn.h"

/* command procedures needed */
DECLARE_DO_FUN(do_quit		);
DECLARE_DO_FUN(do_quit_count	);
DECLARE_DO_FUN(do_replay	);
DECLARE_DO_FUN(do_look		);

void do_quit_org	(CHAR_DATA *ch, const char *argument, bool Count);

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
	
	if (IS_SET(ch->plr_flags, PLR_CONFIRM_DELETE)) {
		char *name;

		if (argument[0] != '\0') {
			char_puts("Delete status removed.\n",ch);
			REMOVE_BIT(ch->plr_flags, PLR_CONFIRM_DELETE);
			return;
		}

		wiznet("$N turns $Mself into line noise.", ch, NULL, 0, 0, 0);
		RESET_FIGHT_TIME(ch);
		name = capitalize(ch->name);
		do_quit_count(ch, str_empty);
		dunlink(PLAYER_PATH, name);
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
	SET_BIT(ch->plr_flags, PLR_CONFIRM_DELETE);
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
	
	char_printf(ch, "You display %d lines of scroll.\n", ch->lines+2);

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
		char_puts("You can't replay.\n",ch);
		return;
	}

	page_to_char(buf_string(ch->pcdata->buffer), ch);
	buf_clear(ch->pcdata->buffer);
}

static const char *garble(CHAR_DATA *ch, const char *i)
{
	static char not_garbled[] = "?!()[]{},.:;'\" ";
	static char buf[MAX_STRING_LENGTH];
	char *o;

	if (!is_affected(ch, gsn_garble))
		return i;

	for (o = buf; *i && o-buf < sizeof(buf)-1; i++, o++) {
		if (strchr(not_garbled, *i))
			*o = *i;
		else
			*o = number_range(' ', 254);
	}
	*o = '\0';
	return buf;
}

void do_say(CHAR_DATA *ch, const char *argument)
{
	OBJ_DATA *char_obj;
	OBJ_DATA *char_obj_next;
  
	if (argument[0] == '\0') {
		char_puts("Say what?\n", ch);
		return;
	}

	argument = garble(ch, argument);
	act_puts("You say '{G$t{x'", ch, argument, NULL,
		 TO_CHAR | ACT_NODEAF, POS_DEAD);
	act("$n says '{G$t{x'", ch, argument, NULL,
	    TO_ROOM | ACT_TOBUF | ACT_NOTWIT | ACT_STRANS | ACT_NODEAF |
	    (IS_NPC(ch) && !IS_AFFECTED(ch, AFF_CHARM) ? ACT_TRANS : 0));

	if (!IS_NPC(ch)) {
 		CHAR_DATA *mob, *mob_next;
		for (mob = ch->in_room->people; mob != NULL; mob = mob_next) {
 			mob_next = mob->next_in_room;
 			if (IS_NPC(mob) && HAS_TRIGGER(mob, TRIG_SPEECH)
 			&&  mob->position == mob->pIndexData->default_pos)
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

void do_tell_raw(CHAR_DATA *ch, CHAR_DATA *victim, const char *msg)
{
	if (IS_SET(ch->comm, COMM_NOTELL)) {
		char_puts("Your message didn't get through.\n", ch);
		return;
	}

	if (victim == NULL 
	|| (IS_NPC(victim) && victim->in_room != ch->in_room)) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	if (IS_SET(victim->comm, (COMM_QUIET | COMM_DEAF))
	&&  !IS_IMMORTAL(ch) && !IS_IMMORTAL(victim)) {
		act_puts("$E is not receiving tells.", ch, 0, victim,
			 TO_CHAR, POS_DEAD);
		return;
	}

	msg = garble(ch, msg);
	act_puts("You tell $N '{G$t{x'",
		 ch, msg, victim, TO_CHAR | ACT_NODEAF, POS_DEAD);
	act_puts("$n tells you '{G$t{x'",
		 ch, msg, victim,
		 TO_VICT | ACT_TOBUF | ACT_NOTWIT | ACT_NODEAF |
		 (IS_NPC(ch) && !IS_AFFECTED(ch, AFF_CHARM) ? ACT_TRANS : 0),
		 POS_SLEEPING);

	if (IS_NPC(ch))
		return;

	if (IS_NPC(victim)) {
		if (HAS_TRIGGER(victim, TRIG_SPEECH))
			mp_act_trigger(msg, victim, ch, NULL, NULL, TRIG_SPEECH);
	}
	else {
		if (!IS_IMMORTAL(victim)
		&&  !IS_IMMORTAL(ch)
		&&  is_name(ch->name, victim->pcdata->twitlist))
			return;

		if (victim->desc == NULL)
			act_puts("$N seems to have misplaced $S link but "
				 "your tell will go through if $E returns.",
				 ch, NULL, victim, TO_CHAR, POS_DEAD);
		else if (IS_SET(victim->comm, COMM_AFK))
			act_puts("$E is AFK, but your tell will go through "
				 "when $E returns.",
				 ch, NULL, victim, TO_CHAR, POS_DEAD);
		victim->reply = ch;
	}
}

void do_tell(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0' || argument[0] == '\0') {
		char_puts("Tell whom what?\n", ch);
		return;
	}

	do_tell_raw(ch, get_char_world(ch, arg), argument);
}

void do_reply(CHAR_DATA *ch, const char *argument)
{
	do_tell_raw(ch, ch->reply, argument);
}

void do_gtell(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *gch;
	int i;
	int flags;

	if (argument[0] == '\0') {
		char_puts("Tell your group what?\n", ch);
		return;
	}

	if (IS_SET(ch->comm, COMM_NOTELL)) {
		char_puts("Your message didn't get through!\n", ch);
		return;
	}

	argument = garble(ch, argument);
	flags = TO_VICT | ACT_TOBUF | ACT_STRANS | 
		(IS_NPC(ch) && !IS_AFFECTED(ch, AFF_CHARM) ? ACT_TRANS : 0);
	for (i = 0, gch = char_list; gch; gch = gch->next) {
		if (is_same_group(gch, ch) && !is_affected(gch, gsn_deafen)) {
			act_puts("$n tells the group '{G$t{x'",
				 ch, argument, gch, flags, POS_DEAD);
			i++;
		}
	}

	if (i > 1 && !is_affected(ch, gsn_deafen))
		act_puts("You tell your group '{G$t{x'",
			 ch, argument, NULL, TO_CHAR, POS_DEAD);
	else
		char_puts("Quit talking to yourself. You are all alone.\n", ch);
}

void do_emote(CHAR_DATA *ch, const char *argument)
{
	if (!IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE)) {
		char_puts("You can't show your emotions.\n", ch);
		return;
	}
	
	if (argument[0] == '\0') {
		char_puts("Emote what?\n", ch);
		return;
	}
	
	argument = garble(ch, argument);
	act("$n $T", ch, NULL, argument,
	    TO_ROOM | ACT_TOBUF | ACT_NOTRIG | ACT_NOTWIT |
	    (IS_NPC(ch) && !IS_AFFECTED(ch, AFF_CHARM) ? ACT_TRANS : 0));
	act("$n $T", ch, NULL, argument, TO_CHAR | ACT_NOTRIG);
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
	act("$n $t", ch, argument, NULL, TO_CHAR);

	flags = TO_CHAR | ACT_TOBUF | ACT_NOTWIT | ACT_NOTRIG |
		(IS_NPC(ch) && !IS_AFFECTED(ch, AFF_CHARM) ? ACT_TRANS : 0);

	for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room) {
		if (vch->desc == NULL || vch == ch)
			continue;

		if ((letter = strstr(argument, vch->name)) == NULL) {
			act("$N $t", vch, argument, ch, flags);
			continue;
		}

		strnzcpy(temp, argument, sizeof(temp));
		temp[strlen(argument) - strlen(letter)] = '\0';
		last[0] = '\0';
		name = vch->name;
	
		for (; *letter != '\0'; letter++) { 
			if (*letter == '\'' && matches == strlen(vch->name)) {
				strnzcat(temp, "r", sizeof(temp));
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
					strnzcat(temp, "you", sizeof(temp));
					last[0] = '\0';
					name = vch->name;
					continue;
				}
				strnzncat(last, letter, sizeof(last), 1);
				continue;
			}

			matches = 0;
			strnzcat(temp, last, sizeof(temp));
			strnzncat(temp, letter, sizeof(temp), 1);
			last[0] = '\0';
			name = vch->name;
		}

		act("$N $t", vch, temp, ch, flags);
	}
}

void do_immtalk(CHAR_DATA *ch, const char *argument)
{
	DESCRIPTOR_DATA *d;

	if (argument[0] == '\0') {
		TOGGLE_BIT(ch->comm, COMM_NOWIZ);
		if (IS_SET(ch->comm, COMM_NOWIZ))
			char_puts("Immortal channel is now OFF\n",ch);
		else
			char_puts("Immortal channel is now ON\n",ch);
		return;
	}

	if (IS_SET(ch->comm, COMM_NOCHANNELS)) {
		char_puts("The gods have revoked your channel privileges.\n", ch);
		return;
	}
	
	if (IS_SET(ch->comm, COMM_NOWIZ))
		do_immtalk(ch, str_empty);

	act_puts("$n: {C$t{x", ch, argument, NULL, TO_CHAR, POS_DEAD);
	for (d = descriptor_list; d; d = d->next) {
		CHAR_DATA *victim = d->original ? d->original : d->character;

		if (d->connected == CON_PLAYING
		&&  IS_IMMORTAL(victim)
		&&  !IS_SET(victim->comm, COMM_NOWIZ))
			act_puts("$n: {C$t{x", ch, argument, d->character,
				 TO_VICT | ACT_TOBUF, POS_DEAD);
	}
}

void do_yell(CHAR_DATA *ch, const char *argument)
{
	DESCRIPTOR_DATA *d;

	if (IS_SET(ch->comm, COMM_NOCHANNELS)) {
		 char_puts("The gods have revoked your channel privileges.\n", ch);
		 return;
	}
	
	if (argument[0] == '\0') {
		char_puts("Yell what?\n", ch);
		return;
	}

	if (ch->in_room == NULL)
		return;

	argument = garble(ch, argument);
	act_puts("You yell '{M$t{x'",
		 ch, argument, NULL, TO_CHAR | ACT_NODEAF, POS_DEAD);

	for (d = descriptor_list; d; d = d->next) {
		if (d->connected == CON_PLAYING
		&&  d->character != ch
		&&  d->character->in_room != NULL
		&&  d->character->in_room->area == ch->in_room->area)
			act_puts("$n yells '{M$t{x'",
				 ch, argument, d->character,
	    			 TO_VICT | ACT_STRANS | ACT_NODEAF,
				 POS_DEAD);
	}
}

void do_shout(CHAR_DATA *ch, const char *argument)
{
	DESCRIPTOR_DATA *d;

	if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
		return;

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
		 ch, argument, NULL,
		 TO_CHAR | ACT_NODEAF, POS_DEAD);

	for (d = descriptor_list; d; d = d->next) {
		if (d->connected == CON_PLAYING
		&&  d->character != ch
		&&  !IS_SET(d->character->comm, COMM_NOSHOUT))
			act_puts("$n shouts '{Y$t{x'",
				 ch, argument, d->character,
	    			 TO_VICT | ACT_NOTWIT |
				 ACT_STRANS | ACT_NODEAF,
				 POS_DEAD);
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

	if (IS_SET(ch->comm, COMM_NOCHANNELS)) {
		 char_puts("The gods have revoked your channel privileges.\n", ch);
		 return;
	}
	
	if (IS_SET(ch->comm, COMM_NOMUSIC))
		do_music(ch, str_empty);
	WAIT_STATE(ch, PULSE_VIOLENCE);

	argument = garble(ch, argument);
	act_puts("You music '{W$t{x'",
		 ch, argument, NULL,
		 TO_CHAR | ACT_NODEAF, POS_DEAD);

	for (d = descriptor_list; d; d = d->next) {
		if (d->connected == CON_PLAYING
		&&  d->character != ch
		&&  !IS_SET(d->character->comm, COMM_NOMUSIC))
			act_puts("$n musics '{W$t{x'",
		        	 ch, argument, d->character,
	    			 TO_VICT | ACT_NOTWIT |
				 ACT_STRANS | ACT_NODEAF,
				 POS_DEAD);
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
		 ch, argument, NULL,
		 TO_CHAR | ACT_NODEAF, POS_DEAD);

	for (d = descriptor_list; d; d = d->next) {
		if (d->connected == CON_PLAYING
		&&  d->character != ch)
			act_puts("$n gossips '{R$t{x'",
		        	 ch, argument, d->character,
	    			 TO_VICT | ACT_STRANS | ACT_NODEAF,
				 POS_DEAD);
	}
}

void do_clan(CHAR_DATA *ch, const char *argument)
{
	CLAN_DATA *clan;
	CHAR_DATA *vch;
	int flags;

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

	if (IS_SET(ch->comm, COMM_NOCLAN))
		do_clan(ch, str_empty);

	argument = garble(ch, argument);
	act_puts("[CLAN] $n: {C$t{x",
		 ch, argument, NULL, TO_CHAR | ACT_NODEAF, POS_DEAD);

	flags = TO_VICT | ACT_TOBUF | ACT_NODEAF |
		(IS_NPC(ch) && !IS_AFFECTED(ch, AFF_CHARM) ? ACT_TRANS : 0);
	for (vch = char_list; vch; vch = vch->next)
		if (vch->clan == ch->clan
		&&  vch != ch
		&&  !IS_SET(vch->comm, COMM_NOCLAN))
			act_puts("[CLAN] $n: {C$t{x",
				 ch, argument, vch, flags, POS_DEAD);
}

void do_pray(CHAR_DATA *ch, const char *argument)
{
	DESCRIPTOR_DATA *d;

	if (IS_NPC(ch))
		return;

	if (IS_SET(ch->comm, COMM_NOCHANNELS)) {
		 char_puts("The gods refuse to listen to you right now.",ch);
		 return;
	}
	
	char_puts("You pray to the heavens for help!\n",ch);
	char_puts("This is not an emote, but a channel to the immortals.\n",
		ch);

	if (argument[0] == '\0')
		argument = "any god";

	for (d = descriptor_list; d; d = d->next) {
		CHAR_DATA *victim = d->original ? d->original : d->character;

		if (d->connected == CON_PLAYING
		&&  IS_IMMORTAL(victim)
		&&  !IS_SET(victim->comm, COMM_NOWIZ))
			act_puts("{W$n{x is PRAYING for: {G$t{x",
				 ch, argument, d->character,
				 TO_VICT | ACT_TOBUF, POS_DEAD);
	 }
}

#if 0
/*
 * All the posing stuff.
 */
struct	pose_table_type
{
	char *	message[2*MAX_CLASS];
};

const	struct	pose_table_type	pose_table	[]	=
{
	{
	{
		"You sizzle with energy.",
		"$n sizzles with energy.",
		"You feel very holy.",
		"$n looks very holy.",
		"You perform a small card trick.",
		"$n performs a small card trick.",
		"You show your bulging muscles.",
		"$n shows $s bulging muscles."
	}
	},

	{
	{
		"You turn into a butterfly, then return to your normal shape.",
		"$n turns into a butterfly, then returns to $s normal shape.",
		"You nonchalantly turn wine into water.",
		"$n nonchalantly turns wine into water.",
		"You wiggle your ears alternately.",
		"$n wiggles $s ears alternately.",
		"You crack nuts between your fingers.",
		"$n cracks nuts between $s fingers."
	}
	},

	{
	{
		"Blue sparks fly from your fingers.",
		"Blue sparks fly from $n's fingers.",
		"A halo appears over your head.",
		"A halo appears over $n's head.",
		"You nimbly tie yourself into a knot.",
		"$n nimbly ties $mself into a knot.",
		"You grizzle your teeth and look mean.",
		"$n grizzles $s teeth and looks mean."
	}
	},

	{
	{
		"Little red lights dance in your eyes.",
		"Little red lights dance in $n's eyes.",
		"You recite words of wisdom.",
		"$n recites words of wisdom.",
		"You juggle with daggers, apples, and eyeballs.",
		"$n juggles with daggers, apples, and eyeballs.",
		"You hit your head, and your eyes roll.",
		"$n hits $s head, and $s eyes roll."
	}
	},

	{
	{
		"A slimy green monster appears before you and bows.",
		"A slimy green monster appears before $n and bows.",
		"Deep in prayer, you levitate.",
		"Deep in prayer, $n levitates.",
		"You steal the underwear off every person in the room.",
		"Your underwear is gone!  $n stole it!",
		"Crunch, crunch -- you munch a bottle.",
		"Crunch, crunch -- $n munches a bottle."
	}
	},

	{
	{
		"You turn everybody into a little pink elephant.",
		"You are turned into a little pink elephant by $n.",
		"An angel consults you.",
		"An angel consults $n.",
		"The dice roll ... and you win again.",
		"The dice roll ... and $n wins again.",
		"... 98, 99, 100 ... you do pushups.",
		"... 98, 99, 100 ... $n does pushups."
	}
	},

	{
	{
		"A small ball of light dances on your fingertips.",
		"A small ball of light dances on $n's fingertips.",
		"Your body glows with an unearthly light.",
		"$n's body glows with an unearthly light.",
		"You count the money in everyone's pockets.",
		"Check your money, $n is counting it.",
		"Arnold Schwarzenegger admires your physique.",
		"Arnold Schwarzenegger admires $n's physique."
	}
	},

	{
	{
		"Smoke and fumes leak from your nostrils.",
		"Smoke and fumes leak from $n's nostrils.",
		"A spot light hits you.",
		"A spot light hits $n.",
		"You balance a pocket knife on your tongue.",
		"$n balances a pocket knife on your tongue.",
		"Watch your feet, you are juggling granite boulders.",
		"Watch your feet, $n is juggling granite boulders."
	}
	},

	{
	{
		"The light flickers as you rap in magical languages.",
		"The light flickers as $n raps in magical languages.",
		"Everyone levitates as you pray.",
		"You levitate as $n prays.",
		"You produce a coin from everyone's ear.",
		"$n produces a coin from your ear.",
		"Oomph!  You squeeze water out of a granite boulder.",
		"Oomph!  $n squeezes water out of a granite boulder."
	}
	},

	{
	{
		"Your head disappears.",
		"$n's head disappears.",
		"A cool breeze refreshes you.",
		"A cool breeze refreshes $n.",
		"You step behind your shadow.",
		"$n steps behind $s shadow.",
		"You pick your teeth with a spear.",
		"$n picks $s teeth with a spear."
	}
	},

	{
	{
		"A fire elemental singes your hair.",
		"A fire elemental singes $n's hair.",
		"The sun pierces through the clouds to illuminate you.",
		"The sun pierces through the clouds to illuminate $n.",
		"Your eyes dance with greed.",
		"$n's eyes dance with greed.",
		"Everyone is swept off their foot by your hug.",
		"You are swept off your feet by $n's hug."
	}
	},

	{
	{
		"The sky changes color to match your eyes.",
		"The sky changes color to match $n's eyes.",
		"The ocean parts before you.",
		"The ocean parts before $n.",
		"You deftly steal everyone's weapon.",
		"$n deftly steals your weapon.",
		"Your karate chop splits a tree.",
		"$n's karate chop splits a tree."
	}
	},

	{
	{
		"The stones dance to your command.",
		"The stones dance to $n's command.",
		"A thunder cloud kneels to you.",
		"A thunder cloud kneels to $n.",
		"The Grey Mouser buys you a beer.",
		"The Grey Mouser buys $n a beer.",
		"A strap of your armor breaks over your mighty thews.",
		"A strap of $n's armor breaks over $s mighty thews."
	}
	},

	{
	{
		"The heavens and grass change colour as you smile.",
		"The heavens and grass change colour as $n smiles.",
		"The Burning Man speaks to you.",
		"The Burning Man speaks to $n.",
		"Everyone's pocket explodes with your fireworks.",
		"Your pocket explodes with $n's fireworks.",
		"A boulder cracks at your frown.",
		"A boulder cracks at $n's frown."
	}
	},

	{
	{
		"Everyone's clothes are transparent, and you are laughing.",
		"Your clothes are transparent, and $n is laughing.",
		"An eye in a pyramid winks at you.",
		"An eye in a pyramid winks at $n.",
		"Everyone discovers your dagger a centimeter from their eye.",
		"You discover $n's dagger a centimeter from your eye.",
		"Mercenaries arrive to do your bidding.",
		"Mercenaries arrive to do $n's bidding."
	}
	},

	{
	{
		"A black hole swallows you.",
		"A black hole swallows $n.",
		"Valentine Michael Smith offers you a glass of water.",
		"Valentine Michael Smith offers $n a glass of water.",
		"Where did you go?",
		"Where did $n go?",
		"Four matched Percherons bring in your chariot.",
		"Four matched Percherons bring in $n's chariot."
	}
	},

	{
	{
		"The world shimmers in time with your whistling.",
		"The world shimmers in time with $n's whistling.",
		"Gods give you a staff.",
		"Gods give $n a staff.",
		"Click.",
		"Click.",
		"Atlas asks you to relieve him.",
		"Atlas asks $n to relieve him."
	}
	}
};
#endif

void do_pose(CHAR_DATA *ch, const char *argument)
{
#if 0
	int level;
	int pose;

	if (IS_NPC(ch))
		return;

	level = UMIN(ch->level, sizeof(pose_table) / sizeof(pose_table[0]) - 1);
	pose  = number_range(0, level);
	act(pose_table[pose].message[2*ch->class+0], ch, NULL, NULL, TO_CHAR);
	act(pose_table[pose].message[2*ch->class+1], ch, NULL, NULL, TO_ROOM | ACT_TOBUF);
#endif
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
	return;
}


void do_quit(CHAR_DATA *ch, const char *argument)
{
	do_quit_org(ch, argument, FALSE);
	return;
}

void do_quit_count(CHAR_DATA *ch, const char *argument)
{
	do_quit_org(ch, argument, TRUE);
	return;
}

void do_quit_org(CHAR_DATA *ch, const char *argument, bool Count)
{
	DESCRIPTOR_DATA *d, *d_next;
	CHAR_DATA *vch, *vch_next;
	OBJ_DATA *obj,*obj_next,*obj_in;
	int cn;
	int id;

	if (IS_NPC(ch))
		return;

	if (ch->position == POS_FIGHTING) {
		char_puts("No way! You are fighting.\n", ch);
		return;
	}

	if (ch->position < POS_STUNNED ) {
		char_puts("You're not DEAD yet.\n", ch);
		return;
	}

	if (IS_AFFECTED(ch, AFF_CHARM)) {
		char_puts("You don't want to leave your master.\n", ch);
		return;
	}

	if (IS_SET(ch->plr_flags, PLR_NOEXP)) {
		char_puts("You don't want to lose your spirit.\n", ch);
		return;
	}

	if (IS_AFFECTED(ch, AFF_SLEEP)) {
		char_puts("You cannot quit, you are in deep sleep.\n", ch);
		return;
	}

	if (auction.item != NULL
	&&  ((ch == auction.buyer) || (ch == auction.seller))) {
		char_puts("Wait till you have sold/bought the item "
			  "on auction.\n",ch);
		return;
	}

	if (!IS_IMMORTAL(ch)) {
		if (IS_PUMPED(ch)) {
			char_puts("Your adrenalin is gushing! You can't quit yet.\n", ch);
			return;
		}

		if (is_affected(ch, gsn_witch_curse)) {
			char_puts("You are cursed. Wait till you DIE!\n", ch);
			return;
		}

		if (ch->in_room->area->clan
		&&  ch->in_room->area->clan != ch->clan) {
			char_puts("You can't quit here.\n", ch);
			return;
		}

		if (ch->in_room && IS_RAFFECTED(ch->in_room, RAFF_ESPIRIT)) {
			char_puts("Evil spirits in the area prevents you from leaving.\n", ch);
			return;
		}

		if (!get_skill(ch, gsn_evil_spirit)
		&&  is_affected(ch, gsn_evil_spirit)) {
			char_puts("Evil spirits in you prevents you from leaving.\n", ch);
			return;
		}
	}

	char_puts("Alas, all good things must come to an end.\n", ch);
	char_puts("You hit reality hard. Reality truth does unspeakable things to you.\n", ch);
	act_puts("$n has left the game.", ch, NULL, NULL, TO_ROOM, POS_RESTING);
	log_printf("%s has quit.", ch->name);
	wiznet("{W$N{x rejoins the real world.",
		ch, NULL, WIZ_LOGINS, 0, ch->level);

	/*
	 * remove quest objs for this char, drop quest objs for other chars
	 */
	for (obj = object_list; obj != NULL; obj = obj_next) {
		obj_next = obj->next;
		if (obj->pIndexData->vnum >= QUEST_OBJ_FIRST
		&&  obj->pIndexData->vnum <= QUEST_OBJ_LAST)
			if (obj->ed == NULL ||
			    strstr(mlstr_mval(obj->ed->description),
							ch->name) != NULL)
				extract_obj(obj);
			else if (obj->carried_by == ch) {
				obj_from_char(obj);
				obj_to_room(obj,ch->in_room);
			}
		if (IS_SET(obj->pIndexData->extra_flags, ITEM_CLAN)
		  ||IS_SET(obj->pIndexData->extra_flags, ITEM_QUIT_DROP)) {
			if (obj->in_room != NULL) continue;
			if ((obj_in=obj->in_obj) != NULL) {
				for (;obj_in->in_obj != NULL; obj_in = obj_in->in_obj);
				if (obj_in->carried_by != ch) continue;
			};
			if (obj->carried_by == ch) obj_from_char(obj);
			else if (obj->carried_by != NULL) continue;
			if (obj_in != NULL) obj_from_obj (obj);
			if (IS_SET(obj->pIndexData->extra_flags, ITEM_QUIT_DROP)) {
				if (ch->in_room != NULL) 
					obj_to_room(obj, ch->in_room);
				else 
					extract_obj(obj);
			}
			else for (cn=0; cn < clans.nused; cn++)
			    if (obj == clan_lookup(cn)->obj_ptr) 
				obj_to_room(obj, get_room_index(clan_lookup(cn)->altar_vnum));
		}
	}

	for (vch = char_list; vch; vch = vch_next) {
		vch_next = vch->next;
		if (is_affected(vch, gsn_doppelganger)
		&&  vch->doppel == ch) {
			char_puts("You shift to your true form as your victim leaves.\n",
				  vch);
			affect_strip(vch, gsn_doppelganger);
		}

		if (vch->guarding == ch) {
			act("You stops guarding $N.", vch, NULL, ch, TO_CHAR);
			act("$n stops guarding you.", vch, NULL, ch, TO_VICT);
			act("$n stops guarding $N.", vch, NULL, ch, TO_NOTVICT);
			vch->guarding  = NULL;
			ch->guarded_by = NULL;
		}

		if (vch->last_fought == ch) {
			vch->last_fought = NULL;
			back_home(vch);
		}
		
		if (vch->hunting == ch)
			vch->hunting = NULL;

		if (vch->hunter == ch)
			vch->hunter = NULL;

		if (vch->target == ch) {
			if (IS_NPC(vch) 
			  && vch->pIndexData->vnum == MOB_VNUM_SHADOW) {
				act ("$n slowly fades away.",
				vch, NULL, NULL, TO_ROOM);
				extract_char(vch, TRUE);
				continue;
			}
			if (IS_NPC(vch)
			  && vch->pIndexData->vnum == MOB_VNUM_STALKER) {
				doprintf(do_clan, vch, 
					"%s has left the realm, I have to leave too.",
					PERS(ch, vch));
				act ("$n slowly fades away.", vch, NULL, NULL,
					TO_ROOM);
				extract_char(vch, TRUE);
			  }
		}
	}

	if (ch->guarded_by != NULL) {
		ch->guarded_by->guarding = NULL;
		ch->guarded_by = NULL;
	}

	/*
	 * After extract_char the ch is no longer valid!
	 */
	save_char_obj(ch, FALSE);
	id = ch->id;
	d = ch->desc;
	if (Count)
		 extract_char(ch, TRUE);
	else
		 extract_char_nocount(ch, TRUE);

	if (d)
		close_descriptor(d);

	/* toast evil cheating bastards    */
	for (d = descriptor_list; d; d = d_next) {
		CHAR_DATA *tch;

		d_next = d->next;
		tch = d->original ? d->original : d->character;
		if (tch && tch->id == id) {
			extract_char_nocount(tch, TRUE);
			close_descriptor(d);
		} 
	}
}

void do_save(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch))
		return;

	if (ch->level < 2) {
		char_puts("You must be at least level 2 for saving.\n",ch);
		return;
	}
	save_char_obj(ch, FALSE);
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
	&&  IS_SET(victim->plr_flags, PLR_NOFOLLOW)
	&&  !IS_IMMORTAL(ch)) {
		act("$N doesn't seem to want any followers.\n",
		    ch, NULL, victim, TO_CHAR);
		return;
	}

	REMOVE_BIT(ch->plr_flags, PLR_NOFOLLOW);
	
	if (ch->master != NULL)
		stop_follower(ch);

	add_follower(ch, victim);
}

void add_follower(CHAR_DATA *ch, CHAR_DATA *master)
{
	if (ch->master != NULL) {
		bug("Add_follower: null master.", 0);
		return;
	}

	ch->master        = master;
	ch->leader        = NULL;

	if (can_see(master, ch))
		act_puts("$n now follows you.", ch, NULL, master, 
			 TO_VICT, POS_RESTING);
	act_puts("You now follow $N.", ch, NULL, master, 
		 TO_CHAR, POS_RESTING);
}

void stop_follower(CHAR_DATA *ch)
{
	if (ch->master == NULL) {
		bug("Stop_follower: null master.", 0);
		return;
	}

	if (IS_AFFECTED(ch, AFF_CHARM)) {
		REMOVE_BIT(ch->affected_by, AFF_CHARM);
		affect_bit_strip(ch, TO_AFFECTS, AFF_CHARM);
	}

	if (can_see(ch->master, ch) && ch->in_room != NULL) {
		act_puts("$n stops following you.",ch, NULL, ch->master, 
			 TO_VICT, POS_RESTING);
		act_puts("You stop following $N.", ch, NULL, ch->master, 
			 TO_CHAR, POS_RESTING);
	}

	if (ch->master->pet == ch)
		ch->master->pet = NULL;

	ch->master = NULL;
	ch->leader = NULL;
}

/* nukes charmed monsters and pets */
void nuke_pets(CHAR_DATA *ch)
{    
	CHAR_DATA *pet;

	if ((pet = ch->pet) != NULL)
	{
		stop_follower(pet);
		if (pet->in_room != NULL)
		    act("$N slowly fades away.",ch,NULL,pet,TO_NOTVICT);
		extract_char_nocount(pet,TRUE);
	}
	ch->pet = NULL;

	return;
}



void die_follower(CHAR_DATA *ch)
{
	CHAR_DATA *fch;
	CHAR_DATA *fch_next;

	if (ch->master != NULL)
		stop_follower(ch);

	ch->leader = NULL;

	for (fch = char_list; fch != NULL; fch = fch_next) {
		fch_next = fch->next;
		if (fch->master == ch)
			stop_follower(fch);
		if (fch->leader == ch)
			fch->leader = NULL;
	}
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

CHAR_DATA* leader_lookup(CHAR_DATA* ch)
{
	CHAR_DATA* res;
	for (res = ch; res->leader != NULL; res = res->leader)
		;
	return res;
}

void do_group(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		CHAR_DATA *gch;

		char_printf(ch, "%s's group:\n", PERS(leader_lookup(ch), ch));

		for (gch = char_list; gch != NULL; gch = gch->next)
			if (is_same_group(gch, ch))
				char_printf(ch,
					    "[%2d %s] %-16s %d/%d hp "
					    "%d/%d mana %d/%d mv   %5d xp\n",
					    gch->level,
					    class_who_name(gch),
					    PERS(gch, ch),
					    gch->hit,   gch->max_hit,
					    gch->mana,  gch->max_mana,
					    gch->move,  gch->max_move,
					    gch->exp);
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
		if (ch->guarding == victim || victim->guarded_by == ch) {
			act("You stop guarding $N.", ch, NULL, victim, TO_CHAR);
			act("$n stops guarding you.",
			    ch, NULL, victim, TO_VICT);
			act("$n stops guarding $N.",
			   ch, NULL, victim, TO_NOTVICT);
			victim->guarded_by = NULL;
			ch->guarding       = NULL;
		}

		victim->leader = NULL;
		act_puts("$n removes $N from $s group.",   ch, NULL, victim, 
			 TO_NOTVICT,POS_SLEEPING);
		act_puts("$n removes you from $s group.",  ch, NULL, victim, 
			 TO_VICT,POS_SLEEPING);
		act_puts("You remove $N from your group.", ch, NULL, victim, 
			 TO_CHAR,POS_SLEEPING);

		if (victim->guarded_by != NULL
		&&  !is_same_group(victim,victim->guarded_by)) {
			act("You stop guarding $N.",
			    victim->guarded_by, NULL, victim, TO_CHAR);
			act("$n stops guarding you.",
			    victim->guarded_by, NULL, victim, TO_VICT);
			act("$n stops guarding $N.",
			    victim->guarded_by, NULL, victim, TO_NOTVICT);
			victim->guarded_by->guarding = NULL;
			victim->guarded_by           = NULL;
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

	if (IS_GOOD(ch) && IS_EVIL(victim)) {
		act_puts("You are too evil for $n's group.", ch, NULL, victim, 
			 TO_VICT, POS_SLEEPING);
		act_puts("$N is too evil for your group!", ch, NULL, victim, 
			 TO_CHAR, POS_SLEEPING);
		return;
	}

	if (IS_GOOD(victim) && IS_EVIL(ch)) {
		act_puts("You are too pure to join $n's group!", ch, NULL,
			 victim, TO_VICT, POS_SLEEPING);
		act_puts("$N is too pure for your group!", ch, NULL, victim, 
			 TO_CHAR,POS_SLEEPING);
		return;
	}

	if ((HAS_SKILL(ch, gsn_ruler_badge) && HAS_SKILL(victim, gsn_disperse))
	||  (HAS_SKILL(ch, gsn_disperse) && HAS_SKILL(victim, gsn_ruler_badge))
	||  (HAS_SKILL(ch, gsn_evil_spirit) && HAS_SKILL(victim, gsn_riding))
	||  (HAS_SKILL(ch, gsn_riding) && HAS_SKILL(victim, gsn_evil_spirit))
	||  (HAS_SKILL(ch, gsn_mastering_spell) &&
	     HAS_SKILL(victim, gsn_spellbane))
	||  (HAS_SKILL(ch, gsn_spellbane) &&
	     HAS_SKILL(victim, gsn_mastering_spell))) {
		act_puts("You hate $n's cabal, how can you join $n's group?", ch, NULL, victim, TO_VICT, POS_SLEEPING);
		act_puts("You hate $N's cabal, how can you want $N to join your group", ch, NULL, victim, TO_CHAR, POS_SLEEPING);
		return;
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
	char buf[MAX_STRING_LENGTH];
	char arg1[MAX_INPUT_LENGTH],arg2[MAX_INPUT_LENGTH];
	CHAR_DATA *gch;
	int members;
	int amount_gold = 0, amount_silver = 0;
	int share_gold, share_silver;
	int extra_gold, extra_silver;

	argument = one_argument(argument, arg1, sizeof(arg1));
		   one_argument(argument, arg2, sizeof(arg2));

	if (arg1[0] == '\0')
	{
	char_puts("Split how much?\n", ch);
	return;
	}
	
	amount_silver = atoi(arg1);

	if (arg2[0] != '\0')
	amount_gold = atoi(arg2);

	if (amount_gold < 0 || amount_silver < 0)
	{
	char_puts("Your group wouldn't like that.\n", ch);
	return;
	}

	if (amount_gold == 0 && amount_silver == 0)
	{
	char_puts("You hand out zero coins, but no one notices.\n", ch);
	return;
	}

	if (ch->gold <  amount_gold || ch->silver < amount_silver)
	{
	char_puts("You don't have that much to split.\n", ch);
	return;
	}
	
	members = 0;
	for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room)
	{
	if (is_same_group(gch, ch) && !IS_AFFECTED(gch,AFF_CHARM))
		members++;
	}

	if (members < 2)
	{
	char_puts("Just keep it all.\n", ch);
	return;
	}
		
	share_silver = amount_silver / members;
	extra_silver = amount_silver % members;

	share_gold   = amount_gold / members;
	extra_gold   = amount_gold % members;

	if (share_gold == 0 && share_silver == 0)
	{
	char_puts("Don't even bother, cheapskate.\n", ch);
	return;
	}

	ch->silver	-= amount_silver;
	ch->silver	+= share_silver + extra_silver;
	ch->gold 	-= amount_gold;
	ch->gold 	+= share_gold + extra_gold;

	if (share_silver > 0)
	char_printf(ch,
		"You split %d silver coins. Your share is %d silver.\n",
		    amount_silver,share_silver + extra_silver);

	if (share_gold > 0)
	char_printf(ch,
		"You split %d gold coins. Your share is %d gold.\n",
		 amount_gold,share_gold + extra_gold);

	if (share_gold == 0)
	snprintf(buf, sizeof(buf), "$n splits %d silver coins. Your share is %d silver.",
		amount_silver,share_silver);
	else if (share_silver == 0)
	snprintf(buf, sizeof(buf), "$n splits %d gold coins. Your share is %d gold.",
		amount_gold,share_gold);
	else
	snprintf(buf, sizeof(buf),
"$n splits %d silver and %d gold coins, giving you %d silver and %d gold.\n",
		amount_silver,amount_gold,share_silver,share_gold);

	for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room) {
	if (gch != ch && is_same_group(gch,ch) && !IS_AFFECTED(gch,AFF_CHARM)) {
		act(buf, ch, NULL, gch, TO_VICT);
		gch->gold += share_gold;
		gch->silver += share_silver;
	}
	}

	return;
}

void do_speak(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	int language;
	RACE_DATA *r;

	if (IS_NPC(ch)
	||  (r = race_lookup(ch->pcdata->race)) == NULL
	||  !r->pcdata)
		return;

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		char_printf(ch, "You now speak %s.\n", 
			flag_string(slang_table, ch->slang));
		char_puts("You can speak :\n", ch);
		char_printf(ch, "       common, %s\n",
			    flag_string(slang_table, r->pcdata->slang));
		return;
	}

	if ((language = flag_value(slang_table, arg)) < 0) {
		char_puts("You never heard of that language.\n", ch);
		return;
	}

	if (language >= SLANG_MAX)
		ch->slang = r->pcdata->slang;
	else
		ch->slang = language;
	
	char_printf(ch,"Now you speak %s.\n",
		    flag_string(slang_table, ch->slang));
}

DO_FUN(do_twit)
{
	char arg[MAX_STRING_LENGTH];

	if (IS_NPC(ch)) {
		char_puts("Huh?\n", ch);
		return;
	}

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		char_printf(ch, "Current twitlist is [%s]\n",
			    ch->pcdata->twitlist);
		return;
	}

	name_toggle(&ch->pcdata->twitlist, arg, ch, "Twitlist");
}

DO_FUN(do_lang)
{
	char arg[MAX_STRING_LENGTH];
	int lang;
	LANG_DATA *l;

	if (langs.nused == 0) {
		char_puts("No languages defined.\n", ch);
		return;
	}

	argument = one_argument(argument, arg, sizeof(arg));

	if (*arg == '\0') {
		l = varr_get(&langs, ch->lang);
		if (l == NULL) {
			log_printf("do_lang: %s: lang == %d\n",
				   ch->name, ch->lang);
			l = VARR_GET(&langs, ch->lang = 0);
		}
		char_printf(ch, "Interface language is '%s'.\n", l->name);
		return;
	}

	lang = lang_lookup(arg);
	if (lang < 0) {
		char_puts("Usage: lang [ ", ch);
		for (lang = 0; lang < langs.nused; lang++) {
			l = VARR_GET(&langs, lang);
			if (IS_SET(l->flags, LANG_HIDDEN))
				continue;
			char_printf(ch, "%s%s",
				    lang == 0 ? str_empty : " | ", l->name);
		}
		char_puts(" ]\n", ch);
		return;
	}

	ch->lang = lang;
	do_lang(ch, str_empty);
	do_look(ch, str_empty);
}

DO_FUN(do_judge)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	if (!HAS_SKILL(ch, gsn_ruler_badge)) {
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
		    IS_GOOD(victim)	? "good" :
		    IS_EVIL(victim)	? "evil" :
		    IS_NEUTRAL(victim)	? "neutral" :
					  "unknown");
}

DO_FUN(do_trust)
{	
	char arg[MAX_INPUT_LENGTH];
	
	if (IS_NPC(ch)) {
		char_puts("Huh?\n", ch);
		return;
	}

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		if (!ch->pcdata->trust) {
			char_puts("You do not allow anyone to cast questionable spells on you.\n", ch);
			return;
		}

		if (IS_SET(ch->pcdata->trust, TRUST_ALL)) {
			char_puts("You allow everyone to cast questionable spells on you.\n", ch);
			return;
		}
		if (IS_SET(ch->pcdata->trust, TRUST_CLAN)) 
			char_puts("You trust your clan with questionable spells.\n", ch);
		if (IS_SET(ch->pcdata->trust, TRUST_GROUP))
			char_puts("You trust your group with questionable spells.\n", ch);
		return;
	}

	if (!str_cmp(arg, "clan")) {
		if (ch->clan == 0) {
			char_puts("You are not in clan.\n", ch);
			return;
		};

		TOGGLE_BIT(ch->pcdata->trust, TRUST_CLAN);
		if (IS_SET(ch->pcdata->trust, TRUST_CLAN)) {
			REMOVE_BIT(ch->pcdata->trust, TRUST_ALL);
			char_puts("You now trust your clan with questionable spells.\n", ch);
		}
		else 
			char_puts("You no longer trust your clan with questionable spells.\n", ch);
		return;
	}

	if (!str_cmp(arg, "group")) {
		TOGGLE_BIT(ch->pcdata->trust, TRUST_GROUP);
		if (IS_SET(ch->pcdata->trust, TRUST_GROUP)) {
			REMOVE_BIT(ch->pcdata->trust, TRUST_ALL);
			char_puts("You allow your group to cast questionable spells on you.\n", ch);
		}
		else
			char_puts("You no longer trust your group with questionable spells.\n", ch);
		return;
	}

	if (!str_cmp(arg, "all")) {
		ch->pcdata->trust = TRUST_ALL;
		char_puts("You allow everyone to cast questionable spells on you.\n", ch);
		return;
	}

	if (!str_cmp(arg, "none")) {
		ch->pcdata->trust = 0;
		char_puts("You do not allow anyone to cast questionable spells on you.\n", ch);
		return;
	}

	char_puts("Syntax: trust {{ group | clan | all | none }\n", ch);
}

DO_FUN(do_wanted)
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
		char_puts("Toggle wanted whom?\n", ch);
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

	if (IS_IMMORTAL(victim) && ch->level < victim->level) {
		act("You do not have the power to arrest $N.",
		    ch, NULL, victim, TO_CHAR);
		return;
	}

	if (victim == ch) {
		char_puts( "You cannot do that to yourself.\n", ch);
		return;
	}

	TOGGLE_BIT(victim->plr_flags, PLR_WANTED);
	if (IS_SET(victim->plr_flags, PLR_WANTED)) {
		act("$n is now WANTED!!!", victim, NULL, ch, TO_NOTVICT);
		act("$n is now WANTED!!!", victim, NULL, ch, TO_VICT);
		char_puts("You are now WANTED!!!\n", victim);
	}
	else {
		act("$n is no longer wanted.", victim, NULL, ch, TO_NOTVICT);
		act("$n is no longer wanted.", victim, NULL, ch, TO_VICT);
		char_puts("You are no longer wanted.\n", victim);
	}
}

/*-----------------------------------------------------------------------------
 * toggle bit stuff
 */
typedef struct toggle_t toggle_t;

struct toggle_t {
	const char *name;	/* flag name				*/
	const char *desc;	/* toggle description			*/
	FLAG *f;		/* flag table				*/
	flag32_t bit;		/* flag bit				*/
	const char *msg_on;	/* msg to print when flag toggled on	*/
	const char *msg_off;	/* ---//--- off				*/
};

static toggle_t *toggle_lookup(const char *name);
static void toggle_print(CHAR_DATA *ch, toggle_t *t);
static flag32_t* toggle_bits(CHAR_DATA *ch, toggle_t *t);

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

	{ "notelnet",		"no telnet parser",
	  comm_flags,	COMM_NOTELNET,
	  "Telnet parser is OFF.",
	  "Telnet parser is ON.",
	},

	{ "noiac",		"no IACs in output",
	  comm_flags,	COMM_NOIAC,
	  "IACs will not be sent to you anymore.",
	  "Text will be sent to you unmodified.",
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

	{ "quiet edit",		"quiet mode in string editor",
	  comm_flags,	COMM_QUIET_EDITOR,
	  "$t set.",
	  "$t removed."
	},

	{ NULL }
};

DO_FUN(do_toggle)
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
		flag32_t* bits;

		if ((t = toggle_lookup(arg)) == NULL
		||  (bits = toggle_bits(ch, t)) == NULL) {
			char_printf(ch, "%s: no such toggle.\n", arg);
			continue;
		}

		TOGGLE_BIT(*bits, t->bit);
		act_puts(IS_SET(*bits, t->bit) ? t->msg_on : t->msg_off,
			 ch, t->desc, NULL, TO_CHAR | ACT_TRANS, POS_DEAD);
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
	flag32_t *bits;

	if ((bits = toggle_bits(ch, t)) < 0)
		return;

	snprintf(buf, sizeof(buf), "  %-11.11s - %-3.3s ($t)",
		 t->name, IS_SET(*bits, t->bit) ? "ON" : "OFF");
	act_puts(buf, ch, t->desc, NULL, TO_CHAR | ACT_TRANS, POS_DEAD);
}

static flag32_t* toggle_bits(CHAR_DATA *ch, toggle_t *t)
{
	if (t->f == comm_flags)
		return &ch->comm;
	return NULL;
}

