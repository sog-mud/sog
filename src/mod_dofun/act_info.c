/*
 * $Id: act_info.c,v 1.74 1998-06-16 16:56:45 fjoe Exp $
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
#include <ctype.h>
#include "merc.h"
#include "db.h"
#include "comm.h"
#include "const.h"
#include "magic.h"
#include "recycle.h"
#include "tables.h"
#include "lookup.h"
#include "resource.h"
#include "act_info.h"
#include "act_comm.h"
#include "const.h"
#include "hometown.h"
#include "interp.h"
#include "update.h"
#include "util.h"
#include "quest.h"

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
char *	format_obj_to_char	args((OBJ_DATA *obj, CHAR_DATA *ch,
				    bool fShort));
void	show_list_to_char	args((OBJ_DATA *list, CHAR_DATA *ch,
				    bool fShort, bool fShowNothing));
void	show_char_to_char_0	args((CHAR_DATA *victim, CHAR_DATA *ch));
void	show_char_to_char_1	args((CHAR_DATA *victim, CHAR_DATA *ch));
void	show_char_to_char	args((CHAR_DATA *list, CHAR_DATA *ch));
bool	check_blind		args((CHAR_DATA *ch));

#define is_empty(s) ((s) == NULL || (s)[0] == '\0')
#define strend(s) (strchr(s, '\0'))

char *format_obj_to_char(OBJ_DATA *obj, CHAR_DATA *ch, bool fShort)
{
	static char buf[MAX_STRING_LENGTH];

	buf[0] = '\0';
	if ((fShort && is_empty(obj->short_descr))
	||  is_empty(obj->description))
		return buf;

	if (IS_SET(ch->comm, COMM_LONG)) {
		if (IS_OBJ_STAT(obj, ITEM_INVIS))
			strcat(buf, msg(INVIS, ch));
		if (IS_OBJ_STAT(obj, ITEM_DARK))
			strcat(buf, msg(DARK, ch));
		if (CAN_DETECT(ch, DETECT_EVIL) && IS_OBJ_STAT(obj, ITEM_EVIL))
			strcat(buf, msg(RED_AURA, ch));
		if (CAN_DETECT(ch, DETECT_GOOD) && IS_OBJ_STAT(obj,ITEM_BLESS))
			strcat(buf, msg(BLUE_AURA, ch));
		if (CAN_DETECT(ch, DETECT_MAGIC) && IS_OBJ_STAT(obj,ITEM_MAGIC))
			strcat(buf, msg(MAGICAL, ch));
		if (IS_OBJ_STAT(obj, ITEM_GLOW))
			strcat(buf, msg(GLOWING, ch));
		if (IS_OBJ_STAT(obj, ITEM_HUM))
			strcat(buf, msg(HUMMING, ch));
	}
	else {
		static char FLAGS[] = "{x[{y.{D.{R.{B.{M.{W.{Y.{x] ";
		strcpy(buf, FLAGS);
		if (IS_OBJ_STAT(obj, ITEM_INVIS)	)   buf[5] = 'I';
		if (IS_OBJ_STAT(obj, ITEM_DARK)		)   buf[8] = 'D';
		if (CAN_DETECT(ch, DETECT_EVIL)
		&& IS_OBJ_STAT(obj, ITEM_EVIL)		)   buf[11] = 'E';
		if (CAN_DETECT(ch, DETECT_GOOD)
		&&  IS_OBJ_STAT(obj,ITEM_BLESS)		)   buf[14] = 'B';
		if (CAN_DETECT(ch, DETECT_MAGIC)
		&& IS_OBJ_STAT(obj, ITEM_MAGIC)		)   buf[17] = 'M';
		if (IS_OBJ_STAT(obj, ITEM_GLOW)		)   buf[20] = 'G';
		if (IS_OBJ_STAT(obj, ITEM_HUM)		)   buf[23] = 'H';
		if (strcmp(buf, FLAGS) == 0)
			buf[0] = '\0';
	}

	if (fShort) {
		strcat(buf, obj->short_descr);
		if (obj->pIndexData->vnum > 5)	/* not money, gold, etc */
			sprintf(strend(buf), " [{g%s{x]",
				get_cond_alias(obj, ch));
		return buf;
	}

	if (obj->in_room != NULL && IS_WATER(obj->in_room)) {
		char* p;

		p = strend(buf);
		strcat(buf, obj->short_descr);
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
		strcat(buf, obj->description);
	return buf;
}


/*
 * Show a list to a character.
 * Can coalesce duplicated items.
 */
void show_list_to_char(OBJ_DATA *list, CHAR_DATA *ch,
		       bool fShort, bool fShowNothing)
{
	char buf[MAX_STRING_LENGTH];
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
	output = new_buf();

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
			if (prgnShow[iShow] != 1) {
				sprintf(buf, "(%2d) ", prgnShow[iShow]);
				add_buf(output,buf);
			}
			else
				add_buf(output,"     ");
		}

		add_buf(output,prgpstrShow[iShow]);
		add_buf(output,"\n\r");
		free_string(prgpstrShow[iShow]);
	}

	if (fShowNothing && nShow == 0) {
		if (IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE))
			send_to_char("     ", ch);
		send_to_char(msg(NOTHING, ch), ch);
	}

	page_to_char(buf_string(output),ch);

	/*
	 * Clean up.
	 */
	free_buf(output);
	free_mem(prgpstrShow, count * sizeof(char *));
	free_mem(prgnShow,    count * sizeof(int)  );

	return;
}


void show_char_to_char_0(CHAR_DATA *victim, CHAR_DATA *ch)
{
	char buf[MAX_STRING_LENGTH];
	int msgnum;

	buf[0] = '\0';

	if (!IS_NPC(ch) && IS_NPC(victim) && ch->pcdata->questmob > 0
	&&  victim->hunter == ch)
		strcat(buf, msg(TARGET, ch));

/*
	sprintf(message,"(%s) ",race_table[RACE(victim)].name);
	message[1] = UPPER(message[1]);
	strcat(buf,message);
*/

	if (IS_SET(ch->comm, COMM_LONG)) {
		if (IS_AFFECTED(victim, AFF_INVISIBLE))
			strcat(buf, msg(INVIS, ch));
		if (IS_AFFECTED(victim, AFF_HIDE)) 
			strcat(buf, msg(HIDDEN, ch));
		if (IS_AFFECTED(victim, AFF_CHARM)) 
			strcat(buf, msg(CHARMED, ch));
		if (IS_AFFECTED(victim, AFF_PASS_DOOR)) 
			strcat(buf, msg(TRANSLUCENT, ch));
		if (IS_AFFECTED(victim, AFF_FAERIE_FIRE)) 
			strcat(buf, msg(PINK_AURA, ch));
		if (IS_NPC(victim) && IS_SET(victim->act,ACT_UNDEAD)
		&&  CAN_DETECT(ch, DETECT_UNDEAD))
			strcat(buf, msg(UNDEAD, ch));
		if (RIDDEN(victim))
			strcat(buf, msg(RIDDEN, ch));
		if (IS_AFFECTED(victim,AFF_IMP_INVIS))
			strcat(buf, msg(IMPROVED, ch));
		if (IS_EVIL(victim) && CAN_DETECT(ch, DETECT_EVIL))
			strcat(buf, msg(RED_AURA, ch));
		if (IS_GOOD(victim) && CAN_DETECT(ch, DETECT_GOOD))
			strcat(buf, msg(GOLDEN_AURA, ch));
		if (IS_AFFECTED(victim, AFF_SANCTUARY))
			strcat(buf, msg(WHITE_AURA, ch));
		if (IS_AFFECTED(victim, AFF_FADE)) 
			strcat(buf, msg(FADE, ch));
		if (!IS_NPC(victim) && IS_SET(victim->act, PLR_WANTED))
			strcat(buf, msg(WANTED, ch));
		if (IS_AFFECTED(victim, AFF_CAMOUFLAGE)) 
			strcat(buf, msg(CAMF, ch));
	}
	else {
		static char FLAGS[] = "{x[{y.{D.{m.{c.{M.{D.{G.{b.{R.{Y.{W.{y.{R.{g.{x] ";
		char* p = strend(buf);
		strcpy(p, FLAGS); 
		if (IS_AFFECTED(victim, AFF_INVISIBLE)  ) p[5] = 'I';
		if (IS_AFFECTED(victim, AFF_HIDE)       ) p[8] = 'H';
		if (IS_AFFECTED(victim, AFF_CHARM)      ) p[11] = 'C';
		if (IS_AFFECTED(victim, AFF_PASS_DOOR)  ) p[14] = 'T';
		if (IS_AFFECTED(victim, AFF_FAERIE_FIRE)) p[17] = 'P';
		if (IS_NPC(victim)
		&&  IS_SET(victim->act,ACT_UNDEAD)
		&&  CAN_DETECT(ch, DETECT_UNDEAD)       ) p[20] = 'U';
		if (RIDDEN(victim)			) p[23] = 'R';
		if (IS_AFFECTED(victim, AFF_IMP_INVIS)  ) p[26] = 'I';
		if (IS_EVIL(victim)
		&& CAN_DETECT(ch, DETECT_EVIL)		) p[29] = 'E';
		if (IS_GOOD(victim)
		&&  CAN_DETECT(ch, DETECT_GOOD)		) p[32] = 'G';
		if (IS_AFFECTED(victim, AFF_SANCTUARY)  ) p[35] = 'S';
		if (IS_AFFECTED(victim, AFF_CAMOUFLAGE) ) p[38] = 'C';
		if (!IS_NPC(victim)
		&&  IS_SET(victim->act, PLR_WANTED)     ) p[41] = 'W';
		if (IS_AFFECTED(victim, AFF_FADE)       ) p[44] = 'F';
		if (strcmp(p, FLAGS) == 0)
			p[0] = '\0';
	}

	if (victim->invis_level >= LEVEL_HERO)
		strcat(buf, "[{WWizi{x] ");

	if (victim->position == victim->start_pos
	&&  victim->long_descr[0] != '\0') {
		char_printf(ch, "%s{g%s{x", buf, victim->long_descr);
		return;
	}

	if (IS_SET(ch->act,PLR_HOLYLIGHT)
	&&  is_affected(victim,gsn_doppelganger)) {
		strcat(buf, "{{");
		strcat(buf, PERS(victim,ch));
		strcat(buf, "} ");
	}

	if (is_affected(victim, gsn_doppelganger)
	&&  victim->doppel->long_descr[0] != '\0') {
		char_printf(ch, "%s%s", buf, victim->doppel->long_descr);
		return;
	}

	if (is_affected(victim, gsn_doppelganger)) {
		strcat(buf, PERS(victim->doppel, ch));
		if (!IS_NPC(victim->doppel) && !IS_SET(ch->comm, COMM_BRIEF))
			strcat(buf, victim->doppel->pcdata->title);
	}
	else {
		strcat(buf, PERS(victim, ch));
		if (!IS_NPC(victim) && !IS_SET(ch->comm, COMM_BRIEF)
		&&  victim->position == POS_STANDING && ch->on == NULL)
			strcat(buf, victim->pcdata->title);
	}

	switch (victim->position) {
	case POS_DEAD:
		strcat(buf, vmsg(IS_DEAD, ch, victim));
		break;

	case POS_MORTAL:
		strcat(buf, vmsg(IS_MORTALLY_WOUNDED, ch, victim));
		break;

	case POS_INCAP:
		strcat(buf, vmsg(IS_INCAPACITATED, ch, victim));
		break;

	case POS_STUNNED:
		strcat(buf, vmsg(IS_LYING_HERE_STUNNED, ch, victim));
		break;

	case POS_SLEEPING:
		if (victim->on == NULL) {
			strcat(buf, msg(SLEEPING, ch));
			break;
		}
		if (victim->on->pIndexData->vnum == 1200)
			return;

		if (IS_SET(victim->on->value[2], SLEEP_AT))
			msgnum = SLEEPING_AT;
		else if (IS_SET(victim->on->value[2], SLEEP_ON))
			msgnum = SLEEPING_ON;
		else
			msgnum = SLEEPING_IN;

		sprintf(strend(buf), msg(msgnum, ch), victim->on->short_descr);
		break;

	case POS_RESTING:
		if (victim->on == NULL) {
			strcat(buf, msg(RESTING, ch));
			break;
		}

		if (IS_SET(victim->on->value[2], REST_AT))
			msgnum = RESTING_AT;
		else if (IS_SET(victim->on->value[2], REST_ON))
			msgnum = RESTING_ON;
		else
			msgnum = RESTING_IN;
		sprintf(strend(buf), msg(msgnum, ch), victim->on->short_descr);
		break;

	case POS_SITTING:
		if (victim->on == NULL) {
			strcat(buf, msg(SITTING, ch));
			break;
		}

		if (IS_SET(victim->on->value[2], SIT_AT))
			msgnum = SITTING_AT;
		else if (IS_SET(victim->on->value[2], SIT_ON))
			msgnum = SITTING_ON;
		else
			msgnum = SITTING_IN;
		sprintf(strend(buf), msg(msgnum, ch), victim->on->short_descr);
		break;

	case POS_STANDING:
		if (victim->on == NULL) {
			if (MOUNTED(victim))
				sprintf(strend(buf), msg(HERE_RIDING, ch),
					PERS(MOUNTED(victim),ch));
			else
				strcat(buf, msg(IS_HERE, ch));
			break;
		}

		if (IS_SET(victim->on->value[2],STAND_AT))
			msgnum = STANDING_AT;
		else if (IS_SET(victim->on->value[2],STAND_ON))
			msgnum = STANDING_ON;
		else
			msgnum = STANDING;
		sprintf(strend(buf), msg(msgnum, ch), victim->on->short_descr);
		break;

	case POS_FIGHTING:
		strcat(buf, msg(FIGHTING, ch));
		if (victim->fighting == NULL)
			strcat(buf, "thin air??");
		else if (victim->fighting == ch)
			strcat(buf, msg(FIGHTING_YOU, ch));
		else if (victim->in_room == victim->fighting->in_room) {
			strcat(buf, PERS(victim->fighting, ch));
			strcat(buf, ".");
		}
		else
			strcat(buf, "somone who left??");
		break;
	}

	strcat(buf, "{x\n\r");
	send_to_char(buf, ch);
	return;
}


