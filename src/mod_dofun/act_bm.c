/*-
 * Copyright (c) 2002 SoG Development Team
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
 * 3. Follow all guidelines in the diku license (contained in the file
 *    license.doc)
 * 4. Follow all guidelines in the Merc license (contained in the file
 *    license.txt)
 * 5. Follow all guidelines in the ROM license (contained in the file
 *    rom.license)
 * 6. Follow all guidelines in the Anatolia license (contained in the file
 *    anatolia.license)
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
 * $Id: act_bm.c,v 1.1.2.4 2002-10-18 18:38:40 tatyana Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "merc.h"
#include "clan.h"
#include "note.h"
#include "bm.h"

DECLARE_DO_FUN(do_help);

int advatoi (const char *);

static void send_notice(CHAR_DATA *victim, OBJ_DATA *obj, int type);

#define NOTICE_BUYER		1
#define NOTICE_BET		2

/*
	Main black market function.
	Usage:
		bm [list]
		bm bet <bet> <number.item | number.>
		bm accept <number.item | number.>
		bm [propose | sell] <number.item>

*/

void do_bm(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	char arg1[MAX_INPUT_LENGTH];
	CHAR_DATA *buyer;
	bmitem_t *item;
	bool loaded_buyer = FALSE, no_buyer = FALSE;

	item = bmitem_list;

	if (IS_NPC(ch))
		return;

        if (IS_SET(ch->in_room->room_flags, ROOM_SILENT)
        &&  !IS_IMMORTAL(ch)) {
                char_puts("You are in silent room.\n", ch);
                return;
        }
	argument = one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		do_help(ch, "'BLACK MARKET'");
		return;
	}

	if (!str_cmp(arg, "list")) {
		uint counter = 0;
		if (item == NULL) {
			act("{D[BLACK MARKET]{x You can buy nothing today!",
			    ch, NULL, NULL, TO_CHAR);
			return;
		}

		for (item = bmitem_list; item != NULL && item->obj != NULL; item = item->next) {
			counter++;
			act("[$j] ", ch, (const void *) counter, NULL,
			    TO_CHAR | ACT_NOLF);
			act_puts3("{CItem{x: $P (level $j, limit by $J): ",
			    ch,
			    (const void *) item->obj->level,
			    item->obj,
			    (const void *) item->obj->pObjIndex->limit,
			    TO_CHAR | ACT_NOUCASE | ACT_NOCANSEE,
			    POS_DEAD);
			if (IS_IMMORTAL(ch)) {
				act_puts3("	current bet - {C$J{x; "
					  "seller - $t; buyer - $T.",
					  ch,
					  IS_NULLSTR(item->seller) ? "nobody" : item->seller,
					  IS_NULLSTR(item->buyer) ? "nobody" : item->buyer,
					  (const void *) item->bet,
					  TO_CHAR, POS_DEAD);
			} else if (IS_HUNTER(ch)) {
				act_puts3("	current bet - {C$J{x; "
					  "seller - $t.",
					  ch,
					  IS_NULLSTR(item->seller) ? "nobody" : item->seller,
					  NULL, (const void *) item->bet,
					  TO_CHAR, POS_DEAD);
			} else {
				act_puts3("	current bet - {C$J{x; "
					  "seller - $t; buyer - $T",
					  ch,
					  item->seller != ch->name ? "unknown" : "{CYou!{x",
					  item->buyer != ch->name ? "unknown" : "{CYou!{x",
					  (const void *) item->bet,
					  TO_CHAR, POS_DEAD);
			}
		}
		return;
	}

	if (!str_cmp(arg, "bet")) {
/* attempt to make a bet */
		uint number;
		int newbet;
		bool found;
		found = loaded_buyer = no_buyer = FALSE;

		argument = one_argument(argument, arg, sizeof(arg));
		number = number_argument(argument, arg1, sizeof(arg1));

		if (IS_HUNTER(ch)) {
			act("{D[BLACK MARKET]{x You can't make a bet, hunter!",
			    ch, NULL, NULL, TO_CHAR);
			return;
		}

		if (arg[0] == '\0') {
			act ("{D[BLACK MARKET]{x Bet how much?",
			     ch, NULL, NULL, TO_CHAR);
			return;
		}

		newbet = advatoi(arg);
		if (newbet == 0) {
			act("{D[BLACK MARKET]{x Incorrect bet.",
			    ch, NULL, NULL, TO_CHAR);
			return;
		}

		if (newbet > (PC(ch)->bank_g)) {
			act("{D[BLACK MARKET]{x You don't have that much"
			    " money in your account!",
			    ch, NULL, NULL, TO_CHAR);
			return;
		}

		for (item = bmitem_list; item != NULL && item->obj != NULL; item = item->next) {
			if ((arg1[0] == '\0' ||
			     is_name(arg1, item->obj->name))
			&&  !--number) {
				found  = TRUE;
				break;
			}
		}

		if (!found) {
			act ("{D[BLACK MARKET]{x There is no such item.",
			     ch, NULL, NULL, TO_CHAR);
			return;
		}

		if (!str_cmp(item->seller, ch->name)) {
			act("{D[BLACK MARKET]{x You can't bet on your "
			    "own object.",
			    ch, NULL, NULL, TO_CHAR);
			return;
		}

		if ((IS_OBJ_STAT(item->obj, ITEM_ANTI_EVIL) && IS_EVIL(ch))
		||  (IS_OBJ_STAT(item->obj, ITEM_ANTI_GOOD) && IS_GOOD(ch))
		||  (IS_OBJ_STAT(item->obj, ITEM_ANTI_NEUTRAL) &&
		     IS_NEUTRAL(ch))) {
			act("{D[BLACK MARKET]{x You can't bet on {D$p{x.",
			    ch, item->obj, NULL, TO_CHAR | ACT_NOCANSEE);
			return;
		}

		if (newbet < item->bet * 11 / 10) {
			act("{D[BLACK MARKET]{x You must bid at least 10% "
			    "over the current bet.",
			    ch, NULL, NULL, TO_CHAR);
			return;
		}

		if (IS_NULLSTR(item->buyer)) {
			PC(ch)->bank_g -= newbet;
			item->buyer = str_dup(ch->name);
			item->bet = newbet;
			act("{D[BLACK MARKET]{x You successfully make a bet "
			    "of $j gold on {D$P{x.",
			    ch, (const void *) newbet, item->obj,
			    TO_CHAR | ACT_NOCANSEE);
			return;
		}

		if ((buyer = get_char_world(ch, item->buyer)) == NULL) {
			if ((buyer = char_load(item->buyer, LOAD_F_NOCREATE)) == NULL)
				no_buyer = TRUE;
			else
				loaded_buyer = TRUE;
		}

		if (IS_NPC(buyer))
			return;

		if (!no_buyer) {
			PC(buyer)->bank_g += item->bet;
			if (loaded_buyer) {
				send_notice(buyer, item->obj, NOTICE_BET);
				char_save(buyer, SAVE_F_PSCAN);
				char_nuke(buyer);
			} else {
				act("{D[BLACK MARKET]{x You bet ($j gold)"
				    " on {D$P{x exceeded.", buyer,
				    (const void *) item->bet, item->obj,
				    TO_CHAR | ACT_NOCANSEE);
			}
		}

		PC(ch)->bank_g -= newbet;
		item->bet = newbet;
		free_string(item->buyer);
		item->buyer = str_dup(ch->name);
		act("{D[BLACK MARKET]{x You successfully make a bet of "
		    "$j gold on {D$P{x.",
		    ch, (const void *) newbet, item->obj,
		    TO_CHAR | ACT_NOCANSEE);
		save_black_market();
		return;
	}

	if (!str_cmp(arg, "accept")) {
/* sell item */
		bmitem_t *prev;
		uint number;
		bool found;
		found = loaded_buyer = no_buyer = FALSE;
		prev = NULL;

		argument = one_argument(argument, arg, sizeof(arg));
		number = number_argument(arg, arg1, sizeof(arg1));

		for (item = bmitem_list; item != NULL && item->obj != NULL; prev = item, item = item->next)
			if ((arg1[0] == '\0' ||
			     is_name(arg1, item->obj->name))
			&&  !--number) {
				found  = TRUE;
				break;
			}

		if (!found) {
			act ("{D[BLACK MARKET]{x There is no such item in "
			     "sale.", ch, NULL, NULL, TO_CHAR);
			return;
		}

		if (IS_NULLSTR(item->buyer)) {
			act("{D[BLACK MARKET]{x Nobody likes to buy it.",
			    ch, NULL, NULL, TO_CHAR);
			return;
		}

		if (ch != get_char_world(ch, item->seller)) {
			act ("{D[BLACK MARKET]{x You haven't access to "
			     "sell {D$p{x.",
			     ch, item->obj, NULL, TO_CHAR | ACT_NOCANSEE);
			return;
		}

		PC(ch)->bank_g += item->bet;

		if ((buyer = get_char_world(ch, item->buyer)) == NULL) {
			if ((buyer = char_load(item->buyer, LOAD_F_NOCREATE)) == NULL)
				no_buyer = TRUE;
			else
				loaded_buyer = TRUE;
		}

		if (no_buyer || IS_NPC(buyer))
			return;

		act("{D[BLACK MARKET]{x You accept selling of {D$P{x for"
		    " $j gold.",
		    ch, (const void *) item->bet, item->obj,
		    TO_CHAR | ACT_NOCANSEE);

		if (loaded_buyer) {
			send_notice(buyer, item->obj, NOTICE_BUYER);
		} else {
			send_notice(buyer, item->obj, NOTICE_BUYER);
			act("{D[BLACK MARKET]{x You bought {D$p{x at black "
			    "market.", buyer, item->obj, NULL,
			    TO_CHAR | ACT_NOCANSEE);
		}
		obj_to_char(item->obj, buyer);
		char_save(buyer, loaded_buyer ? SAVE_F_PSCAN : 0);
		if (loaded_buyer)
			char_nuke(buyer);
		if (prev == NULL)
			bmitem_list = item->next;
		else
			prev->next = item->next;
		bmitem_free(item);
		save_black_market();
		return;
	}

	if (!str_cmp(arg, "propose") || !str_cmp(arg, "sell")) {
		OBJ_DATA *obj;
		int limit;
		int price;

		if (!IS_HUNTER(ch)) {
			act("{D[BLACK MARKET]{x You can't sell anything "
			    "at black market.", ch, NULL, NULL, TO_CHAR);
			return;
		}

		argument = one_argument(argument, arg, sizeof(arg));

		if (arg[0] == '\0') {
			act("{D[BLACK MARKET]{x What do you like to sell?",
			    ch, NULL, NULL, TO_CHAR);
			return;
		}

		obj = get_obj_carry(ch, arg);

		if (obj == NULL) {
			act ("{D[BLACK MARKET]{x You don't have that item.",
			     ch, NULL, NULL, TO_CHAR);
			return;
		}

		if (obj->pObjIndex->limit == -1) {
			act ("{D[BLACK MARKET]{x You can't sell {D$p{x here.",
			     ch, obj, NULL, TO_CHAR | ACT_NOCANSEE);
			return;
		}

		switch (obj->pObjIndex->item_type) {
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
		case ITEM_FISH:
		case ITEM_SCROLL:
		case ITEM_BOAT:
		case ITEM_PILL:
		case ITEM_POTION:
		case ITEM_CLOTHING:
		case ITEM_DRINK_CON:
		case ITEM_MAP:
		case ITEM_WARP_STONE:
		case ITEM_FISHING_POLE:
		case ITEM_CONTAINER:
			if (!can_drop_obj(ch, obj)) {
				act("You can't let go of it.",
				    ch, NULL, NULL, TO_CHAR);
				return;
			}

			if (obj->contains) {
				act_puts("You can sell only empty containers.",
					 ch, NULL, NULL, TO_CHAR, POS_DEAD);
				return;
			}
			break;
		default:
			act_puts("{D[BLACK MARKET]{x You can't sell $T here.",
				 ch, NULL,
				 flag_string(item_types,
					     obj->pObjIndex->item_type),
				 TO_CHAR, POS_SLEEPING);
			break;
		}

		price = advatoi(argument);
		if (price == 0) {
			act("Incorrect price.", ch, NULL, NULL, TO_CHAR);
			return;
		}

		price = ((obj->level * 20) >= price ? price : obj->level * 20);
		act_puts3("{D[BLACK MARKET]{x You propose {D$P{x for sale. "
			  "Starting price is $J gold.",
			  ch, NULL, obj, (const void *) price,
			  TO_CHAR, POS_DEAD);

		obj_from_char(obj);

		limit = obj->pObjIndex->limit;
		item = bmitem_new();
		item->seller = str_dup(ch->name);
		item->bet = price;
		item->obj = obj;
		item->next = bmitem_list;
		bmitem_list = item;
		save_black_market();
		return;
	}

	if (!str_cmp(arg, "show")) {
/* show item */
		uint number;
		bool found = FALSE;
		BUFFER *output;

		argument = one_argument(argument, arg, sizeof(arg));
		number = number_argument(arg, arg1, sizeof(arg1));

		for (item = bmitem_list; item != NULL && item->obj != NULL;
				item = item->next)
			if ((arg1[0] == '\0' ||
			     is_name(arg1, item->obj->name))
			&&  !--number) {
				found  = TRUE;
				break;
			}

		if (!found) {
			act ("{D[BLACK MARKET]{x There is no such item in "
			     "sale.", ch, NULL, NULL, TO_CHAR);
			return;
		}

		if (IS_SET(item->obj->pObjIndex->extra_flags, ITEM_NOIDENT)) {
			act("[BLACK MARKET] True nature of the $P is unknown.",
			    ch, NULL, item->obj, TO_CHAR);
			return;
		}

		output = buf_new(-1);
		format_obj(output, item->obj);
		if (!IS_SET(item->obj->extra_flags, ITEM_ENCHANTED))
			format_obj_affects(output,
					   item->obj->pObjIndex->affected,
					   FOA_F_NODURATION);
		format_obj_affects(output, item->obj->affected, 0);
		page_to_char(buf_string(output), ch);
		buf_free(output);
		return;
	}

	if (!str_cmp(arg, "stop")) {
		if (!IS_IMMORTAL(ch)) {
			act("You can't stop selling.", ch, NULL, NULL, TO_CHAR);
			return;
		}
		return;
	}
	do_help(ch, "'BLACK MARKET'");

}

static void
send_notice(CHAR_DATA *victim, OBJ_DATA *obj, int type)
{
	note_t *note;

	note = new_note();
	note->sender = str_dup("{DShrouded figure{x");
	note->to_list = str_dup(victim->name);
	switch (type) {
	case NOTICE_BET:
		note->subject = str_dup("Participation in tenders");
		note->text = str_printf(
		    "Greeting!\n\n"
		    "   We inform you that your bet on {D%s{x\n"
		    "was exceeded.\n",
		    format_short(&obj->short_descr, obj->name, victim));
		break;
	case NOTICE_BUYER:
		note->subject = str_dup("Your new item");
		note->text = str_printf(
		    "Greeting!\n\n"
		    "   We are glad to inform you that black market bagrain\n"
		    "is striked and you are now owner of {D%s{x.\n",
		    format_short(&obj->short_descr, obj->name, victim));
		break;
	default:
		log("[*****] BUG: Unknown type of notice on black market.");
		return;
	}
	note->type = NOTE_NOTE;
	note_post(note);
}
