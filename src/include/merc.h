#ifndef _MERC_H_
#define _MERC_H_

/*
 * $Id: merc.h,v 1.63 1998-08-06 13:50:12 fjoe Exp $
 */

/***************************************************************************
 *     ANATOLIA 2.1 is copyright 1996-1997 Serdar BULUT, Ibrahim CANPUNAR  *
 *     ANATOLIA has been brought to you by ANATOLIA consortium		   *
 *	 Serdar BULUT {Chronos} 	bulut@rorqual.cc.metu.edu.tr	   *
 *	 Ibrahim Canpunar  {Asena}	canpunar@rorqual.cc.metu.edu.tr    *
 *	 Murat BICER  {KIO}		mbicer@rorqual.cc.metu.edu.tr	   *
 *	 D.Baris ACAR {Powerman}	dbacar@rorqual.cc.metu.edu.tr	   *
 *     By using this code, you have agreed to follow the terms of the	   *
 *     ANATOLIA license, in the file Anatolia/anatolia.licence		   *
 ***************************************************************************/

/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,	   *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *									   *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael	   *
 *  Chastain, Michael Quan, and Mitchell Tse.				   *
 *									   *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc	   *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.						   *
 *									   *
 *  Much time and thought has gone into this software and you are	   *
 *  benefitting.  We hope that you share your changes too.  What goes	   *
 *  around, comes around.						   *
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

#define args(a) a
#define DECLARE_DO_FUN(fun)	DO_FUN	  fun
#define DECLARE_SPEC_FUN(fun) 	SPEC_FUN  fun
#define DECLARE_SPELL_FUN(fun)	SPELL_FUN fun

/*
 * Short scalar types.
 * Diavolo reports AIX compiler has bugs with short types.
 */
#if	!defined(FALSE)
#define FALSE	 0
#endif

#if	!defined(TRUE)
#define TRUE	 1
#endif

typedef int bool;

/*
 * Structure types.
 */
typedef struct	affect_data		AFFECT_DATA;
typedef struct	area_data		AREA_DATA;
typedef struct	ban_data		BAN_DATA;
typedef struct	buf_data		BUFFER;
typedef struct	char_data		CHAR_DATA;
typedef struct	descriptor_data 	DESCRIPTOR_DATA;
typedef struct	exit_data		EXIT_DATA;
typedef struct	ed_data			ED_DATA;
typedef struct	help_data		HELP_DATA;
typedef struct	help_area_data		HELP_AREA;
typedef struct	kill_data		KILL_DATA;
typedef struct	mob_index_data		MOB_INDEX_DATA;
typedef struct	note_data		NOTE_DATA;
typedef struct	obj_data		OBJ_DATA;
typedef struct	obj_index_data		OBJ_INDEX_DATA;
typedef struct	pc_data 		PC_DATA;
typedef struct	reset_data		RESET_DATA;
typedef struct	room_index_data 	ROOM_INDEX_DATA;
typedef struct	shop_data		SHOP_DATA;
typedef struct	time_info_data		TIME_INFO_DATA;
typedef struct	weather_data		WEATHER_DATA;
typedef struct	room_history_data	ROOM_HISTORY_DATA;
typedef struct  mprog_list		MPROG_LIST;
typedef struct  mprog_code		MPROG_CODE;
typedef struct	qtrouble_data		QTROUBLE_DATA;
typedef struct	mlstring		mlstring;
typedef struct	hometown_data		HOMETOWN_DATA;

/*
 * Function types.
 */
typedef void DO_FUN	(CHAR_DATA *ch, const char *argument);
typedef bool SPEC_FUN	(CHAR_DATA *ch);
typedef void SPELL_FUN	(int sn, int level, CHAR_DATA *ch, void *vo,
				int target);

/*
 *  COMMAND extra bits..
 */
#define CMD_KEEP_HIDE		1
#define CMD_GHOST		2


/*
 * String and memory management parameters.
 */
#define MAX_KEY_HASH		1024
#define MAX_STRING_LENGTH	4608
#define MAX_PROMPT_LENGTH	60
#define MAX_TITLE_LENGTH	45
#define MAX_INPUT_LENGTH	256
#define PAGELEN 		22

/*
 * Game parameters.
 */
#define MAX_SOCIALS		256
#define MAX_SKILL		380
#define MAX_ALIAS		50
#define MAX_CLASS		13
#define MAX_PC_RACE		19	/* 18 */
#define MAX_CLAN		10
#define MAX_RELIGION		18
#define MAX_LEVEL		100
#define LEVEL_HERO		(MAX_LEVEL - 9)
#define LEVEL_IMMORTAL		(MAX_LEVEL - 8)


#undef ANATOLIA_MACHINE

#if defined(ANATOLIA_MACHINE)
#define PULSE_PER_SCD		6  /* 6 for comm.c */
#define PULSE_PER_SECOND	4  /* for update.c */
#define PULSE_VIOLENCE		(2 *  PULSE_PER_SECOND)

#else
#define PULSE_PER_SCD		4
#define PULSE_PER_SECOND	4
#define PULSE_VIOLENCE		(3 * PULSE_PER_SECOND)

#endif

#define PULSE_MOBILE		(4 * PULSE_PER_SECOND)
#define PULSE_WATER_FLOAT	(4 * PULSE_PER_SECOND)
#define PULSE_MUSIC		(6 * PULSE_PER_SECOND)
#define PULSE_TRACK		(6 * PULSE_PER_SECOND)
#define PULSE_TICK		(50 * PULSE_PER_SECOND) /* 36 saniye */

/* room_affect_update (not room_update) */
#define PULSE_RAFFECT		(3 * PULSE_MOBILE)
#define PULSE_AREA		(110 * PULSE_PER_SECOND) /* 97 saniye */
#define PULSE_AUCTION		(20 * PULSE_PER_SECOND) /* 20 seconds */

#define FIGHT_DELAY_TIME	(20 * PULSE_PER_SECOND)
#define GHOST_DELAY_TIME	600

#define IMPLEMENTOR		MAX_LEVEL
#define CREATOR 		(MAX_LEVEL - 1)
#define SUPREME 		(MAX_LEVEL - 2)
#define DEITY			(MAX_LEVEL - 3)
#define GOD			(MAX_LEVEL - 4)
#define IMMORTAL		(MAX_LEVEL - 5)
#define DEMI			(MAX_LEVEL - 6)
#define ANGEL			(MAX_LEVEL - 7)
#define AVATAR			(MAX_LEVEL - 8)
#define HERO			LEVEL_HERO


#define CLAN_NONE 	 0
#define CLAN_RULER	 1
#define CLAN_INVADER	 2
#define CLAN_CHAOS	 3
#define CLAN_SHALAFI	 4
#define CLAN_BATTLE	 5
#define CLAN_KNIGHT	 6
#define CLAN_LIONS	 7
#define CLAN_HUNTER	 8
#define CLAN_CONFEDERACY 9

/* Clan status */
#define CLAN_LEADER	2
#define CLAN_SECOND	1
#define CLAN_COMMON	0

/*
 * Clan structure
 */
struct clan_type
{
	char *		long_name;
	char *		short_name;
	int	 	obj_vnum;
	int	 	room_vnum;
	OBJ_DATA *	obj_ptr;
	int		string_prays;	/* string "prays for transportation" */
	int		string_vanishes;/* string "$n vanishes" */
};

struct color_type
{
	char *name;
	char *code;
};


/* To add god, make tattoo in limbo.are, add OBJ_VNUM_TATTOO_(GOD),
 * add here and add to const.c in the religion_table  also increase
 * MAX_RELIGION in merc.h  and make oprog for the tattoo
 */

#define RELIGION_NONE		0
#define RELIGION_APOLLON	1
#define RELIGION_ZEUS		2
#define RELIGION_SIEBELE	3
#define RELIGION_HEPHAESTUS	4
#define RELIGION_EHRUMEN	5
#define RELIGION_AHRUMAZDA	6
#define RELIGION_DEIMOS 	7
#define RELIGION_PHOBOS 	8
#define RELIGION_ODIN		9
#define RELIGION_MARS		10
#define RELIGION_ATHENA 	11
#define RELIGION_GOKTENGRI	12
#define RELIGION_HERA		13
#define RELIGION_VENUS		14
#define RELIGION_ARES		15
#define RELIGION_PROMETHEUS	16
#define RELIGION_EROS		17

/* Religion structure */
struct religion_type
{
	char *	leader;
	char *	name;
	int	vnum;
};


/*
 *  minimum pk level
 */
#define PK_MIN_LEVEL 5

#define MAX_NEWBIES 120   /* number of newbies allowed */
#define MAX_OLDIES 999	  /* number of newbies allowed */

/*
 * Site ban structure.
 */

#define BAN_SUFFIX		A
#define BAN_PREFIX		B
#define BAN_NEWBIES		C
#define BAN_ALL 		D
#define BAN_PERMIT		E
#define BAN_PERMANENT		F
#define BAN_PLAYER		G

struct ethos_type {
	char *name;
};


struct	ban_data
{
	BAN_DATA *	next;
	bool		valid;
	int		ban_flags;
	int		level;
	char *		name;
};



/*
 * Time and weather stuff.
 */
#define SUN_DARK		    0
#define SUN_RISE		    1
#define SUN_LIGHT		    2
#define SUN_SET 		    3

#define SKY_CLOUDLESS		    0
#define SKY_CLOUDY		    1
#define SKY_RAINING		    2
#define SKY_LIGHTNING		    3

struct	time_info_data
{
	int 	hour;
	int 	day;
	int 	month;
	int 	year;
};

struct	weather_data
{
	int 	mmhg;
	int 	change;
	int 	sky;
	int 	sunlight;
};



/*
 * Connected state for a channel.
 */
#define CON_PLAYING			 0
#define CON_GET_NAME			 1
#define CON_GET_OLD_PASSWORD		 2
#define CON_CONFIRM_NEW_NAME		 3
#define CON_GET_NEW_PASSWORD		 4
#define CON_CONFIRM_NEW_PASSWORD	 5
#define CON_GET_NEW_RACE		 6
#define CON_GET_NEW_SEX 		 7
#define CON_GET_NEW_CLASS		 8
#define CON_GET_ALIGNMENT		 9
#define CON_DEFAULT_CHOICE		10
#define CON_GEN_GROUPS			11
#define CON_PICK_WEAPON 		12
#define CON_READ_IMOTD			13
#define CON_READ_MOTD			14
#define CON_BREAK_CONNECT		15
#define CON_ROLL_STATS			16
#define CON_ACCEPT_STATS		17
#define CON_PICK_HOMETOWN		18
#define CON_GET_ETHOS			19
#define CON_CREATE_DONE 		20
#define CON_GET_CODEPAGE		21


/*
 * Descriptor (channel) structure.
 */
struct	descriptor_data
{
	DESCRIPTOR_DATA *	next;
	DESCRIPTOR_DATA *	snoop_by;
	CHAR_DATA * 		character;
	CHAR_DATA *	 	original;
	bool			valid;
	char *			host;
	int			descriptor;
	int			connected;
	int 			wait_for_se;
	bool			fcommand;
	char			inbuf		[4 * MAX_INPUT_LENGTH];
	char			incomm		[MAX_INPUT_LENGTH];
	char			inlast		[MAX_INPUT_LENGTH];
	int 			repeat;
	char *			outbuf;
	int 			outsize;
	int 			outtop;
	char *			showstr_head;
	char *			showstr_point;
	struct codepage*	codepage;
	void *             	pEdit;		/* OLC */
	char **			pString;	/* OLC */
	int			editor;		/* OLC */
};



/*
 * Attribute bonus structures.
 */
struct	str_app_type
{
	int	tohit;
	int	todam;
	int	carry;
	int	wield;
};

struct	int_app_type
{
	int	learn;
};

struct	wis_app_type
{
	int	practice;
};

struct	dex_app_type
{
	int	defensive;
};

struct	con_app_type
{
	int	hitp;
	int	shock;
};



/*
 * TO types for act.
 */
#define TO_ROOM 	    0
#define TO_NOTVICT	    1
#define TO_VICT 	    2
#define TO_CHAR 	    3
#define TO_ALL		    4



/*
 * Help table types.
 */
struct	help_data
{
	HELP_DATA *	next;
	HELP_DATA *	next_area;
	int		level;
	char * 		keyword;
	mlstring *	text;
};
	
struct help_area_data
{
 	HELP_AREA *	next;
 	HELP_DATA *	first;
 	HELP_DATA *	last;
 	AREA_DATA *	area;
 	char *		filename;
};
	

/*
 * Shop types.
 */
#define MAX_TRADE	 5

