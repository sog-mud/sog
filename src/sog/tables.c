/*
 * $Id: tables.c,v 1.175 2001-11-01 13:21:46 kostik Exp $
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
#include <time.h>

#include <merc.h>
#include <lang.h>
#include <note.h>
#include <module_decl.h>
#include <vo_iter.h>
#include <mprog.h>

/*
 * first element of each flaginfo_t[] table describes type of values
 * in the table.
 */

flaginfo_t slang_table[] =
{
	{ "",			TABLE_INTVAL,		FALSE	},

	{ "common",		SLANG_COMMON,		TRUE	},
	{ "human",		SLANG_HUMAN,		TRUE	},
	{ "elvish",		SLANG_ELVISH,		TRUE	},
	{ "dwarvish",		SLANG_DWARVISH,		TRUE	},
	{ "gnomish",		SLANG_GNOMISH,		TRUE	},
	{ "giant",		SLANG_GIANT,		TRUE	},
	{ "trollish",		SLANG_TROLLISH,		TRUE	},
	{ "cat",		SLANG_CAT,		TRUE	},
	{ "mothertongue",	SLANG_MAX,		FALSE	},

	{ NULL, 0, FALSE }
};

flaginfo_t size_table[] =
{
	{ "",			TABLE_INTVAL,		FALSE	},

	{ "tiny",		SIZE_TINY,		TRUE	},
	{ "small",		SIZE_SMALL,		TRUE	},
	{ "medium",		SIZE_MEDIUM,		TRUE	},
	{ "large",		SIZE_LARGE,		TRUE	},
	{ "huge",		SIZE_HUGE,		TRUE	},
	{ "giant",		SIZE_GIANT,		TRUE	},
	{ "gargantuan",		SIZE_GARGANTUAN,	TRUE	},

	{ NULL, 0, FALSE }
};

flaginfo_t mob_act_flags[] =
{
	{ "",			TABLE_BITVAL,		FALSE	},

	{ "familiar",		ACT_FAMILIAR,		TRUE	},
	{ "sentinel",		ACT_SENTINEL,		TRUE	},
	{ "scavenger",		ACT_SCAVENGER,		TRUE	},
	{ "summoned",		ACT_SUMMONED,		TRUE	},
	{ "imm_summon",		ACT_IMMSUMMON,		TRUE	},
	{ "aggressive",		ACT_AGGRESSIVE,		TRUE	},
	{ "stay_area",		ACT_STAY_AREA,		TRUE	},
	{ "wimpy",		ACT_WIMPY,		TRUE	},
	{ "pet",		ACT_PET,		TRUE	},
	{ "imm_steal",		ACT_IMMSTEAL,		TRUE	},
	{ "hunter",		ACT_HUNTER,		TRUE	},
	{ "cleric",		ACT_CLERIC,		TRUE	},
	{ "mage",		ACT_MAGE,		TRUE	},
	{ "thief",		ACT_THIEF,		TRUE	},
	{ "warrior",		ACT_WARRIOR,		TRUE	},
	{ "noalign",		ACT_NOALIGN,		TRUE	},
	{ "nopurge",		ACT_NOPURGE,		TRUE	},
	{ "outdoors",		ACT_OUTDOORS,		TRUE	},
	{ "indoors",		ACT_INDOORS,		TRUE	},
	{ "rideable",		ACT_RIDEABLE,		TRUE	},
	{ "update_always",	ACT_UPDATE_ALWAYS,	TRUE	},
	{ "notrack",		ACT_NOTRACK,		TRUE	},

	{ NULL, 0, FALSE }
};

flaginfo_t mob_flags[] =
{
	{ "",			TABLE_BITVAL,		FALSE	},

	{ "changer",		MOB_CHANGER,		TRUE	},
	{ "gain",		MOB_GAIN,		TRUE	},
	{ "train",		MOB_TRAIN,		TRUE	},
	{ "practice",		MOB_PRACTICE,		TRUE	},
	{ "questor",		MOB_QUESTOR,		TRUE	},
	{ "repairman",		MOB_REPAIRMAN,		TRUE	},
	{ "sage",		MOB_SAGE,		TRUE	},
	{ "healer",		MOB_HEALER,		TRUE	},
	{ "clan_guard",		MOB_CLAN_GUARD,		TRUE	},
	{ "janitor",		MOB_JANITOR,		TRUE	},

	{ NULL, 0, FALSE }
};

flaginfo_t plr_flags[] =
{
	{ "",			TABLE_BITVAL,		FALSE	},

	{ "autolook",		PLR_AUTOLOOK,		FALSE	},
	{ "autoassist",		PLR_AUTOASSIST,		FALSE	},
	{ "autoexit",		PLR_AUTOEXIT,		FALSE	},
	{ "autoloot",		PLR_AUTOLOOT,		FALSE	},
	{ "autosac",		PLR_AUTOSAC,		FALSE	},
	{ "autogold",		PLR_AUTOGOLD,		FALSE	},
	{ "autosplit",		PLR_AUTOSPLIT,		FALSE	},
	{ "bought_pet",		PLR_BOUGHT_PET,		FALSE	},
	{ "holylight",		PLR_HOLYLIGHT,		FALSE	},
	{ "can_loot",		PLR_CANLOOT,		FALSE	},
	{ "nosummon",		PLR_NOSUMMON,		FALSE	},
	{ "nofollow",		PLR_NOFOLLOW,		FALSE	},
	{ "permit",		PLR_PERMIT,		TRUE	},
	{ "log",		PLR_LOG,		FALSE	},
	{ "freeze",		PLR_FREEZE,		FALSE	},
	{ "ghost",		PLR_GHOST,		FALSE	},
	{ "noexp",		PLR_NOEXP,		FALSE	},
	{ "hara_kiri",		PLR_HARA_KIRI,		FALSE	},

	{ NULL, 0, FALSE }
};

flaginfo_t affect_flags[] =
{
	{ "",			TABLE_BITVAL,		FALSE	},

	{ "blind",		AFF_BLIND,		TRUE	},
	{ "scream",		AFF_SCREAM,		TRUE	},
	{ "bloodthirst",	AFF_BLOODTHIRST,	TRUE	},
	{ "stun",		AFF_STUN,		TRUE	},
	{ "weak_stun",		AFF_WEAK_STUN,		TRUE	},
	{ "fear",		AFF_FEAR,		TRUE	},
	{ "web",		AFF_WEB,		TRUE	},
	{ "sanctuary",		AFF_SANCTUARY,		TRUE	},
	{ "faerie_fire",	AFF_FAERIE_FIRE,	TRUE	},
	{ "black_shroud",	AFF_BLACK_SHROUD,	TRUE	},
	{ "curse",		AFF_CURSE,		TRUE	},
	{ "corruption",		AFF_CORRUPTION,		TRUE	},
	{ "poison",		AFF_POISON,		TRUE	},
	{ "protect_evil",	AFF_PROTECT_EVIL,	TRUE	},
	{ "protect_good",	AFF_PROTECT_GOOD,	TRUE	},
	{ "turned",		AFF_TURNED,		TRUE	},
	{ "doomed",		AFF_DOOMED,		TRUE	},
	{ "sleep",		AFF_SLEEP,		TRUE	},
	{ "charm",		AFF_CHARM,		TRUE	},
	{ "flying",		AFF_FLYING,		TRUE	},
	{ "pass_door",		AFF_PASS_DOOR,		TRUE	},
	{ "haste",		AFF_HASTE,		TRUE	},
	{ "calm",		AFF_CALM,		TRUE	},
	{ "plague",		AFF_PLAGUE,		TRUE	},
	{ "weaken",		AFF_WEAKEN,		TRUE	},
	{ "berserk",		AFF_BERSERK,		TRUE	},
	{ "regeneration",	AFF_REGENERATION,	TRUE	},
	{ "slow",		AFF_SLOW,		TRUE	},
	{ "questtarget",	AFF_QUESTTARGET,	FALSE	},

	{ NULL, 0, FALSE }
};

