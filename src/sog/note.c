/*
 * $Id: note.c,v 1.17 1998-08-14 05:45:15 fjoe Exp $
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
#include "recycle.h"
#include "db.h"
#include "comm.h"
#include "log.h"
#include "resource.h"
#include "buffer.h"
#include "string_edit.h"

/* globals from db.c for load_notes */
extern  int     _filbuf         args((FILE *));
extern FILE *                  fpArea;
extern char                    strArea[MAX_INPUT_LENGTH];

/* local procedures */
void load_thread(char *name, NOTE_DATA **list, int type, time_t free_time);
void parse_note(CHAR_DATA *ch, const char *argument, int type);
bool hide_note(CHAR_DATA *ch, NOTE_DATA *pnote);

NOTE_DATA *note_list;
NOTE_DATA *idea_list;
NOTE_DATA *penalty_list;
NOTE_DATA *news_list;
NOTE_DATA *changes_list;

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
	char_printf(ch,"There %s %d new news article%s waiting.\n\r",
	    count > 1 ? "are" : "is",count, count > 1 ? "s" : "");
    }
    if ((count = count_spool(ch,changes_list)) > 0)
    {
	found = TRUE;
	char_printf(ch,"There %s %d change%s waiting to be read.\n\r",
	    count > 1 ? "are" : "is", count, count > 1 ? "s" : "");
    }
    if ((count = count_spool(ch,note_list)) > 0)
    {
	found = TRUE;
	char_printf(ch,"You have %d new note%s waiting.\n\r",
	    count, count > 1 ? "s" : "");
    }
    if ((count = count_spool(ch,idea_list)) > 0)
    {
	found = TRUE;
	char_printf(ch,"You have %d unread idea%s to peruse.\n\r",
	    count, count > 1 ? "s" : "");
    }
    if (IS_TRUSTED(ch,ANGEL) && (count = count_spool(ch,penalty_list)) > 0)
    {
	found = TRUE;
	char_printf(ch,"%d %s been added.\n\r",
	    count, count > 1 ? "penalties have" : "penalty has");
    }

    if (!found && str_cmp(argument, "login"))
	send_to_char("You have no unread notes.\n\r",ch);
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

    switch (type)
    {
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
    if ((fp = fopen(name, "w")) == NULL)
	perror(name);
    else {
	for (; pnote != NULL; pnote = pnote->next) {
	    fprintf(fp, "Sender  %s~\n", pnote->sender);
	    fprintf(fp, "Date    %s~\n", pnote->date);
	    fprintf(fp, "Stamp   %ld\n", pnote->date_stamp);
	    fprintf(fp, "To      %s~\n", pnote->to_list);
	    fprintf(fp, "Subject %s~\n", pnote->subject);
	    fprintf(fp, "Text\n%s~\n",   pnote->text);
	}
	fclose(fp);
	fpReserve = fopen(NULL_FILE, "r");
   	return;
    }
}

void load_notes(void)
{
    load_thread(NOTE_FILE,&note_list, NOTE_NOTE, 14*24*60*60);
    load_thread(IDEA_FILE,&idea_list, NOTE_IDEA, 28*24*60*60);
    load_thread(PENALTY_FILE,&penalty_list, NOTE_PENALTY, 0);
    load_thread(NEWS_FILE,&news_list, NOTE_NEWS, 0);
    load_thread(CHANGES_FILE,&changes_list,NOTE_CHANGES, 0);
}

void load_thread(char *name, NOTE_DATA **list, int type, time_t free_time)
{
    FILE *fp;
    NOTE_DATA *pnotelast;
 
    if ((fp = fopen(name, "r")) == NULL)
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
 
    strcpy(strArea, NOTE_FILE);
    fpArea = fp;
    bug("Load_notes: bad key word.", 0);
    exit(1);
    return;
}

void append_note(NOTE_DATA *pnote)
{
    FILE *fp;
    char *name;
    NOTE_DATA **list;
    NOTE_DATA *last;

    switch(pnote->type)
    {
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
	for (last = *list; last->next != NULL; last = last->next);
	last->next = pnote;
    }

    fclose(fpReserve);
    if ((fp = fopen(name, "a")) == NULL)
        perror(name);
    else {
        fprintf(fp, "Sender  %s~\n", pnote->sender);
        fprintf(fp, "Date    %s~\n", pnote->date);
        fprintf(fp, "Stamp   %ld\n", pnote->date_stamp);
        fprintf(fp, "To      %s~\n", pnote->to_list);
        fprintf(fp, "Subject %s~\n", pnote->subject);
        fprintf(fp, "Text\n%s~\n", pnote->text);
        fclose(fp);
    }
    fpReserve = fopen(NULL_FILE, "r");
}

