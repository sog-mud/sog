#ifndef _ACT_COMM_H_
#define _ACT_COMM_H_

void	check_sex	(CHAR_DATA *ch);
void	add_follower	(CHAR_DATA *ch, CHAR_DATA *master);
void	stop_follower	(CHAR_DATA *ch);
void	nuke_pets	(CHAR_DATA *ch);
void	die_follower	(CHAR_DATA *ch);
bool	is_same_group	(CHAR_DATA *ach, CHAR_DATA *bch);
void	do_ilang	(CHAR_DATA *ch, char *argument);
void    do_music	(CHAR_DATA *ch, char *argument);
void    do_gossip	(CHAR_DATA *ch, char *argument);
void	do_rating	(CHAR_DATA *ch, char *argument);

#endif
