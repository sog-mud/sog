#include <stdarg.h>
#include "compat.h"

/*
 * $Id: snprintf.c,v 1.1 1998-06-06 10:51:57 fjoe Exp $
 */

#ifdef SUNOS
#	include <stdio.h>
#endif

int snprintf(char* buf, size_t size, const char* fmt, ...)
{
	int res;
	va_list ap;

	va_start(ap, fmt);
	res = vsnprintf(buf, size, fmt, ap);
	va_end(ap);

	return res;
}
