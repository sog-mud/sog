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
 * $Id: avltree.c,v 1.13 2003-04-17 11:25:59 tatyana Exp $
 */

#include <assert.h>
#include <stdio.h>

#include <typedef.h>
#include <container.h>
#include <avltree.h>
#include <memalloc.h>

#define GET_DATA(an)	(CAST(void *, (((const char *) (an)) + sizeof(avlnode_t) + sizeof(memchunk_t))))
#define GET_AVLNODE(p)	(CAST(avlnode_t *, (((const char *) (p)) - sizeof(avlnode_t) - sizeof(memchunk_t))))

#define DIR_LEFT	0
#define DIR_RIGHT	1

#define LEFT(node)	(node->link[DIR_LEFT])
#define RIGHT(node)	(node->link[DIR_RIGHT])

#define LTAG(node)	(node->tag[DIR_LEFT])
#define RTAG(node)	(node->tag[DIR_RIGHT])

#define TAG_TREE	0	/* link is subtree	*/
#define TAG_THREAD	1	/* link is thread	*/

static void avlnode_init(avlnode_t *node,
			 avlnode_t *left, int left_tag,
			 avlnode_t *right, int right_tag);
static avlnode_t *avlnode_new(avltree_t *avl, avlnode_t *node,
			      avlnode_t *left, int left_tag,
			      avlnode_t *right, int right_tag);
static void avlnode_free(avltree_t *avl, avlnode_t *node);

static avlnode_t *avlnode_add(void *c, const void *key, int flags,
			      avlnode_t *node);
static avlnode_t *avlnode_delete(void *c, const void *key, bool free_node);

#define AVL_MAX_HEIGHT 32

/*--------------------------------------------------------------------
 * container ops
 */

DEFINE_C_OPS(avltree);

static void
avltree_init(void *c, void *info)
{
	avltree_t *avl = (avltree_t *) c;

	avl->info = info;
	avlnode_init(&avl->root, NULL, TAG_TREE, &avl->root, TAG_TREE);
	avl->count = 0;
}

static void
avltree_destroy(void *c)
{
	avltree_t *avl = (avltree_t *) c;
	avlnode_t *curr = avl->root.link[0];

	avlnode_t *an[AVL_MAX_HEIGHT];
	char ab[AVL_MAX_HEIGHT];
	int ap = 0;

	if (curr == &avl->root)
		return;

	for (;;) {
		while (curr != NULL) {
			ab[ap] = 0;
			an[ap++] = curr;
			curr = LEFT(curr);
		}

		for (; ;) {
			if (ap == 0)
				goto done;

			curr = an[--ap];

			if (ab[ap] == 0) {
				ab[ap++] = 1;
				if (RTAG(curr) == TAG_THREAD)
					continue;
				curr = RIGHT(curr);
				break;
			}

			avlnode_free(avl, curr);
		}
	}

done:
	;
}

static void
avltree_erase(void *c)
{
	avltree_t *avl = (avltree_t *) c;

	avltree_destroy(avl);

	avlnode_init(&avl->root, NULL, TAG_TREE, &avl->root, TAG_TREE);
	avl->count = 0;
}

static void *
avltree_lookup(void *c, const void *k)
{
	avltree_t *avl = (avltree_t *) c;
	avlnode_t *curr;

	curr = avl->root.link[0];
	if (curr == NULL)
		return NULL;

	for (; ;) {
		void *e = GET_DATA(curr);

		int diff = avl->info->ke_cmp(k, e);
		if (diff < 0) {
			curr = LEFT(curr);
			if (curr == NULL)
				return NULL;
		} else if (diff > 0) {
			if (RTAG(curr) == TAG_THREAD)
				return NULL;
			curr = RIGHT(curr);
		} else
			return e;
	}

	return NULL;
}

static void *
avltree_add(void *c, const void *k, int flags)
{
	return avlnode_add(c, k, flags, NULL);
}

