/*
 * $Id: flag.c,v 1.23 1999-03-08 13:56:04 fjoe Exp $
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

/*
 * flag_lookup -- lookup flag by name in flag table
 *		  f should point to real start of table
 *		  (impl. dependent values such as table type should
 *		   be skipped)
 */
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
flag64_t flag_value(const FLAG *flag64_table, const char *argument)
{
	const FLAG *f;
	const char *tname = flag64_table->name;
	flag64_t ttype = flag64_table->bit;

	flag64_table++;
	switch (ttype) {
	case TABLE_BITVAL: {
		flag64_t marked = 0;

		/*
		 * Accept multiple flags.
		 */
		for (;;) {
			char word[MAX_INPUT_LENGTH];
			argument = one_argument(argument, word, sizeof(word));

			if (word[0] == '\0')
				break;

			if ((f = flag_lookup(flag64_table, word)) == NULL)
				return 0;

			SET_BIT(marked, f->bit);
		}

		return marked;
		/* NOT REACHED */
	}

	case TABLE_INTVAL:
		if ((f = flag_lookup(flag64_table, argument)) == NULL)
			return -1;
		return f->bit;
		/* NOT REACHED */

	default:
		log_printf("flag_value: %s: unknown table type %d",
			   tname, ttype);
		break;
	}

	return 0;
}

#define NBUFS 3
#define BUFSZ 512

/*****************************************************************************
 Name:		flag_string( table, flags/stat )
 Purpose:	Returns string with name(s) of the flags or stat entered.
 Called by:	act_olc.c, olc.c, and olc_save.c.
 ****************************************************************************/
const char *flag_string(const FLAG *flag64_table, flag64_t bits)
{
	static char buf[NBUFS][BUFSZ];
	static int cnt = 0;
	int flag;
	const char *tname = flag64_table->name;
	flag64_t ttype = flag64_table->bit;

	cnt = (cnt + 1) % NBUFS;
	buf[cnt][0] = '\0';
	flag64_table++;
	for (flag = 0; flag64_table[flag].name; flag++) {
		switch (ttype) {
		case TABLE_BITVAL:
			if (IS_SET(bits, flag64_table[flag].bit)) {
				strnzcat(buf[cnt], BUFSZ, " ");
				strnzcat(buf[cnt], BUFSZ,
					 flag64_table[flag].name);
			}
			break;

		case TABLE_INTVAL:
			if (flag64_table[flag].bit == bits) {
				strnzcpy(buf[cnt], BUFSZ,
					 flag64_table[flag].name);
				return buf[cnt];
			}
			break;

		default:
			log_printf("flag_value: %s: unknown table type %d",
				   tname, ttype);
			buf[cnt][0] = '\0';
			return buf[cnt];
		}
	}

/*
 * if got there then buf[cnt] is filled with bitval names
 * or (in case the table is TABLE_INTVAL) value was not found
 *
 */
	switch (ttype) {
	case TABLE_BITVAL:
		return buf[cnt][0] ? buf[cnt]+1 : "none";
		/* NOT REACHED */

	case TABLE_INTVAL:
		return "unknown";
		/* NOT REACHED */

	default:
		/* can't happen */
		buf[cnt][0] = '\0';
		return buf[cnt];
	}
}

void show_flags_buf(BUFFER *output, const FLAG *flag64_table)
{
	int  flag;
	int  col = 0;
 
	col = 0;
	flag64_table++;
	for (flag = 0; flag64_table[flag].name; flag++) {
		if (flag64_table[flag].settable) {
			buf_printf(output, "%-19.18s", flag64_table[flag].name);
			if (++col % 4 == 0)
				buf_add(output, "\n");
		}
	}
 
	if (col % 4 != 0)
		buf_add(output, "\n");
}

/*****************************************************************************
 Name:		show_flags
 Purpose:	Displays settable flags and stats.
 ****************************************************************************/
void show_flags(CHAR_DATA *ch, const FLAG *flag64_table)
{
	BUFFER *output;

	output = buf_new(-1);
	show_flags_buf(output, flag64_table);
	page_to_char(buf_string(output), ch);
	buf_free(output);
}

