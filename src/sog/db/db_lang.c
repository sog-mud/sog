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
 * $Id: db_lang.c,v 1.15 1999-06-03 10:24:06 fjoe Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "merc.h"
#include "db.h"
#include "lang.h"

DECLARE_DBLOAD_FUN(load_lang);
DECLARE_DBLOAD_FUN(load_rulecl);
DECLARE_DBINIT_FUN(init_lang);

DBFUN dbfun_langs[] =
{
	{ "LANG",	load_lang	},
	{ "RULECLASS",	load_rulecl	},
	{ NULL }
};

DBDATA db_langs = { dbfun_langs, init_lang };

DECLARE_DBLOAD_FUN(load_expl);
DECLARE_DBLOAD_FUN(load_impl);

DBFUN dbfun_expl[] =
{
	{ "RULE",	load_expl },
	{ NULL }
};

DBFUN dbfun_impl[] =
{
	{ "RULE",	load_impl },
	{ NULL }
};

DBDATA db_expl = { dbfun_expl };
DBDATA db_impl = { dbfun_impl };

/*----------------------------------------------------------------------------
 * lang loader
 */
DBINIT_FUN(init_lang)
{
	db_set_arg(dbdata, "RULECLASS", NULL);
}

DBLOAD_FUN(load_lang)
{
	lang_t *lang = lang_new();
	lang->file_name = get_filename(filename);
	db_set_arg(dbdata, "RULECLASS", lang);

	for (;;) {
		char *word = feof(fp) ? "End" : fread_word(fp);
		bool fMatch = FALSE;

		switch (UPPER(*word)) {
		case 'E':
			if (!str_cmp(word, "End")) {
				if (IS_NULLSTR(lang->name)) {
					db_error("load_lang",
						 "lang name undefined");
					langs.nused--;
					return;
				}
				return;
			}
			break;
		case 'F':
			KEY("Flags", lang->flags,
			    fread_fstring(lang_flags, fp));
			break;
		case 'N':
			KEY("Name", lang->name, str_dup(fread_word(fp)));
			break;
		case 'S':
			KEY("SlangOf", lang->slang_of,
			    lang_lookup(fread_word(fp)));
		}

		if (!fMatch) 
			db_error("load_lang", "%s: Unknown keyword", word);
	}
}

DBLOAD_FUN(load_rulecl)
{
	lang_t *l = arg;
	char *word;
	rulecl_t *rcl = NULL;

	if (!l) {
		db_error("load_rulecl", "#RULECLASS before #LANG");
		return;
	}

	word = feof(fp) ? "End" : fread_word(fp);
	if (!str_cmp(word, "Class"))
		rcl = l->rules + fread_fword(rulecl_names, fp);
	else {
		db_error("load_rulecl", "Class must be defined first");
		return;
	}

	for (;;) {
		char *word = feof(fp) ? "End" : fread_word(fp);
		bool fMatch = FALSE;

		switch (UPPER(*word)) {
		case 'E':
			SKEY("Expl", rcl->file_expl);
			if (!str_cmp(word, "End")) {
				const char *s;
				char path[PATH_MAX];

				s = strrchr(filename, PATH_SEPARATOR);
				if (s) {
					strnzncpy(path, sizeof(path),
						  filename, s - filename);
				}
				else
					path[0] = '\0';

				if (rcl->file_expl) {
					db_set_arg(&db_expl, "RULE", rcl);
					db_load_file(&db_expl, path,
						     rcl->file_expl);
				}

				if (rcl->file_impl) {
					db_set_arg(&db_impl, "RULE", rcl);
					db_load_file(&db_impl, path,
						     rcl->file_impl);
				}
				return;
			}
			break;
		case 'I':
			SKEY("Impl", rcl->file_impl);
			break;
		}

		if (!fMatch) 
			db_error("load_rulecl", "%s: Unknown keyword", word);
	}
}

static void
load_rules(FILE *fp, rulecl_t *rcl, rule_t* (*add_rule)(rulecl_t*, rule_t*));

DBLOAD_FUN(load_expl)
{
	load_rules(fp, arg, erule_add);
}

DBLOAD_FUN(load_impl)
{
	load_rules(fp, arg, irule_add);
}

static void
load_rules(FILE *fp, rulecl_t *rcl, rule_t* (*rule_add)(rulecl_t*, rule_t*))
{
	rule_t r;
	rule_init(&r);

	for (;;) {
		char *word = feof(fp) ? "End" : fread_word(fp);
		bool fMatch = FALSE;

		switch(UPPER(*word)) {
		case 'B':
			KEY("BaseLen", r.arg, fread_number(fp));
			break;

		case 'E':
			if (!str_cmp(word, "End")) {
				if (IS_NULLSTR(r.name)) {
					db_error("load_rules",
						 "rule name undefined");
					rule_clear(&r);
				}
				else if (!rule_add(rcl, &r))
					rule_clear(&r);
				return;
			}
			break;

		case 'F':
			if (!str_cmp(word, "Form")) {
				int fnum = fread_number(fp);
				const char *fstring = fread_string(fp);

				if (fnum < 0) {
					db_error("load_rules", "%d: Negative form number", fnum);
					break;
				}

				vform_add(r.f, fnum, fstring);
				free_string(fstring);
				fMatch = TRUE;
			}
			break;

		case 'N':
			SKEY("Name", r.name);
			break;
		}

		if (!fMatch) 
			db_error("load_rules", "%s: Unknown keyword", word);
	}
}