struct	shop_data
{
	SHOP_DATA * next;			/* Next shop in list		*/
	int	keeper; 		/* Vnum of shop keeper mob	*/
	int	buy_type [MAX_TRADE];	/* Item types shop will buy	*/
	int	profit_buy;		/* Cost multiplier for buying	*/
	int	profit_sell;		/* Cost multiplier for selling	*/
	int	open_hour;		/* First opening hour		*/
	int	close_hour;		/* First closing hour		*/
};



/*
 * Per-class stuff.
 */

#define MAX_GUILD	6
#define MAX_STATS	6
#define STAT_STR	0
#define STAT_INT	1
#define STAT_WIS	2
#define STAT_DEX	3
#define STAT_CON	4
#define STAT_CHA	5

struct	class_type
{
	char *	name;			/* the full name of the class */
	char	who_name	[4];	/* Three-letter name for 'who'	*/
	int	attr_prime;		/* Prime attribute		*/
	int	weapon; 		/* First weapon 		*/
	int	guild[MAX_GUILD];	/* Vnum of guild rooms		*/
	int	skill_adept;		/* Maximum skill level		*/
	int	thac0_00;		/* Thac0 for level  0		*/
	int	thac0_32;		/* Thac0 for level 32		*/
	int	hp_rate;		/* hp rate gained on leveling	*/
	int	mana_rate;		/* mana rate gained on leveling */
	bool	fMana;			/* Class gains mana on level	*/
	int 	points; 		/* Cost in exp of class 	*/
	int	stats[MAX_STATS];	/* Stat modifiers		*/
	int	align;			/* Alignment			*/
};


enum class_names {
	CLASS_WITCH,
	CLASS_CLERIC,
	CLASS_THIEF,
	CLASS_WARRIOR,
	CLASS_PALADIN,
	CLASS_ANTI_PALADIN,
	CLASS_NINJA,
	CLASS_RANGER,
	CLASS_WARLOCK,
	CLASS_SAMURAI,
	CLASS_VAMPIRE,
	CLASS_NECROMANCER,
	CLASS_DUMMY,
	CLASS_NONE = -1
};

#define LANG_COMMON	     0
#define LANG_HUMAN	     1
#define LANG_ELVISH	     2
#define LANG_DWARVISH	     3
#define LANG_GNOMISH	     4
#define LANG_GIANT	     5
#define LANG_TROLLISH	     6
#define LANG_CAT	     7
#define SLANG_MAX	     8

struct item_type
{
	int 	type;
	char *	name;
};

struct weapon_type
{
	char *	name;
	int	vnum;
	int	type;
	int	*gsn;
};

struct wiznet_type
{
	char *	name;
	int	flag;
	int 	level;
};

struct attack_type
{
	char *	name;			/* name */
	char *	noun;			/* message */
	int 	damage; 		/* damage class */
};

struct race_type
{
	char *	name;			/* call name of the race */
	bool	pc_race;		/* can be chosen by pcs  */
	int	det;			/* det bits for the race */
	int	act;			/* act bits for the race */
	int	aff;			/* aff bits for the race */
	int	off;			/* off bits for the race */
	int	imm;			/* imm bits for the race */
	int	res;			/* res bits for the race */
	int	vuln;			/* vuln bits for the race */
	int	form;			/* default form flag for the race */
	int	parts;			/* default parts for the race */
};


struct pc_race_type  /* additional data for pc races */
{
	char *	name;			/* MUST be in race_type */
	char	who_name[6];
	int 	points; 		/* cost in exp of the race */
	int	class_mult[MAX_CLASS];	/* exp multiplier for class, * 100 */
	char *	skills[5];		/* bonus skills for the race */
	int	stats[MAX_STATS];	/* starting stats	*/
	int	max_stats[MAX_STATS];	/* maximum stats	*/
	int	size;			/* aff bits for the race*/
	int 	hp_bonus;		/* Initial hp bonus	*/
	int 	mana_bonus;		/* Initial mana bonus	*/
	int 	prac_bonus;		/* Initial practice bonus */
	int	align;			/* Alignment		*/
	int	slang;			/* spoken language	*/
};

struct spec_type
{
	char *		name;			/* special function name */
	SPEC_FUN *	function;		/* the function */
};


/*
 * Data structure for notes.
 */

#define NOTE_NOTE	0
#define NOTE_IDEA	1
#define NOTE_PENALTY	2
#define NOTE_NEWS	3
#define NOTE_CHANGES	4
struct	note_data
{
	NOTE_DATA *	next;
	bool		valid;
	int		type;
	char *		sender;
	char *		date;
	char *		to_list;
	char *		subject;
	char *		text;
	time_t		date_stamp;
};



/*
 * An affect.
 */
struct	affect_data
{
	AFFECT_DATA *	next;
	bool		valid;
	int		where;
	int		type;
	int		level;
	int		duration;
	int		location;
	int		modifier;
	int 		bitvector;
};

/* where definitions */
#define TO_AFFECTS	0
#define TO_OBJECT	1
#define TO_IMMUNE	2
#define TO_RESIST	3
#define TO_VULN 	4
#define TO_WEAPON	5
#define TO_ACT_FLAG	6
#define TO_DETECTS	7
#define TO_RACE 	8

/* where definitions for room */
#define TO_ROOM_AFFECTS 0
#define TO_ROOM_CONST	1
#define TO_ROOM_FLAGS	2

/* room applies */
#define APPLY_ROOM_NONE 	0
#define APPLY_ROOM_HEAL 	1
#define APPLY_ROOM_MANA 	2

/*
 * A kill structure (indexed by level).
 */
struct	kill_data
{
	int		number;
	int		killed;
};



/***************************************************************************
 *									   *
 *		     VALUES OF INTEREST TO AREA BUILDERS		   *
 *		     (Start of section ... start here)			   *
 *									   *
 ***************************************************************************/

/*
 * Well known mob virtual numbers.
 * Defined in #MOBILES.
 */
#define MOB_VNUM_CITYGUARD	   3060
#define MOB_VNUM_VAGABOND	   3063
#define MOB_VNUM_CAT		   3066
#define MOB_VNUM_FIDO		   3062
#define MOB_VNUM_SAGE		   3162

#define MOB_VNUM_VAMPIRE	    3404

#define MOB_VNUM_SHADOW 	      10
#define MOB_VNUM_SPECIAL_GUARD	      11
#define MOB_VNUM_BEAR		      12
#define MOB_VNUM_DEMON		      13
#define MOB_VNUM_NIGHTWALKER	      14
#define MOB_VNUM_STALKER	      15
#define MOB_VNUM_SQUIRE 	      16
#define MOB_VNUM_MIRROR_IMAGE	      17
#define MOB_VNUM_UNDEAD 	      18
#define MOB_VNUM_LION		      19
#define MOB_VNUM_WOLF		      20
#define MOB_VNUM_SUM_SHADOW	      26

#define MOB_VNUM_LESSER_GOLEM		21
#define MOB_VNUM_STONE_GOLEM		22
#define MOB_VNUM_IRON_GOLEM		23
#define MOB_VNUM_ADAMANTITE_GOLEM	24

#define MOB_VNUM_HUNTER 		25

#define MOB_VNUM_PATROLMAN	   2106
#define GROUP_VNUM_TROLLS	   2100
#define GROUP_VNUM_OGRES	   2101


/* RT ASCII conversions -- used so we can have letters in this file */

#define A			1
#define B			2
#define C			4
#define D			8
#define E			16
#define F			32
#define G			64
#define H			128

#define I			256
#define J			512
#define K			1024
#define L			2048
#define M			4096
#define N			8192
#define O			16384
#define P			32768

#define Q			65536
#define R			131072
#define S			262144
#define T			524288
#define U			1048576
#define V			2097152
#define W			4194304
#define X			8388608

#define Y			16777216
#define Z			33554432
#define aa			67108864	/* doubled due to conflicts */
#define bb			134217728
#define cc			268435456
#define dd			536870912
#define ee			1073741824


/* race table */
#define RACE_NONE		(ee - 1)
#define RACE_HUMAN		(A)
#define RACE_ELF		(B)
#define RACE_HALF_ELF		(C)
#define RACE_DARK_ELF		(D)
#define RACE_ROCKSEER		(E)	/* 5 */
#define RACE_DWARF		(F)
#define RACE_SVIRFNEBLI 	(G)
#define RACE_DUERGAR		(H)
#define RACE_ARIAL		(I)
#define RACE_GNOME		(J)	/* 10 */
#define RACE_STORM_GIANT	(K)
#define RACE_CLOUD_GIANT	(L)
#define RACE_FIRE_GIANT 	(M)
#define RACE_FROST_GIANT	(N)
#define RACE_FELAR		(O)	/* 15 */
#define RACE_GITHYANKI		(P)
#define RACE_SATYR		(Q)
#define RACE_TROLL		(R)

#define RACE_TOP		(T)

/* general align */
#define ALIGN_NONE		-1
#define ALIGN_GOOD		1000
#define ALIGN_NEUTRAL		0
#define ALIGN_EVIL		-1000

/* binary align for race and class */
#define CR_ALL			(0)
#define CR_GOOD 		(A)
#define CR_NEUTRAL		(B)
#define CR_EVIL 		(C)

/* number align */
#define N_ALIGN_ALL		(0)
#define N_ALIGN_GOOD		(1)
#define N_ALIGN_NEUTRAL 	(2)
#define N_ALIGN_EVIL		(3)



/* skills group numbers*/
#define GROUP_NONE		0
#define GROUP_WEAPONSMASTER	(A)
#define GROUP_ATTACK		(B)
#define GROUP_BEGUILING 	(C)
#define GROUP_BENEDICTIONS	(D)
#define GROUP_COMBAT		(E)
#define GROUP_CREATION		(F)
#define GROUP_CURATIVE		(G)
#define GROUP_DETECTION 	(H)
#define GROUP_DRACONIAN 	(I)
#define GROUP_ENCHANTMENT	(J)
#define GROUP_ENHANCEMENT	(K)
#define GROUP_HARMFUL		(L)
#define GROUP_HEALING		(M)
#define GROUP_ILLUSION		(N)
#define GROUP_MALADICTIONS	(O)
#define GROUP_PROTECTIVE	(P)
#define GROUP_TRANSPORTATION	(Q)
#define GROUP_WEATHER		(R)
#define GROUP_FIGHTMASTER	(S)
#define GROUP_SUDDENDEATH	(T)
#define GROUP_MEDITATION	(U)
#define GROUP_CLAN		(V)
#define GROUP_DEFENSIVE 	(W)
#define GROUP_WIZARD		(X)

/*
 * AREA FLAGS
 */
#define AREA_HOMETOWN		(A)


/*
 * ACT bits for mobs.  *ACT*
 * Used in #MOBILES.
 */
#define ACT_NPC			(A)		/* Auto set for mobs	*/
#define ACT_SENTINEL		(B)		/* Stays in one room	*/
#define ACT_SCAVENGER		(C)		/* Picks up objects	*/
#define ACT_AGGRESSIVE		(F)		/* Attacks PC's 	*/
#define ACT_STAY_AREA		(G)		/* Won't leave area	*/
#define ACT_WIMPY		(H)
#define ACT_PET 		(I)		/* Auto set for pets	*/
#define ACT_TRAIN		(J)		/* Can train PC's	*/
#define ACT_PRACTICE		(K)		/* Can practice PC's	*/
#define ACT_HUNTER		(L)
#define ACT_UNDEAD		(O)
#define ACT_CLERIC		(Q)
#define ACT_MAGE		(R)
#define ACT_THIEF		(S)
#define ACT_WARRIOR		(T)
#define ACT_NOALIGN		(U)
#define ACT_NOPURGE		(V)
#define ACT_OUTDOORS		(W)
#define ACT_QUESTOR		(X)
#define ACT_INDOORS		(Y)
#define ACT_RIDEABLE		(Z)
#define ACT_HEALER		(aa)
#define ACT_GAIN		(bb)
#define ACT_UPDATE_ALWAYS	(cc)
#define ACT_CHANGER		(dd)
#define ACT_NOTRACK		(ee)

/* damage classes */
#define DAM_NONE		0
#define DAM_BASH		1
#define DAM_PIERCE		2
#define DAM_SLASH		3
#define DAM_FIRE		4
#define DAM_COLD		5
#define DAM_LIGHTNING		6
#define DAM_ACID		7
#define DAM_POISON		8
#define DAM_NEGATIVE		9
#define DAM_HOLY		10
#define DAM_ENERGY		11
#define DAM_MENTAL		12
#define DAM_DISEASE		13
#define DAM_DROWNING		14
#define DAM_LIGHT		15
#define DAM_OTHER		16
#define DAM_HARM		17
#define DAM_CHARM		18
#define DAM_SOUND		19
#define DAM_THIRST		20
#define DAM_HUNGER		21
#define DAM_LIGHT_V		22
#define DAM_TRAP_ROOM		23

