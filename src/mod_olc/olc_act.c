/*
 * $Id: olc_act.c,v 1.8 1998-07-14 11:16:06 fjoe Exp $
 */

/***************************************************************************
 *  File: olc_act.c                                                        *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was freely distributed with the The Isles 1.1 source code,   *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 *                                                                         *
 ***************************************************************************/



#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "olc.h"
#include "recycle.h"
#include "lookup.h"
#include "comm.h"
#include "db.h"
#include "string_edit.h"
#include "magic.h"
#include "act_move.h"
#include "interp.h"
#include "buffer.h"
#include "tables.h"
#include "mlstring.h"

char * mprog_type_to_name (int type);

/* Return TRUE if area changed, FALSE if not. */
#define REDIT(fun)		bool fun(CHAR_DATA *ch, const char *argument)
#define OEDIT(fun)		bool fun(CHAR_DATA *ch, const char *argument)
#define MEDIT(fun)		bool fun(CHAR_DATA *ch, const char *argument)
#define AEDIT(fun)		bool fun(CHAR_DATA *ch, const char *argument)



struct olc_help_type
{
	char *command;
	const void *structure;
	char *desc;
};



bool show_version(CHAR_DATA *ch, const char *argument)
{
	send_to_char(VERSION, ch);
	send_to_char("\n\r", ch);
	send_to_char(AUTHOR, ch);
	send_to_char("\n\r", ch);
	send_to_char(DATE, ch);
	send_to_char("\n\r", ch);
	send_to_char(CREDITS, ch);
	send_to_char("\n\r", ch);

	return FALSE;
}    

/*
 * This table contains help commands and a brief description of each.
 * ------------------------------------------------------------------
 */
const struct olc_help_type help_table[] =
{
	{ "area",	area_flags,	"Area attributes."		},
	{ "room",	room_flags,	"Room attributes."		},
	{ "sector",	sector_flags,	"Sector types, terrain."	},
	{ "exit",	exit_flags,	"Exit types."			},
	{ "type",	type_flags,	"Types of objects."		},
	{ "extra",	extra_flags,	"Object attributes."		},
	{ "wear",	wear_flags,	"Where to wear object."		},
	{ "spec",	spec_table,	"Available special programs." 	},
	{ "sex",	sex_flags,	"Sexes."			},
	{ "act",	act_flags,	"Mobile attributes."		},
	{ "affect",	affect_flags,	"Mobile affects."		},
	{ "detect",	detect_flags,	"Mobile detections."		},
	{ "prac",	skill_groups,	"Skills group practicer."	},
	{ "wear-loc",	wear_loc_flags,	"Where mobile wears object."	},
	{ "spells",	skill_table,	"Names of current spells." 	},
	{ "container",	container_flags,"Container status."		},

/* ROM specific bits: */

	{ "armor",	ac_type,	"Ac for different attacks."	},
	{ "apply",	apply_flags,	"Apply flags"			},
	{ "form",	form_flags,	"Mobile body form."	        },
	{ "part",	part_flags,	"Mobile body parts."		},
	{ "imm",	imm_flags,	"Mobile immunity."		},
	{ "res",	res_flags,	"Mobile resistance."	        },
	{ "vuln",	vuln_flags,	"Mobile vulnerability."	},
	{ "off",	off_flags,	"Mobile offensive behaviour."	},
	{ "size",	size_flags,	"Mobile size."			},
	{ "position",   position_flags, "Mobile positions."            },
	{ "wclass",     weapon_class,   "Weapon class."                }, 
	{ "wtype",      weapon_type2,   "Special weapon type."         },
	{ "portal",	portal_flags,	"Portal types."		},
	{ "furniture",	furniture_flags,"Furniture types."		},
	{ "liquid",	liq_table,	"Liquid types."		},
	{ "apptype",	apply_types,	"Apply types."			},
	{ "weapon",	attack_table,	"Weapon types."		},
	{ "mprog",	mprog_flags,	"MobProgram flags."		},
	{ NULL,		NULL,		 NULL				}
};



/*****************************************************************************
 Name:		show_flag_cmds
 Purpose:	Displays settable flags and stats.
 Called by:	show_help(olc_act.c).
 ****************************************************************************/
void show_flag_cmds(CHAR_DATA *ch, const struct flag_type *flag_table)
{
	BUFFER *output;
	int  flag;
	int  col;
 
	output = buf_new(0);
	col = 0;
	for (flag = 0; flag_table[flag].name != NULL; flag++) {
		if (flag_table[flag].settable) {
			buf_printf(output, "%-19.18s", flag_table[flag].name);
			if (++col % 4 == 0)
				buf_add(output, "\n\r");
		}
	}
 
	if (col % 4 != 0)
		buf_add(output, "\n\r");

	send_to_char(buf_string(output), ch);
	buf_free(output);
}


/*****************************************************************************
 Name:		show_skill_cmds
 Purpose:	Displays all skill functions.
 		Does remove those damn immortal commands from the list.
 		Could be improved by:
 		(1) Adding a check for a particular class.
 		(2) Adding a check for a level range.
 Called by:	show_help(olc_act.c).
 ****************************************************************************/
void show_skill_cmds(CHAR_DATA *ch, int tar)
{
	int  sn;
	int  col;
	BUFFER *output;
 
	output = buf_new(0);
	col = 0;
	for (sn = 0; sn < MAX_SKILL; sn++) {
		if (!skill_table[sn].name)
			break;

		if (!str_cmp(skill_table[sn].name, "reserved")
		||  skill_table[sn].spell_fun == spell_null)
			continue;

		if (tar == -1 || skill_table[sn].target == tar) {
			buf_printf(output, "%-19.18s", skill_table[sn].name);
			if (++col % 4 == 0)
				buf_add(output, "\n\r");
		}
	}
 
	if (col % 4 != 0)
		buf_add(output, "\n\r");

	send_to_char(buf_string(output), ch);
	buf_free(output);
}



/*****************************************************************************
 Name:		show_spec_cmds
 Purpose:	Displays settable special functions.
 Called by:	show_help(olc_act.c).
 ****************************************************************************/
void show_spec_cmds(CHAR_DATA *ch)
{
	int  spec;
	int  col;
	BUFFER *output;

	output = buf_new(0);
	col = 0;
	buf_add(output, "Preceed special functions with 'spec_'\n\r\n\r");
	for (spec = 0; spec_table[spec].function != NULL; spec++) {
		buf_printf(output, "%-19.18s", &spec_table[spec].name[5]);
		if (++col % 4 == 0)
			buf_add(output, "\n\r");
	}
 
	if (col % 4 != 0)
		buf_add(output, "\n\r");

	send_to_char(buf_string(output), ch);
	buf_free(output);
}



/*****************************************************************************
 Name:		show_help
 Purpose:	Displays help for many tables used in OLC.
 Called by:	olc interpreters.
 ****************************************************************************/
bool show_help(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	char spell[MAX_INPUT_LENGTH];
	int cnt;

	argument = one_argument(argument, arg);
	one_argument(argument, spell);

	/*
	 * Display syntax.
	 */
	if (arg[0] == '\0') {
		send_to_char("Syntax:  ? [command]\n\r\n\r", ch);
		send_to_char("[command]  [description]\n\r", ch);
		for (cnt = 0; help_table[cnt].command != NULL; cnt++) {
			char_printf(ch, "%-10.10s -%s\n\r",
				    capitalize(help_table[cnt].command),
				    help_table[cnt].desc);
		}
		return FALSE;
	}

	/*
	 * Find the command, show changeable data.
	 * ---------------------------------------
	 */
	for (cnt = 0; help_table[cnt].command != NULL; cnt++) {
		if (arg[0] == help_table[cnt].command[0]
		&&  !str_prefix(arg, help_table[cnt].command)) {
			if (help_table[cnt].structure == spec_table) {
				show_spec_cmds(ch);
				return FALSE;
			}
			else if (help_table[cnt].structure == liq_table) {
				show_liqlist(ch);
				return FALSE;
			}
			else if (help_table[cnt].structure == attack_table) {
				show_damlist(ch);
				return FALSE;
			}
			else if (help_table[cnt].structure == skill_table) {

				if (spell[0] == '\0') {
					send_to_char("Syntax:  ? spells "
			        		     "[ignore/attack/defend/self/object/all]\n\r", ch);
					return FALSE;
				}

				if (!str_prefix(spell, "all"))
					show_skill_cmds(ch, -1);
				else if (!str_prefix(spell, "ignore"))
					show_skill_cmds(ch, TAR_IGNORE);
				else if (!str_prefix(spell, "attack"))
					show_skill_cmds(ch, TAR_CHAR_OFFENSIVE);
				else if (!str_prefix(spell, "defend"))
					show_skill_cmds(ch, TAR_CHAR_DEFENSIVE);
				else if (!str_prefix(spell, "self"))
					show_skill_cmds(ch, TAR_CHAR_SELF);
				else if (!str_prefix(spell, "object"))
					show_skill_cmds(ch, TAR_OBJ_INV);
				else
					send_to_char("Syntax:  ? spell "
			    			     "[ignore/attack/defend/self/object/all]\n\r", ch);
			    
					return FALSE;
			}
			else {
				show_flag_cmds(ch, help_table[cnt].structure);
				return FALSE;
			}
		}
	}

	show_help(ch, "");
	return FALSE;
}

REDIT(redit_rlist)
{
	ROOM_INDEX_DATA	*pRoomIndex;
	AREA_DATA		*pArea;
	BUFFER		*buffer;
	char		arg  [ MAX_INPUT_LENGTH    ];
	bool found;
	int vnum;
	int  col = 0;

	one_argument(argument, arg);

	pArea = ch->in_room->area;
	buffer = buf_new(0);
	found   = FALSE;

	for (vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++) {
		if ((pRoomIndex = get_room_index(vnum))) {
			found = TRUE;
			buf_printf(buffer, "[%5d] %-17.16s",
				vnum,
				capitalize(mlstr_cval(pRoomIndex->name, ch)));
			if (++col % 3 == 0)
				buf_add(buffer, "\n\r");
		}
	}

	if (!found) {
		send_to_char("Room(s) not found in this area.\n\r", ch);
		return FALSE;
	}

	if (col % 3 != 0)
		buf_add(buffer, "\n\r");

	page_to_char(buf_string(buffer), ch);
	buf_free(buffer);
	return FALSE;
}

REDIT(redit_mlist)
{
	MOB_INDEX_DATA	*pMobIndex;
	AREA_DATA	*pArea;
	BUFFER		*buffer;
	char		arg  [MAX_INPUT_LENGTH];
	bool fAll, found;
	int vnum;
	int  col = 0;

	one_argument(argument, arg);
	if (arg[0] == '\0') {
		send_to_char("Syntax:  mlist <all/name>\n\r", ch);
		return FALSE;
	}

	buffer = buf_new(0);
	pArea = ch->in_room->area;
	fAll    = !str_cmp(arg, "all");
	found   = FALSE;

	for (vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++) {
		if ((pMobIndex = get_mob_index(vnum)) != NULL) {
			if (fAll || is_name(arg, pMobIndex->player_name)) {
				found = TRUE;
				buf_printf(buffer, "[%5d] %-17.16s",
					   pMobIndex->vnum,
					   mlstr_mval(pMobIndex->short_descr));
				if (++col % 3 == 0)
					buf_add(buffer, "\n\r");
			}
		}
	}

	if (!found) {
		send_to_char("Mobile(s) not found in this area.\n\r", ch);
		return FALSE;
	}

	if (col % 3 != 0)
		buf_add(buffer, "\n\r");

	page_to_char(buf_string(buffer), ch);
	buf_free(buffer);
	return FALSE;
}



REDIT(redit_olist)
{
	OBJ_INDEX_DATA	*pObjIndex;
	AREA_DATA	*pArea;
	BUFFER		*buffer;
	char		arg  [MAX_INPUT_LENGTH];
	bool fAll, found;
	int vnum;
	int  col = 0;

	one_argument(argument, arg);
	if (arg[0] == '\0') {
		send_to_char("Syntax:  olist <all/name/item_type>\n\r", ch);
		return FALSE;
	}

	pArea   = ch->in_room->area;
	buffer  = buf_new(0);
	fAll    = !str_cmp(arg, "all");
	found   = FALSE;

	for (vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++) {
		if ((pObjIndex = get_obj_index(vnum))) {
			if (fAll || is_name(arg, pObjIndex->name)
			|| flag_value(type_flags, arg) == pObjIndex->item_type) {
				found = TRUE;
				buf_printf(buffer, "[%5d] %-17.16s",
					   pObjIndex->vnum,
					   mlstr_mval(pObjIndex->short_descr));
				if (++col % 3 == 0)
					buf_add(buffer, "\n\r");
			}
		}
	}

	if (!found) {
		send_to_char("Object(s) not found in this area.\n\r", ch);
		return FALSE;
	}

	if (col % 3 != 0)
		buf_add(buffer, "\n\r");

	page_to_char(buf_string(buffer), ch);
	buf_free(buffer);
	return FALSE;
}



REDIT(redit_mshow)
{
	MOB_INDEX_DATA *pMob;
	int value;

	if (argument[0] == '\0') {
		send_to_char("Syntax:  mshow <vnum>\n\r", ch);
		return FALSE;
	}

	if (!is_number(argument)) {
		send_to_char("REdit: Ingresa un numero.\n\r", ch);
		return FALSE;
	}

	if (is_number(argument)) {
		value = atoi(argument);
		if (!(pMob = get_mob_index(value))) {
			send_to_char("REdit:  That mobile does not exist.\n\r", ch);
			return FALSE;
		}

		ch->desc->pEdit = (void *)pMob;
	}
 
	medit_show(ch, argument);
	ch->desc->pEdit = (void *)ch->in_room;
	return FALSE; 
}



REDIT(redit_oshow)
{
	OBJ_INDEX_DATA *pObj;
	int value;

	if (argument[0] == '\0') {
		send_to_char("Syntax:  oshow <vnum>\n\r", ch);
		return FALSE;
	}

	if (!is_number(argument)) {
		send_to_char("REdit: Ingresa un numero.\n\r", ch);
		return FALSE;
	}

	if (is_number(argument)) {
		value = atoi(argument);
		if (!(pObj = get_obj_index(value))) {
			send_to_char("REdit:  That object does not exist.\n\r", ch);
			return FALSE;
		}

		ch->desc->pEdit = (void *)pObj;
	}
 
	oedit_show(ch, argument);
	ch->desc->pEdit = (void *)ch->in_room;
	return FALSE; 
}



/*****************************************************************************
 Name:		check_range(lower vnum, upper vnum)
 Purpose:	Ensures the range spans only one area.
 Called by:	aedit_vnum(olc_act.c).
 ****************************************************************************/
bool check_range(int lower, int upper)
{
	AREA_DATA *pArea;
	int cnt = 0;

	for (pArea = area_first; pArea; pArea = pArea->next) {
		/*
		 * lower < area < upper
		 */
		if ((lower <= pArea->min_vnum && pArea->min_vnum <= upper)
		||  (lower <= pArea->max_vnum && pArea->max_vnum <= upper))
			++cnt;

		if (cnt > 1)
			return FALSE;
	}
	return TRUE;
}



AREA_DATA *get_vnum_area(int vnum)
{
	AREA_DATA *pArea;

	for (pArea = area_first; pArea; pArea = pArea->next) {
		 if (vnum >= pArea->min_vnum
		 &&  vnum <= pArea->max_vnum)
		     return pArea;
	}

	return 0;
}



/*
 * Area Editor Functions.
 */
