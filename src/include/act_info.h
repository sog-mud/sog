#ifndef _ACT_INFO_H_
#define _ACT_INFO_H_

/*
 * $Id: act_info.h,v 1.5 1998-07-11 20:55:08 fjoe Exp $
 */

void	set_title(CHAR_DATA *ch, const char *title);
char	*get_cond_alias(OBJ_DATA *obj, CHAR_DATA *ch);
void	do_date(CHAR_DATA* ch, const char *argument);
bool	check_blind_raw(CHAR_DATA *ch);

#endif
