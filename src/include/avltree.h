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
 * $Id: avltree.h,v 1.1 2001-09-12 12:45:23 fjoe Exp $
 */

#ifndef _AVL_TREE_H_
#define _AVL_TREE_H_

typedef struct avltree_info_t avltree_info_t;
struct avltree_info_t {
	size_t esize;		/**< elem size			*/
	ke_cmp_t ke_cmp;	/**< key vs. elem compare	*/

	e_init_t e_init;	/**< init elem			*/
	e_destroy_t e_destroy;	/**< destroy elem		*/
	e_cpy_t e_cpy;		/**< copy elem			*/
};

typedef struct avlnode_t avlnode_t;
struct avlnode_t {
	avlnode_t *link[2];	/**< left and right subtrees or threads	*/
	int8_t tag[2];		/**< left and right thread tags		*/
	int8_t bal;		/**< balance				*/
	int8_t dir_cache;	/**< comparison cache			*/
};

/*
 * AVL tree
 */
typedef struct avltree_t avltree_t;
struct avltree_t {
	avlnode_t root;		/**< tree root			*/
	avltree_info_t *info;	/**< tree data			*/
	int type_tag;
	int count;		/**< number of elems		*/
};

void avltree_init(avltree_t *avl, avltree_info_t *info, int type_tag);
void avltree_destroy(avltree_t *avl);

void avltree_erase(avltree_t *avl);
void *avltree_lookup(avltree_t *avl, const void *k);
void avltree_delete(avltree_t *avl, const void *k);

void *avltree_insert(avltree_t *avl, const void *k, const void *e);
void *avltree_update(avltree_t *avl, const void *k, const void *e);
void *avltree_replace(avltree_t *avl, const void *k, const void *e);

bool avltree_isempty(avltree_t *avl);
void *avltree_random_item(avltree_t *avl);
void *avltree_foreach(avltree_t *avl, foreach_cb_t cb, ...);

void *strkey_avltree_search(avltree_t *avl, const char *k);

#endif /* _AVL_TREE_H_ */
