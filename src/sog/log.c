#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#include "merc.h"
#include "log.h"

#ifdef SUNOS
#	include "compat.h"
#endif

/*
 * Writes a string to the log.
 */
void log_printf(const char *format, ...)
{
	char buf[MAX_STRING_LENGTH];
	va_list ap;

	va_start(ap, format);
	vsnprintf(buf, sizeof(buf), format, ap);
	va_end(ap);

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

