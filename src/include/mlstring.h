#ifndef _MLSTRING_H_
#define _MLSTRING_H_

/*
 * $Id: mlstring.h,v 1.4 1998-07-25 15:02:40 fjoe Exp $
 */

mlstring *	mlstr_new	(void);
void 		mlstr_fread	(FILE *fp, mlstring *ml);
void		mlstr_fwrite	(FILE *fp, const char* name,
				 const mlstring *ml);
void		mlstr_clear	(mlstring *ml);
void 		mlstr_cpy	(mlstring *dest, const mlstring *src);

void		mlstr_printf	(mlstring *ml, ...);
char *		mlstr_val	(const mlstring *ml, int lang);
#define		mlstr_mval(ml)		mlstr_val(ml, 0)
#define		mlstr_cval(ml, ch)	mlstr_val(ml, ch->lang)
bool		mlstr_null	(const mlstring *ml);
int		mlstr_cmp	(const mlstring *ml1, const mlstring *ml2);

char** mlstr_convert(mlstring *ml, int newlang);

bool		mlstr_append	(CHAR_DATA *ch, mlstring *ml, const char *arg);
void		mlstr_format	(mlstring *ml);
bool		mlstr_change	(mlstring *ml, const char *arg);
bool		mlstr_change_desc(mlstring *ml, const char *argument);

void		mlstr_dump	(BUFFER *buf, const char *name,
				 const mlstring *ml);

#endif
