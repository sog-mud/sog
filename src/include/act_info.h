#ifndef _ACT_INFO_H_
#define _ACT_INFO_H_

/*
 * $Id: act_info.h,v 1.4 1998-06-28 04:47:13 fjoe Exp $
 */

void	set_title(CHAR_DATA *ch, char *title,...);
char	*get_cond_alias(OBJ_DATA *obj, CHAR_DATA *ch);
void	do_date(CHAR_DATA*, char*);
bool	check_blind_raw(CHAR_DATA *ch);

#endif
