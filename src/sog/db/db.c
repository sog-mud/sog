/*
 * $Id: db.c,v 1.174 1999-10-20 05:49:51 avn Exp $
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
#include <stdarg.h>
#include <ctype.h>
#include <limits.h>
#include <sys/types.h>
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
const char RACES_PATH		[] = "races";
const char CLASSES_PATH		[] = "classes";
const char CLANS_PATH		[] = "clans";
const char AREA_PATH		[] = "area";
const char LANG_PATH		[] = "lang";
const char MODULES_PATH		[] = "modules";
const char SPEC_PATH		[] = "specs";

const char RACE_EXT		[] = "race";
const char CLASS_EXT		[] = "class";
const char SPEC_EXT		[] = "spec";

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
const char MSGDB_CONF		[] = "msgdb.conf";	/* msgdb */
const char CMD_CONF		[] = "cmd.conf";	/* commands */
const char DAMTYPE_CONF		[] = "damtype.conf";	/* damtypes */
const char MATERIALS_CONF	[] = "materials.conf";	/* materials */
const char LIQUIDS_CONF		[] = "liquids.conf";	/* liquids */

const char AREA_LIST		[] = "area.lst";	/* list of areas */
const char CLAN_LIST		[] = "clan.lst";	/* list of clans */
const char LANG_LIST		[] = "lang.lst";	/* list of languages */

const char BUG_FILE		[] = "bugs.txt";	/* 'bug' and bug() */
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

flag32_t		mud_options;

SHOP_DATA *		shop_first;
SHOP_DATA *		shop_last;

CHAR_DATA *		char_list;
CHAR_DATA *		char_list_lastpc;

OBJ_DATA *		object_list;
TIME_INFO_DATA		time_info;
WEATHER_DATA		weather_info;

ROOM_INDEX_DATA	*	top_affected_room;
int			reboot_counter;

/*
 * Locals.
 */
MOB_INDEX_DATA *	mob_index_hash		[MAX_KEY_HASH];
OBJ_INDEX_DATA *	obj_index_hash		[MAX_KEY_HASH];
ROOM_INDEX_DATA *	room_index_hash		[MAX_KEY_HASH];

int			line_number;

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
int			newmobs;
int			newobjs;

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
bool			fBootDb;
char			filename[PATH_MAX];

/*
 * Local booting procedures.
*/
void    init_mm         (void);
 
void	fix_exits	(void);
void    check_mob_progs	(void);

void	reset_area	(AREA_DATA * pArea);

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

/*
 * db_parse_file - parses file using dbdata
 * dbdata->tab_sz should be properly intialized
 */
void db_parse_file(DBDATA *dbdata, const char *path, const char *file)
{
	char buf[PATH_MAX];
	int linenum;
	FILE *fp;

	strnzcpy(buf, sizeof(buf), filename);
	linenum = line_number;
	line_number = 0;
	snprintf(filename, sizeof(filename), "%s%c%s",
		 path, PATH_SEPARATOR, file);

	if ((fp = fopen(filename, "r")) == NULL) {
		db_error("db_parse_file", strerror(errno));
		strnzcpy(filename, sizeof(filename), buf);
		line_number = linenum;
		return;
	}

	for (; ;) {
		DBFUN *fn;
		char *word;

		if (fread_letter(fp) != '#') {
			db_error("db_parse_file", "'#' not found");
			break;
		}

		word = fread_word(fp);
		if (word[0] == '$')
			break;

		fn = dbfun_lookup(dbdata, word);
		if (fn) 
			fn->fun(dbdata, fp, fn->arg);
		else {
			db_error("db_parse_file", "bad section name");
			break;
		}
	}
	fclose(fp);

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
		db_error("db_load_dir", "%s: %s", path, strerror(errno));
		return;
	}

	if (!dbdata->tab_sz)
		dbdata_init(dbdata);

	snprintf(mask, sizeof(mask), "*.%s", ext);

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
	FILE *fp;

	if ((fp = dfopen(path, file, "r")) == NULL)
		exit(1);

	if (!dbdata->tab_sz)
		dbdata_init(dbdata);
	for (; ;) {
		char *name = fread_word(fp);
		if (name[0] == '$')
			break;

		if (dbdata->dbinit)
			dbdata->dbinit(dbdata);
		db_parse_file(dbdata, path, name);
	}
	fclose(fp);
}

