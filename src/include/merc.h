#ifndef _MERC_H_
#define _MERC_H_

/*
 * $Id: merc.h,v 1.26 1998-06-06 10:51:56 fjoe Exp $
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

/*
 * Accommodate old non-Ansi compilers.
 */
#if defined(TRADITIONAL)
#define const
#define args( list )			( )
#define DECLARE_DO_FUN( fun )		void fun( )
#define DECLARE_SPEC_FUN( fun ) 	bool fun( )
#define DECLARE_SPELL_FUN( fun )	void fun( )

#define DECLARE_MPROG_FUN_BRIBE( fun ) void fun( )
#define DECLARE_MPROG_FUN_ENTRY( fun ) void fun( )
#define DECLARE_MPROG_FUN_GREET( fun ) void fun( )
#define DECLARE_MPROG_FUN_GIVE( fun ) void fun( )
#define DECLARE_MPROG_FUN_FIGHT( fun ) void fun( )
#define DECLARE_MPROG_FUN_DEATH( fun ) bool fun( )
#define DECLARE_MPROG_FUN_AREA( fun ) void fun( )
#define DECLARE_MPROG_FUN_SPEECH( fun ) void fun( )

#define DECLARE_OPROG_FUN_WEAR(fun) void fun()
#define DECLARE_OPROG_FUN_REMOVE(fun) void fun()
#define DECLARE_OPROG_FUN_GET(fun) void fun()
#define DECLARE_OPROG_FUN_DROP(fun) void fun()
#define DECLARE_OPROG_FUN_SAC(fun) bool fun()
#define DECLARE_OPROG_FUN_ENTRY(fun) void fun()
#define DECLARE_OPROG_FUN_GIVE(fun) void fun()
#define DECLARE_OPROG_FUN_GREET(fun) void fun()
#define DECLARE_OPROG_FUN_FIGHT(fun) void fun()
#define DECLARE_OPROG_FUN_DEATH(fun) bool fun()
#define DECLARE_OPROG_FUN_SPEECH(fun) void fun()
#define DECLARE_OPROG_FUN_AREA(fun) void fun()
#else
#define args( list )			list
#define DECLARE_DO_FUN( fun )		DO_FUN	  fun
#define DECLARE_SPEC_FUN( fun ) 	SPEC_FUN  fun
#define DECLARE_SPELL_FUN( fun )	SPELL_FUN fun

#define DECLARE_MPROG_FUN_BRIBE( fun )	    MPROG_FUN_BRIBE fun
#define DECLARE_MPROG_FUN_ENTRY( fun )	    MPROG_FUN_ENTRY fun
#define DECLARE_MPROG_FUN_GREET( fun )	    MPROG_FUN_GREET fun
#define DECLARE_MPROG_FUN_GIVE( fun )	    MPROG_FUN_GIVE fun
#define DECLARE_MPROG_FUN_FIGHT( fun )	    MPROG_FUN_FIGHT fun
#define DECLARE_MPROG_FUN_DEATH( fun )	    MPROG_FUN_DEATH fun
#define DECLARE_MPROG_FUN_AREA( fun )	    MPROG_FUN_AREA fun
#define DECLARE_MPROG_FUN_SPEECH( fun )     MPROG_FUN_SPEECH fun

#define DECLARE_OPROG_FUN_WEAR(fun)	OPROG_FUN_WEAR fun
#define DECLARE_OPROG_FUN_REMOVE(fun)	OPROG_FUN_REMOVE fun
#define DECLARE_OPROG_FUN_GET(fun)	OPROG_FUN_GET fun
#define DECLARE_OPROG_FUN_DROP(fun)	OPROG_FUN_DROP fun
#define DECLARE_OPROG_FUN_SAC(fun)	OPROG_FUN_SAC fun
#define DECLARE_OPROG_FUN_ENTRY(fun)	OPROG_FUN_ENTRY fun
#define DECLARE_OPROG_FUN_GIVE(fun)	OPROG_FUN_GIVE fun
#define DECLARE_OPROG_FUN_GREET(fun)	OPROG_FUN_GREET fun
#define DECLARE_OPROG_FUN_FIGHT(fun)	OPROG_FUN_FIGHT fun
#define DECLARE_OPROG_FUN_DEATH(fun)	OPROG_FUN_DEATH fun
#define DECLARE_OPROG_FUN_SPEECH(fun)	OPROG_FUN_SPEECH fun
#define DECLARE_OPROG_FUN_AREA(fun)	OPROG_FUN_AREA fun
#endif


/* system calls */
int unlink();
int system();



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

#if	defined(_AIX)
#if	!defined(const)
#define const
#endif
typedef int				sh_int;
typedef int				bool;
#else
typedef short	int			sh_int;
typedef unsigned char			bool;
#endif



/*
 * Structure types.
 */
typedef struct	affect_data		AFFECT_DATA;
typedef struct	area_data		AREA_DATA;
typedef struct	ban_data		BAN_DATA;
typedef struct	buf_type		BUFFER;
typedef struct	char_data		CHAR_DATA;
typedef struct	descriptor_data 	DESCRIPTOR_DATA;
typedef struct	exit_data		EXIT_DATA;
typedef struct	extra_descr_data	EXTRA_DESCR_DATA;
typedef struct	help_data		HELP_DATA;
typedef struct	kill_data		KILL_DATA;
typedef struct	mem_data		MEM_DATA;
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
typedef struct	oprog_data		OPROG_DATA;
typedef struct	mprog_data		MPROG_DATA;
typedef struct	room_history_data	ROOM_HISTORY_DATA;
typedef struct	auction_data		AUCTION_DATA;


/*
 * Function types.
 */
typedef void DO_FUN	(CHAR_DATA *ch, char *argument);
typedef bool SPEC_FUN	args( ( CHAR_DATA *ch ) );
typedef void SPELL_FUN	args( ( int sn, int level, CHAR_DATA *ch, void *vo,
				int target ) );
typedef void MPROG_FUN_BRIBE args( ( CHAR_DATA *mob, CHAR_DATA *ch, int amount ) );
typedef void MPROG_FUN_ENTRY args( ( CHAR_DATA *mob ) );
typedef void MPROG_FUN_GREET args( ( CHAR_DATA *mob, CHAR_DATA *ch ) );
typedef void MPROG_FUN_GIVE args(( CHAR_DATA *mob, CHAR_DATA *ch, OBJ_DATA *obj ));
typedef void MPROG_FUN_FIGHT args( ( CHAR_DATA *mob, CHAR_DATA *victim ));
typedef bool MPROG_FUN_DEATH args( ( CHAR_DATA *mob ) );
typedef void MPROG_FUN_AREA args( ( CHAR_DATA *mob ) );
typedef void MPROG_FUN_SPEECH args((CHAR_DATA *mob, CHAR_DATA *ch, char *speech));

typedef void OPROG_FUN_WEAR args((OBJ_DATA *obj, CHAR_DATA *ch));
typedef void OPROG_FUN_REMOVE args((OBJ_DATA *obj, CHAR_DATA *ch));
typedef void OPROG_FUN_GET args((OBJ_DATA *obj, CHAR_DATA *ch));
typedef void OPROG_FUN_DROP args((OBJ_DATA *obj, CHAR_DATA *ch));
typedef bool OPROG_FUN_SAC args((OBJ_DATA *obj, CHAR_DATA *ch));
typedef void OPROG_FUN_ENTRY args((OBJ_DATA *obj));
typedef void OPROG_FUN_GIVE args((OBJ_DATA *obj, CHAR_DATA *from, CHAR_DATA *to));
typedef void OPROG_FUN_GREET args((OBJ_DATA *obj, CHAR_DATA *ch));
typedef void OPROG_FUN_FIGHT args((OBJ_DATA *obj, CHAR_DATA *ch));
typedef bool OPROG_FUN_DEATH args((OBJ_DATA *obj, CHAR_DATA *ch));
typedef void OPROG_FUN_SPEECH args((OBJ_DATA *obj, CHAR_DATA *ch, char *speech));
typedef void OPROG_FUN_AREA args((OBJ_DATA *obj));

