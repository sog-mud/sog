/*
 * $Id: act_wiz.c,v 1.251 2000-09-11 07:09:35 fjoe Exp $
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

#if defined(SUNOS) || defined(SVR4) || defined(LINUX)
#	include <crypt.h>
#endif

#if defined(BSD44)
#	include <fnmatch.h>
#else
#	include "../compat/fnmatch.h"
#endif

#include "merc.h"
#include "obj_prog.h"
#include "cmd.h"
#include "db.h"
#include "ban.h"
#include "socials.h"
#include "mob_prog.h"
#include "string_edit.h"

#include "quest.h"
#include "fight.h"

/* command procedures needed */
DECLARE_DO_FUN(do_rstat	);
DECLARE_DO_FUN(do_mstat	);
DECLARE_DO_FUN(do_dstat	);
DECLARE_DO_FUN(do_ostat	);
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
		if (!IS_OBJ_STAT(obj, ITEM_ENCHANTED))
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

void do_nonote(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	bool loaded = FALSE;

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		do_help(ch, "'WIZ NONOTE'");
		return;
	}

	if ((victim = get_char_world(ch, arg)) == NULL) {
		if ((victim = char_load(arg, LOAD_F_NOCREATE)) == NULL) {
			char_puts("They aren't here.\n", ch);
			return;
		}
		loaded = TRUE;
	}

	if (!IS_TRUSTED(ch, trust_level(victim))) {
		char_puts("You failed.\n", ch);
		goto cleanup;
	}

	if (IS_SET(victim->comm, COMM_NONOTE)) {
		REMOVE_BIT(victim->comm, COMM_NONOTE);
		char_puts("You may write notes again.\n", victim);
		char_puts("NONOTE removed.\n", ch);
		wiznet("$N grants $i right to write notes",
			ch, victim, WIZ_PENALTIES, WIZ_SECURE, 0);
	} else {
		SET_BIT(victim->comm, COMM_NONOTE);
		char_puts("Your notes will be sent to Abyss now.\n", victim);
		char_puts("NONOTE set.\n", ch);
		wiznet("$N revokes $i's right to write notes",
			ch, victim, WIZ_PENALTIES, WIZ_SECURE, 0);
	}

cleanup:
	if (loaded) {
		char_save(victim, SAVE_F_PSCAN);
		char_nuke(victim);
	}
}

