/*
 * $Id: merc.h,v 1.363 2001-09-12 12:32:18 fjoe Exp $
 */

/***************************************************************************
 *     ANATOLIA 2.1 is copyright 1996-1997 Serdar BULUT, Ibrahim CANPUNAR  *
 *     ANATOLIA has been brought to you by ANATOLIA consortium		   *
 *	 Serdar BULUT {Chronos}		bulut@rorqual.cc.metu.edu.tr	   *
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

#include <typedef.h>

/*
 * sex or gender
 */
#define SEX_NEUTRAL	0
#define SEX_MALE	1
#define SEX_FEMALE	2
#define SEX_EITHER	3	/* used only for NPC, means random sex	*/
				/* in create_mob			*/
#define SEX_PLURAL	4

/* align numbers */
#define AN_GOOD		0
#define AN_NEUTRAL	1
#define AN_EVIL		2
#define MAX_AN		3

enum {
	STAT_STR,
	STAT_INT,
	STAT_WIS,
	STAT_DEX,
	STAT_CON,
	STAT_CHA,

	MAX_STAT			/* number of char stats */
};

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

/* basic types */
#include <buffer.h>
#include <str.h>
#include <mlstring.h>
#include <varr.h>
#include <hash.h>
#include <strkey_hash.h>
#include <flag.h>
#include <cmd.h>

#include <tables.h>
#include <hometown.h>

/* utils */
#include <log.h>
#include <util.h>
#include <memalloc.h>

#include <race.h>
#include <class.h>
#include <clan.h>
#include <damtype.h>
#include <material.h>
#include <liquid.h>
#include <forms.h>
#include <vo_iter.h>
#include <trig.h>

#define PFILE_VERSION	12
#define AREA_VERSION	7

/*
 * Game parameters.
 */
#define MAX_INPUT_LENGTH	1024
#define MAX_PROMPT_LENGTH	60
#define MAX_TITLE_LENGTH	45
#define MAX_CHAR_NAME		12
#define DEFAULT_PAGELEN		22
#define MIN_PAGELEN		16
#define MAX_PAGELEN		192
#define MAX_RELIGION		18
#define MAX_NEWBIES		120	/* number of newbies allowed */
#define MAX_OLDIES		999	/* number of oldies allowed */
#define MAX_TRADE		5	/* number of trade types for shops */
#define MAX_DIR			6	/* number of exits */
#define MAX_ALIAS		50	/* number of aliases char can have */
#define MAX_KEY_HASH		1024

/*
 * level consts
 */
#define MAX_LEVEL		50

#define LEVEL_IMP		MAX_LEVEL	/* implementor		*/
#define LEVEL_CRE		(MAX_LEVEL - 1)	/* creator		*/
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
#define OFFENCE_DELAY_TIME	300
#define GHOST_DELAY_TIME	600
#define MISSING_TARGET_DELAY	10

/*
 * vnum globals
 * skill/spell specific vnums should not be defined here
 */
#define MOB_VNUM_SHADOW			10
#define MOB_VNUM_STALKER		15
#define MOB_VNUM_COCOON			27
#define MOB_VNUM_BONE_DRAGON		28

#define OBJ_VNUM_SILVER_ONE		1
#define OBJ_VNUM_GOLD_ONE		2
#define OBJ_VNUM_GOLD_SOME		3
#define OBJ_VNUM_SILVER_SOME		4
#define OBJ_VNUM_COINS			5
#define OBJ_VNUM_CORPSE_NPC		10
#define OBJ_VNUM_CORPSE_PC		11
#define OBJ_VNUM_SEVERED_HEAD		12
#define OBJ_VNUM_TORN_HEART		13
#define OBJ_VNUM_SLICED_ARM		14
#define OBJ_VNUM_SLICED_LEG		15
#define OBJ_VNUM_GUTS			16
#define OBJ_VNUM_BRAINS			17
#define OBJ_VNUM_POTION_VIAL		42
#define OBJ_VNUM_RENEGADE_MARK		76
#define OBJ_VNUM_MAGIC_JAR		93

#define ROOM_VNUM_LIMBO			2
#define ROOM_VNUM_TEMPLE		3001

/*
 * configuration parameters
 */
extern varr control_sockets;
extern varr info_sockets;
extern varr info_trusted;

/* mud server options (etc/system.conf) */
#define OPT_ASCII_ONLY_NAMES	(A)

extern flag_t mud_options;

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
#define SUN_SET			    3

#define SKY_CLOUDLESS		    0
#define SKY_CLOUDY		    1
#define SKY_RAINING		    2
#define SKY_LIGHTNING		    3

struct time_info_data
{
	int	hour;
	int	day;
	int	month;
	int	year;
};

struct weather_data
{
	int	mmhg;
	int	change;
	int	sky;
	int	sunlight;
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
	size_t	size;
	size_t	top;
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

	uint		lang;		/* interface language	*/
	int		pagelen;	/* pager lines		*/
	flag_t		olc_flags;	/* olc flags		*/

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
	CHAR_DATA *		character;
	CHAR_DATA *		original;
	const char *		host;
	const char *		ip;
	int			descriptor;
	int			connected;
	int			wait_for_se;
	bool			fcommand;
	char			inbuf		[4 * MAX_INPUT_LENGTH];
	char			incomm		[MAX_INPUT_LENGTH];
	char			inlast		[MAX_INPUT_LENGTH];
	int			repeat;
	outbuf_t		out_buf;
	outbuf_t		snoop_buf;
	const char *		showstr_head;
	const char *		showstr_point;
	uint			codepage;

/* OLC stuff */
	olced_t	*		olced;
	void *			pEdit;		/* edited obj	*/
	void *			pEdit2;		/* edited obj 2	*/

/* string editor stuff */
	const char **		pString;	/* edited string	*/
	const char *		backup;		/* backup		*/

	dvdata_t *		dvdata;
};

DESCRIPTOR_DATA *	new_descriptor(int fd);
void			free_descriptor(DESCRIPTOR_DATA *d);

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
	const char *	keyword;
	mlstring	text;
};

/*
 * Shop types.
 */

struct shop_data
{
	SHOP_DATA * next;		/* Next shop in list		*/
	int	keeper;			/* Vnum of shop keeper mob	*/
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
	flag_t		flag;
	int		level;
};

