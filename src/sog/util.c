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
 * $Id: util.c,v 1.46 2003-09-30 00:31:39 fjoe Exp $
 */

#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#if !defined (WIN32)
#include <unistd.h>
#endif

#include <merc.h>

static int number_mm(void);

FILE *
dfopen(const char *dir, const char *file, const char *mode)
{
	char name[PATH_MAX];
	FILE *f;
	snprintf(name, sizeof(name), "%s%c%s", dir, PATH_SEPARATOR, file);
	if ((f = fopen(name, mode)) == NULL)
		printlog(LOG_INFO, "%s: %s", name, strerror(errno));
	return f;
}

int
dunlink(const char *dir, const char *file)
{
	char name[PATH_MAX];
	snprintf(name, sizeof(name), "%s%c%s", dir, PATH_SEPARATOR, file);
	return unlink(name);
}

int
dstat(const char *dir, const char *file, struct stat *s)
{
	char name[PATH_MAX];
	snprintf(name, sizeof(name), "%s%c%s", dir, PATH_SEPARATOR, file);
	return stat(name, s);
}

int
d2rename(const char *dir1, const char *file1,
	 const char *dir2, const char *file2)
{
	int res;
	char name1[PATH_MAX];
	char name2[PATH_MAX];
	snprintf(name1, sizeof(name1), "%s%c%s", dir1, PATH_SEPARATOR, file1);
	snprintf(name2, sizeof(name2), "%s%c%s", dir2, PATH_SEPARATOR, file2);
#if defined (WIN32)
	res = unlink(name2);
	if (res == -1)
		printlog(LOG_INFO, "d2rename: can't delete file %s", name2);
#endif
	res = rename(name1, name2);
	if (res < 0)
		printlog(LOG_INFO, "d2rename: error renaming %s -> %s", name1, name2);
	return res;
}

bool
dfexist(const char *dir, const char *file)
{
	struct stat sb;
	char name[PATH_MAX];
	snprintf(name, sizeof(name), "%s%c%s", dir, PATH_SEPARATOR, file);
	return (stat(name, &sb) >= 0);
}

const char *
get_filename(const char *name)
{
	const char *p = (p = strrchr(name, PATH_SEPARATOR)) ? ++p : name;
	return str_dup(p);
}

int cmpint(const void *p1, const void *p2)
{
	return *(const int*) p1 - *(const int*) p2;
}

size_t
cstrlen(const char *cstr)
{
	size_t res;

	if (cstr == NULL)
		return 0;

	res = strlen(cstr);
	while ((cstr = strchr(cstr, '{')) != NULL && *(cstr + 1) != '\0') {
		if (*(cstr + 1) == '{')
			res--;
		else
			res -= 2;
		cstr += 2;
	}

	return res;
}

const char *
cstrfirst(const char *cstr)
{
	if (cstr == NULL)
		return NULL;

	for (; *cstr == '{'; cstr++) {
		if (*(cstr + 1) != '\0')
			cstr++;
	}
	return cstr;
}

void
cstrtoupper(char *cstr)
{
	cstr = CAST(char *, cstrfirst(cstr));
	*cstr = UPPER(*cstr);
}

/*
 * Returns an initial-capped string
 */
char *
capitalize(const char *str)
{
	static char strcap[MAX_STRING_LENGTH];
	int i;

	for (i = 0; str[i] != '\0'; i++)
		strcap[i] = LOWER(str[i]);
	strcap[i] = '\0';
	strcap[0] = UPPER(strcap[0]);
	return strcap;
}

#define NBUF 5
#define NBITS 32

char *
format_flags(flag_t flags)
{
	static int cnt;
	static char buf[NBUF][NBITS+1];
	int count, pos = 0;

	cnt = (cnt + 1) % NBUF;

	for (count = 0; count < NBITS;  count++) {
		if (IS_SET(flags, 1 << count)) {
			if (count < 26)
				buf[cnt][pos] = 'A' + count;
			else
				buf[cnt][pos] = 'a' + (count - 26);
			pos++;
		}
	}

	if (pos == 0)
		buf[cnt][pos++] = '0';

	buf[cnt][pos] = '\0';
	return buf[cnt];
}

const char*
strdump(const char *argument, int dump_level)
{
	static char buf[2][MAX_STRING_LENGTH];
	static int cnt;
	const char *i;
	size_t o;

	if (IS_NULLSTR(argument))
		return argument;

	if (dump_level == DL_NONE)
		return argument;

	cnt = (cnt + 1) % 2;
	for (o = 0, i = argument; o < sizeof(buf[cnt])-2 && *i; i++, o++) {
		switch (*i) {
		case '\a':
			if (dump_level < DL_ALL)
				break;
			buf[cnt][o++] = '\\';
			buf[cnt][o] = 'a';
			continue;
		case '\n':
			if (dump_level < DL_ALL)
				break;
			buf[cnt][o++] = '\\';
			buf[cnt][o] = 'n';
			continue;
		case '\r':
			if (dump_level < DL_ALL)
				break;
			buf[cnt][o++] = '\\';
			buf[cnt][o] = 'r';
			continue;
		case '\\':
			if (dump_level < DL_ALL)
				break;
			buf[cnt][o++] = '\\';
			break;
		case '{':
			buf[cnt][o++] = *i;
			break;
		}
		buf[cnt][o] = *i;
	}
	buf[cnt][o] = '\0';

	return buf[cnt];
}

