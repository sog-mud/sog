/*
 * $Id: act_wiz.c,v 1.294 2001-09-02 16:21:51 fjoe Exp $
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
#include <time.h>
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
#	include <compat/fnmatch.h>
#endif

#include <merc.h>
#include <cmd.h>
#include <rwfile.h>
#include <module.h>

#include <sog.h>
#include <quest.h>

DECLARE_DO_FUN(do_objlist);
DECLARE_DO_FUN(do_limited);
DECLARE_DO_FUN(do_wiznet);
DECLARE_DO_FUN(do_nonote);
DECLARE_DO_FUN(do_nochannels);
DECLARE_DO_FUN(do_smote);
DECLARE_DO_FUN(do_bamfin);
DECLARE_DO_FUN(do_bamfout);
DECLARE_DO_FUN(do_disconnect);
DECLARE_DO_FUN(do_echo);
DECLARE_DO_FUN(do_recho);
DECLARE_DO_FUN(do_zecho);
DECLARE_DO_FUN(do_pecho);
DECLARE_DO_FUN(do_transfer);
DECLARE_DO_FUN(do_at);
DECLARE_DO_FUN(do_goto);
DECLARE_DO_FUN(do_stat);
DECLARE_DO_FUN(do_vnum);
DECLARE_DO_FUN(do_owhere);
DECLARE_DO_FUN(do_mwhere);
DECLARE_DO_FUN(do_protect);
DECLARE_DO_FUN(do_snoop);
DECLARE_DO_FUN(do_switch);
DECLARE_DO_FUN(do_return);
DECLARE_DO_FUN(do_clone);
DECLARE_DO_FUN(do_load);
DECLARE_DO_FUN(do_purge);
DECLARE_DO_FUN(do_restore);
DECLARE_DO_FUN(do_freeze);
DECLARE_DO_FUN(do_log);
DECLARE_DO_FUN(do_noemote);
DECLARE_DO_FUN(do_notell);
DECLARE_DO_FUN(do_peace);
DECLARE_DO_FUN(do_wizlock);
DECLARE_DO_FUN(do_newlock);
DECLARE_DO_FUN(do_set);
DECLARE_DO_FUN(do_string);
DECLARE_DO_FUN(do_sockets);
DECLARE_DO_FUN(do_force);
DECLARE_DO_FUN(do_invis);
DECLARE_DO_FUN(do_incognito);
DECLARE_DO_FUN(do_holylight);
DECLARE_DO_FUN(do_prefix);
DECLARE_DO_FUN(do_advance);
DECLARE_DO_FUN(do_smite);
DECLARE_DO_FUN(do_popularity);
DECLARE_DO_FUN(do_title);
DECLARE_DO_FUN(do_rename);
DECLARE_DO_FUN(do_wizpass);
DECLARE_DO_FUN(do_noaffect);
DECLARE_DO_FUN(do_affrooms);
DECLARE_DO_FUN(do_grant);
DECLARE_DO_FUN(do_qtarget);
DECLARE_DO_FUN(do_slay);
DECLARE_DO_FUN(do_ban);
DECLARE_DO_FUN(do_memory);
DECLARE_DO_FUN(do_dump);
DECLARE_DO_FUN(do_mob);
DECLARE_DO_FUN(do_shapeshift);
DECLARE_DO_FUN(do_mpstat);
DECLARE_DO_FUN(do_maxrnd);
DECLARE_DO_FUN(do_modules);
DECLARE_DO_FUN(do_shutdown);
DECLARE_DO_FUN(do_reboot);

/* local command procedures */
DECLARE_DO_FUN(do_rstat);
DECLARE_DO_FUN(do_mstat);
DECLARE_DO_FUN(do_dstat);
DECLARE_DO_FUN(do_ostat);
#if 0
XXX
DECLARE_DO_FUN(do_mpstat);
#endif
DECLARE_DO_FUN(do_rset);
DECLARE_DO_FUN(do_mset);
DECLARE_DO_FUN(do_oset);
DECLARE_DO_FUN(do_sset);
DECLARE_DO_FUN(do_mfind);
DECLARE_DO_FUN(do_ofind);
DECLARE_DO_FUN(do_mload);
DECLARE_DO_FUN(do_oload);

/* command procedures needed */
DECLARE_DO_FUN(do_look);
DECLARE_DO_FUN(do_stand);
DECLARE_DO_FUN(do_help);
DECLARE_DO_FUN(do_replay);

static void advance(CHAR_DATA *victim, int level);

