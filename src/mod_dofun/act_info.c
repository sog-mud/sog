/*
 * $Id: act_info.c,v 1.143 1998-10-11 16:52:43 fjoe Exp $
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
#include "hometown.h"
#include "interp.h"
#include "update.h"
#include "quest.h"
#include "obj_prog.h"
#include "fight.h"

#include "resource.h"

#if defined(SUNOS) || defined(SVR4)
#	include <crypt.h>
#endif

/* command procedures needed */
DECLARE_DO_FUN(do_exits		);
DECLARE_DO_FUN(do_look		);
DECLARE_DO_FUN(do_help		);
DECLARE_DO_FUN(do_affects	);
DECLARE_DO_FUN(do_murder	);

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
void	show_affects		(CHAR_DATA *ch, BUFFER *output);

#define strend(s) (strchr(s, '\0'))

char *obj_name(OBJ_DATA *obj, CHAR_DATA *ch)
{
	static char buf[MAX_STRING_LENGTH];
	const char *name;
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
		return str_empty;

	if (IS_SET(ch->comm, COMM_LONG)) {
		if (IS_OBJ_STAT(obj, ITEM_INVIS))
			strcat(buf, MSG("({yInvis{x) ", ch->lang));
		if (IS_OBJ_STAT(obj, ITEM_DARK))
			strcat(buf, MSG("({DDark{x) ", ch->lang));
		if (IS_AFFECTED(ch, AFF_DETECT_EVIL)
		&&  IS_OBJ_STAT(obj, ITEM_EVIL))
			strcat(buf, MSG("({RRed Aura{x) ", ch->lang));
		if (IS_AFFECTED(ch, AFF_DETECT_GOOD)
		&&  IS_OBJ_STAT(obj, ITEM_BLESS))
			strcat(buf, MSG("({BBlue Aura{x) ", ch->lang));
		if (IS_AFFECTED(ch, AFF_DETECT_MAGIC)
		&&  IS_OBJ_STAT(obj, ITEM_MAGIC))
			strcat(buf, MSG("({MMagical{x) ", ch->lang));
		if (IS_OBJ_STAT(obj, ITEM_GLOW))
			strcat(buf, MSG("({WGlowing{x) ", ch->lang));
		if (IS_OBJ_STAT(obj, ITEM_HUM))
			strcat(buf, MSG("({YHumming{x) ", ch->lang));
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
				MSG(get_cond_alias(obj), ch->lang));
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
			char_puts("     ", ch);
		char_puts("Nothing.\n\r", ch);
	}

	page_to_char(buf_string(output),ch);

	/*
	 * Clean up.
	 */
	buf_free(output);
	free(prgpstrShow);
	free(prgnShow);
}

#define FLAG_SET(pos, c, exp) (FLAGS[pos] = (exp) ? (flags = TRUE, c) : '.')

void show_char_to_char_0(CHAR_DATA *victim, CHAR_DATA *ch)
{
	BUFFER *output;

	output = buf_new(0);

	if (IS_NPC(victim)) {
		if (!IS_NPC(ch) && ch->pcdata->questmob > 0
		&&  victim->hunter == ch)
			buf_add(output, MSG("{r[{RTARGET{r]{x ", ch->lang));
	}
	else {
		if (IS_SET(victim->act, PLR_WANTED))
			buf_add(output, MSG("({RWanted{x) ", ch->lang));

		if (IS_SET(victim->comm, COMM_AFK))
			buf_add(output, "{c[AFK]{x ");
	}

	if (IS_SET(ch->comm, COMM_LONG)) {
		if (IS_AFFECTED(victim, AFF_INVISIBLE))
			buf_add(output, MSG("({yInvis{x) ", ch->lang));
		if (IS_AFFECTED(victim, AFF_HIDE)) 
			buf_add(output, MSG("({DHidden{x) ", ch->lang));
		if (IS_AFFECTED(victim, AFF_CHARM)) 
			buf_add(output, MSG("({mCharmed{x) ", ch->lang));
		if (IS_AFFECTED(victim, AFF_PASS_DOOR)) 
			buf_add(output, MSG("({cTranslucent{x) ", ch->lang));
		if (IS_AFFECTED(victim, AFF_FAERIE_FIRE)) 
			buf_add(output, MSG("({MPink Aura{x) ", ch->lang));
		if (IS_NPC(victim) && IS_SET(victim->act,ACT_UNDEAD)
		&&  IS_AFFECTED(ch, AFF_DETECT_UNDEAD))
			buf_add(output, MSG("({DUndead{x) ", ch->lang));
		if (RIDDEN(victim))
			buf_add(output, MSG("({GRidden{x) ", ch->lang));
		if (IS_AFFECTED(victim,AFF_IMP_INVIS))
			buf_add(output, MSG("({bImproved{x) ", ch->lang));
		if (IS_EVIL(victim) && IS_AFFECTED(ch, AFF_DETECT_EVIL))
			buf_add(output, MSG("({RRed Aura{x) ", ch->lang));
		if (IS_GOOD(victim) && IS_AFFECTED(ch, AFF_DETECT_GOOD))
			buf_add(output, MSG("({YGolden Aura{x) ", ch->lang));
		if (IS_AFFECTED(victim, AFF_SANCTUARY))
			buf_add(output, MSG("({WWhite Aura{x) ", ch->lang));
		if (IS_AFFECTED(victim, AFF_FADE)) 
			buf_add(output, MSG("({yFade{x) ", ch->lang));
		if (IS_AFFECTED(victim, AFF_CAMOUFLAGE)) 
			buf_add(output, MSG("({gCamf{x) ", ch->lang));
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
		FLAG_SET(26, 'I', IS_AFFECTED(victim, AFF_IMP_INVIS));
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
	if (victim->incog_level >= LEVEL_HERO)
		buf_add(output, "[{DIncog{x] ");

	if (IS_NPC(victim) && victim->position == victim->start_pos) {
		const char *p = mlstr_cval(victim->long_descr, ch);
		if (IS_NULLSTR(p)) {	/* for the hell of "It" (#2006) :) */
			buf_free(output);
			return;
		}
		buf_printf(output, "{g%s{x", p);
	}
	else {
		char *msg;

		if (IS_IMMORTAL(victim))
			buf_add(output, "{W");
		buf_add(output, capitalize(PERS(victim, ch)));
		if (IS_IMMORTAL(victim))
			buf_add(output, "{x");

		if (!IS_NPC(victim) && !IS_SET(ch->comm, COMM_BRIEF)
		&&  victim->position == POS_STANDING)
			buf_printf(output, "%s{x", victim->pcdata->title);
	
		switch (victim->position) {
		case POS_DEAD:
			buf_add(output, vmsg(MSG_IS_DEAD, ch, victim));
			break;
	
		case POS_MORTAL:
			buf_add(output, MSG(" is mortally wounded.", victim->lang));
			break;
	
		case POS_INCAP:
			buf_add(output, MSG(" is incapacitated.", victim->lang));
			break;
	
		case POS_STUNNED:
			buf_add(output, vmsg(MSG_IS_LYING_HERE_STUNNED, ch, victim));
			break;
	
		case POS_SLEEPING:
			if (victim->on == NULL) {
				buf_add(output, MSG(" is sleeping here.", ch->lang));
				break;
			}
	
			if (IS_SET(victim->on->value[2], SLEEP_AT))
				msg = " is sleeping at %s.";
			else if (IS_SET(victim->on->value[2], SLEEP_ON))
				msg = " is sleeping on %s.";
			else
				msg = " is sleeping in %s.";
	
			buf_printf(output, MSG(msg, ch->lang),
				   mlstr_cval(victim->on->short_descr, ch));
			break;
	
		case POS_RESTING:
			if (victim->on == NULL) {
				buf_add(output, MSG(" is resting here.", ch->lang));
				break;
			}
	
			if (IS_SET(victim->on->value[2], REST_AT))
				msg = " is resting at %s.";
			else if (IS_SET(victim->on->value[2], REST_ON))
				msg = " is resting on %s.";
			else
				msg = " is resting in %s.";
			buf_printf(output, MSG(msg, ch->lang),
				mlstr_cval(victim->on->short_descr, ch));
			break;
	
		case POS_SITTING:
			if (victim->on == NULL) {
				buf_add(output, MSG(" is sitting here.", ch->lang));
				break;
			}
	
			if (IS_SET(victim->on->value[2], SIT_AT))
				msg = " is sitting at %s.";
			else if (IS_SET(victim->on->value[2], SIT_ON))
				msg = " is sitting on %s.";
			else
				msg = " is sitting in %s.";
			buf_printf(output, MSG(msg, ch->lang),
				mlstr_cval(victim->on->short_descr, ch));
			break;
	
		case POS_STANDING:
			if (victim->on == NULL) {
				if (MOUNTED(victim))
					buf_printf(output,
						MSG(" is here, riding %s.", ch->lang),
						PERS(MOUNTED(victim),ch));
				else
					buf_add(output,
						MSG(" is here. ", ch->lang));
				break;
			}
	
			if (IS_SET(victim->on->value[2],STAND_AT))
				msg = " is standing at %s.";
			else if (IS_SET(victim->on->value[2],STAND_ON))
				msg = " is standing on %s.";
			else
				msg = " is standing here.";
			buf_printf(output, MSG(msg, ch->lang),
				mlstr_cval(victim->on->short_descr, ch));
			break;
	
		case POS_FIGHTING:
			buf_add(output, MSG(" is here, fighting with ", ch->lang));
			if (victim->fighting == NULL)
				buf_add(output, "thin air??");
			else if (victim->fighting == ch)
				buf_add(output, MSG("YOU!", ch->lang));
			else if (victim->in_room == victim->fighting->in_room)
				buf_printf(output, "%s.",
					   PERS(victim->fighting, ch));
			else
				buf_add(output, "somone who left??");
			break;
		}
	
		buf_add(output, "{x\n\r");
	}

	send_to_char(buf_string(output), ch);
	buf_free(output);
}

char* wear_loc_names[] =
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
	"<stuck in>          $t",
};

