/*
 * $Id: buffer.c,v 1.5 1998-09-01 18:37:57 fjoe Exp $
 */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#include "typedef.h"
#include "const.h"
#include "buffer.h"
#include "log.h"

struct buf_data
{
	BUFFER *	next;
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

/* buffer sizes */
const int buf_size[BUF_LIST_MAX] =
{
	16, 32, 64, 128, 256, 1024, 2048, 4096, 8192, 16384
};

/* local procedure for finding the next acceptable size */
/* -1 indicates out-of-boundary error */
int get_size (int val)
{
	int i;

	for (i = 0; i < BUF_LIST_MAX; i++)
		if (buf_size[i] >= val)
			return buf_size[i];
    
	return -1;
}

BUFFER *buf_new(int size)
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
 
	if (size == 0)
		size = BUF_DEFAULT_SIZE;

	buffer->next		= NULL;
	buffer->state		= BUFFER_SAFE;
	buffer->size		= get_size(size);
	if (buffer->size == -1) {
		log_printf("new_buf: buffer size %d: too large", size);
		exit(1);
	}
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

void buf_clear(BUFFER *buffer)
{
	buffer->string[0]	= '\0';
	buffer->state		= BUFFER_SAFE;
}

bool buf_printf(BUFFER *buffer, const char *format, ...)
{
	char buf[MAX_STRING_LENGTH];
	va_list ap;

	va_start(ap, format);
	vsnprintf(buf, sizeof(buf), format, ap);
	va_end(ap);

	return buf_add(buffer, buf);
}

char* buf_string(BUFFER *buffer)
{
	return buffer->string;
}

