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
 * $Id: word.c,v 1.2 1998-10-02 04:48:41 fjoe Exp $
 */

#include <sys/syslimits.h>
#include <stdio.h>
#include <stdlib.h>

#include "merc.h"
#include "db/db.h"
#include "db/lang.h"
#include "db/word.h"

#define wordhash(s) hashstr(s, 16, MAX_WORD_HASH);

const char* word_form_lookup(varr **hashp, const char *word, int num);

WORD_DATA *word_new(LANG_DATA *l)
{
	WORD_DATA *w = calloc(1, sizeof(WORD_DATA));
	w->base = str_empty;
	w->f = varr_new(sizeof(char*), 4);
	w->lang = l;
	return w;
}

WORD_DATA *word_add(varr **hashp, WORD_DATA *w)
{
	WORD_DATA *wnew;
	varr *v;
	int hash;

	if (IS_NULLSTR(w->name))
		return NULL;

	hash = wordhash(w->name);
	v = hashp[hash];
	if (v == NULL)
		v = hashp[hash] = varr_new(sizeof(WORD_DATA), 4);

	if (varr_bsearch(v, w, cmpstrp))
		return NULL;

	wnew = varr_enew(v);
	*wnew = *w;
	varr_qsort(v, cmpstrp);
	return varr_bsearch(v, w, cmpstrp);
}

void word_form_add(WORD_DATA* w, int fnum, const char *s)
{
	char **p = varr_touch(w->f, fnum);

	if (*p)
		free_string(*p);
	*p = str_dup(s);
}

void word_free(WORD_DATA *w)
{
	int i;

	free_string(w->name);
	free_string(w->base);

	for (i = 0; i < w->f->nused; i++) 
		free_string(VARR_GET(w->f, i));
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

/* local functions */

const char* word_form_lookup(varr **hashp, const char *word, int num)
{
	int hash;
	varr *v;
	WORD_DATA *w;
	char **p;
	static char buf[MAX_STRING_LENGTH];

	if (IS_NULLSTR(word) || num == 0)
		return word;

	hash = wordhash(word);
	if ((v = hashp[hash]) == NULL
	||  (w = varr_bsearch(v, &word, cmpstrp)) == NULL
	||  (p = varr_get(w->f, num)) == NULL)
		return word;

	if (**p != '-')
		return *p;

	strnzcpy(buf, w->base, sizeof(buf));
	strnzcat(buf, *p + 1, sizeof(buf));
	return buf;
}
