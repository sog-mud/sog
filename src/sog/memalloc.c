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
 * $Id: memalloc.c,v 1.3.2.2 2003-09-30 01:25:25 fjoe Exp $
 */

#include <stdlib.h>

#include "typedef.h"
#include "log.h"
#include "memalloc.h"

void * mem_alloc2(int mem_type, size_t mem_len, size_t mem_prealloc)
{
	char *p;
	memchunk_t *m;

	p = malloc(mem_prealloc + sizeof(memchunk_t) + mem_len);
	if (p == NULL)
		return NULL;

	m = (memchunk_t*) (p + mem_prealloc);
	m->mem_type = mem_type;
	m->mem_sign = MEM_VALID;
	m->mem_prealloc = mem_prealloc;

	return ((void*) (p + mem_prealloc + sizeof(memchunk_t)));
}

void mem_free(const void *p)
{
	memchunk_t *m;

	if (p == NULL)
		return;

	m = GET_CHUNK(p);
	if (m->mem_sign != MEM_VALID) {
		printlog("mem_free: invalid pointer");
		return;
	}

	free(((char*) m) - m->mem_prealloc);
}

bool mem_is(const void *p, int mem_type)
{
	memchunk_t *m;

	if (p == NULL)
		return FALSE;

	m = GET_CHUNK(p);
	return (m->mem_sign == MEM_VALID && m->mem_type == mem_type);
}

void mem_validate(const void *p)
{
	memchunk_t *m;

	if (p == NULL)
		return;
	m = GET_CHUNK(p);
	m->mem_sign = MEM_VALID;
}

void mem_invalidate(const void *p)
{
	memchunk_t *m;

	if (p == NULL)
		return;
	m = GET_CHUNK(p);
	m->mem_sign = MEM_INVALID;
}
