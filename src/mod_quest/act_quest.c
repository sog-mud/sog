/*-
 * Copyright (c) 1998 SoG Development Team
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
 * $Id: act_quest.c,v 1.142 2000-10-04 20:28:50 fjoe Exp $
 */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "merc.h"
#include "auction.h"

#include "_quest.h"
#include "_chquest.h"

#ifdef SUNOS
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

static void quest_points(CHAR_DATA *ch, char *arg);
static void quest_info(CHAR_DATA *ch, char *arg);
static void quest_time(CHAR_DATA *ch, char *arg);
static void quest_list(CHAR_DATA *ch, char *arg);
static void quest_buy(CHAR_DATA *ch, char *arg);
static void quest_request(CHAR_DATA *ch, char *arg);
static void quest_complete(CHAR_DATA *ch, char *arg);
static void quest_trouble(CHAR_DATA *ch, char *arg);
static void quest_chquest(CHAR_DATA *ch, char *arg);
static void quest_cancel_cmd(CHAR_DATA *ch, char *arg);

static bool quest_give_item(CHAR_DATA *ch, CHAR_DATA *questor,
			    int item_vnum, int count_max);

static bool buy_gold(CHAR_DATA *ch, CHAR_DATA *questor);

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
	{ "cancel", 	quest_cancel_cmd,POS_RESTING,	0		},
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
				act_char("In your dreams, or what?", ch);
				return;
			}
			if (!IS_SET(qcmd->extra, CMD_KEEP_HIDE)
			&&  HAS_INVIS(ch, ID_HIDDEN | ID_FADE)) { 
				REMOVE_INVIS(ch, ID_HIDDEN | ID_FADE);
				act_puts("You step out of shadows.",
					 ch, NULL, NULL, TO_CHAR, POS_DEAD);
				act("$n steps out of shadows.",
				    ch, NULL, NULL, TO_ROOM);
			}
			qcmd->do_fn(ch, arg);
			return;
		}
		
	char_puts("QUEST COMMANDS:", ch);
	for (qcmd = qcmd_table; qcmd->name != NULL; qcmd++) {
		char_printf(ch, " %s", qcmd->name);
	}
	send_to_char("\n", ch);
	act_char("For more information, type: help quests.", ch);
}

static inline void chquest_status(CHAR_DATA *ch)
{
	chquest_t *q;

	act_char("Challenge quest items:", ch);
	for (q = chquest_list; q; q = q->next) {
		OBJ_DATA *obj;

		char_printf(ch, "- %s (vnum %d) - ",
			    mlstr_mval(&q->obj_index->short_descr),
			    q->obj_index->vnum);

		if (IS_STOPPED(q)) {
			act_char("stopped.", ch);
			continue;
		} else if (IS_WAITING(q)) {
			char_printf(ch, "%d area ticks to start.\n",
				    q->delay);
			continue;
		}

		if ((obj = q->obj) == NULL) {
			act_char("status unknown.", ch);
			continue;
		}

		char_printf(ch, "running (%d ticks left).\n",
			    q->obj->timer);

		while (obj->in_obj)
			obj = obj->in_obj;

		if (obj->carried_by) {
			act_puts3("        $r (vnum $J), carried by $N.",
				  ch, obj->carried_by->in_room, obj->carried_by,
				  (const void*) obj->carried_by->in_room->vnum,
				  TO_CHAR, POS_DEAD);
		} else if (obj->in_room) {
			act_puts3("         $r (vnum $J).",
				  ch, obj->in_room, NULL,
				  (const void*) obj->in_room->vnum,
				  TO_CHAR, POS_DEAD);
		}
	}
}

