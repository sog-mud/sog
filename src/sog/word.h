/*-
 * Copyright (c) 1998 fjoe <fjoe@iclub.nsu.ru>
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
 * $Id: word.h,v 1.9 1999-02-22 04:27:40 fjoe Exp $
 */

#ifndef _WORD_H_
#define _WORD_H_

struct word_data {
	const char *	name;
	size_t		base_len;	/* length of base (not grammatic one --
					 * just length of unchanged part
					 * of the name			*/
	varr 		f;		/* forms */
};

void	word_init	(WORD_DATA*);
void	word_clear	(WORD_DATA*);

WORD_DATA *	word_add	(varr* hash, WORD_DATA *w);
void		word_del	(varr* hash, const char *name);

WORD_DATA *	word_lookup	(varr* hash, const char *name);

void		word_form_add	(WORD_DATA *w, int fnum, const char *s);
void		word_form_del	(WORD_DATA *w, int fnum);

const char *	word_gender(int lang, const char *word, int gender);
const char *	word_case(int lang, const char *word, int num);
const char *	word_quantity(int lang, const char *word, int num);

const char * fix_short(const char *s);

#endif
