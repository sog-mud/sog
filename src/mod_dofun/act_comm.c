/*
 * $Id: act_comm.c,v 1.51 1998-06-24 02:42:22 efdi Exp $
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

#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>
#include "merc.h"
#include "interp.h"
#include "act_wiz.h"
#include "recycle.h"
#include "comm.h"
#include "db.h"
#include "tables.h"
#include "interp.h"
#include "resource.h"
#include "act_comm.h"
#include "quest.h"
#include "log.h"

/* command procedures needed */
DECLARE_DO_FUN(do_quit	);
DECLARE_DO_FUN(do_quit_count);

void do_quit_org	args((CHAR_DATA *ch, char *argument, bool Count));
bool proper_order	args((CHAR_DATA *ch, char *argument));
char *translate(CHAR_DATA *ch, CHAR_DATA *victim, char *argument);
int lang_lookup		args((const char *name));

void do_music(CHAR_DATA *ch, char *argument)
{
	DESCRIPTOR_DATA *d;
	char buf[MAX_INPUT_LENGTH];
	char trans[MAX_STRING_LENGTH];

	if (IS_NPC(ch))
		return;

	if (argument[0] == '\0') {
		/* send_to_char("Music what?.\n\r",ch);
		   return;
		*/
		if (IS_SET(ch->comm, COMM_NOMUSIC)) {
			REMOVE_BIT(ch->comm, COMM_NOMUSIC);
			send_to_char("You will now hear music.\n\r", ch);
			return;
		} else {
			SET_BIT(ch->comm, COMM_NOMUSIC);
			send_to_char("You will no longer hear music.\n\r",ch);
			return;
		}
	}

	strcpy(buf,argument);

	if (!is_affected(ch, gsn_deafen))
		act_puts("You music '{W$T{x'",
		       ch, NULL, buf, TO_CHAR, POS_DEAD);

	for (d = descriptor_list; d != NULL; d = d->next) {
		CHAR_DATA *victim;

		victim = d->original ? d->original : d->character;

		if (d->connected == CON_PLAYING && d->character != ch
		&&  !IS_SET(d->character->comm, COMM_NOMUSIC)) {
			strcpy(trans, translate(ch,d->character,buf));
			act_puts("$n music '{W$t{x'",
		        	 ch, trans, d->character, TO_VICT, POS_DEAD);
		}
	}

	return;
}

void do_gossip(CHAR_DATA *ch, char *argument)
{
	DESCRIPTOR_DATA *d;
	char buf[MAX_INPUT_LENGTH];
	char trans[MAX_STRING_LENGTH];

	if (IS_NPC(ch))
		return;

	if (argument[0] == '\0')
	{
		/* send_to_char("Gossip what?.\n\r",ch);
		   return;
		*/
		if (IS_SET(ch->comm, COMM_NOGOSSIP)) {
			REMOVE_BIT(ch->comm, COMM_NOGOSSIP);
			send_to_char("You will now hear gossips.\n\r", ch);
			return;
		} else {
			SET_BIT(ch->comm, COMM_NOGOSSIP);
			send_to_char("You will no longer hear gossips.\n\r",ch);
			return;
		}
	}

	strcpy(buf,argument);

	if (!is_affected(ch, gsn_deafen))
		act_puts("You gossip '{Y$T{x'",
		       ch, NULL, buf, TO_CHAR,POS_DEAD);

	for (d = descriptor_list; d != NULL; d = d->next) {
		CHAR_DATA *victim;

		victim = d->original ? d->original : d->character;

		if (d->connected == CON_PLAYING && d->character != ch
		&&  !IS_SET(d->character->comm, COMM_NOGOSSIP)) {
			strcpy(trans, translate(ch,d->character,buf));
			act_puts("$n gossips '{Y$t{x'",
		        	 ch, trans, d->character, TO_VICT, POS_DEAD);
		}
	}

	return;
}

void do_ilang(CHAR_DATA *ch, char *argument)
{
	int i;

	if(*argument == '\0') {
		if (ch->i_lang >= nilang) {
			char_puts(msg(COMM_INTERFACE_LANGUAGE_UNDEFINED, ch),
				  ch);
			return;
		}
		char_printf(ch, msg(COMM_SHOW_LANGUAGE, ch),
			    ilang_names[ch->i_lang]);
		return;
	}

	for (i = 0; ilang_names[i] != NULL; i++)
		if (!str_prefix(argument, ilang_names[i])) {
			ch->i_lang = i;
			char_printf(ch, msg(COMM_SHOW_LANGUAGE, ch),
				    ilang_names[i]);
			return;
		}

	char_puts(msg(COMM_LANGUAGE_USAGE_PRE, ch), ch);
	for (i = 0; ilang_names[i] != NULL; i++)
		char_printf(ch, "%s%s", i == 0 ? "" : " | ", ilang_names[i]);
	char_puts(msg(COMM_LANGUAGE_USAGE_POST, ch), ch);
}

/* RT code to delete yourself */

void do_delet(CHAR_DATA *ch, char *argument)
{
	send_to_char("You must type the full command to delete yourself.\n\r",
		     ch);
}

