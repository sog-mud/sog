/*
 * $Id: db.c,v 1.93 1998-12-22 18:00:14 fjoe Exp $
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
#include "rating.h"
#include "update.h"
#include "db.h"

#ifdef SUNOS
#	include "compat.h"
#	define d_namlen d_reclen
#endif

#ifdef SVR4
#	define d_namlen d_reclen
#endif

void load_limited_objects();

extern	int	_filbuf		(FILE *);

#if !defined(OLD_RAND)

#if defined(BSD44)
#	include <unistd.h>
#elif defined(WIN32)
#	define random()		rand()
#	define srandom( x ) srand( x )
	int getpid();
	time_t time(time_t *tloc);
#elif defined(LINUX)
	long random();
	void srandom(unsigned int);
	int getpid();
	time_t time(time_t *tloc);
#endif

#endif


/* externals for counting purposes */
extern  DESCRIPTOR_DATA *descriptor_free;

#if defined (WIN32)
	const char PATH_SEPARATOR = (char) '\\';
#else
	const char PATH_SEPARATOR = (char) '/';
#endif

/*
 * Globals.
 */
SHOP_DATA *		shop_first;
SHOP_DATA *		shop_last;

NOTE_DATA *		note_free;

CHAR_DATA *		char_list;
KILL_DATA		kill_table	[MAX_LEVEL];
NOTE_DATA *		note_list;
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
int 			mobile_count = 0;
int			newmobs = 0;
int			newobjs = 0;

int	nAllocBuf;
int	sAllocBuf;

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

#define CREATE_NOCOUNT	(A)
#define CREATE_NAMED	(B)

int dbfuncmp(const void *p1, const void *p2)
{
	return str_cmp(*(char**)p1, *(char**)p2);
}

int dbfun_qsort(DBFUN *dbfun_table)
{
	int dbfun_count = 0;
	while(dbfun_table[dbfun_count].name)
		dbfun_count++;
	qsort(dbfun_table, dbfun_count, sizeof(*dbfun_table), dbfuncmp);
	return dbfun_count;
}

void db_parse_file(const char *path, const char *file,
		   DBFUN *dbfun_table, int dbfun_count)
{
	FILE *fp;

	snprintf(filename, sizeof(filename), "%s%c%s", path, PATH_SEPARATOR, file);
	if ((fp = fopen(filename, "r")) == NULL) {
		perror(filename);
		exit(1);
	}

	line_number = 1;

	for (; ;) {
		DBFUN *fn;
		char *word;

		if (fread_letter(fp) != '#') 
			db_error("db_parse_file", "'#' not found");

		word = fread_word(fp);
		if (word[0] == '$')
			break;
		fn = bsearch(&word, dbfun_table, dbfun_count,
			     sizeof(*dbfun_table), dbfuncmp);

		if (fn) 
			fn->fun(fp);
		else {
			log("boot_db: bad section name.");
			exit(1);
		}
	}
	fclose(fp);
}

void db_load_file(const char *path, const char *file,
		  DBFUN *dbfun_table, DBINIT_FUN *dbinit)
{
	int dbfun_count = dbfun_qsort(dbfun_table);
	if (dbinit)
		dbinit();
	db_parse_file(path, file, dbfun_table, dbfun_count);
}

void db_load_list(const char *path, const char *file,
		  DBFUN *dbfun_table, DBINIT_FUN *dbinit)
{
	int dbfun_count;
	FILE *fp;

	if ((fp = dfopen(path, file, "r")) == NULL) {
		perror(file);
		exit(1);
	}

	dbfun_count = dbfun_qsort(dbfun_table);
	for (; ;) {
		char *name = fread_word(fp);
		if (name[0] == '$')
			break;

		if (dbinit)
			dbinit();
		db_parse_file(path, name, dbfun_table, dbfun_count);
	}
	fclose(fp);
}

/*
 * Big mama top level function.
 */
void boot_db(void)
{
	long lhour, lday, lmonth;

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
	reboot_counter = 1440;	/* 12 hours */

	fBootDb = TRUE;

	db_load_list(LANG_PATH, LANG_LIST, db_load_langs, NULL);
	load_oldmsgdb();
	load_msgdb();
	db_load_file(ETC_PATH, SKILLS_CONF, db_load_skills, NULL);
	namedp_check(gsn_table);
	namedp_check(spellfn_table);
	db_load_list(RACES_PATH, RACE_LIST, db_load_races, init_race);
	db_load_list(CLASSES_PATH, CLASS_LIST, db_load_classes, init_class);
	db_load_list(CLANS_PATH, CLAN_LIST, db_load_clans, NULL);
	db_load_list(AREA_PATH, AREA_LIST, db_load_areas, init_area);

	fBootDb = FALSE;

	/*
	 * Fix up exits.
	 * Declare db booting over.
	 * Reset all areas once.
	 * Load up the songs, notes and ban files.
	 */
	fix_exits();
	check_mob_progs();
	load_limited_objects();

	convert_objects();           /* ROM OLC */
	area_update();
	load_notes();
	load_bans();

}

/*
 * Sets vnum range for area using OLC protection features.
 */
