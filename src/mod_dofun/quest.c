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
 * $Id: quest.c,v 1.69 1998-09-19 10:38:59 fjoe Exp $
 */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#include "merc.h"
#include "hometown.h"
#include "quest.h"

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
#define VNUM_CANTEEN		34402

#define TROUBLE_MAX 3

static void quest_tell(CHAR_DATA *ch, CHAR_DATA *questor, const char *fmt, ...);
static CHAR_DATA *questor_lookup(CHAR_DATA *ch);
QTROUBLE_DATA *qtrouble_lookup(CHAR_DATA *ch, int vnum);

static void quest_points(CHAR_DATA *ch, char *arg);
static void quest_info(CHAR_DATA *ch, char *arg);
static void quest_time(CHAR_DATA *ch, char *arg);
static void quest_list(CHAR_DATA *ch, char *arg);
static void quest_buy(CHAR_DATA *ch, char *arg);
static void quest_request(CHAR_DATA *ch, char *arg);
static void quest_complete(CHAR_DATA *ch, char *arg);
static void quest_trouble(CHAR_DATA *ch, char *arg);

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

typedef struct qitem_data QITEM_DATA;
struct qitem_data {
	char	*name;
	int	price;
	int	class;
	int	vnum;
	bool	(*do_buy)(CHAR_DATA *ch, CHAR_DATA *questor);
};

struct qitem_data qitem_table[] = {
	{ "the Girth of Real Heroism",	1000, CLASS_NONE,
	   QUEST_VNUM_GIRTH, NULL				},

	{ "the Ring of Real Heroism",	1000, CLASS_NONE,
	   QUEST_VNUM_RING, NULL				},

	{ "the Song of Real Hero",	1000, CLASS_NONE,
	   QUEST_VNUM_SONG, NULL				},

	{ "small magic rug",		 750, CLASS_NONE,
	   QUEST_VNUM_RUG, NULL					},

	{ "350,000 gold pieces",	 500, CLASS_NONE,
	   0, buy_gold						},

	{ "60 practices",		 500, CLASS_NONE,
	   0, buy_prac						},

	{ "tattoo of your religion",	 200, CLASS_NONE,
	   0, buy_tattoo					},

	{ "Decrease number of deaths",	  50, CLASS_SAMURAI,
	   0, buy_death						},

	{ "Katana quest",		 100, CLASS_SAMURAI,
	   0, buy_katana					},

	{ "Vampire skill",		  50, CLASS_VAMPIRE,
	   0, buy_vampire					},

	{ "Bottomless canteen with cranberry juice", 350, CLASS_NONE,
	   VNUM_CANTEEN, NULL					},

	{ NULL }
};

struct qcmd_data {
	char *name;
	void (*do_fn)(CHAR_DATA *ch, char* arg);
	int min_position;
	int extra;
};
typedef struct qcmd_data QCMD_DATA;

QCMD_DATA qcmd_table[] = {
	{ "points",	quest_points,	POS_DEAD,	CMD_KEEP_HIDE},
	{ "info",	quest_info,	POS_DEAD,	CMD_KEEP_HIDE},
	{ "time",	quest_time,	POS_DEAD,	CMD_KEEP_HIDE},
	{ "list",	quest_list,	POS_RESTING,	0},
	{ "buy",	quest_buy,	POS_RESTING,	0},
	{ "request",	quest_request,	POS_RESTING,	0},
	{ "complete",	quest_complete,	POS_RESTING,	0},
	{ "trouble",	quest_trouble,	POS_RESTING,	0},
	{ NULL}
};

/*
 * The main quest function
 */
