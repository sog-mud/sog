/*
 * $Id: auction.c,v 1.40 1999-06-21 16:31:54 fjoe Exp $
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "merc.h"
#include "auction.h"

AUCTION_DATA auction = { NULL };

/***************************************************************************
 *  This snippet was orginally written by Erwin S. Andreasen.              *
 *	erwin@pip.dknet.dk, http://pip.dknet.dk/~pip1773/                  *
 *  Adopted to Anatolia MUD by chronos.                                    *
 ***************************************************************************/

void act_auction(const char *fmt,
		 const void *arg1, const void *arg2, const void *arg3,
		 int act_flags, int min_pos)
{
	DESCRIPTOR_DATA *d;

	for (d = descriptor_list; d != NULL; d = d->next) {
		CHAR_DATA *ch = d->original ? d->original : d->character;

		if (d->connected != CON_PLAYING
		||  IS_SET(ch->comm, COMM_NOAUCTION))
			continue;

		act_puts("{YAUCTION{x: ", ch, NULL, NULL,
			 TO_CHAR | ACT_NOLF, min_pos);
		act_puts3(fmt, ch, arg1, arg2, arg3,
			  TO_CHAR | act_flags, min_pos);
	}
}

/*
  This function allows the following kinds of bets to be made:

  Absolute bet
  ============

  bet 14k, bet 50m66, bet 100k

  Relative bet
  ============

  These bets are calculated relative to the current bet. The '+' symbol adds
  a certain number of percent to the current bet. The default is 25, so
  with a current bet of 1000, bet + gives 1250, bet +50 gives 1500 etc.
  Please note that the number must follow exactly after the +, without any
  spaces!

  The '*' or 'x' bet multiplies the current bet by the number specified,
  defaulting to 2. If the current bet is 1000, bet x  gives 2000, bet x10
  gives 10,000 etc.

*/

int advatoi (const char *s)
/*
  util function, converts an 'advanced' ASCII-number-string into a number.
  Used by parsebet() but could also be used by do_give or do_wimpy.

  Advanced strings can contain 'k' (or 'K') and 'm' ('M') in them, not just
  numbers. The letters multiply whatever is left of them by 1,000 and
  1,000,000 respectively. Example:

  14k = 14 * 1,000 = 14,000
  23m = 23 * 1,000,0000 = 23,000,000

  If any digits follow the 'k' or 'm', the are also added, but the number
  which they are multiplied is divided by ten, each time we get one left. This
  is best illustrated in an example :)

  14k42 = 14 * 1000 + 14 * 100 + 2 * 10 = 14420

  Of course, it only pays off to use that notation when you can skip many 0's.
  There is not much point in writing 66k666 instead of 66666, except maybe
  when you want to make sure that you get 66,666.

  More than 3 (in case of 'k') or 6 ('m') digits after 'k'/'m' are automatically
  disregarded. Example:

  14k1234 = 14,123

  If the number contains any other characters than digits, 'k' or 'm', the
  function returns 0. It also returns 0 if 'k' or 'm' appear more than
  once.

*/

{

/* the pointer to buffer stuff is not really necessary, but originally I
   modified the buffer, so I had to make a copy of it. What the hell, it 
   works:) (read: it seems to work:)
*/

  char string[MAX_INPUT_LENGTH]; /* a buffer to hold a copy of the argument */
  char *stringptr = string; /* a pointer to the buffer so we can move around */
  char tempstring[2];       /* a small temp buffer to pass to atoi*/
  int number = 0;           /* number to be returned */
  int multiplier = 0;       /* multiplier used to get the extra digits right */


  strnzcpy(string, sizeof(string), s);        /* working copy */

  while (isdigit (*stringptr)) /* as long as the current character is a digit */
  {
	  strncpy (tempstring,stringptr,1);           /* copy first digit */
	  number = (number * 10) + atoi (tempstring); /* add to current number */
	  stringptr++;                                /* advance */
  }

  switch (UPPER(*stringptr)) {
	  case 'K'  : multiplier = 1000;    number *= multiplier; stringptr++; break;
	  case 'M'  : multiplier = 1000000; number *= multiplier; stringptr++; break;
	  case '\0' : break;
	  default   : return 0; /* not k nor m nor NUL - return 0! */
  }

  while (isdigit (*stringptr) && (multiplier > 1)) /* if any digits follow k/m, add those too */
  {
	  strncpy (tempstring,stringptr,1);           /* copy first digit */
	  multiplier = multiplier / 10;  /* the further we get to right, the less are the digit 'worth' */
	  number = number + (atoi (tempstring) * multiplier);
	  stringptr++;
  }

  if (*stringptr != '\0' && !isdigit(*stringptr)) /* a non-digit character was found, other than NUL */
	return 0; /* If a digit is found, it means the multiplier is 1 - i.e. extra
	             digits that just have to be ignore, liked 14k4443 -> 3 is ignored */


  return (number);
}


