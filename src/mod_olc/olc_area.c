/*-
 * Copyright (c) 1998 fjoe <fjoe@iclub.nsu.ru>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id: olc_area.c,v 1.15 1998-10-14 11:05:24 fjoe Exp $
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "olc.h"
#include "interp.h"
#include "db/db.h"

#define EDIT_AREA(ch, area)	(area = (AREA_DATA*) ch->desc->pEdit)

DECLARE_OLC_FUN(areaed_create		);
DECLARE_OLC_FUN(areaed_edit		);
DECLARE_OLC_FUN(areaed_touch		);
DECLARE_OLC_FUN(areaed_show		);
DECLARE_OLC_FUN(areaed_list		);

DECLARE_OLC_FUN(areaed_name		);
DECLARE_OLC_FUN(areaed_file		);
DECLARE_OLC_FUN(areaed_flags		);
DECLARE_OLC_FUN(areaed_age		);
DECLARE_OLC_FUN(areaed_reset		);
DECLARE_OLC_FUN(areaed_security		);
DECLARE_OLC_FUN(areaed_builder		);
DECLARE_OLC_FUN(areaed_minvnum		);
DECLARE_OLC_FUN(areaed_maxvnum		);
DECLARE_OLC_FUN(areaed_credits		);
DECLARE_OLC_FUN(areaed_minlevel		);
DECLARE_OLC_FUN(areaed_maxlevel		);

DECLARE_VALIDATE_FUN(validate_security	);
DECLARE_VALIDATE_FUN(validate_minvnum	);
DECLARE_VALIDATE_FUN(validate_maxvnum	);

OLC_CMD_DATA olc_cmds_area[] =
{
/*	{   command	function	arg			}, */

	{ "create",	areaed_create				},
	{ "edit",	areaed_edit				},
	{ "touch",	areaed_touch				},
	{ "show",	areaed_show				},
	{ "list",	areaed_list				},

	{ "age",	areaed_age				},
	{ "area",	areaed_flags,	area_flags		},
	{ "builder",	areaed_builder				},
	{ "filename",	areaed_file,	validate_filename	},
	{ "name",	areaed_name				},
	{ "reset",	areaed_reset				},
	{ "security",	areaed_security, validate_security	},
	{ "minlevel",	areaed_minlevel				},
	{ "maxlevel",	areaed_maxlevel				},
	{ "minvnum",	areaed_minvnum,	validate_minvnum	},
	{ "maxvnum",	areaed_maxvnum,	validate_maxvnum	},
	{ "credits",	areaed_credits				},

	{ "commands",	show_commands				},
	{ "version",	show_version				},

	{ NULL }
};

static AREA_DATA *check_range(AREA_DATA *pArea, int ilower, int iupper);

/*
 * Area Editor Functions.
 */
OLC_FUN(areaed_create)
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