void show_obj_to_char(CHAR_DATA *ch, OBJ_DATA *obj, sflag_t wear_loc)
{
	bool can_see = can_see_obj(ch, obj);
	act(wear_loc_names[wear_loc], ch,
	    can_see ? format_obj_to_char(obj, ch, TRUE) : "something",
	    NULL, TO_CHAR | (can_see ? 0 : TRANS_TEXT));
}

void show_char_to_char_1(CHAR_DATA *victim, CHAR_DATA *ch)
{
	OBJ_DATA *obj;
	int i;
	int percent;
	bool found;
	char *msg;
	const char *desc;

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
		char_printf(ch, "%s is riding %s.\n\r",
			    PERS(victim,ch), PERS(MOUNTED(victim),ch));
	if (RIDDEN(victim))
		act("$N is being ridden by $t.",
		    ch, PERS(RIDDEN(victim), ch), victim, TO_CHAR);

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
	if (percent < 90 && ch->class == CLASS_VAMPIRE && ch->level > 10)
		gain_condition(ch, COND_BLOODLUST, -1);

	if (!IS_IMMORTAL(victim)) {
		char_printf(ch, "(%s) ", race_table[RACE(victim)].name);
		if (!IS_NPC(victim)) 
			char_printf(ch, "(%s) ", class_name(victim->class));
	}

	char_printf(ch, "%s%s%s %s\n\r",
		    IS_IMMORTAL(victim) ? "{W" : str_empty,
		    PERS(victim, ch),
		    IS_IMMORTAL(victim) ? "{x" : str_empty,
		    MSG(msg, ch->lang));

	found = FALSE;
	for (i = 0; show_order[i] != -1; i++)
		if ((obj = get_eq_char(victim, show_order[i]))
		&&  can_see_obj(ch, obj)) {
			if (!found) {
				char_puts("\n\r", ch);
				act("$N is using:", ch, NULL, victim, TO_CHAR);
				found = TRUE;
			}

			show_obj_to_char(ch, obj, show_order[i]);
		}

	for (obj = victim->carrying; obj; obj = obj->next_content)
		if (obj->wear_loc == WEAR_STUCK_IN
		&&  can_see_obj(ch, obj)) {
			if (!found) {
				char_puts("\n\r", ch);
				act("$N is using:", ch, NULL, victim, TO_CHAR);
				found = TRUE;
			}

			show_obj_to_char(ch, obj, WEAR_STUCK_IN);
		}

	if (victim != ch
	&&  !IS_NPC(ch)
	&&  number_percent() < get_skill(ch, gsn_peek)) {
		char_puts("\n\rYou peek at the inventory:\n\r", ch);
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
			char_puts("You see {rglowing red eyes{x watching YOU!\n\r",
				  ch);
			if (!IS_IMMORTAL(rch))
				life_count++;
		}
		else if (!IS_IMMORTAL(rch))
			life_count++;
	}

	if (life_count && IS_AFFECTED(ch, AFF_DETECT_LIFE))
		char_printf(ch, "You feel %d more life %s in the room.\n\r",
			    life_count, (life_count == 1) ? "form" : "forms");
}

void do_clear(CHAR_DATA *ch, const char *argument)
{
	if (!IS_NPC(ch))
		char_puts("\033[0;0H\033[2J", ch);
}

