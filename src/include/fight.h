/*
 * $Id: fight.h,v 1.17 1999-12-04 07:45:58 kostik Exp $
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

void raw_kill(CHAR_DATA *ch, CHAR_DATA *victim);

bool	is_safe 	(CHAR_DATA *ch, CHAR_DATA *victim);
bool	is_safe_nomessage (CHAR_DATA *ch, CHAR_DATA *victim);
bool	is_safe_spell	(CHAR_DATA *ch, CHAR_DATA *victim, bool area);
bool	is_safe_rspell	(AFFECT_DATA *af, CHAR_DATA *victim);
void	violence_update (void);
void	multi_hit	(CHAR_DATA *ch, CHAR_DATA *victim, const char *dt);
bool	damage		(CHAR_DATA *ch, CHAR_DATA *victim, int dam,
			 const char *dt, int class, int dam_flags);
int 	reduce_damage	(CHAR_DATA *ch, int dam, int dam_class);
void	update_pos	(CHAR_DATA *victim);
void	set_fighting	(CHAR_DATA *ch, CHAR_DATA *victim);
void	stop_fighting	(CHAR_DATA *ch, bool fBoth);
bool	can_kill	(CHAR_DATA *ch, CHAR_DATA *victim);
CHAR_DATA *  check_guard     (CHAR_DATA *ch, CHAR_DATA *mob);
void	one_hit 	(CHAR_DATA *ch, CHAR_DATA *victim, const char *dt,
			 bool secondary);
bool	check_obj_dodge	(CHAR_DATA *ch, CHAR_DATA *victim,
			 OBJ_DATA *obj, int bonus);
int	get_dam_class	(CHAR_DATA *ch, OBJ_DATA *wield, const char **dt,
			 int *dam_flags);
bool	backstab_ok	(CHAR_DATA *ch, CHAR_DATA *victim);
void	backstab	(CHAR_DATA *ch, CHAR_DATA *victim, int chance);
void	yell		(CHAR_DATA *victim, CHAR_DATA *ch, const char * argument);
int	get_resistance	(CHAR_DATA *ch, int dam_type);

#define DAMF_NONE	(0)
#define DAMF_SHOW	(A)	/* show dam message */
#define DAMF_SECOND	(B)	/* damage inflicted by second weapon */
#define DAMF_HIT	(C)	/* damage by hit */
#define DAMF_HUNGER	(D)	/* damage by hunger */

#endif
