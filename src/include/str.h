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
 * $Id: str.h,v 1.8.2.1 1999-12-16 12:39:52 fjoe Exp $
 */

#ifndef _STR_H_
#define _STR_H_

/*
 * count allocated memory (slows down free_string)
 */
#define STR_ALLOC_MEM 1

const char *	str_dup		(const char *str);
const char *	str_qdup	(const char *str);
void		free_string	(const char *str);
const char *	str_printf	(const char *format,...);

extern char	str_empty[1];
	
char *	strnzcpy(char *dest, size_t len, const char *src);
#define strnzncpy(dest, len, src, count) \
		strnzcpy((dest), UMIN((len), (count)+1), (src))
char *	strnzcat(char *dest, size_t len, const char *src);
char *	strnzncat(char *dest, size_t len, const char *src, size_t count);
#if !defined (WIN32)
char *	strlwr(const char *s);
#endif

int	str_cmp		(const char *astr, const char *bstr);
int	str_ncmp	(const char *astr, const char *bstr, size_t len);
bool	str_prefix	(const char *astr, const char *bstr);
bool	str_infix	(const char *astr, const char *bstr);
bool	str_suffix	(const char *astr, const char *bstr);

int hashstr(const char *s, int maxn, int hashs);
int hashistr(const char *s, int maxn, int hashs);

int cmpstr(const void*, const void*);

#endif