AEDIT(aedit_show)
{
	AREA_DATA *pArea;

	EDIT_AREA(ch, pArea);

	char_printf(ch, "Name:     [%5d] %s\n\r", pArea->vnum, pArea->name);

#if 0  /* ROM OLC */
	char_printf("Recall:   [%5d] %s\n\r", pArea->recall,
		get_room_index(pArea->recall)
		? get_room_index(pArea->recall)->name : "none");
#endif /* ROM */

	char_printf(ch, "File:     %s\n\r", pArea->file_name);

	char_printf(ch, "Vnums:    [%d-%d]\n\r", pArea->min_vnum, pArea->max_vnum);

	char_printf(ch, "Age:      [%d]\n\r",	pArea->age);

	char_printf(ch, "Players:  [%d]\n\r", pArea->nplayer);

	char_printf(ch, "Security: [%d]\n\r", pArea->security);

	char_printf(ch, "Builders: [%s]\n\r", pArea->builders);

	char_printf(ch, "Credits : [%s]\n\r", pArea->credits);

	char_printf(ch, "Flags:    [%s]\n\r", flag_string(area_flags, pArea->area_flags));

	return FALSE;
}



AEDIT(aedit_reset)
{
	AREA_DATA *pArea;

	EDIT_AREA(ch, pArea);

	reset_area(pArea);
	send_to_char("Area reset.\n\r", ch);

	return FALSE;
}



AEDIT(aedit_create)
{
	AREA_DATA *pArea;

	pArea               =   new_area();
	area_last->next     =   pArea;
	area_last		=   pArea;	/* Thanks, Walker. */
	ch->desc->pEdit     =   (void *)pArea;

	SET_BIT(pArea->area_flags, AREA_ADDED);
	send_to_char("Area Created.\n\r", ch);
	return FALSE;
}



AEDIT(aedit_name)
{
	AREA_DATA *pArea;

	EDIT_AREA(ch, pArea);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:   name [$name]\n\r", ch);
		return FALSE;
	}

	free_string(pArea->name);
	pArea->name = str_dup(argument);

	send_to_char("Name set.\n\r", ch);
	return TRUE;
}

AEDIT(aedit_credits)
{
	AREA_DATA *pArea;

	EDIT_AREA(ch, pArea);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:   credits [$credits]\n\r", ch);
		return FALSE;
	}

	free_string(pArea->credits);
	pArea->credits = str_dup(argument);

	send_to_char("Credits set.\n\r", ch);
	return TRUE;
}


AEDIT(aedit_file)
{
	AREA_DATA *pArea;
	char file[MAX_STRING_LENGTH];
	int i, length;

	EDIT_AREA(ch, pArea);

	one_argument(argument, file);	/* Forces Lowercase */

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  filename [$file]\n\r", ch);
		return FALSE;
	}

	/*
	 * Simple Syntax Check.
	 */
	length = strlen(argument);
	if (length > 8)
	{
		send_to_char("No more than eight characters allowed.\n\r", ch);
		return FALSE;
	}
	
	/*
	 * Allow only letters and numbers.
	 */
	for (i = 0; i < length; i++)
	{
		if (!isalnum(file[i]))
		{
			send_to_char("Only letters and numbers are valid.\n\r", ch);
			return FALSE;
		}
	}    

	free_string(pArea->file_name);
	strcat(file, ".are");
	pArea->file_name = str_dup(file);

	send_to_char("Filename set.\n\r", ch);
	return TRUE;
}



AEDIT(aedit_age)
{
	AREA_DATA *pArea;
	char age[MAX_STRING_LENGTH];

	EDIT_AREA(ch, pArea);

	one_argument(argument, age);

	if (!is_number(age) || age[0] == '\0')
	{
		send_to_char("Syntax:  age [#xage]\n\r", ch);
		return FALSE;
	}

	pArea->age = atoi(age);

	send_to_char("Age set.\n\r", ch);
	return TRUE;
}


#if 0 /* ROM OLC */
AEDIT(aedit_recall)
{
	AREA_DATA *pArea;
	char room[MAX_STRING_LENGTH];
	int  value;

	EDIT_AREA(ch, pArea);

	one_argument(argument, room);

	if (!is_number(argument) || argument[0] == '\0')
	{
		send_to_char("Syntax:  recall [#xrvnum]\n\r", ch);
		return FALSE;
	}

	value = atoi(room);

	if (!get_room_index(value))
	{
		send_to_char("AEdit:  Room vnum does not exist.\n\r", ch);
		return FALSE;
	}

	pArea->recall = value;

	send_to_char("Recall set.\n\r", ch);
	return TRUE;
}
#endif /* ROM OLC */


AEDIT(aedit_security)
{
	AREA_DATA *pArea;
	char sec[MAX_STRING_LENGTH];
	int  value;

	EDIT_AREA(ch, pArea);

	one_argument(argument, sec);

	if (!is_number(sec) || sec[0] == '\0')
	{
		send_to_char("Syntax:  security [#xlevel]\n\r", ch);
		return FALSE;
	}

	value = atoi(sec);

	if (value > ch->pcdata->security || value < 0)
	{
		if (ch->pcdata->security != 0)
			char_printf(ch, "Security is 0-%d.\n\r", ch->pcdata->security);
		else
			send_to_char("Security is 0 only.\n\r", ch);
		return FALSE;
	}

	pArea->security = value;

	send_to_char("Security set.\n\r", ch);
	return TRUE;
}



AEDIT(aedit_builder)
{
	AREA_DATA *pArea;
	char name[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH];

	EDIT_AREA(ch, pArea);

	one_argument(argument, name);

	if (name[0] == '\0')
	{
		send_to_char("Syntax:  builder [$name]  -toggles builder\n\r", ch);
		send_to_char("Syntax:  builder All      -allows everyone\n\r", ch);
		return FALSE;
	}

	name[0] = UPPER(name[0]);

	if (strstr(pArea->builders, name) != '\0')
	{
		pArea->builders = string_replace(pArea->builders, name, "\0");
		pArea->builders = string_unpad(pArea->builders);

		if (pArea->builders[0] == '\0')
		{
			free_string(pArea->builders);
			pArea->builders = str_dup("None");
		}
		send_to_char("Builder removed.\n\r", ch);
		return TRUE;
	}
	else
	{
		buf[0] = '\0';
		if (strstr(pArea->builders, "None") != '\0') {
			pArea->builders = string_replace(pArea->builders, "None", "\0");
			pArea->builders = string_unpad(pArea->builders);
		}

		if (pArea->builders[0] != '\0') {
			strcat(buf, pArea->builders);
			strcat(buf, " ");
		}
		strcat(buf, name);
		free_string(pArea->builders);
		pArea->builders = string_proper(str_dup(buf));

		send_to_char("Builder added.\n\r", ch);
		send_to_char(pArea->builders,ch);
		return TRUE;
	}

	return FALSE;
}



AEDIT(aedit_vnum)
{
	AREA_DATA *pArea;
	char lower[MAX_STRING_LENGTH];
	char upper[MAX_STRING_LENGTH];
	int  ilower;
	int  iupper;

	EDIT_AREA(ch, pArea);

	argument = one_argument(argument, lower);
	one_argument(argument, upper);

	if (!is_number(lower) || lower[0] == '\0'
	|| !is_number(upper) || upper[0] == '\0')
	{
		send_to_char("Syntax:  vnum [#xlower] [#xupper]\n\r", ch);
		return FALSE;
	}

	if ((ilower = atoi(lower)) > (iupper = atoi(upper)))
	{
		send_to_char("AEdit:  Upper must be larger then lower.\n\r", ch);
		return FALSE;
	}
	
	if (!check_range(atoi(lower), atoi(upper)))
	{
		send_to_char("AEdit:  Range must include only this area.\n\r", ch);
		return FALSE;
	}

	if (get_vnum_area(ilower)
	&& get_vnum_area(ilower) != pArea)
	{
		send_to_char("AEdit:  Lower vnum already assigned.\n\r", ch);
		return FALSE;
	}

	pArea->min_vnum = ilower;
	send_to_char("Lower vnum set.\n\r", ch);

	if (get_vnum_area(iupper)
	&& get_vnum_area(iupper) != pArea)
	{
		send_to_char("AEdit:  Upper vnum already assigned.\n\r", ch);
		return TRUE;	/* The lower value has been set. */
	}

	pArea->max_vnum = iupper;
	send_to_char("Upper vnum set.\n\r", ch);

	return TRUE;
}



AEDIT(aedit_lvnum)
{
	AREA_DATA *pArea;
	char lower[MAX_STRING_LENGTH];
	int  ilower;
	int  iupper;

	EDIT_AREA(ch, pArea);

	one_argument(argument, lower);

	if (!is_number(lower) || lower[0] == '\0')
	{
		send_to_char("Syntax:  min_vnum [#xlower]\n\r", ch);
		return FALSE;
	}

	if ((ilower = atoi(lower)) > (iupper = pArea->max_vnum))
	{
		send_to_char("AEdit:  Value must be less than the max_vnum.\n\r", ch);
		return FALSE;
	}
	
	if (!check_range(ilower, iupper))
	{
		send_to_char("AEdit:  Range must include only this area.\n\r", ch);
		return FALSE;
	}

	if (get_vnum_area(ilower)
	&& get_vnum_area(ilower) != pArea)
	{
		send_to_char("AEdit:  Lower vnum already assigned.\n\r", ch);
		return FALSE;
	}

	pArea->min_vnum = ilower;
	send_to_char("Lower vnum set.\n\r", ch);
	return TRUE;
}



AEDIT(aedit_uvnum)
{
	AREA_DATA *pArea;
	char upper[MAX_STRING_LENGTH];
	int  ilower;
	int  iupper;

	EDIT_AREA(ch, pArea);

	one_argument(argument, upper);

	if (!is_number(upper) || upper[0] == '\0')
	{
		send_to_char("Syntax:  max_vnum [#xupper]\n\r", ch);
		return FALSE;
	}

	if ((ilower = pArea->min_vnum) > (iupper = atoi(upper)))
	{
		send_to_char("AEdit:  Upper must be larger then lower.\n\r", ch);
		return FALSE;
	}
	
	if (!check_range(ilower, iupper))
	{
		send_to_char("AEdit:  Range must include only this area.\n\r", ch);
		return FALSE;
	}

	if (get_vnum_area(iupper)
	&& get_vnum_area(iupper) != pArea)
	{
		send_to_char("AEdit:  Upper vnum already assigned.\n\r", ch);
		return FALSE;
	}

	pArea->max_vnum = iupper;
	send_to_char("Upper vnum set.\n\r", ch);

	return TRUE;
}



/*
 * Room Editor Functions.
 */
REDIT(redit_show)
{
	char buf[MAX_STRING_LENGTH];
	ROOM_INDEX_DATA	*pRoom;
	BUFFER *	output;
	OBJ_DATA	*obj;
	CHAR_DATA	*rch;
	int		door;
	bool		fcnt;
	
	EDIT_ROOM(ch, pRoom);

	output = buf_new(0);
	
	buf_add(output, "Description:\n\r");
	mlstr_dump(output, "", pRoom->description);
	mlstr_dump(output, "Name:       ", pRoom->name);
	buf_printf(output, "Area:       [%5d] %s\n\r",
		   pRoom->area->vnum, pRoom->area->name);
	buf_printf(output, "Vnum:       [%5d]\n\rSector:     [%s]\n\r",
		   pRoom->vnum, flag_string(sector_flags, pRoom->sector_type));
	buf_printf(output, "Room flags: [%s]\n\r",
		   flag_string(room_flags, pRoom->room_flags));

	if (pRoom->heal_rate != 100 || pRoom->mana_rate != 100)
		buf_printf(output, "Health rec: [%d]\n\rMana rec  : [%d]\n\r",
			   pRoom->heal_rate , pRoom->mana_rate);

	if (pRoom->clan > 0)
		buf_printf(output, "Clan      : [%d] %s\n\r",
			   pRoom->clan, clan_table[pRoom->clan].short_name);

	if (!IS_NULLSTR(pRoom->owner))
		buf_printf(output, "Owner     : [%s]\n\r", pRoom->owner);

	if (pRoom->ed) {
		ED_DATA *ed;

		buf_add(output, "Desc Kwds:  [");
		for (ed = pRoom->ed; ed != NULL; ed = ed->next) {
			buf_add(output, ed->keyword);
			if (ed->next != NULL)
				buf_add(output, " ");
		}
		buf_add(output, "]\n\r");
	}

	buf_add(output, "Characters: [");
	fcnt = FALSE;
	for (rch = pRoom->people; rch != NULL; rch = rch->next_in_room) {
		one_argument(rch->name, buf);
		buf_add(output, buf);
		if (rch->next_in_room != NULL)
			buf_add(output, " ");
		fcnt = TRUE;
	}

	if (fcnt) 
		buf_add(output, "]\n\r");
	else
		buf_add(output, "none]\n\r");

	buf_add(output, "Objects:    [");
	fcnt = FALSE;
	for (obj = pRoom->contents; obj != NULL; obj = obj->next_content) {
		one_argument(obj->name, buf);
		buf_add(output, buf);
		if (obj->next_content != NULL)
			buf_add(output, " ");
		fcnt = TRUE;
	}

	if (fcnt)
		buf_add(output, "]\n\r");
	else
		buf_add(output, "none]\n\r");

	for (door = 0; door < MAX_DIR; door++) {
		EXIT_DATA *pexit;

		if ((pexit = pRoom->exit[door]) != NULL) {
			char word[MAX_INPUT_LENGTH];
			char reset_state[MAX_STRING_LENGTH];
			const char *state;
			int i, length;

			buf_printf(output, "-%-5s to [%5d] Key: [%5d] ",
				   capitalize(dir_name[door]),
				   pexit->u1.to_room ?
						pexit->u1.to_room->vnum : 0,
				   pexit->key);

			/*
			 * Format up the exit info.
			 * Capitalize all flags that are not part of the reset info.
			 */
			strcpy(reset_state, flag_string(exit_flags, pexit->rs_flags));
			state = flag_string(exit_flags, pexit->exit_info);
			buf_add(output, " Exit flags: [");
			for (; ;) {
				state = one_argument(state, word);

				if (word[0] == '\0') {
					buf_add(output, "]\n\r");
					break;
				}

				if (str_infix(word, reset_state)) {
					length = strlen(word);
					for (i = 0; i < length; i++)
						word[i] = UPPER(word[i]);
				}
				buf_add(output, word);
				buf_add(output, " ");
			}

			if (!IS_NULLSTR(pexit->keyword))
				buf_printf(output, "Kwds: [%s]\n\r",
					   pexit->keyword);
			mlstr_dump(output, "", pexit->description);
		}
	}

	send_to_char(buf_string(output), ch);
	buf_free(output);
	return FALSE;
}




