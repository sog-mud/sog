/*-
 * Copyright (c) 1999 fjoe <fjoe@iclub.nsu.ru>
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
 * $Id: rwfile.c,v 1.2 1999-10-26 13:52:52 fjoe Exp $
 */

static char str_end[] = "End";

int line_number;

#ifdef USE_MMAP

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "typedef.h"
#include "const.h"
#include "rfile.h"
#include "log.h"
#include "str.h"

static int
count_lf(const char *p, off_t len)
{
	int count = 0;

	while (len--)
	for (;;) {
		char *q = memchr(p, '\n', len);
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
		log("%s: %s", name, strerror(errno));
		return NULL;
	}

	if (fstat(fd, &s) < 0) {
		close(fd);
		log("%s: %s", name, strerror(errno));
		return NULL;
	}

	if ((p = mmap(NULL, s.st_size, PROT_READ, 0, fd, 0)) == MAP_FAILED) {
		close(fd);
		log("%s: %s", name, strerror(errno));
		return NULL;
	}

	if (madvise(p, s.st_size, MADV_SEQUENTIAL) < 0)
		log("%s: %s", name, strerror(errno));
		
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
	munmap((void *) fp->p, fp->len);
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
	strnzncpy(buf, sizeof(buf), fp->tok, fp->tok_len);
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
		p = memchr(fp->tok, cEnd, fp->len - fp->pos);

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
		/* XXX db_warning */
		log("fread_word: word too long, truncated");
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

#else /* USE_MMAP */

const char _token[MAX_STRING_LENGTH];

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
			return _token;
		}
	}

	db_error("fread_word", "word too long");
	return NULL;
}

void
fread_keyword(rfile_t *fp)
{
	if (rfile_feof(fp))
		strnzcpy(_token, sizeof(_token), str_end);
	else
		fread_word(fp);
}

#endif /* USE_MMAP */

const char *
fread_sword(rfile_t *fp)
{
	fread_word(fp);
	return str_dup(rfile_tok(fp));
}

const char *fread_string(rfile_t *fp)
{
	char buf[MAX_STRING_LENGTH];
	char *plast = buf;
	int c = fread_letter(fp);

	for (;;) {
		/*
		 * Back off the char type lookup,
		 *   it was too dirty for portability.
		 *   -- Furey
		 */

		if (plast - buf >= sizeof(buf) - 1) {
			bug("fread_string: line too long (truncated)", 0);
			buf[sizeof(buf)-1] = '\0';
			return str_dup(buf);
		}

		switch (c) {
		default:
			*plast++ = c;
			break;
 
		case EOF:
			db_error("fread_string", "EOF");
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
char fread_letter(rfile_t *fp)
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
void fread_to_eol(rfile_t *fp)
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

/*
 * Read a number from a file.
 */
int fread_number(rfile_t *fp)
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
		db_error("fread_number", "bad format");
		log("fread_number: bad format");
		exit(1);
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

flag64_t fread_flags(rfile_t *fp)
{
	flag64_t number;
	bool negative = FALSE;
	char c = fread_letter(fp);

	if (c == '-') {
		negative = TRUE;
		c = xgetc(fp);
	}

	number = 0;

	if (!isdigit(c)) {
		while (('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z')) {
			number += flag_convert(c);
			c = xgetc(fp);
		}
	}

	while (isdigit(c)) {
		number = number * 10 + c - '0';
		c = xgetc(fp);
	}

	if (c == '|')
		number += fread_flags(fp);
	else if (c != ' ')
		xungetc(fp);

	if (negative)
		return -number;

	return number;
}

/*
 * read flag word (not f-word :)
 */
flag64_t fread_fword(const flag_t *table, rfile_t *fp)
{
	const char *name;

	fread_word(fp);
	name = rfile_tok(fp);

	if (is_number(name))
		return atoi(name);

	return flag_value(table, name);
}

flag64_t fread_fstring(const flag_t *table, rfile_t *fp)
{
	const char *s = fread_string(fp);
	flag64_t val;

	if (is_number(s))
		val = atoi(s);
	else
		val = flag_value(table, s);

	free_string(s);
	return val;
}