/* changes your scroll */
DO_FUN(do_scroll)
{
	char arg[MAX_INPUT_LENGTH];
	int lines;

	one_argument(argument, arg);

	if (arg[0] == '\0') {
		char_printf(ch, "You currently display %d lines per "
				"page.\n\r", ch->lines + 2);
		return;
	}

	if (!is_number(arg)) {
		char_puts("You must provide a number.\n\r",ch);
		return;
	}

	lines = atoi(arg);
	if (lines < SCROLL_MIN || lines > SCROLL_MAX) {
		char_printf(ch, "Valid scroll range is %d..%d.\n\r",
			    SCROLL_MIN, SCROLL_MAX);
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
			char_puts("\n\r",ch);
	}

	if (col % 6)
		char_puts("\n\r",ch);
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
		char_printf(ch, "%-16s %s\n\r", swname, sw ? "ON" : "OFF");
void do_autolist(CHAR_DATA *ch, const char *argument)
{
	/* lists most player flags */
	if (IS_NPC(ch))
		return;

	char_puts("action         status\n\r",ch);
	char_puts("---------------------\n\r",ch);
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
		char_puts("You can only be summoned players within "
			     "your PK range.\n\r",ch);
	else
		char_puts("You can be summoned by anyone.\n\r",ch);

	if (IS_SET(ch->act,PLR_NOFOLLOW))
		char_puts("You do not welcome followers.\n\r",ch);
	else
		char_puts("You accept followers.\n\r",ch);
}

void do_autoassist(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch))
	  return;

	if (IS_SET(ch->act,PLR_AUTOASSIST)) {
		char_puts("Autoassist removed.\n\r",ch);
		REMOVE_BIT(ch->act,PLR_AUTOASSIST);
	}
	else {
		char_puts("You will now assist when needed.\n\r",ch);
		SET_BIT(ch->act,PLR_AUTOASSIST);
	}
}

void do_autoexit(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch))
		return;

	if (IS_SET(ch->act,PLR_AUTOEXIT)) {
		char_puts("Exits will no longer be displayed.\n\r",ch);
		REMOVE_BIT(ch->act, PLR_AUTOEXIT);
	}
	else {
		char_puts("Exits will now be displayed.\n\r",ch);
		SET_BIT(ch->act, PLR_AUTOEXIT);
	}
}

void do_autogold(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch))
		return;

	if (IS_SET(ch->act,PLR_AUTOGOLD)) {
		char_puts("Autogold removed.\n\r",ch);
		REMOVE_BIT(ch->act,PLR_AUTOGOLD);
	}
	else {
		char_puts("Automatic gold looting set.\n\r",ch);
		SET_BIT(ch->act,PLR_AUTOGOLD);
	}
}

void do_autoloot(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch))
		return;

	if (IS_SET(ch->act,PLR_AUTOLOOT)) {
		char_puts("Autolooting removed.\n\r",ch);
		REMOVE_BIT(ch->act,PLR_AUTOLOOT);
	}
	else {
		char_puts("Automatic corpse looting set.\n\r",ch);
		SET_BIT(ch->act,PLR_AUTOLOOT);
	}
}

void do_autosac(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch))
		return;

	if (IS_SET(ch->act,PLR_AUTOSAC)) {
		char_puts("Autosacrificing removed.\n\r",ch);
		REMOVE_BIT(ch->act,PLR_AUTOSAC);
	}
	else {
		char_puts("Automatic corpse sacrificing set.\n\r",ch);
		SET_BIT(ch->act,PLR_AUTOSAC);
	}
}

void do_autosplit(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch))
		return;

	if (IS_SET(ch->act,PLR_AUTOSPLIT)) {
		char_puts("Autosplitting removed.\n\r",ch);
		REMOVE_BIT(ch->act,PLR_AUTOSPLIT);
	}
	else {
		char_puts("Automatic gold splitting set.\n\r",ch);
		SET_BIT(ch->act,PLR_AUTOSPLIT);
	}
}

void do_color(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch))
		return;

	if (IS_SET(ch->act,PLR_COLOR)) {
		REMOVE_BIT(ch->act,PLR_COLOR);
		char_puts("Color is now OFF, *sigh*\n\r", ch);
	}
	else {
		SET_BIT(ch->act,PLR_COLOR);
		char_puts("{BC{Ro{Yl{Co{Gr{x is now {RON{x, Way Cool!\n\r",
			     ch);
	}
}

void do_brief(CHAR_DATA *ch, const char *argument)
{
	if (IS_SET(ch->comm,COMM_BRIEF)) {
		char_puts("Full descriptions activated.\n\r",ch);
		REMOVE_BIT(ch->comm,COMM_BRIEF);
	}
	else {
		char_puts("Short descriptions activated.\n\r",ch);
		SET_BIT(ch->comm,COMM_BRIEF);
	}
}

void do_compact(CHAR_DATA *ch, const char *argument)
{
	if (IS_SET(ch->comm,COMM_COMPACT)) {
		char_puts("Compact mode removed.\n\r",ch);
		REMOVE_BIT(ch->comm,COMM_COMPACT);
	}
	else {
		char_puts("Compact mode set.\n\r",ch);
		SET_BIT(ch->comm,COMM_COMPACT);
	}
}

void do_long(CHAR_DATA *ch, const char *argument)
{
	if (IS_SET(ch->comm,COMM_LONG)) {
		char_puts("Long flags mode removed.\n\r",ch);
		REMOVE_BIT(ch->comm,COMM_LONG);
	}
	else {
		char_puts("Long flags mode set.\n\r",ch);
		SET_BIT(ch->comm,COMM_LONG);
	}
}

void do_show(CHAR_DATA *ch, const char *argument)
{
	if (IS_SET(ch->comm,COMM_SHOW_AFFECTS)) {
		char_puts("Affects will no longer be shown in score.\n\r",
			     ch);
		REMOVE_BIT(ch->comm,COMM_SHOW_AFFECTS);
	}
	else {
		char_puts("Affects will now be shown in score.\n\r", ch);
		SET_BIT(ch->comm,COMM_SHOW_AFFECTS);
	}
}

void do_prompt(CHAR_DATA *ch, const char *argument)
{
	const char *prompt;

	if (argument[0] == '\0') {
		if (IS_SET(ch->comm,COMM_PROMPT)) {
			char_puts("You will no longer see prompts.\n\r",ch);
			REMOVE_BIT(ch->comm,COMM_PROMPT);
		}
		else {
			char_puts("You will now see prompts.\n\r",ch);
			SET_BIT(ch->comm,COMM_PROMPT);
		}
		return;
	}

	if (!strcmp(argument, "all"))
		prompt = str_dup(DEFAULT_PROMPT);
	else
		prompt = str_printf("%s ", argument);

	free_string(ch->prompt);
	ch->prompt = prompt;
	char_printf(ch, "Prompt set to '%s'\n\r", ch->prompt);
}

