/***************************************************************************
 *     ANATOLIA 2.1 is copyright 1996-1997 Serdar BULUT, Ibrahim CANPUNAR  *
 *     ANATOLIA has been brought to you by ANATOLIA consortium		   *
 *	 Serdar BULUT {Chronos} 	bulut@rorqual.cc.metu.edu.tr	   *
 *	 Ibrahim Canpunar  {Asena}	canpunar@rorqual.cc.metu.edu.tr    *
 *	 Murat BICER  {KIO}		mbicer@rorqual.cc.metu.edu.tr	   *
 *	 D.Baris ACAR {Powerman}	dbacar@rorqual.cc.metu.edu.tr	   *
 *     By using this code, you have agreed to follow the terms of the	   *
 *     ANATOLIA license, in the file Anatolia/anatolia.licence		   *
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
#include <time.h>

#if !defined(WIN32)
#	include <unistd.h>
#endif
#include <ctype.h>

#include "merc.h"
#include "db.h"
#include "quest.h"
#include "obj_prog.h"
#include "fight.h"
#include "socials.h"
#include "string_edit.h"
#include "spec.h"

void do_specialize(CHAR_DATA* ch, const char* argument) 
{
	CHAR_DATA *mob;
	bool found = FALSE;
	char *weapon;
	char *output;

	for (mob=ch->in_room->people; mob; mob = mob->next_in_room) {
		if (IS_NPC(mob)  
		&& IS_SET(mob->pMobIndex->act, ACT_PRACTICE)
		&& IS_SET(mob->pMobIndex->practicer, GROUP_WEAPONSMASTER))
			found = TRUE;
	}

	if (!found) {
		act("You can't do that here.", ch, NULL, NULL, TO_CHAR);
		return;
	}

	if (argument[0] == '\0') {
		dofun("help", ch, "'SPECIALIZE'");
		return;
	}

	if (!str_prefix(argument, "mace")) {
		weapon = "weapon_mace";
		output = "mace";
	} else if (!str_prefix(argument, "sword")) {
		weapon = "weapon_sword";
		output = "sword";
	} else if (!str_prefix(argument, "axe")) {
		weapon = "weapon_axe";
		output = "axe";
	} else if (!str_prefix(argument, "hand")) {
		weapon = "weapon_hands";
		output = "hand to hand";
	} else if (!str_prefix(argument, "whip")
		|| !str_prefix(argument, "flail")) {
		weapon = "weapon_whip";
		output = "whip/flail";
	} else {
		dofun("help", ch, "'SPECIALIZE'");
		return;
	}

	if (!spec_replace(ch, NULL, weapon)) {
		act_puts("You are now specialized in $T.", 
			ch, NULL, output, TO_CHAR, POS_DEAD);
	} else {
		dofun("help", ch, "'SPECIALIZE'");
	}
}
