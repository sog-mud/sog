/*
 * $Id: merc.h,v 1.260 1999-11-30 14:50:33 kostik Exp $
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

#if !defined(BSD44)
#if !defined(LINUX)
#if !defined(WIN32)
#if !defined(SUNOS)
#if !defined(SVR4)
#error "You must uncomment options for your system in Makefile.osdep"
#endif
#endif
#endif
#endif
#endif

#if defined (WIN32)
#	undef  NOCRYPT
#	define NOCRYPT
#	pragma warning (disable : 4244 4018 4761)
#endif

#include <limits.h>
#include <ctype.h>

#include "typedef.h"
#include "const.h"

/* basic types */
#include "buffer.h"
#include "str.h"
#include "namelist.h"
#include "mlstring.h"
#include "varr.h"
#include "hash.h"
#include "strkey_hash.h"
#include "flag.h"
#include "cmd.h"
#include "cc_rule.h"

#include "tables.h"
#include "comm.h"
#include "comm_act.h"
#include "memalloc.h"
#include "msg.h"
#include "hometown.h"

/* utils */
#include "log.h"
#include "util.h"

#include "race.h"
#include "affect.h"
#include "class.h"
#include "clan.h"
#include "spec.h"
#include "skills.h"
#include "religion.h"
#include "damtype.h"
#include "material.h"
#include "liquid.h"

/*
 * configuration parameters
 */
extern varr control_sockets;
extern varr info_sockets;
extern varr info_trusted;

/* mud server options (etc/system.conf) */
#define OPT_ASCII_ONLY_NAMES	(A)

extern flag32_t mud_options;

/* general align */
#define ALIGN_NONE		-1
#define ALIGN_GOOD		1000
#define ALIGN_NEUTRAL		0
#define ALIGN_EVIL		-1000

/* align restrictions */
#define RA_NONE			(0)
#define RA_GOOD			(A)
#define RA_NEUTRAL		(B)
#define RA_EVIL			(C)

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
enum {
	CON_GET_CODEPAGE,
	CON_GET_NAME,
	CON_RESOLV,
	CON_CONFIRM_NEW_NAME,
	CON_GET_NEW_PASSWORD,
	CON_CONFIRM_NEW_PASSWORD,
	CON_GET_NEW_RACE,
	CON_GET_NEW_SEX,
	CON_GET_NEW_CLASS,
	CON_GET_ALIGNMENT,
	CON_ROLL_STATS,
	CON_ACCEPT_STATS,
	CON_PICK_HOMETOWN,
	CON_GET_ETHOS,
	CON_CREATE_DONE,

	CON_BREAK_CONNECT,
	CON_GET_OLD_PASSWORD,
	CON_READ_IMOTD,
	CON_READ_MOTD,
	CON_PLAYING
};

typedef struct outbuf_t {
	char *	buf;
	uint 	size;
	uint 	top;
} outbuf_t;

/*
 * Structure for an OLC editor command.
 */
struct olced_t {
	const char *	id;
	const char *	name;
	olc_cmd_t *	cmd_table;
};

#define OLCED(ch) (ch->desc->olced)
#define IS_EDIT(ch, ed_id) (OLCED(ch) && OLCED(ch)->id == ed_id)

/*
 * descriptor variable data (that is stored in pfile)
 *
 * both `descriptor_data' and `pc_data' have `dvdata_t*' field
 *
 * dvdata is read from pfile into pc and is shared (with ref counting)
 * between pc_data and descriptor_data
 */
typedef struct dvdata_t dvdata_t;
struct dvdata_t {
	int		refcnt;		/* reference count */

	int		lang;		/* interface language	*/
	int 		pagelen;	/* pager lines		*/

	const char *	prompt;	
	const char *	prefix;

	const char *	alias[MAX_ALIAS];
	const char *	alias_sub[MAX_ALIAS];
};

dvdata_t *	dvdata_new	(void);
dvdata_t *	dvdata_dup	(dvdata_t*);
void		dvdata_free	(dvdata_t*);

/*
 * Descriptor (channel) structure.
 */
struct descriptor_data
{
	DESCRIPTOR_DATA *	next;
	DESCRIPTOR_DATA *	snoop_by;
	CHAR_DATA * 		character;
	CHAR_DATA *	 	original;
	const char *		host;
	const char *		ip;
	int			descriptor;
	int			connected;
	int 			wait_for_se;
	bool			fcommand;
	char			inbuf		[4 * MAX_INPUT_LENGTH];
	char			incomm		[MAX_INPUT_LENGTH];
	char			inlast		[MAX_INPUT_LENGTH];
	int 			repeat;
	outbuf_t		out_buf;
	outbuf_t		snoop_buf;
	const char *		showstr_head;
	const char *		showstr_point;
	struct codepage*	codepage;

/* OLC stuff */
	olced_t	*		olced;
	void *             	pEdit;		/* edited obj	*/
	void *             	pEdit2;		/* edited obj 2	*/

/* string editor stuff */
	const char **		pString;	/* edited string	*/
	const char *		backup;		/* backup		*/

	dvdata_t *		dvdata;
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
	mlstring	text;
};
	
/*
 * Shop types.
 */

struct shop_data
{
	SHOP_DATA * next;		/* Next shop in list		*/
	int	keeper; 		/* Vnum of shop keeper mob	*/
	int	buy_type [MAX_TRADE];	/* Item types shop will buy	*/
	uint	profit_buy;		/* Cost multiplier for buying	*/
	uint	profit_sell;		/* Cost multiplier for selling	*/
	int	open_hour;		/* First opening hour		*/
	int	close_hour;		/* First closing hour		*/
};

/*
 * Per-class stuff.
 */
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
	const char *	name;
	flag32_t	flag;
	int 		level;
};

struct spec_type
{
	char *		name;			/* special function name */
	SPEC_FUN *	function;		/* the function */
};
/***************************************************************************
 *									   *
 *		     VALUES OF INTEREST TO AREA BUILDERS		   *
 *		     (Start of section ... start here)			   *
 *									   *
 ***************************************************************************/

