/*-
 * Copyright (c) 1998 SoG Development Team
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
 * $Id: const.h,v 1.45.2.11 2000-08-17 12:14:05 fjoe Exp $
 */

#ifndef _CONST_H_
#define _CONST_H_

/*
 * Data files used by the server.
 *
 * AREA_LIST contains a list of areas to boot.
 * All files are read in completely at bootup.
 * Most output files (bug, idea, typo, shutdown) are append-only.
 *
 * The NULL_FILE is held open so that we have a stream handle in reserve,
 *   so players can go ahead and telnet to all the other descriptors.
 * Then we close it whenever we need to open a file (e.g. a save file).
 */

#if defined (WIN32)
#	include <stdlib.h>
#	define PATH_MAX	_MAX_PATH
#	define PATH_SEPARATOR '\\'
#else
#	define PATH_SEPARATOR '/'
#endif

#define PFILE_VERSION	12

/*
 * the following path/file name consts are defined in db.c
 */
extern const char TMP_PATH	[];
extern const char PLAYER_PATH	[];
extern const char GODS_PATH	[];
extern const char NOTES_PATH	[];
extern const char ETC_PATH	[];
extern const char CLASSES_PATH	[];
extern const char CLANS_PATH	[];
extern const char RACES_PATH	[];
extern const char AREA_PATH	[];
extern const char LANG_PATH	[];
extern const char PLISTS_PATH	[];
extern const char MODULES_PATH	[];

extern const char TMP_FILE	[];
extern const char NULL_FILE	[];

extern const char HOMETOWNS_CONF[];
extern const char SKILLS_CONF	[];
extern const char RSPELLS_CONF	[];
extern const char SOCIALS_CONF	[];
extern const char SYSTEM_CONF	[];
extern const char LANG_CONF	[];
extern const char MSGDB_CONF	[];
extern const char CMD_CONF	[];

extern const char AREA_LIST	[];
extern const char CLAN_LIST	[];
extern const char CLASS_LIST	[];
extern const char LANG_LIST	[];
extern const char RACE_LIST	[];

extern const char BUG_FILE	[];
extern const char TYPO_FILE	[];
extern const char NOTE_FILE	[];
extern const char IDEA_FILE	[];
extern const char PENALTY_FILE	[];
extern const char NEWS_FILE	[];
extern const char CHANGES_FILE	[];
extern const char SHUTDOWN_FILE	[];
extern const char EQCHECK_SAVE_ALL_FILE [];
extern const char BAN_FILE	[];
extern const char MAXON_FILE	[];
extern const char AREASTAT_FILE	[];
extern const char IMMLOG_FILE	[];
/*
 * sex or gender
 */
enum {
	SEX_NEUTRAL,
	SEX_MALE,
	SEX_FEMALE,
	SEX_EITHER,		/* used only for NPC, means random sex
				   in create_mob */
	SEX_PLURAL
};

/* align numbers */
enum {
	ANUM_GOOD,
	ANUM_NEUTRAL,
	ANUM_EVIL,

	MAX_ANUM
};

enum {
	STAT_STR,
	STAT_INT,
	STAT_WIS,
	STAT_DEX,
	STAT_CON,
	STAT_CHA,

	MAX_STAT			/* number of char stats */
};

#define MAX_STATS	MAX_STAT	/* ROM compatibility */

/*
 * String and memory management parameters.
 */
#define MAX_KEY_HASH		1024
#define MAX_STRING_HASH		16384

#define MAX_STRING_LENGTH	8192
#define MAX_INPUT_LENGTH	1024

#define MAX_PROMPT_LENGTH	60
#define MAX_TITLE_LENGTH	45
#define MAX_CHAR_NAME		12
#define DEFAULT_PAGELEN		22
#define MIN_PAGELEN		16
#define MAX_PAGELEN		192

/*
 * Game parameters.
 */
