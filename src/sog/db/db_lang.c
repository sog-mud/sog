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
 * $Id: db_lang.c,v 1.5 1998-10-30 06:56:55 fjoe Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "merc.h"
#include "db.h"
#include "lang.h"
#include "word.h"

LANG_DATA *	lang_curr;

DECLARE_DBLOAD_FUN(load_lang);

DBFUN db_load_langs[] =
{
	{ "LANG",	load_lang },
	{ NULL }
};

void load_hash(const char* file, varr** hash);

DBLOAD_FUN(load_lang)
{
	lang_curr = lang_new();
	lang_curr->file_name = get_filename(filename);

	for (;;) {
		char *word = feof(fp) ? "End" : fread_word(fp);
		bool fMatch = FALSE;

		switch (UPPER(*word)) {
		case 'C':
			SKEY("CasesFile", lang_curr->file_cases);
			break;

		case 'E':
			if (!str_cmp(word, "End")) {
				if (IS_NULLSTR(lang_curr->name)) {
					db_error("load_lang",
						 "lang name undefined");
					langs.nused--;
					return;
				}
				load_hash(lang_curr->file_genders,
					  lang_curr->hash_genders);
				load_hash(lang_curr->file_cases,
					  lang_curr->hash_cases);
				return;
			}
			break;

		case 'F':
			KEY("Flags", lang_curr->flags,
			    fread_fstring(lang_flags, fp));
			break;

		case 'G':
			SKEY("GendersFile", lang_curr->file_genders);
			break;

		case 'N':
			KEY("Name", lang_curr->name, str_dup(fread_word(fp)));
			break;

		case 'S':
			KEY("SlangOf", lang_curr->slang_of,
			    lang_lookup(fread_word(fp)));
		}

		if (!fMatch) 
			db_error("load_lang", "%s: Unknown keyword", word);
	}
}

/* local functions */

varr** hashp;

DECLARE_DBLOAD_FUN(load_word);

DBFUN db_load_words[] =
{
	{ "WORD",	load_word },
	{ NULL }
};

void load_hash(const char *file, varr **p)
{
	char buf[PATH_MAX];
	int linenum;
	char *s;
	hashp = p;

	if (IS_NULLSTR(file))
		return;

	strnzcpy(buf, filename, sizeof(buf));
	linenum = line_number;

	s = strrchr(filename, '/');
	if (s)
		*(s+1) = '\0';
	else
		filename[0] = '\0';
	db_load_file(filename, file, db_load_words, NULL);

	strnzcpy(filename, buf, sizeof(buf));
	line_number = linenum;
}

DBLOAD_FUN(load_word)
{
	WORD_DATA *w;

	w = word_new(lang_curr->vnum);

	for (;;) {
		char *word = feof(fp) ? "End" : fread_word(fp);
		bool fMatch = FALSE;

		switch(UPPER(*word)) {
		case 'B':
			SKEY("Base", w->base);
			break;

		case 'E':
			if (!str_cmp(word, "End")) {
				if (IS_NULLSTR(w->name)) {
					db_error("load_word",
						 "word name undefined");
					word_free(w);
					return;
				}
				if (!word_add(hashp, w)) {
					word_free(w);
					return;
				}
				return;
			}
			break;

		case 'F':
			if (!str_cmp(word, "Form")) {
				int fnum = fread_number(fp);
				const char *fstring = fread_string(fp);
				word_form_add(w, fnum, fstring);
				free_string(fstring);
				fMatch = TRUE;
			}
			break;

		case 'N':
			SKEY("Name", w->name);
			break;
		}

		if (!fMatch) 
			db_error("load_word", "%s: Unknown keyword", word);
	}
}

