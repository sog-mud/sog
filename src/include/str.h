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
 * $Id: str.h,v 1.14 2001-06-16 18:40:09 fjoe Exp $
 */

#ifndef _STR_H_
#define _STR_H_

/*
 * count allocated memory (slows down free_string)
 */
#define STR_ALLOC_MEM 1

#define MAX_STRING_HASH		16384
#define MAX_STRING_LENGTH	8192

const char *	str_dup		(const char *str);
const char *	str_qdup	(const char *str);
void		free_string	(const char *str);
const char *	str_printf	(const char *format,...)
					__attribute__ ((format(printf, 1, 2)));

extern char	str_empty[1];

#define IS_NULLSTR(str)		(!(str) || *(char*)(str) == '\0')

char *	strnzcpy(char *dest, size_t len, const char *src);
#define strnzncpy(dest, len, src, count) \
		strnzcpy((dest), UMIN((len), (count)+1), (src))
char *	strnzcat(char *dest, size_t len, const char *src);
char *	strnzncat(char *dest, size_t len, const char *src, size_t count);
#if !defined (WIN32)
char *	strlwr(const char *s);
#endif

int	str_cmp		(const char *astr, const char *bstr);
int	str_cscmp	(const char *astr, const char *bstr);
int	str_ncmp	(const char *astr, const char *bstr, size_t len);
bool	str_prefix	(const char *astr, const char *bstr);
bool	str_infix	(const char *astr, const char *bstr);
bool	str_suffix	(const char *astr, const char *bstr);

int hashstr(const char *s, int maxn, int hashs);
int hashcasestr(const char *s, int maxn, int hashs);

/**
 * Compare two pointers to strings (case-insensitive)
 *
 * Assumes that arguments are (const char **)
 */
int cmpstr(const void *p1, const void *p2);

/**
 * Compare two pointers to strings (case-sensitive)
 *
 * Assumes that arguments are (const char **)
 */
int cscmpstr(const void *p1, const void *p2);

#endif
