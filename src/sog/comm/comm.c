/*
 * $Id: comm.c,v 1.130 1998-12-01 10:54:24 fjoe Exp $
 */

/***************************************************************************
 *     ANATOLIA 2.1 is copyright 1996-1997 Serdar BULUT, Ibrahim CANPUNAR  *	
 *     ANATOLIA has been brought to you by ANATOLIA consortium		   *
 *	 Serdar BULUT {Chronos}		bulut@rorqual.cc.metu.edu.tr       *
 *	 Ibrahim Canpunar  {Asena}	canpunar@rorqual.cc.metu.edu.tr    *	
 *	 Murat BICER  {KIO}		mbicer@rorqual.cc.metu.edu.tr	   *	
 *	 D.Baris ACAR {Powerman}	dbacar@rorqual.cc.metu.edu.tr	   *	
 *     By using this code, you have agreed to follow the terms of the      *
 *     ANATOLIA license, in the file Anatolia/anatolia.licence             *	
 ***************************************************************************/

/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku vMud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Thanks to abaddon for proof-reading our comm.c and pointing out bugs.  *
 *  Any remaining bugs are, of course, our work, not his.  :)              *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
*	ROM 2.4 is copyright 1993-1995 Russ Taylor			   *
*	ROM has been brought to you by the ROM consortium		   *
*	    Russ Taylor (rtaylor@pacinfo.com)				   *
*	    Gabrielle Taylor (gtaylor@pacinfo.com)			   *
*	    Brian Moore (rom@rom.efn.org)				   *
*	By using this code, you have agreed to follow the terms of the	   *
*	ROM license, in the file Rom24/doc/rom.license			   *
***************************************************************************/

/*
 * This file contains all of the OS-dependent stuff:
 *   startup, signals, BSD sockets for tcp/ip, i/o, timing.
 *
 * The data flow for input is:
 *    Game_loop ---> Read_from_descriptor ---> Read
 *    Game_loop ---> Read_from_buffer
 *
 * The data flow for output is:
 *    Game_loop ---> Process_Output ---> Write_to_descriptor -> Write
 *
 * The OS-dependent functions are Read_from_descriptor and Write_to_descriptor.
 * -- Furey  26 Jan 1993
 */

#include <sys/types.h>
#if	!defined (WIN32)
#	include <sys/socket.h>
#	include <netinet/in.h>
#	include <arpa/telnet.h>
#	include <arpa/inet.h>
#	include <unistd.h>
#	include <netdb.h>
#	include <sys/wait.h>
#else
#	include <winsock.h>
#	include <sys/timeb.h>
#endif

#include <sys/time.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <locale.h>
#include <stdarg.h>   
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "merc.h"
#include "hometown.h"
#include "quest.h"
#include "update.h"
#include "interp.h"
#include "mob_prog.h"
#include "ban.h"
#include "charset.h"
#include "resolver.h"
#include "olc/olc.h"
#include "db/db.h"
#include "db/word.h"
#include "comm_info.h"
#include "comm_colors.h"

#include "resource.h"

/* command procedures needed */
DECLARE_DO_FUN(do_help		);
DECLARE_DO_FUN(do_look		);
DECLARE_DO_FUN(do_skills	);
DECLARE_DO_FUN(do_outfit	);
DECLARE_DO_FUN(do_unread	);

DESCRIPTOR_DATA	*	new_descriptor	(void);
void			free_descriptor	(DESCRIPTOR_DATA *d);

/*
 * Malloc debugging stuff.
 */
#if defined(MALLOC_DEBUG)
#include <malloc.h>
extern	int	malloc_debug	(int );
extern	int	malloc_verify	(void);
#endif

bool class_ok(CHAR_DATA *ch , int class);

/*
 * Signal handling.
 * Apollo has a problem with __attribute(atomic) in signal.h,
 *   I dance around it.
 */
#if defined(apollo)
#define __attribute(x)
#endif

#include <signal.h>

#if defined(apollo)
#undef __attribute
#endif

struct codepage {
	char* name;
	unsigned char* from;
	unsigned char* to;
};

struct codepage codepages[] = {
	{ "koi8-r",		koi8_koi8,	koi8_koi8	},
	{ "alt (cp866)",	alt_koi8,	koi8_alt	},
	{ "win (cp1251)",	win_koi8,	koi8_win	},
	{ "iso (ISO-8859-5)",	iso_koi8,	koi8_iso	},
	{ "mac",		mac_koi8,	koi8_mac	},
	{ "translit",		koi8_koi8,	koi8_vola	},
};
#define NCODEPAGES (sizeof(codepages) / sizeof(struct codepage))

/*
 * Socket and TCP/IP stuff.
 */

#if defined (WIN32)
#include <winsock.h>

void    gettimeofday    args( ( struct timeval *tp, void *tzp ) );

/*  Definitions for the TELNET protocol. Copied from telnet.h */

#define IAC		255	/* interpret as command: */
#define DONT	254	/* you are not to use option */
#define DO		253	/* please, you use option */
#define WONT	252	/* I won't use option */
#define WILL	251	/* I will use option */
#define SB		250	/* interpret as subnegotiation */
#define GA		249	/* you may reverse the line */
#define EL		248	/* erase the current line */
#define EC		247	/* erase the current character */
#define AYT		246	/* are you there */
#define AO		245	/* abort output--but let prog finish */
#define IP		244	/* interrupt process--permanently */
#define BREAK	243	/* break */
#define DM		242	/* data mark--for connect. cleaning */
#define NOP		241	/* nop */
#define SE		240	/* end sub negotiation */
#define EOR		239 /* end of record (transparent mode) */
#define SYNCH	242	/* for telfunc calls */

#define TELOPT_ECHO	1	/* echo */
#endif

char	echo_off_str	[] = { IAC, WILL, TELOPT_ECHO, '\0' };
char	echo_on_str	[] = { IAC, WONT, TELOPT_ECHO, '\0' };
char 	go_ahead_str	[] = { IAC, GA, '\0' };

char *get_stat_alias		(CHAR_DATA* ch, int which);

/*
 * Global variables.
 */
DESCRIPTOR_DATA *   descriptor_list;	/* All open descriptors		*/
DESCRIPTOR_DATA *   d_next;		/* Next descriptor in loop	*/
FILE *		    fpReserve;		/* Reserved file handle		*/
bool		    merc_down;		/* Shutdown			*/
bool		    wizlock;		/* Game is wizlocked		*/
bool		    newlock;		/* Game is newlocked		*/
char		    str_boot_time[26];
time_t		    current_time;	/* time of this pulse */	
int                 iNumPlayers = 0; /* The number of players on */

const char *		info_trusted = "127.0.0.1";

extern int		max_on;

void	game_loop_unix		(int control, int infofd);
int	init_socket		(int port);
void	process_who		(int port);
void	init_descriptor		(int control);
void	close_descriptor	(DESCRIPTOR_DATA *d);
bool	read_from_descriptor	(DESCRIPTOR_DATA *d);
bool	write_to_descriptor	(int desc, char *txt, uint length);
void	resolv_done		(void);

/*
 * Other local functions (OS-independent).
 */
bool	check_parse_name	(const char *name);
bool	check_reconnect		(DESCRIPTOR_DATA *d, const char *name,
				 bool fConn);
bool	check_playing		(DESCRIPTOR_DATA *d, const char *name);
int	main			(int argc, char **argv);
void	nanny			(DESCRIPTOR_DATA *d, const char *argument);
bool	process_output		(DESCRIPTOR_DATA *d, bool fPrompt);
void	read_from_buffer	(DESCRIPTOR_DATA *d);
void	stop_idling		(CHAR_DATA *ch);
void    bust_a_prompt           (CHAR_DATA *ch);
int 	log_area_popularity(void);

int main(int argc, char **argv)
{
	struct timeval now_time;
	int port;
	int infofd;
	int control;

#if defined WIN32
	WORD	wVersionRequested = MAKEWORD(1, 1);
	WSADATA	wsaData;
	int err;
#endif

	/*
	 * Memory debugging if needed.
	 */
#if defined(MALLOC_DEBUG)
	malloc_debug(2);
#endif

	setlocale(LC_ALL, "");

	/*
	 * Init time.
	 */
	gettimeofday(&now_time, NULL);
	current_time 	= (time_t) now_time.tv_sec;
	strnzcpy(str_boot_time, strtime(current_time), sizeof(str_boot_time));

	/*
	 * Reserve one channel for our use.
	 */
	if ((fpReserve = fopen(NULL_FILE, "r")) == NULL) {
		perror(NULL_FILE);
		exit(1);
	}

	/*
	 * Get the port number.
	 */
	port = 6001;
	if (argc > 1) {
		if (!is_number(argv[1])) {
			fprintf(stderr,
				"Usage: %s [port #]\n",
				get_filename(argv[0]));
			exit(1);
		}
		else if ((port = atoi(argv[1])) <= 1024) {
			fprintf(stderr, "Port number must be above 1024.\n");
			exit(1);
		}
	}

	/*
	 * Run the game.
	 */
	
#if defined (WIN32)
	srand((unsigned) time(NULL));
	err = WSAStartup(wVersionRequested, &wsaData); 
	if (err) {
		perror("WINSOCK.DLL");
		exit(1);
	}
#else
	resolver_init();
#endif

	control = init_socket(port);
	infofd	= init_socket(port+1);

	boot_db();
	log_printf("ready to rock on port %d.", port);
	log_printf("info service started on port %d.", port+1);
	game_loop_unix(control, infofd);

#if defined (WIN32)
	closesocket(control);
	closesocket(infofd);
	WSACleanup();
#else
	close(control);
	close(infofd);
#endif

#if !defined (WIN32)
	resolver_done();
#endif
	log_area_popularity();

	/*
	 * That's all, folks.
	 */
	log("Normal termination of game.");
	return 0;
}

/* stuff for recycling descriptors */
DESCRIPTOR_DATA *descriptor_free;

DESCRIPTOR_DATA *new_descriptor(void)
{
	DESCRIPTOR_DATA *d;

	if (descriptor_free == NULL)
		d = malloc(sizeof(*d));
	else {
		d = descriptor_free;
		descriptor_free = descriptor_free->next;
	}

	memset(d, 0, sizeof(*d));
	return d;
}

void free_descriptor(DESCRIPTOR_DATA *d)
{
	if (!d)
		return;

	free_string(d->host);
	free(d->outbuf);
	d->next = descriptor_free;
	descriptor_free = d;
}

int init_socket(int port)
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
		perror("init_socket: socket");
		exit(1);
	}

	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
		       (char *) &x, sizeof(x)) < 0) {
		perror("Init_socket: SO_REUSEADDR");
#if defined (WIN32)
		closesocket(fd);
#else
		close(fd);
#endif
		exit(1);
	}

	ld.l_onoff  = 0;
	ld.l_linger = 1000;

	if (setsockopt(fd, SOL_SOCKET, SO_LINGER,
		       (char *) &ld, sizeof(ld)) < 0) {
		perror("init_socket: SO_LINGER");
#if defined (WIN32)
		closesocket(fd);
#else
		close(fd);
#endif
		exit(1);
	}

	sa		= sa_zero;
#if !defined (WIN32)
	sa.sin_family   = AF_INET;
#else
	sa.sin_family   = PF_INET;
#endif
	sa.sin_port	= htons(port);

	if (bind(fd, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
		perror("init_socket: bind");
#if defined (WIN32)
		closesocket(fd);
#else
		close(fd);
#endif
		exit(1);
	}

	if (listen(fd, 3) < 0) {
		perror("init_socket: listen");
#if defined (WIN32)
		closesocket(fd);
#else
		close(fd);
#endif
		exit(1);
	}

	return fd;
}

