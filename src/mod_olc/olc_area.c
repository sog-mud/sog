/*
 * $Id: olc_area.c,v 1.7 1998-09-15 02:52:14 fjoe Exp $
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "olc.h"

#define EDIT_AREA(ch, area)	(area = (AREA_DATA*) ch->desc->pEdit)

DECLARE_OLC_FUN(aedit_create		);
DECLARE_OLC_FUN(aedit_edit		);
DECLARE_OLC_FUN(aedit_touch		);
DECLARE_OLC_FUN(aedit_show		);

DECLARE_OLC_FUN(aedit_name		);
DECLARE_OLC_FUN(aedit_file		);
DECLARE_OLC_FUN(aedit_flags		);
DECLARE_OLC_FUN(aedit_age		);
DECLARE_OLC_FUN(aedit_reset		);
DECLARE_OLC_FUN(aedit_security		);
DECLARE_OLC_FUN(aedit_builder		);
DECLARE_OLC_FUN(aedit_minvnum		);
DECLARE_OLC_FUN(aedit_maxvnum		);
DECLARE_OLC_FUN(aedit_credits		);
DECLARE_OLC_FUN(aedit_minlevel		);
DECLARE_OLC_FUN(aedit_maxlevel		);

DECLARE_VALIDATE_FUN(validate_security	);
DECLARE_VALIDATE_FUN(validate_minvnum	);
DECLARE_VALIDATE_FUN(validate_maxvnum	);

OLC_CMD_DATA olc_cmds_area[] =
{
/*	{   command	function	arg			}, */

	{ "create",	aedit_create				},
	{ "edit",	aedit_edit				},
	{ "touch",	aedit_touch				},
	{ "show",	aedit_show				},

	{ "age",	aedit_age				},
	{ "area",	aedit_flags,	area_flags		},
	{ "builder",	aedit_builder				},
	{ "filename",	aedit_file,	validate_filename	},
	{ "name",	aedit_name				},
	{ "reset",	aedit_reset				},
	{ "security",	aedit_security, validate_security	},
	{ "minlevel",	aedit_minlevel				},
	{ "maxlevel",	aedit_maxlevel				},
	{ "minvnum",	aedit_minvnum,	validate_minvnum	},
	{ "maxvnum",	aedit_maxvnum,	validate_maxvnum	},
	{ "credits",	aedit_credits				},

	{ "commands",	show_commands				},
	{ "version",	show_version				},

	{ NULL }
};

static AREA_DATA *check_range(AREA_DATA *pArea, int ilower, int iupper);

/*
 * Area Editor Functions.
 */
OLC_FUN(aedit_create)
{
	AREA_DATA *pArea;

	if (ch->pcdata->security < SECURITY_AREA_CREATE) {
		char_puts("AEdit: Insufficient security.\n\r", ch);
		return FALSE;
	}

	pArea			= new_area();
	area_last->next		= pArea;
	area_last		= pArea;	/* Thanks, Walker. */

	ch->desc->pEdit		= (void*) pArea;
	ch->desc->editor	= ED_AREA;
	touch_area(pArea);
	char_puts("AEdit: Area created.\n\r", ch);
	return FALSE;
}

OLC_FUN(aedit_edit)
{
	AREA_DATA *pArea;
	char arg[MAX_STRING_LENGTH];

	argument = one_argument(argument, arg);
	if (arg[0] == '\0')
		pArea = ch->in_room->area;
	else if (!is_number(arg) || (pArea = area_lookup(atoi(arg))) == NULL) {
		send_to_char("AEdit: That area vnum does not exist.\n\r", ch);
		return FALSE;
	}

	if (!IS_BUILDER(ch, pArea)) {
		char_puts("AEdit: Insufficient security.\n\r", ch);
		return FALSE;
	}

	ch->desc->pEdit		= (void *) pArea;
	ch->desc->editor	= ED_AREA;
	return FALSE;
}

OLC_FUN(aedit_touch)
{
	AREA_DATA *pArea;
	EDIT_AREA(ch, pArea);
	return touch_area(pArea);
}

OLC_FUN(aedit_show)
{
	AREA_DATA *pArea;
	EDIT_AREA(ch, pArea);
	char_printf(ch, "Name:     [%5d] %s\n\r", pArea->vnum, pArea->name);
	char_printf(ch, "File:     %s\n\r", pArea->file_name);
	char_printf(ch, "Vnums:    [%d-%d]\n\r", pArea->min_vnum, pArea->max_vnum);
	char_printf(ch, "Levels:   [%d-%d]\n\r", pArea->min_level, pArea->max_level);
	char_printf(ch, "Age:      [%d]\n\r",	pArea->age);
	char_printf(ch, "Players:  [%d]\n\r", pArea->nplayer);
	char_printf(ch, "Security: [%d]\n\r", pArea->security);
	if (!IS_NULLSTR(pArea->builders))
		char_printf(ch, "Builders: [%s]\n\r", pArea->builders);
	if (!IS_NULLSTR(pArea->credits))
		char_printf(ch, "Credits : [%s]\n\r", pArea->credits);
	char_printf(ch, "Flags:    [%s]\n\r",
			flag_string(area_flags, pArea->flags));
	return FALSE;
}

OLC_FUN(aedit_reset)
{
	AREA_DATA *pArea;
	EDIT_AREA(ch, pArea);
	reset_area(pArea);
	send_to_char("Area reset.\n\r", ch);
	return FALSE;
}

