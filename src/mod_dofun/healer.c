/*
 * $Id: healer.c,v 1.21 1999-05-18 17:52:03 fjoe Exp $
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
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,	   *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *									   *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael	   *
 *  Chastain, Michael Quan, and Mitchell Tse.				   *
 *									   *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc	   *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.						   *
 *									   *
 *  Much time and thought has gone into this software and you are	   *
 *  benefitting.  We hope that you share your changes too.  What goes	   *
 *  around, comes around.						   *
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
    SPELL_FUN *spell;
    heal_t *h;

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

    spell = (SKILL(sn))->spell_fun;
    if (!spell) {
	bug("do_heal: no spell fun for skill", 0);
	return;
    }

    say_spell(mob, sn);
    spell(sn, (h->level)?(h->level):(mob->level),
	mob, ch, TARGET_CHAR);
}