void vnum_check(AREA_DATA *area, int vnum)
{
	if (area->min_vnum == 0 || area->max_vnum == 0) {
		log_printf("%s: min_vnum or max_vnum not assigned",
			   area->file_name);
#if 0
		area->min_vnum = area->max_vnum = vnum;
#endif
	}

	if (vnum < area->min_vnum || vnum > area->max_vnum) {
		log_printf("%s: %d not in area vnum range",
			   area->file_name, vnum);
#if 0
		if (vnum < area->min_vnum)
			area->min_vnum = vnum;
		else
			area->max_vnum = vnum;
#endif
	}
}

/*
 * Adds a reset to a room.  OLC
 * Similar to add_reset in olc.c
 */
void new_reset(ROOM_INDEX_DATA *pR, RESET_DATA *pReset)
{
	RESET_DATA *pr;
 
	if (!pR)
		return;
 
	pr = pR->reset_last;
 
	if (!pr) {
		 pR->reset_first = pReset;
		 pR->reset_last  = pReset;
	}
	else {
		 pR->reset_last->next = pReset;
		 pR->reset_last       = pReset;
		 pR->reset_last->next = NULL;
	}

	top_reset++;
}

/*
 *  Check mobprogs
 */
void check_mob_progs(void)
{
    MOB_INDEX_DATA *pMobIndex;
    MPTRIG        *mptrig;
    int iHash;

    for (iHash = 0; iHash < MAX_KEY_HASH; iHash++)
    {
	for (pMobIndex   = mob_index_hash[iHash];
	      pMobIndex   != NULL;
	      pMobIndex   = pMobIndex->next)
	{
	    for(mptrig = pMobIndex->mptrig_list; mptrig; mptrig = mptrig->next)
	    {
		if (mpcode_lookup(mptrig->vnum) == NULL) {
		    log_printf("check_mob_progs: code vnum %d not found.",
			       mptrig->vnum);
		    exit(1);
		}
	    }
	}
    }
}
 
/*
 * Translate all room exits from virtual to real.
 * Has to be done after all rooms are read in.
 * Check for bad reverse exits.
 */
void fix_exits(void)
{
	extern const int rev_dir [];
	ROOM_INDEX_DATA *pRoomIndex;
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;
	int iHash;
	int door;

	for (iHash = 0; iHash < MAX_KEY_HASH; iHash++)
	{
		for (pRoomIndex  = room_index_hash[iHash];
		      pRoomIndex != NULL;
		      pRoomIndex  = pRoomIndex->next)
		{
		    bool fexit;

		    fexit = FALSE;
		    for (door = 0; door <= 5; door++)
		    {
			if ((pexit = pRoomIndex->exit[door]) != NULL)
			{
			    if (pexit->u1.vnum <= 0 
			    || get_room_index(pexit->u1.vnum) == NULL)
				pexit->u1.to_room = NULL;
			    else
			    {
			   	fexit = TRUE; 
				pexit->u1.to_room = get_room_index(pexit->u1.vnum);
			    }
			}
		    }
		    if (!fexit)
			SET_BIT(pRoomIndex->room_flags,ROOM_NOMOB);
		}
	}

	for (iHash = 0; iHash < MAX_KEY_HASH; iHash++)
	{
		for (pRoomIndex  = room_index_hash[iHash];
		      pRoomIndex != NULL;
		      pRoomIndex  = pRoomIndex->next)
		{
		    for (door = 0; door <= 5; door++)
		    {
			if ((pexit     = pRoomIndex->exit[door]    ) != NULL
			&&   (to_room   = pexit->u1.to_room         ) != NULL
			&&   (pexit_rev = to_room->exit[rev_dir[door]]) != NULL
			&&   pexit_rev->u1.to_room != pRoomIndex 
			&&   (pRoomIndex->vnum < 1200 || pRoomIndex->vnum > 1299))
			    log_printf("fix_exits: %d:%d -> %d:%d -> %d.",
				pRoomIndex->vnum, door,
				to_room->vnum,    rev_dir[door],
				(pexit_rev->u1.to_room == NULL)
				    ? 0 : pexit_rev->u1.to_room->vnum);
		    }
		}
	}

	return;
}

void print_resetmsg(AREA_DATA *pArea)
{
	DESCRIPTOR_DATA *d;
	bool is_empty = mlstr_null(pArea->resetmsg);
	
	for (d = descriptor_list; d != NULL; d = d->next) {
		CHAR_DATA *ch;

		if (d->connected != CON_PLAYING)
			continue;

		ch = d->character;
		if (IS_NPC(ch) || !IS_AWAKE(ch) || ch->in_room->area != pArea)
			continue;

		if (is_empty)
			char_puts("You hear some squeaking sounds...\n", ch);
		else
			char_mlputs(pArea->resetmsg, ch);
	}
}

/*
 * Repopulate areas periodically.
 */
