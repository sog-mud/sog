/*
 * mccp.c - support functions for mccp (the Mud Client Compression Protocol)
 *
 * see http://homepages.ihug.co.nz/~icecube/compress/ and README.Rom24-mccp
 *
 * Copyright (c) 1999, Oliver Jowett <icecube@ihug.co.nz>.
 *
 * This code may be freely distributed and used if this copyright notice is
 * retained intact.
 *
 * $Id: mccp.c,v 1.1.2.1 2002-01-05 16:22:01 matrim Exp $
 */

#include <sys/types.h>
#include <sys/time.h>
#include <arpa/telnet.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <merc.h>

#include "comm.h"

static const char compress_start  [] = { IAC, SB, TELOPT_COMPRESS, WILL, SE, '\0' };
static const char compress_start2 [] = { IAC, SB, TELOPT_COMPRESS2, IAC, SE, '\0' };

/*
 * Memory management - zlib uses these hooks to allocate and free memory
 * it needs
 */

void *
zlib_alloc(void *opaque, unsigned int items, unsigned int size)
{
	return calloc(items, size);
}

void 
zlib_free(void *opaque, void *address)
{
	free(address);
}

/*
 * Begin compressing data on `desc'
 */
bool
compressStart(DESCRIPTOR_DATA *desc)
{
	z_stream *s;
    
	if (desc->out_compress) /* already compressing */
		return TRUE;

	/* allocate and init stream, buffer */
	s = (z_stream *)malloc(sizeof(*s));
	desc->out_compress_buf = (unsigned char *)malloc(COMPRESS_BUF_SIZE);

	s->next_in = NULL;
	s->avail_in = 0;

	s->next_out = desc->out_compress_buf;
	s->avail_out = COMPRESS_BUF_SIZE;

	s->zalloc = zlib_alloc;
	s->zfree  = zlib_free;
	s->opaque = NULL;

	if (deflateInit(s, 9) != Z_OK) {
		/* problems with zlib, try to clean up */
		free(desc->out_compress_buf);
		free(s);
		return FALSE;
	}

	if (desc->mccp_support == 2)
		write_to_descriptor(desc, compress_start2, strlen(compress_start2));
	else
		write_to_descriptor(desc, compress_start, strlen(compress_start));

	/* now we're compressing */
	desc->out_compress = s;
	return TRUE;
}

/* Cleanly shut down compression on `desc' */
bool 
compressEnd(DESCRIPTOR_DATA *desc)
{
	unsigned char dummy[1];

	if (!desc->out_compress)
		return TRUE;

	desc->out_compress->avail_in = 0;
	desc->out_compress->next_in = dummy;

	/* No terminating signature is needed - receiver will get Z_STREAM_END */
	if (deflate(desc->out_compress, Z_FINISH) != Z_STREAM_END)
		return FALSE;

	if (!processCompressed(desc)) /* try to send any residual data */
		return FALSE;

	deflateEnd(desc->out_compress);
	free(desc->out_compress_buf);
	free(desc->out_compress);
	desc->out_compress = NULL;
	desc->out_compress_buf = NULL;

	return TRUE;
}

/* Try to send any pending compressed-but-not-sent data in `desc' */
bool
processCompressed(DESCRIPTOR_DATA *desc)
{
	int iStart, nBlock, nWrite, len;

	if (!desc->out_compress)
		return TRUE;
    
	/* Try to write out some data.. */
	len = desc->out_compress->next_out - desc->out_compress_buf;
	if (len > 0) {
		/* we have some data to write */
		desc->bytes_sent += len;

		for (iStart = 0; iStart < len; iStart += nWrite) {
			nBlock = UMIN (len - iStart, 4096);
			if ((nWrite = write (desc->descriptor,
			    desc->out_compress_buf + iStart, nBlock)) < 0) {
				if (errno == EAGAIN)
					break;
#if defined(LINUX)
				if (errno == ENOSR)
					break;
#endif
				return FALSE; /* write error */
			}

			if (nWrite <= 0)
				break;
		}

		if (iStart) {
			/* We wrote "iStart" bytes */
			if (iStart < len) {
				memmove(desc->out_compress_buf,
					desc->out_compress_buf+iStart, len - iStart);
			}
			desc->out_compress->next_out = desc->out_compress_buf + len - iStart;
		}
	}

	return TRUE;
}

/* write_to_descriptor, the compressed case */
bool
writeCompressed(DESCRIPTOR_DATA *desc, const char *txt, int length)
{
	z_stream *s = desc->out_compress;

	s->next_in = (unsigned char *)(const void *)txt;
	s->avail_in = length;
	desc->bytes_income += length;

	while (s->avail_in) {
		s->avail_out = COMPRESS_BUF_SIZE - (s->next_out - desc->out_compress_buf);

		if (s->avail_out) {
			int status = deflate(s, Z_SYNC_FLUSH);

			if (status != Z_OK) {
				/* Boom */
				return FALSE;
			}
		}

		/* Try to write out some data.. */
		if (!processCompressed(desc))
			return FALSE;

		/* loop */
	}

	/* Done. */
	return TRUE;
}