void game_loop_unix(int control, int infofd)
{
	static struct timeval null_time;
	struct timeval last_time;
 
	gettimeofday(&last_time, NULL);
	current_time = (time_t) last_time.tv_sec;

	/* Main loop */
	while (!merc_down) {
		fd_set in_set;
		fd_set out_set;
		fd_set exc_set;
		DESCRIPTOR_DATA *d;
		INFO_DESC *id;
		INFO_DESC *id_next;
		int maxdesc;

#if defined(MALLOC_DEBUG)
		if (malloc_verify() != 1)
			abort();
#endif

		/*
		 * Poll all active descriptors.
		 */
		FD_ZERO(&in_set );
		FD_ZERO(&out_set);
		FD_ZERO(&exc_set);

		FD_SET(control, &in_set);
		maxdesc	= control;

		FD_SET(infofd, &in_set);
		maxdesc = UMAX(maxdesc, infofd);

#if !defined (WIN32)
		FD_SET(fileno(rfin), &in_set);
		maxdesc = UMAX(maxdesc, fileno(rfin));
#endif

		for (d = descriptor_list; d; d = d->next) {
			maxdesc = UMAX(maxdesc, d->descriptor);
			FD_SET(d->descriptor, &in_set );
			FD_SET(d->descriptor, &out_set);
			FD_SET(d->descriptor, &exc_set);
		}

		for (id = id_list; id; id = id->next) {
			maxdesc = UMAX(maxdesc, id->fd);
			FD_SET(id->fd, &in_set);
		}

		if (select(maxdesc+1,
			   &in_set, &out_set, &exc_set, &null_time) < 0) {
			perror("game_loop: select");
			exit(1);
		}

#if !defined (WIN32)
		if (FD_ISSET(fileno(rfin), &in_set))
			resolv_done();
#endif

		/*
		 * New connection?
		 */
		if (FD_ISSET(control, &in_set))
			init_descriptor(control);

		/*
		 * webwho handler
		 */
		if (FD_ISSET(infofd, &in_set))
			info_newconn(infofd);

		for (id = id_list; id; id = id_next) {
			id_next = id->next;

			if (FD_ISSET(id->fd, &in_set))
				info_process_cmd(id);
		}

		/*
		 * Kick out the freaky folks.
		 */
		for (d = descriptor_list; d; d = d_next) {
			d_next = d->next;   
			if (FD_ISSET(d->descriptor, &exc_set)) {
				FD_CLR(d->descriptor, &in_set );
				FD_CLR(d->descriptor, &out_set);
				if (d->character && d->character->level > 1)
					save_char_obj(d->character, FALSE);
				d->outtop = 0;
				close_descriptor(d);
			}
		}

		/*
		 * Process input.
		 */
		for (d = descriptor_list; d != NULL; d = d_next) {
			d_next		= d->next;
			d->fcommand	= FALSE;

			if (FD_ISSET(d->descriptor, &in_set)) {
				if (d->character != NULL)
					d->character->timer = 0;

				if (!read_from_descriptor(d)) {
					FD_CLR(d->descriptor, &out_set);
					if (d->character != NULL
					&&  d->character->level > 1)
						save_char_obj(d->character,
							      FALSE);
					d->outtop = 0;
					close_descriptor(d);
					continue;
				}
			}

			if (d->character != NULL && d->character->daze > 0)
				--d->character->daze;

			if (d->character != NULL && d->character->wait > 0) {
				--d->character->wait;
				continue;
			}

			read_from_buffer(d);
			if (d->incomm[0] != '\0') {
				d->fcommand = TRUE;
				stop_idling(d->character);

				if (d->showstr_point != NULL)
					show_string(d, d->incomm);
				else if (d->pString != NULL)
					string_add(d->character, d->incomm);
				else if (d->connected == CON_PLAYING) {
					if (!run_olc_editor(d))
			    			substitute_alias(d, d->incomm);
				}
				else
					nanny(d, d->incomm);

				if (d->connected != CON_RESOLV)
					d->incomm[0]	= '\0';
			}
		}

		/*
		 * Autonomous game motion.
		 */
		update_handler();

		/*
		 * Output.
		 */
		for (d = descriptor_list; d != NULL; d = d_next) {
			d_next = d->next;

			if ((d->fcommand || d->outtop > 0)
			&&  FD_ISSET(d->descriptor, &out_set)) {
				if (!process_output(d, TRUE)) {
					if (d->character != NULL
					&&  d->character->level > 1)
						save_char_obj(d->character, FALSE);
					d->outtop = 0;
					close_descriptor(d);
				}
			}
		}

	/*
	 * Synchronize to a clock.
	 * Sleep(last_time + 1/PULSE_PER_SCD - now).
	 * Careful here of signed versus unsigned arithmetic.
	 */
#if !defined (WIN32)
	 {
	    struct timeval now_time;
	    long secDelta;
	    long usecDelta;

	    gettimeofday(&now_time, NULL);
	    usecDelta	= ((int) last_time.tv_usec) - ((int) now_time.tv_usec)
			+ 1000000 / PULSE_PER_SCD;
	    secDelta	= ((int) last_time.tv_sec) - ((int) now_time.tv_sec);
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
		if (select(0, NULL, NULL, NULL, &stall_time) < 0)
		{
		    perror("Game_loop: select: stall");
		    exit(1);
		}
	    }
	}
#else
	{
	    int times_up;
	    int nappy_time;
	    struct _timeb start_time;
	    struct _timeb end_time;
	    _ftime( &start_time );
	    times_up = 0;

	    while( times_up == 0 )
	    {
			_ftime( &end_time );
			if ( ( nappy_time = (int) ( 1000 * (double) ( ( end_time.time - start_time.time ) +
				       ( (double) ( end_time.millitm - start_time.millitm ) /
					1000.0 ) ) ) ) >= (double)( 1000 / PULSE_PER_SCD ) )
			  times_up = 1;
		else
		{
		    Sleep( (int) ( (double) ( 1000 / PULSE_PER_SECOND ) -
				  (double) nappy_time ) );
		    times_up = 1;
		}
	  }
	}
#endif
		gettimeofday(&last_time, NULL);
		current_time = (time_t) last_time.tv_sec;
	}
}

static void cp_print(DESCRIPTOR_DATA* d)
{
	char buf[MAX_STRING_LENGTH];
	int i;

	write_to_buffer(d, "\n\r", 0);
	for (i = 0; i < NCODEPAGES; i++) {
		snprintf(buf, sizeof(buf), "%s%d. %s",
			 i ? " " : "", i+1, codepages[i].name);
		write_to_buffer(d, buf, 0);
	}
	write_to_buffer(d, "\n\rSelect your codepage (non-russian players should choose translit): ", 0);
}

extern const HELP_DATA *help_greeting;

#if !defined(FNDELAY)
#define FNDELAY O_NDELAY
#endif

void init_descriptor(int control)
{
	DESCRIPTOR_DATA *dnew;
	struct sockaddr_in sock;
	int desc;
	int size;
	const char *text;

	size = sizeof(sock);
	getsockname(control, (struct sockaddr *) &sock, &size);
	if ((desc = accept(control, (struct sockaddr *) &sock, &size)) < 0) {
		perror("new_descriptor: accept");
		return;
	}

#if !defined (WIN32)
	if (fcntl(desc, F_SETFL, FNDELAY) < 0) {
		perror("new_descriptor: fcntl: FNDELAY");
		return;
	}
#endif

	/*
	 * Cons a new descriptor.
	 */
	dnew = new_descriptor();

	dnew->descriptor	= desc;
	dnew->connected		= CON_GET_CODEPAGE;
	dnew->showstr_head	= NULL;
	dnew->showstr_point	= NULL;
	dnew->pEdit		= NULL;			/* OLC */
	dnew->pString		= NULL;			/* OLC */
	dnew->editor		= NULL;			/* OLC */
	dnew->outsize		= 2000;
	dnew->outbuf		= malloc(dnew->outsize);
	dnew->wait_for_se	= 0;
	dnew->codepage		= codepages;
	dnew->host		= NULL;

	size = sizeof(sock);
	if (getpeername(desc, (struct sockaddr *) &sock, &size) < 0) {
		perror("new_descriptor: getpeername");
		return;
	}
#if defined (WIN32)
	else {
		/* Copying from ROM 2.4b6 */
		int addr;
		struct hostent *from;

		addr = ntohl(sock.sin_addr.s_addr);
		from = gethostbyaddr((char *) &sock.sin_addr,
				     sizeof(sock.sin_addr), AF_INET);
		dnew->host = str_dup(from ? from->h_name : "unknown");
	}
#endif

	log_printf("sock.sinaddr: %s", inet_ntoa(sock.sin_addr));

	dnew->next		= descriptor_list;
	descriptor_list		= dnew;

	/*
	 * Send the greeting.
	 */
	text = mlstr_mval(help_greeting->text);
	write_to_buffer(dnew, text + (text[0] == '.'), 0);
	cp_print(dnew);
}

void close_descriptor(DESCRIPTOR_DATA *dclose)
{
	CHAR_DATA *ch;
	DESCRIPTOR_DATA *d;

	if (dclose->outtop > 0)
		process_output(dclose, FALSE);

	if (dclose->snoop_by != NULL) 
		write_to_buffer(dclose->snoop_by,
				"Your victim has left the game.\n\r", 0);

	for (d = descriptor_list; d != NULL; d = d->next)
		if (d->snoop_by == dclose)
			d->snoop_by = NULL;

	if ((ch = dclose->character) != NULL) {
		log_printf("Closing link to %s.", ch->name);
		if (dclose->connected == CON_PLAYING) {
			act("$n has lost $s link.", ch, NULL, NULL, TO_ROOM);
			wiznet("Net death has claimed $N.", ch, NULL,
			       WIZ_LINKS, 0, 0);
			ch->desc = NULL;
		}
		else
	    		free_char(dclose->character);
	}

	if (d_next == dclose)
		d_next = d_next->next;   

	if (dclose == descriptor_list)
		descriptor_list = descriptor_list->next;
	else {
		DESCRIPTOR_DATA *d;

		for (d = descriptor_list; d && d->next != dclose; d = d->next)
			;
		if (d != NULL)
			d->next = dclose->next;
		else
			bug("Close_socket: dclose not found.", 0);
	}

#if !defined( WIN32 )
	close(dclose->descriptor);
#else
	closesocket(dclose->descriptor);
#endif
	free_descriptor(dclose);
}

bool read_from_descriptor(DESCRIPTOR_DATA *d)
{
	int iOld;
	int iStart;
	unsigned char *p, *q;
#if 0
	static int cm_stage = 1;
#endif

	/* 
	 * Hold horses if pending command already
	 */
	if (d->incomm[0] != '\0')
		return TRUE;

	/* Check for overflow. */
	iOld = iStart = strlen(d->inbuf);
	if (iStart >= sizeof(d->inbuf) - 10) {
		log_printf("%s input overflow!", d->host);
		write_to_descriptor(d->descriptor,
				    "\n\r*** PUT A LID ON IT!!! ***\n\r", 0);
		return FALSE;
	}

	for (; ;) {
		int nRead;

#if !defined (WIN32)
	nRead = read( d->descriptor, d->inbuf + iStart,
		     sizeof( d->inbuf ) - 10 - iStart );
#else
	nRead = recv( d->descriptor, d->inbuf + iStart,
		     sizeof( d->inbuf ) - 10 - iStart, 0 );
#endif
		if (nRead > 0) {
			iStart += nRead;
			if (d->inbuf[iStart-1] == '\n'
			||  d->inbuf[iStart-1] == '\r')
				break;
		}
		else if (nRead == 0) {
			log("EOF encountered on read.");
			return FALSE;
			break;
		}
#if !defined (WIN32)
		else if (errno == EWOULDBLOCK)
			break;
#else
        else if ( WSAGetLastError() == WSAEWOULDBLOCK)
	    break;
#endif
		else {
			perror("Read_from_descriptor");
			return FALSE;
		}
	}

	d->inbuf[iStart] = '\0';
	if (iOld == iStart)
		return TRUE;

	for (p = d->inbuf+iOld; *p; p++) {
		if (d->wait_for_se)
			goto wse;
		if (*p == IAC
		&&  (d->connected != CON_PLAYING ||
		     d->character == NULL ||
		     !IS_SET(d->character->comm, COMM_NOTELNET))) {
			switch (p[1]) {
			case DONT:
			case DO:
			case WONT:
			case WILL:
				if (p[2] == '\0')
					q = p+2;
				else
					q = p+3;
				break;

			wse:
			case SB:   
				q = strchr(p, SE);
				if (q == NULL) {
					q = strchr(p, '\0');
					d->wait_for_se = 1; 
				}
				else {
					q++; 
					d->wait_for_se = 0; 
				}
				break;

			case IAC:
				q = p+1;
				break;

			default:
				q = p+2;
				break;
			}
			memcpy(p, q, strlen(q)+1);
			if (*(p = q) == '\0')
				break;
		}
	} 
/* I don't know, why this code exist :) */
/*
#ifdef 0
	else 
	    cm_stage=0;
#endif
*/
	return TRUE;
}