/* RT nochannels command, for those spammers */
void do_nochannels(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	bool loaded = FALSE;
	
	one_argument(argument, arg, sizeof(arg));
	
	if (arg[0] == '\0') {
		do_help(ch, "'WIZ NOCHANNEL'");
		return;
	}
	
	if ((victim = get_char_world(ch, arg)) == NULL) {
		if ((victim = char_load(arg, LOAD_F_NOCREATE)) == NULL) {
			char_puts("They aren't here.\n", ch);
			return;
		}
		loaded = TRUE;
	}
	
	if (!IS_TRUSTED(ch, trust_level(victim))) {
		char_puts("You failed.\n", ch);
		goto cleanup;
	}
	
	TOGGLE_BIT(victim->chan, CHAN_NOCHANNELS);
	if (!IS_SET(victim->chan, CHAN_NOCHANNELS)) {
		char_puts("The gods have restored your channel priviliges.\n", 
			  victim);
		char_puts("NOCHANNELS removed.\n", ch);
		wiznet("$N restores channels to $i",
			ch, victim, WIZ_PENALTIES, WIZ_SECURE, 0);
	} else {
		char_puts("The gods have revoked your channel priviliges.\n", 
			  victim);
		char_puts("NOCHANNELS set.\n", ch);
		wiznet("$N revokes $i's channels.",
			ch, victim, WIZ_PENALTIES, WIZ_SECURE, 0);
	}

cleanup:
	if (loaded) {
		char_save(victim, SAVE_F_PSCAN);
		char_nuke(victim);
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

	log(LOG_BUG, "do_disconnect: desc not found");
	char_puts("Descriptor not found!\n", ch);
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
			if (!IS_NPC(ch) && PC(ch)->bamfout[0] != '\0')
				act("$t", ch, PC(ch)->bamfout, rch, TO_VICT);
			else {
				act("$n leaves in a swirling mist.",
				    ch, NULL, rch, TO_VICT);
			}
		}
	}

	pet = GET_PET(ch);
	char_from_room(ch);
	char_to_room(ch, location);

	for (rch = location->people; rch; rch = rch->next_in_room) {
		if (IS_TRUSTED(rch, ch->invis_level)) {
			if (!IS_NPC(ch) && PC(ch)->bamfin[0])
				act("$t", ch, PC(ch)->bamfin, rch, TO_VICT);
			else {
				act("$n appears in a swirling mist.",
				    ch, NULL, rch, TO_VICT);
			}
		}
	}

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
		flag_string(item_types, obj->item_type),
		obj->pObjIndex->reset_num);

	mlstr_dump(output, "Short description: ", &obj->short_descr);
	mlstr_dump(output, "Long description: ", &obj->description);

	buf_printf(output, "Wear bits: %s\n",
		   flag_string(wear_flags, obj->wear_flags));
	buf_printf(output, "Stat bits: %s\n",
		   flag_string(stat_flags, obj->stat_flags));
	buf_printf(output, "Obj bits: %s\n",
		   flag_string(obj_flags, obj->pObjIndex->obj_flags));
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
	if (obj->altar) {
		buf_printf(output, "Altar: %s (vnum %d)\n",
			mlstr_mval(&obj->altar->room->name),
			obj->altar->room->vnum);
	}

	objval_show(output, obj->item_type, obj->value);

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

	if (!IS_OBJ_STAT(obj, ITEM_ENCHANTED))
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
	print_cc_vexpr(&obj->pObjIndex->restrictions, "Restrictions:", output);
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
	CHAR_DATA *victim;
	CHAR_DATA *pet;
	BUFFER *output;
	bool loaded = FALSE;

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		char_puts("Stat whom?\n", ch);
		return;
	}

	if ((victim = get_char_world(ch, argument)) == NULL) {
		if ((victim = char_load(argument, LOAD_F_NOCREATE)) == NULL) {
			char_puts("They aren't here.\n", ch);
			return;
		}
		loaded = TRUE;
	}

	output = buf_new(-1);

	buf_printf(output, "Name: [%s]  ", victim->name);
	if (IS_NPC(victim)) {
		buf_printf(output, "Vnum: [%d]  Reset Zone: [%s]\n",
			   victim->pMobIndex->vnum,
			   NPC(victim)->zone ? NPC(victim)->zone->name : "?");
	} else 
		buf_printf(output, "%sLINE\n", loaded ? "OFF" : "ON");

	buf_printf(output, 
		"Race: %s (%s)  Room: [%d]\n",
		victim->race, ORG_RACE(victim),
		victim->in_room == NULL ? 0 : victim->in_room->vnum);

	mlstr_dump(output, "Gender: ", &victim->gender);

	if (IS_NPC(victim)) {
		buf_printf(output,"Group: [%d]  Count: [%d]  Killed: [%d]\n",
			   victim->pMobIndex->group,
			   victim->pMobIndex->count,
			   victim->pMobIndex->killed);
	}

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


	buf_printf(output, "Hp: %d/%d  Mana: %d/%d  Move: %d/%d  Practices: %d Luck: %d\n", 
		victim->hit,         victim->max_hit,
		victim->mana,        victim->max_mana,
		victim->move,        victim->max_move,
		IS_NPC(ch) ? 0 : PC(victim)->practice,
		GET_LUCK(victim));
		
	if (IS_NPC(victim))
		snprintf(buf, sizeof(buf), "%d", victim->alignment);
	else  {
/*
		buf_printf(output, "It belives the religion of %s.\n",
			religion_table[PC(victim)->religion].leader);
*/
		snprintf(buf, sizeof(buf), "%s-%s", 
			 flag_string(ethos_table, victim->ethos),
			 flag_string(align_names, NALIGN(victim)));
	}

	buf_printf(output,
		"Lv: [%d + %d]  Class: [%s]  Align: [%s]  Gold: [%d]  Silver: [%d]  Exp: [%d]\n",
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
			   victim->damtype);
	}

	buf_printf(output, "Fighting: %s Deaths: %d Carry number: %d  Carry weight: %d\n",
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
		buf_printf(output, "Age: [%d]  Played: [%d]  Timer: [%d]\n",
			   get_age(victim), get_hours(victim),
			   PC(victim)->idle_timer);

		buf_printf(output, "Plr: [%s]\n",
			   flag_string(plr_flags, PC(victim)->plr_flags));
	}
	
	if (victim->comm)
		buf_printf(output, "Comm: [%s]\n",
			   flag_string(comm_flags, victim->comm));

	if (victim->chan)
		buf_printf(output, "Chan: [%s]\n",
			   flag_string(chan_flags, victim->chan));

	if (IS_NPC(victim) && victim->pMobIndex->off_flags)
		buf_printf(output, "Offense: [%s]\n",
			   flag_string(off_flags,
				       victim->pMobIndex->off_flags));

	buf_printf(output, "Form: [%s]\n",
		   flag_string(form_flags, victim->form));
	buf_printf(output, "Parts: [%s]\n",
		   flag_string(part_flags, victim->parts));

	if (victim->affected_by)
		buf_printf(output, "Affected by %s\n", 
			   flag_string(affect_flags, victim->affected_by));

	if (victim->has_invis)
		buf_printf(output, "Has '%s'\n", 
			   flag_string(id_flags, victim->has_invis));

	if (victim->has_detect)
		buf_printf(output, "Has detection of '%s'\n", 
			   flag_string(id_flags, victim->has_detect));

	pet = GET_PET(victim);
	buf_printf(output, "Master: %s  Leader: %s  Pet: %s\n",
		victim->master	? victim->master->name	: "(none)",
		victim->leader	? victim->leader->name	: "(none)",
		pet		? pet->name		: "(none)");

	/* OLC */
	if (!IS_NPC(victim))
		buf_printf(output, "Security: [%d]\n",
			   PC(victim)->security);

	mlstr_dump(output, "Short description: ", &victim->short_descr);
	if (IS_NPC(victim)) {
		mlstr_dump(output, "Long description: ", &victim->long_descr);
		if (victim->pMobIndex->spec_fun != 0) {
			buf_printf(output, "Mobile has special procedure %s.\n",
				   mob_spec_name(victim->pMobIndex->spec_fun));
		}
	}

	show_affects2(ch, victim, output);

	if (!varr_isempty(&victim->sk_affected)) {
		buf_add(output, "Skill affects:\n");
		varr_foreach(&victim->sk_affected, print_sa_cb, output);
	}

	if (!IS_NPC(victim)) {
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
		} else {
			buf_printf(output,
				   "QuestPnts: [%d]  Questnext: [%d]  NOT QUESTING\n",
				   PC(victim)->questpoints,
				   PC(victim)->questtime < 0 ?
					-PC(victim)->questtime : 0);
		}

		buf_add(output, "Quest Troubles: ");
		qtrouble_dump(output, victim);
		buf_add(output, "\n");

		if (!IS_NULLSTR(PC(victim)->twitlist))
			buf_printf(output, "Twitlist: [%s]\n",
				   PC(victim)->twitlist);
	}

	buf_printf(output, "Last fight time: [%s] %s\n",
		   victim->last_fight_time != 1 ?
		  	strtime(victim->last_fight_time) : "NONE",
		   IS_PUMPED(victim) ? "(adrenalin is gushing)" : str_empty);

	buf_printf(output, "Wait state: %d\n", ch->wait);
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

	if (loaded)
		char_nuke(victim);
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
				PERS(in_obj->carried_by, ch),
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
	bool loaded = FALSE;

	if (argument[0] == '\0') {
		do_help(ch, "'WIZ PROTECT'");
		return;
	}

	if ((victim = get_char_world(ch, argument)) == NULL) {
		if ((victim = char_load(argument, LOAD_F_NOCREATE)) == NULL) {
			char_puts("You can't find them.\n",ch);
			return;
		}
		loaded = TRUE;
	}

	if (IS_SET(victim->comm, COMM_SNOOP_PROOF)) {
		act_puts("$N is no longer snoop-proof.", ch, NULL, victim,
			 TO_CHAR, POS_DEAD);
		char_puts("Your snoop-proofing was just removed.\n", victim);
		REMOVE_BIT(victim->comm, COMM_SNOOP_PROOF);
	} else {
		act_puts("$N is now snoop-proof.", ch, NULL, victim, TO_CHAR,
			 POS_DEAD);
		char_puts("You are now immune to snooping.\n", victim);
		SET_BIT(victim->comm, COMM_SNOOP_PROOF);
	}

	if (loaded) {
		char_save(victim, SAVE_F_PSCAN);
		char_nuke(victim);
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
		t_obj = clone_obj(c_obj);
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

		clone = clone_obj(obj);
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

		clone = clone_mob(mob); 
		
		for (obj = mob->carrying; obj != NULL; obj = obj->next_content) {
			new_obj = clone_obj(obj);
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
			if (!OBJ_IS(obj, OBJ_NOPURGE))
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
	bool loaded = FALSE;

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		do_help(ch, "'WIZ FREEZE'");
		return;
	}

	if ((victim = get_char_world(ch, arg)) == NULL) {
		if ((victim = char_load(arg, LOAD_F_NOCREATE)) == NULL) {
			char_puts("They aren't here.\n", ch);
			return;
		}
		loaded = TRUE;
	} else if (IS_NPC(victim)) {
		char_puts("Not on NPC's.\n", ch);
		return;
	}

	if (!IS_TRUSTED(ch, trust_level(victim))) {
		char_puts("You failed.\n", ch);
		goto cleanup;
	}

	TOGGLE_BIT(PC(victim)->plr_flags, PLR_FREEZE);
	if (!IS_SET(PC(victim)->plr_flags, PLR_FREEZE)) {
		char_puts("You can play again.\n", victim);
		char_puts("FREEZE removed.\n", ch);
		wiznet("$N thaws $i.",
			ch, victim, WIZ_PENALTIES, WIZ_SECURE, 0);
	} else {
		char_puts("You can't do ANYthing!\n", victim);
		char_puts("FREEZE set.\n", ch);
		wiznet("$N puts $i in the deep freeze.",
			ch, victim, WIZ_PENALTIES, WIZ_SECURE, 0);
	}

cleanup:
	if (loaded) {
		char_save(victim, SAVE_F_PSCAN);
		char_nuke(victim);
	} else
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
	bool loaded = FALSE;

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		do_help(ch, "'WIZ NOEMOTE'");
		return;              
	}

	if ((victim = get_char_world(ch, arg)) == NULL) {
		if ((victim = char_load(arg, LOAD_F_NOCREATE)) == NULL) {
			char_puts("They aren't here.\n", ch);
			return;
		}
		loaded = TRUE;
	}

	if (!IS_TRUSTED(ch, trust_level(victim))) {
		char_puts("You failed.\n", ch);
		goto cleanup;
	}

	if (IS_SET(victim->comm, COMM_NOEMOTE)) {
		REMOVE_BIT(victim->comm, COMM_NOEMOTE);
		char_puts("You can emote again.\n", victim);
		char_puts("NOEMOTE removed.\n", ch);
		wiznet("$N restores emotes to $i.",
			ch, victim, WIZ_PENALTIES, WIZ_SECURE, 0);
	} else {
		SET_BIT(victim->comm, COMM_NOEMOTE);
		char_puts("You can't emote!\n", victim);
		char_puts("NOEMOTE set.\n", ch);
		wiznet("$N revokes $i's emotes.",
			ch, victim, WIZ_PENALTIES, WIZ_SECURE, 0);
	}

