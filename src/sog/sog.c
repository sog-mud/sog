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
 * $Id: sog.c,v 1.10 2003-04-24 12:42:20 fjoe Exp $
 */

#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <setjmp.h>

#include <merc.h>
#include <module.h>
#include <resolver.h>
#include <note.h>	// load_notes()

static void	usage(const char *name);

static int	init_socket(int port);
static void	open_sockets(varr *v, const char *logm);
static void	close_sockets(varr *v);

static void	game_loop(void);

static void	log_area_popularity(void);

#if defined(WIN32)
static void	gettimeofday(struct timeval *tp, void *tzp);
#endif

static varr_info_t c_info_sockets = {
	&varr_ops, NULL, NULL,

	sizeof(int), 2
};

varr	control_sockets;
varr	info_sockets;
varr	mudftp_sockets;

int
main(int argc, char **argv)
{
	struct timeval now_time;
	int ch;
	int check_info;
	int check_mudftp;
	module_t *m;

#if defined WIN32
	WORD	wVersionRequested = MAKEWORD(1, 1);
	WSADATA	wsaData;
	int err;
#endif

	setlocale(LC_ALL, "");

	/*
	 * Init time.
	 */
	gettimeofday(&now_time, NULL);
	boot_time = current_time = (time_t) now_time.tv_sec;

	/*
	 * Run the game.
	 */

#if defined (WIN32)
	srand((unsigned) time(NULL));
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err) {
		log(LOG_INFO, "winsock.dll: %s", strerror(errno));
		exit(1);
	}
#else
	resolver_init();
#endif

	c_init(&control_sockets, &c_info_sockets);
	c_init(&info_sockets, &c_info_sockets);
	c_init(&mudftp_sockets, &c_info_sockets);

	opterr = 0;
	while ((ch = getopt(argc, argv, "p:i:f:")) != -1) { // notrans
		int *p;

		switch (ch) {
		case 'p':
			if (!is_number(optarg))
				usage(argv[0]);
			p = varr_enew(&control_sockets);
			*p = atoi(optarg);
			break;

		case 'i':
			if (!is_number(optarg))
				usage(argv[0]);
			p = varr_enew(&info_sockets);
			*p = atoi(optarg);
			break;

		case 'f':
			if (!is_number(optarg))
				usage(argv[0]);
			p = varr_enew(&mudftp_sockets);
			*p = atoi(optarg);
			break;

		default:
			usage(argv[0]);
		}
	}

	argc -= optind;
	argv += optind;

	/*
	 * load modules and call boot callbacks
	 */
	boot_modules();

	/*
	 * unload bootdb module (it is not needed anymore)
	 */
	if ((m = mod_lookup("bootdb")) == NULL)			// notrans
		log(LOG_BUG, "bootdb: no such module");
	else if (mod_unload(m) < 0)
		log(LOG_BUG, "bootdb: module can't be unloaded");

	/*
	 * load notes and bans
	 */
	load_notes();

	if (c_isempty(&control_sockets)) {
		log(LOG_INFO, "no control sockets defined");
		exit(1);
	}
	check_info = !c_isempty(&info_sockets);
	check_mudftp = !c_isempty(&mudftp_sockets);

	open_sockets(&control_sockets,
		     "ready to rock on port %d");		// notrans
	open_sockets(&info_sockets,
		     "info service started on port %d");	// notrans
	open_sockets(&mudftp_sockets,
		     "mudftp service started on port %d");	// notrans

	if (c_isempty(&control_sockets)) {
		log(LOG_INFO, "no control sockets could be opened");
		exit(1);
	}

	if (check_info && c_isempty(&info_sockets)) {
		log(LOG_INFO, "no info service sockets could be opened");
		exit(1);
	}

	if (check_mudftp && c_isempty(&mudftp_sockets)) {
		log(LOG_INFO, "no mudftp Service sockets could be opened");
		exit(1);
	}

	init_iterators();
	game_loop();

	close_sockets(&control_sockets);
	close_sockets(&info_sockets);
	close_sockets(&mudftp_sockets);

#if defined (WIN32)
	WSACleanup();
#else
	resolver_destroy();
#endif
	log_area_popularity();

	/*
	 * That's all, folks.
	 */
	log(LOG_INFO, "Normal termination of game.");
	return 0;
}

static void
usage(const char *name)
{
	fprintf(stderr, "Usage: %s [-p port] [-i port] [-f port]\n" // notrans
			"Where:\n"				// notrans
			"\t-p -- listen port\n"			// notrans
			"\t-i -- info service port\n"		// notrans
			"\t-f -- mudftp service port\n",	// notrans
		get_filename(name));
	exit(1);
}

static int
init_socket(int port)
{
	static struct sockaddr_in sa_zero;
	struct sockaddr_in sa;
	struct linger ld;
	int x = 1;
	int fd;

#if defined (WIN32)
	if ((fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
#else
	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
#endif
		log(LOG_INFO, "init_socket(%d): socket: %s",
			   port, strerror(errno));
		return -1;
	}

	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
		       (char *) &x, sizeof(x)) < 0) {
		log(LOG_INFO, "init_socket(%d): setsockopt: SO_REUSEADDR: %s",
			   port, strerror(errno));
#if defined (WIN32)
		closesocket(fd);
#else
		close(fd);
#endif
		return -1;
	}

	ld.l_onoff  = 0;
	if (setsockopt(fd, SOL_SOCKET, SO_LINGER,
		       (char *) &ld, sizeof(ld)) < 0) {
		log(LOG_INFO, "init_socket(%d): setsockopt: SO_LINGER: %s",
			   port, strerror(errno));
#if defined (WIN32)
		closesocket(fd);
#else
		close(fd);
#endif
		return -1;
	}

	sa		= sa_zero;
