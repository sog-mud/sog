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
 * $Id: init_bootdb.c,v 1.28 2003-10-10 16:14:17 fjoe Exp $
 */

#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <compat/fnmatch.h>

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
static void fix_mprogs(void);
static void fix_practicers(void);

MODINIT_FUN(_module_load, m)
{
	bootdb_errors = 0;

	c_init(&mprogs, &c_info_mprogs);

	db_load_file(&db_system, ETC_PATH, SYSTEM_CONF);

	db_load_list(&db_langs, LANG_PATH, LANG_LIST);
	db_load_file(&db_glob_gmlstr, ETC_PATH, GLOB_GMLSTR_FILE);
	load_msgdb();

	db_load_file(&db_cmd, ETC_PATH, CMD_CONF);
	db_load_file(&db_socials, ETC_PATH, SOCIALS_CONF);
	db_load_file(&db_uhandlers, ETC_PATH, UHANDLERS_CONF);

	db_load_file(&db_skills, ETC_PATH, SKILLS_CONF);
	db_load_dir(&db_spec, SPEC_PATH, SPEC_EXT);
	db_load_file(&db_effects, ETC_PATH, EFFECTS_CONF);
	db_load_file(&db_skills, ETC_PATH, DAMTYPES_CONF);
	db_load_file(&db_materials, ETC_PATH, MATERIALS_CONF);
	db_load_file(&db_liquids, ETC_PATH, LIQUIDS_CONF);

	db_load_dir(&db_races, RACES_PATH, RACE_EXT);
	db_load_dir(&db_classes, CLASSES_PATH, CLASS_EXT);
	db_load_dir(&db_clans, CLANS_PATH, CLAN_EXT);

	load_mprogs();

	db_load_list(&db_areas, AREA_PATH, AREA_LIST);
	db_load_file(&db_hometowns, ETC_PATH, HOMETOWNS_CONF);
	db_load_file(&db_forms, ETC_PATH, FORMS_CONF);

	load_hints();

	if (bootdb_errors != 0) {
		printlog(LOG_ERROR, "%d errors found", bootdb_errors);
		exit(1);
	}

	fix_resets();
	fix_exits();
	fix_mprogs();
	fix_practicers();

	msgq_init(&msgq_immtalk, MSGQ_LEN_CHAN);

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

	c_init(&msgdb, &c_info_msgdb);

	line_number = 0;
	snprintf(bootdb_filename, sizeof(bootdb_filename), "%s%c%s",
		 ETC_PATH, PATH_SEPARATOR, MSGDB_FILE);
	if ((fp = rfile_open(ETC_PATH, MSGDB_FILE)) == NULL) {
		printlog(LOG_ERROR, "load_msgdb: %s", strerror(errno));
		return;
	}

	for (;;) {
		const char *key;
		mlstring *mlp;

		mlstr_init(&ml);
		mlstr_fread(fp, &ml);
		key = mlstr_mval(&ml);

		if (!strcmp(key, "$")) {
			mlstr_destroy(&ml);
			break;
		}

		if ((mlp = c_insert(&msgdb, key)) == NULL) {
			printlog(LOG_ERROR, "load_msgdb: %s: duplicate msg", key);
			mlstr_destroy(&ml);
			continue;
		}

		msgcnt++;
		mlstr_cpy(mlp, &ml);
		mlstr_destroy(&ml);
	}

	printlog(LOG_INFO, "load_msgdb: %d msgs loaded", msgcnt);
	rfile_close(fp);
}

