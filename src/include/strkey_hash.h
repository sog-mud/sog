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
 * $Id: strkey_hash.h,v 1.11 1999-12-28 08:58:54 fjoe Exp $
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

int		k_hash_str(const void *k, size_t hsize);
int		ke_cmp_str(const void *k, const void *e);

/*
 * case sensitive hash and compare
 */
int		k_hash_csstr(const void *k, size_t hsize);
int		ke_cmp_csstr(const void *k, const void *e);

/*
 * mlstring compare
 */
int		ke_cmp_mlstr(const void *k, const void *e);
int		ke_cmp_csmlstr(const void *k, const void *e);

void *		strkey_lookup(hash_t *h, const char *name);

/*
 * search elem by name prefix
 */
void *		strkey_search(hash_t *h, const char *name);
void *		mlstrkey_search(hash_t *h, const char *name);

const char *	fread_strkey(rfile_t *fp, hash_t *h, const char *id);
char *		strkey_filename(const char *name, const char *ext);

void *		add_strname_cb(void *p, va_list ap);
void *		add_mlstrname_cb(void *p, va_list ap);

#define	strkey_printall(h, buf)		\
		(hash_printall((h), (buf), add_strname_cb));
#define	mlstrkey_printall(h, buf)	\
		(hash_printall((h), (buf), add_mlstrname_cb))

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
