#ifndef _COMPAT_H_
#define _COMPAT_H_

/*
 * $Id: compat.h,v 1.2 1998-11-02 05:28:42 fjoe Exp $
 */

char* strsep(char**, const char*);

#ifdef SUNOS
	int snprintf(char*, size_t, const char*, ...);
#	define vsnprintf(s, size, fmt, va) vsprintf(s, fmt, va)
#endif

/* Win32 stuff 	*/

#if defined (WIN32)
#include <windows.h>

void *bzero		(void *block, size_t size);
void *bcopy		(void *from, const void *to, size_t size);

typedef struct dirent
{
    WIN32_FIND_DATA FindData;
	HANDLE Data;
    char	d_name[ MAX_PATH + 1 ]; /* file's name */
	BOOL	d_firstread;			/* flag for 1st time */
} DIR;

DIR* opendir	(const char *dirname);
struct dirent *readdir	(DIR *dirstream);
int closedir	(DIR *dirstream);

#endif

#endif