/* Local function. */
bool change_exit(CHAR_DATA *ch, const char *argument, int door)
{
	ROOM_INDEX_DATA *pRoom;
	char command[MAX_INPUT_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	int  value;

	EDIT_ROOM(ch, pRoom);

	/*
	 * Set the exit flags, needs full argument.
	 * ----------------------------------------
	 */
	if ((value = flag_value(exit_flags, argument)) != NO_FLAG)
	{
		ROOM_INDEX_DATA *pToRoom;
		int rev;                                    /* ROM OLC */

		if (!pRoom->exit[door])
		   {
		   	send_to_char("Salida no existe.\n\r",ch);
		   	return FALSE;
		   }
		 /*   pRoom->exit[door] = new_exit(); */

		/*
		 * This room.
		 */
		TOGGLE_BIT(pRoom->exit[door]->rs_flags,  value);
		/* Don't toggle exit_info because it can be changed by players. */
		pRoom->exit[door]->exit_info = pRoom->exit[door]->rs_flags;

		/*
		 * Connected room.
		 */
		pToRoom = pRoom->exit[door]->u1.to_room;     /* ROM OLC */
		rev = rev_dir[door];

		if (pToRoom->exit[rev] != NULL)
		{
		   TOGGLE_BIT(pToRoom->exit[rev]->rs_flags,  value);
		   TOGGLE_BIT(pToRoom->exit[rev]->exit_info, value);
		}

		send_to_char("Exit flag toggled.\n\r", ch);
		return TRUE;
	}

	/*
	 * Now parse the arguments.
	 */
	argument = one_argument(argument, command);
	argument = one_argument(argument, arg);

	if (command[0] == '\0' && argument[0] == '\0')	/* Move command. */
	{
		move_char(ch, door, TRUE);                    /* ROM OLC */
		return FALSE;
	}

	if (command[0] == '?')
	{
		do_help(ch, "EXIT");
		return FALSE;
	}

	if (!str_cmp(command, "delete"))
	{
		ROOM_INDEX_DATA *pToRoom;
		int rev;                                     /* ROM OLC */
		
		if (!pRoom->exit[door])
		{
			send_to_char("REdit:  Cannot delete a null exit.\n\r", ch);
			return FALSE;
		}

		/*
		 * Remove ToRoom Exit.
		 */
		rev = rev_dir[door];
		pToRoom = pRoom->exit[door]->u1.to_room;       /* ROM OLC */
		
		if (pToRoom->exit[rev])
		{
			free_exit(pToRoom->exit[rev]);
			pToRoom->exit[rev] = NULL;
		}

		/*
		 * Remove this exit.
		 */
		free_exit(pRoom->exit[door]);
		pRoom->exit[door] = NULL;

		send_to_char("Exit unlinked.\n\r", ch);
		return TRUE;
	}

	if (!str_cmp(command, "link"))
	{
		EXIT_DATA *pExit;

		if (arg[0] == '\0' || !is_number(arg))
		{
			send_to_char("Syntax:  [direction] link [vnum]\n\r", ch);
			return FALSE;
		}

		value = atoi(arg);

		if (!get_room_index(value))
		{
			send_to_char("REdit:  Cannot link to non-existant room.\n\r", ch);
			return FALSE;
		}

		if (!IS_BUILDER(ch, get_room_index(value)->area))
		{
			send_to_char("REdit:  Cannot link to that area.\n\r", ch);
			return FALSE;
		}

		if (get_room_index(value)->exit[rev_dir[door]])
		{
			send_to_char("REdit:  Remote side's exit already exists.\n\r", ch);
			return FALSE;
		}

		if (!pRoom->exit[door])
		{
			pRoom->exit[door] = new_exit();
		}

		pRoom->exit[door]->u1.to_room = get_room_index(value);   /* ROM OLC */
		pRoom->exit[door]->orig_door = door;
		
/*	pRoom->exit[door]->vnum = value;                Can't set vnum in ROM */

		pRoom                   = get_room_index(value);
		door                    = rev_dir[door];
		pExit                   = new_exit();
		pExit->u1.to_room       = ch->in_room;
/*	pExit->vnum             = ch->in_room->vnum;    Can't set vnum in ROM */
		pExit->orig_door	= door;
		pRoom->exit[door]       = pExit;

		send_to_char("Two-way link established.\n\r", ch);
		return TRUE;
	}
		 
	if (!str_cmp(command, "dig"))
	{
		char buf[MAX_STRING_LENGTH];
		
		if (arg[0] == '\0' || !is_number(arg))
		{
			send_to_char("Syntax: [direction] dig <vnum>\n\r", ch);
			return FALSE;
		}
		
		redit_create(ch, arg);
		snprintf(buf, sizeof(buf), "link %s", arg);
		change_exit(ch, buf, door);
		return TRUE;
	}

	if (!str_cmp(command, "room"))
	{
		if (arg[0] == '\0' || !is_number(arg))
		{
			send_to_char("Syntax:  [direction] room [vnum]\n\r", ch);
			return FALSE;
		}

		if (!pRoom->exit[door])
		{
			pRoom->exit[door] = new_exit();
		}

		value = atoi(arg);

		if (!get_room_index(value))
		{
			send_to_char("REdit:  Cannot link to non-existant room.\n\r", ch);
			return FALSE;
		}

		pRoom->exit[door]->u1.to_room = get_room_index(value);    /* ROM OLC */
		pRoom->exit[door]->orig_door = door;
/*	pRoom->exit[door]->vnum = value;                 Can't set vnum in ROM */

		send_to_char("One-way link established.\n\r", ch);
		return TRUE;
	}

	if (!str_cmp(command, "key"))
	{
		if (arg[0] == '\0' || !is_number(arg))
		{
			send_to_char("Syntax:  [direction] key [vnum]\n\r", ch);
			return FALSE;
		}

		if (!pRoom->exit[door])
		   {
		   	send_to_char("Salida no existe.\n\r",ch);
		   	return FALSE;
		   }

/*	if (!pRoom->exit[door])
		{
			pRoom->exit[door] = new_exit();
		} */

		value = atoi(arg);

		if (!get_obj_index(value))
		{
			send_to_char("REdit:  Item doesn't exist.\n\r", ch);
			return FALSE;
		}

		if (get_obj_index(atoi(argument))->item_type != ITEM_KEY)
		{
			send_to_char("REdit:  Key doesn't exist.\n\r", ch);
			return FALSE;
		}

		pRoom->exit[door]->key = value;

		send_to_char("Exit key set.\n\r", ch);
		return TRUE;
	}

	if (!str_cmp(command, "name"))
	{
		if (arg[0] == '\0')
		{
			send_to_char("Syntax:  [direction] name [string]\n\r", ch);
			send_to_char("         [direction] name none\n\r", ch);
			return FALSE;
		}

		if (!pRoom->exit[door])
		   {
		   	send_to_char("Salida no existe.\n\r",ch);
		   	return FALSE;
		   }

/*	if (!pRoom->exit[door])
		{
			pRoom->exit[door] = new_exit();
		} */

		free_string(pRoom->exit[door]->keyword);
		if (str_cmp(arg,"none"))
			pRoom->exit[door]->keyword = str_dup(arg);
		else
			pRoom->exit[door]->keyword = str_dup("");

		send_to_char("Exit name set.\n\r", ch);
		return TRUE;
	}

	if (!str_prefix(command, "description")) {
		if (!pRoom->exit[door]) {
		   	send_to_char("Salida no existe.\n\r",ch);
		   	return FALSE;
		}

/*	    if (!pRoom->exit[door])
			{
			    pRoom->exit[door] = new_exit();
			} */

		if (!mlstr_append(ch, pRoom->exit[door]->description, arg)) {
			send_to_char("Syntax:  [direction] desc lang\n\r", ch);
			return FALSE;
		}
		return TRUE;
	}

	return FALSE;
}



REDIT(redit_north)
{
	if (change_exit(ch, argument, DIR_NORTH))
		return TRUE;

	return FALSE;
}



REDIT(redit_south)
{
	if (change_exit(ch, argument, DIR_SOUTH))
		return TRUE;

	return FALSE;
}



REDIT(redit_east)
{
	if (change_exit(ch, argument, DIR_EAST))
		return TRUE;

	return FALSE;
}



REDIT(redit_west)
{
	if (change_exit(ch, argument, DIR_WEST))
		return TRUE;

	return FALSE;
}



REDIT(redit_up)
{
	if (change_exit(ch, argument, DIR_UP))
		return TRUE;

	return FALSE;
}



REDIT(redit_down)
{
	if (change_exit(ch, argument, DIR_DOWN))
		return TRUE;

	return FALSE;
}



REDIT(redit_ed)
{
	ROOM_INDEX_DATA *pRoom;
	ED_DATA *ed;
	char command[MAX_INPUT_LENGTH];
	char keyword[MAX_INPUT_LENGTH];
	char lang[MAX_INPUT_LENGTH];

	EDIT_ROOM(ch, pRoom);

	argument = one_argument(argument, command);
	argument = one_argument(argument, keyword);
	argument = one_argument(argument, lang);

	if (command[0] == '\0' || keyword[0] == '\0')
	{
		send_to_char("Syntax:  ed add [keyword]\n\r", ch);
		send_to_char("         ed edit [keyword]\n\r", ch);
		send_to_char("         ed delete [keyword]\n\r", ch);
		send_to_char("         ed format [keyword]\n\r", ch);
		return FALSE;
	}

	if (!str_cmp(command, "add")) {
		if (keyword[0] == '\0' || lang[0] == '\0') {
			send_to_char("Syntax:  ed add keyword lang\n\r", ch);
			return FALSE;
		}

		ed			=   ed_new();
		ed->keyword		=   str_dup(keyword);
		ed->description		=   mlstr_new();
		ed->next		=   pRoom->ed;
		pRoom->ed	=   ed;

		mlstr_append(ch, ed->description, lang);

		return TRUE;
	}


	if (!str_cmp(command, "edit")) {
		if (keyword[0] == '\0' || lang[0] == '\0') {
			send_to_char("Syntax:  ed edit keyword lang\n\r", ch);
			return FALSE;
		}

		ed = ed_lookup(keyword, pRoom->ed);
		if (ed == NULL) {
			send_to_char("REdit:  Extra description keyword not found.\n\r", ch);
			return FALSE;
		}

		mlstr_append(ch, ed->description, lang);

		return TRUE;
	}


	if (!str_cmp(command, "delete")) {
		ED_DATA *ped = NULL;

		if (keyword[0] == '\0') {
			send_to_char("Syntax:  ed delete keyword\n\r", ch);
			return FALSE;
		}

		for (ed = pRoom->ed; ed; ed = ed->next) {
			if (is_name(keyword, ed->keyword))
				break;
			ped = ed;
		}

		if (ed == NULL) {
			send_to_char("REdit:  Extra description keyword not found.\n\r", ch);
			return FALSE;
		}

		if (ped == NULL)
			pRoom->ed = ed->next;
		else
			ped->next = ed->next;

		free_ed(ed);

		send_to_char("Extra description deleted.\n\r", ch);
		return TRUE;
	}


	if (!str_cmp(command, "format")) {
		if (keyword[0] == '\0') {
			send_to_char("Syntax:  ed format keyword\n\r", ch);
			return FALSE;
		}

		ed = ed_lookup(keyword, pRoom->ed);
		if (ed == NULL) {
			send_to_char("REdit:  Extra description keyword not found.\n\r", ch);
			return FALSE;
		}

		mlstr_format(ed->description);
		send_to_char("Extra description formatted.\n\r", ch);
		return TRUE;
	}

	redit_ed(ch, "");
	return FALSE;
}



REDIT(redit_create)
{
	AREA_DATA *pArea;
	ROOM_INDEX_DATA *pRoom;
	int value;
	int iHash;
	
	EDIT_ROOM(ch, pRoom);

	value = atoi(argument);

	if (argument[0] == '\0' || value <= 0)
	{
		send_to_char("Syntax:  create [vnum > 0]\n\r", ch);
		return FALSE;
	}

	pArea = get_vnum_area(value);
	if (!pArea)
	{
		send_to_char("REdit:  That vnum is not assigned an area.\n\r", ch);
		return FALSE;
	}

	if (!IS_BUILDER(ch, pArea))
	{
		send_to_char("REdit:  Vnum in an area you cannot build in.\n\r", ch);
		return FALSE;
	}

	if (get_room_index(value))
	{
		send_to_char("REdit:  Room vnum already exists.\n\r", ch);
		return FALSE;
	}

	pRoom			= new_room_index();
	pRoom->area			= pArea;
	pRoom->vnum			= value;

	if (value > top_vnum_room)
		 top_vnum_room = value;

	iHash			= value % MAX_KEY_HASH;
	pRoom->next			= room_index_hash[iHash];
	room_index_hash[iHash]	= pRoom;
	ch->desc->pEdit		= (void *)pRoom;

	send_to_char("Room created.\n\r", ch);
	return TRUE;
}



REDIT(redit_name)
{
	ROOM_INDEX_DATA *pRoom;

	EDIT_ROOM(ch, pRoom);

	if (!mlstr_change(pRoom->name, argument)) {
		send_to_char("Syntax: name lang name\n\r", ch);
		return FALSE;
	}

	send_to_char("Name set.\n\r", ch);
	return TRUE;
}



REDIT(redit_desc)
{
	ROOM_INDEX_DATA *pRoom;

	EDIT_ROOM(ch, pRoom);

	if (!mlstr_append(ch, pRoom->description, argument)) {
		send_to_char("Syntax: desc lang\n\r", ch);
		return FALSE;
	}
	return TRUE;
}

REDIT(redit_heal)
{
	ROOM_INDEX_DATA *pRoom;
	
	EDIT_ROOM(ch, pRoom);
	
	if (is_number(argument))
		{
		   pRoom->heal_rate = atoi (argument);
		   send_to_char ("Heal rate set.\n\r", ch);
		   return TRUE;
		}

	send_to_char ("Syntax : heal <#xnumber>\n\r", ch);
	return FALSE;
}       

REDIT(redit_mana)
{
	ROOM_INDEX_DATA *pRoom;
	
	EDIT_ROOM(ch, pRoom);
	
	if (is_number(argument))
		{
		   pRoom->mana_rate = atoi (argument);
		   send_to_char ("Mana rate set.\n\r", ch);
		   return TRUE;
		}

	send_to_char ("Syntax : mana <#xnumber>\n\r", ch);
	return FALSE;
}       

REDIT(redit_clan)
{
	ROOM_INDEX_DATA *pRoom;

	EDIT_ROOM(ch, pRoom);
	
	pRoom->clan = clan_lookup(argument);
	
	send_to_char ("Clan set.\n\r", ch);
	return TRUE;
}
	  
REDIT(redit_format)
{
	ROOM_INDEX_DATA *pRoom;

	EDIT_ROOM(ch, pRoom);

	mlstr_format(pRoom->description);

	send_to_char("String formatted.\n\r", ch);
	return TRUE;
}



REDIT(redit_mreset)
{
	ROOM_INDEX_DATA	*pRoom;
	MOB_INDEX_DATA	*pMobIndex;
	CHAR_DATA		*newmob;
	char		arg [ MAX_INPUT_LENGTH ];
	char		arg2 [ MAX_INPUT_LENGTH ];

	RESET_DATA		*pReset;

	EDIT_ROOM(ch, pRoom);

	argument = one_argument(argument, arg);
	argument = one_argument(argument, arg2);

	if (arg[0] == '\0' || !is_number(arg))
	{
		send_to_char ("Syntax:  mreset <vnum> <max #x> <mix #x>\n\r", ch);
		return FALSE;
	}

	if (!(pMobIndex = get_mob_index(atoi(arg))))
	{
		send_to_char("REdit: No mobile has that vnum.\n\r", ch);
		return FALSE;
	}

	if (pMobIndex->area != pRoom->area)
	{
		send_to_char("REdit: No such mobile in this area.\n\r", ch);
		return FALSE;
	}

	/*
	 * Create the mobile reset.
	 */
	pReset              = new_reset_data();
	pReset->command	= 'M';
	pReset->arg1	= pMobIndex->vnum;
	pReset->arg2	= is_number(arg2) ? atoi(arg2) : MAX_MOB;
	pReset->arg3	= pRoom->vnum;
	pReset->arg4	= is_number(argument) ? atoi (argument) : 1;
	add_reset(pRoom, pReset, 0/* Last slot*/);

	/*
	 * Create the mobile.
	 */
	newmob = create_mobile(pMobIndex);
	char_to_room(newmob, pRoom);

	char_printf(ch, "%s (%d) has been loaded and added to resets.\n\r"
		"There will be a maximum of %d loaded to this room.\n\r",
		mlstr_mval(pMobIndex->short_descr),
		pMobIndex->vnum,
		pReset->arg2);
	act("$n has created $N!", ch, NULL, newmob, TO_ROOM);
	return TRUE;
}



struct wear_type
{
	int	wear_loc;
	int	wear_bit;
};



const struct wear_type wear_table[] =
{
	{	WEAR_NONE,	ITEM_TAKE		},
	{	WEAR_LIGHT,	ITEM_LIGHT		},
	{	WEAR_FINGER_L,	ITEM_WEAR_FINGER	},
	{	WEAR_FINGER_R,	ITEM_WEAR_FINGER	},
	{	WEAR_NECK_1,	ITEM_WEAR_NECK		},
	{	WEAR_NECK_2,	ITEM_WEAR_NECK		},
	{	WEAR_BODY,	ITEM_WEAR_BODY		},
	{	WEAR_HEAD,	ITEM_WEAR_HEAD		},
	{	WEAR_LEGS,	ITEM_WEAR_LEGS		},
	{	WEAR_FEET,	ITEM_WEAR_FEET		},
	{	WEAR_HANDS,	ITEM_WEAR_HANDS		},
	{	WEAR_ARMS,	ITEM_WEAR_ARMS		},
	{	WEAR_SHIELD,	ITEM_WEAR_SHIELD	},
	{	WEAR_ABOUT,	ITEM_WEAR_ABOUT		},
	{	WEAR_WAIST,	ITEM_WEAR_WAIST		},
	{	WEAR_WRIST_L,	ITEM_WEAR_WRIST		},
	{	WEAR_WRIST_R,	ITEM_WEAR_WRIST		},
	{	WEAR_WIELD,	ITEM_WIELD		},
	{	WEAR_HOLD,	ITEM_HOLD		},
	{	NO_FLAG,	NO_FLAG			}
};



/*****************************************************************************
 Name:		wear_loc
 Purpose:	Returns the location of the bit that matches the count.
 		1 = first match, 2 = second match etc.
 Called by:	oedit_reset(olc_act.c).
 ****************************************************************************/
int wear_loc(int bits, int count)
{
	int flag;
 
	for (flag = 0; wear_table[flag].wear_bit != NO_FLAG; flag++)
	{
		 if (IS_SET(bits, wear_table[flag].wear_bit) && --count < 1)
		     return wear_table[flag].wear_loc;
	}
 
	return NO_FLAG;
}



/*****************************************************************************
 Name:		wear_bit
 Purpose:	Converts a wear_loc into a bit.
 Called by:	redit_oreset(olc_act.c).
 ****************************************************************************/
int wear_bit(int loc)
{
	int flag;
 
	for (flag = 0; wear_table[flag].wear_loc != NO_FLAG; flag++)
	{
		 if (loc == wear_table[flag].wear_loc)
		     return wear_table[flag].wear_bit;
	}
 
	return 0;
}



REDIT(redit_oreset)
{
	ROOM_INDEX_DATA	*pRoom;
	OBJ_INDEX_DATA	*pObjIndex;
	OBJ_DATA		*newobj;
	OBJ_DATA		*to_obj;
	CHAR_DATA		*to_mob;
	char		arg1 [ MAX_INPUT_LENGTH ];
	char		arg2 [ MAX_INPUT_LENGTH ];
	int			olevel = 0;

	RESET_DATA		*pReset;
	char		output [ MAX_STRING_LENGTH ];

	EDIT_ROOM(ch, pRoom);

	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);

	if (arg1[0] == '\0' || !is_number(arg1))
	{
		send_to_char ("Syntax:  oreset <vnum> <args>\n\r", ch);
		send_to_char ("        -no_args               = into room\n\r", ch);
		send_to_char ("        -<obj_name>            = into obj\n\r", ch);
		send_to_char ("        -<mob_name> <wear_loc> = into mob\n\r", ch);
		return FALSE;
	}

	if (!(pObjIndex = get_obj_index(atoi(arg1))))
	{
		send_to_char("REdit: No object has that vnum.\n\r", ch);
		return FALSE;
	}

	if (pObjIndex->area != pRoom->area)
	{
		send_to_char("REdit: No such object in this area.\n\r", ch);
		return FALSE;
	}

	/*
	 * Load into room.
	 */
	if (arg2[0] == '\0')
	{
		pReset		= new_reset_data();
		pReset->command	= 'O';
		pReset->arg1	= pObjIndex->vnum;
		pReset->arg2	= 0;
		pReset->arg3	= pRoom->vnum;
		pReset->arg4	= 0;
		add_reset(pRoom, pReset, 0/* Last slot*/);

		newobj = create_object(pObjIndex, number_fuzzy(olevel));
		obj_to_room(newobj, pRoom);

		char_printf(ch, "%s (%d) has been loaded and added to resets.\n\r",
			mlstr_mval(pObjIndex->short_descr),
			pObjIndex->vnum);
	}
	else
	/*
	 * Load into object's inventory.
	 */
	if (argument[0] == '\0'
	&& ((to_obj = get_obj_list(ch, arg2, pRoom->contents)) != NULL))
	{
		pReset		= new_reset_data();
		pReset->command	= 'P';
		pReset->arg1	= pObjIndex->vnum;
		pReset->arg2	= 0;
		pReset->arg3	= to_obj->pIndexData->vnum;
		pReset->arg4	= 1;
		add_reset(pRoom, pReset, 0/* Last slot*/);

		newobj = create_object(pObjIndex, number_fuzzy(olevel));
		newobj->cost = 0;
		obj_to_obj(newobj, to_obj);

		char_printf(ch, "%s (%d) has been loaded into "
			"%s (%d) and added to resets.\n\r",
			mlstr_mval(newobj->short_descr),
			newobj->pIndexData->vnum,
			mlstr_mval(to_obj->short_descr),
			to_obj->pIndexData->vnum);
	}
	else
	/*
	 * Load into mobile's inventory.
	 */
	if ((to_mob = get_char_room(ch, arg2)) != NULL)
	{
		int	wear_loc;

		/*
		 * Make sure the location on mobile is valid.
		 */
		if ((wear_loc = flag_value(wear_loc_flags, argument)) == NO_FLAG)
		{
			send_to_char("REdit: Invalid wear_loc.  '? wear-loc'\n\r", ch);
			return FALSE;
		}

		/*
		 * Disallow loading a sword(WEAR_WIELD) into WEAR_HEAD.
		 */
		if (!IS_SET(pObjIndex->wear_flags, wear_bit(wear_loc)))
		{
			char_printf(ch,
			    "%s (%d) has wear flags: [%s]\n\r",
			    mlstr_mval(pObjIndex->short_descr),
			    pObjIndex->vnum,
			flag_string(wear_flags, pObjIndex->wear_flags));
			return FALSE;
		}

		/*
		 * Can't load into same position.
		 */
		if (get_eq_char(to_mob, wear_loc))
		{
			send_to_char("REdit:  Object already equipped.\n\r", ch);
			return FALSE;
		}

		pReset		= new_reset_data();
		pReset->arg1	= pObjIndex->vnum;
		pReset->arg2	= wear_loc;
		if (pReset->arg2 == WEAR_NONE)
			pReset->command = 'G';
		else
			pReset->command = 'E';
		pReset->arg3	= wear_loc;

		add_reset(pRoom, pReset, 0/* Last slot*/);

		olevel  = URANGE(0, to_mob->level - 2, LEVEL_HERO);
		 newobj = create_object(pObjIndex, number_fuzzy(olevel));

		if (to_mob->pIndexData->pShop)	/* Shop-keeper? */
		{
			switch (pObjIndex->item_type)
			{
			default:		olevel = 0;				break;
			case ITEM_PILL:	olevel = number_range( 0, 10);	break;
			case ITEM_POTION:	olevel = number_range( 0, 10);	break;
			case ITEM_SCROLL:	olevel = number_range( 5, 15);	break;
			case ITEM_WAND:	olevel = number_range(10, 20);	break;
			case ITEM_STAFF:	olevel = number_range(15, 25);	break;
			case ITEM_ARMOR:	olevel = number_range( 5, 15);	break;
			case ITEM_WEAPON:	if (pReset->command == 'G')
						    olevel = number_range(5, 15);
				else
				    olevel = number_fuzzy(olevel);
			break;
			}

			newobj = create_object(pObjIndex, olevel);
			if (pReset->arg2 == WEAR_NONE)
			SET_BIT(newobj->extra_flags, ITEM_INVENTORY);
		}
		else
			newobj = create_object(pObjIndex, number_fuzzy(olevel));

		obj_to_char(newobj, to_mob);
		if (pReset->command == 'E')
			equip_char(to_mob, newobj, pReset->arg3);

		char_printf(ch, "%s (%d) has been loaded "
			"%s of %s (%d) and added to resets.\n\r",
			mlstr_mval(pObjIndex->short_descr),
			pObjIndex->vnum,
			flag_string(wear_loc_strings, pReset->arg3),
			mlstr_mval(to_mob->short_descr),
			to_mob->pIndexData->vnum);
		send_to_char(output, ch);
	}
	else	/* Display Syntax */
	{
		send_to_char("REdit:  That mobile isn't here.\n\r", ch);
		return FALSE;
	}

	act("$n has created $p!", ch, newobj, NULL, TO_ROOM);
	return TRUE;
}



