/*
 * $Id: act_info.c,v 1.442 2004-02-19 17:16:42 fjoe Exp $
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

#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#if !defined(WIN32)
#include <unistd.h>
#endif
#include <ctype.h>

#include <merc.h>
#include <lang.h>

#include <sog.h>
#include <quest.h>

#if defined(SUNOS) || defined(SVR4) || defined(LINUX)
#include <crypt.h>
#endif

#include "toggle.h"

DECLARE_DO_FUN(do_clear);
DECLARE_DO_FUN(do_scroll);
DECLARE_DO_FUN(do_socials);
DECLARE_DO_FUN(do_motd);
DECLARE_DO_FUN(do_imotd);
DECLARE_DO_FUN(do_rules);
DECLARE_DO_FUN(do_wizlist);
DECLARE_DO_FUN(do_auto);
DECLARE_DO_FUN(do_prompt);
DECLARE_DO_FUN(do_nogive);
DECLARE_DO_FUN(do_nofollow);
DECLARE_DO_FUN(do_nosummon);
DECLARE_DO_FUN(do_glance);
DECLARE_DO_FUN(do_look);
DECLARE_DO_FUN(do_examine);
DECLARE_DO_FUN(do_exits);
DECLARE_DO_FUN(do_worth);
DECLARE_DO_FUN(do_time);
DECLARE_DO_FUN(do_date);
DECLARE_DO_FUN(do_weather);
DECLARE_DO_FUN(do_help);
DECLARE_DO_FUN(do_who);
DECLARE_DO_FUN(do_whois);
DECLARE_DO_FUN(do_inventory);
DECLARE_DO_FUN(do_equipment);
DECLARE_DO_FUN(do_compare);
DECLARE_DO_FUN(do_credits);
DECLARE_DO_FUN(do_where);
DECLARE_DO_FUN(do_consider);
DECLARE_DO_FUN(do_description);
DECLARE_DO_FUN(do_report);
DECLARE_DO_FUN(do_wimpy);
DECLARE_DO_FUN(do_password);
DECLARE_DO_FUN(do_scan);
DECLARE_DO_FUN(do_request);
DECLARE_DO_FUN(do_hometown);
DECLARE_DO_FUN(do_detect_hidden);
DECLARE_DO_FUN(do_awareness);
DECLARE_DO_FUN(do_bear_call);
DECLARE_DO_FUN(do_identify);
DECLARE_DO_FUN(do_score);
DECLARE_DO_FUN(do_oscore);
DECLARE_DO_FUN(do_affects);
DECLARE_DO_FUN(do_raffects);
DECLARE_DO_FUN(do_resists);
DECLARE_DO_FUN(do_lion_call);
DECLARE_DO_FUN(do_practice);
DECLARE_DO_FUN(do_learn);
DECLARE_DO_FUN(do_teach);
DECLARE_DO_FUN(do_gain);
DECLARE_DO_FUN(do_prayers);
DECLARE_DO_FUN(do_spells);
DECLARE_DO_FUN(do_skills);
DECLARE_DO_FUN(do_glist);
DECLARE_DO_FUN(do_slook);
DECLARE_DO_FUN(do_camp);
DECLARE_DO_FUN(do_demand);
DECLARE_DO_FUN(do_control);
DECLARE_DO_FUN(do_make_arrow);
DECLARE_DO_FUN(do_make_bow);
DECLARE_DO_FUN(do_make);
DECLARE_DO_FUN(do_homepoint);
DECLARE_DO_FUN(do_commands);
DECLARE_DO_FUN(do_wizhelp);
DECLARE_DO_FUN(do_clanlist);
DECLARE_DO_FUN(do_item);
DECLARE_DO_FUN(do_rating);
DECLARE_DO_FUN(do_areas);
DECLARE_DO_FUN(do_compress);

/* command procedures needed */
DECLARE_DO_FUN(do_murder);
DECLARE_DO_FUN(do_say);

/*
 * Local functions.
 */
static void	show_char_to_char_0	(CHAR_DATA *victim,
					 CHAR_DATA *ch);
static void	show_char_to_char	(CHAR_DATA *list, CHAR_DATA *ch);
static void	list_spells(flag_t type, CHAR_DATA *ch, const char *argument);
static bool	list_form_skills(CHAR_DATA *ch, BUFFER *output);

DO_FUN(do_clear, ch, argument)
{
	if (!IS_NPC(ch))
		send_to_char("\033[0;0H\033[2J", ch);	// notrans
}

/* changes your scroll */
DO_FUN(do_scroll, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];
	int pagelen;
	DESCRIPTOR_DATA *d;

	if ((d = ch->desc) == NULL)
		return;

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		act_puts("You currently display $j $qj{lines} per page.",
			 ch, (const void *) (d->dvdata->pagelen + 2), NULL,
			 TO_CHAR, POS_DEAD);
		return;
	}

	if (!is_number(arg)) {
		act_char("You must provide a number.", ch);
		return;
	}

	pagelen = atoi(arg);
	if (pagelen < MIN_PAGELEN || pagelen > MAX_PAGELEN) {
		act_puts3("Valid scroll range is $j..$J.",
			  ch, (const void *) MIN_PAGELEN, NULL,
			  (const void *) MAX_PAGELEN, TO_CHAR, POS_DEAD);
		return;
	}

	d->dvdata->pagelen = pagelen - 2;
	act_puts("Scroll set to $j lines.",
		 ch, (const void *) pagelen, NULL, TO_CHAR, POS_DEAD);
}

static void
show_social(CHAR_DATA *ch, CHAR_DATA *vch, const char *pre, mlstring *ml)
{
	if (!mlstr_null(ml)) {
		act("$t\n    ", ch, pre, NULL,			// notrans
		    TO_CHAR | ACT_NOLF);
		act_mlputs(ml, ch, NULL, vch, TO_CHAR, POS_DEAD);
	}
}

DO_FUN(do_socials, ch, argument)
{
	social_t *soc;
	CHAR_DATA *mob;
	MOB_INDEX_DATA *mob_index;

	if (argument[0] == '\0') {
		act_char("Use social <name> to view what that social does.", ch);
		dofun("alist", ch, "social");
		return;
	}

	if ((soc = social_search(argument)) == NULL) {
		act_char("There is no such social.", ch);
		return;
	}

	do {
		mob_index = get_mob_index(number_range(1, top_vnum_mob));
		// XXX MPC ACT should skip mobs with act triggers :)
	} while (mob_index == NULL);

	/*
	 * create_mob can't return NULL here
	 */
	mob = create_mob(mob_index->vnum, 0);

	show_social(ch, mob,
	   "Having used with no argument specified, you see:",
	   &soc->noarg_char);
	show_social(ch, mob, "And others see:", &soc->noarg_room);
	show_social(ch, mob,
	    "Having targeted yourself, you see:",
	    &soc->self_char);
	show_social(ch, mob, "And others see:", &soc->self_room);
	show_social(ch, mob,
	    "If your target is missing, you will see:",
	    &soc->notfound_char);
	show_social(ch, mob,
	    "Having targeted it to another character, you see:",
	    &soc->found_char);
	show_social(ch, mob, "Your victim see:", &soc->found_vict);
	show_social(ch, mob, "And others see:", &soc->found_notvict);

	extract_char(mob, 0);
}

/* RT Commands to replace news, motd, imotd, etc from ROM */
DO_FUN(do_motd, ch, argument)
{
	do_help(ch, "motd");
}

DO_FUN(do_imotd, ch, argument)
{
	do_help(ch, "imotd");
}

DO_FUN(do_rules, ch, argument)
{
	do_help(ch, "rules");
}

DO_FUN(do_wizlist, ch, argument)
{
	do_help(ch, "wizlist");
}

static toggle_t auto_table[] = {
	{ "assist",
	  "autoassist",
	  NULL, plr_flags, PLR_AUTOASSIST,
	  "You will now assist when needed.",
	  "Autoassist removed."
	},

	{ "exit",
	  "display exits",
	  NULL, plr_flags, PLR_AUTOEXIT,
	  "Exits will now be displayed.",
	  "Exits will no longer be displayed."
	},

	{ "gold",
	  "autoloot gold",
	  NULL, plr_flags, PLR_AUTOGOLD,
	  "Automatic gold looting set.",
	  "Autogold removed."
	},

	{ "look",
	  "autoexamine corpse",
	  NULL, plr_flags, PLR_AUTOLOOK,
	  "Automatic corpse examination set.",
	  "Autolooking removed."
	},

	{ "loot",
	  "autoloot corpse",
	  NULL, plr_flags, PLR_AUTOLOOT,
	  "Automatic corpse looting set.",
	  "Autolooting removed."
	},

	{ "sac",
	  "autosacrifice corpse",
	  NULL, plr_flags, PLR_AUTOSAC,
	  "Automatic corpse sacrificing set.",
	  "Autosacrificing removed."
	},

	{ "split",
	  "autosplit gold",
	  NULL, plr_flags, PLR_AUTOSPLIT,
	  "Automatic gold splitting set.",
	  "Autosplitting removed."
	},

	{ NULL, NULL, NULL, NULL, 0, NULL, NULL }
};

DO_FUN(do_auto, ch, argument)
{
	if (IS_NPC(ch))
		return;

	if (argument[0] == '\0') {
		act_char("Auto flags are:", ch);
		print_toggles(ch, auto_table);

		if (IS_SET(PC(ch)->plr_flags, PLR_NOSUMMON))
			act_char("You can only be summoned players within your PK range.",ch);
		else
			act_char("You can be summoned by anyone.", ch);

		if (IS_SET(PC(ch)->plr_flags, PLR_NOGIVE))
			act_char("You do not take any given object.", ch);
		else
			act_char("You will take all objects given to you.", ch);

		if (IS_SET(PC(ch)->plr_flags, PLR_NOFOLLOW))
			act_char("You do not welcome followers.", ch);
		else
			act_char("You accept followers.", ch);

		return;
	}

	toggle(ch, argument, auto_table);
}