void show_char_to_char_1(CHAR_DATA *victim, CHAR_DATA *ch)
{
	OBJ_DATA *obj;
	int i;
	int percent;
	bool found;
	CHAR_DATA *vict;
	int msgnum;

	vict = is_affected(victim, gsn_doppelganger) ? victim->doppel : victim;

	if (can_see(victim, ch)) {
		if (ch == victim)
			act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING,
					N_LOOKS_AT_SELF);
		else {
			act_nprintf(ch, NULL, victim, TO_VICT, POS_RESTING,
					N_LOOKS_AT_YOU);
			act_nprintf(ch, NULL, victim, TO_NOTVICT, POS_RESTING,
					N_LOOKS_AT_N);
		}
	}

	if (vict->description[0] != '\0')
		send_to_char(vict->description, ch);
	else
		act_nprintf(ch, NULL, victim, TO_CHAR, POS_DEAD,
				SEE_NOTHING_SPECIAL);

	if (MOUNTED(victim))
		char_printf(ch, msg(IS_RIDING_S, ch),
			    PERS(victim,ch), PERS(MOUNTED(victim),ch));
	if (RIDDEN(victim))
		char_printf(ch, vmsg(IS_RIDDEN_BY_S, ch, victim),
			    PERS(victim,ch), PERS(RIDDEN(victim),ch));

	if (victim->max_hit > 0)
		percent = (100 * victim->hit) / victim->max_hit;
	else
		percent = -1;

	if (percent >= 100)
		msgnum = IS_IN_PERFECT_HEALTH;
	else if (percent >= 90)
		msgnum = HAS_A_FEW_SCRATCHES;
	else if (percent >= 75)
		msgnum = HAS_SOME_SMALL_BUT_DISGUSTING_CUTS;
	else if (percent >= 50)
		msgnum = IS_COVERED_WITH_BLEEDING_WOUNDS;
	else if (percent >= 30)
		msgnum = IS_GUSHING_BLOOD;
	else if (percent >= 15)
		msgnum = IS_WRITHING_IN_AGONY;
	else if (percent >= 0)
		msgnum = IS_CONVULSING_ON_THE_GROUND;
	else
		msgnum = IS_NEARLY_DEAD;

	/* vampire ... */
	if (percent < 90 && ch->class == CLASS_VAMPIRE && ch->level > 10)
		gain_condition(ch, COND_BLOODLUST, -1);

	char_printf(ch, "(%s) %s %s\n\r",
		    race_table[RACE(vict)].name, PERS(vict, ch),
		    vmsg(msgnum, ch, vict));

	found = FALSE;
	for (i = 0; show_order[i] != -1; i++) {
		if ((obj = get_eq_char(is_affected(victim,gsn_mirror) ?
				       vict : victim, show_order[i])) != NULL
		&&  can_see_obj(ch, obj)) {

			if (!found) {
				send_to_char("\n\r", ch);
				act(msg(IS_USING, ch), ch, NULL, victim,
				    TO_CHAR);
				found = TRUE;
			}

			act_nprintf(ch, NULL, NULL, TO_CHAR, POS_RESTING,
				   S_S,
				   msg(EQ_USED_AS_LIGHT + show_order[i], ch),
				   format_obj_to_char(obj, ch, TRUE));
		}
	}

	for (obj = is_affected(victim,gsn_mirror) ?
	     vict->carrying : victim->carrying;
	     obj != NULL; obj = obj->next_content) {
		if (obj->wear_loc != WEAR_STUCK_IN || !can_see_obj(ch, obj))
			continue;

		if (!found) {
			send_to_char("\n\r", ch);
			act(msg(IS_USING, ch), ch, NULL, victim, TO_CHAR);
			found = TRUE;
		}
		act_nprintf(ch, NULL, NULL, TO_CHAR, POS_RESTING, S_S,
			   msg(EQ_STUCK_IN, ch),
			   format_obj_to_char(obj, ch, TRUE));
	}

	if (victim != ch
	&&  !IS_NPC(ch)
	&&  number_percent() < get_skill(ch,gsn_peek)) {
		send_to_char(msg(YOU_PEEK_AT_THE_INVENTORY, ch), ch);
		check_improve(ch,gsn_peek,TRUE,4);
		show_list_to_char(is_affected(victim,gsn_mirror) ?
				  vict->carrying : victim->carrying,
				  ch, TRUE, TRUE);
	}

	return;
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
			send_to_char(msg(GLOWING_RED_EYES, ch), ch);
			if (!IS_IMMORTAL(rch))
				life_count++;
		}
		else if (!IS_IMMORTAL(rch))
			life_count++;
	}

	if (life_count && CAN_DETECT(ch,DETECT_LIFE))
		char_printf(ch, msg(FEEL_MORE_LIVES, ch),
			    life_count, (life_count == 1) ? "form" : "forms");
	return;
}


bool check_blind(CHAR_DATA *ch)
{
	if (!IS_NPC(ch) && IS_SET(ch->act,PLR_HOLYLIGHT))
		return TRUE;

	if (IS_AFFECTED(ch, AFF_BLIND)) {
		send_to_char(msg(CANT_SEE_THING, ch), ch);
		return FALSE;
	}

	return TRUE;
}


void do_clear(CHAR_DATA *ch, char *argument)
{
	if (!IS_NPC(ch))
		send_to_char("\033[0;0H\033[2J", ch);
}


/* changes your scroll */
void do_scroll(CHAR_DATA *ch, char *argument)
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
void do_socials(CHAR_DATA *ch, char *argument)
{
	int iSocial;
	int col = 0;

	for (iSocial = 0; social_table[iSocial].name[0] != '\0'; iSocial++) {
		char_printf(ch, "%-12s", social_table[iSocial].name);
		if (++col % 6 == 0)
			send_to_char("\n\r",ch);
	}

	if (col % 6)
		send_to_char("\n\r",ch);
	return;
}


/* RT Commands to replace news, motd, imotd, etc from ROM */
void do_motd(CHAR_DATA *ch, char *argument)
{
	do_help(ch, "motd");
}


void do_imotd(CHAR_DATA *ch, char *argument)
{
	do_help(ch, "imotd");
}


void do_rules(CHAR_DATA *ch, char *argument)
{
	do_help(ch, "rules");
}


void do_story(CHAR_DATA *ch, char *argument)
{
	do_help(ch, "story");
}


void do_wizlist(CHAR_DATA *ch, char *argument)
{
	do_help(ch, "wizlist");
}


/* RT this following section holds all the auto commands from ROM, as well as
   replacements for config */
#define do_print_sw(ch, swname, sw) \
		char_printf(ch, "%-16s {r%s{x\n\r", swname, sw ? "ON" : "OFF");
void do_autolist(CHAR_DATA *ch, char *argument)
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


void do_autoassist(CHAR_DATA *ch, char *argument)
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


void do_autoexit(CHAR_DATA *ch, char *argument)
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


void do_autogold(CHAR_DATA *ch, char *argument)
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


void do_autoloot(CHAR_DATA *ch, char *argument)
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


void do_autosac(CHAR_DATA *ch, char *argument)
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


void do_autosplit(CHAR_DATA *ch, char *argument)
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


void do_color(CHAR_DATA *ch, char *argument)
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


void do_brief(CHAR_DATA *ch, char *argument)
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


void do_compact(CHAR_DATA *ch, char *argument)
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


void do_long(CHAR_DATA *ch, char *argument)
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


void do_show(CHAR_DATA *ch, char *argument)
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


void do_prompt(CHAR_DATA *ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];

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

	if(!strcmp(argument, "all"))
		strcpy(buf, DEFAULT_PROMPT);
	else {
		if (strlen(argument) > 50)
			argument[50] = '\0';
		sprintf(buf, argument); /* %% -> % */
		smash_tilde(buf);
		if (str_suffix("%c",buf))
			strcat(buf," ");
	}

	free_string(ch->prompt);
	ch->prompt = str_dup(buf);
	char_printf(ch, "Prompt set to %s\n\r", ch->prompt);
	return;
}


void do_combine(CHAR_DATA *ch, char *argument)
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


void do_noloot(CHAR_DATA *ch, char *argument)
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


void do_nofollow(CHAR_DATA *ch, char *argument)
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


void do_nosummon(CHAR_DATA *ch, char *argument)
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


void do_look_in(CHAR_DATA* ch, char* arg)
{
	OBJ_DATA *obj;

	if ((obj = get_obj_here(ch, arg)) == NULL) {
		send_to_char(msg(YOU_DONT_SEE_THAT, ch), ch);
		return;
	}

	switch (obj->item_type) {
	default:
		send_to_char(msg(THATS_NOT_CONTAINER, ch), ch);
		break;

	case ITEM_DRINK_CON:
		if (obj->value[1] <= 0) {
			send_to_char(msg(IT_IS_EMPTY, ch), ch);
			break;
		}

		char_printf(ch, msg(ITS_FILLED_S, ch),
			    obj->value[1] < obj->value[0] / 4 ?
			    msg(LESS_THAN_HALF, ch) :
			    obj->value[1] < 3 * obj->value[0] / 4 ?
			    msg(ABOUT_HALF,ch):msg(MORE_THAN_HALF,ch),
			    liq_table[obj->value[2]].liq_color);
		break;

	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
	case ITEM_CORPSE_PC:
		if (IS_SET(obj->value[1], CONT_CLOSED)) {
			send_to_char(msg(IT_IS_CLOSED, ch), ch);
			break;
		}

		act_nprintf(ch, obj, NULL, TO_CHAR, POS_DEAD, P_HOLDS);
		show_list_to_char(obj->contains, ch, TRUE, TRUE);
		break;
	}
}

