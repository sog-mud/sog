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
 * $Id: hash.h,v 1.1 1999-10-06 09:55:58 fjoe Exp $
 */

#ifndef _HASH_H_
#define _HASH_H_

typedef int (*hash_k_hash_t)(const void *k, size_t hsize);
typedef int (*hash_ke_cmp_t)(const void *k, const void *e);
typedef void *(*hash_e_cpy_t)(void *dst, const void *src);

typedef struct hash_t hash_t;
struct hash_t {
	size_t hsize;
	varr *v;

	hash_k_hash_t k_hash;	/* hash of key	*/
	hash_ke_cmp_t ke_cmp;	/* cmp key and elem */
	hash_e_cpy_t e_cpy;	/* copy elems */
};

void	hash_init(hash_t*, size_t hsize, size_t nsize,
		  varr_e_init_t e_einit, varr_e_destroy_t e_destroy);
void	hash_destroy(hash_t*);

void *	hash_lookup(hash_t*, const void *k);
void	hash_delete(hash_t*, const void *k);

void *	hash_insert(hash_t*, const void *k, const void *e);
void *	hash_update(hash_t*, const void *k, const void *e);
void *	hash_replace(hash_t*, const void *k, const void *e);

bool	hash_isempty(hash_t*);
void *	hash_random_item(hash_t *h);
void *	hash_foreach(hash_t *h, void *(*cb)(void *p, void *d), void *d);
void	hash_print_names(hash_t *h, BUFFER *buf);

/*
 * generic functions and defines for hashing structs with
 * `const char *name' as first struct member
 */
#define NAME_HASH_SIZE 256

void		name_init(void *);
void		name_destroy(void*);

int		name_hash(const void *k, size_t hsize);
int		name_struct_cmp(const void *k, const void *e);
const char *	fread_name(FILE *fp, hash_t *h, const char *id);

#define NAME_STRICT_CHECKS
#if defined(NAME_STRICT_CHECKS)
#define NAME_CHECK(h, name, id)						\
	do {								\
		if (!IS_NULLSTR(name)					\
		&&  hash_lookup(h, (name)) == NULL) 			\
			wizlog("%s: unknown name '%s'", id, name);	\
	} while (0);
#else
#define NAME_CHECK(h, name, id)
#endif

#endif