/*
 *  COMMAND extra bits..
 */
#define CMD_KEEP_HIDE		1
#define CMD_GHOST		2


/*
 * String and memory management parameters.
 */
#define MAX_KEY_HASH		 1024
#define MAX_STRING_LENGTH	 4608
#define MAX_INPUT_LENGTH	  256
#define PAGELEN 		   22

#define MPROG_BRIBE		  (A)
#define MPROG_ENTRY		  (B)
#define MPROG_GREET		  (C)
#define MPROG_GIVE		  (D)
#define MPROG_FIGHT		  (E)
#define MPROG_DEATH		  (F)
#define MPROG_AREA		  (G)
#define MPROG_SPEECH		  (H)

#define OPROG_WEAR		  (A)
#define OPROG_REMOVE		  (B)
#define OPROG_DROP		  (C)
#define OPROG_SAC		  (D)
#define OPROG_GIVE		  (E)
#define OPROG_GREET		  (F)
#define OPROG_FIGHT		  (G)
#define OPROG_DEATH		  (H)
#define OPROG_SPEECH		  (I)
#define OPROG_ENTRY		  (J)
#define OPROG_GET		  (K)
#define OPROG_AREA		  (L)

/*
 * Game parameters.
 */
#define MAX_SOCIALS		  256
#define MAX_SKILL		  378
#define MAX_ALIAS		   20
#define MAX_CLASS		   13
#define MAX_PC_RACE		   19	/* 18 */
#define MAX_CLAN		    9
#define MAX_RELIGION		   18
#define MAX_LEVEL		   100
#define LEVEL_HERO		   (MAX_LEVEL - 9)
#define LEVEL_IMMORTAL		   (MAX_LEVEL - 8)


#undef ANATOLIA_MACHINE

#if defined(ANATOLIA_MACHINE)
#define PULSE_PER_SCD		    6  /* 6 for comm.c */
#define PULSE_PER_SECOND	    4  /* for update.c */
#define PULSE_VIOLENCE		  ( 2 *  PULSE_PER_SECOND)

#else
#define PULSE_PER_SECOND	    4
#define PULSE_PER_SCD		    4
#define PULSE_VIOLENCE		  ( 3 * PULSE_PER_SECOND)

#endif

#define PULSE_MOBILE		  ( 4 * PULSE_PER_SECOND)
#define PULSE_WATER_FLOAT	  ( 4 * PULSE_PER_SECOND)
#define PULSE_MUSIC		  ( 6 * PULSE_PER_SECOND)
#define PULSE_TRACK		  ( 6 * PULSE_PER_SECOND)
#define PULSE_TICK		  (50 * PULSE_PER_SECOND) /* 36 saniye */

/* room_affect_update (not room_update) */
#define PULSE_RAFFECT		  ( 3 * PULSE_MOBILE)
#define PULSE_AREA		  (110 * PULSE_PER_SECOND) /* 97 saniye */
#define FIGHT_DELAY_TIME	  (20 * PULSE_PER_SECOND)
#define PULSE_AUCTION		  (20 * PULSE_PER_SECOND) /* 10 seconds */

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


#define CLAN_NONE	0
#define CLAN_RULER	1
#define CLAN_INVADER	2
#define CLAN_CHAOS	3
#define CLAN_SHALAFI	4
#define CLAN_BATTLE	5
#define CLAN_KNIGHT	6
#define CLAN_LIONS	7
#define CLAN_HUNTER	8

/*
 * Cabal structure
 */
struct clan_type
{
  char *long_name;
  char *short_name;
  int obj_vnum;
  int room_vnum;
  OBJ_DATA *obj_ptr;
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
  char *leader;
  char *name;
  int vnum;
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
    bool	valid;
    sh_int	ban_flags;
    sh_int	level;
    char *	name;
};

struct buf_type
{
    BUFFER *	next;
    bool	valid;
    sh_int	state;	/* error state of the buffer */
    sh_int	size;	/* size in k */
    char *	string; /* buffer's string */
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
    CHAR_DATA * 	character;
    CHAR_DATA * 	original;
    bool		valid;
    char *		host;
    sh_int		descriptor;
    sh_int		connected;
    int 		wait_for_se;
    bool		fcommand;
    char		inbuf		[4 * MAX_INPUT_LENGTH];
    char		incomm		[MAX_INPUT_LENGTH];
    char		inlast		[MAX_INPUT_LENGTH];
    int 		repeat;
    char *		outbuf;
    int 		outsize;
    int 		outtop;
    char *		showstr_head;
    char *		showstr_point;
    struct codepage*	codepage;
};



/*
 * Attribute bonus structures.
 */
struct	str_app_type
{
    sh_int	tohit;
    sh_int	todam;
    sh_int	carry;
    sh_int	wield;
};

struct	int_app_type
{
    sh_int	learn;
};

struct	wis_app_type
{
    sh_int	practice;
};

struct	dex_app_type
{
    sh_int	defensive;
};

struct	con_app_type
{
    sh_int	hitp;
    sh_int	shock;
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
    HELP_DATA * next;
    sh_int	level;
    char *	keyword;
    char *	text;
};



/*
 * Shop types.
 */
#define MAX_TRADE	 5

struct	shop_data
{
    SHOP_DATA * next;			/* Next shop in list		*/
    sh_int	keeper; 		/* Vnum of shop keeper mob	*/
    sh_int	buy_type [MAX_TRADE];	/* Item types shop will buy	*/
    sh_int	profit_buy;		/* Cost multiplier for buying	*/
    sh_int	profit_sell;		/* Cost multiplier for selling	*/
    sh_int	open_hour;		/* First opening hour		*/
    sh_int	close_hour;		/* First closing hour		*/
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
    sh_int	attr_prime;		/* Prime attribute		*/
    sh_int	weapon; 		/* First weapon 		*/
    sh_int	guild[MAX_GUILD];	/* Vnum of guild rooms		*/
    sh_int	skill_adept;		/* Maximum skill level		*/
    sh_int	thac0_00;		/* Thac0 for level  0		*/
    sh_int	thac0_32;		/* Thac0 for level 32		*/
    sh_int	hp_rate;		/* hp rate gained on leveling	*/
    sh_int	mana_rate;		/* mana rate gained on leveling */
    bool	fMana;			/* Class gains mana on level	*/
    int 	points; 		/* Cost in exp of class 	*/
    sh_int	stats[MAX_STATS];	/* Stat modifiers		*/
    sh_int	align;			/* Alignment			*/
};


#define CLASS_WITCH		0
#define CLASS_CLERIC		1
#define CLASS_THIEF		2
#define CLASS_WARRIOR		3
#define CLASS_PALADIN		4
#define CLASS_ANTI_PALADIN	5
#define CLASS_NINJA		6
#define CLASS_RANGER		7
#define CLASS_WARLOCK		8
#define CLASS_SAMURAI		9
#define CLASS_VAMPIRE		10
#define CLASS_NECROMANCER	11
#define CLASS_DUMMY		12

#define LANG_COMMON	     0
#define LANG_HUMAN	     1
#define LANG_ELVISH	     2
#define LANG_DWARVISH	     3
#define LANG_GNOMISH	     4
#define LANG_GIANT	     5
#define LANG_TROLLISH	     6
#define LANG_CAT	     7
#define MAX_LANGUAGE	     8

struct item_type
{
    int 	type;
    char *	name;
};

struct weapon_type
{
    char *	name;
    sh_int	vnum;
    sh_int	type;
    sh_int	*gsn;
};

struct wiznet_type
{
    char *	name;
    long	flag;
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
    long	det;			/* det bits for the race */
    long	act;			/* act bits for the race */
    long	aff;			/* aff bits for the race */
    long	off;			/* off bits for the race */
    long	imm;			/* imm bits for the race */
    long	res;			/* res bits for the race */
    long	vuln;			/* vuln bits for the race */
    long	form;			/* default form flag for the race */
    long	parts;			/* default parts for the race */
};


