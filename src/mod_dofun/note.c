/*
 * $Id: note.c,v 1.22 2001-08-03 11:27:37 fjoe Exp $
 */

/***************************************************************************
 *     ANATOLIA 2.1 is copyright 1996-1997 Serdar BULUT, Ibrahim CANPUNAR  *
 *     ANATOLIA has been brought to you by ANATOLIA consortium		   *
 *	 Serdar BULUT {Chronos}		bulut@rorqual.cc.metu.edu.tr	   *
 *	 Ibrahim Canpunar  {Asena}	canpunar@rorqual.cc.metu.edu.tr    *
 *	 Murat BICER  {KIO}		mbicer@rorqual.cc.metu.edu.tr	   *
 *	 D.Baris ACAR {Powerman}	dbacar@rorqual.cc.metu.edu.tr	   *
 *     By using this code, you have agreed to follow the terms of the	   *
 *     ANATOLIA license, in the file Anatolia/anatolia.licence		   *
 ***************************************************************************/

/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,	   *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *									   *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael	   *
 *  Chastain, Michael Quan, and Mitchell Tse.				   *
 *									   *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc	   *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.						   *
 *									   *
 *  Much time and thought has gone into this software and you are	   *
 *  benefitting.  We hope that you share your changes too.  What goes	   *
 *  around, comes around.						   *
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <merc.h>
#include <note.h>

#include <handler.h>

static int count_spool(CHAR_DATA *ch, note_t *spool);
static void parse_note(CHAR_DATA *ch, const char *argument, int type);

void do_unread(CHAR_DATA *ch, const char *argument)
{
	int count;
	bool found = FALSE;

	if (IS_NPC(ch))
		return;

	if ((count = count_spool(ch, news_list)) > 0) {
		found = TRUE;
		act_puts("You have $j news $qj{articles} waiting.",
			 ch, (const void *) count, NULL, TO_CHAR, POS_DEAD);
	}

	if ((count = count_spool(ch, changes_list)) > 0) {
		found = TRUE;
		act_puts("There $qj{are} $j $qj{changes} waiting to be read.",
			 ch, (const void *) count, NULL, TO_CHAR, POS_DEAD);
	}

	if ((count = count_spool(ch, note_list)) > 0) {
		found = TRUE;
		act_puts("You have $j new $qj{notes} waiting.",
			 ch, (const void *) count, NULL, TO_CHAR, POS_DEAD);
	}

	if ((count = count_spool(ch, idea_list)) > 0) {
		found = TRUE;
		act_puts("You have $j unread $qj{ideas} to peruse.",
			 ch, (const void *) count, NULL, TO_CHAR, POS_DEAD);
	}

	if (IS_IMMORTAL(ch) && (count = count_spool(ch, penalty_list)) > 0) {
		found = TRUE;
		act_puts("$j $qj{penalties} $qj{have} been added.",
			 ch, (const void *) count, NULL, TO_CHAR, POS_DEAD);
	}

	if (!found && str_cmp(argument, "login"))
		act_char("You have no unread messages.", ch);
}

void do_note(CHAR_DATA *ch,const char *argument)
{
	parse_note(ch, argument, NOTE_NOTE);
}

void do_idea(CHAR_DATA *ch,const char *argument)
{
	parse_note(ch, argument, NOTE_IDEA);
}

void do_penalty(CHAR_DATA *ch,const char *argument)
{
	parse_note(ch, argument, NOTE_PENALTY);
}

void do_news(CHAR_DATA *ch,const char *argument)
{
	parse_note(ch, argument, NOTE_NEWS);
}

void do_changes(CHAR_DATA *ch,const char *argument)
{
	parse_note(ch, argument, NOTE_CHANGES);
}

/*-------------------------------------------------------------------
 * static functions
 */

static void note_show(BUFFER *buf, note_t *pnote, int vnum);
static const char *note_quote(note_t *pnote);
static void append_note(note_t *pnote);
static void update_read(CHAR_DATA *ch, note_t *pnote);
static bool is_note_to(CHAR_DATA *ch, note_t *pnote);
static bool note_attach(CHAR_DATA *ch, int type);
static void note_remove(CHAR_DATA *ch, note_t *pnote, bool delete);
static bool hide_note(CHAR_DATA *ch, note_t *pnote);
static void parse_note(CHAR_DATA *ch, const char *argument, int type);

