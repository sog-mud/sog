#ifndef _MLSTRING_H_
#define _MLSTRING_H_

/*
 * $Id: mlstring.h,v 1.3 1998-07-14 07:47:48 fjoe Exp $
 */

mlstring *	mlstr_new	(void);
mlstring *	mlstr_fread	(FILE *fp);
void		mlstr_fwrite	(FILE *fp, const char* name,
				 const mlstring *ml);
void		mlstr_free	(mlstring *ml);
mlstring *	mlstr_dup	(const mlstring *ml);

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