void boot_db_system(void)
{
	fBootDb = TRUE;
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
	if (time_info.month >= 7 && time_info.month <=12)
		weather_info.mmhg += number_range(1, 50);
	else
		weather_info.mmhg += number_range(1, 80);

	     if (weather_info.mmhg <=  980) weather_info.sky = SKY_LIGHTNING;
	else if (weather_info.mmhg <= 1000) weather_info.sky = SKY_RAINING;
	else if (weather_info.mmhg <= 1020) weather_info.sky = SKY_CLOUDY;
	else                                weather_info.sky = SKY_CLOUDLESS;

	/* room_affect_data */
	top_affected_room = NULL;
		
	/* reboot counter */
	reboot_counter = 1440;	/* 24 hours */

	db_load_list(&db_langs, LANG_PATH, LANG_LIST);
	db_load_file(&db_cmd, ETC_PATH, CMD_CONF);
	db_load_file(&db_msg, ETC_PATH, MSGDB_CONF);
	db_load_file(&db_socials, ETC_PATH, SOCIALS_CONF);

	db_load_file(&db_skills, ETC_PATH, SKILLS_CONF);
	db_load_dir(&db_spec, SPEC_PATH, SPEC_EXT);
	db_load_file(&db_damtype, ETC_PATH, DAMTYPE_CONF);
	db_load_file(&db_materials, ETC_PATH, MATERIALS_CONF);
	db_load_file(&db_liquids, ETC_PATH, LIQUIDS_CONF);

	db_load_dir(&db_races, RACES_PATH, RACE_EXT);
	db_load_dir(&db_classes, CLASSES_PATH, CLASS_EXT);
	db_load_list(&db_clans, CLANS_PATH, CLAN_LIST);
	db_load_list(&db_areas, AREA_PATH, AREA_LIST);
	db_load_file(&db_hometowns, ETC_PATH, HOMETOWNS_CONF);

	/*
	 * load modules
	 */
	for (i = 0; i < modules.nused; i++) {
		if (mod_load(VARR_GET(&modules, i)) < 0)
			exit(1);
	}

	fBootDb = FALSE;

	/*
	 * Fix up exits.
	 * Declare db booting over.
	 * Reset all areas once.
	 * Load up the songs, notes and ban files.
	 */
	fix_exits();
	check_mob_progs();
	scan_pfiles();

	convert_objects();           /* ROM OLC */
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
		log("%s: min_vnum or max_vnum not assigned",
			   area->file_name);
	}

	if (vnum < area->min_vnum || vnum > area->max_vnum) {
		log("%s: %d not in area vnum range",
			   area->file_name, vnum);
	}
}

/*****************************************************************************
 Name:		reset_add
 Purpose:	Inserts a new reset in the given index slot.
 Called by:	do_resets(olc.c).
 ****************************************************************************/
void reset_add(RESET_DATA *pReset, ROOM_INDEX_DATA *room, int num)
{
	RESET_DATA *reset;
	int iReset = 0;

	if (!room->reset_first) {
		room->reset_first	= pReset;
		room->reset_last	= pReset;
		pReset->next		= NULL;
		return;
	}

	num--;

	if (num == 0)	{ /* First slot (1) selected. */
		pReset->next = room->reset_first;
		room->reset_first = pReset;
		return;
	}

	/*
	 * If negative slot(<= 0 selected) then this will find the last.
	 */
	for (reset = room->reset_first; reset->next; reset = reset->next) {
		if (++iReset == num)
			break;
	}

	pReset->next	= reset->next;
	reset->next	= pReset;
	if (!pReset->next)
		room->reset_last = pReset;
}

/*
 *  Check mobprogs
 */
void check_mob_progs(void)
{
    MOB_INDEX_DATA *mob;
    MPTRIG        *mptrig;
    int iHash;

    for (iHash = 0; iHash < MAX_KEY_HASH; iHash++)
    {
	for (mob = mob_index_hash[iHash]; mob; mob = mob->next)
	{
	    for(mptrig = mob->mptrig_list; mptrig; mptrig = mptrig->next)
	    {
		if (mpcode_lookup(mptrig->vnum) == NULL) {
		    db_error("check_mob_progs", "code vnum %d not found.",
			     mptrig->vnum);
		}
	    }
	}
    }
}
 
void fix_exits_room(ROOM_INDEX_DATA *room)
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

/*
 * OLC
 * Reset one room.  Called by reset_area and olc.
 */
