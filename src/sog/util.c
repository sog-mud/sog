/*
 * $Id: util.c,v 1.1 1998-06-12 14:26:00 fjoe Exp $
 */

#include <stdarg.h>
#include <stdio.h>
#include "merc.h"
#include "util.h"

#ifdef SUNOS
#include "compat.h"
#endif

void doprintf(void (*fn)(CHAR_DATA* ch, char* arg), CHAR_DATA* ch,
	      const char* fmt, ...)
{
	char buf[MAX_STRING_LENGTH];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	fn(ch, buf);
	va_end(ap);
}