struct pc_race_type  /* additional data for pc races */
{
    char *	name;			/* MUST be in race_type */
    char	who_name[6];
    int 	points; 		/* cost in exp of the race */
    sh_int	class_mult[MAX_CLASS];	/* exp multiplier for class, * 100 */
    char *	skills[5];		/* bonus skills for the race */
    sh_int	stats[MAX_STATS];	/* starting stats	*/
    sh_int	max_stats[MAX_STATS];	/* maximum stats	*/
    sh_int	size;			/* aff bits for the race*/
    int 	hp_bonus;		/* Initial hp bonus	*/
    int 	mana_bonus;		/* Initial mana bonus	*/
    int 	prac_bonus;		/* Initial practice bonus */
    sh_int	align;			/* Alignment		*/
    sh_int	language;		/* language		*/
};

struct spec_type
{
    char *	name;			/* special function name */
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
    NOTE_DATA * next;
    bool	valid;
    sh_int	type;
    char *	sender;
    char *	date;
    char *	to_list;
    char *	subject;
    char *	text;
    time_t	date_stamp;
};



/*
 * An affect.
 */
struct	affect_data
{
    AFFECT_DATA *	next;
    bool		valid;
    sh_int		where;
    sh_int		type;
    sh_int		level;
    sh_int		duration;
    sh_int		location;
    sh_int		modifier;
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
    sh_int		number;
    sh_int		killed;
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



/* group number for mobs */
#define GROUP_NONE		0
#define GROUP_WEAPONSMASTER	1
#define GROUP_ATTACK		2
#define GROUP_BEGUILING 	3
#define GROUP_BENEDICTIONS	4
#define GROUP_COMBAT		5
#define GROUP_CREATION		6
#define GROUP_CURATIVE		7
#define GROUP_DETECTION 	8
#define GROUP_DRACONIAN 	9
#define GROUP_ENCHANTMENT	10
#define GROUP_ENHANCEMENT	11
#define GROUP_HARMFUL		12
#define GROUP_HEALING		13
#define GROUP_ILLUSION		14
#define GROUP_MALADICTIONS	15
#define GROUP_PROTECTIVE	16
#define GROUP_TRANSPORTATION	17
#define GROUP_WEATHER		18
#define GROUP_FIGHTMASTER	19
#define GROUP_SUDDENDEATH	20
#define GROUP_MEDITATION	21
#define GROUP_CLAN		22
#define GROUP_DEFENSIVE 	23
#define GROUP_WIZARD		24

/* group bits for mobs */
#define GFLAG_NONE		0
#define GFLAG_WEAPONSMASTER	(A)
#define GFLAG_ATTACK		(B)
#define GFLAG_BEGUILING 	(C)
#define GFLAG_BENEDICTIONS	(D)
#define GFLAG_COMBAT		(E)
#define GFLAG_CREATION		(F)
#define GFLAG_CURATIVE		(G)
#define GFLAG_DETECTION 	(H)
#define GFLAG_DRACONIAN 	(I)
#define GFLAG_ENCHANTMENT	(J)
#define GFLAG_ENHANCEMENT	(K)
#define GFLAG_HARMFUL		(L)
#define GFLAG_HEALING		(M)
#define GFLAG_ILLUSION		(N)
#define GFLAG_MALADICTIONS	(O)
#define GFLAG_PROTECTIVE	(P)
#define GFLAG_TRANSPORTATION	(Q)
#define GFLAG_WEATHER		(R)
#define GFLAG_FIGHTMASTER	(S)
#define GFLAG_SUDDENDEATH	(T)
#define GFLAG_MEDITATION	(U)
#define GFLAG_CLAN		(V)
#define GFLAG_DEFENSIVE 	(W)
#define GFLAG_WIZARD		(X)

/*
 * AREA FLAGS
 */
#define AREA_HOMETOWN		(A)


/*
 * ACT bits for mobs.  *ACT*
 * Used in #MOBILES.
 */
#define ACT_IS_NPC		(A)		/* Auto set for mobs	*/
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
#define ACT_INDOORS		(Y)
#define ACT_RIDEABLE		(Z)
#define ACT_IS_HEALER		(aa)
#define ACT_GAIN		(bb)
#define ACT_UPDATE_ALWAYS	(cc)
#define ACT_IS_CHANGER		(dd)
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
#define OFF_KICK_DIRT		(J)
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
#define AFF_ROOM_SHOCKING		(A)
#define AFF_ROOM_L_SHIELD		(B)
#define AFF_ROOM_THIEF_TRAP		(C)
#define AFF_ROOM_RANDOMIZER		(D)
#define AFF_ROOM_ESPIRIT		(E)
#define AFF_ROOM_CURSE			(K)
#define AFF_ROOM_POISON 		(M)
#define AFF_ROOM_SLEEP			(R)
#define AFF_ROOM_PLAGUE 		(X)
#define AFF_ROOM_SLOW			(dd)

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
#define SEX_NEUTRAL		      0
#define SEX_MALE		      1
#define SEX_FEMALE		      2
#define SEX_MAX			SEX_FEMALE

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
#define OBJ_VNUM_ROSE		   1001
#define OBJ_VNUM_PIT		   3010

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

#define OBJ_VNUM_DEPUTY_BADGE	     70
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
#define APPLY_NONE		      0
#define APPLY_STR		      1
#define APPLY_DEX		      2
#define APPLY_INT		      3
#define APPLY_WIS		      4
#define APPLY_CON		      5
#define APPLY_CHA		      6
#define APPLY_CLASS		      7
#define APPLY_LEVEL		      8
#define APPLY_AGE		      9
#define APPLY_HEIGHT		     10
#define APPLY_WEIGHT		     11
#define APPLY_MANA		     12
#define APPLY_HIT		     13
#define APPLY_MOVE		     14
#define APPLY_GOLD		     15
#define APPLY_EXP		     16
#define APPLY_AC		     17
#define APPLY_HITROLL		     18
#define APPLY_DAMROLL		     19
#define APPLY_SAVES		     20
#define APPLY_SAVING_PARA	     20
#define APPLY_SAVING_ROD	     21
#define APPLY_SAVING_PETRI	     22
#define APPLY_SAVING_BREATH	     23
#define APPLY_SAVING_SPELL	     24
#define APPLY_SPELL_AFFECT	     25
#define APPLY_SIZE		     26

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
#define ROOM_NO_MOB		(C)
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
#define PLR_IS_NPC		(A)		/* Don't EVER set.	*/
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
#define PLR_NO_TITLE		(K)
/* RT personal flags */
#define PLR_NO_EXP		(L)
#define PLR_CHANGED_AFF 	(M)
#define PLR_HOLYLIGHT		(N)
#define PLR_CANLOOT		(P)
#define PLR_NOSUMMON		(Q)
#define PLR_NOFOLLOW		(R)
#define PLR_CANINDUCT		(S)
#define PLR_GHOST		(T)

/* penalty flags */
#define PLR_PERMIT		(U)
#define PLR_LOG 		(W)
#define PLR_DENY		(X)
#define PLR_FREEZE		(Y)
/* #define PLR_QUESTOR		(bb) */
#define PLR_VAMPIRE		(cc)
#define PLR_HARA_KIRI		(dd)
#define PLR_BLINK_ON		(ee)


/* quest done by chronos */
#define IS_ON_QUEST(ch)	(ch->pcdata->questtime > 0)
#define IS_VAMPIRE(ch)	(!IS_NPC(ch) && IS_SET((ch)->act , PLR_VAMPIRE))
#define IS_HARA_KIRI(ch) (IS_SET((ch)->act , PLR_HARA_KIRI))
#define CANT_CHANGE_TITLE(ch) (IS_SET(ch->act , PLR_NO_TITLE))
#define IS_BLINK_ON(ch) (IS_SET((ch)->act , PLR_BLINK_ON))
#define CANT_GAIN_EXP(ch) (IS_SET((ch)->act , PLR_NO_EXP))

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
 * language staff
 */

struct language_type
{
 char *name;
 sh_int vnum;
};

struct translation_type
{
 char *common;
 char *language;
};

/*
 * auction data
 */

struct	auction_data
{
    OBJ_DATA  * item;	/* a pointer to the item */
    CHAR_DATA * seller; /* a pointer to the seller - which may NOT quit */
    CHAR_DATA * buyer;	/* a pointer to the buyer - which may NOT quit */
    int 	bet;	/* last bet - or 0 if noone has bet anything */
    sh_int	going;	/* 1,2, sold */
    sh_int	pulse;	/* how many pulses (.25 sec) until another call-out ? */
};

/*
 * Prototype for a mob.
 * This is the in-memory version of #MOBILES.
 */
struct	mob_index_data
{
    MOB_INDEX_DATA *	next;
    SPEC_FUN *		spec_fun;
    MPROG_DATA *	mprogs;
    int 		progtypes;
    SHOP_DATA * 	pShop;
    sh_int		vnum;
    sh_int		group;
    bool		new_format;
    sh_int		count;
    sh_int		killed;
    char *		player_name;
    char *		short_descr;
    char *		long_descr;
    char *		description;
    long		act;
    long		affected_by;
    long		detection;
    sh_int		alignment;
    sh_int		level;
    sh_int		hitroll;
    int 		hit[3];
    int 		mana[3];
    sh_int		damage[3];
    sh_int		ac[4];
    sh_int		dam_type;
    long		off_flags;
    long		imm_flags;
    long		res_flags;
    long		vuln_flags;
    sh_int		start_pos;
    sh_int		default_pos;
    sh_int		sex;
    sh_int		race;
    long		wealth;
    long		form;
    long		parts;
    sh_int		size;
    char *		material;
    long		practicer;
};




/* memory settings */
#define MEM_CUSTOMER	A
#define MEM_SELLER	B
#define MEM_HOSTILE	C
#define MEM_AFRAID	D

/* memory for mobs */
struct mem_data
{
    MEM_DATA	*next;
    bool	valid;
    int 	id;
    int 	reaction;
    time_t	when;
};

struct mprog_data
{
  MPROG_FUN_BRIBE * bribe_prog;
  MPROG_FUN_ENTRY * entry_prog;
  MPROG_FUN_GIVE *  give_prog;
  MPROG_FUN_GREET * greet_prog;
  MPROG_FUN_FIGHT * fight_prog;
  MPROG_FUN_DEATH * death_prog;
  MPROG_FUN_AREA * area_prog;
  MPROG_FUN_SPEECH * speech_prog;

};

struct oprog_data
{
  OPROG_FUN_WEAR *wear_prog;
  OPROG_FUN_REMOVE *remove_prog;
  OPROG_FUN_GET *get_prog;
  OPROG_FUN_DROP *drop_prog;
  OPROG_FUN_SAC *sac_prog;
  OPROG_FUN_ENTRY *entry_prog;
  OPROG_FUN_GIVE *give_prog;
  OPROG_FUN_GREET *greet_prog;
  OPROG_FUN_FIGHT *fight_prog;
  OPROG_FUN_DEATH *death_prog;
  OPROG_FUN_SPEECH *speech_prog;
  OPROG_FUN_AREA *area_prog;
};


/*
 * One character (PC or NPC). *CHAR_DATA*
 */
struct	char_data
{
    CHAR_DATA * 	next;
    CHAR_DATA * 	next_in_room;
    CHAR_DATA * 	master;
    CHAR_DATA * 	leader;
    CHAR_DATA * 	fighting;
    CHAR_DATA * 	reply;
    CHAR_DATA * 	last_fought;
    time_t		last_fight_time;
    time_t		last_death_time;
    CHAR_DATA * 	pet;
    CHAR_DATA * 	doppel;
    CHAR_DATA * 	guarding;
    CHAR_DATA * 	guarded_by;
    MEM_DATA *		memory;
    SPEC_FUN *		spec_fun;
    MOB_INDEX_DATA *	pIndexData;
    DESCRIPTOR_DATA *	desc;
    AFFECT_DATA *	affected;
    NOTE_DATA * 	pnote;
    OBJ_DATA *		carrying;
    OBJ_DATA *		on;
    ROOM_INDEX_DATA *	in_room;
    ROOM_INDEX_DATA *	was_in_room;
    AREA_DATA * 	zone;
    PC_DATA *		pcdata;
    bool		valid;
    char *		name;
    long		id;
    sh_int		version;
    char *		short_descr;
    char *		long_descr;
    char *		description;
    char *		prompt;
    char *		prefix;
    sh_int		group;
    sh_int		sex;
    sh_int		class;
    sh_int		race;
    sh_int		clan;
    sh_int		hometown;
    sh_int		ethos;
    sh_int		level;
    sh_int		trust;
    int 		played;
    int 		lines;	/* for the pager */
    time_t		logon;
    sh_int		timer;
    sh_int		wait;
    sh_int		daze;
    sh_int		hit;
    int 		max_hit;
    int 		mana;
    int 		max_mana;
    int 		move;
    int 		max_move;
    long		gold;
    long		silver;
    int 		exp;
    long		act;
    long		comm;	/* RT added to pad the vector */
    long		wiznet; /* wiz stuff */
    long		imm_flags;
    long		res_flags;
    long		vuln_flags;
    sh_int		invis_level;
    sh_int		incog_level;
    long		affected_by;
    long		detection;
    sh_int		position;
    sh_int		practice;
    sh_int		train;
    sh_int		carry_weight;
    sh_int		carry_number;
    sh_int		saving_throw;
    sh_int		alignment;
    sh_int		hitroll;
    sh_int		damroll;
    sh_int		armor[4];
    sh_int		wimpy;
    /* stats */
    sh_int		perm_stat[MAX_STATS];
    sh_int		mod_stat[MAX_STATS];
    /* parts stuff */
    long		form;
    long		parts;
    sh_int		size;
    char	*	material;
    /* mobile stuff */
    long		off_flags;
    sh_int		damage[3];
    sh_int		dam_type;
    sh_int		start_pos;
    sh_int		default_pos;
    int 		status;
    int 		progtypes;
    bool		extracted;
    char	*	in_mind;
    int 		quest;
    int 		religion;
    CHAR_DATA	*	hunting;	/* hunt data */
    int 		endur;
    bool		riding; /* mount data */
    CHAR_DATA	*	mount;
    sh_int		language;
    sh_int		i_lang; /* interface language */
    CHAR_DATA *		hunter;	/* who quested to slay */
};



/*
 * Data which only PC's have.
 */
struct	pc_data
{
    PC_DATA *		next;
    BUFFER *		buffer;
    bool		valid;
    char *		pwd;
    char *		bamfin;
    char *		bamfout;
    char *		title;
    time_t		last_note;
    time_t		last_idea;
    time_t		last_penalty;
    time_t		last_news;
    time_t		last_changes;
    int 		perm_hit;
    int 		perm_mana;
    int 		perm_move;
    sh_int		true_sex;
    int 		last_level;
    sh_int		condition	[MAX_COND];
    sh_int		learned 	[MAX_SKILL];
    sh_int		points;
    bool		confirm_delete;
    char *		alias[MAX_ALIAS];
    char *		alias_sub[MAX_ALIAS];
    long		bank_s;
    long		bank_g;
    int 		death;
    int 		played;
    int 		anti_killed;
    int 		has_killed;
    sh_int		questgiver;	/* quest */
    int 		questpoints;	/* quest */
    sh_int		questtime;	/* quest */
    sh_int		questobj;	/* quest */
    sh_int		questmob;	/* quest */
    ROOM_INDEX_DATA *	questroom;	/* quest */
    sh_int		race;	/* orginal race for polymorph */
    sh_int		adr_stops_shown;
    int			pc_killed;	/* how many PC's killed by character */
};


/*
 * Liquids.
 */
#define LIQ_WATER	 0

struct	liq_type
{
    char *	liq_name;
    char *	liq_color;
    sh_int	liq_affect[5];
};



/*
 * Extra description data for a room or object.
 */
struct	extra_descr_data
{
    EXTRA_DESCR_DATA *next;	/* Next in list 		    */
    bool valid;
    char *keyword;		/* Keyword in look/examine	    */
    char *description;		/* What to see			    */
};


/*
 * Prototype for an object.  *OID*
 */
struct	obj_index_data
{
    OBJ_INDEX_DATA *	next;
    EXTRA_DESCR_DATA *	extra_descr;
    AFFECT_DATA *	affected;
    bool		new_format;
    char *		name;
    char *		short_descr;
    char *		description;
    sh_int		vnum;
    sh_int		reset_num;
    char *		material;
    sh_int		item_type;
    int 		extra_flags;
    int 		wear_flags;
    sh_int		level;
    sh_int		condition;
    sh_int		count;
    sh_int		weight;
    int 		cost;
    int 		value[5];
    int 		progtypes;
    int 		limit;
    OPROG_DATA	       *oprogs;
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
    CHAR_DATA * 	carried_by;
    EXTRA_DESCR_DATA *	extra_descr;
    AFFECT_DATA *	affected;
    OBJ_INDEX_DATA *	pIndexData;
    ROOM_INDEX_DATA *	in_room;
    bool		valid;
    bool		enchanted;
    char *		owner;
    char *		name;
    char *		short_descr;
    char *		description;
    sh_int		item_type;
    int 		extra_flags;
    int 		wear_flags;
    sh_int		wear_loc;
    sh_int		weight;
    int 		cost;
    sh_int		level;
    sh_int		condition;
    char *		material;
    sh_int		timer;
    int 		value	[5];
    int 		progtypes;
    char		*from;
    int 		altar;
    int 		pit;
    bool		extracted;
    int 		water_float;
};



/*
 * Exit data.
 */
struct	exit_data
{
    union
    {
	ROOM_INDEX_DATA *	to_room;
	sh_int			vnum;
    } u1;
    sh_int		exit_info;
    sh_int		key;
    char *		keyword;
    char *		description;
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
    sh_int		arg1;
    sh_int		arg2;
    sh_int		arg3;
    sh_int		arg4;
};



/*
 * Area definition.
 */
struct	area_data
{
    AREA_DATA * 	next;
    RESET_DATA *	reset_first;
    RESET_DATA *	reset_last;
    char *		file_name;
    char *		name;
    char *		writer;
    char *		credits;
    sh_int		age;
    sh_int		nplayer;
    sh_int		low_range;
    sh_int		high_range;
    sh_int		min_vnum;
    sh_int		max_vnum;
    bool		empty;
    unsigned long	count;
    char *		resetmsg;
    long		area_flag;
};

struct room_history_data
{
  char *name;
  sh_int went;
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
    CHAR_DATA * 	people;
    OBJ_DATA *		contents;
    EXTRA_DESCR_DATA *	extra_descr;
    AREA_DATA * 	area;
    EXIT_DATA * 	exit	[6];
    EXIT_DATA * 	old_exit[6];
    char *		name;
    char *		description;
    char *		owner;
    sh_int		vnum;
    long		room_flags;
    sh_int		light;
    sh_int		sector_type;
    sh_int		heal_rate;
    sh_int		mana_rate;
    ROOM_HISTORY_DATA * history;
    AFFECT_DATA *	affected;
    long		affected_by;
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
    char *	name;			/* Name of skill		*/
    sh_int	skill_level[MAX_CLASS]; /* Level needed by class	*/
    sh_int	rating[MAX_CLASS];	/* How hard it is to learn	*/
    SPELL_FUN * spell_fun;		/* Spell pointer (for spells)	*/
    sh_int	target; 		/* Legal targets		*/
    sh_int	minimum_position;	/* Position for caster / user	*/
    sh_int *	pgsn;			/* Pointer to associated gsn	*/
    sh_int	slot;			/* Slot for #OBJECT loading	*/
    sh_int	min_mana;		/* Minimum mana used		*/
    sh_int	beats;			/* Waiting time after use	*/
    char *	noun_damage;		/* Damage message		*/
    char *	msg_off;		/* Wear off message		*/
    char *	msg_obj;		/* Wear off message for obects	*/
    sh_int	clan;			/* Cabal spells 		*/
    long	race;			/* Race spells			*/
    sh_int	align;			/* alignment of spells		*/
    sh_int	group;			/* skill group for practicing	*/
};


/*
 * These are skill_lookup return values for common skills and spells.
 */
extern	sh_int	gsn_backstab;
extern	sh_int	gsn_dodge;
extern	sh_int	gsn_envenom;
extern	sh_int	gsn_hide;
extern	sh_int	gsn_peek;
extern	sh_int	gsn_pick_lock;
extern	sh_int	gsn_sneak;
extern	sh_int	gsn_steal;

extern	sh_int	gsn_disarm;
extern	sh_int	gsn_enhanced_damage;
extern	sh_int	gsn_kick;
extern	sh_int	gsn_parry;
extern	sh_int	gsn_rescue;
extern	sh_int	gsn_second_attack;
extern	sh_int	gsn_third_attack;

extern	sh_int	gsn_blindness;
extern	sh_int	gsn_charm_person;
extern	sh_int	gsn_curse;
extern	sh_int	gsn_remove_curse;
extern	sh_int	gsn_invis;
extern	sh_int	gsn_mass_invis;
extern	sh_int	gsn_plague;
extern	sh_int	gsn_poison;
extern	sh_int	gsn_sleep;
extern	sh_int	gsn_fly;
extern	sh_int	gsn_death_protection;
extern	sh_int	gsn_sanctuary;

/* new gsns */
extern sh_int  gsn_axe;
extern sh_int  gsn_dagger;
extern sh_int  gsn_flail;
extern sh_int  gsn_mace;
extern sh_int  gsn_polearm;
extern sh_int  gsn_shield_block;
extern sh_int  gsn_spear;
extern sh_int  gsn_sword;
extern sh_int  gsn_whip;

extern sh_int  gsn_bash;
extern sh_int  gsn_berserk;
extern sh_int  gsn_dirt;
extern sh_int  gsn_hand_to_hand;
extern sh_int  gsn_trip;

extern sh_int  gsn_fast_healing;
extern sh_int  gsn_haggle;
extern sh_int  gsn_lore;
extern sh_int  gsn_meditation;

extern sh_int  gsn_scrolls;
extern sh_int  gsn_staves;
extern sh_int  gsn_wands;
extern sh_int  gsn_recall;
extern sh_int  gsn_detect_hidden;

extern	sh_int	gsn_fourth_attack;
extern sh_int  gsn_dual_backstab;
extern sh_int  gsn_cleave;
extern sh_int  gsn_counter;
extern sh_int  gsn_ambush;
extern sh_int  gsn_camouflage;
extern sh_int  gsn_circle;
extern sh_int  gsn_nerve;
extern sh_int  gsn_endure;
extern sh_int  gsn_quiet_movement;
extern sh_int  gsn_herbs;
extern sh_int  gsn_tame;
extern sh_int  gsn_butcher;
extern sh_int  gsn_assassinate;
extern sh_int  gsn_wanted;
extern sh_int  gsn_caltraps;
extern sh_int  gsn_throw;
extern sh_int  gsn_strangle;
extern sh_int  gsn_blackjack;
extern sh_int  gsn_bloodthirst;
extern sh_int  gsn_spellbane;
extern sh_int  gsn_resistance;
extern sh_int  gsn_deathblow;
extern sh_int  gsn_doppelganger;
extern sh_int  gsn_fade;
extern sh_int  gsn_garble;
extern sh_int  gsn_confuse;
extern sh_int  gsn_mirror;
extern sh_int  gsn_track;
extern sh_int  gsn_vanish;
extern sh_int  gsn_chaos_blade;
extern sh_int  gsn_terangreal;
extern sh_int  gsn_dispel_evil;
extern sh_int  gsn_dispel_good;
extern sh_int  gsn_wrath;
extern sh_int  gsn_stalker;
extern sh_int  gsn_tesseract;
extern sh_int  gsn_randomizer;
extern sh_int  gsn_trophy;
extern sh_int  gsn_truesight;
extern sh_int  gsn_brew;
extern sh_int  gsn_flamestrike;
extern sh_int  gsn_shadowlife;
extern sh_int  gsn_ruler_badge;
extern sh_int  gsn_remove_badge;
extern sh_int  gsn_dragon_strength;
extern sh_int  gsn_dragon_breath;
extern sh_int  gsn_warcry;
extern sh_int  gsn_entangle;
extern sh_int  gsn_dragonsword;
extern sh_int  gsn_knight;
extern sh_int  gsn_guard;
extern sh_int  gsn_guard_call;
extern sh_int  gsn_love_potion;
extern sh_int  gsn_deafen;
extern sh_int  gsn_protective_shield;
extern sh_int  gsn_protection_heat_cold;
extern sh_int  gsn_trance;
extern sh_int  gsn_demon_summon;
extern sh_int  gsn_nightwalker;
extern sh_int  gsn_bear_call;
extern sh_int  gsn_squire;
extern sh_int  gsn_lightning_bolt;
extern sh_int  gsn_disperse;
extern sh_int  gsn_bless;
extern sh_int  gsn_weaken;
extern sh_int  gsn_haste;
extern sh_int  gsn_cure_critical;
extern sh_int  gsn_cure_serious;
extern sh_int  gsn_burning_hands;
extern sh_int  gsn_acid_blast;
extern sh_int  gsn_ray_of_truth;
extern sh_int  gsn_spell_craft;
extern sh_int  gsn_giant_strength;
extern sh_int  gsn_explode;
extern sh_int  gsn_acid_breath;
extern sh_int  gsn_fire_breath;
extern sh_int  gsn_gas_breath;
extern sh_int  gsn_frost_breath;
extern sh_int  gsn_lightning_breath;
extern sh_int  gsn_cure_light;
extern sh_int  gsn_magic_missile;
extern sh_int  gsn_demonfire;
extern sh_int  gsn_faerie_fire;
extern sh_int  gsn_shield;
extern sh_int  gsn_chill_touch;
extern sh_int  gsn_second_weapon;
extern sh_int  gsn_target;
extern sh_int  gsn_sand_storm;
extern sh_int  gsn_scream;
extern sh_int  gsn_enchant_sword;
extern sh_int  gsn_tiger_power;
extern sh_int  gsn_lion_call;
extern sh_int  gsn_hara_kiri;
extern sh_int  gsn_mouse;
extern sh_int  gsn_enhanced_armor;
extern sh_int  gsn_vampire;
extern sh_int  gsn_vampiric_bite;
extern sh_int  gsn_light_res;
extern sh_int  gsn_blink;
extern sh_int  gsn_hunt;
extern sh_int  gsn_path_find;
extern sh_int  gsn_critical;
extern sh_int  gsn_detect_sneak;
extern sh_int  gsn_mend;
extern sh_int  gsn_shielding;
extern sh_int  gsn_blind_fighting;
extern sh_int  gsn_riding;
extern sh_int  gsn_smithing;
extern sh_int  gsn_swimming;
extern sh_int  gsn_move_camf;
extern sh_int  gsn_protection_heat;
extern sh_int  gsn_protection_cold;
extern sh_int  gsn_teleport;
extern sh_int  gsn_witch_curse;
extern sh_int  gsn_terangreal;
extern sh_int  gsn_sebat;
extern sh_int  gsn_kassandra;
extern sh_int  gsn_matandra;
extern sh_int  gsn_armor_use;
extern sh_int  gsn_world_find;
extern sh_int  gsn_vampiric_touch;
extern sh_int  gsn_cure_poison;
extern sh_int  gsn_fire_shield;
extern sh_int  gsn_fear;
extern sh_int  gsn_settraps;
extern sh_int  gsn_mental_attack;
extern sh_int  gsn_secondary_attack;
extern sh_int  gsn_mortal_strike;
extern sh_int  gsn_shield_cleave;
extern sh_int  gsn_weapon_cleave;
extern sh_int  gsn_slow;
extern sh_int  gsn_imp_invis;
extern sh_int  gsn_camp;
extern sh_int  gsn_push;
extern sh_int  gsn_tail;
extern sh_int  gsn_power_stun;
extern sh_int  gsn_clan_recall;
extern sh_int  gsn_escape;
extern sh_int  gsn_lay_hands;
extern sh_int  gsn_grip;
extern sh_int  gsn_concentrate;
extern sh_int  gsn_master_sword;
extern sh_int  gsn_master_hand;
extern sh_int  gsn_fifth_attack;
extern sh_int  gsn_area_attack;
extern sh_int  gsn_reserved;
extern sh_int  gsn_bandage;
extern sh_int  gsn_web;
extern sh_int  gsn_bow;
extern sh_int  gsn_bash_door;
extern sh_int  gsn_katana;
extern sh_int  gsn_bluefire;
extern sh_int  gsn_crush;
extern sh_int  gsn_perception;
extern sh_int  gsn_control_animal;
extern sh_int  gsn_sanctify_lands;
extern sh_int  gsn_deadly_venom;
extern sh_int  gsn_cursed_lands;
extern sh_int  gsn_lethargic_mist;
extern sh_int  gsn_black_death;
extern sh_int  gsn_mysterious_dream;
extern sh_int  gsn_sense_life;
extern sh_int  gsn_arrow;
extern sh_int  gsn_lance;
extern sh_int  gsn_evil_spirit;
extern sh_int  gsn_make_arrow;
extern sh_int  gsn_green_arrow;
extern sh_int  gsn_red_arrow;
extern sh_int  gsn_white_arrow;
extern sh_int  gsn_blue_arrow;
extern sh_int  gsn_make_bow;
extern sh_int  gsn_blindness_dust;
extern sh_int  gsn_poison_smoke;
extern sh_int  gsn_mastering_spell;
extern sh_int  gsn_judge;

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
#define IS_WATER( var ) 	(((var)->sector_type == SECT_WATER_SWIM) || \
				 ((var)->sector_type == SECT_WATER_NOSWIM) )
