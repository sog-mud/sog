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
 * $Id: clan.c,v 1.16 1998-09-24 12:30:11 kostik Exp $
 */

#include <sys/syslimits.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include "merc.h"
#include "interp.h"

varr *clans;

int cn_lookup(const char *name)
{
	int cn;

	if (IS_NULLSTR(name))
		return -1;

	for (cn = 0; cn < clans->nused; cn++)
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
	char_puts("You must enter full command to petition.\n\r",ch);
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
		if (ch->clan || IS_IMMORTAL(ch)) 
			char_printf(ch,
				    "Usage: petition %s<accept | reject> "
				    "<char name>\n\r",
				    IS_IMMORTAL(ch) ? "<clan name> " : "");
		if (!IS_IMMORTAL(ch) && ch->pcdata->clan_status != CLAN_LEADER)
			char_puts("Usage: petition <clan name>\n\r", ch);
		return;
	}

	if (IS_IMMORTAL(ch)) {
		cn = cn_lookup(arg1);
		if (cn <= 0) {
			char_printf(ch, "%s: unknown clan\n\r", arg1);
			do_petition(ch, "");
			return;
		}
		argument = one_argument(argument, arg1);
		if (IS_NULLSTR(arg1)) {
			do_petition(ch, "");
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
				do_petition(ch, "");
				return;
			}
			cn = ch->clan;
		}

		argument = one_argument(argument, arg2);
		if (IS_NULLSTR(arg2)) {
			do_petition(ch, "");
			return;
		}

		if (ch->pcdata->clan_status != CLAN_LEADER
		&&  ch->pcdata->clan_status != CLAN_SECOND
		&&  !IS_IMMORTAL(ch)) {
			char_puts("You don't have enough power to "
				  "accept/reject petitions.\n\r", ch);
			return;
		}

		if (!(victim = get_char_world(ch, arg2))
		||  IS_NPC(victim)) {
			char_puts("Can't find them.\n\r", ch);
			return;
		}

		if (accept) {
			if (victim->pcdata->petition != cn) {
				char_puts("They didn't petition.\n\r", ch);
				return;
			}
			victim->clan = cn;
			victim->pcdata->petition = CLAN_NONE;
			victim->pcdata->clan_status = CLAN_COMMON;
			update_skills(victim);
			char_puts("Ok.\n\r", ch);
			char_puts("Greet new member!\n\r", ch);
			char_printf(victim, "Your petition to %s has been "
				    "accepted.\n\r",
				    clan->name);
			char_printf(victim, "You are now one of %s!\n\r",
				    clan->name);
			return;
		}

/* handle 'petition reject' */
		if (victim->clan == cn) {
			if (victim->pcdata->clan_status == CLAN_LEADER
			&&  !IS_IMMORTAL(ch)) {
				char_puts("You don't have enough power "
					  "to do that.\n\r", ch);
				return;
			}
			victim->clan = CLAN_NONE;
			victim->pcdata->petition = CLAN_NONE;
			update_skills(victim);
			char_puts("Ok.\n\r", ch);
			char_printf(ch, "They are not a member of %s "
					"anymore.\n\r", clan->name);
			char_printf(ch, "You are not a member of %s "
					"anymore.\n\r", clan->name);
			return;
		}

		if (victim->pcdata->petition == cn) {
			victim->pcdata->petition = CLAN_NONE;
			char_puts("Petition was rejected.\n\r", ch);
			char_printf(victim, "Your petition to %s was "
				    "rejected.\n\r",
				    clan->name);
			return;
		}

		char_puts("They didn't petition.\n\r", ch);
		return;
	}

	if (IS_IMMORTAL(ch)) {
		char_puts("Nah, you shouldn't do that.\n\r", ch);
		return;
	}

	if ((cn = cn_lookup(arg1)) <= 0) {
		char_puts("No such clan.\n\r", ch);
		return;
	}

	ch->pcdata->petition = cn;
	char_puts("Petition sent.\n\r", ch);
}

void do_promote(CHAR_DATA *ch, const char *argument)
{
	char arg1[MAX_STRING_LENGTH];
	char arg2[MAX_STRING_LENGTH];
	CHAR_DATA *victim;
	CLAN_DATA *clan;

	if (IS_NPC(ch)
	||  (!IS_IMMORTAL(ch) && ch->pcdata->clan_status != CLAN_LEADER)) {
		char_puts("Huh?\n\r", ch);
		return;
	}

	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (!*arg1 || !*arg2) {
		char_puts("Usage: promote <char name> <commoner | secondary>\n\r",
			  ch);
		if (IS_IMMORTAL(ch))
			char_puts("    or: promote <char name> <leader>\n\r",
				  ch);
		return;
	}

	victim = get_char_world(ch, arg1);
	if (!victim || IS_NPC(victim)) {
		char_puts("They aren't here.\n\r", ch);
		return;
	}

	if (victim->clan == 0 || (clan = clan_lookup(victim->clan)) == NULL
	||  (victim->clan != ch->clan && !IS_IMMORTAL(ch))) {
		char_puts("They are not an a clan.\n\r", ch);
		return;
	}

	if (!IS_IMMORTAL(ch) && victim->pcdata->clan_status == CLAN_LEADER) {
		char_puts("You don't have enough power to promote them.\n\r",
			  ch);
		return;
	}

	if (!str_prefix(arg2, "leader") && IS_IMMORTAL(ch)) {
		victim->pcdata->clan_status = CLAN_LEADER;
		char_puts("Ok.\n\r", ch);
		char_puts("They are now leader in their clan.\n\r", ch);
		char_puts("You are now leader in your clan.\n\r", victim);
		return;
	}

	if (!str_prefix(arg2, "secondary")) {
		if (victim->pcdata->clan_status == CLAN_SECOND) {
			char_puts("They are already second in a clan.\n\r",
				  ch);
			return;
		}

		victim->pcdata->clan_status = CLAN_SECOND;
		char_puts("They are now second in the clan.\n\r", ch);
		char_puts("You are now second in the clan.\n\r", victim);
		return;
	}

	if (!str_prefix(arg2, "commoner")) {
		if (victim->pcdata->clan_status == CLAN_COMMON) {
			char_puts("They are already commoner in a clan.\n\r",
				  ch);
			return;
		}

		victim->pcdata->clan_status = CLAN_COMMON;
		char_puts("They are now commoner in the clan.\n\r", ch);
		char_puts("You are now commoner in the clan.\n\r", victim);
		return;
	}

	do_promote(ch, "");
}

char *get_status_alias(int status)
{
	switch (status) {
	case CLAN_COMMON:
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
	char_puts("clanlist: temporarily disabled.\n\r", ch);
#if 0
	DESCRIPTOR_DATA *d;
	FILE *pfile;
	DIR *dirp;
	char buf[PATH_MAX];
	struct dirent *dp;
	char letter;

	if (IS_NPC(ch))
		return;

	if (ch->clan == CLAN_NONE || ch->pcdata->clan_status == CLAN_COMMON) {
		char_puts("Huh?\n\r", ch);
		return;
	}

                                                                                
	if ((dirp = opendir(PLAYER_DIR)) == NULL) {
		bug("Clan_list: unable to open player directory.", 0);
		exit(1);
	}

	for (d = descriptor_list; d; d = d->next)
		if (d->character)
			do_save(d->character, "");

	char_puts("Now listing members of your clan:\n\r", ch);
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
			char_printf(ch, "%s -- %s\n\r", name,
				    get_status_alias(status));
		fclose(pfile);
	}
#endif
}
