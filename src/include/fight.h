#ifndef _FIGHT_H_
#define _FIGHT_H_

/*
 * $Id: fight.h,v 1.1 1998-06-02 22:18:26 efdi Exp $
 */

void raw_kill_org(CHAR_DATA *ch, CHAR_DATA *victim, int part);

#define raw_kill(ch, victim) raw_kill_org(ch, victim, -1)

#endif
