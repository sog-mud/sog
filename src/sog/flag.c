/*
 * $Id: flag.c,v 1.14 1998-09-22 18:07:15 fjoe Exp $
 */

/***************************************************************************
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
#include "util.h"
#include "log.h"

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
	{ act_flags,		FALSE	},
/*	{ skill_flags,		FALSE	},	not implemented yet */
	{ plr_flags,		FALSE	},
	{ affect_flags,		FALSE	},
	{ raffect_flags,	FALSE	},
	{ off_flags,		FALSE	},
	{ imm_flags,		FALSE	},
	{ form_flags,		FALSE	},
	{ part_flags,		FALSE	},
	{ comm_flags,		FALSE	},
	{ extra_flags,		FALSE	},
	{ wear_flags,		FALSE	},
	{ portal_flags,		FALSE	},
	{ room_flags,		FALSE	},
	{ exit_flags,		FALSE	},
	{ area_flags,		FALSE	},
	{ sector_types,		TRUE	},
	{ wear_loc_strings,	TRUE	},
	{ wear_loc_flags,	TRUE	},
	{ door_resets,		TRUE	},
	{ res_flags,		FALSE	},
	{ vuln_flags,		FALSE	},
	{ apply_flags,		TRUE	},
	{ rapply_flags,		TRUE	},
	{ sex_table,		TRUE	},
	{ furniture_flags,	FALSE	},
	{ weapon_class,		TRUE	},
	{ apply_types,		TRUE	},
	{ weapon_type2,		FALSE	},
	{ size_table,		TRUE	},
	{ position_table,	TRUE	},
	{ position_names,	TRUE	},
	{ ac_type,		TRUE	},
	{ cont_flags,		FALSE	},
	{ skill_groups,		FALSE	},
	{ skill_targets,	TRUE	},
	{ mptrig_types,		TRUE	},
	{ mptrig_flags,		FALSE	},
	{ clan_flags,		FALSE	},
	{ item_types,		TRUE	},
	{ slang_table,		TRUE	},
	{ stat_names,		TRUE	},
	{ skill_flags,		FALSE	},
	{ class_flags,		FALSE	},
	{ align_names,		TRUE	},
	{ NULL }
};

/*****************************************************************************
 Name:		is_stat( table )
 Purpose:	Returns TRUE if the table is a stat table and FALSE if flag.
 Called by:	flag_value and flag_string.
 Note:		This function is local and used only in bit.c.
 ****************************************************************************/
int is_stat(const FLAG *flag_table)
{
	int flag;

	for (flag = 0; flag_stat_table[flag].structure; flag++)
		if (flag_stat_table[flag].structure == flag_table)
			return flag_stat_table[flag].is_stat;

	log_printf("flag_table[0] == '%s': not in flag_stat_table",
		   flag_table[0].name);
	return -1;
}

const FLAG* flag_lookup(const FLAG *f, const char *name)
{
	if (IS_NULLSTR(name))
		return NULL;

	while (f->name != NULL) {
		if (str_prefix(name, f->name) == 0)
			return f;
		f++;
	}
	return NULL;
}

/*****************************************************************************
 Name:		flag_value(table, flag)
 Purpose:	Returns the value of the flags entered.  Multi-flags accepted.
 Called by:	olc.c and olc_act.c.
 ****************************************************************************/
flag_t flag_value(const FLAG *flag_table, const char *argument)
{
	const FLAG *f;
	flag_t marked;

	if (is_stat(flag_table)) {
		if ((f = flag_lookup(flag_table, argument)) == NULL)
			return -1;
		return f->bit;
	}

	marked = 0;

	/*
	 * Accept multiple flags.
	 */
	for (;;) {
		char word[MAX_INPUT_LENGTH];
		argument = one_argument(argument, word);

		if (word[0] == '\0')
			break;

		if ((f = flag_lookup(flag_table, word)) == NULL)
			return 0;

		SET_BIT(marked, f->bit);
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
char *flag_string(const FLAG *flag_table, flag_t bits)
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

void show_flags_buf(BUFFER *output, const FLAG *flag_table)
{
	int  flag;
	int  col;
 
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

}

/*****************************************************************************
 Name:		show_flags
 Purpose:	Displays settable flags and stats.
 ****************************************************************************/
void show_flags(CHAR_DATA *ch, const FLAG *flag_table)
{
	BUFFER *output;

	output = buf_new(0);
	show_flags_buf(output, flag_table);
	page_to_char(buf_string(output), ch);
	buf_free(output);
}

