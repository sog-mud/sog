/*-
 * Copyright (c) 1998 SoG Development Team
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
 * $Id: db_lang.c,v 1.24 2000-10-07 10:58:07 fjoe Exp $
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

static int
fread_lang(rfile_t *fp)
{
	fread_word(fp);
#if !defined(NO_MMAP)
	return lang_nlookup(fp->tok, fp->tok_len);
#else
	return lang_lookup(rfile_tok(fp));
#endif
}

/*----------------------------------------------------------------------------
 * lang loader
 */

static varrdata_t v_langs =
{
	sizeof(lang_t), 2,
	(e_init_t) lang_init
};

DBINIT_FUN(init_lang)
{
	if (DBDATA_VALID(dbdata))
		db_set_arg(dbdata, "RULECLASS", NULL);
	else
		varr_init(&langs, &v_langs);
}

DBLOAD_FUN(load_lang)
{
	lang_t *lang = varr_enew(&langs);
	lang->file_name = get_filename(filename);
	db_set_arg(dbdata, "RULECLASS", lang);

	for (;;) {
		bool fMatch = FALSE;

		fread_keyword(fp);
		switch (rfile_tokfl(fp)) {
		case 'E':
			if (IS_TOKEN(fp, "End")) {
				if (IS_NULLSTR(lang->name)) {
					log(LOG_ERROR, "load_lang: lang name undefined");
					langs.nused--;
					return;
				}
				return;
			}
			break;
		case 'F':
			KEY("Flags", lang->lang_flags,
			    fread_fstring(lang_flags, fp));
			break;
		case 'N':
			KEY("Name", lang->name, fread_sword(fp));
			break;
		case 'S':
			KEY("SlangOf", lang->slang_of, fread_lang(fp));
			break;
		}

		if (!fMatch) {
			log(LOG_ERROR, "load_lang: %s: Unknown keyword",
				 rfile_tok(fp));
			fread_to_eol(fp);
		}
	}
}

DBLOAD_FUN(load_rulecl)
{
	lang_t *l = arg;
	rulecl_t *rcl = NULL;

	if (!l) {
		log(LOG_ERROR, "load_rulecl: #RULECLASS before #LANG");
		return;
	}

	fread_keyword(fp);
	if (IS_TOKEN(fp, "Class"))
		rcl = l->rules + fread_fword(rulecl_names, fp);
	else {
		log(LOG_ERROR, "load_rulecl: Class must be defined first");
		return;
	}

	for (;;) {
		bool fMatch = FALSE;

		fread_keyword(fp);
		switch (rfile_tokfl(fp)) {
		case 'E':
			SKEY("Expl", rcl->file_expl, fread_string(fp));
			if (IS_TOKEN(fp, "End")) {
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
			SKEY("Impl", rcl->file_impl, fread_string(fp));
			break;
		}

		if (!fMatch) {
			log(LOG_ERROR, "load_rulecl: %s: Unknown keyword",
				 rfile_tok(fp));
			fread_to_eol(fp);
		}
	}
}

static void
load_rules(rfile_t *fp, rulecl_t *rcl, rule_t* (*add_rule)(rulecl_t*, rule_t*));

DBLOAD_FUN(load_expl)
{
	load_rules(fp, arg, erule_add);
}

DBLOAD_FUN(load_impl)
{
	load_rules(fp, arg, irule_add);
}

static void
load_rules(rfile_t *fp, rulecl_t *rcl, rule_t* (*rule_add)(rulecl_t*, rule_t*))
{
	rule_t r;
	rule_init(&r);

	for (;;) {
		bool fMatch = FALSE;

		fread_keyword(fp);
		switch(rfile_tokfl(fp)) {
		case 'B':
			KEY("BaseLen", r.arg, fread_number(fp));
			break;

		case 'E':
			if (IS_TOKEN(fp, "End")) {
				if (IS_NULLSTR(r.name)) {
					log(LOG_ERROR, "load_rules: rule name undefined");
					rule_destroy(&r);
				} else if (!rule_add(rcl, &r))
					rule_destroy(&r);
				return;
			}
			break;

		case 'F':
			if (IS_TOKEN(fp, "Form")) {
				int fnum = fread_number(fp);
				const char *fstring = fread_string(fp);

				if (fnum < 0) {
					log(LOG_ERROR, "load_rules: %d: Negative form number", fnum);
					break;
				}

				vform_add(r.f, fnum, fstring);
				free_string(fstring);
				fMatch = TRUE;
			}
			break;

		case 'N':
			SKEY("Name", r.name, fread_string(fp));
			break;
		}

		if (!fMatch) {
			log(LOG_ERROR, "load_rules: %s: Unknown keyword",
				 rfile_tok(fp));
			fread_to_eol(fp);
		}
	}
}

