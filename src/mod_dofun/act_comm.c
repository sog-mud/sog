/*
 * $Id: act_comm.c,v 1.278 2003-09-29 23:11:27 fjoe Exp $
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
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#if !defined (WIN32)
#include <unistd.h>
#endif

#include <merc.h>
#include <lang.h>
#include <note.h>

#include <sog.h>
#include <update.h>

#include "toggle.h"

DECLARE_DO_FUN(do_afk);
DECLARE_DO_FUN(do_delete);
DECLARE_DO_FUN(do_channels);
DECLARE_DO_FUN(do_hints);
DECLARE_DO_FUN(do_deaf);
DECLARE_DO_FUN(do_quiet);
DECLARE_DO_FUN(do_replay);
DECLARE_DO_FUN(do_say);
DECLARE_DO_FUN(do_tell);
DECLARE_DO_FUN(do_reply);
DECLARE_DO_FUN(do_gtell);
DECLARE_DO_FUN(do_emote);
DECLARE_DO_FUN(do_pmote);
DECLARE_DO_FUN(do_immtalk);
DECLARE_DO_FUN(do_yell);
DECLARE_DO_FUN(do_shout);
DECLARE_DO_FUN(do_music);
DECLARE_DO_FUN(do_gossip);
DECLARE_DO_FUN(do_clan);
DECLARE_DO_FUN(do_implore);
DECLARE_DO_FUN(do_pose);
DECLARE_DO_FUN(do_rent);
DECLARE_DO_FUN(do_quit);
DECLARE_DO_FUN(do_save);
DECLARE_DO_FUN(do_follow);
DECLARE_DO_FUN(do_order);
DECLARE_DO_FUN(do_group);
DECLARE_DO_FUN(do_split);
DECLARE_DO_FUN(do_speak);
DECLARE_DO_FUN(do_twit);
DECLARE_DO_FUN(do_lang);
DECLARE_DO_FUN(do_judge);
DECLARE_DO_FUN(do_trust);
DECLARE_DO_FUN(do_wanted);
DECLARE_DO_FUN(do_mark);
DECLARE_DO_FUN(do_petition);
DECLARE_DO_FUN(do_promote);
DECLARE_DO_FUN(do_alias);
DECLARE_DO_FUN(do_unalias);
DECLARE_DO_FUN(do_last);
DECLARE_DO_FUN(do_toggle);
DECLARE_DO_FUN(do_ooc);
DECLARE_DO_FUN(do_oocme);
DECLARE_DO_FUN(do_retell);

/* command procedures needed */
DECLARE_DO_FUN(do_look);
DECLARE_DO_FUN(do_help);

DO_FUN(do_afk, ch, argument)
{
	if (IS_SET(ch->comm, COMM_AFK))
		do_replay(ch, str_empty);
	else
		act_char("You are now in AFK mode.", ch);
	TOGGLE_BIT(ch->comm, COMM_AFK);
}

/* RT code to delete yourself */

DO_FUN(do_delete, ch, argument)
{
	if (IS_NPC(ch))
		return;

	if (IS_SET(PC(ch)->plr_flags, PLR_CONFIRM_DELETE)) {
		if (argument[0] != '\0') {
			act_char("Delete status removed.", ch);
			REMOVE_BIT(PC(ch)->plr_flags, PLR_CONFIRM_DELETE);
			return;
		}

		wiznet("$N turns $Mself into line noise.", ch, NULL, 0, 0, 0);
		char_delete(ch, NULL);
		return;
	}

	if (argument[0] != '\0') {
		act_char("Just type delete. No argument.", ch);
		return;
	}

	act_char("Type delete again to confirm this command.", ch);
	act_char("WARNING: this command is irreversible.", ch);
	act_char("Typing delete with an argument will undo delete status.", ch);
	SET_BIT(PC(ch)->plr_flags, PLR_CONFIRM_DELETE);
	wiznet("$N is contemplating deletion.", ch, NULL, 0, 0, ch->level);
}

/* RT code to display channel status */
DO_FUN(do_channels, ch, argument)
{
	/* lists all channels and their status */
	act_char("   channel     status", ch);
	act_char("---------------------", ch);
	act_puts("music          $t",
		 ch, !IS_SET(ch->chan, CHAN_NOMUSIC) ? "ON" : "OFF", NULL,
		 TO_CHAR | ACT_NOUCASE, POS_DEAD);
	act_puts("shout          $t",
		 ch, !IS_SET(ch->chan, CHAN_NOSHOUT) ? "ON" : "OFF", NULL,
		 TO_CHAR | ACT_NOUCASE, POS_DEAD);
	act_puts("auction        $t",
		 ch, !IS_SET(ch->chan, CHAN_NOAUCTION) ? "ON" : "OFF", NULL,
		 TO_CHAR | ACT_NOUCASE, POS_DEAD);
	if (IS_IMMORTAL(ch)) {
		act_puts("god channel    $t",
			 ch, !IS_SET(ch->chan, CHAN_NOWIZ) ? "ON" : "OFF", NULL,
			 TO_CHAR | ACT_NOUCASE, POS_DEAD);
	}
	act_puts("tells          $t",
		 ch, !IS_SET(ch->chan, COMM_DEAF) ? "ON" : "OFF", NULL,
		 TO_CHAR | ACT_NOUCASE, POS_DEAD);
	if (!IS_NPC(ch)) {
		act_puts("hints          $t",
			 ch, flag_string(hint_levels, PC(ch)->hints_level), NULL,
			 TO_CHAR | ACT_NOUCASE, POS_DEAD);
	}
	act_puts("quiet mode     $t",
		 ch, IS_SET(ch->comm, COMM_QUIET) ? "ON" : "OFF", NULL,
		 TO_CHAR | ACT_NOUCASE, POS_DEAD);

	if (IS_SET(ch->comm, COMM_SNOOP_PROOF))
		act_char("You are immune to snooping.", ch);
	if (IS_SET(ch->comm, COMM_NOTELL))
		 act_char("You cannot use tell.", ch);
	if (IS_SET(ch->chan, CHAN_NOCHANNELS))
		act_char("You cannot use channels.", ch);
	if (IS_SET(ch->comm, COMM_NOEMOTE))
		 act_char("You cannot show emotions.", ch);
}

DO_FUN(do_hints, ch, argument)
{
	flag_t hint_level;
	char arg[MAX_INPUT_LENGTH];

	if (IS_NPC(ch)) {
		act_char("Huh?", ch);
		return;
	}

	if (argument[0] == '\0') {
		act_puts("Current hints level is '$t'.",
			 ch, flag_string(hint_levels, PC(ch)->hints_level), NULL,
			 TO_CHAR, POS_DEAD);
		return;
	}

	one_argument(argument, arg, sizeof(arg));
	hint_level = flag_value(hint_levels, arg);
	if (hint_level < 0) {
		dofun("help", ch, "hints");
		return;
	}

	PC(ch)->hints_level = hint_level;
	if (hint_level == HINT_NONE)
		act_char("Hints disabled.", ch);
	else {
		act_puts("Hints level set to '$t'.",
			 ch, flag_string(hint_levels, PC(ch)->hints_level), NULL,
			 TO_CHAR, POS_DEAD);
	}
}

DO_FUN(do_deaf, ch, argument)
{
	if (IS_SET(ch->comm,COMM_DEAF)) {
		act_char("You can now hear tells again.", ch);
		REMOVE_BIT(ch->comm, COMM_DEAF);
	}
	else {
		act_char("From now on, you won't hear tells.", ch);
		SET_BIT(ch->comm, COMM_DEAF);
	}
}

/* RT quiet blocks out all communication */
DO_FUN(do_quiet, ch, argument)
{
	if (IS_SET(ch->comm,COMM_QUIET)) {
		 act_char("Quiet mode removed.", ch);
		 REMOVE_BIT(ch->comm, COMM_QUIET);
	}
	else {
		act_char("From now on, you will only hear says and emotes.", ch);
		SET_BIT(ch->comm, COMM_QUIET);
	}
}

