/*
 * $Id: interp.c,v 1.79 1998-10-11 16:52:45 fjoe Exp $
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

#include "resource.h"

#undef IMMORTALS_LOGS

bool	check_social	(CHAR_DATA *ch, char *command,const char *argument);

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
const	struct	cmd_type	cmd_table	[] =
{
    /*
     * Common movement commands.
     */
    { "north",		do_north,	POS_STANDING,    0,  LOG_NEVER, 0, CMD_KEEP_HIDE },
    { "east",		do_east,	POS_STANDING,	 0,  LOG_NEVER, 0, CMD_KEEP_HIDE },
    { "south",		do_south,	POS_STANDING,	 0,  LOG_NEVER, 0, CMD_KEEP_HIDE },
    { "west",		do_west,	POS_STANDING,	 0,  LOG_NEVER, 0, CMD_KEEP_HIDE },
    { "up",		do_up,		POS_STANDING,	 0,  LOG_NEVER, 0, CMD_KEEP_HIDE },
    { "down",		do_down,	POS_STANDING,	 0,  LOG_NEVER, 0, CMD_KEEP_HIDE },

    /*
     * Common other commands.
     * Placed here so one and two letter abbreviations work.
     */
    { "at",             do_at,          POS_DEAD,       L6,  LOG_NORMAL, 1, CMD_KEEP_HIDE  },
    { "cast",		do_cast,	POS_FIGHTING,	 0,  LOG_NORMAL, 1, CMD_NOORDER },
    { "auction",        do_auction,     POS_SLEEPING,    0,  LOG_NORMAL, 1, 0  },
    { "buy",		do_buy,		POS_RESTING,	 0,  LOG_NORMAL, 1, 0 },
    { "channels",       do_channels,    POS_DEAD,        0,  LOG_NORMAL, 1, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "dual",		do_second_wield,POS_RESTING,	 0,  LOG_NORMAL, 1, 0 },
    { "exits",		do_exits,	POS_RESTING,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE },
    { "estimate",	do_estimate,	POS_RESTING,	 0,  LOG_NORMAL, 1, 0 },
    { "get",		do_get,		POS_RESTING,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE },

    { "goto",           do_goto,        POS_DEAD,       L8,  LOG_NORMAL, 1, CMD_KEEP_HIDE },
    { "glist",          do_glist,       POS_DEAD,        0,  LOG_NEVER,  1, 0},
    { "group",          do_group,       POS_SLEEPING,    0,  LOG_NORMAL, 1, CMD_KEEP_HIDE },
    { "hit",		do_kill,	POS_FIGHTING,	 0,  LOG_NORMAL, 0, 0 },
    { "inventory",	do_inventory,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE },
    { "kill",		do_kill,	POS_FIGHTING,	 0,  LOG_NORMAL, 1, 0 },
    { "look",		do_look,	POS_RESTING,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE },
    { "order",		do_order,	POS_RESTING,	 0,  LOG_NORMAL, 1,0},
    { "practice",       do_practice,	POS_SLEEPING,    0,  LOG_NORMAL, 1, CMD_KEEP_HIDE },
    { "rest",		do_rest,	POS_SLEEPING,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE },
    { "repair",		do_repair,	POS_SLEEPING,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE },
    { "second",		do_second_wield,POS_RESTING,	 0,  LOG_NORMAL, 1, 0 },
    { "sit",		do_sit,		POS_SLEEPING,    0,  LOG_NORMAL, 1, 0},
    { "smithing",	do_smithing,	POS_RESTING,	 0,  LOG_NORMAL, 1,0},
    { "sockets",        do_sockets,	POS_DEAD,       L4,  LOG_NORMAL, 1, CMD_KEEP_HIDE},
    { "stand",		do_stand,	POS_SLEEPING,	 0,  LOG_NORMAL, 1, 0},
    { "tell",		do_tell,	POS_SLEEPING,	 0,  LOG_NORMAL, 1, CMD_CHARMED_OK },
    { "unlock",         do_unlock,      POS_RESTING,     0,  LOG_NORMAL, 1, 0},
    { "wield",		do_wear,	POS_RESTING,	 0,  LOG_NORMAL, 1, 0 },
    { "wizhelp",	do_wizhelp,	POS_DEAD,	IM,  LOG_NORMAL, 1, CMD_KEEP_HIDE },
    { "language",	do_lang,	POS_DEAD,	0,   LOG_NORMAL, 1, 0},

    /*
     * Informational commands.
     */
    { "affects",	do_affects,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE },
    { "afk",		do_afk,		POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE },
    { "areas",		do_areas,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE },
    { "balance",	do_balance,	POS_STANDING,	 0,  LOG_NORMAL, 1, 0 },
    { "bug",		do_bug,		POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE },
    { "changes",	do_changes,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE },
    { "commands",	do_commands,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE },
    { "compare",	do_compare,	POS_RESTING,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE },
    { "consider",	do_consider,	POS_RESTING,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE },
    { "concentrate",	do_concentrate,	POS_STANDING,	 0,  LOG_NORMAL, 1, 0 },
    { "count",		do_count,	POS_SLEEPING,	HE,  LOG_NORMAL, 1, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "credits",	do_credits,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "date",		do_date,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "deposit",	do_deposit,	POS_STANDING,	 0,  LOG_NORMAL, 1, 0 },
    { "equipment",	do_equipment,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "escape",		do_escape,	POS_FIGHTING,	 0,  LOG_NORMAL,1,0 },
    { "examine",	do_examine,	POS_RESTING,	 0,  LOG_NORMAL, 1, CMD_CHARMED_OK }, 
    { "help",		do_help,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "idea",		do_idea,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "news",		do_news,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "raffects",	do_raffects,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE },
    { "rating",		do_rating,	POS_DEAD,	ML,  LOG_NORMAL, 1, CMD_KEEP_HIDE },
    { "read",		do_look,	POS_RESTING,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "report",		do_report,	POS_RESTING,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "rules",		do_rules,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "scan",           do_scan,        POS_RESTING,     0,  LOG_NORMAL, 1, CMD_KEEP_HIDE },
    { "oscore",		do_oscore,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "skills",		do_skills,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "speak",		do_speak,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "socials",	do_socials,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "show",		do_show,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "spells",		do_spells,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "time",		do_time,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "typo",		do_typo,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "weather",	do_weather,	POS_RESTING,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "who",		do_who,		POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "whois",		do_whois,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "withdraw",	do_withdraw,	POS_STANDING,    0,  LOG_NORMAL, 1, 0 },
    { "wizlist",	do_wizlist,	POS_DEAD,        0,  LOG_NORMAL, 1, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "worth",		do_worth,	POS_SLEEPING,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE | CMD_CHARMED_OK },

    /*
     * Communication commands.
     */
    { "bearcall",       do_bear_call,   POS_FIGHTING,    0,  LOG_NORMAL, 1,0},
    { "clan",		do_clan,	POS_SLEEPING,    0,  LOG_NORMAL, 1, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "=",		do_clan,	POS_SLEEPING,    0,  LOG_NORMAL, 1, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "clanlist",	do_clanlist,	POS_SLEEPING,    0,  LOG_NORMAL, 1, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "clanrecall",	do_crecall,	POS_FIGHTING,	 0,  LOG_NORMAL, 1, 0 },
    { "deaf",		do_deaf,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "emote",		do_emote,	POS_RESTING,	 0,  LOG_NORMAL, 1, CMD_CHARMED_OK },
    { "pmote",		do_pmote,	POS_RESTING,	 0,  LOG_NORMAL, 1, CMD_CHARMED_OK },
    { ",",		do_emote,	POS_RESTING,	 0,  LOG_NORMAL, 0, CMD_CHARMED_OK },
    { "gtell",		do_gtell,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_CHARMED_OK },
    { ";",		do_gtell,	POS_DEAD,	 0,  LOG_NORMAL, 0, CMD_CHARMED_OK },
    { "note",		do_note,	POS_SLEEPING,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "petitio",	do_petitio,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_CHARMED_OK },
    { "petition",	do_petition,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_NOORDER },
    { "pose",		do_pose,	POS_RESTING,	 0,  LOG_NORMAL, 1, CMD_CHARMED_OK },
    { "promote",	do_promote,	POS_DEAD,	 0,  LOG_NORMAL, 1, 0 },
    { "pray",           do_pray,        POS_DEAD,        0,  LOG_NORMAL, 1, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "reply",		do_reply,	POS_SLEEPING,	 0,  LOG_NORMAL, 1, CMD_CHARMED_OK },
    { "replay",		do_replay,	POS_SLEEPING,	 0,  LOG_NORMAL, 1, CMD_CHARMED_OK },
    { "say",		do_say,		POS_RESTING,	 0,  LOG_NORMAL, 1, CMD_CHARMED_OK },
    { "'",		do_say,		POS_RESTING,	 0,  LOG_NORMAL, 0, CMD_CHARMED_OK },
    { "shout",		do_shout,	POS_DEAD,	 3,  LOG_NORMAL, 1, CMD_CHARMED_OK },
    { ".",		do_shout,	POS_DEAD,	 3,  LOG_NORMAL, 1, CMD_CHARMED_OK },
    { "music",		do_music,	POS_DEAD,	 3,  LOG_NORMAL, 1, CMD_CHARMED_OK },
    { "gossip",		do_gossip,	POS_DEAD,	 3,  LOG_NORMAL, 1, CMD_CHARMED_OK },
    { "motd",		do_motd,	POS_DEAD,        0,  LOG_NORMAL, 1, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "wake",		do_wake,	POS_SLEEPING,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE },
    { "warcry",         do_warcry,      POS_FIGHTING,    0,  LOG_NORMAL, 1,0},
    { "unread",		do_unread,	POS_SLEEPING,    0,  LOG_NORMAL, 1, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "yell",		do_yell,	POS_RESTING,	 0,  LOG_NORMAL, 1, CMD_CHARMED_OK },

    /*
     * Configuration commands.
     */
    { "alia",		do_alia,	POS_DEAD,	 0,  LOG_NORMAL, 0, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "alias",		do_alias,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "clear",		do_clear,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "cls",		do_clear,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "autolist",	do_autolist,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "color",		do_color,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "colour",		do_color,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "ansi",		do_color,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "autoassist",	do_autoassist,	POS_DEAD,        0,  LOG_NORMAL, 1, CMD_KEEP_HIDE },
    { "autoexit",	do_autoexit,	POS_DEAD,        0,  LOG_NORMAL, 1, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "autogold",	do_autogold,	POS_DEAD,        0,  LOG_NORMAL, 1, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "autoloot",	do_autoloot,	POS_DEAD,        0,  LOG_NORMAL, 1, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "autosac",	do_autosac,	POS_DEAD,        0,  LOG_NORMAL, 1, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "autosplit",	do_autosplit,	POS_DEAD,        0,  LOG_NORMAL, 1, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "brief",		do_brief,	POS_DEAD,        0,  LOG_NORMAL, 1, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "combine",	do_combine,	POS_DEAD,        0,  LOG_NORMAL, 1, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "compact",	do_compact,	POS_DEAD,        0,  LOG_NORMAL, 1, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "description",	do_description,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "delet",		do_delet,	POS_DEAD,	 0,  LOG_ALWAYS, 0, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "delete",		do_delete,	POS_STANDING,	 0,  LOG_ALWAYS, 1, CMD_KEEP_HIDE | CMD_CHARMED_OK|CMD_NOORDER},
    { "identify",	do_identify,	POS_STANDING,	 0,  LOG_NORMAL, 1, 0 },
    { "long",		do_long,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "nofollow",	do_nofollow,	POS_DEAD,        0,  LOG_NORMAL, 1, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "noloot",		do_noloot,	POS_DEAD,        0,  LOG_NORMAL, 1, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "nosummon",	do_nosummon,	POS_DEAD,        0,  LOG_NORMAL, 1, CMD_KEEP_HIDE },
    { "noiac",		do_noiac,	POS_DEAD,        0,  LOG_NORMAL, 1, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "notelnet",	do_notelnet,	POS_DEAD,        0,  LOG_NORMAL, 1, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "outfit",		do_outfit,	POS_RESTING,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE },
    { "tick",		do_tick,	POS_DEAD,	ML,  LOG_ALWAYS, 1, CMD_KEEP_HIDE },
    { "password",	do_password,	POS_DEAD,	 0,  LOG_NEVER,  1, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "prompt",		do_prompt,	POS_DEAD,        0,  LOG_NORMAL, 1, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "quest",          do_quest,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE | CMD_NOORDER},
    { "qui",		do_qui,		POS_DEAD,	 0,  LOG_NORMAL, 0, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "quit",		do_quit,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE },
    { "quiet",		do_quiet,	POS_SLEEPING, 	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "scroll",		do_scroll,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "title",		do_title,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "twit",		do_twit,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "unalias",	do_unalias,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "wimpy",		do_wimpy,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE | CMD_CHARMED_OK },


    /*
     * Mob command interpreter (placed here for faster scan...)
     */
    { "mob",		do_mob,		POS_DEAD,	 0,  LOG_NEVER,  0, CMD_NOORDER },

    /*
     * Miscellaneous commands.
     */
    { "endure",         do_endure,      POS_STANDING,    0,  LOG_NORMAL, 1, CMD_KEEP_HIDE },
    { "enter", 		do_enter, 	POS_STANDING,	 0,  LOG_NORMAL, 1, 0 },
    { "follow",		do_follow,	POS_RESTING,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE },
    { "gain",		do_gain,	POS_STANDING,	 0,  LOG_NORMAL, 1,0 },
    { "go",		do_enter,	POS_STANDING,	 0,  LOG_NORMAL, 0, 0 },
    { "fade",		do_fade,	POS_RESTING,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE },
    { "herbs",          do_herbs,       POS_STANDING,    0,  LOG_NORMAL, 1,0 },
    { "hara",           do_hara,        POS_STANDING,    0,  LOG_NORMAL,1,0 },

    { "hide",		do_hide,	POS_RESTING,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE },
    { "human",          do_human,       POS_STANDING,    0,  LOG_NORMAL,1,0 },
    { "hunt",           do_hunt,        POS_STANDING,    0,  LOG_NORMAL,1,0 },
    { "leave", 		do_enter, 	POS_STANDING,	 0,  LOG_NORMAL,1, 0 },
    { "rent",		do_rent,	POS_DEAD,	 0,  LOG_NORMAL, 0,0 },
    { "save",		do_save,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE },
    { "sleep",		do_sleep,	POS_SLEEPING,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE },
    { "sneak",		do_sneak,	POS_STANDING,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE },
    { "split",		do_split,	POS_RESTING,	 0,  LOG_NORMAL, 1,0 },
    { "steal",		do_steal,	POS_STANDING,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE },
    { "train",		do_train,	POS_RESTING,	 0,  LOG_NORMAL, 1,0 },
    { "visible",	do_visible,	POS_SLEEPING,	 0,  LOG_NORMAL, 1,0 },
    { "where",		do_where,	POS_RESTING,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE },
    /*
     * Object manipulation commands.
     */
    { "brandish",	do_brandish,	POS_RESTING,	 0,  LOG_NORMAL, 1, 0 },
    { "butcher",        do_butcher,     POS_STANDING,    0,  LOG_NORMAL, 1,0 },
    { "close",		do_close,	POS_RESTING,	 0,  LOG_NORMAL, 1,0 },
    { "detect",         do_detect_hidden,POS_RESTING,    0,  LOG_NORMAL, 1,0},
    { "drink",		do_drink,	POS_RESTING,	 0,  LOG_NORMAL, 1,0 },
    { "drop",		do_drop,	POS_RESTING,	 0,  LOG_NORMAL, 1,0 },
    { "eat",		do_eat,		POS_RESTING,	 0,  LOG_NORMAL, 1,0 },
    { "enchant",	do_enchant, 	POS_RESTING,     0,  LOG_NORMAL, 1,0 },
    { "envenom",	do_envenom,	POS_RESTING,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE },
    { "fill",		do_fill,	POS_RESTING,	 0,  LOG_NORMAL, 1, 0 },
    { "fly",		do_fly,		POS_FIGHTING,	 0,  LOG_NORMAL, 1, 0 },
    { "give",		do_give,	POS_RESTING,	 0,  LOG_NORMAL, 1, 0 },
    { "heal",		do_heal,	POS_RESTING,	 0,  LOG_NORMAL, 1,0 }, 
    { "hold",		do_wear,	POS_RESTING,	 0,  LOG_NORMAL, 1, 0 },
    { "layhands",	do_layhands,	POS_RESTING,	 0,  LOG_NORMAL, 1,0 },
    { "list",		do_list,	POS_RESTING,	 0,  LOG_NORMAL, 1,CMD_KEEP_HIDE },
    { "lock",		do_lock,	POS_RESTING,	 0,  LOG_NORMAL, 1, 0 },
    { "lore",           do_lore,        POS_RESTING,     0,  LOG_NORMAL, 1, 0 },
    { "open",		do_open,	POS_RESTING,	 0,  LOG_NORMAL, 1, 0 },
    { "pick",		do_pick,	POS_RESTING,	 0,  LOG_NORMAL, 1, 0 },
    { "pour",		do_pour,	POS_RESTING,	 0,  LOG_NORMAL, 1,CMD_KEEP_HIDE },
    { "put",		do_put,		POS_RESTING,	 0,  LOG_NORMAL, 1, 0 },
    { "quaff",		do_quaff,	POS_RESTING,	 0,  LOG_NORMAL, 1, 0 },
    { "recite",		do_recite,	POS_RESTING,	 0,  LOG_NORMAL, 1, 0 },
    { "remove",		do_remove,	POS_RESTING,	 0,  LOG_NORMAL, 1, 0 },
    { "request",        do_request,     POS_STANDING,    0,  LOG_NORMAL, 1,0 },
    { "sell",		do_sell,	POS_RESTING,	 0,  LOG_NORMAL, 1, 0 },
    { "take",		do_get,		POS_RESTING,	 0,  LOG_NORMAL, 1,CMD_KEEP_HIDE },
    { "sacrifice",	do_sacrifice,	POS_RESTING,	 0,  LOG_NORMAL, 1,0 },
    { "junk",           do_sacrifice,   POS_RESTING,     0,  LOG_NORMAL, 0,0 },
    { "trophy",         do_trophy,      POS_STANDING,    0,  LOG_NORMAL, 1,0 },
    { "value",		do_value,	POS_RESTING,	 0,  LOG_NORMAL, 1,0 },
    { "wear",		do_wear,	POS_RESTING,	 0,  LOG_NORMAL, 1, 0 },
    { "zap",		do_zap,		POS_RESTING,	 0,  LOG_NORMAL, 1, 0 },

    { "recall",		do_recall,	POS_FIGHTING,	 0,  LOG_NORMAL, 1, 0 },
    { "/",		do_recall,	POS_FIGHTING,	 0,  LOG_NORMAL, 0, 0 },

    /*
     * Combat commands.
     */
    { "ambush",         do_ambush,      POS_STANDING,    0,  LOG_NORMAL, 1,0 },
    { "assassinate",    do_assassinate, POS_STANDING,    0,  LOG_NORMAL, 1,0 },
    { "backstab",	do_backstab,	POS_STANDING,	 0,  LOG_NORMAL, 1,0 },
    { "bash",		do_bash,	POS_FIGHTING,    0,  LOG_NORMAL, 1,0 },
    { "bashdoor",	do_bash_door,	POS_FIGHTING,    0,  LOG_NORMAL, 1,0 },
    { "bs",		do_backstab,	POS_STANDING,	 0,  LOG_NORMAL, 0,0 },
    { "bite",		do_vbite,	POS_STANDING,	 0,  LOG_NORMAL, 0,0 },
    { "blindness",	do_blindness_dust,POS_FIGHTING,	 0,  LOG_ALWAYS, 1,0 },
    { "touch",		do_vtouch,	POS_STANDING,	 0,  LOG_NORMAL, 0,0 },
    { "berserk",	do_berserk,	POS_FIGHTING,	 0,  LOG_NORMAL, 1,0 },
    { "bloodthirst",	do_bloodthirst,	POS_FIGHTING,	 0,  LOG_NORMAL, 1,0 },
    { "blackjack",	do_blackjack,	POS_STANDING,	 0,  LOG_NORMAL, 1,0 },
    { "caltrops",       do_caltrops,    POS_FIGHTING,    0,  LOG_NORMAL, 1,0 },
    { "explode",	do_explode, 	POS_FIGHTING,    0,  LOG_NORMAL, 1,0 },
    { "camouflage",     do_camouflage,  POS_STANDING,    0,  LOG_NORMAL, 1,0 },
    { "circle",         do_circle,      POS_FIGHTING,    0,  LOG_NORMAL, 1,0 },
    { "cleave",         do_cleave,      POS_STANDING,    0,  LOG_NORMAL, 1,0 },

    { "dirt",		do_dirt,	POS_FIGHTING,	 0,  LOG_NORMAL, 1,0 },
    { "disarm",		do_disarm,	POS_FIGHTING,	 0,  LOG_NORMAL, 1,0 },
    { "dishonor",	do_dishonor,	POS_FIGHTING,	 0,  LOG_NORMAL, 1,0 },
    { "dismount",       do_dismount,    POS_STANDING,    0,  LOG_NORMAL, 1,0 },
    { "flee",		do_flee,	POS_FIGHTING,	 0,  LOG_NORMAL, 1,0 },
    { "guard",          do_guard,       POS_STANDING,    0,  LOG_NORMAL, 1,0 },

    { "kick",		do_kick,	POS_FIGHTING,	 0,  LOG_NORMAL, 1,0 },
    { "lioncall",       do_lion_call,   POS_FIGHTING,    0,  LOG_NORMAL, 1,0},
    { "make",           do_make,        POS_STANDING,    0,  LOG_NORMAL, 1,0 },
    { "mount",          do_mount,       POS_STANDING,    0,  LOG_NORMAL, 1,0 },
    { "murde",		do_murde,	POS_FIGHTING,	 0,  LOG_NORMAL, 0,0 },
    { "murder",		do_murder,	POS_FIGHTING,	 0,  LOG_ALWAYS, 1,0 },
    { "nerve",          do_nerve,       POS_FIGHTING,    0,  LOG_NORMAL, 1,0 },
    { "poison",		do_poison_smoke,POS_FIGHTING,	 0,  LOG_ALWAYS, 1,0 },
    { "rescue",		do_rescue,	POS_FIGHTING,	 0,  LOG_NORMAL, 0,0 },
    { "resistance",     do_resistance,  POS_FIGHTING,    0,  LOG_NORMAL, 0,0 },
    { "truesight",      do_truesight,   POS_FIGHTING,    0,  LOG_NORMAL, 0,0 },
    { "thumbling",	do_thumbling,	POS_FIGHTING,	 0,  LOG_NORMAL, 0,CMD_KEEP_HIDE },
    { "shield",		do_shield,	POS_FIGHTING,	 0,  LOG_NORMAL, 1,0 },
    { "spellbane",      do_spellbane,   POS_FIGHTING,    0,  LOG_NORMAL, 0,0 },
    { "strangle",       do_strangle,    POS_STANDING,    0,  LOG_NORMAL, 1,0 },
    { "surrender",	do_surrender,	POS_FIGHTING,    0,  LOG_NORMAL, 1,0 },
    { "tame",           do_tame,        POS_FIGHTING,    0,  LOG_NORMAL, 1,0 },
    { "throw",          do_throw,       POS_FIGHTING,    0,  LOG_NORMAL, 1,0 },
    { "tiger",		do_tiger,	POS_FIGHTING,	 0,  LOG_NORMAL, 1,0 },
    { "trip",		do_trip,	POS_FIGHTING,    0,  LOG_NORMAL, 1,0 },
    { "target",		do_target,	POS_FIGHTING,    0,  LOG_NORMAL, 1,0 },
    { "vampire",	do_vampire,	POS_FIGHTING,    0,  LOG_NORMAL, 1,0 },
    { "vanish",		do_vanish,	POS_FIGHTING,    0,  LOG_NORMAL, 1,0 },
    { "weapon",		do_weapon,	POS_FIGHTING,	 0,  LOG_NORMAL, 1,0 },
    { "blink",		do_blink,	POS_FIGHTING,    0,  LOG_NORMAL, 1,CMD_KEEP_HIDE },


    /*
     * Immortal commands.
     */
    { "advance",	do_advance,	POS_DEAD,	ML,  LOG_ALWAYS, 1, CMD_KEEP_HIDE }, 
    { "set",		do_set,		POS_DEAD,	L1,  LOG_ALWAYS, 1, CMD_KEEP_HIDE },
    { "dump",		do_dump,	POS_DEAD,	ML,  LOG_ALWAYS, 0, CMD_KEEP_HIDE },
    { "rename",		do_rename,	POS_DEAD,	ML,  LOG_ALWAYS, 1, CMD_KEEP_HIDE },
    { "violate",	do_violate,	POS_DEAD,	ML,  LOG_ALWAYS, 1, CMD_KEEP_HIDE },
    { "track",          do_track,       POS_STANDING,    0,  LOG_NORMAL, 0,0 },
    { "trust",		do_trust,	POS_DEAD,	ML,  LOG_ALWAYS, 1, CMD_KEEP_HIDE },

    { "allow",		do_allow,	POS_DEAD,	L2,  LOG_ALWAYS, 1, CMD_KEEP_HIDE },
    { "ban",		do_ban,		POS_DEAD,	L2,  LOG_ALWAYS, 1, CMD_KEEP_HIDE },
    { "deny",		do_deny,	POS_DEAD,	L1,  LOG_ALWAYS, 1, CMD_KEEP_HIDE },
    { "disconnect",	do_disconnect,	POS_DEAD,	L3,  LOG_ALWAYS, 1, CMD_KEEP_HIDE },
    { "freeze",		do_freeze,	POS_DEAD,	L4,  LOG_ALWAYS, 1, CMD_KEEP_HIDE },
    { "permban",	do_permban,	POS_DEAD,	L1,  LOG_ALWAYS, 1, CMD_KEEP_HIDE },
    { "protect",	do_protect,	POS_DEAD,	L1,  LOG_ALWAYS, 1, CMD_KEEP_HIDE },
    { "reboo",		do_reboo,	POS_DEAD,	L1,  LOG_NEVER, 0, CMD_KEEP_HIDE },
    { "reboot",		do_reboot,	POS_DEAD,	L1,  LOG_ALWAYS, 1, CMD_KEEP_HIDE },
    { "smite",		do_smite,	POS_DEAD,	L2,  LOG_ALWAYS, 1,0 },
    { "limited",	do_limited,	POS_DEAD,	L2,  LOG_ALWAYS, 1, CMD_KEEP_HIDE },
    { "popularity",	do_popularity,	POS_DEAD,	L2,  LOG_ALWAYS, 1, CMD_KEEP_HIDE },
    { "shutdow",	do_shutdow,	POS_DEAD,	L1,  LOG_NEVER, 0, CMD_KEEP_HIDE },
    { "shutdown",	do_shutdown,	POS_DEAD,	L1,  LOG_ALWAYS, 1, CMD_KEEP_HIDE},
    { "wizlock",	do_wizlock,	POS_DEAD,	L2,  LOG_ALWAYS, 1, CMD_KEEP_HIDE },
    { "affrooms",	do_affrooms,	POS_DEAD,	L5,  LOG_ALWAYS, 1, CMD_KEEP_HIDE },
    { "force",		do_force,	POS_DEAD,	L5,  LOG_ALWAYS, 1, CMD_KEEP_HIDE },
    { "load",		do_load,	POS_DEAD,	L4,  LOG_ALWAYS, 1, CMD_KEEP_HIDE },
    { "newlock",	do_newlock,	POS_DEAD,	L4,  LOG_ALWAYS, 1, CMD_KEEP_HIDE },
    { "noaffect",	do_noaffect,	POS_DEAD,	L5,  LOG_ALWAYS, 1, CMD_KEEP_HIDE },
    { "nochannels",	do_nochannels,	POS_DEAD,	L5,  LOG_ALWAYS, 1, CMD_KEEP_HIDE },
    { "notitle",	do_notitle,	POS_DEAD,	L5,  LOG_ALWAYS, 1, CMD_KEEP_HIDE },
    { "noemote",	do_noemote,	POS_DEAD,	L5,  LOG_ALWAYS, 1, CMD_KEEP_HIDE },
    { "noshout",	do_noshout,	POS_DEAD,	L5,  LOG_ALWAYS, 1, CMD_KEEP_HIDE },
    { "notell",		do_notell,	POS_DEAD,	L5,  LOG_ALWAYS, 1, CMD_KEEP_HIDE },
    { "pecho",		do_pecho,	POS_DEAD,	L4,  LOG_ALWAYS, 1, CMD_KEEP_HIDE }, 
    { "purge",		do_purge,	POS_DEAD,	L4,  LOG_ALWAYS, 1, CMD_KEEP_HIDE },
    { "restore",	do_restore,	POS_DEAD,	L4,  LOG_ALWAYS, 1, CMD_KEEP_HIDE },
    { "sla",		do_sla,		POS_DEAD,	L3,  LOG_NEVER, 0, CMD_KEEP_HIDE },
    { "slay",		do_slay,	POS_DEAD,	L3,  LOG_ALWAYS, 1, CMD_KEEP_HIDE },
    { "teleport",	do_transfer,    POS_DEAD,	L5,  LOG_ALWAYS, 1, CMD_KEEP_HIDE },	
    { "transfer",	do_transfer,	POS_DEAD,	L5,  LOG_ALWAYS, 1, CMD_KEEP_HIDE },
    { "poofin",		do_bamfin,	POS_DEAD,	L8,  LOG_NORMAL, 1, CMD_KEEP_HIDE },
    { "poofout",	do_bamfout,	POS_DEAD,	L8,  LOG_NORMAL, 1, CMD_KEEP_HIDE },
    { "gecho",		do_echo,	POS_DEAD,	L4,  LOG_ALWAYS, 1, CMD_KEEP_HIDE },
    { ">",		do_echo,	POS_DEAD,	L4,  LOG_ALWAYS, 1, CMD_KEEP_HIDE },
    { "holylight",	do_holylight,	POS_DEAD,	IM,  LOG_NORMAL, 1, CMD_KEEP_HIDE },
    { "incognito",	do_incognito,	POS_DEAD,	IM,  LOG_NORMAL, 1, CMD_KEEP_HIDE },
    { "invis",		do_invis,	POS_DEAD,	IM,  LOG_NORMAL, 0, CMD_KEEP_HIDE },
    { "log",		do_log,		POS_DEAD,	L1,  LOG_ALWAYS, 1, CMD_KEEP_HIDE  },
    { "memory",		do_memory,	POS_DEAD,	IM,  LOG_NEVER, 1, CMD_KEEP_HIDE },
    { "mwhere",		do_mwhere,	POS_DEAD,	IM,  LOG_NORMAL, 1, CMD_KEEP_HIDE },
    { "owhere",		do_owhere,	POS_DEAD,	IM,  LOG_NORMAL, 1, CMD_KEEP_HIDE },
    { "peace",		do_peace,	POS_DEAD,	L5,  LOG_NORMAL, 1, CMD_KEEP_HIDE },
    { "penalty",	do_penalty,	POS_DEAD,	L7,  LOG_NORMAL, 1, CMD_KEEP_HIDE },
    { "echo",		do_recho,	POS_DEAD,	L6,  LOG_ALWAYS, 1, CMD_KEEP_HIDE },
    { "return",         do_return,      POS_DEAD,       L6,  LOG_NORMAL, 1, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "snoop",		do_snoop,	POS_DEAD,	ML,  LOG_ALWAYS, 1, CMD_KEEP_HIDE},
    { "stat",		do_stat,	POS_DEAD,	IM,  LOG_NORMAL, 1, CMD_KEEP_HIDE },
    { "string",		do_string,	POS_DEAD,	L5,  LOG_ALWAYS, 1, CMD_KEEP_HIDE },
    { "switch",		do_switch,	POS_DEAD,	L6,  LOG_ALWAYS, 1, CMD_KEEP_HIDE },
    { "wizinvis",	do_invis,	POS_DEAD,	IM,  LOG_NORMAL, 1, CMD_KEEP_HIDE },
    { "vnum",		do_vnum,	POS_DEAD,	L4,  LOG_NORMAL, 1, CMD_KEEP_HIDE },
    { "zecho",		do_zecho,	POS_DEAD,	L4,  LOG_ALWAYS, 1, CMD_KEEP_HIDE },
    { "clone",		do_clone,	POS_DEAD,	L5,  LOG_ALWAYS, 1, CMD_KEEP_HIDE },
    { "wiznet",		do_wiznet,	POS_DEAD,	IM,  LOG_NORMAL, 1, CMD_KEEP_HIDE },
    { "immtalk",	do_immtalk,	POS_DEAD,	IM,  LOG_NORMAL, 1, CMD_KEEP_HIDE },
    { "imotd",          do_imotd,       POS_DEAD,       IM,  LOG_NORMAL, 1, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { ":",		do_immtalk,	POS_DEAD,	IM,  LOG_NORMAL, 0, CMD_KEEP_HIDE },
    { "smote",		do_smote,	POS_DEAD,	IM,  LOG_NORMAL, 1, CMD_KEEP_HIDE },
    { "prefi",		do_prefi,	POS_DEAD,	IM,  LOG_NORMAL, 0, CMD_KEEP_HIDE },
    { "prefix",		do_prefix,	POS_DEAD,	IM,  LOG_NORMAL, 1, CMD_KEEP_HIDE },
    { "objlist",	do_objlist,	POS_DEAD,	ML,  LOG_NORMAL, 1, CMD_KEEP_HIDE },
    { "settraps",	do_settraps,	POS_STANDING,	 0,  LOG_NORMAL, 1,0},
    { "slook",		do_slook,	POS_SLEEPING,	 0,  LOG_NORMAL, 1,CMD_KEEP_HIDE},
    { "learn",		do_learn,	POS_STANDING,	 0,  LOG_NORMAL, 1,0},
    { "teach",		do_teach,	POS_STANDING,	 0,  LOG_NORMAL, 1,0},
    { "camp",		do_camp,  	POS_STANDING,    0,  LOG_NORMAL, 1,0 },
    { "tail",		do_tail,	POS_FIGHTING,    0,  LOG_NORMAL, 1,0 },
    { "push",		do_push,	POS_STANDING,	 0,  LOG_NORMAL, 1,0 },
    { "demand",         do_demand,      POS_STANDING,    0,  LOG_NORMAL, 1,0 },
    { "bandage",        do_bandage,     POS_FIGHTING,    0,  LOG_NORMAL, 1,0 },
    { "shoot",          do_shoot,       POS_STANDING,    0,  LOG_NORMAL, 1,0 },
    { "find",		do_find,	POS_DEAD,	ML,  LOG_ALWAYS, 1, CMD_KEEP_HIDE },
    { "score",		do_score,	POS_DEAD,	 0,  LOG_NORMAL, 1, CMD_KEEP_HIDE },
    { "katana",		do_katana, 	POS_STANDING,    0,  LOG_NORMAL, 1,0 },
    { "control",       	do_control,	POS_STANDING,    0,  LOG_NORMAL, 1,0 },
    { "ititle",		do_ititle,	POS_DEAD,	IM,  LOG_NORMAL, 1,CMD_KEEP_HIDE },
    { "sense",          do_sense,       POS_RESTING,     0,  LOG_NORMAL, 1,0},

    { "mpdump",		do_mpdump,	POS_DEAD,	IM,  LOG_NEVER,  1, 0 },
    { "mpstat",		do_mpstat,	POS_DEAD,	IM,  LOG_NEVER,  1, 0 },


    { "msgstat",	do_msgstat,	POS_DEAD,	IM,  LOG_NEVER,  1, 0 },
    { "strstat",	do_strstat,	POS_DEAD,	IM,  LOG_NEVER,  1, 0 },

    /*
     * OLC
     */
    { "edit",		do_edit,	POS_DEAD,   IM,  LOG_ALWAYS, 1, CMD_KEEP_HIDE },
    { "create",		do_create,	POS_DEAD,   IM,  LOG_ALWAYS, 1, CMD_KEEP_HIDE },
    { "asave",          do_asave,	POS_DEAD,   IM,  LOG_ALWAYS, 1, CMD_KEEP_HIDE },
    { "alist",		do_alist,	POS_DEAD,   IM,  LOG_NEVER, 1, CMD_KEEP_HIDE },
    { "resets",		do_resets,	POS_DEAD,   IM,  LOG_NORMAL, 1, CMD_KEEP_HIDE },

    /*
     * End of list.
     */
    { str_empty,		0,		POS_DEAD,    0,  LOG_NORMAL, 0, CMD_KEEP_HIDE }
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
	char logline[MAX_INPUT_LENGTH];
#ifdef IMMORTALS_LOGS
	char buf[MAX_INPUT_LENGTH];
	char *strtime;
#endif
	int cmd;
	int trust;
	bool found;

	/*
	 * Strip leading spaces.
	 */
	while (isspace(*argument))
		argument++;
	if (argument[0] == '\0')
		return;

	/*
	 * Implement freeze command.
	 */
	if (!IS_NPC(ch) && IS_SET(ch->act, PLR_FREEZE)) {
		char_puts("You're totally frozen!\n\r", ch);
		return;
	}

	/*
	 * Grab the command word.
	 * Special parsing so ' can be a command,
	 * also no spaces needed after punctuation.
	 */
	strcpy(logline, argument);

#ifdef IMMORTALS_LOGS
 	if (IS_IMMORTAL(ch)) {
		if ((imm_log = dfopen(GODS_PATH, IMMLOG_FILE, "a+")) == NULL)
			bug("cannot open imm_log_file", 0);
		else {
			strtime = (char *) malloc(100);
			strtime = ctime(&current_time);
			strtime[strlen(strtime) -1] = '\0';
			sprintf(buf,"%s :[%s]:%s\n", strtime,ch->name, logline);
			fprintf(imm_log, buf);
			fclose(imm_log);
			free(strtime);
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
		argument = one_argument(argument, command);

	/*
	 * Look for command in command table.
	 */
	found = FALSE;
	trust = get_trust(ch);
	for (cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++) {
		if (command[0] != cmd_table[cmd].name[0]
		||  str_prefix(command, cmd_table[cmd].name)
		||  cmd_table[cmd].level > trust)
			continue;

#if 0
		if (is_order && !IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM)) {
			if (number_percent() < get_curr_stat(ch, STAT_CHA)) {
				do_say(ch, "Nah, I won't do that.");
				return;
			}

			if (number_percent() < get_curr_stat(ch, STAT_CHA)
#endif

		if (is_order) {
			if (IS_SET(cmd_table[cmd].extra, CMD_NOORDER)
			||  cmd_table[cmd].level >= LEVEL_IMMORTAL)
				return;
		}
		else {
			if (IS_AFFECTED(ch, AFF_CHARM)
			&&  !IS_SET(cmd_table[cmd].extra, CMD_CHARMED_OK)
			&&  cmd_table[cmd].level < LEVEL_IMMORTAL) {
				char_puts("First ask your beloved master!\n\r",
					  ch);
				return;
			}
		}

		if (IS_AFFECTED(ch, AFF_STUN) 
		&&  !(cmd_table[cmd].extra & CMD_KEEP_HIDE)) {
			char_puts("You are STUNNED to do that.\n\r", ch);
			return;
		}

		/* Come out of hiding for most commands */
		if (IS_AFFECTED(ch, AFF_HIDE | AFF_FADE) && !IS_NPC(ch)
		&& !(cmd_table[cmd].extra & CMD_KEEP_HIDE)) {
			REMOVE_BIT(ch->affected_by, AFF_HIDE | AFF_FADE);
			char_nputs(MSG_YOU_STEP_OUT_SHADOWS, ch);
			act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING, 
				    MSG_N_STEPS_OUT_OF_SHADOWS);
        	}

		if (IS_AFFECTED(ch, AFF_IMP_INVIS) && !IS_NPC(ch)
		&& (cmd_table[cmd].position == POS_FIGHTING)) {
			affect_bit_strip(ch, TO_AFFECTS, AFF_IMP_INVIS);
			char_puts("You fade into existence.", ch);
			act("$n fades into existence.",
			    ch, NULL, NULL, TO_ROOM);
		}

		found = TRUE;
		break;
	}

	/*
	 * Log and snoop.
	 */
	if (cmd_table[cmd].log == LOG_NEVER)
		strcpy(logline, str_empty);

	if (((!IS_NPC(ch) && IS_SET(ch->act, PLR_LOG))
	||   fLogAll
	||   cmd_table[cmd].log == LOG_ALWAYS) && logline[0] != '\0' 
	&&   logline[0] != '\n') {
		log_printf("Log %s: %s", ch->name, logline);
		wiznet_printf(ch, NULL, WIZ_SECURE, 0, get_trust(ch),
				"Log %s: %s", ch->name, logline);
	}

	if (ch->desc != NULL && ch->desc->snoop_by != NULL) {
		write_to_buffer(ch->desc->snoop_by, "# ", 2);
		write_to_buffer(ch->desc->snoop_by, logline, 0);
		write_to_buffer(ch->desc->snoop_by, "\n\r", 2);
	}

	if (!found) {
		/*
		 * Look for command in socials table.
		 */
		if (!check_social(ch, command, argument)) {
			char_puts("Huh?\n\r", ch);
			return;
		}
		else
			return;
	}

	/*
	 * Character not in position for command?
	 */
	if (ch->position < cmd_table[cmd].position) {
		switch(ch->position) {
			case POS_DEAD:
				char_puts("Lie still; You are DEAD.\n\r", ch);
				break;

			case POS_MORTAL:
			case POS_INCAP:
				char_puts("You are hurt far too bad for that.\n\r", ch);
				break;

			case POS_STUNNED:
				char_puts("You are too stunned to do that.\n\r", ch);
				break;

			case POS_SLEEPING:
				char_puts("In your dreams, or what?\n\r", ch);
				break;

			case POS_RESTING:
				char_nputs(MSG_TOO_RELAXED, ch);
				break;

			case POS_SITTING:
				char_nputs(MSG_BETTER_STANDUP, ch);
				break;

			case POS_FIGHTING:
				char_puts("No way!  You are still fighting!\n\r", ch);
				break;

		}
		return;
	}

	/*
	 * Dispatch the command.
	 */
	(*cmd_table[cmd].do_fun) (ch, argument);

	tail_chain();
}

bool check_social(CHAR_DATA *ch, char *command, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	ROOM_INDEX_DATA *victim_room;
	int cmd;
	bool found;
	found = FALSE;

	for (cmd = 0; social_table[cmd].name != NULL; cmd++) {
		if (command[0] == social_table[cmd].name[0]
		&&  !str_prefix(command, social_table[cmd].name)) {
			found = TRUE;
			break;
		}
	}

	if (!found)
		return FALSE;

	if (!IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE)) {
		char_puts("You are anti-social!\n\r", ch);
		return TRUE;
	}

	switch (ch->position) {
		case POS_DEAD:
			char_puts("Lie still; You are DEAD.\n\r", ch);
			return TRUE;

		case POS_INCAP:
		case POS_MORTAL:
			char_puts("You are hurt far too bad for that.\n\r", ch);
			return TRUE;

		case POS_STUNNED:
			char_puts("You are too stunned to do that.\n\r", ch);
			return TRUE;

		case POS_SLEEPING:
		/*
		 * I just know this is the path to a 12" 'if' statement.  :(
		 * But two players asked for it already!  -- Furey
		 */
			if (!str_cmp(social_table[cmd].name, "snore"))
				break;
			char_puts("In your dreams, or what?\n\r", ch);
			return TRUE;
	}

	if (IS_AFFECTED(ch, AFF_HIDE | AFF_FADE)) {
		REMOVE_BIT(ch->affected_by, AFF_HIDE | AFF_FADE);
		char_nputs(MSG_YOU_STEP_OUT_SHADOWS, ch);
		act_nputs(MSG_N_STEPS_OUT_OF_SHADOWS, ch, NULL, NULL, TO_ROOM,
			  POS_RESTING); 
	}

	if (IS_AFFECTED(ch, AFF_IMP_INVIS) && !IS_NPC(ch)
	&& (cmd_table[cmd].position == POS_FIGHTING)) {
		affect_bit_strip(ch, TO_AFFECTS, AFF_IMP_INVIS);
		char_puts("You fade into existence.\n\r", ch);
		act_puts("$n fades into existence.\n\r", ch, NULL, NULL, TO_ROOM,
			  POS_RESTING);
	}

	one_argument(argument, arg);
	victim = NULL;
	if (arg[0] == '\0') {
		act(social_table[cmd].val[SOC_OTHERS_NO_ARG],
			ch, NULL, victim, TO_ROOM | TO_BUF | CHECK_TWIT);
		act(social_table[cmd].val[SOC_CHAR_NO_ARG],
			ch, NULL, victim, TO_CHAR);
		return TRUE;
	}

	if ((victim = get_char_world(ch, arg)) == NULL
	||  (IS_NPC(victim) && victim->in_room != ch->in_room)) {
		char_puts("They aren't here.\n\r", ch);
		return TRUE;
	}

	if (victim == ch) {
		act(social_table[cmd].val[SOC_OTHERS_AUTO],
			ch, NULL, victim, TO_ROOM | TO_BUF | CHECK_TWIT);
		act(social_table[cmd].val[SOC_CHAR_AUTO],
			ch, NULL, victim, TO_CHAR);
		return TRUE;
	}

	victim_room = victim->in_room;
	victim->in_room = ch->in_room;

	act(social_table[cmd].val[SOC_OTHERS_FOUND],
		ch, NULL, victim, TO_NOTVICT | TO_BUF | CHECK_TWIT);
	act(social_table[cmd].val[SOC_CHAR_FOUND],
		ch, NULL, victim, TO_CHAR);
	act(social_table[cmd].val[SOC_VICT_FOUND],
		ch, NULL, victim, TO_VICT | TO_BUF | CHECK_TWIT);

	victim->in_room = victim_room;

	if (!IS_NPC(ch) && IS_NPC(victim) 
	&&  !IS_AFFECTED(victim, AFF_CHARM)
	&&  IS_AWAKE(victim) && victim->desc == NULL) {
		switch (number_bits(4)) {
			case 0:

			case 1: case 2: case 3: case 4:
			case 5: case 6: case 7: case 8:
				act(social_table[cmd].val[SOC_OTHERS_FOUND],
				    victim, NULL, ch,
				    TO_NOTVICT | TO_BUF | CHECK_TWIT);
				act(social_table[cmd].val[SOC_CHAR_FOUND],
				    victim, NULL, ch, TO_CHAR);
				act(social_table[cmd].val[SOC_VICT_FOUND],
				    victim, NULL, ch, TO_VICT | TO_BUF);
				break;

			case 9: case 10: case 11: case 12:
				act("$n slaps $N.", victim, NULL, ch, 
				    TO_NOTVICT | TO_BUF | CHECK_TWIT);
				act("You slap $N.", victim, NULL, ch, TO_CHAR);
				act("$n slaps you.", victim, NULL, ch, 
				    TO_VICT | TO_BUF);
				break;
		}
	}
	return TRUE;
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

static int x_argument(const char *argument, char *arg, char c)
{
	char *p;
	char *q;
	int number;
    
	p = strchr(argument, c);
	if (p == NULL) {
		strcpy(arg, argument);
		return 1;
	}

	number = strtod(argument, &q);

	/*
	 * if c == '.' q will point to next character after p
	 * otherwise q will point to p
	 */
	if (number < 0 || (q != p && (c == '.' && q != p+1)))
		number = 0;
	strcpy(arg, p+1);
	return number;
}

/*
 * Given a string like 14.foo, return 14 and 'foo'
 */
int number_argument(const char *argument, char *arg)
{
	return x_argument(argument, arg, '.');
}

/* 
 * Given a string like 14*foo, return 14 and 'foo'
 */
int mult_argument(const char *argument, char *arg)
{
	return x_argument(argument, arg, '*');
}

/*
 * Pick off one argument from a string and return the rest.
 * Understands quotes.
 */
const char *one_argument(const char *argument, char *arg_first)
{
	return first_arg(argument, arg_first, TRUE);
}

/*****************************************************************************
 Name:		first_arg
 Purpose:	Pick off one argument from a string and return the rest.
 		Understands quotes, if fCase then case will not be modified
 Called by:	string_add(string.c)
 ****************************************************************************/
const char *first_arg(const char *argument, char *arg_first, bool fCase)
{
	char cEnd = '\0';

/* skip leading spaces */
	while (isspace(*argument))
		argument++;

/* check quotes */
	if (*argument == '\'' || *argument == '"')
        	cEnd = *argument++;

	while (*argument) {
		if ((!cEnd && isspace(*argument)) || *argument == cEnd) {
			argument++;
			break;
		}
		*arg_first++ = fCase ? LOWER(*argument) : *argument;
		argument++;
	}
	*arg_first = '\0';

	while (isspace(*argument))
		argument++;

	return argument;
}

/*
 * Contributed by Alander.
 */
void do_commands(CHAR_DATA *ch, const char *argument)
{
    char buf[MAX_STRING_LENGTH];
    int cmd;
    int col;
 
    col = 0;
    for (cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++)
    {
        if (cmd_table[cmd].level <  LEVEL_HERO
        &&   cmd_table[cmd].level <= get_trust(ch) 
	&&   cmd_table[cmd].show)
	{
	    sprintf(buf, "%-12s", cmd_table[cmd].name);
	    char_puts(buf, ch);
	    if (++col % 6 == 0)
		char_puts("\n\r", ch);
	}
    }
 
    if (col % 6 != 0)
	char_puts("\n\r", ch);
    return;
}

void do_wizhelp(CHAR_DATA *ch, const char *argument)
{
    char buf[MAX_STRING_LENGTH];
    int cmd;
    int col;
 
    col = 0;
    for (cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++)
    {
        if (cmd_table[cmd].level >= LEVEL_HERO
        &&   cmd_table[cmd].level <= get_trust(ch) 
        &&   cmd_table[cmd].show)
	{
	    sprintf(buf, "%-12s", cmd_table[cmd].name);
	    char_puts(buf, ch);
	    if (++col % 6 == 0)
		char_puts("\n\r", ch);
	}
    }
 
    if (col % 6 != 0)
	char_puts("\n\r", ch);
    return;
}


void do_reture(CHAR_DATA *ch, const char *argument)
{
  char_puts("Ok.\n\r", ch);
  return;
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
	    char_puts("Line to long, prefix not processed.\r\n",ch);
	else
	{
	    sprintf(prefix,"%s %s",ch->prefix,argument);
	    argument = prefix;
	}
    }

    if (IS_NPC(ch) || ch->pcdata->alias[0] == NULL
    ||	!str_prefix("alias",argument) || !str_prefix("una",argument) 
    ||  !str_prefix("prefix",argument)) {
	interpret(d->character, argument);
	return;
    }

    strcpy(buf,argument);

    for (alias = 0; alias < MAX_ALIAS; alias++)	 /* go through the aliases */
    {
	if (ch->pcdata->alias[alias] == NULL)
	    break;

	if (!str_prefix(ch->pcdata->alias[alias],argument))
	{
	    point = one_argument(argument,name);
	    if (!strcmp(ch->pcdata->alias[alias],name))
	    {
		buf[0] = '\0';
		strcat(buf,ch->pcdata->alias_sub[alias]);
		strcat(buf," ");
		strcat(buf,point);
		break;
	    }
	    if (strlen(buf) > MAX_INPUT_LENGTH)
	    {
		char_puts("Alias substitution too long. Truncated.\r\n",ch);
		buf[MAX_INPUT_LENGTH -1] = '\0';
	    }
	}
    }
    interpret(d->character, buf);
}

void do_alia(CHAR_DATA *ch, const char *argument)
{
    char_puts("I'm sorry, alias must be entered in full.\n\r",ch);
    return;
}

void do_alias(CHAR_DATA *ch, const char *argument)
{
    CHAR_DATA *rch;
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
    int pos;

    if (ch->desc == NULL)
	rch = ch;
    else
	rch = ch->desc->original ? ch->desc->original : ch;

    if (IS_NPC(rch))
	return;

    argument = one_argument(argument,arg);
    

    if (arg[0] == '\0')
    {
	if (rch->pcdata->alias[0] == NULL)
	{
	    char_puts("You have no aliases defined.\n\r",ch);
	    return;
	}
	char_puts("Your current aliases are:\n\r",ch);

	for (pos = 0; pos < MAX_ALIAS; pos++)
	{
	    if (rch->pcdata->alias[pos] == NULL
	    ||	rch->pcdata->alias_sub[pos] == NULL)
		break;

	    char_printf(ch,"    %s:  %s\n\r",rch->pcdata->alias[pos],
		    rch->pcdata->alias_sub[pos]);
	}
	return;
    }

    if (!str_prefix("una",arg) || !str_cmp("alias",arg))
    {
	char_puts("Sorry, that word is reserved.\n\r",ch);
	return;
    }

    if (argument[0] == '\0')
    {
	for (pos = 0; pos < MAX_ALIAS; pos++)
	{
	    if (rch->pcdata->alias[pos] == NULL
	    ||	rch->pcdata->alias_sub[pos] == NULL)
		break;

	    if (!str_cmp(arg,rch->pcdata->alias[pos]))
	    {
		sprintf(buf,"%s aliases to '%s'.\n\r",rch->pcdata->alias[pos],
			rch->pcdata->alias_sub[pos]);
		char_puts(buf,ch);
		return;
	    }
	}

	char_puts("That alias is not defined.\n\r",ch);
	return;
    }

    if (!str_prefix(argument,"delete") || !str_prefix(argument,"prefix"))
    {
	char_puts("That shall not be done!\n\r",ch);
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
	    smash_tilde(rch->pcdata->alias_sub[pos]);
	    char_printf(ch,"%s is now realiased to '%s'.\n\r",arg,argument);
	    return;
	}
     }

     if (pos >= MAX_ALIAS)
     {
	char_puts("Sorry, you have reached the alias limit.\n\r",ch);
	return;
     }
  
     /* make a new alias */
     rch->pcdata->alias[pos]		= str_dup(arg);
     rch->pcdata->alias_sub[pos]	= str_dup(argument);
     smash_tilde(rch->pcdata->alias_sub[pos]);
     char_printf(ch,"%s is now aliased to '%s'.\n\r",arg,argument);
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
 
    argument = one_argument(argument,arg);

    if (arg == '\0')
    {
	char_puts("Unalias what?\n\r",ch);
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
	    char_puts("Alias removed.\n\r",ch);
	    free_string(rch->pcdata->alias[pos]);
	    free_string(rch->pcdata->alias_sub[pos]);
	    rch->pcdata->alias[pos] = NULL;
	    rch->pcdata->alias_sub[pos] = NULL;
	    found = TRUE;
	}
    }

    if (!found)
	char_puts("No alias of that name to remove.\n\r",ch);
}

     