void reset_room(ROOM_INDEX_DATA *pRoom, int flags)
{
    RESET_DATA  *pReset;
    CHAR_DATA   *pMob;
    CHAR_DATA	*mob;
    OBJ_DATA    *pObj;
    CHAR_DATA   *LastMob = NULL;
    OBJ_DATA    *LastObj = NULL;
    int iExit;
    int level = 0;
    bool last;

    if (!pRoom)
        return;

    pMob        = NULL;
    last        = FALSE;
    
    for (iExit = 0;  iExit < MAX_DIR;  iExit++)
    {
        EXIT_DATA *pExit;
        if ((pExit = pRoom->exit[iExit])
	  /*  && !IS_SET(pExit->exit_info, EX_BASHED)   ROM OLC */)  
        {
            pExit->exit_info = pExit->rs_flags;
            if ((pExit->to_room.r != NULL)
              && ((pExit = pExit->to_room.r->exit[rev_dir[iExit]])))
            {
                /* nail the other side */
                pExit->exit_info = pExit->rs_flags;
            }
        }
    }

    for (pReset = pRoom->reset_first; pReset != NULL; pReset = pReset->next)
    {
        MOB_INDEX_DATA  *pMobIndex;
        OBJ_INDEX_DATA  *pObjIndex;
        OBJ_INDEX_DATA  *pObjToIndex;
        ROOM_INDEX_DATA *pRoomIndex;
	int count,limit=0;
	int cln;
	clan_t* clan=NULL;
        EXIT_DATA *pExit;
        int d0;
        int d1;

        switch (pReset->command)
        {
        default:
                bug("Reset_room: bad command %c.", pReset->command);
                break;

        case 'M':
            if (!(pMobIndex = get_mob_index(pReset->arg1)))
            {
                bug("Reset_room: 'M': bad vnum %d.", pReset->arg1);
                continue;
            }

	    if ((pRoomIndex = get_room_index(pReset->arg3)) == NULL)
	    {
		bug("Reset_area: 'R': bad vnum %d.", pReset->arg3);
		continue;
	    }
            if (pMobIndex->count >= pReset->arg2)
            {
                last = FALSE;
                break;
            }
/* */
	    count = 0;
	    for (mob = pRoomIndex->people; mob != NULL; mob = mob->next_in_room)
		if (mob->pMobIndex == pMobIndex)
		{
		    count++;
		    if (count >= pReset->arg4)
		    {
		    	last = FALSE;
		    	break;
		    }
		}

	    if (count >= pReset->arg4)
		break;

		pMob = create_mob(pMobIndex);
		NPC(pMob)->zone = pRoom->area;
		char_to_room(pMob, pRoom);
		if (IS_EXTRACTED(pMob))
			LastMob = NULL;
		else
            		LastMob = pMob;

		level  = URANGE(0, pMob->level - 2, LEVEL_HERO - 1);
		last = TRUE;
		break;

        case 'O':
            if (!(pObjIndex = get_obj_index(pReset->arg1)))
            {
                log("reset_room: 'O' 1 : bad vnum %d", pReset->arg1);
                log("reset_room: %c %d %d %d",pReset->arg1, pReset->arg2, pReset->arg3, pReset->arg4);
                continue;
            }

            if (!(pRoomIndex = get_room_index(pReset->arg3)))
            {
                log("reset_room: 'O' 2 : bad vnum %d.", pReset->arg3);
                log("reset_room: %c %d %d %d", pReset->arg1, pReset->arg2, pReset->arg3, pReset->arg4);
                continue;
            }

            if ((pRoom->area->nplayer > 0 && !IS_SET(flags, RESET_F_NOPCHECK))
            ||  count_obj_list(pObjIndex, pRoom->contents) > 0) {
		last = FALSE;
		break;
	    }
		    if ((pObjIndex->limit != -1) &&
		         (pObjIndex->count >= pObjIndex->limit))
		      {
		        last = FALSE;
		        break;
		      }

            pObj = create_obj(pObjIndex, 0);
            pObj->cost = 0;
            obj_to_room(pObj, pRoom);
	    last = TRUE;
            break;

        case 'P':
		/* (P)ut command
		 * arg1 - vnum of obj to put
		 * arg2
		 * arg3 - vnum of obj to put into
		 * arg4
		 */

            if (!(pObjIndex = get_obj_index(pReset->arg1)))
            {
                bug("Reset_room: 'P': bad vnum %d.", pReset->arg1);
                continue;
            }
	    
            if (!(pObjToIndex = get_obj_index(pReset->arg3)))
            {
                bug("Reset_room: 'P': bad vnum %d.", pReset->arg3);
                continue;
            }

            if (pReset->arg2 > 50) /* old format */
                limit = 6;
            else if (pReset->arg2 == -1) /* no limit */
                limit = 999;
            else
                limit = pReset->arg2;

            if ((pRoom->area->nplayer > 0 && !IS_SET(flags, RESET_F_NOPCHECK))
              || (LastObj = get_obj_type(pObjToIndex)) == NULL
              || (LastObj->in_room == NULL && !last)
              || (pObjIndex->count >= limit && number_range(0,4) != 0) 
              || (count = count_obj_list(pObjIndex, LastObj->contains)) > pReset->arg4 )
	    {
		last = FALSE;
		break;
	    }
				                /* lastObj->level  -  ROM */

		    if ((pObjIndex->limit != -1             ) &&
		         (pObjIndex->count >= pObjIndex->limit)) {
		        last = FALSE;
		        break;
		      }
	    if (IS_SET(pObjIndex->extra_flags, ITEM_CLAN)) {
		for (cln = 0; cln < clans.nused; cln++) 
			if(pObjIndex->vnum == clan_lookup(cln)->obj_vnum)
				clan = clan_lookup(cln);
		if (clan != NULL && clan->obj_ptr == NULL) {
			pObj = create_obj(pObjIndex, 0);
			clan->obj_ptr = pObj;
			clan->altar_ptr = LastObj;
			obj_to_obj(pObj, LastObj);
		}
		continue;
	    }

	    while (count < pReset->arg4)
	    {
            pObj = create_obj(pObjIndex, 0);
            obj_to_obj(pObj, LastObj);
		count++;
		if (pObjIndex->count >= limit)
		    break;
	    }

	    /* fix object lock state! */
	    LastObj->value[1] = LastObj->pObjIndex->value[1];
	    last = TRUE;
            break;

        case 'G':
        case 'E':
            if (!(pObjIndex = get_obj_index(pReset->arg1)))
            {
                bug("Reset_room: 'E' or 'G': bad vnum %d.", pReset->arg1);
                continue;
            }

            if (!last)
                break;

            if (!LastMob)
            {
                bug("Reset_room: 'E' or 'G': null mob for vnum %d.",
                    pReset->arg1);
                last = FALSE;
                break;
            }

            if (LastMob->pMobIndex->pShop) {  /* Shop-keeper? */
                pObj = create_obj(pObjIndex, 0);
		SET_BIT(pObj->extra_flags, ITEM_INVENTORY);
            }
		else {
		        if ((pObjIndex->limit == -1)  ||
		          (pObjIndex->count < pObjIndex->limit))
		          pObj=create_obj(pObjIndex, 0);
		        else break;

		}

            obj_to_char(pObj, LastMob);
            if (pReset->command == 'E')
                equip_char(LastMob, pObj, pReset->arg3);
            last = TRUE;
            break;

        case 'D':
		    if ((pRoomIndex = get_room_index(pReset->arg1)) == NULL)
		    {
			bug("Reset_area: 'D': bad vnum %d.", pReset->arg1);
			continue;
		    }

		    if ((pExit = pRoomIndex->exit[pReset->arg2]) == NULL)
			break;

		    switch (pReset->arg3)
		    {
		    case 0:
			REMOVE_BIT(pExit->exit_info, EX_CLOSED);
			REMOVE_BIT(pExit->exit_info, EX_LOCKED);
			break;

		    case 1:
			SET_BIT(  pExit->exit_info, EX_CLOSED);
			REMOVE_BIT(pExit->exit_info, EX_LOCKED);
			break;

		    case 2:
			SET_BIT(  pExit->exit_info, EX_CLOSED);
			SET_BIT(  pExit->exit_info, EX_LOCKED);
			break;
		    }

		    last = TRUE;
            break;

        case 'R':
            if (!(pRoomIndex = get_room_index(pReset->arg1)))
            {
                bug("Reset_room: 'R': bad vnum %d.", pReset->arg1);
                continue;
            }

            {

                for (d0 = 0; d0 < pReset->arg2 - 1; d0++)
                {
                    d1                   = number_range(d0, pReset->arg2-1);
                    pExit                = pRoomIndex->exit[d0];
                    pRoomIndex->exit[d0] = pRoomIndex->exit[d1];
                    pRoomIndex->exit[d1] = pExit;
                }
            }
            break;
        }
    }
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
        	if ((pRoom = get_room_index(vnum)))
			reset_room(pRoom, 0);
}

