/*
 * $Id: merc.h,v 1.88 1998-10-08 13:27:54 fjoe Exp $
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

#ifndef _MERC_H_
#define _MERC_H_

#include <sys/syslimits.h>
#include <ctype.h>
#include "typedef.h"
#include "const.h"
#include "tables.h"
#include "comm/comm.h"
#include "db/gsn.h"
#include "db/spellfn.h"
#include "db/msg.h"

/* basic types */
#include "namedp.h"
#include "buffer.h"
#include "str.h"
#include "mlstring.h"
#include "varr.h"
#include "flag.h"

/* utils */
#include "log.h"
#include "util.h"
#include "lookup.h"
#include "recycle.h"
#include "string_edit.h"

#include "class.h"
#include "clan.h"
#include "skills.h"
#include "religion.h"

/*
 *  COMMAND extra bits..
 */
#define CMD_KEEP_HIDE		(A)
#define CMD_GHOST		(B)
#define CMD_NOORDER		(C)

struct ethos_type {
	char *name;
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

struct time_info_data
{
	int 	hour;
	int 	day;
	int 	month;
	int 	year;
};

struct weather_data
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
#define CON_RESOLV			22

/*
 * Descriptor (channel) structure.
 */
struct descriptor_data
{
	DESCRIPTOR_DATA *	next;
	DESCRIPTOR_DATA *	snoop_by;
	CHAR_DATA * 		character;
	CHAR_DATA *	 	original;
	bool			valid;
	const char *		host;
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
	const char *		showstr_head;
	const char *		showstr_point;
	struct codepage*	codepage;
	const char *		editor;		/* editor id (OLC) */
	void *             	pEdit;		/* edited obj (OLC) */
	const char **		pString;	/* edited string (string_edit) */
};

/*
 * Attribute bonus structures.
 */
struct str_app_type
{
	int	tohit;
	int	todam;
	int	carry;
	int	wield;
};

struct int_app_type
{
	int	learn;
};

struct wis_app_type
{
	int	practice;
};

struct dex_app_type
{
	int	defensive;
};

struct con_app_type
{
	int	hitp;
	int	shock;
};

/*
 * Help table types.
 */
struct help_data
{
	HELP_DATA *	next;
	HELP_DATA *	next_in_area;
	AREA_DATA *	area;