struct spec_type
{
	const char *		name;		/* special function name */
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

#define GROUP_BENEDICTIONS	(D)
#define GROUP_COMBAT		(E)
#define GROUP_CREATION		(F)
#define GROUP_CURATIVE		(G)
#define GROUP_DIVINATION	(H)
#define GROUP_DRACONIAN		(I)
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

#define GROUP_MEDITATION	(U)
#define GROUP_CLAN		(V)
#define GROUP_DEFENSIVE		(W)
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
#define AREA_CLOSED		(B)	/* transportation does not work,*/
					/* no quests			*/
#define AREA_NOQUEST		(C)	/* no quests in this area	*/
#define AREA_UPDATE_ALWAYS	(D)	/* always update area (even if  */
					/* there are players in it)	*/
#define AREA_UPDATE_FREQUENTLY	(E)	/* update area more frequently	*/
					/* (every 3 ticks)		*/
#define AREA_TAGGED		(Y)	/* area is tagged (for		*/
					/* conversion or whatever)	*/
#define AREA_CHANGED		(Z)	/* area has been modified	*/

/*
 * ACT bits for mobs.  *ACT*
 * Used in #MOBILES.
 */
#define ACT_FAMILIAR		(A)		/* familiar		*/
#define ACT_SENTINEL		(B)		/* Stays in one room	*/
#define ACT_SCAVENGER		(C)		/* Picks up objects	*/
#define ACT_SUMMONED		(D)
#define ACT_IMMSTEAL		(E)
#define ACT_AGGRESSIVE		(F)		/* Attacks PC's		*/
#define ACT_STAY_AREA		(G)		/* Won't leave area	*/
#define ACT_WIMPY		(H)
#define ACT_PET			(I)
#define ACT_IMMSUMMON		(J)
#define ACT_HUNTER		(L)
#define ACT_CLERIC		(Q)
#define ACT_MAGE		(R)
#define ACT_THIEF		(S)
#define ACT_WARRIOR		(T)
#define ACT_NOALIGN		(U)
#define ACT_NOPURGE		(V)
#define ACT_OUTDOORS		(W)
#define ACT_INDOORS		(Y)
#define ACT_RIDEABLE		(Z)
#define ACT_UPDATE_ALWAYS	(cc)
#define ACT_NOTRACK		(ee)

/* mob_index_data->mob_flags */
#define MOB_CHANGER		(A)
#define MOB_GAIN		(B)
#define MOB_TRAIN		(C)		/* Can train PC's	*/
#define MOB_PRACTICE		(D)		/* Can practice PC's	*/
#define MOB_QUESTOR		(E)
#define MOB_REPAIRMAN		(F)
#define MOB_SAGE		(G)		/* sage (Otho etc.)	*/
#define MOB_HEALER		(H)
#define MOB_CLAN_GUARD		(I)
#define MOB_JANITOR		(J)

#define MOB_IS(mob, f)		(IS_SET((mob)->pMobIndex->mob_flags, (f)))

/* OFF bits for mobiles *OFF  */
#define OFF_AREA_ATTACK		(A)
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
#define FORM_INTANGIBLE		(L)

#define FORM_BIPED		(M)
#define FORM_CENTAUR		(N)
#define FORM_INSECT		(O)
#define FORM_SPIDER		(P)
#define FORM_CRUSTACEAN		(Q)
#define FORM_WORM		(R)
#define FORM_BLOB		(S)

#define FORM_MAMMAL		(V)
#define FORM_BIRD		(W)
#define FORM_REPTILE		(X)
#define FORM_SNAKE		(Y)
#define FORM_DRAGON		(Z)
#define FORM_AMPHIBIAN		(aa)
#define FORM_FISH		(bb)
#define FORM_COLD_BLOOD		(cc)

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
#define AFF_SCREAM		(B)
#define AFF_BLOODTHIRST		(C)
#define AFF_STUN		(D)
#define AFF_WEAK_STUN		(E)
#define AFF_FEAR		(F)
#define AFF_WEB			(G)
#define AFF_SANCTUARY		(H)
#define AFF_FAERIE_FIRE		(I)
#define AFF_BLACK_SHROUD	(J)
#define AFF_CURSE		(K)
#define AFF_CORRUPTION		(L)
#define AFF_POISON		(M)
#define AFF_PROTECT_EVIL	(N)
#define AFF_PROTECT_GOOD	(O)
#define AFF_TURNED		(P)/* Character is turned into other creature */
#define AFF_SLEEP		(R)
#define AFF_CHARM		(S)
#define AFF_FLYING		(T)
#define AFF_PASS_DOOR		(U)
#define AFF_HASTE		(V)
#define AFF_CALM		(W)
#define AFF_PLAGUE		(X)
#define AFF_WEAKEN		(Y)
#define AFF_BERSERK		(aa)
#define AFF_REGENERATION	(cc)
#define AFF_SLOW		(dd)
#define AFF_QUESTTARGET		(ee)

/*
 * invis/detect flags
 */
#define ID_INVIS		(A)
#define ID_HIDDEN		(B)
#define ID_FADE			(C)
#define ID_IMP_INVIS		(D)	/* improved invis */
#define ID_BLEND		(E)	/* forest blending */
#define ID_CAMOUFLAGE		(F)
#define ID_SNEAK		(G)
#define ID_INFRARED		(H)
#define ID_UNDEAD		(I)
#define ID_LIFE			(J)
#define ID_EVIL			(K)
#define ID_MAGIC		(L)
#define ID_GOOD			(M)
#define ID_TRUESEEING		(N)
#define ID_CHARM		(O)	/* Detect charm */

#define ID_ALL_INVIS	(ID_INVIS | ID_HIDDEN | ID_FADE | \
			 ID_IMP_INVIS | ID_BLEND | ID_CAMOUFLAGE)

#define HAS_INVIS(ch, f)	(IS_SET((ch)->has_invis, (f)))
#define SET_INVIS(ch, f)	(SET_BIT((ch)->has_invis, (f)))
#define REMOVE_INVIS(ch, f)	(REMOVE_BIT((ch)->has_invis, (f)))

#define HAS_DETECT(ch, f)	(IS_SET((ch)->has_detect, ((f)|ID_TRUESEEING)))
#define SET_DETECT(ch, f)	(SET_BIT((ch)->has_detect, (f)))
#define REMOVE_DETECT(ch, f)	(REMOVE_BIT((ch)->has_detect, (f)))

/* where definitions for room */
#define TO_ROOM_AFFECTS		0

/* room applies */
#define APPLY_ROOM_NONE		0
#define APPLY_ROOM_HEAL		1
#define APPLY_ROOM_MANA		2

/*
 * *AFF* bits for rooms
 */
#define RAFF_RANDOMIZER		(A)
#define RAFF_ESPIRIT		(B)
#define RAFF_CURSE		(C)
#define	RAFF_LIGHT		(D)

/* AC types */
#define AC_PIERCE			0
#define AC_BASH				1
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
#define SIZE_GARGANTUAN			6

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
#define ITEM_WARP_STONE		30
#define ITEM_ROOM_KEY		31
#define ITEM_GEM		32
#define ITEM_JEWELRY		33
#define ITEM_JUKEBOX		34
#define ITEM_TATTOO		35
#define ITEM_BOOK		36

/*
 * obj_data->stat_flags (variable obj flags)
 */
