/*-
 * Copyright (c) 1999 SoG Development Team
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
 * $Id: db_hometown.c,v 1.13 2001-09-04 19:32:44 fjoe Exp $
 */

#include <stdio.h>

#include <merc.h>
#include <db.h>
#include <rwfile.h>

DECLARE_DBLOAD_FUN(load_hometown);

DECLARE_DBINIT_FUN(init_hometowns);

DBFUN dbfun_hometowns[] =
{
	{ "HOMETOWN",	load_hometown,	NULL	},		// notrans
	{ NULL, NULL, NULL }
};

DBDATA db_hometowns = { dbfun_hometowns, init_hometowns, 0 };

static void fread_altar(hometown_t *h, rfile_t *fp);
static void fread_recall(hometown_t *h, rfile_t *fp);
static void fread_map(hometown_t *h, rfile_t *fp);
static bool check_hometown(hometown_t *h);

DBINIT_FUN(init_hometowns)
{
	if (!DBDATA_VALID(dbdata))
		varr_init(&hometowns, &v_hometowns);
}

DBLOAD_FUN(load_hometown)
{
	hometown_t *h = varr_enew(&hometowns);

	for (;;) {
		bool fMatch = FALSE;

		fread_keyword(fp);
		switch (rfile_tokfl(fp)) {
		case 'A':
			SKEY("Area", h->area, fread_string(fp));
			if (IS_TOKEN(fp, "Altar")) {
				fread_altar(h, fp);
				fMatch = TRUE;
			}
			break;
		case 'E':
			if (IS_TOKEN(fp, "End")) {
				if (!check_hometown(h))
					hometowns.nused--;
				return;
			}
			break;
		case 'M':
			if (IS_TOKEN(fp, "Map")) {
				fread_map(h, fp);
				fMatch = TRUE;
			}
			break;
		case 'R':
			if (IS_TOKEN(fp, "Recall")) {
				fread_recall(h, fp);
				fMatch = TRUE;
			}
			SKEY("RestrictClass", h->restrict_class, fread_string(fp));
			SKEY("RestrictRace", h->restrict_race, fread_string(fp));
			KEY("RestrictAlign", h->restrict_align,
			    fread_fstring(ralign_names, fp));
			break;
		}

		if (!fMatch) {
			log(LOG_ERROR, "load_hometown: %s: Unknown keyword",
				 rfile_tok(fp));
			fread_to_eol(fp);
		}
	}
}

/*----------------------------------------------------------------------------
 * static functions
 */
static void
fread_altar(hometown_t *h, rfile_t *fp)
{
	int anum;
	int vnum;
	ROOM_INDEX_DATA *room;
	OBJ_INDEX_DATA *pit;

	anum = fread_fword(align_names, fp);

	vnum = fread_number(fp);
	if ((room = get_room_index(vnum)) == NULL) {
		log(LOG_ERROR, "fread_altar: %d: no such room", vnum);
		fread_to_eol(fp);
		return;
	}

	vnum = fread_number(fp);
	if ((pit = get_obj_index(vnum)) == NULL) {
		log(LOG_ERROR, "fread_altar: %d: no such obj", vnum);
		fread_to_eol(fp);
		return;
	}

	if (anum < 0) {
		int i;

		for (i = 0; i < MAX_AN; i++) {
			h->altar[i].room = room;
			h->altar[i].pit = pit;
		}
	} else {
		h->altar[anum].room = room;
		h->altar[anum].pit = pit;
	}
}

static void
fread_recall(hometown_t *h, rfile_t *fp)
{
	int anum;
	int vnum;
	ROOM_INDEX_DATA *room;

	anum = fread_fword(align_names, fp);

	vnum = fread_number(fp);
	if ((room = get_room_index(vnum)) == NULL) {
		log(LOG_ERROR, "fread_altar: %d: no such room", vnum);
		fread_to_eol(fp);
		return;
	}

	if (anum < 0) {
		int i;

		for (i = 0; i < MAX_AN; i++)
			h->recall[i] = room;
	} else
		h->recall[anum] = room;
}

static void
fread_map(hometown_t *h, rfile_t *fp)
{
	int anum;
	int vnum;
	OBJ_INDEX_DATA *obj;

	anum = fread_fword(align_names, fp);
	if ((vnum = fread_number(fp)) == 0)
		return;

	if ((obj = get_obj_index(vnum)) == NULL) {
		log(LOG_ERROR, "fread_altar: %d: no such obj", vnum);
		fread_to_eol(fp);
		return;
	}

	if (anum < 0) {
		int i;

		for (i = 0; i < MAX_AN; i++)
			h->map[i] = obj;
	} else
		h->map[anum] = obj;
}

static bool
check_hometown(hometown_t *h)
{
	int i;

	if (IS_NULLSTR(h->area)) {
		log(LOG_ERROR, "load_hometown: Area undefined");
		return FALSE;
	}

	for (i = 0; i < MAX_AN; i++) {
		if (!h->recall[i]) {
			log(LOG_ERROR, "load_hometown: Recall for '%s' undefined",
				 flag_string(align_names, i));
			return FALSE;
		}
		if (!h->altar[i].room) {
			log(LOG_ERROR, "load_hometown: Altar for '%s' undefined",
				 flag_string(align_names, i));
			return FALSE;
		}
		if (!h->altar[i].pit) {
			log(LOG_ERROR, "load_hometown: Pit for '%s' undefined",
				 flag_string(align_names, i));
			return FALSE;
		}
	}

	return TRUE;
}