void area_update(void)
{
	AREA_DATA *pArea;

	for (pArea = area_first; pArea != NULL; pArea = pArea->next) {
		if (++pArea->age < 3)
			continue;

		/*
		 * Check age and reset.
		 * Note: Mud School resets every 3 minutes (not 15).
		 */
		if ((!pArea->empty && (pArea->nplayer == 0 || pArea->age >= 15))
		||  pArea->age >= 31) {
			ROOM_INDEX_DATA *pRoomIndex;

			/*
			 * the rain devastates tracks on the ground
			 */
			if (weather_info.sky == SKY_RAINING)  {
				int i;
				DESCRIPTOR_DATA *d;
				CHAR_DATA *ch;
	 			for (d = descriptor_list; d!=NULL; d=d->next)  {
					if (d->connected != CON_PLAYING)
						continue;

					ch = (d->original != NULL) ?
						d->original : d->character;
					if (ch->in_room->area == pArea
					&&  get_skill(ch, gsn_track) > 50
					&& !IS_SET(ch->in_room->room_flags,
								ROOM_INDOORS))
					char_puts("Rain devastates the tracks on the ground.\n", ch);
				}

				for (i = pArea->min_vnum; i < pArea->max_vnum;
									i++) {
					pRoomIndex = get_room_index(i);
					if (pRoomIndex == NULL
					||  IS_SET(pRoomIndex->room_flags,
								ROOM_INDOORS))
						continue;
					room_record("erased", pRoomIndex, -1);  
					if (number_percent() < 50)
						room_record("erased",
							    pRoomIndex, -1);
				}
			}

			reset_area(pArea);
			wiznet_printf(NULL, NULL, WIZ_RESETS, 0, 0,
		    		"%s has just been reset.", pArea->name);

			print_resetmsg(pArea);

			pArea->age = number_range(0, 3);
			pRoomIndex = get_room_index(200);
			if (pRoomIndex != NULL && pArea == pRoomIndex->area)
				pArea->age = 15 - 2;
			pRoomIndex = get_room_index(210);
			if (pRoomIndex != NULL && pArea == pRoomIndex->area)
				pArea->age = 15 - 2;
			pRoomIndex = get_room_index(220);
			if (pRoomIndex != NULL && pArea == pRoomIndex->area)
				pArea->age = 15 - 2;
			pRoomIndex = get_room_index(230);
			if (pRoomIndex != NULL && pArea == pRoomIndex->area)
				pArea->age = 15 - 2;
			pRoomIndex = get_room_index(ROOM_VNUM_SCHOOL);
			if (pRoomIndex != NULL && pArea == pRoomIndex->area)
				pArea->age = 15 - 2;
			else if (pArea->nplayer == 0) 
				pArea->empty = TRUE;
		}
	}
}

/*
 * OLC
 * Reset one room.  Called by reset_area and olc.
 */
void reset_room(ROOM_INDEX_DATA *pRoom)
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
            if ((pExit->u1.to_room != NULL)
              && ((pExit = pExit->u1.to_room->exit[rev_dir[iExit]])))
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
		if (mob->pIndexData == pMobIndex)
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

		pMob->zone = pRoom->area;
            char_to_room(pMob, pRoom);

            LastMob = pMob;
            level  = URANGE(0, pMob->level - 2, LEVEL_HERO - 1); /* -1 ROM */
            last = TRUE;
            break;

        case 'O':
            if (!(pObjIndex = get_obj_index(pReset->arg1)))
            {
                log_printf("reset_room: 'O' 1 : bad vnum %d", pReset->arg1);
                log_printf("reset_room: %c %d %d %d",pReset->arg1, pReset->arg2, pReset->arg3, pReset->arg4);
                continue;
            }

            if (!(pRoomIndex = get_room_index(pReset->arg3)))
            {
                log_printf("reset_room: 'O' 2 : bad vnum %d.", pReset->arg3);
                log_printf("reset_room: %c %d %d %d", pReset->arg1, pReset->arg2, pReset->arg3, pReset->arg4);
                continue;
            }

            if (pRoom->area->nplayer > 0
              || count_obj_list(pObjIndex, pRoom->contents) > 0)
	    {
		last = FALSE;
		break;
	    }
		    if ((pObjIndex->limit != -1) &&
		         (pObjIndex->count >= pObjIndex->limit))
		      {
		        last = FALSE;
		        break;
		      }

            pObj = create_obj(pObjIndex,              /* UMIN - ROM OLC */
				  UMIN(number_fuzzy(level), LEVEL_HERO -1));
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

            if (pRoom->area->nplayer > 0
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
		         (pObjIndex->count >= pObjIndex->limit))
		      {
		        last = FALSE;
		        dump_to_scr("Reseting area: [P] OBJ limit reached\n");
		        break;
		      }

	    while (count < pReset->arg4)
	    {
            pObj = create_obj(pObjIndex, number_fuzzy(LastObj->level));
            obj_to_obj(pObj, LastObj);
		count++;
		if (pObjIndex->count >= limit)
		    break;
	    }

	    /* fix object lock state! */
	    LastObj->value[1] = LastObj->pIndexData->value[1];
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

            if (LastMob->pIndexData->pShop)   /* Shop-keeper? */
            {
                int olevel=0;

                pObj = create_obj(pObjIndex, olevel);
		SET_BIT(pObj->extra_flags, ITEM_INVENTORY);  /* ROM OLC */
            }
#if 0
	    else   /* ROM OLC else version */
	    {
		int limit;
		if (pReset->arg2 > 50)  /* old format */
		    limit = 6;
		else if (pReset->arg2 == -1 || pReset->arg2 == 0)  /* no limit */
		    limit = 999;
		else
		    limit = pReset->arg2;

		if (pObjIndex->count < limit || number_range(0,4) == 0)
		{
		    pObj = create_obj(pObjIndex, 
			   UMIN(number_fuzzy(level), LEVEL_HERO - 1));
		    /* error message if it is too high */
		    if (pObj->level > LastMob->level + 3
		    ||  (pObjIndex->item_type == ITEM_WEAPON 
		    &&   pReset->command == 'E' 
		    &&   pObj->level < LastMob->level -5 && pObj->level < 45))
			fprintf(stderr,
				"Err: obj %s (%d) -- %d, mob %s (%d) -- %d\n",
				mlstr_mval(pObj->short_descr),
				pObj->pIndexData->vnum,pObj->level,
				mlstr_mval(LastMob->short_descr),
				LastMob->pIndexData->vnum,LastMob->level);
		}
		else
		    break;
	    }
