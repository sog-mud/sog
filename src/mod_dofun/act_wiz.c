/*
 * $Id: act_wiz.c,v 1.200 1999-11-23 16:04:53 fjoe Exp $
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
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#if !defined (WIN32)
#include <unistd.h>
#endif
#include <limits.h>

#if defined(BSD44)
#	include <fnmatch.h>
#else
#	include "../compat/fnmatch.h"
#endif

#include "merc.h"
#include "quest.h"
#include "obj_prog.h"
#include "fight.h"
#include "quest.h"
#include "chquest.h"
#include "cmd.h"
#include "db.h"
#include "ban.h"
#include "socials.h"
#include "mob_prog.h"

/* command procedures needed */
DECLARE_DO_FUN(do_rstat	);
DECLARE_DO_FUN(do_mstat	);
DECLARE_DO_FUN(do_dstat	);
DECLARE_DO_FUN(do_ostat	);
DECLARE_DO_FUN(do_msgstat);
DECLARE_DO_FUN(do_mpstat);
DECLARE_DO_FUN(do_rset	);
DECLARE_DO_FUN(do_mset	);
DECLARE_DO_FUN(do_oset	);
DECLARE_DO_FUN(do_sset	);
DECLARE_DO_FUN(do_mfind	);
DECLARE_DO_FUN(do_ofind	);
DECLARE_DO_FUN(do_mload	);
DECLARE_DO_FUN(do_oload	);
DECLARE_DO_FUN(do_save	);
DECLARE_DO_FUN(do_look	);
DECLARE_DO_FUN(do_stand	);
DECLARE_DO_FUN(do_help	);
DECLARE_DO_FUN(do_replay);

void do_objlist(CHAR_DATA *ch, const char *argument)
{
	FILE *fp;
	OBJ_DATA *obj;
	BUFFER *buf;

	if ((fp = dfopen(TMP_PATH, "objlist.txt", "w+")) == NULL) {
	 	char_puts("File error.\n", ch);
	 	return;
	}

	buf = buf_new(-1);
	for(obj = object_list; obj != NULL; obj = obj->next) {
		if (obj->pObjIndex->affected == NULL)
			continue;

		buf_clear(buf);
		buf_printf(buf, "\n#Obj: %s (Vnum : %d) \n",
			   mlstr_mval(&obj->short_descr),
			   obj->pObjIndex->vnum);
		format_obj(buf, obj);
		if (!IS_SET(obj->extra_flags, ITEM_ENCHANTED))
			format_obj_affects(buf, obj->pObjIndex->affected,
					   FOA_F_NODURATION);
		format_obj_affects(buf, obj->affected, 0);
		fprintf(fp, "%s", fix_string(buf_string(buf)));
	}
	buf_free(buf);
	fclose(fp);
}

void do_limited(CHAR_DATA *ch, const char *argument)
{
	OBJ_DATA *obj;
	OBJ_INDEX_DATA *obj_index;
	int	lCount = 0;
	int	ingameCount;
	int 	nMatch;
	int 	vnum;

	if (argument[0] != '\0')  {
	obj_index = get_obj_index(atoi(argument));
	if (obj_index == NULL)  {
	  char_puts("Not found.\n", ch);
	  return;
	}
	if (obj_index->limit == -1)  {
	  char_puts("Thats not a limited item.\n", ch);
	  return;
	}
	nMatch = 0;
	      char_printf(ch, "%-35s [%5d]  Limit: %3d  Current: %3d\n", 
			   mlstr_mval(&obj_index->short_descr), 
			   obj_index->vnum,
		           obj_index->limit, 
			   obj_index->count);
		  ingameCount = 0;
		  for (obj=object_list; obj != NULL; obj=obj->next)
		    if (obj->pObjIndex->vnum == obj_index->vnum)  {
		      ingameCount++;
		      if (obj->carried_by != NULL) 
			char_printf(ch, "Carried by %-30s\n", obj->carried_by->name);
		      else if (obj->in_room != NULL) 
			char_printf(ch, "At %-20s [%d]\n",
				mlstr_cval(&obj->in_room->name, ch),
				obj->in_room->vnum);
		      else if (obj->in_obj != NULL) 
			char_printf(ch, "In %-20s [%d] \n",
				mlstr_mval(&obj->in_obj->short_descr),
				obj->in_obj->pObjIndex->vnum);
		    }
		    char_printf(ch, "  %d found in game. %d should be in pFiles.\n", 
				ingameCount, obj_index->count-ingameCount);
		return;
	}

	nMatch = 0;
	for (vnum = 0; nMatch < top_obj_index; vnum++)
	  if ((obj_index = get_obj_index(vnum)) != NULL)
	  {
	    nMatch++;
		if (obj_index->limit != -1)  {
		  lCount++;
	      char_printf(ch, "%-37s [%5d]  Limit: %3d  Current: %3d\n", 
			   mlstr_mval(&obj_index->short_descr), 
			   obj_index->vnum,
		           obj_index->limit, 
			   obj_index->count);
		}
	  }
	char_printf(ch, "\n%d of %d objects are limited.\n", lCount, nMatch);
}

void do_wiznet(CHAR_DATA *ch, const char *argument)
{
	int flag;
	CHAR_DATA *vch;

	if (ch->desc == NULL)
		return;

	vch = GET_ORIGINAL(ch);

	if (argument[0] == '\0') {
		/* show wiznet status */
		BUFFER *output;

		output = buf_new(-1);
		buf_printf(output, "Wiznet status: %s\n",
			   IS_SET(PC(vch)->wiznet, WIZ_ON) ? "ON" : "OFF");

		buf_add(output, "\nchannel    | status");
		buf_add(output, "\n-----------|-------\n");
		for (flag = 0; wiznet_table[flag].name != NULL; flag++)
			buf_printf(output, "%-11s|  %s\n",
				   wiznet_table[flag].name,
				   wiznet_table[flag].level > vch->level ?
				   "N/A" :
				   IS_SET(PC(vch)->wiznet,
					  wiznet_table[flag].flag) ?
				   "ON" : "OFF");
		page_to_char(buf_string(output), ch);
		buf_free(output);
		return;
	}

	if (!str_prefix(argument, "on")) {
		char_puts("Welcome to Wiznet!\n", ch);
		SET_BIT(PC(vch)->wiznet, WIZ_ON);
		return;
	}

	if (!str_prefix(argument, "off")) {
		char_puts("Signing off of Wiznet.\n", ch);
		REMOVE_BIT(PC(vch)->wiznet, WIZ_ON);
		return;
	}

	flag = wiznet_lookup(argument);
	if (flag == -1 || vch->level < wiznet_table[flag].level) {
		char_puts("No such option.\n", ch);
		return;
	}
	 
	TOGGLE_BIT(PC(vch)->wiznet, wiznet_table[flag].flag);
	if (!IS_SET(PC(vch)->wiznet, wiznet_table[flag].flag))
		char_printf(ch,"You will no longer see %s on wiznet.\n",
		        wiznet_table[flag].name);
	else
		char_printf(ch, "You will now see %s on wiznet.\n",
			    wiznet_table[flag].name);
}

void do_tick(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	
	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0')  {
		do_help(ch,"'WIZ TICK'");
		return;
	}

	if (!str_prefix(arg, "area")) {
		area_update();
		char_puts("Area updated.\n", ch);
		return;
	}

	if (!str_prefix(arg, "char player")) {
		char_update();
		char_puts("Players updated.\n", ch);
		return;
	}

	if (!str_prefix(arg, "room")) {
		room_update();
		char_puts("Room updated.\n", ch);
		return;
	}

	if (!str_prefix(arg, "track")) {
		track_update();
		char_puts("Tracks updated.\n", ch);
		return;
	}

	if (!str_prefix(arg, "obj")) {
		obj_update();
		char_puts("Objects updated.\n", ch);
		return;
	}

	if (!str_prefix(arg, "chquest")) {
		chquest_update();
		char_puts("Challenge quests updated.\n", ch);
		return;
	}

	if (!str_prefix(arg, "quest")) {
		quest_update();
		char_puts("Auto-quests updated.\n", ch);
		return;
	}

	if (!str_prefix(arg, "clan")) {
		hash_foreach(&clans, clan_item_update_cb);
		char_puts("Clan item location updated.\n", ch);
		return;
	}

	if (!str_prefix(arg, "weather")) {
		weather_update();
		char_puts("Weather updated.\n", ch);
		return;
	}

	do_tick(ch, str_empty);
}

void do_nonote(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		do_help(ch, "'WIZ NONOTE'");
		return;
	}

	if ((victim = get_char_world(ch, arg)) == NULL) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	if (!IS_TRUSTED(ch, trust_level(victim))) {
		char_puts("You failed.\n", ch);
		return;
	}

	if (IS_SET(victim->comm, COMM_NONOTE)) {
		REMOVE_BIT(victim->comm, COMM_NONOTE);
		char_puts("You may write notes again.\n", victim);
		char_puts("NONOTE removed.\n", ch);
		wiznet("$N grants $i right to write notes",
			ch, victim, WIZ_PENALTIES, WIZ_SECURE, 0);
	}
	else {
		SET_BIT(victim->comm, COMM_NONOTE);
		char_puts("Your notes will be sent to Abyss now.\n", victim);
		char_puts("NONOTE set.\n", ch);
		wiznet("$N revokes $i's right to write notes",
			ch, victim, WIZ_PENALTIES, WIZ_SECURE, 0);
	}
}

/* RT nochannels command, for those spammers */
void do_nochannels(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	
	one_argument(argument, arg, sizeof(arg));
	
	if (arg[0] == '\0') {
	    do_help(ch, "'WIZ NOCHANNEL'");
	    return;
	}
	
	if ((victim = get_char_world(ch, arg)) == NULL) {
		char_puts("They aren't here.\n", ch);
		return;
	}
	
	if (!IS_TRUSTED(ch, trust_level(victim))) {
		char_puts("You failed.\n", ch);
		return;
	}
	
	if (IS_SET(victim->comm, COMM_NOCHANNELS)) {
	    REMOVE_BIT(victim->comm, COMM_NOCHANNELS);
	    char_puts("The gods have restored your channel priviliges.\n", 
			      victim);
	    char_puts("NOCHANNELS removed.\n", ch);
		wiznet("$N restores channels to $i",
			ch, victim, WIZ_PENALTIES, WIZ_SECURE, 0);
	} else {
	    SET_BIT(victim->comm, COMM_NOCHANNELS);
	    char_puts("The gods have revoked your channel priviliges.\n", 
			       victim);
	    char_puts("NOCHANNELS set.\n", ch);
		wiznet("$N revokes $i's channels.",
			ch, victim, WIZ_PENALTIES, WIZ_SECURE, 0);
	}
}

void do_smote(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *vch;
	const char *letter, *name;
	char last[MAX_INPUT_LENGTH], temp[MAX_STRING_LENGTH];
	int matches = 0;

	if (!IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE)) {
	    char_puts("You can't show your emotions.\n", ch);
	    return;
	}

	if (argument[0] == '\0') {
	    do_help(ch, "'WIZ SMOTE'");
	    return;
	}

	if (strstr(argument,ch->name) == NULL) {
		char_puts("You must include your name in an smote.\n",ch);
		return;
	}

	char_printf(ch, "%s\n", argument);

	for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room) {
		if (vch->desc == NULL || vch == ch)
			continue;

		if ((letter = strstr(argument,vch->name)) == NULL) {
			char_printf(ch, "%s\n", argument);
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
		char_printf(ch, "%s\n", temp);
	}
}	

void do_bamfin(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *vch = GET_ORIGINAL(ch);

	if (IS_NPC(vch))
		return;

	if (argument[0] == '\0') {
		char_printf(ch, "Your poofin is '%s'\n", PC(vch)->bamfin);
		return;
	}

	if (strstr(argument, vch->name) == NULL) {
		char_puts("You must include your name.\n", ch);
		return;
	}
		     
	free_string(PC(vch)->bamfin);
	PC(vch)->bamfin = str_dup(argument);

	char_printf(ch, "Your poofin is now '%s'\n", PC(vch)->bamfin);
}

void do_bamfout(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *vch = GET_ORIGINAL(ch);

	if (IS_NPC(vch))
		return;

	if (argument[0] == '\0') {
		char_printf(ch, "Your poofout is '%s'\n", PC(ch)->bamfout);
		return;
	}
	
	if (strstr(argument, vch->name) == NULL) {
		char_puts("You must include your name.\n", ch);
		return;
	}
	
	free_string(PC(vch)->bamfout);
	PC(vch)->bamfout = str_dup(argument);
	
	char_printf(ch, "Your poofout is now '%s'\n", PC(vch)->bamfout);
}

void do_disconnect(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	DESCRIPTOR_DATA *d;
	CHAR_DATA *victim;

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		do_help(ch, "'WIZ DISCONNECT'");
		return;
	}

	if (is_number(arg)) {
		int desc;

		desc = atoi(arg);
		for (d = descriptor_list; d != NULL; d = d->next) {
		        if (d->descriptor == desc) {
		        	close_descriptor(d, SAVE_F_NORMAL);
	        		char_puts("Ok.\n", ch);
	        		return;
	        	}
		}
	}

	if ((victim = get_char_world(ch, arg)) == NULL) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	if (victim->desc == NULL) {
		act("$N doesn't have a descriptor.", ch, NULL, victim, TO_CHAR);
		return;
	}

	for (d = descriptor_list; d != NULL; d = d->next)
		if (d == victim->desc) {
			close_descriptor(d, SAVE_F_NORMAL);
			char_puts("Ok.\n", ch);
			return;
		}

	bug("Do_disconnect: desc not found.", 0);
	char_puts("Descriptor not found!\n", ch);
	return;
}

void do_echo(CHAR_DATA *ch, const char *argument)
{
	DESCRIPTOR_DATA *d;
	
	if (argument[0] == '\0') {
		do_help(ch, "'WIZ GECHO'");
		return;
	}
	
	for (d = descriptor_list; d; d = d->next)
		if (d->connected == CON_PLAYING) {
			if (IS_TRUSTED(d->character, trust_level(ch))) {
				act("{W$N:global>{x ",
					d->character, NULL, ch,
					TO_CHAR | ACT_NOLF);
			}
			char_printf(d->character, "%s\n", argument);
		}
}

void do_recho(CHAR_DATA *ch, const char *argument)
{
	DESCRIPTOR_DATA *d;
	
	if (argument[0] == '\0') {
		do_help(ch, "'WIZ ECHO'");
		return;
	}

	for (d = descriptor_list; d; d = d->next)
		if (d->connected == CON_PLAYING
		&&   d->character->in_room == ch->in_room) {
			if (IS_TRUSTED(d->character, trust_level(ch))) {
				act("{W$N:local>{x ",
					d->character, NULL, ch,
					TO_CHAR | ACT_NOLF);
			}
				char_puts("local> ",d->character);
			char_printf(d->character, "%s\n", argument);
		}
}

void do_zecho(CHAR_DATA *ch, const char *argument)
{
	DESCRIPTOR_DATA *d;

	if (argument[0] == '\0') {
		do_help(ch, "'WIZ ZECHO'");
		return;
	}

	for (d = descriptor_list; d; d = d->next)
		if (d->connected == CON_PLAYING
		&&  d->character->in_room != NULL && ch->in_room != NULL
		&&  d->character->in_room->area == ch->in_room->area) {
			if (IS_TRUSTED(d->character, trust_level(ch))) {
				act("{W$N:zone>{x ",
					d->character, NULL, ch,
					TO_CHAR | ACT_NOLF);
			}
			char_printf(d->character, "%s\n", argument);
		}
}