/* OFF bits for mobiles *OFF  */
#define OFF_AREA_ATTACK 	(A)
#define OFF_BACKSTAB		(B)
#define OFF_BASH		(C)
#define OFF_BERSERK		(D)
#define OFF_DISARM		(E)
#define OFF_DODGE		(F)
#define OFF_FADE		(G)
#define OFF_FAST		(H)
#define OFF_KICK		(I)
#define OFF_DIRT_KICK		(J)
#define OFF_PARRY		(K)
#define OFF_RESCUE		(L)
#define OFF_TAIL		(M)
#define OFF_TRIP		(N)
#define OFF_CRUSH		(O)
#define ASSIST_ALL		(P)
#define ASSIST_ALIGN		(Q)
#define ASSIST_RACE		(R)
#define ASSIST_PLAYERS		(S)
#define ASSIST_GUARD		(T)
#define ASSIST_VNUM		(U)

/* return values for check_imm */
#define IS_NORMAL		0
#define IS_IMMUNE		1
#define IS_RESISTANT		2
#define IS_VULNERABLE		3

/* IMM bits for mobs */
#define IMM_SUMMON		(A)
#define IMM_CHARM		(B)
#define IMM_MAGIC		(C)
#define IMM_WEAPON		(D)
#define IMM_BASH		(E)
#define IMM_PIERCE		(F)
#define IMM_SLASH		(G)
#define IMM_FIRE		(H)
#define IMM_COLD		(I)
#define IMM_LIGHTNING		(J)
#define IMM_ACID		(K)
#define IMM_POISON		(L)
#define IMM_NEGATIVE		(M)
#define IMM_HOLY		(N)
#define IMM_ENERGY		(O)
#define IMM_MENTAL		(P)
#define IMM_DISEASE		(Q)
#define IMM_DROWNING		(R)
#define IMM_LIGHT		(S)
#define IMM_SOUND		(T)
#define IMM_STEAL		(U)
#define IMM_WOOD		(X)
#define IMM_SILVER		(Y)
#define IMM_IRON		(Z)

/* RES bits for mobs *RES */
#define RES_SUMMON		(A)
#define RES_CHARM		(B)
#define RES_MAGIC		(C)
#define RES_WEAPON		(D)
#define RES_BASH		(E)
#define RES_PIERCE		(F)
#define RES_SLASH		(G)
#define RES_FIRE		(H)
#define RES_COLD		(I)
#define RES_LIGHTNING		(J)
#define RES_ACID		(K)
#define RES_POISON		(L)
#define RES_NEGATIVE		(M)
#define RES_HOLY		(N)
#define RES_ENERGY		(O)
#define RES_MENTAL		(P)
#define RES_DISEASE		(Q)
#define RES_DROWNING		(R)
#define RES_LIGHT		(S)
#define RES_SOUND		(T)
#define RES_WOOD		(X)
#define RES_SILVER		(Y)
#define RES_IRON		(Z)

/* VULN bits for mobs */
#define VULN_SUMMON		(A)
#define VULN_CHARM		(B)
#define VULN_MAGIC		(C)
#define VULN_WEAPON		(D)
#define VULN_BASH		(E)
#define VULN_PIERCE		(F)
#define VULN_SLASH		(G)
#define VULN_FIRE		(H)
#define VULN_COLD		(I)
#define VULN_LIGHTNING		(J)
#define VULN_ACID		(K)
#define VULN_POISON		(L)
#define VULN_NEGATIVE		(M)
#define VULN_HOLY		(N)
#define VULN_ENERGY		(O)
#define VULN_MENTAL		(P)
#define VULN_DISEASE		(Q)
#define VULN_DROWNING		(R)
#define VULN_LIGHT		(S)
#define VULN_SOUND		(T)
#define VULN_WOOD		(X)
#define VULN_SILVER		(Y)
#define VULN_IRON		(Z)

/* body form */
#define FORM_EDIBLE		(A)
#define FORM_POISON		(B)
#define FORM_MAGICAL		(C)
#define FORM_INSTANT_DECAY	(D)
#define FORM_OTHER		(E)  /* defined by material bit */

/* actual form */
#define FORM_ANIMAL		(G)
#define FORM_SENTIENT		(H)
#define FORM_UNDEAD		(I)
#define FORM_CONSTRUCT		(J)
#define FORM_MIST		(K)
#define FORM_INTANGIBLE 	(L)

#define FORM_BIPED		(M)
#define FORM_CENTAUR		(N)
#define FORM_INSECT		(O)
#define FORM_SPIDER		(P)
#define FORM_CRUSTACEAN 	(Q)
#define FORM_WORM		(R)
#define FORM_BLOB		(S)

#define FORM_MAMMAL		(V)
#define FORM_BIRD		(W)
#define FORM_REPTILE		(X)
#define FORM_SNAKE		(Y)
#define FORM_DRAGON		(Z)
#define FORM_AMPHIBIAN		(aa)
#define FORM_FISH		(bb)
#define FORM_COLD_BLOOD 	(cc)

/* body parts */
#define PART_HEAD		(A)
#define PART_ARMS		(B)
#define PART_LEGS		(C)
#define PART_HEART		(D)
#define PART_BRAINS		(E)
#define PART_GUTS		(F)
#define PART_HANDS		(G)
#define PART_FEET		(H)
#define PART_FINGERS		(I)
#define PART_EAR		(J)
#define PART_EYE		(K)
#define PART_LONG_TONGUE	(L)
#define PART_EYESTALKS		(M)
#define PART_TENTACLES		(N)
#define PART_FINS		(O)
#define PART_WINGS		(P)
#define PART_TAIL		(Q)
/* for combat */
#define PART_CLAWS		(U)
#define PART_FANGS		(V)
#define PART_HORNS		(W)
#define PART_SCALES		(X)
#define PART_TUSKS		(Y)


/*
 * Bits for 'affected_by'.  *AFF*
 * Used in #MOBILES.
 */
#define AFF_BLIND		(A)
#define AFF_INVISIBLE		(B)
#define AFF_IMP_INVIS		(C)
#define AFF_FADE		(D)
#define AFF_SCREAM		(E)
#define AFF_BLOODTHIRST 	(F)
#define AFF_STUN		(G)
#define AFF_SANCTUARY		(H)
#define AFF_FAERIE_FIRE 	(I)
#define AFF_INFRARED		(J)
#define AFF_CURSE		(K)
#define AFF_CORRUPTION		(L)
#define AFF_POISON		(M)
#define AFF_PROTECT_EVIL	(N)
#define AFF_PROTECT_GOOD	(O)
#define AFF_SNEAK		(P)
#define AFF_HIDE		(Q)
#define AFF_SLEEP		(R)
#define AFF_CHARM		(S)
#define AFF_FLYING		(T)
#define AFF_PASS_DOOR		(U)
#define AFF_HASTE		(V)
#define AFF_CALM		(W)
#define AFF_PLAGUE		(X)
#define AFF_WEAKEN		(Y)
#define AFF_WEAK_STUN		(Z)
#define AFF_BERSERK		(aa)
#define AFF_SWIM		(bb)
#define AFF_REGENERATION	(cc)
#define AFF_SLOW		(dd)
#define AFF_CAMOUFLAGE		(ee)


/*
 * *AFF* bits for rooms
 */
#define RAFF_SHOCKING		(A)
#define RAFF_LSHIELD		(B)
#define RAFF_THIEF_TRAP		(C)
#define RAFF_RANDOMIZER		(D)
#define RAFF_ESPIRIT		(E)
#define RAFF_CURSE		(K)
#define RAFF_POISON 		(M)
#define RAFF_SLEEP		(R)
#define RAFF_PLAGUE 		(X)
#define RAFF_SLOW		(dd)

/*
 * Bits for 'detection'.  *DETECT*
 * Used in chars.
 */
#define DETECT_IMP_INVIS	(A)
#define DETECT_FADE		(B)
#define DETECT_EVIL		(C)
#define DETECT_INVIS		(D)
#define DETECT_MAGIC		(E)
#define DETECT_HIDDEN		(F)
#define DETECT_GOOD		(G)
#define DETECT_SNEAK		(H)
#define DETECT_UNDEAD		(I)

/*
 * Lack of bits in "affected_by"
 * thus send them to "detection" with "TO_DETECT"
 * name with "ADET_*"
 */
#define ADET_FEAR		(Q)
#define ADET_FORM_TREE		(R)
#define ADET_FORM_GRASS 	(S)
#define ADET_WEB		(T)

#define DETECT_LIFE		(Y)
#define DARK_VISION		(Z)
#define ACUTE_VISION		(ee)

/*
 * Sex.
 * Used in #MOBILES.
 */
enum {
	SEX_NEUTRAL,
	SEX_MALE,
	SEX_FEMALE,
	SEX_EITHER,		/* used only for NPC, means random sex
				   in create_mobile */
	SEX_MAX = SEX_EITHER
};

/* AC types */
#define AC_PIERCE			0
#define AC_BASH 			1
#define AC_SLASH			2
#define AC_EXOTIC			3

/* dice */
#define DICE_NUMBER			0
#define DICE_TYPE			1
#define DICE_BONUS			2

/* size */
#define SIZE_TINY			0
#define SIZE_SMALL			1
#define SIZE_MEDIUM			2
#define SIZE_LARGE			3
#define SIZE_HUGE			4
#define SIZE_GIANT			5
#define SIZE_GARGANTUAN 		6



/*
 * Well known object virtual numbers.
 * Defined in #OBJECTS.
 */
#define OBJ_VNUM_SILVER_ONE	      1
#define OBJ_VNUM_GOLD_ONE	      2
#define OBJ_VNUM_GOLD_SOME	      3
#define OBJ_VNUM_SILVER_SOME	      4
#define OBJ_VNUM_COINS		      5

#define OBJ_VNUM_CORPSE_NPC	     10
#define OBJ_VNUM_CORPSE_PC	     11
#define OBJ_VNUM_SEVERED_HEAD	     12
#define OBJ_VNUM_TORN_HEART	     13
#define OBJ_VNUM_SLICED_ARM	     14
#define OBJ_VNUM_SLICED_LEG	     15
#define OBJ_VNUM_GUTS		     16
#define OBJ_VNUM_BRAINS 	     17

#define OBJ_VNUM_MUSHROOM	     20
#define OBJ_VNUM_LIGHT_BALL	     21
#define OBJ_VNUM_SPRING 	     22
#define OBJ_VNUM_DISC		     23
#define OBJ_VNUM_PORTAL 	     25
#define OBJ_VNUM_ROSE		     1001
#define OBJ_VNUM_PIT		     3010

#define OBJ_VNUM_SCHOOL_MACE	   3700
#define OBJ_VNUM_SCHOOL_DAGGER	   3701
#define OBJ_VNUM_SCHOOL_SWORD	   3702
#define OBJ_VNUM_SCHOOL_SPEAR	   3717
#define OBJ_VNUM_SCHOOL_STAFF	   3718
#define OBJ_VNUM_SCHOOL_AXE	   3719
#define OBJ_VNUM_SCHOOL_FLAIL	   3720
#define OBJ_VNUM_SCHOOL_WHIP	   3721
#define OBJ_VNUM_SCHOOL_POLEARM    3722
#define OBJ_VNUM_SCHOOL_BOW	   3723
#define OBJ_VNUM_SCHOOL_LANCE	   3724

#define OBJ_VNUM_SCHOOL_VEST	   3703
#define OBJ_VNUM_SCHOOL_SHIELD	   3704
#define OBJ_VNUM_SCHOOL_BANNER	   3716

#define OBJ_VNUM_MAP		   3162
#define OBJ_VNUM_NMAP1		   3385
#define OBJ_VNUM_NMAP2		   3386
#define OBJ_VNUM_MAP_NT 	   3167
#define OBJ_VNUM_MAP_OFCOL	   3162
#define OBJ_VNUM_MAP_SM 	   3164
#define OBJ_VNUM_MAP_TITAN	   3382
#define OBJ_VNUM_MAP_OLD	   5333

#define OBJ_VNUM_WHISTLE	   2116

#define OBJ_VNUM_POTION_VIAL	     42
#define OBJ_VNUM_STEAK		     27
#define OBJ_VNUM_RANGER_STAFF	     28
#define OBJ_VNUM_RANGER_ARROW	     6
#define OBJ_VNUM_RANGER_BOW	     7

#define OBJ_VNUM_DEPUTY_BADGE	   70
#define OBJ_VNUM_RULER_BADGE	   70
#define OBJ_VNUM_RULER_SHIELD1	   71
#define OBJ_VNUM_RULER_SHIELD2	   72
#define OBJ_VNUM_RULER_SHIELD3	   73
#define OBJ_VNUM_RULER_SHIELD4	   74

#define OBJ_VNUM_CHAOS_BLADE	    87

#define OBJ_VNUM_DRAGONDAGGER	    80
#define OBJ_VNUM_DRAGONMACE	    81
#define OBJ_VNUM_PLATE		    82
#define OBJ_VNUM_DRAGONSWORD	    83
#define OBJ_VNUM_DRAGONLANCE	    99

