/*
 * $Id: tables.c,v 1.15 1998-10-13 12:38:08 fjoe Exp $
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

#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include "merc.h"
#include "interp.h"
#include "db/lang.h"

FLAG slang_table[] =
{
	{ "common",		SLANG_COMMON,		TRUE	},
	{ "human",		SLANG_HUMAN,		TRUE	},
	{ "elvish",		SLANG_ELVISH,		TRUE	},
	{ "dwarvish",		SLANG_DWARVISH,		TRUE	},
	{ "gnomish",		SLANG_GNOMISH,		TRUE	},
	{ "giant",		SLANG_GIANT,		TRUE	},
	{ "trollish",		SLANG_TROLLISH,		TRUE	},
	{ "cat",		SLANG_CAT,		TRUE	},
	{ "mothertongue",	SLANG_MAX,		FALSE	},
	{ NULL }
};

FLAG size_table[] =
{ 
	{ "tiny",		SIZE_TINY,		TRUE	},
	{ "small", 		SIZE_SMALL,		TRUE	},
	{ "medium",		SIZE_MEDIUM,		TRUE	},
	{ "large",		SIZE_LARGE,		TRUE	},
	{ "huge", 		SIZE_HUGE,		TRUE	},
	{ "giant",		SIZE_GIANT,		TRUE	},
	{ "gargantuan",		SIZE_GARGANTUAN,	TRUE	},
	{ NULL }
};

FLAG act_flags[] =
{
	{ "npc",		ACT_NPC,		FALSE	},
	{ "sentinel",		ACT_SENTINEL,		TRUE	},
	{ "scavenger",		ACT_SCAVENGER,		TRUE	},
	{ "aggressive",		ACT_AGGRESSIVE,		TRUE	},
	{ "stay_area",		ACT_STAY_AREA,		TRUE	},
	{ "wimpy",		ACT_WIMPY,		TRUE	},
	{ "pet",		ACT_PET,		TRUE	},
	{ "train",		ACT_TRAIN,		TRUE	},
	{ "practice",		ACT_PRACTICE,		TRUE	},
	{ "hunter",		ACT_HUNTER,		TRUE	},
	{ "undead",		ACT_UNDEAD,		TRUE	},
	{ "cleric",		ACT_CLERIC,		TRUE	},
	{ "mage",		ACT_MAGE,		TRUE	},
	{ "thief",		ACT_THIEF,		TRUE	},
	{ "warrior",		ACT_WARRIOR,		TRUE	},
	{ "noalign",		ACT_NOALIGN,		TRUE	},
	{ "nopurge",		ACT_NOPURGE,		TRUE	},
	{ "outdoors",		ACT_OUTDOORS,		TRUE	},
	{ "questor",		ACT_QUESTOR,		TRUE	},
	{ "indoors",		ACT_INDOORS,		TRUE	},
	{ "rideable",		ACT_RIDEABLE,		TRUE	},
	{ "healer",		ACT_HEALER,		TRUE	},
	{ "gain",		ACT_GAIN,		TRUE	},
	{ "update_always",	ACT_UPDATE_ALWAYS,	TRUE	},
	{ "changer",		ACT_CHANGER,		TRUE	},
	{ "notrack",		ACT_NOTRACK,		TRUE	},
	{ NULL }
};

FLAG plr_flags[] =
{
	{ "npc",		PLR_NPC,		FALSE	},
	{ "autoassist",		PLR_AUTOASSIST,		FALSE	},
	{ "autoexit",		PLR_AUTOEXIT,		FALSE	},
	{ "autoloot",		PLR_AUTOLOOT,		FALSE	},
	{ "autosac",		PLR_AUTOSAC,		FALSE	},
	{ "autogold",		PLR_AUTOGOLD,		FALSE	},
	{ "autosplit",		PLR_AUTOSPLIT,		FALSE	},
	{ "color",		PLR_COLOR,		FALSE	},
	{ "bought_pet",		PLR_BOUGHT_PET,		FALSE	},
	{ "holylight",		PLR_HOLYLIGHT,		FALSE	},
	{ "can_loot",		PLR_CANLOOT,		FALSE	},
	{ "nosummon",		PLR_NOSUMMON,		FALSE	},
	{ "nofollow",		PLR_NOFOLLOW,		FALSE	},
	{ "permit",		PLR_PERMIT,		TRUE	},
	{ "log",		PLR_LOG,		FALSE	},
	{ "deny",		PLR_DENY,		FALSE	},
	{ "freeze",		PLR_FREEZE,		FALSE	},
	{ "wanted",		PLR_WANTED,		FALSE	},
	{ "ghost",		PLR_GHOST,		FALSE	},
	{ "notitle",		PLR_NOTITLE,		FALSE	},
	{ "noexp",		PLR_NOEXP,		FALSE	},
	{ "hara_kiri",		PLR_HARA_KIRI,		FALSE	},
	{ "blink",		PLR_BLINK,		FALSE	},
	{ NULL }
};

FLAG affect_flags[] =
{
	{ "blind",		AFF_BLIND,		TRUE	},
	{ "invisible",		AFF_INVISIBLE,		TRUE	},
	{ "detect_evil",	AFF_DETECT_EVIL,	TRUE	},
	{ "detect_invis",	AFF_DETECT_INVIS,	TRUE	},
	{ "detect_magic",	AFF_DETECT_MAGIC,	TRUE	},
	{ "detect_hidden",	AFF_DETECT_HIDDEN,	TRUE	},
	{ "detect_good",	AFF_DETECT_GOOD,	TRUE	},
	{ "sanctuary",		AFF_SANCTUARY,		TRUE	},
	{ "faerie_fire",	AFF_FAERIE_FIRE,	TRUE	},
	{ "infrared",		AFF_INFRARED,		TRUE	},
	{ "curse",		AFF_CURSE,		TRUE	},
	{ "corruption",		AFF_CORRUPTION,		TRUE	},
	{ "poison",		AFF_POISON,		TRUE	},
	{ "protect_evil",	AFF_PROTECT_EVIL,	TRUE	},
	{ "protect_good",	AFF_PROTECT_GOOD,	TRUE	},
	{ "sneak",		AFF_SNEAK,		TRUE	},
	{ "hide",		AFF_HIDE,		TRUE	},
	{ "sleep",		AFF_SLEEP,		TRUE	},
	{ "charm",		AFF_CHARM,		TRUE	},
	{ "flying",		AFF_FLYING,		TRUE	},
	{ "pass_door",		AFF_PASS_DOOR,		TRUE	},
	{ "haste",		AFF_HASTE,		TRUE	},
	{ "calm",		AFF_CALM,		TRUE	},
	{ "plague",		AFF_PLAGUE,		TRUE	},
	{ "weaken",		AFF_WEAKEN,		TRUE	},
	{ "dark_vision",	AFF_DARK_VISION,	TRUE	},
	{ "berserk",		AFF_BERSERK,		TRUE	},
	{ "swim",		AFF_SWIM,		TRUE	},
	{ "regeneration",	AFF_REGENERATION,	TRUE	},
	{ "slow",		AFF_SLOW,		TRUE	},
	{ "camouflage",		AFF_CAMOUFLAGE,		TRUE	},
	{ "imp_invis",		AFF_IMP_INVIS,		TRUE	},
	{ "fade",		AFF_FADE,		TRUE	},
	{ "scream",		AFF_SCREAM,		TRUE	},
	{ "bloodthirst",	AFF_BLOODTHIRST,	TRUE	},
	{ "stun",		AFF_STUN,		TRUE	},
	{ "weak_stun",		AFF_WEAK_STUN,		TRUE	},
	{ "detect_imp_invis",	AFF_DETECT_IMP_INVIS,		TRUE	},
	{ "detect_fade",	AFF_DETECT_FADE,	TRUE	},
	{ "detect_undead",	AFF_DETECT_UNDEAD,	TRUE	},
	{ "detect_fear",	AFF_DETECT_FEAR,	TRUE	},
	{ "detect_form_tree",	AFF_DETECT_FORM_TREE,	TRUE	},
	{ "detect_form_grass",	AFF_DETECT_FORM_GRASS,	TRUE	},
	{ "detect_web",		AFF_DETECT_WEB,		TRUE	},
	{ "detect_life",	AFF_DETECT_LIFE,	TRUE	},
	{ "acute_vision",	AFF_ACUTE_VISION,	TRUE	},
	{ NULL }
};

FLAG off_flags[] =
{
	{ "area_attack",	OFF_AREA_ATTACK,	TRUE	},
	{ "backstab",		OFF_BACKSTAB,		TRUE	},
	{ "bash",		OFF_BASH,		TRUE	},
	{ "berserk",		OFF_BERSERK,		TRUE	},
	{ "disarm",		OFF_DISARM,		TRUE	},
	{ "dodge",		OFF_DODGE,		TRUE	},
	{ "fade",		OFF_FADE,		TRUE	},
	{ "fast",		OFF_FAST,		TRUE	},
	{ "kick",		OFF_KICK,		TRUE	},
	{ "dirt_kick",		OFF_DIRT_KICK,		TRUE	},
	{ "parry",		OFF_PARRY,		TRUE	},
	{ "rescue",		OFF_RESCUE,		TRUE	},
	{ "tail",		OFF_TAIL,		TRUE	},
	{ "trip",		OFF_TRIP,		TRUE	},
	{ "crush",		OFF_CRUSH,		TRUE	},
	{ "assist_all",		ASSIST_ALL,		TRUE	},
	{ "assist_align",	ASSIST_ALIGN,		TRUE	},
	{ "assist_race",	ASSIST_RACE,		TRUE	},
	{ "assist_players",	ASSIST_PLAYERS,		TRUE	},
	{ "assist_guard",	ASSIST_GUARD,		TRUE	},
	{ "assist_vnum",	ASSIST_VNUM,		TRUE	},
	{ NULL }
};

FLAG imm_flags[] =
{
	{ "summon",		IMM_SUMMON,		TRUE	},
	{ "charm",		IMM_CHARM,		TRUE	},
	{ "magic",		IMM_MAGIC,		TRUE	},
	{ "weapon",		IMM_WEAPON,		TRUE	},
	{ "bash",		IMM_BASH,		TRUE	},
	{ "pierce",		IMM_PIERCE,		TRUE	},
	{ "slash",		IMM_SLASH,		TRUE	},
	{ "fire",		IMM_FIRE,		TRUE	},
	{ "cold",		IMM_COLD,		TRUE	},
	{ "lightning",		IMM_LIGHTNING,		TRUE	},
	{ "acid",		IMM_ACID,		TRUE	},
	{ "poison",		IMM_POISON,		TRUE	},
	{ "negative",		IMM_NEGATIVE,		TRUE	},
	{ "holy",		IMM_HOLY,		TRUE	},
	{ "energy",		IMM_ENERGY,		TRUE	},
	{ "mental",		IMM_MENTAL,		TRUE	},
	{ "disease",		IMM_DISEASE,		TRUE	},
	{ "drowning",		IMM_DROWNING,		TRUE	},
	{ "light",		IMM_LIGHT,		TRUE	},
	{ "sound",		IMM_SOUND,		TRUE	},
	{ "wood",		IMM_WOOD,		TRUE	},
	{ "silver",		IMM_SILVER,		TRUE	},
	{ "iron",		IMM_IRON,		TRUE	},
	{ "steal",		IMM_STEAL,		TRUE	},
	{ NULL }
};

FLAG form_flags[] =
{
	{ "edible",		FORM_EDIBLE,		TRUE	},
	{ "poison",		FORM_POISON,		TRUE	},
	{ "magical",		FORM_MAGICAL,		TRUE	},
	{ "instant_decay",	FORM_INSTANT_DECAY,	TRUE	},
	{ "other",		FORM_OTHER,		TRUE	},
	{ "animal",		FORM_ANIMAL,		TRUE	},
	{ "sentient",		FORM_SENTIENT,		TRUE	},
	{ "undead",		FORM_UNDEAD,		TRUE	},
	{ "construct",		FORM_CONSTRUCT,		TRUE	},
	{ "mist",		FORM_MIST,		TRUE	},
	{ "intangible",		FORM_INTANGIBLE,	TRUE	},
	{ "biped",		FORM_BIPED,		TRUE	},
	{ "centaur",		FORM_CENTAUR,		TRUE	},
	{ "insect",		FORM_INSECT,		TRUE	},
	{ "spider",		FORM_SPIDER,		TRUE	},
	{ "crustacean",		FORM_CRUSTACEAN,	TRUE	},
	{ "worm",		FORM_WORM,		TRUE	},
	{ "blob",		FORM_BLOB,		TRUE	},
	{ "mammal",		FORM_MAMMAL,		TRUE	},
	{ "bird",		FORM_BIRD,		TRUE	},
	{ "reptile",		FORM_REPTILE,		TRUE	},
	{ "snake",		FORM_SNAKE,		TRUE	},
	{ "dragon",		FORM_DRAGON,		TRUE	},
	{ "amphibian",		FORM_AMPHIBIAN,		TRUE	},
	{ "fish",		FORM_FISH ,		TRUE	},
	{ "cold_blood",		FORM_COLD_BLOOD,	TRUE	},
	{ NULL }
};

FLAG part_flags[] =
{
	{ "head",		PART_HEAD,		TRUE	},
	{ "arms",		PART_ARMS,		TRUE	},
	{ "legs",		PART_LEGS,		TRUE	},
	{ "heart",		PART_HEART,		TRUE	},
	{ "brains",		PART_BRAINS,		TRUE	},
	{ "guts",		PART_GUTS,		TRUE	},
	{ "hands",		PART_HANDS,		TRUE	},
	{ "feet",		PART_FEET,		TRUE	},
	{ "fingers",		PART_FINGERS,		TRUE	},
	{ "ear",		PART_EAR,		TRUE	},
	{ "eye",		PART_EYE,		TRUE	},
	{ "long_tongue",	PART_LONG_TONGUE,	TRUE	},
	{ "eyestalks",		PART_EYESTALKS,		TRUE	},
	{ "tentacles",		PART_TENTACLES,		TRUE	},
	{ "fins",		PART_FINS,		TRUE	},
	{ "wings",		PART_WINGS,		TRUE	},
	{ "tail",		PART_TAIL,		TRUE	},
	{ "claws",		PART_CLAWS,		TRUE	},
	{ "fangs",		PART_FANGS,		TRUE	},
	{ "horns",		PART_HORNS,		TRUE	},
	{ "scales",		PART_SCALES,		TRUE	},
	{ "tusks",		PART_TUSKS,		TRUE	},
	{ NULL }
};

FLAG comm_flags[] =
{
	{ "quiet",		COMM_QUIET,		TRUE	},
	{ "deaf",		COMM_DEAF,		TRUE	},
	{ "nowiz",		COMM_NOWIZ,		TRUE	},
	{ "nogossip",		COMM_NOGOSSIP,		TRUE	},
	{ "noquestion",		COMM_NOQUESTION,	TRUE	},
	{ "noauction",		COMM_NOAUCTION,		TRUE	},
	{ "nomusic",		COMM_NOMUSIC,		TRUE	},
	{ "noquote",		COMM_NOQUOTE,		TRUE	},
	{ "shoutsoff",		COMM_SHOUTSOFF,		TRUE	},
	{ "true_trust",		COMM_TRUE_TRUST,	TRUE	},
	{ "compact",		COMM_COMPACT,		TRUE	},
	{ "brief",		COMM_BRIEF,		TRUE	},
	{ "prompt",		COMM_PROMPT,		TRUE	},
	{ "long",		COMM_LONG,		TRUE	},
	{ "combine",		COMM_COMBINE,		TRUE	},
	{ "telnet_ga",		COMM_TELNET_GA,		TRUE	},
	{ "show_affects",	COMM_SHOW_AFFECTS,	TRUE	},
	{ "nograts",		COMM_NOGRATS,		TRUE	},
	{ "noemote",		COMM_NOEMOTE,		FALSE	},
	{ "noshout",		COMM_NOSHOUT,		FALSE	},
	{ "notell",		COMM_NOTELL,		FALSE	},
	{ "nochannels",		COMM_NOCHANNELS,	FALSE	},
	{ "snoop_proof",	COMM_SNOOP_PROOF,	FALSE	},
	{ "afk",		COMM_AFK,		TRUE	},
	{ "notelnet",		COMM_NOTELNET,		TRUE	},
	{ "noiac",		COMM_NOIAC,		TRUE	},
	{ NULL }
};

FLAG clan_flags[] =
{
	{ "hidden",		CLAN_HIDDEN,		TRUE	},
	{ "changed",		CLAN_CHANGED,		FALSE	},
	{ NULL }
};

FLAG area_flags[] =
{
	{ "changed",		AREA_CHANGED,		FALSE	},
	{ "hometown",		AREA_HOMETOWN,		TRUE	},
	{ NULL }
};

FLAG sex_table[] =
{
	{ "none",		SEX_NEUTRAL,		TRUE	},
	{ "male",		SEX_MALE,		TRUE	},
	{ "female",		SEX_FEMALE,		TRUE	},
	{ "either",		SEX_EITHER,		TRUE	},
	{ "random",		SEX_EITHER,		TRUE    },
	{ "neutral",		SEX_NEUTRAL,		TRUE	},
	{ NULL	}
};

FLAG exit_flags[] =
{
	{ "door",		EX_ISDOOR,		TRUE    },
	{ "closed",		EX_CLOSED,		TRUE	},
	{ "locked",		EX_LOCKED,		TRUE	},
	{ "noflee",		EX_NOFLEE,		TRUE	},
	{ "pickproof",		EX_PICKPROOF,		TRUE	},
	{ "nopass",		EX_NOPASS,		TRUE	},
	{ "easy",		EX_EASY,		TRUE	},
	{ "hard",		EX_HARD,		TRUE	},
	{ "infuriating",	EX_INFURIATING,		TRUE	},
	{ "noclose",		EX_NOCLOSE,		TRUE	},
	{ "nolock",		EX_NOLOCK,		TRUE	},
	{ "bitval",		EX_BITVAL,		FALSE	},
	{ NULL }
};

FLAG door_resets[] =
{
	{ "open",			0,		TRUE	},
	{ "closed",			1,		TRUE	},
	{ "closed and locked",		2,		TRUE	},
	{ NULL }
};

FLAG room_flags[] =
{
	{ "dark",		ROOM_DARK,		TRUE	},
	{ "nomob",		ROOM_NOMOB,		TRUE	},
	{ "indoors",		ROOM_INDOORS,		TRUE	},
	{ "private",		ROOM_PRIVATE,		TRUE    },
	{ "safe",		ROOM_SAFE,		TRUE	},
	{ "solitary",		ROOM_SOLITARY,		TRUE	},
	{ "pet_shop",		ROOM_PET_SHOP,		TRUE	},
	{ "norecall",		ROOM_NORECALL,		TRUE	},
	{ "imp_only",		ROOM_IMP_ONLY,		TRUE    },
	{ "gods_only",	        ROOM_GODS_ONLY,		TRUE    },
	{ "heroes_only"	,	ROOM_HEROES_ONLY,	TRUE	},
	{ "newbies_only",	ROOM_NEWBIES_ONLY,	TRUE	},
	{ "law",		ROOM_LAW,		TRUE	},
	{ "nowhere",		ROOM_NOWHERE,		TRUE	},
	{ "bank",		ROOM_BANK,		TRUE	},
	{ "nomagic",		ROOM_NOMAGIC,		TRUE	},
	{ "nosummon",		ROOM_NOSUMMON,		TRUE	},
	{ "battle_arena",	ROOM_BATTLE_ARENA,	TRUE	},
	{ "registry",		ROOM_REGISTRY,		TRUE	},
	{ NULL }
};

FLAG sector_types[] =
{
	{ "inside",		SECT_INSIDE,		TRUE	},
	{ "city",		SECT_CITY,		TRUE	},
	{ "field",		SECT_FIELD,		TRUE	},
	{ "forest",		SECT_FOREST,		TRUE	},
	{ "hills",		SECT_HILLS,		TRUE	},
	{ "mountain",		SECT_MOUNTAIN,		TRUE	},
	{ "swim",		SECT_WATER_SWIM,	TRUE	},
	{ "noswim",		SECT_WATER_NOSWIM,	TRUE	},
	{ "unused",		SECT_UNUSED,		TRUE	},
	{ "air",		SECT_AIR,		TRUE	},
	{ "desert",		SECT_DESERT,		TRUE	},
	{ NULL }
};

FLAG item_types[] =
{
	{ "light",		ITEM_LIGHT,		TRUE	},
	{ "scroll",		ITEM_SCROLL,		TRUE	},
	{ "wand",		ITEM_WAND,		TRUE	},
	{ "staff",		ITEM_STAFF,		TRUE	},
	{ "weapon",		ITEM_WEAPON,		TRUE	},
	{ "treasure",		ITEM_TREASURE,		TRUE	},
	{ "armor",		ITEM_ARMOR,		TRUE	},
	{ "potion",		ITEM_POTION,		TRUE	},
	{ "clothing",		ITEM_CLOTHING,		TRUE	},
	{ "furniture",		ITEM_FURNITURE,		TRUE	},
	{ "trash",		ITEM_TRASH,		TRUE	},
	{ "container",		ITEM_CONTAINER,		TRUE	},
	{ "drink",		ITEM_DRINK_CON,		TRUE	},
	{ "key",		ITEM_KEY,		TRUE	},
	{ "food",		ITEM_FOOD,		TRUE	},
	{ "money",		ITEM_MONEY,		TRUE	},
	{ "boat",		ITEM_BOAT,		TRUE	},
	{ "npc_corpse",		ITEM_CORPSE_NPC,	TRUE	},
	{ "pc_corpse",		ITEM_CORPSE_PC,		FALSE	},
	{ "fountain",		ITEM_FOUNTAIN,		TRUE	},
	{ "pill",		ITEM_PILL,		TRUE	},
	{ "protect",		ITEM_PROTECT,		TRUE	},
	{ "map",		ITEM_MAP,		TRUE	},
	{ "portal",		ITEM_PORTAL,		TRUE	},
	{ "warp_stone",		ITEM_WARP_STONE,	TRUE	},
	{ "room_key",		ITEM_ROOM_KEY,		TRUE	},
	{ "gem",		ITEM_GEM,		TRUE	},
	{ "jewelry",		ITEM_JEWELRY,		TRUE	},
	{ "jukebox",		ITEM_JUKEBOX,		TRUE	},
	{ "tattoo",		ITEM_TATTOO,		FALSE	},
	{ NULL }
};

FLAG extra_flags[] =
{
	{ "glow",		ITEM_GLOW,		TRUE	},
	{ "hum",		ITEM_HUM,		TRUE	},
	{ "dark",		ITEM_DARK,		TRUE	},
	{ "lock",		ITEM_LOCK,		TRUE	},
	{ "evil",		ITEM_EVIL,		TRUE	},
	{ "invis",		ITEM_INVIS,		TRUE	},
	{ "magic",		ITEM_MAGIC,		TRUE	},
	{ "nodrop",		ITEM_NODROP,		TRUE	},
	{ "bless",		ITEM_BLESS,		TRUE	},
	{ "antigood",		ITEM_ANTI_GOOD,		TRUE	},
	{ "antievil",		ITEM_ANTI_EVIL,		TRUE	},
	{ "antineutral",	ITEM_ANTI_NEUTRAL,	TRUE	},
	{ "noremove",		ITEM_NOREMOVE,		TRUE	},
	{ "inventory",		ITEM_INVENTORY,		TRUE	},
	{ "nopurge",		ITEM_NOPURGE,		TRUE	},
	{ "rotdeath",		ITEM_ROT_DEATH,		TRUE	},
	{ "visdeath",		ITEM_VIS_DEATH,		TRUE	},
	{ "nosac",		ITEM_NOSAC,		TRUE	},
	{ "nonmetal",		ITEM_NONMETAL,		TRUE	},
	{ "nolocate",		ITEM_NOLOCATE,		TRUE	},
	{ "meltdrop",		ITEM_MELT_DROP,		TRUE	},
	{ "hadtimer",		ITEM_HAD_TIMER,		TRUE	},
	{ "sellextract",	ITEM_SELL_EXTRACT,	TRUE	},
	{ "burnproof",		ITEM_BURN_PROOF,	TRUE	},
	{ "nouncurse",		ITEM_NOUNCURSE,		TRUE	},
	{ "nosell",		ITEM_NOSELL,		TRUE	},
	{ "not_edible",		ITEM_NOT_EDIBLE,	TRUE	},
	{ "quest",		ITEM_QUEST,		TRUE	},
	{ "broken",		ITEM_ENCHANTED,		FALSE	},
	{ NULL }
};

FLAG wear_flags[] =
{
	{ "take",		ITEM_TAKE,		TRUE	},
	{ "finger",		ITEM_WEAR_FINGER,	TRUE	},
	{ "neck",		ITEM_WEAR_NECK,		TRUE	},
	{ "body",		ITEM_WEAR_BODY,		TRUE	},
	{ "head",		ITEM_WEAR_HEAD,		TRUE	},
	{ "legs",		ITEM_WEAR_LEGS,		TRUE	},
	{ "feet",		ITEM_WEAR_FEET,		TRUE	},
	{ "hands",		ITEM_WEAR_HANDS,	TRUE	},
	{ "arms",		ITEM_WEAR_ARMS,		TRUE	},
	{ "shield",		ITEM_WEAR_SHIELD,	TRUE	},
	{ "about",		ITEM_WEAR_ABOUT,	TRUE	},
	{ "waist",		ITEM_WEAR_WAIST,	TRUE	},
	{ "wrist",		ITEM_WEAR_WRIST,	TRUE	},
	{ "wield",		ITEM_WIELD,		TRUE	},
	{ "hold",		ITEM_HOLD,		TRUE	},
	{ "nosac",		ITEM_NO_SAC,		TRUE	},
	{ "float_around",	ITEM_WEAR_FLOAT,	TRUE	},
/*	{ "twohands",		ITEM_TWO_HANDS,		TRUE	}, */
	{ "tattoo",		ITEM_WEAR_TATTOO,	TRUE	},
	{ NULL }
};