flaginfo_t id_flags[] =
{
	{ "",			TABLE_BITVAL,		TRUE	},

	{ "invis",		ID_INVIS,		TRUE	},
	{ "hidden",		ID_HIDDEN,		TRUE	},
	{ "fade",		ID_FADE,		TRUE	},
	{ "imp_invis",		ID_IMP_INVIS,		TRUE	},
	{ "blend",		ID_BLEND,		TRUE	},
	{ "camouflage",		ID_CAMOUFLAGE,		TRUE	},
	{ "sneak",		ID_SNEAK,		TRUE	},
	{ "infrared",		ID_INFRARED,		TRUE	},
	{ "undead",		ID_UNDEAD,		TRUE	},
	{ "life",		ID_LIFE,		TRUE	},
	{ "evil",		ID_EVIL,		TRUE	},
	{ "magic",		ID_MAGIC,		TRUE	},
	{ "good",		ID_GOOD,		TRUE	},
	{ "trueseeing",		ID_TRUESEEING,		TRUE	},

	{ NULL, 0, FALSE }
};

flaginfo_t off_flags[] =
{
	{ "",			TABLE_BITVAL,		FALSE	},

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
	{ "distance",		OFF_DISTANCE,		TRUE	},
	{ "deathblow",		OFF_DEATHBLOW,		TRUE	},
	{ "spellbane",		OFF_SPELLBANE,		TRUE	},
	{ "assist_all",		ASSIST_ALL,		TRUE	},
	{ "assist_align",	ASSIST_ALIGN,		TRUE	},
	{ "assist_race",	ASSIST_RACE,		TRUE	},
	{ "assist_players",	ASSIST_PLAYERS,		TRUE	},
	{ "assist_guard",	ASSIST_GUARD,		TRUE	},
	{ "assist_vnum",	ASSIST_VNUM,		TRUE	},

	{ NULL, 0, FALSE }
};

flaginfo_t form_flags[] =
{
	{ "",			TABLE_BITVAL,		FALSE	},

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

	{ NULL, 0, FALSE }
};

flaginfo_t part_flags[] =
{
	{ "",			TABLE_BITVAL,		FALSE	},

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

	{ NULL, 0, FALSE }
};

flaginfo_t comm_flags[] =
{
	{ "",			TABLE_BITVAL,		FALSE	},

	{ "quiet",		COMM_QUIET,		TRUE	},
	{ "deaf",		COMM_DEAF,		TRUE	},
	{ "noautoflee",		COMM_NOFLEE,		TRUE	},
	{ "nonote",		COMM_NONOTE,		TRUE	},
	{ "showrace",		COMM_SHOWRACE,		TRUE	},
	{ "short_eq",		COMM_SHORT_EQ,		TRUE	},
	{ "short_affects",	COMM_SHORT_AFFECTS,	TRUE	},
	{ "quiet_editor",	COMM_QUIET_EDITOR,	TRUE	},
	{ "compact",		COMM_COMPACT,		TRUE	},
	{ "brief",		COMM_BRIEF,		TRUE	},
	{ "prompt",		COMM_PROMPT,		TRUE	},
	{ "combine",		COMM_COMBINE,		TRUE	},
	{ "telnet_ga",		COMM_TELNET_GA,		TRUE	},
	{ "showaff",		COMM_SHOWAFF,		TRUE	},
	{ "color",		COMM_COLOR,		TRUE	},
	{ "noemote",		COMM_NOEMOTE,		FALSE	},
	{ "notell",		COMM_NOTELL,		FALSE	},
	{ "noeng",		COMM_NOENG,		TRUE	},
	{ "snoop_proof",	COMM_SNOOP_PROOF,	FALSE	},
	{ "afk",		COMM_AFK,		TRUE	},
	{ "long",		COMM_LONG,		TRUE	},
	{ "notelnet",		COMM_NOTELNET,		TRUE	},
	{ "noiac",		COMM_NOIAC,		TRUE	},
	{ "noverbose",		COMM_NOVERBOSE,		TRUE	},
	{ "nobust",		COMM_NOBUST,		TRUE	},

	{ NULL, 0, FALSE }
};

flaginfo_t chan_flags[] =
{
	{ "",			TABLE_BITVAL,		FALSE	},

	{ "nochannels",		CHAN_NOCHANNELS,	FALSE	},
	{ "nowiz",		CHAN_NOWIZ,		TRUE	},
	{ "nogossip",		CHAN_NOGOSSIP,		TRUE	},
	{ "noquestion",		CHAN_NOQUESTION,	TRUE	},
	{ "noauction",		CHAN_NOAUCTION,		TRUE	},
	{ "nomusic",		CHAN_NOMUSIC,		TRUE	},
	{ "noquote",		CHAN_NOQUOTE,		TRUE	},
	{ "noclan",		CHAN_NOCLAN,		TRUE	},
	{ "noshout",		CHAN_NOSHOUT,		FALSE	},

	{ NULL, 0, FALSE }
};

flaginfo_t clan_flags[] =
{
	{ "",			TABLE_BITVAL,		FALSE	},

	{ "hidden",		CLAN_HIDDEN,		TRUE	},
	{ "changed",		CLAN_CHANGED,		FALSE	},

	{ NULL, 0, FALSE }
};

flaginfo_t area_flags[] =
{
	{ "",			TABLE_BITVAL,		FALSE	},

	{ "closed",		AREA_CLOSED,		TRUE	},
	{ "noquest",		AREA_NOQUEST,		TRUE	},
	{ "update_always",	AREA_UPDATE_ALWAYS,	TRUE	},
	{ "update_frequently",	AREA_UPDATE_FREQUENTLY,	TRUE	},
	{ "tagged",		AREA_TAGGED,		FALSE	},
	{ "changed",		AREA_CHANGED,		FALSE	},

	{ NULL, 0, FALSE }
};

flaginfo_t sex_table[] =
{
	{ "",			TABLE_INTVAL,		FALSE	},

	{ "none",		SEX_NEUTRAL,		TRUE	},
	{ "male",		SEX_MALE,		TRUE	},
	{ "female",		SEX_FEMALE,		TRUE	},
	{ "either",		SEX_EITHER,		TRUE	},
	{ "random",		SEX_EITHER,		TRUE    },
	{ "neutral",		SEX_NEUTRAL,		TRUE	},
	{ "plural",		SEX_PLURAL,		TRUE	},

	{ NULL, 0, FALSE }
};

flaginfo_t gender_table[] =
{
	{ "",			TABLE_INTVAL,		FALSE	},

	{ "none",		SEX_NEUTRAL,		TRUE	},
	{ "male",		SEX_MALE,		TRUE	},
	{ "female",		SEX_FEMALE,		TRUE	},
	{ "neutral",		SEX_NEUTRAL,		TRUE	},
	{ "plural",		SEX_PLURAL,		TRUE	},

	{ NULL, 0, FALSE }
};

flaginfo_t exit_flags[] =
{
	{ "",			TABLE_BITVAL,		FALSE	},

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

	{ NULL, 0, FALSE }
};

flaginfo_t door_resets[] =
{
	{ "",			TABLE_INTVAL,		FALSE	},

	{ "open",			0,		TRUE	},
	{ "closed",			1,		TRUE	},
	{ "closed and locked",		2,		TRUE	},

	{ NULL, 0, FALSE }
};

flaginfo_t room_flags[] =
{
	{ "",			TABLE_BITVAL,		FALSE	},

	{ "dark",		ROOM_DARK,		TRUE	},
	{ "nomob",		ROOM_NOMOB,		TRUE	},
	{ "indoors",		ROOM_INDOORS,		TRUE	},
	{ "peace",		ROOM_PEACE,		TRUE	},
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
	{ "guild",		ROOM_GUILD,		TRUE	},
	{ "registry",		ROOM_REGISTRY,		TRUE	},

	{ NULL, 0, FALSE }
};

