/*
 * $Id: olc_mpcode.c,v 1.9 1998-09-01 18:29:26 fjoe Exp $
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
#include "olc.h"

#define MPEDIT(fun)           bool fun(CHAR_DATA *ch, const char *argument)
#define EDIT_MPCODE(Ch, Code)   (Code = (MPCODE*)Ch->desc->pEdit)

/* Mobprog editor */
DECLARE_OLC_FUN(mpedit_create		);
DECLARE_OLC_FUN(mpedit_code		);
DECLARE_OLC_FUN(mpedit_show		);
DECLARE_OLC_FUN(mpedit_list		);

OLC_CMD_DATA mpedit_table[] =
{
/*	{ command	function	}, */

	{ "commands",	show_commands	},
	{ "create",	mpedit_create	},
	{ "code",	mpedit_code	},
	{ "show",	mpedit_show	},
	{ "list",	mpedit_list	},

	{ NULL }
};

void mpedit(CHAR_DATA *ch, const char *argument)
{
	MPCODE *mpcode;
	char arg[MAX_INPUT_LENGTH];
	char command[MAX_INPUT_LENGTH];
	int cmd;
	AREA_DATA *ad;

	strcpy(arg, argument);
	smash_tilde(arg);
	argument = one_argument(arg, command);

	EDIT_MPCODE(ch, mpcode);

	if (mpcode) {
		ad = area_vnum_lookup(mpcode->vnum);

		if (ad == NULL) { /* ??? */
			edit_done(ch);
			return;
		}

		if (!IS_BUILDER(ch, ad)) {
			send_to_char("MPEdit: Insufficient security to modify code.\n\r", ch);
			edit_done(ch);
			return;
		}
	}

	if (command[0] == '\0') {
		mpedit_show(ch, argument);
		return;
	}

	if (!str_cmp(command, "done")) {
		edit_done(ch);
		return;
	}

	for (cmd = 0; mpedit_table[cmd].name != NULL; cmd++) {
		if (!str_prefix(command, mpedit_table[cmd].name)) {
			if ((*mpedit_table[cmd].olc_fun) (ch, argument) && mpcode)
				if ((ad = area_vnum_lookup(mpcode->vnum)) != NULL)
					SET_BIT(ad->flags, AREA_CHANGED);
			return;
		}
	}

	interpret(ch, arg);
}

void do_mpedit(CHAR_DATA *ch, const char *argument)
{
	MPCODE *mpcode;
	char command[MAX_INPUT_LENGTH];

	if (IS_NPC(ch))
		return;

	argument = one_argument(argument, command);

	if(is_number(command)) {
		int vnum = atoi(command);
		AREA_DATA *ad;

		if ((mpcode = mpcode_lookup(vnum)) == NULL) {
			send_to_char("MPEdit : That vnum does not exist.\n\r",ch);
			return;
		}

		ad = area_vnum_lookup(vnum);

		if (ad == NULL) {
			send_to_char("MPEdit : VNUM no asignado a ningun area.\n\r", ch);
			return;
		}

		if (!IS_BUILDER(ch, ad)) {
			send_to_char("MPEdit : Insuficiente seguridad para editar area.\n\r", ch);
			return;
		}

		ch->desc->pEdit		= (void *)mpcode;
		ch->desc->editor	= ED_MPCODE;

		return;
	}

	if (!str_cmp(command, "create")) {
		if (argument[0] == '\0') {
			send_to_char("Syntax: mpedit create [vnum]\n\r", ch);
			return;
		}

		mpedit_create(ch, argument);
		return;
	}

	send_to_char("Syntax : mpedit [vnum]\n\r", ch);
	send_to_char("         mpedit create [vnum]\n\r", ch);
}

MPEDIT (mpedit_create)
{
	MPCODE *mpcode;
	int value = atoi(argument);
	AREA_DATA *ad;

	if (IS_NULLSTR(argument) || value < 1) {
		send_to_char("Syntax: mpedit create [vnum]\n\r", ch);
		return FALSE;
	}

	ad = area_vnum_lookup(value);

	if (ad == NULL) {
		send_to_char("MPEdit : VNUM no asignado a ningun area.\n\r", ch);
		return FALSE;
	}
	
	if (!IS_BUILDER(ch, ad)) {
		send_to_char("MPEdit : Insuficiente seguridad para crear MobProgs.\n\r", ch);
		return FALSE;
	}

	if (mpcode_lookup(value)) {
		send_to_char("MPEdit: Code vnum already exists.\n\r",ch);
		return FALSE;
	}

	mpcode			= mpcode_new();
	mpcode->vnum		= value;
		mpcode_add(mpcode);
		ch->desc->pEdit		= (void *)mpcode;
		ch->desc->editor	= ED_MPCODE;

	send_to_char("MobProgram code created.\n\r",ch);

	return TRUE;
}

MPEDIT(mpedit_show)
{
	MPCODE *mpcode;

	EDIT_MPCODE(ch,mpcode);

	char_printf(ch, "Vnum:       [%d]\n\rCode:\n\r%s\n\r",
		   mpcode->vnum, mpcode->code);

	return FALSE;
}

MPEDIT(mpedit_code)
{
	MPCODE *mpcode;
	EDIT_MPCODE(ch, mpcode);
	return olced_str_text(ch, argument, mpedit_code, &mpcode->code);
}

MPEDIT(mpedit_list)
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
