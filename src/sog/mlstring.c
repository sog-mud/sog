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
 * $Id: mlstring.c,v 1.49 1999-12-20 08:31:21 fjoe Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "merc.h"
#include "string_edit.h"
#include "db.h"
#include "lang.h"

/*
 * multi-language string implementation
 */

static const char* smash_a(const char *s, int len);
static char* fix_mlstring(const char* s);

void mlstr_init(mlstring *mlp)
{
	mlp->u.str = NULL;
	mlp->nlang = 0;
}

void mlstr_init2(mlstring *mlp, const char *mval)
{
	mlp->u.str = str_dup(mval);
	mlp->nlang = 0;
}

void mlstr_destroy(mlstring *mlp)
{
	if (mlp == NULL)
		return;

	if (mlp->nlang == 0)
		free_string(mlp->u.str);
	else {
		int lang;

		for (lang = 0; lang < mlp->nlang; lang++)
			free_string(mlp->u.lstr[lang]);
		free(mlp->u.lstr);
	}
}

void mlstr_clear(mlstring *mlp)
{
	mlstr_destroy(mlp);
	mlstr_init(mlp);
}

void mlstr_fread(rfile_t *fp, mlstring *mlp)
{
	const char *p;
	const char *s;
	int lang;

	p = fread_string(fp);
	if (IS_NULLSTR(p)) {
		mlstr_init(mlp);
		return;
	}

	if (*p != '@' || *(p+1) == '@') {
		mlp->nlang = 0;
		mlp->u.str = smash_a(p, -1);
		free_string(p);
		return;
	}

	mlp->u.lstr = calloc(1, sizeof(char*) * langs.nused);
	mlp->nlang = langs.nused;

	s = p+1;
	for (;;) {
		const char *q;

		/* s points at lang id */
		q = strchr(s, ' ');
		if (q == NULL) {
			db_error("mlstr_fread", "no ` ' after `@' found");
			return;
		}

		if ((lang = lang_nlookup(s, q-s)) < 0) {
			db_error("mlstr_fread", "%s: unknown language", s);
			return;
		}

		if (mlp->u.lstr[lang] != NULL) {
			db_error("mlstr_fread", "lang %s: redefined", s);
			return;
		}

		/* find next '@', skip "@@" */
		for (s = ++q; (s = strchr(s, '@'));) {
			if (*(s+1) != '@')
				break;
			s += 2;
		}
		if (s == NULL)
			s = strchr(q, '\0');
		mlp->u.lstr[lang] = smash_a(q, s-q);
		if (!*s++)
			break;
	}

	free_string(p);
}

void mlstr_fwrite(FILE *fp, const char* name, const mlstring *mlp)
{
	int lang;

	if (!IS_NULLSTR(name)) {
		if (mlstr_null(mlp))
			return;
		fprintf(fp, "%s ", name);
	}

	if (!mlp) {
		fprintf(fp, "~\n");
		return;
	}

	if (mlp->nlang == 0) {
		fprintf(fp, "%s~\n", fix_mlstring(mlp->u.str));
		return;
	}

	for (lang = 0; lang < mlp->nlang && lang < langs.nused; lang++) {
		const char* p = mlp->u.lstr[lang];
		lang_t *l;

		if (IS_NULLSTR(p))
			continue;

		l = VARR_GET(&langs, lang);
		fprintf(fp, "@%s %s", l->name, fix_mlstring(p));
	}
	fputs("~\n", fp);
}

mlstring *
mlstr_cpy(mlstring *dst, const mlstring *src)
{
	mlstr_clear(dst);

	dst->nlang = src->nlang;
	if (src->nlang == 0)
		dst->u.str = str_dup(src->u.str);
	else {
		int lang;

		dst->u.lstr = calloc(1, sizeof(char*) * src->nlang);
		for (lang = 0; lang < src->nlang; lang++) {
			if (IS_NULLSTR(src->u.lstr[lang]))
				continue;
			dst->u.lstr[lang] = str_dup(src->u.lstr[lang]);
		}
	}

	return dst;
}