static void cb_xxx_of(int lang, const char **p, void *arg)
{
	mlstring *owner = (mlstring*) arg;
	const char *q;

	if (IS_NULLSTR(*p))
		return;

	q = str_printf(*p, word_form(mlstr_val(owner, lang), 1,
				     lang, RULES_CASE));
	free_string(*p);
	*p = q;
}

/*
 * Create an instance of a mobile.
 */
CHAR_DATA *create_mob(MOB_INDEX_DATA *pMobIndex)
{
	CHAR_DATA *mob;
	int i;
	AFFECT_DATA af;

	if (pMobIndex == NULL) {
		bug("Create_mobile: NULL pMobIndex.", 0);
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

	mob->comm		= COMM_NOSHOUT | COMM_NOMUSIC;
	mob->affected_by	= pMobIndex->affected_by;
	mob->alignment		= pMobIndex->alignment;
	mob->level		= pMobIndex->level;
	mob->imm_flags		= pMobIndex->imm_flags;
	mob->res_flags		= pMobIndex->res_flags;
	mob->vuln_flags		= pMobIndex->vuln_flags;
	mob->position		= pMobIndex->start_pos;
	free_string(mob->race);
	mob->race			= str_qdup(pMobIndex->race);
	mob->form		= pMobIndex->form;
	mob->parts		= pMobIndex->parts;
	mob->size		= pMobIndex->size;
	mob->clan		= pMobIndex->clan;
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

	mob->sex		= pMobIndex->sex;
	if (mob->sex == SEX_EITHER) { /* random sex */
		MOB_INDEX_DATA *fmob;
		mob->sex = number_range(SEX_MALE, SEX_FEMALE);
		if (mob->sex == SEX_FEMALE
		&&  (fmob = get_mob_index(pMobIndex->fvnum))) {
			mob->name	= str_qdup(fmob->name);
			mlstr_cpy(&mob->short_descr, &fmob->short_descr);
			mlstr_cpy(&mob->long_descr, &fmob->long_descr);
			mlstr_cpy(&mob->description, &fmob->description);
		}
	}

	for (i = 0; i < MAX_STATS; i ++)
		mob->perm_stat[i] = UMIN(25, 11 + mob->level/4);

	mob->perm_stat[STAT_STR] += mob->size - SIZE_MEDIUM;
	mob->perm_stat[STAT_CON] += (mob->size - SIZE_MEDIUM) / 2;

	mob->hitroll		= (mob->level / 2) + pMobIndex->hitroll;
	mob->damroll		= pMobIndex->damage[DICE_BONUS];
	mob->max_hit		= dice(pMobIndex->hit[DICE_NUMBER],
				       pMobIndex->hit[DICE_TYPE])
				  + pMobIndex->hit[DICE_BONUS];
	mob->hit		= mob->max_hit;
	mob->max_mana		= dice(pMobIndex->mana[DICE_NUMBER],
				       pMobIndex->mana[DICE_TYPE])
				  + pMobIndex->mana[DICE_BONUS];
	mob->mana		= mob->max_mana;
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
		af.location	= APPLY_NONE;
		af.modifier	= 0;
		af.bitvector	= AFF_SANCTUARY;
		affect_to_char(mob, &af);
	}

	if (IS_AFFECTED(mob, AFF_HASTE)) {
		af.where	= TO_AFFECTS;
		af.type		= "haste";
		af.level	= mob->level;
	  	af.duration	= -1;
		af.location	= APPLY_DEX;
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
		af.location	= APPLY_SAVES;
		af.modifier	= -1;
		af.bitvector	= AFF_PROTECT_EVIL;
		affect_to_char(mob, &af);
	}

	if (IS_AFFECTED(mob,AFF_PROTECT_GOOD)) {
		af.where	= TO_AFFECTS;
		af.type		= "protection good";
		af.level	= mob->level;
		af.duration	= -1;
		af.location	= APPLY_SAVES;
		af.modifier	= -1;
		af.bitvector	= AFF_PROTECT_GOOD;
		affect_to_char(mob, &af);
	}  

	/* link the mob to the world list */
	if (char_list_lastpc) {
		mob->next = char_list_lastpc->next;
		char_list_lastpc->next = mob;
	}
	else {
		mob->next = char_list;
		char_list = mob;
	}

	pMobIndex->count++;
	return mob;
}