#define MAX_RELIGION		18
#define MAX_NEWBIES		120	/* number of newbies allowed */
#define MAX_OLDIES		999	/* number of newbies allowed */
#define MAX_TRADE		5	/* number of trade types for shops */
#define MAX_COND		6	/* number of char condition stats */
#define MAX_DIR			6	/* number of exits */
#define MAX_ALIAS		50	/* number of aliases char can have */

#define MAX_LEVEL		100

#define LEVEL_IMP		MAX_LEVEL	/* implementor		*/
#define LEVEL_CRE 		(MAX_LEVEL - 1)	/* creator		*/
#define LEVEL_SUP 		(MAX_LEVEL - 2)	/* supreme being	*/
#define LEVEL_DEI		(MAX_LEVEL - 3)	/* deity		*/
#define LEVEL_GOD		(MAX_LEVEL - 4)	/* god			*/
#define LEVEL_IMM		(MAX_LEVEL - 5)	/* immortal		*/
#define LEVEL_DEM		(MAX_LEVEL - 6)	/* demigod		*/
#define LEVEL_ANG		(MAX_LEVEL - 7)	/* angel		*/
#define LEVEL_AVA		(MAX_LEVEL - 8)	/* avatar		*/
#define LEVEL_HERO		(MAX_LEVEL - 9)	/* hero			*/

#define LEVEL_IMMORTAL		LEVEL_AVA
#define LEVEL_NEWBIE		5
#define LEVEL_PK		10	/* min PK level */

#undef ANATOLIA_MACHINE
#if defined(ANATOLIA_MACHINE)
#	define PULSE_PER_SCD		6  /* 6 for comm.c */
#	define PULSE_PER_SECOND		4  /* for update.c */
#	define PULSE_VIOLENCE		(2 *  PULSE_PER_SECOND)
#else
#	define PULSE_PER_SCD		4
#	define PULSE_PER_SECOND		4
#	define PULSE_VIOLENCE		(3 * PULSE_PER_SECOND)
#endif

#define PULSE_MOBILE		(4 * PULSE_PER_SECOND)
#define PULSE_WATER_FLOAT	(4 * PULSE_PER_SECOND)
#define PULSE_MUSIC		(6 * PULSE_PER_SECOND)
#define PULSE_TRACK		(12 * PULSE_PER_SECOND)
#define PULSE_TICK		(50 * PULSE_PER_SECOND) /* 36 saniye */

/* room_affect_update (not room_update) */
#define PULSE_RAFFECT		(3 * PULSE_MOBILE)
#define PULSE_AREA		(110 * PULSE_PER_SECOND) /* 97 saniye */
#define PULSE_AUCTION		(20 * PULSE_PER_SECOND) /* 20 seconds */

#define FIGHT_DELAY_TIME	(20 * PULSE_PER_SECOND)
#define GHOST_DELAY_TIME	600
#define MISSING_TARGET_DELAY	10

/*
 * Well known mob virtual numbers.
 * Defined in #MOBILES.
 */
#define MOB_VNUM_KITTEN			3090

#define MOB_VNUM_SHADOW 		10
#define MOB_VNUM_SPECIAL_GUARD		11
#define MOB_VNUM_BEAR			12
#define MOB_VNUM_DEMON			13
#define MOB_VNUM_NIGHTWALKER		14
#define MOB_VNUM_STALKER		15
#define MOB_VNUM_SQUIRE 		16
#define MOB_VNUM_MIRROR_IMAGE		17
#define MOB_VNUM_UNDEAD 		18
#define MOB_VNUM_LION			19
#define MOB_VNUM_WOLF			20
#define MOB_VNUM_SUM_SHADOW		26
#define MOB_VNUM_COCOON			27
#define MOB_VNUM_BONE_DRAGON		28
#define MOB_VNUM_CRYPT_THING		32 