void do_delete(CHAR_DATA *ch, char *argument)
{
	char strsave[MAX_INPUT_LENGTH];

	if (IS_NPC(ch))
		return;
	
	if (ch->pcdata->confirm_delete) {
		if (argument[0] != '\0') {
			send_to_char("Delete status removed.\n\r",ch);
			ch->pcdata->confirm_delete = FALSE;
			return;
		}

		snprintf(strsave, sizeof(strsave), "%s%s", PLAYER_DIR, capitalize(ch->name));
		wiznet("$N turns $Mself into line noise.",ch,NULL,0,0,0);
		ch->last_fight_time = -1;
		do_quit_count(ch,"");
		unlink(strsave);
		return;
	}

	if (argument[0] != '\0') {
		send_to_char("Just type delete. No argument.\n\r",ch);
		return;
	}

	send_to_char("Type delete again to confirm this command.\n\r"
		     "WARNING: this command is irreversible.\n\r"
		     "Typing delete with an argument will undo delete status.\n\r",
		     ch);
	ch->pcdata->confirm_delete = TRUE;
	wiznet("$N is contemplating deletion.", ch, NULL, 0, 0, get_trust(ch));
}
		

/* RT code to display channel status */

void do_channels(CHAR_DATA *ch, char *argument)
{
	/* lists all channels and their status */
	send_to_char("   channel     status\n\r",ch);
	send_to_char("---------------------\n\r",ch);
	
	send_to_char("music         ", ch);
	if (!IS_SET(ch->comm,COMM_NOMUSIC))
		 send_to_char("ON\n\r",ch);
	else
		 send_to_char("OFF\n\r",ch);

	send_to_char("gossip         ", ch);
	if (!IS_SET(ch->comm,COMM_NOGOSSIP))
		 send_to_char("ON\n\r",ch);
	else
		 send_to_char("OFF\n\r",ch);

	send_to_char("auction        ", ch);
	if (!IS_SET(ch->comm,COMM_NOAUCTION))
		 send_to_char("ON\n\r",ch);
	else
		 send_to_char("OFF\n\r",ch);

	if (IS_IMMORTAL(ch)) {
		send_to_char("god channel    ",ch);
		if(!IS_SET(ch->comm,COMM_NOWIZ))
			send_to_char("ON\n\r",ch);
		else
			send_to_char("OFF\n\r",ch);
	}

	send_to_char("tells          ",ch);
	if (!IS_SET(ch->comm,COMM_DEAF))
		send_to_char("ON\n\r",ch);
	else
		send_to_char("OFF\n\r",ch);

	send_to_char("quiet mode     ",ch);
	if (IS_SET(ch->comm,COMM_QUIET))
		 send_to_char("ON\n\r",ch);
	else
		 send_to_char("OFF\n\r",ch);

	if (IS_SET(ch->comm,COMM_SNOOP_PROOF))
		send_to_char("You are immune to snooping.\n\r",ch);
	
	if (ch->lines != PAGELEN) {
		if (ch->lines)
			char_printf(ch, "You display %d lines of scroll.\n\r",
				    ch->lines+2);
		else
			send_to_char("Scroll buffering is off.\n\r",ch);
	}

	if (IS_SET(ch->comm,COMM_NOTELL))
		 send_to_char("You cannot use tell.\n\r",ch);
	
	if (IS_SET(ch->comm,COMM_NOCHANNELS))
		send_to_char("You cannot use channels.\n\r",ch);

	if (IS_SET(ch->comm,COMM_NOEMOTE))
		 send_to_char("You cannot show emotions.\n\r",ch);
}

void garble(char *garbled, char *speech)
{
	static char not_garbled[] = "?!()[]{},.:;'\" ";
	for (; *speech; speech++, garbled++) {
		if (strchr(not_garbled, *speech))
		 *garbled = *speech;
		else
		 *garbled = number_range(' ', 254);
	}
 *garbled = '\0';
}


/* RT deaf blocks out all shouts */

void do_deaf(CHAR_DATA *ch, char *argument)
{
	
	if (IS_SET(ch->comm,COMM_DEAF))
	{
		send_to_char("You can now hear tells again.\n\r",ch);
		REMOVE_BIT(ch->comm,COMM_DEAF);
	}
	else 
	{
		send_to_char("From now on, you won't hear tells.\n\r",ch);
		SET_BIT(ch->comm,COMM_DEAF);
	}
}

/* RT quiet blocks out all communication */

void do_quiet (CHAR_DATA *ch, char * argument)
{
	if (IS_SET(ch->comm,COMM_QUIET)) {
		 send_to_char("Quiet mode removed.\n\r",ch);
		 REMOVE_BIT(ch->comm,COMM_QUIET);
	}
	else {
		send_to_char("From now on, you will only hear says and emotes.\n\r", ch);
		SET_BIT(ch->comm,COMM_QUIET);
	}
}

void do_replay (CHAR_DATA *ch, char *argument)
{
	if (IS_NPC(ch)) {
		send_to_char("You can't replay.\n\r",ch);
		return;
	}

	page_to_char(buf_string(ch->pcdata->buffer), ch);
	clear_buf(ch->pcdata->buffer);
}

