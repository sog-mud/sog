/*
 * $Id: string_edit.c,v 1.7 1998-08-07 07:57:55 fjoe Exp $
 */

/***************************************************************************
 *  File: string.c                                                         *
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
#include "db.h"
#include "comm.h"
#include "log.h"
#include "olc.h"
#include "string_edit.h"

char *string_linedel(char *, int);
char *string_lineadd(char *, char *, int);
char *numlineas(char *);

/*****************************************************************************
 Name:		string_edit
 Purpose:	Clears string and puts player into editing mode.
 Called by:	none
 ****************************************************************************/
void string_edit(CHAR_DATA *ch, char **pString)
{
    send_to_char("-========- Entering EDIT Mode -=========-\n\r", ch);
    send_to_char("    Type :h on a new line for help\n\r", ch);
    send_to_char(" Terminate with a ~ or @ on a blank line.\n\r", ch);
    send_to_char("-=======================================-\n\r", ch);

    if (*pString == NULL)
    {
        *pString = str_dup("");
    }
    else
    {
        **pString = '\0';
    }

    ch->desc->pString = pString;

    return;
}



/*****************************************************************************
 Name:		string_append
 Purpose:	Puts player into append mode for given string.
 Called by:	(many)olc_act.c
 ****************************************************************************/
void string_append(CHAR_DATA *ch, char **pString)
{
    send_to_char("-=======- Entering APPEND Mode -========-\n\r", ch);
    send_to_char("    Type :h on a new line for help\n\r", ch);
    send_to_char(" Terminate with a ~ or @ on a blank line.\n\r", ch);
    send_to_char("-=======================================-\n\r", ch);

    if (*pString == NULL)
	*pString = str_dup("");
    send_to_char(numlineas(*pString), ch);

/* numlineas entrega el string con \n\r */
/*  if (*(*pString + strlen(*pString) - 1) != '\r')
	send_to_char("\n\r", ch); */

    ch->desc->pString = pString;

    return;
}



/*****************************************************************************
 Name:		string_replace
 Purpose:	Substitutes one string for another.
 Called by:	string_add(string.c) (aedit_builder)olc_act.c.
 ****************************************************************************/
char * string_replace(char * orig, char * old, char * new)
{
    char xbuf[MAX_STRING_LENGTH];
    int i;

    xbuf[0] = '\0';
    strcpy(xbuf, orig);
    if (strstr(orig, old) != NULL)
    {
        i = strlen(orig) - strlen(strstr(orig, old));
        xbuf[i] = '\0';
        strcat(xbuf, new);
        strcat(xbuf, &orig[i+strlen(old)]);
        free_string(orig);
    }

    return str_dup(xbuf);
}



/*****************************************************************************
 Name:		string_add
 Purpose:	Interpreter for string editing.
 Called by:	game_loop_xxxx(comm.c).
 ****************************************************************************/
