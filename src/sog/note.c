/*
 * $Id: note.c,v 1.44 1999-02-17 07:53:25 fjoe Exp $
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
#include <ctype.h>

#include "merc.h"
#include "db/db.h"

/*
 * Data structure for notes.
 */

#define NOTE_NOTE	0
#define NOTE_IDEA	1
#define NOTE_PENALTY	2
#define NOTE_NEWS	3
#define NOTE_CHANGES	4

struct note_data
{
	NOTE_DATA *	next;
	bool		valid;
	int		type;
	const char *	sender;
	const char *	date;
	const char *	to_list;
	const char *	subject;
	const char *	text;
	time_t		date_stamp;
};

/* local procedures */
NOTE_DATA *	new_note	(void);
void		free_note	(NOTE_DATA *note);

void load_thread(char *name, NOTE_DATA **list, int type, time_t free_time);
void parse_note(CHAR_DATA *ch, const char *argument, int type);
bool hide_note(CHAR_DATA *ch, NOTE_DATA *pnote);
void fwrite_note(FILE *fp, NOTE_DATA *pnote);

NOTE_DATA *note_list;
NOTE_DATA *idea_list;
NOTE_DATA *penalty_list;
NOTE_DATA *news_list;
NOTE_DATA *changes_list;

/* stuff for recyling notes */
NOTE_DATA *note_free;

NOTE_DATA *new_note()
{
    NOTE_DATA *note;

    if (note_free == NULL)
	note = alloc_perm(sizeof(*note));
    else
    { 
	note = note_free;
	note_free = note_free->next;
    }
    VALIDATE(note);
    return note;
}

void free_note(NOTE_DATA *note)
{
    if (!IS_VALID(note))
	return;

    free_string(note->text  );
    free_string(note->subject);
    free_string(note->to_list);
    free_string(note->date  );
    free_string(note->sender);
    INVALIDATE(note);

    note->next = note_free;
    note_free   = note;
}

int count_spool(CHAR_DATA *ch, NOTE_DATA *spool)
{
    int count = 0;
    NOTE_DATA *pnote;

    for (pnote = spool; pnote != NULL; pnote = pnote->next)
	if (!hide_note(ch,pnote))
	    count++;

    return count;
}

void do_unread(CHAR_DATA *ch, const char *argument)
{
    int count;
    bool found = FALSE;

    if (IS_NPC(ch))
	return; 

    if ((count = count_spool(ch,news_list)) > 0)
    {
	found = TRUE;
	char_printf(ch,"There %s %d new news article%s waiting.\n",
	    count > 1 ? "are" : "is",count, count > 1 ? "s" : str_empty);
    }
    if ((count = count_spool(ch,changes_list)) > 0)
    {
	found = TRUE;
	char_printf(ch,"There %s %d change%s waiting to be read.\n",
	    count > 1 ? "are" : "is", count, count > 1 ? "s" : str_empty);
    }
    if ((count = count_spool(ch,note_list)) > 0)
    {
	found = TRUE;
	char_printf(ch,"You have %d new note%s waiting.\n",
	    count, count > 1 ? "s" : str_empty);
    }
    if ((count = count_spool(ch,idea_list)) > 0)
    {
	found = TRUE;
	char_printf(ch,"You have %d unread idea%s to peruse.\n",
	    count, count > 1 ? "s" : str_empty);
    }
    if (IS_TRUSTED(ch,ANGEL) && (count = count_spool(ch,penalty_list)) > 0)
    {
	found = TRUE;
	char_printf(ch,"%d %s been added.\n",
	    count, count > 1 ? "penalties have" : "penalty has");
    }

    if (!found && str_cmp(argument, "login"))
	char_puts("You have no unread messages.\n", ch);
}

void do_note(CHAR_DATA *ch,const char *argument)
{
    parse_note(ch,argument,NOTE_NOTE);
}

void do_idea(CHAR_DATA *ch,const char *argument)
{
    parse_note(ch,argument,NOTE_IDEA);
}

void do_penalty(CHAR_DATA *ch,const char *argument)
{
    parse_note(ch,argument,NOTE_PENALTY);
}

void do_news(CHAR_DATA *ch,const char *argument)
{
	parse_note(ch,argument,NOTE_NEWS);
}

void do_changes(CHAR_DATA *ch,const char *argument)
{
	parse_note(ch,argument,NOTE_CHANGES);
}

