/*-
 * Copyright (c) 1999 SoG Development Team
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
 * $Id: eventfun.c,v 1.43 2002-09-09 20:14:28 fjoe Exp $
 */

#include <sys/time.h>
#include <stdio.h>

#include <merc.h>

#include <sog.h>
#include <magic.h>

DECLARE_EVENT_FUN(event_enter_lshield);
DECLARE_EVENT_FUN(event_enter_alarm);
DECLARE_EVENT_FUN(event_enter_shocking);
DECLARE_EVENT_FUN(event_enter_thieftrap);
DECLARE_EVENT_FUN(event_enter_mist);
DECLARE_EVENT_FUN(event_update_plague);
DECLARE_EVENT_FUN(event_update_poison);
DECLARE_EVENT_FUN(event_update_slow);
DECLARE_EVENT_FUN(event_update_sleep);
DECLARE_EVENT_FUN(event_update_espirit);
DECLARE_EVENT_FUN(event_leave_lshield);
DECLARE_EVENT_FUN(event_enter_rlight);
DECLARE_EVENT_FUN(event_update_rlight);
DECLARE_EVENT_FUN(event_updatechar_wcurse);
DECLARE_EVENT_FUN(event_updatechar_plague);
DECLARE_EVENT_FUN(event_updatechar_poison);
DECLARE_EVENT_FUN(event_updatefast_entangle);
DECLARE_EVENT_FUN(event_updatechar_crippled_hands);
DECLARE_EVENT_FUN(event_updatechar_bonedragon);
DECLARE_EVENT_FUN(event_timeoutchar_bonedragon);
DECLARE_EVENT_FUN(event_affectremove_charmperson);

static void
show_owner(CHAR_DATA *ch, AFFECT_DATA *af)
{
	CHAR_DATA *owner;

	owner = af->owner;
	if (!owner) return;

	if ((get_curr_stat(owner, STAT_INT) + get_curr_stat(owner, STAT_WIS) +
		number_range(1, 5)) > (get_curr_stat(ch, STAT_INT) +
		get_curr_stat(ch, STAT_WIS)))
			return;
	act("Sure, this is $N's work!", ch, NULL, owner, TO_CHAR);
}

EVENT_FUN(event_enter_lshield, ch, af)
{
	if (af->owner->in_room != ch->in_room) {
		log(LOG_BUG, "event_enter_lshield: owner of lightning shield left the room");
		affect_remove_room(ch->in_room, af);
		return;
	}

	act_char("The protective shield of room blocks you.", ch);
	act("$N has entered the room.", af->owner, NULL, ch, TO_CHAR);
	dofun("wake", af->owner, str_empty);

	if (!is_safe_rspell(af, ch)) {
		ROOM_INDEX_DATA *room = ch->in_room;

		damage(af->owner, ch, dice(af->level, 4) + 12,
		       af->type, DAM_F_SHOW | DAM_F_TRAP_ROOM);
		affect_remove_room(room , af);
	}
}

EVENT_FUN(event_enter_alarm, ch, af)
{
	DESCRIPTOR_DATA *d;

	if (af->owner == ch)
		return;
	act_char("You trigger a hidden alarm.", ch);
	act("$N enters to the room and triggers a hidden alarm.", NULL, NULL,
		ch, TO_NOTVICT);

	for (d = descriptor_list; d; d = d->next) {
		CHAR_DATA *vch;
		if (d->connected != CON_PLAYING
		    || (vch = d->character) == NULL
		    || vch->in_room == NULL
		    || vch->in_room->area != ch->in_room->area)
			continue;
		act_char("You hear a loud ring.", vch);
	}
}

