#ifndef _MOB_PROG_H_
#define _MOB_PROG_H_

/*
 * $Id: mob_prog.h,v 1.1 1998-06-28 04:47:15 fjoe Exp $
 */

void	program_flow	(int vnum, char *source, CHAR_DATA *mob, CHAR_DATA *ch,
				const void *arg1, const void *arg2);
void	mp_act_trigger	(char *argument, CHAR_DATA *mob, CHAR_DATA *ch,
				const void *arg1, const void *arg2, int type);
bool	mp_percent_trigger (CHAR_DATA *mob, CHAR_DATA *ch, 				
				const void *arg1, const void *arg2, int type);
void	mp_bribe_trigger  (CHAR_DATA *mob, CHAR_DATA *ch, int amount);
bool	mp_exit_trigger   (CHAR_DATA *ch, int dir);
void	mp_give_trigger   (CHAR_DATA *mob, CHAR_DATA *ch, OBJ_DATA *obj);
void 	mp_greet_trigger  (CHAR_DATA *ch);
void	mp_hprct_trigger  (CHAR_DATA *mob, CHAR_DATA *ch);

#endif