#define PERCENT(cur, max)	(max==0?0:((cur)*100)/(max))

/*
 * Character macros.
 */
#define IS_NPC(ch)		(IS_SET((ch)->act, ACT_IS_NPC))
#define IS_IMMORTAL(ch) 	(get_trust(ch) >= LEVEL_IMMORTAL)
#define IS_HERO(ch)		(get_trust(ch) >= LEVEL_HERO)
#define IS_TRUSTED(ch,level)	(get_trust((ch)) >= (level))
#define IS_AFFECTED(ch, sn)	(IS_SET((ch)->affected_by, (sn)))
#define CAN_DETECT(ch, sn)	(IS_SET((ch)->detection, (sn)))

#define IS_PK(ch, vt)		(!IS_NPC((ch)) & !IS_NPC((vt)))

#define RACE(ch)		(ch->race)
#define ORG_RACE(ch)		(IS_NPC(ch) ? ch->pIndexData->race : ch->pcdata->race)

#define GET_AGE(ch)		((int) (17 + ((ch)->played \
				    + current_time - (ch)->logon )/72000))

#define IS_GOOD(ch)		(ch->alignment >= 350)
#define IS_EVIL(ch)		(ch->alignment <= -350)
#define IS_NEUTRAL(ch)		(!IS_GOOD(ch) && !IS_EVIL(ch))