void mlstr_printf(mlstring *dst, const mlstring *format,...)
{
	char buf[MAX_STRING_LENGTH];
	va_list ap;

	mlstr_clear(dst);

	if (format == NULL)
		return;

	dst->nlang = format->nlang;

	va_start(ap, format);
	if (format->nlang == 0) {
		vsnprintf(buf, sizeof(buf), format->u.str, ap);
		dst->u.str = str_dup(buf);
	}
	else {
		int lang;

		dst->u.lstr = calloc(1, sizeof(char*) * format->nlang);
		for (lang = 0; lang < format->nlang; lang++) {
			if (IS_NULLSTR(format->u.lstr[lang]))
				continue;
			vsnprintf(buf, sizeof(buf), format->u.lstr[lang], ap);
			dst->u.lstr[lang] = str_dup(buf);
		}
	}
	va_end(ap);
}

int mlstr_nlang(const mlstring *mlp)
{
	if (mlp == NULL)
		return 0;
	return mlp->nlang;
}

const char * mlstr_val(const mlstring *mlp, int lang)
{
	const char *p;

	if (mlp == NULL)
		return str_empty;

	if (mlp->nlang == 0) {
		p = mlp->u.str;
		return (p ? p : str_empty);
	}

	if (lang >= mlp->nlang
	||  lang < 0
	||  IS_NULLSTR(mlp->u.lstr[lang])) {
		lang_t *l;

		if ((l = varr_get(&langs, lang))
		&&  l->slang_of >= 0
		&&  l->slang_of < mlp->nlang)
			lang = l->slang_of;
		else
			return str_empty;
	}

	p = mlp->u.lstr[lang];
	return (p ? p : str_empty);
}

bool mlstr_null(const mlstring *mlp)
{
	const char *mval = mlstr_mval(mlp);
	return IS_NULLSTR(mval);
}

static const char *
mlstr_valid_cb(int lang, const char **p, va_list ap)
{
	if (!IS_NULLSTR(*p) && !mem_is(*p, MT_STR))
		return *p;

	return NULL;
}

bool mlstr_valid(const mlstring *mlp)
{
	if (mlp->nlang < 0 || mlp->nlang > langs.nused)
		return FALSE;

	return mlstr_foreach((mlstring *) mlp, mlstr_valid_cb) == NULL;
}

int mlstr_cmp(const mlstring *mlp1, const mlstring *mlp2)
{
	int lang;
	int res;

	if (mlp1 == NULL)
		if (mlp2 == NULL)
			return 0;
		else
			return 1;
	else if (mlp2 == NULL)
		return -1;

	if (mlp1->nlang != mlp2->nlang)
		return  mlp1->nlang - mlp2->nlang;

	if (mlp1->nlang == 0)
		return str_cmp(mlp1->u.str, mlp2->u.str);

	for (lang = 0; lang < mlp1->nlang; lang++) {
		res = str_cmp(mlp1->u.lstr[lang], mlp2->u.lstr[lang]);
		if (res)
			return res;
	}

	return 0;
}

const char** mlstr_convert(mlstring *mlp, int newlang)
{
	const char *old;
	int lang;

	if (newlang < 0) {
		/* convert to language-independent */
		if (mlp->nlang) {
			old = mlp->u.lstr[0];
			for (lang = 1; lang < mlp->nlang; lang++)
				free_string(mlp->u.lstr[lang]);
			free(mlp->u.lstr);
			mlp->nlang = 0;
			mlp->u.str = old;
		}
		return &mlp->u.str;
	}

	/* convert to language-dependent */
	if (mlp->nlang == 0) {
		old = mlp->u.str;
		mlp->nlang = langs.nused;
		mlp->u.lstr = calloc(1, sizeof(char*) * langs.nused);
		mlp->u.lstr[0] = old;
	}
	return mlp->u.lstr + newlang;
}

bool mlstr_append(CHAR_DATA *ch, mlstring *mlp, const char *arg)
{
	int lang;

	lang = lang_lookup(arg);
	if (lang < 0 && str_cmp(arg, "all"))
		return FALSE;

	string_append(ch, mlstr_convert(mlp, lang));
	return TRUE;
}

const char *
mlstr_foreach(mlstring *mlp, 
	      const char * (*cb)(int lang, const char **p, va_list ap), ...)
{
	int lang;
	const char *rv = NULL;
	va_list ap;

	if (mlp == NULL)
		return rv;

	va_start(ap, cb);
	if (mlp->nlang == 0)
		rv = cb(0, &mlp->u.str, ap);
	else {
		for (lang = 0; lang < mlp->nlang; lang++) {
			if ((rv = cb(lang, mlp->u.lstr + lang, ap)) != NULL)
				break;
		}
	}
	va_end(ap);
	return rv;
}