#define OBJ_VNUM_BATTLE_PONCHO	     26

#define OBJ_VNUM_BATTLE_THRONE	     542
#define OBJ_VNUM_SHALAFI_ALTAR	     530
#define OBJ_VNUM_CHAOS_ALTAR	     551
#define OBJ_VNUM_INVADER_SKULL	     560
#define OBJ_VNUM_KNIGHT_ALTAR	     521
#define OBJ_VNUM_RULER_STAND	     510
#define OBJ_VNUM_LIONS_ALTAR	     501
#define OBJ_VNUM_HUNTER_ALTAR	     570

#define OBJ_VNUM_POTION_SILVER	      43
#define OBJ_VNUM_POTION_GOLDEN	      44
#define OBJ_VNUM_POTION_SWIRLING      45
#define OBJ_VNUM_KATANA_SWORD	      98

#define OBJ_VNUM_EYED_SWORD		88
#define OBJ_VNUM_FIRE_SHIELD		92
#define OBJ_VNUM_MAGIC_JAR		93
#define OBJ_VNUM_HAMMER 		6522

#define OBJ_VNUM_CHUNK_IRON		6521

/* vnums for tattoos */
#define OBJ_VNUM_TATTOO_APOLLON 	51
#define OBJ_VNUM_TATTOO_ZEUS		52
#define OBJ_VNUM_TATTOO_SIEBELE 	53
#define OBJ_VNUM_TATTOO_HEPHAESTUS	54
#define OBJ_VNUM_TATTOO_EHRUMEN 	55
#define OBJ_VNUM_TATTOO_AHRUMAZDA	56
#define OBJ_VNUM_TATTOO_DEIMOS		57
#define OBJ_VNUM_TATTOO_PHOBOS		58
#define OBJ_VNUM_TATTOO_ODIN		59
#define OBJ_VNUM_TATTOO_MARS		60
#define OBJ_VNUM_TATTOO_ATHENA		61
#define OBJ_VNUM_TATTOO_GOKTENGRI	62
#define OBJ_VNUM_TATTOO_HERA		63
#define OBJ_VNUM_TATTOO_VENUS		64
#define OBJ_VNUM_TATTOO_ARES		65
#define OBJ_VNUM_TATTOO_PROMETHEUS	66
#define OBJ_VNUM_TATTOO_EROS		67

/*
 * Item types.
 * Used in #OBJECTS.
 */
#define ITEM_LIGHT		      1
#define ITEM_SCROLL		      2
#define ITEM_WAND		      3
#define ITEM_STAFF		      4
#define ITEM_WEAPON		      5
#define ITEM_TREASURE		      8
#define ITEM_ARMOR		      9
#define ITEM_POTION		     10
#define ITEM_CLOTHING		     11
#define ITEM_FURNITURE		     12
#define ITEM_TRASH		     13
#define ITEM_CONTAINER		     15
#define ITEM_DRINK_CON		     17
#define ITEM_KEY		     18
#define ITEM_FOOD		     19
#define ITEM_MONEY		     20
#define ITEM_BOAT		     22
#define ITEM_CORPSE_NPC 	     23
#define ITEM_CORPSE_PC		     24
#define ITEM_FOUNTAIN		     25
#define ITEM_PILL		     26
#define ITEM_PROTECT		     27
#define ITEM_MAP		     28
#define ITEM_PORTAL		     29
#define ITEM_WARP_STONE 	     30
#define ITEM_ROOM_KEY		     31
#define ITEM_GEM		     32
#define ITEM_JEWELRY		     33
#define ITEM_JUKEBOX		     34
#define ITEM_TATTOO		     35
 

/*
 * Extra flags.  *EXT*
 * Used in #OBJECTS.
 */
#define ITEM_GLOW		(A)
#define ITEM_HUM		(B)
#define ITEM_DARK		(C)
#define ITEM_LOCK		(D)
#define ITEM_EVIL		(E)
#define ITEM_INVIS		(F)
#define ITEM_MAGIC		(G)
#define ITEM_NODROP		(H)
#define ITEM_BLESS		(I)
#define ITEM_ANTI_GOOD		(J)
#define ITEM_ANTI_EVIL		(K)
#define ITEM_ANTI_NEUTRAL	(L)
#define ITEM_NOREMOVE		(M)
#define ITEM_INVENTORY		(N)
#define ITEM_NOPURGE		(O)
#define ITEM_ROT_DEATH		(P)
#define ITEM_VIS_DEATH		(Q)
#define ITEM_NOSAC		(R)
#define ITEM_NONMETAL		(S)
#define ITEM_NOLOCATE		(T)
#define ITEM_MELT_DROP		(U)
#define ITEM_HAD_TIMER		(V)
#define ITEM_SELL_EXTRACT	(W)
#define ITEM_BURN_PROOF 	(Y)
#define ITEM_NOUNCURSE		(Z)
#define ITEM_NOSELL		(aa)

/*
 * Wear flags.	 *WEAR*
 * Used in #OBJECTS.
 */
#define ITEM_TAKE		(A)
#define ITEM_WEAR_FINGER	(B)
#define ITEM_WEAR_NECK		(C)
#define ITEM_WEAR_BODY		(D)
#define ITEM_WEAR_HEAD		(E)
#define ITEM_WEAR_LEGS		(F)
#define ITEM_WEAR_FEET		(G)
#define ITEM_WEAR_HANDS 	(H)
#define ITEM_WEAR_ARMS		(I)
#define ITEM_WEAR_SHIELD	(J)
#define ITEM_WEAR_ABOUT 	(K)
#define ITEM_WEAR_WAIST 	(L)
#define ITEM_WEAR_WRIST 	(M)
#define ITEM_WIELD		(N)
#define ITEM_HOLD		(O)
#define ITEM_NO_SAC		(P)
#define ITEM_WEAR_FLOAT 	(Q)
#define ITEM_WEAR_TATTOO	(R)

/* weapon class */
#define WEAPON_EXOTIC		0
#define WEAPON_SWORD		1
#define WEAPON_DAGGER		2
#define WEAPON_SPEAR		3
#define WEAPON_MACE		4
#define WEAPON_AXE		5
#define WEAPON_FLAIL		6
#define WEAPON_WHIP		7
#define WEAPON_POLEARM		8
#define WEAPON_BOW		9
#define WEAPON_ARROW		10
#define WEAPON_LANCE		11

/* weapon types */
#define WEAPON_FLAMING		(A)
#define WEAPON_FROST		(B)
#define WEAPON_VAMPIRIC 	(C)
#define WEAPON_SHARP		(D)
#define WEAPON_VORPAL		(E)
#define WEAPON_TWO_HANDS	(F)
#define WEAPON_SHOCKING 	(G)
#define WEAPON_POISON		(H)
#define WEAPON_HOLY		(I)
#define WEAPON_KATANA		(J)

/* gate flags */
#define GATE_NORMAL_EXIT	(A)
#define GATE_NOCURSE		(B)
#define GATE_GOWITH		(C)
#define GATE_BUGGY		(D)
#define GATE_RANDOM		(E)

/* furniture flags */
#define STAND_AT		(A)
#define STAND_ON		(B)
#define STAND_IN		(C)
#define SIT_AT			(D)
#define SIT_ON			(E)
#define SIT_IN			(F)
#define REST_AT 		(G)
#define REST_ON 		(H)
#define REST_IN 		(I)
#define SLEEP_AT		(J)
#define SLEEP_ON		(K)
#define SLEEP_IN		(L)
#define PUT_AT			(M)
#define PUT_ON			(N)
#define PUT_IN			(O)
#define PUT_INSIDE		(P)




/*
 * Apply types (for affects).
 * Used in #OBJECTS.
 */
#define APPLY_NONE		0
#define APPLY_STR		1
#define APPLY_DEX		2
#define APPLY_INT		3
#define APPLY_WIS		4
#define APPLY_CON		5
#define APPLY_CHA		6
#define APPLY_SEX		7
#define APPLY_LEVEL		8
#define APPLY_AGE		9
#define APPLY_HEIGHT		10
#define APPLY_WEIGHT		11
#define APPLY_MANA		12
#define APPLY_HIT		13
#define APPLY_MOVE		14
#define APPLY_GOLD		15
#define APPLY_EXP		16
#define APPLY_AC		17
#define APPLY_HITROLL		18
#define APPLY_DAMROLL		19
#define APPLY_SAVES		20
#define APPLY_SAVING_PARA	20
#define APPLY_SAVING_ROD	21
#define APPLY_SAVING_PETRI	22
#define APPLY_SAVING_BREATH	23
#define APPLY_SAVING_SPELL	24
#define APPLY_SPELL_AFFECT	25
#define APPLY_SIZE		26
#define APPLY_CLASS		27

/*
 * Values for containers (value[1]).
 * Used in #OBJECTS.
 */
#define CONT_CLOSEABLE		      1
#define CONT_PICKPROOF		      2
#define CONT_CLOSED		      4
#define CONT_LOCKED		      8
#define CONT_PUT_ON		     16
#define CONT_FOR_ARROW		     32


/*
 * Well known room virtual numbers.
 * Defined in #ROOMS.
 */
#define ROOM_VNUM_LIMBO 	2
#define ROOM_VNUM_CHAT		1200
#define ROOM_VNUM_TEMPLE	3001
#define ROOM_VNUM_ALTAR 	3054
#define ROOM_VNUM_SCHOOL	3700
#define ROOM_VNUM_BALANCE	4500
#define ROOM_VNUM_CIRCLE	4400
#define ROOM_VNUM_DEMISE	4201
#define ROOM_VNUM_HONOR 	4300
#define ROOM_VNUM_BATTLE	541



/*
 * Room flags.
 * Used in #ROOMS.
 */
#define ROOM_DARK		(A)
#define ROOM_NOMOB		(C)
#define ROOM_INDOORS		(D)
#define ROOM_PRIVATE		(J)
#define ROOM_SAFE		(K)
#define ROOM_SOLITARY		(L)
#define ROOM_PET_SHOP		(M)
#define ROOM_NO_RECALL		(N)
#define ROOM_IMP_ONLY		(O)
#define ROOM_GODS_ONLY		(P)
#define ROOM_HEROES_ONLY	(Q)
#define ROOM_NEWBIES_ONLY	(R)
#define ROOM_LAW		(S)
#define ROOM_NOWHERE		(T)
#define ROOM_BANK		(U)
#define ROOM_NO_MAGIC		(W)
#define ROOM_NOSUMMON		(X)
#define ROOM_BATTLE_ARENA	(Z)
#define ROOM_REGISTRY		(bb)



/*
 * Directions.
 * Used in #ROOMS.
 */
#define DIR_NORTH		      0
#define DIR_EAST		      1
#define DIR_SOUTH		      2
#define DIR_WEST		      3
#define DIR_UP			      4
#define DIR_DOWN		      5



/*
 * Exit flags.
 * Used in #ROOMS.
 */
#define EX_ISDOOR		      (A)
#define EX_CLOSED		      (B)
#define EX_LOCKED		      (C)
#define EX_NOFLEE		      (D)
#define EX_PICKPROOF		      (F)
#define EX_NOPASS		      (G)
#define EX_EASY 		      (H)
#define EX_HARD 		      (I)
#define EX_INFURIATING		      (J)
#define EX_NOCLOSE		      (K)
#define EX_NOLOCK		      (L)



/*
 * Sector types.
 * Used in #ROOMS.
 */
#define SECT_INSIDE		      0
#define SECT_CITY		      1
#define SECT_FIELD		      2
#define SECT_FOREST		      3
#define SECT_HILLS		      4
#define SECT_MOUNTAIN		      5
#define SECT_WATER_SWIM 	      6
#define SECT_WATER_NOSWIM	      7
#define SECT_UNUSED		      8
#define SECT_AIR		      9
#define SECT_DESERT		     10
#define SECT_MAX		     11



/*
 * Equpiment wear locations.
 * Used in #RESETS.
 */
#define WEAR_NONE		     -1
#define WEAR_LIGHT		      0
#define WEAR_FINGER_L		      1
#define WEAR_FINGER_R		      2
#define WEAR_NECK_1		      3
#define WEAR_NECK_2		      4
#define WEAR_BODY		      5
#define WEAR_HEAD		      6
#define WEAR_LEGS		      7
#define WEAR_FEET		      8
#define WEAR_HANDS		      9
#define WEAR_ARMS		     10
#define WEAR_SHIELD		     11
#define WEAR_ABOUT		     12
#define WEAR_WAIST		     13
#define WEAR_WRIST_L		     14
#define WEAR_WRIST_R		     15
#define WEAR_WIELD		     16
#define WEAR_HOLD		     17
#define WEAR_FLOAT		     18
#define WEAR_TATTOO		     19
#define WEAR_SECOND_WIELD	     20
#define WEAR_STUCK_IN		     21
#define MAX_WEAR		     22



