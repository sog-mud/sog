/*
 * $Id: mlstring.c,v 1.5 1998-07-20 02:50:01 efdi Exp $
 */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "merc.h"
#include "db.h"
#include "resource.h"
#include "log.h"
#include "string_edit.h"
#include "buffer.h"
#include "mlstring.h"
#include "util.h"

/*
 * multi-language string implementation
 */

/*
 * multi-language string
 * if nlang == 0 the value is stored in u.str
 * otherwise the value is stored in array of strings u.lstr
 * the size of array is equal to 'nlang'
 */
struct mlstring {
	union {
		char* str;
		char** lstr;
	} u;
	int nlang;
};

static void smash_a(char *s);
static char* fix_mlstring(const char* s);

mlstring *mlstr_new(void)
{
	mlstring *ml;

	ml = alloc_mem(sizeof(*ml));
	ml->nlang = 0;
	ml->u.str = NULL;
	return ml;
}

mlstring *mlstr_fread(FILE *fp)
{
	mlstring *ml;
	char *p;
	char *s;
	int lang;

	ml = alloc_mem(sizeof(*ml));
	p = fread_string(fp);
	if (*p != '@' || *(p+1) == '@') {
		ml->nlang = 0;
		smash_a(p);
		ml->u.str = p;
		return ml;
	}

	ml->nlang = nlang;
	ml->u.lstr = alloc_mem(sizeof(char*) * nlang);
	for (lang = 0; lang < nlang; lang++)
		ml->u.lstr[lang] = NULL;

	s = p+1;
	while (*s) {
		char *q;

		/* s points at lang id */
		q = strchr(s, ' ');
		if (q == NULL)
			db_error("mlstr_fread", "no ` ' after `@' found");
		*q++ = '\0';

		lang = lang_lookup(s);
		if (lang < 0) 
			db_error("mlstr_fread",
				 "lang %s: unknown language", s); 
		if (ml->u.lstr[lang] != NULL)
			db_error("mlstr_fread", "lang %s: redefined", s);

		/* q points at msg */

		/* find next '@', skip "@@" */
		s = q;
		while (TRUE) {
			s = strchr(s, '@');
			if (s == NULL) {
				s = strchr(q, '\0');
				break;
			}
			if (*(s+1) != '@') {
				*s++ = '\0';
				break;
			}
			s += 2;
		}
		smash_a(q);
		ml->u.lstr[lang] = str_dup(q);
	}

	/* some diagnostics */
	for (lang = 0; lang < nlang; lang++)
		if (ml->u.lstr[lang] == NULL)
			log_printf("mlstr_fread: lang %s: undefined",
				 lang_table[lang]);
	free_string(p);
	return ml;
}

void mlstr_fwrite(FILE *fp, const char* name, const mlstring *ml)
{
	int lang;

	if (!ml)
		return;

	if (name != NULL)
		fprintf(fp, "%s ", name);

	if (ml->nlang == 0) {
		fprintf(fp, "%s~\n", fix_mlstring(ml->u.str));
		return;
	}

	for (lang = 0; lang < ml->nlang; lang++) 
		fprintf(fp, "@%s %s",
			lang_table[lang], fix_mlstring(ml->u.lstr[lang]));
	fputs("~\n", fp);
}

void mlstr_free(mlstring *ml)
{
	int lang;

	if (!ml)
		return;

	if (ml->nlang == 0) {
		free_string(ml->u.str);
		return;
	}

	for (lang = 0; lang < ml->nlang; lang++)
		free_string(ml->u.lstr[lang]);
	free_mem(ml->u.lstr, sizeof(char*) * ml->nlang);
	free_mem(ml, sizeof(*ml));
	ml->nlang = 0;
}

mlstring *mlstr_dup(const mlstring *ml)
{
	int lang;
	mlstring *ml_new;

	if (!ml)
		return 0;

	ml_new = alloc_mem(sizeof(*ml_new));
	ml_new->nlang = ml->nlang;
	if (ml->nlang == 0) {
		ml_new->u.str = str_dup(ml->u.str);
		return ml_new;
	}

	ml_new->u.lstr = alloc_mem(sizeof(char*) * ml->nlang);
	for (lang = 0; lang < ml->nlang; lang++)
		ml_new->u.lstr[lang] = str_dup(ml->u.lstr[lang]);
	return ml_new;
}

