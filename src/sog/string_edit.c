/*
 * $Id: string_edit.c,v 1.41 2000-10-04 20:28:53 fjoe Exp $
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

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "merc.h"
#include "string_edit.h"

char *numlines(const char *);

/*****************************************************************************
 Name:		string_append
 Purpose:	Puts player into append mode for given string.
 Called by:	(many)olc_act.c
 ****************************************************************************/
void string_append(CHAR_DATA *ch, const char **pString)
{
	act_char("-=======- Entering APPEND Mode -========-", ch);
	act_char("    Type :h on a new line for help", ch);
	act_char(" Terminate with a ~ or @ on a blank line.", ch);
	act_char("-=======================================-", ch);

	if (*pString == NULL)
		*pString = str_dup(str_empty);
	send_to_char(numlines(*pString), ch);

	ch->desc->pString = pString;
	ch->desc->backup = str_dup(*pString);
}

/*****************************************************************************
 Name:		string_replace
 Purpose:	Substitutes one string for another.
 ****************************************************************************/
const char *
string_replace(const char *orig, const char *old, const char *new, int flags)
{
	char xbuf[MAX_STRING_LENGTH];
	char *p;

	if (IS_NULLSTR(old) || !str_cscmp(old, new))
		return orig;

	xbuf[0] = '\0';
	while ((p = strstr(orig, old)) != NULL) {
		/*
		 * cat prefix
		 */
		if (p > orig)
			strnzncat(xbuf, sizeof(xbuf), orig, p - orig);

		/*
		 * cat replacement
		 */
		strnzcat(xbuf, sizeof(xbuf), new);

		/*
		 * move pointers
		 */
		orig = p + strlen(old);
		if (!IS_SET(flags, SR_F_ALL))
			break;
	}

	/*
	 * cat the rest
	 */
	strnzcat(xbuf, sizeof(xbuf), orig);

	free_string(orig);
	return str_dup(xbuf);
}

void string_add_exit(CHAR_DATA *ch, bool save)
{
	DESCRIPTOR_DATA *d = ch->desc;

	if (!save) {
		act_char("No changes saved.", ch);
		free_string(*d->pString);
		*d->pString = d->backup;
	} else {
		free_string(d->backup);
		if (OLCED(ch) && olc_interpret)
			olc_interpret(d, "touch");
	}

	d->pString = NULL;
	if (IS_SET(ch->comm, COMM_QUIET_EDITOR))
		dofun("replay", ch, str_empty);
}

/*****************************************************************************
 Name:		string_add
 Purpose:	Interpreter for string editing.
 Called by:	game_loop_xxxx(comm.c).
 ****************************************************************************/