void do_pecho(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	argument = one_argument(argument, arg, sizeof(arg));
	
	if (argument[0] == '\0' || arg[0] == '\0') {
		do_help(ch, "'WIZ PECHO'");
		return;
	}
	 
	if  ((victim = get_char_world(ch, arg)) == NULL) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	if (IS_TRUSTED(victim, trust_level(ch)))
		act("{W$N:personal>{x ", victim, NULL, ch, TO_CHAR | ACT_NOLF);

	char_printf(victim, "%s\n", argument);
	char_printf(ch, "personal> %s\n", argument);
}

void do_transfer(CHAR_DATA *ch, const char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	ROOM_INDEX_DATA *location;
	DESCRIPTOR_DATA *d;
	CHAR_DATA *victim;

	argument = one_argument(argument, arg1, sizeof(arg1));
	argument = one_argument(argument, arg2, sizeof(arg2));

	if (arg1[0] == '\0') {
		do_help(ch, "'WIZ TRANSFER'");
		return;
	}

	if (!str_cmp(arg1, "all")) {
		for (d = descriptor_list; d != NULL; d = d->next)
			if (d->connected == CON_PLAYING
			&&  d->character != ch
			&&  d->character->in_room != NULL
			&&  can_see(ch, d->character)) {
				dofun("transfer", ch,
				      "%s %s", d->character->name, arg2);
		}
		return;
	}

	/*
	 * Thanks to Grodyn for the optional location parameter.
	 */
	if (arg2[0]) {
		if ((location = find_location(ch, arg2)) == NULL) {
		    char_puts("No such location.\n", ch);
		    return;
		}

		if (room_is_private(location) && ch->level < MAX_LEVEL) {
		    char_puts("That room is private right now.\n", ch);
		    return;
		}
	} else
		location = ch->in_room;

	if ((victim = get_char_world(ch, arg1)) == NULL) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	if (victim->in_room == NULL) {
		char_puts("They are in limbo.\n", ch);
		return;
	}

	if (!IS_TRUSTED(ch, trust_level(victim))) {
		char_puts("You failed.\n", ch);
		return;
	}

	if (victim->fighting != NULL)
		stop_fighting(victim, TRUE);

	act("$n disappears in a mushroom cloud.", victim, NULL, NULL, TO_ROOM);
	if (ch != victim)
		act("$n has transferred you.", ch, NULL, victim, TO_VICT);
	char_puts("Ok.\n", ch);

	char_from_room(victim);
	act("$N arrives from a puff of smoke.",
	    location->people, NULL, victim, TO_ALL);

	char_to_room(victim, location);
	if (IS_EXTRACTED(victim))
		return;

	do_look(victim, "auto");
}

void do_at(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	ROOM_INDEX_DATA *location;
	ROOM_INDEX_DATA *original;
	OBJ_DATA *on;
	
	argument = one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0' || argument[0] == '\0') {
		do_help(ch, "'WIZ AT'");
		return;                   
	}

	if ((location = find_location(ch, arg)) == NULL) {
		char_puts("No such location.\n", ch);
		return;
	}

	original = ch->in_room;
	on = ch->on;
	char_from_room(ch);

	char_to_room(ch, location);
	if (IS_EXTRACTED(ch))
		return;

	interpret(ch, argument);

	/* handle 'at xxx quit' */
	if (IS_EXTRACTED(ch))
		return;

	char_from_room(ch);
	char_to_room(ch, original);
	if (IS_EXTRACTED(ch))
		return;
	ch->on = on;
}

void do_goto(CHAR_DATA *ch, const char *argument)
{
	ROOM_INDEX_DATA *location;
	CHAR_DATA *rch;
	CHAR_DATA *pet = NULL;

	if (argument[0] == '\0') {
		do_help(ch, "'WIZ GOTO'");
		return;         
	}

	if ((location = find_location(ch, argument)) == NULL) {
		char_puts("No such location.\n", ch);
		return;
	}

	if (!IS_TRUSTED(ch, LEVEL_IMMORTAL)) {
		if (ch->fighting) {
			char_puts("No way! You are fighting.\n", ch);
			return;
		}

		if (IS_PUMPED(ch)) {
			act_puts("You are too pumped to pray now.",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
			return;
		}
#if 0
		if (!IS_SET(ch->in_room->room_flags, ROOM_PEACE)) {
			char_puts("You must be in a safe place in order "
				  "to make a transportation.\n", ch);
			return;
		}
#endif
		if (!IS_BUILDER(ch, location->area)
		||  !IS_BUILDER(ch, ch->in_room->area)) {
			char_puts("You cannot transfer yourself there.\n", ch);
			return;
		}
	}

	if (ch->fighting != NULL)
		stop_fighting(ch, TRUE);

	for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room) {
		if (IS_TRUSTED(rch, ch->invis_level)) {
			if (!IS_NPC(ch)
			&&  PC(ch)->bamfout[0] != '\0') {
				act("$t", ch, PC(ch)->bamfout,
				    rch, TO_VICT);
			} else {
				act("$n leaves in a swirling mist.", ch, NULL,
				    rch, TO_VICT);
			}
		}
	}

	pet = GET_PET(ch);
	char_from_room(ch);

	for (rch = location->people; rch; rch = rch->next_in_room) {
		if (IS_TRUSTED(rch, ch->invis_level)) {
			if (!IS_NPC(ch) && PC(ch)->bamfin[0]) {
				act("$t",
				    rch, PC(ch)->bamfin, NULL, TO_CHAR);
			} else {
				act("$N appears in a swirling mist.",
				    rch, NULL, ch, TO_CHAR);
			}
		}
	}

	char_to_room(ch, location);
	if (IS_EXTRACTED(ch))
		return;
	do_look(ch, "auto");

	if (pet && !IS_AFFECTED(pet, AFF_SLEEP)) {
		if (pet->position != POS_STANDING)
			do_stand(pet, str_empty);
		char_from_room(pet);
		char_to_room(pet, location);
	}
}

/* RT to replace the 3 stat commands */

void do_stat(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	ROOM_INDEX_DATA *location;
	CHAR_DATA *victim;
	const char *string;

	string = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		do_help(ch, "'WIZ STAT'");
		return;
	}

	if (!str_cmp(arg, "room")) {
		do_rstat(ch, string);
		return;
	}
	
	if (!str_cmp(arg, "obj")) {
		do_ostat(ch, string);
		return;
	}

	if (!str_cmp(arg, "char") || !str_cmp(arg, "mob")) {
		do_mstat(ch, string);
		return;
	}
	 
	if (!str_cmp(arg, "desc")) {
		do_dstat(ch, string);
		return;
	}

	if (!str_cmp(arg, "mp")) {
		do_mpstat(ch, string);
		return;
	}

	if (!str_cmp(arg, "msg")) {
		do_msgstat(ch, string);
		return;
	}

	/* do it the old way */

	obj = get_obj_world(ch, argument);
	if (obj != NULL) {
		do_ostat(ch, argument);
	 	return;
	}

	victim = get_char_world(ch, argument);
	if (victim != NULL) {
		do_mstat(ch, argument);
		return;
	}

	location = find_location(ch, argument);
	if (location != NULL) {
		do_rstat(ch, argument);
		return;
	}

	char_puts("Nothing by that name found anywhere.\n",ch);
}

void do_rstat(CHAR_DATA *ch, const char *argument)
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	ROOM_INDEX_DATA *location;
	ROOM_HISTORY_DATA *rh;
	OBJ_DATA *obj;
	CHAR_DATA *rch;
	BUFFER *output;
	int door;

	one_argument(argument, arg, sizeof(arg));
	location = (arg[0] == '\0') ? ch->in_room : find_location(ch, arg);
	if (location == NULL) {
		char_puts("No such location.\n", ch);
		return;
	}

	if (ch->in_room != location 
	&&  room_is_private(location) && !IS_TRUSTED(ch, LEVEL_IMP)) {
		char_puts("That room is private right now.\n", ch);
		return;
	}

	output = buf_new(-1);

	if (ch->in_room->affected_by)
		buf_printf(output, "Affected by %s\n", 
			   flag_string(raffect_flags, ch->in_room->affected_by));

	if (ch->in_room->room_flags)
		buf_printf(output, "Room Flags %s\n", 
			   flag_string(room_flags, ch->in_room->room_flags));

	mlstr_dump(output, "Name: ", &location->name);
	buf_printf(output, "Area: '%s'\n", location->area->name);

	buf_printf(output,
		   "Vnum: %d  Sector: %d  Light: %d  Healing: %d  Mana: %d\n",
		   location->vnum,
		   location->sector_type,
		   location->light,
		   location->heal_rate,
		   location->mana_rate);

	buf_printf(output, "Room flags: [%s].\n",
		   flag_string(room_flags, location->room_flags));
	buf_add(output, "Description:\n");
	mlstr_dump(output, str_empty, &location->description);

	if (location->ed != NULL) {
		ED_DATA *ed;

		buf_add(output, "Extra description keywords: '");
		for (ed = location->ed; ed; ed = ed->next) {
			buf_add(output, ed->keyword);
			if (ed->next != NULL)
				buf_add(output, " ");
		}
		buf_add(output, "'.\n");
	}

	buf_add(output, "Characters:");
	for (rch = location->people; rch; rch = rch->next_in_room) {
		if (can_see(ch,rch)) {
		    buf_add(output, " ");
		    one_argument(rch->name, buf, sizeof(buf));
		    buf_add(output, buf);
		}
	}

	buf_add(output, ".\nObjects:   ");
	for (obj = location->contents; obj; obj = obj->next_content) {
		buf_add(output, " ");
		one_argument(obj->name, buf, sizeof(buf));
		buf_add(output, buf);
	}
	buf_add(output, ".\n");

	for (door = 0; door <= 5; door++) {
		EXIT_DATA *pexit;

		if ((pexit = location->exit[door]) != NULL) {
			buf_printf(output, "Door: %d.  To: %d.  Key: %d.  Exit flags: %d.\nKeyword: '%s'.\n",
				door,
				pexit->to_room.r == NULL ?
				-1 : pexit->to_room.r->vnum,
		    		pexit->key,
		    		pexit->exit_info,
		    		pexit->keyword);
			mlstr_dump(output, "Description: ",
				   &pexit->description);
		}
	}
	buf_add(output, "Tracks:\n");
	for (rh = location->history;rh != NULL;rh = rh->next)
		buf_printf(output,"%s took door %i.\n", rh->name, rh->went);

	send_to_char(buf_string(output), ch);
	buf_free(output);
}

void do_ostat(CHAR_DATA *ch, const char *argument)
{
	int i;
	BUFFER *output;
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		char_puts("Stat what?\n", ch);
		return;
	}

	if ((obj = get_obj_world(ch, argument)) == NULL) {
		char_puts("Nothing like that in hell, heaven or earth.\n",ch);
		return;
	}

	output = buf_new(-1);
	buf_printf(output, "Name(s): %s\n", obj->name);
	if (!mlstr_null(&obj->owner))
		buf_printf(output, "Owner: [%s]\n", mlstr_mval(&obj->owner));
	buf_printf(output, "Vnum: %d  Type: %s  Resets: %d\n",
		obj->pObjIndex->vnum,
		flag_string(item_types, obj->pObjIndex->item_type),
		obj->pObjIndex->reset_num);

	mlstr_dump(output, "Short description: ", &obj->short_descr);
	mlstr_dump(output, "Long description: ", &obj->description);

	buf_printf(output, "Wear bits: %s\n",
		   flag_string(wear_flags, obj->wear_flags));
	buf_printf(output, "Extra bits: %s\n",
		   flag_string(extra_flags, obj->extra_flags));
	buf_printf(output, "Number: %d/%d  Weight: %d/%d/%d (10th pounds)\n",
		1,           get_obj_number(obj),
		obj->weight, get_obj_weight(obj),get_true_weight(obj));

	buf_printf(output,
		  "Level: %d  Cost: %d  Condition: %d  Timer: %d Count: %d\n",
		  obj->level, obj->cost, obj->condition,
		  obj->timer, obj->pObjIndex->count);

	buf_printf(output,
		"In room: %d  In object: %s  Carried by: %s  Wear_loc: %d\n",
		obj->in_room    == NULL    ?        0 : obj->in_room->vnum,
		obj->in_obj     == NULL    ? "(none)" : mlstr_mval(&obj->in_obj->short_descr),
		obj->carried_by == NULL    ? "(none)" : 
		    can_see(ch,obj->carried_by) ? obj->carried_by->name
					 	: "someone",
		obj->wear_loc);
 	buf_printf(output,
		"Next: %s   Next_content: %s\n",
		obj->next	== NULL	   ? "(none)" : mlstr_mval(&obj->next->short_descr),
		obj->next_content == NULL  ? "(none)" : mlstr_mval(&obj->next_content->short_descr));
	buf_printf(output, "Values: %d %d %d %d %d\n",
		obj->value[0], obj->value[1], obj->value[2], obj->value[3],
		obj->value[4]);
	
	/* now give out vital statistics as per identify */
	
	switch (obj->pObjIndex->item_type) {
		int i;
		liquid_t *lq;
	case ITEM_SCROLL: 
	case ITEM_POTION:
	case ITEM_PILL:
		buf_printf(output, "Level %d spells of:", INT_VAL(obj->value[0]));

		for (i = 1; i < 5; i++)
			if (!IS_NULLSTR(obj->value[i].s)) 
				buf_printf(output, " '%s'", obj->value[i].s);
		buf_add(output, ".\n");
		break;

	case ITEM_WAND: 
	case ITEM_STAFF: 
		buf_printf(output, "Has %d(%d) charges of level %d",
			   INT_VAL(obj->value[1]), INT_VAL(obj->value[2]), INT_VAL(obj->value[0]));
	  
		if (!IS_NULLSTR(obj->value[3].s))
			buf_printf(output, " '%s'", obj->value[3].s);
		buf_add(output, ".\n");
		break;

	case ITEM_DRINK_CON:
		if ((lq = liquid_lookup(STR_VAL(obj->value[2]))) == NULL)
			break;
		buf_printf(output, "It holds %s-colored %s.\n",
			   lq->color,
			   lq->name);
		break;
	  
	case ITEM_WEAPON:
		buf_printf(output, "%s\n",
			   flag_string(weapon_class, INT_VAL(obj->value[0])));
		buf_printf(output,"Damage is %dd%d (average %d)\n",
			   INT_VAL(obj->value[1]), INT_VAL(obj->value[2]),
			   (1 + INT_VAL(obj->value[2])) * INT_VAL(obj->value[1]) / 2);
		buf_printf(output, "Damage noun is %s.\n",
			   damtype_noun(obj->value[3].s));
		    
		if (INT_VAL(obj->value[4]))  /* weapon flags */
		        buf_printf(output,"Weapons flags: %s\n",
				   flag_string(weapon_type2, INT_VAL(obj->value[4])));
		break;

	case ITEM_ARMOR:
		buf_printf(output, 
		    "Armor class is %d pierce, %d bash, %d slash, and %d vs. magic\n",
		        INT_VAL(obj->value[0]), INT_VAL(obj->value[1]), INT_VAL(obj->value[2]),
			INT_VAL(obj->value[3]));
		break;

	case ITEM_CONTAINER:
	        buf_printf(output,"Capacity: %d#  Maximum weight: %d#  flags: %s\n",
	        	   INT_VAL(obj->value[0]), INT_VAL(obj->value[3]),
			   flag_string(cont_flags, INT_VAL(obj->value[1])));
	        if (INT_VAL(obj->value[4]) != 100)
	        	buf_printf(output,"Weight multiplier: %d%%\n",
				   INT_VAL(obj->value[4]));
		break;
	}

	if (obj->ed) {
		ED_DATA *ed;

		buf_add(output, "Extra description keywords: '");

		for (ed = obj->ed; ed; ed = ed->next) {
			buf_add(output, ed->keyword);
			if (ed->next)
				buf_add(output, " ");
		}

		buf_add(output, "'\n");
	}

	if (obj->pObjIndex->ed) {
		ED_DATA *ed;

		buf_add(output, "pObjIndex extra description keywords: '");

		for (ed = obj->pObjIndex->ed; ed; ed = ed->next) {
			buf_add(output, ed->keyword);
			if (ed->next)
				buf_add(output, " ");
		}

		buf_add(output, "'\n");
	}

	if (!IS_SET(obj->extra_flags, ITEM_ENCHANTED))
		format_obj_affects(output, obj->pObjIndex->affected,
				   FOA_F_NODURATION);
	format_obj_affects(output, obj->affected, 0);

	if (obj->pObjIndex->oprogs) {
		buf_add(output, "Object progs:\n");
		for (i = 0; i < OPROG_MAX; i++)
			if (obj->pObjIndex->oprogs[i] != NULL)
				buf_printf(output, "%s: %s\n",
					 optype_table[i],
					 oprog_name_lookup(obj->pObjIndex->oprogs[i]));
	}

	buf_printf(output, "Damage condition: %d (%s)\n",
		   obj->condition, get_cond_alias(obj));
	varr_foreach(&obj->pObjIndex->restrictions, print_cc_ruleset_cb,
		     output, "obj", "Restrictions:\n");
	send_to_char(buf_string(output), ch);
	buf_free(output);
}

