/*
 * $Id: db.c,v 1.251 2001-07-31 14:56:29 fjoe Exp $
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

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#if	defined (WIN32)
#	include <compat/compat.h>
#else
#	include <dirent.h>
#endif

#include <merc.h>
#include <socials.h>
#include <db.h>
#include <module.h>
#include <lang.h>
#include <note.h>
#include <ban.h>
#include <rfile.h>
#include <dynafun.h>

#if defined(BSD44)
#	include <fnmatch.h>
#else
#	include "../compat/fnmatch.h"
#endif

#ifdef SUNOS
#	include "compat.h"
#	define d_namlen d_reclen
#endif

#ifdef SVR4
#	define d_namlen d_reclen
#endif

#if !defined(OLD_RAND)

#if defined(BSD44) || defined(LINUX)
#	include <unistd.h>
#elif defined(WIN32)
#	define random()		rand()
#	define srandom( x ) srand( x )
	int getpid();
	time_t time(time_t *tloc);
#endif

#endif

#if !defined(MPC)
/*
 * Globals.
 */
const char TMP_PATH		[] = "tmp";		// notrans
const char PLAYER_PATH		[] = "player";		// notrans
const char GODS_PATH		[] = "gods";		// notrans
const char NOTES_PATH		[] = "notes";		// notrans
const char ETC_PATH		[] = "etc";		// notrans
const char AREA_PATH		[] = "area";		// notrans
const char LANG_PATH		[] = "lang";		// notrans
const char MODULES_PATH		[] = "modules";		// notrans
const char MPC_PATH		[] = "mpc";		// notrans

const char CLASSES_PATH		[] = "classes";		// notrans
const char CLANS_PATH		[] = "clans";		// notrans
const char RACES_PATH		[] = "races";		// notrans
const char SPEC_PATH		[] = "specs";		// notrans

const char RACE_EXT		[] = ".race";		// notrans
const char CLASS_EXT		[] = ".class";		// notrans
const char CLAN_EXT		[] = ".clan";		// notrans
const char SPEC_EXT		[] = ".spec";		// notrans
const char MPC_EXT		[] = ".mpc";		// notrans

#if defined (WIN32)
const char PLISTS_PATH		[] = "clans\\plists";	// notrans
const char NULL_FILE		[] = "NUL";		// notrans
#else
const char PLISTS_PATH		[] = "clans/plists";	// notrans
const char NULL_FILE		[] = "/dev/null";	// notrans
#endif

const char TMP_FILE		[] = "romtmp";		// notrans

const char HOMETOWNS_CONF	[] = "hometowns.conf";	// notrans
const char SKILLS_CONF		[] = "skills.conf";	// notrans
const char SOCIALS_CONF		[] = "socials.conf";	// notrans
const char SYSTEM_CONF		[] = "system.conf";	// notrans
const char LANG_CONF		[] = "lang.conf";	// notrans
const char CMD_CONF		[] = "cmd.conf";	// notrans
const char DAMTYPE_CONF		[] = "damtype.conf";	// notrans
const char MATERIALS_CONF	[] = "materials.conf";	// notrans
const char LIQUIDS_CONF		[] = "liquids.conf";	// notrans
const char FORMS_CONF		[] = "forms.conf";	// notrans
const char CC_EXPR_CONF		[] = "cc_expr.conf";	// notrans
const char UHANDLERS_CONF	[] = "uhandlers.conf";	// notrans

const char GLOB_GMLSTR_FILE	[] = "glob_gmlstr";	// notrans
const char MSGDB_FILE		[] = "msgdb";		// notrans
const char HINTS_FILE		[] = "hints";		// notrans

const char AREA_LIST		[] = "area.lst";	// notrans
const char LANG_LIST		[] = "lang.lst";	// notrans

