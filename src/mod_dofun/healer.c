/*
 * $Id: healer.c,v 1.9 1998-09-01 18:37:58 fjoe Exp $
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

void do_heal(CHAR_DATA *ch, const char *argument)
{
    CHAR_DATA *mob;
    char arg[MAX_INPUT_LENGTH];
    int cost,sn;
    SPELL_FUN *spell;
    char *words;	

    /* check for healer */
	for (mob = ch->in_room->people; mob; mob = mob->next_in_room)
		if (IS_NPC(mob) && IS_SET(mob->act, ACT_HEALER)
		&&  (!mob->clan || mob->clan == ch->clan))
		 	break;
 
    if (mob == NULL) {
        send_to_char("You can't do that here.\n\r", ch);
        return;
    }

    one_argument(argument,arg);

    if (arg[0] == '\0') {
        /* display price list */
	act("Healer offers the following spells.",ch,NULL,mob,TO_CHAR);
	send_to_char("  light   : cure light wounds     10 gold\n\r",ch);
	send_to_char("  serious : cure serious wounds   15 gold\n\r",ch);
	send_to_char("  critic  : cure critical wounds  25 gold\n\r",ch);
	send_to_char("  heal    : healing spell         50 gold\n\r",ch);
	send_to_char("  blind   : cure blindness        20 gold\n\r",ch);
	send_to_char("  disease : cure disease          15 gold\n\r",ch);
	send_to_char("  poison  : cure poison           25 gold\n\r",ch); 
	send_to_char("  uncurse : remove curse          50 gold\n\r",ch);
	send_to_char("  refresh : restore movement       5 gold\n\r",ch);
	send_to_char("  mana    : restore mana          10 gold\n\r",ch);
	send_to_char("  master heal: master heal spell 200 gold\n\r",ch);
	send_to_char("  energize : restore 300 mana    200 gold\n\r",ch);
	send_to_char(" Type heal <type> to be healed.\n\r",ch);
	return;
    }

    if (!str_prefix(arg,"light"))
    {
        spell = spell_cure_light;
	sn    = sn_lookup("cure light");
	words = "judicandus dies";
	 cost  = 1000;
    }

    else if (!str_prefix(arg,"serious"))
    {
	spell = spell_cure_serious;
	sn    = sn_lookup("cure serious");
	words = "judicandus gzfuajg";
	cost  = 1600;
    }

    else if (!str_prefix(arg,"critical"))
    {
	spell = spell_cure_critical;
	sn    = sn_lookup("cure critical");
	words = "judicandus qfuhuqar";
	cost  = 2500;
    }

    else if (!str_prefix(arg,"heal"))
    {
	spell = spell_heal;
	sn = sn_lookup("heal");
	words = "pzar";
	cost  = 5000;
    }

    else if (!str_prefix(arg,"blindness"))
    {
	spell = spell_cure_blindness;
	sn    = sn_lookup("cure blindness");
      	words = "judicandus noselacri";		
        cost  = 2000;
    }

    else if (!str_prefix(arg,"disease"))
    {
	spell = spell_cure_disease;
	sn    = sn_lookup("cure disease");
	words = "judicandus eugzagz";
	cost = 1500;
    }

    else if (!str_prefix(arg,"poison"))
    {
	spell = spell_cure_poison;
	sn    = sn_lookup("cure poison");
	words = "judicandus sausabru";
	cost  = 2500;
    }
	
    else if (!str_prefix(arg,"uncurse") || !str_prefix(arg,"curse"))
    {
	spell = spell_remove_curse; 
	sn    = sn_lookup("remove curse");
	words = "candussido judifgz";
	cost  = 5000;
    }

    else if (!str_prefix(arg,"mana"))
    {
        spell = NULL;
        sn = -3;
        words = "candamira";
        cost = 1000;
    }

	
    else if (!str_prefix(arg,"refresh") || !str_prefix(arg,"moves"))
    {
	spell =  spell_refresh;
	sn    = sn_lookup("refresh");
	words = "candusima"; 
	cost  = 500;
    }

    else if (!str_prefix(arg,"master"))
    {
	spell =  spell_master_healing;
	sn    = sn_lookup("master healing");
	words = "candastra nikazubra"; 
	cost  = 20000;
    }

    else if (!str_prefix(arg,"energize"))
    {
	spell =  NULL;
	sn    = -2;
	words = "energizer"; 
	cost  = 20000;
    }

    else 
    {
	act("Healer does not offer that spell.  Type 'heal' for a list.",
	    ch,NULL,mob,TO_CHAR);
	return;
    }

    if (cost > (ch->gold * 100 + ch->silver))
    {
	act("You do not have that much gold.",
	    ch,NULL,mob,TO_CHAR);
	return;
    }

    WAIT_STATE(ch,PULSE_VIOLENCE);

    deduct_cost(ch,cost);
    mob->gold += cost / 100;

    act("$n utters the words '$T'.",mob,NULL,words,TO_ROOM);
    if (sn == -2)
     {
	ch->mana += 300;
	ch->mana = UMIN(ch->mana,ch->max_mana);
	send_to_char("A warm glow passes through you.\n\r",ch);
     }
    if (sn == -3)
    {
	ch->mana += dice(2,8) + mob->level / 3;
	ch->mana = UMIN(ch->mana,ch->max_mana);
	send_to_char("A warm glow passes through you.\n\r",ch);
    }
  
     if (sn < 0)
	return;
    
     spell(sn,mob->level,mob,ch,TARGET_CHAR);
}
