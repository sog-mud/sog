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
 * $Id: update.c,v 1.190 2000-06-01 17:57:51 fjoe Exp $
 */

#include <stdarg.h>
#include <stdio.h>
#include <dlfcn.h>

#include "merc.h"
#include "module.h"

#include "fight.h"
#include "_update.h"

static void *uhandler_load_cb(void *p, va_list ap);
static void *uhandler_unload_cb(void *p, va_list ap);
static void *uhandler_tick_cb(void *p, va_list ap);

void *
uhandler_load(const char *mod_name)
{
	module_t *m = mod_lookup(mod_name);
	if (!m)
		return NULL;
	hash_foreach(&uhandlers, uhandler_load_cb, m, m->mod_id);
	return NULL;
}

void *
uhandler_unload(const char *mod_name)
{
	module_t *m = mod_lookup(mod_name);
	if (!m)
		return NULL;
	hash_foreach(&uhandlers, uhandler_unload_cb, m->mod_id);
	return NULL;
}

void *
update_handler(void)
{
	log(LOG_INFO, "update_handler");
	hash_foreach(&uhandlers, uhandler_tick_cb);
	return NULL;
}

void *
update_one(const char *hdlr_name)
{
	uhandler_t *hdlr = uhandler_search(hdlr_name);
	if (!hdlr)
		return NULL;
	uhandler_update(hdlr);
	return hdlr;
}

void *
update_reset(const char *hdlr_name)
{
	uhandler_t *hdlr = uhandler_lookup(hdlr_name);
	if (!hdlr)
		return NULL;
	hdlr->cnt = hdlr->ticks;
	return NULL;
}

int
get_pulse(const char *hdlr_name)
{
	uhandler_t *hdlr = uhandler_lookup(hdlr_name);

	if (!hdlr) {
		log(LOG_ERROR, "get_pulse: %s: unknown update handler",
		    hdlr_name);
		return 0;
	}

	return hdlr->ticks;
}

void *
gain_condition(CHAR_DATA *ch, int iCond, int value)
{
	int condition;
	int damage_hunger;

	if (value == 0 || IS_NPC(ch) || ch->level >= LEVEL_IMMORTAL)
		return NULL;

	if (IS_VAMPIRE(ch)
	&&  (iCond == COND_THIRST ||
	     iCond == COND_FULL ||
	     iCond == COND_HUNGER))
		return NULL;

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
			char_puts("You are hungry.\n",  ch);
			break;

		case COND_THIRST:
			char_puts("You are thirsty.\n", ch);
			break;
	 
		case COND_DRUNK:
			if (condition != 0)
				char_puts("You are sober.\n", ch);
			break;

		case COND_BLOODLUST:
			if (condition != 0)
				char_puts("You are hungry for blood.\n",
					     ch);
			break;

		case COND_DESIRE:
			if (condition != 0)
				char_puts("You have missed your home.\n",
					     ch);
			break;
		}
	}

	if (PC(ch)->condition[iCond] == -6 && ch->level >= LEVEL_PK) {
		switch (iCond) {
		case COND_HUNGER:
			char_puts("You are starving!\n",  ch);
			act("$n is starving!",  ch, NULL, NULL, TO_ROOM);
			damage_hunger = ch->max_hit * number_range(2, 4) / 100;
			if (!damage_hunger)
				damage_hunger = 1;
			damage(ch, ch, damage_hunger, NULL,
			       DAM_NONE, DAMF_SHOW | DAMF_HUNGER);
			if (ch->position == POS_SLEEPING) 
				return NULL; 
			break;

		case COND_THIRST:
			char_puts("You are dying of thrist!\n", ch);
			act("$n is dying of thirst!", ch, NULL, NULL, TO_ROOM);
			damage_hunger = ch->max_hit * number_range(2, 4) / 100;
			if (!damage_hunger)
				damage_hunger = 1;
			damage(ch, ch, damage_hunger, NULL,
				DAM_NONE, DAMF_SHOW | DAMF_THIRST);
			if (ch->position == POS_SLEEPING) 
				return NULL; 
			break;

		case COND_BLOODLUST:
			char_puts("You are suffering from thrist of blood!\n",
				  ch);
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
				return NULL; 
			break;

		case COND_DESIRE:
			char_puts("You want to go your home!\n", ch);
			act("$n desires for $s home!", ch, NULL, NULL, TO_ROOM);
			if (ch->position >= POS_STANDING) 
				move_char(ch, number_door(), FALSE);
			break;
		}
	}

	return NULL;
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

static void *
uhandler_unload_cb(void *p, va_list ap)
{
	uhandler_t *hdlr = (uhandler_t *) p;
	int mod = va_arg(ap, int);

	if (mod == hdlr->mod)
		hdlr->fun = NULL;

	return NULL;
}

static void *
uhandler_tick_cb(void *p, va_list ap)
{
	uhandler_t *hdlr = (uhandler_t *) p;

	if (--hdlr->cnt == 0) {
		hdlr->cnt = hdlr->ticks;
		uhandler_update(hdlr);
	}

	return NULL;
}