void do_immtalk(CHAR_DATA *ch, char *argument)
{
	DESCRIPTOR_DATA *d;

	if (argument[0] == '\0') {
		 if (IS_SET(ch->comm,COMM_NOWIZ)) {
			send_to_char("Immortal channel is now ON\n\r",ch);
			REMOVE_BIT(ch->comm,COMM_NOWIZ);
		 }
		 else {
			send_to_char("Immortal channel is now OFF\n\r",ch);
			SET_BIT(ch->comm,COMM_NOWIZ);
		 } 
		 return;
	}

	REMOVE_BIT(ch->comm,COMM_NOWIZ);

	if (!is_affected(ch, gsn_deafen))
		act_puts("{W$n{x: {C$t{x",ch,argument,NULL,TO_CHAR,POS_DEAD);
	for (d = descriptor_list; d != NULL; d = d->next) {
		if (d->connected == CON_PLAYING && IS_IMMORTAL(d->character)
		&&  !IS_SET(d->character->comm,COMM_NOWIZ))
			act_puts("{W$n{x: {C$t{x", ch,argument, d->character,
				 TO_VICT, POS_DEAD);
	}
}



void do_say(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *room_char;
	OBJ_DATA *char_obj;
	CHAR_DATA *vch;
	char buf[MAX_STRING_LENGTH];
	char trans[MAX_STRING_LENGTH];

	if (argument[0] == '\0') {
		send_to_char("Say what?\n\r", ch);
		return;
	}
	if (ch->in_room == NULL)  {
		 send_to_char("But, you are not in a room!\n\r", ch);
		 return;
	}

	if (is_affected(ch, gsn_garble))
		garble(buf, argument);
	else
		strcpy(buf, argument);

	for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room) {
		if (!is_affected(vch, gsn_deafen)) {
		 strcpy(trans, translate(ch, vch, buf));
			act_nprintf(ch, trans, vch, TO_VICT, POS_RESTING, 
					COMM_N_SAYS);
		}
	}

	if (!is_affected(ch, gsn_deafen))
		act_nprintf(ch, NULL, buf, TO_CHAR, POS_RESTING, COMM_YOU_SAY);

	for (room_char = ch->in_room->people; room_char != NULL;
		 room_char = room_char->next_in_room)
	{
		 if (IS_SET(room_char->progtypes,MPROG_SPEECH) && room_char != ch) 
		(room_char->pIndexData->mprogs->speech_prog) (room_char,ch,buf);
	}

	for (char_obj = ch->carrying; char_obj != NULL;
		  char_obj = char_obj->next_content)
	{
		 if (IS_SET(char_obj->progtypes,OPROG_SPEECH))
		(char_obj->pIndexData->oprogs->speech_prog) (char_obj,ch,buf);
	}

	for (char_obj = ch->in_room->contents; char_obj != NULL;
		  char_obj = char_obj->next_content) {
		 if (IS_SET(char_obj->progtypes,OPROG_SPEECH))
		(char_obj->pIndexData->oprogs->speech_prog) (char_obj,ch,buf);
	}

	return;
}



void do_shout(CHAR_DATA *ch, char *argument)
{
	DESCRIPTOR_DATA *d;
	char buf[MAX_INPUT_LENGTH];
	char trans[MAX_STRING_LENGTH];

	if (IS_NPC(ch))
		return;

	if (argument[0] == '\0') {
		 send_to_char("Shout what?.\n\r",ch);
		 return;
	}

	WAIT_STATE(ch, 12);

	if (is_affected(ch,gsn_garble))
		 garble(buf,argument);
	else
		 strcpy(buf,argument);

	if (!is_affected(ch, gsn_deafen))
		act_puts("You shout '{R$T{x'",
		       ch, NULL, buf, TO_CHAR,POS_DEAD);

	for (d = descriptor_list; d != NULL; d = d->next)
	{
	CHAR_DATA *victim;

	victim = d->original ? d->original : d->character;

	if (d->connected == CON_PLAYING &&
		 d->character != ch &&
		 d->character->in_room->area == ch->in_room->area &&
		     !is_affected(d->character, gsn_deafen))
	{
		strcpy(trans, translate(ch,d->character,buf));
		    act_puts("{W$n{x shouts '{R$t{x'",
		              ch,trans,d->character,TO_VICT,POS_DEAD);
	}
	}

	return;
}


void do_tell_raw(CHAR_DATA *ch, char *msg, CHAR_DATA *victim)
{
	char buf[MAX_STRING_LENGTH];

	if (IS_SET(ch->comm, COMM_NOTELL)) {
		send_to_char(msg(COMM_YOUR_MESSAGE_DIDNT_GET_THROUGH, ch), ch);
		return;
	}

	if (victim == NULL 
	|| (IS_NPC(victim) && victim->in_room != ch->in_room)) {
		send_to_char(msg(THEY_ARENT_HERE, ch), ch);
		return;
	}

	if (is_affected(ch, gsn_garble))
		garble(buf, msg);
	else
		strcpy(buf, msg);

	if (victim->desc == NULL && !IS_NPC(victim)) {
		act("$N seems to have misplaced $S link...try again later.",
		    ch, NULL, victim, TO_CHAR);
		snprintf(buf, sizeof(buf), "%s tells you '%s'\n\r", PERS(ch,victim),msg);
		buf[0] = UPPER(buf[0]);
		add_buf(victim->pcdata->buffer,buf);
		return;
	}

/*	if (!IS_IMMORTAL(ch) && !IS_AWAKE(victim)) {
		act("$E can't hear you.", ch, 0, victim, TO_CHAR);
		return;
	}
*/
	if ((IS_SET(victim->comm,COMM_QUIET) || IS_SET(victim->comm,COMM_DEAF))
	&&  !IS_IMMORTAL(ch) && !IS_IMMORTAL(victim)) {
		act_puts("$E is not receiving tells.", ch, 0, victim,
			 TO_CHAR, POS_DEAD);
		return;
	}

/*	if (!IS_IMMORTAL(victim) && !IS_AWAKE(ch)) {
		send_to_char("In your dreams, or what?\n\r", ch);
		return;
	}
*/
	if (!is_affected(ch, gsn_deafen))
		act_nprintf(ch, buf, victim, TO_CHAR, 
				POS_SLEEPING, COMM_YOU_TELL);
	act_nprintf(ch, buf, victim, TO_VICT, POS_SLEEPING, COMM_TELLS_YOU);

	victim->reply = ch;

	return;
}


