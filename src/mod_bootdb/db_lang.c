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
 * $Id: db_lang.c,v 1.39 2003-09-30 00:31:04 fjoe Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <merc.h>
#include <lang.h>
#include <db.h>
#include <rwfile.h>

DECLARE_DBLOAD_FUN(load_lang);
DECLARE_DBLOAD_FUN(load_rulecl);
DECLARE_DBINIT_FUN(init_lang);

DBFUN dbfun_langs[] =
{
	{ "LANG",	load_lang,	NULL	},		// notrans
	{ "RULECLASS",	load_rulecl,	NULL	},		// notrans
	{ NULL, NULL, NULL }
};

DBDATA db_langs = { dbfun_langs, init_lang, 0 };

DECLARE_DBLOAD_FUN(load_expl);
DECLARE_DBLOAD_FUN(load_impl);

DBFUN dbfun_expl[] =
{
	{ "RULE",	load_expl,	NULL },			// notrans
	{ NULL, NULL, NULL }
};

DBFUN dbfun_impl[] =
{
	{ "RULE",	load_impl,	NULL },			// notrans
	{ NULL, NULL, NULL }
};

DBDATA db_expl = { dbfun_expl, NULL, 0 };
DBDATA db_impl = { dbfun_impl, NULL, 0 };

static lang_t *
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

DBINIT_FUN(init_lang)
{
	if (DBDATA_VALID(dbdata))
		db_set_arg(dbdata, "RULECLASS", NULL);
	else
		c_init(&langs, &c_info_langs);
}

DBLOAD_FUN(load_lang)
{
	lang_t *lang = varr_enew(&langs);
	lang->file_name = get_filename(bootdb_filename);
	db_set_arg(dbdata, "RULECLASS", lang);

	for (;;) {
		bool fMatch = FALSE;

		fread_keyword(fp);
		switch (rfile_tokfl(fp)) {
		case 'E':
			if (IS_TOKEN(fp, "End")) {
				if (IS_NULLSTR(lang->name)) {
					printlog(LOG_ERROR, "load_lang: lang name undefined");
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
			printlog(LOG_ERROR, "%s: %s: Unknown keyword",
			    __FUNCTION__, rfile_tok(fp));
			fread_to_eol(fp);
		}
	}
}

DBLOAD_FUN(load_rulecl)
{
	lang_t *l = arg;
	rulecl_t *rcl = NULL;

	if (!l) {
		printlog(LOG_ERROR, "load_rulecl: #RULECLASS before #LANG");
		return;
	}

	fread_keyword(fp);
	if (IS_TOKEN(fp, "Class"))
		rcl = l->rules + fread_fword(rulecl_names, fp);
	else {
		printlog(LOG_ERROR, "load_rulecl: Class must be defined first");
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

				s = strrchr(bootdb_filename, PATH_SEPARATOR);
				if (s) {
					strlncpy(path, bootdb_filename,
					    sizeof(path), s - bootdb_filename);
				} else
					path[0] = '\0';

				if (!IS_NULLSTR(rcl->file_expl)) {
					db_set_arg(&db_expl, "RULE", rcl);
					db_load_file(&db_expl, path,
						     rcl->file_expl);
				}

				if (!IS_NULLSTR(rcl->file_impl)) {
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
			printlog(LOG_ERROR, "%s: %s: Unknown keyword",
			    __FUNCTION__, rfile_tok(fp));
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
					printlog(LOG_ERROR, "load_rules: rule name undefined");
					rule_destroy(&r);
				} else if (!rule_add(rcl, &r))
					rule_destroy(&r);
				return;
			}
			break;

		case 'F':
			if (IS_TOKEN(fp, "Form")) {
				size_t fnum = fread_number(fp);
				const char *fstring = fread_string(fp);

				rule_form_add(&r, fnum, fstring);
				free_string(fstring);
				fMatch = TRUE;
			}
			break;

		case 'N':
			SKEY("Name", r.name, fread_string(fp));
			break;
		}

		if (!fMatch) {
			printlog(LOG_ERROR, "%s: %s: Unknown keyword",
			    __FUNCTION__, rfile_tok(fp));
			fread_to_eol(fp);
		}
	}
}
