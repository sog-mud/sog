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
 * $Id: clan.c,v 1.39 1999-04-21 14:44:29 kostik Exp $
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

DECLARE_DO_FUN(do_asave);

varr clans = { sizeof(clan_t), 4 };

clan_t *clan_new(void)
{
	clan_t *clan;

	clan = varr_enew(&clans);
	clan->skills.nsize = sizeof(clskill_t);
	clan->skills.nstep = 4;

	return clan;
}

void clan_free(clan_t *clan)
{
	varr_free(&clan->skills);
}

void clan_save(clan_t *clan)
{
	SET_BIT(clan->flags, CLAN_CHANGED);
	do_asave(NULL, "clans");
}

int cln_lookup(const char *name)
{
	int cln;

	if (IS_NULLSTR(name))
		return -1;

	for (cln = 0; cln < clans.nused; cln++)
		if (!str_cmp(name, CLAN(cln)->name))
			return cln;

	return -1;
}

const char *clan_name(int cln)
{
	clan_t *clan = clan_lookup(cln);
	if (clan)
		return clan->name;
	return "None";
}

void do_petitio(CHAR_DATA *ch, const char *argument)
{
	char_puts("You must enter full command to petition.\n",ch);
}

/*
 * clan_update_lists - remove 'victim' from leader and second lists of 'clan'
 *		       if memb is TRUE 'victim' will be delete from members
 *		       list
 */
void clan_update_lists(clan_t *clan, CHAR_DATA *victim, bool memb)
{
	const char **nl = NULL;

	switch (victim->pcdata->clan_status) {
	case CLAN_SECOND:
		nl = &clan->second_list;
		break;

	case CLAN_LEADER:
		nl = &clan->leader_list;
		break;
	}
	if (nl)
		name_delete(nl, victim->name, NULL, NULL);

	if (memb)
		name_delete(&clan->member_list, victim->name, NULL, NULL);
}

void do_mark(CHAR_DATA *ch, const char *argument)
{
	OBJ_DATA *mark;
	clan_t *clan = NULL;

	if ((ch->clan == 0) || ((clan=clan_lookup(ch->clan)) == NULL)) {
		char_puts("You are not in clan.\n", ch);
		return;
	}
	if (!clan->mark_vnum) {
		char_puts ("Your clan do not have any mark.\n", ch);
		return;
	}
	if ((mark=get_eq_char(ch, WEAR_CLANMARK))!=NULL) {
		obj_from_char(mark);
		extract_obj(mark, 0);
	}
	mark = create_obj(get_obj_index(clan->mark_vnum), 0);
	obj_to_char (mark, ch);
	equip_char (ch, mark, WEAR_CLANMARK);
}

void do_petition(CHAR_DATA *ch, const char *argument)
{
	bool accept;
	int cln = 0;
	clan_t *clan = NULL;
	char arg1[MAX_STRING_LENGTH];
	OBJ_DATA *mark;

	if (IS_NPC(ch))
		return;	

	argument = one_argument(argument, arg1, sizeof(arg1));

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
		char arg2[MAX_STRING_LENGTH];

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
			if (victim->pcdata->petition != cln) {
				char_puts("They didn't petition.\n", ch);
				return;
			}

			victim->clan = cln;
			victim->pcdata->clan_status = CLAN_COMMONER;
			update_skills(victim);

			name_add(&clan->member_list, victim->name, NULL, NULL);
			clan_save(clan);

			char_puts("Greet new member!\n", ch);
			char_printf(victim, "Your petition to %s has been "
				    "accepted.\n",
				    clan->name);
			char_printf(victim, "You are now one of %s!\n",
				    clan->name);
			if ((mark = get_eq_char(victim, WEAR_CLANMARK)) != NULL) {
				obj_from_char(mark);
				extract_obj(mark, 0);
			}
			if (clan->mark_vnum) {
				mark = create_obj(get_obj_index(clan->mark_vnum), 0);
				obj_to_char(mark, victim);
				equip_char(victim, mark, WEAR_CLANMARK);
			};
			return;
		}

/* handle 'petition reject' */
		if (victim->clan == cln) {
			if (victim->pcdata->clan_status == CLAN_LEADER
			&&  !IS_IMMORTAL(ch)) {
				char_puts("You don't have enough power "
					  "to do that.\n", ch);
				return;
			}

			clan_update_lists(clan, victim, TRUE);
			clan_save(clan);

			victim->clan = CLAN_NONE;
			REMOVE_BIT(victim->pcdata->trust, TRUST_CLAN);
			update_skills(victim);

			char_printf(ch, "They are not a member of %s "
					"anymore.\n", clan->name);
			char_printf(victim, "You are not a member of %s "
					    "anymore.\n", clan->name);

			if ((mark = get_eq_char(victim, WEAR_CLANMARK))) {
				obj_from_char(mark);
				extract_obj(mark, 0);
			}

			return;
		}

		if (victim->pcdata->petition == cln) {
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
			||  vch->pcdata->petition != cln)
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

	if ((cln = cln_lookup(arg1)) <= 0) {
		char_puts("No such clan.\n", ch);
		return;
	}

	if (ch->clan) {
		char_puts("You cannot leave your clan this way.\n", ch);
		return;
	}

	ch->pcdata->petition = cln;
	char_puts("Petition sent.\n", ch);
}

