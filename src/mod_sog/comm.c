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
 * $Id: comm.c,v 1.2 2001-08-14 16:06:54 fjoe Exp $
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/telnet.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <merc.h>
#include <resolver.h>
#include <colors.h>

#include <handler.h>
#include <update.h>

#include "handler_impl.h"
#include "info.h"
#include "charset.h"
#include "comm.h"

DECLARE_RUNGAME_FUN(_run_game);
DECLARE_RUNGAME_FUN(_run_game_bottom);

static void	add_fds(varr *v, fd_set *in_set, int *maxdesc);
static void	check_fds(varr *v, fd_set *in_set, void (*new_conn_cb)(int));

static void	init_descriptor	(int control);
static bool	read_from_descriptor(DESCRIPTOR_DATA *d);

static void	read_from_buffer(DESCRIPTOR_DATA *d);

static bool	process_output	(DESCRIPTOR_DATA *d, bool fPrompt);
static void	show_string	(DESCRIPTOR_DATA *d, const char *input);
static void	stop_idling	(DESCRIPTOR_DATA *d);

static bool	outbuf_empty(DESCRIPTOR_DATA *d);
static void	outbuf_flush(DESCRIPTOR_DATA *d);
static bool	outbuf_adjust(outbuf_t *o, size_t len);

static const char	go_ahead_str[] = { IAC, GA, '\0' };

struct codepage codepages[] = {
	{ "koi8-r",		koi8_koi8,	koi8_koi8	}, // notrans
	{ "alt (cp866)",	alt_koi8,	koi8_alt	}, // notrans
	{ "win (cp1251)",	win_koi8,	koi8_win	}, // notrans
	{ "iso (ISO-8859-5)",	iso_koi8,	koi8_iso	}, // notrans
	{ "mac",		mac_koi8,	koi8_mac	}, // notrans
	{ "translit",		koi8_koi8,	koi8_vola	}, // notrans
};

#define NCODEPAGES (sizeof(codepages) / sizeof(struct codepage))

size_t codepages_sz = NCODEPAGES;

#define CODEPAGE(cp)	(codepages[cp >= NCODEPAGES ? 0 : cp])

/*
 * Write to one char.
 */
void
send_to_char(const char *txt, CHAR_DATA *ch)
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
void
page_to_char(const char *txt, CHAR_DATA *ch)
{
	DESCRIPTOR_DATA *d;

	if (txt == NULL || (d = ch->desc) == NULL)
		return;

	if (d->dvdata->pagelen == 0) {
		send_to_char(txt, ch);
		return;
	}

	ch->desc->showstr_head = str_dup(txt);
	ch->desc->showstr_point = ch->desc->showstr_head;
	show_string(ch->desc, str_empty);
}

static void
percent_hp(CHAR_DATA *ch, char buf[MAX_STRING_LENGTH])
{
	if (ch->hit >= 0) {
		snprintf(buf, sizeof(buf), "%d%%",		// notrans
			 ((100 * ch->hit) / UMAX(1, ch->max_hit)));
	} else
		strnzcpy(buf, sizeof(buf), "BAD!");		// notrans
}

/*
 * Bust a prompt (player settable prompt)
 * coded by Morgenes for Aldara Mud
 * bust
 */
