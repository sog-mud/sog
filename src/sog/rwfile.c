/*-
 * Copyright (c) 1999 fjoe <fjoe@iclub.nsu.ru>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id: rwfile.c,v 1.1 1999-10-25 12:05:24 fjoe Exp $
 */

#ifdef USE_MMAP

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "typedef.h"
#include "const.h"
#include "rfile.h"
#include "log.h"

rfile_t *rfile_open(const char *dir, const char *file)
{
	char name[PATH_MAX];
	int fd;
	struct stat s;
	char *p;
	rfile_t *fp;

	snprintf(name, sizeof(name), "%s%c%s", dir, PATH_SEPARATOR, file);
	if ((fd = open(name, O_RDONLY)) < 0) {
		log("%s: %s", name, strerror(errno));
		return NULL;
	}

	if (fstat(fd, &s) < 0) {
		close(fd);
		log("%s: %s", name, strerror(errno));
		return NULL;
	}

	if ((p = mmap(NULL, s.st_size, PROT_READ, 0, fd, 0)) == MAP_FAILED) {
		close(fd);
		log("%s: %s", name, strerror(errno));
		return NULL;
	}

	if (madvise(p, s.st_size, MADV_SEQUENTIAL) < 0)
		log("%s: %s", name, strerror(errno));
		
	fp = malloc(sizeof(rfile_t));
	fp->p = p;
	fp->len = s.st_size;
	fp->pos = 0;
	fp->fd = fd;
	return fp;
}

void rfile_close(rfile_t *fp)
{
	munmap(fp->p, fp->len);
	close(fp->fd);
	free(fp);
}

#endif