void do_chquest(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		dofun("help", ch, "'WIZ CHQUEST'");
		return;
	}

	if (!str_prefix(arg, "restart")) {
		chquest_start(CHQUEST_F_NODELAY);
		act_char("Challenge quest restarted.", ch);
		return;
	}

	if (!str_prefix(arg, "status")) {
		chquest_status(ch);
		return;
	}

	if (!str_prefix(arg, "start")
	||  !str_prefix(arg, "stop")
	||  !str_prefix(arg, "add")
	||  !str_prefix(arg, "delete")) {
		char arg2[MAX_INPUT_LENGTH];
		OBJ_INDEX_DATA *obj_index;
		chquest_t *q;

		one_argument(argument, arg2, sizeof(arg2));
		if (!is_number(arg2)) {
			do_chquest(ch, str_empty);
			return;
		}

		if ((obj_index = get_obj_index(atoi(arg2))) == NULL) {
			char_printf(ch, "do_chquest: %s: no object with that vnum.\n", arg2);
			return;
		}

		if (!str_prefix(arg, "delete")) {
			chquest_delete(ch, obj_index);
			return;
		}

		if (!str_prefix(arg, "add")) {
			chquest_add(obj_index);
			return;
		}

		if ((q = chquest_lookup(obj_index)) == NULL) {
			char_printf(ch, "do_chquest: %s: no chquests with that vnum.\n", arg2);
			return;
		}

		if (!str_prefix(arg, "start")) {
			if (IS_RUNNING(q)) {
				char_printf(ch, "do_chquest: quest vnum %d "
						"already running.\n",
						q->obj_index->vnum);
				return;
			}
			chquest_startq(q);
			return;
		}

		if (IS_RUNNING(q) && IS_AUCTIONED(q->obj)) {
			act("$p is on auction right now.",
			    ch, q->obj, NULL, TO_CHAR);
			return;
		}

		chquest_stopq(q);
		return;
	}

	do_chquest(ch, str_empty);
}

/*
 * local functions
 */