const char NOTE_FILE		[] = "notes.not";	// notrans
const char IDEA_FILE		[] = "ideas.not";	// notrans
const char PENALTY_FILE		[] = "penal.not";	// notrans
const char NEWS_FILE		[] = "news.not";	// notrans
const char CHANGES_FILE		[] = "chang.not";	// notrans
const char SHUTDOWN_FILE	[] = "shutdown";	// notrans
const char EQCHECK_FILE		[] = "eqcheck";		// notrans
const char EQCHECK_SAVE_ALL_FILE[] = "eqcheck_save_all";// notrans

const char BAN_FILE		[] = "ban.txt";		// notrans
const char MAXON_FILE		[] = "maxon.txt";	// notrans
const char AREASTAT_FILE	[] = "areastat.txt";	// notrans
const char IMMLOG_FILE		[] = "immortals.log";	// notrans

flag_t		mud_options;

SHOP_DATA *		shop_first;
SHOP_DATA *		shop_last;

CHAR_DATA *		char_list;
CHAR_DATA *		char_list_lastpc;

OBJ_DATA *		object_list;
TIME_INFO_DATA		time_info;
WEATHER_DATA		weather_info;

ROOM_INDEX_DATA	*	top_affected_room = NULL;
CHAR_DATA	*	top_affected_char = NULL;
OBJ_DATA	*	top_affected_obj = NULL;

int			reboot_counter = 1440;
int			rebooter = 0;

/*
 * Locals.
 */
MOB_INDEX_DATA *	mob_index_hash		[MAX_KEY_HASH];
OBJ_INDEX_DATA *	obj_index_hash		[MAX_KEY_HASH];
ROOM_INDEX_DATA *	room_index_hash		[MAX_KEY_HASH];

AREA_DATA *		area_first;
AREA_DATA *		area_last;

int			top_affect;
int			top_area;
int			top_ed;
int			top_exit;
int			top_help;
int			top_mob_index;
int			top_obj_index;
int			top_reset;
int			top_room;
int			top_shop;
int                     top_vnum_room;		/* OLC */
int                     top_vnum_mob;		/* OLC */
int                     top_vnum_obj;		/* OLC */
int			top_mprog_index;	/* OLC */
int			top_player;

const char *dir_name[] =
{
	"north", "east", "south", "west", "up", "down"
};

const char *from_dir_name[] =
{
	"the north", "the east", "the south", "the west", "the up", "the down"
};

const int rev_dir[] =
{
	2, 3, 0, 1, 5, 4
};

/*
 * Semi-locals.
 */
char	filename[PATH_MAX];
int	changed_flags;		/* changed object flags for OLC */
hash_t	glob_gmlstr;
hash_t	msgdb;
varr	hints;

static void load_msgdb(void);
static void load_hints(void);

/*
 * Local booting procedures.
 */
#ifdef OLD_RAND
static void	init_mm         (void);
#endif

static void	fix_resets	(void);
static void	fix_exits	(void);

DECLARE_DBLOAD_FUN(load_glob_gmlstr);
DECLARE_DBINIT_FUN(init_glob_gmlstr);

DBFUN dbfun_glob_gmlstr[] = {
	{ "GLOB",	load_glob_gmlstr,	NULL	},	// notrans
	{ NULL, NULL, NULL }
};

DBDATA db_glob_gmlstr = { dbfun_glob_gmlstr, init_glob_gmlstr, 0 };

void dbdata_init(DBDATA *dbdata)
{
	dbdata->tab_sz = 0;
	if (dbdata->dbinit)
		dbdata->dbinit(dbdata);
	while(dbdata->fun_tab[dbdata->tab_sz].name)
		dbdata->tab_sz++;
	qsort(dbdata->fun_tab, dbdata->tab_sz,
	      sizeof(*dbdata->fun_tab), cmpstr);
}

DBFUN *dbfun_lookup(DBDATA *dbdata, const char *name)
{
	return bsearch(&name, dbdata->fun_tab, dbdata->tab_sz,
		       sizeof(*dbdata->fun_tab), cmpstr);
}

