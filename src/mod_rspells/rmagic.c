/*
 * Copyright (c) 1999 Arborn <avn@org.chem.msu.su>
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
 * $Id: rmagic.c,v 1.1 1999-07-30 05:18:21 avn Exp $
 */


#include <sys/time.h>
#include <stdio.h>

#include "merc.h"
#include "fight.h"
#include "raffects.h"

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


void show_owner(CHAR_DATA *ch, ROOM_AFFECT_DATA *raf)
{
	CHAR_DATA *owner;

	owner = raf->owner;
	if (!owner) return;

	if ((get_curr_stat(owner, STAT_INT) + get_curr_stat(owner, STAT_WIS) +
		number_range(1, 5)) > (get_curr_stat(ch, STAT_INT) +
		get_curr_stat(ch, STAT_WIS)))
			return;
	act("Sure, this is $N's work!", ch, NULL, owner, TO_CHAR);
}

EVENT_FUN(event_enter_lshield)
{
	if (raf->owner->in_room != ch->in_room)
		{
		 bug("Event: owner of lightning shield left the room.", 0);
		 affect_remove_room(room, raf); 
		 return;
		}

	char_puts("The protective shield of room blocks you.\n", ch);
	act("$N has entered the room.", raf->owner, NULL, ch, TO_CHAR);
	dofun("wake", raf->owner, str_empty);

	if (!is_safe_rspell(raf, ch)) 
		{
		 damage(raf->owner, ch, dice(raf->level, 4) + 12,
			raf->type, DAM_LIGHTNING, TRUE);
		 affect_remove_room(room , raf);
		}
}

EVENT_FUN(event_enter_shocking)
{
	if (raf->owner == ch) {
		act("You avoid your trap here.", ch, NULL, NULL, TO_CHAR);
		return;
	}
	
	if (!is_safe_rspell(raf, ch)) 
		{
		act("Shocking waves in this room shock you!",
			ch, NULL, NULL, TO_CHAR);
		damage(ch, ch, dice(raf->level, 4) + 12,
			TYPE_HIT, DAM_TRAP_ROOM, TRUE);
		show_owner(ch, raf);
		affect_remove_room(room , raf);
		}
}

EVENT_FUN(event_enter_thieftrap)
{
	if (raf->owner == ch) {
		act("You avoid your trap here.", ch, NULL, NULL, TO_CHAR);
		return;
	}
	
	if (!is_safe_rspell(raf, ch)) 
		{
		damage(ch, ch, dice(raf->level, 5) + 12,
			TYPE_HIT, DAM_TRAP_ROOM, TRUE);
		show_owner(ch, raf);
		affect_remove_room(room , raf);
		}
}

EVENT_FUN(event_enter_mist)
{
	act("There is some mist flowing in the air.", ch, NULL, NULL, TO_CHAR);
}

EVENT_FUN(event_update_plague)
{
	if (raf->level < 1) raf->level = 2;
	if (is_safe_rspell(raf, ch)) return;
	spellfun_call("plague", raf->level - 1, raf->owner, ch);
}

EVENT_FUN(event_update_poison)
{
	if (raf->level < 1) raf->level = 2;
	if (!is_safe_rspell(raf, ch))
		spellfun_call("poison", raf->level - 1, raf->owner, ch);
}

EVENT_FUN(event_update_slow)
{
	if (raf->level < 1) raf->level = 2;
	if (!is_safe_rspell(raf, ch))
		spellfun_call("slow", raf->level - 1, raf->owner, ch);
}

EVENT_FUN(event_update_sleep)
{
	if (raf->level < 1) raf->level = 2;
	if (!is_safe_rspell(raf, ch))
		spellfun_call("sleep", raf->level - 1, raf->owner, ch);
}

EVENT_FUN(event_update_espirit)
{
	AFFECT_DATA af;

	if (raf->level < 1) raf->level = 2;
	if (!is_safe_rspell(raf, ch)
	&& !IS_IMMORTAL(ch)
	&& !saves_spell(raf->level + 2, ch, DAM_MENTAL)
	&& !is_affected(ch, gsn_evil_spirit)
	&& number_bits(3) == 0) {
		af.where	= TO_AFFECTS;
		af.level	= raf->level;
		af.type		= gsn_evil_spirit;
		af.duration	= number_range(1, af.level/30);
		af.location	= APPLY_NONE;
		af.modifier	= 0;
		af.bitvector	= 0;
		affect_join(ch, &af);
		char_puts("You feel worse than ever.\n", ch);
		act("$n looks more evil.", ch, NULL, NULL, TO_ROOM);
	}
}

EVENT_FUN(event_leave_lshield)
{
	if (ch == raf->owner) affect_strip_room(room, raf->type);	
}

EVENT_FUN(event_enter_rlight)
{
	dofun("visible", ch, str_empty);
}

EVENT_FUN(event_update_rlight)
{
	if (IS_AFFECTED(ch, AFF_BLIND))
		spellfun_call("cure blindness", raf->level, ch, ch);
	if (IS_AFFECTED(ch, AFF_CURSE))
		spellfun_call("remove curse", raf->level, ch, ch);
	if (IS_AFFECTED(ch, AFF_POISON))
		spellfun_call("cure poison", raf->level, ch, ch);
	if (IS_AFFECTED(ch, AFF_PLAGUE))
		spellfun_call("cure disease", raf->level, ch, ch);
	spellfun_call("cure critical", raf->level, ch, ch);

	char_puts("A warm feeling fills your body.\n", ch);
}
