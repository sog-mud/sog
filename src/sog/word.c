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
 * $Id: word.c,v 1.20 1999-02-23 22:06:50 fjoe Exp $
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "merc.h"
#include "db.h"
#include "word.h"
#include "lang.h"

#define wordhash(s) hashistr(s, 16, MAX_WORD_HASH)

const char* word_form_lookup(varr *hash, const char *word, int num);
static int cmpword(const void *p1, const void *p2);

void word_init(WORD_DATA *w)
{
	memset(w, 0, sizeof(*w));
	w->f.nsize = sizeof(char*);
	w->f.nstep = 4;
}

void word_clear(WORD_DATA *w)
{
	int i;

	free_string(w->name);
	w->name = NULL;

	for (i = 0; i < w->f.nused; i++) 
		free_string(VARR_GET(&w->f, i));
}

WORD_DATA *word_add(varr *hash, WORD_DATA *w)
{
	WORD_DATA *wnew;
	varr *v;

	if (IS_NULLSTR(w->name))
		return NULL;

	v = hash + wordhash(w->name);
	if (varr_bsearch(v, w, cmpword))
		return NULL;

	wnew = varr_enew(v);
	*wnew = *w;
	varr_qsort(v, cmpword);
	return varr_bsearch(v, w, cmpword);
}

void word_del(varr *hash, const char *name)
{
	varr *v;
	WORD_DATA *w;

	if (IS_NULLSTR(name))
		return;

	v = hash+wordhash(name);
	if ((w = varr_bsearch(v, &name, cmpword)) == NULL)
		return;
	word_clear(w);
	varr_qsort(v, cmpword);
	v->nused--;
}

WORD_DATA *word_lookup(varr *hash, const char *name)
{
	if (IS_NULLSTR(name))
		return NULL;
	return varr_bsearch(hash+wordhash(name), &name, cmpword);
}

void word_form_add(WORD_DATA* w, int fnum, const char *s)
{
	const char **p = varr_touch(&w->f, fnum);
	if (*p)
		free_string(*p);
	*p = str_dup(s);
}

void word_form_del(WORD_DATA *w, int fnum)
{
	const char **p = varr_get(&w->f, fnum);
	if (*p)
		free_string(*p);
	*p = NULL;
}

const char *word_gender(int lang, const char *word, int gender)
{
	LANG_DATA *l = varr_get(&langs, lang);

	if (l == NULL)
		return word;

	return word_form_lookup(l->hash_genders, word,
				(gender + SEX_MAX - 1) % SEX_MAX);
}

const char *word_case(int lang, const char *word, int num)
{
	LANG_DATA *l = varr_get(&langs, lang);

	if (l == NULL)
		return word;

	return word_form_lookup(l->hash_cases, word, num);
}

const char *word_quantity(int lang, const char *word, int num)
{
	LANG_DATA *l = varr_get(&langs, lang);

	if (l == NULL)
		return word;

	num %= 100;
	return word_form_lookup(l->hash_qtys, word, num < 15 ? num : num % 10);
}

/* local functions */

const char* word_form_lookup(varr *hash, const char *word, int num)
{
	WORD_DATA *w;
	char **p;
	char *q;
	static char buf[MAX_STRING_LENGTH];

	if (!num || IS_NULLSTR(word))
		return word;

	if ((q = strchr(word, '~'))) {
		/*
		 * translate only the part of word between tildes
		 */
		char buf2[MAX_STRING_LENGTH];
		char buf3[MAX_STRING_LENGTH];
		const char *r;

		/* copy prefix */
		strnzncpy(buf2, sizeof(buf2), word, q-word);

		/*
		 * translate infix, translation must be done
		 * before copying the result to buf[] because buf is
		 * static
		 */
		r = strchr(q+1, '~');
		if (!r)
			r = strchr(q+1, '\0');
		strnzncpy(buf3, sizeof(buf3), q+1, *r ? r-q-1 : r-q);
		strnzcat(buf2, sizeof(buf2), word_form_lookup(hash, buf3, num));

		/* translate the rest */
		if (*r) {
			strnzcpy(buf3, sizeof(buf3),
				 word_form_lookup(hash, r+1, num));
			strnzcpy(buf, sizeof(buf), buf2);
			strnzcat(buf, sizeof(buf), buf3);
		}
		else
			strnzcpy(buf, sizeof(buf), buf2);

		return buf;
	}

	if ((w = word_lookup(hash, word)) == NULL
	||  (p = varr_get(&w->f, num)) == NULL
	||  IS_NULLSTR(*p))
		return word;

	if (!w->base_len || **p != '-')
		return *p;

	strnzcpy(buf, sizeof(buf), word);
	strnzcpy(buf + w->base_len, sizeof(buf) - w->base_len, *p + 1);
	return buf;
}

/* reverse order (otherwise word_del will not work) */
int cmpword(const void *p1, const void *p2)
{
	return -str_cmp(((WORD_DATA*) p1)->name, ((WORD_DATA*) p2)->name);
}

