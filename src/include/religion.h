/*
 * $Id: religion.h,v 1.3 1998-11-18 05:20:40 fjoe Exp $
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

#ifndef _RELIGION_H_
#define _RELIGION_H_

char *	religion_name(int religion);

/* To add god, make tattoo in limbo.are, add OBJ_VNUM_TATTOO_(GOD),
 * add here and add to const.c in the religion_table  also increase
 * MAX_RELIGION in merc.h  and make oprog for the tattoo
 */

#define RELIGION_NONE		0
#define RELIGION_ATUM_RA	1
#define RELIGION_ZEUS		2
#define RELIGION_SIEBELE	3
#define RELIGION_SHAMASH	4
#define RELIGION_EHRUMEN	5
#define RELIGION_AHURAMAZDA	6
#define RELIGION_DEIMOS 	7
#define RELIGION_PHOBOS 	8
#define RELIGION_ODIN		9
#define RELIGION_TESHUB		10
#define RELIGION_ARES		11
#define RELIGION_GOKTENGRI	12
#define RELIGION_HERA		13
#define RELIGION_VENUS		14
#define RELIGION_SETH		15
#define RELIGION_ENKI		16
#define RELIGION_EROS		17

/* Religion structure */
struct religion_type
{
	char *	leader;
	char *	name;
	int	vnum;
};

extern	const	struct religion_type	religion_table	[];

#endif
