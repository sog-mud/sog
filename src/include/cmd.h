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
 * $Id: cmd.h,v 1.7.2.5 2004-02-22 15:50:26 fjoe Exp $
 */

#ifndef _CMD_H_
#define _CMD_H_

/*
 * cmd logging types
 */
#define LOG_NORMAL	0
#define LOG_ALWAYS	1
#define LOG_NEVER	2

/*
 * cmd flags
 */
#define	CMD_KEEP_HIDE		(A)	/* doesn't strip hidden state */
#define CMD_NOORDER		(B)	/* can't be ordered */
#define CMD_HARMLESS		(C)	/* ok when charmed or wait > 0 */
					/* + doesn't strip hidden state */
#define CMD_HIDDEN		(D)	/* is not show in commands list */
#define CMD_DISABLED		(E)	/* disabled */
#define CMD_FROZEN_OK		(F)	/* ok when frozen */
#define CMD_CLAN		(G)
#define CMD_STRIP_HIDE		(H)	/* always strip hidden state */


/*
 * cmd classes
 */
enum {
	CC_ORDINARY,
	CC_CORE,
	CC_OLC
};

/*
 * Structure for a command in the command lookup table.
 */
struct cmd_t
{
	const char *	name;
	const char *	aliases;
	const char *	dofun_name;
	const char *	sn;
	int		min_pos;
	int		min_level;
	int		cmd_log;
	int		cmd_flags;
	int		cmd_class;
	DO_FUN *	do_fun;
};

cmd_t *	cmd_new		(void);
void	cmd_free	(cmd_t*);

cmd_t *	cmd_lookup	(const char *name);
void	cmd_foreach	(int cmd_class, void *arg,
			void (*callback)(cmd_t *cmd, void* arg));

void	cmd_load	(cmd_t *cmd, void *arg);
void	cmd_unload	(cmd_t *cmd, void *arg);

void	dofun		(const char *name, CHAR_DATA *ch, const char *fmt, ...);

extern varr commands;

#endif

