/*
 * $Id: ban.c,v 1.27 1999-02-17 07:53:27 fjoe Exp $
 */

/***************************************************************************
 *     ANATOLIA 2.1 is copyright 1996-1997 Serdar BULUT, Ibrahim CANPUNAR  *	
 *     ANATOLIA has been brought to you by ANATOLIA consortium		   *
 *	 Serdar BULUT {Chronos}		bulut@rorqual.cc.metu.edu.tr       *
 *	 Ibrahim Canpunar  {Asena}	canpunar@rorqual.cc.metu.edu.tr    *	
 *	 Murat BICER  {KIO}		mbicer@rorqual.cc.metu.edu.tr	   *	
 *	 D.Baris ACAR {Powerman}	dbacar@rorqual.cc.metu.edu.tr	   *	
 *     By using this code, you have agreed to follow the terms of the      *
 *     ANATOLIA license, in the file Anatolia/anatolia.licence             *	
 ***************************************************************************/

/***************************************************************************   
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
*	ROM 2.4 is copyright 1993-1995 Russ Taylor			   *
*	ROM has been brought to you by the ROM consortium		   *
*	    Russ Taylor (rtaylor@pacinfo.com)				   *
*	    Gabrielle Taylor (gtaylor@pacinfo.com)			   *
*	    Brian Moore (rom@rom.efn.org)				   *
*	By using this code, you have agreed to follow the terms of the	   *
*	ROM license, in the file Rom24/doc/rom.license			   *
***************************************************************************/

#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#if	!defined (WIN32)
#include <unistd.h>
#endif
#include <stdio.h>
#include <ctype.h>

#include "merc.h"
#include "ban.h"
#include "db/db.h"

BAN_DATA *	new_ban	(void);
void		free_ban(BAN_DATA *ban);

struct ban_data
{
	BAN_DATA *	next;
	bool		valid;
	flag32_t	ban_flags;
	int		level;
	const char *	name;
};

BAN_DATA *ban_list;
BAN_DATA *ban_free;

BAN_DATA *new_ban(void)
{
    static BAN_DATA ban_zero;
    BAN_DATA *ban;

    if (ban_free == NULL)
	ban = alloc_perm(sizeof(*ban));
    else
    {
	ban = ban_free;
	ban_free = ban_free->next;
    }

    *ban = ban_zero;
    VALIDATE(ban);
    ban->name = str_empty;
    return ban;
}

void free_ban(BAN_DATA *ban)
{
    if (!IS_VALID(ban))
	return;

    free_string(ban->name);
    INVALIDATE(ban);

    ban->next = ban_free;
    ban_free = ban;
}

void save_bans(void)
{
    BAN_DATA *pban;
    FILE *fp;
    bool found = FALSE;

    fclose(fpReserve); 
    if ((fp = dfopen(ETC_PATH, BAN_FILE, "w")) == NULL)
    {
        perror(BAN_FILE);
    }

    for (pban = ban_list; pban != NULL; pban = pban->next)
    {
	if (IS_SET(pban->ban_flags,BAN_PERMANENT))
	{
	    found = TRUE;
	    log_printf("%-20s %-2d %s\n", pban->name, pban->level,
			format_flags(pban->ban_flags));
	    fprintf(fp,"%-20s %-2d %s\n",pban->name,pban->level,
		format_flags(pban->ban_flags));
	}
     }

     fclose(fp);
     fpReserve = fopen(NULL_FILE, "r");
     if (!found)
	dunlink(ETC_PATH, BAN_FILE);

     if (fpReserve == NULL)
	bug("ban_save: can't open null file.", 0);
}

void load_bans(void)
{
    FILE *fp;
    BAN_DATA *ban_last;
 
    if ((fp = dfopen(ETC_PATH, BAN_FILE, "r")) == NULL)
        return;
 
    ban_last = NULL;
    for (; ;)
    {
        BAN_DATA *pban;
        if (feof(fp))
        {
            fclose(fp);
            return;
        }
 
        pban = new_ban();
 
        pban->name = str_dup(fread_word(fp));
	pban->level = fread_number(fp);
	pban->ban_flags = fread_flags(fp);
	fread_to_eol(fp);

        if (ban_list == NULL)
	    ban_list = pban;
	else
	    ban_last->next = pban;
	ban_last = pban;
    }
}

bool check_ban(const char *site, int type)
{
    BAN_DATA *pban;
    char host[MAX_STRING_LENGTH];

    strnzcpy(host, capitalize(site), sizeof(host));
    host[0] = LOWER(host[0]);

    for (pban = ban_list; pban != NULL; pban = pban->next) 
    {
	if(!IS_SET(pban->ban_flags,type))
	    continue;

	if (IS_SET(pban->ban_flags,BAN_PREFIX) 
	&&  IS_SET(pban->ban_flags,BAN_SUFFIX)  
	&&  strstr(pban->name,host) != NULL)
	    return TRUE;

	if (IS_SET(pban->ban_flags,BAN_PREFIX)
	&&  !str_suffix(pban->name,host))
	    return TRUE;

	if (IS_SET(pban->ban_flags,BAN_SUFFIX)
	&&  !str_prefix(pban->name,host))
	    return TRUE;
    }

    return FALSE;
}