/* skills group numbers*/
#define GROUP_NONE		0
#define GROUP_WEAPONSMASTER	(A)
#define GROUP_ATTACK		(B)
#define GROUP_BEGUILING 	(C)
#define GROUP_BENEDICTIONS	(D)
#define GROUP_COMBAT		(E)
#define GROUP_CREATION		(F)
#define GROUP_CURATIVE		(G)
#define GROUP_DIVINATION 	(H)
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
#define GROUP_NECROMANCY	(Y)
#define GROUP_EVOCATION		(Z)
#define GROUP_CONJURATION	(aa)
#define GROUP_SUMMONING		(bb)
#define GROUP_ALTERATION	(cc)
#define GROUP_ABJURATION	(dd)
#define GROUP_CHARM		(ee)

/*
 * area flags
 */
#define AREA_CLOSED		(B)	/* transportation does not work,
					   no quests			*/
#define AREA_NOQUEST		(C)	/* no quests in this area	*/
#define AREA_UPDATE_ALWAYS	(D)	/* always update area (even if  */
					/* there are players in it)	*/
#define AREA_UPDATE_FREQUENTLY	(E)	/* update area more frequently	*/
					/* (every 3 ticks)		*/   
#define AREA_CHANGED		(Z)	/* area has been modified	*/


/*
 * ACT bits for mobs.  *ACT*
 * Used in #MOBILES.
 */
#define ACT_NPC			(A)		/* Auto set for mobs	*/
#define ACT_SENTINEL		(B)		/* Stays in one room	*/
#define ACT_SCAVENGER		(C)		/* Picks up objects	*/
#define ACT_AGGRESSIVE		(F)		/* Attacks PC's		*/
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
#define ACT_CLAN_GUARD		(ff)
#define ACT_SUMMONED		(gg)		/* summoned (golem etc.)*/
#define ACT_SAGE		(hh)		/* sage (Otho etc.)	*/
#define ACT_REPAIRMAN		(ii)
#define ACT_FAMILIAR		(jj)		/* familiar 		*/

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
#define OFF_DISTANCE		(X)
#define OFF_SPELLBANE		(Y)
#define OFF_DEATHBLOW		(Z)

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
#define AFF_INVIS		(B)
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

#define AFF_IMP_INVIS		(ff)	/* improved invis */
#define AFF_FADE		(gg)
#define AFF_SCREAM		(hh)
#define AFF_BLOODTHIRST 	(ii)
#define AFF_STUN		(jj)
#define AFF_WEAK_STUN		(kk)

#define AFF_DETECT_IMP_INVIS	(ll)	/* detect improved invis */
#define AFF_DETECT_FADE		(mm)
#define AFF_DETECT_UNDEAD	(nn)
#define AFF_DETECT_FEAR		(oo)
#define AFF_DETECT_FORM_TREE	(pp)
#define AFF_DETECT_FORM_GRASS 	(qq)
#define AFF_DETECT_WEB		(rr)
#define AFF_DETECT_LIFE		(ss)
#define AFF_ACUTE_VISION	(tt)
#define AFF_BLACK_SHROUD	(uu)
#define AFF_QUESTTARGET		(vv)
#define AFF_BLEND		(ww)	/* Forest blending */
#define AFF_AWARENESS		(xx)	/* Detect blend and camouflage */
#define AFF_TURNED		(yy)	/* Character is turned into other creature */
#define AFF_WATER_BREATHING	(zz)

/* where definitions for room */
#define TO_ROOM_AFFECTS 0
#define TO_ROOM_CONST	1
#define TO_ROOM_FLAGS	2

/* room applies */
#define APPLY_ROOM_NONE 	0
#define APPLY_ROOM_HEAL 	1
#define APPLY_ROOM_MANA 	2
#define APPLY_ROOM_SECT 	3

/*
 * *AFF* bits for rooms
 */
#define RAFF_RANDOMIZER		(A)
#define RAFF_ESPIRIT		(B)
#define RAFF_CURSE		(C)

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
#define ITEM_LIGHT		1
#define ITEM_SCROLL		2
#define ITEM_WAND		3
#define ITEM_STAFF		4
#define ITEM_WEAPON		5
#define ITEM_TREASURE		8
#define ITEM_ARMOR		9
#define ITEM_POTION		10
#define ITEM_CLOTHING		11
#define ITEM_FURNITURE		12
#define ITEM_TRASH		13
#define ITEM_CONTAINER		15
#define ITEM_DRINK_CON		17
#define ITEM_KEY		18
#define ITEM_FOOD		19
#define ITEM_MONEY		20
#define ITEM_BOAT		22
#define ITEM_CORPSE_NPC		23
#define ITEM_CORPSE_PC		24
#define ITEM_FOUNTAIN		25
#define ITEM_PILL		26
#define ITEM_PROTECT		27
#define ITEM_MAP		28
#define ITEM_PORTAL		29
#define ITEM_WARP_STONE 	30
#define ITEM_ROOM_KEY		31
#define ITEM_GEM		32
#define ITEM_JEWELRY		33
#define ITEM_JUKEBOX		34
#define ITEM_TATTOO		35

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
#define ITEM_NOT_EDIBLE		(bb)
#define ITEM_QUEST		(cc)
#define ITEM_ENCHANTED		(dd)	/* obj is enchanted */
#define ITEM_CLAN		(ee)
#define ITEM_QUIT_DROP		(ff)
#define ITEM_PIT		(gg)
#define ITEM_CHQUEST		(hh)
#define ITEM_OLDSTYLE		(zz)	/* obj is in oldstyle format */

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
#define ITEM_WEAR_CLANMARK	(S)

/* obj_data->condition */
#define COND_EXCELLENT		100
#define COND_FINE		80
#define COND_GOOD		60
#define COND_AVERAGE		40
#define COND_POOR		20
#define COND_FRAGILE		0

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
#define	WEAPON_STAFF		12

#define WEAPON_IS(obj, wclass)	(INT_VAL(obj->value[0]) == (wclass))
#define WEAPON_IS_LONG(obj) (WEAPON_IS((obj), WEAPON_STAFF) || WEAPON_IS((obj), WEAPON_SPEAR) || WEAPON_IS((obj), WEAPON_POLEARM) || WEAPON_IS((obj), WEAPON_LANCE))

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
#define WEAPON_THROW		(K)

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
#define APPLY_SIZE		26

/*
 * Skillaffects flags
 */
#define SK_AFF_TEACH		(A)	/* Teach skill */
#define SK_AFF_ALL		(B)	/* Affect ALL skills */
#define SK_AFF_NOTCLAN		(C)	/* Affect only normal skills */

/*
 * Values for containers (value[1]).
 * Used in #OBJECTS.
 */
