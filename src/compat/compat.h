/*
 * $Id: compat.h,v 1.2.4.1 2003-09-30 11:31:44 fjoe Exp $
 */

#ifndef _COMPAT_H_
#define _COMPAT_H_

#if !defined(__FreeBSD__)
size_t strlcat(char *, const char *, size_t);
size_t strlcpy(char *, const char *, size_t);
#endif /* !FreeBSD */

#ifdef SUNOS
int snprintf(char*, size_t, const char*, ...);
#define vsnprintf(s, size, fmt, va) vsprintf(s, fmt, va)

char* strsep(char**, const char*);
#endif /* SUNOS */

/* Win32 stuff */

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
#endif /* WIN32 */

#endif /* _COMPAT_H_ */