void do_tell(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];

	argument = one_argument(argument, arg);
	if (arg[0] == '\0' || argument[0] == '\0') {
		send_to_char("Tell whom what?\n\r", ch);
		return;
	}

	do_tell_raw(ch, argument, get_char_world(ch, arg));
}


void do_reply(CHAR_DATA *ch, char *argument)
{
	do_tell_raw(ch, argument, ch->reply);
}

void do_yell(CHAR_DATA *ch, char *argument)
{
	DESCRIPTOR_DATA *d;
	char buf[MAX_INPUT_LENGTH];
	char trans[MAX_STRING_LENGTH];

	if (argument[0] == '\0')
	{
	send_to_char("Yell what?\n\r", ch);
	return;
	}

	if (is_affected(ch,gsn_garble))
		 garble(buf,argument);
	else
		 strcpy(buf,argument);

	if (!is_affected(ch, gsn_deafen))
		act_puts("You yell '{M$t{x'",
		       ch,buf,NULL,TO_CHAR,POS_DEAD);

	for (d = descriptor_list; d != NULL; d = d->next)
	{
	if (d->connected == CON_PLAYING
	&&   d->character != ch
	&&   d->character->in_room != NULL
	&&   d->character->in_room->area == ch->in_room->area 
		&&   !is_affected(d->character, gsn_deafen)) {
		strcpy(trans, translate(ch, d->character, buf));
		    act_puts("$n yells '{M$t{x'",
		              ch, trans, d->character, TO_VICT, POS_DEAD);
	}
	}

	return;
}


void do_emote(CHAR_DATA *ch, char *argument)
{
	char buf[MAX_INPUT_LENGTH];

	if (!IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE)) {
		send_to_char("You can't show your emotions.\n\r", ch);
		return;
	}
	
	if (argument[0] == '\0') {
		send_to_char("Emote what?\n\r", ch);
		return;
	}
	
	if (is_affected(ch,gsn_garble))
		 garble(buf,argument);
	else
		 strcpy(buf,argument);

	act("$n $T", ch, NULL, buf, TO_ROOM);
	act("$n $T", ch, NULL, buf, TO_CHAR);
	return;
}


void do_pmote(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *vch;
	char *letter,*name;
	char last[MAX_INPUT_LENGTH], temp[MAX_STRING_LENGTH];
	int matches = 0;

	if (!IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE)) {
		send_to_char("You can't show your emotions.\n\r", ch);
		return;
	}
	
	if (argument[0] == '\0') {
		send_to_char("Emote what?\n\r", ch);
		return;
	}
	
	act("$n $t", ch, argument, NULL, TO_CHAR);

	for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
	{
	if (vch->desc == NULL || vch == ch)
		continue;

	if ((letter = strstr(argument,vch->name)) == NULL)
	{
		act("$N $t",vch,argument,ch,TO_CHAR);
		continue;
	}

	strcpy(temp,argument);
	temp[strlen(argument) - strlen(letter)] = '\0';
		last[0] = '\0';
		name = vch->name;
	
	for (; *letter != '\0'; letter++)
	{ 
		if (*letter == '\'' && matches == strlen(vch->name))
		{
		strcat(temp,"r");
		continue;
		}

		if (*letter == 's' && matches == strlen(vch->name))
		{
		matches = 0;
		continue;
		}
		
		    if (matches == strlen(vch->name))
		{
		matches = 0;
		}

		if (*letter == *name)
		{
		matches++;
		name++;
		if (matches == strlen(vch->name))
		{
		    strcat(temp,"you");
		    last[0] = '\0';
		    name = vch->name;
		    continue;
		}
		strncat(last,letter,1);
		continue;
		}

		matches = 0;
		strcat(temp,last);
		strncat(temp,letter,1);
		last[0] = '\0';
		name = vch->name;
	}

	act("$N $t",vch,temp,ch,TO_CHAR);
	}
	
	return;
}


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



void do_pose(CHAR_DATA *ch, char *argument)
{
	int level;
	int pose;

	if (IS_NPC(ch))
		return;

	level = UMIN(ch->level, sizeof(pose_table) / sizeof(pose_table[0]) - 1);
	pose  = number_range(0, level);

	act(pose_table[pose].message[2*ch->class+0], ch, NULL, NULL, TO_CHAR);
	act(pose_table[pose].message[2*ch->class+1], ch, NULL, NULL, TO_ROOM);

	return;
}



void do_bug(CHAR_DATA *ch, char *argument)
{
	append_file(ch, BUG_FILE, argument);
	send_to_char("Bug logged.\n\r", ch);
	return;
}

void do_typo(CHAR_DATA *ch, char *argument)
{
	append_file(ch, TYPO_FILE, argument);
	send_to_char("Typo logged.\n\r", ch);
	return;
}