#define ITEM_GLOW		(A)
#define ITEM_HUM		(B)
#define ITEM_DARK		(C)
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
#define ITEM_ROT_DEATH		(P)
#define ITEM_VIS_DEATH		(Q)
#define ITEM_MELT_DROP		(U)
#define ITEM_HAD_TIMER		(V)
#define ITEM_BURN_PROOF		(Y)
#define ITEM_NOT_EDIBLE		(bb)
#define ITEM_ENCHANTED		(dd)	/* obj is enchanted */

#define IS_OBJ_STAT(obj, stat)		(IS_SET((obj)->stat_flags, (stat)))
#define SET_OBJ_STAT(obj, stat)		(SET_BIT((obj)->stat_flags, (stat)))
#define REMOVE_OBJ_STAT(obj, stat)	(REMOVE_BIT((obj)->stat_flags, (stat)))

/*
 * obj_index_data->obj_flags (permanent obj flags)
 */
#define OBJ_NOPURGE		(A)
#define OBJ_NOSAC		(B)
#define OBJ_NOLOCATE		(C)
#define OBJ_SELL_EXTRACT	(D)
#define OBJ_NOUNCURSE		(E)
#define OBJ_NOSELL		(F)
#define OBJ_QUEST		(G)
#define OBJ_CLAN		(H)
#define OBJ_QUIT_DROP		(I)
#define OBJ_PIT			(J)
#define OBJ_CHQUEST		(K)
#define OBJ_NOFIND		(L)

#define OBJ_IS(obj, f)		(IS_SET((obj)->pObjIndex->obj_flags, (f)))

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
#define ITEM_WEAR_HANDS		(H)
#define ITEM_WEAR_ARMS		(I)
#define ITEM_WEAR_SHIELD	(J)
#define ITEM_WEAR_ABOUT		(K)
#define ITEM_WEAR_WAIST		(L)
#define ITEM_WEAR_WRIST		(M)
#define ITEM_WIELD		(N)
#define ITEM_HOLD		(O)
#define ITEM_WEAR_FLOAT		(Q)
#define ITEM_WEAR_TATTOO	(R)
#define ITEM_WEAR_CLANMARK	(S)
#define ITEM_WEAR_FACE		(T)

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

#define WEAPON_IS(obj, wclass)	(INT(obj->value[0]) == (wclass))
#define WEAPON_IS_LONG(obj)	(WEAPON_IS((obj), WEAPON_STAFF) ||	\
				 WEAPON_IS((obj), WEAPON_SPEAR) ||	\
				 WEAPON_IS((obj), WEAPON_POLEARM) ||	\
				 WEAPON_IS((obj), WEAPON_LANCE))

/* weapon types */
#define WEAPON_FLAMING		(A)
#define WEAPON_FROST		(B)
#define WEAPON_VAMPIRIC		(C)
#define WEAPON_SHARP		(D)
#define WEAPON_VORPAL		(E)
#define WEAPON_TWO_HANDS	(F)
#define WEAPON_SHOCKING		(G)
#define WEAPON_POISON		(H)
#define WEAPON_HOLY		(I)
#define WEAPON_KATANA		(J)
#define WEAPON_THROW		(K)
#define WEAPON_NO_OFFHAND	(L)

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
#define REST_AT			(G)
#define REST_ON			(H)
#define REST_IN			(I)
#define SLEEP_AT		(J)
#define SLEEP_ON		(K)
#define SLEEP_IN		(L)
#define PUT_AT			(M)
#define PUT_ON			(N)
#define PUT_IN			(O)
#define PUT_INSIDE		(P)

/*
 * Book types
 */
#define BOOK_MANUAL		0
#define BOOK_SPELLS		1
#define BOOK_PRAYERS		2

/*
 * Types of bad effects
 */
#define FAIL_NONE		0
#define FAIL_DEATH		1
#define FAIL_HALLUCINATION	2

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
#define APPLY_LUCK		29

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
#define EX_EASY		(H)
#define EX_HARD		(I)
#define EX_INFURIATING	(J)
#define EX_NOCLOSE	(K)
#define EX_NOLOCK	(L)
#define EX_BITVAL	(Z)

/*
 * Sector types.
 * Used in #ROOMS.
 */
#define SECT_INSIDE		0
#define SECT_CITY		1
#define SECT_FIELD		2
#define SECT_FOREST		3
#define SECT_HILLS		4
#define SECT_MOUNTAIN		5
#define SECT_WATER_SWIM		6
#define SECT_WATER_NOSWIM	7
#define SECT_UNDERWATER		8
#define SECT_AIR		9
#define SECT_DESERT		10

#define MAX_SECT		11

/*
 * Equpiment wear locations.
 * Used in #RESETS.
 */
#define WEAR_NONE		     -1
#define WEAR_LIGHT		      0
#define WEAR_FINGER_L		      1
#define WEAR_FINGER_R		      2
#define WEAR_NECK		      3
#define WEAR_FACE		      4
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
#define PLR_BOUGHT_PET		(B)
#define PLR_AUTOASSIST		(C)
#define PLR_AUTOEXIT		(D)
#define PLR_AUTOLOOT		(E)
#define PLR_AUTOSAC		(F)
#define PLR_AUTOGOLD		(G)
#define PLR_AUTOSPLIT		(H)
#define PLR_NOEXP		(L)
#define PLR_AUTOLOOK		(M)
#define PLR_HOLYLIGHT		(N)
#define PLR_CANLOOT		(P)
#define PLR_NOSUMMON		(Q)
#define PLR_NOFOLLOW		(R)
#define PLR_NOCANCEL		(S)
#define PLR_GHOST		(T)
#define PLR_PERMIT		(U)
#define PLR_LOG			(W)
#define PLR_FREEZE		(Y)
#define PLR_PUMPED		(Z)	/* adrenalin is gushing */
#define PLR_PRACTICER		(aa)
#define PLR_CONFIRM_DELETE	(cc)
#define PLR_HARA_KIRI		(dd)
#define PLR_NEW			(ee)

#define WSHOW_RACE		(A)
#define WSHOW_CLASS		(B)
#define WSHOW_CLAN		(C)
#define WSHOW_LEVEL		(D)
#define WSHOW_ALIGN		(E)
#define WSHOW_ETHOS		(G)
#define WSHOW_SEX		(H)
#define WSHOW_SLANG		(I)
#define WSHOW_DEATHS		(J)
#define WSHOW_KILLS		(K)
#define WSHOW_PCKILLS		(L)
#define WSHOW_TITLE		(M)
#define WSHOW_HOMETOWN		(N)
#define WSHOW_AGE		(O)

/* Trust stuff */
#define TRUST_GROUP		(A)
#define TRUST_CLAN		(B)
#define TRUST_ALL		(C) /* mutually exclusive with previous two */

/*
 * hint levels
 */
enum {
	HINT_NONE,
	HINT_SOG,
	HINT_ALL
};

/*
 * OLC flags
 */
