/*
 * $Id: interp.c,v 1.133 1999-04-20 05:18:36 kostik Exp $
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
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#include "merc.h"
#include "interp.h"
#include "olc/olc.h"
#include "db/cmd.h"
#include "db/socials.h"

#undef IMMORTALS_LOGS

void interpret_social(social_t *soc, CHAR_DATA *ch, const char *argument);

/*
 * Command logging types.
 */
#define LOG_NORMAL	0
#define LOG_ALWAYS	1
#define LOG_NEVER	2

/*
 * Log-all switch.
 */
bool				fLogAll		= FALSE;

#ifdef IMMORTALS_LOGS
/*
 * immortals log file
 */
FILE				*imm_log;
#endif

/*
 * Command table.
 */
cmd_t cmd_table[] =
{
    /*
     * Common movement commands.
     */
    { "north",		do_north,	POS_STANDING,    0,  LOG_NEVER, CMD_KEEP_HIDE | CMD_HIDDEN },
    { "east",		do_east,	POS_STANDING,	 0,  LOG_NEVER, CMD_KEEP_HIDE | CMD_HIDDEN },
    { "south",		do_south,	POS_STANDING,	 0,  LOG_NEVER, CMD_KEEP_HIDE | CMD_HIDDEN },
    { "west",		do_west,	POS_STANDING,	 0,  LOG_NEVER, CMD_KEEP_HIDE | CMD_HIDDEN },
    { "up",		do_up,		POS_STANDING,	 0,  LOG_NEVER, CMD_KEEP_HIDE | CMD_HIDDEN },
    { "down",		do_down,	POS_STANDING,	 0,  LOG_NEVER, CMD_KEEP_HIDE | CMD_HIDDEN },

    /*
     * Common other commands.
     * Placed here so one and two letter abbreviations work.
     */
    { "at",             do_at,          POS_DEAD,       L6,  LOG_NORMAL, CMD_KEEP_HIDE  },
    { "cast",		do_cast,	POS_FIGHTING,	 0,  LOG_NORMAL, CMD_NOORDER },
    { "auction",        do_auction,     POS_SLEEPING,    0,  LOG_NORMAL	},
    { "buy",		do_buy,		POS_RESTING,	 0,  LOG_NORMAL	},
    { "channels",       do_channels,    POS_DEAD,        0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "dual",		do_second_wield,POS_RESTING,	 0,  LOG_NORMAL },
    { "exits",		do_exits,	POS_RESTING,	 0,  LOG_NORMAL, CMD_KEEP_HIDE },
    { "estimate",	do_estimate,	POS_RESTING,	 0,  LOG_NORMAL },
    { "get",		do_get,		POS_RESTING,	 0,  LOG_NORMAL, CMD_KEEP_HIDE },

    { "goto",           do_goto,        POS_DEAD,       L8,  LOG_NORMAL, CMD_KEEP_HIDE },
    { "glist",          do_glist,       POS_DEAD,        0,  LOG_NEVER	},
    { "group",          do_group,       POS_SLEEPING,    0,  LOG_NORMAL, CMD_KEEP_HIDE },
    { "hit",		do_kill,	POS_FIGHTING,	 0,  LOG_NORMAL, CMD_HIDDEN },
    { "inventory",	do_inventory,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "kill",		do_kill,	POS_FIGHTING,	 0,  LOG_NORMAL	},
    { "look",		do_look,	POS_RESTING,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "order",		do_order,	POS_RESTING,	 0,  LOG_NORMAL	},
    { "practice",       do_practice,	POS_SLEEPING,    0,  LOG_NORMAL, CMD_KEEP_HIDE },
    { "rest",		do_rest,	POS_SLEEPING,	 0,  LOG_NORMAL, CMD_KEEP_HIDE },
    { "repair",		do_repair,	POS_SLEEPING,	 0,  LOG_NORMAL, CMD_KEEP_HIDE },
    { "label",		do_label,	POS_RESTING,	 0,  LOG_NORMAL, CMD_NOORDER},
    { "second",		do_second_wield,POS_RESTING,	 0,  LOG_NORMAL	},
    { "sit",		do_sit,		POS_SLEEPING,    0,  LOG_NORMAL	},
    { "smithing",	do_smithing,	POS_RESTING,	 0,  LOG_NORMAL	},
    { "sockets",        do_sockets,	POS_DEAD,       L4,  LOG_NORMAL, CMD_KEEP_HIDE},
    { "stand",		do_stand,	POS_SLEEPING,	 0,  LOG_NORMAL	},
    { "tell",		do_tell,	POS_SLEEPING,	 0,  LOG_NORMAL, CMD_CHARMED_OK },
    { "unlock",         do_unlock,      POS_RESTING,     0,  LOG_NORMAL	},
    { "wield",		do_wear,	POS_RESTING,	 0,  LOG_NORMAL	},
    { "wizhelp",	do_wizhelp,	POS_DEAD,	IM,  LOG_NORMAL, CMD_KEEP_HIDE },
    { "language",	do_lang,	POS_DEAD,	0,   LOG_NORMAL	},

    /*
     * Informational commands.
     */
    { "affects",	do_affects,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "afk",		do_afk,		POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE },
    { "areas",		do_areas,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE },
    { "balance",	do_balance,	POS_STANDING,	 0,  LOG_NORMAL	},
    { "bug",		do_bug,		POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE },
    { "changes",	do_changes,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE },
    { "commands",	do_commands,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE },
    { "compare",	do_compare,	POS_RESTING,	 0,  LOG_NORMAL, CMD_KEEP_HIDE },
    { "consider",	do_consider,	POS_RESTING,	 0,  LOG_NORMAL, CMD_KEEP_HIDE },
    { "concentrate",	do_concentrate,	POS_STANDING,	 0,  LOG_NORMAL	},
    { "count",		do_count,	POS_SLEEPING,	HE,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "credits",	do_credits,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "date",		do_date,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "deposit",	do_deposit,	POS_STANDING,	 0,  LOG_NORMAL	},
    { "equipment",	do_equipment,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "escape",		do_escape,	POS_FIGHTING,	 0,  LOG_NORMAL	},
    { "examine",	do_examine,	POS_RESTING,	 0,  LOG_NORMAL, CMD_CHARMED_OK }, 
    { "help",		do_help,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "ideas",		do_idea,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "news",		do_news,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "raffects",	do_raffects,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE },
    { "rating",		do_rating,	POS_DEAD,	ML,  LOG_NORMAL, CMD_KEEP_HIDE },
    { "read",		do_look,	POS_RESTING,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "report",		do_report,	POS_RESTING,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "rules",		do_rules,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "scan",           do_scan,        POS_RESTING,     0,  LOG_NORMAL, CMD_KEEP_HIDE },
    { "oscore",		do_oscore,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "skills",		do_skills,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "speak",		do_speak,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "socials",	do_socials,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "spells",		do_spells,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "time",		do_time,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "typo",		do_typo,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "weather",	do_weather,	POS_RESTING,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "who",		do_who,		POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "whois",		do_whois,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "withdraw",	do_withdraw,	POS_STANDING,    0,  LOG_NORMAL	},
    { "wizlist",	do_wizlist,	POS_DEAD,        0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "worth",		do_worth,	POS_SLEEPING,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK },

    /*
     * Communication commands.
     */
    { "bearcall",       do_bear_call,   POS_FIGHTING,    0,  LOG_NORMAL	},
    { "clan",		do_clan,	POS_SLEEPING,    0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "=",		do_clan,	POS_SLEEPING,    0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "clanlist",	do_clanlist,	POS_SLEEPING,    0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "clanrecall",	do_crecall,	POS_FIGHTING,	 0,  LOG_NORMAL	},
    { "deaf",		do_deaf,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "emote",		do_emote,	POS_RESTING,	 0,  LOG_NORMAL, CMD_CHARMED_OK },
    { "pmote",		do_pmote,	POS_RESTING,	 0,  LOG_NORMAL, CMD_CHARMED_OK },
    { ",",		do_emote,	POS_RESTING,	 0,  LOG_NORMAL, CMD_CHARMED_OK },
    { "gtell",		do_gtell,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { ";",		do_gtell,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "notes",		do_note,	POS_SLEEPING,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "petitio",	do_petitio,	POS_DEAD,	 0,  LOG_NORMAL, CMD_CHARMED_OK },
    { "petition",	do_petition,	POS_DEAD,	 0,  LOG_NORMAL, CMD_NOORDER },
    { "pose",		do_pose,	POS_RESTING,	 0,  LOG_NORMAL, CMD_CHARMED_OK },
    { "promote",	do_promote,	POS_DEAD,	 0,  LOG_NORMAL	},
    { "pray",           do_pray,        POS_DEAD,        0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_FROZEN_OK },
    { "reply",		do_reply,	POS_SLEEPING,	 0,  LOG_NORMAL, CMD_CHARMED_OK },
    { "replay",		do_replay,	POS_SLEEPING,	 0,  LOG_NORMAL, CMD_CHARMED_OK },
    { "say",		do_say,		POS_RESTING,	 0,  LOG_NORMAL, CMD_CHARMED_OK },
    { "'",		do_say,		POS_RESTING,	 0,  LOG_NORMAL, CMD_CHARMED_OK },
    { "shout",		do_shout,	POS_DEAD,	 3,  LOG_NORMAL, CMD_CHARMED_OK },
    { ".",		do_shout,	POS_DEAD,	 3,  LOG_NORMAL, CMD_CHARMED_OK },
    { "music",		do_music,	POS_DEAD,	 3,  LOG_NORMAL, CMD_CHARMED_OK },
    { "gossip",		do_gossip,	POS_DEAD,	 3,  LOG_NORMAL, CMD_CHARMED_OK },
    { "motd",		do_motd,	POS_DEAD,        0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "wake",		do_wake,	POS_SLEEPING,	 0,  LOG_NORMAL, CMD_KEEP_HIDE },
    { "warcry",         do_warcry,      POS_FIGHTING,    0,  LOG_NORMAL	},
    { "unread",		do_unread,	POS_SLEEPING,    0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "yell",		do_yell,	POS_RESTING,	 0,  LOG_NORMAL, CMD_CHARMED_OK },

    { "wanted",		do_wanted,	POS_RESTING,	 0,  LOG_ALWAYS, CMD_NOORDER},
    { "judge",		do_judge,	POS_RESTING,	 0,  LOG_NORMAL, CMD_NOORDER | CMD_CHARMED_OK},

    /*
     * Configuration commands.
     */
    { "alia",		do_alia,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "alias",		do_alias,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_NOORDER },
    { "clear",		do_clear,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "cls",		do_clear,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "autolist",	do_autolist,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "autoassist",	do_autoassist,	POS_DEAD,        0,  LOG_NORMAL, CMD_KEEP_HIDE },
    { "autoexit",	do_autoexit,	POS_DEAD,        0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "autogold",	do_autogold,	POS_DEAD,        0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "autolook",	do_autolook,	POS_DEAD,        0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "autoloot",	do_autoloot,	POS_DEAD,        0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "autosac",	do_autosac,	POS_DEAD,        0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "autosplit",	do_autosplit,	POS_DEAD,        0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "description",	do_description,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "delet",		do_delet,	POS_DEAD,	 0,  LOG_ALWAYS, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_FROZEN_OK },
    { "delete",		do_delete,	POS_STANDING,	 0,  LOG_ALWAYS, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_FROZEN_OK },
    { "identify",	do_identify,	POS_STANDING,	 0,  LOG_NORMAL	},
    { "nofollow",	do_nofollow,	POS_DEAD,        0,  LOG_NORMAL, CMD_KEEP_HIDE },
    { "nosummon",	do_nosummon,	POS_DEAD,        0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_NOORDER },
    { "trust",		do_trust,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_NOORDER },
    { "toggle",		do_toggle,	POS_DEAD,        0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_NOORDER },
    { "outfit",		do_outfit,	POS_RESTING,	 0,  LOG_NORMAL, CMD_KEEP_HIDE },
    { "tick",		do_tick,	POS_DEAD,	ML,  LOG_ALWAYS, CMD_KEEP_HIDE },
    { "password",	do_password,	POS_DEAD,	 0,  LOG_NEVER, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_NOORDER },
    { "prompt",		do_prompt,	POS_DEAD,        0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_NOORDER },
    { "quest",          do_quest,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_NOORDER },
    { "qui",		do_qui,		POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_NOORDER | CMD_FROZEN_OK },
    { "quit",		do_quit,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_NOORDER | CMD_FROZEN_OK },
    { "quiet",		do_quiet,	POS_SLEEPING, 	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_NOORDER },
    { "scroll",		do_scroll,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_NOORDER },
    { "title",		do_title,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_NOORDER },
    { "twit",		do_twit,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_NOORDER },
    { "unalias",	do_unalias,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_NOORDER },
    { "wimpy",		do_wimpy,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_NOORDER },

    /*
     * Mob command interpreter (placed here for faster scan...)
     */
    { "mob",		do_mob,		POS_DEAD,	 0,  LOG_NEVER, CMD_NOORDER },

    /*
     * Miscellaneous commands.
     */
    { "endure",         do_endure,      POS_STANDING,    0,  LOG_NORMAL, CMD_KEEP_HIDE },
    { "enter", 		do_enter, 	POS_STANDING,	 0,  LOG_NORMAL	},
    { "follow",		do_follow,	POS_RESTING,	 0,  LOG_NORMAL, CMD_KEEP_HIDE },
    { "gain",		do_gain,	POS_STANDING,	 0,  LOG_NORMAL	},
    { "go",		do_enter,	POS_STANDING,	 0,  LOG_NORMAL, CMD_HIDDEN },
    { "fade",		do_fade,	POS_RESTING,	 0,  LOG_NORMAL, CMD_KEEP_HIDE },
    { "herbs",          do_herbs,       POS_STANDING,    0,  LOG_NORMAL	},
    { "hara",           do_hara,        POS_STANDING,    0,  LOG_NORMAL	},

    { "hide",		do_hide,	POS_RESTING,	 0,  LOG_NORMAL, CMD_KEEP_HIDE },
    { "hometown",	do_hometown,	POS_STANDING,	 0,  LOG_NORMAL },
    { "homepoint",	do_homepoint,	POS_STANDING,	 0,  LOG_NORMAL },
    { "human",          do_human,       POS_STANDING,    0,  LOG_NORMAL	},
    { "hunt",           do_hunt,        POS_STANDING,    0,  LOG_NORMAL	},
    { "leave", 		do_enter, 	POS_STANDING,	 0,  LOG_NORMAL	},
    { "rent",		do_rent,	POS_DEAD,	 0,  LOG_NORMAL, CMD_HIDDEN },
    { "save",		do_save,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE },
    { "sleep",		do_sleep,	POS_SLEEPING,	 0,  LOG_NORMAL, CMD_KEEP_HIDE },
    { "sneak",		do_sneak,	POS_STANDING,	 0,  LOG_NORMAL, CMD_KEEP_HIDE },
    { "split",		do_split,	POS_RESTING,	 0,  LOG_NORMAL	},
    { "steal",		do_steal,	POS_STANDING,	 0,  LOG_NORMAL, CMD_KEEP_HIDE },
    { "train",		do_train,	POS_RESTING,	 0,  LOG_NORMAL	},
    { "visible",	do_visible,	POS_SLEEPING,	 0,  LOG_NORMAL	},
    { "where",		do_where,	POS_RESTING,	 0,  LOG_NORMAL, CMD_KEEP_HIDE },
    { "item",		do_item,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE|CMD_CHARMED_OK },

    /*
     * Object manipulation commands.
     */
    { "brandish",	do_brandish,	POS_RESTING,	 0,  LOG_NORMAL	},
    { "butcher",        do_butcher,     POS_STANDING,    0,  LOG_NORMAL	},
    { "crucify",	do_crucify,	POS_STANDING,	 0,  LOG_NORMAL },
    { "close",		do_close,	POS_RESTING,	 0,  LOG_NORMAL	},
    { "detect",         do_detect_hidden,POS_RESTING,    0,  LOG_NORMAL	},
    { "drink",		do_drink,	POS_RESTING,	 0,  LOG_NORMAL	},
    { "drop",		do_drop,	POS_RESTING,	 0,  LOG_NORMAL	},
    { "eat",		do_eat,		POS_RESTING,	 0,  LOG_NORMAL	},
    { "enchant",	do_enchant, 	POS_RESTING,     0,  LOG_NORMAL	},
    { "envenom",	do_envenom,	POS_RESTING,	 0,  LOG_NORMAL, CMD_KEEP_HIDE },
    { "fill",		do_fill,	POS_RESTING,	 0,  LOG_NORMAL	},
    { "fly",		do_fly,		POS_FIGHTING,	 0,  LOG_NORMAL	},
    { "give",		do_give,	POS_RESTING,	 0,  LOG_NORMAL	},
    { "heal",		do_heal,	POS_RESTING,	 0,  LOG_NORMAL	}, 
    { "hold",		do_wear,	POS_RESTING,	 0,  LOG_NORMAL	},
    { "layhands",	do_layhands,	POS_RESTING,	 0,  LOG_NORMAL	},
    { "list",		do_list,	POS_RESTING,	 0,  LOG_NORMAL, CMD_KEEP_HIDE },
    { "lock",		do_lock,	POS_RESTING,	 0,  LOG_NORMAL	},
    { "lore",           do_lore,        POS_RESTING,     0,  LOG_NORMAL	},
    { "open",		do_open,	POS_RESTING,	 0,  LOG_NORMAL	},
    { "pick",		do_pick,	POS_RESTING,	 0,  LOG_NORMAL	},
    { "pour",		do_pour,	POS_RESTING,	 0,  LOG_NORMAL, CMD_KEEP_HIDE },
    { "put",		do_put,		POS_RESTING,	 0,  LOG_NORMAL	},
    { "quaff",		do_quaff,	POS_RESTING,	 0,  LOG_NORMAL	},
    { "recite",		do_recite,	POS_RESTING,	 0,  LOG_NORMAL	},
    { "remove",		do_remove,	POS_RESTING,	 0,  LOG_NORMAL	},
    { "request",        do_request,     POS_STANDING,    0,  LOG_NORMAL	},
    { "sell",		do_sell,	POS_RESTING,	 0,  LOG_NORMAL	},
    { "take",		do_get,		POS_RESTING,	 0,  LOG_NORMAL, CMD_KEEP_HIDE },
    { "sacrifice",	do_sacrifice,	POS_RESTING,	 0,  LOG_NORMAL	},
    { "junk",           do_sacrifice,   POS_RESTING,     0,  LOG_NORMAL, CMD_HIDDEN },
    { "trophy",         do_trophy,      POS_STANDING,    0,  LOG_NORMAL	},
    { "value",		do_value,	POS_RESTING,	 0,  LOG_NORMAL	},
    { "wear",		do_wear,	POS_RESTING,	 0,  LOG_NORMAL	},
    { "zap",		do_zap,		POS_RESTING,	 0,  LOG_NORMAL	},

    { "recall",		do_recall,	POS_FIGHTING,	 0,  LOG_NORMAL, CMD_NOORDER	},
    { "/",		do_recall,	POS_FIGHTING,	 0,  LOG_NORMAL, CMD_HIDDEN | CMD_NOORDER },

    /*
     * Combat commands.
     */
    { "ambush",         do_ambush,      POS_STANDING,    0,  LOG_NORMAL },
    { "assassinate",    do_assassinate, POS_STANDING,    0,  LOG_NORMAL },
    { "backstab",	do_backstab,	POS_STANDING,	 0,  LOG_NORMAL	},
    { "bash",		do_bash,	POS_FIGHTING,    0,  LOG_NORMAL },
    { "bashdoor",	do_bash_door,	POS_FIGHTING,    0,  LOG_NORMAL	},
    { "bs",		do_backstab,	POS_STANDING,	 0,  LOG_NORMAL, CMD_HIDDEN },
    { "bite",		do_vbite,	POS_STANDING,	 0,  LOG_NORMAL, CMD_HIDDEN },
    { "blindness",	do_blindness_dust,POS_FIGHTING,	 0,  LOG_ALWAYS	},
    { "touch",		do_vtouch,	POS_STANDING,	 0,  LOG_NORMAL, CMD_HIDDEN },
    { "berserk",	do_berserk,	POS_FIGHTING,	 0,  LOG_NORMAL	},
    { "bloodthirst",	do_bloodthirst,	POS_FIGHTING,	 0,  LOG_NORMAL	},
    { "blackjack",	do_blackjack,	POS_STANDING,	 0,  LOG_NORMAL },
    { "caltrops",       do_caltrops,    POS_FIGHTING,    0,  LOG_NORMAL	},
    { "explode",	do_explode, 	POS_FIGHTING,    0,  LOG_NORMAL	},
    { "camouflage",     do_camouflage,  POS_STANDING,    0,  LOG_NORMAL	},
    { "circle",         do_circle,      POS_FIGHTING,    0,  LOG_NORMAL	},
    { "cleave",         do_cleave,      POS_STANDING,    0,  LOG_NORMAL },

    { "dirt",		do_dirt,	POS_FIGHTING,	 0,  LOG_NORMAL },
    { "disarm",		do_disarm,	POS_FIGHTING,	 0,  LOG_NORMAL	},
    { "dishonor",	do_dishonor,	POS_FIGHTING,	 0,  LOG_NORMAL	},
    { "dismount",       do_dismount,    POS_STANDING,    0,  LOG_NORMAL	},
    { "flee",		do_flee,	POS_FIGHTING,	 0,  LOG_NORMAL	},
    { "guard",          do_guard,       POS_STANDING,    0,  LOG_NORMAL	},

    { "kick",		do_kick,	POS_FIGHTING,	 0,  LOG_NORMAL	},
    { "lioncall",       do_lion_call,   POS_FIGHTING,    0,  LOG_NORMAL	},
    { "make",           do_make,        POS_STANDING,    0,  LOG_NORMAL	},
    { "mount",          do_mount,       POS_STANDING,    0,  LOG_NORMAL	},
    { "murde",		do_murde,	POS_FIGHTING,	 0,  LOG_NORMAL, CMD_HIDDEN },
    { "murder",		do_murder,	POS_FIGHTING,	 0,  LOG_ALWAYS },
    { "nerve",          do_nerve,       POS_FIGHTING,    0,  LOG_NORMAL	},
    { "poison",		do_poison_smoke,POS_FIGHTING,	 0,  LOG_ALWAYS	},
    { "rescue",		do_rescue,	POS_FIGHTING,	 0,  LOG_NORMAL	},
    { "resistance",     do_resistance,  POS_FIGHTING,    0,  LOG_NORMAL	},
    { "truesight",      do_truesight,   POS_FIGHTING,    0,  LOG_NORMAL	},
    { "thumbling",	do_thumbling,	POS_FIGHTING,	 0,  LOG_NORMAL, CMD_KEEP_HIDE },
    { "forest",		do_forest,	POS_FIGHTING,	 0,  LOG_NORMAL },
    { "shield",		do_shield,	POS_FIGHTING,	 0,  LOG_NORMAL	},
    { "spellbane",      do_spellbane,   POS_FIGHTING,    0,  LOG_NORMAL	},
    { "strangle",       do_strangle,    POS_STANDING,    0,  LOG_NORMAL },
    { "surrender",	do_surrender,	POS_FIGHTING,    0,  LOG_NORMAL	},
    { "tame",           do_tame,        POS_FIGHTING,    0,  LOG_NORMAL	},
    { "throw",          do_throw,       POS_FIGHTING,    0,  LOG_NORMAL	},
    { "tiger",		do_tiger,	POS_FIGHTING,	 0,  LOG_NORMAL	},
    { "trip",		do_trip,	POS_FIGHTING,    0,  LOG_NORMAL },
    { "target",		do_target,	POS_FIGHTING,    0,  LOG_NORMAL	},
    { "vampire",	do_vampire,	POS_FIGHTING,    0,  LOG_NORMAL	},
    { "vanish",		do_vanish,	POS_FIGHTING,    0,  LOG_NORMAL	},
    { "weapon",		do_weapon,	POS_FIGHTING,	 0,  LOG_NORMAL	},
    { "blink",		do_blink,	POS_FIGHTING,    0,  LOG_NORMAL, CMD_KEEP_HIDE },
    { "knife", 		do_knife,	POS_STANDING,	 0,  LOG_NORMAL },
    { "mark",		do_mark,	POS_STANDING,	 0,  LOG_NORMAL	},


    /*
     * Immortal commands.
     */
    { "advance",	do_advance,	POS_DEAD,	ML,  LOG_ALWAYS	}, 
    { "set",		do_set,		POS_DEAD,	L1,  LOG_ALWAYS	},
    { "dump",		do_dump,	POS_DEAD,	ML,  LOG_ALWAYS	},
    { "rename",		do_rename,	POS_DEAD,	ML,  LOG_ALWAYS	},
    { "violate",	do_violate,	POS_DEAD,	ML,  LOG_ALWAYS	},
    { "track",          do_track,       POS_STANDING,    0,  LOG_NORMAL	},

    { "allow",		do_allow,	POS_DEAD,	L2,  LOG_ALWAYS	},
    { "ban",		do_ban,		POS_DEAD,	L2,  LOG_ALWAYS	},
    { "deny",		do_deny,	POS_DEAD,	L1,  LOG_ALWAYS	},
    { "disconnect",	do_disconnect,	POS_DEAD,	L3,  LOG_ALWAYS	},
    { "freeze",		do_freeze,	POS_DEAD,	L4,  LOG_ALWAYS	},
    { "permban",	do_permban,	POS_DEAD,	L1,  LOG_ALWAYS	},
    { "protect",	do_protect,	POS_DEAD,	L1,  LOG_ALWAYS	},
    { "reboo",		do_reboo,	POS_DEAD,	L1,  LOG_NEVER, CMD_HIDDEN	},
    { "reboot",		do_reboot,	POS_DEAD,	L1,  LOG_ALWAYS	},
    { "smite",		do_smite,	POS_DEAD,	L2,  LOG_ALWAYS	},
    { "limited",	do_limited,	POS_DEAD,	L2,  LOG_NEVER	},
    { "popularity",	do_popularity,	POS_DEAD,	L2,  LOG_ALWAYS	},
    { "shutdow",	do_shutdow,	POS_DEAD,	L1,  LOG_NEVER, CMD_HIDDEN	},
    { "shutdown",	do_shutdown,	POS_DEAD,	L1,  LOG_ALWAYS	},
    { "wizlock",	do_wizlock,	POS_DEAD,	L2,  LOG_ALWAYS	},
    { "affrooms",	do_affrooms,	POS_DEAD,	L5,  LOG_ALWAYS	},
    { "force",		do_force,	POS_DEAD,	L5,  LOG_ALWAYS	},
    { "load",		do_load,	POS_DEAD,	L4,  LOG_ALWAYS	},
    { "newlock",	do_newlock,	POS_DEAD,	L4,  LOG_ALWAYS	},
    { "noaffect",	do_noaffect,	POS_DEAD,	L5,  LOG_ALWAYS	},
    { "nochannels",	do_nochannels,	POS_DEAD,	L5,  LOG_ALWAYS	},
    { "notitle",	do_notitle,	POS_DEAD,	L5,  LOG_ALWAYS	},
    { "noemote",	do_noemote,	POS_DEAD,	L5,  LOG_ALWAYS	},
    { "noshout",	do_noshout,	POS_DEAD,	L5,  LOG_ALWAYS	},
    { "notell",		do_notell,	POS_DEAD,	L5,  LOG_ALWAYS	},
    { "pecho",		do_pecho,	POS_DEAD,	L4,  LOG_ALWAYS	}, 
    { "purge",		do_purge,	POS_DEAD,	L4,  LOG_ALWAYS	},
    { "restore",	do_restore,	POS_DEAD,	L4,  LOG_ALWAYS	},
    { "sla",		do_sla,		POS_DEAD,	L3,  LOG_NEVER	},
    { "slay",		do_slay,	POS_DEAD,	L3,  LOG_ALWAYS	},
    { "teleport",	do_transfer,    POS_DEAD,	L5,  LOG_ALWAYS	},	
    { "transfer",	do_transfer,	POS_DEAD,	L5,  LOG_ALWAYS	},
    { "poofin",		do_bamfin,	POS_DEAD,	L8,  LOG_NORMAL	},
    { "poofout",	do_bamfout,	POS_DEAD,	L8,  LOG_NORMAL	},
    { "gecho",		do_echo,	POS_DEAD,	L4,  LOG_ALWAYS	},
    { ">",		do_echo,	POS_DEAD,	L4,  LOG_ALWAYS	},
    { "holylight",	do_holylight,	POS_DEAD,	IM,  LOG_NORMAL	},
    { "incognito",	do_incognito,	POS_DEAD,	IM,  LOG_NORMAL	},
    { "invis",		do_invis,	POS_DEAD,	IM,  LOG_NORMAL	},
    { "log",		do_log,		POS_DEAD,	L1,  LOG_ALWAYS	},
    { "memory",		do_memory,	POS_DEAD,	IM,  LOG_NEVER	},
    { "mwhere",		do_mwhere,	POS_DEAD,	IM,  LOG_NORMAL	},
    { "owhere",		do_owhere,	POS_DEAD,	IM,  LOG_NORMAL	},
    { "peace",		do_peace,	POS_DEAD,	L5,  LOG_NORMAL	},
    { "penalty",	do_penalty,	POS_DEAD,	L7,  LOG_NORMAL	},
    { "penalties",	do_penalty,	POS_DEAD,	L7,  LOG_NORMAL	},
    { "echo",		do_recho,	POS_DEAD,	L6,  LOG_ALWAYS	},
    { "return",         do_return,      POS_DEAD,       0,  LOG_NORMAL, CMD_CHARMED_OK | CMD_HIDDEN },
    { "snoop",		do_snoop,	POS_DEAD,	ML,  LOG_ALWAYS	},
    { "stat",		do_stat,	POS_DEAD,	IM,  LOG_NORMAL	},
    { "string",		do_string,	POS_DEAD,	L5,  LOG_ALWAYS	},
    { "switch",		do_switch,	POS_DEAD,	L6,  LOG_ALWAYS	},
    { "wizinvis",	do_invis,	POS_DEAD,	IM,  LOG_NORMAL	},
    { "vnum",		do_vnum,	POS_DEAD,	L4,  LOG_NORMAL	},
    { "zecho",		do_zecho,	POS_DEAD,	L4,  LOG_ALWAYS	},
    { "clone",		do_clone,	POS_DEAD,	L5,  LOG_ALWAYS	},
    { "wiznet",		do_wiznet,	POS_DEAD,	IM,  LOG_NORMAL	},
    { "immtalk",	do_immtalk,	POS_DEAD,	IM,  LOG_NORMAL	},
    { "imotd",          do_imotd,       POS_DEAD,       IM,  LOG_NORMAL	},
    { ":",		do_immtalk,	POS_DEAD,	IM,  LOG_NORMAL	},
    { "smote",		do_smote,	POS_DEAD,	IM,  LOG_NORMAL	},
    { "prefi",		do_prefi,	POS_DEAD,	IM,  LOG_NORMAL, CMD_HIDDEN },
    { "prefix",		do_prefix,	POS_DEAD,	IM,  LOG_NORMAL	},
    { "objlist",	do_objlist,	POS_DEAD,	ML,  LOG_NORMAL	},

    { "settraps",	do_settraps,	POS_STANDING,	 0,  LOG_NORMAL	},
    { "slook",		do_slook,	POS_SLEEPING,	 0,  LOG_NORMAL, CMD_KEEP_HIDE },
    { "learn",		do_learn,	POS_STANDING,	 0,  LOG_NORMAL	},
    { "teach",		do_teach,	POS_STANDING,	 0,  LOG_NORMAL	},
    { "camp",		do_camp,  	POS_STANDING,    0,  LOG_NORMAL	},
    { "tail",		do_tail,	POS_FIGHTING,    0,  LOG_NORMAL	},
    { "push",		do_push,	POS_STANDING,	 0,  LOG_NORMAL	},
    { "demand",         do_demand,      POS_STANDING,    0,  LOG_NORMAL	},
    { "bandage",        do_bandage,     POS_FIGHTING,    0,  LOG_NORMAL	},
    { "shoot",          do_shoot,       POS_FIGHTING,    0,  LOG_NORMAL	},
    { "charge",		do_charge,	POS_STANDING,	 0,  LOG_NORMAL },

    { "find",		do_find,	POS_DEAD,	ML,  LOG_ALWAYS	},
    { "score",		do_score,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "katana",		do_katana, 	POS_STANDING,    0,  LOG_NORMAL	},
    { "control",       	do_control,	POS_STANDING,    0,  LOG_NORMAL	},
    { "ititle",		do_ititle,	POS_DEAD,	IM,  LOG_NORMAL },
    { "sense",          do_sense,       POS_RESTING,     0,  LOG_NORMAL	},

    { "mpstat",		do_mpstat,	POS_DEAD,	IM,  LOG_NEVER	},


    { "msgstat",	do_msgstat,	POS_DEAD,	IM,  LOG_NEVER	},
    { "strstat",	do_strstat,	POS_DEAD,	IM,  LOG_NEVER	},

    { "grant",		do_grant,	POS_DEAD,	ML,	LOG_ALWAYS },
    { "disable",	do_disable,	POS_DEAD,	ML,	LOG_ALWAYS },
    { "enable",		do_enable,	POS_DEAD,	ML,	LOG_ALWAYS },

    /*
     * OLC
     */
    { "edit",		do_edit,	POS_DEAD,   IM,  LOG_ALWAYS	},
    { "create",		do_create,	POS_DEAD,   IM,  LOG_ALWAYS	},
    { "asave",          do_asave,	POS_DEAD,   IM,  LOG_ALWAYS	},
    { "alist",		do_alist,	POS_DEAD,   IM,  LOG_NEVER	},
    { "ashow",		do_ashow,	POS_DEAD,   IM,  LOG_NEVER	},
    { "resets",		do_resets,	POS_DEAD,   IM,  LOG_NORMAL	},

    /*
     * End of list.
     */
     { NULL }
};


void interpret(CHAR_DATA *ch, const char *argument)
{
	interpret_raw(ch, argument, FALSE);
}

/*
 * The main entry point for executing commands.
 * Can be recursively called from 'at', 'order', 'force'.
 */
void interpret_raw(CHAR_DATA *ch, const char *argument, bool is_order)
{
	char command[MAX_INPUT_LENGTH];
	const char *logline;
	cmd_t *cmd;
	bool found;
	social_t *soc = NULL;
	int min_pos;
	flag64_t cmd_flags;

	/*
	 * Strip leading spaces.
	 */
	while (isspace(*argument))
		argument++;

	if (argument[0] == '\0')
		return;

	logline = argument;

	/*
	 * Grab the command word.
	 * Special parsing so ' can be a command,
	 * also no spaces needed after punctuation.
	 */

#ifdef IMMORTALS_LOGS
 	if (IS_IMMORTAL(ch)) {
		if ((imm_log = dfopen(GODS_PATH, IMMLOG_FILE, "a+"))) {
			fprintf(imm_log, "%s [%s] %s\n",
				strtime(time(NULL)), ch->name, logline);
			fprintf(imm_log, buf);
			fclose(imm_log);
		}
	}
#endif

	if (!isalpha(argument[0]) && !isdigit(argument[0])) {
		command[0] = argument[0];
		command[1] = '\0';
		argument++;
		while (isspace(*argument))
			argument++;
	} else
		argument = one_argument(argument, command, sizeof(command));

	/*
	 * Look for command in command table.
	 */
	found = FALSE;
	for (cmd = cmd_table; cmd->name; cmd++) {
		if (str_prefix(command, cmd->name))
			continue;

		/*
		 * Implement freeze command.
		 */
		if (!IS_NPC(ch)
		&&  IS_SET(ch->plr_flags, PLR_FREEZE)
		&&  !IS_SET(cmd->flags, CMD_FROZEN_OK)) {
			char_puts("You're totally frozen!\n", ch);
			continue;
		}

		if (IS_SET(cmd->flags, CMD_DISABLED)) {
			char_puts("Sorry, this command is temporarily disabled.\n", ch);
			return;
		}

		if (cmd->level >= LEVEL_IMMORTAL) {
			if (IS_NPC(ch))
				continue;

			if (ch->level < IMPLEMENTOR
			&&  !is_name(cmd->name, ch->pcdata->granted))
				continue;
		}
		else if (cmd->level > ch->level)
			continue;

		if (is_order) {
			if (IS_SET(cmd->flags, CMD_NOORDER)
			||  cmd->level >= LEVEL_IMMORTAL)
				return;
		}
		else {
			if (IS_AFFECTED(ch, AFF_CHARM)
			&&  !IS_SET(cmd->flags, CMD_CHARMED_OK)
			&&  cmd->level < LEVEL_IMMORTAL 
			&&  !IS_IMMORTAL(ch)) {
				char_puts("First ask your beloved master!\n",
					  ch);
				return;
			}
		}

		if (IS_AFFECTED(ch, AFF_STUN) 
		&&  !(cmd->flags & CMD_KEEP_HIDE)) {
			char_puts("You are STUNNED to do that.\n", ch);
			return;
		}

		found = TRUE;
		break;
	}

	/*
	 * Log and snoop.
	 */
	if (cmd->log == LOG_NEVER)
		logline = str_empty;

	if (((!IS_NPC(ch) && IS_SET(ch->plr_flags, PLR_LOG))
	||   fLogAll
	||   cmd->log == LOG_ALWAYS) && logline[0] != '\0' 
	&&   logline[0] != '\n') {
		log_printf("Log %s: %s", ch->name, logline);
		wiznet("Log $N: $t", ch, logline, WIZ_SECURE, 0, ch->level);
	}

	if (ch->desc && ch->desc->snoop_by) {
		write_to_buffer(ch->desc->snoop_by, "# ", 2);
		write_to_buffer(ch->desc->snoop_by, logline, 0);
		write_to_buffer(ch->desc->snoop_by, "\n\r", 2);
	}

	if (!found) {
		/*
		 * Look for command in socials table.
		 */
		if ((soc = social_lookup(command, str_prefix)) == NULL) {
			char_puts("Huh?\n", ch);
			return;
		}

		if (!IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE)) {
			char_puts("You are anti-social!\n", ch);
			return;
		}

		min_pos = soc->min_pos;
		cmd_flags = 0;
	}
	else {
		min_pos = cmd->position;
		cmd_flags = cmd->flags;
	}

	if (!IS_NPC(ch)) {
		/* Come out of hiding for most commands */
		if (IS_AFFECTED(ch, AFF_HIDE | AFF_FADE)
		&&  !IS_SET(cmd_flags, CMD_KEEP_HIDE)) {
			REMOVE_BIT(ch->affected_by, AFF_HIDE | AFF_FADE);
			act_puts("You step out of shadows.",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
			act("$n steps out of shadows.",
			    ch, NULL, NULL, TO_ROOM);
		}

		if (IS_AFFECTED(ch, AFF_IMP_INVIS)
		&&  min_pos == POS_FIGHTING) {
			affect_bit_strip(ch, TO_AFFECTS, AFF_IMP_INVIS);
			act_puts("You fade into existence.",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
			act("$n fades into existence.",
			    ch, NULL, NULL, TO_ROOM);
		}
	}

	/*
	 * Character not in position for command?
	 */
	if (ch->position < min_pos) {
		switch(ch->position) {
			case POS_DEAD:
				char_puts("Lie still; You are DEAD.\n", ch);
				break;

			case POS_MORTAL:
			case POS_INCAP:
				char_puts("You are hurt far too bad "
					  "for that.\n", ch);
				break;

			case POS_STUNNED:
				char_puts("You are too stunned to do that.\n",
					  ch);
				break;

			case POS_SLEEPING:
				char_puts("In your dreams, or what?\n", ch);
				break;

			case POS_RESTING:
				char_puts("Nah... You feel too relaxed...\n",
					  ch);
				break;

			case POS_SITTING:
				char_puts("Better stand up first.\n", ch);
				break;

			case POS_FIGHTING:
				char_puts("No way! You are still fighting!\n",
					  ch);
				break;

		}
		return;
	}

	if (soc) {
		interpret_social(soc, ch, argument);
		return;
	}

	/*
	 * Dispatch the command.
	 */
	cmd->do_fun(ch, argument);

	tail_chain();
}

void interpret_social(social_t *soc, CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	ROOM_INDEX_DATA *victim_room;

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		act(soc->noarg_char, ch, NULL, NULL, TO_CHAR);
		act(soc->noarg_room,
		    ch, NULL, NULL, TO_ROOM | ACT_TOBUF | ACT_NOTWIT);
		return;
	}

	if ((victim = get_char_world(ch, arg)) == NULL
	||  (IS_NPC(victim) && victim->in_room != ch->in_room)) {
		act(soc->notfound_char, ch, NULL, NULL, TO_CHAR);
		return;
	}

	if (victim == ch) {
		act(soc->self_char, ch, NULL, victim, TO_CHAR);
		act(soc->self_room,
		    ch, NULL, victim, TO_ROOM | ACT_TOBUF | ACT_NOTWIT);
		return;
	}

	victim_room = victim->in_room;
	victim->in_room = ch->in_room;

	act(soc->found_char, ch, NULL, victim, TO_CHAR);
	act(soc->found_vict,
	    ch, NULL, victim, TO_VICT | ACT_TOBUF | ACT_NOTWIT);
	act(soc->found_notvict,
	    ch, NULL, victim, TO_NOTVICT | ACT_TOBUF | ACT_NOTWIT);

	victim->in_room = victim_room;

	if (!IS_NPC(ch) && IS_NPC(victim) 
	&&  !IS_AFFECTED(victim, AFF_CHARM)
	&&  IS_AWAKE(victim) && !victim->desc) {
		switch (number_bits(4)) {
			case 0:

			case 1: case 2: case 3: case 4:
			case 5: case 6: case 7: case 8:
				act(soc->found_char,
				    victim, NULL, ch, TO_CHAR);
				act(soc->found_vict,
				    victim, NULL, ch, TO_VICT | ACT_TOBUF);
				act(soc->found_notvict,
				    victim, NULL, ch,
				    TO_NOTVICT | ACT_TOBUF | ACT_NOTWIT);
				break;

			case 9: case 10: case 11: case 12:
				act("$n slaps $N.", victim, NULL, ch, 
				    TO_NOTVICT | ACT_TOBUF | ACT_NOTWIT);
				act("You slap $N.", victim, NULL, ch, TO_CHAR);
				act("$n slaps you.", victim, NULL, ch, 
				    TO_VICT | ACT_TOBUF);
				break;
		}
	}
}

/*
 * Return true if an argument is completely numeric.
 */
bool is_number(const char *argument)
{
	if (IS_NULLSTR(argument))
    		return FALSE;
 
	if (*argument == '+' || *argument == '-')
    		argument++;
 
	for (; *argument != '\0'; argument++) {
    		if (!isdigit(*argument))
        		return FALSE;
	}
 
	return TRUE;
}

static uint x_argument(const char *argument, char c, char *arg, size_t len)
{
	char *p;
	char *q;
	int number;
    
	if (IS_NULLSTR(argument)) {
		arg[0] = '\0';
		return 0;
	}

	p = strchr(argument, c);
	if (p == NULL) {
		strnzcpy(arg, len, argument);
		return 1;
	}

	number = strtoul(argument, &q, 0);
	if (q == p)
		argument = p+1;
	else 
		number = 1;
	strnzcpy(arg, len, argument);
	return number;
}

/*
 * Given a string like 14.foo, return 14 and 'foo'
 */
uint number_argument(const char *argument, char *arg, size_t len)
{
	return x_argument(argument, '.', arg, len);
}

/* 
 * Given a string like 14*foo, return 14 and 'foo'
 */
uint mult_argument(const char *argument, char *arg, size_t len)
{
	return x_argument(argument, '*', arg, len);
}

/*
 * Pick off one argument from a string and return the rest.
 * Understands quotes.
 */
const char *one_argument(const char *argument, char *arg_first, size_t len)
{
	return first_arg(argument, arg_first, len, TRUE);
}

/*****************************************************************************
 Name:		first_arg
 Purpose:	Pick off one argument from a string and return the rest.
 		Understands quotes, if fCase then arg_first will be lowercased
 Called by:	string_add(string.c)
 ****************************************************************************/
const char *first_arg(const char *argument, char *arg_first, size_t len,
		      bool fCase)
{
	char *q;
	char cEnd = '\0';

	if (IS_NULLSTR(argument)) {
		arg_first[0] = '\0';
		return argument;
	}

/* skip leading spaces */
	while (isspace(*argument))
		argument++;

/* check quotes */
	if (*argument == '\'' || *argument == '"')
        	cEnd = *argument++;

	for (q = arg_first; *argument && q - arg_first + 1 < len; argument++) {
		if ((!cEnd && isspace(*argument)) || *argument == cEnd) {
			argument++;
			break;
		}
		*q++ = fCase ? LOWER(*argument) : *argument;
	}
	*q = '\0';

	while (isspace(*argument))
		argument++;

	return argument;
}

/*
 * Contributed by Alander.
 */
void do_commands(CHAR_DATA *ch, const char *argument)
{
	cmd_t *cmd;
	int col;
 
	col = 0;
	for (cmd = cmd_table; cmd->name; cmd++) {
		if (cmd->level < LEVEL_HERO
		&&  cmd->level <= ch->level 
		&&  !IS_SET(cmd->flags, CMD_HIDDEN)) {
			char_printf(ch, "%-12s", cmd->name);
			if (++col % 6 == 0)
				char_puts("\n", ch);
		}
	}
 
	if (col % 6 != 0)
		char_puts("\n", ch);
}

void do_wizhelp(CHAR_DATA *ch, const char *argument)
{
	cmd_t *cmd;
	int col;
 
	if (IS_NPC(ch)) {
		char_puts("Huh?\n", ch);
		return;
	}

	col = 0;
	for (cmd = cmd_table; cmd->name; cmd++) {
		if (cmd->level < LEVEL_IMMORTAL)
			continue;

		if (ch->level < IMPLEMENTOR
		&&  !is_name(cmd->name, ch->pcdata->granted))
			continue;

		char_printf(ch, "%-12s", cmd->name);
		if (++col % 6 == 0)
			char_puts("\n", ch);
	}
 
	if (col % 6 != 0)
		char_puts("\n", ch);
}

/*********** alias.c **************/

/* does aliasing and other fun stuff */
void substitute_alias(DESCRIPTOR_DATA *d, const char *argument)
{
    CHAR_DATA *ch;
    char buf[MAX_STRING_LENGTH],prefix[MAX_INPUT_LENGTH],name[MAX_INPUT_LENGTH];
    const char *point;
    int alias;

    ch = d->original ? d->original : d->character;

    /* check for prefix */
    if (ch->prefix[0] != '\0' && str_prefix("prefix",argument))
    {
	if (strlen(ch->prefix) + strlen(argument) > MAX_INPUT_LENGTH)
	    char_puts("Line to long, prefix not processed.\n",ch);
	else
	{
	    snprintf(prefix, sizeof(prefix), "%s %s",ch->prefix,argument);
	    argument = prefix;
	}
    }

    if (IS_NPC(ch) || ch->pcdata->alias[0] == NULL
    ||	!str_prefix("alias",argument) || !str_prefix("una",argument) 
    ||  !str_prefix("prefix",argument)) {
	interpret(d->character, argument);
	return;
    }

    strnzcpy(buf, sizeof(buf), argument);

    for (alias = 0; alias < MAX_ALIAS; alias++)	 /* go through the aliases */
    {
	if (ch->pcdata->alias[alias] == NULL)
	    break;

	if (!str_prefix(ch->pcdata->alias[alias],argument))
	{
	    point = one_argument(argument, name, sizeof(name));
	    if (!strcmp(ch->pcdata->alias[alias],name)) {
		snprintf(buf, sizeof(buf), "%s %s",
			 ch->pcdata->alias_sub[alias], point);
		break;
	    }
	    if (strlen(buf) > MAX_INPUT_LENGTH)
	    {
		char_puts("Alias substitution too long. Truncated.\n",ch);
		buf[MAX_INPUT_LENGTH -1] = '\0';
	    }
	}
    }
    interpret(d->character, buf);
}

void do_alia(CHAR_DATA *ch, const char *argument)
{
    char_puts("I'm sorry, alias must be entered in full.\n",ch);
    return;
}

void do_alias(CHAR_DATA *ch, const char *argument)
{
    CHAR_DATA *rch;
    char arg[MAX_INPUT_LENGTH];
    int pos;

    if (ch->desc == NULL)
	rch = ch;
    else
	rch = ch->desc->original ? ch->desc->original : ch;

    if (IS_NPC(rch))
	return;

    argument = one_argument(argument, arg, sizeof(arg));
    

    if (arg[0] == '\0')
    {
	if (rch->pcdata->alias[0] == NULL)
	{
	    char_puts("You have no aliases defined.\n",ch);
	    return;
	}
	char_puts("Your current aliases are:\n",ch);

	for (pos = 0; pos < MAX_ALIAS; pos++)
	{
	    if (rch->pcdata->alias[pos] == NULL
	    ||	rch->pcdata->alias_sub[pos] == NULL)
		break;

	    char_printf(ch,"    %s:  %s\n",rch->pcdata->alias[pos],
		    rch->pcdata->alias_sub[pos]);
	}
	return;
    }

    if (!str_prefix("una",arg) || !str_cmp("alias",arg))
    {
	char_puts("Sorry, that word is reserved.\n",ch);
	return;
    }

    if (argument[0] == '\0')
    {
	for (pos = 0; pos < MAX_ALIAS; pos++)
	{
	    if (rch->pcdata->alias[pos] == NULL
	    ||	rch->pcdata->alias_sub[pos] == NULL)
		break;

	    if (!str_cmp(arg,rch->pcdata->alias[pos])) {
		char_printf(ch, "%s aliases to '%s'.\n",
			    rch->pcdata->alias[pos],
			    rch->pcdata->alias_sub[pos]);
		return;
	    }
	}

	char_puts("That alias is not defined.\n",ch);
	return;
    }

    if (!str_prefix(argument,"delete") || !str_prefix(argument,"prefix"))
    {
	char_puts("That shall not be done!\n",ch);
	return;
    }

    for (pos = 0; pos < MAX_ALIAS; pos++)
    {
	if (rch->pcdata->alias[pos] == NULL)
	    break;

	if (!str_cmp(arg,rch->pcdata->alias[pos])) /* redefine an alias */
	{
	    free_string(rch->pcdata->alias_sub[pos]);
	    rch->pcdata->alias_sub[pos] = str_dup(argument);
	    char_printf(ch,"%s is now realiased to '%s'.\n",arg,argument);
	    return;
	}
     }

     if (pos >= MAX_ALIAS)
     {
	char_puts("Sorry, you have reached the alias limit.\n",ch);
	return;
     }
  
     /* make a new alias */
     rch->pcdata->alias[pos]		= str_dup(arg);
     rch->pcdata->alias_sub[pos]	= str_dup(argument);
     char_printf(ch,"%s is now aliased to '%s'.\n",arg,argument);
}


void do_unalias(CHAR_DATA *ch, const char *argument)
{
    CHAR_DATA *rch;
    char arg[MAX_INPUT_LENGTH];
    int pos;
    bool found = FALSE;
 
    if (ch->desc == NULL)
	rch = ch;
    else
	rch = ch->desc->original ? ch->desc->original : ch;
 
    if (IS_NPC(rch))
	return;
 
    argument = one_argument(argument, arg, sizeof(arg));

    if (arg == '\0')
    {
	char_puts("Unalias what?\n",ch);
	return;
    }

    for (pos = 0; pos < MAX_ALIAS; pos++)
    {
	if (rch->pcdata->alias[pos] == NULL)
	    break;

	if (found)
	{
	    rch->pcdata->alias[pos-1]		= rch->pcdata->alias[pos];
	    rch->pcdata->alias_sub[pos-1]	= rch->pcdata->alias_sub[pos];
	    rch->pcdata->alias[pos]		= NULL;
	    rch->pcdata->alias_sub[pos]		= NULL;
	    continue;
	}

	if(!strcmp(arg,rch->pcdata->alias[pos]))
	{
	    char_puts("Alias removed.\n",ch);
	    free_string(rch->pcdata->alias[pos]);
	    free_string(rch->pcdata->alias_sub[pos]);
	    rch->pcdata->alias[pos] = NULL;
	    rch->pcdata->alias_sub[pos] = NULL;
	    found = TRUE;
	}
    }

    if (!found)
	char_puts("No alias of that name to remove.\n",ch);
}
