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
 * $Id: mlstring.c,v 1.23 1998-10-09 13:42:42 fjoe Exp $
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "merc.h"
#include "db/db.h"
#include "db/lang.h"

/*
 * multi-language string implementation
 */

/*
 * multi-language string
 * if nlang == 0 the value is stored in u.str
 * otherwise the value is stored in array of strings u.lstr
 * the size of array is equal to 'nlang'
 * 'ref' = number of references (COW semantics)
 */
struct mlstring {
	union {
		const char* str;
		const char** lstr;
	} u;
	int nlang;
	int ref;
};

static const char* smash_a(const char *s, int len);
static char* fix_mlstring(const char* s);
static mlstring *mlstr_split(mlstring *ml);

int mlstr_count;
int mlstr_real_count;

mlstring mlstr_empty;

mlstring *mlstr_new(const char *name)
{
	mlstring *res = malloc(sizeof(*res));
	res->u.str = str_dup(name);
	res->nlang = 0;
	res->ref = 1;
	mlstr_real_count++;
	return res;
}

mlstring *mlstr_fread(FILE *fp)
{
	const char *p;
	const char *s;
	int lang;
	mlstring *res;

	p = fread_string(fp);
	if (IS_NULLSTR(p))
		return &mlstr_empty;

	mlstr_count++;
	res = mlstr_new(NULL);
	if (*p != '@' || *(p+1) == '@') {
		res->u.str = smash_a(p, -1);
		free_string(p);
		return res;
	}

	res->u.lstr = calloc(1, sizeof(char*) * langs.nused);
	res->nlang = langs.nused;

	s = p+1;
	for (;;) {
		const char *q;

		/* s points at lang id */
		q = strchr(s, ' ');
		if (q == NULL) {
			db_error("mlstr_fread", "no ` ' after `@' found");
			return res;
		}

		if ((lang = lang_nlookup(s, q-s)) < 0) {
			db_error("mlstr_fread", "%s: unknown language", s);
			return res;
		}

		if (res->u.lstr[lang] != NULL) {
			db_error("mlstr_fread", "lang %s: redefined", s);
			return res;
		}

		/* find next '@', skip "@@" */
		for (s = ++q; (s = strchr(s, '@'));) {
			if (*(s+1) != '@')
				break;
			s += 2;
		}
		if (s == NULL)
			s = strchr(q, '\0');
		res->u.lstr[lang] = smash_a(q, s-q);
		if (!*s++)
			break;
	}

	free_string(p);
	return res;
}

void mlstr_fwrite(FILE *fp, const char* name, const mlstring *ml)
{
	int lang;

	if (name)
		fprintf(fp, "%s ", name);

	if (!ml) {
		fprintf(fp, "~\n");
		return;
	}

	if (ml->nlang == 0) {
		fprintf(fp, "%s~\n", fix_mlstring(ml->u.str));
		return;
	}

	for (lang = 0; lang < ml->nlang && lang < langs.nused; lang++) {
		const char* p = ml->u.lstr[lang];
		LANG_DATA *l;

		if (IS_NULLSTR(p))
			continue;

		l = VARR_GET(&langs, lang);
		fprintf(fp, "@%s %s", l->name, fix_mlstring(p));
	}
	fputs("~\n", fp);
}

void mlstr_free(mlstring *ml)
{
	if (ml == NULL || ml == &mlstr_empty)
		return;

	mlstr_count--;
	if (ml->ref < 1 || --ml->ref)
		return;

	if (ml->nlang == 0)
		free_string(ml->u.str);
	else {
		int lang;

		for (lang = 0; lang < ml->nlang; lang++)
			free_string(ml->u.lstr[lang]);
		free(ml->u.lstr);
	}
	free(ml);
	mlstr_real_count--;
}

mlstring *mlstr_dup(mlstring *ml)
{
	if (ml == NULL)
		return NULL;
	mlstr_count++;
	if (ml != &mlstr_empty)
		ml->ref++;
	return ml;
}