/*
 * Transfer one line from input buffer to input line.
 */
void read_from_buffer(DESCRIPTOR_DATA *d)
{
	int i, j, k;

	/*
	 * Hold horses if pending command already.
	 */
	if (d->incomm[0] != '\0')
		return;

	/*
	 * Look for at least one new line.
	 */
	for (i = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++)
		if (d->inbuf[i] == '\0')
			return;

	/*
	 * Canonical input processing.
	 */
	for (i = 0, k = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++) {
		if (k >= MAX_INPUT_LENGTH - 2) {
			write_to_descriptor(d->descriptor,
					    "Line too long.\n\r", 0);

			/* skip the rest of the line */
			for (; d->inbuf[i] != '\0'; i++)
				if (d->inbuf[i] == '\n' || d->inbuf[i] == '\r')
					break;

			d->inbuf[i]   = '\n';
			d->inbuf[i+1] = '\0';
			break;
		}

		if (d->inbuf[i] == '\b' && k > 0)
			--k;
		else if ((unsigned)d->inbuf[i] >= ' ')
			d->incomm[k++] =
				d->codepage->from[(unsigned char) d->inbuf[i]];
	}

	/*
	 * Finish off the line.
	 */
	if (k == 0)
		d->incomm[k++] = ' ';
	d->incomm[k] = '\0';

	/*
	 * Deal with bozos with #repeat 1000 ...
	 */
	if (k > 1 || d->incomm[0] == '!') {
		if (d->incomm[0] != '!' && strcmp(d->incomm, d->inlast))
			d->repeat = 0;
		else {
			if (++d->repeat >= 100) {
				log_printf("%s input spamming!", d->host);
	        		if (d->character != NULL) {
					wiznet_printf(d->character,NULL,WIZ_SPAM,0,d->character->level, "SPAM SPAM SPAM %s spamming, and OUT!",d->character->name);

	    				wiznet_printf(d->character,NULL,WIZ_SPAM,0,d->character->level, "[%s]'s  Inlast:[%s] Incomm:[%s]!", d->character->name,d->inlast,d->incomm);

					d->repeat = 0;

					write_to_descriptor(d->descriptor, "\n\r*** PUT A LID ON IT!!! ***\n\r", 0);
/*		strcpy(d->incomm, "quit");	*/
					close_descriptor(d);	
					return;
				}
			}
		}
	}

	/*
	 * Do '!' substitution.
	 */
	if (d->incomm[0] == '!')
		strcpy(d->incomm, d->inlast);
	else
		strcpy(d->inlast, d->incomm);

	/*
	 * Shift the input buffer.
	 */
	while (d->inbuf[i] == '\n' || d->inbuf[i] == '\r')
		i++;
	for (j = 0; (d->inbuf[j] = d->inbuf[i+j]) != '\0'; j++)
		;
}

/*
 * Low level output function.
 */
void battle_prompt(CHAR_DATA *ch, CHAR_DATA *victim)
{
	int percent;
	char* msg;
	char buf[MAX_STRING_LENGTH];
 
        if (victim->max_hit > 0)
		percent = victim->hit * 100 / victim->max_hit;
        else
		percent = -1;
  
        if (percent >= 100)
		msg = "{Cis in perfect health{x.";
        else if (percent >= 90)
		msg = "{bhas a few scratches{x.";
        else if (percent >= 75)
		msg = "{Bhas some small but disgusting cuts{x.";
        else if (percent >= 50)
		msg = "{Gis covered with bleeding wounds{x.";
        else if (percent >= 30)
		msg = "{Yis gushing blood{x.";
        else if (percent >= 15)
		msg = "{Mis writhing in agony{x.";
        else if (percent >= 0)
		msg = "{Ris convulsing on the ground{x.";
        else
		msg = "{Ris nearly dead{x.";

	snprintf(buf, sizeof(buf), "%s %s\n", 
		 PERS(victim, ch), GETMSG(msg, ch->lang));
	buf[0] = UPPER(buf[0]);
	send_to_char(buf, ch);
}

/*
 * Some specials added by KIO 
 */
bool process_output(DESCRIPTOR_DATA *d, bool fPrompt)
{
	extern bool merc_down;

	/*
	 * Bust a prompt.
	 */
	if (!merc_down)
		if (d->showstr_point)
			write_to_buffer(d, "[Hit Return to continue]\n\r", 0);
		else if (fPrompt && d->pString && d->connected == CON_PLAYING)
			write_to_buffer(d, "  > ", 0);
		else if (fPrompt && d->connected == CON_PLAYING) {
   			CHAR_DATA *ch;
			CHAR_DATA *victim;

			ch = d->character;

			/* battle prompt */
			if ((victim = ch->fighting) != NULL
			&&  can_see(ch,victim))
				battle_prompt(ch, victim);


			ch = d->original ? d->original : d->character;
			if (!IS_SET(ch->comm, COMM_COMPACT))
				write_to_buffer(d, "\n\r", 2);

			if (IS_SET(ch->comm, COMM_PROMPT))
				bust_a_prompt(d->character);

			if (IS_SET(ch->comm,COMM_TELNET_GA))
				write_to_descriptor(d->descriptor,
						    go_ahead_str, 0);
		}

	/*
	 * Short-circuit if nothing to write.
	 */
	if (d->outtop == 0)
		return TRUE;

	/*
	 * Snoop-o-rama.
	 */
	if (d->snoop_by != NULL) {
		if (d->character != NULL)
			write_to_buffer(d->snoop_by, d->character->name, 0);
		write_to_buffer(d->snoop_by, "> ", 2);
		write_to_buffer(d->snoop_by, d->outbuf, d->outtop);
	}

	/*
	 * OS-dependent output.
	 */
	if (!write_to_descriptor(d->descriptor, d->outbuf, d->outtop)) {
		d->outtop = 0;
		return FALSE;
	}
	else {
		d->outtop = 0;
		return TRUE;
	}
}

void percent_hp(CHAR_DATA *ch, char buf[MAX_STRING_LENGTH])
{
	if (ch->hit >= 0)
		snprintf(buf, sizeof(buf), "%d%%",
			 ((100 * ch->hit) / UMAX(1,ch->max_hit)));
	else
		strnzcpy(buf, "BAD!", sizeof(buf));
}

/*
 * Bust a prompt (player settable prompt)
 * coded by Morgenes for Aldara Mud
 * bust
 */
void bust_a_prompt(CHAR_DATA *ch)
{
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	const char *str;
	const char *i;
	char *point;
	CHAR_DATA *victim;
	EXIT_DATA *pexit;
	bool found;
	const char *dir_name[] = {"N","E","S","W","U","D"};
	int door;
 
	if (IS_SET(ch->comm, COMM_AFK)) {
		char_printf(ch, "{c<AFK>{x %s", ch->prefix);
		return;
	}

	point = buf;
	str = ch->prompt;
	if (IS_NULLSTR(str))
		str = DEFAULT_PROMPT;

	while(*str != '\0') {
		if(*str != '%') {
			*point++ = *str++;
			continue;
		}

		switch(*++str) {
		default:
	        	i = "";
			break;

		case 't':
			snprintf(buf2, sizeof(buf2),
				 "%d%s", (time_info.hour % 12 == 0) ? 
				 12 : time_info.hour % 12, 
				 time_info.hour >= 12 ? "pm" : "am");
			i = buf2;
			break;

		case 'e':
			found = FALSE;
			buf2[0] = '\0';
			for (door = 0; door < 6; door++)
				if ((pexit = ch->in_room->exit[door]) != NULL
				&&  pexit ->u1.to_room != NULL
				&&  can_see_room(ch, pexit->u1.to_room)
				&&  check_blind_raw(ch)
				&&  (!IS_SET(pexit->exit_info, EX_CLOSED) ||
				     IS_IMMORTAL(ch))) {
					found = TRUE;
					strnzcat(buf2, dir_name[door],
						 sizeof(buf2));
					if (IS_SET(pexit->exit_info, EX_CLOSED))
						strnzcat(buf2, "*",
							 sizeof(buf2));
				}
			if (buf2[0])
				strnzcat(buf2, " ", sizeof(buf2));
			i = buf2;
			break;

		case 'n':
			i = ch->name;
			break;

		case 'S':
			i = ch->sex == SEX_MALE ? "Male" :
			    ch->sex == SEX_FEMALE ? "Female" :
			    "None";
			break;

		case 'y':
			percent_hp(ch, buf2);
			i = buf2;
			break;

		case 'o':
			if ((victim = ch->fighting) != NULL) {
				if (can_see(ch, victim)) {
					percent_hp(victim, buf2);
					i = buf2;
				}
				else
					i = "???";
			}
			else
				i = "None";
			break;

		case 'h':
			snprintf(buf2, sizeof(buf2), "%d", ch->hit);
			i = buf2;
			break;

		case 'H':
			snprintf(buf2, sizeof(buf2), "%d", ch->max_hit);
			i = buf2;
			break;

		case 'm':
			snprintf(buf2, sizeof(buf2), "%d", ch->mana);
			i = buf2;
			break;

		case 'M':
			snprintf(buf2, sizeof(buf2), "%d", ch->max_mana);
			i = buf2;
			break;

		case 'v':
			snprintf(buf2, sizeof(buf2), "%d", ch->move);
			i = buf2;
			break;

		case 'V':
			snprintf(buf2, sizeof(buf2), "%d", ch->max_move);
			i = buf2;
			break;

		case 'x':
			snprintf(buf2, sizeof(buf2), "%d", ch->exp);
			i = buf2;
			break;

		case 'X':
			if (!IS_NPC(ch)) {
				snprintf(buf2, sizeof(buf2), "%d",
					 exp_to_level(ch));
				i = buf2;
			}
			else
				i = "";
			break;

		case 'g':
			snprintf(buf2, sizeof(buf2), "%d", ch->gold);
			i = buf2;
			break;

		case 's':
			snprintf(buf2, sizeof(buf2), "%d", ch->silver);
			i = buf2;
			break;

		case 'a':
			i = IS_GOOD(ch) ? "good" :
			    IS_EVIL(ch) ? "evil" :
			    "neutral";
			break;

		case 'r':
			if (ch->in_room != NULL)
				i = (check_blind_raw(ch) && !room_is_dark(ch)) ?
				     mlstr_cval(ch->in_room->name, ch) :
				     "darkness";
			else
				i = "";
			break;

		case 'R':
			if(IS_IMMORTAL(ch) && ch->in_room != NULL) {
				snprintf(buf2, sizeof(buf2), "%d", ch->in_room->vnum);
				i = buf2;
			}
			else
				i = "";
			break;

		case 'z':
			if (IS_IMMORTAL(ch) && ch->in_room != NULL)
				i = ch->in_room->area->name;
			else
				i = "";
			break;

		case '%':
			i = "%%";
			break;

		case 'E':
			i = olc_ed_name(ch);
			if (!IS_NULLSTR(i)) {
				snprintf(buf2, sizeof(buf2), "%s ", i);
				i = buf2;
			}
			break;

		case 'W':
			if (ch->invis_level) {
				snprintf(buf2, sizeof(buf2), "Wizi %d ",
					 ch->invis_level);
				i = buf2;
			}
			else
				i = "";
			break;

		case 'I':
			if (ch->incog_level) {
				snprintf(buf2, sizeof(buf2), "Incog %d ",
					 ch->incog_level);
				i = buf2;
			}
			else
				i = "";
			break;
		}
		++str;
		while((*point = *i) != '\0')
			++point, ++i;
	}

	*point = '\0';
	send_to_char(buf, ch);

	if (ch->prefix[0] != '\0')
		write_to_buffer(ch->desc, ch->prefix, 0);
}

