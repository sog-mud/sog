/*
 * $Id: clan.c,v 1.9 1998-08-17 18:47:03 fjoe Exp $
 */

#include <sys/syslimits.h>
#include <sys/time.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include "merc.h"
#include "comm.h"
#include "db.h"
#include "resource.h"
#include "lookup.h"
#include "log.h"
#include "util.h"
#include "interp.h"

void do_petition(CHAR_DATA *ch, const char *argument)
{
	int clan = CLAN_NONE;
	CHAR_DATA *victim = NULL;
	char arg1[MAX_STRING_LENGTH],
	     arg2[MAX_STRING_LENGTH];

	if (IS_NPC(ch))
		return;	

	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (!*arg1) {
		char_puts("Usage: petition <accept|reject> <char-name>\n\r"
			  "   or: petition <clan-name>\n\r", ch);
		return;
	}

	if (!str_prefix(arg1, "accept")) {
		if (!*arg2) {
			do_petition(ch, "");
			return;
		}

		if (ch->pcdata->clan_status != CLAN_LEADER
		&&  ch->pcdata->clan_status != CLAN_SECOND
		&&  !IS_IMMORTAL(ch)) {
			char_puts("You don't have enough power to accept "
				  "petitions.\n\r", ch);
			return;
		}

		if (!(victim = get_char_world(ch, arg2))
		||  IS_NPC(victim)) {
			char_puts("Can't find them.\n\r", ch);
			return;
		}

		if (victim->pcdata->petition != ch->clan) {
			char_puts("They didn't petition.\n\r", ch);
			return;
		}
		victim->clan = ch->clan;
		victim->pcdata->petition = CLAN_NONE;
		victim->pcdata->clan_status = CLAN_COMMON;
		char_nputs(MSG_OK, ch);
		char_puts("Greet new member!\n\r", ch);
		char_printf(victim, "Your petition to %s has been "
			    "accepted.\n\r",
			    clan_table[victim->clan].short_name);
		char_printf(victim, "You are now one of %s!\n\r",
			    clan_table[victim->clan].long_name);
		return;
	}

	if (!str_prefix(arg1, "reject")) {
		if (!*arg2) {
			do_petition(ch, "");
			return;
		}

		if (ch->pcdata->clan_status != CLAN_LEADER
		&&  ch->pcdata->clan_status != CLAN_SECOND
		&&  !IS_IMMORTAL(ch)) {
			char_puts("You don't have enough power to reject "
				  "petitions.\n\r", ch);
			return;
		}

		if (!(victim = get_char_world(ch, arg2))
		||  IS_NPC(victim)) {
			char_puts("Can't find them.\n\r", ch);
			return;
		}

		if (victim->clan == ch->clan
		&&  victim->pcdata->clan_status != CLAN_LEADER) {
			victim->clan = CLAN_NONE;
			victim->pcdata->petition = CLAN_NONE;
			char_nputs(MSG_OK, ch);
			char_puts("They are not a member of your clan "
				  "any more.\n\r", ch);
			char_puts("You are not a member of any clan "
				  "any more.\n\r", victim);
			return;
		}

		if (victim->pcdata->petition == ch->clan) {
			victim->pcdata->petition = CLAN_NONE;
			char_nputs(MSG_OK, ch);
			char_puts("Petition was rejected.\n\r", ch);
			char_printf(victim, "Your petition to %s was "
				    "rejected.\n\r",
				    clan_table[ch->clan].short_name);
			return;
		}

		char_puts("They didn't petition.\n\r", ch);
		return;
	}

	if ((clan = clan_lookup(arg1)) == -1 ||  clan == CLAN_NONE) {
		char_puts("No such clan.\n\r", ch);
		return;
	}

	ch->pcdata->petition = clan;
	char_puts("Petition sent.\n\r", ch);
}

void do_promote(CHAR_DATA *ch, const char *argument)
{
	char arg1[MAX_STRING_LENGTH],
	     arg2[MAX_STRING_LENGTH];
	CHAR_DATA *victim;

	if (!IS_IMMORTAL(ch)
	&&  (IS_NPC(ch) || ch->pcdata->clan_status != CLAN_LEADER)) {
		char_nputs(MSG_HUH, ch);
		return;
	}

	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (!*arg1 || !*arg2) {
		char_puts("Usage: promote <char-name> <commoner|secondary>\n\r",
			  ch);
		if (IS_IMMORTAL(ch))
			char_puts("    or: promote <char-name> <leader>\n\r",
				  ch);
		return;
	}

	victim = get_char_world(ch, arg1);

	if (!victim || IS_NPC(victim)) {
		char_nputs(MSG_THEY_ARENT_HERE, ch);
		return;
	}

	if (!str_prefix(arg2, "leader")) {
		if (!IS_IMMORTAL(ch)) {
			char_puts("This option is only for immortals.\n\r",
				  ch);
			return;
		}

		if (victim->clan == CLAN_NONE) {
			char_puts("They are clanless.\n\r", ch);
			return;
		}

		victim->pcdata->clan_status = CLAN_LEADER;
		char_nputs(MSG_OK, ch);
		char_puts("They are now leader in their clan.\n\r", ch);
		char_puts("You are now leader in your clan.\n\r", victim);
		return;
	}

	if (ch->clan != victim->clan) {
		char_puts("You don't have enough power to change "
			  "there status.\n\r", ch);
		return;
	}

	if (!str_prefix(arg2, "secondary")) {
		if (victim->pcdata->clan_status == CLAN_SECOND) {
			char_puts("They are already second in your clan.\n\r",
				  ch);
			return;
		}

		if (victim->pcdata->clan_status == CLAN_LEADER
		&&  !IS_IMMORTAL(ch)) {
			char_puts("You don't have enough power to promote "
				  "them to be secondary.\n\r", ch);
			return;
		}

		victim->pcdata->clan_status = CLAN_SECOND;
		char_nputs(MSG_OK, ch);
		char_puts("They are now second in clan.\n\r", ch);
		char_puts("You are now second in clan.\n\r", victim);
		return;
	}

	if (!str_prefix(arg2, "commoner")) {
		if (victim->pcdata->clan_status == CLAN_COMMON) {
			char_puts("They are already common in your clan.\n\r",
				  ch);
			return;
		}

		if (victim->pcdata->clan_status == CLAN_LEADER
		&&  !IS_IMMORTAL(ch)) {
			char_puts("You don't have enough power to promote "
				  "them to be commoner.\n\r", ch);
			return;
		}

		victim->pcdata->clan_status = CLAN_COMMON;
		char_nputs(MSG_OK, ch);
		char_puts("They are now common in your clan.\n\r", ch);
		char_puts("You are now common in your clan.\n\r", victim);
		return;
	}

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
	DESCRIPTOR_DATA *d;
	FILE *pfile;
	DIR *dirp;
	char buf[PATH_MAX];
	struct dirent *dp;
	char letter;

	if (IS_NPC(ch))
		return;

	if (ch->clan == CLAN_NONE || ch->pcdata->clan_status == CLAN_COMMON) {
		char_nputs(MSG_HUH, ch);
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
}
