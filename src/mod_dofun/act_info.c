/*
 * $Id: act_info.c,v 1.125 1998-09-11 16:18:47 fjoe Exp $
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
#include <unistd.h>
#include <ctype.h>
#include "merc.h"
#include "act_info.h"
#include "act_comm.h"
#include "hometown.h"
#include "interp.h"
#include "update.h"
#include "quest.h"
#include "obj_prog.h"
#include "fight.h"

#if defined(SUNOS) || defined(SVR4)
#	include <crypt.h>
#endif

/* command procedures needed */
DECLARE_DO_FUN(do_exits		);
DECLARE_DO_FUN(do_look		);
DECLARE_DO_FUN(do_help		);
DECLARE_DO_FUN(do_affects	);
DECLARE_DO_FUN(do_murder	);
DECLARE_DO_FUN(do_scan2 	);

char *get_stat_alias(CHAR_DATA *ch, int which);

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
	-1
};

/* for do_count */
int max_on = 0;

/*
 * Local functions.
 */
char *	format_obj_to_char	(OBJ_DATA *obj, CHAR_DATA *ch, bool fShort);
void	show_list_to_char	(OBJ_DATA *list, CHAR_DATA *ch,
				 bool fShort, bool fShowNothing);
void	show_char_to_char_0	(CHAR_DATA *victim, CHAR_DATA *ch);
void	show_char_to_char_1	(CHAR_DATA *victim, CHAR_DATA *ch);
void	show_char_to_char	(CHAR_DATA *list, CHAR_DATA *ch);
bool	check_blind		(CHAR_DATA *ch);
void	show_affects		(CHAR_DATA *ch, BUFFER *output);

#define strend(s) (strchr(s, '\0'))

char *obj_name(OBJ_DATA *obj, CHAR_DATA *ch)
{
	static char buf[MAX_STRING_LENGTH];
	char *name;
	char engname[MAX_STRING_LENGTH];

	name = mlstr_cval(obj->short_descr, ch);
	one_argument(obj->name, engname);
	snprintf(buf, sizeof(buf), "%s", name);
	if (name != mlstr_mval(obj->short_descr)) 
		sprintf(strend(buf), " (%s)", engname);
	return buf;
}

char *format_obj_to_char(OBJ_DATA *obj, CHAR_DATA *ch, bool fShort)
{
	static char buf[MAX_STRING_LENGTH];

	buf[0] = '\0';
	if ((fShort && mlstr_null(obj->short_descr))
	||  mlstr_null(obj->description))
		return buf;

	if (IS_SET(ch->comm, COMM_LONG)) {
		if (IS_OBJ_STAT(obj, ITEM_INVIS))
			strcat(buf, msg(MSG_INVIS, ch));
		if (IS_OBJ_STAT(obj, ITEM_DARK))
			strcat(buf, msg(MSG_DARK, ch));
		if (IS_AFFECTED(ch, AFF_DETECT_EVIL) && IS_OBJ_STAT(obj, ITEM_EVIL))
			strcat(buf, msg(MSG_RED_AURA, ch));
		if (IS_AFFECTED(ch, AFF_DETECT_GOOD) && IS_OBJ_STAT(obj,ITEM_BLESS))
			strcat(buf, msg(MSG_BLUE_AURA, ch));
		if (IS_AFFECTED(ch, AFF_DETECT_MAGIC) && IS_OBJ_STAT(obj,ITEM_MAGIC))
			strcat(buf, msg(MSG_MAGICAL, ch));
		if (IS_OBJ_STAT(obj, ITEM_GLOW))
			strcat(buf, msg(MSG_GLOWING, ch));
		if (IS_OBJ_STAT(obj, ITEM_HUM))
			strcat(buf, msg(MSG_HUMMING, ch));
	}
	else {
		static char FLAGS[] = "{x[{y.{D.{R.{B.{M.{W.{Y.{x] ";
		strcpy(buf, FLAGS);
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
		strcat(buf, obj_name(obj, ch));
		if (obj->pIndexData->vnum > 5)	/* not money, gold, etc */
			sprintf(strend(buf), " [{g%s{x]",
				get_cond_alias(obj, ch));
		return buf;
	}

	if (obj->in_room != NULL && IS_WATER(obj->in_room)) {
		char* p;

		p = strend(buf);
		strcat(buf, obj_name(obj, ch));
		p[0] = UPPER(p[0]);
		switch(dice(1,3)) {
		case 1:
			strcat(buf, " is floating gently on the water.");
			break;
		case 2:
			strcat(buf, " is making it's way on the water.");
			break;
		case 3:
			strcat(buf, " is getting wet by the water.");
			break;
		}
	}
	else
		strcat(buf, mlstr_cval(obj->description, ch));
	return buf;
}

/*
 * Show a list to a character.
 * Can coalesce duplicated items.
 */
void show_list_to_char(OBJ_DATA *list, CHAR_DATA *ch,
		       bool fShort, bool fShowNothing)
{
	BUFFER *output;
	char **prgpstrShow;
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
	prgpstrShow = alloc_mem(count * sizeof(char *));
	prgnShow    = alloc_mem(count * sizeof(int)  );
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
		if (prgpstrShow[iShow][0] == '\0') {
			free_string(prgpstrShow[iShow]);
			continue;
		}

		if (IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE)) {
			if (prgnShow[iShow] != 1) 
				buf_printf(output, "(%2d) ", prgnShow[iShow]);
			else
				buf_add(output,"     ");
		}

		buf_add(output, prgpstrShow[iShow]);
		buf_add(output,"\n\r");
		free_string(prgpstrShow[iShow]);
	}

	if (fShowNothing && nShow == 0) {
		if (IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE))
			send_to_char("     ", ch);
		send_to_char(msg(MSG_NOTHING, ch), ch);
	}

	page_to_char(buf_string(output),ch);

	/*
	 * Clean up.
	 */
	buf_free(output);
	free_mem(prgpstrShow, count * sizeof(char *));
	free_mem(prgnShow,    count * sizeof(int)  );
}

#define FLAG_SET(pos, c, exp) (FLAGS[pos] = (exp) ? (flags = TRUE, c) : '.')

void show_char_to_char_0(CHAR_DATA *victim, CHAR_DATA *ch)
{
	BUFFER *output;

	output = buf_new(0);

	if (IS_NPC(victim)) {
		if (!IS_NPC(ch) && ch->pcdata->questmob > 0
		&&  victim->hunter == ch)
			buf_add(output, msg(MSG_TARGET, ch));
	}
	else {
		if (IS_SET(victim->act, PLR_WANTED))
			buf_add(output, msg(MSG_WANTED, ch));

		if (IS_SET(victim->comm, COMM_AFK))
			buf_add(output, "{c[AFK]{x ");
	}

	if (IS_SET(ch->comm, COMM_LONG)) {
		if (IS_AFFECTED(victim, AFF_INVISIBLE))
			buf_add(output, msg(MSG_INVIS, ch));
		if (IS_AFFECTED(victim, AFF_HIDE)) 
			buf_add(output, msg(MSG_HIDDEN, ch));
		if (IS_AFFECTED(victim, AFF_CHARM)) 
			buf_add(output, msg(MSG_CHARMED, ch));
		if (IS_AFFECTED(victim, AFF_PASS_DOOR)) 
			buf_add(output, msg(MSG_TRANSLUCENT, ch));
		if (IS_AFFECTED(victim, AFF_FAERIE_FIRE)) 
			buf_add(output, msg(MSG_PINK_AURA, ch));
		if (IS_NPC(victim) && IS_SET(victim->act,ACT_UNDEAD)
		&&  IS_AFFECTED(ch, AFF_DETECT_UNDEAD))
			buf_add(output, msg(MSG_UNDEAD, ch));
		if (RIDDEN(victim))
			buf_add(output, msg(MSG_RIDDEN, ch));
		if (IS_AFFECTED(victim,AFF_IMP))
			buf_add(output, msg(MSG_IMPROVED, ch));
		if (IS_EVIL(victim) && IS_AFFECTED(ch, AFF_DETECT_EVIL))
			buf_add(output, msg(MSG_RED_AURA, ch));
		if (IS_GOOD(victim) && IS_AFFECTED(ch, AFF_DETECT_GOOD))
			buf_add(output, msg(MSG_GOLDEN_AURA, ch));
		if (IS_AFFECTED(victim, AFF_SANCTUARY))
			buf_add(output, msg(MSG_WHITE_AURA, ch));
		if (IS_AFFECTED(victim, AFF_FADE)) 
			buf_add(output, msg(MSG_FADE, ch));
		if (IS_AFFECTED(victim, AFF_CAMOUFLAGE)) 
			buf_add(output, msg(MSG_CAMF, ch));
	}
	else {
		static char FLAGS[] = "{x[{y.{D.{m.{c.{M.{D.{G.{b.{R.{Y.{W.{y.{g.{x] ";
		bool flags = FALSE;

		FLAG_SET( 5, 'I', IS_AFFECTED(victim, AFF_INVISIBLE));
		FLAG_SET( 8, 'H', IS_AFFECTED(victim, AFF_HIDE));
		FLAG_SET(11, 'C', IS_AFFECTED(victim, AFF_CHARM));
		FLAG_SET(14, 'T', IS_AFFECTED(victim, AFF_PASS_DOOR));
		FLAG_SET(17, 'P', IS_AFFECTED(victim, AFF_FAERIE_FIRE));
		FLAG_SET(20, 'U', IS_NPC(victim) &&
				  IS_SET(victim->act, ACT_UNDEAD) &&
				  IS_AFFECTED(ch, AFF_DETECT_UNDEAD));
		FLAG_SET(23, 'R', RIDDEN(victim));
		FLAG_SET(26, 'I', IS_AFFECTED(victim, AFF_IMP));
		FLAG_SET(29, 'E', IS_EVIL(victim) &&
				  IS_AFFECTED(ch, AFF_DETECT_EVIL));
		FLAG_SET(32, 'G', IS_GOOD(victim) &&
				  IS_AFFECTED(ch, AFF_DETECT_GOOD));
		FLAG_SET(35, 'S', IS_AFFECTED(victim, AFF_SANCTUARY));
		FLAG_SET(38, 'C', IS_AFFECTED(victim, AFF_CAMOUFLAGE));
		FLAG_SET(41, 'F', IS_AFFECTED(victim, AFF_FADE));

		if (flags)
			buf_add(output, FLAGS);
	}

	if (victim->invis_level >= LEVEL_HERO)
		buf_add(output, "[{WWizi{x] ");

	if (IS_NPC(victim) && victim->position == victim->start_pos)
		buf_printf(output, "{g%s{x",
			   mlstr_cval(victim->long_descr, ch));
	else {
		int msgnum;

		buf_add(output, PERS(victim, ch));

		if (!IS_NPC(victim) && !IS_SET(ch->comm, COMM_BRIEF)
		&&  victim->position == POS_STANDING)
			buf_printf(output, "%s{x", victim->pcdata->title);
	
		switch (victim->position) {
		case POS_DEAD:
			buf_add(output, vmsg(MSG_IS_DEAD, ch, victim));
			break;
	
		case POS_MORTAL:
			buf_add(output, vmsg(MSG_IS_MORTALLY_WOUNDED, ch, victim));
			break;
	
		case POS_INCAP:
			buf_add(output, vmsg(MSG_IS_INCAPACITATED, ch, victim));
			break;
	
		case POS_STUNNED:
			buf_add(output, vmsg(MSG_IS_LYING_HERE_STUNNED, ch, victim));
			break;
	
		case POS_SLEEPING:
			if (victim->on == NULL) {
				buf_add(output, msg(MSG_SLEEPING, ch));
				break;
			}
	
			if (IS_SET(victim->on->value[2], SLEEP_AT))
				msgnum = MSG_SLEEPING_AT;
			else if (IS_SET(victim->on->value[2], SLEEP_ON))
				msgnum = MSG_SLEEPING_ON;
			else
				msgnum = MSG_SLEEPING_IN;
	
			buf_printf(output, msg(msgnum, ch),
				mlstr_cval(victim->on->short_descr, ch));
			break;
	
		case POS_RESTING:
			if (victim->on == NULL) {
				buf_add(output, msg(MSG_RESTING, ch));
				break;
			}
	
			if (IS_SET(victim->on->value[2], REST_AT))
				msgnum = MSG_RESTING_AT;
			else if (IS_SET(victim->on->value[2], REST_ON))
				msgnum = MSG_RESTING_ON;
			else
				msgnum = MSG_RESTING_IN;
			buf_printf(output, msg(msgnum, ch),
				mlstr_cval(victim->on->short_descr, ch));
			break;
	
		case POS_SITTING:
			if (victim->on == NULL) {
				buf_add(output, msg(MSG_SITTING, ch));
				break;
			}
	
			if (IS_SET(victim->on->value[2], SIT_AT))
				msgnum = MSG_SITTING_AT;
			else if (IS_SET(victim->on->value[2], SIT_ON))
				msgnum = MSG_SITTING_ON;
			else
				msgnum = MSG_SITTING_IN;
			buf_printf(output, msg(msgnum, ch),
				mlstr_cval(victim->on->short_descr, ch));
			break;
	
		case POS_STANDING:
			if (victim->on == NULL) {
				if (MOUNTED(victim))
					buf_printf(output, msg(MSG_HERE_RIDING, ch),
						PERS(MOUNTED(victim),ch));
				else
					buf_add(output, msg(MSG_IS_HERE, ch));
				break;
			}
	
			if (IS_SET(victim->on->value[2],STAND_AT))
				msgnum = MSG_STANDING_AT;
			else if (IS_SET(victim->on->value[2],STAND_ON))
				msgnum = MSG_STANDING_ON;
			else
				msgnum = MSG_STANDING;
			buf_printf(output, msg(msgnum, ch),
				mlstr_cval(victim->on->short_descr, ch));
			break;
	
		case POS_FIGHTING:
			buf_add(output, msg(MSG_FIGHTING, ch));
			if (victim->fighting == NULL)
				buf_add(output, "thin air??");
			else if (victim->fighting == ch)
				buf_add(output, msg(MSG_FIGHTING_YOU, ch));
			else if (victim->in_room == victim->fighting->in_room)
				buf_printf(output, "%s.",
					   PERS(victim->fighting, ch));
			else
				buf_add(output, "somone who left??");
			break;
		}
	
		buf_add(output, "{x\n\r");
	}

	char_puts(buf_string(output), ch);
	buf_free(output);
}