static CHAR_DATA* questor_lookup(CHAR_DATA *ch)
{
	CHAR_DATA *vch;
	CHAR_DATA *questor = NULL;

	for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room) {
		if (IS_NPC(vch) && MOB_IS(vch, MOB_QUESTOR)) {
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

/*
 * quest do functions
 */

static void quest_points(CHAR_DATA *ch, char* arg)
{
	act("You have {W$j{x $qj{quest points}.",
	    ch, (const void*) PC(ch)->questpoints, NULL, TO_CHAR);
}

static void quest_info(CHAR_DATA *ch, char* arg)
{
	if (!IS_ON_QUEST(ch)) {
		act_char("You aren't currently on a quest.", ch);
		return;
	}

	if (PC(ch)->questmob == -1) {
		act_char("Your quest is ALMOST complete!", ch);
		act_char("Get back to questor before your time runs out!", ch);
		return;
	}

	if (PC(ch)->questobj > 0) {
		OBJ_INDEX_DATA *qinfoobj;

		qinfoobj = get_obj_index(PC(ch)->questobj);
		if (qinfoobj != NULL) {
			OBJ_DATA *obj = create_obj(qinfoobj, 0);
			ROOM_INDEX_DATA *qroom;

			act("You are on a quest to recover the fabled {W$p{x!",
			    ch, obj, NULL, TO_CHAR | ACT_FORMSH);
			extract_obj(obj, 0);

			if (PC(ch)->qroom_vnum
			&&  (qroom = get_room_index(PC(ch)->qroom_vnum))) {
				act("That location is in general area of "
				    "{W$T{x for {W$r{x.",
				    ch, qroom, qroom->area->name,
				    TO_CHAR);
			}
		} else 
			act_char("You aren't currently on a quest.", ch);
		return;
	}

	if (PC(ch)->questmob > 0) {
		MOB_INDEX_DATA *questinfo;

		questinfo = get_mob_index(PC(ch)->questmob);
		if (questinfo != NULL) {
			CHAR_DATA *mob = create_mob(questinfo);
			ROOM_INDEX_DATA *qroom;

			act("You are on a quest to slay the dreaded {W$N{x!",
			    ch, NULL, mob, TO_CHAR | ACT_FORMSH);
			extract_char(mob, 0);

			if (PC(ch)->qroom_vnum
			&&  (qroom = get_room_index(PC(ch)->qroom_vnum))) {
				act("That location is in general area of "
				    "{W$T{x for {W$r{x.",
				    ch, qroom, qroom->area->name,
				    TO_CHAR);
			}
		} else 
			act_char("You aren't currently on a quest.", ch);
		return;
	}
}

static void quest_time(CHAR_DATA *ch, char* arg)
{
	if (!IS_ON_QUEST(ch)) {
		act_char("You aren't currently on a quest.", ch);
		if (PC(ch)->questtime < -1) {
			act("There are {W$j{x $qj{minutes} remaining until "
			    "you can go on another quest.",
			    ch, (const void*) -PC(ch)->questtime, NULL,
			    TO_CHAR);
	    	} else if (PC(ch)->questtime == -1) {
			act_char("There is less than a minute remaining until you can go on another quest.", ch);
		}
	}
	else {
		act("Time left for current quest: {W$j{x $qj{minutes}.",
		    ch, (const void*) PC(ch)->questtime, NULL,
		    TO_CHAR);
	}
}

static void quest_list(CHAR_DATA *ch, char *arg)
{
	CHAR_DATA *questor;
	qitem_t *qitem;

	if ((questor = questor_lookup(ch)) == NULL)
		return;

	act("$n asks $N for list of quest items.", ch, NULL, questor, TO_ROOM);
	act_puts("You ask $N for list of quest items.",
		 ch, NULL, questor, TO_CHAR, POS_DEAD);

	act_char("Current Quest Items available for Purchase:", ch);
	for (qitem = qitem_table; qitem->name; qitem++) {
		if (qitem->restrict_class != NULL
		&&  !is_name(ch->class, qitem->restrict_class))
			continue;

		if (arg[0] != '\0' && !is_name(arg, qitem->name))
			continue;

		char_printf(ch, "%5dqp...........%s\n",
			    qitem->price, qitem->name);
	}
	act_char("To buy an item, type 'QUEST BUY <item>'.", ch);
}

static void quest_buy(CHAR_DATA *ch, char *arg)
{
	CHAR_DATA *questor;
	qitem_t *qitem;

	if ((questor = questor_lookup(ch)) == NULL)
		return;

	if (arg[0] == '\0') {
		act_char("To buy an item, type 'QUEST BUY <item>'.", ch);
		return;
	}

	for (qitem = qitem_table; qitem->name; qitem++)
		if (is_name(arg, qitem->name)) {
			bool buy_ok = FALSE;

			if (qitem->restrict_class != NULL
			&&  !is_name(ch->class, qitem->restrict_class))
				continue;

			if (PC(ch)->questpoints < qitem->price) {
				QUESTOR_TELLS_YOU(questor, ch);
				act_puts("    Sorry, $N, but you don't have "
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
				PC(ch)->questpoints -= qitem->price;
			return;
		}

	QUESTOR_TELLS_YOU(questor, ch);
	act_puts("    I do not have that item, $N.",
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
		act_puts("    But you are already on a quest!",
			 questor, NULL, ch, TO_VICT, POS_DEAD);
    		return;
	} 

	if (PC(ch)->questtime < 0) {
		act_puts("    You're very brave, $N, but let someone else "
			 "have a chance.",
			 questor, NULL, ch, TO_VICT, POS_DEAD);
		act_puts("    Come back later.",
			 questor, NULL, ch, TO_VICT, POS_DEAD);
		return;
	}

	act_puts("    Thank you, brave $N!",
		 questor, NULL, ch, TO_VICT, POS_DEAD);

	/*
	 * find MAX_QMOB_COUNT quest mobs and store their vnums in mob_buf
	 */
	mob_count = 0;
	for (victim = npc_list; victim; victim = victim->next) {
		int diff = victim->level - ch->level;

		if (!IS_NPC(victim)
		||  (ch->level <= MAX_LEVEL / 2 && (diff > 4 || diff < -1))
		||  (ch->level > MAX_LEVEL / 2 && (diff > 6 || diff < 0))
		||  victim->pMobIndex->pShop
		||  IS_RACE(victim->race, ch->race)
		||  victim->invis_level
		||  victim->incog_level
		||  (IS_EVIL(victim) && IS_EVIL(ch))
		||  (IS_GOOD(victim) && IS_GOOD(ch))
		||  victim->pMobIndex->vnum < 100
		||  MOB_IS(victim, MOB_TRAIN | MOB_PRACTICE | MOB_HEALER)
		||  IS_SET(victim->pMobIndex->act,
			   ACT_NOTRACK | ACT_IMMSUMMON | ACT_PET)
		||  questor->pMobIndex == victim->pMobIndex
		||  victim->in_room == NULL
		||  victim->in_room->sector_type == SECT_UNDERWATER
		||  (IS_SET(victim->pMobIndex->act, ACT_SENTINEL) &&
		     IS_SET(victim->in_room->room_flags,
			    ROOM_PRIVATE | ROOM_SOLITARY))
		||  !str_cmp(victim->in_room->area->name,
			     hometown_name(PC(ch)->hometown))
		||  IS_SET(victim->in_room->area->area_flags,
			   AREA_CLOSED | AREA_NOQUEST))
			continue;
		mobs[mob_count++] = victim;
		if (mob_count >= MAX_QMOB_COUNT)
			break;
	}

	if (mob_count == 0) {
		act_puts("    I'm sorry, but i don't have any quests for you "
			 "at this time.", questor, NULL, ch, TO_VICT, POS_DEAD);
		PC(ch)->questtime = -5;
		return;
	}

	victim = mobs[number_range(0, mob_count-1)];
	PC(ch)->qroom_vnum = victim->in_room->vnum;

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
		eyed->ed = ed_new2(eyed->pObjIndex->ed, ch->name);
		eyed->cost = 0;
		eyed->timer = 30;

		obj_to_room(eyed, victim->in_room);
		PC(ch)->questobj = eyed->pObjIndex->vnum;

		act_puts("    Vile pilferers have stolen {W$p{x "
			 "from the royal treasury!",
			 questor, eyed, ch, TO_VICT | ACT_FORMSH, POS_DEAD);
		act_puts("    My court wizardess, with her magic mirror, "
			 "has pinpointed its location.",
			 questor, NULL, ch, TO_VICT, POS_DEAD);
		act_puts3("    Look in the general area of {W$t{x for {W$R{x!",
			  questor, victim->in_room->area->name, ch,
			  victim->in_room, TO_VICT, POS_DEAD);
	}
	else {	/* Quest to kill a mob */
		if (IS_GOOD(ch)) {
			act_puts("    Rune's most heinous criminal, {W$i{x,\n"
				 "    has escaped from the dungeon.",
				 questor, victim, ch,
				 TO_VICT | ACT_FORMSH, POS_DEAD);
			act_puts3("    Since the escape, $i has murdered "
				  "$J $qJ{civilians}!",
				  questor, victim, ch,
				  (const void*) number_range(2, 20),
				  TO_VICT, POS_DEAD);
			act_puts("    The penalty for this crime is death, "
				 "and you are to deliver the sentence!",
				 questor, victim, ch, TO_VICT, POS_DEAD);
		}
		else {
			act_puts("    An enemy of mine, {W$i{x,\n"
				 "    is making vile threats against the crown.",
				 questor, victim, ch,
				 TO_VICT | ACT_FORMSH, POS_DEAD);
			act_puts("    This threat must be eliminated!",
				 questor, victim, ch, TO_VICT, POS_DEAD);
		}

		act_puts3("    Seek $i out in the vicinity of {W$R{x!",
			  questor, victim, ch, victim->in_room,
			  TO_VICT, POS_DEAD);
		act_puts("    That location is in general area of {W$t{x.",
			 questor, victim->in_room->area->name, ch,
			 TO_VICT, POS_DEAD);

		PC(ch)->questmob = victim->pMobIndex->vnum;
		NPC(victim)->hunter = ch;
	}

	PC(ch)->questgiver = questor->pMobIndex->vnum;
	PC(ch)->questtime = number_range(10, 20) + ch->level/10;
	act_puts("    You have {W$j{x $qj{minutes} to complete this quest.", 
		 questor, (const void*) PC(ch)->questtime, ch,
		 TO_VICT, POS_DEAD);
	act_puts("    May the gods go with you!",
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

	if ((questor = questor_lookup(ch)) == NULL)
		return;

	act("$n informs $N $e has completed $s quest.",
	    ch, NULL, questor, TO_ROOM);
	act_puts("You inform $N you have completed your quest.",
		 ch, NULL, questor, TO_CHAR, POS_DEAD);

	if (!IS_ON_QUEST(ch)) {
		QUESTOR_TELLS_YOU(questor, ch);
		act_puts("    You have to REQUEST a quest first, $N.",
			 questor, NULL, ch, TO_VICT, POS_DEAD);
		return;
	}

	if (PC(ch)->questgiver != questor->pMobIndex->vnum) {
		QUESTOR_TELLS_YOU(questor, ch);
		act_puts("    I never sent you on a quest! Perhaps you're "
			 "thinking of someone else.",
			 questor, NULL, ch, TO_VICT, POS_DEAD);
		return;
	}

	if (PC(ch)->questobj > 0) {
		for (obj = ch->carrying; obj; obj = obj_next) {
			obj_next = obj->next_content;

			if (obj->pObjIndex->vnum == PC(ch)->questobj
			&&  IS_OWNER(ch, obj)) {
				act_puts("You hand {W$p{x to $N.",
					 ch, obj, questor, TO_CHAR, POS_DEAD);
				act("$n hands {W$p{x to $N.",
				    ch, obj, questor, TO_ROOM);
				extract_obj(obj, 0);

				qp_reward = number_range(20, 40);
				gold_reward = 35 + number_range(ch->level, 2*ch->level);

				complete = TRUE;
				break;
			}
		}
	} else if (PC(ch)->questmob == -1) {
		qp_reward = number_range(15, 35);
		gold_reward = dice(ch->level/10 + 1, 30);
		gold_reward = URANGE(0, gold_reward, 20*ch->level);
		complete = TRUE;
	}

	QUESTOR_TELLS_YOU(questor, ch);

	if (!complete) {
		act_puts("    You haven't completed the quest yet, "
			 "but there is still time!",
			 questor, NULL, ch, TO_VICT, POS_DEAD);
		return;
	}

	ch->gold += gold_reward;
	PC(ch)->questpoints += qp_reward;

	act_puts("    Congratulations on completing your quest!",
		 questor, NULL, ch, TO_VICT, POS_DEAD);
	act_puts3("    As a reward, I am giving you {W$j{x $qj{quest points} "
		  "and {W$J{x gold.",
		  questor, (const void*) qp_reward,
		  ch, (const void*) gold_reward,
		  TO_VICT, POS_DEAD);

	quest_cancel(ch);
	PC(ch)->questtime = -number_range(8, 12);
}

static void quest_trouble(CHAR_DATA *ch, char *arg)
{
	CHAR_DATA *questor;
	qitem_t *qitem;

	if ((questor = questor_lookup(ch)) == NULL)
		return;

	if (arg[0] == '\0') {
		act_char("To resolve a quest award's trouble, type: 'quest trouble award'.", ch);
		return;
	}

	for (qitem = qitem_table; qitem->name; qitem++) {
		if (qitem->restrict_class != NULL
		&&  !is_name(ch->class, qitem->restrict_class))
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
			act("    Current challenge quest items available:", 
			    ch, NULL, NULL, TO_CHAR);
		}

		if ((carried_by = chquest_carried_by(q->obj)))
			act("    - $p (carried by $N)",
			    ch, q->obj, carried_by, TO_CHAR | ACT_FORMSH);
		else
			act("    - $p (somewhere)",
			    ch, q->obj, NULL, TO_CHAR | ACT_FORMSH);
	}

	if (!found) {
		act("    No challenge quests are running.",
		    ch, NULL, NULL, TO_CHAR);
		return;
	}
}

static void
quest_cancel_cmd(CHAR_DATA *ch, char *arg)
{
	CHAR_DATA *questor;

	if ((questor = questor_lookup(ch)) == NULL)
		return;

	act_puts("You ask $N to cancel your current quest.",
		 ch, NULL, questor, TO_CHAR, POS_DEAD);
	act("$n asks $N to cancel $s current quest.",
	    ch, NULL, questor, TO_ROOM);

	if (!IS_ON_QUEST(ch)) {
		QUESTOR_TELLS_YOU(questor, ch);
		act_puts("    You have to REQUEST a quest first, $N.",
			 questor, NULL, ch, TO_VICT, POS_DEAD);
		return;
	}

	if (PC(ch)->questgiver != questor->pMobIndex->vnum) {
		QUESTOR_TELLS_YOU(questor, ch);
		act_puts("    I never sent you on a quest! Perhaps you're "
			 "thinking of someone else.",
			 questor, NULL, ch, TO_VICT, POS_DEAD);
		return;
	}

	quest_cancel(ch);
	PC(ch)->questtime = -number_range(12, 15);
	act_puts("Your current quest has been cancelled.",
		 ch, NULL, questor, TO_CHAR, POS_DEAD);
}

/*
 * quest buy functions
 */

static void *
qtrouble_cb(void *vo, va_list ap)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;

	CHAR_DATA *ch = va_arg(ap, CHAR_DATA *);
	int item_vnum = va_arg(ap, int);

	if (obj->pObjIndex->vnum == item_vnum 
	&&  IS_OWNER(ch, obj))
		extract_obj(obj, 0);
	return NULL;
}

static bool quest_give_item(CHAR_DATA *ch, CHAR_DATA *questor,
			    int item_vnum, int count_max)
{
	OBJ_DATA *reward;
	qtrouble_t *qt;
	OBJ_INDEX_DATA *pObjIndex = get_obj_index(item_vnum);

	/*
	 * check quest trouble data
	 */
	qt = qtrouble_lookup(ch, item_vnum);

	if (count_max) {
		/*
		 * 'quest trouble'
		 */

		if ((qt && qt->count > count_max)
		||  !IS_SET(pObjIndex->obj_flags, OBJ_QUEST)) {

			/* ch requested this item too many times	*
			 * or the item is not quest			*/

			QUESTOR_TELLS_YOU(questor, ch);
			act_puts("    This item is beyond the trouble option.",
				 questor, NULL, ch, TO_VICT, POS_DEAD);
			return FALSE;
		}
		else if (!qt) {
			/* ch has never bought this item, but requested it */
			QUESTOR_TELLS_YOU(questor, ch);
			act_puts("    Sorry, $N, but you haven't bought "
				 "that quest award, yet.",
				 questor, NULL, ch, TO_VICT, POS_DEAD);
			return FALSE;
		}
	} else {
		/*
		 * 'quest buy'
		 */

		if (qt && qt->count <= TROUBLE_MAX) {
			QUESTOR_TELLS_YOU(questor, ch);
			act_puts("    You have already bought this item.",
				 questor, NULL, ch, TO_VICT, POS_DEAD);
			return FALSE;
		}
	}

	reward = create_obj(pObjIndex, 0);

	/* update quest trouble data */
	if (qt && count_max) {
		/* `quest trouble' */
		vo_foreach(NULL, &iter_obj_world, qtrouble_cb, ch, item_vnum);

		QUESTOR_TELLS_YOU(questor, ch);
		act_puts("    This is the $j$qj{th} time that I am giving "
			 "that award back.",
			 questor, (const void*) qt->count, ch,
			 TO_VICT, POS_DEAD);
		if (qt->count > count_max) 
			act_puts("    And I won't give you that again, "
				 "with trouble option.",
				 questor, NULL, ch, TO_VICT, POS_DEAD);
	}

	if (!qt && IS_SET(pObjIndex->obj_flags, OBJ_QUEST)) {
		qt = malloc(sizeof(*qt));
		qt->vnum = item_vnum;
		qt->count = 0;
		qt->next = PC(ch)->qtrouble;
		PC(ch)->qtrouble = qt;
	}

	if (qt) {
		if (count_max)
			qt->count++;
		else
			qt->count = 1;
	}

	/* ok, give him requested item */

	if (IS_SET(pObjIndex->obj_flags, OBJ_QUEST)) {
		mlstr_cpy(&reward->owner, &ch->short_descr);
		mlstr_printf(&reward->short_descr,
			     &reward->pObjIndex->short_descr,
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
	PC(ch)->bank_g += 50000;
	act("$N gives 50,000 gold pieces to $n.", ch, NULL, questor, TO_ROOM);
	act("$N transfers 50,000 gold pieces to your bank account.",ch, NULL, questor, TO_CHAR);
	return TRUE;
}