	int		level;
	const char * 	keyword;
	mlstring *	text;
};
	
/*
 * Shop types.
 */

struct shop_data
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

#define STAT_STR	0
#define STAT_INT	1
#define STAT_WIS	2
#define STAT_DEX	3
#define STAT_CON	4
#define STAT_CHA	5

enum {
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

#define SLANG_COMMON	     0
#define SLANG_HUMAN	     1
#define SLANG_ELVISH	     2
#define SLANG_DWARVISH	     3
#define SLANG_GNOMISH	     4
#define SLANG_GIANT	     5
#define SLANG_TROLLISH	     6
#define SLANG_CAT	     7
#define SLANG_MAX	     8

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
	flag_t	act;			/* act bits for the race */
	flag_t	aff;			/* aff bits for the race */
	flag_t	off;			/* off bits for the race */
	flag_t	imm;			/* imm bits for the race */
	flag_t	res;			/* res bits for the race */
	flag_t	vuln;			/* vuln bits for the race */
	flag_t	form;			/* default form flag for the race */
	flag_t	parts;			/* default parts for the race */
};

struct pc_race_type  /* additional data for pc races */
{
	char *	name;			/* MUST be in race_type */
	char	who_name[6];
	int 	points; 		/* cost in exp of the race */
		/* FIXX ME */
	int	class_mult[16];		/* exp multiplier for class, * 100 */
	char *	skills[5];		/* bonus skills for the race */
	int	stats[MAX_STATS];	/* starting stats	*/
	int	max_stats[MAX_STATS];	/* maximum stats	*/
	flag_t	size;			/* aff bits for the race*/
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

/*-------------------------------------------------------------------
 * affects stuff
 */
struct affect_data
{
	AFFECT_DATA *	next;
	bool		valid;
	int		where;
	int		type;
	int		level;
	int		duration;
	int		location;
	int		modifier;
	flag_t 		bitvector;
};

/* where definitions */
#define TO_AFFECTS	0
#define TO_OBJECT	1
#define TO_IMMUNE	2
#define TO_RESIST	3
#define TO_VULN 	4
#define TO_WEAPON	5

struct where_data
{
	int	where;
	char *	name;
	FLAG *	table;
	char *	format;
};

WHERE_DATA *where_lookup(int where);

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
struct kill_data
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

/* class align restrictions */
#define CR_NONE			(0)
#define CR_GOOD			(A)
#define CR_NEUTRAL		(B)
#define CR_EVIL			(C)

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
#define AREA_CHANGED		(Z)	/* Area has been modified. */


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
#define AFF_DETECT_EVIL		(C)
#define AFF_DETECT_INVIS	(D)
#define AFF_DETECT_MAGIC	(E)
#define AFF_DETECT_HIDDEN	(F)
#define AFF_DETECT_GOOD		(G)
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
#define AFF_DARK_VISION		(Z)
#define AFF_BERSERK		(aa)
#define AFF_SWIM		(bb)
#define AFF_REGENERATION	(cc)
#define AFF_SLOW		(dd)
#define AFF_CAMOUFLAGE		(ee)

#define AFF_IMP_INVIS			(ff)	/* improved invis */
#define AFF_FADE		(gg)
#define AFF_SCREAM		(hh)
#define AFF_BLOODTHIRST 	(ii)
#define AFF_STUN		(jj)
#define AFF_WEAK_STUN		(kk)

#define AFF_DETECT_IMP_INVIS		(ll)	/* detect improved invis */
#define AFF_DETECT_FADE		(mm)
#define AFF_DETECT_UNDEAD	(nn)
#define AFF_DETECT_FEAR		(oo)
#define AFF_DETECT_FORM_TREE	(pp)
#define AFF_DETECT_FORM_GRASS 	(qq)
#define AFF_DETECT_WEB		(rr)
#define AFF_DETECT_LIFE		(ss)
#define AFF_ACUTE_VISION	(tt)

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
 * Sex.
 * Used in #MOBILES.
 */
enum {
	SEX_NEUTRAL,
	SEX_MALE,
	SEX_FEMALE,
	SEX_EITHER,		/* used only for NPC, means random sex
				   in create_mob */
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
#define APPLY_RACE		28

/*
 * Values for containers (value[1]).
 * Used in #OBJECTS.
 */
#define CONT_CLOSEABLE		(A)
#define CONT_PICKPROOF		(B)
#define CONT_CLOSED		(C)
#define CONT_LOCKED		(D)
#define CONT_PUT_ON		(E)
#define CONT_FOR_ARROW		(F)

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
#define ROOM_NORECALL		(N)
#define ROOM_IMP_ONLY		(O)
#define ROOM_GODS_ONLY		(P)
#define ROOM_HEROES_ONLY	(Q)
#define ROOM_NEWBIES_ONLY	(R)
#define ROOM_LAW		(S)
#define ROOM_NOWHERE		(T)
#define ROOM_BANK		(U)
#define ROOM_NOMAGIC		(W)
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
#define EX_ISDOOR	(A)
#define EX_CLOSED	(B)
#define EX_LOCKED	(C)
#define EX_NOFLEE	(D)
#define EX_PICKPROOF	(F)
#define EX_NOPASS	(G)
#define EX_EASY 	(H)
#define EX_HARD 	(I)
#define EX_INFURIATING	(J)
#define EX_NOCLOSE	(K)
#define EX_NOLOCK	(L)
#define EX_BITVAL	(Z)

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

#define PLR_CONFIRM_DELETE	(cc)
#define PLR_HARA_KIRI		(dd)
#define PLR_BLINK		(ee)
#define PLR_NEW			(ff)

#define IS_HARA_KIRI(ch) (IS_SET((ch)->act , PLR_HARA_KIRI))

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
/* COMM_NOIAC is useful to map '�' (IAC) to '�' when using win1251 codepage */
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
 * Prototype for a mob.
 * This is the in-memory version of #MOBILES.
 */
struct mob_index_data
{
	MOB_INDEX_DATA *	next;
	SPEC_FUN *		spec_fun;
	SHOP_DATA * 		pShop;
	MPTRIG *		mptrig_list;
	int			mptrig_types;
	int			vnum;
	int			group;
	bool			new_format;
	int			count;
	int			killed;
	const char *		name;
	mlstring *		short_descr;
	mlstring *		long_descr;
	mlstring *		description;
	flag_t			act;
	flag_t			affected_by;
	int			alignment;
	int			level;
	int			hitroll;
	int 			hit[3];
	int 			mana[3];
	int			damage[3];
	int			ac[4];
	int			dam_type;
	flag_t			off_flags;
	flag_t			imm_flags;
	flag_t			res_flags;
	flag_t			vuln_flags;
	flag_t			start_pos;
	flag_t			default_pos;
	flag_t			sex;
	int			race;
	int			wealth;
	flag_t			form;
	flag_t			parts;
	flag_t			size;
	const char *		material;
	flag_t			practicer;
	int			clan;
};

/*
 * One character (PC or NPC). *CHAR_DATA*
 */
struct char_data
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
	const char *		name;
	int			id;
	int			version;
	mlstring *		short_descr;
	mlstring *		long_descr;
	mlstring *		description;
	const char *		prompt;
	const char *		prefix;
	int			group;
	sflag_t			sex;
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
	int 			exp;	/* total exp */
	int			exp_tl;	/* exp gained this level */
	sflag_t			act;
	sflag_t			comm;	/* RT added to pad the vector */
	sflag_t			wiznet; /* wiz stuff */
	sflag_t			imm_flags;
	sflag_t			res_flags;
	sflag_t			vuln_flags;
	int			invis_level;
	int			incog_level;
	flag_t			affected_by;
	sflag_t			position;
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
	sflag_t			form;
	sflag_t			parts;
	sflag_t			size;
	const char *		material;
	/* mobile stuff */
	sflag_t			off_flags;
	int			damage[3];
	int			dam_type;
	sflag_t			start_pos;
	sflag_t			default_pos;
	int			mprog_delay;
	int 			status;
	bool			extracted;
	const char *		in_mind;
	int	 		religion;
	CHAR_DATA *		hunting;	/* hunt data */
	int 			endur;
	bool			riding; /* mount data */
	CHAR_DATA *		mount;
	int			slang;	/* spoken language */
	int			lang; /* interface language */
	CHAR_DATA *		hunter;	/* who quested to slay */
};

/*
 * Data which only PC's have.
 */
struct pc_data
{
	PC_DATA *		next;
	BUFFER *		buffer;
	const char *		pwd;
	const char *		bamfin;
	const char *		bamfout;
	const char *		title;
	const char *		twitlist;
	time_t			last_note;
	time_t			last_idea;
	time_t			last_penalty;
	time_t			last_news;
	time_t			last_changes;
	int 			perm_hit;
	int 			perm_mana;
	int 			perm_move;
	sflag_t			true_sex;
	int 			last_level;
	int			condition	[MAX_COND];
	varr			learned;
	int			points;
	const char *		alias[MAX_ALIAS];
	const char *		alias_sub[MAX_ALIAS];
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

struct pc_skill {
	int sn;
	int percent;
};

#define pc_skill_lookup(ch, sn) \
	((PC_SKILL*) varr_bsearch(&ch->pcdata->learned, &sn, cmpint))

/*
 * Liquids.
 */
#define LIQ_WATER	 0

struct liq_type
{
	char *	liq_name;
	char *	liq_color;
	int	liq_affect[5];
};

/*
 * Extra description data for a room or object.
 */
struct ed_data
{
	ED_DATA *	next;		/* Next in list 	    */
	bool		valid;
	const char *	keyword;	/* Keyword in look/examine  */
	mlstring *	description;	/* What to see		    */
};

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
struct obj_index_data
{
	OBJ_INDEX_DATA *	next;
	ED_DATA *		ed;
	AFFECT_DATA *		affected;
	bool			new_format;
	const char *		name;
	mlstring *		short_descr;
	mlstring *		description;
	int			vnum;
	int			reset_num;
	const char *		material;
	flag_t			item_type;
	flag_t			extra_flags;
	flag_t			wear_flags;
	int			level;
	int			condition;
	int			count;
	int			weight;
	int 			cost;
	int 			value[5];
	int 			limit;
	OPROG_FUN **		oprogs;
	int			clan;
};

/*
 * One object.	*OD*
 */
struct obj_data
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
	const char *		name;
	mlstring *		short_descr;
	mlstring *		description;
	sflag_t			item_type;
	sflag_t 		extra_flags;
	sflag_t 		wear_flags;
	sflag_t			wear_loc;
	int			weight;
	int 			cost;
	int			level;
	int			condition;
	const char *		material;
	int			timer;
	int 			value	[5];
	int 			progtypes;
	const char *		from;
	int 			altar;
	int 			pit;
	bool			extracted;
	int 			water_float;
};

/*
 * Exit data.
 */
struct exit_data
{
	union
	{
		ROOM_INDEX_DATA *	to_room;
		int			vnum;
	} u1;
	sflag_t		exit_info;
	int		key;
	const char *	keyword;
	mlstring *	description;
	EXIT_DATA *	next;		/* OLC */
	sflag_t		rs_flags;	/* OLC */
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
struct reset_data
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
struct area_data
{
	AREA_DATA * 	next;
	RESET_DATA *	reset_first;
	RESET_DATA *	reset_last;
	HELP_DATA *	help_first;
	HELP_DATA *	help_last;
	const char *	file_name;
	const char *	name;
	const char *	credits;
	int		age;
	int		nplayer;
	int		min_level;
	int		max_level;
	int		min_vnum;
	int		max_vnum;
	bool		empty;
	const char *	builders;	/* OLC */ /* Listing of */
	int		vnum;		/* OLC */ /* Area vnum  */
	flag_t		flags;		/* OLC */
	int		security;	/* OLC */ /* Value 1-9  */
	unsigned int	count;
	mlstring *	resetmsg;
};

struct room_history_data
{
	const char *name;
	int went;
	ROOM_HISTORY_DATA *next;
	ROOM_HISTORY_DATA *prev;
};

/*
 * Room type.
 */
struct room_index_data
{
	ROOM_INDEX_DATA *	next;
	ROOM_INDEX_DATA *	aff_next;
	CHAR_DATA * 		people;
	OBJ_DATA *		contents;
	ED_DATA *		ed;
	AREA_DATA * 		area;
	EXIT_DATA * 		exit	[6];
	RESET_DATA *		reset_first;	/* OLC */
	RESET_DATA *		reset_last;	/* OLC */
	mlstring *		name;
	mlstring *		description;
	const char *		owner;
	int 			clan;
	int			vnum;
	flag_t			room_flags;
	int			light;
	flag_t			sector_type;
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

/* trigger flags */
#define TRIG_CASEDEP	(A)

struct mptrig
{
	int		type;
	const char *	phrase;
	sflag_t		flags;
	int		vnum;		/* mob prog code vnum */
	MPTRIG * 	next;
	bool		valid;
};

struct mpcode
{
	int		vnum;
	const char *	code;
	MPCODE *	next;
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
 * Utility macros.
 */
#define IS_WATER(var) 	(((var)->sector_type == SECT_WATER_SWIM) || \
				 ((var)->sector_type == SECT_WATER_NOSWIM))
#define PERCENT(cur, max)	(max==0?0:((cur)*100)/(max))
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
#define IS_IMMORTAL(ch) 	(!IS_NPC(ch) && (ch)->level >= LEVEL_IMMORTAL)
#define IS_HERO(ch)		(!IS_NPC(ch) && (ch)->level >= LEVEL_HERO)
#define IS_TRUSTED(ch,level)	(get_trust((ch)) >= (level))
#define IS_AFFECTED(ch, sn)	(IS_SET((ch)->affected_by, (sn)))

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

#define IS_OUTSIDE(ch)	(!IS_SET((ch)->in_room->room_flags, ROOM_INDOORS))

#define WAIT_STATE(ch, npulse)	((ch)->wait = IS_IMMORTAL(ch) ?	\
				 1 : UMAX((ch)->wait, (npulse)))
#define RESET_WAIT_STATE(ch)	((ch)->wait = 1)
#define DAZE_STATE(ch, npulse)	((ch)->daze = UMAX((ch)->daze, (npulse)))
#define get_carry_weight(ch)	((ch)->carry_weight + (ch)->silver/10 +  \
						      (ch)->gold * 2 / 5)
#define HAS_TRIGGER(ch,trig)	(IS_SET((ch)->pIndexData->mptrig_types,(trig)))
#define IS_SWITCHED( ch )       (ch->desc && ch->desc->original)
#define IS_BUILDER(ch, Area)	(!IS_NPC(ch) && !IS_SWITCHED(ch) &&	\
				(ch->pcdata->security >= Area->security	\
				|| is_name(ch->name, Area->builders)))

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

/*
 * Description macros.
 */
/* new defines */
#define MAX_CHARM(ch)	((get_curr_stat(ch,STAT_INT) / 4) + (ch->level / 30))

enum {
	SOC_CHAR_NO_ARG,
	SOC_OTHERS_NO_ARG,
	SOC_CHAR_FOUND,
	SOC_OTHERS_FOUND,
	SOC_VICT_FOUND,
	SOC_CHAR_NOT_FOUND,
	SOC_CHAR_AUTO,
	SOC_OTHERS_AUTO,
	SOC_MAX
};

/*
 * Structure for a social in the socials table.
 */
struct social_type
{
	const char *	name;
	const char *	val[SOC_MAX];
};

/*
 * Global constants.
 */
extern	const	struct str_app_type	str_app 	[26];
extern	const	struct int_app_type	int_app 	[26];
extern	const	struct wis_app_type	wis_app 	[26];
extern	const	struct dex_app_type	dex_app 	[26];
extern	const	struct con_app_type	con_app 	[26];

extern	const	struct cmd_type	cmd_table	[];
extern	const	struct wiznet_type	wiznet_table	[];
extern	const	struct attack_type	attack_table	[];
extern	const	struct race_type	race_table	[];
extern	const	struct pc_race_type	pc_race_table	[MAX_PC_RACE];
extern	const	struct spec_type	spec_table	[];
extern	const	struct liq_type	liq_table	[];
extern	const	struct ethos_type	ethos_table	[];
extern		struct social_type	social_table	[MAX_SOCIALS];

/*
 * Global variables.
 */
extern		SHOP_DATA	  *	shop_first;

extern		CHAR_DATA	  *	char_list;
extern		DESCRIPTOR_DATA   *	descriptor_list;
extern		OBJ_DATA	  *	object_list;

extern		MPCODE	 	 *	mpcode_list;

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
 * The crypt(3) function is not available on some operating systems.
 * In particular, the U.S. Government prohibits its export from the
 * United States to foreign countries.
 * Turn on NOCRYPT to keep passwords in plain text.
 */
#if	defined(NOCRYPT)
#define crypt(s1, s2)	(s1)
#endif

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
#define MPC	MPCODE

/* act_hera.c */
/* enter.c */
RID  *get_random_room	(CHAR_DATA *ch);
/* hunt.c */
void hunt_victim(CHAR_DATA *ch);
int find_path(int in_room_vnum, int out_room_vnum, CHAR_DATA *ch, 
	       int depth, int in_zone);

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
void	affect_check	(CHAR_DATA *ch, int where, flag_t vector);
int	count_users	(OBJ_DATA *obj);
void	deduct_cost	(CHAR_DATA *ch, int cost);
void	affect_enchant	(OBJ_DATA *obj);
int	check_immune	(CHAR_DATA *ch, int dam_type);
bool	check_material	(OBJ_DATA *obj, char *material);
bool	is_metal	(OBJ_DATA *obj);
bool	is_old_mob	(CHAR_DATA *ch);
int	get_age 	(CHAR_DATA *ch);
void	reset_char	(CHAR_DATA *ch);
int	get_trust	(CHAR_DATA *ch);
int	get_curr_stat	(CHAR_DATA *ch, int stat);
int	get_max_train	(CHAR_DATA *ch, int stat);
int	get_max_train2	(CHAR_DATA *ch, int stat);
int	can_carry_n	(CHAR_DATA *ch);
int	can_carry_w	(CHAR_DATA *ch);
bool	is_name 	(const char *str, const char *namelist);
bool	is_name_raw	(const char *str, const char *namelist,
			 int (*cmpfun)(const char*, const char*));
void	name_toggle	(CHAR_DATA *ch, const char *name,
			 const char *editor_name, const char **namelist);
void	affect_to_char	(CHAR_DATA *ch, AFFECT_DATA *paf);
void	affect_to_obj	(OBJ_DATA *obj, AFFECT_DATA *paf);
void	affect_remove	(CHAR_DATA *ch, AFFECT_DATA *paf);
void	affect_remove_obj (OBJ_DATA *obj, AFFECT_DATA *paf);
void	affect_strip	(CHAR_DATA *ch, int sn);
void	affect_bit_strip(CHAR_DATA *ch, int where, flag_t bits);
bool	is_affected	(CHAR_DATA *ch, int sn);
bool	is_bit_affected	(CHAR_DATA *ch, int where, flag_t bits);
int	has_obj_affect	(CHAR_DATA *ch, int vector);
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
void	room_record	(const char *name, ROOM_INDEX_DATA *room,int door);
bool	is_safe_rspell	(int level, CHAR_DATA *victim);
int	count_charmed	(CHAR_DATA *ch);
void	add_mind	(CHAR_DATA *ch, const char *str);
void	remove_mind	(CHAR_DATA *ch, const char *str);
void	back_home	(CHAR_DATA *ch);
CHAR_DATA*	find_char	(CHAR_DATA *ch, const char *argument, int door, int range);
CHAR_DATA*	get_char_spell	(CHAR_DATA *ch, const char *argument, int *door, int range);
void	path_to_track	(CHAR_DATA *ch, CHAR_DATA *victim, int door);
bool	in_PK(CHAR_DATA *ch, CHAR_DATA *victim);
bool	can_gate(CHAR_DATA *ch, CHAR_DATA *victim);
bool	obj_is_pit(OBJ_DATA *obj);
void format_obj(BUFFER *output, OBJ_DATA *obj);
void format_obj_affects(BUFFER *output, AFFECT_DATA *paf, bool duration);
int	get_wear_level(CHAR_DATA *ch, OBJ_DATA *obj);

bool	saves_spell	(int level, CHAR_DATA *victim, int dam_type );
bool	check_dispel	(int dis_level, CHAR_DATA *victim, int sn);
void	obj_cast_spell	(int sn, int level, CHAR_DATA *ch, CHAR_DATA *victim,
			 OBJ_DATA *obj );
bool	saves_dispel	(int dis_level, int spell_level, int duration);

bool remove_obj (CHAR_DATA * ch, int iWear, bool fReplace);
void wear_obj   (CHAR_DATA * ch, OBJ_DATA * obj, bool fReplace);

/* interp.c */
void	interpret	(CHAR_DATA *ch, const char *argument);
void	interpret_raw	(CHAR_DATA *ch, const char *argument, bool is_order);
bool	is_number	(const char *argument);
int	number_argument (const char *argument, char *arg);
int	mult_argument	(const char *argument, char *arg);
const char *	one_argument	(const char *argument, char *arg_first);
const char *	first_arg	(const char *argument, char *arg_first,
				 bool fCase);
const char* PERS(CHAR_DATA *ch, CHAR_DATA *looker);

/* save.c */
void	save_char_obj	(CHAR_DATA *ch, bool reboot);
void	load_char_obj	(DESCRIPTOR_DATA *d, const char *name);

/* special.c */
SF *	spec_lookup	(const char *name);
char *	spec_name	(SPEC_FUN *function);

void new_reset(ROOM_INDEX_DATA *pRoom, RESET_DATA *pReset);

RESET_DATA	*new_reset_data		(void);
void		free_reset_data		(RESET_DATA *pReset);

AREA_DATA	*new_area		(void);
AREA_DATA *	area_lookup		(int vnum);
AREA_DATA *	area_vnum_lookup	(int vnum);
void		free_area		(AREA_DATA *pArea);

EXIT_DATA	*new_exit		(void);
void		free_exit		(EXIT_DATA *pExit);
ROOM_INDEX_DATA *new_room_index		(void);
void		free_room_index		(ROOM_INDEX_DATA *pRoom);
AFFECT_DATA	*new_affect		(void);
void		free_affect		(AFFECT_DATA* pAf);
SHOP_DATA	*new_shop		(void);
void		free_shop		(SHOP_DATA *pShop);
OBJ_INDEX_DATA	*new_obj_index		(void);
void		free_obj_index		(OBJ_INDEX_DATA *pObj);
MOB_INDEX_DATA	*new_mob_index		(void);
void		free_mob_index		(MOB_INDEX_DATA *pMob);
void		show_liqlist		(CHAR_DATA *ch);
void		show_damlist		(CHAR_DATA *ch);

MPTRIG *	mptrig_new              (int type, const char *phrase, int vnum);
void		mptrig_add		(MOB_INDEX_DATA *mob, MPTRIG *mptrig);
void            mptrig_free		(MPTRIG *mptrig);

extern MPCODE *	mpcode_list;
MPCODE *	mpcode_new		(void);
void		mpcode_add		(MPCODE *mpcode);
MPCODE *	mpcode_lookup		(int vnum);
void		mpcode_free		(MPCODE *mpcode);

/* db.c */
extern int		newmobs;
extern int		newobjs;
extern MOB_INDEX_DATA *	mob_index_hash	[MAX_KEY_HASH];
extern OBJ_INDEX_DATA *	obj_index_hash	[MAX_KEY_HASH];
extern ROOM_INDEX_DATA *room_index_hash [MAX_KEY_HASH];
extern int		top_mob_index;
extern int		top_obj_index;
extern int		top_vnum_mob;
extern int		top_vnum_obj;
extern int		top_vnum_room;
extern int  		top_affect;
extern int		top_ed; 
extern int		top_area;
extern int		top_exit;
extern int		top_help;
extern int		top_reset;
extern int		top_room;
extern int		top_shop;
extern AREA_DATA *	area_first;
extern AREA_DATA *	area_last;
extern AREA_DATA *	area_current;
extern HELP_DATA *	help_first;
extern SHOP_DATA *	shop_last;

void	reset_area      (AREA_DATA * pArea);		/* OLC */
void	reset_room	(ROOM_INDEX_DATA *pRoom);	/* OLC */

void		boot_db		(void);
CHAR_DATA *	create_mob	(MOB_INDEX_DATA *pMobIndex);
CHAR_DATA *	create_named_mob(MOB_INDEX_DATA *pMobIndex, const char *name);
void		clone_mob	(CHAR_DATA *parent, CHAR_DATA *clone);
OBJ_DATA *	create_obj	(OBJ_INDEX_DATA *pObjIndex, int level);
OBJ_DATA *	create_named_obj(OBJ_INDEX_DATA *pObjIndex, int level,
				 const char *name);
OBJ_DATA *	create_obj_nocount (OBJ_INDEX_DATA *pObjIndex, int level);
void	clone_obj	(OBJ_DATA *parent, OBJ_DATA *clone); 
void	clear_char	(CHAR_DATA *ch);
ED_DATA * ed_lookup(const char *name, ED_DATA *ed);
MOB_INDEX_DATA *	get_mob_index	(int vnum);
OBJ_INDEX_DATA *	get_obj_index	(int vnum);
ROOM_INDEX_DATA *	get_room_index	(int vnum);
flag_t	flag_convert	(char letter);
void *	alloc_perm	(int sMem);
int	number_fuzzy	(int number);
int	number_range	(int from, int to);
int	number_percent	(void);
int	number_door	(void);
int	number_bits	(int width);
long     number_mm      (void);
int	dice		(int number, int size);
int	interpolate	(int level, int value_00, int value_32);
char *	smash_tilde	(const char *str);
char *	capitalize	(const char *str);
void	append_file	(CHAR_DATA *ch, const char *file, const char *str);
void	tail_chain	(void);
char *format_flags(flag_t flags);

#define chance(num) (number_range(1, 100) <= num)

/* from db2.c */
extern int	social_count;

/* conversion from db.h */
void	convert_mob(MOB_INDEX_DATA *mob);
void	convert_obj(OBJ_INDEX_DATA *obj);

/* macro for flag swapping */
#define GET_UNSET(flag1,flag2)	(~(flag1)&((flag1)|(flag2)))

extern void vnum_check(AREA_DATA *area, int vnum);	/* OLC */

void convert_mobile(MOB_INDEX_DATA *pMobIndex);            /* OLC ROM */
void convert_objects(void);                                /* OLC ROM */
void convert_object(OBJ_INDEX_DATA *pObjIndex);            /* OLC ROM */

char * fix_string(const char *);
int xgetc(FILE *fp);
void xungetc(int c, FILE *fp);

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

/*
 * Global Constants
 */
extern	char *	const			dir_name	[];
extern	const	int			rev_dir		[];
extern	const	struct spec_type	spec_table	[];
extern	char				DEFAULT_PROMPT	[];

/* act_comm.c */
void		check_sex	(CHAR_DATA *ch);
void		add_follower	(CHAR_DATA *ch, CHAR_DATA *master);
void		stop_follower	(CHAR_DATA *ch);
void		nuke_pets	(CHAR_DATA *ch);
void		die_follower	(CHAR_DATA *ch);
void		do_afk		(CHAR_DATA *ch, const char *argument);
void		do_lang		(CHAR_DATA *ch, const char *argument);
void		do_music	(CHAR_DATA *ch, const char *argument);
void		do_gossip	(CHAR_DATA *ch, const char *argument);
void		do_noiac	(CHAR_DATA *ch, const char *argument);
void		do_notelnet	(CHAR_DATA *ch, const char *argument);
CHAR_DATA*	leader_lookup	(CHAR_DATA *ch);
void 		do_tell_raw	(CHAR_DATA *ch, CHAR_DATA *victim,
				 const char *msg);
#define	is_same_group(ach, bch) (leader_lookup(ach) == leader_lookup(bch))

/* act_info.c */
void	set_title(CHAR_DATA *ch, const char *title);
char	*get_cond_alias(OBJ_DATA *obj);
void	do_date(CHAR_DATA* ch, const char *argument);
bool	check_blind_raw(CHAR_DATA *ch);

/* act_move.h */
void move_char(CHAR_DATA *ch, int door, bool follow);
char *find_way(CHAR_DATA *ch, ROOM_INDEX_DATA *rstart, ROOM_INDEX_DATA *rend);
int guild_check(CHAR_DATA *ch, ROOM_INDEX_DATA *room);

/* act_obj.h */
bool can_loot		(CHAR_DATA *ch, OBJ_DATA *obj);
void get_obj		(CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *container);
int floating_time	(OBJ_DATA *obj);
bool may_float		(OBJ_DATA *obj);
bool cant_float 	(OBJ_DATA *obj);
void wear_obj		(CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace);

/* act_wiz.h */
#define wiznet(string, ch, obj, flag, flag_skip, min_level) \
		wiznet_printf(ch, obj, flag, flag_skip, min_level, string)
void wiznet_printf(CHAR_DATA *ch, OBJ_DATA *obj, long flag, long flag_skip,
		   int min_level, char* format,...);
void reboot_muddy(void);
ROOM_INDEX_DATA *find_location(CHAR_DATA *ch, const char *argument);

#endif

