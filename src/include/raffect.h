/*
 * Copyright (c) 1999 Arborn <avn@org.chem.msu.su>
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
 * $Id: raffect.h,v 1.4 1999-10-06 09:56:00 fjoe Exp $
 */

#ifndef _RAFFECTS_H_
#define _RAFFECTS_H_

struct room_affect_data
{
	ROOM_AFFECT_DATA *	next;
	flag32_t		where;
	const char *		type;
	int			level;
	CHAR_DATA *		owner;
	int			duration;
	int			location;
	int			modifier;
	flag32_t		bitvector;
	flag32_t		revents;
};

/*
 * EVENTs for room affects
 */
#define REVENT_ENTER		(A)
#define REVENT_LEAVE		(B)
#define REVENT_UPDATE		(C)

struct rspell_t {
	const char *	sn;
	const char *	enter_fun_name;
	const char *	update_fun_name;
	const char *	leave_fun_name;
	REVENT_FUN *	enter_fun;
	REVENT_FUN *	update_fun;
	REVENT_FUN *	leave_fun;
	flag32_t	revents;
};

extern hash_t rspells;

void rspell_init(rspell_t *sk);
rspell_t *rspell_cpy(rspell_t *dst, const rspell_t *src);
void rspell_destroy(rspell_t *sk);

#define rspell_lookup(sn)	((rspell_t*) hash_lookup(&rspells, sn))

ROOM_AFFECT_DATA *raff_new		(void);
void	 	raff_free		(ROOM_AFFECT_DATA *raf);
void		check_room_affects	(CHAR_DATA *ch, ROOM_INDEX_DATA *room,
					 int event);
void		raffect_to_char		(ROOM_INDEX_DATA *room, CHAR_DATA *ch);
void		raffect_back_char	(ROOM_INDEX_DATA *room, CHAR_DATA *ch);
bool		is_safe_rspell		(ROOM_AFFECT_DATA *raf, CHAR_DATA *victim);

void	affect_to_room	(ROOM_INDEX_DATA *room, ROOM_AFFECT_DATA *paf);
void	affect_remove_room	(ROOM_INDEX_DATA *room, ROOM_AFFECT_DATA *paf);
void	affect_strip_room	(ROOM_INDEX_DATA *ch, const char *sn);
bool	is_affected_room	(ROOM_INDEX_DATA *ch, const char *sn);
void	affect_join_room	(ROOM_INDEX_DATA *ch, ROOM_AFFECT_DATA *paf);
void	affect_join	(CHAR_DATA *ch, AFFECT_DATA *paf);

#endif
