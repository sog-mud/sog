/*-
 * Copyright (c) 1999 arborn <avn@org.chem.msu.su>
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
 * $Id: liquid.c,v 1.3 1999-12-15 15:35:43 fjoe Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include "merc.h"

hash_t liquids;

void liquid_init(liquid_t *lq)
{
	int i;

	mlstr_init(&lq->lq_name, str_empty);
	mlstr_init(&lq->lq_gender, str_empty);
	mlstr_init(&lq->lq_color, str_empty);
	for (i = 0; i < MAX_COND; i++)
		lq->affect[i] = 0;
	lq->sip = 0;
}

liquid_t *liquid_cpy(liquid_t *dst, const liquid_t *src)
{
	int i;

	mlstr_cpy(&dst->lq_name, &src->lq_name);
	mlstr_cpy(&dst->lq_gender, &src->lq_gender);
	mlstr_cpy(&dst->lq_color, &src->lq_color);
	for (i = 0; i < MAX_COND; i++)
		dst->affect[i] = src->affect[i];
	dst->sip = src->sip;
	return dst;
}

void liquid_destroy(liquid_t *lq)
{
	mlstr_destroy(&lq->lq_name);
	mlstr_destroy(&lq->lq_gender);
	mlstr_destroy(&lq->lq_color);
}