void do_look(CHAR_DATA *ch, char *argument)
{
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	char arg3 [MAX_INPUT_LENGTH];
	EXIT_DATA *pexit;
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	char *pdesc;
	int door;
	int number,count;

	if (ch->desc == NULL)
		return;

	if (ch->position < POS_SLEEPING) {
		send_to_char(msg(CANT_SEE_BUT_STARS, ch), ch);
		return;
	}

	if (ch->position == POS_SLEEPING) {
		send_to_char(msg(CANT_SEE_SLEEPING, ch), ch);
		return;
	}

	if (!check_blind(ch))
		return;

	if (!IS_NPC(ch)
	&&  !IS_SET(ch->act, PLR_HOLYLIGHT)
	&&  room_is_dark(ch)) {
		send_to_char(msg(PITCH_BLACK, ch), ch);
		show_char_to_char(ch->in_room->people, ch);
		return;
	}

	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);
	number = number_argument(arg1, arg3);
	count = 0;

	if (arg1[0] == '\0' || !str_cmp(arg1, "auto")) {
		/* 'look' or 'look auto' */
		char_printf(ch, "{W%s{x", ch->in_room->name, ch);

		if (IS_IMMORTAL(ch)
		&&  (IS_NPC(ch) || IS_SET(ch->act,PLR_HOLYLIGHT)))
			char_printf(ch, " [Room %d]",ch->in_room->vnum);

		send_to_char("\n\r", ch);

		if (arg1[0] == '\0'
		||  (!IS_NPC(ch) && !IS_SET(ch->comm, COMM_BRIEF))) {
			send_to_char("  ", ch);
			send_to_char(ch->in_room->description, ch);
		}

		if (!IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOEXIT)) {
			send_to_char("\n\r", ch);
			do_exits(ch, "auto");
		}

		show_list_to_char(ch->in_room->contents, ch, FALSE, FALSE);
		show_char_to_char(ch->in_room->people, ch);
		return;
	}

	if (!str_cmp(arg1, "i") || !str_cmp(arg1, "in") || !str_cmp(arg1,"on")) {
		/* 'look in' */
		if (arg2[0] == '\0') {
			send_to_char(msg(LOOK_IN_WHAT, ch), ch);
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
			pdesc = get_extra_descr(arg3, obj->extra_descr);
			if (pdesc != NULL)
				if (++count == number) {
					send_to_char(pdesc, ch);
					return;
					}
				else
					continue;

			pdesc = get_extra_descr(arg3,
						obj->pIndexData->extra_descr);

			if (pdesc != NULL)
				if (++count == number) {
					send_to_char(pdesc, ch);
					return;
				}
				else
					continue;

			if (is_name(arg3, obj->name))
				if (++count == number) {
					send_to_char(
					  msg(NOTHING_SPECIAL_IT, ch), ch);
					return;
				}
		}
	}

	for (obj = ch->in_room->contents;
	     obj != NULL; obj = obj->next_content) {
		if (can_see_obj(ch, obj)) {
			pdesc = get_extra_descr(arg3, obj->extra_descr);
			if (pdesc != NULL)
					if (++count == number) {
					send_to_char(pdesc, ch);
					return;
					}

			pdesc = get_extra_descr(arg3,
						obj->pIndexData->extra_descr);
			if (pdesc != NULL)
					if (++count == number) {
					send_to_char(pdesc, ch);
					return;
					}
		}

		if (is_name(arg3, obj->name))
			if (++count == number) {
				send_to_char(obj->description, ch);
				send_to_char("\n\r",ch);
				return;
			}
	}

	pdesc = get_extra_descr(arg3,ch->in_room->extra_descr);
	if (pdesc != NULL) {
		if (++count == number) {
			send_to_char(pdesc,ch);
			return;
		}
	}

	if (count > 0 && count != number) {
		if (count == 1)
			char_printf(ch, msg(ONLY_SEE_ONE_S, ch), arg3);
		else
			char_printf(ch, msg(ONLY_SEE_D_THOSE, ch), count);
		return;
	}

		   if (!str_cmp(arg1, "n") || !str_cmp(arg1, "north")) door = 0;
	else if (!str_cmp(arg1, "e") || !str_cmp(arg1, "east")) door = 1;
	else if (!str_cmp(arg1, "s") || !str_cmp(arg1, "south")) door = 2;
	else if (!str_cmp(arg1, "w") || !str_cmp(arg1, "west")) door = 3;
	else if (!str_cmp(arg1, "u") || !str_cmp(arg1, "up" )) door = 4;
	else if (!str_cmp(arg1, "d") || !str_cmp(arg1, "down")) door = 5;
	else {
		send_to_char(msg(YOU_DONT_SEE_THAT, ch), ch);
		return;
	}

	/* 'look direction' */
	if ((pexit = ch->in_room->exit[door]) == NULL) {
		send_to_char(msg(NOTHING_SPECIAL_THERE, ch), ch);
		return;
	}

	if (pexit->description != NULL && pexit->description[0] != '\0')
		send_to_char(pexit->description, ch);
	else
		send_to_char(msg(NOTHING_SPECIAL_THERE, ch), ch);

	if (pexit->keyword    != NULL
	&&  pexit->keyword[0] != '\0'
	&&  pexit->keyword[0] != ' ') {
		if (IS_SET(pexit->exit_info, EX_CLOSED)) {
			act_nprintf(ch, NULL, pexit->keyword, TO_CHAR,
					POS_DEAD, THE_D_IS_CLOSED);
		}
		else if (IS_SET(pexit->exit_info, EX_ISDOOR))
			act_nprintf(ch, NULL, pexit->keyword, TO_CHAR,
					POS_DEAD, THE_D_IS_OPEN);
	}

	return;
}


/* RT added back for the hell of it */
void do_read(CHAR_DATA *ch, char *argument)
{
	do_look(ch,argument);
}


void do_examine(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;

	one_argument(argument, arg);

	if (ch->desc == NULL)
		return;

	if (ch->position < POS_SLEEPING) {
		send_to_char(msg(CANT_SEE_BUT_STARS, ch), ch);
		return;
	}

	if (ch->position == POS_SLEEPING) {
		send_to_char(msg(CANT_SEE_SLEEPING, ch), ch);
		return;
	}

	if (!check_blind(ch))
		return;

	if (arg[0] == '\0') {
		send_to_char(msg(EXA_WHAT, ch), ch);
		return;
	}

	do_look(ch, arg);

	if ((obj = get_obj_here(ch, arg)) != NULL) {
		switch (obj->item_type) {
		case ITEM_MONEY:
			if (obj->value[0] == 0) {
				if (obj->value[1] == 0)
					char_puts(msg(NO_COINS_PILE, ch),
							 ch);
				else if (obj->value[1] == 1)
					char_puts(msg(ONE_GOLD_COIN, ch),
						  ch);
				else
					char_printf(ch, 
						    msg(D_GOLD_COINS, ch),
						    obj->value[1]);
			}
			else if (obj->value[1] == 0) {
				if (obj->value[0] == 1)
					char_puts(msg(ONE_SILVER_COIN, ch),
							 ch);
				else
					char_printf(ch, 
						   msg(D_SILVER_COINS, ch),
						   obj->value[0]);
			}
			else
				char_printf(ch, 
					    msg(D_SILVER_AND_D_GOLD, ch), 
					    obj->value[1],obj->value[0]);
			break;

		case ITEM_DRINK_CON:
		case ITEM_CONTAINER:
		case ITEM_CORPSE_NPC:
		case ITEM_CORPSE_PC:
			do_look_in(ch, argument);
		}
	}

	return;
}


/*
 * Thanks to Zrin for auto-exit part.
 */
void do_exits(CHAR_DATA *ch, char *argument)
{
	extern char * const dir_name[];
	char buf[MAX_STRING_LENGTH];
	EXIT_DATA *pexit;
	bool found;
	bool fAuto;
	int door;

	fAuto  = !str_cmp(argument, "auto");

	if (!check_blind(ch))
		return;

	if (fAuto)
		sprintf(buf, msg(EXITS, ch));
	else if (IS_IMMORTAL(ch))
		sprintf(buf, msg(OBVIOUS_EXITS_IMM, ch),
			ch->in_room->vnum);
	else
		sprintf(buf, msg(OBVIOUS_EXITS, ch));

	found = FALSE;
	for (door = 0; door <= 5; door++) {
		char* p;

		if ((pexit = ch->in_room->exit[door]) != NULL
		&&  pexit->u1.to_room != NULL
		&&  can_see_room(ch,pexit->u1.to_room)
		&&  !IS_SET(pexit->exit_info, EX_CLOSED)) {
			found = TRUE;
			if (fAuto) {
				strcat(buf, " ");
				strcat(buf, dir_name[door]);
			}
			else {
				sprintf(strchr(buf, '\0'), "{C%-5s{x - %s",
					capitalize(dir_name[door]),
					room_dark(pexit->u1.to_room) ?
					msg(TOO_DARK_TO_TELL, ch) :
					pexit->u1.to_room->name);
				p = strend(buf);
				if (IS_IMMORTAL(ch))
					sprintf(p, msg(ROOM_D, ch),
						pexit->u1.to_room->vnum);
				else
					sprintf(p, "\n\r");
			}
		}

		if (number_percent() < get_skill(ch,gsn_perception)
		&&  (pexit = ch->in_room->exit[door]) != NULL
		&&  pexit->u1.to_room != NULL
		&&  can_see_room(ch,pexit->u1.to_room)
		&&   IS_SET(pexit->exit_info, EX_CLOSED)) {
			check_improve(ch,gsn_perception, TRUE, 5);
			found = TRUE;

			p = strchr(buf, '\0');
			if (fAuto)
				sprintf(p, " %s*", dir_name[door]);
			else {
				sprintf(p, "%-5s * (%s)",
					capitalize(dir_name[door]),
					pexit->keyword);

				p = strend(buf);
				if (IS_IMMORTAL(ch))
					sprintf(p, msg(ROOM_D, ch),
						pexit->u1.to_room->vnum);
				else
					sprintf(p, "\n\r");
			}
		}
	}

	if (!found)
		strcat(buf, fAuto ? msg(NONE, ch) : msg(CAPNONE, ch));

	if (fAuto)
		strcat(buf, "]{x\n\r");

	send_to_char(buf, ch);
	return;
}


void do_worth(CHAR_DATA *ch, char *argument)
{
	char_printf(ch, msg(HAVE_D_GOLD_D_SILVER, ch),
		    ch->gold, ch->silver);
	if (!IS_NPC(ch) && ch->level < LEVEL_HERO)
		char_printf(ch, msg(AND_D_EXP, ch),
			    ch->exp, exp_to_level(ch, ch->pcdata->points));
	char_puts(".\n\r", ch);

	if (!IS_NPC(ch))
		char_printf(ch, msg(HAVE_KILLED, ch),
			    ch->pcdata->has_killed,
			    IS_GOOD(ch) ? msg(NON_GOODS, ch) :
			    IS_EVIL(ch) ? msg(NON_EVILS, ch) : 
					  msg(NON_NEUTRALS, ch),
			    ch->pcdata->anti_killed,
			    IS_GOOD(ch) ? msg(GOODS, ch) :
			    IS_EVIL(ch) ? msg(EVILS, ch) : 
					  msg(NEUTRALS, ch));
	return;
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


void do_time(CHAR_DATA *ch, char *argument)
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
			   ITS_S,
			   (time_info.hour>=5 && time_info.hour<9) ?
						msg(TIME_DAWN, ch) :
			   (time_info.hour>=9 && time_info.hour<12) ?
						msg(TIME_MORNING, ch) :
			   (time_info.hour>=12 && time_info.hour<18) ?
						msg(TIME_MID_DAY, ch) :
			   (time_info.hour>=18 && time_info.hour<21) ?
						msg(TIME_EVENING, ch) :
			   msg(TIME_NIGHT, ch));

	if (!IS_IMMORTAL(ch))
		return;

	char_printf(ch, "MUDDY started up at %s\n\r"
			"The system time is %s.\n\r",
			str_boot_time, (char*) ctime(&current_time));
	return;
}


void do_date(CHAR_DATA *ch, char *argument)
{
	time_t t = time(NULL);
	char_puts(ctime(&t), ch);
}


void do_weather(CHAR_DATA *ch, char *argument)
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
	return;
}


void do_help(CHAR_DATA *ch, char *argument)
{
	HELP_DATA *pHelp;
	char argall[MAX_INPUT_LENGTH],argone[MAX_INPUT_LENGTH];

	if (argument[0] == '\0')
		argument = "summary";

	/* this parts handles help a b so that it returns help 'a b' */
	argall[0] = '\0';
	while (argument[0] != '\0') {
		argument = one_argument(argument,argone);
		if (argall[0] != '\0')
			strcat(argall," ");
		strcat(argall, argone);
	}

	for (pHelp = help_first; pHelp != NULL; pHelp = pHelp->next) {
		if (pHelp->level > get_trust(ch))
			continue;

		if (is_name(argall, pHelp->keyword)) {
			if (pHelp->level >= 0 && str_cmp(argall, "imotd"))
				char_printf(ch, "%s\n\r", pHelp->keyword);

			/*
			 * Strip leading '.' to allow initial blanks.
			 */
			if (pHelp->text[0] == '.')
				page_to_char(pHelp->text+1, ch);
			else
				page_to_char(pHelp->text  , ch);
			return;
		}
	}

	send_to_char(msg(NO_HELP_ON_WORD, ch), ch);
	return;
}


