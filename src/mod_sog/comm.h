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
 * $Id: comm.h,v 1.8 2004-02-24 09:58:35 fjoe Exp $
 */

#ifndef _COMM_H_
#define _COMM_H_

struct codepage {
	const char *name;
	u_char *from;
	u_char *to;
};

extern struct codepage codepages[];
extern size_t codepages_sz;

void read_from_buffer(DESCRIPTOR_DATA *d);
void write_to_buffer(DESCRIPTOR_DATA *d, const char *txt, size_t len);
void write_to_snoop(DESCRIPTOR_DATA *d, const char *txt, size_t len);
bool write_to_descriptor(DESCRIPTOR_DATA *d, const char *txt, size_t length);
void append_to_qbuf(DESCRIPTOR_DATA *d, const char *txt);

void charset_print(DESCRIPTOR_DATA *d);
void show_string_addq(DESCRIPTOR_DATA *d, const char *q, ...);

/*
 * services
 */
typedef void SERVICE_FUN (DESCRIPTOR_DATA *d,
			  const char *name, const char *argument);
typedef struct service_cmd_t service_cmd_t;
struct service_cmd_t {
	const char	*name;
	SERVICE_FUN	*fun;
};
#define DECLARE_SERVICE_FUN(fun) SERVICE_FUN fun
#define SERVICE_FUN(fun)					\
	void fun(DESCRIPTOR_DATA *d,				\
		 const char *name __attribute__((unused)),	\
		 const char *argument __attribute__((unused)))

void handle_service(DESCRIPTOR_DATA *d, service_cmd_t *cmds);
DECLARE_SERVICE_FUN(service_unimpl);

void handle_info(DESCRIPTOR_DATA *d);

void handle_mudftp(DESCRIPTOR_DATA *d);
bool mudftp_push(DESCRIPTOR_DATA *d);

#define QBUF_IN_SYNC(d)	((d)->incomm_from_qbuf || (d)->qbuf[0] == '\0')

#endif /* _COMM_H_ */