void string_add(CHAR_DATA *ch, const char *argument)
{
	const char *p;
	size_t len;
        char arg1[MAX_INPUT_LENGTH];

	/*
	 * Thanks to James Seng
	*/

	if (*argument == ':') {
		char arg2[MAX_INPUT_LENGTH];
		char arg3[MAX_INPUT_LENGTH];
		char tmparg3[MAX_INPUT_LENGTH];

		argument = one_argument(argument, arg1, sizeof(arg1));
		argument = first_arg(argument, arg2, sizeof(arg2), FALSE);
		strnzcpy(tmparg3, sizeof(tmparg3), argument);
		argument = first_arg(argument, arg3, sizeof(arg3), FALSE);

		/*
		 * clear string
		 */
		if (!str_cscmp(arg1+1, "c")) {
			act_char("String cleared.", ch);
			free_string(*ch->desc->pString);
			*ch->desc->pString = str_dup(str_empty);
			return;
		}

		/*
		 * show string
		 */
		if (!str_cscmp(arg1+1, "s")) {
			char_printf(ch, "String so far:\n%s",
				    numlines(*ch->desc->pString));
			return;
		}

		/*
		 * replace
		 */
		if (!str_cmp(arg1+1, "r")) {
			if (arg2[0] == '\0') {
				char_printf(ch, "Usage:  :%c \"old string\" \"new string\"\n", arg1[1]);
				return;
			}

			*ch->desc->pString =
				string_replace(*ch->desc->pString, arg2, arg3,
					       arg1[1] == 'r' ? 0 : SR_F_ALL);
			char_printf(ch, "%s'%s' replaced with '%s'.\n",
				    arg1[1] == 'r' ? str_empty : "All ",
				    arg2, arg3);
			return;
		}

		/*
		 * format
		 */
		if (!str_cscmp(arg1+1, "f")) {
			*ch->desc->pString = format_string(*ch->desc->pString);
			act_char("String formatted.", ch);
			return;
		}

		/*
		 * delete line
		 */
		 if (!str_cscmp(arg1+1, "ld")) {
			*ch->desc->pString = string_linedel(*ch->desc->pString,
							    atoi(arg2));
			act_char("Line deleted.", ch);
			return;
		}

		/*
		 * insert line
		 */
		if (!str_cscmp(arg1+1, "li")) {
			*ch->desc->pString = string_lineadd(*ch->desc->pString,
							   tmparg3, atoi(arg2));
			act_char("Line inserted.", ch);
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
			act_char("Line replaced.", ch);
			return;
		}

		/*
		 * quit, do not save changes
		 */
		if (!str_cscmp(arg1+1, "q!")) {
			string_add_exit(ch, FALSE);
			return;
		}

		/*
		 * quit, save changes
		 */
		if (!str_cscmp(arg1+1, "x")
		||  !str_cscmp(arg1+1, "wq")) {
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
			act_char("Sedit help (commands on blank line):\n"
				 ":r 'old' 'new'   - replace a substring (first occurence)\n"
				 ":R 'old' 'new'   - replace a substring (all occurences)\n"
				 "                   (requires '', \"\")\n"
				 ":h               - get help (this info)\n"
				 ":s               - show string so far\n"
				 ":f               - (word wrap) string\n"
				 ":c               - clear string so far\n"
				 ":ld <num>        - delete line #num\n"
				 ":li <num> <str>  - insert <str> before line #num\n"
				 ":lr <num> <str>  - replace line #num with <str>\n"
				 "@, ~, :x, :wq    - finish editing (save changes)\n"
				 ":q!              - abort editing (do not save changes)", ch);
			return;
		}

		act_char("SEdit: Invalid command.", ch);
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
		act_char("String too long, last line skipped.", ch);
		return;
	}

	p = *ch->desc->pString;
	if (argument[len-1] == '\\') {
		strnzncpy(arg1, sizeof(arg1), argument, len-1);
		*ch->desc->pString = str_printf("%s%s",
						*ch->desc->pString, arg1);
	}
	else
		*ch->desc->pString = str_printf("%s%s\n",
						*ch->desc->pString, argument);
	free_string(p);
}

/*
 * Thanks to Kalgen for the new procedure (no more bug!)
 * Original wordwrap() written by Surreality.
 */
/*****************************************************************************
 Name:		format_string
 Purpose:	Special string formating and word-wrapping.
 Called by:	string_add(string.c) (many)olc_act.c
 ****************************************************************************/
