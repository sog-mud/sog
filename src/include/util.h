/*-
 * Copyright (c) 1998 SoG Development Team
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
 * $Id: util.h,v 1.26 2001-09-16 20:04:14 fjoe Exp $
 */

#ifndef _UTIL_H_
#define _UTIL_H_

#if defined(WIN32)
#	include <stdlib.h>
#	define PATH_MAX	_MAX_PATH
#	define PATH_SEPARATOR '\\'
#else
#	define PATH_SEPARATOR '/'
#endif

struct stat;

FILE *	dfopen	(const char *dir, const char *file, const char *mode);
int	dunlink	(const char *dir, const char *file);
int	dstat	(const char *dir, const char *file, struct stat *s);
int	d2rename(const char *dir1, const char *file1,
		 const char *dir2, const char *file2);
bool	dfexist	(const char *dir, const char *file);

const char *	get_filename(const char*);

int cmpint(const void *p1, const void *p2);

size_t		cstrlen		(const char *cstr);
const char *	cstrfirst	(const char *cstr);
void		cstrtoupper	(char *cstr);

char *		capitalize	(const char *str);
char *		format_flags	(flag_t flags);
const char *	strdump		(const char *argument, int dump_level);

uint	number_argument (const char *argument, char *arg, size_t len);
uint	mult_argument	(const char *argument, char *arg, size_t len);
const char *	one_argument	(const char *argument, char *arg_first, size_t);
const char *	first_arg	(const char *argument, char *arg_first, size_t,
				 bool fCase);

bool		is_number	(const char *argument);

bool	_is_name		(const char *str, const char *namelist,
				 int (*cmpfun)(const char*, const char*));
#define is_name(s, nl)		(_is_name((s), (nl), str_prefix))
#define is_name_strict(s, nl)	(_is_name((s), (nl), str_cmp))

char *		strtime		(time_t);

int		interpolate	(int level, int value_00, int value_32);

int		number_fuzzy	(int number);
int		number_range	(int from, int to);
int		number_percent	(void);
int		number_door	(void);
int		number_bits	(int width);
int		dice		(int number, int size);
int		dice_wlb	(int number, int size,
				 CHAR_DATA *ch, CHAR_DATA *victim);

void *	bsearch_lower(const void *key, const void *base,
		      size_t nmemb, size_t size,
		      int (*cmpfun)(const void *, const void *));

#endif