void db_set_arg(DBDATA *dbdata, const char *name, void *arg)
{
	DBFUN *fun;

	if (!dbdata->tab_sz)
		dbdata_init(dbdata);

	if ((fun = dbfun_lookup(dbdata, name)) == NULL)
		return;

	fun->arg = arg;
}

static void
logger_db(const char *buf)
{
	char buf2[MAX_STRING_LENGTH];

	snprintf(buf2, sizeof(buf2), "%s: line %d: %s",		// notrans
		 filename, line_number, buf);
	logger_default(buf2);
}

/*
 * db_parse_file - parses file using dbdata
 * dbdata->tab_sz should be properly intialized
 */
void db_parse_file(DBDATA *dbdata, const char *path, const char *file)
{
	char buf[PATH_MAX];
	int linenum;
	rfile_t *fp;
	logger_t logger_old;

	strnzcpy(buf, sizeof(buf), filename);
	linenum = line_number;
	line_number = 1;
	snprintf(filename, sizeof(filename), "%s%c%s",
		 path, PATH_SEPARATOR, file);

	if ((fp = rfile_open(path, file)) == NULL) {
		log(LOG_ERROR, "db_parse_file: %s", strerror(errno));
		goto bail_out;
		return;
	}

	logger_old = logger_set(LOG_ERROR, logger_db);
	for (; ;) {
		DBFUN *fn;

		if (fread_letter(fp) != '#') {
			log(LOG_ERROR, "db_parse_file: '#' not found");
			break;
		}

		fread_word(fp);
		if (IS_TOKEN(fp, "$"))
			break;

		fn = dbfun_lookup(dbdata, rfile_tok(fp));
		if (fn)
			fn->fun(dbdata, fp, fn->arg);
		else {
			log(LOG_ERROR, "db_parse_file: bad section name");
			break;
		}
	}
	logger_set(LOG_ERROR, logger_old);
	rfile_close(fp);

bail_out:
	strnzcpy(filename, sizeof(filename), buf);
	line_number = linenum;
}

void db_load_file(DBDATA *dbdata, const char *path, const char *file)
{
	if (!dbdata->tab_sz)
		dbdata_init(dbdata);
	if (dbdata->dbinit)
		dbdata->dbinit(dbdata);
	db_parse_file(dbdata, path, file);
}

void db_load_dir(DBDATA *dbdata, const char *path, const char *ext)
{
	struct dirent *dp;
	DIR *dirp;
	char mask[PATH_MAX];

	if ((dirp = opendir(path)) == NULL) {
		log(LOG_ERROR, "db_load_dir: %s: %s", path, strerror(errno));
		return;
	}

	if (!dbdata->tab_sz)
		dbdata_init(dbdata);

	snprintf(mask, sizeof(mask), "*%s", ext);	// notrans

	for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
#if 0
#if defined (LINUX) || defined (WIN32)
		if (strlen(dp->d_name) < 3)
			continue;
#else
		if (dp->d_namlen < 3 || dp->d_type != DT_REG)
			continue;
#endif
#endif
		if (dp->d_type != DT_REG)
			continue;

		if (fnmatch(mask, dp->d_name, 0) == FNM_NOMATCH)
			continue;

		if (dbdata->dbinit)
			dbdata->dbinit(dbdata);
		db_parse_file(dbdata, path, dp->d_name);
	}
	closedir(dirp);
}

void db_load_list(DBDATA *dbdata, const char *path, const char *file)
{
	rfile_t *fp;

	if ((fp = rfile_open(path, file)) == NULL)
		exit(1);

	if (!dbdata->tab_sz)
		dbdata_init(dbdata);
	for (; ;) {
		fread_word(fp);
		if (IS_TOKEN(fp, "$"))
			break;

		if (dbdata->dbinit)
			dbdata->dbinit(dbdata);
		db_parse_file(dbdata, path, rfile_tok(fp));
	}
	rfile_close(fp);
}

void boot_db_system(void)
{
	db_load_file(&db_system, ETC_PATH, SYSTEM_CONF);
}

