/*
 * $Id: act_info.c,v 1.374 2001-06-22 07:13:34 avn Exp $
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

#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#if !defined(WIN32)
#	include <unistd.h>
#endif
#include <ctype.h>

#include "merc.h"
#include "db.h"
#include "obj_prog.h"
#include "socials.h"
#include "rating.h"
#include "string_edit.h"

#include "fight.h"
#include "magic.h"
#include "update.h"
#include "quest.h"

#if defined(SUNOS) || defined(SVR4) || defined(LINUX)
#	include <crypt.h>
#endif

/* command procedures needed */
DECLARE_DO_FUN(do_exits		);
DECLARE_DO_FUN(do_look		);
DECLARE_DO_FUN(do_help		);
DECLARE_DO_FUN(do_affects	);
DECLARE_DO_FUN(do_murder	);
DECLARE_DO_FUN(do_say		);

/*
 * Local functions.
 */
static char *	format_obj_to_char	(OBJ_DATA *obj, CHAR_DATA *ch,
					 bool fShort);
static void	show_list_to_char	(OBJ_DATA *list, CHAR_DATA *ch,
					 bool fShort, bool fShowNothing);
static void	show_char_to_char_0	(CHAR_DATA *victim, CHAR_DATA *ch);
static void	show_char_to_char_1	(CHAR_DATA *victim, CHAR_DATA *ch);
static void	show_char_to_char	(CHAR_DATA *list, CHAR_DATA *ch);
static void	show_obj_to_char	(CHAR_DATA *ch, OBJ_DATA *obj,
					 flag_t wear_loc);
static void list_spells(flag_t type, CHAR_DATA *ch, const char *argument);

static int show_order[] = {
	WEAR_LIGHT,
	WEAR_FINGER_L,
	WEAR_FINGER_R,
	WEAR_NECK,
	WEAR_FACE,
	WEAR_BODY,
	WEAR_HEAD,
	WEAR_LEGS,
	WEAR_FEET,
	WEAR_HANDS,
	WEAR_ARMS,
	WEAR_SHIELD,
	WEAR_ABOUT,
	WEAR_WAIST,
	WEAR_WRIST_L,
	WEAR_WRIST_R,
	WEAR_WIELD,
	WEAR_SECOND_WIELD,
	WEAR_HOLD,
	WEAR_FLOAT,
	WEAR_TATTOO,
	WEAR_CLANMARK,
	-1
};

void do_clear(CHAR_DATA *ch, const char *argument)
{
	if (!IS_NPC(ch))
		send_to_char("\033[0;0H\033[2J", ch);	// notrans
}

/* changes your scroll */
void do_scroll(CHAR_DATA *ch, const char *argument)
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

void
do_socials(CHAR_DATA *ch, const char *argument)
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
		// XXX should skip mobs with act triggers :)
	} while (mob_index == NULL);
	mob = create_mob(mob_index, 0);

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
void do_motd(CHAR_DATA *ch, const char *argument)
{
	do_help(ch, "motd");
}

void do_imotd(CHAR_DATA *ch, const char *argument)
{
	do_help(ch, "imotd");
}

void do_rules(CHAR_DATA *ch, const char *argument)
{
	do_help(ch, "rules");
}

void do_wizlist(CHAR_DATA *ch, const char *argument)
{
	do_help(ch, "wizlist");
}

/* RT this following section holds all the auto commands from ROM, as well as
   replacements for config */
#define do_print_sw(ch, swname, sw)					\
		act_puts("$F16{$t} $T",					\
			 (ch), (swname), (sw) ? "ON" : "OFF",		\
			 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);

void do_autolist(CHAR_DATA *ch, const char *argument)
{
	/* lists most player flags */
	if (IS_NPC(ch))
		return;

	act_char("action         status", ch);
	act_char("---------------------", ch);			// notrans
	do_print_sw(ch, "autoassist", IS_SET(PC(ch)->plr_flags, PLR_AUTOASSIST));
	do_print_sw(ch, "autoexit", IS_SET(PC(ch)->plr_flags, PLR_AUTOEXIT));
	do_print_sw(ch, "autogold", IS_SET(PC(ch)->plr_flags, PLR_AUTOGOLD));
	do_print_sw(ch, "autolook", IS_SET(PC(ch)->plr_flags, PLR_AUTOLOOK));
	do_print_sw(ch, "autoloot", IS_SET(PC(ch)->plr_flags, PLR_AUTOLOOT));
	do_print_sw(ch, "autosac", IS_SET(PC(ch)->plr_flags, PLR_AUTOSAC));
	do_print_sw(ch, "autosplit", IS_SET(PC(ch)->plr_flags, PLR_AUTOSPLIT));

	if (IS_SET(PC(ch)->plr_flags, PLR_NOSUMMON))
		act_char("You can only be summoned players within your PK range.",ch);
	else
		act_char("You can be summoned by anyone.", ch);

	if (IS_SET(PC(ch)->plr_flags, PLR_NOFOLLOW))
		act_char("You do not welcome followers.", ch);
	else
		act_char("You accept followers.", ch);
}

void do_autoassist(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch)) {
		act_char("Huh?", ch);
		return;
	}

	TOGGLE_BIT(PC(ch)->plr_flags, PLR_AUTOASSIST);
	if (IS_SET(PC(ch)->plr_flags, PLR_AUTOASSIST))
		act_char("You will now assist when needed.", ch);
	else
		act_char("Autoassist removed.", ch);
}

void do_autoexit(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch)) {
		act_char("Huh?", ch);
		return;
	}

	TOGGLE_BIT(PC(ch)->plr_flags, PLR_AUTOEXIT);
	if (IS_SET(PC(ch)->plr_flags, PLR_AUTOEXIT))
		act_char("Exits will now be displayed.", ch);
	else 
		act_char("Exits will no longer be displayed.", ch);
}

void do_autogold(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch)) {
		act_char("Huh?", ch);
		return;
	}

	TOGGLE_BIT(PC(ch)->plr_flags, PLR_AUTOGOLD);
	if (IS_SET(PC(ch)->plr_flags, PLR_AUTOGOLD))
		act_char("Automatic gold looting set.", ch);
	else 
		act_char("Autogold removed.", ch);
}

void do_autolook(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch)) {
		act_char("Huh?", ch);
		return;
	}

	TOGGLE_BIT(PC(ch)->plr_flags, PLR_AUTOLOOK);
	if (IS_SET(PC(ch)->plr_flags, PLR_AUTOLOOK))
		act_char("Automatic corpse examination set.", ch);
	else
		act_char("Autolooking removed.", ch);
}

void do_autoloot(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch)) {
		act_char("Huh?", ch);
		return;
	}

	TOGGLE_BIT(PC(ch)->plr_flags, PLR_AUTOLOOT);
	if (IS_SET(PC(ch)->plr_flags, PLR_AUTOLOOT))
		act_char("Automatic corpse looting set.", ch);
	else
		act_char("Autolooting removed.", ch);
}

void do_autosac(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch)) {
		act_char("Huh?", ch);
		return;
	}

	TOGGLE_BIT(PC(ch)->plr_flags, PLR_AUTOSAC);
	if (IS_SET(PC(ch)->plr_flags, PLR_AUTOSAC))
		act_char("Automatic corpse sacrificing set.", ch);
	else
		act_char("Autosacrificing removed.", ch);
}

void do_autosplit(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch)) {
		act_char("Huh?", ch);
		return;
	}

	TOGGLE_BIT(PC(ch)->plr_flags, PLR_AUTOSPLIT);
	if (IS_SET(PC(ch)->plr_flags, PLR_AUTOSPLIT))
		act_char("Automatic gold splitting set.", ch);
	else
		act_char("Autosplitting removed.", ch);
}

void do_prompt(CHAR_DATA *ch, const char *argument)
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

void do_nofollow(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch)) {
		act_char("Huh?", ch);
		return;
	}

	TOGGLE_BIT(PC(ch)->plr_flags, PLR_NOFOLLOW);
	if (IS_SET(PC(ch)->plr_flags,PLR_NOFOLLOW)) {
		act_char("You no longer accept followers.", ch);
		die_follower(ch);
	}
	else
		act_char("You now accept followers.", ch);
}

void do_nosummon(CHAR_DATA *ch, const char *argument)
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

