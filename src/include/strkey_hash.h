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
 * $Id: strkey_hash.h,v 1.4 1999-11-22 14:54:24 fjoe Exp $
 */

#ifndef _STRKEY_HASH_H_
#define _STRKEY_HASH_H_

/*
 * generic functions and defines for hashing structs with
 * `const char *name' as first struct member
 */
#define STRKEY_HASH_SIZE 256

void		strkey_init(void *);
void		strkey_destroy(void*);

int		strkey_hash(const void *k, size_t hsize);
int		strkey_struct_cmp(const void *k, const void *e);

void *		strkey_lookup(hash_t *h, const char *name);

/*
 * search elem by name prefix
 */
void *		strkey_search_cb(void *p, va_list ap);
void *		strkey_search(hash_t *h, const char *name);

const char *	fread_strkey(rfile_t *fp, hash_t *h, const char *id);
void		strkey_printall(hash_t *h, BUFFER *buf);
char *		strkey_filename(const char *name);

#define STRKEY_STRICT_CHECKS
#if defined(STRKEY_STRICT_CHECKS)
#define STRKEY_CHECK(h, key, id)					\
	do {								\
		if (!IS_NULLSTR(key)					\
		&&  strkey_lookup(h, (key)) == NULL) 			\
			wizlog("%s: unknown string key '%s'", id, key);	\
	} while (0);
#else
#define STRKEY_CHECK(h, key, id)
#endif

#endif