static void *
print_sa_cb(void *p, va_list ap)
{
	saff_t *sa = (saff_t *) p;

	BUFFER *buf = va_arg(ap, BUFFER *);

	buf_printf(buf, "        '%s' by %d",
		   sa->sn, sa->mod);
	if (!IS_NULLSTR(sa->type))
		buf_printf(buf, " (skill '%s')", sa->type);
	if (sa->bit) {
		buf_printf(buf, " with bits '%s'",
			   flag_string(sk_aff_flags, sa->bit));
	}
	buf_add(buf, "\n");
	return NULL;
}

void do_mstat(CHAR_DATA *ch, const char *argument)
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	AFFECT_DATA *paf;
	CHAR_DATA *victim;
	CHAR_DATA *pet;
	BUFFER *output;

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		char_puts("Stat whom?\n", ch);
		return;
	}

	if ((victim = get_char_room(ch, argument)) == NULL) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	output = buf_new(-1);

	buf_printf(output, "Name: [%s] Reset Zone: %s\n",
		   victim->name,
		   (IS_NPC(victim) && NPC(victim)->zone) ?
				NPC(victim)->zone->name : "?");

	buf_printf(output, 
		"Vnum: %d  Race: %s (%s)  Group: %d  Sex: %s  Room: %d\n",
		IS_NPC(victim) ? victim->pMobIndex->vnum : 0,
		victim->race, ORG_RACE(victim),
		IS_NPC(victim) ? victim->pMobIndex->group : 0,
		flag_string(sex_table, victim->sex),
		victim->in_room == NULL ? 0 : victim->in_room->vnum);

	if (IS_NPC(victim))
		buf_printf(output,"Count: %d  Killed: %d\n",
			victim->pMobIndex->count, victim->pMobIndex->killed);

	buf_printf(output, "Str: %d(%d)  Int: %d(%d)  Wis: %d(%d)  Dex: %d(%d)  Con: %d(%d) Cha: %d(%d)\n",
		victim->perm_stat[STAT_STR],
		get_curr_stat(victim,STAT_STR),
		victim->perm_stat[STAT_INT],
		get_curr_stat(victim,STAT_INT),
		victim->perm_stat[STAT_WIS],
		get_curr_stat(victim,STAT_WIS),
		victim->perm_stat[STAT_DEX],
		get_curr_stat(victim,STAT_DEX),
		victim->perm_stat[STAT_CON],
		get_curr_stat(victim,STAT_CON),
		victim->perm_stat[STAT_CHA],
		get_curr_stat(victim,STAT_CHA));


	buf_printf(output, "Hp: %d/%d  Mana: %d/%d  Move: %d/%d  Practices: %d\n",
		victim->hit,         victim->max_hit,
		victim->mana,        victim->max_mana,
		victim->move,        victim->max_move,
		IS_NPC(ch) ? 0 : PC(victim)->practice);
		
	if (IS_NPC(victim))
		snprintf(buf, sizeof(buf), "%d", victim->alignment);
	else  {
		buf_printf(output, "It belives the religion of %s.\n",
			religion_table[PC(victim)->religion].leader);

		snprintf(buf, sizeof(buf), "%s-%s", 
			 flag_string(ethos_table, victim->ethos),
			 flag_string(align_names, NALIGN(victim)));
	}

	buf_printf(output,
		"Lv: %d + %d  Class: %s  Align: %s  Gold: %ld  Silver: %ld  Exp: %d\n",
		victim->level,
		victim->add_level,
		victim->class,
		buf,
		victim->gold, victim->silver, GET_EXP(victim));

	buf_printf(output,"Armor: pierce: %d  bash: %d  slash: %d  magic: %d\n",
		    GET_AC(victim,AC_PIERCE), GET_AC(victim,AC_BASH),
		    GET_AC(victim,AC_SLASH),  GET_AC(victim,AC_EXOTIC));

	buf_printf(output, 
		"Hit: %d  Dam: %d  Saves: %d  Size: %s  Position: %s  Wimpy: %d\n",
		GET_HITROLL(victim), GET_DAMROLL(victim), victim->saving_throw,
		flag_string(size_table, victim->size),
		flag_string(position_table, victim->position),
		victim->wimpy);

	if (IS_NPC(victim)) {
		buf_printf(output, "Damage: %dd%d  Message:  %s\n",
			   NPC(victim)->dam.dice_number,
			   NPC(victim)->dam.dice_type,
			   damtype_noun(victim->damtype));
	}

	buf_printf(output, "Fighting: %s Deaths: %d Carry number: %d  Carry weight: %ld\n",
		   victim->fighting ? victim->fighting->name : "(none)" ,
		   IS_NPC(victim) ? 0 : PC(victim)->death,
		   victim->carry_number, get_carry_weight(victim) / 10);

	if (!IS_NPC(victim)) {
		buf_printf(output,
			   "Thirst: %d  Hunger: %d  Full: %d  "
			   "Drunk: %d Bloodlust: %d Desire: %d\n",
			   PC(victim)->condition[COND_THIRST],
			   PC(victim)->condition[COND_HUNGER],
			   PC(victim)->condition[COND_FULL],
			   PC(victim)->condition[COND_DRUNK],
			   PC(victim)->condition[COND_BLOODLUST],
			   PC(victim)->condition[COND_DESIRE]);
		buf_printf(output, "Age: %d  Played: %d  Timer: %d\n",
			   get_age(victim), get_hours(victim),
			   PC(victim)->idle_timer);

		buf_printf(output, "Plr: [%s]\n",
			   flag_string(plr_flags, PC(victim)->plr_flags));
	}
	
	if (victim->comm)
		buf_printf(output, "Comm: [%s]\n",
			   flag_string(comm_flags, victim->comm));

	if (IS_NPC(victim) && victim->pMobIndex->off_flags)
		buf_printf(output, "Offense: [%s]\n",
			   flag_string(off_flags,
				       victim->pMobIndex->off_flags));

	if (victim->imm_flags)
		buf_printf(output, "Immune: [%s]\n",
			   flag_string(imm_flags, victim->imm_flags));
	
	if (victim->res_flags)
		buf_printf(output, "Resist: [%s]\n",
			   flag_string(res_flags, victim->res_flags));

	if (victim->vuln_flags)
		buf_printf(output, "Vulnerable: [%s]\n",
			   flag_string(vuln_flags, victim->vuln_flags));

	buf_printf(output, "Form: [%s]\n",
		   flag_string(form_flags, victim->form));
	buf_printf(output, "Parts: [%s]\n",
		   flag_string(part_flags, victim->parts));

	if (victim->affected_by)
		buf_printf(output, "Affected by %s\n", 
			   flag_string(affect_flags, victim->affected_by));

	pet = GET_PET(victim);
	buf_printf(output, "Master: %s  Leader: %s  Pet: %s\n",
		victim->master	? victim->master->name	: "(none)",
		victim->leader	? victim->leader->name	: "(none)",
		pet		? pet->name		: "(none)");

	/* OLC */
	if (!IS_NPC(victim))
		buf_printf(output, "Security: %d.\n",
			   PC(victim)->security);

	mlstr_dump(output, "Short description: ", &victim->short_descr);
	if (IS_NPC(victim))
		mlstr_dump(output, "Long description: ", &victim->long_descr);

	if (IS_NPC(victim) && victim->pMobIndex->spec_fun != 0)
		buf_printf(output, "Mobile has special procedure %s.\n",
			   mob_spec_name(victim->pMobIndex->spec_fun));

	for (paf = victim->affected; paf != NULL; paf = paf->next) {
	    buf_printf(output, "Spell: '{c%s{x' ", paf->type);
	    buf_printf(output, "modifies '{c%s{x' by {c%d{x for {c%d{x hours ",
		paf->where == TO_SKILLS ?
		    STR_VAL(paf->location) :
		    SFLAGS_VAL(apply_flags, paf->location),
		paf->modifier,
		paf->duration);
		switch (paf->where) {
		    case TO_AFFECTS:
			buf_printf(output, "adding '{c%s{x' affect, ",
				flag_string(affect_flags, paf->bitvector));
			break;
		    case TO_IMMUNE:
			buf_printf(output, "adding '{c%s{x' immunity, ",
				flag_string(imm_flags, paf->bitvector));
			break;
		    case TO_RESIST:
			buf_printf(output, "adding '{c%s{x' resistance, ",
				flag_string(res_flags, paf->bitvector));
			break;
		    case TO_VULN:
			buf_printf(output, "adding '{c%s{x' vulnerability, ",
				flag_string(vuln_flags, paf->bitvector));
			break;
		    case TO_SKILLS:
			buf_printf(output, "with flags '{c%s{x', ",
				flag_string(sk_aff_flags, paf->bitvector));
			break;
		}
	    buf_printf(output, "level {c%d{x.\n", paf->level);
	}

	if (!varr_isempty(&victim->sk_affected)) {
		buf_add(output, "Skill affects:\n");
		varr_foreach(&victim->sk_affected, print_sa_cb, output);
	}

	if (!IS_NPC(victim)) {
		qtrouble_t *qt;

		if (IS_ON_QUEST(victim)) {
			buf_printf(output,
				   "Questgiver: [%d]  QuestPnts: [%d]  Questnext: [%d]\n",
				   PC(victim)->questgiver,
				   PC(victim)->questpoints,
				   PC(victim)->questtime < 0 ?
					-PC(victim)->questtime : 0);
		 	buf_printf(output,
				   "QuestCntDown: [%d]  QuestObj:  [%d]  Questmob: [%d]\n",
				   PC(victim)->questtime > 0 ?
					PC(victim)->questtime : 0,
				   PC(victim)->questobj,
				   PC(victim)->questmob);
		}
		else {
			buf_printf(output,
				   "QuestPnts: [%d]  Questnext: [%d]  NOT QUESTING\n",
				   PC(victim)->questpoints,
				   PC(victim)->questtime < 0 ?
					-PC(victim)->questtime : 0);
		}

		buf_add(output, "Quest Troubles: ");
		for (qt = PC(victim)->qtrouble; qt; qt = qt->next)
			buf_printf(output, "[%d]-[%d] ", qt->vnum, qt->count-1);
		buf_add(output, "\n");

		if (!IS_NULLSTR(PC(victim)->twitlist))
			buf_printf(output, "Twitlist: [%s]\n",
				   PC(victim)->twitlist);
	}

	buf_printf(output, "Last fight time: [%s]\n",
		   strtime(victim->last_fight_time));
	if (IS_PUMPED(victim))
		buf_add(output, "Adrenalin is gushing.\n");
	if (IS_NPC(victim)) {
		NPC_DATA *npc = NPC(victim);
		buf_printf(output, "Last fought: [%s]  In_mind: [%s]  "
				   "Target: [%s]\n", 
			npc->last_fought ?
				npc->last_fought->name : "none",
			npc->in_mind ? npc->in_mind : "none",
			npc->target ? npc->target->name : "none");
	}
	page_to_char(buf_string(output), ch);
	buf_free(output);
}

void do_dstat(CHAR_DATA *ch, const char *argument)
{
	BUFFER *output;
	DESCRIPTOR_DATA *d;
	char arg[MAX_INPUT_LENGTH];
	int desc;

	one_argument(argument, arg, sizeof(arg));
	if (!is_number(arg)) {
		do_help(ch, "'WIZ STAT'");
		return;
	}

	desc = atoi(arg);
	for (d = descriptor_list; d; d = d->next)
		if (d->descriptor == desc)
			break;
	if (!d) {
		char_puts("dstat: descriptor not found\n", ch);
		return;
	}

	output = buf_new(-1);

	buf_printf(output, "Desc: [%d]  Conn: [%d]  "
			   "Outsize: [%d]  Outtop:  [%d]\n"
			   "Snoopsize: [%d]  Snooptop:  [%d]\n",
		   d->descriptor, d->connected,
		   d->out_buf.size, d->out_buf.top,
		   d->snoop_buf.size, d->snoop_buf.top);
	buf_printf(output, "Inbuf: [%s]\n", d->inbuf);
	buf_printf(output, "Incomm: [%s]\n", d->incomm);
	buf_printf(output, "Repeat: [%d]  Inlast: [%s]\n",
		   d->repeat, d->inlast);
	if (d->character)
		buf_printf(output, "Ch: [%s]\n", d->character->name);
	if (d->original)
		buf_printf(output, "Original: [%s]\n", d->original->name);
	if (d->olced)
		buf_printf(output, "OlcEd: [%s]\n", d->olced->name);
	if (d->pString)
		buf_printf(output, "pString: [%s]\n", *d->pString);
	if (d->showstr_head)
		buf_printf(output, "showstr_head: [%s]\n", d->showstr_head);

	page_to_char(buf_string(output), ch);
	buf_free(output);
}

void do_vnum(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	const char *string;

	string = one_argument(argument, arg, sizeof(arg));
	
	if (arg[0] == '\0') {
		do_help(ch, "'WIZ VNUM'");
		return;
	}

	if (!str_cmp(arg, "obj")) {
		do_ofind(ch, string);
		return;
	}

	if (!str_cmp(arg, "mob") || !str_cmp(arg, "char")) { 
		do_mfind(ch, string);
		return;
	}

	/* do both */
	do_mfind(ch, argument);
	do_ofind(ch, argument);
}

void do_mfind(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	MOB_INDEX_DATA *pMobIndex;
	int vnum;
	int nMatch;
	bool found;

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		char_puts("Find whom?\n", ch);
		return;
	}

	found	= FALSE;
	nMatch	= 0;

	/*
	 * Yeah, so iterating over all vnum's takes 10,000 loops.
	 * Get_mob_index is fast, and I don't feel like threading another link.
	 * Do you?
	 * -- Furey
	 */
	for (vnum = 0; nMatch < top_mob_index; vnum++)
		if ((pMobIndex = get_mob_index(vnum)) != NULL) {
		    nMatch++;
		    if (is_name(argument, pMobIndex->name)) {
			found = TRUE;
			char_printf(ch, "[%5d] %s\n", pMobIndex->vnum,
				    mlstr_mval(&pMobIndex->short_descr));
		    }
		}

	if (!found)
		char_puts("No mobiles by that name.\n", ch);
}