static void do_who_raw(CHAR_DATA* ch, CHAR_DATA *wch, char* output)
{
	char const *race;
	char const *class;
	char *pk;
/*	char *clan; */
	char *act;
	char *title;
	char level[100];
	int trusted;

	/*
	 * Figure out what to print for class.
	 */
	class = class_table[wch->class].who_name;
	switch (wch->level) {
		case MAX_LEVEL - 0: class = " IMP    "; break;
		case MAX_LEVEL - 1: class = " CRE    "; break;
		case MAX_LEVEL - 2: class = " SUP    "; break;
		case MAX_LEVEL - 3: class = " DEI    "; break;
		case MAX_LEVEL - 4: class = " GOD    "; break;
		case MAX_LEVEL - 5: class = " IMM    "; break;
		case MAX_LEVEL - 6: class = " DEM    "; break;
		case MAX_LEVEL - 7: class = " ANG    "; break;
		case MAX_LEVEL - 8: class = " AVA    "; break;
	}

	if (IS_IMMORTAL(wch))
		race = "";
	else if (RACE(wch) < MAX_PC_RACE)
		race = pc_race_table[RACE(wch)].who_name;
	else 
		race = "     ";
/*
	if ((wch->clan && ch->clan ==  wch->clan)
	||  IS_IMMORTAL(ch)
	||  (IS_SET(wch->act, PLR_CANINDUCT) && wch->clan == 1)
	||  wch->clan == CLAN_HUNTER)
		clan = clan_table[wch->clan].short_name;
	else
		clan = EMPTY_STRING;
*/

	if (!((ch==wch && ch->level<PK_MIN_LEVEL) || is_safe_nomessage(ch,wch)))
		pk = "{R(PK){x ";
	else
		pk = EMPTY_STRING;

	if(IS_SET(wch->act, PLR_WANTED))
		act = "{R(WANTED){x";
	else
		act = EMPTY_STRING;

	if (IS_NPC(wch))
		title = " Believer of Chronos";
	else
		title = wch->pcdata->title;

	/*
	 * Format it up.
	 */
	sprintf(level, "%3d", wch->level);
	trusted = IS_TRUSTED(ch, LEVEL_IMMORTAL) || ch == wch ||
		  wch->level >= LEVEL_HERO;
	sprintf(strend(output), "[{C%s{x %s {Y%s{x] %s{x%s{x%s{x%s{x\n\r",
		trusted ? level
			: (get_curr_stat(wch, STAT_CHA) < 18) ? level : "   ",
		race,
		class,
		pk,
		act,
		wch->name,
		title);
}


void do_who(CHAR_DATA *ch, char *argument)
{
	char output[4 * MAX_STRING_LENGTH];
	DESCRIPTOR_DATA *d;
	int iClass;
	int iRace;
	int iLevelLower;
	int iLevelUpper;
	int nNumber;
	int nMatch;
	int vnum;
	int count;
	bool rgfClass[MAX_CLASS];
	bool rgfRace[MAX_PC_RACE];
	bool fClassRestrict;
	bool fRaceRestrict;
	bool fImmortalOnly;
	bool fPKRestrict;
	bool fRulerRestrict;
	bool fChaosRestrict;
	bool fShalafiRestrict;
	bool fInvaderRestrict;
	bool fBattleRestrict;
	bool fKnightRestrict;
	bool fLionsRestrict;
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
	fBattleRestrict = FALSE;
	fChaosRestrict = FALSE;
	fRulerRestrict = FALSE;
	fInvaderRestrict = FALSE;
	fShalafiRestrict = FALSE;
	fKnightRestrict = FALSE;
	fLionsRestrict = FALSE;
	vnum = 0;
	fTattoo = FALSE;

	for (iClass = 0; iClass < MAX_CLASS; iClass++)
		rgfClass[iClass] = FALSE;
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

		if (!str_cmp(arg,"ruler")) {
			if (ch->clan != CLAN_RULER && !IS_IMMORTAL(ch)) {
				send_to_char("You are not in that clan!\n\r",
					     ch);
				return;
			}
			else {
				fRulerRestrict = TRUE;
				break;
			}
		}

		if (!str_cmp(arg,"shalafi")) {
			if (ch->clan != CLAN_SHALAFI && !IS_IMMORTAL(ch)) {
				send_to_char("You are not in that clan!\n\r",
					     ch);
				return;
			}
			else {
				fShalafiRestrict = TRUE;
				break;
			}
			}

		if (!str_cmp(arg,"battle")) {
			if (ch->clan != CLAN_BATTLE && !IS_IMMORTAL(ch)) {
				send_to_char("You are not in that clan!\n\r",
					     ch);
				return;
			}
			else {
				fBattleRestrict = TRUE;
				return;
			}
		}

		if (!str_cmp(arg,"invader")) {
			if (ch->clan != CLAN_INVADER && !IS_IMMORTAL(ch)) {
				send_to_char("You are not in that clan!\n\r",
					     ch);
				return;
			}
		}
		else {
			fInvaderRestrict = TRUE;
			break;
		}
		if (!str_cmp(arg,"chaos")) {
			if (ch->clan != CLAN_CHAOS && !IS_IMMORTAL(ch)) {
				send_to_char("You are not in that clan!\n\r",
					     ch);
				return;
			}
			else {
				fChaosRestrict = TRUE;
				break;
			}
		}

		if (!str_cmp(arg,"knight")) {
			if (ch->clan != CLAN_KNIGHT && !IS_IMMORTAL(ch)) {
				send_to_char("You are not in that clan!\n\r",
					     ch);
				return;
			}
			else {
				fKnightRestrict = TRUE;
				break;
			}
		}

		if (!str_cmp(arg,"lions")) {
			if (ch->clan != CLAN_LIONS && !IS_IMMORTAL(ch)) {
				send_to_char("You are not in that clan!\n\r",
					     ch);
				return;
			}
			else {
				fLionsRestrict = TRUE;
				break;
			}
		}

		if (!str_cmp(arg,"tattoo")) {
			if (get_eq_char(ch,WEAR_TATTOO) == NULL) {
				send_to_char("You haven't got a tattoo yet!\n\r",ch);
				return;
			}
			else {
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
		}
		else {
			/*
			 * Look for classes to turn on.
			 */
			if (arg[0] == 'i')
				fImmortalOnly = TRUE;
			else {
				iClass = class_lookup(arg);
				if (iClass == -1 || !IS_IMMORTAL(ch)) {
					iRace = race_lookup(arg);

					if (iRace == 0 || iRace >= MAX_PC_RACE) {
						send_to_char("That's not a "
							     "valid race.\n\r",
							     ch);
						return;
					}
					else {
						fRaceRestrict = TRUE;
						rgfRace[iRace] = TRUE;
					}
				}
				else {
					fClassRestrict = TRUE;
					rgfClass[iClass] = TRUE;
				}
			}
		}
	}

	/*
	 * Now show matching chars.
	 */
	nMatch = 0;
	output[0] = '\0';
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
		||  (fClassRestrict && !rgfClass[wch->class])
		||  (fRaceRestrict && !rgfRace[RACE(wch)])
		||  (fPKRestrict && is_safe_nomessage(ch,wch))
		||  (fTattoo && (vnum == get_eq_char(wch,WEAR_TATTOO)->pIndexData->vnum))
		||  (fRulerRestrict && wch->clan != CLAN_RULER)
		||  (fChaosRestrict && wch->clan != CLAN_CHAOS)
		||  (fBattleRestrict && wch->clan != CLAN_BATTLE)
		||  (fInvaderRestrict && wch->clan != CLAN_INVADER)
		||  (fShalafiRestrict && wch->clan != CLAN_SHALAFI)
		||  (fKnightRestrict && wch->clan != CLAN_KNIGHT)
		||  (fLionsRestrict && wch->clan != CLAN_LIONS))
			continue;

		nMatch++;
		do_who_raw(ch, wch, output);
	}

	count = 0;
	for (d = descriptor_list; d != NULL; d = d->next)
		count += (d->connected == CON_PLAYING);

	max_on = UMAX(count, max_on);
	sprintf(strend(output),
		msg(PLAYERS_FOUND, ch),
		nMatch, max_on);
	page_to_char(output, ch);
	return;
}


/* whois command */
void do_whois (CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	char output[4*MAX_STRING_LENGTH];
	DESCRIPTOR_DATA *d;
	bool found = FALSE;

	one_argument(argument, arg);
	if (arg[0] == '\0') {
		send_to_char(msg(MUST_PROVIDE_NAME, ch), ch);
		return;
	}

	output[0] = '\0';
	for (d = descriptor_list; d != NULL; d = d->next) {
		CHAR_DATA *wch;

		if (d->connected != CON_PLAYING || !can_see(ch,d->character))
				continue;

		if (d->connected != CON_PLAYING
		    || (IS_VAMPIRE(d->character)
		    && !IS_IMMORTAL(ch) && (ch != d->character)))
				continue;

		wch = (d->original != NULL) ? d->original : d->character;

		if (!can_see(ch,wch))
				continue;

		if (!str_prefix(arg,wch->name)) {
			found = TRUE;
			do_who_raw(ch, wch, output);
		}
	}

	if (!found) {
		send_to_char(msg(NO_ONE_THAT_NAME, ch), ch);
		return;
	}

	page_to_char(output, ch);
}


void do_count(CHAR_DATA *ch, char *argument)
{
	int count;
	DESCRIPTOR_DATA *d;

	count = 0;

	for (d = descriptor_list; d != NULL; d = d->next)
		if (d->connected == CON_PLAYING && can_see(ch, d->character))
			count++;

	max_on = UMAX(count,max_on);

	char_printf(ch, msg(D_CHARS_ON, ch), count);
	if (max_on == count)
		char_puts(msg(MOST_SO_FAR_TODAY, ch), ch);
	else
		char_printf(ch, msg(MOST_TODAY_WAS, ch), max_on);
	char_puts(".\n\r", ch);
}


void do_inventory(CHAR_DATA *ch, char *argument)
{
	send_to_char(msg(YOU_ARE_CARRYING, ch), ch);
	show_list_to_char(ch->carrying, ch, TRUE, TRUE);
	return;
}


void do_equipment(CHAR_DATA *ch, char *argument)
{
	OBJ_DATA *obj;
	int i;
	bool found;

	send_to_char(msg(YOU_ARE_USING, ch), ch);
	found = FALSE;
	for (i = 0; show_order[i] >= 0; i++) {
		if ((obj = get_eq_char(ch, show_order[i])) == NULL)
			continue;

		send_to_char(msg(EQ_USED_AS_LIGHT + show_order[i], ch), ch);
		if (can_see_obj(ch, obj))
			char_printf(ch, "%s\n\r",
				    format_obj_to_char(obj, ch, TRUE));
		else
			send_to_char(msg(SOMETHING, ch), ch);
		found = TRUE;
	}

	for(obj = ch->carrying; obj != NULL; obj = obj->next_content) {
		if (obj->wear_loc != WEAR_STUCK_IN)
			continue;

		send_to_char(msg(EQ_STUCK_IN, ch), ch);
		if (can_see_obj(ch, obj))
			char_printf(ch, "%s\n\r",
				    format_obj_to_char(obj, ch, TRUE));
		else
			send_to_char(msg(SOMETHING, ch), ch);
		found = TRUE;
	}

	if (!found)
		send_to_char(msg(NOTHING, ch), ch);

	return;
}


void do_compare(CHAR_DATA *ch, char *argument)
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
		send_to_char(msg(COMPARE_WHAT, ch), ch);
		return;
	}

	if ((obj1 = get_obj_carry(ch, arg1)) == NULL) {
		send_to_char(msg(DONT_HAVE_ITEM, ch), ch);
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
			send_to_char(msg(ARENT_WEAR_COMPARABLE, ch), ch);
			return;
		}
	}
	else if ((obj2 = get_obj_carry(ch,arg2)) == NULL) {
		send_to_char(msg(DONT_HAVE_ITEM, ch), ch);
		return;
	}

	cmsg		= NULL;
	value1	= 0;
	value2	= 0;

	if (obj1 == obj2)
		cmsg = msg(COMPARE_P_TO_ITSELF, ch);
	else if (obj1->item_type != obj2->item_type)
		cmsg = msg(CANT_COMPARE_P_P, ch);
	else {
		switch (obj1->item_type) {
		default:
			cmsg = msg(CANT_COMPARE_P_P, ch);
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
			cmsg = msg(P_P_LOOKS_SAME, ch);
		else if (value1  > value2)
			cmsg = msg(P_LOOKS_BETTER_P, ch);
		else
			cmsg = msg(P_LOOKS_WORSE_P, ch);

	act(cmsg, ch, obj1, obj2, TO_CHAR);
	return;
}


void do_credits(CHAR_DATA *ch, char *argument)
{
	do_help(ch, "muddy");
	return;
}


void do_where(CHAR_DATA *ch, char *argument)
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
		send_to_char(msg(TOO_DARK_SEE, ch), ch);
		return;
	}

	if (!str_cmp(arg,"pk"))
		fPKonly = TRUE;

	if (arg[0] == '\0' || fPKonly) {
		send_to_char(msg(PLAYERS_NEAR_YOU, ch), ch);
		found = FALSE;
		for (d = descriptor_list; d; d = d->next) {
			if (d->connected == CON_PLAYING
			&& (victim = d->character) != NULL
			&&   !IS_NPC(victim)
			&& !(fPKonly && is_safe_nomessage(ch,victim))
			&&   victim->in_room != NULL
			&&   victim->in_room->area == ch->in_room->area
			&&   can_see(ch, victim)) {
				found = TRUE;
				char_printf(ch, "%s%-28s %s\n\r",
		(is_safe_nomessage(ch, (is_affected(victim,gsn_doppelganger) && victim->doppel) ? victim->doppel : victim) || IS_NPC(victim)) ?  "     " :
		"{r(PK){x ",
		(is_affected(victim,gsn_doppelganger) && !IS_SET(ch->act,PLR_HOLYLIGHT)) ?  victim->doppel->name : victim->name,
		victim->in_room->name);
			}
		}
		if (!found)
			send_to_char(msg(CAPNONE, ch), ch);
	}
	else {
		found = FALSE;
		for (victim = char_list;
		     victim != NULL; victim = victim->next) {
			if (victim->in_room != NULL
			&&   victim->in_room->area == ch->in_room->area
			&&   !IS_AFFECTED(victim, AFF_HIDE)
			&&   !IS_AFFECTED(victim, AFF_FADE)
			&&   !IS_AFFECTED(victim, AFF_SNEAK)
			&&   can_see(ch, victim)
			&&   is_name(arg, victim->name)) {
				found = TRUE;
				char_printf(ch, "%-28s %s\n\r",
					    PERS(victim, ch),
					    victim->in_room->name);
				break;
			}
		}
		if (!found)
			act_nprintf(ch, NULL, arg, TO_CHAR, POS_DEAD,
					DIDNT_FIND_ANY);
	}
}