#define OLC_MODE_RAW		(A)	/* show strings without color parsing */
#define OLC_MODE_TRANS		(B)	/* translation mode (show only
					   name/description/gender fields */

#define IS_HARA_KIRI(ch)	(!IS_NPC(ch) &&	\
				 IS_SET(PC(ch)->plr_flags, PLR_HARA_KIRI))
#define IS_CLAN_GUARD(ch)	(IS_NPC(ch) && \
				 IS_SET(ch->pMobIndex->mob_flags, MOB_CLAN_GUARD))
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
			SET_BIT(PC(ch)->plr_flags, PLR_PUMPED);		\
	}

#define RESET_FIGHT_TIME(ch)						\
	{								\
		(ch)->last_fight_time = -1;				\
		if (!IS_NPC(ch))					\
			REMOVE_BIT(PC(ch)->plr_flags, PLR_PUMPED);	\
	}

#define IS_VAMPIRE(ch)	(get_skill(ch, "vampire") == 100)

#define CHAN_NOCHANNELS		(A)
#define CHAN_NOWIZ		(B)
#define CHAN_NOAUCTION		(C)
#define CHAN_NOGOSSIP		(D)
#define CHAN_NOQUESTION		(E)
#define CHAN_NOMUSIC		(F)
#define CHAN_NOQUOTE		(G)
#define CHAN_NOCLAN		(H)
#define CHAN_NOSHOUT		(I)

/* RT comm flags -- may be used on both mobs and chars */
#define COMM_QUIET		(A)
#define COMM_DEAF		(B)
#define COMM_NOFLEE		(C)
#define COMM_NONOTE		(D)
#define COMM_SHOWRACE		(E)
#define COMM_SHORT_EQ		(F)
#define COMM_QUIET_EDITOR	(K)
#define COMM_COMPACT		(L)
#define COMM_BRIEF		(M)
#define COMM_PROMPT		(N)
#define COMM_COMBINE		(O)
#define COMM_TELNET_GA		(P)
#define COMM_SHOWAFF		(Q)
#define COMM_COLOR		(S)
#define COMM_NOEMOTE		(T)
#define COMM_NOTELL		(V)
#define COMM_NOENG		(X)
#define COMM_SNOOP_PROOF	(Y)
#define COMM_AFK		(Z)
#define COMM_LONG		(aa)
#define COMM_NOTELNET		(bb)
#define COMM_NOIAC		(cc)
#define COMM_NOVERBOSE		(dd)
#define COMM_NOBUST		(ee)

#define OLC_
/* IAC replacement if COMM_NOIAC is set */
/* COMM_NOIAC is used to map 'Ñ' (IAC) to 'ñ' when using win1251 codepage */
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
	SHOP_DATA *		pShop;
	int			vnum;
	int			fvnum;
	int			group;
	int			count;
	int			killed;
	const char *		name;
	mlstring		short_descr;
	mlstring		long_descr;
	mlstring		description;

	flag_t			affected_by;
	flag_t			has_invis;
	flag_t			has_detect;

	int			alignment;
	int			level;
	int			hitroll;
	int			xp_multiplier;
	int			hit[3];
	int			mana[3];
	int			damage[3];
	int			ac[4];
	const char *		damtype;
	flag_t			act;
	flag_t			mob_flags;
	flag_t			off_flags;
	flag_t			start_pos;
	flag_t			default_pos;
	mlstring		gender;
	const char *		race;		/* race */
	int			wealth;
	flag_t			form;
	flag_t			parts;
	flag_t			size;
	int16_t			resists[MAX_RESIST];	/* resistances */
	const char *		material;
	flag_t			practicer;
	const char *		clan;
	int			invis_level;	/* mobinvis level */
	int			incog_level;	/* mobincog level */
	AFFECT_DATA *		affected;
	varr			mp_trigs;
};

#define NPC(ch)	((NPC_DATA *) ((ch) + 1))
#define PC(ch)	((PC_DATA *) ((ch) + 1))

CHAR_DATA *	char_new	(MOB_INDEX_DATA *pMobIndex);
void		char_free	(CHAR_DATA *ch);

void	objval_init	(flag_t item_type, vo_t *v);
void	objval_cpy	(flag_t item_type, vo_t *dst, vo_t *src);
void	objval_destroy	(flag_t item_type, vo_t *v);

void	fwrite_objval	(flag_t item_type, vo_t *v, FILE *fp);
void	fread_objval	(flag_t item_type, vo_t *v, rfile_t *fp);

void	objval_show	(BUFFER *output, flag_t item_type, vo_t *v);
int	objval_set	(BUFFER *output, flag_t item_type, vo_t *v,
			 int value_num, const char *argument);

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

	CHAR_DATA *		next;
	CHAR_DATA *		aff_next;
	CHAR_DATA *		next_in_room;

	CHAR_DATA *		master;
	CHAR_DATA *		leader;

	CHAR_DATA *		fighting;
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
	const char *		class;
	const char *		race;
	const char *		clan;
	int			ethos;
	int			level;
	int			wait;
	int			add_level;
	int			daze;
	form_t *		shapeform;

	int			hit;
	int			max_hit;
	int			perm_hit;

	int			mana;
	int			max_mana;
	int			perm_mana;

	int			move;
	int			max_move;
	int			perm_move;

	int			gold;
	int			silver;
	flag_t			comm;	/* RT added to pad the vector */
	flag_t			chan;	/* channels */
	int			invis_level;
	int			incog_level;

	flag_t			affected_by;
	flag_t			has_invis;
	flag_t			has_detect;

	flag_t			position;
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
	int			perm_stat[MAX_STAT];
	int			mod_stat[MAX_STAT];

	int			luck;
	int			luck_mod;

	/* resistances mods */
	int16_t			res_mod[MAX_RESIST];

	/* parts stuff */
	flag_t			form;
	flag_t			parts;
	flag_t			size;
	const char *		material;

	bool			riding;		/* mount data */
	CHAR_DATA *		mount;

	CHAR_DATA *		hunting;	/* hunt data */
	int			endur;

	CHAR_DATA *		doppel;		/* doppelganger and mirror */

	int			slang;		/* spoken language */
	mlstring		gender;
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
	AREA_DATA *	zone;
	CHAR_DATA *	target;
	const char *	in_mind;
	CHAR_DATA *	last_fought;
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
	time_t			last_offence_time;
	flag_t			trust;
	flag_t			plr_flags;
	flag_t			www_show_flags;
	flag_t			wiznet;		/* wiz stuff */
	int			condition	[MAX_COND];
	varr			learned;	/* pc_skill_t */
	varr			specs;		/* spec names */
	int			security;	/* OLC */ /* Builder security */
	int			bank_s;
	int			bank_g;
	int			death;
	int			anti_killed;
	int			has_killed;

	int			questgiver;
	int			questpoints;
	int			questtime;
	int			questobj;
	int			questmob;
	qtrouble_t *		qtrouble;
	int			qroom_vnum;	/* quest location */

	const char *		race;
	int			pc_killed;

	int			clan_status;
	const char *		petition;

	int			plevels;	/* penalty levels */

	int			hometown;
	ROOM_INDEX_DATA	*	homepoint;

	const char *		wanted_by;
	CHAR_DATA *		reply;		/* for do_reply */
	note_t *		pnote;

	int			exp;		/* total exp */
	int			practice;
	int			train;
	int			random_value;	/* for hallucination spell */

	time_t			logon;
	time_t			logoff;

	int			played;
	int			add_age;

	int			idle_timer;

	int			was_in_vnum;	/* room vnum char was in */
	CHAR_DATA *		pet;
	CHAR_DATA *		guarding;
	CHAR_DATA *		guarded_by;
	int			version;

	flag_t			hints_level;

	dvdata_t *		dvdata;

	int			move_used;	/* Moves used in current tick */
};