void
bust_a_prompt(DESCRIPTOR_DATA *d)
{
	char buf[MAX_STRING_LENGTH];
	char *point = buf;
	const char *str = d->dvdata->prompt;
	CHAR_DATA *ch = d->character;

	if (IS_SET(ch->comm, COMM_AFK)) {
		act_puts("{c<AFK>{x $t",			// notrans
			 ch, d->dvdata->prefix, NULL,
			 TO_CHAR | ACT_NOLF, POS_DEAD);
		return;
	}

	if (IS_NULLSTR(str))
		str = DEFAULT_PROMPT;

	while (*str != '\0') {
		const char *i;
		char buf2[MAX_STRING_LENGTH];
		CHAR_DATA *victim;
		EXIT_DATA *pexit;
		bool found;
		const char *sdir_name[] = {"N","E","S","W","U","D"}; // notrans
		int door;

		if (*str != '%') {
			*point++ = *str++;
			continue;
		}

		switch(*++str) {
		default:
			i = "";
			break;

		case 'c':
			i = "\n";
			break;

		case 't':
			snprintf(buf2, sizeof(buf2),
				 "%d%s",			// notrans
				 (time_info.hour % 12 == 0) ?
					12 : time_info.hour % 12, 
				 time_info.hour >= 12 ? "pm" : "am");
			i = buf2;
			break;

		case 'e':
			found = FALSE;
			buf2[0] = '\0';
			for (door = 0; door < 6; door++)
				if ((pexit = ch->in_room->exit[door])
				&&  pexit->to_room.r
				&&  can_see_room(ch, pexit->to_room.r)
				&&  check_blind_nomessage(ch)
				&&  (!IS_SET(pexit->exit_info, EX_CLOSED) ||
				     IS_IMMORTAL(ch))) {
					found = TRUE;
					strnzcat(buf2, sizeof(buf2),
						 sdir_name[door]);
					if (IS_SET(pexit->exit_info, EX_CLOSED))
						strnzcat(buf2, sizeof(buf2),
							 "*");	// notrans
				}
			if (buf2[0])
				strnzcat(buf2, sizeof(buf2), " ");
			i = buf2;
			break;

		case 'n':
			i = ch->name;
			break;

		case 'S':
			i = mlstr_cval(&ch->gender, ch);
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
				} else
					i = "???";		// notrans
			} else
				i = "None";
			break;

		case 'T':
			if (ch->fighting && (victim = ch->fighting->fighting)) {
				if (ch == victim || can_see(ch, victim)) {
					percent_hp(victim, buf2);
					i = buf2;
				} else
					i = "???";		// notrans
			} else
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
			snprintf(buf2, sizeof(buf2), "%d", GET_EXP(ch));
			i = buf2;
			break;

		case 'X':
			if (!IS_NPC(ch)) {
				snprintf(buf2, sizeof(buf2), "%d",
					 exp_to_level(ch));
				i = buf2;
			} else
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
			i = IS_GOOD(ch) ? "good" :		// notrans
			    IS_EVIL(ch) ? "evil" :		// notrans
			    "neutral";				// notrans
			break;

		case 'r':
			if (ch->in_room)
				i = (check_blind_nomessage(ch) && !char_in_dark_room(ch)) ?
				     mlstr_cval(&ch->in_room->name, ch) :
				     "darkness";		// notrans
			else
				i = "";
			break;

		case 'R':
			if (IS_IMMORTAL(ch) && ch->in_room) {
				snprintf(buf2, sizeof(buf2), "%d",
					 ch->in_room->vnum);
				i = buf2;
			} else
				i = "";
			break;

		case 'z':
			if (IS_IMMORTAL(ch) && ch->in_room != NULL)
				i = ch->in_room->area->name;
			else
				i = "";
			break;

		case 'E':
			i = OLCED(ch) ? OLCED(ch)->name : str_empty;
			if (!IS_NULLSTR(i)) {
				snprintf(buf2, sizeof(buf2), "%s", i);
				i = buf2;
			}
			break;

		case 'W':
			if (ch->invis_level) {
				snprintf(buf2, sizeof(buf2),
					 "Wizi %d ",		// notrans
					 ch->invis_level);
				i = buf2;
			} else
				i = str_empty;
			break;

		case 'I':
			if (ch->incog_level) {
				snprintf(buf2, sizeof(buf2),
					 "Incog %d ",		// notrans
					 ch->incog_level);
				i = buf2;
			} else
				i = str_empty;
			break;

		case 'w':
			if (ch->invis_level) {
				snprintf(buf2, sizeof(buf2), "%d",
					 ch->invis_level);
				i = buf2;
			} else
				i = str_empty;
			break;

		case 'i':
			if (ch->incog_level) {
				snprintf(buf2, sizeof(buf2), "%d",
					 ch->incog_level);
				i = buf2;
			} else
				i = str_empty;
			break;
		}
		++str;
		while((*point = *i) != '\0')
			++point, ++i;
	}

	*point = '\0';
	send_to_char(buf, ch);

	if (d->dvdata->prefix[0] != '\0') {
		send_to_char(d->dvdata->prefix, ch);
		send_to_char(" ", ch);
	}
}