#define CONT_CLOSEABLE		(A)
#define CONT_PICKPROOF		(B)
#define CONT_CLOSED		(C)
#define CONT_LOCKED		(D)
#define CONT_PUT_ON		(E)
#define CONT_QUIVER		(F)

/*
 * Room flags.
 * Used in #ROOMS.
 */
#define ROOM_DARK		(A)
#define ROOM_NOMOB		(C)
#define ROOM_INDOORS		(D)
#define ROOM_PEACE		(H)
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
#define ROOM_GUILD		(Y)
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
enum {
	SECT_INSIDE,
	SECT_CITY,
	SECT_FIELD,
	SECT_FOREST,
	SECT_HILLS,
	SECT_MOUNTAIN,
	SECT_WATER_SWIM,
	SECT_WATER_NOSWIM,
	SECT_UNDERWATER,
	SECT_AIR,
	SECT_DESERT,

	MAX_SECT = SECT_DESERT
};

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
#define WEAR_CLANMARK		     21
#define WEAR_STUCK_IN		     22
#define MAX_WEAR		     23

/***************************************************************************
 *									   *
 *		     VALUES OF INTEREST TO AREA BUILDERS		   *
 *		     (End of this section ... stop here)		   *
 *									   *
 ***************************************************************************/

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

#define ETHOS_NONE	0
#define ETHOS_LAWFUL	(A)
#define ETHOS_NEUTRAL	(B)
#define ETHOS_CHAOTIC	(C)

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
#define PLR_NOTITLE		(K)
/* RT personal flags */
#define PLR_NOEXP		(L)
#define PLR_AUTOLOOK		(M)
#define PLR_HOLYLIGHT		(N)
#define PLR_CANLOOT		(P)
#define PLR_NOSUMMON		(Q)
#define PLR_NOFOLLOW		(R)
#define PLR_NOCANCEL		(S)
#define PLR_GHOST		(T)

/* penalty flags */
#define PLR_PERMIT		(U)
#define PLR_LOG 		(W)
#define PLR_FREEZE		(Y)
#define PLR_PUMPED		(Z)	/* adrenalin is gushing */
#define PLR_PRACTICER		(aa)
#define PLR_CONFIRM_DELETE	(cc)
#define PLR_HARA_KIRI		(dd)
#define PLR_NEW			(ff)

/* Trust stuff */
#define TRUST_GROUP		(A)
#define TRUST_CLAN		(B)
#define TRUST_ALL		(C) /* mutually exclusive with previous two */

#define IS_HARA_KIRI(ch)	(!IS_NPC(ch) &&	\
				 IS_SET(PC(ch)->plr_flags, PLR_HARA_KIRI))
#define IS_CLAN_GUARD(ch)	(IS_NPC(ch) && \
				 IS_SET(ch->pMobIndex->act, ACT_CLAN_GUARD))
#define IS_OWNER(ch, obj) (!mlstr_cmp(&ch->short_descr, &obj->owner))

#define IS_WANTED(ch)	(!IS_NPC(ch) && !IS_NULLSTR(PC(ch)->wanted_by))
#define SET_WANTED(ch, w_by)					\
{								\
	if (!IS_NPC(ch)) {					\
		free_string(PC(ch)->wanted_by);			\
		PC(ch)->wanted_by = str_qdup(w_by);		\
	}							\
}

#define SET_HIT(_ch, _hit) ((_ch)->hit = (_ch)->max_hit = (_ch)->perm_hit = (_hit))
#define SET_MANA(_ch, _mana) ((_ch)->mana = (_ch)->max_mana = (_ch)->perm_mana = (_mana))
#define SET_MOVE(_ch, _move) ((_ch)->move = (_ch)->max_move = (_ch)->perm_move = (_move))

/*
 * this should be used for !IS_NPC only
 */
#define IS_PUMPED(ch)	(!IS_NPC(ch) &&					\
			 IS_SET(PC(ch)->plr_flags, PLR_PUMPED))

#define SET_FIGHT_TIME(ch)						\
	{								\
		(ch)->last_fight_time = current_time;			\
		if (!IS_NPC(ch))					\
			SET_BIT(PC(ch)->plr_flags, PLR_PUMPED);	\
	}

#define RESET_FIGHT_TIME(ch)						\
	{								\
		(ch)->last_fight_time = -1;				\
		if (!IS_NPC(ch))					\
			REMOVE_BIT(PC(ch)->plr_flags, PLR_PUMPED);	\
	}

#define IS_VAMPIRE(ch)	(get_skill(ch, "vampire") == 100)

/* RT comm flags -- may be used on both mobs and chars */
#define COMM_QUIET		(A)
#define COMM_DEAF		(B)
#define COMM_NOWIZ		(C)
#define COMM_NOAUCTION		(D)
#define COMM_NOGOSSIP		(E)
#define COMM_NOQUESTION 	(F)
#define COMM_NOMUSIC		(G)
#define COMM_NOQUOTE		(I)
#define COMM_NOCLAN		(J)

/* display flags */
#define COMM_QUIET_EDITOR	(K)
#define COMM_COMPACT		(L)
#define COMM_BRIEF		(M)
#define COMM_PROMPT		(N)
#define COMM_COMBINE		(O)
#define COMM_TELNET_GA		(P)
#define COMM_SHOWAFF		(Q)
#define COMM_COLOR		(S)

/* penalties */
#define COMM_NOEMOTE		(T)
#define COMM_NOSHOUT		(U)
#define COMM_NOTELL		(V)
#define COMM_NOCHANNELS 	(W)
#define COMM_NOENG		(X)
#define COMM_SNOOP_PROOF	(Y)
#define COMM_AFK		(Z)

#define COMM_LONG		(aa)
#define COMM_NOTELNET		(bb)
#define COMM_NOIAC		(cc)
#define COMM_NOVERBOSE		(dd)
#define COMM_NOBUST		(ee)
#define COMM_NOFLEE		(ff)
#define COMM_NONOTE		(gg)

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
#define WIZ_OLC			(U)
#define WIZ_WIZLOG		(V)

/*
 * Prototype for a mob.
 */