static void
avltree_delete(void *c, const void *key)
{
	avlnode_delete(c, key, TRUE);
}

static void
avltree_move(void *c, const void *k, const void *k_new)
{
	avlnode_t *node;

	if ((node = avlnode_delete(c, k, FALSE)) == NULL)
		return;

	if (avlnode_add(c, k_new, CA_F_NOUPDATE, node) == NULL)
		avlnode_free(c, node);
}

static void *
avltree_node_next(avltree_t *avl, avlnode_t *curr)
{
	int rtag = RTAG(curr);

	curr = RIGHT(curr);
	if (rtag == TAG_TREE) {
		while (LEFT(curr) != NULL)
			curr = LEFT(curr);
	}

	if (curr == &avl->root)
		return NULL;

	return GET_DATA(curr);
}

static void *
avltree_first(void *c)
{
	avltree_t *avl = (avltree_t *) c;

	return avltree_node_next(avl, &avl->root);
}

static bool
avltree_cond(void *c, void *elem)
{
	UNUSED_ARG(c);

	return elem != NULL;
}

static void *
avltree_next(void *c, void *elem)
{
	avltree_t *avl = (avltree_t *) c;

	return avltree_node_next(avl, GET_AVLNODE(elem));
}

static size_t
avltree_size(void *c)
{
	avltree_t *avl = (avltree_t *) c;
	return avl->count;
}

static bool
avltree_isempty(void *c)
{
	avltree_t *avl = (avltree_t *) c;
	return avl->count == 0;
}

