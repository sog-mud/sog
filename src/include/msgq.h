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
 * $Id: msgq.h,v 1.3 2004-06-09 08:24:37 tatyana Exp $
 */

#ifndef _MSGQ_H_
#define _MSGQ_H_

/*
 * message queue
 */
struct msgq_t {
	size_t qlen;		/* queue length */
	size_t qlast;		/* index of last msg */
	size_t qcurr;		/* current iterator position */
	const char **qbuf;	/* messages, non-existent are NULL's */
};
typedef struct msgq_t msgq_t;

void msgq_init(msgq_t *msgq, size_t qlen);
void msgq_destroy(msgq_t *msgq);

void msgq_add(msgq_t *msgq, const char *msg);
const char *msgq_first(msgq_t *msgq);
const char *msgq_next(msgq_t *msgq);

extern msgq_t	msgq_sog;	/* SoG channel messages */
extern msgq_t	msgq_immtalk;	/* immtalk messages */
extern msgq_t	msgq_prays;	/* implore messages */

#define MSGQ_LEN_PERS	16
#define MSGQ_LEN_CHAN	16

#endif