cleanup:
	if (loaded) {
		char_save(victim, SAVE_F_PSCAN);
		char_nuke(victim);
	}
}

void do_notell(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	bool loaded = FALSE;

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		do_help(ch, "'WIZ NOTELL'");
		return;
	}

	if ((victim = get_char_world(ch, arg)) == NULL) {
		if ((victim = char_load(arg, LOAD_F_NOCREATE)) == NULL) {
			char_puts("They aren't here.\n", ch);
			return;
		}
		loaded = TRUE;
	}

	if (!IS_TRUSTED(ch, trust_level(victim))) {
		char_puts("You failed.\n", ch);
		goto cleanup;
	}

	if (IS_SET(victim->comm, COMM_NOTELL)) {
		REMOVE_BIT(victim->comm, COMM_NOTELL);
		char_puts("You can tell again.\n", victim);
		char_puts("NOTELL removed.\n", ch);
		wiznet("$N restores tells to $i.",
			ch, victim, WIZ_PENALTIES, WIZ_SECURE, 0);
	} else {
		SET_BIT(victim->comm, COMM_NOTELL);
		char_puts("You can't tell!\n", victim);
		char_puts("NOTELL set.\n", ch);
		wiznet("$N revokes $i's tells.",
			ch, victim, WIZ_PENALTIES, WIZ_SECURE, 0);
	}

