/*
 * $Id: resource.c,v 1.31 1998-08-15 12:40:49 fjoe Exp $
 */

#include <limits.h>
#include <stdio.h>
#include <time.h>
#include "merc.h"
#include "resource.h"
#include "db.h"
#include "comm.h"
#include "log.h"
#include "interp.h"

#ifdef SUNOS
#	include <stdarg.h>
#	include "compat.h"
#endif

#ifdef SVR4
#	include "compat.h"
#endif

char BLANK_STRING[] = "{RBUG!!!{x";

struct msg {
	char** p;
	int sexdep;
};

static int nmsgid;
struct msg** msg_table;
char** lang_table;
int nlang;

enum {
	DEP_NONE,
	DEP_CHAR,
	DEP_VICTIM
};


#define FIX_SEX(sex) ((sex) >= SEX_FEMALE  ? SEX_FEMALE : \
		     (sex) <= SEX_NEUTRAL ?	SEX_NEUTRAL : \
						SEX_MALE)

char *vmsg(int msgid, CHAR_DATA *ch, CHAR_DATA *victim)
{
	struct msg *m;

	if (msgid >= nmsgid || ch->lang >= nlang)
		return BLANK_STRING;

	m = msg_table[ch->lang]+msgid;
	if (m->sexdep) {
		if (m->sexdep == DEP_VICTIM)
			ch = victim;
		return m->p[ch ? FIX_SEX(ch->sex) : SEX_NEUTRAL];
	}
	else
		return (char*)m->p;
}

int lang_lookup(const char *name)
{
	int lang;

	if (IS_NULLSTR(name))
		return -1;

	for (lang = 0; lang < nlang; lang++)
		if (str_cmp(lang_table[lang], name) == 0)
			return lang;
	return -1;
}

void do_lang(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_STRING_LENGTH];
	int lang;

	argument = one_argument(argument, arg);

	if (*arg == '\0') {
		if (ch->lang >= nlang) {
			log_printf("do_lang: %s: lang == %d\n",
				   ch->name, ch->lang);
			ch->lang = 0;
		}
		char_nprintf(ch, MSG_INTERFACE_LANGUAGE_IS, lang_table[ch->lang]);
		return;
	}

	lang = lang_lookup(arg);
	if (lang < 0) {
		char_nputs(MSG_LANG_USAGE_PRE, ch);
		for (lang = 0; lang < nlang; lang++)
			char_printf(ch, "%s%s",
				    lang == 0 ? "" : " | ", lang_table[lang]);
		char_nputs(MSG_LANG_USAGE_POST, ch);
		return;
	}

	ch->lang = lang;
	do_lang(ch, "");
	do_look(ch, "");
}

/*
 * msgdb implementation begins here
 */

#ifdef __FreeBSD__
#	include <sysexits.h>
#else
#	define EX_DATAERR	65
#	define EX_NOINPUT	66
#	define EX_OSERR		71
#endif

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "db.h"

struct msgid {
	char* name;
	int msgid;
};

static struct msgid *msgid_table;
static int nalloc;

#define MSGID_ALLOC_STEP 16

static char LANG_LST[] = "lang.lst";
static char RESOURCE_H[] = "resource.h";
static char STR_START[] = "enum {\n";
static char STR_END[] = "};\n";

#define BUFSZ 1024

static void lang_load(int lang, char* fname);

static void msgid_add(char* name, int msgid);
static msgid_lookup(char* name);
static msgid_cmp(const void*, const void*);
static char* name_lookup(int msgid);