void do_consider(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	char *cmsg;
	char *align;
	int diff;

	one_argument(argument, arg);

	if (arg[0] == '\0') {
		send_to_char(msg(CONSIDER_WHOM, ch), ch);
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL) {
		send_to_char(msg(THEY_ARENT_HERE, ch), ch);
		return;
	}

	if (is_safe(ch, victim)) {
		send_to_char(msg(DONT_EVEN_THINK, ch), ch);
		return;
	}

	diff = victim->level - ch->level;

	     if (diff <= -10) cmsg = msg(CAN_KILL_NAKED, ch);
	else if (diff <=  -5) cmsg = msg(IS_NOT_MATCH_FOR_YOU, ch);
	else if (diff <=  -2) cmsg = msg(LOOKS_EASY_KILL, ch);
	else if (diff <=   1) cmsg = msg(PERFECT_MATCH, ch);
	else if (diff <=   4) cmsg = msg(FEEL_LUCKY_PUNK, ch);
	else if (diff <=   9) cmsg = msg(LAUGHS_AT_YOU_MERCILESSLY, ch);
	else		      cmsg = msg(DEATH_WILL_THANK_YOU, ch);

	if (IS_EVIL(ch) && IS_EVIL(victim))
		align = msg(GRINS_EVILLY_WITH, ch);
	else if (IS_GOOD(victim) && IS_GOOD(ch))
		align = msg(GREETS_YOU_WARMLY, ch);
	else if (IS_GOOD(victim) && IS_EVIL(ch))
		align = msg(HOPE_YOU_WILL_TURN, ch);
	else if (IS_EVIL(victim) && IS_GOOD(ch))
		align = msg(GRINS_EVILLY_AT, ch);
	else if (IS_NEUTRAL(ch) && IS_EVIL(victim))
		align = msg(GRINS_EVILLY, ch);
	else if (IS_NEUTRAL(ch) && IS_GOOD(victim))
		align = msg(SMILES_HAPPILY, ch);
	else if (IS_NEUTRAL(ch) && IS_NEUTRAL(victim))
		align = msg(LOOKS_AS_YOU, ch);
	else
		align = msg(LOOKS_VERY_DISINTERESTED, ch);

	act(cmsg, ch, NULL, victim, TO_CHAR);
	act(align, ch, NULL, victim, TO_CHAR);
	return;
}


void set_title(CHAR_DATA *ch, char *title, ...)
{
	va_list ap;
	char buf[MAX_STRING_LENGTH];
	char* p;
	static char nospace[] = ".,!?";

	if (IS_NPC(ch)) {
		bug("Set_title: NPC.", 0);
		return;
	}

	if (strchr(nospace, title[0]))
		p = buf;
	else {
		buf[0] = ' ';
		p = buf+1;
	}

	va_start(ap, title);
	vsprintf(p, title, ap);
	va_end(ap);

	free_string(ch->pcdata->title);
	ch->pcdata->title = str_dup(buf);
}


void do_title(CHAR_DATA *ch, char *argument)
{
	if (IS_NPC(ch))
		return;

	if (CANT_CHANGE_TITLE(ch)) {
		send_to_char(msg(CANT_CHANGE_TITLE, ch), ch);
		return;
	}

	if (argument[0] == '\0') {
		send_to_char(msg(CHANGE_TITLE_TO_WHAT, ch), ch);
		return;
	}

	if (strchr(argument, '%') != NULL) {
		send_to_char(msg(ILLEGAL_CHARACTER_TITLE, ch), ch);
		return;
	}

	if (strlen(argument) > 45)
		argument[45] = '\0';

	smash_tilde(argument);
	set_title(ch, argument);
	send_to_char(msg(OK, ch), ch);
}


void do_description(CHAR_DATA *ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];

	if (argument[0] != '\0') {
		buf[0] = '\0';
		smash_tilde(argument);

		if (argument[0] == '-') {
			int len;
			bool found = FALSE;

			if (ch->description == NULL
			||  ch->description[0] == '\0') {
				send_to_char(msg(NO_LINES_TO_REMOVE, ch),
					     ch);
				return;
			}

			strcpy(buf, ch->description);

			for (len = strlen(buf); len > 0; len--) {
				if (buf[len] == '\r') {
					if (!found) { /* back it up */
						if (len > 0)
							len--;
						found = TRUE;
					}
					else { /* found the second one */
						buf[len + 1] = '\0';
						free_string(ch->description);
						ch->description = str_dup(buf);
						send_to_char(	
						    msg(YOUR_DESC_IS, ch), 						    ch);
						send_to_char(ch->description ?
							ch->description :
						      msg(BR_NONE_BR, ch), 
						      ch);
						return;
					}
				}
			}
			buf[0] = '\0';
			free_string(ch->description);
			ch->description = str_dup(buf);
			send_to_char(msg(DESC_CLEARED, ch), ch);
			return;
		}

		if (argument[0] == '+') {
			if (ch->description != NULL)
				strcat(buf, ch->description);
			while (isspace(*++argument));

			if (strlen(buf) + strlen(argument) 
				>= MAX_STRING_LENGTH - 2) {
				send_to_char(msg(DESC_TOO_LONG, ch), ch);
				return;
			}

			strcat(buf, argument);
			strcat(buf, "\n\r");
			free_string(ch->description);
			ch->description = str_dup(buf);
		}
	}

	send_to_char(msg(YOUR_DESC_IS, ch), ch);
	send_to_char(ch->description ? ch->description : 
					msg(BR_NONE_BR, ch), ch);
	return;
}


void do_report(CHAR_DATA *ch, char *argument)
{
	act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING, REPORT_I_HAVE,
	       ch->hit, ch->max_hit,
	       ch->mana, ch->max_mana,
	       ch->move, ch->max_move);
	act_nprintf(ch, NULL, NULL, TO_CHAR, POS_DEAD, REPORT_I_HAVE_TO_CH,
	       ch->hit, ch->max_hit,
	       ch->mana, ch->max_mana,
	       ch->move, ch->max_move);
}


/*
 * 'Wimpy' originally by Dionysos.
 */
void do_wimpy(CHAR_DATA *ch, char *argument)
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


void do_password(CHAR_DATA *ch, char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char *pArg;
	char *pwdnew;
	char cEnd;

	if (IS_NPC(ch))
		return;

	/*
	 * Can't use one_argument here because it smashes case.
	 * So we just steal all its code.  Bleagh.
	 */
	pArg = arg1;
	while (isspace(*argument))
		argument++;

	cEnd = ' ';
	if (*argument == '\'' || *argument == '"')
		cEnd = *argument++;

	while (*argument != '\0') {
		if (*argument == cEnd) {
			argument++;
			break;
		}
		*pArg++ = *argument++;
	}
	*pArg = '\0';

	pArg = arg2;
	while (isspace(*argument))
		argument++;

	cEnd = ' ';
	if (*argument == '\'' || *argument == '"')
		cEnd = *argument++;

	while (*argument != '\0') {
		if (*argument == cEnd) {
			argument++;
			break;
		}
		*pArg++ = *argument++;
	}
	*pArg = '\0';

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
	save_char_obj(ch);
	send_to_char("Ok.\n\r", ch);
	return;
}

/* RT configure command */

void do_scan(CHAR_DATA *ch, char *argument)
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
		send_to_char(msg(WRONG_DIRECTION, ch), ch);
		return;
	}

	char_printf(ch, msg(YOU_SCAN_S, ch), dir2);
	act_nprintf(ch, NULL, NULL, TO_ROOM, POS_RESTING, N_SCANS_S, dir2);

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
			send_to_char(msg(SCAN_DOOR_CLOSED, ch), ch);
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

void do_request(CHAR_DATA *ch, char *argument)
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
	act("You request $p from $N.",	 ch, obj, victim, TO_CHAR  );
	act("$n requests $p from you.", ch, obj, victim, TO_VICT  );

	if (IS_SET(obj->progtypes,OPROG_GIVE))
		(obj->pIndexData->oprogs->give_prog) (obj,ch,victim);

	if (IS_SET(victim->progtypes,MPROG_GIVE))
		(victim->pIndexData->mprogs->give_prog) (victim,ch,obj);

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
	affect_to_char (ch, &af);

	return;
}


void do_hometown(CHAR_DATA *ch, char *argument)
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


void do_detect_hidden(CHAR_DATA *ch, char *argument)
{
	AFFECT_DATA af;


	if (IS_NPC(ch)
	||  ch->level < skill_table[gsn_detect_hidden].skill_level[ch->class]) {
		send_to_char(msg(HUH, ch), ch);
		return;
	}

	if (CAN_DETECT(ch, DETECT_HIDDEN)) {
		send_to_char("You are already as alert as you can be. \n\r",ch);
		return;
	}

	if (number_percent() > get_skill(ch, gsn_detect_hidden)) {
		send_to_char("You peer intently at the shadows "
			     "but they are unrevealing.\n\r", ch);
		return;
	}
	af.where     = TO_DETECTS;
	af.type      = gsn_detect_hidden;
	af.level     = ch->level;
	af.duration  = ch->level;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = DETECT_HIDDEN;
	affect_to_char(ch, &af);
	send_to_char("Your awareness improves.\n\r", ch);
	return;
}


