/*
 * Copyright (c) 1999 SoG Development Team
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
 * $Id: raffect.h,v 1.3.2.2 2001-02-25 17:23:39 fjoe Exp $
 */

#ifndef _RAFFECTS_H_
#define _RAFFECTS_H_

#include "merc.h"
/*
 * EVENTs for room affects
 */
#define EVENT_ENTER		(A)
#define EVENT_LEAVE		(B)
#define EVENT_UPDATE		(C)

struct rspell_t {
	const char *	name;
	const char *	enter_fun_name;
	const char *	update_fun_name;
	const char *	leave_fun_name;
	EVENT_FUN *	enter_fun;
	EVENT_FUN *	update_fun;
	EVENT_FUN *	leave_fun;
	int		sn;
	flag32_t	events;
};

extern varr rspells;

#define RSPELL(sn)		((rspell_t*) VARR_GET(&rspells, sn))
#define rspell_lookup(sn)	((rspell_t*) varr_get(&rspells, sn))

int		rsn_lookup	(int sn);

ROOM_AFFECT_DATA *raff_new		(void);
void	 	raff_free		(ROOM_AFFECT_DATA *raf);
void		check_room_affects	(CHAR_DATA *ch, ROOM_INDEX_DATA *room,
					 flag32_t event);
void		raffect_to_char		(ROOM_INDEX_DATA *room, CHAR_DATA *ch);
void		raffect_back_char	(ROOM_INDEX_DATA *room, CHAR_DATA *ch);
bool		is_safe_rspell		(ROOM_AFFECT_DATA *raf, CHAR_DATA *victim);

#endif
