/*
 * $Id: string_edit.c,v 1.60 2004-06-28 19:21:08 tatyana Exp $
 */

/***************************************************************************
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was freely distributed with the The Isles 1.1 source code,   *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 *                                                                         *
 ***************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <merc.h>
#include <lang.h>
#include <mprog.h>

#include <sog.h>

#include "comm.h"

static char *numlines(const char *, int dump_level);

#define ACT_SEDIT_DUMP	(ACT_SEDIT | ACT_NOTRANS | ACT_NOFIXSH)

/*****************************************************************************
 Name:		string_replace
 Purpose:	Substitutes one string for another.
 ****************************************************************************/
const char *
string_replace(const char *orig,
	       const char *old_text, const char *new_text, int flags)
{
	char xbuf[MAX_STRING_LENGTH];
	char *p;

	if (IS_NULLSTR(old_text) || !str_cscmp(old_text, new_text))
		return orig;

	xbuf[0] = '\0';
	while ((p = strstr(orig, old_text)) != NULL) {
		/*
		 * cat prefix
		 */
		if (p > orig)
			strlncat(xbuf, orig, sizeof(xbuf), p - orig);

		/*
		 * cat replacement
		 */
		strlcat(xbuf, new_text, sizeof(xbuf));

		/*
		 * move pointers
		 */
		orig = p + strlen(old_text);
		if (!IS_SET(flags, SR_F_ALL))
			break;
	}

	/*
	 * cat the rest
	 */
	strlcat(xbuf, orig, sizeof(xbuf));

	free_string(orig);
	return str_dup(xbuf);
}

/*
 * Thanks to Kalgen for the new procedure (no more bug!)
 * Original wordwrap() written by Surreality.
 */
/*****************************************************************************
 Name:		format_string
 Purpose:	Special string formating and word-wrapping.
 ****************************************************************************/
const char *
format_string(const char *oldstring)
{
	char xbuf[MAX_STRING_LENGTH];
	char xbuf2[MAX_STRING_LENGTH];
	const char *rdesc;
	uint i = 0;
	bool cap = TRUE;

	if (IS_NULLSTR(oldstring))
		return oldstring;

	xbuf[0] = xbuf2[0] = 0;

	for (rdesc = oldstring; *rdesc; rdesc++) {
		if (*rdesc == '\n') {
			if (xbuf[i-1] != ' ') {
				xbuf[i] = ' ';
				i++;
			}
		} else if (*rdesc == '\r')
			;
		else if (*rdesc == ' ') {
			if (xbuf[i-1] != ' ') {
				xbuf[i] = ' ';
				i++;
			}
		} else if (*rdesc == ')') {
			if (xbuf[i-1] == ' ' && xbuf[i-2] == ' '
			&&  (xbuf[i-3] == '.' || xbuf[i-3] == '?' ||
			     xbuf[i-3] == '!')) {
				xbuf[i-2] = *rdesc;
				xbuf[i-1]  = ' ';
				xbuf[i] = ' ';
				i++;
			} else {
				xbuf[i] = *rdesc;
				i++;
			}
		} else if (*rdesc == '.' || *rdesc == '?' || *rdesc == '!') {
			if (xbuf[i-1] == ' ' && xbuf[i-2] == ' '
			&&  (xbuf[i-3] == '.' || xbuf[i-3] == '?' ||
			     xbuf[i-3] == '!')) {
				xbuf[i-2] = *rdesc;
				if (*(rdesc+1) != '\"') {
					xbuf[i-1]=' ';
					xbuf[i]=' ';
					i++;
				} else {
					xbuf[i-1]='\"';
					xbuf[i]=' ';
					xbuf[i+1]=' ';
					i+=2;
					rdesc++;
				}
			} else {
				xbuf[i] = *rdesc;
				if (*(rdesc+1) != '\"') {
					xbuf[i+1] = ' ';
					xbuf[i+2] = ' ';
					i += 3;
				} else {
					xbuf[i+1] = '\"';
					xbuf[i+2] = ' ';
					xbuf[i+3] = ' ';
					i += 4;
					rdesc++;
				}
			}
			cap = TRUE;
		} else {
			xbuf[i] = *rdesc;
			if (cap) {
				cap = FALSE;
				xbuf[i] = UPPER(xbuf[i]);
			}
			i++;
		}
	}

	xbuf[i] = 0;
	strlcpy(xbuf2, xbuf, sizeof(xbuf2));

	rdesc = xbuf2;
	xbuf[0] = 0;

	for (; ;) {
		for (i = 0; i < 77; i++) {
			if (!*(rdesc+i))
				break;
		}

		if (i < 77)
			break;

		for (i = (xbuf[0] ? 76 : 73); i; i--) {
			if (*(rdesc+i) == ' ')
				break;
		}

		if (i) {
			strlncat(xbuf, rdesc, sizeof(xbuf), i);
			strlcat(xbuf, "\n", sizeof(xbuf));
			rdesc += i+1;
			while (*rdesc == ' ')
				rdesc++;
		} else {
			printlog(LOG_WARN, "format_string: no spaces");
			strlncat(xbuf, rdesc, sizeof(xbuf), 75);
			strlcat(xbuf, "-\n", sizeof(xbuf));	// notrans
			rdesc += 76;
		}
	}

	while (*(rdesc+i) &&
	       (*(rdesc+i) == ' '|| *(rdesc+i) == '\n'|| *(rdesc+i) == '\r'))
		i--;

	strlncat(xbuf, rdesc, sizeof(xbuf), i + 1);
	if (xbuf[strlen(xbuf)-2] != '\n')
		strlcat(xbuf, "\n", sizeof(xbuf));

	free_string(oldstring);
	return str_dup(xbuf);
}