static void update_read(CHAR_DATA *ch, note_t *pnote)
{
	time_t stamp;

	if (IS_NPC(ch))
		return;

	stamp = pnote->date_stamp;

	switch (pnote->type) {
	default:
		return;
	case NOTE_NOTE:
		PC(ch)->last_note = UMAX(PC(ch)->last_note, stamp);
		break;
	case NOTE_IDEA:
		PC(ch)->last_idea = UMAX(PC(ch)->last_idea, stamp);
		break;
	case NOTE_PENALTY:
		PC(ch)->last_penalty = UMAX(PC(ch)->last_penalty, stamp);
		break;
	case NOTE_NEWS:
		PC(ch)->last_news = UMAX(PC(ch)->last_news, stamp);
		break;
	case NOTE_CHANGES:
		PC(ch)->last_changes = UMAX(PC(ch)->last_changes, stamp);
		break;
	}
}

static void note_show(BUFFER *buf, note_t *pnote, int vnum)
{
	buf_printf(buf, BUF_END, "{x[%3d] From: %s, {x%s\n"
			"{x      To  : %s\n"
			"{x      Subj: %s\n"
			"{x%s\n{x",
		   vnum, pnote->sender, pnote->date,
		   pnote->to_list,
		   pnote->subject,
		   pnote->text);
}

static const char *note_quote(note_t *pnote)
{
	const char *p;
	char *q;
	char buf[MAX_STRING_LENGTH];
	bool need_quote;

	if (IS_NULLSTR(pnote->text))
		return str_dup(str_empty);

	snprintf(buf, sizeof(buf),
		 "On %s %s {xwrote to %s:\n"		// notrans
		 "{x\n",
		 pnote->date, pnote->sender, pnote->to_list);

	q = strchr(buf, '\0');
	need_quote = TRUE;
	for (p = pnote->text; *p && q-buf < sizeof(buf); p++) {
		if (need_quote) {
			*q++ = '>';
			*q++ = ' ';
			need_quote = FALSE;
		}

		*q++ = *p;

		if (*p == '\n')
			need_quote = TRUE;
	}
	*q = '\0';

	return str_dup(buf);
}

static void append_note(note_t *pnote)
{
	FILE *fp;
	const char *name;
	note_t **list;
	note_t *last;

	switch(pnote->type) {
	default:
		return;
	case NOTE_NOTE:
		name = NOTE_FILE;
		list = &note_list;
		break;
	case NOTE_IDEA:
		name = IDEA_FILE;
		list = &idea_list;
		break;
	case NOTE_PENALTY:
		name = PENALTY_FILE;
		list = &penalty_list;
		break;
	case NOTE_NEWS:
		 name = NEWS_FILE;
		 list = &news_list;
		 break;
	case NOTE_CHANGES:
		 name = CHANGES_FILE;
		 list = &changes_list;
		 break;
	}

	if (*list == NULL)
		*list = pnote;
	else {
		for (last = *list; last->next; last = last->next);
			last->next = pnote;
	}

	if ((fp = dfopen(NOTES_PATH, name, "a")) == NULL)
		return;
	fwrite_note(fp, pnote);
        fclose(fp);
}

static bool is_note_to(CHAR_DATA *ch, note_t *pnote)
{
	clan_t *clan;

	if (!str_cmp(ch->name, pnote->sender))
		return TRUE;

	if (!str_cmp("all", pnote->to_list))
		return TRUE;

	if (IS_IMMORTAL(ch) && is_name("imm", pnote->to_list))	// notrans
		return TRUE;

	if ((IS_IMMORTAL(ch) || !IS_NULLSTR(ch->clan))
	&&  is_name("clan", pnote->to_list))
		return TRUE;

	if (is_name_strict(ch->name, pnote->to_list))
		return TRUE;

	if ((clan = clan_lookup(ch->clan))
	&&  is_name_strict(clan->name, pnote->to_list))
		return TRUE;

	return FALSE;
}

/*
 * note attach - create note
 * Returns: TRUE  - everything is ok, note attached
 *	    FALSE - char is working on different kind of note right now
 */
