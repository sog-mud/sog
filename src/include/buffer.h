#ifndef _BUFFER_H_
#define _BUFFER_H_

/*
 * $Id: buffer.h,v 1.1 1998-07-03 15:18:40 fjoe Exp $
 */

struct buf_data
{
	BUFFER *	next;
	bool		valid;
	int		state;	/* error state of the buffer */
	int		size;	/* buffer size in bytes */
	char *		string; /* buffer's string */
};

BUFFER *	buf_new		(int size);
void		buf_free	(BUFFER *buffer);
bool		buf_add		(BUFFER *buffer, char *string);
bool		buf_printf	(BUFFER *buffer, const char *format, ...);
void		buf_clear	(BUFFER *buffer);

#define buf_string(buffer) (buffer->string)

#endif