DO_FUN(do_replay, ch, argument)
{
	if (IS_NPC(ch)) {
		act_char("Huh?", ch);
		return;
	}

	page_to_char(buf_string(PC(ch)->buffer), ch);
	buf_clear(PC(ch)->buffer);
}

DO_FUN(do_say, ch, argument)
{
	CHAR_DATA *vch;
	OBJ_DATA *obj;

	if (argument[0] == '\0') {
		act_char("Say what?", ch);
		return;
	}

	if (IS_SET(ch->in_room->room_flags, ROOM_SILENT)
	&&  !IS_IMMORTAL(ch)) {
		act_char("You are in silent room, you can't say anything.", ch);
		return;
	}

	if (ch->shapeform != NULL
	&&  IS_SET(ch->shapeform->index->flags, SHAPEFORM_NOSPEAK)) {
		act("You cannot say anything in this form.",
		    ch, NULL, NULL, TO_CHAR);
		return;
	}

	argument = garble(ch, argument);
	act_say(ch, "$T", argument);

	if (IS_NPC(ch))
		return;

	foreach (vch, char_in_room(ch->in_room)) {
		pull_mob_trigger(
		    TRIG_MOB_SPEECH, vch, ch, CAST(void *, argument));
		if (IS_EXTRACTED(ch)) {
			foreach_done(vch);
			return;
		}

		foreach (obj, obj_of_char(vch)) {
			pull_obj_trigger(
			    TRIG_OBJ_SPEECH, obj, ch, CAST(void *, argument));
			if (IS_EXTRACTED(ch)) {
				foreach_done(obj);
				foreach_done(vch);
				return;
			}
		} end_foreach(obj);
	} end_foreach(vch);

	foreach (obj, obj_in_room(ch->in_room)) {
		pull_obj_trigger(
		    TRIG_OBJ_SPEECH, obj, ch, CAST(void *, argument));
		if (IS_EXTRACTED(ch))
			break;
	} end_foreach(obj);
}

DO_FUN(do_tell, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0' || argument[0] == '\0') {
		act_char("Tell whom what?", ch);
		return;
	}

	tell_char(ch, get_char_world(ch, arg), argument);
}

DO_FUN(do_reply, ch, argument)
{
	if (IS_NPC(ch)) {
		act_char("Huh?", ch);
		return;
	}

	tell_char(ch, PC(ch)->reply, argument);
}

DO_FUN(do_gtell, ch, argument)
{
	CHAR_DATA *gch;
	int i;
	int flags;

	if (IS_SET(ch->in_room->room_flags, ROOM_SILENT)
	&&  !IS_IMMORTAL(ch)) {
		act_char("You are in silent room, you can't tell your group"
			 " anything.", ch);
		return;
	}

	if (argument[0] == '\0') {
		act_char("Tell your group what?", ch);
		return;
	}

	if (IS_SET(ch->comm, COMM_NOTELL)) {
		act_char("Your message didn't get through!", ch);
		return;
	}

	argument = garble(ch, argument);
	flags =
	    TO_VICT | ACTQ_GROUP | ACT_TOBUF | (ACT_SPEECH(ch) & ~ACT_NODEAF);
	for (i = 0, gch = char_list; gch; gch = gch->next) {
		if (IS_NPC(gch))
			break;

		i++;
		if (is_same_group(gch, ch) && !is_sn_affected(gch, "deafen")) {
			act_puts("$n tells the group '{G$t{x'",
				 ch, argument, gch, flags, POS_DEAD);
		}
	}

	if (i > 1 && !is_sn_affected(ch, "deafen")) {
		act_puts("You tell your group '{G$t{x'",
			 ch, argument, NULL,
			 TO_CHAR | ACTQ_GROUP | ACT_SPEECH(ch), POS_DEAD);
	} else
		act_char("Quit talking to yourself. You are all alone.", ch);
}

DO_FUN(do_emote, ch, argument)
{
	int flags;

	if (!IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE)) {
		act_char("You can't show your emotions.", ch);
		return;
	}

	if (ch->shapeform != NULL
	&&  IS_SET(ch->shapeform->index->flags, SHAPEFORM_NOEMOTE)) {
		act_char("You aren't able to express your emotions in this form.", ch);
		return;
	}

	if (argument[0] == '\0') {
		act_char("Emote what?", ch);
		return;
	}

	argument = garble(ch, argument);

	flags = ACT_NOTRIG |
		(!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM) ? ACT_NOTRANS : 0);
	act("$n $T", ch, NULL, argument, TO_CHAR | flags);	// notrans

	flags |= ACT_TOBUF | ACT_NOTWIT;
	act("$n $T", ch, NULL, argument, TO_ROOM | flags);	// notrans
}

DO_FUN(do_pmote, ch, argument)
{
	CHAR_DATA *vch;
	const char *letter,*name;
	char last[MAX_INPUT_LENGTH], temp[MAX_STRING_LENGTH];
	size_t matches = 0;
	int flags;

	if (!IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE)) {
		act_char("You can't show your emotions.", ch);
		return;
	}

	if (ch->shapeform != NULL
	&&  IS_SET(ch->shapeform->index->flags, SHAPEFORM_NOEMOTE)) {
		act_char("You aren't able to express your emotions in this form.", ch);
		return;
	}

	if (argument[0] == '\0') {
		act_char("Emote what?", ch);
		return;
	}

	argument = garble(ch, argument);

	flags = TO_CHAR | ACT_NOTRIG |
		(!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM) ? ACT_NOTRANS : 0);
	act("$n $t", ch, argument, NULL, flags);	// notrans

	flags |= ACT_TOBUF | ACT_NOTWIT;
	for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room) {
		if (vch->desc == NULL || vch == ch)
			continue;

		if ((letter = strstr(argument, vch->name)) == NULL) {
			act("$N $t", vch, argument, ch, flags);	// notrans
			continue;
		}

		strlcpy(temp, argument, sizeof(temp));
		temp[strlen(argument) - strlen(letter)] = '\0';
		last[0] = '\0';
		name = vch->name;

		for (; *letter != '\0'; letter++) {
			if (*letter == '\'' && matches == strlen(vch->name)) {
				strlcat(temp, "r", sizeof(temp));
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
					strlcat(temp, "you", sizeof(temp));
					last[0] = '\0';
					name = vch->name;
					continue;
				}
				strlncat(last, letter, sizeof(last), 1);
				continue;
			}

			matches = 0;
			strlcat(temp, last, sizeof(temp));
			strlncat(temp, letter, sizeof(temp), 1);
			last[0] = '\0';
			name = vch->name;
		}

		act("$N $t", vch, temp, ch, flags);		// notrans
	}
}

DO_FUN(do_immtalk, ch, argument)
{
	CHAR_DATA *vch, *vch_next;
	CHAR_DATA *orig;
	int flags;

	orig = GET_ORIGINAL(ch);
	if (argument[0] == '\0') {
		TOGGLE_BIT(orig->chan, CHAN_NOWIZ);
		if (IS_SET(orig->chan, CHAN_NOWIZ))
			act_char("Immortal channel is now OFF.", ch);
		else
			act_char("Immortal channel is now ON.", ch);
		return;
	}

	if (IS_SET(orig->chan, CHAN_NOCHANNELS)) {
		act_char("The gods have revoked your channel privileges.", ch);
		return;
	}

	if (IS_SET(orig->chan, CHAN_NOWIZ))
		do_immtalk(ch, str_empty);

	flags = ACT_SPEECH(orig) & ~(ACT_STRANS | ACT_NODEAF);
	act_puts("[IMM] $n: {C$t{x",				// notrans
	    orig, argument, NULL, TO_CHAR | ACTQ_IMMTALK | flags, POS_DEAD);

	for (vch = char_list; vch != NULL && !IS_NPC(vch); vch = vch_next) {
		CHAR_DATA *victim = GET_ORIGINAL(vch);
		vch_next = vch->next;

		if (!IS_IMMORTAL(victim)
		||  IS_SET(victim->comm, CHAN_NOWIZ))
			continue;

		act_puts("[IMM] $n: {C$t{x",			// notrans
		    orig, argument, vch, TO_VICT | ACT_TOBUF | flags, POS_DEAD);
	}
}

