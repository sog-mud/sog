/*
 * $Id: religion.c,v 1.5.2.3 2001-07-16 18:29:05 fjoe Exp $
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
#include "merc.h"

/* vnums for tattoos */
#define OBJ_VNUM_TATTOO_LIRAIL		51
#define OBJ_VNUM_TATTOO_AMTINOR		52
#define OBJ_VNUM_TATTOO_AULIN		53
#define OBJ_VNUM_TATTOO_DAKKORN		54
#define OBJ_VNUM_TATTOO_ULWARK		55
#define OBJ_VNUM_TATTOO_MOARITH		56
#define OBJ_VNUM_TATTOO_ILISSA		57
#define OBJ_VNUM_TATTOO_HATHIR		58
#define OBJ_VNUM_TATTOO_THRORGRIM	59
#define OBJ_VNUM_TATTOO_IRIAN		60
#define OBJ_VNUM_TATTOO_STIVROSH	61
#define OBJ_VNUM_TATTOO_GWINNERAI	62
#define OBJ_VNUM_TATTOO_TRUSSK		63
#define OBJ_VNUM_TATTOO_LESSA		64
#define OBJ_VNUM_TATTOO_VILIRNA		65
#define OBJ_VNUM_TATTOO_CALAMIR		66
#define OBJ_VNUM_TATTOO_CARIEL		67

/* God's Name, name of religion, tattoo vnum  */
const struct religion_type religion_table [] =
{
  { "none",		"None",			0			},
  { "Lirail",		"Lawful Good",		OBJ_VNUM_TATTOO_LIRAIL  },
  { "Amtinor",		"Neutral Good",		OBJ_VNUM_TATTOO_AMTINOR	},
  { "Aulin",		"True Neutral",		OBJ_VNUM_TATTOO_AULIN   },
  { "Dakkorn",		"God of Justice",	OBJ_VNUM_TATTOO_DAKKORN	},
  { "Moarith",		"Chaotic Evil",		OBJ_VNUM_TATTOO_MOARITH	},
  { "Ulwark",		"Chaotic Good",		OBJ_VNUM_TATTOO_ULWARK  },
  { "Ilissa",		"Lawful Evil",		OBJ_VNUM_TATTOO_ILISSA	},
  { "Hathir",		"Neutral Evil",		OBJ_VNUM_TATTOO_HATHIR	},
  { "Throrgrim",	"Lawful Neutral",	OBJ_VNUM_TATTOO_THRORGRIM },
  { "Irian",		"Chaotic Neutral",	OBJ_VNUM_TATTOO_IRIAN	},
  { "Stivrosh",		"God of War",		OBJ_VNUM_TATTOO_STIVROSH},
  { "Gwinnerai",	"Goddess of Honor",	OBJ_VNUM_TATTOO_GWINNERAI },
  { "Trussk",		"God of Desease and Suffering",	OBJ_VNUM_TATTOO_TRUSSK	},
  { "Lessa",		"Goddess of Magic",	OBJ_VNUM_TATTOO_LESSA	},
  { "Vilirna",		"Goddess of Life",	OBJ_VNUM_TATTOO_VILIRNA	},
  { "Calamir",		"God of Delusion",	OBJ_VNUM_TATTOO_CALAMIR	},
  { "Cariel",		"Goddess of Revival",	OBJ_VNUM_TATTOO_CARIEL	}
};

const char *religion_name(int religion)
{
	return religion <= RELIGION_NONE || religion > MAX_RELIGION ?
			"none" : religion_table[religion].leader;
}

int religion_lookup(const char *name)
{
	int value;

	for (value = 0; value < MAX_RELIGION; value++) {
		if (!str_prefix(name, religion_table[value].leader))
			return value;
	}

	return -1;
} 