void do_combine(CHAR_DATA *ch, const char *argument)
{
	if (IS_SET(ch->comm,COMM_COMBINE)) {
		char_puts("Long inventory selected.\n\r",ch);
		REMOVE_BIT(ch->comm,COMM_COMBINE);
	}
	else {
		char_puts("Combined inventory selected.\n\r",ch);
		SET_BIT(ch->comm,COMM_COMBINE);
	}
}

void do_noloot(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch))
		return;

	if (IS_SET(ch->act,PLR_CANLOOT)) {
		char_puts("Your corpse is now safe from thieves.\n\r",ch);
		REMOVE_BIT(ch->act,PLR_CANLOOT);
	}
	else {
		char_puts("Your corpse may now be looted.\n\r",ch);
		SET_BIT(ch->act,PLR_CANLOOT);
	}
}

void do_nofollow(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch))
		return;

	if (IS_AFFECTED(ch, AFF_CHARM))  {
		char_puts("You don't want to leave "
			     "your beloved master.\n\r", ch);
		return;
	}

	if (IS_SET(ch->act,PLR_NOFOLLOW)) {
		char_puts("You now accept followers.\n\r", ch);
		REMOVE_BIT(ch->act,PLR_NOFOLLOW);
	}
	else {
		char_puts("You no longer accept followers.\n\r", ch);
		SET_BIT(ch->act,PLR_NOFOLLOW);
		die_follower(ch);
	}
}

void do_nosummon(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch)) {
		if (IS_SET(ch->imm_flags,IMM_SUMMON)) {
			char_puts("You are no longer immune "
				     "to summoning.\n\r", ch);
			REMOVE_BIT(ch->imm_flags,IMM_SUMMON);
		}
		else {
			char_puts("You are now immune to summoning.\n\r",
				     ch);
			SET_BIT(ch->imm_flags,IMM_SUMMON);
		}
	}
	else {
		if (IS_SET(ch->act,PLR_NOSUMMON)) {
			char_puts("You may now be summoned by anyone.\n\r",
				     ch);
			REMOVE_BIT(ch->act,PLR_NOSUMMON);
		}
		else {
			char_puts("You may only be summoned by players "
				     "within your PK range.\n\r", ch);
			SET_BIT(ch->act,PLR_NOSUMMON);
		}
	}
}

