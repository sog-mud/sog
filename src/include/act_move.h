#ifndef _ACT_MOVE_H_
#define _ACT_MOVE_H_

/*
 * $Id: act_move.h,v 1.1 1998-06-21 11:38:37 fjoe Exp $
 */

void move_char(CHAR_DATA *ch, int door, bool follow);
char *find_way(CHAR_DATA *ch, ROOM_INDEX_DATA *rstart, ROOM_INDEX_DATA *rend);
int guild_check(CHAR_DATA *ch, ROOM_INDEX_DATA *room);

#endif