OLC_FUN(aedit_name)
{
	AREA_DATA *pArea;
	EDIT_AREA(ch, pArea);
	return olced_str(ch, argument, aedit_name, &pArea->name);
}

OLC_FUN(aedit_credits)
{
	AREA_DATA *pArea;
	EDIT_AREA(ch, pArea);
	return olced_str(ch, argument, aedit_credits, &pArea->credits);
}

OLC_FUN(aedit_file)
{
	AREA_DATA *pArea;
	EDIT_AREA(ch, pArea);
	return olced_str(ch, argument, aedit_file, &pArea->file_name);
}

OLC_FUN(aedit_age)
{
	AREA_DATA *pArea;
	EDIT_AREA(ch, pArea);
	return olced_number(ch, argument, aedit_age, &pArea->age);
}

OLC_FUN(aedit_flags)
{
	AREA_DATA *pArea;
	EDIT_AREA(ch, pArea);
	return olced_flag(ch, argument, aedit_flags, &pArea->flags);
}

OLC_FUN(aedit_security)
{
	AREA_DATA *pArea;
	EDIT_AREA(ch, pArea);
	return olced_number(ch, argument, aedit_security, &pArea->security);
}

OLC_FUN(aedit_minlevel)
{
	AREA_DATA *pArea;
	EDIT_AREA(ch, pArea);
	return olced_number(ch, argument, aedit_minlevel, &pArea->min_level);
}

OLC_FUN(aedit_maxlevel)
{
	AREA_DATA *pArea;
	EDIT_AREA(ch, pArea);
	return olced_number(ch, argument, aedit_maxlevel, &pArea->max_level);
}

OLC_FUN(aedit_builder)
{
	AREA_DATA *pArea;
	char name[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH];

	EDIT_AREA(ch, pArea);

	one_argument(argument, name);

	if (name[0] == '\0') {
		send_to_char("Syntax:  builder name  -toggles builder\n\r", ch);
		send_to_char("Syntax:  builder All   -allows everyone\n\r", ch);
		return FALSE;
	}

	name[0] = UPPER(name[0]);

	if (strstr(pArea->builders, name) != '\0') {
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
	else {
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

OLC_FUN(aedit_minvnum)
{
	AREA_DATA *pArea;
	EDIT_AREA(ch, pArea);
	return olced_number(ch, argument, aedit_minvnum, &pArea->min_vnum);
}

OLC_FUN(aedit_maxvnum)
{
	AREA_DATA *pArea;
	EDIT_AREA(ch, pArea);
	return olced_number(ch, argument, aedit_maxvnum, &pArea->max_vnum);
}

/* Validators */

VALIDATE_FUN(validate_security)
{
	int sec = *(int*) arg;
	if (sec > ch->pcdata->security || sec < 0) {
		if (ch->pcdata->security != 0)
			char_printf(ch, "AEdit: Valid security range is 0..%d.\n\r", ch->pcdata->security);
		else
			send_to_char("AEdit: Valid security is 0 only.\n\r", ch);
		return FALSE;
	}
	return TRUE;
}

VALIDATE_FUN(validate_minvnum)
{
	int min_vnum = *(int*) arg;
	AREA_DATA *pArea;
	EDIT_AREA(ch, pArea);

	if (min_vnum && pArea->max_vnum) {
		if (min_vnum > pArea->max_vnum) {
			send_to_char("AEdit: Min vnum must be less than max vnum.\n\r", ch);
			return FALSE;
		}
	
		if (check_range(pArea, min_vnum, pArea->max_vnum)) {
			send_to_char("AEdit: Range must include only this area.\n\r", ch);
			return FALSE;
		}
	}
	return TRUE;
}

VALIDATE_FUN(validate_maxvnum)
{
	int max_vnum = *(int*) arg;
	AREA_DATA *pArea;
	EDIT_AREA(ch, pArea);

	if (pArea->min_vnum && max_vnum) {
		if (max_vnum < pArea->min_vnum) {
			send_to_char("AEdit: Max vnum must be greater than min vnum.\n\r", ch);
			return FALSE;
		}
	
		if (check_range(pArea, pArea->min_vnum, max_vnum)) {
			send_to_char("AEdit: Range must include only this area.\n\r", ch);
			return FALSE;
		}
	}
	return TRUE;
}
	
/* Local functions */

#define IN_RANGE(i, l, u) ((l) <= (i) && (i) <= (u))

/*****************************************************************************
 Name:		check_range(lower vnum, upper vnum)
 Purpose:	Ensures the range spans only one area.
 Called by:	aedit_vnum(olc_act.c).
 ****************************************************************************/
static AREA_DATA *check_range(AREA_DATA *this, int ilower, int iupper)
{
	AREA_DATA *pArea;

	for (pArea = area_first; pArea; pArea = pArea->next) {
		if (pArea == this || !pArea->min_vnum || !pArea->max_vnum)
			continue;
		if (IN_RANGE(ilower, pArea->min_vnum, pArea->max_vnum)
		||  IN_RANGE(iupper, pArea->min_vnum, pArea->max_vnum)
		||  IN_RANGE(pArea->min_vnum, ilower, iupper)
		||  IN_RANGE(pArea->max_vnum, ilower, iupper))
			return pArea;
	}
	return NULL;
}
