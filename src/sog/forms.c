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
 * $Id: forms.c,v 1.8 2001-07-29 09:43:18 fjoe Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include "merc.h"

hash_t forms;

void form_init(form_index_t *f)
{
	int i;
	f->name			= str_empty;
	mlstr_init2(&f->description, str_empty);
	mlstr_init2(&f->short_desc, str_empty);
	mlstr_init2(&f->long_desc, str_empty);
	f->damtype		= str_empty;
	f->damage[DICE_TYPE]	= 0;
	f->damage[DICE_NUMBER]	= 0;
	f->damage[DICE_BONUS]	= 1;
	f->hitroll		= 0;
	f->num_attacks		= 0;
	f->flags		= 0;
	f->skill_spec	= str_empty;
	for (i = 0; i < MAX_RESIST; i++)
		f->resists[i] = 0;
	for (i = 0; i < MAX_STAT; i++)
		f->stats[i] = 10;
}

form_index_t* form_cpy(form_index_t *dst, const form_index_t *src)
{
	int i;
	dst->name		= str_qdup(src->name);
	mlstr_cpy(&dst->description, &src->description);
	mlstr_cpy(&dst->short_desc, &src->short_desc);
	mlstr_cpy(&dst->long_desc, &src->long_desc);
	dst->damtype		= str_qdup(src->damtype);
	dst->hitroll		= src->hitroll;
	dst->num_attacks	= src->num_attacks;
	dst->skill_spec		= str_qdup(src->skill_spec);
	dst->flags		= src->flags;
	dst->damage[DICE_TYPE]	= src->damage[DICE_TYPE];
	dst->damage[DICE_NUMBER]= src->damage[DICE_NUMBER];
	dst->damage[DICE_BONUS]	= src->damage[DICE_BONUS];
	for (i = 0; i < MAX_RESIST; i++)
		dst->resists[i] = src->resists[i];
	for (i = 0; i < MAX_STAT; i++)
		dst->stats[i] = src->stats[i];

	return dst;
}

void form_destroy(form_index_t *f)
{
	free_string(f->name);
	mlstr_destroy(&f->description);
	mlstr_destroy(&f->short_desc);
	mlstr_destroy(&f->long_desc);
	free_string(f->damtype);
	free_string(f->skill_spec);
}

bool shapeshift(CHAR_DATA* ch, const char* shapeform)
{
	form_index_t* form_index;
	form_t * form;
	int i;

	if (!(form_index = form_lookup(shapeform))) {
		log(LOG_BUG, "shapeshift: unknown form %s.\n", shapeform);
		return FALSE;
	}

	form = (form_t*) calloc(1, sizeof(form_t));

	form->index = form_index;
	form->damroll = form_index->damage[DICE_BONUS];
	form->hitroll = form_index->hitroll;

	for (i = 0; i < MAX_RESIST; i++)
		form->resists[i] = form_index->resists[i];

	ch->shapeform = form;

	return TRUE;
}

bool revert(CHAR_DATA* ch)
{
	AFFECT_DATA *paf;
	AFFECT_DATA *paf_next;

	for (paf = ch->affected; paf; paf = paf_next) {
		paf_next = paf->next;
		if (paf->where == TO_FORMAFFECTS)
			affect_remove(ch, paf);
	}

	if (!ch->shapeform) {
		log(LOG_BUG, "Revert: character is not shapeshifted.\n");
		return FALSE;
	}

	free(ch->shapeform);

	ch->shapeform = NULL;
	return TRUE;
}
