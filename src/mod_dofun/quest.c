/*
 * $Id: quest.c,v 1.22 1998-05-27 08:47:28 fjoe Exp $
 */

/***************************************************************************
 *     ANATOLIA 2.1 is copyright 1996-1997 Serdar BULUT 		   *
 *     ANATOLIA has been brought to you by ANATOLIA consortium		   *
 *	 Serdar BULUT {Chronos} 	bulut@rorqual.cc.metu.edu.tr	   *
 *	 Ibrahim Canpunar  {Mandrake}	canpunar@rorqual.cc.metu.edu.tr    *
 *	 Murat BICER  {KIO}		mbicer@rorqual.cc.metu.edu.tr	   *
 *	 D.Baris ACAR {Powerman}	dbacar@rorqual.cc.metu.edu.tr	   *
 *     By using this code, you have agreed to follow the terms of the	   *
 *     ANATOLIA license, in the file Anatolia/anatolia.licence		   *
 ***************************************************************************/

/****************************************************************************
*  Automated Quest code written by Vassago of MOONGATE, moongate.ams.com    *
*  4000. Copyright (c) 1996 Ryan Addams, All Rights Reserved. Use of this   *
*  code is allowed provided you add a credit line to the effect of:	    *
*  "Quest Code (c) 1996 Ryan Addams" to your logon screen with the rest     *
*  of the standard diku/rom credits. If you use this or a modified version  *
*  of this code, let me know via email: moongate@moongate.ams.com. Further  *
*  updates will be posted to the rom mailing list. If you'd like to get     *
*  the latest version of quest.c, please send a request to the above add-   *
*  ress. Quest Code v2.01. Please do not remove this notice from this file. *
****************************************************************************/

/****************************************************************************
 *  Revised and adopted to Anatolia by chronos. 			    *
 ****************************************************************************/

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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "recycle.h"
#include "db.h"
#include "comm.h"
#include "resource.h"
#include "hometown.h"
#include "magic.h"
#include "quest.h"

void do_tell_quest(CHAR_DATA *ch, CHAR_DATA *victim, char *argument);
extern	MOB_INDEX_DATA	*mob_index_hash [MAX_KEY_HASH];


/* Local functions */

void generate_quest	args((CHAR_DATA *ch, CHAR_DATA *questman));
void quest_update	args((void));
bool chance		args((int num));
ROOM_INDEX_DATA *	find_location	args((CHAR_DATA *ch, char *arg));

/* CHANCE function. I use this everywhere in my code, very handy :> */

bool chance(int num)
{
    if (number_range(1,100) <= num) return TRUE;
    else return FALSE;
}

/* The main quest function */