#endif									 
#if 0 /* envy else version */
            else
            {
                pObj = create_obj(pObjIndex, number_fuzzy(level));
            }
#endif /* envy else version */

/* Anatolia else version */
		else {
		        if ((pObjIndex->limit == -1)  ||
		          (pObjIndex->count < pObjIndex->limit))
		          pObj=create_obj(pObjIndex,UMIN(number_fuzzy(level),
		                                           LEVEL_HERO - 1));
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
			reset_room(pRoom);
}

/*
 * Create an instance of a mobile.
 */
CHAR_DATA *create_mob_org(MOB_INDEX_DATA *pMobIndex, int flags)
{
	CHAR_DATA *mob;
	int i;
	AFFECT_DATA af;

	mobile_count++;

	if (pMobIndex == NULL) {
		bug("Create_mobile: NULL pMobIndex.", 0);
		exit(1);
	}

	mob = new_char();

	mob->pIndexData		= pMobIndex;

	mob->name		= str_dup(pMobIndex->name);
	if (!IS_SET(flags, CREATE_NAMED)) {
		mob->short_descr	= mlstr_dup(pMobIndex->short_descr);
		mob->long_descr		= mlstr_dup(pMobIndex->long_descr);
		mob->description	= mlstr_dup(pMobIndex->description);
	}
	mob->id			= get_mob_id();
	mob->spec_fun		= pMobIndex->spec_fun;
	mob->class		= CLASS_CLERIC;

	if (pMobIndex->wealth) {
		long wealth;

		wealth = number_range(pMobIndex->wealth/2,
				      3 * pMobIndex->wealth/2);
		mob->gold = number_range(wealth/200,wealth/100);
		mob->silver = wealth - (mob->gold * 100);
	} 

	mob->plr_flags		= ACT_NPC;
	mob->comm		= COMM_NOSHOUT | COMM_NOMUSIC;
	mob->affected_by	= pMobIndex->affected_by;
	mob->alignment		= pMobIndex->alignment;
	mob->level		= pMobIndex->level;
	mob->imm_flags		= pMobIndex->imm_flags;
	mob->res_flags		= pMobIndex->res_flags;
	mob->vuln_flags		= pMobIndex->vuln_flags;
	mob->start_pos		= pMobIndex->start_pos;
	mob->position		= mob->start_pos;
	mob->default_pos	= pMobIndex->default_pos;
	mob->race		= pMobIndex->race;
	mob->form		= pMobIndex->form;
	mob->parts		= pMobIndex->parts;
	mob->size		= pMobIndex->size;
	mob->clan		= pMobIndex->clan;
	mob->invis_level	= pMobIndex->invis_level;
 	mob->group		= pMobIndex->group;
	mob->material		= str_dup(pMobIndex->material);

	mob->dam_type		= pMobIndex->dam_type;
	if (mob->dam_type == 0)
		switch(number_range(1,3)) {
		case (1): mob->dam_type = 3;        break;  /* slash */
		case (2): mob->dam_type = 7;        break;  /* pound */
		case (3): mob->dam_type = 11;       break;  /* pierce */
		}

	mob->sex		= pMobIndex->sex;
	if (mob->sex == SEX_EITHER) /* random sex */
		mob->sex = number_range(1,2);

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
	mob->damage[DICE_NUMBER]= pMobIndex->damage[DICE_NUMBER];
	mob->damage[DICE_TYPE]	= pMobIndex->damage[DICE_TYPE];
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
	if (IS_AFFECTED(mob,AFF_SANCTUARY)) {
		af.where	= TO_AFFECTS;
		af.type		= sn_lookup("sanctuary");
		af.level	= mob->level;
		af.duration	= -1;
		af.location	= APPLY_NONE;
		af.modifier	= 0;
		af.bitvector	= AFF_SANCTUARY;
		affect_to_char(mob, &af);
	}

	if (IS_AFFECTED(mob, AFF_HASTE)) {
		af.where	= TO_AFFECTS;
		af.type		= sn_lookup("haste");
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
		af.type		= sn_lookup("protection evil");
		af.level	= mob->level;
		af.duration	= -1;
		af.location	= APPLY_SAVES;
		af.modifier	= -1;
		af.bitvector	= AFF_PROTECT_EVIL;
		affect_to_char(mob, &af);
	}

	if (IS_AFFECTED(mob,AFF_PROTECT_GOOD)) {
		af.where	= TO_AFFECTS;
		af.type		= sn_lookup("protection good");
		af.level	= mob->level;
		af.duration	= -1;
		af.location	= APPLY_SAVES;
		af.modifier	= -1;
		af.bitvector	= AFF_PROTECT_GOOD;
		affect_to_char(mob, &af);
	}  

	/* link the mob to the world list */
	mob->next		= char_list;
	char_list		= mob;
	pMobIndex->count++;
	return mob;
}

