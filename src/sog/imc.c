/*
 * IMC2 version 0.10 - an inter-mud communications protocol
 * Copyright (C) 1996 & 1997 Oliver Jowett <oliver@randomly.org>
 *
 * IMC2 Gold versions 1.00 though 2.00 are developed by MudWorld.
 * Copyright (C) 1999 - 2002 Haslage Net Electronics (Anthony R. Haslage)
 *
 * IMC2 MUD-Net version 3.10 is developed by Alsherok and Crimson Oracles
 * Copyright (C) 2002 Roger Libiez ( Samson )
 * Additional code Copyright (C) 2002 Orion Elder
 * Registered with the United States Copyright Office
 * TX 5-555-584
 *
 * IMC2 Continuum version 4.00 is developed by Rogel
 * Copyright (C) 2003 by Rogel, WhoStyles Copyright (C) 2003 by Kris Craig
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program (see the file COPYING); if not, write to the
 * Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * $Id: imc.c,v 1.1.2.7 2003-09-30 11:31:46 fjoe Exp $
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/file.h>
#include <compat/compat.h>

#include "imc.h"
#include "merc.h"
#include "socials.h"

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )					\
				if ( !strcasecmp( word, literal ) )	\
				{					\
				      field = value;			\
				      fMatch = TRUE;			\
				      break;				\
				}

static const char *wr_to;
static char wr_buf[IMC_DATA_LENGTH];
static int wr_sequence;

imc_statistics imc_stats;

time_t imc_now;          /* current time */
time_t imc_boot;          /* current time */
int imc_debug_on;
int imc_minplayerlevel; /* Minumum level IMC will see players at, defaults to 10 */
int imc_minimmlevel; /* Minimum level IMC sees someone as an immortal, defaults to IMCMAX_LEVEL */
int imc_minadminlevel; /* Minimum level IMC sees someone as an administrator, defaults to IMCMAX_LEVEL */
imc_siteinfo_struct imc_siteinfo;
imc_event *imc_event_list, *imc_event_free;
static int event_freecount;
static int memory_head; /* next entry in memory table to use, wrapping */
int imcwait; /* Reconnect timer */
unsigned long imc_sequencenumber;	  /* sequence# for outgoing packets */

const char *imc_name;			      /* our imc name */
static char pinger[100];
const char *imc_versionid;

HUBINFO *this_imcmud;
IMC_CHANNEL *first_imc_channel;
IMC_CHANNEL *last_imc_channel;
REMOTEINFO *first_rinfo;
REMOTEINFO *last_rinfo;
IMC_BLACKLIST *first_imc_mudblacklist;
IMC_BLACKLIST *last_imc_mudblacklist;

/* sequence memory */
_imc_memory imc_memory[IMC_MEMORY];

int (*imc_recv_chain)( PACKET *p, int bcast );
int (*imc_recv_hook)( PACKET *p, int bcast );
void imclog( const char *format, ... ) __attribute__ ( ( format( printf, 1, 2 ) ) );
void imcbug( const char *format, ... ) __attribute__ ( ( format( printf, 1, 2 ) ) );

void smash_tilde(const char *str)
{
}

bool imc_str_prefix( const char *astr, const char *bstr )
{
    if ( !astr || astr[0] == '\0' )
    {
	printlog( "imc_str_prefix: null astr." );
	return TRUE;
    }

    if ( !bstr || bstr[0] == '\0' )
    {
	printlog( "imc_str_prefix: null bstr." );
	return TRUE;
    }

    for ( ; *astr; astr++, bstr++ )
    {
	if ( tolower(*astr) != tolower(*bstr) )
	    return TRUE;
    }

    return FALSE;
}

unsigned char imc_whostyle; /* Sets the color pattern for IMC2 whos */
#define MAX_WHOSTYLE 4

const char * whostylename_table[MAX_WHOSTYLE] = 
{   "Continuum Default", "Winter", "Fiery Bowels of Hell", "IMC2 Classic" };

/* Checks if a player's level is greater or equal to a permission level */
bool level_pass( CHAR_DATA *ch, unsigned char perm_level)
{
    int level;
    
    if(!ch)
        return 1;

    if( perm_level == PERM_PLAYER)
       level = imc_minplayerlevel; 
    else if ( perm_level == PERM_IMMORTAL)
       level = imc_minimmlevel;
    else if ( perm_level == PERM_ADMIN)
       level = imc_minadminlevel;
    else
       level = 0; 

    if ( level <= CH_IMCLEVEL(ch))
        return 1;
    else
        return 0;
}

/* Finds the permission level for a player using level_pass and returns the permission level */
unsigned char get_permlevel( CHAR_DATA *ch)
{
    if ( level_pass(ch, PERM_ADMIN))
        return PERM_ADMIN;

    if ( level_pass(ch, PERM_IMMORTAL))
        return PERM_IMMORTAL;

    if ( level_pass(ch, PERM_PLAYER))
        return PERM_PLAYER;

    return PERM_NONE;
}

/* Checks if a network function is able to be used by a player by checking deny and allow flags,
   followed by a permission level check - preceeding all these checks is a non-npc check */   
bool function_usable( CHAR_DATA *ch, long deny_flag, long allow_flag, unsigned char perm_level )
{
    if (IS_NPC(ch))
        return 0;

    if ( IS_SET( IMC_PFLAGS(ch), deny_flag))
        return 0;

    if ( !level_pass( ch, perm_level) && !IS_SET( IMC_PFLAGS(ch), allow_flag ))
        return 0;

    return 1;
} 

/*
 *  Error logging
 */

/* Generic log function which will route the log messages to the appropriate system logging function */
void imclog( const char *format, ... )
{
   char buf[LSS];
   va_list ap;

   va_start( ap, format );
   vsnprintf( buf, LSS, format, ap );
   va_end( ap );

   printlog( buf );
   return;
}

/* Generic bug logging function which will route the message to the appropriate function that handles bug logs */
void imcbug( const char *format, ... )
{
   char buf[LSS];
   va_list ap;

   va_start( ap, format );
   vsnprintf( buf, LSS, format, ap );
   va_end( ap );

   bug( buf, 0 );
   return;
}

/*
 * Table is formatted with mud color code first, then ANSI code, then IMC color code.
 * This should make it easier to modify local color codes.
 *
 * It is currently setup to support the AFKMUD mud color codes.
 * You need to change the &, } and { tokens used in the table below to match what
 * your mud uses to identify a color token with.
 *
 * & is the foreground text token.
 * } is the blink text token.
 * { is the background text token.
 *
 */

imccolor_struct imccolor_table[] =
{
    /* Color code symbols used in IMC must be carefully screened */
    // First three characters in second column are used for mud color parsing tests.
    // ~ is the color token used in the IMC mud standard.
    // All {, }, and & must be replaced with your mud's color tokens to convert the table
    // to your mud effectively. 'Doubling up' in this table is to escape color codes. If your
    // mud does not do this, then make these single characters. Most muds do escape.
    { "&&", "&", "&"  },
    { "{{", "{", "{"  },
    { "}}", "}", "}"  },    
    {  "~", "~","~~"  }, 
    { "&-", "~", "~~" },
    
	/* Foreground Standard Colors */
	{ "{d", "", "~x" }, // Black
	{ "{r", "", "~r" }, // Dark Red
	{ "{g", "", "~g" }, // Dark Green
	{ "{y", "", "~y" }, // Orange/Brown
	{ "{b", "", "~b" }, // Dark Blue
	{ "{m", "", "~p" }, // Purple/Magenta
	{ "{m", "", "~m" }, // Duplicated to support duplicate IMC code for color
	{ "{c", "", "~c" }, // Cyan
	{ "{w", "", "~w" }, // Grey
	{ "{w", "", "~d" }, // Duplicated to support triplicate IMC code for color
	{ "{x", "", "~!" }, // Triplicated to support IMC reset code for color (defaults to grey)

	/* Foreground extended colors */
	{ "{D", "", "~D" }, // Dark Grey
	{ "{R", "", "~R" }, // Red
	{ "{G", "", "~G" }, // Green
	{ "{Y", "", "~Y" }, // Yellow
	{ "{B", "", "~B" }, // Blue
	{ "{M", "", "~M" }, // Pink
	{ "{C", "", "~C" }, // Light Blue
	{ "{W", "", "~W" }, // White

	/* There are no IMC2 color codes for anything beyond this point - yet */
	{ "{z", "", "" },
	{ "{*", "", "" },
	
	{NULL, NULL, NULL}
};

/* convert from imc color -> mud color */
char *color_itom( const char *s )
{
   static char buf[IMC_DATA_LENGTH];
   const char *current;
   char *out;
   int c, i, l=0;
   int count;

   for( current = s, out = buf, count = 0; *current && count < IMC_DATA_LENGTH; )
   {

      for (c=0; c < 4; c++ )
      {
         if (*current == imccolor_table[c].ansi[0])
            break;
      }

      if( c != 4 ) 
      {
         for( i = 0; imccolor_table[i].mud != NULL; i++ )
         {
            l = strlen( imccolor_table[i].imc );
            if( l && !strncmp( current, imccolor_table[i].imc, l ) )
               break;
         }

         if( imccolor_table[i].mud != NULL ) /* match */
         {
            int len;

            len = strlen( imccolor_table[i].mud );
            count += len;
            if( count >= IMC_DATA_LENGTH )
               break;
            strlcpy( out, imccolor_table[i].mud, IMC_DATA_LENGTH );
            out += len;
            current += l;
            continue;
         }
      }

      *out++ = *current++;
      count++;
   }

   *out = 0;
   strlcat(buf, "{x", IMC_DATA_LENGTH);

   return buf;
}

/* convert from mud color -> imc color */
char *color_mtoi( const char *s )
{
   static char buf[IMC_DATA_LENGTH];
   const char *current;
   char *out;
   int c, i, l = 0;
   int count;

   for( current = s, out = buf, count=0; *current && count < IMC_DATA_LENGTH; )
   {
      for (c=0; c < 4; c++ )
      {
         if (*current == imccolor_table[c].ansi[0])
            break;
      }

      if (c != 4)
      {
         for( i = 0; imccolor_table[i].mud != NULL; i++ )
         {
            l = strlen( imccolor_table[i].mud );
            if( l && !strncmp( current, imccolor_table[i].mud, l ) )
               break;
         }

         if( imccolor_table[i].mud != NULL ) /* match */
         {
            int len;

            len = strlen( imccolor_table[i].imc );
            count += len;
            if( count >= IMC_DATA_LENGTH )
               break;
            strlcpy( out, imccolor_table[i].imc, IMC_DATA_LENGTH );
            out += len;
            current += l;
            continue;
         }
      }

      *out++ = *current++;
      count++;
   }

   *out = 0;
   return buf;
}

// Retrieves a string length minus the color codes.
int imc_strlen_color( const char *str )
{
    int c, i, length;

    if( !str || str[0] == '\0' )
        return 0;

    for( length = i = 0 ; i < strlen ( str ) ; i++ )
    {
        for (c=0; c < 4; c++ )
        {
            if (str[i] == imccolor_table[c].ansi[0])
                break;
        }

        if ( c == 4)
            ++length;
        else
        {
            for (c=0; c < 4; c++)
            {
                if ( str[i] == imccolor_table[c].mud[0] 
                        && str[i+1] == imccolor_table[c].mud[1])
                    break;
            }

            if (c !=4)
                length++;
            else
                --length;
        }
    }

    return length;
}

// Creates a series of spaces that are equal in length to a chosen string
char *EmptyBlock( const char *argument, int leftsidespacing,  int rightsidespacing )
{
    static char buf[LSS];
    int length = imc_strlen_color(argument) + leftsidespacing + rightsidespacing;
    int i = 0;

    if (!length)
        return NULL;

    for ( i = 0; i < length && i < LSS - 1; i++)
        buf[i] = ' ';

    buf[i] = '\0';

    return buf;
}

// Inserts an alternating pattern of two symbols(possibly colors) into any text
char *Alternating_Symbols( const char *argument, const char *symbol1, const char *symbol2 )
{
    static char buf[LSS];
    bool start = TRUE;
    int h = 0, i = 0, j = 0, k = 0;

    if (!argument || argument[0] == '\0' || !symbol1 || symbol1[0] == '\0' || !symbol2 || symbol2[0] == '\0')
        return NULL;

    for ( i = 0; i < strlen(argument); i++)
    {
        if ( argument[i] == ' ' || argument[i] == '\n' || argument[i] == '\r')
        {
            if ( j + 1 >= LSS - 1)
                break;

            start = TRUE;
            buf[j++] = argument[i];
            continue;
        }

        if (start && j + strlen(symbol1) + 1 >= LSS - 1)
            break;
        else if ( j + strlen(symbol2) + 1 >= LSS - 1)
            break;

        if (start)
        {
            for (k = 0; k < strlen(symbol1); k++)
                buf[j++] = symbol1[k];
        }
        else
        {
            for (h = 0; h < strlen(symbol2); h++)
                buf[j++] = symbol2[h];
        }

        buf[j++] = argument[i]; 

        start = !start;
    }

    buf[j] = '\0';

    return buf;
}

// Centers any text, even text that includes color.
char *SetFill( const char *argument, int size, const char *filler)
{
    int space, i = 0, j = 0, length = 0;
    static char buf[LSS];
    char startstring[LSS];
    char endstring[LSS];

    if (imc_strlen_color(filler) <= 0 ) 
        return NULL;

    space = (size - imc_strlen_color( argument )) / imc_strlen_color(filler);

    length = space/2;

    for ( ; length > 0; length--)
    {
        if ( j + strlen(filler) >= LSS - 1 )
            break;

        for ( i = 0; filler[i]; i++ )
            startstring[j++] = filler[i];
    }

    startstring[j] = '\0';

    

    if ( ((space/2) * 2) == space)
        length = space/2;
    else
        length = (space/2) + 1;

    for ( j = i = 0; length > 0; length--)
    {
        if ( j + strlen(filler) >= LSS - 1 )
            break;

        for ( i = 0; filler[i]; i++ )
            endstring[j++] = filler[i];
    }

    endstring[j] = '\0';

    snprintf( buf, LSS, "%s%s%s", startstring, argument, endstring );

    return buf;
}

/* Modified version of Smaug's send_to_char_color function */
void imc_to_char( const char *txt, CHAR_DATA *ch )
{
   if( !ch )
   {
	imcbug( "%s", "imc_to_char: NULL ch!" );
	return;
   }

   if( IS_NPC( ch ) )
      return;

   if( !ch->desc )
   {
	imcbug( "imc_to_char: NULL descriptor for %s!", CH_IMCNAME(ch) );
	return;
   }

   char_puts(txt, ch);

   return;
}

/* Generic send_to_pager type function to send to the proper code for each codebase */
// May be changed later to make it more codebase independent
void imc_to_pager(const char *txt, CHAR_DATA *ch)
{
        BUFFER *output;

        output = buf_new(-1);
        buf_add(output, txt);
        page_to_char(buf_string(output), ch);
        buf_free(output);
}

/* free all the keys in "p" */
void imc_freedata( PACKET *p )
{
   int i;

   for( i = 0; i < IMC_MAX_KEYS; i++ )
   {
      if( p->key[i] )
         IMCSTRFREE( p->key[i] );
      if( p->value[i] )
         IMCSTRFREE( p->value[i] );
   }
   return;
}

/*
 *  String manipulation functions, mostly exported
 */
/* lowercase what */
void imc_slower( char *what )
{
   char *p = what;
   while( *p )
   {
      *p = tolower( *p );
      p++;
   }
}

imc_event *imc_new_event( void )
{
   imc_event *p;

   if( imc_event_free )
   {
      p = imc_event_free;
      imc_event_free = p->next;
      event_freecount--;
   }
   else
      IMCCREATE( p, imc_event, 1 );

   p->when = 0;
   p->callback = NULL;
   p->data = NULL;
   p->next = NULL;
   p->timed = 0;

   return p;
}

void imc_free_event( imc_event *p )
{
   if( event_freecount > 10 ) /* pick a number, any number */
      IMCDISPOSE( p );
   else
   {
      p->next = imc_event_free;
      imc_event_free = p;
      event_freecount++;
   }
}

void imc_run_events( time_t newtime )
{
   imc_event *p;
   void (*callback)(void *);
   void *data;

   while( imc_event_list )
   {
      p = imc_event_list;

      if( p->when > newtime )
         break;

      imc_event_list = p->next;
      callback = p->callback;
      data = p->data;
      imc_now = p->when;

      imc_free_event( p );

      if( callback )
         (*callback)(data);
      else
         imcbug( "%s", "imc_run_events: NULL callback" );
   }
   imc_now = newtime;
}

void imc_add_event( int when, void (*callback)(void *), void *data, int timed )
{
   imc_event *p, *scan, **last;

   p = imc_new_event();
   p->when = imc_now+when;
   p->callback = callback;
   p->data = data;
   p->timed = timed;

   for( last = &imc_event_list, scan = *last; scan; last = &scan->next, scan = *last )
      if( scan->when >= p->when )
         break;

   p->next = scan;
   *last = p;
}

void imc_cancel_event( void (*callback)(void *), void *data )
{
   imc_event *p, *p_next, **last;

   for( last = &imc_event_list, p = *last; p; p = p_next )
   {
      p_next = p->next;

      if( (!callback) && p->data == data )
      {
         *last = p_next;
         imc_free_event( p );
      }
      else if( (callback) && p->data == data )
      {
         *last = p_next;
         imc_free_event( p );
      }
      else if( p->callback == callback && data == NULL )
      {
         *last = p_next;
         imc_free_event( p );
      }
      else
         last = &p->next;
   }
}

/* get the value of "key" from "p"; if it isn't present, return "def" */
const char *imc_getkey( PACKET *p, const char *key, const char *def )
{
   int i;

   for( i = 0; i < IMC_MAX_KEYS; i++ )
      if( p->key[i] && !strcasecmp( p->key[i], key ) )
         return p->value[i];

   return def;
}

/* identical to imc_getkey, except get the integer value of the key */
int imc_getkeyi( PACKET *p, char *key, int def )
{
   int i;

   for( i = 0; i < IMC_MAX_KEYS; i++ )
      if( p->key[i] && !strcasecmp( p->key[i], key ) )
         return atoi( p->value[i] );

   return def;
}

/* add "key=value" to "p" */
void imc_addkey( PACKET *p, const char *key, const char *value )
{
   int i;

   for( i = 0; i < IMC_MAX_KEYS; i++ )
   {
      if( p->key[i] && !strcasecmp( key, p->key[i] ) )
      {
         IMCSTRFREE( p->key[i] );
         IMCSTRFREE( p->value[i] );
         break;
      }
   }
   if( !value )
      return;

   for( i = 0; i < IMC_MAX_KEYS; i++ )
   {
      if( !p->key[i] )
      {
         p->key[i]   = IMCSTRALLOC( key );
         p->value[i] = IMCSTRALLOC( value );
         return;
      }
   }
}

/* add "key=value" for an integer value */
void imc_addkeyi( PACKET *p, const char *key, int value )
{
   char temp[20];
   snprintf( temp, 20, "%d", value );
   imc_addkey( p, key, temp );
}

/* clone packet data */
void imc_clonedata( PACKET *p, PACKET *n )
{
   int i;

   for( i = 0; i < IMC_MAX_KEYS; i++ )
   {
      if( p->key[i] )
         n->key[i] = IMCSTRALLOC( p->key[i] );
      else
         n->key[i] = NULL;
    
      if( p->value[i] )
         n->value[i] = IMCSTRALLOC( p->value[i] );
      else
         n->value[i] = NULL;
   }
}

/* clear all keys in "p" */
void imc_initdata( PACKET *p )
{
   int i;

   for( i = 0; i < IMC_MAX_KEYS; i++ )
   {
      p->key[i]   = NULL;
      p->value[i] = NULL;
   }
}

/* checkrepeat: check for repeats in the memory table */
int checkrepeat( const char *mud, unsigned long seq )
{
   int i;

   for( i = 0; i < IMC_MEMORY; i++ )
      if( imc_memory[i].from && !strcasecmp( mud, imc_memory[i].from ) && seq == imc_memory[i].sequence )
         return 1;

   /* not a repeat, so log it */

   if( imc_memory[memory_head].from )
    IMCSTRFREE( imc_memory[memory_head].from );

   imc_memory[memory_head].from     = IMCSTRALLOC( mud );
   imc_memory[memory_head].sequence = seq;
  
   memory_head++;
   if( memory_head == IMC_MEMORY )
      memory_head = 0;

   return 0;
}

/* return 'mud' from 'player@mud' */
const char *imc_mudof( const char *fullname )
{
   static char buf[IMC_MNAME_LENGTH];
   char *where;

   where = strchr( fullname, '@' );
   if( !where )
      strlcpy( buf, fullname, IMC_MNAME_LENGTH );
   else
      strlcpy( buf, where+1, IMC_MNAME_LENGTH );

   return buf;
}

/* return 'player' from 'player@mud' */
const char *imc_nameof( const char *fullname )
{
   static char buf[IMC_PNAME_LENGTH];
   char *where = buf;
   int count = 0;

   while( *fullname && *fullname != '@' && count < IMC_PNAME_LENGTH-1 )
      *where++ = *fullname++, count++;

   *where = 0;
   return buf;
}

/* return 'player@mud' from 'player' and 'mud' */
char *imc_makename( const char *player, const char *mud )
{
   static char buf[IMC_NAME_LENGTH];

   buf[0] = '\0';

   if ( !player || player[0] == '\0' ) 
       return buf;

   strlcpy( buf, player, IMC_PNAME_LENGTH );
   
   if ( mud && mud[0] != '\0' )
   {
       strlcpy( buf + strlen(buf), "@", 2 );
       strlcpy( buf + strlen(buf), mud, IMC_MNAME_LENGTH );
   }

   return buf;
}

/*  imc_getarg: extract a single argument (with given max length) from
 *  argument to arg; if arg==NULL, just skip an arg, don't copy it out
 */
char *imc_getarg( const char *argument, char *arg, int length )
{
   static char buf[LSS];
   int len = 0;

   if (arg && (!argument || argument[0] == '\0'))
           arg[0] = '\0';

   while( *argument && isspace( *argument ) )
   argument++;

   if( arg )
      while( *argument && !isspace( *argument ) && len < length-1 )
         *arg++ = *argument++, len++;
   else
      while( *argument && !isspace( *argument ) )
         argument++;

   while( *argument && !isspace( *argument ) )
      argument++;

   while( *argument && isspace( *argument ) )
      argument++;

   if( arg )
      *arg = '\0';

   strlcpy( buf, argument, LSS);

   return buf;
}

/* Check for a name in a list */
int imc_hasname( const char *list, const char *name )
{
   char *p;
   char arg[IMC_NAME_LENGTH];

   if( !list )
      return( 0 );

   p = imc_getarg( list, arg, IMC_NAME_LENGTH );
   while( arg[0] )
   {
      if( !strcasecmp( name, arg ) )
         return 1;
      p = imc_getarg( p, arg, IMC_NAME_LENGTH );
   }
   return 0;
}

/* Add a name to a list */
void imc_addname( const char **list, char *name )
{
   char buf[IMC_DATA_LENGTH];

   if( imc_hasname( *list, name ) )
      return;

   if( (*list) && (*list)[0] != '\0' )
      snprintf( buf, IMC_DATA_LENGTH, "%s %s", *list, name );
   else
      strlcpy( buf, name, IMC_DATA_LENGTH );

   if( *list )
      IMCSTRFREE( *list );
   *list = IMCSTRALLOC( buf );
}

/* Remove a name from a list */
void imc_removename( const char **list, char *name )
{
   char buf[1000];
   char arg[IMC_NAME_LENGTH];
   char *p;
  
   buf[0] = '\0';
   p = imc_getarg( *list, arg, IMC_NAME_LENGTH );
   while( arg[0] )
   {
      if( strcasecmp( arg, name ) )
      {
         if( buf[0] )
	      strlcat( buf, " ", 1000 );
         strlcat( buf, arg, 1000 );
      }
      p = imc_getarg( p, arg, IMC_NAME_LENGTH );
   }

   if( *list )
      IMCSTRFREE( *list );
   *list = IMCSTRALLOC( buf );
}

/* escape2: escape " -> \", \ -> \\, CR -> \r, LF -> \n */
char *escape2( const char *data )
{
   static char buf[IMC_DATA_LENGTH];
   char *p;

   for( p = buf; *data && ( p-buf < IMC_DATA_LENGTH-1 ); data++, p++ )
   {
      if( *data == '\n' )
      {
         *p++ = '\\';
         *p = 'n';
      }
      else if( *data == '\r' )
      {
         *p++ = '\\';
         *p = 'r';
      }
      else if( *data == '\\' )
      {
         *p++ = '\\';
         *p = '\\';
      }
      else if( *data == '"' )
      {
         *p++ = '\\';
         *p = '"';
      }
      else
         *p = *data;
   }
   *p = 0;

   return buf;
}

/* printkeys: print key-value pairs, escaping values */
char *printkeys( PACKET *data )
{
   static char buf[IMC_DATA_LENGTH];
   char temp[IMC_DATA_LENGTH];
   int len = 0;
   int i;

   buf[0] = '\0';

   for( i = 0; i < IMC_MAX_KEYS; i++ )
   {
      if( !data->key[i] )
         continue;
      strlcpy( buf + len, data->key[i], IMC_DATA_LENGTH-len-1 );
      strlcat( buf, "=", IMC_DATA_LENGTH );
      len = strlen( buf );

      if( !strchr( data->value[i], ' ' ) )
         strlcpy( temp, escape2( data->value[i] ), IMC_DATA_LENGTH-1 );
      else
      {
         temp[0] = '"';
         strlcpy( temp+1, escape2( data->value[i] ), IMC_DATA_LENGTH-3 );
         strlcat( temp, "\"", IMC_DATA_LENGTH - 2 );
      }
      strlcat( temp, " ", IMC_DATA_LENGTH );
      strlcpy( buf + len, temp, IMC_DATA_LENGTH-len );
      len = strlen( buf );
   }
   return buf;
}

/* parsekeys: extract keys from string */
void parsekeys( const char *string, PACKET *data )
{
   const char *p1;
   char *p2;
   char k[IMC_DATA_LENGTH], v[IMC_DATA_LENGTH];
   int quote;

   p1 = string;

   while( *p1 )
   {
      while( *p1 && isspace( *p1 ) )
         p1++;

      p2 = k;
      while( *p1 && *p1 != '=' && p2-k < IMC_DATA_LENGTH-1 )
         *p2++ = *p1++;
      *p2=0;

      if( !k[0] || !*p1 ) /* no more keys? */
         break;

      p1++;			/* skip the '=' */

      if( *p1 == '"' )
      {
         p1++;
         quote = 1;
      }
      else
         quote = 0;

      p2 = v;
      while( *p1 && ( !quote || *p1 != '"' ) && ( quote || *p1 != ' ' ) && p2-v < IMC_DATA_LENGTH+1 )
      {
         if( *p1 == '\\' )
         {
	      switch( *(++p1) )
	      {
	         case '\\':
	            *p2++ = '\\';
	            break;
	         case 'n':
	            *p2++ = '\n';
	            break;
	         case 'r':
	            *p2++ = '\r';
	            break;
	         case '"':
	            *p2++ = '"';
	            break;
	         default:
	            *p2++ = *p1;
	            break;
	      }
	      if( *p1 )
	         p1++;
         }
         else
	      *p2++ = *p1++;
      }
      *p2 = 0;

      if( !v[0] )
         continue;

      imc_addkey( data, k, v );

      if( quote && *p1 )
         p1++;
   }
}

