/*
 * $Id: flag.h,v 1.10 2001-08-13 18:23:15 fjoe Exp $
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

struct flaginfo_t
{
	const char *	name;
	flag_t		bit;
	bool		settable;
};

const flaginfo_t *_flag_lookup(const flaginfo_t *flag_table, const char* name,
			       int (*cmpfun)(const char *, const char *));
#define flag_lookup(t, n)	(_flag_lookup((t), (n), str_prefix))
#define flag_slookup(t, n)	(_flag_lookup((t), (n), str_cmp))

flag_t	_flag_value	(const flaginfo_t *flag_table,
			 const char *argument,
			 int (*cmpfun)(const char *, const char *));
#define flag_value(t, arg)	(_flag_value((t), (arg), str_prefix))
#define flag_svalue(t, arg)	(_flag_value((t), (arg), str_cmp))

const char *	flag_string	(const flaginfo_t *flag_table, flag_t bits);
const char *	flag_istring	(const flaginfo_t *flag_table, flag_t val);

void show_flags_buf(BUFFER *output, const flaginfo_t *flag_table);

#endif

