/*-
 * Copyright (c) 1998-2002 SoG Development Team
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
 * $Id: act_fishing.c,v 1.1.2.4 2002-09-09 19:26:29 tatyana Exp $
 */

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "fight.h"

void do_castout(CHAR_DATA *ch, const char *argument)
{
	OBJ_DATA *pole;
	int success, sk;

	if (IS_NPC(ch))
		return;

	if (IS_SET(PC(ch)->plr_flags, PLR_FISHING)) {
		act("You are already fishing!", ch, NULL, NULL, TO_CHAR);
		return;
	}

	if ((pole = get_eq_char(ch, WEAR_HOLD)) == NULL
	||   pole->pObjIndex->item_type != ITEM_FISHING_POLE) {
		act("You need to be holding a fishing pole first.",
		    ch, NULL, NULL, TO_CHAR);
		return;
	}

	if (!IS_SET(ch->in_room->room_flags, ROOM_SALTWATER | ROOM_FRESHWATER)) {
		act("This is not a good place to fish, you'll want "
		    "to find a better spot.", ch, NULL, NULL, TO_CHAR);
		return;
	}

	success = number_percent();

	if (IS_AFFECTED(ch, AFF_BLIND)) {
		if ((sk = get_skill(ch, gsn_blind_fishing)) == 0) {
			act("You don't know how to fish blinded.",
			    ch, NULL, NULL, TO_CHAR);
			return;
		}

		if (number_percent() < sk)
			check_improve(ch, gsn_blind_fishing, TRUE, 1);
		else {
			check_improve(ch, gsn_blind_fishing, FALSE, 1);
			success = -30;
		}
	}

/* mounted fishing */

	if (MOUNTED(ch)) {
		if ((sk = get_skill(ch, gsn_mounted_fishing)) == 0) {
			act("You don't know how to fish mounted.",
			    ch, NULL, NULL, TO_CHAR);
			return;
		}

		if (number_percent() + 3 < sk)
			check_improve(ch, gsn_mounted_fishing, TRUE, 1);
		else {
			act("You are tangled with $N and fall down.",
                            ch, NULL, ch->mount, TO_CHAR);
			act("$n is tangled with you and fall down.",
                            ch, NULL, ch->mount, TO_VICT);
			act("$n is tangled with $N and fall down.",
                            ch, NULL, ch->mount, TO_NOTVICT);

			if (ch->mount != NULL && ch->mount != ch) {
				ch->mount->mount = NULL;
				ch->mount->position = POS_RESTING;
			}

			ch->mount = NULL;
			ch->position = POS_RESTING;
			damage(ch, ch, dice(4, 6), gsn_mounted_fishing,
			       DAM_BASH, DAMF_SHOW);
			WAIT_STATE(ch, 15);
			check_improve(ch, gsn_mounted_fishing, FALSE, 1);
			return;
		}
	}

/* mastering fishing */
	sk = get_skill(ch, gsn_mastering_fishing);
	if (number_percent() + 9 < sk && success > 0) {
		check_improve(ch, gsn_mastering_fishing, TRUE, 1);
		success *= 3;
	} else
		check_improve(ch, gsn_mastering_fishing, FALSE, 1);

/* improved fisihing */
	sk = get_skill(ch, gsn_improved_fishing);
	if (number_percent() + 10 < sk) {
		check_improve(ch, gsn_improved_fishing, TRUE, 1);
		success += 15;
	} else
		check_improve(ch, gsn_improved_fishing, FALSE, 1);

/* expert fishing */
	sk = get_skill(ch, gsn_expert_fishing);
	if (number_percent() + 8 < sk && success > 0) {
		check_improve(ch, gsn_expert_fishing, TRUE, 1);
		success = 100;
	} else
		check_improve(ch, gsn_expert_fishing, FALSE, 1);

	if (success <= 30) {
		act("You pull your arm back and try to cast out "
		    "your line, but it gets all tangled up. Try again.",
                    ch, NULL, NULL, TO_CHAR);
		act("$n pulls $gn{his} arm back, trying to cast $gn{his} "
		    "fishing line out into the water, but ends up just a bit "
		    "tangled.", ch, NULL, NULL, TO_ROOM);
		WAIT_STATE(ch, 23);
		return;
	}

/* Ok, now they've gone through the checks, now set them fishing */
	SET_BIT(PC(ch)->plr_flags, PLR_FISHING);
	act("You cast your line out into the water, "
	    "hoping for a bite.", ch, NULL, NULL, TO_CHAR);
	act("$n casts $gn{his} line out into the water, hoping "
	    "to catch some food.", ch, NULL, NULL, TO_ROOM);
	WAIT_STATE(ch, 25);
}