/*
 * Object Editor Functions.
 */
void show_obj_values(BUFFER *output, OBJ_INDEX_DATA *obj)
{
	switch(obj->item_type) {
	default:	/* No values. */
		break;
		     
	case ITEM_LIGHT:
		if (obj->value[2] == -1 || obj->value[2] == 999) /* ROM OLC */
			buf_printf(output, "[v2] Light:  Infinite[-1]\n\r");
		else
			buf_printf(output, "[v2] Light:  [%d]\n\r", obj->value[2]);
		break;

	case ITEM_WAND:
	case ITEM_STAFF:
		buf_printf(output,
			"[v0] Level:          [%d]\n\r"
			"[v1] Charges Total:  [%d]\n\r"
			"[v2] Charges Left:   [%d]\n\r"
			"[v3] Spell:          %s\n\r",
			obj->value[0],
			obj->value[1],
			obj->value[2],
			obj->value[3] != -1 ? skill_table[obj->value[3]].name
			                    : "none");
		break;

	case ITEM_PORTAL:
		buf_printf(output,
			    "[v0] Charges:        [%d]\n\r"
			    "[v1] Exit Flags:     %s\n\r"
			    "[v2] Portal Flags:   %s\n\r"
			    "[v3] Goes to (vnum): [%d]\n\r",
			    obj->value[0],
			    flag_string(exit_flags, obj->value[1]),
			    flag_string(portal_flags , obj->value[2]),
			    obj->value[3]);
		break;
			
	case ITEM_FURNITURE:          
		buf_printf(output,
			    "[v0] Max people:      [%d]\n\r"
			    "[v1] Max weight:      [%d]\n\r"
			    "[v2] Furniture Flags: %s\n\r"
			    "[v3] Heal bonus:      [%d]\n\r"
			    "[v4] Mana bonus:      [%d]\n\r",
			    obj->value[0],
			    obj->value[1],
			    flag_string(furniture_flags, obj->value[2]),
			    obj->value[3],
			    obj->value[4]);
		break;

	case ITEM_SCROLL:
	case ITEM_POTION:
	case ITEM_PILL:
		buf_printf(output,
			"[v0] Level:  [%d]\n\r"
			"[v1] Spell:  %s\n\r"
			"[v2] Spell:  %s\n\r"
			"[v3] Spell:  %s\n\r"
			"[v4] Spell:  %s\n\r",
			obj->value[0],
			obj->value[1] != -1 ? skill_table[obj->value[1]].name
			                    : "none",
			obj->value[2] != -1 ? skill_table[obj->value[2]].name
		                             : "none",
			obj->value[3] != -1 ? skill_table[obj->value[3]].name
			                    : "none",
			obj->value[4] != -1 ? skill_table[obj->value[4]].name
			                    : "none");
		break;

/* ARMOR for ROM */

	case ITEM_ARMOR:
		buf_printf(output,
			"[v0] Ac pierce       [%d]\n\r"
			"[v1] Ac bash         [%d]\n\r"
			"[v2] Ac slash        [%d]\n\r"
			"[v3] Ac exotic       [%d]\n\r",
			obj->value[0],
			obj->value[1],
			obj->value[2],
			obj->value[3]);
			break;

/* WEAPON changed in ROM: */
/* I had to split the output here, I have no idea why, but it helped -- Hugin */
/* It somehow fixed a bug in showing scroll/pill/potions too ?! */
	case ITEM_WEAPON:
		buf_printf(output, "[v0] Weapon class:   %s\n\r",
			    flag_string(weapon_class, obj->value[0]));
		buf_printf(output, "[v1] Number of dice: [%d]\n\r", obj->value[1]);
		buf_printf(output, "[v2] Type of dice:   [%d]\n\r", obj->value[2]);
		buf_printf(output, "[v3] Type:           %s\n\r",
			    attack_table[obj->value[3]].name);
		buf_printf(output, "[v4] Special type:   %s\n\r",
			    flag_string(weapon_type2,  obj->value[4]));
		break;

	case ITEM_CONTAINER:
		buf_printf(output,
			"[v0] Weight:     [%d kg]\n\r"
			"[v1] Flags:      [%s]\n\r"
			"[v2] Key:     %s [%d]\n\r"
			"[v3] Capacity    [%d]\n\r"
			"[v4] Weight Mult [%d]\n\r",
			obj->value[0],
			flag_string(container_flags, obj->value[1]),
		        get_obj_index(obj->value[2]) ?
			mlstr_mval(get_obj_index(obj->value[2])->short_descr) :
			"none",
		        obj->value[2],
		        obj->value[3],
		        obj->value[4]);
		break;

	case ITEM_DRINK_CON:
		buf_printf(output,
			    "[v0] Liquid Total: [%d]\n\r"
			    "[v1] Liquid Left:  [%d]\n\r"
			    "[v2] Liquid:       %s\n\r"
			    "[v3] Poisoned:     %s\n\r",
			    obj->value[0],
			    obj->value[1],
			    liq_table[obj->value[2]].liq_name,
			    obj->value[3] != 0 ? "Yes" : "No");
		break;

	case ITEM_FOUNTAIN:
		buf_printf(output,
			    "[v0] Liquid Total: [%d]\n\r"
			    "[v1] Liquid Left:  [%d]\n\r"
			    "[v2] Liquid:	    %s\n\r",
			    obj->value[0],
			    obj->value[1],
			    liq_table[obj->value[2]].liq_name);
		break;
			    
	case ITEM_FOOD:
		buf_printf(output,
			"[v0] Food hours: [%d]\n\r"
			"[v1] Full hours: [%d]\n\r"
			"[v3] Poisoned:   %s\n\r",
			obj->value[0],
			obj->value[1],
			obj->value[3] != 0 ? "Yes" : "No");
		break;

	case ITEM_MONEY:
		buf_printf(output, "[v0] Gold:   [%d]\n\r", obj->value[0]);
		break;
	}
}