struct mob_index_data
{
	MOB_INDEX_DATA *	next;
	SPEC_FUN *		spec_fun;
	SHOP_DATA * 		pShop;
	MPTRIG *		mptrig_list;
	int			mptrig_types;
	int			vnum;
	int			fvnum;
	int			group;
	int			count;
	int			killed;
	const char *		name;
	mlstring		short_descr;
	mlstring		long_descr;
	mlstring		description;
	flag64_t		affected_by;
	int			alignment;
	int			level;
	int			hitroll;
	int 			hit[3];
	int 			mana[3];
	int			damage[3];
	int			ac[4];
	const char *		damtype;
	flag64_t		act;
	flag32_t		off_flags;
	flag32_t		imm_flags;
	flag32_t		res_flags;
	flag32_t		vuln_flags;
	flag32_t		start_pos;
	flag32_t		default_pos;
	flag32_t		sex;
	const char *		race;		/* race */
	int			wealth;
	flag32_t		form;
	flag32_t		parts;
	flag32_t		size;
	const char *		material;
	flag32_t		practicer;
	const char *		clan;
	int			invis_level;	/* mobinvis level */
	int			incog_level;	/* mobincog level */
};

#define NPC(ch)	((NPC_DATA *) ((ch) + 1))
#define PC(ch)	((PC_DATA *) ((ch) + 1))

CHAR_DATA *	char_new	(MOB_INDEX_DATA *pMobIndex);
void 		char_free	(CHAR_DATA *ch);

/* char_load flags */
#define LOAD_F_NOCREATE	(A)
#define LOAD_F_MOVE	(B)

/* char_save flags (these are mutually exclusive) */
#define SAVE_F_NONE	(A)
#define SAVE_F_NORMAL	(B)
#define SAVE_F_REBOOT	(C)
#define SAVE_F_PSCAN	(D)

CHAR_DATA *	char_load	(const char *name, int flags);
void		char_save	(CHAR_DATA *ch, int flags);
void		char_nuke	(CHAR_DATA *ch);
void		move_pfile	(const char *name,
				 int minvnum, int maxvnum, int delta);
void		move_pfiles	(int minvnum, int maxvnum, int delta);

void	objval_init	(flag32_t item_type, vo_t *v);
void	objval_cpy	(flag32_t item_type, vo_t *dst, vo_t *src);
void	objval_destroy	(flag32_t item_type, vo_t *v);

void	fwrite_objval	(flag32_t item_type, vo_t *v, FILE *fp);
void	fread_objval	(flag32_t item_type, vo_t *v, rfile_t *fp);

/*
 * Common data for both PC and NPC.
 * PC/NPC specific data is kept right after this struct
 * (Use PC(ch) and NPC(ch) macros to obtain it)
 */
struct char_data
{
	/*
	 * pointer to mob index
	 * IS_NPC relies on this pointer set to NULL or non NULL
	 */
	MOB_INDEX_DATA *	pMobIndex;

	DESCRIPTOR_DATA *	desc;

	CHAR_DATA * 		next;
	CHAR_DATA * 		next_in_room;

	CHAR_DATA * 		master;
	CHAR_DATA * 		leader;

	CHAR_DATA * 		fighting;
	time_t			last_fight_time;
	time_t			last_death_time;

	AFFECT_DATA *		affected;
	varr			sk_affected;
	OBJ_DATA *		carrying;
	OBJ_DATA *		on;
	ROOM_INDEX_DATA *	in_room;
	const char *		name;
	mlstring		short_descr;
	mlstring		long_descr;
	mlstring		description;
	flag32_t		sex;
	const char *		class;
	const char *		race;
	const char *		clan;
	int			ethos;
	int			level;
	int			wait;
	int 			add_level;
	int			daze;

	int			hit;
	int 			max_hit;
	int 			perm_hit;

	int 			mana;
	int 			max_mana;
	int 			perm_mana;

	int 			move;
	int 			max_move;
	int 			perm_move;

	int			gold;
	int			silver;
	flag64_t		comm;	/* RT added to pad the vector */
	flag32_t		imm_flags;
	flag32_t		res_flags;
	flag32_t		vuln_flags;
	int			invis_level;
	int			incog_level;
	flag64_t		affected_by;
	flag32_t		position;
	int			carry_weight;
	int			carry_number;
	int			saving_throw;
	int			alignment;
	int			hitroll;
	int			damroll;
	const char *		damtype;
	int			armor[4];
	int			wimpy;

	/* stats */
	int			perm_stat[MAX_STATS];
	int			mod_stat[MAX_STATS];

	/* parts stuff */
	flag32_t		form;
	flag32_t		parts;
	flag32_t		size;
	const char *		material;

	bool			riding;		/* mount data */
	CHAR_DATA *		mount;

	CHAR_DATA *		hunting;	/* hunt data */
	int 			endur;

	CHAR_DATA *		doppel;		/* doppelganger and mirror */

	int			slang;		/* spoken language */
};

/*
 * NPC-specific data
 */
struct npc_data {
	int		mprog_delay;
	CHAR_DATA *	mprog_target;
	struct {
		int dice_number;
		int dice_type;
	} dam;
	AREA_DATA * 	zone;
	CHAR_DATA *	target;
	const char *	in_mind;
	CHAR_DATA * 	last_fought;
	CHAR_DATA *	hunter;		/* who quested to slay */
};

/*
 * PC-specific data
 */
struct pc_data
{
	BUFFER *		buffer;
	const char *		pwd;
	const char *		bamfin;
	const char *		bamfout;
	const char *		title;
	const char *		twitlist;
	const char *		granted;	/* granted wiz commands */
	const char *		form_name;
	const char *		enemy_list;
	time_t			last_note;
	time_t			last_idea;
	time_t			last_penalty;
	time_t			last_news;
	time_t			last_changes;
	time_t			last_offence;
	flag32_t		trust;
	flag32_t		plr_flags;
	flag32_t		wiznet;		/* wiz stuff */
	int			condition	[MAX_COND];
	varr			learned;	/* pc_skill_t */
	varr			specs;		/* spec names */
	int 			security;	/* OLC */ /* Builder security */
	int			bank_s;
	int			bank_g;
	int 			death;
	int 			anti_killed;
	int 			has_killed;
	
	int			questgiver;
	int	 		questpoints;
	int			questtime;
	int			questobj;
	int			questmob;
	qtrouble_t *		qtrouble;
	ROOM_INDEX_DATA *	questroom;

	const char *		race;
	int			pc_killed;

	int			clan_status;
	const char *		petition;

	int			plevels;	/* penalty levels */

	int			hometown;
	ROOM_INDEX_DATA	*	homepoint;

	const char *		wanted_by;
	CHAR_DATA * 		reply;		/* for do_reply */
	note_t * 		pnote;