CHAR_DATA *create_mob_of(MOB_INDEX_DATA *pMobIndex, mlstring *owner)
{
	CHAR_DATA *mob = create_mob(pMobIndex);

	mlstr_foreach(&mob->short_descr, owner, cb_xxx_of);
	mlstr_foreach(&mob->long_descr, owner, cb_xxx_of);
	mlstr_foreach(&mob->description, owner, cb_xxx_of);

	return mob;
}

/* duplicate a mobile exactly -- except inventory */
void clone_mob(CHAR_DATA *parent, CHAR_DATA *clone)
{
	int i;
	AFFECT_DATA *paf;

	if (parent == NULL || clone == NULL || !IS_NPC(parent))
		return;
	
	/* start fixing values */ 
	clone->name 		= str_qdup(parent->name);
	mlstr_cpy(&clone->short_descr, &parent->short_descr);
	mlstr_cpy(&clone->long_descr, &parent->long_descr);
	mlstr_cpy(&clone->description, &parent->description);
	clone->sex		= parent->sex;
	free_string(clone->class);
	clone->class		= str_qdup(parent->class);
	free_string(clone->race);
	clone->race		= str_qdup(parent->race);
	clone->level		= parent->level;
	clone->wait		= parent->wait;
	clone->hit		= parent->hit;
	clone->max_hit		= parent->max_hit;
	clone->mana		= parent->mana;
	clone->max_mana		= parent->max_mana;
	clone->move		= parent->move;
	clone->max_move		= parent->max_move;
	clone->gold		= parent->gold;
	clone->silver		= parent->silver;
	clone->comm		= parent->comm;
	clone->imm_flags	= parent->imm_flags;
	clone->res_flags	= parent->res_flags;
	clone->vuln_flags	= parent->vuln_flags;
	clone->invis_level	= parent->invis_level;
	clone->incog_level	= parent->incog_level;
	clone->affected_by	= parent->affected_by;
	clone->position		= parent->position;
	clone->saving_throw	= parent->saving_throw;
	clone->alignment	= parent->alignment;
	clone->hitroll		= parent->hitroll;
	clone->damroll		= parent->damroll;
	clone->wimpy		= parent->wimpy;
	clone->form		= parent->form;
	clone->parts		= parent->parts;
	clone->size		= parent->size;
	clone->material		= str_qdup(parent->material);
	clone->damtype		= str_qdup(parent->damtype);
	clone->hunting		= NULL;
	clone->clan		= parent->clan;
	NPC(clone)->dam	= NPC(parent)->dam;


	for (i = 0; i < 4; i++)
		clone->armor[i]	= parent->armor[i];

	for (i = 0; i < MAX_STATS; i++) {
		clone->perm_stat[i]	= parent->perm_stat[i];
		clone->mod_stat[i]	= parent->mod_stat[i];
	}

	/* now add the affects */
	for (paf = parent->affected; paf != NULL; paf = paf->next)
		affect_to_char(clone,paf);

}

/*
 * Create an instance of an object.
 */
