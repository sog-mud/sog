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
 * $Id: material.h,v 1.6 1999-12-16 12:24:44 fjoe Exp $
 */

#ifndef _MATERIAL_H_
#define _MATERIAL_H_

#define MATERIAL_INDESTRUCT	(A)
#define MATERIAL_METAL		(B)
#define MATERIAL_EDIBLE		(C)
#define MATERIAL_SUSC_HEAT	(D)
#define MATERIAL_SUSC_COLD	(E)
#define MATERIAL_FRAGILE	(F)

typedef struct material_t material_t;
struct material_t {
	const char *name;
	int float_time;
	int dam_class;
	flag_t mat_flags;
};

extern hash_t materials;

void 		material_init	(material_t *sk);
material_t *	material_cpy	(material_t *dst, const material_t *src);
void		material_destroy	(material_t *sk);

#define material_lookup(mn)	((material_t *) strkey_lookup(&materials, (mn)))
#define material_search(mn)	((material_t *) strkey_search(&materials, (mn)))

bool		material_is	(OBJ_DATA *obj, flag_t flag);
flag_t		get_mat_flags	(OBJ_DATA *obj);
int		floating_time	(OBJ_DATA *obj);

#endif