void show_char_to_char_1(CHAR_DATA *victim, CHAR_DATA *ch)
{
	OBJ_DATA *obj;
	int i;
	int percent;
	bool found;
	int msgnum;
	char *desc;

	if (can_see(victim, ch)) {
		if (ch == victim)
			act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING,
					MSG_N_LOOKS_AT_SELF);
		else {
			act_nprintf(ch, NULL, victim, TO_VICT, POS_RESTING,
					MSG_N_LOOKS_AT_YOU);
			act_nprintf(ch, NULL, victim, TO_NOTVICT, POS_RESTING,
					MSG_N_LOOKS_AT_N);
		}
	}

	if (IS_NPC(victim))
		desc = mlstr_cval(victim->description, ch);
	else
		desc = mlstr_mval(victim->description);

	if (!IS_NULLSTR(desc))
		char_puts(desc, ch);
	else
		act_nprintf(victim, NULL, ch, TO_VICT, POS_DEAD,
				MSG_SEE_NOTHING_SPECIAL);

	if (MOUNTED(victim))
		char_printf(ch, msg(MSG_IS_RIDING_S, ch),
			    PERS(victim,ch), PERS(MOUNTED(victim),ch));
	if (RIDDEN(victim))
		char_printf(ch, vmsg(MSG_IS_RIDDEN_BY_S, ch, victim),
			    PERS(victim,ch), PERS(RIDDEN(victim),ch));

	if (victim->max_hit > 0)
		percent = (100 * victim->hit) / victim->max_hit;
	else
		percent = -1;

	if (percent >= 100)
		msgnum = MSG_IS_IN_PERFECT_HEALTH;
	else if (percent >= 90)
		msgnum = MSG_HAS_A_FEW_SCRATCHES;
	else if (percent >= 75)
		msgnum = MSG_HAS_SOME_SMALL_BUT_DISGUSTING_CUTS;
	else if (percent >= 50)
		msgnum = MSG_IS_COVERED_WITH_BLEEDING_WOUNDS;
	else if (percent >= 30)
		msgnum = MSG_IS_GUSHING_BLOOD;
	else if (percent >= 15)
		msgnum = MSG_IS_WRITHING_IN_AGONY;
	else if (percent >= 0)
		msgnum = MSG_IS_CONVULSING_ON_THE_GROUND;
	else
		msgnum = MSG_IS_NEARLY_DEAD;

	/* vampire ... */
	if (percent < 90 && ch->class == CLASS_VAMPIRE && ch->level > 10)
		gain_condition(ch, COND_BLOODLUST, -1);

	if (!IS_IMMORTAL(victim)) {
		char_printf(ch, "(%s) ", race_table[RACE(victim)].name);
		if (!IS_NPC(victim)) 
			char_printf(ch, "(%s) ", class_name(victim->class));
	}

	char_printf(ch, "%s %s\n\r",
		    PERS(victim, ch), vmsg(msgnum, ch, victim));

	found = FALSE;
	for (i = 0; show_order[i] != -1; i++) {
		if ((obj = get_eq_char(victim, show_order[i])) != NULL
		&&  can_see_obj(ch, obj)) {

			if (!found) {
				send_to_char("\n\r", ch);
				act(msg(MSG_IS_USING, ch), ch, NULL, victim,
				    TO_CHAR);
				found = TRUE;
			}

			act_printf(ch, NULL, NULL, TO_CHAR, POS_RESTING,
				   "%s%s",
				   msg(MSG_EQ_USED_AS_LIGHT + show_order[i], ch),
				   format_obj_to_char(obj, ch, TRUE));
		}
	}

	for (obj = victim->carrying; obj != NULL; obj = obj->next_content) {
		if (obj->wear_loc != WEAR_STUCK_IN || !can_see_obj(ch, obj))
			continue;

		if (!found) {
			send_to_char("\n\r", ch);
			act(msg(MSG_IS_USING, ch), ch, NULL, victim, TO_CHAR);
			found = TRUE;
		}
		act_printf(ch, NULL, NULL, TO_CHAR, POS_RESTING, "%s%s",
			   msg(MSG_EQ_STUCK_IN, ch),
			   format_obj_to_char(obj, ch, TRUE));
	}

	if (victim != ch
	&&  !IS_NPC(ch)
	&&  number_percent() < get_skill(ch, gsn_peek)) {
		send_to_char(msg(MSG_YOU_PEEK_AT_THE_INVENTORY, ch), ch);
		check_improve(ch, gsn_peek, TRUE, 4);
		show_list_to_char(victim->carrying, ch, TRUE, TRUE);
	}
}

void show_char_to_char(CHAR_DATA *list, CHAR_DATA *ch)
{
	CHAR_DATA *rch;
	int life_count=0;

	for (rch = list; rch != NULL; rch = rch->next_in_room) {
		if (rch == ch)
			continue;

		if (get_trust(ch) < rch->invis_level)
			continue;

		if (can_see(ch, rch))
			show_char_to_char_0(rch, ch);
		else if (room_is_dark(ch) && IS_AFFECTED(rch, AFF_INFRARED)) {
			send_to_char(msg(MSG_GLOWING_RED_EYES, ch), ch);
			if (!IS_IMMORTAL(rch))
				life_count++;
		}
		else if (!IS_IMMORTAL(rch))
			life_count++;
	}

	if (life_count && IS_AFFECTED(ch,AFF_DETECT_LIFE))
		char_printf(ch, msg(MSG_FEEL_MORE_LIVES, ch),
			    life_count, (life_count == 1) ? "form" : "forms");
	return;
}

bool check_blind_raw(CHAR_DATA *ch)
{
	if (!IS_NPC(ch) && IS_SET(ch->act,PLR_HOLYLIGHT))
		return TRUE;

	if (IS_AFFECTED(ch, AFF_BLIND))
		return FALSE;

	return TRUE;
}

bool check_blind(CHAR_DATA *ch)
{
	bool can_see = check_blind_raw(ch);

	if (!can_see)
		char_nputs(MSG_CANT_SEE_THING, ch);

	return can_see;
}

void do_clear(CHAR_DATA *ch, const char *argument)
{
	if (!IS_NPC(ch))
		send_to_char("\033[0;0H\033[2J", ch);
}

/* changes your scroll */
void do_scroll(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	int lines;

	one_argument(argument,arg);

	if (arg[0] == '\0') {
		if (ch->lines == 0)
			send_to_char("You do not page long messages.\n\r", ch);
		else
			char_printf(ch, "You currently display %d lines per "
					"page.\n\r", ch->lines + 2);
		return;
	}

	if (!is_number(arg)) {
		send_to_char("You must provide a number.\n\r",ch);
		return;
	}

	lines = atoi(arg);
	if (lines == 0) {
		send_to_char("Paging disabled.\n\r",ch);
		ch->lines = 0;
		return;
	}

	if (lines < 10 || lines > 100) {
		send_to_char("You must provide a reasonable number.\n\r",ch);
		return;
	}

	char_printf(ch, "Scroll set to %d lines.\n\r", lines);
	ch->lines = lines - 2;
}

/* RT does socials */
void do_socials(CHAR_DATA *ch, const char *argument)
{
	int iSocial;
	int col = 0;

	for (iSocial = 0; social_table[iSocial].name != NULL; iSocial++) {
		char_printf(ch, "%-12s", social_table[iSocial].name);
		if (++col % 6 == 0)
			send_to_char("\n\r",ch);
	}

	if (col % 6)
		send_to_char("\n\r",ch);
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

void do_story(CHAR_DATA *ch, const char *argument)
{
	do_help(ch, "story");
}

void do_wizlist(CHAR_DATA *ch, const char *argument)
{
	do_help(ch, "wizlist");
}

/* RT this following section holds all the auto commands from ROM, as well as
   replacements for config */
#define do_print_sw(ch, swname, sw) \
		char_printf(ch, "%-16s {r%s{x\n\r", swname, sw ? "ON" : "OFF");
void do_autolist(CHAR_DATA *ch, const char *argument)
{
	/* lists most player flags */
	if (IS_NPC(ch))
		return;

	send_to_char("action         status\n\r",ch);
	send_to_char("---------------------\n\r",ch);
	do_print_sw(ch, "color", IS_SET(ch->act,PLR_COLOR));
	do_print_sw(ch, "autoassist", IS_SET(ch->act,PLR_AUTOASSIST));
	do_print_sw(ch, "autoexit", IS_SET(ch->act,PLR_AUTOEXIT));
	do_print_sw(ch, "autogold", IS_SET(ch->act,PLR_AUTOGOLD));
	do_print_sw(ch, "autoloot", IS_SET(ch->act,PLR_AUTOLOOT));
	do_print_sw(ch, "autosac", IS_SET(ch->act,PLR_AUTOSAC));
	do_print_sw(ch, "autosplit", IS_SET(ch->act,PLR_AUTOSPLIT));
	do_print_sw(ch, "compact mode", IS_SET(ch->comm,COMM_COMPACT));
	do_print_sw(ch, "long flags", IS_SET(ch->comm,COMM_LONG));
	do_print_sw(ch, "prompt", IS_SET(ch->comm,COMM_PROMPT));
	do_print_sw(ch, "combine items", IS_SET(ch->comm,COMM_COMBINE));

	if (IS_SET(ch->act, PLR_NOSUMMON))
		send_to_char("You can only be summoned players within "
			     "your PK range.\n\r",ch);
	else
		send_to_char("You can be summoned by anyone.\n\r",ch);

	if (IS_SET(ch->act,PLR_NOFOLLOW))
		send_to_char("You do not welcome followers.\n\r",ch);
	else
		send_to_char("You accept followers.\n\r",ch);
}

void do_autoassist(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch))
	  return;

	if (IS_SET(ch->act,PLR_AUTOASSIST)) {
		send_to_char("Autoassist removed.\n\r",ch);
		REMOVE_BIT(ch->act,PLR_AUTOASSIST);
	}
	else {
		send_to_char("You will now assist when needed.\n\r",ch);
		SET_BIT(ch->act,PLR_AUTOASSIST);
	}
}

void do_autoexit(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch))
		return;

	if (IS_SET(ch->act,PLR_AUTOEXIT)) {
		send_to_char("Exits will no longer be displayed.\n\r",ch);
		REMOVE_BIT(ch->act,PLR_AUTOEXIT);
	}
	else {
		send_to_char("Exits will now be displayed.\n\r",ch);
		SET_BIT(ch->act,PLR_AUTOEXIT);
	}
}

void do_autogold(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch))
		return;

	if (IS_SET(ch->act,PLR_AUTOGOLD)) {
		send_to_char("Autogold removed.\n\r",ch);
		REMOVE_BIT(ch->act,PLR_AUTOGOLD);
	}
	else {
		send_to_char("Automatic gold looting set.\n\r",ch);
		SET_BIT(ch->act,PLR_AUTOGOLD);
	}
}

void do_autoloot(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch))
		return;

	if (IS_SET(ch->act,PLR_AUTOLOOT)) {
		send_to_char("Autolooting removed.\n\r",ch);
		REMOVE_BIT(ch->act,PLR_AUTOLOOT);
	}
	else {
		send_to_char("Automatic corpse looting set.\n\r",ch);
		SET_BIT(ch->act,PLR_AUTOLOOT);
	}
}

void do_autosac(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch))
		return;

	if (IS_SET(ch->act,PLR_AUTOSAC)) {
		send_to_char("Autosacrificing removed.\n\r",ch);
		REMOVE_BIT(ch->act,PLR_AUTOSAC);
	}
	else {
		send_to_char("Automatic corpse sacrificing set.\n\r",ch);
		SET_BIT(ch->act,PLR_AUTOSAC);
	}
}

void do_autosplit(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch))
		return;

	if (IS_SET(ch->act,PLR_AUTOSPLIT)) {
		send_to_char("Autosplitting removed.\n\r",ch);
		REMOVE_BIT(ch->act,PLR_AUTOSPLIT);
	}
	else {
		send_to_char("Automatic gold splitting set.\n\r",ch);
		SET_BIT(ch->act,PLR_AUTOSPLIT);
	}
}

void do_color(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch))
		return;

	if (IS_SET(ch->act,PLR_COLOR)) {
		REMOVE_BIT(ch->act,PLR_COLOR);
		send_to_char("Color is now OFF, *sigh*\n\r", ch);
	}
	else {
		SET_BIT(ch->act,PLR_COLOR);
		send_to_char("{BC{Ro{Yl{Co{Gr{x is now {RON{x, Way Cool!\n\r",
			     ch);
	}
}

void do_brief(CHAR_DATA *ch, const char *argument)
{
	if (IS_SET(ch->comm,COMM_BRIEF)) {
		send_to_char("Full descriptions activated.\n\r",ch);
		REMOVE_BIT(ch->comm,COMM_BRIEF);
	}
	else {
		send_to_char("Short descriptions activated.\n\r",ch);
		SET_BIT(ch->comm,COMM_BRIEF);
	}
}

void do_compact(CHAR_DATA *ch, const char *argument)
{
	if (IS_SET(ch->comm,COMM_COMPACT)) {
		send_to_char("Compact mode removed.\n\r",ch);
		REMOVE_BIT(ch->comm,COMM_COMPACT);
	}
	else {
		send_to_char("Compact mode set.\n\r",ch);
		SET_BIT(ch->comm,COMM_COMPACT);
	}
}

void do_long(CHAR_DATA *ch, const char *argument)
{
	if (IS_SET(ch->comm,COMM_LONG)) {
		send_to_char("Long flags mode removed.\n\r",ch);
		REMOVE_BIT(ch->comm,COMM_LONG);
	}
	else {
		send_to_char("Long flags mode set.\n\r",ch);
		SET_BIT(ch->comm,COMM_LONG);
	}
}

void do_show(CHAR_DATA *ch, const char *argument)
{
	if (IS_SET(ch->comm,COMM_SHOW_AFFECTS)) {
		send_to_char("Affects will no longer be shown in score.\n\r",
			     ch);
		REMOVE_BIT(ch->comm,COMM_SHOW_AFFECTS);
	}
	else {
		send_to_char("Affects will now be shown in score.\n\r", ch);
		SET_BIT(ch->comm,COMM_SHOW_AFFECTS);
	}
}

void do_prompt(CHAR_DATA *ch, const char *argument)
{
	char *prompt;

	if (argument[0] == '\0') {
		if (IS_SET(ch->comm,COMM_PROMPT)) {
			send_to_char("You will no longer see prompts.\n\r",ch);
			REMOVE_BIT(ch->comm,COMM_PROMPT);
		}
		else {
			send_to_char("You will now see prompts.\n\r",ch);
			SET_BIT(ch->comm,COMM_PROMPT);
		}
		return;
	}

	if (!strcmp(argument, "all"))
		prompt = str_dup(DEFAULT_PROMPT);
	else {
		prompt = str_printf("%s ", argument);
		smash_tilde(prompt);
	}

	free_string(ch->prompt);
	ch->prompt = prompt;
	char_printf(ch, "Prompt set to '%s'\n\r", ch->prompt);
}

