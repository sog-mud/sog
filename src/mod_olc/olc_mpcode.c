/*
 * $Id: olc_mpcode.c,v 1.30 2000-10-07 18:15:00 fjoe Exp $
 */

/* The following code is based on ILAB OLC by Jason Dinkel */
/* Mobprogram code by Lordrom for Nevermore Mud */

#include <sys/types.h>
#include <ctype.h>
#include <time.h>
#include "olc.h"

#define EDIT_MPCODE(ch, mpcode)   (mpcode = (MPCODE*) ch->desc->pEdit)

/* Mobprog editor */
DECLARE_OLC_FUN(mped_create		);
DECLARE_OLC_FUN(mped_edit		);
DECLARE_OLC_FUN(mped_touch		);
DECLARE_OLC_FUN(mped_show		);
DECLARE_OLC_FUN(mped_list		);

DECLARE_OLC_FUN(mped_code		);

olc_cmd_t olc_cmds_mpcode[] =
{
/*	{ command	function	}, */

	{ "create",	mped_create	},
	{ "edit",	mped_edit	},
	{ "",		NULL		},
	{ "touch",	mped_touch	},
	{ "show",	mped_show	},
	{ "list",	mped_list	},

	{ "code",	mped_code	},

	{ "commands",	show_commands	},

	{ NULL }
};

OLC_FUN(mped_create)
{
	MPCODE *mpcode;
	int value;
	AREA_DATA *pArea;
	char arg[MAX_STRING_LENGTH];

	one_argument(argument, arg, sizeof(arg));
	value = atoi(arg);
	if (!value)
		OLC_ERROR("'OLC CREATE'");

	pArea = area_vnum_lookup(value);
	if (!pArea) {
		act_char("MPEd: That vnum is not assigned an area.", ch);
		return FALSE;
	}

	if (!IS_BUILDER(ch, pArea)) {
		act_char("MPEd: Insufficient security.", ch);
		return FALSE;
	}

	if (mpcode_lookup(value)) {
		act_char("MPEd: vnum already exists.", ch);
		return FALSE;
	}

	mpcode		= mpcode_new();
	mpcode->vnum	= value;
	mpcode_add(mpcode);

	ch->desc->pEdit	= (void*) mpcode;
	OLCED(ch)	= olced_lookup(ED_MPCODE);
	TOUCH_AREA(pArea);
	act_char("MPEd: mpcode created.", ch);
	return FALSE;
}

OLC_FUN(mped_edit)
{
	MPCODE *mpcode;
	AREA_DATA *pArea;
	int value;
	char arg[MAX_INPUT_LENGTH];

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0')
		OLC_ERROR("'OLC EDIT'");

	value = atoi(arg);
	mpcode = mpcode_lookup(value);
	if (!mpcode) {
		act_char("MPEd: Vnum does not exist.", ch);
		return FALSE;
	}

	pArea = area_vnum_lookup(mpcode->vnum);
	if (!IS_BUILDER(ch, pArea)) {
		act_char("MPEd: Insufficient security.", ch);
	       	return FALSE;
	}

	ch->desc->pEdit = (void*) mpcode;
	OLCED(ch)	= olced_lookup(ED_MPCODE);
	return FALSE;
}

OLC_FUN(mped_touch)
{
	MPCODE *mpcode;
	EDIT_MPCODE(ch, mpcode);
	TOUCH_VNUM(mpcode->vnum);
	return FALSE;
}

OLC_FUN(mped_show)
{
	MPCODE *mpcode;
	char arg[MAX_INPUT_LENGTH];

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		if (IS_EDIT(ch, ED_MPCODE))
			EDIT_MPCODE(ch, mpcode);
		else
			OLC_ERROR("'OLC ASHOW'");
	}
	else {
		int value = atoi(arg);
		mpcode = mpcode_lookup(value);
		if (!mpcode) {
			act_char("MPEd: Vnum does not exist.", ch);
			return FALSE;
		}
	}

	char_printf(ch, "Vnum: [%d]\n", mpcode->vnum);
	act_char("Code:", ch);
	char_printf(ch, "%s\n", mpcode->code);

	return FALSE;
}

OLC_FUN(mped_list)
{
	int count = 1;
	MPCODE *mpcode;
	BUFFER *buffer;
	bool fAll = !str_cmp(argument, "all");
	char blah;
	AREA_DATA *ad;

	buffer = buf_new(-1);

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

			buf_printf(buffer, BUF_END, "[%3d] (%c) %5d\n", count, blah, mpcode->vnum);
			count++;
		}

	if (count == 1) {
		if (fAll)
			buf_append(buffer, "No mobprogs found.\n");
		else
			buf_append(buffer, "No mobprogs found in this area.\n");
	}

	page_to_char(buf_string(buffer), ch);
	buf_free(buffer);

	return FALSE;
}

OLC_FUN(mped_code)
{
	MPCODE *mpcode;
	EDIT_MPCODE(ch, mpcode);
	return olced_str_text(ch, argument, cmd, &mpcode->code);
}

