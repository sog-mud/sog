/*-
 * Copyright (c) 2001 SoG Development Team
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
 * $Id: init_bootdb.c,v 1.6 2001-08-28 16:37:32 avn Exp $
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(BSD44)
#	include <fnmatch.h>
#else
#	include <compat/fnmatch.h>
#endif

#include <merc.h>
#include <db.h>
#include <lang.h>
#include <rwfile.h>
#include <mprog.h>

DECLARE_MODINIT_FUN(_module_load);
DECLARE_MODINIT_FUN(_module_unload);

static void load_msgdb(void);
static void load_hints(void);
static void load_mprogs(void);

static void fix_resets(void);
static void fix_exits(void);

MODINIT_FUN(_module_load, m)
{
	bootdb_errors = 0;

	db_load_file(&db_system, ETC_PATH, SYSTEM_CONF);

	db_load_list(&db_langs, LANG_PATH, LANG_LIST);
	db_load_file(&db_glob_gmlstr, ETC_PATH, GLOB_GMLSTR_FILE);
	load_msgdb();

	db_load_file(&db_cmd, ETC_PATH, CMD_CONF);
	db_load_file(&db_socials, ETC_PATH, SOCIALS_CONF);

	db_load_file(&db_skills, ETC_PATH, SKILLS_CONF);
	db_load_dir(&db_spec, SPEC_PATH, SPEC_EXT);
	db_load_file(&db_damtype, ETC_PATH, DAMTYPE_CONF);
	db_load_file(&db_materials, ETC_PATH, MATERIALS_CONF);
	db_load_file(&db_liquids, ETC_PATH, LIQUIDS_CONF);

	db_load_dir(&db_races, RACES_PATH, RACE_EXT);
	db_load_dir(&db_classes, CLASSES_PATH, CLASS_EXT);
	db_load_dir(&db_clans, CLANS_PATH, CLAN_EXT);
	db_load_list(&db_areas, AREA_PATH, AREA_LIST);
	db_load_file(&db_hometowns, ETC_PATH, HOMETOWNS_CONF);
	db_load_file(&db_forms, ETC_PATH, FORMS_CONF);

	load_hints();
	load_mprogs();

	if (bootdb_errors != 0) {
		log(LOG_ERROR, "%d errors found", bootdb_errors);
		exit(1);
	}

	fix_resets();
	fix_exits();

	return 0;
}

MODINIT_FUN(_module_unload, m)
{
	/* empty */
	return 0;
}

/*--------------------------------------------------------------------
 * local functions
 */

static void
load_msgdb(void)
{
	int msgcnt = 0;
	rfile_t *fp;
	mlstring ml;

	hash_init(&msgdb, &h_msgdb);

	line_number = 0;
	snprintf(bootdb_filename, sizeof(bootdb_filename), "%s%c%s",
		 ETC_PATH, PATH_SEPARATOR, MSGDB_FILE);
	if ((fp = rfile_open(ETC_PATH, MSGDB_FILE)) == NULL) {
		log(LOG_ERROR, "load_msgdb: %s", strerror(errno));
		return;
	}

	mlstr_init(&ml);
	for (;;) {
		const char *key;

		mlstr_fread(fp, &ml);
		key = mlstr_mval(&ml);

		if (!strcmp(key, "$"))
			break;

		if (!hash_insert(&msgdb, key, &ml))
			log(LOG_ERROR, "load_msgdb: %s: duplicate msg", key);
		else
			msgcnt++;
	}
	mlstr_destroy(&ml);

	log(LOG_INFO, "load_msgdb: %d msgs loaded", msgcnt);
	rfile_close(fp);
}

static void
load_hints(void)
{
	rfile_t *fp;

	varr_init(&hints, &v_hints);

	line_number = 0;
	snprintf(bootdb_filename, sizeof(bootdb_filename), "%s%c%s",
		 ETC_PATH, PATH_SEPARATOR, HINTS_FILE);

	if ((fp = rfile_open(ETC_PATH, HINTS_FILE)) == NULL) {
		log(LOG_ERROR, "load_hints: %s", strerror(errno));
		return;
	}

	for (;;) {
		flag_t hint_level = fread_fword(hint_levels, fp);
		hint_t *t;

		if (!hint_level)
			break;

		t = varr_enew(&hints);
		mlstr_fread(fp, &t->phrase);
		t->hint_level = hint_level;
	}

	rfile_close(fp);
}

static void
load_mprog(const char *name)
{
	char buf[MAX_INPUT_LENGTH];
	char *q;
	rfile_t *fp;
	mprog_t mprog;
	mprog_t *mp;

	fp = rfile_open(MPC_PATH, name);
	if (fp == NULL) {
		log(LOG_ERROR, "load_mprog: fopen: %s: %s",
		    name, strerror(errno));
		return;
	}

	/*
	 * find rightmost '.'
	 */
	q = strrchr(name, '.');
	if (q == NULL)
		q = strchr(name, '\0');

	mprog_init(&mprog);
	mprog.name = str_ndup(name, q - name);

	if ((q = strchr(name, '_')) == NULL) {
		log(LOG_ERROR, "load_mprog: %s: unable to determine mprog type (no underscores in name)", name);
		mprog_destroy(&mprog);
		goto bailout;
	}
	strnzncpy(buf, sizeof(buf), name, (size_t) (q - name));

	if ((mprog.type = flag_svalue(mprog_types, buf)) < 0) {
		log(LOG_ERROR, "load_mprog: %s: unknown type", buf);
		mprog_destroy(&mprog);
		goto bailout;
	}

	if ((mp = (mprog_t *) hash_insert(&mprogs, mprog.name, &mprog)) == NULL) {
		fprintf(stderr, "load_mprog: %s: duplicate mprog", mprog.name);
		mprog_destroy(&mprog);
		goto bailout;
	}

	mp->text = str_ndup(fp->p, fp->len);

bailout:
	rfile_close(fp);
}