EVENT_FUN(event_enter_shocking, ch, af)
{
	if (af->owner == ch) {
		act("You avoid your trap here.", ch, NULL, NULL, TO_CHAR);
		return;
	}

	if (!is_safe_rspell(af, ch)) {
		ROOM_INDEX_DATA *room = ch->in_room;

		act("Shocking waves in this room shock you!",
			ch, NULL, NULL, TO_CHAR);
		damage(ch, ch, dice(af->level, 4) + 12,
		       af->type, DAM_F_SHOW | DAM_F_TRAP_ROOM);
		show_owner(ch, af);
		affect_remove_room(room , af);
	}
}

EVENT_FUN(event_enter_thieftrap, ch, af)
{
	if (af->owner == ch) {
		act("You avoid your trap here.", ch, NULL, NULL, TO_CHAR);
		return;
	}

	if (!is_safe_rspell(af, ch)) {
		ROOM_INDEX_DATA *room = ch->in_room;

		damage(ch, ch, dice(af->level, 5) + 12,
		       af->type, DAM_F_SHOW | DAM_F_TRAP_ROOM);
		show_owner(ch, af);
		affect_remove_room(room , af);
	}
}

EVENT_FUN(event_enter_mist, ch, af)
{
	act("There is some mist flowing in the air.", ch, NULL, NULL, TO_CHAR);
}

EVENT_FUN(event_update_plague, ch, af)
{
	if (af->level < 1)
		af->level = 2;
	if (!is_safe_rspell(af, ch))
		spellfun_call("plague", NULL, af->level - 1, af->owner, ch);
}

EVENT_FUN(event_update_poison, ch, af)
{
	if (af->level < 1)
		af->level = 2;
	if (!is_safe_rspell(af, ch))
		spellfun_call("poison", NULL, af->level - 1, af->owner, ch);
}

EVENT_FUN(event_update_slow, ch, af)
{
	if (af->level < 1)
		af->level = 2;
	if (!is_safe_rspell(af, ch))
		spellfun_call("slow", NULL, af->level - 1, af->owner, ch);
}

EVENT_FUN(event_update_sleep, ch, af)
{
	if (af->level < 1)
		af->level = 2;
	if (!is_safe_rspell(af, ch))
		spellfun_call("sleep", NULL, af->level - 1, af->owner, ch);
}

EVENT_FUN(event_update_espirit, ch, af)
{
	if (af->level < 1)
		af->level = 2;
	if (!is_safe_rspell(af, ch)
	&&  !IS_IMMORTAL(ch)
	&&  !saves_spell(af->level + 2, ch, DAM_MENTAL)
	&&  !is_sn_affected(ch, "evil spirit")
	&&  number_bits(3) == 0) {
		AFFECT_DATA *paf;

		paf = aff_new(TO_AFFECTS, "evil spirit");
		paf->level	= af->level;
		paf->duration	= number_range(1, af->level/30);
		affect_join(ch, paf);
		aff_free(paf);

		act_char("You feel worse than ever.", ch);
		act("$n looks more evil.", ch, NULL, NULL, TO_ROOM);
	}
}

EVENT_FUN(event_leave_lshield, ch, af)
{
	if (ch == af->owner)
		affect_strip_room(ch->in_room, af->type);
}

EVENT_FUN(event_enter_rlight, ch, af)
{
	make_visible(ch, FALSE);
}

EVENT_FUN(event_update_rlight, ch, af)
{
	dofun("visible", ch, str_empty);

	act_char("A warm feeling fills your body.", ch);

	if (IS_AFFECTED(ch, AFF_BLIND))
		spellfun_call("cure blindness", NULL, af->level, ch, ch);
	if (IS_AFFECTED(ch, AFF_CURSE))
		spellfun_call("remove curse", NULL, af->level, ch, ch);
	if (IS_AFFECTED(ch, AFF_POISON))
		spellfun_call("cure poison", NULL, af->level, ch, ch);
	if (IS_AFFECTED(ch, AFF_PLAGUE))
		spellfun_call("cure disease", NULL, af->level, ch, ch);
	spellfun_call("cure critical wounds", NULL, af->level, ch, ch);

}

