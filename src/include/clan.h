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
 * $Id: clan.h,v 1.29 2001-09-13 16:21:49 fjoe Exp $
 */

#ifndef _CLAN_H_
#define _CLAN_H_

/*----------------------------------------------------------------------
 * clan stuff (clan.c)
 */

/* Clan status */
#define CLAN_LEADER	2
#define CLAN_SECOND	1
#define CLAN_COMMONER	0

/*
 * Clan structure
 */
struct clan_t
{
	const char *	name;		/* clan name */

	int		recall_vnum;	/* recall room vnum */
	const char *	skill_spec;	/* skill spec for this clan */

	flag_t		clan_flags;	/* clan flags */

	int		altar_vnum;	/* vnum of room with clan item */
	int		obj_vnum;	/* vnum of clan item */
	int		mark_vnum;	/* vnum of clan mark */
	OBJ_DATA *	obj_ptr;	/* pointer to clan item */
	OBJ_DATA *	altar_ptr;	/* pointer to altar (obj with clan item)*/
	const char *	leader_list;	/* list of leaders */
	const char *	member_list;	/* list of members */
	const char *	second_list;	/* list of secondaries */
};

/* clan flags */
#define CLAN_HIDDEN	(A)		/* clan will not appear in who */
#define CLAN_CHANGED	(Z)

extern avltree_t clans;
extern avltree_info_t c_info_clans;

#define clan_lookup(cln)	((clan_t*) c_strkey_lookup(&clans, (cln)))
#define clan_search(cln)	((clan_t*) c_strkey_search(&clans, (cln)))

#define IS_CLAN(cln1, cln2)	(!str_cmp((cln1), (cln2)))

#endif