void mlstr_printf(mlstring *ml,...)
{
	char buf[MAX_STRING_LENGTH];
	va_list ap;

	va_start(ap, ml);

	if (!ml)
		return;

	if (ml->nlang == 0) {
		vsnprintf(buf, sizeof(buf), ml->u.str, ap);
		free_string(ml->u.str);
		ml->u.str = str_dup(buf);
	}
	else {
		int lang;

		for (lang = 0; lang < ml->nlang; lang++) {
			vsnprintf(buf, sizeof*buf, ml->u.lstr[lang], ap);
			free_string(ml->u.lstr[lang]);
			ml->u.lstr[lang] = str_dup(buf);
		}
	}
	va_end(ap);
}

char * mlstr_val(const mlstring *ml, int lang)
{
	if (!ml)
		return "";

	if (ml->nlang == 0)
		return ml->u.str;
	if (lang >= ml->nlang || lang < 0)
		lang = 0;
	return ml->u.lstr[lang];
}

bool mlstr_null(const mlstring *ml)
{
	char *mval = mlstr_mval(ml);
	return (mval == NULL) || (*mval == '\0');
}

int mlstr_cmp(const mlstring *ml1, const mlstring *ml2)
{
	int lang;
	int res;

	if (!ml1 || !ml2)
		return 0;

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

char** mlstr_convert(mlstring *ml, int newlang)
{
	char *old;
	int lang;

	if (!ml)
		return 0;

	if (newlang < 0) {
		/* convert to language-independent */
		if (ml->nlang) {
			old = ml->u.lstr[0];
			for (lang = 1; lang < ml->nlang; lang++)
				free_string(ml->u.lstr[lang]);
			free_mem(ml->u.lstr, sizeof(char*) * ml->nlang);
			ml->nlang = 0;
			ml->u.str = old;
		}
		return &ml->u.str;
	}

	/* convert to language-dependent */
	if (ml->nlang == 0) {
		old = ml->u.str;
		ml->nlang = nlang;
		ml->u.lstr = alloc_mem(sizeof(char*) * nlang);
		ml->u.lstr[0] = old;
		for (lang = 1; lang < nlang; lang++)
			ml->u.lstr[lang] = NULL;
	}
	return ml->u.lstr+newlang;
}

bool mlstr_append(CHAR_DATA *ch, mlstring *ml, const char *arg)
{
	int lang;

	lang = lang_lookup(arg);
	if (lang < 0 && str_cmp(arg, "all"))
		return FALSE;

	string_append(ch, mlstr_convert(ml, lang));
	return TRUE;
}

void mlstr_format(mlstring *ml)
{
	int lang;

	if (!ml)
		return;

	if (ml->nlang == 0) {
		ml->u.str = format_string(ml->u.str);
		return;
	}

	for (lang = 0; lang < ml->nlang; lang++)
		ml->u.lstr[lang] = format_string(ml->u.lstr[lang]);
}

bool mlstr_change(mlstring *ml, const char *argument)
{
	char arg[MAX_STRING_LENGTH];
	int lang;
	char **p;

	argument = one_argument(argument, arg);
	lang = lang_lookup(arg);
	if (lang < 0 && str_cmp(arg, "all"))
		return FALSE;

	p = mlstr_convert(ml, lang);
	free_string(*p);
	*p = str_dup(argument);
	return TRUE;
}

/*
 * The same as mlstr_change, but '\n' is appended and first symbol
 * is uppercased
 */
bool mlstr_change_desc(mlstring *ml, const char *argument)
{
	char arg[MAX_STRING_LENGTH];
	int lang;
	char **p;

	argument = one_argument(argument, arg);
	lang = lang_lookup(arg);
	if (lang < 0 && str_cmp(arg, "all"))
		return FALSE;

	p = mlstr_convert(ml, lang);
	free_string(*p);
	*p = str_add(argument, "\n\r", NULL);
	**p = UPPER(**p);
	return TRUE;
}

void mlstr_dump(BUFFER *buf, const char *name, const mlstring *ml)
{
	char space[MAX_STRING_LENGTH];
	size_t namelen;
	int lang;
	static char FORMAT[] = "%s[%s] [%s]\n\r";

	if (!ml)
		return;

	namelen = strlen(name);
	namelen = URANGE(0, namelen, sizeof(space)-1);
	memset(space, ' ', namelen);
	space[namelen] = '\0';

	if (ml->nlang == 0) {
		buf_printf(buf, FORMAT, name, "all", ml->u.str);
		return;
	}

	buf_printf(buf, FORMAT, name, lang_table[0], ml->u.lstr[0]);
	for (lang = 1; lang < ml->nlang; lang++)
		buf_printf(buf, FORMAT,
			   space, lang_table[lang], ml->u.lstr[lang]);
}

static void smash_a(char *s)
{
	while ((s = strchr(s, '@')) != NULL) {
		strcpy(s, s+1);
		if (*s == '\0')
			break;
		s++;
	}
}

static char * fix_mlstring(const char *s)
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