void do_ofind(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_INDEX_DATA *pObjIndex;
	int vnum;
	int nMatch;
	bool found;

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		char_puts("Find what?\n", ch);
		return;
	}

	found	= FALSE;
	nMatch	= 0;

	/*
	 * Yeah, so iterating over all vnum's takes 10,000 loops.
	 * Get_obj_index is fast, and I don't feel like threading another link.
	 * Do you?
	 * -- Furey
	 */
	for (vnum = 0; nMatch < top_obj_index; vnum++)
		if ((pObjIndex = get_obj_index(vnum)) != NULL) {
		    nMatch++;
		    if (is_name(argument, pObjIndex->name)) {
			found = TRUE;
			char_printf(ch, "[%5d] %s\n", pObjIndex->vnum,
				    mlstr_mval(&pObjIndex->short_descr));
		    }
		}

	if (!found)
		char_puts("No objects by that name.\n", ch);
}

void do_owhere(CHAR_DATA *ch, const char *argument)
{
	BUFFER *buffer = NULL;
	OBJ_DATA *obj;
	OBJ_DATA *in_obj;
	int number = 0, max_found = 200, vnum = -1;

	if (argument[0] == '\0') {
		do_help(ch, "'WIZ OWHERE'");
		return;
	}
	
	if (is_number(argument)) vnum = atoi(argument);

	for (obj = object_list; obj != NULL; obj = obj->next) {
		if (!can_see_obj(ch, obj)
		|| (vnum > 0 && obj->pObjIndex->vnum != vnum) 
		|| (vnum < 0 && !is_name(argument, obj->name)))
	        	continue;
	
		if (buffer == NULL)
			buffer = buf_new(-1);
		number++;
	
		for (in_obj = obj; in_obj->in_obj != NULL;
		     in_obj = in_obj->in_obj)
	        	;
	
		if (in_obj->carried_by != NULL
		&&  can_see(ch,in_obj->carried_by)
		&&  in_obj->carried_by->in_room != NULL)
			buf_printf(buffer,
				   "%3d) %s is carried by %s [Room %d]\n",
				number,
				mlstr_mval(&obj->short_descr),
				fix_short(PERS(in_obj->carried_by, ch)),
				in_obj->carried_by->in_room->vnum);
		else if (in_obj->in_room != NULL
		     &&  can_see_room(ch, in_obj->in_room))
	        	buf_printf(buffer, "%3d) %s is in %s [Room %d]\n",
	        		number, mlstr_mval(&obj->short_descr),
				mlstr_cval(&in_obj->in_room->name, ch), 
				in_obj->in_room->vnum);
		else
			buf_printf(buffer, "%3d) %s is somewhere\n",number,
				mlstr_mval(&obj->short_descr));
	
	    if (number >= max_found)
	        break;
	}
	
	if (buffer == NULL)
		char_puts("Nothing like that in heaven or earth.\n", ch);
	else {
		page_to_char(buf_string(buffer),ch);
		buf_free(buffer);
	}
}

void do_mwhere(CHAR_DATA *ch, const char *argument)
{
	BUFFER *buffer;
	CHAR_DATA *victim;
	int count = 0, vnum = -1;

	if (argument[0] == '\0') {
		DESCRIPTOR_DATA *d;

		/* show characters logged */

		buffer = buf_new(-1);
		for (d = descriptor_list; d != NULL; d = d->next)
		{
		    if (d->character != NULL && d->connected == CON_PLAYING
		    &&  d->character->in_room != NULL && can_see(ch,d->character)
		    &&  can_see_room(ch,d->character->in_room))
		    {
			victim = d->character;
			count++;
			if (d->original != NULL)
			    buf_printf(buffer,"%3d) %s (in the body of %s) is in %s [%d]\n",
				count, d->original->name,
				mlstr_mval(&victim->short_descr),
				mlstr_mval(&victim->in_room->name),
				victim->in_room->vnum);
			else
			    buf_printf(buffer,"%3d) %s is in %s [%d]\n",
				count, victim->name,
				mlstr_mval(&victim->in_room->name),
				victim->in_room->vnum);
		    }
		}

	    page_to_char(buf_string(buffer),ch);
		buf_free(buffer);
		return;
	}

	buffer = NULL;
	if (is_number(argument)) vnum = atoi(argument);

	for (victim = char_list; victim; victim = victim->next)
		if (victim->in_room
		&&  can_see(ch, victim)
		&&  (is_name(argument, victim->name)
		    || (IS_NPC(victim) && victim->pMobIndex->vnum == vnum))) {
			if (buffer == NULL)
				buffer = buf_new(-1);

			count++;
			buf_printf(buffer, "%3d) [%5d] %-28s [%5d] %s\n",
			  count, IS_NPC(victim) ? victim->pMobIndex->vnum : 0,
			  IS_NPC(victim) ?
			      mlstr_mval(&victim->short_descr) : victim->name,
			  victim->in_room->vnum,
			  mlstr_mval(&victim->in_room->name));
		}

	if (buffer) {
		page_to_char(buf_string(buffer),ch);
		buf_free(buffer);
	}
	else
		act("You didn't find any $T.", ch, NULL, argument, TO_CHAR);
}

void do_protect(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *victim;

	if (argument[0] == '\0') {
		do_help(ch, "'WIZ PROTECT'");
		return;
	}

	if ((victim = get_char_world(ch,argument)) == NULL) {
		char_puts("You can't find them.\n",ch);
		return;
	}

	if (IS_SET(victim->comm,COMM_SNOOP_PROOF)) {
		act_puts("$N is no longer snoop-proof.", ch, NULL, victim,
			 TO_CHAR, POS_DEAD);
		char_puts("Your snoop-proofing was just removed.\n", victim);
		REMOVE_BIT(victim->comm, COMM_SNOOP_PROOF);
	}
	else {
		act_puts("$N is now snoop-proof.", ch, NULL, victim, TO_CHAR,
			 POS_DEAD);
		char_puts("You are now immune to snooping.\n", victim);
		SET_BIT(victim->comm, COMM_SNOOP_PROOF);
	}
}
	
void do_snoop(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	DESCRIPTOR_DATA *d;
	CHAR_DATA *victim;

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		do_help(ch, "'WIZ SNOOP'");
		return;
	}

	if ((victim = get_char_world(ch, arg)) == NULL) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	if (victim->desc == NULL) {
		char_puts("No descriptor to snoop.\n", ch);
		return;
	}

	if (victim == ch) {
		char_puts("Cancelling all snoops.\n", ch);
		wiznet("$N stops being such a snoop.", ch, NULL, WIZ_SNOOPS,
		       WIZ_SECURE, trust_level(ch));
		for (d = descriptor_list; d != NULL; d = d->next)
			if (d->snoop_by == ch->desc)
				d->snoop_by = NULL;
		return;
	}

	if (victim->desc->snoop_by != NULL) {
		char_puts("Busy already.\n", ch);
		return;
	}

	if (ch->in_room != victim->in_room 
	&&  room_is_private(victim->in_room)
	&&  !IS_TRUSTED(ch, LEVEL_IMP)) {
		char_puts("That character is in a private room.\n",ch);
		return;
	}

	if (!IS_TRUSTED(ch, trust_level(victim))
	||  IS_SET(victim->comm, COMM_SNOOP_PROOF)) {
		char_puts("You failed.\n", ch);
		return;
	}

	if (ch->desc != NULL)
		for (d = ch->desc->snoop_by; d != NULL; d = d->snoop_by)
		    if (d->character == victim || d->original == victim) {
			char_puts("No snoop loops.\n", ch);
			return;
		    }

	victim->desc->snoop_by = ch->desc;
	wiznet("$N starts snooping on $i.",
		ch, victim, WIZ_SNOOPS, WIZ_SECURE, trust_level(ch));
	char_puts("Ok.\n", ch);
}

void do_switch(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	if (ch->desc == NULL)
		return;
	
	one_argument(argument, arg, sizeof(arg));
	
	if (arg[0] == '\0') {
		do_help(ch, "'WIZ SWITCH'");
		return;
	}

	if (ch->desc->original != NULL) {
		char_puts("You are already switched.\n", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg)) == NULL) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	if (victim == ch) {
		char_puts("Ok.\n", ch);
		return;
	}

	if (!IS_NPC(victim)) {
		char_puts("You can only switch into mobiles.\n", ch);
		return;
	}

	if (victim->desc != NULL) {
		char_puts("Character in use.\n", ch);
		return;
	}

	wiznet("$N switches into $i.",
		ch, victim, WIZ_SWITCHES, WIZ_SECURE, trust_level(ch));

	ch->desc->character = victim;
	ch->desc->original  = ch;
	victim->desc        = ch->desc;
	ch->desc            = NULL;
	victim->comm = ch->comm;
	char_puts("Ok.\n", victim);
}

void do_return(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *vch = GET_ORIGINAL(ch);

	if (vch == NULL || vch == ch)
		return;

	char_puts("You return to your original body.\n", ch);
	wiznet("$N returns from $i.",
		vch, ch, WIZ_SWITCHES, WIZ_SECURE, trust_level(vch));
	ch->desc->character	= vch;
	ch->desc->original	= NULL;
	vch->desc		= ch->desc;
	ch->desc		= NULL;

	do_replay(vch, str_empty);
}

/* for clone, to insure that cloning goes many levels deep */
static void recursive_clone(CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *clone)
{
	OBJ_DATA *c_obj, *t_obj;
	for (c_obj = obj->contains; c_obj != NULL; c_obj = c_obj->next_content) {
			t_obj = create_obj(c_obj->pObjIndex, 0);
			clone_obj(c_obj, t_obj);
			obj_to_obj(t_obj, clone);
			recursive_clone(ch, c_obj, t_obj);
		}
}

/* command that is similar to load */
void do_clone(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	const char *rest;
	CHAR_DATA *mob;
	OBJ_DATA  *obj;

	rest = one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		do_help(ch, "'WIZ CLONE'");
		return;
	}

	if (!str_prefix(arg,"object")) {
		mob = NULL;
		obj = get_obj_here(ch,rest);
		if (obj == NULL) {
		    char_puts("You don't see that here.\n",ch);
		    return;
		}
	} else if (!str_prefix(arg,"mobile") || !str_prefix(arg,"character")) {
		obj = NULL;
		mob = get_char_room(ch,rest);
		if (mob == NULL) {
		    char_puts("You don't see that here.\n",ch);
		    return;
		}
	} else { /* find both */
		mob = get_char_room(ch,argument);
		obj = get_obj_here(ch,argument);
		if (mob == NULL && obj == NULL) {
			char_puts("You don't see that here.\n",ch);
			return;
		}
	}

	/* clone an object */
	if (obj) {
		OBJ_DATA *clone;

		clone = create_obj(obj->pObjIndex, 0); 
		clone_obj(obj, clone);
		if (obj->carried_by != NULL)
		    obj_to_char(clone, ch);
		else
		    obj_to_room(clone, ch->in_room);
		recursive_clone(ch, obj, clone);

		act("$n has created $p.", ch, clone, NULL, TO_ROOM);
		act("You clone $p.", ch, clone, NULL, TO_CHAR);
		wiznet("$N clones $p.",
			ch, clone, WIZ_LOAD, WIZ_SECURE, trust_level(ch));
		return;
	} else if (mob != NULL) {
		CHAR_DATA *clone;
		OBJ_DATA *new_obj;

		if (!IS_NPC(mob)) {
		    char_puts("You can only clone mobiles.\n",ch);
		    return;
		}

		clone = create_mob(mob->pMobIndex);
		clone_mob(mob, clone); 
		
		for (obj = mob->carrying; obj != NULL; obj = obj->next_content) {
				new_obj = create_obj(obj->pObjIndex, 0);
				clone_obj(obj, new_obj);
				recursive_clone(ch, obj, new_obj);
				obj_to_char(new_obj, clone);
				new_obj->wear_loc = obj->wear_loc;
			}
		act("$n has created $N.", ch, NULL, clone, TO_ROOM);
		act("You clone $N.", ch, NULL, clone, TO_CHAR);
		wiznet("$N clones $i.",
			ch, clone, WIZ_LOAD, WIZ_SECURE, trust_level(ch));
		char_to_room(clone, ch->in_room);
	}
}

/* RT to replace the two load commands */

void do_load(CHAR_DATA *ch, const char *argument)
{
	 char arg[MAX_INPUT_LENGTH];

	argument = one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		do_help(ch, "'WIZ LOAD'");
		return;
	}

	if (!str_cmp(arg,"mob") || !str_cmp(arg,"char")) {
		do_mload(ch, argument);
		return;
	}

	if (!str_cmp(arg,"obj")) {
		do_oload(ch, argument);
		return;
	}
	/* echo syntax */
	do_load(ch,str_empty);
}

void do_mload(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	MOB_INDEX_DATA *pMobIndex;
	CHAR_DATA *victim;
	int vnum;
	
	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0' || !is_number(arg)) {
		do_help(ch, "'WIZ LOAD'");
		return;
	}

	if ((pMobIndex = get_mob_index(vnum = atoi(arg))) == NULL) {
		char_printf(ch, "%d: No mob has that vnum.\n", vnum);
		return;
	}

	victim = create_mob(pMobIndex);
	act("$n has created $N!", ch, NULL, victim, TO_ROOM);
	wiznet("$N loads $i.", ch, victim,
	       WIZ_LOAD, WIZ_SECURE, trust_level(ch));
	char_puts("Ok.\n", ch);
	char_to_room(victim, ch->in_room);
}

void do_oload(CHAR_DATA *ch, const char *argument)
{
	char arg1[MAX_INPUT_LENGTH] ,arg2[MAX_INPUT_LENGTH];
	OBJ_INDEX_DATA *pObjIndex;
	OBJ_DATA *obj;
	int vnum;
	
	argument = one_argument(argument, arg1, sizeof(arg1));
	one_argument(argument, arg2, sizeof(arg2));

	if (arg1[0] == '\0' || !is_number(arg1)) {
		do_help(ch, "'WIZ LOAD'");
		return;
	}
	
	vnum = atoi(arg1);
	if ((pObjIndex = get_obj_index(vnum)) == NULL) {
		char_printf(ch, "%d: No objects with this vnum.\n", vnum);
		return;
	}

	obj = create_obj(pObjIndex, 0);
	if (CAN_WEAR(obj, ITEM_TAKE))
		obj_to_char(obj, ch);
	else
		obj_to_room(obj, ch->in_room);
	act("$n has created $p!", ch, obj, NULL, TO_ROOM);
	wiznet("$N loads $p.", ch, obj, WIZ_LOAD, WIZ_SECURE, trust_level(ch));
	char_puts("Ok.\n", ch);
}

void do_purge(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	DESCRIPTOR_DATA *d;

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		/* 'purge' */
		CHAR_DATA *vnext;
		OBJ_DATA  *obj_next;

		for (victim = ch->in_room->people; victim; victim = vnext) {
			vnext = victim->next_in_room;
			if (IS_NPC(victim)
			&&  !IS_SET(victim->pMobIndex->act, ACT_NOPURGE) 
			&&  victim != ch /* safety precaution */)
				extract_char(victim, 0);
		}

		for (obj = ch->in_room->contents; obj != NULL; obj = obj_next) {
			obj_next = obj->next_content;
			if (!IS_OBJ_STAT(obj, ITEM_NOPURGE))
				extract_obj(obj, 0);
		}

		act("$n purges the room!", ch, NULL, NULL, TO_ROOM);
		char_puts("Ok.\n", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg)) == NULL) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	if (!IS_NPC(victim)) {
		if (!IS_TRUSTED(ch, LEVEL_GOD)) {
			char_puts("Not at your level.\n", ch);
			return;
		}
		if (ch == victim) {
			char_puts("Ho ho ho.\n", ch);
			return;
		}

		if (!IS_TRUSTED(ch, trust_level(victim))) {
			char_puts("Maybe that wasn't a good idea...\n",ch);
			char_printf(ch,"%s tried to purge you!\n",ch->name);
			return;
		}

		act("$n disintegrates $N.", ch, 0, victim, TO_NOTVICT);

		char_save(victim, 0);
		d = victim->desc;
		extract_char(victim, 0);
		if (d)
			close_descriptor(d, SAVE_F_NORMAL);
		return;
	}

	act("$n purges $N.", ch, NULL, victim, TO_NOTVICT);
	extract_char(victim, 0);
}

