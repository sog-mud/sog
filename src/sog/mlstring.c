/*
 * $Id: mlstring.c,v 1.2 1998-07-11 20:55:13 fjoe Exp $
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

mlstring mlstr_empty = {
	{ "" },
	0
};

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
	if (*p != '@') {
		ml->nlang = 0;
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

		/* p points at lang id */
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
		s = strchr(q, '@');
		if (s == NULL)
			s = strchr(q, '\0');
		else
			*s++ = '\0';

		ml->u.lstr[lang] = str_dup(q);
	}

	/* some diagnostics */
	for (lang = 0; lang < nlang; lang++)
		if (ml->u.lstr[lang] == NULL)
			log_printf("mlstr_fread: lang %s: undefined",
				 lang_table[lang]);
	return ml;
}

void mlstr_fwrite(FILE *fp, const mlstring *ml)
{
	int lang;

	if (ml->nlang == 0) {
		fprintf(fp, "%s~\n", fix_string(ml->u.str));
		return;
	}

	for (lang = 0; lang < ml->nlang; lang++)
		fprintf(fp, "@%s %s",
			lang_table[lang], fix_string(ml->u.lstr[lang]));
	fputs("~\n", fp);
}

void mlstr_free(mlstring *ml)
{
	int lang;

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

mlstring *mlstr_dup(mlstring *ml)
{
	int lang;
	mlstring *ml_new;

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

char * mlstr_val(CHAR_DATA *ch, const mlstring *ml)
{
	int lang;

	if (ml->nlang == 0)
		return ml->u.str;
	if (ch->lang >= ml->nlang || ch->lang < 0)
		lang = 0;
	else
		lang = ch->lang;
	return ml->u.lstr[lang];
}

char * mlstr_mval(const mlstring *ml)
{
	if (ml->nlang == 0)
		return ml->u.str;
	return ml->u.lstr[0];
}

char** mlstr_convert(mlstring *ml, int newlang)
{
	char *old;
	int lang;

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
	*p = str_add(argument, "\n", NULL);
	**p = UPPER(**p);
	return TRUE;
}

void mlstr_buf(BUFFER *buf, const char *name, const mlstring *ml)
{
	char space[MAX_STRING_LENGTH];
	size_t namelen;
	int lang;
	static char FORMAT[] = "%s[%s] [%s]\n\r";

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