flaginfo_t sector_types[] =
{
	{ "",			TABLE_INTVAL,		FALSE	},

	{ "inside",		SECT_INSIDE,		TRUE	},
	{ "city",		SECT_CITY,		TRUE	},
	{ "field",		SECT_FIELD,		TRUE	},
	{ "forest",		SECT_FOREST,		TRUE	},
	{ "hills",		SECT_HILLS,		TRUE	},
	{ "mountain",		SECT_MOUNTAIN,		TRUE	},
	{ "swim",		SECT_WATER_SWIM,	TRUE	},
	{ "noswim",		SECT_WATER_NOSWIM,	TRUE	},
	{ "underwater",		SECT_UNDERWATER,	TRUE	},
	{ "air",		SECT_AIR,		TRUE	},
	{ "desert",		SECT_DESERT,		TRUE	},

	{ NULL, 0, FALSE }
};

flaginfo_t item_types[] =
{
	{ "",			TABLE_INTVAL,		FALSE	},

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
	{ "book",		ITEM_BOOK,		TRUE	},
	{ "tattoo",		ITEM_TATTOO,		FALSE	},

	{ NULL, 0, FALSE }
};

flaginfo_t stat_flags[] =
{
	{ "",			TABLE_BITVAL,		FALSE	},

	{ "glow",		ITEM_GLOW,		TRUE	},
	{ "hum",		ITEM_HUM,		TRUE	},
	{ "dark",		ITEM_DARK,		TRUE	},
	{ "evil",		ITEM_EVIL,		TRUE	},
	{ "invis",		ITEM_INVIS,		TRUE	},
	{ "magic",		ITEM_MAGIC,		TRUE	},
	{ "nodrop",		ITEM_NODROP,		TRUE	},
	{ "bless",		ITEM_BLESS,		TRUE	},
	{ "antigood",		ITEM_ANTI_GOOD,		TRUE	},
	{ "antievil",		ITEM_ANTI_EVIL,		TRUE	},
	{ "antineutral",	ITEM_ANTI_NEUTRAL,	TRUE	},
	{ "noremove",		ITEM_NOREMOVE,		TRUE	},
	{ "inventory",		ITEM_INVENTORY,		FALSE	},
	{ "rotdeath",		ITEM_ROT_DEATH,		TRUE	},
	{ "visdeath",		ITEM_VIS_DEATH,		TRUE	},
	{ "meltdrop",		ITEM_MELT_DROP,		TRUE	},
	{ "hadtimer",		ITEM_HAD_TIMER,		FALSE	},
	{ "burnproof",		ITEM_BURN_PROOF,	TRUE	},
	{ "not_edible",		ITEM_NOT_EDIBLE,	TRUE	},
	{ "enchanted",		ITEM_ENCHANTED,		FALSE	},

	{ NULL, 0, FALSE }
};

flaginfo_t obj_flags[] =
{
	{ "",			TABLE_BITVAL,		FALSE	},

	{ "nopurge",		OBJ_NOPURGE,		TRUE	},
	{ "nosac",		OBJ_NOSAC,		TRUE	},
	{ "nolocate",		OBJ_NOLOCATE,		TRUE	},
	{ "sellextract",	OBJ_SELL_EXTRACT,	TRUE	},
	{ "nouncurse",		OBJ_NOUNCURSE,		TRUE	},
	{ "nosell",		OBJ_NOSELL,		TRUE	},
	{ "quest",		OBJ_QUEST,		TRUE	},
	{ "clan_item",		OBJ_CLAN,		TRUE	},
	{ "quit_drop",		OBJ_QUIT_DROP,		TRUE	},
	{ "pit",		OBJ_PIT,		TRUE	},
	{ "chquest",		OBJ_CHQUEST,		TRUE	},
	{ "nofind",		OBJ_NOFIND,		TRUE	},

	{ NULL, 0, FALSE }
};

flaginfo_t wear_flags[] =
{
	{ "",			TABLE_BITVAL,		FALSE	},

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
	{ "float_around",	ITEM_WEAR_FLOAT,	TRUE	},
	{ "tattoo",		ITEM_WEAR_TATTOO,	TRUE	},
	{ "clanmark",		ITEM_WEAR_CLANMARK,	TRUE	},
	{ "face",		ITEM_WEAR_FACE,		TRUE	},

	{ NULL, 0, FALSE }
};

/*
* Used when adding an affect to tell where it goes.
* See addaffect and delaffect in act_olc.c
*/
flaginfo_t apply_flags[] =
{
	{ "",			TABLE_INTVAL,		FALSE	},

	{ "none",		APPLY_NONE,		TRUE	},
	{ "str",		APPLY_STR,		TRUE	},
	{ "dex",		APPLY_DEX,		TRUE	},
	{ "int",		APPLY_INT,		TRUE	},
	{ "wis",		APPLY_WIS,		TRUE	},
	{ "con",		APPLY_CON,		TRUE	},
	{ "cha",		APPLY_CHA,		TRUE	},
	{ "sex",		APPLY_SEX,		TRUE	},
	{ "level",		APPLY_LEVEL,		TRUE	},
	{ "age",		APPLY_AGE,		TRUE	},
	{ "height",		APPLY_HEIGHT,		TRUE	},
	{ "weight",		APPLY_WEIGHT,		TRUE	},
	{ "mana",		APPLY_MANA,		TRUE	},
	{ "hp",			APPLY_HIT,		TRUE	},
	{ "move",		APPLY_MOVE,		TRUE	},
	{ "gold",		APPLY_GOLD,		TRUE	},
	{ "exp",		APPLY_EXP,		TRUE	},
	{ "ac",			APPLY_AC,		TRUE	},
	{ "hitroll",		APPLY_HITROLL,		TRUE	},
	{ "damroll",		APPLY_DAMROLL,		TRUE	},
	{ "saves",		APPLY_SAVES,		TRUE	},
	{ "saving_para",	APPLY_SAVING_PARA,	TRUE	},
	{ "saving_rod",		APPLY_SAVING_ROD,	TRUE	},
	{ "saving_petri",	APPLY_SAVING_PETRI,	TRUE	},
	{ "saving_breath",	APPLY_SAVING_BREATH,	TRUE	},
	{ "saving_spell",	APPLY_SAVING_SPELL,	TRUE	},
	{ "size",		APPLY_SIZE,		TRUE	},
	{ "luck",		APPLY_LUCK,		TRUE	},

	{ NULL, 0, FALSE }
};

flaginfo_t rapply_flags[] =
{
	{ "",			TABLE_INTVAL,		FALSE	},

	{ "healrate",		APPLY_ROOM_HEAL,	TRUE	},
	{ "manarate",		APPLY_ROOM_MANA,	TRUE	},

	{ NULL, 0, FALSE }
};

/*
 * What is seen.
 */
flaginfo_t wear_loc_strings[] =
{
	{ "",				TABLE_INTVAL,		FALSE	},

	{ "in the inventory",		WEAR_NONE,		TRUE	},
	{ "as a light",			WEAR_LIGHT,		TRUE	},
	{ "on the left finger",		WEAR_FINGER_L,		TRUE	},
	{ "on the right finger",	WEAR_FINGER_R,		TRUE	},
	{ "around the neck",		WEAR_NECK,		TRUE	},
	{ "on the face",		WEAR_FACE,		TRUE	},
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
	{ "clan mark",			WEAR_CLANMARK,		TRUE	},
	{ "stuck in",			WEAR_STUCK_IN,		TRUE	},

	{ NULL, 0, FALSE }
};