static void do_look_in(CHAR_DATA* ch, const char *argument)
{
	OBJ_DATA *obj;
	liquid_t *lq;
	clan_t *clan;

	if ((obj = get_obj_here(ch, argument)) == NULL) {
		act_char("You don't see that here.", ch);
		return;
	}

	switch (obj->item_type) {
	default:
		act_char("That is not a container.", ch);
		break;
	case ITEM_DRINK_CON:
		if ((lq = liquid_lookup(STR(obj->value[2]))) == NULL) {
			log(LOG_BUG, "Do_look_in: bad liquid %s.", STR(obj->value[2]));
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
		show_list_to_char(obj->contains, ch, TRUE, TRUE);
		break;
	}
}

#define LOOK_F_NORDESC	(A)

static void do_look_room(CHAR_DATA *ch, int flags)
{
	if (!room_is_dark(ch)
	&&  check_blind_raw(ch)) {
		const char *name;
		const char *engname;

		name = mlstr_cval(&ch->in_room->name, ch);
		engname = mlstr_mval(&ch->in_room->name);
		act_puts("{W$t", ch, name, NULL, 		// notrans
			 TO_CHAR | ACT_NOTRANS | ACT_NOLF, POS_DEAD);
		if (GET_LANG(ch) && name != engname) {
			act_puts(" ($t){x", ch, engname, NULL,	// notrans
				 TO_CHAR | ACT_NOTRANS | ACT_NOLF, POS_DEAD);
		} else {
			send_to_char("{x", ch);
		}
		
		if (IS_IMMORTAL(ch)
		||  IS_BUILDER(ch, ch->in_room->area)) {
			act_puts(" [Room $j]", ch,
				 (const void *) ch->in_room->vnum, NULL,
				 TO_CHAR | ACT_NOLF, POS_DEAD);
		}

		send_to_char("\n", ch);

 		if (!IS_SET(flags, LOOK_F_NORDESC)) {
			send_to_char("  ", ch);			// notrans
			act_puts(mlstr_cval(&ch->in_room->description, ch),
				 ch, NULL, NULL, TO_CHAR | ACT_NOLF, POS_DEAD);
		}

		if (!IS_NPC(ch) && IS_SET(PC(ch)->plr_flags, PLR_AUTOEXIT)) {
			send_to_char("\n", ch);
			do_exits(ch, "auto");
		}
	}
	else 
		act_char("It is pitch black...", ch);

	show_list_to_char(ch->in_room->contents, ch, FALSE, FALSE);
	show_char_to_char(ch->in_room->people, ch);
}

void do_glance(CHAR_DATA *ch, const char *argument)
{
	do_look_room(ch, LOOK_F_NORDESC);
}

void do_look(CHAR_DATA *ch, const char *argument)
{
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	char arg3 [MAX_INPUT_LENGTH];
	EXIT_DATA *pexit;
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	ED_DATA *ed;
	int door;
	int number,count;

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
		do_look_room(ch, arg1[0] == '\0' ||
			 	 (!IS_NPC(ch) && !IS_SET(ch->comm, COMM_BRIEF)) ?
				  0 : LOOK_F_NORDESC);
		return;
	}

	if (!check_blind(ch))
		return;

	if (!str_cmp(arg1, "i")
	||  !str_cmp(arg1, "in")
	||  !str_cmp(arg1,"on")) {
		/* 'look in' */
		if (arg2[0] == '\0') {
			act_char("Look in what?", ch);
			return;
		}

		do_look_in(ch, arg2);
		return;
	}

	if ((victim = get_char_room(ch, arg1)) != NULL) {
		show_char_to_char_1(victim, ch);

		/* Love potion */
		if (is_affected(ch, "love potion") && (victim != ch)) {
			AFFECT_DATA af;

			affect_strip(ch, "love potion");

			add_follower(ch, victim);
			set_leader(ch, victim);

			af.where = TO_AFFECTS;
			af.type = "charm person";
			af.level = ch->level;
			af.duration =  number_fuzzy(victim->level / 4);
			af.bitvector = AFF_CHARM;
			af.modifier = 0;
			af.owner = NULL;
			INT(af.location) = 0;
			affect_to_char(ch, &af);

			act("Isn't $n just so nice?",
			    victim, NULL, ch, TO_VICT);
			act("$N looks at you with adoring eyes.",
			    victim, NULL, ch, TO_CHAR);
			act("$N looks at $n with adoring eyes.",
			    victim, NULL, ch, TO_NOTVICT);
		}

		return;
	}

	for (obj = ch->carrying; obj != NULL; obj = obj->next_content) {
		if (can_see_obj(ch, obj)) {
			/* player can see object */
			ed = ed_lookup(arg3, obj->ed);
			if (ed != NULL) {
				if (++count == number) {
					act_puts(mlstr_cval(&ed->description, ch),
						 ch, NULL, NULL,
						 TO_CHAR | ACT_NOLF, POS_DEAD);
					return;
				} else
					continue;
			}

			ed = ed_lookup(arg3, obj->pObjIndex->ed);

			if (ed != NULL) {
				if (++count == number) {
					act_puts(mlstr_cval(&ed->description, ch),
						 ch, NULL, NULL,
						 TO_CHAR | ACT_NOLF, POS_DEAD);
					return;
				} else
					continue;
			}

			if (IS_OBJ_NAME(obj, arg3))
				if (++count == number) {
					act_char("You see nothing special about it.", ch);
					return;
				}
		}
	}

	for (obj = ch->in_room->contents;
	     obj != NULL; obj = obj->next_content) {
		if (can_see_obj(ch, obj)) {
			ed = ed_lookup(arg3, obj->ed);
			if (ed != NULL)
				if (++count == number) {
					act_puts(mlstr_cval(&ed->description, ch),
						 ch, NULL, NULL,
						 TO_CHAR | ACT_NOLF, POS_DEAD);
					return;
				}

			ed = ed_lookup(arg3, obj->pObjIndex->ed);
			if (ed != NULL)
				if (++count == number) {
					act_puts(mlstr_cval(&ed->description, ch),
						 ch, NULL, NULL,
						 TO_CHAR | ACT_NOLF, POS_DEAD);
					return;
				}
		}

		if (IS_OBJ_NAME(obj, arg3))
			if (++count == number) {
				act_puts(format_long(&obj->description, ch),
					 ch, NULL, NULL, TO_CHAR, POS_DEAD);
				return;
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

	if ((door = door_lookup(ch, arg1)) < 0)
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

void do_examine(CHAR_DATA *ch, const char *argument)
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

	if ((obj = get_obj_here(ch, arg)) == NULL)
		return;

	switch (obj->item_type) {
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
void do_exits(CHAR_DATA *ch, const char *argument)
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
		&&  check_blind_raw(ch)) { 
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
				    	room_dark(pexit->to_room.r) ?
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

	send_to_char(buf_string(buf), ch);
	buf_free(buf);

	if (check_perception)
		check_improve(ch, "perception", TRUE, 5);
}

void do_worth(CHAR_DATA *ch, const char *argument)
{
	act_puts3("You have $j gold, $J silver",
		  ch, (const void *) ch->gold, NULL, (const void *) ch->silver,
		  TO_CHAR | ACT_NOLF, POS_DEAD);
	if (!IS_NPC(ch)) {
		act_puts(", and $j experience",
			 ch, (const void *) PC(ch)->exp, NULL,
			 TO_CHAR | ACT_NOLF, POS_DEAD);
		if (ch->level < LEVEL_HERO) {
			act_puts(" ($j exp to level)",
				 ch, (const void *) exp_to_level(ch), NULL,
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

void do_time(CHAR_DATA *ch, const char *argument)
{
	extern char str_boot_time[];
	int day = time_info.day + 1;
	int hour;

	hour = time_info.hour % 12;
	if (!hour)
		hour = 12;
	act_puts("It is $j o'clock $T, ",
		 ch, (const void *) hour, time_info.hour >= 12 ? "pm" : "am",
		 TO_CHAR | ACT_NOTRANS | ACT_NOLF, POS_DEAD);
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
		 ch, str_boot_time, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
	act_puts("The system time is $t.",
		 ch, strtime(time(NULL)), NULL,
		 TO_CHAR | ACT_NOTRANS, POS_DEAD);
	act_puts("Reboot in $j $qj{minutes}.",
		 ch, (const void *) reboot_counter, NULL, TO_CHAR, POS_DEAD);
}

void do_date(CHAR_DATA *ch, const char *argument)
{
	act_puts("$t", ch, strtime(time(NULL)), NULL,
		 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
}

void do_weather(CHAR_DATA *ch, const char *argument)
{
	static char * const sky_look[4] = {
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

void do_help(CHAR_DATA *ch, const char *argument)
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

void do_who(CHAR_DATA *ch, const char *argument)
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

		if (is_affected(wch, "vampire")
		&&  !IS_IMMORTAL(ch) && ch != wch)
			continue;

		if (wch->level < iLevelLower || wch->level > iLevelUpper
		||  (IS_SET(flags, WHO_F_IMM) && wch->level < LEVEL_IMMORTAL)
		||  (IS_SET(flags, WHO_F_PK) && (IS_IMMORTAL(wch) ||
						 !in_PK(ch, wch)))
		||  (IS_SET(flags, WHO_F_CLAN) && IS_NULLSTR(wch->clan))
		||  (ralign && ((RALIGN(wch) & ralign) == 0))
		||  (rethos && ((wch->ethos & rethos) == 0)))
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
void do_whois(CHAR_DATA *ch, const char *argument)
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

		if (d->connected != CON_PLAYING || !can_see(ch,d->character))
				continue;

		if (d->connected != CON_PLAYING
		||  (is_affected(d->character, "vampire") &&
		     !IS_IMMORTAL(ch) && (ch != d->character)))
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

void do_inventory(CHAR_DATA *ch, const char *argument)
{
	act_char("You are carrying:", ch);
	show_list_to_char(ch->carrying, ch, TRUE, TRUE);
}

void do_equipment(CHAR_DATA *ch, const char *argument)
{
	OBJ_DATA *obj;
	int i;
	bool found;

	act_char("You are using:", ch);
	found = FALSE;
	for (i = 0; show_order[i] >= 0; i++) {
		if ((obj = get_eq_char(ch, show_order[i])) == NULL)
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

void do_compare(CHAR_DATA *ch, const char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	OBJ_DATA *obj1;
	OBJ_DATA *obj2;
	int value1;
	int value2;
	char *cmsg;

	argument = one_argument(argument, arg1, sizeof(arg1));
	argument = one_argument(argument, arg2, sizeof(arg2));
	if (arg1[0] == '\0') {
		act_char("Compare what to what?", ch);
		return;
	}

	if ((obj1 = get_obj_carry(ch, arg1)) == NULL) {
		act_char("You do not have that item.", ch);
		return;
	}

	if (arg2[0] == '\0') {
		for (obj2 = ch->carrying;
		     obj2 != NULL; obj2 = obj2->next_content)
			if (obj2->wear_loc != WEAR_NONE
			&&  can_see_obj(ch,obj2)
			&&  obj1->item_type == obj2->item_type
			&&  (obj1->wear_flags & obj2->wear_flags & ~ITEM_TAKE))
				break;

		if (obj2 == NULL) {
			act_char("You aren't wearing anything comparable.", ch);
			return;
		}
	}
	else if ((obj2 = get_obj_carry(ch,arg2)) == NULL) {
		act_char("You do not have that item.", ch);
		return;
	}

	cmsg		= NULL;
	value1	= 0;
	value2	= 0;

	if (obj1 == obj2)
		cmsg = "You compare $p to itself.  It looks about the same.";
	else if (obj1->item_type != obj2->item_type)
		cmsg = "You can't compare $p and $P.";
	else {
		switch (obj1->item_type) {
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

void do_credits(CHAR_DATA *ch, const char *argument)
{
	do_help(ch, "'SHADES OF GRAY'");
}

void do_where(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	DESCRIPTOR_DATA *d;
	bool found;
	bool fPKonly = FALSE;

	one_argument(argument, arg, sizeof(arg));

	if (!check_blind(ch))
		return;

	if (room_is_dark(ch)) {
		act_char("It's too dark to see.", ch);
		return;
	}

	if (!str_cmp(arg,"pk"))
		fPKonly = TRUE;

	if (arg[0] == '\0' || fPKonly) {
		act_char("Players near you:", ch);
		found = FALSE;
		for (d = descriptor_list; d; d = d->next) {
			if (d->connected == CON_PLAYING
			&&  (victim = d->character) != NULL
			&&  !IS_NPC(victim)
			&&  (!fPKonly || in_PK(ch, victim))
			&&  victim->in_room != NULL
			&&  victim->in_room->area == ch->in_room->area
			&&  can_see(ch, victim)) {
				CHAR_DATA *doppel;
				found = TRUE;

				if (is_affected(victim, "doppelganger")
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

void do_consider(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	char *cmsg;
	char *align;
	int diff;

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		act_char("Consider killing whom?", ch);
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL) {
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

void do_description(CHAR_DATA *ch, const char *argument)
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
		string_append(ch, mlstr_convert(&ch->description, -1));
		return;
	}

	do_description(ch, str_empty);
}

void do_report(CHAR_DATA *ch, const char *argument)
{
	dofun("say", ch, "I have %d/%d hp %d/%d mana %d/%d mv.",
	      ch->hit, ch->max_hit,
	      ch->mana, ch->max_mana,
	      ch->move, ch->max_move);
}

/*
 * 'Wimpy' originally by Dionysos.
 */
void do_wimpy(CHAR_DATA *ch, const char *argument)
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

void do_password(CHAR_DATA *ch, const char *argument)
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

void do_scan(CHAR_DATA *ch, const char *argument)
{
	char dir[MAX_INPUT_LENGTH];
	ROOM_INDEX_DATA *in_room;
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *exit;	/* pExit */
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

		if ((exit = in_room->exit[door]) == NULL
		||  (to_room = exit->to_room.r) == NULL)
			return;

		if (IS_SET(exit->exit_info, EX_CLOSED)
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

void do_request(CHAR_DATA *ch, const char *argument)
{
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA  *obj;
	AFFECT_DATA af;
	int carry_w, carry_n;

	if (is_affected(ch, "reserved")) {
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

	if ((victim = get_char_room(ch, arg2)) == NULL) {
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

	if (((obj = get_obj_carry(victim , arg1)) == NULL
	&&  (obj = get_obj_wear(victim, arg1)) == NULL)
	||  IS_OBJ_STAT(obj, ITEM_INVENTORY)) {
		do_say(victim, "Sorry, I don't have that.");
		return;
	}

	if (!IS_GOOD(victim)) {
		do_say(victim, "I'm not about to give you anything!");
		do_murder(victim, ch->name);
		return;
	}

	if (obj->wear_loc != WEAR_NONE)
		unequip_char(victim, obj);

	if (!can_drop_obj(ch, obj)) {
		do_say(victim, "Sorry, I can't let go of it. It's cursed.");
		return;
	}

	if ((carry_n = can_carry_n(ch)) >= 0
	&&  ch->carry_number + get_obj_number(obj) > carry_n) {
		act_char("Your hands are full.", ch);
		return;
	}

	if ((carry_w = can_carry_w(ch)) >= 0
	&&  get_carry_weight(ch) + get_obj_weight(obj) > carry_w) {
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

	obj_from_char(obj);
	obj_to_char(obj, ch);
	act("$n requests $p from $N.", ch, obj, victim, TO_NOTVICT);
	act("You request $p from $N.",	 ch, obj, victim, TO_CHAR);
	act("$n requests $p from you.", ch, obj, victim, TO_VICT);

	oprog_call(OPROG_GIVE, obj, ch, victim);

	ch->move -= (50 + ch->level);
	ch->move = UMAX(ch->move, 0);
	ch->hit -= 3 * (ch->level / 2);
	ch->hit = UMAX(ch->hit, 0);

	act("You feel grateful for the trust of $N.", ch, NULL, victim,
	    TO_CHAR);
	act_char("and for the goodness you have seen in the world.", ch);

	af.type = "reserved";
	af.where = TO_AFFECTS;
	af.level = ch->level;
	af.duration = ch->level / 10;
	INT(af.location) = APPLY_NONE;
	af.modifier = 0;
	af.bitvector = 0;
	af.owner = NULL;
	affect_to_char(ch, &af);
}

void do_hometown(CHAR_DATA *ch, const char *argument)
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

void do_detect_hidden(CHAR_DATA *ch, const char *argument)
{
	AFFECT_DATA	af;
	int		chance;

	if ((chance = get_skill(ch, "detect hide")) == 0) {
		act_char("Huh?", ch);
		return;
	}

	if (HAS_DETECT(ch, ID_HIDDEN)) {
		act_char("You are already as alert as you can be.", ch);
		return;
	}

	if (number_percent() > chance) {
		act_char("You peer intently at the shadows but they are unrevealing.", ch);
		check_improve(ch, "detect hide", FALSE, 1);
		return;
	}

	af.where     = TO_DETECTS;
	af.type      = "detect hide";
	af.level     = LEVEL(ch);
	af.duration  = LEVEL(ch);
	INT(af.location) = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = ID_HIDDEN;
	af.owner     = NULL;
	affect_to_char(ch, &af);
	act_char("Your awareness improves.", ch);
	check_improve(ch, "detect hide", TRUE, 1);
}

void do_awareness(CHAR_DATA *ch, const char *argument)
{
	AFFECT_DATA	af;
	int		chance;

	if ((chance = get_skill(ch, "awareness")) == 0) {
		act_char("Huh?", ch);
		return;
	}

	if (is_affected(ch, "awareness")) {
		act_char("You are already as alert as you can be.", ch);
		return;
	}

	if (number_percent() > chance) {
		act_char("You peer intently at the shadows but they are unrevealing.", ch);
		check_improve(ch, "awareness", FALSE, 1);
		return;
	}

	af.where     = TO_INVIS;
	af.type      = "awareness";
	af.level     = LEVEL(ch);
	af.duration  = LEVEL(ch);
	INT(af.location) = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = ID_BLEND | ID_CAMOUFLAGE;
	af.owner     = NULL;
	affect_to_char(ch, &af);

	affect_to_char(ch, &af);

	act_char("Your awareness improves.", ch);
	check_improve(ch, "awareness", TRUE, 1);
}

#define MOB_VNUM_BEAR			12

void do_bear_call(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *	gch;
	CHAR_DATA *	bear;
	CHAR_DATA *	bear2;
	AFFECT_DATA	af;
	int		i;
	int		chance;
	int		mana;

	if ((chance = get_skill(ch, "bear call")) == 0) {
		act_char("Huh?", ch);
		return;
	}

	act_char("You call for bears help you.", ch);
	act("$n shouts a bear call.",ch,NULL,NULL,TO_ROOM);

	if (is_affected(ch, "bear call")) {
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

	if (number_percent() > chance) {
		act_char("No bears listen you.", ch);
		check_improve(ch, "bear call", FALSE, 1);
		return;
	}

	check_improve(ch, "bear call", TRUE, 1);
	bear = create_mob(get_mob_index(MOB_VNUM_BEAR), 0);

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

	af.where	= TO_AFFECTS;
	af.type 	= "bear call";
	af.level	= ch->level;
	af.duration	= skill_beats("bear call");
	af.bitvector	= 0;
	af.modifier	= 0;
	INT(af.location)= APPLY_NONE;
	af.owner	= NULL;
	affect_to_char(ch, &af);

	char_to_room(bear, ch->in_room);
	char_to_room(bear2, ch->in_room);
}

void do_identify(CHAR_DATA *ch, const char *argument)
{
	OBJ_DATA *obj;
	CHAR_DATA *rch;

	if ((obj = get_obj_carry(ch, argument)) == NULL) {
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
	spellfun_call("identify", NULL, 0, ch, obj);
}

static void format_stat(char *buf, size_t len, CHAR_DATA *ch, int stat)
{
	if (ch->level < 20 && !IS_NPC(ch))
		strnzcpy(buf, len, get_stat_alias(ch, stat));
	else {
		snprintf(buf, len, "%2d (%2d)",		// notrans
			 ch->perm_stat[stat],
			 get_curr_stat(ch, stat));
	}
}

void do_score(CHAR_DATA *ch, const char *argument)
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

	strnzcpy(title, sizeof(title),
		 IS_NPC(ch) ? " Believer of Chronos." : PC(ch)->title);
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
		"mobile" : flag_string(ethos_table, ch->ethos),	// notrans
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
		ch->carry_number, can_carry_n(ch),
		GET_AC(ch,AC_EXOTIC));

	buf_printf(output, BUF_END,
"     {G| {RWeight Carried: {x%9d/%-9d {RArmor vs bash   : {x%5d     {G|{x\n",
	get_carry_weight(ch), can_carry_w(ch), GET_AC(ch,AC_BASH));

	buf_printf(output, BUF_END,
"     {G| {RGold          : {Y%9d           {RArmor vs pierce : {x%5d     {G|{x\n",
		 ch->gold, GET_AC(ch,AC_PIERCE));

	buf_printf(output, BUF_END,
"     {G| {RSilver        : {W%9d           {RArmor vs slash  : {x%5d     {G|{x\n",
		 ch->silver, GET_AC(ch,AC_SLASH));

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

void do_oscore(CHAR_DATA *ch, const char *argument)
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
		ch->carry_number, can_carry_n(ch),
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
	}
	else {
		for (i = 0; i < 4; i++) {
			static char* ac_name[4] = {
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
					   "{cdefenseless against{x %s.\n",
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

	switch (ch->ethos) {
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
		buf_append(output, "  You have no ethos");
		if (!IS_NPC(ch))
			buf_append(output, ", report it to the gods!\n");
		else
			buf_append(output, ".\n");
	}

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

void do_affects(CHAR_DATA *ch, const char *argument)
{
	BUFFER *output;

	output = buf_new(GET_LANG(ch));
	show_affects(ch, ch, output);
	page_to_char(buf_string(output), ch);
	buf_free(output);
}

void do_raffects(CHAR_DATA *ch, const char *argument)
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

static const char * get_resist_alias(int resist)
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

void do_resistances(CHAR_DATA *ch, const char *argument)
{
	int i;

	bool found = FALSE;

	for (i = 0; i < MAX_RESIST; i++) {
		int res;

		if (!(res = get_resist(ch, i)) || i == DAM_CHARM)
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
				  TO_CHAR | ACT_NOTRANS, POS_DEAD);
		}
	}

	if (!found) {
		act("You don't have any resistances and vulnerabilities.",
			ch, NULL, NULL, TO_CHAR);
	}
}

#define MOB_VNUM_LION			19

void do_lion_call(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *	gch;
	CHAR_DATA *	lion;
	CHAR_DATA *	lion2;
	AFFECT_DATA	af;
	int		i;
	int		chance;
	int		mana;

	if ((chance = get_skill(ch, "lion call")) == 0) {
		act_char("Huh?", ch);
		return;
	}

	act_char("You call for lions help you.", ch);
	act("$n shouts a lion call.",ch,NULL,NULL,TO_ROOM);

	if (is_affected(ch, "lion call")) {
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

	if (number_percent() > chance) {
		check_improve(ch, "lion call", FALSE, 1);
		act_char("No lions hear you.", ch);
		return;
	}

	check_improve(ch, "lion call", TRUE, 1);
	lion = create_mob(get_mob_index(MOB_VNUM_LION), 0);

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

	af.where	= TO_AFFECTS;
	af.type 	= "lion call";
	af.level	= ch->level;
	af.duration	= skill_beats("lion call");
	af.bitvector	= 0;
	af.modifier	= 0;
	INT(af.location)= APPLY_NONE;
	af.owner	= NULL;
	affect_to_char(ch, &af);

	char_to_room(lion, ch->in_room);
	char_to_room(lion2, ch->in_room);
}

/* new practice */
void do_practice(CHAR_DATA *ch, const char *argument)
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
		int i;

		output = buf_new(GET_LANG(ch));

		for (i = 0; i < pc->learned.nused; i++) {
			spec_skill_t spec_sk;
			pc_sk = VARR_GET(&pc->learned, i);

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
	pc_sk = (pc_skill_t*) vstr_search(&pc->learned, arg);
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
		if (!IS_NPC(mob) || !MOB_IS(mob, MOB_PRACTICE))
			continue;

		found = TRUE;

		if (IS_SET(sk->skill_flags, SKILL_CLAN)) {
			if (!IS_CLAN(ch->clan, mob->clan))
				break;
			continue;
		}

		if ((mob->pMobIndex->practicer == 0 &&
		    (sk->group == GROUP_NONE ||
		     IS_SET(sk->group,	GROUP_CREATION | GROUP_HARMFUL |
					GROUP_PROTECTIVE | GROUP_DIVINATION |
					GROUP_WEATHER)))
		||  IS_SET(mob->pMobIndex->practicer, sk->group))
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

void do_learn(CHAR_DATA *ch, const char *argument)
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
	pc_sk = (pc_skill_t*) vstr_search(&pc->learned, arg);
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
		
	if ((practicer = get_char_room(ch,arg)) == NULL) {
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

void do_teach(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch) || ch->level != LEVEL_HERO) {
		act_char("You must be a hero.", ch);
		return;
	}
	SET_BIT(PC(ch)->plr_flags, PLR_PRACTICER);
	act_char("Now, you can teach youngsters your 100% skills.", ch);
}

/* used to converter of prac and train */
void do_gain(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *tr;
	PC_DATA *pc;

	if (IS_NPC(ch))
		return;

	/* find a trainer */
	for (tr = ch->in_room->people; tr; tr = tr->next_in_room) {
		if (IS_NPC(tr)
		&&  MOB_IS(tr, MOB_PRACTICE | MOB_TRAIN | MOB_GAIN))
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
			do_tell_raw(tr, ch, "You are not ready yet.");
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
			do_tell_raw(tr, ch, "You are not ready yet.");
			return;
		}

		act("$N helps you apply your practice to training.",
		    ch, NULL, tr, TO_CHAR);
		pc->practice -= 10;
		pc->train +=1 ;
		return;
	}

	do_tell_raw(tr, ch, "I do not understand...");
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
void do_prayers(CHAR_DATA *ch, const char *argument)
{
	if (ch->shapeform &&
	IS_SET(ch->shapeform->index->flags, FORM_NOCAST)) {
		act("You are unable to pray in this form.",
			ch, NULL, NULL, TO_CHAR);
		return;
	}
	list_spells(ST_PRAYER, ch, argument);
}

void do_spells(CHAR_DATA *ch, const char *argument) 
{
	if (ch->shapeform &&
	IS_SET(ch->shapeform->index->flags, FORM_NOCAST)) {
		act("You cannot cast any spells in this form.",
			ch, NULL, NULL, TO_CHAR);
		return;
	}

	list_spells(ST_SPELL, ch, argument);
}

static void list_spells(flag_t type, CHAR_DATA *ch, const char *argument)
{
	BUFFER *list[LEVEL_IMMORTAL+1];
	int lev;
	int i;
	bool found = FALSE;
	char buf[MAX_STRING_LENGTH];
	BUFFER *output;

	if (IS_NPC(ch))
		return;
	
	/* initialize data */
	for (lev = 0; lev <= LEVEL_IMMORTAL; lev++)
		list[lev] = NULL;
	
	for (i = 0; i < PC(ch)->learned.nused; i++) {
		pc_skill_t *pc_sk = VARR_GET(&PC(ch)->learned, i);
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
			 "%-19s [%-11s] %4d mana",
			 pc_sk->sn, knowledge, skill_mana(ch, pc_sk->sn));
			
		if (list[lev] == NULL) {
			list[lev] = buf_new(0);
			buf_printf(list[lev], BUF_END,
				   "\nLevel %2d: %s", lev, buf);
		} else
			buf_printf(list[lev], BUF_END,
				   "\n          %s", buf);	// notrans
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
	
	output = buf_new(0);
	for (lev = 0; lev <= UMIN(ch->level, LEVEL_IMMORTAL); lev++)
		if (list[lev] != NULL) {
			buf_append(output, buf_string(list[lev]));
			buf_free(list[lev]);
		}
	buf_append(output, "\n");
	page_to_char(buf_string(output), ch);
	buf_free(output);
}

void do_skills(CHAR_DATA *ch, const char *argument)
{
	BUFFER *skill_list[LEVEL_IMMORTAL+1];
	char skill_columns[LEVEL_IMMORTAL+1];
	int lev;
	int i;
	bool found = FALSE;
	char buf[MAX_STRING_LENGTH];
	BUFFER *output;
	
	if (IS_NPC(ch))
		return;
	
	/* initialize data */
	for (lev = 0; lev <= LEVEL_IMMORTAL; lev++) {
		skill_columns[lev] = 0;
		skill_list[lev] = NULL;
	}
	
	for (i = 0; i < PC(ch)->learned.nused; i++) {
		pc_skill_t *pc_sk = VARR_GET(&PC(ch)->learned, i);
		skill_t *sk;
		spec_skill_t spec_sk;
		const char *knowledge;

		if (pc_sk->percent == 0
		||  (sk = skill_lookup(pc_sk->sn)) == NULL
		||  sk->skill_type != ST_SKILL)
			continue;

		knowledge = skill_knowledge_alias(ch, pc_sk, &spec_sk);
		if (knowledge == NULL)
			continue;

		found = TRUE;
		lev = spec_sk.level;
		
		snprintf(buf, sizeof(buf), "%-19s %-11s  ",	// notrans
			pc_sk->sn, knowledge);

		if (skill_list[lev] == NULL) {
			skill_list[lev] = buf_new(0);
			buf_printf(skill_list[lev], BUF_END,
				   "\nLevel %2d: %s", lev, buf);
		} else {
			if (++skill_columns[lev] % 2 == 0) {
				buf_append(skill_list[lev],
					   "\n          ");	// notrans
			}
			buf_append(skill_list[lev], buf);
		}
	}
	
	/* return results */
	
	if (!found) {
		act_char("You know no skills.", ch);
		return;
	}
	
	output = buf_new(0);
	for (lev = 0; lev <= UMIN(ch->level, LEVEL_IMMORTAL); lev++)
		if (skill_list[lev] != NULL) {
			buf_append(output, buf_string(skill_list[lev]));
			buf_free(skill_list[lev]);
		}
	buf_append(output, "\n");
	page_to_char(buf_string(output), ch);
	buf_free(output);
}

static void *
glist_cb(void *p, va_list ap)
{
	skill_t *sk = (skill_t*) p;

	CHAR_DATA *ch = va_arg(ap, CHAR_DATA *);
	flag_t group = va_arg(ap, flag_t);
	int *pcol = va_arg(ap, int *);

	if (group == sk->group) {
		const char *sn = gmlstr_mval(&sk->sk_name);
		act_puts("$t$f-18{$T}", ch,		           // notrans
			 pc_skill_lookup(ch, sn) ?  "*" : " ", sn, // notrans
			 TO_CHAR | ACT_NOTRANS | ACT_NOLF, POS_DEAD);
		if (*pcol)
			send_to_char("\n", ch);
		*pcol = 1 - *pcol;
	}

	return NULL;
}

void do_glist(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	flag_t group = GROUP_NONE;
	int col = 0;

	one_argument(argument, arg, sizeof(arg));
	
	if (arg[0] == '\0') {
		act_char("Syntax: glist group", ch);
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
	hash_foreach(&skills, glist_cb, ch, group, &col);
	if (col)
		send_to_char("\n", ch);
}

void do_slook(CHAR_DATA *ch, const char *argument)
{
	pc_skill_t *pc_sk = NULL;
	skill_t *sk;
	char arg[MAX_INPUT_LENGTH];

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		act_char("Syntax : slook <skill | spell>", ch);
		return;
	}

	if (!IS_NPC(ch))
		pc_sk = (pc_skill_t*) vstr_search(&PC(ch)->learned, arg);

	if (pc_sk == NULL)
		sk = skill_search(arg);
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

void do_camp(CHAR_DATA *ch, const char *argument)
{
	AFFECT_DATA af;
	int chance;
	int mana;

	if ((chance = get_skill(ch, "camp")) == 0) {
		act_char("Huh?", ch);
		return;
	}

	if (is_affected(ch, "camp")) {
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

	if (number_percent() > chance) {
		act_char("You failed to make your camp.", ch);
		check_improve(ch, "camp", FALSE, 4);
		return;
	}

	check_improve(ch, "camp", TRUE, 4);
	WAIT_STATE(ch, skill_beats("camp"));

	act_char("You succeeded to make your camp.", ch);
	act("$n succeeded to make $s camp.", ch, NULL, NULL, TO_ROOM);

	af.where	= TO_AFFECTS;
	af.type 	= "camp";
	af.level	= ch->level;
	af.duration	= 12;
	af.bitvector	= 0;
	af.modifier	= 0;
	INT(af.location)= APPLY_NONE;
	af.owner	= NULL;
	affect_to_char(ch, &af);

	af.where	= TO_ROOM_AFFECTS;
	af.type		= "camp";
	af.level	= ch->level;
	af.duration	= ch->level / 20;
	af.bitvector	= 0;
	af.modifier	= 2 * LEVEL(ch);
	INT(af.location)= APPLY_ROOM_HEAL;
	af.owner	= ch;
	affect_to_room(ch->in_room, &af);

	af.modifier	= LEVEL(ch);
	INT(af.location)= APPLY_ROOM_MANA;
	affect_to_room(ch->in_room, &af);
}

void do_demand(CHAR_DATA *ch, const char *argument)
{
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA  *obj;
	int chance;
	int carry_w, carry_n;

	argument = one_argument(argument, arg1, sizeof(arg1));
	argument = one_argument(argument, arg2, sizeof(arg2));

	if (IS_NPC(ch))
		return;


	if (arg1[0] == '\0' || arg2[0] == '\0') {
		act_char("Demand what from whom?", ch);
		return;
	}

	if ((victim = get_char_room(ch, arg2)) == NULL) {
		act_char("They aren't here.", ch);
		return;
	}

	if (!IS_NPC(victim)) {
		act_char("Why don't you just want that directly from the player?", ch);
		return;
	}

	WAIT_STATE(ch, get_pulse("violence"));

	chance = IS_EVIL(victim) ? 10 : IS_GOOD(victim) ? -5 : 0;
	chance += (get_curr_stat(ch,STAT_CHA) - 15) * 10;
	chance += LEVEL(ch) - LEVEL(victim);

	chance = (get_skill(ch, "demand")) * chance / 100;

	if (number_percent() > chance) {
		do_say(victim, "I'm not about to give you anything!");
		check_improve(ch, "demand", FALSE, 1);
		do_murder(victim, ch->name);
		return;
	}

	check_improve(ch, "demand", TRUE, 1);

	if (((obj = get_obj_carry(victim , arg1)) == NULL
	&&   (obj = get_obj_wear(victim, arg1)) == NULL)
	||  IS_OBJ_STAT(obj, ITEM_INVENTORY)) {
		do_say(victim, "Sorry, I don't have that.");
		return;
	}

	if (OBJ_IS(obj, OBJ_QUIT_DROP)) {
		do_say(victim, "Forgive me, my master, I can't give it to anyone.");
		return;
	}

	if (obj->wear_loc != WEAR_NONE)
		unequip_char(victim, obj);

	if (!can_drop_obj(ch, obj)) {
		do_say(victim, "It's cursed so, I can't let go of it. "
			       "Forgive me, my master.");
		return;
	}

	if ((carry_n = can_carry_n(ch)) >= 0
	&&  ch->carry_number + get_obj_number(obj) > carry_n) {
		act_char("Your hands are full.", ch);
		return;
	}

	if ((carry_w = can_carry_w(ch)) >= 0
	&&  get_carry_weight(ch) + get_obj_weight(obj) > carry_w) {
		act_char("You can't carry that much weight.", ch);
		return;
	}

	if (!can_see_obj(ch, obj)) {
		act("You don't see that.", ch, NULL, victim, TO_CHAR);
		return;
	}

	obj_from_char(obj);
	obj_to_char(obj, ch);
	act("$n demands $p from $N.", ch, obj, victim, TO_NOTVICT);
	act("You demand $p from $N.",	ch, obj, victim, TO_CHAR  );
	act("$n demands $p from you.", ch, obj, victim, TO_VICT  );

	oprog_call(OPROG_GIVE, obj, ch, victim);
	act_char("Your power makes all around the world shivering.", ch);
}

void do_control(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int chance;
	race_t *r;

	argument = one_argument(argument, arg, sizeof(arg));

	if ((chance = get_skill(ch, "control animal")) == 0) {
		act_char("Huh?", ch);
		return;
	}

	if (arg[0] == '\0') {
		act_char("Charm what?", ch);
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL) {
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

	chance += (get_curr_stat(ch,STAT_CHA) - 20) * 5;
	chance += (ch->level - victim->level) * 3;
	chance +=
	(get_curr_stat(ch,STAT_INT) - get_curr_stat(victim,STAT_INT)) * 5;

	if (IS_AFFECTED(victim, AFF_CHARM)
	||  IS_AFFECTED(ch, AFF_CHARM)
	||  number_percent() > chance
	||  ch->level < (victim->level + 2)
	||  (get_resist(victim, DAM_CHARM) == 100)
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

void do_make_arrow(CHAR_DATA *ch, const char *argument)
{
	OBJ_DATA *arrow;
	AFFECT_DATA af;
	OBJ_INDEX_DATA *pObjIndex;
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
	}
	else if (!str_prefix(arg, "red")) {
		color = "red arrow";
		vnum = OBJ_VNUM_RED_ARROW;
	}
	else if (!str_prefix(arg, "white")) {
		color = "white arrow";
		vnum = OBJ_VNUM_WHITE_ARROW;
	}
	else if (!str_prefix(arg, "blue")) {
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
	pObjIndex = get_obj_index(vnum);
	for (count = 0; count < LEVEL(ch) / 5; count++) {
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

		arrow = create_obj(pObjIndex, 0);
		arrow->level = ch->level;
		INT(arrow->value[1]) = 4 + LEVEL(ch) / 10;
		INT(arrow->value[2]) = 4 + LEVEL(ch) / 10;

		af.where	 = TO_OBJECT;
		af.type		 = "make arrow";
		af.level	 = ch->level;
		af.duration	 = -1;
		INT(af.location) = APPLY_HITROLL;
		af.modifier	 = LEVEL(ch) / 10;
		af.bitvector 	 = 0;
		af.owner	 = NULL;
		affect_to_obj(arrow, &af);

		INT(af.location) = APPLY_DAMROLL;
		affect_to_obj(arrow, &af);

		obj_to_char(arrow, ch);
		act_puts("You successfully make $p.",
			 ch, arrow, NULL, TO_CHAR, POS_DEAD);
	}
}

#define OBJ_VNUM_RANGER_BOW		7

void do_make_bow(CHAR_DATA *ch, const char *argument)
{
	OBJ_DATA *	bow;
	AFFECT_DATA	af;
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

	bow = create_obj(get_obj_index(OBJ_VNUM_RANGER_BOW), 0);
	bow->level = ch->level;
	INT(bow->value[1]) = 4 + ch->level / 15;
	INT(bow->value[2]) = 4 + ch->level / 15;

	af.where	= TO_OBJECT;
	af.type		= "make bow";
	af.level	= ch->level;
	af.duration	= -1;
	INT(af.location)= APPLY_HITROLL;
	af.modifier	= LEVEL(ch) / 10;
	af.bitvector 	= 0;
	af.owner	= NULL;
	affect_to_obj(bow, &af);

	INT(af.location)= APPLY_DAMROLL;
	affect_to_obj(bow, &af);

	obj_to_char(bow, ch);
	act_puts("You successfully make $p.", ch, bow, NULL, TO_CHAR, POS_DEAD);
}

void do_make(CHAR_DATA *ch, const char *argument)
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

/*Added by Osya*/
void do_homepoint(CHAR_DATA *ch, const char *argument)
{
        AFFECT_DATA af;
        int chance;
        char arg[MAX_INPUT_LENGTH];

        if ((chance = get_skill(ch, "homepoint")) == 0) {
                act_char("Huh?", ch);
                return;
        }

        if (is_affected(ch, "homepoint")) {
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

        ch->mana = 0 ;

        if (number_percent() > chance) {
                act_char("You failed to make your homepoint.", ch);
                check_improve(ch, "homepoint", FALSE, 4);
                return;
        }

        check_improve(ch, "homepoint", TRUE, 4);
        WAIT_STATE(ch, skill_beats("homepoint"));

        act_char("You succeeded to make your homepoint.", ch);
        act("$n succeeded to make $s homepoint. ", ch, NULL, NULL, TO_ROOM);

        af.where        = TO_AFFECTS;
        af.type         = "homepoint";
        af.level        = ch->level;
        af.duration     = 100;
        af.bitvector    = 0;
        af.modifier     = 0;
        INT(af.location)= APPLY_NONE;
	af.owner	= NULL;
        affect_to_char(ch, &af);

        argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] && !str_prefix(arg, "motherland"))
		PC(ch)->homepoint = NULL;
        else 
		PC(ch)->homepoint = ch->in_room; 
}

/*
 * static functions
 */
static char *format_obj_to_char(OBJ_DATA *obj, CHAR_DATA *ch, bool fShort)
{
	static char buf[MAX_STRING_LENGTH];

	buf[0] = '\0';
	if ((fShort && mlstr_null(&obj->short_descr))
	||  mlstr_null(&obj->description))
		return str_empty;

	if (IS_SET(ch->comm, COMM_LONG)) {
		if (IS_OBJ_STAT(obj, ITEM_INVIS))
			strnzcat(buf, sizeof(buf),
				 GETMSG("({yInvis{x) ", GET_LANG(ch)));
		if (IS_OBJ_STAT(obj, ITEM_DARK))
			strnzcat(buf, sizeof(buf),
				 GETMSG("({DDark{x) ", GET_LANG(ch)));
		if (HAS_DETECT(ch, ID_EVIL)
		&&  IS_OBJ_STAT(obj, ITEM_EVIL))
			strnzcat(buf, sizeof(buf),
				 GETMSG("({RRed Aura{x) ", GET_LANG(ch)));
		if (HAS_DETECT(ch, ID_GOOD)
		&&  IS_OBJ_STAT(obj, ITEM_BLESS))
			strnzcat(buf, sizeof(buf),
				 GETMSG("({BBlue Aura{x) ", GET_LANG(ch)));
		if (HAS_DETECT(ch, ID_MAGIC)
		&&  IS_OBJ_STAT(obj, ITEM_MAGIC))
			strnzcat(buf, sizeof(buf),
				 GETMSG("({MMagical{x) ", GET_LANG(ch)));
		if (IS_OBJ_STAT(obj, ITEM_GLOW))
			strnzcat(buf, sizeof(buf),
				 GETMSG("({WGlowing{x) ", GET_LANG(ch)));
		if (IS_OBJ_STAT(obj, ITEM_HUM))
			strnzcat(buf, sizeof(buf),
				 GETMSG("({YHumming{x) ", GET_LANG(ch)));
	} else {
		static char FLAGS[] = "{x[{y.{D.{R.{B.{M.{W.{Y.{x] "; // notrans
		strnzcpy(buf, sizeof(buf), FLAGS);
		if (IS_OBJ_STAT(obj, ITEM_INVIS))
			buf[5] = 'I';
		if (IS_OBJ_STAT(obj, ITEM_DARK))
			buf[8] = 'D';
		if (HAS_DETECT(ch, ID_EVIL)
		&&  IS_OBJ_STAT(obj, ITEM_EVIL))
			buf[11] = 'E';
		if (HAS_DETECT(ch, ID_GOOD)
		&&  IS_OBJ_STAT(obj,ITEM_BLESS))
			buf[14] = 'B';
		if (HAS_DETECT(ch, ID_MAGIC)
		&&  IS_OBJ_STAT(obj, ITEM_MAGIC))
			buf[17] = 'M';
		if (IS_OBJ_STAT(obj, ITEM_GLOW))
			buf[20] = 'G';
		if (IS_OBJ_STAT(obj, ITEM_HUM))
			buf[23] = 'H';
		if (strcmp(buf, FLAGS) == 0)
			buf[0] = '\0';
	}

	if (fShort) {
		strnzcat(buf, sizeof(buf),
			 format_short(&obj->short_descr,
				      obj->pObjIndex->name, ch));
		if (obj->pObjIndex->vnum > 5 /* not money, gold, etc */
		&&  (obj->condition < COND_EXCELLENT ||
		     !IS_SET(ch->comm, COMM_NOVERBOSE))) {
			char buf2[MAX_STRING_LENGTH];
			snprintf(buf2, sizeof(buf2), " [{g%s{x]",  // notrans
				 GETMSG(get_cond_alias(obj), GET_LANG(ch)));
			strnzcat(buf, sizeof(buf), buf2);
		}
		return buf;
	}

	if (obj->in_room && IS_WATER(obj->in_room)) {
		char* p;

		p = strchr(buf, '\0');
		strnzcat(buf, sizeof(buf),
			 format_short(&obj->short_descr,
				      obj->pObjIndex->name, ch));
		p[0] = UPPER(p[0]);
		switch(number_range(1, 3)) {
		case 1:
			strnzcat(buf, sizeof(buf),
				 " is floating gently on the water.");
			break;
		case 2:
			strnzcat(buf, sizeof(buf),
				 " is making it's way on the water.");
			break;
		case 3:
			strnzcat(buf, sizeof(buf),
				 " is getting wet by the water.");
			break;
		}
	}
	else {
		char tmp[MAX_STRING_LENGTH];
		actopt_t opt;

		opt.to_lang = GET_LANG(ch);
		opt.act_flags = ACT_NOUCASE | ACT_NOLF;

		act_buf(format_long(&obj->description, ch), ch, ch,
			NULL, NULL, NULL, &opt, tmp, sizeof(tmp));
		strnzcat(buf, sizeof(buf), tmp);
	}
	return buf;
}

/*
 * Show a list to a character.
 * Can coalesce duplicated items.
 */
static void show_list_to_char(OBJ_DATA *list, CHAR_DATA *ch,
			      bool fShort, bool fShowNothing)
{
	BUFFER *output;
	const char **prgpstrShow;
	int *prgnShow;
	char *pstrShow;
	OBJ_DATA *obj;
	int nShow;
	int iShow;
	int count;
	bool fCombine;

	if (ch->desc == NULL)
		return;

	/*
	 * Alloc space for output lines.
	 */
	output = buf_new(0);

	count = 0;
	for (obj = list; obj != NULL; obj = obj->next_content)
		count++;
	prgpstrShow = malloc(count * sizeof(char *));
	prgnShow    = malloc(count * sizeof(int)  );
	nShow	= 0;

	/*
	 * Format the list of objects.
	 */
	for (obj = list; obj != NULL; obj = obj->next_content) {
		if (obj->wear_loc == WEAR_NONE && can_see_obj(ch, obj)) {
			pstrShow = format_obj_to_char(obj, ch, fShort);

			fCombine = FALSE;

			if (IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE)) {
				/*
				 * Look for duplicates, case sensitive.
				 * Matches tend to be near end so run loop
				 * backwords.
				 */
				for (iShow = nShow - 1; iShow >= 0; iShow--) {
					if (!strcmp(prgpstrShow[iShow],
						    pstrShow)) {
						prgnShow[iShow]++;
						fCombine = TRUE;
						break;
					}
				}
			}

			/*
			 * Couldn't combine, or didn't want to.
			 */
			if (!fCombine) {
				prgpstrShow [nShow] = str_dup(pstrShow);
				prgnShow    [nShow] = 1;
				nShow++;
			}
		}
	}

	/*
	 * Output the formatted list.
	 */
	for (iShow = 0; iShow < nShow; iShow++) {
		if (prgpstrShow[iShow][0] == '\0')
			continue;

		if (IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE)) {
			if (prgnShow[iShow] != 1) {
				buf_printf(output, BUF_END,
				    "(%2d) ",			// notrans
				    prgnShow[iShow]);
			} else
				buf_append(output,"     ");	// notrans
		}

		buf_append(output, prgpstrShow[iShow]);
		buf_append(output, "\n");
		free_string(prgpstrShow[iShow]);
	}

	if (fShowNothing && nShow == 0) {
		if (IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE))
			send_to_char("     ", ch);		// notrans
		act_char("Nothing.", ch);
	}

	page_to_char(buf_string(output),ch);

	/*
	 * Clean up.
	 */
	buf_free(output);
	free(prgpstrShow);
	free(prgnShow);
}

#define FLAG_SET(pos, c, exp) (buf[pos] = (exp) ? (c) : '.')

static void show_char_to_char_0(CHAR_DATA *victim, CHAR_DATA *ch)
{
	const char *msg = str_empty;
	const void *arg = NULL;
	const void *arg3 = NULL;
	BUFFER *output;

	if (is_affected(victim, "doppelganger")
	&&  (IS_NPC(ch) || !IS_SET(PC(ch)->plr_flags, PLR_HOLYLIGHT)))
		victim = victim->doppel;

	if (is_affected(ch, "hallucination") && !IS_NPC(ch))
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
		&& HAS_DETECT(ch, ID_CHARM))
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
			buf_append(output, "({RRed Aura{x) ");
		if (IS_GOOD(victim) && HAS_DETECT(ch, ID_GOOD))
			buf_append(output, "({YGolden Aura{x) ");
		if (IS_AFFECTED(victim, AFF_SANCTUARY))
			buf_append(output, "({WWhite Aura{x) ");
		if (IS_AFFECTED(victim, AFF_BLACK_SHROUD))
			buf_append(output, "({DBlack Aura{x) ");
		if (HAS_INVIS(victim, ID_FADE))
			buf_append(output, "({yFade{x) ");
		if (HAS_INVIS(victim, ID_CAMOUFLAGE))
			buf_append(output, "({gCamf{x) ");
		if (HAS_INVIS(victim, ID_BLEND))
			buf_append(output, "({gBlending{x) ");
		if (IS_SET(ch->comm, COMM_SHOWRACE)) {
			buf_act(output, BUF_END, "({c$t{x) ", 	// notrans
				NULL, victim->race, NULL, ACT_NOLF);	
		}
	} else {
		static char FLAGS[] = "{x[{y.{D.{m.{c.{M.{D.{G.{b.{R.{Y.{W.{y.{g.{g."; // notrans
		char buf[sizeof(FLAGS)];
		bool diff;

		strnzcpy(buf, sizeof(buf), FLAGS);
		FLAG_SET( 5, 'I', HAS_INVIS(victim, ID_INVIS)
		|| HAS_INVIS(victim, ID_IMP_INVIS));
		FLAG_SET( 8, 'H', HAS_INVIS(victim, ID_HIDDEN));
		FLAG_SET(11, 'C', IS_AFFECTED(victim, AFF_CHARM)
				  && HAS_DETECT(ch, ID_CHARM));
		FLAG_SET(14, 'T', IS_AFFECTED(victim, AFF_PASS_DOOR));
		FLAG_SET(17, 'P', IS_AFFECTED(victim, AFF_FAERIE_FIRE));
		FLAG_SET(20, 'U', IS_SET(victim->form, FORM_UNDEAD) &&
				  HAS_DETECT(ch, ID_UNDEAD));
		FLAG_SET(23, 'R', RIDDEN(victim));
		FLAG_SET(29, 'E', IS_EVIL(victim) &&
				  HAS_DETECT(ch, ID_EVIL));
		FLAG_SET(32, 'G', IS_GOOD(victim) &&
				  HAS_DETECT(ch, ID_GOOD));
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

	send_to_char(buf_string(output), ch);

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

static char* wear_loc_names[] =
{
	"<used as light>     $t",
	"<worn on finger>    $t",
	"<worn on finger>    $t",
	"<worn around neck>  $t",
	"<worn on face>      $t",
	"<worn on torso>     $t",
	"<worn on head>      $t",
	"<worn on legs>      $t",
	"<worn on feet>      $t",
	"<worn on hands>     $t",
	"<worn on arms>      $t",
	"<worn as shield>    $t",
	"<worn about body>   $t",
	"<worn about waist>  $t",
	"<worn about wrist>  $t",
	"<worn about wrist>  $t",
	"<wielded>           $t",
	"<held>              $t",
	"<floating nearby>   $t",
	"<scratched tattoo>  $t",
	"<dual wielded>      $t",
	"<clan mark>         $t",
	"<stuck in>          $t",
};

static void show_obj_to_char(CHAR_DATA *ch, OBJ_DATA *obj, flag_t wear_loc)
{
	bool can_see = can_see_obj(ch, obj);
	act(wear_loc_names[wear_loc], ch,
	    can_see ? format_obj_to_char(obj, ch, TRUE) : "something",
	    NULL, TO_CHAR | (can_see ? ACT_NOTRANS : 0));
}

static void show_char_to_char_1(CHAR_DATA *victim, CHAR_DATA *ch)
{
	OBJ_DATA *obj;
	int i;
	int percent;
	bool found;
	char *msg;
	const char *desc;
	CHAR_DATA *doppel = victim;
	CHAR_DATA *mirror = victim;
	char buf[MAX_STRING_LENGTH];

	if (is_affected(victim, "doppelganger")) {
		if (IS_NPC(ch) || !IS_SET(PC(ch)->plr_flags, PLR_HOLYLIGHT)) {
			doppel = victim->doppel;
			if (is_affected(victim, "mirror"))
				mirror = victim->doppel;
		}
	}

	if (can_see(victim, ch)) {
		if (ch == victim)
			act("$n looks at $mself.",
			    ch, NULL, NULL, TO_ROOM);
		else {
			act_puts("$n looks at you.",
				 ch, NULL, victim, TO_VICT, POS_RESTING);
			act("$n looks at $N.",
			    ch, NULL, victim, TO_NOTVICT);
		}
	}

	if (is_affected(ch, "hallucination") && !IS_NPC(ch)) 
		doppel = nth_char(doppel, PC(ch)->random_value);

	if (doppel->shapeform) 
		desc = mlstr_cval(&doppel->shapeform->index->description, ch);
	else if (IS_NPC(doppel))
		desc = mlstr_cval(&doppel->description, ch);
	else
		desc = mlstr_mval(&doppel->description);

	if (!IS_NULLSTR(desc)) {
		if (doppel->shapeform || IS_NPC(doppel)) {
			act_puts(desc, ch, NULL, NULL,
				 TO_CHAR | ACT_NOLF, POS_DEAD);
		} else {
			act_puts("$t{x", ch, desc, NULL,	// notrans
				 TO_CHAR | ACT_NOLF, POS_DEAD);
		}
	} else {
		act_puts("You see nothing special about $m.",
			 victim, NULL, ch, TO_VICT, POS_DEAD);
	}

	if (MOUNTED(victim))
		act_puts("$N is riding $i.",
			 ch, MOUNTED(victim), victim, TO_CHAR, POS_DEAD);
	if (RIDDEN(victim))
		act_puts("$N is being ridden by $i.",
			 ch, RIDDEN(victim), victim, TO_CHAR, POS_DEAD);

	if (victim->max_hit > 0)
		percent = (100 * victim->hit) / victim->max_hit;
	else
		percent = -1;

	if (percent >= 100)
		msg = "{Cis in perfect health{x.";
	else if (percent >= 90)
		msg = "{bhas a few scratches{x.";
	else if (percent >= 75)
		msg = "{Bhas some small but disgusting cuts{x.";
	else if (percent >= 50)
		msg = "{Gis covered with bleeding wounds{x.";
	else if (percent >= 30)
		msg = "{Yis gushing blood{x.";
	else if (percent >= 15)
		msg = "{Mis writhing in agony{x.";
	else if (percent >= 0)
		msg = "{Ris convulsing on the ground{x.";
	else
		msg = "{Ris nearly dead{x.";

	/* vampire ... */
	if (percent < 90 && IS_VAMPIRE(ch))
		gain_condition(ch, COND_BLOODLUST, -1);

	if (!IS_IMMORTAL(doppel)) {
		act_puts("($t) ", ch, doppel->race, NULL,	// notrans
			 TO_CHAR | ACT_NOTRANS | ACT_NOLF, POS_DEAD);
		if (!IS_NPC(doppel)) {
			act_puts("($t) ($T) ", ch,		// notrans
				 doppel->class, mlstr_mval(&doppel->gender),
				 TO_CHAR | ACT_NOTRANS | ACT_NOLF, POS_DEAD);
		}
	}

	strnzcpy(buf, sizeof(buf), PERS(victim, ch));
	buf[0] = UPPER(buf[0]);
	if (IS_IMMORTAL(victim))
		send_to_char("{W", ch);				// notrans
	act_puts("$N", ch, NULL, victim,			// notrans
		 TO_CHAR | ACT_NOLF | ACT_FORMSH, POS_DEAD);
	if (IS_IMMORTAL(victim))
		send_to_char("{x", ch);
	act_puts(" $t", ch, msg, NULL, TO_CHAR, POS_DEAD);	// notrans

	found = FALSE;
	for (i = 0; show_order[i] != -1; i++)
		if ((obj = get_eq_char(mirror, show_order[i]))
		&&  can_see_obj(ch, obj)) {
			if (!found) {
				send_to_char("\n", ch);
				act("$N is using:", ch, NULL, victim, TO_CHAR);
				found = TRUE;
			}

			show_obj_to_char(ch, obj, show_order[i]);
		}

	for (obj = mirror->carrying; obj; obj = obj->next_content)
		if (obj->wear_loc == WEAR_STUCK_IN
		&&  can_see_obj(ch, obj)) {
			if (!found) {
				send_to_char("\n", ch);
				act("$N is using:", ch, NULL, victim, TO_CHAR);
				found = TRUE;
			}

			show_obj_to_char(ch, obj, WEAR_STUCK_IN);
		}

	if (victim != ch
	&&  (!IS_IMMORTAL(victim) || IS_IMMORTAL(ch))
	&&  !IS_NPC(ch)
	&&  number_percent() < get_skill(ch, "peek")) {
		send_to_char("\n", ch);
		act_char("You peek at the inventory:", ch);
		check_improve(ch, "peek", TRUE, 4);
		show_list_to_char(mirror->carrying, ch, TRUE, TRUE);
	}
}

static void show_char_to_char(CHAR_DATA *list, CHAR_DATA *ch)
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
			if (room_is_dark(ch) && HAS_DETECT(rch, ID_INFRARED))
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

/*
 * Contributed by Alander.
 */
void do_commands(CHAR_DATA *ch, const char *argument)
{
	int col;
	int i;
	varr v;
 
	varr_cpy(&v, &commands);
	varr_qsort(&v, cmpstr);

	col = 0;
	for (i = 0; i < v.nused; i++) {
		cmd_t *cmd = VARR_GET(&v, i);

		if (cmd->min_level < LEVEL_HERO
		&&  cmd->min_level <= ch->level 
		&&  !IS_SET(cmd->cmd_flags, CMD_HIDDEN)) {
			act_puts("$f-12{$t}", ch, cmd->name, NULL,   // notrans
				 TO_CHAR | ACT_NOTRANS | ACT_NOLF | ACT_NOUCASE,
				 POS_DEAD);
			if (++col % 6 == 0)
				send_to_char("\n", ch);
		}
	}
 
	if (col % 6 != 0)
		send_to_char("\n", ch);

	varr_destroy(&v);
}

void do_wizhelp(CHAR_DATA *ch, const char *argument)
{
	int i;
	int col;
	varr v;

	if (IS_NPC(ch)) {
		act_char("Huh?", ch);
		return;
	}

	varr_cpy(&v, &commands);
	varr_qsort(&v, cmpstr);

	col = 0;
	for (i = 0; i < v.nused; i++) {
		cmd_t *cmd = VARR_GET(&v, i);

		if (cmd->min_level < LEVEL_IMMORTAL)
			continue;

		if (ch->level < LEVEL_IMP
		&&  !is_name(cmd->name, PC(ch)->granted))
			continue;

		act_puts("$f-12{$t}", ch, cmd->name, NULL,	// notrans
			 TO_CHAR | ACT_NOTRANS | ACT_NOLF | ACT_NOUCASE,
			 POS_DEAD);
		if (++col % 6 == 0)
			send_to_char("\n", ch);
	}
 
	if (col % 6 != 0)
		send_to_char("\n", ch);

	varr_destroy(&v);
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
			flag_string(ethos_table, vch->ethos),
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

void do_clanlist(CHAR_DATA *ch, const char *argument)
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

static void *
item_cb(void *p, va_list ap)
{
	clan_t *clan = (clan_t *) p;

	ROOM_INDEX_DATA *in_room = va_arg(ap, ROOM_INDEX_DATA *);

	if (in_room->vnum == clan->altar_vnum)
		return p;

	return NULL;
}

void do_item(CHAR_DATA* ch, const char* argument)
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
		clan = hash_foreach(&clans, item_cb, in_obj->in_room);
		if (clan) {
			act_puts("It is altar of $t.",
				 ch, clan->name, NULL, TO_CHAR, POS_DEAD);
		}
	} else 
		act_puts("$p is somewhere.",
			 ch, clan->obj_ptr, NULL, TO_CHAR, POS_DEAD);
}

void do_rating(CHAR_DATA *ch, const char *argument)
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

void do_areas(CHAR_DATA *ch, const char *argument)
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

