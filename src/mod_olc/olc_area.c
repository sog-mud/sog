#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "olc.h"
#include "comm.h"
#include "string_edit.h"
#include "db.h"
#include "tables.h"

#define AEDIT(fun)		bool fun(CHAR_DATA *ch, const char *argument)
#define EDIT_AREA(Ch, Area)	(Area = (AREA_DATA *)Ch->desc->pEdit)

DECLARE_OLC_FUN(aedit_show		);
DECLARE_OLC_FUN(aedit_create		);
DECLARE_OLC_FUN(aedit_name		);
DECLARE_OLC_FUN(aedit_file		);
DECLARE_OLC_FUN(aedit_age		);
DECLARE_OLC_FUN(aedit_reset		);
DECLARE_OLC_FUN(aedit_security		);
DECLARE_OLC_FUN(aedit_builder		);
DECLARE_OLC_FUN(aedit_vnum		);
DECLARE_OLC_FUN(aedit_lvnum		);
DECLARE_OLC_FUN(aedit_uvnum		);
DECLARE_OLC_FUN(aedit_credits		);

OLC_CMD_DATA aedit_table[] =
{
/*  {   command		function	}, */

    {   "age",		aedit_age	},
    {   "builder",	aedit_builder	}, /* s removed -- Hugin */
    {   "create",	aedit_create	},
    {   "filename",	aedit_file	},
    {   "name",		aedit_name	},
/*  {   "recall",	aedit_recall	},   ROM OLC */
    {	"reset",	aedit_reset	},
    {   "security",	aedit_security	},
    {	"show",		aedit_show	},
    {   "vnum",		aedit_vnum	},
    {   "lvnum",	aedit_lvnum	},
    {   "uvnum",	aedit_uvnum	},
    {   "credits",	aedit_credits	},

    {   "?",		show_help	},
    {   "commands",	show_commands	},
    {   "version",	show_version	},

    {	NULL,		0,		}
};

AREA_DATA *area_in_range = NULL;

/* Entry point for editing area_data. */
void do_aedit(CHAR_DATA *ch, const char *argument)
{
    AREA_DATA *pArea;
    int value;
    char arg[MAX_STRING_LENGTH];

    if (IS_NPC(ch))
    	return;

    pArea	= ch->in_room->area;

    argument	= one_argument(argument,arg);

    if (is_number(arg)) {
	value = atoi(arg);
	if (!(pArea = get_area_data(value))) {
	    send_to_char("That area vnum does not exist.\n\r", ch);
	    return;
	}
    } else if (!str_cmp(arg, "create")) {
	if (ch->pcdata->security < 9) {
		send_to_char("AEdit: Insufficient security for creating areas.\n\r", ch);
		return;
	}

	aedit_create(ch, "");
	ch->desc->editor = ED_AREA;
	return;
    }

    if (!IS_BUILDER(ch,pArea)) {
	send_to_char("Insufficient security for editing areas.\n\r",ch);
	return;
    }

    ch->desc->pEdit = (void *)pArea;
    ch->desc->editor = ED_AREA;
    return;
}

/* Area Interpreter, called by do_aedit. */
void aedit(CHAR_DATA *ch, const char *argument)
{
    AREA_DATA *pArea;
    char command[MAX_INPUT_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int  cmd;
    int  value;

    EDIT_AREA(ch, pArea);
    strcpy(arg, argument);
    smash_tilde(arg);
    argument = one_argument(arg, command);

    if (!IS_BUILDER(ch, pArea))
    {
	send_to_char("AEdit:  Insufficient security to modify area.\n\r", ch);
	edit_done(ch);
	return;
    }

    if (!str_cmp(command, "done"))
    {
	edit_done(ch);
	return;
    }

    if (command[0] == '\0')
    {
	aedit_show(ch, argument);
	return;
    }

    if ((value = flag_value(area_flags, command))) {
	TOGGLE_BIT(pArea->flags, value);
	send_to_char("Flag toggled.\n\r", ch);
	return;
    }

    /* Search Table and Dispatch Command. */
    for (cmd = 0; aedit_table[cmd].name != NULL; cmd++)
    {
	if (!str_prefix(command, aedit_table[cmd].name))
	{
	    if ((*aedit_table[cmd].olc_fun) (ch, argument))
	    {
		SET_BIT(pArea->flags, AREA_CHANGED);
		return;
	    }
	    else
		return;
	}
    }

    /* Default to Standard Interpreter. */
    interpret(ch, arg);
    return;
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

		area_in_range = pArea;
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
	char_printf(ch, "File:     %s\n\r", pArea->file_name);
	char_printf(ch, "Vnums:    [%d-%d]\n\r", pArea->min_vnum, pArea->max_vnum);
	char_printf(ch, "Age:      [%d]\n\r",	pArea->age);
	char_printf(ch, "Players:  [%d]\n\r", pArea->nplayer);
	char_printf(ch, "Security: [%d]\n\r", pArea->security);
	char_printf(ch, "Builders: [%s]\n\r", pArea->builders);
	char_printf(ch, "Credits : [%s]\n\r", pArea->credits);
	char_printf(ch, "Flags:    [%s]\n\r",
			flag_string(area_flags, pArea->flags));
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

	SET_BIT(pArea->flags, AREA_ADDED);
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
	return FALSE;
}

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
	||  !is_number(upper) || upper[0] == '\0') {
		char_puts("Syntax:  vnum [#xlower] [#xupper]\n\r", ch);
		return FALSE;
	}

	if ((ilower = atoi(lower)) > (iupper = atoi(upper))) {
		char_puts("Upper must be larger then lower.\n\r", ch);
		return FALSE;
	}
	
	if (!check_range(atoi(lower), atoi(upper))) {
		char_puts("Range must include only this area.\n\r", ch);
		char_printf(ch, "This range intersects with %s.\n\r",
			    area_in_range->name);
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

