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
 * $Id: avltree.c,v 1.1 2001-09-12 12:45:27 fjoe Exp $
 */

#include <assert.h>
#include <stdio.h>

#include <typedef.h>
#include <buffer.h>
#include <avltree.h>
#include <memalloc.h>
#include <varr.h>
#include <util.h>

#define GET_DATA(an) ((void *)(uintptr_t) (((const char *) an) + sizeof(avlnode_t) + sizeof(memchunk_t)))
#define GET_AN(p) ((str *)(uintptr_t) (((const char *) p) - sizeof(avlnode_t) - sizeof(memchunk_t)))

#define DIR_LEFT	0
#define DIR_RIGHT	1

#define LEFT(node)		(node->link[DIR_LEFT])
#define RIGHT(node)	(node->link[DIR_RIGHT])

#define LTAG(node)		(node->tag[DIR_LEFT])
#define RTAG(node)		(node->tag[DIR_RIGHT])

#define TAG_TREE	0	/* link is subtree	*/
#define TAG_THREAD	1	/* link is thread	*/

static void avlnode_init(avlnode_t *node,
			 avlnode_t *left, int left_tag,
			 avlnode_t *right, int right_tag);
static avlnode_t *avlnode_new(avltree_t *avl,
			      avlnode_t *left, int left_tag,
			      avlnode_t *right, int right_tag,
			      const void *e);
static void avlnode_copy_data(avltree_t *avl, avlnode_t *node, const void *e);

/*
 * avltree_add flags
 */
#define ATA_F_INSERT	(A)	/* insert element if does not exist */
#define ATA_F_UPDATE	(B)	/* update element if exists */

static void *avltree_add(avltree_t *avl, const void *k, const void *e,
			 int flags);

void
avltree_init(avltree_t *avl, avltree_info_t *info, int type_tag)
{
	avlnode_init(&avl->root, NULL, TAG_TREE, &avl->root, TAG_THREAD);

	avl->info = info;
	avl->type_tag = type_tag;
	avl->count = 0;
}

void
avltree_destroy(avltree_t *avl)
{
	/* XXX */
	UNUSED_ARG(avl);
}

void
avltree_erase(avltree_t *avl)
{
	avltree_destroy(avl);
	avl->count = 0;
}

void *
avltree_lookup(avltree_t *avl, const void *k)
{
	avlnode_t *curr;

	curr = avl->root.link[0];
	if (curr == NULL)
		return NULL;

	for (; ;) {
		int diff = avl->info->ke_cmp(k, GET_DATA(curr));
		if (diff < 0) {
			curr = LEFT(curr);
			if (curr == NULL)
				return NULL;
		} else if (diff > 0) {
			if (RTAG(curr) == TAG_THREAD)
				return NULL;
			curr = RIGHT(curr);
		} else
			return GET_DATA(curr);
	}

	return NULL;
}

void
avltree_delete(avltree_t *avl, const void *k)
{
	/* XXX */
	UNUSED_ARG(avl);
	UNUSED_ARG(k);
}

void *
avltree_insert(avltree_t *avl, const void *k, const void *e)
{
	return avltree_add(avl, k, e, ATA_F_INSERT);
}

void *
avltree_update(avltree_t *avl, const void *k, const void *e)
{
	return avltree_add(avl, k, e, ATA_F_UPDATE);
}

void *
avltree_replace(avltree_t *avl, const void *k, const void *e)
{
	return avltree_add(avl, k, e, ATA_F_INSERT | ATA_F_UPDATE);
}

bool
avltree_isempty(avltree_t *avl)
{
	return avl->count != NULL;
}

static
FOREACH_CB_FUN(get_nth_elem_cb, p, ap)
{
	int *pnum = va_arg(ap, int *);

	if (!--*pnum)
		return GET_DATA(p);

	return NULL;
}

void *
avltree_random_item(avltree_t *avl)
{
	int num;

	if (avltree_isempty(avl))
		return NULL;

	num = number_range(1, avl->count);
	return avltree_foreach(avl, get_nth_elem_cb, &num);
}

void *
avltree_foreach(avltree_t *avl, foreach_cb_t cb, ...)
{
	/* XXX */
	UNUSED_ARG(avl);
	UNUSED_ARG(cb);
	return NULL;
}

void *
strkey_avltree_search(avltree_t *avl, const char *name)
{
	void *p;

	if (avltree_isempty(avl))
		return NULL;

	if ((p = avltree_lookup(avl, name)) != NULL)
		return p;

	return avltree_foreach(avl, vstr_search_cb, name);
}

/*--------------------------------------------------------------------
 * static functions
 */

static void
avlnode_init(avlnode_t *node,
	      avlnode_t *left, int left_tag, avlnode_t *right, int right_tag)
{
	LEFT(node) = left;
	LTAG(node) = left_tag;
	RIGHT(node) = right;
	RTAG(node) = right_tag;

	node->bal = 0;
	node->dir_cache = 0;
}

static avlnode_t *
avlnode_new(avltree_t *avl,
	     avlnode_t *left, int left_tag, avlnode_t *right, int right_tag,
	     const void *e)
{
	avlnode_t *node;

	node = mem_alloc2(avl->type_tag, avl->info->esize, sizeof(avlnode_t));
	avlnode_init(node, left, left_tag, right, right_tag);

	avl->count++;
	if (avl->info->e_init != NULL)
		avl->info->e_init(GET_DATA(node));

	if (e != NULL)
		avlnode_copy_data(avl, node, e);

	return node;
}

