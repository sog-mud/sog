/*
 * $Id: act_info.c,v 1.271.2.34 2001-06-19 09:22:30 kostik Exp $
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

#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>

#if !defined(WIN32)
#	include <unistd.h>
#endif
#include <ctype.h>

#include "merc.h"
#include "db.h"
#include "quest.h"
#include "obj_prog.h"
#include "fight.h"
#include "socials.h"
#include "rating.h"
#include "string_edit.h"

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
					 flag32_t wear_loc);

static int show_order[] = {
	WEAR_LIGHT,
	WEAR_FINGER_L,
	WEAR_FINGER_R,
	WEAR_NECK_1,
	WEAR_NECK_2,
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
		char_puts("\033[0;0H\033[2J", ch);
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
		char_printf(ch, "You currently display %d lines per page.\n",
			    d->dvdata->pagelen + 2);
		return;
	}

	if (!is_number(arg)) {
		char_puts("You must provide a number.\n",ch);
		return;
	}

	pagelen = atoi(arg);
	if (pagelen < MIN_PAGELEN || pagelen > MAX_PAGELEN) {
		char_printf(ch, "Valid scroll range is %d..%d.\n",
			    MIN_PAGELEN, MAX_PAGELEN);
		return;
	}

	d->dvdata->pagelen = pagelen - 2;
	char_printf(ch, "Scroll set to %d lines.\n", pagelen);
}

/* RT does socials */
void do_socials(CHAR_DATA *ch, const char *argument)
{
	social_t *soc;
	bool found;

	if (argument[0] == '\0') {
		char_puts("\nUse social <name> to view "
			"what that social does.\n\n", ch);
		dofun("alist", ch, "social");
		return;
	}

	if ((soc = social_lookup(argument, str_prefix)) == NULL) {
		char_puts("There is no such social.\n", ch);
		return;
	}

	/* noarg */
	char_puts("Using this social without an argument", ch);
	found = FALSE;

	if (soc->noarg_char) {
		char_puts(", you'll see:\n   ", ch);
		found = TRUE;
	}
	act_puts(soc->noarg_char, ch, NULL, NULL, TO_CHAR, POS_DEAD);

	if (soc->noarg_room) {
		if (found)
			char_puts("O", ch);
		else
			char_puts(", o", ch);
		char_puts("thers will see:\n   ", ch);
		found = TRUE;
	}
	act_puts(soc->noarg_room, ch, NULL, NULL, TO_CHAR, POS_DEAD);

	if (!found)
		char_puts(" is pointless.\n", ch);

	/* self */
	char_puts("\nUsing it with your name as an argument", ch);
	found = FALSE;

	if (soc->self_char) {
		char_puts(", you'll see:\n   ", ch);
		found = TRUE;
	}
	act_puts(soc->self_char, ch, NULL, ch, TO_CHAR, POS_DEAD);

	if (soc->self_room) {
		if (found)
			char_puts("O", ch);
		else
			char_puts(", o", ch);
		char_puts("thers will see:\n   ", ch);
		found = TRUE;
	}
	act_puts(soc->self_room, ch, NULL, ch, TO_CHAR, POS_DEAD);

	if (!found)
		char_puts(" is pointless.\n", ch);

	/* notfound */
	if (soc->notfound_char) {
		char_puts("\nIf your victim is absent, you'll see:\n   ", ch);
		act_puts(soc->notfound_char, ch, NULL, NULL, TO_CHAR, POS_DEAD);
	}

	/* other */
	char_puts("\nUsing it on other character", ch);
	found = FALSE;
	if (soc->found_char) {
		char_puts(", you'll see:\n   ", ch);
		found = TRUE;
	}
	act_puts(soc->found_char, ch, NULL, ch, TO_CHAR, POS_DEAD);

	if (soc->found_vict) {
		if (found)
			char_puts("Y", ch);
		else
			char_puts(", y", ch);
		char_puts("our victim will see:\n   ", ch);
		found = TRUE;
	}
	act_puts(soc->found_vict, ch, NULL, ch, TO_CHAR, POS_DEAD);

	if (soc->found_notvict) {
		if (found)
			char_puts("O", ch);
		else
			char_puts(", o", ch);
		char_puts("thers will see:\n   ", ch);
		found = TRUE;
	}
	act_puts(soc->found_notvict, ch, NULL, ch, TO_CHAR, POS_DEAD);

	if (!found)
		char_puts(" is pointless.\n", ch);
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
#define do_print_sw(ch, swname, sw) \
		char_printf(ch, "%-16s %s\n", swname, sw ? "ON" : "OFF");

void do_autolist(CHAR_DATA *ch, const char *argument)
{
	/* lists most player flags */
	if (IS_NPC(ch))
		return;

	char_puts("action         status\n",ch);
	char_puts("---------------------\n",ch);
	do_print_sw(ch, "autoassist", IS_SET(PC(ch)->plr_flags, PLR_AUTOASSIST));
	do_print_sw(ch, "autoexit", IS_SET(PC(ch)->plr_flags, PLR_AUTOEXIT));
	do_print_sw(ch, "autogold", IS_SET(PC(ch)->plr_flags, PLR_AUTOGOLD));
	do_print_sw(ch, "autolook", IS_SET(PC(ch)->plr_flags, PLR_AUTOLOOK));
	do_print_sw(ch, "autoloot", IS_SET(PC(ch)->plr_flags, PLR_AUTOLOOT));
	do_print_sw(ch, "autosac", IS_SET(PC(ch)->plr_flags, PLR_AUTOSAC));
	do_print_sw(ch, "autosplit", IS_SET(PC(ch)->plr_flags, PLR_AUTOSPLIT));

	if (IS_SET(PC(ch)->plr_flags, PLR_NOSUMMON))
		char_puts("You can only be summoned players within "
			     "your PK range.\n",ch);
	else
		char_puts("You can be summoned by anyone.\n",ch);

	if (IS_SET(PC(ch)->plr_flags, PLR_NOFOLLOW))
		char_puts("You do not welcome followers.\n",ch);
	else
		char_puts("You accept followers.\n",ch);
}

void do_autoassist(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch)) {
		char_puts("Huh?\n", ch);
		return;
	}

	TOGGLE_BIT(PC(ch)->plr_flags, PLR_AUTOASSIST);
	if (IS_SET(PC(ch)->plr_flags, PLR_AUTOASSIST))
		char_puts("You will now assist when needed.\n",ch);
	else
		char_puts("Autoassist removed.\n",ch);
}

void do_autoexit(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch)) {
		char_puts("Huh?\n", ch);
		return;
	}

	TOGGLE_BIT(PC(ch)->plr_flags, PLR_AUTOEXIT);
	if (IS_SET(PC(ch)->plr_flags, PLR_AUTOEXIT))
		char_puts("Exits will now be displayed.\n",ch);
	else 
		char_puts("Exits will no longer be displayed.\n",ch);
}

void do_autogold(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch)) {
		char_puts("Huh?\n", ch);
		return;
	}

	TOGGLE_BIT(PC(ch)->plr_flags, PLR_AUTOGOLD);
	if (IS_SET(PC(ch)->plr_flags, PLR_AUTOGOLD))
		char_puts("Automatic gold looting set.\n",ch);
	else 
		char_puts("Autogold removed.\n",ch);
}

void do_autolook(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch)) {
		char_puts("Huh?\n", ch);
		return;
	}

	TOGGLE_BIT(PC(ch)->plr_flags, PLR_AUTOLOOK);
	if (IS_SET(PC(ch)->plr_flags, PLR_AUTOLOOK))
		char_puts("Automatic corpse examination set.\n", ch);
	else
		char_puts("Autolooking removed.\n", ch);
}

void do_autoloot(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch)) {
		char_puts("Huh?\n", ch);
		return;
	}

	TOGGLE_BIT(PC(ch)->plr_flags, PLR_AUTOLOOT);
	if (IS_SET(PC(ch)->plr_flags, PLR_AUTOLOOT))
		char_puts("Automatic corpse looting set.\n", ch);
	else
		char_puts("Autolooting removed.\n", ch);
}

void do_autosac(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch)) {
		char_puts("Huh?\n", ch);
		return;
	}

	TOGGLE_BIT(PC(ch)->plr_flags, PLR_AUTOSAC);
	if (IS_SET(PC(ch)->plr_flags, PLR_AUTOSAC))
		char_puts("Automatic corpse sacrificing set.\n",ch);
	else
		char_puts("Autosacrificing removed.\n",ch);
}

void do_autosplit(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch)) {
		char_puts("Huh?\n", ch);
		return;
	}

	TOGGLE_BIT(PC(ch)->plr_flags, PLR_AUTOSPLIT);
	if (IS_SET(PC(ch)->plr_flags, PLR_AUTOSPLIT))
		char_puts("Automatic gold splitting set.\n",ch);
	else
		char_puts("Autosplitting removed.\n",ch);
}

void do_prompt(CHAR_DATA *ch, const char *argument)
{
	const char *prompt;
	DESCRIPTOR_DATA *d;

	if ((d = ch->desc) == NULL)
		return;

	if (argument[0] == '\0') {
		bust_a_prompt(d);
		char_puts("\n", ch);
		return;
	}

	if (!str_prefix(argument, "show")) {
		char_printf(ch, "Current prompt is '%s'.\n",
			    d->dvdata->prompt);
		return;
	}

	if (!str_cmp(argument, "all") || !str_cmp(argument, "default"))
		prompt = str_dup(DEFAULT_PROMPT);
	else
		prompt = str_printf("%s ", argument);

	free_string(d->dvdata->prompt);
	d->dvdata->prompt = prompt;
	char_printf(ch, "Prompt set to '%s'.\n", d->dvdata->prompt);
}

void do_nofollow(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch)) {
		char_puts("Huh?\n", ch);
		return;
	}

	TOGGLE_BIT(PC(ch)->plr_flags, PLR_NOFOLLOW);
	if (IS_SET(PC(ch)->plr_flags,PLR_NOFOLLOW)) {
		char_puts("You no longer accept followers.\n", ch);
		die_follower(ch);
	}
	else
		char_puts("You now accept followers.\n", ch);
}

void do_nosummon(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch)) {
		TOGGLE_BIT(ch->imm_flags, IMM_SUMMON);
		if (IS_SET(ch->imm_flags, IMM_SUMMON))
			char_puts("You are now immune to summoning.\n", ch);
		else
			char_puts("You are no longer immune "
				  "to summoning.\n", ch);
	}
	else {
		TOGGLE_BIT(PC(ch)->plr_flags, PLR_NOSUMMON);
		if (IS_SET(PC(ch)->plr_flags,PLR_NOSUMMON))
			char_puts("You may only be summoned by players "
				  "within your PK range.\n", ch);
		else 
			char_puts("You may now be summoned by anyone.\n", ch);
	}
}

