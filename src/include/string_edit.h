/*
 * $Id: string_edit.h,v 1.8 2000-04-04 13:58:23 fjoe Exp $
 */

/***************************************************************************
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was freely distributed with the The Isles 1.1 source code,   *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 *                                                                         *
 ***************************************************************************/

#ifndef _STRING_EDIT_H_
#define _STRING_EDIT_H_

/*
 * string_replace flags
 */
#define SR_F_ALL	(A)	/* replace all occurences */

void		string_append   (CHAR_DATA *ch, const char **pString);
const char *	string_replace	(const char *orig,
				 const char *old, const char *new,
				 int flags);
void		string_add      (CHAR_DATA *ch, const char *argument);
void		string_add_exit	(CHAR_DATA *ch, bool save);
const char *	format_string   (const char *oldstring /*, bool fSpace */);

const char *	string_linedel(const char *, int);
const char *	string_lineadd(const char *, char *, int);

#endif
