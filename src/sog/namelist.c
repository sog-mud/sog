/*-
 * Copyright (c) 1999 fjoe <fjoe@iclub.nsu.ru>
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
 * $Id: namelist.c,v 1.1 1999-10-06 09:56:08 fjoe Exp $
 */

#include <stdio.h>
#include <string.h>
#include "merc.h"

/*---------------------------------------------------------------------------
 * name list stuff
 *
 * name list is simply string of names
 * separated by spaces. if name contains spaces itself it is enclosed
 * in single quotes
 *
 */

/*
 * See if a string is one of the names of an object.
 */
bool is_name_raw(const char *str, const char *namelist,
		 int (*cmpfun)(const char*, const char*))
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

bool is_name(const char *str, const char *namelist)
{
	return is_name_raw(str, namelist, str_prefix);
}

void cat_name(char *buf, const char *name, size_t len)
{
	bool have_spaces = strpbrk(name, " \t") != NULL;

	if (buf[0])
		strnzcat(buf, len, " ");
	if (have_spaces)
		strnzcat(buf, len, "'");
	strnzcat(buf, len, name);
	if (have_spaces)
		strnzcat(buf, len, "'");
}

/* 
 * name_edit flags
 */
#define NE_F_DELETE	(A)	/* delete name if found		*/
#define NE_F_ADD	(B)	/* add name if not found	*/

/*
 * name_edit - edit 'name' according to 'flags' in name list pointed by 'nl'
 *             if ch == NULL name_edit will be silent
 *	       (and 'editor_name' is not used)
 * Return values: TRUE  - name was found in namelist
 *		  FALSE - name was not found
 *
 */
bool name_edit(const char **nl, const char *name, int flags,
	       CHAR_DATA *ch, const char *editor_name)
{
	bool found = FALSE;
	const char *p = *nl;
	char buf[MAX_STRING_LENGTH];

	buf[0] = '\0';
	for (;;) {
		char arg[MAX_STRING_LENGTH];

		p = first_arg(p, arg, sizeof(arg), FALSE);

		if (arg[0] == '\0')
			break;

		if (!str_cmp(name, arg)) {
			found = TRUE;
			if (IS_SET(flags, NE_F_DELETE))
				continue;
		}

		cat_name(buf, arg, sizeof(buf));
	}

	if (!found) {
		if (!IS_SET(flags, NE_F_ADD))
			return found;

		if (strlen(buf) + strlen(name) + 4 > MAX_STRING_LENGTH) {
			if (ch)
				char_printf(ch, "%s: name list too long\n",
					    editor_name);
			return found;
		}
		cat_name(buf, name, sizeof(buf));
		if (ch)
			char_printf(ch, "%s: %s: name added.\n",
				    editor_name, name);
	}
	else {
		if (!IS_SET(flags, NE_F_DELETE))
			return found;

		if (ch)
			char_printf(ch, "%s: %s: name removed.\n",
				    editor_name, name);
	}

	free_string(*nl);
	*nl = str_dup(buf);
	return found;
}

bool name_add(const char **nl, const char *name,
	      CHAR_DATA *ch, const char *editor_name)
{
	return name_edit(nl, name, NE_F_ADD, ch, editor_name);
}

bool name_delete(const char **nl, const char *name,
		 CHAR_DATA *ch, const char *editor_name)
{
	return name_edit(nl, name, NE_F_DELETE, ch, editor_name);
}

bool name_toggle(const char **nl, const char *name,
		 CHAR_DATA *ch, const char *editor_name)
{
	if (!str_cmp(name, "all")) {
		free_string(*nl);
		*nl = str_dup(name);
		if (ch)
			char_printf(ch, "%s: name list set to ALL.\n",
				    editor_name);
		return TRUE;
	}

	if (!str_cmp(name, "none")) {
		free_string(*nl);
		*nl = str_empty;
		if (ch)
			char_printf(ch, "%s: name list reset.\n", editor_name);
		return TRUE;
	}

	if (!str_cmp(*nl, "all")) {
		free_string(*nl);
		*nl = str_empty;
	}

	return name_edit(nl, name, NE_F_ADD | NE_F_DELETE, ch, editor_name);
}