void string_add(CHAR_DATA *ch, const char *argument)
{
    char buf[MAX_STRING_LENGTH];

    /*
     * Thanks to James Seng
     */

    if (*argument == ',')
    {
        char arg1 [MAX_INPUT_LENGTH];
        char arg2 [MAX_INPUT_LENGTH];
        char arg3 [MAX_INPUT_LENGTH];
        char tmparg3 [MAX_INPUT_LENGTH];

        argument = one_argument(argument, arg1);
        argument = first_arg(argument, arg2, FALSE);
	strcpy(tmparg3, argument);
	smash_tilde(tmparg3);
        argument = first_arg(argument, arg3, FALSE);
	smash_tilde(arg3);

        if (!str_cmp(arg1, ":c"))
        {
            send_to_char("String cleared.\n\r", ch);
	    free_string(*ch->desc->pString);
	    *ch->desc->pString = str_dup("");
            return;
        }

        if (!str_cmp(arg1, ":s"))
        {
            char_printf(ch, "String so far:\n\r%s",
            		*ch->desc->pString);
            return;
        }

        if (!str_cmp(arg1, ":r"))
        {
            if (arg2[0] == '\0')
            {
                send_to_char(
                    "usage:  :r \"old string\" \"new string\"\n\r", ch);
                return;
            }

	    smash_tilde(arg3);
            *ch->desc->pString =
                string_replace(*ch->desc->pString, arg2, arg3);
            char_printf(ch, "'%s' replaced with '%s'.\n\r", arg2, arg3);
            return;
        }

        if (!str_cmp(arg1, ":f"))
        {
            *ch->desc->pString = format_string(*ch->desc->pString);
            send_to_char("String formatted.\n\r", ch);
            return;
        }
        
	if (!str_cmp(arg1, ":ld"))
	{
		*ch->desc->pString = string_linedel(*ch->desc->pString, atoi(arg2));
		send_to_char("Line deleted.\n\r", ch);
		return;
	}

	if (!str_cmp(arg1, ":li"))
	{
		*ch->desc->pString = string_lineadd(*ch->desc->pString, tmparg3, atoi(arg2));
		send_to_char("Line inserted.\n\r", ch);
		return;
	}

	if (!str_cmp(arg1, ":lr"))
	{
		*ch->desc->pString = string_linedel(*ch->desc->pString, atoi(arg2));
		*ch->desc->pString = string_lineadd(*ch->desc->pString, tmparg3, atoi(arg2));
		send_to_char("Line replaced.\n\r", ch);
		return;
	}

        if (!str_cmp(arg1, ":h"))
        {
            send_to_char("Sedit help (commands on blank line):   \n\r", ch);
            send_to_char(":r 'old' 'new'   - replace a substring \n\r", ch);
            send_to_char("                   (requires '', \"\") \n\r", ch);
            send_to_char(":h               - get help (this info)\n\r", ch);
            send_to_char(":s               - show string so far  \n\r", ch);
            send_to_char(":f               - (word wrap) string  \n\r", ch);
            send_to_char(":c               - clear string so far \n\r", ch);
            send_to_char(":ld <num>        - delete line #num\n\r", ch);
            send_to_char(":li <num> <str>  - insert <str> before line #num\n\r", ch);
	    send_to_char(":lr <num> <str>  - replace line #num with <str>\n\r", ch);
            send_to_char("@                - end string          \n\r", ch);
            return;
        }

        send_to_char("SEdit:  Invalid comma command.\n\r", ch);
        return;
    }

    if (*argument == '~' || *argument == '@')
    {
	if (ch->desc->editor == ED_MPCODE) /* para los mobprogs */
	{
		MOB_INDEX_DATA *mob;
		int hash;
		MPROG_LIST *mpl;
		MPROG_CODE *mpc;

		EDIT_MPCODE(ch, mpc);

		if (mpc != NULL)
			for (hash = 0; hash < MAX_KEY_HASH; hash++)
				for (mob = mob_index_hash[hash]; mob; mob = mob->next)
					for (mpl = mob->mprogs; mpl; mpl = mpl->next)
						if (mpl->vnum == mpc->vnum)
						{
							char_printf(ch, "Arreglando mob %d.\n\r", mob->vnum);
							mpl->code = mpc->code;
						}
	}

        ch->desc->pString = NULL;
        return;
    }

    strcpy(buf, *ch->desc->pString);

    /*
     * Truncate strings to MAX_STRING_LENGTH.
     * --------------------------------------
     */
    if (strlen(buf) + strlen(argument) >= (MAX_STRING_LENGTH - 4))
    {
        send_to_char("String too long, last line skipped.\n\r", ch);

	/* Force character out of editing mode. */
        ch->desc->pString = NULL;
        return;
    }

    /*
     * Ensure no tilde's inside string.
     * --------------------------------
     */

    strcat(buf, argument);
    strcat(buf, "\n\r");
    free_string(*ch->desc->pString);
    smash_tilde(buf);
    *ch->desc->pString = str_dup(buf);
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
char *format_string(char *oldstring /*, bool fSpace */)
{
  char xbuf[MAX_STRING_LENGTH];
  char xbuf2[MAX_STRING_LENGTH];
  char *rdesc;
  int i=0;
  bool cap=TRUE;
  
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
  strcpy(xbuf2,xbuf);
  
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
      *(rdesc+i)=0;
      strcat(xbuf,rdesc);
      strcat(xbuf,"\n\r");
      rdesc += i+1;
      while (*rdesc == ' ') rdesc++;
    }
    else
    {
      bug ("No spaces", 0);
      *(rdesc+75)=0;
      strcat(xbuf,rdesc);
      strcat(xbuf,"-\n\r");
      rdesc += 76;
    }
  }
  while (*(rdesc+i) && (*(rdesc+i)==' '||
                        *(rdesc+i)=='\n'||
                        *(rdesc+i)=='\r'))
    i--;
  *(rdesc+i+1)=0;
  strcat(xbuf,rdesc);
  if (xbuf[strlen(xbuf)-2] != '\n')
    strcat(xbuf,"\n\r");

  free_string(oldstring);
  return(str_dup(xbuf));
}



