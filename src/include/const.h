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
 * $Id: const.h,v 1.57 2000-03-03 04:09:08 avn Exp $
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

#define PFILE_VERSION	11
#define AREA_VERSION	2

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
 * Conditions.
 */
enum {
	COND_DRUNK,
	COND_FULL,
	COND_THIRST,
	COND_HUNGER,
	COND_BLOODLUST,
	COND_DESIRE,
	MAX_COND
};


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
#define MAX_DIR			6	/* number of exits */
#define MAX_ALIAS		50	/* number of aliases char can have */

#define MAX_LEVEL		50

#define LEVEL_IMP		MAX_LEVEL	/* implementor		*/
#define LEVEL_CRE 		(MAX_LEVEL - 1)	/* creator		*/
#define LEVEL_DEI		(MAX_LEVEL - 2)	/* deity		*/
#define LEVEL_GOD		(MAX_LEVEL - 3)	/* god			*/
#define LEVEL_AVA		(MAX_LEVEL - 4)	/* avatar		*/
#define LEVEL_HERO		(MAX_LEVEL - 5)	/* hero			*/

#define LEVEL_IMMORTAL		LEVEL_AVA
#define LEVEL_NEWBIE		5
#define LEVEL_PK		10	/* min PK level */

#define PULSE_PER_SCD		4
#define PULSE_PER_SECOND	4

#define FIGHT_DELAY_TIME	(20 * PULSE_PER_SECOND)
#define OFFENCE_DELAY_TIME	600
#define GHOST_DELAY_TIME	600
#define MISSING_TARGET_DELAY	10

extern const char BUG_FILE	[];
extern const char TYPO_FILE	[];

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
#define OBJ_VNUM_RULER_BADGE		70
#define OBJ_VNUM_RULER_SHIELD1		71
#define OBJ_VNUM_RULER_SHIELD2		72
#define OBJ_VNUM_RULER_SHIELD3		73
#define OBJ_VNUM_RULER_SHIELD4		74

#define OBJ_VNUM_CHAOS_BLADE		97

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

#define A	((flag_t) 1 <<  0)
#define B	((flag_t) 1 <<  1)
#define C	((flag_t) 1 <<  2)
#define D	((flag_t) 1 <<  3)
#define E	((flag_t) 1 <<  4)
#define F	((flag_t) 1 <<  5)
#define G	((flag_t) 1 <<  6)
#define H	((flag_t) 1 <<  7)

#define I	((flag_t) 1 <<  8)
#define J	((flag_t) 1 <<  9)
#define K	((flag_t) 1 << 10)
#define L	((flag_t) 1 << 11)
#define M	((flag_t) 1 << 12)
#define N	((flag_t) 1 << 13)
#define O	((flag_t) 1 << 14)
#define P	((flag_t) 1 << 15)

#define Q	((flag_t) 1 << 16)
#define R	((flag_t) 1 << 17)
#define S	((flag_t) 1 << 18)
#define T	((flag_t) 1 << 19)
#define U	((flag_t) 1 << 20)
#define V	((flag_t) 1 << 21)
#define W	((flag_t) 1 << 22)
#define X	((flag_t) 1 << 23)

#define Y	((flag_t) 1 << 24)
#define Z	((flag_t) 1 << 25)
#define aa	((flag_t) 1 << 26) /* letters doubled due to conflicts */
#define bb	((flag_t) 1 << 27)
#define cc	((flag_t) 1 << 28)
#define dd	((flag_t) 1 << 29)
#define ee	((flag_t) 1 << 30)

#endif