void msgdb_load()
{
	int i;
	FILE *f;
	char buf[BUFSZ];
	int line = 0;
	int msgid = 0;
	int search_start = 1;

	/*
	 * load msg identifiers
	 */

	f = fopen(RESOURCE_H, "r");
	if (f == NULL) {
		perror(RESOURCE_H);
		exit(EX_NOINPUT);
	}

	while(fgets(buf, sizeof(buf), f) != NULL) {
		static char SEP[] = " \t,";
		char *p;

		line++;

		if (search_start) {
			if (strcmp(buf, STR_START) == 0)
				search_start = 0;
			continue;
		}

		if (strcmp(buf, STR_END) == 0)
			break;

		/* strip trailing '\n' and check if line is too long */
		p = strchr(buf, '\n');
		if (p == NULL) {
			fprintf(stderr, "%s:%d: line too long\n",
				RESOURCE_H, line);
			exit(EX_DATAERR);
		}
		else
			*p = '\0';

		/* skip separators */
		for (p = buf; *p && strchr(SEP, *p) != NULL; p++);
		if (*p == '\0')
			continue;

		msgid_add(strsep(&p, SEP), msgid++);
	}

	fclose(f);

	if (msgid_table == NULL) {
		fprintf(stderr, "%s: no msgids found\n", RESOURCE_H);
		exit(EX_DATAERR);
	}

	qsort(msgid_table, nmsgid, sizeof(*msgid_table), msgid_cmp);

	/*
	 * load language files
	 */
	f = fopen(LANG_LST, "r");
	if (f == NULL) {
		perror(LANG_LST);
		exit(EX_NOINPUT);
	}

	if (fscanf(f, "%d", &nlang) != 1 || nlang <= 0) {
		fprintf(stderr, "%s: syntax error\n", LANG_LST);
		exit(EX_DATAERR);
	}
	msg_table = alloc_perm(nlang * sizeof(*msg_table));
	lang_table = alloc_perm((nlang+1) * sizeof(*lang_table));
	lang_table[nlang] = NULL;

	for (i = 0; i < nlang; i++) {
		char buf2[BUFSZ];

		if (fgets(buf, sizeof(buf), f) == NULL) {
			fprintf(stderr, "%s: premature end of file\n",
				LANG_LST);
			exit(EX_DATAERR);
		}

		if (fscanf(f, "%s %s", buf, buf2) != 2) {
			fprintf(stderr, "%s: syntax error\n", LANG_LST);
			exit(EX_DATAERR);
		}
		lang_table[i] = str_dup(buf);
		lang_load(i, buf2);
	}

	for (i = 0; i < nmsgid; i++)
		free_string(msgid_table[i].name);
	free(msgid_table);

	fclose(f);
}

