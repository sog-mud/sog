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
 * $Id: mpc_dynafun.c,v 1.32 2004-06-28 19:21:03 tatyana Exp $
 */

#include <stdio.h>
#include <setjmp.h>

#if !defined(MPC)

#include <merc.h>

#include <sog.h>

#include "mpc_impl.h"
#include "mpc_dynafun.h"

bool
has_sp(CHAR_DATA *ch, const char *spn, const char *spn_rm, const char *spn_add)
{
	const char **pspn;

	if (is_name_strict(spn_add, spn))
		return TRUE;

	C_FOREACH (const char **, pspn, &PC(ch)->specs) {
		if (!str_cmp(*pspn, spn_rm))
			return FALSE;

		if (is_name_strict(*pspn, spn))
			return TRUE;
	}

	return FALSE;
}

int
spclass_count(CHAR_DATA *ch, const char *spclass_name,
	      const char *spn_rm, const char *spn_add)
{
	const char **pspn;

	int count = 0;
	int spclass;
	spec_t *spec;

	if ((spclass = flag_svalue(spec_classes, spclass_name)) < 0)
		return 0;

	if ((spec = spec_lookup(spn_add)) != NULL
	&&  spec->spec_class == spclass)
		count++;

	C_FOREACH (const char **, pspn, &PC(ch)->specs) {
		if (!str_cmp(*pspn, spn_rm))
			continue;

		if ((spec = spec_lookup(*pspn)) != NULL
		&&  spec->spec_class == spclass)
			count++;
	}

	return count;
}

bool
is_immortal(CHAR_DATA *ch)
{
	return IS_IMMORTAL(ch);
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

bool
char_name_is(CHAR_DATA *ch, const char *nl)
{
	return is_name(ch->name, nl);
}

bool
is_owner(OBJ_DATA *obj, CHAR_DATA *ch)
{
	return !mlstr_cmp(&ch->short_descr, &obj->owner);
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
weapon_is(OBJ_DATA *obj, int wclass)
{
	return WEAPON_IS(obj, wclass);
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

bool
can_wear(OBJ_DATA *obj, int wear)
{
	return CAN_WEAR(obj, wear);
}

bool
is_owner_name(OBJ_DATA *obj, const char *name)
{
	return is_name_strict(name, mlstr_mval(&obj->owner));
}

bool
is_pumped(CHAR_DATA *ch)
{
	return IS_PUMPED(ch);
}

bool
is_mount(CHAR_DATA *ch, CHAR_DATA *mob)
{
	return !IS_NPC(ch) && ch->mount == mob;
}

bool
is_act(CHAR_DATA *ch, int act_flags)
{
	return IS_NPC(ch) && IS_SET(ch->pMobIndex->act, act_flags);
}

bool
has_detect(CHAR_DATA *ch, int f)
{
	return HAS_DETECT(ch, f);
}

bool
has_invis(CHAR_DATA *ch, int f)
{
	return HAS_INVIS(ch, f);
}

bool
room_is(ROOM_INDEX_DATA *r, int f)
{
	return IS_SET(r->room_flags, f);
}

bool
char_form_is(CHAR_DATA *ch, int form)
{
	return IS_SET(ch->form, form);
}

void
set_obj_owner(OBJ_DATA *obj, CHAR_DATA *ch)
{
	mlstr_cpy(&obj->owner, &ch->short_descr);
}

void
mob_interpret(CHAR_DATA *ch, const char *argument)
{
	interpret(ch, argument, FALSE);
}

CHAR_DATA *
load_mob(CHAR_DATA *ch, int vnum)
{
	CHAR_DATA *mob;

	if ((mob = create_mob(vnum, 0)) == NULL)
		return NULL;

	char_to_room(mob, ch->in_room);
	return mob;
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

AFFECT_DATA *
mpc_aff_new(int where, const char *sn)
{
	AFFECT_DATA *aff;
	AFFECT_DATA **paf;

	if (current_mpc == NULL)
		return NULL;

	aff = aff_new(where, sn);
	paf = varr_enew(&current_mpc->affects);
	*paf = aff;
	return aff;
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