OBJ_DATA *create_obj(OBJ_INDEX_DATA *pObjIndex, int flags)
{
	AFFECT_DATA *paf;
	OBJ_DATA *obj;
	int i;

	if (pObjIndex == NULL) {
		bug("Create_object: NULL pObjIndex.", 0);
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
	obj->extra_flags	= pObjIndex->extra_flags;
	obj->wear_flags		= pObjIndex->wear_flags;
	objval_cpy(pObjIndex->item_type, obj->value, pObjIndex->value);
	obj->weight		= pObjIndex->weight;
	obj->condition		= pObjIndex->condition;
	obj->cost		= pObjIndex->cost;

	/*
	 * Mess with object properties.
	 */
	switch (pObjIndex->item_type) {
	case ITEM_LIGHT:
		if (INT_VAL(obj->value[2]) == 999)
			INT_VAL(obj->value[2]) = -1;
		break;

	case ITEM_JUKEBOX:
		for (i = 0; i < 5; i++)
			INT_VAL(obj->value[i]) = -1;
		break;
	}
	
	for (paf = pObjIndex->affected; paf != NULL; paf = paf->next) 
		if (paf->where == TO_SKILLS
		||  INT_VAL(paf->location) == APPLY_SPELL_AFFECT)
			SET_BIT(obj->extra_flags, ITEM_ENCHANTED);
	
	if (IS_SET(obj->extra_flags, ITEM_ENCHANTED))
		for (paf = pObjIndex->affected; paf ; paf = paf->next)
			affect_to_obj(obj, paf);
	obj->next	= object_list;
	object_list	= obj;
	if (!IS_SET(flags, CO_F_NOCOUNT))
		pObjIndex->count++;
	return obj;
}

OBJ_DATA *create_obj_of(OBJ_INDEX_DATA *pObjIndex, mlstring *owner)
{
	OBJ_DATA *obj = create_obj(pObjIndex, 0);

	mlstr_foreach(&obj->short_descr, owner, cb_xxx_of);
	mlstr_foreach(&obj->description, owner, cb_xxx_of);

	return obj;
}

/* duplicate an object exactly -- except contents */
void clone_obj(OBJ_DATA *parent, OBJ_DATA *clone)
{
	AFFECT_DATA *paf;
	ED_DATA *ed,*ed2;

	if (parent == NULL || clone == NULL)
		return;

	/* start fixing the object */
	clone->name 		= str_qdup(parent->name);
	mlstr_cpy(&clone->short_descr, &parent->short_descr);
	mlstr_cpy(&clone->description, &parent->description);
	clone->extra_flags	= parent->extra_flags;
	clone->wear_flags	= parent->wear_flags;
	clone->weight		= parent->weight;
	clone->cost		= parent->cost;
	clone->level		= parent->level;
	clone->condition	= parent->condition;
	clone->material		= str_qdup(parent->material);
	clone->timer		= parent->timer;
	mlstr_cpy(&clone->owner, &parent->owner);
	objval_cpy(parent->pObjIndex->item_type, clone->value, parent->value);

	for (paf = parent->affected; paf != NULL; paf = paf->next) 
		affect_to_obj(clone,paf);

	/* extended desc */
	for (ed = parent->ed; ed != NULL; ed = ed->next) {
		ed2		= ed_new();
		ed2->keyword	= str_qdup(ed->keyword);
		mlstr_cpy(&ed2->description, &ed->description);
		ed2->next	= clone->ed;
		clone->ed	= ed2;
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

	if (fBootDb)
		db_error("get_mob_index", "bad vnum %d.", vnum);
		
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

	if (fBootDb)
		db_error("get_obj_index", "bad vnum %d.", vnum);
		
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

	if (fBootDb)
		db_error("get_room_index", "bad vnum %d.", vnum);

	return NULL;
}

int xgetc(FILE *fp)
{
	int c = getc(fp);
	if (c == '\n')
		line_number++;
	return c;
}

void xungetc(int c, FILE *fp)
{
	if (c == '\n')
		line_number--;
	ungetc(c, fp);
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

const char *
smash_spaces(const char *s)
{
	static char buf[2][MAX_STRING_LENGTH];
	static int ind = 0;
	char *p;

	if (IS_NULLSTR(s))
		return str_empty;

	if (strpbrk(s, " \t") == NULL)
		return s;

	ind = (ind + 1) % 2;
	for (p = buf[ind]; *s && p-buf[ind] < sizeof(buf[0])-1; p++, s++) {
		switch (*s) {
		case ' ':
			*p = '_';
			break;

		default:
			*p = *s;
			break;
		}
	}

	*p = '\0';
	return buf[ind];
}

const char *fread_string(FILE *fp)
{
	char buf[MAX_STRING_LENGTH];
	char *plast;
	int c;

	plast = buf;

	/*
	 * Skip blanks.
	 * Read first char.
	 */
	do
		c = xgetc(fp);
	while (isspace(c));

	for (;;) {
		/*
		 * Back off the char type lookup,
		 *   it was too dirty for portability.
		 *   -- Furey
		 */

		if (plast - buf >= sizeof(buf) - 1) {
			bug("fread_string: line too long (truncated)", 0);
			buf[sizeof(buf)-1] = '\0';
			return str_dup(buf);
		}

		switch (c) {
		default:
			*plast++ = c;
			break;
 
		case EOF:
			db_error("fread_string", "EOF");
			return str_empty;
 
		case '\r':
			break;
 
		case '~':
			if ((c = xgetc(fp)) == '~') {
				*plast++ = c;
				break;
			}
			xungetc(c, fp);
			*plast = '\0';
			return str_dup(buf);
		}
		c = xgetc(fp);
	}
}

/*
 * Read a letter from a file.
 */
char fread_letter(FILE *fp)
{
	char c;

	do {
		c = xgetc(fp);
	} while (isspace(c));
	return c;
}

/*
 * Read a number from a file.
 */
int fread_number(FILE *fp)
{
	int number;
	bool sign;
	char c;

	do
		c = xgetc(fp);
	while (isspace(c));

	number = 0;

	sign   = FALSE;
	if (c == '+')
		c = xgetc(fp);
	else if (c == '-') {
		sign = TRUE;
		c = xgetc(fp);
	}

	if (!isdigit(c)) {
		if (fBootDb)
			db_error("fread_number", "bad format");
		log("fread_number: bad format");
		exit(1);
	}

	while (isdigit(c)) {
		number = number * 10 + c - '0';
		c      = xgetc(fp);
	}

	if (sign)
		number = 0 - number;

	if (c == '|')
		number += fread_number(fp);
	else
		xungetc(c, fp);

	return number;
}

flag64_t fread_flags(FILE *fp)
{
	flag64_t number;
	char c;
	bool negative = FALSE;

	do
		c = xgetc(fp);
	while (isspace(c));

	if (c == '-') {
		negative = TRUE;
		c = xgetc(fp);
	}

	number = 0;

	if (!isdigit(c)) {
		while (('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z')) {
			number += flag_convert(c);
			c = xgetc(fp);
		}
	}

	while (isdigit(c)) {
		number = number * 10 + c - '0';
		c = xgetc(fp);
	}

	if (c == '|')
		number += fread_flags(fp);
	else if (c != ' ')
		xungetc(c, fp);

	if (negative)
		return -number;

	return number;
}

flag64_t flag_convert(char letter)
{
	flag64_t bitsum = 0;
	char i;

	if ('A' <= letter && letter <= 'Z') {
		bitsum = A;
		for (i = letter; i > 'A'; i--)
			bitsum <<= 1;
	}
	else if ('a' <= letter && letter <= 'z') {
		bitsum = aa;
		for (i = letter; i > 'a'; i--)
			bitsum <<= 1;
	}

	return bitsum;
}

/*
 * Read to end of line (for comments).
 */
void fread_to_eol(FILE *fp)
{
	char c;

	do {
		c = xgetc(fp);
	} while (c != '\n' && c != '\r');

	do {
		c = xgetc(fp);
	} while (c == '\n' || c == '\r');

	xungetc(c, fp);
}

/*
 * Read one word (into static buffer).
 */
char *fread_word(FILE *fp)
{
	static char word[MAX_INPUT_LENGTH];
	char *pword;
	char cEnd;

	for (;;) {
		do {
			cEnd = xgetc(fp);
		} while (isspace(cEnd));

		if (cEnd == '%') {
			fread_to_eol(fp);
			continue;
		}

		break;
	}

	if (cEnd == '\'' || cEnd == '"')
		pword   = word;
	else {
		word[0] = cEnd;
		pword   = word+1;
		cEnd    = ' ';
	}

	for (; pword < word + MAX_INPUT_LENGTH; pword++)
	{
		*pword = xgetc(fp);
		if (cEnd == ' ' ? isspace(*pword) : *pword == cEnd)
		{
		    if (cEnd == ' ')
			xungetc(*pword, fp);
		    *pword = '\0';
		    return word;
		}
	}

	db_error("fread_word", "word too long");
	return NULL;
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
 
void init_mm()
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

	log("scan_pfiles: start (eqcheck is %s)",
		   eqcheck ? "active" : "inactive");

	if (eqcheck
	&&  dunlink(TMP_PATH, EQCHECK_FILE) < 0)
		log("scan_pfiles: unable to deactivate eq checker (%s)",
			   strerror(errno));

	if ((dirp = opendir(PLAYER_PATH)) == NULL) {
		bug("scan_pfiles: unable to open player directory.",
		    0);
		exit(1);
	}

	for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
		CHAR_DATA *ch;
		OBJ_DATA *obj, *obj_next;
		bool changed;

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

		for (obj = ch->carrying; obj; obj = obj_next) {
			obj_next = obj->next_content;

			obj->pObjIndex->count++;

			if (obj->pObjIndex->limit < 0
			||  !eqcheck
			||  number_percent() < 95)
				continue;

			changed = TRUE;
			log("scan_pfiles: %s: %s (vnum %d)",
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

	log("scan_pfiles: end (eqcheck is %s)",
		   dfexist(TMP_PATH, EQCHECK_FILE) ? "active" : "inactive");
}

void move_pfiles(int minvnum, int maxvnum, int delta)
{
	struct dirent *dp;
	DIR *dirp;

	if ((dirp = opendir(PLAYER_PATH)) == NULL) {
		bug("move_pfiles : unable to open player directory.",
		    0);
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
#define NBITS 52

char *format_flags(flag64_t flags)
{
	static int cnt;
	static char buf[NBUF][NBITS+1];
	int count, pos = 0;

	cnt = (cnt + 1) % NBUF;

	for (count = 0; count < NBITS;  count++)
		if (IS_SET(flags, (flag64_t) 1 << count)) {
	        	if (count < 26)
	        		buf[cnt][pos] = 'A' + count;
	        	else
				buf[cnt][pos] = 'a' + (count - 26);
			pos++;
		}

	if (pos == 0) 
		buf[cnt][pos++] = '0';

	buf[cnt][pos] = '\0';
	return buf[cnt];
}

void db_error(const char* fn, const char* fmt,...)
{
	char buf[MAX_STRING_LENGTH];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	if (fBootDb) {
		log("%s: line %d: %s: %s",
			   filename, line_number, fn, buf);
		exit(1);
	}

	log("%s: %s", fn, buf);
}

/*****************************************************************************
 Name:	        convert_objects
 Purpose:	Converts all old format objects to new format
 Called by:	boot_db (db.c).
 ****************************************************************************/
void convert_objects(void)
{
	int i;
	if (newobjs == top_obj_index)
		return; /* all objects in new format */

	for (i = 0; i < MAX_KEY_HASH; i++) {
		OBJ_INDEX_DATA *pObj;

		for (pObj = obj_index_hash[i]; pObj; pObj = pObj->next)
 			if (IS_SET(pObj->extra_flags, ITEM_OLDSTYLE))
				convert_object(pObj);
	}
}

/*****************************************************************************
 Name:		convert_object
 Purpose:	Converts an ITEM_OLDSTYLE obj to new format
 Called by:	convert_objects (db2.c).
 Note:          Dug out of create_obj (db.c)
 Author:        Hugin
 ****************************************************************************/
void convert_object(OBJ_INDEX_DATA *pObjIndex)
{
    int level;
    int number, type;  /* for dice-conversion */

    level = pObjIndex->level;

    pObjIndex->cost     = 10*level;

    switch (pObjIndex->item_type) {
        default:
            bug("Obj_convert: vnum %d bad type.", pObjIndex->item_type);
            break;

        case ITEM_LIGHT:
		if (INT_VAL(pObjIndex->value[2]) == 999)
			INT_VAL(pObjIndex->value[2]) = -1;
		break;
        case ITEM_TREASURE:
        case ITEM_FURNITURE:
        case ITEM_TRASH:
        case ITEM_CONTAINER:
        case ITEM_DRINK_CON:
        case ITEM_KEY:
        case ITEM_FOOD:
        case ITEM_BOAT:
        case ITEM_CORPSE_NPC:
        case ITEM_CORPSE_PC:
        case ITEM_FOUNTAIN:
        case ITEM_MAP:
        case ITEM_CLOTHING:
        case ITEM_SCROLL:
	    break;

        case ITEM_WAND:
        case ITEM_STAFF:
            pObjIndex->value[2] = pObjIndex->value[1];
	    break;

        case ITEM_WEAPON:

	    /*
	     * The conversion below is based on the values generated
	     * in one_hit() (fight.c).  Since I don't want a lvl 50 
	     * weapon to do 15d3 damage, the min value will be below
	     * the one in one_hit, and to make up for it, I've made 
	     * the max value higher.
	     * (I don't want 15d2 because this will hardly ever roll
	     * 15 or 30, it will only roll damage close to 23.
	     * I can't do 4d8+11, because one_hit there is no dice-
	     * bounus value to set...)
	     *
	     * The conversion below gives:

	     level:   dice      min      max      mean
	       1:     1d8      1(2)    8(7)     5(5)
	       2:     2d5      2(3)   10(8)     6(6)
	       3:     2d5      2(3)   10(8)     6(6)
	       5:     2d6      2(3)   12(10)     7(7)
	      10:     4d5      4(5)   20(14)    12(10)
	      20:     5d5      5(7)   25(21)    15(14)
	      30:     5d7      5(10)   35(29)    20(20)
	      50:     5d11     5(15)   55(44)    30(30)

	     */

	    number = UMIN(level/4 + 1, 5);
	    type   = (level + 7)/number;

            pObjIndex->value[1] = number;
            pObjIndex->value[2] = type;
	    break;

        case ITEM_ARMOR:
            pObjIndex->value[0] = level / 5 + 3;
            pObjIndex->value[1] = pObjIndex->value[0];
            pObjIndex->value[2] = pObjIndex->value[0];
	    break;

        case ITEM_POTION:
        case ITEM_PILL:
            break;

        case ITEM_MONEY:
	    INT_VAL(pObjIndex->value[0]) = pObjIndex->cost;
	    break;
    }

    REMOVE_BIT(pObjIndex->extra_flags, ITEM_OLDSTYLE);
    TOUCH_VNUM(pObjIndex->vnum);
    ++newobjs;
}

/*
 * read flag word (not f-word :)
 */
flag64_t fread_fword(const flag_t *table, FILE *fp)
{
	char *name = fread_word(fp);

	if (is_number(name))
		return atoi(name);

	return flag_value(table, name);
}

void fwrite_ival(FILE *fp, const flag_t *table, const char *name, int val)
{
	const flag_t *f;

	if (!IS_NULLSTR(name))
		fprintf(fp, "%s ", name);

	if ((f = flag_ilookup(table, val)) != NULL)
		fprintf(fp, "'%s'", f->name);
	else
		fprintf(fp, "%d", val);

	if (!IS_NULLSTR(name))
		fputc('\n', fp);
}

flag64_t fread_fstring(const flag_t *table, FILE *fp)
{
	const char *s = fread_string(fp);
	flag64_t val;

	if (is_number(s))
		val = atoi(s);
	else
		val = flag_value(table, s);

	free_string(s);
	return val;
}

int fread_clan(FILE *fp)
{
	int cln;
	const char *name;

	name = fread_string(fp);
	cln = cln_lookup(name);
	if (cln < 0) {
		db_error("fread_clan", "%s: unknown clan", name);
		cln = 0;
	}
	free_string(name);
	return cln;
}

