/*-
 * Copyright (c) 1999 SoG Development Team
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
 * $Id: name_edit.c,v 1.11 2003-09-29 23:11:50 fjoe Exp $
 */

#include <stdio.h>
#include <string.h>

#include <merc.h>
#include <sog.h>

/*---------------------------------------------------------------------------
 * name list stuff
 *
 * name list is simply string of names
 * separated by spaces. if name contains spaces itself it is enclosed
 * in single quotes
 *
 */

static bool name_edit(const char **nl, const char *name, int flags,
		      CHAR_DATA *ch, const char *editor_name);
static void cat_name(char *buf, const char *name, size_t len);

void cat_name(char *buf, const char *name, size_t len)
{
	bool have_spaces = strpbrk(name, " \t") != NULL;	// notrans

	if (buf[0])
		strlcat(buf, " ", len);
	if (have_spaces)
		strlcat(buf, "'", len);
	strlcat(buf, name, len);
	if (have_spaces)
		strlcat(buf, "'", len);
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
static bool name_edit(const char **nl, const char *name, int flags,
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
			if (ch) {
				act_puts("$t: name list too long",
					 ch, editor_name, NULL,
					 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE,
					 POS_DEAD);
			}
			return found;
		}
		cat_name(buf, name, sizeof(buf));
		if (ch) {
			act_puts("$t: $T: name added.",
				 ch, editor_name, name,
				 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
		}
	} else {
		if (!IS_SET(flags, NE_F_DELETE))
			return found;

		if (ch) {
			act_puts("$t: $T: name removed.",
				 ch, editor_name, name,
				 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
		}
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
		if (ch) {
			act_puts("$t: name list set to ALL.",
				 ch, editor_name, NULL,
				 TO_CHAR | ACT_NOTRANS, POS_DEAD);
		}
		return TRUE;
	}

	if (!str_cmp(name, "none")) {
		free_string(*nl);
		*nl = str_empty;
		if (ch) {
			act_puts("$t: name list reset.",
				 ch, editor_name, NULL,
				 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
		}
		return TRUE;
	}

	if (!str_cmp(*nl, "all")) {
		free_string(*nl);
		*nl = str_empty;
	}

	return name_edit(nl, name, NE_F_ADD | NE_F_DELETE, ch, editor_name);
}
