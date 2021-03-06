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
 * $Id: mlstring.c,v 1.74 2004-02-19 21:14:14 fjoe Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <merc.h>
#include <lang.h>
#include <rwfile.h>

/*
 * multi-language string implementation
 */

static const char* smash_a(const char *s, int len, bool smashdot);
static char* fix_mlstring(const char* s);

void
mlstr_init(mlstring *mlp)
{
	mlp->u.str = NULL;
	mlp->nlang = 0;
}

void
mlstr_init2(mlstring *mlp, const char *mval)
{
	mlp->u.str = str_dup(mval);
	mlp->nlang = 0;
}

void
mlstr_destroy(mlstring *mlp)
{
	if (mlp == NULL)
		return;

	if (mlp->nlang == 0)
		free_string(mlp->u.str);
	else {
		size_t lang;

		for (lang = 0; lang < mlp->nlang; lang++)
			free_string(mlp->u.lstr[lang]);
		free(mlp->u.lstr);
	}
}

void
mlstr_clear(mlstring *mlp)
{
	mlstr_destroy(mlp);
	mlstr_init(mlp);
}

void
mlstr_fread(rfile_t *fp, mlstring *mlp)
{
	const char *p;
	const char *s;
	size_t lang;
	lang_t *l;

	mlstr_clear(mlp);

	p = fread_string(fp);
	if (IS_NULLSTR(p))
		return;

	if (*p != '@' || *(p+1) == '@') {
		mlp->u.str = smash_a(p, -1, FALSE);
		free_string(p);
		return;
	}

	mlp->u.lstr = calloc(1, sizeof(char *) * c_size(&langs));
	mlp->nlang = c_size(&langs);

	s = p+1;
	for (;;) {
		const char *q;

		/* s points at lang id */
		q = strchr(s, ' ');
		if (q == NULL) {
			printlog(LOG_ERROR, "mlstr_fread: no ` ' after `@' found");
			return;
		}

		if ((l = lang_nlookup(s, q-s)) == NULL) {
			printlog(LOG_ERROR, "mlstr_fread: %s: unknown language", s);
			return;
		}
		lang = varr_index(&langs, l);

		if (mlp->u.lstr[lang] != NULL) {
			printlog(LOG_ERROR, "mlstr_fread: lang %s: redefined", s);
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
		mlp->u.lstr[lang] = smash_a(q, s-q, TRUE);
		if (!*s++)
			break;
	}

	free_string(p);
}

void
mlstr_fwrite(FILE *fp, const char* name, const mlstring *mlp)
{
	size_t lang;

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

	for (lang = 0; lang < mlp->nlang && lang < c_size(&langs); lang++) {
		const char* p = mlp->u.lstr[lang];
		lang_t *l;

		if (IS_NULLSTR(p))
			continue;

		l = VARR_GET(&langs, lang);
		fprintf(fp, "@%s %s", l->name, fix_mlstring(p));// notrans
	}
	fputs("~\n", fp);					// notrans
}

mlstring *
mlstr_cpy(mlstring *dst, const mlstring *src)
{
	mlstr_clear(dst);

	dst->nlang = src->nlang;
	if (src->nlang == 0)
		dst->u.str = str_dup(src->u.str);
	else {
		size_t lang;

		dst->u.lstr = calloc(1, sizeof(char*) * src->nlang);
		for (lang = 0; lang < src->nlang; lang++) {
			if (IS_NULLSTR(src->u.lstr[lang]))
				continue;
			dst->u.lstr[lang] = str_dup(src->u.lstr[lang]);
		}
	}

	return dst;
}

void
mlstr_printf(mlstring *dst, const mlstring *format,...)
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
		size_t lang;

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

int
mlstr_nlang(const mlstring *mlp)
{
	if (mlp == NULL)
		return 0;
	return mlp->nlang;
}

const char *
mlstr_val(const mlstring *mlp, size_t lang)
{
	const char *p;

	if (mlp == NULL)
		return str_empty;

	if (mlp->nlang == 0) {
		p = mlp->u.str;
		return (p ? p : str_empty);
	}

	if (lang >= mlp->nlang
	||  IS_NULLSTR(mlp->u.lstr[lang])) {
		lang_t *l;

		if ((l = varr_get(&langs, lang))
		&&  l->slang_of != NULL)
			lang = varr_index(&langs, l->slang_of);
		else
			return str_empty;
	}

	p = mlp->u.lstr[lang];
	return (p ? p : str_empty);
}

bool
mlstr_null(const mlstring *mlp)
{
	const char **p;

	MLSTR_FOREACH(p, mlp) {
		if (!IS_NULLSTR(*p))
			return FALSE;
	}

	return TRUE;
}

bool
mlstr_valid(const mlstring *mlp)
{
	const char **p;

	if (mlp->nlang > c_size(&langs))
		return FALSE;

	MLSTR_FOREACH(p, mlp) {
		if (!IS_NULLSTR(*p) && !mem_is(*p, MT_STR))
			return FALSE;
	}

	return TRUE;
}

int
mlstr_cmp(const mlstring *mlp1, const mlstring *mlp2)
{
	size_t lang;
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

const char **
mlstr_convert(mlstring *mlp, lang_t *newlang)
{
	const char *old;
	size_t lang;

	if (newlang == NULL) {
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
		mlp->nlang = c_size(&langs);
		mlp->u.lstr = calloc(1, sizeof(char *) * c_size(&langs));
		mlp->u.lstr[0] = old;
	}
	return mlp->u.lstr + varr_index(&langs, newlang);
}

const char **
mlstr_first(const mlstring *mlp)
{
	if (mlp == NULL)
		return NULL;

	if (mlp->nlang == 0)
		return &CAST(mlstring *, mlp)->u.str;

	return &mlp->u.lstr[0];
}

const char **
mlstr_next(const mlstring *mlp, const char **p)
{
	if (mlp == NULL)
		return NULL;

	if (mlp->nlang == 0)
		return NULL;

	if ((size_t) (p - mlp->u.lstr) < mlp->nlang - 1)
		return p + 1;

	return NULL;
}

uint
mlstr_lang(mlstring *mlp, const char **p)
{
	if (mlp->nlang == 0)
		return 0;

	return p - mlp->u.lstr;
}

bool
mlstr_edit(mlstring *mlp, const char *argument)
{
	char arg[MAX_STRING_LENGTH];
	lang_t *lang;
	const char **p;

	argument = one_argument(argument, arg, sizeof(arg));
	if (!str_cmp(arg, "reset")) {
		mlstr_convert(mlp, NULL);
		return TRUE;
	}

	if (!str_cmp(arg, "all")) {
		if (mlstr_nlang(mlp) > 0)
			return FALSE;
		lang = NULL;
	} else if ((lang = lang_lookup(arg)) == NULL)
		return FALSE;

	p = mlstr_convert(mlp, lang);
	free_string(*p);
	*p = str_dup(argument);
	return TRUE;
}

/*
 * The same as mlstr_edit, but '\n' is appended.
 */
bool
mlstr_editnl(mlstring *mlp, const char *argument)
{
	char arg[MAX_STRING_LENGTH];
	lang_t *lang;
	const char **p;

	argument = one_argument(argument, arg, sizeof(arg));
	if (!str_cmp(arg, "reset")) {
		mlstr_convert(mlp, NULL);
		return TRUE;
	}

	if (!str_cmp(arg, "all")) {
		if (mlstr_nlang(mlp) > 0)
			return FALSE;
		lang = NULL;
	} else if ((lang = lang_lookup(arg)) == NULL)
		return FALSE;

	p = mlstr_convert(mlp, lang);
	free_string(*p);
	*p = str_printf("%s\n", argument);
	return TRUE;
}

void
mlstr_dump(BUFFER *buf, const char *name, const mlstring *mlp, int dump_level)
{
	char space[MAX_STRING_LENGTH];
	size_t namelen;
	size_t lang;
	static char FORMAT[] = "%s[%s] [%s]\n";			// notrans
	lang_t *l;

	if (mlp == NULL || mlp->nlang == 0) {
		buf_printf(buf, BUF_END, FORMAT, name, "all",
		    mlp == NULL ?
			"(null)" :				// notrans
			strdump(mlp->u.str, dump_level));
		return;
	}

	if (c_isempty(&langs))
		return;

	l = VARR_GET(&langs, 0);
	buf_printf(buf, BUF_END, FORMAT,
		   name, l->name, strdump(mlp->u.lstr[0], dump_level));

	if (c_size(&langs) < 2)
		return;

	namelen = strlen(name);
	namelen = UMIN(namelen, sizeof(space)-1);
	memset(space, ' ', namelen);
	space[namelen] = '\0';

	for (lang = 1; lang < mlp->nlang && lang < c_size(&langs); lang++) {
		l = VARR_GET(&langs, lang);
		buf_printf(buf, BUF_END, FORMAT,
			   space, l->name,
			   strdump(mlp->u.lstr[lang], dump_level));
	}
}

bool
mlstr_addnl(mlstring *mlp)
{
	const char **p;
	bool changed = FALSE;

	MLSTR_FOREACH(p, mlp) {
		char buf[MAX_STRING_LENGTH];
		size_t len;

		if (*p == NULL
		||  (len = strlen(*p)) == 0
		||  (*p)[len-1] == '\n')
			continue;

		snprintf(buf, sizeof(buf), "%s\n", *p);
		free_string(*p);
		*p = str_dup(buf);

		changed = TRUE;
	}

	return changed;
}

bool
mlstr_stripnl(mlstring *mlp, size_t trailing_nls)
{
	const char **p;
	bool changed = FALSE;

	MLSTR_FOREACH(p, mlp) {
		char buf[MAX_STRING_LENGTH];
		size_t len, oldlen;

		if (*p == NULL
		||  (oldlen = len = strlen(*p)) == 0)
			continue;

		while ((*p)[len-1] == '\n') {
			if (len < (trailing_nls + 1)
			||  (*p)[len - (trailing_nls + 1)] != '\n')
				break;

			len--;
		}

		if (oldlen != len) {
			strlncpy(buf, *p, sizeof(buf), len);
			free_string(*p);
			*p = str_dup(buf);

			changed = TRUE;
		}
	}

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

static const char *
smash_a(const char *s, int len, bool smashdot)
{
	char buf[MAX_STRING_LENGTH];
	char *p = buf;
	const char *q = s;

	if (len < 0 || (size_t) (len + 1) > sizeof(buf))
		len = sizeof(buf)-1;

	if (smashdot && *q == '.')
		q++;

	while (q-s < len && *q) {
		if (*q == '@' && *(q+1) == '@')
			q++;
		*p++ = *q++;
	}
	*p = '\0';
	return str_dup(buf);
}

static char *
fix_mlstring(const char *s)
{
	char *p;
	static char buf[MAX_STRING_LENGTH*2];

	buf[0] = '\0';

	if (s == NULL)
		return buf;

	s = fix_string(s);
	while((p = strchr(s, '@')) != NULL) {
		*p = '\0';
		strlcat(buf, s, sizeof(buf));
		strlcat(buf, "@@", sizeof(buf));		// notrans
		s = p+1;
	}
	strlcat(buf, s, sizeof(buf));
	return buf;
}