#define MOB_VNUM_LESSER_GOLEM		21
#define MOB_VNUM_STONE_GOLEM		22
#define MOB_VNUM_IRON_GOLEM		23
#define MOB_VNUM_ADAMANTITE_GOLEM	24

#define MOB_VNUM_HUNTER 		25
#define MOB_VNUM_BLACK_CAT		30
#define MOB_VNUM_BLACK_CROW		31

#define MOB_VNUM_PATROLMAN		2106
#define GROUP_VNUM_TROLLS		2100
#define GROUP_VNUM_OGRES		2101


/*
 * Well known object virtual numbers.
 * Defined in #OBJECTS.
 */
#define OBJ_VNUM_SILVER_ONE		1
#define OBJ_VNUM_GOLD_ONE		2
#define OBJ_VNUM_GOLD_SOME		3
#define OBJ_VNUM_SILVER_SOME		4
#define OBJ_VNUM_COINS			5

#define OBJ_VNUM_CROSS			8
#define OBJ_VNUM_CORPSE_NPC		10
#define OBJ_VNUM_CORPSE_PC		11
#define OBJ_VNUM_SEVERED_HEAD		12
#define OBJ_VNUM_TORN_HEART		13
#define OBJ_VNUM_SLICED_ARM		14
#define OBJ_VNUM_SLICED_LEG		15
#define OBJ_VNUM_GUTS			16
#define OBJ_VNUM_BRAINS			17

#define OBJ_VNUM_MUSHROOM		20
#define OBJ_VNUM_LIGHT_BALL		21
#define OBJ_VNUM_SPRING 		22
#define OBJ_VNUM_DISC			23
#define OBJ_VNUM_PORTAL 		25
#define OBJ_VNUM_DUMMY			30
#define OBJ_VNUM_ROSE			1001
#define OBJ_VNUM_HOLY_HAMMER		18

#define OBJ_VNUM_SCHOOL_VEST		3703
#define OBJ_VNUM_SCHOOL_SHIELD		3704
#define OBJ_VNUM_SCHOOL_BANNER		3716

#define OBJ_VNUM_MAP			3162
#define OBJ_VNUM_NMAP1			3385
#define OBJ_VNUM_NMAP2			3386

#define OBJ_VNUM_WHISTLE		2116

#define OBJ_VNUM_POTION_VIAL		42
#define OBJ_VNUM_STEAK			27
#define OBJ_VNUM_RANGER_STAFF		28
#define OBJ_VNUM_WOODEN_ARROW		6
#define OBJ_VNUM_GREEN_ARROW		34
#define OBJ_VNUM_RED_ARROW		35
#define OBJ_VNUM_WHITE_ARROW		36
#define OBJ_VNUM_BLUE_ARROW		37
#define OBJ_VNUM_RANGER_BOW		7

#define OBJ_VNUM_DEPUTY_BADGE		70
#define OBJ_VNUM_ENFORCER_BADGE		70
#define OBJ_VNUM_ENFORCER_SHIELD1	71
#define OBJ_VNUM_ENFORCER_SHIELD2	72
#define OBJ_VNUM_ENFORCER_SHIELD3	73
#define OBJ_VNUM_ENFORCER_SHIELD4	74
#define OBJ_VNUM_RENEGADE_MARK		76

#define OBJ_VNUM_CHAOS_WEAPON		97

#define OBJ_VNUM_DRAGONDAGGER		80
#define OBJ_VNUM_DRAGONMACE		81
#define OBJ_VNUM_PLATE			82
#define OBJ_VNUM_DRAGONSWORD		83
#define OBJ_VNUM_DRAGONLANCE		99

#define OBJ_VNUM_BATTLE_PONCHO		26

#define OBJ_VNUM_POTION_SILVER		43
#define OBJ_VNUM_POTION_GOLDEN		44
#define OBJ_VNUM_POTION_SWIRLING	45
#define OBJ_VNUM_KATANA_SWORD		98