void do_combine(CHAR_DATA *ch, const char *argument)
{
	if (IS_SET(ch->comm,COMM_COMBINE)) {
		send_to_char("Long inventory selected.\n\r",ch);
		REMOVE_BIT(ch->comm,COMM_COMBINE);
	}
	else {
		send_to_char("Combined inventory selected.\n\r",ch);
		SET_BIT(ch->comm,COMM_COMBINE);
	}
}

void do_noloot(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch))
		return;

	if (IS_SET(ch->act,PLR_CANLOOT)) {
		send_to_char("Your corpse is now safe from thieves.\n\r",ch);
		REMOVE_BIT(ch->act,PLR_CANLOOT);
	}
	else {
		send_to_char("Your corpse may now be looted.\n\r",ch);
		SET_BIT(ch->act,PLR_CANLOOT);
	}
}

void do_nofollow(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch))
		return;

	if (IS_AFFECTED(ch, AFF_CHARM))  {
		send_to_char("You don't want to leave "
			     "your beloved master.\n\r", ch);
		return;
	}

	if (IS_SET(ch->act,PLR_NOFOLLOW)) {
		send_to_char("You now accept followers.\n\r", ch);
		REMOVE_BIT(ch->act,PLR_NOFOLLOW);
	}
	else {
		send_to_char("You no longer accept followers.\n\r", ch);
		SET_BIT(ch->act,PLR_NOFOLLOW);
		die_follower(ch);
	}
}

void do_nosummon(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch)) {
		if (IS_SET(ch->imm_flags,IMM_SUMMON)) {
			send_to_char("You are no longer immune "
				     "to summoning.\n\r", ch);
			REMOVE_BIT(ch->imm_flags,IMM_SUMMON);
		}
		else {
			send_to_char("You are now immune to summoning.\n\r",
				     ch);
			SET_BIT(ch->imm_flags,IMM_SUMMON);
		}
	}
	else {
		if (IS_SET(ch->act,PLR_NOSUMMON)) {
			send_to_char("You may now be summoned by anyone.\n\r",
				     ch);
			REMOVE_BIT(ch->act,PLR_NOSUMMON);
		}
		else {
			send_to_char("You may only be summoned by players "
				     "within your PK range.\n\r", ch);
			SET_BIT(ch->act,PLR_NOSUMMON);
		}
	}
}

void do_look_in(CHAR_DATA* ch, const char *argument)
{
	OBJ_DATA *obj;

	if ((obj = get_obj_here(ch, argument)) == NULL) {
		send_to_char(msg(MSG_YOU_DONT_SEE_THAT, ch), ch);
		return;
	}

	switch (obj->item_type) {
	default:
		send_to_char(msg(MSG_THATS_NOT_CONTAINER, ch), ch);
		break;

	case ITEM_DRINK_CON:
		if (obj->value[1] <= 0) {
			send_to_char(msg(MSG_IT_IS_EMPTY, ch), ch);
			break;
		}

		char_printf(ch, msg(MSG_ITS_FILLED_S, ch),
			    obj->value[1] < obj->value[0] / 4 ?
			    msg(MSG_LESS_THAN_HALF, ch) :
			    obj->value[1] < 3 * obj->value[0] / 4 ?
			    msg(MSG_ABOUT_HALF,ch):msg(MSG_MORE_THAN_HALF,ch),
			    liq_table[obj->value[2]].liq_color);
		break;

	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
	case ITEM_CORPSE_PC:
		if (IS_SET(obj->value[1], CONT_CLOSED)) {
			send_to_char(msg(MSG_IT_IS_CLOSED, ch), ch);
			break;
		}

		act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD, MSG_P_HOLDS);
		show_list_to_char(obj->contains, ch, TRUE, TRUE);
		break;
	}
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
		char_nputs(MSG_CANT_SEE_BUT_STARS, ch);
		return;
	}

	if (ch->position == POS_SLEEPING) {
		char_nputs(MSG_CANT_SEE_SLEEPING, ch);
		return;
	}

	if (!check_blind(ch))
		return;

	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);
	number = number_argument(arg1, arg3);
	count = 0;

	if (arg1[0] == '\0' || !str_cmp(arg1, "auto")) {

		/* 'look' or 'look auto' */

	    if (!room_is_dark(ch)) {
		char *name;
		char *engname;

		name = mlstr_cval(ch->in_room->name, ch);
		engname = mlstr_mval(ch->in_room->name);
		char_printf(ch, "{W%s", name);
		if (ch->lang && name != engname)
			char_printf(ch, " (%s){x", engname);
		else
			char_puts("{x", ch);
		
		if ((IS_IMMORTAL(ch) && (IS_NPC(ch) ||
					 IS_SET(ch->act, PLR_HOLYLIGHT)))
		||  IS_BUILDER(ch, ch->in_room->area))
			char_printf(ch, " [Room %d]",ch->in_room->vnum);

		send_to_char("\n\r", ch);

		if (arg1[0] == '\0'
		||  (!IS_NPC(ch) && !IS_SET(ch->comm, COMM_BRIEF)))
			char_printf(ch, "  %s",
				    mlstr_cval(ch->in_room->description, ch));

		if (!IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOEXIT)) {
			send_to_char("\n\r", ch);
			do_exits(ch, "auto");
		}
	    } else 
		char_nputs(MSG_PITCH_BLACK, ch);

		show_list_to_char(ch->in_room->contents, ch, FALSE, FALSE);
		show_char_to_char(ch->in_room->people, ch);
		return;
	}

	if (!str_cmp(arg1, "i")
	||  !str_cmp(arg1, "in")
	||  !str_cmp(arg1,"on")) {
		/* 'look in' */
		if (arg2[0] == '\0') {
			char_nputs(MSG_LOOK_IN_WHAT, ch);
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

			if (ch->master)
				stop_follower(ch);
			add_follower(ch, victim);
			ch->leader = victim;

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
			if (ed != NULL)
				if (++count == number) {
					char_mlputs(ed->description, ch);
					return;
				}
				else
					continue;

			ed = ed_lookup(arg3, obj->pIndexData->ed);

			if (ed != NULL)
				if (++count == number) {
					char_mlputs(ed->description, ch);
					return;
				}
				else
					continue;

			if (is_name(arg3, obj->name))
				if (++count == number) {
					char_nputs(MSG_NOTHING_SPECIAL_IT, ch);
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
					char_mlputs(ed->description, ch);
					return;
				}

			ed = ed_lookup(arg3, obj->pIndexData->ed);
			if (ed != NULL)
				if (++count == number) {
					char_mlputs(ed->description, ch);
					return;
				}
		}

		if (is_name(arg3, obj->name))
			if (++count == number) {
				char_mlputs(obj->description, ch);
				send_to_char("\n\r",ch);
				return;
			}
	}

	ed = ed_lookup(arg3, ch->in_room->ed);
	if (ed != NULL) {
		if (++count == number) {
			char_mlputs(ed->description, ch);
			return;
		}
	}

	if (count > 0 && count != number) {
		if (count == 1)
			char_nprintf(ch, MSG_ONLY_SEE_ONE_S, arg3);
		else
			char_nprintf(ch, MSG_ONLY_SEE_D_THOSE, count);
		return;
	}

		   if (!str_cmp(arg1, "n") || !str_cmp(arg1, "north")) door = 0;
	else if (!str_cmp(arg1, "e") || !str_cmp(arg1, "east")) door = 1;
	else if (!str_cmp(arg1, "s") || !str_cmp(arg1, "south")) door = 2;
	else if (!str_cmp(arg1, "w") || !str_cmp(arg1, "west")) door = 3;
	else if (!str_cmp(arg1, "u") || !str_cmp(arg1, "up" )) door = 4;
	else if (!str_cmp(arg1, "d") || !str_cmp(arg1, "down")) door = 5;
	else {
		char_nputs(MSG_YOU_DONT_SEE_THAT, ch);
		return;
	}

	/* 'look direction' */
	if ((pexit = ch->in_room->exit[door]) == NULL) {
		char_nputs(MSG_NOTHING_SPECIAL_THERE, ch);
		return;
	}

	if (!IS_NULLSTR(mlstr_mval(pexit->description)))
		char_mlputs(pexit->description, ch);
	else
		char_nputs(MSG_NOTHING_SPECIAL_THERE, ch);

	if (pexit->keyword    != NULL
	&&  pexit->keyword[0] != '\0'
	&&  pexit->keyword[0] != ' ') {
		if (IS_SET(pexit->exit_info, EX_CLOSED)) {
			act_nprintf(ch, NULL, pexit->keyword, TO_CHAR,
					POS_DEAD, MSG_THE_D_IS_CLOSED);
		}
		else if (IS_SET(pexit->exit_info, EX_ISDOOR))
			act_nprintf(ch, NULL, pexit->keyword, TO_CHAR,
					POS_DEAD, MSG_THE_D_IS_OPEN);
	}
}

void do_examine(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;

	one_argument(argument, arg);

	if (ch->desc == NULL)
		return;

	if (ch->position < POS_SLEEPING) {
		char_nputs(MSG_CANT_SEE_BUT_STARS, ch);
		return;
	}

	if (ch->position == POS_SLEEPING) {
		char_nputs(MSG_CANT_SEE_SLEEPING, ch);
		return;
	}

	if (!check_blind(ch))
		return;

	if (arg[0] == '\0') {
		char_nputs(MSG_EXA_WHAT, ch);
		return;
	}

	do_look(ch, arg);

	if ((obj = get_obj_here(ch, arg)) != NULL) {
		switch (obj->item_type) {
		case ITEM_MONEY:
			if (obj->value[0] == 0) {
				if (obj->value[1] == 0)
					char_nputs(MSG_NO_COINS_PILE, ch);
				else if (obj->value[1] == 1)
					char_nputs(MSG_ONE_GOLD_COIN, ch);
				else
					char_nprintf(ch, MSG_D_GOLD_COINS,
						     obj->value[1]);
			}
			else if (obj->value[1] == 0) {
				if (obj->value[0] == 1)
					char_nputs(MSG_ONE_SILVER_COIN, ch);
				else
					char_nprintf(ch, MSG_D_SILVER_COINS,
						     obj->value[0]);
			}
			else
				char_nprintf(ch, MSG_D_SILVER_AND_D_GOLD, 
					    obj->value[1], obj->value[0]);
			break;

		case ITEM_DRINK_CON:
		case ITEM_CONTAINER:
		case ITEM_CORPSE_NPC:
		case ITEM_CORPSE_PC:
			do_look_in(ch, argument);
		}
	}
}

/*
 * Thanks to Zrin for auto-exit part.
 */
void do_exits(CHAR_DATA *ch, const char *argument)
{
	extern char * const dir_name[];
	EXIT_DATA *pexit;
	bool found;
	bool fAuto;
	int door;

	fAuto  = !str_cmp(argument, "auto");

	if (!check_blind(ch))
		return;

	if (fAuto)
		char_nputs(MSG_EXITS, ch);
	else if (IS_IMMORTAL(ch))
		char_nprintf(ch, MSG_MSG_OBVIOUS_EXITS_IMM, ch->in_room->vnum);
	else
		char_nputs(MSG_OBVIOUS_EXITS, ch);

	found = FALSE;
	for (door = 0; door <= 5; door++) {
		if ((pexit = ch->in_room->exit[door]) != NULL
		&&  pexit->u1.to_room != NULL
		&&  can_see_room(ch,pexit->u1.to_room)
		&&  !IS_SET(pexit->exit_info, EX_CLOSED)) {
			found = TRUE;
			if (fAuto)
				char_printf(ch, " %s", dir_name[door]);
			else {
				char_printf(ch, "{C%-5s{x - %s",
					    capitalize(dir_name[door]),
					    room_dark(pexit->u1.to_room) ?
					    msg(MSG_TOO_DARK_TO_TELL, ch) :
					    mlstr_cval(pexit->u1.to_room->name,
							ch));
				if (IS_IMMORTAL(ch))
					char_nprintf(ch, MSG_ROOM_D,
						     pexit->u1.to_room->vnum);
				else
					char_puts("\n\r", ch);
			}
		}

		if (number_percent() < get_skill(ch,gsn_perception)
		&&  (pexit = ch->in_room->exit[door]) != NULL
		&&  pexit->u1.to_room != NULL
		&&  can_see_room(ch,pexit->u1.to_room)
		&&   IS_SET(pexit->exit_info, EX_CLOSED)) {
			check_improve(ch,gsn_perception, TRUE, 5);
			found = TRUE;

			if (fAuto)
				char_printf(ch, " %s*", dir_name[door]);
			else {
				char_printf(ch, "%-5s * (%s)",
					    capitalize(dir_name[door]),
					    pexit->keyword);

				if (IS_IMMORTAL(ch))
					char_nprintf(ch, MSG_ROOM_D,
						     pexit->u1.to_room->vnum);
				else
					char_puts("\n\r", ch);
			}
		}
	}

	if (!found)
		char_nputs(fAuto ? MSG_NONE : MSG_NONE_DOT, ch);

	if (fAuto)
		char_puts("]{x\n\r", ch);
}

void do_worth(CHAR_DATA *ch, const char *argument)
{
	char_printf(ch, msg(MSG_HAVE_D_GOLD_D_SILVER, ch),
		    ch->gold, ch->silver);
	if (!IS_NPC(ch) && ch->level < LEVEL_HERO)
		char_printf(ch, msg(MSG_AND_D_EXP, ch),
			    ch->exp, exp_to_level(ch));
	char_puts(".\n\r", ch);

	if (!IS_NPC(ch))
		char_printf(ch, msg(MSG_HAVE_KILLED, ch),
			    ch->pcdata->has_killed,
			    IS_GOOD(ch) ? msg(MSG_NON_GOODS, ch) :
			    IS_EVIL(ch) ? msg(MSG_NON_EVILS, ch) : 
					  msg(MSG_NON_NEUTRALS, ch),
			    ch->pcdata->anti_killed,
			    IS_GOOD(ch) ? msg(MSG_GOODS, ch) :
			    IS_EVIL(ch) ? msg(MSG_EVILS, ch) : 
					  msg(MSG_NEUTRALS, ch));
}

char *	const	day_name	[] =
{
	"the Moon", "the Bull", "Deception", "Thunder", "Freedom",
	"the Great Gods", "the Sun"
};