/*
* Used when adding an affect to tell where it goes.
* See addaffect and delaffect in act_olc.c
*/
FLAG apply_flags[] =
{
	{ "strength",		APPLY_STR,		TRUE	},
	{ "dexterity",		APPLY_DEX,		TRUE	},
	{ "intelligence",	APPLY_INT,		TRUE	},
	{ "wisdom",		APPLY_WIS,		TRUE	},
	{ "constitution",	APPLY_CON,		TRUE	},
	{ "charisma",		APPLY_CHA,		TRUE	},
	{ "sex",		APPLY_SEX,		TRUE	},
	{ "level",		APPLY_LEVEL,		TRUE	},
	{ "age",		APPLY_AGE,		TRUE	},
	{ "height",		APPLY_HEIGHT,		TRUE	},
	{ "weight",		APPLY_WEIGHT,		TRUE	},
	{ "mana",		APPLY_MANA,		TRUE	},
	{ "hp",			APPLY_HIT,		TRUE	},
	{ "move",		APPLY_MOVE,		TRUE	},
	{ "gold",		APPLY_GOLD,		TRUE	},
	{ "experience",		APPLY_EXP,		TRUE	},
	{ "ac",			APPLY_AC,		TRUE	},
	{ "hitroll",		APPLY_HITROLL,		TRUE	},
	{ "damroll",		APPLY_DAMROLL,		TRUE	},
	{ "saves",		APPLY_SAVES,		TRUE	},
	{ "savingpara",		APPLY_SAVING_PARA,	TRUE	},
	{ "savingrod",		APPLY_SAVING_ROD,	TRUE	},
	{ "savingpetri",	APPLY_SAVING_PETRI,	TRUE	},
	{ "savingbreath",	APPLY_SAVING_BREATH,	TRUE	},
	{ "savingspell",	APPLY_SAVING_SPELL,	TRUE	},
	{ "spellaffect",	APPLY_SPELL_AFFECT,	FALSE	},
	{ "size",		APPLY_SIZE,		TRUE	},
	{ "class",		APPLY_CLASS,		TRUE	},
	{ "race",		APPLY_RACE,		TRUE	},
	{ NULL }
};

