/*
 * $Id: rspells.c,v 1.2 1999-05-24 18:06:58 avn Exp $
 */

/***************************************************************************
 *     ANATOLIA 2.1 is copyright 1996-1997 Serdar BULUT, Ibrahim CANPUNAR  *	
 *     ANATOLIA has been brought to you by ANATOLIA consortium		   *
 *	 Serdar BULUT {Chronos}		bulut@rorqual.cc.metu.edu.tr       *
 *	 Ibrahim Canpunar  {Asena}	canpunar@rorqual.cc.metu.edu.tr    *	
 *	 Murat BICER  {KIO}		mbicer@rorqual.cc.metu.edu.tr	   *	
 *	 D.Baris ACAR {Powerman}	dbacar@rorqual.cc.metu.edu.tr	   *	
 *     By using this code, you have agreed to follow the terms of the      *
 *     ANATOLIA license, in the file Anatolia/anatolia.licence             *	
 ***************************************************************************/

/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
*	ROM 2.4 is copyright 1993-1995 Russ Taylor			   *
*	ROM has been brought to you by the ROM consortium		   *
*	    Russ Taylor (rtaylor@pacinfo.com)				   *
*	    Gabrielle Taylor (gtaylor@pacinfo.com)			   *
*	    Brian Moore (rom@rom.efn.org)				   *
*	By using this code, you have agreed to follow the terms of the	   *
*	ROM license, in the file Rom24/doc/rom.license			   *
***************************************************************************/

#include <sys/time.h>
#include <stdio.h>

#include "merc.h"
#include "fight.h"
#include "rspells.h"

DECLARE_DO_FUN(do_wake);

typedef struct
{
	char * const	name;
	int		event;
	EVENT_FUN * 	event_fun;
	int		sn;
} raffcall_t;

raffcall_t raff_table[] = 
{
    { "lightning shield",	EVENT_ENTER,	event_enter_lshield,0	},
    { "shocking trap",		EVENT_ENTER,	event_enter_shocking,0	},
    { "settraps",		EVENT_ENTER,	event_enter_thieftrap,0	},
    { "lethargic mist",		EVENT_ENTER,	event_enter_mist,0	},
    { "mysterious dream",	EVENT_ENTER,	event_enter_mist,0	},
    { "black death",		EVENT_UPDATE,	event_update_plague,0	},
    { "deadly venom",		EVENT_UPDATE,	event_update_poison,0	},
    { "lethargic mist",		EVENT_UPDATE,	event_update_slow,0	},
    { "mysterious dream",	EVENT_UPDATE,	event_update_sleep,0	},
    { "evil spirit",		EVENT_UPDATE,	event_update_espirit,0	},
    { "lightning shield",	EVENT_LEAVE,	event_leave_lshield,0	},

    { NULL }
};

void init_raff_table(void)
{
	raffcall_t	*rac;

	for (rac = raff_table; rac->name != NULL; rac++)
		if ((rac->sn = sn_lookup(rac->name)) == -1) {
			log_printf("[*****] BUG: init_raff_table:"
			"invalid skill name (%s) in table", rac->name);
			rac->sn = 0;
		}
}

EVENT_FUN * get_event_fun(int sn, int event)
{
	raffcall_t	*rac;

	for (rac = raff_table; rac->name != NULL; rac++)
		if (rac->event == event
		&& rac->sn == sn)
			return rac->event_fun;
	log_printf("[*****] BUG: get_event_fun: skill %s event %d", 
		skill_name(sn), event);
	log_printf("[*****] BUG: get_event_fun: No such event in table");
	return NULL;
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
	do_wake(raf->owner, str_empty);

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
			raf->type, DAM_TRAP_ROOM, TRUE);
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
			raf->type, DAM_TRAP_ROOM, TRUE);
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
	if (!is_safe_rspell(raf, ch))
		spell_plague(gsn_plague, raf->level - 1, raf->owner,
			ch, TAR_CHAR_OFFENSIVE);
}

EVENT_FUN(event_update_poison)
{
	if (raf->level < 1) raf->level = 2;
	if (!is_safe_rspell(raf, ch))
		spell_poison(gsn_poison, raf->level - 1, raf->owner,
			ch, TAR_CHAR_OFFENSIVE);
}

EVENT_FUN(event_update_slow)
{
	if (raf->level < 1) raf->level = 2;
	if (!is_safe_rspell(raf, ch))
		spell_slow(gsn_slow, raf->level - 1, raf->owner,
			ch, TAR_CHAR_OFFENSIVE);
}

EVENT_FUN(event_update_sleep)
{
	if (raf->level < 1) raf->level = 2;
	if (!is_safe_rspell(raf, ch))
		spell_sleep(gsn_sleep, raf->level - 1, raf->owner,
			ch, TAR_CHAR_OFFENSIVE);
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