static
void
lang_load(int lang, char* fname)
{
	int i;
	int line = 0;
	int err = 0;
	struct msg *curr = NULL;
	int ncurr = 0;
	char buf[BUFSZ];
	FILE* f;

	f = fopen(fname, "r");
	if (f == NULL) {
		perror(fname);
		exit(EX_NOINPUT);
	}

	msg_table[lang] = alloc_perm(nmsgid * sizeof(**msg_table));
	memset(msg_table[lang], 0, nmsgid * sizeof(**msg_table));

	while (fgets(buf, sizeof(buf), f)) {
		char* p;
		static char WS[] = " \t";

		/* strip trailing '\n' and check if line is too long */
		line++;
		p = strchr(buf, '\n');
		if (p == NULL) {
			fprintf(stderr, "%s:%d: line too long\n", fname, line);
			exit(EX_DATAERR);
		}
		else
			*p = '\0';

		/* skip leading spaces */
		for (p = buf; *p && strchr(WS, *p) != NULL; p++);
		if (*p == '\0')
			continue;

		/* skip comment lines */
		if (*p == '#')
			continue;

		if (isalpha(*p)) {
			char* name;
			int msgid;

			name = strsep(&p, WS);
			if ((msgid = msgid_lookup(name)) < 0) {
				fprintf(stderr, "%s:%d: '%s': unknown "
						"identifier\n",
					fname, line, name);
				exit(EX_DATAERR);
			}
			curr = msg_table[lang] + msgid;

			if (curr->p != NULL) {
				fprintf(stderr, "%s:%d: '%s' redefined\n",
					fname, line, name);
				exit(EX_DATAERR);
			}
		
			if (p != NULL) {
				while (*p && strchr(WS, *p) != NULL)
					p++;
				if (*p == '\0')
					p = NULL;
			}
			if (p == NULL || *p == '"') {
				curr->sexdep = 0;
				curr->p = NULL;
			}
			else {
				char* depname;

				depname = strsep(&p, WS);
				if (strcmp(depname, "sexdep") == 0)
					curr->sexdep = DEP_CHAR;
				else if (strcmp(depname, "vsexdep") == 0)
					curr->sexdep = DEP_VICTIM;
				else {
					fprintf(stderr,
						"%s:%d: invalid "
						"sex dependancy\n",
						fname, line);
					exit(EX_DATAERR);
				}
					
				if (p != NULL) {
					while (*p && strchr(WS, *p) != NULL)
						p++;
					if (*p == '\0')
						p = NULL;
				}

				ncurr = 0;
				curr->p = alloc_perm(SEX_MAX*sizeof(char*));
				for (i = 0; i < SEX_MAX; i++)
					curr->p[i] = NULL;

			}
			if (p == NULL)
				continue;
		}

		if (*p == '"') {
			char* q;

			if (curr == NULL
			||  (curr->sexdep == 0 && curr->p)
			||  (curr->sexdep && ncurr >= SEX_MAX)) {
				fprintf(stderr, "%s:%d: unexpected msg\n",
					fname, line);
				exit(EX_DATAERR);
			}

			for(q = p+1; *q; q++) 
				if (*q == '\\') {
					int c;

					if (*++q == '\0')
						break;

					switch (*q) {
					case 'n':
						c = '\n';
						break;
					case 'r':
						c = '\r';
						break;
					case 'a':
						c = '\a';
						break;
					default:
						c = '\0';
						break;
					}

					if (c) {
						memcpy(q, q+1, strlen(q+1) + 1);
						*--q = c;
					}

					continue;
				}
				else if (*q == '"')
					break;
			
			if (*q == '\0') {
				fprintf(stderr, "%s:%d: missing '\"' \n",
					fname, line);
				exit(EX_DATAERR);
			}

			*q = '\0';
			q = str_dup(p+1);
			if (curr->sexdep)
				curr->p[ncurr++] = q;
			else
				(char*) curr->p = q;
		}
		else {
			fprintf(stderr, "%s:%d: syntax error\n", fname, line);
			exit(EX_DATAERR);
		}
	}

	for (i = 0; i < nmsgid; i++) {
		int undefined = 0;
		struct msg *m = msg_table[lang] + i;

		if (m->sexdep) {
			int j;

			for (j = 0; j < SEX_MAX; j++)
				if (m->p[j] == NULL) {
					undefined = 1;
					break;
				}
		}
		else if (m->p == NULL) 
			undefined = 1;
		if (undefined) {
			fprintf(stderr, "%s: %s undefined\n",
				fname, name_lookup(i));
			err = 1;
		}
	}

	fclose(f);

	if (err)
		exit(EX_DATAERR);
}

static
int
msgid_cmp(const void* p1, const void* p2)
{
	return strcmp(*(char**)p1, *(char**)p2);
}

static
int
msgid_lookup(char* name)
{
	struct msgid *m;
	m = bsearch(&name, msgid_table, nmsgid, sizeof(*msgid_table),
		    msgid_cmp);
	if (m == NULL)
		return -1;
	return m->msgid;
}

static
void
msgid_add(char* name, int msgid)
{
	if (nmsgid == nalloc) {
		struct msgid *m;

		nalloc += MSGID_ALLOC_STEP;
		m = realloc(msgid_table, nalloc * sizeof(*msgid_table));
		if (m == NULL) {
			perror("msgid_add");
			exit(EX_OSERR);
		}
		msgid_table = m;
	}
	msgid_table[nmsgid].name = str_dup(name);
	msgid_table[nmsgid].msgid = msgid;
	nmsgid++;
}

static
char*
name_lookup(int msgid)
{
	int i;

	for (i = 0; i < nmsgid; i++)
		if (msgid_table[i].msgid == msgid)
			return msgid_table[i].name;

	return NULL;
}