char *generate2( PACKET *p )
{
   static char temp[IMC_PACKET_LENGTH];
   char newpath[IMC_PATH_LENGTH];

   if( !p->type[0] || !p->i.from[0] || !p->i.to[0] )
   {
      imcbug( "%s", "generate2: bad packet!" );
      imcbug( "type: %s from: %s to: %s", p->type, p->i.from, p->i.to );
      imcbug( "path: %s data: %s", p->i.path, printkeys( p ) );
      return NULL;		/* catch bad packets here */
   }

   if( !p->i.path[0] )
      strlcpy( newpath, imc_name, IMC_PATH_LENGTH );
   else
      snprintf( newpath, IMC_PATH_LENGTH, "%s!%s", p->i.path, imc_name );

   snprintf( temp, IMC_PACKET_LENGTH, "%s %lu %s %s %s %s", 
           p->i.from, p->i.sequence, newpath, p->type, p->i.to, printkeys( p ) );
   return temp;
}

PACKET *interpret2( char *argument )
{
   char seq[20];
   static PACKET out;

   imc_initdata( &out );
   argument = imc_getarg( argument, out.i.from, IMC_NAME_LENGTH );
   argument = imc_getarg( argument, seq, 20 );
   argument = imc_getarg( argument, out.i.path, IMC_PATH_LENGTH );
   argument = imc_getarg( argument, out.type, IMC_TYPE_LENGTH );
   argument = imc_getarg( argument, out.i.to, IMC_NAME_LENGTH );

   if( !out.i.from[0] || !seq[0] || !out.i.path[0] || !out.type[0] || !out.i.to[0] )
   {
      imcbug( "%s", "interpret2: bad packet received, discarding" );
      return NULL;
   }

   parsekeys( argument, &out );

   out.i.sequence = strtoul( seq, NULL, 10 );
   return &out;
}

_imc_vinfo imc_vinfo[] =
{
  { 0, NULL, NULL },
  { 1, NULL, NULL },
  { 2, generate2, interpret2 }
};

/* return 1 if 'name' is a part of 'path'  (internal) */
int inpath( char *path, char *name )
{
   char buf[IMC_MNAME_LENGTH+3];
   char tempn[IMC_MNAME_LENGTH], tempp[IMC_PATH_LENGTH];

   strlcpy( tempn, name, IMC_MNAME_LENGTH );
   strlcpy( tempp, path, IMC_PATH_LENGTH );
   imc_slower( tempn );
   imc_slower( tempp );

   if( !strcasecmp( tempp, tempn ) )
      return 1;

   snprintf( buf, IMC_MNAME_LENGTH+3, "%s!", tempn );
   if( !strncmp( tempp, buf, strlen( buf ) ) )
      return 1;

   snprintf( buf, IMC_MNAME_LENGTH+3, "!%s", tempn );
   if( strlen(buf) < strlen(tempp) && !strcasecmp( tempp + strlen(tempp) - strlen(buf), buf ) )
      return 1;

   snprintf( buf, IMC_MNAME_LENGTH+3, "!%s!", tempn );
   if( strstr( tempp, buf ) )
      return 1;

   return 0;
}

/* find an info entry for "name" */
REMOTEINFO *imc_find_reminfo( const char *name, int type )
{
   REMOTEINFO *p;

   for( p = first_rinfo; p; p = p->next )
   {
      if( !strcasecmp( name, p->name ) )
         return p;
   }
   return NULL;
}

/* return 'b' from 'a!b!c!d!e' */
const char * imc_hubinpath( const char *path )
{
    const char *separator, *where;
    static char buf[IMC_NAME_LENGTH];
    int i = 0;

    if ( !path || path[0] == '\0')
        return NULL;

    if ( (separator = strchr( path, '!' ) ) == NULL )
       return path;

    where = separator + 1;

    while ( *where != '!' && *where != '\0' )
    {
        where++;
        i++;
    }

    i++;

    strlcpy( buf, separator + 1, i );
    return buf;

}

/* return 'a' from 'a!b!c!d!e' */
char *imc_firstinpath( char *path )
{
   static char buf[IMC_NAME_LENGTH];
   char *p;

   for( p = buf; *path && *path != '!'; *p++ = *path++ )
      ;

   *p = 0;
   return buf;
}

/* create a new info entry, insert into list */
REMOTEINFO *imc_new_reminfo( char *mud )
{
   REMOTEINFO *p, *mud_prev;

   IMCCREATE( p, REMOTEINFO, 1 );

   p->name    = IMCSTRALLOC( mud );
   p->netname = NULL;
   p->version = NULL;
   p->path   = NULL;
   p->top_sequence = 0;
   p->expired = FALSE;

   for( mud_prev = first_rinfo; mud_prev; mud_prev = mud_prev->next )
      if( strcasecmp( mud_prev->name, mud ) >= 0 )
         break;

   if( !mud_prev )
      IMCLINK( p, first_rinfo, last_rinfo, next, prev );
   else
      IMCINSERT( p, mud_prev, first_rinfo, next, prev );

   return p;
}

/* update our routing table based on a packet received with path "path" */
void updateroutes( char *path )
{
   REMOTEINFO *p;
   char *sender, *temp;

   /* loop through each item in the path, and update routes to there */

   temp = path;
   while( temp && temp[0] != '\0' )
   {
      sender = imc_firstinpath( temp );

      if( strcasecmp( sender, imc_name ) )
      {
         /* not from us */
         /* check if its in the list already */

         p = imc_find_reminfo( sender, 1 );
         if( !p )			/* not in list yet, create a new entry */
         {
	      p = imc_new_reminfo( sender );

          p->netname = IMCSTRALLOC( "unknown" );
	      p->version = IMCSTRALLOC( "unknown" );
         }
         else
         {				/* already in list, update the entry */
	      p->expired = FALSE;
         }
      }

      /* get the next item in the path */

      temp = strchr( temp, '!' );
      if( temp )
         temp++;			/* skip to just after the next '!' */
   }
}

/* check if a packet from a given source is blacklisted */
bool imc_isblacklisted( char *who )
{
   IMC_BLACKLIST *mud;

   for( mud = first_imc_mudblacklist; mud; mud = mud->next )
   {
	if( !strcasecmp( mud->name, imc_mudof( who ) ) )
	   return TRUE;
   }
   return FALSE;
}

static struct 
{
    const char *name;
    long allow_flag;
    long deny_flag;
    long pset_flag;
    int minlevel;
        
} imc_functions[]=
{
    { "IMC-Status",   ALLOW_IMCSTATUS,    DENY_IMCSTATUS,    PSET_IMCSTATUS,   PERM_IMMORTAL },
    { "IMC-Chan-FYI", ALLOW_IMCCHANFYI,   DENY_IMCCHANFYI,   PSET_IMCCHANFYI,  PERM_IMMORTAL },
    { "imcptell",     ALLOW_NOTSUPPORTED, DENY_IMCPTELL,     PSET_IMCPTELL,    PERM_PLAYER },
    { "imcpbeep",     ALLOW_NOTSUPPORTED, DENY_IMCPBEEP,     PSET_IMCPBEEP,    PERM_PLAYER },
    { "imcpvisible",  ALLOW_IMCPVISIBLE,  DENY_IMCPVISIBLE,  PSET_IMCPVISIBLE, PERM_PLAYER },
    { "imcpcolor",    ALLOW_NOTSUPPORTED, DENY_NOTSUPPORTED, PSET_IMCPCOLOR,   PERM_PLAYER }
};

#define numfunctions ( sizeof( imc_functions ) / sizeof( imc_functions[0] ) )

imc_char_data *imc_getdata( CHAR_DATA *ch )
{
   imc_char_data *d;

   IMCCREATE( d, imc_char_data, 1 );
   if( !ch ) /* fake system character */
   {
      d->perm_level = PERM_NONE;
      strlcpy( d->name, "*", IMC_NAME_LENGTH );
      return d;
   }

   d->perm_level = get_permlevel(ch);

   strlcpy( d->name, CH_IMCNAME(ch), IMC_NAME_LENGTH );

   return d;
}

void receive_oldchannel( imc_char_data *from, int number, const char *argument, int emote )
{
   DESCRIPTOR_DATA *d;
   CHAR_DATA *victim;
   char buf[IMC_DATA_LENGTH], str[IMC_DATA_LENGTH];
   int chan;
   
   if( number != 2 && number != 15 )
      return;

   if ( number == 2 )
   {
       chan = 0;

       if (!emote)
           snprintf( buf, IMC_DATA_LENGTH, "~c[~CIMC-Status~c] ~C%s ~cannounces '%s'\n", 
                   from->name, argument);
       else
           snprintf( buf, IMC_DATA_LENGTH, "~c[~CIMC-Status~c] ~C%s ~c%s\n", from->name, argument );
   }
   else
   {
       chan = 1;

       if (!emote)
           snprintf( buf, IMC_DATA_LENGTH, "~c[~CIMC-Chan-FYI~c] ~C%s ~cannounces '%s'\n", 
                   from->name, argument );
       else
           snprintf( buf, IMC_DATA_LENGTH, "~c[~CIMC-Chan-FYI~c] ~C%s ~c%s\n", from->name, argument );
   }

   snprintf( str, IMC_DATA_LENGTH, "%s", color_itom(buf));

   for( d = first_descriptor; d; d = d->next )
   {
      if( d->connected == CON_PLAYING && ( victim = d->original ? d->original : d->character ) != NULL
	 && !IS_SET( IMC_PFLAGS( victim ), imc_functions[chan].pset_flag )
	 && function_usable( victim, imc_functions[chan].deny_flag, imc_functions[chan].allow_flag,
         imc_functions[chan].minlevel ) )
      {
          imc_to_char( str, victim );
      }
   }
}

void imc_recv_chat( imc_char_data *from, int channel, const char *argument )
{
   receive_oldchannel( from, channel, argument, 0 );
}

void imc_recv_emote( imc_char_data *from, int channel, const char *argument )
{
   receive_oldchannel( from, channel, argument, 1 );
}

/* convert back from 'd' to 'p' */
void setdata( PACKET *p, imc_char_data *d )
{
   imc_initdata( p );

   if( !d )
   {
      strlcpy( p->from, "*", IMC_NAME_LENGTH );
      imc_addkeyi( p, "level", -1 );
      return;
   }

   strlcpy( p->from, d->name, IMC_NAME_LENGTH );

   imc_addkeyi( p, "level", d->perm_level );
}

/* send a tell to a remote player */
void imc_send_tell( imc_char_data *from, char *to, const char *argument, int isreply )
{
   PACKET out;

   if( !this_imcmud || this_imcmud->state < CONN_COMPLETE )
       return;

   if( !strcasecmp( imc_mudof( to ), "*" ) )
      return; /* don't let them do this */

   setdata( &out, from );

   strlcpy( out.to, to, IMC_NAME_LENGTH );
   strlcpy( out.type, "tell", IMC_TYPE_LENGTH );
   imc_addkey( &out, "text", argument );
   if( isreply )
      imc_addkeyi( &out, "isreply", isreply );

   imc_send( &out );
   imc_freedata( &out );
}

/* send a 'you have been blacklisted' response */
void imc_sendblacklistresponse( char *to )
{
   char buf[IMC_DATA_LENGTH];

   if( strcasecmp( imc_nameof( to ), "*" ) )
   {
      snprintf( buf, IMC_DATA_LENGTH, "%s is blocking packets from your mud.", imc_name );
      imc_send_tell( NULL, to, buf, 1 );
   }
}

void imc_recv_imcptell( imc_char_data *from, char *to, const char *argument, int isreply )
{
   DESCRIPTOR_DATA *d;
   CHAR_DATA *victim = NULL;
   IMC_BLACKLIST *entry;
   char buf[IMC_DATA_LENGTH];
   char tstring[LSS];

   if( !strcasecmp( to, "*" ) ) /* drop messages to system */
      return;

   for( d = first_descriptor; d; d = d->next )
      if( d->connected == CON_PLAYING && ( victim = d->original ? d->original : d->character ) != NULL 
              && IS_IMCVISIBLE(victim) &&  !strcasecmp( to, CH_IMCNAME(victim) ))
          break;
   
   if(!d)
   {
      snprintf( buf, IMC_DATA_LENGTH, "%s is not here.", to );
      imc_send_tell( NULL, from->name, buf, 1 );
      return;
   }
   

   if( !function_usable(victim, DENY_IMCPTELL, ALLOW_IMCPTELL, PERM_PLAYER)
          || IS_SET(IMC_PFLAGS(victim), PSET_IMCPTELL)) 
   {
      if( strcasecmp( imc_nameof( from->name ), "*" ) )
      {
         snprintf( buf, IMC_DATA_LENGTH, "%s is not receiving tells.", to );
	     imc_send_tell( NULL, from->name, buf, 1 );
      }

      return;
   } 

   for( entry = FIRST_IMCBLACKLIST(victim); entry; entry = entry->next )
        if(!strcasecmp( from->name, entry->name ) || !strcasecmp( imc_mudof(from->name), entry->name))
           break;

   if (entry)
   {
      snprintf( buf, IMC_DATA_LENGTH, "%s has blacklisted you.", to );
      imc_send_tell( NULL, from->name, buf, 1 );
      return;
   }

   if( IS_SET(IMC_PFLAGS(victim), PSET_IMCPAFK) )
   {
      snprintf( buf, IMC_DATA_LENGTH, "%s is AFK.", to );
      imc_send_tell( NULL, from->name, buf, 1 );
      return;
   }
    
   if( strcasecmp( imc_nameof( from->name ), "*" ) )     /* not a system message */
   {
      if( IMC_RREPLY(victim) )
         IMCSTRFREE( IMC_RREPLY(victim) );

      IMC_RREPLY(victim) = IMCSTRALLOC( from->name );
   }

   strlcpy( tstring, color_itom("~C%s ~cimcptells you ~c'~W%s~c'\n" ), LSS);

   snprintf( buf, IMC_DATA_LENGTH, tstring,
           from->name, color_itom(argument) );

   imc_to_char( buf, victim );

   return;
}

void imc_recv_whoreply( const char *to, const char *text, int sequence, int inlen )
{
   DESCRIPTOR_DATA *d;
   CHAR_DATA *victim;

   for( d = first_descriptor; d; d = d->next )
   {
      if( d->connected == CON_PLAYING && ( victim = d->original ? d->original : d->character ) != NULL
	 && !strcasecmp( to, CH_IMCNAME(victim) ) )
      {
         imc_to_pager( color_itom(text), victim );
         return;
      }
   }
}

const char *channel_mudof( const char *fullname )
{
   static char buf[IMC_PNAME_LENGTH];
   char *where = buf;
   int count=0;

   while( *fullname && *fullname != ':' && count < IMC_PNAME_LENGTH-1 )
      *where++ = *fullname++, count++;

   *where = 0;
   return buf;
}

/* see if someone can talk on a channel - lots of string stuff here! */
bool imc_audible( IMC_CHANNEL *c, const char *who )
{
   if( !c->refreshed ) // Crash fix
  	 return FALSE;

   /* checking a mud? */
   if( !strchr( who, '@' ) )
   {
      char arg[IMC_DATA_LENGTH];
      char *p;
      int invited = 0;

      /* added compress policy - shogar - 1/29/2000 */
      if( c->policy != CHAN_PRIVATE && c->policy != CHAN_CPRIVATE )
         return TRUE;

      if( !strcasecmp( channel_mudof( c->name ) , imc_name ) )
         return TRUE;

      /* Private channel - can only hear if someone with the right mud name is there. */
      p = imc_getarg( c->invited, arg, IMC_NAME_LENGTH );
      while( arg[0] )
      {
         if( !strcasecmp( who, arg ) || !strcasecmp( who, imc_mudof( arg ) ) )
         {
	      invited = 1;
	      break;
         }
         p = imc_getarg( p, arg, IMC_NAME_LENGTH );
      }

      if( !invited )
         return FALSE;

      p = imc_getarg( c->excluded, arg, IMC_NAME_LENGTH );
      while( arg[0] )
      {
         if( !strcasecmp( who, arg ) || !strcasecmp( who, imc_mudof( arg ) ) )
            return FALSE;
	
         p = imc_getarg( p, arg, IMC_NAME_LENGTH );
      }
      return TRUE;
   }

   /* owners and operators always can */
   if( !strcasecmp( c->owner, who ) || imc_hasname( c->operators, who ) )
      return TRUE;

   /* IMC locally can use any channel */
   if( !strcasecmp( imc_nameof( who ), "IMC" ) && !strcasecmp( imc_mudof( who ), imc_name ) )
      return FALSE;
  
   /* added compress copen policy - shogar - 1/29/2000 */
   if( c->policy == CHAN_OPEN || c->policy == CHAN_COPEN )
   {
      /* open policy. default yes. override with excludes, then invites */
    
      if( ( imc_hasname( c->excluded, who ) || imc_hasname( c->excluded, imc_mudof( who ) ) )
        && !imc_hasname( c->invited, who ) && !imc_hasname( c->invited, imc_mudof( who ) ) )
         return FALSE;
      else
         return TRUE;
   }

   /* closed or private. default no, override with invites, then excludes */
   if( ( imc_hasname( c->invited, who ) || imc_hasname( c->invited, imc_mudof( who ) ) )
     && !imc_hasname( c->excluded, who ) && !imc_hasname( c->excluded, imc_mudof( who ) ) )
      return TRUE;
   else
      return FALSE;
}

void update_imchistory( IMC_CHANNEL *channel, const char *message )
{
   char msg[LSS], buf[LSS];
   struct tm *local;
   time_t t;
   int x;

   if( !channel )
   {
	imcbug( "%s", "update_imchistory: NULL channel received!" );
	return;
   }

   if( !message || message[0] == '\0' )
   {
	imcbug( "%s", "update_imchistory: NULL message received!" );
	return;
   }

   strlcpy( msg, message, LSS );
   for( x = 0; x < MAX_IMCHISTORY; x++ )
   {
      if( channel->history[x] == NULL )
      {
         t = time( NULL );
         local = localtime( &t );
         snprintf( buf, LSS, "~R[%-2.2d/%-2.2d %-2.2d:%-2.2d] ~G%s\n",
		local->tm_mon+1, local->tm_mday, local->tm_hour, local->tm_min, color_mtoi(msg) );
         channel->history[x] = IMCSTRALLOC( color_itom(buf) );
         break;
      }

      if( x == MAX_IMCHISTORY - 1 )
      {
         int y;

         for( y = 1; y < MAX_IMCHISTORY; y++ )
         {
            int z = y-1;

            if( channel->history[z] != NULL )
            {
               IMCSTRFREE( channel->history[z] );
               channel->history[z] = IMCSTRALLOC( channel->history[y] );
            }
         }

         t = time( NULL );
         local = localtime( &t );
         snprintf( buf, LSS, "~R[%-2.2d/%-2.2d %-2.2d:%-2.2d] ~G%s\n",
		local->tm_mon+1, local->tm_mday, local->tm_hour, local->tm_min, color_mtoi(msg) );
	   IMCSTRFREE( channel->history[x] );
         channel->history[x] = IMCSTRALLOC( color_itom(buf) );
      }
   }
   return;
}

void imc_showchannel( IMC_CHANNEL *c, const char *from, const char *txt, int emote )
{
   DESCRIPTOR_DATA *d;
   CHAR_DATA *ch;
   char buf[LSS];

   if( !c->local_name || c->local_name[0] == '\0' )
      return;
  
   if( emote < 2 )
      snprintf( buf, LSS, emote ? c->emoteformat : c->regformat, from, color_itom(txt)); 
   else
      snprintf( buf, LSS, c->socformat, color_itom( txt ) );
 
   for( d = first_descriptor; d; d = d->next )
   {
	if( !d->character )
	   continue;

      ch = d->character;

      if( get_permlevel(ch) < c->perm_level
       || !imc_audible( c, imc_makename( CH_IMCNAME(ch), imc_name ) ) 
	 || !imc_hasname( IMC_CSUBSCRIBED(ch), c->local_name ) )
      continue;

      char_printf( ch, "%s\n", buf );
  }
  update_imchistory( c, buf );
}

void imc_sendmessage( IMC_CHANNEL *c, const char *name, const char *text, int emote )
{
   PACKET out;

   strlcpy( out.from, name, IMC_NAME_LENGTH );
   imc_initdata( &out );

   imc_addkey( &out, "channel", c->name );
   imc_addkey( &out, "text", text );
   imc_addkeyi( &out, "emote", emote );

   /* send a message out on a channel */
   if( c->policy == CHAN_PRIVATE || c->policy == CHAN_CPRIVATE )
   {
      strlcpy( out.type, "ice-msg-p", IMC_TYPE_LENGTH );
      snprintf( out.to, IMC_NAME_LENGTH, "IMC@%s", channel_mudof( c->name ) );
   }
   else
   {
      /* broadcast */
      strlcpy( out.type, "ice-msg-b", IMC_TYPE_LENGTH );
      strlcpy( out.to, "*@*", IMC_NAME_LENGTH );
   }

   imc_send( &out );
   imc_freedata( &out );
}

/* respond to a who request with the given data */
void imc_send_whoreply( const char *to, const char *data, int sequence )
{
   PACKET out;

   if( !this_imcmud || this_imcmud->state < CONN_COMPLETE )
      return;

   if( !strcasecmp( imc_mudof( to ), "*" ) )
      return; /* don't let them do this */

   imc_initdata( &out );

   strlcpy( out.to, to, IMC_NAME_LENGTH );

   strlcpy( out.type, "who-reply", IMC_TYPE_LENGTH );
   strlcpy( out.from, "*", IMC_NAME_LENGTH );

   if( sequence != -1 )
      imc_addkeyi( &out, "sequence", sequence );

   imc_addkey( &out, "text", data );

   imc_send( &out );
   imc_freedata( &out );
}

void imc_whoreply_start( const char *to )
{
   wr_sequence = 0;
   wr_to = IMCSTRALLOC( to );
   wr_buf[0] = '\0';
}

void imc_whoreply_add( const char *text )
{
   /* give a bit of a margin for error here */
   if( strlen( wr_to ) + strlen( text ) >= IMC_DATA_LENGTH-500 )
   {
      imc_send_whoreply( wr_to, wr_buf, wr_sequence );
      wr_sequence++;
      strlcpy( wr_buf, text, IMC_DATA_LENGTH );
      return;
   }
   strlcat( wr_buf, text, IMC_DATA_LENGTH );
}

void imc_whoreply_end( void )
{
   imc_send_whoreply( wr_to, wr_buf, -( wr_sequence + 1 ) );
   IMCSTRFREE( wr_to );
   wr_buf[0] = '\0';
}

// Displays the person's rank or permission level status
const char *imcrankbuffer( CHAR_DATA *ch )
{
   static char rbuf[LSS];
   char imclevel_color;
   unsigned char perm_level;
   char rankcolor_code[4] = { 'g', 'G', 'C', 'Y'}; 

   perm_level = get_permlevel(ch);
   imclevel_color = rankcolor_code[perm_level];


   if( CH_IMCRANK(ch) && CH_IMCRANK(ch)[0] != '\0' )
   {
       snprintf( rbuf, LSS, "~%c%s", rankcolor_code[perm_level], color_mtoi(CH_IMCRANK(ch)));
       return rbuf;
   }

   if ( perm_level < PERM_IMMORTAL )
       snprintf( rbuf, LSS, "~%cPlayer", rankcolor_code[perm_level] ); 
   else if ( perm_level >= PERM_ADMIN )
       snprintf( rbuf, LSS, "~%cAdministrator", rankcolor_code[perm_level] );
   else
       snprintf( rbuf, LSS, "~%cImmortal", rankcolor_code[perm_level] );

   return rbuf;
}

/* expanded for minimal mud-specific code. I really don't want to replicate
 * stock in-game who displays here, since it's one of the most commonly
 * changed pieces of code. shrug.
 */
