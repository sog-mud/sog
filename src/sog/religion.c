/*
 * $Id: religion.c,v 1.1 1998-09-04 05:27:47 fjoe Exp $
 */

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
