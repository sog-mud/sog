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
 * $Id: material.c,v 1.10 2001-09-13 16:22:22 fjoe Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include <merc.h>

avltree_t materials;

static void
material_init(material_t *mat)
{
	mat->name = str_empty;
	mat->float_time = 0;
	mat->dam_class = DAM_NONE;
	mat->mat_flags = 0;
}

static void
material_destroy(material_t *mat)
{
	free_string(mat->name);
}

avltree_info_t c_info_materials =
{
	&avltree_ops,

	(e_init_t) material_init,
	(e_destroy_t) material_destroy,

	MT_PVOID, sizeof(material_t), ke_cmp_str
};

bool
material_is(OBJ_DATA *obj, flag_t flag)
{
	material_t *mat;

	if ((mat = material_lookup(obj->material)) == NULL)
		return FALSE;

	return IS_SET(mat->mat_flags, flag);
}

flag_t
get_mat_flags(OBJ_DATA *obj)
{
	material_t *mat;

	if ((mat = material_lookup(obj->material)) == NULL)
		return 0;

	return mat->mat_flags;
}

int
floating_time(OBJ_DATA *obj)
{
	int  ftime = 0;
	material_t *mat;

	if (obj->item_type == ITEM_BOAT)
		return -1;

	if ((mat = material_lookup(obj->material)) != NULL
	&&  (ftime = mat->float_time) == 0)
		return ftime;

	switch (obj->item_type) {
	default:
		break;
	case ITEM_POTION:
	case ITEM_FOOD:
	case ITEM_CONTAINER:
		ftime += 1;
		break;
	case ITEM_CORPSE_NPC:
	case ITEM_CORPSE_PC:
		ftime += 3;
		break;
	}

	return ftime;
}