void do_quest(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *questman;
	OBJ_DATA *obj = NULL, *obj_next;
	OBJ_INDEX_DATA *qinfoobj;
	MOB_INDEX_DATA *questinfo;
	char buf [MAX_STRING_LENGTH];
	char bufvampire[100];
	char bufsamurai[100];
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	int sn, trouble_vnum = 0, trouble_n;

	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (IS_NPC(ch)) 
		return;

	if (!strcmp(arg1, "info")) {
		if (IS_QUESTOR(ch)) {
			if (ch->pcdata->questmob == -1) {
				send_to_char(msg(QUEST_IS_ALMOST_COMPLETE, ch), 
						ch);
			} else if (ch->pcdata->questobj > 0) {
				qinfoobj = get_obj_index(ch->pcdata->questobj);
				if (qinfoobj != NULL) {
					char_printf(ch, 
						msg(QUEST_RECOVER_FABLED, ch), 
						qinfoobj->name);
					if (ch->pcdata->questroom)
					  char_printf(ch, 
					    msg(QUEST_INFO_LOCATION, ch),
					    ch->pcdata->questroom->area->name, 
					    ch->pcdata->questroom->name);
				} else 
					send_to_char(
					     msg(QUEST_ARENT_ON_QUEST, ch), ch);
					return;
			} else if (ch->pcdata->questmob > 0) {
				questinfo = 
				    get_mob_index(ch->pcdata->questmob);
				if (questinfo != NULL) {
					char_printf(ch, 
					    msg(QUEST_SLAY_DREADED, ch),
					    questinfo->short_descr);
					if (ch->pcdata->questroom)
					  char_printf(ch, 
					    msg(QUEST_INFO_LOCATION, ch),
					    ch->pcdata->questroom->area->name, 
					    ch->pcdata->questroom->name);
				} else 
					send_to_char(msg(QUEST_ARENT_ON_QUEST, 
							ch), ch);
				return;
			}
		} else
			send_to_char(msg(QUEST_ARENT_ON_QUEST, ch), ch);
		return;
	}

	if (!strcmp(arg1, "points")) {
		char_printf(ch, msg(QUEST_YOU_HAVE_D_QP, ch), 
				ch->pcdata->questpoints);
		return;
	} else if (!strcmp(arg1, "time")) {
		if (!IS_QUESTOR(ch)) {
			send_to_char(msg(QUEST_ARENT_ON_QUEST, ch), ch);
			if (ch->pcdata->questtime < -1) {
				sprintf(buf, msg(QUEST_D_MIN_REMAINING, ch), 
					    -ch->pcdata->questtime);
				send_to_char(buf, ch);
	    		} else if (ch->pcdata->questtime == -1) {
				send_to_char(msg(QUEST_LESS_MINUTE, ch), ch);
	    		}
		} else if (ch->pcdata->questtime > 0) {
			char_printf(ch, msg(QUEST_LEFT_FOR_QUEST, ch), 
					ch->pcdata->questtime);
		}
		return;
	}

/* Checks for a character in the room with spec_questmaster set. This special
   procedure must be defined in special.c. You could instead use an
   ACT_QUESTMASTER flag instead of a special procedure. */

	for (questman = ch->in_room->people; questman != NULL; 
			questman = questman->next_in_room) {
		if (!IS_NPC(questman)) 
			continue;
		if (questman->spec_fun == spec_lookup("spec_questmaster")) 
			break;
	}

	if (questman == NULL 
	    || questman->spec_fun != spec_lookup("spec_questmaster")) {
		send_to_char(msg(MOVE_YOU_CANT_DO_THAT_HERE, ch), ch);
		return;
	}

	if (questman->fighting != NULL) {
		send_to_char(msg(QUEST_WAIT_FIGHT_STOPS, ch), ch);
		return;
	}

	ch->pcdata->questgiver = questman->pIndexData->vnum;

	if (!strcmp(arg1, "list")) {
		act_nprintf(ch, NULL, questman, TO_ROOM, POS_RESTING, 
				QUEST_N_ASKS_LIST);
		act_nprintf(ch, NULL, questman, TO_CHAR, POS_DEAD, 
				QUEST_YOU_ASK_LIST);
		bufvampire[0] = '\0';
		if (ch->class == CLASS_VAMPIRE)
			sprintf(bufvampire,
				"  50qp...........Vampire skill\n\r");
		bufsamurai[0] = '\0';
		if (ch->class == CLASS_SAMURAI)
			sprintf(bufsamurai,"%s%s",
				"  50qp...........Decrease number of death\n\r",
				"  100qp..........Katana quest\n\r");
			sprintf(buf, 
			    "%s"
			    "  1000qp.........the Girth of Real Heroism\n\r"
			    "  1000qp.........the Ring of Real Heroism\n\r"
			    "  1000qp.........the Real Hero's Weapon\n\r"
			    "  500qp..........350,000 gold pieces\n\r"
			    "  500qp..........60 Practices\n\r"
			    "  200qp..........tattoo of your religion\n\r"
			    "%s%s%s", msg(QUEST_ITEMS_AVAIL_PURCHASE, ch),
			    bufvampire, bufsamurai, msg(QUEST_TYPE_BUY, ch));
			send_to_char(buf, ch);
			return;
	} else if (!strcmp(arg1, "buy")) {
		if (arg2[0] == '\0') {
			send_to_char(msg(QUEST_TYPE_BUY, ch), ch);
			return;
		} else if (is_name(arg2, "girth")) {
			if (ch->pcdata->questpoints >= 1000) {
				ch->pcdata->questpoints -= 1000;
				obj = create_object(get_obj_index(
					QUEST_ITEM1), ch->level);
				if (IS_SET(ch->quest,QUEST_GIRTH) ||
				    IS_SET(ch->quest,QUEST_GIRTH2) ||
				    IS_SET(ch->quest,QUEST_GIRTH3))
					do_tell_quest(ch, questman, 
					    msg(QUEST_ITEM_BEYOND, ch));
				else 
					SET_BIT(ch->quest,QUEST_GIRTH);
			} else {
				sprintf(buf, msg(QUEST_NOT_ENOUGH_QP, ch), 
					ch->name);
				do_tell_quest(ch, questman, buf);
				return;
			}
		} else if (is_name(arg2, "ring")) {
			if (ch->pcdata->questpoints >= 1000) {
				ch->pcdata->questpoints -= 1000;
				obj = create_object(get_obj_index(
					QUEST_ITEM2), ch->level);
				if (IS_SET(ch->quest,QUEST_RING) ||
				    IS_SET(ch->quest,QUEST_RING2) ||
				    IS_SET(ch->quest,QUEST_RING3))
					do_tell_quest(ch, questman, 
					    msg(QUEST_ITEM_BEYOND, ch));
				else 
					SET_BIT(ch->quest, QUEST_RING);
			} else {
				sprintf(buf, msg(QUEST_NOT_ENOUGH_QP, ch), 
					ch->name);
				do_tell_quest(ch, questman, buf);
				return;
			}
		} else if (is_name(arg2, "weapon")) {
			if (ch->pcdata->questpoints >= 1000) {
				ch->pcdata->questpoints -= 1000;
				obj = create_object(get_obj_index(
					QUEST_ITEM3), ch->level);
				if (IS_SET(ch->quest,QUEST_WEAPON) ||
				    IS_SET(ch->quest,QUEST_WEAPON2) ||
				    IS_SET(ch->quest,QUEST_WEAPON3))
					do_tell_quest(ch, questman, 
					    msg(QUEST_ITEM_BEYOND, ch));
				else 
					SET_BIT(ch->quest,QUEST_WEAPON);
			} else {
				sprintf(buf, msg(QUEST_NOT_ENOUGH_QP, ch), 
					ch->name);
				do_tell_quest(ch, questman, buf);
				return;
			}
		} else if (is_name(arg2, "practices pracs prac practice")) {
			if (ch->pcdata->questpoints >= 500) {
				ch->pcdata->questpoints -= 500;
				ch->practice += 60;
				act_nprintf(ch, NULL, questman, TO_ROOM,
					    POS_RESTING, QUEST_N_GIVES_PRACS);
				act_nprintf(ch, NULL, questman, TO_CHAR, 
					    POS_DEAD, QUEST_N_GIVES_YOU_PRACS);
				return;
			} else {
				sprintf(buf, msg(QUEST_NOT_ENOUGH_QP, ch), 
					ch->name);
				do_tell_quest(ch,questman,buf);
				return;
			}
		} else if (is_name(arg2, "vampire")) {
			if (ch->class != CLASS_VAMPIRE) {
				sprintf(buf, msg(QUEST_CANT_GAIN_VAMPIRE,ch),
					ch->name);
				do_tell_quest(ch,questman,buf);
				return;
			}
			if (ch->pcdata->questpoints >= 50) {
				ch->pcdata->questpoints -= 50;
				sn = skill_lookup("vampire");
				ch->pcdata->learned[sn] = 100;
				act_nprintf(ch, NULL, questman, TO_ROOM, 
					   POS_RESTING, QUEST_N_GIVES_SECRET);
				act_nprintf(ch, NULL, questman, TO_CHAR, 
					   POS_DEAD, QUEST_N_GIVES_YOU_SECRET);
				act_nprintf(ch, NULL, questman, TO_ALL, POS_DEAD,
					    UPDATE_WEATHER_LIGHTNING_FLASHES);
				return;
			} else {
				sprintf(buf, msg(QUEST_NOT_ENOUGH_QP, ch), 
					ch->name);
				do_tell_quest(ch,questman,buf);
				return;
			}
		} else if (is_name(arg2, "dead samurai death")) {
			if (ch->class != CLASS_SAMURAI) {
				sprintf(buf, msg(QUEST_NOT_SAMURAI, ch),
					ch->name);
				do_tell_quest(ch,questman,buf);
				return;
			}

			if (ch->pcdata->death < 1) {
				sprintf(buf, msg(QUEST_NO_DEATHS, ch),
						ch->name);
				do_tell_quest(ch,questman,buf);
				return;
			}

			if (ch->pcdata->questpoints >= 50) {
				ch->pcdata->questpoints -= 50;
				ch->pcdata->death -= 1;
			} else {
				sprintf(buf, msg(QUEST_NOT_ENOUGH_QP, ch), 
					ch->name);
				do_tell_quest(ch,questman,buf);
				return;
			}
		} else if (is_name(arg2, "katana sword")) {
			AFFECT_DATA af;
			OBJ_DATA *katana;
			if (ch->class != CLASS_SAMURAI) {
				sprintf(buf, msg(QUEST_NOT_SAMURAI, ch),
					ch->name);
				do_tell_quest(ch,questman,buf);
				return;
			}

			if ((katana = get_obj_list(ch,
				"katana",ch->carrying)) == NULL) {
				sprintf(buf, msg(QUEST_DONT_HAVE_KATANA, ch),
					ch->name);
				do_tell_quest(ch,questman,buf);
				return;
			}

			if (ch->pcdata->questpoints >= 100) {
				ch->pcdata->questpoints -= 100;
				af.where	= TO_WEAPON;
				af.type 	= gsn_katana;
				af.level	= 100;
				af.duration	= -1;
				af.modifier	= 0;
				af.bitvector	= WEAPON_KATANA;
				af.location	= APPLY_NONE;
				affect_to_obj(katana, &af);
				sprintf(buf, msg(QUEST_AS_YOU_WIELD_IT, ch));
				do_tell_quest(ch,questman,buf);
			} else {
				sprintf(buf, msg(QUEST_NOT_ENOUGH_QP, ch), 
					ch->name);
				do_tell_quest(ch,questman,buf);
				return;
			}
		} else if (is_name(arg2, "tattoo religion")) {
			OBJ_DATA *tattoo;
			if (!(ch->religion)) {
				send_to_char(msg(QUEST_NO_RELIGION, ch), ch);
				return;
			}
			tattoo = get_eq_char(ch, WEAR_TATTOO);
			if (tattoo != NULL) {
				send_to_char(msg(QUEST_ALREADY_TATTOOED,
						     ch), ch);
				return;
			}

			if (ch->pcdata->questpoints >= 200) {
				ch->pcdata->questpoints -= 200;

				tattoo = create_object(get_obj_index(
					religion_table[ch->religion].vnum),100);

				obj_to_char(tattoo, ch);
				equip_char(ch, tattoo, WEAR_TATTOO);
				act_nprintf(ch, tattoo, questman, TO_ROOM, 
						POS_RESTING, QUEST_N_TATTOOS_N);
				act_nprintf(ch, tattoo, questman, TO_CHAR, 
						POS_DEAD, QUEST_N_TATTOOS_YOU);
				return;
			} else {
				sprintf(buf, msg(QUEST_NOT_ENOUGH_QP, ch), 
					ch->name);
				do_tell_quest(ch, questman, buf);
				return;
			}
		} else if (is_name(arg2, "gold gp")) {
			if (ch->pcdata->questpoints >= 500) {
				ch->pcdata->questpoints -= 500;
				ch->gold += 350000;
				act_nprintf(ch, NULL, questman, TO_ROOM, 
					POS_RESTING, QUEST_N_GIVES_GOLD);
				act_nprintf(ch, NULL, questman, TO_CHAR, 
					POS_DEAD, QUEST_N_GIVES_YOU_GOLD);
				return;
			} else {
				sprintf(buf, msg(QUEST_NOT_ENOUGH_QP, ch), 
					ch->name);
				do_tell_quest(ch, questman, buf);
				return;
			}
		} else {
			sprintf(buf, msg(QUEST_NOT_HAVE_ITEM, ch), ch->name);
			do_tell_quest(ch,questman,buf);
		}

		if (obj != NULL) {
			if (obj->pIndexData->vnum == QUEST_ITEM1
		    	|| obj->pIndexData->vnum == QUEST_ITEM2
		    	|| obj->pIndexData->vnum == QUEST_ITEM3) {
				sprintf(buf, obj->short_descr,
					IS_GOOD(ch) ? "holy" :
						IS_NEUTRAL(ch) ? "blue-green" : 
								"evil", 
					ch->name);
				free_string(obj->short_descr);
				obj->short_descr = str_dup(buf);
			}
			act_nprintf(ch, obj, questman, TO_ROOM, POS_RESTING, 
					QUEST_GIVES_P_TO_N);
			act_nprintf(ch, obj, questman, TO_CHAR, POS_DEAD, 
					QUEST_GIVES_YOU_P);
			obj_to_char(obj, ch);
		}
		return;
	} else if (!strcmp(arg1, "request")) {
		act_nprintf(ch, NULL, questman, TO_ROOM, POS_RESTING, 
				QUEST_N_ASKS_FOR_QUEST);
		act_nprintf(ch, NULL, questman, TO_CHAR, POS_DEAD, 
				QUEST_YOU_ASK_FOR_QUEST);
		if (IS_QUESTOR(ch)) {
	    		sprintf(buf, msg(QUEST_YOU_ALREADY_ON_QUEST, ch));
	    		do_tell_quest(ch,questman,buf);
	    		return;
		} 
		if (ch->pcdata->questtime < 0) {
	    		sprintf(buf, msg(QUEST_BRAVE_BUT_LET_SOMEONE_ELSE, ch), 
				ch->name);
	    		do_tell_quest(ch,questman,buf);
			sprintf(buf, msg(QUEST_COME_BACK_LATER, ch));
			do_tell_quest(ch, questman, buf);
			return;
		}

		sprintf(buf, msg(QUEST_THANK_YOU_BRAVE, ch), ch->name);
		do_tell_quest(ch,questman,buf);

		generate_quest(ch, questman);

		if (ch->pcdata->questmob > 0 
		    || ch->pcdata->questobj > 0) {
			ch->pcdata->questtime = number_range(15,30);
			sprintf(buf, msg(QUEST_YOU_HAVE_D_MINUTES, ch), 
				ch->pcdata->questtime);
			do_tell_quest(ch,questman,buf);
			sprintf(buf, msg(QUEST_MAY_THE_GODS_GO, ch));
			do_tell_quest(ch, questman, buf);
		}
		return;
	} else if (!strcmp(arg1, "complete")) {
			act_nprintf(ch, NULL, questman, TO_ROOM, POS_RESTING, 
					QUEST_INFORMS_COMPLETE);
			act_nprintf(ch, NULL, questman, TO_CHAR, POS_DEAD, 
					QUEST_YOU_INFORM_COMPLETE);
			if (ch->pcdata->questgiver != questman->pIndexData->vnum) {
				sprintf(buf, vmsg(QUEST_NEVER_QUEST, ch, questman));
				do_tell_quest(ch,questman,buf);
				return;
			}

			if (IS_QUESTOR(ch)) {
				if (ch->pcdata->questmob == -1 
				    && ch->pcdata->questtime > 0) {
					int reward = 0, pointreward = 0, 
					    pracreward = 0, level;

					level = ch->level;
					reward = dice(level, 30);
					reward=URANGE(00,reward,20*ch->level);
					pointreward = number_range(20, 40);

					sprintf(buf, msg(QUEST_GRATS_COMPLETE, 
						ch));
					do_tell_quest(ch,questman,buf);
					sprintf(buf, msg(QUEST_AS_A_REWARD, ch), 
						pointreward, reward);
					do_tell_quest(ch, questman, buf);
					if (chance(2)) {
					pracreward = number_range(1, 6);
					sprintf(buf, msg(QUEST_GAIN_PRACS, ch), 
						pracreward);
					send_to_char(buf, ch);
					ch->practice += pracreward;
				}

				cancel_quest(ch);
				ch->pcdata->questtime = -5;
				ch->gold += reward;
				ch->pcdata->questpoints += pointreward;

				return;
			} else if (ch->pcdata->questobj > 0 
				   && ch->pcdata->questtime > 0) {
				bool obj_found = FALSE;

				for (obj = ch->carrying; obj != NULL; obj= obj_next) {
					obj_next = obj->next_content;

					if (obj != NULL 
				   &&obj->pIndexData->vnum==ch->pcdata->questobj
				   && strstr(obj->extra_descr->description, 
						     ch->name) != NULL) {
						obj_found = TRUE;
						break;
					}
				}
				if (obj_found == TRUE) {
					int reward, pointreward, pracreward;

					reward=number_range(350,20 * ch->level);
					pointreward = number_range(15,40);

					act_nprintf(ch, obj, questman, TO_CHAR, 
						POS_DEAD, QUEST_YOU_HAND_P);
					act_nprintf(ch, obj, questman, TO_ROOM, 
						POS_RESTING, QUEST_N_HANDS_P);

					sprintf(buf, 
						msg(QUEST_GRATS_COMPLETE, ch));
					do_tell_quest(ch,questman,buf);
					sprintf(buf, msg(QUEST_AS_A_REWARD, ch), 
							pointreward, reward);
					do_tell_quest(ch,questman,buf);
					if (chance(15)) {
					pracreward = number_range(1, 6);
						sprintf(buf, 
						    msg(QUEST_GAIN_PRACS, ch), 
						    pracreward);
						send_to_char(buf, ch);
						ch->practice += pracreward;
					}

					cancel_quest(ch);
					ch->pcdata->questtime = -5;
					ch->gold += reward;
					ch->pcdata->questpoints += pointreward;
					extract_obj(obj);
					return;
				} else {
					sprintf(buf, msg(QUEST_HAVENT_COMPLETE, ch));
					do_tell_quest(ch,questman,buf);
					return;
				}
				return;
			} else if ((ch->pcdata->questmob > 0 
				    || ch->pcdata->questobj > 0) 
				    && ch->pcdata->questtime > 0) {
					sprintf(buf, msg(QUEST_HAVENT_COMPLETE, ch));
					do_tell_quest(ch,questman,buf);
					return;
				}
			}
			if (ch->pcdata->questtime < 0)
				sprintf(buf, 
					msg(QUEST_DIDNT_COMPLETE_IN_TIME, ch));
			else 
				sprintf(buf, msg(QUEST_HAVE_TO_REQUEST, ch), 
					ch->name);
			do_tell_quest(ch,questman,buf);
			return;
		} else if (!strcmp(arg1, "trouble")) {
			if (arg2[0] == '\0') {
				send_to_char(msg(QUEST_TYPE_TROUBLE, ch), ch);
				return;
			}

			trouble_n = 0;
			if (is_name(arg2, "girth")) {
				if (IS_SET(ch->quest,QUEST_GIRTH)) {
					REMOVE_BIT(ch->quest,QUEST_GIRTH);
					SET_BIT(ch->quest,QUEST_GIRTH2);
					trouble_n = 1;
				} else if (IS_SET(ch->quest,QUEST_GIRTH2)) {
					REMOVE_BIT(ch->quest,QUEST_GIRTH2);
					SET_BIT(ch->quest,QUEST_GIRTH3);
					trouble_n = 2;
				} else if (IS_SET(ch->quest,QUEST_GIRTH3)) {
					REMOVE_BIT(ch->quest,QUEST_GIRTH3);
					trouble_n = 3;
				}
				if (trouble_n) 
					trouble_vnum = QUEST_ITEM1;
			} else if (is_name(arg2, "weapon")) {
				if (IS_SET(ch->quest,QUEST_WEAPON)) {
					REMOVE_BIT(ch->quest,QUEST_WEAPON);
					SET_BIT(ch->quest,QUEST_WEAPON2);
					trouble_n = 1;
				} else if (IS_SET(ch->quest,QUEST_WEAPON2)) {
					REMOVE_BIT(ch->quest,QUEST_WEAPON2);
					SET_BIT(ch->quest,QUEST_WEAPON3);
					trouble_n = 2;
				} else if (IS_SET(ch->quest,QUEST_WEAPON3)) {
					REMOVE_BIT(ch->quest,QUEST_WEAPON3);
					trouble_n = 3;
				}
			if (trouble_n) 
				trouble_vnum = QUEST_ITEM3;
		} else if (is_name(arg2, "ring")) {
			if (IS_SET(ch->quest,QUEST_RING)) {
				REMOVE_BIT(ch->quest,QUEST_RING);
				SET_BIT(ch->quest,QUEST_RING2);
				trouble_n = 1;
			} else if (IS_SET(ch->quest,QUEST_RING2)) {
				REMOVE_BIT(ch->quest,QUEST_RING2);
				SET_BIT(ch->quest,QUEST_RING3);
				trouble_n = 2;
			} else if (IS_SET(ch->quest,QUEST_RING3)) {
				REMOVE_BIT(ch->quest,QUEST_RING3);
				trouble_n = 3;
			}
			if (trouble_n) 
				trouble_vnum = QUEST_ITEM2;
		}
		if (!trouble_n) {
			sprintf(buf, msg(QUEST_HAVENT_BOUGHT, ch), ch->name);
			do_tell_quest(ch,questman,buf);
			return;
		}

		for (obj = object_list; obj != NULL; obj = obj_next) {
			obj_next = obj->next;
			if (obj->pIndexData->vnum == trouble_vnum 
			    && strstr(obj->short_descr, ch->name)) {
				extract_obj(obj);
				break;
			}
		}
		obj = create_object(get_obj_index(trouble_vnum),ch->level);
		sprintf(buf, obj->short_descr,
				IS_GOOD(ch) ? "holy" :
				IS_NEUTRAL(ch) ? "blue-green" : "evil", 
			ch->name);
		free_string(obj->short_descr);
		obj->short_descr = str_dup(buf);
		act_nprintf(ch, obj, questman, TO_ROOM, POS_RESTING, 
				QUEST_GIVES_P_TO_N);
		act_nprintf(ch, obj, questman, TO_CHAR, POS_DEAD, 
				QUEST_GIVES_YOU_P);
		obj_to_char(obj, ch);
		sprintf(buf, msg(QUEST_THIS_IS_THE_I_S, ch), trouble_n,
			(trouble_n == 1) ? msg(QUEST_ST, ch) : 
				(trouble_n == 2) ? msg(QUEST_ND, ch) : 
					msg(QUEST_RD, ch));
		do_tell_quest(ch,questman,buf);
		if (trouble_n == 3) {
			sprintf(buf, msg(QUEST_WONT_GIVE_AGAIN, ch));
			do_tell_quest(ch,questman,buf);
		}
		return;
	}

	send_to_char(msg(QUEST_COMMANDS, ch), ch);
	send_to_char(msg(QUEST_TYPE_HELP_QUEST, ch), ch);
	return;
}

