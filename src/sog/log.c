/*-
 * Copyright (c) 1999 SoG Development Team
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id: log.c,v 1.25 2000-04-16 09:21:55 fjoe Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>

#include "typedef.h"
#include "str.h"
#include "log.h"
#include "util.h"

#ifdef SUNOS
#	include "compat/compat.h"
#endif
#ifdef WIN32
#	include <string.h>
#	define vsnprintf	_vsnprintf
#endif

typedef struct logdata_t {
	int llevel;
	logger_t logger;
	logger_t logger_default;
} logdata_t;

/*
 * this list must be sorted by llevel (asc)
 */
static logdata_t logtab[] = {
	{ LOG_INFO,	logger_default,	logger_default	},
	{ LOG_WARN,	logger_default, logger_default	},
	{ LOG_ERROR,	logger_error,	logger_error	},
};
#define NLOG (sizeof(logtab) / sizeof(logdata_t))

/*
 * Writes a string to the log.
 */
void
log(int llevel, const char *format, ...)
{
	char buf[MAX_STRING_LENGTH];
	va_list ap;
	logdata_t *ld;

	va_start(ap, format);
	vsnprintf(buf, sizeof(buf), format, ap);
	va_end(ap);

	ld = bsearch(&llevel, logtab, NLOG, sizeof(logdata_t), cmpint);
	if (ld == NULL)
		ld = logtab;
	ld->logger(buf);
}

logger_t
logger_set(int llevel, logger_t logger_new)
{
	logdata_t *ld;
	logger_t logger_old;

	ld = bsearch(&llevel, logtab, NLOG, sizeof(logdata_t), cmpint);
	if (ld == NULL)
		return NULL;

	if (logger_new == NULL)
		logger_new = ld->logger_default;

	logger_old = ld->logger;
	ld->logger = logger_new;
	return logger_old;
}

void
logger_default(const char *buf)
{
#if defined(WIN32)
	FILE *logfile;
#endif

	fprintf(stderr, "%s :: %s\n", strtime(time(NULL)), buf);

#if defined (WIN32)
	/* Also add to logfile */
	logfile = fopen("sog.log", "a+b");
	if (logfile) {
		fprintf(logfile, "%s :: %s\n", strtime(current_time), buf);
		fclose(logfile);
	}
#endif
}

void
logger_error(const char *buf)
{
	char buf2[MAX_STRING_LENGTH];
	snprintf(buf2, sizeof(buf2), "[*****] BUG: %s", buf);
	logger_default(buf2);
}