/***************************************************************************
 *									   *
 *		     VALUES OF INTEREST TO AREA BUILDERS		   *
 *		     (End of this section ... stop here)		   *
 *									   *
 ***************************************************************************/

/*
 * Conditions.
 */
#define COND_DRUNK			0
#define COND_FULL			1
#define COND_THIRST			2
#define COND_HUNGER			3
#define COND_BLOODLUST			4
#define COND_DESIRE			5

#define MAX_COND			6



/*
 * Positions.
 */
#define POS_DEAD		      0
#define POS_MORTAL		      1
#define POS_INCAP		      2
#define POS_STUNNED		      3
#define POS_SLEEPING		      4
#define POS_RESTING		      5
#define POS_SITTING		      6
#define POS_FIGHTING		      7
#define POS_STANDING		      8



/*
 * ACT bits for players.
 */
#define PLR_NPC			ACT_NPC		/* Don't EVER set.	*/
#define PLR_BOUGHT_PET		(B)

/* RT auto flags */
#define PLR_AUTOASSIST		(C)
#define PLR_AUTOEXIT		(D)
#define PLR_AUTOLOOT		(E)
#define PLR_AUTOSAC		(F)
#define PLR_AUTOGOLD		(G)
#define PLR_AUTOSPLIT		(H)
#define PLR_COLOR		(I)
#define PLR_WANTED		(J)
#define PLR_NOTITLE		(K)
/* RT personal flags */
#define PLR_NOEXP		(L)
#define PLR_CHANGED_AFF 	(M)
#define PLR_HOLYLIGHT		(N)
#define PLR_CANLOOT		(P)
#define PLR_NOSUMMON		(Q)
#define PLR_NOFOLLOW		(R)
#define PLR_GHOST		(T)

/* penalty flags */
#define PLR_PERMIT		(U)
#define PLR_LOG 		(W)
#define PLR_DENY		(X)
#define PLR_FREEZE		(Y)

/* #define PLR_QUESTOR		(bb) */
#define PLR_VAMPIRE		(cc)
#define PLR_HARA_KIRI		(dd)
#define PLR_BLINK		(ee)


#define IS_VAMPIRE(ch)	(!IS_NPC(ch) && IS_SET((ch)->act , PLR_VAMPIRE))
#define IS_HARA_KIRI(ch) (IS_SET((ch)->act , PLR_HARA_KIRI))
#define CANT_CHANGE_TITLE(ch) (IS_SET(ch->act , PLR_NOTITLE))
#define IS_BLINK_ON(ch) (IS_SET((ch)->act , PLR_BLINK))
#define CANT_GAIN_EXP(ch) (IS_SET((ch)->act , PLR_NOEXP))

/*
#define IS_PUMPED(ch) ((ch)->last_fight_time != -1 && \
		       current_time - (ch)->last_fight_time < FIGHT_DELAY_TIME)
*/
#define IS_PUMPED(ch) (ch->pumped)
#define SET_FIGHT_TIME(ch)					\
	{							\
		(ch)->last_fight_time = current_time;		\
		(ch)->pumped = TRUE;				\
	}
#define RESET_FIGHT_TIME(ch)					\
	{							\
		(ch)->last_fight_time = -1;			\
		(ch)->pumped = FALSE;				\
	}

/* RT comm flags -- may be used on both mobs and chars */
#define COMM_QUIET		(A)
#define COMM_DEAF		(B)
#define COMM_NOWIZ		(C)
#define COMM_NOAUCTION		(D)
#define COMM_NOGOSSIP		(E)
#define COMM_NOQUESTION 	(F)
#define COMM_NOMUSIC		(G)
#define COMM_NOQUOTE		(I)
#define COMM_SHOUTSOFF		(J)

/* display flags */
#define COMM_TRUE_TRUST 	(K)
#define COMM_COMPACT		(L)
#define COMM_BRIEF		(M)
#define COMM_PROMPT		(N)
#define COMM_COMBINE		(O)
#define COMM_TELNET_GA		(P)
#define COMM_SHOW_AFFECTS	(Q)
#define COMM_NOGRATS		(R)

/* penalties */
#define COMM_NOEMOTE		(T)
#define COMM_NOSHOUT		(U)
#define COMM_NOTELL		(V)
#define COMM_NOCHANNELS 	(W)
#define COMM_SNOOP_PROOF	(Y)
#define COMM_AFK		(Z)

#define COMM_LONG		(aa)
#define COMM_NOTELNET		(bb)
#define COMM_NOIAC		(cc)
#define COMM_NOENG		(dd)

/* IAC replacement if COMM_NOIAC is set */
/* COMM_NOIAC is useful to map 'Ñ' (IAC) to 'ñ' when using win1251 codepage */
#define IAC_REPL	223

/* WIZnet flags */
#define WIZ_ON			(A)
#define WIZ_TICKS		(B)
#define WIZ_LOGINS		(C)
#define WIZ_SITES		(D)
#define WIZ_LINKS		(E)
#define WIZ_DEATHS		(F)
#define WIZ_RESETS		(G)
#define WIZ_MOBDEATHS		(H)
#define WIZ_FLAGS		(I)
#define WIZ_PENALTIES		(J)
#define WIZ_SACCING		(K)
#define WIZ_LEVELS		(L)
#define WIZ_SECURE		(M)
#define WIZ_SWITCHES		(N)
#define WIZ_SNOOPS		(O)
#define WIZ_RESTORE		(P)
#define WIZ_LOAD		(Q)
#define WIZ_NEWBIE		(R)
#define WIZ_PREFIX		(S)
#define WIZ_SPAM		(T)

/*
 * spoken language stuff
 */

struct slang_type
{
	char *name;
	int vnum;
};

struct translation_type
{
	char *common;
	char *slang;
};

/*
 * Prototype for a mob.
 * This is the in-memory version of #MOBILES.
 */
struct	mob_index_data
{
	MOB_INDEX_DATA *	next;
	SPEC_FUN *		spec_fun;
	int 			progtypes;
	SHOP_DATA * 		pShop;
	MPROG_LIST *		mprogs;
	AREA_DATA *		area;		/* OLC */
	int			vnum;
	int			group;
	bool			new_format;
	int			count;
	int			killed;
	char *			player_name;
	mlstring *		short_descr;
	mlstring *		long_descr;
	mlstring *		description;
	int			act;
	int			affected_by;
	int			detection;
	int			alignment;
	int			level;
	int			hitroll;
	int 			hit[3];
	int 			mana[3];
	int			damage[3];
	int			ac[4];
	int			dam_type;
	int			off_flags;
	int			imm_flags;
	int			res_flags;
	int			vuln_flags;
	int			start_pos;
	int			default_pos;
	int			sex;
	int			race;
	int			wealth;
	int			form;
	int			parts;
	int			size;
	char *			material;
	int			mprog_flags;
	int			practicer;
};


/*
 * One character (PC or NPC). *CHAR_DATA*
 */
struct	char_data
{
	CHAR_DATA * 		next;
	CHAR_DATA * 		next_in_room;
	CHAR_DATA * 		master;
	CHAR_DATA * 		leader;
	CHAR_DATA * 		fighting;
	CHAR_DATA * 		reply;
	CHAR_DATA * 		last_fought;
	time_t			last_fight_time;
	bool			pumped;
	time_t			last_death_time;
	CHAR_DATA * 		pet;
	CHAR_DATA *		mprog_target;
	CHAR_DATA *	 	guarding;
	CHAR_DATA * 		guarded_by;
	SPEC_FUN *		spec_fun;
	MOB_INDEX_DATA *	pIndexData;
	DESCRIPTOR_DATA *	desc;
	AFFECT_DATA *		affected;
	NOTE_DATA * 		pnote;
	OBJ_DATA *		carrying;
	OBJ_DATA *		on;
	ROOM_INDEX_DATA *	in_room;
	ROOM_INDEX_DATA *	was_in_room;
	AREA_DATA * 		zone;
	PC_DATA *		pcdata;
	bool			valid;
	char *			name;
	int			id;
	int			version;
	mlstring *		short_descr;
	mlstring *		long_descr;
	mlstring *		description;
	char *			prompt;
	char *			prefix;
	int			group;
	int			sex;
	int			class;
	int			race;
	int			clan;
	int			hometown;
	int			ethos;
	int			level;
	int			trust;
	int 			played;
	int 			lines;	/* for the pager */
	time_t			logon;
	int			timer;
	int			wait;
	int			daze;
	int			hit;
	int 			max_hit;
	int 			mana;
	int 			max_mana;
	int 			move;
	int 			max_move;
	int			gold;
	int			silver;
	int 			exp;
	int			act;
	int			comm;	/* RT added to pad the vector */
	int			wiznet; /* wiz stuff */
	int			imm_flags;
	int			res_flags;
	int			vuln_flags;
	int			invis_level;
	int			incog_level;
	int			affected_by;
	int			detection;
	int			position;
	int			practice;
	int			train;
	int			carry_weight;
	int			carry_number;
	int			saving_throw;
	int			alignment;
	int			hitroll;
	int			damroll;
	int			armor[4];
	int			wimpy;
	/* stats */
	int			perm_stat[MAX_STATS];
	int			mod_stat[MAX_STATS];
	/* parts stuff */
	int			form;
	int			parts;
	int			size;
	char *			material;
	/* mobile stuff */
	int			off_flags;
	int			damage[3];
	int			dam_type;
	int			start_pos;
	int			default_pos;
	int			mprog_delay;
	int 			status;
	bool			extracted;
	char *			in_mind;
	int	 		religion;
	CHAR_DATA *		hunting;	/* hunt data */
	int 			endur;
	bool			riding; /* mount data */
	CHAR_DATA *		mount;
	int			slang;	/* spoken language */
	int			lang; /* interface language */
	CHAR_DATA *		hunter;	/* who quested to slay */
	int			is_aff_holler;
};


/*
 * Data which only PC's have.
 */
struct	pc_data
{
	PC_DATA *		next;
	BUFFER *		buffer;
	bool			valid;
	char *			pwd;
	char *			bamfin;
	char *			bamfout;
	char *			title;
	time_t			last_note;
	time_t			last_idea;
	time_t			last_penalty;
	time_t			last_news;
	time_t			last_changes;
	int 			perm_hit;
	int 			perm_mana;
	int 			perm_move;
	int			true_sex;
	int 			last_level;
	int			condition	[MAX_COND];
	int			learned 	[MAX_SKILL];
	int			points;
	bool			confirm_delete;
	char *			alias[MAX_ALIAS];
	char *			alias_sub[MAX_ALIAS];
	int 			security;	/* OLC */ /* Builder security */
	int			bank_s;
	int			bank_g;
	int 			death;
	int 			played;
	int 			anti_killed;
	int 			has_killed;
	int			questgiver;	/* quest */
	int	 		questpoints;	/* quest */
	int			questtime;	/* quest */
	int			questobj;	/* quest */
	int			questmob;	/* quest */
	QTROUBLE_DATA *		qtrouble;
	ROOM_INDEX_DATA *	questroom;	/* quest */
	int			race;	/* orginal race for polymorph */
	int			pc_killed; /* how many PC's killed by char */
	int			clan_status; /* leader, secondary or common */
	int			petition; /* whom petitioned? */
};


/*
 * Liquids.
 */
#define LIQ_WATER	 0

struct	liq_type
{
	char *	liq_name;
	char *	liq_color;
	int	liq_affect[5];
};



/*
 * Extra description data for a room or object.
 */
struct	ed_data
{
	ED_DATA *	next;		/* Next in list 	    */
	bool			valid;
	char *			keyword;	/* Keyword in look/examine  */
	mlstring *		description;	/* What to see		    */
};


typedef int OPROG_FUN(OBJ_DATA *obj, CHAR_DATA *ch, void *arg);

enum {
	OPROG_WEAR,
	OPROG_REMOVE,
	OPROG_DROP,
	OPROG_SAC,
	OPROG_GIVE,
	OPROG_GREET,
	OPROG_FIGHT,
	OPROG_DEATH,
	OPROG_SPEECH,
	OPROG_ENTRY,
	OPROG_GET,
	OPROG_AREA,

	OPROG_MAX
};

/*
 * Prototype for an object.  *OID*
 */
struct	obj_index_data
{
	OBJ_INDEX_DATA *	next;
	ED_DATA *	ed;
	AFFECT_DATA *		affected;
	AREA_DATA *		area;		/* OLC */
	bool			new_format;
	char *			name;
	mlstring *		short_descr;
	mlstring *		description;
	int			vnum;
	int			reset_num;
	char *			material;
	int			item_type;
	int 			extra_flags;
	int 			wear_flags;
	int			level;
	int			condition;
	int			count;
	int			weight;
	int 			cost;
	int 			value[5];
	int 			limit;
	OPROG_FUN *		oprogs[OPROG_MAX];
};



