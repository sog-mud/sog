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
 * $Id: db_lang.c,v 1.11 1999-02-22 04:27:40 fjoe Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "merc.h"
#include "db.h"
#include "word.h"
#include "lang.h"

LANG_DATA *	lang;

DECLARE_DBLOAD_FUN(load_lang);

DBFUN dbfun_langs[] =
{
	{ "LANG",	load_lang },
	{ NULL }
};

DBDATA db_langs = { dbfun_langs };

DECLARE_DBLOAD_FUN(load_word);

DBFUN dbfun_words[] =
{
	{ "WORD",	load_word },
	{ NULL }
};

DBDATA db_words = { dbfun_words };

DBLOAD_FUN(load_lang)
{
	LANG_DATA *lang = lang_new();
	lang->file_name = get_filename(filename);

	for (;;) {
		char *word = feof(fp) ? "End" : fread_word(fp);
		bool fMatch = FALSE;

		switch (UPPER(*word)) {
		case 'C':
			SKEY("CasesFile", lang->file_cases);
			break;

		case 'E':
			if (!str_cmp(word, "End")) {
				const char *s;
				char path[PATH_MAX];

				if (IS_NULLSTR(lang->name)) {
					db_error("load_lang",
						 "lang name undefined");
					langs.nused--;
					return;
				}

				s = strrchr(filename, PATH_SEPARATOR);
				if (s)
					strnzcpy(path, filename,
						 UMIN(s - filename + 1,
						      sizeof(path)));
				else
					path[0] = '\0';

				if (lang->file_genders) {
					db_set_arg(&db_words, "WORD",
						   lang->hash_genders);
					db_load_file(&db_words, path,
						     lang->file_genders);
				}

				if (lang->file_cases) {
					db_set_arg(&db_words, "WORD",
						   lang->hash_cases);
					db_load_file(&db_words, path,
						     lang->file_cases);
				}

				if (lang->file_qtys) {
					db_set_arg(&db_words, "WORD",
						   lang->hash_qtys);
					db_load_file(&db_words, path,
						     lang->file_qtys);
				}
				return;
			}
			break;
		case 'F':
			KEY("Flags", lang->flags,
			    fread_fstring(lang_flags, fp));
			break;
		case 'G':
			SKEY("GendersFile", lang->file_genders);
			break;
		case 'N':
			KEY("Name", lang->name, str_dup(fread_word(fp)));
			break;
		case 'Q':
			SKEY("QtysFile", lang->file_qtys);
			break;
		case 'S':
			KEY("SlangOf", lang->slang_of,
			    lang_lookup(fread_word(fp)));
		}

		if (!fMatch) 
			db_error("load_lang", "%s: Unknown keyword", word);
	}
}

DBLOAD_FUN(load_word)
{
	varr *hash = arg;
	WORD_DATA w;

	word_init(&w);
	for (;;) {
		char *word = feof(fp) ? "End" : fread_word(fp);
		bool fMatch = FALSE;

		switch(UPPER(*word)) {
		case 'B':
			if (!str_cmp(word, "Base")) {
				free_string(fread_string(fp));
				fMatch = TRUE;
				break;
			}
			KEY("BaseLen", w.base_len, fread_number(fp));
			break;

		case 'E':
			if (!str_cmp(word, "End")) {
				if (IS_NULLSTR(w.name)) {
					db_error("load_word",
						 "word name undefined");
					word_clear(&w);
					return;
				}
				if (!word_add(hash, &w)) {
					word_clear(&w);
					return;
				}
				return;
			}
			break;

		case 'F':
			if (!str_cmp(word, "Form")) {
				int fnum = fread_number(fp);
				const char *fstring = fread_string(fp);
				word_form_add(&w, fnum, fstring);
				free_string(fstring);
				fMatch = TRUE;
			}
			break;

		case 'N':
			SKEY("Name", w.name);
			break;
		}

		if (!fMatch) 
			db_error("load_word", "%s: Unknown keyword", word);
	}
}

