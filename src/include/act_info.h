#ifndef _ACT_INFO_H_
#define _ACT_INFO_H_

/*
 * $Id: act_info.h,v 1.3 1998-05-09 12:20:15 fjoe Exp $
 */

void	set_title(CHAR_DATA *ch, char *title,...);
char	*get_cond_alias(OBJ_DATA *obj, CHAR_DATA *ch);
void	do_date(CHAR_DATA*, char*);

#endif