static void
restore_char(CHAR_DATA *ch, CHAR_DATA *vch)
{
	affect_strip(vch, "plague");
	affect_strip(vch, "poison");
	affect_strip(vch, "blindness");
	affect_strip(vch, "sleep");
	affect_strip(vch, "curse");
	        
	vch->hit 	= vch->max_hit;
	vch->mana	= vch->max_mana;
	vch->move	= vch->max_move;
	update_pos(vch);
	act("$n has restored you.", ch, NULL, vch, TO_VICT);
}

void do_restore(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	CHAR_DATA *vch;
	DESCRIPTOR_DATA *d;

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0' || !str_cmp(arg, "room")) {
		/*
		 * cure room
		 */
		for (vch = ch->in_room->people; vch; vch = vch->next_in_room)
			restore_char(ch, vch);
		wiznet("$N restored room $j.",
			ch, (const void*) ch->in_room->vnum,
			WIZ_RESTORE, WIZ_SECURE, trust_level(ch));
		char_puts("Room restored.\n", ch);
		return;
	}
	
	if (ch->level >=  MAX_LEVEL - 1 && !str_cmp(arg, "all")) {
		/*
		 * cure all
		 */
		for (d = descriptor_list; d != NULL; d = d->next) {
			vch = d->character;

			if (vch == NULL || IS_NPC(vch))
				continue;
	            
			restore_char(ch, vch);
		}
		wiznet("$N restored all active players.",
			ch, NULL, WIZ_RESTORE, WIZ_SECURE, trust_level(ch));
		char_puts("All active players restored.\n", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg)) == NULL) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	restore_char(ch, victim);
	wiznet("$N restored $i.",
		ch, victim, WIZ_RESTORE, WIZ_SECURE, trust_level(ch));
	char_puts("Ok.\n", ch);
}
		
void do_freeze(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		do_help(ch, "'WIZ FREEZE'");
		return;
	}

	if ((victim = get_char_world(ch, arg)) == NULL) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	if (IS_NPC(victim)) {
		char_puts("Not on NPC's.\n", ch);
		return;
	}

	if (!IS_TRUSTED(ch, trust_level(victim))) {
		char_puts("You failed.\n", ch);
		return;
	}

	TOGGLE_BIT(PC(victim)->plr_flags, PLR_FREEZE);
	if (!IS_SET(PC(victim)->plr_flags, PLR_FREEZE)) {
		char_puts("You can play again.\n", victim);
		char_puts("FREEZE removed.\n", ch);
		wiznet("$N thaws $i.",
			ch, victim, WIZ_PENALTIES, WIZ_SECURE, 0);
	}
	else {
		char_puts("You can't do ANYthing!\n", victim);
		char_puts("FREEZE set.\n", ch);
		wiznet("$N puts $i in the deep freeze.",
			ch, victim, WIZ_PENALTIES, WIZ_SECURE, 0);
	}
	char_save(victim, 0);
}

void do_log(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		do_help(ch, "'WIZ LOG'");
		return;                 
	}

	if (!str_cmp(arg, "all")) {
		if (fLogAll) {
			fLogAll = FALSE;
			char_puts("Log ALL off.\n", ch);
		} else {
			fLogAll = TRUE;
			char_puts("Log ALL on.\n", ch);
		}
		return;
	}

	if ((victim = get_char_world(ch, arg)) == NULL) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	if (IS_NPC(victim)) {
		char_puts("Not on NPC's.\n", ch);
		return;
	}

	/*
	 * No level check, gods can log anyone.
	 */
	TOGGLE_BIT(PC(victim)->plr_flags, PLR_LOG);
	if (!IS_SET(PC(victim)->plr_flags, PLR_LOG))
		char_puts("LOG removed.\n", ch);
	else 
		char_puts("LOG set.\n", ch);
}

void do_noemote(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		do_help(ch, "'WIZ NOEMOTE'");
		return;              
	}

	if ((victim = get_char_world(ch, arg)) == NULL) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	if (!IS_TRUSTED(ch, trust_level(victim))) {
		char_puts("You failed.\n", ch);
		return;
	}

	if (IS_SET(victim->comm, COMM_NOEMOTE)) {
		REMOVE_BIT(victim->comm, COMM_NOEMOTE);
		char_puts("You can emote again.\n", victim);
		char_puts("NOEMOTE removed.\n", ch);
		wiznet("$N restores emotes to $i.",
			ch, victim, WIZ_PENALTIES, WIZ_SECURE, 0);
	}
	else {
		SET_BIT(victim->comm, COMM_NOEMOTE);
		char_puts("You can't emote!\n", victim);
		char_puts("NOEMOTE set.\n", ch);
		wiznet("$N revokes $i's emotes.",
			ch, victim, WIZ_PENALTIES, WIZ_SECURE, 0);
	}
}

void do_notell(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		do_help(ch, "'WIZ NOTELL'");
		return;
	}

	if ((victim = get_char_world(ch, arg)) == NULL) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	if (!IS_TRUSTED(ch, trust_level(victim))) {
		char_puts("You failed.\n", ch);
		return;
	}

	if (IS_SET(victim->comm, COMM_NOTELL)) {
		REMOVE_BIT(victim->comm, COMM_NOTELL);
		char_puts("You can tell again.\n", victim);
		char_puts("NOTELL removed.\n", ch);
		wiznet("$N restores tells to $i.",
			ch, victim, WIZ_PENALTIES, WIZ_SECURE, 0);
	}
	else {
		SET_BIT(victim->comm, COMM_NOTELL);
		char_puts("You can't tell!\n", victim);
		char_puts("NOTELL set.\n", ch);
		wiznet("$N revokes $i's tells.",
			ch, victim, WIZ_PENALTIES, WIZ_SECURE, 0);
	}
}

void do_peace(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *rch;

	for (rch = ch->in_room->people; rch; rch = rch->next_in_room) {
		if (!rch->fighting)
			continue;
		stop_fighting(rch, FALSE);
		if (IS_NPC(rch)) {
			/*
			 * avoid aggressive mobs and hunting mobs attacks
			 */
			AFFECT_DATA af;

			af.where = TO_AFFECTS;
			af.type = "calm";
			af.level = MAX_LEVEL;
			af.duration = 15;
			af.location = APPLY_NONE;
			af.modifier = 0;
			af.bitvector = AFF_CALM;
			affect_to_char(rch, &af);
		}
	}

	char_puts("Ok.\n", ch);
}

void do_wizlock(CHAR_DATA *ch, const char *argument)
{
	extern bool wizlock;
	wizlock = !wizlock;

	if (wizlock) {
		wiznet("$N has wizlocked the game.", ch, NULL, 0, 0, 0);
		char_puts("Game wizlocked.\n", ch);
	}
	else {
		wiznet("$N removes wizlock.", ch, NULL, 0, 0, 0);
		char_puts("Game un-wizlocked.\n", ch);
	}
}

/* RT anti-newbie code */
void do_newlock(CHAR_DATA *ch, const char *argument)
{
	extern bool newlock;
	newlock = !newlock;
	
	if (newlock) {
		wiznet("$N locks out new characters.", ch, NULL, 0, 0, 0);
		char_puts("New characters have been locked out.\n", ch);
	}
	else {
		wiznet("$N allows new characters back in.", ch, NULL, 0, 0, 0);
		char_puts("Newlock removed.\n", ch);
	}
}

/* RT set replaces sset, mset, oset, and rset */
void do_set(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];

	argument = one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		do_help(ch, "'WIZ SET'");
		return;
	}

	if (!str_prefix(arg,"mobile") || !str_prefix(arg,"character")) {
		do_mset(ch,argument);
		return;
	}

	if (!str_prefix(arg,"skill") || !str_prefix(arg,"spell")) {
		do_sset(ch,argument);
		return;
	}

	if (!str_prefix(arg,"object")) {
		do_oset(ch,argument);
		return;
	}

	if (!str_prefix(arg,"room")) {
		do_rset(ch,argument);
		return;
	}

	/* echo syntax */
	do_set(ch, str_empty);
}

static void *
sset_cb(void *p, va_list ap)
{
	skill_t *sk = (skill_t*) p;

	CHAR_DATA *victim = va_arg(ap, CHAR_DATA *);
	int val = va_arg(ap, int);

	set_skill(victim, sk->name, val);
	return NULL;
}

void do_sset(CHAR_DATA *ch, const char *argument)
{
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	char arg3 [MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int value;

	argument = one_argument(argument, arg1, sizeof(arg1));
	argument = one_argument(argument, arg2, sizeof(arg2));
	argument = one_argument(argument, arg3, sizeof(arg3));

	if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0') {
		char_puts("Syntax:\n",ch);
		char_puts("  set skill <name> <spell or skill> <value>\n", ch);
		char_puts("  set skill <name> all <value>\n",ch);  
		char_puts("(use the name of the skill, not the number)\n",ch);
		return;
	}

	if ((victim = get_char_world(ch, arg1)) == NULL) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	if (IS_NPC(victim)) {
		char_puts("Not on NPC's.\n", ch);
		return;
	}

	/*
	 * Snarf the value.
	 */
	if (!is_number(arg3)) {
		char_puts("Value must be numeric.\n", ch);
		return;
	}

	value = atoi(arg3);
	if (value < 0) {
		char_puts("Value should not be negative.\n", ch);
		return;
	}

	if (!str_cmp(arg2, "all")) {
		hash_foreach(&skills, sset_cb, victim, value);
		char_puts("Ok.\n", ch);
	} else {
		const char *sn;
		skill_t *sk;

		if ((sk = skill_search(arg2)) == NULL) {
			if (value) {
				char_puts("do_sset: Cannot set non-zero value for unknown skill.\n", ch);
				return;
			}
			sn = arg2;
		} else
			sn = sk->name;

		set_skill(victim, sn, value);
		char_printf(ch, "do_sset: '%s': %d%%\n", sn, value);
	}
	update_skills(victim);
}

void do_string(CHAR_DATA *ch, const char *argument)
{
	char type[MAX_INPUT_LENGTH];
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA *obj;

	argument = one_argument(argument, type, sizeof(type));
	argument = one_argument(argument, arg1, sizeof(arg1));
	argument = one_argument(argument, arg2, sizeof(arg2));
	strnzcpy(arg3, sizeof(arg3), argument);

	if (type[0] == '\0' || arg1[0] == '\0'
	||  arg2[0] == '\0' || arg3[0] == '\0') {
		do_help(ch, "'WIZ STRING'");
		return;
	}
	
	if (!str_prefix(type, "character") || !str_prefix(type, "mobile")) {
		if ((victim = get_char_room(ch, arg1)) == NULL) {
			char_puts("They aren't here.\n", ch);
			return;
		}

		/* clear zone for mobs */
		if (IS_NPC(victim))
			NPC(victim)->zone = NULL;

		/* string something */

	 	if (!str_prefix(arg2, "name")) {
			if (!IS_NPC(victim)) {
				char_puts("Not on PC's.\n", ch);
				return;
			}
			free_string(victim->name);
			victim->name = str_dup(arg3);
			return;
		}
		
		if (!str_prefix(arg2, "short")) {
			if (!IS_NPC(victim)) {
				char_puts(" Not on PC's.\n", ch);
				return;
			}
			mlstr_edit(&victim->short_descr, arg3);
			return;
		}

		if (!str_prefix(arg2, "desc")) {
			mlstr_append(ch, &victim->description, arg3);
			return;
		}

		if (!str_prefix(arg2, "long")) {
			if (!IS_NPC(victim)) {
				char_puts("Not on PC's.\n", ch);
				return;
			}
			mlstr_editnl(&victim->long_descr, arg3);
			return;
		}

		if (!str_prefix(arg2, "title")) {
			if (IS_NPC(victim)) {
				char_puts("Not on NPC's.\n", ch);
				return;
			}

			set_title(victim, arg3);
			return;
		}
	}
	
	if (!str_prefix(type,"object")) {
		/* string an obj */
		
	 	if ((obj = get_obj_room(ch, arg1)) == NULL) {
			char_puts("Nothing like that in heaven or earth.\n",
				  ch);
			return;
		}
		
		if (obj->pObjIndex->limit >= 0) {
			char_puts("You cannot string limited objs.\n", ch);
			return;
		}

		if (!str_prefix(arg2, "name")) {
			free_string(obj->name);
			obj->name = str_dup(arg3);
			return;
		}

		if (!str_prefix(arg2, "short")) {
			mlstr_edit(&obj->short_descr, arg3);
			return;
		}

		if (!str_prefix(arg2, "long")) {
			mlstr_edit(&obj->description, arg3);
			return;
		}

		if (!str_prefix(arg2, "ed")
		||  !str_prefix(arg2, "extended")
		||  !str_prefix(arg2, "exd")) {
			ED_DATA *ed;

			if (obj->carried_by != ch) {
				char_puts("Obj must be in your inventory.\n", ch);
				return;
			}

			argument = one_argument(argument, arg3, sizeof(arg3));
			if (argument == NULL) {
				char_puts("Syntax: oset <object> ed <keyword> "
					  "lang\n", ch);
				return;
			}

			ed = ed_new();
			ed->keyword	= str_dup(arg3);
			ed->next	= obj->ed;
			mlstr_append(ch, &ed->description, argument);
			obj->ed	= ed;
			return;
		}
	}
	
	/* echo bad use message */
	do_string(ch,str_empty);
}

void do_oset(CHAR_DATA *ch, const char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_STRING_LENGTH];
	OBJ_DATA *obj;
	int value;

	argument = one_argument(argument, arg1, sizeof(arg1));
	argument = one_argument(argument, arg2, sizeof(arg2));
		   one_argument(argument, arg3, sizeof(arg3));

	if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0') {
		do_help(ch, "'WIZ SET'");
		return;
	}

	if ((obj = get_obj_world(ch, arg1)) == NULL) {
		char_puts("Nothing like that in heaven or earth.\n", ch);
		return;
	}

	value = atoi(arg3);

	/*
	 * Set something.
	 */
	if (!str_cmp(arg2, "value0") || !str_cmp(arg2, "v0")) {
		obj->value[0] = value;
		return;
	}

	if (!str_cmp(arg2, "value1") || !str_cmp(arg2, "v1")) {
		obj->value[1] = value;
		return;
	}

	if (!str_cmp(arg2, "value2") || !str_cmp(arg2, "v2")) {
		obj->value[2] = value;
		return;
	}

	if (!str_cmp(arg2, "value3") || !str_cmp(arg2, "v3")) {
		obj->value[3] = value;
		return;
	}

	if (!str_cmp(arg2, "value4") || !str_cmp(arg2, "v4")) {
		obj->value[4] = value;
		return;
	}

	if (!str_prefix(arg2, "level")) {
		obj->level = value;
		return;
	}
		
	if (!str_prefix(arg2, "cost")) {
		obj->cost = value;
		return;
	}

	if (!str_prefix(arg2, "timer")) {
		obj->timer = value;
		return;
	}
		
	if (!str_prefix(arg2, "owner")) {
		mlstr_destroy(&obj->owner);
		mlstr_init(&obj->owner, arg3);
		return;
	}

	/*
	 * Generate usage message.
	 */
	do_oset(ch, str_empty);
	return;
}