/*
 * Append onto an output buffer.
 */
void write_to_buffer(DESCRIPTOR_DATA *d, const char *txt, uint length)
{
	uint size;
	int i;
	bool noiac = (d->connected == CON_PLAYING &&
		      d->character != NULL &&
		      IS_SET(d->character->comm, COMM_NOIAC));
	
	/*
	 * Find length in case caller didn't.
	 */
	if (length <= 0)
		length = strlen(txt);

	/*
	 * Adjust size in case of IACs (they will be doubled)
	 */
	size = length;
	if (!noiac)
		for (i = 0; i < length; i++)
			if (d->codepage->to[(unsigned char) txt[i]] == IAC)
				size++;

	/*
	 * Initial \n\r if needed.
	 */
	if (d->outtop == 0 && !d->fcommand) {
		d->outbuf[0]	= '\n';
		d->outbuf[1]	= '\r';
		d->outtop	= 2;
	}

	/*
	 * Expand the buffer as needed.
	 */
	while (d->outtop + size >= d->outsize) {
		char *outbuf;

		if (d->outsize >= 32000) {
			bug("Buffer overflow. Closing.\n\r",0);
			close_descriptor(d);
			return;
 		}
		outbuf = malloc(2 * d->outsize);
		strncpy(outbuf, d->outbuf, d->outtop);
		free(d->outbuf);
		d->outbuf = outbuf;
		d->outsize *= 2;
	}

	/*
	 * Copy.
	 */
	while (length--) {
		unsigned char c;

		c = d->codepage->to[(unsigned char) *txt++];
		d->outbuf[d->outtop] = c;
		if (c == IAC)
			if (noiac)
				d->outbuf[d->outtop] = IAC_REPL;
			else 
				d->outbuf[++d->outtop] = IAC;
		d->outtop++;
	}
	return;
}

/*
 * Lowest level output function.
 * Write a block of text to the file descriptor.
 * If this gives errors on very long blocks (like 'ofind all'),
 *   try lowering the max block size.
 */