bool mlstr_edit(mlstring *mlp, const char *argument)
{
	char arg[MAX_STRING_LENGTH];
	int lang;
	const char **p;

	argument = one_argument(argument, arg, sizeof(arg));
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
bool mlstr_editnl(mlstring *mlp, const char *argument)
{
	char arg[MAX_STRING_LENGTH];
	int lang;
	const char **p;

	argument = one_argument(argument, arg, sizeof(arg));
	lang = lang_lookup(arg);
	if (lang < 0 && str_cmp(arg, "all"))
		return FALSE;

	p = mlstr_convert(mlp, lang);
	free_string(*p);
	*p = str_printf("%s\n", argument);
	return TRUE;
}

void mlstr_dump(BUFFER *buf, const char *name, const mlstring *mlp)
{
	char space[MAX_STRING_LENGTH];
	size_t namelen;
	int lang;
	static char FORMAT[] = "%s[%s] [%s]\n";
	lang_t *l;

	if (mlp == NULL || mlp->nlang == 0) {
		buf_printf(buf, FORMAT, name, "all",
			   mlp == NULL ? "(null)" : mlp->u.str);
		return;
	}

	if (langs.nused == 0)
		return;

	l = VARR_GET(&langs, 0);
	buf_printf(buf, FORMAT, name, l->name, mlp->u.lstr[0]);

	if (langs.nused < 1)
		return;

	namelen = strlen(name);
	namelen = URANGE(0, namelen, sizeof(space)-1);
	memset(space, ' ', namelen);
	space[namelen] = '\0';

	for (lang = 1; lang < mlp->nlang && lang < langs.nused; lang++) {
		l = VARR_GET(&langs, lang);
		buf_printf(buf, FORMAT,
			   space, l->name, mlp->u.lstr[lang]);
	}
}

static const char *
cb_addnl(int lang, const char **p, va_list ap)
{
	char buf[MAX_STRING_LENGTH];
	size_t len;

	bool *pchanged;

	if (*p == NULL
	||  (len = strlen(*p)) == 0
	||  (*p)[len-1] == '\n')
		return NULL;

	snprintf(buf, sizeof(buf), "%s\n", *p);
	free_string(*p);
	*p = str_dup(buf);

	pchanged = va_arg(ap, bool *);
	*pchanged = TRUE;
	return NULL;
}

static const char *
cb_stripnl(int lang, const char **p, va_list ap)
{
	char buf[MAX_STRING_LENGTH];
	size_t len;

	bool *pchanged;

	if (*p == NULL
	||  (len = strlen(*p)) == 0
	||  (*p)[len-1] != '\n')
		return NULL;

	strnzncpy(buf, sizeof(buf), *p, len-1);
	free_string(*p);
	*p = str_dup(buf);

	pchanged = va_arg(ap, bool *);
	*pchanged = TRUE;
	return NULL;
}

bool mlstr_addnl(mlstring *mlp)
{
	bool changed = FALSE;
	mlstr_foreach(mlp, cb_addnl, &changed);
	return changed;
}

bool mlstr_stripnl(mlstring *mlp)
{
	bool changed = FALSE;
	mlstr_foreach(mlp, cb_stripnl, &changed);
	return changed;
}

void
gmlstr_init(gmlstr_t *gml)
{
	mlstr_init2(&gml->ml, str_empty);
	mlstr_init2(&gml->gender, str_empty);
}

gmlstr_t *
gmlstr_cpy(gmlstr_t *dst, const gmlstr_t *src)
{
	mlstr_cpy(&dst->ml, &src->ml);
	mlstr_cpy(&dst->gender, &src->gender);
	return dst;
}

void
gmlstr_destroy(gmlstr_t *gml)
{
	mlstr_destroy(&gml->ml);
	mlstr_destroy(&gml->gender);
}

static const char *smash_a(const char *s, int len)
{
	char buf[MAX_STRING_LENGTH];
	char *p = buf;
	const char *q = s;

	if (len < 0 || len > sizeof(buf)-1)
		len = sizeof(buf)-1;

	while (q-s < len && *q) {
		if (*q == '@' && *(q+1) == '@')
			q++;
		*p++ = *q++;
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
		strnzcat(buf, sizeof(buf), s);
		strnzcat(buf, sizeof(buf), "@@");
		s = p+1;
	}
	strnzcat(buf, sizeof(buf), s);
	return buf;
}

