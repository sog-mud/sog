/*-
 * Copyright (c) 2000 SoG Development Team
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
 * $Id: updfun.c,v 1.8 2000-06-01 17:57:51 fjoe Exp $
 */

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "db.h"
#include "chquest.h"
#include "auction.h"
#include "mob_prog.h"
#include "obj_prog.h"

#include "quest.h"
#include "fight.h"
#include "_update.h"

/* locals */
static void	*check_assist_cb(void *vo, va_list ap);
static int	potion_cure_level(OBJ_DATA *potion);
static int	potion_arm_level(OBJ_DATA *potion);
static bool	is_potion(OBJ_DATA *potion, const char *sn);
static int	hit_gain(CHAR_DATA *ch);
static int	mana_gain(CHAR_DATA *ch);
static int	move_gain(CHAR_DATA *ch);
static void	update_obj_affects(OBJ_DATA *obj);
static bool	update_melt_obj(OBJ_DATA *obj);
static bool	update_potion(OBJ_DATA *obj);
static bool	update_drinkcon(OBJ_DATA *obj);
static inline void contents_to_obj(OBJ_DATA *obj, OBJ_DATA *to_obj);
static inline void save_corpse_contents(OBJ_DATA *corpse);
static void	*find_aggr_cb(void *vo, va_list ap);
static void	*raff_update_cb(void *vo, va_list ap);
static void	*put_back_cb(void *p, va_list ap);
static void	*clan_item_update_cb(void *p, va_list ap);
static void	print_resetmsg(AREA_DATA *pArea);

void *
violence_update_cb(void *vo, va_list ap)
{
	CHAR_DATA *ch = (CHAR_DATA *) vo;
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;

	/* decrement the wait */
	if (ch->desc == NULL)
		ch->wait = UMAX(0, ch->wait - get_pulse("violence"));

	if ((victim = ch->fighting) == NULL || ch->in_room == NULL)
		return NULL;

	if (IS_AWAKE(ch) && ch->in_room == victim->in_room)
		multi_hit(ch, victim, NULL);
	else
		stop_fighting(ch, FALSE);

	if ((victim = ch->fighting) == NULL)
		return NULL;

	if (IS_NPC(ch) && !IS_NPC(victim))
		NPC(ch)->last_fought = victim;

	SET_FIGHT_TIME(ch);

	if (victim->in_room != ch->in_room)
		return NULL;

	for (obj = ch->carrying; obj; obj = obj_next) {
		obj_next = obj->next_content;
		if (ch->fighting == NULL)
			break;
		oprog_call(OPROG_FIGHT, obj, ch, NULL);
	}

	if ((victim = ch->fighting) == NULL
	||  victim->in_room != ch->in_room)
		return NULL;

	/*
	 * Fun for the whole family!
	 */
	vo_foreach(ch->in_room, &iter_char_room, check_assist_cb, ch, victim);
	if (IS_NPC(ch)) {
		if (HAS_TRIGGER(ch, TRIG_FIGHT)) {
			mp_percent_trigger(ch, victim, NULL, NULL, TRIG_FIGHT);
			if (IS_EXTRACTED(ch)
			||  IS_EXTRACTED(victim))
				return NULL;
		}
		if (HAS_TRIGGER(ch, TRIG_HPCNT))
			mp_hprct_trigger(ch, victim);
	}

	return NULL;
}