mlstring *mlstr_printf(mlstring *ml,...)
{
	char buf[MAX_STRING_LENGTH];
	va_list ap;
	mlstring *res;

	if (ml == NULL)
		return NULL;

	mlstr_count++;
	res = mlstr_new(NULL);
	res->nlang = ml->nlang;

	va_start(ap, ml);
	if (ml->nlang == 0) {
		vsnprintf(buf, sizeof(buf), ml->u.str, ap);
		res->u.str = str_dup(buf);
	}
	else {
		int lang;

		res->u.lstr = calloc(1, sizeof(char*) * res->nlang);
		for (lang = 0; lang < ml->nlang; lang++) {
			if (IS_NULLSTR(ml->u.lstr[lang]))
				continue;
			vsnprintf(buf, sizeof(buf), ml->u.lstr[lang], ap);
			res->u.lstr[lang] = str_dup(buf);
		}
	}
	va_end(ap);
	return res;
}

int mlstr_nlang(const mlstring *ml)
{
	if (ml == NULL)
		return 0;
	return ml->nlang;
}

const char * mlstr_val(const mlstring *ml, int lang)
{
	if (ml == NULL)
		return str_empty;

	if (ml->nlang == 0)
		return ml->u.str;

	if (lang >= ml->nlang
	||  lang < 0
	||  IS_NULLSTR(ml->u.lstr[lang])) {
		LANG_DATA *l;

		if ((l = varr_get(&langs, lang))
		&&  l->slang_of >= 0
		&&  l->slang_of < ml->nlang)
			lang = l->slang_of;
		else
			return str_empty;
	}

	return ml->u.lstr[lang];
}

bool mlstr_null(const mlstring *ml)
{
	const char *mval = mlstr_mval(ml);
	return (mval == NULL) || (*mval == '\0');
}

int mlstr_cmp(const mlstring *ml1, const mlstring *ml2)
{
	int lang;
	int res;

	if (ml1 == NULL)
		if (ml2 == NULL)
			return 0;
		else
			return 1;
	else if (ml2 == NULL)
		return -1;

	if (ml1->nlang != ml2->nlang)
		return  ml1->nlang - ml2->nlang;

	if (ml1->nlang == 0)
		return str_cmp(ml1->u.str, ml2->u.str);

	for (lang = 0; lang < ml1->nlang; lang++) {
		res = str_cmp(ml1->u.lstr[lang], ml2->u.lstr[lang]);
		if (res)
			return res;
	}

	return 0;
}

const char** mlstr_convert(mlstring **mlp, int newlang)
{
	const char *old;
	int lang;

	*mlp = mlstr_split(*mlp);
	if (newlang < 0) {
		/* convert to language-independent */
		if ((*mlp)->nlang) {
			old = (*mlp)->u.lstr[0];
			for (lang = 1; lang < (*mlp)->nlang; lang++)
				free_string((*mlp)->u.lstr[lang]);
			free((*mlp)->u.lstr);
			(*mlp)->nlang = 0;
			(*mlp)->u.str = old;
		}
		return &((*mlp)->u.str);
	}

	/* convert to language-dependent */
	if ((*mlp)->nlang == 0) {
		old = (*mlp)->u.str;
		(*mlp)->nlang = langs.nused;
		(*mlp)->u.lstr = calloc(1, sizeof(char*) * langs.nused);
		(*mlp)->u.lstr[0] = old;
	}
	return ((*mlp)->u.lstr)+newlang;
}

bool mlstr_append(CHAR_DATA *ch, mlstring **mlp, const char *arg)
{
	int lang;

	lang = lang_lookup(arg);
	if (lang < 0 && str_cmp(arg, "all"))
		return FALSE;

	string_append(ch, mlstr_convert(mlp, lang));
	return TRUE;
}

