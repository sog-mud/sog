/*
 * $Id: healer.c,v 1.31 1999-09-08 10:39:58 fjoe Exp $
 */

/*-
* Copyright (c) 1999 avn <avn@org.chem.msu.su>
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
*/

#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"

typedef struct
{
	char * const	keyword;
	char * const	name;
	char * const	spellname;
	int		level;
	int		price;
} heal_t;

/*
 * negative price means skill healing, available to BR.
 * for positive prices call spellfun(), for negative dofun()
 */
heal_t heal_table[] =
{
    { "light",	  "cure light wounds",	  "cure light",    0, 10	},
    { "serious",  "cure serious wounds",  "cure serious",  0, 15	},
    { "critical", "cure critical wounds", "cure critical", 0, 25	},
    { "heal",	  "healing spell",	  "heal",          0, 50	},
    { "blind",	  "cure blindness",	  "cure blindness",0, 20	},
    { "disease",  "cure disease",	  "cure disease",  0, 15	},
    { "poison",	  "cure poison",	  "cure poison",   0, 25	},
    { "uncurse",  "remove curse",	  "remove curse",  0, 50	},
    { "refresh",  "restore movement",	  "refresh",       0, 5,	},
    { "mana",	  "restore mana",	  "mana restore", 20, 10	},
    { "master",	  "master heal spell",	  "master healing",0, 200	},
    { "energize", "restore 300 mana",	  "mana restore",  0, 200	},
    { "herbs",	  "ranger's healing",	  "herbs",         0, -100	},

    { NULL }
};

int get_heal_cost(heal_t *h, CHAR_DATA *mob, CHAR_DATA *ch)
{
    int price;

    price = (h->price > 0)?(h->price):(-h->price);
    if (ch->clan && ch->clan == mob->clan) price /= 2;
    return price;
}

void do_heal(CHAR_DATA *ch, const char *argument)
{
    CHAR_DATA *mob;
    char arg[MAX_INPUT_LENGTH];
    int cost, sn;
    heal_t *h;

    /* check for healer */
	for (mob = ch->in_room->people; mob; mob = mob->next_in_room)
		if (IS_NPC(mob) && IS_SET(mob->pMobIndex->act, ACT_HEALER)
		&&  (!mob->clan || mob->clan == ch->clan))
		 	break;
 
    if (mob == NULL) {
        char_puts("You can't do that here.\n", ch);
        return;
    }

    one_argument(argument, arg, sizeof(arg));

    if (arg[0] == '\0') {
        /* display price list */
	act("$N offers the following spells.",ch,NULL,mob,TO_CHAR);
	for (h = heal_table; h->keyword; h++)
	    char_printf(ch, "%-10.9s : %-20.19s : %3d gold\n",
		h->keyword, h->name, get_heal_cost(h, mob, ch));
	char_puts(" Type heal <type> to be healed.\n",ch);
	return;
    }

    for (h = heal_table; h->keyword; h++)
	if (!str_prefix(arg, h->keyword)) break;

    if (h->keyword == NULL)	
    {
	act("$N does not offer that spell.  Type 'heal' for a list.",
	    ch,NULL,mob,TO_CHAR);
	return;
    }
    cost = get_heal_cost(h, mob, ch);

    if (HAS_SKILL(ch, gsn_spellbane) && (h->price > 0)) {
	char_puts("You are Battle Rager, not the filthy magician\n",ch);
	return;
    }

    if (cost > (ch->gold * 100 + ch->silver))
    {
	act("You do not have that much gold.",
	    ch,NULL,mob,TO_CHAR);
	return;
    }

    WAIT_STATE(ch,PULSE_VIOLENCE);

    if (cost < 0) {
	deduct_cost(ch, -cost);
	dofun(h->spellname, mob, ch->name);
	return;
    }

    sn = sn_lookup(h->spellname);
    if (sn < 0 || SKILL(sn)->skill_type != ST_SPELL) {
	bug("do_heal: invalid spell name in table");
	return;
    }
    deduct_cost(ch, cost);
    say_spell(mob, sn);
    spellfun_call(h->spellname, mob->level, mob, ch);
}