void process_imcplist( imc_char_data *from, const char *argument )
{
   DESCRIPTOR_DATA *d;
   CHAR_DATA *person;
   char buf[LSS], personbuf[LSS], tailbuf[LSS], rank[LSS], buf2[LSS];
   char stats[20];
   int pcount = 0, i = 0;
   
   imc_whoreply_start( from->name );
   imc_whoreply_add( "\n\r" );
   
   switch( imc_whostyle )
   {
       case 1: // The Winter Who Style by Kris
           snprintf( buf, LSS, " Players on %s ", imc_siteinfo.name );
           imc_whoreply_add( SetFill( EmptyBlock( buf, 0, 1 ), 77, "~W _" ) );    
           imc_whoreply_add( "\n\r");
           snprintf( buf2, LSS, "%s~C|", Alternating_Symbols( buf, "~W", "~C" ) );
           imc_whoreply_add( "~C|" );
           imc_whoreply_add( SetFill( buf2, 77, "~B*~C|" ) );
           imc_whoreply_add( "\n\r");

           if( imc_siteinfo.port > 0 )
              snprintf( buf, LSS, "~W**** ~wtelnet://%s:%d ~W****", 
                      imc_siteinfo.host, imc_siteinfo.port );
           else
              snprintf( buf, LSS, "~W**** ~wtelnet://%s ~W****", imc_siteinfo.host );

           imc_whoreply_add( SetFill( buf, 78, " ") );
           imc_whoreply_add( "\n\r\n\r" );
           break;
       
       case 2: // The Fiery Bowels of Hell Who Style by Kris
           snprintf( buf, LSS, "~YPlayers on %s", imc_siteinfo.name );
           imc_whoreply_add( SetFill( buf, 78, " " ));
           imc_whoreply_add( "\n\r" );
           imc_whoreply_add( SetFill( 
                "~R   (      ~r, ~R)        )     (      ~r, ~R)        )    (      ~r, ~R)        )    ",
                78, " " ) );
           imc_whoreply_add( "\n\r" );
           imc_whoreply_add( SetFill( 
                "~R ( ~r,~R(~r.   ~R(    (~r,`~R)  )    ( ~r,~R(~r.   ~R(    (~r,`~R)  )   ( ~r,~R(~r.   ~R(    (~r,`~R)  )    ( ~r,~R(~r.  ~R",
                78, " " ) ); 
           imc_whoreply_add( "\n\r" );
           imc_whoreply_add( SetFill(
                "~R  )~r' ~R(~r' ~R(~r'~R) ( )  ) (~r.~R)    )~r' ~R(~r' ~R(~r'~R) ( )  ) (~r.~R) }  )~r' ~R(~r' ~R(~r'~R) ( )  ) (~r.~R)    )~r' ~R(~r' ~R",
                78, " " ) ); 
           imc_whoreply_add( "\n\r" );
           imc_whoreply_add( SetFill(
                "~R (~r' ~R)~r,~R)~r.  ~B_~R)~B_  ~R( (~r,~R)~r' ~R)  (~r' ~R)~r,~R)~r.  ~B_~R)~B_  ~R( (~r,~R)~r' ~R) (~r' ~R)~r,~R)~r.  ~B_~R)~B_  ~R( (~r,~R)~r' ~R)  (~r' ~R)~r,~R)~r. ~R",
                78, " " ) ); 
           imc_whoreply_add( "\n\r" );

           if( imc_siteinfo.port > 0 )
              snprintf( buf, LSS, "~Y--->  ~ytelnet://%s:%d  ~Y<---", 
                      imc_siteinfo.host, imc_siteinfo.port );
           else
              snprintf( buf, LSS, "~Y--->  ~ytelnet://%s  ~Y<---", imc_siteinfo.host );

           imc_whoreply_add( SetFill( buf, 78, " ") );
           imc_whoreply_add( "\n\r\n\r" );
           break;
       
       case 3: // IMC2 Classic Who Style - Slight altered from older versions of the IMC2 client
   
           snprintf( buf, LSS, "~R-=[ ~WPlayers on %s ~R]=-", imc_siteinfo.name );
           imc_whoreply_add( SetFill( buf, 78, " ") );
           imc_whoreply_add( "\n\r" );
    
           if( imc_siteinfo.port > 0 )
              snprintf( buf, LSS, "~Y-=[ ~Wtelnet://%s:%d ~Y]=-", imc_siteinfo.host, imc_siteinfo.port );
           else
              snprintf( buf, LSS, "~Y-=[ telnet://%s ]=-", imc_siteinfo.host );
   
       
           imc_whoreply_add( SetFill( buf, 78, " ") );
           imc_whoreply_add( "\n\r" );
           break;

       default: // IMC2 4.00 Continuum Default Who Style by Kris
       
           snprintf( buf, LSS, " Players on %s ", imc_siteinfo.name );
           imc_whoreply_add( SetFill( EmptyBlock( buf, 0, 1 ), 77, "~C _" ) );    
           imc_whoreply_add( "\n\r");
           snprintf( buf2, LSS, "%s~G|", Alternating_Symbols( buf, "~Y", "~C" ) );
           imc_whoreply_add( "~G|" );
           imc_whoreply_add( SetFill( buf2, 77, "~cx~G|" ) );
           imc_whoreply_add( "\n\r");

           if( imc_siteinfo.port > 0 )
              snprintf( buf, LSS, "~C**** ~ctelnet://%s:%d ~C****", 
                      imc_siteinfo.host, imc_siteinfo.port );
           else
              snprintf( buf, LSS, "~C**** ~ctelnet://%s ~C****", imc_siteinfo.host );

           imc_whoreply_add( SetFill( buf, 78, " ") );
           imc_whoreply_add( "\n\r\n\r" );
           break;
   }

   i = 0;

   /* See if there are any visible immortals online to even display players header. */
   for( d = first_descriptor; d; d = d->next )
	if( d->character && (d->connected == CON_PLAYING) && level_pass( d->character, PERM_IMMORTAL)
            && IS_IMCVISIBLE(d->character))
       i++; 

   if( i > 0 )
   {

      switch( imc_whostyle )
      {
          case 1: // The Winter Who Style by Kris
              imc_whoreply_add( SetFill( "~CAdmins  ~Wand  ~CImmortals", 78, " ") );
              imc_whoreply_add( "\n\r~W");
              imc_whoreply_add( SetFill( "", 78, "-" ) );
              break;
          case 2: // The Fiery Bowels of Hell Who Style by Kris
              imc_whoreply_add( SetFill( "~RAdmins~Y----~rand~Y----~RImmortals", 78, "~Y-") );
              break;
          case 3: // IMC2 Classic Who Style - Slight altered from older versions of the IMC2 client
              imc_whoreply_add( SetFill( "-=[ ~WImmortals ~R]=-", 78, "~R-") );
              break;
          default: // IMC2 4.00 Continuum Default Who Style by Kris
              imc_whoreply_add( SetFill( "~CAdmins  ~cand  ~CImmortals", 78, " ") );
              imc_whoreply_add( "\n\r~Y");
              imc_whoreply_add( SetFill( "", 78, "-" ) );
              break;
      }

      imc_whoreply_add( "\n\r\n\r" );

      for( d = first_descriptor; d; d = d->next ) 
      {
	   if( d->character && d->connected == CON_PLAYING ) 
	   {
            person = d->character;

            if ( !level_pass(d->character, PERM_IMMORTAL ) || !IS_IMCVISIBLE(person))
               continue;

            pcount++;

		    strlcpy( rank, SetFill( imcrankbuffer(person), 22, " ") , LSS );

            if ( IS_SET(IMC_PFLAGS(person), PSET_IMCPAFK) )
               strlcpy( stats, "[AFK]", 20 );
            else
               strlcpy( stats, "[---]", 20 );
            
          
            switch( imc_whostyle )
            {

                case 2: // The Fiery Bowels of Hell Who Style by Kris
                  snprintf( personbuf, LSS, "~Y%s ~y%s ~Y%s~y%s\n\r", rank, stats, 
                  CH_IMCNAME(person), color_mtoi(CH_IMCTITLE(person)));
                  break;
               case 3: // IMC2 Classic Who Style - Slight altered from older versions of the IMC2 client
                  snprintf( personbuf, LSS, "%s ~D%s ~G%s%s\n\r", rank, stats, 
                  CH_IMCNAME(person), color_mtoi(CH_IMCTITLE(person)));
                  break;
               case 1: // The Winter Who Style by Kris
               default: // IMC2 4.00 Continuum Default Who Style by Kris
	              snprintf( personbuf, LSS, "~C%s ~w%s ~C%s~c%s\n\r", rank, stats, 
                  CH_IMCNAME(person), color_mtoi(CH_IMCTITLE(person)));
                  break;
            }

	        imc_whoreply_add( personbuf );
	   }
      }
   }

   i = 0;

   /* See if there are any visible players online to even display players header. */
   for( d = first_descriptor; d; d = d->next )
	if( d->character && (d->connected == CON_PLAYING) && !level_pass( d->character, PERM_IMMORTAL)
            && IS_IMCVISIBLE(d->character))
       i++; 

   if( i > 0 )
   {
      
   
      switch( imc_whostyle )
      {
          case 1: // The Winter Who Style by Kris
              imc_whoreply_add( "\n\r\n\r" );
              imc_whoreply_add( SetFill( "  ~GMortal Players  ", 78, "~W*") ); 
              break;        
          case 2: // The Fiery Bowels of Hell Who Style by Kris
              imc_whoreply_add( "\n\r\n\r" );
              imc_whoreply_add( SetFill( "  ~RMortal Players  ", 78, "~Y*") );
              break;
          case 3: // IMC2 Classic Who Style - Slight altered from older versions of the IMC2 client
              imc_whoreply_add( SetFill( "-=[ ~WPlayers ~B]=-", 78, "~B-") );
              break;
          default: // IMC2 4.00 Continuum Default Who Style by Kris
              imc_whoreply_add( "\n\r\n\r" );
              imc_whoreply_add( SetFill( "  ~GMortal Players  ", 78, "~Y*") ); 
              break;     
      };

      imc_whoreply_add( "\n\r\n\r" );
              
      for( d = first_descriptor; d; d = d->next ) 
      {
	   if( d->character && d->connected == CON_PLAYING ) 
	   {
            person = d->character;

            if( level_pass(d->character, PERM_IMMORTAL ) || !IS_IMCVISIBLE(person) )
               continue;

            pcount++;

		    strlcpy( rank, SetFill( imcrankbuffer(person), 22, " "), LSS );

            if ( IS_SET(IMC_PFLAGS(person), PSET_IMCPAFK) )
               strlcpy( stats, "[AFK]", 20 );
            else
               strlcpy( stats, "[---]", 20 );
          
            switch( imc_whostyle )
            {
               case 2: // The Fiery Bowels of Hell Who Style by Kris
                  snprintf( personbuf, LSS, "~R%s ~r%s ~R%s~r%s\n\r", rank, stats, 
                  CH_IMCNAME(person), color_mtoi(CH_IMCTITLE(person)));
                  break;
               case 3: // IMC2 Classic Who Style - Slight altered from older versions of the IMC2 client
                  snprintf( personbuf, LSS, "%s ~D%s ~G%s%s\n\r", rank, stats, 
                  CH_IMCNAME(person), color_mtoi(CH_IMCTITLE(person)));
                  break;
               case 1: // The Winter Who Style by Kris
               default: // IMC2 4.00 Continuum Default Who Style by Kris
                  snprintf( personbuf, LSS, "~G%s ~w%s ~G%s~g%s\n\r", rank, stats, 
                  CH_IMCNAME(person), color_mtoi(CH_IMCTITLE(person)));
                  break;
            }
            
            personbuf[LSS] = '\0';
	        imc_whoreply_add( personbuf );
	   }
      }
   }

   switch( imc_whostyle )
   {
       case 1: // The Winter Who Style by Kris
           snprintf( tailbuf, LSS, "\n\r\n\r~W** ~C%d ~cPlayer%s ~W**-", pcount, pcount == 1 ? "" : "s" );
           imc_whoreply_add( tailbuf );

           snprintf( tailbuf, LSS, "~W** ~GHomepage: ~g%s ~W**-** ~C%3d ~cMax Since Reboot ~W**\n\r", 
                   imc_siteinfo.www, IMCMAXPLAYERS );
           imc_whoreply_add( tailbuf );
           break;
       case 2: // The Fiery Bowels of Hell Who Style by Kris
           snprintf( tailbuf, LSS, "\n\r\n\r~R** ~Y%d ~yPlayer%s ~R**-", pcount, pcount == 1 ? "" : "s" );
           imc_whoreply_add( tailbuf );

           snprintf( tailbuf, LSS, "~R** ~YHomepage: ~y%s ~R**-** ~Y%3d ~yMax Since Reboot ~R**\n\r", 
                   imc_siteinfo.www, IMCMAXPLAYERS );
           imc_whoreply_add( tailbuf );
           break;
       case 3: // IMC2 Classic Who Style - Slight altered from older versions of the IMC2 client
           snprintf( tailbuf, LSS, "\n\r~Y[~W%d Player%s~Y] ", pcount, pcount == 1 ? "" : "s" );
           imc_whoreply_add( tailbuf );

           snprintf( tailbuf, LSS, "~Y[~WHomepage: %s~Y] [~W%3d Max Since Reboot~Y]\n\r", 
                   imc_siteinfo.www, IMCMAXPLAYERS );
           imc_whoreply_add( tailbuf );
           break;
       default: // IMC2 4.00 Continuum Default Who Style by Kris
           snprintf( tailbuf, LSS, "\n\r\n\r~Y** ~C%d ~cPlayer%s ~Y**-", pcount, pcount == 1 ? "" : "s" );
           imc_whoreply_add( tailbuf );

           snprintf( tailbuf, LSS, "~Y** ~GHomepage: ~g%s ~Y**-** ~C%3d ~cMax Since Reboot ~Y**\n\r", 
                   imc_siteinfo.www, IMCMAXPLAYERS );
           imc_whoreply_add( tailbuf );
           break;
   }

   imc_whoreply_end();
}

/* Displays generic information about a player. */
void process_imcpinfo( imc_char_data *from, const char *to )
{
   DESCRIPTOR_DATA *d;
   CHAR_DATA *victim = NULL;
   char buf[LSS], permissions[LSS], functions[LSS];
   unsigned char i = 0;
   static char * const male_female [] = { "Neither", "Male", "Female" }; 

   if ( !to || to[0] == '\0')
       return;

   for( d = first_descriptor; d; d = d->next )
   {
      if( d->connected == CON_PLAYING && ( victim = d->original ? d->original : d->character ) != NULL
             && IS_IMCVISIBLE(victim) && !strcasecmp( to, CH_IMCNAME(victim) ) )
          break;
   }

   if (!d)
   {
       imc_send_whoreply( from->name, "That player is either currently offline or not visible.\n\r", -1);
       return;
   }

   i = get_permlevel(victim);

   if ( i == PERM_NONE )
       strlcpy( permissions, "Ineligible for IMC", LSS );
   else if ( i == PERM_PLAYER )
       strlcpy( permissions, "Player", LSS );
   else if ( i == PERM_IMMORTAL )
       strlcpy( permissions, "Immortal", LSS );
   else
       strlcpy( permissions, "Administrator", LSS);

   functions[0] = '\0';

   for ( i = 0; i < numfunctions; i++)
   {
       if ( function_usable( victim, imc_functions[i].deny_flag, imc_functions[i].allow_flag,
                  imc_functions[i].minlevel) && !IS_SET( IMC_PFLAGS(victim), imc_functions[i].pset_flag)) 
           snprintf( functions + strlen(functions), LSS, "%s ", imc_functions[i].name);
   }  


   snprintf( buf, LSS,     "~cPlayer Information\n\r"
                           "~W------------------\n\r"
                           "~cName: ~W%s\n\r"
                           "~cSex: ~W%s\n\r"
                           "~cPermission Level: ~W%s\n\r"
                           "~cSubscribed Channels: ~W%s\n\r"
                           "~cActive Network Functions: ~W%s\n\r",
          CH_IMCNAME(victim), male_female[URANGE(0, CH_IMCSEX(victim), 2)], permissions,
          (IMC_CSUBSCRIBED(victim) && IMC_CSUBSCRIBED(victim)[0] != '\0' ? IMC_CSUBSCRIBED(victim) : "None"),
          (functions && functions[0] != '\0' ? functions : ""));

   imc_send_whoreply( from->name, buf, -1 );
   return;
}

/* connect to hub */
bool imc_connect_to( void )
{
   int desc;
   struct sockaddr_in sa;
   char buf[IMC_DATA_LENGTH];
   int r;

   if( !this_imcmud )
   {
      imcbug( "%s", "No connection data loaded" );
      return FALSE;
   }

   if( this_imcmud->desc > 0 )
   {
      imcbug( "%s", "Already connected" );
      return FALSE;
   }

   imclog( "Connecting to %s", this_imcmud->hubname );

   /* warning: this blocks. It would be better to farm the query out to
    * another process, but that is difficult to do without lots of changes
    * to the core mud code. You may want to change this code if you have an
    * existing resolver process running.
    */
   if( ( sa.sin_addr.s_addr = inet_addr( this_imcmud->host ) ) == -1UL )
   {
      struct hostent *hostinfo;

      if( !( hostinfo = gethostbyname( this_imcmud->host ) ) )
      {
         imcbug( "%s", "imc_connect: couldn't resolve hostname" );
         return FALSE;
      }
      sa.sin_addr.s_addr = *(unsigned long *) hostinfo->h_addr;
   }

   sa.sin_port = htons( this_imcmud->port );
   sa.sin_family = AF_INET;

   desc = socket( AF_INET, SOCK_STREAM, 0 );
   if( desc < 0 )
   {
      perror( "socket" );
      return FALSE;
   }

   r = fcntl( desc, F_GETFL, 0 );
   if( r < 0 || fcntl( desc, F_SETFL, O_NONBLOCK | r ) < 0 )
   {
      perror( "imc_connect: fcntl" );
      close( desc );
      return FALSE;
   }

   if( connect( desc, (struct sockaddr *)&sa, sizeof(sa) ) < 0 )
   {
      if( errno != EINPROGRESS )
      {
         perror( "connect" );
         close( desc );
         return FALSE;
      }
      this_imcmud->in_progress = TRUE;
   }
   else
	this_imcmud->in_progress = FALSE;

   this_imcmud->state    = CONN_SENDCLIENTPWD;
   this_imcmud->desc     = desc;
   this_imcmud->insize   = IMC_MINBUF;
   IMCCREATE( this_imcmud->inbuf, char, this_imcmud->insize );
   this_imcmud->outsize  = IMC_MINBUF;
   IMCCREATE( this_imcmud->outbuf, char, this_imcmud->outsize );
   this_imcmud->inbuf[0] = this_imcmud->outbuf[0] = '\0';
   this_imcmud->newoutput = 0;

   /* NetName connection authentication by Kris */
   snprintf( buf, IMC_DATA_LENGTH, "PW %s %s version=%d %s", imc_name, this_imcmud->clientpw, 
           IMC_VERSION, imc_siteinfo.netname );
   do_imcsend( buf );

   return TRUE;
}

const char *imc_list( void )
{
   static char buf[IMC_DATA_LENGTH];
   REMOTEINFO *p;
   int count = 1;

   snprintf( buf, IMC_DATA_LENGTH, "~WActive muds on IMC:\n"
                "~c%-15.15s  ~C%-35.35s  ~g%-10.10s  ~G%-10.10s"
        "\n\n~c%-15.15s  ~C%-35.35s  ~g%-10.10s  ~G%-10.10s",
                "Name", "IMC Version", "Network", "Hub", 
          imc_siteinfo.name, imc_versionid, imc_siteinfo.netname, this_imcmud->hubname );

   for( p = first_rinfo; p; p = p->next, count++ )
   {
      snprintf( buf + strlen(buf), IMC_DATA_LENGTH - strlen(buf), 
              "\n~c%-15.15s  ~C%-35.35s  ~g%-10.10s  ~G%-10.10s",
         p->name, p->version, p->netname, p->expired ? "~Rexpired" : imc_hubinpath(p->path));
   }

   snprintf( buf + strlen( buf ), IMC_DATA_LENGTH - strlen(buf),
           "\n~W%d muds on IMC found.\n", count );

   return buf;
}

// Formats a number into a string, with numbers seperated by commas
// Merely eye-candy, but why not? -- Kratas
const char *comma( int number )
{
    char buf[200];
    static char buf_comma[200];
    int x, x_pcomma, ex_amount;

    strlcpy( buf_comma, "", 200 );
    snprintf( buf, 200, "%d", number );
    ex_amount = strlen( buf ) % 3;
    
    for( x = 0, x_pcomma = 0 ; x < strlen( buf ) ; x++, x_pcomma++ )
    {
        if( (x-ex_amount) % 3 == 0 && x > 0 )
        {
            buf_comma[x_pcomma] = ',';
            x_pcomma++;
            buf_comma[x_pcomma] = buf[x];
        }
        else
            buf_comma[x_pcomma] = buf[x];
    }
    buf_comma[x_pcomma] = '\0';

    return buf_comma;
}

/* get some IMC stats, return a string describing them */
const char *imc_getstats( const char *choice )
{
   static char buf[IMC_DATA_LENGTH];
   int evcount = 0;
   imc_event *ev = imc_event_list;
   long secondspast;

   if ( !choice || choice[0] == '\0' || !strcasecmp( choice, "network" ) )
   {

   
       while( ev )
       {
          evcount++;
          ev = ev->next;
       }

       if( (imc_now - imc_stats.start) > 0)
          secondspast = imc_now - imc_stats.start;
       else
          secondspast = 1;

       snprintf( buf, IMC_DATA_LENGTH,
                               "~WGeneral IMC Statistics\n"
                               "~cReceived packets   : ~C%ld\n"
                               "~cReceived bytes     : ~C%s (%ld/second)\n"
                               "~cTransmitted packets: ~C%ld\n"
                               "~cTransmitted bytes  : ~C%s (%ld/second)\n"
                               "~cMaximum packet size: ~C%d\n"
                               "~cPending events     : ~C%d\n"
                               "~cSequence drops     : ~C%d\n"
                               "~cDebug              : ~C%s\n"
                               "~cLast IMC Boot      : ~C%s\n",
            imc_stats.rx_pkts, comma( imc_stats.rx_bytes), imc_stats.rx_bytes / secondspast,
            imc_stats.tx_pkts, comma( imc_stats.tx_bytes), imc_stats.tx_bytes / secondspast,
            imc_stats.max_pkt, evcount, imc_stats.sequence_drops, imc_debug_on ? "Yes" : "No",
            ctime( &imc_boot ) );

       return buf;
   }  

   if (!strcasecmp( choice, "general"))
   {
      snprintf( buf, IMC_DATA_LENGTH,
                              "~WSite Information:\n"
                              "~cName           ~W:~C %s\n"
                              "~cIMC Version    ~W:~C %s\n"
                              "~cAddress        ~W:~C telnet://%s:%d\n"
                              "~cWebpage        ~W:~C http://%s\n"
                              "~cAdmin Email    ~W:~C %s\n"
                              "~cDetails        ~W:~C %s\n", 
         imc_siteinfo.name, imc_versionid, imc_siteinfo.host, imc_siteinfo.port, imc_siteinfo.www, 
         imc_siteinfo.email, imc_siteinfo.details );

      return buf;
   }

   return "Bad invocation of imc_getstats."; 
}

void imc_recv_who( imc_char_data *from, const char *type )
{
   char arg[LSS];

   if ( !type || type[0] == '\0')
       return;

   type = imc_getarg( type, arg, LSS );

   /* ... and lets send 'em the data! */
   if( !strcasecmp( arg, "who" ) )
      process_imcplist( from, type );
   else if( !strcasecmp( arg, "finger" ) )
      process_imcpinfo( from, type );
   else if( !strcasecmp( arg, "info" ) )
      imc_send_whoreply( from->name, imc_getstats("general"), -1 );
   else if( !strcasecmp( arg, "list" ) )
      imc_send_whoreply( from->name, imc_list(), -1 );
   else if( !strcasecmp( arg, "istats" ) )
      imc_send_whoreply( from->name, imc_getstats("network"), -1 );
   else if( !strcasecmp( arg, "options" ) || !strcasecmp( arg, "services" ) || !strcasecmp( arg, "help" ) )
      imc_send_whoreply( from->name, 
         "~WAvailable imcminfo types:\n\r"
         "~chelp       ~W- ~Cthis list\n\r"
         "~cwho        ~W- ~Cwho listing\n\r"
         "~cinfo       ~W- ~Cmud information\n\r"
         "~clist       ~W- ~Cactive IMC connections\n\r"
         "~cistats     ~W- ~Cnetwork traffic statistics\n\r"
         "~cfinger xxx ~W- ~Cfinger player xxx\n\r", 
         -1 );
   else
      imc_send_whoreply( from->name, "Sorry, no information is available of that type.\n\r", -1 );

   return;
}

/* respond with a whois-reply */
void imc_send_whoisreply( const char *to, const char *data )
{
   PACKET out;

   if( !this_imcmud || this_imcmud->state < CONN_COMPLETE )
      return;

   if( !strcasecmp( imc_mudof( to ), "*" ) )
      return; /* don't let them do this */

   imc_initdata( &out );

   strlcpy( out.to, to, IMC_NAME_LENGTH );
   strlcpy( out.type, "whois-reply", IMC_TYPE_LENGTH );
   strlcpy( out.from, "*", IMC_NAME_LENGTH );
   imc_addkey( &out, "text", data );

   imc_send( &out );
   imc_freedata( &out );
}

void imc_recv_whois( imc_char_data *from, const char *to )
{
   DESCRIPTOR_DATA *d;
   CHAR_DATA *victim;
   char buf[LSS];

   /* This circumvents the Validator packets that were used to deter people from being in an invisible
      state from IMC2 3.00 to 3.10. They were easily circumvented and so removed from the IMC2 client
      code. */
   if ( !imc_str_prefix ( "Validator", from->name ) )
   {
       snprintf( buf, LSS, "imcpfind %s : %s@%s is online.\n\r", to, to, imc_name );
       imc_send_whoisreply( from->name, buf );
       return;
   }

   for( d = first_descriptor; d; d = d->next )
   {
      if( d->connected == CON_PLAYING && ( victim = d->original ? d->original : d->character ) != NULL 
             && IS_IMCVISIBLE(victim)
             && ( !strcasecmp( to, CH_IMCNAME(victim) ) 
             || ( strlen(to) > 3 && !imc_str_prefix( to, CH_IMCNAME(victim) ) ) ) )
      {
         snprintf( buf, LSS, "imcpfind %s : %s@%s is online.\n\r", to, CH_IMCNAME(victim), imc_name );
         imc_send_whoisreply( from->name, buf );
      }
   }
}

void imc_recv_whoisreply( const char *to, const char *text )
{
   DESCRIPTOR_DATA *d;
   CHAR_DATA *victim;

   for( d = first_descriptor; d; d = d->next )
   {
      if( d->connected == CON_PLAYING && ( victim = d->original ? d->original : d->character ) != NULL
	 && !strcasecmp( (char *)to, CH_IMCNAME(victim)) )
      {
         imc_to_char( color_itom(text), victim );
         return;
      }
   }
}

void imc_recv_imcpbeep( imc_char_data *from, const char *to )
{
   DESCRIPTOR_DATA *d;
   CHAR_DATA *victim = NULL;
   IMC_BLACKLIST *entry;
   char buf[IMC_DATA_LENGTH];

   if( !strcasecmp( to, "*" ) ) /* drop messages to system */
      return;

   for( d = first_descriptor; d; d = d->next )
      if( d->connected == CON_PLAYING && ( victim = d->original ? d->original : d->character ) != NULL 
              && IS_IMCVISIBLE(victim) && !strcasecmp( to, CH_IMCNAME(victim) ))
          break;
   
   if(!d)
   {
      snprintf( buf, IMC_DATA_LENGTH, "%s is not here.", to );
      imc_send_tell( NULL, from->name, buf, 1 );
      return;
   }
   

   if( !function_usable(victim, DENY_IMCPBEEP, ALLOW_IMCPBEEP, PERM_PLAYER)
          || IS_SET(IMC_PFLAGS(victim), PSET_IMCPBEEP)) 
   {
      if( strcasecmp( imc_nameof( from->name ), "*" ) )
      {
         snprintf( buf, IMC_DATA_LENGTH, "%s is not receiving beeps.", to );
	     imc_send_tell( NULL, from->name, buf, 1 );
      }

      return;
   } 

   for( entry = FIRST_IMCBLACKLIST(victim); entry; entry = entry->next )
        if( !strcasecmp( from->name, entry->name ) || !strcasecmp( imc_mudof(from->name), entry->name))
           break;

   if (entry)
   {
      snprintf( buf, IMC_DATA_LENGTH, "%s has blacklisted you.", to );
      imc_send_tell( NULL, from->name, buf, 1 );
      return;
   }

   if ( IS_SET(IMC_PFLAGS(victim), PSET_IMCPAFK) )
   {
      snprintf( buf, IMC_DATA_LENGTH, "%s is AFK.", to );
      imc_send_tell( NULL, from->name, buf, 1 );
      return;
   }
    
   /* always display the true name here */
   snprintf( buf, IMC_DATA_LENGTH, color_itom("~c\a%s imcpbeeps you.\n"), from->name );

   imc_to_char( buf, victim );
   return;
}

/* called when a keepalive has been received */
void imc_recv_keepalive( const char *from, const char *version, const char *networkname )
{
   REMOTEINFO *p;

   if( !strcasecmp( from, imc_name ) )
      return;
  
   /*  this should never fail, imc.c should create an entry if one doesn't exist (in the path update code) */
   p = imc_find_reminfo( from, 0 );
   if( !p )		    /* boggle */
      return;

   if( strcasecmp( version, p->version ) )    /* remote version has changed? */
   {
      IMCSTRFREE( p->version );              /* if so, update it */
      p->version = IMCSTRALLOC( version );
   }

   if( strcasecmp( networkname, p->netname ) )
   {
      IMCSTRFREE( p->netname );
      p->netname = IMCSTRALLOC( networkname );
   }

   return;
}

/* send a traceroute request */
void imc_send_traceroute( const char *to, const char *playername )
{
   PACKET out;

   if( !this_imcmud || this_imcmud->state < CONN_COMPLETE )
       return;

   imc_initdata( &out );
   strlcpy( out.type, "ping", IMC_TYPE_LENGTH );
   strlcpy( out.from, playername, IMC_NAME_LENGTH );
   strlcpy( out.to, "*@", IMC_MNAME_LENGTH - 2 );
   strlcpy( out.to+2, to, IMC_MNAME_LENGTH-2 );

   imc_send( &out );
   imc_freedata( &out );
}

/* send a traceroute packet back */
void imc_send_traceroutereply( const char *to, const char *path )
{
   PACKET out;

   if( !this_imcmud || this_imcmud->state < CONN_COMPLETE )
       return;

   imc_initdata( &out );
   strlcpy( out.type, "ping-reply", IMC_TYPE_LENGTH );
   strlcpy( out.from, "*", IMC_NAME_LENGTH );
   strlcpy( out.to, to, IMC_MNAME_LENGTH );
   imc_addkey( &out, "path", path );

   imc_send( &out );
   imc_freedata( &out );
}

/* called when a traceroute is received */
void imc_recv_traceroute( const char *from, const char *path )
{
   /* return the traceroute */
   imc_send_traceroutereply( from, path );
}

/* Changes a route's ! to -> for a more understandable display */
const char *display_route( const char *route)
{
    static char buf[LSS];
    int i = 0;

    if (!route || *route == '\0')
        return NULL;
    
    while( i < LSS - 1 && *route != '\0' )
    {
        if ( *route == '!' )
        {
            buf[i++] = '-';
            buf[i++] = '>';
            route++;
        }
        else
            buf[i++] = *route++;
    }

    buf[i] = '\0';
    return buf;
}

