/*
 * $Id: winstuff.c,v 1.4 2003-09-29 23:11:19 fjoe Exp $
 */

#include <string.h>
#include <compat/compat.h>

DIR *
opendir(const char *dirname)
{
	char fullpath[MAX_PATH];
	int iCnt, iTmp = strlen (dirname);
	DIR *dirstr;

	if (dirname == NULL || iTmp == 0)
		return NULL;

	dirstr = (DIR *) malloc (sizeof (DIR));
	if (dirstr == NULL)
		return NULL;

	strlcpy(fullpath, dirname, sizeof(fullpath));
	for (iCnt = 0; iCnt < iTmp; iCnt++) {
		if (fullpath[iCnt] == '/')
			fullpath[iCnt] = '\\';
	}
	if (fullpath[iTmp - 1] == '\\')
		strlcat(fullpath, "*.*", sizeof(fullpath));
	else
		strlcat(fullpath, "\\*.*", sizeof(fullpath));
	dirstr->d_firstread = TRUE;
	dirstr->Data = FindFirstFile((LPCTSTR) fullpath, &dirstr->FindData);
	if (dirstr->Data == INVALID_HANDLE_VALUE) {
		/* This may be incorrect... */
		free (dirstr);
		return NULL;
	}
	return dirstr;
}

struct dirent *
readdir(DIR *dirstream)
{
	if (dirstream == NULL)
		return NULL;

	/* First read */
	if (dirstream->d_firstread)
		dirstream->d_firstread = FALSE; /* We already have read name */
	else {
		if (!FindNextFile (dirstream->Data, &dirstream->FindData))
			return NULL;
	}

	/* copy and return */
	strlcpy (dirstream->d_name, dirstream->FindData.cFileName,
	    sizeof(dirstream->d_name));
	return (struct dirent *) dirstream;
}

int
closedir(DIR *dirstream)
{
	if (dirstream == NULL)
		return -1;
	FindClose(dirstream->Data);
	free(dirstream);
	return 0;
}

/*
 * Copied from Envy
 */
void
gettimeofday(struct timeval *tp, void *tzp)
{
	tp->tv_sec = time(NULL);
	tp->tv_usec = 0;
}
