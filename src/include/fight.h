/*
 * $Id: fight.h,v 1.25 2000-06-01 09:36:46 fjoe Exp $
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

#ifndef _FIGHT_H_
#define _FIGHT_H_

/*
 * dam_flags
 */
#define DAMF_NONE	(0)
#define DAMF_SHOW	(A)	/* show dam message */
#define DAMF_SECOND	(B)	/* damage inflicted by second weapon */
#define DAMF_HIT	(C)	/* damage by hit */
#define DAMF_HUNGER	(D)	/* damage by hunger */
#define DAMF_NOREDUCE	(E)	/* damage should not be reduced */
#define DAMF_THIRST	(F)	/* damage by thirst */
#define DAMF_LIGHT_V	(G)	/* light in room */
#define DAMF_TRAP_ROOM	(H)	/* room trap damage */

#define MODULE_NAME MOD_FIGHT
#include "dynafun_decl.h"

__MODULE_START_DECL(MOD_FIGHT)

DECLARE_FUN4(void, one_hit,
	     CHAR_DATA, ch, CHAR_DATA, victim, cchar_t, dt, bool, secondary)
DECLARE_FUN2(void, handle_death,
	     CHAR_DATA, ch, CHAR_DATA, victim)
DECLARE_FUN3(void, multi_hit,
	     CHAR_DATA, ch, CHAR_DATA, victim, cchar_t, dt)
DECLARE_FUN6(bool, damage,
	     CHAR_DATA, ch, CHAR_DATA, victim, int, dam,
	     cchar_t, dt, int, class, int, dam_flags)
DECLARE_FUN1(void, update_pos,
	     CHAR_DATA, victim)
DECLARE_FUN2(void, set_fighting,
	     CHAR_DATA, ch, CHAR_DATA, victim)
DECLARE_FUN2(void, stop_fighting,
	     CHAR_DATA, ch, bool, fBoth)
DECLARE_FUN2(OBJ_DATA, raw_kill,
	     CHAR_DATA, ch, CHAR_DATA, victim)
DECLARE_FUN1(cchar_t, vs_dam_alias,
	     int, dam)
DECLARE_FUN1(cchar_t, vp_dam_alias,
	     int, dam)
DECLARE_FUN4(bool, check_obj_dodge,
	     CHAR_DATA, ch, CHAR_DATA, victim,
	     OBJ_DATA, obj, int, bonus)
DECLARE_FUN2(bool, is_safe,
	     CHAR_DATA, ch, CHAR_DATA, victim)
DECLARE_FUN2(bool, is_safe_nomessage,
	     CHAR_DATA, ch, CHAR_DATA, victim)
DECLARE_FUN3(bool, is_safe_spell,
	     CHAR_DATA, ch, CHAR_DATA, victim, bool, area)
DECLARE_FUN2(bool, is_safe_rspell,
	     AFFECT_DATA, af, CHAR_DATA, victim)
DECLARE_FUN2(int, get_dam_class,
	     CHAR_DATA, ch, OBJ_DATA, wield)
DECLARE_FUN2(CHAR_DATA, check_guard,
	     CHAR_DATA, ch, CHAR_DATA, mob)
DECLARE_FUN2(bool, in_PK,
	     CHAR_DATA, ch, CHAR_DATA, victim)

__MODULE_END_DECL

#undef MODULE_NAME

#endif
