#ifndef _UTIL_H_
#define _UTIL_H_

/*
 * $Id: util.h,v 1.2 1998-07-11 20:55:17 fjoe Exp $
 */

void doprintf(DO_FUN*, CHAR_DATA*, const char *fmt, ...);
char* strnzcpy(char *dest, const char *src, size_t);
char* strnzcat(char *dest, const char *src, size_t);

#endif
