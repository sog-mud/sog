/*
 * $Id: lookup.c,v 1.4 1998-06-28 04:47:15 fjoe Exp $
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
#include "tables.h"
#include "db.h"

int flag_lookup (const char *name, const struct flag_type *flag_table)
{
    int flag;

    for (flag = 0; flag_table[flag].name != NULL; flag++)
    {
	if (LOWER(name[0]) == LOWER(flag_table[flag].name[0])
	&&  !str_prefix(name,flag_table[flag].name))
	    return flag_table[flag].bit;
    }

    return 0;
}


int position_lookup (const char *name)
{
   int pos;

   for (pos = 0; position_table[pos].name != NULL; pos++)
   {
	if (LOWER(name[0]) == LOWER(position_table[pos].name[0])
	&&  !str_prefix(name,position_table[pos].name))
	    return pos;
   }
   
   return -1;
}

int sex_lookup (const char *name)
{
   int sex;
   
   for (sex = 0; sex_table[sex].name != NULL; sex++)
   {
	if (LOWER(name[0]) == LOWER(sex_table[sex].name[0])
	&&  !str_prefix(name,sex_table[sex].name))
	    return sex;
   }

   return -1;
}

int size_lookup (const char *name)
{
   int size;
 
   for ( size = 0; size_table[size].name != NULL; size++)
   {
        if (LOWER(name[0]) == LOWER(size_table[size].name[0])
        &&  !str_prefix( name,size_table[size].name))
            return size;
   }
 
   return 0;
}

int lang_lookup (const char *name)
{
   int lang;

   if (LOWER(name[0]) == 'm' && 
     (!str_prefix(name,"mothertongue") || !str_prefix(name,"motherlanguage")))
   return MAX_LANGUAGE;

   for (lang = 0; lang < MAX_LANGUAGE; lang++)
   {
	if (LOWER(name[0]) == LOWER(language_table[lang].name[0])
	&&  !str_prefix(name,language_table[lang].name))
	    return lang;
   }
   
   return -1;
}

	 
/* returns material number */
int material_lookup (const char *name)
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


int weapon_lookup (const char *name)
{
	int type;

	for (type = 0; weapon_table[type].name != NULL; type++)
	{
		if (LOWER(name[0]) == LOWER(weapon_table[type].name[0])
		&&  !str_prefix(name,weapon_table[type].name))
		    return type;
	}
 
	return -1;
}


int item_lookup(const char *name)
{
	int type;

	for (type = 0; item_table[type].name != NULL; type++)
	{
	    if (LOWER(name[0]) == LOWER(item_table[type].name[0])
	    &&  !str_prefix(name,item_table[type].name))
	        return item_table[type].type;
	}
 
	return -1;
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


/* returns class number */
int class_lookup (const char *name)
{
   int class;
 
   for (class = 0; class < (MAX_CLASS-1); class++)
   {
	    if (LOWER(name[0]) == LOWER(class_table[class].name[0])
	    &&  !str_prefix(name,class_table[class].name))
	        return class;
   }
 
   return -1;
}


int clan_lookup (const char *argument)
{
   int clan;
 
   for (clan = 0; clan < MAX_CLAN; clan++)
   {
	    if (LOWER(argument[0]) == LOWER(clan_table[clan].short_name[0])
	    &&  !str_prefix(argument,clan_table[clan].short_name))
	        return clan;
   }
 
   return -1;
}

