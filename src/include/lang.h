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
 * $Id: lang.h,v 1.7 1999-02-16 20:26:04 fjoe Exp $
 */

#ifndef _LANG_H_
#define _LANG_H_

struct lang_data {
	const char *	name;
	int		vnum;
	const char *	file_name;

	flag32_t	flags;
	int		slang_of;		/* id of base lang */

	const char *	file_genders;
	const char *	file_cases;
	const char *	file_qtys;

	varr	hash_genders[MAX_WORD_HASH];
	varr 	hash_cases[MAX_WORD_HASH];
	varr 	hash_qtys[MAX_WORD_HASH];
};

#define LANG_HIDDEN		(A)
#define LANG_GENDERS_CHANGED	(X)
#define LANG_CASES_CHANGED	(Y)
#define LANG_CHANGED		(Z)

extern varr	langs;

LANG_DATA *	lang_new(void);
int		lang_lookup(const char*);
int		lang_nlookup(const char*, size_t len);

#endif
