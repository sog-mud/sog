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
 * $Id: eventfun.c,v 1.6 1999-12-20 12:09:52 kostik Exp $
 */


#include <sys/time.h>
#include <stdio.h>

#include "merc.h"
#include "fight.h"

DECLARE_EVENT_FUN(event_enter_lshield		);
DECLARE_EVENT_FUN(event_enter_shocking		);
DECLARE_EVENT_FUN(event_enter_thieftrap		);
DECLARE_EVENT_FUN(event_enter_mist		);
DECLARE_EVENT_FUN(event_update_plague		);
DECLARE_EVENT_FUN(event_update_poison		);
DECLARE_EVENT_FUN(event_update_slow		);
DECLARE_EVENT_FUN(event_update_sleep		);
DECLARE_EVENT_FUN(event_update_espirit		);
DECLARE_EVENT_FUN(event_leave_lshield		);
DECLARE_EVENT_FUN(event_enter_rlight		);
DECLARE_EVENT_FUN(event_update_rlight		);

void show_owner(CHAR_DATA *ch, AFFECT_DATA *af)
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

EVENT_FUN(event_enter_lshield)
{
	if (af->owner->in_room != ch->in_room)
		{
		 bug("Event: owner of lightning shield left the room.", 0);
		 affect_remove_room(ch->in_room, af); 
		 return;
		}

	char_puts("The protective shield of room blocks you.\n", ch);
	act("$N has entered the room.", af->owner, NULL, ch, TO_CHAR);
	dofun("wake", af->owner, str_empty);

	if (!is_safe_rspell(af, ch)) {
		ROOM_INDEX_DATA *room = ch->in_room;

		damage(af->owner, ch, dice(af->level, 4) + 12,
		       af->type, DAM_LIGHTNING, DAMF_SHOW);
		affect_remove_room(room , af);
	}
}

EVENT_FUN(event_enter_shocking)
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
		       NULL, DAM_TRAP_ROOM, DAMF_SHOW | DAMF_HUNGER);
		show_owner(ch, af);
		affect_remove_room(room , af);
	}
}

EVENT_FUN(event_enter_thieftrap)
{
	if (af->owner == ch) {
		act("You avoid your trap here.", ch, NULL, NULL, TO_CHAR);
		return;
	}
	
	if (!is_safe_rspell(af, ch)) {
		ROOM_INDEX_DATA *room = ch->in_room;

		damage(ch, ch, dice(af->level, 5) + 12,
		       NULL, DAM_TRAP_ROOM, DAMF_SHOW | DAMF_HUNGER);
		show_owner(ch, af);
		affect_remove_room(room , af);
	}
}

EVENT_FUN(event_enter_mist)
{
	act("There is some mist flowing in the air.", ch, NULL, NULL, TO_CHAR);
}

EVENT_FUN(event_update_plague)
{
	if (af->level < 1) af->level = 2;
	if (is_safe_rspell(af, ch)) return;
	spellfun_call("plague", NULL, af->level - 1, af->owner, ch);
}

EVENT_FUN(event_update_poison)
{
	if (af->level < 1) af->level = 2;
	if (!is_safe_rspell(af, ch))
		spellfun_call("poison", NULL, af->level - 1, af->owner, ch);
}

EVENT_FUN(event_update_slow)
{
	if (af->level < 1) af->level = 2;
	if (!is_safe_rspell(af, ch))
		spellfun_call("slow", NULL, af->level - 1, af->owner, ch);
}

EVENT_FUN(event_update_sleep)
{
	if (af->level < 1) af->level = 2;
	if (!is_safe_rspell(af, ch))
		spellfun_call("sleep", NULL, af->level - 1, af->owner, ch);
}