/*****************************************************************************
 Name:		string_add
 Purpose:	Interpreter for string editing.
 Called by:	game_loop_xxxx(comm.c).
 ****************************************************************************/
void
string_add(CHAR_DATA *ch, const char *argument)
{
	const char *p;
	size_t len;
        char arg1[MAX_INPUT_LENGTH];

	if (IS_SET(ch->comm, COMM_MUDFTP_EDITOR)
	&&  !!str_cmp(argument, "@")
	&&  !!str_cmp(argument, "~")
	&&  !!str_cmp(argument, ":q")
	&&  !!str_cmp(argument, ":q!")
	&&  !!str_cmp(argument, ":x")
	&&  !!str_cmp(argument, ":h")) {
		act("Type :q to manually abort mudFTP mode.",
		    ch, NULL, NULL, TO_CHAR | ACT_SEDIT);
		act("If mudFTP is not supported by your client,",
		    ch, NULL, NULL, TO_CHAR | ACT_SEDIT);
		act("abort this edit and toggle mudftp off.",
		    ch, NULL, NULL, TO_CHAR | ACT_SEDIT | ACT_NOUCASE);
		return;
	}

	if (*argument == ':') {
		char arg2[MAX_INPUT_LENGTH];
		char arg3[MAX_INPUT_LENGTH];
		char tmparg3[MAX_INPUT_LENGTH];

		argument = first_arg(argument, arg1, sizeof(arg1), FALSE);
		argument = first_arg(argument, arg2, sizeof(arg2), FALSE);
		strlcpy(tmparg3, argument, sizeof(tmparg3));
		argument = first_arg(argument, arg3, sizeof(arg3), FALSE);

		/*
		 * clear text
		 */
		if (!str_cscmp(arg1+1, "c")) {
			act_puts("Text cleared.",
				 ch, NULL, NULL, TO_CHAR | ACT_SEDIT, POS_DEAD);
			free_string(*ch->desc->pString);
			*ch->desc->pString = str_dup(str_empty);
			return;
		}

		/*
		 * show text
		 */
		if (!str_cscmp(arg1+1, "s")) {
			act_puts("Text so far:\n"
				 "$t", ch,
				 numlines(*ch->desc->pString, DL_COLOR), NULL,
				 TO_CHAR | ACT_SEDIT_DUMP | ACT_NOLF, POS_DEAD);
			return;
		}

		/*
		 * preview text
		 */
		if (!str_cscmp(arg1+1, "p")) {
			act_puts("Text so far (preview):\n"
				 "$t{x", ch,
				 numlines(*ch->desc->pString, DL_NONE), NULL,
				 TO_CHAR | ACT_SEDIT_DUMP | ACT_NOLF, POS_DEAD);
			return;
		}

		/*
		 * replace
		 */
		if (!str_cmp(arg1+1, "r")) {
			if (arg2[0] == '\0') {
				arg1[2] = '\0';
				act_puts("Usage:  :$t \"old string\" \"new string\"",
					 ch, arg1+1, NULL,
					 TO_CHAR | ACT_SEDIT | ACT_NOTRANS, POS_DEAD);
				return;
			}

			*ch->desc->pString = string_replace(
			    *ch->desc->pString, arg2, arg3,
			    arg1[1] == 'r' ? 0 : SR_F_ALL);
			if (arg1[1] == 'r') {
				act_puts("'$t' replaced with '$T'.",
				    ch, strdump(arg2, DL_COLOR),
				    strdump(arg3, DL_COLOR),
				    TO_CHAR | ACT_SEDIT_DUMP, POS_DEAD);
			} else {
				act_puts("All occurences of '$t' replaced with '$T'.",
				    ch, strdump(arg2, DL_COLOR),
				    strdump(arg3, DL_COLOR),
				    TO_CHAR | ACT_SEDIT_DUMP, POS_DEAD);
			}
			return;
		}

		/*
		 * format
		 */
		if (!str_cscmp(arg1+1, "f")) {
			*ch->desc->pString = format_string(*ch->desc->pString);
			act_puts("String formatted.",
				 ch, NULL, NULL, TO_CHAR | ACT_SEDIT, POS_DEAD);
			return;
		}

		/*
		 * delete line
		 */
		 if (!str_cscmp(arg1+1, "ld")) {
			*ch->desc->pString = string_linedel(*ch->desc->pString,
							    atoi(arg2));
			act_puts("Line deleted.",
				 ch, NULL, NULL, TO_CHAR | ACT_SEDIT, POS_DEAD);
			return;
		}

		/*
		 * insert line
		 */
		if (!str_cscmp(arg1+1, "li")) {
			*ch->desc->pString = string_lineadd(*ch->desc->pString,
							   tmparg3, atoi(arg2));
			act_puts("Line inserted.",
				 ch, NULL, NULL, TO_CHAR | ACT_SEDIT, POS_DEAD);
			return;
		}

		/*
		 * replace line
		 */
		if (!str_cscmp(arg1+1, "lr")) {
			*ch->desc->pString = string_linedel(*ch->desc->pString,
							    atoi(arg2));
			*ch->desc->pString = string_lineadd(*ch->desc->pString,
							   tmparg3, atoi(arg2));
			act_puts("Line replaced.",
				 ch, NULL, NULL, TO_CHAR | ACT_SEDIT, POS_DEAD);
			return;
		}

		/*
		 * quit, do not save changes
		 */
		if (!str_cscmp(arg1+1, "q") || !str_cscmp(arg1+1, "q!")) {
			string_add_exit(ch, FALSE);
			return;
		}

		/*
		 * quit, save changes
		 */
		if (!str_cscmp(arg1+1, "x")) {
			string_add_exit(ch, TRUE);
			return;
		}

		/*
		 * help
		 */
		if (!str_cscmp(arg1+1, "h")) {
			/*
			 * use do_help here
			 */
			act_puts("Sedit help (commands on blank line):\n"
				 ":r 'old' 'new'   - replace a substring (first occurence)\n"
				 ":R 'old' 'new'   - replace a substring (all occurences)\n"
				 "                   (requires '', \"\")\n"
				 ":h               - get help (this info)\n"
				 ":s               - show text so far\n"
				 ":p               - preview text\n"
				 ":f               - format (word wrap) text\n"
				 ":c               - clear text so far\n"
				 ":ld <num>        - delete line #num\n"
				 ":li <num> <str>  - insert <str> before line #num\n"
				 ":lr <num> <str>  - replace line #num with <str>\n"
				 "@, ~, :x         - finish editing (save changes)\n"
				 ":q               - abort editing (do not save changes)",
				 ch, NULL, NULL, TO_CHAR | ACT_SEDIT, POS_DEAD);
			return;
		}

		act_puts("SEdit: Invalid command.",
			 ch, NULL, NULL, TO_CHAR | ACT_SEDIT, POS_DEAD);
		return;
	}

	if (*argument == '~' || *argument == '@') {
		string_add_exit(ch, TRUE);
		return;
	}

	if (*argument == '.')
		argument++;

	/*
	 * Truncate strings to MAX_STRING_LENGTH.
	 * --------------------------------------
	 */
	len = strlen(argument);
	if (strlen(*ch->desc->pString) + len >= (MAX_STRING_LENGTH - 4)) {
		act_puts("Text too long, last line skipped.",
			 ch, NULL, NULL, TO_CHAR | ACT_SEDIT, POS_DEAD);
		return;
	}

	p = *ch->desc->pString;
	if (argument[len-1] == '\\') {
		strlncpy(arg1, argument, sizeof(arg1), len - 1);
		*ch->desc->pString = str_printf(
		    "%s%s", *ch->desc->pString, arg1);		// notrans
	} else {
		*ch->desc->pString = str_printf(
		    "%s%s\n", *ch->desc->pString, argument);	// notrans
	}
	free_string(p);
}

