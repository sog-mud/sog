#include <stdarg.h>
#include <stdio.h>
#include <sys/types.h>
#include "compat.h"

/*
 * $Id: snprintf.c,v 1.2 1998-06-06 12:35:26 fjoe Exp $
 */

int snprintf(char* buf, size_t size, const char* fmt, ...)
{
	int res;
	va_list ap;

	va_start(ap, fmt);
	res = vsnprintf(buf, size, fmt, ap);
	va_end(ap);

	return res;
}