flaginfo_t wear_loc_flags[] =
{
	{ "",			TABLE_INTVAL,		FALSE	},

	{ "none",		WEAR_NONE,		TRUE	},
	{ "inventory",		WEAR_NONE,		TRUE	},
	{ "light",		WEAR_LIGHT,		TRUE	},
	{ "lfinger",		WEAR_FINGER_L,		TRUE	},
	{ "rfinger",		WEAR_FINGER_R,		TRUE	},
	{ "neck",		WEAR_NECK,		TRUE	},
	{ "face",		WEAR_FACE,		TRUE	},
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
	{ "clanmark",		WEAR_CLANMARK,		TRUE	},

	{ NULL, 0, FALSE }
};

flaginfo_t cont_flags[] =
{
	{ "",			TABLE_BITVAL,		FALSE	},

	{ "closable",		CONT_CLOSEABLE,		TRUE	},
	{ "pickproof",		CONT_PICKPROOF,		TRUE	},
	{ "closed",		CONT_CLOSED,		TRUE	},
	{ "locked",		CONT_LOCKED,		TRUE	},
	{ "put_on",		CONT_PUT_ON,		TRUE	},
	{ "quiver",		CONT_QUIVER,		TRUE	},

	{ NULL, 0, FALSE }
};

flaginfo_t ac_type[] =
{
	{ "",			TABLE_INTVAL,		FALSE	},

	{ "pierce",		AC_PIERCE,		TRUE	},
	{ "bash",		AC_BASH,		TRUE	},
	{ "slash",		AC_SLASH,		TRUE	},
	{ "exotic",		AC_EXOTIC,		TRUE	},

	{ NULL, 0, FALSE }
};

flaginfo_t weapon_class[] =
{
	{ "",			TABLE_INTVAL,		FALSE	},

	{ "exotic",		WEAPON_EXOTIC,		TRUE    },
	{ "sword",		WEAPON_SWORD,		TRUE    },
	{ "dagger",		WEAPON_DAGGER,		TRUE    },
	{ "spear",		WEAPON_SPEAR,		TRUE    },
	{ "staff",		WEAPON_STAFF,		TRUE	}, /* compat */
	{ "mace",		WEAPON_MACE,		TRUE    },
	{ "axe",		WEAPON_AXE,		TRUE    },
	{ "flail",		WEAPON_FLAIL,		TRUE    },
	{ "whip",		WEAPON_WHIP,		TRUE    },
	{ "polearm",		WEAPON_POLEARM,		TRUE    },
	{ "bow",		WEAPON_BOW,		TRUE	},
	{ "arrow",		WEAPON_ARROW,		TRUE	},
	{ "lance",		WEAPON_LANCE,		TRUE	},

	{ NULL, 0, FALSE }
};

flaginfo_t weapon_type2[] =
{
	{ "",			TABLE_BITVAL,		FALSE	},

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
	{ "throwing",		WEAPON_THROW,		TRUE	},
	{ "nooffhand",		WEAPON_NO_OFFHAND,	TRUE	},

	{ NULL, 0, FALSE }
};

flaginfo_t sk_aff_flags[] =
{
	{ "",			TABLE_BITVAL,		FALSE	},

	{ "teach",		SK_AFF_TEACH,		TRUE	},
	{ "all",		SK_AFF_ALL,		TRUE	},
	{ "notclan",		SK_AFF_NOTCLAN,		TRUE	},

	{ NULL, 0, FALSE }
};

flaginfo_t position_table[] =
{
	{ "",			TABLE_INTVAL,		FALSE	},

	{ "dead",		POS_DEAD,		TRUE	},
	{ "mort",		POS_MORTAL,		TRUE	},
	{ "incap",		POS_INCAP,		TRUE	},
	{ "stun",		POS_STUNNED,		TRUE	},
	{ "sleep",		POS_SLEEPING,		TRUE	},
	{ "rest",		POS_RESTING,		TRUE	},
	{ "sit",		POS_SITTING,		TRUE	},
	{ "fight",		POS_FIGHTING,		TRUE	},
	{ "stand",		POS_STANDING,		TRUE	},

	{ NULL, 0, FALSE }
};

flaginfo_t position_names[] =
{
	{ "",			TABLE_INTVAL,	FALSE	},

	{ "DEAD",		POS_DEAD,	FALSE	},
	{ "mortally wounded",	POS_MORTAL,	FALSE	},
	{ "incapacitated",	POS_INCAP,	FALSE	},
	{ "stunned",		POS_STUNNED,	FALSE	},
	{ "sleeping",		POS_SLEEPING,	FALSE	},
	{ "resting",		POS_RESTING,	FALSE	},
	{ "sitting",		POS_SITTING,	FALSE	},
	{ "fighting",		POS_FIGHTING,	FALSE	},
	{ "standing",		POS_STANDING,	FALSE	},

	{ NULL, 0, FALSE }
};

flaginfo_t portal_flags[]=
{
	{ "",			TABLE_BITVAL,		FALSE	},

	{ "normal_exit",	GATE_NORMAL_EXIT,	TRUE	},
	{ "nocurse",		GATE_NOCURSE,		TRUE	},
	{ "gowith",		GATE_GOWITH,		TRUE	},
	{ "buggy",		GATE_BUGGY,		TRUE	},
	{ "random",		GATE_RANDOM,		TRUE	},

	{ NULL, 0, FALSE }
};

flaginfo_t furniture_flags[]=
{
	{ "",			TABLE_BITVAL,		FALSE	},

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

	{ NULL, 0, FALSE }
};

flaginfo_t raffect_flags[] =
{
	{ "",			TABLE_BITVAL,		FALSE	},

	{ "randomizer",		RAFF_RANDOMIZER,	TRUE	},
	{ "espirit",		RAFF_ESPIRIT,		TRUE	},
	{ "curse",		RAFF_CURSE,		TRUE	},

	{ NULL, 0, FALSE }
};

flaginfo_t skill_groups[] =
{
	{ "",			TABLE_BITVAL,		FALSE	},

	{ "none",		GROUP_NONE,		TRUE	},
	{ "weaponsmaster",	GROUP_WEAPONSMASTER,	TRUE	},
	{ "attack",		GROUP_ATTACK,		TRUE	},
	{ "benedictions",	GROUP_BENEDICTIONS,	TRUE	},
	{ "combat",		GROUP_COMBAT,		TRUE	},
	{ "creation",		GROUP_CREATION,		TRUE	},
	{ "curative",		GROUP_CURATIVE,		TRUE	},
	{ "divination",		GROUP_DIVINATION,	TRUE	},
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
	{ "meditation",		GROUP_MEDITATION,	TRUE	},
	{ "clan",		GROUP_CLAN,		TRUE	},
	{ "defensive",		GROUP_DEFENSIVE,	TRUE	},
	{ "wizard",		GROUP_WIZARD,		TRUE	},
	{ "necromancy",		GROUP_NECROMANCY,	TRUE	},
	{ "evocation",		GROUP_EVOCATION,	TRUE	},
	{ "conjuration",	GROUP_CONJURATION,	TRUE	},
	{ "alteration",		GROUP_ALTERATION,	TRUE	},
	{ "summoning",		GROUP_SUMMONING,	TRUE	},
	{ "abjuration",		GROUP_ABJURATION,	TRUE	},
	{ "charm",		GROUP_CHARM,		TRUE	},

	{ NULL, 0, FALSE }
};

flaginfo_t skill_targets[] =
{
	{ "",			TABLE_INTVAL,		FALSE	},

	{ "ignore",		TAR_IGNORE,		TRUE	},
	{ "charoff",		TAR_CHAR_OFFENSIVE,	TRUE	},
	{ "chardef",		TAR_CHAR_DEFENSIVE,	TRUE	},
	{ "charself",		TAR_CHAR_SELF,		TRUE	},
	{ "objinv",		TAR_OBJ_INV,		TRUE	},
	{ "objchardef",		TAR_OBJ_CHAR_DEF,	TRUE	},
	{ "objcharoff",		TAR_OBJ_CHAR_OFF,	TRUE	},

	{ NULL, 0, FALSE }
};