EVENT_FUN(event_update_espirit)
{
	AFFECT_DATA af2;

	if (af->level < 1) af->level = 2;
	if (!is_safe_rspell(af, ch)
	&&  !IS_IMMORTAL(ch)
	&&  !saves_spell(af->level + 2, ch, DAM_MENTAL)
	&&  !is_affected(ch, "evil spirit")
	&&  number_bits(3) == 0) {
		af2.where	= TO_AFFECTS;
		af2.level	= af->level;
		af2.type	= "evil spirit";
		af2.duration	= number_range(1, af->level/30);
		INT(af2.location) = APPLY_NONE;
		af2.modifier	= 0;
		af2.bitvector	= 0;
		affect_join(ch, &af2);
		char_puts("You feel worse than ever.\n", ch);
		act("$n looks more evil.", ch, NULL, NULL, TO_ROOM);
	}
}

EVENT_FUN(event_leave_lshield)
{
	if (ch == af->owner) affect_strip_room(ch->in_room, af->type);	
}

EVENT_FUN(event_enter_rlight)
{
	dofun("visible", ch, str_empty);
}

EVENT_FUN(event_update_rlight)
{
	if (IS_AFFECTED(ch, AFF_BLIND))
		spellfun_call("cure blindness", NULL, af->level, ch, ch);
	if (IS_AFFECTED(ch, AFF_CURSE))
		spellfun_call("remove curse", NULL, af->level, ch, ch);
	if (IS_AFFECTED(ch, AFF_POISON))
		spellfun_call("cure poison", NULL, af->level, ch, ch);
	if (IS_AFFECTED(ch, AFF_PLAGUE))
		spellfun_call("cure disease", NULL, af->level, ch, ch);
	spellfun_call("cure critical", NULL, af->level, ch, ch);

	char_puts("A warm feeling fills your body.\n", ch);
}

EVENT_FUN(event_updatechar_wcurse)
{
	AFFECT_DATA  witch;
	
	if (ch->in_room == NULL)
		return;

	act("The witch curse makes $n feel $s life slipping away.",
	    ch, NULL, NULL, TO_ROOM);
	char_puts("The witch curse makes you feeling your life slipping away.\n", ch);

	witch.where = af->where;
	witch.type  = af->type;
	witch.level = af->level;
	witch.duration = af->duration;
	witch.location = af->location;
	witch.modifier = af->modifier * 2;
	witch.bitvector = 0;
	witch.owner = af->owner;
	witch.events = af->events;

	affect_remove(ch, af);
	affect_to_char(ch ,&witch);
	ch->hit = UMIN(ch->hit, ch->max_hit);
	if (ch->hit < 1) {
		if (IS_IMMORTAL(ch))
			ch->hit = 1;
		else {
			ch->position = POS_DEAD;
			handle_death(ch, ch);
		}
	}
}

EVENT_FUN(event_updatechar_plague)
{
	AFFECT_DATA plague;
	CHAR_DATA *vch;
	int dam;

	if (ch->in_room == NULL)
		return;
        
	act("$n writhes in agony as plague sores erupt from $s skin.",
	    ch, NULL, NULL, TO_ROOM);
	char_puts("You writhe in agony from the plague.\n", ch);
	    
	if (af->level == 1)
		return;
	    
	plague.where 	 = TO_AFFECTS;
	plague.type 	 = "plague";
	plague.level 	 = af->level - 1; 
	plague.duration	 = number_range(1,2 * plague.level);
	INT(plague.location) = APPLY_STR;
	plague.modifier	 = -5;
	plague.bitvector = AFF_PLAGUE;
	    
	for (vch = ch->in_room->people; vch; vch = vch->next_in_room)
		if (!saves_spell(plague.level + 2, vch, DAM_DISEASE) 
		&& !IS_IMMORTAL(vch) 
		&& !IS_AFFECTED(vch, AFF_PLAGUE) 
		&& number_bits(2) == 0) {
			char_puts("You feel hot and feverish.\n", vch);
			act("$n shivers and looks very ill.",
			    vch, NULL, NULL, TO_ROOM);
			affect_join(vch, &plague);
		}

	dam = UMIN(ch->level, af->level/5 + 1);
	ch->mana -= dam;
	ch->move -= dam;
	damage(ch, ch, dam, "plague", DAM_DISEASE, DAMF_NONE);
	if (number_range(1, 100) < 70)
		damage(ch, ch, UMAX(ch->max_hit/20, 50), 
		       "plague", DAM_DISEASE, DAMF_SHOW);
}