void *
mobile_update_cb(void *vo, va_list ap)
{
	CHAR_DATA *ch = (CHAR_DATA *) vo;
	EXIT_DATA *pexit;
	int door;
	OBJ_DATA *obj;

	bool bust_prompt = FALSE;
	flag_t act;

	if (ch->position == POS_FIGHTING)
		SET_FIGHT_TIME(ch);

	if (!IS_NPC(ch)) {
/* update ghost state */
		if (ch->last_death_time != -1
		&&  current_time - ch->last_death_time >= GHOST_DELAY_TIME
		&&  IS_SET(PC(ch)->plr_flags, PLR_GHOST)) {
			char_puts("You return to your normal form.\n", ch);
			REMOVE_BIT(PC(ch)->plr_flags, PLR_GHOST);
		}

		PC(ch)->random_value = number_range(0, 1000);
	}

/* update pumped state */
	if (ch->last_fight_time != -1
	&&  current_time - ch->last_fight_time >= FIGHT_DELAY_TIME
	&&  IS_PUMPED(ch)) {
		if (!IS_NPC(ch) && ch->desc != NULL
		&&  ch->desc->pString == NULL 
		&&  (ch->last_death_time == -1 ||
		     ch->last_death_time < ch->last_fight_time)) {
			REMOVE_BIT(PC(ch)->plr_flags, PLR_PUMPED);
			act("You settle down.", ch, NULL, NULL, TO_CHAR);
		}
	}

	if (IS_AFFECTED(ch, AFF_REGENERATION) && ch->in_room != NULL) {
		ch->hit = UMIN(ch->hit + ch->level / 10, ch->max_hit);
		if (IS_RACE(ch->race, "troll"))
			ch->hit = UMIN(ch->hit + ch->level / 10, ch->max_hit);
		if (ch->hit != ch->max_hit)
			bust_prompt = TRUE;
	}

	if (IS_AFFECTED(ch, AFF_CORRUPTION) && ch->in_room != NULL) {
		ch->hit -=  ch->level / 10;
		if (ch->hit < 1) {
			if (IS_IMMORTAL(ch)) 
				ch->hit = 1;
			else {
				ch->position = POS_DEAD;
				handle_death(ch, ch);
				return NULL;
			}
		}
		bust_prompt = TRUE;
	}

	if (ch->in_room && ch->in_room->sector_type == SECT_UNDERWATER
	&&  !is_affected(ch, "water breathing")
	&&  !IS_IMMORTAL(ch)) {
		act("$n gasps for fresh air, but inhales water.",
		    ch, NULL, NULL, TO_ROOM);
		act("You gasp for fresh air, but inhale water.",
		    ch, NULL, NULL, TO_CHAR);
		ch->hit -= ch->max_hit/20;
		if (ch->hit < 1) {
			ch->position = POS_DEAD;
			handle_death(ch, ch);
			return NULL;
		}
		bust_prompt = TRUE;
	}

	check_events(ch, ch->affected, EVENT_CHAR_UPDFAST);
	if (IS_EXTRACTED(ch))
		return NULL;

	if (ch->desc
	&&  bust_prompt
	&&  !ch->desc->pString
	&&  !ch->desc->showstr_point
	&&  !IS_SET(ch->comm, COMM_NOBUST))
		char_puts(str_empty, ch);

	/*
	 * that's all for PCs and charmed mobiles
	 */
	if (!IS_NPC(ch)
	||  IS_AFFECTED(ch, AFF_CHARM))
		return NULL;

	act = ch->pMobIndex->act;
	if (IS_SET(act, ACT_HUNTER) && ch->hunting) {
		dofun("hunt", ch, str_empty);
		if (IS_EXTRACTED(ch))
			return NULL;
	}

	if (ch->in_room->area->empty
	&&  !IS_SET(act, ACT_UPDATE_ALWAYS))
		return NULL;

	/* Examine call for special procedure */
	if (ch->pMobIndex->spec_fun != 0) {
		if (ch->pMobIndex->spec_fun(ch))
			return NULL;
	}

/* check triggers (only if mobile still in default position) */

	if (ch->position == ch->pMobIndex->default_pos) {
		if (HAS_TRIGGER(ch, TRIG_DELAY)
		&&  NPC(ch)->mprog_delay > 0) {
			if (--NPC(ch)->mprog_delay <= 0) {
				mp_percent_trigger(ch, NULL, NULL, NULL, TRIG_DELAY);
				return NULL;
			}
		} 
		if (HAS_TRIGGER(ch, TRIG_RANDOM)) {
			if(mp_percent_trigger(ch, NULL, NULL, NULL, TRIG_RANDOM))
			return NULL;
		}
	}

/* potion using and stuff for intelligent mobs */

	if (ch->pMobIndex->pShop == NULL
	&&  (ch->position == POS_STANDING ||
	     ch->position == POS_RESTING ||
	     ch->position == POS_FIGHTING)
	&&  get_curr_stat(ch, STAT_INT) > 15
	&&  (ch->hit < ch->max_hit * 90 / 100 ||
	     IS_AFFECTED(ch, AFF_BLIND) ||
	     IS_AFFECTED(ch, AFF_POISON) ||
	     IS_AFFECTED(ch, AFF_PLAGUE) ||
	     ch->fighting != NULL)) {
		for (obj = ch->carrying; obj; obj = obj->next_content) {
			if (obj->item_type != ITEM_POTION)
				continue;

			if (ch->hit < ch->max_hit * 90 / 100) {
				int cl = potion_cure_level(obj);
				if (cl > 0) {
					if (ch->hit < ch->max_hit*0.5
					&&  cl > 3) {
						quaff_obj(ch, obj);
						if (IS_EXTRACTED(ch))
							break;
						continue;
					}
					if (ch->hit < ch->max_hit*0.7) {
						quaff_obj(ch, obj);
						if (IS_EXTRACTED(ch))
							break;
						continue;
					}
				}
			}

			if (IS_AFFECTED(ch, AFF_POISON)
			&&  is_potion(obj, "cure poison")) {
				quaff_obj(ch, obj);
				if (IS_EXTRACTED(ch))
					break;
				continue;
			}

			if (IS_AFFECTED(ch, AFF_PLAGUE)
			&&  is_potion(obj, "cure disease")) {
				quaff_obj(ch, obj);
				if (IS_EXTRACTED(ch))
					break;
				continue;
			}

			if (IS_AFFECTED(ch, AFF_BLIND)
			&&  is_potion(obj, "cure blindness")) {
				quaff_obj(ch, obj);
				if (IS_EXTRACTED(ch))
					break;
				continue;
			}

			if (ch->fighting) {
				int al = potion_arm_level(obj);

				if (ch->level - ch->fighting->level < 7
				&&  al > 3) {
					quaff_obj(ch, obj);
					if (IS_EXTRACTED(ch))
						break;
					continue;
				}

				if (ch->level - ch->fighting->level < 8
				&&  al > 2) {
					quaff_obj(ch, obj);
					if (IS_EXTRACTED(ch))
						break;
					continue;
				}

				if (ch->level - ch->fighting->level < 9
				&&  al > 1) {
					quaff_obj(ch, obj);
					if (IS_EXTRACTED(ch))
						break;
					continue;
				}

				if (ch->level - ch->fighting->level < 10
				&&  al > 0) {
					quaff_obj(ch, obj);
					if (IS_EXTRACTED(ch))
						break;
					continue;
				}
			} /* if (ch->fighting) */
		} /* for */

		if (IS_EXTRACTED(ch))
			return NULL;
	}

/* That's all for sleeping / busy monster, and empty zones */
	if (ch->position != POS_STANDING)
		return NULL;

/* Scavenge */
	if (IS_SET(act, ACT_SCAVENGER)
	&&  ch->in_room->contents != NULL
	&&  number_bits(6) == 0) {
		OBJ_DATA *obj;
		OBJ_DATA *obj_best = NULL;
		int max = 1;

		for (obj = ch->in_room->contents; obj; obj = obj->next_content) {
			if (CAN_WEAR(obj, ITEM_TAKE)
			&&  can_loot(ch, obj)
			&&  obj->cost > max) {
				obj_best = obj;
				max	 = obj->cost;
			}
		}

		if (obj_best)
			get_obj(ch, obj_best, NULL, NULL);
	}

/* Wander */
	if (!IS_SET(act, ACT_SENTINEL) 
	&&  number_bits(3) == 0
	&&  (door = number_bits(5)) <= 5
	&&  !RIDDEN(ch)
	&&  (pexit = ch->in_room->exit[door]) != NULL
	&&  pexit->to_room.r != NULL
	&&  !IS_SET(pexit->exit_info, EX_CLOSED)
	&&  !IS_SET(pexit->to_room.r->room_flags, ROOM_NOMOB | ROOM_GUILD)
	&&  (!IS_SET(act, ACT_STAY_AREA) ||
	     pexit->to_room.r->area == ch->in_room->area) 
	&&  (!IS_SET(act, ACT_AGGRESSIVE) ||
	     !IS_SET(pexit->to_room.r->room_flags, ROOM_PEACE))
	&&  (!IS_SET(act, ACT_OUTDOORS) ||
	     !IS_SET(pexit->to_room.r->room_flags, ROOM_INDOORS)) 
	&&  (!IS_SET(act, ACT_INDOORS) ||
	     IS_SET(pexit->to_room.r->room_flags, ROOM_INDOORS)))
		move_char(ch, door, FALSE);

	return NULL;
}

void
weather_update(void)
{
	CHAR_DATA *ch;
	int diff;
	const char *msg_daytime = NULL;
	const char *msg_weather = NULL;

	switch (++time_info.hour) {
	case  5:
		weather_info.sunlight = SUN_LIGHT;
		msg_daytime = "The day has begun.";
		break;

	case  6:
		weather_info.sunlight = SUN_RISE;
		msg_daytime = "The sun rises in the east.";
		break;

	case 19:
		weather_info.sunlight = SUN_SET;
		msg_daytime = "The sun slowly disappears in the west.";
		break;

	case 20:
		weather_info.sunlight = SUN_DARK;
		msg_daytime = "The night has begun.";
		break;

	case 24:
		time_info.hour = 0;
		time_info.day++;
		break;
	}

	if (time_info.day >= 35) {
		time_info.day = 0;
		time_info.month++;
	}

	if (time_info.month >= 17) {
		time_info.month = 0;
		time_info.year++;
	}

	/*
	 * Weather change.
	 */
	if (time_info.month >= 9 && time_info.month <= 16)
		diff = weather_info.mmhg >  985 ? -2 : 2;
	else
		diff = weather_info.mmhg > 1015 ? -2 : 2;

	weather_info.change   += diff * dice(1, 4) + dice(2, 6) - dice(2, 6);
	weather_info.change    = UMAX(weather_info.change, -12);
	weather_info.change    = UMIN(weather_info.change,  12);

	weather_info.mmhg += weather_info.change;
	weather_info.mmhg  = UMAX(weather_info.mmhg,  960);
	weather_info.mmhg  = UMIN(weather_info.mmhg, 1040);

	switch (weather_info.sky) {
	default: 
		log(LOG_ERROR, "Weather_update: bad sky %d.", weather_info.sky);
		weather_info.sky = SKY_CLOUDLESS;
		break;

	case SKY_CLOUDLESS:
		if (weather_info.mmhg < 990
		||  (weather_info.mmhg < 1010 && number_bits(2) == 0)) {
			msg_weather = "The sky is getting cloudy.";
			weather_info.sky = SKY_CLOUDY;
		}
		break;

	case SKY_CLOUDY:
		if (weather_info.mmhg < 970
		||  (weather_info.mmhg < 990 && number_bits(2) == 0)) {
			msg_weather = "It starts to rain.";
			weather_info.sky = SKY_RAINING;
		}
		else if (weather_info.mmhg > 1030 && number_bits(2) == 0) {
			msg_weather = "The clouds disappear.";
			weather_info.sky = SKY_CLOUDLESS;
		}
		break;

	case SKY_RAINING:
		if (weather_info.mmhg > 1030
		||  (weather_info.mmhg > 1010 && number_bits(2) == 0)) {
			msg_weather = "The rain stopped.";
			weather_info.sky = SKY_CLOUDY;
		}
		else if (weather_info.mmhg < 970 && number_bits(2) == 0) {
			msg_weather = "Lightning flashes in the sky.";
			weather_info.sky = SKY_LIGHTNING;
		}
		break;

	case SKY_LIGHTNING:
		if (weather_info.mmhg > 1010
		||  (weather_info.mmhg > 990 && number_bits(2) == 0)) {
			msg_weather = "The lightning has stopped.";
			weather_info.sky = SKY_RAINING;
		}
		break;
	}

	if (!msg_daytime && !msg_weather)
		return;

	for (ch = char_list; ch; ch = ch->next) {
		if (IS_NPC(ch) && !HAS_TRIGGER(ch, TRIG_ACT))
			continue;
		if (!IS_OUTSIDE(ch) || !IS_AWAKE(ch))
			continue;

		act(msg_daytime, ch, NULL, NULL, TO_CHAR);
		act(msg_weather, ch, NULL, NULL, TO_CHAR);
	}
}

