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
 * $Id: clan.h,v 1.12 1999-02-12 10:33:28 kostik Exp $
 */

#ifndef _CLAN_H_
#define _CLAN_H_

/*----------------------------------------------------------------------
 * clan stuff (clan.c)
 */

#define CLAN_NONE 	 0

/* Clan status */
#define CLAN_LEADER	2
#define CLAN_SECOND	1
#define CLAN_COMMONER	0

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

	int 		altar_vnum;	/* vnum of room with clan item */
	int	 	obj_vnum;	/* vnum of clan item */
	int		mark_vnum;	/* vnum of clan mark */
	OBJ_DATA *	obj_ptr;
	const char *	leader_list;	/* list of leaders */
	const char *	member_list;	/* list of members */
	const char *	second_list;	/* list of secondaries */
};

/* clan flags */
#define CLAN_HIDDEN	(A)		/* clan will not appear in who */
#define CLAN_CHANGED	(Z)

CLAN_DATA *	clan_new	(void);		/* allocate new clan data */
void		clan_free	(CLAN_DATA*);	/* free clan data */
void		clan_save	(CLAN_DATA*);	/* save clan to disk */
int		cn_lookup	(const char* name); /* clan number lookup */
const char*	clan_name	(int cn);	/* clan name lookup */
bool		clan_item_ok	(int cn);	/* check clan item */

extern varr	clans;

#define CLAN(cn)		((CLAN_DATA*) VARR_GET(&clans, cn))
#define clan_lookup(cn)		((CLAN_DATA*) varr_get(&clans, cn))

struct clan_skill {
	int	sn;		/* skill number. leave this field first	 */
				/* in order sn_vlookup to work properly  */
	int	level;		/* level at which skill become available */
	int	percent;	/* initial percent			 */
};

#define clan_skill_lookup(clan, sn) \
	((CLAN_SKILL*) varr_bsearch(&clan->skills, &sn, cmpint))

#endif