void save_notes(int type)
{
	FILE *fp;
	char *name;
	NOTE_DATA *pnote;

	switch (type) {
	default:
		return;
	case NOTE_NOTE:
		name = NOTE_FILE;
		pnote = note_list;
		break;
	case NOTE_IDEA:
		name = IDEA_FILE;
		pnote = idea_list;
		break;
	case NOTE_PENALTY:
		name = PENALTY_FILE;
		pnote = penalty_list;
		break;
	case NOTE_NEWS:
		name = NEWS_FILE;
		pnote = news_list;
		break;
	case NOTE_CHANGES:
		name = CHANGES_FILE;
		pnote = changes_list;
		break;
	}

	fclose(fpReserve);
	if ((fp = dfopen(NOTES_PATH, name, "w")) == NULL) {
		perror(name);
		return;
	}

	for (; pnote != NULL; pnote = pnote->next) 
		fwrite_note(fp, pnote);
	fclose(fp);
	fpReserve = fopen(NULL_FILE, "r");
}

void load_notes(void)
{
	load_thread(NOTE_FILE, &note_list, NOTE_NOTE, 14*24*60*60);
	load_thread(IDEA_FILE, &idea_list, NOTE_IDEA, 28*24*60*60);
	load_thread(PENALTY_FILE, &penalty_list, NOTE_PENALTY, 0);
	load_thread(NEWS_FILE, &news_list, NOTE_NEWS, 0);
	load_thread(CHANGES_FILE, &changes_list,NOTE_CHANGES, 0);
}

void load_thread(char *name, NOTE_DATA **list, int type, time_t free_time)
{
    FILE *fp;
    NOTE_DATA *pnotelast;
 
    if ((fp = dfopen(NOTES_PATH, name, "r")) == NULL)
	return;
	 
    pnotelast = NULL;
    for (; ;)
    {
	NOTE_DATA *pnote;
	char letter;
	 
	do
	{
	    letter = getc(fp);
            if (feof(fp))
            {
                fclose(fp);
                return;
            }
        }
        while (isspace(letter));
        ungetc(letter, fp);
 
        pnote           = alloc_perm(sizeof(*pnote));
 
        if (str_cmp(fread_word(fp), "sender"))
            break;
        pnote->sender   = fread_string(fp);
 
        if (str_cmp(fread_word(fp), "date"))
            break;
        pnote->date     = fread_string(fp);
 
        if (str_cmp(fread_word(fp), "stamp"))
            break;
        pnote->date_stamp = fread_number(fp);
 
        if (str_cmp(fread_word(fp), "to"))
            break;
        pnote->to_list  = fread_string(fp);
 
        if (str_cmp(fread_word(fp), "subject"))
            break;
        pnote->subject  = fread_string(fp);
 
        if (str_cmp(fread_word(fp), "text"))
            break;
        pnote->text     = fread_string(fp);
 
        if (free_time && pnote->date_stamp < current_time - free_time)
        {
	    free_note(pnote);
            continue;
        }

	pnote->type = type;
 
        if (*list == NULL)
            *list           = pnote;
        else
            pnotelast->next     = pnote;
 
        pnotelast       = pnote;
    }
 
	strnzcpy(filename, NOTES_PATH, sizeof(filename));
	strnzcat(filename, name, sizeof(filename));
	db_error("load_notes", "bad keyword");
}

void append_note(NOTE_DATA *pnote)
{
	FILE *fp;
	char *name;
	NOTE_DATA **list;
	NOTE_DATA *last;

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

	fclose(fpReserve);
	if ((fp = dfopen(NOTES_PATH, name, "a")) == NULL) {
        	perror(name);
		return;
	}
	fwrite_note(fp, pnote);
        fclose(fp);
	fpReserve = fopen(NULL_FILE, "r");
}

bool is_note_to(CHAR_DATA *ch, NOTE_DATA *pnote)
{
	CLAN_DATA *clan;

	if (!str_cmp(ch->name, pnote->sender))
		return TRUE;

	if (!str_cmp("all", pnote->to_list))
		return TRUE;

	if (IS_IMMORTAL(ch) && is_name("imm", pnote->to_list))
		return TRUE;

	if ((IS_IMMORTAL(ch) || ch->clan) && is_name("clan", pnote->to_list))
		return TRUE;

	if (is_name_raw(ch->name, pnote->to_list, str_cmp))
		return TRUE;

	if ((clan = clan_lookup(ch->clan))
	&&  is_name_raw(clan->name, pnote->to_list, str_cmp))
		return TRUE;

	return FALSE;
}

/*
 * note attach - create note
 * Returns: TRUE  - everything is ok, note attached
 *	    FALSE - char is working on different kind of note right now
 */
