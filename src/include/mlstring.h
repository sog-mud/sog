#ifndef _MLSTRING_H_
#define _MLSTRING_H_

/*
 * $Id: mlstring.h,v 1.7 1998-08-15 07:47:34 fjoe Exp $
 */

mlstring * 	mlstr_fread	(FILE *fp);
void		mlstr_fwrite	(FILE *fp, const char* name,
				 const mlstring *ml);
void		mlstr_free	(mlstring *ml);
mlstring *	mlstr_dup	(mlstring *ml);
mlstring *	mlstr_printf	(mlstring *ml, ...);

char *		mlstr_val	(const mlstring *ml, int lang);
#define		mlstr_mval(ml)		mlstr_val(ml, 0)
#define		mlstr_cval(ml, ch)	mlstr_val(ml, ch->lang)
bool		mlstr_null	(const mlstring *ml);
int		mlstr_cmp	(const mlstring *ml1, const mlstring *ml2);

char** mlstr_convert(mlstring **mlp, int newlang);

bool		mlstr_append	(CHAR_DATA *ch, mlstring **mlp,
				 const char *arg);
void		mlstr_format	(mlstring **mlp);
bool		mlstr_change	(mlstring **mlp, const char *arg);
bool		mlstr_change_desc(mlstring **mlp, const char *arg);

void		mlstr_dump	(BUFFER *buf, const char *name,
				 const mlstring *ml);
extern mlstring mlstr_empty;

#endif