OLC_FUN(areaed_edit)
{
	AREA_DATA *pArea;
	char arg[MAX_STRING_LENGTH];

	argument = one_argument(argument, arg);
	if (arg[0] == '\0')
		pArea = ch->in_room->area;
	else if (!is_number(arg) || (pArea = area_lookup(atoi(arg))) == NULL) {
		char_puts("AEdit: That area vnum does not exist.\n\r", ch);
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

OLC_FUN(areaed_touch)
{
	AREA_DATA *pArea;
	EDIT_AREA(ch, pArea);
	return touch_area(pArea);
}

OLC_FUN(areaed_show)
{
	AREA_DATA *pArea;
	EDIT_AREA(ch, pArea);
	char_printf(ch, "Name:     [%5d] %s\n\r", pArea->vnum, pArea->name);
	char_printf(ch, "File:     %s\n\r", pArea->file_name);
	char_printf(ch, "Vnums:    [%d-%d]\n\r",
		    pArea->min_vnum, pArea->max_vnum);
	char_printf(ch, "Levels:   [%d-%d]\n\r",
		    pArea->min_level, pArea->max_level);
	char_printf(ch, "Age:      [%d]\n\r",	pArea->age);
	char_printf(ch, "Players:  [%d]\n\r", pArea->nplayer);
	char_printf(ch, "Security: [%d]\n\r", pArea->security);
	if (!IS_NULLSTR(pArea->builders))
		char_printf(ch, "Builders: [%s]\n\r", pArea->builders);
	if (!IS_NULLSTR(pArea->credits))
		char_printf(ch, "Credits:  [%s]\n\r", pArea->credits);
	char_printf(ch, "Flags:    [%s]\n\r",
			flag_string(area_flags, pArea->flags));
	return FALSE;
}

OLC_FUN(areaed_list)
{
	char arg[MAX_STRING_LENGTH];
	AREA_DATA *pArea;
	BUFFER *output = NULL;

	one_argument(argument, arg);

	for (pArea = area_first; pArea; pArea = pArea->next) {
		if (arg[0] != '\0' && !strstr(strlwr(pArea->name), arg))
			continue;

		if (output == NULL) {
			output = buf_new(0);
    			buf_printf(output, "[%3s] [%-27s] (%-5s-%5s) [%-10s] %3s [%-10s]\n\r",
				   "Num", "Area Name", "lvnum", "uvnum",
				   "Filename", "Sec", "Builders");
		}

		buf_printf(output, "[%3d] %-29.29s (%-5d-%5d) %-12.12s [%d] [%-10.10s]\n\r",
			   pArea->vnum, pArea->name,
			   pArea->min_vnum, pArea->max_vnum,
			   pArea->file_name, pArea->security, pArea->builders);
    	}

	if (output != NULL) {
		send_to_char(buf_string(output), ch);
		buf_free(output);
	}
	else
		char_puts("No areas with that name found.\n\r", ch);
	return FALSE;
}

OLC_FUN(areaed_reset)
{
	AREA_DATA *pArea;
	EDIT_AREA(ch, pArea);
	reset_area(pArea);
	char_puts("Area reset.\n\r", ch);
	return FALSE;
}

OLC_FUN(areaed_name)
{
	AREA_DATA *pArea;
	EDIT_AREA(ch, pArea);
	return olced_str(ch, argument, areaed_name, &pArea->name);
}

OLC_FUN(areaed_credits)
{
	AREA_DATA *pArea;
	EDIT_AREA(ch, pArea);
	return olced_str(ch, argument, areaed_credits, &pArea->credits);
}

OLC_FUN(areaed_file)
{
	AREA_DATA *pArea;
	EDIT_AREA(ch, pArea);
	return olced_str(ch, argument, areaed_file, &pArea->file_name);
}

OLC_FUN(areaed_age)
{
	AREA_DATA *pArea;
	EDIT_AREA(ch, pArea);
	return olced_number(ch, argument, areaed_age, &pArea->age);
}

OLC_FUN(areaed_flags)
{
	AREA_DATA *pArea;
	EDIT_AREA(ch, pArea);
	return olced_flag(ch, argument, areaed_flags, &pArea->flags);
}

OLC_FUN(areaed_security)
{
	AREA_DATA *pArea;
	EDIT_AREA(ch, pArea);
	return olced_number(ch, argument, areaed_security, &pArea->security);
}

OLC_FUN(areaed_minlevel)
{
	AREA_DATA *pArea;
	EDIT_AREA(ch, pArea);
	return olced_number(ch, argument, areaed_minlevel, &pArea->min_level);
}

OLC_FUN(areaed_maxlevel)
{
	AREA_DATA *pArea;
	EDIT_AREA(ch, pArea);
	return olced_number(ch, argument, areaed_maxlevel, &pArea->max_level);
}

OLC_FUN(areaed_builder)
{
	AREA_DATA *pArea;
	char name[MAX_STRING_LENGTH];

	EDIT_AREA(ch, pArea);

	one_argument(argument, name);
	if (name[0] == '\0') {
		do_help(ch, "'OLC AREA BUILDER'");
		return FALSE;
	}
	name_toggle(ch, name, "AreaEd", &pArea->builders);
	return TRUE;
}

OLC_FUN(areaed_minvnum)
{
	AREA_DATA *pArea;
	EDIT_AREA(ch, pArea);
	return olced_number(ch, argument, areaed_minvnum, &pArea->min_vnum);
}

OLC_FUN(areaed_maxvnum)
{
	AREA_DATA *pArea;
	EDIT_AREA(ch, pArea);
	return olced_number(ch, argument, areaed_maxvnum, &pArea->max_vnum);
}

/* Validators */

VALIDATE_FUN(validate_security)
{
	int sec = *(int*) arg;
	if (sec > ch->pcdata->security || sec < 0) {
		if (ch->pcdata->security != 0)
			char_printf(ch, "AEdit: Valid security range is 0..%d.\n\r", ch->pcdata->security);
		else
			char_puts("AEdit: Valid security is 0 only.\n\r", ch);
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
			char_puts("AEdit: Min vnum must be less than max vnum.\n\r", ch);
			return FALSE;
		}
	
		if (check_range(pArea, min_vnum, pArea->max_vnum)) {
			char_puts("AEdit: Range must include only this area.\n\r", ch);
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
			char_puts("AEdit: Max vnum must be greater than min vnum.\n\r", ch);
			return FALSE;
		}
	
		if (check_range(pArea, pArea->min_vnum, max_vnum)) {
			char_puts("AEdit: Range must include only this area.\n\r", ch);
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
 Called by:	areaed_vnum(olc_act.c).
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