CHAR_DATA *create_mob(MOB_INDEX_DATA *pMobIndex)
{
	return create_mob_org(pMobIndex, 0);
}

CHAR_DATA *create_named_mob(MOB_INDEX_DATA *pMobIndex, const char *name)
{
	CHAR_DATA *res;

	res = create_mob_org(pMobIndex, CREATE_NAMED);
	res->short_descr	= mlstr_printf(pMobIndex->short_descr, name);
	res->long_descr		= mlstr_printf(pMobIndex->long_descr, name);
	res->description	= mlstr_printf(pMobIndex->description, name);
	return res;
}

/* duplicate a mobile exactly -- except inventory */
void clone_mob(CHAR_DATA *parent, CHAR_DATA *clone)
{
	int i;
	AFFECT_DATA *paf;

	if (parent == NULL || clone == NULL || !IS_NPC(parent))
		return;
	
	/* start fixing values */ 
	clone->name 		= str_dup(parent->name);
	clone->version		= parent->version;
	clone->short_descr	= mlstr_dup(parent->short_descr);
	clone->long_descr	= mlstr_dup(parent->long_descr);
	clone->description	= mlstr_dup(parent->description);
	clone->group		= parent->group;
	clone->sex		= parent->sex;
	clone->class		= parent->class;
	clone->race		= parent->race;
	clone->level		= parent->level;
	clone->timer		= parent->timer;
	clone->wait		= parent->wait;
	clone->hit		= parent->hit;
	clone->max_hit		= parent->max_hit;
	clone->mana		= parent->mana;
	clone->max_mana		= parent->max_mana;
	clone->move		= parent->move;
	clone->max_move		= parent->max_move;
	clone->gold		= parent->gold;
	clone->silver		= parent->silver;
	clone->exp		= parent->exp;
	clone->comm		= parent->comm;
	clone->imm_flags	= parent->imm_flags;
	clone->res_flags	= parent->res_flags;
	clone->vuln_flags	= parent->vuln_flags;
	clone->invis_level	= parent->invis_level;
	clone->affected_by	= parent->affected_by;
	clone->position		= parent->position;
	clone->practice		= parent->practice;
	clone->train		= parent->train;
	clone->saving_throw	= parent->saving_throw;
	clone->alignment	= parent->alignment;
	clone->hitroll		= parent->hitroll;
	clone->damroll		= parent->damroll;
	clone->wimpy		= parent->wimpy;
	clone->form		= parent->form;
	clone->parts		= parent->parts;
	clone->size		= parent->size;
	clone->material		= str_dup(parent->material);
	clone->dam_type		= parent->dam_type;
	clone->start_pos	= parent->start_pos;
	clone->default_pos	= parent->default_pos;
	clone->spec_fun		= parent->spec_fun;
	clone->status		= parent->status;
	clone->hunting		= NULL;
	clone->clan		= parent->clan;
	clone->invis_level	= parent->invis_level;

	for (i = 0; i < 4; i++)
		clone->armor[i]	= parent->armor[i];

	for (i = 0; i < MAX_STATS; i++)
	{
		clone->perm_stat[i]	= parent->perm_stat[i];
		clone->mod_stat[i]	= parent->mod_stat[i];
	}

	for (i = 0; i < 3; i++)
		clone->damage[i]	= parent->damage[i];

	/* now add the affects */
	for (paf = parent->affected; paf != NULL; paf = paf->next)
		affect_to_char(clone,paf);

}

/*
 * Create an instance of an object.
 */
OBJ_DATA *create_obj_org(OBJ_INDEX_DATA *pObjIndex, int level, int flags)
{
	AFFECT_DATA *paf;
	OBJ_DATA *obj;
	int i;

	if (pObjIndex == NULL) {
		bug("Create_object: NULL pObjIndex.", 0);
		exit(1);
	}

	obj = new_obj();

	obj->pIndexData	= pObjIndex;
 	obj->level = pObjIndex->level;
	obj->wear_loc	= -1;

	if (!IS_SET(flags, CREATE_NAMED)) {
		obj->name		= str_dup(pObjIndex->name);
		obj->short_descr	= mlstr_dup(pObjIndex->short_descr);
		obj->description	= mlstr_dup(pObjIndex->description);
	}
	obj->material		= str_dup(pObjIndex->material);
	obj->extra_flags	= pObjIndex->extra_flags;
	obj->wear_flags		= pObjIndex->wear_flags;
	obj->value[0]		= pObjIndex->value[0];
	obj->value[1]		= pObjIndex->value[1];
	obj->value[2]		= pObjIndex->value[2];
	obj->value[3]		= pObjIndex->value[3];
	obj->value[4]		= pObjIndex->value[4];
	obj->weight		= pObjIndex->weight;
	obj->owner		= str_dup(str_empty); /* used with body parts */
	obj->condition		= pObjIndex->condition;
	obj->cost = pObjIndex->cost;

	/*
	 * Mess with object properties.
	 */
	switch (pObjIndex->item_type) {
	case ITEM_LIGHT:
		if (obj->value[2] == 999)
			obj->value[2] = -1;
		break;

	case ITEM_JUKEBOX:
		for (i = 0; i < 5; i++)
		   obj->value[i] = -1;
		break;
	}
	
	for (paf = pObjIndex->affected; paf != NULL; paf = paf->next) 
		if (paf->location == APPLY_SPELL_AFFECT)
			affect_to_obj(obj,paf);
	
	obj->next	= object_list;
	object_list	= obj;
	if (!IS_SET(flags, CREATE_NOCOUNT))
		pObjIndex->count++;
	return obj;
}