/*
 * One object.	*OD*
 */
struct	obj_data
{
	OBJ_DATA *		next;
	OBJ_DATA *		next_content;
	OBJ_DATA *		contains;
	OBJ_DATA *		in_obj;
	OBJ_DATA *		on;
	CHAR_DATA * 		carried_by;
	ED_DATA *		ed;
	AFFECT_DATA *		affected;
	OBJ_INDEX_DATA *	pIndexData;
	ROOM_INDEX_DATA *	in_room;
	bool			valid;
	bool			enchanted;
	char *			name;
	mlstring *		short_descr;
	mlstring *		description;
	int			item_type;
	int 			extra_flags;
	int 			wear_flags;
	int			wear_loc;
	int			weight;
	int 			cost;
	int			level;
	int			condition;
	char *			material;
	int			timer;
	int 			value	[5];
	int 			progtypes;
	char *			from;
	int 			altar;
	int 			pit;
	bool			extracted;
	int 			water_float;
};



/*
 * Exit data.
 */
struct	exit_data
{
	union
	{
		ROOM_INDEX_DATA *	to_room;
		int			vnum;
	} u1;
	int		exit_info;
	int		key;
	char *		keyword;
	mlstring *	description;
	EXIT_DATA *	next;		/* OLC */
	int		rs_flags;	/* OLC */
	int		orig_door;	/* OLC */
};



/*
 * Reset commands:
 *   '*': comment
 *   'M': read a mobile
 *   'O': read an object
 *   'P': put object in object
 *   'G': give object to mobile
 *   'E': equip object to mobile
 *   'D': set state of door
 *   'R': randomize room exits
 *   'S': stop (end of list)
 */

/*
 * Area-reset definition.
 */
struct	reset_data
{
	RESET_DATA *	next;
	char		command;
	int		arg1;
	int		arg2;
	int		arg3;
	int		arg4;
};



/*
 * Area definition.
 */
struct	area_data
{
	AREA_DATA * 	next;
	RESET_DATA *	reset_first;
	RESET_DATA *	reset_last;
	HELP_AREA *	helps;
	char *		file_name;
	char *		name;
	char *		writer;
	char *		credits;
	int		age;
	int		nplayer;
	int		low_range;
	int		high_range;
	int		min_vnum;
	int		max_vnum;
	bool		empty;
	char *		builders;	/* OLC */ /* Listing of */
	int		vnum;		/* OLC */ /* Area vnum  */
	int		area_flags;	/* OLC */
	int		security;	/* OLC */ /* Value 1-9  */
	unsigned int	count;
	mlstring *	resetmsg;
	int		area_flag;
};

struct room_history_data
{
	char *name;
	int went;
	ROOM_HISTORY_DATA *next;
	ROOM_HISTORY_DATA *prev;
};

/*
 * Room type.
 */
struct	room_index_data
{
	ROOM_INDEX_DATA *	next;
	ROOM_INDEX_DATA *	aff_next;
	CHAR_DATA * 		people;
	OBJ_DATA *		contents;
	ED_DATA *	ed;
	AREA_DATA * 		area;
	EXIT_DATA * 		exit	[6];
	RESET_DATA *		reset_first;	/* OLC */
	RESET_DATA *		reset_last;	/* OLC */
	mlstring *		name;
	mlstring *		description;
	char *			owner;
	int			vnum;
	int			clan;
	int			room_flags;
	int			light;
	int			sector_type;
	int			heal_rate;
	int			mana_rate;
	ROOM_HISTORY_DATA * 	history;
	AFFECT_DATA *		affected;
	int			affected_by;
};

/*
 * MOBprog definitions
 */                   
#define TRIG_ACT	(A)
#define TRIG_BRIBE	(B)
#define TRIG_DEATH	(C)
#define TRIG_ENTRY	(D)
#define TRIG_FIGHT	(E)
#define TRIG_GIVE	(F)
#define TRIG_GREET	(G)
#define TRIG_GRALL	(H)
#define TRIG_KILL	(I)
#define TRIG_HPCNT	(J)
#define TRIG_RANDOM	(K)
#define TRIG_SPEECH	(L)
#define TRIG_EXIT	(M)
#define TRIG_EXALL	(N)
#define TRIG_DELAY	(O)
#define TRIG_SURR	(P)

struct mprog_list
{
	int		trig_type;
	char *		trig_phrase;
	int		vnum;
	char *  	code;
	MPROG_LIST * 	next;
	bool		valid;
};

struct mprog_code
{
	int		vnum;
	char *		code;
	MPROG_CODE *	next;
};

/*
 * Types of attacks.
 * Must be non-overlapping with spell/skill types,
 * but may be arbitrary beyond that.
 */
#define TYPE_UNDEFINED		     -1
#define TYPE_HIT		     1000
#define TYPE_HUNGER		     999



/*
 *  Target types.
 */
#define TAR_IGNORE		    0
#define TAR_CHAR_OFFENSIVE	    1
#define TAR_CHAR_DEFENSIVE	    2
#define TAR_CHAR_SELF		    3
#define TAR_OBJ_INV		    4
#define TAR_OBJ_CHAR_DEF	    5
#define TAR_OBJ_CHAR_OFF	    6

#define TARGET_CHAR		    0
#define TARGET_OBJ		    1
#define TARGET_ROOM		    2
#define TARGET_NONE		    3



/*
 * Skills include spells as a particular case.
 */
struct	skill_type
{
	char *		name;			/* Name of skill	    */
	int		skill_level[MAX_CLASS]; /* Level needed by class    */
	int		rating[MAX_CLASS];	/* How hard it is to learn  */
	SPELL_FUN *	spell_fun;		/* Spell pointer	    */
	int		target; 		/* Legal targets	    */
	int		minimum_position;	/* Position for caster	    */
	int *		pgsn;			/* Pointer to  gsn	    */
	int		slot;			/* Slot for #OBJECT loading */
	int		min_mana;		/* Minimum mana used	    */
	int		beats;			/* Waiting time after use   */
	char *		noun_damage;		/* Damage message	    */
	char *		msg_off;		/* Wear off message	    */
	char *		msg_obj;		/* Wear off message for obj */
	int		clan;			/* Cabal spells 	    */
	int		race;			/* Race spells		    */
	int		align;			/* alignment of spells	    */
	int		group;			/* skill group for prac	    */
};


/*
 * These are skill_lookup return values for common skills and spells.
 */
extern	int	gsn_backstab;
extern	int	gsn_dodge;
extern	int	gsn_envenom;
extern	int	gsn_hide;
extern	int	gsn_peek;
extern	int	gsn_pick_lock;
extern	int	gsn_sneak;
extern	int	gsn_steal;

extern	int	gsn_disarm;
extern	int	gsn_enhanced_damage;
extern	int	gsn_kick;
extern	int	gsn_parry;
extern	int	gsn_rescue;
extern	int	gsn_second_attack;
extern	int	gsn_third_attack;

extern	int	gsn_blindness;
extern	int	gsn_charm_person;
extern	int	gsn_curse;
extern	int	gsn_remove_curse;
extern	int	gsn_invis;
extern	int	gsn_mass_invis;
extern	int	gsn_plague;
extern	int	gsn_poison;
extern	int	gsn_sleep;
extern	int	gsn_fly;
extern	int	gsn_death_protection;
extern	int	gsn_sanctuary;

/* new gsns */
extern int  gsn_axe;
extern int  gsn_dagger;
extern int  gsn_flail;
extern int  gsn_mace;
extern int  gsn_polearm;
extern int  gsn_shield_block;
extern int  gsn_spear;
extern int  gsn_sword;
extern int  gsn_whip;

extern int  gsn_bash;
extern int  gsn_berserk;
extern int  gsn_dirt;
extern int  gsn_hand_to_hand;
extern int  gsn_trip;

extern int  gsn_fast_healing;
extern int  gsn_haggle;
extern int  gsn_lore;
extern int  gsn_meditation;

extern int  gsn_scrolls;
extern int  gsn_staves;
extern int  gsn_wands;
extern int  gsn_recall;
extern int  gsn_detect_hidden;

extern int  gsn_fourth_attack;
extern int  gsn_dual_backstab;
extern int  gsn_cleave;
extern int  gsn_counter;
extern int  gsn_ambush;
extern int  gsn_camouflage;
extern int  gsn_circle;
extern int  gsn_nerve;
extern int  gsn_endure;
extern int  gsn_quiet_movement;
extern int  gsn_herbs;
extern int  gsn_tame;
extern int  gsn_butcher;
extern int  gsn_assassinate;
extern int  gsn_caltrops;
extern int  gsn_throw;
extern int  gsn_strangle;
extern int  gsn_blackjack;
extern int  gsn_bloodthirst;
extern int  gsn_spellbane;
extern int  gsn_resistance;
extern int  gsn_deathblow;
extern int  gsn_fade;
extern int  gsn_garble;
extern int  gsn_confuse;
extern int  gsn_track;
extern int  gsn_vanish;
extern int  gsn_chaos_blade;
extern int  gsn_terangreal;
extern int  gsn_dispel_evil;
extern int  gsn_dispel_good;
extern int  gsn_wrath;
extern int  gsn_stalker;
extern int  gsn_tesseract;
extern int  gsn_randomizer;
extern int  gsn_trophy;
extern int  gsn_truesight;
extern int  gsn_brew;
extern int  gsn_flamestrike;
extern int  gsn_shadowlife;
extern int  gsn_ruler_badge;
extern int  gsn_remove_badge;
extern int  gsn_dragon_strength;
extern int  gsn_dragon_breath;
extern int  gsn_warcry;
extern int  gsn_entangle;
extern int  gsn_dragonsword;
extern int  gsn_knight;
extern int  gsn_guard;
extern int  gsn_guard_call;
extern int  gsn_love_potion;
extern int  gsn_deafen;
extern int  gsn_protective_shield;
extern int  gsn_protection_heat_cold;
extern int  gsn_trance;
extern int  gsn_demon_summon;
extern int  gsn_nightwalker;
extern int  gsn_bear_call;
extern int  gsn_squire;
extern int  gsn_lightning_bolt;
extern int  gsn_disperse;
extern int  gsn_bless;
extern int  gsn_weaken;
extern int  gsn_haste;
extern int  gsn_cure_critical;
extern int  gsn_cure_serious;
extern int  gsn_burning_hands;
extern int  gsn_acid_blast;
extern int  gsn_ray_of_truth;
extern int  gsn_spell_craft;
extern int  gsn_giant_strength;
extern int  gsn_explode;
extern int  gsn_acid_breath;
extern int  gsn_fire_breath;
extern int  gsn_gas_breath;
extern int  gsn_frost_breath;
extern int  gsn_lightning_breath;
extern int  gsn_cure_light;
extern int  gsn_magic_missile;
extern int  gsn_demonfire;
extern int  gsn_faerie_fire;
extern int  gsn_shield;
extern int  gsn_chill_touch;
extern int  gsn_second_weapon;
extern int  gsn_target;
extern int  gsn_sand_storm;
extern int  gsn_scream;
extern int  gsn_enchant_sword;
extern int  gsn_tiger_power;
extern int  gsn_lion_call;
extern int  gsn_hara_kiri;
extern int  gsn_mouse;
extern int  gsn_enhanced_armor;
extern int  gsn_vampire;
extern int  gsn_vampiric_bite;
extern int  gsn_light_res;
extern int  gsn_blink;
extern int  gsn_hunt;
extern int  gsn_path_find;
extern int  gsn_critical;
extern int  gsn_detect_sneak;
extern int  gsn_mend;
extern int  gsn_shielding;
extern int  gsn_blind_fighting;
extern int  gsn_riding;
extern int  gsn_smithing;
extern int  gsn_swimming;
extern int  gsn_move_camf;
extern int  gsn_protection_heat;
extern int  gsn_protection_cold;
extern int  gsn_teleport;
extern int  gsn_witch_curse;
extern int  gsn_terangreal;
extern int  gsn_sebat;
extern int  gsn_kassandra;
extern int  gsn_matandra;
extern int  gsn_armor_use;
extern int  gsn_world_find;
extern int  gsn_vampiric_touch;
extern int  gsn_cure_poison;
extern int  gsn_fire_shield;
extern int  gsn_fear;
extern int  gsn_settraps;
extern int  gsn_mental_attack;
extern int  gsn_secondary_attack;
extern int  gsn_mortal_strike;
extern int  gsn_shield_cleave;
extern int  gsn_weapon_cleave;
extern int  gsn_slow;
extern int  gsn_imp_invis;
extern int  gsn_camp;
extern int  gsn_push;
extern int  gsn_tail;
extern int  gsn_power_stun;
extern int  gsn_clan_recall;
extern int  gsn_escape;
extern int  gsn_lay_hands;
extern int  gsn_grip;
extern int  gsn_concentrate;
extern int  gsn_master_sword;
extern int  gsn_master_hand;
extern int  gsn_fifth_attack;
extern int  gsn_area_attack;
extern int  gsn_reserved;
extern int  gsn_bandage;
extern int  gsn_web;
extern int  gsn_bow;
extern int  gsn_bash_door;
extern int  gsn_katana;
extern int  gsn_bluefire;
extern int  gsn_crush;
extern int  gsn_perception;
extern int  gsn_control_animal;
extern int  gsn_sanctify_lands;
extern int  gsn_deadly_venom;
extern int  gsn_cursed_lands;
extern int  gsn_lethargic_mist;
extern int  gsn_black_death;
extern int  gsn_mysterious_dream;
extern int  gsn_sense_life;
extern int  gsn_arrow;
extern int  gsn_lance;
extern int  gsn_evil_spirit;
extern int  gsn_make_arrow;
extern int  gsn_green_arrow;
extern int  gsn_red_arrow;
extern int  gsn_white_arrow;
extern int  gsn_blue_arrow;
extern int  gsn_make_bow;
extern int  gsn_blindness_dust;
extern int  gsn_poison_smoke;
extern int  gsn_mastering_spell;
extern int  gsn_frenzy;
extern int  gsn_holler;
extern int  gsn_holler_self;
extern int gsn_thumbling;