/*
 * Used above in string_add.  Because this function does not
 * modify case if fCase is FALSE and because it understands
 * parenthesis, it would probably make a nice replacement
 * for one_argument.
 */
/*****************************************************************************
 Name:		first_arg
 Purpose:	Pick off one argument from a string and return the rest.
 		Understands quates, parenthesis (barring) ('s) and
 		percentages.
 Called by:	string_add(string.c)
 ****************************************************************************/
const char *first_arg(const char *argument, char *arg_first, bool fCase)
{
    char cEnd;

    while (*argument == ' ')
	argument++;

    cEnd = ' ';
    if (*argument == '\'' || *argument == '"'
      || *argument == '%'  || *argument == '(')
    {
        if (*argument == '(')
        {
            cEnd = ')';
            argument++;
        }
        else cEnd = *argument++;
    }

    while (*argument != '\0')
    {
	if (*argument == cEnd)
	{
	    argument++;
	    break;
	}
    if (fCase) *arg_first = LOWER(*argument);
            else *arg_first = *argument;
	arg_first++;
	argument++;
    }
    *arg_first = '\0';

    while (*argument == ' ')
	argument++;

    return argument;
}




/*
 * Used in olc_act.c for aedit_builders.
 */
char * string_unpad(char * argument)
{
    char buf[MAX_STRING_LENGTH];
    char *s;

    s = argument;

    while (*s == ' ')
        s++;

    strcpy(buf, s);
    s = buf;

    if (*s != '\0')
    {
        while (*s != '\0')
            s++;
        s--;

        while(*s == ' ')
            s--;
        s++;
        *s = '\0';
    }

    free_string(argument);
    return str_dup(buf);
}



/*
 * Same as capitalize but changes the pointer's data.
 * Used in olc_act.c in aedit_builder.
 */
char * string_proper(char * argument)
{
    char *s;

    s = argument;

    while (*s != '\0')
    {
        if (*s != ' ')
        {
            *s = UPPER(*s);
            while (*s != ' ' && *s != '\0')
                s++;
        }
        else
        {
            s++;
        }
    }

    return argument;
}

char *string_linedel(char *string, int line)
{
	char *strtmp = string;
	char buf[MAX_STRING_LENGTH];
	int cnt = 1, tmp = 0;

	buf[0] = '\0';

	for (; *strtmp != '\0'; strtmp++)
	{
		if (cnt != line)
			buf[tmp++] = *strtmp;

		if (*strtmp == '\n')
		{
			if (*(strtmp + 1) == '\r')
			{
				if (cnt != line)
					buf[tmp++] = *(++strtmp);
				else
					++strtmp;
			}

			cnt++;
		}
	}

	buf[tmp] = '\0';

	free_string(string);
	return str_dup(buf);
}

char *string_lineadd(char *string, char *newstr, int line)
{
	char *strtmp = string;
	int cnt = 1, tmp = 0;
	bool done = FALSE;
	char buf[MAX_STRING_LENGTH];

	buf[0] = '\0';

	for (; *strtmp != '\0' || (!done && cnt == line); strtmp++)
	{
		if (cnt == line && !done)
		{
			strcat(buf, newstr);
			strcat(buf, "\n\r");
			tmp += strlen(newstr) + 2;
			cnt++;
			done = TRUE;
		}

		buf[tmp++] = *strtmp;

		if (done && *strtmp == '\0')
			break;

		if (*strtmp == '\n')
		{
			if (*(strtmp + 1) == '\r')
				buf[tmp++] = *(++strtmp);

			cnt++;
		}

		buf[tmp] = '\0';
	}

	free_string(string);
	return str_dup(buf);
}

/* buf queda con la linea sin \n\r */
char *getline(char *str, char *buf)
{
	int tmp = 0;
	bool found = FALSE;

	while (*str)
	{
		if (*str == '\n')
		{
			found = TRUE;
			break;
		}

		buf[tmp++] = *(str++);
	}

	if (found)
	{
		if (*(str + 1) == '\r')
			str += 2;
		else
			str += 1;
	} /* para que quedemos en el inicio de la prox linea */

	buf[tmp] = '\0';

	return str;
}

char *numlineas(char *string)
{
	int cnt = 1;
	static char buf[MAX_STRING_LENGTH*2];
	char buf2[MAX_STRING_LENGTH], tmpb[MAX_STRING_LENGTH];

	buf[0] = '\0';

	while (*string)
	{
		string = getline(string, tmpb);
		sprintf(buf2, "%2d. %s\n\r", cnt++, tmpb);
		strcat(buf, buf2);
	}

	return buf;
}
