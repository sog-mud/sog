#ifndef _QUEST_H_
#define _QUEST_H_

/*
 * $Id: quest.h,v 1.4 1998-06-03 07:27:19 fjoe Exp $
 */

void cancel_quest(CHAR_DATA *ch);

/* quest rewards */
#define QUEST_ITEM1 94
#define QUEST_ITEM2 95
#define QUEST_ITEM3 96

/* The Quests */
#define QUEST_EYE		(B)
#define QUEST_WEAPON		(C)
#define QUEST_GIRTH		(D)
#define QUEST_RING		(E)
#define QUEST_WEAPON2		(F)
#define QUEST_GIRTH2		(G)
#define QUEST_RING2		(H)
#define QUEST_WEAPON3		(I)
#define QUEST_GIRTH3		(J)
#define QUEST_RING3		(K)

/*
 * Quest obj vnums must take a continuous interval for proper quest generating.
 */
#define QUEST_OBJ_FIRST 84
#define QUEST_OBJ_LAST  87

#endif