	int 			exp;		/* total exp */
	int			practice;
	int			train;

	time_t			logon;
	time_t			logoff;

	int 			played;
	int			add_age;

	int			idle_timer;

	int	 		religion;

	ROOM_INDEX_DATA *	was_in_room;
	CHAR_DATA * 		pet;
	CHAR_DATA *	 	guarding;
	CHAR_DATA * 		guarded_by;
	int			version;

	dvdata_t *		dvdata;
};

/*
 * PC learned skill
 */
typedef struct pc_skill_t pc_skill_t;
struct pc_skill_t {
	const char *sn;	/* skill name				*/
	int percent;	/* skill percentage			*/
};

pc_skill_t *pc_skill_lookup(CHAR_DATA *ch, const char *sn);

void pc_skill_init(pc_skill_t *);

/*
 * Extra description data for a room or object.
 */
struct ed_data
{
	ED_DATA *	next;		/* Next in list 	    */
	const char *	keyword;	/* Keyword in look/examine  */
	mlstring	description;	/* What to see		    */
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
 * Prototype for an object.
 */
struct obj_index_data
{
	OBJ_INDEX_DATA *	next;
	ED_DATA *		ed;
	AFFECT_DATA *		affected;
	const char *		name;
	mlstring		short_descr;
	mlstring		description;
	int			vnum;
	int			reset_num;
	const char *		material;
	flag32_t		item_type;
	flag64_t		extra_flags;
	flag32_t		wear_flags;
	int			level;
	int			condition;
	int			count;
	int			weight;
	uint 			cost;
	vo_t 			value[5];
	int 			limit;
	OPROG_FUN **		oprogs;
	flag32_t		gender;
	varr			restrictions; /* varr of cc_ruleset_t */
};

/*
 * One object.
 */
struct obj_data
{
	OBJ_DATA *		next;
	OBJ_DATA *		next_content;
	OBJ_DATA *		contains;
	OBJ_DATA *		in_obj;
	OBJ_DATA *		on;
	CHAR_DATA * 		carried_by;
	CHAR_DATA *		last_owner;
	ED_DATA *		ed;
	AFFECT_DATA *		affected;
	OBJ_INDEX_DATA *	pObjIndex;
	ROOM_INDEX_DATA *	in_room;
	const char *		name;
	mlstring		short_descr;
	mlstring		description;
	flag32_t 		extra_flags;
	flag32_t 		wear_flags;
	flag32_t		wear_loc;
	int			weight;
	uint 			cost;
	int			level;
	int			condition;
	const char *		material;
	int			timer;
	vo_t 			value[5];
	int 			progtypes;
	mlstring		owner;
	altar_t *		altar;
	int 			water_float;
};

/*
 * Exit data.
 */
struct exit_data
{
	vo_t		to_room;
	flag32_t	exit_info;
	flag32_t	size;
	int		key;
	const char *	keyword;
	mlstring	description;
	EXIT_DATA *	next;		/* OLC */
	flag32_t	rs_flags;	/* OLC */
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
	const char *	clan;
	const char *	builders;	/* name list of builders	*/
	int		vnum;		/* area vnum			*/
	flag32_t	area_flags;
	int		security;	/* area security (1..9)		*/
	uint		count;
	mlstring	resetmsg;	/* reset message		*/
};

struct room_history_data
{
	const char *name;
	int went;
	ROOM_HISTORY_DATA *next;
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
	mlstring		name;
	mlstring		description;
	int			vnum;
	flag32_t		room_flags;
	int			light;
	flag32_t		sector_type;
	int			heal_rate;
	int			mana_rate;
	ROOM_HISTORY_DATA * 	history;
	AFFECT_DATA *		affected;
	flag32_t		affected_by;
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
#define TRIG_REGEXP	(B)

struct mptrig
{
	int		type;
	const char *	phrase;
	flag32_t	mptrig_flags;
	int		vnum;		/* mob prog code vnum */
	void *		extra;
	MPTRIG * 	next;
};

struct mpcode
{
	int		vnum;
	const char *	code;
	MPCODE *	next;
};

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

/*
 * Utility macros.
 */
#define IS_WATER(var) 	(((var)->sector_type == SECT_WATER_SWIM) || \
				 ((var)->sector_type == SECT_WATER_NOSWIM))
#define PERCENT(cur, max)	(max==0?0:((cur)*100)/(max))

/*
 * Character macros.
 */
#define IS_NPC(ch)		(ch->pMobIndex != NULL)
#define IS_IMMORTAL(ch) 	(!IS_NPC(ch) && (ch)->level >= LEVEL_IMMORTAL)
#define IS_HERO(ch)		(!IS_NPC(ch) && (ch)->level >= LEVEL_HERO)

int trust_level(CHAR_DATA *ch);
#define GET_ORIGINAL(ch)	(ch->desc && ch->desc->original ?	\
				 	ch->desc->original : ch)
#define IS_TRUSTED(ch, lev)	(trust_level(ch) >= (lev))

#define IS_AFFECTED(ch, bit)	(IS_SET((ch)->affected_by, (bit)))

#define ORG_RACE(ch)		(IS_NPC(ch) ? (ch)->pMobIndex->race : PC(ch)->race)

#define LEVEL(ch)		((ch)->level + (ch)->add_level)

#define GET_EXP(ch)		(IS_NPC(ch) ? 0 : PC(ch)->exp)
#define GET_PET(ch)		(IS_NPC(ch) ? NULL : PC(ch)->pet)
#define GET_LANG(ch)		(ch->desc ? ch->desc->dvdata->lang : 0)
#define GET_RELIGION(ch)	(IS_NPC(ch) ? 0 : PC(ch)->religion)

#define IS_GOOD(ch)		((ch)->alignment >= 350)
#define IS_EVIL(ch)		((ch)->alignment <= -350)
#define IS_NEUTRAL(ch)		(!IS_GOOD(ch) && !IS_EVIL(ch))

#define RALIGN(ch)	(IS_GOOD(ch) ? RA_GOOD :	\
			 IS_EVIL(ch) ? RA_EVIL :	\
				       RA_NEUTRAL)
#define NALIGN(ch)	(IS_GOOD(ch) ? ANUM_GOOD :	\
			 IS_EVIL(ch) ? ANUM_EVIL :	\
				       ANUM_NEUTRAL)

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

