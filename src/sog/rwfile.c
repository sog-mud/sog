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
 * $Id: rwfile.c,v 1.23 2001-09-12 12:32:52 fjoe Exp $
 */

#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <typedef.h>
#include <rwfile.h>
#include <log.h>
#include <varr.h>
#include <hash.h>
#include <str.h>
#include <mlstring.h>
#include <flag.h>
#include <util.h>

#if !defined(NO_MMAP)

#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

static char str_end[] = "End";					// notrans
int line_number;

static int
count_lf(const char *p, off_t len)
{
	int count = 0;

	while (len--)
	for (;;) {
		char *q = memchr(p, '\n', (size_t) len);
		if (q == NULL)
			return count;

		count++;
		len -= q - p;
		p = q + 1;
	}

	/* NOT REACHED */
	return 0;
}

int
xgetc(rfile_t *fp)
{
	int c;

	if (rfile_feof(fp))
		return EOF;

	c = fp->p[fp->pos++];
	if (c == '\n')
		line_number++;
	return c;
}

void
xungetc(rfile_t *fp)
{
	if (!fp->pos)
		return;

	fp->pos--;

	if (!rfile_feof(fp)
	&&  fp->p[fp->pos] == '\n')
		line_number--;
}

rfile_t *
rfile_open(const char *dir, const char *file)
{
	char name[PATH_MAX];
	int fd;
	struct stat s;
	char *p;
	rfile_t *fp;

	snprintf(name, sizeof(name), "%s%c%s", dir, PATH_SEPARATOR, file);
	if ((fd = open(name, O_RDONLY)) < 0) {
		log(LOG_INFO, "%s: %s", name, strerror(errno));
		return NULL;
	}

	if (fstat(fd, &s) < 0) {
		close(fd);
		log(LOG_INFO, "%s: %s", name, strerror(errno));
		return NULL;
	}

	if ((p = mmap(NULL, (size_t) s.st_size, PROT_READ, 0, fd, (off_t) 0)) == MAP_FAILED) {
		close(fd);
		log(LOG_INFO, "%s: %s", name, strerror(errno));
		return NULL;
	}

#if !defined(LINUX)
	if (madvise(p, (size_t) s.st_size, MADV_SEQUENTIAL) < 0)
		log(LOG_INFO, "%s: %s", name, strerror(errno));
#endif

	fp = malloc(sizeof(rfile_t));
	fp->p = p;
	fp->len = s.st_size;
	fp->pos = 0;
	fp->fd = fd;
	fp->tok = str_empty;
	fp->tok_len = 0;
	return fp;
}

void
rfile_close(rfile_t *fp)
{
	munmap((void *)(uintptr_t)fp->p, (size_t) fp->len);
	close(fp->fd);
	free(fp);
}

bool
IS_TOKEN(rfile_t *fp, const char *k)
{
	int i;

	for (i = 0; i < fp->tok_len && *k; i++, k++) {
		if (UPPER(fp->tok[i]) != UPPER(*k))
			return FALSE;
	}

	return (i == fp->tok_len) && (*k == '\0');
}

const char *
rfile_tok(rfile_t *fp)
{
	static char buf[MAX_STRING_LENGTH];
	strnzncpy(buf, sizeof(buf), fp->tok, (size_t) fp->tok_len);
	return buf;
}

void
fread_word(rfile_t *fp)
{
	char cEnd;
	off_t oldpos;

	for (;;) {
		cEnd = fread_letter(fp);

		if (cEnd == '%') {
			fread_to_eol(fp);
			continue;
		}

		break;
	}

	if (rfile_feof(fp)) {
		fp->tok = str_empty;
		fp->tok_len = 0;
		return;
	}

	if (cEnd == '\'' || cEnd == '"') {
		u_char *p;

		/*
		 * find closing quote
		 */
		fp->tok = fp->p + fp->pos;
		p = memchr(fp->tok, cEnd, (size_t) (fp->len - fp->pos));

		/*
		 * calculate fp->tok_len and advance fp->pos
		 */
		oldpos = fp->pos;
		fp->pos = (p == NULL ? fp->len : p - fp->p);
		fp->tok_len = fp->pos - oldpos;
		if (p)
			fp->pos++;
		line_number += count_lf(fp->tok, fp->tok_len);
	} else {
		oldpos = fp->pos - 1;
		fp->tok = fp->p + oldpos;

		/*
		 * find first `isspace' symbol
		 */
		for (; fp->pos < fp->len; fp->pos++) {
			if (isspace(fp->p[fp->pos]))
				break;
		}
		fp->tok_len = fp->pos - oldpos;
	}

	if (fp->tok_len >= MAX_STRING_LENGTH) {
		log(LOG_WARN, "fread_word: word too long, truncated");
		fp->tok_len = MAX_STRING_LENGTH-1;
	}
}