#define IS_AWAKE(ch)		(ch->position > POS_SLEEPING)
#define GET_AC(ch,type) 	((ch)->armor[type]			    \
			+ ( IS_AWAKE(ch)			    \
			? dex_app[get_curr_stat(ch,STAT_DEX)].defensive : 0 ))
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

#define IS_GOLEM( ch )		(IS_NPC(ch) &&\
				(ch->pIndexData->vnum==MOB_VNUM_LESSER_GOLEM \
				 || ch->pIndexData->vnum==MOB_VNUM_STONE_GOLEM \
				 || ch->pIndexData->vnum==MOB_VNUM_IRON_GOLEM \
				 || ch->pIndexData->vnum==MOB_VNUM_ADAMANTITE_GOLEM ) )
/*
 * Object macros.
 */
#define CAN_WEAR(obj, part)	(IS_SET((obj)->wear_flags,  (part)))
#define IS_OBJ_STAT(obj, stat)	(IS_SET((obj)->extra_flags, (stat)))
#define IS_WEAPON_STAT(obj,stat)(IS_SET((obj)->value[4],(stat)))
#define WEIGHT_MULT(obj)	((obj)->item_type == ITEM_CONTAINER ? \
	(obj)->value[4] : 100)

/* skill defines */
#define SKILL_LEVEL_OK(ch,sn)	(ch->level >= skill_table[sn].skill_level[ch->class] )
#define SKILL_RACE_OK(ch,sn)	IS_SET(skill_table[sn].race,(1 << (ORG_RACE(ch) - 1) ))
#define SKILL_CLAN_OK(ch,sn)	(skill_table[sn].clan == CLAN_NONE || skill_table[sn].clan == ch->clan)
#define SKILL_ALIGN_OK(ch,sn)	(skill_table[sn].align == ALIGN_NONE || skill_table[sn].align == ch->alignment)