/*
 * Utility macros.
 */
#define IS_VALID(data)		((data) != NULL && (data)->valid)
#define VALIDATE(data)		((data)->valid = TRUE)
#define INVALIDATE(data)	((data)->valid = FALSE)
#define UMIN(a, b)		((a) < (b) ? (a) : (b))
#define UMAX(a, b)		((a) > (b) ? (a) : (b))
#define URANGE(a, b, c) 	((b) < (a) ? (a) : ((b) > (c) ? (c) : (b)))
#define LOWER(c)		((c) >= 'A' && (c) <= 'Z' ? (c)+'a'-'A' : (c))
#define UPPER(c)		((c) >= 'a' && (c) <= 'z' ? (c)+'A'-'a' : (c))
#define IS_SET(flag, bit)	((flag) & (bit))
#define SET_BIT(var, bit)	((var) |= (bit))
#define REMOVE_BIT(var, bit)	((var) &= ~(bit))
#define IS_WATER(var) 	(((var)->sector_type == SECT_WATER_SWIM) || \
				 ((var)->sector_type == SECT_WATER_NOSWIM))
#define PERCENT(cur, max)	(max==0?0:((cur)*100)/(max))
#define IS_NULLSTR(str)		((str) == NULL || (str)[0] == '\0')
#define strend(s) (strchr(s, '\0'))
#define ENTRE(min,num,max)	( ((min) < (num)) && ((num) < (max)) )
#define CHECK_POS(a, b, c)						\
		{							\
			(a) = (b);					\
			if ((a) < 0)					\
				bug("CHECK_POS : " c " == %d < 0", a);	\
		}

/*
 * Character macros.
 */
#define IS_NPC(ch)		(IS_SET((ch)->act, ACT_NPC))
#define IS_IMMORTAL(ch) 	(get_trust(ch) >= LEVEL_IMMORTAL)
#define IS_HERO(ch)		(get_trust(ch) >= LEVEL_HERO)
#define IS_TRUSTED(ch,level)	(get_trust((ch)) >= (level))
#define IS_AFFECTED(ch, sn)	(IS_SET((ch)->affected_by, (sn)))
#define CAN_DETECT(ch, sn)	(IS_SET((ch)->detection, (sn)))

#define IS_PK(ch, vt)		(!IS_NPC((ch)) & !IS_NPC((vt)))

#define RACE(ch)		(ch->race)
#define ORG_RACE(ch)		(IS_NPC(ch) ? ch->pIndexData->race : ch->pcdata->race)

#define GET_AGE(ch)		((int) (17 + ((ch)->played \
				    + current_time - (ch)->logon)/72000))

#define IS_GOOD(ch)		(ch->alignment >= 350)
#define IS_EVIL(ch)		(ch->alignment <= -350)
#define IS_NEUTRAL(ch)		(!IS_GOOD(ch) && !IS_EVIL(ch))

#define IS_AWAKE(ch)		(ch->position > POS_SLEEPING)
#define GET_AC(ch,type) 	((ch)->armor[type]			    \
			+ (IS_AWAKE(ch)			    \
			? dex_app[get_curr_stat(ch,STAT_DEX)].defensive : 0))
#define GET_HITROLL(ch) \
		((ch)->hitroll+str_app[get_curr_stat(ch,STAT_STR)].tohit)
#define GET_DAMROLL(ch) \
		((ch)->damroll+str_app[get_curr_stat(ch,STAT_STR)].todam)

#define IS_OUTSIDE(ch)		(!IS_SET(				    \
				    (ch)->in_room->room_flags,		    \
				    ROOM_INDOORS))

#define WAIT_STATE(ch, npulse)	((ch)->wait = IS_IMMORTAL((ch))?1:UMAX((ch)->wait, (npulse)))
#define DAZE_STATE(ch, npulse)	((ch)->daze = UMAX((ch)->daze, (npulse)))
#define get_carry_weight(ch)	((ch)->carry_weight + (ch)->silver/10 +  \
						      (ch)->gold * 2 / 5)
#define HAS_TRIGGER(ch,trig)	(IS_SET((ch)->pIndexData->mprog_flags,(trig)))
#define IS_SWITCHED( ch )       (ch->desc && ch->desc->original)
#define IS_BUILDER(ch, Area)	(!IS_NPC(ch) && !IS_SWITCHED(ch) &&	\
				(ch->pcdata->security >= Area->security	\
				|| strstr(Area->builders, ch->name)	\
				|| strstr(Area->builders, "All")))

/*
 * room macros
 */

#define IS_ROOM_AFFECTED(room, sn)	(IS_SET((room)->affected_by, (sn)))
#define IS_RAFFECTED(room, sn)	(IS_SET((room)->affected_by, (sn)))

#define MOUNTED(ch)	((!IS_NPC(ch) && ch->mount && ch->riding) ? \
				ch->mount : NULL)
#define RIDDEN(ch)	((IS_NPC(ch) && ch->mount && ch->riding) ? \
				ch->mount : NULL)
#define IS_DRUNK(ch)	(IS_NPC(ch)  ? \
			      FALSE : ch->pcdata->condition[COND_DRUNK] > 10)

#define IS_GOLEM(ch)		(IS_NPC(ch) &&\
				(ch->pIndexData->vnum==MOB_VNUM_LESSER_GOLEM \
				 || ch->pIndexData->vnum==MOB_VNUM_STONE_GOLEM \
				 || ch->pIndexData->vnum==MOB_VNUM_IRON_GOLEM \
				 || ch->pIndexData->vnum==MOB_VNUM_ADAMANTITE_GOLEM))
/*
 * Object macros.
 */
#define CAN_WEAR(obj, part)	(IS_SET((obj)->wear_flags,  (part)))
#define IS_OBJ_STAT(obj, stat)	(IS_SET((obj)->extra_flags, (stat)))
#define IS_WEAPON_STAT(obj,stat)(IS_SET((obj)->value[4],(stat)))
#define WEIGHT_MULT(obj)	((obj)->item_type == ITEM_CONTAINER ? \
	(obj)->value[4] : 100)

/* skill defines */
#define SKILL_LEVEL_OK(ch,sn)	(ch->level >= skill_table[sn].skill_level[ch->class])
#define SKILL_RACE_OK(ch,sn)	IS_SET(skill_table[sn].race,(1 << (ORG_RACE(ch) - 1)))
#define SKILL_CLAN_OK(ch,sn)	(skill_table[sn].clan == CLAN_NONE || skill_table[sn].clan == ch->clan)
#define SKILL_ALIGN_OK(ch,sn)	(skill_table[sn].align == ALIGN_NONE || skill_table[sn].align == ch->alignment)

#define SKILL_OK(ch, sn)	(IS_IMMORTAL(ch) || \
				 (((SKILL_LEVEL_OK(ch, sn) && \
				    SKILL_RACE_OK(ch, sn)) || \
				   skill_is_native(ch, sn)) && \
				  SKILL_CLAN_OK(ch, sn) && \
				  SKILL_ALIGN_OK(ch, sn)))

/*
 * Description macros.
 */
/* new defines */
#define MAX_CHARM(ch)	((get_curr_stat(ch,STAT_INT) / 4) + (ch->level / 30))

/*
 * Structure for a social in the socials table.
 */
struct	social_type
{
	char      name[20];
	char *    char_no_arg;
	char *    others_no_arg;
	char *    char_found;
	char *    others_found;
	char *    vict_found;
	char *    char_not_found;
	char *    char_auto;
	char *    others_auto;
};



/*
 * Global constants.
 */
extern	const	struct	str_app_type	str_app 	[26];
extern	const	struct	int_app_type	int_app 	[26];
extern	const	struct	wis_app_type	wis_app 	[26];
extern	const	struct	dex_app_type	dex_app 	[26];
extern	const	struct	con_app_type	con_app 	[26];

extern	const	struct	slang_type	slang_table	   [SLANG_MAX];
extern	const	struct	translation_type translation_table	[];
extern	const	struct	cmd_type	cmd_table	[];
extern	const	struct	class_type	class_table	[MAX_CLASS];
extern	const	struct	weapon_type	weapon_table	[];
extern	const	struct	item_type	item_table	[];
extern	const	struct	wiznet_type	wiznet_table	[];
extern	const	struct	attack_type	attack_table	[];
extern	const	struct	race_type	race_table	[];
extern	const	struct	pc_race_type	pc_race_table	[MAX_PC_RACE];
extern		struct	clan_type	clan_table	[];
extern	const	struct	color_type	color_table	[];
extern	const	struct	religion_type	religion_table	[];
extern	const	struct	spec_type	spec_table	[];
extern	const	struct	liq_type	liq_table	[];
extern	const	struct	skill_type	skill_table	[MAX_SKILL];
extern	const	struct	ethos_type	ethos_table	[];
extern		struct social_type	social_table	[MAX_SOCIALS];
extern	char *	const			title_table	[MAX_CLASS]
							[MAX_LEVEL+1]
							[2];



/*
 * Global variables.
 */
extern		HELP_DATA	  *	help_first;
extern		SHOP_DATA	  *	shop_first;

extern		CHAR_DATA	  *	char_list;
extern		DESCRIPTOR_DATA   *	descriptor_list;
extern		OBJ_DATA	  *	object_list;

extern		MPROG_CODE	  *	mprog_list;

extern		ROOM_INDEX_DATA   *	top_affected_room;

extern		char			bug_buf 	[];
extern		time_t			current_time;
extern		bool			fLogAll;
extern		FILE *			fpReserve;
extern		KILL_DATA		kill_table	[];
extern		char			log_buf 	[];
extern		TIME_INFO_DATA		time_info;
extern		WEATHER_DATA		weather_info;
extern		int			total_levels;
extern		int			reboot_counter;

extern		bool			MOBtrigger;

/*
 * OS-dependent declarations.
 * These are all very standard library functions,
 *   but some systems have incomplete or non-ansi header files.
 */
#if	defined(_AIX)
char *	crypt		(const char *key, const char *salt);
#endif

#if	defined(apollo)
int	atoi		(const char *string);
void *	calloc		(unsigned nelem, size_t size);
char *	crypt		(const char *key, const char *salt);
#endif

#if	defined(hpux)
char *	crypt		(const char *key, const char *salt);
#endif

#if	defined(linux)
char *	crypt		(const char *key, const char *salt);
#endif

#if	defined(BSD44)
char *	crypt		(const char *key, const char *salt);
#endif

#if	defined(MIPS_OS)
char *	crypt		(const char *key, const char *salt);
#endif

#if	defined(NeXT)
char *	crypt		(const char *key, const char *salt);
#endif

#if	defined(sequent)
char *	crypt		(const char *key, const char *salt);
int	fclose		(FILE *stream);
int	fprintf 	(FILE *stream, const char *format, ...);
int	fread		(void *ptr, int size, int n, FILE *stream);
int	fseek		(FILE *stream, int offset, int ptrname);
void	perror		(const char *s);
int	ungetc		(int c, FILE *stream);
#endif

#if	defined(ultrix)
char *	crypt		(const char *key, const char *salt);
#endif



/*
 * The crypt(3) function is not available on some operating systems.
 * In particular, the U.S. Government prohibits its export from the
 *   United States to foreign countries.
 * Turn on NOCRYPT to keep passwords in plain text.
 */
#if	defined(NOCRYPT)
#define crypt(s1, s2)	(s1)
#endif



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

