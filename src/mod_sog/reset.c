/*-
 * Copyright (c) 2001 SoG Development Team
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
 * $Id: reset.c,v 1.4 2001-08-20 16:47:45 fjoe Exp $
 */

#include <stdio.h>
#include <merc.h>

#include <sog.h>

static int reset_room_vnum;	/* vnum of room being reset */
static int reset_num;		/* number of reset in room being reset */

static void
logger_reset(const char *buf)
{
	char buf2[MAX_STRING_LENGTH];
	snprintf(buf2, sizeof(buf2), "reset_room %d[%d]: %s",	// notrans
		 reset_room_vnum, reset_num, buf);
	logger_default(buf2);
}

static void *
clan_item_cb(void *p, va_list ap)
{
	clan_t *clan = (clan_t *) p;

	OBJ_INDEX_DATA *pObjIndex = va_arg(ap, OBJ_INDEX_DATA *);

	if (clan->obj_ptr == NULL
	||  pObjIndex->vnum != clan->obj_vnum)
		return NULL;

	return p;
}

/*
 * OLC
 * Reset one room.  Called by reset_area and olc.
 */
void
reset_room(ROOM_INDEX_DATA *pRoom, int flags)
{
	RESET_DATA *pReset;

	CHAR_DATA *last_mob = NULL;
	bool lmob = FALSE;		/* last mob was reset */

	OBJ_DATA *last_obj = NULL;
	bool lobj = FALSE;		/* last obj was reset */

	int iExit;
	logger_t logger_old;

	for (iExit = 0; iExit < MAX_DIR; iExit++) {
		EXIT_DATA *pExit;

		if ((pExit = pRoom->exit[iExit]) == NULL
		/*  || IS_SET(pExit->exit_info, EX_BASHED) */)
			continue;

                /*
		 * nail both sides
		 */
		pExit->exit_info = pExit->rs_flags;
		if (pExit->to_room.r != NULL
		&&  (pExit = pExit->to_room.r->exit[rev_dir[iExit]]) != NULL)
			pExit->exit_info = pExit->rs_flags;
	}

	reset_room_vnum = pRoom->vnum;
	logger_old = logger_set(LOG_BUG, logger_reset);

	for (reset_num = 0, pReset = pRoom->reset_first; pReset != NULL;
					pReset = pReset->next, reset_num++) {
		MOB_INDEX_DATA *pMobIndex;
		OBJ_INDEX_DATA *pObjIndex;
		OBJ_DATA *obj;
		int count, limit;
		EXIT_DATA *pExit;
		int d0;

		switch (pReset->command) {
		default:
			log(LOG_BUG, "bad command %c", pReset->command);
			break;

		case 'M':
			/*
			 * mob to room
			 *	arg1 - mob vnum
			 *	arg2 - mob count limit (total)
			 *	arg3 - room vnum
			 *	arg4 - mob count limit (in room)
			 */
			if ((pMobIndex = get_mob_index(pReset->arg1)) == NULL) {
				log(LOG_BUG, "%d: no such mob", pReset->arg1);
				lmob = FALSE;
				break;
			}

			if (pMobIndex->count >= pReset->arg2) {
				lmob = FALSE;
				break;
			}

			count = 0;
			for (last_mob = pRoom->people; last_mob != NULL; last_mob = last_mob->next_in_room) {
				if (last_mob->pMobIndex == pMobIndex)
					count++;
			}

			if (count >= pReset->arg4) {
				lmob = FALSE;
				break;
			}

			/*
			 * create_mob can't return NULL because
			 * get_mob_index returned not NULL
			 */
			last_mob = create_mob(pMobIndex->vnum, 0);
			NPC(last_mob)->zone = pRoom->area;
			char_to_room(last_mob, pRoom);
			if (IS_EXTRACTED(last_mob)) {
				lmob = FALSE;
				break;
			}
			lmob = TRUE;
			break;

		case 'G':
		case 'E':
			/*
			 * give obj to char or equip char (reset by 'M')
			 *	arg1 - obj vnum
			 *	arg2 -
			 *	arg3 - wear location (for 'E')
			 *	arg4 -
			 */
			if (!lmob)
				break;

			if (last_mob == NULL) {
				log(LOG_BUG, "no previous mob");
				lobj = FALSE;
				break;
			}

			if ((pObjIndex = get_obj_index(pReset->arg1)) == NULL) {
				log(LOG_BUG, "%d: no such obj", pReset->arg1);
				lobj = FALSE;
				break;
			}

			if ((pObjIndex->limit != -1)
			&&  pObjIndex->count >= pObjIndex->limit) {
				lobj = FALSE;
				break;
			}

			if (number_percent() < pReset->arg0) {
				lobj = FALSE;
				break;
			}

			/*
			 * create_obj can't fail here because get_obj_index
			 * returned not NULL
			 */
			last_obj = create_obj(pObjIndex->vnum, 0);
			pObjIndex->reset_num++;
			if (pReset->command == 'G'
			&&  last_mob->pMobIndex->pShop) /* Shop-keeper? */
				SET_OBJ_STAT(last_obj, ITEM_INVENTORY);

			obj_to_char(last_obj, last_mob);
			if (pReset->command == 'E')
				equip_char(last_mob, last_obj, pReset->arg3);
			lobj = TRUE;
			break;

		case 'O':
			/*
			 * obj to room
			 *	arg1 - obj vnum
			 *	arg2 -
			 *	arg3 - room vnum
			 *	arg3 -
			 * obj limits are checked
			 */
			if ((pObjIndex = get_obj_index(pReset->arg1)) == NULL) {
				log(LOG_BUG, "%d: no such obj", pReset->arg1);
				lobj = FALSE;
				break;
			}

			if (number_percent() < pReset->arg0) {
				lobj = FALSE;
				break;
			}

			if ((pRoom->area->nplayer > 0 &&
			     !IS_SET(flags, RESET_F_NOPCHECK))
			||  count_obj_list(pObjIndex, pRoom->contents) > 0
			||  (pObjIndex->limit != -1 &&
			     pObjIndex->count >= pObjIndex->limit)) {
				lobj = FALSE;
				break;
			}

			/*
			 * create_obj can't fail here because get_obj_index
			 * returned not NULL
			 */
			last_obj = create_obj(pObjIndex->vnum, 0);
			pObjIndex->reset_num++;
			last_obj->cost = 0;
			lobj = TRUE;
			obj_to_room(last_obj, pRoom);
			break;

		case 'P':
			/*
			 * put obj in last obj (reset by 'E', 'G' or 'O')
			 *	arg1 - vnum of obj to put
			 *	arg2 - obj count limit (total)
			 *	arg3 -
			 *	arg4 - min obj count (in obj)
			 * obj limits are checked
			 */

			if (!lobj)
				break;

			if (last_obj == NULL) {
				log(LOG_BUG, "no previous obj");
				break;
			}

			if ((pObjIndex = get_obj_index(pReset->arg1)) == NULL) {
				log(LOG_BUG, "%d: no such obj", pReset->arg1);
				break;
			}

			if (pReset->arg2 > 50) /* old format */
				limit = 6;
			else if (pReset->arg2 == -1) /* no limit */
				limit = 999;
			else
				limit = pReset->arg2;

			if (number_percent() < pReset->arg0)
				break;

			if (pRoom->area->nplayer > 0
			&&  !IS_SET(flags, RESET_F_NOPCHECK))
				break;

			if (IS_SET(pObjIndex->obj_flags, OBJ_CLAN)) {
				clan_t* clan;

				clan = hash_foreach(&clans, clan_item_cb,
						    pObjIndex);
				if (clan != NULL) {
					/*
					 * create_obj can't fail here because
					 * get_obj_index returned not NULL
					 */
					obj = create_obj(pObjIndex->vnum, 0);
					pObjIndex->reset_num++;
					clan->obj_ptr = obj;
					clan->altar_ptr = obj;
					obj_to_obj(obj, last_obj);
				}
				break;
			}

			count = count_obj_list(pObjIndex, last_obj->contains);
			for (; ;) {
				if (count >= pReset->arg4
				||  (pObjIndex->count >= limit &&
				     number_range(0, 4) != 0)
				||  (pObjIndex->limit != -1 &&
				     pObjIndex->count >= pObjIndex->limit))
					break;

				/*
				 * create_obj can't fail here because
				 * get_obj_index returned not NULL
				 */
				obj = create_obj(pObjIndex->vnum, 0);
				pObjIndex->reset_num++;
				obj_to_obj(obj, last_obj);
				count++;
			}

			/* fix object lock state! */
			last_obj->value[1] = last_obj->pObjIndex->value[1];
			break;

		case 'R':
			/*
			 * randomize exits
			 */
			for (d0 = 0; d0 < pReset->arg2 - 1; d0++) {
				int d1 = number_range(d0, pReset->arg2 - 1);
				pExit = pRoom->exit[d0];
				pRoom->exit[d0] = pRoom->exit[d1];
				pRoom->exit[d1] = pExit;
			}
			break;
		} /* switch */
	} /* for */

	logger_set(LOG_BUG, logger_old);
}

/*
 * OLC
 * Reset one area.
 */
void
reset_area(AREA_DATA *pArea)
{
	ROOM_INDEX_DATA *pRoom;
	int vnum;

	for (vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++)
		if ((pRoom = get_room_index(vnum)) != NULL)
			reset_room(pRoom, 0);
}
