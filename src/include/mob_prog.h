/*
 * $Id: mob_prog.h,v 1.4 1998-09-17 15:51:21 fjoe Exp $
 */

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

/***************************************************************************
 *                                                                         *
 *  Based on MERC 2.2 MOBprograms by N'Atas-ha.                            *
 *  Written and adapted to ROM 2.4 by                                      *
 *          Markku Nylander (markku.nylander@uta.fi)                       *
 *                                                                         *
 ***************************************************************************/

#ifndef _MOB_PROG_H_
#define _MOB_PROG_H_

void	program_flow	(int vnum, CHAR_DATA *mob, CHAR_DATA *ch,
			 const void *arg1, const void *arg2);
void	mp_act_trigger	(const char *argument, CHAR_DATA *mob, CHAR_DATA *ch,
				const void *arg1, const void *arg2, int type);
bool	mp_percent_trigger (CHAR_DATA *mob, CHAR_DATA *ch, 				
				const void *arg1, const void *arg2, int type);
void	mp_bribe_trigger  (CHAR_DATA *mob, CHAR_DATA *ch, int amount);
bool	mp_exit_trigger   (CHAR_DATA *ch, int dir);
void	mp_give_trigger   (CHAR_DATA *mob, CHAR_DATA *ch, OBJ_DATA *obj);
void 	mp_greet_trigger  (CHAR_DATA *ch);
void	mp_hprct_trigger  (CHAR_DATA *mob, CHAR_DATA *ch);

#endif