flaginfo_t stat_aliases[] =
{
	{ "",			TABLE_INTVAL,		FALSE	},

	{ "str",		STAT_STR,		TRUE	},
	{ "int",		STAT_INT,		TRUE	},
	{ "dex",		STAT_DEX,		TRUE	},
	{ "wis",		STAT_WIS,		TRUE	},
	{ "con",		STAT_CON,		TRUE	},
	{ "cha",		STAT_CHA,		TRUE	},

	{ NULL, 0, FALSE }
};

flaginfo_t stat_names[] =
{
	{ "",			TABLE_INTVAL,		FALSE	},

	{ "strength",		STAT_STR,		TRUE	},
	{ "intelligence",	STAT_INT,		TRUE	},
	{ "dexterity",		STAT_DEX,		TRUE	},
	{ "wisdom",		STAT_WIS,		TRUE	},
	{ "constitution",	STAT_CON,		TRUE	},
	{ "charisma",		STAT_CHA,		TRUE	},

	{ NULL, 0, FALSE }
};

flaginfo_t material_flags[] =
{
	{ "",			TABLE_BITVAL,		FALSE	},

	{ "indestruct",		MATERIAL_INDESTRUCT,	TRUE	},
	{ "metal",		MATERIAL_METAL,		TRUE	},
	{ "edible",		MATERIAL_EDIBLE,	TRUE	},
	{ "susc_heat",		MATERIAL_SUSC_HEAT,	TRUE	},
	{ "susc_cold",		MATERIAL_SUSC_COLD,	TRUE	},
	{ "fragile",		MATERIAL_FRAGILE,	TRUE	},

	{ NULL, 0, FALSE }
};

flaginfo_t skill_flags[] =
{
	{ "",			TABLE_BITVAL,		FALSE	},

	{ "clan",		SKILL_CLAN,		TRUE	},
	{ "range",		SKILL_RANGE,		TRUE	},
	{ "area_attack",	SKILL_AREA_ATTACK,	TRUE	},
	{ "questionable",	SKILL_QUESTIONABLE,	TRUE	},
	{ "form",		SKILL_FORM,		TRUE	},
	{ "missile",		SKILL_MISSILE,		TRUE	},
	{ "shadow",		SKILL_SHADOW,		TRUE	},

	{ NULL, 0, FALSE }
};

flaginfo_t skill_types[] =
{
	{ "",			TABLE_INTVAL,		FALSE	},

	{ "skill",		ST_SKILL,		TRUE	},
	{ "spell",		ST_SPELL,		TRUE	},
	{ "prayer",		ST_PRAYER,		TRUE	},

	{ NULL, 0, FALSE }
};

flaginfo_t class_flags[] =
{
	{ "",			TABLE_BITVAL,		FALSE	},

	{ "magic",		CLASS_MAGIC,		TRUE	},
	{ "noch",		CLASS_NOCH,		TRUE	},
	{ "closed",		CLASS_CLOSED,		TRUE	},
	{ "changed",		CLASS_CHANGED,		FALSE	},

	{ NULL, 0, FALSE }
};

flaginfo_t race_flags[] =
{
	{ "",			TABLE_BITVAL,		FALSE	},

	{ "noch",		RACE_NOCH,		TRUE	},
	{ "changed",		RACE_CHANGED,		FALSE	},

	{ NULL, 0, FALSE }
};

flaginfo_t shapeform_flags[] =
{
	{ "",			TABLE_BITVAL,		FALSE	},
	{ "nospeak",		FORM_NOSPEAK,		TRUE	},
	{ "nocast",		FORM_NOCAST,		TRUE	},
	{ "castself",		FORM_CASTSELF,		TRUE	},
	{ "noget",		FORM_NOGET,		TRUE	},

	{ NULL, 0, FALSE }
};

flaginfo_t ralign_names[] =
{
	{ "",			TABLE_BITVAL,		FALSE	},

	{ "none",		RA_NONE,		TRUE	},
	{ "evil",		RA_EVIL,		TRUE	},
	{ "neutral",		RA_NEUTRAL,		TRUE	},
	{ "good",		RA_GOOD,		TRUE	},

	{ NULL, 0, FALSE }
};

flaginfo_t align_names[] =
{
	{ "",			TABLE_INTVAL,		FALSE	},

	{ "good",		AN_GOOD,		TRUE	},
	{ "neutral",		AN_NEUTRAL,		TRUE	},
	{ "evil",		AN_EVIL,		TRUE	},

	{ NULL, 0, FALSE }
};

flaginfo_t lang_flags[] =
{
	{ "",			TABLE_BITVAL,		FALSE	},

	{ "hidden",		LANG_HIDDEN,		TRUE	},
	{ "changed",		LANG_CHANGED,		FALSE	},

	{ NULL, 0, FALSE }
};

flaginfo_t ethos_table[] =
{
	{ "",			TABLE_BITVAL,		FALSE	},

	{ "none",		ETHOS_NONE,		TRUE	},
	{ "lawful",		ETHOS_LAWFUL,		TRUE	},
	{ "neutral",		ETHOS_NEUTRAL,		TRUE	},
	{ "chaotic",		ETHOS_CHAOTIC,		TRUE	},

	{ NULL, 0, FALSE }
};

flaginfo_t rulecl_names[] =
{
	{ "",			TABLE_INTVAL,		FALSE	},

	{ "cases",		RULES_CASE,		TRUE	},
	{ "genders",		RULES_GENDER,		TRUE	},
	{ "qtys",		RULES_QTY,		TRUE	},

	{ NULL, 0, FALSE }
};

flaginfo_t rulecl_flags[] =
{
	{ "",			TABLE_BITVAL,		FALSE	},

	{ "expl_changed",	RULES_EXPL_CHANGED,	FALSE	},
	{ "impl_changed",	RULES_IMPL_CHANGED,	FALSE	},

	{ NULL, 0, FALSE }
};

flaginfo_t note_types[] =
{
	{ "",			TABLE_INTVAL,		FALSE	},

	{ "note",		NOTE_NOTE,		FALSE	},
	{ "idea",		NOTE_IDEA,		FALSE	},
	{ "penalty",		NOTE_PENALTY,		FALSE	},
	{ "news",		NOTE_NEWS,		FALSE	},
	{ "change",		NOTE_CHANGES,		FALSE	},

	{ NULL, 0, FALSE }
};

flaginfo_t options_table[] =
{
	{ "",			TABLE_BITVAL,		FALSE	},

	{ "ascii_only_names",	OPT_ASCII_ONLY_NAMES,	FALSE	},

	{ NULL, 0, FALSE }
};

flaginfo_t ban_actions[] =
{
	{ "",			TABLE_INTVAL,		FALSE	},

	{ "allow",		BA_ALLOW,		FALSE	},
	{ "deny",		BA_DENY,		FALSE	},

	{ NULL, 0, FALSE }
};

flaginfo_t ban_classes[] =
{
	{ "",			TABLE_INTVAL,		FALSE	},

	{ "all",		BCL_ALL,		FALSE	},
	{ "players",		BCL_PLAYERS,		FALSE	},
	{ "newbies",		BCL_NEWBIES,		FALSE	},

	{ NULL, 0, FALSE }
};

flaginfo_t cmd_flags[] =
{
	{ "",			TABLE_BITVAL,		FALSE	},

	{ "keep_hide",		CMD_KEEP_HIDE,		TRUE	},
	{ "noorder",		CMD_NOORDER,		TRUE	},
	{ "charmed_ok",		CMD_CHARMED_OK,		TRUE	},
	{ "hidden",		CMD_HIDDEN,		TRUE	},
	{ "disabled",		CMD_DISABLED,		TRUE	},
	{ "frozen_ok",		CMD_FROZEN_OK,		TRUE	},
	{ "strict_match",	CMD_STRICT_MATCH,	TRUE	},

	{ NULL, 0, FALSE }
};

