#ifndef _ACT_WIZ_H_
#define _ACT_WIZ_H_

/*
 * $Id: act_wiz.h,v 1.1 1998-04-14 08:54:27 fjoe Exp $
 */

#define wiznet(string, ch, obj, flag, flag_skip, min_level) \
		wiznet_printf(ch, obj, flag, flag_skip, min_level, string)
void wiznet_printf(CHAR_DATA *ch, OBJ_DATA *obj, long flag, long flag_skip,
		   int min_level, char* format,...);
void reboot_anatolia(void);

#endif
