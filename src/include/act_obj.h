#ifndef _ACT_OBJ_H_
#define _ACT_OBJ_H_

/*
 * $Id: act_obj.h,v 1.2 1998-06-28 04:47:13 fjoe Exp $
 */

bool can_loot		(CHAR_DATA *ch, OBJ_DATA *obj);
void get_obj		(CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *container);
int floating_time	(OBJ_DATA *obj);
bool may_float		(OBJ_DATA *obj);
bool cant_float 	(OBJ_DATA *obj);
void wear_obj		(CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace);

#endif