/* called when a traceroute reply is received */
void imc_recv_traceroutereply( const char *from, const char *to, const char *pathto, const char *pathfrom )
{
   char incoming[LSS], outgoing[LSS];
   DESCRIPTOR_DATA *d = NULL;
   CHAR_DATA *ch = NULL;

   for( d = first_descriptor; d; d = d->next )
      if( d->connected == CON_PLAYING && (ch = d->original ? d->original : d->character) != NULL
          && ( !strcasecmp( CH_IMCNAME(ch), pinger) || !strcasecmp( CH_IMCNAME(ch), to ) ) ) 
              break;

   pinger[0] = '\0';

   if(!d)
       return;

   if ( pathfrom && pathfrom[0] != '\0' )
       snprintf( incoming, LSS, "%s->%s", display_route(pathfrom), imc_name );

   if ( pathto && pathto[0] != '\0' )
       snprintf( outgoing, LSS, "%s->%s", display_route(pathto), from );

   char_printf( ch, color_itom(  
           "~WTraceroute Information for %s\n"
           "~cSend Path    ~W:   ~C%s\n"
           "~cReturn Path  ~W:   ~C%s\n\n"),
           from, outgoing, incoming );
   return;
}

/* send a keepalive to everyone */
void imc_send_keepalive( void )
{
   PACKET out;

   if( !this_imcmud || this_imcmud->state < CONN_COMPLETE )
       return;

   imc_initdata( &out );
   strlcpy( out.type, "is-alive", IMC_TYPE_LENGTH );
   strlcpy( out.from, "*", IMC_NAME_LENGTH );
   strlcpy( out.to, "*@*", IMC_NAME_LENGTH );
   imc_addkey( &out, "versionid", imc_versionid );
   imc_addkey( &out, "networkname", imc_siteinfo.netname );

   imc_send( &out );
   imc_freedata( &out );
}

void getdata( PACKET *p, imc_char_data *d )
{
   strlcpy( d->name, p->from, IMC_NAME_LENGTH );
   d->perm_level = imc_getkeyi( p, "level", 0 );
   return;
}


/* Commands called by the interface layer */

/* handle a packet destined for us, or a broadcast */
void imc_recv( PACKET *p )
{
   imc_char_data d;
   int bcast;
   REMOTEINFO *i;

   bcast = !strcasecmp( imc_mudof( p->i.to ), "*" ) ? 1 : 0;

   getdata( p, &d );

   i = imc_find_reminfo( imc_mudof( p->from ), 0 );
   if( i )
   {
      if( i->path )
         IMCSTRFREE( i->path );
      i->path = IMCSTRALLOC( p->i.path );
      i->expired = FALSE;
   }

   /* chat: message to a channel (broadcast) */
   if( !strcasecmp( p->type, "chat" ) && !imc_isblacklisted( p->from ) )
      imc_recv_chat( &d, imc_getkeyi( p, "channel", 0 ), imc_getkey( p, "text", "" ) );

   /* emote: emote to a channel (broadcast) */
   else if( !strcasecmp( p->type, "emote" ) && !imc_isblacklisted( p->from ) )
      imc_recv_emote( &d, imc_getkeyi( p, "channel", 0 ), imc_getkey( p, "text", "" ) );

   /* tell: tell a player here something */
   else if( !strcasecmp( p->type, "tell" ) )
   {
      if( imc_isblacklisted( p->from ) )
         imc_sendblacklistresponse( p->from );
      else
         imc_recv_imcptell( &d, p->to, imc_getkey( p, "text", "" ), imc_getkeyi( p, "isreply", 0 ) );
   }

   /* who-reply: receive a who response */
   else if( !strcasecmp( p->type, "who-reply" ) || !strcasecmp( p->type, "wHo-reply" ) )
      imc_recv_whoreply( p->to, imc_getkey( p, "text", "" ), imc_getkeyi( p, "sequence", -1 ), -1 );

   /* who: receive a who request */
   else if( !strcasecmp( p->type, "who" ) || !strcasecmp( p->type, "wHo" ) )
   {
      if( imc_isblacklisted( p->from ) )
         imc_sendblacklistresponse( p->from );
      else
         imc_recv_who( &d, imc_getkey( p, "type", "who" ) );
   }

   /* whois-reply: receive a whois response */
   else if( !strcasecmp( p->type, "whois-reply" ) )
      imc_recv_whoisreply( p->to, imc_getkey( p, "text", "" ) );

   /* whois: receive a whois request */
   else if( !strcasecmp( p->type, "whois" ) )
      imc_recv_whois( &d, p->to );

   /* beep: beep a player */
   else if( !strcasecmp( p->type, "beep" ) )
   {
      if( imc_isblacklisted( p->from ) )
         imc_sendblacklistresponse( p->from );
      else
         imc_recv_imcpbeep( &d, p->to );
   }

   /* is-alive: receive a keepalive (broadcast) */
   else if( !strcasecmp( p->type, "is-alive" ) )
      imc_recv_keepalive( imc_mudof( p->from ), imc_getkey( p, "versionid", "unknown" ), 
              imc_getkey( p, "networkname", "unknown") );

   /* traceroute: receive a send path */
   else if( !strcasecmp( p->type, "ping" ) )
      imc_recv_traceroute( p->from, p->i.path );

   /* traceroute-reply: receive a send path and a return path */
   else if( !strcasecmp( p->type, "ping-reply" ) )
      imc_recv_traceroutereply( imc_mudof( p->from ), imc_nameof(p->to), imc_getkey( p, "path", NULL ), 
              p->i.path );

   /* handle keepalive requests - shogar */
   else if( !strcasecmp( p->type, "keepalive-request" ) )
      imc_send_keepalive();

   /* expire closed hubs - shogar */
   else if( !strcasecmp( p->type, "close-notify" ) )
   {
      REMOTEINFO *r;
      char fake[90];

      r = imc_find_reminfo( imc_getkey( p, "host", "unknown" ), 0 );
      if( r )
      {
         r->expired = TRUE;
         for( r = first_rinfo; r; r = r->next )
         {
            char *sf;
            snprintf( fake, 90, "!%s", imc_getkey( p, "host", "___unknown " ) );
            if( r->name && r->path && ( sf = strstr( r->path, fake ) ) && sf
		   && ( *( sf + strlen( fake ) ) == '!' || *( sf + strlen( fake ) ) == 0 ) )
            {
               r->expired = TRUE;
               imc_send_traceroute( r->name, "*");
            }
         }
      }
   }

   /* call catch-all fn if present */
   else
   {
      PACKET out;

      if( imc_recv_hook )
         if( (*imc_recv_hook)( p, bcast ) )
            return;

      if( bcast || !strcasecmp( p->type, "reject" ) )
         return;

      /* reject packet */
      strlcpy( out.type, "reject", IMC_TYPE_LENGTH );
      strlcpy( out.to, p->from, IMC_NAME_LENGTH );
      strlcpy( out.from, p->to, IMC_NAME_LENGTH );

      imc_clonedata( p, &out );
      imc_addkey( &out, "old-type", p->type );
      imc_send( &out );
      imc_freedata( &out );
   }
}

/* send a packet to a mud using the right version */
void do_send_packet( PACKET *p )
{
   char *output;

   output = ( *imc_vinfo[IMC_VERSION].generate )( p );

   if( output )
   {
      imc_stats.tx_pkts++;
      if( strlen( output ) > imc_stats.max_pkt )
         imc_stats.max_pkt = strlen( output );
      do_imcsend( output );
   }
}

void imc_send( PACKET *p )
{
   if( !this_imcmud || this_imcmud->state == CONN_NONE ) 
   {
      imcbug( "%s", "imc_send when not active!" );
      return;
   }
  
   /* initialize packet fields that the caller shouldn't/doesn't set */

   p->i.path[0] = '\0';
  
   p->i.sequence = imc_sequencenumber++;
   if( !imc_sequencenumber )
      imc_sequencenumber++;
  
   strlcpy( p->i.to, p->to, IMC_NAME_LENGTH );
   strlcpy( p->i.from, p->from, IMC_NAME_LENGTH );
   strlcat( p->i.from, "@", IMC_NAME_LENGTH - strlen(p->i.from ) );
   strlcpy( p->i.from + strlen( p->i.from ), imc_name, IMC_NAME_LENGTH - strlen( p->i.from ) );

   do_send_packet( p );
}

/* delete the info entry "p" */
void imc_delete_reminfo( REMOTEINFO *p )
{
   IMCUNLINK( p, first_rinfo, last_rinfo, next, prev );
   IMCSTRFREE( p->name );
   IMCSTRFREE( p->version );
   IMCSTRFREE( p->netname );

   imc_cancel_event( NULL, p );
   IMCDISPOSE( p );
}

/* send a keepalive, and queue the next keepalive event */
void ev_keepalive( void *data )
{
   imc_send_keepalive();
}

/* send a keepalive request to everyone - shogar */
void imc_request_keepalive( void )
{
   PACKET out;

   imc_initdata( &out );
   strlcpy( out.type, "keepalive-request", IMC_TYPE_LENGTH );
   strlcpy( out.from, "*", IMC_TYPE_LENGTH );
   strlcpy( out.to, "*@*", IMC_TYPE_LENGTH );
   imc_addkey( &out, "versionid", imc_versionid );

   imc_send( &out );
   imc_freedata( &out );
}

void ev_request_keepalive( void *data )
{
   imc_request_keepalive();
}

/* put a line onto descriptors output buffer */
void do_imcsend( char *line )
{
   int len;
   char *newbuf;
   int newsize = this_imcmud->outsize;

   if( this_imcmud->state == CONN_NONE )
      return;

   if( !this_imcmud->outbuf[0] )
      this_imcmud->newoutput = 1;

   len = strlen( this_imcmud->outbuf ) + strlen( line ) + 3;

   if( len > this_imcmud->outsize )
   {
      while( newsize < len )
         newsize *= 2;

	IMCCREATE( newbuf, char, newsize );
      strlcpy( newbuf, this_imcmud->outbuf, newsize );
      IMCDISPOSE( this_imcmud->outbuf );
      this_imcmud->outbuf = newbuf;
      this_imcmud->outsize = newsize;
   }
   if( len < this_imcmud->outsize/2 && len >= IMC_MINBUF )
   {
      newsize = this_imcmud->outsize/2;

      IMCCREATE( newbuf, char, newsize );
      strlcpy( newbuf, this_imcmud->outbuf, newsize );
      IMCDISPOSE( this_imcmud->outbuf );
      this_imcmud->outbuf = newbuf;
      this_imcmud->outsize = newsize;
   }

   strlcat( this_imcmud->outbuf, line, newsize );

   if ( imc_debug_on )
      imclog(line);

   strlcat( this_imcmud->outbuf, "\n", newsize );
}

/* read waiting data from descriptor.
 * read to a temp buffer to avoid repeated allocations
 */
void do_imcread( void )
{
   int size;
   int r;
   char temp[IMC_MAXBUF];
   char *newbuf;
   int newsize = 0;

   r = read( this_imcmud->desc, temp, IMC_MAXBUF-1 );
   if( !r || ( r < 0 && errno != EAGAIN && errno != EWOULDBLOCK ) )
   {
      if( r < 0 )                    /* read error */
         imclog( "Read error on connection to %s", this_imcmud->hubname );
      else                        /* socket was closed */
         imclog( "EOF encountered on connection to %s", this_imcmud->hubname );

      imc_shutdown( TRUE );
      return;
   }

   if( r < 0 )			/* EAGAIN error */
      return;

   temp[r] = '\0';

   size = strlen( this_imcmud->inbuf ) + r + 1;

   if( size >= this_imcmud->insize )
   {
      newsize = this_imcmud->insize;

      while( newsize < size )
         newsize *= 2;

      IMCCREATE( newbuf, char, newsize );
      strlcpy( newbuf, this_imcmud->inbuf, newsize );
      IMCDISPOSE( this_imcmud->inbuf );
      this_imcmud->inbuf = newbuf;
      this_imcmud->insize = newsize;
   }

   if( size < this_imcmud->insize/2 && size >= IMC_MINBUF )
   {
      newsize = this_imcmud->insize;
      newsize /= 2;

      IMCCREATE( newbuf, char, newsize );
      strlcpy( newbuf, this_imcmud->inbuf, newsize );
      IMCDISPOSE( this_imcmud->inbuf );
      this_imcmud->inbuf = newbuf;
      this_imcmud->insize = newsize;
   }

   strlcat( this_imcmud->inbuf, temp, this_imcmud->insize );

   if (imc_debug_on)
      imclog(temp);

   imc_stats.rx_bytes += r;
}

/* write to descriptor */
void do_imcwrite( void )
{
   int size, w;

   if( this_imcmud->state == CONN_SENDCLIENTPWD )
   {
      /* Wait for server password */
      this_imcmud->state = CONN_WAITSERVERPWD;
      return;
   }

   size = strlen( this_imcmud->outbuf );
   if( !size )			/* nothing to write */
      return;

   w = write( this_imcmud->desc, this_imcmud->outbuf, size );
   if( !w || ( w < 0 && errno != EAGAIN && errno != EWOULDBLOCK ) )
   {
      if( w < 0 )			/* write error */
         imcbug( "Write error on connection to %s", this_imcmud->hubname );
      else			/* socket was closed */
         imcbug( "EOF encountered on connection to %s", this_imcmud->hubname );

      imc_shutdown( TRUE );
      return;
   }

   if( w < 0 )			/* EAGAIN */
      return;

   /* throw away data we wrote */
   strlcpy( this_imcmud->outbuf, this_imcmud->outbuf + w, this_imcmud->outsize );

   imc_stats.tx_bytes += w;
}

/*  try to read a line from the input buffer, NULL if none ready
 *  all lines are \n terminated in theory, but take other combinations
 */
char *imcgetline( char *buffer )
{
   int i;
   static char buf[IMC_PACKET_LENGTH];

   /* copy until \n, \r, end of buffer, or out of space */
   for( i = 0; buffer[i] && buffer[i] != '\n' && buffer[i] != '\r' && i+1 < IMC_PACKET_LENGTH; i++ )
      buf[i] = buffer[i];

   /* end of buffer and we haven't hit the maximum line length */
   if( !buffer[i] && i+1 < IMC_PACKET_LENGTH )
   {
      buf[0] = '\0';
      return NULL; /* so no line available */
   }

   /* terminate return string */
   buf[i] = '\0';

   /* strip off extra control codes */
   while( buffer[i] && ( buffer[i] == '\n' || buffer[i] == '\r' ) )
      i++;

   /* remove the line from the input buffer */
   strlcpy( buffer, buffer+i, this_imcmud->insize );

   return buf;
}

/* handle a password response from a server - for connection negotiation. DON'T REMOVE THIS */
void serverpassword( char *argument )
{
   char arg1[3], name[IMC_MNAME_LENGTH], pw[IMC_PW_LENGTH], version[20];
   int rversion;

   argument = imc_getarg( argument, arg1, 4 );	/* has to be PW */
   argument = imc_getarg( argument, name, IMC_MNAME_LENGTH );
   argument = imc_getarg( argument, pw, IMC_PW_LENGTH );
   argument = imc_getarg( argument, version, 20 );

   if( strcasecmp( arg1, "PW" ) )
   {
      imclog( "%s: non-PW password packet", this_imcmud->hubname );
      imc_shutdown( FALSE );
      return;
   }

   if( !this_imcmud || strcasecmp( this_imcmud->serverpw, pw ) )
   {
	imclog( "%s", "Password failure for hub!" );
      imc_shutdown( FALSE );
      return;
   }

   this_imcmud->state        = CONN_COMPLETE;

   /* check for a version string (assume version 0 if not present) */
   if( sscanf( version, "version=%d", &rversion ) != 1 )
      rversion = 0;

   /* check for generator/interpreter */
   if( !imc_vinfo[rversion].generate || !imc_vinfo[rversion].interpret )
   {
      imclog( "%s: Unsupported version %d", this_imcmud->hubname, rversion );
      imc_shutdown( FALSE );
      return;
   }

   imclog( "%s: Connected (version %d)", this_imcmud->hubname, rversion );
}

/* start up IMC */
bool imc_startup_network( void )
{
  
   imclog( "%s", "IMC2 Network Initializing" );

   imc_stats.start    = imc_now;
   imc_stats.rx_pkts  = 0;
   imc_stats.tx_pkts  = 0;
   imc_stats.rx_bytes = 0;
   imc_stats.tx_bytes = 0;
   imc_stats.sequence_drops = 0;

   /* Connect to Hub */
   if( !imc_connect_to() )
      return FALSE;

   imc_add_event( 20, ev_keepalive, NULL, 1 );
   /* fill my imcmlist please - shogar */
   imc_add_event( 30, ev_request_keepalive, NULL, 1 );

   return TRUE;
}

void imc_freechardata( CHAR_DATA *ch )
{
   if( IS_NPC(ch) )
	return;

   if( IMC_CSUBSCRIBED(ch) )
	IMCSTRFREE( IMC_CSUBSCRIBED(ch) );
   if( IMC_RREPLY(ch) )
	IMCSTRFREE( IMC_RREPLY(ch) );

   IMCDISPOSE( CH_IMCDATA(ch) );

   return;
}

void imc_initchar( CHAR_DATA *ch )
{
   if( IS_NPC(ch) )
	return;

   IMCCREATE( PC(ch)->imcchardata, IMC_CHARDATA, 1 );
   IMC_CSUBSCRIBED(ch)	= NULL;
   IMC_RREPLY(ch)	= NULL;
   IMC_PFLAGS(ch) = 0;
   FIRST_IMCBLACKLIST(ch) = NULL;
   LAST_IMCBLACKLIST(ch)  = NULL;

   return;
}

/*
 * Read a string from file fp using IMCSTRALLOC [Taken from Smaug's fread_string]
 */
const char *imcfread_string( FILE *fp )
{
    char buf[LSS];
    char *plast;
    char c;
    int ln;

    plast = buf;
    buf[0] = '\0';
    ln = 0;

    /*
     * Skip blanks.
     * Read first char.
     */
    do
    {
	if ( feof(fp) )
	{
	    imcbug( "%s", "imcfread_string: EOF encountered on read." );
	    return IMCSTRALLOC( "" );
	}
	c = getc( fp );
    }
    while ( isspace(c) );

    if ( ( *plast++ = c ) == '~' )
	return IMCSTRALLOC( "" );

    for ( ;; )
    {
	if ( ln >= (LSS - 1) )
	{
	     imcbug( "%s", "imcfread_string: string too long" );
	     *plast = '\0';
	     return IMCSTRALLOC( buf );
	}
	switch ( *plast = getc( fp ) )
	{
	default:
	    plast++; ln++;
	    break;

	case EOF:
	    imcbug( "%s", "imcfread_string: EOF" );
	    *plast = '\0';
	    return IMCSTRALLOC( buf );
	    break;

	case '\n':
	    plast++;  ln++;
	    *plast++ = '\r';  ln++;
	    break;

	case '\r':
	    break;

	case '~':
	    *plast = '\0';
	    return IMCSTRALLOC( buf );
	}
    }
}

/*
 * Read a number from a file. [Taken from Smaug's fread_number]
 */
int imcfread_number( FILE *fp )
{
    int number;
    bool sign;
    char c;

    do
    {
        if ( feof(fp) )
        {
          imclog( "%s", "imcfread_number: EOF encountered on read." );
          return 0;
        }
	c = getc( fp );
    }
    while ( isspace(c) );

    number = 0;

    sign   = FALSE;
    if ( c == '+' )
    {
	c = getc( fp );
    }
    else if ( c == '-' )
    {
	sign = TRUE;
	c = getc( fp );
    }

    if ( !isdigit(c) )
    {
	imclog( "imcfread_number: bad format. (%c)", c );
	return 0;
    }

    while ( isdigit(c) )
    {
        if ( feof(fp) )
        {
          imclog( "%s", "imcfread_number: EOF encountered on read." );
          return number;
        }
	number = number * 10 + c - '0';
	c      = getc( fp );
    }

    if ( sign )
	number = 0 - number;

    if ( c == '|' )
	number += imcfread_number( fp );
    else if ( c != ' ' )
	ungetc( c, fp );

    return number;
}

/*
 * Read to end of line into static buffer [Taken from Smaug's fread_line]
 */
char *imcfread_line( FILE *fp )
{
    static char line[LSS];
    char *pline;
    char c;
    int ln;

    pline = line;
    line[0] = '\0';
    ln = 0;

    /*
     * Skip blanks.
     * Read first char.
     */
    do
    {
	if ( feof(fp) )
	{
	    imcbug( "%s", "imcfread_line: EOF encountered on read." );
	    strlcpy( line, "", LSS );
	    return line;
	}
	c = getc( fp );
    }
    while ( isspace(c) );

    ungetc( c, fp );
    do
    {
	if ( feof(fp) )
	{
	    imcbug( "%s", "imcfread_line: EOF encountered on read." );
	    *pline = '\0';
	    return line;
	}
	c = getc( fp );
	*pline++ = c; ln++;
	if ( ln >= (LSS - 1) )
	{
	    imcbug( "%s", "imcfread_line: line too long" );
	    break;
	}
    }
    while ( c != '\n' && c != '\r' );

    do
    {
	c = getc( fp );
    }
    while ( c == '\n' || c == '\r' );

    ungetc( c, fp );
    *pline = '\0';
    return line;
}

/*
 * Read one word (into static buffer). [Taken from Smaug's fread_word]
 */
char *imcfread_word( FILE *fp )
{
    static char word[MSS];
    char *pword;
    char cEnd;

    do
    {
	if ( feof(fp) )
	{
	    imclog( "%s", "imcfread_word: EOF encountered on read." );
	    word[0] = '\0';
	    return word;
	}
	cEnd = getc( fp );
    }
    while ( isspace( cEnd ) );

    if ( cEnd == '\'' || cEnd == '"' )
    {
	pword   = word;
    }
    else
    {
	word[0] = cEnd;
	pword   = word+1;
	cEnd    = ' ';
    }

    for ( ; pword < word + MSS; pword++ )
    {
	if ( feof(fp) )
	{
	    imclog( "%s", "imcfread_word: EOF encountered on read." );
	    *pword = '\0';
	    return word;
	}
	*pword = getc( fp );
	if ( cEnd == ' ' ? isspace(*pword) : *pword == cEnd )
	{
	    if ( cEnd == ' ' )
		ungetc( *pword, fp );
	    *pword = '\0';
	    return word;
	}
    }

    imclog( "%s", "imcfread_word: word too long" );
    return NULL;
}

/*
 * Read a letter from a file. [Taken from Smaug's fread_letter]
 */
char imcfread_letter( FILE *fp )
{
    char c;

    do
    {
        if ( feof(fp) )
        {
          imclog( "%s", "imcfread_letter: EOF encountered on read." );
          return '\0';
        }
	c = getc( fp );
    }
    while ( isspace(c) );

    return c;
}

/*
 * Read to end of line (for comments). [Taken from Smaug's fread_to_eol]
 */
void imcfread_to_eol( FILE *fp )
{
    char c;

    do
    {
	if ( feof(fp) )
	{
	    imclog( "%s", "imcfread_to_eol: EOF encountered on read." );
	    return;
	}
	c = getc( fp );
    }
    while ( c != '\n' && c != '\r' );

    do
    {
	c = getc( fp );
    }
    while ( c == '\n' || c == '\r' );

    ungetc( c, fp );
    return;
}

bool imc_loadchar( CHAR_DATA *ch, FILE *fp, const char *word )
{
   if( IS_NPC(ch) )
	 return FALSE;

   if ( feof(fp))
       return FALSE;

   if( !strcasecmp( word, "IMCPFlags" ) )
   {
       IMC_PFLAGS(ch) = imcfread_number(fp);
       return TRUE;
   }

   if ( !strcasecmp( word, "IMCCSubscribed"))
   {
       IMC_CSUBSCRIBED(ch) = imcfread_string(fp);
       return TRUE;
   }
   
   if( !strcasecmp( word, "IMCBlacklist" ) )
   {
       IMC_BLACKLIST *temp;

       IMCCREATE( temp, IMC_BLACKLIST, 1 );
       temp->name = imcfread_string( fp );
       IMCLINK( temp, FIRST_IMCBLACKLIST(ch), LAST_IMCBLACKLIST(ch), next, prev );
       return TRUE;
   }

   return FALSE;
}

void imc_savechar( CHAR_DATA *ch, FILE *fp )
{
   IMC_BLACKLIST *temp;

   if( IS_NPC(ch) )
	return;

   if( IMC_PFLAGS(ch) )
      fprintf( fp, "IMCPFlags      %ld\n", IMC_PFLAGS(ch) );
   if( IMC_CSUBSCRIBED(ch) )
      fprintf( fp, "IMCCSubscribed      %s~\n", IMC_CSUBSCRIBED(ch) );
   for( temp = FIRST_IMCBLACKLIST(ch); temp; temp = temp->next )
      fprintf( fp, "IMCBlacklist	%s~\n", temp->name );
   return;
}

void imcfread_config_file( FILE *fin )
{
   char *word;
   bool fMatch;

   for( ;; )
   {
	word   = feof( fin ) ? "end" : imcfread_word( fin );
	fMatch = FALSE;
	
	switch( word[0] ) 
	{
	   case '#':
		fMatch = TRUE;
		imcfread_to_eol( fin );
		break;
	   case 'A':
		KEY( "Autoconnect",	this_imcmud->autoconnect,	imcfread_number( fin ) );
		KEY( "AdminLevel",	imc_minadminlevel,			imcfread_number( fin ) );
		break;
	   case 'C':
		
		
		KEY( "ClientPwd",	this_imcmud->clientpw,		imcfread_string( fin ) );
		break;
	   case 'D':
	   	KEY( "Debug",		imc_debug_on,		imcfread_number( fin ) );
	   	break;
	   case 'E':
		if( !strcasecmp( word, "End" ) )
		{
		   return;
		}
		break;
       case 'H':
        KEY( "HubAddr",	this_imcmud->host,		imcfread_string( fin ) );
        KEY( "HubName",	this_imcmud->hubname,		imcfread_string( fin ) );
        KEY( "HubPort",	this_imcmud->port,		imcfread_number( fin ) );
        break;

	   case 'I':
		KEY( "InfoName",		imc_siteinfo.name,	imcfread_string( fin ) );
		KEY( "InfoHost",		imc_siteinfo.host,	imcfread_string( fin ) );
		KEY( "InfoPort",		imc_siteinfo.port,	imcfread_number( fin ) );
		KEY( "InfoEmail",		imc_siteinfo.email,	imcfread_string( fin ) );
		KEY( "InfoWWW",		imc_siteinfo.www,		imcfread_string( fin ) );
		KEY( "InfoDetails",	imc_siteinfo.details,	imcfread_string( fin ) );
		KEY( "InfoBase",		imc_siteinfo.base,	imcfread_string( fin ) );
		break;
	   case 'L':
		KEY( "LocalName",		imc_name,			imcfread_string( fin ) );
		break;
	   case 'M':
		KEY( "MinPlayerLevel",		imc_minplayerlevel,		imcfread_number( fin ) );
        KEY( "MinImmLevel",     imc_minimmlevel,        imcfread_number( fin ) );
		break;
       case 'N':
        KEY( "NetworkName",     imc_siteinfo.netname, imcfread_string( fin ) ); 
       case 'S':
        KEY( "ServerPwd",	this_imcmud->serverpw,		imcfread_string( fin ) );
        break;
       case 'W':
        KEY( "WhoStyle",       imc_whostyle,                  imcfread_number( fin ) );
        break;

	}

	if( !fMatch ) 
	   imcbug( "imcfread_config_file: Bad keyword: %s\n", word );
   }
}

/* There should only one of these..... */
void imc_delete_info( void )
{
   if( this_imcmud->hubname )
      IMCSTRFREE( this_imcmud->hubname );
   if( this_imcmud->host )
      IMCSTRFREE( this_imcmud->host );
   if( this_imcmud->clientpw )
      IMCSTRFREE( this_imcmud->clientpw );
   if( this_imcmud->serverpw )
      IMCSTRFREE( this_imcmud->serverpw );

   imc_cancel_event( NULL, this_imcmud );

   IMCDISPOSE( this_imcmud );
}