void
string_add_exit(CHAR_DATA *ch, bool save)
{
	DESCRIPTOR_DATA *d = ch->desc;

	if (save) {
		free_string(d->backup);
		if (OLCED(ch) && olc_interpret)
			olc_interpret(d, "touch");
	} else {
		act_puts("No changes saved.",
			 ch, NULL, NULL, TO_CHAR | ACT_SEDIT, POS_DEAD);
		free_string(*d->pString);
		*d->pString = d->backup;
	}

	if (IS_EDIT2(ch, "mprogs")) {
		if (save) {
			mprog_t *mp = d->pEdit2;
			mp->status = MP_S_DIRTY;
			MPROG_COMPILE(ch, mp);
		}
		OLCED2(ch) = NULL;
		d->pEdit2 = NULL;
	}

	d->pString = NULL;
	if (IS_SET(ch->comm, COMM_QUIET_EDITOR))
		dofun("replay", ch, str_empty);
}

const char *
string_linedel(const char *string, int line)
{
	const char *strtmp = string;
	char buf[MAX_STRING_LENGTH];
	int cnt = 1, tmp = 0;

	buf[0] = '\0';

	for (; *strtmp != '\0'; strtmp++) {
		if (cnt != line)
			buf[tmp++] = *strtmp;

		if (*strtmp == '\n')
			cnt++;
	}

	buf[tmp] = '\0';

	free_string(string);
	return str_dup(buf);
}

