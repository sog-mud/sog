#ifndef _ACT_WIZ_H_
#define _ACT_WIZ_H_

/*
 * $Id: act_wiz.h,v 1.3 1998-07-11 20:55:09 fjoe Exp $
 */

#define wiznet(string, ch, obj, flag, flag_skip, min_level) \
		wiznet_printf(ch, obj, flag, flag_skip, min_level, string)
void wiznet_printf(CHAR_DATA *ch, OBJ_DATA *obj, long flag, long flag_skip,
		   int min_level, char* format,...);
void reboot_anatolia(void);
ROOM_INDEX_DATA *find_location(CHAR_DATA *ch, const char *argument);

#endif