FLAG rapply_flags[] =
{
	{ "healrate",		APPLY_ROOM_HEAL,	TRUE	},
	{ "manarate",		APPLY_ROOM_MANA,	TRUE	},
	{ NULL }
};

/*
 * What is seen.
 */
FLAG wear_loc_strings[] =
{
	{ "in the inventory",		WEAR_NONE,		TRUE	},
	{ "as a light",			WEAR_LIGHT,		TRUE	},
	{ "on the left finger",		WEAR_FINGER_L,		TRUE	},
	{ "on the right finger",	WEAR_FINGER_R,		TRUE	},
	{ "around the neck (1)",	WEAR_NECK_1,		TRUE	},
	{ "around the neck (2)",	WEAR_NECK_2,		TRUE	},
	{ "on the body",		WEAR_BODY,		TRUE	},
	{ "over the head",		WEAR_HEAD,		TRUE	},
	{ "on the legs",		WEAR_LEGS,		TRUE	},
	{ "on the feet",		WEAR_FEET,		TRUE	},
	{ "on the hands",		WEAR_HANDS,		TRUE	},
	{ "on the arms",		WEAR_ARMS,		TRUE	},
	{ "as a shield",		WEAR_SHIELD,		TRUE	},
	{ "about the shoulders",	WEAR_ABOUT,		TRUE	},
	{ "around the waist",		WEAR_WAIST,		TRUE	},
	{ "on the left wrist",		WEAR_WRIST_L,		TRUE	},
	{ "on the right wrist",		WEAR_WRIST_R,		TRUE	},
	{ "wielded",			WEAR_WIELD,		TRUE	},
	{ "held in the hands",		WEAR_HOLD,		TRUE	},
	{ "floating nearby",		WEAR_FLOAT,		TRUE	},
	{ "scratched tattoo",		WEAR_TATTOO,		TRUE	},
	{ "second wielded",		WEAR_SECOND_WIELD,	TRUE	},
	{ "stuck in",			WEAR_STUCK_IN,		TRUE	},
	{ NULL }
};

FLAG wear_loc_flags[] =
{
	{ "none",		WEAR_NONE,		TRUE	},
	{ "inventory",		WEAR_NONE,		TRUE	},
	{ "light",		WEAR_LIGHT,		TRUE	},
	{ "lfinger",		WEAR_FINGER_L,		TRUE	},
	{ "rfinger",		WEAR_FINGER_R,		TRUE	},
	{ "neck1",		WEAR_NECK_1,		TRUE	},
	{ "neck2",		WEAR_NECK_2,		TRUE	},
	{ "body",		WEAR_BODY,		TRUE	},
	{ "head",		WEAR_HEAD,		TRUE	},
	{ "legs",		WEAR_LEGS,		TRUE	},
	{ "feet",		WEAR_FEET,		TRUE	},
	{ "hands",		WEAR_HANDS,		TRUE	},
	{ "arms",		WEAR_ARMS,		TRUE	},
	{ "shield",		WEAR_SHIELD,		TRUE	},
	{ "about",		WEAR_ABOUT,		TRUE	},
	{ "waist",		WEAR_WAIST,		TRUE	},
	{ "lwrist",		WEAR_WRIST_L,		TRUE	},
	{ "rwrist",		WEAR_WRIST_R,		TRUE	},
	{ "wielded",		WEAR_WIELD,		TRUE	},
	{ "hold",		WEAR_HOLD,		TRUE	},
	{ "floating",		WEAR_FLOAT,		TRUE	},
	{ "tattoo",		WEAR_TATTOO,		TRUE	},
	{ "second",		WEAR_SECOND_WIELD,	TRUE	},
	{ "stuck",		WEAR_STUCK_IN,		TRUE	},
	{ NULL }
};

FLAG cont_flags[] =
{
	{ "closable",		CONT_CLOSEABLE,		TRUE	},
	{ "pickproof",		CONT_PICKPROOF,		TRUE	},
	{ "closed",		CONT_CLOSED,		TRUE	},
	{ "locked",		CONT_LOCKED,		TRUE	},
	{ "put_on",		CONT_PUT_ON,		TRUE	},
	{ "for_arrow",		CONT_FOR_ARROW,		TRUE	},
	{ NULL }
};

FLAG ac_type[] =
{
	{ "pierce",		AC_PIERCE,		TRUE	},
	{ "bash",		AC_BASH,		TRUE	},
	{ "slash",		AC_SLASH,		TRUE	},
	{ "exotic",		AC_EXOTIC,		TRUE	},
	{ NULL }
};

FLAG weapon_class[] =
{
	{ "exotic",		WEAPON_EXOTIC,		TRUE    },
	{ "sword",		WEAPON_SWORD,		TRUE    },
	{ "dagger",		WEAPON_DAGGER,		TRUE    },
	{ "spear",		WEAPON_SPEAR,		TRUE    },
	{ "staff",		WEAPON_SPEAR,		TRUE	}, /* compat */
	{ "mace",		WEAPON_MACE,		TRUE    },
	{ "axe",		WEAPON_AXE,		TRUE    },
	{ "flail",		WEAPON_FLAIL,		TRUE    },
	{ "whip",		WEAPON_WHIP,		TRUE    },
	{ "polearm",		WEAPON_POLEARM,		TRUE    },
	{ "bow",		WEAPON_BOW,		TRUE	},
	{ "arrow",		WEAPON_ARROW,		TRUE	},
	{ "lance",		WEAPON_LANCE,		TRUE	},
	{ NULL }
};