static bool note_attach(CHAR_DATA *ch, int type)
{
	note_t *pnote;
	PC_DATA *pc = PC(ch);

	if (pc->pnote) {
		if (pc->pnote->type != type) {
			act_puts("You have an unfinished $t in progress.",
				 ch,
				 flag_string(note_types, pc->pnote->type),
				 NULL, TO_CHAR, POS_DEAD);
			return FALSE;
		}

		return TRUE;
	}

	pnote		= new_note();
	pnote->next	= NULL;
	pnote->sender	= str_qdup(ch->name);
	pnote->date	= str_empty;
	pnote->to_list	= str_empty;
	pnote->subject	= str_empty;
	pnote->text	= str_empty;
	pnote->type	= type;
	pc->pnote	= pnote;
	return TRUE;
}

static void note_remove(CHAR_DATA *ch, note_t *pnote, bool delete)
{
	char to_new[MAX_INPUT_LENGTH];
	char to_one[MAX_INPUT_LENGTH];
	note_t *prev;
	note_t **list;
	const char *to_list;

	if (!delete) {
		/* make a new list */
		to_new[0] = '\0';
		to_list	= pnote->to_list;
		while (*to_list != '\0') {
			to_list	= one_argument(to_list, to_one, sizeof(to_one));
			if (to_one[0] != '\0' && str_cmp(ch->name, to_one)) {
				strnzcat(to_new, sizeof(to_new), " ");
				strnzcat(to_new, sizeof(to_new), to_one);
			}
		}

		/* Just a simple recipient removal? */
		if (str_cmp(ch->name, pnote->sender) && to_new[0] != '\0') {
			free_string(pnote->to_list);
			pnote->to_list = str_dup(to_new + 1);
			return;
		}
	}

	/* nuke the whole note */

	switch(pnote->type) {
	default:
		return;
	case NOTE_NOTE:
		list = &note_list;
		break;
	case NOTE_IDEA:
		list = &idea_list;
		break;
	case NOTE_PENALTY:
		list = &penalty_list;
		break;
	case NOTE_NEWS:
		list = &news_list;
		break;
	case NOTE_CHANGES:
		list = &changes_list;
		break;
	}

	/*
	 * Remove note from linked list.
	 */
	if (pnote == *list)
		*list = pnote->next;
	else {
		for (prev = *list; prev != NULL; prev = prev->next) {
			if (prev->next == pnote)
				break;
		}

		if (prev == NULL)
			return;

		prev->next = pnote->next;
	}

	save_notes(pnote->type);
	free_note(pnote);
}

static bool hide_note(CHAR_DATA *ch, note_t *pnote)
{
	time_t last_read;

	if (IS_NPC(ch))
		return TRUE;

	switch (pnote->type) {
	default:
		return TRUE;
	case NOTE_NOTE:
		last_read = PC(ch)->last_note;
		break;
	case NOTE_IDEA:
		last_read = PC(ch)->last_idea;
		break;
	case NOTE_PENALTY:
		last_read = PC(ch)->last_penalty;
		break;
	case NOTE_NEWS:
		last_read = PC(ch)->last_news;
		break;
	case NOTE_CHANGES:
		last_read = PC(ch)->last_changes;
		break;
	}

	if (pnote->date_stamp <= last_read)
		return TRUE;

	if (!str_cmp(ch->name, pnote->sender))
		return TRUE;

	if (!is_note_to(ch, pnote))
		return TRUE;

	return FALSE;
}