void *
char_update_cb(void *vo, va_list ap)
{   
	CHAR_DATA *ch = (CHAR_DATA *) vo;

	AFFECT_DATA *paf;
	AFFECT_DATA *paf_next;
	int chance;
	race_t *r;

	if ((r = race_lookup(ch->race)) == NULL)
		return NULL;

	/* reset path find */
	if (!IS_NPC(ch) && (chance = get_skill(ch, "path find"))) {
		if (number_percent() < chance) {
			ch->endur += chance / 2;
			check_improve(ch, "path find", TRUE, 8);
		} else
			check_improve(ch, "path find", FALSE, 16);
	}
		
	if (!ch->fighting) {
		flag_t inv_skip = 0;

		affect_check(ch, -1, -1);

		/* Remove caltrops effect after fight off */
		if (is_affected(ch, "caltrops"))
			affect_strip(ch, "caltrops");

		if (!MOUNTED(ch)) {
			if (!HAS_INVIS(ch, ID_HIDDEN) 
			&&  (r->has_invis & ID_HIDDEN))
				char_puts("You step back into the shadows.\n", ch);

			if (!HAS_INVIS(ch, ID_SNEAK)
			&&  (r->has_invis & ID_SNEAK))
				char_puts("You move silently again.\n", ch);
		} else
			inv_skip |= ID_ALL_INVIS;

		SET_BIT(ch->affected_by, r->aff & ~AFF_FLYING);
		SET_INVIS(ch, r->has_invis & ~inv_skip);
		SET_DETECT(ch, r->has_detect);
	}

	/* Remove vampire effect when morning. */
	if (is_affected(ch, "vampire")
	&&  (weather_info.sunlight == SUN_LIGHT ||
	     weather_info.sunlight == SUN_RISE))
		dofun("human", ch, str_empty);

	if (!IS_NPC(ch) && is_affected(ch, "thumbling")) {
		if (dice(5, 6) > get_curr_stat(ch, STAT_DEX)) {
			act("You failed to reach the true source of tennis ball power.", ch, NULL, NULL, TO_CHAR);
			act("$n falls to the ground flat on $s face.", ch, NULL, NULL, TO_ROOM);
			affect_strip(ch, "thumbling");
		}
	}

	if (ch->position >= POS_STUNNED) {
		int old_hit = ch->hit;
		int old_mana = ch->mana;
		int old_move = ch->move;
		NPC_DATA *npc;

		/* check to see if we need to go home */
		if (IS_NPC(ch)
		&&  (npc = NPC(ch))->zone != NULL 
		&&  ch->in_room != NULL
		&&  npc->zone != ch->in_room->area
		&&  ch->desc == NULL 
		&&  ch->fighting == NULL
/* && ch->progtypes==0 */
		&&  !IS_AFFECTED(ch, AFF_CHARM)
		&&  npc->last_fought == NULL
		&&  !RIDDEN(ch)) {
			back_home(ch);
			return NULL;
		}

		if (ch->hit < ch->max_hit)
			ch->hit += hit_gain(ch);
		else
			ch->hit = ch->max_hit;

		if (ch->mana < ch->max_mana)
			ch->mana += mana_gain(ch);
		else
			ch->mana = ch->max_mana;

		if (ch->move < ch->max_move)
			ch->move += move_gain(ch);
		else
			ch->move = ch->max_move;

		if (ch->desc
		&&  (old_hit != ch->hit || old_mana != ch->mana ||
		     old_move != ch->move)
		&&  !ch->desc->pString
		&&  !ch->desc->showstr_point
		&&  !IS_SET(ch->comm, COMM_NOBUST))
			char_puts(str_empty, ch);
	}

	if (ch->position == POS_STUNNED)
		update_pos(ch);

	if (!IS_NPC(ch) && ch->level < LEVEL_IMMORTAL) {
		OBJ_DATA *obj;
		PC_DATA *pc = PC(ch);

		if ((obj = get_eq_char(ch, WEAR_LIGHT))
		&&  obj->item_type == ITEM_LIGHT
		&&  INT(obj->value[2]) > 0) {
			if (--INT(obj->value[2]) == 0) {
				if (ch->in_room->light > 0)
					--ch->in_room->light;
				act("$p goes out.", ch, obj, NULL, TO_ROOM);
				act("$p flickers and goes out.",
				    ch, obj, NULL, TO_CHAR);
				extract_obj(obj, 0);
			} else if (INT(obj->value[2]) <= 5)
				act("$p flickers.", ch, obj, NULL, TO_CHAR);
		}

		if (++pc->idle_timer >= 12) {
			if (!pc->was_in_vnum) {
				pc->was_in_vnum = ch->in_room->vnum;
				if (ch->fighting != NULL)
					stop_fighting(ch, TRUE);
				act("$n disappears into the void.",
				    ch, NULL, NULL, TO_ROOM);
				char_puts("You disappear into the void.\n", ch);
				char_save(ch, 0);
  				char_from_room(ch);
				char_to_room(ch, get_room_index(ROOM_VNUM_LIMBO));
				if (IS_EXTRACTED(ch))
					return NULL;
			}
		}

		if (!pc->was_in_vnum) {
			gain_condition(ch, COND_DRUNK, -1);
			if (IS_VAMPIRE(ch))
				gain_condition(ch, COND_BLOODLUST, -1);
			gain_condition(ch, COND_FULL, 
				       ch->size > SIZE_MEDIUM ? -4 : -2);
			if (ch->in_room->sector_type == SECT_DESERT)
				gain_condition(ch, COND_THIRST, -3);
			else
				gain_condition(ch, COND_THIRST, -1);
			gain_condition(ch, COND_HUNGER, 
				       ch->size > SIZE_MEDIUM ? -2 : -1);
			if (IS_EXTRACTED(ch))
				return NULL;
		}
	}

	for (paf = ch->affected; paf != NULL; paf = paf_next) {
		paf_next = paf->next;
		if (paf->duration > 0) {
			paf->duration--;
			if (number_range(0, 4) == 0 && paf->level > 0)
				paf->level--;
			/* spell strength fades with time */
		} else if (paf->duration == 0) {
			skill_t *sk;

			if ((paf_next == NULL ||
			     paf_next->type != paf->type ||
			     paf_next->duration > 0)
			&&  (sk = skill_lookup(paf->type)) != NULL
			&&  !mlstr_null(&sk->msg_off)) 
				act_mlputs(&sk->msg_off, ch, NULL, NULL,
					   TO_CHAR, POS_DEAD);

			check_one_event(ch, paf, EVENT_CHAR_TIMEOUT);
			if (IS_EXTRACTED(ch))
				break;

			affect_remove(ch, paf);
		}
	}

	check_events(ch, ch->affected, EVENT_CHAR_UPDATE);
	if (IS_EXTRACTED(ch))
		return NULL;

	if (ch->position == POS_INCAP 
	&&  number_range(0, 1) == 0)
		damage(ch, ch, 1, NULL, DAM_NONE, DAMF_NONE);
	else if (ch->position == POS_MORTAL)
		damage(ch, ch, 1, NULL, DAM_NONE, DAMF_NONE);

	return NULL;
}