void do_rent(CHAR_DATA *ch, char *argument)
{
	send_to_char("There is no rent here.  Just save and quit.\n\r", ch);
	return;
}


void do_qui(CHAR_DATA *ch, char *argument)
{
	send_to_char("If you want to QUIT, you have to spell it out.\n\r", ch);
	return;
}


void do_quit(CHAR_DATA *ch, char *argument)
{
	do_quit_org(ch, argument, FALSE);
	return;
}

void do_quit_count(CHAR_DATA *ch, char *argument)
{
	do_quit_org(ch, argument, TRUE);
	return;
}

void do_quit_org(CHAR_DATA *ch, char *argument, bool Count)
{
	DESCRIPTOR_DATA *d, *d_next;
	CHAR_DATA *vch;
	OBJ_DATA *obj,*obj_next;
	int id;

	if (IS_NPC(ch))
		return;

	if (ch->position == POS_FIGHTING) {
		send_to_char("No way! You are fighting.\n\r", ch);
		return;
	}

	if (ch->position  < POS_STUNNED ) {
		send_to_char("You're not DEAD yet.\n\r", ch);
		return;
	}

	if (ch->last_fight_time != -1 && !IS_IMMORTAL(ch)
	&&  (current_time - ch->last_fight_time) < FIGHT_DELAY_TIME) {
		send_to_char("Your adrenalin is gushing! You can't quit yet.\n\r",ch);
		return;
	}

	if (IS_AFFECTED(ch, AFF_CHARM)) {
		send_to_char("You don't want to leave your master.\n\r", ch);
		return;
	}

	if (IS_SET(ch->act, PLR_NO_EXP)) {
		send_to_char("You don't want to lose your spirit.\n\r", ch);
		return;
	}

	if (auction->item != NULL
	&&  ((ch == auction->buyer) || (ch == auction->seller))) {
		send_to_char ("Wait till you have sold/bought the item on auction.\n\r",ch);
		return;
	}

	if (!IS_IMMORTAL(ch)
	&&  ch->in_room && IS_RAFFECTED(ch->in_room, AFF_ROOM_ESPIRIT)) {
		send_to_char("Evil spirits in the area prevents you from leaving.\n\r", ch);
		return;
	}

	if (!IS_IMMORTAL(ch)
	&&  ch->clan != CLAN_INVADER && is_affected(ch, gsn_evil_spirit)) {
		send_to_char("Evil spirits in you prevents you from leaving.\n\r", ch);
		return;
	}

	send_to_char("Alas, all good things must come to an end.\n\r", ch);
	char_puts("You hit reality hard. Reality truth does unspeakable things to you.\n\r", ch);
	act_puts("{W$n{x has left the game.", ch, NULL, NULL, TO_ROOM ,POS_DEAD);
	log_printf("%s has quit.", ch->name);
	wiznet("{W$N{x rejoins the real world.",ch,NULL,WIZ_LOGINS,0,get_trust(ch));

	/*
	 * remove quest objs for this char, drop quest objs for other chars
	 */
	for (obj = object_list; obj != NULL; obj = obj_next) {
		obj_next = obj->next;
		if (obj->pIndexData->vnum >= QUEST_OBJ_FIRST
		&&  obj->pIndexData->vnum <= QUEST_OBJ_LAST)
			if (obj->extra_descr == NULL ||
			    strstr(obj->extra_descr->description,
							ch->name) != NULL)
				extract_obj(obj);
			else if (obj->carried_by == ch) {
				obj_from_char(obj);
				obj_to_room(obj,ch->in_room);
			}
	}


	for (vch = char_list; vch != NULL; vch = vch->next) {
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
	}

	if (ch->guarded_by != NULL) {
		ch->guarded_by->guarding = NULL;
		ch->guarded_by = NULL;
	}


	/*
	 * After extract_char the ch is no longer valid!
	 */
	save_char_obj(ch);
	id = ch->id;
	d = ch->desc;
	if (Count)
		 extract_char(ch, TRUE);
	else
		 extract_char_nocount(ch, TRUE);

	if (d != NULL)
		close_socket(d);

	/* toast evil cheating bastards    */
	for (d = descriptor_list; d != NULL; d = d_next) {
		CHAR_DATA *tch;

		d_next = d->next;
		tch = d->original ? d->original : d->character;
		if (tch && tch->id == id) {
			extract_char_nocount(tch, TRUE);
			close_socket(d);
		} 
	}
}



void do_save(CHAR_DATA *ch, char *argument)
{
	if (IS_NPC(ch))
	return;

	if (ch->level < 2) {
		send_to_char("You must be at least level 2 for saving.\n\r",ch);
		return;
	}
	save_char_obj(ch);
	WAIT_STATE(ch, PULSE_VIOLENCE);
	return;
}