#if !defined (WIN32)
	sa.sin_family   = AF_INET;
#else
	sa.sin_family   = PF_INET;
#endif
	sa.sin_port	= htons(port);

	if (bind(fd, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
		log(LOG_INFO, "init_socket(%d): bind: %s", port, strerror(errno));
#if defined (WIN32)
		closesocket(fd);
#else
		close(fd);
#endif
		return -1;
	}

	if (listen(fd, 3) < 0) {
		log(LOG_INFO, "init_socket(%d): listen: %s",
			   port, strerror(errno));
#if defined (WIN32)
		closesocket(fd);
#else
		close(fd);
#endif
		return -1;
	}

	return fd;
}

static void
open_sockets(varr *v, const char *logm)
{
	size_t j = 0;
	int *pport;

	C_FOREACH(pport, v) {
		int sock;
		int *pport2;

		if ((sock = init_socket(*pport)) < 0)
			continue;

		log(LOG_INFO, logm, *pport);
		pport2 = VARR_GET(v, j);
		*pport2 = sock;
		j++;
	}

	v->nused = j;
}

static void
close_sockets(varr *v)
{
	int *pfd;

	C_FOREACH(pfd, v) {
#if defined (WIN32)
		closesocket(*pfd);
#else
		close(*pfd);
#endif
	}
}

jmp_buf game_loop_jmpbuf;
bool do_longjmp = FALSE;

static void
game_loop(void)
{
	struct timeval last_time;

	gettimeofday(&last_time, NULL);
	current_time = (time_t) last_time.tv_sec;

	/* Main loop */
	while (!merc_down) {
		fd_set in_set, out_set, exc_set;
#if !defined (WIN32)
		struct timeval now_time;
		long secDelta;
		long usecDelta;
#else
		int times_up;
		int nappy_time;
		struct _timeb start_time;
		struct _timeb end_time;
#endif

		if (setjmp(game_loop_jmpbuf) == 0) {
			if (run_game != NULL)
				run_game(&in_set, &out_set, &exc_set);
		} else {
			if (run_game_bottom != NULL)
				run_game_bottom(&in_set, &out_set, &exc_set);
		}

		/*
		 * Synchronize to a clock.
		 * Sleep(last_time + 1/PULSE_PER_SCD - now).
		 * Careful here of signed versus unsigned arithmetic.
		 */
#if !defined(WIN32)
		gettimeofday(&now_time, NULL);
		usecDelta =
		    ((int) last_time.tv_usec) - ((int) now_time.tv_usec) +
		    1000000 / PULSE_PER_SCD;
		secDelta = ((int) last_time.tv_sec) - ((int) now_time.tv_sec);

		while (usecDelta < 0) {
			usecDelta += 1000000;
			secDelta  -= 1;
		}

		while (usecDelta >= 1000000) {
			usecDelta -= 1000000;
			secDelta  += 1;
		}

		if (secDelta > 0 || (secDelta == 0 && usecDelta > 0)) {
			struct timeval stall_time;

			stall_time.tv_usec = usecDelta;
			stall_time.tv_sec  = secDelta;
			if (select(0, NULL, NULL, NULL, &stall_time) < 0) {
				log(LOG_INFO, "game_loop: select: stall: %s",
				    strerror(errno));
				continue;
			}
		}
#else
		times_up = 0;
		_ftime(&start_time);

		while (times_up == 0) {
			_ftime(&end_time);

			nappy_time =
			    (int) (1000 * (double) ((end_time.time - start_time.time) +
			    ((double) (end_time.millitm - start_time.millitm) / 1000.0)))
			if (nappy_time >= (double)( 1000 / PULSE_PER_SCD ) )
				times_up = 1;
			else {
				Sleep(
				    (int) ((double) (1000 / PULSE_PER_SECOND) -
					   (double) nappy_time));
				times_up = 1;
			}
		}
#endif

		gettimeofday(&last_time, NULL);
		current_time = (time_t) last_time.tv_sec;
	}
}

static void
log_area_popularity(void)
{
	FILE *fp;
	AREA_DATA *area;

	if ((fp = dfopen(TMP_PATH, AREASTAT_FILE, "w")) == NULL)
		return;
	fprintf(fp, "\nBooted %s\n"
		    "Area popularity statistics (in char * ticks)\n", // notrans
	        strtime(boot_time));

	for (area = area_first; area != NULL; area = area->next)
		fprintf(fp, "%-60s %u\n", area->name, area->count); // notrans

	fclose(fp);
}

#if defined (WIN32)

/*
 * Windows 95 and Windows NT support functions (copied from Envy)
 */
void gettimeofday(struct timeval *tp, void *tzp)
{
	tp->tv_sec	= time(NULL);
	tp->tv_usec	= 0;
}
#endif