void
close_descriptor(DESCRIPTOR_DATA *dclose, int save_flags)
{
	DESCRIPTOR_DATA *d;

	if (!outbuf_empty(dclose))
		process_output(dclose, FALSE);

	if (dclose->snoop_by != NULL)
		write_to_buffer(dclose->snoop_by,
				"Your victim has left the game.\n\r", 0);

	for (d = descriptor_list; d != NULL; d = d->next)
		if (d->snoop_by == dclose)
			d->snoop_by = NULL;

	if (dclose->character != NULL) {
		CHAR_DATA *ch = dclose->original ? dclose->original : dclose->character;
		if (!IS_SET(save_flags, SAVE_F_NONE))
			char_save(ch, save_flags);
		log(LOG_INFO, "Closing link to %s.", ch->name);
		if (dclose->connected == CON_PLAYING) {
			act("$n has lost $s link.", ch, NULL, NULL, TO_ROOM);
			wiznet("Net death has claimed $N.", ch, NULL,
			       WIZ_LINKS, 0, 0);
			dclose->character->desc = NULL;
		} else
			char_nuke(ch);
	}

	if (dclose == descriptor_list)
		descriptor_list = descriptor_list->next;
	else {
		for (d = descriptor_list; d && d->next != dclose; d = d->next)
			;
		if (d != NULL)
			d->next = dclose->next;
		else
			log(LOG_BUG, "close_socket: dclose not found.");
	}

#if !defined( WIN32 )
	close(dclose->descriptor);
#else
	closesocket(dclose->descriptor);
#endif
	free_descriptor(dclose);
}

/*--------------------------------------------------------------------
 * semi-locals
 */

/*
 * Append onto an output buffer.
 */
void
write_to_buffer(DESCRIPTOR_DATA *d, const char *txt, size_t len)
{
	size_t size;
	bool noiac = (d->character != NULL &&
		      IS_SET(d->character->comm, COMM_NOIAC));

	/*
	 * Find length in case caller didn't.
	 */
	if (len == 0)
		len = strlen(txt);

	/*
	 * Snoop-o-rama.
	 */
	if (d->snoop_by)
		write_to_snoop(d, txt, len);

	/*
	 * Initial \n\r if needed.
	 */
	if (d->out_buf.top == 0
	&&  !d->fcommand
	&&  (!d->character || !IS_SET(d->character->comm, COMM_TELNET_GA))) {
		d->out_buf.buf[0]	= '\n';
		d->out_buf.buf[1]	= '\r';
		d->out_buf.top		= 2;
	}

	/*
	 * Adjust size in case of IACs (they will be doubled)
	 */
	size = len;
	if (!noiac) {
		size_t i;

		for (i = 0; i < len; i++) {
			if (CODEPAGE(d->codepage).to[(u_char) txt[i]] == IAC)
				size++;
		}
	}

	/* adjust output buffer size */
	if (!outbuf_adjust(&d->out_buf, size)) {
		close_descriptor(d, SAVE_F_NORMAL);
		return;
	}

	/* copy */
	while (len--) {
		u_char c;

		c = CODEPAGE(d->codepage).to[(u_char) *txt++];
		d->out_buf.buf[d->out_buf.top] = c;
		if (c == IAC) {
			if (noiac)
				d->out_buf.buf[d->out_buf.top] = IAC_REPL;
			else
				d->out_buf.buf[++d->out_buf.top] = IAC;
		}
		d->out_buf.top++;
	}
}

void
write_to_snoop(DESCRIPTOR_DATA *d, const char *txt, size_t len)
{
	if (len == 0)
		len = strlen(txt);

	/* adjust snoop buffer size */
	if (!outbuf_adjust(&d->snoop_buf, len)) {
		close_descriptor(d, SAVE_F_NORMAL);
		return;
	}

	/* copy */
	strnzncpy(d->snoop_buf.buf + d->snoop_buf.top, d->snoop_buf.size,
		  txt, len);
	d->snoop_buf.top += len;
}

/*
 * Lowest level output function.
 * Write a block of text to the file descriptor.
 * If this gives errors on very long blocks (like 'ofind all'),
 *   try lowering the max block size.
 */