void do_rset(CHAR_DATA *ch, const char *argument)
{
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	char arg3 [MAX_INPUT_LENGTH];
	ROOM_INDEX_DATA *location;
	int value;

	argument = one_argument(argument, arg1, sizeof(arg1));
	argument = one_argument(argument, arg2, sizeof(arg2));
		   one_argument(argument, arg3, sizeof(arg3));

	if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0') {
		do_help(ch, "'WIZ SET'");
		return;
	}

	if ((location = find_location(ch, arg1)) == NULL) {
		char_puts("No such location.\n", ch);
		return;
	}

	if (ch->in_room != location 
	&&  room_is_private(location) && !IS_TRUSTED(ch, LEVEL_IMP)) {
		char_puts("That room is private right now.\n",ch);
		return;
	}

	/*
	 * Snarf the value.
	 */
	if (!is_number(arg3)) {
		char_puts("Value must be numeric.\n", ch);
		return;
	}
	value = atoi(arg3);

	/*
	 * Set something.
	 */
	if (!str_prefix(arg2, "flags")) {
		location->room_flags	= value;
		return;
	}

	if (!str_prefix(arg2, "sector")) {
		location->sector_type	= value;
		return;
	}

	/*
	 * Generate usage message.
	 */
	do_rset(ch, str_empty);
	return;
}

void do_sockets(CHAR_DATA *ch, const char *argument)
{
	BUFFER *output;
	char arg[MAX_INPUT_LENGTH];
	DESCRIPTOR_DATA *d;
	int count;

	count = 0;
	output = buf_new(-1);

	one_argument(argument, arg, sizeof(arg));
	if (strchr(arg, '@') == NULL) 
		strnzcat(arg, sizeof(arg), "*@*");

	for (d = descriptor_list; d; d = d->next) {
		char buf[MAX_STRING_LENGTH];
		CHAR_DATA *vch = d->original ? d->original : d->character;

		if (vch && !can_see(ch, vch))
			continue;

		snprintf(buf, sizeof(buf), "%s@%s",
			 vch ? vch->name : NULL,
			 d->host);

		if (fnmatch(arg, buf, FNM_CASEFOLD) != 0)
			continue;

		count++;
		buf_printf(output, "[%3d %16s] %s (%s)",
			   d->descriptor,
			   flag_string(desc_con_table, d->connected),
			   buf,
			   d->ip);
		if (vch && PC(vch)->idle_timer)
			buf_printf(output, " idle %d", PC(vch)->idle_timer);
		buf_add(output, "\n");
	}

	if (count == 0) {
		char_puts("No one by that name is connected.\n",ch);
		buf_free(output);
		return;
	}

	buf_printf(output, "%d user%s\n",
		   count, count == 1 ? str_empty : "s");
	page_to_char(buf_string(output), ch);
	buf_free(output);
}

/*
 * Thanks to Grodyn for pointing out bugs in this function.
 */
void do_force(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	argument = one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0' || argument[0] == '\0') {
		do_help(ch, "'WIZ FORCE'");
		return;
	}

	one_argument(argument, arg2, sizeof(arg2));
	
	if (!str_cmp(arg2, "delete") || !str_prefix(arg2, "mob")) {
		char_puts("That will NOT be done.\n",ch);
		return;
	}

	if (!str_cmp(arg, "all")
	||  !str_cmp(arg, "players")) {
		CHAR_DATA *vch;
		CHAR_DATA *vch_next;

		if (!IS_TRUSTED(ch, MAX_LEVEL - 3)) {
			char_puts("Not at your level!\n",ch);
			return;
		}

		for (vch = char_list; vch && !IS_NPC(vch); vch = vch_next) {
			vch_next = vch->next;

			if (!IS_TRUSTED(ch, trust_level(vch)))
				continue;
			act_puts("$n forces you to '$t'.",
				 ch, argument, vch, TO_VICT, POS_DEAD);
			interpret_raw(vch, argument, TRUE);
		}

		char_puts("Ok.\n", ch);
		return;
	} else if (!str_cmp(arg, "gods")) {
		CHAR_DATA *vch;
		CHAR_DATA *vch_next;
	
		if (!IS_TRUSTED(ch, MAX_LEVEL - 2)) {
			char_puts("Not at your level!\n",ch);
			return;
		}
	
		for (vch = char_list; vch && !IS_NPC(vch); vch = vch_next) {
			vch_next = vch->next;
	
			if (!IS_IMMORTAL(vch)
			||  !IS_TRUSTED(ch, trust_level(vch)))
				continue;
			act_puts("$n forces you to '$t'.",
				 ch, argument, vch, TO_VICT, POS_DEAD);
			interpret(vch, argument);
		}

		char_puts("Ok.\n", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg)) == NULL) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	if (victim == ch) {
		char_puts("Aye aye, right away!\n", ch);
		return;
	}

	if (ch->in_room != victim->in_room 
	&&  room_is_private(victim->in_room)
	&&  !IS_TRUSTED(ch, LEVEL_IMP)) {
		char_puts("That character is in a private room.\n", ch);
		return;
	}

	if (!IS_NPC(victim)
	&&  !IS_TRUSTED(ch, MAX_LEVEL-3)) {
		char_puts("Not at your level!\n",ch);
		return;
	}

	if (!IS_TRUSTED(ch, trust_level(victim))) {
		char_puts("Do it yourself!\n", ch);
		return;
	}

	act_puts("$n forces you to '$t'.",
		 ch, argument, victim, TO_VICT, POS_DEAD);
	interpret(victim, argument);

	char_puts("Ok.\n", ch);
}

/*
 * New routines by Dionysos.
 */
void do_invis(CHAR_DATA *ch, const char *argument)
{
	int level;
	char arg[MAX_STRING_LENGTH];

	/* RT code for taking a level argument */
	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		/*
		 * take the default path
		 */
		if (ch->invis_level) {
			ch->invis_level = 0;
			act("$n slowly fades into existence.",
			    ch, NULL, NULL, TO_ROOM);
			char_puts("You slowly fade back into existence.\n", ch);
		} else {
			act("$n slowly fades into thin air.",
			    ch, NULL, NULL, TO_ROOM);
			ch->invis_level = UMIN(LEVEL_IMMORTAL, trust_level(ch));
			char_puts("You slowly vanish into thin air.\n", ch);
		}
	} else {
		/*
		 * do the level thing
		 */
		level = atoi(arg);
		if (level < 2 || level > trust_level(ch)) {
			char_printf(ch, "Invis level must be in range 2..%d.\n",
				    trust_level(ch));
			return;
		} else {
			act("$n slowly fades into thin air.",
			    ch, NULL, NULL, TO_ROOM);
			ch->invis_level = level;
			char_puts("You slowly vanish into thin air.\n", ch);
		}
	}
}

void do_incognito(CHAR_DATA *ch, const char *argument)
{
	int level;
	char arg[MAX_STRING_LENGTH];
	
	one_argument(argument, arg, sizeof(arg));
	
	/* RT code for taking a level argument */
	if (arg[0] == '\0') {
		/*
		 * take the default path
		 */
		if (ch->incog_level) {
			ch->incog_level = 0;
			act("$n is no longer cloaked.",
			    ch, NULL, NULL, TO_ROOM);
			char_puts("You are no longer cloaked.\n", ch);
		} else {
			ch->incog_level = UMIN(LEVEL_IMMORTAL, trust_level(ch));
			act("$n cloaks $s presence.", ch, NULL, NULL, TO_ROOM);
			char_puts("You cloak your presence.\n", ch);
		}
	} else {
		/*
		 * do the level thing
		 */
		level = atoi(arg);
		if (level < 2 || level > ch->level) {
			char_printf(ch, "Incog level must be in range 2..%d.\n",
				    trust_level(ch));
			return;
		} else {
			ch->incog_level = level;
			act("$n cloaks $s presence.", ch, NULL, NULL, TO_ROOM);
			char_puts("You cloak your presence.\n", ch);
		}
	}
}

void do_holylight(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *vch = GET_ORIGINAL(ch);

	if (IS_NPC(vch))
		return;

	TOGGLE_BIT(PC(vch)->plr_flags, PLR_HOLYLIGHT);
	char_printf(ch, "Holy light mode %s.\n",
		    IS_SET(PC(vch)->plr_flags, PLR_HOLYLIGHT) ?
			"on" : "off");
}

/* prefix command: it will put the string typed on each line typed */

void do_prefi(CHAR_DATA *ch, const char *argument)
{
	char_puts("You cannot abbreviate the prefix command.\n", ch);
}

void do_prefix(CHAR_DATA *ch, const char *argument)
{
	DESCRIPTOR_DATA *d;

	if ((d = ch->desc) == NULL)
		return;

	if (argument[0] == '\0') {
		if (d->dvdata->prefix[0] == '\0') {
			char_puts("You have no prefix to clear.\n",ch);
			return;
		}

		free_string(d->dvdata->prefix);
		d->dvdata->prefix = str_empty;
		char_puts("Prefix removed.\n",ch);
		return;
	}

	free_string(d->dvdata->prefix);
	d->dvdata->prefix = str_dup(argument);
	char_printf(ch, "Prefix set to '%s'.\n", argument);
}

void do_advance(CHAR_DATA *ch, const char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int level;

	argument = one_argument(argument, arg1, sizeof(arg1));
	argument = one_argument(argument, arg2, sizeof(arg2));

	if (arg1[0] == '\0' || arg2[0] == '\0' || !is_number(arg2)) {
		do_help(ch, "'WIZ ADVANCE'");
		return;
	}

	if ((victim = get_char_room(ch, arg1)) == NULL) {
		char_puts("That player is not here.\n", ch);
		return;
	}

	if (IS_NPC(victim)) {
		char_puts("Not on NPC's.\n", ch);
		return;
	}

	if ((level = atoi(arg2)) < 1 || level > MAX_LEVEL) {
		char_printf(ch, "Level must be in range 1..%d.\n", MAX_LEVEL);
		return;
	}

	if (level > ch->level) {
		char_puts("Limited to your level.\n", ch);
		return;
	}

	if (!IS_TRUSTED(ch, trust_level(victim))) {
		char_puts("You are not allowed to do that.\n", ch);
		return;
	}

	advance(victim, level);
}

void do_mset(CHAR_DATA *ch, const char *argument)
{
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	char arg3 [MAX_INPUT_LENGTH];
	char arg4 [MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int value, val2;

	argument = one_argument(argument, arg1, sizeof(arg1));
	argument = one_argument(argument, arg2, sizeof(arg2));
	argument = one_argument(argument, arg3, sizeof(arg3));
		   one_argument(argument, arg4, sizeof(arg4));

	if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0') {
		do_help(ch, "'WIZ SET'");
		return;
	}

	if ((victim = get_char_world(ch, arg1)) == NULL) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	/*
	 * Snarf the value (which need not be numeric).
	 */
	value = is_number(arg3) ? atoi(arg3) : -1;
	val2  = is_number(arg4) ? atoi(arg4) : -1;

	/*
	 * Set something.
	 */
	if (!str_cmp(arg2, "str")) {
		if (value < 3 || value > get_max_train(victim,STAT_STR))
		{
		    char_printf(ch,
			"Strength range is 3 to %d\n.",
			get_max_train(victim,STAT_STR));
		    return;
		}

		victim->perm_stat[STAT_STR] = value;
		return;
	}

	if (!str_cmp(arg2, "trouble")) {
		if (IS_NPC(victim)) {
			char_puts("Not on NPC's.\n", ch);
			return;
		}
		
		if (value == -1 || val2 == -1) {
			char_puts("Usage: set char <name> trouble "
				  "<vnum> <value>.\n", ch);
			return;
		}

		qtrouble_set(victim, value, val2+1);
		char_puts("Ok.\n", ch);
		return;
	}

	if (!str_cmp(arg2, "security"))	{ /* OLC */
		if (IS_NPC(ch)) {
			char_puts("Si, claro.\n", ch);
			return;
		}

		if (IS_NPC(victim)) {
			char_puts("Not on NPC's.\n", ch);
			return;
		}

		if ((value > PC(ch)->security || value < 0)
		&& ch->level != MAX_LEVEL) {
			if (PC(ch)->security != 0)
				char_printf(ch, "Valid security is 0-%d.\n",
					    PC(ch)->security);
			else
				char_puts("Valid security is 0 only.\n", ch);
			return;
		}
		PC(victim)->security = value;
		return;
	}

	if (!str_cmp(arg2, "int"))
	{
	    if (value < 3 || value > get_max_train(victim,STAT_INT))
	    {
	        char_printf(ch, "Intelligence range is 3 to %d.\n",
			get_max_train(victim,STAT_INT));
	        return;
	    }
	
	    victim->perm_stat[STAT_INT] = value;
	    return;
	}

	if (!str_cmp(arg2, "wis"))
	{
		if (value < 3 || value > get_max_train(victim,STAT_WIS))
		{
		    char_printf(ch,
			"Wisdom range is 3 to %d.\n",get_max_train(victim,STAT_WIS));
		    return;
		}

		victim->perm_stat[STAT_WIS] = value;
		return;
	}
	if (!str_cmp(arg2, "questp"))
	{
		 if (value == -1) value = 0;
		 if (!IS_NPC(victim)) PC(victim)->questpoints = value;
		return;
	}

	if (!str_cmp(arg2, "questt")) {
		if (IS_NPC(victim)) {
			char_puts("Not on NPC.\n", ch);
			return;
		}

		if (value == -1)
			value = 30;
		PC(victim)->questtime = value;
		return;
	}

	if (!str_cmp(arg2, "relig")) {
		if (IS_NPC(victim)) {
			char_puts("Not on NPC.\n", ch);
			return;
		}

		if (value == -1) value = 0;
		PC(victim)->religion = value;
		return;
	}


	if (!str_cmp(arg2, "dex"))
	{
		if (value < 3 || value > get_max_train(victim,STAT_DEX))
		{
		    char_printf(ch,
			"Dexterity ranges is 3 to %d.\n",
			get_max_train(victim,STAT_DEX));
		    return;
		}

		victim->perm_stat[STAT_DEX] = value;
		return;
	}

	if (!str_cmp(arg2, "con"))
	{
		if (value < 3 || value > get_max_train(victim,STAT_CON))
		{
		    char_printf(ch,
			"Constitution range is 3 to %d.\n",
			get_max_train(victim,STAT_CON));
		    return;
		}

		victim->perm_stat[STAT_CON] = value;
		return;
	}
	if (!str_cmp(arg2, "cha"))
	{
		if (value < 3 || value > get_max_train(victim,STAT_CHA))
		{
		    char_printf(ch,
			"Constitution range is 3 to %d.\n",
			get_max_train(victim,STAT_CHA));
		    return;
		}

		victim->perm_stat[STAT_CHA] = value;
		return;
	}

	if (!str_prefix(arg2, "sex")) {
		if (value < 0 || value > 2) {
		    char_puts("Sex range is 0 to 2.\n", ch);
		    return;
		}

		victim->sex = value;
		return;
	}

	if (!str_prefix(arg2, "class")) {
		class_t *cl;

		if (IS_NPC(victim)) {
			char_puts("Mobiles have no class.\n", ch);
			return;
		}

		if ((cl = class_search(arg3)) == NULL) {
			BUFFER *output = buf_new(-1);
			buf_add(output, "Possible classes are: ");
			strkey_printall(&classes, output);
			send_to_char(buf_string(output), ch);
			buf_free(output);
			return;
		}

		free_string(victim->class);
		victim->class = str_qdup(cl->name);
		spec_update(victim);
		PC(victim)->exp = exp_for_level(victim, victim->level);
		return;
	}

	if (!str_prefix(arg2, "level"))
	{
		if (!IS_NPC(victim))
		{
		    char_puts("Not on PC's.\n", ch);
		    return;
		}

		if (value < 0 || value > 100)
		{
		    char_puts("Level range is 0 to 100.\n", ch);
		    return;
		}
		victim->level = value;
		return;
	}

	if (!str_prefix(arg2, "gold"))
	{
		victim->gold = value;
		return;
	}

	if (!str_prefix(arg2, "hp"))
	{
		if (value < -10 || value > 30000)
		{
		    char_puts("Hp range is -10 to 30,000 hit points.\n", ch);
		    return;
		}
		victim->perm_hit = value;
		return;
	}

	if (!str_prefix(arg2, "mana"))
	{
		if (value < 0 || value > 60000)
		{
		    char_puts("Mana range is 0 to 60,000 mana points.\n", ch);
		    return;
		}
		victim->perm_mana = value;
		return;
	}

	if (!str_prefix(arg2, "move")) {
		if (value < 0 || value > 60000) {
			char_puts("Move range is 0 to 60,000 move points.\n", ch);
			return;
		}
		victim->perm_move = value;
		return;
	}

	if (!str_prefix(arg2, "practice")) {
		if (IS_NPC(victim)) {
			char_puts("Not on NPC.\n", ch);
			return;
		}

		if (value < 0 || value > 250) {
			char_puts("Practice range is 0 to 250 sessions.\n", ch);
			return;
		}
		PC(victim)->practice = value;
		return;
	}

	if (!str_prefix(arg2, "train")) {
		if (IS_NPC(victim)) {
			char_puts("Not on NPC.\n", ch);
			return;
		}

		if (value < 0 || value > 50) {
			char_puts("Training session range is 0 to 50 sessions.\n", ch);
			return;
		}
		PC(victim)->train = value;
		return;
	}

	if (!str_prefix(arg2, "align")) {
		if (value < -1000 || value > 1000) {
			char_puts("Alignment range is -1000 to 1000.\n", ch);
			return;
		}
		victim->alignment = value;
		char_puts("Remember to check their hometown.\n", ch);
		return;
	}

	if (!str_prefix(arg2, "ethos")) {
		int ethos;

		if (IS_NPC(victim)) {
			char_puts("Mobiles don't have an ethos.\n", ch);
			return;
		}

		ethos = flag_value(ethos_table, arg3);
		if (ethos < 0) {
			char_puts("%s: Unknown ethos.\n", ch);
			char_puts("Valid ethos types are:\n", ch);
			show_flags(ch, ethos_table);
			return;
		}

		victim->ethos = ethos;
		return;
	}

	if (!str_prefix(arg2, "hometown")) {
		if (IS_NPC(victim)) {
			char_puts("Mobiles don't have hometowns.\n", ch);
			return;
		}

		if ((value = htn_lookup(arg3)) == -1) {
			char_puts("No such hometown", ch);
			return;
		}
	    
		PC(victim)->hometown = value;
		return;
	}

	if (!str_prefix(arg2, "thirst"))
	{
		if (IS_NPC(victim))
		{
		    char_puts("Not on NPC's.\n", ch);
		    return;
		}

		if (value < -1 || value > 100)
		{
		    char_puts("Thirst range is -1 to 100.\n", ch);
		    return;
		}

		PC(victim)->condition[COND_THIRST] = value;
		return;
	}

	if (!str_prefix(arg2, "drunk"))
	{
		if (IS_NPC(victim))
		{
		    char_puts("Not on NPC's.\n", ch);
		    return;
		}

		if (value < -1 || value > 100)
		{
		    char_puts("Drunk range is -1 to 100.\n", ch);
		    return;
		}

		PC(victim)->condition[COND_DRUNK] = value;
		return;
	}

	if (!str_prefix(arg2, "full"))
	{
		if (IS_NPC(victim))
		{
		    char_puts("Not on NPC's.\n", ch);
		    return;
		}

		if (value < -1 || value > 100)
		{
		    char_puts("Full range is -1 to 100.\n", ch);
		    return;
		}

		PC(victim)->condition[COND_FULL] = value;
		return;
	}

	if (!str_prefix(arg2, "hunger"))
	{
		if (IS_NPC(victim))
		{
		    char_puts("Not on NPC's.\n", ch);
		    return;
		}

		if (value < -1 || value > 100)
		{
		    char_puts("Hunger range is -1 to 100.\n", ch);
		    return;
		}

		PC(victim)->condition[COND_HUNGER] = value;
		return;
	}

	if (!str_prefix(arg2, "bloodlust"))
	{
		if (IS_NPC(victim))
		{
		    char_puts("Not on NPC's.\n", ch);
		    return;
		}

		if (value < -1 || value > 100)
		{
		    char_puts("Full range is -1 to 100.\n", ch);
		    return;
		}

		PC(victim)->condition[COND_BLOODLUST] = value;
		return;
	}

	if (!str_prefix(arg2, "desire")) {
		if (IS_NPC(victim)) {
			char_puts("Not on NPC.\n", ch);
			return;
		}

		if (value < -1 || value > 100)
		{
		    char_puts("Full range is -1 to 100.\n", ch);
		    return;
		}

		PC(victim)->condition[COND_DESIRE] = value;
		return;
	}

	if (!str_prefix(arg2, "race")) {
		race_t *r;

		if (IS_NPC(victim)) {
			char_puts("Not on NPC.\n", ch);
			return;
		}

		if ((r = race_search(arg3)) == NULL) {
			char_puts("That is not a valid race.\n",ch);
			return;
		}

		if (!IS_NPC(victim) && !r->race_pcdata) {
			char_puts("That is not a valid player race.\n",ch);
			return;
		}

		free_string(victim->race);
		victim->race = str_qdup(r->name);
		if (!IS_NPC(victim)) {
			free_string(PC(victim)->race);
			PC(victim)->race = str_qdup(r->name);
		}
			
		spec_update(victim);
		PC(victim)->exp = exp_for_level(victim, victim->level);
		return;
	}

	if (!str_prefix(arg2, "noghost")) {
		if (IS_NPC(victim)) {
			char_puts("Not on NPC.\n", ch);
			return;
		}
		REMOVE_BIT(PC(victim)->plr_flags, PLR_GHOST);
		char_puts("Ok.\n", ch);
		return;
	}

	/*
	 * Generate usage message.
	 */
	do_mset(ch, str_empty);
}