DO_FUN(do_objlist, ch, argument)
{
	FILE *fp;
	OBJ_DATA *obj;
	BUFFER *buf;

	if ((fp = dfopen(TMP_PATH, "objlist.txt", "w+")) == NULL) {  // notrans
		act_char("File error.", ch);			     // notrans
		return;
	}

	buf = buf_new(0);
	for(obj = object_list; obj != NULL; obj = obj->next) {
		if (obj->pObjIndex->affected == NULL)
			continue;

		buf_clear(buf);
		buf_printf(buf, BUF_END, "\n#Obj: %s (Vnum : %d) \n", // notrans
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

DO_FUN(do_limited, ch, argument)
{
	OBJ_DATA *obj;
	int	lCount = 0;
	int	ingameCount;
	int	nMatch;
	BUFFER *buf;

	if (argument[0] != '\0')  {
		OBJ_INDEX_DATA *obj_index;

		obj_index = get_obj_index(atoi(argument));
		if (obj_index == NULL)  {
			act_char("Not found.", ch);
			return;
		}

		if (obj_index->limit == -1)  {
			act_char("That's not a limited item.", ch);
			return;
		}

		nMatch = 0;
		buf = buf_new(0);
		buf_printf(buf, BUF_END,
			   "%-35s [%5d]  Limit: %3d  Current: %3d\n", // notrans
			   mlstr_mval(&obj_index->short_descr),
			   obj_index->vnum,
		           obj_index->limit,
			   obj_index->count);

		ingameCount = 0;
		for (obj = object_list; obj != NULL; obj = obj->next) {
			if (obj->pObjIndex->vnum != obj_index->vnum)
				continue;

			ingameCount++;
			if (obj->carried_by != NULL) {
				buf_printf(buf, BUF_END,
					   "Carried by %-30s\n",  // notrans
					   obj->carried_by->name);
			} else if (obj->in_room != NULL) {
				buf_printf(buf, BUF_END, "At %-20s [%d]\n", // notrans
					   mlstr_cval(&obj->in_room->name, ch),
					   obj->in_room->vnum);
			} else if (obj->in_obj != NULL) {
				buf_printf(buf, BUF_END, "In %-20s [%d] \n", // notrans
					   mlstr_mval(&obj->in_obj->short_descr),
					   obj->in_obj->pObjIndex->vnum);
			}
		}

		buf_printf(buf, BUF_END,
			   "  %d found in game. %d should be in pFiles.\n", // notrans
			   ingameCount, obj_index->count-ingameCount);
		return;
	} else {
		OBJ_INDEX_DATA *obj_index;
		int i;
		nMatch = 0;

		buf = buf_new(0);
		for (i = 0; i < MAX_KEY_HASH; i++) {
			for (obj_index = obj_index_hash[i]; obj_index; obj_index = obj_index->next) {
				nMatch++;
				if (obj_index->limit == -1)
					continue;

				lCount++;
				buf_printf(buf, BUF_END,
					   "%-37s [%5d]  Limit: %3d  Current: %3d\n", // notrans
					   mlstr_mval(&obj_index->short_descr), 
					   obj_index->vnum,
					   obj_index->limit,
					   obj_index->count);
			}
		}

		buf_printf(buf, BUF_END,
			   "\n%d of %d objects are limited.\n", lCount, nMatch);// notrans
	}

	page_to_char(buf_string(buf), ch);
	buf_free(buf);
}

static flag_t
wiznet_lookup(const char *name)
{
	int flag;

	for (flag = 0; wiznet_table[flag].name != NULL; flag++) {
		if (!str_prefix(name,wiznet_table[flag].name))
			return flag;
	}

	return -1;
}

DO_FUN(do_wiznet, ch, argument)
{
	int flag;
	CHAR_DATA *vch;

	if (ch->desc == NULL)
		return;

	vch = GET_ORIGINAL(ch);

	if (argument[0] == '\0') {
		/* show wiznet status */
		BUFFER *output;

		output = buf_new(GET_LANG(ch));
		buf_printf(output, BUF_END, "Wiznet status: %s\n",
			   IS_SET(PC(vch)->wiznet, WIZ_ON) ? "ON" : "OFF"); // notrans

		buf_append(output, "\nchannel    | status");
		buf_append(output, "\n-----------|-------\n");	// notrans
		for (flag = 0; wiznet_table[flag].name != NULL; flag++)
			buf_printf(output, BUF_END, "%-11s|  %s\n", // notrans
				   wiznet_table[flag].name,
				   wiznet_table[flag].level > vch->level ?
				   "N/A" :		// notrans
				   IS_SET(PC(vch)->wiznet,
					  wiznet_table[flag].flag) ?
				   "ON" : "OFF");	// notrans
		page_to_char(buf_string(output), ch);
		buf_free(output);
		return;
	}

	if (!str_prefix(argument, "on")) {
		act_char("Welcome to Wiznet!", ch);		// notrans
		SET_BIT(PC(vch)->wiznet, WIZ_ON);
		return;
	}

	if (!str_prefix(argument, "off")) {
		act_char("Signing off of Wiznet.", ch);		// notrans
		REMOVE_BIT(PC(vch)->wiznet, WIZ_ON);
		return;
	}

	flag = wiznet_lookup(argument);
	if (flag == -1 || vch->level < wiznet_table[flag].level) {
		act_char("No such option.", ch);
		return;
	}

	TOGGLE_BIT(PC(vch)->wiznet, wiznet_table[flag].flag);
	if (!IS_SET(PC(vch)->wiznet, wiznet_table[flag].flag)) {
		act_puts("You will no longer see $t on wiznet.", // notrans
			 ch, wiznet_table[flag].name, NULL,
			 TO_CHAR | ACT_NOTRANS, POS_DEAD);
	} else {
		act_puts("You will now see $t on wiznet.",	// notrans
			 ch, wiznet_table[flag].name, NULL,
			 TO_CHAR | ACT_NOTRANS, POS_DEAD);
	}
}

DO_FUN(do_nonote, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	bool loaded = FALSE;
	bool altered = FALSE;

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		do_help(ch, "'WIZ NONOTE'");
		return;
	}

	if ((victim = get_char_world(ch, arg)) == NULL) {
		if ((victim = char_load(arg, LOAD_F_NOCREATE)) == NULL) {
			act_char("They aren't here.", ch);
			return;
		}
		loaded = TRUE;
	}

	if (!IS_TRUSTED(ch, trust_level(victim))) {
		act_char("You failed.", ch);
		goto cleanup;
	}

	TOGGLE_BIT(victim->comm, COMM_NONOTE);
	altered = TRUE;

	if (!IS_SET(victim->comm, COMM_NONOTE)) {
		act_char("You may write notes again.", victim);
		act_char("NONOTE removed.", ch);
		wiznet("$N grants $i right to write notes.",
			ch, victim, WIZ_PENALTIES, WIZ_SECURE, 0);
	} else {
		act_char("Your notes will be sent to Abyss now.", victim);
		act_char("NONOTE set.", ch);
		wiznet("$N revokes $i's right to write notes.",
			ch, victim, WIZ_PENALTIES, WIZ_SECURE, 0);
	}

cleanup:
	if (altered)
		char_save(victim, loaded ? SAVE_F_PSCAN : 0);
	if (loaded) 
		char_nuke(victim);
}

/* RT nochannels command, for those spammers */
DO_FUN(do_nochannels, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	bool loaded = FALSE;
	bool altered = FALSE;

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		do_help(ch, "'WIZ NOCHANNEL'");
		return;
	}

	if ((victim = get_char_world(ch, arg)) == NULL) {
		if ((victim = char_load(arg, LOAD_F_NOCREATE)) == NULL) {
			act_char("They aren't here.", ch);
			return;
		}
		loaded = TRUE;
	}

	if (!IS_TRUSTED(ch, trust_level(victim))) {
		act_char("You failed.", ch);
		goto cleanup;
	}

	TOGGLE_BIT(victim->chan, CHAN_NOCHANNELS);
	altered = TRUE;

	if (!IS_SET(victim->chan, CHAN_NOCHANNELS)) {
		act_char("The gods have restored your channel priviliges.", victim);
		act_char("NOCHANNELS removed.", ch);
		wiznet("$N restores channels to $i.",
			ch, victim, WIZ_PENALTIES, WIZ_SECURE, 0);
	} else {
		act_char("The gods have revoked your channel priviliges.", victim);
		act_char("NOCHANNELS set.", ch);
		wiznet("$N revokes $i's channels.",
			ch, victim, WIZ_PENALTIES, WIZ_SECURE, 0);
	}

cleanup:
	if (altered)
		char_save(victim, loaded ? SAVE_F_PSCAN : 0);
	if (loaded)
		char_nuke(victim);
}

DO_FUN(do_smote, ch, argument)
{
	CHAR_DATA *vch;
	const char *letter, *name;
	char last[MAX_INPUT_LENGTH], temp[MAX_STRING_LENGTH];
	size_t matches = 0;

	if (!IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE)) {
		act_char("You can't show your emotions.", ch);
		return;
	}

	if (argument[0] == '\0') {
		do_help(ch, "'WIZ SMOTE'");
		return;
	}

	if (strstr(argument,ch->name) == NULL) {
		act_char("You must include your name in an smote.", ch);
		return;
	}

	act_puts("$t", ch, argument, NULL,
		 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);

	for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room) {
		if (vch->desc == NULL || vch == ch)
			continue;

		if ((letter = strstr(argument, vch->name)) == NULL) {
			act_puts("$t", ch, argument, NULL,
				 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
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
		act_puts("$t", ch, temp, NULL,
			 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
	}
}	

DO_FUN(do_bamfin, ch, argument)
{
	CHAR_DATA *vch = GET_ORIGINAL(ch);

	if (IS_NPC(vch))
		return;

	if (argument[0] == '\0') {
		act_puts("Your poofin is '$t'", ch, PC(vch)->bamfin, NULL,
			 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
		return;
	}

	if (strstr(argument, vch->name) == NULL) {
		act_char("You must include your name.", ch);
		return;
	}
		     
	free_string(PC(vch)->bamfin);
	PC(vch)->bamfin = str_dup(argument);

	act_puts("Your poofin is now '$t'", ch, PC(vch)->bamfin, NULL,
		 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
}

DO_FUN(do_bamfout, ch, argument)
{
	CHAR_DATA *vch = GET_ORIGINAL(ch);

	if (IS_NPC(vch))
		return;

	if (argument[0] == '\0') {
		act_puts("Your poofout is '$t'", ch, PC(ch)->bamfout, NULL,
			 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
		return;
	}

	if (strstr(argument, vch->name) == NULL) {
		act_char("You must include your name.", ch);
		return;
	}

	free_string(PC(vch)->bamfout);
	PC(vch)->bamfout = str_dup(argument);

	act_puts("Your poofout is now '$t'", ch, PC(vch)->bamfout, NULL,
		 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
}

DO_FUN(do_disconnect, ch, argument)
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
				act_char("Ok.", ch);
				return;
			}
		}
	}

	if ((victim = get_char_world(ch, arg)) == NULL) {
		act_char("They aren't here.", ch);
		return;
	}

	if (victim->desc == NULL) {
		act("$N doesn't have a descriptor.", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (!IS_TRUSTED(ch, trust_level(victim))) {
		act_char("You failed.", ch);
		return;
	}

	for (d = descriptor_list; d != NULL; d = d->next) {
		if (d == victim->desc) {
			close_descriptor(d, SAVE_F_NORMAL);
			act_char("Ok.", ch);
			return;
		}
	}

	log(LOG_BUG, "do_disconnect: desc not found");
	act_char("Descriptor not found!", ch);
}

DO_FUN(do_echo, ch, argument)
{
	DESCRIPTOR_DATA *d;
	
	if (argument[0] == '\0') {
		do_help(ch, "'WIZ GECHO'");
		return;
	}
	
	for (d = descriptor_list; d; d = d->next) {
		if (d->connected == CON_PLAYING) {
			if (IS_TRUSTED(d->character, trust_level(ch))) {
				act("{W$N:global>{x ",
					d->character, NULL, ch,
					TO_CHAR | ACT_NOLF);
			}
			act_puts("$t", d->character, argument, NULL,
				 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
		}
	}
}

DO_FUN(do_recho, ch, argument)
{
	DESCRIPTOR_DATA *d;
	
	if (argument[0] == '\0') {
		do_help(ch, "'WIZ ECHO'");
		return;
	}

	for (d = descriptor_list; d; d = d->next) {
		if (d->connected == CON_PLAYING
		&&   d->character->in_room == ch->in_room) {
			if (IS_TRUSTED(d->character, trust_level(ch))) {
				act("{W$N:local>{x ",
					d->character, NULL, ch,
					TO_CHAR | ACT_NOLF);
			}
			act_puts("$t", d->character, argument, NULL,
				 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
		}
	}
}

DO_FUN(do_zecho, ch, argument)
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
			act_puts("$t", d->character, argument, NULL,
				 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
		}
}

DO_FUN(do_pecho, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	argument = one_argument(argument, arg, sizeof(arg));
	
	if (argument[0] == '\0' || arg[0] == '\0') {
		do_help(ch, "'WIZ PECHO'");
		return;
	}
	 
	if  ((victim = get_char_world(ch, arg)) == NULL) {
		act_char("They aren't here.", ch);
		return;
	}

	if (IS_TRUSTED(victim, trust_level(ch)))
		act("{W$N:personal>{x ", victim, NULL, ch, TO_CHAR | ACT_NOLF);

	act_puts("$t", victim, argument, NULL,
		 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
	act_puts("personal> $t", ch, argument, NULL,
		 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
}

DO_FUN(do_transfer, ch, argument)
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
		for (d = descriptor_list; d != NULL; d = d->next) {
			if (d->connected == CON_PLAYING
			&&  d->character != ch
			&&  d->character->in_room != NULL
			&&  can_see(ch, d->character)) {
				char buf[MAX_INPUT_LENGTH];

				snprintf(buf, sizeof(buf),
					 "%s %s",		// notrans
					 d->character->name, arg2);
				dofun("transfer", ch, buf);
			}
		}
		return;
	}

	/*
	 * Thanks to Grodyn for the optional location parameter.
	 */
	if (arg2[0]) {
		if ((location = find_location(ch, arg2)) == NULL) {
		    act_char("No such location.", ch);
		    return;
		}

		if (room_is_private(location) && ch->level < MAX_LEVEL) {
		    act_char("That room is private right now.", ch);
		    return;
		}
	} else
		location = ch->in_room;

	if ((victim = get_char_world(ch, arg1)) == NULL) {
		act_char("They aren't here.", ch);
		return;
	}

	if (victim->in_room == NULL) {
		act_char("They are in limbo.", ch);
		return;
	}

	if (!IS_TRUSTED(ch, trust_level(victim))) {
		act_char("You failed.", ch);
		return;
	}

	if (victim->fighting != NULL)
		stop_fighting(victim, TRUE);

	act("$n disappears in a mushroom cloud.", victim, NULL, NULL, TO_ROOM);
	if (ch != victim)
		act("$n has transferred you.", ch, NULL, victim, TO_VICT);
	act_char("Ok.", ch);

	char_from_room(victim);
	act("$N arrives from a puff of smoke.",
	    location->people, NULL, victim, TO_ALL);

	char_to_room(victim, location);
	if (IS_EXTRACTED(victim))
		return;

	do_look(victim, "auto");
}

DO_FUN(do_at, ch, argument)
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
		act_char("No such location.", ch);
		return;
	}

	original = ch->in_room;
	on = ch->on;
	char_from_room(ch);

	char_to_room(ch, location);
	if (IS_EXTRACTED(ch))
		return;

	interpret(ch, argument, FALSE);

	/* handle 'at xxx quit' */
	if (IS_EXTRACTED(ch))
		return;

	char_from_room(ch);
	char_to_room(ch, original);
	if (IS_EXTRACTED(ch))
		return;
	ch->on = on;
}

DO_FUN(do_goto, ch, argument)
{
	ROOM_INDEX_DATA *location;
	CHAR_DATA *rch;
	CHAR_DATA *pet = NULL;

	if (argument[0] == '\0') {
		do_help(ch, "'WIZ GOTO'");
		return;
	}

	if ((location = find_location(ch, argument)) == NULL) {
		act_char("No such location.", ch);
		return;
	}

	if (!IS_TRUSTED(ch, LEVEL_IMMORTAL)) {
		if (ch->fighting) {
			act_char("No way! You are fighting.", ch);
			return;
		}

		if (IS_PUMPED(ch)) {
			act_puts("You are too pumped to pray now.",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
			return;
		}
#if 0
		if (!IS_SET(ch->in_room->room_flags, ROOM_PEACE)) {
			act_char("You must be in a safe place in order to make a transportation.", ch);	// notrans
			return;
		}
#endif
		if (!IS_BUILDER(ch, location->area)
		||  !IS_BUILDER(ch, ch->in_room->area)) {
			act_char("You cannot transfer yourself there.", ch);
			return;
		}
	}

	if (ch->fighting != NULL)
		stop_fighting(ch, TRUE);

	for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room) {
		if (IS_TRUSTED(rch, ch->invis_level)) {
			if (!IS_NPC(ch) && PC(ch)->bamfout[0] != '\0')
				act("$t", ch, PC(ch)->bamfout, rch,
				    TO_VICT | ACT_NOUCASE);
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
				act("$t", ch, PC(ch)->bamfin, rch,
				    TO_VICT | ACT_NOUCASE);
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

DO_FUN(do_stat, ch, argument)
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

#if 0
	XXX
	if (!str_cmp(arg, "mp")) {
		do_mpstat(ch, string);
		return;
	}
#endif

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

	act_char("Nothing by that name found anywhere.", ch);
}

DO_FUN(do_rstat, ch, argument)
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
		act_char("No such location.", ch);
		return;
	}

	if (ch->in_room != location
	&&  room_is_private(location) && !IS_TRUSTED(ch, LEVEL_IMP)) {
		act_char("That room is private right now.", ch);
		return;
	}

	output = buf_new(0);

	if (ch->in_room->affected_by) {
		buf_printf(output, BUF_END, "Affected by %s\n",  // notrans
			   flag_string(raffect_flags, ch->in_room->affected_by));
	}

	if (ch->in_room->room_flags) {
		buf_printf(output, BUF_END, "Room Flags %s\n",	// notrans
			   flag_string(room_flags, ch->in_room->room_flags));
	}

	mlstr_dump(output, "Name: ", &location->name, DUMP_LEVEL(ch)); // notrans
	buf_printf(output, BUF_END, "Area: '%s'\n", location->area->name); // notrans

	buf_printf(output, BUF_END,
		   "Vnum: %d  Sector: %d  Light: %d  Healing: %d  Mana: %d\n", // notrans
		   location->vnum,
		   location->sector_type,
		   location->light,
		   location->heal_rate,
		   location->mana_rate);

	buf_printf(output, BUF_END, "Room flags: [%s].\n",	// notrans
		   flag_string(room_flags, location->room_flags));
	buf_append(output, "Description:\n");			// notrans
	mlstr_dump(output, str_empty, &location->description, DUMP_LEVEL(ch));

	if (location->ed != NULL) {
		ED_DATA *ed;

		buf_append(output, "Exd keywords: [");		// notrans
		for (ed = location->ed; ed; ed = ed->next) {
			buf_append(output, ed->keyword);
			if (ed->next != NULL)
				buf_append(output, " ");
		}
		buf_append(output, "]\n");
	}

	buf_append(output, "Characters:");			// notrans
	for (rch = location->people; rch; rch = rch->next_in_room) {
		if (can_see(ch,rch)) {
			buf_append(output, " ");
			one_argument(rch->name, buf, sizeof(buf));
			buf_append(output, buf);
		}
	}

	buf_append(output, ".\nObjects:   ");			// notrans
	for (obj = location->contents; obj; obj = obj->next_content) {
		buf_append(output, " ");
		one_argument(obj->pObjIndex->name, buf, sizeof(buf));
		buf_append(output, buf);
	}
	buf_append(output, ".\n");

	for (door = 0; door <= 5; door++) {
		EXIT_DATA *pexit;

		if ((pexit = location->exit[door]) != NULL) {
			buf_printf(output, BUF_END, "Door: %d.  To: %d.  Key: %d.  Exit flags: %d.\nKeyword: '%s'.\n",				// notrans
				door,
				pexit->to_room.r == NULL ?
				-1 : pexit->to_room.r->vnum,
				pexit->key,
				pexit->exit_info,
				pexit->keyword);
			mlstr_dump(output, "Description: ",	// notrans
				   &pexit->description, DUMP_LEVEL(ch));
		}
	}
	buf_append(output, "Tracks:\n");			// notrans
	for (rh = location->history;rh != NULL;rh = rh->next)
		buf_printf(output, BUF_END,"%s took door %i.\n", rh->name, rh->went);	// notrans

	send_to_char(buf_string(output), ch);
	buf_free(output);
}

DO_FUN(do_ostat, ch, argument)
{
#if 0
	XXX
	int i;
#endif
	BUFFER *output;
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		act_char("Stat what?", ch);
		return;
	}

	if ((obj = get_obj_world(ch, argument)) == NULL) {
		act_char("Nothing like that in hell, heaven or earth.", ch);
		return;
	}

	output = buf_new(0);
	buf_printf(output, BUF_END,
		   "Name:  [%s]\n", obj->pObjIndex->name);	 // notrans
	if (!IS_NULLSTR(obj->label))
		buf_printf(output, BUF_END,
			   "Label: [%s]\n", obj->label+1);	 // notrans
	if (!mlstr_null(&obj->owner))
		buf_printf(output, BUF_END,
			   "Owner: [%s]\n", mlstr_mval(&obj->owner)); // notrans
	buf_printf(output, BUF_END,
		   "Vnum: %d  Type: %s  Resets: %d\n",		 // notrans
		obj->pObjIndex->vnum,
		flag_string(item_types, obj->item_type),
		obj->pObjIndex->reset_num);

	mlstr_dump(output, "Short description: ",		// notrans
		   &obj->short_descr, DUMP_LEVEL(ch));
	buf_append(output, "Long description:\n");		// notrans
	mlstr_dump(output, str_empty, &obj->description, DUMP_LEVEL(ch));

	buf_printf(output, BUF_END, "Wear bits: [%s]\n",	// notrans
		   flag_string(wear_flags, obj->wear_flags));
	buf_printf(output, BUF_END, "Stat bits: [%s]\n",	// notrans
		   flag_string(stat_flags, obj->stat_flags));
	buf_printf(output, BUF_END, "Obj bits:  [%s]\n",	// notrans
		   flag_string(obj_flags, obj->pObjIndex->obj_flags));
	buf_printf(output, BUF_END, "Number: %d/%d  Weight: %d/%d/%d (10th pounds)\n", // notrans
		1,           get_obj_number(obj),
		obj->weight, get_obj_weight(obj), get_obj_realweight(obj));

	buf_printf(output, BUF_END,
		  "Level: %d  Cost: %d  Condition: %d  Timer: %d Count: %d\n", // notrans
		  obj->level, obj->cost, obj->condition,
		  obj->timer, obj->pObjIndex->count);

	buf_printf(output, BUF_END,
		"In room: %d  In object: %s  Carried by: %s  Wear_loc: %d\n", // notrans
		obj->in_room    == NULL    ?        0 : obj->in_room->vnum,
		obj->in_obj     == NULL    ? "(none)" : mlstr_mval(&obj->in_obj->short_descr),
		obj->carried_by == NULL    ? "(none)" :
		    can_see(ch,obj->carried_by) ? obj->carried_by->name
						: "someone",
		obj->wear_loc);
	buf_printf(output, BUF_END,
		"Next: %s   Next_content: %s\n",		// notrans
		obj->next	== NULL	   ? "(none)" : mlstr_mval(&obj->next->short_descr),
		obj->next_content == NULL  ? "(none)" : mlstr_mval(&obj->next_content->short_descr));
	if (obj->altar) {
		buf_printf(output, BUF_END, "Altar: %s (vnum %d)\n", // notrans
			mlstr_mval(&obj->altar->room->name),
			obj->altar->room->vnum);
	}

	objval_show(output, obj->item_type, obj->value);

	if (obj->ed) {
		ED_DATA *ed;

		buf_append(output, "Exd keywords: [");		// notrans

		for (ed = obj->ed; ed; ed = ed->next) {
			buf_append(output, ed->keyword);
			if (ed->next)
				buf_append(output, " ");
		}

		buf_append(output, "]\n");
	}

	if (obj->pObjIndex->ed) {
		ED_DATA *ed;

		buf_append(output, "Obj index exd keywords: ["); // notrans

		for (ed = obj->pObjIndex->ed; ed; ed = ed->next) {
			buf_append(output, ed->keyword);
			if (ed->next)
				buf_append(output, " ");
		}

		buf_append(output, "]\n");
	}

	if (!IS_OBJ_STAT(obj, ITEM_ENCHANTED))
		format_obj_affects(output, obj->pObjIndex->affected,
				   FOA_F_NODURATION);
	format_obj_affects(output, obj->affected, 0);

#if 0
	XXX
	if (obj->pObjIndex->oprogs) {
		buf_append(output, "Object progs:\n");		// notrans
		for (i = 0; i < OPROG_MAX; i++) {
			if (obj->pObjIndex->oprogs[i] != NULL) {
				buf_printf(output, BUF_END,
				    "%s: %s\n", // notrans
				    optype_table[i],
				    oprog_name_lookup(obj->pObjIndex->oprogs[i]));
			}
		}
	}
#endif

	buf_printf(output, BUF_END, "Damage condition: %d (%s)\n", // notrans
		   obj->condition, get_cond_alias(obj));
#if 0
	XXX
	print_cc_vexpr(&obj->pObjIndex->restrictions, "Restrictions:", output); // notrans
#endif

	send_to_char(buf_string(output), ch);
	buf_free(output);
}

static void *
print_sa_cb(void *p, va_list ap)
{
	saff_t *sa = (saff_t *) p;

	BUFFER *buf = va_arg(ap, BUFFER *);

	buf_printf(buf, BUF_END, "        '%s' by %d",	// notrans
		   sa->sn, sa->mod);
	if (!IS_NULLSTR(sa->type))
		buf_printf(buf, BUF_END, " (skill '%s')", sa->type); // notrans
	if (sa->bit) {
		buf_printf(buf, BUF_END, " with bits '%s'", // notrans
			   flag_string(sk_aff_flags, sa->bit));
	}
	buf_append(buf, "\n");
	return NULL;
}

DO_FUN(do_mstat, ch, argument)
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	CHAR_DATA *pet;
	BUFFER *output;
	bool loaded = FALSE;
	int i, j;
	bool found = FALSE;

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		act_char("Stat whom?", ch);
		return;
	}

	if ((victim = get_char_world(ch, argument)) == NULL) {
		if ((victim = char_load(argument, LOAD_F_NOCREATE)) == NULL) {
			act_char("They aren't here.", ch);
			return;
		}
		loaded = TRUE;
	}

	if (!IS_TRUSTED(ch, trust_level(victim))) {
		act_char("You failed.", ch);
		goto cleanup;
	}

	output = buf_new(0);

	buf_printf(output, BUF_END, "Name: [%s]  ", victim->name); // notrans
	if (IS_NPC(victim)) {
		buf_printf(output, BUF_END,
			   "Vnum: [%d]  Reset Zone: [%s]\n",	  // notrans
			   victim->pMobIndex->vnum,
			   NPC(victim)->zone ?
			   NPC(victim)->zone->name : "?");	  // notrans
	} else
		buf_printf(output, BUF_END,
			   "%sLINE\n", loaded ? "OFF" : "ON");	  // notrans

	buf_printf(output, BUF_END,
		"Race: %s (%s)  Room: [%d]\n",			  // notrans
		victim->race, ORG_RACE(victim),
		victim->in_room == NULL ? 0 : victim->in_room->vnum);

	mlstr_dump(output, "Gender: ", &victim->gender, DL_NONE); // notrans

	if (IS_NPC(victim)) {
		buf_printf(output, BUF_END, "Group: [%d]  Count: [%d]  Killed: [%d]\n", // notrans
			   victim->pMobIndex->group,
			   victim->pMobIndex->count,
			   victim->pMobIndex->killed);
	}

	buf_printf(output, BUF_END, "Str: %d(%d)  Int: %d(%d)  Wis: %d(%d)  Dex: %d(%d)  Con: %d(%d) Cha: %d(%d)\n", // notrans
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


	buf_printf(output, BUF_END, "Hp: %d/%d  Mana: %d/%d  Move: %d/%d  Practices: %d Luck: %d\n", // notrans
		victim->hit,         victim->max_hit,
		victim->mana,        victim->max_mana,
		victim->move,        victim->max_move,
		IS_NPC(ch) ? 0 : PC(victim)->practice,
		GET_LUCK(victim));

	if (IS_NPC(victim))
		snprintf(buf, sizeof(buf), "%d", victim->alignment);
	else  {
/*
		buf_printf(output, BUF_END, "It belives the religion of %s.\n",
			religion_table[PC(victim)->religion].leader);
*/
		snprintf(buf, sizeof(buf), "%s-%s",		// notrans
			 flag_string(ethos_table, victim->ethos),
			 flag_string(align_names, NALIGN(victim)));
	}

	buf_printf(output, BUF_END,
		"Lv: [%d + %d]  Class: [%s]  Align: [%s]  Gold: [%d]  Silver: [%d]  Exp: [%d]\n", // notrans
		victim->level,
		victim->add_level,
		victim->class,
		buf,
		victim->gold, victim->silver, GET_EXP(victim));

	buf_printf(output, BUF_END,"Armor: pierce: %d  bash: %d  slash: %d  magic: %d\n", // notrans
		    GET_AC(victim,AC_PIERCE), GET_AC(victim,AC_BASH),
		    GET_AC(victim,AC_SLASH),  GET_AC(victim,AC_EXOTIC));

	buf_printf(output, BUF_END,
		"Hit: %d  Dam: %d  Saves: %d  Size: %s  Position: %s  Wimpy: %d\n", // notrans
		GET_HITROLL(victim), GET_DAMROLL(victim), victim->saving_throw,
		flag_string(size_table, victim->size),
		flag_string(position_table, victim->position),
		victim->wimpy);

	if (IS_NPC(victim)) {
		MOB_INDEX_DATA *pMobIndex = victim->pMobIndex;

		buf_printf(output, BUF_END,
			   "Damage: %dd%d  Message:  %s\n",	// notrans
			   NPC(victim)->dam.dice_number,
			   NPC(victim)->dam.dice_type,
			   victim->damtype);
		buf_printf(output, BUF_END, "Act: [%s]\n",	// notrans
			   flag_string(mob_act_flags, pMobIndex->act));
		if (pMobIndex->mob_flags) {
			buf_printf(output, BUF_END, "Mob: [%s]\n", // notrans
				   flag_string(mob_flags, pMobIndex->mob_flags));
		}
		buf_printf(output, BUF_END, "Off: [%s]\n",	// notrans
			   flag_string(off_flags, pMobIndex->off_flags));
	}

	buf_printf(output, BUF_END, "Fighting: %s Deaths: %d Carry number: %d  Carry weight: %d\n", // notrans
		   victim->fighting ? victim->fighting->name : "(none)" , // notrans
		   IS_NPC(victim) ? 0 : PC(victim)->death,
		   victim->carry_number, get_carry_weight(victim) / 10);

	if (!IS_NPC(victim)) {
		buf_printf(output, BUF_END,
			   "Thirst: %d  Hunger: %d  Full: %d  "	   // notrans
			   "Drunk: %d Bloodlust: %d Desire: %d\n", // notrans
			   PC(victim)->condition[COND_THIRST],
			   PC(victim)->condition[COND_HUNGER],
			   PC(victim)->condition[COND_FULL],
			   PC(victim)->condition[COND_DRUNK],
			   PC(victim)->condition[COND_BLOODLUST],
			   PC(victim)->condition[COND_DESIRE]);
		buf_printf(output, BUF_END, "Age: [%d]  Played: [%d]  Timer: [%d]\n", // notrans
			   get_age(victim), get_hours(victim),
			   PC(victim)->idle_timer);

		buf_printf(output, BUF_END, "Plr: [%s]\n",	// notrans
			   flag_string(plr_flags, PC(victim)->plr_flags));
	}

	if (victim->comm) {
		buf_printf(output, BUF_END, "Comm: [%s]\n",	// notrans
			   flag_string(comm_flags, victim->comm));
	}

	if (victim->chan) {
		buf_printf(output, BUF_END, "Chan: [%s]\n",	// notrans
			   flag_string(chan_flags, victim->chan));
	}

	if (IS_NPC(victim) && victim->pMobIndex->off_flags) {
		buf_printf(output, BUF_END, "Offense: [%s]\n",	// notrans
			   flag_string(off_flags,
				       victim->pMobIndex->off_flags));
	}

	buf_printf(output, BUF_END, "Form: [%s]\n",		// notrans
		   flag_string(form_flags, victim->form));
	buf_printf(output, BUF_END, "Parts: [%s]\n",		// notrans
		   flag_string(part_flags, victim->parts));

	if (victim->affected_by) {
		buf_printf(output, BUF_END, "Affected by %s\n",	// notrans
			   flag_string(affect_flags, victim->affected_by));
	}

	if (victim->has_invis) {
		buf_printf(output, BUF_END, "Has '%s'\n",	// notrans
			   flag_string(id_flags, victim->has_invis));
	}

	if (victim->has_detect) {
		buf_printf(output, BUF_END,
			   "Has detection of '%s'\n",		// notrans
			   flag_string(id_flags, victim->has_detect));
	}

	pet = GET_PET(victim);
	buf_printf(output, BUF_END, "Master: %s  Leader: %s  Pet: %s\n", // notrans
		victim->master	? victim->master->name	: "(none)",  // notrans
		victim->leader	? victim->leader->name	: "(none)",  // notrans
		pet		? pet->name		: "(none)"); // notrans

	/* OLC */
	if (!IS_NPC(victim)) {
		buf_printf(output, BUF_END, "Security: [%d]\n",	     // notrans
			   PC(victim)->security);
	}

	mlstr_dump(output, "Short description: ",		     // notrans
		   &victim->short_descr, DUMP_LEVEL(ch));
	if (IS_NPC(victim)) {
		buf_append(output, "Long description:\n");	     // notrans
		mlstr_dump(output, str_empty,
			   &victim->long_descr, DUMP_LEVEL(ch));
		if (victim->pMobIndex->spec_fun != 0) {
			buf_printf(output, BUF_END, "Mobile has special procedure %s.\n", // notrans
				   mob_spec_name(victim->pMobIndex->spec_fun));
		}
	}

	show_affects(ch, victim, output);

	if (!varr_isempty(&victim->sk_affected)) {
		buf_append(output, "Skill affects:\n");		// notrans
		varr_foreach(&victim->sk_affected, print_sa_cb, output);
	}

	if (!IS_NPC(victim)) {
		if (IS_ON_QUEST(victim)) {
			buf_printf(output, BUF_END,
				   "Questgiver: [%d]  QuestPnts: [%d]  Questnext: [%d]\n", // notrans
				   PC(victim)->questgiver,
				   PC(victim)->questpoints,
				   PC(victim)->questtime < 0 ?
					-PC(victim)->questtime : 0);
			buf_printf(output, BUF_END,
				   "QuestCntDown: [%d]  QuestObj:  [%d]  Questmob: [%d]\n", // notrans
				   PC(victim)->questtime > 0 ?
					PC(victim)->questtime : 0,
				   PC(victim)->questobj,
				   PC(victim)->questmob);
		} else {
			buf_printf(output, BUF_END,
				   "QuestPnts: [%d]  Questnext: [%d]  NOT QUESTING\n",	// notrans
				   PC(victim)->questpoints,
				   PC(victim)->questtime < 0 ?
					-PC(victim)->questtime : 0);
		}

		buf_append(output, "Quest Troubles: ");		// notrans
		qtrouble_dump(output, victim);
		buf_append(output, "\n");

		if (!IS_NULLSTR(PC(victim)->twitlist))
			buf_printf(output, BUF_END,
				   "Twitlist: [%s]\n",		// notrans
				   PC(victim)->twitlist);
	}

	buf_printf(output, BUF_END,
		   "Last fight time: [%s] %s\n",		// notrans
		   victim->last_fight_time != 1 ?
		   strtime(victim->last_fight_time) : "NONE",	// notrans
		   IS_PUMPED(victim) ? "(adrenalin is gushing)" : str_empty); // notrans

	buf_printf(output, BUF_END, "Wait state: %d\n", ch->wait); // notrans
	if (IS_NPC(victim)) {
		NPC_DATA *npc = NPC(victim);
		buf_printf(output, BUF_END,
			   "Last fought: [%s]  In_mind: [%s] Target: [%s]\n", // notrans
			   npc->last_fought ?
			   npc->last_fought->name : "none",
			   npc->in_mind ? npc->in_mind : "none",
			   npc->target ? npc->target->name : "none");
	}

	for (i = 0, j = 0; i < MAX_RESIST; i++) {
		const char *dam_class;
		int resist;

		if ((resist = get_resist(victim, i, TRUE)) == 0)
			continue;

		if (!found) {
			buf_append(output, "Resists:\n");		// notrans
			found = TRUE;
		}

		dam_class = flag_string(dam_classes, i);
		if (strlen(dam_class) > 7) {
			buf_printf(output, BUF_END, "\t%s\t%d%%",	// notrans
			    dam_class, resist);
		} else {
			buf_printf(output, BUF_END, "\t%s\t\t%d%%",// notrans
			    dam_class, resist);
		}

		if (++j % 3 == 0)
			buf_append(output, "\n");
	}

	if (j % 3 != 0)
		buf_append(output, "\n");

	page_to_char(buf_string(output), ch);
	buf_free(output);

cleanup:
	if (loaded)
		char_nuke(victim);
}

DO_FUN(do_dstat, ch, argument)
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
		act_char("dstat: descriptor not found", ch);	// notrans
		return;
	}

	output = buf_new(0);

	buf_printf(output, BUF_END, "Desc: [%d]  Conn: [%d]  "	 // notrans
			   "Outsize: [%d]  Outtop:  [%d]\n"	 // notrans
			   "Snoopsize: [%d]  Snooptop:  [%d]\n", // notrans
		   d->descriptor, d->connected,
		   d->out_buf.size, d->out_buf.top,
		   d->snoop_buf.size, d->snoop_buf.top);
	buf_printf(output, BUF_END, "Inbuf: [%s]\n", d->inbuf);	    // notrans
	buf_printf(output, BUF_END, "Incomm: [%s]\n", d->incomm);   // notrans
	buf_printf(output, BUF_END, "Repeat: [%d]  Inlast: [%s]\n", // notrans
		   d->repeat, d->inlast);
	if (d->character)
		buf_printf(output, BUF_END,
			   "Ch: [%s]\n", d->character->name);	    // notrans
	if (d->original)
		buf_printf(output, BUF_END,
			   "Original: [%s]\n", d->original->name);  // notrans
	if (d->olced)
		buf_printf(output, BUF_END,
			   "OlcEd: [%s]\n", d->olced->name);	    // notrans
	if (d->pString)
		buf_printf(output, BUF_END,
			   "pString: [%s]\n", *d->pString);	    // notrans
	if (d->showstr_head)
		buf_printf(output, BUF_END,
			   "showstr_head: [%s]\n", d->showstr_head); // notrans

	page_to_char(buf_string(output), ch);
	buf_free(output);
}