void
save_update(void)
{
	static time_t last_save_time = -1;
	CHAR_DATA *ch, *ch_next;

	if (last_save_time == -1 || current_time - last_save_time > 300) {
		last_save_time = current_time;
		for (ch = char_list; ch && !IS_NPC(ch); ch = ch_next) {
			ch_next = ch->next;
			if (PC(ch)->idle_timer > 20)
				quit_char(ch, XC_F_NOCOUNT);
			else
				char_save(ch, 0);
		}
	}
}

void *
water_float_update_cb(void *vo, va_list ap)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;

	if (obj->in_room == NULL || !IS_WATER(obj->in_room))
		return NULL;

	obj->water_float = obj->water_float > 0 ?  obj->water_float - 1 : -1;

	if (obj->item_type == ITEM_DRINK_CON) {
		INT(obj->value[1]) = URANGE(1, INT(obj->value[1]) + 8,
					    INT(obj->value[0]));
		act("$p makes bubbles on the water.",
		    obj->in_room->people, obj, NULL, TO_ALL);
		obj->water_float = INT(obj->value[0]) - INT(obj->value[1]);
		INT(obj->value[2]) = 0;
	}

	if (obj->water_float == 0) {
		act("$p sinks down the water.",
		    obj->in_room->people, obj, NULL, TO_ALL); 
		extract_obj(obj, 0);
	}

	return NULL;
}

void *
obj_update_cb(void *vo, va_list ap)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	OBJ_DATA *t_obj;
	char *message;

	if (IS_AUCTIONED(obj))
		return NULL;

	update_obj_affects(obj);

	/* find the uppest obj container */
	for(t_obj = obj; t_obj->in_obj; t_obj = t_obj->in_obj)
		;

	if ((t_obj->in_room != NULL &&
	     t_obj->in_room->area->nplayer > 0)
        ||  (t_obj->carried_by &&
	     t_obj->carried_by->in_room &&
	     t_obj->carried_by->in_room->area->nplayer > 0))
		oprog_call(OPROG_AREA, obj, NULL, NULL);

	if (material_is(obj, MATERIAL_SUSC_HEAT) 
	&&  update_melt_obj(obj))
		return NULL;

	if (obj->item_type == ITEM_POTION
	&&  update_potion(obj))
		return NULL;

	if (obj->item_type == ITEM_DRINK_CON
	&&  update_drinkcon(obj))
		return NULL;

	if (obj->condition > -1 && (obj->timer <= 0 || --obj->timer > 0))
		return NULL;

	switch (obj->item_type) {
	default:
		message = "$p crumbles into dust.";
		break;
	case ITEM_FOUNTAIN:
		message = "$p dries up.";
		break;
	case ITEM_CORPSE_NPC:
		message = "$p decays into dust.";
		break;
	case ITEM_CORPSE_PC:
		message = "$p decays into dust.";
		break;
	case ITEM_FOOD:
		message = "$p decomposes.";
		break;
	case ITEM_POTION:
		message = "$p has evaporated from disuse.";	
		break;
	case ITEM_PORTAL:
		message = "$p fades out of existence.";
		break;
	case ITEM_CONTAINER: 
		if (CAN_WEAR(obj, ITEM_WEAR_FLOAT))
			if (obj->contains)
				message = "$p flickers and vanishes, spilling "
					  "its contents on the floor.";
			else
				message = "$p flickers and vanishes.";
		else
			message = "$p crumbles into dust.";
			break;
	}

	if (obj->carried_by) {
		if (IS_NPC(obj->carried_by) 
		&&  obj->carried_by->pMobIndex->pShop != NULL)
			obj->carried_by->silver += obj->cost/5;
		else {
			act(message, obj->carried_by, obj, NULL, TO_CHAR);
			if (obj->wear_loc == WEAR_FLOAT)
				act(message, obj->carried_by, obj, NULL,
				    TO_ROOM);
		}
	}

	if (obj->in_room && !OBJ_IS(obj, OBJ_PIT))
		act(message, obj->in_room->people, obj, NULL, TO_ALL);

	if (obj->item_type == ITEM_CORPSE_PC && obj->contains)
		save_corpse_contents(obj);
	extract_obj(obj, 0);

	return NULL;
}

void *
aggr_update_cb(void *vo, va_list ap)
{
	CHAR_DATA *wch = (CHAR_DATA *) vo;

	if (IS_NPC(wch))
		return wch;

	if (IS_IMMORTAL(wch))
		return NULL;

	if (IS_AWAKE(wch)
	&&  IS_AFFECTED(wch, AFF_BLOODTHIRST)
	&&  wch->fighting == NULL
	&&  vo_foreach(wch->in_room, &iter_char_room,
		       bloodthirst_cb, wch) != NULL)
		return NULL;

	vo_foreach(wch->in_room, &iter_char_room, find_aggr_cb, wch);
	return NULL;
}

void
light_update(void)
{   
	CHAR_DATA *ch;
	int dam_light;
	DESCRIPTOR_DATA *d;

	for (d = descriptor_list; d != NULL; d = d->next) {
		if (d->connected != CON_PLAYING)
			continue;

		ch = (d->original != NULL) ? d->original : d->character;

		if (IS_IMMORTAL(ch))
			continue;

		/* also checks vampireness */
		if ((dam_light = isn_dark_safe(ch)) == 0) 
			continue;	

		if (dam_light != 2
		&&  number_percent() < get_skill(ch, "light resistance")) {
			check_improve(ch, "light resistance", TRUE, 32);
			continue;
		}

		if (dam_light == 1)
			char_puts("The light in the room disturbs you.\n", ch);
		else
			char_puts("Sun light disturbs you.\n",ch);

		dam_light = 1 + (ch->max_hit * 4)/ 100;
		damage(ch, ch, dam_light, NULL, DAM_LIGHT,
			DAMF_SHOW | DAMF_LIGHT_V);

		if (ch->position == POS_STUNNED)
			update_pos(ch);

		if (number_percent() < 10)
			gain_condition(ch, COND_DRUNK,  -1);
	}
}

void
room_update(void)
{   
	ROOM_INDEX_DATA *room;
	ROOM_INDEX_DATA *room_next;

	for (room = top_affected_room; room; room = room_next) {
		AFFECT_DATA *paf;
		AFFECT_DATA *paf_next;

		room_next = room->aff_next;

		for (paf = room->affected; paf != NULL; paf = paf_next) {
			paf_next = paf->next;

			if (paf->duration > 0) {
				paf->duration--;
				if (number_range(0,4) == 0 && paf->level > 0)
					paf->level--;
			} else if (paf->duration == 0) {
				check_one_event(NULL, paf, EVENT_ROOM_TIMEOUT);
				affect_remove_room(room, paf);
			}
		}
	}
}