void generate_quest(CHAR_DATA *ch, CHAR_DATA *questman)
{
    CHAR_DATA *victim;
    MOB_INDEX_DATA *vsearch;
    ROOM_INDEX_DATA *room;
    OBJ_DATA *eyed;
    char buf [MAX_STRING_LENGTH];
    int level_diff, i;
    int mob_buf[1000],mob_count;
    int found;

    room	=	alloc_perm(sizeof (*room));

    mob_count = 0;
    for (i=0; i< MAX_KEY_HASH; i++)
    {
     if ((vsearch  = mob_index_hash[i]) == NULL) continue;
     level_diff = vsearch->level - ch->level;
     if ((ch->level < 51 && (level_diff > 4 || level_diff < -1))
	   || (ch->level > 50 && (level_diff > 6 || level_diff < 0))
	   || vsearch->pShop != NULL
	   || IS_SET(vsearch->act,ACT_TRAIN)
	   || IS_SET(vsearch->act,ACT_PRACTICE)
	   || IS_SET(vsearch->act,ACT_IS_HEALER)
	   || IS_SET(vsearch->act,ACT_NOTRACK)
	   || IS_SET(vsearch->imm_flags, IMM_SUMMON))
	continue;
     mob_buf[mob_count] = vsearch->vnum;
     mob_count++;
     if (mob_count > 999) break;
    }

    if (chance(40))
    {
	int objvnum = 0;
	int i;


	found = number_range(0,mob_count-1);
	for(i=0; i< mob_count; i++)
	{
	 if ((vsearch = get_mob_index(mob_buf[found])) == NULL)
		{
		 bug("Error unknown mob in quest: %d",i);
		 found++;
		 if (found > (mob_count-1)) break;
		 else continue;
		}
	 else break;
	}

       if (vsearch == NULL || (victim = get_char_world(ch, vsearch->player_name)) == NULL)
       {
	sprintf(buf, msg(QUEST_DONT_HAVE_QUESTS, ch));
	do_tell_quest(ch,questman,buf);
	sprintf(buf, msg(QUEST_TRY_AGAIN_LATER, ch));
	do_tell_quest(ch,questman,buf);
	ch->pcdata->questtime = -5;
	return;
       }

       if ((room = find_location(ch, victim->name)) == NULL)
       {
	sprintf(buf, msg(QUEST_DONT_HAVE_QUESTS, ch));
	do_tell_quest(ch,questman,buf);
	sprintf(buf, msg(QUEST_TRY_AGAIN_LATER, ch));
	do_tell_quest(ch,questman,buf);
	ch->pcdata->questtime = -5;
	return;
       }

	switch(number_range(0,3))
	{
	    case 0:
	    objvnum = QUEST_OBJQUEST1;
	    break;

	    case 1:
	    objvnum = QUEST_OBJQUEST2;
	    break;

	    case 2:
	    objvnum = QUEST_OBJQUEST3;
	    break;

	    case 3:
	    objvnum = QUEST_OBJQUEST4;
	    break;

	}


	if (IS_GOOD(ch))
		i=0;
	else if (IS_EVIL(ch))
		i=2;
	else i = 1;

	eyed = create_object(get_obj_index(objvnum), ch->level);
	eyed->owner = str_dup(ch->name);
	eyed->from = str_dup(ch->name);
	eyed->altar = hometown_table[ch->hometown].altar[i];
	eyed->pit = hometown_table[ch->hometown].pit[i];
	eyed->level = ch->level;

	sprintf(buf, eyed->description, ch->name);
	free_string(eyed->description);
	eyed->description = str_dup(buf);

	sprintf(buf, eyed->pIndexData->extra_descr->description, ch->name);
	eyed->extra_descr = new_extra_descr();
	eyed->extra_descr->keyword =
		  str_dup(eyed->pIndexData->extra_descr->keyword);
	eyed->extra_descr->description = str_dup(buf);
	eyed->extra_descr->next = NULL;

	eyed->cost = 0;
	eyed->timer = 30;

	obj_to_room(eyed, room);
	ch->pcdata->questobj = eyed->pIndexData->vnum;
	ch->pcdata->questroom = room;

	sprintf(buf, msg(QUEST_VILE_PILFERERS, ch), eyed->short_descr);
	do_tell_quest(ch,questman,buf);
	do_tell_quest(ch,questman, msg(QUEST_MY_COURT_WIZARDESS, ch));

	/* I changed my area names so that they have just the name of the area
	   and none of the level stuff. You may want to comment these next two
	   lines. - Vassago */

	sprintf(buf, msg(QUEST_LOCATION_IS_IN_AREA, ch), room->area->name, room->name);
	do_tell_quest(ch,questman,buf);
	return;
    }

    /* Quest to kill a mob */

    else
    {
	found = number_range(0,mob_count-1);
	for(i=0; i< mob_count; i++) {
	 if ((vsearch = get_mob_index(mob_buf[found])) == NULL
	      || (IS_EVIL(vsearch) && !IS_EVIL(ch))
	      || (IS_GOOD(vsearch) && !IS_GOOD(ch))
	      || (IS_NEUTRAL(vsearch) && !IS_GOOD(ch))) {
		/*
		 * bug("Error unknown mob in quest: %d",i);
		 */
		 found++;
		 if (found > (mob_count-1)) 
			break;
		 else 
			continue;
		}
	 else break;
	}

     if (vsearch == NULL
	 || (victim = get_char_world(ch, vsearch->player_name)) == NULL
	 || (room = find_location(ch, victim->name)) == NULL
	 || IS_SET(room->area->area_flag,AREA_HOMETOWN))
     {
	sprintf(buf, msg(QUEST_DONT_HAVE_QUESTS, ch));
	do_tell_quest(ch,questman,buf);
	sprintf(buf, msg(QUEST_TRY_AGAIN_LATER, ch));
	do_tell_quest(ch,questman,buf);
	ch->pcdata->questtime = -5;
	return;
     }

    if (IS_GOOD(ch))
       {
	sprintf(buf, msg(QUEST_RUNES_MOST_HEINOUS, ch), victim->short_descr);
	do_tell_quest(ch,questman,buf);
	sprintf(buf, vmsg(QUEST_HAS_MURDERED, ch, victim), victim->short_descr, 
			number_range(2,20));
	do_tell_quest(ch, questman, buf);
	do_tell_quest(ch, questman, msg(QUEST_THE_PENALTY_IS, ch));
	}
    else
       {
	sprintf(buf, msg(QUEST_ENEMY_OF_MINE, ch), victim->short_descr);
	do_tell_quest(ch, questman, buf);
	sprintf(buf, msg(QUEST_ELEMINATE_THREAT, ch));
	do_tell_quest(ch,questman,buf);
       }

     if (room->name != NULL)
     {
	sprintf(buf, msg(QUEST_SEEK_S_OUT, ch), victim->short_descr,room->name);
	do_tell_quest(ch,questman,buf);

	/* I changed my area names so that they have just the name of the area
	   and none of the level stuff. You may want to comment these next two
	   lines. - Vassago */

	ch->pcdata->questroom = room;
	sprintf(buf, msg(QUEST_LOCATION_IS_IN_AREA, ch), room->area->name, room->name);
	do_tell_quest(ch,questman,buf);
     }
     ch->pcdata->questmob = victim->pIndexData->vnum;
     victim->hunter = ch;
    }
    return;
}