cleanup:
	if (loaded) {
		char_save(victim, SAVE_F_PSCAN);
		char_nuke(victim);
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
			INT(af.location) = APPLY_NONE;
			af.modifier = 0;
			af.bitvector = AFF_CALM;
			af.owner	= NULL;
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
		do_help(ch, "1.'WIZ SET'");
		return;
	}

	if (!str_prefix(arg, "mobile") || !str_prefix(arg, "character")) {
		do_mset(ch, argument);
		return;
	}

	if (!str_prefix(arg, "skill")) {
		do_sset(ch, argument);
		return;
	}

	if (!str_prefix(arg, "object")) {
		do_oset(ch, argument);
		return;
	}

	if (!str_prefix(arg, "room")) {
		do_rset(ch, argument);
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

	set_skill(victim, gmlstr_mval(&sk->sk_name), val);
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
		do_set(ch, str_empty);
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
			sn = gmlstr_mval(&sk->sk_name);

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

		if (CAN_WEAR(obj, ITEM_WEAR_CLANMARK)) {
			char_puts("You cannot string clan marks.\n", ch);
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
		do_set(ch, str_empty);
		return;
	}

	if ((obj = get_obj_world(ch, arg1)) == NULL) {
		char_puts("Nothing like that in heaven or earth.\n", ch);
		return;
	}

	value = atoi(arg3);

	if (!str_prefix(arg2, "timer")) {
		obj->timer = value;
		return;
	}
		
	if (!str_prefix(arg2, "owner")) {
		mlstr_destroy(&obj->owner);
		mlstr_init2(&obj->owner, arg3);
		return;
	}

	/*
	 * Generate usage message.
	 */
	do_set(ch, str_empty);
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
		do_set(ch, str_empty);
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
	do_set(ch, str_empty);
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
		buf_printf(output, "[%3d %12s] %s (%s)",
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

static void *
force_cb(void *vo, va_list ap)
{
	CHAR_DATA *vch = (CHAR_DATA *) vo;
	CHAR_DATA *ch = va_arg(ap, CHAR_DATA *);
	const char *argument = va_arg(ap, const char *);
	bool imms_only = va_arg(ap, bool);

	if (IS_NPC(vch))
		return vch;

	if (!IS_TRUSTED(ch, trust_level(vch))
	||  (imms_only && !IS_IMMORTAL(vch)))
		return NULL;
	act_puts("$n forces you to '$t'.",
		 ch, argument, vch, TO_VICT, POS_DEAD);
	interpret_raw(vch, argument, TRUE);
	return NULL;
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
		if (!IS_TRUSTED(ch, MAX_LEVEL - 3)) {
			char_puts("Not at your level!\n",ch);
			return;
		}

		vo_foreach(NULL, &iter_char_world, force_cb,
			   ch, argument, FALSE);
		char_puts("Ok.\n", ch);
		return;
	} else if (!str_cmp(arg, "gods")) {
		if (!IS_TRUSTED(ch, MAX_LEVEL - 2)) {
			char_puts("Not at your level!\n",ch);
			return;
		}
	
		vo_foreach(NULL, &iter_char_world, force_cb,
			   ch, argument, TRUE);
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
	bool loaded = FALSE;
	int level;

	argument = one_argument(argument, arg1, sizeof(arg1));
	argument = one_argument(argument, arg2, sizeof(arg2));

	if (arg1[0] == '\0' || arg2[0] == '\0' || !is_number(arg2)) {
		do_help(ch, "'WIZ ADVANCE'");
		return;
	}

	if ((victim = get_char_world(ch, arg1)) == NULL) {
		if ((victim = char_load(arg1, LOAD_F_NOCREATE)) == NULL) {
			char_puts("They aren't here.\n", ch);
			return;
		}
		loaded = TRUE;
	} else if (IS_NPC(victim)) {
		char_puts("Not on NPC's.\n", ch);
		return;
	}

	if ((level = atoi(arg2)) < 1 || level > MAX_LEVEL) {
		char_printf(ch, "Level must be in range 1..%d.\n", MAX_LEVEL);
		goto cleanup;
	}

	if (level > ch->level) {
		char_puts("Limited to your level.\n", ch);
		goto cleanup;
	}

	if (!IS_TRUSTED(ch, trust_level(victim))) {
		char_puts("You are not allowed to do that.\n", ch);
		goto cleanup;
	}

	advance(victim, level);

cleanup:
	if (loaded) {
		char_save(victim, SAVE_F_PSCAN);
		char_nuke(victim);
	}
}

void do_mset(CHAR_DATA *ch, const char *argument)
{
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	char arg3 [MAX_INPUT_LENGTH];
	char arg4 [MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int value, val2;
	const char *p;
	bool loaded = FALSE;
	bool altered = FALSE;

	argument = one_argument(argument, arg1, sizeof(arg1));
	argument = one_argument(argument, arg2, sizeof(arg2));
	p = one_argument(argument, arg3, sizeof(arg3));
	one_argument(p, arg4, sizeof(arg4));

	if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0') {
		do_set(ch, str_empty);
		return;
	}

	if ((victim = get_char_world(ch, arg1)) == NULL) {
		if ((victim = char_load(arg1, LOAD_F_NOCREATE)) == NULL) {
			char_puts("They aren't here.\n", ch);
			return;
		}
		loaded = TRUE;
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
		    goto cleanup;
		}

		victim->perm_stat[STAT_STR] = value;
		altered = TRUE;
		goto cleanup;
	}

	if (!str_cmp(arg2, "trouble")) {
		if (IS_NPC(victim)) {
			char_puts("Not on NPC's.\n", ch);
			goto cleanup;
		}
		
		if (value == -1 || val2 == -1) {
			char_puts("Usage: set char <name> trouble "
				  "<vnum> <value>.\n", ch);
			goto cleanup;
		}

		qtrouble_set(victim, value, val2+1);
		char_puts("Ok.\n", ch);
		altered = TRUE;
		goto cleanup;
	}

	if (!str_cmp(arg2, "security"))	{ /* OLC */
		if (IS_NPC(ch)) {
			char_puts("Si, claro.\n", ch);
			goto cleanup;
		}

		if (IS_NPC(victim)) {
			char_puts("Not on NPC's.\n", ch);
			goto cleanup;
		}

		if ((value > PC(ch)->security || value < 0)
		&& ch->level != MAX_LEVEL) {
			if (PC(ch)->security != 0)
				char_printf(ch, "Valid security is 0-%d.\n",
					    PC(ch)->security);
			else
				char_puts("Valid security is 0 only.\n", ch);
			goto cleanup;
		}
		PC(victim)->security = value;
		altered = TRUE;
		goto cleanup;
	}

	if (!str_cmp(arg2, "int"))
	{
	    if (value < 3 || value > get_max_train(victim,STAT_INT))
	    {
	        char_printf(ch, "Intelligence range is 3 to %d.\n",
			get_max_train(victim,STAT_INT));
	        goto cleanup;
	    }
	
	    victim->perm_stat[STAT_INT] = value;
		altered = TRUE;
	    goto cleanup;
	}

	if (!str_cmp(arg2, "wis"))
	{
		if (value < 3 || value > get_max_train(victim,STAT_WIS))
		{
		    char_printf(ch,
			"Wisdom range is 3 to %d.\n",get_max_train(victim,STAT_WIS));
		    goto cleanup;
		}

		victim->perm_stat[STAT_WIS] = value;
		altered = TRUE;
		goto cleanup;
	}

	if (!str_cmp(arg2, "qpoints")) {
		if (value == -1)
			value = 0;

		if (IS_NPC(victim)) {
			char_puts("Not on NPC's.\n", ch);
			goto cleanup;
		}

		PC(victim)->questpoints = value;
		altered = TRUE;
		goto cleanup;
	}

	if (!str_cmp(arg2, "qtime")) {
		if (IS_NPC(victim)) {
			char_puts("Not on NPC's.\n", ch);
			goto cleanup;
		}

		if (value == -1)
			value = 30;
		PC(victim)->questtime = value;
		altered = TRUE;
		goto cleanup;
	}

	if (!str_cmp(arg2, "dex"))
	{
		if (value < 3 || value > get_max_train(victim,STAT_DEX))
		{
		    char_printf(ch,
			"Dexterity ranges is 3 to %d.\n",
			get_max_train(victim,STAT_DEX));
		    goto cleanup;
		}

		victim->perm_stat[STAT_DEX] = value;
		 altered = TRUE;
		goto cleanup;
	}

	if (!str_cmp(arg2, "con"))
	{
		if (value < 3 || value > get_max_train(victim,STAT_CON))
		{
		    char_printf(ch,
			"Constitution range is 3 to %d.\n",
			get_max_train(victim,STAT_CON));
		    goto cleanup;
		}

		victim->perm_stat[STAT_CON] = value;
		 altered = TRUE;
		goto cleanup;
	}
	if (!str_cmp(arg2, "cha"))
	{
		if (value < 3 || value > get_max_train(victim,STAT_CHA))
		{
		    char_printf(ch,
			"Constitution range is 3 to %d.\n",
			get_max_train(victim,STAT_CHA));
		    goto cleanup;
		}

		victim->perm_stat[STAT_CHA] = value;
		 altered = TRUE;
		goto cleanup;
	}

	if (!str_prefix(arg2, "sex")) {
		int sex;

		if (IS_NPC(victim)) {
			sex = flag_value(gender_table, arg4);
			if (sex < 0) {
				char_puts("Valid values are:\n", ch);
				show_flags(ch, gender_table);
				goto cleanup;
			}
			mlstr_edit(&victim->gender, argument);
			goto cleanup;
		}

		sex = flag_value(gender_table, arg3);
		if (sex != SEX_MALE
		&&  sex != SEX_FEMALE) {
			char_puts("Sex should be one of 'male', 'female'.\n", ch);
			goto cleanup;
		}

		mlstr_destroy(&victim->gender);
		mlstr_init2(&victim->gender, flag_string(gender_table, sex));
		 altered = TRUE;
		goto cleanup;
	}

	if (!str_prefix(arg2, "class")) {
		class_t *cl;

		if (IS_NPC(victim)) {
			char_puts("Mobiles have no class.\n", ch);
			goto cleanup;
		}

		if ((cl = class_search(arg3)) == NULL) {
			BUFFER *output = buf_new(-1);
			buf_add(output, "Possible classes are: ");
			strkey_printall(&classes, output);
			send_to_char(buf_string(output), ch);
			buf_free(output);
			goto cleanup;
		}

		free_string(victim->class);
		victim->class = str_qdup(cl->name);
		spec_update(victim);
		PC(victim)->exp = exp_for_level(victim, victim->level);
		 altered = TRUE;
		goto cleanup;
	}

	if (!str_prefix(arg2, "clan")) {
		clan_t *cl, *clo;
		OBJ_DATA *mark;
		bool same_clan;

		if ((cl = clan_search(arg3)) == NULL) {
			BUFFER *output = buf_new(-1);
			buf_add(output, "Valid clan names are: ");
			strkey_printall(&clans, output);
			send_to_char(buf_string(output), ch);
			buf_free(output);
			goto cleanup;
		}

		same_clan = IS_CLAN(victim->clan, cl->name);
		if (!same_clan) {
			if (!IS_NPC(victim)
			&&  (clo = clan_lookup(victim->clan))
			&&  !IS_CLAN(clo->name, "none")) {
				clan_update_lists(clo, victim, TRUE);
				clan_save(clo);
			}

			free_string(victim->clan);
			if (IS_CLAN(cl->name, "none"))
				victim->clan = str_empty;
			else
				victim->clan = str_qdup(cl->name);
		}

		if ((mark = get_eq_char(victim, WEAR_CLANMARK)) != NULL) {
			obj_from_char(mark);
			extract_obj(mark, 0);
		}

		if (IS_NPC(victim)) {
			char_puts("Ok.\n", ch);
			goto cleanup;
		}

		if (!same_clan) {
			PC(victim)->clan_status = CLAN_COMMONER;
			if (!IS_CLAN(cl->name, "none")) {
				name_add(&cl->member_list, victim->name,
					 NULL, NULL);
				clan_save(cl);
			}

			spec_update(victim);
			update_skills(victim);
		}

		if (cl->mark_vnum
		&&  (mark = create_obj(get_obj_index(cl->mark_vnum), 0))) {
			obj_to_char(mark, victim);
			equip_char(victim, mark, WEAR_CLANMARK);
		}

		char_puts("Ok.\n", ch);
		altered = TRUE;
		goto cleanup;
	}

	if (!str_prefix(arg2, "level"))
	{
		if (!IS_NPC(victim))
		{
		    char_puts("Not on PC's.\n", ch);
		    goto cleanup;
		}

		if (value < 0 || value > 100)
		{
		    char_puts("Level range is 0 to 100.\n", ch);
		    goto cleanup;
		}
		victim->level = value;
		 altered = TRUE;
		goto cleanup;
	}

	if (!str_prefix(arg2, "gold"))
	{
		victim->gold = value;
		 altered = TRUE;
		goto cleanup;
	}

	if (!str_prefix(arg2, "hp")) {
		int delta = value - victim->perm_hit;

		if (victim->perm_hit + delta < 1
		||  victim->perm_hit + delta > 30000) {
			char_puts("perm_hit will be out of range 1..30,000.\n", ch);
			goto cleanup;
		}

		victim->perm_hit += delta;
		victim->max_hit += delta;
		victim->hit = victim->max_hit;
		update_pos(victim);
		 altered = TRUE;
		goto cleanup;
	}

	if (!str_prefix(arg2, "mana")) {
		int delta = value - victim->perm_mana;

		if (victim->perm_mana + delta < 1
		||  victim->perm_mana + delta > 60000) {
			char_puts("perm_mana will be out of range 1..60,000.\n", ch);
			goto cleanup;
		}

		victim->perm_mana += delta;
		victim->max_mana += delta;
		victim->mana = victim->max_mana;
		 altered = TRUE;
		goto cleanup;
	}

	if (!str_prefix(arg2, "move")) {
		int delta = value - victim->perm_move;

		if (victim->perm_move + delta < 0
		||  victim->perm_move + delta > 60000) {
			char_puts("perm_move will be out of range 1..60,000.\n", ch);
			goto cleanup;
		}

		victim->perm_move += delta;
		victim->max_move += delta;
		victim->move = victim->max_move;
		 altered = TRUE;
		goto cleanup;
	}

	if (!str_prefix(arg2, "practice")) {
		if (IS_NPC(victim)) {
			char_puts("Not on NPC's.\n", ch);
			goto cleanup;
		}

		if (value < 0 || value > 250) {
			char_puts("Practice range is 0 to 250 sessions.\n", ch);
			goto cleanup;
		}
		PC(victim)->practice = value;
		 altered = TRUE;
		goto cleanup;
	}

	if (!str_prefix(arg2, "train")) {
		if (IS_NPC(victim)) {
			char_puts("Not on NPC's.\n", ch);
			goto cleanup;
		}

		if (value < 0 || value > 50) {
			char_puts("Training session range is 0 to 50 sessions.\n", ch);
			goto cleanup;
		}
		PC(victim)->train = value;
		 altered = TRUE;
		goto cleanup;
	}

	if (!str_prefix(arg2, "align")) {
		if (value < -1000 || value > 1000) {
			char_puts("Alignment range is -1000 to 1000.\n", ch);
			goto cleanup;
		}
		victim->alignment = value;
		char_puts("Remember to check their hometown.\n", ch);
		 altered = TRUE;
		goto cleanup;
	}

	if (!str_prefix(arg2, "ethos")) {
		int ethos;

		if (IS_NPC(victim)) {
			char_puts("Mobiles don't have an ethos.\n", ch);
			goto cleanup;
		}

		ethos = flag_value(ethos_table, arg3);
		if (ethos < 0) {
			char_puts("%s: Unknown ethos.\n", ch);
			char_puts("Valid ethos types are:\n", ch);
			show_flags(ch, ethos_table);
			goto cleanup;
		}

		victim->ethos = ethos;
		 altered = TRUE;
		goto cleanup;
	}

	if (!str_prefix(arg2, "hometown")) {
		if (IS_NPC(victim)) {
			char_puts("Mobiles don't have hometowns.\n", ch);
			goto cleanup;
		}

		if ((value = htn_lookup(arg3)) == -1) {
			char_puts("No such hometown", ch);
			goto cleanup;
		}
	    
		PC(victim)->hometown = value;
		 altered = TRUE;
		goto cleanup;
	}

	if (!str_prefix(arg2, "thirst"))
	{
		if (IS_NPC(victim))
		{
		    char_puts("Not on NPC's.\n", ch);
		    goto cleanup;
		}

		if (value < -1 || value > 100)
		{
		    char_puts("Thirst range is -1 to 100.\n", ch);
		    goto cleanup;
		}

		PC(victim)->condition[COND_THIRST] = value;
		 altered = TRUE;
		goto cleanup;
	}

	if (!str_prefix(arg2, "drunk"))
	{
		if (IS_NPC(victim))
		{
		    char_puts("Not on NPC's.\n", ch);
		    goto cleanup;
		}

		if (value < -1 || value > 100)
		{
		    char_puts("Drunk range is -1 to 100.\n", ch);
		    goto cleanup;
		}

		PC(victim)->condition[COND_DRUNK] = value;
		 altered = TRUE;
		goto cleanup;
	}

	if (!str_prefix(arg2, "full"))
	{
		if (IS_NPC(victim))
		{
		    char_puts("Not on NPC's.\n", ch);
		    goto cleanup;
		}

		if (value < -1 || value > 100)
		{
		    char_puts("Full range is -1 to 100.\n", ch);
		    goto cleanup;
		}

		PC(victim)->condition[COND_FULL] = value;
		 altered = TRUE;
		goto cleanup;
	}

	if (!str_prefix(arg2, "hunger"))
	{
		if (IS_NPC(victim))
		{
		    char_puts("Not on NPC's.\n", ch);
		    goto cleanup;
		}

		if (value < -1 || value > 100)
		{
		    char_puts("Hunger range is -1 to 100.\n", ch);
		    goto cleanup;
		}

		PC(victim)->condition[COND_HUNGER] = value;
		 altered = TRUE;
		goto cleanup;
	}

	if (!str_prefix(arg2, "bloodlust"))
	{
		if (IS_NPC(victim))
		{
		    char_puts("Not on NPC's.\n", ch);
		    goto cleanup;
		}

		if (value < -1 || value > 100)
		{
		    char_puts("Full range is -1 to 100.\n", ch);
		    goto cleanup;
		}

		PC(victim)->condition[COND_BLOODLUST] = value;
		 altered = TRUE;
		goto cleanup;
	}

	if (!str_prefix(arg2, "desire")) {
		if (IS_NPC(victim)) {
			char_puts("Not on NPC's.\n", ch);
			goto cleanup;
		}

		if (value < -1 || value > 100)
		{
		    char_puts("Full range is -1 to 100.\n", ch);
		    goto cleanup;
		}

		PC(victim)->condition[COND_DESIRE] = value;
		 altered = TRUE;
		goto cleanup;
	}

	if (!str_prefix(arg2, "race")) {
		race_t *r;

		if (IS_NPC(victim)) {
			char_puts("Not on NPC's.\n", ch);
			goto cleanup;
		}

		if ((r = race_search(arg3)) == NULL) {
			char_puts("That is not a valid race.\n",ch);
			goto cleanup;
		}

		if (!IS_NPC(victim) && !r->race_pcdata) {
			char_puts("That is not a valid player race.\n",ch);
			goto cleanup;
		}

		free_string(victim->race);
		victim->race = str_qdup(r->name);
		if (!IS_NPC(victim)) {
			free_string(PC(victim)->race);
			PC(victim)->race = str_qdup(r->name);
		}
			
		race_resetstats(victim);
		spec_update(victim);
		PC(victim)->exp = exp_for_level(victim, victim->level);
		 altered = TRUE;
		goto cleanup;
	}

	if (!str_prefix(arg2, "noghost")) {
		if (IS_NPC(victim)) {
			char_puts("Not on NPC's.\n", ch);
			goto cleanup;
		}
		REMOVE_BIT(PC(victim)->plr_flags, PLR_GHOST);
		char_puts("Ok.\n", ch);
		 altered = TRUE;
		goto cleanup;
	}

	/*
	 * Generate usage message.
	 */
	do_set(ch, str_empty);

cleanup:
	if (altered)
		char_save(victim, loaded ? SAVE_F_PSCAN : 0);
	if (loaded) 
		char_nuke(victim);
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
		buf_printf(output, "%-20s %-8u       ",
			   area->name, area->count);
	}
	buf_add(output, "\n\n");
	page_to_char(buf_string(output), ch);
	buf_free(output);
}