flaginfo_t cmd_logtypes[] =
{
	{ "",			TABLE_INTVAL,		FALSE	},

	{ "normal",		LOG_NORMAL,		TRUE	},
	{ "always",		LOG_ALWAYS,		TRUE	},
	{ "never",		LOG_NEVER,		TRUE	},

	{ NULL, 0, FALSE }
};

/*
 * "imm" should be before "immortal" for flag_lookup to work properly
 * "immortal" should be before "ava" for convenience
 */
flaginfo_t level_table[] =
{
	{ "",			TABLE_INTVAL,		FALSE	},

	{ "imp",		LEVEL_IMP,		TRUE	},
	{ "cre",		LEVEL_CRE,		TRUE	},
	{ "dei",		LEVEL_DEI,		TRUE	},
	{ "god",		LEVEL_GOD,		TRUE	},
	{ "immortal",		LEVEL_IMMORTAL,		TRUE	},
	{ "ava",		LEVEL_AVA,		TRUE	},

	{ "hero",		LEVEL_HERO,		TRUE	},
	{ "newbie",		LEVEL_NEWBIE,		TRUE	},
	{ "pk",			LEVEL_PK,		TRUE	},

	{ NULL, 0, FALSE }
};

/* wiznet table and prototype for future flag setting */
const struct wiznet_type wiznet_table[] =
{
	{ "on",			WIZ_ON,		LEVEL_IMMORTAL	},
	{ "prefix",		WIZ_PREFIX,	LEVEL_IMMORTAL	},
	{ "ticks",		WIZ_TICKS,	LEVEL_IMMORTAL	},
	{ "logins",		WIZ_LOGINS,	LEVEL_IMMORTAL	},
	{ "sites",		WIZ_SITES,	LEVEL_GOD	},
	{ "links",		WIZ_LINKS,	LEVEL_GOD	},
	{ "newbies",		WIZ_NEWBIE,	LEVEL_IMMORTAL	},
	{ "spam",		WIZ_SPAM,	LEVEL_GOD	},
	{ "deaths",		WIZ_DEATHS,	LEVEL_IMMORTAL	},
	{ "resets",		WIZ_RESETS,	LEVEL_GOD	},
	{ "mobdeaths",		WIZ_MOBDEATHS,	LEVEL_GOD	},
	{ "flags",		WIZ_FLAGS,	LEVEL_GOD	},
	{ "penalties",		WIZ_PENALTIES,	LEVEL_GOD	},
	{ "saccing",		WIZ_SACCING,	LEVEL_GOD	},
	{ "levels",		WIZ_LEVELS,	LEVEL_IMMORTAL	},
	{ "load",		WIZ_LOAD,	LEVEL_CRE	},
	{ "restore",		WIZ_RESTORE,	LEVEL_CRE	},
	{ "snoops",		WIZ_SNOOPS,	LEVEL_CRE	},
	{ "switches",		WIZ_SWITCHES,	LEVEL_CRE	},
	{ "secure",		WIZ_SECURE,	LEVEL_CRE	},
	{ "olc",		WIZ_OLC,	LEVEL_GOD	},
	{ "log",		WIZ_WIZLOG,	LEVEL_CRE	},
	{ NULL, 0, FALSE }
};

flaginfo_t desc_con_table[] =
{
	{ "",			TABLE_INTVAL,			FALSE	},

	{ "playing",		CON_PLAYING,			FALSE	},
	{ "get name",		CON_GET_NAME,			FALSE	},
	{ "get old pwd",	CON_GET_OLD_PASSWORD,		FALSE	},
	{ "confirm name",	CON_CONFIRM_NEW_NAME,		FALSE	},
	{ "get pwd",		CON_GET_NEW_PASSWORD,		FALSE	},
	{ "confirm pwd",	CON_CONFIRM_NEW_PASSWORD,	FALSE	},
	{ "get race",		CON_GET_NEW_RACE,		FALSE	},
	{ "get sex",		CON_GET_NEW_SEX,		FALSE	},
	{ "get class",		CON_GET_NEW_CLASS,		FALSE	},
	{ "get align",		CON_GET_ALIGNMENT,		FALSE	},
	{ "read imotd",		CON_READ_IMOTD,			FALSE	},
	{ "read motd",		CON_READ_MOTD,			FALSE	},
	{ "pick home",		CON_PICK_HOMETOWN,		FALSE	},
	{ "get ethos",		CON_GET_ETHOS,			FALSE	},
	{ "create done",	CON_CREATE_DONE,		FALSE	},
	{ "get charset",	CON_GET_CODEPAGE,		FALSE	},
	{ "resolving",		CON_RESOLV,			FALSE	},
	{ NULL, 0, FALSE }
};

flaginfo_t events_classes[] =
{
	{ "",			TABLE_INTVAL,			FALSE	},

	{ "enter_room",		EVENT_ROOM_ENTER,		TRUE	},
	{ "leave_room",		EVENT_ROOM_LEAVE,		TRUE	},
	{ "update_room",	EVENT_ROOM_UPDATE,		TRUE	},
	{ "timeout_room",	EVENT_ROOM_TIMEOUT,		TRUE	},
	{ "update_char",	EVENT_CHAR_UPDATE,		TRUE	},
	{ "update_fast_char",	EVENT_CHAR_UPDFAST,		TRUE	},
	{ "timeout_char",	EVENT_CHAR_TIMEOUT,		TRUE	},
	{ NULL, 0, FALSE }
};

flaginfo_t dam_classes[] =
{
	{ "",			TABLE_INTVAL,			FALSE	},

	{ "none",		DAM_NONE,			TRUE	},

	{ "bash",		DAM_BASH,			TRUE	},
	{ "pierce",		DAM_PIERCE,			TRUE	},
	{ "slash",		DAM_SLASH,			TRUE	},

	{ "fire",		DAM_FIRE,			TRUE	},
	{ "cold",		DAM_COLD,			TRUE	},
	{ "lightning",		DAM_LIGHTNING,			TRUE	},
	{ "acid",		DAM_ACID,			TRUE	},
	{ "poison",		DAM_POISON,			TRUE	},
	{ "negative",		DAM_NEGATIVE,			TRUE	},
	{ "holy",		DAM_HOLY,			TRUE	},
	{ "energy",		DAM_ENERGY,			TRUE	},
	{ "mental",		DAM_MENTAL,			TRUE	},
	{ "disease",		DAM_DISEASE,			TRUE	},
	{ "light",		DAM_LIGHT,			TRUE	},
	{ "charm",		DAM_CHARM,			TRUE	},
	{ "sound",		DAM_SOUND,			TRUE	},
	{ "harm",		DAM_HARM,			TRUE	},

	{ "iron",		DAM_IRON,			TRUE	},
	{ "wood",		DAM_WOOD,			TRUE	},
	{ "silver",		DAM_SILVER,			TRUE	},
	{ "other",		DAM_OTHER,			TRUE	},

	{ "weapon",		DAM_WEAPON,			TRUE	},
	{ "magic",		DAM_MAGIC,			TRUE	},

	{ NULL, 0, FALSE }
};

