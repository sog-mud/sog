#ifndef _MLSTRING_H_
#define _MLSTRING_H_

/*
 * $Id: mlstring.h,v 1.1 1998-07-10 10:39:40 fjoe Exp $
 */

mlstring * fread_mlstring(FILE *fp);
void fwrite_mlstring(FILE *fp, mlstring *ml);
void free_mlstring(mlstring *ml);
char* ml_string(CHAR_DATA *ch, mlstring *ml);
char* ml_estring(mlstring *ml);
bool mlstring_append(CHAR_DATA *ch, mlstring *ml, const char *arg);
void format_mlstring(mlstring *ml);
bool mlstring_change(mlstring *ml, const char *arg);
void mlstring_buf(BUFFER *buf, const char *name, mlstring *ml);

extern mlstring mlstr_empty;

#endif
