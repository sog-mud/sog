/*-
 * Copyright (c) 1998 fjoe <fjoe@iclub.nsu.ru>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id: act_quest.c,v 1.111 1999-06-10 20:05:28 fjoe Exp $
 */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#include "merc.h"
#include "quest.h"
#include "chquest.h"

#ifdef SUNOS
#	include <stdarg.h>
#	include "compat/compat.h"
#endif

/*
 * quest items
 */
#define QUEST_VNUM_GIRTH	94
#define QUEST_VNUM_RING		95
#define QUEST_VNUM_RUG		50
#define QUEST_VNUM_SONG		40
#define QUEST_VNUM_CANTEEN	34402

#define TROUBLE_MAX 3

static CHAR_DATA *questor_lookup(CHAR_DATA *ch);
qtrouble_t *qtrouble_lookup(CHAR_DATA *ch, int vnum);

static void quest_points(CHAR_DATA *ch, char *arg);
static void quest_info(CHAR_DATA *ch, char *arg);
static void quest_time(CHAR_DATA *ch, char *arg);
static void quest_list(CHAR_DATA *ch, char *arg);
static void quest_buy(CHAR_DATA *ch, char *arg);
static void quest_request(CHAR_DATA *ch, char *arg);
static void quest_complete(CHAR_DATA *ch, char *arg);
static void quest_trouble(CHAR_DATA *ch, char *arg);
static void quest_chquest(CHAR_DATA *ch, char *arg);

static bool quest_give_item(CHAR_DATA *ch, CHAR_DATA *questor,
			    int item_vnum, int count_max);

static bool buy_gold(CHAR_DATA *ch, CHAR_DATA *questor);
static bool buy_prac(CHAR_DATA *ch, CHAR_DATA *questor);
static bool buy_tattoo(CHAR_DATA *ch, CHAR_DATA *questor);
static bool buy_death(CHAR_DATA *ch, CHAR_DATA *questor);
static bool buy_katana(CHAR_DATA *ch, CHAR_DATA *questor);
static bool buy_vampire(CHAR_DATA *ch, CHAR_DATA *questor);

enum qitem_type {
	TYPE_ITEM,
	TYPE_OTHER
};

typedef struct qitem_t qitem_t;
struct qitem_t {
	char		*name;
	int		price;
	const char	*restrict_class;
	int		vnum;
	bool		(*do_buy)(CHAR_DATA *ch, CHAR_DATA *questor);
};

qitem_t qitem_table[] = {
	{ "small magic rug",		 750, NULL,
	   QUEST_VNUM_RUG, NULL					},

	{ "50,000 gold pieces",		 500, NULL,
	   0, buy_gold						},

	{ "60 practices",		 500, NULL,
	   0, buy_prac						},

	{ "tattoo of your religion",	 200, NULL,
	   0, buy_tattoo					},

	{ "Decrease number of deaths",	  50, "samurai",
	   0, buy_death						},

	{ "Katana quest",		 100, "samurai",
	   0, buy_katana					},

	{ "Vampire skill",		  50, "vampire",
	   0, buy_vampire					},

	{ "Bottomless canteen with cranberry juice", 350, NULL,
	   QUEST_VNUM_CANTEEN, NULL				},

	{ NULL }
};

struct qcmd_data {
	char *name;
	void (*do_fn)(CHAR_DATA *ch, char* arg);
	int min_position;
	int extra;
};
typedef struct qcmd_data qcmd_t;

qcmd_t qcmd_table[] = {
	{ "points",	quest_points,	POS_DEAD,	CMD_KEEP_HIDE	},
	{ "info",	quest_info,	POS_DEAD,	CMD_KEEP_HIDE	},
	{ "time",	quest_time,	POS_DEAD,	CMD_KEEP_HIDE	},
	{ "list",	quest_list,	POS_RESTING,	0		},
	{ "buy",	quest_buy,	POS_RESTING,	0		},
	{ "request",	quest_request,	POS_RESTING,	0		},
	{ "complete",	quest_complete,	POS_RESTING,	0		},
	{ "trouble",	quest_trouble,	POS_RESTING,	0		},
	{ "items",	quest_chquest,	POS_RESTING,	0		},
	{ NULL}
};

