/*
 * $Id: mlstring.c,v 1.1 1998-07-10 10:39:40 fjoe Exp $
 */

#include <stdio.h>
#include <string.h>
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

mlstring * fread_mlstring(FILE *fp)
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
			db_error("fread_mlstring", "no ` ' after `@' found");
		*q++ = '\0';

		lang = lang_lookup(s);
		if (lang < 0) 
			db_error("fread_mlstring",
				 "lang %s: unknown language", s); 
		if (ml->u.lstr[lang] != NULL)
			db_error("fread_mlstring", "lang %s: redefined", s);

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
			log_printf("fread_mlstring: lang %s: undefined",
				 lang_table[lang]);
	return ml;
}

void fwrite_mlstring(FILE *fp, mlstring *ml)
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

void free_mlstring(mlstring *ml)
{
	int lang;

	if (ml->nlang == 0) {
		free_string(ml->u.str);
		return;
	}

	for (lang = 0; lang < ml->nlang; lang++)
		free_string(ml->u.lstr[lang]);
	free_mem(ml->u.lstr, sizeof(char*) * ml->nlang);
	ml->nlang = 0;
}

char * ml_string(CHAR_DATA *ch, mlstring *ml)
{
	if (ml->nlang == 0)
		return ml->u.str;
	return ml->u.lstr[URANGE(0, ch->lang, nlang-1)];
}

char * ml_estring(mlstring *ml)
{
	if (ml->nlang == 0)
		return ml->u.str;
	return ml->u.lstr[0];
}

char** mlstring_convert(mlstring *ml, int newlang)
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

bool mlstring_append(CHAR_DATA *ch, mlstring *ml, const char *arg)
{
	int lang;

	lang = lang_lookup(arg);
	if (lang < 0 && str_cmp(arg, "all"))
		return FALSE;

	string_append(ch, mlstring_convert(ml, lang));
	return TRUE;
}

void format_mlstring(mlstring *ml)
{
	int lang;

	if (ml->nlang == 0) {
		ml->u.str = format_string(ml->u.str);
		return;
	}

	for (lang = 0; lang < ml->nlang; lang++)
		ml->u.lstr[lang] = format_string(ml->u.lstr[lang]);
}

bool mlstring_change(mlstring *ml, char *argument)
{
	char arg[MAX_STRING_LENGTH];
	int lang;
	char **p;

	argument = one_argument(argument, arg);
	lang = lang_lookup(arg);
	if (lang < 0 && str_cmp(arg, "all"))
		return FALSE;

	p = mlstring_convert(ml, lang);
	free_string(*p);
	*p = str_dup(argument);
	return TRUE;
}

void mlstring_buf(BUFFER *buf, const char *name, mlstring *ml)
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

