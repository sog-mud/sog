/*
 * $Id: skills.c,v 1.39 1998-11-14 09:01:12 fjoe Exp $
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
#include "merc.h"
#include "update.h"

#include "resource.h"

varr skills = { sizeof(SKILL_DATA), 8 };

/* command procedures needed */
DECLARE_DO_FUN(do_help		);
DECLARE_DO_FUN(do_say		);

int	ch_skill_nok	(CHAR_DATA *ch , int sn);

/* used to converter of prac and train */
void do_gain(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *tr;

	if (IS_NPC(ch))
		return;

	/* find a trainer */
	for (tr = ch->in_room->people; tr; tr = tr->next_in_room)
		if (IS_NPC(tr)
		&&  (IS_SET(tr->act,ACT_PRACTICE) ||
		     IS_SET(tr->act,ACT_TRAIN) ||
		     IS_SET(tr->act,ACT_GAIN)))
			break;

	if (tr == NULL || !can_see(ch, tr)) {
		char_puts("You can't do that here.\n\r",ch);
		return;
	}

	one_argument(argument, arg);

	if (arg[0] == '\0') {
		do_say(tr, "You may convert 10 practices into 1 train.");
		do_say(tr, "You may revert 1 train into 10 practices.");
		do_say(tr, "Simply type 'gain convert' or 'gain revert'.");
		return;
	}

	if (!str_prefix(arg, "revert")) {
		if (ch->train < 1) {
			do_tell_raw(tr, ch, "You are not yet ready.");
			return;
		}

		act("$N helps you apply your training to practice",
		    ch, NULL, tr, TO_CHAR);
		ch->practice += 10;
		ch->train -=1 ;
		return;
	}

	if (!str_prefix(arg, "convert")) {
		if (ch->practice < 10) {
			do_tell_raw(tr, ch, "You are not yet ready.");
			return;
		}

		act("$N helps you apply your practice to training",
		    ch, NULL, tr, TO_CHAR);
		ch->practice -= 10;
		ch->train +=1 ;
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
	
	for (i = 0; i < ch->pcdata->learned.nused; i++) {
		PC_SKILL *ps = VARR_GET(&ch->pcdata->learned, i);
		SKILL_DATA *sk;

		if (ps->percent == 0
		||  (sk = skill_lookup(ps->sn)) == NULL
		||  sk->spell_fun == NULL)
			continue;

		found = TRUE;
		lev = skill_level(ch, ps->sn);

		if (lev > (IS_IMMORTAL(ch) ? LEVEL_IMMORTAL : LEVEL_HERO))
			continue;

		snprintf(buf, sizeof(buf),
			 ch->level < lev ?
				"%-18s  n/a      " : "%-18s  %3d mana  ",
			 sk->name, mana_cost(ch, ps->sn));
			
		if (spell_list[lev][0] == '\0')
			snprintf(spell_list[lev], sizeof(spell_list[lev]),
				 "\n\rLevel %2d: %s", lev, buf);
		else { /* append */
			if (++spell_columns[lev] % 2 == 0)
				strnzcat(spell_list[lev], "\n\r          ",
					 sizeof(spell_list[lev]));
			strnzcat(spell_list[lev], buf, sizeof(spell_list[lev]));
		}
	}

	/* return results */
	
	if (!found) {
		char_puts("You know no spells.\n\r",ch);
		return;
	}
	
	output = buf_new(-1);
	for (lev = 0; lev <= LEVEL_IMMORTAL; lev++)
		if (spell_list[lev][0] != '\0')
			buf_add(output, spell_list[lev]);
	buf_add(output, "\n\r");
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
	
	for (i = 0; i < ch->pcdata->learned.nused; i++) {
		PC_SKILL *ps = VARR_GET(&ch->pcdata->learned, i);
		SKILL_DATA *sk;

		if (ps->percent == 0
		||  (sk = skill_lookup(ps->sn)) == NULL
		||  sk->spell_fun)
			continue;

		found = TRUE;
		lev = skill_level(ch, ps->sn);

		if (lev > (IS_IMMORTAL(ch) ? LEVEL_IMMORTAL : LEVEL_HERO))
			continue;

		snprintf(buf, sizeof(buf),
			 ch ->level < lev ?
				"%-18s n/a      " : "%-18s %3d%%      ",
			 sk->name, ps->percent);

		if (skill_list[lev][0] == '\0')
			snprintf(skill_list[lev], sizeof(skill_list[lev]),
				 "\n\rLevel %2d: %s", lev, buf);
		else { /* append */
			if (++skill_columns[lev] % 2 == 0)
				strnzcat(skill_list[lev], "\n\r          ",
					 sizeof(skill_list[lev]));
			strnzcat(skill_list[lev], buf, sizeof(skill_list[lev]));
		}
	}
	
	/* return results */
	
	if (!found) {
		char_puts("You know no skills.\n\r",ch);
		return;
	}
	
	output = buf_new(-1);
	for (lev = 0; lev <= LEVEL_IMMORTAL; lev++)
		if (skill_list[lev][0] != '\0')
			buf_add(output, skill_list[lev]);
	buf_add(output, "\n\r");
	page_to_char(buf_string(output), ch);
	buf_free(output);
}

int base_exp(CHAR_DATA *ch)
{
	int expl;
	CLASS_DATA *cl;
	RACE_DATA *r;
	RACE_CLASS_DATA *rcl;

	if (IS_NPC(ch)
	||  (cl = class_lookup(ch->class)) == NULL
	||  (r = race_lookup(ch->pcdata->race)) == NULL
	||  !r->pcdata
	||  (rcl = race_class_lookup(r, cl->name)) == NULL)
		return 1500;

	expl = 1000 + r->pcdata->points + cl->points;
	return expl * rcl->mult/100;
}

int exp_for_level(CHAR_DATA *ch, int level)
{
	int i = base_exp(ch) * level;
	return i + i * (level-1) / 20;
}

int exp_to_level(CHAR_DATA *ch)
{ 
	return exp_for_level(ch, ch->level+1) - ch->exp;
}

/* checks for skill improvement */
void check_improve(CHAR_DATA *ch, int sn, bool success, int multiplier)
{
	PC_SKILL *ps;
	CLASS_DATA *cl;
	CLASS_SKILL *cs;
	int chance;
	int rating;

	if (IS_NPC(ch)
	||  (cl = class_lookup(ch->class)) == NULL
	||  (ps = pc_skill_lookup(ch, sn)) == NULL
	||  ps->percent == 0 || ps->percent == 100
	||  skill_level(ch, sn) > ch->level)
		return;

	if ((cs = class_skill_lookup(cl, sn)))
		rating = cs->rating;
	else
		rating = 1;

	/* check to see if the character has a chance to learn */
	chance = 10 * int_app[get_curr_stat(ch,STAT_INT)].learn;
	chance /= (multiplier *	rating * 4);
	chance += ch->level;

	if (number_range(1, 1000) > chance)
		return;

/* now that the character has a CHANCE to learn, see if they really have */	

	if (success) {
		chance = URANGE(5, 100 - ps->percent, 95);
		if (number_percent() < chance) {
			act_nprintf(ch, NULL, NULL, TO_CHAR, POS_DEAD,
				    MSG_HAVE_BECOME_BETTER, skill_name(sn));
			ps->percent++;
			gain_exp(ch, 2 * rating);
		}
	}
	else {
		chance = URANGE(5, ps->percent / 2, 30);
		if (number_percent() < chance) {
			act_nprintf(ch, NULL, NULL, TO_CHAR, POS_DEAD,
				    MSG_LEARN_FROM_MISTAKES, skill_name(sn));
			if ((ps->percent += number_range(1, 3)) > 100)
				ps->percent = 100;
			gain_exp(ch, 2 * rating);
		}
	}
}

/*
 * simply adds sn to ch's known skills (if skill is not already known).
 */
void set_skill_raw(CHAR_DATA *ch, int sn, int percent, bool replace)
{
	PC_SKILL *ps;

	if (sn <= 0)
		return;

	if ((ps = pc_skill_lookup(ch, sn))) {
		if (replace || ps->percent < percent)
			ps->percent = percent;
		return;
	}
	ps = varr_enew(&ch->pcdata->learned);
	ps->sn = sn;
	ps->percent = percent;
	varr_qsort(&ch->pcdata->learned, cmpint);
}

/* use for adding/updating all skills available for that ch  */
void update_skills(CHAR_DATA *ch)
{
	int i;
	CLASS_DATA *cl;
	RACE_DATA *r;
	CLAN_DATA *clan;
	const char *p;

/* NPCs do not have skills */
	if (IS_NPC(ch)
	||  (cl = class_lookup(ch->class)) == NULL
	||  (r = race_lookup(ch->race)) == NULL
	||  !r->pcdata)
		return;

/* add class skills */
	for (i = 0; i < cl->skills.nused; i++) {
		CLASS_SKILL *cs = VARR_GET(&cl->skills, i);
		set_skill_raw(ch, cs->sn, 1, FALSE);
	}

/* add race skills */
	for (i = 0; i < r->pcdata->skills.nused; i++) {
		RACE_SKILL *rs = VARR_GET(&r->pcdata->skills, i);
		set_skill_raw(ch, rs->sn, 100, FALSE);
	}

	if ((p = r->pcdata->bonus_skills))
		for (;;) {
			int sn;
			char name[MAX_STRING_LENGTH];

			p = one_argument(p, name);
			if (name[0] == '\0')
				break;
		
			sn = sn_lookup(name);
			if (sn < 0)
				continue;

			set_skill_raw(ch, sn, 100, FALSE);
		}

/* add clan skills */
	if ((clan = clan_lookup(ch->clan))) {
		for (i = 0; i < clan->skills.nused; i++) {
			CLAN_SKILL *cs = VARR_GET(&clan->skills, i);
			set_skill_raw(ch, cs->sn, 1, FALSE);
		}
	}

/* remove not matched skills */
	for (i = 0; i < ch->pcdata->learned.nused; i++) {
		PC_SKILL *ps = VARR_GET(&ch->pcdata->learned, i);
		if (skill_level(ch, ps->sn) > LEVEL_HERO && !IS_IMMORTAL(ch))
			ps->percent = 0;
	}
}

void set_skill(CHAR_DATA *ch, int sn, int percent)
{
	set_skill_raw(ch, sn, percent, TRUE);
}

void do_glist(CHAR_DATA *ch , const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	int col = 0;
	flag_t group;
	int sn;

	one_argument(argument, arg);
	
	if (arg[0] == '\0') {
		char_puts("Syntax: glist group\n\r"
			  "Use 'glist ?' to get the list of groups.\n\r", ch);
		return;
	}

	if (!str_cmp(arg, "?")) {
		show_flags(ch, skill_groups);
		return;
	}

	if ((group = flag_value(skill_groups, arg)) < 0) {
		char_puts("That is not a valid group.\n\r", ch);
		return;
	}

	char_printf(ch, "Now listing group '%s':\n\r",
		    flag_string(skill_groups, group));

	for (sn = 0; sn < skills.nused; sn++) {
		SKILL_DATA *sk = VARR_GET(&skills, sn);
		if (group == sk->group) {
			char_printf(ch, "%c%-18s",
				    pc_skill_lookup(ch, sn) ? '*' : ' ',
				    sk->name);
			if (col)
				char_puts("\n\r", ch);
			col = 1 - col;
		}
	}

	if (col)
		char_puts("\n\r", ch);
}

void do_slook(CHAR_DATA *ch, const char *argument)
{
	int sn = -1;
	char arg[MAX_INPUT_LENGTH];

	one_argument(argument,arg);
	if (arg[0] == '\0') {
		char_puts("Syntax : slook <skill | spell>\n\r",ch);
		return;
	}

	if (!IS_NPC(ch)) {
		PC_SKILL *ps;
		if ((ps = skill_vlookup(&ch->pcdata->learned, arg)))
			sn = ps->sn;
	}

	if (sn < 0 && (sn = sn_lookup(arg)) < 0) { 
		char_puts("That is not a spell or skill.\n\r",ch);
		return; 
	}

	char_printf(ch, "Skill '%s' in group '%s'.\n\r",
		    SKILL(sn)->name,
		    flag_string(skill_groups, SKILL(sn)->group));
}

#define PC_PRACTICER	123

void do_learn(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	int sn;
	CHAR_DATA *mob;
	int adept;
	CLASS_DATA *cl;
	CLASS_SKILL *cs;
	PC_SKILL *ps;
	SKILL_DATA *sk;
	int rating;

	if (IS_NPC(ch) || (cl = class_lookup(ch->class)) == NULL)
		return;

	if (!IS_AWAKE(ch)) {
		char_puts("In your dreams, or what?\n\r", ch);
		return;
	}	

	if (argument[0] == '\0') {
		char_puts("Syntax: learn <skill | spell> <player>\n\r", ch);
		return;
	}

	if (ch->practice <= 0) {
		char_puts("You have no practice sessions left.\n\r", ch);
		return;
	}

	argument = one_argument(argument,arg);

	if ((ps = skill_vlookup(&ch->pcdata->learned, arg)) == NULL
	||  ps->percent == 0
	||  skill_level(ch, sn = ps->sn) > ch->level) {
		char_puts("You can't learn that.\n\r", ch);
		return;
	}

	ps = pc_skill_lookup(ch, sn);

	if (sn == gsn_vampire) {
		char_puts("You can't practice that, only available "
			  "at questor.\n\r", ch);
		return;
	}	

	argument = one_argument(argument,arg);
		
	if ((mob = get_char_room(ch,arg)) == NULL) {
		char_puts("Your hero is not here.\n\r", ch);
		return;
	}
			
	if (IS_NPC(mob) || mob->level != HERO) {
		char_puts("You must find a hero, not an ordinary one.\n\r",
			  ch);
		return;
	}

	if (mob->status != PC_PRACTICER) {
		char_puts("Your hero doesn't want to teach you anything.\n\r",ch);
		return;
	}

	if (get_skill(mob, sn) < 100) {
		char_puts("Your hero doesn't know that skill enough to teach you.\n\r",ch);
		return;
	}

	sk = SKILL(sn);
	adept = cl->skill_adept;

	if (ps->percent >= adept) {
		char_printf(ch, "You are already learned at %s.\n\r",
			    sk->name);
		return;
	}

	ch->practice--;

	cs = class_skill_lookup(cl, sn);
	rating = cs ? UMAX(cs->rating, 1) : 1;
	ps->percent += int_app[get_curr_stat(ch,STAT_INT)].learn / rating;

	act("You teach $T.", mob, NULL, sk->name, TO_CHAR);
	act("$n teaches $T.", mob, NULL, sk->name, TO_ROOM);
	mob->status = 0;

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
		char_puts("You must be a hero.\n\r",ch);
		return;
	}
	ch->status = PC_PRACTICER;
	char_puts("Now, you can teach youngsters your 100% skills.\n\r",ch);
}