static void do_look_in(CHAR_DATA* ch, const char *argument)
{
	OBJ_DATA *obj;

	if ((obj = get_obj_here(ch, argument)) == NULL) {
		char_puts("You don't see that here.\n", ch);
		return;
	}

	switch (obj->pObjIndex->item_type) {
	default:
		char_puts("That is not a container.\n", ch);
		break;

	case ITEM_DRINK_CON:
		if (obj->value[1] == 0) {
			char_puts("It is empty.\n", ch);
			break;
		}

		act_puts3("It's $ufilled with a $U liquid.",
			  ch,
			  obj->value[1] < 0 ?
				"" :
			  obj->value[1] < obj->value[0] / 4 ?
				"less than half-" :
			  obj->value[1] < 3 * obj->value[0] / 4 ?
			 	"about half-" :
			 	"more than half-",
			  obj, liq_table[obj->value[2]].liq_color,
			  TO_CHAR, POS_DEAD);
		break;

	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
	case ITEM_CORPSE_PC:
		if (IS_SET(obj->value[1], CONT_CLOSED) 
		&&  (!ch->clan ||
		     clan_lookup(ch->clan)->altar_ptr != obj)) {
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
	if ((!room_is_dark(ch) || IS_AFFECTED(ch, AFF_DARK_VISION))
	&& check_blind_raw(ch)) {
		const char *name;
		const char *engname;

		name = mlstr_cval(&ch->in_room->name, ch);
		engname = mlstr_mval(&ch->in_room->name);
		char_printf(ch, "{W%s", name);
		if (GET_LANG(ch) && name != engname)
			char_printf(ch, " (%s){x", engname);
		else
			char_puts("{x", ch);
		
		if (IS_IMMORTAL(ch)
		||  IS_BUILDER(ch, ch->in_room->area))
			char_printf(ch, " [Room %d]",ch->in_room->vnum);

		char_puts("\n", ch);

 		if (!IS_SET(flags, LOOK_F_NORDESC)) {
			char_puts("  ", ch);
			act_puts(mlstr_cval(&ch->in_room->description, ch),
				 ch, NULL, NULL, TO_CHAR | ACT_NOLF, POS_DEAD);
		}

		if (!IS_NPC(ch) && IS_SET(PC(ch)->plr_flags, PLR_AUTOEXIT)) {
			char_puts("\n", ch);
			do_exits(ch, "auto");
		}
	}
	else 
		char_puts("It is pitch black...\n", ch);

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
		char_puts("You can't see anything but stars!\n", ch);
		return;
	}

	if (ch->position == POS_SLEEPING) {
		char_puts("You can't see anything, you're sleeping!\n", ch);
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
			char_puts("Look in what?\n", ch);
			return;
		}

		do_look_in(ch, arg2);
		return;
	}

	if ((victim = get_char_room(ch, arg1)) != NULL) {
		show_char_to_char_1(victim, ch);

		/* Love potion */
		if (is_affected(ch, gsn_love_potion) && (victim != ch)) {
			AFFECT_DATA af;

			affect_strip(ch, gsn_love_potion);

			add_follower(ch, victim);
			set_leader(ch, victim);

			af.where = TO_AFFECTS;
			af.type = gsn_charm_person;
			af.level = ch->level;
			af.duration =  number_fuzzy(victim->level / 4);
			af.bitvector = AFF_CHARM;
			af.modifier = 0;
			af.location = 0;
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

			if (is_name(arg3, obj->name))
				if (++count == number) {
					char_puts("You see nothing special about it.\n", ch);
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

		if (is_name(arg3, obj->name))
			if (++count == number) {
				act_puts(format_long(&obj->description, ch),
					 ch, NULL, NULL, TO_CHAR,
					 POS_DEAD);
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

		   if (!str_cmp(arg1, "n") || !str_cmp(arg1, "north")) door = 0;
	else if (!str_cmp(arg1, "e") || !str_cmp(arg1, "east")) door = 1;
	else if (!str_cmp(arg1, "s") || !str_cmp(arg1, "south")) door = 2;
	else if (!str_cmp(arg1, "w") || !str_cmp(arg1, "west")) door = 3;
	else if (!str_cmp(arg1, "u") || !str_cmp(arg1, "up" )) door = 4;
	else if (!str_cmp(arg1, "d") || !str_cmp(arg1, "down")) door = 5;
	else {
		char_puts("You don't see that here.\n", ch);
		return;
	}

	/* 'look direction' */
	if ((pexit = ch->in_room->exit[door]) == NULL) {
		char_puts("Nothing special there.\n", ch);
		return;
	}

	if (!IS_NULLSTR(mlstr_mval(&pexit->description)))
		act_puts(mlstr_cval(&pexit->description, ch),
			 ch, NULL, NULL, TO_CHAR | ACT_NOLF, POS_DEAD);
	else
		char_puts("Nothing special there.\n", ch);

	if (pexit->keyword    != NULL
	&&  pexit->keyword[0] != '\0'
	&&  pexit->keyword[0] != ' ') {
		if (IS_SET(pexit->exit_info, EX_CLOSED)) {
			act_puts("The $d is closed.",
				 ch, NULL, pexit->keyword, TO_CHAR, POS_DEAD);
		}
		else if (IS_SET(pexit->exit_info, EX_ISDOOR))
			act_puts("The $d is open.",
				 ch, NULL, pexit->keyword, TO_CHAR, POS_DEAD);
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
		char_puts("You can't see anything but stars!\n", ch);
		return;
	}

	if (ch->position == POS_SLEEPING) {
		char_puts("You can't see anything, you're sleeping!\n", ch);
		return;
	}

	if (!check_blind(ch))
		return;

	if (arg[0] == '\0') {
		char_puts("Examine what?\n", ch);
		return;
	}

	do_look(ch, arg);

	if ((obj = get_obj_here(ch, arg)) == NULL)
		return;

	switch (obj->pObjIndex->item_type) {
	case ITEM_MONEY: {
		const char *msg;

		if (obj->value[0] == 0) {
			if (obj->value[1] == 0)
				msg = "Odd...there's no coins in the pile.";
			else if (obj->value[1] == 1)
				msg = "Wow. One gold coin.";
			else
				msg = "There are $J $qJ{gold coins} in this pile.";
		}
		else if (obj->value[1] == 0) {
			if (obj->value[0] == 1)
				msg = "Wow. One silver coin.";
			else
				msg = "There are $j $qj{silver coins} in the pile.";
		}
		else {
			msg = "There are $J gold and $j $qj{silver coins} in the pile."; 
		}
		act_puts3(msg, ch,
			  (const void*) obj->value[0], NULL,
			  (const void*) obj->value[1],
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

	fAuto  = !str_cmp(argument, "auto");

	if (fAuto)
		char_puts("{C[Exits:", ch);
	else if (IS_IMMORTAL(ch) || IS_BUILDER(ch, ch->in_room->area))
		char_printf(ch, "Obvious exits from room %d:\n",
			    ch->in_room->vnum);
	else
		char_puts("Obvious exits:\n", ch);

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
				else if ((chance = get_skill(ch, gsn_perception))){
					if (number_percent() < chance) {
						check_perception = TRUE;
						show_closed = TRUE;
					}
				}
				if (!show_closed)
					continue;
			}

			found = TRUE;
			if (fAuto)
				char_printf(ch, " %s%s", dir_name[door],
					    show_closed ? "*" : str_empty);
			else {
				char_printf(ch, "{C%-5s%s{x - %s",
					    capitalize(dir_name[door]),
					    show_closed ? "*" : str_empty,
					    room_dark(pexit->to_room.r) ?
					    GETMSG("Too dark to tell", GET_LANG(ch)) :
					    mlstr_cval(&pexit->to_room.r->name, ch));
				if (IS_IMMORTAL(ch)
				||  IS_BUILDER(ch, pexit->to_room.r->area))
					char_printf(ch, " (room %d)",
						    pexit->to_room.r->vnum);
				char_puts("\n", ch);
			}
		}
	}

	if (!found)
		char_puts(fAuto ? " none" : "None.\n", ch);

	if (fAuto)
		char_puts("]{x\n", ch);

	if (check_perception)
		check_improve(ch, gsn_perception, TRUE, 5);
}

void do_worth(CHAR_DATA *ch, const char *argument)
{
	char_printf(ch, "You have %d gold, %d silver", ch->gold, ch->silver);
	if (!IS_NPC(ch) && ch->level < LEVEL_HERO)
		char_printf(ch, ", and %d experience (%d exp to level)",
			    PC(ch)->exp, exp_to_level(ch));
	char_puts(".\n", ch);

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
	"the Moon", "the Bull", "Deception", "Thunder", "Freedom",
	"the Great Gods", "the Sun"
};

static const char* month_name[] =
{
	"Winter", "the Winter Wolf", "the Frost Giant", "the Old Forces",
	"the Grand Struggle", "the Spring", "Nature", "Futility", "the Dragon",
	"the Sun", "the Heat", "the Battle", "the Dark Shades", "the Shadows",
	"the Long Shadows", "the Ancient Darkness", "the Great Evil"
};

void do_time(CHAR_DATA *ch, const char *argument)
{
	extern char str_boot_time[];
	char *suf;
	int day;

	day	= time_info.day + 1;

	     if (day > 4 && day <  20) suf = "th";
	else if (day % 10 ==  1      ) suf = "st";
	else if (day % 10 ==  2      ) suf = "nd";
	else if (day % 10 ==  3      ) suf = "rd";
	else			       suf = "th";

	char_printf(ch,
		    "It is %d o'clock %s, Day of %s, %d%s the Month of %s.\n",
		    (time_info.hour % 12 == 0) ? 12 : time_info.hour %12,
		    time_info.hour >= 12 ? "pm" : "am",
		    day_name[day % 7],
		    day, suf, month_name[time_info.month]);

	if (!IS_SET(ch->in_room->room_flags, ROOM_INDOORS) || IS_IMMORTAL(ch))
		act_puts("It's $T.", ch, NULL,
			(time_info.hour>=5 && time_info.hour<9) ?   "dawn"    :
			(time_info.hour>=9 && time_info.hour<12) ?  "morning" :
			(time_info.hour>=12 && time_info.hour<18) ? "mid-day" :
			(time_info.hour>=18 && time_info.hour<21) ? "evening" :
								    "night",
			TO_CHAR, POS_DEAD);

	if (!IS_IMMORTAL(ch))
		return;

	char_printf(ch, "\nSoG started up at %s.\n"
			"The system time is %s.\n"
			"Reboot in %d minutes.\n",
			str_boot_time, strtime(time(NULL)),reboot_counter);
}

void do_date(CHAR_DATA *ch, const char *argument)
{
	char_printf(ch, "%s\n", strtime(time(NULL)));
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
		char_puts("You can't see the weather indoors.\n", ch);
		return;
	}

	char_printf(ch, "The sky is %s and %s.\n",
		    sky_look[weather_info.sky],
		    weather_info.change >= 0 ?
		    "a warm southerly breeze blows" :
		    "a cold northern gust blows");
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
	flag32_t flags = 0;
	flag32_t ralign = 0;
	flag32_t rethos = 0;

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
		int i;
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
				char_puts("You haven't got a tattoo yet!\n", ch);
				goto bail_out;
			}
			SET_BIT(flags, WHO_F_TATTOO);
			tattoo_vnum = obj->pObjIndex->vnum;
			continue;
		}

		if (!str_cmp(arg, "clan")) {
			if (IS_IMMORTAL(ch))
				SET_BIT(flags, WHO_F_CLAN);
			else if (ch->clan) {
				SET_BIT(flags, WHO_F_RCLAN);
				i = ch->clan;
				name_add(&clan_names, CLAN(i)->name, NULL,NULL);
			}
			continue;
		}

		if (!IS_IMMORTAL(ch))
			continue;

		if ((i = cln_lookup(arg)) > 0) {
			name_add(&clan_names, CLAN(i)->name, NULL, NULL);
			SET_BIT(flags, WHO_F_RCLAN);
			continue;
		}

		if ((i = rn_lookup(arg)) > 0 && RACE(i)->race_pcdata) {
			name_add(&race_names, RACE(i)->name, NULL, NULL);
			SET_BIT(flags, WHO_F_RRACE);
			continue;
		}

		if ((i = cn_lookup(arg)) >= 0) {
			name_add(&class_names, CLASS(i)->name, NULL, NULL);
			SET_BIT(flags, WHO_F_RCLASS);
			continue;
		}

		if ((p = strchr(arg, '-'))) {
			*p++ = '\0';
			if (arg[0]) {
				if ((i = flag_value(ethos_table, arg)))
					SET_BIT(rethos, i);
				else
					char_printf(ch, "%s: unknown ethos.\n", arg);
			}
			if (*p) {
				if ((i = flag_value(ralign_names, p)))
					SET_BIT(ralign, i);
				else
					char_printf(ch, "%s: unknown align.\n", p);
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
				char_printf(ch,
					    "%s: explicit argument (skipped)\n",
					    arg);
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
		race_t *race;
		class_t *class;

		if (d->connected != CON_PLAYING)
			continue;

		wch = d->original ? d->original : d->character;
		if (!wch || !can_see(ch, wch))
			continue;

		if (is_affected(wch, gsn_vampire)
		&&  !IS_IMMORTAL(ch) && ch != wch)
			continue;

		if (wch->level < iLevelLower || wch->level > iLevelUpper
		||  (IS_SET(flags, WHO_F_IMM) && wch->level < LEVEL_IMMORTAL)
		||  (IS_SET(flags, WHO_F_PK) && (IS_IMMORTAL(wch) ||
						 !in_PK(ch, wch)))
		||  (IS_SET(flags, WHO_F_CLAN) && !wch->clan)
		||  (ralign && ((RALIGN(wch) & ralign) == 0))
		||  (rethos && ((wch->ethos & rethos) == 0)))
			continue;

		if (IS_SET(flags, WHO_F_TATTOO)) {
			if ((obj = get_eq_char(wch, WEAR_TATTOO)) == NULL
			||  tattoo_vnum != obj->pObjIndex->vnum)
				continue;
		}

		if (IS_SET(flags, WHO_F_RCLAN)) {
			if (!wch->clan
			||  (clan = clan_lookup(wch->clan)) == NULL
			||  !is_name(clan->name, clan_names))
				continue;
		}

		if (IS_SET(flags, WHO_F_RRACE)) {
			if ((race = race_lookup(wch->race)) == NULL
			||  !is_name(race->name, race_names))
				continue;
		}

		if (IS_SET(flags, WHO_F_RCLASS)) {
			if ((class = class_lookup(wch->class)) == NULL
			||  !is_name(class->name, class_names))
				continue;
		}

		count++;
		do_who_raw(ch, wch, output);
	}

	buf_printf(output, "{x\nPlayers found: %d. Most so far today: %d.\n",
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
		char_puts("You must provide a name.\n", ch);
		return;
	}

	for (d = descriptor_list; d != NULL; d = d->next) {
		CHAR_DATA *wch;

		if (d->connected != CON_PLAYING || !can_see(ch,d->character))
				continue;

		if (d->connected != CON_PLAYING
		||  (is_affected(d->character, gsn_vampire) &&
		     !IS_IMMORTAL(ch) && (ch != d->character)))
			continue;

		wch = (d->original != NULL) ? d->original : d->character;

		if (!can_see(ch, wch))
			continue;

		if (!str_prefix(arg, wch->name)) {
			if (output == NULL)
				output = buf_new(-1);
			count++;
			do_who_raw(ch, wch, output);
		}
	}

	if (output == NULL) {
		char_puts("No one of that name is playing.\n", ch);
		return;
	}

	buf_printf(output, "{x\nPlayers found: %d. Most so far today: %d.\n",
		   count, top_player);
	page_to_char(buf_string(output), ch);
	buf_free(output);
}

void do_inventory(CHAR_DATA *ch, const char *argument)
{
	char_puts("You are carrying:\n", ch);
	show_list_to_char(ch->carrying, ch, TRUE, TRUE);
}

void do_equipment(CHAR_DATA *ch, const char *argument)
{
	OBJ_DATA *obj;
	int i;
	bool found;

	char_puts("You are using:\n", ch);
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
		char_puts("Nothing.\n", ch);
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
		char_puts("Compare what to what?\n", ch);
		return;
	}

	if ((obj1 = get_obj_carry(ch, arg1)) == NULL) {
		char_puts("You do not have that item.\n", ch);
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
			char_puts("You aren't wearing anything comparable.\n", ch);
			return;
		}
	}
	else if ((obj2 = get_obj_carry(ch,arg2)) == NULL) {
		char_puts("You do not have that item.\n", ch);
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
			value1 = obj1->value[0]+obj1->value[1]+obj1->value[2];
			value2 = obj2->value[0]+obj2->value[1]+obj2->value[2];
			break;

		case ITEM_WEAPON:
			value1 = (1 + obj1->value[2]) * obj1->value[1];
			value2 = (1 + obj2->value[2]) * obj2->value[1];
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
		char_puts("It's too dark to see.\n", ch);
		return;
	}

	if (!str_cmp(arg,"pk"))
		fPKonly = TRUE;

	if (arg[0] == '\0' || fPKonly) {
		char_puts("Players near you:\n", ch);
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

				if (is_affected(victim, gsn_doppelganger)
				&&  (IS_NPC(ch) || !IS_SET(PC(ch)->plr_flags, PLR_HOLYLIGHT)))
					doppel = victim->doppel;
				else
					doppel = victim;

				char_printf(ch, "%s%-28s %s\n",
					(in_PK(ch, doppel) &&
					!IS_IMMORTAL(ch)) ?
					"{r[{RPK{r]{x " : "     ",
					PERS(victim, ch),
					mlstr_mval(&victim->in_room->name));
			}
		}
		if (!found)
			char_puts("None.\n", ch);
	}
	else {
		found = FALSE;
		for (victim = char_list; victim; victim = victim->next) {
			if (victim->in_room
			&&  victim->in_room->area == ch->in_room->area
			&&  can_see(ch, victim)
			&&  is_name(arg, victim->name)) {
				found = TRUE;
				char_printf(ch, "%-28s %s\n",
					PERS(victim, ch),
					mlstr_mval(&victim->in_room->name));
				break;
			}
		}
		if (!found) {
			act_puts("You didn't find any $T.",
				 ch, NULL, arg, TO_CHAR, POS_DEAD);
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
		char_puts("Consider killing whom?\n", ch);
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	if (victim == ch) {
		char_puts("Suicide is against your way.\n", ch);
		return;
	}

	if (!in_PK(ch, victim)) {
		char_puts("Don't even think about it.\n", ch);
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
	char arg[MAX_STRING_LENGTH];

	if (IS_NPC(ch)) {
		char_puts("Huh?\n", ch);
		return;
	}

	one_argument(argument, arg, sizeof(arg));

	if (!str_prefix(arg, "edit")) {
		string_append(ch, mlstr_convert(&ch->description, -1));
		return;
	}

	char_printf(ch, "Your description is:\n"
			 "%s\n"
			 "{xUse 'desc edit' to edit your description.\n",
		    mlstr_mval(&ch->description));
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
	class_t *cl;

	if ((cl = class_lookup(ch->class))
	&&  !CAN_FLEE(ch, cl)) {
		char_printf(ch, "You don't deal with wimpies, "
				"or such feary things.\n");
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
		char_puts("Your courage exceeds your wisdom.\n", ch);
		return;
	}

	if (wimpy > ch->max_hit/2) {
		char_puts("Such cowardice ill becomes you.\n", ch);
		return;
	}

	ch->wimpy	= wimpy;

	char_printf(ch, "Wimpy set to %d hit points.\n", wimpy);
	return;
}

void do_password(CHAR_DATA *ch, const char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char *pwdnew;

	if (IS_NPC(ch))
		return;

	argument = first_arg(argument, arg1, sizeof(arg1), FALSE);
	argument = first_arg(argument, arg2, sizeof(arg2), FALSE);

	if (arg1[0] == '\0' || arg2[0] == '\0') {
		char_puts("Syntax: password <old> <new>.\n", ch);
		return;
	}

	if (strcmp(crypt(arg1, PC(ch)->pwd), PC(ch)->pwd)) {
		WAIT_STATE(ch, 10 * PULSE_PER_SECOND);
		char_puts("Wrong password.  Wait 10 seconds.\n", ch);
		return;
	}

	if (strlen(arg2) < 5) {
		char_puts("New password must be at least "
			     "five characters long.\n", ch);
		return;
	}

	/*
	 * No tilde allowed because of player file format.
	 */
	pwdnew = crypt(arg2, ch->name);
	if (strchr(pwdnew, '~') != NULL) {
		char_puts("New password not acceptable, "
			     "try again.\n", ch);
		return;
	}

	free_string(PC(ch)->pwd);
	PC(ch)->pwd = str_dup(pwdnew);
	char_save(ch, 0);
	char_puts("Ok.\n", ch);
}

static void scan_list(ROOM_INDEX_DATA *scan_room, CHAR_DATA *ch, 
		      int depth, int door)
{
	CHAR_DATA *rch;

	if (scan_room == NULL) 
		return;

	for (rch = scan_room->people; rch; rch = rch->next_in_room) {
		if (rch == ch || !can_see(ch, rch))
			continue;
		char_printf(ch, "    %s.\n", PERS(rch, ch));
	}
}

static void scan_all(CHAR_DATA *ch)
{
	EXIT_DATA *pExit;
	int door;

	act("$n looks all around.", ch, NULL, NULL, TO_ROOM);
	if (!check_blind(ch))
		return;

	char_puts("Looking around you see:\n", ch);

	char_puts("{Chere{x:\n", ch);
	scan_list(ch->in_room, ch, 0, -1);
	for (door = 0; door < 6; door++) {
		if ((pExit = ch->in_room->exit[door]) == NULL
		|| !pExit->to_room.r
		|| !can_see_room(ch,pExit->to_room.r))
			continue;
		char_printf(ch, "{C%s{x:\n", dir_name[door]);
		if (IS_SET(pExit->exit_info, EX_CLOSED)) {
			char_puts("    You see closed door.\n", ch);
			continue;
		}
		scan_list(pExit->to_room.r, ch, 1, door);
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
	CHAR_DATA *person;
	int numpeople;

	one_argument(argument, dir, sizeof(dir));
	if (strchr(dir, '.')) {
		range = number_argument(dir, dir, sizeof(dir));
		if (range > 1 + ch->level/10) {
			act("You cannot see that far.",
			    ch, NULL, NULL, TO_CHAR);
			return;
		}
	}
	else
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
		char_puts("Wrong direction.\n", ch);
		return;
	}

	act("$n scans $t.", ch, dir_name[door], NULL, TO_ROOM);
	if (!check_blind(ch))
		return;

	act_puts("You scan $t.", ch, dir_name[door], NULL, TO_CHAR, POS_DEAD);

	in_room = ch->in_room;
	for (i = 1; i <= range; i++) {
		exit = in_room->exit[door];
		if (!exit)
			return;
		to_room = exit->to_room.r;
		if (!to_room)
			return;

		if (IS_SET(exit->exit_info,EX_CLOSED)
		&&  can_see_room(ch,exit->to_room.r)) {
			if (i == 1)
				char_puts("	You see closed door.\n", ch);
			return;
		}
		for (numpeople = 0, person = to_room->people; person != NULL;
		     person = person->next_in_room)
			if (can_see(ch,person)) {
				numpeople++;
				break;
			}

		if (numpeople) {
			char_printf(ch, "***** Range %d *****\n", i);
			show_char_to_char(to_room->people, ch);
			char_puts("\n", ch);
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

	if (is_affected(ch, gsn_reserved)) {
		char_puts("Wait for a while to request again.\n", ch);
		return;
	}

	argument = one_argument(argument, arg1, sizeof(arg1));
	argument = one_argument(argument, arg2, sizeof(arg2));

	if (IS_NPC(ch))
		return;

	if (arg1[0] == '\0' || arg2[0] == '\0') {
		char_puts("Request what from whom?\n", ch);
		return;
	}

	if ((victim = get_char_room(ch, arg2)) == NULL) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	if (!IS_NPC(victim)) {
		char_puts("Why don't you just ask the player?\n", ch);
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

	WAIT_STATE(ch, PULSE_VIOLENCE);
	ch->move -= 10;
	ch->move = UMAX(ch->move, 0);

	if (victim->level >= ch->level + 10 || victim->level >= ch->level * 2) {
		do_say(victim, "In good time, my child");
		return;
	}

	if (((obj = get_obj_carry(victim , arg1)) == NULL
	&&  (obj = get_obj_wear(victim, arg1)) == NULL)
	||  IS_SET(obj->extra_flags, ITEM_INVENTORY)) {
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
		do_say(victim, "Sorry, I can't let go of it.  It's cursed.");
		return;
	}

	if ((carry_n = can_carry_n(ch)) >= 0
	&&  ch->carry_number + get_obj_number(obj) > carry_n) {
		char_puts("Your hands are full.\n", ch);
		return;
	}

	if ((carry_w = can_carry_w(ch)) >= 0
	&&  get_carry_weight(ch) + get_obj_weight(obj) > carry_w) {
		char_puts("You can't carry that much weight.\n", ch);
		return;
	}

	if (!can_see_obj(ch, obj)) {
		act("You don't see that.", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (IS_SET(obj->extra_flags, ITEM_QUIT_DROP)) {
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
	char_puts("and for the goodness you have seen in the world.\n",ch);

	af.type = gsn_reserved;
	af.where = TO_AFFECTS;
	af.level = ch->level;
	af.duration = ch->level / 10;
	af.location = APPLY_NONE;
	af.modifier = 0;
	af.bitvector = 0;
	affect_to_char(ch, &af);
}

void do_hometown(CHAR_DATA *ch, const char *argument)
{
	int amount;
	int htn;
	race_t *r;
	class_t *cl;
	PC_DATA *pc;

	if (IS_NPC(ch)) {
		act_puts("You can't change your hometown!",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		return;
	}

	if ((r = race_lookup(ORG_RACE(ch))) == NULL
	||  !r->race_pcdata
	||  (cl = class_lookup(ch->class)) == NULL)
		return;

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
		char_puts("Choose from: ", ch);
		hometown_print_avail(ch);
		char_puts(".\n", ch);
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
	int		sn;

	if ((sn = sn_lookup("detect hide")) < 0
	||  (chance = get_skill(ch, sn)) == 0) {
		char_puts("Huh?\n", ch);
		return;
	}

	if (IS_AFFECTED(ch, AFF_DETECT_HIDDEN)) {
		char_puts("You are already as alert as you can be. \n",ch);
		return;
	}

	if (number_percent() > chance) {
		char_puts("You peer intently at the shadows "
			     "but they are unrevealing.\n", ch);
		check_improve(ch, sn, FALSE, 1);
		return;
	}

	af.where     = TO_AFFECTS;
	af.type      = sn;
	af.level     = LEVEL(ch);
	af.duration  = LEVEL(ch);
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_DETECT_HIDDEN;
	affect_to_char(ch, &af);
	char_puts("Your awareness improves.\n", ch);
	check_improve(ch, sn, TRUE, 1);
}

void do_awareness(CHAR_DATA *ch, const char *argument)
{
	AFFECT_DATA	af;
	int		chance;
	int		sn;

	if ((sn = sn_lookup("awareness")) < 0
	||  (chance = get_skill(ch, sn)) == 0) {
		char_puts("Huh?\n", ch);
		return;
	}

	if (IS_AFFECTED(ch, AFF_AWARENESS)) {
		char_puts("You are already as alert as you can be. \n",ch);
		return;
	}

	if (number_percent() > chance) {
		char_puts("You peer intently at the shadows "
			     "but they are unrevealing.\n", ch);
		check_improve(ch, sn, FALSE, 1);
		return;
	}

	af.where     = TO_AFFECTS;
	af.type      = sn;
	af.level     = LEVEL(ch);
	af.duration  = LEVEL(ch);
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_AWARENESS;
	affect_to_char(ch, &af);

	af.bitvector = AFF_ACUTE_VISION;
	affect_to_char(ch, &af);

	char_puts("Your awareness improves.\n", ch);
	check_improve(ch, sn, TRUE, 1);
}

void do_bear_call(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *	gch;
	CHAR_DATA *	bear;
	CHAR_DATA *	bear2;
	AFFECT_DATA	af;
	int		i;
	int		chance;
	int		sn;
	int		mana;

	if ((sn = sn_lookup("bear call")) < 0
	||  (chance = get_skill(ch, sn)) == 0) {
		char_puts("Huh?\n", ch);
		return;
	}

	char_puts("You call for bears help you.\n",ch);
	act("$n shouts a bear call.",ch,NULL,NULL,TO_ROOM);

	if (is_affected(ch, sn)) {
		char_puts("You cannot summon the strength to handle "
			     "more bears right now.\n", ch);
		return;
	}

	for (gch = npc_list; gch; gch = gch->next) {
		if (IS_AFFECTED(gch, AFF_CHARM)
		&&  gch->master == ch
		&&  gch->pMobIndex->vnum == MOB_VNUM_BEAR) {
			char_puts("What's wrong with the bear you've got?",
				     ch);
			return;
		}
	}

	if (ch->in_room != NULL
	&&  IS_SET(ch->in_room->room_flags, ROOM_NOMOB)) {
		char_puts("No bears listen you.\n", ch);
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
		char_puts("No bears come to your rescue.\n", ch);
		return;
	}

	mana = SKILL(sn)->min_mana;
	if (ch->mana < mana) {
		char_puts("You don't have enough mana "
			     "to shout a bear call.\n", ch);
		return;
	}
	ch->mana -= mana;

	if (number_percent() > chance) {
		char_puts("No bears listen you.\n", ch);
		check_improve(ch, sn, FALSE, 1);
		return;
	}

	check_improve(ch, sn, TRUE, 1);
	bear = create_mob(get_mob_index(MOB_VNUM_BEAR), 0);

	for (i=0;i < MAX_STATS; i++)
		bear->perm_stat[i] = UMIN(25,2 * ch->perm_stat[i]);

	SET_HIT(bear, ch->perm_hit);
	SET_MANA(bear, ch->perm_mana);
	bear->alignment = ch->alignment;
	bear->level = UMIN(100, 1 * ch->level-2);
	for (i=0; i < 3; i++)
		bear->armor[i] = interpolate(bear->level, 100, -100);
	bear->armor[3] = interpolate(bear->level, 100, 0);
	bear->sex = ch->sex;
	bear->gold = 0;
	bear->silver = 0;

	bear2 = create_mob(bear->pMobIndex, 0);
	clone_mob(bear, bear2);

	SET_BIT(bear->affected_by, AFF_CHARM);
	SET_BIT(bear2->affected_by, AFF_CHARM);
	bear->master = bear2->master = ch;
	bear->leader = bear2->leader = ch;

	char_puts("Two bears come to your rescue!\n",ch);
	act("Two bears come to $n's rescue!", ch, NULL, NULL, TO_ROOM);

	af.where	= TO_AFFECTS;
	af.type 	= sn;
	af.level	= ch->level;
	af.duration	= SKILL(sn)->beats;
	af.bitvector	= 0;
	af.modifier	= 0;
	af.location	= APPLY_NONE;
	affect_to_char(ch, &af);

	char_to_room(bear, ch->in_room);
	char_to_room(bear2, ch->in_room);
}

void do_identify(CHAR_DATA *ch, const char *argument)
{
	OBJ_DATA *obj;
	CHAR_DATA *rch;

	if ((obj = get_obj_carry(ch, argument)) == NULL) {
		 char_puts("You are not carrying that.\n", ch);
		 return;
	}

	for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
		if (IS_NPC(rch) && IS_SET(rch->pMobIndex->act, ACT_SAGE))
			break;

	if (!rch) {
		 char_puts("No one here seems to know much "
			      "about that.\n", ch);
		 return;
	}

	if (IS_IMMORTAL(ch))
		act("$n looks at you!", rch, obj, ch, TO_VICT);
	else if (ch->gold < 1) {
		act("$n resumes to identify by looking at $p.",
		       rch, obj, 0, TO_ROOM);
		char_puts("You need at least 1 gold.\n", ch);
		return;
	}
	else {
		ch->gold -= 1;
		char_puts("Your purse feels lighter.\n", ch);
	}

	act("$n gives a wise look at $p.", rch, obj, 0, TO_ROOM);
	spellfun_call("identify", 0, ch, obj);
}

static void format_stat(char *buf, size_t len, CHAR_DATA *ch, int stat)
{
	if (ch->level < 20 && !IS_NPC(ch))
		strnzcpy(buf, len, get_stat_alias(ch, stat));
	else {
		snprintf(buf, len, "%2d (%2d)",
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
	class_t *cl;
	BUFFER *output;
	bool can_flee;

	if ((cl = class_lookup(ch->class)) == NULL)
		return;

	output = buf_new(GET_LANG(ch));
	buf_add(output, "\n      {G/~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~/~~\\{x\n");

	strnzcpy(title, sizeof(title),
		 IS_NPC(ch) ? " Believer of Chronos." : PC(ch)->title);
	name = IS_NPC(ch) ? capitalize(mlstr_val(&ch->short_descr, GET_LANG(ch))) :
			    ch->name;
	delta = strlen(title) - cstrlen(title) + MAX_CHAR_NAME - strlen(name);
	title[32+delta] = '\0';
	snprintf(buf2, sizeof(buf2), "     {G|{x   %%s%%-%ds {Y%%3d years old   {G|____|{x\n", 33+delta);
	buf_printf(output, buf2, name, title, get_age(ch));

	buf_add(output, "     {G|{C+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+{G|{x\n");

	format_stat(buf2, sizeof(buf2), ch, STAT_STR);
	buf_printf(output, "     {G| {RLevel: {x%-3d (%+3d)    {C| {RStr: {x%-11.11s {C| {RReligion  : {x%-10.10s {G|{x\n",
		   ch->level,
		   ch->add_level,
		   buf2,
		   religion_name(GET_RELIGION(ch)));

	format_stat(buf2, sizeof(buf2), ch, STAT_INT);
	buf_printf(output,
"     {G| {RRace : {x%-11.11s  {C| {RInt: {x%-11.11s {C| {RPractice  : {x%-3d        {G|{x\n",
		race_name(ch->race),
		buf2,
		IS_NPC(ch) ? 0 : PC(ch)->practice);

	format_stat(buf2, sizeof(buf2), ch, STAT_WIS);
	buf_printf(output,
"     {G| {RSex  : {x%-11.11s  {C| {RWis: {x%-11.11s {C| {RTrain     : {x%-3d        {G|{x\n",
		   ch->sex == 0 ?	"sexless" :
		   ch->sex == 1 ?	"male" :
					"female",
		   buf2,
		   IS_NPC(ch) ? 0 : PC(ch)->train);

	format_stat(buf2, sizeof(buf2), ch, STAT_DEX);
	buf_printf(output,
"     {G| {RClass: {x%-12.12s {C| {RDex: {x%-11.11s {C| {RQuest Pnts: {x%-5d      {G|{x\n",
		IS_NPC(ch) ? "mobile" : cl->name,
		buf2,
		IS_NPC(ch) ? 0 : PC(ch)->questpoints);

	format_stat(buf2, sizeof(buf2), ch, STAT_CON);
	buf_printf(output,
"     {G| {RAlign: {x%-12.12s {C| {RCon: {x%-11.11s {C| {R%-10.10s: {x%-3d        {G|{x\n",
		flag_string(align_names, NALIGN(ch)),
		buf2,
		IS_NPC(ch) ? "Quest?" : (IS_ON_QUEST(ch) ? "Quest Time" : "Next Quest"),
		IS_NPC(ch) ? 0 : abs(PC(ch)->questtime));
	can_flee = CAN_FLEE(ch, cl);
	format_stat(buf2, sizeof(buf2), ch, STAT_CHA);
	buf_printf(output,
"     {G| {REthos: {x%-12.12s {C| {RCha: {x%-11.11s {C| {R%s     : {x%-5d      {G|{x\n",
		IS_NPC(ch) ? "mobile" : flag_string(ethos_table, ch->ethos),
		buf2,
		can_flee ? "Wimpy" : "Death",
		can_flee ? ch->wimpy : PC(ch)->death);

	snprintf(buf2, sizeof(buf2), "%s %s.",
		 GETMSG("You are", GET_LANG(ch)),
		 GETMSG(flag_string(position_names, ch->position), GET_LANG(ch)));
	buf_printf(output, "     {G| {RHome : {x%-31.31s {C|{x %-22.22s {G|{x\n",
		IS_NPC(ch) ? "Midgaard" : hometown_name(PC(ch)->hometown),
		buf2);

	buf_add(output, "     {G|{C+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+{G|{x{x\n");

	if (!IS_NPC(ch)) {
		CHAR_DATA *vch;

		if ((vch = PC(ch)->guarding) != NULL) {
			ekle = 1;
			buf_printf(output,
"     {G| {GYou are guarding: {x%-12.12s                                  {G|{x\n",
				   vch->name);
		}

		if ((vch = PC(ch)->guarded_by) != NULL) {
			ekle = 1;
			buf_printf(output,
"     {G| {GYou are guarded by: {x%-12.12s                                {G|{x\n",
				    vch->name);
		}
	}

	if (!IS_NPC(ch)) {
		if (PC(ch)->condition[COND_DRUNK] > 10) {
			ekle = 1;
			buf_printf(output,
"     {G| {GYou are drunk.                                                  {G|{x\n");
		}

		if (PC(ch)->condition[COND_THIRST] <= 0) {
			ekle = 1;
			buf_printf(output,
"     {G| {YYou are thirsty.                                                {G|{x\n");
		}
/*		if (PC(ch)->condition[COND_FULL]   ==	0) */
		if (PC(ch)->condition[COND_HUNGER] <= 0) {
			ekle = 1;
			buf_printf(output,
"     {G| {YYou are hungry.                                                 {G|{x\n");
		}

		if (IS_SET(PC(ch)->plr_flags, PLR_GHOST)) {
			ekle = 1;
			buf_add(output,
"     {G| {cYou are ghost.                                                  {G|{x\n");
		}

		if (PC(ch)->condition[COND_BLOODLUST] <= 0) {
			ekle = 1;
			buf_printf(output,
"     {G| {YYou are hungry for blood.                                       {G|{x\n");
		}

		if (PC(ch)->condition[COND_DESIRE] <=  0) {
			ekle = 1;
			buf_printf(output,
"     {G| {YYou are desiring your home.                                     {G|{x\n");
		}
	}

	if (!IS_IMMORTAL(ch) && IS_PUMPED(ch)) {
		ekle = 1;
		buf_printf(output,
"     {G| {RYour adrenalin is gushing!                                      {G|{x\n");
	}

	if (ekle)
		buf_add(output,
"     {G|{C+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+{G|{x\n");

	buf_printf(output,
"     {G| {RItems Carried : {x%9d/%-9d {RArmor vs magic  : {x%5d     {G|{x\n",
		ch->carry_number, can_carry_n(ch),
		GET_AC(ch,AC_EXOTIC));

	buf_printf(output,
"     {G| {RWeight Carried: {x%9d/%-9d {RArmor vs bash   : {x%5d     {G|{x\n",
	get_carry_weight(ch), can_carry_w(ch), GET_AC(ch,AC_BASH));

	buf_printf(output,
"     {G| {RGold          : {Y%9d           {RArmor vs pierce : {x%5d     {G|{x\n",
		 ch->gold, GET_AC(ch,AC_PIERCE));

	buf_printf(output,
"     {G| {RSilver        : {W%9d           {RArmor vs slash  : {x%5d     {G|{x\n",
		 ch->silver, GET_AC(ch,AC_SLASH));

	buf_printf(output,
"     {G| {RCurrent exp   : {x%9d           {RSaves vs Spell  : {x%5d     {G|{x\n",
		GET_EXP(ch), ch->saving_throw);

	buf_printf(output,
"     {G| {RExp to level  : {x%9d           {RHitP: {x%5d/%-5d           {G|{x\n",
		IS_NPC(ch) ? 0 : exp_to_level(ch), ch->hit, ch->max_hit);

	buf_printf(output,
"     {G| {RHitroll       : {x%9d           {RMana: {x%5d/%-5d           {G|{x\n",
		   GET_HITROLL(ch),ch->mana, ch->max_mana);
	buf_printf(output,
"     {G| {RDamroll       : {x%9d           {RMove: {x%5d/%-5d           {G|{x\n",
		    GET_DAMROLL(ch), ch->move, ch->max_move);
	buf_add(output, "  {G/~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~/   |{x\n");
	buf_add(output, "  {G\\________________________________________________________________\\__/{x\n");

	if (IS_SET(ch->comm, COMM_SHOWAFF))
		show_affects(ch, output);
	send_to_char(buf_string(output), ch);
	buf_free(output);
}

void do_oscore(CHAR_DATA *ch, const char *argument)
{
	class_t *cl;
	char buf2[MAX_STRING_LENGTH];
	int i;
	BUFFER *output;

	if ((cl = class_lookup(ch->class)) == NULL)
		return;

	output = buf_new(GET_LANG(ch));

	buf_printf(output, "%s %s%s\n{x",
		GETMSG("You are", GET_LANG(ch)),
		IS_NPC(ch) ? capitalize(mlstr_val(&ch->short_descr, GET_LANG(ch))) :
			     ch->name,
		IS_NPC(ch) ? " The Believer of Chronos." : PC(ch)->title);

	buf_printf(output, "Level {c%d(%+d){x, {c%d{x years old (%d hours).\n",
		ch->level, ch->add_level, get_age(ch), get_hours(ch));

	buf_printf(output,
		"Race: {c%s{x  Sex: {c%s{x  Class: {c%s{x  "
		"Hometown: {c%s{x\n",
		race_name(ch->race),
		ch->sex == 0 ? "sexless" : ch->sex == 1 ? "male" : "female",
		IS_NPC(ch) ? "mobile" : cl->name,
		IS_NPC(ch) ? "Midgaard" : hometown_name(PC(ch)->hometown));

	buf_printf(output,
		"You have {c%d{x/{c%d{x hit, {c%d{x/{c%d{x mana, "
		"{c%d{x/{c%d{x movement.\n",
		ch->hit, ch->max_hit, ch->mana, ch->max_mana,
		ch->move, ch->max_move);

	if (!IS_NPC(ch)) {
		buf_printf(output,
			"You have {c%d{x practices and "
			"{c%d{x training sessions.\n",
			PC(ch)->practice, PC(ch)->train);
	}

	buf_printf(output, "You are carrying {c%d{x/{c%d{x items "
		"with weight {c%ld{x/{c%d{x pounds.\n",
		ch->carry_number, can_carry_n(ch),
		get_carry_weight(ch), can_carry_w(ch));

	if (ch->level > 20 || IS_NPC(ch))
		buf_printf(output,
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
		buf_printf(output,
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
		 ch->gold + ch->silver ? ch->gold + ch->silver == 1 ?
					"coin" : "coins" : str_empty);
	if (ch->gold)
		buf_printf(output, buf2, ch->gold, ch->silver);
	else
		buf_printf(output, buf2, ch->silver);

	/* KIO shows exp to level */
	if (!IS_NPC(ch) && ch->level < LEVEL_HERO)
		buf_printf(output, "You need {c%d{x exp to level.\n",
			exp_to_level(ch));

	if (!IS_NPC(ch))
		buf_printf(output,
			"Quest Points: {c%d{x.  "
			"%s: {c%d{x.\n",
			PC(ch)->questpoints, 
			IS_NPC(ch) ? "Quest?" : (IS_ON_QUEST(ch) ? 
					"Quest Time" : "Next Quest"),
			IS_NPC(ch) ? 0 : abs(PC(ch)->questtime));

	if (CAN_FLEE(ch, cl))
		buf_printf(output, "Wimpy set to {c%d{x hit points.",
			   ch->wimpy);
	else
		buf_printf(output, "Total {c%d{x deaths up to now.",
			   PC(ch)->death);

	if (!IS_NPC(ch)) {
		CHAR_DATA *vch;

		if ((vch = PC(ch)->guarding) != NULL) {
			buf_printf(output, "  You are guarding: {W%s{x\n",
			   	   vch->name);
		}

		if ((vch = PC(ch)->guarded_by) != NULL) {
			buf_printf(output, "  You are guarded by: {W%s{x\n",
				   vch->name);
		}
	}

	buf_add(output, "\n");

	if (!IS_NPC(ch)) {
		if (PC(ch)->condition[COND_DRUNK] > 10)
			buf_add(output, "You are {cdrunk{x.\n");

		if (PC(ch)->condition[COND_THIRST] <= 0)
			buf_add(output, "You are {rthirsty{x.\n");

/*		if (PC(ch)->condition[COND_FULL] == 0) */
		if (PC(ch)->condition[COND_HUNGER] <= 0)
			buf_add(output, "You are {rhungry{x.\n");
		if (PC(ch)->condition[COND_BLOODLUST] <= 0)
			buf_add(output, "You are {rhungry for {Rblood{x.\n");
		if (PC(ch)->condition[COND_DESIRE] <= 0)
			buf_add(output, "You are {rdesiring your home{x.\n");
		if (IS_SET(PC(ch)->plr_flags, PLR_GHOST))
			buf_add(output, "You are {cghost{x.\n");
	}

	buf_printf(output, "You are %s.\n",
		   GETMSG(flag_string(position_names, ch->position),
			  GET_LANG(ch)));

	if ((ch->position == POS_SLEEPING || ch->position == POS_RESTING ||
	     ch->position == POS_FIGHTING || ch->position == POS_STANDING)
	&& !IS_IMMORTAL(ch) && IS_PUMPED(ch))
		buf_add(output, "Your {radrenalin is gushing{x!\n");

	/* print AC values */
	if (ch->level >= 25) {
		buf_printf(output,
			   "Armor: pierce: {c%d{x  bash: {c%d{x  "
			   "slash: {c%d{x  magic: {c%d{x\n",
			   GET_AC(ch, AC_PIERCE), GET_AC(ch, AC_BASH),
			   GET_AC(ch, AC_SLASH), GET_AC(ch, AC_EXOTIC));

		buf_printf(output,
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

			buf_add(output, "You are ");
			if (GET_AC(ch,i) >= 101)
				buf_printf(output,
					   "{chopelessly vulnerable{x to %s.\n",
					   ac_name[i]);
			else if (GET_AC(ch,i) >= 80)
				buf_printf(output,
					   "{cdefenseless against{x %s.\n",
					   ac_name[i]);
			else if (GET_AC(ch,i) >= 60)
				buf_printf(output, "{cbarely protected{x from %s.\n",
					   ac_name[i]);
			else if (GET_AC(ch,i) >= 40)
				buf_printf(output, "{cslightly armored{x against %s.\n",
					   ac_name[i]);
			else if (GET_AC(ch,i) >= 20)
				buf_printf(output, "{csomewhat armored{x against %s.\n",
					   ac_name[i]);
			else if (GET_AC(ch,i) >= 0)
				buf_printf(output, "{carmored{x against %s.\n",
					   ac_name[i]);
			else if (GET_AC(ch,i) >= -20)
				buf_printf(output, "{cwell-armored{x against %s.\n",
					   ac_name[i]);
			else if (GET_AC(ch,i) >= -40)
				buf_printf(output, "{cvery well-armored{x against %s.\n",
					   ac_name[i]);
			else if (GET_AC(ch,i) >= -60)
				buf_printf(output, "{cheavily armored{x against %s.\n",
					   ac_name[i]);
			else if (GET_AC(ch,i) >= -80)
				buf_printf(output, "{csuperbly armored{x against %s.\n",
					   ac_name[i]);
			else if (GET_AC(ch,i) >= -100)
				buf_printf(output, "{calmost invulnerable{x to %s.\n",
					   ac_name[i]);
			else
				buf_printf(output, "{cdivinely armored{x against %s.\n",
					   ac_name[i]);
		}
	}

	/* RT wizinvis and holy light */
	if (IS_IMMORTAL(ch)) {
		buf_printf(output, "Holy Light: {c%s{x",
			   IS_SET(PC(ch)->plr_flags, PLR_HOLYLIGHT) ?
			   "on" : "off");

		if (ch->invis_level)
			buf_printf(output, "  Invisible: {clevel %d{x",
				ch->invis_level);

		if (ch->incog_level)
			buf_printf(output, "  Incognito: {clevel %d{x",
				ch->incog_level);
		buf_add(output, "\n");
	}

	if (ch->level >= 20)
		buf_printf(output, "Hitroll: {c%d{x  Damroll: {c%d{x.\n",
			GET_HITROLL(ch), GET_DAMROLL(ch));

	buf_add(output, "You are ");
	if (IS_GOOD(ch))
		buf_add(output, "good.");
	else if (IS_EVIL(ch))
		buf_add(output, "evil.");
	else
		buf_add(output, "neutral.");

	switch (ch->ethos) {
	case ETHOS_LAWFUL:
		buf_add(output, "  You have a lawful ethos.\n");
		break;
	case ETHOS_NEUTRAL:
		buf_add(output, "  You have a neutral ethos.\n");
		break;
	case ETHOS_CHAOTIC:
		buf_add(output, "  You have a chaotic ethos.\n");
		break;
	default:
		buf_add(output, "  You have no ethos");
		if (!IS_NPC(ch))
			buf_add(output, ", report it to the gods!\n");
		else
			buf_add(output, ".\n");
	}

	i = GET_RELIGION(ch);
	if (i <= RELIGION_NONE || i > MAX_RELIGION)
		buf_add(output, "You don't believe any religion.\n");
	else
		buf_printf(output,"Your religion is the way of %s.\n",
			   religion_table[i].leader);

	if (IS_SET(ch->comm, COMM_SHOWAFF))
		show_affects(ch, output);
	send_to_char(buf_string(output), ch);
	buf_free(output);
}

void do_affects(CHAR_DATA *ch, const char *argument)
{
	BUFFER *output;

	output = buf_new(GET_LANG(ch));
	show_affects(ch, output);
	page_to_char(buf_string(output), ch);
	buf_free(output);
}

void do_raffects(CHAR_DATA *ch, const char *argument)
{
	ROOM_AFFECT_DATA *paf, *paf_last = NULL;

	if (ch->in_room->affected == NULL) {
		char_puts("The room is not affected by any spells.\n",ch);
		return;
	}

	char_puts("The room is affected by the following spells:\n", ch);
	for (paf = ch->in_room->affected; paf != NULL; paf = paf->next) {
		if (paf_last != NULL && paf->type == paf_last->type)
			if (ch->level >= 20)
				char_puts("                      ", ch);
			else
				continue;
		else
			char_printf(ch, "Spell: {c%-15s{x",
				    skill_name(paf->type));

		if (ch->level >= 20) {
			char_printf(ch, ": modifies {c%s{x by {c%d{x ",
				    flag_string(rapply_flags, paf->location),
				    paf->modifier);
			if (paf->duration == -1 || paf->duration == -2)
				char_puts("permanently.", ch);
			else
				char_printf(ch, "for {c%d{x hours.",
					    paf->duration);
		}
		char_puts("\n", ch);
		paf_last = paf;
	}
}

void do_lion_call(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *	gch;
	CHAR_DATA *	lion;
	CHAR_DATA *	lion2;
	AFFECT_DATA	af;
	int		i;
	int		chance;
	int		sn;
	int		mana;

	if ((sn = sn_lookup("lion call")) < 0
	||  (chance = get_skill(ch, sn)) == 0) {
		char_puts("Huh?\n", ch);
		return;
	}

	char_puts("You call for lions help you.\n",ch);
	act("$n shouts a lion call.",ch,NULL,NULL,TO_ROOM);

	if (is_affected(ch, sn)) {
		char_puts("You cannot summon the strength to handle "
			     "more lions right now.\n", ch);
		return;
	}

	for (gch = npc_list; gch; gch = gch->next) {
		if (IS_AFFECTED(gch,AFF_CHARM)
		&&  gch->master == ch
		&&  gch->pMobIndex->vnum == MOB_VNUM_LION) {
			char_puts("What's wrong with the lion you've got?", ch);
			return;
		}
	}

	if (ch->in_room != NULL
	&& IS_SET(ch->in_room->room_flags, ROOM_NOMOB)) {
		char_puts("No lions hear you.\n", ch);
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
		char_puts("No lions come to your rescue.\n", ch);
		return;
	}

	mana = SKILL(sn)->min_mana;
	if (ch->mana < mana) {
		char_puts("You don't have enough mana "
			     "to shout a lion call.\n", ch);
		return;
	}
	ch->mana -= mana;

	if (number_percent() > chance) {
		check_improve(ch, sn, FALSE, 1);
		char_puts("No lions hear you.\n", ch);
		return;
	}

	check_improve(ch, sn, TRUE, 1);
	lion = create_mob(get_mob_index(MOB_VNUM_LION), 0);

	for (i=0;i < MAX_STATS; i++)
		lion->perm_stat[i] = UMIN(25,2 * ch->perm_stat[i]);

	SET_HIT(lion, ch->perm_hit);
	SET_MANA(lion, ch->perm_mana);
	lion->alignment = ch->alignment;
	lion->level = UMIN(100,1 * ch->level-2);
	for (i=0; i < 3; i++)
		lion->armor[i] = interpolate(lion->level,100,-100);
	lion->armor[3] = interpolate(lion->level,100,0);
	lion->sex = ch->sex;
	lion->gold = 0;
	lion->silver = 0;

	lion2 = create_mob(lion->pMobIndex, 0);
	clone_mob(lion,lion2);

	SET_BIT(lion->affected_by, AFF_CHARM);
	SET_BIT(lion2->affected_by, AFF_CHARM);
	lion->master = lion2->master = ch;
	lion->leader = lion2->leader = ch;

	char_puts("Two lions come to your rescue!\n",ch);
	act("Two lions come to $n's rescue!",ch,NULL,NULL,TO_ROOM);

	af.where	= TO_AFFECTS;
	af.type 	= sn;
	af.level	= ch->level;
	af.duration	= SKILL(sn)->beats;
	af.bitvector	= 0;
	af.modifier	= 0;
	af.location	= APPLY_NONE;
	affect_to_char(ch, &af);

	char_to_room(lion, ch->in_room);
	char_to_room(lion2, ch->in_room);
}

/* new practice */
void do_practice(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA	*mob;
	int		sn;
	skill_t	*sk;
	pcskill_t	*ps;
	class_t	*cl;
	cskill_t	*cs;
	int		adept;
	bool		found;
	int		rating;
	char		arg[MAX_STRING_LENGTH];
	PC_DATA	*	pc;

	if (IS_NPC(ch))
		return;

	pc = PC(ch);

	if (argument[0] == '\0') {
		BUFFER *output;
		int col = 0;
		int i;

		output = buf_new(-1);

		for (i = 0; i < pc->learned.nused; i++) {
			ps = VARR_GET(&pc->learned, i);

			if (ps->percent == 0
			||  (sk = skill_lookup(ps->sn)) == NULL
			||  skill_level(ch, ps->sn) > ch->level)
				continue;

			buf_printf(output, "%-19s %3d%%  ",
				   sk->name, ps->percent);
			if (++col % 3 == 0)
				buf_add(output, "\n");
		}

		if (col % 3)
			buf_add(output, "\n");

		buf_printf(output, "You have %d practice sessions left.\n",
			   pc->practice);

		page_to_char(buf_string(output), ch);
		buf_free(output);
		return;
	}

	if (!IS_AWAKE(ch)) {
		char_puts("In your dreams, or what?\n", ch);
		return;
	}	

	if ((cl = class_lookup(ch->class)) == NULL) {
		log("do_practice: %s: class %d: unknown",
			   ch->name, ch->class);
		return;
	}

	if (pc->practice <= 0) {
		char_puts("You have no practice sessions left.\n", ch);
		return;
	}

	one_argument(argument, arg, sizeof(arg));
	ps = (pcskill_t*) skill_vlookup(&pc->learned, arg);
	if (!ps || get_skill(ch, sn = ps->sn) == 0) {
		char_puts("You can't practice that.\n", ch);
		return;
	}

	if (sn == gsn_vampire) {
		char_puts("You can't practice that, only available "
			  "at questor.\n", ch);
		return;
	}

	found = FALSE;
	sk = SKILL(sn);
	for (mob = ch->in_room->people; mob != NULL; mob = mob->next_in_room) {
		if (!IS_NPC(mob) || !IS_SET(mob->pMobIndex->act, ACT_PRACTICE))
			continue;

		found = TRUE;

		if (IS_SET(sk->skill_flags, SKILL_CLAN)) {
			if (ch->clan == mob->clan)
				break;
			continue;
		}

		if ((mob->pMobIndex->practicer == 0 &&
		    (sk->group == GROUP_NONE ||
		     IS_SET(sk->group,	GROUP_CREATION | GROUP_HARMFUL |
					GROUP_PROTECTIVE | GROUP_DETECTION |
					GROUP_WEATHER)))
		||  IS_SET(mob->pMobIndex->practicer, sk->group))
			break;
	}

	if (mob == NULL) {
		if (found) {
			char_puts("You can't do that here. "
				  "Use 'slook skill', 'help practice' "
				  "for more info.\n", ch);
		} else {
			char_puts("You couldn't find anyone "
				  "who can teach you.\n", ch);
		}
		return;
	}

	adept = cl->skill_adept;
	if (ps->percent >= adept) {
		char_printf(ch, "You are already learned at %s.\n",
			    sk->name);
		return;
	}

	pc->practice--;

	cs = cskill_lookup(cl, sn);
	rating = cs ? UMAX(cs->rating, 1) : 1;
	ps->percent += int_app[get_curr_stat(ch, STAT_INT)].learn / rating;

	if (ps->percent < adept) {
		act("You practice $T.", ch, NULL, sk->name, TO_CHAR);
		act("$n practices $T.", ch, NULL, sk->name, TO_ROOM);
	}
	else {
		ps->percent = adept;
		act("You are now learned at $T.", ch, NULL, sk->name, TO_CHAR);
		act("$n is now learned at $T.", ch, NULL, sk->name, TO_ROOM);
	}
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
		&&  IS_SET(tr->pMobIndex->act,
			   ACT_PRACTICE | ACT_TRAIN | ACT_GAIN))
			break;
	}

	if (tr == NULL || !can_see(ch, tr)) {
		char_puts("You can't do that here.\n",ch);
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

/* RT spells and skills show the players spells (or skills) */
void do_spells(CHAR_DATA *ch, const char *argument)
{
	char spell_list[LEVEL_IMMORTAL+1][MAX_STRING_LENGTH];
	char spell_columns[LEVEL_IMMORTAL+1];
	int lev;
	int i;
	bool found = FALSE;
	char buf[MAX_STRING_LENGTH];
	BUFFER *output;

	if (IS_NPC(ch))
		return;
	
	/* initialize data */
	for (lev = 0; lev <= LEVEL_IMMORTAL; lev++) {
		spell_columns[lev] = 0;
		spell_list[lev][0] = '\0';
	}
	
	for (i = 0; i < PC(ch)->learned.nused; i++) {
		pcskill_t *ps = VARR_GET(&PC(ch)->learned, i);
		skill_t *sk;

		if (ps->percent == 0
		||  (sk = skill_lookup(ps->sn)) == NULL
		||  sk->skill_type != ST_SPELL)
			continue;

		found = TRUE;
		lev = skill_level(ch, ps->sn);

		if (lev > (IS_IMMORTAL(ch) ? LEVEL_IMMORTAL : LEVEL_HERO))
			continue;

		if (ch->level < lev)
			snprintf(buf, sizeof(buf), "%-19s n/a       ",
				 sk->name);
		else
			snprintf(buf, sizeof(buf), "%-19s %4d mana  ",
				 sk->name, mana_cost(ch, ps->sn));
			
		if (spell_list[lev][0] == '\0')
			snprintf(spell_list[lev], sizeof(spell_list[lev]),
				 "\nLevel %2d: %s", lev, buf);
		else { /* append */
			if (++spell_columns[lev] % 2 == 0)
				strnzcat(spell_list[lev],
					 sizeof(spell_list[lev]),
					 "\n          ");
			strnzcat(spell_list[lev], sizeof(spell_list[lev]),
				 buf);
		}
	}

	/* return results */
	
	if (!found) {
		char_puts("You know no spells.\n",ch);
		return;
	}
	
	output = buf_new(-1);
	for (lev = 0; lev <= LEVEL_IMMORTAL; lev++)
		if (spell_list[lev][0] != '\0')
			buf_add(output, spell_list[lev]);
	buf_add(output, "\n");
	page_to_char(buf_string(output), ch);
	buf_free(output);
}

void do_skills(CHAR_DATA *ch, const char *argument)
{
	char skill_list[LEVEL_IMMORTAL+1][MAX_STRING_LENGTH];
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
		skill_list[lev][0] = '\0';
	}
	
	for (i = 0; i < PC(ch)->learned.nused; i++) {
		pcskill_t *ps = VARR_GET(&PC(ch)->learned, i);
		skill_t *sk;

		if (ps->percent == 0
		||  (sk = skill_lookup(ps->sn)) == NULL
		||  sk->skill_type != ST_SKILL)
			continue;

		found = TRUE;
		lev = skill_level(ch, ps->sn);

		if (lev > (IS_IMMORTAL(ch) ? LEVEL_IMMORTAL : LEVEL_HERO))
			continue;

		snprintf(buf, sizeof(buf),
			 ch->level < lev ?
				"%-19s n/a      " : "%-19s %3d%%      ",
			 sk->name, ps->percent);

		if (skill_list[lev][0] == '\0')
			snprintf(skill_list[lev], sizeof(skill_list[lev]),
				 "\nLevel %2d: %s", lev, buf);
		else { /* append */
			if (++skill_columns[lev] % 2 == 0)
				strnzcat(skill_list[lev],
					 sizeof(skill_list[lev]),
					 "\n          ");
			strnzcat(skill_list[lev], sizeof(skill_list[lev]), buf);
		}
	}
	
	/* return results */
	
	if (!found) {
		char_puts("You know no skills.\n",ch);
		return;
	}
	
	output = buf_new(-1);
	for (lev = 0; lev <= LEVEL_IMMORTAL; lev++)
		if (skill_list[lev][0] != '\0')
			buf_add(output, skill_list[lev]);
	buf_add(output, "\n");
	page_to_char(buf_string(output), ch);
	buf_free(output);
}

void do_glist(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	int col = 0;
	flag64_t group = GROUP_NONE;
	int sn;

	one_argument(argument, arg, sizeof(arg));
	
	if (arg[0] == '\0') {
		char_puts("Syntax: glist group\n"
			  "Use 'glist ?' to get the list of groups.\n", ch);
		return;
	}

	if (!str_cmp(arg, "?")) {
		show_flags(ch, skill_groups);
		return;
	}

	if (str_prefix(arg, "none")
	&&  (group = flag_value(skill_groups, arg)) == 0) {
		char_puts("That is not a valid group.\n", ch);
		do_glist(ch, str_empty);
		return;
	}

	char_printf(ch, "Now listing group '%s':\n",
		    flag_string(skill_groups, group));

	for (sn = 0; sn < skills.nused; sn++) {
		skill_t *sk = VARR_GET(&skills, sn);
		if (group == sk->group) {
			char_printf(ch, "%c%-18s",
				    pcskill_lookup(ch, sn) ? '*' : ' ',
				    sk->name);
			if (col)
				char_puts("\n", ch);
			col = 1 - col;
		}
	}

	if (col)
		char_puts("\n", ch);
}

void do_slook(CHAR_DATA *ch, const char *argument)
{
	int sn = -1;
	char arg[MAX_INPUT_LENGTH];

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		char_puts("Syntax : slook <skill | spell>\n",ch);
		return;
	}

/* search in known skills first */
	if (!IS_NPC(ch)) {
		pcskill_t *ps;
		ps = (pcskill_t*) skill_vlookup(&PC(ch)->learned, arg);
		if (ps)
			sn = ps->sn;
	}

/* search in all skills */
	if (sn < 0)
		sn = sn_lookup(arg);

	if (sn < 0) { 
		char_puts("That is not a spell or skill.\n",ch);
		return; 
	}

	char_printf(ch, "Skill '%s' in group '%s'.\n",
		    SKILL(sn)->name,
		    flag_string(skill_groups, SKILL(sn)->group));
}

void do_learn(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	int sn;
	CHAR_DATA *practicer;
	int adept;
	class_t *cl;
	cskill_t *cs;
	pcskill_t *ps;
	skill_t *sk;
	int rating;
	PC_DATA *pc;

	if (IS_NPC(ch) || (cl = class_lookup(ch->class)) == NULL)
		return;

	if (!IS_AWAKE(ch)) {
		char_puts("In your dreams, or what?\n", ch);
		return;
	}	

	if (argument[0] == '\0') {
		char_puts("Syntax: learn <skill | spell> <player>\n", ch);
		return;
	}

	pc = PC(ch);

	if (pc->practice <= 0) {
		char_puts("You have no practice sessions left.\n", ch);
		return;
	}

	argument = one_argument(argument, arg, sizeof(arg));
	ps = (pcskill_t*) skill_vlookup(&pc->learned, arg);
	if (!ps || get_skill(ch, sn = ps->sn) == 0) {
		char_puts("You can't learn that.\n", ch);
		return;
	}

	if (sn == gsn_vampire) {
		char_puts("You can't practice that, only available "
			  "at questor.\n", ch);
		return;
	}	

	argument = one_argument(argument, arg, sizeof(arg));
		
	if ((practicer = get_char_room(ch,arg)) == NULL) {
		char_puts("Your hero is not here.\n", ch);
		return;
	}
			
	if (IS_NPC(practicer) || practicer->level != LEVEL_HERO) {
		char_puts("You must find a hero, not an ordinary one.\n",
			  ch);
		return;
	}

	if (!IS_SET(PC(practicer)->plr_flags, PLR_PRACTICER)) {
		char_puts("Your hero doesn't want to teach you anything.\n",ch);
		return;
	}

	if (get_skill(practicer, sn) < 100) {
		char_puts("Your hero doesn't know that skill enough to teach you.\n",ch);
		return;
	}

	sk = SKILL(sn);
	adept = cl->skill_adept;

	if (ps->percent >= adept) {
		char_printf(ch, "You are already learned at %s.\n",
			    sk->name);
		return;
	}

	pc->practice--;

	cs = cskill_lookup(cl, sn);
	rating = cs ? UMAX(cs->rating, 1) : 1;
	ps->percent += int_app[get_curr_stat(ch, STAT_INT)].learn / rating;

	act("You teach $T.", practicer, NULL, sk->name, TO_CHAR);
	act("$n teaches $T.", practicer, NULL, sk->name, TO_ROOM);
	REMOVE_BIT(PC(practicer)->plr_flags, PLR_PRACTICER);

	if (ps->percent < adept) {
		act("You learn $T.", ch, NULL, sk->name, TO_CHAR);
		act("$n learn $T.", ch, NULL, sk->name, TO_ROOM);
	}
	else {
		ps->percent = adept;
		act("You are now learned at $T.", ch, NULL, sk->name, TO_CHAR);
		act("$n is now learned at $T.", ch, NULL, sk->name, TO_ROOM);
	}
}

void do_teach(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch) || ch->level != LEVEL_HERO) {
		char_puts("You must be a hero.\n",ch);
		return;
	}
	SET_BIT(PC(ch)->plr_flags, PLR_PRACTICER);
	char_puts("Now, you can teach youngsters your 100% skills.\n",ch);
}

void do_camp(CHAR_DATA *ch, const char *argument)
{
	AFFECT_DATA af;
	ROOM_AFFECT_DATA raf;
	int sn;
	int chance;
	int mana;

	if ((sn = sn_lookup("camp")) < 0
	||  (chance = get_skill(ch, sn)) == 0) {
		char_puts("Huh?\n", ch);
		return;
	}

	if (is_affected(ch, sn)) {
		char_puts("You don't have enough power to handle more "
			     "camp areas.\n", ch);
		return;
	}

	if (IS_SET(ch->in_room->room_flags, ROOM_PEACE |
					    ROOM_PRIVATE | ROOM_SOLITARY)
	||  (ch->in_room->sector_type != SECT_FIELD
	&&   ch->in_room->sector_type != SECT_FOREST
	&&   ch->in_room->sector_type != SECT_MOUNTAIN
	&&   ch->in_room->sector_type != SECT_HILLS)) {
		char_puts("There are not enough leaves to camp here.\n",
			     ch);
		return;
	}

	mana = SKILL(sn)->min_mana;
	if (ch->mana < mana) {
		char_puts("You don't have enough mana to make a camp.\n",
			     ch);
		return;
	}
	ch->mana -= mana;

	if (number_percent() > chance) {
		char_puts("You failed to make your camp.\n", ch);
		check_improve(ch, sn, FALSE, 4);
		return;
	}

	check_improve(ch, sn, TRUE, 4);
	WAIT_STATE(ch, SKILL(sn)->beats);

	char_puts("You succeeded to make your camp.\n", ch);
	act("$n succeeded to make $s camp.", ch, NULL, NULL, TO_ROOM);

	af.where	= TO_AFFECTS;
	af.type 	= sn;
	af.level	= ch->level;
	af.duration	= 12;
	af.bitvector	= 0;
	af.modifier	= 0;
	af.location	= APPLY_NONE;
	affect_to_char(ch, &af);

	raf.where	= TO_ROOM_CONST;
	raf.type	= sn;
	raf.level	= ch->level;
	raf.duration	= ch->level / 20;
	raf.bitvector	= 0;
	raf.modifier	= 2 * LEVEL(ch);
	raf.location	= APPLY_ROOM_HEAL;
	raf.owner	= ch;
	raf.events	= 0;
	affect_to_room(ch->in_room, &raf);

	raf.modifier	= LEVEL(ch);
	raf.location	= APPLY_ROOM_MANA;
	affect_to_room(ch->in_room, &raf);
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
		char_puts("Demand what from whom?\n", ch);
		return;
	}

	if ((victim = get_char_room(ch, arg2)) == NULL) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	if (!IS_NPC(victim)) {
		char_puts("Why don't you just want that directly "
			     "from the player?\n", ch);
		return;
	}

	WAIT_STATE(ch, PULSE_VIOLENCE);

	chance = IS_EVIL(victim) ? 10 : IS_GOOD(victim) ? -5 : 0;
	chance += (get_curr_stat(ch,STAT_CHA) - 15) * 10;
	chance += LEVEL(ch) - LEVEL(victim);

	chance = (get_skill(ch, gsn_demand))*chance/100;

	if (number_percent() > chance) {
		do_say(victim, "I'm not about to give you anything!");
		check_improve(ch, gsn_demand, FALSE, 1);
		do_murder(victim, ch->name);
		return;
	}

	check_improve(ch, gsn_demand, TRUE, 1);

	if (((obj = get_obj_carry(victim , arg1)) == NULL
	&&   (obj = get_obj_wear(victim, arg1)) == NULL)
	||  IS_SET(obj->extra_flags, ITEM_INVENTORY)) {
		do_say(victim, "Sorry, I don't have that.");
		return;
	}

	if (IS_SET(obj->extra_flags, ITEM_QUIT_DROP)) {
		do_say(victim, "Forgive me, my master, I can't give it to anyone.");
		return;
	}

	if (obj->wear_loc != WEAR_NONE)
		unequip_char(victim, obj);

	if (!can_drop_obj(ch, obj)) {
		do_say(victim, "It's cursed so, I can't let go of it. "
			       "Forgive me, my master");
		return;
	}

	if ((carry_n = can_carry_n(ch)) >= 0
	&&  ch->carry_number + get_obj_number(obj) > carry_n) {
		char_puts("Your hands are full.\n", ch);
		return;
	}

	if ((carry_w = can_carry_w(ch)) >= 0
	&&  get_carry_weight(ch) + get_obj_weight(obj) > carry_w) {
		char_puts("You can't carry that much weight.\n", ch);
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
	char_puts("Your power makes all around the world shivering.\n",ch);
}

void do_control(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int chance;
	int sn;
	race_t *r;

	argument = one_argument(argument, arg, sizeof(arg));

	if ((sn = sn_lookup("control animal")) < 0
	||  (chance = get_skill(ch, sn)) == 0) {
		char_puts("Huh?\n", ch);
		return;
	}

	if (arg[0] == '\0') {
		char_puts("Charm what?\n", ch);
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	if ((r = race_lookup(ORG_RACE(victim))) <=0 
	|| !IS_SET(r->form, FORM_ANIMAL)) {
		char_puts("You should try this on animals?\n", ch);
		return;
	}

	if (count_charmed(ch))
		return;

	if (is_safe(ch, victim))
		return;

	WAIT_STATE(ch, SKILL(sn)->beats);

	chance += (get_curr_stat(ch,STAT_CHA) - 20) * 5;
	chance += (ch->level - victim->level) * 3;
	chance +=
	(get_curr_stat(ch,STAT_INT) - get_curr_stat(victim,STAT_INT)) * 5;

	if (IS_AFFECTED(victim, AFF_CHARM)
	||  IS_AFFECTED(ch, AFF_CHARM)
	||  number_percent() > chance
	||  ch->level < (victim->level + 2)
	||  IS_SET(victim->imm_flags,IMM_CHARM)
	||  (IS_NPC(victim) && victim->pMobIndex->pShop != NULL)) {
		check_improve(ch, sn, FALSE, 2);
		do_say(victim,"I'm not about to follow you!");
		do_murder(victim, ch->name);
		return;
	}

	check_improve(ch, sn, TRUE, 2);

	if (victim->master)
		stop_follower(victim);
	SET_BIT(victim->affected_by, AFF_CHARM);
	victim->master = victim->leader = ch;

	act("Isn't $n just so nice?", ch, NULL, victim, TO_VICT);
	if (ch != victim)
		act("$N looks at you with adoring eyes.",
		    ch, NULL, victim, TO_NOTVICT);
}

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
	int sn;
	int color = -1;

	if (IS_NPC(ch))
		return;

	if ((sn = sn_lookup("make arrow")) < 0
	||  (chance = get_skill(ch, sn)) == 0) {
		char_puts("You don't know how to make arrows.\n", ch);
		return;
	}

	if (ch->in_room->sector_type != SECT_FIELD
	&&  ch->in_room->sector_type != SECT_FOREST
	&&  ch->in_room->sector_type != SECT_HILLS) {
		char_puts("You couldn't find enough wood.\n", ch);
		return;
	}

	mana = SKILL(sn)->min_mana;
	wait = SKILL(sn)->beats;

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0')
		vnum = OBJ_VNUM_WOODEN_ARROW;
	else if (!str_prefix(arg, "green")) {
		color = sn_lookup("green arrow");
		vnum = OBJ_VNUM_GREEN_ARROW;
	}
	else if (!str_prefix(arg, "red")) {
		color = sn_lookup("red arrow");
		vnum = OBJ_VNUM_RED_ARROW;
	}
	else if (!str_prefix(arg, "white")) {
		color = sn_lookup("white arrow");
		vnum = OBJ_VNUM_WHITE_ARROW;
	}
	else if (!str_prefix(arg, "blue")) {
		color = sn_lookup("blue arrow");
		vnum = OBJ_VNUM_BLUE_ARROW;
	}

	if (vnum < 0) {
		char_puts("You don't know how to make "
			  "that kind of arrow.\n", ch);
		return;
	}

	if (color > 0) {
		color_chance = get_skill(ch, color);
		mana += SKILL(color)->min_mana;
		wait += SKILL(color)->beats;
	}

	if (ch->mana < mana) {
		char_puts("You don't have enough energy "
			  "to make that kind of arrows.\n", ch);
		return;
	}

	ch->mana -= mana;
	WAIT_STATE(ch, wait);

	char_puts("You start to make arrows!\n",ch);
	act("$n starts to make arrows!", ch, NULL, NULL, TO_ROOM);
	pObjIndex = get_obj_index(vnum);
	for(count = 0; count < LEVEL(ch) / 5; count++) {
		if (number_percent() > chance * color_chance / 100) {
			char_puts("You failed to make the arrow, "
				  "and broke it.\n", ch);
			check_improve(ch, sn, FALSE, 3);
			if (color > 0)
				check_improve(ch, color, FALSE, 3);
			continue;
		}

		check_improve(ch, sn, TRUE, 3);
		if (color > 0)
			check_improve(ch, color, TRUE, 3);

		arrow = create_obj(pObjIndex, 0);
		arrow->level = ch->level;
		arrow->value[1] = 4 + LEVEL(ch) / 10;
		arrow->value[2] = 4 + LEVEL(ch) / 10;

		af.where	 = TO_OBJECT;
		af.type		 = sn;
		af.level	 = ch->level;
		af.duration	 = -1;
		af.location	 = APPLY_HITROLL;
		af.modifier	 = LEVEL(ch) / 10;
		af.bitvector 	 = 0;
		affect_to_obj(arrow, &af);

		af.where	= TO_OBJECT;
		af.type		= sn;
		af.level	= ch->level;
		af.duration	= -1;
		af.location	= APPLY_DAMROLL;
		af.modifier	= LEVEL(ch) / 10;
		af.bitvector	= 0;
		affect_to_obj(arrow, &af);

		obj_to_char(arrow, ch);
		act_puts("You successfully make $p.",
			 ch, arrow, NULL, TO_CHAR, POS_DEAD);
	}
}

void do_make_bow(CHAR_DATA *ch, const char *argument)
{
	OBJ_DATA *	bow;
	AFFECT_DATA	af;
	int		mana;
	int		sn;
	int		chance;

	if (IS_NPC(ch))
		return;

	if ((sn = sn_lookup("make bow")) < 0
	||  (chance = get_skill(ch, sn)) == 0) {
		char_puts("You don't know how to make bows.\n", ch);
		return;
	}

	if (ch->in_room->sector_type != SECT_FIELD
	&&  ch->in_room->sector_type != SECT_FOREST
	&&  ch->in_room->sector_type != SECT_HILLS) {
		char_puts("You couldn't find enough wood.\n", ch);
		return;
	}

	mana = SKILL(sn)->min_mana;
	if (ch->mana < mana) {
		char_puts("You don't have enough energy to make a bow.\n",
			     ch);
		return;
	}
	ch->mana -= mana;
	WAIT_STATE(ch, SKILL(sn)->beats);

	if (number_percent() > chance) {
		char_puts("You failed to make the bow, and broke it.\n",
			     ch);
		check_improve(ch, sn, FALSE, 1);
		return;
	}
	check_improve(ch, sn, TRUE, 1);

	bow = create_obj(get_obj_index(OBJ_VNUM_RANGER_BOW), 0);
	bow->level = ch->level;
	bow->value[1] = 4 + ch->level / 15;
	bow->value[2] = 4 + ch->level / 15;

	af.where	= TO_OBJECT;
	af.type		= sn;
	af.level	= ch->level;
	af.duration	= -1;
	af.location	= APPLY_HITROLL;
	af.modifier	= LEVEL(ch) / 10;
	af.bitvector 	= 0;
	affect_to_obj(bow, &af);

	af.where	= TO_OBJECT;
	af.type		= sn;
	af.level	= ch->level;
	af.duration	= -1;
	af.location	= APPLY_DAMROLL;
	af.modifier	= LEVEL(ch) / 10;
	af.bitvector 	= 0;
	affect_to_obj(bow, &af);

	obj_to_char(bow, ch);
	act_puts("You successfully make $p.", ch, bow, NULL, TO_CHAR, POS_DEAD);
}

void do_make(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		char_puts("You can make either bow or arrow.\n",ch);
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
        int sn;
        int chance;
        char arg[MAX_INPUT_LENGTH];

        if ((sn = sn_lookup("homepoint")) < 0
        ||  (chance = get_skill(ch, sn)) == 0) {
                char_puts("Huh?\n", ch);
                return;
        }

        if (is_affected(ch, sn)) {
                char_puts("You fatigue for searching new home.\n", ch) ;
                return;
        }

        if (IS_SET(ch->in_room->room_flags, ROOM_SAFE | ROOM_PEACE |
                                            ROOM_PRIVATE | ROOM_SOLITARY)
        ||  (ch->in_room->sector_type != SECT_FIELD
        &&   ch->in_room->sector_type != SECT_FOREST
        &&   ch->in_room->sector_type != SECT_MOUNTAIN
        &&   ch->in_room->sector_type != SECT_HILLS)) {
                char_puts("You are cannot set home here.\n",
                             ch);
                return;
        }

        if (ch->mana < ch->max_mana) {
                char_puts("You don't have strength to make a new home.\n",
                             ch);
                return;
        }

        ch->mana = 0 ;

        if (number_percent() > chance) {
                char_puts("You failed to make your homepoint.\n", ch);
                check_improve(ch, sn, FALSE, 4);
                return;
        }

        check_improve(ch, sn, TRUE, 4);
        WAIT_STATE(ch, SKILL(sn)->beats);

        char_puts("You succeeded to make your homepoint.\n", ch);
        act("$n succeeded to make $s homepoint. ", ch, NULL, NULL, TO_ROOM);

        af.where        = TO_AFFECTS;
        af.type         = sn;
        af.level        = ch->level;
        af.duration     = 100;
        af.bitvector    = 0;
        af.modifier     = 0;
        af.location     = APPLY_NONE;
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
		if (IS_AFFECTED(ch, AFF_DETECT_EVIL)
		&&  IS_OBJ_STAT(obj, ITEM_EVIL))
			strnzcat(buf, sizeof(buf),
				 GETMSG("({DEvil Aura{x) ", GET_LANG(ch)));
		if (IS_AFFECTED(ch, AFF_DETECT_GOOD)
		&&  IS_OBJ_STAT(obj, ITEM_BLESS))
			strnzcat(buf, sizeof(buf),
				 GETMSG("({BBlue Aura{x) ", GET_LANG(ch)));
		if (IS_AFFECTED(ch, AFF_DETECT_MAGIC)
		&&  IS_OBJ_STAT(obj, ITEM_MAGIC))
			strnzcat(buf, sizeof(buf),
				 GETMSG("({MMagical{x) ", GET_LANG(ch)));
		if (IS_OBJ_STAT(obj, ITEM_GLOW))
			strnzcat(buf, sizeof(buf),
				 GETMSG("({WGlowing{x) ", GET_LANG(ch)));
		if (IS_OBJ_STAT(obj, ITEM_HUM))
			strnzcat(buf, sizeof(buf),
				 GETMSG("({YHumming{x) ", GET_LANG(ch)));
	}
	else {
		static char FLAGS[] = "{x[{y.{D.{D.{B.{M.{W.{Y.{x] ";
		strnzcpy(buf, sizeof(buf), FLAGS);
		if (IS_OBJ_STAT(obj, ITEM_INVIS)	)   buf[5] = 'I';
		if (IS_OBJ_STAT(obj, ITEM_DARK)		)   buf[8] = 'D';
		if (IS_AFFECTED(ch, AFF_DETECT_EVIL)
		&& IS_OBJ_STAT(obj, ITEM_EVIL)		)   buf[11] = 'E';
		if (IS_AFFECTED(ch, AFF_DETECT_GOOD)
		&&  IS_OBJ_STAT(obj,ITEM_BLESS)		)   buf[14] = 'B';
		if (IS_AFFECTED(ch, AFF_DETECT_MAGIC)
		&& IS_OBJ_STAT(obj, ITEM_MAGIC)		)   buf[17] = 'M';
		if (IS_OBJ_STAT(obj, ITEM_GLOW)		)   buf[20] = 'G';
		if (IS_OBJ_STAT(obj, ITEM_HUM)		)   buf[23] = 'H';
		if (strcmp(buf, FLAGS) == 0)
			buf[0] = '\0';
	}

	if (fShort) {
		strnzcat(buf, sizeof(buf),
			 format_short(&obj->short_descr, obj->name, ch));
		if (obj->pObjIndex->vnum > 5 /* not money, gold, etc */
		&&  (obj->condition < COND_EXCELLENT ||
		     !IS_SET(ch->comm, COMM_NOVERBOSE))) {
			char buf2[MAX_STRING_LENGTH];
			snprintf(buf2, sizeof(buf2), " [{g%s{x]",
				 GETMSG(get_cond_alias(obj), GET_LANG(ch)));
			strnzcat(buf, sizeof(buf), buf2);
		}
		return buf;
	}

	if (obj->in_room && IS_WATER(obj->in_room)) {
		char* p;

		p = strchr(buf, '\0');
		strnzcat(buf, sizeof(buf),
			 format_short(&obj->short_descr, obj->name, ch));
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
	output = buf_new(-1);

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
			if (prgnShow[iShow] != 1) 
				buf_printf(output, "(%2d) ", prgnShow[iShow]);
			else
				buf_add(output,"     ");
		}

		buf_add(output, prgpstrShow[iShow]);
		buf_add(output,"\n");
		free_string(prgpstrShow[iShow]);
	}

	if (fShowNothing && nShow == 0) {
		if (IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE))
			char_puts("     ", ch);
		char_puts("Nothing.\n", ch);
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
	race_t *r;
        if ((r = race_lookup(victim->race)) == NULL)
		r = race_lookup(rn_lookup("unique"));

	if (is_affected(victim, gsn_doppelganger)
	&&  (IS_NPC(ch) || !IS_SET(PC(ch)->plr_flags, PLR_HOLYLIGHT)))
		victim = victim->doppel;

	if (IS_NPC(victim)) {
		if (!IS_NPC(ch) && PC(ch)->questmob > 0
		&&  NPC(victim)->hunter == ch)
			char_puts("{r[{RTARGET{r]{x ", ch);
	}
	else {
		if (IS_WANTED(victim))
			char_puts("({RWanted{x) ", ch);

		if (IS_SET(victim->comm, COMM_AFK))
			char_puts("{c[AFK]{x ", ch);
	}

	if (IS_SET(ch->comm, COMM_LONG)) {
		if (IS_AFFECTED(victim, AFF_INVIS))
			char_puts("({yInvis{x) ", ch);
		if (IS_AFFECTED(victim, AFF_HIDE))
			char_puts("({DHidden{x) ", ch);
		if (IS_AFFECTED(victim, AFF_CHARM))
			char_puts("({mCharmed{x) ", ch);
		if (IS_AFFECTED(victim, AFF_PASS_DOOR))
			char_puts("({cTranslucent{x) ", ch);
		if (IS_AFFECTED(victim, AFF_FAERIE_FIRE))
			char_puts("({MPink Aura{x) ", ch);
		if (IS_UNDEAD(victim, r) && IS_AFFECTED(ch, AFF_DETECT_UNDEAD))
			char_puts("({DUndead{x) ", ch);
		if (RIDDEN(victim))
			char_puts("({GRidden{x) ", ch);
		if (IS_AFFECTED(victim,AFF_IMP_INVIS))
			char_puts("({bImproved{x) ", ch);
		if (IS_EVIL(victim) && IS_AFFECTED(ch, AFF_DETECT_EVIL))
			char_puts("({DEvil aura{x) ", ch);
		if (IS_GOOD(victim) && IS_AFFECTED(ch, AFF_DETECT_GOOD))
			char_puts("({WGood aura{x) ", ch);
		if (IS_AFFECTED(victim, AFF_SANCTUARY))
			char_puts("({WWhite Aura{x) ", ch);
		if (IS_AFFECTED(victim, AFF_BLACK_SHROUD))
			char_puts("({DBlack Aura{x) ", ch);
		if (is_affected(victim, gsn_golden_aura))
			char_puts("({YGolden aura{x) ", ch);
		if (IS_AFFECTED(victim, AFF_FADE))
			char_puts("({yFade{x) ", ch);
		if (IS_AFFECTED(victim, AFF_CAMOUFLAGE))
			char_puts("({gCamf{x) ", ch);
		if (IS_AFFECTED(victim, AFF_BLEND))
			char_puts("({gBlending{x) ", ch);
		if (is_affected(victim, gsn_ice_sphere))
			act_puts("({CIn ice sphere{x) ", ch, NULL, NULL, TO_CHAR | ACT_NOLF, POS_DEAD);
		if (is_affected(victim, gsn_fire_sphere))
			act_puts("({RIn fire sphere{x) ", ch, NULL, NULL, TO_CHAR | ACT_NOLF, POS_DEAD);
		if (IS_SET(ch->comm, COMM_SHOW_RACE))
			act_puts("({c$T{x) ", ch, NULL, race_name(victim->race), TO_CHAR | ACT_NOLF, POS_DEAD);
	}
	else {
		static char FLAGS[] = "{x[{y.{D.{m.{c.{M.{D.{G.{b.{x.{Y.{W.{y.{g.{g.{x.{x";
		char buf[sizeof(FLAGS)];
		bool diff;

		strnzcpy(buf, sizeof(buf), FLAGS);
		FLAG_SET( 5, 'I', IS_AFFECTED(victim, AFF_INVIS));
		FLAG_SET( 8, 'H', IS_AFFECTED(victim, AFF_HIDE));
		FLAG_SET(11, 'C', IS_AFFECTED(victim, AFF_CHARM));
		FLAG_SET(14, 'T', IS_AFFECTED(victim, AFF_PASS_DOOR));
		FLAG_SET(17, 'P', IS_AFFECTED(victim, AFF_FAERIE_FIRE));
		FLAG_SET(20, 'U', IS_UNDEAD(victim, r) &&
				  IS_AFFECTED(ch, AFF_DETECT_UNDEAD));
		FLAG_SET(23, 'R', RIDDEN(victim));
		FLAG_SET(26, 'I', IS_AFFECTED(victim, AFF_IMP_INVIS));
		if (IS_AFFECTED(ch, AFF_DETECT_EVIL) && IS_EVIL(victim)) {
			FLAG_SET(28, 'D', TRUE);
			FLAG_SET(29, 'E', TRUE);
		} else if (IS_AFFECTED(ch, AFF_DETECT_GOOD)
		    && IS_GOOD(victim)) {
			FLAG_SET(28, 'W', TRUE);
			FLAG_SET(29, 'G', TRUE);
		}
		FLAG_SET(32, 'G', is_affected(victim, gsn_golden_aura));

		if (IS_AFFECTED(victim, AFF_SANCTUARY)) {
			FLAG_SET(35, 'S', TRUE);
			FLAG_SET(34, 'W', TRUE);
		}

		if (IS_AFFECTED(victim, AFF_BLACK_SHROUD)) {
			FLAG_SET(35, 'B', TRUE);
			FLAG_SET(34, 'D', TRUE);
		}

		FLAG_SET(38, 'F', IS_AFFECTED(victim, AFF_FADE));
		FLAG_SET(41, 'C', IS_AFFECTED(victim, AFF_CAMOUFLAGE));
		FLAG_SET(44, 'B', IS_AFFECTED(victim, AFF_BLEND));
		if (is_affected(victim, gsn_ice_sphere)) {
			FLAG_SET(46, 'C', TRUE);
			FLAG_SET(47, 'I', TRUE);
		}
		if (is_affected(victim, gsn_fire_sphere)) {
			FLAG_SET(46, 'R', TRUE);
			FLAG_SET(47, 'F', TRUE);
		}

		diff = strcmp(buf, FLAGS);
		if (diff)
			char_puts(buf, ch);
		else if (IS_SET(ch->comm, COMM_SHOW_RACE))
			char_puts("{x[", ch);

		if (IS_SET(ch->comm, COMM_SHOW_RACE)) {
			act_puts("{c$T{x] ", ch, NULL, race_name(victim->race),
				 TO_CHAR | ACT_NOLF, POS_DEAD);
		} else if (diff)
			char_puts("] ", ch);
	}

	if (victim->invis_level >= LEVEL_HERO)
		char_puts("[{WWizi{x] ", ch);
	if (victim->incog_level >= LEVEL_HERO)
		char_puts("[{DIncog{x] ", ch);


	if (IS_NPC(victim)
	&&  victim->position == victim->pMobIndex->start_pos) {
		act_puts(format_long(&victim->long_descr, ch),
			 ch, NULL, NULL, TO_CHAR | ACT_NOLF, POS_DEAD);
		return;
	}

	if (IS_IMMORTAL(victim))
		char_puts("{W", ch);
	else
		char_puts("{x", ch);

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
		if (IS_SET(victim->on->value[2], SLEEP_AT))
			msg = "$N {xis sleeping at $p.";
		else if (IS_SET(victim->on->value[2], SLEEP_ON))
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
		if (IS_SET(victim->on->value[2], REST_AT))
			msg = "$N {xis resting at $p.";
		else if (IS_SET(victim->on->value[2], REST_ON))
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
		if (IS_SET(victim->on->value[2], SIT_AT))
			msg = "$N {xis sitting at $p.";
		else if (IS_SET(victim->on->value[2], SIT_ON))
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
		if (IS_SET(victim->on->value[2],STAND_AT))
			msg = "$N {xis standing at $p.";
		else if (IS_SET(victim->on->value[2],STAND_ON))
			msg = "$N {xis standing on $p.";
		else
			msg = "$N {xis standing in $p.";
		break;
	
	case POS_FIGHTING:
		if (victim->fighting == NULL) {
			arg = "thin air??";
			msg = "$N {xis here, fighting with $t.";
		}
		else if (victim->fighting == ch) {
			arg = "YOU!";
			msg = "$N {xis here, fighting with $t.";
		}
		else if (victim->in_room == victim->fighting->in_room) {
			arg = victim->fighting;
			msg = "$N {xis here, fighting with $i.";
		}
		else {
			arg = "someone who left??";
			msg = "$N {xis here, fighting with $t.";
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
	"<worn around neck>  $t",
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

static void show_obj_to_char(CHAR_DATA *ch, OBJ_DATA *obj, flag32_t wear_loc)
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

	if (is_affected(victim, gsn_doppelganger)) {
		if (IS_NPC(ch) || !IS_SET(PC(ch)->plr_flags, PLR_HOLYLIGHT)) {
			doppel = victim->doppel;
			if (is_affected(victim, gsn_mirror))
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

	if (IS_NPC(doppel))
		desc = mlstr_cval(&doppel->description, ch);
	else
		desc = mlstr_mval(&doppel->description);

	if (!IS_NULLSTR(desc)) {
		if (IS_NPC(doppel)) {
			act_puts(desc, ch, desc, NULL,
				 TO_CHAR | ACT_NOLF, POS_DEAD);
		} else {
			act_puts("$t{x", ch, desc, NULL,
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
		char_printf(ch, "(%s) ", race_name(doppel->race));
		if (!IS_NPC(doppel)) 
			char_printf(ch, "(%s) ", class_name(doppel));
		char_printf(ch, "(%s) ", flag_string(sex_table, doppel->sex));
	}

	strnzcpy(buf, sizeof(buf), PERS(victim, ch));
	buf[0] = UPPER(buf[0]);
	char_printf(ch, "%s%s%s %s\n",
		    IS_IMMORTAL(victim) ? "{W" : str_empty,
		    buf,
		    IS_IMMORTAL(victim) ? "{x" : str_empty,
		    GETMSG(msg, GET_LANG(ch)));

	found = FALSE;
	for (i = 0; show_order[i] != -1; i++)
		if ((obj = get_eq_char(mirror, show_order[i]))
		&&  can_see_obj(ch, obj)) {
			if (!found) {
				char_puts("\n", ch);
				act("$N is using:", ch, NULL, victim, TO_CHAR);
				found = TRUE;
			}

			show_obj_to_char(ch, obj, show_order[i]);
		}

	for (obj = mirror->carrying; obj; obj = obj->next_content)
		if (obj->wear_loc == WEAR_STUCK_IN
		&&  can_see_obj(ch, obj)) {
			if (!found) {
				char_puts("\n", ch);
				act("$N is using:", ch, NULL, victim, TO_CHAR);
				found = TRUE;
			}

			show_obj_to_char(ch, obj, WEAR_STUCK_IN);
		}

	if (victim != ch
	&&  (!IS_IMMORTAL(victim) || IS_IMMORTAL(ch))
	&&  !IS_NPC(ch)
	&&  number_percent() < get_skill(ch, gsn_peek)) {
		char_puts("\nYou peek at the inventory:\n", ch);
		check_improve(ch, gsn_peek, TRUE, 4);
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
			if (room_is_dark(ch) && IS_AFFECTED(rch, AFF_INFRARED))
				char_puts("You see {rglowing red eyes{x watching YOU!\n", ch);
			life_count++;
		}
	}

	if (list && list->in_room == ch->in_room
	&&  life_count
	&&  IS_AFFECTED(ch, AFF_DETECT_LIFE))
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
 
	col = 0;
	for (i = 0; i < commands.nused; i++) {
		cmd_t *cmd = VARR_GET(&commands, i);

		if (cmd->min_level < LEVEL_HERO
		&&  cmd->min_level <= ch->level 
		&&  !IS_SET(cmd->cmd_flags, CMD_HIDDEN)) {
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
	int i;
	int col;
 
	if (IS_NPC(ch)) {
		char_puts("Huh?\n", ch);
		return;
	}

	col = 0;
	for (i = 0; i < commands.nused; i++) {
		cmd_t *cmd = VARR_GET(&commands, i);

		if (cmd->min_level < LEVEL_IMMORTAL)
			continue;

		if (ch->level < LEVEL_IMP
		&&  !is_name(cmd->name, PC(ch)->granted))
			continue;

		char_printf(ch, "%-12s", cmd->name);
		if (++col % 6 == 0)
			char_puts("\n", ch);
	}
 
	if (col % 6 != 0)
		char_puts("\n", ch);
}

static void
show_clanlist(CHAR_DATA *ch, clan_t *clan,
	      const char *list, const char *name_list)
{
	BUFFER *output;
	char name[MAX_STRING_LENGTH];
	int cnt = 0;
	CHAR_DATA *vch;

	output = buf_new(-1);
	buf_printf(output, "List of %s of %s:\n", name_list, clan->name);
	buf_add(output, "Status   Level Race  Class   Ethos-align   Name\n");
	buf_add(output, "-------- ----- ----- ----- --------------- -------------\n");

	list = first_arg(list, name, sizeof(name), FALSE);
	for (; name[0]; list = first_arg(list, name, sizeof(name), FALSE)) {
		race_t *r;
		class_t *cl;

		if ((vch = char_load(name, LOAD_F_NOCREATE)) == NULL) {
			buf_printf(output, "[{RInvalid entry{x] %s (report this to immortals)\n", name);
			continue;
		}

		if (str_cmp(clan_name(vch->clan), clan->name)) {
			buf_printf(output, "[{RInvalid entry{x] %s (report this to immortals)\n", vch->name);
			char_nuke(vch);
			continue;
		}

		cnt++;

		r = race_lookup(vch->race);
		cl = class_lookup(vch->class);
		buf_printf(output, "%-8s  %3d  %-5s  %-3s  %7s-%-7s %s\n",
			flag_string(clan_status_table, PC(vch)->clan_status),
			vch->level,
			r && r->race_pcdata ? r->race_pcdata->who_name : "none",
			cl ? cl->who_name : "none",
			flag_string(ethos_table, vch->ethos),
			flag_string(align_names, NALIGN(vch)),
			vch->name);
		char_nuke(vch);
	}

	if (!cnt)
		buf_add(output, "None.\n");

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

	if (IS_IMMORTAL(ch) && arg2[0]) {
		int cln;

		if ((cln = cln_lookup(arg2)) < 0) {
			char_printf(ch, "%s: no such clan.\n", arg2);
			return;
		}
		clan = CLAN(cln);
	}

	if (!clan
	&&  (!ch->clan || (clan = clan_lookup(ch->clan)) == NULL)) {
		char_puts("You are not in a clan.\n", ch);
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

void do_item(CHAR_DATA* ch, const char* argument)
{
	clan_t* clan = NULL;
	OBJ_DATA* in_obj;
	int cln;
	char arg[MAX_STRING_LENGTH];

	one_argument(argument, arg, sizeof(arg));
	if (IS_IMMORTAL(ch) && arg[0]) {
		if ((cln = cln_lookup(arg)) < 0) {
			char_printf(ch, "%s: no such clan.\n", arg);
			return;
		}
		clan = CLAN(cln);
	}

	if (!clan
	&&  (!ch->clan || (clan = clan_lookup(ch->clan)) == NULL)) {
		char_puts("You are not in clan, you should not worry about your clan item.\n", ch);
		return;
	}

	if (clan->obj_ptr == NULL) {
		char_puts("Your clan do not have an item of power.\n",ch);
		return;
	}

	for (in_obj = clan->obj_ptr; in_obj->in_obj; in_obj = in_obj->in_obj)
		;

	if (in_obj->carried_by) {
		act_puts3("$p is in $R, carried by $N.",
			  ch, clan->obj_ptr, in_obj->carried_by,
			  in_obj->carried_by->in_room,
			  TO_CHAR, POS_DEAD);
	}
	else if (in_obj->in_room) {
		act_puts3("$p is in $R.",
			  ch, clan->obj_ptr, NULL, in_obj->in_room,
			  TO_CHAR, POS_DEAD);
		for (cln = 0; cln < clans.nused; cln++) 
			if (in_obj->in_room->vnum == CLAN(cln)->altar_vnum) {
				act_puts("It is altar of $t.",
					 ch, CLAN(cln)->name, NULL,
					 TO_CHAR, POS_DEAD);
			}
	}
	else 
		act_puts("$p is somewhere.",
			 ch, clan->obj_ptr, NULL, TO_CHAR, POS_DEAD);
}

void do_rating(CHAR_DATA *ch, const char *argument)
{
	int i;

	char_puts("Name                    | PC's killed\n", ch);
	char_puts("------------------------+------------\n", ch);
	for (i = 0; i < RATING_TABLE_SIZE; i++) {
		if (rating_table[i].name == NULL)
			continue;
		char_printf(ch, "%-24s| %d\n",
			    rating_table[i].name, rating_table[i].pc_killed);
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
		char_puts("No argument is used with this command.\n",ch);
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
	output = buf_new(-1);
	buf_add(output, "Current areas of Shades of Gray: \n");
	for (iArea = 0; iArea < iAreaHalf; iArea++) {
		SKIP_CLOSED(pArea1);
		SKIP_CLOSED(pArea2);

		buf_printf(output,"{{%2d %3d} {B%-20.20s{x %8.8s ",
			   pArea1->min_level, pArea1->max_level,
			   pArea1->name,
			   pArea1->credits);

		if (pArea2 != NULL) 
			buf_printf(output,"{{%2d %3d} {B%-20.20s{x %8.8s",
				pArea2->min_level, pArea2->max_level,
				pArea2->name,
				pArea2->credits);
		buf_add(output, "\n");

		pArea1 = pArea1->next;
		if (pArea2 != NULL)
			pArea2 = pArea2->next;
	}

	buf_printf(output, "\n%d areas total.\n", maxArea);
	page_to_char(buf_string(output), ch);	
	buf_free(output);
}

void do_acute_vision(CHAR_DATA *ch, const char *argument)
{
	AFFECT_DATA	af;
	int		chance;
	int		sn;

	if ((sn = sn_lookup("acute eye")) < 0
	||  (chance = get_skill(ch, sn)) == 0) {
		char_puts("Huh?\n", ch);
		return;
	}
	if (IS_AFFECTED(ch, AFF_ACUTE_VISION)) {
			char_puts("Your vision is already acute. \n",ch);
			return;
	}


	if (number_percent() > chance) {
		char_puts("You peer intently through leaf "
			     "but they are unrevealing.\n", ch);
		check_improve(ch, sn, FALSE, 1);
		return;
	}

	af.where	= TO_AFFECTS;
	af.type		= sn;
	af.level	= LEVEL(ch);;
	af.duration	= 3 + LEVEL(ch) / 5;
	af.location	= APPLY_NONE;
	af.modifier	= 0;
	af.bitvector	= AFF_ACUTE_VISION;
	affect_to_char(ch, &af);
	char_puts("Your vision sharpens.\n", ch);
	check_improve(ch, sn, TRUE, 1);
}

