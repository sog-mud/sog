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
 * $Id: help.c,v 1.18 2001-01-23 21:47:00 fjoe Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "typedef.h"
#include "merc.h"

HELP_DATA *		help_first;
HELP_DATA *		help_last;

extern int top_help;

HELP_DATA * help_new(void)
{
	top_help++;
	return calloc(1, sizeof(HELP_DATA));
}

void help_add(AREA_DATA *pArea, HELP_DATA* pHelp)
{
/* insert into global help list */
	if (help_first == NULL)
		help_first = pHelp;
	if (help_last != NULL)
		help_last->next = pHelp;
	help_last		= pHelp;
	pHelp->next		= NULL;
	
/* insert into help list for given area */
	if (pArea->help_first == NULL)
		pArea->help_first = pHelp;
	if (pArea->help_last != NULL)
		pArea->help_last->next_in_area = pHelp;
	pArea->help_last = pHelp;
	pHelp->next_in_area	= NULL;

/* link help with given area */
	pHelp->area		= pArea;
}

HELP_DATA *help_lookup(int num, const char *keyword)
{
	HELP_DATA *res;

	if (num <= 0 || IS_NULLSTR(keyword))
		return NULL;

	for (res = help_first; res != NULL; res = res->next)
		if (is_name(keyword, res->keyword) && !--num)
			return res;
	return NULL;
}

void help_show(CHAR_DATA *ch, BUFFER *output, const char *keyword)
{
	help_show_raw(ch->level, GET_LANG(ch), output, keyword);
}

void help_show_raw(int level, int lang, BUFFER *output, const char *keyword)
{
	HELP_DATA *pHelp;
	HELP_DATA *pFirst = NULL;
	bool topic_list = FALSE;
	int num = -1;
	char buf[MAX_STRING_LENGTH];

	if (strchr(keyword, '.')) {
		num = number_argument(keyword, buf, sizeof(buf));
		keyword = buf;
	}

	if (IS_NULLSTR(keyword)) 
		keyword = "SUMMARY";				// notrans

	for (pHelp = help_first; pHelp; pHelp = pHelp->next) {
		if (pHelp->level > level
		||  !is_name(keyword, pHelp->keyword)) 
			continue;

		if (pFirst == NULL) {
			if (num < 0) {
				pFirst = pHelp;
				continue;
			}

			if (!--num) {
				pFirst = pHelp;
				break;
			}
			continue;
		}

		/* found second matched help topic */
		if (!topic_list) {
			buf_append(output, "Available topics:\n");
			buf_printf(output, BUF_END,
			    "    o %s\n",			// notrans
			    pFirst->keyword);
			topic_list = TRUE;
		}
		buf_printf(output, BUF_END, "    o %s\n",	// notrans
		    pHelp->keyword);
	}

	if (pFirst == NULL) {
		buf_printf(output, BUF_END,
			   "%s: no help on that word.\n", keyword);
		return;
	}

	if (!topic_list) {
		const char *text;

		if (pFirst->level > -2
		&&  str_cmp(pFirst->keyword, "imotd")) {
			buf_printf(output, BUF_END,
			     "{C%s{x\n\n", pFirst->keyword);	// notrans
		}

		text = mlstr_val(&pFirst->text, lang);
		if (text)
			buf_printf(output, BUF_END, "%s\n", text);
	}
}

void help_free(HELP_DATA *pHelp)
{
	HELP_DATA *p;
	HELP_DATA *prev;
	AREA_DATA *pArea;

/* remove from global list */
	for (prev = NULL, p = help_first; p; p = p->next) {
		if (p == pHelp)
			break;
		prev = p;
	}
		
	if (p) {
		if (prev) 
			prev->next = pHelp->next;
		else
			help_first = help_first->next;
		if (help_last == pHelp)
			help_last = prev;
	}

/* remove from area */
	pArea = pHelp->area;
	for (prev = NULL, p = pArea->help_first; p; p = p->next_in_area) {
		if (p == pHelp)
			break;
		prev = p;
	}

	if (p) {
		if (prev)
			prev->next_in_area = pHelp->next_in_area;
		else
			pArea->help_first = pArea->help_first->next_in_area;
		if (pArea->help_last == pHelp)
			help_last = prev;
	}

/* free memory */
	free_string(pHelp->keyword);
	mlstr_destroy(&pHelp->text);
	free(pHelp);
	top_help--;
}