bool imc_read_config( void ) 
{
   FILE *fin;
   char cbase[MSS];

   if( this_imcmud != NULL )
	imc_delete_info();
   this_imcmud = NULL;

   imclog( "%s", "Loading IMC2 network data..." );

   if( ( fin = dfopen(IMC_PATH, IMC_CONFIG_FILE, "r" ) ) == NULL ) 
   {
	imclog( "%s", "Can't open configuration file" );
	imclog( "%s", "Network configuration aborted." );
	return FALSE;
   }

   for( ; ; )
   {
    	char letter;
 	char *word;

   	letter = imcfread_letter( fin );

	if( letter == '#' )
	{
	   imcfread_to_eol( fin );
	   continue;
      }

	if( letter != '$' )
	{
	   imcbug( "%s", "imc_read_config: $ not found" );
	   break;
	}

	word = imcfread_word( fin );
	if( !strcasecmp( word, "IMCCONFIG" ) && this_imcmud == NULL )
	{
	   IMCCREATE( this_imcmud, HUBINFO, 1 );

	   imc_minplayerlevel = 10;
       imc_minimmlevel = IMCMAX_LEVEL;
	   imc_minadminlevel = IMCMAX_LEVEL;
       imc_debug_on = 0;
       imc_whostyle = 0;

	   imcfread_config_file( fin );
	   continue;
	}
      else if( !strcasecmp( word, "END" ) )
	   break;
	else
	{
	   imcbug( "imc_read_config: Bad section in config file: %s", word );
	   continue;
      }
   }
   SFCLOSE( fin );

   if( !this_imcmud )
   {
	imcbug( "%s", "imc_read_config: No hub connection information!!" );
	imcbug( "%s", "Network configuration aborted." );
	return FALSE;
   }

   if( !this_imcmud->hubname || !this_imcmud->host || !this_imcmud->clientpw || !this_imcmud->serverpw || !this_imcmud->port )
   {
	imcbug( "%s", "imc_read_config: Missing required configuration info." );
	imcbug( "%s", "Network configuration aborted." );
	return FALSE;
   }

   if( !imc_name || imc_name[0] == '\0' )
   {
	imcbug( "%s", "imc_read_config: Mud name not loaded in configuration file." );
	imcbug( "%s", "Network configuration aborted." );
	return FALSE;
   }

   if( !imc_siteinfo.netname || imc_siteinfo.netname[0] == '\0' )
   {
    imcbug( "%s", "imc_read_config: Missing NetworkName parameter in configuration file." );
    imcbug( "%s", "Network configuration aborted." );
    return FALSE;
   }

   if( !imc_siteinfo.name || imc_siteinfo.name[0] == '\0' )
   {
	imcbug( "%s", "imc_read_config: Missing InfoName parameter in configuration file." );
	imcbug( "%s", "Network configuration aborted." );
	return FALSE;
   }

   if( !imc_siteinfo.host || imc_siteinfo.host[0] == '\0' )
   {
	imcbug( "%s", "imc_read_config: Missing InfoHost parameter in configuration file." );
	imcbug( "%s", "Network configuration aborted." );
	return FALSE;
   }

   if( !imc_siteinfo.email || imc_siteinfo.email[0] == '\0' )
   {
	imcbug( "%s", "imc_read_config: Missing InfoEmail parameter in configuration file." );
	imcbug( "%s", "Network configuration aborted." );
	return FALSE;
   }

   if( !imc_siteinfo.base || imc_siteinfo.base[0] == '\0' )
	imc_siteinfo.base = IMCSTRALLOC( "Unknown Codebase" );

   if( !imc_siteinfo.www || imc_siteinfo.www[0] == '\0' )
	imc_siteinfo.www = IMCSTRALLOC( "Not Specified" );

   if( !imc_siteinfo.details || imc_siteinfo.details[0] == '\0' )
	imc_siteinfo.details = IMCSTRALLOC( "No details provided." );

   if( imc_versionid )
	IMCSTRFREE( imc_versionid );

   snprintf( cbase, MSS, "%s %s", IMC_VERSION_ID, imc_siteinfo.base );

   imc_versionid = IMCSTRALLOC( cbase );

   return TRUE;
}

/* read an IMC blacklist file */
void imc_readblacklist( void )
{
   FILE *inf;
   char *word;
   char temp[IMC_NAME_LENGTH];
   IMC_BLACKLIST *entry;

   if( !( inf = dfopen(IMC_PATH, IMC_BLACKLIST_FILE, "r" ) ) )
   {
      imcbug( "%s", "imc_readblacklist: couldn't open blacklist file" );
      return;
   }

   word = imcfread_word( inf );
   if( strcasecmp( word, "#BLACKLIST" ) )
   {
	imcbug( "%s", "imc_readblacklist: Corrupt file" );
	SFCLOSE( inf );
	return;
   }

   while( !feof( inf ) && !ferror( inf ) )
   {
	strlcpy( temp, imcfread_word( inf ), IMC_NAME_LENGTH );
	if( !strcasecmp( temp, "#END" ) )
	{
	   SFCLOSE( inf );
	   return;
	}

    IMCCREATE( entry, IMC_BLACKLIST, 1 );
    entry->name = IMCSTRALLOC( temp );
    IMCLINK( entry, first_imc_mudblacklist, last_imc_mudblacklist, next, prev );
   }

   if( ferror( inf ) )
   {
      perror( "imc_readblacklist" );
      SFCLOSE( inf );
      return;
   }

   SFCLOSE( inf );
   return;
}

void imc_loadhistory( void )
{
   char filename[SSS];
   FILE *tempfile;
   IMC_CHANNEL *tempchan = NULL;
   int x;

   for( tempchan = first_imc_channel; tempchan; tempchan = tempchan->next )
   {
	if( !tempchan->local_name )
	   continue;

      snprintf( filename, SSS, "%s/%s.hist", IMC_PATH, tempchan->local_name );

      if( ( tempfile = fopen(filename, "r" ) ) == NULL ) 
	   continue;

      for( x = 0; x < MAX_IMCHISTORY; x++ )
      {
         if( feof( tempfile ) )
		tempchan->history[x] = NULL;
	   else
            tempchan->history[x] = IMCSTRALLOC( imcfread_line( tempfile ) );
	}

    SFCLOSE( tempfile );
	unlink( filename );
   }
}

void imc_savehistory( void )
{
   char filename[SSS];
   FILE *tempfile;
   IMC_CHANNEL *tempchan = NULL;
   int x;

   for( tempchan = first_imc_channel; tempchan; tempchan = tempchan->next )
   {
	if( !tempchan->local_name )
	   continue;

	if( !tempchan->history[0] )
	   continue;

	snprintf( filename, SSS, "%s/%s.hist", IMC_PATH, tempchan->local_name );

	if( ( tempfile = fopen(filename, "w" ) ) == NULL ) 
	   continue;

	for( x = 0; x < MAX_IMCHISTORY; x++ )
	{
	    if( tempchan->history[x] != NULL )
    	    fprintf( tempfile, "%s", tempchan->history[x] );
	}
	SFCLOSE( tempfile );
   }
}

void imc_startup( bool force )
{
   if( this_imcmud && this_imcmud->state > CONN_NONE )
   {
      imclog( "imc_startup: called with a connection already up" );
      return;
   }

   imc_now = time( NULL );                  /* start our clock */
   imc_boot = imc_now;

   imc_sequencenumber = imc_now;

   /* Do not re-read configuration file on a network error reconnect. File reading is slow,
      and it will just be a waste of resources */
   if( !imcwait && !imc_read_config() )
	return;

   if( this_imcmud->autoconnect )
    force = TRUE;

   if( !force )
   {
	imclog( "%s", "IMC2 data loaded. Autoconnect not set. IMC will need to be connected manually." );
	return;
   }

   if( !imc_startup_network() )
       return;
    
   imc_initchannels();
   imc_loadhistory();
   imc_readblacklist();

   return;
}

void imc_freechan( IMC_CHANNEL *c )
{
   int x;

   if( !c ) /* How? */
   {
	imcbug( "%s", "imc_freechan: Freeing NULL channel!" );
	return;
   }
   IMCUNLINK( c, first_imc_channel, last_imc_channel, next, prev );
   if( c->name )
      IMCSTRFREE( c->name );
   if( c->owner )
      IMCSTRFREE( c->owner );
   if( c->operators )
      IMCSTRFREE( c->operators );
   if( c->invited )
      IMCSTRFREE( c->invited );
   if( c->excluded )
      IMCSTRFREE( c->excluded );
   if( c->local_name )
      IMCSTRFREE( c->local_name );
   if( c->regformat )
      IMCSTRFREE( c->regformat );
   if( c->emoteformat )
      IMCSTRFREE( c->emoteformat );
   if( c->socformat )
      IMCSTRFREE( c->socformat );

   for( x = 0; x < MAX_IMCHISTORY; x++ )
   {
	if( c->history[x] && c->history[x] != '\0' )
	   IMCSTRFREE( c->history[x] );
   }
   IMCDISPOSE( c );
   return;
}

void imc_shutdownchannels( void )
{
   IMC_CHANNEL *ic, *icnext;

   for( ic = first_imc_channel; ic; ic = icnext )
   {
	icnext = ic->next;

	imc_freechan( ic );
   }
}

void imc_shutdown_network( void )
{
   imc_event *ev, *ev_next;
   REMOTEINFO *p, *pnext;

   imclog( "%s", "Shutting down network" );

   imclog( "rx %ld packets, %ld bytes (%ld/second)", imc_stats.rx_pkts, imc_stats.rx_bytes,
	(imc_now == imc_stats.start) ? 0 : imc_stats.rx_bytes / (imc_now - imc_stats.start) );

   imclog( "tx %ld packets, %ld bytes (%ld/second)", imc_stats.tx_pkts, imc_stats.tx_bytes,
	(imc_now == imc_stats.start) ? 0 : imc_stats.tx_bytes / (imc_now - imc_stats.start));

   imclog( "largest packet %d bytes", imc_stats.max_pkt );

   imclog( "dropped %d packets by sequence number", imc_stats.sequence_drops );

   close( this_imcmud->desc );
   this_imcmud->desc = -1;
   this_imcmud->state = CONN_NONE;

   imc_savehistory();
   imc_shutdownchannels();

   for( p = first_rinfo; p; p = pnext )
   {
      pnext = p->next;
	imc_delete_reminfo( p );
   }

   for( ev = imc_event_list; ev; ev = ev_next )
   {
      ev_next = ev->next;
      IMCDISPOSE( ev );
   }
   for( ev = imc_event_free; ev; ev = ev_next )
   {
      ev_next = ev->next;
      IMCDISPOSE( ev );
   }
   imc_event_list = imc_event_free = NULL;
}

/* close down IMC */
void imc_shutdown( bool reconnect )
{
   IMC_BLACKLIST *entry, *entry_next;

   if( !this_imcmud || this_imcmud->state == CONN_NONE )
   {
      imcbug( "%s", "imc_shutdown: called without a hub connection up" );
      return;
   }

   imc_shutdown_network( );

   for( entry = first_imc_mudblacklist; entry; entry = entry_next )
   {
      entry_next = entry->next;

      if( entry->name )
         IMCSTRFREE( entry->name );

      IMCUNLINK( entry, first_imc_mudblacklist, last_imc_mudblacklist, next, prev );

      IMCDISPOSE( entry );
   }

   if( reconnect )
   {
      imcwait = 100; /* About 20 seconds or so */
	  imclog( "%s", "Connection to hub was lost. Reconnecting in approximately 20 seconds." );
   }

   return;
}

/* interpret an incoming packet using the right version */
PACKET *do_interpret_packet( char *line )
{
   int v;
   PACKET *p;

   if( !line[0] )
      return NULL;

   v = IMC_VERSION;

   p = (*imc_vinfo[v].interpret)(line);

   return p;
}

int imc_fill_fdsets( int maxfd, fd_set *iread, fd_set *iwrite, fd_set *exc )
{
   if( !this_imcmud || this_imcmud->state == CONN_NONE )
      return maxfd;

   /* set up fd_sets for select */

   if( maxfd < this_imcmud->desc )
      maxfd = this_imcmud->desc;

   switch( this_imcmud->state )
   {
      case CONN_SENDCLIENTPWD:	/* connected/error when writable */
         FD_SET( this_imcmud->desc, iwrite );
         break;
      case CONN_COMPLETE:
      case CONN_WAITSERVERPWD:
         FD_SET( this_imcmud->desc, iread );
         if( this_imcmud->outbuf && this_imcmud->outbuf[0] != '\0' )
	      FD_SET( this_imcmud->desc, iwrite );
         break;
   }
   return maxfd;
}

/* low-level idle function: read/write buffers as needed, etc */
void imc_idle_select( fd_set *iread, fd_set *iwrite, fd_set *exc, time_t now )
{
   char *command;
   PACKET *p;
   int error, error_len;

   if( this_imcmud->desc < 1  || this_imcmud->state == CONN_NONE)
	return;

   if( imc_sequencenumber < (unsigned long)imc_now )
      imc_sequencenumber = (unsigned long)imc_now;

   imc_run_events( now );

   /* an exception was raised in the select for the descriptor */
   if( this_imcmud->state != CONN_NONE && FD_ISSET( this_imcmud->desc, exc ) )
   {
      imc_shutdown( TRUE );
      return;
   }

   /* some data has been sent to the descriptor - insert it into the input buffer */
   if( this_imcmud->state != CONN_NONE && FD_ISSET( this_imcmud->desc, iread ) )
      do_imcread( );
   
   /* process the input buffer */
   while( this_imcmud->state != CONN_NONE && ( command = imcgetline( this_imcmud->inbuf ) ) != NULL )
   {
      if( strlen( command ) > imc_stats.max_pkt )
	   imc_stats.max_pkt = strlen( command );

      switch( this_imcmud->state )
      {
         REMOTEINFO *route;

         case CONN_NONE:
	      break;
         case CONN_WAITSERVERPWD:
	      serverpassword( command );
	      break;
         case CONN_COMPLETE:
          

          if (!(p = do_interpret_packet( command )))
              break;

	      imc_stats.rx_pkts++;

          /* check for duplication, and register the packet in the sequence memory */
          if( p->i.sequence && checkrepeat( imc_mudof( p->i.from ), p->i.sequence ) )
              break;

          /* check for really old packets */
          route = imc_find_reminfo( imc_mudof( p->i.from ), 1 );
          
          if( route && ( p->i.sequence + IMC_PACKET_LIFETIME ) < route->top_sequence )
          {
              imc_stats.sequence_drops++;
              break; 
          }

          if( route && p->i.sequence > route->top_sequence )
              route->top_sequence = p->i.sequence;

          /* update our routing info */
          updateroutes( p->i.path );

          /* Receive it if it's for us, otherwise it gets silently dropped */
          if( !strcasecmp( imc_mudof( p->i.to ), "*" ) || !strcasecmp( imc_mudof( p->i.to ), imc_name ) )
          {
              strlcpy( p->to, imc_nameof( p->i.to ), IMC_NAME_LENGTH );    /* strip the name from the 'to' */
              strlcpy( p->from, p->i.from, IMC_NAME_LENGTH );

              imc_recv( p );
          }

          imc_freedata( p );
          break;
      }
   }

	if( this_imcmud->desc > 0 ) { // Something could have caused shutdown during reading
		if (this_imcmud->in_progress
		&&  FD_ISSET(this_imcmud->desc, iwrite)) {
			error_len = sizeof(error);
			getsockopt(this_imcmud->desc, SOL_SOCKET,
				   SO_ERROR, &error, &error_len);
		if (error) {
			imcbug("Coulnd't connect to host %s:%d, reason: %s",
				this_imcmud->host, this_imcmud->port,
				strerror(error));
		imc_shutdown(TRUE);
		return;
		}
	this_imcmud->in_progress = FALSE;
	}

	if (this_imcmud->state != CONN_NONE
	&&  !this_imcmud->in_progress
	&&  (FD_ISSET(this_imcmud->desc, iwrite) || this_imcmud->newoutput)) {
		do_imcwrite();
		this_imcmud->newoutput = this_imcmud->outbuf[0];
	}
	}
}

void imc_loop( void )
{
   fd_set in_set, out_set, exc_set;
   static struct timeval null_time;
   int maxdesc = 0;

	if (!this_imcmud)
		return;

   /* Reconnect in case of a network error */
   if( imcwait > 1 )
      imcwait--;
   else if ( imcwait == 1 )
   {
       imc_startup(TRUE);
       imcwait = 0;
   }

   if( this_imcmud->state == CONN_NONE || this_imcmud->desc == -1 )
	return;

   FD_ZERO( &in_set  );
   FD_ZERO( &out_set );
   FD_ZERO( &exc_set );

   maxdesc = imc_fill_fdsets( maxdesc, &in_set, &out_set, &exc_set );
   if( select( maxdesc+1, &in_set, &out_set, &exc_set, &null_time ) < 0 )
   {
	perror( "imc_loop: select: poll" );
	imc_shutdown( TRUE );
	return;
   }
   imc_idle_select( &in_set, &out_set, &exc_set, current_time );
   return;
}

IMC_CHANNEL *imc_findchannel( const char *name )
{
   IMC_CHANNEL *c;

   for( c = first_imc_channel; c; c = c->next )
      if( !strcasecmp( c->name, name ) )
         return c;
   return NULL;
}

IMC_CHANNEL *imc_findlchannel( const char *name )
{
   IMC_CHANNEL *c;

   for( c = first_imc_channel; c; c = c->next )
      if( c->local_name && !strcasecmp( c->local_name, name ) )
         return c;
  return NULL;
}

void imc_recv_msg_r( const char *from, const char *realfrom, const char *chan, const char *txt, int emote )
{
   IMC_CHANNEL *c;
   const char *mud;

   mud = imc_mudof( from );

   /* forged? */
   if( !strchr( chan, ':' ) || strcasecmp( mud, channel_mudof( chan ) ) )
      return;

   c = imc_findchannel( chan );
   if( !c )
      return;

   if( !c->local_name || ( c->policy != CHAN_PRIVATE && c->policy != CHAN_CPRIVATE ) )
      return;

   /* We assume that anything redirected is automatically audible - since we trust the hub... 
    * What's all this *WE* business? *I* don't trust it :)
    */
   imc_showchannel( c, realfrom, txt, emote );
}

void imc_recv_msg_b( const char *from, const char *chan, const char *txt, int emote )
{
   IMC_CHANNEL *c;

   c = imc_findchannel( chan );
   if( !c )
      return;

   if( !c->local_name || c->policy == CHAN_PRIVATE || c->policy == CHAN_CPRIVATE )
      return;
  
   if( !imc_audible( c, from ) )
      return;
  
   imc_showchannel( c, from, txt, emote );
}

void imc_recv_update( const char *from, const char *chan, const char *owner, const char *operators, const char *policy, const char *invited, const char *excluded )
{
   IMC_CHANNEL *c;
   const char *mud;

   mud = imc_mudof( from );

   /* forged? */
   if( !strchr( chan, ':' ) || strcasecmp( mud, channel_mudof( chan ) ) )
      return;

   c = imc_findchannel( chan );

   if( !c )
   {
	IMCCREATE( c, IMC_CHANNEL, 1 );
      c->name = IMCSTRALLOC( chan );
      c->owner = IMCSTRALLOC( owner );
      c->operators = IMCSTRALLOC( operators );
      c->invited = IMCSTRALLOC( invited );
      c->excluded = IMCSTRALLOC( excluded );
      c->local_name = NULL;
	c->perm_level = PERM_ADMIN;
	c->refreshed = TRUE;
	IMCLINK( c, first_imc_channel, last_imc_channel, next, prev );
   }
   else
   {
	if( c->name )
         IMCSTRFREE( c->name );
 	if( c->owner )
         IMCSTRFREE( c->owner );
	if( c->operators )
         IMCSTRFREE( c->operators );
	if( c->invited )
         IMCSTRFREE( c->invited );
	if( c->excluded )
         IMCSTRFREE( c->excluded );
      c->name = IMCSTRALLOC( chan );
      c->owner = IMCSTRALLOC( owner );
      c->operators = IMCSTRALLOC( operators );
      c->invited = IMCSTRALLOC( invited );
      c->excluded = IMCSTRALLOC( excluded );
	c->refreshed = TRUE;
   }

   if( !strcasecmp( policy, "open" ) )
      c->policy = CHAN_OPEN;
   else if( !strcasecmp( policy, "closed" ) )
      c->policy = CHAN_CLOSED;
   else if( !strcasecmp( policy, "copen" ) )
      c->policy = CHAN_COPEN;
   else if( !strcasecmp( policy, "cprivate" ) )
      c->policy = CHAN_CPRIVATE;
   else
      c->policy = CHAN_PRIVATE;

   if( c->local_name && !imc_audible( c, imc_name ) )
      imc_freechan( c );
}

void imc_save_channels( void )
{
   IMC_CHANNEL *c;
   FILE *fp;
   char name[LSS];

   strlcpy( name, IMC_CHANNEL_FILE, LSS );

   fp = dfopen(IMC_PATH, name, "w" );
   if( !fp )
   {
      imcbug( "Can't write to %s", name );
      return;
   }
  
   for( c = first_imc_channel; c; c = c->next )
   {
	if( !c->local_name || c->local_name[0] == '\0' )
	   continue;

      fprintf( fp, "%s", "#IMCCHAN\n" );
	fprintf( fp, "ChanName   %s~\n", c->name );
	fprintf( fp, "ChanLocal  %s~\n", c->local_name );
	fprintf( fp, "ChanRegF   %s~\n", c->regformat );
	fprintf( fp, "ChanEmoF   %s~\n", c->emoteformat );
	fprintf( fp, "ChanSocF   %s~\n", c->socformat );
	fprintf( fp, "ChanLevel  %d\n",  c->perm_level );
	fprintf( fp, "%s", "End\n\n" );
   }
   fprintf( fp, "%s", "#END\n" );
   SFCLOSE( fp );
}

void imc_recv_destroy( const char *from, const char *channel )
{
   IMC_CHANNEL *c;
   const char *mud;

   mud = imc_mudof( from );

   if( !strchr( channel, ':' ) || strcasecmp( mud, channel_mudof( channel ) ) )
      return;

   c = imc_findchannel( channel );
   if( !c )
      return;

   imc_freechan( c );
   imc_save_channels();
}

void ev_imc_firstrefresh( void *dummy )
{
   PACKET out;

   if( !this_imcmud || this_imcmud->state < CONN_COMPLETE  )
      return;
  
   strlcpy( out.from, "*", IMC_NAME_LENGTH );
   strlcpy( out.to, "IMC@*", IMC_NAME_LENGTH );
   strlcpy( out.type, "ice-refresh", IMC_TYPE_LENGTH );
   imc_initdata( &out );
   imc_addkey( &out, "channel", "*" );
   imc_send( &out );
   imc_freedata( &out );
}

int imc_recv_chan( PACKET *p, int bcast )
{
   /* redirected msg */
   if( !strcasecmp( p->type, "ice-msg-r" ) )
   {
      imc_recv_msg_r( p->from, imc_getkey( p, "realfrom", "" ), imc_getkey( p, "channel", "" ),
	   imc_getkey( p, "text", "" ), imc_getkeyi( p, "emote", 0 ) );
        return 1;
   }
   else if( !strcasecmp( p->type, "ice-msg-b" ) )
   {
      imc_recv_msg_b( p->from, imc_getkey( p, "channel", "" ), imc_getkey( p, "text", "" ), imc_getkeyi( p, "emote", 0 ) );
      return 1;
   }
   else if( !strcasecmp( p->type, "ice-update" ) )
   {
      imc_recv_update( p->from, imc_getkey( p, "channel", "" ), imc_getkey( p, "owner", "" ),
         imc_getkey( p, "operators", "" ), imc_getkey( p, "policy", ""),
         imc_getkey( p, "invited", "" ), imc_getkey( p, "excluded", "" ) );
      return 1;
   }
   else if( !strcasecmp( p->type, "ice-destroy" ) )
   {
      imc_recv_destroy( p->from, imc_getkey( p, "channel", "" ) );
      return 1;
   }
   else
      return 0;
}

void imc_readchannel( IMC_CHANNEL *channel, FILE *fp )
{
   char *word;
   bool fMatch;

   for ( ; ; )
   {
	word   = feof( fp ) ? "End" : imcfread_word( fp );
	fMatch = FALSE;

	switch( toupper(word[0]) )
	{
	  case '*':
	    fMatch = TRUE;
	    imcfread_to_eol( fp );
	    break;

	  case 'C':
		KEY( "ChanName",		channel->name,		imcfread_string( fp ) );
		KEY( "ChanLocal",		channel->local_name,	imcfread_string( fp ) );
		KEY( "ChanRegF",		channel->regformat,	imcfread_string( fp ) );
		KEY( "ChanEmoF",		channel->emoteformat,	imcfread_string( fp ) );
		KEY( "ChanSocF",		channel->socformat,	imcfread_string( fp ) );
		KEY( "ChanLevel",		channel->perm_level,		imcfread_number( fp ) );
		break;

	  case 'E':
	    if ( !strcasecmp( word, "End" ) )
		return;
	    break;
	}

	if( !fMatch )
	    imcbug( "imc_readchannel: no match: %s", word );
   }
}

void imc_loadchannels( void )
{
   FILE *fp;
   IMC_CHANNEL *channel;

   first_imc_channel = NULL;
   last_imc_channel = NULL;

   fp = dfopen(IMC_PATH, IMC_CHANNEL_FILE, "r" );
   if( !fp )
   {
      imcbug( "%s", "Can't open imc channel file" );
      return;
   }

   for ( ; ; )
   {
	char letter;
	char *word;

	letter = imcfread_letter( fp );
	if ( letter == '*' )
	{
	   imcfread_to_eol( fp );
	   continue;
	}

	if( letter != '#' )
	{
	   imcbug( "%s", "imc_loadchannels: # not found." );
	   break;
	}

      word = imcfread_word( fp );
	if ( !strcasecmp( word, "IMCCHAN" ) )
	{
	   int x;

	   IMCCREATE( channel, IMC_CHANNEL, 1 );
	   imc_readchannel( channel, fp );

	   for( x = 0; x < MAX_IMCHISTORY; x++ )
		channel->history[x] = NULL;

	   channel->refreshed = FALSE; // Prevents crash trying to use a bogus channel
	   IMCLINK( channel, first_imc_channel, last_imc_channel, next, prev );
         imclog( "configured %s as %s", channel->name, channel->local_name );
	   continue;
	}
	else
         if ( !strcasecmp( word, "END"	) )
	        break;
	else
	{
	   imcbug( "imc_loadchannels: bad section: %s.", word );
	   continue;
	}
    }
    SFCLOSE( fp );
    return;
}

/* global init */
void imc_initchannels( void )
{
   imclog( "%s", "IMC Channel client starting..." );

   imc_recv_chain = imc_recv_hook;
   imc_recv_hook = imc_recv_chan;

   imc_add_event( 60, ev_imc_firstrefresh, NULL, 1 );
   imc_loadchannels();
}

/* need exactly 2 %s's, and no other format specifiers */
bool verify_format( const char *fmt, short sneed )
{
   const char *c;
   int i = 0;

   c = fmt;
   while( ( c = strchr( c, '%' ) ) != NULL )
   {
      if( *( c + 1 ) == '%' )  /* %% */
      {
         c += 2;
         continue;
      }
    
      if( *( c + 1 ) != 's' )  /* not %s */
         return FALSE;

      c++;
      i++;
   }
   if( i != sneed )
      return FALSE;

   return TRUE;
}