const char *skill_name(int sn)
{
	SKILL_DATA *sk = varr_get(&skills, sn);
	if (sk)
		return sk->name;
	return "none";
}

/* for returning skill information */
int get_skill(CHAR_DATA *ch, int sn)
{
	int skill;
	SKILL_DATA *sk;

	if ((sk = skill_lookup(sn)) == NULL)
		return 0;

	if (!IS_NPC(ch)) {
		PC_SKILL *ps;

		if ((ps = pc_skill_lookup(ch, sn)) == NULL
		||  skill_level(ch, sn) > ch->level)
			skill = 0;
		else
			skill = ps->percent;
	}
	else {
		/* mobiles */
		if (sk->spell_fun)
			skill = 40 + 2 * ch->level;
		else if (sn == gsn_track)
			skill = 100;
		else if ((sn == gsn_sneak || sn == gsn_hide ||
			  sn == gsn_pick || sn == gsn_backstab) &&
			 IS_SET(ch->act, ACT_THIEF))
			skill = ch->level * 2 + 20;
		else if ((sn == gsn_dodge && IS_SET(ch->off_flags,OFF_DODGE)) ||
 		         (sn == gsn_parry && IS_SET(ch->off_flags,OFF_PARRY)) ||
			 (sn == gsn_dirt && IS_SET(ch->off_flags, OFF_DIRT_KICK)))
			skill = ch->level * 2;
 		else if (sn == gsn_shield_block)
			skill = 10 + 2 * ch->level;
		else if (sn == gsn_second_attack &&
			 (IS_SET(ch->act, ACT_WARRIOR) ||
			  IS_SET(ch->act, ACT_THIEF)))
			skill = 10 + 3 * ch->level;
		else if (sn == gsn_third_attack && IS_SET(ch->act,ACT_WARRIOR))
			skill = 4 * ch->level - 40;
		else if (sn == gsn_fourth_attack && IS_SET(ch->act,ACT_WARRIOR))
			skill = 4 * ch->level - 60;
		else if (sn == gsn_hand_to_hand)
			skill = 40 + 2 * ch->level;
 		else if (sn == gsn_trip && IS_SET(ch->off_flags,OFF_TRIP)) 
			skill = 10 + 3 * ch->level;
 		else if ((sn == gsn_bash || sn == gsn_bash_door) &&
			 IS_SET(ch->off_flags,OFF_BASH))
			skill = 10 + 3 * ch->level;
		else if ((sn == gsn_critical) && IS_SET(ch->act, ACT_WARRIOR))
			skill = ch->level;
		else if (sn == gsn_disarm &&
			 (IS_SET(ch->off_flags,OFF_DISARM) ||
			  IS_SET(ch->act,ACT_WARRIOR) ||
			  IS_SET(ch->act,ACT_THIEF)))
			skill = 20 + 3 * ch->level;
		else if (sn == gsn_grip &&
			 (IS_SET(ch->act,ACT_WARRIOR) ||
			  IS_SET(ch->act,ACT_THIEF)))
			skill = ch->level;
		else if ((sn == gsn_berserk || sn == gsn_tiger_power) &&
			 IS_SET(ch->off_flags, OFF_BERSERK))
			skill = 3 * ch->level;
		else if (sn == gsn_kick)
			skill = 10 + 3 * ch->level;
		else if (sn == gsn_rescue)
			skill = 40 + ch->level; 
		else if (sn == gsn_sword || sn == gsn_dagger ||
			 sn == gsn_spear || sn == gsn_mace ||
			 sn == gsn_axe || sn == gsn_flail ||
			 sn == gsn_whip || sn == gsn_polearm ||
			 sn == gsn_bow || sn == gsn_arrow || sn == gsn_lance)
			skill = 40 + 5 * ch->level / 2;
		else if (sn == gsn_crush && IS_SET(ch->off_flags, OFF_CRUSH))
			skill = 10 + 3 * ch->level;
		else 
			skill = 0;
	}

	if (ch->daze > 0) {
		if (sk->spell_fun)
			skill /= 2;
		else
			skill = 2 * skill / 3;
	}

	if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK]  > 10)
		skill = 9 * skill / 10;

	return URANGE(0, skill, 100);
}