char *	const	month_name	[] =
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
		    "It is %d o'clock %s, Day of %s, %d%s the Month of %s.\n\r",
		    (time_info.hour % 12 == 0) ? 12 : time_info.hour %12,
		    time_info.hour >= 12 ? "pm" : "am",
		    day_name[day % 7],
		    day, suf, month_name[time_info.month]);

	if (!IS_SET(ch->in_room->room_flags,ROOM_INDOORS) || IS_IMMORTAL(ch))
		act_nprintf(ch, NULL, NULL, TO_CHAR, POS_RESTING,
			   MSG_ITS_S,
			   (time_info.hour>=5 && time_info.hour<9) ?
						msg(MSG_TIME_DAWN, ch) :
			   (time_info.hour>=9 && time_info.hour<12) ?
						msg(MSG_TIME_MORNING, ch) :
			   (time_info.hour>=12 && time_info.hour<18) ?
						msg(MSG_TIME_MID_DAY, ch) :
			   (time_info.hour>=18 && time_info.hour<21) ?
						msg(MSG_TIME_EVENING, ch) :
			   msg(MSG_TIME_NIGHT, ch));

	if (!IS_IMMORTAL(ch))
		return;

	char_printf(ch, "MUDDY started up at %s\n\r"
			"The system time is %s.\n\r",
			str_boot_time, (char*) ctime(&current_time));
}

void do_date(CHAR_DATA *ch, const char *argument)
{
	time_t t = time(NULL);
	char_puts(ctime(&t), ch);
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
		send_to_char("You can't see the weather indoors.\n\r", ch);
		return;
	}

	char_printf(ch, "The sky is %s and %s.\n\r",
		    sky_look[weather_info.sky],
		    weather_info.change >= 0 ?
		    "a warm southerly breeze blows" :
		    "a cold northern gust blows");
}

void do_help(CHAR_DATA *ch, const char *argument)
{
	BUFFER *output;

	if (argument[0] == '\0')
		argument = "summary";

	output = buf_new(0);
	help_show(ch, output, argument);
	page_to_char(buf_string(output), ch);
	buf_free(output);
}

static void do_who_raw(CHAR_DATA* ch, CHAR_DATA *wch, BUFFER* output)
{
	CLAN_DATA *clan;
	CLASS_DATA *cl;

	if ((cl = class_lookup(wch->class)) == NULL)
		return;

	buf_add(output, "[");
	if (IS_IMMORTAL(ch) || ch == wch
	||  wch->level >= LEVEL_HERO || get_curr_stat(wch, STAT_CHA) < 18)
		buf_printf(output, "{C%3d{x ", wch->level);
	else
		buf_add(output, "    ");

	if (wch->level >= LEVEL_HERO) {
		if (IS_IMMORTAL(ch))
			buf_add(output, "  ");
		buf_add(output, "{G");
		switch (wch->level) {
		case IMPLEMENTOR:	buf_add(output, " IMP "); break;
		case CREATOR:		buf_add(output, " CRE "); break;
		case SUPREME:		buf_add(output, " SUP "); break;
		case DEITY:		buf_add(output, " DEI "); break;
		case GOD:		buf_add(output, " GOD "); break;
		case IMMORTAL:		buf_add(output, " IMM "); break;
		case DEMI:		buf_add(output, " DEM "); break;
		case ANGEL:		buf_add(output, " ANG "); break;
		case AVATAR:		buf_add(output, " AVA "); break;
		case HERO:		buf_add(output, "HERO "); break;
		}
		buf_add(output, "{x");
		if (IS_IMMORTAL(ch))
			buf_add(output, "  ");
	}
	else {
		if (RACE(wch) < MAX_PC_RACE)
			buf_add(output, pc_race_table[RACE(wch)].who_name);
		else 
			buf_add(output, "     ");

		if (IS_IMMORTAL(ch))
			buf_printf(output, " {Y%s{x", cl->who_name);
	}
	buf_add(output, "] ");

	if (wch->clan
	&&  (clan = clan_lookup(wch->clan))
	&&  (!IS_SET(clan->flags, CLAN_HIDDEN) ||
	     wch->clan == ch->clan || IS_IMMORTAL(ch)))
		buf_printf(output, "[{c%s{x] ", clan->name);

	if (IS_SET(wch->comm, COMM_AFK))
		buf_add(output, "{c[AFK]{x ");

	if (wch->invis_level >= LEVEL_HERO)
		buf_add(output, "[{WWizi{x] ");

	if (in_PK(ch, wch) && !IS_IMMORTAL(ch))
		buf_add(output, "{r[{RPK{r]{x ");

	if (IS_SET(wch->act, PLR_WANTED))
		buf_add(output, "{R(MSG_WANTED){x ");

	if (IS_IMMORTAL(wch))
		buf_printf(output, "{W%s{x", wch->name);
	else
		buf_add(output, wch->name);

	if (IS_NPC(wch))
		buf_add(output, " Believer of Chronos");
	else
		buf_add(output, wch->pcdata->title);

	buf_add(output, "{x\n\r");
}

void do_who(CHAR_DATA *ch, const char *argument)
{
	BUFFER *output;
	DESCRIPTOR_DATA *d;
#if 0
	int iClass;
#endif
	int iRace;
	int iLevelLower;
	int iLevelUpper;
	int nNumber;
	int nMatch;
	int vnum;
	int count;
	bool rgfRace[MAX_PC_RACE];
	bool fClassRestrict;
	bool fRaceRestrict;
	bool fImmortalOnly;
	bool fPKRestrict;
	bool fTattoo;

	/*
	 * Set default arguments.
	 */
	iLevelLower    = 0;
	iLevelUpper    = MAX_LEVEL;
	fClassRestrict = FALSE;
	fRaceRestrict = FALSE;
	fPKRestrict = FALSE;
	fImmortalOnly  = FALSE;
	vnum = 0;
	fTattoo = FALSE;

	for (iRace = 0; iRace < MAX_PC_RACE; iRace++)
		rgfRace[iRace] = FALSE;

	/*
	 * Parse arguments.
	 */
	nNumber = 0;
	for (;;) {
		char arg[MAX_STRING_LENGTH];

		argument = one_argument(argument, arg);
		if (arg[0] == '\0')
			break;

		if (!str_cmp(arg,"pk")) {
			fPKRestrict = TRUE;
			break;
		}

		if (!str_cmp(arg,"tattoo")) {
			if (get_eq_char(ch,WEAR_TATTOO) == NULL) {
				send_to_char("You haven't got a tattoo yet!\n\r",ch);
				return;
			} else {
				fTattoo = TRUE;
				vnum = get_eq_char(ch,WEAR_TATTOO)->pIndexData->vnum;
				break;
			}
		}

		if (is_number(arg) && IS_IMMORTAL(ch)) {
			switch (++nNumber) {
			case 1: iLevelLower = atoi(arg); break;
			case 2: iLevelUpper = atoi(arg); break;
			default:
				send_to_char("This function of who is for "
					     "immortals.\n\r",ch);
				return;
			}
			continue;
		}

		/*
		 * Look for classes to turn on.
		 */
		if (arg[0] == 'i')
			fImmortalOnly = TRUE;
		else {
			iRace = race_lookup(arg);

			if (iRace == 0 || iRace >= MAX_PC_RACE) {
#if 0
				iClass = cln_lookup(arg);
				if (iClass == -1 || !IS_IMMORTAL(ch)) {
					send_to_char("That's not a "
						     "valid race.\n\r",
						     ch);
					return;
				}
				fClassRestrict = TRUE;
				rgfClass[iClass] = TRUE;
#endif
			}
			else {
				fRaceRestrict = TRUE;
				rgfRace[iRace] = TRUE;
			}
		}
	}

	/*
	 * Now show matching chars.
	 */
	nMatch = 0;
	output = buf_new(0);
	for (d = descriptor_list; d != NULL; d = d->next) {
		CHAR_DATA *wch;

		/*
		 * Check for match against restrictions.
		 * Don't use trust as that exposes trusted mortals.
		 */
		if (d->connected != CON_PLAYING || !can_see(ch, d->character))
				continue;

		if (IS_VAMPIRE(d->character) && !IS_IMMORTAL(ch)
		&&  ch != d->character)
			continue;

		wch = (d->original != NULL) ? d->original : d->character;
		if (!can_see(ch, wch)) /* can't see switched wizi imms */
			continue;

		if (wch->level < iLevelLower || wch->level > iLevelUpper
		||  (fImmortalOnly && wch->level < LEVEL_HERO)
#if 0
		||  (fClassRestrict && !rgfClass[wch->class])
#endif
		||  (fClassRestrict && IS_IMMORTAL(wch))
		||  (fRaceRestrict && !rgfRace[RACE(wch)])
		||  (fRaceRestrict && IS_IMMORTAL(wch))
		||  (fPKRestrict && !in_PK(ch, wch))
		||  (fTattoo
		   && (vnum == get_eq_char(wch,WEAR_TATTOO)->pIndexData->vnum)))
			continue;

		nMatch++;
		do_who_raw(ch, wch, output);
	}

	count = 0;
	for (d = descriptor_list; d != NULL; d = d->next)
		count += (d->connected == CON_PLAYING);

	max_on = UMAX(count, max_on);
	buf_printf(output, msg(MSG_PLAYERS_FOUND, ch), nMatch, max_on);
	page_to_char(buf_string(output), ch);
	buf_free(output);
}

/* whois command */
void do_whois(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	BUFFER *output = NULL;
	DESCRIPTOR_DATA *d;

	one_argument(argument, arg);
	if (arg[0] == '\0') {
		char_nputs(MSG_MUST_PROVIDE_NAME, ch);
		return;
	}

	for (d = descriptor_list; d != NULL; d = d->next) {
		CHAR_DATA *wch;

		if (d->connected != CON_PLAYING || !can_see(ch,d->character))
				continue;

		if (d->connected != CON_PLAYING
		||  (IS_VAMPIRE(d->character)
		     && !IS_IMMORTAL(ch) && (ch != d->character)))
				continue;

		wch = (d->original != NULL) ? d->original : d->character;

		if (!can_see(ch,wch))
			continue;

		if (!str_prefix(arg,wch->name)) {
			if (output == NULL)
				output = buf_new(0);
			do_who_raw(ch, wch, output);
		}
	}

	if (output == NULL) {
		char_nputs(MSG_NO_ONE_THAT_NAME, ch);
		return;
	}

	page_to_char(buf_string(output), ch);
	buf_free(output);
}

void do_count(CHAR_DATA *ch, const char *argument)
{
	int count;
	DESCRIPTOR_DATA *d;

	count = 0;

	for (d = descriptor_list; d != NULL; d = d->next)
		if (d->connected == CON_PLAYING && can_see(ch, d->character))
			count++;

	max_on = UMAX(count,max_on);

	char_printf(ch, msg(MSG_D_CHARS_ON, ch), count);
	if (max_on == count)
		char_puts(msg(MSG_MOST_SO_FAR_TODAY, ch), ch);
	else
		char_printf(ch, msg(MSG_MOST_TODAY_WAS, ch), max_on);
	char_puts(".\n\r", ch);
}

void do_inventory(CHAR_DATA *ch, const char *argument)
{
	send_to_char(msg(MSG_YOU_ARE_CARRYING, ch), ch);
	show_list_to_char(ch->carrying, ch, TRUE, TRUE);
}

void do_equipment(CHAR_DATA *ch, const char *argument)
{
	OBJ_DATA *obj;
	int i;
	bool found;

	send_to_char(msg(MSG_YOU_ARE_USING, ch), ch);
	found = FALSE;
	for (i = 0; show_order[i] >= 0; i++) {
		if ((obj = get_eq_char(ch, show_order[i])) == NULL)
			continue;

		send_to_char(msg(MSG_EQ_USED_AS_LIGHT + show_order[i], ch), ch);
		if (can_see_obj(ch, obj))
			char_printf(ch, "%s\n\r",
				    format_obj_to_char(obj, ch, TRUE));
		else
			send_to_char(msg(MSG_SOMETHING, ch), ch);
		found = TRUE;
	}

	for(obj = ch->carrying; obj != NULL; obj = obj->next_content) {
		if (obj->wear_loc != WEAR_STUCK_IN)
			continue;

		send_to_char(msg(MSG_EQ_STUCK_IN, ch), ch);
		if (can_see_obj(ch, obj))
			char_printf(ch, "%s\n\r",
				    format_obj_to_char(obj, ch, TRUE));
		else
			send_to_char(msg(MSG_SOMETHING, ch), ch);
		found = TRUE;
	}

	if (!found)
		char_nputs(MSG_NOTHING, ch);
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

	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);
	if (arg1[0] == '\0') {
		send_to_char(msg(MSG_COMPARE_WHAT, ch), ch);
		return;
	}

	if ((obj1 = get_obj_carry(ch, arg1)) == NULL) {
		send_to_char(msg(MSG_DONT_HAVE_ITEM, ch), ch);
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
			send_to_char(msg(MSG_ARENT_WEAR_COMPARABLE, ch), ch);
			return;
		}
	}
	else if ((obj2 = get_obj_carry(ch,arg2)) == NULL) {
		send_to_char(msg(MSG_DONT_HAVE_ITEM, ch), ch);
		return;
	}

	cmsg		= NULL;
	value1	= 0;
	value2	= 0;

	if (obj1 == obj2)
		cmsg = msg(MSG_COMPARE_P_TO_ITSELF, ch);
	else if (obj1->item_type != obj2->item_type)
		cmsg = msg(MSG_CANT_COMPARE_P_P, ch);
	else {
		switch (obj1->item_type) {
		default:
			cmsg = msg(MSG_CANT_COMPARE_P_P, ch);
			break;

		case ITEM_ARMOR:
			value1 = obj1->value[0]+obj1->value[1]+obj1->value[2];
			value2 = obj2->value[0]+obj2->value[1]+obj2->value[2];
			break;

		case ITEM_WEAPON:
			if (obj1->pIndexData->new_format)
				value1 = (1 + obj1->value[2]) * obj1->value[1];
			else
				value1 = obj1->value[1] + obj1->value[2];

			if (obj2->pIndexData->new_format)
				value2 = (1 + obj2->value[2]) * obj2->value[1];
			else
				value2 = obj2->value[1] + obj2->value[2];
			break;
		}
	}

	if (cmsg == NULL)
		if (value1 == value2)
			cmsg = msg(MSG_P_P_LOOKS_SAME, ch);
		else if (value1  > value2)
			cmsg = msg(MSG_P_LOOKS_BETTER_P, ch);
		else
			cmsg = msg(MSG_P_LOOKS_WORSE_P, ch);

	act(cmsg, ch, obj1, obj2, TO_CHAR);
}

void do_credits(CHAR_DATA *ch, const char *argument)
{
	do_help(ch, "muddy");
}

