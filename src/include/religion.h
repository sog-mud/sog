/*
 * $Id: religion.h,v 1.3.4.2 2001-05-21 18:53:48 fjoe Exp $
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

const char *	religion_name(int religion);
int		religion_lookup(const char *name);

/* To add god, make tattoo in limbo.are, add OBJ_VNUM_TATTOO_(GOD),
 * add here and add to const.c in the religion_table  also increase
 * MAX_RELIGION in merc.h  and make oprog for the tattoo
 */

#define RELIGION_NONE		0
#define RELIGION_LIRAIL		1
#define RELIGION_AMTINOR	2
#define RELIGION_AULIN		3
#define RELIGION_DAKKORN	4
#define RELIGION_MOARITH	5
#define RELIGION_ULWARK		6
#define RELIGION_ILISSA		7
#define RELIGION_HATHIR 	8
#define RELIGION_THRORGRIM	9
#define RELIGION_IRIAN		10
#define RELIGION_STIVROSH	11
#define RELIGION_GWINNERAI	12
#define RELIGION_TRUSSK		13
#define RELIGION_LESSA		14
#define RELIGION_VILIRNA	15
#define RELIGION_CALAMIR	16
#define RELIGION_CARIEL		17

/* Religion structure */
struct religion_type
{
	char *	leader;
	char *	name;
	int	vnum;
};

extern	const	struct religion_type	religion_table	[];

#endif
