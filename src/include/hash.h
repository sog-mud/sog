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
 * $Id: hash.h,v 1.12 2001-09-12 19:42:34 fjoe Exp $
 */

#ifndef _HASH_H_
#define _HASH_H_

extern c_ops_t hash_ops;

/*
 * first five elems must be the same as in varrdata_t
 */
typedef struct hashdata_t {
	c_ops_t *ops;			/**< container ops		*/

	size_t nsize;			/**< size of elem		*/
	size_t nstep;			/**< allocation step		*/

	e_init_t e_init;		/**< init elem			*/
	e_destroy_t e_destroy;		/**< destroy elem		*/
	e_cpy_t e_cpy;			/**< copy elem			*/

	size_t hsize;
	k_hash_t k_hash;		/**< hash of key		*/
	ke_cmp_t ke_cmp;		/**< cmp key and elem		*/
} hashdata_t;

typedef struct hash_t hash_t;
struct hash_t {
	hashdata_t *h_data;
	varr *v;
};

void	hash_init(void *h, void *info);
void	hash_destroy(void *h);

void	hash_printall(hash_t *h, BUFFER *buf, foreach_cb_t addname_cb);

int	vnum_hash(const void *k, size_t hsize);
int	vnum_ke_cmp(const void *k, const void *e);

#endif
