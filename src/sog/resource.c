/*
 * $Id: resource.c,v 1.10 1998-05-05 01:18:45 fjoe Exp $
 */

#include <sys/time.h>
#include <stdio.h>
#include "merc.h"
#include "resource.h"
#include "db.h"

char BLANK_STRING[] = "{RBUG!!!{x";

struct msg {
	char** p;
	int sexdep;
};

static int nlang;
static int nmsgid;
static struct msg** lang_table;

#define FIX_SEX(ch) (ch->sex >= SEX_FEMALE  ?	SEX_FEMALE : \
		     ch->sex <= SEX_NEUTRAL ?	SEX_NEUTRAL : \
						SEX_MALE)

char *msg(int msgid, CHAR_DATA *ch)
{
	struct msg *m;

	if (msgid >= nmsgid || ch->i_lang >= nlang)
		return BLANK_STRING;

	m = lang_table[ch->i_lang]+msgid;
	if (m->sexdep)
		return m->p[FIX_SEX(ch)];
	else
		return (char*)m->p;
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
#include <sys/syslimits.h>
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

static void lang_load(int langnum, char* fname);

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
	lang_table = alloc_perm(nlang * sizeof(*lang_table));

	for (i = 0; i < nlang; i++) {
		if (fscanf(f, "%s", buf) != 1) {
			fprintf(stderr, "%s: syntax error\n", LANG_LST);
			exit(EX_DATAERR);
		}

		lang_load(i, buf);
	}

	for (i = 0; i < nmsgid; i++)
		free_string(msgid_table[i].name);
	free(msgid_table);

	fclose(f);
}

static
void
lang_load(int langnum, char* fname)
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

	lang_table[langnum] = alloc_perm(nmsgid * sizeof(**lang_table));

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
				fprintf(stderr, "%s:%d: '%s': unknown identifier\n",
					fname, line, name);
				exit(EX_DATAERR);
			}
			curr = lang_table[langnum] + msgid;

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
				curr->sexdep = 1;
				ncurr = 0;
				curr->p = alloc_perm((SEX_MAX+1)*sizeof(char*));
				for (i = 0; i <= SEX_MAX; i++)
					curr->p[i] = NULL;

				strsep(&p, WS);
				if (p != NULL) {
					while (*p && strchr(WS, *p) != NULL)
						p++;
					if (*p == '\0')
						p = NULL;
				}
			}
			if (p == NULL)
				continue;
		}

		if (*p == '"') {
			char* q;

			if (curr == NULL
			||  (curr->sexdep == 0 && curr->p)
			||  (curr->sexdep && ncurr > SEX_MAX)) {
				fprintf(stderr, "%s:%d: unexpected msg\n",
					fname, line);
				exit(EX_DATAERR);
			}

			for(q = p+1; *q; q++) 
				if (*q == '\\') {
					q++;
					if (*q == '\0')
						break;
					continue;
				}
				else if (*q == '"')
					break;
			
			if (*q == '\0') {
				fprintf(stderr, "%s:%d: unmatched '\"' \n",
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
		struct msg *m = lang_table[langnum] + i;

		if (m->sexdep) {
			int j;

			for (j = 0; j <= SEX_MAX; j++)
				if (m->p[j] == NULL) {
					undefined = 1;
					break;
				}
				else
					fprintf(stderr, "'%s': '%s'\n",
						name_lookup(i), m->p[j]);
		}
		else if (m->p == NULL) 
			undefined = 1;
		else
			fprintf(stderr, "'%s': '%s'\n",
				name_lookup(i), m->p);

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