void do_title(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	bool loaded = TRUE;

	if (IS_NPC(ch))
		return;

	if (!IS_IMMORTAL(ch)
	&&  (clan_lookup(ch->clan) == NULL ||
	     PC(ch)->clan_status != CLAN_LEADER)) {
		char_puts("Huh?\n", ch);
		return;
	}
	    
	argument = one_argument(argument, arg, sizeof(arg));
	if (argument[0] == '\0') {
		do_help(ch, "'TITLE'");
		return;
	}

	if ((victim = get_char_world(ch, arg)) == NULL) {
		if ((victim = char_load(arg, LOAD_F_NOCREATE)) == NULL) {
			char_puts("They aren't here.\n", ch);
			return;
		}
		loaded = TRUE;
	} else if (IS_NPC(victim)) {
		char_puts("Not on NPC's.\n", ch);
		return;
	}

	if (IS_IMMORTAL(victim)) {
		if (!IS_TRUSTED(ch, trust_level(victim))) {
			char_puts("You failed.\n", ch);
			goto cleanup;
		}
	} else if (!IS_IMMORTAL(ch) && !IS_CLAN(ch->clan, victim->clan)) {
		char_puts("You failed.\n", ch);
		goto cleanup;
	}

	set_title(victim, argument);
	act("$n grants $N a new title!", ch, NULL, victim, TO_NOTVICT);
	act("$n grants you a new title!", ch, NULL, victim, TO_VICT);
	act("You grant $N a new title!", ch, NULL, victim, TO_CHAR);

cleanup:
	if (loaded) {
		char_save(victim, SAVE_F_PSCAN);
		char_nuke(victim);
	}
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
	bool loaded = FALSE;
	OBJ_DATA *obj;
	clan_t *clan;
		
	argument = first_arg(argument, old_name, sizeof(old_name), FALSE); 
		   first_arg(argument, new_name, sizeof(new_name), FALSE);
		
	if (IS_NULLSTR(old_name) || IS_NULLSTR(new_name)) {
		do_help(ch, "'WIZ ITITLE'");
		return;
	}
		
	if ((victim = get_char_world(ch, old_name)) == NULL) {
		if ((victim = char_load(old_name, LOAD_F_NOCREATE)) == NULL) {
			char_puts("They aren't here.\n",ch);
			return;
		}
		loaded = TRUE;
	} else if (IS_NPC(victim)) {   
		char_puts("You cannot use Rename on NPCs.\n",ch);
		return;
	}

	if (!IS_TRUSTED(ch, trust_level(victim))) {
		char_puts("You failed.\n",ch);
		goto cleanup;
	}
		
	if (!str_cscmp(new_name, victim->name)) {
		char_puts("Old and new names are the same.\n", ch);
		goto cleanup;
	}

	if (!pc_name_ok(new_name)) {
		char_puts("The new name is illegal.\n",ch);
		goto cleanup;
	}

/* delete old pfile */
	if (str_cmp(new_name, victim->name)) {
		DESCRIPTOR_DATA *d;

		for (d = descriptor_list; d; d = d->next)
			if (d->character
			&&  !str_cmp(d->character->name, new_name)) {
				char_puts ("A player with the name you specified already exists!\n",ch);
				goto cleanup;
			}

		/* check pfile */
		file_name = capitalize(new_name);
		if (dfexist(PLAYER_PATH, file_name)) {
			char_puts("A player with that name already exists!\n",
				  ch);
			goto cleanup;		
		}

		/* check .gz pfile */
		snprintf(strsave, sizeof(strsave), "%s.gz", file_name);
		if (dfexist(PLAYER_PATH, strsave)) {
			char_puts ("A player with that name already exists in a compressed file!\n",ch);
			goto cleanup;		
		}

		dunlink(PLAYER_PATH, capitalize(victim->name)); 
	}

	/* change poofin/poofout */
	PC(victim)->bamfin = string_replace(PC(victim)->bamfin, victim->name,
					    new_name, SR_F_ALL);
	PC(victim)->bamfout = string_replace(PC(victim)->bamfin, victim->name,
					     new_name, SR_F_ALL);

	if ((clan = clan_lookup(victim->clan))) {
		bool touched = FALSE;

		if (name_delete(&clan->member_list, victim->name, NULL, NULL)) {
			touched = TRUE;
			name_add(&clan->member_list, new_name, NULL, NULL);
		}

		if (name_delete(&clan->leader_list, victim->name, NULL, NULL)) {
			touched = TRUE;
			name_add(&clan->leader_list, new_name, NULL, NULL);
		}

		if (name_delete(&clan->second_list, victim->name, NULL, NULL)) {
			touched = TRUE;
			name_add(&clan->second_list, new_name, NULL, NULL);
		}

		if (touched)
			clan_save(clan);
	}

	/* change object owners */
	for (obj = object_list; obj; obj = obj->next) {
		if (!str_cmp(mlstr_mval(&obj->owner), victim->name)) {
			mlstr_destroy(&obj->owner);
			mlstr_init2(&obj->owner, new_name);
		}
	}

	free_string(victim->name);
	victim->name = str_dup(new_name);
	mlstr_destroy(&victim->short_descr);
	mlstr_init2(&victim->short_descr, new_name);
		
	char_puts("Character renamed.\n", ch);
	act_puts("$n has renamed you to $N!",
		 ch, NULL, victim, TO_VICT, POS_DEAD);

cleanup:
	/*
	 * NOTE: Players who are level 1 do NOT get saved under a new name 
	 */

	if (loaded) {
		char_nuke(victim);
		char_save(victim, SAVE_F_PSCAN);
	} else
		char_save(victim, 0);
} 

