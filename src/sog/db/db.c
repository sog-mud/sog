/*
 * $Id: db.c,v 1.209 2000-02-10 14:08:57 fjoe Exp $
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

#include "merc.h"
#include "chquest.h"
#include "rating.h"
#include "socials.h"
#include "db.h"
#include "module.h"
#include "lang.h"
#include "note.h"
#include "ban.h"
#include "rfile.h"

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

void scan_pfiles();

extern	int	_filbuf		(FILE *);

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

/*
 * Globals.
 */
const char TMP_PATH		[] = "tmp";
const char PLAYER_PATH		[] = "player";
const char GODS_PATH		[] = "gods";
const char NOTES_PATH		[] = "notes";
const char ETC_PATH		[] = "etc";
const char AREA_PATH		[] = "area";
const char LANG_PATH		[] = "lang";
const char MODULES_PATH		[] = "modules";

const char CLASSES_PATH		[] = "classes";
const char CLANS_PATH		[] = "clans";
const char RACES_PATH		[] = "races";
const char SPEC_PATH		[] = "specs";

const char RACE_EXT		[] = ".race";
const char CLASS_EXT		[] = ".class";
const char CLAN_EXT		[] = ".clan";
const char SPEC_EXT		[] = ".spec";

#if defined (WIN32)
const char PLISTS_PATH		[] = "clans\\plists";
const char NULL_FILE		[] = "NUL";
#else
const char PLISTS_PATH		[] = "clans/plists";
const char NULL_FILE		[] = "/dev/null";
#endif

const char TMP_FILE		[] = "romtmp";

const char HOMETOWNS_CONF	[] = "hometowns.conf";	/* hometowns */
const char SKILLS_CONF		[] = "skills.conf";	/* skills */
const char SOCIALS_CONF		[] = "socials.conf";	/* socials */
const char SYSTEM_CONF		[] = "system.conf";	/* system conf */
const char LANG_CONF		[] = "lang.conf";	/* lang definitions */
const char CMD_CONF		[] = "cmd.conf";	/* commands */
const char DAMTYPE_CONF		[] = "damtype.conf";	/* damtypes */
const char MATERIALS_CONF	[] = "materials.conf";	/* materials */
const char LIQUIDS_CONF		[] = "liquids.conf";	/* liquids */
const char FORMS_CONF		[] = "forms.conf";	/* shapeforms */
const char CC_EXPR_CONF		[] = "cc_expr.conf";	/* cc_exprs */
const char GLOB_GMLSTR_FILE	[] = "glob_gmlstr";	/* global gmlstrs */
const char MSGDB_FILE		[] = "msgdb";		/* msgdb */

const char AREA_LIST		[] = "area.lst";	/* list of areas */
const char LANG_LIST		[] = "lang.lst";	/* list of languages */

const char BUG_FILE		[] = "bugs.txt";	/* 'bug' and log(LOG_ERROR, ) */
const char TYPO_FILE		[] = "typos.txt";	/* 'typo' */
const char NOTE_FILE		[] = "notes.not";	/* 'notes' */
const char IDEA_FILE		[] = "ideas.not";
const char PENALTY_FILE		[] = "penal.not";
const char NEWS_FILE		[] = "news.not";
const char CHANGES_FILE		[] = "chang.not";
const char SHUTDOWN_FILE	[] = "shutdown";	/* For 'shutdown' */
const char EQCHECK_FILE		[] = "eqcheck";		/* limited eq checks */
const char BAN_FILE		[] = "ban.txt";
const char MAXON_FILE		[] = "maxon.txt";
const char AREASTAT_FILE	[] = "areastat.txt";
const char IMMLOG_FILE		[] = "immortals.log";

flag_t		mud_options;

SHOP_DATA *		shop_first;
SHOP_DATA *		shop_last;

CHAR_DATA *		char_list;
CHAR_DATA *		char_list_lastpc;

OBJ_DATA *		object_list;
TIME_INFO_DATA		time_info;
WEATHER_DATA		weather_info;

ROOM_INDEX_DATA	*	top_affected_room;
int			reboot_counter = 1440;

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

static void load_msgdb(void);

/*
 * Local booting procedures.
*/
static void	init_mm         (void);
 
static void	fix_resets	(void);
static void	fix_exits	(void);

void	reset_area	(AREA_DATA * pArea);

DECLARE_DBLOAD_FUN(load_glob_gmlstr);
DECLARE_DBINIT_FUN(init_glob_gmlstr);

DBFUN dbfun_glob_gmlstr[] =
{
	{ "GLOB",	load_glob_gmlstr	},
	{ NULL }
};