void do_bear_call(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *gch;
	CHAR_DATA *bear;
	CHAR_DATA *bear2;
	AFFECT_DATA af;
	int i;

	if (IS_NPC(ch)
	||  ch->level < skill_table[gsn_bear_call].skill_level[ch->class]) {
		send_to_char(msg(HUH, ch), ch);
		return;
	}

	send_to_char("You call for bears help you.\n\r",ch);
	act("$n shouts a bear call.",ch,NULL,NULL,TO_ROOM);

	if (is_affected(ch, gsn_bear_call)) {
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
	&&  IS_SET(ch->in_room->room_flags, ROOM_NO_MOB)) {
		send_to_char("No bears listen you.\n\r", ch);
		return;
	}

	if (number_percent() > get_skill(ch, gsn_bear_call)) {
		send_to_char("No bears listen you.\n\r", ch);
		check_improve(ch,gsn_bear_call,TRUE,1);
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

	if (ch->mana < 125) {
		send_to_char("You don't have enough mana "
			     "to shout a bear call.\n\r", ch);
		return;
	}
	ch->mana -= 125;

	check_improve(ch, gsn_bear_call, TRUE, 1);
	bear = create_mobile(get_mob_index(MOB_VNUM_BEAR));

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

	bear2 = create_mobile(bear->pIndexData);
	clone_mobile(bear, bear2);

	SET_BIT(bear->affected_by, AFF_CHARM);
	SET_BIT(bear2->affected_by, AFF_CHARM);
	bear->master = bear2->master = ch;
	bear->leader = bear2->leader = ch;

	char_to_room(bear, ch->in_room);
	char_to_room(bear2, ch->in_room);
	send_to_char("Two bears come to your rescue!\n\r",ch);
	act("Two bears come to $n's rescue!",ch,NULL,NULL,TO_ROOM);

	af.where	      = TO_AFFECTS;
	af.type 	      = gsn_bear_call;
	af.level	      = ch->level;
	af.duration	      = 24;
	af.bitvector	      = 0;
	af.modifier	      = 0;
	af.location	      = APPLY_NONE;
	affect_to_char(ch, &af);

}


void do_identify(CHAR_DATA *ch, char *argument)
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
		act("$n looks at you!\n\r", rch, obj, ch, TO_VICT);
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


size_t cstrlen(char* cstr)
{
	size_t res;

	if (cstr == NULL)
		return 0;

	res = strlen(cstr);
	while ((cstr = strchr(cstr, '{')) != NULL) {
		if (*(cstr+1) == '{')
			res--;
		else
			res -= 2;
		cstr += 2;
	}

	return res;
}

void do_score(CHAR_DATA *ch, char *argument)
{
	char buf2[MAX_INPUT_LENGTH];
	char title[MAX_STRING_LENGTH];
	int ekle = 0;
	int delta;

	send_to_char("\n\r      {G/~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~/~~\\{x\n\r", ch);

	strcpy(title, IS_NPC(ch) ? "Believer of Chronos." : ch->pcdata->title);
	delta = strlen(title) - cstrlen(title);
	title[32+delta] = '\0';
	sprintf(buf2, "     {G|   {W%%-12s{x%%-%ds {Y%%3d years old   {G|____|{x\n\r", 33+delta);
	char_printf(ch, buf2, ch->name, title, get_age(ch));

	send_to_char("     {G|{C+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+{G|{x\n\r", ch);

	char_printf(ch, "     {G| {RLevel:  {x%3d          {C|  {RStr:  {x%2d(%2d)  {C| {RReligion  :  {x%-10s {G|{x\n\r",
		ch->level,ch->perm_stat[STAT_STR],get_curr_stat(ch,STAT_STR),
		      (ch->religion <= RELIGION_NONE) ||
		 (ch->religion > MAX_RELIGION) ? "none" :
		religion_table[ch->religion].leader);

	char_printf(ch,
"     {G| {RRace :  {x%-11s  {C|  {RInt:  {x%2d(%2d)  {C| {RPractice  :   {x%3d       {G|{x\n\r",
		race_table[ORG_RACE(ch)].name,
		ch->perm_stat[STAT_INT], get_curr_stat(ch,STAT_INT),
		ch->practice);

	char_printf(ch,
"     {G| {RSex  :  {x%-11s  {C|  {RWis:  {x%2d(%2d)  {C| {RTrain     :   {x%3d       {G|{x\n\r",
	ch->sex == 0 ? "sexless" : ch->sex == 1 ? "male" : "female",
	ch->perm_stat[STAT_WIS], get_curr_stat(ch,STAT_WIS),ch->train);

	char_printf(ch,
"     {G| {RClass:  {x%-12s {C|  {RDex:  {x%2d(%2d)  {C| {RQuest Pnts:  {x%4d       {G|{x\n\r",
		IS_NPC(ch) ? "mobile" : class_table[ch->class].name,
		ch->perm_stat[STAT_DEX], get_curr_stat(ch,STAT_DEX),
		IS_NPC(ch) ? 0 : ch->pcdata->questpoints);

	char_printf(ch,
"     {G| {RHome :  {x%-12s {C|  {RCon:  {x%2d(%2d)  {C| {R%-10s:   {x%3d       {G|{x\n\r",
		IS_NPC(ch) ? "Midgaard" : hometown_table[ch->hometown].name,
		ch->perm_stat[STAT_CON], get_curr_stat(ch,STAT_CON),
		IS_NPC(ch) ? "Quest?" : (IS_ON_QUEST(ch) ? "Quest Time" : "Next Quest"),
		IS_NPC(ch) ? 0 : abs(ch->pcdata->questtime));
	char_printf(ch,
"     {G| {REthos:  {x%-11s  {C|  {RCha:  {x%2d(%2d)  {C| {R%s     :  {x%4d       {G|{x\n\r",
		IS_NPC(ch) ? "mobile" : ch->ethos == 1 ? "lawful" :
	ch->ethos == 2 ? "neutral" : ch->ethos == 3 ? "chaotic" : "none",
		ch->perm_stat[STAT_CHA], get_curr_stat(ch,STAT_CHA),
		ch->class == CLASS_SAMURAI ? "Death" : "Wimpy" ,
		ch->class == CLASS_SAMURAI ? ch->pcdata->death : ch->wimpy);

	char_printf(ch, "     {G| {RAlign:  {x%-11s  {C|                |{x %-7s %-19s {G|{x\n\r",
		IS_GOOD(ch) ? "good" : IS_EVIL(ch) ? "evil" : "neutral",
		msg(YOU_ARE, ch),
		msg(POS_NAME_DEAD + ch->position, ch));

	send_to_char("     {G|{C+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+{G|{x{x\n\r", ch);

	if (ch->guarding != NULL) {
		ekle = 1;
		char_printf(ch,
"     {G| {GYou are guarding: {x%-10s                                    {G|{x\n\r",
			    ch->guarding->name);
	}

	if (ch->guarded_by != NULL) {
		ekle = 1;
		char_printf(ch,
"     {G| {GYou are guarded by: {x%-10s                                  {G|{x\n\r",
			    ch->guarded_by->name);
	}

	if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10) {
		ekle = 1;
		char_printf(ch,
"     {G| {GYou are drunk.                                                  {G|{x\n\r");
	}

	if (!IS_NPC(ch) && ch->pcdata->condition[COND_THIRST] <= 0) {
		ekle = 1;
		char_printf(ch,
"     {G| {YYou are thirsty.                                                {G|{x\n\r");
	}
/*    if (!IS_NPC(ch) && ch->pcdata->condition[COND_FULL]   ==	0) */
	if (!IS_NPC(ch) && ch->pcdata->condition[COND_HUNGER] <= 0) {
		ekle = 1;
		char_printf(ch,
"     {G| {YYou are hungry.                                                 {G|{x\n\r");
	}

	if (!IS_NPC(ch) && ch->pcdata->condition[COND_BLOODLUST] <= 0) {
		ekle = 1;
		char_printf(ch,
"     {G| {YYou are hungry for blood.                                       {G|{x\n\r");
	}

	if (!IS_NPC(ch) && ch->pcdata->condition[COND_DESIRE] <=  0) {
		ekle = 1;
		char_printf(ch,
"     {G| {YYou are desiring your home.                                     {G|{x\n\r");
	}

	if (ch->last_fight_time != -1 && !IS_IMMORTAL(ch) &&
	    (current_time - ch->last_fight_time)<FIGHT_DELAY_TIME) {
		ekle = 1;
		char_printf(ch,
"     {G| {RYour adrenalin is gushing!                                      {G|{x\n\r");
	}

	if (ekle)
		char_printf(ch,
"     {G|{C+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+{G|{x\n\r");

	char_printf(ch,
"     {G| {RItems Carried :   {x%2d/%-4d           {RArmor vs magic  : {x%4d      {G|{x\n\r",
		ch->carry_number, can_carry_n(ch),
		GET_AC(ch,AC_EXOTIC));

	char_printf(ch,
"     {G| {RWeight Carried:  {x%4d/%-8d      {RArmor vs bash   : {x%4d      {G|{x\n\r",
	get_carry_weight(ch), can_carry_w(ch),GET_AC(ch,AC_BASH));

	char_printf(ch,
"     {G| {RGold          :   {Y%-10ld        {RArmor vs pierce : {x%4d      {G|{x\n\r",
		 ch->gold,GET_AC(ch,AC_PIERCE));

	char_printf(ch,
"     {G| {RSilver        :   {W%-10ld        {RArmor vs slash  : {x%4d      {G|{x\n\r",
		 ch->silver,GET_AC(ch,AC_SLASH));

	char_printf(ch,
"     {G| {RCurrent exp   :   {x%-6d            {RSaves vs Spell  : {x%4d      {G|{x\n\r",
		ch->exp,ch->saving_throw);

	char_printf(ch,
"     {G| {RExp to level  :   {x%-6d                                        {G|{x\n\r",
		IS_NPC(ch) ? 0 : exp_to_level(ch,ch->pcdata->points));

	char_printf(ch,
"     {G|                                     {RHitP: {x%5d / %5d         {G|{x\n\r",
		   ch->hit, ch->max_hit);
	char_printf(ch,
"     {G| {RHitroll       :   {x%-3d               {RMana: {x%5d / %5d         {G|{x\n\r",
		   GET_HITROLL(ch),ch->mana, ch->max_mana);
	char_printf(ch,
"     {G| {RDamroll       :   {x%-3d               {RMove: {x%5d / %5d         {G|{x\n\r",
		    GET_DAMROLL(ch), ch->move, ch->max_move);
	char_printf(ch, "  {G/~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~/   |{x\n\r");
	char_printf(ch, "  {G\\________________________________________________________________\\__/{x\n\r");

	if (IS_SET(ch->comm, COMM_SHOW_AFFECTS))
		do_affects(ch, NULL);
}

void do_oscore(CHAR_DATA *ch, char *argument)
{
	char buf2[MAX_STRING_LENGTH];
	int i;

	char_printf(ch,
		"%s {W%s{x%s, level {W%d{x, {c%d{x years old "
		"(%d hours).\n\r",
		msg(YOU_ARE, ch),
		ch->name,
		IS_NPC(ch) ? "" : ch->pcdata->title, ch->level, get_age(ch),
		(ch->played + (int) (current_time - ch->logon)) / 3600);

	if (get_trust(ch) != ch->level)
		char_printf(ch, "You are trusted at level %d.\n\r", get_trust(ch));

	char_printf(ch,
		"Race: {c%s{x  Sex: {c%s{x  Class: {c%s{x  "
		"Hometown: {c%s{x\n\r",
		race_table[ORG_RACE(ch)].name,
		ch->sex == 0 ? "sexless" : ch->sex == 1 ? "male" : "female",
		IS_NPC(ch) ? "mobile" : class_table[ch->class].name,
		IS_NPC(ch) ? "Midgaard" : hometown_table[ch->hometown].name);

	char_printf(ch,
		"You have {W%d{x/{c%d{x hit, {W%d{x/{c%d{x mana, "
		"{W%d{x/{c%d{x movement.\n\r",
		ch->hit, ch->max_hit, ch->mana, ch->max_mana,
		ch->move, ch->max_move);

	char_printf(ch,
		"You have {c%d{x practices and "
		"{c%d{x training sessions.\n\r",
		ch->practice, ch->train);

	char_printf(ch, "You are carrying {W%d{x/{c%d{x items "
		"with weight {W%ld{x/{c%d{x pounds.\n\r",
		ch->carry_number, can_carry_n(ch),
		get_carry_weight(ch), can_carry_w(ch));

	if (ch->level > 20 || IS_NPC(ch))
		char_printf(ch,
			"Str: {c%d{x({Y%d{x)  Int: {c%d{x({Y%d{x)  "
			"Wis: {c%d{x({Y%d{x)  Dex: {c%d{x({Y%d{x)  "
			"Con: {c%d{x({Y%d{x)  Cha: {c%d{x({Y%d{x)\n\r",
			ch->perm_stat[STAT_STR], get_curr_stat(ch, STAT_STR),
			ch->perm_stat[STAT_INT], get_curr_stat(ch, STAT_INT),
			ch->perm_stat[STAT_WIS], get_curr_stat(ch, STAT_WIS),
			ch->perm_stat[STAT_DEX], get_curr_stat(ch, STAT_DEX),
			ch->perm_stat[STAT_CON], get_curr_stat(ch, STAT_CON),
			ch->perm_stat[STAT_CHA], get_curr_stat(ch, STAT_CHA));
	else
		char_printf(ch,
			"Str: {c%-9s{x Wis: {c%-9s{x Con: {c%-9s{x\n\r"
			"Int: {c%-9s{x Dex: {c%-9s{x Cha: {c%-11s{x\n\r",
			get_stat_alias(ch, STAT_STR),
			get_stat_alias(ch, STAT_WIS),
			get_stat_alias(ch, STAT_CON),
			get_stat_alias(ch, STAT_INT),
			get_stat_alias(ch, STAT_DEX),
			get_stat_alias(ch, STAT_CHA));

	sprintf(buf2, "You have scored {c%d{x exp, and have %s%s%s.\n\r",
		ch->exp,
		ch->gold + ch->silver == 0 ? "no money" :
					     ch->gold ? "{Y%ld gold{x " : "",
		ch->silver ? "{W%ld silver{x " : "",
		ch->gold + ch->silver ? ch->gold + ch->silver == 1 ?
					"coin" : "coins" : "");
	if (ch->gold)
		char_printf(ch, buf2, ch->gold, ch->silver);
	else
		char_printf(ch, buf2, ch->silver);

	/* KIO shows exp to level */
	if (!IS_NPC(ch) && ch->level < LEVEL_HERO)
		char_printf(ch, "You need {c%d{x exp to level.\n\r",
			exp_to_level(ch,ch->pcdata->points));

	if (!IS_NPC(ch))
		char_printf(ch,
			"Quest Points: {c%d{x.  "
			"%s: {c%d{x.\n\r",
			ch->pcdata->questpoints, 
			IS_NPC(ch) ? "Quest?" : (IS_ON_QUEST(ch) ? 
					"Quest Time" : "Next Quest"),
			IS_NPC(ch) ? 0 : abs(ch->pcdata->questtime));

	if ((ch->class == CLASS_SAMURAI) && (ch->level >= 10))
		char_printf(ch, "Total {c%d{x deaths up to now.",
			ch->pcdata->death);
	else
		char_printf(ch, "Wimpy set to {c%d{x hit points.", ch->wimpy);

	if (ch->guarding != NULL)
		char_printf(ch, "  You are guarding: {W%s{x", ch->guarding->name);

	if (ch->guarded_by != NULL)
		char_printf(ch, "  You are guarded by: {W%s{x",
			ch->guarded_by->name);
	send_to_char("\n\r", ch);

	if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10)
		send_to_char("You are {cdrunk{x.\n\r", ch);

	if (!IS_NPC(ch) && ch->pcdata->condition[COND_THIRST] <= 0)
		send_to_char("You are {rthirsty{x.\n\r", ch);

