/*
 * $Id: skills.c,v 1.17 1998-08-15 12:40:49 fjoe Exp $
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
#include "magic.h"
#include "db.h"
#include "comm.h"
#include "resource.h"
#include "update.h"
#include "lookup.h"
#include "tables.h"

/* command procedures needed */
DECLARE_DO_FUN(do_help		);
DECLARE_DO_FUN(do_say		);

/* used to converter of prac and train */
void do_gain(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *trainer;

	if (IS_NPC(ch))
			return;

	/* find a trainer */
	for (trainer = ch->in_room->people; 
			  trainer != NULL; 
			  trainer = trainer->next_in_room)
			if (IS_NPC(trainer) && 	(IS_SET(trainer->act,ACT_PRACTICE) ||
				IS_SET(trainer->act,ACT_TRAIN) || IS_SET(trainer->act,ACT_GAIN)))
				break;

	if (trainer == NULL || !can_see(ch,trainer))
	{
			send_to_char("You can't do that here.\n\r",ch);
			return;
	}

	one_argument(argument,arg);

	if (arg[0] == '\0')
	{
			do_say(trainer,"You may convert 10 practices into 1 train.");
			do_say(trainer,"You may revert 1 train into 10 practices.");
			do_say(trainer,"Simply type 'gain convert' or 'gain revert'.");
			return;
	}

	if (!str_prefix(arg,"revert"))
	{
			if (ch->train < 1)
			{
				act("$N tells you 'You are not yet ready.'",
					ch,NULL,trainer,TO_CHAR);
				return;
			}

			act("$N helps you apply your training to practice",
					ch,NULL,trainer,TO_CHAR);
			ch->practice += 10;
			ch->train -=1 ;
			return;
	}

	if (!str_prefix(arg,"convert"))
	{
			if (ch->practice < 10)
			{
				act("$N tells you 'You are not yet ready.'",
					ch,NULL,trainer,TO_CHAR);
				return;
			}

			act("$N helps you apply your practice to training",
					ch,NULL,trainer,TO_CHAR);
			ch->practice -= 10;
			ch->train +=1 ;
			return;
	}

	act("$N tells you 'I do not understand...'",ch,NULL,trainer,TO_CHAR);

}


/* RT spells and skills show the players spells (or skills) */

void do_spells(CHAR_DATA *ch, const char *argument)
{
	char spell_list[LEVEL_HERO][MAX_STRING_LENGTH];
	char spell_columns[LEVEL_HERO];
	int sn,lev,mana;
	bool found = FALSE;
	char buf[MAX_STRING_LENGTH];
	char output[4*MAX_STRING_LENGTH];

	if (IS_NPC(ch))
		return;
	
	/* initialize data */
	output[0] = '\0';
	for (lev = 0; lev < LEVEL_HERO; lev++) {
		spell_columns[lev] = 0;
		spell_list[lev][0] = '\0';
	}
	
	for (sn = 0; sn < MAX_SKILL; sn++) {
		if (skill_table[sn].name == NULL)
			break;

		lev = skill_is_native(ch, sn) ?
			1 : skill_table[sn].skill_level[ch->class];

		if (lev >= LEVEL_HERO
		||  skill_table[sn].spell_fun == spell_null
		||  !SKILL_RACE_OK(ch, sn) || !SKILL_CLAN_OK(ch, sn)
		||  ch->pcdata->learned[sn] == 0)
			continue;

		found = TRUE;
		if (ch->level < lev)
			sprintf(buf, "%-18s  n/a      ", skill_table[sn].name);
		else {
			mana = UMAX(skill_table[sn].min_mana,
			       100/(2 + ch->level - lev));
			sprintf(buf, "%-18s  %3d mana  ",
				skill_table[sn].name,mana);
		}
			
		if (spell_list[lev][0] == '\0')
			sprintf(spell_list[lev],"\n\rLevel %2d: %s", lev, buf);
		else { /* append */
			if (++spell_columns[lev] % 2 == 0)
				strcat(spell_list[lev], "\n\r          ");
			strcat(spell_list[lev], buf);
		}
	}

	/* return results */
	
	if (!found) {
		send_to_char("You know no spells.\n\r",ch);
		return;
	}
	
	for (lev = 0; lev < LEVEL_HERO; lev++)
		if (spell_list[lev][0] != '\0')
			strcat(output, spell_list[lev]);
	strcat(output, "\n\r");
	page_to_char(output, ch);
}