flaginfo_t resist_info_flags[] =
{
	{ "",			TABLE_INTVAL,		FALSE	},

	{ "bashing",		DAM_BASH,		TRUE	},
	{ "piercing",		DAM_PIERCE,		TRUE	},
	{ "slashing",		DAM_SLASH,		TRUE	},

	{ "fire attacks",	DAM_FIRE,		TRUE	},
	{ "cold attacks",	DAM_COLD,		TRUE	},
	{ "lightning",		DAM_LIGHTNING,		TRUE	},
	{ "acid",		DAM_ACID,		TRUE	},
	{ "poison",		DAM_POISON,		TRUE	},
	{ "negative energy",	DAM_NEGATIVE,		TRUE	},
	{ "holy energy",	DAM_HOLY,		TRUE	},
	{ "energy attacks",	DAM_ENERGY,		TRUE	},
	{ "mental attacks",	DAM_MENTAL,		TRUE	},
	{ "disease",		DAM_DISEASE,		TRUE	},
	{ "light based attacks",DAM_LIGHT,		TRUE	},
	{ "charming spells",	DAM_CHARM,		TRUE	},
	{ "sound based attack",	DAM_SOUND,		TRUE	},
	{ "harm spells",	DAM_HARM,		TRUE	},

	{ "iron weapons",	DAM_IRON,		TRUE	},
	{ "wooden weapons",	DAM_WOOD,		TRUE	},
	{ "silver weapons",	DAM_SILVER,		TRUE	},
	{ "other weird attacks",DAM_OTHER,		TRUE	},

	{ "magic atacks",	DAM_MAGIC,		TRUE	},
	{ "physical attacks",	DAM_WEAPON,		TRUE	},

	{ NULL, 0, FALSE }
};

flaginfo_t spec_classes[] =
{
	{ "",			TABLE_INTVAL,			FALSE	},

	{ "class",		SPEC_CLASS,			TRUE	},
	{ "race",		SPEC_RACE,			TRUE	},
	{ "clan",		SPEC_CLAN,			TRUE	},
	{ "weapon",		SPEC_WEAPON,			TRUE	},
	{ "major_school",	SPEC_MAJORSCHOOL,		TRUE	},
	{ "minor_school",	SPEC_MINORSCHOOL,		TRUE	},
	{ "form",		SPEC_FORM,			TRUE	},

	{ NULL, 0, FALSE }
};

flaginfo_t spec_flags[] =
{
	{ "",			TABLE_BITVAL,			FALSE	},

	{ "changed",		SPF_CHANGED,			FALSE	},

	{ NULL, 0, FALSE }
};

flaginfo_t cond_table[] =
{
	{ "",			TABLE_INTVAL,			FALSE	},

	{ "drunkness",		COND_DRUNK,			TRUE	},
	{ "fullness",		COND_FULL,			TRUE	},
	{ "thirst",		COND_THIRST,			TRUE	},
	{ "hunger",		COND_HUNGER,			TRUE	},
	{ "bloodlust",		COND_BLOODLUST,			TRUE	},
	{ "desire",		COND_DESIRE,			TRUE	},

	{ NULL, 0, FALSE }
};

flaginfo_t affect_where_types[] =
{
	{ "",			TABLE_INTVAL,			FALSE	},

	{ "affects",		TO_AFFECTS,			TRUE	},
	{ "object",		TO_OBJECT,			FALSE	},
	{ "weapon",		TO_WEAPON,			FALSE	},
	{ "skills",		TO_SKILLS,			TRUE	},
	{ "race",		TO_RACE,			FALSE	},
	{ "detects",		TO_DETECTS,			TRUE	},
	{ "invis",		TO_INVIS,			TRUE	},
	{ "form",		TO_FORM,			FALSE	},
	{ "formaffects",	TO_FORMAFFECTS,			FALSE	},
	{ "resists",		TO_RESISTS,			TRUE	},

	{ NULL, 0, FALSE }
};

flaginfo_t www_flags_table[] =
{
	{ "",			TABLE_BITVAL,			FALSE	},

	{ "race",		WSHOW_RACE,			FALSE	},
	{ "class",		WSHOW_CLASS,			FALSE	},
	{ "clan",		WSHOW_CLAN,			FALSE	},
	{ "level",		WSHOW_LEVEL,			FALSE	},
	{ "align",		WSHOW_ALIGN,			FALSE	},
	{ "ethos",		WSHOW_ETHOS,			FALSE	},
	{ "sex",		WSHOW_SEX,			FALSE	},
	{ "language",		WSHOW_SLANG,			FALSE	},
	{ "deaths",		WSHOW_DEATHS,			FALSE	},
	{ "kills",		WSHOW_KILLS,			FALSE	},
	{ "pckills",		WSHOW_PCKILLS,			FALSE	},
	{ "title",		WSHOW_TITLE,			FALSE	},
	{ "hometown",		WSHOW_HOMETOWN,			FALSE	},
	{ "age",		WSHOW_AGE,			FALSE	},

	{ NULL, 0, FALSE }
};

flaginfo_t clan_status_table[] =
{
	{ "",			TABLE_INTVAL,			FALSE	},

	{ "commoner",		CLAN_COMMONER,			TRUE	},
	{ "secondary",		CLAN_SECOND,			TRUE	},
	{ "leader",		CLAN_LEADER,			TRUE	},

	{ NULL, 0, FALSE }
};

flaginfo_t book_class[] =
{
	{ "",			TABLE_INTVAL,			FALSE	},

	{ "spellbook",		BOOK_SPELLS,			TRUE	},
	{ "manual",		BOOK_MANUAL,			TRUE	},
	{ "prayerbook",		BOOK_PRAYERS,			TRUE	},

	{ NULL, 0, FALSE }
};

flaginfo_t fail_effects[] =
{
	{ "",			TABLE_INTVAL,			FALSE	},

	{ "none",		FAIL_NONE,			TRUE	},
	{ "death",		FAIL_DEATH,			TRUE	},
	{ "hallucination",	FAIL_HALLUCINATION,		TRUE	},

	{ NULL, 0, FALSE }
};

flaginfo_t iterator_names[] =
{
	{ "",			TABLE_INTVAL,			FALSE	},

	{ "char_world",		(flag_t) &iter_char_world,	TRUE	},
	{ "npc_world",		(flag_t) &iter_npc_world,	TRUE	},
	{ "char_room",		(flag_t) &iter_char_room,	TRUE	},
	{ "obj_world",		(flag_t) &iter_obj_world,	TRUE	},
	{ "obj_room",		(flag_t) &iter_obj_room,	TRUE	},
	{ "obj_char",		(flag_t) &iter_obj_char,	TRUE	},
	{ "obj_obj",		(flag_t) &iter_obj_obj,		TRUE	},

	{ NULL, 0, FALSE }
};

flaginfo_t hint_levels[] =
{
	{ "",			TABLE_INTVAL,			FALSE	},

	{ "none",		HINT_NONE,			FALSE	},
	{ "sog",		HINT_SOG,			TRUE	},
	{ "all",		HINT_ALL,			TRUE	},

	{ NULL, 0, FALSE }
};

flaginfo_t olc_flags[] =
{
	{ "",			TABLE_BITVAL,			FALSE	},

	{ "raw_strings",	OLC_MODE_RAW,			TRUE	},
	{ "brief_ashow",	OLC_MODE_TRANS,			TRUE	},

	{ NULL, 0, FALSE }
};

