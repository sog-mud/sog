/*
 * $Id: log.c,v 1.5 1998-09-01 18:38:00 fjoe Exp $
 */

#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#include "typedef.h"
#include "const.h"
#include "log.h"

#ifdef SUNOS
#	include "compat.h"
#endif

/*
 * Writes a string to the log.
 */
void log_printf(const char *format, ...)
{
	time_t current_time;
	char buf[MAX_STRING_LENGTH];
	va_list ap;

	va_start(ap, format);
	vsnprintf(buf, sizeof(buf), format, ap);
	va_end(ap);

	time(&current_time);
	fprintf(stderr, "%s :: %s\n", ctime(&current_time), buf);
}


/*
 * Reports a bug.
 */
void bug(const char *str, int param)
{
	char buf[MAX_STRING_LENGTH];

	strcpy(buf, "[*****] BUG: ");
	sprintf(buf + strlen(buf), str, param);
	log(buf);
}