void run_imccadmin( CHAR_DATA *ch, const char *argument )
{
   char cmd[IMC_NAME_LENGTH];
   char chan[IMC_NAME_LENGTH];
   char data[IMC_DATA_LENGTH];
   char *p;
   PACKET out;
   IMC_CHANNEL *c;

   CHECKIMC(ch);
  
   p = imc_getarg( argument, cmd, IMC_NAME_LENGTH );
   p = imc_getarg( p, chan, IMC_NAME_LENGTH );
   strlcpy( data, p, IMC_DATA_LENGTH );

   if( !cmd[0] || !chan[0] )
   {
      imc_to_char( "Syntax: imccadmin <command> <hub:channelname> [<data..>]\n", ch );
	return;
   }

   p = strchr( chan, ':' );
   if( !p )
   {
      c = imc_findlchannel( chan );
      if( c )
         strlcpy( chan, c->name, IMC_NAME_LENGTH );
   }

   snprintf( out.to, IMC_NAME_LENGTH, "IMC@%s", channel_mudof( chan ) );
   strlcpy( out.type, "ice-cmd", IMC_TYPE_LENGTH );
   strlcpy( out.from, CH_IMCNAME(ch), IMC_NAME_LENGTH );
   imc_initdata( &out );
   imc_addkey( &out, "channel", chan );
   imc_addkey( &out, "command", cmd );
   imc_addkey( &out, "data", data );

   imc_send( &out );
   imc_freedata( &out );

   imc_to_char( "Command sent.\n", ch );
   return;
}

void run_imccsettings( CHAR_DATA *ch, const char *argument )
{
   char channame[IMC_NAME_LENGTH], field[IMC_NAME_LENGTH];
   IMC_CHANNEL *channel = NULL;
   bool local = FALSE;

   CHECKIMC(ch);
  
   smash_tilde(argument);
   argument = imc_getarg( argument, channame, IMC_NAME_LENGTH );
   argument = imc_getarg( argument, field, IMC_NAME_LENGTH );

   if( !channame || channame[0] == '\0' || !field || field[0] == '\0' )
   {
      imc_to_char( color_itom(
                 "~GSyntax: ~cimccsettings  <channel name> <field> [value]\n\n"
                 "~cWhere the field may be one of the following:\n"
                 "~C  localize unlocalize rename permlevel regformat emoteformat socformat\n"
                 "~cAll changes are saved on edit. More information on each field may be obtained\n"
                 "~cby typing a field name without an accompanying value. Type \"~Gimccsettings\n"
                  "<channel name> show~c\" for a display of the current values for a channel.\n"
                   ), ch);
      return;
   }

   if( (channel = imc_findlchannel(channame)) == NULL && (channel = imc_findchannel(channame)) == NULL)
   {
       imc_to_char( "That channel could not be found.\n", ch );
       return;
   }

   if (channel->local_name && channel->local_name[0] != '\0')
       local = TRUE;

   if( !strcasecmp( field, "show" ) )
   {
       char buf[LSS];
       char *polly;
	
       /* for compress policy - shogar - 1/29/2000 */
       switch( channel->policy )
       {
           case CHAN_OPEN:
               polly = "~cOpen -- Open to all muds";
               break;
           case CHAN_COPEN:
               polly = "~cCOpen -- Compressed Open channel";
               break;
           case CHAN_CLOSED:
               polly = "~RClosed -- Sent to all muds but only invited people can talk or listen";
               break;
           case CHAN_PRIVATE:
               polly = "~yPrivate -- Only invited people can talk or listen";
               break;
           case CHAN_CPRIVATE:
               polly = "~yCPrivate -- Compressed Private channel";
               break;
           default:
               polly = "~wUnknown -- We don't know what this is";
               break;
	
       }
       
       snprintf( buf, LSS, 
               "~WChannel %s:\n"
               "~c  Local name~W: ~C%s\n"
               "~c  Regformat ~W: ~C%s\n"
               "~c  Emformat  ~W: ~C%s\n"
               "~c  Socformat ~W: ~C%s\n"
               "~c  Level     ~W: ~C%d\n"
               "\n"
               "~c  Policy    ~W: ~C%s\n"
               "~c  Owner     ~W: ~C%s\n"
               "~c  Operators ~W: ~C%s\n"
               "~c  Invited   ~W: ~C%s\n"
               "~c  Excluded  ~W: ~C%s\n",
               channel->name, channel->local_name ? channel->local_name : "", 
               channel->regformat ? channel->regformat : "", channel->emoteformat ? channel->emoteformat : "",
               channel->socformat ? channel->socformat : "", channel->perm_level ? channel->perm_level : 0,
               polly, channel->owner, channel->operators, channel->invited, channel->excluded );

       imc_to_char( color_itom(buf), ch );
       return;
   }

   if( !strcasecmp( field, "localize" ) )
   {
       char formatstring[MSS];
      
       if( !argument || argument[0] == '\0' || strlen(argument) > IMC_MNAME_LENGTH 
               || (strchr( argument, ' ' )) ) 
       {
           imc_to_char( color_itom(
                       "~cDescription: ~CLocalize sets the localname to use the channel. It also\n"
                       "~C           ~Ccreates creates defaults for all three format strings.\n"
                       "~cAccepted Values: ~CTakes any string without spaces up to 20 characters.\n"
                       ), ch);
           return;
       }
	
       if( local )
       {
           char_printf( ch, "Channel %s has already been localized as %s.\n", 
                   channel->name, channel->local_name );
           return;
       }
	
       if( channel->local_name )
           IMCSTRFREE( channel->local_name );
       channel->local_name = IMCSTRALLOC( argument );
	
       if( channel->regformat )
           IMCSTRFREE( channel->regformat );
       snprintf( formatstring, MSS, "~R[~Y%s~R] ~C%%s: ~c%%s", channel->local_name );
       channel->regformat = IMCSTRALLOC( color_itom(formatstring) );

       if( channel->emoteformat )
           IMCSTRFREE( channel->emoteformat );
       snprintf( formatstring, MSS, "~R[~Y%s~R] ~c%%s %%s", channel->local_name );
       channel->emoteformat = IMCSTRALLOC( color_itom(formatstring) );

       if( channel->socformat )
           IMCSTRFREE( channel->socformat );
       snprintf( formatstring, MSS, "~R[~Y%s~R] ~c%%s", channel->local_name );
       channel->socformat = IMCSTRALLOC( color_itom(formatstring) );
	
       char_printf( ch, "Channel %s has now been localized as %s.\n", 
               channel->name, channel->local_name );
       imc_save_channels();
       return;
   }

   if( !strcasecmp( field, "unlocalize" ) )
   {
       if( !argument || argument[0] == '\0' || strcasecmp( argument, "now" ) )
       {
           imc_to_char( color_itom(
                       "~cDescription: ~CUnlocalize clears the localname, the permission level,\n"
                       "~C             and all three format strings for a channel.\n" 
                       "~cAccepted Values: ~CTakes the value \"now\" to confirm action.\n" 
                       ), ch);
           return;
       }
	
       if( !local )
       {
           imc_to_char( "That channel has not been localized.\n", ch );
           return;
       }
	
       if( channel->local_name )
           IMCSTRFREE( channel->local_name );
       if( channel->regformat )
           IMCSTRFREE( channel->regformat );
       if( channel->emoteformat )
           IMCSTRFREE( channel->emoteformat );
       if( channel->socformat )
           IMCSTRFREE( channel->socformat );

       channel->perm_level = PERM_ADMIN;
	
       char_printf( ch, "Channel %s has been unlocalized successfully.\n", 
               channel->name);
       imc_save_channels();
       return;
   }

   if( !strcasecmp( field, "rename" ) )
   {
       if( !argument || argument[0] == '\0' || strlen(argument) > IMC_MNAME_LENGTH 
               || (strchr( argument, ' ' )) ) 
       {
           imc_to_char( color_itom(
                       "~cDescription: ~CRename modifies the localname for using the channel.\n" 
                       "~cAccepted Values: ~CTakes any string without spaces up to 20 characters.\n"
                       ), ch);
           return;
       }
	
       if( !local )
       {
           imc_to_char( "That channel has not been localized.\n", ch );
           return;
       }

       if ( imc_findlchannel( argument ))
       {
           imc_to_char( "That localname already exists.\n", ch );
           return;
       }
	
       if( channel->local_name )
           IMCSTRFREE( channel->local_name );
       channel->local_name = IMCSTRALLOC( argument );

       char_printf( ch, "Channel %s has had its local name successfully changed to %s.\n", 
               channel->name, channel->local_name);
       imc_save_channels();
       return;
   }

   if( !strcasecmp( field, "permlevel" ) )
   {
       int value = 0;
      
       if( !argument || argument[0] == '\0' || !is_number(argument))
       {
           imc_to_char( color_itom(
                       "~cDescription: ~CPermlevel sets the minimum permission level for using a\n"
                       "~C             localized channel.\n"
                       "~cAccepted Values: ~CTakes any integer between 1 and the admin permission level.\n"
                       ), ch);
           return;
       }

       value = atoi(argument);

       if ( value < 1 || value > PERM_ADMIN )
       {
           char_printf( ch, "That value was outside of the range of 1 to %d.\n", PERM_ADMIN );
           return;
       }
	
       if( !local )
       {
           imc_to_char( "That channel has not been localized.\n", ch );
           return;
       }

       channel->perm_level = value;

       char_printf( ch, "That channel has had its minimum permission level changed to %d.\n", value );
       imc_save_channels();
       return;
   }

   if( !strcasecmp( field, "regformat" ) )
   {
       if( !argument || argument[0] == '\0' || strlen(argument) > SSS )
       {
           imc_to_char( color_itom(
                       "~cDescription: ~CRegformat modifies the format string for regular channel use.\n" 
                       "~cAccepted Values: ~CTakes any string up to 256 characters with two %s's.\n"
                       ), ch);
           return;
       }

       if( !verify_format( argument, 2 ) )
       {
           imc_to_char( "The regular format must contain two %s's.\n", ch );
           return;
       }
	
       if( !local )
       {
           imc_to_char( "That channel has not been localized.\n", ch );
           return;
       }
	
       if( channel->regformat )
           IMCSTRFREE( channel->regformat );
       channel->regformat = IMCSTRALLOC( argument );

       char_printf( ch, "Channel %s has successfully had its regular format string changed.\n", 
               channel->name );
       imc_save_channels();
       return;
   }

   if( !strcasecmp( field, "emoteformat" ) )
   {
       if( !argument || argument[0] == '\0' || strlen(argument) > SSS )
       {
           imc_to_char( color_itom(
                       "~cDescription: ~CEmoteformat modifies the format string for emote channel use.\n" 
                       "~cAccepted Values: ~CTakes any string up to 256 characters with two %s's.\n"
                       ), ch);
           return;
       }

       if( !verify_format( argument, 2 ) )
       {
           imc_to_char( "The emote format must contain two %s's.\n", ch );
           return;
       }
	
       if( !local )
       {
           imc_to_char( "That channel has not been localized.\n", ch );
           return;
       }
	
       if( channel->emoteformat )
           IMCSTRFREE( channel->emoteformat );
       channel->emoteformat = IMCSTRALLOC( argument );

       char_printf( ch, "Channel %s has successfully had its emote format string changed.\n", 
               channel->name );
       imc_save_channels();
       return;
   }

   if( !strcasecmp( field, "socformat" ) )
   {
       if( !argument || argument[0] == '\0' || strlen(argument) > SSS )
       {
           imc_to_char( color_itom(
                       "~cDescription: ~CSocformat modifies the format string for social channel use.\n" 
                       "~cAccepted Values: ~CTakes any string up to 256 characters with one %s.\n"
                       ), ch);
           return;
       }

       if( !verify_format( argument, 1 ) )
       {
           imc_to_char( "The social format must contain one %s.\n", ch );
           return;
       }
	
       if( !local )
       {
           imc_to_char( "That channel has not been localized.\n", ch );
           return;
       }
	
       if( channel->socformat )
           IMCSTRFREE( channel->socformat );
       channel->socformat = IMCSTRALLOC( argument );

       char_printf( ch, "Channel %s has successfully had its social format string changed.\n", 
               channel->name );
       imc_save_channels();
       return;
   }

   run_imccsettings( ch, "" );
   return;
}

void run_imcclist( CHAR_DATA *ch, const char *argument )
{
   char buf[LSS];
   char *polly;
   IMC_CHANNEL *c;
   bool allpriv = FALSE;

   CHECKIMC(ch);

   if ( argument && !strcasecmp( argument, "all" ))
       allpriv = TRUE;

   /* added level to the display for imcclist with no arguments for 2.00 - shogar - /1/26/2000 */
   snprintf( buf, LSS, "~W%-15s %-15s %-15s %-7s %s\n", 
           "Name", "Local name", "Owner", "Level", "Policy" );

   /* for compress policy - shogar - 1/29/2000 */
   for( c = first_imc_channel; c; c = c->next )
   {

       if( !imc_audible( c, imc_makename( CH_IMCNAME(ch), imc_name ) ) && c->refreshed && !allpriv)
           continue;

           switch( c->policy )
           {
               case CHAN_OPEN:
                   polly = "~copen";
                   break;
               case CHAN_COPEN:
                   polly = "~ccopen";
                   break;
               case CHAN_CLOSED:
                   polly = "~Rclosed";
                   break;
               case CHAN_PRIVATE:
                   polly = "~yprivate";
                   break;
               case CHAN_CPRIVATE:
                   polly = "~ycprivate";
                   break;
               default:
                   polly = "~wUnknown";
                   break;
           }

       snprintf( buf+strlen(buf), LSS, 
               "~c%-15.15s ~C%-*.*s ~g%-15.15s ~G%-7d %s\n", c->name, c->local_name ? 15 : 17, 
               c->local_name ? 15 : 17, c->local_name ? c->local_name : "~R(not local)  ", c->owner,
               c->perm_level ? c->perm_level : 0, polly );
   }

   imc_to_pager( color_itom(buf), ch );

   if (!allpriv)
   {
       imc_to_char( color_itom( 
                        "\n~WType \"~Yimcclist all~W\" to see the entire listing, including private\n"
                            "~Wchannels that you are not invited to.\n"), ch );
   }

   return;
}

void imc_save_config( void )
{
   FILE *fp;

   if( ( fp = dfopen(IMC_PATH, IMC_CONFIG_FILE, "w" ) ) == NULL ) 
   {
	imclog( "%s", "Couldn't write to config file." );
	return;
   }

   fprintf( fp, "%s", "$IMCCONFIG\n\n" );
   fprintf( fp, "%s", "# This is the IMC2 4.00 Continuum version of the IMC2 config file.\n" );
   fprintf( fp, "%s", "# When changing this information, be sure you don't remove the tildes!\n" );
   fprintf( fp, "%s", "# This information can be edited online using the 'imcmsettings' command.\n\n\n" );
   fprintf( fp, "%s", "# Your connection information goes here.\n\n\n" );
   fprintf( fp, "%s", "# This is the name that the network will know your mud as.\n");
   fprintf( fp, "LocalName    %s~\n\n", imc_name );
   fprintf( fp, "%s", "# This is the name of the IMC2 network that your mud will be connected to.\n");
   fprintf( fp, "NetworkName  %s~\n\n", imc_siteinfo.netname );
   fprintf( fp, "%s", "# This is the network name of the hub that you will connect to.\n");
   fprintf( fp, "HubName   %s~\n\n", this_imcmud->hubname );
   fprintf( fp, "%s", "# This is the hostname or ip address of the hub that you will connect to.\n");
   fprintf( fp, "HubAddr   %s~\n\n", this_imcmud->host );
   fprintf( fp, "%s", "# This is the port on the host of the hub that you will connect to.\n");
   fprintf( fp, "HubPort   %d\n\n",  this_imcmud->port );
   fprintf( fp, "%s", "# This is the password that you will use to confirm who you are to the server.\n");
   fprintf( fp, "ClientPwd   %s~\n\n", this_imcmud->clientpw );
   fprintf( fp, "%s", "# This is the password that the server will use to confirm who it is to you.\n");
   fprintf( fp, "ServerPwd   %s~\n\n", this_imcmud->serverpw );
   fprintf( fp, "%s", "# This sets whether your mud will attempt to connect to IMC on startup.\n");
   fprintf( fp, "Autoconnect  %d\n\n",  this_imcmud->autoconnect );
   fprintf( fp, "%s", "# This option sets whether your mud will log incoming and outgoing IMC packets.\n");
   fprintf( fp, "Debug        %d\n\n\n",  imc_debug_on );
   fprintf( fp, "%s", "# Level settings for IMC access go here.\n\n\n");
   fprintf( fp, "%s", "# Minimum level for a player to use IMC.\n"); 
   fprintf( fp, "MinPlayerLevel     %d\n\n",  imc_minplayerlevel );
   fprintf( fp, "%s", "# Minimum level for someone to achieve immortal privileges.\n");
   fprintf( fp, "MinImmLevel        %d\n\n",  imc_minimmlevel );
   fprintf( fp, "%s", "# Minimum level for someone to achieve administrative privileges.\n");
   fprintf( fp, "AdminLevel         %d\n\n\n",  imc_minadminlevel );
   fprintf( fp, "%s", "# Information about your mud that you might want other muds to see goes here.\n\n\n");
   fprintf( fp, "%s", "# This is the full name of your mud that you wish to display.\n");
   fprintf( fp, "InfoName     %s~\n\n", imc_siteinfo.name );
   fprintf( fp, "%s", "# This is the hostname or ip address that you wish to have others connect to.\n");
   fprintf( fp, "InfoHost     %s~\n\n", imc_siteinfo.host );
   fprintf( fp, "%s", "# This is the port that you wish to have others connect to for your mud.\n");
   fprintf( fp, "InfoPort     %d\n\n",  imc_siteinfo.port );
   fprintf( fp, "%s", "# This is the email address that you would like to be contacted at.\n");
   fprintf( fp, "InfoEmail    %s~\n\n", imc_siteinfo.email );
   fprintf( fp, "%s", "# This is the mud website that you would like interested parties to go to.\n");
   fprintf( fp, "InfoWWW      %s~\n\n", imc_siteinfo.www );
   fprintf( fp, "%s", "# This is the codebase that your mud is based upon.\n");
   fprintf( fp, "InfoBase     %s~\n\n", imc_siteinfo.base );
   fprintf( fp, "%s", "# This is a description of your mud that you would like other muds to see.\n");
   fprintf( fp, "InfoDetails  %s~\n\n\n", imc_siteinfo.details );
   fprintf( fp, "%s", "# Miscellaneous Configuration Components.\n\n\n");
   fprintf( fp, "%s", "# Sets the color pattern for IMC2, current options are:\n");
   fprintf( fp, "%s", "# 0 - Continuum Default 1 - Winter 2 - Fiery Bowels of Hell 3 - IMC2 Classic\n");
   fprintf( fp, "WhoStyle        %d\n\n\n", imc_whostyle );

   fprintf( fp, "%s", "End\n\n" );
   fprintf( fp, "%s", "$END\n" );
   SFCLOSE( fp );
   return;
}

/* Save current mud-level blacklist. Short, simple. */
void imc_saveblacklist( void )
{
   FILE *out;
   IMC_BLACKLIST *entry;

   if( !( out = dfopen(IMC_PATH, IMC_BLACKLIST_FILE, "w" ) ) )
   {
      imcbug( "%s", "imc_saveblacklist: error opening blacklist file for write" );
      return;
   }

   fprintf( out, "%s", "#BLACKLIST\n" );

   for( entry = first_imc_mudblacklist; entry; entry = entry->next )
      fprintf( out, "%s\n", entry->name );

   fprintf( out, "%s", "#END\n" );

   SFCLOSE( out );
   return;
}

/* send an emote out on a channel */
void imc_send_emote( imc_char_data *from, int channel, char *argument, char *to )
{
   PACKET out;
   char tobuf[IMC_MNAME_LENGTH];

   if( !this_imcmud || this_imcmud->state < CONN_COMPLETE )
      return;

   setdata( &out, from );

   strlcpy( out.type, "emote", IMC_TYPE_LENGTH );
   imc_addkeyi( &out, "channel", channel );
   imc_addkey( &out, "text", argument );

   to = imc_getarg( to, tobuf, IMC_MNAME_LENGTH );
   while( tobuf[0] )
   {
      if( !strcasecmp( tobuf, "*" ) || !strcasecmp( tobuf, imc_name ) || imc_find_reminfo( tobuf, 0 ) )
      {
         strlcpy( out.to, "*@", IMC_NAME_LENGTH );
         strlcat( out.to, tobuf, IMC_NAME_LENGTH );
         imc_send( &out );
      }
      to = imc_getarg( to, tobuf, IMC_MNAME_LENGTH );
   }
   imc_freedata( &out );
}

/* send a who-request to a remote mud */
void imc_send_who( imc_char_data *from, const char *to, const char *type )
{
   PACKET out;

   if( !this_imcmud || this_imcmud->state < CONN_COMPLETE )
      return;

   if( !strcasecmp( imc_mudof( to ), "*" ) )
      return; /* don't let them do this */

   setdata( &out, from );

   snprintf( out.to, IMC_NAME_LENGTH, "*@%s", to );
   strlcpy( out.type, "who", IMC_TYPE_LENGTH );

   imc_addkey( &out, "type", type );

   imc_send( &out );
   imc_freedata( &out );
}

/* send a whois-request to a remote mud */
void imc_send_whois( imc_char_data *from, const char *to )
{
   PACKET out;

   if( !this_imcmud || this_imcmud->state < CONN_COMPLETE )
      return;

   if( strchr( to, '@' ) )
      return;

   setdata( &out, from );

   snprintf( out.to, IMC_NAME_LENGTH, "%s@*", to );
   strlcpy( out.type, "whois", IMC_TYPE_LENGTH );

   imc_send( &out );
   imc_freedata( &out );
}

/* beep a remote player */
void imc_send_beep( imc_char_data *from, const char *to )
{
   PACKET out;

   if( !this_imcmud || this_imcmud->state < CONN_COMPLETE )
      return;

   if( !strcasecmp( imc_mudof( to ), "*" ) )
      return; /* don't let them do this */

   setdata( &out, from );
   strlcpy( out.type, "beep", IMC_TYPE_LENGTH );
   strlcpy( out.to, to, IMC_NAME_LENGTH );

   imc_send( &out );
   imc_freedata( &out );
}

void run_imcptell( CHAR_DATA *ch, const char *argument )
{
   char buf[LSS], buf1[LSS];
   imc_char_data *chdata = imc_getdata( ch );

   CHECKIMC( ch );

   argument = imc_getarg( argument, buf, LSS );

   if( !buf[0] || !strchr(buf, '@') || !argument[0] )
   {
      imc_to_char( "Who@Where do you want to imcptell?\n", ch );
      return;
   }

   if( !function_usable( ch, DENY_IMCPTELL, ALLOW_IMCPTELL, PERM_PLAYER ) )
   {
      imc_to_char( "You are not authorized to use imcptell.\n", ch );
      return;
   }

   if( !IS_IMCVISIBLE(ch) )
   {
      imc_to_char( "You are invisible.\n", ch );
      return;
   }

   if( IS_SET( IMC_PFLAGS(ch), PSET_IMCPTELL ) )
   {
      imc_to_char( "Enable incoming imcptells first ('imcpsettings +imcptell').\n", ch );
      return;
   }

   CHECKMUDOF( ch, buf );
   imc_send_tell( chdata, buf, color_mtoi(argument), 0 );
  
   snprintf( buf1, LSS, color_itom("~cYou imcptell ~C%s ~c'~W%s~c'\n"), buf, argument );
   imc_to_char( buf1, ch );

   IMCDISPOSE( chdata );
   return;
}

void run_imcpreply( CHAR_DATA *ch, const char *argument )
{
   char buf[LSS + 30];

   CHECKIMC(ch);

   if( !IMC_RREPLY(ch) )
   {
      imc_to_char( "You haven't received an imcptell yet.\n", ch );
      return;
   }

   if ( !argument || argument[0] == '\0')
   {
       imc_to_char( "What do you want to imcptell that person?\n", ch );
       return;
   }

   snprintf( buf, LSS + 30, "%s %s", IMC_RREPLY(ch), argument );

   run_imcptell( ch, buf );
   return;
}

void run_imcpbeep( CHAR_DATA *ch, const char *argument )
{
   char buf[LSS];
   imc_char_data *chdata = imc_getdata( ch );

   CHECKIMC(ch);

   if( !argument || argument[0] == '\0' || !strchr(argument, '@') )
   {
      imc_to_char( "Who@Where do you want to imcbeep?\n", ch );
      return;
   }

   if( !function_usable(ch, DENY_IMCPBEEP, ALLOW_IMCPBEEP, PERM_PLAYER) )
   {
      imc_to_char( "You are not authorized to imcpbeep.\n", ch );
      return;
   }

   if( !IS_IMCVISIBLE(ch) )
   {
      imc_to_char( "You are invisible.\n", ch );
      return;
   }

   if( IS_SET(IMC_PFLAGS(ch), PSET_IMCPBEEP) )
   {
      imc_to_char( "Enable incoming imcpbeep first ('imcpsettings +imcpbeep').\n", ch );
      return;
   }

   CHECKMUDOF( ch, argument );
   imc_send_beep( chdata, argument );
   snprintf( buf, LSS, color_itom("~cYou imcpbeep ~C%s~c.\n"), argument );
   
   imc_to_char( buf, ch );
   
   WAIT_STATE( ch, PULSE_PER_SECOND * 5 );
   
   IMCDISPOSE( chdata );
   return;
}

void run_imcpafk( CHAR_DATA *ch, const char *argument )
{
    CHECKIMC(ch);

    if ( IS_SET(IMC_PFLAGS(ch), PSET_IMCPAFK) )
    {
        imc_to_char( "You are no longer imcpafk.\n", ch );
        REMOVE_BIT( IMC_PFLAGS(ch), PSET_IMCPAFK); 
        return;
    }

    imc_to_char( "You are now imcpafk.\n", ch );
    SET_BIT( IMC_PFLAGS(ch), PSET_IMCPAFK);
    return;
}

void run_imcplist( CHAR_DATA *ch, const char *argument )
{
   imc_char_data *chdata = imc_getdata( ch );

   CHECKIMC(ch);

   if( !argument || argument[0] == '\0' )
   {
      imc_to_char( "Which mud do you want to imcplist?\n", ch );
      return;
   }
  
   CHECKMUD( ch, argument );
   imc_send_who( chdata, argument, "who" );
   IMCDISPOSE( chdata );
}

void run_imcpfind( CHAR_DATA *ch, const char *argument )
{
   imc_char_data *chdata = imc_getdata( ch );

   CHECKIMC(ch);

   if( !argument || argument[0] == '\0' )
   {
      imc_to_char( "Who do you want to imcpfind?\n", ch );
      return;
   }

   imc_send_whois( chdata, argument );
   IMCDISPOSE( chdata );
}

void run_imcpinfo( CHAR_DATA *ch, const char *argument )
{
   char name[LSS];
   imc_char_data *chdata = imc_getdata( ch );

   CHECKIMC(ch);

   if( !argument || argument[0] == '\0' || !strchr(argument, '@') )
   {
      imc_to_char( "Who@Where do you want to imcpinfo?\n", ch );
      return;
   }

   CHECKMUD( ch, imc_mudof(argument) );
   snprintf( name, LSS, "finger %s", imc_nameof(argument) );
   imc_send_who( chdata, imc_mudof(argument), name );
   IMCDISPOSE( chdata );
}

void run_imcminfo( CHAR_DATA *ch, const char *argument )
{
   char arg[LSS];
   imc_char_data *chdata = imc_getdata( ch );

   CHECKIMC(ch);

   argument = imc_getarg( argument, arg, LSS );

   if( !arg || arg[0] == '\0' )
   {
      imc_to_char( "Where do you want to imcminfo?\n", ch );
      return;
   }

   CHECKMUD( ch, arg );
   imc_send_who( chdata, arg, (argument && argument[0] != '\0' ? argument : "help" ) );
   IMCDISPOSE( chdata );
}

void run_imcmlist( CHAR_DATA *ch, const char *argument )
{
   CHECKIMC(ch);

   imc_to_pager( color_itom( imc_list() ), ch );
   return;
}

