#ifndef _STRING_EDIT_H_
#define _STRING_EDIT_H_

/*
 * $Id: string_edit.h,v 1.1 1998-06-28 04:47:16 fjoe Exp $
 */

void	string_edit	(CHAR_DATA *ch, char **pString);
void    string_append   (CHAR_DATA *ch, char **pString);
char *	string_replace	(char * orig, char * old, char * new);
void    string_add      (CHAR_DATA *ch, char *argument);
char *  format_string   (char *oldstring /*, bool fSpace */);
char *  first_arg       (char *argument, char *arg_first, bool fCase);
char *	string_unpad	(char * argument);
char *	string_proper	(char * argument);

#endif
