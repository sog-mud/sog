/*-
 * Copyright (c) 1999 fjoe <fjoe@iclub.nsu.ru>
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
 * $Id: memalloc.h,v 1.1 1999-06-10 11:49:20 fjoe Exp $
 */

#ifndef _MEMALLOC_H_
#define _MEMALLOC_H_

enum {
	MT_STR,
	MT_CHAR,
	MT_OBJ,
	MT_ROOM,
};

#define MEM_VALID	0x5a	/* valid chunk signature	*/

typedef struct memchunk_t {
	char		mem_type;	/* memory chunk type		*/
	char		mem_sign;	/* memory chunk signature	*/
	unsigned char	mem_prealloc;	/* preallocated data size	*/
	char		mem_dummy;
} memchunk_t;

#define mem_alloc(mem_type, mem_len) mem_alloc2(mem_type, mem_len, 0)
void *	mem_alloc2(int mem_type, size_t mem_len, size_t mem_prealloc);
void	mem_free(const void *p);

#define GET_CHUNK(p) ((memchunk_t*) (((char*) p) - sizeof(memchunk_t)))
bool	mem_is(const void *p, int mem_type);

#endif