DO_FUN(do_vnum, ch, argument)
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

DO_FUN(do_mfind, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];
	int i;
	BUFFER *buf = NULL;

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		act_char("Find whom?", ch);
		return;
	}

	for (i = 0; i < MAX_KEY_HASH; i++) {
		MOB_INDEX_DATA *mob_index;

		for (mob_index = mob_index_hash[i]; mob_index; mob_index = mob_index->next) {
			if (!is_name(argument, mob_index->name))
				continue;

			if (buf == NULL)
				buf = buf_new(0);

			buf_printf(buf, BUF_END, "[%5d] %s\n",	// notrans
				   mob_index->vnum,
				   mlstr_mval(&mob_index->short_descr));
		}
	}

	if (!buf)
		act_char("No mobiles by that name.", ch);
	else {
		page_to_char(buf_string(buf), ch);
		buf_free(buf);
	}
}

DO_FUN(do_ofind, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];
	int i;
	BUFFER *buf = NULL;

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		act_char("Find what?", ch);
		return;
	}

	for (i = 0; i < MAX_KEY_HASH; i++) {
		OBJ_INDEX_DATA *obj_index;

		for (obj_index = obj_index_hash[i]; obj_index; obj_index = obj_index->next) {
			if (!is_name(argument, obj_index->name))
				continue;

			if (buf == NULL)
				buf = buf_new(0);

			buf_printf(buf, BUF_END, "[%5d] %s\n",	// notrans
				   obj_index->vnum,
				   mlstr_mval(&obj_index->short_descr));
		}
	}

	if (!buf)
		act_char("No objects by that name.", ch);
	else {
		page_to_char(buf_string(buf), ch);
		buf_free(buf);
	}
}