bool note_attach(CHAR_DATA *ch, int type)
{
	NOTE_DATA *pnote;

	if (ch->pnote) {
		if (ch->pnote->type != type) {
			char_puts("You already have a different note "
				  "in progress.\n", ch);
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
	ch->pnote	= pnote;
	return TRUE;
}

void note_remove(CHAR_DATA *ch, NOTE_DATA *pnote, bool delete)
{
    char to_new[MAX_INPUT_LENGTH];
    char to_one[MAX_INPUT_LENGTH];
    NOTE_DATA *prev;
    NOTE_DATA **list;
    const char *to_list;

    if (!delete)
    {
	/* make a new list */
        to_new[0]	= '\0';
        to_list	= pnote->to_list;
        while (*to_list != '\0')
        {
    	    to_list	= one_argument(to_list, to_one, sizeof(to_one));
    	    if (to_one[0] != '\0' && str_cmp(ch->name, to_one))
	    {
	        strcat(to_new, " ");
	        strcat(to_new, to_one);
	    }
        }
        /* Just a simple recipient removal? */
       if (str_cmp(ch->name, pnote->sender) && to_new[0] != '\0')
       {
	   free_string(pnote->to_list);
	   pnote->to_list = str_dup(to_new + 1);
	   return;
       }
    }
    /* nuke the whole note */

    switch(pnote->type)
    {
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
    {
	*list = pnote->next;
    }
    else
    {
	for (prev = *list; prev != NULL; prev = prev->next)
	{
	    if (prev->next == pnote)
		break;
	}

	if (prev == NULL)
	{
	    bug("Note_remove: pnote not found.", 0);
	    return;
	}

	prev->next = pnote->next;
    }

    save_notes(pnote->type);
    free_note(pnote);
    return;
}

bool hide_note(CHAR_DATA *ch, NOTE_DATA *pnote)
{
    time_t last_read;

    if (IS_NPC(ch))
	return TRUE;

    switch (pnote->type)
    {
	default:
	    return TRUE;
	case NOTE_NOTE:
	    last_read = ch->pcdata->last_note;
	    break;
	case NOTE_IDEA:
	    last_read = ch->pcdata->last_idea;
	    break;
	case NOTE_PENALTY:
	    last_read = ch->pcdata->last_penalty;
	    break;
	case NOTE_NEWS:
	    last_read = ch->pcdata->last_news;
	    break;
	case NOTE_CHANGES:
	    last_read = ch->pcdata->last_changes;
	    break;
    }
    
    if (pnote->date_stamp <= last_read)
	return TRUE;

    if (!str_cmp(ch->name,pnote->sender))
	return TRUE;

    if (!is_note_to(ch,pnote))
	return TRUE;

    return FALSE;
}

void update_read(CHAR_DATA *ch, NOTE_DATA *pnote)
{
    time_t stamp;

    if (IS_NPC(ch))
	return;

    stamp = pnote->date_stamp;

    switch (pnote->type)
    {
        default:
            return;
        case NOTE_NOTE:
	    ch->pcdata->last_note = UMAX(ch->pcdata->last_note,stamp);
            break;
        case NOTE_IDEA:
	    ch->pcdata->last_idea = UMAX(ch->pcdata->last_idea,stamp);
            break;
        case NOTE_PENALTY:
	    ch->pcdata->last_penalty = UMAX(ch->pcdata->last_penalty,stamp);
            break;
        case NOTE_NEWS:
	    ch->pcdata->last_news = UMAX(ch->pcdata->last_news,stamp);
            break;
        case NOTE_CHANGES:
	    ch->pcdata->last_changes = UMAX(ch->pcdata->last_changes,stamp);
            break;
    }
}

void print_note(BUFFER *buf, NOTE_DATA *pnote, int vnum)
{
	buf_printf(buf, "{x[%3d] From: %s, {x%s\n"
			"{x      To  : %s\n"
			"{x      Subj: %s\n"
			"{x%s\n{x",
		   vnum, pnote->sender, pnote->date,
		   pnote->to_list,
		   pnote->subject,
		   pnote->text);
}

const char * quote_note(NOTE_DATA *pnote)
{
	const char *p;
	char *q;
	char buf[MAX_STRING_LENGTH];
	bool need_quote;

	if (IS_NULLSTR(pnote->text))
		return str_dup(str_empty);

	snprintf(buf, sizeof(buf),
		 "On %s %s {xwrote to %s:\n"
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

void parse_note(CHAR_DATA *ch, const char *argument, int type)
{
	char arg[MAX_INPUT_LENGTH];
	NOTE_DATA *pnote;
	NOTE_DATA **list;
	char *list_name;
	int vnum;
	int anum;
	DESCRIPTOR_DATA *d;

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
					output = buf_new(-1);
					print_note(output, pnote, vnum);
					page_to_char(buf_string(output), ch);
					buf_free(output);
					update_read(ch, pnote);
					return;
				}
				else if (is_note_to(ch, pnote))
					vnum++;
			}
			char_puts("You have no unread messages.\n", ch);
			return;
        	}
		else if (is_number(argument)) {
			fAll = FALSE;
			anum = atoi(argument);
		}
		else {
			char_puts("Read which number?\n", ch);
			return;
		}
 
		vnum = 0;
		for (pnote = *list; pnote != NULL; pnote = pnote->next) {
			if (is_note_to(ch, pnote) && (vnum++ == anum || fAll)) {
				output = buf_new(-1);
				print_note(output, pnote, vnum-1);
				page_to_char(buf_string(output), ch);
				buf_free(output);
				if (!fAll)
					return;
			}
		}
 
		char_printf(ch,"There aren't that many %s.\n",list_name);
		return;
	}

	if (!str_prefix(arg, "list")) {
		char buf[MAX_INPUT_LENGTH];
		char from[MAX_INPUT_LENGTH];
		char to[MAX_INPUT_LENGTH];

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

				char_printf(ch, "[%3d%c] %s: %s\n{x",
					    vnum-1,
					    hide_note(ch, pnote) ? ' ' : 'N', 
					    pnote->sender, pnote->subject);
			}
		}
		return;
	}

	if (!str_prefix(arg, "remove")) {
		if (!is_number(argument)) {
			char_puts("Note remove which number?\n", ch);
			return;
		}
 
		anum = atoi(argument);
		vnum = 0;
		for (pnote = *list; pnote != NULL; pnote = pnote->next) {
			if (is_note_to(ch, pnote) && vnum++ == anum) {
				note_remove(ch, pnote, FALSE);
				char_puts("Ok.\n", ch);
				return;
			}
		}
 
		char_printf(ch, "There aren't that many %s.", list_name);
		return;
	}
 
	if (!str_prefix(arg, "delete") && ch->level >= MAX_LEVEL - 1) {
		if (!is_number(argument)) {
			char_puts("Note delete which number?\n", ch);
			return;
		}
 
		anum = atoi(argument);
		vnum = 0;
		for (pnote = *list; pnote != NULL; pnote = pnote->next) {
			if (is_note_to(ch, pnote) && vnum++ == anum) {
				note_remove(ch, pnote, TRUE);
				char_puts("Ok.\n", ch);
				return;
			}
		}

		char_printf(ch,"There aren't that many %s.", list_name);
		return;
	}

	if (!str_prefix(arg, "catchup")) {
		switch(type) {
		case NOTE_NOTE:	
			ch->pcdata->last_note = current_time;
			break;
		case NOTE_IDEA:
			ch->pcdata->last_idea = current_time;
			break;
		case NOTE_PENALTY:
			ch->pcdata->last_penalty = current_time;
			break;
		case NOTE_NEWS:
			ch->pcdata->last_news = current_time;
			break;
		case NOTE_CHANGES:
			ch->pcdata->last_changes = current_time;
			break;
		}
		return;
	}