static void *
avltree_random_elem(void *c)
{
	return c_random_elem_foreach(c);
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
avlnode_new(avltree_t *avl, avlnode_t *node,
	    avlnode_t *left, int left_tag, avlnode_t *right, int right_tag)
{
	if (node == NULL) {
		void *p = mem_alloc2(
		    avl->info->type_tag, avl->info->esize, sizeof(avlnode_t));
		if (avl->info->e_init != NULL)
			avl->info->e_init(p);

		node = GET_AVLNODE(p);
	}

	avl->count++;
	avlnode_init(node, left, left_tag, right, right_tag);

	return node;
}

static void
avlnode_free(avltree_t *avl, avlnode_t *node)
{
	void *p = GET_DATA(node);
	if (avl->info->e_destroy != NULL)
		avl->info->e_destroy(p);
	mem_free(p);
}

static avlnode_t *
avlnode_add(void *c, const void *k, int flags, avlnode_t *node)
{
	avltree_t *avl = (avltree_t *) c;
	avlnode_t *curr, *next;
	avlnode_t *r, *s, *t;

	t = &avl->root;
	s = curr = LEFT(t);

	if (s == NULL) {
		/*
		 * tree is empty
		 */

		if (IS_SET(flags, CA_F_NOINSERT))
			return NULL;

		next = LEFT(t) = avlnode_new(
		    avl, node, NULL, TAG_TREE, t, TAG_THREAD);
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

				if (IS_SET(flags, CA_F_NOINSERT))
					return NULL;

				next = avlnode_new(
				    avl, node,
				    NULL, TAG_TREE, curr, TAG_THREAD);
				LEFT(curr) = next;
				break;
			}
		} else if (diff > 0) {
			curr->dir_cache = DIR_RIGHT;

			next = RIGHT(curr);
			if (RTAG(curr) == TAG_THREAD) {
				/*
				 * insert to the right
				 */

				if (IS_SET(flags, CA_F_NOINSERT))
					return NULL;

				next = avlnode_new(
				    avl, node,
				    NULL, TAG_TREE, RIGHT(curr), RTAG(curr));
				RIGHT(curr) = next;
				RTAG(curr) = TAG_TREE;
				break;
			}

			assert(next != NULL);
		} else {
			void *elem;

			/*
			 * found it
			 */
			if (IS_SET(flags, CA_F_NOUPDATE))
				return NULL;

			elem = GET_DATA(curr);
			if (avl->info->e_destroy != NULL)
				avl->info->e_destroy(elem);
			if (avl->info->e_init != NULL)
				avl->info->e_init(elem);
			return elem;
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
		} else {
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

static avlnode_t *
avlnode_delete(void *c, const void *key, bool free_node)
{
	avltree_t *avl = (avltree_t *) c;

	avlnode_t *pa[AVL_MAX_HEIGHT];		/* Stack P: Nodes. */
	unsigned char a[AVL_MAX_HEIGHT];	/* Stack P: Bits. */
	int k = 1;				/* Stack P: Pointer. */

	avlnode_t *curr, *rv;
	avlnode_t **q;

	curr = avl->root.link[0];
	if (curr == NULL)
		return NULL;

	a[0] = 0;
	pa[0] = &avl->root;

	/*
	 * find elem
	 */
	for (;;) {
		int diff = avl->info->ke_cmp(key, GET_DATA(curr));

		if (diff == 0)
			break;

		pa[k] = curr;
		if (diff < 0) {
			if (LEFT(curr) != NULL) {
				curr = LEFT(curr);
				a[k] = 0;
			} else
				return NULL;
		} else if (diff > 0) {
			if (RTAG(curr) == TAG_TREE) {
				curr = RIGHT(curr);
				a[k] = 1;
			} else
				return NULL;
		}
		k++;
	}

	q = &pa[k - 1]->link[a[k - 1]];

	/*
	 * disconnect from the tree
	 */
	if (RTAG(curr) == TAG_THREAD) {
		if (LEFT(curr) != NULL) {
			avlnode_t *const x = LEFT(curr);

			*q = x;
			(*q)->bal = 0;
			if (RTAG(x) == TAG_THREAD) {
				if (a[k - 1] == 1)
					RIGHT(x) = RIGHT(curr);
				else
					RIGHT(x) = pa[k - 1];
			}
		} else {
			*q = curr->link[a[k - 1]];
			if (a[k - 1] == 0)
				pa[k - 1]->link[0] = NULL;
			else
				pa[k - 1]->tag[1] = TAG_THREAD;
		}
	} else {
		avlnode_t *r = RIGHT(curr);

		if (LEFT(r) == NULL) {
			LEFT(r) = LEFT(curr);
			r->bal = curr->bal;

			if (LEFT(r) != NULL) {
				avlnode_t *s = LEFT(r);

				while (RTAG(s) == TAG_TREE)
					s = RIGHT(s);

				assert (RTAG(s) == TAG_THREAD);
				RIGHT(s) = r;
			}

			*q = r;
			a[k] = 1;
			pa[k++] = r;
		} else {
			avlnode_t *s = LEFT(r);
			avlnode_t **qq;
			avlnode_t *t = r;

			a[k] = 1;
			qq = &pa[k++];

			a[k] = 0;
			pa[k++] = r;

			while (LEFT(s) != NULL) {
				r = s;
				s = LEFT(r);
				a[k] = 0;
				pa[k++] = r;
			}

			*qq = *q = s;
			if (RTAG(s) == TAG_TREE)
				LEFT(r) = RIGHT(s);
			else
				LEFT(r) = NULL;

			LEFT(s) = LEFT(curr);
			LTAG(s) = LTAG(curr);

			RIGHT(s) = t;
			RTAG(s) = TAG_TREE;

			s->bal = curr->bal;

			if ((r = LEFT(s)) != NULL) {
				while (RTAG(r) != TAG_THREAD)
					r = RIGHT(r);

				assert (RIGHT(r) == curr);
				RIGHT(r) = s;
			}
		}
	}

	/*
	 * call e_destroy() and free memory
	 */
	avl->count--;
	rv = curr;
	if (free_node)
		avlnode_free(avl, curr);

	/*
	 * fixup balance
	 */
	assert (k > 0);

	while (--k) {
		avlnode_t *const s = pa[k];

		if (a[k] == 0) {
			avlnode_t *const r = RIGHT(s);

			/* D10. */
			if (s->bal == -1) {
				s->bal = 0;
				continue;
			} else if (s->bal == 0) {
				s->bal = +1;
				break;
			}

			assert (s->bal == +1);

			if (RTAG(s) == TAG_THREAD || r->bal == 0) {
				RIGHT(s) = LEFT(r);
				LEFT(r) = s;
				r->bal = -1;
				pa[k - 1]->link[a[k - 1]] = r;
				break;
			} else if (r->bal == +1) {
				if (LEFT(r) != NULL) {
					RTAG(s) = TAG_TREE;
					RIGHT(s) = LEFT(r);
				} else
					RTAG(s) = TAG_THREAD;

				LEFT(r) = s;
				s->bal = r->bal = 0;
				pa[k - 1]->link[a[k - 1]] = r;
			} else {
				assert (r->bal == -1);

				curr = LEFT(r);
				if (RTAG(curr) == TAG_TREE)
					LEFT(r) = RIGHT(curr);
				else
					LEFT(r) = NULL;
				RIGHT(curr) = r;
				RTAG(curr) = TAG_TREE;
				if (LEFT(curr) == NULL) {
					RIGHT(s) = curr;
					RTAG(s) = TAG_THREAD;
				} else {
					RIGHT(s) = LEFT(curr);
					RTAG(s) = TAG_TREE;
				}

				LEFT(curr) = s;
				if (curr->bal == +1) {
					s->bal = -1;
					r->bal = 0;
				} else if (curr->bal == 0)
					s->bal = r->bal = 0;
				else {
					assert (curr->bal == -1);
					s->bal = 0;
					r->bal = +1;
				}

				curr->bal = 0;
				pa[k - 1]->link[a[k - 1]] = curr;
				if (a[k - 1] == 1)
					pa[k - 1]->tag[1] = TAG_TREE;
			}
		} else {
			avlnode_t *const r = LEFT(s);

			if (s->bal == +1) {
				s->bal = 0;
				continue;
			} else if (s->bal == 0) {
				s->bal = -1;
				break;
			}

			assert (s->bal == -1);
			if (LEFT(s) == NULL || r->bal == 0) {
				LEFT(s) = RIGHT(r);
				RIGHT(r) = s;
				r->bal = +1;
				pa[k - 1]->link[a[k - 1]] = r;
				break;
			} else if (r->bal == -1) {
				if (RTAG(r) == TAG_TREE)
					LEFT(s) = RIGHT(r);
				else
					LEFT(s) = NULL;
				RIGHT(r) = s;
				RTAG(r) = TAG_TREE;
				s->bal = r->bal = 0;
				pa[k - 1]->link[a[k - 1]] = r;
			} else {
				assert (r->bal == +1);

				curr = RIGHT(r);
				if (LEFT(curr) != NULL) {
					RTAG(r) = TAG_TREE;
					RIGHT(r) = LEFT(curr);
				} else
					RTAG(r) = TAG_THREAD;

				LEFT(curr) = r;
				if (RTAG(curr) == TAG_THREAD)
					LEFT(s) = NULL;
				else
					LEFT(s) = RIGHT(curr);
				RIGHT(curr) = s;
				RTAG(curr) = TAG_TREE;

				if (curr->bal == -1) {
					s->bal = +1;
					r->bal = 0;
				} else if (curr->bal == 0)
					s->bal = r->bal = 0;
				else {
					assert (curr->bal == +1);
					s->bal = 0, r->bal = -1;
				}

				curr->bal = 0;
				if (a[k - 1] == 1)
					pa[k - 1]->tag[1] = TAG_TREE;
				pa[k - 1]->link[a[k - 1]] = curr;
			}
		}
	}

	return rv;
}