void do_smite(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *victim;

	if (argument[0] == '\0') {
	  char_puts("You are so frustrated you smite yourself!  OWW!\n", 
			ch);
	  return;
	}

	if ((victim = get_char_world(ch, argument)) == NULL) {
	  char_puts("You'll have to smite them some other day.\n", ch);
	  return;
	}

	if (IS_NPC(victim)) {
	  char_puts("That poor mob never did anything to you.\n", ch);
	  return;
	}

	if (victim->level > ch->level) {
	  char_puts("How dare you!\n", ch);
	  return;
	}

	if (victim->position < POS_SLEEPING) {
	  char_puts("Take pity on the poor thing.\n", ch);
	  return;
	}

	act("A bolt comes down out of the heavens and smites you!", victim, NULL,
		ch, TO_CHAR);
	act("You reach down and smite $n!", victim, NULL, ch, TO_VICT);
	act("A bolt from the heavens smites $n!", victim, NULL, ch, TO_NOTVICT);
	victim->hit = 1;
	victim->mana = 0;
	victim->move = 0;
	return;
}

void do_popularity(CHAR_DATA *ch, const char *argument)
{
	BUFFER *output;
	AREA_DATA *area;
	extern AREA_DATA *area_first;
	int i;

	output = buf_new(-1);
	buf_add(output, "Area popularity statistics (in char * ticks)\n");

	for (area = area_first,i=0; area != NULL; area = area->next,i++) {
		if (i % 2 == 0) 
			buf_add(output, "\n");
		buf_printf(output, "%-20s %-8lu       ",
			   area->name, area->count);
	}
	buf_add(output, "\n\n");
	page_to_char(buf_string(output), ch);
	buf_free(output);
}

void do_ititle(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	argument = one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0')  {
		do_help(ch, "'WIZ ITITLE'");
		return;
	}

	victim = get_char_world(ch, arg);
	if (victim == NULL)  {
		char_puts("Nobody is playing with that name.\n", ch);
		return;
	}

	if (IS_NPC(victim)) {
		char_puts("Not on NPC's.\n", ch);
		return;
	}

	if (argument[0] == '\0') {
		char_puts("Change the title to what?\n", ch);
		return;
	}

	set_title(victim, argument);
	char_puts("Ok.\n", ch);
}

/*
 * .gz files are checked for too, just in case.
 */

void do_rename(CHAR_DATA* ch, const char *argument)
{
	char old_name[MAX_INPUT_LENGTH], 
	     new_name[MAX_INPUT_LENGTH];
	char strsave[PATH_MAX];
	char *file_name;

	CHAR_DATA *victim;
	clan_t *clan;
		
	argument = first_arg(argument, old_name, sizeof(old_name), FALSE); 
		   first_arg(argument, new_name, sizeof(new_name), FALSE);
		
	if (IS_NULLSTR(old_name) || IS_NULLSTR(new_name)) {
		do_help(ch, "'WIZ ITITLE'");
		return;
	}
		
	victim = get_char_world(ch, old_name);
		
	if (!victim) {
		char_puts("There is no such a person online.\n",ch);
		return;
	}
		
	if (IS_NPC(victim)) {   
		char_puts("You cannot use Rename on NPCs.\n",ch);
		return;
	}

	if (!IS_TRUSTED(ch, trust_level(victim))) {
		char_puts("You failed.\n",ch);
		return;
	}
		
	if (!victim->desc || (victim->desc->connected != CON_PLAYING)) {
		char_puts("This player has lost his link or is inside a pager or the like.\n",ch);
		return;
	}

	if (!pc_name_ok(new_name)) {
		char_puts("The new name is illegal.\n",ch);
		return;
	}

/* delete old pfile */
	if (str_cmp(new_name, victim->name)) {
		DESCRIPTOR_DATA *d;
		OBJ_DATA *obj;

		for (d = descriptor_list; d; d = d->next)
			if (d->character
			&&  !str_cmp(d->character->name, new_name)) {
				char_puts ("A player with the name you specified already exists!\n",ch);
				return;
			}

		/* check pfile */
		file_name = capitalize(new_name);
		if (dfexist(PLAYER_PATH, file_name)) {
			char_puts("A player with that name already exists!\n",
				  ch);
			return;		
		}

		/* check .gz pfile */
		snprintf(strsave, sizeof(strsave), "%s.gz", file_name);
		if (dfexist(PLAYER_PATH, strsave)) {
			char_puts ("A player with that name already exists in a compressed file!\n",ch);
			return;		
		}

		if ((clan = clan_lookup(victim->clan))) {
			bool touched = FALSE;

			if (name_delete(&clan->member_list, victim->name,
					NULL, NULL)) {
				touched = TRUE;
				name_add(&clan->member_list, new_name,
					 NULL, NULL);
			}

			if (name_delete(&clan->leader_list, victim->name,
					NULL, NULL)) {
				touched = TRUE;
				name_add(&clan->leader_list, new_name,
					 NULL, NULL);
			}

			if (name_delete(&clan->second_list, victim->name,
					NULL, NULL)) {
				touched = TRUE;
				name_add(&clan->second_list, new_name,
					 NULL, NULL);
			}

			if (touched)
				clan_save(clan);
		}

		/* change object owners */
		for (obj = object_list; obj; obj = obj->next)
			if (!str_cmp(mlstr_mval(&obj->owner), victim->name)) {
				mlstr_destroy(&obj->owner);
				mlstr_init(&obj->owner, new_name);
			}

		dunlink(PLAYER_PATH, capitalize(victim->name)); 
	}

	/*
	 * NOTE: Players who are level 1 do NOT get saved under a new name 
	 */
	free_string(victim->name);
	victim->name = str_dup(new_name);
	mlstr_destroy(&victim->short_descr);
	mlstr_init(&victim->short_descr, new_name);
	char_save(victim, 0);
		
	char_puts("Character renamed.\n", ch);
	act_puts("$n has renamed you to $N!",
		 ch, NULL, victim, TO_VICT, POS_DEAD);
} 

void do_notitle(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	if (argument[0] == '\0') {
		do_help(ch, "'WIZ NOTITLE'");
		return;
	}

	argument = one_argument(argument, arg, sizeof(arg));

	if ((victim = get_char_world(ch ,arg)) == NULL) {
		char_puts("He is not currently playing.\n", ch);
		return;
	}
	 
	TOGGLE_BIT(PC(victim)->plr_flags, PLR_NOTITLE);
	if (!IS_SET(PC(victim)->plr_flags, PLR_NOTITLE))
	 	char_puts("You can change your title again.\n", victim);
	else 
		char_puts("You won't be able to change your title anymore.\n",
			  victim);
	char_puts("Ok.\n", ch);
}
	   
void do_noaffect(CHAR_DATA *ch, const char *argument)
{
	AFFECT_DATA *paf,*paf_next;
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	if (!IS_IMMORTAL(ch))
		return;

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		do_help(ch, "'WIZ NOAFFECT'");
		return;
	}

	if ((victim = get_char_world(ch, arg)) == NULL) {
		char_puts("He is not currently playing.\n", ch);
		return;
	}
	 
	for (paf = victim->affected; paf != NULL; paf = paf_next) {
		paf_next = paf->next;
		if (paf->duration >= 0) {
			skill_t *sk;

			if ((sk = skill_lookup(paf->type))
			&&  !IS_NULLSTR(sk->msg_off))
				act_puts(sk->msg_off, victim, NULL, NULL, 
					 TO_CHAR, POS_RESTING);
		  
			affect_remove(victim, paf);
		}
	}
}

void do_affrooms(CHAR_DATA *ch, const char *argument)
{
	ROOM_INDEX_DATA *room;
	ROOM_INDEX_DATA *room_next;
	AFFECT_DATA *af;
	int count = 0;

	if (!top_affected_room) 
		char_puts("No affected rooms.\n", ch);

	for (room = top_affected_room; room ; room = room_next) {
		room_next = room->aff_next;
		for (af = room->affected; af; af = af->next) {
			count++;
			char_printf(ch,
				    "%d) [Vnum: %5d] "
				    "spell '{c%s{x', owner: %s, level {c%d{x "
				    "for {c%d{x hours.\n",
				    count,
				    room->vnum,
				    af->type,
				    mlstr_mval(&af->owner->short_descr),
				    af->level,
				    af->duration);
		}
	}
}

void do_msgstat(CHAR_DATA *ch, const char *argument)
{
	varr *v;
	msg_t *mp;
	int i;
	BUFFER *output;

	if (argument[0] == '\0') {
		for (i = 0; i < MAX_MSG_HASH; i++) {
			varr *v = msg_hash_table+i;
			char_printf(ch, "%3d: %d msgs\n", i, v->nused);
		}
		return;
	}

	if (!is_number(argument)) {
		do_help(ch, "'WIZ STAT'");
		return;
	}

	i = atoi(argument);
	if (i < 0 || i >= MAX_MSG_HASH) {
		char_printf(ch, "Valid hash key range is 0..%d\n",
			    MAX_KEY_HASH);
		return;
	}

	v = msg_hash_table+i;
	output = buf_new(-1);
	buf_printf(output, "Dumping msgs with hash #%d\n", i);
	for (i = 0; i < v->nused; i++) {
		mp = VARR_GET(v, i);
		mlstr_dump(output, str_empty, &mp->ml);
		buf_add(output, "\n");
	}
	page_to_char(buf_string(output), ch);
	buf_free(output);
}