void do_reelin(CHAR_DATA *ch, const char *argument)
{
	int success, fish_vnum, sk;
	OBJ_DATA *fish, *pole;
	OBJ_INDEX_DATA *index;
	int carry_w, carry_n;

	if (IS_NPC(ch))
		return;

	if (!IS_SET(PC(ch)->plr_flags, PLR_FISHING)) {
		act("You aren't even fishing!", ch, NULL, NULL, TO_CHAR);
		return;
	}

	if (!IS_SET(ch->in_room->room_flags, ROOM_SALTWATER | ROOM_FRESHWATER)) {
		act("This is not a good place to fish, you'll want "
		    "to find a better spot.", ch, NULL, NULL, TO_CHAR);
		return;
	}

	if ((pole = get_eq_char(ch, WEAR_HOLD)) == NULL
	||   pole->pObjIndex->item_type != ITEM_FISHING_POLE) {
		act("It seems like you lost your fishing pole.",
		    ch, NULL, NULL, TO_CHAR);
		return;
	}

	if (!IS_SET(PC(ch)->plr_flags, PLR_FISH_ON)) {
		act("You reel in your line, but alas... nothing on the end. "
		    "Better luck next time.", ch, NULL, NULL, TO_CHAR);
		REMOVE_BIT(PC(ch)->plr_flags, PLR_FISHING);
		act("$n reels $gn{his} line in, but with nothing on the end.",
                    ch, NULL, NULL, TO_ROOM);
		WAIT_STATE(ch, 27);
		return;
	}

	REMOVE_BIT(PC(ch)->plr_flags, PLR_FISHING | PLR_FISH_ON);

/* Ok, they are fishing and have a fish on */
	success = number_percent();
	if (IS_AFFECTED(ch, AFF_BLIND)) {
		sk = get_skill(ch, gsn_blind_fishing);
		if (sk == 0) {
			act("You don't know how to fish blinded.",
                            ch, NULL, NULL, TO_CHAR);
			return;
		}

		if (number_percent() < sk)
			check_improve(ch, gsn_blind_fishing, TRUE, 1);
		else {
			check_improve(ch, gsn_blind_fishing, FALSE, 1);
			success -= 30;
		}
	}

	if (MOUNTED(ch)) {
		if ((sk = get_skill(ch, gsn_mounted_fishing)) == 0) {
			act("You don't know how to fish mounted.",
                            ch, NULL, NULL, TO_CHAR);
			return;
		}

		if (number_percent() < sk - 3) {
			act("You are tangled with $N and fall down.",
			    ch, NULL, ch->mount, TO_CHAR);
			act("$n is tangled with you and fall down.",
			    ch, NULL, ch->mount, TO_VICT);
			act("$n is tangled with $N and fall down.",
			    ch, NULL, ch->mount, TO_NOTVICT);
			if (ch->mount != NULL && ch->mount != ch) {
				ch->mount->mount = NULL;
				ch->mount->position = POS_RESTING;
			}

			ch->mount = NULL;
			ch->position = POS_RESTING;
			damage(ch, ch, dice(4,6),
			       gsn_mounted_fishing, DAM_BASH, DAMF_SHOW);
			WAIT_STATE(ch, 15);
			check_improve(ch, gsn_mounted_fishing, FALSE, 1);
			return;
		} else
			check_improve(ch, gsn_mounted_fishing, TRUE, 1);
	}

	sk = get_skill(ch, gsn_mastering_fishing);
	if (number_percent() < sk - 9) {
		check_improve(ch, gsn_mastering_fishing, TRUE, 1);
		success *= 2;
	} else
		check_improve(ch, gsn_mastering_fishing, FALSE, 1);

	sk = get_skill(ch, gsn_improved_fishing);
	if (number_percent() < sk - 10) {
		check_improve(ch, gsn_improved_fishing, TRUE, 1);
		success += 20;
	} else
		check_improve(ch, gsn_improved_fishing, FALSE, 1);

	sk = get_skill(ch, gsn_expert_fishing);
	if (number_percent() < (sk - 8)
	&&  success > 0) {
		check_improve(ch, gsn_expert_fishing, TRUE, 1);
		success = 100;
	} else
		check_improve(ch, gsn_expert_fishing, FALSE, 1);

	if (success <= 60) {
		act("You reel in your line, putting up a good fight, but you "
                    "lose him! Try again?", ch, NULL, NULL, TO_CHAR);
		act("$n reels $gn{his} line in, fighting with whatever is "
		    "on the end, but loses the catch.",
		    ch, NULL, NULL, TO_ROOM);
		return;
		WAIT_STATE(ch, 29);
		SET_FIGHT_TIME(ch);
	}

	check_improve(ch, gsn_winter_fishing, TRUE, 4);
	check_improve(ch, gsn_outraging_fishing, TRUE, 4);

	fish_vnum = fish_vnum_lookup(ch->in_room->room_flags);
	index = get_obj_index(fish_vnum);
	if (index == NULL) {
		act("Something wrong. Report it to immortals.",
		    ch, NULL, NULL, TO_CHAR);
		log("[*****] BUG: NULL object, vnum %d.", fish_vnum);
		return;
	}

	fish = create_obj(index, 0);
	fish->timer = number_fuzzy(30);
	act("You reel in $p! Nice catch!", ch, fish, NULL, TO_CHAR);
	act("Wow! $n reels in a helluva catch! Looks like $p!",
            ch, fish, NULL, TO_ROOM);

	if (((carry_n = can_carry_n(ch)) >= 0 &&
	      ch->carry_number + get_obj_number(fish) > carry_n)
	||  ((carry_w = can_carry_w(ch)) >= 0 &&
		get_carry_weight(ch) + get_obj_weight(fish) > carry_w))
		obj_to_room(fish, ch->in_room);
	else
		obj_to_char(fish, ch);

	WAIT_STATE(ch, 31);
	SET_FIGHT_TIME(ch);
}