DO_FUN(do_owhere, ch, argument)
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
		|| (vnum < 0 && !IS_OBJ_NAME(obj, argument)))
			continue;

		if (buffer == NULL)
			buffer = buf_new(0);
		number++;

		for (in_obj = obj; in_obj->in_obj != NULL;
		     in_obj = in_obj->in_obj)
			;

		if (in_obj->carried_by != NULL
		&&  can_see(ch,in_obj->carried_by)
		&&  in_obj->carried_by->in_room != NULL)
			buf_printf(buffer, BUF_END,
			    "%3d) %s is carried by %s [Room %d]\n", // notrans
			    number, mlstr_mval(&obj->short_descr),
			    PERS(in_obj->carried_by, ch, GET_LANG(ch), ACT_FORMSH),
			    in_obj->carried_by->in_room->vnum);
		else if (in_obj->in_room != NULL
		     &&  can_see_room(ch, in_obj->in_room))
			buf_printf(buffer, BUF_END, "%3d) %s is in %s [Room %d]\n", // notrans
				number, mlstr_mval(&obj->short_descr),
				mlstr_cval(&in_obj->in_room->name, ch),
				in_obj->in_room->vnum);
		else
			buf_printf(buffer, BUF_END, "%3d) %s is somewhere\n", number, // notrans
				mlstr_mval(&obj->short_descr));

	    if (number >= max_found)
	        break;
	}

	if (buffer == NULL)
		act_char("Nothing like that in heaven or earth.", ch);
	else {
		page_to_char(buf_string(buffer),ch);
		buf_free(buffer);
	}
}

DO_FUN(do_mwhere, ch, argument)
{
	BUFFER *buffer;
	CHAR_DATA *victim;
	int count = 0, vnum = -1;

	if (argument[0] == '\0') {
		DESCRIPTOR_DATA *d;

		/* show characters logged */

		buffer = buf_new(0);
		for (d = descriptor_list; d != NULL; d = d->next) {
			if (d->character == NULL
			||  d->connected != CON_PLAYING
			||  d->character->in_room == NULL
			||  !can_see(ch, d->character)
			||  !can_see_room(ch, d->character->in_room))
				continue;

			victim = d->character;
			count++;
			if (d->original != NULL) {
				buf_printf(buffer, BUF_END,
					   "%3d) %s (in the body of %s) is in %s [%d]\n", // notrans
					   count,
					   d->original->name,
					   mlstr_mval(&victim->short_descr),
					   mlstr_mval(&victim->in_room->name),
					   victim->in_room->vnum);
			} else {
				buf_printf(buffer, BUF_END,
					   "%3d) %s is in %s [%d]\n",  // notrans
					   count,
					   victim->name,
					   mlstr_mval(&victim->in_room->name),
					   victim->in_room->vnum);
			}
		}

		page_to_char(buf_string(buffer),ch);
		buf_free(buffer);
		return;
	}

	buffer = NULL;
	if (is_number(argument))
		vnum = atoi(argument);

	for (victim = char_list; victim; victim = victim->next) {
		if (victim->in_room
		&&  can_see(ch, victim)
		&&  (is_name(argument, victim->name) ||
		     (IS_NPC(victim) && victim->pMobIndex->vnum == vnum))) {
			if (buffer == NULL)
				buffer = buf_new(0);

			count++;
			buf_printf(buffer, BUF_END,
				   "%3d) [%5d] %-28s [%5d] %s\n",  // notrans
				   count,
				   IS_NPC(victim) ? victim->pMobIndex->vnum : 0,
				   IS_NPC(victim) ?
					mlstr_mval(&victim->short_descr) :
					victim->name,
				   victim->in_room->vnum,
				   mlstr_mval(&victim->in_room->name));
		}
	}

	if (buffer) {
		page_to_char(buf_string(buffer),ch);
		buf_free(buffer);
	} else
		act("You didn't find any $T.", ch, NULL, argument, TO_CHAR);
}

DO_FUN(do_protect, ch, argument)
{
	CHAR_DATA *victim;
	bool loaded = FALSE;
	bool altered = FALSE;

	if (argument[0] == '\0') {
		do_help(ch, "'WIZ PROTECT'");
		return;
	}

	if ((victim = get_char_world(ch, argument)) == NULL) {
		if ((victim = char_load(argument, LOAD_F_NOCREATE)) == NULL) {
			act_char("You can't find them.", ch);
			return;
		}
		loaded = TRUE;
	}

	if (!IS_TRUSTED(ch, trust_level(victim))) {
		act_char("You failed.", ch);
		goto cleanup;
	}

	TOGGLE_BIT(victim->comm, COMM_SNOOP_PROOF);
	altered = TRUE;

	if (!IS_SET(victim->comm, COMM_SNOOP_PROOF)) {
		act_puts("$N is no longer snoop-proof.", ch, NULL, victim,
			 TO_CHAR, POS_DEAD);
		act_char("Your snoop-proofing was just removed.", victim);
	} else {
		act_puts("$N is now snoop-proof.", ch, NULL, victim, TO_CHAR,
			 POS_DEAD);
		act_char("You are now immune to snooping.", victim);
	}

cleanup:
	if (altered)
		char_save(victim, loaded ? SAVE_F_PSCAN : 0);
	if (loaded)
		char_nuke(victim);
}

DO_FUN(do_snoop, ch, argument)
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
		act_char("They aren't here.", ch);
		return;
	}

	if (victim->desc == NULL) {
		act_char("No descriptor to snoop.", ch);	// notrans
		return;
	}

	if (victim == ch) {
		act_char("Cancelling all snoops.", ch);		// notrans
		wiznet("$N stops being such a snoop.",		// notrans
		       ch, NULL, WIZ_SNOOPS,
		       WIZ_SECURE, trust_level(ch));
		for (d = descriptor_list; d != NULL; d = d->next)
			if (d->snoop_by == ch->desc)
				d->snoop_by = NULL;
		return;
	}

	if (victim->desc->snoop_by != NULL) {
		act_char("Busy already.", ch);
		return;
	}

	if (ch->in_room != victim->in_room
	&&  room_is_private(victim->in_room)
	&&  !IS_TRUSTED(ch, LEVEL_IMP)) {
		act_char("That character is in a private room.", ch); // notrans
		return;
	}

	if (!IS_TRUSTED(ch, trust_level(victim))
	||  IS_SET(victim->comm, COMM_SNOOP_PROOF)) {
		act_char("You failed.", ch);
		return;
	}

	if (ch->desc != NULL) {
		for (d = ch->desc->snoop_by; d != NULL; d = d->snoop_by) {
			if (d->character == victim || d->original == victim) {
				act_char("No snoop loops.", ch); // notrans
				return;
			}
		}
	}

	victim->desc->snoop_by = ch->desc;
	wiznet("$N starts snooping on $i.",
		ch, victim, WIZ_SNOOPS, WIZ_SECURE, trust_level(ch));
	act_char("Ok.", ch);
}

DO_FUN(do_switch, ch, argument)
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
		act_char("You are already switched.", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg)) == NULL) {
		act_char("They aren't here.", ch);
		return;
	}

	if (victim == ch) {
		act_char("Ok.", ch);
		return;
	}

	if (!IS_NPC(victim)) {
		act_char("You can only switch into mobiles.", ch);
		return;
	}

	if (victim->desc != NULL) {
		act_char("Character in use.", ch);
		return;
	}

	wiznet("$N switches into $i.",
		ch, victim, WIZ_SWITCHES, WIZ_SECURE, trust_level(ch));

	ch->desc->character = victim;
	ch->desc->original  = ch;
	victim->desc        = ch->desc;
	ch->desc            = NULL;
	victim->comm = ch->comm;
	act_char("Ok.", victim);
}

DO_FUN(do_return, ch, argument)
{
	CHAR_DATA *vch = GET_ORIGINAL(ch);

	if (vch == NULL || vch == ch)
		return;

	act_char("You return to your original body.", ch);
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
DO_FUN(do_clone, ch, argument)
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

	if (!str_prefix(arg, "object")) {
		mob = NULL;
		obj = get_obj_here(ch,rest);
		if (obj == NULL) {
		    act_char("You don't see that here.", ch);
		    return;
		}
	} else if (!str_prefix(arg, "mobile") || !str_prefix(arg, "character")) {
		obj = NULL;
		mob = get_char_here(ch, rest);
		if (mob == NULL) {
		    act_char("You don't see that here.", ch);
		    return;
		}
	} else { /* find both */
		mob = get_char_here(ch, argument);
		obj = get_obj_here(ch, argument);
		if (mob == NULL && obj == NULL) {
			act_char("You don't see that here.", ch);
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
		OBJ_DATA *nobj;

		if (!IS_NPC(mob)) {
		    act_char("You can only clone mobiles.", ch);
		    return;
		}

		clone = clone_mob(mob);

		for (obj = mob->carrying; obj != NULL; obj = obj->next_content) {
			nobj = clone_obj(obj);
			recursive_clone(ch, obj, nobj);
			obj_to_char(nobj, clone);
			nobj->wear_loc = obj->wear_loc;
		}
		act("$n has created $N.", ch, NULL, clone, TO_ROOM);
		act("You clone $N.", ch, NULL, clone, TO_CHAR);
		wiznet("$N clones $i.",
			ch, clone, WIZ_LOAD, WIZ_SECURE, trust_level(ch));
		char_to_room(clone, ch->in_room);
	}
}

/* RT to replace the two load commands */

DO_FUN(do_load, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];

	argument = one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		do_help(ch, "'WIZ LOAD'");
		return;
	}

	if (!str_cmp(arg, "mob") || !str_cmp(arg, "char")) {
		do_mload(ch, argument);
		return;
	}

	if (!str_cmp(arg, "obj")) {
		do_oload(ch, argument);
		return;
	}

	/* echo syntax */
	do_load(ch, str_empty);
}

DO_FUN(do_mload, ch, argument)
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
		act_puts("$j: No mob has that vnum.",
			 ch, (const void *) vnum, NULL, TO_CHAR, POS_DEAD);
		return;
	}

	/*
	 * create_mob can't return NULL because get_mob_index returned not NULL
	 */
	victim = create_mob(pMobIndex->vnum, 0);
	act("$n has created $N!", ch, NULL, victim, TO_ROOM);
	wiznet("$N loads $i.", ch, victim,
	       WIZ_LOAD, WIZ_SECURE, trust_level(ch));
	act_char("Ok.", ch);
	char_to_room(victim, ch->in_room);
}

DO_FUN(do_oload, ch, argument)
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
		act_puts("$j: No objects with this vnum.",
			 ch, (const void *) vnum, NULL, TO_CHAR, POS_DEAD);
		return;
	}

	/*
	 * create_obj can't return NULL because get_obj_index returned not NULL
	 */
	obj = create_obj(pObjIndex->vnum, 0);
	if (CAN_WEAR(obj, ITEM_TAKE))
		obj_to_char(obj, ch);
	else
		obj_to_room(obj, ch->in_room);
	act("$n has created $p!", ch, obj, NULL, TO_ROOM);
	wiznet("$N loads $p.", ch, obj, WIZ_LOAD, WIZ_SECURE, trust_level(ch));
	act_char("Ok.", ch);
}

DO_FUN(do_purge, ch, argument)
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
		act_char("Ok.", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg)) == NULL) {
		act_char("They aren't here.", ch);
		return;
	}

	if (!IS_NPC(victim)) {
		if (!IS_TRUSTED(ch, LEVEL_GOD)) {
			act_char("Not at your level.", ch);
			return;
		}
		if (ch == victim) {
			act_char("Ho ho ho.", ch);
			return;
		}

		if (!IS_TRUSTED(ch, trust_level(victim))) {
			act_char("Maybe that wasn't a good idea...", ch);
			act_puts("$t tried to purge you!", ch, ch->name, NULL,
				 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
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

DO_FUN(do_restore, ch, argument)
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
		act_char("Room restored.", ch);
		return;
	}
	
	if (ch->level >= MAX_LEVEL - 1
	&&  !str_cmp(arg, "all")) {
		/*
		 * cure all
		 */
		for (d = descriptor_list; d != NULL; d = d->next) {
			if (d->connected != CON_PLAYING)
				continue;

			vch = d->character;

			if (vch == NULL || IS_NPC(vch))
				continue;
	            
			restore_char(ch, vch);
		}
		wiznet("$N restored all active players.",
			ch, NULL, WIZ_RESTORE, WIZ_SECURE, trust_level(ch));
		act_char("All active players restored.", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg)) == NULL) {
		act_char("They aren't here.", ch);
		return;
	}

	restore_char(ch, victim);
	wiznet("$N restored $i.",
		ch, victim, WIZ_RESTORE, WIZ_SECURE, trust_level(ch));
	act_char("Ok.", ch);
}
		
DO_FUN(do_freeze, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	bool loaded = FALSE;
	bool altered = FALSE;

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		do_help(ch, "'WIZ FREEZE'");
		return;
	}

	if ((victim = get_char_world(ch, arg)) == NULL) {
		if ((victim = char_load(arg, LOAD_F_NOCREATE)) == NULL) {
			act_char("They aren't here.", ch);
			return;
		}
		loaded = TRUE;
	} else if (IS_NPC(victim)) {
		act_char("Not on NPC's.", ch);
		return;
	}

	if (!IS_TRUSTED(ch, trust_level(victim))) {
		act_char("You failed.", ch);
		goto cleanup;
	}

	TOGGLE_BIT(PC(victim)->plr_flags, PLR_FREEZE);
	altered = TRUE;

	if (!IS_SET(PC(victim)->plr_flags, PLR_FREEZE)) {
		act_char("You can play again.", victim);
		act_char("FREEZE removed.", ch);
		wiznet("$N thaws $i.",
			ch, victim, WIZ_PENALTIES, WIZ_SECURE, 0);
	} else {
		act_char("You can't do ANYthing!", victim);
		act_char("FREEZE set.", ch);
		wiznet("$N puts $i in the deep freeze.",
			ch, victim, WIZ_PENALTIES, WIZ_SECURE, 0);
	}

cleanup:
	if (altered)
		char_save(victim, loaded ? SAVE_F_PSCAN : 0);
	if (loaded)
		char_nuke(victim);
}

