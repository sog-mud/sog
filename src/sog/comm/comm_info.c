/*-
 * Copyright (c) 1998 fjoe <fjoe@iclub.nsu.ru>
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
 * $Id: comm_info.c,v 1.7 1999-06-10 11:47:33 fjoe Exp $
 */

#include <sys/types.h>
#if	!defined (WIN32)
#	include <sys/socket.h>
#	include <netinet/in.h>
#	include <arpa/inet.h>
#	include <unistd.h>
#else
#	include <winsock.h>
#endif

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "merc.h"
#include "comm_info.h"
#include "comm_colors.h"

extern int	max_on;

INFO_DESC *	id_list;
int		top_id;

static INFO_DESC *	id_free_list;

static INFO_DESC *	info_desc_new(int fd);
static void		info_desc_free(INFO_DESC *id);

void info_newconn(int infofd)
{
	int fd;
	struct sockaddr_in sock;
	int size = sizeof(sock);
	INFO_DESC *id;
	int i;

	getsockname(infofd, (struct sockaddr*) &sock, &size);
	if ((fd = accept(infofd, (struct sockaddr*) &sock, &size)) < 0) {
		log("info_newconn: accept: %s", strerror(errno));
		return;
	}

	if (getpeername(fd, (struct sockaddr *) &sock, &size) < 0) {
		log("info_newconn: getpeername: %s", strerror(errno));
#ifdef WIN32
		closesocket(fd);
#else
		close(fd);
#endif
		return;
	}

	log("info_newconn: sock.sin_addr: %s", inet_ntoa(sock.sin_addr));

	for (i = 0; i < info_trusted.nused; i++) {
		struct in_addr* in_addr = VARR_GET(&info_trusted, i);
		if (!memcmp(in_addr, &sock.sin_addr, sizeof(struct in_addr)))
			break;
	}

	if (i >= info_trusted.nused) {
		log("info_newconn: incoming connection refused");
#ifdef WIN32
		closesocket(fd);
#else
		close(fd);
#endif
		return;
	}

#if !defined (WIN32)
	if (fcntl(fd, F_SETFL, FNDELAY) < 0) {
		log("info_newconn: fcntl: FNDELAY: %s", strerror(errno));
		close(fd);
		return;
	}
#endif

	id = info_desc_new(fd);
	id->next = id_list;
	id_list = id;
}

void info_process_cmd(INFO_DESC *id)
{
	BUFFER *output;
	char buf[MAX_STRING_LENGTH * 2];
	char *p;
	char *q;
	int format;

	DESCRIPTOR_DATA *d;
	int nread;

#if !defined (WIN32)
	if ((nread = read(id->fd, buf, sizeof(buf))) < 0)
	 {
		if (errno == EWOULDBLOCK)
#else
	if ((nread = recv(id->fd, buf, sizeof(buf), 0)) < 0)
	 {
        if ( WSAGetLastError() == WSAEWOULDBLOCK)
#endif
			return;
		log("info_input: read: %s", strerror(errno));
		goto bail_out;
	}
	buf[nread] = '\0';

	for (p = buf; *p && strchr(" \n\r\t", *p); p++)
		;

	if ((q = strpbrk(p, " \n\r\t")))
		*q = '\0';
	log("process_who: output format requested: '%s'", p);
	format = format_lookup(p);

	output = buf_new(-1);

	buf_printf(output, "%d\n", max_on);
	p = buf_string(output);
#if !defined (WIN32)
	write(id->fd, p, strlen(p));
#else
	send(id->fd, p, strlen(p), 0);
#endif

	for (d = descriptor_list; d; d = d->next) {
		CHAR_DATA *wch = d->original ? d->original : d->character;

		if (!wch
		||  d->connected != CON_PLAYING
		||  wch->invis_level
		||  wch->incog_level
		||  IS_AFFECTED(wch, AFF_FADE | AFF_HIDE | AFF_CAMOUFLAGE | 
				     AFF_INVIS | AFF_IMP_INVIS))
			continue;

		buf_clear(output);
		do_who_raw(NULL, wch, output);
		parse_colors(buf_string(output), buf, sizeof(buf), format);
#if !defined (WIN32)
		write(id->fd, buf, strlen(buf));
#else
		send(id->fd, buf, strlen(buf), 0);
#endif

	}
	buf_free(output);

bail_out:
	info_desc_free(id);
}

static INFO_DESC *info_desc_new(int fd)
{
	INFO_DESC *id;

	if (id_free_list) {
		id = id_free_list;
		id_free_list = id_free_list->next;
	}
	else {
		top_id++;
		id = malloc(sizeof(*id));
	}

	id->fd = fd;

	return id;
}

static void info_desc_free(INFO_DESC *id)
{
	if (id == id_list)
		id_list = id_list->next;
	else {
		INFO_DESC *prev;

		for (prev = id_list; prev; prev = prev->next)
			if (prev->next == id)
				break;

		if (!prev) {
			log("info_desc_free: descriptor not found");
			return;
		}

		prev->next = id->next;
	}

#ifdef WIN32
	closesocket(id->fd);
#else
	close(id->fd);
#endif

	id->next = id_free_list;
	id_free_list = id;
}

