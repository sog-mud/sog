#ifndef _STRING_EDIT_H_
#define _STRING_EDIT_H_

/*
 * $Id: string_edit.h,v 1.2 1998-07-11 20:55:16 fjoe Exp $
 */

void	string_edit	(CHAR_DATA *ch, char **pString);
void    string_append   (CHAR_DATA *ch, char **pString);
char *	string_replace	(char * orig, char * old, char * new);
void    string_add      (CHAR_DATA *ch, const char *argument);
char *  format_string   (char *oldstring /*, bool fSpace */);
const char *  first_arg (const char *argument, char *arg_first, bool fCase);
char *	string_unpad	(char * argument);
char *	string_proper	(char * argument);

#endif