int parsebet (const int currentbet, const char *argument)
{
  int newbet = 0;               /* a variable to temporarily hold the new bet */
  char string[MAX_INPUT_LENGTH];/* a buffer to modify the bet string */
  char *stringptr = string;     /* a pointer we can move around */

				/* make a work copy of argument */
  strnzcpy(string, sizeof(string), argument);

  if (*stringptr)               /* check for an empty string */
  {

	if (isdigit (*stringptr)) /* first char is a digit assume e.g. 433k */
	  newbet = advatoi (stringptr); /* parse and set newbet to that value */

	else
	  if (*stringptr == '+') /* add ?? percent */
	  {
	    if (strlen (stringptr) == 1) /* only + specified, assume default */
	      newbet = (currentbet * 125) / 100; /* default: add 25% */
	    else
	      newbet = (currentbet * (100 + atoi (++stringptr))) / 100; /* cut off the first char */
	  }
	  else
	    {
	    if ((*stringptr == '*') || (*stringptr == 'x')) /* multiply */
	      if (strlen (stringptr) == 1) /* only x specified, assume default */
	        newbet = currentbet * 2 ; /* default: twice */
	      else /* user specified a number */
	        newbet = currentbet * atoi (++stringptr); /* cut off the first char */
	    }
  }

  return newbet;        /* return the calculated bet */
}

void auction_give_obj(CHAR_DATA* victim)
{
	int carry_w, carry_n;
	OBJ_DATA *obj = auction.item;

	act("The auctioneer appears before you in a puff of smoke\n"
	    "and hands you $p.", victim, obj, NULL, TO_CHAR);
	act("The auctioneer appears before $n and hands $m $p.",
	    victim, obj, NULL, TO_ROOM);

	if ((carry_w = can_carry_w(victim)) >= 0
	&&  victim->carry_weight + get_obj_weight(obj) > carry_w) {
		act("$p is too heavy for you to carry.",
		    victim, obj, NULL, TO_CHAR);
		act("$n is carrying too much to carry $p and $e drops it.",
		    victim, obj, NULL, TO_ROOM);
		obj_to_room (obj, victim->in_room);
	} else if ((carry_n = can_carry_n(victim)) >= 0
	       &&  victim->carry_number + get_obj_number(obj) > carry_n) {
		act("You can't carry that many items and you drop $p.",
		    victim, obj, NULL, TO_CHAR);
		act("$n is carrying too many items and $e drops $p.",
		    victim, obj, NULL, TO_ROOM);
		obj_to_room (obj, victim->in_room);
	} else {
		obj_to_char (obj, victim);
	}
	auction.item = NULL;
}

void auction_update(void)
{
	if (auction.item == NULL)
		return;

	if (--auction.pulse > 0)
		return;

	auction.pulse = PULSE_AUCTION;
	switch (++auction.going) { /* increase the going state */
	case 1 : /* going once */
	case 2 : /* going twice */
	        if (auction.bet > 0) {
			act_auction("$p: going $T for $J gold.",
				    auction.item, 
				    (auction.going == 1) ? "once" : "twice",
				    (const void*) auction.bet,
				    ACT_FORMSH, POS_RESTING);
	        } else {
	        	act_auction("$p: going $T, starting price $J gold.",
				    auction.item, 
				    (auction.going == 1) ? "once" : "twice",
				    (const void*) auction.starting,
				    ACT_FORMSH, POS_RESTING);
		}
	        break;

	 case 3 : /* SOLD! */
	        if (auction.bet > 0) {
			int tax;
			int pay;

	        	act_auction("$p: sold to $N for $J gold.",
				    auction.item, auction.buyer,
				    (const void*) auction.bet,
				    ACT_FORMSH, POS_RESTING);

			auction_give_obj(auction.buyer);

			pay = (auction.bet * 85) / 100;
			tax = auction.bet - pay;

			 /* give him the money */
			act_puts3("The auctioneer pays you $j gold, "
				  "charging an auction fee of $J gold.",
				  auction.seller, (const void*) pay,
				  NULL, (const void*) tax, TO_CHAR, POS_DEAD);
			auction.seller->pcdata->bank_g += pay;
		} else {
			/* not sold */
	        	act_auction("No bets received for $p.",
				    auction.item, NULL, NULL,
				    ACT_FORMSH, POS_RESTING);
			act_auction("Object has been removed from auction.",
				    NULL, NULL, NULL,
				    ACT_FORMSH, POS_RESTING);
			auction_give_obj(auction.seller);
	        }
        }
} 