void mlstr_format(mlstring **mlp)
{
	int lang;

	if (*mlp == NULL)
		return;

	*mlp = mlstr_split(*mlp);
	if ((*mlp)->nlang == 0) {
		(*mlp)->u.str = format_string((*mlp)->u.str);
		return;
	}

	for (lang = 0; lang < (*mlp)->nlang; lang++)
		(*mlp)->u.lstr[lang] = format_string((*mlp)->u.lstr[lang]);
}

bool mlstr_edit(mlstring **mlp, const char *argument)
{
	char arg[MAX_STRING_LENGTH];
	int lang;
	const char **p;

	argument = one_argument(argument, arg);
	lang = lang_lookup(arg);
	if (lang < 0 && str_cmp(arg, "all"))
		return FALSE;

	p = mlstr_convert(mlp, lang);
	free_string(*p);
	*p = str_dup(argument);
	return TRUE;
}

/*
 * The same as mlstr_edit, but '\n' is appended.
 */
bool mlstr_editnl(mlstring **mlp, const char *argument)
{
	char arg[MAX_STRING_LENGTH];
	int lang;
	const char **p;

	argument = one_argument(argument, arg);
	lang = lang_lookup(arg);
	if (lang < 0 && str_cmp(arg, "all"))
		return FALSE;

	p = mlstr_convert(mlp, lang);
	free_string(*p);
	*p = str_add(argument, "\n\r", NULL);
	return TRUE;
}

void mlstr_dump(BUFFER *buf, const char *name, const mlstring *ml)
{
	char space[MAX_STRING_LENGTH];
	size_t namelen;
	int lang;
	static char FORMAT[] = "%s[%s] [%s]\n\r";
	LANG_DATA *l;

	if (ml == NULL || ml->nlang == 0) {
		buf_printf(buf, FORMAT, name, "all",
			   ml == NULL ? "(null)" : ml->u.str);
		return;
	}

	if (langs.nused == 0)
		return;

	l = VARR_GET(&langs, 0);
	buf_printf(buf, FORMAT, name, l->name, ml->u.lstr[0]);

	if (langs.nused < 1)
		return;

	namelen = strlen(name);
	namelen = URANGE(0, namelen, sizeof(space)-1);
	memset(space, ' ', namelen);
	space[namelen] = '\0';

	for (lang = 1; lang < ml->nlang && lang < langs.nused; lang++) {
		l = VARR_GET(&langs, lang);
		buf_printf(buf, FORMAT,
			   space, l->name, ml->u.lstr[lang]);
	}
}

static const char *smash_a(const char *s, int len)
{
	char buf[MAX_STRING_LENGTH];
	char *p = buf;

	if (len < 0 || len > sizeof(buf)-1)
		len = sizeof(buf)-1;

	while (p-buf < len && *s) {
		if (*s == '@' && *(s+1) == '@')
			s++;
		*p++ = *s++;
	}
	*p = '\0';
	return str_dup(buf);
}

static char *fix_mlstring(const char *s)
{
	char *p;
	static char buf[MAX_STRING_LENGTH*2];

	buf[0] = '\0';

	if (s == NULL)
		return buf;

	s = fix_string(s);
	while((p = strchr(s, '@')) != NULL) {
		*p = '\0';
		strnzcat(buf, s, sizeof(buf));
		strnzcat(buf, "@@", sizeof(buf));
		s = p+1;
	}
	strnzcat(buf, s, sizeof(buf));
	return buf;
}

static mlstring *mlstr_split(mlstring *ml)
{
	int lang;
	mlstring *res;

	if (ml != NULL && ml != &mlstr_empty && ml->ref < 2) 
		return ml;

	res = mlstr_new(NULL);

	if (ml == NULL || ml == &mlstr_empty) 
		return res;

	res->nlang = ml->nlang;
	ml->ref--;
	if (ml->nlang == 0) {
		res->u.str = str_dup(ml->u.str);
		return res;
	}

	res->u.lstr = malloc(sizeof(char*) * res->nlang);
	for (lang = 0; lang < res->nlang; lang++)
		res->u.lstr[lang] = str_dup(ml->u.lstr[lang]);
	return res;
}