FLAG weapon_type2[] =
{
	{ "flaming",		WEAPON_FLAMING,		TRUE	},
	{ "frost",		WEAPON_FROST,		TRUE	},
	{ "vampiric",		WEAPON_VAMPIRIC,	TRUE	},
	{ "sharp",		WEAPON_SHARP,		TRUE	},
	{ "vorpal",		WEAPON_VORPAL,		TRUE	},
	{ "twohands",		WEAPON_TWO_HANDS,	TRUE	},
	{ "shocking",		WEAPON_SHOCKING,	TRUE	},
	{ "poison",		WEAPON_POISON,		TRUE	},
	{ "holy",		WEAPON_HOLY,		TRUE	},
	{ "katana",		WEAPON_KATANA,		TRUE	},
	{ NULL }
};

FLAG res_flags[] =
{
	{ "summon",		RES_SUMMON,		TRUE	},
	{ "charm",		RES_CHARM,		TRUE	},
	{ "magic",		RES_MAGIC,		TRUE	},
	{ "weapon",		RES_WEAPON,		TRUE	},
	{ "bash",		RES_BASH,		TRUE	},
	{ "pierce",		RES_PIERCE,		TRUE	},
	{ "slash",		RES_SLASH,		TRUE	},
	{ "fire",		RES_FIRE,		TRUE	},
	{ "cold",		RES_COLD,		TRUE	},
	{ "lightning",		RES_LIGHTNING,		TRUE	},
	{ "acid",		RES_ACID,		TRUE	},
	{ "poison",		RES_POISON,		TRUE	},
	{ "negative",		RES_NEGATIVE,		TRUE	},
	{ "holy",		RES_HOLY,		TRUE	},
	{ "energy",		RES_ENERGY,		TRUE	},
	{ "mental",		RES_MENTAL,		TRUE	},
	{ "disease",		RES_DISEASE,		TRUE	},
	{ "drowning",		RES_DROWNING,		TRUE	},
	{ "light",		RES_LIGHT,		TRUE	},
	{ "sound",		RES_SOUND,		TRUE	},
	{ "wood",		RES_WOOD,		TRUE	},
	{ "silver",		RES_SILVER,		TRUE	},
	{ "iron",		RES_IRON,		TRUE	},
	{ NULL }
};

FLAG vuln_flags[] =
{
	{ "summon",		VULN_SUMMON,		TRUE	},
	{ "charm",		VULN_CHARM,		TRUE	},
	{ "magic",		VULN_MAGIC,		TRUE	},
	{ "weapon",		VULN_WEAPON,		TRUE	},
	{ "bash",		VULN_BASH,		TRUE	},
	{ "pierce",		VULN_PIERCE,		TRUE	},
	{ "slash",		VULN_SLASH,		TRUE	},
	{ "fire",		VULN_FIRE,		TRUE	},
	{ "cold",		VULN_COLD,		TRUE	},
	{ "lightning",		VULN_LIGHTNING,		TRUE	},
	{ "acid",		VULN_ACID,		TRUE	},
	{ "poison",		VULN_POISON,		TRUE	},
	{ "negative",		VULN_NEGATIVE,		TRUE	},
	{ "holy",		VULN_HOLY,		TRUE	},
	{ "energy",		VULN_ENERGY,		TRUE	},
	{ "mental",		VULN_MENTAL,		TRUE	},
	{ "disease",		VULN_DISEASE,		TRUE	},
	{ "drowning",		VULN_DROWNING,		TRUE	},
	{ "light",		VULN_LIGHT,		TRUE	},
	{ "sound",		VULN_SOUND,		TRUE	},
	{ "wood",		VULN_WOOD,		TRUE	},
	{ "silver",		VULN_SILVER,		TRUE	},
	{ "iron",		VULN_IRON,		TRUE	},
	{ NULL }
};

FLAG position_table[] =
{
	{ "dead",		POS_DEAD,		TRUE	},
	{ "mort",		POS_MORTAL,		TRUE	},
	{ "incap",		POS_INCAP,		TRUE	},
	{ "stun",		POS_STUNNED,		TRUE	},
	{ "sleep",		POS_SLEEPING,		TRUE	},
	{ "rest",		POS_RESTING,		TRUE	},
	{ "sit",		POS_SITTING,		TRUE	},
	{ "fight",		POS_FIGHTING,		TRUE	},
	{ "stand",		POS_STANDING,		TRUE	},
	{ NULL }
};

FLAG position_names[] =
{
	{ "DEAD",		POS_DEAD	},
	{ "mortally wounded",	POS_MORTAL	},
	{ "incapacitated",	POS_INCAP	},
	{ "stunned",		POS_STUNNED	},
	{ "sleeping",		POS_SLEEPING	},
	{ "resting",		POS_RESTING	},
	{ "sitting",		POS_SITTING	},
	{ "fighting",		POS_FIGHTING	},
	{ "standing",		POS_STANDING	},
	{ NULL }
};

FLAG portal_flags[]=
{
	{ "normal_exit",	GATE_NORMAL_EXIT,	TRUE	},
	{ "no_curse",		GATE_NOCURSE,		TRUE	},
	{ "go_with",		GATE_GOWITH,		TRUE	},
	{ "buggy",		GATE_BUGGY,		TRUE	},
	{ "random",		GATE_RANDOM,		TRUE	},
	{ NULL }
};

FLAG furniture_flags[]=
{
	{ "stand_at",		STAND_AT,		TRUE	},
	{ "stand_on",		STAND_ON,		TRUE	},
	{ "stand_in",		STAND_IN,		TRUE	},
	{ "sit_at",		SIT_AT,			TRUE	},
	{ "sit_on",		SIT_ON,			TRUE	},
	{ "sit_in",		SIT_IN,			TRUE	},
	{ "rest_at",		REST_AT,		TRUE	},
	{ "rest_on",		REST_ON,		TRUE	},
	{ "rest_in",		REST_IN,		TRUE	},
	{ "sleep_at",		SLEEP_AT,		TRUE	},
	{ "sleep_on",		SLEEP_ON,		TRUE	},
	{ "sleep_in",		SLEEP_IN,		TRUE	},
	{ "put_at",		PUT_AT,			TRUE	},
	{ "put_on",		PUT_ON,			TRUE	},
	{ "put_in",		PUT_IN,			TRUE	},
	{ "put_inside",		PUT_INSIDE,		TRUE	},
	{ NULL }
};

FLAG apply_types[] =
{
	{ "affects",		TO_AFFECTS,		TRUE	},
/*	{ "object",		TO_OBJECT,		TRUE	}, */
	{ "immune",		TO_IMMUNE,		TRUE	},
	{ "resist",		TO_RESIST,		TRUE	},
	{ "vuln",		TO_VULN,		TRUE	},
/*	{ "weapon",		TO_WEAPON,		TRUE	}, */
	{ NULL }
};

FLAG raffect_flags[] =
{
	{ "shocking",		RAFF_SHOCKING,		TRUE	},
	{ "lshield",		RAFF_LSHIELD,		TRUE	},
	{ "thief_trap",		RAFF_THIEF_TRAP,	TRUE	},
	{ "randomizer",		RAFF_RANDOMIZER,	TRUE	},
	{ "espirit",		RAFF_ESPIRIT,		TRUE	},
	{ "curse",		RAFF_CURSE,		TRUE	},
	{ "poison",		RAFF_POISON,		TRUE	},
	{ "sleep",		RAFF_SLEEP,		TRUE	},
	{ "plague",		RAFF_PLAGUE,		TRUE	},
	{ "slow",		RAFF_SLOW,		TRUE	},
	{ NULL }
};

FLAG skill_groups[] =
{
	{ "none",		GROUP_NONE,		TRUE	},
	{ "weaponsmaster",	GROUP_WEAPONSMASTER,	TRUE	},
	{ "attack",		GROUP_ATTACK,		TRUE	},
	{ "beguiling",		GROUP_BEGUILING,	TRUE	},
	{ "benedictions",	GROUP_BENEDICTIONS,	TRUE	},
	{ "combat",		GROUP_COMBAT,		TRUE	},
	{ "creation",		GROUP_CREATION,		TRUE	},
	{ "curative",		GROUP_CURATIVE,		TRUE	},
	{ "detection",		GROUP_DETECTION,	TRUE	},
	{ "draconian",		GROUP_DRACONIAN,	TRUE	},
	{ "enchantment",	GROUP_ENCHANTMENT,	TRUE	},
	{ "enhancement",	GROUP_ENHANCEMENT,	TRUE	},
	{ "harmful",		GROUP_HARMFUL,		TRUE	},
	{ "healing",		GROUP_HEALING,		TRUE	},
	{ "illusion",		GROUP_ILLUSION,		TRUE	},
	{ "maladictions",	GROUP_MALADICTIONS,	TRUE	},
	{ "protective",		GROUP_PROTECTIVE,	TRUE	},
	{ "transportation",	GROUP_TRANSPORTATION,	TRUE	},
	{ "weather",		GROUP_WEATHER,		TRUE	},
	{ "fightmaster",	GROUP_FIGHTMASTER,	TRUE	},
	{ "suddendeath",	GROUP_SUDDENDEATH,	TRUE	},
	{ "meditation",		GROUP_MEDITATION,	TRUE	},
	{ "clan",		GROUP_CLAN,		TRUE	},
	{ "defensive",		GROUP_DEFENSIVE,	TRUE	},
	{ "wizard",		GROUP_WIZARD,		TRUE	},
	{ NULL }
};

FLAG mptrig_types[] =
{
	{ "act",		TRIG_ACT,		TRUE	},
	{ "bribe",		TRIG_BRIBE,		TRUE 	},
	{ "death",		TRIG_DEATH,		TRUE    },
	{ "entry",		TRIG_ENTRY,		TRUE	},
	{ "fight",		TRIG_FIGHT,		TRUE	},
	{ "give",		TRIG_GIVE,		TRUE	},
	{ "greet",		TRIG_GREET,		TRUE    },
	{ "grall",		TRIG_GRALL,		TRUE	},
	{ "kill",		TRIG_KILL,		TRUE	},
	{ "hpcnt",		TRIG_HPCNT,		TRUE    },
	{ "random",		TRIG_RANDOM,		TRUE	},
	{ "speech",		TRIG_SPEECH,		TRUE	},
	{ "exit",		TRIG_EXIT,		TRUE    },
	{ "exall",		TRIG_EXALL,		TRUE    },
	{ "delay",		TRIG_DELAY,		TRUE    },
	{ "surr",		TRIG_SURR,		TRUE    },
	{ NULL }
};

FLAG mptrig_flags[] =
{
	{ "case-sensitive",	TRIG_CASEDEP,		FALSE	},
	{ NULL }
};

FLAG skill_targets[] =
{
	{ "ignore",		TAR_IGNORE,		TRUE	},
	{ "charoff",		TAR_CHAR_OFFENSIVE,	TRUE	},
	{ "chardef",		TAR_CHAR_DEFENSIVE,	TRUE	},
	{ "charself",		TAR_CHAR_SELF,		TRUE	},
	{ "objinv",		TAR_OBJ_INV,		TRUE	},
	{ "objchardef",		TAR_OBJ_CHAR_DEF,	TRUE	},
	{ "objcharoff",		TAR_OBJ_CHAR_OFF,	TRUE	},
	{ NULL }
};