DO_FUN(do_log, ch, argument)
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
			act_char("Log ALL off.", ch);
		} else {
			fLogAll = TRUE;
			act_char("Log ALL on.", ch);
		}
		return;
	}

	if ((victim = get_char_world(ch, arg)) == NULL) {
		act_char("They aren't here.", ch);
		return;
	}

	if (IS_NPC(victim)) {
		act_char("Not on NPC's.", ch);
		return;
	}

	/*
	 * No level check, gods can log anyone.
	 */
	TOGGLE_BIT(PC(victim)->plr_flags, PLR_LOG);
	if (!IS_SET(PC(victim)->plr_flags, PLR_LOG))
		act_char("LOG removed.", ch);
	else 
		act_char("LOG set.", ch);
}

DO_FUN(do_noemote, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	bool loaded = FALSE;
	bool altered = FALSE;

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		do_help(ch, "'WIZ NOEMOTE'");
		return;              
	}

	if ((victim = get_char_world(ch, arg)) == NULL) {
		if ((victim = char_load(arg, LOAD_F_NOCREATE)) == NULL) {
			act_char("They aren't here.", ch);
			return;
		}
		loaded = TRUE;
	}

	if (!IS_TRUSTED(ch, trust_level(victim))) {
		act_char("You failed.", ch);
		goto cleanup;
	}

	TOGGLE_BIT(victim->comm, COMM_NOEMOTE);
	altered = TRUE;

	if (!IS_SET(victim->comm, COMM_NOEMOTE)) {
		act_char("You can emote again.", victim);
		act_char("NOEMOTE removed.", ch);
		wiznet("$N restores emotes to $i.",
			ch, victim, WIZ_PENALTIES, WIZ_SECURE, 0);
	} else {
		act_char("You can't emote!", victim);
		act_char("NOEMOTE set.", ch);
		wiznet("$N revokes $i's emotes.",
			ch, victim, WIZ_PENALTIES, WIZ_SECURE, 0);
	}

cleanup:
	if (altered)
		char_save(victim, loaded ? SAVE_F_PSCAN : 0);
	if (loaded) 
		char_nuke(victim);
}

DO_FUN(do_notell, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	bool loaded = FALSE;
	bool altered = FALSE;

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		do_help(ch, "'WIZ NOTELL'");
		return;
	}

	if ((victim = get_char_world(ch, arg)) == NULL) {
		if ((victim = char_load(arg, LOAD_F_NOCREATE)) == NULL) {
			act_char("They aren't here.", ch);
			return;
		}
		loaded = TRUE;
	}

	if (!IS_TRUSTED(ch, trust_level(victim))) {
		act_char("You failed.", ch);
		goto cleanup;
	}

	TOGGLE_BIT(victim->comm, COMM_NOTELL);
	altered = TRUE;

	if (!IS_SET(victim->comm, COMM_NOTELL)) {
		act_char("You can tell again.", victim);
		act_char("NOTELL removed.", ch);
		wiznet("$N restores tells to $i.",
			ch, victim, WIZ_PENALTIES, WIZ_SECURE, 0);
	} else {
		act_char("You can't tell!", victim);
		act_char("NOTELL set.", ch);
		wiznet("$N revokes $i's tells.",
			ch, victim, WIZ_PENALTIES, WIZ_SECURE, 0);
	}

cleanup:
	if (altered)
		char_save(victim, loaded ? SAVE_F_PSCAN : 0);
	if (loaded) 
		char_nuke(victim);
}

DO_FUN(do_peace, ch, argument)
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
			AFFECT_DATA *paf;

			paf = aff_new(TO_AFFECTS, "calm");
			paf->level	= MAX_LEVEL;
			paf->duration	= 15;
			paf->bitvector	= AFF_CALM;
			affect_to_char(rch, paf);
			aff_free(paf);
		}
	}

	act_char("Ok.", ch);
}

DO_FUN(do_wizlock, ch, argument)
{
	wizlock = !wizlock;

	if (wizlock) {
		wiznet("$N has wizlocked the game.", ch, NULL, 0, 0, 0);
		act_char("Game wizlocked.", ch);
	} else {
		wiznet("$N removes wizlock.", ch, NULL, 0, 0, 0);
		act_char("Game un-wizlocked.", ch);
	}
}

/* RT anti-newbie code */
DO_FUN(do_newlock, ch, argument)
{
	newlock = !newlock;

	if (newlock) {
		wiznet("$N locks out new characters.", ch, NULL, 0, 0, 0);
		act_char("New characters have been locked out.", ch);
	} else {
		wiznet("$N allows new characters back in.", ch, NULL, 0, 0, 0);
		act_char("Newlock removed.", ch);
	}
}

/* RT set replaces sset, mset, oset, and rset */
DO_FUN(do_set, ch, argument)
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

DO_FUN(do_sset, ch, argument)
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
		act_char("They aren't here.", ch);
		return;
	}

	if (IS_NPC(victim)) {
		act_char("Not on NPC's.", ch);
		return;
	}

	/*
	 * Snarf the value.
	 */
	if (!is_number(arg3)) {
		act_char("Value must be numeric.", ch);
		return;
	}

	value = atoi(arg3);
	if (value < 0) {
		act_char("Value should not be negative.", ch);
		return;
	}

	if (!str_cmp(arg2, "all")) {
		hash_foreach(&skills, sset_cb, victim, value);
		act_char("Ok.", ch);
	} else {
		const char *sn;
		skill_t *sk;

		if ((sk = skill_search(arg2)) == NULL) {
			if (value) {
				act_char("do_sset: Cannot set non-zero value for unknown skill.", ch);
				return;
			}
			sn = arg2;
		} else
			sn = gmlstr_mval(&sk->sk_name);

		set_skill(victim, sn, value);
		act_puts("do_sset: '$T': $j%", ch, (const void *) value, sn,
			 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
	}
	update_skills(victim);
}

DO_FUN(do_string, ch, argument)
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
		if ((victim = get_char_here(ch, arg1)) == NULL) {
			act_char("They aren't here.", ch);
			return;
		}

		/* clear zone for mobs */
		if (IS_NPC(victim))
			NPC(victim)->zone = NULL;

		/* string something */

	 	if (!str_prefix(arg2, "name")) {
			if (!IS_NPC(victim)) {
				act_char("Not on PC's.", ch);
				return;
			}
			free_string(victim->name);
			victim->name = str_dup(arg3);
			return;
		}
		
		if (!str_prefix(arg2, "short")) {
			if (!IS_NPC(victim)) {
				act_char("Not on PC's.", ch);
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
				act_char("Not on PC's.", ch);
				return;
			}
			mlstr_edit(&victim->long_descr, arg3);
			return;
		}
	}
	
	if (!str_prefix(type, "object")) {
		/* string an obj */
		
	 	if ((obj = get_obj_room(ch, arg1)) == NULL) {
			act_char("Nothing like that in heaven or earth.", ch);
			return;
		}
		
		if (obj->pObjIndex->limit >= 0) {
			act_char("You cannot string limited objs.", ch);
			return;
		}

		if (CAN_WEAR(obj, ITEM_WEAR_CLANMARK)) {
			act_char("You cannot string clan marks.", ch);
			return;
		}

		if (!str_prefix(arg2, "label")) {
			name_toggle(&obj->label, arg3, ch, "label");
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
				act_char("Obj must be in your inventory.", ch);
				return;
			}

			argument = one_argument(argument, arg3, sizeof(arg3));
			if (argument == NULL) {
				act_char("Syntax: oset <object> ed <keyword> lang", ch); // notrans
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

DO_FUN(do_oset, ch, argument)
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
		act_char("Nothing like that in heaven or earth.", ch);
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

DO_FUN(do_rset, ch, argument)
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
		act_char("No such location.", ch);
		return;
	}

	if (ch->in_room != location 
	&&  room_is_private(location) && !IS_TRUSTED(ch, LEVEL_IMP)) {
		act_char("That room is private right now.", ch);
		return;
	}

	/*
	 * Snarf the value.
	 */
	if (!is_number(arg3)) {
		act_char("Value must be numeric.", ch);
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

DO_FUN(do_sockets, ch, argument)
{
	BUFFER *output;
	char arg[MAX_INPUT_LENGTH];
	DESCRIPTOR_DATA *d;
	int count;

	count = 0;
	output = buf_new(GET_LANG(ch));

	one_argument(argument, arg, sizeof(arg));
	if (strchr(arg, '@') == NULL) 
		strnzcat(arg, sizeof(arg), "*@*");		// notrans

	for (d = descriptor_list; d; d = d->next) {
		char buf[MAX_STRING_LENGTH];
		CHAR_DATA *vch = d->original ? d->original : d->character;

		if (vch && !can_see(ch, vch))
			continue;

		snprintf(buf, sizeof(buf), "%s@%s",		// notrans
			 vch ? vch->name : NULL,
			 d->host);

		if (fnmatch(arg, buf, FNM_CASEFOLD) != 0)
			continue;

		count++;
		buf_printf(output, BUF_END,
			   "[%3d %12s] %s (%s)",		// notrans
			   d->descriptor,
			   flag_string(desc_con_table, d->connected),
			   buf,
			   d->ip);
		if (vch && PC(vch)->idle_timer)
			buf_printf(output, BUF_END, " idle %d", PC(vch)->idle_timer);	// notrans
		buf_append(output, "\n");
	}

	if (count == 0) {
		act_char("No one by that name is connected.", ch);
		buf_free(output);
		return;
	}

	buf_printf(output, BUF_END, "%d user%s\n",		// notrans
		   count, count == 1 ? str_empty : "s");	// notrans
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
	interpret(vch, argument, TRUE);
	return NULL;
}

/*
 * Thanks to Grodyn for pointing out bugs in this function.
 */
DO_FUN(do_force, ch, argument)
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
		act_char("That will NOT be done.", ch);
		return;
	}

	if (!str_cmp(arg, "all")
	||  !str_cmp(arg, "players")) {
		if (!IS_TRUSTED(ch, MAX_LEVEL - 3)) {
			act_char("Not at your level!", ch);
			return;
		}

		vo_foreach(NULL, &iter_char_world, force_cb,
			   ch, argument, FALSE);
		act_char("Ok.", ch);
		return;
	} else if (!str_cmp(arg, "gods")) {
		if (!IS_TRUSTED(ch, MAX_LEVEL - 2)) {
			act_char("Not at your level!", ch);
			return;
		}
	
		vo_foreach(NULL, &iter_char_world, force_cb,
			   ch, argument, TRUE);
		act_char("Ok.", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg)) == NULL) {
		act_char("They aren't here.", ch);
		return;
	}

	if (victim == ch) {
		act_char("Aye aye, right away!", ch);
		return;
	}

	if (ch->in_room != victim->in_room
	&&  room_is_private(victim->in_room)
	&&  !IS_TRUSTED(ch, LEVEL_IMP)) {
		act_char("That character is in a private room.", ch); // notrans
		return;
	}

	if (!IS_NPC(victim)
	&&  !IS_TRUSTED(ch, MAX_LEVEL-3)) {
		act_char("Not at your level!", ch);
		return;
	}

	if (!IS_TRUSTED(ch, trust_level(victim))) {
		act_char("Do it yourself!", ch);
		return;
	}

	act_puts("$n forces you to '$t'.",
		 ch, argument, victim, TO_VICT, POS_DEAD);
	interpret(victim, argument, FALSE);

	act_char("Ok.", ch);
}

/*
 * New routines by Dionysos.
 */
DO_FUN(do_invis, ch, argument)
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
			act_char("You slowly fade back into existence.", ch);
		} else {
			act("$n slowly fades into thin air.",
			    ch, NULL, NULL, TO_ROOM);
			ch->invis_level = UMIN(LEVEL_IMMORTAL, trust_level(ch));
			act_char("You slowly vanish into thin air.", ch);
		}
	} else {
		/*
		 * do the level thing
		 */
		level = atoi(arg);
		if (level < 2 || level > trust_level(ch)) {
			int tlevel = trust_level(ch);
			act_puts("Invis level must be in range 2..$j.",
				 ch, (const void *) tlevel, NULL,
				 TO_CHAR, POS_DEAD);
			return;
		} else {
			act("$n slowly fades into thin air.",
			    ch, NULL, NULL, TO_ROOM);
			ch->invis_level = level;
			act_char("You slowly vanish into thin air.", ch);
		}
	}
}

DO_FUN(do_incognito, ch, argument)
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
			act_char("You are no longer cloaked.", ch);
		} else {
			ch->incog_level = UMIN(LEVEL_IMMORTAL, trust_level(ch));
			act("$n cloaks $s presence.", ch, NULL, NULL, TO_ROOM);
			act_char("You cloak your presence.", ch);
		}
	} else {
		/*
		 * do the level thing
		 */
		level = atoi(arg);
		if (level < 2 || level > ch->level) {
			int tlevel = trust_level(ch);
			act_puts("Incog level must be in range 2..$j.",
				 ch, (const void *) tlevel, NULL,
				 TO_CHAR, POS_DEAD);
			return;
		} else {
			ch->incog_level = level;
			act("$n cloaks $s presence.", ch, NULL, NULL, TO_ROOM);
			act_char("You cloak your presence.", ch);
		}
	}
}

DO_FUN(do_holylight, ch, argument)
{
	CHAR_DATA *vch = GET_ORIGINAL(ch);

	if (IS_NPC(vch))
		return;

	TOGGLE_BIT(PC(vch)->plr_flags, PLR_HOLYLIGHT);
	act_puts(IS_SET(PC(vch)->plr_flags, PLR_HOLYLIGHT) ?
		     "Holy light mode on." : "Holy light mode off",
		 ch, NULL, NULL, TO_CHAR, POS_DEAD);
}

/* prefix command: it will put the string typed on each line typed */

