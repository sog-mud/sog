/*
 * $Id: effects.h,v 1.1 2000-06-01 09:36:46 fjoe Exp $
 */

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

#ifndef _EFFECTS_H_
#define _EFFECTS_H_

#define MODULE_NAME MOD_EFFECTS
#include "dynafun_decl.h"

__MODULE_START_DECL(MOD_EFFECTS)

DECLARE_FUN3(void, acid_effect,
	     void, vo, int, level, int, dam)
DECLARE_FUN3(void, cold_effect,
	     void, vo, int, level, int, dam)
DECLARE_FUN3(void, fire_effect,
	     void, vo, int, level, int, dam)
DECLARE_FUN3(void, poison_effect,
	     void, vo, int, level, int, dam)
DECLARE_FUN3(void, shock_effect,
	     void, vo, int, level, int, dam)
DECLARE_FUN3(void, sand_effect,
	     void, vo, int, level, int, dam)
DECLARE_FUN3(void, scream_effect,
	     void, vo, int, level, int, dam)

__MODULE_END_DECL

#undef MODULE_NAME

#endif