void do_quest(CHAR_DATA *ch, const char *argument)
{
	char cmd[MAX_INPUT_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	QCMD_DATA *qcmd;

	argument = one_argument(argument, cmd);
	argument = one_argument(argument, arg);

	if (IS_NPC(ch)) 
		return;

	for (qcmd = qcmd_table; qcmd->name != NULL; qcmd++)
		if (str_prefix(cmd, qcmd->name) == 0) {
			if (ch->position < qcmd->min_position) {
				char_nputs(MSG_IN_YOUR_DREAMS, ch);
				return;
			}
			if (!IS_SET(qcmd->extra, CMD_KEEP_HIDE)
			&&  IS_SET(ch->affected_by, AFF_HIDE | AFF_FADE)) { 
				char_nputs(MSG_YOU_STEP_OUT_SHADOWS, ch);
				REMOVE_BIT(ch->affected_by, AFF_HIDE | AFF_FADE);
				act_nprintf(ch, NULL, NULL, TO_ROOM,
					   POS_RESTING, MSG_N_STEPS_OUT_OF_SHADOWS);         
			}
			qcmd->do_fn(ch, arg);
			return;
		}
		
	char_nputs(MSG_COMMANDS, ch);
	char_nputs(MSG_TYPE_HELP_QUEST, ch);
}

void quest_handle_death(CHAR_DATA *ch, CHAR_DATA *victim)
{
	if (IS_GOLEM(ch) && ch->master != NULL
	&&  ch->master->class == CLASS_NECROMANCER)
		ch = ch->master;

	if (victim->hunter != NULL)
		if (victim->hunter == ch) {
			char_nputs(MSG_ALMOST_COMPLETE_QUEST, ch);
			char_nputs(MSG_RETURN_TO_QUESTER, ch);
			ch->pcdata->questmob = -1;
		}
		else {
			char_nputs(MSG_YOU_COMPLETED_SOMEONES_QUEST, ch);

			ch = victim->hunter;
			char_nputs(MSG_SOMEONE_COMPLETED_YOUR_QUEST, ch);
			quest_cancel(ch);
			ch->pcdata->questtime = -number_range(5, 10);
		}
}

void quest_cancel(CHAR_DATA *ch)
{
	CHAR_DATA *fch;

	/*
	 * remove mob->hunter
	 */
	for (fch = char_list; fch; fch = fch->next)
		if (fch->hunter == ch) {
			fch->hunter = NULL;
			break;
		}

	if (IS_NPC(ch)) {
		bug("quest_cancel: called for NPC", 0);
		return;
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

	for (ch = char_list; ch != NULL; ch = ch_next) {
		ch_next = ch->next;

		if (IS_NPC(ch)) 
			continue;
		if (ch->pcdata->questtime < 0) {
			if (++ch->pcdata->questtime == 0) {
				char_nputs(MSG_YOU_MAY_NOW_QUEST_AGAIN, ch);
				return;
			}
		} else if (IS_ON_QUEST(ch)) {
			if (--ch->pcdata->questtime == 0) {
				char_nputs(MSG_RUN_OUT_TIME, ch);
				quest_cancel(ch);
				ch->pcdata->questtime = -number_range(5, 10);
			} else if (ch->pcdata->questtime < 6) {
				char_nputs(MSG_BETTER_HURRY, ch);
				return;
			}
		}
	}
}

void qtrouble_set(CHAR_DATA *ch, int vnum, int count)
{
	QTROUBLE_DATA *qt;

	if ((qt = qtrouble_lookup(ch, vnum)) != NULL)
		qt->count = count;
	else {
		qt = alloc_mem(sizeof(*qt));
		qt->vnum = vnum;
		qt->count = count;
		qt->next = ch->pcdata->qtrouble;
		ch->pcdata->qtrouble = qt;
	}
}

/*
 * local functions
 */

static void quest_tell(CHAR_DATA *ch, CHAR_DATA *questor, const char *fmt, ...)
{
	va_list ap;
	char buf[MAX_STRING_LENGTH];

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	act_nprintf(questor, buf, ch, TO_VICT, POS_DEAD, MSG_TELLS_YOU);
}

static CHAR_DATA* questor_lookup(CHAR_DATA *ch)
{
	CHAR_DATA *vch;
	CHAR_DATA *questor = NULL;

	for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room) {
		if (!IS_NPC(vch)) 
			continue;
		if (IS_SET(vch->act, ACT_QUESTOR)) {
			questor = vch;
			break;
		}
	}

	if (questor == NULL) {
		char_nputs(MSG_YOU_CANT_DO_THAT_HERE, ch);
		return NULL;
	}

	if (questor->fighting != NULL) {
		char_nputs(MSG_WAIT_FIGHT_STOPS, ch);
		return NULL;
	}

	return questor;
}

QTROUBLE_DATA *qtrouble_lookup(CHAR_DATA *ch, int vnum)
{
	QTROUBLE_DATA *qt;

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
	char_nprintf(ch, MSG_YOU_HAVE_D_QP, ch->pcdata->questpoints);
}

static void quest_info(CHAR_DATA *ch, char* arg)
{
	if (!IS_ON_QUEST(ch)) {
		char_nputs(MSG_ARENT_ON_QUEST, ch);
		return;
	}

	if (ch->pcdata->questmob == -1) {
		char_nputs(MSG_IS_ALMOST_COMPLETE, ch);
		return;
	}

	if (ch->pcdata->questobj > 0) {
		OBJ_INDEX_DATA *qinfoobj;

		qinfoobj = get_obj_index(ch->pcdata->questobj);
		if (qinfoobj != NULL) {
			char_nprintf(ch, MSG_RECOVER_FABLED, qinfoobj->name);
			if (ch->pcdata->questroom)
				char_nprintf(ch, MSG_INFO_LOCATION,
					ch->pcdata->questroom->area->name, 
					mlstr_mval(ch->pcdata->questroom->name));
		}
		else 
			char_nputs(MSG_ARENT_ON_QUEST, ch);
		return;
	}

	if (ch->pcdata->questmob > 0) {
		MOB_INDEX_DATA *questinfo;

		questinfo = get_mob_index(ch->pcdata->questmob);
		if (questinfo != NULL) {
			char_nprintf(ch, MSG_SLAY_DREADED,
				     mlstr_mval(questinfo->short_descr));
			if (ch->pcdata->questroom)
				char_nprintf(ch, MSG_INFO_LOCATION,
					ch->pcdata->questroom->area->name, 
					mlstr_mval(ch->pcdata->questroom->name));
		} else 
			char_nputs(MSG_ARENT_ON_QUEST, ch);
		return;
	}
}

static void quest_time(CHAR_DATA *ch, char* arg)
{
	if (!IS_ON_QUEST(ch)) {
		char_nputs(MSG_ARENT_ON_QUEST, ch);
		if (ch->pcdata->questtime < -1)
			char_nprintf(ch, MSG_D_MIN_REMAINING,
				     -ch->pcdata->questtime);
	    	else if (ch->pcdata->questtime == -1)
			char_nputs(MSG_LESS_MINUTE, ch);
	}
	else
		char_nprintf(ch, MSG_LEFT_FOR_QUEST, ch->pcdata->questtime);
	return;
}

static void quest_list(CHAR_DATA *ch, char *arg)
{
	CHAR_DATA *questor;
	QITEM_DATA *qitem;

	if ((questor = questor_lookup(ch)) == NULL)
		return;

	act_nprintf(ch, NULL, questor, TO_ROOM, POS_RESTING, MSG_N_ASKS_LIST);
	act_nprintf(ch, NULL, questor, TO_CHAR, POS_DEAD, 
		    MSG_YOU_ASK_LIST);

	char_nputs(MSG_ITEMS_AVAIL_PURCHASE, ch);
	for (qitem = qitem_table; qitem->name; qitem++) {
		if (qitem->class != CLASS_NONE
		&&  qitem->class != ch->class)
			continue;

		if (arg[0] != '\0' && !is_name(arg, qitem->name))
			continue;

		char_printf(ch, "%5dqp...........%s\n\r",
			    qitem->price, qitem->name);
	}
	char_nputs(MSG_TYPE_BUY, ch);
}

static void quest_buy(CHAR_DATA *ch, char *arg)
{
	CHAR_DATA *questor;
	QITEM_DATA *qitem;

	if ((questor = questor_lookup(ch)) == NULL)
		return;

	if (arg[0] == '\0') {
		char_nputs(MSG_TYPE_BUY, ch);
		return;
	}

	for (qitem = qitem_table; qitem->name; qitem++)
		if (is_name(arg, qitem->name)) {
			bool buy_ok = FALSE;

			if (qitem->class != CLASS_NONE
			&&  qitem->class != ch->class)
				continue;

			if (ch->pcdata->questpoints < qitem->price) {
				quest_tell(ch, questor,
					   msg(MSG_NOT_ENOUGH_QP, ch),
					   ch->name);
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

	quest_tell(ch, questor, msg(MSG_NOT_HAVE_ITEM, ch), ch->name);
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

	act_nprintf(ch, NULL, questor, TO_ROOM, POS_RESTING, 
				MSG_N_ASKS_FOR_QUEST);
	act_nprintf(ch, NULL, questor, TO_CHAR, POS_DEAD, 
				MSG_YOU_ASK_FOR_QUEST);
	if (IS_ON_QUEST(ch)) {
    		quest_tell(ch, questor, msg(MSG_YOU_ALREADY_ON_QUEST, ch));
    		return;
	} 

	if (ch->pcdata->questtime < 0) {
		quest_tell(ch, questor,
			   msg(MSG_BRAVE_BUT_LET_SOMEONE_ELSE, ch), ch->name);
		quest_tell(ch, questor, msg(MSG_COME_BACK_LATER, ch));
		return;
	}

	quest_tell(ch, questor, msg(MSG_THANK_YOU_BRAVE, ch), ch->name);

	/*
	 * find MAX_QMOB_COUNT quest mobs and store their vnums in mob_buf
	 */
	mob_count = 0;
	for (victim = char_list; victim; victim = victim->next) {
		int diff = victim->level - ch->level;

		if (!IS_NPC(victim)
		||  (ch->level < 51 && (diff > 4 || diff < -1))
		||  (ch->level > 50 && (diff > 6 || diff < 0))
		||  victim->pIndexData->pShop != NULL
		||  (IS_EVIL(victim) && IS_EVIL(ch))
		||  (IS_GOOD(victim) && IS_GOOD(ch))
		||  IS_SET(victim->pIndexData->act, ACT_TRAIN)
		||  IS_SET(victim->pIndexData->act, ACT_PRACTICE)
		||  IS_SET(victim->pIndexData->act, ACT_HEALER)
		||  IS_SET(victim->pIndexData->act, ACT_NOTRACK)
		||  IS_SET(victim->pIndexData->act, ACT_PET)
		||  IS_SET(victim->pIndexData->imm_flags, IMM_SUMMON)
		||  questor->pIndexData == victim->pIndexData
		||  victim->in_room == NULL
		||  (IS_SET(victim->pIndexData->act, ACT_SENTINEL) &&
		     (IS_SET(victim->in_room->room_flags, ROOM_SAFE) ||
		      IS_SET(victim->in_room->room_flags, ROOM_PRIVATE) ||
		      IS_SET(victim->in_room->room_flags, ROOM_SOLITARY))
		    )
		||  IS_SET(victim->in_room->area->flags, AREA_HOMETOWN))
			continue;
		mobs[mob_count++] = victim;
		if (mob_count >= MAX_QMOB_COUNT)
			break;
	}

	if (mob_count == 0) {
		quest_tell(ch, questor, msg(MSG_DONT_HAVE_QUESTS, ch));
		quest_tell(ch, questor, msg(MSG_TRY_AGAIN_LATER, ch));
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
		eyed = create_named_obj(get_obj_index(obj_vnum), ch->level,
					   ch->name);
		eyed->from = str_dup(ch->name);
		eyed->altar = hometown_table[ch->hometown].altar[i];
		eyed->pit = hometown_table[ch->hometown].pit[i];
		eyed->level = ch->level;
		eyed->ed = ed_new2(eyed->pIndexData->ed, ch->name);
		eyed->cost = 0;
		eyed->timer = 30;

		obj_to_room(eyed, victim->in_room);
		ch->pcdata->questobj = eyed->pIndexData->vnum;

		quest_tell(ch, questor, msg(MSG_VILE_PILFERERS, ch),
			   mlstr_mval(eyed->short_descr));
		quest_tell(ch, questor, msg(MSG_MY_COURT_WIZARDESS, ch));
		quest_tell(ch, questor, msg(MSG_LOCATION_IS_IN_AREA, ch),
			   victim->in_room->area->name,
			   mlstr_mval(victim->in_room->name));
	}
	else {	/* Quest to kill a mob */
		if (IS_GOOD(ch)) {
			quest_tell(ch, questor,
				   msg(MSG_RUNES_MOST_HEINOUS, ch),
				   mlstr_mval(victim->short_descr));
			quest_tell(ch, questor,
				   vmsg(MSG_HAS_MURDERED, ch, victim),
				   mlstr_mval(victim->short_descr),
				   number_range(2, 20));
			quest_tell(ch, questor, msg(MSG_THE_PENALTY_IS, ch));
		}
		else {
			quest_tell(ch, questor, msg(MSG_ENEMY_OF_MINE, ch),
				   mlstr_mval(victim->short_descr));
			quest_tell(ch, questor,
				   msg(MSG_ELIMINATE_THREAT, ch));
		}

		quest_tell(ch, questor, msg(MSG_SEEK_S_OUT, ch),
			   mlstr_mval(victim->short_descr),
			   victim->in_room->area->name,
			   mlstr_mval(victim->in_room->name));

		ch->pcdata->questmob = victim->pIndexData->vnum;
		victim->hunter = ch;
	}

	ch->pcdata->questgiver = questor->pIndexData->vnum;
	ch->pcdata->questtime = number_range(10, 20) + ch->level/10;
	quest_tell(ch, questor, msg(MSG_YOU_HAVE_D_MINUTES, ch), 
			ch->pcdata->questtime);
	quest_tell(ch, questor, msg(MSG_MAY_THE_GODS_GO, ch));
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

	act_nprintf(ch, NULL, questor, TO_ROOM, POS_RESTING, 
					MSG_INFORMS_COMPLETE);
	act_nprintf(ch, NULL, questor, TO_CHAR, POS_DEAD, 
					MSG_YOU_INFORM_COMPLETE);

	if (!IS_ON_QUEST(ch)) {
		quest_tell(ch, questor, msg(MSG_HAVE_TO_REQUEST, ch),
			   ch->name); 
		return;
	}

	if (ch->pcdata->questgiver != questor->pIndexData->vnum) {
		quest_tell(ch, questor, vmsg(MSG_NEVER_QUEST, ch, questor));
		return;
	}

	if (ch->pcdata->questobj > 0)
		for (obj = ch->carrying; obj != NULL; obj = obj_next) {
			obj_next = obj->next_content;

			if (obj != NULL
			&&  obj->pIndexData->vnum == ch->pcdata->questobj
			&&  strstr(mlstr_mval(obj->ed->description),
							ch->name) != NULL) {
				act_nprintf(ch, obj, questor, TO_CHAR, 
					POS_DEAD, MSG_YOU_HAND_P);
				act_nprintf(ch, obj, questor, TO_ROOM, 
					POS_RESTING, MSG_N_HANDS_P);
				extract_obj(obj);

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

	if (!complete) {
		quest_tell(ch, questor, msg(MSG_HAVENT_COMPLETE, ch));
		return;
	}

	ch->gold += gold_reward;
	ch->pcdata->questpoints += qp_reward;

	quest_tell(ch, questor, msg(MSG_GRATS_COMPLETE, ch));
	quest_tell(ch, questor, msg(MSG_AS_A_REWARD, ch),
		   qp_reward, gold_reward);

	if (prac_reward) {
		ch->practice += prac_reward;
		quest_tell(ch, questor, msg(MSG_GAIN_PRACS, ch), prac_reward);
	}

	quest_cancel(ch);
	ch->pcdata->questtime = -number_range(8, 12);
}

static void quest_trouble(CHAR_DATA *ch, char *arg)
{
	CHAR_DATA *questor;
	QITEM_DATA *qitem;

	if ((questor = questor_lookup(ch)) == NULL)
		return;

	if (arg[0] == '\0') {
		char_nputs(MSG_TYPE_TROUBLE, ch);
		return;
	}

	for (qitem = qitem_table; qitem->name; qitem++) {
		if (qitem->class != CLASS_NONE && qitem->class != ch->class)
			continue;

		if (qitem->vnum && is_name(arg, qitem->name)) {
			quest_give_item(ch, questor, qitem->vnum, TROUBLE_MAX);
			return;
		}
	}

	quest_tell(ch, questor, msg(MSG_HAVENT_BOUGHT, ch), ch->name);
}

/*
 * quest buy functions
 */

static bool quest_give_item(CHAR_DATA *ch, CHAR_DATA *questor,
			    int item_vnum, int count_max)
{
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;
	QTROUBLE_DATA *qt;

	/* check quest trouble data */

	qt = qtrouble_lookup(ch, item_vnum);

	if (count_max) {
		if (qt == NULL) {
			/* ch has never bought this item, but requested it */
			quest_tell(ch, questor, msg(MSG_HAVENT_BOUGHT, ch),
				   ch->name);
			return FALSE;
		}
		else if (qt->count > count_max) {
			/* ch requested this item too many times */
			quest_tell(ch, questor, msg(MSG_ITEM_BEYOND, ch));
			return FALSE;
		}
	}

	/* update quest trouble data */

	if (qt != NULL && count_max) {
		/* `quest trouble' */
		for (obj = object_list; obj != NULL; obj = obj_next) {
			obj_next = obj->next;
			/* XXX */
			if (obj->pIndexData->vnum == item_vnum 
			&&  strstr(mlstr_mval(obj->short_descr), ch->name)) {
				extract_obj(obj);
				break;
			}
		}

		quest_tell(ch, questor,
			   msg(MSG_THIS_IS_THE_NTH_TIME, ch), qt->count);
		if (qt->count == count_max) 
			quest_tell(ch, questor, msg(MSG_WONT_GIVE_AGAIN, ch));
	}

	if (qt == NULL) {
		qt = alloc_mem(sizeof(*qt));
		qt->vnum = item_vnum;
		qt->count = 0;
		qt->next = ch->pcdata->qtrouble;
		ch->pcdata->qtrouble = qt;
	}


	if (count_max)
		qt->count++;
	else
		qt->count = 1;

	/* ok, give him requested item */

	obj = create_obj(get_obj_index(item_vnum), ch->level);
	mlstr_free(obj->short_descr);
	obj->short_descr = mlstr_printf(obj->pIndexData->short_descr,
			IS_GOOD(ch) ?		"holy" :
			IS_NEUTRAL(ch) ?	"blue-green" : 
						"evil", 
			ch->name);
	obj_to_char(obj, ch);

	act_nprintf(ch, obj, questor, TO_ROOM, POS_RESTING, 
		    MSG_GIVES_P_TO_N);
	act_nprintf(ch, obj, questor, TO_CHAR, POS_DEAD, MSG_GIVES_YOU_P);

	return TRUE;
}

static bool buy_gold(CHAR_DATA *ch, CHAR_DATA *questor)
{
	ch->pcdata->bank_g += 350000;
	act_nprintf(ch, NULL, questor, TO_ROOM, POS_RESTING,
						MSG_N_GIVES_GOLD);
	act_nprintf(ch, NULL, questor, TO_CHAR, POS_DEAD,
						MSG_N_GIVES_YOU_GOLD);
	return TRUE;
}

static bool buy_prac(CHAR_DATA *ch, CHAR_DATA *questor)
{
	ch->practice += 60;
	act_nprintf(ch, NULL, questor, TO_ROOM, POS_RESTING,
					MSG_N_GIVES_PRACS);
	act_nprintf(ch, NULL, questor, TO_CHAR, POS_DEAD,
					MSG_N_GIVES_YOU_PRACS);
	return TRUE;
}

static bool buy_tattoo(CHAR_DATA *ch, CHAR_DATA *questor)
{
	OBJ_DATA *tattoo;

	if (ch->religion == NULL) {
		char_nputs(MSG_NO_RELIGION, ch);
		return FALSE;
	}

	tattoo = get_eq_char(ch, WEAR_TATTOO);
	if (tattoo != NULL) {
		char_nputs(MSG_ALREADY_TATTOOED, ch);
		return FALSE;
	}

	tattoo = create_obj(get_obj_index(religion_table[ch->religion].vnum),
			       100);

	obj_to_char(tattoo, ch);
	equip_char(ch, tattoo, WEAR_TATTOO);
	act_nprintf(ch, tattoo, questor, TO_ROOM, POS_RESTING,
						MSG_N_TATTOOS_N);
	act_nprintf(ch, tattoo, questor, TO_CHAR, POS_DEAD,
						MSG_N_TATTOOS_YOU);
	return TRUE;
}

static bool buy_death(CHAR_DATA *ch, CHAR_DATA *questor)
{
	if (ch->pcdata->death < 1) {
		quest_tell(ch, questor, msg(MSG_NO_DEATHS, ch), ch->name);
		return FALSE;
	}

	ch->pcdata->death -= 1;
	return TRUE;
}

static bool buy_katana(CHAR_DATA *ch, CHAR_DATA *questor)
{
	AFFECT_DATA af;
	OBJ_DATA *katana;

	if ((katana = get_obj_list(ch, "katana", ch->carrying)) == NULL) {
		quest_tell(ch, questor, msg(MSG_DONT_HAVE_KATANA, ch),
			   ch->name);
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
	quest_tell(ch, questor, msg(MSG_AS_YOU_WIELD_IT, ch));
	return TRUE;
}

static bool buy_vampire(CHAR_DATA *ch, CHAR_DATA *questor)
{
	set_skill(ch, gsn_vampire, 100);
	act_nprintf(ch, NULL, questor, TO_ROOM, POS_RESTING,
					MSG_N_GIVES_SECRET);
	act_nprintf(ch, NULL, questor, TO_CHAR, POS_DEAD,
					MSG_N_GIVES_YOU_SECRET);
	act_nprintf(ch, NULL, questor, TO_ALL, POS_DEAD,
		    MSG_WEATHER_LIGHTNING_FLASHES);
	return TRUE;
}