DO_FUN(do_prefix, ch, argument)
{
	DESCRIPTOR_DATA *d;

	if ((d = ch->desc) == NULL)
		return;

	if (argument[0] == '\0') {
		if (d->dvdata->prefix[0] == '\0') {
			act_char("You have no prefix to clear.", ch);
			return;
		}

		free_string(d->dvdata->prefix);
		d->dvdata->prefix = str_empty;
		act_char("Prefix removed.", ch);
		return;
	}

	free_string(d->dvdata->prefix);
	d->dvdata->prefix = str_dup(argument);
	act_puts("Prefix set to '$t'.", ch, argument, NULL,
		 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
}

DO_FUN(do_advance, ch, argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	bool loaded = FALSE;
	bool altered = FALSE;
	int level;

	argument = one_argument(argument, arg1, sizeof(arg1));
	argument = one_argument(argument, arg2, sizeof(arg2));

	if (arg1[0] == '\0' || arg2[0] == '\0' || !is_number(arg2)) {
		do_help(ch, "'WIZ ADVANCE'");
		return;
	}

	if ((victim = get_char_world(ch, arg1)) == NULL) {
		if ((victim = char_load(arg1, LOAD_F_NOCREATE)) == NULL) {
			act_char("They aren't here.", ch);
			return;
		}
		loaded = TRUE;
	} else if (IS_NPC(victim)) {
		act_char("Not on NPC's.", ch);
		return;
	}

	if ((level = atoi(arg2)) < 1 || level > MAX_LEVEL) {
		act_puts("Level must be in range 1..$j.",
			 ch, (const void *) MAX_LEVEL, NULL, TO_CHAR, POS_DEAD);
		goto cleanup;
	}

	if (level > ch->level) {
		act_char("Limited to your level.", ch);
		goto cleanup;
	}

	if (!IS_TRUSTED(ch, trust_level(victim))) {
		act_char("You are not allowed to do that.", ch);
		goto cleanup;
	}

	advance(victim, level);
	altered = TRUE;

cleanup:
	if (altered)
		char_save(victim, loaded ? SAVE_F_PSCAN : 0);
	if (loaded)
		char_nuke(victim);
}

DO_FUN(do_mset, ch, argument)
{
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	char arg3 [MAX_INPUT_LENGTH];
	char arg4 [MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int value, val2;
	int stat;
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
			act_char("They aren't here.", ch);
			return;
		}
		loaded = TRUE;
	}

	if (!IS_TRUSTED(ch, trust_level(victim))) {
		act_char("You failed.", ch);
		goto cleanup;
	}

	/*
	 * Snarf the value (which need not be numeric).
	 */
	value = is_number(arg3) ? atoi(arg3) : -1;
	val2  = is_number(arg4) ? atoi(arg4) : -1;

	/*
	 * Set something.
	 */
	if (!str_cmp(arg2, "trouble")) {
		if (IS_NPC(victim)) {
			act_char("Not on NPC's.", ch);
			goto cleanup;
		}

		if (value == -1 || val2 == -1) {
			act_char("Usage: set char <name> trouble <vnum> <value>", ch); // notrans
			goto cleanup;
		}

		qtrouble_set(victim, value, val2+1);
		act_char("Ok.", ch);
		altered = TRUE;
		goto cleanup;
	}

	if (!str_cmp(arg2, "security"))	{ /* OLC */
		if (IS_NPC(ch)) {
			act_char("Si, claro.", ch);		// notrans
			goto cleanup;
		}

		if (IS_NPC(victim)) {
			act_char("Not on NPC's.", ch);
			goto cleanup;
		}

		if ((value > PC(ch)->security || value < 0)
		&& ch->level != MAX_LEVEL) {
			if (PC(ch)->security != 0) {
				act_puts("Valid security is 0-$j.",
					 ch, (const void *) PC(ch)->security,
					 NULL, TO_CHAR, POS_DEAD);
			} else
				act_char("Valid security is 0 only.", ch);
			goto cleanup;
		}
		PC(victim)->security = value;
		altered = TRUE;
		goto cleanup;
	}

	if ((stat = flag_svalue(stat_aliases, arg2)) >= 0) {
		int max_value = get_max_train(victim, stat);

		if (value < 3 || value > max_value) {
			act_puts("$T range is 3..$j.",
				 ch, (const void *) max_value,
				 flag_string(stat_names, stat),
				 TO_CHAR, POS_DEAD);
			goto cleanup;
		}

		victim->perm_stat[stat] = value;
		altered = TRUE;
		goto cleanup;
	}

	if (!str_cmp(arg2, "qpoints")) {
		if (value == -1)
			value = 0;

		if (IS_NPC(victim)) {
			act_char("Not on NPC's.", ch);
			goto cleanup;
		}

		PC(victim)->questpoints = value;
		altered = TRUE;
		goto cleanup;
	}

	if (!str_cmp(arg2, "qtime")) {
		if (IS_NPC(victim)) {
			act_char("Not on NPC's.", ch);
			goto cleanup;
		}

		if (value == -1)
			value = 30;
		PC(victim)->questtime = value;
		altered = TRUE;
		goto cleanup;
	}

	if (!str_prefix(arg2, "sex")) {
		int sex;

		if (IS_NPC(victim)) {
			sex = flag_value(gender_table, arg4);
			if (sex < 0) {
				act_char("Valid values are:", ch);
				show_flags(ch, gender_table);
				goto cleanup;
			}
			mlstr_edit(&victim->gender, argument);
			goto cleanup;
		}

		sex = flag_value(gender_table, arg3);
		if (sex != SEX_MALE
		&&  sex != SEX_FEMALE) {
			act_char("Sex should be one of 'male', 'female'.", ch);
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
			act_char("Mobiles have no class.", ch);
			goto cleanup;
		}

		if ((cl = class_search(arg3)) == NULL) {
			BUFFER *output = buf_new(0);
			buf_append(output, "Possible classes are:\n");
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
			BUFFER *output = buf_new(0);
			buf_append(output, "Valid clan names are: ");
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
				clan_update_lists(clo->name, victim, TRUE);
				clan_save(clo->name);
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
			act_char("Ok.", ch);
			goto cleanup;
		}

		if (!same_clan) {
			PC(victim)->clan_status = CLAN_COMMONER;
			if (!IS_CLAN(cl->name, "none")) {
				name_add(&cl->member_list, victim->name,
					 NULL, NULL);
				clan_save(cl->name);
			}

			spec_update(victim);
			update_skills(victim);
		}

		if (cl->mark_vnum != 0
		&&  (mark = create_obj(cl->mark_vnum, 0)) != NULL) {
			obj_to_char(mark, victim);
			equip_char(victim, mark, WEAR_CLANMARK);
		}

		act_char("Ok.", ch);
		altered = TRUE;
		goto cleanup;
	}

	if (!str_prefix(arg2, "level")) {
		if (!IS_NPC(victim)) {
			act_char("Not on PC's.", ch);
			goto cleanup;
		}

		if (value < 0 || value > 100) {
			act_char("Level range is 0 to 100.", ch);
			goto cleanup;
		}
		victim->level = value;
		altered = TRUE;
		goto cleanup;
	}

	if (!str_prefix(arg2, "gold")) {
		victim->gold = value;
		altered = TRUE;
		goto cleanup;
	}

	if (!str_prefix(arg2, "hp")) {
		int delta = value - victim->perm_hit;

		if (victim->perm_hit + delta < 1
		||  victim->perm_hit + delta > 30000) {
			act_char("perm_hit will be out of range 1..30,000.", ch);
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
			act_char("perm_mana will be out of range 1..60,000.", ch);
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
			act_char("perm_move will be out of range 1..60,000.", ch);
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
			act_char("Not on NPC's.", ch);
			goto cleanup;
		}

		if (value < 0 || value > 250) {
			act_char("Practice range is 0 to 250 sessions.", ch);
			goto cleanup;
		}
		PC(victim)->practice = value;
		altered = TRUE;
		goto cleanup;
	}

	if (!str_prefix(arg2, "train")) {
		if (IS_NPC(victim)) {
			act_char("Not on NPC's.", ch);
			goto cleanup;
		}

		if (value < 0 || value > 50) {
			act_char("Training session range is 0 to 50 sessions.", ch);
			goto cleanup;
		}
		PC(victim)->train = value;
		altered = TRUE;
		goto cleanup;
	}

	if (!str_prefix(arg2, "align")) {
		if (value < -1000 || value > 1000) {
			act_char("Alignment range is -1000 to 1000.", ch);
			goto cleanup;
		}
		victim->alignment = value;
		act_char("Remember to check their hometown.", ch);
		altered = TRUE;
		goto cleanup;
	}

	if (!str_prefix(arg2, "ethos")) {
		int ethos;

		if (IS_NPC(victim)) {
			act_char("Mobiles don't have an ethos.", ch);
			goto cleanup;
		}

		ethos = flag_value(ethos_table, arg3);
		if (ethos < 0) {
			act_char("%s: Unknown ethos.", ch);
			act_char("Valid ethos types are:", ch);
			show_flags(ch, ethos_table);
			goto cleanup;
		}

		victim->ethos = ethos;
		altered = TRUE;
		goto cleanup;
	}

	if (!str_prefix(arg2, "hometown")) {
		if (IS_NPC(victim)) {
			act_char("Mobiles don't have hometowns.", ch);
			goto cleanup;
		}

		if ((value = htn_lookup(arg3)) == -1) {
			act_char("No such hometown.", ch);
			goto cleanup;
		}
	    
		PC(victim)->hometown = value;
		altered = TRUE;
		goto cleanup;
	}

	if (!str_prefix(arg2, "thirst")) {
		if (IS_NPC(victim)) {
			act_char("Not on NPC's.", ch);
			goto cleanup;
		}

		if (value < -1 || value > 100) {
			act_char("Thirst range is -1 to 100.", ch);
			goto cleanup;
		}

		PC(victim)->condition[COND_THIRST] = value;
		altered = TRUE;
		goto cleanup;
	}

	if (!str_prefix(arg2, "drunk")) {
		if (IS_NPC(victim)) {
			act_char("Not on NPC's.", ch);
			goto cleanup;
		}

		if (value < -1 || value > 100) {
			act_char("Drunk range is -1 to 100.", ch);
			goto cleanup;
		}

		PC(victim)->condition[COND_DRUNK] = value;
		altered = TRUE;
		goto cleanup;
	}

	if (!str_prefix(arg2, "full")) {
		if (IS_NPC(victim)) {
			act_char("Not on NPC's.", ch);
			goto cleanup;
		}

		if (value < -1 || value > 100) {
			act_char("Full range is -1 to 100.", ch);
			goto cleanup;
		}

		PC(victim)->condition[COND_FULL] = value;
		altered = TRUE;
		goto cleanup;
	}

	if (!str_prefix(arg2, "hunger")) {
		if (IS_NPC(victim)) {
			act_char("Not on NPC's.", ch);
			goto cleanup;
		}

		if (value < -1 || value > 100) {
			act_char("Hunger range is -1 to 100.", ch);
			goto cleanup;
		}

		PC(victim)->condition[COND_HUNGER] = value;
		altered = TRUE;
		goto cleanup;
	}

	if (!str_prefix(arg2, "bloodlust")) {
		if (IS_NPC(victim)) {
			act_char("Not on NPC's.", ch);
			goto cleanup;
		}

		if (value < -1 || value > 100) {
			act_char("Full range is -1 to 100.", ch);
			goto cleanup;
		}

		PC(victim)->condition[COND_BLOODLUST] = value;
		altered = TRUE;
		goto cleanup;
	}

	if (!str_prefix(arg2, "desire")) {
		if (IS_NPC(victim)) {
			act_char("Not on NPC's.", ch);
			goto cleanup;
		}

		if (value < -1 || value > 100) {
			act_char("Desire range is -1 to 100.", ch);
			goto cleanup;
		}

		PC(victim)->condition[COND_DESIRE] = value;
		altered = TRUE;
		goto cleanup;
	}

	if (!str_prefix(arg2, "race")) {
		race_t *r;

		if (IS_NPC(victim)) {
			act_char("Not on NPC's.", ch);
			goto cleanup;
		}

		if ((r = race_search(arg3)) == NULL) {
			act_char("That is not a valid race.", ch);
			goto cleanup;
		}

		if (!IS_NPC(victim) && !r->race_pcdata) {
			act_char("That is not a valid player race.", ch);
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
			act_char("Not on NPC's.", ch);
			goto cleanup;
		}
		REMOVE_BIT(PC(victim)->plr_flags, PLR_GHOST);
		act_char("Ok.", ch);
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

DO_FUN(do_smite, ch, argument)
{
	CHAR_DATA *victim;

	if (argument[0] == '\0') {
		act_char("You are so frustrated you smite yourself!  OWW!", ch);// notrans
		return;
	}

	if ((victim = get_char_world(ch, argument)) == NULL) {
		act_char("You'll have to smite them some other day.", ch);
		return;
	}

	if (IS_NPC(victim)) {
		act_char("That poor mob never did anything to you.", ch);
		return;
	}

	if (!IS_TRUSTED(ch, trust_level(victim))) {
		act_char("How dare you!", ch);
		return;
	}

	if (victim->position < POS_SLEEPING) {
		act_char("Take pity on the poor thing.", ch);
		return;
	}

	act("A bolt comes down out of the heavens and smites you!",
	    victim, NULL, ch, TO_CHAR);
	act("You reach down and smite $n!", victim, NULL, ch, TO_VICT);
	act("A bolt from the heavens smites $n!", victim, NULL, ch, TO_NOTVICT);
	victim->hit = 1;
	victim->mana = 0;
	victim->move = 0;
}

DO_FUN(do_popularity, ch, argument)
{
	BUFFER *output;
	AREA_DATA *area;
	int i;

	output = buf_new(GET_LANG(ch));
	buf_append(output, "Area popularity statistics (in char * ticks)\n");

	for (area = area_first,i=0; area != NULL; area = area->next,i++) {
		if (i % 2 == 0) 
			buf_append(output, "\n");
		buf_printf(output, BUF_END, "%-20s %-8u       ",// notrans
			   area->name, area->count);
	}
	buf_append(output, "\n\n");				// notrans
	page_to_char(buf_string(output), ch);
	buf_free(output);
}

DO_FUN(do_title, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	bool loaded = FALSE;
	bool altered = FALSE;

	if (IS_NPC(ch))
		return;

	if (!IS_IMMORTAL(ch)
	&&  (clan_lookup(ch->clan) == NULL ||
	     PC(ch)->clan_status != CLAN_LEADER)) {
		act_char("Huh?", ch);
		return;
	}

	argument = one_argument(argument, arg, sizeof(arg));
	if (argument[0] == '\0') {
		do_help(ch, "'TITLE'");
		return;
	}

	if ((victim = get_char_world(ch, arg)) == NULL) {
		if ((victim = char_load(arg, LOAD_F_NOCREATE)) == NULL) {
			act_char("They aren't here.", ch);
			return;
		}
		loaded = TRUE;
	} else if (IS_NPC(victim)) {
		act_char("Not on NPC's.", ch);
		return;
	}

	if (IS_IMMORTAL(victim)) {
		if (!IS_TRUSTED(ch, trust_level(victim))) {
			act_char("You failed.", ch);
			goto cleanup;
		}
	} else if (!IS_IMMORTAL(ch) && !IS_CLAN(ch->clan, victim->clan)) {
		act_char("You failed.", ch);
		goto cleanup;
	}

	set_title(victim, argument);
	altered = TRUE;

	act("$n grants $N a new title!", ch, NULL, victim, TO_NOTVICT);
	act("$n grants you a new title!", ch, NULL, victim, TO_VICT);
	act("You grant $N a new title!", ch, NULL, victim, TO_CHAR);

cleanup:
	if (altered)
		char_save(victim, loaded ? SAVE_F_PSCAN : 0);
	if (loaded)
		char_nuke(victim);
}

/*
 * .gz files are checked too, just in case.
 */

DO_FUN(do_rename, ch, argument)
{
	char old_name[MAX_INPUT_LENGTH], 
	     new_name[MAX_INPUT_LENGTH];
	char strsave[PATH_MAX];
	char *file_name;

	CHAR_DATA *victim;
	bool loaded = FALSE;
	bool altered = FALSE;
	OBJ_DATA *obj;
	clan_t *clan;

	argument = first_arg(argument, old_name, sizeof(old_name), FALSE); 
		   first_arg(argument, new_name, sizeof(new_name), FALSE);

	if (IS_NULLSTR(old_name) || IS_NULLSTR(new_name)) {
		do_help(ch, "'WIZ RENAME'");
		return;
	}

	if ((victim = get_char_world(ch, old_name)) == NULL) {
		if ((victim = char_load(old_name, LOAD_F_NOCREATE)) == NULL) {
			act_char("They aren't here.", ch);
			return;
		}
		loaded = TRUE;
	} else if (IS_NPC(victim)) {
		act_char("You cannot use Rename on NPCs.", ch);
		return;
	}

	if (!IS_TRUSTED(ch, trust_level(victim))) {
		act_char("You failed.", ch);
		goto cleanup;
	}

	if (!str_cscmp(new_name, victim->name)) {
		act_char("Old and new names are the same.", ch);
		goto cleanup;
	}

	if (!pc_name_ok(new_name)) {
		act_char("The new name is illegal.", ch);
		goto cleanup;
	}

/* delete old pfile */
	if (str_cmp(new_name, victim->name)) {
		DESCRIPTOR_DATA *d;

		for (d = descriptor_list; d; d = d->next)
			if (d->character
			&&  !str_cmp(d->character->name, new_name)) {
				act_char("A player with the name you specified already exists!", ch);
				goto cleanup;
			}

		/* check pfile */
		file_name = capitalize(new_name);
		if (dfexist(PLAYER_PATH, file_name)) {
			act_char("A player with that name already exists!", ch);
			goto cleanup;		
		}

		/* check .gz pfile */
		snprintf(strsave, sizeof(strsave),
			 "%s.gz", file_name);			// notrans
		if (dfexist(PLAYER_PATH, strsave)) {
			act_char("A player with that name already exists in a compressed file!", ch);
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
			clan_save(clan->name);
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
	altered = TRUE;
		
	act_char("Character renamed.", ch);
	act_puts("$n has renamed you to $N!",
		 ch, NULL, victim, TO_VICT, POS_DEAD);

cleanup:
	/*
	 * NOTE: Players who are level 1 do NOT get saved under a new name 
	 */

	if (altered)
		char_save(victim, loaded ? SAVE_F_PSCAN : 0);
	if (loaded) 
		char_nuke(victim);
} 

DO_FUN(do_wizpass, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	const char *pwdnew;
	bool loaded = FALSE;
	bool altered = FALSE;

	if (argument[0] == '\0') {
		do_help(ch, "'WIZ WIZPASS'");
		return;
	}

	argument = one_argument(argument, arg, sizeof(arg));
		   first_arg(argument, arg2, sizeof(arg2), FALSE);

	if ((victim = get_char_world(ch, arg)) == NULL) {
		if ((victim = char_load(arg, LOAD_F_NOCREATE)) == NULL) {
			act_char("They aren't here.", ch);
			return;
		}
		loaded = TRUE;
	} else if (IS_NPC(victim)) {
		act_char("Not on NPC's.", ch);
		return;
	}
	 
	if (!IS_TRUSTED(ch, trust_level(victim))) {
		act_char("You failed.", ch);
		goto cleanup;
	}

	if (arg2[0] == '\0')
		pwdnew = str_empty;
	else {
		if (strlen(arg2) < 5) {
			act_char("New password must be at least five characters long.", ch);
			goto cleanup;
		}

		/*
		 * No tilde allowed because of player file format.
		 */
		pwdnew = crypt(arg2, victim->name);
		if (strchr(pwdnew, '~') != NULL) {
			act_char("New password not acceptable, try again.", ch);
			goto cleanup;
		}
	}

	free_string(PC(victim)->pwd);
	PC(victim)->pwd = str_dup(pwdnew);
	altered = TRUE;

	if (ch == victim) {
		act_puts("You set your password to '$t'.",
			 ch, arg2, victim, TO_CHAR, POS_DEAD);
	} else {
		act_puts("$t: password changed to '$T'.",
			 ch, victim->name, arg2,
			 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
		act_puts("$N sets your password to '$t'.",
			 victim, arg2, ch, TO_CHAR, POS_DEAD);
	}

cleanup:
	if (altered)
		char_save(victim, loaded ? SAVE_F_PSCAN : 0);
	if (loaded) 
		char_nuke(victim);
}
   
DO_FUN(do_noaffect, ch, argument)
{
	AFFECT_DATA *paf,*paf_next;
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	bool loaded = FALSE;
	bool altered = FALSE;

	if (!IS_IMMORTAL(ch))
		return;

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		do_help(ch, "'WIZ NOAFFECT'");
		return;
	}

	if ((victim = get_char_world(ch, arg)) == NULL) {
		if ((victim = char_load(arg, LOAD_F_NOCREATE)) == NULL) {
			act_char("They aren't here.", ch);
			return;
		}
		loaded = TRUE;
	}
	 
	if (!IS_TRUSTED(ch, trust_level(victim))) {
		act_char("You failed.", ch);
		goto cleanup;
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
	altered = TRUE;

cleanup:
	if (altered)
		char_save(victim, loaded ? SAVE_F_PSCAN : 0);
	if (loaded) 
		char_nuke(victim);
}

DO_FUN(do_affrooms, ch, argument)
{
	ROOM_INDEX_DATA *room;
	CHAR_DATA	*rch;
	OBJ_DATA	*obj;
	BUFFER		*buf;

	AFFECT_DATA *af;
	int count = 0;

	buf = buf_new(GET_LANG(ch));

	if (!top_affected_room)
		buf_append(buf, "No affected rooms.\n");	// notrans
	else
		buf_append(buf, "Affected rooms:\n");		// notrans

	for (room = top_affected_room; room ; room = room->aff_next)
		for (af = room->affected; af; af = af->next) {
			count++;
			buf_printf(buf, BUF_END,
				    "%d) [Vnum: %5d] "		// notrans
				    "spell '{c%s{x', owner: %s, level {c%d{x " // notrans
				    "for {c%d{x hours.\n",	// notrans
				    count,
				    room->vnum,
				    af->type,
				    mlstr_mval(&af->owner->short_descr),
				    af->level,
				    af->duration);
		}

	count = 0;
	if (!top_affected_char)
		buf_append(buf, "No characters under owned affects.\n"); // notrans
	else
		buf_append(buf, "Characters under owned affects:\n"); // notrans

	for (rch = top_affected_char; rch ; rch = rch->aff_next)
		for (af = rch->affected; af; af = af->next) {
			if (!af->owner)
				continue;
			count++;
			buf_printf(buf, BUF_END,
				    "%d) [%s] "			// notrans
				    "spell '{c%s{x', owner: %s, level {c%d{x " // notrans
				    "for {c%d{x hours.\n",	// notrans
				    count,
				    mlstr_mval(&rch->short_descr),
				    af->type,
				    mlstr_mval(&af->owner->short_descr),
				    af->level,
				    af->duration);
		}

	count = 0;
	if (!top_affected_obj)
		buf_append(buf, "No objects under owned affects.\n"); // notrans
	else
		buf_append(buf, "Objects under owned affects:\n"); // notrans

	for (obj = top_affected_obj; obj ; obj = obj->aff_next)
		for (af = obj->affected; af; af = af->next) {
			count++;
			buf_printf(buf, BUF_END,
				    "%d) [Vnum: %5d] "		// notrans
				    "spell '{c%s{x', owner: %s, level {c%d{x " // notrans
				    "for {c%d{x hours.\n",	// notrans
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

DO_FUN(do_grant, ch, argument)
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
			act_char("They aren't here.", ch);
			return;
		}
		loaded = TRUE;
	} else if (IS_NPC(victim)) {
		act_char("Not on NPC's.", ch);
		return;
	}

	if (!IS_TRUSTED(ch, trust_level(victim))) {
		act_char("You failed.", ch);
		goto cleanup;
	}

	if (arg2[0] == '\0') {
		act_puts("Granted commands for $t: [$T]",
			 ch, victim->name, PC(victim)->granted,
			 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
		goto cleanup;
	}

	if (is_number(arg2)) {
		size_t i;
		int lev = atoi(arg2);

		if (lev < LEVEL_IMMORTAL) {
			act_puts("grant: granted level must be at least $j",
				 ch, (const void *) LEVEL_IMMORTAL, NULL,
				 TO_CHAR, POS_DEAD);
			goto cleanup;
		}

		if (lev > trust_level(ch)) {
			act_char("grant: granted level cannot be higher than yours.", ch);
			goto cleanup;
		}

		for (i = 0; i < varr_size(&commands); i++) {
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
			act_puts("$t: command not found.", ch, arg2, NULL,
				 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
			continue;
		}

		if (cmd) {
			CHAR_DATA *vch = GET_ORIGINAL(ch);

			if (cmd->min_level < LEVEL_IMMORTAL) {
				act_puts("$t: not a wizard command.",
					 ch, arg2, NULL,
					 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE,
					 POS_DEAD);
				continue;
			}

			if (cmd->min_level > trust_level(ch)
			&&  (IS_NPC(vch) ||
			     !is_name(cmd->name, PC(vch)->granted))) {
				act_char("grant: cmd min level cannot be higher than yours.", ch);
				goto cleanup;
			}
		}

		altered = TRUE;
		name_toggle(&PC(victim)->granted, arg2, ch, "grant");
	}

cleanup:
	if (altered)
		char_save(victim, loaded ? SAVE_F_PSCAN : 0);
	if (loaded)
		char_nuke(victim);
}

DO_FUN(do_qtarget, ch, argument)
{
	int low, high;
	char arg[MAX_INPUT_LENGTH];
	AFFECT_DATA *paf;
	CHAR_DATA *vch;

	argument = one_argument(argument, arg, sizeof(arg));
	low = atoi(arg);
	argument = one_argument(argument, arg, sizeof(arg));
	high = atoi(arg);
	if ((vch = get_char_here(ch, argument)) == NULL) {
		act_char("They are not here.", ch);
		return;
	}

	if (!IS_NPC(vch)) {
		act_char("Not on PC's.", ch);
		return;
	}

	paf = aff_new(TO_AFFECTS, "qtarget");
	paf->level	= low;
	paf->duration	= -1;
	paf->modifier	= high;
	paf->bitvector	= AFF_QUESTTARGET;
	affect_to_char(vch, paf);
	aff_free(paf);
}

DO_FUN(do_slay, ch, argument)
{
	CHAR_DATA *victim;
	char arg[MAX_INPUT_LENGTH];

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		do_help(ch, "'WIZ SLAY'");
		return;
	}

	if ((victim = get_char_here(ch, arg)) == NULL) {
		act_char("They aren't here.", ch);
		return;
	}

	if (ch == victim) {
		act_char("Suicide is a mortal sin.", ch);
		return;
	}

	if (IS_IMMORTAL(victim)) {
		act_char("You failed.", ch);
		return;
	}

	act("You slay $M in cold blood!", ch, NULL, victim, TO_CHAR);
	act("$n slays you in cold blood!", ch, NULL, victim, TO_VICT);
	act("$n slays $N in cold blood!", ch, NULL, victim, TO_NOTVICT);
	raw_kill(ch, victim);
}

DO_FUN(do_ban, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];

	argument = one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		BUFFER *output;

		output = buf_new(GET_LANG(ch));
		dump_bans(output);
		page_to_char(buf_string(output), ch);
		buf_free(output);

		return;
	}

	if (!str_prefix(arg, "add"))
		ban_add(ch, argument);
	else if (!str_prefix(arg, "delete"))
		ban_delete(ch, argument);
	else
		dofun("help", ch, "'WIZ BAN'");
}

DO_FUN(do_memory, ch, argument)
{
	BUFFER *buf;

	buf = buf_new(0);
	buf_printf(buf, BUF_END, "Affects  : %d (%d bytes)\n",	// notrans
		   affect_count, affect_count * sizeof(AFFECT_DATA));
	buf_printf(buf, BUF_END, "Areas    : %d (%d bytes)\n",	// notrans
		   area_count, area_count * sizeof(AREA_DATA));
	buf_printf(buf, BUF_END, "ExDes    : %d (%d bytes)\n",	// notrans
		   ed_count, ed_count * sizeof(ED_DATA));
	buf_printf(buf, BUF_END, "Exits    : %d (%d bytes)\n",	// notrans
		   exit_count, exit_count * sizeof(EXIT_DATA));
	buf_printf(buf, BUF_END, "Helps    : %d (%d bytes)\n",	// notrans
		   help_count, help_count * sizeof(HELP_DATA));
	buf_printf(buf, BUF_END, "Socials  : %d (%d bytes)\n",	// notrans
		   socials.nused, socials.nused * sizeof(social_t));
	buf_printf(buf, BUF_END, "Mob idx  : %d (%d bytes)\n",  // notrans
		   mob_index_count, mob_index_count * sizeof(MOB_INDEX_DATA));
	buf_printf(buf, BUF_END, "Mobs     : %d (%d (%d) bytes), "  // notrans
			"%d free (%d (%d) bytes), max vnum %d\n", // notrans
		    npc_count,
		    npc_count * (sizeof(CHAR_DATA) + sizeof(NPC_DATA)),
		    npc_count * (sizeof(CHAR_DATA) + sizeof(NPC_DATA) +
				 sizeof(memchunk_t)),
		    npc_free_count,
		    npc_free_count * (sizeof(CHAR_DATA) + sizeof(NPC_DATA)),
		    npc_free_count * (sizeof(CHAR_DATA) + sizeof(NPC_DATA) +
				      sizeof(memchunk_t)),
		    top_vnum_mob);
	buf_printf(buf, BUF_END, "Players  : %d (%d (%d) bytes), " // notrans
			         "%d free (%d (%d) bytes)\n",	// notrans
		    pc_count,
		    pc_count * (sizeof(CHAR_DATA) + sizeof(PC_DATA)),
		    pc_count * (sizeof(CHAR_DATA) + sizeof(PC_DATA) +
				 sizeof(memchunk_t)),
		    pc_free_count,
		    pc_free_count * (sizeof(CHAR_DATA) + sizeof(PC_DATA)),
		    pc_free_count * (sizeof(CHAR_DATA) + sizeof(PC_DATA) +
				      sizeof(memchunk_t)));
	buf_printf(buf, BUF_END,
		   "Obj idx  : %d (%d bytes), max vnum %d\n",	// notrans
		    obj_index_count, obj_index_count * sizeof(OBJ_INDEX_DATA),
		    top_vnum_obj);
	buf_printf(buf, BUF_END,
		   "Objs     : %d (%d (%d) bytes, %d free)\n",	// notrans
		    obj_count,
		    obj_count * sizeof(OBJ_DATA),
		    obj_count * (sizeof(OBJ_DATA) + sizeof(memchunk_t)),
		    obj_free_count);
	buf_printf(buf, BUF_END, "Resets   : %d (%d bytes)\n",	// notrans
		   reset_count, reset_count * sizeof(RESET_DATA));
	buf_printf(buf, BUF_END,
		   "Rooms    : %d (%d (%d) bytes, max vnum %d)\n", // notrans
		   room_count,
		   room_count * sizeof(ROOM_INDEX_DATA),
		   room_count * (sizeof(ROOM_INDEX_DATA) + sizeof(memchunk_t)),
		   top_vnum_room);
	buf_printf(buf, BUF_END, "Shops    : %d (%d bytes)\n",	// notrans
		   shop_count, shop_count * sizeof(SHOP_DATA));
	buf_printf(buf, BUF_END, "Buffers  : %d (%d bytes)\n",	// notrans
					nAllocBuf, sAllocBuf);
	buf_printf(buf, BUF_END,
#if STR_ALLOC_MEM
		    "strings  : %d (%d allocated, %d bytes)\n",  // notrans
#else
		    "strings  : %d (%d allocated)\n",		// notrans
#endif
		    str_count, str_real_count,
#if STR_ALLOC_MEM
		    str_alloc_mem
#endif
		    );
	buf_printf(buf, BUF_END,
		   "dvdata   : %d (%d allocated, %d bytes)\n",	// notrans
		    dvdata_count, dvdata_real_count,
		    dvdata_real_count * sizeof(dvdata_t));
	page_to_char(buf_string(buf), ch);
	buf_free(buf);
}

DO_FUN(do_dump, ch, argument)
{
	int i;
	FILE *fp;

	if ((fp = dfopen(TMP_PATH, "mem.dmp", "w")) == NULL)	// notrans
		return;

	/* areas */
	fprintf(fp, "Areas      %d (%d bytes)\n",		// notrans
		area_count, area_count * sizeof(AREA_DATA));
	/* helps */
	fprintf(fp, "Helps      %d (%d bytes)\n",		// notrans
		help_count, help_count * sizeof(HELP_DATA));
	fprintf(fp, "Resets     %d (%d bytes)\n",		// notrans
		reset_count, reset_count * sizeof(RESET_DATA));

	/* mobile prototypes */
	fprintf(fp, "MobProt    %d (%d bytes)\n",		// notrans
		mob_index_count, mob_index_count * sizeof(MOB_INDEX_DATA));
	fprintf(fp, "Mobs       %d (%d bytes), %d free (%d bytes)\n", // notrans
		npc_count,
		npc_count * (sizeof(CHAR_DATA) + sizeof(NPC_DATA)),
		npc_free_count,
		npc_free_count * (sizeof(CHAR_DATA) + sizeof(NPC_DATA)));

	fprintf(fp, "Players    %d (%d bytes), %d free (%d bytes)\n", // notrans
		pc_count,
		pc_count * (sizeof(CHAR_DATA) + sizeof(PC_DATA)),
		pc_free_count,
		pc_free_count * (sizeof(CHAR_DATA) + sizeof(PC_DATA)));
	fprintf(fp, "Descs      %d (%d bytes), %d free (%d bytes)\n", // notrans
		desc_count, desc_count * sizeof(DESCRIPTOR_DATA),
		desc_free_count, desc_free_count * sizeof(DESCRIPTOR_DATA));
	fprintf(fp, "Dvdata     %d (%d bytes)\n",
		dvdata_real_count, dvdata_real_count * sizeof(dvdata_t));

	fprintf(fp, "ObjProt    %d (%d bytes)\n",		// notrans
		obj_index_count, obj_index_count * (sizeof(OBJ_INDEX_DATA)));
	fprintf(fp, "Objs       %d (%d bytes)\n",		// notrans
		obj_count, obj_count * sizeof(OBJ_DATA));

	/* rooms */
	fprintf(fp,"Rooms       %d (%d bytes)\n",		// notrans
		room_count, room_count * (sizeof(ROOM_INDEX_DATA)));
	 /* exits */
	fprintf(fp,"Exits	%d (%d bytes)\n",		// notrans
		exit_count, exit_count * (sizeof(EXIT_DATA)));
	/* shops */
	fprintf(fp,"Shops       %d (%d bytes)\n",		// notrans
		shop_count, shop_count * (sizeof(EXIT_DATA)));

	/* affects */
	fprintf(fp, "Affects    %d (%d bytes)\n",		// notrans
		affect_count, affect_count * (sizeof(AFFECT_DATA)));
	/* extra descriptions */
	fprintf(fp, "Exds       %d (%d bytes)\n",
		ed_count, ed_count * sizeof(ED_DATA));

	fclose(fp);

	/* start printing out mobile data */
	if ((fp = dfopen(TMP_PATH, "mob.dmp", "w")) == NULL)	// notrans
		return;

	fprintf(fp,"\nMobile Analysis\n");			// notrans
	fprintf(fp,  "---------------\n");			// notrans

	for (i = 0; i < MAX_KEY_HASH; i++) {
		MOB_INDEX_DATA *pMobIndex;

		for (pMobIndex = mob_index_hash[i]; pMobIndex != NULL;
		     pMobIndex = pMobIndex->next) {
			fprintf(fp, "#%d %d active %d killed     %s\n", // notrans
				pMobIndex->vnum,
				pMobIndex->count,
				pMobIndex->killed,
				mlstr_mval(&pMobIndex->short_descr));
		}
	}
	fclose(fp);

	/* start printing out object data */
	if ((fp = dfopen(TMP_PATH, "obj.dmp", "w")) == NULL)	// notrans
		return;

	fprintf(fp,"\nObject Analysis\n");			// notrans
	fprintf(fp,  "---------------\n");			// notrans

	for (i = 0; i < MAX_KEY_HASH; i++) {
		OBJ_INDEX_DATA *pObjIndex;

		for (pObjIndex = obj_index_hash[i]; pObjIndex != NULL;
		     pObjIndex = pObjIndex->next) {
			fprintf(fp, "#%-4d %3d active %3d reset      %s\n", // notrans
				pObjIndex->vnum,
				pObjIndex->count,
				pObjIndex->reset_num,
				mlstr_mval(&pObjIndex->short_descr));
		}
	}

	fclose(fp);
}

DO_FUN(do_shapeshift, ch, argument)
{
	char arg[MAX_STRING_LENGTH];
	form_index_t *form;

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		act_char("Shapeshift into what?", ch);
		return;
	}

	if ((form = form_search(arg)) == NULL) {
		act_char("No such form.", ch);
		return;
	}

	shapeshift(ch, arg);
}

#if 0
XXX
/*
 * Displays MOBprogram triggers of a mobile
 *
 * Syntax: mpstat [name]
 */
DO_FUN(do_mpstat, ch, argument)
{
	char arg[MAX_STRING_LENGTH];
	MPTRIG  *mptrig;
	CHAR_DATA *victim;
	NPC_DATA *npc;
	int i;
	BUFFER *buf;

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		act_char("Mpstat whom?", ch);		// notrans
		return;
	}

	if ((victim = get_char_world(ch, arg)) == NULL) {
		act_char("No such creature.", ch);
		return;
	}

	if (!IS_NPC(victim)) {
		act_char("That is not a mobile.", ch);
		return;
	}

	buf = buf_new(0);
	buf_printf(buf, BUF_END, "Mobile #%-6d [%s]\n",		// notrans
		   victim->pMobIndex->vnum, mlstr_mval(&victim->short_descr));

	npc = NPC(victim);
	buf_printf(buf, BUF_END, "Delay   %-6d [%s]\n",		// notrans
		   npc->mprog_delay,
		   npc->mprog_target == NULL ?
			"No target" : npc->mprog_target->name); // notrans

	if (!victim->pMobIndex->mptrig_types)
		buf_append(buf, "[No programs set]");		// notrans
	else {
		i = 0;
		for (mptrig = victim->pMobIndex->mptrig_list; mptrig != NULL;
							mptrig = mptrig->next) {
			buf_printf(buf, BUF_END,
				   "[%2d] Trigger [%-8s] Program [%4d] Phrase [%s]\n", // notrans
				   ++i, flag_string(mptrig_types, mptrig->type),
				   mptrig->vnum, mptrig->phrase);
		}
	}

	page_to_char(buf_string(buf), ch);
	buf_free(buf);
}
#endif

extern int max_rnd_cnt;
extern int rnd_cnt;

DO_FUN(do_maxrnd, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];

	one_argument(argument, arg, sizeof(arg));
	if (is_number(arg)) {
		max_rnd_cnt = atoi(arg);
		rnd_cnt = 0;
	} else if (arg[0] != '\0') {
		act_char("Syntax: maxrnd <max_rnd_cnt>.", ch);
		return;
	}

	act_puts("Current max_rnd_cnt = $j",
		 ch, (const void *) max_rnd_cnt, NULL, TO_CHAR, POS_DEAD);
}

/*--------------------------------------------------------------------
 * static functions
 */

/*
 * assumes !IS_NPC(victim)
 */
static void
advance(CHAR_DATA *victim, int level)
{
	int iLevel;
	int tra;
	int pra;

	tra = PC(victim)->train;
	pra = PC(victim)->practice;
	PC(victim)->plevels = 0;

	/*
	 * Lower level:
	 *   Reset to level 1.
	 *   Then raise again.
	 *   Currently, an imp can lower another imp.
	 *   -- Swiftest
	 */
	if (level <= victim->level) {
		int temp_prac;
		int delta;

		act_char("**** OOOOHHHHHHHHHH  NNNNOOOO ****", victim);
		temp_prac = PC(victim)->practice;
		victim->level = 1;
		PC(victim)->exp	= 0;

		delta = 20 - victim->perm_hit;
		victim->perm_hit += delta;
		victim->max_hit += delta;

		delta = 100 - victim->perm_mana;
		victim->perm_mana += delta;
		victim->max_mana += delta;

		delta = 100 - victim->perm_move;
		victim->perm_move += delta;
		victim->max_move += delta;

		advance_level(victim);
		PC(victim)->practice= temp_prac;
	} else 
		act_char("**** OOOOHHHHHHHHHH  YYYYEEEESSS ****", victim);

	for (iLevel = victim->level; iLevel < level; iLevel++) {
		act_puts("{CYou raise a level!!{x ",
			 victim, NULL, NULL, TO_CHAR | ACT_NOLF, POS_DEAD);
		PC(victim)->exp += exp_to_level(victim);
		victim->level++;
		advance_level(victim);
	}

	update_skills(victim);
	PC(victim)->train	= tra;
	PC(victim)->practice	= pra;
}

DO_FUN(do_modules, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		dofun("help", ch, "'WIZ MODULES'");
		return;
	}

	if (!str_prefix(arg, "reload")
	||  !str_prefix(arg, "load")) {
		module_t *m;
		time_t curr_time;

		one_argument(argument, arg, sizeof(arg));
		if (arg[0] == '\0') {
			do_modules(ch, str_empty);
			return;
		}

		if ((m = mod_lookup(arg)) == NULL) {
			act_puts("$t: unknown module name.",
				 ch, arg, NULL,
				 TO_CHAR | ACT_NOUCASE | ACT_NOTRANS, POS_DEAD);
			return;
		}

		if (m->dlh == NULL) {
			act_puts("$t: module was unloaded and can't be reloaded.",
				 ch, arg, NULL,
				 TO_CHAR | ACT_NOUCASE | ACT_NOTRANS, POS_DEAD);
			return;
		}

		log(LOG_INFO, "do_modules: reloading module '%s'", m->name);
		act_puts("Reloading module '$t'.",
			 ch, m->name, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);

		/*
		 * clear input buffer
		 */
		if (ch->desc != NULL)
			ch->desc->incomm[0] = '\0';

		log_setchar(ch);
		time(&curr_time);
		if (!mod_reload(m, curr_time))
			act_char("Ok.", ch);
		log_unsetchar();
		return;
	}

	if (!str_prefix(arg, "list")
	||  !str_prefix(arg, "status")) {
		size_t i;
		BUFFER *buf;

		if (modules.nused == 0) {
			act_char("No modules found.", ch);
			return;
		}

		buf = buf_new(GET_LANG(ch));
		buf_append(buf, "  Module  Prio          Load time         Deps\n");
		buf_append(buf, "--------- ---- -------------------------- -----------------------------------\n");	// notrans
		for (i = 0; i < varr_size(&modules); i++) {
			module_t *m = VARR_GET(&modules, i);
			buf_printf(buf, BUF_END, "%9s %4d [%24s] %s\n", // notrans
				   m->name,
				   m->mod_prio,
				   m->dlh == NULL ? "module was unloaded" :
				   m->last_reload ? strtime(m->last_reload) :
						    "never",
				   m->mod_deps);
		}

		page_to_char(buf_string(buf), ch);
		buf_free(buf);

		return;
	}

	do_modules(ch, str_empty);
}