void run_imcmsettings( CHAR_DATA *ch, const char *argument )
{
   char arg1[MSS];

   if( !argument || argument[0] == '\0' )
   {
       imc_to_char( color_itom(
                  "~GSyntax: ~cimcmsettings  <field> [value]\n\n"
                  "~cWhere the field may be one of the following:\n"
                  "~G--Connection Related Fields --\n"
                  "~C  connected imcname hubname hubaddr hubport clientpwd serverpwd\n"
                  "~C  autoconnect debug netname\n"
                  "~G--Privilege Related Fields --\n"
                  "~C  minplayerlevel minimmlevel minadminlevel\n"
                  "~G--Information Display Related Fields\n"
                  "~C  infoname infohost infoport infoemail infowww infobase infodetails\n"
                  "~G--Miscellaneous Fields\n"
                  "~C  whostyle\n"
                  "~cAll changes are saved on edit. More information on each field may be obtained\n"
                  "~cby typing a field name without an accompanying value. Type \"~Gimcmsettings show~c\"\n"
                  "~cfor a display of the current values for all available fields.\n"
                    ), ch);
       return;
   }

   if( !strcasecmp( argument, "show" ) )
   {
       imc_to_char( color_itom("~WCurrent Values:\n"), ch );
       char_printf( ch, color_itom("~cMud IMC2 Name  ~W: ~C%s\n"), imc_name );
       char_printf( ch, color_itom("~cNetwork Name   ~W: ~C%s\n"), imc_siteinfo.netname );
       char_printf( ch, color_itom("~cConnected      ~W: ~C%s\n"), this_imcmud->state == CONN_COMPLETE 
               ? "Yes" : "No");
	   char_printf( ch, color_itom("~cHubName        ~W: ~C%s\n"), this_imcmud->hubname );
	   char_printf( ch, color_itom("~cHub Address    ~W: ~C%s\n"), this_imcmud->host );
	   char_printf( ch, color_itom("~cHub Port       ~W: ~C%d\n"), this_imcmud->port );
	   char_printf( ch, color_itom("~cClientPwd      ~W: ~C%s\n"), this_imcmud->clientpw );
	   char_printf( ch, color_itom("~cServerPwd      ~W: ~C%s\n"), this_imcmud->serverpw );
	   char_printf( ch, color_itom("~cAutoconnect    ~W: ~C%s\n"), this_imcmud->autoconnect ? "Yes" : "No");
	   char_printf( ch, color_itom("~cDebug          ~W: ~C%s\n"), imc_debug_on ? "Yes" : "No" );
	   char_printf( ch, color_itom("~cMinPlayerLevel ~W: ~C%d\n"), imc_minplayerlevel );
       char_printf( ch, color_itom("~cMinImmLevel    ~W: ~C%d\n"), imc_minimmlevel );
	   char_printf( ch, color_itom("~cAdminlevel     ~W: ~C%d\n"), imc_minadminlevel );
	   char_printf( ch, color_itom("~cInfoname       ~W: ~C%s\n"), imc_siteinfo.name );
	   char_printf( ch, color_itom("~cInfohost       ~W: ~C%s\n"), imc_siteinfo.host );
	   char_printf( ch, color_itom("~cInfoport       ~W: ~C%d\n"), imc_siteinfo.port );
	   char_printf( ch, color_itom("~cInfoemail      ~W: ~C%s\n"), imc_siteinfo.email );
	   char_printf( ch, color_itom("~cInfobase       ~W: ~C%s\n"), imc_siteinfo.base );
	   char_printf( ch, color_itom("~cInfoWWW        ~W: ~C%s\n"), imc_siteinfo.www );
	   char_printf( ch, color_itom("~cInfoDetails    ~W: ~C%s\n"), imc_siteinfo.details );
       char_printf( ch, color_itom("~cWhoStyle       ~W: ~C%s\n\n"), whostylename_table[imc_whostyle] );

	   return;
   }

   argument = imc_getarg( argument, arg1, MSS );
   smash_tilde(argument);

   if( !strcasecmp( arg1, "connected" ) ) 
   {
      if( !argument || argument[0] == '\0')
      {
          imc_to_char( color_itom(
                       "~cDescription: ~CConnected either connects or disconnects the IMC client\n"
                       "             ~Cfrom the hub. Also, it can stop the reconnect process\n"
                       "             ~Cwith \"no\".\n"
                       "~cAccepted Values: ~CTakes the values \"yes\" and \"no\".\n" 
                       ), ch);
          return;
      }

      if( !strcasecmp( argument, "yes" ) )
      {
          if (this_imcmud->state > CONN_NONE)
          {
              imc_to_char( "The IMC client is already connected to the hub.\n", ch );
              return;
          }
          
          imc_to_char( "The IMC client will now connect itself to the hub.\n", ch );
          imcwait = 0;
          imc_startup( TRUE );
          return;
      }

      if( !strcasecmp( argument, "no" ) )
      {
          if ( imcwait > 0 )
          {
              imc_to_char( "The IMC client will now stop its reconnect process to the hub.\n", ch );
              imcwait = 0;
              return;
          }
          
          if (this_imcmud->state < CONN_COMPLETE)
          {
              imc_to_char( "The IMC client is already disconnected from the hub.\n", ch );
              return;
          }

          imc_to_char( "The IMC client will now disconnect itself from the hub.\n", ch );
          imc_shutdown( FALSE );
          return;
      }

      run_imcmsettings(ch, "connected");
      return;
   }

   if( !strcasecmp( arg1, "imcname" ) )
   {
      if( !argument || argument[0] == '\0' || strlen(argument) > IMC_MNAME_LENGTH  
              || (strchr( argument, ' ' )) ) 
      {
          imc_to_char( color_itom(
                       "~cDescription: ~CImcname is the name that the IMC2 network will know\n"
                       "             ~Cthis mud as.\n" 
                       "~cAccepted Values: ~CTakes any string without spaces up to 20 characters.\n"
                       ), ch);
          return;
      }

      if( this_imcmud->state > CONN_NONE )
      {
          imc_to_char( "Imcname can not be modified while connected.\n", ch );
          return;
      }
	
      if( imc_name )
          IMCSTRFREE( imc_name );

	  imc_name = IMCSTRALLOC( argument );
	  char_printf( ch, "This mud will now be known to the network as %s\n", argument );
	  imc_save_config();
      return;
   }

   if( !strcasecmp( arg1, "hubname" ) )
   {
      if( !argument || argument[0] == '\0' || strlen(argument) > IMC_MNAME_LENGTH 
              || (strchr( argument, ' ' )) ) 
      {
          imc_to_char( color_itom(
                       "~cDescription: ~CHubname sets the name that IMC will call the hub.\n"
                       "~cAccepted Values: ~CTakes any string without spaces up to 20 characters.\n"
                       ), ch);
          return;
      }

      if( this_imcmud->state > CONN_NONE )
      {
          imc_to_char( "Hubname can not be modified while connected.\n", ch );
          return;
      }
	
      if( this_imcmud->hubname )
          IMCSTRFREE( this_imcmud->hubname );

	  this_imcmud->hubname = IMCSTRALLOC( argument );
	  char_printf( ch, "The hub will now be known as %s.\n", argument );
	  imc_save_config();
      return;
   }

   if( !strcasecmp( arg1, "hubaddr" ) )
   {
      if( !argument || argument[0] == '\0' || strlen(argument) > SSS 
              || (strchr( argument, ' ' )) ) 
      {
          imc_to_char( color_itom(
                       "~cDescription: ~CHubaddr sets the hostname or ip address that IMC\n"
                       "             ~Cwill use to try to connect to the hub.\n"
                       "~cAccepted Values: ~CTakes any string without spaces up to 256 characters.\n"
                       ), ch);
          return;
      }

      if( this_imcmud->state > CONN_NONE )
      {
          imc_to_char( "Hubaddr can not be modified while connected.\n", ch );
          return;
      }
	
      if( this_imcmud->host )
          IMCSTRFREE( this_imcmud->host );

	  this_imcmud->host = IMCSTRALLOC( argument );
	  char_printf( ch, "The hub is now located at %s.\n", argument );
	  imc_save_config();
      return;
   }

   if( !strcasecmp( arg1, "hubport" ) )
   {
      int value = 0;

      if( !argument || argument[0] == '\0' || !is_number(argument))
      {
          imc_to_char( color_itom(
                       "~cDescription: ~CHubport sets the port that IMC will try to connect\n"
                       "             ~Cto the hub at.\n"
                       "~cAccepted Values: ~CTakes any integer between 1 and 65,535.\n" 
                       ), ch);
          return;
      }

      if( this_imcmud->state > CONN_NONE )
      {
          imc_to_char( "Hubport can not be modified while connected.\n", ch );
          return;
      }

      value = atoi(argument);

      if ( value < 1 || value > 65535 )
      {
          imc_to_char( "That value was outside of the range of 1 to 65,535.\n", ch );
          return;
      }

      this_imcmud->port = value;
      char_printf( ch, "The hub is now located on port %d.\n", value );
	  imc_save_config();
	  return;
   }

   if( !strcasecmp( arg1, "clientpwd" ) )
   {
      if( !argument || argument[0] == '\0' || strlen(argument) > IMC_PW_LENGTH 
              || (strchr( argument, ' ' )) ) 
      {
          imc_to_char( color_itom(
                       "~cDescription: ~CClientpwd is the password that the IMC client uses to\n"
                       "             ~Cauthenticate itself to the hub.\n" 
                       "~cAccepted Values: ~CTakes any string without spaces up to 20 characters.\n"
                       ), ch);
          return;
      }

      if( this_imcmud->state > CONN_NONE )
      {
          imc_to_char( "Clientpwd can not be modified while connected.\n", ch );
          return;
      }
	
      if( this_imcmud->clientpw )
          IMCSTRFREE( this_imcmud->clientpw );

	  this_imcmud->clientpw = IMCSTRALLOC( argument );
	  char_printf( ch, "The client will authenticate itself to the hub with %s.\n", argument );
	  imc_save_config();
      return;
   }

   if( !strcasecmp( arg1, "serverpwd" ) )
   {
      if( !argument || argument[0] == '\0' || strlen(argument) > IMC_PW_LENGTH 
              || (strchr( argument, ' ' )) ) 
      {
          imc_to_char( color_itom(
                       "~cDescription: ~CServerpwd is the password that the IMC hub uses to\n"
                       "             ~Cauthenticate itself to the client.\n" 
                       "~cAccepted Values: ~CTakes any string without spaces up to 20 characters.\n"
                       ), ch);
          return;
      }

      if( this_imcmud->state > CONN_NONE )
      {
          imc_to_char( "Serverpwd can not be modified while connected.\n", ch );
          return;
      }
	
      if( this_imcmud->serverpw )
          IMCSTRFREE( this_imcmud->serverpw );

	  this_imcmud->serverpw = IMCSTRALLOC( argument );
	  char_printf( ch, "The hub will authenticate itself to the client with %s.\n", argument );
	  imc_save_config();
      return;
   }

   if( !strcasecmp( arg1, "autoconnect" ) ) 
   {
      if( !argument || argument[0] == '\0')
      {
          imc_to_char( color_itom(
                       "~cDescription: ~CAutoconnect sets whether or not IMC will start at mud boot.\n"
                       "~cAccepted Values: ~CTakes the values \"yes\" and \"no\".\n" 
                       ), ch);
          return;
      }

      if( !strcasecmp( argument, "yes" ) )
      {
          if (this_imcmud->autoconnect)
          {
              imc_to_char( "Autoconnect is already on.\n", ch );
              return;
          }
          
          this_imcmud->autoconnect = TRUE;
          imc_to_char( "Autoconnect is now on.\n", ch );
          imc_save_config();
          return;
      }

      if( !strcasecmp( argument, "no" ) )
      {
          if (!this_imcmud->autoconnect)
          {
              imc_to_char( "Autoconnect is already off.\n", ch );
              return;
          }

          this_imcmud->autoconnect = FALSE;
          imc_to_char( "Autoconnect is now off.\n", ch );
          imc_save_config();
          return;
      }

      run_imcmsettings(ch, "autoconnect");
      return;
   }

   if( !strcasecmp( arg1, "debug" ) ) 
   {
      if( !argument || argument[0] == '\0')
      {
          imc_to_char( color_itom(
                       "~cDescription: ~CDebug sets whether or not all incoming and outgoing\n"
                       "             ~Cpackets are logged.\n"
                       "~cAccepted Values: ~CTakes the values \"yes\" and \"no\".\n" 
                       ), ch);
          return;
      }

      if( !strcasecmp( argument, "yes" ) )
      {
          if (imc_debug_on)
          {
              imc_to_char( "Debug is already on.\n", ch );
              return;
          }
          
          imc_debug_on = TRUE;
          imc_to_char( "Debug is now on.\n", ch );
          imc_save_config();
          return;
      }

      if( !strcasecmp( argument, "no" ) )
      {
          if (!imc_debug_on)
          {
              imc_to_char( "Debug is already off.\n", ch );
              return;
          }

          imc_debug_on = FALSE;
          imc_to_char( "Debug is now off.\n", ch );
          imc_save_config();
          return;
      }

      run_imcmsettings(ch, "debug");
      return;
   }

   if( !strcasecmp( arg1, "netname" ) )
   {
      if( !argument || argument[0] == '\0' || strlen(argument) > 10  
              || (strchr( argument, ' ' )) ) 
      {
          imc_to_char( color_itom(
                       "~cDescription: ~CNetname is the name of the IMC2 network that this mud\n"
                       "             ~Cis connected to. The name must be the exact same as the\n"
                       "             ~Chub in order to connect to a IMC2 3.11 Hermes or later hub.\n"
                       "~cAccepted Values: ~CTakes any string without spaces up to 10 characters.\n"
                       ), ch);
          return;
      }

      if( this_imcmud->state > CONN_NONE )
      {
          imc_to_char( "Netname can not be modified while connected.\n", ch );
          return;
      }
	
      if( imc_siteinfo.netname )
          IMCSTRFREE( imc_siteinfo.netname );

	  imc_siteinfo.netname = IMCSTRALLOC( argument );
	  char_printf( ch, "The IMC2 network that this mud is connected to will now be known as %s\n", 
              argument );
	  imc_save_config();
      return;
   }

   if( !strcasecmp( arg1, "minplayerlevel" ) )
   {
      int value = 0;

      if( !argument || argument[0] == '\0' || !is_number(argument))
      {
          imc_to_char( color_itom(
                       "~cDescription: ~CMinplayerlevel sets the minimum level for a player to use IMC.\n"
                       "~cAccepted Values: ~CTakes any integer between 1 and the mud's maximum level.\n" 
                       ), ch);
          return;
      }

      value = atoi(argument);

      if ( value < 1 || value > IMCMAX_LEVEL )
      {
          char_printf( ch, "That value was outside of the range of 1 to %d.\n", IMCMAX_LEVEL );
          return;
      }

      imc_minplayerlevel = value;
      char_printf( ch, "The minimum player access level has been set to %d.\n", value );
	  imc_save_config();
	  return;
   }

   if( !strcasecmp( arg1, "minimmlevel" ) )
   {
      int value = 0;

      if( !argument || argument[0] == '\0' || !is_number(argument))
      {
          imc_to_char( color_itom(
                       "~cDescription: ~CMinimmlevel sets the minimum level for someone to have\n"
                       "             ~Cimmortal privileges in IMC.\n"
                       "~cAccepted Values: ~CTakes any integer between 1 and the mud's maximum level.\n" 
                       ), ch);
          return;
      }

      value = atoi(argument);

      if ( value < 1 || value > IMCMAX_LEVEL )
      {
          char_printf( ch, "That value was outside of the range of 1 to %d.\n", IMCMAX_LEVEL );
          return;
      }

      imc_minimmlevel = value;
      char_printf( ch, "The minimum immortal privilege level has been set to %d.\n", value );
	  imc_save_config();
	  return;
   }

   if( !strcasecmp( arg1, "minadminlevel" ) )
   {
      int value = 0;

      if( !argument || argument[0] == '\0' || !is_number(argument))
      {
          imc_to_char( color_itom(
                       "~cDescription: ~CMinadminlevel sets the minimum level for someone to have\n"
                       "             ~Cadministrative privileges.\n"
                       "~cAccepted Values: ~CTakes any integer between 1 and the mud's maximum level.\n" 
                       ), ch);
          return;
      }

      value = atoi(argument);

      if ( value < 1 || value > IMCMAX_LEVEL )
      {
          char_printf( ch, "That value was outside of the range of 1 to %d.\n", IMCMAX_LEVEL );
          return;
      }

      imc_minadminlevel = value;
      char_printf( ch, "The minimum administrative privilege level has been set to %d.\n", value );
	  imc_save_config();
	  return;
   }

   if( !strcasecmp( arg1, "infoname" ) )
   {
      if( !argument || argument[0] == '\0' || strlen(argument) > 35 )
      {
          imc_to_char( color_itom(
                       "~cDescription: ~CInfoname is what information services such as imcplist\n"
                       "             ~Cand imcminfo will display as this mud's name.\n" 
                       "~cAccepted Values: ~CTakes any string up to 35 characters.\n"
                       ), ch);
          return;
      }

      if( imc_siteinfo.name )
          IMCSTRFREE( imc_siteinfo.name );

	  imc_siteinfo.name = IMCSTRALLOC( argument );
	  char_printf( ch, "This mud will now be displayed as %s.\n", argument );
	  imc_save_config();
      return;
   }

   if( !strcasecmp( arg1, "infohost" ) )
   {
      if( !argument || argument[0] == '\0' || strlen(argument) > SSS 
              || (strchr( argument, ' ' )) ) 
      {
          imc_to_char( color_itom(
                       "~cDescription: ~CInfohost is what information services such as imcplist\n"
                       "             ~Cand imcminfo will display as this mud's host.\n"
                       "~cAccepted Values: ~CTakes any string without spaces up to 256 characters.\n"
                       ), ch);
          return;
      }

      if( imc_siteinfo.host )
          IMCSTRFREE( imc_siteinfo.host );

	  imc_siteinfo.host = IMCSTRALLOC( argument );
	  char_printf( ch, "This mud will now be displayed as being at %s.\n", argument );
	  imc_save_config();
      return;
   }

   if( !strcasecmp( arg1, "infoport" ) )
   {
      int value = 0;

      if( !argument || argument[0] == '\0' || !is_number(argument))
      {
          imc_to_char( color_itom(
                       "~cDescription: ~CInfoport is what information services such as imcplist\n"
                       "             ~Cand imcminfo will display as this mud's port.\n"
                       "~cAccepted Values: ~CTakes any integer between 1 and 65,535.\n" 
                       ), ch);
          return;
      }

      value = atoi(argument);

      if ( value < 1 || value > 65535 )
      {
          imc_to_char( "That value was outside of the range of 1 to 65,535.\n", ch );
          return;
      }

      imc_siteinfo.port = value;
      char_printf( ch, "This mud will now be displayed as being on port %d.\n", value );
	  imc_save_config();
	  return;
   }

   if( !strcasecmp( arg1, "infoemail" ) )
   {
      if( !argument || argument[0] == '\0' || strlen(argument) > SSS + 24 )
      {
          imc_to_char( color_itom(
                       "~cDescription: ~CInfoemail is what information services such as imcplist\n"
                       "             ~Cand imcminfo will display as this mud's administrative email.\n" 
                       "~cAccepted Values: ~CTakes any string up to 280 characters.\n"
                       ), ch);
          return;
      }

      if( imc_siteinfo.email )
          IMCSTRFREE( imc_siteinfo.email );

	  imc_siteinfo.email = IMCSTRALLOC( argument );
	  char_printf( ch, "This mud's administrative email will now be displayed as %s.\n", argument );
	  imc_save_config();
      return;
   }

   if( !strcasecmp( arg1, "infowww" ) )
   {
      if( !argument || argument[0] == '\0' || strlen(argument) > SSS 
              || (strchr( argument, ' ' )) ) 
      {
          imc_to_char( color_itom(
                       "~cDescription: ~CInfowww is what information services such as imcplist\n"
                       "             ~Cand imcminfo will display as this mud's web address.\n"
                       "~cAccepted Values: ~CTakes any string without spaces up to 256 characters.\n"
                       ), ch);
          return;
      }

      if( imc_siteinfo.www )
          IMCSTRFREE( imc_siteinfo.www );

	  imc_siteinfo.www = IMCSTRALLOC( argument );
	  char_printf( ch, "This mud's webpage will now be displayed as being at %s.\n", argument );
	  imc_save_config();
      return;
   }

   if( !strcasecmp( arg1, "infobase" ) )
   {
      char cbase[MSS];

      if( !argument || argument[0] == '\0' || strlen(argument) > 20 )
      {
          imc_to_char( color_itom(
                       "~cDescription: ~CInfoname is what information services such as imcplist\n"
                       "             ~Cand imcminfo will display as this mud's codebase.\n" 
                       "~cAccepted Values: ~CTakes any string up to 20 characters.\n"
                       ), ch);
          return;
      }

      if( imc_siteinfo.base )
          IMCSTRFREE( imc_siteinfo.base );

	  imc_siteinfo.base = IMCSTRALLOC( argument );
	  char_printf( ch, "This mud's codebase will now be displayed as %s.\n", argument );
	  imc_save_config();

      if( imc_versionid )
          IMCSTRFREE( imc_versionid);

      snprintf( cbase, MSS, "%s %s", IMC_VERSION_ID, imc_siteinfo.base );
      
      imc_versionid = IMCSTRALLOC( cbase );
      return;
   }

   if( !strcasecmp( arg1, "infodetails" ) )
   {
      if( !argument || argument[0] == '\0' || strlen(argument) > SSS )
      {
          imc_to_char( color_itom(
                       "~cDescription: ~CInfodetails is what information services such as imcplist\n"
                       "             ~Cand imcminfo will display as the mud's short description.\n" 
                       "~cAccepted Values: ~CTakes any string up to 256 characters.\n"
                       ), ch);
          return;
      }

      if( imc_siteinfo.details )
          IMCSTRFREE( imc_siteinfo.details );

	  imc_siteinfo.details = IMCSTRALLOC( argument );
	  char_printf( ch, "This mud's short description will now be \"%s\".\n", argument );
	  imc_save_config();
      return;
   }

   if( !strcasecmp( arg1, "whostyle" ) )
   {
      int value = 0;

      if( !argument || argument[0] == '\0' || strlen(argument) > 25 )
      {
          imc_to_char( color_itom(
                       "~cDescription: ~CWhostyle sets the color pattern used for your imcplist.\n"
                       "~cAccepted Values: ~CTakes any integer between 0 and the mud's maximum whostyle\n" 
                       "                 ~Cor any string up to 25 characters for the whostyle's name.\n"
                       "~cCurrent WhoStyles:\n"
                       ), ch);

          for ( value = 0; value < MAX_WHOSTYLE; value++)
              char_printf( ch, color_itom("~C                 %d - %s\n"), value, whostylename_table[value]);

          return;
      }

      if ( !is_number(argument) )
      {
          for( value = 0; value < MAX_WHOSTYLE; value++)
              if( !imc_str_prefix( whostylename_table[value], argument))
                  break;
      }
      else
          value = atoi(argument);

      if ( value < 0 || value >= MAX_WHOSTYLE )
      {
          char_printf( ch, "That value was outside of the range of 0 to %d.\n", MAX_WHOSTYLE-1 );
          return;
      }

      imc_whostyle = value;
      char_printf( ch, "The color pattern style for your imcplist has been changed to %s.\n", 
              whostylename_table[value]);
	  imc_save_config();
	  return;
   }

   run_imcmsettings( ch, "" );
   return;
}

void run_imcmblacklist( CHAR_DATA *ch, const char *argument )
{
   int count;
   IMC_BLACKLIST *entry;
   char arg[MSS];

   CHECKIMC(ch);

   if( !argument || argument[0] == '\0')
   {
       imc_to_char( color_itom(
                  "~GSyntax: ~cimcmblacklist list         - ~Clists blacklisted muds\n"
                    "~c        imcmblacklist add <mud>    - ~Cadds a new mud to the blacklist\n"
                    "~c        imcmblacklist remove <mud> - ~Cremoves a mud from the blacklist\n" ), ch);
       return;
   }

   argument = imc_getarg( argument, arg, MSS );

   if( !strcasecmp( arg, "list" ) )
   {
      imc_to_char( color_itom("~cCurrent Blacklisted Muds:\n"), ch );
      for( count = 0, entry = first_imc_mudblacklist; entry; entry = entry->next, count++ )
         char_printf( ch, color_itom("~C %s\n"), entry->name );

      if( !count )
         imc_to_char( color_itom("~C None\n"), ch );
      else
         char_printf( ch, color_itom("\n~c[total of %d blacklisted muds]\n"), count );

      return;
   }

   if( !argument || argument[0] == '\0')
   {
       run_imcmblacklist(ch, "");
       return;
   }

   smash_tilde(argument);

   if( !strcasecmp( arg, "add" ) )
   {

      for( entry = first_imc_mudblacklist; entry; entry = entry->next )
         if( !strcasecmp( argument, entry->name ) )
             break;

      if (entry)
      {
          imc_to_char( "That mud is already blacklisted.\n", ch);
          return;
      }
       
      IMCCREATE( entry, IMC_BLACKLIST, 1 );
      entry->name = IMCSTRALLOC( argument );

      IMCLINK( entry, first_imc_mudblacklist, last_imc_mudblacklist, next, prev );

      imc_saveblacklist( );

      imc_to_char( "That mud will now be blacklisted.\n", ch);
	  return;
   }

   if( !strcasecmp( arg, "remove" ) )
   {
      for( entry = first_imc_mudblacklist; entry; entry = entry->next )
         if( !strcasecmp( argument, entry->name ) )
             break;

      if (!entry)
      {
          imc_to_char( "Mud not found on blacklist.\n", ch );
          return;
      }
      
      if( entry->name )
          IMCSTRFREE( entry->name );

      IMCUNLINK( entry, first_imc_mudblacklist, last_imc_mudblacklist, next, prev );

      IMCDISPOSE( entry );

      imc_saveblacklist();

      imc_to_char ( "Mud removed from blacklist.\n", ch);
      return;
   }

   run_imcmblacklist( ch, "" );
   return;
}

void run_imcmstat( CHAR_DATA *ch, const char *argument )
{
   CHECKIMC(ch);

   if( !argument || argument[0] == '\0') 
   {
      imc_to_char( color_itom(
                  "~GSyntax: ~cimcmstat general - ~Cdisplays general information on the mud\n"
                    "~c        imcmstat network - ~Cdisplays network statistical information for the mud\n"
                    ), ch);
       return;
   }
   if( !imc_str_prefix( argument, "general" ))
   {
      imc_to_char( color_itom( imc_getstats("general") ), ch );
      return;
   }
   if( !imc_str_prefix( argument, "network" ) )
   {
      imc_to_char( color_itom( imc_getstats( "network") ), ch );
      return;
   }

   run_imcmstat( ch, "" );
   return;
}

