/*-
 * Copyright (c) 1998 fjoe <fjoe@iclub.nsu.ru>
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
 * $Id: util.c,v 1.9 1998-09-19 10:39:00 fjoe Exp $
 */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "merc.h"

#ifdef SUNOS
#	include "compat/compat.h"
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

char *strlwr(char *s)
{
	char *p;
	for (p = s; *p; p++)
		*p = LOWER(*p);
	return s;
}

/*
 * Compare strings, case insensitive.
 */
int str_cmp(const char *astr, const char *bstr)
{
	if (astr == NULL)
		return bstr == NULL ? 0 : -1;
	if (bstr == NULL)
		return 1;
	return strcasecmp(astr, bstr);
}

/*
 * Compare strings, case insensitive, for prefix matching.
 * Return TRUE if astr not a prefix of bstr
 *   (compatibility with historical functions).
 */
bool str_prefix(const char *astr, const char *bstr)
{
	if (astr == NULL)
	{
		bug("Strn_cmp: null astr.", 0);
		return TRUE;
	}

	if (bstr == NULL)
	{
		bug("Strn_cmp: null bstr.", 0);
		return TRUE;
	}

	for (; *astr; astr++, bstr++)
	{
		if (LOWER(*astr) != LOWER(*bstr))
		    return TRUE;
	}

	return FALSE;
}

/*
 * Compare strings, case insensitive, for match anywhere.
 * Returns TRUE is astr not part of bstr.
 *   (compatibility with historical functions).
 */
bool str_infix(const char *astr, const char *bstr)
{
	int sstr1;
	int sstr2;
	int ichar;
	char c0;

	if ((c0 = LOWER(astr[0])) == '\0')
		return FALSE;

	sstr1 = strlen(astr);
	sstr2 = strlen(bstr);

	for (ichar = 0; ichar <= sstr2 - sstr1; ichar++)
	{
		if (c0 == LOWER(bstr[ichar]) && !str_prefix(astr, bstr + ichar))
		    return FALSE;
	}

	return TRUE;
}

/*
 * Compare strings, case insensitive, for suffix matching.
 * Return TRUE if astr not a suffix of bstr
 *   (compatibility with historical functions).
 */
bool str_suffix(const char *astr, const char *bstr)
{
	int sstr1;
	int sstr2;

	sstr1 = strlen(astr);
	sstr2 = strlen(bstr);
	if (sstr1 <= sstr2 && !str_cmp(astr, bstr + sstr2 - sstr1))
		return FALSE;
	else
		return TRUE;
}

FILE *dfopen(const char *dir, const char *file, const char *mode)
{
	char *name = str_add(dir, "/", file, NULL);
	FILE *fp = fopen(name, mode);
	free_string(name);
	return fp;
}

int dunlink(const char *dir, const char *file)
{
	int res;
	char *name = str_add(dir, "/", file, NULL);
	res = unlink(name);
	free_string(name);
	return res;
}

int d2rename(const char *dir1, const char *file1,
	     const char *dir2, const char *file2)
{
	int res;
	char *name1 = str_add(dir1, "/", file1, NULL);
	char *name2 = str_add(dir2, "/", file2, NULL);
	res = rename(name1, name2);
	free_string(name1);
	free_string(name2);
	return res;
}

int cmpint(const void *p1, const void *p2)
{
	return *(int*) p1 - *(int*) p2;
}

int cmpstr(const void *p1, const void *p2)
{
	return str_cmp(*(char**)p1, *(char**) p2);
}

size_t cstrlen(const char *cstr)
{
	size_t res;

	if (cstr == NULL)
		return 0;

	res = strlen(cstr);
	while ((cstr = strchr(cstr, '{')) != NULL) {
		if (*(cstr+1) == '{')
			res--;
		else
			res -= 2;
		cstr += 2;
	}

	return res;
}

const char *cstrfirst(const char *cstr)
{
	if (cstr == NULL)
		return NULL;

	for (; *cstr == '{'; cstr++)
		if (*(cstr+1))
			cstr++;
	return cstr;
}