static void
load_hints(void)
{
	rfile_t *fp;

	c_init(&hints, &c_info_hints);

	line_number = 0;
	snprintf(bootdb_filename, sizeof(bootdb_filename), "%s%c%s",
		 ETC_PATH, PATH_SEPARATOR, HINTS_FILE);

	if ((fp = rfile_open(ETC_PATH, HINTS_FILE)) == NULL) {
		printlog(LOG_ERROR, "load_hints: %s", strerror(errno));
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
	mprog_t *mp;
	const char *mp_name;
	int mp_type;

	fp = rfile_open(MPC_PATH, name);
	if (fp == NULL) {
		printlog(LOG_ERROR, "load_mprog: fopen: %s: %s",
		    name, strerror(errno));
		return;
	}

	/*
	 * find rightmost '.'
	 */
	q = strrchr(name, '.');
	if (q == NULL)
		q = strchr(name, '\0');

	mp_name = str_ndup(name, q - name);

	if ((q = strchr(name, '_')) == NULL) {
		printlog(LOG_ERROR, "load_mprog: %s: unable to determine mprog type (no underscores in name)", name);
		free_string(mp_name);
		goto bailout;
	}
	strlncpy(buf, name, sizeof(buf), q - name);

	if ((mp_type = flag_svalue(mprog_types, buf)) < 0) {
		printlog(LOG_ERROR, "load_mprog: %s: unknown type", buf);
		free_string(mp_name);
		goto bailout;
	}

	if ((mp = (mprog_t *) c_insert(&mprogs, mp_name)) == NULL) {
		printlog(LOG_ERROR, "load_mprog: %s: duplicate mprog", mp_name);
		free_string(mp_name);
		goto bailout;
	}

	mp->name = mp_name;
	mp->type = mp_type;
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

	if ((dirp = opendir(MPC_PATH)) == NULL) {
		printlog(LOG_ERROR, "load_mprogs: %s: %s",
		    MPC_PATH, strerror(errno));
		return;
	}

	snprintf(mask, sizeof(mask), "*%s", MPC_EXT);		// notrans

	for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
#if !defined(LINUX)
		if (dp->d_type != DT_REG)
			continue;
#endif

		if (fnmatch(mask, dp->d_name, 0) == FNM_NOMATCH)
			continue;

		load_mprog(dp->d_name);
	}

	closedir(dirp);
}

/*
 * Translate all room exits from virtual to real.
 * Has to be done after all rooms are read in.
 */
