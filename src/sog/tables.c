/*
 * $Id: tables.c,v 1.120 1999-12-20 12:40:37 fjoe Exp $
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
#include "lang.h"
#include "ban.h"
#include "note.h"

/*
 * first element of each flaginfo_t[] table describes type of values
 * in the table.
 */

flaginfo_t slang_table[] =
{
	{ "",			TABLE_INTVAL,			},

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

flaginfo_t size_table[] =
{ 
	{ "",			TABLE_INTVAL			},

	{ "tiny",		SIZE_TINY,		TRUE	},
	{ "small", 		SIZE_SMALL,		TRUE	},
	{ "medium",		SIZE_MEDIUM,		TRUE	},
	{ "large",		SIZE_LARGE,		TRUE	},
	{ "huge", 		SIZE_HUGE,		TRUE	},
	{ "giant",		SIZE_GIANT,		TRUE	},
	{ "gargantuan",		SIZE_GARGANTUAN,	TRUE	},

	{ NULL }
};

flaginfo_t act_flags[] =
{
	{ "",			TABLE_BITVAL			},

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
	{ "undead",		ACT_UNDEAD,		TRUE	},
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

	{ NULL }
};

flaginfo_t mob_flags[] =
{
	{ "",			TABLE_BITVAL			},

	{ "changer",		MOB_CHANGER,		TRUE	},
	{ "gain",		MOB_GAIN,		TRUE	},
	{ "train",		MOB_TRAIN,		TRUE	},
	{ "practice",		MOB_PRACTICE,		TRUE	},
	{ "questor",		MOB_QUESTOR,		TRUE	},
	{ "repairman",		MOB_REPAIRMAN,		TRUE	},
	{ "sage",		MOB_SAGE,		TRUE	},
	{ "healer",		MOB_HEALER,		TRUE	},
	{ "clan_guard",		MOB_CLAN_GUARD,		TRUE	},

	{ NULL }
};

flaginfo_t plr_flags[] =
{
	{ "",			TABLE_BITVAL			},

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
	{ "notitle",		PLR_NOTITLE,		FALSE	},
	{ "noexp",		PLR_NOEXP,		FALSE	},
	{ "hara_kiri",		PLR_HARA_KIRI,		FALSE	},

	{ NULL }
};

flaginfo_t affect_flags[] =
{
	{ "",			TABLE_BITVAL			},

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

	{ NULL }
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

	{ NULL }
};

flaginfo_t off_flags[] =
{
	{ "",			TABLE_BITVAL			},

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
	{ "distance", 		OFF_DISTANCE,		TRUE	},
	{ "deathblow",		OFF_DEATHBLOW,		TRUE	},
	{ "spellbane",		OFF_SPELLBANE,		TRUE	},
	{ "assist_all",		ASSIST_ALL,		TRUE	},
	{ "assist_align",	ASSIST_ALIGN,		TRUE	},
	{ "assist_race",	ASSIST_RACE,		TRUE	},
	{ "assist_players",	ASSIST_PLAYERS,		TRUE	},
	{ "assist_guard",	ASSIST_GUARD,		TRUE	},
	{ "assist_vnum",	ASSIST_VNUM,		TRUE	},

	{ NULL }
};

flaginfo_t form_flags[] =
{
	{ "",			TABLE_BITVAL			},

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

flaginfo_t part_flags[] =
{
	{ "",			TABLE_BITVAL			},

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

flaginfo_t comm_flags[] =
{
	{ "",			TABLE_BITVAL			},

	{ "quiet",		COMM_QUIET,		TRUE	},
	{ "deaf",		COMM_DEAF,		TRUE	},
	{ "noautoflee",		COMM_NOFLEE,		TRUE	},
	{ "nonote",		COMM_NONOTE,		TRUE	},
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

	{ NULL }
};

flaginfo_t chan_flags[] =
{
	{ "",			TABLE_BITVAL			},

	{ "nochannels",		CHAN_NOCHANNELS,	FALSE	},
	{ "nowiz",		CHAN_NOWIZ,		TRUE	},
	{ "nogossip",		CHAN_NOGOSSIP,		TRUE	},
	{ "noquestion",		CHAN_NOQUESTION,	TRUE	},
	{ "noauction",		CHAN_NOAUCTION,		TRUE	},
	{ "nomusic",		CHAN_NOMUSIC,		TRUE	},
	{ "noquote",		CHAN_NOQUOTE,		TRUE	},
	{ "noclan",		CHAN_NOCLAN,		TRUE	},
	{ "noshout",		CHAN_NOSHOUT,		FALSE	},

	{ NULL }
};

flaginfo_t clan_flags[] =
{
	{ "",			TABLE_BITVAL			},

	{ "hidden",		CLAN_HIDDEN,		TRUE	},
	{ "changed",		CLAN_CHANGED,		FALSE	},

	{ NULL }
};

flaginfo_t area_flags[] =
{
	{ "",			TABLE_BITVAL			},

	{ "closed",		AREA_CLOSED,		TRUE	},
	{ "noquest",		AREA_NOQUEST,		TRUE	},
	{ "update_always",	AREA_UPDATE_ALWAYS,	TRUE	},
	{ "update_frequently",	AREA_UPDATE_FREQUENTLY,	TRUE	},
	{ "tagged",		AREA_TAGGED,		TRUE	},
	{ "changed",		AREA_CHANGED,		FALSE	},

	{ NULL }
};

flaginfo_t sex_table[] =
{
	{ "",			TABLE_INTVAL			},

	{ "none",		SEX_NEUTRAL,		TRUE	},
	{ "male",		SEX_MALE,		TRUE	},
	{ "female",		SEX_FEMALE,		TRUE	},
	{ "either",		SEX_EITHER,		TRUE	},
	{ "random",		SEX_EITHER,		TRUE    },
	{ "neutral",		SEX_NEUTRAL,		TRUE	},
	{ "plural",		SEX_PLURAL,		TRUE	},

	{ NULL	}
};

flaginfo_t gender_table[] =
{
	{ "",			TABLE_INTVAL			},

	{ "none",		SEX_NEUTRAL,		TRUE	},
	{ "male",		SEX_MALE,		TRUE	},
	{ "female",		SEX_FEMALE,		TRUE	},
	{ "neutral",		SEX_NEUTRAL,		TRUE	},
	{ "plural",		SEX_PLURAL,		TRUE	},

	{ NULL	}
};

flaginfo_t exit_flags[] =
{
	{ "",			TABLE_BITVAL			},

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

	{ NULL }
};

flaginfo_t door_resets[] =
{
	{ "",			TABLE_INTVAL			},

	{ "open",			0,		TRUE	},
	{ "closed",			1,		TRUE	},
	{ "closed and locked",		2,		TRUE	},

	{ NULL }
};

flaginfo_t room_flags[] =
{
	{ "",			TABLE_BITVAL			},

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

	{ NULL }
};

flaginfo_t sector_types[] =
{
	{ "",			TABLE_INTVAL			},

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

	{ NULL }
};

flaginfo_t item_types[] =
{
	{ "",			TABLE_INTVAL			},

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

flaginfo_t stat_flags[] =
{
	{ "",			TABLE_BITVAL			},

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

	{ NULL }
};

flaginfo_t obj_flags[] =
{
	{ "",			TABLE_BITVAL			},

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

	{ NULL }
};

flaginfo_t wear_flags[] =
{
	{ "",			TABLE_BITVAL			},

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

	{ NULL }
};

/*
* Used when adding an affect to tell where it goes.
* See addaffect and delaffect in act_olc.c
*/
flaginfo_t apply_flags[] =
{
	{ "",			TABLE_INTVAL			},

	{ "none",		APPLY_NONE,		TRUE	},
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
	{ "size",		APPLY_SIZE,		TRUE	},
	{ "resist_bash",	APPLY_RESIST_BASH,	TRUE	},
	{ "resist_pierce",	APPLY_RESIST_PIERCE,	TRUE	},
	{ "resist_slash",	APPLY_RESIST_SLASH,	TRUE	},
	{ "resist_fire",	APPLY_RESIST_FIRE,	TRUE	},
	{ "resist_cold",	APPLY_RESIST_COLD,	TRUE	},
	{ "resist_lightning",	APPLY_RESIST_LIGHTNING, TRUE	},
	{ "resist_acid",	APPLY_RESIST_ACID,	TRUE	},
	{ "resist_holy", 	APPLY_RESIST_HOLY,	TRUE	},
	{ "resist_negative",	APPLY_RESIST_NEGATIVE,	TRUE	},
	{ "resist_energy",	APPLY_RESIST_ENERGY,	TRUE	},
	{ "resist_mental",	APPLY_RESIST_MENTAL,	TRUE	},
	{ "resist_sound",	APPLY_RESIST_SOUND,	TRUE	},
	{ "resist_disease",	APPLY_RESIST_DISEASE,	TRUE	},
	{ "resist_poison",	APPLY_RESIST_POISON,	TRUE	},
	{ "resist_charm",	APPLY_RESIST_CHARM,	TRUE	},
	{ "resist_harm",	APPLY_RESIST_HARM,	TRUE	},
	{ "resist_light",	APPLY_RESIST_LIGHT,	TRUE	},

	{ NULL }
};

flaginfo_t rapply_flags[] =
{
	{ "",			TABLE_INTVAL			},

	{ "healrate",		APPLY_ROOM_HEAL,	TRUE	},
	{ "manarate",		APPLY_ROOM_MANA,	TRUE	},

	{ NULL }
};

flaginfo_t resist_flags[] =
{
	{ "",			TABLE_INTVAL			},

	{ "bash",		RESIST_BASH,		TRUE	},	
	{ "pierce",		RESIST_PIERCE,		TRUE	},	
	{ "slash",		RESIST_SLASH,		TRUE	},	
	{ "fire",		RESIST_FIRE,		TRUE	},	
	{ "cold",		RESIST_COLD,		TRUE	},	
	{ "lightning",		RESIST_LIGHTNING,	TRUE	},	
	{ "acid",		RESIST_ACID,		TRUE	},	
	{ "holy",	 	RESIST_HOLY,		TRUE	},	
	{ "negative",		RESIST_NEGATIVE,	TRUE	},	
	{ "energy",		RESIST_ENERGY,		TRUE	},	
	{ "mental",		RESIST_MENTAL,		TRUE	},	
	{ "sound",		RESIST_SOUND,		TRUE	},	
	{ "disease",		RESIST_DISEASE,		TRUE	},
	{ "poison",		RESIST_POISON,		TRUE	},
	{ "charm",		RESIST_CHARM,		TRUE	},
	{ "harm",		RESIST_HARM,		TRUE	},
	{ "light",		RESIST_LIGHT,		TRUE	},

	{ NULL }
};

flaginfo_t resist_info_flags[] =
{
	{ "",			TABLE_INTVAL			},

	{ "bashing",		RESIST_BASH,		TRUE	},	
	{ "piercing",		RESIST_PIERCE,		TRUE	},	
	{ "slashing",		RESIST_SLASH,		TRUE	},	
	{ "fire attacks",	RESIST_FIRE,		TRUE	},	
	{ "cold attacks",	RESIST_COLD,		TRUE	},	
	{ "lightning",		RESIST_LIGHTNING,	TRUE	},	
	{ "acid",		RESIST_ACID,		TRUE	},	
	{ "holy energy", 	RESIST_HOLY,		TRUE	},	
	{ "negative energy",	RESIST_NEGATIVE,	TRUE	},	
	{ "energy attacks",	RESIST_ENERGY,		TRUE	},	
	{ "mental attacks",	RESIST_MENTAL,		TRUE	},	
	{ "sound based attack",	RESIST_SOUND,		TRUE	},	
	{ "disease",		RESIST_DISEASE,		TRUE	},	
	{ "poison",		RESIST_POISON,		TRUE	},
	{ "charming spells",	RESIST_CHARM,		TRUE	},
	{ "harm spells",	RESIST_HARM,		TRUE	},
	{ "light based attacks",RESIST_LIGHT,		TRUE	},

	{ NULL }
};
/*
 * What is seen.
 */
flaginfo_t wear_loc_strings[] =
{
	{ "",				TABLE_INTVAL			},

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
	{ "clan mark",			WEAR_CLANMARK,		TRUE	},
	{ "stuck in",			WEAR_STUCK_IN,		TRUE	},

	{ NULL }
};

flaginfo_t wear_loc_flags[] =
{
	{ "",			TABLE_INTVAL			},

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
	{ "clanmark",		WEAR_CLANMARK,		TRUE	},

	{ NULL }
};

flaginfo_t cont_flags[] =
{
	{ "",			TABLE_BITVAL			},

	{ "closable",		CONT_CLOSEABLE,		TRUE	},
	{ "pickproof",		CONT_PICKPROOF,		TRUE	},
	{ "closed",		CONT_CLOSED,		TRUE	},
	{ "locked",		CONT_LOCKED,		TRUE	},
	{ "put_on",		CONT_PUT_ON,		TRUE	},
	{ "quiver",		CONT_QUIVER,		TRUE	},

	{ NULL }
};

flaginfo_t ac_type[] =
{
	{ "",			TABLE_INTVAL			},

	{ "pierce",		AC_PIERCE,		TRUE	},
	{ "bash",		AC_BASH,		TRUE	},
	{ "slash",		AC_SLASH,		TRUE	},
	{ "exotic",		AC_EXOTIC,		TRUE	},

	{ NULL }
};

flaginfo_t weapon_class[] =
{
	{ "",			TABLE_INTVAL			},

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

	{ NULL }
};

flaginfo_t weapon_type2[] =
{
	{ "",			TABLE_BITVAL			},

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

	{ NULL }
};

flaginfo_t sk_aff_flags[] =
{
	{ "",			TABLE_BITVAL			},

	{ "teach",		SK_AFF_TEACH,		TRUE	},
	{ "all",		SK_AFF_ALL,		TRUE	},
	{ "notclan",		SK_AFF_NOTCLAN,		TRUE	},

	{ NULL }
};

flaginfo_t position_table[] =
{
	{ "",			TABLE_INTVAL			},

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

flaginfo_t position_names[] =
{
	{ "",			TABLE_INTVAL	},

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

flaginfo_t portal_flags[]=
{
	{ "",			TABLE_BITVAL			},

	{ "normal_exit",	GATE_NORMAL_EXIT,	TRUE	},
	{ "nocurse",		GATE_NOCURSE,		TRUE	},
	{ "gowith",		GATE_GOWITH,		TRUE	},
	{ "buggy",		GATE_BUGGY,		TRUE	},
	{ "random",		GATE_RANDOM,		TRUE	},

	{ NULL }
};

flaginfo_t furniture_flags[]=
{
	{ "",			TABLE_BITVAL			},

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

flaginfo_t raffect_flags[] =
{
	{ "",			TABLE_BITVAL			},

	{ "randomizer",		RAFF_RANDOMIZER,	TRUE	},
	{ "espirit",		RAFF_ESPIRIT,		TRUE	},
	{ "curse",		RAFF_CURSE,		TRUE	},

	{ NULL }
};

flaginfo_t skill_groups[] =
{
	{ "",			TABLE_BITVAL			},

	{ "none",		GROUP_NONE,		TRUE	},
	{ "weaponsmaster",	GROUP_WEAPONSMASTER,	TRUE	},
	{ "attack",		GROUP_ATTACK,		TRUE	},
	{ "beguiling",		GROUP_BEGUILING,	TRUE	},
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
	{ "suddendeath",	GROUP_SUDDENDEATH,	TRUE	},
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
	{ "charm", 		GROUP_CHARM,		TRUE	},

	{ NULL }
};

flaginfo_t mptrig_types[] =
{
	{ "",			TABLE_INTVAL			},

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

flaginfo_t mptrig_flags[] =
{
	{ "",			TABLE_BITVAL			},

	{ "case-sensitive",	TRIG_CASEDEP,		FALSE	},

	{ NULL }
};

flaginfo_t skill_targets[] =
{
	{ "",			TABLE_INTVAL			},

	{ "ignore",		TAR_IGNORE,		TRUE	},
	{ "charoff",		TAR_CHAR_OFFENSIVE,	TRUE	},
	{ "chardef",		TAR_CHAR_DEFENSIVE,	TRUE	},
	{ "charself",		TAR_CHAR_SELF,		TRUE	},
	{ "objinv",		TAR_OBJ_INV,		TRUE	},
	{ "objchardef",		TAR_OBJ_CHAR_DEF,	TRUE	},
	{ "objcharoff",		TAR_OBJ_CHAR_OFF,	TRUE	},

	{ NULL }
};

flaginfo_t stat_names[] =
{
	{ "",			TABLE_INTVAL			},

	{ "str",		STAT_STR,		TRUE	},
	{ "int",		STAT_INT,		TRUE	},
	{ "dex",		STAT_DEX,		TRUE	},
	{ "wis",		STAT_WIS,		TRUE	},
	{ "con",		STAT_CON,		TRUE	},
	{ "cha",		STAT_CHA,		TRUE	},

	{ NULL }
};

flaginfo_t material_flags[] =
{
	{ "",			TABLE_BITVAL			},

	{ "indestruct",		MATERIAL_INDESTRUCT,	TRUE	},
	{ "metal",		MATERIAL_METAL,		TRUE	},
	{ "edible",		MATERIAL_EDIBLE,	TRUE	},
	{ "susc_heat",		MATERIAL_SUSC_HEAT,	TRUE	},
	{ "susc_cold",		MATERIAL_SUSC_COLD,	TRUE	},
	{ "fragile",		MATERIAL_FRAGILE,	TRUE	},

	{ NULL }
};

flaginfo_t skill_flags[] =
{
	{ "",			TABLE_BITVAL			},

	{ "clan",		SKILL_CLAN,		TRUE	},
	{ "range",		SKILL_RANGE,		TRUE	},
	{ "area_attack",	SKILL_AREA_ATTACK,	TRUE	},
	{ "questionable",	SKILL_QUESTIONABLE,	TRUE	},
	{ "form",		SKILL_FORM,		TRUE	},

	{ NULL }
};

flaginfo_t skill_types[] =
{
	{ "",			TABLE_INTVAL,			},

	{ "skill",		ST_SKILL,		TRUE	},
	{ "spell",		ST_SPELL,		TRUE	},
	{ "prayer",		ST_PRAYER,		TRUE	},

	{ NULL }
};

flaginfo_t class_flags[] =
{
	{ "",			TABLE_BITVAL			},

	{ "magic",		CLASS_MAGIC,		TRUE	},
	{ "noch",		CLASS_NOCH,		TRUE	},
	{ "changed",		CLASS_CHANGED,		FALSE	},

	{ NULL }
};

flaginfo_t race_flags[] =
{
	{ "",			TABLE_BITVAL			},

	{ "noch",		RACE_NOCH,		TRUE	},
	{ "changed",		RACE_CHANGED,		FALSE	},

	{ NULL }
};

flaginfo_t shapeform_flags[] = 
{
	{ "",			TABLE_BITVAL			},
	{ "nospeak",		FORM_NOSPEAK,		TRUE	},
	{ "nocast",		FORM_NOCAST,		TRUE	},
	{ "castself",		FORM_CASTSELF,		TRUE	},
	{ "noget",		FORM_NOGET,		TRUE	},

	{ NULL}
};

flaginfo_t ralign_names[] =
{
	{ "",			TABLE_BITVAL			},

	{ "evil",		RA_EVIL,		TRUE	},
	{ "neutral",		RA_NEUTRAL,		TRUE	},
	{ "good",		RA_GOOD,		TRUE	},

	{ NULL }
};

flaginfo_t align_names[] =
{
	{ "",			TABLE_INTVAL			},

	{ "good",		ANUM_GOOD,		TRUE	},
	{ "neutral",		ANUM_NEUTRAL,		TRUE	},
	{ "evil",		ANUM_EVIL,		TRUE	},

	{ NULL }
};

flaginfo_t lang_flags[] =
{
	{ "",			TABLE_BITVAL			},

	{ "hidden",		LANG_HIDDEN,		TRUE	},
	{ "changed",		LANG_CHANGED,		FALSE	},

	{ NULL }
};

flaginfo_t ethos_table[] =
{
	{ "",			TABLE_BITVAL			},

	{ "lawful",		ETHOS_LAWFUL,		TRUE	},
	{ "neutral",		ETHOS_NEUTRAL,		TRUE	},
	{ "chaotic",		ETHOS_CHAOTIC,		TRUE	},

	{ NULL }
};

flaginfo_t rulecl_names[] =
{
	{ "",			TABLE_INTVAL			},

	{ "cases",		RULES_CASE,		TRUE	},
	{ "genders",		RULES_GENDER,		TRUE	},
	{ "qtys",		RULES_QTY,		TRUE	},

	{ NULL }
};

flaginfo_t rulecl_flags[] =
{
	{ "",			TABLE_BITVAL			},

	{ "expl_changed",	RULES_EXPL_CHANGED		},
	{ "impl_changed",	RULES_IMPL_CHANGED		},

	{ NULL }
};

flaginfo_t note_types[] =
{
	{ "",			TABLE_INTVAL			},

	{ "note",		NOTE_NOTE			},
	{ "idea",		NOTE_IDEA			},
	{ "penalty",		NOTE_PENALTY			},
	{ "news",		NOTE_NEWS			},
	{ "change",		NOTE_CHANGES			},

	{ NULL }
};

flaginfo_t options_table[] =
{
	{ "",			TABLE_BITVAL			},

	{ "ascii_only_names",	OPT_ASCII_ONLY_NAMES		},

	{ NULL }
};

flaginfo_t ban_actions[] =
{
	{ "",			TABLE_INTVAL			},

	{ "allow",		BA_ALLOW			},
	{ "deny",		BA_DENY				},

	{ NULL }
};

flaginfo_t ban_classes[] =
{
	{ "",			TABLE_INTVAL			},

	{ "all",		BCL_ALL				},
	{ "players",		BCL_PLAYERS			},
	{ "newbies",		BCL_NEWBIES			},

	{ NULL }
};

flaginfo_t cmd_flags[] =
{
	{ "",			TABLE_BITVAL,			},

	{ "keep_hide",		CMD_KEEP_HIDE,		TRUE	},
	{ "noorder",		CMD_NOORDER,		TRUE	},
	{ "charmed_ok",		CMD_CHARMED_OK,		TRUE	},
	{ "hidden",		CMD_HIDDEN,		TRUE	},
	{ "disabled",		CMD_DISABLED,		TRUE	},
	{ "frozen_ok",		CMD_FROZEN_OK,		TRUE	},

	{ NULL }
};

flaginfo_t cmd_logtypes[] =
{
	{ "",			TABLE_INTVAL,			},

	{ "normal",		LOG_NORMAL,		TRUE	},
	{ "always",		LOG_ALWAYS,		TRUE	},
	{ "never",		LOG_NEVER,		TRUE	},

	{ NULL }
};

flaginfo_t cmd_classes[] =
{
	{ "",			TABLE_INTVAL			},

	{ "ordinary",		CC_ORDINARY,		TRUE	},
	{ "core",		CC_CORE,		TRUE	},
	{ "olc",		CC_OLC,			TRUE	},

	{ NULL }
};

/*
 * "imm" should be before "immortal" for flag_lookup to work properly
 * "immortal" should be before "ava" for convenience
 */
flaginfo_t level_table[] =
{
	{ "",			TABLE_INTVAL,			},

	{ "imp",		LEVEL_IMP,		TRUE	},
	{ "cre",		LEVEL_CRE,		TRUE	},
	{ "dei",		LEVEL_DEI,		TRUE	},
	{ "god",		LEVEL_GOD,		TRUE	},
	{ "immortal",		LEVEL_IMMORTAL,		TRUE	},
	{ "ava",		LEVEL_AVA,		TRUE	},

	{ "hero",		LEVEL_HERO,		TRUE	},
	{ "newbie",		LEVEL_NEWBIE,		TRUE	},
	{ "pk",			LEVEL_PK,		TRUE	},

	{ NULL }
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
	{ "olc",		WIZ_OLC,	LEVEL_IMMORTAL	},
	{ "wizlog",		WIZ_WIZLOG,	LEVEL_IMMORTAL	},
	{ NULL }
};

flaginfo_t desc_con_table[] =
{
	{ "",			TABLE_INTVAL				},

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
	{ NULL }
};

flaginfo_t events_table[] =
{
	{ "",			TABLE_BITVAL				},

	{ "enter_room",		EVENT_ROOM_ENTER,		TRUE	},
	{ "leave_room",		EVENT_ROOM_LEAVE,		TRUE	},
	{ "update_room",	EVENT_ROOM_UPDATE,		TRUE	},
	{ "timeout_room",	EVENT_ROOM_TIMEOUT,		TRUE	},
	{ "update_char",	EVENT_CHAR_UPDATE,		TRUE	},
	{ "update_fast_char",	EVENT_CHAR_UPDFAST,		TRUE	},
	{ "timeout_char",	EVENT_CHAR_TIMEOUT,		TRUE	},
	{ NULL }
};

flaginfo_t dam_classes[] =
{
	{ "",			TABLE_INTVAL				},

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
	{ "drowning",		DAM_DROWNING,			TRUE	},
	{ "light",		DAM_LIGHT,			TRUE	},
	{ "other",		DAM_OTHER,			TRUE	},
	{ "harm",		DAM_HARM,			TRUE	},
	{ "charm",		DAM_CHARM,			TRUE	},
	{ "sound",		DAM_SOUND,			TRUE	},
	{ "thirst",		DAM_THIRST,			TRUE	},
	{ "hunger",		DAM_HUNGER,			TRUE	},
	{ "light_v",		DAM_LIGHT_V,			TRUE	},
	{ "trap_room",		DAM_TRAP_ROOM,			TRUE	},
	{ "wood",		DAM_WOOD,			TRUE	},
	{ "silver",		DAM_SILVER,			TRUE	},

	{ NULL }
};

flaginfo_t spec_classes[] =
{
	{ "",			TABLE_INTVAL				},

	{ "class",		SPEC_CLASS,			TRUE	},
	{ "race",		SPEC_RACE,			TRUE	},
	{ "clan",		SPEC_CLAN,			TRUE	},
	{ "weapon",		SPEC_WEAPON,			TRUE	},
	{ "major_school",	SPEC_MAJORSCHOOL,		TRUE	},
	{ "minor_school",	SPEC_MINORSCHOOL,		TRUE	},
	{ "form",		SPEC_FORM,			TRUE	},

	{ NULL }
};

flaginfo_t spec_flags[] =
{
	{ "",			TABLE_BITVAL,				},

	{ "changed",		SPF_CHANGED,			FALSE	},

	{ NULL }
};

flaginfo_t cond_table[] =
{
	{ "",			TABLE_INTVAL				},

	{ "drunkness",		COND_DRUNK,			TRUE	},
	{ "fullness",		COND_FULL,			TRUE	},
	{ "thirst",		COND_THIRST,			TRUE	},
	{ "hunger",		COND_HUNGER,			TRUE	},
	{ "bloodlust",		COND_BLOODLUST,			TRUE	},
	{ "desire",		COND_DESIRE,			TRUE	},

	{ NULL }
};

flaginfo_t affect_where_types[] =
{
	{ "",			TABLE_INTVAL				},

	{ "to_affects",		TO_AFFECTS,			TRUE	},
	{ "to_object",		TO_OBJECT,			FALSE	},
	{ "to_immune",		TO_IMMUNE,			TRUE	},
	{ "to_resist",		TO_RESIST,			TRUE	},
	{ "to_vuln",		TO_VULN,			TRUE	},
	{ "to_weapon",		TO_WEAPON,			FALSE	},
	{ "to_skills",		TO_SKILLS,			TRUE	},
	{ "to_race",		TO_RACE,			FALSE	},
	{ "to_detects",		TO_DETECTS,			TRUE	},
	{ "to_invis",		TO_INVIS,			TRUE	},
	{ "to_form",		TO_FORM,			FALSE	},
	{ "to_formaffects",	TO_FORMAFFECTS,			FALSE	},

	{ NULL }
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