/*
 * PC learned skill
 */
typedef struct pc_skill_t pc_skill_t;
struct pc_skill_t {
	const char *sn;	/* skill name				*/
	int percent;	/* skill percentage			*/
};

void pc_skill_init(pc_skill_t *);

#define pc_skill_lookup(ch, sn)						\
	((pc_skill_t *) varr_bsearch(&PC(ch)->learned, &sn, cmpstr))

/*
 * Extra description data for a room or object.
 */
struct ed_data
{
	ED_DATA *	next;		/* Next in list		    */
	const char *	keyword;	/* Keyword in look/examine  */
	mlstring	description;	/* What to see		    */
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
	flag_t			stat_flags;
	flag_t			obj_flags;
	flag_t			wear_flags;
	int			level;
	int			condition;
	int			count;
	int			weight;
	uint			cost;
	flag_t			item_type;
	vo_t			value[5];
	int			limit;
	mlstring		gender;
	varr			mp_trigs;
};

/*
 * One object.
 */
struct obj_data
{
	OBJ_DATA *		next;
	OBJ_DATA *		aff_next;
	OBJ_DATA *		next_content;
	OBJ_DATA *		contains;
	OBJ_DATA *		in_obj;
	OBJ_DATA *		on;
	CHAR_DATA *		carried_by;
	CHAR_DATA *		last_owner;
	ED_DATA *		ed;
	AFFECT_DATA *		affected;
	OBJ_INDEX_DATA *	pObjIndex;
	ROOM_INDEX_DATA *	in_room;
	const char *		label;
	mlstring		short_descr;
	mlstring		description;
	flag_t			stat_flags;
	flag_t			wear_flags;
	flag_t			wear_loc;
	int			weight;
	uint			cost;
	int			level;
	int			condition;
	const char *		material;
	int			timer;
	flag_t			item_type;
	vo_t			value[5];
	int			progtypes;
	mlstring		owner;
	altar_t *		altar;
	int			water_float;
};

/*
 * Exit data.
 */
struct exit_data
{
	vo_t		to_room;
	flag_t		exit_info;
	flag_t		size;
	int		key;
	const char *	keyword;
	gmlstr_t	short_descr;
	mlstring	description;
	EXIT_DATA *	next;		/* OLC */
	flag_t		rs_flags;	/* OLC */
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
	int		arg0;
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
	AREA_DATA *	next;
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
	flag_t		area_flags;
	int		security;	/* area security (1..9)		*/
	uint		count;
	mlstring	resetmsg;	/* reset message		*/
	int		ver;		/* area file version		*/
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
	CHAR_DATA *		people;
	OBJ_DATA *		contents;
	ED_DATA *		ed;
	AREA_DATA *		area;
	EXIT_DATA *		exit	[6];
	RESET_DATA *		reset_first;	/* OLC */
	RESET_DATA *		reset_last;	/* OLC */
	mlstring		name;
	mlstring		description;
	int			vnum;
	flag_t			room_flags;
	int			light;
	flag_t			sector_type;
	int			heal_rate;
	int			heal_rate_mod;
	int			mana_rate;
	int			mana_rate_mod;
	ROOM_HISTORY_DATA *	history;
	AFFECT_DATA *		affected;
	flag_t			affected_by;
};

#define GET_HEAL_RATE(r)	((r)->heal_rate + (r)->heal_rate_mod)
#define GET_MANA_RATE(r)	((r)->mana_rate + (r)->mana_rate_mod)

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
#define IS_WATER(var)	(((var)->sector_type == SECT_WATER_SWIM) || \
				 ((var)->sector_type == SECT_WATER_NOSWIM))
#define PERCENT(cur, max)	(max==0?0:((cur)*100)/(max))

/*
 * Character macros.
 */
#define IS_NPC(ch)		(ch->pMobIndex != NULL)
#define IS_IMMORTAL(ch)		(!IS_NPC(ch) && (ch)->level >= LEVEL_IMMORTAL)
#define IS_HERO(ch)		(!IS_NPC(ch) && (ch)->level >= LEVEL_HERO)

#define GET_ORIGINAL(ch)	(ch->desc && ch->desc->original ?	\
					ch->desc->original : ch)
#define IS_TRUSTED(ch, lev)	(trust_level(ch) >= (lev))

#define IS_AFFECTED(ch, bit)	(IS_SET((ch)->affected_by, (bit)))

#define ORG_RACE(ch)		(IS_NPC(ch) ? (ch)->pMobIndex->race : PC(ch)->race)

#define LEVEL(ch)		((ch)->level + (ch)->add_level)

#define GET_EXP(ch)		(IS_NPC(ch) ? 0 : PC(ch)->exp)
#define GET_PET(ch)		(IS_NPC(ch) ? NULL : PC(ch)->pet)
#define GET_LANG(ch)		(ch->desc ? ch->desc->dvdata->lang : 0)

#define IS_GOOD_ALIGN(align)	((align) >= 350)
#define IS_EVIL_ALIGN(align)	((align) <= -350)
#define IS_NEUTRAL_ALIGN(align)	(!IS_GOOD_ALIGN(align) && !IS_EVIL_ALIGN(align))

#define IS_GOOD(ch)		(IS_GOOD_ALIGN((ch)->alignment))
#define IS_EVIL(ch)		(IS_EVIL_ALIGN((ch)->alignment))
#define IS_NEUTRAL(ch)		(IS_NEUTRAL_ALIGN((ch)->alignment))

#define RALIGN(ch)	(IS_GOOD(ch) ? RA_GOOD :	\
			 IS_EVIL(ch) ? RA_EVIL :	\
				       RA_NEUTRAL)
#define NALIGN(ch)	(IS_GOOD(ch) ? AN_GOOD :	\
			 IS_EVIL(ch) ? AN_EVIL :	\
				       AN_NEUTRAL)