/*
 * Lookup a skill by name.
 */
int sn_lookup(const char *name)
{
	int sn;

	if (IS_NULLSTR(name))
		return -1;

	for (sn = 0; sn < skills.nused; sn++)
		if (LOWER(name[0]) == LOWER(SKILL(sn)->name[0])
		&&  !str_prefix(name, SKILL(sn)->name))
			return sn;

	return -1;
}

int char_sn_lookup(CHAR_DATA *ch, const char *name)
{
	int i;

	if (IS_NULLSTR(name) || IS_NPC(ch))
		return -1;

	for (i = 0; i < ch->pcdata->learned.nused; i++) {
		PC_SKILL *ps = VARR_GET(&ch->pcdata->learned, i);
		SKILL_DATA *skill;

		if (ps->percent == 0
		||  (skill = skill_lookup(ps->sn)) == NULL
		||  skill_level(ch, ps->sn) > ch->level)
			continue;

		if (!str_prefix(name, skill->name))
			return ps->sn;
	}

	return -1;
}

/* for returning weapon information */
int get_weapon_sn(OBJ_DATA *wield)
{
	int sn;

	if (wield == NULL)
		return gsn_hand_to_hand;

	if (wield->pIndexData->item_type != ITEM_WEAPON)
		return 0;

	switch (wield->value[0]) {
	default :               sn = -1;		break;
	case(WEAPON_SWORD):     sn = gsn_sword;		break;
	case(WEAPON_DAGGER):    sn = gsn_dagger;	break;
	case(WEAPON_SPEAR):     sn = gsn_spear;		break;
	case(WEAPON_MACE):      sn = gsn_mace;		break;
	case(WEAPON_AXE):       sn = gsn_axe;		break;
	case(WEAPON_FLAIL):     sn = gsn_flail;		break;
	case(WEAPON_WHIP):      sn = gsn_whip;		break;
	case(WEAPON_POLEARM):   sn = gsn_polearm;	break;
	case(WEAPON_BOW):	sn = gsn_bow;		break;
	case(WEAPON_ARROW):	sn = gsn_arrow;		break;
	case(WEAPON_LANCE):	sn = gsn_lance;		break;
	}
	return sn;
}

