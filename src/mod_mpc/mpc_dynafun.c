/*-
 * Copyright (c) 2001 SoG Development Team
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
 * $Id: mpc_dynafun.c,v 1.17 2001-09-12 19:42:56 fjoe Exp $
 */

#include <stdio.h>

#if !defined(MPC)

#include <merc.h>

#include "mpc_dynafun.h"

static
FOREACH_CB_FUN(has_sp_cb, p, ap)
{
	const char **pspn = (const char **) p;

	const char *spn = va_arg(ap, const char *);
	const char *spn_rm = va_arg(ap, const char *);

	if (!str_cmp(*pspn, spn_rm))
		return NULL;

	if (is_name_strict(*pspn, spn))
		return p;

	return NULL;
}

int
has_sp(CHAR_DATA *ch, const char *spn, const char *spn_rm, const char *spn_add)
{
	if (is_name_strict(spn_add, spn))
		return TRUE;

	return !!c_foreach(&PC(ch)->specs, has_sp_cb, spn, spn_rm);
}

static
FOREACH_CB_FUN(spclass_count_cb, p, ap)
{
	const char **pspn = (const char **) p;

	int spclass = va_arg(ap, int);
	const char *spn_rm = va_arg(ap, const char *);
	int *pcount = va_arg(ap, int *);

	spec_t *spec;

	if (!str_cmp(*pspn, spn_rm))
		return NULL;

	if ((spec = spec_lookup(*pspn)) != NULL
	&&  spec->spec_class == spclass)
		(*pcount)++;

	return NULL;
}

int
spclass_count(CHAR_DATA *ch, const char *spclass_name,
	      const char *spn_rm, const char *spn_add)
{
	int count = 0;
	int spclass;
	spec_t *spec;

	if ((spclass = flag_svalue(spec_classes, spclass_name)) < 0)
		return 0;

	if ((spec = spec_lookup(spn_add)) != NULL
	&&  spec->spec_class == spclass)
		count++;

	c_foreach(&PC(ch)->specs, spclass_count_cb, spclass, spn_rm, &count);
	return count;
}

bool
is_immortal(CHAR_DATA *ch)
{
	return IS_IMMORTAL(ch);
}

int
char_sex(CHAR_DATA *ch)
{
	return flag_value(sex_table, mlstr_mval(&ch->gender));
}

bool
is_ghost(CHAR_DATA *ch)
{
	return !IS_NPC(ch) && IS_SET(PC(ch)->plr_flags, PLR_GHOST);
}

bool
is_good(CHAR_DATA *ch)
{
	return IS_GOOD(ch);
}

bool
is_neutral(CHAR_DATA *ch)
{
	return IS_NEUTRAL(ch);
}

bool
is_evil(CHAR_DATA *ch)
{
	return IS_EVIL(ch);
}

ROOM_INDEX_DATA *
char_room(CHAR_DATA *ch)
{
	return ch->in_room;
}

int
char_level(CHAR_DATA *ch)
{
	return LEVEL(ch);
}

int
real_char_level(CHAR_DATA *ch)
{
	return LEVEL(ch);
}

void
set_weapon_dice_type(OBJ_DATA *obj, int dice_type)
{
	if (obj->pObjIndex->item_type != ITEM_WEAPON)
		return;

	INT(obj->value[2]) = dice_type;
}

CHAR_DATA *
char_fighting(CHAR_DATA *ch)
{
	return ch->fighting;
}

void
set_obj_level(OBJ_DATA *obj, int level)
{
	obj->level = level;
}

bool
is_owner(CHAR_DATA *ch, OBJ_DATA *obj)
{
	return !mlstr_cmp(&ch->short_descr, &obj->owner);
}

bool
is_class(CHAR_DATA *ch, const char *cl)
{
	return IS_CLASS(ch->class, cl);
}

int
obj_level(OBJ_DATA *obj)
{
	return obj->level;
}

int
room_sector(ROOM_INDEX_DATA *room)
{
	return room->sector_type;
}

int
char_max_hit(CHAR_DATA *ch)
{
	return ch->max_hit;
}

int
obj_timer(OBJ_DATA *obj)
{
	return obj->timer;
}

void
set_obj_timer(OBJ_DATA *obj, int timer)
{
	obj->timer = timer;
}

void
set_char_gold(CHAR_DATA *ch, int gold)
{
	ch->gold = gold;
}

void
set_char_silver(CHAR_DATA *ch, int silver)
{
	ch->silver = silver;
}

void
wait_state(CHAR_DATA *ch, int ws)
{
	WAIT_STATE(ch, ws);
}

bool
is_affected(CHAR_DATA *ch, int aff)
{
	return IS_AFFECTED(ch, aff);
}

int
char_hit(CHAR_DATA *ch)
{
	return ch->hit;
}

int
obj_wear_loc(OBJ_DATA *obj)
{
	return obj->wear_loc;
}

bool
is_npc(CHAR_DATA *ch)
{
	return IS_NPC(ch);
}

int
umin(int i1, int i2)
{
	return UMIN(i1, i2);
}

int
umax(int i1, int i2)
{
	return UMAX(i1, i2);
}

int
char_position(CHAR_DATA *ch)
{
	return ch->position;
}

bool
is_wanted(CHAR_DATA *ch)
{
	return IS_WANTED(ch);
}

bool
is_awake(CHAR_DATA *ch)
{
	return IS_AWAKE(ch);
}

int
obj_item_type(OBJ_DATA *obj)
{
	return obj->item_type;
}

int
weapon_is(OBJ_DATA *obj, int wclass)
{
	return WEAPON_IS(obj, wclass);
}

const char *
char_clan(CHAR_DATA *ch)
{
	return ch->clan;
}

int
time_hour()
{
	return time_info.hour;
}

#define IS_FIGHTING(ch, victim)						\
	((RIDDEN(ch) && (victim)->fighting == RIDDEN(ch)) ||		\
	 (victim)->fighting == (ch))

CHAR_DATA *
get_random_fighting(CHAR_DATA *ch)
{
	CHAR_DATA *victim;
	int count = 0;
	int num;

	for (victim = ch->in_room->people;
	     victim != NULL; victim = victim->next_in_room) {
		if (IS_FIGHTING(ch, victim))
			count++;
	}

	if (!count)
		return NULL;

	num = number_range(1, count);
	count = 0;

	for (victim = ch->in_room->people;
	     victim != NULL; victim = victim->next_in_room) {
		if (IS_FIGHTING(ch, victim)
		&&  ++count == num)
			break;
	}

	return victim;
}

int
char_gold(CHAR_DATA *ch)
{
	return ch->gold;
}

int
char_silver(CHAR_DATA *ch)
{
	return ch->silver;
}

#else /* !defined(MPC) */

void
print(int i)
{
	fprintf(stderr, "===> %s: %d\n", __FUNCTION__, i);
}

void
print2(int i, int j)
{
	fprintf(stderr, "===> %s: %d, %d\n", __FUNCTION__, i, j);
}

void
prints(const char *s)
{
	fprintf(stderr, "===> %s: '%s'\n", __FUNCTION__, s);
}

#endif /* !defined(MPC) */
