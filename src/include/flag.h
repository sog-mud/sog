/*
 * $Id: flag.h,v 1.3 1999-02-15 18:19:39 fjoe Exp $
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

#ifndef _FLAG_H_
#define _FLAG_H_

enum {
	TABLE_BITVAL,	/* table contains bit values		*/
	TABLE_INTVAL	/* table contains integer values	*/
};

struct flag
{
	const char *	name;
	flag64_t	bit;
	bool		settable;
};

const FLAG *	flag_lookup	(const FLAG *flag64_table, const char* name);
flag64_t	flag_value	(const FLAG *flag64_table, const char *argument);
const char *	flag_string	(const FLAG *flag64_table, flag64_t bits);

void show_flags_buf(BUFFER *output, const FLAG *flag64_table);
void show_flags(CHAR_DATA *ch, const FLAG *flag64_table);

#endif