static void
load_mprogs()
{
	struct dirent *dp;
	DIR *dirp;
	char mask[PATH_MAX];

	hash_init(&mprogs, &h_mprogs);

	if ((dirp = opendir(MPC_PATH)) == NULL) {
		log(LOG_ERROR, "load_mprogs: %s: %s",
		    MPC_PATH, strerror(errno));
		return;
	}

	snprintf(mask, sizeof(mask), "*%s", MPC_EXT);		// notrans

	for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
		if (dp->d_type != DT_REG)
			continue;

		if (fnmatch(mask, dp->d_name, 0) == FNM_NOMATCH)
			continue;

		load_mprog(dp->d_name);
	}

	closedir(dirp);
}

static void
fix_exits_room(ROOM_INDEX_DATA *room)
{
	int door;

	for (door = 0; door < MAX_DIR; door++) {
		EXIT_DATA *pexit;

		if ((pexit = room->exit[door]) == NULL)
			continue;

		pexit->to_room.r = get_room_index(pexit->to_room.vnum);
	}
}

/*
 * Translate all room exits from virtual to real.
 * Has to be done after all rooms are read in.
 */
static void
fix_exits(void)
{
	ROOM_INDEX_DATA *room;
	int iHash;

	for (iHash = 0; iHash < MAX_KEY_HASH; iHash++)
		for (room = room_index_hash[iHash]; room; room = room->next)
			fix_exits_room(room);
}

/*
 * lookup last obj ('E', 'G' or 'O') with specified vnum in room resets
 */
static RESET_DATA *
reset_EGO_lookup(ROOM_INDEX_DATA *room, int vnum)
{
	RESET_DATA *r;
	RESET_DATA *rv = NULL;

	for (r = room->reset_first; r != NULL; r = r->next) {
		switch (r->command) {
		case 'E':
		case 'G':
		case 'O':
			if (r->arg1 == vnum)
				rv = r;
			break;
		}
	}

	return rv;
}

static bool
reset_in_EGO(RESET_DATA *after, RESET_DATA *reset)
{
	RESET_DATA *r;

	for (r = after->next; r != NULL && r->command == 'P'; r = r->next) {
		if (r == reset)
			return TRUE;
	}

	return FALSE;
}

/*
 * fix up old resets
 *
 * new resets scheme put obj in last obj
 * old scheme specify the obj to put into by vnum
 */
static void
fix_resets_room(ROOM_INDEX_DATA *room)
{
	RESET_DATA *r, *r_next;

restart:
	for (r = room->reset_first; r != NULL; r = r_next) {
		ROOM_INDEX_DATA *to_room = NULL;
		RESET_DATA *after;

		r_next = r->next;
		if (r->command != 'P' || r->arg3 == 0)
			continue;

		/*
		 * try to find in current room
		 */
		after = reset_EGO_lookup(room, r->arg3);
		if (!after) {
			/*
			 * search in area
			 */
			int min_vnum = room->area->min_vnum;
			int max_vnum = room->area->max_vnum;
			int vnum;

			for (vnum = min_vnum; vnum <= max_vnum; vnum++) {
				ROOM_INDEX_DATA *vrooom; /* yeah, vrooom */

				if ((vrooom = get_room_index(vnum)) != NULL
				&&  (after = reset_EGO_lookup(vrooom, r->arg3)))
					to_room = vrooom;
			}

			if (to_room == NULL) {
				log(LOG_BUG, "fix_resets_room: no 'E', 'G' or 'O' reset for obj vnum %d in area", r->arg3);
				continue;
			}
		} else if (reset_in_EGO(after, r)) {
			/*
			 * nothing to be done
			 */
			continue;
		}

		TOUCH_AREA(room->area);
		if (to_room) {
			log(LOG_INFO, "fix_resets_room: moving reset 'P 0 %d %d %d %d' to room %d", r->arg1, r->arg2, r->arg3, r->arg4, to_room->vnum);
		} else {
			to_room = room;
			log(LOG_INFO, "fix_resets_room: moving reset 'P 0 %d %d %d %d' inside room", r->arg1, r->arg2, r->arg3, r->arg4);
		}

		reset_del(room, r);
		reset_add(to_room, r, after);

		if (to_room == room)
			goto restart;
	}
}

static void
fix_resets(void)
{
	AREA_DATA *area;

	for (area = area_first; area != NULL; area = area->next) {
		int vnum;

		for (vnum = area->min_vnum; vnum <= area->max_vnum; vnum++) {
			ROOM_INDEX_DATA *room;

			if ((room = get_room_index(vnum)) != NULL)
				fix_resets_room(room);
		}
	}
}