bool set_obj_values(CHAR_DATA *ch, OBJ_INDEX_DATA *pObj,
		    int value_num, const char *argument)
{
	BUFFER *output;

	output = buf_new(0);
	switch(pObj->item_type) {
	default:
		break;
		     
	case ITEM_LIGHT:
		switch (value_num) {
		default:
			do_help(ch, "ITEM_LIGHT");
			return FALSE;
		case 2:
			buf_add(output, "HOURS OF LIGHT SET.\n\r\n\r");
			pObj->value[2] = atoi(argument);
			break;
		}
		break;

	case ITEM_WAND:
	case ITEM_STAFF:
		switch (value_num) {
		default:
			do_help(ch, "ITEM_STAFF_WAND");
			return FALSE;
		case 0:
			buf_add(output, "SPELL LEVEL SET.\n\r\n\r");
			pObj->value[0] = atoi(argument);
			break;
		case 1:
			buf_add(output, "TOTAL NUMBER OF CHARGES SET.\n\r\n\r");
			pObj->value[1] = atoi(argument);
			break;
		case 2:
			buf_add(output, "CURRENT NUMBER OF CHARGES SET.\n\r\n\r");
			pObj->value[2] = atoi(argument);
			break;
		case 3:
			buf_add(output, "SPELL TYPE SET.\n\r");
			pObj->value[3] = skill_lookup(argument);
			break;
		}
		break;

	case ITEM_SCROLL:
	case ITEM_POTION:
	case ITEM_PILL:
		switch (value_num) {
		default:
			do_help(ch, "ITEM_SCROLL_POTION_PILL");
			return FALSE;
		case 0:
			buf_add(output, "SPELL LEVEL SET.\n\r\n\r");
			pObj->value[0] = atoi(argument);
			break;
		case 1:
			buf_add(output, "SPELL TYPE 1 SET.\n\r\n\r");
			pObj->value[1] = skill_lookup(argument);
			break;
		case 2:
			buf_add(output, "SPELL TYPE 2 SET.\n\r\n\r");
			pObj->value[2] = skill_lookup(argument);
			break;
		case 3:
			buf_add(output, "SPELL TYPE 3 SET.\n\r\n\r");
			pObj->value[3] = skill_lookup(argument);
			break;
		case 4:
			buf_add(output, "SPELL TYPE 4 SET.\n\r\n\r");
			pObj->value[4] = skill_lookup(argument);
			break;
 		}
		break;

/* ARMOR for ROM: */

	case ITEM_ARMOR:
		switch (value_num) {
		default:
			do_help(ch, "ITEM_ARMOR");
			return FALSE;
		case 0:
			buf_add(output, "AC PIERCE SET.\n\r\n\r");
			pObj->value[0] = atoi(argument);
			break;
		case 1:
			buf_add(output, "AC BASH SET.\n\r\n\r");
			pObj->value[1] = atoi(argument);
			break;
		case 2:
			buf_add(output, "AC SLASH SET.\n\r\n\r");
			pObj->value[2] = atoi(argument);
			break;
		case 3:
			buf_add(output, "AC EXOTIC SET.\n\r\n\r");
			pObj->value[3] = atoi(argument);
			break;
		}
		break;

/* WEAPONS changed in ROM */

	case ITEM_WEAPON:
		switch (value_num) {
		default:
			do_help(ch, "ITEM_WEAPON");
			return FALSE;
		case 0:
			buf_add(output, "WEAPON CLASS SET.\n\r\n\r");
			pObj->value[0] = flag_value(weapon_class, argument);
			break;
		case 1:
			buf_add(output, "NUMBER OF DICE SET.\n\r\n\r");
			pObj->value[1] = atoi(argument);
			break;
		case 2:
			buf_add(output, "TYPE OF DICE SET.\n\r\n\r");
			pObj->value[2] = atoi(argument);
			break;
		case 3:
			buf_add(output, "WEAPON TYPE SET.\n\r\n\r");
			pObj->value[3] = attack_lookup(argument);
			break;
		case 4:
			buf_add(output, "SPECIAL WEAPON TYPE TOGGLED.\n\r\n\r");
			pObj->value[4] ^= (flag_value(weapon_type2, argument) != NO_FLAG
			? flag_value(weapon_type2, argument) : 0);
			break;
		}
		break;

	case ITEM_PORTAL:
		switch (value_num) {
		default:
			do_help(ch, "ITEM_PORTAL");
			return FALSE;
			        
		case 0:
			buf_add(output, "CHARGES SET.\n\r\n\r");
				     pObj->value[0] = atoi (argument);
			break;
		case 1:
			buf_add(output, "EXIT FLAGS SET.\n\r\n\r");
			pObj->value[1] = flag_value(exit_flags, argument);
			break;
		case 2:
			buf_add(output, "PORTAL FLAGS SET.\n\r\n\r");
			pObj->value[2] = flag_value(portal_flags, argument);
			break;
		case 3:
			buf_add(output, "EXIT VNUM SET.\n\r\n\r");
			pObj->value[3] = atoi (argument);
			break;
		}
		break;

	case ITEM_FURNITURE:
		switch (value_num) {
		default:
			do_help(ch, "ITEM_FURNITURE");
			return FALSE;
			        
		case 0:
			buf_add(output, "NUMBER OF PEOPLE SET.\n\r\n\r");
			pObj->value[0] = atoi (argument);
			break;
		case 1:
			buf_add(output, "MAX WEIGHT SET.\n\r\n\r");
			pObj->value[1] = atoi (argument);
			break;
		case 2:
		        buf_add(output, "FURNITURE FLAGS TOGGLED.\n\r\n\r");
			pObj->value[2] ^= (flag_value(furniture_flags, argument) != NO_FLAG
			? flag_value(furniture_flags, argument) : 0);
			break;
		case 3:
			buf_add(output, "HEAL BONUS SET.\n\r\n\r");
			pObj->value[3] = atoi (argument);
			break;
		case 4:
			buf_add(output, "MANA BONUS SET.\n\r\n\r");
			pObj->value[4] = atoi (argument);
			break;
		}
		break;
		   
	case ITEM_CONTAINER:
		switch (value_num) {
			int value;
			
		default:
			do_help(ch, "ITEM_CONTAINER");
			return FALSE;
		case 0:
			buf_add(output, "WEIGHT CAPACITY SET.\n\r\n\r");
			pObj->value[0] = atoi(argument);
			break;
		case 1:
			if ((value = flag_value(container_flags, argument))
			          != NO_FLAG)
				TOGGLE_BIT(pObj->value[1], value);
			else {
				do_help (ch, "ITEM_CONTAINER");
				return FALSE;
			}
			buf_add(output, "CONTAINER TYPE SET.\n\r\n\r");
			break;
		case 2:
			if (atoi(argument) != 0) {
				if (!get_obj_index(atoi(argument))) {
					buf_add(output, "THERE IS NO SUCH ITEM.\n\r\n\r");
					return FALSE;
				}

				if (get_obj_index(atoi(argument))->item_type != ITEM_KEY) {
					buf_add(output, "THAT ITEM IS NOT A KEY.\n\r\n\r");
					return FALSE;
				}
			}
			buf_add(output, "CONTAINER KEY SET.\n\r\n\r");
			pObj->value[2] = atoi(argument);
			break;
		case 3:
			buf_add(output, "CONTAINER MAX WEIGHT SET.\n\r");
			pObj->value[3] = atoi(argument);
			break;
		case 4:
			buf_add(output, "WEIGHT MULTIPLIER SET.\n\r\n\r");
			pObj->value[4] = atoi (argument);
			break;
		}
		break;

	case ITEM_DRINK_CON:
		switch (value_num) {
		default:
			do_help(ch, "ITEM_DRINK");
/* OLC		    do_help(ch, "liquids");    */
			return FALSE;
		case 0:
			buf_add(output, "MAXIMUM AMOUT OF LIQUID HOURS SET.\n\r\n\r");
			pObj->value[0] = atoi(argument);
			break;
		case 1:
			buf_add(output, "CURRENT AMOUNT OF LIQUID HOURS SET.\n\r\n\r");
			pObj->value[1] = atoi(argument);
			break;
		case 2:
			buf_add(output, "LIQUID TYPE SET.\n\r\n\r");
			pObj->value[2] = (liq_lookup(argument) != -1 ?
			        		       liq_lookup(argument) : 0);
			break;
		case 3:
			buf_add(output, "POISON VALUE TOGGLED.\n\r\n\r");
			pObj->value[3] = (pObj->value[3] == 0) ? 1 : 0;
			break;
		}
		break;

	case ITEM_FOUNTAIN:
		switch (value_num) {
		default:
			do_help(ch, "ITEM_FOUNTAIN");
/* OLC		    do_help(ch, "liquids");    */
			return FALSE;
		case 0:
			buf_add(output, "MAXIMUM AMOUT OF LIQUID HOURS SET.\n\r\n\r");
			pObj->value[0] = atoi(argument);
			break;
		case 1:
			buf_add(output, "CURRENT AMOUNT OF LIQUID HOURS SET.\n\r\n\r");
			pObj->value[1] = atoi(argument);
			break;
		case 2:
			buf_add(output, "LIQUID TYPE SET.\n\r\n\r");
			pObj->value[2] = (liq_lookup(argument) != -1 ?
						liq_lookup(argument) : 0);
			break;
		}
		break;
			    	
	case ITEM_FOOD:
		switch (value_num) {
		default:
			do_help(ch, "ITEM_FOOD");
			return FALSE;
		case 0:
			buf_add(output, "HOURS OF FOOD SET.\n\r\n\r");
			pObj->value[0] = atoi(argument);
			break;
		case 1:
			buf_add(output, "HOURS OF FULL SET.\n\r\n\r");
			pObj->value[1] = atoi(argument);
			break;
		case 3:
			buf_add(output, "POISON VALUE TOGGLED.\n\r\n\r");
			pObj->value[3] = (pObj->value[3] == 0) ? 1 : 0;
			break;
		}
		break;

	case ITEM_MONEY:
		switch (value_num) {
		default:
			do_help(ch, "ITEM_MONEY");
			return FALSE;
		case 0:
			buf_add(output, "GOLD AMOUNT SET.\n\r\n\r");
			pObj->value[0] = atoi(argument);
			break;
		case 1:
			buf_add(output, "SILVER AMOUNT SET.\n\r\n\r");
			pObj->value[1] = atoi(argument);
			break;
		}
		break;
	}

	show_obj_values(output, pObj);

	char_puts(buf_string(output), ch);
	buf_free(output);

	return TRUE;
}



OEDIT(oedit_show)
{
	OBJ_INDEX_DATA *pObj;
	AFFECT_DATA *paf;
	int cnt;
	BUFFER *output;

	EDIT_OBJ(ch, pObj);

	output = buf_new(0);
	buf_printf(output, "Name:        [%s]\n\rArea:        [%5d] %s\n\r",
		pObj->name,
		!pObj->area ? -1        : pObj->area->vnum,
		!pObj->area ? "No Area" : pObj->area->name);


	buf_printf(output, "Vnum:        [%5d]\n\rType:        [%s]\n\r",
		pObj->vnum,
		flag_string(type_flags, pObj->item_type));

	if (pObj->limit == -1)
		buf_printf(output, "Limit:       [%5d]\n\r", pObj->limit);
	else
		buf_add(output, "Limit:       [none]\n\r");

	buf_printf(output, "Level:       [%5d]\n\r", pObj->level);

	buf_printf(output, "Wear flags:  [%s]\n\r",
		flag_string(wear_flags, pObj->wear_flags));

	buf_printf(output, "Extra flags: [%s]\n\r",
		flag_string(extra_flags, pObj->extra_flags));

	buf_printf(output, "Material:    [%s]\n\r",                /* ROM */
		pObj->material);

	buf_printf(output, "Condition:   [%5d]\n\r",               /* ROM */
		pObj->condition);

	buf_printf(output, "Weight:      [%5d]\n\rCost:        [%5d]\n\r",
		pObj->weight, pObj->cost);

	if (pObj->ed) {
		ED_DATA *ed;

		buf_add(output, "Ex desc kwd: ");

		for (ed = pObj->ed; ed; ed = ed->next)
			buf_printf(output, "[%s]", ed->keyword);

		buf_add(output, "\n\r");
	}

	mlstr_dump(output, "Short desc: ", pObj->short_descr);
	mlstr_dump(output, "Long desc: ", pObj->description);

	for (cnt = 0, paf = pObj->affected; paf; paf = paf->next) {
		if (cnt == 0) {
			buf_add(output, "Number Modifier Affects\n\r");
			buf_add(output, "------ -------- -------\n\r");
		}
		buf_printf(output, "[%4d] %-8d %s\n\r", cnt,
			paf->modifier,
			flag_string(apply_flags, paf->location));
		cnt++;
	}

	show_obj_values(output, pObj);
	char_puts(buf_string(output), ch);
	buf_free(output);

	return FALSE;
}


/*
 * Need to issue warning if flag isn't valid. -- does so now -- Hugin.
 */
OEDIT(oedit_addaffect)
{
	int value;
	OBJ_INDEX_DATA *pObj;
	AFFECT_DATA *pAf;
	char loc[MAX_STRING_LENGTH];
	char mod[MAX_STRING_LENGTH];

	EDIT_OBJ(ch, pObj);

	argument = one_argument(argument, loc);
	one_argument(argument, mod);

	if (loc[0] == '\0' || mod[0] == '\0' || !is_number(mod))
	{
		send_to_char("Syntax:  addaffect [location] [#xmod]\n\r", ch);
		return FALSE;
	}

	if ((value = flag_value(apply_flags, loc)) == NO_FLAG) /* Hugin */
	{
		send_to_char("Valid affects are:\n\r", ch);
		show_help(ch, "apply");
		return FALSE;
	}

	pAf             =   new_affect();
	pAf->location   =   value;
	pAf->modifier   =   atoi(mod);
	pAf->where	    =   TO_OBJECT;
	pAf->type       =   -1;
	pAf->duration   =   -1;
	pAf->bitvector  =   0;
	pAf->level      =	pObj->level;
	pAf->next       =   pObj->affected;
	pObj->affected  =   pAf;

	send_to_char("Affect added.\n\r", ch);
	return TRUE;
}

OEDIT(oedit_addapply)
{
	int value,bv,typ;
	OBJ_INDEX_DATA *pObj;
	AFFECT_DATA *pAf;
	char loc[MAX_STRING_LENGTH];
	char mod[MAX_STRING_LENGTH];
	char type[MAX_STRING_LENGTH];
	char bvector[MAX_STRING_LENGTH];

	EDIT_OBJ(ch, pObj);

	argument = one_argument(argument, type);
	argument = one_argument(argument, loc);
	argument = one_argument(argument, mod);
	one_argument(argument, bvector);

	if (type[0] == '\0' || (typ = flag_value(apply_types, type)) == NO_FLAG)
	{
		send_to_char("Invalid apply type. Valid apply types are:\n\r", ch);
		show_help(ch, "apptype");
		return FALSE;
	}

	if (loc[0] == '\0' || (value = flag_value(apply_flags, loc)) == NO_FLAG)
	{
		 send_to_char("Valid applys are:\n\r", ch);
		show_help(ch, "apply");
		return FALSE;
	}

	if (bvector[0] == '\0' || (bv = flag_value(bitvector_type[typ].table, bvector)) == NO_FLAG)
	{
		send_to_char("Invalid bitvector type.\n\r", ch);
		send_to_char("Valid bitvector types are:\n\r", ch);
		show_help(ch, bitvector_type[typ].help);
		return FALSE;
	}

	if (mod[0] == '\0' || !is_number(mod))
	{
		send_to_char("Syntax:  addapply [type] [location] [#xmod] [bitvector]\n\r", ch);
		return FALSE;
	}

	pAf             =   new_affect();
	pAf->location   =   value;
	pAf->modifier   =   atoi(mod);
	pAf->where	    =   apply_types[typ].bit;
	pAf->type	    =	-1;
	pAf->duration   =   -1;
	pAf->bitvector  =   bv;
	pAf->level      =	pObj->level;
	pAf->next       =   pObj->affected;
	pObj->affected  =   pAf;

	send_to_char("Apply added.\n\r", ch);
	return TRUE;
}

/*
 * My thanks to Hans Hvidsten Birkeland and Noam Krendel(Walker)
 * for really teaching me how to manipulate pointers.
 */
OEDIT(oedit_delaffect)
{
	OBJ_INDEX_DATA *pObj;
	AFFECT_DATA *pAf;
	AFFECT_DATA *pAf_next;
	char affect[MAX_STRING_LENGTH];
	int  value;
	int  cnt = 0;

	EDIT_OBJ(ch, pObj);

	one_argument(argument, affect);

	if (!is_number(affect) || affect[0] == '\0')
	{
		send_to_char("Syntax:  delaffect [#xaffect]\n\r", ch);
		return FALSE;
	}

	value = atoi(affect);

	if (value < 0)
	{
		send_to_char("Only non-negative affect-numbers allowed.\n\r", ch);
		return FALSE;
	}

	if (!(pAf = pObj->affected))
	{
		send_to_char("OEdit:  Non-existant affect.\n\r", ch);
		return FALSE;
	}

	if(value == 0)	/* First case: Remove first affect */
	{
		pAf = pObj->affected;
		pObj->affected = pAf->next;
		free_affect(pAf);
	}
	else		/* Affect to remove is not the first */
	{
		while ((pAf_next = pAf->next) && (++cnt < value))
			 pAf = pAf_next;

		if(pAf_next)		/* See if it's the next affect */
		{
			pAf->next = pAf_next->next;
			free_affect(pAf_next);
		}
		else                                 /* Doesn't exist */
		{
			 send_to_char("No such affect.\n\r", ch);
			 return FALSE;
		}
	}

	send_to_char("Affect removed.\n\r", ch);
	return TRUE;
}