DBDATA db_glob_gmlstr = { dbfun_glob_gmlstr, init_glob_gmlstr };

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

	snprintf(buf2, sizeof(buf2), "%s: line %d: %s",
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

	snprintf(mask, sizeof(mask), "*%s", ext);

	for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
#if defined (LINUX) || defined (WIN32)
		if (strlen(dp->d_name) < 3)
			continue;
#else
		if (dp->d_namlen < 3 || dp->d_type != DT_REG)
			continue;
#endif

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
	long lhour, lday, lmonth;
	int i;

#ifdef __FreeBSD__
	extern char* malloc_options;
	malloc_options = "X";
#endif

	/*
	 * Init random number generator.
	 */
	init_mm();

	/*
	 * Set time and weather.
	 */

	lhour	= (current_time - 650336715) / (PULSE_TICK / PULSE_PER_SCD);
	time_info.hour	= lhour  % 24;
	lday		= lhour  / 24;
	time_info.day	= lday   % 35;
	lmonth		= lday   / 35;
	time_info.month	= lmonth % 17;
	time_info.year	= lmonth / 17;

	     if (time_info.hour <  5) weather_info.sunlight = SUN_DARK;
	else if (time_info.hour <  6) weather_info.sunlight = SUN_RISE;
	else if (time_info.hour < 19) weather_info.sunlight = SUN_LIGHT;
	else if (time_info.hour < 20) weather_info.sunlight = SUN_SET;
	else                          weather_info.sunlight = SUN_DARK;

	weather_info.change	= 0;
	weather_info.mmhg	= 960;
	if (time_info.month >= 7 && time_info.month <= 12)
		weather_info.mmhg += number_range(1, 50);
	else
		weather_info.mmhg += number_range(1, 80);

	     if (weather_info.mmhg <=  980) weather_info.sky = SKY_LIGHTNING;
	else if (weather_info.mmhg <= 1000) weather_info.sky = SKY_RAINING;
	else if (weather_info.mmhg <= 1020) weather_info.sky = SKY_CLOUDY;
	else                                weather_info.sky = SKY_CLOUDLESS;

	db_load_list(&db_langs, LANG_PATH, LANG_LIST);
	db_load_file(&db_glob_gmlstr, ETC_PATH, GLOB_GMLSTR_FILE);
	load_msgdb();
	db_load_file(&db_cmd, ETC_PATH, CMD_CONF);
	db_load_file(&db_socials, ETC_PATH, SOCIALS_CONF);
	db_load_file(&db_cc_expr, ETC_PATH, CC_EXPR_CONF);

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

	/*
	 * load modules
	 */
	for (i = 0; i < modules.nused; i++) {
		if (mod_load(VARR_GET(&modules, i)) < 0)
			exit(1);
	}

	/*
	 * Fix up exits.
	 * Reset all areas once.
	 * Load up the songs, notes and ban files.
	 */
	fix_resets();
	fix_exits();
	scan_pfiles();

	area_update();
	load_notes();
	load_bans();
	chquest_start(0);
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
		log(LOG_ERROR, "reset_add: `after' reset not found");
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

void print_resetmsg(AREA_DATA *pArea)
{
	DESCRIPTOR_DATA *d;
	bool is_empty = mlstr_null(&pArea->resetmsg);
	
	for (d = descriptor_list; d != NULL; d = d->next) {
		CHAR_DATA *ch;

		if (d->connected != CON_PLAYING)
			continue;

		ch = d->original ? d->original : d->character;
		if (IS_NPC(ch) || !IS_AWAKE(ch) || ch->in_room->area != pArea)
			continue;

		if (is_empty)
			act_puts("You hear some squeaking sounds...",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		else
			act_puts(mlstr_cval(&pArea->resetmsg, ch),
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
	}
}

/*
 * Repopulate areas periodically.
 */
void area_update(void)
{
	AREA_DATA *pArea;

	for (pArea = area_first; pArea != NULL; pArea = pArea->next) {
		ROOM_INDEX_DATA *pRoomIndex;

		if (++pArea->age < 3)
			continue;

		/*
		 * Check age and reset.
		 */
		if ((pArea->empty || (pArea->nplayer != 0 && pArea->age < 15))
		&&  pArea->age < 31
		&&  !IS_SET(pArea->area_flags, AREA_UPDATE_ALWAYS))
			continue;

		/*
		 * the rain devastates tracks on the ground
		 */
		if (weather_info.sky == SKY_RAINING)  {
			int i;
			DESCRIPTOR_DATA *d;
			CHAR_DATA *ch;

	 		for (d = descriptor_list; d; d = d->next)  {
				if (d->connected != CON_PLAYING)
					continue;

				ch = d->original ?  d->original : d->character;
				if (ch->in_room->area == pArea
				&&  get_skill(ch, "track") > 50
				&&  !IS_SET(ch->in_room->room_flags,
					    ROOM_INDOORS)) {
					act_puts("Rain devastates the tracks on the ground.",
						 ch, NULL, NULL, TO_CHAR, POS_DEAD);
				}
			}

			for (i = pArea->min_vnum; i < pArea->max_vnum; i++) {
				pRoomIndex = get_room_index(i);
				if (pRoomIndex == NULL
				||  IS_SET(pRoomIndex->room_flags,
								ROOM_INDOORS))
					continue;
				if (number_percent() < 50)
					room_record("erased", pRoomIndex, -1);
			}
		}

		reset_area(pArea);
		wiznet("$t has just been reset.",
			NULL, pArea->name, WIZ_RESETS, 0, 0);

		print_resetmsg(pArea);

		pArea->age = number_range(0, 3);
		if (IS_SET(pArea->area_flags, AREA_UPDATE_FREQUENTLY))
			pArea->age = 15 - 2;
		else if (pArea->nplayer == 0) 
			pArea->empty = TRUE;
	}
}

static void *
clan_item_cb(void *p, va_list ap)
{
	clan_t *clan = (clan_t *) p;
	
	OBJ_INDEX_DATA *pObjIndex = va_arg(ap, OBJ_INDEX_DATA *);

	if (clan->obj_ptr == NULL
	||  pObjIndex->vnum != clan->obj_vnum)
		return NULL;

	return p;
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
				log(LOG_ERROR, "fix_resets_room: no 'E', 'G' or 'O' reset for obj vnum %d in area", r->arg3);
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

static int reset_room_vnum;	/* vnum of room being reset */
static int reset_num;		/* number of reset in room being reset */

static void logger_reset(const char *buf)
{
	char buf2[MAX_STRING_LENGTH];
	snprintf(buf2, sizeof(buf2), "reset_room %d[%d]: %s",
		 reset_room_vnum, reset_num, buf);
	logger_default(buf2);
}

/*
 * OLC
 * Reset one room.  Called by reset_area and olc.
 */
void reset_room(ROOM_INDEX_DATA *pRoom, int flags)
{
	RESET_DATA *pReset;

	CHAR_DATA *last_mob = NULL;
	bool lmob = FALSE;		/* last mob was reset */

	OBJ_DATA *last_obj = NULL;
	bool lobj = FALSE;		/* last obj was reset */

	int iExit;
	logger_t logger_old;

	for (iExit = 0; iExit < MAX_DIR; iExit++) {
		EXIT_DATA *pExit;

		if ((pExit = pRoom->exit[iExit]) == NULL
		/*  || IS_SET(pExit->exit_info, EX_BASHED) */)
			continue;

                /*
		 * nail both sides
		 */
		pExit->exit_info = pExit->rs_flags;
		if (pExit->to_room.r != NULL
		&&  (pExit = pExit->to_room.r->exit[rev_dir[iExit]]) != NULL)
			pExit->exit_info = pExit->rs_flags;
	}

	reset_room_vnum = pRoom->vnum;
	logger_old = logger_set(LOG_ERROR, logger_reset);

	for (reset_num = 0, pReset = pRoom->reset_first; pReset != NULL;
					pReset = pReset->next, reset_num++) {
		MOB_INDEX_DATA *pMobIndex;
		OBJ_INDEX_DATA *pObjIndex;
		OBJ_DATA *obj;
		int count, limit;
		EXIT_DATA *pExit;
		int d0;

		switch (pReset->command) {
		default:
			log(LOG_ERROR, "bad command %c", pReset->command);
			break;

		case 'M':
			/*
			 * mob to room
			 *	arg1 - mob vnum
			 *	arg2 - mob count limit (total)
			 *	arg3 - room vnum
			 *	arg4 - mob count limit (in room)
			 */
			if ((pMobIndex = get_mob_index(pReset->arg1)) == NULL) {
				log(LOG_ERROR, "%d: no such mob", pReset->arg1);
				lmob = FALSE;
				break;
			}

			if (pMobIndex->count >= pReset->arg2) {
				lmob = FALSE;
				break;
			}

			count = 0;
			for (last_mob = pRoom->people; last_mob != NULL; last_mob = last_mob->next_in_room) {
				if (last_mob->pMobIndex == pMobIndex)
					count++;
			}

			if (count >= pReset->arg4) {
				lmob = FALSE;
				break;
			}

			last_mob = create_mob(pMobIndex);
			NPC(last_mob)->zone = pRoom->area;
			char_to_room(last_mob, pRoom);
			if (IS_EXTRACTED(last_mob)) {
				lmob = FALSE;
				break;
			}
			lmob = TRUE;
			break;

		case 'G':
		case 'E':
			/*
			 * give obj to char or equip char (reset by 'M')
			 *	arg1 - obj vnum
			 *	arg2 -
			 *	arg3 - wear location (for 'E')
			 *	arg4 -
			 */
			if (!lmob)
				break;

			if (last_mob == NULL) {
				log(LOG_ERROR, "no previous mob");
				lobj = FALSE;
				break;
			}

			if ((pObjIndex = get_obj_index(pReset->arg1)) == NULL) {
				log(LOG_ERROR, "%d: no such obj", pReset->arg1);
				lobj = FALSE;
				break;
			}

			if ((pObjIndex->limit != -1)
			&&  pObjIndex->count >= pObjIndex->limit) {
				lobj = FALSE;
				break;
			}

			last_obj = create_obj(pObjIndex, 0);
			if (pReset->command == 'G'
			&&  last_mob->pMobIndex->pShop) /* Shop-keeper? */
				SET_OBJ_STAT(last_obj, ITEM_INVENTORY);

			obj_to_char(last_obj, last_mob);
			if (pReset->command == 'E')
				equip_char(last_mob, last_obj, pReset->arg3);
			lobj = TRUE;
			break;

		case 'O':
			/*
			 * obj to room
			 *	arg1 - obj vnum
			 *	arg2 -
			 *	arg3 - room vnum
			 *	arg3 -
			 * obj limits are checked
			 */
			if ((pObjIndex = get_obj_index(pReset->arg1)) == NULL) {
				log(LOG_ERROR, "%d: no such obj", pReset->arg1);
				lobj = FALSE;
				break;
			}

			if ((pRoom->area->nplayer > 0 &&
			     !IS_SET(flags, RESET_F_NOPCHECK))
			||  count_obj_list(pObjIndex, pRoom->contents) > 0
			||  (pObjIndex->limit != -1 &&
			     pObjIndex->count >= pObjIndex->limit)) {
				lobj = FALSE;
				break;
			}

			last_obj = create_obj(pObjIndex, 0);
			last_obj->cost = 0;
			lobj = TRUE;
			obj_to_room(last_obj, pRoom);
			break;

		case 'P':
			/*
			 * put obj in last obj (reset by 'E', 'G' or 'O')
			 * 	arg1 - vnum of obj to put
		 	 * 	arg2 - obj count limit (total)
			 * 	arg3 - 
			 * 	arg4 - min obj count (in obj)
			 * obj limits are checked
			 */

			if (!lobj)
				break;

			if (last_obj == NULL) {
				log(LOG_ERROR, "no previous obj");
				break;
			}

			if ((pObjIndex = get_obj_index(pReset->arg1)) == NULL) {
				log(LOG_ERROR, "%d: no such obj", pReset->arg1);
				break;
			}
	    
			if (pReset->arg2 > 50) /* old format */
				limit = 6;
			else if (pReset->arg2 == -1) /* no limit */
				limit = 999;
			else
				limit = pReset->arg2;

			if (pRoom->area->nplayer > 0
			&&  !IS_SET(flags, RESET_F_NOPCHECK))
				break;

			if (IS_SET(pObjIndex->obj_flags, OBJ_CLAN)) {
				clan_t* clan;

				clan = hash_foreach(&clans, clan_item_cb,
						    pObjIndex);
				if (clan != NULL) {
					obj = create_obj(pObjIndex, 0);
					clan->obj_ptr = obj;
					clan->altar_ptr = obj;
					obj_to_obj(obj, last_obj);
				}
				break;
			}

			count = count_obj_list(pObjIndex, last_obj->contains);
			for (;;) {
				if (count >= pReset->arg4
				||  (pObjIndex->count >= limit &&
				     number_range(0, 4) != 0)
				||  (pObjIndex->limit != -1 &&
				     pObjIndex->count >= pObjIndex->limit))
					break;

				obj = create_obj(pObjIndex, 0);
				obj_to_obj(obj, last_obj);
				count++;
			}

			/* fix object lock state! */
			last_obj->value[1] = last_obj->pObjIndex->value[1];
			break;

		case 'R':
			/*
			 * randomize exits
			 */
			for (d0 = 0; d0 < pReset->arg2 - 1; d0++) {
				int d1 = number_range(d0, pReset->arg2 - 1);
				pExit = pRoom->exit[d0];
				pRoom->exit[d0] = pRoom->exit[d1];
				pRoom->exit[d1] = pExit;
			}
			break;
		} /* switch */
	} /* for */

	logger_set(LOG_ERROR, logger_old);
}

/*
 * OLC
 * Reset one area.
 */
void reset_area(AREA_DATA *pArea)
{
	ROOM_INDEX_DATA *pRoom;
	int vnum;

	for (vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++)
		if ((pRoom = get_room_index(vnum)) != NULL)
			reset_room(pRoom, 0);
}

static const char *
cb_xxx_of(int lang, const char **p, va_list ap)
{
	mlstring *owner = va_arg(ap, mlstring *);
	const char *q;

	if (IS_NULLSTR(*p))
		return NULL;

	q = str_printf(*p, word_form(mlstr_val(owner, lang), 1,
				     lang, RULES_CASE));
	free_string(*p);
	*p = q;
	return NULL;
}

/*
 * Create an instance of a mobile.
 */
CHAR_DATA *create_mob(MOB_INDEX_DATA *pMobIndex)
{
	CHAR_DATA *mob;
	int i;
	AFFECT_DATA af;
	AFFECT_DATA *paf;

	if (pMobIndex == NULL) {
		log(LOG_ERROR, "create_mob: NULL pMobIndex.");
		exit(1);
	}

	mob = char_new(pMobIndex);

	mob->name	= str_qdup(pMobIndex->name);
	mlstr_cpy(&mob->short_descr, &pMobIndex->short_descr);
	mlstr_cpy(&mob->long_descr, &pMobIndex->long_descr);
	mlstr_cpy(&mob->description, &pMobIndex->description);
	mob->class = str_empty;

	if (pMobIndex->wealth) {
		long wealth;

		wealth = number_range(pMobIndex->wealth/2,
				      3 * pMobIndex->wealth/2);
		mob->gold = number_range(wealth/200,wealth/100);
		mob->silver = wealth - (mob->gold * 100);
	} 

	mob->affected_by	= pMobIndex->affected_by;
	mob->has_invis		= pMobIndex->has_invis;
	mob->has_detect		= pMobIndex->has_detect;
	mob->alignment		= pMobIndex->alignment;
	mob->level		= pMobIndex->level;
	mob->position		= pMobIndex->start_pos;
	free_string(mob->race);
	mob->race		= str_qdup(pMobIndex->race);
	mob->form		= pMobIndex->form;
	mob->parts		= pMobIndex->parts;
	mob->size		= pMobIndex->size;
	free_string(mob->clan);
	mob->clan		= str_qdup(pMobIndex->clan);
	mob->invis_level	= pMobIndex->invis_level;
	mob->incog_level	= pMobIndex->incog_level;
	mob->material		= str_qdup(pMobIndex->material);

	mob->damtype		= str_qdup(pMobIndex->damtype);
	if (IS_NULLSTR(mob->damtype)) {
		switch (number_range(1, 3)) {
		case 1:
			mob->damtype = str_dup("slash");
			break;
		case 2:
			mob->damtype = str_dup("pound");
			break;
		case 3:
			mob->damtype = str_dup("pierce");
			break;
		}
	}

	if (flag_value(sex_table, mlstr_mval(&pMobIndex->gender)) == SEX_EITHER) {
		MOB_INDEX_DATA *fmob;
		int sex = number_range(SEX_MALE, SEX_FEMALE);

		mlstr_destroy(&mob->gender);
		mlstr_init2(&mob->gender, flag_string(gender_table, sex));

		if (sex == SEX_FEMALE
		&&  (fmob = get_mob_index(pMobIndex->fvnum))) {
			mob->name	= str_qdup(fmob->name);
			mlstr_cpy(&mob->short_descr, &fmob->short_descr);
			mlstr_cpy(&mob->long_descr, &fmob->long_descr);
			mlstr_cpy(&mob->description, &fmob->description);
		}
	} else {
		mlstr_cpy(&mob->gender, &pMobIndex->gender);
	}

	for (i = 0; i < MAX_RESIST; i++) 
		mob->resists[i] = pMobIndex->resists[i];

	for (i = 0; i < MAX_STATS; i ++)
		mob->perm_stat[i] = UMIN(25, 11 + mob->level/4);

	mob->perm_stat[STAT_STR] += mob->size - SIZE_MEDIUM;
	mob->perm_stat[STAT_CON] += (mob->size - SIZE_MEDIUM) / 2;

	mob->hitroll		= (mob->level / 2) + pMobIndex->hitroll;
	mob->damroll		= pMobIndex->damage[DICE_BONUS];
	SET_HIT(mob, dice(pMobIndex->hit[DICE_NUMBER],
			  pMobIndex->hit[DICE_TYPE]) +
		     pMobIndex->hit[DICE_BONUS]);
	SET_MANA(mob, dice(pMobIndex->mana[DICE_NUMBER],
			   pMobIndex->mana[DICE_TYPE]) +
		      pMobIndex->mana[DICE_BONUS]);
	NPC(mob)->dam.dice_number = pMobIndex->damage[DICE_NUMBER];
	NPC(mob)->dam.dice_type = pMobIndex->damage[DICE_TYPE];
	for (i = 0; i < 4; i++)
		mob->armor[i]	= pMobIndex->ac[i]; 

	if (IS_SET(pMobIndex->act, ACT_WARRIOR)) {
		mob->perm_stat[STAT_STR] += 3;
		mob->perm_stat[STAT_INT] -= 1;
		mob->perm_stat[STAT_CON] += 2;
	}
		
	if (IS_SET(pMobIndex->act, ACT_THIEF)) {
		mob->perm_stat[STAT_DEX] += 3;
		mob->perm_stat[STAT_INT] += 1;
		mob->perm_stat[STAT_WIS] -= 1;
	}
		
	if (IS_SET(pMobIndex->act, ACT_CLERIC)) {
		mob->perm_stat[STAT_WIS] += 3;
		mob->perm_stat[STAT_DEX] -= 1;
		mob->perm_stat[STAT_STR] += 1;
	}
		
	if (IS_SET(pMobIndex->act, ACT_MAGE)) {
		mob->perm_stat[STAT_INT] += 3;
		mob->perm_stat[STAT_STR] -= 1;
		mob->perm_stat[STAT_DEX] += 1;
	}
		
	if (IS_SET(pMobIndex->off_flags, OFF_FAST))
		mob->perm_stat[STAT_DEX] += 2;
		    
	/* let's get some spell action */
	if (IS_AFFECTED(mob, AFF_SANCTUARY)) {
		af.where	= TO_AFFECTS;
		af.type		= "sanctuary";
		af.level	= mob->level;
		af.duration	= -1;
		INT(af.location)= APPLY_NONE;
		af.modifier	= 0;
		af.bitvector	= AFF_SANCTUARY;
		affect_to_char(mob, &af);
	}

	if (IS_AFFECTED(mob, AFF_HASTE)) {
		af.where	= TO_AFFECTS;
		af.type		= "haste";
		af.level	= mob->level;
	  	af.duration	= -1;
		INT(af.location)= APPLY_DEX;
		af.modifier	= 1 + (mob->level >= 18) + (mob->level >= 25) + 
				  (mob->level >= 32);
		af.bitvector	= AFF_HASTE;
		affect_to_char(mob, &af);
	}

	if (IS_AFFECTED(mob,AFF_PROTECT_EVIL)) {
		af.where	= TO_AFFECTS;
		af.type		= "protection evil";
		af.level	= mob->level;
		af.duration	= -1;
		INT(af.location)= APPLY_SAVES;
		af.modifier	= -1;
		af.bitvector	= AFF_PROTECT_EVIL;
		affect_to_char(mob, &af);
	}

	if (IS_AFFECTED(mob,AFF_PROTECT_GOOD)) {
		af.where	= TO_AFFECTS;
		af.type		= "protection good";
		af.level	= mob->level;
		af.duration	= -1;
		INT(af.location)= APPLY_SAVES;
		af.modifier	= -1;
		af.bitvector	= AFF_PROTECT_GOOD;
		affect_to_char(mob, &af);
	}  

	for (paf = pMobIndex->affected; paf != NULL; paf = paf->next)
		affect_to_char(mob, paf);

	/* link the mob to the world list */
	if (char_list_lastpc) {
		mob->next = char_list_lastpc->next;
		char_list_lastpc->next = mob;
	} else {
		mob->next = char_list;
		char_list = mob;
	}

	pMobIndex->count++;
	return mob;
}

CHAR_DATA *create_mob_of(MOB_INDEX_DATA *pMobIndex, mlstring *owner)
{
	CHAR_DATA *mob = create_mob(pMobIndex);

	mlstr_foreach(&mob->short_descr, cb_xxx_of, owner);
	mlstr_foreach(&mob->long_descr, cb_xxx_of, owner);
	mlstr_foreach(&mob->description, cb_xxx_of, owner);

	return mob;
}

/* duplicate a mobile exactly -- except inventory */
CHAR_DATA *
clone_mob(CHAR_DATA *parent)
{
	int i;
	AFFECT_DATA *paf, *paf_next;
	CHAR_DATA *clone;

	clone = create_mob(parent->pMobIndex);

	/* start fixing values */ 
	free_string(clone->name);
	clone->name 		= str_qdup(parent->name);
	mlstr_cpy(&clone->short_descr, &parent->short_descr);
	mlstr_cpy(&clone->long_descr, &parent->long_descr);
	mlstr_cpy(&clone->description, &parent->description);
	mlstr_cpy(&clone->gender, &parent->gender);
	free_string(clone->class);
	clone->class		= str_qdup(parent->class);
	free_string(clone->race);
	clone->race		= str_qdup(parent->race);
	clone->level		= parent->level;
	clone->wait		= parent->wait;

	clone->hit		= parent->hit;
	clone->max_hit		= parent->max_hit;
	clone->perm_hit		= parent->perm_hit;

	clone->mana		= parent->mana;
	clone->max_mana		= parent->max_mana;
	clone->perm_mana	= parent->perm_mana;

	clone->move		= parent->move;
	clone->max_move		= parent->max_move;
	clone->perm_move	= parent->perm_move;

	clone->gold		= parent->gold;
	clone->silver		= parent->silver;
	clone->comm		= parent->comm;
	clone->invis_level	= parent->invis_level;
	clone->incog_level	= parent->incog_level;
	clone->affected_by	= parent->affected_by;
	clone->has_invis	= parent->has_invis;
	clone->has_detect	= parent->has_detect;
	clone->position		= parent->position;
	clone->saving_throw	= parent->saving_throw;
	clone->alignment	= parent->alignment;
	clone->hitroll		= parent->hitroll;
	clone->damroll		= parent->damroll;
	clone->wimpy		= parent->wimpy;
	clone->form		= parent->form;
	clone->parts		= parent->parts;
	clone->size		= parent->size;
	free_string(clone->material);
	clone->material		= str_qdup(parent->material);
	free_string(clone->damtype);
	clone->damtype		= str_qdup(parent->damtype);
	clone->hunting		= NULL;
	free_string(clone->clan);
	clone->clan	= str_qdup(parent->clan);
	NPC(clone)->dam	= NPC(parent)->dam;

	for (i = 0; i < 4; i++)
		clone->armor[i]	= parent->armor[i];

	for (i = 0; i < MAX_STATS; i++) {
		clone->perm_stat[i]	= parent->perm_stat[i];
		clone->mod_stat[i]	= parent->mod_stat[i];
	}

	/*
	 * clone affects
	 */
	for (paf = clone->affected; paf != NULL; paf = paf_next) {
		paf_next = paf->next;
		affect_remove(clone, paf);
	}
	clone->affected = NULL;

	for (paf = parent->affected; paf != NULL; paf = paf->next)
		affect_to_char(clone, paf);

	return clone;
}

/*
 * Create an instance of an object.
 */
OBJ_DATA *create_obj(OBJ_INDEX_DATA *pObjIndex, int flags)
{
	OBJ_DATA *obj;
	int i;

	if (pObjIndex == NULL) {
		log(LOG_ERROR, "create_obj: NULL pObjIndex");
		exit(1);
	}

	obj = new_obj();

	obj->pObjIndex	= pObjIndex;
 	obj->level = pObjIndex->level;
	obj->wear_loc	= -1;

	obj->name		= str_qdup(pObjIndex->name);
	mlstr_cpy(&obj->short_descr, &pObjIndex->short_descr);
	mlstr_cpy(&obj->description, &pObjIndex->description);
	obj->material		= str_qdup(pObjIndex->material);
	obj->stat_flags		= pObjIndex->stat_flags;
	obj->wear_flags		= pObjIndex->wear_flags;
	obj->weight		= pObjIndex->weight;
	obj->condition		= pObjIndex->condition;
	obj->cost		= pObjIndex->cost;

	/*
	 * objval_destroy is not needed since obj was just created
	 */
	obj->item_type = pObjIndex->item_type;
	objval_cpy(obj->item_type, obj->value, pObjIndex->value);

	/*
	 * Mess with object properties.
	 */
	switch (obj->item_type) {
	case ITEM_LIGHT:
		if (INT(obj->value[2]) == 999)
			INT(obj->value[2]) = -1;
		break;

	case ITEM_JUKEBOX:
		for (i = 0; i < 5; i++)
			INT(obj->value[i]) = -1;
		break;
	}
	
	obj->next	= object_list;
	object_list	= obj;
	if (!IS_SET(flags, CO_F_NOCOUNT))
		pObjIndex->count++;
	return obj;
}

OBJ_DATA *create_obj_of(OBJ_INDEX_DATA *pObjIndex, mlstring *owner)
{
	OBJ_DATA *obj = create_obj(pObjIndex, 0);

	mlstr_foreach(&obj->short_descr, cb_xxx_of, owner);
	mlstr_foreach(&obj->description, cb_xxx_of, owner);

	return obj;
}

/* duplicate an object exactly -- except contents */
OBJ_DATA *
clone_obj(OBJ_DATA *parent)
{
	AFFECT_DATA *paf;
	ED_DATA *ed, *ed2;
	OBJ_DATA *clone;

	clone = create_obj(parent->pObjIndex, 0);

	/* start fixing the object */
	free_string(clone->name);
	clone->name 		= str_qdup(parent->name);

	mlstr_cpy(&clone->short_descr, &parent->short_descr);
	mlstr_cpy(&clone->description, &parent->description);
	clone->stat_flags	= parent->stat_flags;
	clone->wear_flags	= parent->wear_flags;
	clone->weight		= parent->weight;
	clone->cost		= parent->cost;
	clone->level		= parent->level;
	clone->condition	= parent->condition;
	clone->material		= str_qdup(parent->material);
	clone->timer		= parent->timer;
	mlstr_cpy(&clone->owner, &parent->owner);

	/*
	 * obj values
	 */
	objval_destroy(parent->item_type, clone->value);
	objval_cpy(parent->item_type, clone->value, parent->value);

	/*
	 * affects
	 */
	for (paf = parent->affected; paf != NULL; paf = paf->next) 
		affect_to_obj(clone, paf);

	/*
	 * extended desc
	 */
	for (ed = parent->ed; ed != NULL; ed = ed->next) {
		ed2		= ed_new();
		ed2->keyword	= str_qdup(ed->keyword);
		mlstr_cpy(&ed2->description, &ed->description);
		ed2->next	= clone->ed;
		clone->ed	= ed2;
	}

	return clone;
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
 * smash '\r', dup '~'
 */
char *fix_string(const char *s)
{
	static char buf[MAX_STRING_LENGTH * 2];
	char *p = buf;

	if (IS_NULLSTR(s))
		return str_empty;

	for (p = buf; *s && p-buf < sizeof(buf)-2; s++) {
		switch (*s) {
		case '\r':
			break;

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
		return "''";

	if (strpbrk(w, " \t") == NULL)
		return w;

	snprintf(buf, sizeof(buf), "'%s'", w);
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
init_mm()
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
	srandom(time(NULL)^getpid());
#endif
	return;
}
 
long number_mm(void)
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
 * Simple linear interpolation.
 */
int interpolate(int level, int value_00, int value_32)
{
	return value_00 + level * (value_32 - value_00) / 32;
}

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

/*
 * Append a string to a file.
 */
void append_file(CHAR_DATA *ch, const char *file, const char *str)
{
	FILE *fp;

	if (IS_NPC(ch) || str[0] == '\0')
		return;

	if ((fp = dfopen(TMP_PATH, file, "a")) == NULL)
		char_puts("Could not open the file!\n", ch);
	else {
		fprintf(fp, "[%5d] %s: %s\n",
		    ch->in_room ? ch->in_room->vnum : 0, ch->name, str);
		fclose(fp);
	}
}

/*
 * This function is here to aid in debugging.
 * If the last expression in a function is another function call,
 *   gcc likes to generate a JMP instead of a CALL.
 * This is called "tail chaining."
 * It hoses the debugger call stack for that call.
 * So I make this the last call in certain critical functions,
 *   where I really need the call stack to be right for debugging!
 *
 * If you don't understand this, then LEAVE IT ALONE.
 * Don't remove any calls to tail_chain anywhere.
 *
 * -- Furey
 */
void tail_chain(void)
{
	return;
}

/*
 * Count all objects in pfiles
 * Remove limited objects (with probability 1/10)
 * Update rating list
 */
void scan_pfiles()
{
	struct dirent *dp;
	DIR *dirp;
	bool eqcheck = dfexist(TMP_PATH, EQCHECK_FILE);
	bool should_clear;
	char fullname [PATH_MAX];

	log(LOG_INFO, "scan_pfiles: start (eqcheck is %s)",
		   eqcheck ? "active" : "inactive");

	if (eqcheck
	&&  dunlink(TMP_PATH, EQCHECK_FILE) < 0)
		log(LOG_INFO, "scan_pfiles: unable to deactivate eq checker (%s)",
			   strerror(errno));

	if ((dirp = opendir(PLAYER_PATH)) == NULL) {
		log(LOG_ERROR, "scan_pfiles: unable to open player directory");
		exit(1);
	}

	for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
		CHAR_DATA *ch;
		OBJ_DATA *obj, *obj_next;
		bool changed;
		struct stat s;

#if defined (LINUX) || defined (WIN32)
		if (strlen(dp->d_name) < 3)
			continue;
#else
		if (dp->d_namlen < 3 || dp->d_type != DT_REG)
			continue;
#endif
		if (strchr(dp->d_name, '.')
		||  (ch = char_load(dp->d_name, LOAD_F_NOCREATE)) == NULL)
			continue;

		changed = FALSE;
		should_clear = FALSE;

		/* Remove limited eq from the pfile if it's two weeks old */

		snprintf(fullname, sizeof(fullname), "%s%c%s", PLAYER_PATH, 
			PATH_SEPARATOR, dp->d_name);
		if (stat(fullname, &s) < 0) {
			log(LOG_ERROR, "scan_pfiles: unable to stat %s.", fullname);
		} else {
			should_clear = (time(NULL) - s.st_mtime) > 60*60*24*14;
		}

		for (obj = ch->carrying; obj; obj = obj_next) {
			obj_next = obj->next_content;

			obj->pObjIndex->count++;

			if (obj->pObjIndex->limit < 0
			|| !should_clear)
				continue;

			changed = TRUE;
			log(LOG_INFO, "scan_pfiles: %s: %s (vnum %d)",
				   ch->name,
				   mlstr_mval(&obj->pObjIndex->short_descr),
				   obj->pObjIndex->vnum);
			extract_obj(obj, XO_F_NORECURSE);
		}

		if (!IS_IMMORTAL(ch))
			rating_add(ch);

		if (changed || PC(ch)->version < PFILE_VERSION)
			char_save(ch, SAVE_F_PSCAN);

		char_nuke(ch);
	}
	closedir(dirp);

	log(LOG_INFO, "scan_pfiles: end (eqcheck is %s)",
		   dfexist(TMP_PATH, EQCHECK_FILE) ? "active" : "inactive");
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

const char *GETMSG(const char *msg, int lang)
{
	mlstring *ml = msg_lookup(msg);
	if (ml == NULL)
		return msg;
	return mlstr_val(ml, lang);
}