void do_where(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	DESCRIPTOR_DATA *d;
	bool found;
	bool fPKonly = FALSE;

	one_argument(argument, arg);

	if (!check_blind(ch))
		return;

	if (room_is_dark(ch) && !IS_SET(ch->act, PLR_HOLYLIGHT)) {
		send_to_char(msg(MSG_TOO_DARK_SEE, ch), ch);
		return;
	}

	if (!str_cmp(arg,"pk"))
		fPKonly = TRUE;

	if (arg[0] == '\0' || fPKonly) {
		send_to_char(msg(MSG_PLAYERS_NEAR_YOU, ch), ch);
		found = FALSE;
		for (d = descriptor_list; d; d = d->next) {
			if (d->connected == CON_PLAYING
			&&  (victim = d->character) != NULL
			&&  !IS_NPC(victim)
			&&  !(fPKonly && in_PK(ch, victim))
			&&  victim->in_room != NULL
			&&  victim->in_room->area == ch->in_room->area
			&&  can_see(ch, victim)) {
				found = TRUE;
				char_printf(ch, "%s%-28s %s\n\r",
					(in_PK(ch, victim) &&
					!IS_IMMORTAL(ch)) ?
					"{r[{RPK{r]{x " : "     ",
					PERS(victim, ch),
					mlstr_cval(victim->in_room->name, ch));
			}
		}
		if (!found)
			send_to_char(msg(MSG_NONE_DOT, ch), ch);
	}
	else {
		found = FALSE;
		for (victim = char_list;
		     victim != NULL; victim = victim->next) {
			if (victim->in_room != NULL
			&&  victim->in_room->area == ch->in_room->area
			&&  can_see(ch, victim)
			&&  is_name(arg, victim->name)) {
				found = TRUE;
				char_printf(ch, "%-28s %s\n\r",
					PERS(victim, ch),
					mlstr_cval(victim->in_room->name, ch));
				break;
			}
		}
		if (!found)
			act_nprintf(ch, NULL, arg, TO_CHAR, POS_DEAD,
					MSG_DIDNT_FIND_ANY);
	}
}

void do_consider(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	char *cmsg;
	char *align;
	int diff;

	one_argument(argument, arg);

	if (arg[0] == '\0') {
		send_to_char(msg(MSG_CONSIDER_WHOM, ch), ch);
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL) {
		send_to_char(msg(MSG_THEY_ARENT_HERE, ch), ch);
		return;
	}

	if (is_safe(ch, victim)) {
		send_to_char(msg(MSG_DONT_EVEN_THINK, ch), ch);
		return;
	}

	diff = victim->level - ch->level;

	     if (diff <= -10) cmsg = msg(MSG_CAN_KILL_NAKED, ch);
	else if (diff <=  -5) cmsg = msg(MSG_IS_NOT_MATCH_FOR_YOU, ch);
	else if (diff <=  -2) cmsg = msg(MSG_LOOKS_EASY_KILL, ch);
	else if (diff <=   1) cmsg = msg(MSG_PERFECT_MATCH, ch);
	else if (diff <=   4) cmsg = msg(MSG_FEEL_LUCKY_PUNK, ch);
	else if (diff <=   9) cmsg = msg(MSG_LAUGHS_AT_YOU_MERCILESSLY, ch);
	else		      cmsg = msg(MSG_DEATH_WILL_THANK_YOU, ch);

	if (IS_EVIL(ch) && IS_EVIL(victim))
		align = msg(MSG_GRINS_EVILLY_WITH, ch);
	else if (IS_GOOD(victim) && IS_GOOD(ch))
		align = msg(MSG_GREETS_YOU_WARMLY, ch);
	else if (IS_GOOD(victim) && IS_EVIL(ch))
		align = msg(MSG_HOPE_YOU_WILL_TURN, ch);
	else if (IS_EVIL(victim) && IS_GOOD(ch))
		align = msg(MSG_GRINS_EVILLY_AT, ch);
	else if (IS_NEUTRAL(ch) && IS_EVIL(victim))
		align = msg(MSG_GRINS_EVILLY, ch);
	else if (IS_NEUTRAL(ch) && IS_GOOD(victim))
		align = msg(MSG_SMILES_HAPPILY, ch);
	else if (IS_NEUTRAL(ch) && IS_NEUTRAL(victim))
		align = msg(MSG_LOOKS_AS_YOU, ch);
	else
		align = msg(MSG_LOOKS_VERY_DISINTERESTED, ch);

	act(cmsg, ch, NULL, victim, TO_CHAR);
	act(align, ch, NULL, victim, TO_CHAR);
	return;
}

void set_title(CHAR_DATA *ch, const char *title)
{
	char buf[MAX_TITLE_LENGTH];
	static char nospace[] = ".,!?':";

	buf[0] = '\0';

	if (title) {
		if (strchr(nospace, *cstrfirst(title)) == NULL) {
			buf[0] = ' ';
			buf[1] = '\0';
		}

		strnzcat(buf, title, sizeof(buf));
		smash_tilde(buf);
	}

	free_string(ch->pcdata->title);
	ch->pcdata->title = str_dup(buf);
}

void do_title(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch))
		return;

	if (CANT_CHANGE_TITLE(ch)) {
		char_nputs(MSG_CANT_CHANGE_TITLE, ch);
		return;
	}

	if (argument[0] == '\0') {
		char_nputs(MSG_CHANGE_TITLE_TO_WHAT, ch);
		return;
	}

	if (strstr(argument, "{/")) {
		char_nputs(MSG_ILLEGAL_CHARACTER_TITLE, ch);
		return;
	}
		
	set_title(ch, argument);
	char_nputs(MSG_OK, ch);
}

void do_description(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_STRING_LENGTH];

	one_argument(argument, arg);

	if (str_cmp(arg, "edit") == 0) {
		string_append(ch, mlstr_convert(&ch->description, -1));
		return;
	}

	char_nprintf(ch, MSG_YOUR_DESC_IS, mlstr_mval(ch->description));
}

void do_report(CHAR_DATA *ch, const char *argument)
{
	act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING, MSG_REPORT_I_HAVE,
	       ch->hit, ch->max_hit,
	       ch->mana, ch->max_mana,
	       ch->move, ch->max_move);
	act_nprintf(ch, NULL, NULL, TO_CHAR, POS_DEAD, MSG_REPORT_I_HAVE_TO_CH,
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

	if ((ch->class == CLASS_SAMURAI) && (ch->level >=10)) {
		char_printf(ch, "You don't deal with wimpies, "
				"or such feary things.\n\r");
		if (ch->wimpy != 0)
			ch->wimpy = 0;
		return;
	}

	one_argument(argument, arg);

	if (arg[0] == '\0')
		wimpy = ch->max_hit / 5;
	else
		wimpy = atoi(arg);

	if (wimpy < 0) {
		send_to_char("Your courage exceeds your wisdom.\n\r", ch);
		return;
	}

	if (wimpy > ch->max_hit/2) {
		send_to_char("Such cowardice ill becomes you.\n\r", ch);
		return;
	}

	ch->wimpy	= wimpy;

	char_printf(ch, "Wimpy set to %d hit points.\n\r", wimpy);
	return;
}

void do_password(CHAR_DATA *ch, const char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char *pwdnew;

	if (IS_NPC(ch))
		return;

	argument = first_arg(argument, arg1, FALSE);
	argument = first_arg(argument, arg2, FALSE);

	if (arg1[0] == '\0' || arg2[0] == '\0') {
		send_to_char("Syntax: password <old> <new>.\n\r", ch);
		return;
	}

	if (strcmp(crypt(arg1, ch->pcdata->pwd), ch->pcdata->pwd)) {
		WAIT_STATE(ch, 40);
		send_to_char("Wrong password.  Wait 10 seconds.\n\r", ch);
		return;
	}

	if (strlen(arg2) < 5) {
		send_to_char("New password must be at least "
			     "five characters long.\n\r", ch);
		return;
	}

	/*
	 * No tilde allowed because of player file format.
	 */
	pwdnew = crypt(arg2, ch->name);
	if (strchr(pwdnew, '~') != NULL) {
		send_to_char("New password not acceptable, "
			     "try again.\n\r", ch);
		return;
	}

	free_string(ch->pcdata->pwd);
	ch->pcdata->pwd = str_dup(pwdnew);
	save_char_obj(ch, FALSE);
	char_nputs(MSG_OK, ch);
	return;
}

/* RT configure command */

