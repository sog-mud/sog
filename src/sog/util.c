/*
 * $Id: util.c,v 1.3 1998-07-14 18:26:24 fjoe Exp $
 */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "merc.h"
#include "util.h"

#ifdef SUNOS
#include "compat.h"
#endif

void doprintf(DO_FUN *fn, CHAR_DATA* ch, const char* fmt, ...)
{
	char buf[MAX_STRING_LENGTH];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	fn(ch, buf);
	va_end(ap);
}

/*
 * strnzcpy - copy from dest to src and always append terminating '\0'.
 *            len MUST BE > 0
 */
char *strnzcpy(char *dest, const char *src, size_t len)
{
	strncpy(dest, src, len);
	dest[len-1] = '\0';
	return dest;
}

char *strnzcat(char *dest, const char *src, size_t len)
{
	size_t old_len;

	old_len = strlen(dest);
	if (old_len >= len - 1)
		return dest;

	strncat(dest, src, len - old_len - 1);
	return dest;
}