/*
 * Big mama top level function.
 */
void boot_db(void)
{
#ifdef OLD_RAND
	/*
	 * Init random number generator.
	 */
	init_mm();
#endif

	init_dynafuns();

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
	load_notes();
	load_bans();

	fix_resets();
	fix_exits();

	/*
	 * load modules and call boot callbacks
	 */
	boot_modules();
}

/*
 * Sets vnum range for area using OLC protection features.
 */
void vnum_check(AREA_DATA *area, int vnum)
{
	if (area->min_vnum == 0 || area->max_vnum == 0) {
		log(LOG_INFO, "%s: min_vnum or max_vnum not assigned",
			   area->file_name);
	}

	if (vnum < area->min_vnum || vnum > area->max_vnum) {
		log(LOG_INFO, "%s: %d not in area vnum range",
			   area->file_name, vnum);
	}
}

/*****************************************************************************
 Name:		reset_add
 Purpose:	Inserts a new reset in the given index slot.
 Called by:	do_resets(olc.c).
 ****************************************************************************/
void reset_add(ROOM_INDEX_DATA *room, RESET_DATA *reset, RESET_DATA *after)
{
	RESET_DATA *r;

	if (after == NULL) {
		/*
		 * add to the end
		 */

		reset->next = NULL;
		if (room->reset_first == NULL)
			room->reset_first = reset;
		if (room->reset_last == NULL)
			room->reset_last = reset;
		else
			room->reset_last = room->reset_last->next = reset;
		return;
	}

	for (r = room->reset_first; r != NULL; r = r->next) {
		if (r == after)
			break;
	}

	if (r == NULL) {
		log(LOG_BUG, "reset_add: `after' reset not found");
		return;
	}

	reset->next = r->next;
	r->next = reset;
	if (reset->next == NULL)
		room->reset_last = reset;
}

void
reset_del(ROOM_INDEX_DATA *room, RESET_DATA *reset)
{
	RESET_DATA *r;
	RESET_DATA *prev = NULL;

	for (r = room->reset_first; r != NULL; r = r->next) {
		if (r == reset)
			break;
		prev = r;
	}

	if (r == NULL)
		return;

	if (prev == NULL)
		room->reset_first = r->next;
	else
		prev->next = r->next;
	if (r->next == NULL)
		room->reset_last = prev;
}