void do_look_in(CHAR_DATA* ch, const char *argument)
{
	OBJ_DATA *obj;

	if ((obj = get_obj_here(ch, argument)) == NULL) {
		char_puts("You don't see that here.\n\r", ch);
		return;
	}

	switch (obj->pIndexData->item_type) {
	default:
		char_puts("That is not a container.\n\r", ch);
		break;

	case ITEM_DRINK_CON:
		if (obj->value[1] <= 0) {
			char_puts("It is empty.\n\r", ch);
			break;
		}

		char_printf(ch, "It's %sfilled with a %s liquid.\n\r",
			    obj->value[1] < obj->value[0] / 4 ?
			    MSG("less than half-", ch->lang) :
			    obj->value[1] < 3 * obj->value[0] / 4 ?
			    MSG("about half-", ch->lang) :
			    MSG("more than half-", ch->lang),
			    liq_table[obj->value[2]].liq_color);
		break;

	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
	case ITEM_CORPSE_PC:
		if (IS_SET(obj->value[1], CONT_CLOSED)) {
			char_puts("It is closed.\n\r", ch);
			break;
		}

		act_printf(ch, obj, NULL, TO_CHAR, POS_DEAD, "$p holds:");
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
		char_puts("You can't see anything but stars!\n\r", ch);
		return;
	}

	if (ch->position == POS_SLEEPING) {
		char_puts("You can't see anything, you're sleeping!\n\r", ch);
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
		const char *name;
		const char *engname;

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

		char_puts("\n\r", ch);

		if (arg1[0] == '\0'
		||  (!IS_NPC(ch) && !IS_SET(ch->comm, COMM_BRIEF)))
			char_printf(ch, "  %s",
				    mlstr_cval(ch->in_room->description, ch));

		if (!IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOEXIT)) {
			char_puts("\n\r", ch);
			do_exits(ch, "auto");
		}
	    } else 
		char_puts("It is pitch black...\n\r", ch);

		show_list_to_char(ch->in_room->contents, ch, FALSE, FALSE);
		show_char_to_char(ch->in_room->people, ch);
		return;
	}

	if (!str_cmp(arg1, "i")
	||  !str_cmp(arg1, "in")
	||  !str_cmp(arg1,"on")) {
		/* 'look in' */
		if (arg2[0] == '\0') {
			char_puts("Look in what?\n\r", ch);
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
					char_puts("You see nothing special about it.\n\r", ch);
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
				char_puts("\n\r",ch);
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
			char_printf(ch, "You only see one %s here.\n\r", arg3);
		else
			char_printf(ch, "You only see %d of those here.\n\r", count);
		return;
	}

		   if (!str_cmp(arg1, "n") || !str_cmp(arg1, "north")) door = 0;
	else if (!str_cmp(arg1, "e") || !str_cmp(arg1, "east")) door = 1;
	else if (!str_cmp(arg1, "s") || !str_cmp(arg1, "south")) door = 2;
	else if (!str_cmp(arg1, "w") || !str_cmp(arg1, "west")) door = 3;
	else if (!str_cmp(arg1, "u") || !str_cmp(arg1, "up" )) door = 4;
	else if (!str_cmp(arg1, "d") || !str_cmp(arg1, "down")) door = 5;
	else {
		char_puts("You don't see that here.\n\r", ch);
		return;
	}

	/* 'look direction' */
	if ((pexit = ch->in_room->exit[door]) == NULL) {
		char_puts("Nothing special there.\n\r", ch);
		return;
	}

	if (!IS_NULLSTR(mlstr_mval(pexit->description)))
		char_mlputs(pexit->description, ch);
	else
		char_puts("Nothing special there.\n\r", ch);

	if (pexit->keyword    != NULL
	&&  pexit->keyword[0] != '\0'
	&&  pexit->keyword[0] != ' ') {
		if (IS_SET(pexit->exit_info, EX_CLOSED)) {
			act_printf(ch, NULL, pexit->keyword, TO_CHAR,
				   POS_DEAD, "The $d is closed.");
		}
		else if (IS_SET(pexit->exit_info, EX_ISDOOR))
			act_printf(ch, NULL, pexit->keyword, TO_CHAR,
				   POS_DEAD, "The $d is open.");
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
		char_puts("You can't see anything but stars!\n\r", ch);
		return;
	}

	if (ch->position == POS_SLEEPING) {
		char_puts("You can't see anything, you're sleeping!\n\r", ch);
		return;
	}

	if (!check_blind(ch))
		return;

	if (arg[0] == '\0') {
		char_puts("Examine what?\n\r", ch);
		return;
	}

	do_look(ch, arg);

	if ((obj = get_obj_here(ch, arg)) != NULL) {
		switch (obj->pIndexData->item_type) {
		case ITEM_MONEY:
			if (obj->value[0] == 0) {
				if (obj->value[1] == 0)
					char_puts("Odd...there's no coins in the pile.\n\r", ch);
				else if (obj->value[1] == 1)
					char_puts("Wow. One gold coin.\n\r", ch);
				else
					char_printf(ch, "There are %d gold coins in the pile.\n\r",
						     obj->value[1]);
			}
			else if (obj->value[1] == 0) {
				if (obj->value[0] == 1)
					char_puts("Wow. One silver coin.\n\r", ch);
				else
					char_printf(ch, "There are %d silver coins in the pile.\n\r",
						     obj->value[0]);
			}
			else
				char_printf(ch, "There are %d gold and %d silver coins in the pile.\n\r", 
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

	if (fAuto)
		char_puts("{C[Exits:", ch);
	else if (IS_IMMORTAL(ch))
		char_printf(ch, "Obvious exits from room %d:\n\r",
			    ch->in_room->vnum);
	else
		char_puts("Obvious exits:\n\r", ch);

	found = FALSE;
	for (door = 0; door < MAX_DIR; door++) {
		if ((pexit = ch->in_room->exit[door]) != NULL
		&&  pexit->u1.to_room != NULL
		&&  can_see_room(ch, pexit->u1.to_room)
		&&  check_blind_raw(ch)) { 
			bool show_closed = FALSE;

			if (IS_SET(pexit->exit_info, EX_CLOSED)) {
				int chance;

				if (IS_IMMORTAL(ch))
					show_closed = TRUE;
				else if (!fAuto &&
					 (chance = get_skill(ch, gsn_perception))) {
					 if (number_percent() < chance) {
						check_improve(ch, gsn_perception,
							      TRUE, 5);
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
					    room_dark(pexit->u1.to_room) ?
					    MSG("Too dark to tell", ch->lang) :
					    mlstr_cval(pexit->u1.to_room->name,
							ch));
				if (IS_IMMORTAL(ch))
					char_printf(ch, " (room %d)",
						    pexit->u1.to_room->vnum);
				char_puts("\n\r", ch);
			}
		}
	}

	if (!found)
		char_puts(fAuto ? " none" : "None.\n\r", ch);

	if (fAuto)
		char_puts("]{x\n\r", ch);
}

void do_worth(CHAR_DATA *ch, const char *argument)
{
	char_printf(ch, "You have %d gold, %d silver", ch->gold, ch->silver);
	if (!IS_NPC(ch) && ch->level < LEVEL_HERO)
		char_printf(ch, ", and %d experience (%d exp to level)",
			    ch->exp, exp_to_level(ch));
	char_puts(".\n\r", ch);

	if (!IS_NPC(ch))
		char_printf(ch, msg(MSG_HAVE_KILLED, ch),
			    ch->pcdata->has_killed,
			    IS_GOOD(ch) ? MSG("non-goods", ch->lang) :
			    IS_EVIL(ch) ? MSG("non-evils", ch->lang) : 
					  MSG("non-neutrals", ch->lang),
			    ch->pcdata->anti_killed,
			    IS_GOOD(ch) ? MSG("goods", ch->lang) :
			    IS_EVIL(ch) ? MSG("evils", ch->lang) : 
					  MSG("neutrals", ch->lang));
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
		act_printf(ch, NULL, NULL, TO_CHAR, POS_RESTING,
			   "It's %s.",
			   (time_info.hour>=5 && time_info.hour<9) ?
						MSG("dawn", ch->lang) :
			   (time_info.hour>=9 && time_info.hour<12) ?
						MSG("morning", ch->lang) :
			   (time_info.hour>=12 && time_info.hour<18) ?
						MSG("mid-day", ch->lang) :
			   (time_info.hour>=18 && time_info.hour<21) ?
						MSG("evening", ch->lang) :
			   MSG("night", ch->lang));

	if (!IS_IMMORTAL(ch))
		return;

	char_printf(ch, "\n\rMUDDY started up at %s"
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
		char_puts("You can't see the weather indoors.\n\r", ch);
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
		buf_printf(output, "%3d ", wch->level);
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
			buf_printf(output, " %s", cl->who_name);
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
	if (wch->incog_level >= LEVEL_HERO)
		buf_add(output, "[{DIncog{x] ");

	if (in_PK(ch, wch) && !IS_IMMORTAL(ch))
		buf_add(output, "{r[{RPK{r]{x ");

	if (IS_SET(wch->act, PLR_WANTED))
		buf_add(output, "{R(WANTED){x ");

	if (IS_IMMORTAL(wch))
		buf_printf(output, "{W%s{x", wch->name);
	else
		buf_add(output, wch->name);

	if (IS_NPC(wch))
		buf_add(output, " Believer of Chronos");
	else
		buf_add(output, wch->pcdata->title);

	buf_add(output, "\n\r{x");
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
				char_puts("You haven't got a tattoo yet!\n\r",ch);
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
				char_puts("This function of who is for "
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
					char_puts("That's not a "
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

		if (is_affected(d->character, gsn_vampire) && !IS_IMMORTAL(ch)
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
	buf_printf(output, MSG("\n\rPlayers found: %d. Most so far today: %d.\n\r", ch->lang), nMatch, max_on);
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
		||  (is_affected(d->character, gsn_vampire) &&
		     !IS_IMMORTAL(ch) && (ch != d->character)))
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
		char_puts("No one of that name is playing.\n\r", ch);
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

	char_printf(ch, "There are %d characters on, ", count);
	if (max_on == count)
		char_puts("the most so far today", ch);
	else
		char_printf(ch, "the most on today was %d", max_on);
	char_puts(".\n\r", ch);
}

void do_inventory(CHAR_DATA *ch, const char *argument)
{
	char_puts("You are carrying:\n\r", ch);
	show_list_to_char(ch->carrying, ch, TRUE, TRUE);
}

void do_equipment(CHAR_DATA *ch, const char *argument)
{
	OBJ_DATA *obj;
	int i;
	bool found;

	char_puts("You are using:\n\r", ch);
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
		char_puts("Nothing.\n\r", ch);
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
		char_puts("Compare what to what?\n\r", ch);
		return;
	}

	if ((obj1 = get_obj_carry(ch, arg1)) == NULL) {
		char_puts("You do not have that item.\n\r", ch);
		return;
	}

	if (arg2[0] == '\0') {
		for (obj2 = ch->carrying;
		     obj2 != NULL; obj2 = obj2->next_content)
			if (obj2->wear_loc != WEAR_NONE
			&&  can_see_obj(ch,obj2)
			&&  obj1->pIndexData->item_type == obj2->pIndexData->item_type
			&&  (obj1->wear_flags & obj2->wear_flags & ~ITEM_TAKE))
				break;

		if (obj2 == NULL) {
			char_puts("You aren't wearing anything comparable.\n\r", ch);
			return;
		}
	}
	else if ((obj2 = get_obj_carry(ch,arg2)) == NULL) {
		char_puts("You do not have that item.\n\r", ch);
		return;
	}

	cmsg		= NULL;
	value1	= 0;
	value2	= 0;

	if (obj1 == obj2)
		cmsg = "You compare $p to itself.  It looks about the same.";
	else if (obj1->pIndexData->item_type != obj2->pIndexData->item_type)
		cmsg = "You can't compare $p and $P.";
	else {
		switch (obj1->pIndexData->item_type) {
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

	if (cmsg == NULL)
		if (value1 == value2)
			cmsg = "$p and $P look about the same.";
		else if (value1  > value2)
			cmsg = "$p looks better than $P.";
		else
			cmsg = "$p looks worse than $P.";

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
		char_puts("It's too dark to see.\n\r", ch);
		return;
	}

	if (!str_cmp(arg,"pk"))
		fPKonly = TRUE;

	if (arg[0] == '\0' || fPKonly) {
		char_puts("Players near you:\n\r", ch);
		found = FALSE;
		for (d = descriptor_list; d; d = d->next) {
			if (d->connected == CON_PLAYING
			&&  (victim = d->character) != NULL
			&&  !IS_NPC(victim)
			&&  (!fPKonly || in_PK(ch, victim))
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
			char_puts("None.\n\r", ch);
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
			act_printf(ch, NULL, arg, TO_CHAR, POS_DEAD,
				   "You didn't find any $T.");
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
		char_puts("Consider killing whom?\n\r", ch);
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL) {
		char_puts("They aren't here.\n\r", ch);
		return;
	}

	if (victim == ch) {
		char_puts("Suicide is against your way.\n\r", ch);
		return;
	}

	if (!in_PK(ch, victim)) {
		char_puts("Don't even think about it.\n\r", ch);
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

	if (IS_SET(ch->act, PLR_NOTITLE)) {
		char_puts("You can't change your title.\n\r", ch);
		return;
	}

	if (argument[0] == '\0') {
		char_puts("Change your title to what?\n\r", ch);
		return;
	}

	if (strstr(argument, "{/")) {
		char_nputs(MSG_ILLEGAL_CHARACTER_TITLE, ch);
		return;
	}
		
	set_title(ch, argument);
	char_puts("Ok.\n\r", ch);
}

void do_description(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_STRING_LENGTH];

	one_argument(argument, arg);

	if (str_cmp(arg, "edit") == 0) {
		string_append(ch, mlstr_convert(&ch->description, -1));
		return;
	}

	char_printf(ch, "Your description is:\n\r"
			 "%s\n\r"
			 "Use 'desc edit' to edit your description.\n\r",
		    mlstr_mval(ch->description));
}

void do_report(CHAR_DATA *ch, const char *argument)
{
	doprintf(do_say, ch, "I have %d/%d hp %d/%d mana %d/%d mv.",
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
		char_puts("Your courage exceeds your wisdom.\n\r", ch);
		return;
	}

	if (wimpy > ch->max_hit/2) {
		char_puts("Such cowardice ill becomes you.\n\r", ch);
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
		char_puts("Syntax: password <old> <new>.\n\r", ch);
		return;
	}

	if (strcmp(crypt(arg1, ch->pcdata->pwd), ch->pcdata->pwd)) {
		WAIT_STATE(ch, 40);
		char_puts("Wrong password.  Wait 10 seconds.\n\r", ch);
		return;
	}

	if (strlen(arg2) < 5) {
		char_puts("New password must be at least "
			     "five characters long.\n\r", ch);
		return;
	}

	/*
	 * No tilde allowed because of player file format.
	 */
	pwdnew = crypt(arg2, ch->name);
	if (strchr(pwdnew, '~') != NULL) {
		char_puts("New password not acceptable, "
			     "try again.\n\r", ch);
		return;
	}

	free_string(ch->pcdata->pwd);
	ch->pcdata->pwd = str_dup(pwdnew);
	save_char_obj(ch, FALSE);
	char_puts("Ok.\n\r", ch);
	return;
}

void scan_list(ROOM_INDEX_DATA *scan_room, CHAR_DATA *ch, 
		int depth, int door)
{
	CHAR_DATA *rch;

	if (scan_room == NULL) 
		return;
	for (rch = scan_room->people; rch != NULL; rch = rch->next_in_room) {
		if (rch == ch || 
		    (!IS_NPC(rch) && rch->invis_level > get_trust(ch)))
			continue;
		if (can_see(ch, rch)) 
			char_printf(ch, "	%s.\n\r", PERS(rch, ch));
	}
}

void do_scan2(CHAR_DATA *ch, const char *argument)
{
	extern char *const dir_name[];
	EXIT_DATA *pExit;
	int door;

	act("$n looks all around.", ch, NULL, NULL, TO_ROOM);
	if (!check_blind(ch))
		return;

	char_puts("Looking around you see:\n\r", ch);

	char_puts("{Chere{x:\n\r", ch);
	scan_list(ch->in_room, ch, 0, -1);
	for (door = 0; door < 6; door++) {
		if ((pExit = ch->in_room->exit[door]) == NULL
		|| !pExit->u1.to_room
		|| !can_see_room(ch,pExit->u1.to_room))
			continue;
		char_printf(ch, "{C%s{x:\n\r", dir_name[door]);
		if (IS_SET(pExit->exit_info, EX_CLOSED)) {
			char_puts("	You see closed door.\n\r", ch);
			continue;
		}
		scan_list(pExit->u1.to_room, ch, 1, door);
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

	one_argument(argument,dir);

	if (dir[0] == '\0') {
		do_scan2(ch, str_empty);
		return;
	}

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
	default:
		char_puts("Wrong direction.\n\r", ch);
		return;
	}

	act("$n scans $t.", ch, dir_name[door], NULL, TO_ROOM | TRANS_TEXT);
	if (!check_blind(ch))
		return;

	act_puts("You scan $t.", ch, dir_name[door], NULL, TO_CHAR | TRANS_TEXT,
		 POS_DEAD);

	range = 1 + ch->level/10;

	in_room = ch->in_room;
	for (i = 1; i <= range; i++) {
		exit = in_room->exit[door];
		if (!exit)
			return;
		to_room = exit->u1.to_room;
		if (!to_room)
			return;

		if (IS_SET(exit->exit_info,EX_CLOSED)
		&&  can_see_room(ch,exit->u1.to_room)) {
			char_puts("	You see closed door.\n\r", ch);
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
			char_puts("\n\r", ch);
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
		char_puts("Wait for a while to request again.\n\r", ch);
		return;
	}

	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (IS_NPC(ch))
		return;

	if (arg1[0] == '\0' || arg2[0] == '\0') {
		char_puts("Request what from whom?\n\r", ch);
		return;
	}

	if ((victim = get_char_room(ch, arg2)) == NULL) {
		char_puts("They aren't here.\n\r", ch);
		return;
	}

	if (!IS_NPC(victim)) {
		char_puts("Why don't you just ask the player?\n\r", ch);
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
		char_puts("Your hands are full.\n\r", ch);
		return;
	}

	if (ch->carry_weight + get_obj_weight(obj) > can_carry_w(ch)) {
		char_puts("You can't carry that much weight.\n\r", ch);
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
	char_puts("and for the goodness you have seen in the world.\n\r",ch);

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
		char_puts("You can't change your hometown!\n\r", ch);
		return;
	}

	if (ORG_RACE(ch) == 11 || ORG_RACE(ch) == 12
	||  ORG_RACE(ch) == 13 || ORG_RACE(ch) == 14) {
		char_puts("Your hometown is permenantly Titan Valley!\n\r",
			     ch);
		return;
	}

	if (ch->class == CLASS_VAMPIRE || ch->class == CLASS_NECROMANCER) {
		char_puts("Your hometown is permenantly Old Midgaard!\n\r",
			     ch);
		return;
	}

	if (!IS_SET(ch->in_room->room_flags, ROOM_REGISTRY)) {
		char_puts("You have to be in the Registry "
			     "to change your hometown.\n\r", ch);
		return;
	}

	amount = (ch->level * ch->level * 250) + 1000;

	if (argument[0] == '\0') {
		char_printf(ch, "It will cost you %d gold.\n\r", amount);
		return;
	}

	if (ch->gold < amount) {
		char_puts("You don't have enough money "
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

	char_puts("That is not a valid choice.\n\r"
		     "Choose from", ch);
	for (i = 0; hometown_table[i].name; i++) {
		static char* comma = ", ";

		char_printf(ch, "%s%s", i == 0 ? comma+1 : comma,
			    hometown_table[i].name);
	}
	char_puts(".\n\r", ch);
}

void do_detect_hidden(CHAR_DATA *ch, const char *argument)
{
	AFFECT_DATA	af;
	int		chance;
	int		sn;

	if ((sn = sn_lookup("detect hide")) < 0
	||  (chance = get_skill(ch, sn)) == 0) {
		char_puts("Huh?\n\r", ch);
		return;
	}

	if (IS_AFFECTED(ch, AFF_DETECT_HIDDEN)) {
		char_puts("You are already as alert as you can be. \n\r",ch);
		return;
	}

	if (number_percent() > chance) {
		char_puts("You peer intently at the shadows "
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
	char_puts("Your awareness improves.\n\r", ch);
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
		char_puts("Huh?\n\r", ch);
		return;
	}

	char_puts("You call for bears help you.\n\r",ch);
	act("$n shouts a bear call.",ch,NULL,NULL,TO_ROOM);

	if (is_affected(ch, sn)) {
		char_puts("You cannot summon the strength to handle "
			     "more bears right now.\n\r", ch);
		return;
	}

	for (gch = char_list; gch != NULL; gch = gch->next) {
		if (IS_NPC(gch) && IS_AFFECTED(gch,AFF_CHARM)
		&&  gch->master == ch
		&&  gch->pIndexData->vnum == MOB_VNUM_BEAR) {
			char_puts("What's wrong with the bear you've got?",
				     ch);
			return;
		}
	}

	if (ch->in_room != NULL
	&&  IS_SET(ch->in_room->room_flags, ROOM_NOMOB)) {
		char_puts("No bears listen you.\n\r", ch);
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
		char_puts("No bears come to your rescue.\n\r", ch);
		return;
	}

	mana = SKILL(sn)->min_mana;
	if (ch->mana < mana) {
		char_puts("You don't have enough mana "
			     "to shout a bear call.\n\r", ch);
		return;
	}
	ch->mana -= mana;

	if (number_percent() > chance) {
		char_puts("No bears listen you.\n\r", ch);
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
	char_puts("Two bears come to your rescue!\n\r",ch);
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
		 char_puts("You are not carrying that.\n\r", ch);
		 return;
	}

	for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
		if (IS_NPC(rch) && rch->pIndexData->vnum == MOB_VNUM_SAGE)
			break;

	if (!rch) {
		 char_puts("No one here seems to know much "
			      "about that.\n\r", ch);
		 return;
	}

	if (IS_IMMORTAL(ch))
		act("$n looks at you!", rch, obj, ch, TO_VICT);
	else if (ch->gold < 1) {
		act("$n resumes to identify by looking at $p.",
		       rch, obj, 0, TO_ROOM);
		char_puts(" You need at least 1 gold.\n\r", ch);
		return;
	}
	else {
		ch->gold -= 1;
		char_puts("Your purse feels lighter.\n\r", ch);
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
		MSG("You are", ch->lang),
		MSG(flag_string(position_names, ch->position), ch->lang));

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
	send_to_char(buf_string(output), ch);
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
		MSG("You are", ch->lang),
		ch->name,
		IS_NPC(ch) ? str_empty : ch->pcdata->title, ch->level, get_age(ch),
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

	buf_printf(output, "You are %s.\n\r",
		   MSG(flag_string(position_names, ch->position), ch->lang));

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
	send_to_char(buf_string(output), ch);
	buf_free(output);
}

void show_affects(CHAR_DATA *ch, BUFFER *output)
{
	AFFECT_DATA *paf, *paf_last = NULL;

	if (ch->affected == NULL) {
		buf_add(output, MSG("You are not affected by any spells.\n\r", ch->lang));
		return;
	}

	buf_add(output, MSG("You are affected by the following spells:\n\r", ch->lang));
	for (paf = ch->affected; paf != NULL; paf = paf->next) {
		if (paf_last != NULL && paf->type == paf_last->type)
			if (ch->level >= 20)
				buf_add(output, "                      ");
			else
				continue;
		else
			buf_printf(output, "Spell: {c%-15s{x",
				   skill_name(paf->type));

		if (ch->level >= 20) {
			buf_printf(output, ": modifies {c%s{x by {c%d{x ",
				   flag_string(apply_flags, paf->location),
				   paf->modifier);
			if (paf->duration == -1 || paf->duration == -2)
				buf_add(output, MSG("permanently.", ch->lang));
			else
				buf_printf(output, MSG("for {c%d{x hours.", ch->lang),
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
		char_puts("Huh?\n\r", ch);
		return;
	}

	char_puts("You call for lions help you.\n\r",ch);
	act("$n shouts a lion call.",ch,NULL,NULL,TO_ROOM);

	if (is_affected(ch, sn)) {
		char_puts("You cannot summon the strength to handle "
			     "more lions right now.\n\r", ch);
		return;
	}

	for (gch = char_list; gch != NULL; gch = gch->next) {
		if (IS_NPC(gch) && IS_AFFECTED(gch,AFF_CHARM)
		&&  gch->master == ch
		&& gch->pIndexData->vnum == MOB_VNUM_LION) {
			char_puts("What's wrong with the lion "
				     "you've got?",ch);
			return;
		}
	}

	if (ch->in_room != NULL
	&& IS_SET(ch->in_room->room_flags, ROOM_NOMOB)) {
		char_puts("No lions can listen you.\n\r", ch);
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
		char_puts("No lions come to your rescue.\n\r", ch);
		return;
	}

	mana = SKILL(sn)->min_mana;
	if (ch->mana < mana) {
		char_puts("You don't have enough mana "
			     "to shout a lion call.\n\r", ch);
		return;
	}
	ch->mana -= mana;

	if (number_percent() > chance) {
		check_improve(ch, sn, FALSE, 1);
		char_puts("No lions listen you.\n\r", ch);
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
	char_puts("Two lions come to your rescue!\n\r",ch);
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
char *get_cond_alias(OBJ_DATA *obj)
{
	char *stat;
	int istat;

	istat = obj->condition;

	if	(istat >  99)	stat = "excellent";
	else if (istat >= 80)	stat = "good";
	else if (istat >= 60)	stat = "fine";
	else if (istat >= 40)	stat = "average";
	else if (istat >= 20)	stat = "poor";
	else			stat = "fragile";

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

		for (i = 0; i < ch->pcdata->learned.nused; i++) {
			ps = VARR_GET(&ch->pcdata->learned, i);

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
		char_puts("You have no practice sessions left.\n\r", ch);
		return;
	}

	one_argument(argument, arg);

	if ((ps = skill_vlookup(&ch->pcdata->learned, arg)) == NULL
	||  ps->percent == 0
	||  skill_level(ch, sn = ps->sn) > ch->level) {
		char_puts("You can't practice that.\n\r", ch);
		return;
	}

	if (sn == gsn_vampire) {
		char_puts("You can't practice that, only available "
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
		char_puts("Huh?\n\r", ch);
		return;
	}

	if (is_affected(ch, sn)) {
		char_puts("You don't have enough power to handle more "
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
		char_puts("There are not enough leaves to camp here.\n\r",
			     ch);
		return;
	}

	mana = SKILL(sn)->min_mana;
	if (ch->mana < mana) {
		char_puts("You don't have enough mana to make a camp.\n\r",
			     ch);
		return;
	}
	ch->mana -= mana;

	if (number_percent() > chance) {
		char_puts("You failed to make your camp.\n\r", ch);
		check_improve(ch, sn, FALSE, 4);
		return;
	}

	check_improve(ch, sn, TRUE, 4);
	WAIT_STATE(ch, SKILL(sn)->beats);

	char_puts("You succeeded to make your camp.\n\r", ch);
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
		char_puts("You can't do that.\n\r", ch);
		return;
	}

	if (arg1[0] == '\0' || arg2[0] == '\0') {
		char_puts("Demand what from whom?\n\r", ch);
		return;
	}

	if ((victim = get_char_room(ch, arg2)) == NULL) {
		char_puts("They aren't here.\n\r", ch);
		return;
	}

	if (!IS_NPC(victim)) {
		char_puts("Why don't you just want that directly "
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
		char_puts("Your hands are full.\n\r", ch);
		return;
	}

	if (ch->carry_weight + get_obj_weight(obj) > can_carry_w(ch)) {
		char_puts("You can't carry that much weight.\n\r", ch);
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
	char_puts("Your power makes all around the world shivering.\n\r",ch);
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
		char_puts("Huh?\n\r", ch);
		return;
	}

	if (arg[0] == '\0') {
		char_puts("Charm what?\n\r", ch);
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL) {
		char_puts("They aren't here.\n\r", ch);
		return;
	}

	if (race_table[ORG_RACE(victim)].pc_race) {
		char_puts("You should try this on monsters?\n\r", ch);
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
		char_puts("You don't know how to make arrows.\n\r", ch);
		return;
	}

	if (ch->in_room->sector_type != SECT_FIELD
	&&  ch->in_room->sector_type != SECT_FOREST
	&&  ch->in_room->sector_type != SECT_HILLS) {
		char_puts("You couldn't find enough wood.\n\r", ch);
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
		char_puts("You don't know how to make "
			     "that kind of arrow.\n\r", ch);
		return;
	}

	if (color) {
		mana += SKILL(color)->min_mana;
		wait += SKILL(color)->beats;
	}

	if (ch->mana < mana) {
		char_puts("You don't have enough energy "
			     "to make that kind of arrows.\n\r", ch);
		return;
	}

	ch->mana -= mana;
	WAIT_STATE(ch, wait);

	char_puts("You start to make arrows!\n\r",ch);
	act("$n starts to make arrows!",ch,NULL,NULL,TO_ROOM);
	for(count = 0; count < ch->level / 5; count++) {
		if (number_percent() > chance) {
			char_puts("You failed to make the arrow, "
				     "and broke it.\n\r", ch);
			check_improve(ch, sn, FALSE, 3);
			if (color)
				check_improve(ch, color, FALSE, 3);
			continue;
		}

		char_puts("You successfully make an arrow.\n\r", ch);
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
		char_puts("You don't know how to make bows.\n\r", ch);
		return;
	}

	if (ch->in_room->sector_type != SECT_FIELD
	&&  ch->in_room->sector_type != SECT_FOREST
	&&  ch->in_room->sector_type != SECT_HILLS) {
		char_puts("You couldn't find enough wood.\n\r", ch);
		return;
	}

	mana = SKILL(sn)->min_mana;
	if (ch->mana < mana) {
		char_puts("You don't have enough energy to make a bow.\n\r",
			     ch);
		return;
	}
	ch->mana -= mana;
	WAIT_STATE(ch, SKILL(sn)->beats);

	if (number_percent() > chance) {
		char_puts("You failed to make the bow, and broke it.\n\r",
			     ch);
		check_improve(ch, sn, FALSE, 1);
		return;
	}
	char_puts("You successfully make bow.\n\r", ch);
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
		char_puts("You can make either bow or arrow.\n\r",ch);
		return;
	}

	if (!str_prefix(arg, "arrow"))
		do_make_arrow(ch, argument);
	else if (!str_prefix(arg, "bow"))
		do_make_bow(ch, argument);
	else
		do_make(ch, str_empty);
}