#define QUESTOR_TELLS_YOU(questor, ch)			\
	act_puts("$n tells you:", questor, NULL, ch,	\
		 TO_VICT, POS_DEAD);

/*
 * The main quest function
 */
void do_quest(CHAR_DATA *ch, const char *argument)
{
	char cmd[MAX_INPUT_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	qcmd_t *qcmd;

	argument = one_argument(argument, cmd, sizeof(cmd));
	argument = one_argument(argument, arg, sizeof(arg));

	if (IS_NPC(ch)) 
		return;

	for (qcmd = qcmd_table; qcmd->name != NULL; qcmd++)
		if (str_prefix(cmd, qcmd->name) == 0) {
			if (ch->position < qcmd->min_position) {
				char_puts("In your dreams, or what?\n", ch);
				return;
			}
			if (!IS_SET(qcmd->extra, CMD_KEEP_HIDE)
			&&  IS_SET(ch->affected_by, AFF_HIDE | AFF_FADE)) { 
				REMOVE_BIT(ch->affected_by,
					   AFF_HIDE | AFF_FADE);
				act_puts("You step out of shadows.",
					 ch, NULL, NULL, TO_CHAR, POS_DEAD);
				act("$n steps out of shadows.",
				    ch, NULL, NULL, TO_ROOM);
			}
			qcmd->do_fn(ch, arg);
			return;
		}
		
	char_puts("QUEST COMMANDS: points info time request complete list buy trouble.\n", ch);
	char_puts("For more information, type: help quests.\n", ch);
}

void quest_handle_death(CHAR_DATA *ch, CHAR_DATA *victim)
{
	if (IS_NPC(ch)
	&&  IS_SET(ch->pIndexData->act, ACT_SUMMONED)
	&&  ch->master != NULL)
		ch = ch->master;

	if (victim->hunter)
		if (victim->hunter == ch) {
			act_puts("You have almost completed your QUEST!\n"
				 "Return to questmaster before your time "
				 "runs out!",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
			ch->pcdata->questmob = -1;
		}
		else {
			act_puts("You have completed someone's quest.",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);

			ch = victim->hunter;
			act_puts("Someone has completed you quest.",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
			quest_cancel(ch);
			ch->pcdata->questtime = -number_range(5, 10);
		}
}

void quest_cancel(CHAR_DATA *ch)
{
	CHAR_DATA *fch;

	if (IS_NPC(ch)) {
		bug("quest_cancel: called for NPC", 0);
		return;
	}

	/*
	 * remove mob->hunter
	 */
	for (fch = npc_list; fch; fch = fch->next)
		if (fch->hunter == ch) {
			fch->hunter = NULL;
			break;
		}

	ch->pcdata->questtime = 0;
	ch->pcdata->questgiver = 0;
	ch->pcdata->questmob = 0;
	ch->pcdata->questobj = 0;
	ch->pcdata->questroom = NULL;
}

/*
 * Called from update_handler() by pulse_area
 */
void quest_update(void)
{
	CHAR_DATA *ch, *ch_next;

	for (ch = char_list; ch && !IS_NPC(ch); ch = ch_next) {
		ch_next = ch->next;

		if (ch->pcdata->questtime < 0) {
			if (++ch->pcdata->questtime == 0) {
				char_puts("{*You may now quest again.\n", ch);
				return;
			}
		} else if (IS_ON_QUEST(ch)) {
			if (--ch->pcdata->questtime == 0) {
				char_puts("You have run out of time for your quest!\n", ch);
				quest_cancel(ch);
				ch->pcdata->questtime = -number_range(5, 10);
			} else if (ch->pcdata->questtime < 6) {
				char_puts("Better hurry, you're almost out of time for your quest!\n", ch);
				return;
			}
		}
	}
}

void qtrouble_set(CHAR_DATA *ch, int vnum, int count)
{
	qtrouble_t *qt;

	if ((qt = qtrouble_lookup(ch, vnum)) != NULL)
		qt->count = count;
	else {
		qt = malloc(sizeof(*qt));
		qt->vnum = vnum;
		qt->count = count;
		qt->next = ch->pcdata->qtrouble;
		ch->pcdata->qtrouble = qt;
	}
}

/*
 * local functions
 */

static CHAR_DATA* questor_lookup(CHAR_DATA *ch)
{
	CHAR_DATA *vch;
	CHAR_DATA *questor = NULL;

	for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room) {
		if (!IS_NPC(vch)) 
			continue;
		if (IS_SET(vch->pIndexData->act, ACT_QUESTOR)) {
			questor = vch;
			break;
		}
	}

	if (questor == NULL) {
		act_puts("You can't do that here.",
			 questor, NULL, ch, TO_VICT, POS_DEAD);
		return NULL;
	}

	if (questor->fighting != NULL) {
		act_puts("Wait until the fighting stops.",
			 questor, NULL, ch, TO_VICT, POS_DEAD);
		return NULL;
	}

	if (questor->position < POS_RESTING)
		return NULL;

	if (!can_see(questor, ch)) {
		act_puts("I do not give any quests to folks I can't see.",
			 questor, NULL, ch, TO_VICT, POS_DEAD);
		return NULL;
	}
	return questor;
}

qtrouble_t *qtrouble_lookup(CHAR_DATA *ch, int vnum)
{
	qtrouble_t *qt;

	for (qt = ch->pcdata->qtrouble; qt != NULL; qt = qt->next)
		if (qt->vnum == vnum)
			return qt;

	return NULL;
}

/*
 * quest do functions
 */

static void quest_points(CHAR_DATA *ch, char* arg)
{
	char_printf(ch, "You have {W%d{x quest points.\n",
		    ch->pcdata->questpoints);
}

static void quest_info(CHAR_DATA *ch, char* arg)
{
	if (!IS_ON_QUEST(ch)) {
		char_puts("You aren't currently on a quest.\n", ch);
		return;
	}

	if (ch->pcdata->questmob == -1) {
		char_puts("Your quest is ALMOST complete!\nGet back to questor before your time runs out!\n", ch);
		return;
	}

	if (ch->pcdata->questobj > 0) {
		OBJ_INDEX_DATA *qinfoobj;

		qinfoobj = get_obj_index(ch->pcdata->questobj);
		if (qinfoobj != NULL) {
			char_printf(ch, "You are on a quest to recover the fabled {W%s{x!\n",
				    qinfoobj->name);
			if (ch->pcdata->questroom)
				char_printf(ch, "That location is in general area of {W%s{x for {W%s{x.\n",
					    ch->pcdata->questroom->area->name, 
					    mlstr_mval(&ch->pcdata->questroom->name));
		}
		else 
			char_puts("You aren't currently on a quest.\n", ch);
		return;
	}

	if (ch->pcdata->questmob > 0) {
		MOB_INDEX_DATA *questinfo;

		questinfo = get_mob_index(ch->pcdata->questmob);
		if (questinfo != NULL) {
			char_printf(ch, "You are on a quest to slay the dreaded {W%s{x!\n",
				    mlstr_mval(&questinfo->short_descr));
			if (ch->pcdata->questroom)
				char_printf(ch, "That location is in general area of {W%s{x for {W%s{x.\n",
					    ch->pcdata->questroom->area->name, 
					    mlstr_mval(&ch->pcdata->questroom->name));
		} else 
			char_puts("You aren't currently on a quest.\n", ch);
		return;
	}
}

static void quest_time(CHAR_DATA *ch, char* arg)
{
	if (!IS_ON_QUEST(ch)) {
		char_puts("You aren't currently on a quest.\n", ch);
		if (ch->pcdata->questtime < -1)
			char_printf(ch, "There are {W%d{x minutes remaining until you can go on another quest.\n",
				    -ch->pcdata->questtime);
	    	else if (ch->pcdata->questtime == -1)
			char_puts("There is less than a minute remaining until you can go on another quest.\n", ch);
	}
	else
		char_printf(ch, "Time left for current quest: {W%d{x.\n",
			    ch->pcdata->questtime);
}

static void quest_list(CHAR_DATA *ch, char *arg)
{
	CHAR_DATA *questor;
	qitem_t *qitem;
	class_t *cl;

	if ((questor = questor_lookup(ch)) == NULL
	||  (cl = class_lookup(ch->class)) == NULL)
		return;

	act("$n asks $N for list of quest items.", ch, NULL, questor, TO_ROOM);
	act_puts("You ask $N for list of quest items.",
		 ch, NULL, questor, TO_CHAR, POS_DEAD);

	char_puts("Current Quest Items available for Purchase:\n", ch);
	for (qitem = qitem_table; qitem->name; qitem++) {
		if (qitem->restrict_class != NULL
		&&  !is_name(cl->name, qitem->restrict_class))
			continue;

		if (arg[0] != '\0' && !is_name(arg, qitem->name))
			continue;

		char_printf(ch, "%5dqp...........%s\n",
			    qitem->price, qitem->name);
	}
	char_puts("To buy an item, type 'QUEST BUY <item>'.\n", ch);
}

static void quest_buy(CHAR_DATA *ch, char *arg)
{
	CHAR_DATA *questor;
	qitem_t *qitem;
	class_t *cl;

	if ((questor = questor_lookup(ch)) == NULL
	||  (cl = class_lookup(ch->class)) == NULL)
		return;

	if (arg[0] == '\0') {
		char_puts("To buy an item, type 'QUEST BUY <item>'.\n", ch);
		return;
	}

	for (qitem = qitem_table; qitem->name; qitem++)
		if (is_name(arg, qitem->name)) {
			bool buy_ok = FALSE;

			if (qitem->restrict_class != NULL
			&&  !is_name(cl->name, qitem->restrict_class))
				continue;

			if (ch->pcdata->questpoints < qitem->price) {
				QUESTOR_TELLS_YOU(questor, ch);
				act_puts("Sorry, $N, but you don't have "
					 "enough quest points for that.",
					 questor, NULL, ch, TO_VICT, POS_DEAD);
				return;
			}

			if (qitem->vnum == 0)
				buy_ok = qitem->do_buy(ch, questor);
			else
				buy_ok = quest_give_item(ch, questor,
						qitem->vnum, 0);

			if (buy_ok) 
				ch->pcdata->questpoints -= qitem->price;
			return;
		}

	QUESTOR_TELLS_YOU(questor, ch);
	act_puts("I do not have that item, $N.",
		 questor, NULL, ch, TO_VICT, POS_DEAD);
}

#define MAX_QMOB_COUNT 512

static void quest_request(CHAR_DATA *ch, char *arg)
{
	int i;
	CHAR_DATA *mobs[MAX_QMOB_COUNT];
	size_t mob_count;
	CHAR_DATA *victim = NULL;
	CHAR_DATA *questor;

	if ((questor = questor_lookup(ch)) == NULL)
		return;

	act("$n asks $N for a quest.", ch, NULL, questor, TO_ROOM);
	act_puts("You ask $N for a quest.",
		 ch, NULL, questor, TO_CHAR, POS_DEAD);
	QUESTOR_TELLS_YOU(questor, ch);

	if (IS_ON_QUEST(ch)) {
		act_puts("But you are already on a quest!",
			 questor, NULL, ch, TO_VICT, POS_DEAD);
    		return;
	} 

	if (ch->pcdata->questtime < 0) {
		act_puts("You're very brave, $N, but let someone else "
			 "have a chance.",
			 questor, NULL, ch, TO_VICT, POS_DEAD);
		act_puts("Come back later.",
			 questor, NULL, ch, TO_VICT, POS_DEAD);
		return;
	}

	act_puts("Thank you, brave $N!", questor, NULL, ch, TO_VICT, POS_DEAD);

	/*
	 * find MAX_QMOB_COUNT quest mobs and store their vnums in mob_buf
	 */
	mob_count = 0;
	for (victim = npc_list; victim; victim = victim->next) {
		int diff = victim->level - ch->level;

		if (!IS_NPC(victim)
		||  (ch->level < 51 && (diff > 4 || diff < -1))
		||  (ch->level > 50 && (diff > 6 || diff < 0))
		||  victim->pIndexData->pShop
		||  victim->race == ch->race
		||  victim->invis_level
		||  victim->incog_level
		||  (IS_EVIL(victim) && IS_EVIL(ch))
		||  (IS_GOOD(victim) && IS_GOOD(ch))
		||  victim->pIndexData->vnum < 100
		||  IS_SET(victim->pIndexData->act,
			   ACT_TRAIN | ACT_PRACTICE | ACT_HEALER |
			   ACT_NOTRACK | ACT_PET)
		||  IS_SET(victim->pIndexData->imm_flags, IMM_SUMMON)
		||  questor->pIndexData == victim->pIndexData
		||  victim->in_room == NULL
		||  (IS_SET(victim->pIndexData->act, ACT_SENTINEL) &&
		     IS_SET(victim->in_room->room_flags,
			    ROOM_PRIVATE | ROOM_SOLITARY))
		||  !str_cmp(victim->in_room->area->name,
			     hometown_name(ch->hometown))
		||  IS_SET(victim->in_room->area->area_flags,
			   AREA_CLOSED | AREA_NOQUEST))
			continue;
		mobs[mob_count++] = victim;
		if (mob_count >= MAX_QMOB_COUNT)
			break;
	}

	if (mob_count == 0) {
		act_puts("I'm sorry, but i don't have any quests for you "
			 "at this time.", questor, NULL, ch, TO_VICT, POS_DEAD);
		ch->pcdata->questtime = -5;
		return;
	}

	victim = mobs[number_range(0, mob_count-1)];
	ch->pcdata->questroom = victim->in_room;

	if (chance(40)) { /* Quest to find an obj */
		OBJ_DATA *eyed;
		int obj_vnum;

		if (IS_GOOD(ch))
			i = 0;
		else if (IS_EVIL(ch))
			i = 2;
		else
			i = 1;

		obj_vnum = number_range(QUEST_OBJ_FIRST, QUEST_OBJ_LAST);
		eyed = create_obj(get_obj_index(obj_vnum), 0);
		eyed->level = ch->level;
		mlstr_cpy(&eyed->owner, &ch->short_descr);
		eyed->ed = ed_new2(eyed->pIndexData->ed, ch->name);
		eyed->cost = 0;
		eyed->timer = 30;

		obj_to_room(eyed, victim->in_room);
		ch->pcdata->questobj = eyed->pIndexData->vnum;

		act_puts("Vile pilferers have stolen {W$p{x "
			 "from the royal treasury!",
			 questor, eyed, ch, TO_VICT | ACT_FORMSH, POS_DEAD);
		act_puts("My court wizardess, with her magic mirror, "
			 "has pinpointed its location.",
			 questor, NULL, ch, TO_VICT, POS_DEAD);
		act_puts3("Look in the general area of {W$t{x for {W$R{x!",
			  questor, victim->in_room->area->name, ch,
			  victim->in_room, TO_VICT, POS_DEAD);
	}
	else {	/* Quest to kill a mob */
		if (IS_GOOD(ch)) {
			act_puts("Rune's most heinous criminal, {W$i{x, "
				 "has escaped from the dungeon.",
				 questor, victim, ch,
				 TO_VICT | ACT_FORMSH, POS_DEAD);
			act_puts3("Since the escape, $i has murdered "
				  "$J $qJ{civilians}!",
				  questor, victim, ch,
				  (const void*) number_range(2, 20),
				  TO_VICT, POS_DEAD);
			act_puts("The penalty for this crime is death, "
				 "and you are to deliver the sentence!",
				 questor, NULL, ch, TO_VICT, POS_DEAD);
		}
		else {
			act_puts("An enemy of mine, {W$i{x, is making "
				 "vile threats against the crown.",
				 questor, victim, ch,
				 TO_VICT | ACT_FORMSH, POS_DEAD);
			act_puts("This threat must be eliminated!",
				 questor, NULL, ch, TO_VICT, POS_DEAD);
		}

		act_puts3("Seek $i out in the vicinity of {W$R{x!",
			  questor, victim, ch, victim->in_room,
			  TO_VICT, POS_DEAD);
		act_puts("That location is in general area of {W$t{x.",
			 questor, victim->in_room->area->name, ch,
			 TO_VICT, POS_DEAD);

		ch->pcdata->questmob = victim->pIndexData->vnum;
		victim->hunter = ch;
	}

	ch->pcdata->questgiver = questor->pIndexData->vnum;
	ch->pcdata->questtime = number_range(10, 20) + ch->level/10;
	act_puts("You have {W$j{x $qj{minutes} to complete this quest.", 
		 questor, (const void*) ch->pcdata->questtime, ch,
		 TO_VICT, POS_DEAD);
	act_puts("May the gods go with you!",
		 questor, NULL, ch, TO_VICT, POS_DEAD);
}

static void quest_complete(CHAR_DATA *ch, char *arg)
{
	bool complete = FALSE;
	CHAR_DATA *questor;
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;

	int gold_reward = 0;
	int qp_reward = 0;
	int prac_reward = 0;

	if ((questor = questor_lookup(ch)) == NULL)
		return;

	act("$n informs $N $e has completed $s quest.",
	    ch, NULL, questor, TO_ROOM);
	act_puts("You inform $N you have completed your quest.",
		 ch, NULL, questor, TO_CHAR, POS_DEAD);

	if (!IS_ON_QUEST(ch)) {
		QUESTOR_TELLS_YOU(questor, ch);
		act_puts("You have to REQUEST a quest first, $N.",
			 questor, NULL, ch, TO_VICT, POS_DEAD);
		return;
	}

	if (ch->pcdata->questgiver != questor->pIndexData->vnum) {
		QUESTOR_TELLS_YOU(questor, ch);
		act_puts("I never sent you on a quest! Perhaps you're "
			 "thinking of someone else.",
			 questor, NULL, ch, TO_VICT, POS_DEAD);
		return;
	}

	if (ch->pcdata->questobj > 0)
		for (obj = ch->carrying; obj; obj = obj_next) {
			obj_next = obj->next_content;

			if (obj->pIndexData->vnum == ch->pcdata->questobj
			&&  IS_OWNER(ch, obj)) {
				act_puts("You hand {W$p{x to $N.",
					 ch, obj, questor, TO_CHAR, POS_DEAD);
				act("$n hands {W$p{x to $N.",
				    ch, obj, questor, TO_ROOM);
				extract_obj(obj, 0);

				if (chance(15))
					prac_reward = number_range(1, 6);
				qp_reward = number_range(20, 40);
				gold_reward = 35 + number_range(ch->level, 2*ch->level);

				complete = TRUE;
				break;
			}
		}
	else if (ch->pcdata->questmob == -1) {
		if (chance(2))
			prac_reward = number_range(1, 6);
		qp_reward = number_range(15, 35);
		gold_reward = dice(ch->level/10 + 1, 30);
		gold_reward = URANGE(0, gold_reward, 20*ch->level);
		complete = TRUE;
	}

	QUESTOR_TELLS_YOU(questor, ch);

	if (!complete) {
		act_puts("You haven't completed the quest yet, but there is "
			 "still time!",
			 questor, NULL, ch, TO_VICT, POS_DEAD);
		return;
	}

	ch->gold += gold_reward;
	ch->pcdata->questpoints += qp_reward;

	act_puts("Congratulations on completing your quest!",
		 questor, NULL, ch, TO_VICT, POS_DEAD);
	act_puts3("As a reward, I am giving you {W$j{x $qj{quest points} "
		  "and {W$J{x $qJ{gold}.",
		  questor, (const void*) qp_reward,
		  ch, (const void*) gold_reward,
		  TO_VICT, POS_DEAD);

	if (prac_reward) {
		ch->practice += prac_reward;
		act_puts("You gain {W$j{x $qj{practices}!",
			 questor, (const void*) prac_reward, ch,
			 TO_VICT, POS_DEAD);
	}

	quest_cancel(ch);
	ch->pcdata->questtime = -number_range(8, 12);
}

static void quest_trouble(CHAR_DATA *ch, char *arg)
{
	CHAR_DATA *questor;
	qitem_t *qitem;
	class_t *cl;

	if ((questor = questor_lookup(ch)) == NULL
	||  (cl = class_lookup(ch->class)) == NULL)
		return;

	if (arg[0] == '\0') {
		char_puts("To resolve a quest award's trouble, type: 'quest trouble award'.\n", ch);
		return;
	}

	for (qitem = qitem_table; qitem->name; qitem++) {
		if (qitem->restrict_class != NULL
		&&  !is_name(cl->name, qitem->restrict_class))
			continue;

		if (qitem->vnum && is_name(arg, qitem->name)) {
			quest_give_item(ch, questor, qitem->vnum, TROUBLE_MAX);
			return;
		}
	}

	act_puts("Sorry, $N, but you haven't bought that quest award, yet.",
		 questor, NULL, ch, TO_VICT, POS_DEAD);
}

static void quest_chquest(CHAR_DATA *ch, char *arg)
{
	CHAR_DATA *questor;
	bool found;
	chquest_t *q;

	if ((questor = questor_lookup(ch)) == NULL)
		return;

	act("$n asks $N about current challenge quests.",
	    ch, NULL, questor, TO_ROOM);
	act_puts("You ask $N about current challenge quests.",
		 ch, NULL, questor, TO_CHAR, POS_DEAD);
	QUESTOR_TELLS_YOU(questor, ch);

	found = FALSE;
	for (q = chquest_list; q; q = q->next) {
		CHAR_DATA *carried_by;

		if (!IS_RUNNING(q))
			continue;

		if (!found) {
			found = TRUE;
			act("Current challenge quest items available:", 
			    ch, NULL, NULL, TO_CHAR);
		}

		if ((carried_by = chquest_carried_by(q->obj)))
			act("- $p (carried by $N)",
			    ch, q->obj, carried_by, TO_CHAR | ACT_FORMSH);
		else
			act("- $p (somewhere)",
			    ch, q->obj, NULL, TO_CHAR | ACT_FORMSH);
	}

	if (!found) {
		act("No challenge quests are running.",
		    ch, NULL, NULL, TO_CHAR);
		return;
	}
}

/*
 * quest buy functions
 */

static bool quest_give_item(CHAR_DATA *ch, CHAR_DATA *questor,
			    int item_vnum, int count_max)
{
	OBJ_DATA *reward;
	qtrouble_t *qt;
	OBJ_INDEX_DATA *pObjIndex = get_obj_index(item_vnum);

	QUESTOR_TELLS_YOU(questor, ch);

	/*
	 * check quest trouble data
	 */
	qt = qtrouble_lookup(ch, item_vnum);

	if (count_max) {
		/*
		 * 'quest trouble'
		 */

		if ((qt && qt->count > count_max)
		||  !IS_SET(pObjIndex->extra_flags, ITEM_QUEST)) {

			/* ch requested this item too many times	*
			 * or the item is not quest			*/

			act_puts("This item is beyond the trouble option.",
				 questor, NULL, ch, TO_VICT, POS_DEAD);
			return FALSE;
		}
		else if (!qt) {
			/* ch has never bought this item, but requested it */
			act_puts("Sorry, $N, but you haven't bought "
				 "that quest award, yet.",
				 questor, NULL, ch, TO_VICT, POS_DEAD);
			return FALSE;
		}
	}
	else {
		/*
		 * 'quest buy'
		 */

		if (qt && qt->count <= TROUBLE_MAX) {
			act_puts("You have already bought this item.",
				 questor, NULL, ch, TO_VICT, POS_DEAD);
			return FALSE;
		}
	}

	reward = create_obj(pObjIndex, 0);
	if (get_wear_level(ch, reward) < reward->level) {
		act_puts("$p is too powerful for you.",
			 questor, reward, ch, TO_VICT, POS_DEAD);
		extract_obj(reward, 0);
		return FALSE;
	}

	/* update quest trouble data */

	if (qt && count_max) {
		OBJ_DATA *obj;
		OBJ_DATA *obj_next;

		/* `quest trouble' */
		for (obj = object_list; obj != NULL; obj = obj_next) {
			obj_next = obj->next;
			if (obj->pIndexData->vnum == item_vnum 
			&&  IS_OWNER(ch, obj)) {
				extract_obj(obj, 0);
				break;
			}
		}

		act_puts("This is the $j$qj{th} time that I am giving "
			 "that award back.",
			 questor, (const void*) qt->count, ch,
			 TO_VICT, POS_DEAD);
		if (qt->count > count_max) 
			act_puts("And I won't give you that again, "
				 "with trouble option.",
				 questor, NULL, ch, TO_VICT, POS_DEAD);
	}

	if (!qt && IS_SET(pObjIndex->extra_flags, ITEM_QUEST)) {
		qt = malloc(sizeof(*qt));
		qt->vnum = item_vnum;
		qt->count = 0;
		qt->next = ch->pcdata->qtrouble;
		ch->pcdata->qtrouble = qt;
	}

	if (qt) {
		if (count_max)
			qt->count++;
		else
			qt->count = 1;
	}

	/* ok, give him requested item */

	if (IS_SET(pObjIndex->extra_flags, ITEM_QUEST)) {
		mlstr_cpy(&reward->owner, &ch->short_descr);
		mlstr_printf(&reward->short_descr,
			     &reward->pIndexData->short_descr,
			     IS_GOOD(ch) ?	"holy" :
			     IS_NEUTRAL(ch) ?	"blue-green" : 
						"evil", 
			     ch->name);
	}

	obj_to_char(reward, ch);

	act("$N gives {W$p{x to $n.", ch, reward, questor, TO_ROOM);
	act_puts("$N gives you {W$p{x.",
		 ch, reward, questor, TO_CHAR, POS_DEAD);

	return TRUE;
}

static bool buy_gold(CHAR_DATA *ch, CHAR_DATA *questor)
{
	ch->pcdata->bank_g += 50000;
	act("$N gives 50,000 gold pieces to $n.", ch, NULL, questor, TO_ROOM);
	act("$N transfers 50,000 gold pieces to your bank account.",ch, NULL, questor, TO_CHAR);
	return TRUE;
}

static bool buy_prac(CHAR_DATA *ch, CHAR_DATA *questor)
{
	ch->practice += 60;
	act("$N gives 60 practices to $n.", ch, NULL, questor, TO_ROOM);
	act_puts("$N gives you 60 practices.",
		 ch, NULL, questor, TO_CHAR, POS_DEAD);
	return TRUE;
}

static bool buy_tattoo(CHAR_DATA *ch, CHAR_DATA *questor)
{
	OBJ_DATA *tattoo;

	if (!ch->religion) {
		char_puts("You don't have a religion to have a tattoo.\n", ch);
		return FALSE;
	}

	tattoo = get_eq_char(ch, WEAR_TATTOO);
	if (tattoo != NULL) {
		char_puts("But you have already your tattoo!\n", ch);
		return FALSE;
	}

	tattoo = create_obj(get_obj_index(religion_table[ch->religion].vnum), 0);

	obj_to_char(tattoo, ch);
	equip_char(ch, tattoo, WEAR_TATTOO);
	act("$N tattoos $n with {W$p{x!", ch, tattoo, questor, TO_ROOM);
	act_puts("$N tattoos you with {W$p{x!",
		 ch, tattoo, questor, TO_CHAR, POS_DEAD);
	return TRUE;
}

static bool buy_death(CHAR_DATA *ch, CHAR_DATA *questor)
{
	if (ch->pcdata->death < 1) {
		QUESTOR_TELLS_YOU(questor, ch);
		act_puts("Sorry, $N, but you haven't got any deaths yet.",
			 questor, NULL, ch, TO_VICT, POS_DEAD);
		return FALSE;
	}

	ch->pcdata->death -= 1;
	return TRUE;
}

static bool buy_katana(CHAR_DATA *ch, CHAR_DATA *questor)
{
	AFFECT_DATA af;
	OBJ_DATA *katana;

	QUESTOR_TELLS_YOU(questor, ch);

	if ((katana = get_obj_list(ch, "katana", ch->carrying)) == NULL) {
		act_puts("Sorry, $N, but you don't have your katana with you.",
			 questor, NULL, ch, TO_VICT, POS_DEAD);
		return FALSE;
	}

	af.where	= TO_WEAPON;
	af.type 	= gsn_katana;
	af.level	= 100;
	af.duration	= -1;
	af.modifier	= 0;
	af.bitvector	= WEAPON_KATANA;
	af.location	= APPLY_NONE;
	affect_to_obj(katana, &af);
	act_puts("As you wield it, you will feel that its power will increase "
		 "continuosly.", questor, NULL, ch, TO_VICT, POS_DEAD);
	return TRUE;
}

static bool buy_vampire(CHAR_DATA *ch, CHAR_DATA *questor)
{
	set_skill(ch, gsn_vampire, 100);
	act("$N gives secret of undead to $n.", ch, NULL, questor, TO_ROOM);
	act_puts("$N gives you SECRET of undead.",
		 ch, NULL, questor, TO_CHAR, POS_DEAD);
	act("Lightning flashes in the sky.", ch, NULL, NULL, TO_ALL);
	return TRUE;
}