FLAG stat_names[] =
{
	{ "str",		STAT_STR,		TRUE	},
	{ "int",		STAT_INT,		TRUE	},
	{ "dex",		STAT_DEX,		TRUE	},
	{ "wis",		STAT_WIS,		TRUE	},
	{ "con",		STAT_CON,		TRUE	},
	{ "cha",		STAT_CHA,		TRUE	},
	{ NULL }
};

FLAG skill_flags[] =
{
	{ "clan",		SKILL_CLAN,		TRUE	},
	{ "range",		SKILL_RANGE,		TRUE	},
	{ NULL }
};

FLAG class_flags[] =
{
	{ "mage",		CLASS_MAGIC,		TRUE	},
	{ NULL }
};

FLAG align_names[] =
{
	{ "evil",		CR_EVIL,		TRUE	},
	{ "neutral",		CR_NEUTRAL,		TRUE	},
	{ "good",		CR_GOOD,		TRUE	},
	{ NULL }
};

FLAG lang_flags[] =
{
	{ "hidden",		LANG_HIDDEN,		TRUE	},
	{ "genders_changed",	LANG_GENDERS_CHANGED,	FALSE	},
	{ "cases_changed",	LANG_CASES_CHANGED,	FALSE	},
	{ "changed",		LANG_CHANGED,		FALSE	},
	{ NULL }
};

/* wiznet table and prototype for future flag setting */
const   struct wiznet_type      wiznet_table    []              =
{
   {    "on",           WIZ_ON,         IM },
   {    "prefix",	WIZ_PREFIX,	IM },
   {    "ticks",        WIZ_TICKS,      IM },
   {    "logins",       WIZ_LOGINS,     IM },
   {    "sites",        WIZ_SITES,      L4 },
   {    "links",        WIZ_LINKS,      L7 },
   {	"newbies",	WIZ_NEWBIE,	IM },
   {	"spam",		WIZ_SPAM,	L5 },
   {    "deaths",       WIZ_DEATHS,     IM },
   {    "resets",       WIZ_RESETS,     L4 },
   {    "mobdeaths",    WIZ_MOBDEATHS,  L4 },
   {    "flags",	WIZ_FLAGS,	L5 },
   {	"penalties",	WIZ_PENALTIES,	L5 },
   {	"saccing",	WIZ_SACCING,	L5 },
   {	"levels",	WIZ_LEVELS,	IM },
   {	"load",		WIZ_LOAD,	L2 },
   {	"restore",	WIZ_RESTORE,	L2 },
   {	"snoops",	WIZ_SNOOPS,	L2 },
   {	"switches",	WIZ_SWITCHES,	L2 },
   {	"secure",	WIZ_SECURE,	L1 },
   {	NULL,		0,		0  }
};

/* attack table */
const 	struct attack_type	attack_table	[]		=
{
	{ "none",	"hit",			-1		}, /*  0 */
	{ "slice",	"slice", 		DAM_SLASH	},	
	{ "stab",	"stab",			DAM_PIERCE	},
	{ "slash",	"slash",		DAM_SLASH	},
	{ "whip",	"whip",			DAM_SLASH	},
	{ "claw",	"claw",			DAM_SLASH	}, /*  5 */
	{ "blast",	"blast",		DAM_BASH	},
	{ "pound",	"pound",		DAM_BASH	},
	{ "crush",	"crush",		DAM_BASH	},
	{ "grep",	"grep",			DAM_SLASH	},
	{ "bite",	"bite",			DAM_PIERCE	}, /* 10 */
	{ "pierce",	"pierce",		DAM_PIERCE	},
	{ "suction",	"suction",		DAM_BASH	},
	{ "beating",	"beating",		DAM_BASH	},
	{ "digestion",	"digestion",		DAM_ACID	},
	{ "charge",	"charge",		DAM_BASH	}, /* 15 */
	{ "slap",	"slap",			DAM_BASH	},
	{ "punch",	"punch",		DAM_BASH	},
	{ "wrath",	"wrath",		DAM_ENERGY	},
	{ "magic",	"magic",		DAM_ENERGY	},
	{ "divine",	"divine power",		DAM_HOLY	}, /* 20 */
	{ "cleave",	"cleave",		DAM_SLASH	},
	{ "scratch",	"scratch",		DAM_PIERCE	},
	{ "peck",	"peck",			DAM_PIERCE	},
	{ "peckb",	"peck",			DAM_BASH	},
	{ "chop",	"chop",			DAM_SLASH	}, /* 25 */
	{ "sting",	"sting",		DAM_PIERCE	},
	{ "smash",	"smash",		DAM_BASH	},
	{ "shbite",	"shocking bite",	DAM_LIGHTNING	},
	{ "flbite",	"flaming bite",		DAM_FIRE	},
	{ "frbite",	"freezing bite",	DAM_COLD	}, /* 30 */
	{ "acbite",	"acidic bite",		DAM_ACID	},
	{ "chomp",	"chomp",		DAM_PIERCE	},
	{ "drain",	"life drain",		DAM_NEGATIVE	},
	{ "thrust",	"thrust",		DAM_PIERCE	},
	{ "slime",	"slime",		DAM_ACID	}, /* 35 */
	{ "shock",	"shock",		DAM_LIGHTNING	},
	{ "thwack",	"thwack",		DAM_BASH	},
	{ "flame",	"flame",		DAM_FIRE	},
	{ "chill",	"chill",		DAM_COLD	},
	{ "kiss",	"kiss",			DAM_MENTAL	}, /* 40 */
	{ NULL }
};

