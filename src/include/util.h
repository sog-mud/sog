#ifndef _UTIL_H_
#define _UTIL_H_

/*
 * $Id: util.h,v 1.4 1998-09-01 18:29:20 fjoe Exp $
 */

void	doprintf(DO_FUN*, CHAR_DATA*, const char *fmt, ...);

char *	strnzcpy(char *dest, const char *src, size_t);
char *	strnzcat(char *dest, const char *src, size_t);
char *	strlwr(char *s);

int	str_cmp		(const char *astr, const char *bstr);
bool	str_prefix	(const char *astr, const char *bstr);
bool	str_infix	(const char *astr, const char *bstr);
bool	str_suffix	(const char *astr, const char *bstr);

FILE *	dfopen(const char *dir, const char *file, const char *mode);
int	dunlink(const char *dir, const char *file);
int	d2rename(const char *dir1, const char *file1,
		 const char *dir2, const char *file2);

int cmpint(const void *p1, const void *p2);
int cmpstr(const void *p1, const void *p2);

size_t cstrlen(const char* cstr);

#endif