void ban_site(CHAR_DATA *ch, const char *argument, bool fPerm)
{
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    char *name;
    BUFFER *buffer;
    BAN_DATA *pban, *prev;
    bool prefix = FALSE,suffix = FALSE;
    int type;

    argument = one_argument(argument, arg1, sizeof(arg1));
    argument = one_argument(argument, arg2, sizeof(arg2));

	if (arg1[0] == '\0') {
		if (ban_list == NULL) {
			char_puts("No sites banned at this time.\n",ch);
			return;
  		}

		buffer = buf_new(-1);

        	buf_add(buffer, "Banned sites  level  type     status\n");
		for (pban = ban_list;pban != NULL;pban = pban->next) {
			char buf2[MAX_STRING_LENGTH];

			snprintf(buf2, sizeof(buf2), "%s%s%s",
				IS_SET(pban->ban_flags,BAN_PREFIX) ? "*" : str_empty,
				pban->name,
				IS_SET(pban->ban_flags,BAN_SUFFIX) ? "*" : str_empty);

			buf_printf(buffer,"%-12s    %-3d  %-7s  %s\n",
				buf2, pban->level,
				IS_SET(pban->ban_flags,BAN_NEWBIES) ?
					"newbies" :
				IS_SET(pban->ban_flags,BAN_PLAYER)  ?
					"player" :
				IS_SET(pban->ban_flags,BAN_PERMIT)  ?
					"permit" :
				IS_SET(pban->ban_flags,BAN_ALL)     ?
					"all"	: str_empty,
	    			IS_SET(pban->ban_flags,BAN_PERMANENT) ?
					"perm" : "temp");
		}

        	page_to_char(buf_string(buffer), ch);
		buf_free(buffer);
        	return;
	}

    /* find out what type of ban */
    if (arg2[0] == '\0' || !str_prefix(arg2,"all"))
	type = BAN_ALL;
    else if (!str_prefix(arg2,"newbies"))
	type = BAN_NEWBIES;
    else if (!str_prefix(arg2,"player"))
	type = BAN_PLAYER;
    else if (!str_prefix(arg2,"permit"))
	type = BAN_PERMIT;
    else
    {
	char_puts("Acceptable ban types are all, newbies, player, and permit.\n",
	    ch); 
	return;
    }

    name = arg1;

    if (name[0] == '*')
    {
	prefix = TRUE;
	name++;
    }

    if (name[strlen(name) - 1] == '*')
    {
	suffix = TRUE;
	name[strlen(name) - 1] = '\0';
    }

    if (strlen(name) == 0)
    {
	char_puts("You have to ban SOMETHING.\n",ch);
	return;
    }

    prev = NULL;
    for (pban = ban_list; pban != NULL; prev = pban, pban = pban->next)
    {
        if (!str_cmp(name,pban->name))
        {
	    if (pban->level > ch->level)
	    {
            	char_puts("That ban was set by a higher power.\n", ch);
            	return;
	    }
	    else
	    {
		if (prev == NULL)
		    ban_list = pban->next;
		else
		    prev->next = pban->next;
		free_ban(pban);
	    }
        }
    }

    pban = new_ban();
    pban->name = str_dup(name);
    pban->level = ch->level;

    /* set ban type */
    pban->ban_flags = type;

    if (prefix)
	SET_BIT(pban->ban_flags,BAN_PREFIX);
    if (suffix)
	SET_BIT(pban->ban_flags,BAN_SUFFIX);
    if (fPerm)
	SET_BIT(pban->ban_flags,BAN_PERMANENT);

    pban->next  = ban_list;
    ban_list    = pban;
    save_bans();
    char_printf(ch, "%s has been banned.\n",pban->name);
    return;
}

void do_ban(CHAR_DATA *ch, const char *argument)
{
    ban_site(ch,argument,FALSE);
}

void do_permban(CHAR_DATA *ch, const char *argument)
{
    ban_site(ch,argument,TRUE);
}

void do_allow(CHAR_DATA *ch, const char *argument)                        
{
    char arg[MAX_INPUT_LENGTH];
    BAN_DATA *prev;
    BAN_DATA *curr;

    one_argument(argument, arg, sizeof(arg));

    if (arg[0] == '\0')
    {
        char_puts("Remove which site from the ban list?\n", ch);
        return;
    }

    prev = NULL;
    for (curr = ban_list; curr != NULL; prev = curr, curr = curr->next)
    {
        if (!str_cmp(arg, curr->name))
        {
	    if (curr->level > ch->level)
	    {
		char_puts(
		   "You are not powerful enough to lift that ban.\n",ch);
		return;
	    }
            if (prev == NULL)
                ban_list   = ban_list->next;
            else
                prev->next = curr->next;

            free_ban(curr);
	    char_printf(ch,"Ban on %s lifted.\n",arg);
	    save_bans();
            return;
        }
    }

    char_puts("Site is not banned.\n", ch);
    return;
}

