/*
 * $Id: buffer.c,v 1.8 1998-11-02 05:28:29 fjoe Exp $
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

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#include "typedef.h"
#include "const.h"
#include "buffer.h"
#include "log.h"
#include "db/msg.h"

struct buf_data
{
	BUFFER *	next;
	int		lang;	/* buffer language, -1 == none */
	int		state;	/* error state of the buffer */
	int		size;	/* buffer size in bytes */
	char *		string; /* buffer's string */
};

#define BUF_LIST_MAX		10
#define BUF_DEFAULT_SIZE 	1024

extern int nAllocBuf;
extern int sAllocBuf;

/* valid states */
enum {
	BUFFER_SAFE,
	BUFFER_OVERFLOW,
	BUFFER_FREED
};

BUFFER *free_list;

static bool buf_cat(BUFFER *buffer, const char *string);
static int get_size (int val);

BUFFER *buf_new(int lang)
{
	BUFFER *buffer;
 
	if (free_list == NULL) {
		buffer		= malloc(sizeof(*buffer));
		nAllocBuf++;
	}
	else {
		buffer		= free_list;
		free_list	= free_list->next;
	}
 
	buffer->next		= NULL;
	buffer->lang		= lang;
	buffer->state		= BUFFER_SAFE;
	buffer->size		= BUF_DEFAULT_SIZE;
	buffer->string		= malloc(buffer->size);
	buffer->string[0]	= '\0';
 
	sAllocBuf += buffer->size;
	return buffer;
}

void buf_free(BUFFER *buffer)
{
	sAllocBuf -= buffer->size;

	free(buffer->string);
	buffer->string	= NULL;
	buffer->size	= 0;
	buffer->state	= BUFFER_FREED;

	buffer->next	= free_list;
	free_list	= buffer;
}

bool buf_add(BUFFER *buffer, const char *string)
{
	return buf_cat(buffer,
		       buffer->lang < 0 ? string : GETMSG(string, buffer->lang));
}

bool buf_printf(BUFFER *buffer, const char *format, ...)
{
	char buf[MAX_STRING_LENGTH];
	va_list ap;

	va_start(ap, format);
	vsnprintf(buf, sizeof(buf),
		  buffer->lang < 0 ? format : GETMSG(format, buffer->lang),
		  ap);
	va_end(ap);

	return buf_cat(buffer, buf);
}

void buf_clear(BUFFER *buffer)
{
	buffer->string[0]	= '\0';
	buffer->state		= BUFFER_SAFE;
}

char* buf_string(BUFFER *buffer)
{
	return buffer->string;
}

/*----------------------------------------------------------------------------
 * local functions
 */

/* buffer sizes */
const int buf_size[BUF_LIST_MAX] =
{
	16, 32, 64, 128, 256, 1024, 2048, 4096, 8192, 16384
};

/* local procedure for finding the next acceptable size */
/* -1 indicates out-of-boundary error */
static int get_size (int val)
{
	int i;

	for (i = 0; i < BUF_LIST_MAX; i++)
		if (buf_size[i] >= val)
			return buf_size[i];
    
	return -1;
}

static bool buf_cat(BUFFER *buffer, const char *string)
{
	int len;
	char *oldstr;
	int oldsize;

	oldstr = buffer->string;
	oldsize = buffer->size;

	if (buffer->state != BUFFER_SAFE)
		/* don't waste time on bad strings! */
		return FALSE;

	len = strlen(buffer->string) + strlen(string) + 1;

	if (len >= buffer->size) { /* increase the buffer size */
		buffer->size 	= get_size(len);
		if (buffer->size == -1) { /* overflow */
			buffer->size = oldsize;
			buffer->state = BUFFER_OVERFLOW;
			log_printf("buf_add: '%s': buffer overflow", string);
			return FALSE;
		}
	}

	if (buffer->size != oldsize) {
		char *p;

		p = realloc(buffer->string, buffer->size);
		if (p == NULL) {
			buffer->size = oldsize;
			buffer->state = BUFFER_OVERFLOW;
			log_printf("buf_add: '%s': realloc failed", string);
			return FALSE;
		}

		buffer->string	= p;
		sAllocBuf += buffer->size - oldsize;
	}

	strcat(buffer->string, string);
	return TRUE;
}

