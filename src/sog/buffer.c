/*
 * $Id: buffer.c,v 1.3 1998-08-10 10:37:52 fjoe Exp $
 */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "merc.h"
#include "buffer.h"
#include "db.h"
#include "log.h"

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
		buffer = alloc_perm(sizeof(*buffer));
		nAllocBuf++;
	}
	else {
		buffer = free_list;
		free_list = free_list->next;
	}
 
	if (size == 0)
		size = BUF_DEFAULT_SIZE;

	buffer->next        = NULL;
	buffer->state       = BUFFER_SAFE;
	buffer->size        = get_size(size);
	if (buffer->size == -1) {
		log_printf("new_buf: buffer size %d: too large", size);
		exit(1);
	}
	buffer->string      = alloc_mem(buffer->size);
	buffer->string[0]   = '\0';
	VALIDATE(buffer);
 
	sAllocBuf += buffer->size;

	return buffer;
}


void buf_free(BUFFER *buffer)
{
	if (!IS_VALID(buffer))
		return;

	sAllocBuf -= buffer->size;

	free_mem(buffer->string, buffer->size);
	buffer->string = NULL;
	buffer->size   = 0;
	buffer->state  = BUFFER_FREED;
	INVALIDATE(buffer);

	buffer->next  = free_list;
	free_list      = buffer;
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
		buffer->string	= alloc_mem(buffer->size);
		strcpy(buffer->string, oldstr);
		free_mem(oldstr, oldsize);
		sAllocBuf += buffer->size - oldsize;
	}

	strcat(buffer->string, string);
	return TRUE;
}


void buf_clear(BUFFER *buffer)
{
	buffer->string[0] = '\0';
	buffer->state     = BUFFER_SAFE;
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

