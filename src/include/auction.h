#ifndef _AUCTION_H_
#define _AUCTION_H_

/*
 * $Id: auction.h,v 1.1 1998-07-05 16:30:55 fjoe Exp $
 */

#define MIN_START_PRICE 20

typedef struct auction_data AUCTION_DATA;

struct auction_data
{
	OBJ_DATA  * item;	/* a pointer to the item */
	CHAR_DATA * seller;	/* a pointer to the seller (may NOT quit) */
	CHAR_DATA * buyer;	/* a pointer to the buyer (may NOT quit) */
	int	starting;
	int 	bet;		/* last bet - or 0 if noone has bet anything */
	int	going;		/* 1,2, sold */
	int	pulse;		/* how many pulses (.25 sec) until another
				 * call-out ? */
};

extern AUCTION_DATA auction;

#endif