#define COINS_WEIGHT(gold, silver) ((silver) / 10 + (gold) * 2 / 5)
#define get_carry_weight(ch)	((ch)->carry_weight +			\
				 COINS_WEIGHT((ch)->silver, (ch)->gold))
#define MONEY_WEIGHT(obj)	COINS_WEIGHT(INT_VAL(obj->value[0]),	\
					     INT_VAL(obj->value[1]))

#define HAS_TRIGGER(ch,trig)	(IS_SET((ch)->pMobIndex->mptrig_types, (trig)))
#define IS_SWITCHED( ch )       (ch->desc && ch->desc->original)
#define IS_BUILDER(ch, Area)	(!IS_NPC(ch) && !IS_SWITCHED(ch) &&	      \
				 (PC(ch)->security >= (Area)->security || \
				  _is_name(ch->name, Area->builders, str_cmp)))

#define MOUNTED(ch)	((!IS_NPC(ch) && ch->mount && ch->riding) ? \
				ch->mount : NULL)
#define RIDDEN(ch)	((IS_NPC(ch) && ch->mount && ch->riding) ? \
				ch->mount : NULL)
#define IS_DRUNK(ch)	(!IS_NPC(ch) && PC(ch)->condition[COND_DRUNK] > 10)

#define IS_EXTRACTED(ch)	(!mem_is(ch, MT_CHAR) || \
				 ch->last_death_time >= current_time)
/*
 * Object macros.
 */
#define CAN_WEAR(obj, part)	(IS_SET((obj)->wear_flags,  (part)))
#define IS_OBJ_STAT(obj, stat)	(IS_SET((obj)->extra_flags, (stat)))
#define IS_WEAPON_STAT(obj,stat)(IS_SET(INT_VAL((obj)->value[4]), (stat)))
#define WEIGHT_MULT(obj)	((obj)->pObjIndex->item_type == ITEM_CONTAINER ? \
	INT_VAL((obj)->value[4]) : 100)

/*
 * Description macros.
 */
/* new defines */
#define MAX_CHARM(ch)	((get_curr_stat(ch,STAT_INT) / 4) + (ch->level / 30))

/*
 * Global constants.
 */
extern	const	struct str_app_type	str_app 	[26];
extern	const	struct int_app_type	int_app 	[26];
extern	const	struct wis_app_type	wis_app 	[26];
extern	const	struct dex_app_type	dex_app 	[26];
extern	const	struct con_app_type	con_app 	[26];

extern	const	struct wiznet_type	wiznet_table	[];
extern	const	struct spec_type	spec_table	[];

/*
 * Global variables.
 */
extern		SHOP_DATA	  *	shop_first;

extern		CHAR_DATA	  *	char_list;
extern		CHAR_DATA	  *	char_list_lastpc;
#define npc_list (char_list_lastpc ? char_list_lastpc->next : char_list)

extern		DESCRIPTOR_DATA   *	descriptor_list;
extern		DESCRIPTOR_DATA   *	descriptor_free;

extern		OBJ_DATA	  *	object_list;

extern		MPCODE	 	 *	mpcode_list;

extern		ROOM_INDEX_DATA   *	top_affected_room;

extern		char			bug_buf 	[];
extern		time_t			current_time;
extern		bool			fLogAll;
extern		char			log_buf 	[];
extern		TIME_INFO_DATA		time_info;
extern		WEATHER_DATA		weather_info;
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

void	handle_death(CHAR_DATA *ch, CHAR_DATA *victim);

ROOM_INDEX_DATA  *get_random_room(CHAR_DATA *ch, AREA_DATA *area);

/* effect.c */
void	acid_effect	(void *vo, int level, int dam);
void	cold_effect	(void *vo, int level, int dam);
void	fire_effect	(void *vo, int level, int dam);
void	poison_effect	(void *vo, int level, int dam);
void	shock_effect	(void *vo, int level, int dam);
void	sand_effect	(void *vo, int level, int dam);
void	scream_effect	(void *vo, int level, int dam);

/* handler.c */
const char *get_stat_alias(CHAR_DATA *ch, int stat);
int	count_users	(OBJ_DATA *obj);
void	deduct_cost	(CHAR_DATA *ch, uint cost);
int	check_immune	(CHAR_DATA *ch, int dam_class);
int	check_exit	(const char *arg);

int	get_hours	(CHAR_DATA *ch);
int	get_age 	(CHAR_DATA *ch);

int	get_curr_stat	(CHAR_DATA *ch, int stat);
int	get_max_train	(CHAR_DATA *ch, int stat);
int	can_carry_n	(CHAR_DATA *ch);
int	can_carry_w	(CHAR_DATA *ch);
int	age_to_num	(int);

bool	pc_name_ok	(const char *name);
void	char_from_room	(CHAR_DATA *ch);
void	char_to_room	(CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex);
void	obj_to_char	(OBJ_DATA *obj, CHAR_DATA *ch);
void	obj_from_char	(OBJ_DATA *obj);
int	apply_ac	(OBJ_DATA *obj, int iWear, int type);
OBJ_DATA *	get_eq_char	(CHAR_DATA *ch, int iWear);
void		_equip_char	(CHAR_DATA *ch, OBJ_DATA *obj);
OBJ_DATA *	equip_char	(CHAR_DATA *ch, OBJ_DATA *obj, int iWear);
void	unequip_char	(CHAR_DATA *ch, OBJ_DATA *obj);
int	count_obj_list	(OBJ_INDEX_DATA *obj, OBJ_DATA *list);
void	obj_from_room	(OBJ_DATA *obj);
void	obj_to_room	(OBJ_DATA *obj, ROOM_INDEX_DATA *pRoomIndex);
void	obj_to_obj	(OBJ_DATA *obj, OBJ_DATA *obj_to);
void	obj_from_obj	(OBJ_DATA *obj);

/* extract obj flags */
#define XO_F_NOCOUNT	(A)	/* do not update obj count		*/
#define XO_F_NORECURSE	(B)	/* do not extract contained in objs	*/
#define XO_F_NOCHQUEST	(C)	/* do not check for chquest objs	*/
#define XO_F_NUKE	(D)	/* fast extract (used in char_free)
				   does not call obj_from_xxx
				   and is always recursive		*/

/* quit_char/extract_char */
#define XC_F_COUNT	(A)	/* update obj count			*/
#define XC_F_INCOMPLETE	(B)	/* do not extract char from char_list	*/