static void
fix_exits()
{
	ROOM_INDEX_DATA *room;

	C_FOREACH (room, &rooms) {
		int door;

		for (door = 0; door < MAX_DIR; door++) {
			EXIT_DATA *pexit;

			if ((pexit = room->exit[door]) == NULL)
				continue;

			pexit->to_room.r = get_room_index(pexit->to_room.vnum);
		}
	}
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
				printlog(LOG_BUG, "fix_resets_room: no 'E', 'G' or 'O' reset for obj vnum %d in area", r->arg3);
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
			printlog(LOG_INFO, "fix_resets_room: moving reset 'P 0 %d %d %d %d' to room %d", r->arg1, r->arg2, r->arg3, r->arg4, to_room->vnum);
		} else {
			to_room = room;
			printlog(LOG_INFO, "fix_resets_room: moving reset 'P 0 %d %d %d %d' inside room", r->arg1, r->arg2, r->arg3, r->arg4);
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

#if FIX_MPROGS
static trig_t *
trig_lookup_v(varr *v, const char *name, int *ptrigvnum)
{
	trig_t *trig;

	C_FOREACH(trig, v) {
		if (!strcmp(trig->trig_prog, name)) {
			*ptrigvnum = varr_index(v, trig);
			return trig;
		}
	}

	return NULL;
}

static trig_t *
trig_lookup_mob(const char *name, int *pvnum, int *ptrigvnum)
{
	int i;
	trig_t *rv = NULL;
	MOB_INDEX_DATA *mob;

	C_FOREACH (mob, &mobiles) {
		trig_t *trig;

		trig = trig_lookup_v(&mob->mp_trigs, name, ptrigvnum);
		if (trig != NULL) {
			if (rv != NULL)
				return NULL;

			*pvnum = mob->vnum;
			rv = trig;
		}
	}

	return rv;
}

static trig_t *
trig_lookup_obj(const char *name, int *pvnum, int *ptrigvnum)
{
	int i;
	trig_t *rv = NULL;
	OBJ_INDEX_DATA *obj;

	C_FOREACH (obj, &objects) {
		trig_t *trig;

		trig = trig_lookup_v(&obj->mp_trigs, name, ptrigvnum);
		if (trig != NULL) {
			if (rv != NULL)
				return NULL;

			*pvnum = obj->vnum;
			rv = trig;
		}
	}

	return rv;
}

static trig_t *
trig_lookup_room(const char *name, int *pvnum, int *ptrigvnum)
{
	ROOM_INDEX_DATA *room;
	trig_t *rv = NULL;

	C_FOREACH (room, &rooms) {
		trig_t *trig;

		trig = trig_lookup_v(&room->mp_trigs, name, ptrigvnum);
		if (trig != NULL) {
			if (rv != NULL)
				return NULL;

			*pvnum = room->vnum;
			rv = trig;
		}
	}

	return rv;
}
#endif

static void
fix_mprogs(void)
{
#ifdef FIX_MPROGS
	FILE *fp;
	mprog_t *mp;

	if ((fp = dfopen(TMP_PATH, "mprogs", "a")) == NULL) {
		printlog(LOG_ERROR, "%s: %s", __FUNCTION__, strerror(errno));
		return;
	}

	do {
		C_FOREACH(mp, &mprogs) {
			trig_t *trig;
			spec_t *spec = NULL;
			int vnum = 0;
			int trignum = 0;
			const char *mp_name;

			switch (mp->type) {
			case MP_T_MOB:
				trig = trig_lookup_mob(
				    mp->name, &vnum, &trignum);
				break;

			case MP_T_OBJ:
				trig = trig_lookup_obj(
				    mp->name, &vnum, &trignum);
				break;

			case MP_T_ROOM:
				trig = trig_lookup_room(
				    mp->name, &vnum, &trignum);
				break;

			case MP_T_SPEC:
				C_FOREACH(spec, &specs) {
					if (!strcmp(spec->mp_trig.trig_prog, mp->name))
						break;
				}

				if (spec == NULL) {
					printlog(LOG_ERROR, "%s: mprog %s: spec not found",
					    __FUNCTION__, mp->name);
					continue;
				}

				trig = &spec->mp_trig;
				break;

			default:
				continue;
			}

			if (trig == NULL)
				continue;

			if (trig->trig_prog[0] == '@')
				continue;

			if (spec != NULL) {
				mp_name = genmpname_str(
				    mp->type, spec->spec_name);
				SET_BIT(spec->spec_flags, SPF_CHANGED);
			} else {
				mp_name = genmpname_vnum(
				    mp->type, vnum, trignum);
			}

			if (mprog_lookup(mp_name) != NULL) {
				printlog(LOG_ERROR, "%s: %s: duplicate mprog",
				    __FUNCTION__, mp_name);
				continue;
			}

			c_move(&mprogs, mp->name, mp_name);
			free_string(mp->name);
			mp->name = str_dup(mp_name);

			printlog(LOG_INFO, "%s: %s -> %s",
			    __FUNCTION__, trig->trig_prog, mp->name);
			fprintf(fp, "mpc/%s.mpc\n", trig->trig_prog);

			free_string(trig->trig_prog);
			trig->trig_prog = str_qdup(mp->name);
			break;
		}
	} while (mp != NULL);

	fclose(fp);
#endif
}

#define V7_MOB_PRACTICE	(D)

static void
fix_practicers()
{
	AREA_DATA *area;

	for (area = area_first; area != NULL; area = area->next) {
		int vnum;

		if (area->ver >= 8)
			continue;

		for (vnum = area->min_vnum; vnum < area->max_vnum; vnum++) {
			MOB_INDEX_DATA *mob = get_mob_index(vnum);

			if (mob == NULL
			||  !IS_SET(mob->mob_flags, V7_MOB_PRACTICE))
				continue;

			printlog(LOG_INFO, "fix_practicers: vnum %d", vnum);
			REMOVE_BIT(mob->mob_flags, V7_MOB_PRACTICE);
			TOUCH_AREA(area);
			if (IS_SET(mob->mob_flags, MOB_CLAN_GUARD)) {
				if (IS_SET(mob->mob_flags, MOB_GAIN | MOB_TRAIN)) {
					REMOVE_BIT(mob->mob_flags,
					    MOB_GAIN | MOB_TRAIN);
					printlog(LOG_INFO,
					    "fix_practicers: vnum %d: MOB_GAIN and MOB_TRAIN removed", vnum);
				}
				continue;
			}

			SET_BIT(mob->mob_flags, MOB_GAIN | MOB_TRAIN);
			if (c_isempty(&mob->practicer)) {
				mob_add_practicer(mob, GROUP_NONE);
				printlog(LOG_INFO,
				     "fix_practicers: vnum %d: group none",
				     vnum);
			}
		}
	}
}
