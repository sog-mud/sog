/*
 * $Id: auction_impl.c,v 1.61 2004-02-19 16:55:41 fjoe Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include <ctype.h>
#include <string.h>

#include <merc.h>

#include <sog.h>

#include "auction_impl.h"

DECLARE_UPDATE_FUN(auction_update);

static int advatoi(const char *s);

AUCTION_DATA auction = { NULL, NULL, NULL, 0, 0, 0 };

/***************************************************************************
 *  This snippet was orginally written by Erwin S. Andreasen.              *
 *	erwin@pip.dknet.dk, http://pip.dknet.dk/~pip1773/                  *
 *  Adopted to Anatolia MUD by chronos.                                    *
 ***************************************************************************/

void
act_auction(const char *fmt,
	    const void *arg1, const void *arg2, const void *arg3,
	    int act_flags, int min_pos)
{
	DESCRIPTOR_DATA *d;

	for (d = descriptor_list; d != NULL; d = d->next) {
		CHAR_DATA *ch = d->character;

		if (d->connected != CON_PLAYING
		||  IS_SET(ch->chan, CHAN_NOAUCTION))
			continue;

		act_puts("{YAUCTION{x: ", ch, NULL, NULL,
			 TO_CHAR | ACT_NOLF, min_pos);
		act_puts3(fmt, ch, arg1, arg2, arg3,
			  TO_CHAR | act_flags, min_pos);
	}
}

int
parsebet(const int currentbet, const char *argument)
{
  int newbet = 0;               /* a variable to temporarily hold the new bet */
  char string[MAX_INPUT_LENGTH];/* a buffer to modify the bet string */
  char *stringptr = string;     /* a pointer we can move around */

				/* make a work copy of argument */
  strlcpy(string, argument, sizeof(string));

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
	  } else {
	    if ((*stringptr == '*') || (*stringptr == 'x')) { /* multiply */
	      if (strlen (stringptr) == 1) /* only x specified, assume default */
	        newbet = currentbet * 2 ; /* default: twice */
	      else /* user specified a number */
	        newbet = currentbet * atoi (++stringptr); /* cut off the first char */
	    }
	 }
  }

  return newbet;        /* return the calculated bet */
}

void
auction_give_obj(CHAR_DATA* victim)
{
	OBJ_DATA *obj = auction.item;

	act("The auctioneer appears before you in a puff of smoke\n"
	    "and hands you $p.", victim, obj, NULL, TO_CHAR);
	act("The auctioneer appears before $n and hands $m $p.",
	    victim, obj, NULL, TO_ROOM);
	obj_to_char_check(obj, victim);
	auction.item = NULL;
}

UPDATE_FUN(auction_update)
{
	if (auction.item == NULL)
		return;

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
			PC(auction.seller)->bank_g += pay;
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

/*-------------------------------------------------------------------------
 *  local functions
 */

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

static int
advatoi(const char *s)
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


  strlcpy(string, s, sizeof(string));        /* working copy */

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