/* below this point only certain people can edit notes */

	if ((type == NOTE_NEWS && !IS_TRUSTED(ch, ANGEL))
	||  (type == NOTE_CHANGES && !IS_TRUSTED(ch, CREATOR))) {
		char_printf(ch, "You aren't high enough level to write %s.",
			    list_name);
		return;
	}

	if (!str_prefix(arg, "edit")) {
		if (!note_attach(ch, type))
			return;
		string_append(ch, &ch->pnote->text);
		return;
	}

	if (!str_prefix(arg, "subject")) {
		if (!note_attach(ch, type))
			return;

		free_string(ch->pnote->subject);
		ch->pnote->subject = str_dup(argument);
		char_puts("Ok.\n", ch);
		return;
	}

	if (!str_prefix(arg, "to")) {
		if (!note_attach(ch, type))
			return;

		free_string(ch->pnote->to_list);
		ch->pnote->to_list = str_dup(argument);
		char_puts("Ok.\n", ch);
		return;
	}

	if (!str_prefix(arg, "forward")) {
		char buf[MAX_INPUT_LENGTH];

		argument = one_argument(argument, buf, sizeof(buf));
		if (!is_number(buf)) {
			char_puts("Forward which number?\n", ch);
			return;
		}

		anum = atoi(buf);
		vnum = 0;
		for (pnote = *list; pnote; pnote = pnote->next)
			if (is_note_to(ch, pnote) && vnum++ == anum)
				break;

		if (!pnote) {
			char_printf(ch, "There aren't that many %s.\n",
				    list_name);
			return;
		}

		if (!note_attach(ch, type))
			return;

		free_string(ch->pnote->text);
		ch->pnote->text = str_printf(
			"* Forwarded by: %s\n"
			"* Originally to: %s\n"
			"* Originally by: %s, %s\n"
			"\n"
			"---------- Forwarded message ----------\n"
			"%s",
			ch->name, pnote->to_list, pnote->sender, pnote->date,
			pnote->text);

		free_string(ch->pnote->subject);
		ch->pnote->subject = str_qdup(pnote->subject);

		string_append(ch, &ch->pnote->text);
		return;
	}

	if (!str_prefix(arg, "quote")) {
		char buf[MAX_INPUT_LENGTH];

		argument = one_argument(argument, buf, sizeof(buf));
		if (!is_number(buf)) {
			char_puts("Quote which number?\n", ch);
			return;
		}

		anum = atoi(buf);
		vnum = 0;
		for (pnote = *list; pnote; pnote = pnote->next)
			if (is_note_to(ch, pnote) && vnum++ == anum)
				break;

		if (!pnote) {
			char_printf(ch, "There aren't that many %s.\n",
				    list_name);
			return;
		}

		if (!note_attach(ch, type))
			return;

		free_string(ch->pnote->text);
		ch->pnote->text = quote_note(pnote);

		free_string(ch->pnote->subject);
		ch->pnote->subject = str_qdup(pnote->subject);

		string_append(ch, &ch->pnote->text);
		return;
	}

	if (!str_prefix(arg, "clear") || !str_prefix(arg, "cancel")) {
		if (ch->pnote) {
			free_note(ch->pnote);
			ch->pnote = NULL;
		}

		char_puts("Ok.\n", ch);
		return;
	}

	if (!str_prefix(arg, "show")) {
		BUFFER *output;

		if (!ch->pnote) {
			char_puts("You have no note in progress.\n", ch);
			return;
		}

		if (ch->pnote->type != type) {
			char_puts("You aren't working on that kind of note.\n",ch);
			return;
		}

		output = buf_new(-1);
		buf_printf(output, "{xFrom: %s\n"
				   "{xTo  : %s\n"
				   "{xSubj: %s\n"
				   "{x%s\n"
				   "{x",
			   ch->pnote->sender,
			   ch->pnote->to_list,
			   ch->pnote->subject,
			   ch->pnote->text);
		page_to_char(buf_string(output), ch);
		buf_free(output);

		return;
	}

	if (!str_prefix(arg, "post") || !str_prefix(arg, "send")) {
		if (ch->pnote == NULL) {
			char_puts("You have no note in progress.\n", ch);
			return;
		}

		if (ch->pnote->type != type) {
			char_puts("You aren't working on that kind of note.\n",
				  ch);
			return;
		}

		if (IS_NULLSTR(ch->pnote->to_list)) {
			char_puts("You need to provide a recipient "
				  "(name, clan name, all, or immortal).\n",
				  ch);
			return;
		}

		if (IS_NULLSTR(ch->pnote->subject)) {
			char_puts("You need to provide a subject.\n", ch);
			return;
		}

		if (IS_NULLSTR(ch->pnote->text)) {
			char_puts("You need to provide a text.\n", ch);
			return;
		}

		ch->pnote->next		= NULL;
		ch->pnote->date		= str_dup(strtime(current_time));
		ch->pnote->date_stamp	= current_time;

		append_note(ch->pnote);

		/* Show new note message */
		for (d = descriptor_list; d; d = d->next) {
			CHAR_DATA *fch = d->character;
			if (fch != NULL
			&&  fch != ch
			&&  is_note_to(fch, ch->pnote)
			&&  d->connected == CON_PLAYING)
				do_unread(fch, "login");
		}

		ch->pnote = NULL;
		return;
	}

	char_puts("You can't do that.\n", ch);
}

void fwrite_note(FILE *fp, NOTE_DATA *pnote)
{
	fwrite_string(fp, "Sender", pnote->sender);
	fwrite_string(fp, "Date", pnote->date);
	fprintf(fp, "Stamp %ld\n", pnote->date_stamp);
	fwrite_string(fp, "To", pnote->to_list);
	fwrite_string(fp, "Subject", pnote->subject);
	fwrite_string(fp, "Text", pnote->text);
}