void
fread_keyword(rfile_t *fp)
{
	if (rfile_feof(fp)) {
		fp->tok = str_end;
		fp->tok_len = strlen(str_end);
	} else
		fread_word(fp);
}

#else /* !defined(NO_MMAP) */

char _token[MAX_STRING_LENGTH];

static int last_c;

int
xgetc(rfile_t *fp)
{
	int c = getc(fp);
	if (c == '\n')
		line_number++;
	return last_c = c;
}

void
xungetc(rfile_t *fp)
{
	if (last_c == '\n')
		line_number--;
	ungetc(last_c, fp);
}

/*
 * Read one word (into static buffer).
 */
void
fread_word(rfile_t *fp)
{
	char *pword;
	char cEnd;

	for (;;) {
		cEnd = fread_letter(fp);

		if (cEnd == '%') {
			fread_to_eol(fp);
			continue;
		}

		break;
	}

	if (cEnd == '\'' || cEnd == '"')
		pword   = _token;
	else {
		_token[0] = cEnd;
		pword   = _token+1;
		cEnd    = ' ';
	}

	for (; pword < _token + sizeof(_token); pword++) {
		*pword = xgetc(fp);
		if (cEnd == ' ' ? isspace(*pword) : *pword == cEnd) {
			if (cEnd == ' ')
				xungetc(fp);
			*pword = '\0';
			return;
		}
	}

	log(LOG_ERROR, "fread_word: word too long");
}

void
fread_keyword(rfile_t *fp)
{
	if (rfile_feof(fp))
		strnzcpy(_token, sizeof(_token), str_end);
	else
		fread_word(fp);
}

#endif /* !defined(NO_MMAP) */

const char *
fread_sword(rfile_t *fp)
{
	fread_word(fp);
	return str_dup(rfile_tok(fp));
}

const char *
fread_string(rfile_t *fp)
{
	char buf[MAX_STRING_LENGTH];
	char *plast = buf;
	int c;

	c = fread_letter(fp);
	if (c == '.')
		c = xgetc(fp);

	for (;;) {
		if (plast >= buf + sizeof(buf) - 1) {
			log(LOG_ERROR, "fread_string: line too long (truncated)");
			buf[sizeof(buf)-1] = '\0';
			return str_dup(buf);
		}

		switch (c) {
		default:
			*plast++ = c;
			break;

		case EOF:
			log(LOG_ERROR, "fread_string: EOF");
			*plast = '\0';
			return str_dup(buf);

		case '~':
			if ((c = xgetc(fp)) == '~') {
				*plast++ = c;
				break;
			}
			xungetc(fp);
			*plast = '\0';
			return str_dup(buf);
		}
		c = xgetc(fp);
	}
}

/*
 * Read a letter from a file.
 */
char
fread_letter(rfile_t *fp)
{
	char c;

	do {
		c = xgetc(fp);
	} while (isspace(c));
	return c;
}

/*
 * Read to end of line (for comments).
 */
void
fread_to_eol(rfile_t *fp)
{
	char c;

	do {
		c = xgetc(fp);
	} while (c != '\n' && c != '\r');

	do {
		c = xgetc(fp);
	} while (c == '\n' || c == '\r');

	xungetc(fp);
}

void
fread_to_end(rfile_t *fp)
{
	for (;;) {
		fread_to_eol(fp);
		fread_keyword(fp);
		if (IS_TOKEN(fp, "End"))
			break;
	}
}

