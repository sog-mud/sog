/*
 * $Id: flag.c,v 1.6 1998-08-17 18:47:03 fjoe Exp $
 */

/***************************************************************************
 *  File: bit.c                                                            *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was written by Jason Dinkel and inspired by Russ Taylor,     *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 *                                                                         *
 ***************************************************************************/
/*
 The code below uses a table lookup system that is based on suggestions
 from Russ Taylor.  There are many routines in handler.c that would benefit
 with the use of tables.  You may consider simplifying your code base by
 implementing a system like below with such functions. -Jason Dinkel
 */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "tables.h"
#include "util.h"
#include "db.h"
#include "comm.h"

struct flag_stat_type
{
	const FLAG *	structure;
	bool		is_stat;
};

/*****************************************************************************
 Name:		flag_stat_table
 Purpose:	This table catagorizes the tables following the lookup
 		functions below into stats and flags.  Flags can be toggled
 		but stats can only be assigned.  Update this table when a
 		new set of flags is installed.
 ****************************************************************************/
const struct flag_stat_type flag_stat_table[] =
{
/*	{ structure		is_stat	}, */
	{ area_flags,		FALSE	},
	{ sex_table,		TRUE	},
	{ exit_flags,		FALSE	},
	{ door_resets,		TRUE	},
	{ room_flags,		FALSE	},
	{ sector_types,		TRUE	},
	{ type_flags,		TRUE	},
	{ extra_flags,		FALSE	},
	{ wear_flags,		FALSE	},
	{ act_flags,		FALSE	},
	{ affect_flags,		FALSE	},
	{ detect_flags,		FALSE	},
	{ skill_groups,		FALSE	},
	{ apply_flags,		TRUE	},
	{ apply_types,		TRUE	},
	{ wear_loc_flags,	TRUE	},
	{ wear_loc_strings,	TRUE	},
	{ container_flags,	FALSE	},
	{ form_flags,		FALSE	},
	{ part_flags,		FALSE	},
	{ ac_type,		TRUE	},
	{ size_table,		TRUE	},
	{ position_table,	TRUE	},
	{ off_flags,		FALSE	},
	{ imm_flags,		FALSE	},
	{ res_flags,		FALSE	},
	{ vuln_flags,		FALSE	},
	{ weapon_class,		TRUE	},
	{ weapon_type2,		FALSE	},
	{ apply_types,		TRUE	},
	{ mptrig_flags,		FALSE	},
	{ mptrig_types,		TRUE	},
	{ furniture_flags,	TRUE	},
	{ NULL }
};

/*****************************************************************************
 Name:		is_stat( table )
 Purpose:	Returns TRUE if the table is a stat table and FALSE if flag.
 Called by:	flag_value and flag_string.
 Note:		This function is local and used only in bit.c.
 ****************************************************************************/
bool is_stat(const FLAG *flag_table)
{
	int flag;

	for (flag = 0; flag_stat_table[flag].structure; flag++)
		if (flag_stat_table[flag].structure == flag_table)
			return flag_stat_table[flag].is_stat;

	return FALSE;
}

static int flag_lookup(const char *name, const FLAG *f)
{
	if (IS_NULLSTR(name))
		return 0;

	while (f->name != NULL) {
		if (str_prefix(name, f->name) == 0)
			return f->bit;
		f++;
	}
	return -1;
}

/*****************************************************************************
 Name:		flag_value(table, flag)
 Purpose:	Returns the value of the flags entered.  Multi-flags accepted.
 Called by:	olc.c and olc_act.c.
 ****************************************************************************/
int flag_value(const FLAG *flag_table, const char *argument)
{
	int marked = 0;
	bool found = FALSE;

	if (is_stat(flag_table)) 
		return flag_lookup(argument, flag_table);

	/*
	 * Accept multiple flags.
	 */
	for (;;) {
		int bit;
		char word[MAX_INPUT_LENGTH];
		argument = one_argument(argument, word);

		if (word[0] == '\0')
			break;

		if ((bit = flag_lookup(word, flag_table)) < 0)
			return 0;

		SET_BIT(marked, bit);
		found = TRUE;
	}

	return marked;
}

#define NBUFS 3
#define BUFSZ 512

/*****************************************************************************
 Name:		flag_string( table, flags/stat )
 Purpose:	Returns string with name(s) of the flags or stat entered.
 Called by:	act_olc.c, olc.c, and olc_save.c.
 ****************************************************************************/
char *flag_string(const FLAG *flag_table, int bits)
{
	static char buf[NBUFS][BUFSZ];
	static int cnt = 0;
	int flag;
	bool stat;

	cnt = (cnt + 1) % NBUFS;
	buf[cnt][0] = '\0';

	stat = is_stat(flag_table);
	for (flag = 0; flag_table[flag].name != NULL; flag++) {
		if (!stat) {
			if (IS_SET(bits, flag_table[flag].bit)) {
				strnzcat(buf[cnt], " ", BUFSZ);
				strnzcat(buf[cnt], flag_table[flag].name,
					 BUFSZ);
			}
		}
		else if (flag_table[flag].bit == bits) {
			strnzcat(buf[cnt], " ", BUFSZ);
			strnzcat(buf[cnt], flag_table[flag].name, BUFSZ);
			break;
		}
	}
	return (buf[cnt][0] != '\0') ? buf[cnt]+1 : stat ? "unknown" : "none";
}