bool is_note_to(CHAR_DATA *ch, NOTE_DATA *pnote)
{
    if (!str_cmp(ch->name, pnote->sender))
	return TRUE;

    if (!str_cmp("all", pnote->to_list))
	return TRUE;

    if (IS_IMMORTAL(ch) && is_name("imm", pnote->to_list))
	return TRUE;

    if (is_name(ch->name, pnote->to_list))
	return TRUE;

    if (is_name(clan_table[ch->clan].short_name, pnote->to_list))
	return TRUE;

    return FALSE;
}

void note_attach(CHAR_DATA *ch, int type)
{
    NOTE_DATA *pnote;

    if (ch->pnote != NULL)
	return;

    pnote = new_note();

    pnote->next		= NULL;
    pnote->sender	= str_dup(ch->name);
    pnote->date		= str_dup("");
    pnote->to_list	= str_dup("");
    pnote->subject	= str_dup("");
    pnote->text		= str_dup("");
    pnote->type		= type;
    ch->pnote		= pnote;
    return;
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
    	    to_list	= one_argument(to_list, to_one);
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

    switch(type)
    {
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

    argument = one_argument(argument, arg);

    if (arg[0] == '\0' || !str_prefix(arg, "read"))
    {
        bool fAll;
 
        if (!str_cmp(argument, "all"))
        {
            fAll = TRUE;
            anum = 0;
        }
 
        else if (argument[0] == '\0' || !str_prefix(argument, "next"))
        /* read next unread note */
        {
            vnum = 0;
            for (pnote = *list; pnote != NULL; pnote = pnote->next)
            {
                if (!hide_note(ch,pnote))
                {
                    char_printf(ch, "[%3d] %s: %s\n\r%s\n\rTo: %s\n\r",
                        vnum,
                        pnote->sender,
                        pnote->subject,
                        pnote->date,
                        pnote->to_list);
                    page_to_char(pnote->text, ch);
                    update_read(ch,pnote);
                    return;
                }
                else if (is_note_to(ch,pnote))
                    vnum++;
            }
	    char_printf(ch,"You have no unread %s.\n\r",list_name);
            return;
        }
 
        else if (is_number(argument))
        {
            fAll = FALSE;
            anum = atoi(argument);
        }
        else
        {
            send_to_char("Read which number?\n\r", ch);
            return;
        }
 
        vnum = 0;
        for (pnote = *list; pnote != NULL; pnote = pnote->next)
        {
            if (is_note_to(ch, pnote) && (vnum++ == anum || fAll))
            {
                char_printf(ch, "[%3d] %s: %s\n\r%s\n\rTo: %s\n\r",
                    vnum - 1,
                    pnote->sender,
                    pnote->subject,
                    pnote->date,
                    pnote->to_list
                   );
                page_to_char(pnote->text, ch);
                return;
            }
        }
 
	char_printf(ch,"There aren't that many %s.\n\r",list_name);
        return;
    }

    if (!str_prefix(arg, "edit")) {
	note_attach(ch,type);
	if (ch->pnote->type != type)
	{
	    send_to_char(
		"You already have a different note in progress.\n\r",ch);
	    return;
	}

	string_append(ch, &ch->pnote->text);
	return;
    }

    if (!str_prefix(arg, "list"))
    {
	vnum = 0;
	for (pnote = *list; pnote != NULL; pnote = pnote->next)
	{
	    if (is_note_to(ch, pnote))
	    {
		char_printf(ch, "[%3d%s] %s: %s\n\r",
		    vnum, hide_note(ch,pnote) ? " " : "N", 
		    pnote->sender, pnote->subject);
		vnum++;
	    }
	}
	return;
    }

    if (!str_prefix(arg, "remove"))
    {
        if (!is_number(argument))
        {
            send_to_char("Note remove which number?\n\r", ch);
            return;
        }
 
        anum = atoi(argument);
        vnum = 0;
        for (pnote = *list; pnote != NULL; pnote = pnote->next) {
            if (is_note_to(ch, pnote) && vnum++ == anum) {
                note_remove(ch, pnote, FALSE);
                char_nputs(MSG_OK, ch);
                return;
            }
        }
 
	char_printf(ch, "There aren't that many %s.",list_name);
        return;
    }
 
    if (!str_prefix(arg, "delete") && get_trust(ch) >= MAX_LEVEL - 1) {
        if (!is_number(argument)) {
            send_to_char("Note delete which number?\n\r", ch);
            return;
        }
 
        anum = atoi(argument);
        vnum = 0;
        for (pnote = *list; pnote != NULL; pnote = pnote->next) {
            if (is_note_to(ch, pnote) && vnum++ == anum) {
                note_remove(ch, pnote,TRUE);
                char_nputs(MSG_OK, ch);
                return;
            }
        }

 	char_printf(ch,"There aren't that many %s.",list_name);
        return;
    }

    if (!str_prefix(arg,"catchup"))
    {
	switch(type)
	{
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
    if ((type == NOTE_NEWS && !IS_TRUSTED(ch,ANGEL))
    ||  (type == NOTE_CHANGES && !IS_TRUSTED(ch,CREATOR)))
    {
	char_printf(ch,"You aren't high enough level to write %s.",list_name);
	return;
    }

    if (!str_prefix(arg, "subject")) {
	note_attach(ch,type);
        if (ch->pnote->type != type) {
            send_to_char(
                "You already have a different note in progress.\n\r",ch);
            return;
        }

	free_string(ch->pnote->subject);
	ch->pnote->subject = str_dup(argument);
	char_nputs(MSG_OK, ch);
	return;
    }

    if (!str_prefix(arg, "to")) {
	note_attach(ch,type);
        if (ch->pnote->type != type) {
            send_to_char(
                "You already have a different note in progress.\n\r",ch);
            return;
        }
	free_string(ch->pnote->to_list);
	ch->pnote->to_list = str_dup(argument);
	char_nputs(MSG_OK, ch);
	return;
    }

    if (!str_prefix(arg, "clear")) {
	if (ch->pnote != NULL) {
	    free_note(ch->pnote);
	    ch->pnote = NULL;
	}

	char_nputs(MSG_OK, ch);
	return;
    }

    if (!str_prefix(arg, "show")) {
	if (ch->pnote == NULL) {
	    send_to_char("You have no note in progress.\n\r", ch);
	    return;
	}

	if (ch->pnote->type != type) {
	    send_to_char("You aren't working on that kind of note.\n\r",ch);
	    return;
	}

	char_printf(ch, "%s: %s\n\rTo: %s\n\r",
	    ch->pnote->sender,
	    ch->pnote->subject,
	    ch->pnote->to_list
	   );
	send_to_char(ch->pnote->text, ch);
	return;
    }

    if (!str_prefix(arg, "post") || !str_prefix(arg, "send")) {
	char *strtime;

	if (ch->pnote == NULL) {
	    send_to_char("You have no note in progress.\n\r", ch);
	    return;
	}

        if (ch->pnote->type != type) {
            send_to_char("You aren't working on that kind of note.\n\r",ch);
            return;
        }

	if (!str_cmp(ch->pnote->to_list,"")) {
	    send_to_char(
		"You need to provide a recipient (name, all, or immortal).\n\r",
		ch);
	    return;
	}

	if (!str_cmp(ch->pnote->subject,"")) {
	    send_to_char("You need to provide a subject.\n\r",ch);
	    return;
	}

	ch->pnote->next			= NULL;
	strtime				= ctime(&current_time);
	strtime[strlen(strtime)-1]	= '\0';
	ch->pnote->date			= str_dup(strtime);
	ch->pnote->date_stamp		= current_time;

	append_note(ch->pnote);

	/* Show new note message */
	for (d = descriptor_list; d; d = d->next) {
		CHAR_DATA *fch = d->character;
		if (fch != NULL
		&&  fch != ch
		&&  is_note_to(fch, ch->pnote)
		&&  d->connected == CON_PLAYING)
			char_puts("You have new note.\n\r", fch);
	}

	ch->pnote = NULL;
	return;
    }

    char_nputs(MSG_YOU_CANT_DO_THAT, ch);
    return;
}

