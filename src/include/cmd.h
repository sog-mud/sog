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
 * $Id: cmd.h,v 1.22 2002-11-21 09:33:44 fjoe Exp $
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
#define	CMD_KEEP_HIDE		(A)
#define CMD_NOORDER		(B)
#define CMD_CHARMED_OK		(C)
#define CMD_HIDDEN		(D)
#define CMD_DISABLED		(E)
#define CMD_FROZEN_OK		(F)
#define CMD_STRICT_MATCH	(G)

/*
 * Structure for a command in the command lookup table.
 */
struct cmd_t
{
	const char *	name;		/* cmd name */
	const char *	dofun_name;	/* name of do_function */
	const char *	aliases;	/* cmd aliases */
	int		min_pos;	/* min position for cmd */
	int		min_level;	/* min level for cmd */
	int		cmd_log;	/* cmd logging type */
	int		cmd_flags;	/* cmd flags */
	int		cmd_mod;	/* module where cmd is implemented */
	DO_FUN *	do_fun;
};

extern varr commands;
extern varr_info_t c_info_commands;

#define cmd_lookup(name)	((cmd_t *) c_strkey_lookup(&commands, (name)))
#define cmd_search(name)	((cmd_t *) c_strkey_search(&commands, (name)))

void	cmd_load	(cmd_t *cmd, int cmd_mod, module_t *m);
void	cmd_unload	(cmd_t *cmd, int cmd_mod);

void	dofun		(const char *name, CHAR_DATA *ch, const char *argument);

#endif