/* 
 * Create an object with modifying the count 
 */
OBJ_DATA *create_obj(OBJ_INDEX_DATA *pObjIndex, int level)
{
	return create_obj_org(pObjIndex, level, 0);
}

OBJ_DATA *create_named_obj(OBJ_INDEX_DATA *pObjIndex, int level,
			   const char *name)
{
	OBJ_DATA *res;

	res = create_obj_org(pObjIndex, level, CREATE_NAMED);
	res->name		= str_printf(pObjIndex->name, name);
	res->short_descr	= mlstr_printf(pObjIndex->short_descr, name);
	res->description	= mlstr_printf(pObjIndex->description, name);
	return res;
}

/*
 * for player load/quit
 * Create an object and do not modify the count 
 */
OBJ_DATA *create_obj_nocount(OBJ_INDEX_DATA *pObjIndex, int level)
{
	return create_obj_org(pObjIndex, level, CREATE_NOCOUNT);
}

/* duplicate an object exactly -- except contents */
void clone_obj(OBJ_DATA *parent, OBJ_DATA *clone)
{
	int i;
	AFFECT_DATA *paf;
	ED_DATA *ed,*ed2;

	if (parent == NULL || clone == NULL)
		return;

	/* start fixing the object */
	clone->name 		= str_dup(parent->name);
	clone->short_descr	= mlstr_dup(parent->short_descr);
	clone->description	= mlstr_dup(parent->description);
	clone->extra_flags	= parent->extra_flags;
	clone->wear_flags	= parent->wear_flags;
	clone->weight		= parent->weight;
	clone->cost		= parent->cost;
	clone->level		= parent->level;
	clone->condition	= parent->condition;
	clone->material		= str_dup(parent->material);
	clone->timer		= parent->timer;
	clone->owner		= parent->owner;
	clone->extracted	= parent->extracted;
	clone->pit		= parent->pit;
	clone->altar		= parent->altar;

	for (i = 0;  i < 5; i ++)
		clone->value[i]	= parent->value[i];

	for (paf = parent->affected; paf != NULL; paf = paf->next) 
		affect_to_obj(clone,paf);

	/* extended desc */
	for (ed = parent->ed; ed != NULL; ed = ed->next) {
		ed2			= ed_new();
		ed2->keyword		= str_dup(ed->keyword);
		ed2->description	= mlstr_dup(ed->description);
		ed2->next		= clone->ed;
		clone->ed		= ed2;
	}

}

/*
 * Get an extra description from a list.
 */