#define SKILL_OK(ch, sn)	(((SKILL_LEVEL_OK(ch, sn) && \
				   SKILL_RACE_OK(ch, sn)) || \
				  skill_is_native(ch, sn)) && \
				 SKILL_CLAN_OK(ch, sn) && \
				 SKILL_ALIGN_OK(ch, sn))

/*
 * Description macros.
 */
#define PERS(ch, looker)	( can_see( looker, (ch) ) ?	    \
				( IS_NPC(ch) ? (ch)->short_descr :((IS_VAMPIRE(ch) && !IS_IMMORTAL(looker))? "An ugly creature" : (ch)->name ) ) \
				: ( (ch)->level > (MAX_LEVEL-7) ? "an immortal" : "someone" ) )
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

extern	const	struct	language_type	language_table	   [MAX_LANGUAGE];
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
extern  const	char*			group_table	[];
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

extern		AUCTION_DATA	  *	auction;
extern		ROOM_INDEX_DATA   *	top_affected_room;

extern		char			bug_buf 	[];
extern		time_t			current_time;
extern		bool			fLogAll;
extern		FILE *			fpReserve;
extern		KILL_DATA		kill_table	[];
extern		char			log_buf 	[];
extern		TIME_INFO_DATA		time_info;
extern		WEATHER_DATA		weather_info;
extern		long			total_levels;
extern		int			reboot_counter;

