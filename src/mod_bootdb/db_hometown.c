/*-
 * Copyright (c) 1999 fjoe <fjoe@iclub.nsu.ru>
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
 * $Id: db_hometown.c,v 1.1 1999-03-10 17:30:35 fjoe Exp $
 */

#include <stdio.h>

#include "merc.h"
#include "db.h"

DECLARE_DBLOAD_FUN(load_hometown);

DBFUN dbfun_hometowns[] =
{
	{ "HOMETOWN",	load_hometown	},
	{ NULL }
};

DBDATA db_hometowns = { dbfun_hometowns };

static void fread_altar(hometown_t *h, FILE *fp);
static void fread_recall(hometown_t *h, FILE *fp);
static bool check_hometown(hometown_t *h);

DBLOAD_FUN(load_hometown)
{
	hometown_t *h = varr_enew(&hometowns);

	for (;;) {
		char *word = feof(fp) ? "End" : fread_word(fp);
		bool fMatch = FALSE;

		switch (UPPER(word[0])) {
		case 'A':
			SKEY("Area", h->area);
			if (!str_cmp(word, "Altar")) {
				fread_altar(h, fp);
				fMatch = TRUE;
			}
			break;
		case 'E':
			if (!str_cmp(word, "End")) {
				if (!check_hometown(h))
					hometowns.nused--;
				return;
			}
			break;
		case 'R':
			if (!str_cmp(word, "Recall")) {
				fread_recall(h, fp);
				fMatch = TRUE;
			}
			SKEY("RestrictClass", h->restrict_class);
			SKEY("RestrictRace", h->restrict_race);
			KEY("RestrictAlign", h->restrict_align,
			    fread_fstring(ralign_names, fp));
			break;
		}

		if (!fMatch)
			db_error("load_hometown", "%s: Unknown keyword", word);
	}
}

/*----------------------------------------------------------------------------
 * static functions
 */
static void fread_altar(hometown_t *h, FILE *fp)
{
	const char *align = fread_word(fp);
	int anum;
	ROOM_INDEX_DATA *room = get_room_index(fread_number(fp));
	OBJ_INDEX_DATA *pit = get_obj_index(fread_number(fp));

	anum = flag_value(align_names, align);
	if (anum < 0) {
		int i;

		for (i = 0; i < MAX_ANUM; i++) {
			h->altar[i].room = room;
			h->altar[i].pit = pit;
		}
	}
	else {
		h->altar[anum].room = room;
		h->altar[anum].pit = pit;
	}
}

static void fread_recall(hometown_t *h, FILE *fp)
{
	const char *align = fread_word(fp);
	int anum;
	ROOM_INDEX_DATA *room = get_room_index(fread_number(fp));

	anum = flag_value(align_names, align);
	if (anum < 0) {
		int i;

		for (i = 0; i < MAX_ANUM; i++)
			h->recall[i] = room;
	}
	else 
		h->recall[anum] = room;
}

static bool check_hometown(hometown_t *h)
{
	int i;

	if (IS_NULLSTR(h->area)) {
		db_error("load_hometown", "Area undefined");
		return FALSE;
	}

	for (i = 0; i < MAX_ANUM; i++) {
		if (!h->recall[i]) {
			db_error("load_hometown",
				 "Recall for '%s' undefined",
				 flag_string(align_names, i));
			return FALSE;
		}
		if (!h->altar[i].room) {
			db_error("load_hometown",
				 "Altar for '%s' undefined",
				 flag_string(align_names, i));
			return FALSE;
		}
		if (!h->altar[i].pit) {
			db_error("load_hometown",
				 "Pit for '%s' undefined",
				 flag_string(align_names, i));
			return FALSE;
		}
	}

	return TRUE;
}