bool
write_to_descriptor(int desc, const char *txt, size_t length)
{
	size_t	iStart;
	ssize_t	nWrite;
	size_t	nBlock;

	if (!length)
		length = strlen(txt);

	for (iStart = 0; iStart < length; iStart += nWrite) {
		nBlock = UMIN(length - iStart, 4096);
#if !defined( WIN32 )
		if ((nWrite = write(desc, txt + iStart, nBlock)) < 0) {
#else
		if ((nWrite = send(desc, txt + iStart, nBlock, 0)) < 0) {
#endif
			log(LOG_INFO, "write_to_descriptor: %s", strerror(errno));
			return FALSE;
		}
	}
	return TRUE;
}

void
charset_print(DESCRIPTOR_DATA *d)
{
	char buf[MAX_STRING_LENGTH];
	size_t i;

	write_to_buffer(d, "\n\r", 0);				// notrans
	for (i = 0; i < NCODEPAGES; i++) {
		snprintf(buf, sizeof(buf), "%s%d. %s",		// notrans
			 i ? " " : "", i+1, codepages[i].name);
		write_to_buffer(d, buf, 0);
	}
	write_to_buffer(d, "\n\rSelect your charset (non-russian players should choose translit): ", 0);
}

RUNGAME_FUN(_run_game, in_set, out_set, exc_set)
{
	DESCRIPTOR_DATA *d, *d_next;
	INFO_DESC *id;
	INFO_DESC *id_next;
	int maxdesc;
	static struct timeval null_time;

	/*
	 * Poll all active descriptors.
	 */
	FD_ZERO(in_set);
	FD_ZERO(out_set);
	FD_ZERO(exc_set);

	maxdesc = 0;
	add_fds(&control_sockets, in_set, &maxdesc);
	add_fds(&info_sockets, in_set, &maxdesc);

#if !defined (WIN32)
	FD_SET(fileno(rfin), in_set);
	maxdesc = UMAX(maxdesc, fileno(rfin));
#endif

	for (d = descriptor_list; d; d = d->next) {
		maxdesc = UMAX(maxdesc, d->descriptor);
		FD_SET(d->descriptor, in_set);
		FD_SET(d->descriptor, out_set);
		FD_SET(d->descriptor, exc_set);
	}

	for (id = id_list; id; id = id->next) {
		maxdesc = UMAX(maxdesc, id->fd);
		FD_SET(id->fd, in_set);
	}

	if (select(maxdesc+1, in_set, out_set, exc_set, &null_time) < 0) {
		log(LOG_INFO, "game_loop: select: %s", strerror(errno));
		return;
	}

#if !defined (WIN32)
	if (FD_ISSET(fileno(rfin), in_set))
		resolv_done();
#endif

	check_fds(&control_sockets, in_set, init_descriptor);
	check_fds(&info_sockets, in_set, info_newconn);

	for (id = id_list; id; id = id_next) {
		id_next = id->next;

		if (FD_ISSET(id->fd, in_set))
			info_process_cmd(id);
	}

	/*
	 * Kick out the freaky folks.
	 */
	for (d = descriptor_list; d; d = d_next) {
		d_next = d->next;
		if (FD_ISSET(d->descriptor, exc_set)) {
			FD_CLR(d->descriptor, in_set);
			FD_CLR(d->descriptor, out_set);
			outbuf_flush(d);
			close_descriptor(d, SAVE_F_NORMAL);
		}
	}

	_run_game_bottom(in_set, out_set, exc_set);
}

RUNGAME_FUN(_run_game_bottom, in_set, out_set, exc_set)
{
	DESCRIPTOR_DATA *d, *d_next;
	/*
	 * Process input.
	 */
	for (d = descriptor_list; d != NULL; d = d_next) {
		d_next		= d->next;
		d->fcommand	= FALSE;

		if (FD_ISSET(d->descriptor, in_set)) {
			if (d->character && !IS_NPC(d->character))
				PC(d->character)->idle_timer = 0;

			if (!read_from_descriptor(d)) {
				FD_CLR(d->descriptor, out_set);
				outbuf_flush(d);
				close_descriptor(d, SAVE_F_NORMAL);
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
			stop_idling(d);

			if (d->showstr_point)
				show_string(d, d->incomm);
			else if (d->pString)
				string_add(d->character, d->incomm);
			else if (d->connected == CON_PLAYING) {
				substitute_alias(d, d->incomm);
			} else
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

		if ((d->fcommand || !outbuf_empty(d))
		&&  FD_ISSET(d->descriptor, out_set)) {
			if (!process_output(d, TRUE))
				close_descriptor(d, SAVE_F_NORMAL);
		}
	}
}

/*--------------------------------------------------------------------
 * local functions
 */

#define GETINT(v, i) (*(int*) VARR_GET(v, i))

static void
add_fds(varr *v, fd_set *in_set, int *maxdesc)
{
	size_t i;

	for (i = 0; i < v->nused; i++) {
		int fd = GETINT(v, i);
		FD_SET(fd, in_set);
		if (*maxdesc < fd) *maxdesc = fd;
	}
}

static void
check_fds(varr *v, fd_set *in_set, void (*new_conn_cb)(int))
{
	size_t i;

	for (i = 0; i < v->nused; i++) {
		int fd = GETINT(v, i);
		if (FD_ISSET(fd, in_set))
			new_conn_cb(fd);
	}
}

#if !defined(FNDELAY)
#define FNDELAY O_NDELAY
#endif

static void
init_descriptor(int control)
{
	DESCRIPTOR_DATA *dnew;
	struct sockaddr_in sock;
	int desc;
	int size;
	HELP_DATA *greeting;

	size = sizeof(sock);
	getsockname(control, (struct sockaddr *) &sock, &size);
	if ((desc = accept(control, (struct sockaddr *) &sock, &size)) < 0) {
		log(LOG_INFO, "init_descriptor: accept: %s", strerror(errno));
		return;
	}

#if !defined (WIN32)
	if (fcntl(desc, F_SETFL, FNDELAY) < 0) {
		log(LOG_INFO, "init_descriptor: fcntl: FNDELAY: %s",
			   strerror(errno));
		return;
	}
#endif

	/*
	 * Cons a new descriptor.
	 */
	dnew = new_descriptor(desc);

	size = sizeof(sock);
	if (getpeername(desc, (struct sockaddr *) &sock, &size) < 0) {
		log(LOG_INFO, "init_descriptor: getpeername: %s",
			   strerror(errno));
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
		dnew->host = str_dup(
		    from ? from->h_name : "unknown");		// notrans
	}
#endif

	log(LOG_INFO, "sock.sinaddr: %s", inet_ntoa(sock.sin_addr));

	dnew->next		= descriptor_list;
	descriptor_list		= dnew;

	/*
	 * Send the greeting.
	 */
	if ((greeting = help_lookup(1, "GREETING"))) {		// notrans
		char buf[MAX_STRING_LENGTH];
		parse_colors(mlstr_mval(&greeting->text), buf, sizeof(buf),
			     FORMAT_DUMB);
		write_to_buffer(dnew, buf + (buf[0] == '.'), 0);
	}
	charset_print(dnew);
}

static bool
read_from_descriptor(DESCRIPTOR_DATA *d)
{
	size_t iOld;
	size_t iStart;
	u_char *p, *q;

	/*
	 * Hold horses if pending command already
	 */
	if (d->incomm[0] != '\0')
		return TRUE;

	/* Check for overflow. */
	iOld = iStart = strlen(d->inbuf);
	if (iStart >= sizeof(d->inbuf) - 10) {
		log(LOG_INFO, "%s input overflow!", d->host);
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
			log(LOG_INFO, "EOF encountered on read.");
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
			log(LOG_INFO, "read_from_descriptor: %s", strerror(errno));
			return FALSE;
		}
	}

	d->inbuf[iStart] = '\0';
	if (iOld == iStart)
		return TRUE;

	for (p = d->inbuf+iOld; *p;) {
		u_char *r;

		if (*p != IAC
		||  (d->character != NULL &&
		     IS_SET(d->character->comm, COMM_NOTELNET))) {
			p++;
			continue;
		}

		if (d->wait_for_se)
			goto wse;

		switch (p[1]) {
		case DONT:
		case DO:
		case WONT:
		case WILL:
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
			memmove(p, p+1, strlen(p));
			p++;
			continue;
			/* NOTREACHED */

		default:
			q = p+2;
			break;
		}
		if ((r = strchr(p, '\0')) < q)
			q = r;
		memmove(p, q, strlen(q)+1);
	}

	return TRUE;
}

/*
 * Transfer one line from input buffer to input line.
 */
static void
read_from_buffer(DESCRIPTOR_DATA *d)
{
	int i, j, k;
	bool repeat;

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
		else if ((u_char) d->inbuf[i] >= ' ')
			d->incomm[k++] =
			    CODEPAGE(d->codepage).from[(u_char) d->inbuf[i]];
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
	repeat = !str_cmp(d->incomm, "!");
	if (k > 1 || repeat) {
		if (!repeat && strcmp(d->incomm, d->inlast))
			d->repeat = 0;
		else {
			CHAR_DATA *ch = d->original ? d->original :
						      d->character;
			if (ch && ++d->repeat >= 100) {
				char buf[MAX_STRING_LENGTH];

				log(LOG_INFO, "%s input spamming!", d->host);
				snprintf(buf, sizeof(buf),
					 "Inlast:[%s] Incomm:[%s]!", // notrans
					 d->inlast, d->incomm);

				wiznet("SPAM SPAM SPAM $N spamming, and OUT!",
					ch, NULL, WIZ_SPAM, 0, ch->level);
				wiznet("[$N]'s $t!",
					ch, buf, WIZ_SPAM, 0, ch->level);

				write_to_descriptor(d->descriptor, "\n\r*** PUT A LID ON IT!!! ***\n\r", 0);
				d->repeat = 0;
				if (d->showstr_point) {
					if (d->showstr_head) {
						free_string(d->showstr_head);
						d->showstr_head = NULL;
					}
					d->showstr_point = NULL;
				}
				if (d->pString) {
					free_string(*d->pString);
					*d->pString = d->backup;
					d->pString = NULL;
				}
				strnzcpy(d->incomm, sizeof(d->incomm), "quit");
			}
		}
	}

	/*
	 * Do '!' substitution.
	 */
	if (repeat)
		strnzcpy(d->incomm, sizeof(d->incomm), d->inlast);
	else
		strnzcpy(d->inlast, sizeof(d->inlast), d->incomm);

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
static void
battle_prompt(CHAR_DATA *ch, CHAR_DATA *victim)
{
	int percent;
	const char* msg;

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

	act_puts("$N $t", ch, msg, victim, TO_CHAR, POS_DEAD);	// notrans
}

/*
 * Some specials added by KIO
 */
static bool
process_output(DESCRIPTOR_DATA *d, bool fPrompt)
{
	bool ga = FALSE;
	bool retval;
	DESCRIPTOR_DATA *snoopy;
	CHAR_DATA *ch = d->character;

	/*
	 * Bust a prompt.
	 */
	if (!merc_down && ch) {
		if (d->showstr_point) {
			act_puts("[Hit Return to continue]",
				 ch, NULL, NULL, TO_CHAR | ACT_NOLF, POS_DEAD);
			ga = TRUE;
		} else if (fPrompt && d->connected == CON_PLAYING) {
			if (d->pString) {
				send_to_char("  > ", ch);	// notrans
				ga = TRUE;
			} else {
				CHAR_DATA *victim;

				/* battle prompt */
				if ((victim = ch->fighting) != NULL
				&&  can_see(ch,victim))
					battle_prompt(ch, victim);

				if (!IS_SET(ch->comm, COMM_COMPACT))
					send_to_char("\n", ch);

				if (IS_SET(ch->comm, COMM_PROMPT))
					bust_a_prompt(d);
				ga = TRUE;
			}
		}

		if (ga && !IS_SET(ch->comm, COMM_TELNET_GA))
			ga = FALSE;
	}

	/*
	 * Short-circuit if nothing to write.
	 */
	if (outbuf_empty(d))
		return TRUE;

	/*
	 * Snoop-o-rama
	 */
	if ((snoopy = d->snoop_by) != NULL && d->snoop_buf.top > 0) {
		char buf[MAX_STRING_LENGTH];

		snprintf(buf, sizeof(buf), "\n\r===> %s\n\r",	// notrans
			 d->character ? d->character->name : str_empty);
		write_to_buffer(snoopy, buf, 0);
		write_to_buffer(snoopy, d->snoop_buf.buf, d->snoop_buf.top);
		write_to_buffer(snoopy, "\n\r", 0);		// notrans
	}

	/*
	 * OS-dependent output.
	 */
	if (d->out_buf.top > 0) {
		if (!write_to_descriptor(d->descriptor,
					 d->out_buf.buf, d->out_buf.top)) {
			retval = FALSE;
			goto bail_out;
		}
	}

	if (ga) {
		if (!write_to_descriptor(d->descriptor, go_ahead_str, 0)) {
			retval = FALSE;
			goto bail_out;
		}
	}

	retval = TRUE;

bail_out:
	outbuf_flush(d);
	return retval;
}

static void
stop_idling(DESCRIPTOR_DATA *d)
{
	CHAR_DATA *ch = d->character;
	PC_DATA *pc;
	ROOM_INDEX_DATA *to_room;

	if (ch == NULL
	||  IS_NPC(ch)
	||  d->connected != CON_PLAYING
	||  !(pc = PC(ch))->was_in_vnum
	||  ch->in_room->vnum != ROOM_VNUM_LIMBO)
		return;

	to_room = get_room_index(pc->was_in_vnum);
	if (to_room == NULL)
		to_room = get_room_index(ROOM_VNUM_TEMPLE);

	pc->idle_timer = 0;
	pc->was_in_vnum = 0;

	char_from_room(ch);
	act("$N has returned from the void.",
	    to_room->people, NULL, ch, TO_ALL);
	char_to_room(ch, to_room);

	if (pc->pet) {
		char_from_room(pc->pet);
		act("$N has returned from the void.",
		    to_room->people, NULL, pc->pet, TO_ALL);
		char_to_room(pc->pet, to_room);
	}
}

/* string pager */
static void
show_string(DESCRIPTOR_DATA *d, const char *input)
{
	char buffer[4*MAX_STRING_LENGTH];
	char buf[MAX_INPUT_LENGTH];
	char *scan;
	int lines = 0;

	one_argument(input, buf, sizeof(buf));
	if (buf[0] != '\0') {
		if (d->showstr_head) {
			free_string(d->showstr_head);
			d->showstr_head = NULL;
		}
		d->showstr_point  = NULL;
		return;
	}

	for (scan = buffer; scan < buffer + sizeof(buffer) - 2;
						scan++, d->showstr_point++) {
		/*
		 * simple copy if not eos and not eol
		 */
		if ((*scan = *d->showstr_point) && (*scan) != '\n') 
			continue;

		/*
		 * bamf out buffer if we reached eos or d->dvdata->pagelen limit
		 */
		if (!*scan
		||  (d->dvdata->pagelen > 0 && ++lines >= d->dvdata->pagelen)) {
			const char *chk;

			if (*scan)
				*++scan = '\0';
			send_to_char(buffer, d->character);

			for (chk = d->showstr_point; isspace(*chk); chk++)
				;
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

static bool
outbuf_empty(DESCRIPTOR_DATA *d)
{
	return (d->out_buf.top == 0);
}

static
void outbuf_flush(DESCRIPTOR_DATA *d)
{
	d->out_buf.top = 0;
	d->snoop_buf.top = 0;
}

/*
 * Expand the buffer as needed to hold more 'len' characters.
 */
static bool
outbuf_adjust(outbuf_t *o, size_t len)
{
	char *newbuf;
	size_t newsize;

	len += o->top;
	if (len < o->size)
		return TRUE;

	if ((newsize = o->size) == 0)
		newsize = 1024;

	while (newsize < len) {
		if (newsize > 32768) {
			log(LOG_INFO, "outbuf_adjust: buffer overflow, closing");
			return FALSE;
		}

		newsize <<= 1;
	}

	if ((newbuf = realloc(o->buf, newsize)) == NULL) {
		log(LOG_INFO, "outbuf_adjust: not enough memory to expand output buffer");
		return FALSE;
	}

	o->buf = newbuf;
	o->size  = newsize;
	return TRUE;
}
