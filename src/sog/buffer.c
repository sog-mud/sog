/*
 * $Id: buffer.c,v 1.26 2001-02-11 14:35:43 fjoe Exp $
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

#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "typedef.h"

#include "mlstring.h"
#include "varr.h"
#include "hash.h"
#include "buffer.h"
#include "log.h"
#include "comm_act.h"
#include "db.h"
#include "str.h"
#include "memalloc.h"

struct buf_data
{
	BUFFER *	next;
	int		lang;	/* buffer language, -1 == none */
	int		size;	/* buffer size in bytes */
	char *		string; /* buffer's string */
};

#define BUF_LIST_MAX		12
#define BUF_DEFAULT_SIZE 	1024
#define BUF_OFLOW		1	/* overflow mem tag */

int	nAllocBuf;
int	sAllocBuf;

BUFFER *free_list;

static bool buf_resize(BUFFER *buffer, const char *string);
static bool buf_copy(BUFFER *buffer, int where, const char *string);
static int get_size (int val);

BUFFER *
buf_new(int lang)
{
	BUFFER *buffer;
 
	if (free_list == NULL) {
		buffer		= mem_alloc(MT_BUFFER, sizeof(*buffer));
		nAllocBuf++;
	} else {
		buffer		= free_list;
		free_list	= free_list->next;
		mem_validate(buffer);
	}
 
	buffer->next		= NULL;
	buffer->lang		= lang;
	buffer->size		= BUF_DEFAULT_SIZE;
	buffer->string		= malloc(buffer->size);
	buffer->string[0]	= '\0';
 
	sAllocBuf += buffer->size;
	return buffer;
}

void
buf_free(BUFFER *buffer)
{
	sAllocBuf -= buffer->size;

	free(buffer->string);
	buffer->string	= NULL;
	buffer->size	= 0;

	buffer->next	= free_list;
	free_list	= buffer;
	mem_invalidate(buffer);
}

bool
buf_prepend(BUFFER *buffer, const char *string)
{
	if (IS_NULLSTR(string))
		return TRUE;

	return buf_copy(buffer, BUF_START, buffer->lang < 0 ?
				string : GETMSG(string, buffer->lang));
}

bool
buf_append(BUFFER *buffer, const char *string)
{
	if (IS_NULLSTR(string))
		return TRUE;

	return buf_copy(buffer, BUF_END, buffer->lang < 0 ?
				string : GETMSG(string, buffer->lang));
}

bool
buf_printf(BUFFER *buffer, int where, const char *format, ...)
{
	char buf[MAX_STRING_LENGTH];
	va_list ap;

	va_start(ap, format);
	vsnprintf(buf, sizeof(buf),
		  buffer->lang < 0 ? format : GETMSG(format, buffer->lang), ap);
	va_end(ap);

	return buf_copy(buffer, where, buf);
}

bool
buf_act(BUFFER *buffer, int where, const char *format, CHAR_DATA *ch,
	const void *arg1, const void *arg2, const void *arg3, int act_flags)
{
	actopt_t opt;
	char tmp[MAX_STRING_LENGTH];

	opt.to_lang = UMAX(0, buffer->lang);
	opt.act_flags = act_flags;

	act_buf(format, ch, ch, arg1, arg2, arg3, &opt, tmp, sizeof(tmp));
	return buf_copy(buffer, where, tmp);
}

void
buf_clear(BUFFER *buffer)
{
	buffer->string[0] = '\0';
	mem_untag(buffer, BUF_OFLOW);
}

char *
buf_string(BUFFER *buffer)
{
	return buffer->string;
}

int
buf_lang(BUFFER *buffer)
{
	return buffer->lang;
}

/*----------------------------------------------------------------------------
 * local functions
 */

/* buffer sizes */
static const int buf_size[BUF_LIST_MAX] =
{
	16, 32, 64, 128, 256, 1024, 2048, 4096, 8192, 16384, 32768, 65536
};

/* local procedure for finding the next acceptable size */
/* -1 indicates out-of-boundary error */
static int
get_size (int val)
{
	int i;

	for (i = 0; i < BUF_LIST_MAX; i++)
		if (buf_size[i] >= val)
			return buf_size[i];
    
	return -1;
}

static bool
buf_resize(BUFFER *buffer, const char *string)
{
	int len;
	char *oldstr;
	int oldsize;

	oldstr = buffer->string;
	oldsize = buffer->size;

	if (!mem_is(buffer, MT_BUFFER)
	||  mem_tagged(buffer, BUF_OFLOW))
		/* don't waste time on bad strings! */
		return FALSE;

	len = strlen(buffer->string) + strlen(string) + 1;

	if (len >= buffer->size) { /* increase the buffer size */
		buffer->size = get_size(len);
		if (buffer->size == -1) { /* overflow */
			buffer->size = oldsize;
			mem_tag(buffer, BUF_OFLOW);
			log(LOG_INFO, "buf_append: '%s': buffer overflow", string);
			return FALSE;
		}
	}

	if (buffer->size != oldsize) {
		char *p;

		p = realloc(buffer->string, buffer->size);
		if (p == NULL) {
			buffer->size = oldsize;
			mem_tag(buffer, BUF_OFLOW);
			log(LOG_INFO, "buf_append: '%s': realloc failed", string);
			return FALSE;
		}

		buffer->string	= p;
		sAllocBuf += buffer->size - oldsize;
	}

	return TRUE;
}

static bool
buf_copy(BUFFER *buffer, int where, const char *string)
{
	if (!buf_resize(buffer, string))
		return FALSE;

	if (where == BUF_START) {
		memmove(buffer->string + strlen(string), buffer->string,
			strlen(buffer->string) + 1);
		memcpy(buffer->string, string, strlen(string));
	} else
		strcat(buffer->string, string);
	return TRUE;
}
