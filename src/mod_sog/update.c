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
 * $Id: update.c,v 1.203 2001-08-21 07:02:08 fjoe Exp $
 */

#include <stdarg.h>
#include <stdio.h>
#include <dlfcn.h>

#include <merc.h>
#include <module.h>

#include <sog.h>

#include <update.h>
#include "update_impl.h"

DECLARE_MODINIT_FUN(_module_boot);

static void *uhandler_load_cb(void *p, va_list ap);
static void *uhandler_unload_cb(void *p, va_list ap);
static void *uhandler_tick_cb(void *p, va_list ap);

MODINIT_FUN(_module_boot, m)
{
	int hour, day, month;

	/*
	 * Set time and weather.
	 */
	hour = (current_time - 650336715) / (get_pulse("char") / PULSE_PER_SCD);
	time_info.hour	= hour  % 24;
	day		= hour  / 24;
	time_info.day	= day   % 35;
	month		= day   / 35;
	time_info.month	= month % 17;
	time_info.year	= month / 17;

	     if (time_info.hour <  5) weather_info.sunlight = SUN_DARK;
	else if (time_info.hour <  6) weather_info.sunlight = SUN_RISE;
	else if (time_info.hour < 19) weather_info.sunlight = SUN_LIGHT;
	else if (time_info.hour < 20) weather_info.sunlight = SUN_SET;
	else                          weather_info.sunlight = SUN_DARK;

	weather_info.change	= 0;
	weather_info.mmhg	= 960;
	if (time_info.month >= 7 && time_info.month <= 12)
		weather_info.mmhg += number_range(1, 50);
	else
		weather_info.mmhg += number_range(1, 80);

	     if (weather_info.mmhg <=  980) weather_info.sky = SKY_LIGHTNING;
	else if (weather_info.mmhg <= 1000) weather_info.sky = SKY_RAINING;
	else if (weather_info.mmhg <= 1020) weather_info.sky = SKY_CLOUDY;
	else                                weather_info.sky = SKY_CLOUDLESS;

	scan_pfiles();
	update_one("area");

	return 0;
}

void
uhandler_load(const char *mod_name)
{
	module_t *m = mod_lookup(mod_name);
	if (!m) {
		log(LOG_ERROR, "%s: %s: unknown module",
		    __FUNCTION__, mod_name);
		return;
	}
	hash_foreach(&uhandlers, uhandler_load_cb, m, m->mod_id);
}

void
uhandler_unload(const char *mod_name)
{
	module_t *m = mod_lookup(mod_name);
	if (!m)
		return;
	hash_foreach(&uhandlers, uhandler_unload_cb, m->mod_id);
}

void
update_handler(void)
{
	hash_foreach(&uhandlers, uhandler_tick_cb);
}

int
get_pulse(const char *hdlr_name)
{
	uhandler_t *hdlr = uhandler_lookup(hdlr_name);

	if (!hdlr) {
		log(LOG_BUG, "get_pulse: %s: unknown update handler",
		    hdlr_name);
		return 0;
	}

	return hdlr->ticks;
}

void *
update_one(const char *hdlr_name)
{
	uhandler_t *hdlr = uhandler_search(hdlr_name);
	if (hdlr)
		uhandler_update(hdlr);
	return hdlr;
}

void
update_reset(const char *hdlr_name)
{
	uhandler_t *hdlr = uhandler_lookup(hdlr_name);
	if (!hdlr)
		return;
	hdlr->cnt = hdlr->ticks;
}