void do_skills(CHAR_DATA *ch, const char *argument)
{
	char skill_list[LEVEL_HERO][MAX_STRING_LENGTH];
	char skill_columns[LEVEL_HERO];
	int sn,lev;
	bool found = FALSE;
	char buf[MAX_STRING_LENGTH];
	
	if (IS_NPC(ch))
		return;
	
	/* initialize data */
	for (lev = 0; lev < LEVEL_HERO; lev++) {
		skill_columns[lev] = 0;
		skill_list[lev][0] = '\0';
	}
	
	for (sn = 0; sn < MAX_SKILL; sn++) {
		if (skill_table[sn].name == NULL)
			break;

		lev = skill_is_native(ch, sn) ?
		      1 : skill_table[sn].skill_level[ch->class];

		if (lev >= LEVEL_HERO
		||  skill_table[sn].spell_fun != spell_null
		||  !SKILL_RACE_OK(ch, sn) || !SKILL_CLAN_OK(ch, sn)
		||  ch->pcdata->learned[sn] == 0)
			continue;

		found = TRUE;

		if (ch->level < lev)
			sprintf(buf, "%-18s n/a      ", skill_table[sn].name);
		else
			sprintf(buf, "%-18s %3d%%      ",
				skill_table[sn].name, ch->pcdata->learned[sn]);
	
		if (skill_list[lev][0] == '\0')
			sprintf(skill_list[lev],"\n\rLevel %2d: %s",lev,buf);
		else { /* append */
			if (++skill_columns[lev] % 2 == 0)
				strcat(skill_list[lev],"\n\r          ");
			strcat(skill_list[lev],buf);
		}
	}
	
	/* return results */
	
	if (!found) {
		send_to_char("You know no skills.\n\r",ch);
		return;
	}
	
	for (lev = 0; lev < LEVEL_HERO; lev++)
		if (skill_list[lev][0] != '\0')
			send_to_char(skill_list[lev],ch);
	send_to_char("\n\r",ch);
}


int base_exp(CHAR_DATA *ch, int points)
{
	int expl;

	if (IS_NPC(ch))    return 1500;
	expl = 1000 + pc_race_table[ORG_RACE(ch)].points +
						class_table[ch->class].points;

	return (expl * pc_race_table[ORG_RACE(ch)].class_mult[ch->class]/100);
}

int exp_to_level(CHAR_DATA *ch, int points)
{ 
	int base;

	base = base_exp(ch,points);
	return (base - exp_this_level(ch,ch->level,points));
}

int exp_this_level(CHAR_DATA *ch, int level, int points)
{
	int base;

	base = base_exp(ch,points);
	return (ch->exp - (ch->level * base));
}
	

int exp_per_level(CHAR_DATA *ch, int points)
{
	int expl;

	if (IS_NPC(ch))
			return 1000; 

	expl = 1000 + pc_race_table[ORG_RACE(ch)].points +
						class_table[ch->class].points;

	return expl * pc_race_table[ORG_RACE(ch)].class_mult[ch->class]/100;
}

		
/* checks for skill improvement */
void check_improve(CHAR_DATA *ch, int sn, bool success, int multiplier)
{
	int chance;

	if (IS_NPC(ch))
			return;

	if (ch->level < skill_table[sn].skill_level[ch->class]
	||  skill_table[sn].rating[ch->class] == 0
	||  ch->pcdata->learned[sn] == 0
	||  ch->pcdata->learned[sn] == 100)
			return;  /* skill is not known */ 

	/* check to see if the character has a chance to learn */
	chance = 10 * int_app[get_curr_stat(ch,STAT_INT)].learn;
	chance /= (multiplier *	skill_table[sn].rating[ch->class] * 4);
	chance += ch->level;

	if (number_range(1,1000) > chance)
		return;

	/* now that the character has a CHANCE to learn, see if they really have */	

	if (success) {
			chance = URANGE(5,100 - ch->pcdata->learned[sn], 95);
			if (number_percent() < chance) {
				act_nprintf(ch, NULL, NULL, TO_CHAR, POS_DEAD,
				           MSG_HAVE_BECOME_BETTER,
					       skill_table[sn].name);
				ch->pcdata->learned[sn]++;
				gain_exp(ch,2 * skill_table[sn].rating[ch->class]);
			}
	}
	else {
			chance = URANGE(5,ch->pcdata->learned[sn]/2,30);
			if (number_percent() < chance) {
				act_nprintf(ch, NULL, NULL, TO_CHAR, POS_DEAD,
					   MSG_LEARN_FROM_MISTAKES,
					skill_table[sn].name);
				ch->pcdata->learned[sn] += number_range(1,3);
				ch->pcdata->learned[sn] = UMIN(ch->pcdata->learned[sn],100);
				gain_exp(ch,2 * skill_table[sn].rating[ch->class]);
			}
	}
}