flaginfo_t mt_types[] =
{
	{ "",			TABLE_INTVAL,			FALSE	},

	{ "void",		MT_VOID,			FALSE	},
	{ "void *",		MT_PVOID,			FALSE	},
	{ "int",		MT_INT,				FALSE	},
	{ "const char *",	MT_STR,				FALSE	},
	{ "CHAR_DATA *",	MT_CHAR,			FALSE	},
	{ "OBJ_DATA *",		MT_OBJ,				FALSE	},
	{ "ROOM_INDEX_DATA *",	MT_ROOM,			FALSE	},
	{ "AREA_DATA *",	MT_AREA,			FALSE	},
	{ "AFFECT_DATA *",	MT_AFFECT,			FALSE	},
	{ "BUFFER *",		MT_BUFFER,			FALSE	},
	{ "OBJ_INDEX_DATA *",	MT_OBJ_INDEX,			FALSE	},
	{ "MOB_INDEX_DATA *",	MT_MOB_INDEX,			FALSE	},
	{ "DESCRIPTOR_DATA *",	MT_DESCRIPTOR,			FALSE	},
	{ "skill_t *",		MT_SKILL,			FALSE	},

	{ "va_list",		MT_VA_LIST,			FALSE	},
	{ "size_t",		MT_SIZE_T,			FALSE	},
	{ "bool",		MT_BOOL,			FALSE	},
	{ "const void *",	MT_PCVOID,			FALSE	},
	{ "actopt_t *",		MT_ACTOPT,			FALSE	},
	{ "const char **",	MT_PCCHAR,			FALSE	},
	{ "char *",		MT_PCHAR,			FALSE	},
	{ "gmlstr_t *",		MT_GMLSTR,			FALSE	},
	{ "mlstring *",		MT_MLSTR,			FALSE	},
	{ "int *",		MT_PINT,			FALSE	},
	{ "spec_skill_t *",	MT_SPEC_SKILL,			FALSE	},
	{ "uint",		MT_UINT,			FALSE	},

	{ NULL, 0, FALSE }
};

flaginfo_t mptrig_types[] =
{
	{ "",			TABLE_INTVAL,		FALSE	},

	{ "mob_act",		TRIG_MOB_ACT,		TRUE	},
	{ "mob_bribe",		TRIG_MOB_BRIBE,		TRUE	},
	{ "mob_death",		TRIG_MOB_DEATH,		TRUE	},
	{ "mob_entry",		TRIG_MOB_ENTRY,		TRUE	},
	{ "mob_fight",		TRIG_MOB_FIGHT,		TRUE	},
	{ "mob_give",		TRIG_MOB_GIVE,		TRUE	},
	{ "mob_greet",		TRIG_MOB_GREET,		TRUE    },
	{ "mob_kill",		TRIG_MOB_KILL,		TRUE	},
	{ "mob_random",		TRIG_MOB_RANDOM,	TRUE	},
	{ "mob_speech",		TRIG_MOB_SPEECH,	TRUE	},
	{ "mob_exit",		TRIG_MOB_EXIT,		TRUE    },
	{ "mob_delay",		TRIG_MOB_DELAY,		TRUE    },
	{ "mob_look",		TRIG_MOB_LOOK,		TRUE	},
	{ "mob_open",		TRIG_MOB_OPEN,		TRUE	},
	{ "mob_get",		TRIG_MOB_GET,		TRUE	},
	{ "mob_tell",		TRIG_MOB_TELL,		TRUE	},
	{ "mob_cmd",		TRIG_MOB_CMD,		TRUE	},
	{ "mob_yell",		TRIG_MOB_YELL,		TRUE	},

	{ "obj_wear",		TRIG_OBJ_WEAR,		TRUE	},
	{ "obj_remove",		TRIG_OBJ_REMOVE,	TRUE	},
	{ "obj_drop",		TRIG_OBJ_DROP,		TRUE	},
	{ "obj_sac",		TRIG_OBJ_SAC,		TRUE	},
	{ "obj_give",		TRIG_OBJ_GIVE,		TRUE	},
	{ "obj_greet",		TRIG_OBJ_GREET,		TRUE	},
	{ "obj_fight",		TRIG_OBJ_FIGHT,		TRUE	},
	{ "obj_death",		TRIG_OBJ_DEATH,		TRUE	},
	{ "obj_speech",		TRIG_OBJ_SPEECH,	TRUE	},
	{ "obj_entry",		TRIG_OBJ_ENTRY,		TRUE	},
	{ "obj_get",		TRIG_OBJ_GET,		TRUE	},
	{ "obj_random",		TRIG_OBJ_RANDOM,	TRUE	},
	{ "obj_enter",		TRIG_OBJ_ENTER,		TRUE	},
	{ "obj_look",		TRIG_OBJ_LOOK,		TRUE	},
	{ "obj_open",		TRIG_OBJ_OPEN,		TRUE	},
	{ "obj_close",		TRIG_OBJ_CLOSE,		TRUE	},
	{ "obj_lock",		TRIG_OBJ_LOCK,		TRUE	},
	{ "obj_unlock",		TRIG_OBJ_UNLOCK,	TRUE	},
	{ "obj_damage",		TRIG_OBJ_DAMAGE,	TRUE	},
	{ "obj_repair",		TRIG_OBJ_REPAIR,	TRUE	},
	{ "obj_cmd",		TRIG_OBJ_CMD,		TRUE	},
	{ "obj_use",		TRIG_OBJ_USE,		TRUE	},

	{ "room_random",	TRIG_ROOM_RANDOM,	TRUE	},
	{ "room_cmd",		TRIG_ROOM_CMD,		TRUE	},
	{ "room_greet",		TRIG_ROOM_GREET,	TRUE	},
	{ "room_dopen",		TRIG_ROOM_DOPEN,	TRUE	},
	{ "room_dclose",	TRIG_ROOM_DCLOSE,	TRUE	},
	{ "room_dlock",		TRIG_ROOM_DLOCK,	TRUE	},
	{ "room_dunlock",	TRIG_ROOM_DUNLOCK,	TRUE	},

	{ "spec",		TRIG_SPEC,		TRUE	},

	{ NULL, 0, FALSE }
};

flaginfo_t mptrig_flags[] =
{
	{ "",			TABLE_BITVAL,		FALSE	},

	{ "case-sensitive",	TRIG_F_CASEDEP,		FALSE	},
	{ "regexp",		TRIG_F_REGEXP,		FALSE	},

	{ NULL, 0, FALSE }
};

flaginfo_t mprog_types[] =
{
	{ "",			TABLE_INTVAL,		FALSE	},

	{ "mob",		MP_T_MOB,		TRUE	},
	{ "obj",		MP_T_OBJ,		TRUE	},
	{ "room",		MP_T_ROOM,		TRUE	},
	{ "spec",		MP_T_SPEC,		TRUE	},

	{ NULL, 0, FALSE }
};

flaginfo_t mprog_flags[] =
{
	{ "",			TABLE_BITVAL,		FALSE	},

	{ "trace",		MP_F_TRACE,		TRUE	},
	{ "changed",		MP_F_CHANGED,		FALSE	},

	{ NULL, 0, FALSE }
};

flaginfo_t mprog_states[] =
{
	{ "",			TABLE_INTVAL,		FALSE	},

	{ "dirty",		MP_S_DIRTY,		FALSE	},
	{ "ready",		MP_S_READY,		FALSE	},

	{ NULL, 0, FALSE }
};

/*
 * Attribute bonus tables.
 */
const	struct	str_app_type	str_app		[26]		=
{
    { -5, -4,   0,    0 },  /* 0  */
    { -5, -4,   30,  10 },  /* 1  */
    { -3, -2,   30,  20 },
    { -3, -1,  100,  30 },  /* 3  */
    { -2, -1,  250,  40 },
    { -2, -1,  550,  50 },  /* 5  */
    { -1,  0,  800,  60 },
    { -1,  0,  900,  70 },
    {  0,  0, 1000,  80 },
    {  0,  0, 1000,  90 },
    {  0,  0, 1150, 100 }, /* 10  */
    {  0,  0, 1150, 110 },
    {  0,  0, 1300, 120 },
    {  0,  0, 1300, 130 }, /* 13  */
    {  0,  1, 1400, 140 },
    {  1,  1, 1500, 150 }, /* 15  */
    {  1,  2, 1650, 160 },
    {  2,  3, 1800, 220 },
    {  2,  3, 2000, 250 }, /* 18  */
    {  3,  4, 2250, 300 },
    {  3,  5, 2500, 350 }, /* 20  */
    {  4,  6, 3000, 400 },
    {  4,  6, 3500, 450 },
    {  5,  7, 4000, 500 },
    {  5,  8, 4500, 550 },
    {  6,  9, 5000, 600 }  /* 25   */
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
