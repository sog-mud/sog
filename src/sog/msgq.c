/*-
 * Copyright (c) 2001 SoG Development Team
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
 * $Id: msgq.c,v 1.4 2004-06-09 08:24:38 tatyana Exp $
 */

#include <sys/types.h>
#include <stdlib.h>
#include <typedef.h>
#include <msgq.h>
#include <str.h>

msgq_t msgq_immtalk;
msgq_t msgq_prays;

void
msgq_init(msgq_t *msgq, size_t qlen)
{
	msgq->qlen = qlen;
	msgq->qlast = 0;
	msgq->qbuf = calloc(1, sizeof(const char *) * msgq->qlen);
}

void
msgq_destroy(msgq_t *msgq)
{
	size_t i;

	for (i = 0; i < msgq->qlen; i++)
		free_string(msgq->qbuf[i]);
	free(msgq->qbuf);
}

void
msgq_add(msgq_t *msgq, const char *msg)
{
	free_string(msgq->qbuf[msgq->qlast]);
	msgq->qbuf[msgq->qlast] = str_dup(msg);
	msgq->qlast = (msgq->qlast + 1) % msgq->qlen;
}

const char *
msgq_first(msgq_t *msgq)
{
	/*
	 * find first not-NULL element from qlast
	 */
	msgq->qcurr = msgq->qlast;
	do {
		if (msgq->qbuf[msgq->qcurr] != NULL)
			break;
		msgq->qcurr = (msgq->qcurr + 1) % msgq->qlen;
	} while (msgq->qcurr != msgq->qlast);

	/*
	 * we should return NULL if queue is empty -- we do so
	 */
	return msgq->qbuf[msgq->qcurr];
}

const char *
msgq_next(msgq_t *msgq)
{
	msgq->qcurr = (msgq->qcurr + 1) % msgq->qlen;
	if (msgq->qcurr == msgq->qlast)
		return NULL;
	return msgq->qbuf[msgq->qcurr];
}
