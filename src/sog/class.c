/*
 * $Id: class.c,v 1.1 1998-09-01 18:37:57 fjoe Exp $
 */
#include <stdio.h>

#include "merc.h"

varr *	classes;

/*
 * guild_check - == 0 - the room is not a guild
 *		  > 0 - the room is guild and ch is allowed there
 *		  < 0 - the room is guild and ch is not allowed there
 */
int guild_check(CHAR_DATA *ch, ROOM_INDEX_DATA *room)
{
	int class = -1;
	int iClass, iGuild;

	for (iClass = 0; iClass < classes->nused; iClass++) {
		CLASS_DATA *cl = CLASS(iClass);
		for (iGuild = 0; iGuild < cl->guild->nused; iGuild++) {
		    	if (room->vnum == *(int*) VARR_GET(cl->guild, iGuild)) {
				if (iClass == ch->class)
					return 1;
				class = iClass;
			}
		}
	}

	if (class == -1)
		return 0;

	if (IS_IMMORTAL(ch))
		return 1;

	return -1;
}

char *class_name(int vnum)
{
	CLASS_DATA *cl;

	if ((cl = class_lookup(vnum)) == NULL)
		return "Mobile";
	return cl->name;
}

char *class_who_name(int vnum)
{
	CLASS_DATA *cl;

	if ((cl = class_lookup(vnum)) == NULL)
		return "Mob";
	return cl->who_name;
}

/* returns class number */
int cln_lookup(const char *name)
{
	int num;
 
	for (num = 0; num < classes->nused; num++) {
		if (LOWER(name[0]) == LOWER(CLASS(num)->name[0])
		&&  !str_prefix(name, (CLASS(num)->name)))
			return num;
	}
 
	return -1;
}

/* command for retrieving stats */
int get_curr_stat(CHAR_DATA *ch, int stat)
{
	int max;

	if (IS_NPC(ch) || ch->level >= LEVEL_IMMORTAL)
		max = 25;
	else 
		max = UMIN(get_max_train(ch, stat), 25);
  
	return URANGE(3, ch->perm_stat[stat] + ch->mod_stat[stat], max);
}

/* command for returning max training score */
int get_max_train(CHAR_DATA *ch, int stat)
{
	CLASS_DATA *cl;

	if (IS_NPC(ch) || ch->level >= LEVEL_IMMORTAL)
		return 25;

	if ((cl = class_lookup(ch->class)) == NULL)
		return 0;

/* ORG_RACE && RACE serdar*/
	return UMIN(25, (20 + pc_race_table[ORG_RACE(ch)].stats[stat] +
			 cl->stats[stat]));
}

/*   
 * command for returning max training score
 * for do_train and stat2train in comm.c
 */
int get_max_train2(CHAR_DATA *ch, int stat)
{
	CLASS_DATA *cl;

	if (IS_NPC(ch) || ch->level >= LEVEL_IMMORTAL)
		return 25;

	if ((cl = class_lookup(ch->class)) == NULL)
		return 0;

	return UMIN(25, (20 + pc_race_table[ORG_RACE(ch)].stats[stat] + 
			 cl->stats[stat]));
}

bool clan_ok(CHAR_DATA *ch, int sn) 
{
	return TRUE;
}

char *title_lookup(CHAR_DATA *ch)
{
	CLASS_DATA *class;

	if ((class = class_lookup(ch->class)) == NULL
	||  (ch->level < 0 || ch->level > MAX_LEVEL))
		return str_empty;

	return class->titles[ch->level][URANGE(1, ch->sex, 2)-1];
}