void
gain_condition(CHAR_DATA *ch, int iCond, int value)
{
	int condition;
	int damage_hunger;

	if (value == 0 || IS_NPC(ch) || ch->level >= LEVEL_IMMORTAL)
		return;

	if (IS_VAMPIRE(ch)
	&&  (iCond == COND_THIRST ||
	     iCond == COND_FULL ||
	     iCond == COND_HUNGER))
		return;

	condition = PC(ch)->condition[iCond];

	PC(ch)->condition[iCond] = URANGE(-6, condition + value, 96);

	if (iCond == COND_FULL && (PC(ch)->condition[COND_FULL] < 0))
		PC(ch)->condition[COND_FULL] = 0;

	if ((iCond == COND_DRUNK) && (PC(ch)->condition[COND_DRUNK] < 1)) 
		PC(ch)->condition[COND_DRUNK] = 0;

	if (PC(ch)->condition[iCond] < 1
	&&  PC(ch)->condition[iCond] > -6) {
		switch (iCond) {
		case COND_HUNGER:
			act_char("You are hungry.", ch);
			break;

		case COND_THIRST:
			act_char("You are thirsty.", ch);
			break;

		case COND_DRUNK:
			if (condition != 0)
				act_char("You are sober.", ch);
			break;

		case COND_BLOODLUST:
			if (condition != 0)
				act_char("You are hungry for blood.", ch);
			break;

		case COND_DESIRE:
			if (condition != 0)
				act_char("You are missing your home.", ch);
			break;
		}
	}

	if (PC(ch)->condition[iCond] == -6 && ch->level >= LEVEL_PK) {
		switch (iCond) {
		case COND_HUNGER:
			act_char("You are starving!", ch);
			act("$n is starving!",  ch, NULL, NULL, TO_ROOM);
			damage_hunger = ch->max_hit * number_range(2, 4) / 100;
			if (!damage_hunger)
				damage_hunger = 1;
			damage(ch, ch, damage_hunger, NULL,
			       DAM_NONE, DAMF_SHOW | DAMF_HUNGER);
			if (ch->position == POS_SLEEPING)
				return;
			break;

		case COND_THIRST:
			act_char("You are dying of thirst!", ch);
			act("$n is dying of thirst!", ch, NULL, NULL, TO_ROOM);
			damage_hunger = ch->max_hit * number_range(2, 4) / 100;
			if (!damage_hunger)
				damage_hunger = 1;
			damage(ch, ch, damage_hunger, NULL,
				DAM_NONE, DAMF_SHOW | DAMF_THIRST);
			if (ch->position == POS_SLEEPING)
				return;
			break;

		case COND_BLOODLUST:
			act_char("You are suffering from thirst of blood!", ch);
			act("$n is suffering from thirst of blood!",
			    ch, NULL, NULL, TO_ROOM);
			if (ch->in_room && ch->in_room->people
			&&  ch->fighting == NULL) {
				if (!IS_AWAKE(ch))
					dofun("stand", ch, str_empty);
				vo_foreach(ch->in_room, &iter_char_room,
					   bloodthirst_cb, ch);
				if (ch->fighting != NULL)
					break;
			}

			damage_hunger = ch->max_hit * number_range(2, 4) / 100;
			if (!damage_hunger)
				damage_hunger = 1;
			damage(ch, ch, damage_hunger, NULL,
				DAM_NONE, DAMF_SHOW | DAMF_THIRST);
			if (ch->position == POS_SLEEPING)
				return;
			break;

		case COND_DESIRE:
			act_char("You want to go your home!", ch);
			act("$n desires for $s home!", ch, NULL, NULL, TO_ROOM);
			if (ch->position >= POS_STANDING)
				move_char(ch, number_door(), 0);
			break;
		}
	}
}

/*--------------------------------------------------------------------
 * static functions
 */

static void *
uhandler_load_cb(void *p, va_list ap)
{
	uhandler_t *hdlr = (uhandler_t *) p;
	module_t *m = va_arg(ap, module_t*);
	int mod = va_arg(ap, int);

	if (mod == hdlr->mod) {
		hdlr->fun = dlsym(m->dlh, hdlr->fun_name);
		if (hdlr->fun == NULL)
			log(LOG_ERROR, "update_load: %s", dlerror());
	}

	return NULL;
}

static
FOREACH_CB_FUN(uhandler_unload_cb, p, ap)
{
	uhandler_t *hdlr = (uhandler_t *) p;
	int mod = va_arg(ap, int);

	if (mod == hdlr->mod)
		hdlr->fun = NULL;

	return NULL;
}

static
FOREACH_CB_FUN(uhandler_tick_cb, p, ap)
{
	uhandler_t *hdlr = (uhandler_t *) p;

	if (--hdlr->cnt == 0) {
		hdlr->cnt = hdlr->ticks;
		uhandler_update(hdlr);
	}

	return NULL;
}