void do_scan(CHAR_DATA *ch, const char *argument)
{
	char dir[MAX_INPUT_LENGTH];
	char *dir2;
	ROOM_INDEX_DATA *in_room;
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *exit;	/* pExit */
	int door;
	int range;
	int i;
	CHAR_DATA *person;
	int numpeople;

	one_argument(argument,dir);

	if (dir[0] == '\0') {
		do_scan2(ch, "");
		return;
	}

	switch (dir[0]) {
	case 'N':
	case 'n':
		door = 0;
		dir2 = "north";
		break;
	case 'E':
	case 'e':
		door = 1;
		dir2 = "east";
		break;
	case 'S':
	case 's':
		door = 2;
		dir2 = "south";
		break;
	case 'W':
	case 'w':
		door = 3;
		dir2 = "west";
		break;
	case 'U':
	case 'u':
		door = 4;
		dir2 = "up";
		break;
	case 'D':
	case 'd':
		door = 5;
		dir2 = "down";
		break;
	default:
		send_to_char(msg(MSG_WRONG_DIRECTION, ch), ch);
		return;
	}

	char_printf(ch, msg(MSG_YOU_SCAN_S, ch), dir2);
	act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING, MSG_N_SCANS_S, dir2);

	if (!check_blind(ch))
		return;

	range = 1 + (ch->level)/10;

	in_room = ch->in_room;
	for (i=1; i <= range; i++) {
		exit = in_room->exit[door];
		if (!exit)
			return;
		to_room = exit->u1.to_room;
		if (!to_room)
			return;

		if (IS_SET(exit->exit_info,EX_CLOSED)
		&&  can_see_room(ch,exit->u1.to_room)) {
			send_to_char(msg(MSG_SCAN_DOOR_CLOSED, ch), ch);
			return;
		}
		for (numpeople = 0, person = to_room->people; person != NULL;
		     person = person->next_in_room)
			if (can_see(ch,person)) {
				numpeople++;
				break;
			}

		if (numpeople) {
			char_printf(ch, "***** Range %d *****\n\r", i);
			show_char_to_char(to_room->people, ch);
			send_to_char("\n\r", ch);
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

	if (is_affected(ch, gsn_reserved)) {
		send_to_char("Wait for a while to request again.\n\r", ch);
		return;
	}

	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (IS_NPC(ch))
		return;

	if (arg1[0] == '\0' || arg2[0] == '\0') {
		send_to_char("Request what from whom?\n\r", ch);
		return;
	}

	if ((victim = get_char_room(ch, arg2)) == NULL) {
		send_to_char("They aren't here.\n\r", ch);
		return;
	}

	if (!IS_NPC(victim)) {
		send_to_char("Why don't you just ask the player?\n\r", ch);
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

	if (ch->carry_number + get_obj_number(obj) > can_carry_n(ch)) {
		send_to_char("Your hands are full.\n\r", ch);
		return;
	}

	if (ch->carry_weight + get_obj_weight(obj) > can_carry_w(ch)) {
		send_to_char("You can't carry that much weight.\n\r", ch);
		return;
	}

	if (!can_see_obj(ch, obj)) {
		act("You don't see that.", ch, NULL, victim, TO_CHAR);
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
	send_to_char("and for the goodness you have seen in the world.\n\r",ch);

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
	int i;
	int amount;

	if (IS_NPC(ch)) {
		send_to_char("You can't change your hometown!\n\r", ch);
		return;
	}

	if (ORG_RACE(ch) == 11 || ORG_RACE(ch) == 12
	||  ORG_RACE(ch) == 13 || ORG_RACE(ch) == 14) {
		send_to_char("Your hometown is permenantly Titan Valley!\n\r",
			     ch);
		return;
	}

	if (ch->class == CLASS_VAMPIRE || ch->class == CLASS_NECROMANCER) {
		send_to_char("Your hometown is permenantly Old Midgaard!\n\r",
			     ch);
		return;
	}

	if (!IS_SET(ch->in_room->room_flags, ROOM_REGISTRY)) {
		send_to_char("You have to be in the Registry "
			     "to change your hometown.\n\r", ch);
		return;
	}

	amount = (ch->level * ch->level * 250) + 1000;

	if (argument[0] == '\0') {
		char_printf(ch, "It will cost you %d gold.\n\r", amount);
		return;
	}

	if (ch->gold < amount) {
		send_to_char("You don't have enough money "
			     "to change hometowns!\n\r", ch);
		return;
	}

	for (i = 0; hometown_table[i].name; i++) {
		char* restrict_msg;

		if (str_prefix(argument, hometown_table[i].name))
			continue;

		if (ch->hometown == i) {
			char_printf(ch, "But you already live in %s!\n\r",
				    hometown_table[i].name);
			return;
		}

		restrict_msg = hometown_table[i].check_fn(ch);
		if (restrict_msg != NULL) {
			char_printf(ch, "%s.\n\r");
			return;
		}

		ch->gold -= amount;
		char_printf(ch, "Your hometown is changed to %s.\n\r",
			    hometown_table[i].name);
		ch->hometown = i;
		return;
	}

	send_to_char("That is not a valid choice.\n\r"
		     "Choose from", ch);
	for (i = 0; hometown_table[i].name; i++) {
		static char* comma = ", ";

		char_printf(ch, "%s%s", i == 0 ? comma+1 : comma,
			    hometown_table[i].name);
	}
	send_to_char(".\n\r", ch);
}

void do_detect_hidden(CHAR_DATA *ch, const char *argument)
{
	AFFECT_DATA	af;
	int		chance;
	int		sn;

	if ((sn = sn_lookup("detect hide")) < 0
	||  (chance = get_skill(ch, sn)) == 0) {
		char_nputs(MSG_HUH, ch);
		return;
	}

	if (IS_AFFECTED(ch, AFF_DETECT_HIDDEN)) {
		send_to_char("You are already as alert as you can be. \n\r",ch);
		return;
	}

	if (number_percent() > chance) {
		send_to_char("You peer intently at the shadows "
			     "but they are unrevealing.\n\r", ch);
		return;
	}

	af.where     = TO_AFFECTS;
	af.type      = sn;
	af.level     = ch->level;
	af.duration  = ch->level;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_DETECT_HIDDEN;
	affect_to_char(ch, &af);
	send_to_char("Your awareness improves.\n\r", ch);
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
		char_nputs(MSG_HUH, ch);
		return;
	}

	send_to_char("You call for bears help you.\n\r",ch);
	act("$n shouts a bear call.",ch,NULL,NULL,TO_ROOM);

	if (is_affected(ch, sn)) {
		send_to_char("You cannot summon the strength to handle "
			     "more bears right now.\n\r", ch);
		return;
	}

	for (gch = char_list; gch != NULL; gch = gch->next) {
		if (IS_NPC(gch) && IS_AFFECTED(gch,AFF_CHARM)
		&&  gch->master == ch
		&&  gch->pIndexData->vnum == MOB_VNUM_BEAR) {
			send_to_char("What's wrong with the bear you've got?",
				     ch);
			return;
		}
	}

	if (ch->in_room != NULL
	&&  IS_SET(ch->in_room->room_flags, ROOM_NOMOB)) {
		send_to_char("No bears listen you.\n\r", ch);
		return;
	}

	if (IS_SET(ch->in_room->room_flags, ROOM_SAFE)
	||  IS_SET(ch->in_room->room_flags, ROOM_PRIVATE)
	||  IS_SET(ch->in_room->room_flags, ROOM_SOLITARY)
	||  (ch->in_room->exit[0] == NULL && ch->in_room->exit[1] == NULL
	&&   ch->in_room->exit[2] == NULL && ch->in_room->exit[3] == NULL
	&&   ch->in_room->exit[4] == NULL && ch->in_room->exit[5] == NULL)
	||  (ch->in_room->sector_type != SECT_FIELD
	&&   ch->in_room->sector_type != SECT_FOREST
	&&   ch->in_room->sector_type != SECT_MOUNTAIN
	&&   ch->in_room->sector_type != SECT_HILLS)) {
		send_to_char("No bears come to your rescue.\n\r", ch);
		return;
	}

	mana = SKILL(sn)->min_mana;
	if (ch->mana < mana) {
		send_to_char("You don't have enough mana "
			     "to shout a bear call.\n\r", ch);
		return;
	}
	ch->mana -= mana;

	if (number_percent() > chance) {
		send_to_char("No bears listen you.\n\r", ch);
		check_improve(ch, sn, FALSE, 1);
		return;
	}

	check_improve(ch, sn, TRUE, 1);
	bear = create_mob(get_mob_index(MOB_VNUM_BEAR));

	for (i=0;i < MAX_STATS; i++)
		bear->perm_stat[i] = UMIN(25,2 * ch->perm_stat[i]);

	bear->max_hit = IS_NPC(ch) ? ch->max_hit : ch->pcdata->perm_hit;
	bear->hit = bear->max_hit;
	bear->max_mana = IS_NPC(ch) ? ch->max_mana : ch->pcdata->perm_mana;
	bear->mana = bear->max_mana;
	bear->alignment = ch->alignment;
	bear->level = UMIN(100, 1 * ch->level-2);
	for (i=0; i < 3; i++)
		bear->armor[i] = interpolate(bear->level, 100, -100);
	bear->armor[3] = interpolate(bear->level, 100, 0);
	bear->sex = ch->sex;
	bear->gold = 0;

	bear2 = create_mob(bear->pIndexData);
	clone_mob(bear, bear2);

	SET_BIT(bear->affected_by, AFF_CHARM);
	SET_BIT(bear2->affected_by, AFF_CHARM);
	bear->master = bear2->master = ch;
	bear->leader = bear2->leader = ch;

	char_to_room(bear, ch->in_room);
	char_to_room(bear2, ch->in_room);
	send_to_char("Two bears come to your rescue!\n\r",ch);
	act("Two bears come to $n's rescue!",ch,NULL,NULL,TO_ROOM);

	af.where	      = TO_AFFECTS;
	af.type 	      = sn;
	af.level	      = ch->level;
	af.duration	      = SKILL(sn)->beats;
	af.bitvector	      = 0;
	af.modifier	      = 0;
	af.location	      = APPLY_NONE;
	affect_to_char(ch, &af);
}

void do_identify(CHAR_DATA *ch, const char *argument)
{
	OBJ_DATA *obj;
	CHAR_DATA *rch;

	if ((obj = get_obj_carry(ch, argument)) == NULL) {
		 send_to_char("You are not carrying that.\n\r", ch);
		 return;
	}

	for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
		if (IS_NPC(rch) && rch->pIndexData->vnum == MOB_VNUM_SAGE)
			break;

	if (!rch) {
		 send_to_char("No one here seems to know much "
			      "about that.\n\r", ch);
		 return;
	}

	if (IS_IMMORTAL(ch))
		act("$n looks at you!", rch, obj, ch, TO_VICT);
	else if (ch->gold < 1) {
		act("$n resumes to identify by looking at $p.",
		       rch, obj, 0, TO_ROOM);
		send_to_char(" You need at least 1 gold.\n\r", ch);
		return;
	}
	else {
		ch->gold -= 1;
		send_to_char("Your purse feels lighter.\n\r", ch);
	}

	act("$n gives a wise look at $p.", rch, obj, 0, TO_ROOM);
	spell_identify(0, 0, ch, obj ,0);
}

void do_score(CHAR_DATA *ch, const char *argument)
{
	char buf2[MAX_INPUT_LENGTH];
	char title[MAX_STRING_LENGTH];
	int ekle = 0;
	int delta;
	CLASS_DATA *cl;
	BUFFER *output;

	if ((cl = class_lookup(ch->class)) == NULL)
		return;

	output = buf_new(0);
	buf_add(output, "\n\r      {G/~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~/~~\\{x\n\r");

	strnzcpy(title, IS_NPC(ch) ? "Believer of Chronos." : ch->pcdata->title,
		 sizeof(title));
	delta = strlen(title) - cstrlen(title);
	title[32+delta] = '\0';
	snprintf(buf2, sizeof(buf2), "     {G|   {W%%-12s{x%%-%ds {Y%%3d years old   {G|____|{x\n\r", 33+delta);
	buf_printf(output, buf2, ch->name, title, get_age(ch));

	buf_add(output, "     {G|{C+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+{G|{x\n\r");

	buf_printf(output, "     {G| {RLevel:  {x%3d          {C|  {RStr:  {x%2d(%2d)  {C| {RReligion  :  {x%-10s {G|{x\n\r",
		   ch->level,
		   ch->perm_stat[STAT_STR],
		   get_curr_stat(ch,STAT_STR),
		   religion_name(ch->religion));

	buf_printf(output,
"     {G| {RRace :  {x%-11s  {C|  {RInt:  {x%2d(%2d)  {C| {RPractice  :   {x%3d       {G|{x\n\r",
		race_table[ORG_RACE(ch)].name,
		ch->perm_stat[STAT_INT],
		get_curr_stat(ch, STAT_INT),
		ch->practice);

	buf_printf(output,
"     {G| {RSex  :  {x%-11s  {C|  {RWis:  {x%2d(%2d)  {C| {RTrain     :   {x%3d       {G|{x\n\r",
		   ch->sex == 0 ? "sexless" : ch->sex == 1 ? "male" : "female",
		   ch->perm_stat[STAT_WIS],
		   get_curr_stat(ch,STAT_WIS),
		   ch->train);

	buf_printf(output,
"     {G| {RClass:  {x%-12s {C|  {RDex:  {x%2d(%2d)  {C| {RQuest Pnts:  {x%4d       {G|{x\n\r",
		IS_NPC(ch) ? "mobile" : cl->name,
		ch->perm_stat[STAT_DEX], get_curr_stat(ch,STAT_DEX),
		IS_NPC(ch) ? 0 : ch->pcdata->questpoints);

	buf_printf(output,
"     {G| {RHome :  {x%-12s {C|  {RCon:  {x%2d(%2d)  {C| {R%-10s:   {x%3d       {G|{x\n\r",
		IS_NPC(ch) ? "Midgaard" : hometown_table[ch->hometown].name,
		ch->perm_stat[STAT_CON], get_curr_stat(ch,STAT_CON),
		IS_NPC(ch) ? "Quest?" : (IS_ON_QUEST(ch) ? "Quest Time" : "Next Quest"),
		IS_NPC(ch) ? 0 : abs(ch->pcdata->questtime));
	buf_printf(output,
"     {G| {REthos:  {x%-11s  {C|  {RCha:  {x%2d(%2d)  {C| {R%s     :  {x%4d       {G|{x\n\r",
		IS_NPC(ch) ? "mobile" : ch->ethos == 1 ? "lawful" :
	ch->ethos == 2 ? "neutral" : ch->ethos == 3 ? "chaotic" : "none",
		ch->perm_stat[STAT_CHA], get_curr_stat(ch,STAT_CHA),
		ch->class == CLASS_SAMURAI ? "Death" : "Wimpy" ,
		ch->class == CLASS_SAMURAI ? ch->pcdata->death : ch->wimpy);

	buf_printf(output, "     {G| {RAlign:  {x%-11s  {C|                |{x %-7s %-19s {G|{x\n\r",
		IS_GOOD(ch) ? "good" : IS_EVIL(ch) ? "evil" : "neutral",
		msg(MSG_YOU_ARE, ch),
		msg(MSG_POS_NAME_DEAD + ch->position, ch));

	buf_add(output, "     {G|{C+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+{G|{x{x\n\r");

	if (ch->guarding != NULL) {
		ekle = 1;
		buf_printf(output,
"     {G| {GYou are guarding: {x%-10s                                    {G|{x\n\r",
			    ch->guarding->name);
	}

	if (ch->guarded_by != NULL) {
		ekle = 1;
		buf_printf(output,
"     {G| {GYou are guarded by: {x%-10s                                  {G|{x\n\r",
			    ch->guarded_by->name);
	}

	if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10) {
		ekle = 1;
		buf_printf(output,
"     {G| {GYou are drunk.                                                  {G|{x\n\r");
	}

	if (!IS_NPC(ch) && ch->pcdata->condition[COND_THIRST] <= 0) {
		ekle = 1;
		buf_printf(output,
"     {G| {YYou are thirsty.                                                {G|{x\n\r");
	}
/*    if (!IS_NPC(ch) && ch->pcdata->condition[COND_FULL]   ==	0) */
	if (!IS_NPC(ch) && ch->pcdata->condition[COND_HUNGER] <= 0) {
		ekle = 1;
		buf_printf(output,
"     {G| {YYou are hungry.                                                 {G|{x\n\r");
	}

	if (!IS_NPC(ch) && IS_SET(ch->act, PLR_GHOST)) {
		ekle = 1;
		buf_add(output,
"     {G| {cYou are ghost.                                                  {G|{x\n\r");
	}

	if (!IS_NPC(ch) && ch->pcdata->condition[COND_BLOODLUST] <= 0) {
		ekle = 1;
		buf_printf(output,
"     {G| {YYou are hungry for blood.                                       {G|{x\n\r");
	}

	if (!IS_NPC(ch) && ch->pcdata->condition[COND_DESIRE] <=  0) {
		ekle = 1;
		buf_printf(output,
"     {G| {YYou are desiring your home.                                     {G|{x\n\r");
	}

	if (!IS_IMMORTAL(ch) && IS_PUMPED(ch)) {
		ekle = 1;
		buf_printf(output,
"     {G| {RYour adrenalin is gushing!                                      {G|{x\n\r");
	}

	if (ekle)
		buf_add(output,
"     {G|{C+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+{G|{x\n\r");

	buf_printf(output,
"     {G| {RItems Carried :   {x%2d/%-4d           {RArmor vs magic  : {x%4d      {G|{x\n\r",
		ch->carry_number, can_carry_n(ch),
		GET_AC(ch,AC_EXOTIC));

	buf_printf(output,
"     {G| {RWeight Carried:  {x%4d/%-8d      {RArmor vs bash   : {x%4d      {G|{x\n\r",
	get_carry_weight(ch), can_carry_w(ch),GET_AC(ch,AC_BASH));

	buf_printf(output,
"     {G| {RGold          :   {Y%-10ld        {RArmor vs pierce : {x%4d      {G|{x\n\r",
		 ch->gold,GET_AC(ch,AC_PIERCE));

	buf_printf(output,
"     {G| {RSilver        :   {W%-10ld        {RArmor vs slash  : {x%4d      {G|{x\n\r",
		 ch->silver,GET_AC(ch,AC_SLASH));

	buf_printf(output,
"     {G| {RCurrent exp   :   {x%-6d            {RSaves vs Spell  : {x%4d      {G|{x\n\r",
		ch->exp,ch->saving_throw);

	buf_printf(output,
"     {G| {RExp to level  :   {x%-6d                                        {G|{x\n\r",
		IS_NPC(ch) ? 0 : exp_to_level(ch));

	buf_printf(output,
"     {G|                                     {RHitP: {x%5d / %5d         {G|{x\n\r",
		   ch->hit, ch->max_hit);
	buf_printf(output,
"     {G| {RHitroll       :   {x%-3d               {RMana: {x%5d / %5d         {G|{x\n\r",
		   GET_HITROLL(ch),ch->mana, ch->max_mana);
	buf_printf(output,
"     {G| {RDamroll       :   {x%-3d               {RMove: {x%5d / %5d         {G|{x\n\r",
		    GET_DAMROLL(ch), ch->move, ch->max_move);
	buf_add(output, "  {G/~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~/   |{x\n\r");
	buf_add(output, "  {G\\________________________________________________________________\\__/{x\n\r");

	if (IS_SET(ch->comm, COMM_SHOW_AFFECTS))
		show_affects(ch, output);
	page_to_char(buf_string(output), ch);
	buf_free(output);
}

void do_oscore(CHAR_DATA *ch, const char *argument)
{
	CLASS_DATA *cl;
	char buf2[MAX_STRING_LENGTH];
	int i;
	BUFFER *output;

	if ((cl = class_lookup(ch->class)) == NULL)
		return;

	output = buf_new(0);

	buf_printf(output,
		"%s {W%s{x%s, level {c%d{x, {c%d{x years old "
		"(%d hours).\n\r",
		msg(MSG_YOU_ARE, ch),
		ch->name,
		IS_NPC(ch) ? "" : ch->pcdata->title, ch->level, get_age(ch),
		(ch->played + (int) (current_time - ch->logon)) / 3600);

	if (get_trust(ch) != ch->level)
		buf_printf(output, "You are trusted at level %d.\n\r", get_trust(ch));

	buf_printf(output,
		"Race: {c%s{x  Sex: {c%s{x  Class: {c%s{x  "
		"Hometown: {c%s{x\n\r",
		race_table[ORG_RACE(ch)].name,
		ch->sex == 0 ? "sexless" : ch->sex == 1 ? "male" : "female",
		IS_NPC(ch) ? "mobile" : cl->name,
		IS_NPC(ch) ? "Midgaard" : hometown_table[ch->hometown].name);

	buf_printf(output,
		"You have {c%d{x/{c%d{x hit, {c%d{x/{c%d{x mana, "
		"{c%d{x/{c%d{x movement.\n\r",
		ch->hit, ch->max_hit, ch->mana, ch->max_mana,
		ch->move, ch->max_move);

	buf_printf(output,
		"You have {c%d{x practices and "
		"{c%d{x training sessions.\n\r",
		ch->practice, ch->train);

	buf_printf(output, "You are carrying {c%d{x/{c%d{x items "
		"with weight {c%ld{x/{c%d{x pounds.\n\r",
		ch->carry_number, can_carry_n(ch),
		get_carry_weight(ch), can_carry_w(ch));

	if (ch->level > 20 || IS_NPC(ch))
		buf_printf(output,
			"Str: {c%d{x({c%d{x)  Int: {c%d{x({c%d{x)  "
			"Wis: {c%d{x({c%d{x)  Dex: {c%d{x({c%d{x)  "
			"Con: {c%d{x({c%d{x)  Cha: {c%d{x({c%d{x)\n\r",
			ch->perm_stat[STAT_STR], get_curr_stat(ch, STAT_STR),
			ch->perm_stat[STAT_INT], get_curr_stat(ch, STAT_INT),
			ch->perm_stat[STAT_WIS], get_curr_stat(ch, STAT_WIS),
			ch->perm_stat[STAT_DEX], get_curr_stat(ch, STAT_DEX),
			ch->perm_stat[STAT_CON], get_curr_stat(ch, STAT_CON),
			ch->perm_stat[STAT_CHA], get_curr_stat(ch, STAT_CHA));
	else
		buf_printf(output,
			"Str: {c%-9s{x Wis: {c%-9s{x Con: {c%-9s{x\n\r"
			"Int: {c%-9s{x Dex: {c%-9s{x Cha: {c%-11s{x\n\r",
			get_stat_alias(ch, STAT_STR),
			get_stat_alias(ch, STAT_WIS),
			get_stat_alias(ch, STAT_CON),
			get_stat_alias(ch, STAT_INT),
			get_stat_alias(ch, STAT_DEX),
			get_stat_alias(ch, STAT_CHA));

	snprintf(buf2, sizeof(buf2),
		 "You have scored {c%d{x exp, and have %s%s%s.\n\r",
		 ch->exp,
		 ch->gold + ch->silver == 0 ? "no money" :
					      ch->gold ? "{Y%ld gold{x " : "",
		 ch->silver ? "{W%ld silver{x " : "",
		 ch->gold + ch->silver ? ch->gold + ch->silver == 1 ?
					"coin" : "coins" : "");
	if (ch->gold)
		buf_printf(output, buf2, ch->gold, ch->silver);
	else
		buf_printf(output, buf2, ch->silver);

	/* KIO shows exp to level */
	if (!IS_NPC(ch) && ch->level < LEVEL_HERO)
		buf_printf(output, "You need {c%d{x exp to level.\n\r",
			exp_to_level(ch));

	if (!IS_NPC(ch))
		buf_printf(output,
			"Quest Points: {c%d{x.  "
			"%s: {c%d{x.\n\r",
			ch->pcdata->questpoints, 
			IS_NPC(ch) ? "Quest?" : (IS_ON_QUEST(ch) ? 
					"Quest Time" : "Next Quest"),
			IS_NPC(ch) ? 0 : abs(ch->pcdata->questtime));

	if ((ch->class == CLASS_SAMURAI) && (ch->level >= 10))
		buf_printf(output, "Total {c%d{x deaths up to now.",
			ch->pcdata->death);
	else
		buf_printf(output, "Wimpy set to {c%d{x hit points.", ch->wimpy);

	if (ch->guarding != NULL)
		buf_printf(output, "  You are guarding: {W%s{x", ch->guarding->name);

	if (ch->guarded_by != NULL)
		buf_printf(output, "  You are guarded by: {W%s{x",
			ch->guarded_by->name);
	buf_add(output, "\n\r");

	if (!IS_NPC(ch)) {
		if (ch->pcdata->condition[COND_DRUNK] > 10)
			buf_add(output, "You are {cdrunk{x.\n\r");

		if (ch->pcdata->condition[COND_THIRST] <= 0)
			buf_add(output, "You are {rthirsty{x.\n\r");

/*		if (ch->pcdata->condition[COND_FULL] == 0) */
		if (ch->pcdata->condition[COND_HUNGER] <= 0)
			buf_add(output, "You are {rhungry{x.\n\r");
		if (ch->pcdata->condition[COND_BLOODLUST] <= 0)
			buf_add(output, "You are {rhungry for {Rblood{x.\n\r");
		if (ch->pcdata->condition[COND_DESIRE] <= 0)
			buf_add(output, "You are {rdesiring your home{x.\n\r");
		if (IS_SET(ch->act, PLR_GHOST))
			buf_add(output, "You are {cghost{x.\n\r");
	}

	buf_printf(output, "%s %s.\n\r",
		msg(MSG_YOU_ARE, ch),
		msg(MSG_POS_NAME_DEAD + ch->position, ch));

	if ((ch->position == POS_SLEEPING || ch->position == POS_RESTING ||
	     ch->position == POS_FIGHTING || ch->position == POS_STANDING)
	&& !IS_IMMORTAL(ch) && IS_PUMPED(ch))
		buf_add(output, "Your {radrenalin is gushing{x!\n\r");

	/* print AC values */
	if (ch->level >= 25) {
		buf_printf(output,
			   "Armor: pierce: {c%d{x  bash: {c%d{x  "
			   "slash: {c%d{x  magic: {c%d{x\n\r",
			   GET_AC(ch, AC_PIERCE), GET_AC(ch, AC_BASH),
			   GET_AC(ch, AC_SLASH), GET_AC(ch, AC_EXOTIC));

		buf_printf(output,
			   "Saves vs. spell: {c%d{x\n\r",
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
					   "{chopelessly vulnerable{x to %s.\n\r",
					   ac_name[i]);
			else if (GET_AC(ch,i) >= 80)
				buf_printf(output,
					   "{cdefenseless against{x %s.\n\r",
					   ac_name[i]);
			else if (GET_AC(ch,i) >= 60)
				buf_printf(output, "{cbarely protected{x from %s.\n\r",
					   ac_name[i]);
			else if (GET_AC(ch,i) >= 40)
				buf_printf(output, "{cslightly armored{x against %s.\n\r",
					   ac_name[i]);
			else if (GET_AC(ch,i) >= 20)
				buf_printf(output, "{csomewhat armored{x against %s.\n\r",
					   ac_name[i]);
			else if (GET_AC(ch,i) >= 0)
				buf_printf(output, "{carmored{x against %s.\n\r",
					   ac_name[i]);
			else if (GET_AC(ch,i) >= -20)
				buf_printf(output, "{cwell-armored{x against %s.\n\r",
					   ac_name[i]);
			else if (GET_AC(ch,i) >= -40)
				buf_printf(output, "{cvery well-armored{x against %s.\n\r",
					   ac_name[i]);
			else if (GET_AC(ch,i) >= -60)
				buf_printf(output, "{cheavily armored{x against %s.\n\r",
					   ac_name[i]);
			else if (GET_AC(ch,i) >= -80)
				buf_printf(output, "{csuperbly armored{x against %s.\n\r",
					   ac_name[i]);
			else if (GET_AC(ch,i) >= -100)
				buf_printf(output, "{calmost invulnerable{x to %s.\n\r",
					   ac_name[i]);
			else
				buf_printf(output, "{cdivinely armored{x against %s.\n\r",
					   ac_name[i]);
		}
	}

	/* RT wizinvis and holy light */
	if (IS_IMMORTAL(ch)) {
		buf_printf(output, "Holy Light: %s",
			IS_SET(ch->act, PLR_HOLYLIGHT) ? "{con{x" : "{coff{x");

		if (ch->invis_level)
			buf_printf(output, "  Invisible: {clevel %d{x",
				ch->invis_level);

		if (ch->incog_level)
			buf_printf(output, "  Incognito: {clevel %d{x",
				ch->incog_level);
		buf_add(output, "\n\r");
	}

	if (ch->level >= 20)
		buf_printf(output, "Hitroll: {c%d{x  Damroll: {c%d{x.\n\r",
			GET_HITROLL(ch), GET_DAMROLL(ch));

	buf_add(output, "You are ");
	if (IS_GOOD(ch))
		buf_add(output, "good.");
	else if (IS_EVIL(ch))
		buf_add(output, "evil.");
	else
		buf_add(output, "neutral.");

	switch (ch->ethos) {
	case 1:
		buf_add(output, "  You have a lawful ethos.\n\r");
		break;
	case 2:
		buf_add(output, "  You have a neutral ethos.\n\r");
		break;
	case 3:
		buf_add(output, "  You have a chaotic ethos.\n\r");
		break;
	default:
		if (!IS_NPC(ch))
			buf_add(output, "  You have no ethos, "
				     "report it to the gods!\n\r");
	}

	if (IS_NPC(ch))
		ch->religion = 0; /* XXX */

	if ((ch->religion <= RELIGION_NONE) || (ch->religion > MAX_RELIGION))
		buf_add(output, "You don't believe any religion.\n\r");
	else
		buf_printf(output,"Your religion is the way of %s.\n\r",
			religion_table[ch->religion].leader);

	if (IS_SET(ch->comm, COMM_SHOW_AFFECTS))
		show_affects(ch, output);
	page_to_char(buf_string(output), ch);
	buf_free(output);
}