ED_DATA *ed_lookup(const char *name, ED_DATA *ed)
{
	for (; ed != NULL; ed = ed->next) {
		if (is_name(name, ed->keyword))
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

	for (p = buf; p-buf < sizeof(buf)-2 && *s; s++)
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

	*p = '\0';
	return buf;
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
			bug("Fread_string: EOF", 0);
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

void fwrite_string(FILE *fp, const char *name, const char *str)
{
	if (IS_NULLSTR(name))
		fprintf(fp, "%s~\n", fix_string(str));
	else if (!IS_NULLSTR(str))
		fprintf(fp, "%s %s~\n", name, fix_string(str));
}

/*
 * Read a letter from a file.
 */
char fread_letter(FILE *fp)
{
	char c;

	do
		c = xgetc(fp);
	while (isspace(c));
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
	else if (c != ' ')
		xungetc(c, fp);

	return number;
}

flag_t fread_flags(FILE *fp)
{
	flag_t number;
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

flag_t flag_convert(char letter)
{
	flag_t bitsum = 0;
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

	do
		c = xgetc(fp);
	while (c != '\n' && c != '\r');

	do
		c = xgetc(fp);
	while (c == '\n' || c == '\r');

	xungetc(c, fp);
	return;
}

/*
 * Read one word (into static buffer).
 */
char *fread_word(FILE *fp)
{
	static char word[MAX_INPUT_LENGTH];
	char *pword;
	char cEnd;

	do
		cEnd = xgetc(fp);
	while (isspace(cEnd));

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

void *alloc_mem(int sMem)
{
	return calloc(1, sMem);
}

void free_mem(void *p, int sMem)
{
	free(p);
}

void *alloc_perm(int sMem)
{
	return calloc(1, sMem);
}

void do_areas(CHAR_DATA *ch, const char *argument)
{
	AREA_DATA *pArea1;
	AREA_DATA *pArea2;
	int iArea;
	int iAreaHalf;
	BUFFER *output;

	if (argument[0] != '\0') {
		char_puts("No argument is used with this command.\n",ch);
		return;
	}

	iAreaHalf = (top_area + 1) / 2;
	pArea1    = area_first;
	pArea2    = area_first;
	for (iArea = 0; iArea < iAreaHalf; iArea++)
		pArea2 = pArea2->next;

	output = buf_new(-1);
	buf_add(output, "Current areas of Muddy Realms: \n");
	for (iArea = 0; iArea < iAreaHalf; iArea++) {
		buf_printf(output,"{{%2d %3d} {B%-20.20s{x %8.8s ",
			pArea1->min_level,pArea1->max_level,
			pArea1->name,
			pArea1->credits);

		if (pArea2 != NULL) 
			buf_printf(output,"{{%2d %3d} {B%-20.20s{x %8.8s",
				pArea2->min_level,pArea2->max_level,
				pArea2->name,
				pArea2->credits);
		buf_add(output, "\n");

		pArea1 = pArea1->next;
		if (pArea2 != NULL)
			pArea2 = pArea2->next;
	}

	buf_add(output,"\n");	
	page_to_char(buf_string(output), ch);	
	buf_free(output);
}

void do_memory(CHAR_DATA *ch, const char *argument)
{
	extern int mlstr_count;
	extern int mlstr_real_count;
	extern int str_count;
	extern int str_real_count;

	char_printf(ch, "Affects  : %d\n", top_affect );
	char_printf(ch, "Areas    : %d\n", top_area   );
	char_printf(ch, "ExDes    : %d\n", top_ed     );
	char_printf(ch, "Exits    : %d\n", top_exit   );
	char_printf(ch, "Helps    : %d\n", top_help   );
	char_printf(ch, "Socials  : %d\n", social_count);
	char_printf(ch, "Mobs     : %d (%d new format, %d in use)\n",
					top_mob_index, newmobs, mobile_count); 
	char_printf(ch, "Objs     : %d (%d new format)\n",
					top_obj_index, newobjs); 
	char_printf(ch, "Resets   : %d\n", top_reset  );
	char_printf(ch, "Rooms    : %d\n", top_room   );
	char_printf(ch, "Shops    : %d\n", top_shop   );
	char_printf(ch, "Buffers  : %d (%d bytes)\n",
					nAllocBuf, sAllocBuf);
	char_printf(ch, "strings  : %d (%d allocated)\n",
			str_count, str_real_count);
	char_printf(ch, "mlstrings: %d (%d allocated)\n",
			mlstr_count, mlstr_real_count);
}

void do_dump(CHAR_DATA *ch, const char *argument)
{
	int count,count2,num_pcs,aff_count;
	CHAR_DATA *fch;
	MOB_INDEX_DATA *pMobIndex;
	OBJ_DATA *obj;
	OBJ_INDEX_DATA *pObjIndex;
	ROOM_INDEX_DATA *room;
	EXIT_DATA *exit;
	PC_DATA *pc;
	DESCRIPTOR_DATA *d;
	AFFECT_DATA *af;
	FILE *fp;
	int vnum,nMatch = 0;

	/* open file */
	fclose(fpReserve);
	fp = dfopen(TMP_PATH, "mem.dmp", "w");

	/* report use of data structures */
	
	num_pcs = 0;
	aff_count = 0;

	/* mobile prototypes */
	fprintf(fp,"MobProt	%4d (%8d bytes)\n",
		top_mob_index, top_mob_index * (sizeof(*pMobIndex))); 

	/* mobs */
	count = 0;
	for (fch = char_list; fch != NULL; fch = fch->next)
	{
		count++;
		if (fch->pcdata != NULL)
		    num_pcs++;
		for (af = fch->affected; af != NULL; af = af->next)
		    aff_count++;
	}

	fprintf(fp,"Mobs	%4d (%8d bytes)\n",
		count, count * (sizeof(*fch)));

	fprintf(fp,"Pcdata	%4d (%8d bytes)\n",
		num_pcs, num_pcs * (sizeof(*pc)));

	/* descriptors */
	count = 0; count2 = 0;
	for (d = descriptor_list; d != NULL; d = d->next)
		count++;
	for (d= descriptor_free; d != NULL; d = d->next)
		count2++;

	fprintf(fp, "Descs	%4d (%8d bytes), %2d free (%d bytes)\n",
		count, count * (sizeof(*d)), count2, count2 * (sizeof(*d)));

	/* object prototypes */
	for (vnum = 0; nMatch < top_obj_index; vnum++)
		if ((pObjIndex = get_obj_index(vnum)) != NULL)
		{
		    for (af = pObjIndex->affected; af != NULL; af = af->next)
			aff_count++;
		    nMatch++;
		}

	fprintf(fp,"ObjProt	%4d (%8d bytes)\n",
		top_obj_index, top_obj_index * (sizeof(*pObjIndex)));

	/* objects */
	count = 0;
	for (obj = object_list; obj != NULL; obj = obj->next) {
		count++;
		for (af = obj->affected; af != NULL; af = af->next)
		    aff_count++;
	}

	fprintf(fp,"Objs	%4d (%8d bytes)\n",
		count, count * (sizeof(*obj)));

	/* affects */
	fprintf(fp,"Affects	%4d (%8d bytes)\n",
		aff_count, aff_count * (sizeof(*af)));

	/* rooms */
	fprintf(fp,"Rooms	%4d (%8d bytes)\n",
		top_room, top_room * (sizeof(*room)));

	 /* exits */
	fprintf(fp,"Exits	%4d (%8d bytes)\n",
		top_exit, top_exit * (sizeof(*exit)));

	fclose(fp);

	/* start printing out mobile data */
	fp = dfopen(TMP_PATH, "mob.dmp", "w");

	fprintf(fp,"\nMobile Analysis\n");
	fprintf(fp,  "---------------\n");
	nMatch = 0;
	for (vnum = 0; nMatch < top_mob_index; vnum++)
		if ((pMobIndex = get_mob_index(vnum)) != NULL)
		{
		    nMatch++;
		    fprintf(fp,"#%-4d %3d active %3d killed     %s\n",
			pMobIndex->vnum,pMobIndex->count,
			pMobIndex->killed,mlstr_mval(pMobIndex->short_descr));
		}
	fclose(fp);

	/* start printing out object data */
	fp = dfopen(TMP_PATH, "obj.dmp", "w");

	fprintf(fp,"\nObject Analysis\n");
	fprintf(fp,  "---------------\n");
	nMatch = 0;
	for (vnum = 0; nMatch < top_obj_index; vnum++)
		if ((pObjIndex = get_obj_index(vnum)) != NULL)
		{
		    nMatch++;
		    fprintf(fp,"#%-4d %3d active %3d reset      %s\n",
			pObjIndex->vnum,pObjIndex->count,
			pObjIndex->reset_num,
			mlstr_mval(pObjIndex->short_descr));
		}

	/* close file */
	fclose(fp);
	fpReserve = fopen(NULL_FILE, "r");
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

	fclose(fpReserve);
	if ((fp = dfopen(TMP_PATH, file, "a")) == NULL) {
		perror(file);
		char_puts("Could not open the file!\n", ch);
	}
	else {
		fprintf(fp, "[%5d] %s: %s\n",
		    ch->in_room ? ch->in_room->vnum : 0, ch->name, str);
		fclose(fp);
	}

	fpReserve = fopen(NULL_FILE, "r");
	return;
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
 * Add the objects in players not logged on to object count 
 */
void load_limited_objects()
{
	struct dirent *dp;

	DIR *dirp;
	FILE *pfile;
	char letter;
	char *word;
	bool fReadLevel;
	int vnum;

	if ((dirp = opendir(PLAYER_PATH)) == NULL) {
		bug("Load_limited_objects: unable to open player directory.",
		    0);
		exit(1);
	}

	for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
		const char* pname;

#if defined (LINUX) || defined (WIN32)
		if (strlen(dp->d_name) < 3)
			continue;
#else
		if (dp->d_namlen < 3 || dp->d_type != DT_REG)
			continue;
#endif
		fReadLevel = FALSE;
		if ((pfile = dfopen(PLAYER_PATH, dp->d_name, "r")) == NULL) {
			bug("Load_limited_objects: Can't open player file.", 0);
			continue;
		}

		pname = NULL;
		for (letter = fread_letter(pfile); letter != EOF;
						letter = fread_letter(pfile)) {
			if (letter == '#') {
				word = fread_word(pfile);

				if (!str_cmp(word, "O")
				||  !str_cmp(word, "OBJECT")) {
				  fread_word(pfile); 
				  vnum = fread_number(pfile);
				  if (get_obj_index(vnum) != NULL)
				  	get_obj_index(vnum)->count++;
				}
			}
			else if (letter == 'P') {
				word = fread_word(pfile);

				if (!str_cmp(word, "C_Killed")) {
					if (pname == NULL) {
						bug("load_limited_objects: "
						    "PC_Killed before Name "
						    "in pfile", 0);
						exit(1);
					}

					rating_add(pname, fread_number(pfile));
				}	
			} 
			else if (letter == 'N') {
				if (!str_cmp(fread_word(pfile), "ame") == 0
				&&  pname == NULL)
					pname = fread_string(pfile);
			}
		}

		free_string(pname);
		fclose(pfile);
	}
	closedir(dirp);
}

#define NBUF 5
#define NBITS 52

char *format_flags(flag_t flags)
{
	static int cnt;
	static char buf[NBUF][NBITS+1];
	int count, pos = 0;

	cnt = (cnt + 1) % NBUF;

	for (count = 0; count < NBITS;  count++)
		if (IS_SET(flags, (flag_t) 1 << count)) {
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
		log_printf("%s: line %d: %s: %s",
			   filename, line_number, fn, buf);
		exit(1);
	}

	log_printf("%s: %s", fn, buf);
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
	    pObjIndex->value[0] = pObjIndex->cost;
	    break;
    }

    REMOVE_BIT(pObjIndex->extra_flags, ITEM_OLDSTYLE);
    ++newobjs;
}

/*
 * read flag word (not f-word :)
 */
flag_t fread_fword(const FLAG *table, FILE *fp)
{
	char *name = fread_word(fp);

	if (is_number(name))
		return atoi(name);

	return flag_value(table, name);
}

flag_t fread_fstring(const FLAG *table, FILE *fp)
{
	const char *s = fread_string(fp);
	flag_t val;

	if (is_number(s))
		val = atoi(s);
	else
		val = flag_value(table, s);

	free_string(s);
	return val;
}

void *fread_namedp(NAMEDP *table, FILE *fp)
{
	char *name = fread_word(fp);
	NAMEDP *np = namedp_lookup(table, name);

	if (np == NULL)
		db_error("fread_namedp", "%s: unknown named pointer", name);

	np->touched = TRUE;
	return np->p;
}

int fread_clan(FILE *fp)
{
	int cn;
	const char *name;

	name = fread_string(fp);
	cn = cn_lookup(name);
	if (cn < 0) {
		db_error("fread_clan", "%s: unknown clan", name);
		cn = 0;
	}
	free_string(name);
	return cn;
}

