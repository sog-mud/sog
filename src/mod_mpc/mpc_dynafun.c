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
 * $Id: mpc_dynafun.c,v 1.13 2001-09-07 15:40:18 fjoe Exp $
 */

#include <stdio.h>

#if !defined(MPC)

#include <merc.h>

#include <sog.h>

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

	return !!varr_foreach(&PC(ch)->specs, has_sp_cb, spn, spn_rm);
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

	varr_foreach(&PC(ch)->specs, spclass_count_cb, spclass, spn_rm, &count);
	return count;
}

void
mob_interpret(CHAR_DATA *ch, const char *argument)
{
	interpret(ch, argument, FALSE);
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

OBJ_DATA *
load_obj(CHAR_DATA *ch, int vnum)
{
	OBJ_DATA *obj;

	if ((obj = create_obj(vnum, 0)) == NULL)
		return NULL;

	obj_to_char(obj, ch);
	return obj;
}

bool
is_ghost(CHAR_DATA *ch)
{
	return !IS_NPC(ch) && IS_SET(PC(ch)->plr_flags, PLR_GHOST);
}

void
purge_obj(OBJ_DATA *obj)
{
	extract_obj(obj, 0);
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

bool
transfer_group(CHAR_DATA *ch, ROOM_INDEX_DATA *room)
{
	CHAR_DATA *victim, *victim_next;
	bool found = FALSE;

	for (victim = ch->in_room->people; victim != NULL; victim = victim_next) {
		victim_next = victim->next_in_room;

		if (is_same_group(ch, victim)
		&&  transfer_char(victim, room)
		&&  !found)
			found = TRUE;
	}

	return found;
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

void
affect_char(CHAR_DATA *ch, int where, const char *sn,
	    int level, int duration, int loc, int mod, int bits)
{
	AFFECT_DATA *paf;

	if (!IS_APPLY_WHERE(where))
		return;

	paf = aff_new(where, sn);

	paf->level = level;
	paf->duration = duration;

	INT(paf->location) = loc;
	paf->modifier = mod;
	paf->bitvector = bits;

	affect_to_char(ch, paf);
	aff_free(paf);
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

void
set_char_hit(CHAR_DATA *ch, int hit)
{
	ch->hit = hit;
	update_pos(ch);
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