/*
 * quit_char assumes !IS_NPC(ch)
 */
void	extract_obj(OBJ_DATA *obj, int flags);
void	extract_char(CHAR_DATA *ch, int flags);
void	quit_char(CHAR_DATA *ch, int flags);

CHAR_DATA *	get_char_room	(CHAR_DATA *ch, const char *argument);
CHAR_DATA *	get_char_world	(CHAR_DATA *ch, const char *argument);
CHAR_DATA *	get_char_area	(CHAR_DATA *ch, const char *argument);
OBJ_DATA *	get_obj_type	(OBJ_INDEX_DATA *pObjIndexData);
OBJ_DATA *	get_obj_list	(CHAR_DATA *ch, const char *argument,
			    OBJ_DATA *list);
OBJ_DATA *	get_obj_carry	(CHAR_DATA *ch, const char *argument);
OBJ_DATA *	get_obj_wear	(CHAR_DATA *ch, const char *argument);
OBJ_DATA *	get_obj_here	(CHAR_DATA *ch, const char *argument);
OBJ_DATA *	get_obj_room	(CHAR_DATA *ch, const char *argument);
OBJ_DATA *	get_obj_world	(CHAR_DATA *ch, const char *argument);
OBJ_DATA *	create_money	(int gold, int silver);
int	get_obj_number	(OBJ_DATA *obj);
int	get_obj_realnumber	(OBJ_DATA *obj);
int	get_obj_weight	(OBJ_DATA *obj);
int	get_true_weight (OBJ_DATA *obj);
bool	room_is_dark	(CHAR_DATA *ch);
bool	room_dark	(ROOM_INDEX_DATA *pRoomIndex);
int	isn_dark_safe	(CHAR_DATA *ch);
bool	room_is_private (ROOM_INDEX_DATA *pRoomIndex);
bool	can_see 	(CHAR_DATA *ch, CHAR_DATA *victim);
bool	can_see_obj	(CHAR_DATA *ch, OBJ_DATA *obj);
bool	can_see_room	(CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex);
bool	can_drop_obj	(CHAR_DATA *ch, OBJ_DATA *obj);
void	room_record	(const char *name, ROOM_INDEX_DATA *room,int door);
int	count_charmed	(CHAR_DATA *ch);
void	damage_to_obj	(CHAR_DATA *ch, OBJ_DATA *wield, OBJ_DATA *worn,
			int damage);
bool	make_eq_damage	(CHAR_DATA *ch, CHAR_DATA *victim,
			int loc_wield, int loc_destroy);
bool	random_eq_damage(CHAR_DATA *ch, CHAR_DATA *victim,
			int loc_wield);

/*
 * the followind three functions assume IS_NPC(ch)
 */
void	add_mind	(CHAR_DATA *ch, const char *str);
void	remove_mind	(CHAR_DATA *ch, const char *str);
void	back_home	(CHAR_DATA *ch);

CHAR_DATA*	find_char	(CHAR_DATA *ch, const char *argument, int door, int range);
CHAR_DATA*	get_char_spell	(CHAR_DATA *ch, const char *argument, int *door, int range);
void	path_to_track	(CHAR_DATA *ch, CHAR_DATA *victim, int door);
bool	in_PK(CHAR_DATA *ch, CHAR_DATA *victim);
bool	can_gate(CHAR_DATA *ch, CHAR_DATA *victim);
void	transfer_char(CHAR_DATA *ch, CHAR_DATA *vch,
		      ROOM_INDEX_DATA *to_room,
		      const char *msg_out,
		      const char *msg_travel,
		      const char *msg_in);
void	look_at(CHAR_DATA *ch, ROOM_INDEX_DATA *room);

void	recall(CHAR_DATA *ch, ROOM_INDEX_DATA *room);

bool	saves_spell	(int level, CHAR_DATA *victim, int dam_class);
bool	check_dispel	(int dis_level, CHAR_DATA *victim, const char *sn);
bool	saves_dispel	(int dis_level, int spell_level, int duration);

extern const char *target_name;

bool	spellbane	(CHAR_DATA *bch, CHAR_DATA *ch,
			 int bane_chance, int bane_damage);
bool	check_trust	(CHAR_DATA *ch, CHAR_DATA *victim);
void	obj_cast_spell	(const char *sn, int level, CHAR_DATA *ch, void *vo);
void	spellfun_call	(const char *sn_fun,
			 const char *sn, int level, CHAR_DATA *ch, void *vo);

bool remove_obj (CHAR_DATA * ch, int iWear, bool fReplace);
void wear_obj   (CHAR_DATA * ch, OBJ_DATA * obj, bool fReplace);
int free_hands	(CHAR_DATA * ch);

bool	check_blind	(CHAR_DATA *ch);
bool	check_blind_raw	(CHAR_DATA *ch);

void	show_affects(CHAR_DATA *ch, BUFFER *output);

void	set_leader(CHAR_DATA *ch, CHAR_DATA *lch);

/* interp.c */
void	interpret	(CHAR_DATA *ch, const char *argument);
void	interpret_raw	(CHAR_DATA *ch, const char *argument, bool is_order);
uint	number_argument (const char *argument, char *arg, size_t len);
uint	mult_argument	(const char *argument, char *arg, size_t len);
const char *	one_argument	(const char *argument, char *arg_first, size_t);
const char *	first_arg	(const char *argument, char *arg_first, size_t,
				 bool fCase);

/*
 * victim is assumed to be !IS_NPC
 */
void		delete_player	(CHAR_DATA *victim, char* msg);

/* special.c */
SPEC_FUN *	mob_spec_lookup	(const char *name);
char *		mob_spec_name(SPEC_FUN *function);

RESET_DATA *	reset_new	(void);
void		reset_free	(RESET_DATA *pReset);
void		reset_add	(RESET_DATA *pReset, ROOM_INDEX_DATA *pRoom,
				 int num);

AREA_DATA	*new_area		(void);
AREA_DATA *	area_lookup		(int vnum);
AREA_DATA *	area_vnum_lookup	(int vnum);
void		free_area		(AREA_DATA *pArea);

#define TOUCH_AREA(pArea)						\
	{								\
		if (pArea != NULL)					\
			SET_BIT((pArea)->area_flags, AREA_CHANGED);	\
	}
#define TOUCH_VNUM(vnum)	TOUCH_AREA(area_vnum_lookup(vnum))