void
check_reboot(void)
{
	DESCRIPTOR_DATA *d;

	switch(reboot_counter) {
	case -1:
		break;
	case 0:
		reboot_mud();
		return;
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 10:
	case 15:
		for (d = descriptor_list; d != NULL; d = d->next) {
			if (d->character != NULL) {
				if (rebooter || !IS_IMMORTAL(d->character)) {
					char_printf(d->character, 
						    "{*{W*****{R rEBOOT IN {W%d{R MIN. {W*****{x\n",
					    	    reboot_counter);
				} else {
					char_printf(d->character, 
						    "{*{W*****{R AUTOMAGIC rEBOOT IN {W%d{R MIN. {W*****{x\n",
					    	    reboot_counter);
				}
			}
		}

		/* FALLTHRU */
	default: 
		reboot_counter--;
		break;
	}
}

void *
track_update_cb(void *vo, va_list ap)
{   
	CHAR_DATA *ch = (CHAR_DATA *) vo;

	CHAR_DATA *victim;
	CHAR_DATA *vch, *vch_next;
	ROOM_INDEX_DATA *was_in_room;
	NPC_DATA *npc;

	if (IS_AFFECTED(ch, AFF_CALM | AFF_CHARM | AFF_SCREAM)
        ||  ch->fighting
	||  !ch->in_room
        ||  !IS_AWAKE(ch) 
	||  RIDDEN(ch))
		return NULL;

	/*
	 * track the victim
	 */
	npc = NPC(ch);
	victim = npc->target ? npc->target : npc->last_fought;

	if (victim != NULL) {
		add_mind(ch, victim->name);

        	if (!IS_SET(ch->pMobIndex->act, ACT_NOTRACK)
		&&  (was_in_room = ch->in_room) != victim->in_room) {
			dofun("track", ch, victim->name);
			if (IS_EXTRACTED(ch))
				return NULL;

			/*
			 * forget about it if we could not find
			 * the tracks and have no specific target set
			 * (summoned shadow, stalker)
			 */
			if (was_in_room == ch->in_room
			&&  npc->target == NULL
			&&  number_range(0, 19) == 0) {
				npc->last_fought = NULL;
				back_home(ch);
				return NULL;
			}
		}
	}

	/*
	 * attack the victims in mind
	 */
	if (npc->in_mind == NULL)
		return NULL;

	for (vch = ch->in_room->people; vch; vch = vch_next) {
		vch_next = vch->next_in_room;

		if (IS_IMMORTAL(vch)
		||  !can_see(ch, vch)
		||  is_safe_nomessage(ch, vch)
		||  !is_name(vch->name, npc->in_mind))
			continue;

		act_yell(ch, "So we meet again, $i!", vch, NULL);
		dofun("murder", ch, vch->name);
		break;
	}

	return NULL;
}

void
raffect_update(void)
{
	ROOM_INDEX_DATA *room;

	for (room = top_affected_room; room ; room = room->aff_next)
		vo_foreach(room, &iter_char_room, raff_update_cb, room);
}

void
clan_update(void)
{
	if (time_info.hour == 0)
		hash_foreach(&clans, clan_item_update_cb);
}

void
olc_asave_update(void)
{
	dofun("asave", NULL, "explicit");
}

void
song_update(void)
{
}

void
tip_update(void)
{
	DESCRIPTOR_DATA *d;
	tip_t *t;
	static int index;
	int nind;
	flag_t mask;
	mlstring *pml;

	do {
		nind = number_range(0, tips.nused - 1);
	} while (nind == index);
	index = nind;

	t = (tip_t *)VARR_GET(&tips, index);
	mask = t->comm;
	pml = &t->phrase;

	/* Found tip has just been created, skip it */
	if (mlstr_null(pml))
		return;

	for (d = descriptor_list; d; d = d->next) {
		CHAR_DATA *ch = d->character;

		if (!ch
		|| d->connected != CON_PLAYING
		|| !IS_SET(ch->comm, mask))
			continue;

		act_puts("{YTIP: {x", ch, NULL, NULL, TO_CHAR | ACT_NOLF, POS_DEAD);
		act_mlputs(pml, ch, NULL, NULL, TO_CHAR, POS_DEAD);
	}
}

void
popularity_update(void)
{
	CHAR_DATA *ch;

	for (ch = char_list; ch && !IS_NPC(ch); ch = ch->next)
		if (ch->in_room)
			ch->in_room->area->count++;
}

void
area_update(void)
{
	AREA_DATA *pArea;

	for (pArea = area_first; pArea != NULL; pArea = pArea->next) {
		ROOM_INDEX_DATA *pRoomIndex;

		if (++pArea->age < 3)
			continue;

		/*
		 * Check age and reset.
		 */
		if ((pArea->empty || (pArea->nplayer != 0 && pArea->age < 15))
		&&  pArea->age < 31
		&&  !IS_SET(pArea->area_flags, AREA_UPDATE_ALWAYS))
			continue;

		/*
		 * the rain devastates tracks on the ground
		 */
		if (weather_info.sky == SKY_RAINING)  {
			int i;
			DESCRIPTOR_DATA *d;
			CHAR_DATA *ch;

	 		for (d = descriptor_list; d; d = d->next)  {
				if (d->connected != CON_PLAYING)
					continue;

				ch = d->original ?  d->original : d->character;
				if (ch->in_room->area == pArea
				&&  get_skill(ch, "track") > 50
				&&  !IS_SET(ch->in_room->room_flags,
					    ROOM_INDOORS)) {
					act_puts("Rain devastates the tracks on the ground.",
						 ch, NULL, NULL, TO_CHAR, POS_DEAD);
				}
			}

			for (i = pArea->min_vnum; i < pArea->max_vnum; i++) {
				pRoomIndex = get_room_index(i);
				if (pRoomIndex == NULL
				||  IS_SET(pRoomIndex->room_flags,
								ROOM_INDOORS))
					continue;
				if (number_percent() < 50)
					room_record("erased", pRoomIndex, -1);
			}
		}

		reset_area(pArea);
		wiznet("$t has just been reset.",
			NULL, pArea->name, WIZ_RESETS, 0, 0);

		print_resetmsg(pArea);

		pArea->age = number_range(0, 3);
		if (IS_SET(pArea->area_flags, AREA_UPDATE_FREQUENTLY))
			pArea->age = 15 - 2;
		else if (pArea->nplayer == 0) 
			pArea->empty = TRUE;
	}
}

void chquest_update(void)
{
	chquest_t *q;

	for (q = chquest_list; q; q = q->next) {
		if (!IS_WAITING(q))
			continue;
		if (!--q->delay)
			chquest_startq(q);
	}
}

void quest_update(void)
{
	CHAR_DATA *ch, *ch_next;

	for (ch = char_list; ch && !IS_NPC(ch); ch = ch_next) {
		ch_next = ch->next;

		if (PC(ch)->questtime < 0) {
			if (++PC(ch)->questtime == 0) {
				char_puts("{*You may now quest again.\n", ch);
				return;
			}
		} else if (IS_ON_QUEST(ch)) {
			if (--PC(ch)->questtime == 0) {
				char_puts("You have run out of time for your quest!\n", ch);
				quest_cancel(ch);
				PC(ch)->questtime = -number_range(5, 10);
			} else if (PC(ch)->questtime < 6) {
				char_puts("Better hurry, you're almost out of time for your quest!\n", ch);
				return;
			}
		}
	}
}

