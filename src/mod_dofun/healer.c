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
 * $Id: healer.c,v 1.47 2001-02-11 14:35:39 fjoe Exp $
 */

#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"

#include "magic.h"
#include "update.h"

typedef struct {
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
    { "light",							// notrans
      "cure light wounds",	"cure light",		0, 1000		},

    { "serious",						// notrans
      "cure serious wounds",	"cure serious",		0, 1500		},

    { "critical",						// notrans
      "cure critical wounds",	"cure critical",	0, 2500		},

    { "heal",							// notrans
      "healing spell",	 	"heal",          	0, 5000		},

    { "blind",							// notrans
      "cure blindness",	 	"cure blindness",	0, 2000		},

    { "disease",						// notrans
      "cure disease",		"cure disease",		0, 1500		},

    { "poison",							// notrans
      "cure poison",		"cure poison",		0, 2500		},

    { "uncurse",						// notrans
      "remove curse",		"remove curse",		0, 5000		},

    { "refresh",						// notrans
      "restore movement",	"refresh",		0, 500,		},

    { "mana",							// notrans
      "restore mana",		"mana restore",		20, 1000	},

    { "master",							// notrans
      "master heal spell",	"master healing",	0, 20000	},

    { "energize",						// notrans
      "restore 300 mana",	"mana restore",		0, 20000	},

    { "herbs",							// notrans
      "ranger's healing",	"herbs",		0, -10000	},

    { NULL }
};

static int
get_heal_cost(heal_t *h, CHAR_DATA *mob, CHAR_DATA *ch)
{
	int price;

	price = abs(h->price);
	if (!IS_NULLSTR(ch->clan)
	&&  IS_CLAN(ch->clan, mob->clan))
		price /= 2;
	return price;
}

void do_heal(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *mob;
	char arg[MAX_INPUT_LENGTH];
	int cost;
	heal_t *h;

	/* check for healer */
	for (mob = ch->in_room->people; mob; mob = mob->next_in_room) {
		if (IS_NPC(mob)
		&&  MOB_IS(mob, MOB_HEALER)
		&&  (IS_NULLSTR(mob->clan) || IS_CLAN(mob->clan, ch->clan)))
		 	break;
	}
 
	if (mob == NULL) {
		act_char("You can't do that here.", ch);
		return;
	}

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		/*
		 * display price list
		 */
		BUFFER *buf;

		buf = buf_new(GET_LANG(ch));
		buf_act(buf, BUF_END, "$N offers the following spells.",
			ch, NULL, mob, NULL, TO_CHAR);
		for (h = heal_table; h->keyword; h++) {
			if (has_spec(ch, "clan_battleragers") && (h->price > 0))
				continue;
			buf_printf(buf, BUF_END,
				   "%-10.9s : %-20.19s : %3d gold\n",
				   h->keyword, h->name,
				   get_heal_cost(h, mob, ch) / 100);
		}
		buf_append(buf, "Type heal <spell> to be healed.\n");
		page_to_char(buf_string(buf), ch);
		buf_free(buf);
		return;
	}

	for (h = heal_table; h->keyword; h++) {
		if (!str_prefix(arg, h->keyword))
			break; 
	}

	if (h->keyword == NULL)	{
		act("$N does not offer that spell.  Type 'heal' for a list.",
		    ch, NULL, mob, TO_CHAR);
		return;
	}
	cost = get_heal_cost(h, mob, ch);

	if (has_spec(ch, "clan_battleragers") && (h->price > 0)) {
		act_char("You are Battlerager, not the filthy magician.", ch);
		return;
	}

	if (cost > (ch->gold * 100 + ch->silver)) {
		act("You do not have that much gold.", ch, NULL, mob, TO_CHAR);
		return;
	}

	WAIT_STATE(ch, get_pulse("violence"));

	if (h->price < 0) {
		deduct_cost(ch, cost);
		dofun(h->spellname, mob, ch->name);
		return;
	}

	deduct_cost(ch, cost);
	say_spell(mob, skill_search(h->spellname));
	spellfun_call(h->spellname, NULL,
		      h->level ? h->level : mob->level, mob, ch);
}