int get_weapon_skill(CHAR_DATA *ch, int sn)
{
	 int sk;

/* -1 is exotic */
	if (sn == -1)
		sk = 3 * ch->level;
	else if (!IS_NPC(ch))
		sk = get_skill(ch, sn);
	else if (sn == gsn_hand_to_hand)
		sk = 40 + 2 * ch->level;
	else 
		sk = 40 + 5 * ch->level / 2;

	return URANGE(0, sk, 100);
} 

/*
 * Utter mystical words for an sn.
 */
void say_spell(CHAR_DATA *ch, int sn)
{
	char buf  [MAX_STRING_LENGTH];
	CHAR_DATA *rch;
	const char *pName;
	int iSyl;
	int length;
	int skill;

	struct syl_type
	{
		char *	old;
		char *	new;
	};

	static const struct syl_type syl_table[] =
	{
		{ " ",		" "		},
		{ "ar",		"abra"		},
		{ "au",		"kada"		},
		{ "bless",	"fido"		},
		{ "blind",	"nose"		},
		{ "bur",	"mosa"		},
		{ "cu",		"judi"		},
		{ "de",		"oculo"		},
		{ "en",		"unso"		},
		{ "light",	"dies"		},
		{ "lo",		"hi"		},
		{ "mor",	"zak"		},
		{ "move",	"sido"		},
		{ "ness",	"lacri"		},
		{ "ning",	"illa"		},
		{ "per",	"duda"		},
		{ "ra",		"gru"		},
		{ "fresh",	"ima"		},
		{ "re",		"candus"	},
		{ "son",	"sabru"		},
		{ "tect",	"infra"		},
		{ "tri",	"cula"		},
		{ "ven",	"nofo"		},
		{ "a", "a" }, { "b", "b" }, { "c", "q" }, { "d", "e" },
		{ "e", "z" }, { "f", "y" }, { "g", "o" }, { "h", "p" },
		{ "i", "u" }, { "j", "y" }, { "k", "t" }, { "l", "r" },
		{ "m", "w" }, { "n", "i" }, { "o", "a" }, { "p", "s" },
		{ "q", "d" }, { "r", "f" }, { "s", "g" }, { "t", "h" },
		{ "u", "j" }, { "v", "z" }, { "w", "x" }, { "x", "n" },
		{ "y", "l" }, { "z", "k" },
		{ str_empty, str_empty }
	};

	buf[0]	= '\0';
	for (pName = skill_name(sn); *pName != '\0'; pName += length) {
		for (iSyl = 0; (length = strlen(syl_table[iSyl].old)); iSyl++) {
			if (!str_prefix(syl_table[iSyl].old, pName)) {
				strcat(buf, syl_table[iSyl].new);
				break;
			}
		}
		if (length == 0)
			length = 1;
	}

	for (rch = ch->in_room->people; rch; rch = rch->next_in_room) {
		if (rch == ch)
			continue;

		skill = (get_skill(rch, gsn_spell_craft) * 9) / 10;
		if (skill < number_percent()) {
			act("$n utters the words, '$t'.", ch, buf, rch, TO_VICT);
			check_improve(rch, gsn_spell_craft, FALSE, 5);
		}
		else  {
			act("$n utters the words, '$t'.",
			    ch, skill_name(sn), rch, TO_VICT);
			check_improve(rch, gsn_spell_craft, TRUE, 5);
		}
	}
}

