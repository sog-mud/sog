#ifndef _RATING_H_
#define _RATING_H_

/*
 * $Id: rating.h,v 1.1 1998-06-02 22:18:27 efdi Exp $
 */

void	rating_update(CHAR_DATA* ch, CHAR_DATA* victim);
void	rating_add(char* name, int pc_killed);
void	do_rating(CHAR_DATA* ch, char* arg);

#endif