/*
 * OS-dependent declarations.
 * These are all very standard library functions,
 *   but some systems have incomplete or non-ansi header files.
 */
#if	defined(_AIX)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(apollo)
int	atoi		args( ( const char *string ) );
void *	calloc		args( ( unsigned nelem, size_t size ) );
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(hpux)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(linux)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(BSD44)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(MIPS_OS)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(NeXT)
char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(sequent)
char *	crypt		args( ( const char *key, const char *salt ) );
int	fclose		args( ( FILE *stream ) );
int	fprintf 	args( ( FILE *stream, const char *format, ... ) );
int	fread		args( ( void *ptr, int size, int n, FILE *stream ) );
int	fseek		args( ( FILE *stream, long offset, int ptrname ) );
void	perror		args( ( const char *s ) );
int	ungetc		args( ( int c, FILE *stream ) );
#endif

#if	defined(ultrix)
char *	crypt		args( ( const char *key, const char *salt ) );
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

/* act_hera.c */
/* enter.c */
RID  *get_random_room	args ( (CHAR_DATA *ch) );
/* hunt.c */
void hunt_victim(CHAR_DATA *ch);

/* act_move.c */
void	move_char	args( ( CHAR_DATA *ch, int door, bool follow ) );
char *	find_way	args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *rstart, ROOM_INDEX_DATA *rend ) );

/* act_obj.c */
bool can_loot		args( (CHAR_DATA *ch, OBJ_DATA *obj) );
void get_obj		args( ( CHAR_DATA *ch, OBJ_DATA *obj,OBJ_DATA *container ) );
int floating_time	args( ( OBJ_DATA *obj ) );
bool may_float		args( ( OBJ_DATA *obj ) );
bool cant_float 	args( ( OBJ_DATA *obj ) );

/* ban.c */
bool	check_ban	args( ( char *site, int type) );

/* effect.c */
void	acid_effect	args( (void *vo, int level, int dam, int target) );
void	cold_effect	args( (void *vo, int level, int dam, int target) );
void	fire_effect	args( (void *vo, int level, int dam, int target) );
void	poison_effect	args( (void *vo, int level, int dam, int target) );
void	shock_effect	args( (void *vo, int level, int dam, int target) );
void	sand_effect	args( (void *vo, int level, int dam, int target) );
void	scream_effect	args( (void *vo, int level, int dam, int target) );


/* fight.c */
bool	is_safe 	args( (CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	is_safe_nomessage args( (CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	is_safe_spell	args( (CHAR_DATA *ch, CHAR_DATA *victim, bool area ) );
void	violence_update args( ( void ) );
void	multi_hit	args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
bool	damage		args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam,
				int dt, int class, bool show ) );
bool	damage_old	args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam,
				int dt, int class, bool show ) );
void	update_pos	args( ( CHAR_DATA *victim ) );
void	stop_fighting	args( ( CHAR_DATA *ch, bool fBoth ) );
bool	can_kill	args( ( CHAR_DATA *ch, CHAR_DATA *victim) );
void   gods_protect_msg args( ( CHAR_DATA *ch, CHAR_DATA *victim) );
CHAR_DATA *  check_guard     args((CHAR_DATA *ch, CHAR_DATA *mob));

