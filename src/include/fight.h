#ifndef _FIGHT_H_
#define _FIGHT_H_

/*
 * $Id: fight.h,v 1.5 1998-09-01 18:37:57 fjoe Exp $
 */

void raw_kill_org(CHAR_DATA *ch, CHAR_DATA *victim, int part);

#define raw_kill(ch, victim) raw_kill_org(ch, victim, -1)

bool	is_safe 	(CHAR_DATA *ch, CHAR_DATA *victim);
bool	is_safe_nomessage (CHAR_DATA *ch, CHAR_DATA *victim);
bool	is_safe_spell	(CHAR_DATA *ch, CHAR_DATA *victim, bool area);
void	violence_update (void);
void	multi_hit	(CHAR_DATA *ch, CHAR_DATA *victim, int dt);
bool	damage		(CHAR_DATA *ch, CHAR_DATA *victim, int dam,
				int dt, int class, bool show);
bool	damage_old	(CHAR_DATA *ch, CHAR_DATA *victim, int dam,
				int dt, int class, bool show);
void	update_pos	(CHAR_DATA *victim);
void	stop_fighting	(CHAR_DATA *ch, bool fBoth);
bool	can_kill	(CHAR_DATA *ch, CHAR_DATA *victim);
void   gods_protect_msg (CHAR_DATA *ch, CHAR_DATA *victim);
CHAR_DATA *  check_guard     (CHAR_DATA *ch, CHAR_DATA *mob);
void	one_hit 	(CHAR_DATA *ch, CHAR_DATA *victim, int dt,
			 bool secondary);
bool	check_obj_dodge	(CHAR_DATA *ch, CHAR_DATA *victim,
			 OBJ_DATA *obj, int bonus);

#endif
