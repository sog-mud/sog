#ifndef _QUEST_H_
#define _QUEST_H_

/*
 * $Id: quest.h,v 1.7 1998-06-28 04:47:16 fjoe Exp $
 */

/*
 * Quest obj vnums must take a continuous interval for proper quest generating.
 */
#define QUEST_OBJ_FIRST 84
#define QUEST_OBJ_LAST  87

struct qtrouble_data {
	int vnum;
	int count;
	QTROUBLE_DATA *next;
};

#define IS_ON_QUEST(ch)	(ch->pcdata->questtime > 0)

void quest_handle_death(CHAR_DATA *ch, CHAR_DATA *victim);
void quest_cancel(CHAR_DATA *ch);
void quest_update(void);

int qtrouble_get(CHAR_DATA *ch, int vnum);
void qtrouble_set(CHAR_DATA *ch, int vnumi, int count);

#endif