void do_grant(CHAR_DATA *ch, const char *argument)
{
	cmd_t *cmd;
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	argument = one_argument(argument, arg1, sizeof(arg1));
	argument = one_argument(argument, arg2, sizeof(arg2));
	if (arg1[0] == '\0') {
		do_help(ch, "'WIZ GRANT'");
		return;
	}

	if ((victim = get_char_room(ch, arg1)) == NULL) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	if (IS_NPC(victim)) {
		char_puts("Not on NPC.\n", ch);
		return;
	}

	if (arg2[0] == '\0') {
		char_printf(ch, "Granted commands for %s: [%s]\n",
			    victim->name, PC(victim)->granted);
		return;
	}

	if (is_number(arg2)) {
		int i;
		int lev = atoi(arg2);

		if (lev < LEVEL_IMMORTAL) {
			char_printf(ch, "grant: granted level must be at least %d\n", LEVEL_IMMORTAL);
			return;
		}

		if (lev > trust_level(ch)) {
			char_puts("grant: granted level cannot be higher"
				  " than yours.\n", ch);
			return;
		}

		for (i = 0; i < commands.nused; i++) {
			cmd = VARR_GET(&commands, i);

			if (cmd->min_level < LEVEL_IMMORTAL
			||  cmd->min_level > lev)
				continue;

			name_add(&PC(victim)->granted, cmd->name,
				 ch, "grant");
		}

		return;
	}

	for (; arg2[0]; argument = one_argument(argument, arg2, sizeof(arg2))) {
		if ((cmd = cmd_lookup(arg2)) == NULL
		&&  str_cmp(arg2, "none")
		&&  str_cmp(arg2, "all")) {
			char_printf(ch, "%s: command not found.\n", arg2);
			continue;
		}

		if (cmd) {
			CHAR_DATA *vch = GET_ORIGINAL(ch);

			if (cmd->min_level < LEVEL_IMMORTAL) {
				char_printf(ch, "%s: not a wizard command.\n",
					    arg2);
				continue;
			}

			if (cmd->min_level > trust_level(ch)
			&&  (IS_NPC(vch) ||
			     !is_name(cmd->name, PC(vch)->granted))) {
				char_puts("grant: cmd min level cannot be "
					  "higher than yours.\n", ch);
				return;
			}
		}

		name_toggle(&PC(victim)->granted, arg2, ch, "grant");
	}
}

void do_disable(CHAR_DATA *ch, const char *argument)
{
	cmd_t *cmd;
	char arg[MAX_INPUT_LENGTH];

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		do_help(ch, "'WIZ ENABLE DISABLE'");
		return;
	}

	if (!str_cmp(arg, "?")) {
		int i;
		char_puts("Disabled commands:\n", ch);
		for (i = 0; i < commands.nused; i++) {
			cmd = VARR_GET(&commands, i);
			if (IS_SET(cmd->cmd_flags, CMD_DISABLED))
				char_printf(ch, "%s\n", cmd->name);
		}
		return;
	}

	for (; arg[0]; argument = one_argument(argument, arg, sizeof(arg))) {
		if ((cmd = cmd_lookup(arg)) == NULL) {
			char_printf(ch, "%s: command not found.\n", arg);
			continue;
		}

		if (!str_cmp(cmd->name, "enable")) {
			char_puts("'enable' command cannot be disabled.\n",
				  ch);
			return;
		}

		SET_BIT(cmd->cmd_flags, CMD_DISABLED);
		char_printf(ch, "%s: command disabled.\n", cmd->name);
	}
}

void do_enable(CHAR_DATA *ch, const char *argument)
{
	cmd_t *cmd;
	char arg[MAX_INPUT_LENGTH];

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		do_help(ch, "'WIZ ENABLE DISABLE'");
		return;
	}

	for (; arg[0]; argument = one_argument(argument, arg, sizeof(arg))) {
		if ((cmd = cmd_lookup(arg)) == NULL) {
			char_printf(ch, "%s: command not found.\n", arg);
			continue;
		}

		REMOVE_BIT(cmd->cmd_flags, CMD_DISABLED);
		char_printf(ch, "%s: command enabled.\n", cmd->name);
	}
}

void do_qtarget(CHAR_DATA *ch, const char *argument)
{
	int low, high;
	char arg[MAX_INPUT_LENGTH];
	AFFECT_DATA af;
	CHAR_DATA *vch;

	argument = one_argument(argument, arg, sizeof(arg));
	low = atoi(arg);
	argument = one_argument(argument, arg, sizeof(arg));
	high = atoi(arg);
	if ((vch = get_char_room(ch, argument)) == NULL) {
		char_puts("They are not here.\n", ch);
		return;
	}

	if (!IS_NPC(vch)) {
		char_puts("Not on PC's.\n", ch);
		return;
	}

	af.where	= TO_AFFECTS;
	af.type		= "qtarget";
	af.level	= low;
	af.duration	= -1;
	af.modifier	= high;
	af.bitvector	= AFF_QUESTTARGET;
	af.location	= APPLY_NONE;
	affect_to_char(vch, &af);
}

void do_sla(CHAR_DATA *ch, const char *argument)
{
	char_puts("If you want to SLAY, spell it out.\n", ch);
	return;
}

void do_slay(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *victim;
	char arg[MAX_INPUT_LENGTH];

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		do_help(ch, "'WIZ SLAY'");
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	if (ch == victim) {
		char_puts("Suicide is a mortal sin.\n", ch);
		return;
	}

	if (IS_IMMORTAL(victim)) {
		char_puts("You failed.\n", ch);
		return;
	}

	act("You slay $M in cold blood!", ch, NULL, victim, TO_CHAR);
	act("$n slays you in cold blood!", ch, NULL, victim, TO_VICT);
	act("$n slays $N in cold blood!", ch, NULL, victim, TO_NOTVICT);
	raw_kill(ch, victim);
}

void do_ban(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];

	argument = one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		ban_t *pban;

		if (ban_list == NULL) {
			char_puts("No ban rules defined.\n", ch);
			return;
  		}

		char_puts("Ban rules:\n", ch);
		for (pban = ban_list; pban; pban = pban->next)
			char_printf(ch, "%s\n", format_ban(pban));
		return;
	}

	if (!str_prefix(arg, "add"))
		ban_add(ch, argument);
	else if (!str_prefix(arg, "delete"))
		ban_delete(ch, argument);
	else
		dofun("help", ch, "'WIZ BAN'");
}

void do_memory(CHAR_DATA *ch, const char *argument)
{
	extern int str_count;
	extern int str_real_count;
#if STR_ALLOC_MEM
	extern int str_alloc_mem;
#endif
	extern int npc_count;
	extern int npc_free_count;
	extern int pc_count;
	extern int pc_free_count;
	extern int dvdata_count;
	extern int dvdata_real_count;

	char_printf(ch, "Affects  : %d (%d bytes)\n",
		    top_affect, top_affect * sizeof(AFFECT_DATA));
	char_printf(ch, "Areas    : %d (%d bytes)\n",
		    top_area, top_area * sizeof(AREA_DATA));
	char_printf(ch, "ExDes    : %d (%d bytes)\n",
		    top_ed, top_ed * sizeof(ED_DATA));
	char_printf(ch, "Exits    : %d (%d bytes)\n",
		    top_exit, top_exit * sizeof(EXIT_DATA));
	char_printf(ch, "Helps    : %d (%d bytes)\n",
		    top_help, top_help * sizeof(HELP_DATA));
	char_printf(ch, "Socials  : %d (%d bytes)\n",
		    socials.nused, socials.nused * sizeof(social_t));
	char_printf(ch, "Mob idx  : %d (%d bytes, %d old, max vnum %d)\n",
		    top_mob_index, top_mob_index * sizeof(MOB_INDEX_DATA),
		    top_mob_index - newmobs, top_vnum_mob); 
	char_printf(ch, "Mobs     : %d (%d (%d) bytes), "
			"%d free (%d (%d) bytes)\n",
		    npc_count,
		    npc_count * (sizeof(CHAR_DATA) + sizeof(NPC_DATA)),
		    npc_count * (sizeof(CHAR_DATA) + sizeof(NPC_DATA) +
				 sizeof(memchunk_t)),
		    npc_free_count,
		    npc_free_count * (sizeof(CHAR_DATA) + sizeof(NPC_DATA)),
		    npc_free_count * (sizeof(CHAR_DATA) + sizeof(NPC_DATA) +
				      sizeof(memchunk_t)));
	char_printf(ch, "Players  : %d (%d (%d) bytes), "
			"%d free (%d (%d) bytes)\n",
		    pc_count,
		    pc_count * (sizeof(CHAR_DATA) + sizeof(PC_DATA)),
		    pc_count * (sizeof(CHAR_DATA) + sizeof(PC_DATA) +
				 sizeof(memchunk_t)),
		    pc_free_count,
		    pc_free_count * (sizeof(CHAR_DATA) + sizeof(PC_DATA)),
		    pc_free_count * (sizeof(CHAR_DATA) + sizeof(PC_DATA) +
				      sizeof(memchunk_t)));
	char_printf(ch, "Obj idx  : %d (%d bytes, %d old, max vnum %d)\n",
		    top_obj_index, top_obj_index * sizeof(OBJ_INDEX_DATA),
		    top_obj_index - newobjs, top_vnum_obj); 
	char_printf(ch, "Objs     : %d (%d (%d) bytes, %d free)\n",
		    obj_count,
		    obj_count * sizeof(OBJ_DATA),
		    obj_count * (sizeof(OBJ_DATA) + sizeof(memchunk_t)),
		    obj_free_count);
	char_printf(ch, "Resets   : %d (%d bytes)\n",
		    top_reset, top_reset * sizeof(RESET_DATA));
	char_printf(ch, "Rooms    : %d (%d (%d) bytes, max vnum %d)\n",
		    top_room,
		    top_room * sizeof(ROOM_INDEX_DATA),
		    top_room * (sizeof(ROOM_INDEX_DATA) + sizeof(memchunk_t)),
		    top_vnum_room);
	char_printf(ch, "Shops    : %d (%d bytes)\n",
		    top_shop, top_shop * sizeof(SHOP_DATA));
	char_printf(ch, "Buffers  : %d (%d bytes)\n",
					nAllocBuf, sAllocBuf);
	char_printf(ch,
#if STR_ALLOC_MEM
		    "strings  : %d (%d allocated, %d bytes)\n",
#else
		    "strings  : %d (%d allocated)\n",
#endif
		    str_count, str_real_count,
#if STR_ALLOC_MEM
		    str_alloc_mem
#endif
		    );
	char_printf(ch, "dvdata   : %d (%d allocated, %d bytes)\n",
		    dvdata_count, dvdata_real_count,
		    dvdata_real_count * sizeof(dvdata_t));
}

void do_dump(CHAR_DATA *ch, const char *argument)
{
	int count,count2,num_pcs,aff_count;
	CHAR_DATA *fch;
	MOB_INDEX_DATA *pMobIndex;
	OBJ_DATA *obj;
	OBJ_INDEX_DATA *pObjIndex;
	ROOM_INDEX_DATA *room;
	EXIT_DATA *exit;
	PC_DATA *pc;
	DESCRIPTOR_DATA *d;
	AFFECT_DATA *af;
	FILE *fp;
	int vnum,nMatch = 0;

	if ((fp = dfopen(TMP_PATH, "mem.dmp", "w")) == NULL)
		return;

	/* report use of data structures */
	
	num_pcs = 0;
	aff_count = 0;

	/* mobile prototypes */
	fprintf(fp,"MobProt	%4d (%8d bytes)\n",
		top_mob_index, top_mob_index * (sizeof(*pMobIndex))); 

	/* mobs */
	count = 0;
	for (fch = char_list; fch != NULL; fch = fch->next)
	{
		count++;
		if (PC(fch) != NULL)
		    num_pcs++;
		for (af = fch->affected; af != NULL; af = af->next)
		    aff_count++;
	}

	fprintf(fp,"Mobs	%4d (%8d bytes)\n",
		count, count * (sizeof(*fch)));

	fprintf(fp,"Pcdata	%4d (%8d bytes)\n",
		num_pcs, num_pcs * (sizeof(*pc)));

	/* descriptors */
	count = 0; count2 = 0;
	for (d = descriptor_list; d != NULL; d = d->next)
		count++;
	for (d= descriptor_free; d != NULL; d = d->next)
		count2++;

	fprintf(fp, "Descs	%4d (%8d bytes), %2d free (%d bytes)\n",
		count, count * (sizeof(*d)), count2, count2 * (sizeof(*d)));

	/* object prototypes */
	for (vnum = 0; nMatch < top_obj_index; vnum++)
		if ((pObjIndex = get_obj_index(vnum)) != NULL)
		{
		    for (af = pObjIndex->affected; af != NULL; af = af->next)
			aff_count++;
		    nMatch++;
		}

	fprintf(fp,"ObjProt	%4d (%8d bytes)\n",
		top_obj_index, top_obj_index * (sizeof(*pObjIndex)));

	/* objects */
	count = 0;
	for (obj = object_list; obj != NULL; obj = obj->next) {
		count++;
		for (af = obj->affected; af != NULL; af = af->next)
		    aff_count++;
	}

	fprintf(fp,"Objs	%4d (%8d bytes)\n",
		count, count * (sizeof(*obj)));

	/* affects */
	fprintf(fp,"Affects	%4d (%8d bytes)\n",
		aff_count, aff_count * (sizeof(*af)));

	/* rooms */
	fprintf(fp,"Rooms	%4d (%8d bytes)\n",
		top_room, top_room * (sizeof(*room)));

	 /* exits */
	fprintf(fp,"Exits	%4d (%8d bytes)\n",
		top_exit, top_exit * (sizeof(*exit)));

	fclose(fp);

	/* start printing out mobile data */
	if ((fp = dfopen(TMP_PATH, "mob.dmp", "w")) == NULL)
		return;

	fprintf(fp,"\nMobile Analysis\n");
	fprintf(fp,  "---------------\n");
	nMatch = 0;
	for (vnum = 0; nMatch < top_mob_index; vnum++)
		if ((pMobIndex = get_mob_index(vnum)) != NULL)
		{
		    nMatch++;
		    fprintf(fp,"#%-4d %3d active %3d killed     %s\n",
			pMobIndex->vnum,pMobIndex->count,
			pMobIndex->killed,mlstr_mval(&pMobIndex->short_descr));
		}
	fclose(fp);

	/* start printing out object data */
	if ((fp = dfopen(TMP_PATH, "obj.dmp", "w")) == NULL)
		return;

	fprintf(fp,"\nObject Analysis\n");
	fprintf(fp,  "---------------\n");
	nMatch = 0;
	for (vnum = 0; nMatch < top_obj_index; vnum++)
		if ((pObjIndex = get_obj_index(vnum)) != NULL)
		{
		    nMatch++;
		    fprintf(fp,"#%-4d %3d active %3d reset      %s\n",
			pObjIndex->vnum,pObjIndex->count,
			pObjIndex->reset_num,
			mlstr_mval(&pObjIndex->short_descr));
		}

	/* close file */
	fclose(fp);
}

void do_mob(CHAR_DATA *ch, const char *argument)
{
	/*
	 * Security check!
	 */
	if (ch->desc)
		return;
	mob_interpret(ch, argument);
}

/* 
 * Displays MOBprogram triggers of a mobile
 *
 * Syntax: mpstat [name]
 */
void do_mpstat(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_STRING_LENGTH];
	MPTRIG  *mptrig;
	CHAR_DATA *victim;
	NPC_DATA *npc;
	int i;

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		char_puts("Mpstat whom?\n", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg)) == NULL) {
		char_puts("No such creature.\n", ch);
		return;
	}

	if (!IS_NPC(victim)) {
		char_puts("That is not a mobile.\n", ch);
		return;
	}

	char_printf(ch, "Mobile #%-6d [%s]\n",
		victim->pMobIndex->vnum, mlstr_mval(&victim->short_descr));

	npc = NPC(victim);
	char_printf(ch, "Delay   %-6d [%s]\n",
		npc->mprog_delay,
		npc->mprog_target == NULL ?
		"No target" : npc->mprog_target->name);

	if (!victim->pMobIndex->mptrig_types) {
		char_puts("[No programs set]\n", ch);
		return;
	}

	for (i = 0, mptrig = victim->pMobIndex->mptrig_list; mptrig != NULL;
						mptrig = mptrig->next) {
		char_printf(ch,
			"[%2d] Trigger [%-8s] Program [%4d] Phrase [%s]\n",
			++i,
			flag_string(mptrig_types, mptrig->type),
			mptrig->vnum,
			mptrig->phrase);
	}
}