EVENT_FUN(event_updatechar_poison)
{
	if (IS_AFFECTED(ch, AFF_SLOW)) return;

	act("$n shivers and suffers.", ch, NULL, NULL, TO_ROOM); 
	char_puts("You shiver and suffer.\n", ch);
	damage(ch, ch, af->level/10 + 1, "poison", DAM_POISON, DAMF_SHOW);
}

EVENT_FUN(event_updatefast_entangle)
{
	AFFECT_DATA *paf;

	if (!(paf = is_affected(ch, "entanglement"))) 
		return;

	if (!paf->owner || !ch->fighting) {
		affect_strip(ch, "entanglement");
		return;
	}

	if (paf->owner != ch->fighting) {
		affect_strip(ch, "entanglement");
		return;
	}

	if (INT(paf->location) == APPLY_DEX) {	/* victim case */
		if (number_percent() < get_curr_stat(ch, STAT_DEX)) {
			act("You manage to get free.", ch, NULL, NULL, TO_CHAR);
			act("$n manages to get free.", ch, NULL, NULL, TO_ROOM);
			if (is_affected(paf->owner, "entanglement")) 
				affect_strip(paf->owner, "entanglement");
			affect_strip(ch, "entanglement");
		}
	}
}

EVENT_FUN(event_updatechar_crippled_hands)
{
	if (get_eq_char(ch, WEAR_WIELD) 
	|| get_eq_char(ch, WEAR_SECOND_WIELD)
	|| get_eq_char(ch, WEAR_HOLD)) {
		char_puts("The pain pulses in your crippled hands.", ch);
		damage(ch, ch, ch->level/2, "crippled hands", DAM_HARM, TRUE);
	}
}

EVENT_FUN(event_timeoutchar_bonedragon)
{
	CHAR_DATA *chm, *drag;
	int i, dlev;

	if (!IS_NPC(ch)	|| ch->pMobIndex->vnum != MOB_VNUM_COCOON)
		return;

	if ((chm = ch->master) == NULL) {
		bug("hatchout_dragon: no master set!");
		extract_char(ch, 0);
		return;
	}

	dlev = chm->level * 2 / 3 + af->level / 14;

	act("Cocoon explodes and nasty dracolich emerges!",
	    ch, NULL, NULL, TO_ALL);

	drag = create_mob(get_mob_index(MOB_VNUM_BONE_DRAGON));
	for (i=0; i < MAX_STATS; i++)
		drag->perm_stat[i] = UMIN(25, 15 + dlev / 10);
	drag->perm_stat[STAT_STR] += 3;
	drag->perm_stat[STAT_DEX] += 1;
	drag->perm_stat[STAT_CON] += 1;
	drag->max_hit = UMIN(30000, number_range(100*dlev, 200*dlev));
	drag->hit = drag->max_hit;
	drag->max_mana = dice(dlev, 30);
	drag->mana = drag->max_mana;
	drag->level = dlev;
	for (i = 0; i < 3; i++)
		drag->armor[i] = interpolate(dlev, 100, -120);
	drag->armor[3] = interpolate(dlev, 100, -40);
	drag->gold = 0;
	NPC(drag)->dam.dice_number = number_fuzzy(13);
	NPC(drag)->dam.dice_type = number_fuzzy(9);
	drag->damroll = dlev/2 + dice(3, 11);

        if (GET_PET(ch) == NULL) {
	        add_follower(drag, chm);
	        drag->leader = chm;
	        PC(chm)->pet = drag;
	} else {
        drag->master = drag->leader = chm;	
	}
	char_to_room(drag, ch->in_room);
	extract_char(ch, 0);
}

