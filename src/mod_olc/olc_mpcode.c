/*
 * $Id: olc_mpcode.c,v 1.13 1998-09-19 10:39:10 fjoe Exp $
 */

/* The following code is based on ILAB OLC by Jason Dinkel */
/* Mobprogram code by Lordrom for Nevermore Mud */

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "interp.h"
#include "olc/olc.h"

#define EDIT_MPCODE(ch, mpcode)   (mpcode = (MPCODE*) ch->desc->pEdit)

/* Mobprog editor */
DECLARE_OLC_FUN(mpedit_create		);
DECLARE_OLC_FUN(mpedit_edit		);
DECLARE_OLC_FUN(mpedit_touch		);
DECLARE_OLC_FUN(mpedit_show		);

DECLARE_OLC_FUN(mpedit_code		);
DECLARE_OLC_FUN(mpedit_list		);

OLC_CMD_DATA olc_cmds_mpcode[] =
{
/*	{ command	function	}, */

	{ "create",	mpedit_create	},
	{ "edit",	mpedit_edit	},
	{ "touch",	mpedit_touch	},
	{ "show",	mpedit_show	},

	{ "code",	mpedit_code	},
	{ "list",	mpedit_list	},

	{ "commands",	show_commands	},

	{ NULL }
};

OLC_FUN(mpedit_create)
{
	MPCODE *mpcode;
	int value;
	AREA_DATA *pArea;
	char arg[MAX_STRING_LENGTH];

	one_argument(argument, arg);
	value = atoi(arg);
	if (!value) {
		do_help(ch, "'OLC CREATE'");
		return FALSE;
	}

	pArea = area_vnum_lookup(value);
	if (!pArea) {
		char_puts("MPEdit: That vnum is not assigned an area.\n\r", ch);
		return FALSE;
	}

	if (!IS_BUILDER(ch, pArea)) {
		char_puts("MPEdit: Insufficient security.\n\r", ch);
		return FALSE;
	}

	if (get_mob_index(value)) {
		char_puts("MPEdit: vnum already exists.\n\r", ch);
		return FALSE;
	}

	mpcode			= mpcode_new();
	mpcode->vnum		= value;
	mpcode_add(mpcode);

	ch->desc->pEdit		= (void *)mpcode;
	ch->desc->editor	= ED_MPCODE;
	touch_area(pArea);
	char_puts("MPEdit: mpcode created.\n\r", ch);
	return FALSE;
}

OLC_FUN(mpedit_edit)
{
	MPCODE *mpcode;
	AREA_DATA *pArea;
	int value;
	char arg[MAX_STRING_LENGTH];

	argument = one_argument(argument, arg);
	value = atoi(arg);
	mpcode = mpcode_lookup(value);

	if (!mpcode) {
		char_puts("MPEdit: Vnum does not exist.\n\r", ch);
		return FALSE;
	}

	pArea = area_vnum_lookup(mpcode->vnum);
	if (!IS_BUILDER(ch, pArea)) {
		char_puts("MEdit: Insufficient security.\n\r", ch);
	       	return FALSE;
	}

	ch->desc->pEdit = (void*) mpcode;
	ch->desc->editor = ED_MPCODE;
	return FALSE;
}

OLC_FUN(mpedit_touch)
{
	MPCODE *mpcode;
	EDIT_MPCODE(ch, mpcode);
	return touch_vnum(mpcode->vnum);
}

OLC_FUN(mpedit_show)
{
	MPCODE *mpcode;
	EDIT_MPCODE(ch, mpcode);

	char_printf(ch, "Vnum:       [%d]\n\rCode:\n\r%s\n\r",
		   mpcode->vnum, mpcode->code);

	return FALSE;
}

OLC_FUN(mpedit_code)
{
	MPCODE *mpcode;
	EDIT_MPCODE(ch, mpcode);
	return olced_str_text(ch, argument, mpedit_code, &mpcode->code);
}

OLC_FUN(mpedit_list)
{
	int count = 1;
	MPCODE *mpcode;
	BUFFER *buffer;
	bool fAll = !str_cmp(argument, "all");
	char blah;
	AREA_DATA *ad;

	buffer = buf_new(0);

	for (mpcode = mpcode_list; mpcode !=NULL; mpcode = mpcode->next)
		if (fAll || ENTRE(ch->in_room->area->min_vnum, mpcode->vnum, ch->in_room->area->max_vnum)) {
			ad = area_vnum_lookup(mpcode->vnum);

			if (ad == NULL)
				blah = '?';
			else
			if (IS_BUILDER(ch, ad))
				blah = '*';
			else
				blah = ' ';

			buf_printf(buffer, "[%3d] (%c) %5d\n\r", count, blah, mpcode->vnum);
			count++;
		}

	if (count == 1) {
		if (fAll)
			buf_add(buffer, "No mobprogs found.\n\r");
		else
			buf_add(buffer, "No mobprogs found in this area.\n\r");
	}

	page_to_char(buf_string(buffer), ch);
	buf_free(buffer);

	return FALSE;
}