EXIT_DATA	*new_exit		(void);
void		free_exit		(EXIT_DATA *pExit);
ROOM_INDEX_DATA *new_room_index		(void);
void		free_room_index		(ROOM_INDEX_DATA *pRoom);
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
void		mptrig_fix		(MOB_INDEX_DATA *mob);
void            mptrig_free		(MPTRIG *mptrig);

extern MPCODE *	mpcode_list;
MPCODE *	mpcode_new		(void);
void		mpcode_add		(MPCODE *mpcode);
MPCODE *	mpcode_lookup		(int vnum);
void		mpcode_free		(MPCODE *mpcode);

/* extra descr recycling */
ED_DATA	*ed_new		(void);
ED_DATA *ed_new2	(const ED_DATA *ed, const char* name);
ED_DATA *ed_dup		(const ED_DATA *ed);
void	ed_fread	(rfile_t *fp, ED_DATA **edp);
void	ed_fwrite	(FILE *fp, ED_DATA *ed);
void	ed_free		(ED_DATA *ed);

/* event function lists */
event_fun_t *	evf_new();
void		evf_free(event_fun_t *evf);

/* object recycling */
OBJ_DATA *	new_obj	(void);
void		free_obj(OBJ_DATA *obj);

extern int obj_count;
extern int obj_free_count;

HELP_DATA *	help_new	(void);
void		help_add	(AREA_DATA*, HELP_DATA*);
HELP_DATA *	help_lookup	(int num, const char *keyword);
void		help_show	(CHAR_DATA *ch, BUFFER *output,
				 const char *keyword);
void		help_free	(HELP_DATA*);

void		boot_db_system	(void);
void		boot_db		(void);

CHAR_DATA *	create_mob	(MOB_INDEX_DATA *pMobIndex);
CHAR_DATA *	create_mob_of	(MOB_INDEX_DATA *pMobIndex, mlstring *owner);
void		clone_mob	(CHAR_DATA *parent, CHAR_DATA *clone);

/* create_obj flags */
#define CO_F_NOCOUNT	(A)	/* do not update obj count */

OBJ_DATA *	create_obj	(OBJ_INDEX_DATA *pObjIndex, int flags);
OBJ_DATA *	create_obj_of	(OBJ_INDEX_DATA *pObjIndex, mlstring *owner);
void		clone_obj	(OBJ_DATA *parent, OBJ_DATA *clone); 

void	clear_char	(CHAR_DATA *ch);
ED_DATA * ed_lookup(const char *name, ED_DATA *ed);
MOB_INDEX_DATA *	get_mob_index	(int vnum);
OBJ_INDEX_DATA *	get_obj_index	(int vnum);
ROOM_INDEX_DATA *	get_room_index	(int vnum);
int	number_fuzzy	(int number);
int	number_range	(int from, int to);
int	number_percent	(void);
int	number_door	(void);
int	number_bits	(int width);
long	number_mm	(void);
int	dice		(int number, int size);
int	interpolate	(int level, int value_00, int value_32);
char *	capitalize	(const char *str);
void	append_file	(CHAR_DATA *ch, const char *file, const char *str);
void	tail_chain	(void);
char *format_flags(flag64_t flags);

#define chance(num) (number_range(1, 100) <= num)

/*
 * Global Constants
 */
extern	const char *			dir_name	[];
extern	const char *			from_dir_name	[];
extern	const	int			rev_dir		[];
extern	const	struct spec_type	spec_table	[];
extern	char				DEFAULT_PROMPT	[];

void		add_follower	(CHAR_DATA *ch, CHAR_DATA *master);
void		stop_follower	(CHAR_DATA *ch);

void		check_sex	(CHAR_DATA *ch);

/*
 * ch is assumed to be !IS_NPC
 */
void		nuke_pets	(CHAR_DATA *ch);
void		die_follower	(CHAR_DATA *ch);
void		do_afk		(CHAR_DATA *ch, const char *argument);
void		do_lang		(CHAR_DATA *ch, const char *argument);
void		do_music	(CHAR_DATA *ch, const char *argument);
void		do_gossip	(CHAR_DATA *ch, const char *argument);
CHAR_DATA*	leader_lookup	(CHAR_DATA *ch);
const char *	garble		(CHAR_DATA *ch, const char *txt);
void 		do_tell_raw	(CHAR_DATA *ch, CHAR_DATA *victim,
				 const char *msg);
#define	is_same_group(ach, bch) (leader_lookup(ach) == leader_lookup(bch))
void	do_who_raw	(CHAR_DATA *ch, CHAR_DATA *vch, BUFFER *output);

void move_char(CHAR_DATA *ch, int door, bool follow);
bool move_char_org(CHAR_DATA *ch, int door, bool follow, bool is_charge);
bool guild_ok(CHAR_DATA *ch, ROOM_INDEX_DATA *room);
int mount_success(CHAR_DATA *ch, CHAR_DATA *mount, int canattack);
int	find_door	(CHAR_DATA *ch, char *arg);

bool can_loot		(CHAR_DATA *ch, OBJ_DATA *obj);
void get_obj		(CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *container,
			 const char *msg_others);
void wear_obj		(CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace);
void quaff_obj		(CHAR_DATA *ch, OBJ_DATA *obj);
void set_title		(CHAR_DATA *ch, const char *argument);

/* act_wiz.h */
void wiznet(const char *msg, CHAR_DATA *ch, const void *arg,
	    flag32_t flag, flag32_t flag_skip, int min_level);
void reboot_mud(void);
ROOM_INDEX_DATA *find_location(CHAR_DATA *ch, const char *argument);

void substitute_alias(DESCRIPTOR_DATA *d, const char *argument);
const char *get_cond_alias(OBJ_DATA *obj);

/*
 * `advance' and `gain_exp' assume !IS_NPC(victim)
 */
void advance		(CHAR_DATA *ch, int level);
void gain_exp		(CHAR_DATA *ch, int gain);
void delevel		(CHAR_DATA *ch);

bool (*olc_interpret)(DESCRIPTOR_DATA *d, const char *argument);

void	gain_condition	(CHAR_DATA *ch, int iCond, int value);
void	update_handler	(void);
void	char_update	(void);
void	area_update	(void);
void	room_update	(void);
void	track_update	(void);
void	obj_update	(void);
void *	clan_item_update_cb(void *p, va_list ap);
void	weather_update	(void);

flag32_t wiznet_lookup	(const char *name);

#endif