/*	if (!IS_NPC(ch) && ch->pcdata->condition[COND_FULL] == 0) */
	if (!IS_NPC(ch) && ch->pcdata->condition[COND_HUNGER] <= 0)
		send_to_char("You are {rhungry{x.\n\r", ch);
	if (!IS_NPC(ch) && ch->pcdata->condition[COND_BLOODLUST] <= 0)
		send_to_char("You are {rhungry for {Rblood{x.\n\r", ch);
	if (!IS_NPC(ch) && ch->pcdata->condition[COND_DESIRE] <= 0)
		send_to_char("You are {rdesiring your home{x.\n\r", ch);

	char_printf(ch, "%s %s.\n\r",
		msg(YOU_ARE, ch),
		msg(POS_NAME_DEAD + ch->position, ch));

	if ((ch->position == POS_SLEEPING || ch->position == POS_RESTING ||
	     ch->position == POS_FIGHTING || ch->position == POS_STANDING)
	&& ch->last_fight_time != -1 && !IS_IMMORTAL(ch)
	&& (current_time - ch->last_fight_time) < FIGHT_DELAY_TIME)
		send_to_char("Your {radrenalin is gushing{x!\n\r", ch);

	/* print AC values */
	if (ch->level >= 25)
		char_printf(ch,
			"Armor: pierce: {c%d{x  bash: {c%d{x  "
			"slash: {c%d{x  magic: {c%d{x\n\r",
			GET_AC(ch, AC_PIERCE), GET_AC(ch, AC_BASH),
			GET_AC(ch, AC_SLASH), GET_AC(ch, AC_EXOTIC));

	for (i = 0; i < 4; i++) {
		static char* ac_name[4] = { "{cpiercing{x", "{cbashing{x",
					    "{cslashing{x", "{cmagic{x" };

		send_to_char("You are ", ch);
		if (GET_AC(ch,i) >= 101)
			char_printf(ch, "{chopelessly vulnerable{x to %s.\n\r",
				ac_name[i]);
		else if (GET_AC(ch,i) >= 80)
			char_printf(ch, "{cdefenseless against{x %s.\n\r",
				ac_name[i]);
		else if (GET_AC(ch,i) >= 60)
			char_printf(ch, "{cbarely protected{x from %s.\n\r",
				ac_name[i]);
		else if (GET_AC(ch,i) >= 40)
			char_printf(ch, "{cslightly armored{x against %s.\n\r",
				ac_name[i]);
		else if (GET_AC(ch,i) >= 20)
			char_printf(ch, "{csomewhat armored{x against %s.\n\r",
				ac_name[i]);
		else if (GET_AC(ch,i) >= 0)
			char_printf(ch, "{carmored{x against %s.\n\r",
				ac_name[i]);
		else if (GET_AC(ch,i) >= -20)
			char_printf(ch, "{cwell-armored{x against %s.\n\r",
				ac_name[i]);
		else if (GET_AC(ch,i) >= -40)
			char_printf(ch, "{cvery well-armored{x against %s.\n\r",
				ac_name[i]);
		else if (GET_AC(ch,i) >= -60)
			char_printf(ch, "{cheavily armored{x against %s.\n\r",
				ac_name[i]);
		else if (GET_AC(ch,i) >= -80)
			char_printf(ch, "{csuperbly armored{x against %s.\n\r",
				ac_name[i]);
		else if (GET_AC(ch,i) >= -100)
			char_printf(ch, "{calmost invulnerable{x to %s.\n\r",
				ac_name[i]);
		else
			char_printf(ch, "{cdivinely armored{x against %s.\n\r",
				ac_name[i]);
	}

	/* RT wizinvis and holy light */
	if (IS_IMMORTAL(ch)) {
		char_printf(ch, "Holy Light: %s",
			IS_SET(ch->act, PLR_HOLYLIGHT) ? "{con{x" : "{coff{x");

		if (ch->invis_level)
			char_printf(ch, "  Invisible: {clevel %d{x",
				ch->invis_level);

		if (ch->incog_level)
			char_printf(ch, "  Incognito: {clevel %d{x",
				ch->incog_level);
		send_to_char("\n\r", ch);
	}

	if (ch->level >= 20)
		char_printf(ch, "Hitroll: {W%d{x  Damroll: {W%d{x.\n\r",
			GET_HITROLL(ch), GET_DAMROLL(ch));

	send_to_char("You are ", ch);
	if (IS_GOOD(ch))
		send_to_char("good.",ch);
	else if (IS_EVIL(ch))
		send_to_char("evil.",ch);
	else
		send_to_char ("neutral.", ch);

	switch (ch->ethos) {
	case 1:
		send_to_char("  You have a lawful ethos.\n\r", ch);
		break;
	case 2:
		send_to_char("  You have a neutral ethos.\n\r", ch);
		break;
	case 3:
		send_to_char("  You have a chaotic ethos.\n\r", ch);
		break;
	default:
		if (!IS_NPC(ch))
			send_to_char("  You have no ethos, "
				     "report it to the gods!\n\r", ch);
	}

	if (IS_NPC(ch))
		ch->religion = 0; /* XXX */

	if ((ch->religion <= RELIGION_NONE) || (ch->religion > MAX_RELIGION))
		send_to_char("You don't believe any religion.\n\r",ch);
	else
		char_printf(ch,"Your religion is the way of %s.\n\r",
			religion_table[ch->religion].leader);

	if (IS_SET(ch->comm, COMM_SHOW_AFFECTS))
		do_affects(ch, NULL);
}


void do_affects(CHAR_DATA *ch, char *argument)
{
	AFFECT_DATA *paf, *paf_last = NULL;

	if (ch->affected == NULL) {
		char_nputs(NOT_AFFECTED_SPELLS, ch);
		return;
	}

	char_nputs(YOU_ARE_AFFECTED, ch);
	for (paf = ch->affected; paf != NULL; paf = paf->next) {
		if (paf_last != NULL && paf->type == paf_last->type)
			if (ch->level >= 20)
				char_puts("                      ", ch);
			else
				continue;
		else
			char_printf(ch, "%s {c%-15s{x", msg(AFF_SPELL, ch),
				    skill_table[paf->type].name);

		if (ch->level >= 20) {
			char_printf(ch, ": %s {c%s{x %s {c%d{x ",
				    msg(AFF_MODIFIES, ch),
				    affect_loc_name(paf->location),
				    msg(AFF_BY, ch),
				    paf->modifier);
			if (paf->duration == -1 || paf->duration == -2)
				char_nputs(AFF_PERMANENTLY, ch);
			else
				char_printf(ch, msg(AFF_FOR_D_HOURS, ch),
					    paf->duration);
		}
		send_to_char("\n\r", ch);
		paf_last = paf;
	}

	return;
}