#define IS_AWAKE(ch)		(ch->position > POS_SLEEPING)
#define GET_AC(ch, type)	((ch)->armor[type]			    \
			+ (IS_AWAKE(ch)			    \
			? dex_app[get_curr_stat(ch,STAT_DEX)].defensive : 0))
#define GET_HITROLL(ch) \
		(((ch)->shapeform) ? ((ch)->shapeform->hitroll) : ((ch)->hitroll+str_app[get_curr_stat(ch,STAT_STR)].tohit))
#define GET_DAMROLL(ch) \
		(((ch)->shapeform) ? ((ch)->shapeform->damroll) : ((ch)->damroll+str_app[get_curr_stat(ch,STAT_STR)].todam))

#define GET_LUCK(ch) URANGE(0, (ch)->luck+(ch)->luck_mod, 100)

#define IS_OUTSIDE(ch)	(!IS_SET((ch)->in_room->room_flags, ROOM_INDOORS))

#define WAIT_STATE(ch, npulse)	((ch)->wait = IS_IMMORTAL(ch) ?	\
				 1 : UMAX((ch)->wait, (npulse)))
#define RESET_WAIT_STATE(ch)	((ch)->wait = 1)
#define DAZE_STATE(ch, npulse)	((ch)->daze = UMAX((ch)->daze, (npulse)))

#define GOLD_WEIGHT(gold)	((gold) * 2 / 5)
#define SILVER_WEIGHT(silver)	((silver) / 10)

#define COINS_WEIGHT(is_gold, amount)	((is_gold) ?			\
				GOLD_WEIGHT(amount) :			\
				SILVER_WEIGHT(amount))
#define get_carry_weight(ch)	((ch)->carry_weight +			\
				 SILVER_WEIGHT((ch)->silver) +		\
				 GOLD_WEIGHT((ch)->gold))
#define MONEY_WEIGHT(obj)	(SILVER_WEIGHT(INT(obj->value[0])) +	\
				 GOLD_WEIGHT(INT(obj->value[1])))

#define GET_AVE(v1, v2)		((1 + INT(v2)) * INT(v1) / 2)
#define GET_WEAPON_AVE(obj)	GET_AVE((obj)->value[1], (obj)->value[2])

#define IS_SWITCHED( ch )       (ch->desc && ch->desc->original)
#define IS_BUILDER(ch, Area)	(!IS_NPC(ch) && !IS_SWITCHED(ch) &&	      \
				 (PC(ch)->security >= (Area)->security || \
				  is_name_strict(ch->name, Area->builders)))

#define MOUNTED(ch)	((!IS_NPC(ch) && ch->mount && ch->riding) ? \
				ch->mount : NULL)
#define RIDDEN(ch)	((IS_NPC(ch) && ch->mount && ch->riding) ? \
				ch->mount : NULL)
#define IS_DRUNK(ch)	(!IS_NPC(ch) && PC(ch)->condition[COND_DRUNK] > 10)

#define IS_EXTRACTED(ch)	(!mem_is(ch, MT_CHAR) || \
				 ch->last_death_time >= current_time)

#define FLEE_EXP(ch)	(number_range(10, (ch)->level * 100))

/*
 * Object macros.
 */
#define CAN_WEAR(obj, part)	(IS_SET((obj)->wear_flags,  (part)))
#define IS_WEAPON_STAT(obj,stat)(IS_SET(INT((obj)->value[4]), (stat)))
#define WEIGHT_MULT(obj)	((obj)->pObjIndex->item_type == ITEM_CONTAINER ? \
	INT((obj)->value[4]) : 100)

/*
 * Description macros.
 */
/* new defines */
#define MAX_CHARM(ch)	((get_curr_stat(ch,STAT_INT) / 4) + (ch->level / 30))

/*
 * Global constants.
 */
extern	const	struct str_app_type	str_app[26];
extern	const	struct int_app_type	int_app[26];
extern	const	struct wis_app_type	wis_app[26];
extern	const	struct dex_app_type	dex_app[26];
extern	const	struct con_app_type	con_app[26];

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

extern		ROOM_INDEX_DATA   *	top_affected_room;
extern		CHAR_DATA	  *	top_affected_char;
extern		OBJ_DATA	  *	top_affected_obj;

extern		char			bug_buf[];
extern		time_t			current_time;
extern		time_t			boot_time;
extern		bool			fLogAll;
extern		char			log_buf[];
extern		TIME_INFO_DATA		time_info;
extern		WEATHER_DATA		weather_info;
extern		int			reboot_counter;
extern		bool			newlock;
extern		bool			wizlock;

/*
 * The crypt(3) function is not available on some operating systems.
 * In particular, the U.S. Government prohibits its export from the
 * United States to foreign countries.
 * Turn on NOCRYPT to keep passwords in plain text.
 */
#if	defined(NOCRYPT)
#define crypt(s1, s2)	(s1)
#endif

#define IS_OBJ_NAME(obj, _name)	(is_name(_name, (obj)->pObjIndex->name) ||\
				 is_name(_name, (obj)->label))

RESET_DATA *	reset_new	(void);
void		reset_free	(RESET_DATA *pReset);

void	reset_add(ROOM_INDEX_DATA *room, RESET_DATA *reset, RESET_DATA *after);
void	reset_del(ROOM_INDEX_DATA *room, RESET_DATA *reset);
RESET_DATA *reset_lookup(ROOM_INDEX_DATA *room, int rnum);

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

/* extra descr recycling */
ED_DATA	*ed_new		(void);
ED_DATA *ed_new2	(const ED_DATA *ed, const char* name);
ED_DATA *ed_dup		(const ED_DATA *ed);
void	ed_fread	(rfile_t *fp, ED_DATA **edp);
void	ed_fwrite	(FILE *fp, ED_DATA *ed);
void	ed_free		(ED_DATA *ed);

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
void		help_show_raw	(int level, uint lang, BUFFER *output,
				 const char *keyword);
void		help_free	(HELP_DATA*);

void		boot_db_system	(void);
void		boot_db		(void);

ED_DATA * ed_lookup(const char *name, ED_DATA *ed);

MOB_INDEX_DATA *	get_mob_index	(int vnum);
OBJ_INDEX_DATA *	get_obj_index	(int vnum);
ROOM_INDEX_DATA *	get_room_index	(int vnum);

/*
 * Global Constants
 */
extern	const char *			dir_name	[];
extern	const char *			from_dir_name	[];
extern	const int			rev_dir		[];
extern	const int			show_order	[];
extern	const char			DEFAULT_PROMPT	[];

extern RUNGAME_FUN *run_game;
extern RUNGAME_FUN *run_game_bottom;
extern bool (*olc_interpret)(DESCRIPTOR_DATA *d, const char *argument);

/*
 * dump levels
 */
enum {
	DL_NONE,	/* do nothing */
	DL_COLOR,	/* color sequences escaping */
	DL_ALL		/* DL_COLOR and control characters escaping */
};

