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
 * $Id: act_auction.c,v 1.11 2003-04-25 12:49:10 fjoe Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include <merc.h>

#include <sog.h>
#include <magic.h>
#include <update.h>

#include "auction_impl.h"

DECLARE_DO_FUN(do_auction);

void
do_auction(CHAR_DATA *ch, const char *argument)
{
	int tax;
	OBJ_DATA *obj;
	char arg1[MAX_INPUT_LENGTH];
	char starting[MAX_INPUT_LENGTH];

	argument = one_argument(argument, arg1, sizeof(arg1));

	if (IS_NPC(ch))    /* NPC can't auction cos it can be extracted! */
		return;

	if (!str_cmp(arg1, "off")) {
		act_char("Auction channel is now OFF.", ch);
		SET_BIT(ch->chan, CHAN_NOAUCTION);
		return;
	}

	if (IS_SET(ch->chan, CHAN_NOAUCTION)) {
		act_char("Auction channel is now ON.", ch);
		REMOVE_BIT(ch->chan, CHAN_NOAUCTION);
	}

	if (IS_SET(ch->in_room->room_flags, ROOM_SILENT)
	&&  !IS_IMMORTAL(ch)) {
		act_char("You are in silent room, auction"
			 " is not available.", ch);
		return;
	}

	if (arg1[0] == '\0') {
		if (auction.item != NULL) {
			/* show item data here */
			if (IS_IMMORTAL(ch)) {
				act_puts("Sold by: $i. Last bet by: $N.",
					 ch, auction.seller, auction.buyer,
					 TO_CHAR, POS_DEAD);
			}
			if (auction.bet > 0) {
				act_puts("Current bet on this item is "
					 "$j gold.", ch,
					 (const void*) auction.bet, NULL,
					 TO_CHAR, POS_DEAD);
			} else {
				act_puts("Starting price for this item is "
					 "$j gold.", ch,
					 (const void*) auction.starting, NULL,
					 TO_CHAR, POS_DEAD);
				act_puts("No bets on this item have been "
					 "received.", ch, NULL, NULL,
					 TO_CHAR, POS_DEAD);
			}
			spellfun("identify", NULL, 0, ch, auction.item);
			return;
		} else {
			act_char("Auction WHAT?", ch);
			return;
		}
	}

	if (IS_IMMORTAL(ch) && !str_cmp(arg1, "stop")) {
		if (auction.item == NULL) {
			act_char("There is no auction going on you can stop.", ch);
			return;
		} else { /* stop the auction */
			act_auction("Sale of $p has been stopped by $N.",
				    auction.item, ch, NULL,
				    ACT_FORMSH | ACT_NOCANSEE, POS_DEAD);
			auction_give_obj(auction.seller);

			/* return money to the buyer */
			if (auction.buyer != NULL) {
				PC(auction.buyer)->bank_g += auction.bet;
				act_puts("Your money has been returned.",
					 auction.buyer, NULL, NULL,
					 TO_CHAR, POS_DEAD);
			}

			/* return money to the seller */
			if (auction.seller != NULL) {
				PC(auction.seller)->bank_g +=
					(auction.starting * 20) / 100;
				act_puts("Your money has been returned.",
					 auction.seller, NULL, NULL,
					 TO_CHAR, POS_DEAD);
			}
			return;
		}
	}

	if (!str_cmp(arg1, "bet")) {
	        int newbet;

		if (auction.item == NULL) {
			act_puts("There isn't anything being auctioned "
				 "right now.",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
			return;
		}

		if (ch == auction.seller) {
			act_puts("You cannot bet on your own equipment...:)",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
			return;
		}

	        /* make - perhaps - a bet now */
	        if (argument[0] == '\0') {
			act_puts("Bet how much?",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
			return;
	        }

		newbet = parsebet(auction.bet, argument);

	        if (newbet > ch->gold
		&&  newbet > PC(ch)->bank_g) {
			act_puts("You don't have that much money!",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
			return;
	        }

		if (auction.bet > 0) {
			if (newbet < auction.bet + 1) {
				act_puts("You must bid at least 1 gold "
					 "over the current bet.",
					 ch, NULL, NULL, TO_CHAR, POS_DEAD);
				return;
			}
		} else {
			if (newbet < auction.starting) {
				act_puts("You cannot bid less than the "
					 "starting price.",
					 ch, NULL, NULL, TO_CHAR, POS_DEAD);
				return;
			}
		}

	        /* the actual bet is OK! */

	        /* return the gold to the last buyer, if one exists */
	        if (auction.buyer != NULL)
			auction.buyer->gold += auction.bet;

		if (newbet > PC(ch)->bank_g)
			ch->gold -= newbet;
		else
			PC(ch)->bank_g -= newbet;
	        auction.buyer = ch;
	        auction.bet   = newbet;
	        auction.going = 0;
	        update_reset("auction"); /* start the auction over again */

	        act_auction("A bet of $J gold has been received on $p.",
			    auction.item, NULL, (const void*) newbet,
			    ACT_FORMSH | ACT_NOCANSEE, POS_RESTING);
	        return;
	}

	/* finally... */

	obj = get_obj_carry(ch, ch, arg1); /* does char have the item ? */

	if (obj == NULL) {
		act_puts("You aren't carrying that.",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		return;
	}

	if (auction.item != NULL) {
		act_puts("Try again later - $p is being auctioned right now!",
			 ch, auction.item, NULL, TO_CHAR | ACT_NOCANSEE,
			 POS_DEAD);
		return;
	}

	if (IS_OBJ_STAT(obj, ITEM_KEEP)) {
		act_char("Unkeep it first.", ch);
		return;
	}

	argument = one_argument(argument, starting, sizeof(starting));
	if (starting[0] == '\0')
		auction.starting = MIN_START_PRICE;
	else if ((auction.starting = atoi(starting)) < MIN_START_PRICE) {
		act_puts("You must specify the starting price "
			 "(at least $j gold).",
			 ch, (const void*) MIN_START_PRICE, NULL,
			 TO_CHAR, POS_DEAD);
		return;
	}

	switch (obj->item_type) {
	default:
		if (!OBJ_IS(obj, OBJ_CHQUEST)) {
			act_puts("You cannot auction $T.",
				 ch, NULL,
				 flag_string(item_types,
					     obj->item_type),
				 TO_CHAR, POS_SLEEPING);
			break;
		}
		/* FALLTHRU */

	case ITEM_CONTAINER:
		if (obj->contains) {
			act_puts("You can auction only empty containers.",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
			break;
		}

		/* FALLTHRU */

	case ITEM_LIGHT:
	case ITEM_WEAPON:
	case ITEM_ARMOR:
	case ITEM_STAFF:
	case ITEM_WAND:
	case ITEM_GEM:
	case ITEM_TREASURE:
	case ITEM_JEWELRY:
	case ITEM_FURNITURE:
	case ITEM_FOOD:
	case ITEM_SCROLL:
	case ITEM_BOAT:
	case ITEM_POTION:
	case ITEM_PILL:
	case ITEM_MAP:
	case ITEM_CLOTHING:
	case ITEM_DRINK_CON:
	case ITEM_WARP_STONE:
		tax = (auction.starting * 20) / 100;
		if (PC(ch)->bank_g < tax && ch->gold < tax) {
			act_puts("You do not have enough gold to pay "
				 "an auction fee of $j gold.",
				 ch, (const void*) tax, NULL,
				 TO_CHAR, POS_DEAD);
			return;
		}

		act_puts("The auctioneer charges you an auction fee "
			 "of $j gold.",
			 ch, (const void*) tax, NULL,
			 TO_CHAR, POS_DEAD);

		if (PC(ch)->bank_g < tax)
			ch->gold -= tax;
		else
			PC(ch)->bank_g -= tax;

		obj_from_char(obj);
		auction.item = obj;
		auction.bet = 0;	/* obj->cost / 100 */
		auction.buyer = NULL;
		auction.seller = ch;
		update_reset("auction");
		auction.going = 0;

		act_auction("A new item has been received: {Y$p{x.",
			    obj, NULL, NULL, ACT_FORMSH | ACT_NOCANSEE,
			    POS_RESTING);
		break;
	} /* switch */
}
