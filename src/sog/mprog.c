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
 * $Id: mprog.c,v 1.2 2001-08-26 05:49:16 fjoe Exp $
 */

#include <stdarg.h>
#include <stdlib.h>

#include <typedef.h>
#include <str.h>
#include <varr.h>
#include <hash.h>
#include <strkey_hash.h>
#include <mprog.h>
#include <buffer.h>

hash_t mprogs;

int (*mprog_compile)(mprog_t *mp);
int (*mprog_execute)(mprog_t *mp, va_list ap);

hashdata_t h_mprogs = {
	sizeof(mprog_t), 8,
	(e_init_t) mprog_init,
	(e_destroy_t) mprog_destroy,
	(e_cpy_t) mprog_cpy,

	STRKEY_HASH_SIZE,
	k_hash_str,
	ke_cmp_str
};

void
mprog_init(mprog_t *mp)
{
	mp->name = str_empty;
	mp->type = MP_T_NONE;
	mp->status = MP_S_DIRTY;
	mp->flags = 0;
	mp->text = str_empty;
	mp->errbuf = NULL;
}

void
mprog_destroy(mprog_t *mp)
{
	free_string(mp->name);
	free_string(mp->text);
	if (mp->errbuf != NULL)
		buf_free(mp->errbuf);
}

mprog_t *
mprog_cpy(mprog_t *dst, const mprog_t *src)
{
	dst->name = str_qdup(src->name);
	dst->type = src->type;
	dst->status = src->status;
	dst->flags = src->flags;
	dst->text = str_qdup(src->text);
	dst->errbuf = NULL;

	return dst;
}
