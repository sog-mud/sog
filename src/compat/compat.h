/*
 * $Id: compat.h,v 1.3 2003-09-29 23:11:19 fjoe Exp $
 */

#ifndef _COMPAT_H_
#define _COMPAT_H_

#if !defined(__FreeBSD__)
size_t strlcat(char *, const char *, size_t);
size_t strlcpy(char *, const char *, size_t);
char* strsep(char**, const char*);
#endif /* !FreeBSD */

#ifdef SUNOS
int snprintf(char*, size_t, const char*, ...);
#define vsnprintf(s, size, fmt, va) vsprintf(s, fmt, va)
#define d_namlen d_reclen
#endif /* SUNOS */

#ifdef SVR4
#define d_namlen d_reclen
#endif /* SVR4 */

#if defined(WIN32)
#include <windows.h>

typedef struct dirent
{
	WIN32_FIND_DATA FindData;
	HANDLE	Data;
	char	d_name[MAX_PATH + 1];	/* file's name */
	BOOL	d_firstread;		/* flag for 1st time */
} DIR;

DIR* opendir(const char *dirname);
struct dirent *readdir(DIR *dirstream);
int closedir(DIR *dirstream);

void gettimeofday(struct timeval *tp, void *tzp);

#define bzero(p, len)		memset(p, '\0', len)
#define bcopy(from, to, len)	memcpy(to, from, len);

#define snprintf	_snprintf
#define vsnprintf	_vsnprintf
#define unlink		_unlink
#endif /* WIN32 */

#endif /* _COMPAT_H_ */