/* use for adding all skills available for that ch  */
void group_add(CHAR_DATA *ch)
{
	int sn;

	if (IS_NPC(ch)) /* NPCs do not have skills */
		return;

	for (sn = 0;  sn < MAX_SKILL; sn++)
		if (skill_table[sn].clan == 0 && SKILL_RACE_OK(ch, sn)
		&& ch->pcdata->learned[sn] < 1
		&& skill_table[sn].skill_level[ch->class] < LEVEL_IMMORTAL)
			ch->pcdata->learned[sn] = 1;
}


void do_slist(CHAR_DATA *ch, const char *argument)
{
	char skill_list[LEVEL_HERO][MAX_STRING_LENGTH];
	char skill_columns[LEVEL_HERO];
	int sn,lev,class;
	bool found = FALSE;
	char output[4*MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH]; 
	char arg[MAX_INPUT_LENGTH];
	
	if (IS_NPC(ch))
		return;
	
	output[0] = '\0';
	argument = one_argument(argument, arg);
	if (arg[0] == '\0') {
		send_to_char("syntax: slist <class name>.\n\r",ch);
		return;
	}

	class = class_lookup(arg);
	if (class == -1) {
		send_to_char("That is not a valid class.\n\r",ch);
		return;
	}

	/* initilize data */
	for (lev = 0; lev < LEVEL_HERO; lev++)
	{
		skill_columns[lev] = 0;
		skill_list[lev][0] = '\0';
	}
	
	for (sn = 0; sn < MAX_SKILL; sn++)
	{
		if (skill_table[sn].name == NULL)
		break;

	
		if (skill_table[sn].skill_level[class] < LEVEL_HERO &&
			  skill_table[sn].clan == CLAN_NONE &&
			  skill_table[sn].race == RACE_NONE)
		{
		found = TRUE;
		lev = skill_table[sn].skill_level[class];
		sprintf(buf,"%-18s          ",skill_table[sn].name); 
		if (skill_list[lev][0] == '\0')
		  sprintf(skill_list[lev],"\n\rLevel %2d: %s",lev,buf);
		else /* append */
		{
		  if (++skill_columns[lev] % 2 == 0)
		    strcat(skill_list[lev],"\n\r          ");
		  strcat(skill_list[lev],buf);
		}
		}
	}
	
	/* return results */
	
	if (!found) {
		send_to_char("That class know no skills.\n\r",ch);
		return;
	}
	
	for (lev = 0; lev < LEVEL_HERO; lev++)
		if (skill_list[lev][0] != '\0')
			strcat(output, skill_list[lev]);
	strcat(output, "\n\r");
	page_to_char(output, ch);
}


/* returns group number */
int group_lookup (const char *name)
{
	int gr;

	for (gr = 0; skill_groups[gr].name != NULL; gr++)
		if (str_prefix(name, skill_groups[gr].name) == 0)
			return gr;

	return -1;
} 

void do_glist(CHAR_DATA *ch , const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	int col = 0;
	int group, count;

	one_argument(argument, arg);
	
	if (arg[0] == '\0') {
		send_to_char("Syntax: glist <group>\n\r",ch);
		return;
	}

	if ((group = group_lookup(arg)) == -1) {
		send_to_char("That is not a valid group.\n\r",ch);
		return;
	}

	char_printf(ch, "Now listing group %s :\n\r", skill_groups[group].name);

	for (count = 0 ; count < MAX_SKILL; count++)
		if (skill_groups[group].bit == skill_table[count].group) {
			char_printf(ch, "%c%-18s",
				    SKILL_OK(ch, count) ? '*' : ' ',
				    skill_table[count].name);
			if (col)
				char_puts("\n\r", ch);
			col = 1 - col;
		}
	if (col)
		char_puts("\n\r", ch);
}

