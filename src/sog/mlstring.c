/*
 * $Id: mlstring.c,v 1.11 1998-08-15 07:47:34 fjoe Exp $
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
 * 'ref' = number of references (COW semantics)
 */
struct mlstring {
	union {
		char* str;
		char** lstr;
	} u;
	int nlang;
	int ref;
};

mlstring mlstr_empty;

static void smash_a(char *s);
static char* fix_mlstring(const char* s);
static mlstring *mlstr_new();
static mlstring *mlstr_split(mlstring *ml);

int mlstr_count;

mlstring *mlstr_fread(FILE *fp)
{
	char *p;
	char *s;
	int lang;
	mlstring *res = mlstr_new();

	p = fread_string(fp);
	if (*p != '@' || *(p+1) == '@') {
		res->nlang = 0;
		smash_a(p);
		res->u.str = p;
		return res;
	}

	res->u.lstr = alloc_mem(sizeof(char*) * nlang);
	for (lang = 0; lang < nlang; lang++)
		res->u.lstr[lang] = NULL;
	res->nlang = nlang;

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
		if (res->u.lstr[lang] != NULL)
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
		res->u.lstr[lang] = str_dup(q);
	}

	/* some diagnostics */
	for (lang = 0; lang < nlang; lang++)
		if (res->u.lstr[lang] == NULL)
			log_printf("mlstr_fread: lang %s: undefined",
				 lang_table[lang]);
	free_string(p);
	return res;
}

void mlstr_fwrite(FILE *fp, const char* name, const mlstring *ml)
{
	int lang;

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
	if (ml == NULL || ml == &mlstr_empty || !ml->ref || --ml->ref)
		return;

	if (ml->nlang == 0)
		free(ml->u.str);
	else {
		int lang;

		for (lang = 0; lang < ml->nlang; lang++)
			free(ml->u.lstr[lang]);
		free_mem(ml->u.lstr, sizeof(char*) * ml->nlang);
	}
	free_mem(ml, sizeof(*ml));
	mlstr_count--;
}

mlstring *mlstr_dup(mlstring *ml)
{
	if (ml == NULL)
		return NULL;

	if (ml == &mlstr_empty)
		return &mlstr_empty;

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

	res = mlstr_new();
	res->nlang = ml->nlang;

	va_start(ap, ml);
	if (ml->nlang == 0) {
		vsnprintf(buf, sizeof(buf), ml->u.str, ap);
		res->u.str = str_dup(buf);
	}
	else {
		int lang;

		for (lang = 0; lang < ml->nlang; lang++) {
			vsnprintf(buf, sizeof(buf), ml->u.lstr[lang], ap);
			res->u.lstr[lang] = str_dup(buf);
		}
	}
	va_end(ap);
	return res;
}

char * mlstr_val(const mlstring *ml, int lang)
{
	if (ml == NULL)
		return NULL;

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

char** mlstr_convert(mlstring **mlp, int newlang)
{
	char *old;
	int lang;

	*mlp = mlstr_split(*mlp);
	if (newlang < 0) {
		/* convert to language-independent */
		if ((*mlp)->nlang) {
			old = (*mlp)->u.lstr[0];
			for (lang = 1; lang < (*mlp)->nlang; lang++)
				free((*mlp)->u.lstr[lang]);
			free_mem((*mlp)->u.lstr, sizeof(char*) * (*mlp)->nlang);
			(*mlp)->nlang = 0;
			(*mlp)->u.str = old;
		}
		return &((*mlp)->u.str);
	}

	/* convert to language-dependent */
	if ((*mlp)->nlang == 0) {
		old = (*mlp)->u.str;
		(*mlp)->nlang = nlang;
		(*mlp)->u.lstr = alloc_mem(sizeof(char*) * nlang);
		(*mlp)->u.lstr[0] = old;
		for (lang = 1; lang < nlang; lang++)
			(*mlp)->u.lstr[lang] = NULL;
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

bool mlstr_change(mlstring **mlp, const char *argument)
{
	char arg[MAX_STRING_LENGTH];
	int lang;
	char **p;

	argument = one_argument(argument, arg);
	lang = lang_lookup(arg);
	if (lang < 0 && str_cmp(arg, "all"))
		return FALSE;

	p = mlstr_convert(mlp, lang);
	free(*p);
	*p = str_dup(argument);
	return TRUE;
}

/*
 * The same as mlstr_change, but '\n' is appended and first symbol
 * is uppercased
 */
bool mlstr_change_desc(mlstring **mlp, const char *argument)
{
	char arg[MAX_STRING_LENGTH];
	int lang;
	char **p;

	argument = one_argument(argument, arg);
	lang = lang_lookup(arg);
	if (lang < 0 && str_cmp(arg, "all"))
		return FALSE;

	p = mlstr_convert(mlp, lang);
	free(*p);
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

	if (ml == NULL || ml->nlang == 0) {
		buf_printf(buf, FORMAT, name, "all",
			   ml == NULL ? "(null)" : ml->u.str);
		return;
	}

	namelen = strlen(name);
	namelen = URANGE(0, namelen, sizeof(space)-1);
	memset(space, ' ', namelen);
	space[namelen] = '\0';

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

static mlstring *mlstr_new()
{
	mlstring *res = alloc_mem(sizeof(*res));
	res->ref = 1;
	mlstr_count++;
	return res;
}

static mlstring *mlstr_split(mlstring *ml)
{
	int lang;
	mlstring *res;

	if (ml != NULL && ml->ref < 2) 
		return ml;

	res = mlstr_new();

	if (ml == NULL) {
		res->u.str = NULL;
		res->nlang = 0;
		return res;
	}

	res->nlang = ml->nlang;
	ml->ref--;
	if (ml->nlang == 0) {
		res->u.str = str_dup(ml->u.str);
		return res;
	}

	res->u.lstr = alloc_mem(sizeof(char*) * res->nlang);
	for (lang = 0; lang < res->nlang; lang++)
		res->u.lstr[lang] = str_dup(ml->u.lstr[lang]);
	return res;
}