/* Called from update_handler() by pulse_area */
void cancel_quest(CHAR_DATA *ch)
{
	CHAR_DATA *fch;
	ch->pcdata->questtime = 0;
	ch->pcdata->questgiver = 0;
	ch->pcdata->questmob = 0;
	ch->pcdata->questobj = 0;
	ch->pcdata->questroom = 0;
	/* here remove mob->hunter */
	for (fch = char_list; fch; fch = fch->next)
		if (fch->hunter == ch) {
			fch->hunter = 0;
			break;
		}
}

void quest_update(void)
{
	CHAR_DATA *ch, *ch_next;

	for (ch = char_list; ch != NULL; ch = ch_next) {
		ch_next = ch->next;

		if (IS_NPC(ch)) 
			continue;
		if (ch->pcdata->questtime < 0) {
			ch->pcdata->questtime++;

			if (ch->pcdata->questtime == 0) {
				send_to_char(msg(QUEST_YOU_MAY_NOW_QUEST_AGAIN,
						 ch), ch);
				return;
			}
		} else if (IS_QUESTOR(ch)) {
			if (--ch->pcdata->questtime <= 0) {
				send_to_char(msg(QUEST_RUN_OUT_TIME, ch), ch);
				cancel_quest(ch);
			}
			if (ch->pcdata->questtime > 0 
			    && ch->pcdata->questtime < 6) {
				send_to_char(msg(QUEST_BETTER_HURRY, ch), ch);
				return;
			}
		}
	}
	return;
}

void do_tell_quest(CHAR_DATA *ch, CHAR_DATA *victim, char *argument)
{
	char_printf(ch, msg(QUEST_QUESTOR_TELLS_YOU, ch),victim->name,argument);
	return;
}

