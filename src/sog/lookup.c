/*
 * $Id: lookup.c,v 1.12 1998-09-01 18:29:17 fjoe Exp $
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

#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include "merc.h"

int position_lookup(const char *name)
{
	return flag_value(position_table, name);
}

int size_lookup (const char *name)
{
	return flag_value(size_table, name);
}

int weapon_lookup (const char *name)
{
	return flag_value(weapon_class, name);
}

int item_lookup(const char *name)
{
	return flag_value(item_types, name);
}

/* returns material number */
int material_lookup(const char *name)
{
	return 0;
}

/* returns race number */
int race_lookup (const char *name)
{
   int race;

   for (race = 0; race_table[race].name != NULL; race++)
   {
		if (LOWER(name[0]) == LOWER(race_table[race].name[0])
		&&  !str_prefix(name,race_table[race].name))
		    return race;
   }

   return 0;
} 


int liq_lookup (const char *name)
{
	int liq;

	for (liq = 0; liq_table[liq].liq_name != NULL; liq++)
	{
		if (LOWER(name[0]) == LOWER(liq_table[liq].liq_name[0])
		&& !str_prefix(name,liq_table[liq].liq_name))
		    return liq;
	}

	return LIQ_WATER;
}
int attack_lookup  (const char *name)
{
	int att;

	for (att = 0; attack_table[att].name != NULL; att++)
	{
		if (LOWER(name[0]) == LOWER(attack_table[att].name[0])
		&&  !str_prefix(name,attack_table[att].name))
		    return att;
	}

	return 0;
}


/* returns a flag for wiznet */
long wiznet_lookup (const char *name)
{
	int flag;

	for (flag = 0; wiznet_table[flag].name != NULL; flag++)
	{
		if (LOWER(name[0]) == LOWER(wiznet_table[flag].name[0])
		&& !str_prefix(name,wiznet_table[flag].name))
		    return flag;
	}

	return -1;
}