void do_wizpass(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	const char *pwdnew;
	bool loaded = FALSE;

	if (argument[0] == '\0') {
		do_help(ch, "'WIZ WIZPASS'");
		return;
	}

	argument = one_argument(argument, arg, sizeof(arg));
		   one_argument(argument, arg2, sizeof(arg2));

	if ((victim = get_char_world(ch, arg)) == NULL) {
		if ((victim = char_load(arg, LOAD_F_NOCREATE)) == NULL) {
			char_puts("They aren't here.\n", ch);
			return;
		}
		loaded = TRUE;
	} else if (IS_NPC(victim)) {
		char_puts("Not on NPC's.\n", ch);
		return;
	}
	 
	if (!IS_TRUSTED(ch, trust_level(victim))) {
		char_puts("You failed.\n", ch);
		goto cleanup;
	}

	if (arg2[0] == '\0')
		pwdnew = str_empty;
	else {
		if (strlen(arg2) < 5) {
			char_puts("New password must be at least five characters long.\n", ch);
			goto cleanup;
		}

		/*
		 * No tilde allowed because of player file format.
		 */
		pwdnew = crypt(arg2, victim->name);
		if (strchr(pwdnew, '~') != NULL) {
			char_puts("New password not acceptable, try again.\n", ch);
			goto cleanup;
		}
	}

	free_string(PC(victim)->pwd);
	PC(victim)->pwd = str_dup(pwdnew);
	char_printf(ch, "%s: password changed to '%s'.\n", victim->name, arg2);

cleanup:
	if (loaded) {
		char_save(victim, SAVE_F_PSCAN);
		char_nuke(victim);
	} else {
		act_puts("$N sets your password to '$t'.",
			 ch, arg2, victim, TO_CHAR, POS_DEAD);
		char_save(victim, 0);
	}
}
   