DO_FUN(do_yell, ch, argument)
{
	if (IS_NPC(ch) && IS_SET(ch->pMobIndex->act, ACT_IMMOBILE))
		return;

	if (IS_SET(ch->in_room->room_flags, ROOM_SILENT)
	&&  !IS_IMMORTAL(ch)) {
		act_char("You are in silent room, you can't yell.", ch);
		return;
	}

	if (IS_SET(ch->chan, CHAN_NOCHANNELS)) {
		 act_char("The gods have revoked your channel privileges.", ch);
		 return;
	}

	if (argument[0] == '\0') {
		act_char("Yell what?", ch);
		return;
	}

	if (ch->shapeform != NULL
	&&  IS_SET(ch->shapeform->index->flags, SHAPEFORM_NOSPEAK)) {
		act("You cannot yell in this form.", ch, NULL, NULL, TO_CHAR);
		return;
	}

	argument = garble(ch, argument);
	act_puts("You yell '{M$t{x'",
		 ch, argument, NULL, TO_CHAR | ACT_SPEECH(ch), POS_DEAD);
	act_yell(ch, "$T", argument, NULL);
}

DO_FUN(do_shout, ch, argument)
{
	DESCRIPTOR_DATA *d;

	if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
		return;

	if (argument[0] == '\0') {
		TOGGLE_BIT(ch->chan, CHAN_NOSHOUT);
		if (IS_SET(ch->chan, CHAN_NOSHOUT))
			act_char("You will no longer hear shouts.", ch);
		else
			act_char("You will now hear shouts.", ch);
		return;
	}

	if (IS_SET(ch->in_room->room_flags, ROOM_SILENT)
	&&  !IS_IMMORTAL(ch)) {
		act_char("You are in silent room, you can't shout.", ch);
		return;
	}

	if (IS_SET(ch->chan, CHAN_NOCHANNELS)) {
		 act_char("The gods have revoked your channel privileges.", ch);
		 return;
	}

	if (ch->shapeform != NULL
	&&  IS_SET(ch->shapeform->index->flags, SHAPEFORM_NOSPEAK)) {
		act("You cannot shout in this form.", ch, NULL, NULL, TO_CHAR);
		return;
	}

	if (IS_SET(ch->chan, CHAN_NOSHOUT))
		do_shout(ch, str_empty);
	WAIT_STATE(ch, get_pulse("violence"));

	argument = garble(ch, argument);
	act_puts("You shout '{Y$t{x'",
	    ch, argument, NULL, TO_CHAR | ACTQ_CHAN | ACT_SPEECH(ch), POS_DEAD);

	for (d = descriptor_list; d; d = d->next) {
		if (d->connected == CON_PLAYING
		&&  d->character != ch
		&&  !IS_SET(d->character->chan, CHAN_NOSHOUT)
		&&  (!IS_SET(d->character->in_room->room_flags, ROOM_SILENT) ||
		     IS_IMMORTAL(d->character))) {
			act_puts("$n shouts '{Y$t{x'",
			    ch, argument, d->character,
			    TO_VICT | ACTQ_CHAN | ACT_NOTWIT | ACT_SPEECH(ch),
			    POS_DEAD);
		}
	}
}

DO_FUN(do_music, ch, argument)
{
	DESCRIPTOR_DATA *d;

	if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
		return;

	if (argument[0] == '\0') {
		TOGGLE_BIT(ch->chan, CHAN_NOMUSIC);
		if (IS_SET(ch->chan, CHAN_NOMUSIC))
			act_char("You will no longer hear music.", ch);
		else
			act_char("You will now hear music.", ch);
		return;
	}

	if (IS_SET(ch->in_room->room_flags, ROOM_SILENT)
	&&  !IS_IMMORTAL(ch)) {
		act_char("You are in silent room, you can't music.", ch);
		return;
	}

	if (IS_SET(ch->chan, CHAN_NOCHANNELS)) {
		 act_char("The gods have revoked your channel privileges.", ch);
		 return;
	}

	if (IS_SET(ch->chan, CHAN_NOMUSIC))
		do_music(ch, str_empty);
	WAIT_STATE(ch, get_pulse("violence"));

	argument = garble(ch, argument);
	act_puts("You music '{W$t{x'",
		 ch, argument, NULL, TO_CHAR | ACT_SPEECH(ch), POS_DEAD);

	for (d = descriptor_list; d; d = d->next) {
		if (d->connected == CON_PLAYING
		&&  d->character != ch
		&&  !IS_SET(d->character->chan, CHAN_NOMUSIC)
		&&  (!IS_SET(d->character->in_room->room_flags, ROOM_SILENT) ||
		     IS_IMMORTAL(d->character))) {
			act_puts("$n musics '{W$t{x'",
				 ch, argument, d->character,
				 TO_VICT | ACT_NOTWIT | ACT_SPEECH(ch),
				 POS_DEAD);
		}
	}
}

DO_FUN(do_gossip, ch, argument)
{
	DESCRIPTOR_DATA *d;

	if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
		return;

	if (IS_SET(ch->chan, CHAN_NOCHANNELS)) {
		 act_char("The gods have revoked your channel privileges.", ch);
		 return;
	}

	if (ch->shapeform != NULL
	&&  IS_SET(ch->shapeform->index->flags, SHAPEFORM_NOSPEAK)) {
		act("You cannot gossip in this form.", ch, NULL, NULL, TO_CHAR);
		return;
	}

	if (argument[0] == '\0') {
		act_char("Gossip what?", ch);
		return;
	}

	WAIT_STATE(ch, get_pulse("violence"));

	if (!IS_IMMORTAL(ch)) {
		if (ch->mana < ch->max_mana) {
			act_char("You don't have enough mana.", ch);
			return;
		}
		ch->mana = 0;
	}

	argument = garble(ch, argument);
	act_puts("You gossip '{R$t{x'",
	    ch, argument, NULL, TO_CHAR | ACTQ_CHAN | ACT_SPEECH(ch), POS_DEAD);

	for (d = descriptor_list; d; d = d->next) {
		if (d->connected == CON_PLAYING
		&&  d->character != ch) {
			act_puts("$n gossips '{R$t{x'",
			    ch, argument, d->character,
			    TO_VICT | ACTQ_CHAN | ACT_SPEECH(ch), POS_DEAD);
		}
	}
}

DO_FUN(do_clan, ch, argument)
{
	clan_t *clan;

	if ((clan = clan_lookup(ch->clan)) == NULL) {
		act_char("You are not in a clan.", ch);
		return;
	}

	if (argument[0] == '\0') {
		TOGGLE_BIT(ch->chan, CHAN_NOCLAN);
		if (IS_SET(ch->chan, CHAN_NOCLAN))
			act_char("You will no longer hear clan talks.", ch);
		else
			act_char("You will now hear clan talks.", ch);
		return;
	}

	if (IS_SET(ch->in_room->room_flags, ROOM_SILENT)
	&&  !IS_IMMORTAL(ch)) {
		act_char("You are in silent room, you can't tell to your "
			 "clan anything.", ch);
		return;
	}

	if (IS_SET(ch->chan, CHAN_NOCLAN))
		do_clan(ch, str_empty);

	argument = garble(ch, argument);
	act_puts("[CLAN] $lu{$n}: {C$t{x",
	    ch, argument, NULL, TO_CHAR | ACTQ_CLAN | ACT_SPEECH(ch), POS_DEAD);
	act_clan(ch, "$T", argument);
}

