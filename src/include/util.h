#ifndef _UTIL_H_
#define _UTIL_H_

/*
 * $Id: util.h,v 1.3 1998-08-10 10:37:57 fjoe Exp $
 */

void	doprintf(DO_FUN*, CHAR_DATA*, const char *fmt, ...);
char *	strnzcpy(char *dest, const char *src, size_t);
char *	strnzcat(char *dest, const char *src, size_t);
char *	strlwr(char *s);

#endif