void show_affects(CHAR_DATA *ch, BUFFER *output)
{
	AFFECT_DATA *paf, *paf_last = NULL;

	if (ch->affected == NULL) {
		buf_add(output, msg(MSG_NOT_AFFECTED_SPELLS, ch));
		return;
	}

	buf_add(output, msg(MSG_YOU_ARE_AFFECTED, ch));
	for (paf = ch->affected; paf != NULL; paf = paf->next) {
		if (paf_last != NULL && paf->type == paf_last->type)
			if (ch->level >= 20)
				buf_add(output, "                      ");
			else
				continue;
		else
			buf_printf(output, "%s {c%-15s{x",
				   msg(MSG_AFF_SPELL, ch),
				   skill_name(paf->type));

		if (ch->level >= 20) {
			buf_printf(output, ": %s {c%s{x %s {c%d{x ",
				   msg(MSG_AFF_MODIFIES, ch),
				   flag_string(apply_flags, paf->location),
				   msg(MSG_AFF_BY, ch),
				   paf->modifier);
			if (paf->duration == -1 || paf->duration == -2)
				buf_add(output, msg(MSG_AFF_PERMANENTLY, ch));
			else
				buf_printf(output, msg(MSG_AFF_FOR_D_HOURS, ch),
					   paf->duration);
		}
		buf_add(output, "\n\r");
		paf_last = paf;
	}
}

void do_affects(CHAR_DATA *ch, const char *argument)
{
	BUFFER *output;

	output = buf_new(0);
	show_affects(ch, output);
	page_to_char(buf_string(output), ch);
	buf_free(output);
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
		char_nputs(MSG_HUH, ch);
		return;
	}

	send_to_char("You call for lions help you.\n\r",ch);
	act("$n shouts a lion call.",ch,NULL,NULL,TO_ROOM);

	if (is_affected(ch, sn)) {
		send_to_char("You cannot summon the strength to handle "
			     "more lions right now.\n\r", ch);
		return;
	}

	for (gch = char_list; gch != NULL; gch = gch->next) {
		if (IS_NPC(gch) && IS_AFFECTED(gch,AFF_CHARM)
		&&  gch->master == ch
		&& gch->pIndexData->vnum == MOB_VNUM_LION) {
			send_to_char("What's wrong with the lion "
				     "you've got?",ch);
			return;
		}
	}

	if (ch->in_room != NULL
	&& IS_SET(ch->in_room->room_flags, ROOM_NOMOB)) {
		send_to_char("No lions can listen you.\n\r", ch);
		return;
	}

	if (IS_SET(ch->in_room->room_flags, ROOM_SAFE)
	||  IS_SET(ch->in_room->room_flags, ROOM_PRIVATE)
	||  IS_SET(ch->in_room->room_flags, ROOM_SOLITARY)
	||  (ch->in_room->exit[0] == NULL && ch->in_room->exit[1] == NULL
	&&   ch->in_room->exit[2] == NULL && ch->in_room->exit[3] == NULL
	&&   ch->in_room->exit[4] == NULL && ch->in_room->exit[5] == NULL)
	||  (ch->in_room->sector_type != SECT_FIELD
	&&   ch->in_room->sector_type != SECT_FOREST
	&&   ch->in_room->sector_type != SECT_MOUNTAIN
	&&   ch->in_room->sector_type != SECT_HILLS)) {
		send_to_char("No lions come to your rescue.\n\r", ch);
		return;
	}

	mana = SKILL(sn)->min_mana;
	if (ch->mana < mana) {
		send_to_char("You don't have enough mana "
			     "to shout a lion call.\n\r", ch);
		return;
	}
	ch->mana -= mana;

	if (number_percent() > chance) {
		check_improve(ch, sn, FALSE, 1);
		send_to_char("No lions listen you.\n\r", ch);
		return;
	}

	check_improve(ch, sn, TRUE, 1);
	lion = create_mob(get_mob_index(MOB_VNUM_LION));

	for (i=0;i < MAX_STATS; i++)
		lion->perm_stat[i] = UMIN(25,2 * ch->perm_stat[i]);

	lion->max_hit = IS_NPC(ch) ? ch->max_hit : ch->pcdata->perm_hit;
	lion->hit = lion->max_hit;
	lion->max_mana = IS_NPC(ch) ? ch->max_mana : ch->pcdata->perm_mana;
	lion->mana = lion->max_mana;
	lion->alignment = ch->alignment;
	lion->level = UMIN(100,1 * ch->level-2);
	for (i=0; i < 3; i++)
		lion->armor[i] = interpolate(lion->level,100,-100);
	lion->armor[3] = interpolate(lion->level,100,0);
	lion->sex = ch->sex;
	lion->gold = 0;

	lion2 = create_mob(lion->pIndexData);
	clone_mob(lion,lion2);

	SET_BIT(lion->affected_by, AFF_CHARM);
	SET_BIT(lion2->affected_by, AFF_CHARM);
	lion->master = lion2->master = ch;
	lion->leader = lion2->leader = ch;

	char_to_room(lion,ch->in_room);
	char_to_room(lion2,ch->in_room);
	send_to_char("Two lions come to your rescue!\n\r",ch);
	act("Two lions come to $n's rescue!",ch,NULL,NULL,TO_ROOM);

	af.where	      = TO_AFFECTS;
	af.type 	      = sn;
	af.level	      = ch->level;
	af.duration	      = SKILL(sn)->beats;
	af.bitvector	      = 0;
	af.modifier	      = 0;
	af.location	      = APPLY_NONE;
	affect_to_char(ch, &af);
}

/* object condition aliases */
char *get_cond_alias(OBJ_DATA *obj, CHAR_DATA *ch)
{
	char *stat;
	int istat;

	istat = obj->condition;

	if	(istat >  99) stat = msg(MSG_COND_EXCELLENT, ch);
	else if (istat >= 80) stat = msg(MSG_COND_GOOD, ch);
	else if (istat >= 60) stat = msg(MSG_COND_FINE, ch);
	else if (istat >= 40) stat = msg(MSG_COND_AVERAGE, ch);
	else if (istat >= 20) stat = msg(MSG_COND_POOR, ch);
	else			stat = msg(MSG_COND_FRAGILE, ch);

	return stat;
}

