/*
 * $Id: healer.c,v 1.23 1999-06-22 12:37:18 fjoe Exp $
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

DECLARE_DO_FUN(	do_say	);

typedef struct
{
	char * const	keyword;
	char * const	name;
	char * const	spellname;
	int		level;
	int		price;
} heal_t;

heal_t heal_table[] =
{
    { "light",		"cure light wounds",	"cure light", 0, 10 },
    { "serious",	"cure serious wounds",	"cure serious", 0, 15 },
    { "critical",	"cure critical wounds",	"cure critical", 0, 25 },
    { "heal",		"healing spell",	"heal", 0, 50 },
    { "blind",		"cure blindness",	"cure blindness", 0, 20 },
    { "disease",	"cure disease",		"cure disease", 0, 15 },
    { "poison",		"cure poison",		"cure poison", 0, 25 },
    { "uncurse",	"remove curse",		"remove curse", 0, 50 },
    { "refresh",	"restore movement",	"refresh", 0, 5 },
    { "mana",		"restore mana",		"mana restore", 20, 10 },
    { "master",		"master heal spell",	"master healing", 0, 200 },
    { "energize",	"restore 300 mana",	"mana restore", 0, 200 },

    { NULL }
};

void do_heal(CHAR_DATA *ch, const char *argument)
{
    CHAR_DATA *mob;
    char arg[MAX_INPUT_LENGTH];
    int sn;
    int cost;
    heal_t *h;
    skill_t *sk;

    /* check for healer */
	for (mob = ch->in_room->people; mob; mob = mob->next_in_room)
		if (IS_NPC(mob) && IS_SET(mob->pIndexData->act, ACT_HEALER)
		&&  (!mob->clan || mob->clan == ch->clan))
		 	break;
 
    if (mob == NULL) {
        char_puts("You can't do that here.\n", ch);
        return;
    }

    if (HAS_SKILL(ch, gsn_spellbane)) {
	char_puts("You are Battle Rager, not the filthy magician\n",ch);
	return;
    }

    one_argument(argument, arg, sizeof(arg));

    if (arg[0] == '\0') {
        /* display price list */
	act("$N offers the following spells.",ch,NULL,mob,TO_CHAR);
	for (h = heal_table; h->keyword; h++)
	    char_printf(ch, "%10s : %20s : %d gold\n",
		h->keyword, h->name, h->price);
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
    cost = 100 * h->price;

    if (cost > (ch->gold * 100 + ch->silver))
    {
	act("You do not have that much gold.",
	    ch,NULL,mob,TO_CHAR);
	return;
    }

    WAIT_STATE(ch,PULSE_VIOLENCE);

    if (!can_see(mob, ch)) {
	do_say(mob, "I can't cast on those whom I don't see.");
	return;
    }

    deduct_cost(ch, cost);

    sn = sn_lookup(h->spellname);
    if (sn == -1) {
	bug("do_heal: invalid spell name", 0);
	return;
    }

	sk = SKILL(sn);
	if (sk->skill_type != ST_SPELL) {
		bug("do_heal: skill is not a spell", 0);
		return;
	}

	if (sk->fun == NULL) {
		bug("do_heal: no spell fun for skill", 0);
		return;
	}

	say_spell(mob, sn);
	sk->fun(sn, h->level ? h->level : mob->level, mob, ch);
}