void do_lion_call(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *gch;
	CHAR_DATA *bear;
	CHAR_DATA *bear2;
	AFFECT_DATA af;
	int i;

	if (IS_NPC(ch)
	||  ch->level < skill_table[gsn_lion_call].skill_level[ch->class]) {
		char_nputs(HUH, ch);
		return;
	}

	send_to_char("You call for lions help you.\n\r",ch);
	act("$n shouts a lion call.",ch,NULL,NULL,TO_ROOM);

	if (is_affected(ch, gsn_lion_call)) {
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
	&& IS_SET(ch->in_room->room_flags, ROOM_NO_MOB)) {
		send_to_char("No lions can listen you.\n\r", ch);
		return;
	}

	if (number_percent() > get_skill(ch, gsn_lion_call)) {
		send_to_char("No lions listen you.\n\r", ch);
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

	if (ch->mana < 125) {
		send_to_char("You don't have enough mana "
			     "to shout a lion call.\n\r", ch);
		return;
	}
	ch->mana -= 125;

	bear = create_mobile(get_mob_index(MOB_VNUM_LION));

	for (i=0;i < MAX_STATS; i++)
		bear->perm_stat[i] = UMIN(25,2 * ch->perm_stat[i]);

	bear->max_hit = IS_NPC(ch) ? ch->max_hit : ch->pcdata->perm_hit;
	bear->hit = bear->max_hit;
	bear->max_mana = IS_NPC(ch) ? ch->max_mana : ch->pcdata->perm_mana;
	bear->mana = bear->max_mana;
	bear->alignment = ch->alignment;
	bear->level = UMIN(100,1 * ch->level-2);
	for (i=0; i < 3; i++)
		bear->armor[i] = interpolate(bear->level,100,-100);
	bear->armor[3] = interpolate(bear->level,100,0);
	bear->sex = ch->sex;
	bear->gold = 0;

	bear2 = create_mobile(bear->pIndexData);
	clone_mobile(bear,bear2);

	SET_BIT(bear->affected_by, AFF_CHARM);
	SET_BIT(bear2->affected_by, AFF_CHARM);
	bear->master = bear2->master = ch;
	bear->leader = bear2->leader = ch;

	char_to_room(bear,ch->in_room);
	char_to_room(bear2,ch->in_room);
	send_to_char("Two lions come to your rescue!\n\r",ch);
	act("Two bears come to $n's rescue!",ch,NULL,NULL,TO_ROOM);

	af.where	      = TO_AFFECTS;
	af.type 	      = gsn_lion_call;
	af.level	      = ch->level;
	af.duration	      = 24;
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

	if	(istat >  99) stat = msg(COND_EXCELLENT, ch);
	else if (istat >= 80) stat = msg(COND_GOOD, ch);
	else if (istat >= 60) stat = msg(COND_FINE, ch);
	else if (istat >= 40) stat = msg(COND_AVERAGE, ch);
	else if (istat >= 20) stat = msg(COND_POOR, ch);
	else			stat = msg(COND_FRAGILE, ch);

	return stat;
}


/* room affects */
void do_raffects(CHAR_DATA *ch, char *argument)
{
	AFFECT_DATA *paf, *paf_last = NULL;

	if (ch->in_room->affected == NULL) {
		send_to_char("The room is not affected by any spells.\n\r",ch);
		return;
	}

	send_to_char("The room is affected by the following spells:\n\r", ch);
	for (paf = ch->in_room->affected; paf != NULL; paf = paf->next) {
		if (paf_last != NULL && paf->type == paf_last->type)
			if (ch->level >= 20)
				char_puts("                      ", ch);
			else
				continue;
		else
			char_printf(ch, "%s {c%-15s{x", msg(AFF_SPELL, ch),
				    skill_table[paf->type].name);

		if (ch->level >= 20) {
			char_printf(ch, ": %s {c%s{x %s {c%d{x ",
				    msg(AFF_MODIFIES, ch),
				    raffect_loc_name(paf->location),
				    msg(AFF_BY, ch),
				    paf->modifier);
			if (paf->duration == -1 || paf->duration == -2)
				char_nputs(AFF_PERMANENTLY, ch);
			else
				char_printf(ch, msg(AFF_FOR_D_HOURS, ch),
					    paf->duration);
		}
		send_to_char("\n\r", ch);
		paf_last = paf;
	}

	return;
}


/* new practice */
void do_practice(CHAR_DATA *ch, char *argument)
{
	char buf2[20000];
	int sn;
	CHAR_DATA *mob;
	int adept;

	if (IS_NPC(ch))
		return;

	if (argument[0] == '\0') {
		int col = 0;

		buf2[0] = '\0';
		for (sn = 0; sn < MAX_SKILL; sn++) {
			if (skill_table[sn].name == NULL)
				break;
			if (!SKILL_OK(ch, sn))
				continue;

			sprintf(strend(buf2), "%-18s %3d%%  ",
				skill_table[sn].name, ch->pcdata->learned[sn]);
			if (++col % 3 == 0)
				strcat(buf2, "\n\r");
		}

		if (col % 3 != 0)
			strcat(buf2, "\n\r");

		sprintf(strend(buf2),
			"You have %d practice sessions left.\n\r",
			ch->practice);

		page_to_char(buf2, ch);
		return;
	}

	if (!IS_AWAKE(ch)) {
		char_nputs(I_YOUR_DREAMS, ch);
		return;
	}

	if (ch->practice <= 0) {
		send_to_char("You have no practice sessions left.\n\r", ch);
		return;
	}

	if ((sn = find_spell(ch, argument)) < 0) {
		send_to_char("You can't practice that.\n\r", ch);
		return;
	}

	if (sn == gsn_vampire) {
		send_to_char("You can't practice that, only available "
			     "at questor.\n\r", ch);
		return;
	}

	for (mob = ch->in_room->people; mob != NULL; mob = mob->next_in_room) {
		if (!IS_NPC(mob) || !IS_SET(mob->act, ACT_PRACTICE))
			continue;

		if (skill_table[sn].clan != CLAN_NONE) {
			if (ch->clan == mob->clan)
				break;
			continue;
		}

		if ((mob->pIndexData->practicer == 0
		&&  (skill_table[sn].group == GROUP_NONE
		||   skill_table[sn].group == GROUP_CREATION
		||   skill_table[sn].group == GROUP_HARMFUL
		||   skill_table[sn].group == GROUP_PROTECTIVE
		||   skill_table[sn].group == GROUP_DETECTION
		||   skill_table[sn].group == GROUP_WEATHER))
		||  (mob->pIndexData->practicer & (1 << skill_table[sn].group)))
			break;
	}

	if (mob == NULL) {
		  send_to_char("You can't do that here. "
			       "USE glist, slook for more info.\n\r", ch);
		  return;
	}

	adept = IS_NPC(ch) ? 100 : class_table[ch->class].skill_adept;

	if (ch->pcdata->learned[sn] >= adept) {
		char_printf(ch, "You are already learned at %s.\n\r",
			    skill_table[sn].name);
		return;
	}

	if (!ch->pcdata->learned[sn])
		ch->pcdata->learned[sn] = 1;
	ch->practice--;
	ch->pcdata->learned[sn] += int_app[get_curr_stat(ch,STAT_INT)].learn /
				   UMAX(skill_table[sn].rating[ch->class],1);
	if (ch->pcdata->learned[sn] < adept) {
		act("You practice $T.",
		    ch, NULL, skill_table[sn].name, TO_CHAR);
		act("$n practices $T.",
		    ch, NULL, skill_table[sn].name, TO_ROOM);
	}
	else {
		ch->pcdata->learned[sn] = adept;
		act("You are now learned at $T.",
		    ch, NULL, skill_table[sn].name, TO_CHAR);
		act("$n is now learned at $T.",
		    ch, NULL, skill_table[sn].name, TO_ROOM);
	}
	return;
}


void do_camp(CHAR_DATA *ch, char *argument)
{
	AFFECT_DATA af,af2;

	if (IS_NPC(ch)
	||  ch->level < skill_table[gsn_camp].skill_level[ch->class]) {
		char_nputs(HUH, ch);
		return;
	}

	if (is_affected(ch, gsn_camp)) {
		send_to_char("You don't have enough power to handle more "
			     "camp areas.\n\r", ch);
		return;
	}

	if (number_percent() > get_skill(ch, gsn_camp)) {
		send_to_char("You failed to make your camp.\n\r", ch);
		check_improve(ch, gsn_camp, TRUE, 4);
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

	if (ch->mana < 150) {
		send_to_char("You don't have enough mana to make a camp.\n\r",
			     ch);
		return;
	}

	check_improve(ch, gsn_camp, TRUE, 4);
	ch->mana -= 150;

	WAIT_STATE(ch, skill_table[gsn_camp].beats);

	send_to_char("You succeeded to make your camp.\n\r", ch);
	act("$n succeeded to make $s camp.", ch, NULL, NULL, TO_ROOM);

	af.where	      = TO_AFFECTS;
	af.type 	      = gsn_camp;
	af.level	      = ch->level;
	af.duration	      = 12;
	af.bitvector	      = 0;
	af.modifier	      = 0;
	af.location	      = APPLY_NONE;
	affect_to_char(ch, &af);

	af2.where	      = TO_ROOM_CONST;
	af2.type	      = gsn_camp;
	af2.level	      = ch->level;
	af2.duration	      = ch->level / 20;
	af2.bitvector	      = 0;
	af2.modifier	      = 2 * ch->level;
	af2.location	      = APPLY_ROOM_HEAL;
	affect_to_room(ch->in_room, &af2);

	af2.modifier	      = ch->level;
	af2.location	      = APPLY_ROOM_MANA;
	affect_to_room(ch->in_room, &af2);
}


void do_demand(CHAR_DATA *ch, char *argument)
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
		char_nputs(YOU_CANT_DO_THAT, ch);
		return;
	}

	if (arg1[0] == '\0' || arg2[0] == '\0') {
		send_to_char("Demand what from whom?\n\r", ch);
		return;
	}

	if ((victim = get_char_room(ch, arg2)) == NULL) {
		char_nputs(THEY_ARENT_HERE, ch);
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


	if (IS_SET(obj->progtypes,OPROG_GIVE))
		(obj->pIndexData->oprogs->give_prog) (obj,ch,victim);

	if (IS_SET(victim->progtypes,MPROG_GIVE))
		(victim->pIndexData->mprogs->give_prog) (victim,ch,obj);

	send_to_char("Your power makes all around the world shivering.\n\r",ch);
}


void do_control(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int chance;

	argument = one_argument(argument, arg);

	if (IS_NPC(ch)
	||  ch->level < skill_table[gsn_control_animal].skill_level[ch->class]) {
		char_nputs(HUH, ch);
		return;
	}

	if (arg[0] == '\0') {
		send_to_char("Charm what?\n\r", ch);
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL) {
		char_nputs(THEY_ARENT_HERE, ch);
		return;
	}

	if (race_table[ORG_RACE(victim)].pc_race) {
		send_to_char("You should try this on monsters?\n\r", ch);
		return;
	}

	if (is_safe(ch,victim))
		return;

	if (count_charmed(ch))
		return;

	WAIT_STATE(ch, PULSE_VIOLENCE);

	chance = get_skill(ch,gsn_control_animal);

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
		check_improve(ch,gsn_control_animal,FALSE,2);
		do_say(victim,"I'm not about to follow you!");
		do_murder(victim, ch->name);
		return;
	}

	check_improve(ch,gsn_control_animal,TRUE,2);

	if (victim->master)
		stop_follower(victim);
	SET_BIT(victim->affected_by, AFF_CHARM);
	victim->master = victim->leader = ch;

	act("Isn't $n just so nice?", ch, NULL, victim, TO_VICT);
	if (ch != victim)
		act("$N looks at you with adoring eyes.", ch, NULL, victim,
		    TO_CHAR);
	return;
}


void do_make_arrow(CHAR_DATA *ch, char *argument)
{
	OBJ_DATA *arrow;
	AFFECT_DATA tohit,todam,saf;
	int count,color,mana,wait;
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	char *str;

	if (IS_NPC(ch))
		return;

	if (!SKILL_OK(ch, gsn_make_arrow)) {
		send_to_char("You don't know how to make arrows.\n\r", ch);
		return;
	}

	if (ch->in_room->sector_type != SECT_FIELD
	&&  ch->in_room->sector_type != SECT_FOREST
	&&  ch->in_room->sector_type != SECT_HILLS) {
		send_to_char("You couldn't find enough wood.\n\r", ch);
		return;
	}

	mana = skill_table[gsn_make_arrow].min_mana;
	wait = skill_table[gsn_make_arrow].beats;

	argument = one_argument(argument, arg);
	     if (arg[0] == '\0') color = 0;
	else if (!str_prefix(arg,"green")) color = gsn_green_arrow;
	else if (!str_prefix(arg,"red")) color = gsn_red_arrow;
	else if (!str_prefix(arg,"white")) color = gsn_white_arrow;
	else if (!str_prefix(arg,"blue")) color = gsn_blue_arrow;
	else {
		send_to_char("You don't know how to make "
			     "that kind of arrow.\n\r", ch);
		return;
	}

	if (color != 0) {
		mana += skill_table[color].min_mana;
		wait += skill_table[color].beats;
	}

	if (ch->mana < mana) {
		send_to_char("You don't have enough energy "
			     "to make that kind of arrows.\n\r", ch);
		return;
	}
	ch->mana -= mana;
	WAIT_STATE(ch,wait);

	send_to_char("You start to make arrows!\n\r",ch);
	act("$n starts to make arrows!",ch,NULL,NULL,TO_ROOM);
	for(count = 0; count < (ch->level/5); count++) {
		if (number_percent() > get_skill(ch, gsn_make_arrow)) {
			send_to_char("You failed to make the arrow, "
				     "and broke it.\n\r", ch);
			check_improve(ch, gsn_make_arrow, FALSE, 3);
			continue;
		}
		send_to_char("You successfully make an arrow.\n\r", ch);
		check_improve(ch, gsn_make_arrow, TRUE, 3);

		arrow = create_object(get_obj_index(OBJ_VNUM_RANGER_ARROW),
				      ch->level);
		arrow->level = ch->level;
		arrow->value[1] = 4 + ch->level / 10;
		arrow->value[2] = 4 + ch->level / 10;

		tohit.where		 = TO_OBJECT;
		tohit.type		 = gsn_make_arrow;
		tohit.level		 = ch->level;
		tohit.duration		 = -1;
		tohit.location		 = APPLY_HITROLL;
		tohit.modifier		 = ch->level / 10;
		tohit.bitvector 	 = 0;
		affect_to_obj(arrow, &tohit);

		todam.where		 = TO_OBJECT;
		todam.type		 = gsn_make_arrow;
		todam.level		 = ch->level;
		todam.duration		 = -1;
		todam.location		 = APPLY_DAMROLL;
		todam.modifier		 = ch->level / 10;
		todam.bitvector 	 = 0;
		affect_to_obj(arrow,&todam);

		if (color != 0) {
			saf.where	 = TO_WEAPON;
			saf.type	 = color;
			saf.level	 = ch->level;
			saf.duration	 = -1;
			saf.location	 = 0;
			saf.modifier	 = 0;

			if (color == gsn_green_arrow) {
				saf.bitvector	= WEAPON_POISON;
				str = "green";
			}
			else if (color == gsn_red_arrow) {
				saf.bitvector	= WEAPON_FLAMING;
				str = "red";
			}
			else if (color == gsn_white_arrow) {
				saf.bitvector	= WEAPON_FROST;
				str = "white";
			}
			else {
				saf.bitvector	= WEAPON_SHOCKING;
				str = "blue";
			}
		}
		else
			str = "wooden";

		sprintf(buf, arrow->name, str);
		free_string(arrow->name);
		arrow->name = str_dup(buf);

		sprintf(buf, arrow->short_descr, str);
		free_string(arrow->short_descr);
		arrow->short_descr = str_dup(buf);

		sprintf(buf, arrow->description, str);
		free_string(arrow->description);
		arrow->description = str_dup(buf);

		if (color)
			affect_to_obj(arrow, &saf);
		obj_to_char(arrow, ch);
		arrow = NULL;
	}
}


void do_make_bow(CHAR_DATA *ch, char *argument)
{
	OBJ_DATA *bow;
	AFFECT_DATA tohit,todam;
	int mana,wait;

	if (IS_NPC(ch))
		return;

	if (!SKILL_OK(ch, gsn_make_bow)) {
		send_to_char("You don't know how to make bows.\n\r", ch);
		return;
	}

	if (ch->in_room->sector_type != SECT_FIELD
	&&  ch->in_room->sector_type != SECT_FOREST
	&&  ch->in_room->sector_type != SECT_HILLS) {
		send_to_char("You couldn't find enough wood.\n\r", ch);
		return;
	}

	mana = skill_table[gsn_make_bow].min_mana;
	wait = skill_table[gsn_make_bow].beats;

	if (ch->mana < mana) {
		send_to_char("You don't have enough energy to make a bow.\n\r",
			     ch);
		return;
	}
	ch->mana -= mana;
	WAIT_STATE(ch,wait);

	if (number_percent() > get_skill(ch, gsn_make_bow)) {
		send_to_char("You failed to make the bow, and broke it.\n\r",
			     ch);
		check_improve(ch, gsn_make_bow, FALSE, 1);
		return;
	}
	send_to_char("You successfully make bow.\n\r", ch);
	check_improve(ch, gsn_make_bow, TRUE, 1);

	bow = create_object(get_obj_index(OBJ_VNUM_RANGER_BOW), ch->level);
	bow->level = ch->level;
	bow->value[1] = 4 + ch->level / 15;
	bow->value[2] = 4 + ch->level / 15;

	tohit.where		 = TO_OBJECT;
	tohit.type		 = gsn_make_arrow;
	tohit.level		 = ch->level;
	tohit.duration		 = -1;
	tohit.location		 = APPLY_HITROLL;
	tohit.modifier		 = ch->level / 10;
	tohit.bitvector 	 = 0;
	affect_to_obj(bow,&tohit);

	todam.where		 = TO_OBJECT;
	todam.type		 = gsn_make_arrow;
	todam.level		 = ch->level;
	todam.duration		 = -1;
	todam.location		 = APPLY_DAMROLL;
	todam.modifier		 = ch->level / 10;
	todam.bitvector 	 = 0;
	affect_to_obj(bow,&todam);

	obj_to_char(bow,ch);
}


void do_make(CHAR_DATA *ch, char *argument)
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