EVENT_FUN(event_updatechar_wcurse, ch, af)
{
	AFFECT_DATA *paf;

	if (ch->in_room == NULL)
		return;

	act("The witch curse makes $n feel $s life slipping away.",
	    ch, NULL, NULL, TO_ROOM);
	act_char("The witch curse makes you feeling your life slipping away.", ch);

	paf = aff_dup(af);
	paf->modifier *= 2;
	affect_remove(ch, af);
	affect_to_char(ch, paf);
	aff_free(paf);

	ch->hit = UMIN(ch->hit, ch->max_hit);
	if (ch->hit < 1) {
		if (IS_IMMORTAL(ch))
			ch->hit = 1;
		else
			handle_death(ch, ch);
	}
}

EVENT_FUN(event_updatechar_plague, ch, af)
{
	AFFECT_DATA *paf;
	CHAR_DATA *vch;
	int dam;

	if (ch->in_room == NULL)
		return;

	act("$n writhes in agony as plague sores erupt from $s skin.",
	    ch, NULL, NULL, TO_ROOM);
	act_char("You writhe in agony from the plague.", ch);

	if (af->level == 1)
		return;

	paf = aff_new(TO_AFFECTS, "plague");
	paf->level	 = af->level - 1;
	paf->duration = number_range(1, 2 * paf->level);
	INT(paf->location) = APPLY_STR;
	paf->modifier = -5;
	paf->bitvector= AFF_PLAGUE;

	for (vch = ch->in_room->people; vch; vch = vch->next_in_room) {
		if (!saves_spell(paf->level + 2, vch, DAM_DISEASE)
		&&  !IS_IMMORTAL(vch)
		&&  !IS_AFFECTED(vch, AFF_PLAGUE)
		&&  number_bits(2) == 0) {
			act_char("You feel hot and feverish.", vch);
			act("$n shivers and looks very ill.",
			    vch, NULL, NULL, TO_ROOM);
			affect_join(vch, paf);
		}
	}

	aff_free(paf);

	dam = UMIN(ch->level, af->level/5 + 1);
	ch->mana -= dam;
	ch->move -= dam;
	damage(ch, ch, dam, af->type, DAM_F_NONE);
	if (number_range(1, 100) < 70) {
		damage(ch, ch, UMAX(ch->max_hit/20, 50),
		       af->type, DAM_F_NONE);
	}
}

EVENT_FUN(event_updatechar_poison, ch, af)
{
	if (IS_AFFECTED(ch, AFF_SLOW)) return;

	act("$n shivers and suffers.", ch, NULL, NULL, TO_ROOM);
	act_char("You shiver and suffer.", ch);
	damage(ch, ch, af->level/10 + 1, af->type, DAM_F_NONE);
}

EVENT_FUN(event_updatefast_entangle, ch, af)
{
	AFFECT_DATA *paf;

	if ((paf = affect_find(ch->affected, "entanglement")) == NULL)
		return;

	if (paf->owner == NULL || ch->fighting == NULL) {
		affect_strip(ch, "entanglement");
		return;
	}

	if (paf->owner != ch->fighting) {
		affect_strip(ch, "entanglement");
		return;
	}

	if (INT(paf->location) == APPLY_NONE) { /* ch case */
		OBJ_DATA * weapon = get_eq_char(ch, WEAR_SECOND_WIELD);
		if (!weapon
		||  !(WEAPON_IS(weapon, WEAPON_WHIP) ||
		      WEAPON_IS(weapon, WEAPON_FLAIL))) {
			if (is_sn_affected(paf->owner, "entanglement"))
				affect_strip(paf->owner, "entanglement");
			affect_strip(ch, "entanglement");
		}
	}

	if (INT(paf->location) == APPLY_DEX) {	/* victim case */
		if (number_percent() < get_curr_stat(ch, STAT_DEX)) {
			act("You manage to get free.", ch, NULL, NULL, TO_CHAR);
			act("$n manages to get free.", ch, NULL, NULL, TO_ROOM);
			if (is_sn_affected(paf->owner, "entanglement"))
				affect_strip(paf->owner, "entanglement");
			affect_strip(ch, "entanglement");
		}
	}
}