OEDIT(oedit_name)
{
	OBJ_INDEX_DATA *pObj;

	EDIT_OBJ(ch, pObj);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  name [string]\n\r", ch);
		return FALSE;
	}

	free_string(pObj->name);
	pObj->name = str_dup(argument);

	send_to_char("Name set.\n\r", ch);
	return TRUE;
}



OEDIT(oedit_short)
{
	OBJ_INDEX_DATA *pObj;

	EDIT_OBJ(ch, pObj);

	if (argument[0] == '\0') {
		send_to_char("Syntax:  short lang [string]\n\r", ch);
		return FALSE;
	}

	mlstr_change(pObj->short_descr, argument);
	send_to_char("Short description set.\n\r", ch);
	return TRUE;
}



OEDIT(oedit_long)
{
	OBJ_INDEX_DATA *pObj;

	EDIT_OBJ(ch, pObj);

	if (argument[0] == '\0') {
		send_to_char("Syntax:  long lang [string]\n\r", ch);
		return FALSE;
	}

	mlstr_change(pObj->description, argument);
	send_to_char("Long description set.\n\r", ch);
	return TRUE;
}



bool set_value(CHAR_DATA *ch, OBJ_INDEX_DATA *pObj, const char *argument, int value)
{
	if (argument[0] == '\0')
	{
		set_obj_values(ch, pObj, -1, "");     /* '\0' changed to "" -- Hugin */
		return FALSE;
	}

	if (set_obj_values(ch, pObj, value, argument))
		return TRUE;

	return FALSE;
}



/*****************************************************************************
 Name:		oedit_values
 Purpose:	Finds the object and sets its value.
 Called by:	The four valueX functions below. (now five -- Hugin)
 ****************************************************************************/
bool oedit_values(CHAR_DATA *ch, const char *argument, int value)
{
	OBJ_INDEX_DATA *pObj;

	EDIT_OBJ(ch, pObj);

	if (set_value(ch, pObj, argument, value))
		 return TRUE;

	return FALSE;
}

OEDIT(oedit_value0)
{
	if (oedit_values(ch, argument, 0))
		 return TRUE;

	return FALSE;
}

OEDIT(oedit_value1)
{
	if (oedit_values(ch, argument, 1))
		 return TRUE;

	return FALSE;
}

OEDIT(oedit_value2)
{
	if (oedit_values(ch, argument, 2))
		 return TRUE;

	return FALSE;
}

OEDIT(oedit_value3)
{
	if (oedit_values(ch, argument, 3))
		 return TRUE;

	return FALSE;
}

OEDIT(oedit_value4)
{
	if (oedit_values(ch, argument, 4))
		 return TRUE;

	return FALSE;
}

OEDIT(oedit_weight)
{
	OBJ_INDEX_DATA *pObj;

	EDIT_OBJ(ch, pObj);

	if (argument[0] == '\0' || !is_number(argument))
	{
		send_to_char("Syntax:  weight [number]\n\r", ch);
		return FALSE;
	}

	pObj->weight = atoi(argument);

	send_to_char("Weight set.\n\r", ch);
	return TRUE;
}

OEDIT(oedit_limit)
{
	OBJ_INDEX_DATA *pObj;

	EDIT_OBJ(ch, pObj);

	if (str_cmp(argument, "none") == NULL) {
		pObj->limit = -1;
		char_puts("Limit removed.\n\r", ch);
		return TRUE;
	}

	if (argument[0] == '\0' || !is_number(argument)) {
		send_to_char("Syntax:  olimit [number | none]\n\r", ch);
		return FALSE;
	}

	pObj->limit = atoi(argument);

	char_puts("Limit set.\n\r", ch);
	return TRUE;
}

OEDIT(oedit_cost)
{
	OBJ_INDEX_DATA *pObj;

	EDIT_OBJ(ch, pObj);

	if (argument[0] == '\0' || !is_number(argument))
	{
		send_to_char("Syntax:  cost [number]\n\r", ch);
		return FALSE;
	}

	pObj->cost = atoi(argument);

	send_to_char("Cost set.\n\r", ch);
	return TRUE;
}

OEDIT(oedit_create)
{
	OBJ_INDEX_DATA *pObj;
	AREA_DATA *pArea;
	int  value;
	int  iHash;

	value = atoi(argument);
	if (argument[0] == '\0' || value == 0)
	{
		send_to_char("Syntax:  oedit create [vnum]\n\r", ch);
		return FALSE;
	}

	pArea = get_vnum_area(value);
	if (!pArea)
	{
		send_to_char("OEdit:  That vnum is not assigned an area.\n\r", ch);
		return FALSE;
	}

	if (!IS_BUILDER(ch, pArea))
	{
		send_to_char("OEdit:  Vnum in an area you cannot build in.\n\r", ch);
		return FALSE;
	}

	if (get_obj_index(value))
	{
		send_to_char("OEdit:  Object vnum already exists.\n\r", ch);
		return FALSE;
	}
		 
	pObj			= new_obj_index();
	pObj->vnum			= value;
	pObj->area			= pArea;
		 
	if (value > top_vnum_obj)
		top_vnum_obj = value;

	iHash			= value % MAX_KEY_HASH;
	pObj->next			= obj_index_hash[iHash];
	obj_index_hash[iHash]	= pObj;
	ch->desc->pEdit		= (void *)pObj;

	send_to_char("Object Created.\n\r", ch);
	return TRUE;
}



OEDIT(oedit_ed)
{
	OBJ_INDEX_DATA *pObj;
	ED_DATA *ed;
	char command[MAX_INPUT_LENGTH];
	char keyword[MAX_INPUT_LENGTH];
	char lang[MAX_INPUT_LENGTH];

	EDIT_OBJ(ch, pObj);

	argument = one_argument(argument, command);
	argument = one_argument(argument, keyword);
		   one_argument(argument, lang);

	if (command[0] == '\0') {
		send_to_char("Syntax:  ed add [keyword]\n\r", ch);
		send_to_char("         ed delete [keyword]\n\r", ch);
		send_to_char("         ed edit [keyword]\n\r", ch);
		send_to_char("         ed format [keyword]\n\r", ch);
		return FALSE;
	}

	if (!str_cmp(command, "add")) {
		if (keyword[0] == '\0' || lang[0] == '\0') {
			send_to_char("Syntax:  ed add keyword lang\n\r", ch);
			return FALSE;
		}

		ed			= ed_new();
		ed->keyword		= str_dup(keyword);
		ed->description		= mlstr_new();
		ed->next		= pObj->ed;
		pObj->ed	= ed;

		mlstr_append(ch, ed->description, lang);

		return TRUE;
	}

	if (!str_cmp(command, "edit")) {
		if (keyword[0] == '\0' || lang[0] == '\0') {
			send_to_char("Syntax:  ed edit [keyword]\n\r", ch);
			return FALSE;
		}

		ed = ed_lookup(keyword, pObj->ed);
		if (ed == NULL) {
			send_to_char("OEdit:  Extra description keyword not found.\n\r", ch);
			return FALSE;
		}

		mlstr_append(ch, ed->description, lang);
		return TRUE;
	}

	if (!str_cmp(command, "delete")) {
		ED_DATA *ped = NULL;

		if (keyword[0] == '\0') {
			send_to_char("Syntax:  ed delete keyword\n\r", ch);
			return FALSE;
		}

		for (ed = pObj->ed; ed; ed = ed->next) {
			if (is_name(keyword, ed->keyword))
				break;
			ped = ed;
		}

		if (ed == NULL) {
			send_to_char("OEdit:  Extra description keyword not found.\n\r", ch);
			return FALSE;
		}

		if (ped == NULL)
			pObj->ed = ed->next;
		else
			ped->next = ed->next;

		free_ed(ed);

		send_to_char("Extra description deleted.\n\r", ch);
		return TRUE;
	}


	if (!str_cmp(command, "format")) {
		if (keyword[0] == '\0') {
			send_to_char("Syntax:  ed format keyword\n\r", ch);
			return FALSE;
		}

		ed = ed_lookup(keyword, pObj->ed);
		if (ed == NULL) {
		         send_to_char("OEdit:  Extra description keyword not found.\n\r", ch);
		         return FALSE;
		}

		mlstr_format(ed->description);

		send_to_char("Extra description formatted.\n\r", ch);
		return TRUE;
	}

	oedit_ed(ch, "");
	return FALSE;
}





/* ROM object functions : */

OEDIT(oedit_extra)      /* Moved out of oedit() due to naming conflicts -- Hugin */
{
	OBJ_INDEX_DATA *pObj;
	int value;

	if (argument[0] != '\0')
	{
		EDIT_OBJ(ch, pObj);

		if ((value = flag_value(extra_flags, argument)) != NO_FLAG)
		{
			TOGGLE_BIT(pObj->extra_flags, value);

			send_to_char("Extra flag toggled.\n\r", ch);
			return TRUE;
		}
	}

	send_to_char("Syntax:  extra [flag]\n\r"
			  "Type '? extra' for a list of flags.\n\r", ch);
	return FALSE;
}


OEDIT(oedit_wear)      /* Moved out of oedit() due to naming conflicts -- Hugin */
{
	OBJ_INDEX_DATA *pObj;
	int value;

	 if (argument[0] != '\0')
	{
		EDIT_OBJ(ch, pObj);

		if ((value = flag_value(wear_flags, argument)) != NO_FLAG)
		{
			TOGGLE_BIT(pObj->wear_flags, value);

			send_to_char("Wear flag toggled.\n\r", ch);
			return TRUE;
		}
	}

	send_to_char("Syntax:  wear [flag]\n\r"
			  "Type '? wear' for a list of flags.\n\r", ch);
	return FALSE;
}


OEDIT(oedit_type)      /* Moved out of oedit() due to naming conflicts -- Hugin */
{
	OBJ_INDEX_DATA *pObj;
	int value;

	if (argument[0] != '\0')
	{
		EDIT_OBJ(ch, pObj);

		if ((value = flag_value(type_flags, argument)) != NO_FLAG)
		{
			pObj->item_type = value;

			send_to_char("Type set.\n\r", ch);

			/*
			 * Clear the values.
			 */
			pObj->value[0] = 0;
			pObj->value[1] = 0;
			pObj->value[2] = 0;
			pObj->value[3] = 0;
			pObj->value[4] = 0;     /* ROM */

			return TRUE;
		}
	}

	send_to_char("Syntax:  type [flag]\n\r"
			  "Type '? type' for a list of flags.\n\r", ch);
	return FALSE;
}

OEDIT(oedit_material)
{
	OBJ_INDEX_DATA *pObj;

	EDIT_OBJ(ch, pObj);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  material [string]\n\r", ch);
		return FALSE;
	}

	free_string(pObj->material);
	pObj->material = str_dup(argument);

	send_to_char("Material set.\n\r", ch);
	return TRUE;
}

OEDIT(oedit_level)
{
	OBJ_INDEX_DATA *pObj;

	EDIT_OBJ(ch, pObj);

	if (argument[0] == '\0' || !is_number(argument))
	{
		send_to_char("Syntax:  level [number]\n\r", ch);
		return FALSE;
	}

	pObj->level = atoi(argument);

	send_to_char("Level set.\n\r", ch);
	return TRUE;
}



OEDIT(oedit_condition)
{
	OBJ_INDEX_DATA *pObj;
	int value;

	if (argument[0] != '\0'
	&& (value = atoi (argument)) >= 0
	&& (value <= 100))
	{
		EDIT_OBJ(ch, pObj);

		pObj->condition = value;
		send_to_char("Condition set.\n\r", ch);

		return TRUE;
	}

	send_to_char("Syntax:  condition [number]\n\r"
			  "Where number can range from 0 (ruined) to 100 (perfect).\n\r",
			  ch);
	return FALSE;
}





/*
 * Mobile Editor Functions.
 */
MEDIT(medit_show)
{
	MOB_INDEX_DATA *pMob;
	MPROG_LIST *list;
	BUFFER *buf;

	EDIT_MOB(ch, pMob);

	buf = buf_new(0);

	buf_printf(buf, "Name:        [%s]\n\rArea:        [%5d] %s\n\r",
		pMob->player_name,
		!pMob->area ? -1        : pMob->area->vnum,
		!pMob->area ? "No Area" : pMob->area->name);

	buf_printf(buf, "Act:         [%s]\n\r",
		flag_string(act_flags, pMob->act));

	buf_printf(buf, "Vnum:        [%5d] Sex:   [%s]   Race: [%s]\n\r",
		pMob->vnum,
		flag_string(sex_flags, pMob->sex),
		race_table[pMob->race].name);

	buf_printf(buf, "Level:       [%2d]    Align: [%4d]      Hitroll: [%2d] Dam Type:    [%s]\n\r",
		pMob->level,	pMob->alignment,
		pMob->hitroll,	attack_table[pMob->dam_type].name);

	if (pMob->group)
		buf_printf(buf, "Group:       [%5d]\n\r", pMob->group);

	buf_printf(buf, "Hit dice:    [%2dd%-3d+%4d] ",
			 pMob->hit[DICE_NUMBER],
			 pMob->hit[DICE_TYPE],
			 pMob->hit[DICE_BONUS]);

	buf_printf(buf, "Damage dice: [%2dd%-3d+%4d] ",
			 pMob->damage[DICE_NUMBER],
			 pMob->damage[DICE_TYPE],
			 pMob->damage[DICE_BONUS]);

	buf_printf(buf, "Mana dice:   [%2dd%-3d+%4d]\n\r",
			 pMob->mana[DICE_NUMBER],
			 pMob->mana[DICE_TYPE],
			 pMob->mana[DICE_BONUS]);

/* ROM values end */

	buf_printf(buf, "Affected by: [%s]\n\r",
		flag_string(affect_flags, pMob->affected_by));
	buf_printf(buf, "Can detect:  [%s]\n\r",
		flag_string(detect_flags, pMob->detection));

/* ROM values: */

	buf_printf(buf, "Armor:       [pierce: %d  bash: %d  slash: %d  magic: %d]\n\r",
		pMob->ac[AC_PIERCE], pMob->ac[AC_BASH],
		pMob->ac[AC_SLASH], pMob->ac[AC_EXOTIC]);

	buf_printf(buf, "Form:        [%s]\n\r",
		flag_string(form_flags, pMob->form));

	buf_printf(buf, "Parts:       [%s]\n\r",
		flag_string(part_flags, pMob->parts));

	buf_printf(buf, "Imm:         [%s]\n\r",
		flag_string(imm_flags, pMob->imm_flags));

	buf_printf(buf, "Res:         [%s]\n\r",
		flag_string(res_flags, pMob->res_flags));

	buf_printf(buf, "Vuln:        [%s]\n\r",
		flag_string(vuln_flags, pMob->vuln_flags));

	buf_printf(buf, "Off:         [%s]\n\r",
		flag_string(off_flags,  pMob->off_flags));

	buf_printf(buf, "Size:        [%s]\n\r",
		flag_string(size_flags, pMob->size));

	buf_printf(buf, "Material:    [%s]\n\r",
		 pMob->material);

	buf_printf(buf, "Start pos.   [%s]\n\r",
		flag_string(position_flags, pMob->start_pos));

	buf_printf(buf, "Default pos  [%s]\n\r",
		flag_string(position_flags, pMob->default_pos));

	buf_printf(buf, "Wealth:      [%5d]\n\r", pMob->wealth);

/* ROM values end */

	if (pMob->spec_fun)
		buf_printf(buf, "Spec fun:    [%s]\n\r",  spec_name(pMob->spec_fun));
	if (pMob->practicer)
		buf_printf(buf, "Practicer:   [%s]\n\r",
			flag_string(skill_groups, pMob->practicer));

	mlstr_dump(buf, "Short descr: ", pMob->short_descr);
	mlstr_dump(buf, "Long descr: ", pMob->long_descr);
	mlstr_dump(buf, "Description: ", pMob->description);

	if (pMob->pShop) {
		SHOP_DATA *pShop;
		int iTrade;

		pShop = pMob->pShop;

		buf_printf(buf, "Shop data for [%5d]:\n\r"
				"  Markup for purchaser: %d%%\n\r"
				"  Markdown for seller:  %d%%\n\r",
			pShop->keeper, pShop->profit_buy, pShop->profit_sell);
		buf_printf(buf, "  Hours: %d to %d.\n\r",
			pShop->open_hour, pShop->close_hour);

		for (iTrade = 0; iTrade < MAX_TRADE; iTrade++) {
			if (pShop->buy_type[iTrade] != 0) {
			if (iTrade == 0) {
				buf_add(buf, "  Number Trades Type\n\r");
				buf_add(buf, "  ------ -----------\n\r");
			}
			buf_printf(buf, "  [%4d] %s\n\r", iTrade,
				flag_string(type_flags, pShop->buy_type[iTrade]));
			}
		}
	}

	if (pMob->mprogs) {
		int cnt;

		buf_printf(buf, "\n\rMOBPrograms for [%5d]:\n\r", pMob->vnum);

		for (cnt=0, list=pMob->mprogs; list; list=list->next) {
			if (cnt ==0) {
				buf_add(buf, " Number Vnum Trigger Phrase\n\r");
				buf_add(buf, " ------ ---- ------- ------\n\r");
			}

			buf_printf(buf, "[%5d] %4d %7s %s\n\r", cnt,
			list->vnum,mprog_type_to_name(list->trig_type),
			list->trig_phrase);
			cnt++;
		}
	}

	char_puts(buf_string(buf), ch);
	buf_free(buf);

	return FALSE;
}