void auction_update(void)
{
	if (auction.item == NULL)
		return;

	switch (++auction.going) { /* increase the going state */
	case 1 : /* going once */
	case 2 : /* going twice */
	        if (auction.bet > 0) {
			act_auction("$p: going $T for $J gold.",
				    auction.item, 
				    (auction.going == 1) ? "once" : "twice",
				    (const void*) auction.bet,
				    ACT_FORMSH, POS_RESTING);
	        } else {
	        	act_auction("$p: going $T, starting price $J gold.",
				    auction.item, 
				    (auction.going == 1) ? "once" : "twice",
				    (const void*) auction.starting,
				    ACT_FORMSH, POS_RESTING);
		}
	        break;

	 case 3 : /* SOLD! */
	        if (auction.bet > 0) {
			int tax;
			int pay;

	        	act_auction("$p: sold to $N for $J gold.",
				    auction.item, auction.buyer,
				    (const void*) auction.bet,
				    ACT_FORMSH, POS_RESTING);

			auction_give_obj(auction.buyer);

			pay = (auction.bet * 85) / 100;
			tax = auction.bet - pay;

			 /* give him the money */
			act_puts3("The auctioneer pays you $j gold, "
				  "charging an auction fee of $J gold.",
				  auction.seller, (const void*) pay,
				  NULL, (const void*) tax, TO_CHAR, POS_DEAD);
			PC(auction.seller)->bank_g += pay;
		} else {
			/* not sold */
	        	act_auction("No bets received for $p.",
				    auction.item, NULL, NULL,
				    ACT_FORMSH, POS_RESTING);
			act_auction("Object has been removed from auction.",
				    NULL, NULL, NULL,
				    ACT_FORMSH, POS_RESTING);
			auction_give_obj(auction.seller);
	        }
        }
} 

/*----------------------------------------------------------------------------
 * locals
 */

static void *
check_assist_cb(void *vo, va_list ap)
{
	CHAR_DATA *rch = (CHAR_DATA *) vo;

	CHAR_DATA *ch;
	CHAR_DATA *victim;

	if (!IS_AWAKE(rch) || rch->fighting != NULL)
		return NULL;

	ch = va_arg(ap, CHAR_DATA *);
	victim = va_arg(ap, CHAR_DATA *);

	/*
	 * ASSIST_PLAYERS mobs
	 */
	if (IS_NPC(rch) && !IS_NPC(ch)
	&&  IS_SET(rch->pMobIndex->off_flags, ASSIST_PLAYERS)
	&&  rch->level + 6 > victim->level) {
		dofun("emote", rch, "screams and attacks!");
		multi_hit(rch, victim, NULL);
		return NULL;
	}

	/*
	 * charmed chars or PCs with PLR_AUTOASSIST
	 */
	if (((!IS_NPC(rch) && IS_SET(PC(rch)->plr_flags, PLR_AUTOASSIST)) ||
	     IS_AFFECTED(rch, AFF_CHARM) ||
	     IS_NPC(rch))
	&&  is_same_group(ch, rch)
	&&  !is_safe_nomessage(rch, victim)) {
		multi_hit (rch, victim, NULL);
		return NULL;
	}

	if (!IS_NPC(ch)) {
		if (RIDDEN(rch) == ch)
			multi_hit(rch, victim, NULL);
		return NULL;
	}

	/* that's all for !IS_NPC */
	if (!IS_NPC(rch))
		return NULL;

	if (IS_SET(rch->pMobIndex->off_flags, ASSIST_ALL)
	||  (IS_SET(rch->pMobIndex->off_flags, ASSIST_RACE) &&
	     IS_RACE(rch->race, ch->race))
	||  (rch->pMobIndex == ch->pMobIndex &&
	     IS_SET(rch->pMobIndex->off_flags, ASSIST_VNUM))
	||  (IS_SET(rch->pMobIndex->off_flags, ASSIST_ALIGN) &&
	     NALIGN(rch) == NALIGN(ch))) {
		int number = 0;
		CHAR_DATA *target = NULL;
		CHAR_DATA *vch;

		if (number_bits(1) == 0)
			return NULL;

		for (vch = ch->in_room->people; vch; vch = vch->next_in_room) {
			if (can_see(rch, vch)
			&&  is_same_group(vch, victim)
			&&  number_range(0, number) == 0) {
				target = vch;
				number++;
			}
		}

		if (target != NULL) {
			dofun("emote", rch, "screams and attacks!");
			multi_hit(rch, target, NULL);
		}
	}

	return NULL;
}

static int
potion_cure_level(OBJ_DATA *potion)
{
	int cl = 0;
	int i;

	for (i = 1; i < 5;i++) {
		if (IS_SKILL(potion->value[i].s, "cure critical"))
			cl += 3;
		if (IS_SKILL(potion->value[i].s, "cure light"))
			cl += 1;
		if (IS_SKILL(potion->value[i].s, "cure serious"))
			cl += 2;
		if (IS_SKILL(potion->value[i].s, "heal"))
			cl += 4;
	}

	return cl;
}

static int
potion_arm_level(OBJ_DATA *potion)
{
	int al = 0;
	int i;

	for (i = 1; i < 5; i++) {
		if (IS_SKILL(potion->value[i].s, "armor"))
			al += 1;
		if (IS_SKILL(potion->value[i].s, "shield"))
			al += 1;
		if (IS_SKILL(potion->value[i].s, "stone skin"))
			al += 2;
		if (IS_SKILL(potion->value[i].s, "sanctuary"))
			al += 4;
		if (IS_SKILL(potion->value[i].s, "protection"))
			al += 3;
	}
	return al;
}

static bool
is_potion(OBJ_DATA *potion, const char *sn)
{
	int i;
	for (i = 1; i < 5; i++) {
		if (IS_SKILL(potion->value[i].s, sn))
			return TRUE;
	}
	return FALSE;
}

static int
hit_gain(CHAR_DATA *ch)
{
	int gain;
	int number;

	if (IS_NPC(ch)) {
		gain =  5 + ch->level;
 		if (IS_AFFECTED(ch, AFF_REGENERATION))
			gain *= 2;

		switch(ch->position) {
		default:		gain /= 2;		break;
		case POS_SLEEPING:	gain = 3 * gain/2;	break;
		case POS_RESTING:				break;
		case POS_FIGHTING:	gain /= 3;		break;
 		}
	} else {
		class_t *cl;

		if ((cl = class_lookup(ch->class)) == NULL)
			return 0;

		gain = UMAX(3, 2 * get_curr_stat(ch, STAT_CON) +
			       (7 * ch->level) / 4); 
		gain = (gain * cl->hp_rate) / 100;
 		number = number_percent();
		if (number < get_skill(ch, "fast healing")) {
			gain += number * gain / 100;
			if (ch->hit < ch->max_hit)
				check_improve(ch, "fast healing", TRUE, 8);
		}

		if (number < get_skill(ch, "trance")) {
			gain += number * gain / 150;
			if (ch->hit < ch->max_hit)
				check_improve(ch, "trance", TRUE, 8);
		}

		switch (ch->position) {
		default:		gain /= 4;		break;
		case POS_SLEEPING: 				break;
		case POS_RESTING:	gain /= 2;		break;
		case POS_FIGHTING:	gain /= 6;		break;
		}

		if (PC(ch)->condition[COND_HUNGER]   < 0)
			gain = 0;

		if (PC(ch)->condition[COND_THIRST] < 0)
			gain = 0;
	}

	gain = gain * GET_HEAL_RATE(ch->in_room) / 100;
	
	if (ch->on != NULL && ch->on->item_type == ITEM_FURNITURE)
		gain = gain * INT(ch->on->value[3]) / 100;

	if (IS_AFFECTED(ch, AFF_POISON))
		gain /= 4;

	if (IS_AFFECTED(ch, AFF_PLAGUE))
		gain /= 8;

	if (IS_AFFECTED(ch, AFF_HASTE))
		gain /= 2;

	if (IS_AFFECTED(ch, AFF_SLOW))
		gain *= 2;

	if (get_curr_stat(ch, STAT_CON) > 20)
		gain = (gain * 14) / 10;

	if (IS_HARA_KIRI(ch))
		gain *= 3;

	return UMIN(gain, ch->max_hit - ch->hit);
}