/* race table */
const 	struct	race_type	race_table	[]		=
{
/*
    {
	name,		pc_race?,
	act bits,	aff_by bits,	off bits,
	imm,		res,		vuln,
	form,		parts 
    },
*/
    { "unique",		FALSE, 0, 0, 0, 0, 0, 0, 0, 0 },

    { /* 1 */
	"human",	TRUE, 
	0,		0, 		0,
	0, 		0,		0,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    { /* 2 */
	"elf",		TRUE,
	0,		AFF_INFRARED|AFF_SNEAK,	0,
	0,		RES_CHARM,	VULN_IRON,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    { /* 3 */
        "half-elf",     TRUE,
        0,              AFF_INFRARED,   0,
        0,              RES_CHARM,      VULN_IRON,
        A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K
    },

    { /* 4 */
        "dark-elf",     TRUE, 	
        0,              AFF_INFRARED|AFF_SNEAK, 0,
        0,              RES_CHARM,      VULN_IRON,
        A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K
    },

    { /* 5 */
        "rockseer",     TRUE,
        0,              AFF_INFRARED|AFF_SNEAK|AFF_PASS_DOOR, 0,
        0,              RES_CHARM,      VULN_WOOD,
        A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K
    },

    { /* 6 */
	"dwarf",	TRUE,
	0,		AFF_INFRARED,	0,
	0,		RES_POISON|RES_DISEASE, VULN_DROWNING,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    {  /* 7 */
      "svirfnebli",     TRUE,
      0,                AFF_INFRARED,   0,
      0,                RES_MAGIC|RES_POISON|RES_DISEASE, VULN_BASH,
      A|H|M|V,          A|B|C|D|E|F|G|H|I|J|K
    },

    { /* 8 */
      "duergar",        TRUE,
      0,                AFF_INFRARED,   0,
      0,                RES_MAGIC|RES_POISON|RES_DISEASE, VULN_DROWNING,
      A|H|M|V,          A|B|C|D|E|F|G|H|I|J|K
    },

    { /* 9 */
        "arial",        TRUE,
        0,              AFF_FLYING,             0,
        0,              0,              0,
        A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K
    },

    { /* 10 */
        "gnome",        TRUE,
        0,              AFF_INFRARED,   0,
        0,              RES_POISON,     VULN_BASH,
        A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K
    },

    { /* 11 */
	"storm giant",	TRUE,
	0,		AFF_FLYING,		0,
	0,		RES_LIGHTNING,	VULN_MENTAL|VULN_COLD|VULN_FIRE,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    { /* 12 */
	"cloud giant",	TRUE,
	0,		AFF_FLYING,		0,
	0,		RES_WEAPON,	VULN_MENTAL|VULN_LIGHTNING,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    { /* 13 */
	"fire giant",	TRUE,
	0,		0,		0,
	0,		RES_FIRE,	VULN_MENTAL|VULN_LIGHTNING|VULN_COLD,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    { /* 14 */
	"frost giant",	TRUE,
	0,		0,		0,
	0,		RES_COLD,	VULN_MENTAL|VULN_LIGHTNING|VULN_FIRE,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    { /* 15 */
        "felar",        TRUE,
        0,              AFF_INFRARED,   OFF_TAIL,
        0,              RES_LIGHT|RES_COLD,     VULN_FIRE|VULN_DROWNING,
        A|H|M|V,        A|C|D|E|F|H|J|K|Q|U|V
    },

    { /* 16 */
	"githyanki",	TRUE,
	0,		AFF_INFRARED,	0,
	0, 		0,		0,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    { /* 17 */
	"satyr",	TRUE,
	0,		AFF_INFRARED,	0,
	0,		RES_WOOD|RES_DISEASE,	0,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    {  /* 18 */
	"troll",	TRUE,
	0,		AFF_REGENERATION|AFF_INFRARED,	OFF_BERSERK,
 	0,	RES_CHARM|RES_BASH,	VULN_FIRE|VULN_ACID,
	A|B|H|M|V,		A|B|C|D|E|F|G|H|I|J|K|U|V
    },

    { /* 19 */
        "black dragon", FALSE,
	0, 		AFF_INFRARED|AFF_FLYING,	0,
	0,		RES_BASH|RES_CHARM|RES_ACID,	VULN_PIERCE,
        A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K
    },

    { /* 20 */
        "blue dragon", 	FALSE,
	0, 		AFF_INFRARED|AFF_FLYING,	0,
	0,		RES_BASH|RES_CHARM|RES_LIGHTNING, 
	VULN_PIERCE,
        A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K
    },

    { /* 21 */
        "green dragon", FALSE,
	0, 		AFF_INFRARED|AFF_FLYING,	0,
	0,		RES_BASH|RES_CHARM|RES_POISON,	VULN_PIERCE,
        A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K
    },

    { /* 22 */
        "red dragon", 	FALSE,
	0, 		AFF_INFRARED|AFF_FLYING,	0,
	0,		RES_BASH|RES_CHARM|RES_FIRE,	VULN_PIERCE,
        A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K
    },

    { /* 23 */
        "white dragon", FALSE,
	0, 		AFF_INFRARED|AFF_FLYING,	0,
	0,		RES_BASH|RES_CHARM|RES_COLD,	VULN_PIERCE,
        A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K
    },

    { /* 24 */
        "brass dragon", FALSE,
	0, 		AFF_INFRARED|AFF_FLYING,	0,
	0,		RES_BASH|RES_CHARM|RES_FIRE,	VULN_PIERCE,
        A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K
    },

    { /* 25 */
        "gold dragon", 	FALSE,
	0, 		AFF_INFRARED|AFF_FLYING,	0,
	0,		RES_BASH|RES_CHARM|RES_FIRE|RES_POISON, 
	VULN_PIERCE,
        A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K
    },

    { /* 26 */
        "silver dragon",FALSE,
	0, 		AFF_INFRARED|AFF_FLYING,	0,
	0,		RES_BASH|RES_CHARM|RES_COLD,	VULN_PIERCE,
        A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K
    },

    { /* 27 */
        "bronze dragon",FALSE,
	0, 		AFF_INFRARED|AFF_FLYING,	0,
	0,		RES_BASH|RES_CHARM|RES_LIGHTNING, 
	VULN_PIERCE,
        A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K
    },

    { /* 28 */
        "copper dragon",FALSE,
	0, 		AFF_INFRARED|AFF_FLYING,	0,
	0,		RES_BASH|RES_CHARM|RES_ACID,	VULN_PIERCE,
        A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K
    },

    { /* 29 */
	"bat",		FALSE,
	0,		AFF_FLYING|AFF_DARK_VISION,	OFF_DODGE|OFF_FAST,
	0,		0,		VULN_LIGHT,
	A|G|V,		A|C|D|E|F|H|J|K|P
    },

    { /* 30 */
	"bear",		FALSE, 
	0,		0,		OFF_CRUSH|OFF_DISARM|OFF_BERSERK,
	0,		RES_BASH|RES_COLD,	0,
	A|G|V,		A|B|C|D|E|F|H|J|K|U|V
    },

    {  /* 31 */
	"cat",		FALSE,
	0,		AFF_DARK_VISION,	OFF_FAST|OFF_DODGE,
	0,		0,		0,
	A|G|V,		A|C|D|E|F|H|J|K|Q|U|V
    },

    {  /* 32 */
	"centipede",	FALSE,
	0,		AFF_DARK_VISION,	0,
	0,		RES_PIERCE|RES_COLD,	VULN_BASH,
 	A|B|G|O,		A|C|K	
    },

    {  /* 33 */
	"dog",		FALSE,
	0,		0,		OFF_FAST,
	0,		0,		0,
	A|G|V,		A|C|D|E|F|H|J|K|U|V
    },

    { /* 34 */
	"doll",		FALSE,
	0,		0,		0,
	IMM_COLD|IMM_POISON|IMM_HOLY|IMM_NEGATIVE|IMM_MENTAL|IMM_DISEASE
	|IMM_DROWNING,	RES_BASH|RES_LIGHT,
	VULN_SLASH|VULN_FIRE|VULN_ACID|VULN_LIGHTNING|VULN_ENERGY,
	E|J|M|cc,	A|B|C|G|H|K
    },

    { /* 35 */
	"fido",		FALSE,
	0,		0,		OFF_DODGE|ASSIST_RACE,
	0,		0,			VULN_MAGIC,
	A|B|G|V,	A|C|D|E|F|H|J|K|Q|V
    },		
   
    { /* 36 */
	"fox",		FALSE,
	0,		AFF_DARK_VISION,	OFF_FAST|OFF_DODGE,
	0,		0,		0,
	A|G|V,		A|C|D|E|F|H|J|K|Q|V
    },

    { /* 37 */
	"goblin",	FALSE,
	0,		AFF_INFRARED,	0,
	0,		RES_DISEASE,	VULN_MAGIC,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    { /* 38 */
	"hobgoblin",	FALSE,
	0,		AFF_INFRARED,	0,
	0,		RES_DISEASE|RES_POISON,	0,
	A|H|M|V,        A|B|C|D|E|F|G|H|I|J|K|Y
    },

    {  /* 39 */
	"kobold",	FALSE,
	0,		AFF_INFRARED,	0,
	0,		RES_POISON,	VULN_MAGIC,
	A|B|H|M|V,	A|B|C|D|E|F|G|H|I|J|K|Q
    },

    {  /* 40 */
	"lizard",	FALSE,
	0,		0,		0,
	0,		RES_POISON,	VULN_COLD,
	A|G|X|cc,	A|C|D|E|F|H|K|Q|V
    },

    {  /* 41 */
	"modron",	FALSE,
	0,		AFF_INFRARED,	ASSIST_RACE|ASSIST_ALIGN,
	IMM_CHARM|IMM_DISEASE|IMM_MENTAL|IMM_HOLY|IMM_NEGATIVE,
			RES_FIRE|RES_COLD|RES_ACID,	0,
	H,		A|B|C|G|H|J|K
    },

    {  /* 42 */
	"orc",		FALSE,
	0,		AFF_INFRARED,	0,
	0,		RES_DISEASE,	VULN_LIGHT,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    {  /* 43 */
	"pig",		FALSE,
	0,		0,		0,
	0,		0,		0,
	A|G|V,	 	A|C|D|E|F|H|J|K
    },	

    { /* 44 */
	"rabbit",	FALSE,
	0,		0,		OFF_DODGE|OFF_FAST,
	0,		0,		0,
	A|G|V,		A|C|D|E|F|H|J|K
    },
    
    { /* 45 */
	"school monster",	FALSE,
	ACT_NOALIGN,		0,		0,
	IMM_CHARM|IMM_SUMMON,	0,		VULN_MAGIC,
	A|M|V,		A|B|C|D|E|F|H|J|K|Q|U
    },	

    {  /* 46 */
	"snake",	FALSE,
	0,		0,		0,
	0,		RES_POISON,	VULN_COLD,
	A|G|X|Y|cc,	A|D|E|F|K|L|Q|V|X
    },
 
    {  /* 47 */
	"song bird",	FALSE,
	0,		AFF_FLYING,		OFF_FAST|OFF_DODGE,
	0,		0,		0,
	A|G|W,		A|C|D|E|F|H|K|P
    },

    {  /* 48 */
	"water fowl",	FALSE,
	0,		AFF_SWIM|AFF_FLYING,	0,
	0,		RES_DROWNING,		0,
	A|G|W,		A|C|D|E|F|H|K|P
    },		
  
    {  /* 49 */
	"wolf",		FALSE,
	0,		/*AFF_DARK_VISION*/0,	OFF_FAST|OFF_DODGE,
	0,		0,		0,	
	A|G|V,		A|C|D|E|F|J|K|Q|V
    },

    {  /* 50 */
	"wyvern",	FALSE, 		
	0,		AFF_FLYING|AFF_DETECT_INVIS|AFF_DETECT_HIDDEN,	OFF_BASH|OFF_FAST|OFF_DODGE,
	IMM_POISON,	0,	VULN_LIGHT,
	A|B|G|Z,		A|C|D|E|F|H|J|K|Q|V|X
    },

    { /* 51 */
        "dragon", 	FALSE,
	0, 		AFF_INFRARED|AFF_FLYING,	0,
	0,		RES_FIRE|RES_BASH|RES_CHARM,	VULN_PIERCE|VULN_COLD,
	A|H|Z,		A|C|D|E|F|G|H|I|J|K|P|Q|U|V|X	
    },

    { /* 52 */
	"giant",	FALSE,
	0,		0,		0,
	0,		RES_FIRE|RES_COLD,	VULN_MENTAL|VULN_LIGHTNING,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    { /* 53 */
	"golem",	FALSE,
	0,		0,		0,
	IMM_POISON|IMM_DISEASE,		RES_WEAPON,	VULN_MENTAL|VULN_MAGIC,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    { /* 54 */
	"undead",	FALSE,
	0,		AFF_PASS_DOOR|AFF_INFRARED|AFF_DETECT_INVIS|AFF_DETECT_HIDDEN|AFF_DETECT_GOOD|AFF_DETECT_EVIL,	0,
	IMM_DROWNING|IMM_DISEASE|IMM_POISON,	RES_NEGATIVE|RES_ENERGY,
	VULN_SILVER|VULN_HOLY|VULN_LIGHT,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    { /* 55 */
	"drow",		FALSE,
	0,		AFF_INFRARED,	0,
	0,		RES_CHARM,		VULN_IRON,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    { /* 56 */
	"draconian",	FALSE,
	0,		0,		0,
	0,		RES_SLASH|RES_POISON,	VULN_COLD,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    { /* 57 */
	"sprite",	FALSE,
	0,		AFF_FLYING|AFF_DETECT_EVIL,		G,
	0,		0,			VULN_COLD|VULN_WEAPON,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    { /* 58 */
	"aarakocra",	FALSE,
	ACT_SCAVENGER,	AFF_FLYING,	OFF_FAST,
	0,		0,		0,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K|P
    },

    { /*  59 */
	"beholder",	FALSE,
	0,		AFF_REGENERATION,		0,
	0,		RES_MAGIC,		0,
	A|H|M|V,	A|E|K
    },

    {  /* 60 */
	"tiger",	FALSE,
	0,	/*AFF_DARK_VISION*/0, OFF_FAST|OFF_DODGE,
	0,		RES_WEAPON,		0,
	A|G|V,		A|C|D|E|F|H|J|K|Q|U|V
    },

    {  /* 61 */
	"lion",		FALSE,
	ACT_SENTINEL,	/*AFF_DARK_VISION*/0,	OFF_FAST|OFF_DODGE,
	0,		RES_WEAPON,		0,
	A|G|V,		A|C|D|E|F|H|J|K|Q|U|V
    },

    {  /* 62 */
	"centaur",	FALSE,
	0,		0, OFF_BASH|OFF_KICK|OFF_DIRT_KICK|OFF_DODGE,
	0,		RES_WEAPON,		0,
	A|G|N|V,		A|B|C|D|E|F|G|H|I|J|K|Q
    },

    {  /* 63 */
	"death knight",	FALSE,
	ACT_UNDEAD,	0, 	OFF_PARRY|OFF_DISARM,
	IMM_DISEASE|IMM_POISON|IMM_DROWNING,	RES_FIRE|RES_NEGATIVE,
	VULN_HOLY,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    { /* 65 */
        "dracolich", 	FALSE,
	0, 		AFF_INFRARED|AFF_FLYING,	0,
	IMM_CHARM|IMM_LIGHTNING|IMM_COLD,	RES_FIRE|RES_BASH, 
	VULN_PIERCE|VULN_COLD,
	A|H|Z,		A|C|D|E|F|G|H|I|J|K|P|Q|U|V|X	
    },

    {  /* 66 */
	"air elemental",FALSE,
	0,		0,		OFF_FADE,
	0,		RES_WEAPON|RES_SUMMON,		0,
	A|H|Z,		A|D	
    },

    {  /* 67 */
	"earth elemental",	FALSE,
	0,		0,	OFF_FADE,
	0,		RES_WEAPON|RES_SUMMON,		0,
	A|H|Z,		A|D	
    },

    {  /* 68 */
	"fire elemental",	FALSE,
	0,		0,	OFF_FADE,
	IMM_FIRE,		RES_WEAPON,		VULN_COLD,
	A|H|Z,		A|D	
    },

    {  /* 69 */
	"water elemental",	FALSE,
	0,		0,	OFF_FADE,
	0,		RES_WEAPON|RES_FIRE|RES_COLD,	0,
	A|H|Z,		A|D	
    },

    {  /* 70 */
	"fish",		FALSE,
	0,		AFF_SWIM,	0,
	0,		0,		VULN_COLD|VULN_FIRE,
	A|H|Z,		A|D|E|F|J|K|O|X	
    },

    {  /* 71 */
	"gargoyle",	FALSE,
	ACT_AGGRESSIVE,	AFF_FLYING,	OFF_FAST,
	0,		RES_WEAPON,	VULN_COLD|VULN_FIRE,
	A|G|W,		A|C|D|E|F|H|K|P
    },

    {  /* 72 */
	"ghost",	FALSE,
	ACT_UNDEAD,	AFF_FLYING,	OFF_FADE,
	IMM_DROWNING|IMM_NEGATIVE|IMM_DISEASE|IMM_POISON,
	RES_WEAPON, VULN_HOLY,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    {  /* 73 */
	"ghoul",	FALSE,
	0,		0,		0,
	IMM_DISEASE,	RES_CHARM,	VULN_MAGIC,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    {  /* 74 */		
	"gnoll",	FALSE,
	0,		AFF_DETECT_INVIS|AFF_DETECT_HIDDEN,	OFF_DISARM,
	0,		0,		0,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    {  /* 75 */		
	"harpy",	FALSE,
	0,		AFF_FLYING|AFF_DETECT_INVIS|AFF_DETECT_HIDDEN, OFF_DISARM,
	0,		RES_CHARM,		0,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K|P
    },

    {  /* 76 */		
	"lamia",	FALSE,
	ACT_AGGRESSIVE,	0,		OFF_FAST|OFF_DODGE,
	0,		RES_CHARM,		0,
	A|G|V,		A|B|C|D|E|F|G|H|I|J|K|Q|U
    },

    {  /* 77 */		
	"lich",		FALSE,
	0,		0,		OFF_FAST|OFF_DODGE,
	IMM_POISON|IMM_DISEASE|IMM_CHARM|IMM_LIGHTNING|IMM_COLD,	
	RES_WEAPON|RES_FIRE,	VULN_HOLY,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    {  /* 77 */		
	"minotaur",	FALSE,
	0,		0,		OFF_CRUSH|OFF_FAST|OFF_BASH,
	0,		0,		0,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K|W
    },

    {  /* 78 */		
	"ogre",		FALSE,
	0,		AFF_INFRARED,	0,
	0,		RES_DISEASE,	0,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    {  /* 79 */		
	"zombie",	FALSE,
	0,		AFF_SLOW,	0,
	IMM_MAGIC|IMM_POISON|IMM_CHARM,	RES_COLD, VULN_WEAPON,
	A|H|M|V,	A|B|C|D|E|F|G|H|I|J|K
    },

    {  /* 80 */
	"horse",	FALSE,
	0,		0,		OFF_FAST|OFF_DODGE,
	0,		0,		0,	
	A|G|V,		A|C|D|E|F|J|K|Q
    },

    {  /* 81 */
	"unique",	FALSE,
	0,		0,		0,
	0,		0,		0,		
	0,		0
    },

    {
	NULL, 0, 0, 0, 0, 0, 0
    }
};


const	struct	pc_race_type	pc_race_table	[MAX_PC_RACE]	=
{
	{
		"null race", "", 0,
		{ 100, 100, 100, 100 ,100,100,100,100,100,100,100,100,100},
		{ str_empty },
		{ 13, 13, 13, 13, 13 }, { 18, 18, 18, 18, 18 }, 0,0,0,0,0
	},
 
/*
    {
	"race name", 	short name, 	points,	{ class multipliers },
	{ bonus skills },
	{ stat mods },	{ max stats },	size , hp_bonus, mana_bonus, 
	  practice_bonus, align
    },
    {
	"giant",	"Giant",500,{ -1, 100, -1, 100,-1,100,-1,100 ,-1, -1,-1,-1,100},
	{ "bash","enhanced damage" },
	{ 2, -4, 0, -4, 2 ,0},	{ 25, 15, 17, 16, 23,25 }, SIZE_HUGE,
	20,-20,0,CR_NONE, SLANG_GIANT
    },

*/

      {
       "human",	"Human",	0,	{100,100,100,100,100,100,100,100,100 ,100,100,100,100},
       { str_empty },
       { 0, 0, 0, 0, 0 ,0},	{ 20, 20, 20, 20, 20,25 },SIZE_MEDIUM,
        0,0,0,CR_NONE,SLANG_HUMAN
      },

      { 	
	"elf",		" Elf ",500,{ 100,100,100,100,100,-1,-1,100,100,-1 ,-1,-1,-1}, 
	{ str_empty },
	{ -2, 5, 0, 2, -2,0 },	{ 18, 25, 21, 23, 16,25 }, SIZE_MEDIUM,
	-10,30,0,CR_GOOD,SLANG_ELVISH
	},

    {
	"half-elf",	"H-Elf",200,{ 100,100,100,100,100,100,100,100,100,-1 ,-1,100,100},
	{ str_empty },
	{ -1, 2, 0, 1, -1,0 },	{ 19, 21, 21, 21, 19 ,25}, SIZE_MEDIUM,
	-10,10,0,CR_NONE,SLANG_ELVISH
    },

    {
	"dark-elf",	"D-Elf",500,{ 100, 100, 100, 100,-1,100,100,-1,100 ,-1,-1,100,-1},
	{ str_empty },
	{ -2, 4, 0, 2, -2 ,0},	{ 18, 24, 21, 24, 16,25 }, SIZE_MEDIUM,
	-10,20,0,CR_EVIL,SLANG_ELVISH
    },

    {
	"rockseer","Rocks",500,{ 100, 100, 100, 100,-1,-1,100,100,100 ,-1,-1,-1,100},
	{ "wand", "meld into stone" },
	{ -1, 2, 0, 4, -2 ,0},	{ 18, 24, 21, 24, 16,25 }, SIZE_MEDIUM,
	-10,20,0,CR_NEUTRAL,SLANG_ELVISH
    },
    
    {
	"dwarf",	"Dwarf",250,{ 100, 100, -1, 100,-1,-1,-1,-1,100 ,-1,-1,-1,-1},
	{ "berserk" },
	{ 0, 0, 1, -4, 5,0 },	{ 21, 19, 19, 18, 25,25 }, SIZE_MEDIUM,
	30,0,0,CR_NONE,SLANG_DWARVISH
    },

    {
	"svirfnebli",	"Svirf",250,{ 100, 100, 100, 100,-1 ,-1,-1,-1 ,100, -1,-1,-1,-1},
	{ "lore", "identify" },
	{ 0, -1, 3, 0, 0,0 },	{ 20, 19, 23, 18, 19 ,25}, SIZE_SMALL,
	10,0,2,CR_NEUTRAL,SLANG_GNOMISH
    },

    {
	"duergar",	"Duerg",250,{ -1, 100, 100, 100,-1 ,100,-1,-1,-1 , -1,-1,100,-1},
	{ "berserk", "pick lock" },
	{ 2, -2, -1, 1, 1 ,0},	{ 22, 16, 18, 20, 21,25 }, SIZE_MEDIUM,
	25,-10,0,CR_EVIL,SLANG_DWARVISH
    },

    {
	"arial",	"Arial",250,{ 100, 100, 100, 100,-1,100,100,-1,100 ,-1,-1,100,-1},
	{ str_empty },
	{ -2, 3, 0, 5, -2 ,0},	{ 19, 23, 20, 25, 18,25 }, SIZE_MEDIUM,
	-10,50,0,CR_NONE,SLANG_HUMAN
    },

    {
	"gnome",	"Gnome",300,{ 100, 100, 100, 100,-1,-1,-1,-1,100 ,-1,-1,-1,-1},
	{ "identify" },
	{ -2, 3, 5, 0, -2 ,0},	{ 18, 23, 25, 20, 18 ,25}, SIZE_SMALL,
	0,10,5,CR_NEUTRAL,SLANG_GNOMISH
    },

    {
	"storm giant",	"StGia",600,{ -1, 100, -1, 100,-1,-1,-1,100 ,-1,-1,-1,-1,100},
	{ "bash","enhanced damage" ,"sword" },
	{ 5, -2, -2, -6, 3 ,0},	{ 24, 17, 17, 17, 21,25 }, SIZE_HUGE,
	20,-20,0,CR_GOOD,SLANG_GIANT
    },

    {
	"cloud giant",	"ClGia",500,{ -1, 100, -1, 100,-1,-1,-1,100,-1,-1,-1,-1,100},
	{ "bash","enhanced damage" , "sword"},
	{ 5, -4, -3, -6, 3 ,0},	{ 25, 16, 17, 16, 22,25 }, SIZE_HUGE,
	20,-20,0,CR_NEUTRAL,SLANG_GIANT
    },

    {
	"fire giant",	"FiGia",500,{ -1, 100, -1, 100,-1,100,-1,100 ,-1, -1,-1,-1,100},
	{ "bash","enhanced damage" , "sword"},
	{ 5, -4, -3, -6, 3 ,0},	{ 25, 15, 17, 16, 23,25 }, SIZE_HUGE,
	20,-20,0,CR_EVIL,SLANG_GIANT
    },

    {
	"frost giant",	"FrGia",500,{ -1, 100, -1, 100,-1,100,-1,100 ,-1, -1,-1,-1,100},
	{ "bash","enhanced damage","axe" },
	{ 5, -4, -3, -6, 3 ,0},	{ 25, 15, 17, 16, 23,25 }, SIZE_HUGE,
	20,-20,0,CR_EVIL,SLANG_GIANT
    },


    {
	"felar",	"Felar",250,{ -1, -1, 100, 100,-1 ,100,100,100 ,100,-1,-1,-1,100},
	{ "hand to hand", "tail" },
	{ -1, -1, 0, 2, 2 ,0},	{ 17, 17, 20, 23, 23,25 }, SIZE_MEDIUM,
	10,-10,0,CR_NONE,SLANG_CAT
    },


 /*  {
	"dragon","Drago",750,{ 100, 100, -1,100,-1,-1,-1,-1,100,-1,-1,100,-1},
	{ "hand to hand" , "bash"},
	{ 2, 1, 0, -6, 2,0 },	{ 21, 19, 19, 18, 25,25 }, SIZE_GIANT,
	30,0,0,CR_NONE
    },

   {
	"black dragon","BlaDr",750,{ 100, 100, -1, 100,-1,-1,-1,-1,100 ,-1,-1,100,-1},
	{ "hand to hand" , "bash"},
	{ 2, 0, 0, -6, 2,0 },	{ 21, 19, 19, 18, 25,25 }, SIZE_GARGANTUAN,
	30,0,0,CR_EVIL
    },

    {
	"blue dragon","BluDr",750,{ 100, 100, -1, 100,-1,-1,-1,-1,100 ,-1,-1,100,-1},
	{ "hand to hand" , "bash"},
	{ 2, 1, 0, -6, 2,0 },	{ 21, 19, 19, 18, 25,25 }, SIZE_GARGANTUAN,
	30,0,0,CR_EVIL
    },

    {
	"green dragon","GReDr",750,{ 100, 100, -1, 100,-1,-1,-1,-1,100 ,-1,-1,100,-1},
	{ "hand to hand" , "bash"},
	{ 2, 1, 0, -6, 2,0 },	{ 21, 19, 19, 18, 25,25 }, SIZE_GARGANTUAN,
	30,0,0,CR_EVIL
    },

    {
	"red dragon","RedDr",1000,{ 100, 100, -1, 100,-1,-1,-1,-1,100 ,-1,-1,100,-1},
	{ "hand to hand" , "bash"},
	{ 2, 2, 0, -6, 2,0 },	{ 21, 19, 19, 18, 25,25 }, SIZE_GARGANTUAN,
	30,0,0,CR_EVIL
    },

    {
	"white dragon","WhiDr",750,{ 100, 100, -1, 100,-1,-1,-1,-1,100 ,-1,-1,100,-1},
	{ "hand to hand" , "bash"},
	{ 2, -2, 0, -6, 2,0 },	{ 21, 19, 19, 18, 25,25 }, SIZE_GIANT,
	30,0,0,CR_EVIL
    },

    {
	"brass dragon","WhiDr",750,{ 100, 100, -1, 100,-1,-1,-1,-1,100 ,-1,-1,100,-1},
	{ "hand to hand" , "bash"},
	{ 2, 0, 0, -6, 2,0 },	{ 21, 19, 19, 18, 25,25 }, SIZE_GARGANTUAN,
	30,0,0,CR_GOOD
    },

    {
	"gold dragon","WhiDr",1000,{ 100, 100, -1, 100,-1,-1,-1,-1,100 ,-1,-1,100,-1},
	{ "hand to hand" , "bash"},
	{ 2, 2, 0, -6, 2,0 },	{ 21, 19, 19, 18, 25,25 }, SIZE_GARGANTUAN,
	30,0,0,CR_GOOD
    },

    {
	"silver dragon","WhiDr",1000,{ 100, 100, -1, 100,-1,-1,-1,-1,100 ,-1,-1,100,-1},
	{ "hand to hand" , "bash"},
	{ 2, 2, 0, -6, 2,0 },	{ 21, 19, 19, 18, 25,25 }, SIZE_GARGANTUAN,
	30,0,0,CR_GOOD
    },

    {
	"bronze dragon","WhiDr",750,{ 100, 100, -1, 100,-1,-1,-1,-1,100 ,-1,-1,100,-1},
	{ "hand to hand" , "bash"},
	{ 2, 1, 0, -6, 2,0 },	{ 21, 19, 19, 18, 25,25 }, SIZE_GARGANTUAN,
	30,0,0,CR_GOOD
    },

    {
	"copper dragon","WhiDr",750,{ 100, 100, -1, 100,-1,-1,-1,-1,100 ,-1,-1,100,-1},
	{ "hand to hand" , "bash"},
	{ 2, 0, 0, -6, 2,0 },	{ 21, 19, 19, 18, 25,25 }, SIZE_GARGANTUAN,
	30,0,0,CR_GOOD
    },
*/
    {
	"githyanki",	"Githy",250,{ 100, -1, -1, 100,-1,-1,100,100,100 ,-1,-1,100,100},
	{ "sword" ,"dagger"},
	{ 0, 2, -2, 3, -1,0},	{ 19, 23, 20, 25, 18,25 }, SIZE_MEDIUM,
	0,0,0,CR_EVIL,SLANG_HUMAN
    },

    {
	"satyr","Satyr",250,{ -1, -1, 100, 100,-1,-1,-1,100,-1 ,-1,-1,-1,100},
	{ "camouflage" ,"ambush", "steal" },
	{ -1, 0, 0, 3, 0,0},	{ 19, 23, 20, 25, 18,25 }, SIZE_MEDIUM,
	0,0,0,CR_NEUTRAL,SLANG_HUMAN
    },

    {
	"troll","Troll",400,{ -1,-1,-1,100,-1,100,-1,100,-1,-1,-1,-1,100},
	{ "fast healing","trance" },
	{ 2, -2, -1, -2, 3,0},	{ 19, 23, 20, 25, 18,25 }, SIZE_LARGE,
	0,0,0,CR_EVIL,SLANG_TROLLISH
    }

  };

const struct ethos_type ethos_table [] =
{
  { "Null" },
  { "Lawful" },
  { "Neutral" },
  { "Chaotic" }
};

/*
 * Attribute bonus tables.
 */
const	struct	str_app_type	str_app		[26]		=
{
    { -5, -4,   0,  0 },  /* 0  */
    { -5, -4,   3,  1 },  /* 1  */
    { -3, -2,   3,  2 },
    { -3, -1,  10,  3 },  /* 3  */
    { -2, -1,  25,  4 },
    { -2, -1,  55,  5 },  /* 5  */
    { -1,  0,  80,  6 },
    { -1,  0,  90,  7 },
    {  0,  0, 100,  8 },
    {  0,  0, 100,  9 },
    {  0,  0, 115, 10 }, /* 10  */
    {  0,  0, 115, 11 },
    {  0,  0, 130, 12 },
    {  0,  0, 130, 13 }, /* 13  */
    {  0,  1, 140, 14 },
    {  1,  1, 150, 15 }, /* 15  */
    {  1,  2, 165, 16 },
    {  2,  3, 180, 22 },
    {  2,  3, 200, 25 }, /* 18  */
    {  3,  4, 225, 30 },
    {  3,  5, 250, 35 }, /* 20  */
    {  4,  6, 300, 40 },
    {  4,  6, 350, 45 },
    {  5,  7, 400, 50 },
    {  5,  8, 450, 55 },
    {  6,  9, 500, 60 }  /* 25   */
};



const	struct	int_app_type	int_app		[26]		=
{
    {  3 },	/*  0 */
    {  5 },	/*  1 */
    {  7 },
    {  8 },	/*  3 */
    {  9 },
    { 10 },	/*  5 */
    { 11 },
    { 12 },
    { 13 },
    { 15 },
    { 17 },	/* 10 */
    { 19 },
    { 22 },
    { 25 },
    { 28 },
    { 31 },	/* 15 */
    { 34 },
    { 37 },
    { 40 },	/* 18 */
    { 44 },
    { 49 },	/* 20 */
    { 55 },
    { 60 },
    { 70 },
    { 80 },
    { 85 }	/* 25 */
};



const	struct	wis_app_type	wis_app		[26]		=
{
    { 0 },	/*  0 */
    { 0 },	/*  1 */
    { 0 },
    { 0 },	/*  3 */
    { 0 },
    { 1 },	/*  5 */
    { 1 },
    { 1 },
    { 1 },
    { 1 },
    { 1 },	/* 10 */
    { 1 },
    { 1 },
    { 1 },
    { 1 },
    { 2 },	/* 15 */
    { 2 },
    { 2 },
    { 3 },	/* 18 */
    { 3 },
    { 3 },	/* 20 */
    { 3 },
    { 4 },
    { 4 },
    { 4 },
    { 5 }	/* 25 */
};



const	struct	dex_app_type	dex_app		[26]		=
{
    {   60 },   /* 0 */
    {   50 },   /* 1 */
    {   50 },
    {   40 },
    {   30 },
    {   20 },   /* 5 */
    {   10 },
    {    0 },
    {    0 },
    {    0 },
    {    0 },   /* 10 */
    {    0 },
    {    0 },
    {    0 },
    {    0 },
    { - 10 },   /* 15 */
    { - 15 },
    { - 20 },
    { - 30 },
    { - 40 },
    { - 50 },   /* 20 */
    { - 60 },
    { - 75 },
    { - 90 },
    { -105 },
    { -120 }    /* 25 */
};


const	struct	con_app_type	con_app		[26]		=
{
    {  0, 20 },   /*  0 */
    {  1, 25 },   /*  1 */
    {  1, 30 },
    {  2, 35 },	  /*  3 */
    {  3, 40 },
    {  4, 45 },   /*  5 */
    {  5, 50 },
    {  6, 55 },
    {  7, 60 },
    {  8, 65 },
    {  9, 70 },   /* 10 */
    { 10, 75 },
    { 11, 80 },
    { 12, 85 },
    { 13, 88 },
    { 14, 90 },   /* 15 */
    { 15, 95 },
    { 16, 97 },
    { 17, 99 },   /* 18 */
    { 18, 99 },
    { 19, 99 },   /* 20 */
    { 20, 99 },
    { 21, 99 },
    { 22, 99 },
    { 23, 99 },
    { 24, 99 }    /* 25 */
};


/*
 * Liquid properties.
 * Used in world.obj.
 */
const	struct	liq_type	liq_table	[]	=
{
/*    name			color	proof, full, thirst, food, ssize */
    { "water",			"clear",	{   0, 1, 10, 0, 16 }	},
    { "beer",			"amber",	{  12, 1,  8, 1, 12 }	},
    { "red wine",		"burgundy",	{  30, 1,  8, 1,  5 }	},
    { "ale",			"brown",	{  15, 1,  8, 1, 12 }	},
    { "dark ale",		"dark",		{  16, 1,  8, 1, 12 }	},

    { "whisky",			"golden",	{ 120, 1,  5, 0,  2 }	},
    { "lemonade",		"pink",		{   0, 1,  9, 2, 12 }	},
    { "firebreather",		"boiling",	{ 190, 0,  4, 0,  2 }	},
    { "local specialty",	"clear",	{ 151, 1,  3, 0,  2 }	},
    { "slime mold juice",	"green",	{   0, 2, -8, 1,  2 }	},

    { "milk",			"white",	{   0, 2,  9, 3, 12 }	},
    { "tea",			"tan",		{   0, 1,  8, 0,  6 }	},
    { "coffee",			"black",	{   0, 1,  8, 0,  6 }	},
    { "blood",			"red",		{   0, 2, -1, 2,  6 }	},
    { "salt water",		"clear",	{   0, 1, -2, 0,  1 }	},

    { "coke",			"brown",	{   0, 2,  9, 2, 12 }	}, 
    { "root beer",		"brown",	{   0, 2,  9, 2, 12 }   },
    { "elvish wine",		"green",	{  35, 2,  8, 1,  5 }   },
    { "white wine",		"golden",	{  28, 1,  8, 1,  5 }   },
    { "champagne",		"golden",	{  32, 1,  8, 1,  5 }   },

    { "mead",			"honey-colored",{  34, 2,  8, 2, 12 }   },
    { "rose wine",		"pink",		{  26, 1,  8, 1,  5 }	},
    { "benedictine wine",	"burgundy",	{  40, 1,  8, 1,  5 }   },
    { "vodka",			"clear",	{ 130, 1,  5, 0,  2 }   },
    { "cranberry juice",	"red",		{   0, 1,  9, 2, 12 }	},

    { "orange juice",		"orange",	{   0, 2,  9, 3, 12 }   }, 
    { "absinthe",		"green",	{ 200, 1,  4, 0,  2 }	},
    { "brandy",			"golden",	{  80, 1,  5, 0,  4 }	},
    { "aquavit",		"clear",	{ 140, 1,  5, 0,  2 }	},
    { "schnapps",		"clear",	{  90, 1,  5, 0,  2 }   },

    { "icewine",		"purple",	{  50, 2,  6, 1,  5 }	},
    { "amontillado",		"burgundy",	{  35, 2,  8, 1,  5 }	},
    { "sherry",			"red",		{  38, 2,  7, 1,  5 }   },	
    { "framboise",		"red",		{  50, 1,  7, 1,  5 }   },
    { "rum",			"amber",	{ 151, 1,  4, 0,  2 }	},

    { "cordial",		"clear",	{ 100, 1,  5, 0,  2 }   },
    { NULL,			NULL,		{   0, 0,  0, 0,  0 }	}
};

WHERE_DATA where_table[] =
{
	{ TO_AFFECTS,	"affect",  affect_flags, "{c%s{x affect"	   },
	{ TO_IMMUNE,	"immune",  imm_flags,	 "immunity to {c%s{x"	   },
	{ TO_RESIST,	"resist",  res_flags,	 "resistance to {c%s{x"	   },
	{ TO_VULN,	"vuln",	   vuln_flags,	 "vulnerability to {c%s{x" },
	{ -1 }
};

WHERE_DATA *where_lookup(sflag_t where)
{
	WHERE_DATA *wd;

	for (wd = where_table; wd->where != -1; wd++)
		if (wd->where == where)
			return wd;
	return NULL;
}