MEDIT(medit_create)
{
	MOB_INDEX_DATA *pMob;
	AREA_DATA *pArea;
	int  value;
	int  iHash;

	value = atoi(argument);
	if (argument[0] == '\0' || value == 0)
	{
		send_to_char("Syntax:  medit create [vnum]\n\r", ch);
		return FALSE;
	}

	pArea = get_vnum_area(value);

	if (!pArea)
	{
		send_to_char("MEdit:  That vnum is not assigned an area.\n\r", ch);
		return FALSE;
	}

	if (!IS_BUILDER(ch, pArea))
	{
		send_to_char("MEdit:  Vnum in an area you cannot build in.\n\r", ch);
		return FALSE;
	}

	if (get_mob_index(value))
	{
		send_to_char("MEdit:  Mobile vnum already exists.\n\r", ch);
		return FALSE;
	}

	pMob			= new_mob_index();
	pMob->vnum		= value;
	pMob->area		= pArea;
		 
	if (value > top_vnum_mob)
		top_vnum_mob = value;        

	pMob->act		= ACT_NPC;
	iHash			= value % MAX_KEY_HASH;
	pMob->next		= mob_index_hash[iHash];
	mob_index_hash[iHash]	= pMob;
	ch->desc->pEdit		= (void *)pMob;

	send_to_char("Mobile Created.\n\r", ch);
	return TRUE;
}



MEDIT(medit_spec)
{
	MOB_INDEX_DATA *pMob;

	EDIT_MOB(ch, pMob);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  spec [special function]\n\r", ch);
		return FALSE;
	}


	if (!str_cmp(argument, "none"))
	{
		 pMob->spec_fun = NULL;

		 send_to_char("Spec removed.\n\r", ch);
		 return TRUE;
	}

	if (spec_lookup(argument))
	{
		pMob->spec_fun = spec_lookup(argument);
		send_to_char("Spec set.\n\r", ch);
		return TRUE;
	}

	send_to_char("MEdit: No such special function.\n\r", ch);
	return FALSE;
}

MEDIT(medit_damtype)
{
	MOB_INDEX_DATA *pMob;

	EDIT_MOB(ch, pMob);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  damtype [damage message]\n\r", ch);
		send_to_char("Para ver una lista de tipos de mensajes, pon '? weapon'.\n\r", ch);
		return FALSE;
	}

	pMob->dam_type = attack_lookup(argument);
	send_to_char("Damage type set.\n\r", ch);
	return TRUE;
}


MEDIT(medit_align)
{
	MOB_INDEX_DATA *pMob;

	EDIT_MOB(ch, pMob);

	if (argument[0] == '\0' || !is_number(argument))
	{
		send_to_char("Syntax:  alignment [number]\n\r", ch);
		return FALSE;
	}

	pMob->alignment = atoi(argument);

	send_to_char("Alignment set.\n\r", ch);
	return TRUE;
}



MEDIT(medit_level)
{
	MOB_INDEX_DATA *pMob;

	EDIT_MOB(ch, pMob);

	if (argument[0] == '\0' || !is_number(argument))
	{
		send_to_char("Syntax:  level [number]\n\r", ch);
		return FALSE;
	}

	pMob->level = atoi(argument);

	send_to_char("Level set.\n\r", ch);
	return TRUE;
}



MEDIT(medit_desc)
{
	MOB_INDEX_DATA *pMob;

	EDIT_MOB(ch, pMob);

	if (argument[0] != '\0') {
		mlstr_append(ch, pMob->description, argument);
		return TRUE;
	}

	send_to_char("Syntax:  desc lang\n\r", ch);
	return FALSE;
}




MEDIT(medit_long)
{
	MOB_INDEX_DATA *pMob;

	EDIT_MOB(ch, pMob);

	if (!mlstr_change_desc(pMob->long_descr, argument)) {
		send_to_char("Syntax: long lang [string]\n\r", ch);
		return FALSE;
	}

	send_to_char("Long description set.\n\r", ch);
	return TRUE;
}



MEDIT(medit_short)
{
	MOB_INDEX_DATA *pMob;

	EDIT_MOB(ch, pMob);

	if (argument[0] == '\0') {
		send_to_char("Syntax:  short lang [string]\n\r", ch);
		return FALSE;
	}

	mlstr_change(pMob->short_descr, argument);
	send_to_char("Short description set.\n\r", ch);
	return TRUE;
}



MEDIT(medit_name)
{
	MOB_INDEX_DATA *pMob;

	EDIT_MOB(ch, pMob);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  name [string]\n\r", ch);
		return FALSE;
	}

	free_string(pMob->player_name);
	pMob->player_name = str_dup(argument);

	send_to_char("Name set.\n\r", ch);
	return TRUE;
}

MEDIT(medit_shop)
{
	MOB_INDEX_DATA *pMob;
	char command[MAX_INPUT_LENGTH];
	char arg1[MAX_INPUT_LENGTH];

	argument = one_argument(argument, command);
	argument = one_argument(argument, arg1);

	EDIT_MOB(ch, pMob);

	if (command[0] == '\0')
	{
		send_to_char("Syntax:  shop hours [#xopening] [#xclosing]\n\r", ch);
		send_to_char("         shop profit [#xbuying%] [#xselling%]\n\r", ch);
		send_to_char("         shop type [#x0-4] [item type]\n\r", ch);
		send_to_char("         shop assign\n\r", ch);
		send_to_char("         shop remove\n\r", ch);
		return FALSE;
	}


	if (!str_cmp(command, "hours"))
	{
		if (arg1[0] == '\0' || !is_number(arg1)
		|| argument[0] == '\0' || !is_number(argument))
		{
			send_to_char("Syntax:  shop hours [#xopening] [#xclosing]\n\r", ch);
			return FALSE;
		}

		if (!pMob->pShop)
		{
			send_to_char("MEdit:  Debes crear un shop primero (shop assign).\n\r", ch);
			return FALSE;
		}

		pMob->pShop->open_hour = atoi(arg1);
		pMob->pShop->close_hour = atoi(argument);

		send_to_char("Shop hours set.\n\r", ch);
		return TRUE;
	}


	if (!str_cmp(command, "profit"))
	{
		if (arg1[0] == '\0' || !is_number(arg1)
		|| argument[0] == '\0' || !is_number(argument))
		{
			send_to_char("Syntax:  shop profit [#xbuying%] [#xselling%]\n\r", ch);
			return FALSE;
		}

		if (!pMob->pShop)
		{
			send_to_char("MEdit:  Debes crear un shop primero (shop assign).\n\r", ch);
			return FALSE;
		}

		pMob->pShop->profit_buy     = atoi(arg1);
		pMob->pShop->profit_sell    = atoi(argument);

		send_to_char("Shop profit set.\n\r", ch);
		return TRUE;
	}


	if (!str_cmp(command, "type"))
	{
		int value;

		if (arg1[0] == '\0' || !is_number(arg1)
		|| argument[0] == '\0')
		{
			send_to_char("Syntax:  shop type [#x0-4] [item type]\n\r", ch);
			return FALSE;
		}

		if (atoi(arg1) >= MAX_TRADE)
		{
			char_printf(ch, "MEdit:  May sell %d items max.\n\r", MAX_TRADE);
			return FALSE;
		}

		if (!pMob->pShop)
		{
			send_to_char("MEdit:  Debes crear un shop primero (shop assign).\n\r", ch);
			return FALSE;
		}

		if ((value = flag_value(type_flags, argument)) == NO_FLAG)
		{
			send_to_char("MEdit:  That type of item is not known.\n\r", ch);
			return FALSE;
		}

		pMob->pShop->buy_type[atoi(arg1)] = value;

		send_to_char("Shop type set.\n\r", ch);
		return TRUE;
	}

	/* shop assign && shop delete by Phoenix */

	if (!str_prefix(command, "assign"))
	{
		if (pMob->pShop)
		{
		 	send_to_char("Mob already has a shop assigned to it.\n\r", ch);
		 	return FALSE;
		}

		pMob->pShop		= new_shop();
		if (!shop_first)
		 	shop_first	= pMob->pShop;
		if (shop_last)
			shop_last->next	= pMob->pShop;
		shop_last		= pMob->pShop;

		pMob->pShop->keeper	= pMob->vnum;

		send_to_char("New shop assigned to mobile.\n\r", ch);
		return TRUE;
	}

	if (!str_prefix(command, "remove"))
	{
		SHOP_DATA *pShop;

		pShop		= pMob->pShop;
		pMob->pShop	= NULL;

		if (pShop == shop_first)
		{
			if (!pShop->next)
			{
			shop_first = NULL;
			shop_last = NULL;
			}
			else
			shop_first = pShop->next;
		}
		else
		{
			SHOP_DATA *ipShop;

			for (ipShop = shop_first; ipShop; ipShop = ipShop->next)
			{
			if (ipShop->next == pShop)
			{
				if (!pShop->next)
				{
					shop_last = ipShop;
					shop_last->next = NULL;
				}
				else
					ipShop->next = pShop->next;
			}
			}
		}

		free_shop(pShop);

		send_to_char("Mobile is no longer a shopkeeper.\n\r", ch);
		return TRUE;
	}

	medit_shop(ch, "");
	return FALSE;
}


/* ROM medit functions: */

/* Moved out of medit() due to naming conflicts -- Hugin */
MEDIT(medit_sex)
{
	MOB_INDEX_DATA *pMob;
	int value;

	if (argument[0] != '\0') {
		EDIT_MOB(ch, pMob);

		if ((value = flag_value(sex_flags, argument)) != NO_FLAG) {
			pMob->sex = value;
			send_to_char("Sex set.\n\r", ch);
			return TRUE;
		}
	}

	send_to_char("Syntax: sex [sex]\n\r"
			  "Type '? sex' for a list of flags.\n\r", ch);
	return FALSE;
}


/* Moved out of medit() due to naming conflicts -- Hugin */
MEDIT(medit_act)
{
	MOB_INDEX_DATA *pMob;
	int value;

	if (argument[0] != '\0') {
		EDIT_MOB(ch, pMob);

		if ((value = flag_value(act_flags, argument)) != NO_FLAG) {
			pMob->act ^= value;
			SET_BIT(pMob->act, ACT_NPC);

			send_to_char("Act flag toggled.\n\r", ch);
			return TRUE;
		}
	}

	send_to_char("Syntax: act [flag]\n\r"
			  "Type '? act' for a list of flags.\n\r", ch);
	return FALSE;
}


MEDIT(medit_affect)      /* Moved out of medit() due to naming conflicts -- Hugin */
{
	MOB_INDEX_DATA *pMob;
	int value;

	if (argument[0] != '\0')
	{
		EDIT_MOB(ch, pMob);

		if ((value = flag_value(affect_flags, argument)) != NO_FLAG)
		{
			pMob->affected_by ^= value;

			send_to_char("Affect flag toggled.\n\r", ch);
			return TRUE;
		}
	}

	send_to_char("Syntax: affect [flag]\n\r"
			  "Type '? affect' for a list of flags.\n\r", ch);
	return FALSE;
}


MEDIT(medit_detect) 
{
	MOB_INDEX_DATA *pMob;
	int value;

	if (argument[0] != '\0') {
		EDIT_MOB(ch, pMob);

		if ((value = flag_value(detect_flags, argument)) != NO_FLAG) {
			pMob->detection ^= value;
			send_to_char("Detection flag toggled.\n\r", ch);
			return TRUE;
		}
	}

	char_puts("Syntax: detect [flag]\n\r"
		  "Type '? detect' for a list of flags.\n\r", ch);
	return FALSE;
}


MEDIT(medit_prac) 
{
	MOB_INDEX_DATA *pMob;
	int value;

	if (argument[0] != '\0') {
		EDIT_MOB(ch, pMob);

		if ((value = flag_value(skill_groups, argument)) != NO_FLAG) {
			pMob->practicer ^= value;
			send_to_char("Skill group practicer flag toggled.\n\r",
				ch);
			return TRUE;
		}
	}

	char_puts("Syntax: prac [flag]\n\r"
		  "Type '? prac' for a list of flags.\n\r", ch);
	return FALSE;
}


MEDIT(medit_ac)
{
	MOB_INDEX_DATA *pMob;
	char arg[MAX_INPUT_LENGTH];
	int pierce, bash, slash, exotic;

	do   /* So that I can use break and send the syntax in one place */
	{
		if (argument[0] == '\0')  break;

		EDIT_MOB(ch, pMob);
		argument = one_argument(argument, arg);

		if (!is_number(arg))  break;
		pierce = atoi(arg);
		argument = one_argument(argument, arg);

		if (arg[0] != '\0')
		{
			if (!is_number(arg))  break;
			bash = atoi(arg);
			argument = one_argument(argument, arg);
		}
		else
			bash = pMob->ac[AC_BASH];

		if (arg[0] != '\0')
		{
			if (!is_number(arg))  break;
			slash = atoi(arg);
			argument = one_argument(argument, arg);
		}
		else
			slash = pMob->ac[AC_SLASH];

		if (arg[0] != '\0')
		{
			if (!is_number(arg))  break;
			exotic = atoi(arg);
		}
		else
			exotic = pMob->ac[AC_EXOTIC];

		pMob->ac[AC_PIERCE] = pierce;
		pMob->ac[AC_BASH]   = bash;
		pMob->ac[AC_SLASH]  = slash;
		pMob->ac[AC_EXOTIC] = exotic;
		
		send_to_char("Ac set.\n\r", ch);
		return TRUE;
	} while (FALSE);    /* Just do it once.. */

	send_to_char("Syntax:  ac [ac-pierce [ac-bash [ac-slash [ac-exotic]]]]\n\r"
			  "help MOB_AC  gives a list of reasonable ac-values.\n\r", ch);
	return FALSE;
}

