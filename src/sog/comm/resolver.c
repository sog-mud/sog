/*-
 * Copyright (c) 1998 SoG Development Team
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
 * $Id: resolver.c,v 1.12.2.2 2001-11-11 20:37:11 avn Exp $
 */

#if !defined (WIN32)

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include "typedef.h"
#include "log.h"
#include "resolver.h"

FILE *	rfin;
FILE *	rfout;

static void	cleanup(int);
static void	resolver_loop(void);

int	rpid;
int	fildes[4];

void resolver_init(void)
{
	if (pipe(fildes) < 0 || pipe(fildes+2) < 0) {
		log("resolver_init: pipe: %s", strerror(errno));
		exit(1);
	}

	signal(SIGPIPE, SIG_IGN);

	rpid = fork();
	if (rpid < 0) {
		log("resolver_init: fork: %s", strerror(errno));
		exit(1);
	}

	if (rpid == 0)
		resolver_loop();

	signal(SIGHUP, cleanup);
	signal(SIGQUIT, cleanup);
	signal(SIGILL, cleanup);
	signal(SIGABRT, cleanup);
	signal(SIGFPE, cleanup);
	signal(SIGBUS, cleanup);
	signal(SIGSEGV, cleanup);
	signal(SIGALRM, cleanup);
	signal(SIGTERM, cleanup);
#if !defined (LINUX)
	signal(SIGEMT, cleanup);
	signal(SIGSYS, cleanup);
#endif

	close(fildes[1]);
	close(fildes[2]);
	rfin = fdopen(fildes[0], "r");
	rfout = fdopen(fildes[3], "w");
	if (rfin == NULL || rfout == NULL) {
		log("resolver_init: fdopen: %s", strerror(errno));
		exit(1);
	}

	setvbuf(rfin, NULL, _IOLBF, 0);
	setvbuf(rfout, NULL, _IOLBF, 0);

	fcntl(fileno(rfin), F_SETFL, O_NONBLOCK);
}

void resolver_done(void)
{
	fclose(rfin);
	fclose(rfout);
	kill(rpid, SIGTERM);
	wait(NULL);
}

/* local functions */

static void cleanup(int s)
{
	resolver_done();
	signal(s, SIG_DFL);
	raise(s);
}

static void resolver_loop(void)
{
	FILE *fin;
	FILE *fout;
	char buf[128];

	signal(SIGINT, SIG_IGN);
	signal(SIGTRAP, SIG_IGN);

	close(fildes[0]);
	close(fildes[3]);
	fin = fdopen(fildes[2], "r");
	fout = fdopen(fildes[1], "w");
	if (fin == NULL || fout == NULL) {
		log("resolver_loop: fdopen: %s", strerror(errno));
		exit(1);
	}

	setvbuf(fin, NULL, _IOLBF, 0);
	setvbuf(fout, NULL, _IOLBF, 0);

	while(fgets(buf, sizeof(buf), fin)) {
		struct in_addr addr;
		struct hostent *hostent;
		char *p;

		if ((p = strchr(buf, '\n')) == NULL) {
			while(fgetc(fin) != '\n')
				;
			continue;
		}
		*p = '\0';

		if ((p = strchr(buf, '@')) == NULL)
			return;
		*p++ = '\0';

		log("resolver_loop: %s@%s", buf, p);

		inet_aton(p, &addr);
		hostent = gethostbyaddr((char*) &addr, sizeof(addr), AF_INET);
		fprintf(fout, "%s@%s\n",
			buf, hostent ? hostent->h_name : p);
	}

	if (errno)
		log("resolver_loop: %s", strerror(errno));
	fclose(fin);
	fclose(fout);
	exit(0);
}

#endif