#define DUMP_LEVEL(ch)	((ch)->desc &&					\
			 IS_SET((ch)->desc->dvdata->olc_flags,		\
				OLC_MODE_RAW) ? DL_COLOR : DL_NONE)

/* rebooting flag */
extern bool merc_down;

/*-------------------------------------------------------------------
 * affects stuff
 */

/* where definitions */
#define TO_AFFECTS	0
#define TO_OBJECT	1
#define TO_WEAPON	2
#define TO_SKILLS	3
#define TO_RACE		4
#define TO_DETECTS	5
#define TO_INVIS	6
#define TO_FORM		7
#define TO_FORMAFFECTS	8
#define TO_FORMRESISTS	9
#define TO_RESISTS	10

struct affect_data
{
	AFFECT_DATA *	next;
	flag_t		where;
	const char *	type;
	int		level;
	int		duration;
	vo_t		location;
	int		modifier;
	flag_t		bitvector;
	CHAR_DATA *	owner;
};

typedef struct saff_t {
	const char *	sn;		/* affect which skill */
	const char *	type;		/* affected by which skill */
	int		mod;		/* modify skill knowledge by */
	flag_t		bit;		/* with bits ... */
} saff_t;

AFFECT_DATA *	aff_new		(int where, const char *sn);
AFFECT_DATA *	aff_dup		(const AFFECT_DATA *af);
void		aff_free	(AFFECT_DATA *af);

AFFECT_DATA *	aff_dup_list	(AFFECT_DATA *af, int level);
void		aff_free_list	(AFFECT_DATA *af);

#define AFF_X_NOTYPE	(A)		/* do not r/w paf->type */
#define AFF_X_NOLD	(B)		/* do not r/w paf->level/duration */

AFFECT_DATA *	aff_fread	(rfile_t *fp, int r_flags);
AFFECT_DATA *	aff_fread_v5	(rfile_t *fp);

void		aff_fwrite	(AFFECT_DATA *paf, FILE *fp, int w_flags);
void		aff_fwrite_list	(const char *pre, const char *pre_notype,
				 AFFECT_DATA *paf, FILE *fp, int w_flags);

void		saff_init(saff_t *sa);
void		saff_destroy(saff_t *sa);

typedef struct where_t where_t;
struct where_t
{
	flag_t		where;
	flaginfo_t *	loc_table;
	flaginfo_t *	bit_table;
	const char *	loc_format;
	const char *	bit_format;
};

where_t *where_lookup(flag_t where);

#define IS_APPLY_WHERE(where)					\
	((where) == TO_AFFECTS ||				\
	 (where) == TO_OBJECT ||				\
	 (where) == TO_WEAPON ||				\
	 (where) == TO_DETECTS ||				\
	 (where) == TO_INVIS ||					\
	 (where) == TO_FORMAFFECTS)
#define IS_APPLY_AFFECT(paf)	IS_APPLY_WHERE((paf)->where)

/*----------------------------------------------------------------------
 * skills stuff
 */

/* skill flags */
#define SKILL_CLAN		(A)
#define SKILL_RANGE		(B)
#define SKILL_AREA_ATTACK	(C)
#define SKILL_QUESTIONABLE	(D)
#define SKILL_FORM		(E)
#define SKILL_MISSILE		(F)
#define SKILL_SHADOW		(G)	/* for "shadow magic" skill */

/* skill types */
#define ST_SKILL	0
#define ST_SPELL	1
#define ST_PRAYER	2

/*
 * EVENTs for room affects
 */
enum {
	EVENT_ROOM_ENTER,
	EVENT_ROOM_LEAVE,
	EVENT_ROOM_UPDATE,
	EVENT_ROOM_TIMEOUT,
	EVENT_CHAR_UPDATE,
	EVENT_CHAR_UPDFAST,
	EVENT_CHAR_TIMEOUT,
};

typedef struct skill_t skill_t;
struct skill_t {
	gmlstr_t	sk_name;		/* skill name */
	const char *	fun_name;		/* skill function name */
	SPELL_FUN *	fun;			/* skill function */
	flag_t		target;			/* legal target */
	flag_t		min_pos;		/* position for caster */
	int		min_mana;		/* min mana used */
	int		beats;			/* waiting time after use */
	int		rank;			/* Shows rank of difficulty of
						 * spell or prayer (0..7) */
	gmlstr_t	noun_damage;		/* damage message */
	mlstring	msg_off;		/* wear off message */
	mlstring	msg_obj;		/* wear off message for obj */
	flag_t		skill_flags;		/* skill flags */
	const char *	restrict_race;		/* race restrictions */
	flag_t		group;			/* skill group */
	flag_t		skill_type;		/* skill type */
	varr		events;			/* evf_t, sorted by event */
};

typedef struct evf_t {
	flag_t			event;
	const char *		fun_name;
	EVENT_FUN *		fun;
} evf_t;

extern hash_t skills;
extern hashdata_t h_skills;

#define IS_SKILL(sn1, sn2)	(!str_cmp((sn1), (sn2)))

void skill_init(skill_t *sk);
skill_t *skill_cpy(skill_t *dst, const skill_t *src);
void skill_destroy(skill_t *sk);

void skills_dump(BUFFER *output, int skill_type);

/*
 * misc skill lookup functions
 */

/* fast skill lookup by precise name */
#define skill_lookup(sn)	((skill_t*) strkey_lookup(&skills, (sn)))
#define skill_search(sn)	((skill_t*) mlstrkey_search(&skills, (sn)))

/*----------------------------------------------------------------------
 * specs stuff
 */

/*
 * spec classes
 */
enum {
	SPEC_CLASS,
	SPEC_RACE,
	SPEC_CLAN,
	SPEC_WEAPON,
	SPEC_FORM,
	SPEC_MAJORSCHOOL,
	SPEC_MINORSCHOOL
};

#define	SPF_CHANGED	(Z)

struct spec_t {
	const char *spec_name;	/* spec name, also used as file name	*/
	flag_t spec_class;	/* spec class				*/
	varr spec_skills;	/* spec_skill_t				*/
	flag_t spec_flags;	/* spec flags				*/
	trig_t mp_trig;		/* spec mpc trigger			*/
};

extern hash_t specs;
extern hashdata_t h_specs;

typedef struct spec_skill_t spec_skill_t;
struct spec_skill_t {
	const char *	sn;		/* skill name			*/
	int		level;		/* level needed to gain skill	*/
	int		rating;		/* how hard it is to learn	*/

	int		min;		/* min (initial) skill percents */
	int		adept;		/* adept percents		*/
	int		max;		/* max skill percents		*/
};

void	spec_init(spec_t *spec);
spec_t *spec_cpy(spec_t *dst, const spec_t *src);
void	spec_destroy(spec_t *spec);