DO_FUN(do_shutdown, ch, argument)
{
	bool active;
	char arg[MAX_INPUT_LENGTH];

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		dofun("help", ch, "'WIZ SHUTDOWN'");
		return;
	}

	active = dfexist(TMP_PATH, SHUTDOWN_FILE);

	if (!str_prefix(arg, "status")) {
		act_puts(active ? "Shutdown status: active" :
				  "Shutdown status: inactive",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		return;
	}

	if (!str_prefix(arg, "activate")) {
		if (!active) {
			FILE *fp = dfopen(TMP_PATH, SHUTDOWN_FILE, "w");
			if (!fp) {
				act_puts("Error: $t",
					 ch, strerror(errno), NULL,
					 TO_CHAR | ACT_NOTRANS, POS_DEAD);
				return;
			}
			fclose(fp);
			wiznet("$N has activated shutdown.", ch, NULL, 0, 0, 0);
			act_char("Shutdown activated.", ch);
		} else
			act_char("Shutdown already activated.", ch);
		return;
	}

	if (!str_prefix(arg, "deactivate") || !str_prefix(arg, "cancel")) {
		if (!active)
			act_char("Shutdown already inactive.", ch);
		else {
			if (dunlink(TMP_PATH, SHUTDOWN_FILE) < 0) {
				act_puts("Error: $t",
					 ch, strerror(errno), NULL,
					 TO_CHAR | ACT_NOTRANS, POS_DEAD);
				return;
			}
			wiznet("$N has deactivated shutdown.",
				ch, NULL, 0, 0, 0);
			act_char("Shutdown deactivated.", ch);
		}
		return;
	}

	do_shutdown(ch, str_empty);
}

DO_FUN(do_reboot, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];

	argument = one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		dofun("help", ch, "'WIZ REBOOT'");
		return;
	}

	if (is_name(arg, "cancel")) {
		reboot_counter = -1;
		act_char("Reboot canceled.", ch);
		return;
	}

	if (is_name(arg, "now")) {
		rebooter = 1;
		reboot_mud();
		return;
	}

	if (is_name(arg, "status")) {
		if (reboot_counter == -1)
			act_char("Automatic rebooting is inactive.", ch);
		else {
			act_puts("Reboot in $j $qj{minutes}.",
				 ch, (const void *) reboot_counter, NULL,
				 TO_CHAR, POS_DEAD);
		}
		return;
	}

	if (is_number(arg)) {
		reboot_counter = atoi(arg);
		rebooter = 1;
		act_puts("SoG will reboot in $j $qj{ticks}.",
			 ch, (const void *) reboot_counter, NULL,
			 TO_CHAR, POS_DEAD);
		return;
	}

	do_reboot(ch, "");
}