/*
 * Return true if an argument is completely numeric.
 */
bool
is_number(const char *argument)
{
	if (IS_NULLSTR(argument))
		return FALSE;

	if (*argument == '+' || *argument == '-')
		argument++;

	for (; *argument != '\0'; argument++) {
		if (!isdigit(*argument))
			return FALSE;
	}

	return TRUE;
}

static uint
x_argument(const char *argument, int c, char *arg, size_t len)
{
	char *p;
	char *q;
	int number;

	if (IS_NULLSTR(argument)) {
		arg[0] = '\0';
		return 0;
	}

	p = strchr(argument, c);
	if (p == NULL) {
		strlcpy(arg, argument, len);
		return 1;
	}

	number = strtoul(argument, &q, 0);
	if (q == p)
		argument = p+1;
	else
		number = 1;
	strlcpy(arg, argument, len);
	return number;
}

/*
 * Given a string like 14.foo, return 14 and 'foo'
 */
uint
number_argument(const char *argument, char *arg, size_t len)
{
	return x_argument(argument, '.', arg, len);
}

/*
 * Given a string like 14*foo, return 14 and 'foo'
 */
uint
mult_argument(const char *argument, char *arg, size_t len)
{
	return x_argument(argument, '*', arg, len);
}

/*
 * Pick off one argument from a string and return the rest.
 * Understands quotes.
 */
const char *
one_argument(const char *argument, char *arg_first, size_t len)
{
	return first_arg(argument, arg_first, len, TRUE);
}

/*****************************************************************************
 Name:		first_arg
 Purpose:	Pick off one argument from a string and return the rest.
		Understands quotes, if fCase then arg_first will be lowercased
 Called by:	string_add(string.c)
 ****************************************************************************/
const char *
first_arg(const char *argument, char *arg_first, size_t len, bool fCase)
{
	char *q;
	char cEnd = '\0';

	if (IS_NULLSTR(argument)) {
		arg_first[0] = '\0';
		return argument;
	}

/* skip leading spaces */
	while (isspace(*argument))
		argument++;

/* check quotes */
	if ((argument[0] == '\'' || argument[0] == '"')
	&&  argument[1] != '\0')
		cEnd = *argument++;

	for (q = arg_first; *argument && q + 1 < arg_first + len; argument++) {
		if ((!cEnd && isspace(*argument)) || *argument == cEnd) {
			argument++;
			break;
		}
		*q++ = fCase ? LOWER(*argument) : *argument;
	}
	*q = '\0';

	while (isspace(*argument))
		argument++;

	return argument;
}

/*
 * See if a string is one of the names of an object.
 */
bool
_is_name(const char *str, const char *namelist,
	 int (*cmpfun)(const char *, const char *))
{
	char name[MAX_INPUT_LENGTH], part[MAX_INPUT_LENGTH];
	const char *list, *string;

	if (IS_NULLSTR(namelist) || IS_NULLSTR(str))
		return FALSE;

	if (!str_cmp(namelist, "all"))
		return TRUE;

	string = str;
	/* we need ALL parts of string to match part of namelist */
	for (; ;) { /* start parsing string */
		str = one_argument(str, part, sizeof(part));

		if (part[0] == '\0')
			return TRUE;

		/* check to see if this is part of namelist */
		list = namelist;
		for (; ;) { /* start parsing namelist */
			list = one_argument(list, name, sizeof(name));
			if (name[0] == '\0')  /* this name was not found */
				return FALSE;

			if (!cmpfun(string, name))
				return TRUE; /* full pattern match */

			if (!cmpfun(part, name))
				break;
		}
	}
}

char *
strtime(time_t tm)
{
	char *p = ctime(&tm);
	p[24] = '\0';
	return p;
}

/*
 * Simple linear interpolation.
 */
int
interpolate(int level, int value_00, int value_32)
{
	return value_00 + level * (value_32 - value_00) / 32;
}

/*
 * Stick a little fuzz on a number.
 */
int
number_fuzzy(int number)
{
	switch (number_bits(2))
	{
	case 0:  number -= 1; break;
	case 3:  number += 1; break;
	}

	return UMAX(1, number);
}

/*
 * Generate a random number.
 */