/* find min level of the skill for char */
int skill_level(CHAR_DATA *ch, int sn)
{
	int slevel;
	SKILL_DATA *sk;
	CLAN_DATA *clan;
	CLAN_SKILL *clan_skill;
	CLASS_DATA *cl;
	CLASS_SKILL *class_skill;
	RACE_DATA *r;
	RACE_SKILL *race_skill;

	if (IS_NPC(ch))
		return ch->level;

	slevel = LEVEL_IMMORTAL;

/* noone can use ill-defined skills */
/* broken chars can't use any skills */
	if ((sk = skill_lookup(sn)) == NULL
	||  (cl = class_lookup(ch->class)) == NULL
	||  (r = race_lookup(ch->race)) == NULL
	||  !r->pcdata)
		return slevel;

	if ((clan = clan_lookup(ch->clan))
	&&  (clan_skill = clan_skill_lookup(clan, sn)))
		slevel = UMIN(slevel, clan_skill->level);

	if ((class_skill = class_skill_lookup(cl, sn))) {
		slevel = UMIN(slevel, class_skill->level);
		if (is_name(sk->name, r->pcdata->bonus_skills))
			slevel = UMIN(slevel, 1);
	}

	if ((race_skill = race_skill_lookup(r, sn)))
		slevel = UMIN(slevel, race_skill->level);

	return slevel;
}

int mana_cost(CHAR_DATA *ch, int sn)
{
	SKILL_DATA *sk;

	if ((sk = skill_lookup(sn)) == NULL)
		return 0;

	return UMAX(sk->min_mana, 100 / (2 + ch->level - skill_level(ch, sn)));
}

void *skill_vlookup(varr *v, const char *name)
{
	int i;

	if (IS_NULLSTR(name))
		return NULL;

	for (i = 0; i < v->nused; i++) {
		SKILL_DATA *skill;
		int *psn = (int*) VARR_GET(v, i);

		if ((skill = skill_lookup(*psn))
		&&  !str_prefix(name, skill->name))
			return psn;
	}

	return NULL;
}