const char *format_string(const char *oldstring /*, bool fSpace */)
{
  char xbuf[MAX_STRING_LENGTH];
  char xbuf2[MAX_STRING_LENGTH];
  const char *rdesc;
  int i=0;
  bool cap=TRUE;
  
	if (IS_NULLSTR(oldstring))
		return oldstring;

  xbuf[0]=xbuf2[0]=0;
  
  i=0;
  
  for (rdesc = oldstring; *rdesc; rdesc++)
  {
    if (*rdesc=='\n')
    {
      if (xbuf[i-1] != ' ')
      {
        xbuf[i]=' ';
        i++;
      }
    }
    else if (*rdesc=='\r') ;
    else if (*rdesc==' ')
    {
      if (xbuf[i-1] != ' ')
      {
        xbuf[i]=' ';
        i++;
      }
    }
    else if (*rdesc==')')
    {
      if (xbuf[i-1]==' ' && xbuf[i-2]==' ' && 
          (xbuf[i-3]=='.' || xbuf[i-3]=='?' || xbuf[i-3]=='!'))
      {
        xbuf[i-2]=*rdesc;
        xbuf[i-1]=' ';
        xbuf[i]=' ';
        i++;
      }
      else
      {
        xbuf[i]=*rdesc;
        i++;
      }
    }
    else if (*rdesc=='.' || *rdesc=='?' || *rdesc=='!') {
      if (xbuf[i-1]==' ' && xbuf[i-2]==' ' && 
          (xbuf[i-3]=='.' || xbuf[i-3]=='?' || xbuf[i-3]=='!')) {
        xbuf[i-2]=*rdesc;
        if (*(rdesc+1) != '\"')
        {
          xbuf[i-1]=' ';
          xbuf[i]=' ';
          i++;
        }
        else
        {
          xbuf[i-1]='\"';
          xbuf[i]=' ';
          xbuf[i+1]=' ';
          i+=2;
          rdesc++;
        }
      }
      else
      {
        xbuf[i]=*rdesc;
        if (*(rdesc+1) != '\"')
        {
          xbuf[i+1]=' ';
          xbuf[i+2]=' ';
          i += 3;
        }
        else
        {
          xbuf[i+1]='\"';
          xbuf[i+2]=' ';
          xbuf[i+3]=' ';
          i += 4;
          rdesc++;
        }
      }
      cap = TRUE;
    }
    else
    {
      xbuf[i]=*rdesc;
      if (cap)
        {
          cap = FALSE;
          xbuf[i] = UPPER(xbuf[i]);
        }
      i++;
    }
  }
  xbuf[i]=0;
  strnzcpy(xbuf2, sizeof(xbuf2), xbuf);
  
  rdesc=xbuf2;
  
  xbuf[0]=0;
  
  for (; ;)
  {
    for (i=0; i<77; i++)
    {
      if (!*(rdesc+i)) break;
    }
    if (i<77)
    {
      break;
    }
    for (i=(xbuf[0]?76:73) ; i ; i--)
    {
      if (*(rdesc+i)==' ') break;
    }
    if (i)
    {
      strnzncat(xbuf, sizeof(xbuf), rdesc, i);
      strnzcat(xbuf, sizeof(xbuf), "\n");
      rdesc += i+1;
      while (*rdesc == ' ') rdesc++;
    }
    else
    {
      log(LOG_WARN, "format_string: no spaces");
      strnzncat(xbuf, sizeof(xbuf), rdesc, 75);
      strnzcat(xbuf, sizeof(xbuf), "-\n");
      rdesc += 76;
    }
  }
  while (*(rdesc+i) && (*(rdesc+i)==' '||
                        *(rdesc+i)=='\n'||
                        *(rdesc+i)=='\r'))
    i--;
  strnzncat(xbuf, sizeof(xbuf), rdesc, i+1);
  if (xbuf[strlen(xbuf)-2] != '\n')
    strnzcat(xbuf, sizeof(xbuf), "\n");

  free_string(oldstring);
  return(str_dup(xbuf));
}

const char *string_linedel(const char *string, int line)
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

const char *string_lineadd(const char *string, char *newstr, int line)
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
			strnzcat(buf, sizeof(buf), newstr);
			strnzcat(buf, sizeof(buf), "\n");
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

/*
 * getline -- copy str to buf up to '\n', len is buf size
 */
const char *getline(const char *str, char *buf, size_t len)
{
	const char *p;

	p = strchr(str, '\n');
	if (!p) 
		p = strchr(str, '\0');

	strnzncpy(buf, len, str, p - str);

	if (*p == '\n') {
		p++;
		if (*p == '\r')
			p++;
	}

	return p;
}

char *numlines(const char *string)
{
	int cnt = 1;
	static char buf[MAX_STRING_LENGTH*2];
	char buf2[MAX_STRING_LENGTH], tmpb[MAX_STRING_LENGTH];

	buf[0] = '\0';

	while (*string) {
		string = getline(string, tmpb, sizeof(tmpb));
		snprintf(buf2, sizeof(buf2),
			 "%2d. %s\n", cnt++, tmpb);
		strnzcat(buf, sizeof(buf), buf2);
	}

	return buf;
}