void do_promote(CHAR_DATA *ch, const char *argument)
{
	char arg1[MAX_STRING_LENGTH];
	char arg2[MAX_STRING_LENGTH];
	CHAR_DATA *victim;
	clan_t *clan;

	if (IS_NPC(ch)
	||  (!IS_IMMORTAL(ch) && ch->pcdata->clan_status != CLAN_LEADER)) {
		char_puts("Huh?\n", ch);
		return;
	}

	argument = one_argument(argument, arg1, sizeof(arg1));
	argument = one_argument(argument, arg2, sizeof(arg2));

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
		if (victim->pcdata->clan_status == CLAN_LEADER) {
			char_puts("They are already leader in a clan.\n",
				  ch);
			return;
		}

		clan_update_lists(clan, victim, FALSE);
		name_add(&clan->leader_list, victim->name, NULL, NULL);
		clan_save(clan);

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

		clan_update_lists(clan, victim, FALSE);
		name_add(&clan->second_list, victim->name, NULL, NULL);
		clan_save(clan);

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

		clan_update_lists(clan, victim, FALSE);
		clan_save(clan);

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

void show_clanlist(CHAR_DATA *ch, clan_t *clan,
		   const char *list, const char *name_list)
{
	BUFFER *output;
	char name[MAX_STRING_LENGTH];
	bool found = FALSE;

	output = buf_new(-1);
	buf_printf(output, "List of %s of %s:\n", name_list, clan->name);

	list = first_arg(list, name, sizeof(name), FALSE);
	for (; name[0]; list = first_arg(list, name, sizeof(name), FALSE)) {
		found = TRUE;
		buf_printf(output, "- %s\n", name);
	}

	if (!found)
		buf_add(output, "None.\n");

	page_to_char(buf_string(output), ch);
	buf_free(output);
}

void do_clanlist(CHAR_DATA *ch, const char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	clan_t *clan = NULL;

	argument = one_argument(argument, arg1, sizeof(arg1));
		   one_argument(argument, arg2, sizeof(arg2));

	if (IS_IMMORTAL(ch) && arg2[0]) {
		int cln;

		if ((cln = cln_lookup(arg2)) < 0) {
			char_printf(ch, "%s: no such clan.\n", arg2);
			return;
		}
		clan = CLAN(cln);
	}

	if (!clan
	&&  (!ch->clan || (clan = clan_lookup(ch->clan)) == NULL)) {
		char_puts("You are not in a clan.\n", ch);
		return;
	}

	if (arg1[0] == '\0' || !str_prefix(arg1, "member")) {
		show_clanlist(ch, clan, clan->member_list, "members");
		return;
	}

	if (!str_prefix(arg1, "leader")) {
		show_clanlist(ch, clan, clan->leader_list, "leaders");
		return;
	}

	if (!str_prefix(arg1, "second")) {
		show_clanlist(ch, clan, clan->second_list, "secondaries");
		return;
	}

	do_clanlist(ch, str_empty);
}

void do_item(CHAR_DATA* ch, const char* argument)
{
	clan_t* clan = NULL;
	OBJ_DATA* in_obj;
	int cln;
	char arg[MAX_STRING_LENGTH];

	one_argument(argument, arg, sizeof(arg));
	if (IS_IMMORTAL(ch) && arg[0]) {
		if ((cln = cln_lookup(arg)) < 0) {
			char_printf(ch, "%s: no such clan.\n", arg);
			return;
		}
		clan = CLAN(cln);
	}

	if (!clan
	&&  (!ch->clan || (clan = clan_lookup(ch->clan)) == NULL)) {
		char_puts("You are not in clan, you should not worry about your clan item.\n", ch);
		return;
	}

	if (clan->obj_ptr == NULL) {
		char_puts("Your clan do not have an item of power.\n",ch);
		return;
	}

	for (in_obj = clan->obj_ptr; in_obj->in_obj; in_obj = in_obj->in_obj)
		;

	if (in_obj->carried_by) {
		act_puts3("$p is in $R, carried by $N.",
			  ch, clan->obj_ptr, in_obj->carried_by,
			  in_obj->carried_by->in_room,
			  TO_CHAR, POS_DEAD);
	}
	else if (in_obj->in_room) {
		act_puts3("$p is in $R.",
			  ch, clan->obj_ptr, NULL, in_obj->in_room,
			  TO_CHAR, POS_DEAD);
		for (cln = 0; cln < clans.nused; cln++) 
			if (in_obj->in_room->vnum == CLAN(cln)->altar_vnum) {
				act_puts("It is altar of $t",
					 ch, CLAN(cln)->name, NULL,
					 TO_CHAR | ACT_TRANS, POS_DEAD);
			}
	}
	else 
		act_puts("$p is somewhere.",
			 ch, clan->obj_ptr, NULL, TO_CHAR, POS_DEAD);
}

bool clan_item_ok(int cln)
{
	clan_t* clan;
	OBJ_DATA* obj;
	int room_in;
	int i;
	if (!(clan=clan_lookup(cln)) || !(clan->obj_ptr)) 
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