/* new practice */
void do_practice(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA	*mob;
	int		sn;
	SKILL_DATA	*sk;
	PC_SKILL	*ps;
	CLASS_DATA	*cl;
	CLASS_SKILL	*cs;
	int		adept;
	bool		found;
	int		rating;
	char		arg[MAX_STRING_LENGTH];

	if (IS_NPC(ch))
		return;

	if (argument[0] == '\0') {
		BUFFER *output;
		int col = 0;
		int i;

		output = buf_new(0);

		for (i = 0; i < ch->pcdata->learned->nused; i++) {
			ps = VARR_GET(ch->pcdata->learned, i);

			if (ps->percent == 0
			||  (sk = skill_lookup(ps->sn)) == NULL
			||  skill_level(ch, ps->sn) > ch->level)
				continue;

			buf_printf(output, "%-18s %3d%%  ",
				   sk->name, ps->percent);
			if (++col % 3 == 0)
				buf_add(output, "\n\r");
		}

		if (col % 3)
			buf_add(output, "\n\r");

		buf_printf(output, "You have %d practice sessions left.\n\r",
			   ch->practice);

		page_to_char(buf_string(output), ch);
		buf_free(output);
		return;
	}

	if ((cl = CLASS(ch->class)) == NULL) {
		log_printf("do_practice: %s: class %d: unknown",
			   ch->name, ch->class);
		return;
	}

	if (ch->practice <= 0) {
		send_to_char("You have no practice sessions left.\n\r", ch);
		return;
	}

	one_argument(argument, arg);

	if ((ps = skill_vlookup(ch->pcdata->learned, arg)) == NULL
	||  ps->percent == 0
	||  skill_level(ch, sn = ps->sn) > ch->level) {
		send_to_char("You can't practice that.\n\r", ch);
		return;
	}

	if (sn == gsn_vampire) {
		send_to_char("You can't practice that, only available "
			     "at questor.\n\r", ch);
		return;
	}

	found = FALSE;
	sk = SKILL(sn);
	for (mob = ch->in_room->people; mob != NULL; mob = mob->next_in_room) {
		if (!IS_NPC(mob) || !IS_SET(mob->act, ACT_PRACTICE))
			continue;

		found = TRUE;

		if (IS_SET(sk->flags, SKILL_CLAN)) {
			if (ch->clan == mob->clan)
				break;
			continue;
		}

		if ((mob->pIndexData->practicer == 0 &&
		    (sk->group == GROUP_NONE ||
		     IS_SET(sk->group,	GROUP_CREATION | GROUP_HARMFUL |
					GROUP_PROTECTIVE | GROUP_DETECTION |
					GROUP_WEATHER)))
		||  IS_SET(mob->pIndexData->practicer, sk->group))
			break;
	}

	if (mob == NULL) {
		if (found)
			char_puts("You can't do that here. "
				  "Use 'slook skill', 'help practice' "
				  "for more info.\n\r", ch);
		else
			char_puts("You couldn't find anyone "
				  "who can teach you.\n\r", ch);
		return;
	}

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

void do_camp(CHAR_DATA *ch, const char *argument)
{
	AFFECT_DATA af;
	int sn;
	int chance;
	int mana;

	if ((sn = sn_lookup("camp")) < 0
	||  (chance = get_skill(ch, sn)) == 0) {
		char_nputs(MSG_HUH, ch);
		return;
	}

	if (is_affected(ch, sn)) {
		send_to_char("You don't have enough power to handle more "
			     "camp areas.\n\r", ch);
		return;
	}

	if (IS_SET(ch->in_room->room_flags, ROOM_SAFE)
	||  IS_SET(ch->in_room->room_flags, ROOM_PRIVATE)
	||  IS_SET(ch->in_room->room_flags, ROOM_SOLITARY)
	||  (ch->in_room->sector_type != SECT_FIELD
	&&   ch->in_room->sector_type != SECT_FOREST
	&&   ch->in_room->sector_type != SECT_MOUNTAIN
	&&   ch->in_room->sector_type != SECT_HILLS)) {
		send_to_char("There are not enough leaves to camp here.\n\r",
			     ch);
		return;
	}

	mana = SKILL(sn)->min_mana;
	if (ch->mana < mana) {
		send_to_char("You don't have enough mana to make a camp.\n\r",
			     ch);
		return;
	}
	ch->mana -= mana;

	if (number_percent() > chance) {
		send_to_char("You failed to make your camp.\n\r", ch);
		check_improve(ch, sn, FALSE, 4);
		return;
	}

	check_improve(ch, sn, TRUE, 4);
	WAIT_STATE(ch, SKILL(sn)->beats);

	send_to_char("You succeeded to make your camp.\n\r", ch);
	act("$n succeeded to make $s camp.", ch, NULL, NULL, TO_ROOM);

	af.where	= TO_AFFECTS;
	af.type 	= sn;
	af.level	= ch->level;
	af.duration	= 12;
	af.bitvector	= 0;
	af.modifier	= 0;
	af.location	= APPLY_NONE;
	affect_to_char(ch, &af);

	af.where	= TO_ROOM_CONST;
	af.type		= sn;
	af.level	= ch->level;
	af.duration	= ch->level / 20;
	af.bitvector	= 0;
	af.modifier	= 2 * ch->level;
	af.location	= APPLY_ROOM_HEAL;
	affect_to_room(ch->in_room, &af);

	af.modifier	= ch->level;
	af.location	= APPLY_ROOM_MANA;
	affect_to_room(ch->in_room, &af);
}

void do_demand(CHAR_DATA *ch, const char *argument)
{
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA  *obj;
	int chance;

	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (IS_NPC(ch))
		return;

	if (ch->class != CLASS_ANTI_PALADIN) {
		char_nputs(MSG_YOU_CANT_DO_THAT, ch);
		return;
	}

	if (arg1[0] == '\0' || arg2[0] == '\0') {
		send_to_char("Demand what from whom?\n\r", ch);
		return;
	}

	if ((victim = get_char_room(ch, arg2)) == NULL) {
		char_nputs(MSG_THEY_ARENT_HERE, ch);
		return;
	}

	if (!IS_NPC(victim)) {
		send_to_char("Why don't you just want that directly "
			     "from the player?\n\r", ch);
		return;
	}

	WAIT_STATE(ch, PULSE_VIOLENCE);

	chance = IS_EVIL(victim) ? 10 : IS_GOOD(victim) ? -5 : 0;
	chance += (get_curr_stat(ch,STAT_CHA) - 15) * 10;
	chance += ch->level - victim->level;

	if (victim->level >= ch->level + 10 || victim->level >= ch->level * 2)
		chance = 0;

	if (number_percent() > chance) {
		do_say(victim, "I'm not about to give you anything!");
		do_murder(victim, ch->name);
		return;
	}

	if (((obj = get_obj_carry(victim , arg1)) == NULL
	&&   (obj = get_obj_wear(victim, arg1)) == NULL)
	||  IS_SET(obj->extra_flags, ITEM_INVENTORY)) {
		do_say(victim, "Sorry, I don't have that.");
		return;
	}


	if (obj->wear_loc != WEAR_NONE)
		unequip_char(victim, obj);

	if (!can_drop_obj(ch, obj)) {
		do_say(victim, "It's cursed so, I can't let go of it. "
			       "Forgive me, my master");
		return;
	}

	if (ch->carry_number + get_obj_number(obj) > can_carry_n(ch)) {
		send_to_char("Your hands are full.\n\r", ch);
		return;
	}

	if (ch->carry_weight + get_obj_weight(obj) > can_carry_w(ch)) {
		send_to_char("You can't carry that much weight.\n\r", ch);
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
	send_to_char("Your power makes all around the world shivering.\n\r",ch);
}

void do_control(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int chance;
	int sn;

	argument = one_argument(argument, arg);

	if ((sn = sn_lookup("control animal")) < 0
	||  (chance = get_skill(ch, sn)) == 0) {
		char_nputs(MSG_HUH, ch);
		return;
	}

	if (arg[0] == '\0') {
		send_to_char("Charm what?\n\r", ch);
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL) {
		char_nputs(MSG_THEY_ARENT_HERE, ch);
		return;
	}

	if (race_table[ORG_RACE(victim)].pc_race) {
		send_to_char("You should try this on monsters?\n\r", ch);
		return;
	}

	if (is_safe(ch, victim))
		return;

	if (count_charmed(ch))
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
	||  (IS_NPC(victim) && victim->pIndexData->pShop != NULL)) {
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
	AFFECT_DATA af, saf;
	int count, color, mana, wait;
	char arg[MAX_INPUT_LENGTH];
	char *str = "wooden";
	int chance;
	int sn;

	if (IS_NPC(ch))
		return;

	if ((sn = sn_lookup("make arrow")) < 0
	||  (chance = get_skill(ch, sn)) == 0) {
		send_to_char("You don't know how to make arrows.\n\r", ch);
		return;
	}

	if (ch->in_room->sector_type != SECT_FIELD
	&&  ch->in_room->sector_type != SECT_FOREST
	&&  ch->in_room->sector_type != SECT_HILLS) {
		send_to_char("You couldn't find enough wood.\n\r", ch);
		return;
	}

	mana = SKILL(sn)->min_mana;
	wait = SKILL(sn)->beats;

	color = -1;
	argument = one_argument(argument, arg);
	if (arg[0] == '\0')
		color = 0;
	else if (!str_prefix(arg, "green")) {
		color = sn_lookup("green arrow");
		saf.bitvector	= WEAPON_POISON;
		str = "green";
	}
	else if (!str_prefix(arg, "red")) {
		color = sn_lookup("red arrow");
		saf.bitvector	= WEAPON_FLAMING;
		str = "red";
	}
	else if (!str_prefix(arg, "white")) {
		color = sn_lookup("white arrow");
		saf.bitvector	= WEAPON_FROST;
		str = "white";
	}
	else if (!str_prefix(arg, "blue")) {
		color = sn_lookup("blue arrow");
		saf.bitvector	= WEAPON_SHOCKING;
		str = "blue";
	}

	if (color < 0) {
		send_to_char("You don't know how to make "
			     "that kind of arrow.\n\r", ch);
		return;
	}

	if (color) {
		mana += SKILL(color)->min_mana;
		wait += SKILL(color)->beats;
	}

	if (ch->mana < mana) {
		send_to_char("You don't have enough energy "
			     "to make that kind of arrows.\n\r", ch);
		return;
	}

	ch->mana -= mana;
	WAIT_STATE(ch, wait);

	send_to_char("You start to make arrows!\n\r",ch);
	act("$n starts to make arrows!",ch,NULL,NULL,TO_ROOM);
	for(count = 0; count < ch->level / 5; count++) {
		if (number_percent() > chance) {
			send_to_char("You failed to make the arrow, "
				     "and broke it.\n\r", ch);
			check_improve(ch, sn, FALSE, 3);
			if (color)
				check_improve(ch, color, FALSE, 3);
			continue;
		}

		send_to_char("You successfully make an arrow.\n\r", ch);
		check_improve(ch, sn, TRUE, 3);
		if (color)
			check_improve(ch, color, TRUE, 3);

		arrow = create_named_obj(get_obj_index(OBJ_VNUM_RANGER_ARROW),
					 ch->level, str);
		arrow->level = ch->level;
		arrow->value[1] = 4 + ch->level / 10;
		arrow->value[2] = 4 + ch->level / 10;

		af.where	 = TO_OBJECT;
		af.type		 = sn;
		af.level	 = ch->level;
		af.duration	 = -1;
		af.location	 = APPLY_HITROLL;
		af.modifier	 = ch->level / 10;
		af.bitvector 	 = 0;
		affect_to_obj(arrow, &af);

		af.where	= TO_OBJECT;
		af.type		= sn;
		af.level	= ch->level;
		af.duration	= -1;
		af.location	= APPLY_DAMROLL;
		af.modifier	= ch->level / 10;
		af.bitvector	= 0;
		affect_to_obj(arrow, &af);

		if (color) {
			saf.where	 = TO_WEAPON;
			saf.type	 = color;
			saf.level	 = ch->level;
			saf.duration	 = -1;
			saf.location	 = 0;
			saf.modifier	 = 0;
			affect_to_obj(arrow, &saf);
		}

		obj_to_char(arrow, ch);
		arrow = NULL;
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
		send_to_char("You don't know how to make bows.\n\r", ch);
		return;
	}

	if (ch->in_room->sector_type != SECT_FIELD
	&&  ch->in_room->sector_type != SECT_FOREST
	&&  ch->in_room->sector_type != SECT_HILLS) {
		send_to_char("You couldn't find enough wood.\n\r", ch);
		return;
	}

	mana = SKILL(sn)->min_mana;
	if (ch->mana < mana) {
		send_to_char("You don't have enough energy to make a bow.\n\r",
			     ch);
		return;
	}
	ch->mana -= mana;
	WAIT_STATE(ch, SKILL(sn)->beats);

	if (number_percent() > chance) {
		send_to_char("You failed to make the bow, and broke it.\n\r",
			     ch);
		check_improve(ch, sn, FALSE, 1);
		return;
	}
	send_to_char("You successfully make bow.\n\r", ch);
	check_improve(ch, sn, TRUE, 1);

	bow = create_obj(get_obj_index(OBJ_VNUM_RANGER_BOW), ch->level);
	bow->level = ch->level;
	bow->value[1] = 4 + ch->level / 15;
	bow->value[2] = 4 + ch->level / 15;

	af.where	= TO_OBJECT;
	af.type		= sn;
	af.level	= ch->level;
	af.duration	= -1;
	af.location	= APPLY_HITROLL;
	af.modifier	= ch->level / 10;
	af.bitvector 	= 0;
	affect_to_obj(bow, &af);

	af.where	= TO_OBJECT;
	af.type		= sn;
	af.level	= ch->level;
	af.duration	= -1;
	af.location	= APPLY_DAMROLL;
	af.modifier	= ch->level / 10;
	af.bitvector 	= 0;
	affect_to_obj(bow, &af);

	obj_to_char(bow, ch);
}

void do_make(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];

	argument = one_argument(argument,arg);
	if (arg[0] == '\0') {
		send_to_char("You can make either bow or arrow.\n\r",ch);
		return;
	}

	if (!str_prefix(arg, "arrow"))
		do_make_arrow(ch, argument);
	else if (!str_prefix(arg, "bow"))
		do_make_bow(ch, argument);
	else
		do_make(ch, "");
}

