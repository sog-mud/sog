#if defined (WIN32)

#include <string.h>
#include "compat.h"

void *bzero		(void *block, size_t size)
{ return memset (block, 0, size); }

void *bcopy		(void *to, const void *from, size_t size)
{ return memcpy (to, from, size); }

DIR* opendir	(const char *dirname)
{
	char fullpath [MAX_PATH];
	int	 iCnt, iTmp = strlen (dirname);
	DIR *dirstr;

	if (dirname==NULL || iTmp ==0)
		return NULL;

	dirstr = (DIR *) malloc (sizeof (DIR));

	if (dirstr==NULL)
		return NULL;

	strcpy (fullpath, dirname);
	
	for (iCnt=0; iCnt<iTmp; iCnt++)
		if (fullpath[iCnt]=='/')
			fullpath[iCnt]='\\';

	if (fullpath[iTmp-1]=='\\')
		strcat (fullpath, "*.*");
	else
		strcat (fullpath, "\\*.*");

	dirstr->d_firstread=TRUE;
	dirstr->Data = FindFirstFile ((LPCTSTR) (fullpath), &(dirstr->FindData));

	// This may be don't correct...
	if (dirstr->Data == INVALID_HANDLE_VALUE) 
	{
		free (dirstr);
		return NULL;
	}

	return dirstr;
}

struct dirent* readdir	(DIR *dirstream)
{
	if (dirstream==NULL)
		return NULL;

	// First read
	if (dirstream->d_firstread)
		dirstream->d_firstread=FALSE; // We already have readed name
	else
	{
		if (!FindNextFile (dirstream->Data, &(dirstream->FindData)))
			return NULL;
	}

	// copy and return
	strcpy (dirstream->d_name, dirstream->FindData.cFileName);
	return (struct dirent *) dirstream;
}

int closedir	(DIR *dirstream)
{
	if (dirstream==NULL)
		return -1;
	FindClose (dirstream->Data);
	free (dirstream);
	return 0;
}

#endif
