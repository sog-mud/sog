#ifndef _RATING_H_
#define _RATING_H_

/*
 * $Id: rating.h,v 1.2 1998-07-11 20:55:15 fjoe Exp $
 */

void	rating_update(CHAR_DATA* ch, CHAR_DATA* victim);
void	rating_add(char* name, int pc_killed);
void	do_rating(CHAR_DATA* ch, const char *argument);

#endif