MEDIT(medit_form)
{
	MOB_INDEX_DATA *pMob;
	int value;

	if (argument[0] != '\0')
	{
		EDIT_MOB(ch, pMob);

		if ((value = flag_value(form_flags, argument)) != NO_FLAG)
		{
			pMob->form ^= value;
			send_to_char("Form toggled.\n\r", ch);
			return TRUE;
		}
	}

	send_to_char("Syntax: form [flags]\n\r"
			  "Type '? form' for a list of flags.\n\r", ch);
	return FALSE;
}

MEDIT(medit_part)
{
	MOB_INDEX_DATA *pMob;
	int value;

	if (argument[0] != '\0')
	{
		EDIT_MOB(ch, pMob);

		if ((value = flag_value(part_flags, argument)) != NO_FLAG)
		{
			pMob->parts ^= value;
			send_to_char("Parts toggled.\n\r", ch);
			return TRUE;
		}
	}

	send_to_char("Syntax: part [flags]\n\r"
			  "Type '? part' for a list of flags.\n\r", ch);
	return FALSE;
}

MEDIT(medit_imm)
{
	MOB_INDEX_DATA *pMob;
	int value;

	if (argument[0] != '\0')
	{
		EDIT_MOB(ch, pMob);

		if ((value = flag_value(imm_flags, argument)) != NO_FLAG)
		{
			pMob->imm_flags ^= value;
			send_to_char("Immunity toggled.\n\r", ch);
			return TRUE;
		}
	}

	send_to_char("Syntax: imm [flags]\n\r"
			  "Type '? imm' for a list of flags.\n\r", ch);
	return FALSE;
}

MEDIT(medit_res)
{
	MOB_INDEX_DATA *pMob;
	int value;

	if (argument[0] != '\0')
	{
		EDIT_MOB(ch, pMob);

		if ((value = flag_value(res_flags, argument)) != NO_FLAG)
		{
			pMob->res_flags ^= value;
			send_to_char("Resistance toggled.\n\r", ch);
			return TRUE;
		}
	}

	send_to_char("Syntax: res [flags]\n\r"
			  "Type '? res' for a list of flags.\n\r", ch);
	return FALSE;
}

MEDIT(medit_vuln)
{
	MOB_INDEX_DATA *pMob;
	int value;

	if (argument[0] != '\0')
	{
		EDIT_MOB(ch, pMob);

		if ((value = flag_value(vuln_flags, argument)) != NO_FLAG)
		{
			pMob->vuln_flags ^= value;
			send_to_char("Vulnerability toggled.\n\r", ch);
			return TRUE;
		}
	}

	send_to_char("Syntax: vuln [flags]\n\r"
			  "Type '? vuln' for a list of flags.\n\r", ch);
	return FALSE;
}

MEDIT(medit_material)
{
	MOB_INDEX_DATA *pMob;

	EDIT_MOB(ch, pMob);

	if (argument[0] == '\0')
	{
		send_to_char("Syntax:  material [string]\n\r", ch);
		return FALSE;
	}

	free_string(pMob->material);
	pMob->material = str_dup(argument);

	send_to_char("Material set.\n\r", ch);
	return TRUE;
}

MEDIT(medit_off)
{
	MOB_INDEX_DATA *pMob;
	int value;

	if (argument[0] != '\0')
	{
		EDIT_MOB(ch, pMob);

		if ((value = flag_value(off_flags, argument)) != NO_FLAG)
		{
			pMob->off_flags ^= value;
			send_to_char("Offensive behaviour toggled.\n\r", ch);
			return TRUE;
		}
	}

	send_to_char("Syntax: off [flags]\n\r"
			  "Type '? off' for a list of flags.\n\r", ch);
	return FALSE;
}

MEDIT(medit_size)
{
	MOB_INDEX_DATA *pMob;
	int value;

	if (argument[0] != '\0')
	{
		EDIT_MOB(ch, pMob);

		if ((value = flag_value(size_flags, argument)) != NO_FLAG)
		{
			pMob->size = value;
			send_to_char("Size set.\n\r", ch);
			return TRUE;
		}
	}

	send_to_char("Syntax: size [size]\n\r"
			  "Type '? size' for a list of sizes.\n\r", ch);
	return FALSE;
}

static bool medit_dice(CHAR_DATA *ch, const char *argument,
		       const char *name, int *dice)
{
	static char syntax[] = "Syntax: %s <number>d<type>+<bonus>\n\r";
	int num, type, bonus;
	char* p;

	if (argument[0] == '\0')
		goto bail_out;
	
	num = strtod(argument, &p);
	if (num < 1 || *p != 'd')
		goto bail_out;

	type = strtod(p+1, &p);
	if (type < 1 || *p != '+')
		goto bail_out;
	
	bonus = strtod(p+1, &p);
	if (bonus < 0 || *p != '\0')
		goto bail_out;

	dice[DICE_NUMBER] = num;
	dice[DICE_TYPE]   = type;
	dice[DICE_BONUS]  = bonus;

	char_printf(ch, "%s set to %dd%d+%d.\n\r", name, num, type, bonus);
	return TRUE;

bail_out:
	char_printf(ch, syntax, name);
	return FALSE;
}

MEDIT(medit_hitdice)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return medit_dice(ch, argument, "hitdice", pMob->hit);
}

MEDIT(medit_manadice)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return medit_dice(ch, argument, "manadice", pMob->mana);
}

MEDIT(medit_damdice)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return medit_dice(ch, argument, "damdice", pMob->damage);
}


MEDIT(medit_race)
{
	MOB_INDEX_DATA *pMob;
	int race;

	if (argument[0] != '\0'
	&& (race = race_lookup(argument)) != 0)
	{
		EDIT_MOB(ch, pMob);

		pMob->race = race;
		pMob->act	  |= race_table[race].act;
		pMob->affected_by |= race_table[race].aff;
		pMob->off_flags   |= race_table[race].off;
		pMob->imm_flags   |= race_table[race].imm;
		pMob->res_flags   |= race_table[race].res;
		pMob->vuln_flags  |= race_table[race].vuln;
		pMob->form        |= race_table[race].form;
		pMob->parts       |= race_table[race].parts;

		send_to_char("Race set.\n\r", ch);
		return TRUE;
	}

	if (argument[0] == '?') {
		send_to_char("Available races are:", ch);

		for (race = 0; race_table[race].name != NULL; race++)
		{
			if ((race % 3) == 0)
			send_to_char("\n\r", ch);
			char_printf(ch, " %-15s", race_table[race].name);
		}

		send_to_char("\n\r", ch);
		return FALSE;
	}

	send_to_char("Syntax:  race [race]\n\r"
			  "Type 'race ?' for a list of races.\n\r", ch);
	return FALSE;
}


MEDIT(medit_position)
{
	MOB_INDEX_DATA *pMob;
	char arg[MAX_INPUT_LENGTH];
	int value;

	argument = one_argument(argument, arg);

	switch (arg[0])
	{
	default:
		break;

	case 'S':
	case 's':
		if (str_prefix(arg, "start"))
			break;

		if ((value = flag_value(position_flags, argument)) == NO_FLAG)
			break;

		EDIT_MOB(ch, pMob);

		pMob->start_pos = value;
		send_to_char("Start position set.\n\r", ch);
		return TRUE;

	case 'D':
	case 'd':
		if (str_prefix(arg, "default"))
			break;

		if ((value = flag_value(position_flags, argument)) == NO_FLAG)
			break;

		EDIT_MOB(ch, pMob);

		pMob->default_pos = value;
		send_to_char("Default position set.\n\r", ch);
		return TRUE;
	}

	send_to_char("Syntax:  position [start/default] [position]\n\r"
			  "Type '? position' for a list of positions.\n\r", ch);
	return FALSE;
}


MEDIT(medit_gold)
{
	MOB_INDEX_DATA *pMob;

	EDIT_MOB(ch, pMob);

	if (argument[0] == '\0' || !is_number(argument))
	{
		send_to_char("Syntax:  wealth [number]\n\r", ch);
		return FALSE;
	}

	pMob->wealth = atoi(argument);

	send_to_char("Wealth set.\n\r", ch);
	return TRUE;
}

MEDIT(medit_hitroll)
{
	MOB_INDEX_DATA *pMob;

	EDIT_MOB(ch, pMob);

	if (argument[0] == '\0' || !is_number(argument))
	{
		send_to_char("Syntax:  hitroll [number]\n\r", ch);
		return FALSE;
	}

	pMob->hitroll = atoi(argument);

	send_to_char("Hitroll set.\n\r", ch);
	return TRUE;
}

void show_liqlist(CHAR_DATA *ch)
{
	int liq;
	BUFFER *buffer;
	
	buffer = buf_new(0);
	
	for (liq = 0; liq_table[liq].liq_name != NULL; liq++) {
		if ((liq % 21) == 0)
			buf_add(buffer,"Name                 Color          Proof Full Thirst Food Ssize\n\r");

		buf_printf(buffer, "%-20s %-14s %5d %4d %6d %4d %5d\n\r",
			liq_table[liq].liq_name,liq_table[liq].liq_color,
			liq_table[liq].liq_affect[0],liq_table[liq].liq_affect[1],
			liq_table[liq].liq_affect[2],liq_table[liq].liq_affect[3],
			liq_table[liq].liq_affect[4]);
	}

	page_to_char(buf_string(buffer), ch);
	buf_free(buffer);
}

void show_damlist(CHAR_DATA *ch)
{
	int att;
	BUFFER *buffer;
	
	buffer = buf_new(0);
	
	for (att = 0; attack_table[att].name != NULL; att++) {
		if ((att % 21) == 0)
			buf_add(buffer,"Name                 Noun\n\r");

		buf_printf(buffer, "%-20s %-20s\n\r",
			attack_table[att].name,attack_table[att].noun);
	}

	page_to_char(buf_string(buffer),ch);
	buf_free(buffer);
}

MEDIT(medit_group)
{
	MOB_INDEX_DATA *pMob;
	MOB_INDEX_DATA *pMTemp;
	char arg[MAX_STRING_LENGTH];
	int temp;
	BUFFER *buffer;
	bool found = FALSE;
	
	EDIT_MOB(ch, pMob);
	
	if (argument[0] == '\0') {
		send_to_char("Syntax: group [number]\n\r", ch);
		send_to_char("        group show [number]\n\r", ch);
		return FALSE;
	}
	
	if (is_number(argument))
	{
		pMob->group = atoi(argument);
		send_to_char("Group set.\n\r", ch);
		return TRUE;
	}
	
	argument = one_argument(argument, arg);
	
	if (!strcmp(arg, "show") && is_number(argument)) {
		if (atoi(argument) == 0) {
			send_to_char("Are you crazy?\n\r", ch);
			return FALSE;
		}

		buffer = buf_new(0);

		for (temp = 0; temp < 65536; temp++) {
			pMTemp = get_mob_index(temp);
			if (pMTemp && (pMTemp->group == atoi(argument))) {
				found = TRUE;
				buf_printf(buffer, "[%5d] %s\n\r",
					   pMTemp->vnum, pMTemp->player_name);
			}
		}

		if (found)
			page_to_char(buf_string(buffer), ch);
		else
			send_to_char("No mobs in that group.\n\r", ch);

		buf_free(buffer);
		return FALSE;
	}
	
	return FALSE;
}

REDIT(redit_owner)
{
	ROOM_INDEX_DATA *pRoom;

	EDIT_ROOM(ch, pRoom);

	if (argument[0] == '\0') {
		send_to_char("Syntax:  owner [owner]\n\r", ch);
		send_to_char("         owner none\n\r", ch);
		return FALSE;
	}

	free_string(pRoom->owner);
	if (!str_cmp(argument, "none"))
		pRoom->owner = str_dup("");
	else
		pRoom->owner = str_dup(argument);

	send_to_char("Owner set.\n\r", ch);
	return TRUE;
}

MEDIT (medit_mpadd)
{
	int value;
	MOB_INDEX_DATA *pMob;
	MPROG_LIST *list;
	MPROG_CODE *code;
	char trigger[MAX_STRING_LENGTH];
	char phrase[MAX_STRING_LENGTH];
	char num[MAX_STRING_LENGTH];

	EDIT_MOB(ch, pMob);
	argument=one_argument(argument, num);
	argument=one_argument(argument, trigger);
	argument=one_argument(argument, phrase);

	if (!is_number(num) || trigger[0] =='\0' || phrase[0] =='\0') {
		 char_puts("Syntax:   mpadd [vnum] [trigger] [phrase]\n\r",ch);
		 return FALSE;
	}

	if ((value = flag_value (mprog_flags, trigger)) == NO_FLAG) {
		 send_to_char("Valid flags are:\n\r",ch);
		 show_help(ch, "mprog");
		 return FALSE;
	}

	if ((code =get_mprog_index (atoi(num))) == NULL) {
		 send_to_char("No such MOBProgram.\n\r",ch);
		 return FALSE;
	}

	list                  = new_mprog();
	list->vnum            = atoi(num);
	list->trig_type       = value;
	list->trig_phrase     = str_dup(phrase);
	list->code            = code->code;
	SET_BIT(pMob->mprog_flags,value);
	list->next            = pMob->mprogs;
	pMob->mprogs          = list;

	send_to_char("Mprog Added.\n\r",ch);
	return TRUE;
}

MEDIT (medit_mpdel)
{
	MOB_INDEX_DATA *pMob;
	MPROG_LIST *list;
	MPROG_LIST *list_next;
	char mprog[MAX_STRING_LENGTH];
	int value;
	int cnt = 0;

	EDIT_MOB(ch, pMob);

	one_argument(argument, mprog);
	if (!is_number(mprog) || mprog[0] == '\0') {
		send_to_char("Syntax:  mpdel [#mprog]\n\r",ch);
		return FALSE;
	}

	value = atoi (mprog);

	if (value < 0) {
		 char_puts("Only non-negative mprog-numbers allowed.\n\r",ch);
		 return FALSE;
	}

	if (!(list = pMob->mprogs)) {
		 send_to_char("MEdit:  Nonexistent mprog.\n\r",ch);
		 return FALSE;
	}

	if (value == 0) {
		REMOVE_BIT(pMob->mprog_flags, pMob->mprogs->trig_type);
		list = pMob->mprogs;
		pMob->mprogs = list->next;
		free_mprog(list);
	}
	else {
		while ((list_next = list->next) && (++cnt < value))
			list = list_next;

		if (list_next) {
			REMOVE_BIT(pMob->mprog_flags, list_next->trig_type);
		        list->next = list_next->next;
		        free_mprog(list_next);
		}
		else {
		        send_to_char("No such mprog.\n\r",ch);
		        return FALSE;
		}
	}

	send_to_char("Mprog removed.\n\r", ch);
	return TRUE;
}

REDIT(redit_room)
{
	ROOM_INDEX_DATA *room;
	int value;

	EDIT_ROOM(ch, room);

	if ((value = flag_value(room_flags, argument)) == NO_FLAG) {
		send_to_char("Syntax: room [flags]\n\r", ch);
		return FALSE;
	}

	TOGGLE_BIT(room->room_flags, value);
	send_to_char("Room flags toggled.\n\r", ch);
	return TRUE;
}

REDIT(redit_sector)
{
	ROOM_INDEX_DATA *room;
	int value;

	EDIT_ROOM(ch, room);

	if ((value = flag_value(sector_flags, argument)) == NO_FLAG) {
		send_to_char("Syntax: sector [tipo]\n\r", ch);
		return FALSE;
	}

	room->sector_type = value;
	send_to_char("Sector type set.\n\r", ch);

	return TRUE;
}
