#ifndef _RAFFECTS_H_
#define _RAFFECTS_H_

/*
 * $Id: raffect.h,v 1.1 1998-07-03 15:18:46 fjoe Exp $
 */

void	raffect_to_char(ROOM_INDEX_DATA *room, CHAR_DATA *ch);
void	raffect_back_char(ROOM_INDEX_DATA *room, CHAR_DATA *ch);
bool	is_safe_rspell(int level, CHAR_DATA *victim);

#endif
