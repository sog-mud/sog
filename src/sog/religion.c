/*
 * $Id: religion.c,v 1.2 1998-09-17 15:51:22 fjoe Exp $
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

#include <stdio.h>
#include "const.h"
#include "typedef.h"
#include "religion.h"
#include "tables.h"

/* God's Name, name of religion, tattoo vnum  */
const struct religion_type religion_table [] =
{
  { "",			"None",			0 },
  { "Atum-Ra",		"Lawful Good",		OBJ_VNUM_TATTOO_APOLLON },
  { "Zeus",		"Neutral Good",		OBJ_VNUM_TATTOO_ZEUS },
  { "Siebele",		"True Neutral",		OBJ_VNUM_TATTOO_SIEBELE },
  { "Shamash",		"God of Justice",	OBJ_VNUM_TATTOO_HEPHAESTUS },
  { "Ahuramazda",	"Chaotic Good",		OBJ_VNUM_TATTOO_EHRUMEN },
  { "Ehrumen",		"Chaotic Evil",		OBJ_VNUM_TATTOO_AHRUMAZDA },
  { "Deimos",		"Lawful Evil",		OBJ_VNUM_TATTOO_DEIMOS },
  { "Phobos",		"Neutral Evil",		OBJ_VNUM_TATTOO_PHOBOS },
  { "Odin",		"Lawful Neutral",	OBJ_VNUM_TATTOO_ODIN },
  { "Teshub",		"Chaotic Neutral",	OBJ_VNUM_TATTOO_MARS },
  { "Ares",		"God of War",		OBJ_VNUM_TATTOO_ATHENA },
  { "Goktengri",	"God of Honor",		OBJ_VNUM_TATTOO_GOKTENGRI },
  { "Hera",		"Goddess of Hate",	OBJ_VNUM_TATTOO_HERA },
  { "Venus",		"Goddess of Beauty",	OBJ_VNUM_TATTOO_VENUS },
  { "Seth",		"God of Anger",		OBJ_VNUM_TATTOO_ARES },
  { "Enki",		"God of Knowledge",	OBJ_VNUM_TATTOO_PROMETHEUS },
  { "Eros",		"God of Love",		OBJ_VNUM_TATTOO_EROS }
};

char *religion_name(int religion)
{
	return religion <= RELIGION_NONE || religion > MAX_RELIGION ?
			"none" : religion_table[religion].leader;
}