void do_noaffect(CHAR_DATA *ch, const char *argument)
{
	AFFECT_DATA *paf,*paf_next;
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	bool loaded = FALSE;

	if (!IS_IMMORTAL(ch))
		return;

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		do_help(ch, "'WIZ NOAFFECT'");
		return;
	}

	if ((victim = get_char_world(ch, arg)) == NULL) {
		if ((victim = char_load(arg, LOAD_F_NOCREATE)) == NULL) {
			char_puts("They aren't here.\n", ch);
			return;
		}
		loaded = TRUE;
	}
	 
	for (paf = victim->affected; paf != NULL; paf = paf_next) {
		paf_next = paf->next;
		if (paf->duration >= 0) {
			skill_t *sk;

			if ((sk = skill_lookup(paf->type))
			&&  !mlstr_null(&sk->msg_off))
				act_mlputs(&sk->msg_off, victim, NULL, NULL, 
					   TO_CHAR, POS_RESTING);
		  
			affect_remove(victim, paf);
		}
	}

	if (loaded) {
		char_save(victim, SAVE_F_PSCAN);
		char_nuke(victim);
	}
}

void do_affrooms(CHAR_DATA *ch, const char *argument)
{
	ROOM_INDEX_DATA *room;
	CHAR_DATA	*rch;
	OBJ_DATA	*obj;
	BUFFER		*buf;
	
	AFFECT_DATA *af;
	int count = 0;

	buf = buf_new(-1);

	if (!top_affected_room) 
		buf_add(buf, "No affected rooms.\n");
	else
		buf_add(buf, "Affected rooms:\n");

	for (room = top_affected_room; room ; room = room->aff_next)
		for (af = room->affected; af; af = af->next) {
			count++;
			buf_printf(buf,
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

	count = 0;
	if (!top_affected_char) 
		buf_add(buf, "No characters under owned affects.\n");
	else
		buf_add(buf, "Characters under owned affects:\n");

	for (rch = top_affected_char; rch ; rch = rch->aff_next)
		for (af = rch->affected; af; af = af->next) {
			if (!af->owner)
				continue;
			count++;
			buf_printf(buf,
				    "%d) [%s] "
				    "spell '{c%s{x', owner: %s, level {c%d{x "
				    "for {c%d{x hours.\n",
				    count,
				    mlstr_mval(&rch->short_descr),
				    af->type,
				    mlstr_mval(&af->owner->short_descr),
				    af->level,
				    af->duration);
		}

	count = 0;
	if (!top_affected_obj) 
		buf_add(buf, "No objects under owned affects.\n");
	else
		buf_add(buf, "Objects under owned affects:\n");

	for (obj = top_affected_obj; obj ; obj = obj->aff_next)
		for (af = obj->affected; af; af = af->next) {
			count++;
			buf_printf(buf,
				    "%d) [Vnum: %5d] "
				    "spell '{c%s{x', owner: %s, level {c%d{x "
				    "for {c%d{x hours.\n",
				    count,
				    obj->pObjIndex->vnum,
				    af->type,
				    mlstr_mval(&af->owner->short_descr),
				    af->level,
				    af->duration);
		}
	page_to_char(buf_string(buf), ch);
	buf_free(buf);
}

void do_grant(CHAR_DATA *ch, const char *argument)
{
	cmd_t *cmd;
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	bool loaded = FALSE;
	bool altered = FALSE;

	argument = one_argument(argument, arg1, sizeof(arg1));
	argument = one_argument(argument, arg2, sizeof(arg2));
	if (arg1[0] == '\0') {
		do_help(ch, "'WIZ GRANT'");
		return;
	}

	if ((victim = get_char_world(ch, arg1)) == NULL) {
		if ((victim = char_load(arg1, LOAD_F_NOCREATE)) == NULL) {
			char_puts("They aren't here.\n", ch);
			return;
		}
		loaded = TRUE;
	} else if (IS_NPC(victim)) {
		char_puts("Not on NPC's.\n", ch);
		return;
	}

	if (arg2[0] == '\0') {
		char_printf(ch, "Granted commands for %s: [%s]\n",
			    victim->name, PC(victim)->granted);
		goto cleanup;
	}

	if (is_number(arg2)) {
		int i;
		int lev = atoi(arg2);

		if (lev < LEVEL_IMMORTAL) {
			char_printf(ch, "grant: granted level must be at least %d\n", LEVEL_IMMORTAL);
			goto cleanup;
		}

		if (lev > trust_level(ch)) {
			char_puts("grant: granted level cannot be higher"
				  " than yours.\n", ch);
			goto cleanup;
		}

		for (i = 0; i < commands.nused; i++) {
			cmd = VARR_GET(&commands, i);

			if (cmd->min_level < LEVEL_IMMORTAL
			||  cmd->min_level > lev)
				continue;

			altered = TRUE;
			name_add(&PC(victim)->granted, cmd->name,
				 ch, "grant");
		}

		goto cleanup;
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
				goto cleanup;
			}
		}

		altered = TRUE;
		name_toggle(&PC(victim)->granted, arg2, ch, "grant");
	}

cleanup:
	if (loaded) {
		if (altered)
			char_save(victim, SAVE_F_PSCAN);
		char_nuke(victim);
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
	INT(af.location)= APPLY_NONE;
	af.owner	= NULL;
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
	char_printf(ch, "Mob idx  : %d (%d bytes, max vnum %d)\n",
		    top_mob_index, top_mob_index * sizeof(MOB_INDEX_DATA),
		    top_vnum_mob); 
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
	char_printf(ch, "Obj idx  : %d (%d bytes, max vnum %d)\n",
		    top_obj_index, top_obj_index * sizeof(OBJ_INDEX_DATA),
		    top_vnum_obj); 
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
	for (fch = char_list; fch != NULL; fch = fch->next) {
		count++;
		if (!IS_NPC(fch))
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

void do_shapeshift(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_STRING_LENGTH];
	form_index_t *form;
	
	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		char_puts("Shapeshift into what?.\n", ch);
		return;
	}

	if ((form = form_search(arg)) == NULL) {
		char_puts("No such form.\n", ch);
		return;
	}

	shapeshift(ch, arg);
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

extern int max_rnd_cnt;
extern int rnd_cnt;

void
do_maxrnd(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];

	one_argument(argument, arg, sizeof(arg));
	if (is_number(arg)) {
		max_rnd_cnt = atoi(arg);
		rnd_cnt = 0;
	} else if (arg[0] != '\0') {
		char_puts("Syntax: maxrnd <max_rnd_cnt>\n", ch);
		return;
	}

	char_printf(ch, "Current max_rnd_cnt = %d\n", max_rnd_cnt);
}