DO_FUN(do_implore, ch, argument)
{
	CHAR_DATA *vch, *vch_next;

	if (IS_NPC(ch))
		return;

	if (IS_SET(ch->chan, CHAN_NOCHANNELS)) {
		 act_char("The gods refuse to listen to you right now.", ch);
		 return;
	}

	if (argument[0] == '\0')
		argument = "help";

	act_puts("You implore the immortals for '$t'",
		 ch, argument, NULL, TO_CHAR, POS_DEAD);

	for (vch = char_list; vch != NULL && !IS_NPC(vch); vch = vch_next) {
		CHAR_DATA *victim = GET_ORIGINAL(vch);
		vch_next = vch->next;

		if (!IS_IMMORTAL(victim)
		||  IS_SET(victim->chan, CHAN_NOWIZ))
			continue;

		act_puts("{W$n{x implores you for '{G$t{x'",
			 ch, argument, vch,
			 TO_VICT | ACT_TOBUF, POS_DEAD);
	 }
}

DO_FUN(do_pose, ch, argument)
{
	class_t *cl;
	pose_t *pose;

	if (IS_NPC(ch)
	||  (cl = class_lookup(ch->class)) == NULL
	||  c_isempty(&cl->poses))
		return;

	pose = c_random_elem(&cl->poses);
	act(pose->self, ch, NULL, NULL, TO_CHAR);
	act(pose->others, ch, NULL, NULL, TO_ROOM | ACT_TOBUF);
}

DO_FUN(do_rent, ch, argument)
{
	act_char("There is no rent here.  Just save and quit.", ch);
	return;
}

DO_FUN(do_quit, ch, argument)
{
	PC_DATA *pc;

	if (IS_NPC(ch)) {
		act_char("Huh?", ch);
		return;
	}

	pc = PC(ch);
	if (pc->pnote) {
		act_puts("You have an unfinished $w in progress.",
			 ch, flag_string(note_types, pc->pnote->type), NULL,
			 TO_CHAR, POS_DEAD);
		return;
	}

	quit_char(ch, XC_F_NOCOUNT);
}

DO_FUN(do_save, ch, argument)
{
	if (IS_NPC(ch))
		return;
	if (ch->level < 2) {
		act_char("You must be at least level 2 for saving.", ch);
		return;
	}

	/*
	 * char_save intentionally omitted
	 */
	act_puts("Saving.", ch, NULL, NULL, TO_CHAR, POS_DEAD);

	WAIT_STATE(ch, get_pulse("violence"));
}