static int
mana_gain(CHAR_DATA *ch)
{
	int gain;
	int number;

	if (IS_NPC(ch)) {
		gain = 5 + ch->level;
		switch (ch->position) {
		default:		gain /= 2;		break;
		case POS_SLEEPING:	gain = 3 * gain/2;	break;
		case POS_RESTING:				break;
		case POS_FIGHTING:	gain /= 3;		break;
		}
	} else {
		class_t *cl;

		if ((cl = class_lookup(ch->class)) == NULL)
			return 0;

		gain = get_curr_stat(ch, STAT_WIS) +
		       (2 * get_curr_stat(ch, STAT_INT)) + ch->level;
		gain = (gain * cl->mana_rate) / 100;
		number = number_percent();
		if (number < get_skill(ch, "meditation")) {
			gain += number * gain / 100;
			if (ch->mana < ch->max_mana)
				check_improve(ch, "meditation", TRUE, 8);
		}

		if (number < get_skill(ch, "trance")) {
			gain += number * gain / 100;
			if (ch->mana < ch->max_mana)
				check_improve(ch, "trance", TRUE, 8);
		}

		if (!IS_SET(cl->class_flags, CLASS_MAGIC))
			gain /= 2;

		switch (ch->position) {
		default:		gain /= 4;	break;
		case POS_SLEEPING: 			break;
		case POS_RESTING:	gain /= 2;	break;
		case POS_FIGHTING:	gain /= 6;	break;
		}

		if (PC(ch)->condition[COND_HUNGER] < 0
		||  PC(ch)->condition[COND_THIRST] < 0)
			gain = 0;
	}

	gain = gain * GET_MANA_RATE(ch->in_room) / 100;

	if (ch->on != NULL && ch->on->item_type == ITEM_FURNITURE)
		gain = gain * INT(ch->on->value[4]) / 100;

	if (IS_AFFECTED(ch, AFF_POISON))
		gain /= 4;

	if (IS_AFFECTED(ch, AFF_PLAGUE))
		gain /= 8;

	if (IS_AFFECTED(ch, AFF_HASTE))
		gain /= 2;

	if (IS_AFFECTED(ch, AFF_SLOW))
		gain *= 2;

	if (get_curr_stat(ch, STAT_INT) > 20)
		gain = (gain * 13) / 10;
	if (get_curr_stat(ch, STAT_WIS) > 20)
		gain = (gain * 11) / 10;
	if (IS_HARA_KIRI(ch))
		gain *= 3;
	if (is_affected(ch, "lich"))
		gain -= 2*ch->level;

	return UMIN(gain, ch->max_mana - ch->mana);
}

static int
move_gain(CHAR_DATA *ch)
{
	int gain;

	if (ch->in_room == NULL)
		return 0;

	if (IS_NPC(ch))
		gain = ch->level;
	else {
		gain = UMAX(15, 2 * ch->level);

		switch (ch->position) {
		case POS_SLEEPING:
			gain += 2 * get_curr_stat(ch, STAT_DEX);
			break;

		case POS_RESTING:
			gain += get_curr_stat(ch, STAT_DEX);
			break;
		}

		if (PC(ch)->condition[COND_HUNGER] < 0
		||  PC(ch)->condition[COND_THIRST] < 0)
			gain = 3;
	}

	gain = gain * GET_HEAL_RATE(ch->in_room) / 100;

	if (ch->on != NULL && ch->on->item_type == ITEM_FURNITURE)
		gain = gain * INT(ch->on->value[3]) / 100;

	if (IS_AFFECTED(ch, AFF_POISON))
		gain /= 4;

	if (IS_AFFECTED(ch, AFF_PLAGUE))
		gain /= 8;

	if (IS_AFFECTED(ch, AFF_HASTE | AFF_SLOW))
		gain /= 2;

	if (get_curr_stat(ch, STAT_DEX) > 20)
		gain = gain * 14 / 10;

	if (IS_HARA_KIRI(ch))
		gain *= 3;

	return UMIN(gain, ch->max_move - ch->move);
}

static void
update_obj_affects(OBJ_DATA *obj)
{
	AFFECT_DATA *paf, *paf_next;

	/* go through affects and decrement */
	for (paf = obj->affected; paf != NULL; paf = paf_next) {
		paf_next    = paf->next;
		if (paf->duration > 0) {
        		paf->duration--;
			/* spell strength fades with time */
        		if (number_range(0,4) == 0 && paf->level > 0)
				paf->level--;
        	} else if (paf->duration == 0) {
			skill_t *sk;

			if ((paf_next == NULL || paf_next->type != paf->type ||
			     paf_next->duration > 0)
			&&  paf->type > 0
			&&  (sk = skill_lookup(paf->type)) != NULL
			&&  !mlstr_null(&sk->msg_obj)) {
				if (obj->carried_by != NULL) {
					act_mlputs(&sk->msg_obj,
						   obj->carried_by, obj, NULL,
						   TO_CHAR, POS_DEAD);
				}

				if (obj->in_room != NULL 
				&&  obj->in_room->people) {
					act_mlputs(&sk->msg_obj,
						   obj->in_room->people, obj,
						   NULL, TO_ALL, POS_DEAD);
				}
                	}
			affect_remove_obj(obj, paf);
        	}
	}
}

static bool
update_melt_obj(OBJ_DATA *obj)
{
	if (obj->carried_by != NULL
	&&  obj->carried_by->in_room->sector_type == SECT_DESERT
	&&  number_percent() < 40) {
		act("The extreme heat melts $p.", obj->carried_by, obj, NULL,
		    TO_CHAR);
		extract_obj(obj, 0);
		return TRUE;
	}

	if (obj->in_room != NULL
	&&  obj->in_room->sector_type == SECT_DESERT
	&&  number_percent() < 50) {
		act("The extreme heat melts $p.", obj->in_room->people, obj,
		    NULL, TO_ALL);
		extract_obj(obj, 0);
		return TRUE;
	}

	return FALSE;
}

static bool
update_potion(OBJ_DATA *obj)
{
	if (obj->carried_by
	&&  obj->carried_by->in_room->sector_type == SECT_DESERT
	&&  !IS_NPC(obj->carried_by)
	&&  number_percent() < 20)  {
		act("$p evaporates.", obj->carried_by, obj, NULL, TO_CHAR);
		extract_obj(obj, 0);
		return TRUE;
	}

	if (obj->in_room
	&&  obj->in_room->sector_type == SECT_DESERT
	&&  number_percent() < 30) {
		act("$p evaporates by the extreme heat.", obj->in_room->people,
		    obj, NULL, TO_ALL);
		extract_obj(obj, 0);
		return TRUE;
	}

	return FALSE;
}

static bool
update_drinkcon(OBJ_DATA *obj)
{
	if (INT(obj->value[1]) == 0)
		return FALSE;		/* empty */

	if (obj->carried_by
	&&  INT(obj->value[1]) > 0
	&&  obj->carried_by->in_room->sector_type == SECT_DESERT
	&&  !IS_NPC(obj->carried_by)
	&&  number_percent() < 20)  {
		act("Liquid in $p evaporates.",
		    obj->carried_by, obj, NULL, TO_CHAR);
		INT(obj->value[1]) = 0;
		return TRUE;
	}

	if (obj->in_room
	&&  INT(obj->value[1]) > 0
	&&  obj->in_room->sector_type == SECT_DESERT
	&&  number_percent() < 30) {
		act("Liquid in $p evaporates.",
		    obj->in_room->people, obj, NULL, TO_ALL);
		INT(obj->value[1]) = 0;
		return TRUE;
	}

	return FALSE;
}

