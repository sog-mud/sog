/*
 * $Id: olc_mpcode.c,v 1.4 1998-08-14 22:33:06 fjoe Exp $
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
#include "recycle.h"
#include "comm.h"
#include "db.h"
#include "string_edit.h"
#include "buffer.h"

#define MPEDIT(fun)           bool fun(CHAR_DATA *ch, const char *argument)


const struct olc_cmd_type mpedit_table[] =
{
/*	{	command		function	}, */

	{	"commands",	show_commands	},
	{	"create",	mpedit_create	},
	{	"code",		mpedit_code	},
	{	"show",		mpedit_show	},
	{	"list",		mpedit_list	},
	{	"?",		show_help	},

	{	NULL,		0		}
};

void mpedit(CHAR_DATA *ch, const char *argument)
{
    MPROG_CODE *pMcode;
    char arg[MAX_INPUT_LENGTH];
    char command[MAX_INPUT_LENGTH];
    int cmd;
    AREA_DATA *ad;

    strcpy(arg, argument);
    smash_tilde(arg);
    argument = one_argument(arg, command);

    EDIT_MPCODE(ch, pMcode);

    if (pMcode)
    {
	ad = get_vnum_area(pMcode->vnum);

	if (ad == NULL) /* ??? */
	{
		edit_done(ch);
		return;
	}

	if (!IS_BUILDER(ch, ad))
	{
		send_to_char("MPEdit: Insufficient security to modify code.\n\r", ch);
		edit_done(ch);
		return;
	}
    }

    if (command[0] == '\0')
    {
        mpedit_show(ch, argument);
        return;
    }

    if (!str_cmp(command, "done"))
    {
        edit_done(ch);
        return;
    }

    for (cmd = 0; mpedit_table[cmd].name != NULL; cmd++)
    {
	if (!str_prefix(command, mpedit_table[cmd].name))
	{
		if ((*mpedit_table[cmd].olc_fun) (ch, argument) && pMcode)
			if ((ad = get_vnum_area(pMcode->vnum)) != NULL)
				SET_BIT(ad->area_flags, AREA_CHANGED);
		return;
	}
    }

    interpret(ch, arg);

    return;
}

void do_mpedit(CHAR_DATA *ch, const char *argument)
{
    MPROG_CODE *pMcode;
    char command[MAX_INPUT_LENGTH];

    argument = one_argument(argument, command);

    if(is_number(command))
    {
	int vnum = atoi(command);
	AREA_DATA *ad;

	if ((pMcode = get_mprog_index(vnum)) == NULL)
	{
		send_to_char("MPEdit : That vnum does not exist.\n\r",ch);
		return;
	}

	ad = get_vnum_area(vnum);

	if (ad == NULL)
	{
		send_to_char("MPEdit : VNUM no asignado a ningun area.\n\r", ch);
		return;
	}

	if (!IS_BUILDER(ch, ad))
	{
		send_to_char("MPEdit : Insuficiente seguridad para editar area.\n\r", ch);
		return;
	}

	ch->desc->pEdit		= (void *)pMcode;
	ch->desc->editor	= ED_MPCODE;

	return;
    }

    if (!str_cmp(command, "create"))
    {
	if (argument[0] == '\0')
	{
		send_to_char("Sintaxis : mpedit create [vnum]\n\r", ch);
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
    MPROG_CODE *pMcode;
    int value = atoi(argument);
    AREA_DATA *ad;

    if (IS_NULLSTR(argument) || value < 1)
    {
	send_to_char("Sintaxis : mpedit create [vnum]\n\r", ch);
	return FALSE;
    }

    ad = get_vnum_area(value);

    if (ad == NULL)
    {
    	send_to_char("MPEdit : VNUM no asignado a ningun area.\n\r", ch);
    	return FALSE;
    }
    
    if (!IS_BUILDER(ch, ad))
    {
        send_to_char("MPEdit : Insuficiente seguridad para crear MobProgs.\n\r", ch);
        return FALSE;
    }

    if (get_mprog_index(value))
    {
	send_to_char("MPEdit: Code vnum already exists.\n\r",ch);
	return FALSE;
    }

    pMcode			= new_mpcode();
    pMcode->vnum		= value;
    pMcode->next		= mprog_list;
    mprog_list			= pMcode;
    ch->desc->pEdit		= (void *)pMcode;
    ch->desc->editor		= ED_MPCODE;

    send_to_char("MobProgram code created.\n\r",ch);

    return TRUE;
}

MPEDIT(mpedit_show)
{
    MPROG_CODE *pMcode;

    EDIT_MPCODE(ch,pMcode);

    char_printf(ch, "Vnum:       [%d]\n\rCode:\n\r%s\n\r",
           pMcode->vnum, pMcode->code);

    return FALSE;
}

MPEDIT(mpedit_code)
{
    MPROG_CODE *pMcode;
    EDIT_MPCODE(ch, pMcode);

    if (argument[0] =='\0')
    {
       string_append(ch, &pMcode->code);
       return TRUE;
    }

    send_to_char("Syntax: code\n\r",ch);
    return FALSE;
}

MPEDIT(mpedit_list)
{
    int count = 1;
    MPROG_CODE *mprg;
    BUFFER *buffer;
    bool fAll = !str_cmp(argument, "all");
    char blah;
    AREA_DATA *ad;

    buffer = buf_new(0);

    for (mprg = mprog_list; mprg !=NULL; mprg = mprg->next)
	if (fAll || ENTRE(ch->in_room->area->min_vnum, mprg->vnum, ch->in_room->area->max_vnum))
	{
		ad = get_vnum_area(mprg->vnum);

		if (ad == NULL)
			blah = '?';
		else
		if (IS_BUILDER(ch, ad))
			blah = '*';
		else
			blah = ' ';

		buf_printf(buffer, "[%3d] (%c) %5d\n\r", count, blah, mprg->vnum);
		count++;
	}

    if (count == 1)
    {
    	if (fAll)
    		buf_add(buffer, "No mobprogs found.\n\r");
    	else
    		buf_add(buffer, "No mobprogs found in this area.\n\r");
    }

    page_to_char(buf_string(buffer), ch);
    buf_free(buffer);

    return FALSE;
}
