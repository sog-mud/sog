#ifndef _ACT_COMM_H_
#define _ACT_COMM_H_

/*
 * $Id: act_comm.h,v 1.12 1998-06-30 11:09:49 fjoe Exp $
 */

void		check_sex	(CHAR_DATA *ch);
void		add_follower	(CHAR_DATA *ch, CHAR_DATA *master);
void		stop_follower	(CHAR_DATA *ch);
void		nuke_pets	(CHAR_DATA *ch);
void		die_follower	(CHAR_DATA *ch);
void		do_ilang	(CHAR_DATA *ch, char *argument);
void		do_music	(CHAR_DATA *ch, char *argument);
void		do_gossip	(CHAR_DATA *ch, char *argument);
void		do_rating	(CHAR_DATA *ch, char *argument);
void		do_noiac	(CHAR_DATA *ch, char *argument);
void		do_notelnet	(CHAR_DATA *ch, char *argument);
CHAR_DATA*	leader_lookup	(CHAR_DATA *ch);
void 		do_tell_raw	(CHAR_DATA *ch, CHAR_DATA *victim, char *msg);

#define	is_same_group(ach, bch) (leader_lookup(ach) == leader_lookup(bch))

#endif