static inline void
contents_to_obj(OBJ_DATA *obj, OBJ_DATA *to_obj)
{
	OBJ_DATA *obj_next;

	for (; obj; obj = obj_next) {
		obj_next = obj->next_content;
		obj_from_obj(obj);
		obj_to_obj(obj, to_obj);
	}
}

static inline void
save_corpse_contents(OBJ_DATA *corpse)
{
	OBJ_DATA *obj, *obj_next;
	OBJ_DATA *pit;
	altar_t *altar;

/* in another object */
	if (corpse->in_obj) {
		contents_to_obj(corpse->contains, corpse->in_obj);
		return;
	}

/* carried by */
	if (corpse->carried_by) {
		for (obj = corpse->contains; obj; obj = obj_next) {
			obj_next = obj->next_content;
			obj_from_obj(obj);
			obj_to_char(obj, corpse->carried_by);
		}
		return;
	}

/* pit lookup */
	pit = NULL;
	if ((altar = corpse->altar)) {
		for (pit = altar->room->contents; pit; pit = pit->next_content)
			if (pit->pObjIndex == altar->pit)
				break;
	} else {
		log(LOG_INFO, "save_corpse_contents: null altar (owner: %s)",
			   mlstr_mval(&corpse->owner));
	}

/* put contents into altar */
	if (!pit) {
		for (obj = corpse->contains; obj; obj = obj_next) {
			obj_next = obj->next_content;
			obj_from_obj(obj);
			obj_to_room(obj, altar->room);
		}
		return;
	}

/* put contents into pit */
	contents_to_obj(corpse->contains, pit);
}

static void *
find_aggr_cb(void *vo, va_list ap)
{
	CHAR_DATA *ch = (CHAR_DATA *) vo;

	CHAR_DATA *wch;
	CHAR_DATA *vch;
	flag_t act;
	NPC_DATA *npc;
	int count;
	CHAR_DATA *victim;

	if (!IS_NPC(ch))
		return NULL;

	wch = va_arg(ap, CHAR_DATA *);
	npc = NPC(ch);
	act = ch->pMobIndex->act;

	if ((!IS_SET(act, ACT_AGGRESSIVE) &&
	     npc->last_fought == NULL &&
	     npc->target == NULL)
	||  IS_SET(ch->in_room->room_flags, ROOM_PEACE | ROOM_SAFE)
	||  IS_AFFECTED(ch, AFF_CALM)
	||  ch->fighting != NULL
	||  RIDDEN(ch)
	||  IS_AFFECTED(ch, AFF_CHARM)
	||  !IS_AWAKE(ch)
	||  (IS_SET(act, ACT_WIMPY) && IS_AWAKE(wch))
	||  !can_see(ch, wch) 
	||  number_bits(1) == 0
	||  is_safe_nomessage(ch, wch))
		return NULL;

	if (npc->target != NULL) {
		if (npc->target == wch)
			multi_hit(ch, wch, NULL);
		return NULL;
	}

	if (npc->last_fought != NULL) {
		if (npc->last_fought == wch
		&&  !IS_AFFECTED(ch, AFF_SCREAM | AFF_CALM)) {
			act_yell(ch, "$i! Now you die!", wch, NULL);
			wch = check_guard(wch, ch); 
			multi_hit(ch, wch, NULL);
		}
		return NULL;
	}

	/*
	 * Ok we have a 'wch' player character and a 'ch' npc aggressor.
	 * Now make the aggressor fight a RANDOM pc victim in the room,
	 * giving each 'vch' an equal chance of selection.
	 */
	count = 0;
	victim = NULL;
	for (vch = wch->in_room->people; vch != NULL; vch = vch->next_in_room) {
		if (!IS_NPC(vch)
		&&  vch->level < LEVEL_IMMORTAL
		&&  ch->level >= vch->level - 5 
		&&  (!IS_SET(act, ACT_WIMPY) || !IS_AWAKE(vch))
		&&  can_see(ch, vch)
		&&  !is_safe_nomessage(ch, vch)
		/* do not attack vampires */
		&&  !IS_VAMPIRE(vch)
		/* good vs good :( */
		&&  !(IS_GOOD(ch) && IS_GOOD(vch))) {
			if (number_range(0, count++) == 0)
				victim = vch;
		}
	}

	if (victim == NULL)
		return NULL;

	victim = check_guard(victim, ch); 
	if (get_skill(ch, "backstab")) {
		dofun("backstab", ch, victim->name);
		if (IS_EXTRACTED(victim) || ch->fighting != NULL)
			return NULL;
	}
	multi_hit(ch, victim, NULL);
	return NULL;
}

static void *
raff_update_cb(void *vo, va_list ap)
{
	ROOM_INDEX_DATA *room = va_arg(ap, ROOM_INDEX_DATA *);
	check_events(vo, room->affected, EVENT_ROOM_UPDATE);
	return NULL;
}

static void *
put_back_cb(void *p, va_list ap)
{
	clan_t *clan = (clan_t *) p;

	OBJ_DATA *obj = va_arg(ap, OBJ_DATA *);

	if (obj->in_room->vnum == clan->altar_vnum)
		return p;

	return NULL;
}

static void *
clan_item_update_cb(void *p, va_list ap)
{
	clan_t *clan = (clan_t *) p;

	OBJ_DATA *obj;

	if (clan->obj_ptr == NULL) 
		return NULL;

	if (clan->altar_ptr == NULL) {
		log(LOG_ERROR, "clan_item_update_cb: clan %s: no altar_ptr", clan->name);
		return NULL;
	}

	for (obj = clan->obj_ptr; obj->in_obj; obj = obj->in_obj)
		;

	/*
	 * do not move clan items which are in altars
	 */
	if (obj->in_room
	&&  hash_foreach(&clans, put_back_cb, obj) != NULL)
		return NULL;

	if (clan->obj_ptr->in_obj)
		obj_from_obj(clan->obj_ptr);
	if (clan->obj_ptr->carried_by)
		obj_from_char(clan->obj_ptr);
	if (clan->obj_ptr->in_room)
		obj_from_room(clan->obj_ptr);

	obj_to_obj(clan->obj_ptr, clan->altar_ptr);
	return NULL;
}

static void
print_resetmsg(AREA_DATA *pArea)
{
	DESCRIPTOR_DATA *d;
	bool is_empty = mlstr_null(&pArea->resetmsg);
	
	for (d = descriptor_list; d != NULL; d = d->next) {
		CHAR_DATA *ch;

		if (d->connected != CON_PLAYING)
			continue;

		ch = d->original ? d->original : d->character;
		if (IS_NPC(ch) || !IS_AWAKE(ch) || ch->in_room->area != pArea)
			continue;

		if (is_empty)
			act_puts("You hear some squeaking sounds...",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		else
			act_puts(mlstr_cval(&pArea->resetmsg, ch),
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
	}
}

void *
bloodthirst_cb(void *vo, va_list ap)
{
	CHAR_DATA *vch = (CHAR_DATA *) vo;
	CHAR_DATA *ch;

	if (IS_IMMORTAL(vch))
		return NULL;

	ch = va_arg(ap, CHAR_DATA *);
	if (ch != vch
	&&  can_see(ch, vch)
	&&  !is_safe_nomessage(ch, vch)) {
		dofun("yell", ch, "BLOOD! I NEED BLOOD!");
		dofun("murder", ch, vch->name);
		if (IS_EXTRACTED(ch)
		||  ch->fighting != NULL)
			return vch;
	}

	return NULL;
}