const char *
string_lineadd(const char *string, const char *newstr, int line)
{
	const char *strtmp = string;
	int cnt = 1, tmp = 0;
	bool done = FALSE;
	char buf[MAX_STRING_LENGTH];

	buf[0] = '\0';

	if (newstr[0] == '.')
		newstr++;

	for (; *strtmp != '\0' || (!done && cnt == line); strtmp++) {
		if (cnt == line && !done) {
			strlcat(buf, newstr, sizeof(buf));
			strlcat(buf, "\n", sizeof(buf));
			tmp += strlen(newstr) + 1;
			cnt++;
			done = TRUE;
		}

		buf[tmp++] = *strtmp;

		if (done && *strtmp == '\0')
			break;

		if (*strtmp == '\n')
			cnt++;

		buf[tmp] = '\0';
	}

	free_string(string);
	return str_dup(buf);
}

/*****************************************************************************
 Name:		string_append
 Purpose:	Puts player into append mode for given string.
 ****************************************************************************/
void
string_append(CHAR_DATA *ch, const char **pString)
{
	if (ch->desc == NULL)
		return;

	if (*pString == NULL)
		*pString = str_dup(str_empty);

	ch->desc->pString = pString;
	ch->desc->backup = str_dup(*pString);

	if (IS_SET(ch->comm, COMM_MUDFTP_EDITOR)) {
		if (mudftp_push(ch->desc)) {
			act("Editing string via mudFTP push connection. Use :q to abort.",
			    ch, NULL, NULL, TO_CHAR | ACT_SEDIT);
			return;
		}
		act("Unable to initiate mudFTP push connection.",
		    ch, NULL, NULL, TO_CHAR | ACT_SEDIT);
		act("Toggle mudftp off and try again.",
		    ch, NULL, NULL, TO_CHAR | ACT_SEDIT);
		return;
	}

	act_puts("-=======- Entering APPEND Mode -========-\n"
		 "    Type :h on a new line for help\n"
		 " Terminate with a ~ or @ on a blank line.\n"
		 "-=======================================-\n"
		 "$t",
		 ch, numlines(*pString, DL_COLOR), NULL,
		 TO_CHAR | ACT_SEDIT_DUMP | ACT_NOLF, POS_DEAD);
}

bool
mlstr_append(CHAR_DATA *ch, mlstring *mlp, const char *arg)
{
	lang_t *lang;

	if (!str_cmp(arg, "reset")) {
		mlstr_convert(mlp, NULL);
		return TRUE;
	}

	if (!str_cmp(arg, "all")) {
		if (mlstr_nlang(mlp) > 0)
			return FALSE;
		lang = NULL;
	} else if ((lang = lang_lookup(arg)) == NULL)
		return FALSE;

	string_append(ch, mlstr_convert(mlp, lang));
	return TRUE;
}

/*--------------------------------------------------------------------
 * local functions
 */

/*
 * getline -- copy str to buf up to '\n', len is buf size
 */
static const char *
getline(const char *str, char *buf, size_t len)
{
	const char *p;

	p = strchr(str, '\n');
	if (!p)
		p = strchr(str, '\0');

	strlncpy(buf, str, len, p - str);

	if (*p == '\n') {
		p++;
		if (*p == '\r')
			p++;
	}

	return p;
}

static char *
numlines(const char *string, int dump_level)
{
	int cnt = 1;
	static char buf[MAX_STRING_LENGTH*2];
	char buf2[MAX_STRING_LENGTH], tmpb[MAX_STRING_LENGTH];

	buf[0] = '\0';

	while (*string) {
		string = getline(string, tmpb, sizeof(tmpb));
		snprintf(buf2, sizeof(buf2), "%2d. %s\n",	// notrans
			 cnt++, strdump(tmpb, dump_level));
		strlcat(buf, buf2, sizeof(buf));
	}

	return buf;
}