EVENT_FUN(event_updatechar_crippled_hands, ch, af)
{
	if (get_eq_char(ch, WEAR_WIELD)
	|| get_eq_char(ch, WEAR_SECOND_WIELD)
	|| get_eq_char(ch, WEAR_HOLD)) {
		act_char("The pain pulses in your crippled hands.", ch);
		damage(ch, ch, ch->level/2, "crippled hands", DAM_F_NONE);
	}
}

EVENT_FUN(event_updatechar_bonedragon, ch, af)
{
	if (!IS_NPC(ch)
	|| !ch->master
	|| ch->pMobIndex->vnum != MOB_VNUM_COCOON)
		return;

	if (af->modifier > 0 && --af->modifier == 0)
		act("You feel it is time to feed your dragon.",
			ch->master, NULL, NULL, TO_CHAR);

	if (af->duration < 3)
		act("You feel your dragon is about to hatch.",
			ch->master, NULL, NULL, TO_CHAR);
}

EVENT_FUN(event_timeoutchar_bonedragon, ch, af)
{
	CHAR_DATA *chm, *drag;
	int i, dlev;

	if (!IS_NPC(ch))
		return;

	if (ch->pMobIndex->vnum == MOB_VNUM_BONE_DRAGON && ch->leader) {
		act("You feel it is time to feed your dragon.",
			ch->leader, NULL, NULL, TO_CHAR);
		return;
	}

	if (ch->pMobIndex->vnum != MOB_VNUM_COCOON)
		return;

	if ((chm = ch->master) == NULL || chm->in_room != ch->in_room) {
		act("Cocoon explodes, revealing stinking flesh.",
			ch, NULL, NULL, TO_ROOM);
		extract_char(ch, 0);
		return;
	}

	dlev = UMIN(chm->level + 10, chm->level * 2 / 3 + af->level / 14);

	act("Cocoon explodes and nasty dracolich emerges!",
	    ch, NULL, NULL, TO_ALL);

	drag = create_mob(MOB_VNUM_BONE_DRAGON, 0);
	if (drag == NULL)
		return;

	for (i = 0; i < MAX_STAT; i++)
		drag->perm_stat[i] = UMIN(25, 15 + dlev / 10);
	drag->perm_stat[STAT_STR] += 3;
	drag->perm_stat[STAT_DEX] += 1;
	drag->perm_stat[STAT_CON] += 1;
	drag->max_hit = UMIN(30000, number_range(100*dlev, 200*dlev));
	drag->perm_hit = drag->hit = drag->max_hit;
	drag->max_mana = dice(dlev, 30);
	drag->perm_mana = drag->mana = drag->max_mana;
	drag->level = dlev;
	for (i = 0; i < 3; i++)
		drag->armor[i] = interpolate(dlev, 100, -120);
	drag->armor[3] = interpolate(dlev, 100, -40);
	drag->gold = 0;
	drag->silver = 0;
	NPC(drag)->dam.dice_number = number_fuzzy(13);
	NPC(drag)->dam.dice_type = number_fuzzy(9);
	drag->damroll = dlev/2 + dice(3, 11);
	ch->master = NULL;

        if (GET_PET(ch) == NULL) {
	        add_follower(drag, chm);
	        drag->leader = chm;
	        PC(chm)->pet = drag;
	}
	else
		act("But you already have a pet.", chm, NULL, NULL, TO_CHAR);

	char_to_room(drag, ch->in_room);
	extract_char(ch, 0);
}

EVENT_FUN(event_affectremove_charmperson, ch, af)
{
	ch->leader = NULL;
}