/* fast spec lookup by precise name */
#define spec_lookup(spn)	((spec_t*) strkey_lookup(&specs, (spn)))
#define spec_search(spn)	((spec_t*) strkey_search(&specs, (spn)))
#define spec_skill_lookup(spec, sn)					\
	((spec_skill_t*) varr_bsearch(&spec->spec_skills, &sn, cmpstr))

/*----------------------------------------------------------------------
 * socials stuff
 */

/*
 * Structure for a social in the socials table.
 */
struct social_t
{
	const char *	name;
	flag_t		min_pos;

	mlstring	found_char;
	mlstring	found_vict;
	mlstring	found_notvict;

	mlstring	noarg_char;
	mlstring	noarg_room;

	mlstring	self_char;
	mlstring	self_room;

	mlstring	notfound_char;
};
typedef struct social_t social_t;

extern varr socials;
extern varrdata_t v_socials;

void	social_init	(social_t *soc);
void	social_destroy	(social_t *soc);

#define social_lookup(name)	((social_t *) vstr_lookup(&socials, (name)))
#define social_search(name)	((social_t *) vstr_search(&socials, (name)))

/*----------------------------------------------------------------------
 * socials stuff
 */

struct auction_data
{
	OBJ_DATA  * item;	/* a pointer to the item */
	CHAR_DATA * seller;	/* a pointer to the seller (may NOT quit) */
	CHAR_DATA * buyer;	/* a pointer to the buyer (may NOT quit) */
	int	starting;
	int	bet;		/* last bet - or 0 if noone has bet anything */
	int	going;		/* 1,2, sold */
};

extern AUCTION_DATA auction;

#define IS_AUCTIONED(obj) (auction.item == obj)

/*----------------------------------------------------------------------
 * hints stuff
 */

typedef struct {
	mlstring	phrase;
	flag_t		hint_level;
} hint_t;

extern varr hints;
extern varrdata_t v_hints;

void hint_init(hint_t *t);
void hint_destroy(hint_t *t);

/*----------------------------------------------------------------------
 * rating stuff
 */

#define RATING_TABLE_SIZE	20

struct rating_t {
	const char *name;
	int pc_killed;
};

extern rating_t rating_table[RATING_TABLE_SIZE];

/*----------------------------------------------------------------------
 * global mlstrings with gender
 */

extern hash_t glob_gmlstr;
extern hashdata_t h_glob_gmlstr;

#define	glob_lookup(gn)	((gmlstr_t *) strkey_lookup(&glob_gmlstr, (gn)))

/*----------------------------------------------------------------------
 * ban stuff
 */

/* ban actions */
enum {
	BA_ALLOW,
	BA_DENY
};

/* ban classes */
enum {
	BCL_ALL,
	BCL_PLAYERS,
	BCL_NEWBIES
};

struct ban_t
{
	int		ban_num;
	flag_t		ban_action;
	flag_t		ban_class;
	const char *	ban_mask;
	ban_t *		next;
};

extern ban_t *ban_list;

/*
 * changed flags
 */
#define CF_MSGDB	(A)
#define CF_SOCIAL	(B)
#define CF_CMD		(C)
#define CF_SKILL	(D)
#define CF_MATERIAL	(E)
#define CF_LIQUID	(F)
#define CF_DAMT		(G)
#define CF_HINT		(H)

extern int changed_flags;

#define SLIST_ADD(type, list, item)					\
	{								\
		if ((list) == NULL)					\
			(list) = (item);				\
		else {							\
			type *p;					\
									\
			for (p = (list); p->next != NULL; p = p->next)	\
				;					\
			p->next = (item);				\
		}							\
		(item)->next = NULL;					\
	}

extern MOB_INDEX_DATA *	mob_index_hash	[MAX_KEY_HASH];
extern OBJ_INDEX_DATA *	obj_index_hash	[MAX_KEY_HASH];
extern ROOM_INDEX_DATA *room_index_hash [MAX_KEY_HASH];
extern int		mob_index_count;
extern int		obj_index_count;
extern int		affect_count;
extern int		ed_count;
extern int		area_count;
extern int		exit_count;
extern int		help_count;
extern int		reset_count;
extern int		room_count;
extern int		shop_count;
extern int		str_count;
extern int		str_real_count;
#if STR_ALLOC_MEM
extern int		str_alloc_mem;
#endif
extern int		npc_count;
extern int		npc_free_count;
extern int		pc_count;
extern int		pc_free_count;
extern int		desc_count;
extern int		desc_free_count;
extern int		dvdata_count;
extern int		dvdata_real_count;
extern int		top_player;
extern int		top_vnum_mob;
extern int		top_vnum_obj;
extern int		top_vnum_room;
extern int		rebooter;
extern AREA_DATA *	area_first;
extern AREA_DATA *	area_last;
extern AREA_DATA *	area_current;
extern SHOP_DATA *	shop_last;

/*
 * the following path/file name consts are defined in db.c
 */
extern const char TMP_PATH	[];
extern const char PLAYER_PATH	[];
extern const char GODS_PATH	[];
extern const char NOTES_PATH	[];
extern const char ETC_PATH	[];
extern const char AREA_PATH	[];
extern const char LANG_PATH	[];
extern const char MODULES_PATH	[];
extern const char MPC_PATH	[];

extern const char CLASSES_PATH	[];
extern const char CLANS_PATH	[];
extern const char PLISTS_PATH	[];
extern const char RACES_PATH	[];
extern const char SPEC_PATH	[];

extern const char CLASS_EXT	[];
extern const char CLAN_EXT	[];
extern const char RACE_EXT	[];
extern const char SPEC_EXT	[];
extern const char MPC_EXT	[];

extern const char TMP_FILE	[];
extern const char NULL_FILE	[];

extern const char MODULES_CONF	[];
extern const char HOMETOWNS_CONF[];
extern const char SKILLS_CONF	[];
extern const char SOCIALS_CONF	[];
extern const char SYSTEM_CONF	[];
extern const char LANG_CONF	[];
extern const char CMD_CONF	[];
extern const char DAMTYPE_CONF	[];
extern const char MATERIALS_CONF[];
extern const char LIQUIDS_CONF	[];
extern const char CC_EXPR_CONF	[];
extern const char UHANDLERS_CONF[];
extern const char FORMS_CONF	[];

extern const char MSGDB_FILE	[];
extern const char HINTS_FILE	[];
extern const char GLOB_GMLSTR_FILE[];

extern const char AREA_LIST	[];
extern const char LANG_LIST	[];

extern const char NOTE_FILE	[];
extern const char IDEA_FILE	[];
extern const char PENALTY_FILE	[];
extern const char NEWS_FILE	[];
extern const char CHANGES_FILE	[];
extern const char SHUTDOWN_FILE	[];
extern const char EQCHECK_FILE	[];
extern const char EQCHECK_SAVE_ALL_FILE [];
extern const char BAN_FILE	[];
extern const char MAXON_FILE	[];
extern const char AREASTAT_FILE	[];
extern const char IMMLOG_FILE	[];

#endif
