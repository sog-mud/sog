#ifndef _COMPAT_H_
#define _COMPAT_H_

/*
 * $Id: compat.h,v 1.1 1998-06-06 10:51:54 fjoe Exp $
 */

char* strsep(char**, const char*);

#ifdef SUNOS
	int snprintf(char*, size_t, const char*, ...);
#	define vsnprintf(s, size, fmt, va) vsprintf(s, fmt, va)
#endif

#endif
