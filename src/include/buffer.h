#ifndef _BUFFER_H_
#define _BUFFER_H_

/*
 * $Id: buffer.h,v 1.4 1998-09-01 18:37:57 fjoe Exp $
 */

BUFFER *	buf_new		(int size);
void		buf_free	(BUFFER *buffer);
bool		buf_add		(BUFFER *buffer, const char *string);
bool		buf_printf	(BUFFER *buffer, const char *format, ...);
void		buf_clear	(BUFFER *buffer);
char *		buf_string	(BUFFER *buffer);

#endif