/*
 * Read a number from a file.
 */
int
fread_number(rfile_t *fp)
{
	int number = 0;
	bool sign = FALSE;
	char c = fread_letter(fp);

	if (c == '+')
		c = xgetc(fp);
	else if (c == '-') {
		sign = TRUE;
		c = xgetc(fp);
	}

	if (!isdigit(c)) {
		log(LOG_ERROR, "fread_number: bad format");
		return 0;
	}

	while (isdigit(c)) {
		number = number * 10 + c - '0';
		c      = xgetc(fp);
	}

	if (sign)
		number = 0 - number;

	if (c == '|')
		number += fread_number(fp);
	else
		xungetc(fp);

	return number;
}

static int64_t
flag_convert(int letter, int low_end)
{
	int64_t rv;

	if ('A' <= letter && letter <= 'Z')
		rv = (A) << (letter - 'A');
	else if ('a' <= letter && letter <= low_end)
		rv = (int64_t) (aa) << (letter - 'a');
	else
		rv = 0;

	return rv;
}

static int64_t
fread_flagsxx(rfile_t *fp, int low_end)
{
	int64_t number;
	bool negative = FALSE;
	char c = fread_letter(fp);

	if (c == '-') {
		negative = TRUE;
		c = xgetc(fp);
	}

	number = 0;

	if (!isdigit(c)) {
		while (('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z')) {
			number += flag_convert(c, low_end);
			c = xgetc(fp);
		}
	}

	while (isdigit(c)) {
		number = number * 10 + c - '0';
		c = xgetc(fp);
	}

	if (c == '|')
		number += fread_flagsxx(fp, low_end);
	else if (!isspace(c))
		xungetc(fp);

	return negative ? -number : number;
}

flag_t
fread_flags(rfile_t *fp)
{
	return fread_flagsxx(fp, 'e');
}

int64_t
fread_flags64(rfile_t *fp)
{
	return fread_flagsxx(fp, 'z');
}

/*
 * read flag word (not f-word :)
 */
flag_t
fread_fword(const flaginfo_t *table, rfile_t *fp)
{
	const char *name;

	fread_word(fp);
	name = rfile_tok(fp);

	if (is_number(name))
		return atoi(name);

	return flag_svalue(table, name);
}

flag_t
fread_fstring(const flaginfo_t *table, rfile_t *fp)
{
	const char *s = fread_string(fp);
	flag_t val;

	if (is_number(s))
		val = atoi(s);
	else
		val = flag_svalue(table, s);

	free_string(s);
	return val;
}

/*
 * duplicate '~'
 */
char *
fix_string(const char *s)
{
	static char buf[MAX_STRING_LENGTH * 2];
	char *p = buf;

	if (IS_NULLSTR(s))
		return str_empty;

	if (*s == '.' || isspace(*s))
		*p++ = '.';

	for (; *s && p < buf + sizeof(buf) - 2; s++) {
		switch (*s) {
		case '~':
			*p++ = *s;
			/* FALLTHRU */

		default:
			*p++ = *s;
			break;
		}
	}

	*p = '\0';
	return buf;
}

const char *
fix_word(const char *w)
{
	static char buf[MAX_STRING_LENGTH];

	if (IS_NULLSTR(w))
		return "''";					// notrans

	if (strpbrk(w, " \t") == NULL)				// notrans
		return w;

	snprintf(buf, sizeof(buf), "'%s'", w);			// notrans
	return buf;
}

void
fwrite_string(FILE *fp, const char *name, const char *str)
{
	if (IS_NULLSTR(name))
		fprintf(fp, "%s~\n", fix_string(str));
	else if (!IS_NULLSTR(str))
		fprintf(fp, "%s %s~\n", name, fix_string(str));
}

void
fwrite_word(FILE *fp, const char *name, const char *w)
{
	if (!IS_NULLSTR(w))
		fprintf(fp, "%s %s\n", name, fix_word(w));
}

void
fwrite_number(FILE *fp, const char *name, int num)
{
	if (num)
		fprintf(fp, "%s %d\n", name, num);
}
