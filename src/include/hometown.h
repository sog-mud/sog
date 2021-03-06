/*-
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
 * $Id: hometown.h,v 1.11 2001-09-15 17:12:32 fjoe Exp $
 */

#ifndef _HOMETOWN_H_
#define _HOMETOWN_H_

struct altar_t {
	ROOM_INDEX_DATA *	room;
	OBJ_INDEX_DATA *	pit;
};

struct hometown_t {
	const char* area;

	ROOM_INDEX_DATA *	recall[MAX_AN];		/* recall points */
	OBJ_INDEX_DATA *	map[MAX_AN];		/* maps		 */
	altar_t			altar[MAX_AN];
	const char *		restrict_race;
	const char *		restrict_class;
	flag_t			restrict_align;
};

extern varr hometowns;
extern varr_info_t c_info_hometowns;

int		htn_lookup	(const char *name);
const char *	hometown_name	(int htn);

#define HOMETOWN(htn)	((hometown_t*) VARR_GET(&hometowns, htn))

bool		hometown_restrict(hometown_t *h, CHAR_DATA *ch);
int		hometown_permanent(CHAR_DATA *ch);

/* the following three functions assume ch is !IS_NPC */
ROOM_INDEX_DATA*	get_recall(CHAR_DATA *ch);
OBJ_INDEX_DATA*		get_map(CHAR_DATA *ch);
altar_t *		get_altar(CHAR_DATA *ch);
ROOM_INDEX_DATA*	get_random_recall(void);

#endif