static void
avlnode_copy_data(avltree_t *avl, avlnode_t *node, const void *e)
{
	if (avl->info->e_cpy != NULL)
		avl->info->e_cpy(GET_DATA(node), e);
	else
		memcpy(GET_DATA(node), e, avl->info->esize);
}

static void *
avltree_add(avltree_t *avl, const void *k, const void *e, int flags)
{
	avlnode_t *curr, *next;
	avlnode_t *r, *s, *t;

	t = &avl->root;
	s = curr = LEFT(t);

	if (s == NULL) {
		/*
		 * tree is empty
		 */

		if (!IS_SET(flags, ATA_F_INSERT))
			return NULL;

		next = LEFT(t) = avlnode_new(
		    avl, NULL, TAG_TREE, t, TAG_THREAD, e);
		assert(avl->count == 1);
		return GET_DATA(next);
	}

	for (; ;) {
		int diff = avl->info->ke_cmp(k, GET_DATA(curr));

		if (diff < 0) {
			curr->dir_cache = DIR_LEFT;

			if ((next = LEFT(curr)) == NULL) {
				/*
				 * insert to the left
				 */

				next = avlnode_new(
				    avl, NULL, TAG_TREE, curr, TAG_THREAD, e);
				LEFT(curr) = next;
				break;
			}
		} else if (diff > 0) {
			curr->dir_cache = DIR_RIGHT;

			next = RIGHT(curr);
			if ((RTAG(curr)) == TAG_THREAD) {
				/*
				 * insert to the right
				 */

				next = avlnode_new(
				    avl, NULL, TAG_TREE,
				    RIGHT(curr), RTAG(curr), e);
				RIGHT(curr) = next;
				RTAG(curr) = TAG_TREE;
				break;
			}

			assert(next != NULL);
		} else {
			/*
			 * found it
			 */
			if (!IS_SET(flags, ATA_F_UPDATE))
				return NULL;

			avlnode_copy_data(avl, curr, e);
			return GET_DATA(curr);
		}

		if (next->bal != 0) {
			t = curr;
			s = next;
		}

		curr = next;
	}

	/*
	 * fixup balance
	 */

	r = curr = s->link[s->dir_cache];
	while (curr != next) {
		curr->bal = curr->dir_cache * 2 - 1;
		curr = curr->link[curr->dir_cache];
	}

	if (s->dir_cache == DIR_LEFT) {
		if (s->bal == 0 || s->bal == 1) {
			s->bal -= 1;
			return GET_DATA(next);
		}

		assert(s->bal == -1);
		if (r->bal == -1) {
			curr = r;
			if (RTAG(r) == TAG_THREAD) {
				LEFT(s) = NULL;
				RIGHT(r) = s;
				RTAG(r) = TAG_TREE;
			} else {
				LEFT(s) = RIGHT(r);
				RIGHT(r) = s;
			}

			s->bal = r->bal = 0;
		} else{
			assert(r->bal == 1);

			curr = RIGHT(r);
			RIGHT(r) = LEFT(curr);
			LEFT(curr) = r;
			LEFT(s) = RIGHT(curr);
			RIGHT(curr) = s;

			if (curr->bal == -1) {
				s->bal = 1;
				r->bal = 0;
			} else if (curr->bal == 0)
				s->bal = r->bal = 0;
			else {
				assert(curr->bal == 1);

				s->bal = 0;
				r->bal = -1;
			}

			curr->bal = 0;
			RTAG(curr) = TAG_TREE;
			if (LEFT(s) == s)
				LEFT(s) = NULL;
			if (RIGHT(r) == NULL) {
				RIGHT(r) = curr;
				RTAG(r) = TAG_THREAD;
			}
		}
	} else {
		if (s->bal == -1 || s->bal == 0) {
			s->bal += 1;
			return GET_DATA(next);
		}

		assert(s->bal == 1);
		if (r->bal == 1) {
			curr = r;
			if (LEFT(r) == NULL) {
				RTAG(s) = TAG_THREAD;
				LEFT(r) = s;
			} else {
				RIGHT(s) = LEFT(r);
				RTAG(s) = TAG_TREE;
				LEFT(r) = s;
			}

			s->bal = r->bal = 0;
		} else {
			assert(r->bal == -1);

			curr = LEFT(r);
			LEFT(r) = RIGHT(curr);
			RIGHT(curr) = r;
			RIGHT(s) = LEFT(curr);
			LEFT(curr) = s;

			if (curr->bal == 1) {
				s->bal = -1;
				r->bal = 0;
			} else if (curr->bal == 0) {
				s->bal = r->bal = 0;
			} else {
				assert(curr->bal == -1);
				s->bal = 0;
				r->bal = 1;
			}

			RTAG(curr) = TAG_TREE;
			if (RIGHT(s) == NULL) {
				RIGHT(s) = curr;
				RTAG(s) = TAG_THREAD;
			}
			if (LEFT(r) == r)
				LEFT(r) = NULL;
			curr->bal = 0;
		}
	}

	if (t != &avl->root && s == RIGHT(t))
		RIGHT(t) = curr;
	else
		LEFT(t) = curr;

	return GET_DATA(next);
}