#define OBJ_VNUM_EYED_SWORD		88
#define OBJ_VNUM_FIRE_SHIELD		92
#define OBJ_VNUM_MAGIC_JAR		93
#define OBJ_VNUM_HAMMER 		6522

#define OBJ_VNUM_CHUNK_IRON		6521

/*
 * Well known room virtual numbers.
 * Defined in #ROOMS.
 */
#define ROOM_VNUM_LIMBO 	2
#define ROOM_VNUM_CHAT		1200
#define ROOM_VNUM_TEMPLE	3001
#define ROOM_VNUM_SCHOOL	3700

/* RT ASCII conversions */

#define A	((flag64_t) 1 <<  0)
#define B	((flag64_t) 1 <<  1)
#define C	((flag64_t) 1 <<  2)
#define D	((flag64_t) 1 <<  3)
#define E	((flag64_t) 1 <<  4)
#define F	((flag64_t) 1 <<  5)
#define G	((flag64_t) 1 <<  6)
#define H	((flag64_t) 1 <<  7)

#define I	((flag64_t) 1 <<  8)
#define J	((flag64_t) 1 <<  9)
#define K	((flag64_t) 1 << 10)
#define L	((flag64_t) 1 << 11)
#define M	((flag64_t) 1 << 12)
#define N	((flag64_t) 1 << 13)
#define O	((flag64_t) 1 << 14)
#define P	((flag64_t) 1 << 15)

#define Q	((flag64_t) 1 << 16)
#define R	((flag64_t) 1 << 17)
#define S	((flag64_t) 1 << 18)
#define T	((flag64_t) 1 << 19)
#define U	((flag64_t) 1 << 20)
#define V	((flag64_t) 1 << 21)
#define W	((flag64_t) 1 << 22)
#define X	((flag64_t) 1 << 23)

#define Y	((flag64_t) 1 << 24)
#define Z	((flag64_t) 1 << 25)
#define aa	((flag64_t) 1 << 26) /* letters doubled due to conflicts */
#define bb	((flag64_t) 1 << 27)
#define cc	((flag64_t) 1 << 28)
#define dd	((flag64_t) 1 << 29)
#define ee	((flag64_t) 1 << 30)
#define ff	((flag64_t) 1 << 31)

#define gg	((flag64_t) 1 << 32)
#define hh	((flag64_t) 1 << 33)
#define ii	((flag64_t) 1 << 34)
#define jj	((flag64_t) 1 << 35)
#define kk	((flag64_t) 1 << 36)
#define ll	((flag64_t) 1 << 37)
#define mm	((flag64_t) 1 << 38)
#define nn	((flag64_t) 1 << 39)

#define oo	((flag64_t) 1 << 40)
#define pp	((flag64_t) 1 << 41)
#define qq	((flag64_t) 1 << 42)
#define rr	((flag64_t) 1 << 43)
#define ss	((flag64_t) 1 << 44)
#define tt	((flag64_t) 1 << 45)
#define uu	((flag64_t) 1 << 46)
#define vv	((flag64_t) 1 << 47)

#define ww	((flag64_t) 1 << 48)
#define xx	((flag64_t) 1 << 49)
#define yy	((flag64_t) 1 << 50)
#define zz	((flag64_t) 1 << 51)
#define aaa	((flag64_t) 1 << 52)
#define bbb	((flag64_t) 1 << 53)
#define ccc	((flag64_t) 1 << 54)
#define ddd	((flag64_t) 1 << 55)
#define eee	((flag64_t) 1 << 56)
#define fff	((flag64_t) 1 << 57)
#define ggg	((flag64_t) 1 << 58)
#define hhh	((flag64_t) 1 << 59)
#define iii	((flag64_t) 1 << 60)
#define jjj	((flag64_t) 1 << 61)
#define kkk	((flag64_t) 1 << 62)
#define lll	((flag64_t) 1 << 63)
#endif
