/*-
 * Copyright (c) 2001 SoG Development Team
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
 * $Id: container.h,v 1.8 2003-05-08 14:00:02 fjoe Exp $
 */

#ifndef _CONTAINER_H_
#define _CONTAINER_H_

#define CA_F_NOUPDATE	(A)
#define CA_F_NOINSERT	(B)

struct c_ops_t {
	void (*c_init)(void *c, void *info);
	void (*c_destroy)(void *c);
	void (*c_erase)(void *c);

	void *(*c_lookup)(void *c, const void *k);

	void *(*c_add)(void *c, const void *k, int flags);
	void (*c_delete)(void *c, const void *k);
	void (*c_move)(void *c, const void *k, const void *k_new);

	void *(*c_first)(void *c);
	bool (*c_cond)(void *c, void *elem);
	void *(*c_next)(void *c, void *elem);

	size_t (*c_size)(void *c);
	bool (*c_isempty)(void *c);
	void *(*c_random_elem)(void *c);
};

#define C_OPS(c)		(**(c_ops_t ***) (c))

#define c_init(c, info)		((*(c_ops_t **) (info))->c_init((c), (info)))
#define c_destroy(c)		(C_OPS(c)->c_destroy(c))
#define c_erase(c)		(C_OPS(c)->c_erase(c))

#define c_lookup(c, k)		(C_OPS(c)->c_lookup((c), (k)))

#define c_insert(c, k)		(C_OPS(c)->c_add((c), (k), CA_F_NOUPDATE))
#define c_update(c, k)		(C_OPS(c)->c_add((c), (k),  CA_F_NOINSERT))
#define c_get(c, k)		(C_OPS(c)->c_add((c), (k), 0))

#define c_delete(c, k)		(C_OPS(c)->c_delete((c), (k)))
#define c_move(c, k, k_new)	(C_OPS(c)->c_move((c), (k), (k_new)))

#define c_size(c)		(C_OPS(c)->c_size(c))
#define c_isempty(c)		(C_OPS(c)->c_isempty(c))

#define C_FOREACH(elem, cont)					\
	for ((elem) = C_OPS(cont)->c_first(cont);		\
	     C_OPS(cont)->c_cond((cont), (elem));		\
	     (elem) = C_OPS(cont)->c_next((cont), (elem)))

#define c_random_elem(c)	(C_OPS(c)->c_random_elem(c))
void *	c_random_elem_foreach(void *c);

int	ke_cmp_int(const void *k, const void *e);

void	strkey_init(void *);
void	strkey_destroy(void *);

int	ke_cmp_str(const void *k, const void *e);
int	ke_cmp_csstr(const void *k, const void *e);
int	ke_cmp_mlstr(const void *k, const void *e);
int	ke_cmp_csmlstr(const void *k, const void *e);

void *	c_strkey_lookup(void *c, const char *name);
#define c_mlstrkey_lookup(c, name)	(c_strkey_lookup(c, name))
void *	c_strkey_search(void *c, const char *name);
void *	c_mlstrkey_search(void *c, const char *name);

void	c_strkey_dump(void *c, BUFFER *buf);
void	c_mlstrkey_dump(void *c, BUFFER *buf);

const char *c_fread_strkey(const char *ctx, rfile_t *fp, void *c);
#define fread_strkey(fp, c)						\
	c_fread_strkey(__FUNCTION__, (fp), (c))

char *	strkey_filename(const char *name, const char *ext);

#define C_STRKEY_STRICT_CHECKS
#if defined(C_STRKEY_STRICT_CHECKS)
#define C_STRKEY_CHECK(ctx, c, key)					\
	do {								\
		if (!IS_NULLSTR(key)					\
		&&  c_strkey_lookup((c), (key)) == NULL)		\
			log(LOG_WARN, "%s: unknown string key '%s'",	\
			    (ctx), (key));				\
	} while (0)
#define STRKEY_CHECK(c, key)	C_STRKEY_CHECK(__FUNCTION__, (c), (key))
#else
#define C_STRKEY_CHECK(ctx, c, key)
#define STRKEY_CHECK(c, key)
#endif

#define DEFINE_C_OPS(name)						\
	static void name##_init(void *c, void *info);			\
	static void name##_destroy(void *c);				\
	static void name##_erase(void *c);				\
									\
	static void *name##_lookup(void *c, const void *k);		\
									\
	static void *name##_add(void *c, const void *k, int flags);	\
	static void name##_delete(void *c, const void *k);		\
	static void name##_move(void *c, const void *k, const void *k_new); \
									\
	static void *name##_first(void *c);				\
	static bool name##_cond(void *c, void *elem);			\
	static void *name##_next(void *c, void *elem);			\
									\
	static size_t name##_size(void *c);				\
	static bool name##_isempty(void *c);				\
	static void *name##_random_elem(void *c);			\
									\
	c_ops_t name##_ops = {						\
		name##_init,						\
		name##_destroy,						\
		name##_erase,						\
									\
		name##_lookup,						\
									\
		name##_add,						\
		name##_delete,						\
		name##_move,						\
									\
		name##_first,						\
		name##_cond,						\
		name##_next,						\
									\
		name##_size,						\
		name##_isempty,						\
		name##_random_elem					\
	}

#endif /* _CONTAINER_H_ */
