/*
 * $Id: mudftp.c,v 1.4 2003-09-30 00:31:29 fjoe Exp $
 *
 * MUDftp module
 * (c) Copyright 1997, 1998 Erwin S. Andreasen and Oliver Jowett
 * This code may be freely redistributable, as long as this header is left
 * intact.
 *
 * Thanks to:
 * - Jessica Boyd for the ROM version
 * - Dominic J. Eidson for the ROT version
 * - George Greer for the Circle version
 */

#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if defined(SUNOS) || defined(SVR4) || defined(LINUX)
#include <crypt.h>
#endif

#include <merc.h>
#include <lang.h>
#include <colors.h>

#include <sog.h>

#include "comm.h"

static void mudftp_reply(DESCRIPTOR_DATA *d, const char *fmt, ...);
static void finish_file(DESCRIPTOR_DATA *d);
static void mudftp_notify(const char *fmt, ...);
static CHAR_DATA *find_mudftp_char(DESCRIPTOR_DATA *m);
static const char *ftp_checksum(const char *str);
static int line_count(const char *s);

static DECLARE_SERVICE_FUN(cmd_push);
static DECLARE_SERVICE_FUN(cmd_stop);
static DECLARE_SERVICE_FUN(cmd_put);
static DECLARE_SERVICE_FUN(cmd_noop);
static DECLARE_SERVICE_FUN(cmd_quit);

static service_cmd_t info_cmds[] =
{
	{ "PUSH",	cmd_push,		},
	{ "STOP",	cmd_stop,		},
	{ "PUT",	cmd_put			},
	{ "GET",	service_unimpl		},
	{ "NOOP",	cmd_noop		},
	{ "QUIT",	cmd_quit		},
	{ NULL,		service_unimpl		},
};

void
handle_mudftp(DESCRIPTOR_DATA *d)
{
	if (check_ban(d, BCL_MUDFTP) == BAN_DENY)
		return;

	switch (d->connected) {
	case CON_MUDFTP_AUTH: {
		/*
		 * Authorization line: <username> <password>
		 */
		char name[MAX_INPUT_LENGTH];
		DESCRIPTOR_DATA *m, *m_next;
		CHAR_DATA *ch = NULL;
		const char *argument;

		argument = one_argument(d->incomm, name, sizeof(name));
		/*
		 * Find the descriptor of the connected character
		 */
		for (m = descriptor_list; m != NULL; m = m->next) {
			if (m != d
			&&  m->connected == CON_PLAYING) {
				ch = m->original ? m->original : m->character;
				if (!str_cmp(ch->name, name))
					break;
			}
		}

		if (ch == NULL
		||  !!strcmp(crypt(argument, PC(ch)->pwd), PC(ch)->pwd)) {
			mudftp_notify(
			    "mudFTP authorization for '%s' failed", name);
			mudftp_reply(d, "FAILED");
			close_descriptor(d, 0);
			return;
		}

		/* Search for old mudftp connections */
		for (m = descriptor_list; m != NULL; m = m_next) {
			m_next = m->next;

			if (m != d
			&&  (m->connected == CON_MUDFTP_COMMAND ||
			     m->connected == CON_MUDFTP_DATA)
			&&  !str_cmp(m->mftp_username, name))
				close_descriptor(m, 0);
		}

		d->mftp_username = str_dup(name);
		mudftp_reply(d, "OK mudFTP 2.0 ready");
		d->connected = CON_MUDFTP_COMMAND;
		mudftp_notify("mudFTP authorization for %s@%s", name, d->host);
		break;
	}
	case CON_MUDFTP_COMMAND:
		if (!!str_cmp(d->incomm, "NOOP")) {
			mudftp_notify(
			    "mudftp command: '%s' from %s@%s",
			    d->incomm, d->mftp_username, d->host);
		}
		handle_service(d, info_cmds);
		break;
	case CON_MUDFTP_DATA:
		do {
			/* Lines that overflow the buffer are silently lost */
			if (strlen(d->mftp_data) + strlen(d->incomm) + 2 <
			    MAX_STRING_LENGTH) {
				const char *p = d->mftp_data;
				d->mftp_data = str_printf("%s%s",
				    d->mftp_data, d->incomm);
				free_string(p);
			}

			/* All of the file received? */
			if (--d->mftp_lines_left == 0)
				finish_file(d);

			d->incomm[0] = '\0';
			read_from_buffer(d);
		} while (d->incomm[0] != '\0');
		break;
	default:
		printlog(LOG_INFO, "handle_mudftp: invalid state %d", d->connected);
		close_descriptor(d, 0);
		break;
	}
}

/*
 * Try to push a string to this desc. false if we can't
 */