static void parse_note(CHAR_DATA *ch, const char *argument, int type)
{
	char arg[MAX_INPUT_LENGTH];
	note_t *pnote;
	note_t **list;
	char *list_name;
	int vnum;
	int anum;
	DESCRIPTOR_DATA *d;
	PC_DATA *pc;

	if (IS_NPC(ch))
		return;

	switch(type) {
	default:
		return;
	case NOTE_NOTE:
		list = &note_list;
		list_name = "notes";
		break;
	case NOTE_IDEA:
		list = &idea_list;
		list_name = "ideas";
		break;
	case NOTE_PENALTY:
		list = &penalty_list;
		list_name = "penalties";
		break;
	case NOTE_NEWS:
		list = &news_list;
		list_name = "news";
		break;
	case NOTE_CHANGES:
		list = &changes_list;
		list_name = "changes";
		break;
	}

	argument = one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0' || !str_prefix(arg, "read")) {
		bool fAll;
		BUFFER *output;

		if (!str_cmp(argument, "all")) {
			fAll = TRUE;
			anum = 0;
		}
		else if (argument[0] == '\0' || !str_prefix(argument, "next")) {
			/* read next unread note */

			vnum = 0;
			for (pnote = *list; pnote; pnote = pnote->next) {
				if (!hide_note(ch, pnote)) {
					output = buf_new(0);
					note_show(output, pnote, vnum);
					page_to_char(buf_string(output), ch);
					buf_free(output);
					update_read(ch, pnote);
					return;
				}
				else if (is_note_to(ch, pnote))
					vnum++;
			}
			act_char("You have no unread messages.", ch);
			return;
		} else if (is_number(argument)) {
			fAll = FALSE;
			anum = atoi(argument);
		} else {
			act_char("Read which number?", ch);
			return;
		}

		vnum = 0;
		for (pnote = *list; pnote != NULL; pnote = pnote->next) {
			if (is_note_to(ch, pnote) && (vnum++ == anum || fAll)) {
				output = buf_new(0);
				note_show(output, pnote, vnum-1);
				page_to_char(buf_string(output), ch);
				buf_free(output);
				if (!fAll)
					return;
			}
		}

		act_puts("There aren't that many $t.",
			 ch, list_name, NULL,
			 TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return;
	}

	if (!str_prefix(arg, "list")) {
		char buf[MAX_INPUT_LENGTH];
		char from[MAX_INPUT_LENGTH];
		char to[MAX_INPUT_LENGTH];
		BUFFER *output;

#		define CHECK_TO		(A)
#		define CHECK_FROM	(B)

		int flags = 0;

		for (;;) {
			argument = one_argument(argument, buf, sizeof(buf));

			if (!str_cmp(buf, "from")) {
				argument = one_argument(argument,
							from, sizeof(from));
				if (from[0] == '\0')
					break;
				SET_BIT(flags, CHECK_FROM);
				continue;
			}

			if (!str_cmp(buf, "to")) {
				argument = one_argument(argument,
							to, sizeof(to));
				if (to[0] == '\0')
					break;
				SET_BIT(flags, CHECK_TO);
				continue;
			}

			break;
		}

		vnum = 0;
		output = buf_new(0);
		for (pnote = *list; pnote != NULL; pnote = pnote->next) {
			if (is_note_to(ch, pnote)) {
				vnum++;

				if (IS_SET(flags, CHECK_TO)
				&&  (!str_cmp("all", pnote->to_list) ||
				     !is_name(to, pnote->to_list)))
					continue;

				if (IS_SET(flags, CHECK_FROM)
				&&  str_prefix(from, pnote->sender))
					continue;

				buf_printf(output, BUF_END,
				    "[%3d%c] %s: %s\n{x",	// notrans
				    vnum-1, hide_note(ch, pnote) ? ' ' : 'N',
				    pnote->sender, pnote->subject);
			}
		}

		page_to_char(buf_string(output), ch);
		buf_free(output);
		return;
	}

	if (!str_prefix(arg, "remove")) {
		if (!is_number(argument)) {
			act_char("Remove which number?", ch);
			return;
		}

		anum = atoi(argument);
		vnum = 0;
		for (pnote = *list; pnote != NULL; pnote = pnote->next) {
			if (is_note_to(ch, pnote) && vnum++ == anum) {
				note_remove(ch, pnote, FALSE);
				act_char("Ok.", ch);
				return;
			}
		}

		act_puts("There aren't that many $t.",
			 ch, list_name, NULL,
			 TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return;
	}

	if (!str_prefix(arg, "catchup")) {
		switch(type) {
		case NOTE_NOTE:
			PC(ch)->last_note = current_time;
			break;
		case NOTE_IDEA:
			PC(ch)->last_idea = current_time;
			break;
		case NOTE_PENALTY:
			PC(ch)->last_penalty = current_time;
			break;
		case NOTE_NEWS:
			PC(ch)->last_news = current_time;
			break;
		case NOTE_CHANGES:
			PC(ch)->last_changes = current_time;
			break;
		}
		return;
	}

/* below this point only certain people can edit notes */

	if ((type == NOTE_NEWS || type == NOTE_CHANGES)
	&&  !IS_TRUSTED(ch, LEVEL_IMMORTAL)) {
		act_puts("You aren't high enough level to write $t.",
			 ch, list_name, NULL,
			 TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return;
	}

	if (IS_SET(ch->comm, COMM_NONOTE)) {
		act_char("News pigeon refuses to take your letter.", ch);
		return;
	}

	pc = PC(ch);

	if (!str_prefix(arg, "delete")) {
		if (!is_number(argument)) {
			act_char("Delete which number?", ch);
			return;
		}

		anum = atoi(argument);
		vnum = 0;
		for (pnote = *list; pnote != NULL; pnote = pnote->next) {
			if (is_note_to(ch, pnote) && vnum++ == anum) {
				note_remove(ch, pnote, TRUE);
				act_char("Ok.", ch);
				return;
			}
		}

		act_puts("There aren't that many $t.",
			 ch, list_name, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return;
	}

	if (!str_prefix(arg, "edit")) {
		if (!note_attach(ch, type))
			return;
		string_append(ch, &pc->pnote->text);
		return;
	}

	if (!str_cmp(arg, "+")) {
		const char *p;

		if (!note_attach(ch, type))
			return;

		if (strlen(pc->pnote->text) + strlen(argument) >= MAX_STRING_LENGTH-4) {
			act_char("Note too long.", ch);
			return;
		}

		p = str_printf("%s%s\n", pc->pnote->text, argument);
		free_string(pc->pnote->text);
		pc->pnote->text = p;
		act_char("Ok.", ch);
		return;
	}

	if (!str_cmp(arg, "-")) {
		int len;
		bool found = FALSE;
		char buf[MAX_STRING_LENGTH];

		if (!note_attach(ch, type))
			return;

		if (IS_NULLSTR(pc->pnote->text)) {
			act_char("No lines left to remove.", ch);
			return;
		}

		if (argument[0] != '\0') {
			if (!is_number(argument)) {
				act_char("Remove which number?", ch);
				return;
			}
			pc->pnote->text = string_linedel(pc->pnote->text,
							 atoi(argument));
			act_char("Ok.", ch);
			return;
		}

		strnzcpy(buf, sizeof(buf), pc->pnote->text);
		for (len = strlen(buf); len > 0; len--) {
			if (buf[len] != '\n')
				continue;

			if (!found) {	/* back it up */
				if (len > 0)
					len--;
				found = TRUE;
			} else {
				buf[len + 1] = '\0';
				free_string(pc->pnote->text);
				pc->pnote->text = str_dup(buf);
				act_char("Ok.", ch);
				return;
			}
		}

		free_string(pc->pnote->text);
		pc->pnote->text = str_empty;
		act_char("Ok.", ch);
		return;
	}

	if (!str_prefix(arg, "subject")) {
		if (!note_attach(ch, type))
			return;

		free_string(pc->pnote->subject);
		pc->pnote->subject = str_dup(argument);
		act_char("Ok.", ch);
		return;
	}

	if (!str_prefix(arg, "to")) {
		if (!note_attach(ch, type))
			return;

		free_string(pc->pnote->to_list);
		pc->pnote->to_list = str_dup(argument);
		act_char("Ok.", ch);
		return;
	}

	if (!str_prefix(arg, "forward")) {
		char buf[MAX_INPUT_LENGTH];

		argument = one_argument(argument, buf, sizeof(buf));
		if (!is_number(buf)) {
			act_char("Forward which number?", ch);
			return;
		}

		anum = atoi(buf);
		vnum = 0;
		for (pnote = *list; pnote; pnote = pnote->next)
			if (is_note_to(ch, pnote) && vnum++ == anum)
				break;

		if (!pnote) {
			act_puts("There aren't that many $t.",
				 ch, list_name, NULL,
				 TO_CHAR | ACT_NOTRANS, POS_DEAD);
			return;
		}

		if (!note_attach(ch, type))
			return;

		free_string(pc->pnote->text);
		pc->pnote->text = str_printf(
			"* Forwarded by: %s\n"			    // notrans
			"* Originally to: %s\n"			    // notrans
			"* Originally by: %s, %s\n"		    // notrans
			"\n"
			"---------- Forwarded message ----------\n" // notrans
			"%s",
			ch->name, pnote->to_list, pnote->sender, pnote->date,
			pnote->text);

		free_string(pc->pnote->subject);
		pc->pnote->subject = str_qdup(pnote->subject);

		string_append(ch, &pc->pnote->text);
		return;
	}

	if (!str_prefix(arg, "quote")
	||  !str_prefix(arg, "reply")) {
		char buf[MAX_INPUT_LENGTH];

		argument = one_argument(argument, buf, sizeof(buf));
		if (!is_number(buf)) {
			act_char("Quote which number?", ch);
			return;
		}

		anum = atoi(buf);
		vnum = 0;
		for (pnote = *list; pnote; pnote = pnote->next)
			if (is_note_to(ch, pnote) && vnum++ == anum)
				break;

		if (!pnote) {
			act_puts("There aren't that many $t.",
				 ch, list_name, NULL,
				 TO_CHAR | ACT_NOTRANS, POS_DEAD);
			return;
		}

		if (!note_attach(ch, type))
			return;

		free_string(pc->pnote->text);
		pc->pnote->text = note_quote(pnote);

		free_string(pc->pnote->subject);
		pc->pnote->subject = str_qdup(pnote->subject);

		string_append(ch, &pc->pnote->text);
		return;
	}

	if (!str_prefix(arg, "clear") || !str_prefix(arg, "cancel")) {
		if (pc->pnote) {
			free_note(pc->pnote);
			pc->pnote = NULL;
		}

		act_char("Ok.", ch);
		return;
	}

	if (!str_prefix(arg, "show")) {
		BUFFER *output;

		if (!pc->pnote) {
			act_char("You have no note in progress.", ch);
			return;
		}

		if (pc->pnote->type != type) {
			act_char("You aren't working on that kind of note.", ch);
			return;
		}

		output = buf_new(0);
		buf_printf(output, BUF_END, "{xFrom: %s\n"
				   "{xTo  : %s\n"
				   "{xSubj: %s\n"
				   "{x%s\n"
				   "{x",
			   pc->pnote->sender,
			   pc->pnote->to_list,
			   pc->pnote->subject,
			   pc->pnote->text);
		page_to_char(buf_string(output), ch);
		buf_free(output);

		return;
	}

	if (!str_prefix(arg, "post") || !str_prefix(arg, "send")) {
		if (pc->pnote == NULL) {
			act_char("You have no note in progress.", ch);
			return;
		}

		if (pc->pnote->type != type) {
			act_char("You aren't working on that kind of note.", ch);
			return;
		}

		if (IS_NULLSTR(pc->pnote->to_list)) {
			act_char("You need to provide a recipient (name, clan name, all, or immortal).", ch);
			return;
		}

		if (IS_NULLSTR(pc->pnote->subject)) {
			act_char("You need to provide a subject.", ch);
			return;
		}

		if (IS_NULLSTR(pc->pnote->text)) {
			act_char("You need to provide a text.", ch);
			return;
		}

		pc->pnote->next	= NULL;
		pc->pnote->date	= str_dup(strtime(current_time));
		pc->pnote->date_stamp= current_time;

		append_note(pc->pnote);

		/* Show new note message */
		for (d = descriptor_list; d; d = d->next) {
			CHAR_DATA *fch = d->character;
			if (fch != NULL
			&&  fch != ch
			&&  is_note_to(fch, pc->pnote)
			&&  d->connected == CON_PLAYING)
				do_unread(fch, "login");	// notrans
		}

		pc->pnote = NULL;
		return;
	}

	act_char("You can't do that.", ch);
}

static int count_spool(CHAR_DATA *ch, note_t *spool)
{
	int count = 0;
	note_t *pnote;

	for (pnote = spool; pnote != NULL; pnote = pnote->next) {
		if (!hide_note(ch, pnote))
			count++;
	}

	return count;
}
