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
 * $Id: liquid.c,v 1.7 2001-07-29 09:43:21 fjoe Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include "merc.h"

hash_t liquids;

void liquid_init(liquid_t *lq)
{
	int i;

	gmlstr_init(&lq->lq_name);
	mlstr_init2(&lq->lq_color, str_empty);
	for (i = 0; i < MAX_COND; i++)
		lq->affect[i] = 0;
	lq->sip = 0;
}

liquid_t *liquid_cpy(liquid_t *dst, const liquid_t *src)
{
	int i;

	gmlstr_cpy(&dst->lq_name, &src->lq_name);
	mlstr_cpy(&dst->lq_color, &src->lq_color);
	for (i = 0; i < MAX_COND; i++)
		dst->affect[i] = src->affect[i];
	dst->sip = src->sip;
	return dst;
}

void liquid_destroy(liquid_t *lq)
{
	gmlstr_destroy(&lq->lq_name);
	mlstr_destroy(&lq->lq_color);
}
