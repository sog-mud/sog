#ifndef _FLAG_H_
#define _FLAG_H_

/*
 * $Id: flag.h,v 1.1 1998-09-01 18:37:57 fjoe Exp $
 */

/*-------------------------------------------------------------------
 * flag/stat stuff
 *
 * all flags in structures which are edited via OLC MUST BE flag_t
 * (otherwise olced_flag would not work)
 *
 */

struct flag
{
	char *	name;
	flag_t	bit;
	bool	settable;
};

int		is_stat		(const FLAG *flag_table);
const FLAG *	flag_lookup	(const FLAG *flag_table, const char* name);
flag_t		flag_value	(const FLAG *flag_table, const char *argument);
char *		flag_string	(const FLAG *flag_table, flag_t bits);

void show_flags_buf(BUFFER *output, const FLAG *flag_table);
void show_flags(CHAR_DATA *ch, const FLAG *flag_table);

#endif

