/*
 * $Id: note.c,v 1.69 2003-09-30 00:31:38 fjoe Exp $
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__FreeBSD__)
#include <osreldate.h>
#endif

#include <merc.h>
#include <note.h>
#include <rwfile.h>

static void load_thread(const char *name, note_t **list,
			int type, time_t free_time);

note_t *note_list;
note_t *idea_list;
note_t *penalty_list;
note_t *news_list;
note_t *changes_list;

static note_t *note_free;

note_t *new_note()
{
	note_t *note;

	if (note_free == NULL)
		note = malloc(sizeof(*note));
	else {
		note = note_free;
		note_free = note_free->next;
	}
	memset(note, 0, sizeof(*note));
	return note;
}

void free_note(note_t *note)
{
	free_string(note->text  );
	free_string(note->subject);
	free_string(note->to_list);
	free_string(note->date  );
	free_string(note->sender);

	note->next = note_free;
	note_free   = note;
}

void save_notes(int type)
{
	FILE *fp;
	const char *name;
	note_t *pnote;

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

	if ((fp = dfopen(NOTES_PATH, name, "w")) == NULL)
		return;

	for (; pnote; pnote = pnote->next)
		fwrite_note(fp, pnote);
	fclose(fp);
}

void load_notes(void)
{
	load_thread(NOTE_FILE, &note_list, NOTE_NOTE, 14*24*60*60);
	load_thread(IDEA_FILE, &idea_list, NOTE_IDEA, 28*24*60*60);
	load_thread(PENALTY_FILE, &penalty_list, NOTE_PENALTY, 0);
	load_thread(NEWS_FILE, &news_list, NOTE_NEWS, 0);
	load_thread(CHANGES_FILE, &changes_list,NOTE_CHANGES, 0);
}

void fwrite_note(FILE *fp, note_t *pnote)
{
	fprintf(fp, "Sender  %s~\n", fix_string(pnote->sender));
	fprintf(fp, "Date    %s~\n", fix_string(pnote->date));
#if defined (__FreeBSD__) && __FreeBSD_version >= 500000
	fprintf(fp, "Stamp   %d\n", pnote->date_stamp);
#else
	fprintf(fp, "Stamp   %ld\n", pnote->date_stamp);
#endif
	fprintf(fp, "To      %s~\n", fix_string(pnote->to_list));
	fprintf(fp, "Subject %s~\n", fix_string(pnote->subject));
	fprintf(fp, "Text\n%s~\n",   fix_string(pnote->text));
}

static void load_thread(const char *name, note_t **list,
			int type, time_t free_time)
{
	rfile_t *fp;
	note_t *pnotelast;

	if (!dfexist(NOTES_PATH, name))
		return;

	if ((fp = rfile_open(NOTES_PATH, name)) == NULL)
		return;

	pnotelast = NULL;
	for (; ;) {
		note_t *pnote;

		fread_letter(fp);
		if (rfile_feof(fp)) {
			rfile_close(fp);
			return;
		}
		xungetc(fp);

		pnote = new_note();

		fread_word(fp);
		if (!IS_TOKEN(fp, "sender"))
			break;
		pnote->sender = fread_string(fp);

		fread_word(fp);
		if (!IS_TOKEN(fp, "date"))
			break;
		pnote->date = fread_string(fp);

		fread_word(fp);
		if (!IS_TOKEN(fp, "stamp"))
			break;
		pnote->date_stamp = fread_number(fp);

		fread_word(fp);
		if (!IS_TOKEN(fp, "to"))
			break;
		pnote->to_list = fread_string(fp);

		fread_word(fp);
		if (!IS_TOKEN(fp, "subject"))
			break;
		pnote->subject  = fread_string(fp);

		fread_word(fp);
		if (!IS_TOKEN(fp, "text"))
			break;
		pnote->text = fread_string(fp);

		if (free_time && pnote->date_stamp < current_time - free_time) {
			free_note(pnote);
			continue;
		}

		pnote->type = type;

		if (*list == NULL)
			*list = pnote;
		else
			pnotelast->next = pnote;

		pnotelast = pnote;
	}

	printlog(LOG_ERROR, "load_notes: %s: %s: bad keyword", name, rfile_tok(fp));
}
