/*
 * $Id: flag.h,v 1.15 2004-02-10 14:15:52 fjoe Exp $
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

const flaginfo_t *flag_slookup(const flaginfo_t *flag_table, const char *name);
const flaginfo_t *flag_cslookup(const flaginfo_t *flag_table, const char *name);
const flaginfo_t *flag_lookup(const flaginfo_t *flag_table, const char *name);

flag_t	_flag_value(const flaginfo_t *flag_table, const char *argument,
		    const flaginfo_t *(*lookup)(
			const flaginfo_t *, const char *));
#define flag_svalue(t, arg)	(_flag_value((t), (arg), flag_slookup))
#define flag_csvalue(t, arg)	(_flag_value((t), (arg), flag_cslookup))
#define flag_value(t, arg)	(_flag_value((t), (arg), flag_lookup))

const char *	flag_string	(const flaginfo_t *flag_table, flag_t bits);
const char *	flag_string_def	(const flaginfo_t *flag_table,
				 flag_t bits, flag_t def_bits);
const char *	flag_string_int	(const flaginfo_t *flag_table, flag_t val);

void show_flags_buf(BUFFER *output, const flaginfo_t *flag_table);

#endif