DO_FUN(do_follow, ch, argument)
{
/* RT changed to allow unlimited following and follow the NOFOLLOW rules */
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		act_char("Follow whom?", ch);
		return;
	}

	if ((victim = get_char_here(ch, arg)) == NULL) {
		act_char("They aren't here.", ch);
		return;
	}

	if (IS_AFFECTED(ch, AFF_CHARM) && ch->master != NULL) {
		act("But you'd rather follow $N!", ch, NULL,ch->master,TO_CHAR);
		return;
	}

	if (victim == ch) {
		if (ch->master == NULL) {
			act_char("You already follow yourself.", ch);
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

DO_FUN(do_order, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	argument = one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0' || argument[0] == '\0') {
		act_char("Order whom to do what?", ch);
		return;
	}

	if (ch->shapeform != NULL
	&&  IS_SET(ch->shapeform->index->flags, SHAPEFORM_NOSPEAK)) {
		act_char("You cannot give orders in this form.", ch);
		return;
	}

	if (IS_AFFECTED(ch, AFF_CHARM)) {
		act_char("You feel like taking, not giving, orders.", ch);
		return;
	}

	if (!str_cmp(arg, "all")) {
		bool found = FALSE;
		CHAR_DATA *vch;

		foreach (vch, char_in_room(ch->in_room)) {
			if (!IS_AFFECTED(vch, AFF_CHARM)
			||  vch->master != ch)
				continue;

			act("$n orders you to '$t', you do.",
			    ch, argument, vch, TO_VICT);
			interpret(vch, argument, TRUE);
			found = TRUE;
		} end_foreach(vch);

		if (found) {
			WAIT_STATE(ch, get_pulse("violence"));
			act_char("Ok.", ch);
		} else
			act_char("You have no followers here.", ch);
		return;
	}

	if ((victim = get_char_here(ch, arg)) == NULL) {
		act_char("They aren't here.", ch);
		return;
	}

	if (victim == ch) {
		act_char("Aye aye, right away!", ch);
		return;
	}

	if (is_sn_affected(victim, "deafen")) {
		act("$N doesn't hear you.", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (!IS_AFFECTED(victim, AFF_CHARM)
	||  victim->master != ch) {
		act_char("Do it yourself!", ch);
		return;
	}

	act("$n orders you to '$t', you do.", ch, argument, victim, TO_VICT);
	interpret(victim, argument, TRUE);
	WAIT_STATE(ch, get_pulse("violence"));
	act_char("Ok.", ch);
}
static bool
is_in_opposite_clans(CHAR_DATA *ch1, CHAR_DATA *ch2)
{
	return (has_spec(ch1, "clan_rulers") && has_spec(ch2, "clan_chaos"))
	|| (has_spec(ch1, "clan_chaos") && has_spec(ch2, "clan_rulers"))
	|| (has_spec(ch1, "clan_invaders") && has_spec(ch2, "clan_knights"))
	|| (has_spec(ch1, "clan_knights") && has_spec(ch2, "clan_invaders"))
	|| (has_spec(ch1, "clan_shalafi") && has_spec(ch2, "clan_battleragers"))
	|| (has_spec(ch1, "clan_battleragers") && has_spec(ch2, "clan_shalafi"));
}

DO_FUN(do_group, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	CHAR_DATA *gch;

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		BUFFER *buf;

		buf = buf_new(GET_LANG(ch));
		buf_act(buf, BUF_END, "$N's group:",
			ch, NULL, leader_lookup(ch), 0);

		for (gch = char_list; gch; gch = gch->next) {
			if (is_same_group(gch, ch)) {
				if (IS_NPC(gch)) {
					buf_printf(buf, BUF_END,
					    "[%2d %s] %-16s %d/%d hp "
					    "%d/%d mana %d/%d mv\n",
					    gch->level,
					    class_who_name(gch),
					    PERS(gch, ch, GET_LANG(ch), ACT_FORMSH),
					    gch->hit,   gch->max_hit,
					    gch->mana,  gch->max_mana,
					    gch->move,  gch->max_move);
				} else {
					buf_printf(buf, BUF_END,
					    "[%2d %s] %-16s %d/%d hp "
					    "%d/%d mana %d/%d mv %d tnl\n",
					    gch->level,
					    class_who_name(gch),
					    PERS(gch, ch, GET_LANG(ch), ACT_FORMSH),
					    gch->hit,   gch->max_hit,
					    gch->mana,  gch->max_mana,
					    gch->move,  gch->max_move,
					    exp_to_level(gch));
				}
			}
		}

		send_to_char(buf_string(buf), ch);
		buf_free(buf);
		return;
	}

	if ((victim = get_char_here(ch, arg)) == NULL) {
		act_char("They aren't here.", ch);
		return;
	}

	if (victim == ch) {
		act_char("Huh? Grouping with yourself?!", ch);
		return;
	}

	if (ch->master != NULL || ch->leader != NULL) {
		act_char("But you are following someone else!", ch);
		return;
	}

	if (victim->master != ch && ch != victim) {
		act("$N isn't following you.", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (IS_AFFECTED(victim, AFF_CHARM)) {
		act_char("You can't remove charmed mobs from your group.", ch);
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
DO_FUN(do_split, ch, argument)
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
	ch->gold	-= amount_gold;
	ch->gold	+= share_gold + extra_gold;

	if (share_silver > 0) {
		act_puts3("You split $j $qj{silver coins}. "
			  "Your share is $J silver.",
			  ch, (const void*) amount_silver, NULL,
			  (const void *) (share_silver + extra_silver),
			  TO_CHAR, POS_DEAD);
	}

	if (share_gold > 0) {
		act_puts3("You split $j $qj{gold coins}. "
			  "Your share is $J gold.",
			  ch, (const void*) amount_gold, NULL,
			  (const void *) (share_gold + extra_gold),
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

DO_FUN(do_speak, ch, argument)
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
		act_puts("You now speak $t.",
			 ch, flag_string(slang_table, ch->slang), NULL,
			 TO_CHAR | ACT_NOUCASE, POS_DEAD);
		act_char("You can speak:", ch);
		act_puts("       common, $t",
			 ch, flag_string(slang_table, r->race_pcdata->slang),
			 NULL, TO_CHAR | ACT_NOUCASE, POS_DEAD);
		return;
	}

	if ((language = flag_value(slang_table, arg)) < 0) {
		act_char("You never heard of that language.", ch);
		return;
	}

	if (language != SLANG_COMMON
	&&  (language != r->race_pcdata->slang && !IS_IMMORTAL(ch))) {
		act_puts("You do not know how to speak $t.",
			 ch, flag_string(slang_table, language),
			 NULL, TO_CHAR | ACT_NOUCASE, POS_DEAD);
		return;
	}

	ch->slang = language;
	act_puts("Now you speak $t.",
		 ch, flag_string(slang_table, ch->slang), NULL,
		 TO_CHAR | ACT_NOUCASE, POS_DEAD);
}

DO_FUN(do_twit, ch, argument)
{
	char arg[MAX_STRING_LENGTH];

	if (IS_NPC(ch)) {
		act_char("Huh?", ch);
		return;
	}

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		act_puts("Current twitlist is [$t]",
			 ch, PC(ch)->twitlist, NULL,
			 TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return;
	}

	name_toggle(&PC(ch)->twitlist, arg, ch, "Twitlist");
}

DO_FUN(do_lang, ch, argument)
{
	char arg[MAX_STRING_LENGTH];
	lang_t *l;
	DESCRIPTOR_DATA *d;

	if (c_isempty(&langs)) {
		act_char("No languages defined.", ch);
		return;
	}

	if ((d = ch->desc) == NULL)
		return;

	argument = one_argument(argument, arg, sizeof(arg));

	if (*arg == '\0') {
		l = varr_get(&langs, d->dvdata->lang);
		if (l == NULL) {
			log(LOG_INFO, "do_lang: %s: lang == %d\n",
				   ch->name, d->dvdata->lang);
			l = VARR_GET(&langs, d->dvdata->lang = 0);
		}
		act_puts("Interface language is '$t'.",
			 ch, l->name, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return;
	}

	if ((l = lang_lookup(arg)) == NULL) {
		act_puts("Usage: lang [ ",
			 ch, NULL, NULL, TO_CHAR | ACT_NOLF, POS_DEAD);
		C_FOREACH(l, &langs) {
			if (IS_SET(l->lang_flags, LANG_HIDDEN))
				continue;
			act_puts(" $T$t",			// notrans
				 ch, l->name,
				 varr_index(&langs, l) == 0 ?
					str_empty : " | ", // notrans
				 TO_CHAR | ACT_NOLF | ACT_NOTRANS, POS_DEAD);
		}
		act_char(" ]", ch);				// notrans
		return;
	}

	d->dvdata->lang = varr_index(&langs, l);
	do_lang(ch, str_empty);
	do_look(ch, str_empty);
}

DO_FUN(do_judge, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	if (!has_spec(ch, "clan_rulers")) {
		act_char("Huh?", ch);
		return;
	}

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		act_char("Judge whom?", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg)) == NULL) {
		act_char("They aren't here.", ch);
		return;
	}

	if (IS_NPC(victim)) {
		act_char("Not a mobile, of course.", ch);
		return;
	}

	if (IS_IMMORTAL(victim) && !IS_IMMORTAL(ch)) {
		act_char("You do not have the power to judge Immortals.", ch);
		return;
	}

	act_puts3("$N is $u-$U.",
		  ch,
		  flag_string(ethos_table, PC(victim)->ethos),
		  victim,
		  flag_string(align_names, NALIGN(victim)),
		  TO_CHAR | ACT_NOTRANS | ACT_FORMSH, POS_DEAD);
}

DO_FUN(do_trust, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];
	PC_DATA *pc;

	if (IS_NPC(ch)) {
		act_char("Huh?", ch);
		return;
	}

	pc = PC(ch);
	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		if (!pc->trust) {
			act_char("You do not allow anyone to cast questionable spells on you.", ch);
			return;
		}

		if (IS_SET(pc->trust, TRUST_ALL)) {
			act_char("You allow everyone to cast questionable spells on you.", ch);
			return;
		}
		if (IS_SET(pc->trust, TRUST_CLAN))
			act_char("You trust your clan with questionable spells.", ch);
		if (IS_SET(pc->trust, TRUST_GROUP))
			act_char("You trust your group with questionable spells.", ch);
		return;
	}

	if (!str_cmp(arg, "clan")) {
		if (IS_NULLSTR(ch->clan)) {
			act_char("You are not in clan.", ch);
			return;
		};

		TOGGLE_BIT(pc->trust, TRUST_CLAN);
		if (IS_SET(pc->trust, TRUST_CLAN)) {
			REMOVE_BIT(pc->trust, TRUST_ALL);
			act_char("You now trust your clan with questionable spells.", ch);
		} else
			act_char("You no longer trust your clan with questionable spells.", ch);
		return;
	}

	if (!str_cmp(arg, "group")) {
		TOGGLE_BIT(pc->trust, TRUST_GROUP);
		if (IS_SET(pc->trust, TRUST_GROUP)) {
			REMOVE_BIT(pc->trust, TRUST_ALL);
			act_char("You allow your group to cast questionable spells on you.", ch);
		}
		else
			act_char("You no longer trust your group with questionable spells.", ch);
		return;
	}

	if (!str_cmp(arg, "all")) {
		pc->trust = TRUST_ALL;
		act_char("You allow everyone to cast questionable spells on you.", ch);
		return;
	}

	if (!str_cmp(arg, "none")) {
		pc->trust = 0;
		act_char("You do not allow anyone to cast questionable spells on you.", ch);
		return;
	}

	act_char("Syntax: trust {{ group | clan | all | none $}", ch);
}

DO_FUN(do_wanted, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		act_char("Set wanted whom?", ch);
		return;
	}

	if ((victim = get_char_area(ch, arg)) == NULL) {
		act_char("They aren't here.", ch);
		return;
	}

	if (IS_NPC(victim)) {
		act_char("Not a mobile, of course.", ch);
		return;
	}

	if (IS_IMMORTAL(victim) && ch->level < victim->level) {
		act("You do not have the power to arrest $N.",
		    ch, NULL, victim, TO_CHAR);
		return;
	}

	if (victim == ch) {
		act_char("You cannot do that to yourself.", ch);
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
	}
	else {
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
DO_FUN(do_mark, ch, argument)
{
	OBJ_DATA *mark;
	clan_t *clan = NULL;

	if ((clan = clan_lookup(ch->clan)) == NULL) {
		act_char("You are not in clan.", ch);
		return;
	}

	if (!clan->mark_vnum) {
		act_char("Your clan do not have any mark.", ch);
		return;
	}

	if ((mark = get_eq_char(ch, WEAR_CLANMARK)) != NULL)
		extract_obj(mark, 0);

	if ((mark = create_obj(clan->mark_vnum, 0)) != NULL) {
		obj_to_char(mark, ch);
		equip_char(ch, mark, WEAR_CLANMARK);
	}
}

DO_FUN(do_petition, ch, argument)
{
	bool accept;
	clan_t *clan = NULL;
	char arg1[MAX_STRING_LENGTH];
	OBJ_DATA *mark;
	PC_DATA *pc;

	if (IS_NPC(ch))
		return;

	argument = one_argument(argument, arg1, sizeof(arg1));
	pc = PC(ch);

	if (IS_NULLSTR(arg1)) {
		if (pc->clan_status == CLAN_LEADER
		||  pc->clan_status == CLAN_SECOND
		||  IS_IMMORTAL(ch)) {
			if (IS_IMMORTAL(ch)) {
				act_char("Usage: petition <clan name> <accept | reject> <char name>", ch);
			} else {
				act_char("Usage: petition <accept | reject> <char name>", ch);
			}
		} else if (IS_NULLSTR(ch->clan))
			act_char("Usage: petition <clan name>", ch);
		return;
	}

	/*
	 * immortals should specify clan explicitly
	 */
	if (IS_IMMORTAL(ch)) {
		if ((clan = clan_search(arg1)) == NULL) {
			act_puts("$t: unknown clan", ch, arg1, NULL,
				 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
			do_petition(ch, str_empty);
			return;
		}

		argument = one_argument(argument, arg1, sizeof(arg1));
		if (IS_NULLSTR(arg1)) {
			do_petition(ch, str_empty);
			return;
		}
	}

	if ((accept = !str_prefix(arg1, "accept"))
	||  !str_prefix(arg1, "reject")) {
		CHAR_DATA *victim;
		PC_DATA *vpc;
		char arg2[MAX_STRING_LENGTH];
		bool loaded = FALSE;
		bool v_changed = FALSE;
		bool changed;

		if (pc->clan_status != CLAN_LEADER
		&&  pc->clan_status != CLAN_SECOND
		&&  !IS_IMMORTAL(ch)) {
			act_char("You don't have enough power to accept/reject petitions.", ch);
			return;
		}

		if (clan == NULL
		&&  (clan = clan_lookup(ch->clan)) == NULL) {
			do_petition(ch, str_empty);
			return;
		}

		argument = one_argument(argument, arg2, sizeof(arg2));
		if (IS_NULLSTR(arg2)) {
			do_petition(ch, str_empty);
			return;
		}

		if ((victim = get_char_world(ch, arg2)) == NULL
		||  IS_NPC(victim)) {
			victim = char_load(arg2, LOAD_F_NOCREATE);
			if (victim == NULL) {
				act_char("Can't find them.", ch);
				return;
			}
			loaded = TRUE;
		}

		vpc = PC(victim);

		if (accept) {
			if (!IS_CLAN(vpc->petition, clan->name)) {
				act_char("They didn't petition.", ch);
				goto cleanup;
			}

			free_string(vpc->petition);
			vpc->petition = str_empty;

			free_string(victim->clan);
			victim->clan = str_dup(clan->name);

			vpc->clan_status = CLAN_COMMONER;
			spec_update(victim);

			name_add(&clan->member_list, victim->name, NULL, NULL);
			clan_save(clan->name);

			act_char("Greet new member!", ch);
			v_changed = TRUE;
			if (!loaded)
				act("Your petition to $t has been accepted.",
				    victim, clan->name, NULL, TO_CHAR);
			if ((mark = get_eq_char(victim, WEAR_CLANMARK)) != NULL)
				extract_obj(mark, 0);

			if (clan->mark_vnum != 0
			&&  (mark = create_obj(clan->mark_vnum, 0)) != NULL) {
				obj_to_char(mark, victim);
				equip_char(victim, mark, WEAR_CLANMARK);
			}

			goto cleanup;
		}

/* handle 'petition reject' */
		if (IS_CLAN(victim->clan, clan->name)) {
			if ((vpc->clan_status == CLAN_LEADER &&
			     !IS_IMMORTAL(ch))
			||  (IS_IMMORTAL(victim) &&
			     !IS_TRUSTED(ch, trust_level(victim)))) {
				act_char("You don't have enough power to do that.", ch);
				return;
			}

			clan_update_lists(clan->name, victim, TRUE);
			clan_save(clan->name);

			free_string(victim->clan);
			victim->clan = str_empty;

			vpc->clan_status = CLAN_COMMONER;
			REMOVE_BIT(vpc->trust, TRUST_CLAN);
			spec_update(victim);
			set_title(victim, str_empty);

			act("They are not a member of $t anymore.",
			    ch, clan->name, NULL, TO_CHAR);
			v_changed = TRUE;
			if (!loaded) {
				act("You are not a member of $t anymore.",
				    victim, clan->name, NULL, TO_CHAR);
			}

			if ((mark = get_eq_char(victim, WEAR_CLANMARK))) {
				extract_obj(mark, 0);
				mark = create_obj(OBJ_VNUM_RENEGADE_MARK, 0);
				if (mark != NULL) {
					obj_to_char(mark, victim);
					equip_char(victim, mark, WEAR_CLANMARK);
				}
			}

			goto cleanup;
		}

		if (IS_CLAN(vpc->petition, clan->name)) {
			free_string(vpc->petition);
			vpc->petition = str_empty;
			act_char("Petition was rejected.", ch);
			v_changed = TRUE;
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
			clan_save(clan->name);

		act_char("They didn't petition.", ch);

	cleanup:
		if (v_changed)
			char_save(victim, loaded ? SAVE_F_PSCAN : 0);
		if (loaded)
			char_nuke(victim);
		return;
	}

	if (IS_IMMORTAL(ch)
	||  pc->clan_status == CLAN_LEADER
	||  pc->clan_status == CLAN_SECOND) {
		/*
		 * list characters petitioned to clan
		 */

		DESCRIPTOR_DATA *d;
		bool found = FALSE;

		if (clan == NULL
		&&  (clan = clan_lookup(ch->clan)) == NULL) {
			do_petition(ch, str_empty);
			return;
		}

		for (d = descriptor_list; d; d = d->next) {
			CHAR_DATA *vch = d->original ? d->original :
						       d->character;

			if (!vch
			||  !IS_NULLSTR(vch->clan)
			||  !IS_CLAN(PC(vch)->petition, clan->name))
				continue;

			if (!found) {
				found = TRUE;
				act_puts("List of players petitioned to $t:",
					 ch, clan->name, NULL,
					 TO_CHAR | ACT_NOTRANS, POS_DEAD);
			}
			act_puts("$t", ch, vch->name, NULL,
				 TO_CHAR | ACT_NOTRANS, POS_DEAD);
		}

		if (!found) {
			act_puts("Noone has petitioned to $t.",
				 ch, clan->name, NULL,
				 TO_CHAR | ACT_NOTRANS, POS_DEAD);
		}
		return;
	}

	/*
	 * send petition to clan
	 */

	if (ch->level < LEVEL_PK) {
		act("You are not ready to join clans.",
		    ch, NULL, NULL, TO_CHAR);
		return;
	}

	if ((clan = clan_search(arg1)) == 0) {
		act_char("No such clan.", ch);
		return;
	}

	if (!IS_NULLSTR(ch->clan)) {
		act_puts("You cannot leave your clan this way.",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		return;
	}

	if ((mark = get_eq_char(ch, WEAR_CLANMARK)) != NULL
	&&  mark->pObjIndex->vnum == OBJ_VNUM_RENEGADE_MARK) {
		act_puts("You are renegade and cannot join clans anymore.",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		return;
	}

	free_string(pc->petition);
	pc->petition = str_qdup(clan->name);
	act_char("Petition sent.", ch);
}

DO_FUN(do_promote, ch, argument)
{
	char arg1[MAX_STRING_LENGTH];
	char arg2[MAX_STRING_LENGTH];
	CHAR_DATA *victim;
	clan_t *clan;
	PC_DATA *vpc;
	bool changed = FALSE, loaded = FALSE;

	if (IS_NPC(ch)
	||  (!IS_IMMORTAL(ch) && PC(ch)->clan_status != CLAN_LEADER)) {
		act_char("Huh?", ch);
		return;
	}

	argument = one_argument(argument, arg1, sizeof(arg1));
	argument = one_argument(argument, arg2, sizeof(arg2));

	if (!*arg1 || !*arg2) {
		act_char("Usage: promote <char name> <commoner | secondary>", ch);
		if (IS_IMMORTAL(ch))
			act_char("   or: promote <char name> <leader>", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg1)) == NULL) {
		if ((victim = char_load(arg1, LOAD_F_NOCREATE)) == NULL) {
			act("No such player.", ch, NULL, NULL, TO_CHAR);
			return;
		}
		loaded = TRUE;
	}

	if ((clan = clan_lookup(victim->clan)) == NULL
	||  (!IS_CLAN(victim->clan, ch->clan) && !IS_IMMORTAL(ch))) {
		act("They are not an a clan.", ch, NULL, victim, TO_CHAR);
		goto cleanup;
	}

	vpc = PC(victim);

	if (!IS_IMMORTAL(ch) && vpc->clan_status == CLAN_LEADER) {
		act_char("You don't have enough power to promote them.", ch);
		goto cleanup;
	}

	if (!str_prefix(arg2, "leader") && IS_IMMORTAL(ch)) {
		if (vpc->clan_status == CLAN_LEADER) {
			act_char("They are already leader in a clan.", ch);
			goto cleanup;
		}

		clan_update_lists(clan->name, victim, FALSE);
		name_add(&clan->leader_list, victim->name, NULL, NULL);
		clan_save(clan->name);

		vpc->clan_status = CLAN_LEADER;
		if (ch != victim)
			act_char("They are now leader in their clan.", ch);
		act_char("You are now leader in your clan.", victim);
		changed = TRUE;
		goto cleanup;
	}

	if (!str_prefix(arg2, "secondary")) {
		if (vpc->clan_status == CLAN_SECOND) {
			act_char("They are already second in a clan.", ch);
			goto cleanup;
		}

		clan_update_lists(clan->name, victim, FALSE);
		name_add(&clan->second_list, victim->name, NULL, NULL);
		clan_save(clan->name);

		vpc->clan_status = CLAN_SECOND;
		if (ch != victim)
			act_char("They are now second in the clan.", ch);
		act_char("You are now second in the clan.", victim);
		changed = TRUE;
		goto cleanup;
	}

	if (!str_prefix(arg2, "commoner")) {
		if (vpc->clan_status == CLAN_COMMONER) {
			act_char("They are already commoner in a clan.", ch);
			goto cleanup;
		}

		clan_update_lists(clan->name, victim, FALSE);
		clan_save(clan->name);

		vpc->clan_status = CLAN_COMMONER;
		if (ch != victim) {
			act("They are now commoner in the clan.",
			    ch, NULL, victim, TO_CHAR);
		}
		act_char("You are now commoner in the clan.", victim);
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

DO_FUN(do_alias, ch, argument)
{
	DESCRIPTOR_DATA *d;
	char arg[MAX_INPUT_LENGTH];
	int pos;

	if ((d = ch->desc) == NULL)
		return;

	argument = one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		if (d->dvdata->alias[0] == NULL) {
			act_char("You have no aliases defined.", ch);
			return;
		}

		act_char("Your current aliases are:", ch);
		for (pos = 0; pos < MAX_ALIAS; pos++) {
			if (d->dvdata->alias[pos] == NULL
			||  d->dvdata->alias_sub[pos] == NULL)
				break;

			act_puts("    $t: $T",			// notrans
				 ch, d->dvdata->alias[pos],
				 d->dvdata->alias_sub[pos],
				 TO_CHAR | ACT_NOTRANS, POS_DEAD);
		}
		return;
	}

	if (!str_cmp(arg, "unalias") || !str_cmp(arg, "alias")) {
		act_char("Sorry, that word is reserved.", ch);
		return;
	}

	if (argument[0] == '\0') {
		for (pos = 0; pos < MAX_ALIAS; pos++) {
			if (d->dvdata->alias[pos] == NULL
			||  d->dvdata->alias_sub[pos] == NULL)
				break;

			if (!str_cmp(arg, d->dvdata->alias[pos])) {
				act_puts("$t aliases to '$T'.",
					 ch, d->dvdata->alias[pos],
					 d->dvdata->alias_sub[pos],
					 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE,
					 POS_DEAD);
				return;
			}
		}

		act_char("That alias is not defined.", ch);
		return;
	}

	if (!str_prefix(argument, "delete")
	||  !str_prefix(argument, "prefix")) {
		act_char("That shall not be done!", ch);
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

			act_puts("$t is now realiased to '$T'.",
				 ch, arg, argument,
				 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
			return;
		}
	}

	if (pos >= MAX_ALIAS) {
		act_char("Sorry, you have reached the alias limit.", ch);
		return;
	}

	/*
	 * make a new alias
	 */
	d->dvdata->alias[pos]		= str_dup(arg);
	d->dvdata->alias_sub[pos]	= str_dup(argument);

	act_puts("$t is now aliased to '$T'.", ch, arg, argument,
		 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
}

DO_FUN(do_unalias, ch, argument)
{
	DESCRIPTOR_DATA *d;
	char arg[MAX_INPUT_LENGTH];
	int pos;
	bool found = FALSE;

	if ((d = ch->desc) == NULL)
		return;

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg == '\0') {
		act_char("Unalias what?", ch);
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
			act_char("Alias removed.", ch);
			free_string(d->dvdata->alias[pos]);
			free_string(d->dvdata->alias_sub[pos]);
			d->dvdata->alias[pos] = NULL;
			d->dvdata->alias_sub[pos] = NULL;
			found = TRUE;
		}
	}

	if (!found)
		act_char("No alias of that name to remove.", ch);
}

DO_FUN(do_last, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];
	const char *qname = NULL;
	msgq_t *msgq = NULL;
	clan_t *clan;
	const char *msg;
	bool found = FALSE;

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		do_help(ch, "LAST");
		return;
	}

	if (!str_prefix(arg, "say") && !IS_NPC(ch)) {
		qname = "says";
		msgq = &PC(ch)->msgq_say;
	} else if (!str_prefix(arg, "tell") && !IS_NPC(ch)) {
		qname = "tells";
		msgq = &PC(ch)->msgq_tell;
	} else if ((!str_prefix(arg, "group") ||
		    !str_prefix(arg, "gtell")) && !IS_NPC(ch)) {
		qname = "group tells";
		msgq = &PC(ch)->msgq_group;
	} else if (!str_prefix(arg, "clan") &&
		   (clan = clan_lookup(ch->clan)) != NULL) {
		qname = "clan talks";
		msgq = &clan->msgq_clan;
	} else if ((!str_prefix(arg, "sog") ||
		    !str_prefix(arg, "ooc")) && !IS_NPC(ch)) {
		qname = "OOC talks";
		msgq = &PC(ch)->msgq_sog;
	} else if ((!str_prefix(arg, "shout") ||
		    !str_prefix(arg, "gossip") ||
		    !str_prefix(arg, "channels")) && !IS_NPC(ch)) {
		qname = "shouts and gossips";
		msgq = &PC(ch)->msgq_chan;
	} else if (!str_prefix(arg, "immtalk") && IS_IMMORTAL(ch)) {
		qname = "imm talks";
		msgq = &msgq_immtalk;
	}

	if (msgq == NULL) {
		do_last(ch, str_empty);
		return;
	}

	for (msg = msgq_first(msgq); msg != NULL; msg = msgq_next(msgq)) {
		if (!found) {
			act_puts("Last $t were:",
				 ch, qname, NULL, TO_CHAR, POS_DEAD);
			found = TRUE;
		}

		send_to_char(msg, ch);
	}

	if (!found)
		act_puts("Nothing here.", ch, NULL, NULL, TO_CHAR, POS_DEAD);
}

/*
 * alphabetize these table by name if you are adding new entries
 */
toggle_t toggle_table[] =
{
	{ "affects",						// notrans
	  "show affects in score",
	  NULL, comm_flags,	COMM_SHOWAFF,
	  "Affects will now be shown in score.",
	  "Affects will no longer be shown in score."
	},

	{ "brief",						// notrans
	  "brief descriptions",
	  NULL, comm_flags,	COMM_BRIEF,
	  "Short descriptions activated.",
	  "Full descriptions activated."
	},

	{ "color",						// notrans
	  "ANSI colors",
	  NULL, comm_flags,	COMM_COLOR,
	  "{BC{Ro{Yl{Co{Gr{x is now {RON{x, Way Cool!",
	  "Color is now OFF, *sigh*."
	},

	{ "compact",						// notrans
	  "compact mode",
	  NULL, comm_flags,	COMM_COMPACT,
	  "Compact mode on.",
	  "Compact mode off."
	},

	{ "combine",						// notrans
	  "combined items in inventory list",
	  NULL, comm_flags,	COMM_COMBINE,
	  "Combined inventory selected.",
	  "Long inventory selected."
	},

	{ "long flags",						// notrans
	  "long flags mode",
	  NULL, comm_flags,	COMM_LONG,
	  "Long flags mode on.",
	  "Long flags mode off."
	},

	{ "nobust",						// notrans
	  "do not bust prompt if hp/mana/move changed mode",
	  NULL, comm_flags,	COMM_NOBUST,
	  "Do not bust prompt if hp/mana/move changed mode on.",
	  "Do not bust prompt if hp/mana/move changed mode off."
	},

	{ "noeng",						// notrans
	  "do not display english obj/mob names",
	  NULL, comm_flags,	COMM_NOENG,
	  "You will not see english obj/mob names anymore.",
	  "You will now see english obj/mob names."
	},

	{ "noflee",						// notrans
	  "do not flee from combat in lost-link",
	  NULL, comm_flags,	COMM_NOFLEE,
	  "You will not flee automagically from combat in lost-link anymore.",
	  "You will flee automagically from combat in lost-link."
	},

	{ "notelnet",						// notrans
	  "no telnet parser",
	  NULL, comm_flags,	COMM_NOTELNET,
	  "Telnet parser is OFF.",
	  "Telnet parser is ON.",
	},

	{ "noiac",						// notrans
	  "no IACs in output",
	  NULL, comm_flags,	COMM_NOIAC,
	  "IACs will not be sent to you anymore.",
	  "Text will be sent to you unmodified.",
	},

	{ "noverbose",						// notrans
	  "no verbose messages",
	  NULL, comm_flags,	COMM_NOVERBOSE,
	  "You will no longer see verbose messages.",
	  "Now you will see verbose messages."
	},

	{ "prompt",						// notrans
	  "show prompt",
	  NULL, comm_flags,	COMM_PROMPT,
	  "You will now see prompts.",
	  "You will no longer see prompts."
	},

	{ "telnet GA",						// notrans
	  "send IAC GA (goahead) after each prompt",
	  NULL, comm_flags,	COMM_TELNET_GA,
	  "IAC GA will be sent after each prompt.",
	  "IAC GA will not be sent after prompts.",
	},

	{ "quiet edit",						// notrans
	  "quiet mode in string editor",
	  NULL, comm_flags,	COMM_QUIET_EDITOR,
	  "Quiet mode on.",
	  "Quiet mode off."
	},

	{ "show race",						// notrans
	  "show race in long desc",
	  NULL, comm_flags,	COMM_SHOWRACE,
	  "You will now see race in long desc.",
	  "You will no longer see race in long desc.",
	},

	{ "trans mode",						// notrans
	  "OLC translation mode",
	  "create edit ashow", olc_flags,	OLC_MODE_TRANS,	// notrans
	  "OLC translation mode on.",
	  "OLC translation mode off."
	},

	{ "raw strings",					// notrans
	  "raw strings mode",
	  "create edit ashow stat", olc_flags,	OLC_MODE_RAW,	// notrans
	  "Raw string mode on.",
	  "Raw string mode off."
	},

	{ "short equipment",					// notrans
	  "short equipment mode",
	  NULL, comm_flags,	COMM_SHORT_EQ,
	  "You will no longer see empty equipment slots.",
	  "You will now see empty equipment slots."
	},

	{ "short aff",						// notrans
	  "short affects mode",
	  NULL, comm_flags,	COMM_SHORT_AFF,
	  "Short affects mode on.",
	  "Short affects mode off."
	},

	{ "battle",						// notrans
	  "do not show battle prompt",
	  NULL, comm_flags,	COMM_NO_BATTLE_PROMPT,
	  "You will no longer see battle prompt when fighting.",
	  "You will now see battle prompt when fighting.",
	},

	{ "mudftp",						// notrans
	  "mudFTP editor mode",
	  "create edit", comm_flags, COMM_MUDFTP_EDITOR,
	  "mudFTP editor mode on.",
	  "mudFTP editor mode off."
	},

	{ NULL, NULL, NULL, NULL, 0, NULL, NULL }
};

DO_FUN(do_toggle, ch, argument)
{
	if (argument[0] == '\0') {
		act_char("Your current settings are:", ch);
		print_toggles(ch, toggle_table);
		return;
	}

	toggle(ch, argument, toggle_table);
}

DO_FUN(do_ooc, ch, argument)
{
	DESCRIPTOR_DATA *d;

	if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
		return;

        if (IS_SET(ch->in_room->room_flags, ROOM_SILENT)
        &&  !IS_IMMORTAL(ch)) {
                act_char("You are in silent room.", ch);
                return;
        }

	if (argument[0] == '\0') {
		TOGGLE_BIT(ch->chan, CHAN_NOOOC);
		if (IS_SET(ch->chan, CHAN_NOOOC)) {
			act_char("You will no longer hear general Shades of Gray line.", ch);
		} else {
			act_char("You will now hear general Shades of Gray line.", ch);
		}
		return;
	}

	if (IS_SET(ch->chan, CHAN_NOCHANNELS)) {
		 act_char("The gods have revoked your channel privileges.", ch);
		 return;
	}

	if (IS_SET(ch->chan, CHAN_NOOOC))
		do_ooc(ch, str_empty);
	WAIT_STATE(ch, get_pulse("violence"));

	argument = garble(ch, argument);
	act_puts("[Shades of Gray] $lu{$n}: {m$t{x",
		 ch, argument, NULL,
		 TO_CHAR | ACTQ_SOG | (ACT_SPEECH(ch) & ~ACT_STRANS), POS_DEAD);

	for (d = descriptor_list; d; d = d->next) {
		if (d->connected == CON_PLAYING
		&&  d->character != ch
		&&  !IS_SET(d->character->chan, CHAN_NOOOC)
		&&  (!IS_SET(d->character->in_room->room_flags, ROOM_SILENT) ||
		     IS_IMMORTAL(d->character))) {
			act_puts("[Shades of Gray] $lu{$n}: {m$t{x",
				 ch, argument, d->character,
				 TO_VICT | ACTQ_SOG | ACT_NOTWIT |
				 (ACT_SPEECH(ch) & ~ACT_STRANS),
				 POS_DEAD);
		}
	}
}

DO_FUN(do_oocme, ch, argument)
{
	DESCRIPTOR_DATA *d;

	if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
		return;

        if (IS_SET(ch->in_room->room_flags, ROOM_SILENT)
        &&  !IS_IMMORTAL(ch)) {
                act_char("You are in silent room.", ch);
                return;
        }

	if (argument[0] == '\0')
		return;

	if (IS_SET(ch->chan, CHAN_NOCHANNELS)) {
		 act_char("The gods have revoked your channel privileges.", ch);
		 return;
	}

	WAIT_STATE(ch, get_pulse("violence"));

	if (is_sn_affected(ch, "garble"))
		return;

	act_puts("[Shades of Gray] {m$lu{$n} $t{x",
		 ch, argument, NULL,
		 TO_CHAR | ACTQ_SOG | (ACT_SPEECH(ch) & ~ACT_STRANS), POS_DEAD);

	for (d = descriptor_list; d; d = d->next) {
		if (d->connected == CON_PLAYING
		&&  d->character != ch
		&&  !IS_SET(d->character->chan, CHAN_NOOOC)
		&&  (!IS_SET(d->character->in_room->room_flags, ROOM_SILENT) ||
		     IS_IMMORTAL(d->character))) {
			act_puts("[Shades of Gray] {m$lu{$n} $t{x",
				 ch, argument, d->character,
				 TO_VICT | ACTQ_SOG | ACT_NOTWIT |
				 (ACT_SPEECH(ch) & ~ACT_STRANS),
				 POS_DEAD);
		}
	}
}

DO_FUN(do_retell, ch, argument)
{
	if (IS_NPC(ch)) {
		act_char("Huh?", ch);
		return;
	}

	tell_char(ch, PC(ch)->retell, argument);
}