void run_imcpsettings( CHAR_DATA *ch, const char *argument )
{
   char arg[MSS];
   bool toggle;
   unsigned char mode;
   int i = 0;

   CHECKIMC(ch);

   if ( !argument || argument[0] == '\0')
   {
       imc_to_char( color_itom(
                    "~GSyntax: ~cimcpsettings show   - ~Clists IMC network functions and IMC channels\n"
                    "~c        imcpsettings subscribe +channel - ~Ccreates a subscription to a channel\n"
                    "~c        imcpsettings subscribe -channel - ~Cdeletes a subscription to a channel\n"
                    "~c        imcpsettings blacklist +mud - ~Cadds a mud to the blacklist\n"
                    "~c        imcpsettings blacklist -mud - ~Cremoves a mud from the blacklist\n"
                    "~c        imcpsettings blacklist +person@mud - ~Cadds a player to the blacklist\n"
                    "~c        imcpsettings blacklist -person@mud - ~Cremoves a player from the blacklist\n"
                    "~c        imcpsettings function +nfunction  - ~Cswitches on a network function\n"
                    "~c        imcpsettings function -nfunction  - ~Cswitches off a network function\n"
                    "\n~cValid network functions are:\n"), ch);
      
       for ( i = 0; i < numfunctions; i++)
           char_printf( ch, color_itom("~C \'%s\'"), imc_functions[i].name);
       
       imc_to_char( color_itom(
            "\n~GMultiple settings may be given in one command.\n"
            "There is also the 'all' option to set or unset all available functions.\n"), ch);

       return;
   }

   smash_tilde(argument);
   argument = imc_getarg( argument, arg, MSS);

   if( !strcasecmp(arg, "show"))
   {
      IMC_BLACKLIST *entry = NULL;
      char afunc[MSS], nfunc[MSS], dfunc[MSS];
      char sbuf[LSS];
      unsigned char j = 0, k = 0, l = 0;

      imc_to_char( color_itom("~WPlayer Settings:\n"), ch);

      strlcpy( afunc, "~cActive Network Functions:~C", MSS);
      strlcpy( nfunc, "~cNonactive Network Functions:~C", MSS);
      strlcpy( dfunc, "~cDenied Network Functions:~C", MSS);

      for( i = 0; i < numfunctions; i++ )
      {
         if( !function_usable(ch, imc_functions[i].deny_flag, imc_functions[i].allow_flag, 
                     imc_functions[i].minlevel) ) 
         {
             snprintf( dfunc+strlen(dfunc), MSS - strlen(dfunc), " %s", imc_functions[i].name );
             j++;
         }
         else if ( IS_SET( IMC_PFLAGS(ch), imc_functions[i].pset_flag ) )
         {
             snprintf( nfunc + strlen(nfunc), MSS - strlen(nfunc), " %s", imc_functions[i].name );
             k++;
         }
         else
         {
             snprintf( afunc + strlen(afunc), MSS - strlen(afunc), " %s", imc_functions[i].name );
             l++;
         }

         if ( j >= 4 )
         {
             strlcpy( dfunc+strlen(dfunc), "\n                           ", MSS - strlen(dfunc));
             j = 0;
         }

         if ( k >= 4 )
         {
             strlcpy( nfunc + strlen(nfunc), "\n                            ", MSS - strlen(nfunc));
             k = 0;
         }

         if ( l >= 4 )
         {
             strlcpy( afunc + strlen(afunc), "\n                         ", MSS - strlen(afunc));
             l = 0;
         }
      }

      snprintf( sbuf, LSS, "%s\n%s\n%s\n", afunc, nfunc, dfunc );
      imc_to_char( color_itom(sbuf), ch );

      char_printf( ch, color_itom("~cChannel Subscriptions: ~C%s\n"),
              (IMC_CSUBSCRIBED(ch) && IMC_CSUBSCRIBED(ch)[0] != '\0' ? IMC_CSUBSCRIBED(ch) : ""));

      imc_to_char( color_itom("~cBlacklisted Muds:~C"), ch );

      if ( FIRST_IMCBLACKLIST(ch) != NULL && LAST_IMCBLACKLIST(ch) != NULL )
          for( entry = FIRST_IMCBLACKLIST(ch); entry; entry = entry->next )
             char_printf( ch, color_itom("~C %s"), entry->name );

      imc_to_char( "\n", ch );

      return;
   }

   if ( !strcasecmp( arg, "subscribe" ) )
       mode = 1;
   else if ( !strcasecmp( arg, "blacklist" ) )
       mode = 2;
   else if ( !strcasecmp( arg, "function" ) )
       mode = 3;
   else
   {
       run_imcpsettings( ch, "" );
       return;
   }

    /* turn some things on or off */

   while( ( argument = imc_getarg( argument, arg, MSS ) ), arg[0] )
   {
      if( arg[0] == '-' )
         toggle = FALSE;
      else if( arg[0] == '+' )
         toggle = TRUE;
      else
      {
         run_imcpsettings(ch, "");
         return;
      }

      if ( mode == 1 ) // Channel Subscriptions
      {
          if( imc_hasname( IMC_CSUBSCRIBED(ch), arg + 1 ) )
          {
              if ( toggle )
              {
                  imc_to_char ( "Channel already subscribed to.\n", ch );
                  continue;
              }

              imc_removename( &IMC_CSUBSCRIBED(ch), arg + 1 );
              imc_to_char( "No longer subscribed to channel.\n", ch );
          }
          else
          {
              if( !imc_findlchannel( arg + 1 ) )
              {
                  imc_to_char( "No such channel localized.\n", ch );
              }
              else
              {
                  if ( !toggle )
                  {
                      imc_to_char ( "Not subscribed to channel.\n", ch );
                      continue;
                  }

                  imc_addname( &IMC_CSUBSCRIBED(ch), arg + 1 );
                  imc_to_char( "Channel turned on.\n", ch );
              }
          }

          continue;
      }
      else if ( mode == 2 ) // Blacklist Entries
      {
           IMC_BLACKLIST *entry = NULL;

           for( entry = FIRST_IMCBLACKLIST(ch); entry; entry = entry->next )
               if( !strcasecmp( arg+1, entry->name ) )
                   break;

           if (entry)
           {
               if ( toggle )
               {
                   if ((strchr( arg+1, '@' )) ) 
                       imc_to_char( "That person is already on your blacklist.\n", ch);
                   else
                       imc_to_char( "That mud is already on your blacklist.\n", ch);
                   return;
               }
               
               if( entry->name )
                   IMCSTRFREE( entry->name );

               IMCUNLINK( entry, FIRST_IMCBLACKLIST(ch), LAST_IMCBLACKLIST(ch), next, prev );

               IMCDISPOSE( entry );

               if ((strchr( arg+1, '@' )) ) 
                   imc_to_char( "That person has been removed from your blacklist.\n", ch);
               else
                   imc_to_char( "That mud has been removed from your blacklist.\n", ch);
           }
           else
           {
               if ( !toggle )
               {
                   if ((strchr( arg+1, '@' )) ) 
                       imc_to_char( "That person is not on your blacklist.\n", ch);
                   else
                       imc_to_char( "That mud is not on your blacklist.\n", ch);
                   return;
               }

               IMCCREATE( entry, IMC_BLACKLIST, 1 );
               entry->name = IMCSTRALLOC( arg+1 );

               IMCLINK( entry, FIRST_IMCBLACKLIST(ch), LAST_IMCBLACKLIST(ch), next, prev );

               if ((strchr( arg+1, '@' )) ) 
                   imc_to_char( "That person has been added to your blacklist.\n", ch);
               else
                   imc_to_char( "That mud has been added to your blacklist.\n", ch);
           }

           continue;
      }
      else // Network Functions
      {
          if( !strcasecmp( arg+1, "all" ) )
          {
              if( toggle )
              {
                  imc_to_char( "ALL available IMC network functions are now on.\n", ch );
            
                  for( i = 0; i < numfunctions; i++ )
                  {
                      if( function_usable(ch, imc_functions[i].deny_flag, imc_functions[i].allow_flag,
                                  imc_functions[i].minlevel) 
                              && imc_functions[i].pset_flag != PSET_NOTSUPPORTED )
                          REMOVE_BIT( IMC_PFLAGS(ch), imc_functions[i].pset_flag);
                  }
              }
              else
              {
                  imc_to_char( "ALL available IMC network functions are now off.\n", ch );
            
                  for( i = 0; i < numfunctions; i++ )
                  {
                      if( function_usable(ch, imc_functions[i].deny_flag, imc_functions[i].allow_flag,
                                  imc_functions[i].minlevel) 
                              && imc_functions[i].pset_flag != PSET_NOTSUPPORTED)
                          SET_BIT( IMC_PFLAGS(ch), imc_functions[i].pset_flag);
                  }
              }
         
              return;
          }
          else
          {
         
              for( i = 0; i < numfunctions; i++ )
              {
                  if( function_usable(ch, imc_functions[i].deny_flag, imc_functions[i].allow_flag,
                              imc_functions[i].minlevel) && !strcasecmp( imc_functions[i].name, arg+1 ) )
                      break;
              }

         
              if( i == numfunctions )
              {
                  char_printf( ch, "You don't have access to an IMC network function called \"%s\".\n",
                         arg + 1 );
                  continue;
              }
          }

          if( imc_functions[i].pset_flag == PSET_NOTSUPPORTED)
          {
              char_printf( ch, "%s can not have its status changed.\n", imc_functions[i].name ); 
              continue;
          }

      
          if( toggle && !IS_SET(IMC_PFLAGS(ch), imc_functions[i].pset_flag) )
          {
              char_printf( ch, "%s is already on.\n", imc_functions[i].name );
              continue;
          }

          if( !toggle && IS_SET(IMC_PFLAGS(ch), imc_functions[i].pset_flag) )
          {
              char_printf( ch, "%s is already off.\n", imc_functions[i].name );
              continue;
          }

          if( toggle )
          {
              char_printf( ch, "%s is now ON.\n", imc_functions[i].name );
              REMOVE_BIT( IMC_PFLAGS(ch), imc_functions[i].pset_flag );
          }
          else
          {
              char_printf( ch, "%s is now OFF.\n", imc_functions[i].name );
              SET_BIT( IMC_PFLAGS(ch), imc_functions[i].pset_flag );
          }
      }

      continue; // end of while
   }

   return;
}

void run_imcpflags( CHAR_DATA *ch, const char *argument )
{
   char arg[MSS];
   CHAR_DATA *victim = NULL;
   DESCRIPTOR_DATA *d;
   int fn, i;

   CHECKIMC(ch);

   argument = imc_getarg( argument, arg, MSS );

   if( !arg || arg[0] == '\0' )
   {
      imc_to_char( color_itom(
               "~GSyntax: ~cimcpflags <char>                     - ~Ccheck flag settings\n"
		       "        ~cimcpflags <char> +<network function> - ~Cset allow flag\n"
		       "        ~cimcpflags <char> -<network function> - ~Cset deny flag\n"
		       "        ~cimcpflags <char> =<network function> - ~Creset allow/deny flag\n"), ch );
      return;
   }

   for( d = first_descriptor; d; d = d->next )
      if( d->connected == CON_PLAYING && ( victim = d->original ? d->original : d->character ) != NULL
              && !strcasecmp( arg, CH_IMCNAME(victim) ))
          break;

   if ( !d )
   {
      imc_to_char( "They are not here.\n", ch );
      return;
   }

   if( get_permlevel(victim) >= get_permlevel(ch) )
   {
      imc_to_char( "You can't set their privileges.\n", ch );
      return;
   }

   if( !argument || argument[0] == '\0' )
   {
      char_printf( ch, "&WAllow/Deny Flags Settings for %s\n",
              CH_IMCNAME(victim) );

      for( i = 0; i < numfunctions; i++ )
      {
         char funcstring[SSS];

         snprintf( funcstring, SSS, "~c%16s ~W- %s\n", imc_functions[i].name, 
                  IS_SET(IMC_PFLAGS(victim), imc_functions[i].allow_flag) ? "~CAllow Flag~c" :
                  IS_SET(IMC_PFLAGS(victim), imc_functions[i].allow_flag) ? "~CDeny Flag~c" : 
                  "~CNo Flags");

         imc_to_char( color_itom(funcstring), ch );
      }

      return;
   }
  
   if( argument[0] == '-' )
      fn = 0;
   else if( argument[0] == '+' )
      fn = 1;
   else if( argument[0] == '=' )
      fn = 2;
   else
   {
      run_imcpflags(ch, ""); 
      return;
   }

   argument++;

   for( i = 0; i < numfunctions; i++ )
      if( !strcasecmp( imc_functions[i].name, argument ) )
          break;

   if( i == numfunctions )
   {
      imc_to_char( "No such network function.\n", ch );
      return;
   }


   switch( fn )
   {
      case 0: /* set deny flag */

      if( get_permlevel(victim) < imc_functions[i].minlevel )
      {
         imc_to_char( "That player is already below this network function's permission level.\n", ch );
         return;
      }    

      if( imc_functions[i].deny_flag == DENY_NOTSUPPORTED )
      {
         imc_to_char( "That network function can not be denied.\n", ch );
         return;
      }

      if( IS_SET(IMC_PFLAGS(victim), imc_functions[i].deny_flag) )
      {
         imc_to_char( "Deny flag already set.\n", ch );
         return;
      }

      SET_BIT( IMC_PFLAGS(victim), imc_functions[i].deny_flag );
      REMOVE_BIT( IMC_PFLAGS(victim), imc_functions[i].allow_flag );
      char_printf( victim, "The gods have revoked your %s privileges.\n", imc_functions[i].name );
      imc_to_char( "Deny flag set.\n", ch );
      return;

      case 1: /* set allow flag */
      if( get_permlevel(victim) >= imc_functions[i].minlevel )
      {
          imc_to_char( "That player is already at or above this network function's permission level.\n",ch);
          return;
      }

      if( IS_SET(IMC_PFLAGS(victim), imc_functions[i].allow_flag) )
      {
         imc_to_char( "Allow flag already set.\n", ch );
         return;
      }

      if( imc_functions[i].allow_flag == ALLOW_NOTSUPPORTED )
      {
         imc_to_char( "That network function can not be specially allowed.\n", ch ); 
         return;
      }

      SET_BIT( IMC_PFLAGS(victim), imc_functions[i].allow_flag );
      REMOVE_BIT( IMC_PFLAGS(victim), imc_functions[i].deny_flag );
      char_printf( victim, "The gods have given you %s privileges.\n", imc_functions[i].name );
      imc_to_char( "Allow flag set.\n", ch );
      return;

      case 2: /* clears flags */
      if( IS_SET(IMC_PFLAGS(victim), imc_functions[i].allow_flag) )
      {
         REMOVE_BIT( IMC_PFLAGS(victim), imc_functions[i].allow_flag );
         char_printf( victim, "The gods have removed your %s privileges.\n", imc_functions[i].name );
         imc_to_char( "Allow flag cleared.\n", ch );
      }
      else if ( IS_SET(IMC_PFLAGS(victim), imc_functions[i].deny_flag) )
      {
         REMOVE_BIT( IMC_PFLAGS(victim), imc_functions[i].deny_flag );
         char_printf( victim, "The gods have restored your %s privileges.\n", imc_functions[i].name );
         imc_to_char( "Deny flag cleared.\n", ch );
      }
      else
          imc_to_char( "No flags to clear.\n", ch );
    
      return;
   }
}

/*  Traceroute
 *
 *  Be lazy - only remember the last traceroute
 */
void run_imcmfind( CHAR_DATA *ch, const char *argument )
{
   CHECKIMC(ch);
  
   if( !argument || argument[0] == '\0' )
   {
      imc_to_char( "Which mud do you want to traceroute?\n", ch );
      return;
   }

   CHECKMUD( ch, argument );

   strlcpy( pinger, CH_IMCNAME(ch), 100 );
   imc_send_traceroute( argument, CH_IMCNAME(ch) );
}

command_struct command_table[] =
{
/* Player Accessible Commands */

// Channel Directed Commands

    // Lists all available channels
    { "imcclist", run_imcclist, PERM_PLAYER}, // Continuum Version
    { "imcchanlist", run_imcclist, PERM_PLAYER}, // Mud-Net Version
    { "ilist", run_imcclist, PERM_PLAYER}, // Original IMC2 Version
    

// Mud Directed Commands

    // Retrieves all available information on a mud
    { "imcminfo", run_imcminfo, PERM_PLAYER}, // Continuum Version
    { "imcquery", run_imcminfo, PERM_PLAYER}, // Mud-Net Version
    { "rquery", run_imcminfo, PERM_PLAYER}, // Original IMC2 Version
    // Lists all currently and recently connected muds
    { "imcmlist", run_imcmlist, PERM_PLAYER}, // Continuum Version
    { "imclist", run_imcmlist, PERM_PLAYER}, // Original and Mud-Net Version

// Player Directed Commands

    // Sets a player as being 'away from keyboard'
    { "imcpafk", run_imcpafk, PERM_PLAYER}, // Continuum Original
    // Sends a beep sound to a visible player
    { "imcpbeep", run_imcpbeep, PERM_PLAYER}, // Continuum Version
    { "imcbeep", run_imcpbeep, PERM_PLAYER}, // Mud-Net Version
    { "rbeep", run_imcpbeep, PERM_PLAYER},  // Original IMC2 Version
    // Searches for all visible players with a given name
    { "imcpfind", run_imcpfind, PERM_PLAYER}, // Continuum Version
    { "imclocate", run_imcpfind, PERM_PLAYER}, // Mud-Net Version
    { "rwhois", run_imcpfind, PERM_PLAYER},  // Original IMC2 Version
    // Retrieves all available information on a player
    { "imcpinfo", run_imcpinfo, PERM_PLAYER}, // Continuum Version
    { "imcfinger", run_imcpinfo, PERM_PLAYER}, // Mud-Net Version
    { "rfinger", run_imcpinfo, PERM_PLAYER},  // Original IMC2 Version
    // Lists all visible players on a mud
    { "imcplist", run_imcplist, PERM_PLAYER}, // Continuum Version
    { "imcwho", run_imcplist, PERM_PLAYER}, // Mud-Net Version
    { "rwho", run_imcplist, PERM_PLAYER},  // Original IMC2 Version
    // Continues a private communication with a player
    { "imcpreply", run_imcpreply,PERM_PLAYER}, // Continuum Version
    { "imcreply", run_imcpreply, PERM_PLAYER}, // Mud-Net Version
    { "rreply", run_imcpreply, PERM_PLAYER},  // Original IMC2 Version
    // Modifies the general IMC settings of a player, 
    // along with the player's channel settings
    { "imcpsettings", run_imcpsettings, PERM_PLAYER},  // Continuum Version
    { "imcflags", run_imcpsettings, PERM_PLAYER}, // Mud-Net Version covering part of functionality
    { "imclisten", run_imcpsettings, PERM_PLAYER},// Mud-Net Version covering part of functionality
    { "rchannels", run_imcpsettings, PERM_PLAYER},// Original IMC2 Version covering part of functionality
    { "ichan", run_imcpsettings, PERM_PLAYER}, // Original IMC2 Version covering part of functionality
    // Sends a private communication to a player
    { "imcptell", run_imcptell, PERM_PLAYER}, // Continuum Version
    { "imctell", run_imcptell, PERM_PLAYER},  // Mud-Net Version
    { "rtell", run_imcptell, PERM_PLAYER},   // Original IMC2 Version


/* Immortal Accessible Commands */

    // Mud Directed Commands

    // Returns the path to a mud by using traceroute
    { "imcmfind", run_imcmfind, PERM_IMMORTAL}, // Continuum Version
    { "imcping", run_imcmfind, PERM_IMMORTAL},  // Mud-Net Version
    { "rping", run_imcmfind, PERM_IMMORTAL},  // Original IMC2 Version
    // Retrieves general and network information about the mud
    { "imcmstat", run_imcmstat, PERM_IMMORTAL}, // Continuum Version
    { "imcstats", run_imcmstat, PERM_IMMORTAL}, // Mud-Net Version
    { "istats", run_imcmstat, PERM_IMMORTAL}, // Original IMC2 Version


/* Administration Accessible Commands */

// Channel Directed Commands

    // Remotely administers a channel's global settings
    { "imccadmin", run_imccadmin, PERM_ADMIN}, // Continuum Version
    { "imccommand", run_imccadmin, PERM_ADMIN}, // Mud-Net Version
    { "icommand", run_imccadmin, PERM_ADMIN}, // Original IMC2 Version
    // Modifies a channel's mud specific settings
    { "imccsettings", run_imccsettings, PERM_ADMIN}, // Continuum Version
    { "imcsetup", run_imccsettings, PERM_ADMIN}, // Mud-Net Version
    { "isetup", run_imccsettings, PERM_ADMIN}, // Original IMC2 Version

// Mud Directed Commands 

    // Modifies a list of hostile muds to be blacklisted
    { "imcmblacklist", run_imcmblacklist, PERM_ADMIN}, // Continuum Version
    { "imcignore", run_imcmblacklist, PERM_ADMIN}, // Mud-Net Version
    { "rignore", run_imcmblacklist, PERM_ADMIN}, // Original IMC2 Version
    // Modifies the general IMC settings for the mud 
    { "imcmsettings", run_imcmsettings, PERM_ADMIN}, // Continuum Version
    { "imc", run_imcmsettings, PERM_ADMIN}, // Original and Mud-Net Version

// Player Directed Commands 

    // Modifies the privileges of a player
    { "imcpflags", run_imcpflags, PERM_ADMIN}, // Continuum Version
    { "imcdeny", run_imcpflags, PERM_ADMIN}, // Mud-Net Version
    { "rchanset", run_imcpflags, PERM_ADMIN}, // Original IMC2 Version
    {NULL, NULL, 0}
};

/* check for IMC channels, return TRUE to stop command processing, FALSE otherwise */
bool imc_command_hook ( CHAR_DATA *ch, const char *command, const char *argument )
{
   unsigned char i=0;
    
   if( IS_NPC(ch) )
      return FALSE;

   if( !this_imcmud )
   {
       imcbug( "%s", "Ooops. IMC being called without configuration loaded." );
       return FALSE;
   }

   for ( i=0; command_table[i].name != NULL; i++)
   {
       if( !level_pass( ch, command_table[i].permlevel ))
           continue;
      
      if( !strcasecmp( command, command_table[i].name ) )
      {
         (*command_table[i].ComFunction)(ch, argument); 
         return TRUE;
      }
   }

   return imc_channel_handler(ch, command, argument);
}

const char *return_imcsocial( const char *socialname, unsigned char type )
{
#ifdef SOCIALSTRUCT
    social_t *social;
#else
    int cmd = 0;
#endif

#ifdef SOCIALSTRUCT
    if( ( social = social_lookup(socialname, str_prefix)) == NULL )
        return NULL;

    if ( type == 1 )
        return social->found_notvict;
    else if ( type == 2 )
        return social->self_room;
    else
        return social->noarg_room;
#else
	for ( cmd = 0; social_table[cmd].name[0] != '\0'; cmd++ )
	   if( !imc_str_prefix( socialname, social_table[cmd].name ) )
	      break;

    if ( social_table[cmd].name[0] == '\0' )
        return NULL;

    if ( type == 1 )
        return social_table[cmd].others_found;
    else if ( type == 2 )
        return social_table[cmd].others_auto;
    else
        return social_table[cmd].others_no_arg;
#endif

}

const char *imc_act_string( const char *format, CHAR_DATA *ch, const char *vname )
{
   static char * const he_she  [] = { "it",  "he",  "she" }; 
   static char * const him_her [] = { "it",  "him", "her" }; 
   static char * const his_her [] = { "its", "his", "her" }; 
   static char buf[LSS];  
   char tmp_str[LSS];  
   const char *i = ""; 
   char *point;
 
   if ( !format || format[0] == '\0' || !ch )
	return NULL; 

   point = buf;
	
   while( *format != '\0' ) 
   { 
	
       if( *format != '$' ) 
       { 
           *point++ = *format++; 
           continue; 
	   } 
	    
      ++format; 
	    
      if( ( !vname || vname[0] == '\0' ) 
	 && ( *format == 'N' || *format == 'E' || *format == 'M' || *format == 'S' || *format == 'K' ) )
	   i = " !!!!! ";
      else 
      { 
         switch( *format ) 
	   {
	      default:  i = " !!!!! ";                                       break;
	      case 'n': i = imc_makename( CH_IMCNAME(ch), imc_name );        break;
 	      case 'N': i = vname;                                           break;
	      case 'e': i = he_she[URANGE(0, CH_IMCSEX(ch), 2)];             break;
            case 'E': i = "it";                                            break;
            case 'm': i = him_her[URANGE(0, CH_IMCSEX(ch), 2)];            break;
	      case 'M': i = "it";                                            break;
	      case 's': i = his_her[URANGE(0, CH_IMCSEX(ch), 2)];            break;
	      case 'S': i = "its";                                           break;
            case 'k': imc_getarg( CH_IMCNAME(ch), tmp_str, LSS ); 
               i = (char *) tmp_str;                                       break;
            case 'K': imc_getarg( vname, tmp_str, LSS );
               i = (char *) tmp_str;                                       break;
	   }
	}
      ++format;
      while( ( *point = *i ) != '\0' )
	   ++point, ++i; 
   }
   *point = 0;
   point++;
   *point = '\0';

   buf[0] = toupper( buf[0] );
   return buf;
}


bool imc_channel_handler( CHAR_DATA *ch, const char *command, const char *argument)
{
   IMC_CHANNEL *c;
   DESCRIPTOR_DATA *d;
   char mbuf[LSS], vname[MSS];
   int emote = 0;
    
   c = imc_findlchannel( command );

   if( !c )
      return FALSE;

   if( c->perm_level > get_permlevel(ch) )
      return FALSE;

   if( !c->refreshed )
	  return FALSE;

   if( !imc_audible( c, imc_makename( CH_IMCNAME(ch), imc_name ) ) )
   {
      imc_to_char( "You cannot use that channel.\n", ch );
      return TRUE;
   }

   if( !argument || argument[0] == '\0' )
   {
	int x;

    char_printf( ch, color_itom("~cThe last %d %s messages:\n"), MAX_IMCHISTORY, c->local_name );

	for( x = 0; x < MAX_IMCHISTORY; x++ )
	{
	   if( c->history[x] != NULL )
		imc_to_char( c->history[x], ch );
	   else
		break;
	}
	return TRUE;
   }

   if( !imc_hasname( IMC_CSUBSCRIBED(ch), c->local_name ) )
      return FALSE;

   if( *argument == ',' )
   {
      emote = 1;
      argument++;
   }

   /* Social code redone by Rogel, based off of work performed by Senir */
   else if( *argument == '@' )
   {
      char socialname[MSS], newstring[LSS];
      unsigned char type = 0;

      argument++;
      argument = imc_getarg( argument, socialname, MSS );

      if( !socialname || socialname[0] == '\0' )
      {
          imc_to_char( "Putting a social there might help.\n", ch );
          return TRUE;
      }

      imc_getarg( argument, vname, MSS );

      if( vname && vname[0] != '\0' )
      {
          char *separator;

          vname[0] = toupper(vname[0]);

          if( ( separator = strchr( vname, '@' ) ) != NULL )
          {
              ++separator;
              *separator = toupper(*separator);
          } 

          if (strcasecmp( vname, "Self" ) && (strcasecmp( CH_IMCNAME(ch), imc_nameof(vname) ) 
                  || (separator && strcasecmp( imc_mudof(vname), imc_name)))) 
          {
              if ( !separator )
              {
                  imc_to_char( "The social target must be in the form of person@mud.\n", ch );
                  return TRUE;
              }

              type = 1; /* Victim is not the social sender */

          }
          else
              type = 2; /* Victim is the social sender */

      }
      else
          type = 3; /* Victimless social */

      if ( return_imcsocial( socialname, type ) == NULL 
              || return_imcsocial( socialname, type)[0] == '\0' )
      {
          socialname[0] = toupper(socialname[0]);
          char_printf( ch, "%s is either not a social or that part of it is blank.\n", socialname );
          return TRUE;
      }

      strlcpy( newstring, imc_act_string( return_imcsocial(socialname, type), ch, vname ), LSS );

      argument = newstring;
      emote = 2;
   }

   imc_sendmessage( c, CH_IMCNAME(ch), color_mtoi(argument), emote );
   /* Local echo - bear in mind this doesn't mean your message actually got out!
    * This is a punt, nothing more.
    */
   if( emote < 2 )
   {
       snprintf( mbuf, LSS, emote ? c->emoteformat : c->regformat, imc_makename( CH_IMCNAME(ch), imc_name ), 
           argument ); 
   }
   else
      snprintf( mbuf, LSS, c->socformat, argument );
   for( d = first_descriptor; d; d = d->next )
   {
	if( d->character && imc_hasname( IMC_CSUBSCRIBED(d->character), c->local_name )
	   && get_permlevel(d->character) >= c->perm_level )
	   char_printf( d->character, "%s\n", mbuf );
   }
   update_imchistory( c, mbuf );
   return TRUE;
}
