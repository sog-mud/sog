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
 * $Id: info.c,v 1.28 2001-08-13 18:23:40 fjoe Exp $
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

#if defined(SUNOS) || defined(SVR4) || defined(LINUX)
#	include <crypt.h>
#endif

#include <merc.h>
#include <lang.h>
#include <colors.h>

#include <handler.h>

#include "info.h"

INFO_DESC *	id_list;
int		top_id;
char		buf[2 * MAX_STRING_LENGTH];

static INFO_DESC *	id_free_list;

static INFO_DESC *	info_desc_new(int fd);
static void		info_desc_free(INFO_DESC *id);

typedef void CMD_FUN (const char *argument);
typedef struct {
	const char	*name;
	CMD_FUN	*fun;
} infocmd_t;
#define DECLARE_CMD_FUN(fun)	static CMD_FUN fun
#define CMD_FUN(fun)						\
	static void fun(const char *argument __attribute__((unused)))

DECLARE_CMD_FUN(cmd_who);
DECLARE_CMD_FUN(cmd_auth);
DECLARE_CMD_FUN(cmd_help);
DECLARE_CMD_FUN(cmd_show);
DECLARE_CMD_FUN(cmd_setf);
DECLARE_CMD_FUN(cmd_dumb);

/* Currently supported commands:
 * WHO <fmt>				- outputs list of visible players
 * AUTH <plr> <pwd>			- authentificate a player
 * HELP <fmt> <lang> <lev> <topic>	- show help
 * SHOW	<fmt> <plr>				- dump player's information
 * SETF <plr> <pwd> <flags>		- set www_show_flags equal to <flags>
 */

infocmd_t info_cmds[] =
{
	{ "WHO",	cmd_who			},
	{ "AUTH",	cmd_auth		},
	{ "HELP",	cmd_help		},
	{ "SHOW",	cmd_show		},
	{ "SETF",	cmd_setf		},
	{ NULL,		cmd_dumb		}
};