bool write_to_descriptor(int desc, char *txt, uint length)
{
	uint iStart;
	uint nWrite;
	uint nBlock;

	if (!length)
		length = strlen(txt);

	for (iStart = 0; iStart < length; iStart += nWrite) {
		nBlock = UMIN(length - iStart, 4096);
#if !defined( WIN32 )
		if ((nWrite = write(desc, txt + iStart, nBlock)) < 0) {
#else
		if ((nWrite = send(desc, txt + iStart, nBlock, 0)) < 0) {
#endif
			perror("write_to_descriptor");
			return FALSE;
		}
	} 
	return TRUE;
}

int search_sockets(DESCRIPTOR_DATA *inp)
{
	DESCRIPTOR_DATA *d;

	if (IS_IMMORTAL(inp->character))
		return 0;

	for(d = descriptor_list; d; d = d->next) {
		if(!strcmp(inp->host, d->host)) {
			if (d->character && inp->character
			&&  !strcmp(inp->character->name, d->character->name)) 
				continue;
			return 1;
		}
	}
	return 0;
}
  
int align_restrict(CHAR_DATA *ch);
int hometown_check(CHAR_DATA *ch);
int hometown_ok(CHAR_DATA *ch, int home);
int ethos_check(CHAR_DATA *ch);

void advance(CHAR_DATA *victim, int level);

/*
 * Deal with sockets that haven't logged in yet.
 */
void nanny(DESCRIPTOR_DATA *d, const char *argument)
{
	DESCRIPTOR_DATA *d_old, *d_next;
	char buf[MAX_STRING_LENGTH];
	char buf1[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *ch;
	char *pwdnew;
	int iClass,race,i;
	int obj_count;
	int obj_count2;
	OBJ_DATA *obj;
	OBJ_DATA *inobj;
	int nextquest = 0;
	struct sockaddr_in sock;
	int size;
	RACE_DATA *r;

	while (isspace(*argument))
		argument++;

	ch = d->character;

	switch (d->connected) {
	default:
		bug("Nanny: bad d->connected %d.", d->connected);
		close_descriptor(d);
		return;

	case CON_GET_CODEPAGE: {
		int num;

		if (argument[0] == '\0') {
			close_descriptor(d);
			return;
		}

		if (argument[1] != '\0'
		||  (num = argument[0] - '1') < 0
		||  num >= NCODEPAGES) {
			cp_print(d);
			break;
		}

		d->codepage = codepages+num;
		log_printf("'%s' codepage selected", d->codepage->name);
		d->connected = CON_GET_NAME;
		write_to_buffer(d, "By which name do you wish to be known? ", 0);
		break;
	}

	case CON_GET_NAME:
		if (argument[0] == '\0') {
			close_descriptor(d);
			return;
		}

		if (!check_parse_name(argument)) {
			write_to_buffer(d, "Illegal name, try another.\n\r"
					   "Name: ", 0);
			return;
		}

		load_char_obj(d, argument);
		ch   = d->character;

		if (d->host == NULL) {
			size = sizeof(sock);
			if (getpeername(d->descriptor,
					(struct sockaddr *) &sock, &size) < 0)
				d->host = str_dup("(unknown)");
			else {
#if defined (WIN32)
				printf("%s@%s\n", ch->name, inet_ntoa(sock.sin_addr));
#else
				fprintf(rfout, "%s@%s\n",
					ch->name, inet_ntoa(sock.sin_addr));
#endif
				d->connected = CON_RESOLV;
/* wait until sock.sin_addr gets resolved */
				break;
			}
		}

		/* FALLTHRU */

	case CON_RESOLV:
		if (d->host == NULL)
			break;

	/*
	 * Swiftest: I added the following to ban sites.  I don't
	 * endorse banning of sites, but Copper has few descriptors now
	 * and some people from certain sites keep abusing access by
	 * using automated 'autodialers' and leaving connections hanging.
	 *
	 * Furey: added suffix check by request of Nickel of HiddenWorlds.
	 */
	if (check_ban(d->host,BAN_ALL)) {
		write_to_buffer(d, "Your site has been banned from this mud.\n\r", 0);
		close_descriptor(d);
		return;
	}

	if (!IS_IMMORTAL(ch)) {
		if (check_ban(d->host,BAN_PLAYER)) {
			write_to_buffer(d,"Your site has been banned for players.\n\r",0);
			close_descriptor(d);
			return;
	        }

#undef NO_PLAYING_TWICE
#ifdef NO_PLAYING_TWICE
		if(search_sockets(d)) {
			write_to_buffer(d, "Playing twice is restricted...\n\r", 0);
			close_descriptor(d);
			return;
		} 
#endif
	  if (iNumPlayers > MAX_OLDIES && !IS_SET(ch->act, PLR_NEW))  {
	     sprintf(buf, 
	   "\n\rThere are currently %i players mudding out of a maximum of %i.\n\rPlease try again soon.\n\r",iNumPlayers - 1, MAX_OLDIES);
	     write_to_buffer(d, buf, 0);
	     close_descriptor(d);
	     return;
	  }
	  if (iNumPlayers > MAX_NEWBIES && IS_SET(ch->act, PLR_NEW))  {
	     sprintf(buf,
	   "\n\rThere are currently %i players mudding. New player creation is \n\rlimited to when there are less than %i players. Please try again soon.\n\r",
		     iNumPlayers - 1, MAX_NEWBIES);
	     write_to_buffer(d, buf, 0);
	     close_descriptor(d);
	     return;
	  }
	   }
	     
	if (IS_SET(ch->act, PLR_DENY))
	{
	    log_printf("Denying access to %s@%s.", argument, d->host);
	    write_to_buffer(d, "You are denied access.\n\r", 0);
	    close_descriptor(d);
	    return;
	}

		if (check_reconnect(d, argument, FALSE))
			REMOVE_BIT(ch->act, PLR_NEW);
		else if (wizlock && !IS_HERO(ch)) {
			write_to_buffer(d, "The game is wizlocked.\n\r", 0);
			close_descriptor(d);
			return;
		}

		if (!IS_SET(ch->act, PLR_NEW)) {
			/* Old player */
			write_to_descriptor(d->descriptor, echo_off_str, 0);
 			write_to_buffer(d, "Password: ", 0);
			d->connected = CON_GET_OLD_PASSWORD;
			return;
		}
		else {
			/* New player */
 			if (newlock) {
				write_to_buffer(d, "The game is newlocked.\n\r", 0);
				close_descriptor(d);
				return;
			}

			if (check_ban(d->host, BAN_NEWBIES)) {
				write_to_buffer(d, "New players are not allowed from your site.\n\r", 0);
				close_descriptor(d);
				return;
			}
 	    
 			do_help(ch,"NAME");
			d->connected = CON_CONFIRM_NEW_NAME;
			return;
		}
		break;

/* RT code for breaking link */
	case CON_BREAK_CONNECT:
		switch(*argument) {
		case 'y' : case 'Y':
			for (d_old = descriptor_list; d_old; d_old = d_next) {
				d_next = d_old->next;
				if (d_old == d || d_old->character == NULL)
					continue;

				if (str_cmp(ch->name,d_old->character->name))
					continue;

				close_descriptor(d_old);
			}

			if (check_reconnect(d, ch->name, TRUE))
				return;
			write_to_buffer(d,"Reconnect attempt failed.\n\r",0);

			/* FALLTHRU */

		case 'n' : case 'N':
	 		write_to_buffer(d,"Name: ",0);
			if (d->character != NULL) {
				free_char(d->character);
				d->character = NULL;
			}
			d->connected = CON_GET_NAME;
			break;

		default:
			write_to_buffer(d, "Please type Y or N? ", 0);
			break;
		}
		break;

	case CON_CONFIRM_NEW_NAME:
		switch (*argument) {
		case 'y': case 'Y':
			snprintf(buf, sizeof(buf),
				 "New character.\n\r"
				 "Give me a password for %s: ", ch->name);
			write_to_buffer(d, buf, 0);
			write_to_descriptor(d->descriptor, echo_off_str, 0);
			d->connected = CON_GET_NEW_PASSWORD;
			break;

		case 'n': case 'N':
			write_to_buffer(d, "Ok, what IS it, then? ", 0);
			free_char(d->character);
			d->character = NULL;
			d->connected = CON_GET_NAME;
			break;

		default:
			write_to_buffer(d, "Please type Yes or No? ", 0);
			break;
		}
		break;

	case CON_GET_NEW_PASSWORD:
#if defined(unix)
		write_to_buffer( d, "\n\r", 2 );
#endif

		if (strlen(argument) < 5) {
			write_to_buffer(d, "Password must be at least five characters long.\n\rPassword: ", 0);
			return;
		}

		pwdnew = crypt(argument, ch->name);
		free_string(ch->pcdata->pwd);
		ch->pcdata->pwd	= str_dup(pwdnew);
		write_to_buffer(d, "Please retype password: ", 0);
		d->connected = CON_CONFIRM_NEW_PASSWORD;
		break;

	case CON_CONFIRM_NEW_PASSWORD:
#if defined(unix)
	write_to_buffer( d, "\n\r", 2 );
#endif

		if (strcmp(crypt(argument, ch->pcdata->pwd), ch->pcdata->pwd)) {
			write_to_buffer(d, "Passwords don't match.\n\r"
					   "Retype password: ", 0);
			d->connected = CON_GET_NEW_PASSWORD;
			return;
		}

		write_to_descriptor(d->descriptor, (char *) echo_on_str, 0);
		write_to_buffer(d, "The Muddy MUD is home for the following races:\n\r", 0);
		do_help(ch, "RACETABLE");
		d->connected = CON_GET_NEW_RACE;
		break;

	case CON_GET_NEW_RACE:
		one_argument(argument, arg);

		if (!str_cmp(arg, "help")) {
			argument = one_argument(argument, arg);
			if (argument[0] == '\0') {
				write_to_buffer(d, "The Muddy MUD Realms is the home for the following races:\n\r", 0);
	  			do_help(ch,"RACETABLE");
			}
			else {
				do_help(ch, argument);
				write_to_buffer(d, "What is your race? ('help <race>' for more information) ",0);
			}	
			break;
		}

		race = rn_lookup(argument);
		r = RACE(race);

		if (race == 0 || !r->pcdata) {
			write_to_buffer(d, "That is not a valid race.\n\r", 0);
			write_to_buffer(d, "The following races are available:\n\r  ", 0);
			for (race = 1; race < races.nused; race++) {
				r = RACE(race);
		        	if (!r->pcdata)
	        	        	break;
				if (race == 8 || race == 14)
					write_to_buffer(d,"\n\r  ",0);
				write_to_buffer(d,"(",0);
				write_to_buffer(d, r->name, 0);
				write_to_buffer(d,") ",0);
	        	}
			write_to_buffer(d, "\n\r", 0);
			write_to_buffer(d, "What is your race? ('help <race>' for more information) ", 0);
			break;
		}

		SET_ORG_RACE(ch, race);
		ch->race = race;
		for (i=0; i < MAX_STATS;i++)
			ch->mod_stat[i] = 0;

		/* Add race stat modifiers 
		for (i = 0; i < MAX_STATS; i++)
			ch->mod_stat[i] += r->pcdata->stats[i];	*/

		/* Add race modifiers */
		ch->max_hit += r->pcdata->hp_bonus;
		ch->hit = ch->max_hit;
		ch->max_mana += r->pcdata->mana_bonus;
		ch->mana = ch->max_mana;
		ch->practice = r->pcdata->prac_bonus;

		ch->affected_by = ch->affected_by| r->aff;
		ch->imm_flags	= ch->imm_flags| r->imm;
		ch->res_flags	= ch->res_flags| r->res;
		ch->vuln_flags	= ch->vuln_flags| r->vuln;
		ch->form	= r->form;
		ch->parts	= r->parts;

		/* add cost */
		ch->pcdata->points = r->pcdata->points;
		ch->size = r->pcdata->size;

		write_to_buffer(d, "What is your sex (M/F)? ", 0);
		d->connected = CON_GET_NEW_SEX;
		break;

	case CON_GET_NEW_SEX:
	switch (argument[0])
	{
	case 'm': case 'M': ch->sex = SEX_MALE;    
			    ch->pcdata->true_sex = SEX_MALE;
			    break;
	case 'f': case 'F': ch->sex = SEX_FEMALE; 
			    ch->pcdata->true_sex = SEX_FEMALE;
			    break;
	default:
	    write_to_buffer(d, "That's not a sex.\n\rWhat IS your sex? ", 0);
	    return;
	}
	
	do_help(ch,"class help");

	strcpy(buf, "Select a class:\n\r[ ");
	sprintf(buf1,"  (Continuing:) ");
	for (iClass = 0; iClass < classes.nused; iClass++)
	{
	  if (class_ok(ch,iClass))
	    {
	     if (iClass < 7)
	      {
	      	strcat(buf, CLASS(iClass)->name);
	      	strcat(buf, " ");
	      }
	     else
	      {
	      	strcat(buf1, CLASS(iClass)->name);
	      	strcat(buf1, " ");
	      }
	    }
	}
	strcat(buf, "\n\r");
	strcat(buf1, "]:\n\r");
	write_to_buffer(d, buf, 0);
	write_to_buffer(d, buf1, 0);
	        write_to_buffer(d,
		"What is your class ('help <class>' for more information)? ",0);
	    d->connected = CON_GET_NEW_CLASS;
	    break;

	case CON_GET_NEW_CLASS:
	iClass = cln_lookup(argument);
	argument = one_argument(argument,arg);

	if (!str_cmp(arg,"help"))
	  {
	    if (argument[0] == '\0')
		do_help(ch,"class help");
	    else
		do_help(ch,argument);
	        write_to_buffer(d,
		"What is your class ('help <class>' for more information)? ",0);
	    return;
	  }

	if (iClass == -1)
	{
	    write_to_buffer(d,
		"That's not a class.\n\rWhat IS your class? ", 0);
	    return;
	}

	if (!class_ok(ch,iClass))
	  {
	    write_to_buffer(d, 
	    "That class is not available for your race or sex.\n\rChoose again: ",0);
	    return;
	  }

		ch->class = iClass;
		ch->pcdata->points += CLASS(iClass)->points;
		act("You are now $t.", ch, CLASS(iClass)->name, NULL, TO_CHAR);

		for (i = 0; i < MAX_STATS; i++)
			ch->perm_stat[i] = number_range(10, get_max_train(ch, i));

		snprintf(buf, sizeof(buf),
			 "Str:%s  Int:%s  Wis:%s  Dex:%s  Con:%s  Cha:%s\n\rAccept (Y/N)? ",
			 get_stat_alias(ch, STAT_STR),
			 get_stat_alias(ch, STAT_INT),
			 get_stat_alias(ch, STAT_WIS),
			 get_stat_alias(ch, STAT_DEX),
			 get_stat_alias(ch, STAT_CON),
			 get_stat_alias(ch, STAT_CHA));


		do_help(ch, "stats");
		write_to_buffer(d, "\n\rNow rolling for your stats (10-20+).\n\r", 0);
		write_to_buffer(d, "You don't get many trains, so choose well.\n\r", 0);
		write_to_buffer(d, buf, 0);
		d->connected = CON_ACCEPT_STATS;
		break;

	case CON_ACCEPT_STATS:
	switch(argument[0])
	  {
	  case 'H': case 'h': case '?':
	    do_help(ch,"stats");
	    break;
	  case 'y': case 'Y':	
	    for (i=0; i < MAX_STATS;i++)
	      ch->mod_stat[i] = 0;
	    write_to_buffer(d, "\n\r", 2);
	    if (!align_restrict(ch))
	    {
	    write_to_buffer(d, "You may be good, neutral, or evil.\n\r",0);
	    write_to_buffer(d, "Which alignment (G/N/E)? ",0);
	    d->connected = CON_GET_ALIGNMENT;
	    }
	    else 
	    {
	     write_to_buffer(d, "[Hit Return to Continue]\n\r",0);
	     ch->endur = 100; 
	     d->connected = CON_PICK_HOMETOWN;
	    }
	    break;
	    
		case 'n': case 'N':
			for (i = 0; i < MAX_STATS; i++)
				ch->perm_stat[i] = number_range(10, get_max_train(ch, i));

			snprintf(buf, sizeof(buf),
				 "Str:%s  Int:%s  Wis:%s  Dex:%s  Con:%s  Cha:%s\n\rAccept (Y/N)? ",
				 get_stat_alias(ch, STAT_STR),
				 get_stat_alias(ch, STAT_INT),
				 get_stat_alias(ch, STAT_WIS),
				 get_stat_alias(ch, STAT_DEX),
				 get_stat_alias(ch, STAT_CON),
				 get_stat_alias(ch, STAT_CHA));

			write_to_buffer(d, buf,0);
			d->connected = CON_ACCEPT_STATS;
			break;

		default:
			write_to_buffer(d,"Please answer (Y/N)? ",0);
			break;
		}
		break;
	    
	case CON_GET_ALIGNMENT:
	switch(argument[0])
	  {
	  case 'g' : case 'G' : 
		ch->alignment = 1000; 
	        write_to_buffer(d, "Now your character is good.\n\r",0);
		break;
	  case 'n' : case 'N' : 
		ch->alignment = 0;	
	        write_to_buffer(d, "Now your character is neutral.\n\r",0);
		break;
	  case 'e' : case 'E' : 
		ch->alignment = -1000; 
	        write_to_buffer(d, "Now your character is evil.\n\r",0);
		break;
	  default:
	    write_to_buffer(d,"That's not a valid alignment.\n\r",0);
	    write_to_buffer(d,"Which alignment (G/N/E)? ",0);
	    return;
	  }
	      write_to_buffer(d, "\n\r[Hit Return to Continue]\n\r",0);	  
	      ch->endur = 100; 
	      d->connected = CON_PICK_HOMETOWN;
	break;
	
	  case CON_PICK_HOMETOWN:
	sprintf(buf1,", [O]fcol");
	sprintf(buf,"[M]idgaard, [N]ew Thalos%s?",
		IS_NEUTRAL(ch) ? buf1 : str_empty);
	if (ch->endur)
	 {
	  ch->endur = 0;
	  if (!hometown_check(ch))
	   {
	    do_help(ch,"hometown");
	        write_to_buffer(d, buf,0);	  
	    d->connected = CON_PICK_HOMETOWN;
	    return;
	   }
	      else
	   {
	        write_to_buffer(d, "[Hit Return to Continue]\n\r",0);	  
	    ch->endur = 100;
	    d->connected = CON_GET_ETHOS;
	   }
	      break;
	 }
	switch(argument[0]) 
	     {
	  case 'H' : case 'h' : case '?' : 
		do_help(ch, "hometown"); 
	            write_to_buffer(d, buf,0);	  
		return;
	  case 'M' : case 'm' : 
		if (hometown_ok(ch,0)) 
		 {
		  ch->hometown = 0; 
		  write_to_buffer(d,"Now your hometown is Midgaard.\n\r",0);
		  break;
		 }
 	  case 'N' : case 'n' : 
		if (hometown_ok(ch,1)) 
		 {
		  ch->hometown = 1; 
		  write_to_buffer(d,"Now your hometown is New Thalos.\n\r",0);
		  break;
		 }
	  case 'O' : case 'o' :
		if (hometown_ok(ch,3)) 
		 {
		  ch->hometown = 3; 
		  write_to_buffer(d,"Now your hometown is Ofcol.\n\r",0);
		  break;
		 }
	  default:
	   write_to_buffer(d, "\n\rThat is not a valid hometown.\n\r", 0);
	   write_to_buffer(d, 
		"Which hometown do you want <type help for more info>? ", 0);
	   return;
	 }
	    ch->endur = 100;
	    write_to_buffer(d, "\n\r[Hit Return to Continue]\n\r",0);	  
	    d->connected = CON_GET_ETHOS;
	    break;
	
	  case CON_GET_ETHOS:
	if (!ch->endur)
	 {
	  switch(argument[0]) 
	      {
	   case 'H': case 'h': case '?': 
		do_help(ch, "alignment"); return; break;
	   case 'L': case 'l': 
	 	sprintf(buf,"\n\rNow you are lawful-%s.\n\r",
		   IS_GOOD(ch) ? "good" : IS_EVIL(ch) ? "evil" : "neutral");
	        write_to_buffer(d, buf, 0);
		ch->ethos = ETHOS_LAWFUL; 
		break;
	   case 'N': case 'n': 
	 	sprintf(buf,"\n\rNow you are neutral-%s.\n\r",
		   IS_GOOD(ch) ? "good" : IS_EVIL(ch) ? "evil" : "neutral");
	        write_to_buffer(d, buf, 0);
		ch->ethos = ETHOS_NEUTRAL; 
		break;
	   case 'C': case 'c': 
	 	sprintf(buf,"\n\rNow you are chaotic-%s.\n\r",
		   IS_GOOD(ch) ? "good" : IS_EVIL(ch) ? "evil" : "neutral");
	        write_to_buffer(d, buf, 0);
		ch->ethos = ETHOS_CHAOTIC; 
		break;
	   default:
	    write_to_buffer(d, "\n\rThat is not a valid ethos.\n\r", 0);
	    write_to_buffer(d, "What ethos do you want, (L/N/C) <type help for more info>? ",0);
	    return;
	   }
	} else {
	  ch->endur = 0;
	  if (!ethos_check(ch)) {
		write_to_buffer(d, "What ethos do you want, (L/N/C) "
				   "<type help for more info> ?", 0);
		d->connected = CON_GET_ETHOS;
		return;
	   } else
		ch->ethos = 1;
	 }
	     write_to_buffer(d, "\n\r[Hit Return to Continue]\n\r",0);
	     d->connected = CON_CREATE_DONE;
	     break;

	case CON_CREATE_DONE:
		log_printf("%s@%s new player.", ch->name, d->host);
		write_to_buffer(d, "\n\r", 2);
		do_help(ch, "motd");
		char_puts("[Press Enter to continue]", ch);
		d->connected = CON_READ_MOTD;
		break;

	case CON_GET_OLD_PASSWORD:
		write_to_buffer(d, "\n\r", 2);

	if (strcmp(crypt(argument, ch->pcdata->pwd), ch->pcdata->pwd)) {
	    write_to_buffer(d, "Wrong password.\n\r", 0);
	    log_printf("Wrong password by %s@%s", ch->name, d->host);
	    if (ch->endur == 2)
	    	close_descriptor(d);
	    else {
	    	write_to_descriptor(d->descriptor, (char *) echo_off_str, 0);
 	    	write_to_buffer(d, "Password: ", 0);
	    	d->connected = CON_GET_OLD_PASSWORD;
		ch->endur++;
	    }
	    return;
	}
 

	if (ch->pcdata->pwd[0] == (int) NULL) {
	    write_to_buffer(d, "Warning! Null password!\n\r",0);
	    write_to_buffer(d, "Please report old password with bug.\n\r",0);
	    write_to_buffer(d, 
		"Type 'password null <new password>' to fix.\n\r",0);
	}


	write_to_descriptor(d->descriptor, (char *) echo_on_str, 0);

	if (check_reconnect(d, ch->name, TRUE))
	    return;

	if (check_playing(d, ch->name))
	    return;

	/* Count objects in loaded player file */
	for (obj = ch->carrying,obj_count = 0; obj != NULL; 
	     obj = obj->next_content)
	  obj_count += get_obj_realnumber(obj);

	strcpy(buf,ch->name);

	free_char(ch);
	load_char_obj(d, buf);
	ch = d->character;

	if (IS_SET(ch->act, PLR_NEW)) {
	  write_to_buffer(d,
			  "Please login again to create a new character.\n\r",
			  0);
	  close_descriptor(d);
	  return;
	}
	  
	/* Count objects in refreshed player file */
	for (obj = ch->carrying,obj_count2 = 0; obj != NULL;
	     obj = obj->next_content)
	  obj_count2 += get_obj_realnumber(obj);


	log_printf("%s@%s has connected.", ch->name, d->host);
	d->connected = CON_READ_IMOTD;

	/* This player tried to use the clone cheat -- 
	 * Log in once, connect a second time and enter only name,
	     * drop all and quit with first character, finish login with second.
	     * This clones the player's inventory.
	     */
	if (obj_count != obj_count2) {
	  log_printf("%s@%s tried to use the clone cheat.", ch->name, d->host);
	  for (obj = ch->carrying; obj != NULL; obj = inobj) {
	    inobj = obj->next_content;
	    extract_obj_nocount(obj);
	  }

	  for (obj_count = 0; obj_count < MAX_STATS; obj_count++)
	    ch->perm_stat[obj_count]--;

	  save_char_obj(ch, FALSE);
	  char_puts("The gods frown upon your actions.\n", ch);
	}

		/* FALL THRU */

	case CON_READ_IMOTD:
		if (IS_HERO(ch))
			do_help(ch, "imotd");
		write_to_buffer(d,"\n\r",2);
		do_help(ch, "motd");
		d->connected = CON_READ_MOTD;

		/* FALL THRU */

	case CON_READ_MOTD:
		update_skills(ch);
		write_to_buffer(d, 
		"\n\rWelcome to Muddy Multi User Dungeon. Enjoy!!...\n\r",
		    0);
		ch->next	= char_list;
		char_list	= ch;
		d->connected	= CON_PLAYING;
		{
			int count;
			FILE *max_on_file;
			int tmp = 0;
			count = 0;
			for (d = descriptor_list; d != NULL; d = d->next)
				if (d->connected == CON_PLAYING)
			       		count++;
			max_on = UMAX(count, max_on);
			if(!(max_on_file = dfopen(TMP_PATH, MAXON_FILE, "r")))
				log_printf("nanny: couldn't open %s for reading",
					   MAXON_FILE);
			else {
				fscanf(max_on_file, "%d", &tmp);
				fclose(max_on_file);
			}
			if (tmp < max_on) {
				if(!(max_on_file = dfopen(TMP_PATH, MAXON_FILE, "w")))
					log_printf("nanny: couldn't open %s "
						   "for writing", MAXON_FILE);
				else {
					fprintf(max_on_file, "%d", max_on);
					log("Global max_on changed.");
					fclose(max_on_file);
				}
			}
		}

		if (ch->level == 0) {
			OBJ_DATA *wield;

			ch->level	= 1;
			ch->exp		= base_exp(ch);
			ch->hit		= ch->max_hit;
			ch->mana	= ch->max_mana;
			ch->move	= ch->max_move;
			ch->train	= 3;
			ch->practice	+= 5;
			ch->pcdata->death = 0;

			set_title(ch, title_lookup(ch));

			do_outfit(ch, str_empty);

			obj_to_char(create_obj(get_obj_index(OBJ_VNUM_MAP), 0), ch);
			obj_to_char(create_obj(get_obj_index(OBJ_VNUM_NMAP1), 0), ch);
			obj_to_char(create_obj(get_obj_index(OBJ_VNUM_NMAP2), 0), ch);

			if (ch->hometown == 0 && IS_EVIL(ch))
				obj_to_char(create_obj(get_obj_index(OBJ_VNUM_MAP_SM), 0), ch);

			if (ch->hometown == 1)
				obj_to_char(create_obj(get_obj_index(OBJ_VNUM_MAP_NT), 0), ch);

			if (ch->hometown == 3)
				obj_to_char(create_obj(get_obj_index(OBJ_VNUM_MAP_OFCOL), 0), ch);

			if (ch->hometown == 2)
				obj_to_char(create_obj(get_obj_index(OBJ_VNUM_MAP_TITAN), 0), ch);

			if (ch->hometown == 4)
				obj_to_char(create_obj(get_obj_index(OBJ_VNUM_MAP_OLD), 0), ch);

			if ((wield = get_eq_char(ch, WEAR_WIELD)))
				set_skill_raw(ch, get_weapon_sn(wield),
					      40, FALSE);

			char_to_room(ch, get_room_index(ROOM_VNUM_SCHOOL));
			char_puts("\n", ch);
			do_help(ch, "NEWBIE INFO");
			char_puts("\n", ch);
		}
		else {
			if (ch->in_room
			&&  (room_is_private(ch->in_room) ||
			     (ch->in_room->area->clan &&
			      ch->in_room->area->clan != ch->clan)))
				ch->in_room = NULL;

			if (ch->in_room) 
				char_to_room(ch, ch->in_room);
			else if (IS_IMMORTAL(ch))
				char_to_room(ch, get_room_index(ROOM_VNUM_CHAT));
			else
				char_to_room(ch, get_room_index(ROOM_VNUM_TEMPLE));
		}

		if (!IS_IMMORTAL(ch)) {
			act("$n has entered the game.", ch, NULL,NULL, TO_ROOM);
			for (i = 2; exp_for_level(ch, i) < ch->exp; i++)
				;

			if (i < ch->level) {
				int con;
				int wis;
				int inte;
				int dex;

				con = ch->perm_stat[STAT_CON];
				wis = ch->perm_stat[STAT_WIS];
				inte = ch->perm_stat[STAT_INT];
				dex = ch->perm_stat[STAT_DEX];
				ch->perm_stat[STAT_CON] = get_max_train(ch, STAT_CON);
				ch->perm_stat[STAT_WIS] = get_max_train(ch, STAT_WIS);
				ch->perm_stat[STAT_INT] = get_max_train(ch, STAT_INT);
				ch->perm_stat[STAT_DEX] = get_max_train(ch, STAT_DEX);
				do_remove(ch, "all");
				advance(ch, i-1);
		 		ch->perm_stat[STAT_CON] = con;
		 		ch->perm_stat[STAT_WIS] = wis;
		 		ch->perm_stat[STAT_INT] = inte;
		 		ch->perm_stat[STAT_DEX] = dex;
			}
		}

		reset_char(ch);

		/* quest code */
		nextquest = -abs(ch->pcdata->questtime);
		quest_cancel(ch);
		ch->pcdata->questtime = nextquest;
		/* !quest code */

		wiznet("{W$N{x joins us.", ch, NULL, WIZ_LOGINS, 0, ch->level);

		for (i = 0; i < MAX_STATS; i++) {
			int max_stat = get_max_train(ch, i);

			if (ch->perm_stat[i] > max_stat) {
				ch->train += ch->perm_stat[i] - max_stat;
				ch->perm_stat[i] = max_stat;
			}
		}

		do_look(ch, "auto");

		if (ch->gold > 6000 && !IS_IMMORTAL(ch)) {
			char_printf(ch, "You are taxed %d gold to pay for the Mayor's bar.\n\r", (ch->gold - 6000) / 2);
			ch->gold -= (ch->gold - 6000) / 2;
		}
	
		if (ch->pet != NULL) {
			char_to_room(ch->pet,ch->in_room);
			act("$n has entered the game.",
			    ch->pet, NULL, NULL, TO_ROOM);
		}

		do_unread(ch, "login");  
		break;
	}
}

/*
 * Parse a name for acceptability.
 */
bool check_parse_name(const char *name)
{
	const char *pc;
	bool fIll,adjcaps = FALSE,cleancaps = FALSE;
 	int total_caps = 0;
	int i;

	/*
	 * Reserved words.
	 */
	if (is_name(name, "all auto immortals self someone something the you "
			  "demise balance circle loner honor none clan"))
		return FALSE;
	
	/*
	 * Length restrictions.
	 */
	 
	if (strlen(name) <  2)
		return FALSE;

	if (strlen(name) > 12)
		return FALSE;

	/*
	 * Alphanumerics only.
	 * Lock out IllIll twits.
	 */
	fIll = TRUE;
	for (pc = name; *pc != '\0'; pc++) {
		if (!isalpha(*pc))
			return FALSE;

		if (isupper(*pc)) { /* ugly anti-caps hack */
			if (adjcaps)
				cleancaps = TRUE;
			total_caps++;
			adjcaps = TRUE;
		}
		else
			adjcaps = FALSE;

		if (LOWER(*pc) != 'i' && LOWER(*pc) != 'l')
			fIll = FALSE;
	}

	if (fIll)
		return FALSE;

	if (total_caps > strlen(name) / 2)
		return FALSE;

	/*
	 * Prevent players from naming themselves after mobs.
	 */
	{
		MOB_INDEX_DATA *pMobIndex;
		int iHash;

		for (iHash = 0; iHash < MAX_KEY_HASH; iHash++) {
			for (pMobIndex  = mob_index_hash[iHash];
			     pMobIndex != NULL; pMobIndex  = pMobIndex->next) 
				if (is_name(name, pMobIndex->name))
					return FALSE;
		}
	}

	for (i = 0; i < clans.nused; i++) {
		CLASS_DATA *clan = VARR_GET(&clans, i);
		if (!str_cmp(name, clan->name))
			return FALSE;
	}

	return TRUE;
}

/*
 * Look for link-dead player to reconnect.
 */
bool check_reconnect(DESCRIPTOR_DATA *d, const char *name, bool fConn)
{
	CHAR_DATA *ch;
	DESCRIPTOR_DATA *d2;

	if (!fConn) {
		for (d2 = descriptor_list; d2; d2 = d2->next) {
			if (d2 == d)
				continue;
			ch = d2->original ? d2->original : d2->character;
			if (ch && !str_cmp(d->character->name, ch->name)) {
				free_string(d->character->pcdata->pwd);
				d->character->pcdata->pwd = str_dup(ch->pcdata->pwd);
				return TRUE;
			}
		}
	}

	for (ch = char_list; ch != NULL; ch = ch->next) {
		if (!IS_NPC(ch)
		&&  (!fConn || ch->desc == NULL)
		&&  !str_cmp(d->character->name, ch->name)) {
			if (!fConn) {
				free_string(d->character->pcdata->pwd);
				d->character->pcdata->pwd = str_dup(ch->pcdata->pwd);
			}
			else {
				free_char(d->character);
				d->character	= ch;
				ch->desc	= d;
				ch->timer	= 0;
				char_puts("Reconnecting. Type replay to see missed tells.\n", ch);
				act("$n has reconnected.",
				    ch, NULL, NULL, TO_ROOM);

				log_printf("%s@%s reconnected.",
					   ch->name, d->host);
				wiznet("$N groks the fullness of $S link.",
				       ch, NULL, WIZ_LINKS, 0, 0);
				d->connected = CON_PLAYING;
			}
			return TRUE;
		}
	}

	return FALSE;
}

/*
 * Check if already playing.
 */
bool check_playing(DESCRIPTOR_DATA *d, const char *name)
{
	DESCRIPTOR_DATA *dold;

	for (dold = descriptor_list; dold; dold = dold->next) {
		if (dold != d
		&&  dold->character != NULL
		&&  dold->connected != CON_GET_CODEPAGE
		&&  dold->connected != CON_GET_NAME
		&&  dold->connected != CON_RESOLV
		&&  dold->connected != CON_GET_OLD_PASSWORD
		&&  !str_cmp(name, dold->original ?  dold->original->name :
						     dold->character->name)) {
			write_to_buffer(d, "That character is already playing.\n\r",0);
			write_to_buffer(d, "Do you wish to connect anyway (Y/N)?",0);
			d->connected = CON_BREAK_CONNECT;
			return TRUE;
		}
	}

	return FALSE;
}

void stop_idling(CHAR_DATA *ch)
{
	if (ch == NULL
	||  ch->desc == NULL
	||  ch->desc->connected != CON_PLAYING
	||  !ch->was_in_room
	||  ch->in_room->vnum != ROOM_VNUM_LIMBO)
		return;

	ch->timer = 0;
	char_from_room(ch);
	char_to_room(ch, ch->was_in_room);
	ch->was_in_room	= NULL;
	act("$n has returned from the void.", ch, NULL, NULL, TO_ROOM);
}

void char_puts(const char *txt, CHAR_DATA *ch)
{
	send_to_char(GETMSG(txt, ch->lang), ch);
}

void char_printf(CHAR_DATA *ch, const char *format, ...)
{
	char buf[MAX_STRING_LENGTH];
	va_list ap;

	va_start(ap, format);
	vsnprintf(buf, sizeof(buf), GETMSG(format, ch->lang), ap);
	va_end(ap);
	send_to_char(buf, ch);
}

/*
 * Write to one char.
 */
void send_to_char(const char *txt, CHAR_DATA *ch)
{
	char buf[MAX_STRING_LENGTH*4];

	if (txt == NULL || ch->desc == NULL)
		return;

	parse_colors(txt, buf, sizeof(buf), OUTPUT_FORMAT(ch));
	write_to_buffer(ch->desc, buf, 0);
}

/*
 * Send a page to one char.
 */
void page_to_char(const char *txt, CHAR_DATA *ch)
{
	if (txt == NULL || ch->desc == NULL)
		return; /* ben yazdim ibrahim */

	if (ch->lines == 0) {
		char_puts(txt, ch);
		return;
	}
	
	ch->desc->showstr_head = str_dup(txt);
	ch->desc->showstr_point = ch->desc->showstr_head;
	show_string(ch->desc, str_empty);
}

/* string pager */
void show_string(struct descriptor_data *d, char *input)
{
	char buffer[4*MAX_STRING_LENGTH];
	char buf[MAX_INPUT_LENGTH];
	char *scan;
	const char *chk;
	int lines = 0, toggle = 1;
	int show_lines;

	one_argument(input, buf);
	if (buf[0] != '\0') {
		if (d->showstr_head) {
			free_string(d->showstr_head);
			d->showstr_head = NULL;
		}
		d->showstr_point  = NULL;
		return;
	}

	if (d->character)
		show_lines = d->character->lines;
	else
		show_lines = 0;

	for (scan = buffer; ; scan++, d->showstr_point++) {
		if (((*scan = *d->showstr_point) == '\n' || *scan == '\r')
		&&  (toggle = -toggle) < 0)
			lines++;
		else if (!*scan || (show_lines > 0 && lines >= show_lines)) {
			*scan = '\0';
			send_to_char(buffer, d->character);
			for (chk = d->showstr_point; isspace(*chk); chk++);

			if (!*chk) {
				if (d->showstr_head) {
					free_string(d->showstr_head);
					d->showstr_head = NULL;
				}
				d->showstr_point  = NULL;
			}
			return;
		}
	}
}

/* common and slang should have the same size */
char common[] =
	"aAbBcCdDeEfFgGhHiIjJkKlLmMnNoOpPqQrRsStTuUvVwWxXyYzZ"
	"áÁâÂ÷×çÇäÄåÅ³£öÖúÚéÉêÊëËìÌíÍîÎïÏðÐòÒóÓôÔõÕæÆèÈãÃþÞûÛýÝÿßùÙøØüÜàÀñÑ";
char slang[] =
	"eEcCdDfFiIgGhHjJoOkKlLmMnNpPuUqQrRsStTvVyYwWxXzZaAbB"
	"õÕâÂâÂïÏâÂáÁùÙâÂâÂõÕâÂâÂâÂâÂéÉáÁâÂâÂòÒâÂùÙâÂâÂâÂâÂâÂâÂÿßõÕøØáÁõÕùÙ";

/* ch says, victim hears */
char *translate(CHAR_DATA *ch, CHAR_DATA *victim, const char *i)
{
	static char trans[MAX_STRING_LENGTH];
	char *o;
	RACE_DATA *r;

	if (*i == '\0'
	||  (ch == NULL) || (victim == NULL)
	||  IS_NPC(ch) || IS_NPC(victim)
	||  IS_IMMORTAL(ch) || IS_IMMORTAL(victim)
	||  ch->slang == SLANG_COMMON
	||  ((r = race_lookup(ORG_RACE(victim))) &&
	     r->pcdata &&
	     ch->slang == r->pcdata->slang)) {
		if (IS_IMMORTAL(victim))
			snprintf(trans, sizeof(trans), "[%s] %s",
				 flag_string(slang_table, ch->slang), i);
		else
			strnzcpy(trans, i, sizeof(trans));
		return trans;
	}

	snprintf(trans, sizeof(trans), "[%s] ",
		 flag_string(slang_table, ch->slang));
	o = strchr(trans, '\0');
	for (; *i && o-trans < sizeof(trans)-1; i++, o++) {
		char *p = strchr(common, *i);
		*o = p ? slang[p-common] : *i;
	}
	*o = '\0';

	return trans;
}

static char * const he_she  [] = { "it",  "he",  "she" };
static char * const him_her [] = { "it",  "him", "her" };
static char * const his_her [] = { "its", "his", "her" };
 
struct tdata {
	char	type;
	int	arg;
	char *	p;
};

#define TSTACK_SZ 4

static int SEX(CHAR_DATA *ch, CHAR_DATA *looker)
{
	if (is_affected(ch, gsn_doppelganger)
	&&  (IS_NPC(looker) || !IS_SET(looker->act, PLR_HOLYLIGHT)))
		ch = ch->doppel;
	return URANGE(0, ch->sex, SEX_MAX-1);
}

static
void act_raw(CHAR_DATA *ch, CHAR_DATA *to,
	     const void *arg1, const void *arg2, char *str, int flags)
{
	CHAR_DATA *	vch = (CHAR_DATA*) arg2;
	OBJ_DATA *	obj1 = (OBJ_DATA*) arg1;
	OBJ_DATA *	obj2 = (OBJ_DATA*) arg2;
	char 		buf	[MAX_STRING_LENGTH];
	char 		tmp	[MAX_STRING_LENGTH];

	char *		point = buf;
	char *		s = str;

	struct tdata	tstack[TSTACK_SZ];
	int		sp = -1;

	while(*s) {
		char		code;
		char		subcode;
		const char *	i;

		switch (*s) {
		default:
			*point++ = *s++;
			break;

		case '}':
			if (sp < 0) {
				*point++ = *s++;
				continue;
			}

			if (sp < TSTACK_SZ) {
				const char *tr;

				*point = '\0';

				switch (tstack[sp].type) {
				case 'g':
					tr = word_gender(to->lang, tstack[sp].p,
								tstack[sp].arg);
					break;

				default:
					tr = word_case(to->lang, tstack[sp].p,
								tstack[sp].arg);
					break;
				}

				strnzcpy(tstack[sp].p, tr,
					 sizeof(buf) - (tstack[sp].p - buf));
				point = strchr(tstack[sp].p, '\0');
			}

			sp--;
			s++;
			continue;

		case '{':
			if (*(s+1) == '}')
				s++;
			*point++ = *s++;
			continue;

		case '$':
			s++;

			switch (code = *s++) {
			default:  
				i = " <@@@> ";
				log_printf("act_raw: '%s': bad code $%c",
					   str, code);
				continue;
	
			case 't': 
			case 'T':
				i = code == 't' ? arg1 : arg2;
				if (IS_SET(flags, ACT_TRANS))
					i = GETMSG(i, to->lang);
				if (IS_SET(flags, ACT_STRANS))
					i = translate(ch, to, i);
				break;
	
			case 'n':
				i = PERS(ch, to);
				break;
	
			case 'N':
				i = PERS(vch, to);
				break;
	
			case 'e':
				i = he_she[SEX(ch, to)];
				break;
	
			case 'E':
				i = he_she[SEX(vch, to)];
				break;
	
			case 'm':
				i = him_her[SEX(ch, to)];
				break;
	
			case 'M':
				i = him_her[SEX(vch, to)];
				break;
	
			case 's':
				i = his_her[SEX(ch, to)];
				break;
	
			case 'S':
				i = his_her[SEX(vch, to)];
				break;
	
			case 'p':
				i = can_see_obj(to, obj1) ?
					mlstr_cval(obj1->short_descr, to) :
					"something";
				break;
	
			case 'P':
				i = can_see_obj(to, obj2) ?
					mlstr_cval(obj2->short_descr, to) :
					"something";
				break;
	
			case 'd':
				if (IS_NULLSTR(arg2))
					i = "door";
				else {
					one_argument(arg2, tmp);
					i = tmp;
				}
				break;

			case 'g':
			case 'c':
				if (*(s+1) != '{') {
					log_printf("act_raw: '%s': "
						   "syntax error", str);
					continue;
				}

				if (++sp >= TSTACK_SZ) {
					log_printf("act_raw: '%s': "
						   "tstack overflow", str);
					continue;
				}

				tstack[sp].p = point;
				tstack[sp].type = code;
				subcode = *s++;
				s++;

				if (code == 'c') {
					tstack[sp].arg = subcode - '0';
					continue;
				}

				switch (subcode) {
				case 'v':
					tstack[sp].arg = vch->sex;
					break;

				case 'c':
					tstack[sp].arg = ch->sex;
					break;

				case 't':
					tstack[sp].arg = to->sex;
					break;

				default:
					log_printf("act_raw: '%s': "
						   "bad subcode '%c'",
						   str, subcode);
					sp--;
					break;
				}
				continue;
			}
	
			if (i) {
				while ((*point++ = *i++));
				point--;
			}
			break;
		}
	}
 
	*point++	= '\n';
	*point		= '\0';

/* first non-control char is uppercased */
	s = (char*) cstrfirst(buf);
	*s = UPPER(*s);

	parse_colors(buf, tmp, sizeof(tmp), OUTPUT_FORMAT(to));

	if (!IS_NPC(to)) {
		if ((IS_SET(to->comm, COMM_AFK) || to->desc == NULL)
		&&  IS_SET(flags, ACT_TOBUF))
			buf_add(to->pcdata->buffer, tmp);
		else if (to->desc)
			write_to_buffer(to->desc, tmp, 0);
	}
	else {
		if (!IS_SET(flags, ACT_NOTRIG))
			mp_act_trigger(tmp, to, ch, arg1, arg2, TRIG_ACT);
		if (to->desc)
			write_to_buffer(to->desc, tmp, 0);
	}
}

bool act_skip(CHAR_DATA *ch, CHAR_DATA *vch, CHAR_DATA *to,
	      int flags, int min_pos)
{
	if (to->position < min_pos)
		return TRUE;

	if (IS_SET(flags, TO_CHAR) && to != ch)
		return TRUE;
	if (IS_SET(flags, TO_VICT) && (to != vch || to == ch))
		return TRUE;
	if (IS_SET(flags, TO_ROOM) && to == ch)
		return TRUE;
	if (IS_SET(flags, TO_NOTVICT) && (to == ch || to == vch))
		return TRUE;

	if (IS_NPC(to)
	&&  to->desc == NULL
	&&  !HAS_TRIGGER(to, TRIG_ACT))
		return TRUE;
 
	if (IS_SET(flags, ACT_NOMORTAL) && !IS_NPC(to) && !IS_IMMORTAL(to))
		return TRUE;

/* twitlist handling */
	if (IS_SET(flags, ACT_NOTWIT)
	&&  !IS_NPC(to) && !IS_IMMORTAL(to)
	&&  !IS_NPC(ch) && !IS_IMMORTAL(ch)
	&&  is_name(ch->name, to->pcdata->twitlist))
		return TRUE;

/* check "deaf dumb blind" chars */
	if (IS_SET(flags, ACT_NODEAF) && is_affected(to, gsn_deafen))
		return TRUE;

/* skip verbose messages */
	if (IS_SET(flags, ACT_VERBOSE)
	&&  IS_SET(to->comm, COMM_NOVERBOSE))
		return TRUE;

	return FALSE;
}

void act_nprintf(CHAR_DATA *ch, const void *arg1, 
		 const void *arg2, int flags, int min_pos, int msgid, ...)
{
	CHAR_DATA *to;
	CHAR_DATA *vch = (CHAR_DATA *) arg2;
	char buf[MAX_STRING_LENGTH];
	va_list ap;

	if (ch == NULL || ch->in_room == NULL)
		return;

	if (IS_SET(flags, TO_VICT)) {
		if (!vch) {
			bug("Act: null vch with TO_VICT.", 0);
			return;
		}

		if (vch->in_room == NULL)
			return;

		to = vch->in_room->people;
	}
	else
		to = ch->in_room->people;
 
	va_start(ap, msgid);
	for(; to ; to = to->next_in_room) {
		if (act_skip(ch, vch, to, flags, min_pos))
			continue;
		vsnprintf(buf, sizeof(buf), vmsg(msgid, to, ch), ap);
		act_raw(ch, to, arg1, arg2, buf, flags);
	}
	va_end(ap);
}

void act_printf(CHAR_DATA *ch, const void *arg1, 
		const void *arg2, int flags, int min_pos,
		const char* format, ...)
{
	CHAR_DATA *to;
	CHAR_DATA *vch = (CHAR_DATA *) arg2;
	char buf[MAX_STRING_LENGTH];
	va_list ap;

	if (ch == NULL || ch->in_room == NULL || format == NULL)
		return;

	if (IS_SET(flags, TO_VICT)) {
		if (vch == NULL) {
	        	bug("act_printf: null vch with TO_VICT.", 0);
	        	return;
		}

		if (vch->in_room == NULL)
			return;

		to = vch->in_room->people;
	}
	else
		to = ch->in_room->people;
 
	va_start(ap, format);
	for(; to ; to = to->next_in_room) {
		if (act_skip(ch, vch, to, flags, min_pos))
			continue;
		vsnprintf(buf, sizeof(buf), GETMSG(format, to->lang), ap);
		act_raw(ch, to, arg1, arg2, buf, flags);
	}
	va_end(ap);
}

/*
 *  writes bug directly to user screen.
 */
void dump_to_scr(char *text)
{
#if !defined( WIN32 )
	write(1, text, strlen(text));
#else
	printf (text);
#endif
	
}

int log_area_popularity(void)
{
	FILE *fp;
	AREA_DATA *area;
	extern AREA_DATA *area_first;

	fp = dfopen(TMP_PATH, AREASTAT_FILE, "w");
	fprintf(fp,"\nBooted %sArea popularity statistics (in char * ticks)\n",
	        str_boot_time);

	for (area = area_first; area != NULL; area = area->next)
		if (area->count >= 5000000)
			fprintf(fp,"%-60s overflow\n",area->name);
		else
			fprintf(fp,"%-60s %u\n",area->name,area->count);

	fclose(fp);

	return 1;
}

char *get_stat_alias(CHAR_DATA *ch, int where)
{
	char *stat;
	int istat;

	if (where == STAT_STR)  {
	  istat=get_curr_stat(ch,STAT_STR);
	  if      (istat >  22) stat = "Titantic";
	  else if (istat >= 20) stat = "Herculian";
	  else if (istat >= 18) stat = "Strong";
	  else if (istat >= 14) stat = "Average";
	  else if (istat >= 10) stat = "Poor";
	  else                    stat = "Weak";
	  return(stat);
	}
	
	if (where == STAT_WIS)  {
	  istat=get_curr_stat(ch,STAT_WIS);
	  if      (istat >  22) stat = "Excellent";
	  else if (istat >= 20) stat = "Wise";
	  else if (istat >= 18) stat = "Good";
	  else if (istat >= 14) stat = "Average";
	  else if (istat >= 10) stat = "Dim";
	  else                    stat = "Fool";
	  return(stat);
	}

	if (where == STAT_CON)  {
	  istat=get_curr_stat(ch,STAT_CON);
	  if      (istat >  22) stat = "Iron";
	  else if (istat >= 20) stat = "Hearty";
	  else if (istat >= 18) stat = "Healthy";
	  else if (istat >= 14) stat = "Average";
	  else if (istat >= 10) stat = "Poor";
	  else                    stat = "Fragile";
	  return(stat);
	}

	if (where == STAT_INT)  {
	  istat=get_curr_stat(ch,STAT_INT);
	  if      (istat >  22) stat = "Genious";
	  else if (istat >= 20) stat = "Clever";
	  else if (istat >= 18) stat = "Good";
	  else if (istat >= 14) stat = "Average";
	  else if (istat >= 10) stat = "Poor";
	  else                    stat = "Hopeless";
	  return(stat);
	}
	
	if (where == STAT_DEX)  {
	  istat=get_curr_stat(ch,STAT_DEX);
	  if      (istat >  22) stat = "Fast";
	  else if (istat >= 20) stat = "Quick";
	  else if (istat >= 18) stat = "Dextrous";
	  else if (istat >= 14) stat = "Average";
	  else if (istat >= 10) stat = "Clumsy";
	  else                    stat = "Slow";
	  return(stat);
	}

	if (where == STAT_CHA)  {
	  istat=get_curr_stat(ch,STAT_CHA);
	  if      (istat >  22) stat = "Charismatic";
	  else if (istat >= 20) stat = "Familier";
	  else if (istat >= 18) stat = "Good";
	  else if (istat >= 14) stat = "Average";
	  else if (istat >= 10) stat = "Poor";
	  else                    stat = "Mongol";
	  return(stat);
	}

   bug("stat_alias: Bad stat number.", 0);
   return(NULL);

}

bool class_ok(CHAR_DATA *ch, int class)
{
	RACE_DATA *r;
	CLASS_DATA *cl;

	if ((cl = class_lookup(class)) == NULL
	||  (r = race_lookup(ORG_RACE(ch))) == NULL
	||  !r->pcdata)
		return FALSE;

	if (race_class_lookup(r, cl->name) == NULL
	||  (cl->restrict_sex >= 0 && cl->restrict_sex != ch->sex))
		return FALSE;

	return TRUE;
}

int align_restrict(CHAR_DATA *ch)
{
	DESCRIPTOR_DATA *d = ch->desc;
	RACE_DATA *r;

	if ((r = race_lookup(ORG_RACE(ch))) == NULL
	||  !r->pcdata)
		return CR_NONE;

	if (r->pcdata->restrict_align == CR_GOOD
	||  CLASS(ch->class)->restrict_align == CR_GOOD) {
		write_to_buffer(d, "Your character has good tendencies.\n\r",0);
		ch->alignment = 1000;
		return CR_GOOD;
	}

	if (r->pcdata->restrict_align == CR_NEUTRAL
	||  CLASS(ch->class)->restrict_align == CR_NEUTRAL) {
		write_to_buffer(d, "Your character has neutral tendencies.\n\r",0);
		ch->alignment = 0;
		return CR_NEUTRAL;
	}

	if (r->pcdata->restrict_align == CR_EVIL
	||  CLASS(ch->class)->restrict_align == CR_EVIL) {
		write_to_buffer(d, "Your character has evil tendencies.\n\r",0);
		ch->alignment = -1000;
		return CR_EVIL;
	}		

	return CR_NONE;		
}

int hometown_check(CHAR_DATA *ch)
{
 DESCRIPTOR_DATA *d = ch->desc;

  if (ch->class == 10 || ch->class == 11)
   {
	write_to_buffer(d,"\n\r",0);
	write_to_buffer(d,"Your hometown is Old midgaard, permanently.\n\r",0);
	ch->hometown = 4;
	write_to_buffer(d,"\n\r",0);
	return 1;
   }

  if (ORG_RACE(ch) == 11 || ORG_RACE(ch) == 12
	|| ORG_RACE(ch) == 13 || ORG_RACE(ch) == 14)
   {
	write_to_buffer(d,"\n\r",0);
	write_to_buffer(d,"Your hometown is Valley of Titans, permanently.\n\r",0);
	ch->hometown = 2;
	write_to_buffer(d,"\n\r",0);
	return 1;
   }
 return 0;
}

int hometown_ok(CHAR_DATA *ch, int home)
{
	if (!IS_NEUTRAL(ch) && home == 3) return 0;
		return 1;
}

int ethos_check(CHAR_DATA *ch)
{
	DESCRIPTOR_DATA *d = ch->desc;

	if (ch->class == CLASS_PALADIN) {
		write_to_buffer(d, "You are Lawful.\n\r", 0);
		return 1;
	}
	return 0;
}

void resolv_done()
{
#if !defined (WIN32)
	char *host;
	char buf[MAX_STRING_LENGTH];
	char *p;
	DESCRIPTOR_DATA *d;

	if (fgets(buf, sizeof(buf), rfin) == NULL)
		return;

	if ((p = strchr(buf, '\n')) == NULL) {
		log_printf("rfin: line too long, skipping to '\\n'");
		while(fgetc(rfin) != '\n')
			;
		return;
	}
	*p = '\0';

	if ((host = strchr(buf, '@')) == NULL)
		return;
	*host++ = '\0';

	log_printf("resolv_done: %s@%s", buf, host);

	for (d = descriptor_list; d; d = d->next) {
		if (d->host
		||  d->character == NULL
		||  str_cmp(buf, d->character->name))
			continue;
		d->host = str_dup(host);
		return;
	}
#endif
}

/* Windows 95 and Windows NT support functions (copied from Envy) */
#if defined (WIN32)

void gettimeofday (struct timeval *tp, void *tzp)
{
    tp->tv_sec  = time( NULL );
    tp->tv_usec = 0;
}
#endif