RESET_DATA *
reset_lookup(ROOM_INDEX_DATA *room, int rnum)
{
	RESET_DATA *r;

	for (r = room->reset_first; r != NULL; r = r->next) {
		if (!--rnum)
			break;
	}

	return r;
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
void fix_exits(void)
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

/*
 * Get an extra description from a list.
 */
ED_DATA *ed_lookup(const char *name, ED_DATA *ed)
{
	int num;
	char arg[MAX_INPUT_LENGTH];

	num = number_argument(name, arg, sizeof(arg));
	for (; ed != NULL; ed = ed->next) {
		if (arg[0] && !is_name(arg, ed->keyword))
			continue;
		if (!--num)
			return ed;
	}
	return NULL;
}

/*
 * Translates mob virtual number to its mob index struct.
 * Hash table lookup.
 */
MOB_INDEX_DATA *get_mob_index(int vnum)
{
	MOB_INDEX_DATA *pMobIndex;

	if (vnum <= 0)
		return NULL;

	for (pMobIndex = mob_index_hash[vnum % MAX_KEY_HASH];
	     pMobIndex; pMobIndex = pMobIndex->next)
		if (pMobIndex->vnum == vnum)
			return pMobIndex;

	return NULL;
}

/*
 * Translates mob virtual number to its obj index struct.
 * Hash table lookup.
 */
OBJ_INDEX_DATA *get_obj_index(int vnum)
{
	OBJ_INDEX_DATA *pObjIndex;

	if (vnum <= 0)
		return NULL;

	for (pObjIndex = obj_index_hash[vnum % MAX_KEY_HASH];
	     pObjIndex; pObjIndex = pObjIndex->next)
		if (pObjIndex->vnum == vnum)
			return pObjIndex;

	return NULL;
}

/*
 * Translates mob virtual number to its room index struct.
 * Hash table lookup.
 */
ROOM_INDEX_DATA *get_room_index(int vnum)
{
	ROOM_INDEX_DATA *pRoomIndex;

	if (vnum <= 0)
		return NULL;

	for (pRoomIndex = room_index_hash[vnum % MAX_KEY_HASH];
	     pRoomIndex; pRoomIndex = pRoomIndex->next)
		if (pRoomIndex->vnum == vnum)
			return pRoomIndex;

	return NULL;
}

/*
 * duplicate '~'
 */
char *fix_string(const char *s)
{
	static char buf[MAX_STRING_LENGTH * 2];
	char *p = buf;

	if (IS_NULLSTR(s))
		return str_empty;

	if (*s == '.' || isspace(*s))
		*p++ = '.';

	for (; *s && p < buf + sizeof(buf) - 2; s++) {
		switch (*s) {
		case '~':
			*p++ = *s;
			/* FALLTHRU */

		default:
			*p++ = *s;
			break;
		}
	}

	*p = '\0';
	return buf;
}

const char *
fix_word(const char *w)
{
	static char buf[MAX_STRING_LENGTH];

	if (IS_NULLSTR(w))
		return "''";					// notrans

	if (strpbrk(w, " \t") == NULL)				// notrans
		return w;

	snprintf(buf, sizeof(buf), "'%s'", w);			// notrans
	return buf;
}

void fwrite_string(FILE *fp, const char *name, const char *str)
{
	if (IS_NULLSTR(name))
		fprintf(fp, "%s~\n", fix_string(str));
	else if (!IS_NULLSTR(str))
		fprintf(fp, "%s %s~\n", name, fix_string(str));
}

void fwrite_word(FILE *fp, const char *name, const char *w)
{
	if (!IS_NULLSTR(w))
		fprintf(fp, "%s %s\n", name, fix_word(w));
}

void fwrite_number(FILE *fp, const char *name, int num)
{
	if (num)
		fprintf(fp, "%s %d\n", name, num);
}
#endif /* !defined(MPC) */

/*
 * Stick a little fuzz on a number.
 */
int number_fuzzy(int number)
{
	switch (number_bits(2))
	{
	case 0:  number -= 1; break;
	case 3:  number += 1; break;
	}

	return UMAX(1, number);
}

static long number_mm(void);

/*
 * Generate a random number.
 */
int number_range(int from, int to)
{
	int power;
	int number;

	if (from == 0 && to == 0)
		return 0;

	if ((to = to - from + 1) <= 1)
		return from;

	for (power = 2; power < to; power <<= 1)
		;

	while ((number = number_mm() & (power -1)) >= to)
		;

	return from + number;
}

/*
 * Generate a percentile roll.
 */
int number_percent(void)
{
	int percent;

	while ((percent = number_mm() & (128-1)) > 99)
		;

	return 1 + percent;
}

/*
 * Generate a random door.
 */
int number_door(void)
{
	int door;

	while ((door = number_mm() & (8-1)) > 5)
		;

	return door;
}

int number_bits(int width)
{
	return number_mm() & ((1 << width) - 1);
}

/*
 * I've gotten too many bad reports on OS-supplied random number generators.
 * This is the Mitchell-Moore algorithm from Knuth Volume II.
 * Best to leave the constants alone unless you've read Knuth.
 * -- Furey
 */

/* I noticed streaking with this random number generator, so I switched
	back to the system srandom call.  If this doesn't work for you,
	define OLD_RAND to use the old system -- Alander */

#if defined (OLD_RAND)
static  int     rgiState[2+55];
#endif

static void
init_mm(void)
{
#if defined (OLD_RAND)
	int *piState;
	int iState;

	piState     = &rgiState[2];

	piState[-2] = 55 - 55;
	piState[-1] = 55 - 24;

	piState[0]  = ((int) current_time) & ((1 << 30) - 1);
	piState[1]  = 1;
	for (iState = 2; iState < 55; iState++)
	{
		piState[iState] = (piState[iState-1] + piState[iState-2])
		                & ((1 << 30) - 1);
	}
#else
//	srandom(time(NULL)^getpid());
	srandomdev();
#endif
	return;
}

#define MAX_RND_CNT	128

int max_rnd_cnt	= MAX_RND_CNT;
int rnd_cnt;

static long
number_mm(void)
{
#if defined (OLD_RAND)
	int *piState;
	int iState1;
	int iState2;
	int iRand;

	piState             = &rgiState[2];
	iState1             = piState[-2];
	iState2             = piState[-1];
	iRand               = (piState[iState1] + piState[iState2])
		                & ((1 << 30) - 1);
	piState[iState1]    = iRand;
	if (++iState1 == 55)
		iState1 = 0;
	if (++iState2 == 55)
		iState2 = 0;
	piState[-2]         = iState1;
	piState[-1]         = iState2;
	return iRand >> 6;
#else
	if (max_rnd_cnt > 0 && (rnd_cnt++ % max_rnd_cnt) == 0)
		init_mm();
	return random() >> 6;
#endif
}

/*
 * Roll some dice.
 */
int dice(int number, int size)
{
	int idice;
	int sum;

	switch (size)
	{
	case 0: return 0;
	case 1: return number;
	}

	for (idice = 0, sum = 0; idice < number; idice++)
		sum += number_range(1, size);

	return sum;
}

/*
 * Roll some dice with luck bonus.
 * ch wants roll to be bigger, victim wants roll to be smaller.
 */

int dice_wlb(int number, int size, CHAR_DATA *ch, CHAR_DATA *victim)
{
	int idice;
	int sum;

	int luck_diff;

	luck_diff = ((ch == NULL) ? 50 : GET_LUCK(ch)) -
		 ((victim == NULL) ? 50 : GET_LUCK(victim));

	switch (size) {
	case 0:
		return 0;
	case 1:
		return number;
	}

	for (idice = 0, sum = 0; idice < number; idice++) {
		int cand;
		int num;
		if (luck_diff >= 0) {
			num = luck_diff / 20 + 1 +
				(number_range(0, 19) < luck_diff % 20) ? 1 : 0;

			for (cand = 0; num; num--)
				cand = UMAX(cand, number_range(1, size));
		} else {
			num = (-luck_diff) / 20 + 1 +
				(number_range(0, 19) < (-luck_diff) % 20) ? 1 : 0;

			for (cand = 0; num; num--)
				cand = UMIN(cand, number_range(1, size));
		}
		sum += cand;
	}
	return sum;
}

/*
 * Simple linear interpolation.
 */
int interpolate(int level, int value_00, int value_32)
{
	return value_00 + level * (value_32 - value_00) / 32;
}

#if !defined(MPC)
/*
 * Returns an initial-capped string.
 */
char *capitalize(const char *str)
{
	static char strcap[MAX_STRING_LENGTH];
	int i;

	for (i = 0; str[i] != '\0'; i++)
		strcap[i] = LOWER(str[i]);
	strcap[i] = '\0';
	strcap[0] = UPPER(strcap[0]);
	return strcap;
}

void move_pfiles(int minvnum, int maxvnum, int delta)
{
	struct dirent *dp;
	DIR *dirp;

	if ((dirp = opendir(PLAYER_PATH)) == NULL) {
		log(LOG_ERROR, "move_pfiles: unable to open player directory");
		exit(1);
	}
	for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {

#if defined (LINUX) || defined (WIN32)
		if (strlen(dp->d_name) < 3)
			continue;
#else
		if (dp->d_namlen < 3 || dp->d_type != DT_REG)
			continue;
#endif

		if (strchr(dp->d_name, '.'))
			continue;

		move_pfile(dp->d_name, minvnum, maxvnum, delta);
	}
	closedir(dirp);

}

#define NBUF 5
#define NBITS 32

char *format_flags(flag_t flags)
{
	static int cnt;
	static char buf[NBUF][NBITS+1];
	int count, pos = 0;

	cnt = (cnt + 1) % NBUF;

	for (count = 0; count < NBITS;  count++) {
		if (IS_SET(flags, 1 << count)) {
			if (count < 26)
				buf[cnt][pos] = 'A' + count;
			else
				buf[cnt][pos] = 'a' + (count - 26);
			pos++;
		}
	}

	if (pos == 0) 
		buf[cnt][pos++] = '0';

	buf[cnt][pos] = '\0';
	return buf[cnt];
}

static hashdata_t h_glob_gmlstr =
{
	sizeof(gmlstr_t), 1,
	(e_init_t) gmlstr_init,
	(e_destroy_t) gmlstr_destroy,
	(e_cpy_t) gmlstr_cpy,

	STRKEY_HASH_SIZE,
	k_hash_str,
	ke_cmp_mlstr
};

DBINIT_FUN(init_glob_gmlstr)
{
	if (!DBDATA_VALID(dbdata))
		hash_init(&glob_gmlstr, &h_glob_gmlstr);
}

DBLOAD_FUN(load_glob_gmlstr)
{
	gmlstr_t gml;

	gmlstr_init(&gml);
	mlstr_fread(fp, &gml.ml);
	if (mlstr_null(&gml.ml)) {
		gmlstr_destroy(&gml);
		log(LOG_ERROR, "load_glob_gmlstr: null gmlstr");
		fread_to_end(fp);
		return;
	}

	for (;;) {
		bool fMatch = FALSE;

		fread_keyword(fp);
		switch (rfile_tokfl(fp)) {
		case 'E':
			if (IS_TOKEN(fp, "End")) {
				if (hash_insert(&glob_gmlstr, gmlstr_mval(&gml), &gml) == NULL) {
					log(LOG_ERROR, "load_gmlstr: duplicate gmlstr");
				}
				gmlstr_destroy(&gml);
				return;
			}
			break;
		case 'G':
			MLSKEY("gender", gml.gender);
			break;
		}

		if (!fMatch) {
			log(LOG_ERROR, "load_glob_gmlstr: %s: Unknown keyword",
				 rfile_tok(fp));
			fread_to_eol(fp);
		}
	}
}

static hashdata_t h_msgdb =
{
	sizeof(mlstring), 1,
	(e_init_t) mlstr_init,
	(e_destroy_t) mlstr_destroy,
	(e_cpy_t) mlstr_cpy,

	STRKEY_HASH_SIZE,
	k_hash_csstr,
	ke_cmp_csmlstr
};

static void
load_msgdb(void)
{
	int msgcnt = 0;
	rfile_t *fp;
	mlstring ml;

	hash_init(&msgdb, &h_msgdb);

	line_number = 0;
	snprintf(filename, sizeof(filename), "%s%c%s",
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
hint_init(hint_t *t)
{
	mlstr_init(&t->phrase);
}

static void
hint_destroy(hint_t *t)
{
	mlstr_destroy(&t->phrase);
}

static varrdata_t v_hints =
{
	sizeof(hint_t), 4,
	(e_init_t) hint_init,
	(e_destroy_t) hint_destroy,
	NULL
};

static void
load_hints(void)
{
	rfile_t *fp;

	varr_init(&hints, &v_hints);

	line_number = 0;
	snprintf(filename, sizeof(filename), "%s%c%s",
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

const char *GETMSG(const char *msg, size_t lang)
{
	mlstring *ml;

	if (lang == 0 || (ml = msg_lookup(msg)) == NULL)
		return msg;
	return mlstr_val(ml, lang);
}
#endif /* !defined(MPC) */