void info_newconn(int infofd)
{
	int fd;
	struct sockaddr_in sock;
	int size = sizeof(sock);
	INFO_DESC *id;
	size_t i;

	getsockname(infofd, (struct sockaddr*) &sock, &size);
	if ((fd = accept(infofd, (struct sockaddr*) &sock, &size)) < 0) {
		log(LOG_INFO, "info_newconn: accept: %s", strerror(errno));
		return;
	}

	if (getpeername(fd, (struct sockaddr *) &sock, &size) < 0) {
		log(LOG_INFO, "info_newconn: getpeername: %s", strerror(errno));
#ifdef WIN32
		closesocket(fd);
#else
		close(fd);
#endif
		return;
	}

	log(LOG_INFO, "info_newconn: sock.sin_addr: %s", inet_ntoa(sock.sin_addr));

	for (i = 0; i < info_trusted.nused; i++) {
		struct in_addr* in_addr = VARR_GET(&info_trusted, i);
		if (!memcmp(in_addr, &sock.sin_addr, sizeof(struct in_addr)))
			break;
	}

	if (i >= info_trusted.nused) {
		log(LOG_INFO, "info_newconn: incoming connection refused");
#ifdef WIN32
		closesocket(fd);
#else
		close(fd);
#endif
		return;
	}

#if !defined (WIN32)
	if (fcntl(fd, F_SETFL, FNDELAY) < 0) {
		log(LOG_INFO, "info_newconn: fcntl: FNDELAY: %s", strerror(errno));
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
	char cmd_name[10];
	const char *p;
	char *q;
        int nread;
	infocmd_t *icmd;

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
		log(LOG_INFO, "info_input: read: %s", strerror(errno));
		info_desc_free(id);
		return;
	}
	buf[nread] = '\0';
	q = strchr(buf, '\n');
	if (q)
		*q = '\0';

	p = one_argument(buf, cmd_name, sizeof(cmd_name));

	for (icmd = info_cmds; icmd->name; icmd++)
		if (!str_cmp(icmd->name, cmd_name))
			break;

	if (icmd->fun)
		icmd->fun(p);

#if !defined (WIN32)
		write(id->fd, buf, strlen(buf));
#else
		send(id->fd, buf, strlen(buf), 0);
#endif
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
			log(LOG_INFO, "info_desc_free: descriptor not found");
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

CMD_FUN(cmd_who)
{
	BUFFER *output;
	DESCRIPTOR_DATA *d;
	int format;
	char arg[MAX_INPUT_LENGTH];

	one_argument(argument, arg, sizeof(arg));

	output = buf_new(0);

	buf_printf(output, BUF_END, "%d\n", top_player);

	for (d = descriptor_list; d; d = d->next) {
		CHAR_DATA *wch;

		wch = d->original ? d->original : d->character;
		if (!wch
		|| d->connected != CON_PLAYING
		|| wch->invis_level
		|| wch->incog_level
		|| HAS_INVIS(wch, ID_ALL_INVIS)
		|| is_affected(wch, "vampire"))
			continue;

		do_who_raw(NULL, wch, output);
	}
	format = format_lookup(arg);
	parse_colors(buf_string(output), buf, sizeof(buf), format);
	buf_free(output);
}

CMD_FUN(cmd_auth)
{
	char arg1[MAX_INPUT_LENGTH],arg2[MAX_INPUT_LENGTH];
	CHAR_DATA *ch;

	argument = first_arg(argument, arg1, sizeof(arg1), FALSE);
	argument = first_arg(argument, arg2, sizeof(arg2), FALSE);

	ch = char_load(arg1, LOAD_F_NOCREATE);
	if (!ch || strcmp(crypt(arg2, ch->name), PC(ch)->pwd)) {
		strncpy(buf, "AUTH FAILED.\n", sizeof(buf));
		if (ch)
			char_nuke(ch);
		return;
	}
	strncpy(buf, "AUTH OK.\n", sizeof(buf));
	char_nuke(ch);
}

CMD_FUN(cmd_help)
{
	BUFFER *output;
	int format, lev;
	char arg[MAX_INPUT_LENGTH];
	lang_t *l;
	size_t lang;

	argument = one_argument(argument, arg, sizeof(arg));
	format = format_lookup(arg);

	argument = one_argument(argument, arg, sizeof(arg));
	if ((l = lang_lookup(arg)) == NULL)
		lang = 0;
	else
		lang = varr_index(&langs, l);

	argument = one_argument(argument, arg, sizeof(arg));
	lev = atoi(arg);
	
	output = buf_new(0);
	help_show_raw(lev, lang, output, argument);
	parse_colors(buf_string(output), buf, sizeof(buf), format);
	buf_free(output);
}

CMD_FUN(cmd_show)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *ch;
	BUFFER *output;
	int format;

	argument = one_argument(argument, arg, sizeof(arg));
	format = format_lookup(arg);

	argument = first_arg(argument, arg, sizeof(arg), FALSE);
	ch = char_load(arg, LOAD_F_NOCREATE);
	if (!ch) {
		strncpy(buf, "This character has not born yet.\n", sizeof(buf));
		return;
	}

	buf[0] = '\0';

	output = buf_new(0);
	buf_printf(output, BUF_END, "%s\n%s\n%s %s\n%d\n%s\n%s\n%s\n%s\n%d\n%d %d\n%d\n%s\n%s\n%d\n%s\n",
		IS_SET(PC(ch)->www_show_flags, WSHOW_RACE) ? PC(ch)->race : "Unknown",
		IS_SET(PC(ch)->www_show_flags, WSHOW_CLASS) ? ch->class : "Unknown",
		(IS_SET(PC(ch)->www_show_flags, WSHOW_CLAN) && !IS_NULLSTR(ch->clan)) ? ch->clan : "Unknown",
		(IS_SET(PC(ch)->www_show_flags, WSHOW_CLAN) && !IS_NULLSTR(ch->clan)) ? flag_string(clan_status_table, PC(ch)->clan_status) : "",
		IS_SET(PC(ch)->www_show_flags, WSHOW_LEVEL) ? ch->level : -1,
		IS_SET(PC(ch)->www_show_flags, WSHOW_ALIGN) ? flag_string(align_names, NALIGN(ch)) : "Unknown",
		IS_SET(PC(ch)->www_show_flags, WSHOW_ETHOS) ? flag_string(ethos_table, ch->ethos) : "Unknown",
		IS_SET(PC(ch)->www_show_flags, WSHOW_SEX) ? mlstr_mval(&ch->gender) : "Unknown",
		IS_SET(PC(ch)->www_show_flags, WSHOW_SLANG) ? flag_string(slang_table, ch->slang) : "Unknown",
		IS_SET(PC(ch)->www_show_flags, WSHOW_DEATHS) ? PC(ch)->death : -1,
		IS_SET(PC(ch)->www_show_flags, WSHOW_KILLS) ? PC(ch)->has_killed : -1,
		IS_SET(PC(ch)->www_show_flags, WSHOW_KILLS) ? PC(ch)->anti_killed : -1,
		IS_SET(PC(ch)->www_show_flags, WSHOW_PCKILLS) ? PC(ch)->pc_killed : -1,
		IS_SET(PC(ch)->www_show_flags, WSHOW_TITLE) ? PC(ch)->title : "",
		IS_SET(PC(ch)->www_show_flags, WSHOW_HOMETOWN) ? hometown_name(PC(ch)->hometown) : "Unknown",
		IS_SET(PC(ch)->www_show_flags, WSHOW_AGE) ? get_age(ch) : -1,
		IS_WANTED(ch) ? "WANTED" : "");

	parse_colors(buf_string(output), buf, sizeof(buf), format);
	buf_free(output);
	char_nuke(ch);
}

CMD_FUN(cmd_setf)
{
	char arg1[MAX_INPUT_LENGTH],arg2[MAX_INPUT_LENGTH];
	CHAR_DATA *ch, *vch;

	argument = first_arg(argument, arg1, sizeof(arg1), FALSE);
	argument = first_arg(argument, arg2, sizeof(arg2), FALSE);

	ch = char_load(arg1, LOAD_F_NOCREATE);
	if (!ch || strcmp(crypt(arg2, ch->name), PC(ch)->pwd)) {
		strncpy(buf, "AUTH FAILED.\n", sizeof(buf));
		if (ch)
			char_nuke(ch);
		return;
	}

	for (vch = char_list; vch && !IS_NPC(vch); vch = vch->next)
		if (!str_cmp(ch->name, vch->name)) {
			strncpy(buf, "CHAR LOGGED IN.\n", sizeof(buf));
			char_nuke(ch);
			return;
		}
	
	PC(ch)->www_show_flags = flag_value(www_flags_table, argument);
	char_save(ch, SAVE_F_PSCAN);
	strncpy(buf, "FLAGS SET: ", sizeof(buf));
	snprintf(buf, sizeof(buf), "%s%s\n", buf,
		flag_string(www_flags_table, PC(ch)->www_show_flags));
	char_nuke(ch);
}

CMD_FUN(cmd_dumb)
{
	strncpy(buf, "ERROR: unsupported or illegal function.\n", sizeof(buf));
}