bool
mudftp_push(DESCRIPTOR_DATA *d)
{
	CHAR_DATA *ch = d->original ? d->original : d->character;
	DESCRIPTOR_DATA *m;

	for (m = descriptor_list; m != NULL; m = m->next) {
		const char *text;

		if (m->connected != CON_MUDFTP_COMMAND
		||  m->mftp_mode != MUDFTP_PUSH
		||  !!str_cmp(m->mftp_username, ch->name))
			continue;

		text = *d->pString;
		mudftp_reply(m, "SENDING tmp/%lx %d %s",
		    (unsigned long) d->pString,
		    line_count(text), ftp_checksum(text));
		m->codepage = d->codepage;
		write_to_buffer(m, text, 0);
		m->mftp_mode = MUDFTP_PUSH_WAIT;
		return TRUE;
	}

	return FALSE;
}

static
SERVICE_FUN(cmd_push)
{
	if (d->mftp_mode != MUDFTP_NORMAL) {
		mudftp_reply(d, "ERROR Already in push mode");
		return;
	}

	d->mftp_mode = MUDFTP_PUSH;
	mudftp_reply(d, "OK Pushing you data as it arrives");
}

static
SERVICE_FUN(cmd_stop)
{
	CHAR_DATA *ch = find_mudftp_char(d);

	if (d->mftp_mode == MUDFTP_PUSH_WAIT)
		d->mftp_mode = MUDFTP_PUSH;

	if ((ch = find_mudftp_char(d)) == NULL) {
		mudftp_reply(d, "FAILED");
		return;
	}

	free_string(d->mftp_data);
	d->mftp_data = str_empty;
	free_string(d->mftp_filename);
	d->mftp_filename = NULL;

	string_add_exit(ch, FALSE);	/* Abort editing */
	mudftp_reply(d, "OK");
}

static
SERVICE_FUN(cmd_put)
{
	char arg[MAX_INPUT_LENGTH];

	if (d->mftp_mode == MUDFTP_PUSH_WAIT)
		d->mftp_mode = MUDFTP_PUSH;

	argument = one_argument(argument, arg, sizeof(arg));
	if (!is_number(argument) || atoi(argument) < 0) {
		mudftp_reply(d, "ERROR Missing filename or number of lines");
		return;
	}

	free_string(d->mftp_filename);
	d->mftp_filename = str_dup(arg);
	free_string(d->mftp_data);
	d->mftp_data = str_empty;
	d->mftp_lines_left = atoi(argument);
	if (d->mftp_lines_left)
		d->connected = CON_MUDFTP_DATA;
	else
		finish_file(d);
}

static
SERVICE_FUN(cmd_noop)
{
	mudftp_reply(d, "OK");
}

static
SERVICE_FUN(cmd_quit)
{
	close_descriptor(d, 0);
}

static void
mudftp_reply(DESCRIPTOR_DATA *d, const char *fmt, ...)
{
	char buf[MAX_STRING_LENGTH];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	printlog(LOG_INFO, "mudftp_reply: [%s]", buf);
	write_to_buffer(d, buf, 0);
	write_to_buffer(d, "\n", 0);
}

static void
finish_file(DESCRIPTOR_DATA *d)
{
	unsigned long temp_file;

	mudftp_notify("Transfer of %s done from %s@%s",
	    d->mftp_filename, d->mftp_username, d->host);

	d->connected = CON_MUDFTP_COMMAND;
	/* Put the file in its rightful spot */
	if (1 == sscanf(d->mftp_filename, "tmp/%lx", &temp_file)) {
		CHAR_DATA *ch = find_mudftp_char(d);

		if (ch && ((unsigned long) ch->desc->pString) == temp_file) {
			mudftp_reply(d, "OK %s", ftp_checksum(d->mftp_data));

			free_string(*ch->desc->pString);
			*ch->desc->pString = str_qdup(d->mftp_data);
			free_string(d->mftp_data);
			d->mftp_data = str_empty;

			string_add_exit(ch, TRUE); /* Finish editing */
			return;
		}
	}

	mudftp_reply(d, "FAILED Something went wrong");

	free_string(d->mftp_data);
	d->mftp_data = str_empty;
	free_string(d->mftp_filename);
	d->mftp_filename = NULL;
}

static CHAR_DATA *
find_mudftp_char(DESCRIPTOR_DATA *m)
{
	DESCRIPTOR_DATA *d;

	for (d = descriptor_list; d != NULL; d = d->next) {
		if (d->connected == CON_PLAYING
		&&  !str_cmp(d->character->name, m->mftp_username)
		&&  d->pString)
			return d->character;
	}

	return NULL;
}

static void
mudftp_notify(const char *fmt, ... )
{
	va_list va;
	char buf[MAX_STRING_LENGTH];

	va_start(va, fmt);
	vsnprintf(buf, sizeof(buf), fmt, va);
	va_end(va);

	wiznet("$t.", NULL, buf, WIZ_LINKS, 0, 0);
	printlog(LOG_INFO, "%s", buf);
}

/*
 * This algorithm is derived from the one supposedly used in Perl
 */
static const char *
ftp_checksum(const char *str)
{
	static char buf[10];
	unsigned hash = 0;

	while (*str)
		hash = hash * 33U + *str++;

	snprintf(buf, sizeof(buf), "%08x", hash);
	return buf;
}

static int
line_count(const char *s)
{
	int count = 0;

	while (*s) {
		if (*s++ == '\n')
			count++;
	}

	return count;
}
