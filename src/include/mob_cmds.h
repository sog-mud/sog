/*
 * $Id: mob_cmds.h,v 1.8 2001-06-24 10:50:39 avn Exp $
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

#ifndef _MOB_CMDS_H_
#define _MOB_CMDS_H_

struct	mob_cmd_type
{
    const char * 	name;
    DO_FUN *		do_fun;
};

/* the command table itself */
extern	const	struct	mob_cmd_type	mob_cmd_table	[];

/*
 * Command functions.
 * Defined in mob_cmds.c
 */
DECLARE_DO_FUN(do_mpasound	);
DECLARE_DO_FUN(do_mpgecho	);
DECLARE_DO_FUN(do_mpzecho	);
DECLARE_DO_FUN(do_mpkill	);
DECLARE_DO_FUN(do_mpattack	);
DECLARE_DO_FUN(do_mpassist	);
DECLARE_DO_FUN(do_mpjunk	);
DECLARE_DO_FUN(do_mpechoaround	);
DECLARE_DO_FUN(do_mpecho	);
DECLARE_DO_FUN(do_mpechoat	);
DECLARE_DO_FUN(do_mpmload	);
DECLARE_DO_FUN(do_mpoload	);
DECLARE_DO_FUN(do_mppurge	);
DECLARE_DO_FUN(do_mpgoto	);
DECLARE_DO_FUN(do_mpat		);
DECLARE_DO_FUN(do_mptransfer	);
DECLARE_DO_FUN(do_mpgtransfer	);
DECLARE_DO_FUN(do_mpforce	);
DECLARE_DO_FUN(do_mpgforce	);
DECLARE_DO_FUN(do_mpvforce	);
DECLARE_DO_FUN(do_mpdamage	);
DECLARE_DO_FUN(do_mpremember	);
DECLARE_DO_FUN(do_mpforget	);
DECLARE_DO_FUN(do_mpdelay	);
DECLARE_DO_FUN(do_mpcancel	);
DECLARE_DO_FUN(do_mpcall	);
DECLARE_DO_FUN(do_mpflee	);
DECLARE_DO_FUN(do_mpotransfer	);
DECLARE_DO_FUN(do_mpremove	);
DECLARE_DO_FUN(do_mpslay	);

#endif

