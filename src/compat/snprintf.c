#include <stdarg.h>
#include <stdio.h>
#include <sys/types.h>
#include <compat/compat.h>

/*
 * $Id: snprintf.c,v 1.3 2003-09-29 23:11:19 fjoe Exp $
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