/* handler.c */
AD	*affect_find args( (AFFECT_DATA *paf, int sn));
void	affect_check	args( (CHAR_DATA *ch, int where, int vector) );
int	count_users	args( (OBJ_DATA *obj) );
void	deduct_cost	args( (CHAR_DATA *ch, int cost) );
void	affect_enchant	args( (OBJ_DATA *obj) );
int	check_immune	args( (CHAR_DATA *ch, int dam_type) );
bool	check_material	args( (OBJ_DATA *obj, char *material ) );
bool	is_metal	args( (OBJ_DATA *obj ) );
bool	clan_ok	args( ( CHAR_DATA *ch, sh_int sn ) );
int	liq_lookup	args( ( const char *name) );
int	material_lookup args( ( const char *name) );
int	weapon_lookup	args( ( const char *name) );
int	weapon_type	args( ( const char *name) );
char	*weapon_name	args( ( int weapon_Type) );
int	item_lookup	args( ( const char *name) );
char	*item_name	args( ( int item_type) );
int	attack_lookup	args( ( const char *name) );
int	race_lookup	args( ( const char *name) );
long	wiznet_lookup	args( ( const char *name) );
int	class_lookup	args( ( const char *name) );
int	clan_lookup	args( ( const char *argument) );
bool	is_old_mob	args ( (CHAR_DATA *ch) );
int	get_skill	args( ( CHAR_DATA *ch, int sn ) );
int	get_weapon_sn	args( ( CHAR_DATA *ch ) );
int	get_second_sn	args( ( CHAR_DATA *ch ) );
int	get_weapon_skill args(( CHAR_DATA *ch, int sn ) );
int	get_age 	args( ( CHAR_DATA *ch ) );
void	reset_char	args( ( CHAR_DATA *ch )  );
int	get_trust	args( ( CHAR_DATA *ch ) );
int	get_curr_stat	args( ( CHAR_DATA *ch, int stat ) );
int	get_max_train	args( ( CHAR_DATA *ch, int stat ) );
int	get_max_train2	args( ( CHAR_DATA *ch, int stat ) );
int	can_carry_n	args( ( CHAR_DATA *ch ) );
int	can_carry_w	args( ( CHAR_DATA *ch ) );
bool	is_name 	args( ( char *str, char *namelist ) );
bool	is_name_imm	args( ( char *str, char *namelist ) );
void	affect_to_char	args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void	affect_to_obj	args( ( OBJ_DATA *obj, AFFECT_DATA *paf ) );
void	affect_remove	args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void	affect_remove_obj args( (OBJ_DATA *obj, AFFECT_DATA *paf ) );
void	affect_strip	args( ( CHAR_DATA *ch, int sn ) );
bool	is_affected	args( ( CHAR_DATA *ch, int sn ) );
void	affect_to_room	args( ( ROOM_INDEX_DATA *room, AFFECT_DATA *paf ) );
void	affect_remove_room	args( ( ROOM_INDEX_DATA *room, AFFECT_DATA *paf ) );
void	affect_strip_room	args( ( ROOM_INDEX_DATA *ch, int sn ) );
bool	is_affected_room	args( ( ROOM_INDEX_DATA *ch, int sn ) );
void	affect_join_room	args( ( ROOM_INDEX_DATA *ch, AFFECT_DATA *paf ) );
void	affect_join	args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void	char_from_room	args( ( CHAR_DATA *ch ) );
void	char_to_room	args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex ) );
void	obj_to_char	args( ( OBJ_DATA *obj, CHAR_DATA *ch ) );
void	obj_from_char	args( ( OBJ_DATA *obj ) );
int	apply_ac	args( ( OBJ_DATA *obj, int iWear, int type ) );
OD *	get_eq_char	args( ( CHAR_DATA *ch, int iWear ) );
void	equip_char	args( ( CHAR_DATA *ch, OBJ_DATA *obj, int iWear ) );
void	unequip_char	args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
int	count_obj_list	args( ( OBJ_INDEX_DATA *obj, OBJ_DATA *list ) );
void	obj_from_room	args( ( OBJ_DATA *obj ) );
void	obj_to_room	args( ( OBJ_DATA *obj, ROOM_INDEX_DATA *pRoomIndex ) );
void	obj_to_obj	args( ( OBJ_DATA *obj, OBJ_DATA *obj_to ) );
void	obj_from_obj	args( ( OBJ_DATA *obj ) );
void	extract_obj	args( ( OBJ_DATA *obj ) );
void	extract_obj_nocount	args( ( OBJ_DATA *obj ) );
void	extract_obj_1	args( ( OBJ_DATA *obj, bool count ) );
void	extract_char	args( ( CHAR_DATA *ch, bool fPull ) );
void	extract_char_nocount	args( ( CHAR_DATA *ch, bool fPull ) );
void	extract_char_org	args( ( CHAR_DATA *ch, bool fPull, bool Count ) );
CHAR_DATA *	get_char_room	args( ( CHAR_DATA *ch, char *argument ) );
CHAR_DATA *	get_char_room2	args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *room,char *argument, int *number ) );
CHAR_DATA *	get_char_world	args( ( CHAR_DATA *ch, char *argument ) );
CHAR_DATA *	get_char_area	args( ( CHAR_DATA *ch, char *argument ) );
OD *	get_obj_type	args( ( OBJ_INDEX_DATA *pObjIndexData ) );
OD *	get_obj_list	args( ( CHAR_DATA *ch, char *argument,
			    OBJ_DATA *list ) );
OD *	get_obj_carry	args( ( CHAR_DATA *ch, char *argument ) );
OD *	get_obj_wear	args( ( CHAR_DATA *ch, char *argument ) );
OD *	get_obj_here	args( ( CHAR_DATA *ch, char *argument ) );
OD *	get_obj_world	args( ( CHAR_DATA *ch, char *argument ) );
OD *	create_money	args( ( int gold, int silver ) );
int	get_obj_number	args( ( OBJ_DATA *obj ) );
int	get_obj_realnumber	args( ( OBJ_DATA *obj ) );
int	get_obj_weight	args( ( OBJ_DATA *obj ) );
int	get_true_weight args( ( OBJ_DATA *obj ) );
bool	room_is_dark	args( ( CHAR_DATA *ch ) );
bool	room_dark	args( ( ROOM_INDEX_DATA *pRoomIndex ) );
bool	isn_dark_safe	args( ( CHAR_DATA *ch ) );
bool	is_room_owner	args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *room) );
bool	room_is_private args( ( ROOM_INDEX_DATA *pRoomIndex ) );
bool	can_see 	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	can_see_obj	args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
bool	can_see_room	args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex) );
bool	can_drop_obj	args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
char *	item_type_name	args( ( OBJ_DATA *obj ) );
char *	affect_loc_name args( ( int location ) );
char * raffect_loc_name args( ( int location ) );
char *	affect_bit_name args( ( int vector ) );
char *	detect_bit_name args( ( int location ) );
char * raffect_bit_name args( ( int vector ) );
char *	extra_bit_name	args( ( int extra_flags ) );
char *	wear_bit_name	args( ( int wear_flags ) );
char *	act_bit_name	args( ( int act_flags ) );
char *	off_bit_name	args( ( int off_flags ) );
char *	imm_bit_name	args( ( int imm_flags ) );
char *	form_bit_name	args( ( int form_flags ) );
char *	part_bit_name	args( ( int part_flags ) );
char *	weapon_bit_name args( ( int weapon_flags ) );
char *	comm_bit_name	args( ( int comm_flags ) );
char *	cont_bit_name	args( ( int cont_flags) );
char *	flag_room_name	args( ( int vector) );
void	room_record	args( ( char *name, ROOM_INDEX_DATA *room,sh_int door) );
int	ch_skill_nok	args( (CHAR_DATA *ch , int sn));
int	skill_is_native	args( (CHAR_DATA *ch , int sn));
int	affect_check_obj	args( (CHAR_DATA *ch, int vector) );
bool	is_safe_rspell	args( ( int level, CHAR_DATA *victim) );
int	count_charmed	args( ( CHAR_DATA *ch ) );
void	add_mind	args( ( CHAR_DATA *ch, char *str) );
void	remove_mind	args( ( CHAR_DATA *ch, char *str) );
void	back_home	args( ( CHAR_DATA *ch ) );
CHAR_DATA*	find_char	args( ( CHAR_DATA *ch, char *argument, int door, int range) );
CHAR_DATA*	get_char_spell	args( ( CHAR_DATA *ch, char *argument, int *door, int range) );
void	path_to_track	args( ( CHAR_DATA *ch, CHAR_DATA *victim, int door) );

/* interp.c */
void	interpret	args( ( CHAR_DATA *ch, char *argument, bool is_order ) );
bool	is_number	args( ( char *arg ) );
int	number_argument args( ( char *argument, char *arg ) );
int	mult_argument	args( ( char *argument, char *arg) );
char *	one_argument	args( ( char *argument, char *arg_first ) );
/* interp-alias.c */
void	substitute_alias args( (DESCRIPTOR_DATA *d, char *input) );


/* save.c */
void	save_char_obj	args( ( CHAR_DATA *ch ) );
bool	load_char_obj	args( ( DESCRIPTOR_DATA *d, char *name ) );

/* skills.c */
int	exp_to_level	args( ( CHAR_DATA *ch, int points ) );
int	base_exp	args( ( CHAR_DATA *ch, int points ) );
int	exp_this_level	args( ( CHAR_DATA *ch, int level, int points ) );

int	exp_per_level	args( ( CHAR_DATA *ch, int points ) );
void	check_improve	args( ( CHAR_DATA *ch, int sn, bool success,
				    int multiplier ) );
int	group_lookup	args( (const char *name) );
void	group_add	args( ( CHAR_DATA *ch ) );

/* special.c */
SF *	spec_lookup	args( ( const char *name ) );
char *	spec_name	args( ( SPEC_FUN *function ) );

/* teleport.c */
RID *	room_by_name	args( ( char *target, int level, bool error) );

/*  obj_prog.c */
void oprog_set(OBJ_INDEX_DATA *, const char *, const char *);

/* mob_prog.c */
void mprog_set(MOB_INDEX_DATA *, const char *, const char *);


#undef	MID
#undef	OD
#undef	OID
#undef	RID
#undef	SF
#undef	AD

#endif

