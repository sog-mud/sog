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
 * $Id: clan.c,v 1.28 1999-02-09 14:28:14 fjoe Exp $
 */

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if	defined (WIN32)
#	include <compat/compat.h>
#else
#	include <dirent.h>
#endif

#include "merc.h"

varr clans = { sizeof(CLAN_DATA), 4 };

CLAN_DATA *clan_new(void)
{
	CLAN_DATA *clan;

	clan = varr_enew(&clans);
	clan->skills.nsize = sizeof(CLAN_SKILL);
	clan->skills.nstep = 4;

	return clan;
}

void clan_free(CLAN_DATA *clan)
{
	varr_free(&clan->skills);
}

int cn_lookup(const char *name)
{
	int cn;

	if (IS_NULLSTR(name))
		return -1;

	for (cn = 0; cn < clans.nused; cn++)
		if (!str_cmp(name, CLAN(cn)->name))
			return cn;

	return -1;
}

const char *clan_name(int cn)
{
	CLAN_DATA *clan = clan_lookup(cn);
	if (clan)
		return clan->name;
	return "None";
}

void do_petitio(CHAR_DATA *ch, const char *argument)
{
	char_puts("You must enter full command to petition.\n",ch);
}

void do_petition(CHAR_DATA *ch, const char *argument)
{
	bool accept;
	int cn = 0;
	CLAN_DATA *clan = NULL;
	char arg1[MAX_STRING_LENGTH];

	if (IS_NPC(ch))
		return;	

	argument = one_argument(argument, arg1);

	if (IS_NULLSTR(arg1)) {
		if (IS_IMMORTAL(ch)
		||  (ch->clan && (ch->pcdata->clan_status == CLAN_LEADER ||
				  ch->pcdata->clan_status == CLAN_SECOND)))
			char_printf(ch,
				    "Usage: petition %s<accept | reject> "
				    "<char name>\n",
				    IS_IMMORTAL(ch) ? "<clan name> " : str_empty);
		if (IS_IMMORTAL(ch) || !ch->clan)
			char_puts("Usage: petition <clan name>\n", ch);
		return;
	}

	if (IS_IMMORTAL(ch)) {
		cn = cn_lookup(arg1);
		if (cn <= 0) {
			char_printf(ch, "%s: unknown clan\n", arg1);
			do_petition(ch, str_empty);
			return;
		}
		argument = one_argument(argument, arg1);
		if (IS_NULLSTR(arg1)) {
			do_petition(ch, str_empty);
			return;
		}
		clan = CLAN(cn);
	}

	if ((accept = !str_prefix(arg1, "accept"))
	||  !str_prefix(arg1, "reject")) {
		CHAR_DATA *victim;
		char arg2[MAX_STRING_LENGTH];

		if (!IS_IMMORTAL(ch)) {
			if (ch->clan == 0
			||  (clan = clan_lookup(ch->clan)) == NULL) {
				do_petition(ch, str_empty);
				return;
			}
			cn = ch->clan;
		}

		argument = one_argument(argument, arg2);
		if (IS_NULLSTR(arg2)) {
			do_petition(ch, str_empty);
			return;
		}

		if (ch->pcdata->clan_status != CLAN_LEADER
		&&  ch->pcdata->clan_status != CLAN_SECOND
		&&  !IS_IMMORTAL(ch)) {
			char_puts("You don't have enough power to "
				  "accept/reject petitions.\n", ch);
			return;
		}

		if (!(victim = get_char_world(ch, arg2))
		||  IS_NPC(victim)) {
			char_puts("Can't find them.\n", ch);
			return;
		}

		if (accept) {
			if (victim->pcdata->petition != cn) {
				char_puts("They didn't petition.\n", ch);
				return;
			}
			victim->clan = cn;
			victim->pcdata->clan_status = CLAN_COMMONER;
			update_skills(victim);
			char_puts("Greet new member!\n", ch);
			char_printf(victim, "Your petition to %s has been "
				    "accepted.\n",
				    clan->name);
			char_printf(victim, "You are now one of %s!\n",
				    clan->name);
			return;
		}

/* handle 'petition reject' */
		if (victim->clan == cn) {
			if (victim->pcdata->clan_status == CLAN_LEADER
			&&  !IS_IMMORTAL(ch)) {
				char_puts("You don't have enough power "
					  "to do that.\n", ch);
				return;
			}
			victim->clan = CLAN_NONE;
			REMOVE_BIT(victim->pcdata->trust, TRUST_CLAN);
			update_skills(victim);
			char_printf(ch, "They are not a member of %s "
					"anymore.\n", clan->name);
			char_printf(victim, "You are not a member of %s "
					    "anymore.\n", clan->name);
			return;
		}

		if (victim->pcdata->petition == cn) {
			victim->pcdata->petition = CLAN_NONE;
			char_puts("Petition was rejected.\n", ch);
			char_printf(victim, "Your petition to %s was "
				    "rejected.\n",
				    clan->name);
			return;
		}

		char_puts("They didn't petition.\n", ch);
		return;
	}

	if (IS_IMMORTAL(ch)
	||  (ch->clan && (ch->pcdata->clan_status == CLAN_LEADER ||
			  ch->pcdata->clan_status == CLAN_SECOND))) {
		DESCRIPTOR_DATA *d;
		bool found = FALSE;

		if (IS_IMMORTAL(ch)) {
			if ((cn = cn_lookup(arg1)) <= 0) {
				char_puts("No such clan.\n", ch);
				return;
			}
		}
		else
			cn = ch->clan;
			
		for (d = descriptor_list; d; d = d->next) {
			CHAR_DATA *vch = d->original ? d->original :
						       d->character;

			if (!vch
			||  vch->clan
			||  vch->pcdata->petition != cn)
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

	if ((cn = cn_lookup(arg1)) <= 0) {
		char_puts("No such clan.\n", ch);
		return;
	}

	if (ch->pcdata->petition) {
		char_puts("You have already petitioned to another clan.\n", ch);
		return;
	}

	ch->pcdata->petition = cn;
	char_puts("Petition sent.\n", ch);
}

void do_promote(CHAR_DATA *ch, const char *argument)
{
	char arg1[MAX_STRING_LENGTH];
	char arg2[MAX_STRING_LENGTH];
	CHAR_DATA *victim;
	CLAN_DATA *clan;

	if (IS_NPC(ch)
	||  (!IS_IMMORTAL(ch) && ch->pcdata->clan_status != CLAN_LEADER)) {
		char_puts("Huh?\n", ch);
		return;
	}

	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (!*arg1 || !*arg2) {
		char_puts("Usage: promote <char name> <commoner | secondary>\n",
			  ch);
		if (IS_IMMORTAL(ch))
			char_puts("    or: promote <char name> <leader>\n",
				  ch);
		return;
	}

	victim = get_char_world(ch, arg1);
	if (!victim || IS_NPC(victim)) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	if (victim->clan == 0 || (clan = clan_lookup(victim->clan)) == NULL
	||  (victim->clan != ch->clan && !IS_IMMORTAL(ch))) {
		char_puts("They are not an a clan.\n", ch);
		return;
	}

	if (!IS_IMMORTAL(ch) && victim->pcdata->clan_status == CLAN_LEADER) {
		char_puts("You don't have enough power to promote them.\n",
			  ch);
		return;
	}

	if (!str_prefix(arg2, "leader") && IS_IMMORTAL(ch)) {
		victim->pcdata->clan_status = CLAN_LEADER;
		char_puts("Ok.\n", ch);
		char_puts("They are now leader in their clan.\n", ch);
		char_puts("You are now leader in your clan.\n", victim);
		return;
	}

	if (!str_prefix(arg2, "secondary")) {
		if (victim->pcdata->clan_status == CLAN_SECOND) {
			char_puts("They are already second in a clan.\n",
				  ch);
			return;
		}

		victim->pcdata->clan_status = CLAN_SECOND;
		char_puts("They are now second in the clan.\n", ch);
		char_puts("You are now second in the clan.\n", victim);
		return;
	}

	if (!str_prefix(arg2, "commoner")) {
		if (victim->pcdata->clan_status == CLAN_COMMONER) {
			char_puts("They are already commoner in a clan.\n",
				  ch);
			return;
		}

		victim->pcdata->clan_status = CLAN_COMMONER;
		char_puts("They are now commoner in the clan.\n", ch);
		char_puts("You are now commoner in the clan.\n", victim);
		return;
	}

	do_promote(ch, str_empty);
}

char *get_status_alias(int status)
{
	switch (status) {
	case CLAN_COMMONER:
		return "commoner";
	case CLAN_SECOND:
		return "secondary";
	case CLAN_LEADER:
		return "leader";
	}
	return "commoner";
}

void do_clanlist(CHAR_DATA *ch, const char *argument)
{
	char_puts("clanlist: temporarily disabled.\n", ch);
#if 0
	DESCRIPTOR_DATA *d;
	FILE *pfile;
	DIR *dirp;
	char buf[PATH_MAX];
	struct dirent *dp;
	char letter;

	if (IS_NPC(ch))
		return;

	if (ch->clan == CLAN_NONE || ch->pcdata->clan_status == CLAN_COMMONER) {
		char_puts("Huh?\n", ch);
		return;
	}

                                                                                
	if ((dirp = opendir(PLAYER_DIR)) == NULL) {
		bug("Clan_list: unable to open player directory.", 0);
		exit(1);
	}

	for (d = descriptor_list; d; d = d->next)
		if (d->character)
			do_save(d->character, str_empty);

	char_puts("Now listing members of your clan:\n", ch);
	for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
		char *name = NULL;
		int clan = -1, status = -1;

		if (dp->d_namlen < 3)
			continue;

		snprintf(buf, sizeof(buf), "%s%s", PLAYER_DIR, dp->d_name);

		if ((pfile = fopen(buf, "r")) == NULL) {
			bug("Clan_list: Can't open player file.", 0);
			continue;
		}

		for (letter = fread_letter(pfile); letter != EOF;
						letter = fread_letter(pfile)) {
			char *word;

			if (letter == 'N'
			&&  !strcmp(word = fread_word(pfile), "ame")) {
				if (name)
					continue;
				name = fread_string(pfile);
				continue;
			}

			if (letter == 'C'
			&&  !strcmp(word = fread_word(pfile), "lan")) {
				char *p = fread_string(pfile);
				clan = clan_lookup(p);
				free_string(p);
			}
			else if (letter == 'C'
			&&  !strcmp(word, "lanStatus"))
				status = fread_number(pfile);
		}

		if (name && clan == ch->clan)
			char_printf(ch, "%s -- %s\n", name,
				    get_status_alias(status));
		fclose(pfile);
	}
#endif
}
void do_item(CHAR_DATA* ch, const char* argument)
{
	CLAN_DATA* clan;
	OBJ_DATA* in_obj;
	int cn;
	if(ch->clan==0||(clan=clan_lookup(ch->clan))==NULL) {
		char_puts("You are not in clan, you should not worry about your clan item.\n",ch);
		return;
	}

	if (clan->obj_ptr == NULL) {
		char_puts("Your clan do not have an item of power.\n",ch);
		return;
	}

	for (in_obj=clan->obj_ptr; in_obj->in_obj!=NULL; in_obj=in_obj->in_obj);
	if (in_obj->carried_by !=NULL)
		char_printf(ch, "%s is in %s, carried by %s\n",
			mlstr_mval(clan->obj_ptr->short_descr),
			mlstr_mval(in_obj->carried_by->in_room->name),
			PERS(in_obj->carried_by, ch));
	else {
		char_printf(ch, "%s is in %s.\n",
			mlstr_mval(clan->obj_ptr->short_descr),
			mlstr_mval(in_obj->in_room->name));
		for (cn=0; cn < clans.nused; cn++) 
			if (in_obj->in_room->vnum==clan_lookup(cn)->altar_vnum)
			char_printf(ch,"It is altar of %s.\n",
			 clan_lookup(cn)->name);
	}
	return;
}

bool clan_item_ok(int cn)
{
	CLAN_DATA* clan;
	OBJ_DATA* obj;
	int room_in;
	int i;
	if (!(clan=clan_lookup(cn)) || !(clan->obj_ptr)) 
		return TRUE;
	for (obj=clan->obj_ptr; obj->in_obj!=NULL; obj=obj->in_obj);
	if (obj->in_room) 
		room_in=obj->in_room->vnum;
	else 
		return TRUE;
	if (room_in == clan->altar_vnum) return TRUE;
	for (i=0; i < clans.nused; i++)
		if (room_in == clan_lookup(i)->altar_vnum) return FALSE;
	return TRUE;
}
