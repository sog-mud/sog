#ifndef _MLSTRING_H_
#define _MLSTRING_H_

/*
 * $Id: mlstring.h,v 1.2 1998-07-11 20:55:13 fjoe Exp $
 */

mlstring *	mlstr_new	(void);
mlstring *	mlstr_fread	(FILE *fp);
void		mlstr_fwrite	(FILE *fp, const mlstring *ml);
void		mlstr_free	(mlstring *ml);
mlstring *	mlstr_dup	(const mlstring *ml);

void		mlstr_printf	(mlstring *ml, ...);
char *		mlstr_val	(CHAR_DATA *ch, const mlstring *ml);
char *		mlstr_mval	(const mlstring *ml);

char** mlstr_convert(mlstring *ml, int newlang);

bool		mlstr_append	(CHAR_DATA *ch, mlstring *ml, const char *arg);
void		mlstr_format	(mlstring *ml);
bool		mlstr_change	(mlstring *ml, const char *arg);
bool		mlstr_change_desc(mlstring *ml, const char *argument);

void		mlstr_buf	(BUFFER *buf, const char *name,
				 const mlstring *ml);

#endif
