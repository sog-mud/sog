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
 * $Id: clan.h,v 1.5 1998-10-06 13:18:25 fjoe Exp $
 */

#ifndef _CLAN_H_
#define _CLAN_H_

/*----------------------------------------------------------------------
 * clan stuff (clan.c)
 */

#define CLAN_NONE 	 0
#define CLAN_RULER	 1
#define CLAN_INVADER	 2
#define CLAN_CHAOS	 3
#define CLAN_SHALAFI	 4
#define CLAN_BATTLE	 5
#define CLAN_KNIGHT	 6
#define CLAN_LIONS	 7
#define CLAN_HUNTER	 8
#define CLAN_CONFEDERACY 9

/* Clan status */
#define CLAN_LEADER	2
#define CLAN_SECOND	1
#define CLAN_COMMON	0

/*
 * Clan structure
 */
struct clan_data
{
	const char *	name;		/* clan name */
	const char *	file_name;	/* file name */

	int	 	recall_vnum;	/* recall room vnum */
	const char *	msg_prays;	/* what to print in clanrecall */
	const char *	msg_vanishes;

	varr		skills;		/* clan skills */

	flag_t		flags;		/* clan flags */

	int	 	obj_vnum;	/* unused (for a while) */
	OBJ_DATA *	obj_ptr;
};

/* clan flags */
#define CLAN_HIDDEN	(A)		/* clan will not appear in who */
#define CLAN_CHANGED	(Z)

CLAN_DATA *	clan_new	(void);
void		clan_free	(CLAN_DATA*);
int		cn_lookup	(const char* name);
const char*	clan_name	(int cn);

extern varr	clans;

#define CLAN(cn)		((CLAN_DATA*) VARR_GET(&clans, cn))
#define clan_lookup(cn)		((CLAN_DATA*) varr_get(&clans, cn))

struct clan_skill {
	int	sn;
	int	level;
};

#define clan_skill_lookup(clan, sn) \
	((CLAN_SKILL*) varr_bsearch(&clan->skills, &sn, cmpint))

#endif
