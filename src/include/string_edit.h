/*
 * $Id: string_edit.h,v 1.3 1998-09-17 15:51:23 fjoe Exp $
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

void	string_edit	(CHAR_DATA *ch, char **pString);
void    string_append   (CHAR_DATA *ch, char **pString);
char *	string_replace	(char * orig, char * old, char * new);
void    string_add      (CHAR_DATA *ch, const char *argument);
char *  format_string   (char *oldstring /*, bool fSpace */);
const char *  first_arg (const char *argument, char *arg_first, bool fCase);
char *	string_unpad	(char * argument);
char *	string_proper	(char * argument);

#endif