#define PLAYER_DIR	"../player/"		/* Player files */
#define GOD_DIR 	"../gods/"		/* list of gods */
#define TEMP_FILE	"../player/romtmp"
#define NULL_FILE	"/dev/null"		/* To reserve one stream */

#define AREA_LIST	"area.lst"  /* List of areas*/
#define BUG_FILE	"bugs.txt" /* For 'bug' and bug()*/
#define TYPO_FILE	"typos.txt" /* For 'typo'*/
#define NOTE_FILE	"notes.not"/* For 'notes'*/
#define IDEA_FILE	"ideas.not"
#define PENALTY_FILE	"penal.not"
#define NEWS_FILE	"news.not"
#define CHANGES_FILE	"chang.not"
#define SHUTDOWN_FILE	"shutdown.txt"/* For 'shutdown'*/
#define BAN_FILE	"ban.txt"
#define AREASTAT_FILE	"area_stat.txt"
#define IMM_LOG_FILE	"../gods/immortals.log"

/*
 * Our function prototypes.
 * One big lump ... this is every function in Merc.
 */
#define MID	MOB_INDEX_DATA
#define OD	OBJ_DATA
#define OID	OBJ_INDEX_DATA
#define RID	ROOM_INDEX_DATA
#define SF	SPEC_FUN
#define AD	AFFECT_DATA
#define MPC	MPROG_CODE

/* act_hera.c */
/* enter.c */
RID  *get_random_room	(CHAR_DATA *ch);
/* hunt.c */
void hunt_victim(CHAR_DATA *ch);

/* ban.c */
bool	check_ban	(char *site, int type);

/* effect.c */
void	acid_effect	(void *vo, int level, int dam, int target);
void	cold_effect	(void *vo, int level, int dam, int target);
void	fire_effect	(void *vo, int level, int dam, int target);
void	poison_effect	(void *vo, int level, int dam, int target);
void	shock_effect	(void *vo, int level, int dam, int target);
void	sand_effect	(void *vo, int level, int dam, int target);
void	scream_effect	(void *vo, int level, int dam, int target);


/* handler.c */
AD	*affect_find (AFFECT_DATA *paf, int sn);
void	affect_check	(CHAR_DATA *ch, int where, int vector);
int	count_users	(OBJ_DATA *obj);
void	deduct_cost	(CHAR_DATA *ch, int cost);
void	affect_enchant	(OBJ_DATA *obj);
int	check_immune	(CHAR_DATA *ch, int dam_type);
bool	check_material	(OBJ_DATA *obj, char *material);
bool	is_metal	(OBJ_DATA *obj);
bool	clan_ok	(CHAR_DATA *ch, int sn);
int	weapon_type	(const char *name);
char	*weapon_name	(int weapon_Type);
char	*item_name	(int item_type);
bool	is_old_mob	(CHAR_DATA *ch);
int	get_skill	(CHAR_DATA *ch, int sn);
int	get_weapon_sn	(CHAR_DATA *ch, int type);
int	get_weapon_skill (CHAR_DATA *ch, int sn);
int	get_age 	(CHAR_DATA *ch);
void	reset_char	(CHAR_DATA *ch);
int	get_trust	(CHAR_DATA *ch);
int	get_curr_stat	(CHAR_DATA *ch, int stat);
int	get_max_train	(CHAR_DATA *ch, int stat);
int	get_max_train2	(CHAR_DATA *ch, int stat);
int	can_carry_n	(CHAR_DATA *ch);
int	can_carry_w	(CHAR_DATA *ch);
bool	is_name 	(const char *str, const char *namelist);
bool	is_name_imm	(char *str, char *namelist);
void	affect_to_char	(CHAR_DATA *ch, AFFECT_DATA *paf);
void	affect_to_obj	(OBJ_DATA *obj, AFFECT_DATA *paf);
void	affect_remove	(CHAR_DATA *ch, AFFECT_DATA *paf);
void	affect_remove_obj (OBJ_DATA *obj, AFFECT_DATA *paf);
void	affect_strip	(CHAR_DATA *ch, int sn);
bool	is_affected	(CHAR_DATA *ch, int sn);
void	affect_to_room	(ROOM_INDEX_DATA *room, AFFECT_DATA *paf);
void	affect_remove_room	(ROOM_INDEX_DATA *room, AFFECT_DATA *paf);
void	affect_strip_room	(ROOM_INDEX_DATA *ch, int sn);
bool	is_affected_room	(ROOM_INDEX_DATA *ch, int sn);
void	affect_join_room	(ROOM_INDEX_DATA *ch, AFFECT_DATA *paf);
void	affect_join	(CHAR_DATA *ch, AFFECT_DATA *paf);
void	char_from_room	(CHAR_DATA *ch);
void	char_to_room	(CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex);
void	obj_to_char	(OBJ_DATA *obj, CHAR_DATA *ch);
void	obj_from_char	(OBJ_DATA *obj);
int	apply_ac	(OBJ_DATA *obj, int iWear, int type);
OD *	get_eq_char	(CHAR_DATA *ch, int iWear);
void	equip_char	(CHAR_DATA *ch, OBJ_DATA *obj, int iWear);
void	unequip_char	(CHAR_DATA *ch, OBJ_DATA *obj);
int	count_obj_list	(OBJ_INDEX_DATA *obj, OBJ_DATA *list);
void	obj_from_room	(OBJ_DATA *obj);
void	obj_to_room	(OBJ_DATA *obj, ROOM_INDEX_DATA *pRoomIndex);
void	obj_to_obj	(OBJ_DATA *obj, OBJ_DATA *obj_to);
void	obj_from_obj	(OBJ_DATA *obj);
void	extract_obj	(OBJ_DATA *obj);
void	extract_obj_nocount	(OBJ_DATA *obj);
void	extract_obj_1	(OBJ_DATA *obj, bool count);
void	extract_char	(CHAR_DATA *ch, bool fPull);
void	extract_char_nocount	(CHAR_DATA *ch, bool fPull);
void	extract_char_org	(CHAR_DATA *ch, bool fPull, bool Count);
CHAR_DATA *	get_char_room	(CHAR_DATA *ch, const char *argument);
CHAR_DATA *	get_char_room2	(CHAR_DATA *ch, ROOM_INDEX_DATA *room,const char *argument, int *number);
CHAR_DATA *	get_char_world	(CHAR_DATA *ch, const char *argument);
CHAR_DATA *	get_char_area	(CHAR_DATA *ch, const char *argument);
OD *	get_obj_type	(OBJ_INDEX_DATA *pObjIndexData);
OD *	get_obj_list	(CHAR_DATA *ch, const char *argument,
			    OBJ_DATA *list);
OD *	get_obj_carry	(CHAR_DATA *ch, const char *argument);
OD *	get_obj_wear	(CHAR_DATA *ch, const char *argument);
OD *	get_obj_here	(CHAR_DATA *ch, const char *argument);
OD *	get_obj_world	(CHAR_DATA *ch, const char *argument);
OD *	create_money	(int gold, int silver);
int	get_obj_number	(OBJ_DATA *obj);
int	get_obj_realnumber	(OBJ_DATA *obj);
int	get_obj_weight	(OBJ_DATA *obj);
int	get_true_weight (OBJ_DATA *obj);
bool	room_is_dark	(CHAR_DATA *ch);
bool	room_dark	(ROOM_INDEX_DATA *pRoomIndex);
bool	isn_dark_safe	(CHAR_DATA *ch);
bool	is_room_owner	(CHAR_DATA *ch, ROOM_INDEX_DATA *room);
bool	room_is_private (ROOM_INDEX_DATA *pRoomIndex);
bool	can_see 	(CHAR_DATA *ch, CHAR_DATA *victim);
bool	can_see_obj	(CHAR_DATA *ch, OBJ_DATA *obj);
bool	can_see_room	(CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex);
bool	can_drop_obj	(CHAR_DATA *ch, OBJ_DATA *obj);
char *	item_type_name	(OBJ_DATA *obj);
char *	affect_loc_name (int location);
char * raffect_loc_name (int location);
char *	affect_bit_name (int vector);
char *	detect_bit_name (int location);
char * raffect_bit_name (int vector);
char *	extra_bit_name	(int extra_flags);
char *	wear_bit_name	(int wear_flags);
char *	act_bit_name	(int act_flags);
char *	off_bit_name	(int off_flags);
char *	imm_bit_name	(int imm_flags);
char *	form_bit_name	(int form_flags);
char *	part_bit_name	(int part_flags);
char *	weapon_bit_name (int weapon_flags);
char *	comm_bit_name	(int comm_flags);
char *	cont_bit_name	(int cont_flags);
char *	flag_room_name	(int vector);
void	room_record	(char *name, ROOM_INDEX_DATA *room,int door);
int	ch_skill_nok	(CHAR_DATA *ch , int sn);
int	skill_is_native	(CHAR_DATA *ch , int sn);
int	affect_check_obj	(CHAR_DATA *ch, int vector);
bool	is_safe_rspell	(int level, CHAR_DATA *victim);
int	count_charmed	(CHAR_DATA *ch);
void	add_mind	(CHAR_DATA *ch, char *str);
void	remove_mind	(CHAR_DATA *ch, char *str);
void	back_home	(CHAR_DATA *ch);
CHAR_DATA*	find_char	(CHAR_DATA *ch, const char *argument, int door, int range);
CHAR_DATA*	get_char_spell	(CHAR_DATA *ch, const char *argument, int *door, int range);
void	path_to_track	(CHAR_DATA *ch, CHAR_DATA *victim, int door);
bool	in_PK(CHAR_DATA *ch, CHAR_DATA *victim);
bool	can_gate(CHAR_DATA *ch, CHAR_DATA *victim);
bool	obj_is_pit(OBJ_DATA *obj);

/* interp.c */
void	interpret	(CHAR_DATA *ch, const char *argument);
void	interpret_raw	(CHAR_DATA *ch, const char *argument, bool is_order);
bool	is_number	(const char *argument);
int	number_argument (const char *argument, char *arg);
int	mult_argument	(const char *argument, char *arg);
const char *	one_argument	(const char *argument, char *arg_first);
char* PERS(CHAR_DATA *ch, CHAR_DATA *looker);


/* save.c */
void	save_char_obj	(CHAR_DATA *ch, bool reboot);
bool	load_char_obj	(DESCRIPTOR_DATA *d, const char *name);

/* skills.c */
int	exp_to_level	(CHAR_DATA *ch, int points);
int	base_exp	(CHAR_DATA *ch, int points);
int	exp_this_level	(CHAR_DATA *ch, int level, int points);

int	exp_per_level	(CHAR_DATA *ch, int points);
void	check_improve	(CHAR_DATA *ch, int sn, bool success,
				    int multiplier);
int	group_lookup	(const char *name);
void	group_add	(CHAR_DATA *ch);

/* special.c */
SF *	spec_lookup	(const char *name);
char *	spec_name	(SPEC_FUN *function);

#undef	MID
#undef	OD
#undef	OID
#undef	RID
#undef	SF
#undef	AD

/*****************************************************************************
 *                                    OLC                                    *
 *****************************************************************************/

/*
 * Object defined in limbo.are
 * Used in save.c to load objects that don't exist.
 */
#define OBJ_VNUM_DUMMY	30

/*
 * Area flags.
 */
#define         AREA_NONE       0
#define         AREA_CHANGED    1	/* Area has been modified. */
#define         AREA_ADDED      2	/* Area has been added to. */
#define         AREA_LOADING    4	/* Used for counting in db.c */

#define MAX_DIR	6
#define NO_FLAG -99	/* Must not be used in flags or stats. */

/*
 * Global Constants
 */
extern	char *	const	dir_name        [];
extern	const	int	rev_dir         [];          /* ROM OLC */
extern	const	struct	spec_type	spec_table	[];

/*
 * Global variables
 */
extern		AREA_DATA *		area_first;
extern		AREA_DATA *		area_last;
extern		SHOP_DATA *		shop_last;

extern		int			top_affect;
extern		int			top_area;
extern		int			top_ed;
extern		int			top_exit;
extern		int			top_help;
extern		int			top_mob_index;
extern		int			top_obj_index;
extern		int			top_reset;
extern		int			top_room;
extern		int			top_shop;

extern		int			top_vnum_mob;
extern		int			top_vnum_obj;
extern		int			top_vnum_room;

extern		char			str_empty       [1];

extern		MOB_INDEX_DATA *	mob_index_hash  [MAX_KEY_HASH];
extern		OBJ_INDEX_DATA *	obj_index_hash  [MAX_KEY_HASH];
extern		ROOM_INDEX_DATA *	room_index_hash [MAX_KEY_HASH];

struct flag_type
{
    char *name;
    int bit;
    bool settable;
};

struct position_type
{
    char *name;
    char *short_name;
};

struct sex_type
{
    char *name;
};

struct size_type
{
    char *name;
};

struct	bit_type
{
	const	struct	flag_type *	table;
	char *				help;
};

#endif