void do_slook(CHAR_DATA *ch, const char *argument)
{
	int sn;
	char arg[MAX_INPUT_LENGTH];

	one_argument(argument,arg);
	if (arg[0] == '\0') {
			 send_to_char("Syntax : slook <skill or spell name>.\n\r",ch);
			 return;
	}

	if ((sn = skill_lookup(arg)) == -1) { 
			send_to_char("That is not a spell or skill.\n\r",ch);
			return; 
	}

	char_printf(ch, "Skill '%s' in group '%s'.\n\r",
		    skill_table[sn].name,
		    flag_name_lookup(skill_groups, skill_table[sn].group));
}

#define PC_PRACTICER	123

void do_learn(CHAR_DATA *ch, const char *argument)
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	int sn;
	CHAR_DATA *mob;
	int adept;

	if (IS_NPC(ch))
			return;

			if (!IS_AWAKE(ch))
			{
				send_to_char("In your dreams, or what?\n\r", ch);
				return;
			}

			if (argument[0] == '\0')
			{
				send_to_char("Syntax: learn <skill | spell> <player>.\n\r", ch);
				return;
			}

			if (ch->practice <= 0)
			{
				send_to_char("You have no practice sessions left.\n\r", ch);
				return;
			}

			argument = one_argument(argument,arg);

			if ((sn = find_spell(ch,arg)) < 0) {
				send_to_char("You can't practice that.\n\r", ch);
				return;
			}

			if (sn == gsn_vampire)
			{
			 send_to_char("You can't practice that, only available at questor.\n\r",ch);
			 return;
			}

			argument = one_argument(argument,arg);
			
			if ((mob = get_char_room(ch,arg)) == NULL)
			{
				send_to_char("Your hero is not here.\n\r", ch);
				return;
			}
			
			if (IS_NPC(mob) || mob->level != HERO)
			{
			  send_to_char("You must find a hero , not an ordinary one.\n\r",ch);
			  return;
			}

			if (mob->status != PC_PRACTICER)
			{
			  send_to_char("Your hero doesn't want to teach you anything.\n\r",ch);
			  return;
			}

			if (get_skill(mob,sn) < 100)
			{
			  send_to_char("Your hero doesn't know that skill enough to teach you.\n\r",ch);
			  return;
			}

			adept = class_table[ch->class].skill_adept;

			if (ch->pcdata->learned[sn] >= adept)
			{
				sprintf(buf, "You are already learned at %s.\n\r",
					skill_table[sn].name);
				send_to_char(buf, ch);
			}
			else
			{
				if (!ch->pcdata->learned[sn]) ch->pcdata->learned[sn] = 1;
				ch->practice--;
				ch->pcdata->learned[sn] += 
					int_app[get_curr_stat(ch,STAT_INT)].learn / 
				    UMAX(skill_table[sn].rating[ch->class],1);
				mob->status = 0;
				act("You teach $T.",
					    mob, NULL, skill_table[sn].name, TO_CHAR);
				act("$n teachs $T.",
					    mob, NULL, skill_table[sn].name, TO_ROOM);
				if (ch->pcdata->learned[sn] < adept)
				{
					act("You learn $T.",
					    ch, NULL, skill_table[sn].name, TO_CHAR);
					act("$n learn $T.",
					    ch, NULL, skill_table[sn].name, TO_ROOM);
				}
				else
				{
					ch->pcdata->learned[sn] = adept;
					act("You are now learned at $T.",
					    ch, NULL, skill_table[sn].name, TO_CHAR);
					act("$n is now learned at $T.",
					    ch, NULL, skill_table[sn].name, TO_ROOM);
				}
			}
	return;
}


void do_teach(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch) || ch->level != LEVEL_HERO)
	{
			send_to_char("You must be a hero.\n\r",ch);
			return;
	}
	ch->status = PC_PRACTICER;
	send_to_char("Now , you can teach youngsters your 100% skills.\n\r",ch);
	return;
}