DO_FUN(do_prompt, ch, argument)
{
	const char *prompt;
	DESCRIPTOR_DATA *d;

	if ((d = ch->desc) == NULL)
		return;

	if (IS_SET(ch->comm, COMM_AFK))
		dofun("afk", ch, str_empty);

	if (argument[0] == '\0') {
		bust_a_prompt(d);
		send_to_char("\n", ch);
		return;
	}

	if (!str_prefix(argument, "show")) {
		act_puts("Current prompt is '$t'.",
			 ch, d->dvdata->prompt, NULL,
			 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
		return;
	}

	if (!str_cmp(argument, "all") || !str_cmp(argument, "default"))
		prompt = str_dup(DEFAULT_PROMPT);
	else
		prompt = str_printf("%s ", argument);

	free_string(d->dvdata->prompt);
	d->dvdata->prompt = prompt;
	act_puts("Prompt set to '$t'.", ch, d->dvdata->prompt, NULL,
		 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
}

DO_FUN(do_nogive, ch, argument)
{
	if (IS_NPC(ch)) {
		act_char("Huh?", ch);
		return;
	}

	TOGGLE_BIT(PC(ch)->plr_flags, PLR_NOGIVE);
	if (IS_SET(PC(ch)->plr_flags, PLR_NOGIVE))
		act_char("You no longer take any given objects.", ch);
	else
		act_char("You will take any given objects.", ch);
}

DO_FUN(do_nofollow, ch, argument)
{
	if (IS_NPC(ch)) {
		act_char("Huh?", ch);
		return;
	}

	TOGGLE_BIT(PC(ch)->plr_flags, PLR_NOFOLLOW);
	if (IS_SET(PC(ch)->plr_flags,PLR_NOFOLLOW)) {
		act_char("You no longer accept followers.", ch);
		die_follower(ch);
	} else
		act_char("You now accept followers.", ch);
}

DO_FUN(do_nosummon, ch, argument)
{
	if (IS_NPC(ch)) {
		act_char("Huh?", ch);
	} else {
		TOGGLE_BIT(PC(ch)->plr_flags, PLR_NOSUMMON);
		if (IS_SET(PC(ch)->plr_flags,PLR_NOSUMMON))
			act_char("You may only be summoned by players within your PK range.", ch);
		else
			act_char("You may now be summoned by anyone.", ch);
	}
}

static DO_FUN(do_look_in, ch, argument)
{
	OBJ_DATA *obj;
	liquid_t *lq;
	clan_t *clan;

	if ((obj = get_obj_here(ch, argument)) == NULL) {
		act_char("You don't see that here.", ch);
		return;
	}

	switch (obj->pObjIndex->item_type) {
	default:
		act_char("That is not a container.", ch);
		break;
	case ITEM_DRINK_CON:
		if ((lq = liquid_lookup(STR(obj->value[2]))) == NULL) {
			printlog(LOG_BUG, "Do_look_in: bad liquid %s.", STR(obj->value[2]));
			break;
		}
		if (INT(obj->value[1]) == 0) {
			act_char("It is empty.", ch);
			break;
		}

		act_puts3("It's $ufilled with a $V liquid.",
			  ch,
			  INT(obj->value[1]) < 0 ?
				"" :
			  INT(obj->value[1]) < INT(obj->value[0]) / 4 ?
				"less than half-" :
			  INT(obj->value[1]) < 3 * INT(obj->value[0]) / 4 ?
			 	"about half-" :
			 	"more than half-",
			  obj, &lq->lq_color, TO_CHAR, POS_DEAD);
		break;

	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
	case ITEM_CORPSE_PC:
		if (IS_SET(INT(obj->value[1]), CONT_CLOSED)
		&&  ((clan = clan_lookup(ch->clan)) == NULL ||
		      clan->altar_ptr != obj)) {
			act("It is closed.", ch, obj, NULL, TO_CHAR);
			break;
		}

		act_puts("$p holds:", ch, obj, NULL, TO_CHAR, POS_DEAD);
		show_list_to_char(
		    obj->contains, ch, FO_F_SHORT | FO_F_SHOW_NOTHING);
		break;
	}
}

#define LOOK_F_NORDESC	(A)

static void
do_look_room(CHAR_DATA *ch, int flags)
{
	if (!char_in_dark_room(ch)
	&&  check_blind_nomessage(ch)) {
		const char *name;
		const char *engname;

		name = mlstr_cval(&ch->in_room->name, ch);
		engname = mlstr_mval(&ch->in_room->name);
		act_puts("{W$t", ch, name, NULL,		// notrans
			 TO_CHAR | ACT_NOTRANS | ACT_NOLF, POS_DEAD);
		if (GET_LANG(ch) && name != engname) {
			act_puts(" ($t){x", ch, engname, NULL,	// notrans
				 TO_CHAR | ACT_NOTRANS | ACT_NOLF, POS_DEAD);
		} else {
			act_puts("{x", ch, NULL, NULL,
				 TO_CHAR | ACT_NOTRANS | ACT_NOLF, POS_DEAD);
		}

		if (IS_IMMORTAL(ch)
		||  IS_BUILDER(ch, ch->in_room->area)) {
			act_puts(" [Room $j]", ch,
				 (const void *) ch->in_room->vnum, NULL,
				 TO_CHAR | ACT_NOLF, POS_DEAD);
		}

		act_puts("\n", ch, NULL, NULL,
			 TO_CHAR | ACT_NOTRANS | ACT_NOLF, POS_DEAD);

		if (!IS_SET(flags, LOOK_F_NORDESC)) {
			act_puts("  ", ch, NULL, NULL,		// notrans
				 TO_CHAR | ACT_NOTRANS | ACT_NOLF, POS_DEAD);
			act_puts(mlstr_cval(&ch->in_room->description, ch),
				 ch, NULL, NULL, TO_CHAR | ACT_NOLF, POS_DEAD);
		}

		if (!IS_NPC(ch) && IS_SET(PC(ch)->plr_flags, PLR_AUTOEXIT)) {
			act_puts("\n", ch, NULL, NULL,
				 TO_CHAR | ACT_NOTRANS | ACT_NOLF, POS_DEAD);
			do_exits(ch, "auto");
		}
	} else
		act_char("It is pitch black...", ch);

	show_list_to_char(ch->in_room->contents, ch, 0);
	show_char_to_char(ch->in_room->people, ch);
}

DO_FUN(do_glance, ch, argument)
{
	do_look_room(ch, LOOK_F_NORDESC);
}

static void
look_obj(CHAR_DATA *ch, OBJ_DATA *obj, ED_DATA *ed, bool show_desc)
{
	if (ed != NULL) {
		act_puts(mlstr_cval(&ed->description, ch),
			 ch, NULL, NULL,
			 TO_CHAR | ACT_NOLF, POS_DEAD);
	} else if (show_desc) {
		act_puts(format_long(&obj->description, ch),
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
	} else if (!OBJ_HAS_TRIGGER(obj, TRIG_OBJ_LOOK))
		act_char("You see nothing special about it.", ch);

	pull_obj_trigger(TRIG_OBJ_LOOK, obj, ch, NULL);
}

DO_FUN(do_look, ch, argument)
{
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	char arg3 [MAX_INPUT_LENGTH];
	EXIT_DATA *pexit;
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	ED_DATA *ed;
	int door;
	int number, count;

	if (ch->desc == NULL)
		return;

	if (ch->position < POS_SLEEPING) {
		act_char("You can't see anything but stars!", ch);
		return;
	}

	if (ch->position == POS_SLEEPING) {
		act_char("You can't see anything, you're sleeping!", ch);
		return;
	}

	argument = one_argument(argument, arg1, sizeof(arg1));
	argument = one_argument(argument, arg2, sizeof(arg2));
	number = number_argument(arg1, arg3, sizeof(arg3));
	count = 0;

	if (arg1[0] == '\0' || !str_cmp(arg1, "auto")) {
		/* 'look' or 'look auto' */
		do_look_room(
		    ch, arg1[0] == '\0' || (!IS_NPC(ch) && !IS_SET(ch->comm, COMM_BRIEF)) ?  0 : LOOK_F_NORDESC);
		return;
	}

	if (!check_blind(ch))
		return;

	if (!str_cmp(arg1, "i")
	||  !str_cmp(arg1, "in")
	||  !str_cmp(arg1, "on")) {
		/* 'look in' */
		if (arg2[0] == '\0') {
			act_char("Look in what?", ch);
			return;
		}

		do_look_in(ch, arg2);
		return;
	}

	if ((victim = get_char_here(ch, arg1)) != NULL) {
		look_char(ch, victim);
		return;
	}

	for (obj = ch->carrying; obj != NULL; obj = obj->next_content) {
		if (!can_see_obj(ch, obj))
			continue;

		/* player can see object */
		ed = ed_lookup(arg3, obj->ed);
		if (ed != NULL) {
			if (++count == number) {
				look_obj(ch, obj, ed, FALSE);
				return;
			}

			continue;
		}

		ed = ed_lookup(arg3, obj->pObjIndex->ed);
		if (ed != NULL) {
			if (++count == number) {
				look_obj(ch, obj, ed, FALSE);
				return;
			}

			continue;
		}

		if (IS_OBJ_NAME(obj, arg3)) {
			if (++count == number) {
				look_obj(ch, obj, NULL, FALSE);
				return;
			}
		}
	}

	for (obj = ch->in_room->contents;
	     obj != NULL; obj = obj->next_content) {
		if (!can_see_obj(ch, obj))
			continue;

		ed = ed_lookup(arg3, obj->ed);
		if (ed != NULL) {
			if (++count == number) {
				look_obj(ch, obj, ed, TRUE);
				return;
			}

			continue;
		}

		ed = ed_lookup(arg3, obj->pObjIndex->ed);
		if (ed != NULL) {
			if (++count == number) {
				look_obj(ch, obj, ed, TRUE);
				return;
			}

			continue;
		}

		if (IS_OBJ_NAME(obj, arg3)) {
			if (++count == number) {
				look_obj(ch, obj, ed, TRUE);
				return;
			}
		}
	}

	ed = ed_lookup(arg3, ch->in_room->ed);
	if (ed != NULL) {
		if (++count == number) {
			act_puts(mlstr_cval(&ed->description, ch),
				 ch, NULL, NULL, TO_CHAR | ACT_NOLF, POS_DEAD);
			return;
		}
	}

	if (count > 0 && count != number) {
		if (count == 1)
			act_puts("You only see one $t here.",
				 ch, arg3, NULL, TO_CHAR, POS_DEAD);
		else
			act_puts("You only see $j of those here.",
				 ch, (const void*) count, NULL,
				 TO_CHAR, POS_DEAD);
		return;
	}

	if ((door = find_door_nomessage(ch, arg1)) < 0)
		return;

	/* 'look direction' */
	if ((pexit = ch->in_room->exit[door]) == NULL) {
		act_char("Nothing special there.", ch);
		return;
	}

	if (!IS_NULLSTR(mlstr_mval(&pexit->description)))
		act_puts(mlstr_cval(&pexit->description, ch),
			 ch, NULL, NULL, TO_CHAR | ACT_NOLF, POS_DEAD);
	else
		act_char("Nothing special there.", ch);

	if (IS_SET(pexit->exit_info, EX_ISDOOR)) {
		if (IS_SET(pexit->exit_info, EX_CLOSED)) {
			act_puts("$d is closed.",
				 ch, &pexit->short_descr, NULL,
				 TO_CHAR, POS_DEAD);
		} else if (IS_SET(pexit->exit_info, EX_ISDOOR)) {
			act_puts("$d is open.",
				 ch, &pexit->short_descr,
				 NULL, TO_CHAR, POS_DEAD);
		}
	}
}

DO_FUN(do_examine, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;

	one_argument(argument, arg, sizeof(arg));

	if (ch->desc == NULL)
		return;

	if (ch->position < POS_SLEEPING) {
		act_char("You can't see anything but stars!", ch);
		return;
	}

	if (ch->position == POS_SLEEPING) {
		act_char("You can't see anything, you're sleeping!", ch);
		return;
	}

	if (!check_blind(ch))
		return;

	if (arg[0] == '\0') {
		act_char("Examine what?", ch);
		return;
	}

	do_look(ch, arg);
	if (IS_EXTRACTED(ch))
		return;

	if ((obj = get_obj_here(ch, arg)) == NULL)
		return;

	switch (obj->pObjIndex->item_type) {
	case ITEM_MONEY: {
		const char *msg;

		if (INT(obj->value[0]) == 0) {
			if (INT(obj->value[1]) == 0)
				msg = "Odd...there's no coins in the pile.";
			else if (INT(obj->value[1]) == 1)
				msg = "Wow. One gold coin.";
			else
				msg = "There are $J $qJ{gold coins} in this pile.";
		} else if (INT(obj->value[1]) == 0) {
			if (INT(obj->value[0]) == 1)
				msg = "Wow. One silver coin.";
			else
				msg = "There are $j $qj{silver coins} in the pile.";
		} else {
			msg = "There are $J gold and $j $qj{silver coins} in the pile.";
		}
		act_puts3(msg, ch,
			  (const void*) INT(obj->value[0]), NULL,
			  (const void*) INT(obj->value[1]),
			  TO_CHAR, POS_DEAD);
		break;
	}

	case ITEM_DRINK_CON:
	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
	case ITEM_CORPSE_PC:
		do_look_in(ch, argument);
		break;
	}
}

/*
 * Thanks to Zrin for auto-exit part.
 */
DO_FUN(do_exits, ch, argument)
{
	EXIT_DATA *pexit;
	bool found;
	bool fAuto;
	bool check_perception = FALSE;
	int door;
	BUFFER *buf;

	fAuto  = !str_cmp(argument, "auto");
	buf = buf_new(GET_LANG(ch));

	if (fAuto)
		buf_append(buf, "{C[Exits:");
	else if (IS_IMMORTAL(ch) || IS_BUILDER(ch, ch->in_room->area)) {
		buf_printf(buf, BUF_END, "Obvious exits from room %d:\n",
			   ch->in_room->vnum);
	} else
		buf_append(buf, "Obvious exits:\n");

	found = FALSE;
	for (door = 0; door < MAX_DIR; door++) {
		if ((pexit = ch->in_room->exit[door]) != NULL
		&&  pexit->to_room.r != NULL
		&&  can_see_room(ch, pexit->to_room.r)
		&&  check_blind_nomessage(ch)) {
			bool show_closed = FALSE;

			if (IS_SET(pexit->exit_info, EX_CLOSED)) {
				int chance;

				if (IS_IMMORTAL(ch))
					show_closed = TRUE;
				else if ((chance = get_skill(ch, "perception"))){
					if (number_percent() < chance) {
						check_perception = TRUE;
						show_closed = TRUE;
					}
				}

				if (!show_closed)
					continue;
			}

			found = TRUE;
			if (fAuto) {
				buf_printf(buf, BUF_END,
				    " %s%s",			    // notrans
				    dir_name[door],
				    show_closed ? "*" : str_empty); // notrans
			} else {
				buf_printf(buf, BUF_END,
				    "{C%-5s%s{x - %s",		// notrans
				    capitalize(dir_name[door]),
				    show_closed ?
					"*" : str_empty,	// notrans
					room_is_dark(pexit->to_room.r) ?
					    GETMSG("Too dark to tell", GET_LANG(ch)) :
					    mlstr_cval(&pexit->to_room.r->name, ch));

				if (IS_IMMORTAL(ch)
				||  IS_BUILDER(ch, pexit->to_room.r->area)) {
					buf_printf(buf, BUF_END, " (room %d)",
						   pexit->to_room.r->vnum);
				}
				buf_append(buf, "\n");
			}
		}
	}

	if (!found) {
		if (fAuto) {
			buf_append(buf, " none");
		} else
			buf_append(buf, "None.\n");
	}

	if (fAuto)
		buf_append(buf, "]{x\n");		// notrans

	act_puts("$t", ch, buf_string(buf), NULL,
		 TO_CHAR | ACT_NOTRANS | ACT_NOLF, POS_DEAD);
	buf_free(buf);

	if (check_perception)
		check_improve(ch, "perception", TRUE, 5);
}

DO_FUN(do_worth, ch, argument)
{
	act_puts3("You have $j gold, $J silver",
		  ch, (const void *) ch->gold, NULL, (const void *) ch->silver,
		  TO_CHAR | ACT_NOLF, POS_DEAD);
	if (!IS_NPC(ch)) {
		act_puts(", and $j experience",
			 ch, (const void *) PC(ch)->exp, NULL,
			 TO_CHAR | ACT_NOLF, POS_DEAD);
		if (ch->level < LEVEL_HERO) {
			int etl = exp_to_level(ch);

			act_puts(" ($j exp to level)",
				 ch, (const void *) etl, NULL,
				 TO_CHAR | ACT_NOLF, POS_DEAD);
		}
	}
	act_puts(".", ch, NULL, NULL, TO_CHAR, POS_DEAD);	// notrans

	if (!IS_NPC(ch)) {
		act_puts("You have killed $j $T",
			 ch, (const void*) PC(ch)->has_killed,
			 IS_GOOD(ch) ? "non-goods" :
			 IS_EVIL(ch) ? "non-evils" : 
				       "non-neutrals",
			 TO_CHAR | ACT_NOLF, POS_DEAD);
		act_puts(" and $j $T.",
			 ch, (const void*) PC(ch)->anti_killed,
			 IS_GOOD(ch) ? "goods" :
			 IS_EVIL(ch) ? "evils" : 
				       "neutrals",
			 TO_CHAR, POS_DEAD);
	}
}

static const char* day_name[] =
{
	"the Moon",			// notrans
	"the Bull",			// notrans
	"Deception",			// notrans
	"Thunder",			// notrans
	"Freedom",			// notrans
	"the Great Gods",		// notrans
	"the Sun"			// notrans
};

static const char* month_name[] =
{
	"Winter",			// notrans
	"the Winter Wolf",		// notrans
	"the Frost Giant",		// notrans
	"the Old Forces",		// notrans
	"the Grand Struggle",		// notrans
	"the Spring",			// notrans
	"Nature",			// notrans
	"Futility",			// notrans
	"the Dragon",			// notrans
	"the Sun",			// notrans
	"the Heat",			// notrans
	"the Battle",			// notrans
	"the Dark Shades",		// notrans
	"the Shadows",			// notrans
	"the Long Shadows",		// notrans
	"the Ancient Darkness",		// notrans
	"the Great Evil"		// notrans
};

DO_FUN(do_time, ch, argument)
{
	int day = time_info.day + 1;
	int hour;

	hour = time_info.hour % 12;
	if (!hour)
		hour = 12;
	act_puts("It is $j o'clock $T, ",
		 ch, (const void *) hour, time_info.hour >= 12 ? "pm" : "am",
		 TO_CHAR | ACT_NOLF, POS_DEAD);
	act_puts3("Day of $t, $J$qJ{th} the Month of $T.",
		  ch, day_name[day % 7], month_name[time_info.month],
		  (const void *) day, TO_CHAR | ACT_NOTRANS, POS_DEAD);

	if (!IS_SET(ch->in_room->room_flags, ROOM_INDOORS) || IS_IMMORTAL(ch)) {
		act_puts("It's $T.", ch, NULL,
			 time_info.hour >=  5 && time_info.hour < 9 ?
				"dawn"	  :
			 time_info.hour >=  9 && time_info.hour < 12 ?
				"morning" :
			 time_info.hour >= 12 && time_info.hour < 18 ?
				"mid-day" :
			 time_info.hour >= 18 && time_info.hour < 21 ?
				"evening" :
				"night",
			TO_CHAR, POS_DEAD);
	}

	if (!IS_IMMORTAL(ch))
		return;

	send_to_char("\n", ch);
	act_puts("SoG started up at $t.",
		 ch, strtime(boot_time), NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
	act_puts("The system time is $t.",
		 ch, strtime(time(NULL)), NULL,
		 TO_CHAR | ACT_NOTRANS, POS_DEAD);
	act_puts("Reboot in $j $qj{minutes}.",
		 ch, (const void *) reboot_counter, NULL, TO_CHAR, POS_DEAD);
}

DO_FUN(do_date, ch, argument)
{
	act_puts("$t", ch, strtime(time(NULL)), NULL,
		 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
}

DO_FUN(do_weather, ch, argument)
{
	static const char *sky_look[4] = {
		"cloudless",
		"cloudy",
		"rainy",
		"lit by flashes of lightning"
	};

	if (!IS_OUTSIDE(ch)) {
		act_char("You can't see the weather indoors.", ch);
		return;
	}

	act_puts("The sky is $t and $T.",
		 ch, sky_look[weather_info.sky],
		 weather_info.change >= 0 ?
		 	"a warm southerly breeze blows" :
		 	"a cold northern gust blows",
		 TO_CHAR, POS_DEAD);
}

DO_FUN(do_help, ch, argument)
{
	BUFFER *output;
	output = buf_new(GET_LANG(ch));
	help_show(ch, output, argument);
	page_to_char(buf_string(output), ch);
	buf_free(output);
}

#define WHO_F_IMM	(A)		/* imm only			*/
#define WHO_F_PK	(B)		/* PK only			*/
#define WHO_F_TATTOO	(C)		/* same tattoo only		*/
#define WHO_F_CLAN	(D)		/* clan only			*/
#define WHO_F_RCLAN	(E)		/* specified clans only		*/
#define WHO_F_RRACE	(F)		/* specified races only		*/
#define WHO_F_RCLASS	(G)		/* specified classes only	*/

DO_FUN(do_who, ch, argument)
{
	BUFFER *output;
	DESCRIPTOR_DATA *d;
	flag_t flags = 0;
	flag_t ralign = 0;
	flag_t rethos = 0;

	int iLevelLower = 0;
	int iLevelUpper = MAX_LEVEL;

	int tattoo_vnum = 0;	/* who tattoo data */
	OBJ_DATA *obj;

	int nNumber;
	int count = 0;

	const char *clan_names = str_empty;
	const char *race_names = str_empty;
	const char *class_names = str_empty;
	char *p;

	/*
	 * Parse arguments.
	 */
	nNumber = 0;
	for (;;) {
		race_t *r;
		class_t *cl;
		clan_t *clan;
		char arg[MAX_INPUT_LENGTH];

		argument = one_argument(argument, arg, sizeof(arg));
		if (arg[0] == '\0')
			break;

		if (!str_prefix(arg, "immortals")) {
			SET_BIT(flags, WHO_F_IMM);
			continue;
		}

		if (!str_cmp(arg, "pk")) {
			SET_BIT(flags, WHO_F_PK);
			continue;
		}

		if (!str_cmp(arg, "tattoo")) {
			if ((obj = get_eq_char(ch, WEAR_TATTOO)) == NULL) {
				act_char("You haven't got a tattoo yet!", ch);
				goto bail_out;
			}
			SET_BIT(flags, WHO_F_TATTOO);
			tattoo_vnum = obj->pObjIndex->vnum;
			continue;
		}

		if (!str_cmp(arg, "clan")) {
			if (IS_IMMORTAL(ch))
				SET_BIT(flags, WHO_F_CLAN);
			else if (!IS_NULLSTR(ch->clan)) {
				SET_BIT(flags, WHO_F_RCLAN);
				name_add(&clan_names, ch->clan, NULL,NULL);
			}
			continue;
		}

		if (!IS_IMMORTAL(ch))
			continue;

		if ((clan = clan_search(arg)) != NULL) {
			name_add(&clan_names, clan->name, NULL, NULL);
			SET_BIT(flags, WHO_F_RCLAN);
			continue;
		}

		if ((r = race_search(arg)) != NULL && r->race_pcdata) {
			name_add(&race_names, r->name, NULL, NULL);
			SET_BIT(flags, WHO_F_RRACE);
			continue;
		}

		if ((cl = class_search(arg)) != NULL) {
			name_add(&class_names, cl->name, NULL, NULL);
			SET_BIT(flags, WHO_F_RCLASS);
			continue;
		}

		if ((p = strchr(arg, '-'))) {
			int i;

			*p++ = '\0';
			if (arg[0]) {
				if ((i = flag_value(ethos_table, arg)))
					SET_BIT(rethos, i);
				else {
					act_puts("$t: unknown ethos.",
						 ch, arg, NULL, TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
				}
			}
			if (*p) {
				if ((i = flag_value(ralign_names, p)))
					SET_BIT(ralign, i);
				else {
					act_puts("$t: unknown align.",
						 ch, p, NULL, TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
				}
			}
			continue;
		}

		if (is_number(arg)) {
			switch (++nNumber) {
			case 1:
				iLevelLower = atoi(arg);
				break;
			case 2:
				iLevelUpper = atoi(arg);
				break;
			default:
				act_puts("$t: explicit argument (skipped)",
					 ch, arg, NULL,
					 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE,
					 POS_DEAD);
				break;
			}
			continue;
		}
	}

	/*
	 * Now show matching chars.
	 */
	output = buf_new(GET_LANG(ch));
	for (d = descriptor_list; d; d = d->next) {
		CHAR_DATA *wch;

		clan_t *clan;

		if (d->connected != CON_PLAYING)
			continue;

		wch = d->original ? d->original : d->character;
		if (!wch || !can_see(ch, wch))
			continue;

		if (is_sn_affected(wch, "vampire")
		&&  !IS_IMMORTAL(ch) && ch != wch)
			continue;

		if (wch->level < iLevelLower || wch->level > iLevelUpper
		||  (IS_SET(flags, WHO_F_IMM) && wch->level < LEVEL_IMMORTAL)
		||  (IS_SET(flags, WHO_F_PK) && (IS_IMMORTAL(wch) ||
						 !in_PK(ch, wch)))
		||  (IS_SET(flags, WHO_F_CLAN) && IS_NULLSTR(wch->clan))
		||  (ralign && ((RALIGN(wch) & ralign) == 0))
		||  (rethos && ((PC(wch)->ethos & rethos) == 0)))
			continue;

		if (IS_SET(flags, WHO_F_TATTOO)) {
			if ((obj = get_eq_char(wch, WEAR_TATTOO)) == NULL
			||  tattoo_vnum != obj->pObjIndex->vnum)
				continue;
		}

		if (IS_SET(flags, WHO_F_RCLAN)) {
			if ((clan = clan_lookup(wch->clan)) == NULL
			||  !is_name(clan->name, clan_names))
				continue;
		}

		if (IS_SET(flags, WHO_F_RRACE)
		&&  !is_name_strict(PC(wch)->race, race_names))
			continue;

		if (IS_SET(flags, WHO_F_RCLASS)
		&&  !is_name_strict(wch->class, class_names))
			continue;

		count++;
		do_who_raw(ch, wch, output);
	}

	buf_printf(output, BUF_END, "{x\nPlayers found: %d. Most so far today: %d.\n",
		   count, top_player);
	page_to_char(buf_string(output), ch);
	buf_free(output);

bail_out:
	free_string(clan_names);
	free_string(class_names);
	free_string(race_names);
}

/* whois command */
DO_FUN(do_whois, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];
	BUFFER *output = NULL;
	DESCRIPTOR_DATA *d;
	int count = 0;

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		act_char("You must provide a name.", ch);
		return;
	}

	for (d = descriptor_list; d != NULL; d = d->next) {
		CHAR_DATA *wch;

		if (d->connected != CON_PLAYING
		||  !can_see(ch, d->character))
			continue;

		if (is_sn_affected(d->character, "vampire")
		&&  !IS_IMMORTAL(ch)
		&&  ch != d->character)
			continue;

		wch = (d->original != NULL) ? d->original : d->character;

		if (!can_see(ch, wch))
			continue;

		if (!str_prefix(arg, wch->name)) {
			if (output == NULL)
				output = buf_new(0);
			count++;
			do_who_raw(ch, wch, output);
		}
	}

	if (output == NULL) {
		act_char("No one of that name is playing.", ch);
		return;
	}

	buf_printf(output, BUF_END, "{x\nPlayers found: %d. Most so far today: %d.\n",
		   count, top_player);
	page_to_char(buf_string(output), ch);
	buf_free(output);
}

DO_FUN(do_inventory, ch, argument)
{
	act_char("You are carrying:", ch);
	show_list_to_char(ch->carrying, ch, FO_F_SHORT | FO_F_SHOW_NOTHING);
}

DO_FUN(do_equipment, ch, argument)
{
	OBJ_DATA *obj;
	int i;
	bool found;

	act_char("You are using:", ch);
	if (ch->shapeform) {
		/* Don't show empty slots, because we mean there is no
		 * slots at all
		 */
		act_char("Nothing.", ch);
		return;
	}

	found = FALSE;
	for (i = 0; show_order[i] >= 0; i++) {
		if ((obj = get_eq_char(ch, show_order[i])) == NULL
		&&  IS_SET(ch->comm, COMM_SHORT_EQ))
			continue;

		show_obj_to_char(ch, obj, show_order[i]);
		found = TRUE;
	}

	for(obj = ch->carrying; obj != NULL; obj = obj->next_content) {
		if (obj->wear_loc != WEAR_STUCK_IN)
			continue;

		show_obj_to_char(ch, obj, WEAR_STUCK_IN);
		found = TRUE;
	}

	if (!found)
		act_char("Nothing.", ch);
}

DO_FUN(do_compare, ch, argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	OBJ_DATA *obj1;
	OBJ_DATA *obj2;
	int value1;
	int value2;
	const char *cmsg;

	argument = one_argument(argument, arg1, sizeof(arg1));
	argument = one_argument(argument, arg2, sizeof(arg2));
	if (arg1[0] == '\0') {
		act_char("Compare what to what?", ch);
		return;
	}

	if ((obj1 = get_obj_carry(ch, ch, arg1)) == NULL) {
		act_char("You do not have that item.", ch);
		return;
	}

	if (arg2[0] == '\0') {
		for (obj2 = ch->carrying;
		     obj2 != NULL; obj2 = obj2->next_content)
			if (obj2->wear_loc != WEAR_NONE
			&&  can_see_obj(ch,obj2)
			&&  obj1->pObjIndex->item_type == obj2->pObjIndex->item_type
			&&  (obj1->wear_flags & obj2->wear_flags & ~ITEM_TAKE))
				break;

		if (obj2 == NULL) {
			act_char("You aren't wearing anything comparable.", ch);
			return;
		}
	} else if ((obj2 = get_obj_carry(ch, ch, arg2)) == NULL) {
		act_char("You do not have that item.", ch);
		return;
	}

	cmsg		= NULL;
	value1	= 0;
	value2	= 0;

	if (obj1 == obj2)
		cmsg = "You compare $p to itself.  It looks about the same.";
	else if (obj1->pObjIndex->item_type != obj2->pObjIndex->item_type)
		cmsg = "You can't compare $p and $P.";
	else {
		switch (obj1->pObjIndex->item_type) {
		default:
			cmsg = "You can't compare $p and $P.";
			break;

		case ITEM_ARMOR:
			value1 = INT(obj1->value[0]) +
				 INT(obj1->value[1]) +
				 INT(obj1->value[2]);
			value2 = INT(obj2->value[0]) +
				 INT(obj2->value[1]) +
				 INT(obj2->value[2]);
			break;

		case ITEM_WEAPON:
			value1 = GET_WEAPON_AVE(obj1);
			value2 = GET_WEAPON_AVE(obj2);
			break;
		}
	}

	if (cmsg == NULL) {
		if (value1 == value2)
			cmsg = "$p and $P look about the same.";
		else if (value1  > value2)
			cmsg = "$p looks better than $P.";
		else
			cmsg = "$p looks worse than $P.";
	}

	act(cmsg, ch, obj1, obj2, TO_CHAR);
}

DO_FUN(do_credits, ch, argument)
{
	do_help(ch, "DIKU");
	do_help(ch, "'SHADES OF GRAY'");
}

DO_FUN(do_where, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	DESCRIPTOR_DATA *d;
	bool found;
	bool fPKonly = FALSE;

	one_argument(argument, arg, sizeof(arg));

	if (!check_blind(ch))
		return;

	if (char_in_dark_room(ch)) {
		act_char("It's too dark to see.", ch);
		return;
	}

	if (!str_cmp(arg,"pk"))
		fPKonly = TRUE;

	if (arg[0] == '\0' || fPKonly) {
		act_char("Players near you:", ch);
		found = FALSE;
		for (d = descriptor_list; d; d = d->next) {
			victim = d->character;

			if (d->connected == CON_PLAYING
			&&  !IS_NPC(victim)
			&&  (!fPKonly || in_PK(ch, victim))
			&&  victim->in_room != NULL
			&&  victim->in_room->area == ch->in_room->area
			&&  !IS_SET(victim->in_room->room_flags, ROOM_NOWHERE)
			&&  can_see(ch, victim)) {
				CHAR_DATA *doppel;
				found = TRUE;

				if (is_sn_affected(victim, "doppelganger")
				&&  (IS_NPC(ch) || !IS_SET(PC(ch)->plr_flags, PLR_HOLYLIGHT)))
					doppel = victim->doppel;
				else
					doppel = victim;

				if (in_PK(ch, doppel)) {
					act_puts("{r[{RPK{r]{x ",
						 ch, NULL, NULL,
						 TO_CHAR | ACT_NOLF, POS_DEAD);
				} else {
					act_puts("     ",	// notrans
					    ch, NULL, NULL,
					    TO_CHAR | ACT_NOLF, POS_DEAD);
				}

				act_puts("$f-28{$N} $t", ch,	// notrans
					 mlstr_mval(&victim->in_room->name),
					 victim,
					 TO_CHAR | ACT_NOTRANS | ACT_FORMSH,
					 POS_DEAD);
			}
		}
		if (!found)
			act_char("None.", ch);
	} else {
		found = FALSE;
		for (victim = char_list; victim; victim = victim->next) {
			if (victim->in_room
			&&  victim->in_room->area == ch->in_room->area
			&&  can_see(ch, victim)
			&&  is_name(arg, victim->name)) {
				found = TRUE;
				act_puts("$f-28{$N} $t", ch,	// notrans
					 mlstr_mval(&victim->in_room->name),
					 victim,
					 TO_CHAR | ACT_NOTRANS | ACT_FORMSH,
					 POS_DEAD);
				break;
			}
		}

		if (!found) {
			act_puts("You didn't find any $T.", ch, NULL, arg,
				 TO_CHAR | ACT_NOTRANS, POS_DEAD);
		}
	}
}

DO_FUN(do_consider, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	const char *cmsg;
	const char *align;
	int diff;

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		act_char("Consider killing whom?", ch);
		return;
	}

	if ((victim = get_char_here(ch, arg)) == NULL) {
		act_char("They aren't here.", ch);
		return;
	}

	if (victim == ch) {
		act_char("Suicide is against your way.", ch);
		return;
	}

	if (!in_PK(ch, victim)) {
		act_char("Don't even think about it.", ch);
		return;
	}

	diff = victim->level - ch->level;

	     if (diff <= -10) cmsg = "You can kill $N naked and weaponless.";
	else if (diff <=  -5) cmsg = "$N is no match for you.";
	else if (diff <=  -2) cmsg = "$N looks like an easy kill.";
	else if (diff <=   1) cmsg = "The perfect match!";
	else if (diff <=   4) cmsg = "$N says '{GDo you feel lucky, punk?{x'.";
	else if (diff <=   9) cmsg = "$N laughs at you mercilessly.";
	else		      cmsg = "Death will thank you for your gift.";

	if (IS_EVIL(ch) && IS_EVIL(victim))
		align = "$N grins evilly with you.";
	else if (IS_GOOD(victim) && IS_GOOD(ch))
		align = "$N greets you warmly.";
	else if (IS_GOOD(victim) && IS_EVIL(ch))
		align = "$N smiles at you, hoping you will turn from your evil path.";
	else if (IS_EVIL(victim) && IS_GOOD(ch))
		align = "$N grins evilly at you.";
	else if (IS_NEUTRAL(ch) && IS_EVIL(victim))
		align = "$N grins evilly.";
	else if (IS_NEUTRAL(ch) && IS_GOOD(victim))
		align = "$N smiles happily.";
	else if (IS_NEUTRAL(ch) && IS_NEUTRAL(victim))
		align = "$N looks just as disinterested as you.";
	else
		align = "$N looks very disinterested.";

	act(cmsg, ch, NULL, victim, TO_CHAR);
	act(align, ch, NULL, victim, TO_CHAR);
}

DO_FUN(do_description, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];

	if (IS_NPC(ch)) {
		act_char("Huh?", ch);
		return;
	}

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		act_char("Your description is:", ch);
		act_puts("$t{x", 				// notrans
			 ch, mlstr_mval(&ch->description), NULL,
			 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
		act_char("Use 'desc edit' to edit your description.", ch);
		return;
	}

	if (!str_prefix(arg, "edit")) {
		string_append(ch, mlstr_convert(&ch->description, NULL));
		return;
	}

	do_description(ch, str_empty);
}

DO_FUN(do_report, ch, argument)
{
	char buf[MAX_INPUT_LENGTH];

	snprintf(buf, sizeof(buf), "I have %d/%d hp %d/%d mana %d/%d mv.",
		 ch->hit, ch->max_hit,
		 ch->mana, ch->max_mana,
		 ch->move, ch->max_move);
	dofun("say", ch, buf);
}

/*
 * 'Wimpy' originally by Dionysos.
 */
DO_FUN(do_wimpy, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];
	int wimpy;

	if (!can_flee(ch)) {
		act_char("You don't deal with wimpies, or such feary things.", ch);
		if (ch->wimpy)
			ch->wimpy = 0;
		return;
	}

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0')
		wimpy = ch->max_hit / 5;
	else
		wimpy = atoi(arg);

	if (wimpy < 0) {
		act_char("Your courage exceeds your wisdom.", ch);
		return;
	}

	if (wimpy > ch->max_hit/2) {
		act_char("Such cowardice ill becomes you.", ch);
		return;
	}

	ch->wimpy	= wimpy;

	act_puts("Wimpy set to $j hit points.",
		 ch, (const void *) wimpy, NULL, TO_CHAR, POS_DEAD);
}

DO_FUN(do_password, ch, argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	const char *pwdnew;

	if (IS_NPC(ch))
		return;

	argument = first_arg(argument, arg1, sizeof(arg1), FALSE);
	argument = first_arg(argument, arg2, sizeof(arg2), FALSE);

	if (arg1[0] == '\0' || arg2[0] == '\0') {
		act_char("Syntax: password <old> <new>.", ch);
		return;
	}

	if (strcmp(crypt(arg1, PC(ch)->pwd), PC(ch)->pwd)) {
		WAIT_STATE(ch, 10 * PULSE_PER_SECOND);
		act_char("Wrong password. Wait 10 seconds.", ch);
		return;
	}

	if (strlen(arg2) < 5) {
		act_char("New password must be at least five characters long.", ch);
		return;
	}

	/*
	 * No tilde allowed because of player file format.
	 */
	pwdnew = crypt(arg2, ch->name);
	if (strchr(pwdnew, '~') != NULL) {
		act_char("New password not acceptable, try again.", ch);
		return;
	}

	free_string(PC(ch)->pwd);
	PC(ch)->pwd = str_dup(pwdnew);
	char_save(ch, 0);
	act_char("Ok.", ch);
}

static void scan_list(ROOM_INDEX_DATA *scan_room, CHAR_DATA *ch)
{
	CHAR_DATA *rch;

	if (scan_room == NULL) 
		return;

	for (rch = scan_room->people; rch; rch = rch->next_in_room) {
		if (rch == ch || !can_see(ch, rch))
			continue;
		act_puts("    $N.", ch, NULL, rch,		// notrans
			 TO_CHAR | ACT_FORMSH, POS_DEAD);
	}
}

static void scan_all(CHAR_DATA *ch)
{
	EXIT_DATA *pExit;
	int door;

	act("$n looks all around.", ch, NULL, NULL, TO_ROOM);
	if (!check_blind(ch))
		return;

	act_char("Looking around you see:", ch);

	act_char("{Chere{x:", ch);
	scan_list(ch->in_room, ch);
	for (door = 0; door < 6; door++) {
		if ((pExit = ch->in_room->exit[door]) == NULL
		|| !pExit->to_room.r
		|| !can_see_room(ch,pExit->to_room.r))
			continue;
		act_puts("{C$t{x:", ch, dir_name[door], NULL,	// notrans
			 TO_CHAR, POS_DEAD);
		if (IS_SET(pExit->exit_info, EX_CLOSED)) {
			act_char("    You see closed door.", ch);
			continue;
		}
		scan_list(pExit->to_room.r, ch);
	}
}

DO_FUN(do_scan, ch, argument)
{
	char dir[MAX_INPUT_LENGTH];
	ROOM_INDEX_DATA *in_room;
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	int door;
	int range;
	int i;

	one_argument(argument, dir, sizeof(dir));
	if (strchr(dir, '.')) {
		range = number_argument(dir, dir, sizeof(dir));
		if (range > 1 + ch->level/10) {
			act("You cannot see that far.",
			    ch, NULL, NULL, TO_CHAR);
			return;
		}
	} else
		range = 1 + ch->level/10;

	switch (dir[0]) {
	case 'N':
	case 'n':
		door = 0;
		break;
	case 'E':
	case 'e':
		door = 1;
		break;
	case 'S':
	case 's':
		door = 2;
		break;
	case 'W':
	case 'w':
		door = 3;
		break;
	case 'U':
	case 'u':
		door = 4;
		break;
	case 'D':
	case 'd':
		door = 5;
		break;
	case '\0':
		if (IS_IMMORTAL(ch))
			scan_all(ch);
		else
			act("Scan which direction?", ch, NULL, NULL, TO_CHAR);
		return;
	default:
		act_char("Wrong direction.", ch);
		return;
	}

	act("$n scans $t.", ch, dir_name[door], NULL, TO_ROOM);
	if (!check_blind(ch))
		return;

	act_puts("You scan $t.", ch, dir_name[door], NULL, TO_CHAR, POS_DEAD);

	in_room = ch->in_room;
	for (i = 1; i <= range; i++) {
		CHAR_DATA *vch;

		if ((pexit = in_room->exit[door]) == NULL
		||  (to_room = pexit->to_room.r) == NULL)
			return;

		if (IS_SET(pexit->exit_info, EX_CLOSED)
		&&  can_see_room(ch, to_room)) {
			if (i == 1)
				act_char("	You see closed door.", ch);
			return;
		}

		for (vch = to_room->people; vch != NULL; vch = vch->next_in_room) {
			if (can_see(ch, vch))
				break;
		}

		if (vch) {
			act_puts("***** Range $j *****",
				 ch, (const void *) i, NULL, TO_CHAR, POS_DEAD);
			show_char_to_char(to_room->people, ch);
			send_to_char("\n", ch);
		}
		in_room = to_room;
	}
}

DO_FUN(do_request, ch, argument)
{
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA  *obj;
	AFFECT_DATA *paf;

	if (is_sn_affected(ch, "reserved")) {
		act_char("Wait for a while to request again.", ch);
		return;
	}

	argument = one_argument(argument, arg1, sizeof(arg1));
	argument = one_argument(argument, arg2, sizeof(arg2));

	if (IS_NPC(ch))
		return;

	if (arg1[0] == '\0' || arg2[0] == '\0') {
		act_char("Request what from whom?", ch);
		return;
	}

	if ((victim = get_char_here(ch, arg2)) == NULL) {
		act_char("They aren't here.", ch);
		return;
	}

	if (!IS_NPC(victim)) {
		act_char("Why don't you just ask the player?", ch);
		return;
	}

	if (!IS_GOOD(ch)) {
		do_say(victim,
		       "I will not give anything to someone so impure.");
		return;
	}

	if (ch->move < (50 + ch->level)) {
		do_say(victim, "You look rather tired, "
			       "why don't you rest a bit first?");
		return;
	}

	WAIT_STATE(ch, get_pulse("violence"));
	ch->move -= 10;
	ch->move = UMAX(ch->move, 0);

	if (victim->level >= ch->level + 10 || victim->level >= ch->level * 2) {
		do_say(victim, "In good time, my child");
		return;
	}

	if (((obj = get_obj_carry(victim, victim, arg1)) == NULL &&
	     (obj = get_obj_wear(victim, victim, arg1)) == NULL)
	||  IS_OBJ_STAT(obj, ITEM_INVENTORY)) {
		do_say(victim, "Sorry, I don't have that.");
		return;
	}

	if (!IS_GOOD(victim)) {
		do_say(victim, "I'm not about to give you anything!");
		do_murder(victim, ch->name);
		return;
	}

	if (!can_drop_obj(victim, obj)) {
		do_say(victim, "Sorry, I can't let go of it. It's cursed.");
		return;
	}

	if (!can_carry_more_n(ch, get_obj_number(obj))) {
		act_char("Your hands are full.", ch);
		return;
	}

	if (!can_carry_more_w(ch, get_obj_weight(obj))) {
		act_char("You can't carry that much weight.", ch);
		return;
	}

	if (!can_see_obj(ch, obj)) {
		act("You don't see that.", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (OBJ_IS(obj, OBJ_QUIT_DROP)) {
		do_say(victim, "Sorry, I must keep it myself.");
		return;
	}

	if (obj->wear_loc != WEAR_NONE
	&&  !remove_obj(victim, obj->wear_loc, TRUE)) {
		do_say(victim, "Sorry, I can't remove it.");
		return;
	}

	obj_to_char(obj, ch);

	act("$n requests $p from $N.", ch, obj, victim, TO_NOTVICT);
	act("You request $p from $N.",	 ch, obj, victim, TO_CHAR);
	act("$n requests $p from you.", ch, obj, victim, TO_VICT);

	pull_obj_trigger(TRIG_OBJ_GIVE, obj, victim, ch);

	if (!IS_EXTRACTED(ch)) {
		ch->move -= (50 + ch->level);
		ch->move = UMAX(ch->move, 0);
		ch->hit -= 3 * (ch->level / 2);
		ch->hit = UMAX(ch->hit, 0);

		paf = aff_new(TO_AFFECTS, "reserved");
		paf->level = ch->level;
		paf->duration = ch->level / 10;
		affect_to_char(ch, paf);
		aff_free(paf);

		if (!IS_EXTRACTED(victim)) {
			act("You feel grateful for the trust of $N.",
			    ch, NULL, victim, TO_CHAR);
			act_char("and for the goodness you have seen in the world.", ch);
		}
	}
}

DO_FUN(do_hometown, ch, argument)
{
	int amount;
	int htn;
	PC_DATA *pc;

	if (IS_NPC(ch)) {
		act_puts("You can't change your hometown!",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		return;
	}

	if (!IS_SET(ch->in_room->room_flags, ROOM_REGISTRY)) {
		act_puts("You have to be in the Registry "
			 "to change your hometown.",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		return;
	}

	if ((htn = hometown_permanent(ch)) >= 0) {
		act_puts("Your hometown is $t, permanently. "
			 "You can't change your hometown.",
			 ch, hometown_name(htn), NULL,
			 TO_CHAR, POS_DEAD);
		return;
	}

	amount = (ch->level * 250) + 1000;

	if (argument[0] == '\0') {
		act_puts("The change of hometown will cost you $j gold.",
			 ch, (const void*) amount, NULL, TO_CHAR, POS_DEAD);
		act_puts("Choose from: ", ch, NULL, NULL,
			 TO_CHAR | ACT_NOLF, POS_DEAD);
		hometown_print_avail(ch);
		send_to_char(".\n", ch);
		return;
	}

	if ((htn = htn_lookup(argument)) < 0) {
		act_puts("That's not a valid hometown.",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		return;
	}

	pc = PC(ch);
	if (htn == pc->hometown) {
		act_puts("But you already live in $t!",
			 ch, hometown_name(htn), NULL,
			 TO_CHAR, POS_DEAD);
		return;
	}

	if (hometown_restrict(HOMETOWN(htn), ch)) {
		act_puts("You are not allowed there.",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		return;
	}

	if (pc->bank_g < amount) {
		act_puts("You don't have enough money in bank "
			 "to change hometowns!",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		return;
	}

	pc->hometown = htn;
	pc->bank_g -= amount;
	act_puts("Now your hometown is $t.",
		 ch, hometown_name(pc->hometown),
		 NULL, TO_CHAR, POS_DEAD);
}

DO_FUN(do_detect_hidden, ch, argument)
{
	AFFECT_DATA *paf;

	if (HAS_DETECT(ch, ID_HIDDEN)) {
		act_char("You are already as alert as you can be.", ch);
		return;
	}

	if (number_percent() > get_skill(ch, "detect hide")) {
		act_char("You peer intently at the shadows but they are unrevealing.", ch);
		check_improve(ch, "detect hide", FALSE, 1);
		return;
	}

	paf = aff_new(TO_DETECTS, "detect hide");
	paf->level     = LEVEL(ch);
	paf->duration  = LEVEL(ch);
	paf->bitvector = ID_HIDDEN;
	affect_to_char(ch, paf);
	aff_free(paf);

	act_char("Your awareness improves.", ch);
	check_improve(ch, "detect hide", TRUE, 1);
}

DO_FUN(do_awareness, ch, argument)
{
	AFFECT_DATA	*paf;

	if (is_sn_affected(ch, "forest awareness")) {
		act_char("You are already as alert as you can be.", ch);
		return;
	}

	if (number_percent() > get_skill(ch, "forest awareness")) {
		act_char("You peer intently at the shadows but they are unrevealing.", ch);
		check_improve(ch, "forest awareness", FALSE, 1);
		return;
	}

	paf = aff_new(TO_DETECTS, "forest awareness");
	paf->level     = LEVEL(ch);
	paf->duration  = LEVEL(ch);
	paf->bitvector = ID_BLEND | ID_CAMOUFLAGE;
	affect_to_char(ch, paf);
	aff_free(paf);

	act_char("Your awareness improves.", ch);
	check_improve(ch, "forest awareness", TRUE, 1);
}

#define MOB_VNUM_BEAR			12

DO_FUN(do_bear_call, ch, argument)
{
	CHAR_DATA *	gch;
	CHAR_DATA *	bear;
	CHAR_DATA *	bear2;
	AFFECT_DATA	*paf;
	int		i;
	int		mana;

	act_char("You call for bears help you.", ch);
	act("$n shouts a bear call.",ch,NULL,NULL,TO_ROOM);

	if (is_sn_affected(ch, "bear call")) {
		act_char("You cannot summon the strength to handle more bears right now.", ch);
		return;
	}

	for (gch = npc_list; gch; gch = gch->next) {
		if (IS_AFFECTED(gch, AFF_CHARM)
		&&  gch->master == ch
		&&  gch->pMobIndex->vnum == MOB_VNUM_BEAR) {
			act_char("What's wrong with the bear you've got?", ch);
			return;
		}
	}

	if (ch->in_room != NULL
	&&  IS_SET(ch->in_room->room_flags, ROOM_NOMOB)) {
		act_char("No bears listen you.", ch);
		return;
	}

	if (IS_SET(ch->in_room->room_flags, ROOM_PEACE |
					    ROOM_PRIVATE | ROOM_SOLITARY)
	||  (ch->in_room->exit[0] == NULL && ch->in_room->exit[1] == NULL
	&&   ch->in_room->exit[2] == NULL && ch->in_room->exit[3] == NULL
	&&   ch->in_room->exit[4] == NULL && ch->in_room->exit[5] == NULL)
	||  (ch->in_room->sector_type != SECT_FIELD
	&&   ch->in_room->sector_type != SECT_FOREST
	&&   ch->in_room->sector_type != SECT_MOUNTAIN
	&&   ch->in_room->sector_type != SECT_HILLS)) {
		act_char("No bears come to your rescue.", ch);
		return;
	}

	mana = skill_mana(ch, "bear call");
	if (ch->mana < mana) {
		act_char("You don't have enough mana to shout a bear call.", ch);
		return;
	}
	ch->mana -= mana;

	if (number_percent() > get_skill(ch, "bear call")) {
		act_char("No bears listen you.", ch);
		check_improve(ch, "bear call", FALSE, 1);
		return;
	}

	check_improve(ch, "bear call", TRUE, 1);
	bear = create_mob(MOB_VNUM_BEAR, 0);
	if (bear == NULL)
		return;

	for (i = 0; i < MAX_STAT; i++)
		bear->perm_stat[i] = UMIN(25, 2 * ch->perm_stat[i]);

	SET_HIT(bear, ch->perm_hit);
	SET_MANA(bear, ch->perm_mana);
	bear->alignment = ch->alignment;
	bear->level = UMIN(100, 1 * ch->level-2);
	for (i=0; i < 3; i++)
		bear->armor[i] = interpolate(bear->level, 100, -100);
	bear->armor[3] = interpolate(bear->level, 100, 0);
	bear->gold = 0;
	bear->silver = 0;

	bear2 = clone_mob(bear);

	SET_BIT(bear->affected_by, AFF_CHARM);
	SET_BIT(bear2->affected_by, AFF_CHARM);
	bear->master = bear2->master = ch;
	bear->leader = bear2->leader = ch;

	act_char("Two bears come to your rescue!", ch);
	act("Two bears come to $n's rescue!", ch, NULL, NULL, TO_ROOM);

	paf = aff_new(TO_AFFECTS, "bear call");
	paf->level	= ch->level;
	paf->duration	= skill_beats("bear call");
	affect_to_char(ch, paf);
	aff_free(paf);

	char_to_room(bear, ch->in_room);
	char_to_room(bear2, ch->in_room);
}

DO_FUN(do_identify, ch, argument)
{
	OBJ_DATA *obj;
	CHAR_DATA *rch;

	if ((obj = get_obj_carry(ch, ch, argument)) == NULL) {
		 act_char("You are not carrying that.", ch);
		 return;
	}

	for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
		if (IS_NPC(rch) && MOB_IS(rch, MOB_SAGE))
			break;

	if (!rch) {
		 act_char("No one here seems to know much about that.", ch);
		 return;
	}

	if (IS_IMMORTAL(ch))
		act("$n looks at you!", rch, obj, ch, TO_VICT);
	else if (ch->gold < 1) {
		act("$n resumes to identify by looking at $p.",
		       rch, obj, 0, TO_ROOM);
		act_char("You need at least 1 gold.", ch);
		return;
	} else {
		ch->gold -= 1;
		act_char("Your purse feels lighter.", ch);
	}

	act("$n gives a wise look at $p.", rch, obj, 0, TO_ROOM);
	spellfun("identify", NULL, 0, ch, obj);
}

static void format_stat(char *buf, size_t len, CHAR_DATA *ch, int stat)
{
	if (ch->level < 20 && !IS_NPC(ch))
		strlcpy(buf, get_stat_alias(ch, stat), len);
	else {
		snprintf(buf, len, "%2d (%2d)",		// notrans
			 ch->perm_stat[stat],
			 get_curr_stat(ch, stat));
	}
}

DO_FUN(do_score, ch, argument)
{
	char buf2[MAX_INPUT_LENGTH];
	char title[MAX_STRING_LENGTH];
	const char *name;
	int ekle = 0;
	int delta;
	BUFFER *output;
	bool _can_flee;

	output = buf_new(GET_LANG(ch));
	buf_append(output, "\n      {G/~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~/~~\\{x\n");	// notrans

	strlcpy(title, IS_NPC(ch) ? " Believer of Chronos." : PC(ch)->title,
	    sizeof(title));
	name = IS_NPC(ch) ? capitalize(mlstr_val(&ch->short_descr, GET_LANG(ch))) :
			    ch->name;
	delta = strlen(title) - cstrlen(title) + MAX_CHAR_NAME - strlen(name);
	title[32+delta] = '\0';
	snprintf(buf2, sizeof(buf2), "     {G|{x   %%s%%-%ds {Y%%3d years old   {G|____|{x\n", 33+delta);
	buf_printf(output, BUF_END, buf2, name, title, get_age(ch));

	buf_append(output, "     {G|{C+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+{G|{x\n");		// notrans

	format_stat(buf2, sizeof(buf2), ch, STAT_STR);
	buf_printf(output, BUF_END, "     {G| {RLevel: {x%-3d (%+3d)    {C| {RStr: {x%-11.11s {C| {RAvatara   : {x%-10.10s {G|{x\n",
		   ch->level,
		   ch->add_level,
		   buf2,
		   "");

	format_stat(buf2, sizeof(buf2), ch, STAT_INT);
	buf_printf(output, BUF_END,
"     {G| {RRace : {x%-11.11s  {C| {RInt: {x%-11.11s {C| {RPractice  : {x%-3d        {G|{x\n",
		ch->race,
		buf2,
		IS_NPC(ch) ? 0 : PC(ch)->practice);

	format_stat(buf2, sizeof(buf2), ch, STAT_WIS);
	buf_printf(output, BUF_END,
"     {G| {RSex  : {x%-11.11s  {C| {RWis: {x%-11.11s {C| {RTrain     : {x%-3d        {G|{x\n",
		   mlstr_mval(&ch->gender),
		   buf2,
		   IS_NPC(ch) ? 0 : PC(ch)->train);

	format_stat(buf2, sizeof(buf2), ch, STAT_DEX);
	buf_printf(output, BUF_END,
"     {G| {RClass: {x%-12.12s {C| {RDex: {x%-11.11s {C| {RQuest Pnts: {x%-5d      {G|{x\n",
		IS_NPC(ch) ? "mobile" : ch->class,		// notrans
		buf2,
		IS_NPC(ch) ? 0 : PC(ch)->questpoints);

	format_stat(buf2, sizeof(buf2), ch, STAT_CON);
	buf_printf(output, BUF_END,
"     {G| {RAlign: {x%-12.12s {C| {RCon: {x%-11.11s {C| {R%-10.10s: {x%-3d        {G|{x\n",
		flag_string(align_names, NALIGN(ch)),
		buf2,
		IS_NPC(ch) ? "Quest?" : (IS_ON_QUEST(ch) ? "Quest Time" : "Next Quest"),	// notrans
		IS_NPC(ch) ? 0 : abs(PC(ch)->questtime));
	_can_flee = can_flee(ch);
	format_stat(buf2, sizeof(buf2), ch, STAT_CHA);
	buf_printf(output, BUF_END,
"     {G| {REthos: {x%-12.12s {C| {RCha: {x%-11.11s {C| {R%s     : {x%-5d      {G|{x\n",
	    IS_NPC(ch) ?
		"mobile" : flag_string(ethos_table, PC(ch)->ethos), // notrans
		buf2,
		_can_flee ? "Wimpy" : "Death",
		_can_flee ? ch->wimpy : PC(ch)->death);

	snprintf(buf2, sizeof(buf2), "%s %s.",			// notrans
		 GETMSG("You are", GET_LANG(ch)),
		 GETMSG(flag_string(position_names, ch->position), GET_LANG(ch)));
	buf_printf(output, BUF_END, "     {G| {RHome : {x%-31.31s {C|{x %-22.22s {G|{x\n",
		IS_NPC(ch) ? "Midgaard" : hometown_name(PC(ch)->hometown),
		buf2);

	buf_append(output, "     {G|{C+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+{G|{x{x\n");	// notrans

	if (!IS_NPC(ch)) {
		CHAR_DATA *vch;

		if ((vch = PC(ch)->guarding) != NULL) {
			ekle = 1;
			buf_printf(output, BUF_END,
"     {G| {GYou are guarding: {x%-12.12s                                  {G|{x\n",
				   vch->name);
		}

		if ((vch = PC(ch)->guarded_by) != NULL) {
			ekle = 1;
			buf_printf(output, BUF_END,
"     {G| {GYou are guarded by: {x%-12.12s                                {G|{x\n",
				    vch->name);
		}
	}

	if (!IS_NPC(ch)) {
		if (PC(ch)->condition[COND_DRUNK] > 10) {
			ekle = 1;
			buf_printf(output, BUF_END,
"     {G| {GYou are drunk.                                                  {G|{x\n");
		}

		if (PC(ch)->condition[COND_THIRST] <= 0) {
			ekle = 1;
			buf_printf(output, BUF_END,
"     {G| {YYou are thirsty.                                                {G|{x\n");
		}
/*		if (PC(ch)->condition[COND_FULL]   ==	0) */
		if (PC(ch)->condition[COND_HUNGER] <= 0) {
			ekle = 1;
			buf_printf(output, BUF_END,
"     {G| {YYou are hungry.                                                 {G|{x\n");
		}

		if (IS_SET(PC(ch)->plr_flags, PLR_GHOST)) {
			ekle = 1;
			buf_append(output,
"     {G| {cYou are ghost.                                                  {G|{x\n");
		}

		if (PC(ch)->condition[COND_BLOODLUST] <= 0) {
			ekle = 1;
			buf_printf(output, BUF_END,
"     {G| {YYou are hungry for blood.                                       {G|{x\n");
		}

		if (PC(ch)->condition[COND_DESIRE] <=  0) {
			ekle = 1;
			buf_printf(output, BUF_END,
"     {G| {YYou are desiring your home.                                     {G|{x\n");
		}
	}

	if (!IS_IMMORTAL(ch) && IS_PUMPED(ch)) {
		ekle = 1;
		buf_printf(output, BUF_END,
"     {G| {RYour adrenalin is gushing!                                      {G|{x\n");
	}

	if (ekle)
		buf_append(output,
"     {G|{C+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+{G|{x\n");		// notrans

	buf_printf(output, BUF_END,
"     {G| {RItems Carried : {x%9d/%-9d {RArmor vs magic  : {x%5d     {G|{x\n",
		get_carry_number(ch), can_carry_n(ch),
		GET_AC(ch,AC_EXOTIC));

	buf_printf(output, BUF_END,
"     {G| {RWeight Carried: {x%9d/%-9d {RArmor vs bash   : {x%5d     {G|{x\n",
	get_carry_weight(ch), can_carry_w(ch), GET_AC(ch, AC_BASH));

	buf_printf(output, BUF_END,
"     {G| {RGold          : {Y%9d           {RArmor vs pierce : {x%5d     {G|{x\n",
		 ch->gold, GET_AC(ch, AC_PIERCE));

	buf_printf(output, BUF_END,
"     {G| {RSilver        : {W%9d           {RArmor vs slash  : {x%5d     {G|{x\n",
		 ch->silver, GET_AC(ch, AC_SLASH));

	buf_printf(output, BUF_END,
"     {G| {RCurrent exp   : {x%9d           {RSaves vs Spell  : {x%5d     {G|{x\n",
		GET_EXP(ch), ch->saving_throw);

	buf_printf(output, BUF_END,
"     {G| {RExp to level  : {x%9d           {RHitP: {x%5d/%-5d           {G|{x\n",
		IS_NPC(ch) ? 0 : exp_to_level(ch), ch->hit, ch->max_hit);

	buf_printf(output, BUF_END,
"     {G| {RHitroll       : {x%9d           {RMana: {x%5d/%-5d           {G|{x\n",
		   GET_HITROLL(ch),ch->mana, ch->max_mana);
	buf_printf(output, BUF_END,
"     {G| {RDamroll       : {x%9d           {RMove: {x%5d/%-5d           {G|{x\n",
		    GET_DAMROLL(ch), ch->move, ch->max_move);
	buf_append(output, "  {G/~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~/   |{x\n");	// notrans
	buf_append(output, "  {G\\________________________________________________________________\\__/{x\n");	// notrans

	if (IS_SET(ch->comm, COMM_SHOWAFF))
		show_affects(ch, ch, output);
	send_to_char(buf_string(output), ch);
	buf_free(output);
}

DO_FUN(do_oscore, ch, argument)
{
	char buf2[MAX_STRING_LENGTH];
	int i;
	BUFFER *output;

	output = buf_new(GET_LANG(ch));

	buf_printf(output, BUF_END, "%s %s%s\n{x",		// notrans
		GETMSG("You are", GET_LANG(ch)),
		IS_NPC(ch) ? capitalize(mlstr_val(&ch->short_descr, GET_LANG(ch))) :
			     ch->name,
		IS_NPC(ch) ? " The Believer of Chronos." : PC(ch)->title);

	buf_printf(output, BUF_END, "Level {c%d(%+d){x, {c%d{x years old (%d hours).\n",
		ch->level, ch->add_level, get_age(ch), get_hours(ch));

	buf_printf(output, BUF_END,
		"Race: {c%s{x  Sex: {c%s{x  Class: {c%s{x  "
		"Hometown: {c%s{x\n",
		ch->race,
		mlstr_mval(&ch->gender),
		IS_NPC(ch) ? "mobile" : ch->class,
		IS_NPC(ch) ? "Midgaard" : hometown_name(PC(ch)->hometown));

	buf_printf(output, BUF_END,
		"You have {c%d{x/{c%d{x hit, {c%d{x/{c%d{x mana, "
		"{c%d{x/{c%d{x movement.\n",
		ch->hit, ch->max_hit, ch->mana, ch->max_mana,
		ch->move, ch->max_move);

	if (!IS_NPC(ch)) {
		buf_printf(output, BUF_END,
			"You have {c%d{x practices and "
			"{c%d{x training sessions.\n",
			PC(ch)->practice, PC(ch)->train);
	}

	buf_printf(output, BUF_END, "You are carrying {c%d{x/{c%d{x items "
		"with weight {c%d{x/{c%d{x pounds.\n",
		get_carry_number(ch), can_carry_n(ch),
		get_carry_weight(ch), can_carry_w(ch));

	if (ch->level > 20 || IS_NPC(ch))
		buf_printf(output, BUF_END,
			"Str: {c%d{x({c%d{x)  Int: {c%d{x({c%d{x)  "
			"Wis: {c%d{x({c%d{x)  Dex: {c%d{x({c%d{x)  "
			"Con: {c%d{x({c%d{x)  Cha: {c%d{x({c%d{x)\n",
			ch->perm_stat[STAT_STR], get_curr_stat(ch, STAT_STR),
			ch->perm_stat[STAT_INT], get_curr_stat(ch, STAT_INT),
			ch->perm_stat[STAT_WIS], get_curr_stat(ch, STAT_WIS),
			ch->perm_stat[STAT_DEX], get_curr_stat(ch, STAT_DEX),
			ch->perm_stat[STAT_CON], get_curr_stat(ch, STAT_CON),
			ch->perm_stat[STAT_CHA], get_curr_stat(ch, STAT_CHA));
	else
		buf_printf(output, BUF_END,
			"Str: {c%-9s{x Wis: {c%-9s{x Con: {c%-9s{x\n"
			"Int: {c%-9s{x Dex: {c%-9s{x Cha: {c%-11s{x\n",
			get_stat_alias(ch, STAT_STR),
			get_stat_alias(ch, STAT_WIS),
			get_stat_alias(ch, STAT_CON),
			get_stat_alias(ch, STAT_INT),
			get_stat_alias(ch, STAT_DEX),
			get_stat_alias(ch, STAT_CHA));

	snprintf(buf2, sizeof(buf2),
		 "You have scored {c%d{x exp, and have %s%s%s.\n",
		 GET_EXP(ch),
		 ch->gold + ch->silver == 0 ? "no money" :
		 ch->gold ? "{Y%ld gold{x " : str_empty,
		 ch->silver ? "{W%ld silver{x " : str_empty,
		 ch->gold + ch->silver == 1 ? "coin" :
		 ch->gold + ch->silver ? "coins" : str_empty);
	if (ch->gold)
		buf_printf(output, BUF_END, buf2, ch->gold, ch->silver);
	else
		buf_printf(output, BUF_END, buf2, ch->silver);

	/* KIO shows exp to level */
	if (!IS_NPC(ch) && ch->level < LEVEL_HERO)
		buf_printf(output, BUF_END, "You need {c%d{x exp to level.\n",
			exp_to_level(ch));

	if (!IS_NPC(ch))
		buf_printf(output, BUF_END,
			"Quest Points: {c%d{x.  "
			"%s: {c%d{x.\n",
			PC(ch)->questpoints, 
			IS_NPC(ch) ? "Quest?" : (IS_ON_QUEST(ch) ? 
					"Quest Time" : "Next Quest"),
			IS_NPC(ch) ? 0 : abs(PC(ch)->questtime));

	if (can_flee(ch))
		buf_printf(output, BUF_END, "Wimpy set to {c%d{x hit points.",
			   ch->wimpy);
	else
		buf_printf(output, BUF_END, "Total {c%d{x deaths up to now.",
			   PC(ch)->death);

	if (!IS_NPC(ch)) {
		CHAR_DATA *vch;

		if ((vch = PC(ch)->guarding) != NULL) {
			buf_printf(output, BUF_END, "  You are guarding: {W%s{x\n",
				   vch->name);
		}

		if ((vch = PC(ch)->guarded_by) != NULL) {
			buf_printf(output, BUF_END, "  You are guarded by: {W%s{x\n",
				   vch->name);
		}
	}

	buf_append(output, "\n");

	if (!IS_NPC(ch)) {
		if (PC(ch)->condition[COND_DRUNK] > 10)
			buf_append(output, "You are {cdrunk{x.\n");

		if (PC(ch)->condition[COND_THIRST] <= 0)
			buf_append(output, "You are {rthirsty{x.\n");

/*		if (PC(ch)->condition[COND_FULL] == 0) */
		if (PC(ch)->condition[COND_HUNGER] <= 0)
			buf_append(output, "You are {rhungry{x.\n");
		if (PC(ch)->condition[COND_BLOODLUST] <= 0)
			buf_append(output, "You are {rhungry for {Rblood{x.\n");
		if (PC(ch)->condition[COND_DESIRE] <= 0)
			buf_append(output, "You are {rdesiring your home{x.\n");
		if (IS_SET(PC(ch)->plr_flags, PLR_GHOST))
			buf_append(output, "You are {cghost{x.\n");
	}

	buf_printf(output, BUF_END, "%s %s.\n",			// notrans
		   GETMSG("You are", GET_LANG(ch)),
		   GETMSG(flag_string(position_names, ch->position),
			  GET_LANG(ch)));

	if ((ch->position == POS_SLEEPING || ch->position == POS_RESTING ||
	     ch->position == POS_FIGHTING || ch->position == POS_STANDING)
	&& !IS_IMMORTAL(ch) && IS_PUMPED(ch))
		buf_append(output, "Your {radrenalin is gushing{x!\n");

	/* print AC values */
	if (ch->level >= 25) {
		buf_printf(output, BUF_END,
			   "Armor: pierce: {c%d{x  bash: {c%d{x  "
			   "slash: {c%d{x  magic: {c%d{x\n",
			   GET_AC(ch, AC_PIERCE), GET_AC(ch, AC_BASH),
			   GET_AC(ch, AC_SLASH), GET_AC(ch, AC_EXOTIC));

		buf_printf(output, BUF_END,
			   "Saves vs. spell: {c%d{x\n",
			   ch->saving_throw);
	} else {
		for (i = 0; i < 4; i++) {
			static const char *ac_name[4] = {
				"{cpiercing{x",
				"{cbashing{x",
				"{cslashing{x",
				"{cmagic{x"
			};

			buf_append(output, "You are ");
			if (GET_AC(ch,i) >= 101)
				buf_printf(output, BUF_END,
					   "{chopelessly vulnerable{x to %s.\n",
					   ac_name[i]);
			else if (GET_AC(ch,i) >= 80)
				buf_printf(output, BUF_END,
					   "{cdefenceless against{x %s.\n",
					   ac_name[i]);
			else if (GET_AC(ch,i) >= 60)
				buf_printf(output, BUF_END, "{cbarely protected{x from %s.\n",
					   ac_name[i]);
			else if (GET_AC(ch,i) >= 40)
				buf_printf(output, BUF_END, "{cslightly armored{x against %s.\n",
					   ac_name[i]);
			else if (GET_AC(ch,i) >= 20)
				buf_printf(output, BUF_END, "{csomewhat armored{x against %s.\n",
					   ac_name[i]);
			else if (GET_AC(ch,i) >= 0)
				buf_printf(output, BUF_END, "{carmored{x against %s.\n",
					   ac_name[i]);
			else if (GET_AC(ch,i) >= -20)
				buf_printf(output, BUF_END, "{cwell-armored{x against %s.\n",
					   ac_name[i]);
			else if (GET_AC(ch,i) >= -40)
				buf_printf(output, BUF_END, "{cvery well-armored{x against %s.\n",
					   ac_name[i]);
			else if (GET_AC(ch,i) >= -60)
				buf_printf(output, BUF_END, "{cheavily armored{x against %s.\n",
					   ac_name[i]);
			else if (GET_AC(ch,i) >= -80)
				buf_printf(output, BUF_END, "{csuperbly armored{x against %s.\n",
					   ac_name[i]);
			else if (GET_AC(ch,i) >= -100)
				buf_printf(output, BUF_END, "{calmost invulnerable{x to %s.\n",
					   ac_name[i]);
			else
				buf_printf(output, BUF_END, "{cdivinely armored{x against %s.\n",
					   ac_name[i]);
		}
	}

	/* RT wizinvis and holy light */
	if (IS_IMMORTAL(ch)) {
		buf_printf(output, BUF_END, "Holy Light: {c%s{x",
			   IS_SET(PC(ch)->plr_flags, PLR_HOLYLIGHT) ?
			   "on" : "off");

		if (ch->invis_level)
			buf_printf(output, BUF_END, "  Invisible: {clevel %d{x",
				ch->invis_level);

		if (ch->incog_level)
			buf_printf(output, BUF_END, "  Incognito: {clevel %d{x",
				ch->incog_level);
		buf_append(output, "\n");
	}

	if (ch->level >= 20)
		buf_printf(output, BUF_END, "Hitroll: {c%d{x  Damroll: {c%d{x.\n",
			GET_HITROLL(ch), GET_DAMROLL(ch));

	buf_append(output, "You are ");
	if (IS_GOOD(ch))
		buf_append(output, "good.");
	else if (IS_EVIL(ch))
		buf_append(output, "evil.");
	else
		buf_append(output, "neutral.");

	if (!IS_NPC(ch)) {
		switch (PC(ch)->ethos) {
		case ETHOS_LAWFUL:
			buf_append(output, "  You have a lawful ethos.\n");
			break;
		case ETHOS_NEUTRAL:
			buf_append(output, "  You have a neutral ethos.\n");
			break;
		case ETHOS_CHAOTIC:
			buf_append(output, "  You have a chaotic ethos.\n");
			break;
		default:
			buf_append(output, "  You have no ethos.\n");
			break;
		}
	} else
		buf_append(output, "  You have no ethos.\n");

/*
	if (i <= RELIGION_NONE || i > MAX_RELIGION)
		buf_append(output, "You don't believe any religion.\n");
	else
		buf_printf(output, BUF_END,"Your religion is the way of %s.\n",
			   religion_table[i].leader);
*/

	if (IS_SET(ch->comm, COMM_SHOWAFF))
		show_affects(ch, ch, output);
	send_to_char(buf_string(output), ch);
	buf_free(output);
}

DO_FUN(do_affects, ch, argument)
{
	BUFFER *output;

	output = buf_new(GET_LANG(ch));
	show_affects(ch, ch, output);
	page_to_char(buf_string(output), ch);
	buf_free(output);
}

DO_FUN(do_raffects, ch, argument)
{
	AFFECT_DATA *paf, *paf_last = NULL;

	if (ch->in_room->affected == NULL) {
		act_char("The room is not affected by any spells.", ch);
		return;
	}

	act_char("The room is affected by the following spells:", ch);
	for (paf = ch->in_room->affected; paf != NULL; paf = paf->next) {
		if (paf_last != NULL && IS_SKILL(paf->type, paf_last->type)) {
			if (ch->level >= 20) {
				act_puts("                      ",
					 ch, NULL, NULL,
					 TO_CHAR | ACT_NOLF, POS_DEAD);
			} else
				continue;
		} else {
			act_puts("Spell: {c$F-15{$t}{x",
				 ch, paf->type, NULL,
				 TO_CHAR | ACT_NOTRANS | ACT_NOLF, POS_DEAD);
		}

		if (ch->level >= 20) {
			act_puts(": modifies {c$T{x by {c$j{x ",
				 ch, (const void *) paf->modifier,
				 SFLAGS(rapply_flags, paf->location),
				 TO_CHAR | ACT_NOTRANS | ACT_NOLF, POS_DEAD);
			if (paf->duration == -1 || paf->duration == -2) {
				act_puts("permanently.",
					 ch, NULL, NULL,
					 TO_CHAR | ACT_NOLF | ACT_NOUCASE,
					 POS_DEAD);
			} else {
				act_puts("for {c$j{x $qj{hours}.",
					 ch, (const void *) paf->duration, NULL,
					 TO_CHAR | ACT_NOLF, POS_DEAD);
			}
		}
		send_to_char("\n", ch);
		paf_last = paf;
	}
}

static const char *
get_resist_alias(int resist)
{
	if (resist < -90)
		return "eradicated by";
	else if (resist < -70)
		return "obliterated by";
	else if (resist < -50)
		return "massacred by";
	else if (resist < -30)
		return "hurt by";
	else if (resist < 0)
		return "vulnerable to";
	else if (resist == 0)
		return "don't have any resistance against";
	else if (resist < 10)
		return "slightly toughened against";
	else if (resist < 20)
		return "toughened against";
	else if (resist < 30)
		return "slightly resistant against";
	else if (resist < 50)
		return "resistant against";
	else if (resist < 70)
		return "highly resistant against";
	else if (resist < 100)
		return "almost immune to";
	else
		return "immune to";
}

DO_FUN(do_resists, ch, argument)
{
	int i;
	bool found = FALSE;

	for (i = 0; i < MAX_RESIST; i++) {
		int res;

		if (i == DAM_CHARM
		||  (res = get_resist(ch, i, FALSE)) == 0)
			continue;

		found = TRUE;
		if (ch->level < MAX_LEVEL / 3) {
			act_puts("You are $t $T.",
				 ch, get_resist_alias(res),
				 flag_string(resist_info_flags, i),
				 TO_CHAR | ACT_NOTRANS, POS_DEAD);
		} else {
			act_puts3("You are $t $T ($J%).",
				  ch, get_resist_alias(res),
				  flag_string(resist_info_flags, i),
				  (const void *) res,
				  TO_CHAR, POS_DEAD);
		}
	}

	if (!found) {
		act("You don't have any resistances and vulnerabilities.",
		    ch, NULL, NULL, TO_CHAR);
	}
}

#define MOB_VNUM_LION			19

DO_FUN(do_lion_call, ch, argument)
{
	CHAR_DATA *	gch;
	CHAR_DATA *	lion;
	CHAR_DATA *	lion2;
	AFFECT_DATA	*paf;
	int		i;
	int		mana;

	act_char("You call for lions help you.", ch);
	act("$n shouts a lion call.", ch, NULL, NULL, TO_ROOM);

	if (is_sn_affected(ch, "lion call")) {
		act_char("You cannot summon the strength to handle more lions right now.", ch);
		return;
	}

	for (gch = npc_list; gch; gch = gch->next) {
		if (IS_AFFECTED(gch,AFF_CHARM)
		&&  gch->master == ch
		&&  gch->pMobIndex->vnum == MOB_VNUM_LION) {
			act_char("What's wrong with the lion you've got?", ch);
			return;
		}
	}

	if (ch->in_room != NULL
	&& IS_SET(ch->in_room->room_flags, ROOM_NOMOB)) {
		act_char("No lions hear you.", ch);
		return;
	}

	if (IS_SET(ch->in_room->room_flags, ROOM_PEACE |
					    ROOM_PRIVATE | ROOM_SOLITARY)
	||  (ch->in_room->exit[0] == NULL && ch->in_room->exit[1] == NULL
	&&   ch->in_room->exit[2] == NULL && ch->in_room->exit[3] == NULL
	&&   ch->in_room->exit[4] == NULL && ch->in_room->exit[5] == NULL)
	||  (ch->in_room->sector_type != SECT_FIELD
	&&   ch->in_room->sector_type != SECT_FOREST
	&&   ch->in_room->sector_type != SECT_MOUNTAIN
	&&   ch->in_room->sector_type != SECT_HILLS)) {
		act_char("No lions come to your rescue.", ch);
		return;
	}

	mana = skill_mana(ch, "lion call");
	if (ch->mana < mana) {
		act_char("You don't have enough mana to shout a lion call.", ch);
		return;
	}
	ch->mana -= mana;

	if (number_percent() > get_skill(ch, "lion call")) {
		check_improve(ch, "lion call", FALSE, 1);
		act_char("No lions hear you.", ch);
		return;
	}

	check_improve(ch, "lion call", TRUE, 1);
	lion = create_mob(MOB_VNUM_LION, 0);
	if (lion == NULL)
		return;

	for (i = 0; i < MAX_STAT; i++)
		lion->perm_stat[i] = UMIN(25,2 * ch->perm_stat[i]);

	SET_HIT(lion, ch->perm_hit);
	SET_MANA(lion, ch->perm_mana);
	lion->alignment = ch->alignment;
	lion->level = UMIN(100,1 * ch->level-2);
	for (i = 0; i < 3; i++)
		lion->armor[i] = interpolate(lion->level,100,-100);
	lion->armor[3] = interpolate(lion->level,100,0);
	lion->gold = 0;
	lion->silver = 0;

	lion2 = clone_mob(lion);

	SET_BIT(lion->affected_by, AFF_CHARM);
	SET_BIT(lion2->affected_by, AFF_CHARM);
	lion->master = lion2->master = ch;
	lion->leader = lion2->leader = ch;

	act_char("Two lions come to your rescue!", ch);
	act("Two lions come to $n's rescue!",ch,NULL,NULL,TO_ROOM);

	paf = aff_new(TO_AFFECTS, "lion call");
	paf->level	= ch->level;
	paf->duration	= skill_beats("lion call");
	affect_to_char(ch, paf);
	aff_free(paf);

	char_to_room(lion, ch->in_room);
	char_to_room(lion2, ch->in_room);
}

/* new practice */
DO_FUN(do_practice, ch, argument)
{
	CHAR_DATA	*mob;
	pc_skill_t	*pc_sk;
	spec_skill_t	spec_sk;
	bool		found;
	char		arg[MAX_STRING_LENGTH];
	PC_DATA	*	pc;
	skill_t *	sk;

	if (IS_NPC(ch))
		return;

	pc = PC(ch);

	if (argument[0] == '\0') {
		BUFFER *output;
		int col = 0;

		output = buf_new(GET_LANG(ch));

		C_FOREACH(pc_sk, &pc->learned) {
			spec_sk.sn = pc_sk->sn;
			spec_stats(ch, &spec_sk);

			if (pc_sk->percent == 0
			||  spec_sk.max <= 0
			||  pc_sk->percent >= spec_sk.adept)
				continue;

			buf_printf(output, BUF_END, "%-19s %3d%%  ", // notrans
				   pc_sk->sn,
				   UMAX(100 * pc_sk->percent / spec_sk.adept, 1));
			if (++col % 3 == 0)
				buf_append(output, "\n");
		}

		if (col % 3)
			buf_append(output, "\n");
		if (!col)
			buf_append(output, "You have nothing to practice.\n");

		buf_printf(output, BUF_END, "You have %d practice sessions left.\n",
			   pc->practice);

		page_to_char(buf_string(output), ch);
		buf_free(output);
		return;
	}

	if (!IS_AWAKE(ch)) {
		act_char("In your dreams, or what?", ch);
		return;
	}

	if (pc->practice <= 0) {
		act_char("You have no practice sessions left.", ch);
		return;
	}

	one_argument(argument, arg, sizeof(arg));
	pc_sk = (pc_skill_t*) c_strkey_search(&pc->learned, arg);
	if (pc_sk == NULL
	||  (sk = skill_lookup(pc_sk->sn)) == NULL
	||  pc_sk->percent == 0
	||  get_skill(ch, pc_sk->sn) == 0) {
		act_char("You can't practice that.", ch);
		return;
	}

	if (IS_SKILL(pc_sk->sn, "vampire")) {
		act_char("You can't practice that, only available at questor.", ch);
		return;
	}

	found = FALSE;
	for (mob = ch->in_room->people; mob != NULL; mob = mob->next_in_room) {
		int group;

		if (!IS_NPC(mob) || !MOB_IS_PRACTICER(mob))
			continue;

		found = TRUE;
		if (sk->group == GROUP_CREATION
		||  sk->group == GROUP_HARMFUL
		||  sk->group == GROUP_PROTECTIVE
		||  sk->group == GROUP_DIVINATION
		||  sk->group == GROUP_WEATHER)
			group = GROUP_NONE;
		else
			group = sk->group;
		if (MOB_PRACTICES(mob, group)
		&&  (group != GROUP_CLAN || IS_CLAN(ch->clan, mob->clan)))
			break;
	}

	if (mob == NULL) {
		if (found) {
			act_char("You can't do that here. Use 'slook skill', 'help practice' for more info.", ch);
		} else {
			act_char("You couldn't find anyone who can teach you.", ch);
		}
		return;
	}

	spec_sk.sn = pc_sk->sn;
	spec_stats(ch, &spec_sk);
	if (pc_sk->percent >= spec_sk.adept) {
		act_puts("You are already learned at $t.",
			 ch, pc_sk->sn, NULL,
			 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
		return;
	}

	pc->practice--;

	pc_sk->percent += int_app[get_curr_stat(ch, STAT_INT)].learn /
							spec_sk.rating;
	if (pc_sk->percent < spec_sk.adept) {
		act("You practice $v.", ch, &sk->sk_name, NULL, TO_CHAR);
		act("$n practices $v.", ch, &sk->sk_name, NULL, TO_ROOM);
	} else {
		pc_sk->percent = spec_sk.adept;
		act("You are now learned at $v.",
		    ch, &sk->sk_name, NULL, TO_CHAR);
		act("$n is now learned at $v.",
		    ch, &sk->sk_name, NULL, TO_ROOM);
	}
}

DO_FUN(do_learn, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *practicer;
	pc_skill_t *pc_sk;
	pc_skill_t *pc_sk2;
	spec_skill_t spec_sk;
	PC_DATA *pc;

	if (IS_NPC(ch))
		return;

	if (!IS_AWAKE(ch)) {
		act_char("In your dreams, or what?", ch);
		return;
	}	

	if (argument[0] == '\0') {
		act_char("Syntax: learn <skill | spell> <player>", ch);
		return;
	}

	pc = PC(ch);

	if (pc->practice <= 0) {
		act_char("You have no practice sessions left.", ch);
		return;
	}

	argument = one_argument(argument, arg, sizeof(arg));
	pc_sk = (pc_skill_t*) c_strkey_search(&pc->learned, arg);
	if (pc_sk == NULL
	||  skill_lookup(pc_sk->sn) == NULL
	||  pc_sk->percent == 0
	||  get_skill(ch, pc_sk->sn) == 0) {
		act_char("You can't learn that.", ch);
		return;
	}

	if (IS_SKILL(pc_sk->sn, "vampire")) {
		act_char("You can't practice that, only available at questor.", ch);
		return;
	}	

	argument = one_argument(argument, arg, sizeof(arg));
		
	if ((practicer = get_char_here(ch,arg)) == NULL) {
		act_char("Your hero is not here.", ch);
		return;
	}
			
	if (IS_NPC(practicer) || practicer->level != LEVEL_HERO) {
		act_char("You must find a hero, not an ordinary one.",
			  ch);
		return;
	}

	if (!IS_SET(PC(practicer)->plr_flags, PLR_PRACTICER)) {
		act_char("Your hero doesn't want to teach you anything.", ch);
		return;
	}

	spec_sk.sn = pc_sk->sn;
	spec_stats(practicer, &spec_sk);
	if ((pc_sk2 = pc_skill_lookup(practicer, pc_sk->sn)) == NULL
	||  pc_sk2->percent < spec_sk.max) {
		act_char("Your hero doesn't know that skill enough to teach you.", ch);
		return;
	}

	spec_stats(ch, &spec_sk);
	if (pc_sk->percent >= spec_sk.adept) {
		act_puts("You are already learned at $t.",
			 ch, pc_sk->sn, NULL,
			 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
		return;
	}

	pc->practice--;

	pc_sk->percent += int_app[get_curr_stat(ch, STAT_INT)].learn /
						spec_sk.rating;

	act("You teach $T.", practicer, NULL, pc_sk->sn, TO_CHAR);
	act("$n teaches $T.", practicer, NULL, pc_sk->sn, TO_ROOM);
	REMOVE_BIT(PC(practicer)->plr_flags, PLR_PRACTICER);

	if (pc_sk->percent < spec_sk.adept) {
		act("You learn $T.", ch, NULL, pc_sk->sn, TO_CHAR);
		act("$n learn $T.", ch, NULL, pc_sk->sn, TO_ROOM);
	} else {
		pc_sk->percent = spec_sk.adept;
		act("You are now learned at $T.", ch, NULL, pc_sk->sn, TO_CHAR);
		act("$n is now learned at $T.", ch, NULL, pc_sk->sn, TO_ROOM);
	}
}

DO_FUN(do_teach, ch, argument)
{
	if (IS_NPC(ch) || ch->level != LEVEL_HERO) {
		act_char("You must be a hero.", ch);
		return;
	}
	SET_BIT(PC(ch)->plr_flags, PLR_PRACTICER);
	act_char("Now, you can teach youngsters your 100% skills.", ch);
}

/* used to converter of prac and train */
DO_FUN(do_gain, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *tr;
	PC_DATA *pc;

	if (IS_NPC(ch))
		return;

	/* find a trainer */
	for (tr = ch->in_room->people; tr; tr = tr->next_in_room) {
		if (IS_NPC(tr) && MOB_IS(tr, MOB_TRAIN | MOB_GAIN))
			break;
	}

	if (tr == NULL || !can_see(ch, tr)) {
		act_char("You can't do that here.", ch);
		return;
	}

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		do_say(tr, "You may convert 10 practices into 1 train.");
		do_say(tr, "You may revert 1 train into 10 practices.");
		do_say(tr, "Simply type 'gain convert' or 'gain revert'.");
		return;
	}

	pc = PC(ch);

	if (!str_prefix(arg, "revert")) {
		if (pc->train < 1) {
			tell_char(tr, ch, "You are not ready yet.");
			return;
		}

		act("$N helps you apply your training to practice.",
		    ch, NULL, tr, TO_CHAR);
		pc->practice += 10;
		pc->train -=1 ;
		return;
	}

	if (!str_prefix(arg, "convert")) {
		if (pc->practice < 10) {
			tell_char(tr, ch, "You are not ready yet.");
			return;
		}

		act("$N helps you apply your practice to training.",
		    ch, NULL, tr, TO_CHAR);
		pc->practice -= 10;
		pc->train +=1 ;
		return;
	}

	tell_char(tr, ch, "I do not understand...");
}

static const char *
skill_knowledge_alias(CHAR_DATA *ch, pc_skill_t *pc_sk, spec_skill_t *spec_sk)
{
	int percent;
	const char *knowledge;

	spec_sk->sn = pc_sk->sn;
	spec_stats(ch, spec_sk);

	if (spec_sk->level > ch->level
	||  !spec_sk->max)
		return NULL;

	percent = URANGE(1, 100 * pc_sk->percent / spec_sk->max, 100);

	if (!percent)
		knowledge = "not learned";
	else if (percent < 15)
		knowledge = "ridiculous";
	else if (percent < 30)
		knowledge = "very bad";
	else if (percent < 50)
		knowledge = "bad";
	else if (percent < 75)
		knowledge = "average";
	else if (percent < 80)
		knowledge = "good";
	else if (percent < 90)
		knowledge = "very good";
	else if (percent < 100)
		knowledge = "remarkable";
	else
		knowledge = "perfect";

	return knowledge;
}

/* RT spells and skills show the players spells (or skills) */
DO_FUN(do_prayers, ch, argument)
{
	if (ch->shapeform != NULL
	&&  IS_SET(ch->shapeform->index->flags, SHAPEFORM_NOCAST)) {
		act("You are unable to pray in this form.",
		    ch, NULL, NULL, TO_CHAR);
		return;
	}
	list_spells(ST_PRAYER, ch, argument);
}

DO_FUN(do_spells, ch, argument)
{
	if (ch->shapeform != NULL
	&&  IS_SET(ch->shapeform->index->flags, SHAPEFORM_NOCAST)) {
		act("You cannot cast any spells in this form.",
		    ch, NULL, NULL, TO_CHAR);
		return;
	}

	list_spells(ST_SPELL, ch, argument);
}

static void
list_spells(flag_t type, CHAR_DATA *ch,
	    const char *argument __attribute__((unused)))
{
	BUFFER *list[LEVEL_IMMORTAL+1];
	int lev;
	bool found = FALSE;
	char buf[MAX_STRING_LENGTH];
	BUFFER *output;
	pc_skill_t *pc_sk;

	if (IS_NPC(ch))
		return;

	/* initialize data */
	for (lev = 0; lev <= LEVEL_IMMORTAL; lev++)
		list[lev] = NULL;

	C_FOREACH(pc_sk, &PC(ch)->learned) {
		skill_t *sk;
		spec_skill_t spec_sk;
		const char *knowledge;

		if (pc_sk->percent == 0
		||  (sk = skill_lookup(pc_sk->sn)) == NULL
		||  sk->skill_type != type)
			continue;

		knowledge = skill_knowledge_alias(ch, pc_sk, &spec_sk);
		if (knowledge == NULL)
			continue;

		found = TRUE;
		lev = spec_sk.level;
		snprintf(buf, sizeof(buf),
			 "%-24s [%-11s] %4d mana",
			 pc_sk->sn, knowledge, skill_mana(ch, pc_sk->sn));

		if (list[lev] == NULL) {
			list[lev] = buf_new(GET_LANG(ch));
			buf_printf(list[lev], BUF_END,
				   "\nLevel %2d  : %s", lev, buf);
		} else
			buf_printf(list[lev], BUF_END,
				   "\n            %s", buf);	// notrans
	}

	/* return results */

	if (!found) {
		switch(type) {
			case ST_PRAYER:
				act_char("You do not have any empowers.", ch);
				return;
			case ST_SPELL:
				act_char("You know no spells.", ch);
				return;
		}
	}

	output = buf_new(GET_LANG(ch));
	for (lev = 0; lev <= UMIN(ch->level, LEVEL_IMMORTAL); lev++)
		if (list[lev] != NULL) {
			buf_append(output, buf_string(list[lev]));
			buf_free(list[lev]);
		}
	buf_append(output, "\n");
	page_to_char(buf_string(output), ch);
	buf_free(output);
}

static bool
list_form_skills(CHAR_DATA *ch, BUFFER *output)
{
	spec_t * fsp;

	if (!ch->shapeform)
		return FALSE;

	if (IS_NULLSTR(ch->shapeform->index->skill_spec))
		return FALSE;

	if (!(fsp=spec_lookup(ch->shapeform->index->skill_spec))) {
		printlog(LOG_BUG, "get_skill: bad form (%s) spec (%s).\n",
		    ch->shapeform->index->name,
		    ch->shapeform->index->skill_spec);
		return FALSE;
	} else if (!c_isempty(&fsp->spec_skills)) {
		spec_skill_t *spec_sk;

		buf_printf(output, BUF_END, "Your form skills:\n");

		C_FOREACH(spec_sk, &fsp->spec_skills) {
			if (!IS_NULLSTR(spec_sk->sn)) {
				buf_printf(output, BUF_END, "%s\n",
					   spec_sk->sn);
			}
		}

		return TRUE;
	} else {
		return FALSE;
	}
}

DO_FUN(do_skills, ch, argument)
{
	BUFFER *skill_list[LEVEL_IMMORTAL+1];
	char skill_columns[LEVEL_IMMORTAL+1];
	int lev;
	bool found = FALSE;
	char buf[MAX_STRING_LENGTH];
	BUFFER *output;
	pc_skill_t *pc_sk;

	if (IS_NPC(ch))
		return;

	/* initialize data */
	for (lev = 0; lev <= LEVEL_IMMORTAL; lev++) {
		skill_columns[lev] = 0;
		skill_list[lev] = NULL;
	}

	C_FOREACH(pc_sk, &PC(ch)->learned) {
		skill_t *sk;
		spec_skill_t spec_sk;
		const char *knowledge;

		if (pc_sk->percent == 0
		||  (sk = skill_lookup(pc_sk->sn)) == NULL
		||  sk->skill_type != ST_SKILL)
			continue;

		/* Don't show skills in form if they wouldn't work anyway */
		if (ch->shapeform && !IS_SET(sk->skill_flags, SKILL_FORM))
			continue;

		knowledge = skill_knowledge_alias(ch, pc_sk, &spec_sk);
		if (knowledge == NULL)
			continue;

		found = TRUE;
		lev = spec_sk.level;

		snprintf(buf, sizeof(buf), "%-19s %-11s  ",	// notrans
			pc_sk->sn, knowledge);

		if (skill_list[lev] == NULL) {
			skill_list[lev] = buf_new(GET_LANG(ch));
			buf_printf(skill_list[lev], BUF_END,
				   "\nLevel %2d  : %s", lev, buf);
		} else {
			if (++skill_columns[lev] % 2 == 0) {
				buf_append(skill_list[lev],
					   "\n            ");	// notrans
			}
			buf_append(skill_list[lev], buf);
		}
	}


	/* return results */

	output = buf_new(GET_LANG(ch));
	if (found) {
		for (lev = 0; lev <= UMIN(ch->level, LEVEL_IMMORTAL); lev++)
			if (skill_list[lev] != NULL) {
				buf_append(output, buf_string(skill_list[lev]));
				buf_free(skill_list[lev]);
			}
		buf_append(output, "\n");
	}
	found = list_form_skills(ch, output) || found;

	if (found)
		page_to_char(buf_string(output), ch);
	else
		act_char("You don't know any skills.", ch);

	buf_free(output);
}

DO_FUN(do_glist, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];
	flag_t group = GROUP_NONE;
	skill_t *sk;
	int col = 0;

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		act_char("Syntax: glist <group>.", ch);
		act_char("Use 'glist ?' to get the list of groups.", ch);
		return;
	}

	if (!str_cmp(arg, "?")) {
		show_flags(ch, skill_groups);
		return;
	}

	if (str_prefix(arg, "none")
	&&  (group = flag_value(skill_groups, arg)) == 0) {
		act_char("That is not a valid group.", ch);
		do_glist(ch, str_empty);
		return;
	}

	act_puts("Now listing group '$t':",
		 ch, flag_string(skill_groups, group), NULL,
		 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);

	C_FOREACH(sk, &skills) {
		const char *sn;

		if (group != sk->group)
			continue;

		sn = gmlstr_mval(&sk->sk_name);
		act_puts("$t$f-18{$T}", ch,		           // notrans
			 pc_skill_lookup(ch, sn) ?  "*" : " ", sn, // notrans
			 TO_CHAR | ACT_NOTRANS | ACT_NOLF, POS_DEAD);
		if (col)
			send_to_char("\n", ch);
		col = 1 - col;
	}

	if (col)
		send_to_char("\n", ch);
}

DO_FUN(do_slook, ch, argument)
{
	pc_skill_t *pc_sk = NULL;
	skill_t *sk;
	char arg[MAX_INPUT_LENGTH];

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		act_char("Syntax : slook <skill | spell>.", ch);
		return;
	}

	if (!IS_NPC(ch))
		pc_sk = (pc_skill_t*) c_strkey_search(&PC(ch)->learned, arg);

	if (pc_sk == NULL)
		sk = skill_search(arg, ST_ALL);
	else
		sk = skill_lookup(pc_sk->sn);

	if (sk == NULL) {
		act_char("That is not a spell or skill.", ch);
		return;
	}

	act_puts("Skill '$t' in group '$T'.",
		 ch, gmlstr_mval(&sk->sk_name),
		 flag_string(skill_groups, sk->group),
		 TO_CHAR | ACT_NOTRANS, POS_DEAD);
}

DO_FUN(do_camp, ch, argument)
{
	AFFECT_DATA *paf;
	int mana;

	if (is_sn_affected(ch, "camp")) {
		act_char("You don't have enough power to handle more camp areas.", ch);
		return;
	}

	if (IS_SET(ch->in_room->room_flags, ROOM_PEACE |
					    ROOM_PRIVATE | ROOM_SOLITARY)
	||  (ch->in_room->sector_type != SECT_FIELD
	&&   ch->in_room->sector_type != SECT_FOREST
	&&   ch->in_room->sector_type != SECT_MOUNTAIN
	&&   ch->in_room->sector_type != SECT_HILLS)) {
		act_char("There are not enough leaves to camp here.", ch);
		return;
	}

	mana = skill_mana(ch, "camp");
	if (ch->mana < mana) {
		act_char("You don't have enough mana to make a camp.", ch);
		return;
	}
	ch->mana -= mana;

	if (number_percent() > get_skill(ch, "camp")) {
		act_char("You failed to make your camp.", ch);
		check_improve(ch, "camp", FALSE, 4);
		return;
	}

	check_improve(ch, "camp", TRUE, 4);
	WAIT_STATE(ch, skill_beats("camp"));

	act_char("You succeeded to make your camp.", ch);
	act("$n succeeded to make $s camp.", ch, NULL, NULL, TO_ROOM);

	paf = aff_new(TO_AFFECTS, "camp");
	paf->level	= ch->level;
	paf->duration	= 12;
	affect_to_char(ch, paf);

	paf->where	= TO_ROOM_AFFECTS;
	paf->duration	= ch->level / 20;
	paf->modifier	= 2 * LEVEL(ch);
	INT(paf->location)= APPLY_ROOM_HEAL;
	paf->owner	= ch;
	affect_to_room(ch->in_room, paf);

	INT(paf->location)= APPLY_ROOM_MANA;
	affect_to_room(ch->in_room, paf);
	aff_free(paf);
}

DO_FUN(do_demand, ch, argument)
{
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA  *obj;
	int chance;

	if (IS_NPC(ch))
		return;

	argument = one_argument(argument, arg1, sizeof(arg1));
	argument = one_argument(argument, arg2, sizeof(arg2));
	if (arg1[0] == '\0' || arg2[0] == '\0') {
		act_char("Demand what from whom?", ch);
		return;
	}

	if ((victim = get_char_here(ch, arg2)) == NULL) {
		act_char("They aren't here.", ch);
		return;
	}

	if (!IS_NPC(victim)) {
		act_char("Why don't you just want that directly from the player?", ch);
		return;
	}

	if (victim->position == POS_SLEEPING) {
		act("You should wake $N first.", ch, NULL, victim, TO_CHAR);
		return;
	}
	if (!can_see(victim, ch)) {
		do_say(victim,
			"If you want something from me, stop hiding from me!");
		return;
	}

	if (victim->position == POS_FIGHTING) {
		do_say(victim,
		    "Let me finish my fight, and then we'll talk.");
		return;
	}

	WAIT_STATE(ch, get_pulse("violence"));

	chance = IS_EVIL(victim) ? 10 : IS_GOOD(victim) ? -5 : 0;
	chance += (get_curr_stat(ch, STAT_CHA) - 15) * 10;
	chance += LEVEL(ch) - LEVEL(victim);
	chance = get_skill(ch, "demand") * chance / 100;

	if (number_percent() > chance) {
		do_say(victim, "I'm not about to give you anything!");
		check_improve(ch, "demand", FALSE, 1);
		do_murder(victim, ch->name);
		return;
	}

	check_improve(ch, "demand", TRUE, 1);

	if (((obj = get_obj_carry(victim, victim, arg1)) == NULL
	&&   (obj = get_obj_wear(victim, victim, arg1)) == NULL)
	||  IS_OBJ_STAT(obj, ITEM_INVENTORY)) {
		do_say(victim, "Sorry, I don't have that.");
		return;
	}

	if (OBJ_IS(obj, OBJ_QUIT_DROP)) {
		do_say(victim, "Forgive me, my master, I can't give it to anyone.");
		return;
	}

	if (!can_drop_obj(victim, obj)) {
		do_say(victim, "It's cursed, so I can't let go of it. Forgive me, my master.");
		return;
	}

	if (!can_carry_more_n(ch, get_obj_number(obj))) {
		act_char("Your hands are full.", ch);
		return;
	}

	if (!can_carry_more_w(ch, get_obj_weight(obj))) {
		act_char("You can't carry that much weight.", ch);
		return;
	}

	if (!can_see_obj(ch, obj)) {
		act("You don't see that.", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (OBJ_IS(obj, OBJ_QUIT_DROP)) {
		do_say(victim, "Sorry, I must keep it myself.");
		return;
	}

	if (obj->wear_loc != WEAR_NONE
	&&  !remove_obj(victim, obj->wear_loc, TRUE)) {
		do_say(victim, "I can't remove it. Forgive me, my master.");
		return;
	}

	obj_to_char(obj, ch);
	act("$n demands $p from $N.", ch, obj, victim, TO_NOTVICT);
	act("You demand $p from $N.",	ch, obj, victim, TO_CHAR  );
	act("$n demands $p from you.", ch, obj, victim, TO_VICT  );

	pull_obj_trigger(TRIG_OBJ_GIVE, obj, victim, ch);
	if (!IS_EXTRACTED(ch))
		act_char("Your power makes all around the world shivering.", ch);
}

DO_FUN(do_control, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];
	int chance;
	CHAR_DATA *victim;
	race_t *r;

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		act_char("Charm what?", ch);
		return;
	}

	if ((victim = get_char_here(ch, arg)) == NULL) {
		act_char("They aren't here.", ch);
		return;
	}

	if ((r = race_lookup(ORG_RACE(victim))) == NULL
	||  !IS_SET(r->form, FORM_ANIMAL)) {
		act_char("You should try this on animals?", ch);
		return;
	}

	if (count_charmed(ch))
		return;

	if (is_safe(ch, victim))
		return;

	WAIT_STATE(ch, skill_beats("control animal"));

	chance = get_skill(ch, "control animal");
	chance += (get_curr_stat(ch,STAT_CHA) - 20) * 5;
	chance += (ch->level - victim->level) * 3;
	chance +=
	(get_curr_stat(ch,STAT_INT) - get_curr_stat(victim,STAT_INT)) * 5;

	if (IS_AFFECTED(victim, AFF_CHARM)
	||  IS_AFFECTED(ch, AFF_CHARM)
	||  number_percent() > chance
	||  ch->level < (victim->level + 2)
	||  get_resist(victim, DAM_CHARM, TRUE) == 100
	||  (IS_NPC(victim) && victim->pMobIndex->pShop != NULL)) {
		check_improve(ch, "control animal", FALSE, 2);
		do_say(victim, "I'm not about to follow you!");
		do_murder(victim, ch->name);
		return;
	}

	check_improve(ch, "control animal", TRUE, 2);

	if (victim->master)
		stop_follower(victim);
	SET_BIT(victim->affected_by, AFF_CHARM);
	victim->master = victim->leader = ch;

	act("Isn't $n just so nice?", ch, NULL, victim, TO_VICT);
	if (ch != victim)
		act("$N looks at you with adoring eyes.",
		    ch, NULL, victim, TO_NOTVICT);
}

#define OBJ_VNUM_WOODEN_ARROW		6
#define OBJ_VNUM_GREEN_ARROW		34
#define OBJ_VNUM_RED_ARROW		35
#define OBJ_VNUM_WHITE_ARROW		36
#define OBJ_VNUM_BLUE_ARROW		37

DO_FUN(do_make_arrow, ch, argument)
{
	OBJ_DATA *arrow;
	int count, mana, wait;
	char arg[MAX_INPUT_LENGTH];
	int chance;
	int color_chance = 100;
	int vnum = -1;
	const char *color = NULL;

	if (IS_NPC(ch))
		return;

	if ((chance = get_skill(ch, "make arrow")) == 0) {
		act_char("You don't know how to make arrows.", ch);
		return;
	}

	if (ch->in_room->sector_type != SECT_FIELD
	&&  ch->in_room->sector_type != SECT_FOREST
	&&  ch->in_room->sector_type != SECT_HILLS) {
		act_char("You couldn't find enough wood.", ch);
		return;
	}

	mana = skill_mana(ch, "make arrow");
	wait = skill_beats("make arrow");

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0')
		vnum = OBJ_VNUM_WOODEN_ARROW;
	else if (!str_prefix(arg, "green")) {
		color = "green arrow";
		vnum = OBJ_VNUM_GREEN_ARROW;
	} else if (!str_prefix(arg, "red")) {
		color = "red arrow";
		vnum = OBJ_VNUM_RED_ARROW;
	} else if (!str_prefix(arg, "white")) {
		color = "white arrow";
		vnum = OBJ_VNUM_WHITE_ARROW;
	} else if (!str_prefix(arg, "blue")) {
		color = "blue arrow";
		vnum = OBJ_VNUM_BLUE_ARROW;
	}

	if (vnum < 0) {
		act_char("You don't know how to make that kind of arrows.", ch);
		return;
	}

	if (color != NULL) {
		color_chance = get_skill(ch, color);
		mana += skill_mana(ch, color);
		wait += skill_beats(color);
	}

	if (ch->mana < mana) {
		act_char("You don't have enough energy to make that kind of arrows.", ch);
		return;
	}

	ch->mana -= mana;
	WAIT_STATE(ch, wait);

	act("$n starts to make arrows!", ch, NULL, NULL, TO_ROOM);
	for (count = 0; count < LEVEL(ch) / 5; count++) {
		AFFECT_DATA *paf;

		if (number_percent() > chance * color_chance / 100) {
			act_char("You failed to make the arrow, and broke it.", ch);
			check_improve(ch, "make arrow", FALSE, 3);
			if (color != NULL)
				check_improve(ch, color, FALSE, 3);
			continue;
		}

		check_improve(ch, "make arrow", TRUE, 3);
		if (color != NULL)
			check_improve(ch, color, TRUE, 3);

		if ((arrow = create_obj(vnum, 0)) == NULL)
			return;
		arrow->level = ch->level;
		INT(arrow->value[1]) = 4 + LEVEL(ch) / 10;
		INT(arrow->value[2]) = 4 + LEVEL(ch) / 10;

		paf = aff_new(TO_OBJECT, "make arrow");
		paf->level	= ch->level;
		paf->duration	= -1;
		INT(paf->location) = APPLY_HITROLL;
		paf->modifier	= LEVEL(ch) / 10;
		affect_to_obj(arrow, paf);

		INT(paf->location) = APPLY_DAMROLL;
		affect_to_obj(arrow, paf);
		aff_free(paf);

		act("You successfully make $p.", ch, arrow, NULL, TO_CHAR);
		act("$n makes $p.", ch, arrow, NULL, TO_ROOM);
		obj_to_char_check(arrow, ch);
	}
}

#define OBJ_VNUM_RANGER_BOW		7

DO_FUN(do_make_bow, ch, argument)
{
	OBJ_DATA *	bow;
	AFFECT_DATA	*paf;
	int		mana;
	int		chance;

	if (IS_NPC(ch))
		return;

	if ((chance = get_skill(ch, "make bow")) == 0) {
		act_char("You don't know how to make bows.", ch);
		return;
	}

	if (ch->in_room->sector_type != SECT_FIELD
	&&  ch->in_room->sector_type != SECT_FOREST
	&&  ch->in_room->sector_type != SECT_HILLS) {
		act_char("You couldn't find enough wood.", ch);
		return;
	}

	mana = skill_mana(ch, "make bow");
	if (ch->mana < mana) {
		act_char("You don't have enough energy to make a bow.", ch);
		return;
	}
	ch->mana -= mana;
	WAIT_STATE(ch, skill_beats("make bow"));

	if (number_percent() > chance) {
		act_char("You failed to make the bow, and broke it.", ch);
		check_improve(ch, "make bow", FALSE, 1);
		return;
	}
	check_improve(ch, "make bow", TRUE, 1);

	if ((bow = create_obj(OBJ_VNUM_RANGER_BOW, 0)) == NULL)
		return;
	bow->level = ch->level;
	INT(bow->value[1]) = 4 + ch->level / 15;
	INT(bow->value[2]) = 4 + ch->level / 15;

	paf = aff_new(TO_OBJECT, "make bow");
	paf->level	= ch->level;
	paf->duration	= -1;
	INT(paf->location)= APPLY_HITROLL;
	paf->modifier	= LEVEL(ch) / 10;
	affect_to_obj(bow, paf);

	INT(paf->location)= APPLY_DAMROLL;
	affect_to_obj(bow, paf);
	aff_free(paf);

	act("You successfully make $p.", ch, bow, NULL, TO_CHAR);
	act("$n makes $p.", ch, bow, NULL, TO_ROOM);
	obj_to_char_check(bow, ch);
}

DO_FUN(do_make, ch, argument)
{
	char arg[MAX_INPUT_LENGTH];

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		act_char("You can make either bow or arrow.", ch);
		return;
	}

	if (!str_prefix(arg, "arrow"))
		do_make_arrow(ch, argument);
	else if (!str_prefix(arg, "bow"))
		do_make_bow(ch, argument);
	else
		do_make(ch, str_empty);
}

DO_FUN(do_homepoint, ch, argument)
{
        AFFECT_DATA *paf;
        char arg[MAX_INPUT_LENGTH];

        if (is_sn_affected(ch, "homepoint")) {
                act_char("You fatigue for searching new home.", ch);
                return;
        }

        if (IS_SET(ch->in_room->room_flags, ROOM_SAFE | ROOM_PEACE |
                                            ROOM_PRIVATE | ROOM_SOLITARY)
        ||  (ch->in_room->sector_type != SECT_FIELD
        &&   ch->in_room->sector_type != SECT_FOREST
        &&   ch->in_room->sector_type != SECT_MOUNTAIN
        &&   ch->in_room->sector_type != SECT_HILLS)) {
                act_char("You are cannot set home here.", ch);
                return;
        }

        if (ch->mana < ch->max_mana) {
                act_char("You don't have strength to make a new home.", ch);
                return;
        }

        ch->mana = 0;

        if (number_percent() > get_skill(ch, "homepoint")) {
                act_char("You failed to make your homepoint.", ch);
                check_improve(ch, "homepoint", FALSE, 4);
                return;
        }

        check_improve(ch, "homepoint", TRUE, 4);
        WAIT_STATE(ch, skill_beats("homepoint"));

        act_char("You succeeded to make your homepoint.", ch);
        act("$n succeeded to make $s homepoint. ", ch, NULL, NULL, TO_ROOM);

	paf = aff_new(TO_AFFECTS, "homepoint");
	paf->level	= ch->level;
	paf->duration	= 100;
	affect_to_char(ch, paf);
	aff_free(paf);

        argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] && !str_prefix(arg, "motherland"))
		PC(ch)->homepoint = NULL;
        else
		PC(ch)->homepoint = ch->in_room;
}

/*
 * static functions
 */

#define FLAG_SET(pos, c, exp) (buf[pos] = (exp) ? (c) : '.')

static void
show_char_to_char_0(CHAR_DATA *victim, CHAR_DATA *ch)
{
	const char *msg = str_empty;
	const void *arg = NULL;
	const void *arg3 = NULL;
	BUFFER *output;

	if (is_sn_affected(victim, "doppelganger")
	&&  (IS_NPC(ch) || !IS_SET(PC(ch)->plr_flags, PLR_HOLYLIGHT)))
		victim = victim->doppel;

	if (is_sn_affected(ch, "hallucination") && !IS_NPC(ch))
		victim = nth_char(victim, PC(ch)->random_value);

	output = buf_new(0);
	if (IS_NPC(victim)) {
		if (!IS_NPC(ch) && PC(ch)->questmob > 0
		&&  NPC(victim)->hunter == ch)
			buf_append(output, "{r[{RTARGET{r]{x ");
	} else {
		if (IS_WANTED(victim))
			buf_append(output, "({RWanted{x) ");

		if (IS_SET(victim->comm, COMM_AFK))
			buf_append(output, "{c[AFK]{x ");
	}

	if (IS_SET(ch->comm, COMM_LONG)) {
		if (HAS_INVIS(victim, ID_INVIS)
		|| HAS_INVIS(victim, ID_IMP_INVIS))
			buf_append(output, "({yInvis{x) ");
		if (HAS_INVIS(victim, ID_HIDDEN))
			buf_append(output, "({DHidden{x) ");
		if (IS_AFFECTED(victim, AFF_CHARM)
		&& (HAS_DETECT(ch, ID_CHARM) || victim->master == ch))
			buf_append(output, "({mCharmed{x) ");
		if (IS_AFFECTED(victim, AFF_PASS_DOOR))
			buf_append(output, "({cTranslucent{x) ");
		if (IS_AFFECTED(victim, AFF_FAERIE_FIRE))
			buf_append(output, "({MPink Aura{x) ");
		if (IS_SET(victim->form, FORM_UNDEAD)
		&&  HAS_DETECT(ch, ID_UNDEAD))
			buf_append(output, "({DUndead{x) ");
		if (RIDDEN(victim))
			buf_append(output, "({GRidden{x) ");
		if (IS_EVIL(victim) && HAS_DETECT(ch, ID_EVIL))
			buf_append(output, "({DUnholy Aura{x) ");
		if (IS_GOOD(victim) && HAS_DETECT(ch, ID_GOOD))
			buf_append(output, "({WHoly Aura{x) ");
		if (IS_AFFECTED(victim, AFF_SANCTUARY))
			buf_append(output, "({WWhite Aura{x) ");
		if (IS_AFFECTED(victim, AFF_BLACK_SHROUD))
			buf_append(output, "({DBlack Aura{x) ");
		if (is_sn_affected(victim, "golden aura"))
			buf_append(output, "({YGolden Aura{x) ");
		if (HAS_INVIS(victim, ID_FADE))
			buf_append(output, "({yFade{x) ");
		if (HAS_INVIS(victim, ID_CAMOUFLAGE))
			buf_append(output, "({gCamf{x) ");
		if (HAS_INVIS(victim, ID_BLEND))
			buf_append(output, "({gBlending{x) ");
		if (IS_SET(ch->comm, COMM_SHOWRACE)) {
			buf_act(output, BUF_END, "({c$t{x) ",	// notrans
				NULL, victim->race, NULL, ACT_NOLF);
		}
	} else {
		static char FLAGS[] = "{x[{y.{D.{m.{c.{M.{D.{G.{b.{x.{Y.{W.{y.{g.{g."; // notrans
		char buf[sizeof(FLAGS)];
		bool diff;

		strlcpy(buf, FLAGS, sizeof(buf));
		FLAG_SET( 5, 'I', HAS_INVIS(victim, ID_INVIS)
		|| HAS_INVIS(victim, ID_IMP_INVIS));
		FLAG_SET( 8, 'H', HAS_INVIS(victim, ID_HIDDEN));
		FLAG_SET(11, 'C', IS_AFFECTED(victim, AFF_CHARM)
				  && (HAS_DETECT(ch, ID_CHARM)
				  || victim->master == ch));
		FLAG_SET(14, 'T', IS_AFFECTED(victim, AFF_PASS_DOOR));
		FLAG_SET(17, 'P', IS_AFFECTED(victim, AFF_FAERIE_FIRE));
		FLAG_SET(20, 'U', IS_SET(victim->form, FORM_UNDEAD) &&
				  HAS_DETECT(ch, ID_UNDEAD));
		FLAG_SET(23, 'R', RIDDEN(victim));
		if (HAS_DETECT(ch, ID_EVIL) && IS_EVIL(victim)) {
			FLAG_SET(28, 'D', TRUE);
			FLAG_SET(29, 'E', TRUE);
		} else if (HAS_DETECT(ch, ID_GOOD) && IS_GOOD(victim)) {
			FLAG_SET(28, 'W', TRUE);
			FLAG_SET(29, 'G', TRUE);
		}
		FLAG_SET(32, 'G', is_sn_affected(victim, "golden aura"));
		FLAG_SET(35, 'S', IS_AFFECTED(victim, AFF_SANCTUARY));

		if (IS_AFFECTED(victim, AFF_BLACK_SHROUD)) {
			FLAG_SET(35, 'B', TRUE);
			FLAG_SET(34, 'D', TRUE);
		}


		FLAG_SET(38, 'F', HAS_INVIS(victim, ID_FADE));
		FLAG_SET(41, 'C', HAS_INVIS(victim, ID_CAMOUFLAGE));
		FLAG_SET(44, 'B', HAS_INVIS(victim, ID_BLEND));

		diff = strcmp(buf, FLAGS);
		if (diff)
			buf_append(output, buf);
		else if (IS_SET(ch->comm, COMM_SHOWRACE))
			buf_append(output, "{x[");		// notrans

		if (IS_SET(ch->comm, COMM_SHOWRACE)) {
			buf_act(output, BUF_END, "{c$t{x] ",	// notrans
				NULL, victim->race, NULL, ACT_NOLF);
		} else if (diff)
			buf_append(output, "{x] ");		// notrans
	}

	if (victim->invis_level >= LEVEL_HERO)
		buf_append(output, "[{WWizi{x] ");
	if (victim->incog_level >= LEVEL_HERO)
		buf_append(output, "[{DIncog{x] ");

	act_puts("$t", ch, buf_string(output), NULL,
		 TO_CHAR | ACT_NOTRANS | ACT_NOLF, POS_DEAD);

	if (IS_NPC(victim)
	&&  victim->position == victim->pMobIndex->start_pos) {
		act_puts(format_long(&victim->long_descr, ch),
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		return;
	}

	if (victim->shapeform && victim->position == POS_STANDING) {
		act_puts(format_long(&victim->shapeform->index->long_desc, ch),
			 ch, NULL, NULL, TO_CHAR | ACT_NOLF, POS_DEAD);
		return;
	}

	if (IS_IMMORTAL(victim)) {
		act_puts("{W", ch, NULL, NULL,			// notrans
			 TO_CHAR | ACT_NOLF, POS_DEAD);
	} else {
		act_puts("{x", ch, NULL, NULL,			// notrans
			 TO_CHAR | ACT_NOLF, POS_DEAD);
	}

	switch (victim->position) {
	case POS_DEAD:
		msg = "$N {xis DEAD!!";
		break;

	case POS_MORTAL:
		msg = "$N {xis mortally wounded.";
		break;

	case POS_INCAP:
		msg = "$N {xis incapacitated.";
		break;

	case POS_STUNNED:
		msg = "$N {xis lying here stunned.";
		break;

	case POS_SLEEPING:
		if (victim->on == NULL) {
			msg = "$N {xis sleeping here.";
			break;
		}

		arg = victim->on;
		if (IS_SET(INT(victim->on->value[2]), SLEEP_AT))
			msg = "$N {xis sleeping at $p.";
		else if (IS_SET(INT(victim->on->value[2]), SLEEP_ON))
			msg = "$N {xis sleeping on $p.";
		else
			msg = "$N {xis sleeping in $p.";
		break;

	case POS_RESTING:
		if (victim->on == NULL) {
			msg = "$N {xis resting here.";
			break;
		}

		arg = victim->on;
		if (IS_SET(INT(victim->on->value[2]), REST_AT))
			msg = "$N {xis resting at $p.";
		else if (IS_SET(INT(victim->on->value[2]), REST_ON))
			msg = "$N {xis resting on $p.";
		else
			msg = "$N {xis resting in $p.";
		break;

	case POS_SITTING:
		if (victim->on == NULL) {
			msg = "$N {xis sitting here.";
			break;
		}

		arg = victim->on;
		if (IS_SET(INT(victim->on->value[2]), SIT_AT))
			msg = "$N {xis sitting at $p.";
		else if (IS_SET(INT(victim->on->value[2]), SIT_ON))
			msg = "$N {xis sitting on $p.";
		else
			msg = "$N {xis sitting in $p.";
		break;

	case POS_STANDING:
		if (victim->on == NULL) {
			if (!IS_NPC(victim)
			&&  !IS_SET(ch->comm, COMM_BRIEF))
				arg = PC(victim)->title;
			else
				arg = str_empty;

			if (MOUNTED(victim)) {
				arg3 = MOUNTED(victim);
				msg = "$N{x$t {xis here, riding $I.";
			}
			else
				msg = "$N{x$t {xis here.";
			break;
		}

		arg = victim->on;
		if (IS_SET(INT(victim->on->value[2]), STAND_AT))
			msg = "$N {xis standing at $p.";
		else if (IS_SET(INT(victim->on->value[2]), STAND_ON))
			msg = "$N {xis standing on $p.";
		else
			msg = "$N {xis standing in $p.";
		break;

	case POS_FIGHTING:
		if (victim->fighting == NULL) {
			arg = NULL;
			msg = "$N {xis here, fighting with thin air??";
		}
		else if (victim->fighting == ch) {
			arg = NULL;
			msg = "$N {xis here, fighting with YOU!";
		}
		else if (victim->in_room == victim->fighting->in_room) {
			arg = victim->fighting;
			msg = "$N {xis here, fighting with $i.";
		}
		else {
			arg = NULL;
			msg = "$N {xis here, fighting with someone who left??";
		}
		break;
	}

	act_puts3(msg, ch, arg, victim, arg3, TO_CHAR | ACT_FORMSH, POS_DEAD);
}

static void
show_char_to_char(CHAR_DATA *list, CHAR_DATA *ch)
{
	CHAR_DATA *rch;
	int life_count = 0;

	for (rch = list; rch; rch = rch->next_in_room) {
		if (rch == ch
		||  (!IS_TRUSTED(ch, rch->incog_level) &&
		     ch->in_room != rch->in_room))
			continue;

		if (!IS_TRUSTED(ch, rch->invis_level)) {
			AREA_DATA *pArea;

			if (!IS_NPC(rch))
				continue;

			pArea = area_vnum_lookup(rch->pMobIndex->vnum);
			if (pArea == NULL
			||  !IS_BUILDER(ch, pArea))
				continue;
		}

		if (can_see(ch, rch))
			show_char_to_char_0(rch, ch);
		else {
			if (char_in_dark_room(ch) && HAS_DETECT(rch, ID_INFRARED))
				act_char("You see {rglowing red eyes{x watching YOU!", ch);
			life_count++;
		}
	}

	if (list && list->in_room == ch->in_room
	&&  life_count
	&&  HAS_DETECT(ch, ID_LIFE))
		act_puts("You feel $j more life $qj{forms} in the room.",
			 ch, (const void*) life_count, NULL,
			 TO_CHAR, POS_DEAD);
}

#define CMD_ALLOWED(cmd, ch)						\
	((cmd)->min_level < LEVEL_HERO &&				\
	 (cmd)->min_level <= (ch)->level &&				\
	 (IS_NULLSTR((cmd)->sn) || get_skill((ch), (cmd)->sn) != 0))

#define WIZCMD_ALLOWED(cmd, ch)						\
	((cmd)->min_level >= LEVEL_IMMORTAL &&				\
	 ((ch)->level >= LEVEL_IMP ||					\
	  is_name((cmd)->name, PC(ch)->granted)))

static void
show_aliases(CHAR_DATA *ch, const char *argument, bool wiz)
{
	cmd_t *cmd;
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *vch = wiz ? GET_ORIGINAL(ch) : ch;

	one_argument(argument, arg, sizeof(arg));
	C_FOREACH(cmd, &commands) {
		char a[MAX_INPUT_LENGTH];
		const char *p;

		if (wiz ? !WIZCMD_ALLOWED(cmd, vch) : !CMD_ALLOWED(cmd, vch))
			continue;

		if (!str_prefix(arg, cmd->name)) {
			if (!IS_NULLSTR(cmd->aliases)) {
				act_puts("Aliases for '$t' are: [$T]",
				    ch, cmd->name, cmd->aliases,
				    TO_CHAR | ACT_NOTRANS, POS_DEAD);
			} else {
				act_puts("$Tcommand '$t' does not have aliases.",
				    ch, cmd->name, wiz ? "wiz" : "",
				    TO_CHAR | ACT_NOTRANS, POS_DEAD);
			}

			return;
		}

		p = one_argument(cmd->aliases, a, sizeof(a));
		for (; a[0] != '\0'; p = one_argument(p, a, sizeof(a))) {
			if (!str_prefix(arg, a)) {
				act_puts("'$t' as an alias for '$T'.",
				    ch, a, cmd->name,
				    TO_CHAR | ACT_NOTRANS, POS_DEAD);
				return;
			}
		}
	}

	act_puts("$t: No such $Tcommand or alias found.",
	    ch, arg, wiz ? "wiz" : "",
	    TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
}

static void
show_commands(CHAR_DATA *ch, const char *argument, bool wiz)
{
	int col;
	varr v;
	cmd_t *cmd;
	CHAR_DATA *vch = wiz ? GET_ORIGINAL(ch) : ch;

	if (!IS_NULLSTR(argument)) {
		show_aliases(ch, argument, wiz);
		return;
	}

	c_init(&v, &c_info_commands);
	C_FOREACH(cmd, &commands) {
		if (wiz ? WIZCMD_ALLOWED(cmd, vch) :
			  CMD_ALLOWED(cmd, vch) &&
			  !IS_SET(cmd->cmd_flags, CMD_HIDDEN)) {
			cmd_t *ncmd = varr_enew(&v);
			*ncmd = *cmd;
		}
	}
	varr_qsort(&v, cmpstr);

	col = 0;
	C_FOREACH(cmd, &v) {
		act_puts("$f-12{$t}", ch, cmd->name, NULL,   // notrans
			 TO_CHAR | ACT_NOTRANS | ACT_NOLF | ACT_NOUCASE,
			 POS_DEAD);
		if (++col % 6 == 0)
			send_to_char("\n", ch);
	}

	if (col % 6 != 0)
		send_to_char("\n", ch);

	varr_destroy_nd(&v);
}

DO_FUN(do_commands, ch, argument)
{
	show_commands(ch, argument, FALSE);
}

DO_FUN(do_wizhelp, ch, argument)
{
	show_commands(ch, argument, TRUE);
}

static void
show_clanlist(CHAR_DATA *ch, clan_t *clan,
	      const char *list, const char *name_list)
{
	BUFFER *output;
	char name[MAX_STRING_LENGTH];
	int cnt = 0;
	CHAR_DATA *vch;

	output = buf_new(0);
	buf_printf(output, BUF_END, "List of %s of %s:\n", name_list, clan->name);
	buf_append(output, "Status   Level Race  Class   Ethos-align   Name\n");
	buf_append(output, "-------- ----- ----- ----- --------------- -------------\n");	// notrans

	list = first_arg(list, name, sizeof(name), FALSE);
	for (; name[0]; list = first_arg(list, name, sizeof(name), FALSE)) {
		class_t *cl;
		race_t *r;

		if ((vch = char_load(name, LOAD_F_NOCREATE)) == NULL) {
			buf_printf(output, BUF_END, "[{RInvalid entry{x] %s (report this to immortals)\n", name);
			continue;
		}

		if (str_cmp(vch->clan, clan->name)) {
			buf_printf(output, BUF_END, "[{RInvalid entry{x] %s (report this to immortals)\n", vch->name);
			goto cleanup;
		}

		cnt++;
		r = race_lookup(vch->race);
		cl = class_lookup(vch->class);
		buf_printf(output, BUF_END, "%-8s  %3d  %-5s  %-3s  %7s-%-7s %s\n", // notrans
			flag_string(clan_status_table, PC(vch)->clan_status),
			vch->level,
			r && r->race_pcdata ? r->race_pcdata->who_name : "none",
			cl ? cl->who_name : "none",
			flag_string(ethos_table, PC(vch)->ethos),
			flag_string(align_names, NALIGN(vch)),
			vch->name);

cleanup:
		char_nuke(vch);
	}

	if (!cnt)
		buf_append(output, "None.\n");

	page_to_char(buf_string(output), ch);
	buf_free(output);
}

DO_FUN(do_clanlist, ch, argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	clan_t *clan = NULL;

	argument = one_argument(argument, arg1, sizeof(arg1));
		   one_argument(argument, arg2, sizeof(arg2));

	if (IS_IMMORTAL(ch) && arg2[0] != '\0') {
		if ((clan = clan_search(arg2)) == NULL) {
			act_puts("$t: no such clan.",
				 ch, arg2, NULL,
				 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
			return;
		}
	}

	if (!clan && (clan = clan_lookup(ch->clan)) == NULL) {
		act_char("You are not in a clan.", ch);
		return;
	}

	if (arg1[0] == '\0' || !str_prefix(arg1, "member")) {
		show_clanlist(ch, clan, clan->member_list, "members");
		return;
	}

	if (!str_prefix(arg1, "leader")) {
		show_clanlist(ch, clan, clan->leader_list, "leaders");
		return;
	}

	if (!str_prefix(arg1, "second")) {
		show_clanlist(ch, clan, clan->second_list, "secondaries");
		return;
	}

	do_clanlist(ch, str_empty);
}

DO_FUN(do_item, ch, argument)
{
	clan_t* clan = NULL;
	OBJ_DATA* in_obj;
	char arg[MAX_STRING_LENGTH];

	one_argument(argument, arg, sizeof(arg));
	if (IS_IMMORTAL(ch) && arg[0] != '\0') {
		if ((clan = clan_search(arg)) == NULL) {
			act_puts("$t: no such clan.",
				 ch, arg, NULL,
				 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
			return;
		}
	}

	if (!clan && (clan = clan_lookup(ch->clan)) == NULL) {
		act_char("You are not in clan, you should not worry about your clan item.", ch);
		return;
	}

	if (clan->obj_ptr == NULL) {
		act_char("Your clan do not have an item of power.", ch);
		return;
	}

	for (in_obj = clan->obj_ptr; in_obj->in_obj; in_obj = in_obj->in_obj)
		;

	if (in_obj->carried_by) {
		act_puts3("$p is in $R, carried by $N.",
			  ch, clan->obj_ptr, in_obj->carried_by,
			  in_obj->carried_by->in_room,
			  TO_CHAR, POS_DEAD);
	} else if (in_obj->in_room) {
		act_puts3("$p is in $R.",
			  ch, clan->obj_ptr, NULL, in_obj->in_room,
			  TO_CHAR, POS_DEAD);
		C_FOREACH(clan, &clans) {
			if (in_obj->in_room->vnum == clan->altar_vnum) {
				act_puts("It is altar of $t.",
				    ch, clan->name, NULL, TO_CHAR, POS_DEAD);
				break;
			}
		}
	} else {
		act_puts("$p is somewhere.",
			 ch, clan->obj_ptr, NULL, TO_CHAR, POS_DEAD);
	}
}

DO_FUN(do_rating, ch, argument)
{
	int i;

	act_char("Name                    | PC's killed", ch);
	act_char("------------------------+------------", ch);	// notrans

	for (i = 0; i < RATING_TABLE_SIZE; i++) {
		if (rating_table[i].name == NULL)
			continue;
		act_puts("$f-24{$T}| $j", ch,		// notrans
			 (const void *) rating_table[i].pc_killed,
			 rating_table[i].name,
			 TO_CHAR | ACT_NOTRANS, POS_DEAD);
	}
}

#define SKIP_CLOSED(pArea)						\
	{								\
		while (pArea && IS_SET(pArea->area_flags, AREA_CLOSED))	\
			pArea = pArea->next;				\
	}

DO_FUN(do_areas, ch, argument)
{
	AREA_DATA *pArea1;
	AREA_DATA *pArea2;
	int iArea;
	int iAreaHalf;
	int maxArea = 0;
	BUFFER *output;

	if (argument[0] != '\0') {
		act_char("No argument is used with this command.", ch);
		return;
	}

	/*
	 * count total number of areas, skipping closed ones
	 */
	for (pArea1 = area_first; pArea1 != NULL; pArea1 = pArea1->next)
		if (!IS_SET(pArea1->area_flags, AREA_CLOSED))
			maxArea++;

	/*
	 * move pArea2 to the half of area list (skipping closed ones)
	 * pArea2 can't be NULL after SKIP_CLOSED because iArea < iAreaHalf
	 */
	iAreaHalf = (maxArea + 1) / 2;
	pArea1 = pArea2 = area_first;
	for (iArea = 0; iArea < iAreaHalf; iArea++) {
		SKIP_CLOSED(pArea2);
		pArea2 = pArea2->next;
	}

	/*
	 * print areas list
	 * pArea1 can't be NULL after SKIP_CLOSED because iArea < iAreaHalf
	 */
	output = buf_new(GET_LANG(ch));
	buf_append(output, "Current areas of Shades of Gray: \n");
	for (iArea = 0; iArea < iAreaHalf; iArea++) {
		SKIP_CLOSED(pArea1);
		SKIP_CLOSED(pArea2);

		buf_printf(output, BUF_END,
			   "{{%2d %3d} {B%-20.20s{x %8.8s ",	  // notrans
			   pArea1->min_level, pArea1->max_level,
			   pArea1->name,
			   pArea1->credits);

		if (pArea2 != NULL) 
			buf_printf(output, BUF_END,"{{%2d %3d} {B%-20.20s{x %8.8s",	// notrans
				pArea2->min_level, pArea2->max_level,
				pArea2->name,
				pArea2->credits);
		buf_append(output, "\n");

		pArea1 = pArea1->next;
		if (pArea2 != NULL)
			pArea2 = pArea2->next;
	}

	buf_printf(output, BUF_END, "\n%d areas total.\n", maxArea);
	page_to_char(buf_string(output), ch);
	buf_free(output);
}

DO_FUN(do_compress, ch, argument)
{
	char arg1[MAX_INPUT_LENGTH];
	double crate;
	const char *status;
	const char *support;
	BUFFER *output;


	if (!ch->desc) {
		act_char("What descriptor?!", ch);
		return;
	}

	one_argument(argument, arg1, sizeof(arg1));

	if (!str_cmp(arg1, "on") && ch->desc->mccp_support) {
    		if (ch->desc->out_compress == NULL) {
			if (!compressStart(ch->desc))
				act_char("Failed.", ch);
			else
				act_char("Ok, compression enabled.", ch);
		}
		else
			act_char("Compression is already enabled.", ch);

		return;
	}

	if (!str_cmp(arg1, "off") && ch->desc->mccp_support) {
		if (ch->desc->out_compress != NULL) {
			if (!compressEnd(ch->desc))
				act_char("Failed.", ch);
			else
				act_char("Ok, compression disabled.", ch);
		}
		else
			act_char("Compression is already disabled.", ch);

		return;
	}

	/*
	 *  Default - show statistics
	 */
	output = buf_new(0);
	
	crate = 100 * (1 - (double)ch->desc->bytes_sent/(double)ch->desc->bytes_income);

	switch (ch->desc->mccp_support) {
	case 1:
		support = "mccp version 1 supported by client";
		break;
	case 2:
		support = "mccp version 2 supported by client";
		break;
	default:
		support = "mccp not supported by client";
		break;
	}


	if (ch->desc->out_compress != NULL)
		status = "enabled";
	else
		status = "disabled";
    
	buf_printf(output, BUF_END,
			"Support: %s\n"
			"Status:  %s\n\n"
			"Total bytes.........%-d\n"
			"Sent bytes..........%-d\n"
			"Compression rate....%-4.2f%%\n",
			GETMSG(support, GET_LANG(ch)),
			GETMSG(status, GET_LANG(ch)),
			ch->desc->bytes_income, ch->desc->bytes_sent, crate);

	page_to_char(buf_string(output), ch);
	buf_free(output);
}