void do_flag(CHAR_DATA *ch, const char *argument)
{
    char arg1[MAX_INPUT_LENGTH],arg2[MAX_INPUT_LENGTH],arg3[MAX_INPUT_LENGTH];
    char word[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int *flag, old = 0, new = 0, marked = 0, pos;
    char type;
    const FLAG *flag_table;

    argument = one_argument(argument,arg1);
    argument = one_argument(argument,arg2);
    argument = one_argument(argument,arg3);

    type = argument[0];

    if (type == '=' || type == '-' || type == '+')
        argument = one_argument(argument,word);

    if (arg1[0] == '\0')
    {
	send_to_char("Syntax:\n\r",ch);
	send_to_char("  flag mob  <name> <field> <flags>\n\r",ch);
	send_to_char("  flag char <name> <field> <flags>\n\r",ch);
	send_to_char("  flag obj  <name> <field> <flags>\n\r",ch);
	send_to_char("  flag room <room> <field> <flags>\n\r",ch);
	send_to_char("  mob  flags: act,aff,off,imm,res,vuln,form,part,det,pra\n\r",ch);
	send_to_char("  char flags: plr,comm,aff,imm,res,vuln,\n\r",ch);
	send_to_char("  obj  flags: extra,wear,weap,cont,gate,exit\n\r",ch);
	send_to_char("  room flags: room\n\r",ch);
	send_to_char("  +: add flag, -: remove flag, = set equal to\n\r",ch);
	send_to_char("  otherwise flag toggles the flags listed.\n\r",ch);
	return;
    }

    if (arg2[0] == '\0')
    {
	send_to_char("What do you wish to set flags on?\n\r",ch);
	return;
    }

    if (arg3[0] == '\0')
    {
	send_to_char("You need to specify a flag to set.\n\r",ch);
	return;
    }

    if (argument[0] == '\0')
    {
	send_to_char("Which flags do you wish to change?\n\r",ch);
	return;
    }

    if (!str_prefix(arg1,"mob") || !str_prefix(arg1,"char"))
    {
	victim = get_char_world(ch,arg2);
	if (victim == NULL)
	{
	    send_to_char("You can't find them.\n\r",ch);
	    return;
	}

        /* select a flag to set */
	if (!str_prefix(arg3,"act"))
	{
	    if (!IS_NPC(victim))
	    {
		send_to_char("Use plr for PCs.\n\r",ch);
		return;
	    }

	    flag = &victim->act;
	    flag_table = act_flags;
	}

	else if (!str_prefix(arg3,"plr"))
	{
	    if (IS_NPC(victim))
	    {
		send_to_char("Use act for NPCs.\n\r",ch);
		return;
	    }

	    flag = &victim->act;
	    flag_table = plr_flags;
	}

 	else if (!str_prefix(arg3,"aff"))
	{
	    flag = &victim->affected_by;
	    flag_table = affect_flags;
	}

	else if (!str_prefix(arg3, "det"))
	{
	    flag = &victim->detection;
	    flag_table = detect_flags;
	}

  	else if (!str_prefix(arg3,"immunity"))
	{
	    flag = &victim->imm_flags;
	    flag_table = imm_flags;
	}

	else if (!str_prefix(arg3,"resist"))
	{
	    flag = &victim->res_flags;
	    flag_table = imm_flags;
	}

	else if (!str_prefix(arg3,"vuln"))
	{
	    flag = &victim->vuln_flags;
	    flag_table = imm_flags;
	}

	else if (!str_prefix(arg3,"form"))
	{
	    if (!IS_NPC(victim))
	    {
	 	send_to_char("Form can't be set on PCs.\n\r",ch);
		return;
	    }

	    flag = &victim->form;
	    flag_table = form_flags;
	}

	else if (!str_prefix(arg3,"parts"))
	{
	    if (!IS_NPC(victim))
	    {
		send_to_char("Parts can't be set on PCs.\n\r",ch);
		return;
	    }

	    flag = &victim->parts;
	    flag_table = part_flags;
	}

	else if (!str_prefix(arg3,"comm"))
	{
	    if (IS_NPC(victim))
	    {
		send_to_char("Comm can't be set on NPCs.\n\r",ch);
		return;
	    }

	    flag = &victim->comm;
	    flag_table = comm_flags;
	}

	else 
	{
	    send_to_char("That's not an acceptable flag.\n\r",ch);
	    return;
	}

	old = *flag;
	victim->zone = NULL;

	if (type != '=')
	    new = old;

        /* mark the words */
        for (; ;)
        {
	    argument = one_argument(argument,word);

	    if (word[0] == '\0')
		break;

	    pos = flag_lookup(word,flag_table);
	    if (pos < 0)
	    {
		send_to_char("That flag doesn't exist!\n\r",ch);
		return;
	    }
	    else
		SET_BIT(marked,pos);
	}

	for (pos = 0; flag_table[pos].name != NULL; pos++)
	{
	    if (!flag_table[pos].settable && IS_SET(old,flag_table[pos].bit))
	    {
		SET_BIT(new,flag_table[pos].bit);
		continue;
	    }

	    if (IS_SET(marked,flag_table[pos].bit))
	    {
		switch(type)
		{
		    case '=':
		    case '+':
			SET_BIT(new,flag_table[pos].bit);
			break;
		    case '-':
			REMOVE_BIT(new,flag_table[pos].bit);
			break;
		    default:
			if (IS_SET(new,flag_table[pos].bit))
			    REMOVE_BIT(new,flag_table[pos].bit);
			else
			    SET_BIT(new,flag_table[pos].bit);
		}
	    }
	}
	*flag = new;
	return;
    }
}
