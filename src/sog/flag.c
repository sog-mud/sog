/*
 * $Id: flag.c,v 1.34 2001-01-23 21:47:00 fjoe Exp $
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
const flaginfo_t *
_flag_lookup(const flaginfo_t *f, const char *name,
	     int (*cmpfun)(const char *, const char *))
{
	if (IS_NULLSTR(name))
		return NULL;

	while (f->name != NULL) {
		if (!cmpfun(name, f->name))
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
flag_t
_flag_value(const flaginfo_t *flag_table, const char *argument,
	    int (*cmpfun)(const char *, const char *))
{
	const flaginfo_t *f;
	const char *tname = flag_table->name;
	flag_t ttype = flag_table->bit;

	flag_table++;
	switch (ttype) {
	case TABLE_BITVAL: {
		flag_t marked = 0;

		/*
		 * Accept multiple flags.
		 */
		for (;;) {
			char word[MAX_INPUT_LENGTH];
			argument = one_argument(argument, word, sizeof(word));

			if (word[0] == '\0')
				break;

			f = _flag_lookup(flag_table, word, cmpfun);
			if (f == NULL) {
				log(LOG_ERROR, "_flag_value: %s: unknown flag name", word);
				continue;
			}

			SET_BIT(marked, f->bit);
		}

		return marked;
		/* NOT REACHED */
	}

	case TABLE_INTVAL:
		if ((f = _flag_lookup(flag_table, argument, cmpfun)) == NULL)
			return -1;
		return f->bit;
		/* NOT REACHED */

	default:
		log(LOG_BUG, "_flag_value: %s: unknown table type %d",
			   tname, ttype);
		break;
	}

	return 0;
}

static const flaginfo_t*
flag_ilookup(const flaginfo_t *f, flag_t val)
{
	for (f++; f->name != NULL; f++) {
		if (f->bit == val)
			return f;
	}

	return NULL;
}

const char *
flag_istring(const flaginfo_t *f, flag_t val)
{
	static char buf[MAX_STRING_LENGTH];

	if ((f = flag_ilookup(f, val)) != NULL)
		return f->name;

	snprintf(buf, sizeof(buf), "%d", (int) val);
	return buf;
}

#define NBUFS 3
#define BUFSZ 512

/*****************************************************************************
 Name:		flag_string( table, flags/stat )
 Purpose:	Returns string with name(s) of the flags or stat entered.
 Called by:	act_olc.c, olc.c, and olc_save.c.
 ****************************************************************************/
const char *flag_string(const flaginfo_t *flag_table, flag_t bits)
{
	static char buf[NBUFS][BUFSZ];
	static int cnt = 0;
	int flag;
	const char *tname = flag_table->name;
	flag_t ttype = flag_table->bit;

	cnt = (cnt + 1) % NBUFS;
	buf[cnt][0] = '\0';
	flag_table++;
	for (flag = 0; flag_table[flag].name; flag++) {
		switch (ttype) {
		case TABLE_BITVAL:
			if (IS_SET(bits, flag_table[flag].bit)) {
				strnzcat(buf[cnt], BUFSZ, " ");
				strnzcat(buf[cnt], BUFSZ,
					 flag_table[flag].name);
			}
			break;

		case TABLE_INTVAL:
			if (flag_table[flag].bit == bits) {
				strnzcpy(buf[cnt], BUFSZ,
					 flag_table[flag].name);
				return buf[cnt];
			}
			break;

		default:
			log(LOG_BUG, "flag_value: %s: unknown table type %d",
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
		return "unknown";				// notrans
		/* NOT REACHED */

	default:
		/* can't happen */
		buf[cnt][0] = '\0';
		return buf[cnt];
	}
}

void show_flags_buf(BUFFER *output, const flaginfo_t *flag_table)
{
	int  flag;
	int  col = 0;
 
	col = 0;
	flag_table++;
	for (flag = 0; flag_table[flag].name; flag++) {
		if (flag_table[flag].settable) {
			buf_printf(output, BUF_END, "%-19.18s",	// notrans
			    flag_table[flag].name);
			if (++col % 4 == 0)
				buf_append(output, "\n");
		}
	}
 
	if (col % 4 != 0)
		buf_append(output, "\n");
}

/*****************************************************************************
 Name:		show_flags
 Purpose:	Displays settable flags and stats.
 ****************************************************************************/
void show_flags(CHAR_DATA *ch, const flaginfo_t *flag_table)
{
	BUFFER *output;

	output = buf_new(-1);
	show_flags_buf(output, flag_table);
	page_to_char(buf_string(output), ch);
	buf_free(output);
}