void do_follow(CHAR_DATA *ch, char *argument)
{
/* RT changed to allow unlimited following and follow the NOFOLLOW rules */
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	one_argument(argument, arg);

	if (arg[0] == '\0') {
		send_to_char("Follow whom?\n\r", ch);
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL) {
		char_nputs(THEY_ARENT_HERE, ch);
		return;
	}

	if (IS_AFFECTED(ch, AFF_CHARM) && ch->master != NULL) {
		act("But you'd rather follow $N!", ch, NULL,ch->master,TO_CHAR);
		return;
	}

	if (victim == ch) {
		if (ch->master == NULL) {
			send_to_char("You already follow yourself.\n\r", ch);
			return;
		}
		stop_follower(ch);
		return;
	}

	if (!IS_NPC(victim) && IS_SET(victim->act,PLR_NOFOLLOW) && !IS_IMMORTAL(ch)) {
		act("$N doesn't seem to want any followers.\n\r",
		    ch, NULL, victim, TO_CHAR);
			return;
	}

	REMOVE_BIT(ch->act,PLR_NOFOLLOW);
	
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
		affect_strip(ch, gsn_charm_person);
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


void do_order(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH],arg2[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	CHAR_DATA *och;
	CHAR_DATA *och_next;
	bool found;
	bool fAll;

	argument = one_argument(argument, arg);
	one_argument(argument,arg2);

	if (!str_cmp(arg2,"delete")) {
		send_to_char("That will NOT be done.\n\r",ch);
		return;
	}

	if (arg[0] == '\0' || argument[0] == '\0') {
		send_to_char("Order whom to do what?\n\r", ch);
		return;
	}

	if (IS_AFFECTED(ch, AFF_CHARM)) {
		char_puts("You feel like taking, not giving, orders.\n\r", ch);
		return;
	}

	if (!str_cmp(arg, "all")) {
		fAll   = TRUE;
		victim = NULL;
	} else {
		fAll   = FALSE;
		if ((victim = get_char_room(ch, arg)) == NULL) {
			char_nputs(THEY_ARENT_HERE, ch);
			return;
		}

		if (victim == ch) {
			send_to_char("Aye aye, right away!\n\r", ch);
			return;
		}

		if (!IS_AFFECTED(victim, AFF_CHARM) || victim->master != ch 
		||(IS_IMMORTAL(victim) && get_trust(victim) >= get_trust(ch))) {
			send_to_char("Do it yourself!\n\r", ch);
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
			if (!proper_order(och, argument))
				continue;
			act_nprintf(ch, NULL, och, TO_VICT, POS_RESTING,
				       COMM_ORDERS_YOU_TO, argument);
			interpret(och, argument, TRUE);
		}
	}

	if (found) {
		WAIT_STATE(ch,PULSE_VIOLENCE);
		char_nputs(OK, ch);
	} else
		send_to_char("You have no followers here.\n\r", ch);
	return;
}

bool proper_order(CHAR_DATA *ch, char *argument)
{
	char command[MAX_INPUT_LENGTH];
	bool found;
	int trust, cmd_num;
	DO_FUN *cmd;

	if (!IS_NPC(ch))
		return TRUE;

	one_argument(argument, command);
	found = FALSE;

	trust = get_trust(ch);

	for (cmd_num = 0; cmd_table[cmd_num].name[0] != '\0'; cmd_num++)
		if (command[0] == cmd_table[cmd_num].name[0]
		&& !str_prefix(command, cmd_table[cmd_num].name)
		&& cmd_table[cmd_num].level <= trust) {
			found = TRUE;
			break;
		}

	if (!found)
		return TRUE;

	cmd = cmd_table[cmd_num].do_fun;

	if (((cmd == do_bash) || (cmd == do_dirt) || (cmd == do_kick)
	|| (cmd == do_murder) || (cmd == do_trip)) 
	&& ch->fighting == NULL) 
		return FALSE;

	if ((cmd == do_assassinate) || (cmd == do_ambush)
	|| (cmd == do_blackjack) || (cmd == do_cleave) || (cmd == do_kill)
	|| (cmd == do_murder) || (cmd == do_recall) || (cmd == do_strangle)
	|| (cmd == do_vtouch))
		return FALSE;

	if (cmd == do_close || cmd == do_lock || cmd == do_open
	||  cmd == do_unlock )  {
		if (race_table[RACE(ch)].pc_race)
			return TRUE;
		if (RACE(ch) != 34	/* doll */
		&&  RACE(ch) != 37	/* goblin */
		&&  RACE(ch) != 38	/* hobgoblin */
		&&  RACE(ch) != 39	/* kobolt */
		&&  RACE(ch) != 40	/* lizard */
		&&  RACE(ch) != 41	/* modron */
		&&  RACE(ch) != 42)	/* orc */
			return FALSE;
		else 
			return TRUE;
	}

	if ((cmd == do_backstab) || (cmd == do_hide) || (cmd == do_pick) 
	|| (cmd == do_sneak)) {
		if (IS_SET(ch->act, ACT_THIEF))
			return TRUE;
		else
			return FALSE;
	}

	return TRUE;
}


CHAR_DATA* leader_lookup(CHAR_DATA* ch)
{
	CHAR_DATA* res;
	for (res = ch; res->leader != NULL; res = res->leader)
		;
	return res;
}



void do_group(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	one_argument(argument, arg);

	if (arg[0] == '\0') {
		CHAR_DATA *gch;

		char_printf(ch, "%s's group:\n\r", PERS(leader_lookup(ch), ch));

		for (gch = char_list; gch != NULL; gch = gch->next) {
			if (is_same_group(gch, ch))
				char_printf(ch, "[%2d %s] %-16s %d/%d hp %d/%d mana %d/%d mv   %5d xp\n\r",
					    gch->level,
		    IS_NPC(gch) ? "Mob" : class_table[gch->class].who_name,
		    capitalize(PERS(gch, ch)),
		    gch->hit,   gch->max_hit,
		    gch->mana,  gch->max_mana,
		    gch->move,  gch->max_move,
		    gch->exp);
		}
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL) {
		send_to_char("They aren't here.\n\r", ch);
		return;
	}

	if (victim == ch) {
		char_puts("Huh? Grouping with yourself?!\n\r", ch);
		return;
	}

	if (ch->master != NULL || ch->leader != NULL) {
		send_to_char("But you are following someone else!\n\r", ch);
		return;
	}

	if (victim->master != ch && ch != victim) {
		act("$N isn't following you.", ch, NULL, victim, TO_CHAR);
		return;
	}
	
	if (IS_AFFECTED(victim, AFF_CHARM)) {
		send_to_char("You can't remove charmed mobs from your group.\n\r",ch);
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
void do_split(CHAR_DATA *ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	char arg1[MAX_INPUT_LENGTH],arg2[MAX_INPUT_LENGTH];
	CHAR_DATA *gch;
	int members;
	int amount_gold = 0, amount_silver = 0;
	int share_gold, share_silver;
	int extra_gold, extra_silver;

	argument = one_argument(argument, arg1);
		   one_argument(argument, arg2);

	if (arg1[0] == '\0')
	{
	send_to_char("Split how much?\n\r", ch);
	return;
	}
	
	amount_silver = atoi(arg1);

	if (arg2[0] != '\0')
	amount_gold = atoi(arg2);

	if (amount_gold < 0 || amount_silver < 0)
	{
	send_to_char("Your group wouldn't like that.\n\r", ch);
	return;
	}

	if (amount_gold == 0 && amount_silver == 0)
	{
	send_to_char("You hand out zero coins, but no one notices.\n\r", ch);
	return;
	}

	if (ch->gold <  amount_gold || ch->silver < amount_silver)
	{
	send_to_char("You don't have that much to split.\n\r", ch);
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
	send_to_char("Just keep it all.\n\r", ch);
	return;
	}
		
	share_silver = amount_silver / members;
	extra_silver = amount_silver % members;

	share_gold   = amount_gold / members;
	extra_gold   = amount_gold % members;

	if (share_gold == 0 && share_silver == 0)
	{
	send_to_char("Don't even bother, cheapskate.\n\r", ch);
	return;
	}

	ch->silver	-= amount_silver;
	ch->silver	+= share_silver + extra_silver;
	ch->gold 	-= amount_gold;
	ch->gold 	+= share_gold + extra_gold;

	if (share_silver > 0)
	char_printf(ch,
		"You split %d silver coins. Your share is %d silver.\n\r",
		    amount_silver,share_silver + extra_silver);

	if (share_gold > 0)
	char_printf(ch,
		"You split %d gold coins. Your share is %d gold.\n\r",
		 amount_gold,share_gold + extra_gold);

	if (share_gold == 0)
	snprintf(buf, sizeof(buf), "$n splits %d silver coins. Your share is %d silver.",
		amount_silver,share_silver);
	else if (share_silver == 0)
	snprintf(buf, sizeof(buf), "$n splits %d gold coins. Your share is %d gold.",
		amount_gold,share_gold);
	else
	snprintf(buf, sizeof(buf),
"$n splits %d silver and %d gold coins, giving you %d silver and %d gold.\n\r",
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



void do_gtell(CHAR_DATA *ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *gch;
	int i;

	if (argument[0] == '\0')
	{
	send_to_char("Tell your group what?\n\r", ch);
	return;
	}

	if (IS_SET(ch->comm, COMM_NOTELL))
	{
	send_to_char("Your message didn't get through!\n\r", ch);
	return;
	}

	if (is_affected(ch,gsn_garble))
		 garble(buf,argument);
	else
		 strcpy(buf,argument);

	/*
	 * Note use of send_to_char, so gtell works on sleepers.
	 */

	for (i = 0, gch = char_list; gch != NULL; gch = gch->next)
	{
		 if (is_same_group(gch, ch) && !is_affected(gch, gsn_deafen))
		{
		  act_puts("{W$n{x tells the group '{G$t{x'",
		          ch,buf,gch,TO_VICT,POS_DEAD);
		  i++;
		}
	}

	if (i > 1 && !is_affected(ch, gsn_deafen))
		 act_puts("You tell your group '{G$t{x'",
		        ch,buf,NULL,TO_CHAR,POS_DEAD);
	else send_to_char("Quit talking to yourself. You are all alone.",ch);

	return;
}



void do_cb(CHAR_DATA *ch, char *argument)
{
	DESCRIPTOR_DATA *d;
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_INPUT_LENGTH];

	if (!(ch->clan)) {
	send_to_char("You are not in a Cabal.\n\r",ch);
	return;
		 }

	snprintf(buf, sizeof(buf), "[%s] $n: {C$t{x",clan_table[ch->clan].short_name);

	if (is_affected(ch,gsn_garble))
		 garble(buf2,argument);
	else
		 strcpy(buf2,argument);

	if (!is_affected(ch, gsn_deafen))
		act_puts(buf, ch, argument, NULL, TO_CHAR,POS_DEAD);
	for (d = descriptor_list; d != NULL; d = d->next) {
	if (d->connected == CON_PLAYING && 
		 (d->character->clan == ch->clan) &&
/*             !IS_SET(d->character->comm,COMM_NOCB) &&   */
		 !is_affected(d->character, gsn_deafen))
		act_puts(buf, ch,buf2,d->character,TO_VICT,POS_DEAD);
	}

	return;
}

void do_pray(CHAR_DATA *ch, char *argument)
{
	DESCRIPTOR_DATA *d;

	if (IS_SET(ch->comm,COMM_NOCHANNELS))
	{
		 send_to_char("The gods refuse to listen to you right now.",ch);
		 return;
	}
	
	send_to_char("You pray to the heavens for help!\n\r",ch);
	send_to_char("This is not an emote, but a channel to the immortals.\n\r",
		ch);

	for (d = descriptor_list; d != NULL; d = d->next)
		 {
	if (d->connected == CON_PLAYING && IS_IMMORTAL(d->character) &&
		!IS_SET(d->character->comm,COMM_NOWIZ))
		 {
		if (argument[0] == '\0')
		act_puts("{W$n{x is PRAYING for: {Gany god{x",
			ch,argument,d->character,TO_VICT,POS_DEAD);
		else
		act_puts("{W$n{x is PRAYING for: {G$t{x",
			ch,argument,d->character,TO_VICT,POS_DEAD);
		 }
		 }
	return;
}

char char_lang_lookup(char c)
{
	int i;
	
	for(i=0; translation_table[i].common[0] != '\0'; i++)
	if (translation_table[i].common[0] == c) 
		return translation_table[i].language[0];
	return c;
}

/* 
 * ch says
 * victim hears 
 */
char *translate(CHAR_DATA *ch, CHAR_DATA *victim, char *argument)
{
	static char trans[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH];
	char c;
	int i;

	if (*argument == '\0'
	||  (ch == NULL) || (victim == NULL)
	||  IS_NPC(ch) || IS_NPC(victim)
	||  IS_IMMORTAL(ch) || IS_IMMORTAL(victim)
	||  ch->language == LANG_COMMON
	||  ch->language == pc_race_table[ORG_RACE(victim)].language) {
		if (IS_IMMORTAL(victim))
			snprintf(trans, sizeof(trans), "{{%s} %s",
				language_table[ch->language].name, argument);
		else
			snprintf(trans, sizeof(trans), argument);
		return trans;
	}

	for(i = 0 ; *argument != '\0'; argument++,i++) {
		 c = char_lang_lookup(*argument);
		 buf[i] = c;
	}
	buf[i] = '\0';

	snprintf(trans, sizeof(trans), "{{%s} %s",
		 language_table[ch->language].name, buf);
	return trans;
}


void do_speak(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	int language;

	if (IS_NPC(ch)) return;

	argument = one_argument(argument,arg);
	if (arg[0] == '\0') {
		char_printf(ch, "You now speak %s.\n\r", 
			language_table[ch->language].name);
		send_to_char("You can speak :\n\r", ch);
		char_printf(ch, "       common, %s\n\r",
			language_table[pc_race_table[ORG_RACE(ch)].language].name);
		return;
	}

	language = lang_lookup(arg);

	if (language == -1)
		{
		send_to_char("You never heard of that language.\n\r", ch);
		return;
		}

	if (language >= MAX_LANGUAGE)
	ch->language = pc_race_table[ORG_RACE(ch)].language;
	else ch->language = language;
	
	char_printf(ch,"Now you speak %s.\n\r",language_table[ch->language].name);
}

/* Thanx zihni@karmi.emu.edu.tr for the code of do_judge */
void do_judge(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	one_argument(argument, arg);

	if (ch->clan != CLAN_RULER) {
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if (!clan_ok(ch,gsn_judge)) return;

	if (arg[0] == '\0') {
		send_to_char("Judge whom?\n\r", ch);
		return;
	}

	/* judge thru world */
	if ((victim = get_char_world(ch, arg)) == NULL) {
		 send_to_char("They aren't here.\n\r", ch);
		 return;
	}


	if (IS_NPC(victim)) {
		 send_to_char("Not a mobile, of course.\n\r", ch);
		 return;
	}

	if (IS_IMMORTAL(victim) && !IS_IMMORTAL(ch)) {
		 send_to_char("You do not have the power to judge Immortals.\n\r",ch);
		return;
	}

	char_printf(ch,"%s's ethos is %s and aligment is %s.\n\r",
		victim->name,
		victim->ethos == 1 ? "Lawful" :
		victim->ethos == 2 ? "Neutral" :
		victim->ethos == 3 ? "Chaotic" : "unknown",
		IS_GOOD(victim) ? "Good" :
		IS_EVIL(victim) ? "Evil" : "Neutral");

	return;
}


void do_noiac(CHAR_DATA *ch, char *arg)
{
	if (IS_SET(ch->comm, COMM_NOIAC)) {
		REMOVE_BIT(ch->comm, COMM_NOIAC);
		char_puts("Text will be sent to you unmodified.\n\r", ch);
	}
	else {
		SET_BIT(ch->comm, COMM_NOIAC);
		char_printf(ch, "IACs will not be sent to you anymore (will be replaced with '%c'.\n\r", IAC_REPL);
	}
}

void do_notelnet(CHAR_DATA *ch, char *arg)
{
	if (IS_SET(ch->comm, COMM_NOTELNET)) {
		REMOVE_BIT(ch->comm, COMM_NOTELNET);
		char_puts("Telnet filter is ON.\n\r", ch);
	}
	else {
		SET_BIT(ch->comm, COMM_NOTELNET);
		char_puts("Telnet filter is OFF.\n\r", ch);
	}
}