int
number_range(int from, int to)
{
	int power;
	int number;

	if (from == 0 && to == 0)
		return 0;

	if ((to = to - from + 1) <= 1)
		return from;

	for (power = 2; power < to; power <<= 1)
		;

	while ((number = number_mm() & (power - 1)) >= to)
		;

	return from + number;
}

/*
 * Generate a percentile roll.
 */
int
number_percent(void)
{
	int percent;

	while ((percent = number_mm() & (128-1)) > 99)
		;

	return 1 + percent;
}

/*
 * Generate a random door.
 */
int
number_door(void)
{
	int door;

	while ((door = number_mm() & (8-1)) > 5)
		;

	return door;
}

int
number_bits(int width)
{
	return number_mm() & ((1 << width) - 1);
}

/*
 * Roll some dice.
 */
int
dice(int number, int size)
{
	int idice;
	int sum;

	switch (size)
	{
	case 0: return 0;
	case 1: return number;
	}

	for (idice = 0, sum = 0; idice < number; idice++)
		sum += number_range(1, size);

	return sum;
}

/*
 * Roll some dice with luck bonus.
 * ch wants roll to be bigger, victim wants roll to be smaller.
 */
int
dice_wlb(int number, int size, CHAR_DATA *ch, CHAR_DATA *victim)
{
	int idice;
	int sum;

	int luck_diff;

	switch (size) {
	case 0:
		return 0;
	case 1:
		return number;
	}

	if ((ch != NULL) && IS_AFFECTED(ch, AFF_DOOMED))
		return number;

	if ((victim != NULL) && IS_AFFECTED(victim, AFF_DOOMED))
		return number * size;

	luck_diff = ((ch == NULL) ? 50 : GET_LUCK(ch)) -
		 ((victim == NULL) ? 50 : GET_LUCK(victim));


	for (idice = 0, sum = 0; idice < number; idice++) {
		int cand;
		int num;
		if (luck_diff >= 0) {
			num = luck_diff / 20 + 1 +
				(number_range(0, 19) < luck_diff % 20) ? 1 : 0;

			for (cand = number_range(1, size); num > 0; num--) {
				int this_die = number_range(1, size);
				cand = UMAX(cand, this_die);
			}
		} else {
			num = (-luck_diff) / 20 + 1 +
				(number_range(0, 19) < (-luck_diff) % 20) ? 1 : 0;

			for (cand = number_range(1, size); num > 0; num--) {
				int this_die = number_range(1, size);
				cand = UMIN(cand, this_die);
			}
		}
		sum += cand;
	}
	return sum;
}

/*
 * I've gotten too many bad reports on OS-supplied random number generators.
 * This is the Mitchell-Moore algorithm from Knuth Volume II.
 * Best to leave the constants alone unless you've read Knuth.
 * -- Furey
 */

/* I noticed streaking with this random number generator, so I switched
	back to the system srandom call.  If this doesn't work for you,
	define OLD_RAND to use the old system -- Alander */

#if defined (OLD_RAND)
static  int     rgiState[2+55];
#endif

static void
init_mm(void)
{
#if defined (OLD_RAND)
	int *piState;
	int iState;

	piState     = &rgiState[2];

	piState[-2] = 55 - 55;
	piState[-1] = 55 - 24;

	piState[0]  = current_time & ((1 << 30) - 1);
	piState[1]  = 1;
	for (iState = 2; iState < 55; iState++)
	{
		piState[iState] = (piState[iState-1] + piState[iState-2])
		                & ((1 << 30) - 1);
	}
#else
#if defined(BSD44)
	srandomdev();
#else
	srandom(time(NULL)^getpid());
#endif
#endif
	return;
}

#define MAX_RND_CNT	128

int max_rnd_cnt	= MAX_RND_CNT;
int rnd_cnt;

static int
number_mm(void)
{
#if defined (OLD_RAND)
	int *piState;
	int iState1;
	int iState2;
	int iRand;

	piState             = &rgiState[2];
	iState1             = piState[-2];
	iState2             = piState[-1];
	iRand               = (piState[iState1] + piState[iState2])
		                & ((1 << 30) - 1);
	piState[iState1]    = iRand;
	if (++iState1 == 55)
		iState1 = 0;
	if (++iState2 == 55)
		iState2 = 0;
	piState[-2]         = iState1;
	piState[-1]         = iState2;
	return iRand >> 6;
#else
	if (max_rnd_cnt > 0 && (rnd_cnt++ % max_rnd_cnt) == 0)
		init_mm();
	return random() >> 6;
#endif
}

void *
bsearch_lower(const void *key, const void *base, size_t nmemb, size_t size,
	      int (*cmpfun)(const void *, const void *))
{
	char *p = bsearch(key, base, nmemb, size, cmpfun);

	if (p == NULL)
		return p;

	while (p > (const char *) base) {
		if (!!cmpfun(key, p - size))
			break;

		p -= size;
	}

	return p;
}
