#ifndef _ACT_OBJ_H_
#define _ACT_OBJ_H_

/*
 * $Id: act_obj.h,v 1.1 1998-06-18 05:19:12 fjoe Exp $
 */

bool can_loot		(CHAR_DATA *ch, OBJ_DATA *obj);
void get_obj		(CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *container);
int floating_time	(OBJ_DATA *obj);
bool may_float		(OBJ_DATA *obj);
bool cant_float 	(OBJ_DATA *obj);

#endif