void do_auction(CHAR_DATA *ch, const char *argument)
{
	int tax;
	OBJ_DATA *obj;
	char arg1[MAX_INPUT_LENGTH];
	char starting[MAX_INPUT_LENGTH];

	argument = one_argument(argument, arg1, sizeof(arg1));

	if (IS_NPC(ch))    /* NPC can't auction cos it can be extracted! */
		return;

	if (IS_SET(ch->comm, COMM_NOAUCTION)) {
		char_puts("Auction channel is now ON.\n",ch);
		REMOVE_BIT(ch->comm, COMM_NOAUCTION);
	}

	if (arg1[0] == '\0')
		if (auction.item != NULL) {
			/* show item data here */
			if (IS_IMMORTAL(ch)) {
				act_puts("Sold by: $i, Last bet by: $N.", 
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
					 (const void*) auction.bet, NULL,
					 TO_CHAR, POS_DEAD);
				act_puts("No bets on this item have been "
					 "received", ch, NULL, NULL,
					 TO_CHAR, POS_DEAD);
			}
			spell_identify(0, 0, ch, auction.item, 0);
			return;
		}
		else {	
			char_puts("Auction WHAT?\n", ch);
			return;
		}

	if (!str_cmp(arg1, "off")) {
		char_puts("Auction channel is now OFF.\n",ch);
		SET_BIT(ch->comm,COMM_NOAUCTION);
		return;
	}

	if (IS_IMMORTAL(ch) && !str_cmp(arg1, "stop"))
		if (auction.item == NULL) {
			char_puts("There is no auction going on "
				  "you can stop.\n",ch);
			return;
		}
		else { /* stop the auction */
			act_auction("Sale of $p has been stopped by $N.",
				    auction.item, ch, NULL,
				    ACT_FORMSH, POS_DEAD);
			auction_give_obj(auction.seller);

			/* return money to the buyer */
			if (auction.buyer != NULL) {
				auction.buyer->gold += auction.bet;
				act_puts("Your money has been returned.",
					 auction.buyer, NULL, NULL,
					 TO_CHAR, POS_DEAD);
	    		}

			/* return money to the seller */
			if (auction.seller != NULL) {
				auction.seller->gold +=
					(auction.starting * 20) / 100;
				act_puts("Your money has been returned.",
					 auction.seller, NULL, NULL,
					 TO_CHAR, POS_DEAD);
			}
	    		return;
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

	        if (newbet > ch->gold) {
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
		}
		else {
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

	        ch->gold -= newbet; /* substract the gold - important :) */
	        auction.buyer = ch;
	        auction.bet   = newbet;
	        auction.going = 0;
	        auction.pulse = PULSE_AUCTION; /* start the auction over again */

	        act_auction("A bet of $J gold has been received on $p.",
			    auction.item, NULL, (const void*) newbet,
			    ACT_FORMSH, POS_RESTING);
	        return;
	}

	/* finally... */

	obj = get_obj_carry (ch, arg1); /* does char have the item ? */ 

	if (obj == NULL) {
		act_puts("You aren't carrying that.",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		return;
	}

	if (auction.item != NULL) {
		act_puts("Try again later - $p is being auctioned right now!",
			 ch, auction.item, NULL, TO_CHAR, POS_DEAD);
		return;
	}

	argument = one_argument(argument, starting, sizeof(starting));
	if (starting[0] == '\0')
		auction.starting = MIN_START_PRICE;
	else if ((auction.starting = atoi(starting)) < MIN_START_PRICE) {
		act_puts("You must specify the starting price "
			 "(at least $j gold).\n",
			 ch, (const void*) MIN_START_PRICE, NULL,
			 TO_CHAR, POS_DEAD);
		return;
	}

	switch (obj->pIndexData->item_type) {
	default:
		if (!IS_SET(obj->pIndexData->extra_flags, ITEM_CHQUEST)) {
			act_puts("You cannot auction $T.",
				 ch, NULL,
				 flag_string(item_types,
					     obj->pIndexData->item_type),
				 TO_CHAR, POS_SLEEPING);
			break;
		}

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
		tax = (auction.starting * 20) / 100;
		if (ch->gold < tax) {
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
		ch->gold -= tax;

		obj_from_char(obj);
		auction.item = obj;
		auction.bet = 0; 	/* obj->cost / 100 */
		auction.buyer = NULL;
		auction.seller = ch;
		auction.pulse = PULSE_AUCTION;
		auction.going = 0;

		act_auction("A new item has been received: {Y$p{x.",
			    obj, NULL, NULL, ACT_FORMSH, POS_RESTING);
		break;
	} /* switch */
}
