/*
 * $Id: mob_cmds.c,v 1.45 1999-12-17 12:59:02 fjoe Exp $
 */

/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
*	ROM 2.4 is copyright 1993-1995 Russ Taylor			   *
*	ROM has been brought to you by the ROM consortium		   *
*	    Russ Taylor (rtaylor@pacinfo.com)				   *
*	    Gabrielle Taylor (gtaylor@pacinfo.com)			   *
*	    Brian Moore (rom@rom.efn.org)				   *
*	By using this code, you have agreed to follow the terms of the	   *
*	ROM license, in the file Rom24/doc/rom.license			   *
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  Based on MERC 2.2 MOBprograms by N'Atas-ha.                            *
 *  Written and adapted to ROM 2.4 by                                      *
 *          Markku Nylander (markku.nylander@uta.fi)                       *
 *                                                                         *
 ***************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "mob_cmds.h"
#include "mob_prog.h"
#include "fight.h"

/*
 * Command table.
 */
const	struct	mob_cmd_type	mob_cmd_table	[] =
{
	{	"asound", 	do_mpasound	},
	{	"gecho",	do_mpgecho	},
	{	"zecho",	do_mpzecho	},
	{	"kill",		do_mpkill	},
	{	"attack",	do_mpattack	},
	{	"assist",	do_mpassist	},
	{	"junk",		do_mpjunk	},
	{	"echo",		do_mpecho	},
	{	"echoaround",	do_mpechoaround	},
	{	"echoat",	do_mpechoat	},
	{	"mload",	do_mpmload	},
	{	"oload",	do_mpoload	},
	{	"purge",	do_mppurge	},
	{	"goto",		do_mpgoto	},
	{	"at",		do_mpat		},
	{	"transfer",	do_mptransfer	},
	{	"gtransfer",	do_mpgtransfer	},
	{	"otransfer",	do_mpotransfer	},
	{	"force",	do_mpforce	},
	{	"gforce",	do_mpgforce	},
	{	"vforce",	do_mpvforce	},
	{	"damage",	do_mpdamage	},
	{	"remember",	do_mpremember	},
	{	"forget",	do_mpforget	},
	{	"delay",	do_mpdelay	},
	{	"cancel",	do_mpcancel	},
	{	"call",		do_mpcall	},
	{	"flee",		do_mpflee	},
	{	"remove",	do_mpremove	},
	{	"religion",	do_mpreligion	},
	{	"slay",		do_mpslay	},
	{	str_empty,		0	}
};

/*
 * Mob command interpreter. Implemented separately for security and speed
 * reasons. A trivial hack of interpret()
 */
void mob_interpret(CHAR_DATA *ch, const char *argument)
{
    char command[MAX_INPUT_LENGTH];
    int cmd;

    argument = one_argument(argument, command, sizeof(command));

    /*
     * Look for command in command table.
     */
    for (cmd = 0; mob_cmd_table[cmd].name[0] != '\0'; cmd++)
    {
	if (command[0] == mob_cmd_table[cmd].name[0]
	&&   !str_prefix(command, mob_cmd_table[cmd].name))
	{
	    (*mob_cmd_table[cmd].do_fun) (ch, argument);
	    tail_chain();
	    return;
	}
    }
    log("mob_interpret: invalid cmd from mob %d: '%s'",
	IS_NPC(ch) ? ch->pMobIndex->vnum : 0, command);
}

/*
 * Prints the argument to all active players in the game
 *
 * Syntax: mob gecho [string]
 */
void do_mpgecho(CHAR_DATA *ch, const char *argument)
{
    DESCRIPTOR_DATA *d;

    if (argument[0] == '\0')
    {
	bug("MpGEcho: missing argument from vnum %d",
	    IS_NPC(ch) ? ch->pMobIndex->vnum : 0);
	return;
    }

    for (d = descriptor_list; d; d = d->next)
    {
	if (d->connected == CON_PLAYING)
 	{
	    if (IS_IMMORTAL(d->character))
		char_puts("Mob echo> ", d->character);
	    char_puts(argument, d->character);
	    char_puts("\n", d->character);
	}
    }
}

/*
 * Prints the argument to all players in the same area as the mob
 *
 * Syntax: mob zecho [string]
 */
void do_mpzecho(CHAR_DATA *ch, const char *argument)
{
    DESCRIPTOR_DATA *d;

    if (argument[0] == '\0')
    {
	bug("MpZEcho: missing argument from vnum %d",
	    IS_NPC(ch) ? ch->pMobIndex->vnum : 0);
	return;
    }

    if (ch->in_room == NULL)
	return;

    for (d = descriptor_list; d; d = d->next)
    {
	if (d->connected == CON_PLAYING 
	&&   d->character->in_room != NULL 
	&&   d->character->in_room->area == ch->in_room->area)
 	{
	    if (IS_IMMORTAL(d->character))
		char_puts("Mob echo> ", d->character);
	    char_puts(argument, d->character);
	    char_puts("\n", d->character);
	}
    }
}

/*
 * Prints the argument to all the rooms aroud the mobile
 *
 * Syntax: mob asound [string]
 */
void do_mpasound(CHAR_DATA *ch, const char *argument)
{

    ROOM_INDEX_DATA *was_in_room;
    int              door;

    if (argument[0] == '\0')
	return;

    was_in_room = ch->in_room;
    for (door = 0; door < 6; door++)
    {
    	EXIT_DATA       *pexit;
      
      	if ((pexit = was_in_room->exit[door]) != NULL
	  &&   pexit->to_room.r != NULL
	  &&   pexit->to_room.r != was_in_room)
      	{
	    ch->in_room = pexit->to_room.r;
	    act(argument, ch, NULL, NULL, TO_ROOM | ACT_NOTRIG);
	}
    }
    ch->in_room = was_in_room;
    return;

}

/*
 * Lets the mobile kill any player or mobile without murder
 *
 * Syntax: mob kill [victim]
 */
void do_mpkill(CHAR_DATA *ch, const char *argument)
{
    char      arg[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim;

    one_argument(argument, arg, sizeof(arg));

    if (arg[0] == '\0')
	return;

    if ((victim = get_char_room(ch, arg)) == NULL)
	return;

    if (victim == ch || IS_NPC(victim) || ch->position == POS_FIGHTING)
	return;

    if (IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim)
    {
	bug("MpKill - Charmed mob attacking master from vnum %d.",
	    IS_NPC(ch) ? ch->pMobIndex->vnum : 0);
	return;
    }

    multi_hit(ch, victim, NULL);
}

void do_mpattack(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA* victim;

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') return;
	
	if ((victim=get_char_room(ch,arg)) == NULL) return;
	
	if (victim == ch) return;
	
	multi_hit(ch, victim, NULL);
}

/*
 * Lets the mobile assist another mob or player
 *
 * Syntax: mob assist [character]
 */
void do_mpassist(CHAR_DATA *ch, const char *argument)
{
    char      arg[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim;

    one_argument(argument, arg, sizeof(arg));

    if (arg[0] == '\0')
	return;

    if ((victim = get_char_room(ch, arg)) == NULL)
	return;

    if (victim == ch || ch->fighting != NULL || victim->fighting == NULL ||
	victim->fighting->in_room != ch->in_room)
	return;

    multi_hit(ch, victim->fighting, NULL);
}


/*
 * Lets the mobile destroy an object in its inventory
 * it can also destroy a worn object and it can destroy 
 * items using all.xxxxx or just plain all of them 
 *
 * Syntax: mob junk [item]
 */

void do_mpjunk(CHAR_DATA *ch, const char *argument)
{
    char      arg[ MAX_INPUT_LENGTH ];
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;

    one_argument(argument, arg, sizeof(arg));

    if (arg[0] == '\0')
	return;

    if (str_cmp(arg, "all") && str_prefix("all.", arg))
    {
    	if ((obj = get_obj_wear(ch, arg)) != NULL)
      	{
      	    unequip_char(ch, obj);
	    extract_obj(obj, 0);
    	    return;
      	}
      	if ((obj = get_obj_carry(ch, arg)) == NULL)
	    return; 
	extract_obj(obj, 0);
    }
    else
      	for (obj = ch->carrying; obj != NULL; obj = obj_next)
      	{
            obj_next = obj->next_content;
	    if (arg[3] == '\0' || is_name(&arg[4], obj->name))
            {
          	if (obj->wear_loc != WEAR_NONE)
	    	unequip_char(ch, obj);
          	extract_obj(obj, 0);
            } 
      	}

    return;

}

/*
 * Prints the message to everyone in the room other than the mob and victim
 *
 * Syntax: mob echoaround [victim] [string]
 */

void do_mpechoaround(CHAR_DATA *ch, const char *argument)
{
    char       arg[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim;

    argument = one_argument(argument, arg, sizeof(arg));

    if (arg[0] == '\0')
	return;

    if ((victim=get_char_room(ch, arg)) == NULL)
	return;

    act(argument, ch, NULL, victim, TO_NOTVICT);
}

/*
 * Prints the message to only the victim
 *
 * Syntax: mob echoat [victim] [string]
 */
void do_mpechoat(CHAR_DATA *ch, const char *argument)
{
    char       arg[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim;

    argument = one_argument(argument, arg, sizeof(arg));

    if (arg[0] == '\0' || argument[0] == '\0')
	return;

    if ((victim = get_char_room(ch, arg)) == NULL)
	return;

    act(argument, ch, NULL, victim, TO_VICT);
}

/*
 * Prints the message to the room at large
 *
 * Syntax: mpecho [string]
 */
void do_mpecho(CHAR_DATA *ch, const char *argument)
{
    if (argument[0] == '\0')
	return;
    act(argument, ch, NULL, NULL, TO_ROOM);
}

/*
 * Lets the mobile load another mobile.
 *
 * Syntax: mob mload [vnum]
 */
void do_mpmload(CHAR_DATA *ch, const char *argument)
{
    char            arg[ MAX_INPUT_LENGTH ];
    MOB_INDEX_DATA *pMobIndex;
    CHAR_DATA      *victim;
    int vnum;

    one_argument(argument, arg, sizeof(arg));

    if (ch->in_room == NULL || arg[0] == '\0' || !is_number(arg))
	return;

    vnum = atoi(arg);
    if ((pMobIndex = get_mob_index(vnum)) == NULL)
    {
	log("mpmload: bad mob index (%d) from mob %d",
	    vnum, IS_NPC(ch) ? ch->pMobIndex->vnum : 0);
	return;
    }
    victim = create_mob(pMobIndex);
    char_to_room(victim, ch->in_room);
}

/*
 * Lets the mobile load an object
 *
 * Syntax: mob oload [vnum] {R}
 */
void do_mpoload(CHAR_DATA *ch, const char *argument)
{
    char arg1[ MAX_INPUT_LENGTH ];
    char arg2[ MAX_INPUT_LENGTH ];
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA       *obj;
    bool            fToroom = FALSE, fWear = FALSE;

    argument = one_argument(argument, arg1, sizeof(arg1));
               one_argument(argument, arg2, sizeof(arg2));

    if (arg1[0] == '\0' || !is_number(arg1))
    {
        bug("Mpoload - Bad syntax from vnum %d.",
	    IS_NPC(ch) ? ch->pMobIndex->vnum : 0);
        return;
    }
 
    /*
     * omitted - load to mobile's inventory
     * 'R'     - load to room
     * 'W'     - load to mobile and force wear
     */
    if (arg2[0] == 'R' || arg2[0] == 'r')
	fToroom = TRUE;
    else if (arg2[0] == 'W' || arg2[0] == 'w')
	fWear = TRUE;

    if ((pObjIndex = get_obj_index(atoi(arg1))) == NULL)
    {
	bug("Mpoload - Bad vnum arg from vnum %d.", 
		IS_NPC(ch) ? ch->pMobIndex->vnum : 0);
	return;
    }

    obj = create_obj(pObjIndex, 0);
    if ((fWear || !fToroom) && CAN_WEAR(obj, ITEM_TAKE))
    {
	obj_to_char(obj, ch);
	if (fWear)
	    wear_obj(ch, obj, TRUE);
    }
    else
	obj_to_room(obj, ch->in_room);
}

/*
 * Lets the mobile purge all objects and other npcs in the room,
 * or purge a specified object or mob in the room. The mobile cannot
 * purge itself for safety reasons.
 *
 * syntax mob purge {target}
 */
void do_mppurge(CHAR_DATA *ch, const char *argument)
{
    char       arg[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim;
    OBJ_DATA  *obj;

    one_argument(argument, arg, sizeof(arg));

    if (arg[0] == '\0')
    {
        /* 'purge' */
        CHAR_DATA *vnext;
        OBJ_DATA  *obj_next;

	for (victim = ch->in_room->people; victim != NULL; victim = vnext)
	{
	    vnext = victim->next_in_room;
	    if (IS_NPC(victim) && victim != ch 
	    &&   !IS_SET(victim->pMobIndex->act, ACT_NOPURGE))
		extract_char(victim, 0);
	}

	for (obj = ch->in_room->contents; obj != NULL; obj = obj_next)
	{
	    obj_next = obj->next_content;
	    if (!OBJ_IS(obj, OBJ_NOPURGE))
		extract_obj(obj, 0);
	}

	return;
    }

    if ((victim = get_char_room(ch, arg)) == NULL)
    {
	if ((obj = get_obj_here(ch, arg)))
	{
	    extract_obj(obj, 0);
	}
	else
	{
	    bug("Mppurge - Bad argument from vnum %d.",
		IS_NPC(ch) ? ch->pMobIndex->vnum : 0);
	}
	return;
    }

    if (!IS_NPC(victim))
    {
	bug("Mppurge - Purging a PC from vnum %d.", 
		IS_NPC(ch) ? ch->pMobIndex->vnum : 0);
	return;
    }
    extract_char(victim, 0);
}

/*
 * Lets the mobile goto any location it wishes that is not private.
 *
 * Syntax: mob goto [location]
 */
void do_mpgoto(CHAR_DATA *ch, const char *argument)
{
    char             arg[ MAX_INPUT_LENGTH ];
    ROOM_INDEX_DATA *location;

    one_argument(argument, arg, sizeof(arg));
    if (arg[0] == '\0')
    {
	bug("Mpgoto - No argument from vnum %d.", 
		IS_NPC(ch) ? ch->pMobIndex->vnum : 0);
	return;
    }

    if ((location = find_location(ch, arg)) == NULL)
    {
	bug("Mpgoto - No such location from vnum %d.", 
		IS_NPC(ch) ? ch->pMobIndex->vnum : 0);
	return;
    }

    if (ch->fighting != NULL)
	stop_fighting(ch, TRUE);

    char_from_room(ch);
    char_to_room(ch, location);
}

/* 
 * Lets the mobile do a command at another location.
 *
 * Syntax: mob at [location] [commands]
 */
void do_mpat(CHAR_DATA *ch, const char *argument)
{
    char             arg[ MAX_INPUT_LENGTH ];
    ROOM_INDEX_DATA *location;
    ROOM_INDEX_DATA *original;
    OBJ_DATA 	    *on;

    argument = one_argument(argument, arg, sizeof(arg));

    if (arg[0] == '\0' || argument[0] == '\0')
    {
	bug("Mpat - Bad argument from vnum %d.", 
		IS_NPC(ch) ? ch->pMobIndex->vnum : 0);
	return;
    }

    if ((location = find_location(ch, arg)) == NULL)
    {
	bug("Mpat - No such location from vnum %d.",
		IS_NPC(ch) ? ch->pMobIndex->vnum : 0);
	return;
    }

    original = ch->in_room;
    on = ch->on;
    char_from_room(ch);
    char_to_room(ch, location);
    if (IS_EXTRACTED(ch))
	return;

	interpret(ch, argument);

	if (IS_EXTRACTED(ch))
		return;

	char_from_room(ch);
	char_to_room(ch, original);
	if (!IS_EXTRACTED(ch))
		ch->on = on;
}
 
/*
 * Lets the mobile transfer people.  The 'all' argument transfers
 *  everyone in the current room to the specified location
 *
 * Syntax: mob transfer [target|'all'] [location]
 */
void do_mptransfer(CHAR_DATA *ch, const char *argument)
{
    char             arg1[ MAX_INPUT_LENGTH ];
    char             arg2[ MAX_INPUT_LENGTH ];
    ROOM_INDEX_DATA *location;
    CHAR_DATA       *victim;

    argument = one_argument(argument, arg1, sizeof(arg1));
    argument = one_argument(argument, arg2, sizeof(arg2));

    if (arg1[0] == '\0')
    {
	bug("Mptransfer - Bad syntax from vnum %d.", 
		IS_NPC(ch) ? ch->pMobIndex->vnum : 0);
	return;
    }

    if (!str_cmp(arg1, "all"))
    {
	CHAR_DATA *victim_next;

	for (victim = ch->in_room->people; victim != NULL; victim = victim_next)
	{
	    victim_next = victim->next_in_room;
	    if (!IS_NPC(victim))
		dofun("mob", ch, "transfer %s %s", victim->name, arg2);
	}
	return;
    }

    /*
     * Thanks to Grodyn for the optional location parameter.
     */
    if (arg2[0] == '\0')
    {
	location = ch->in_room;
    }
    else
    {
	if ((location = find_location(ch, arg2)) == NULL)
	{
	    bug("Mptransfer - No such location from vnum %d.",
	        IS_NPC(ch) ? ch->pMobIndex->vnum : 0);
	    return;
	}

	if (room_is_private(location))
	    return;
    }

    if ((victim = get_char_world(ch, arg1)) == NULL)
	return;

    if (victim->in_room == NULL)
	return;

    if (victim->fighting != NULL)
	stop_fighting(victim, TRUE);
    char_from_room(victim);
    char_to_room(victim, location);
    if (!IS_EXTRACTED(victim))
    	dofun("look", victim, "auto");
}

/*
 * Lets the mobile transfer all chars in same group as the victim.
 *
 * Syntax: mob gtransfer [victim] [location]
 */
void do_mpgtransfer(CHAR_DATA *ch, const char *argument)
{
    char             arg1[ MAX_INPUT_LENGTH ];
    char             arg2[ MAX_INPUT_LENGTH ];
    CHAR_DATA       *who, *victim, *victim_next;

    argument = one_argument(argument, arg1, sizeof(arg1));
    argument = one_argument(argument, arg2, sizeof(arg2));

    if (arg1[0] == '\0')
    {
	bug("Mpgtransfer - Bad syntax from vnum %d.", 
		IS_NPC(ch) ? ch->pMobIndex->vnum : 0);
	return;
    }

    if ((who = get_char_room(ch, arg1)) == NULL)
	return;

    for (victim = ch->in_room->people; victim; victim = victim_next)
    {
    	victim_next = victim->next_in_room;
    	if (is_same_group(who, victim))
		dofun("mob", ch, "transfer %s %s", victim->name, arg2);
    }
}

/*
 * Lets the mobile force someone to do something. Must be mortal level
 * and the all argument only affects those in the room with the mobile.
 *
 * Syntax: mob force [victim] [commands]
 */
void do_mpforce(CHAR_DATA *ch, const char *argument)
{
    char arg[ MAX_INPUT_LENGTH ];

    argument = one_argument(argument, arg, sizeof(arg));

    if (arg[0] == '\0' || argument[0] == '\0')
    {
	bug("Mpforce - Bad syntax from vnum %d.", 
		IS_NPC(ch) ? ch->pMobIndex->vnum : 0);
	return;
    }

    if (!str_cmp(arg, "all"))
    {
        CHAR_DATA *vch;
        CHAR_DATA *vch_next;

	for (vch = char_list; vch; vch = vch_next)
	{
	    vch_next = vch->next;

	    if (vch->in_room == ch->in_room
	    &&  IS_TRUSTED(ch, trust_level(vch))
	    &&  can_see(ch, vch))
		interpret(vch, argument);
	}
    }
    else
    {
	CHAR_DATA *victim;

	if ((victim = get_char_room(ch, arg)) == NULL)
	    return;

	if (victim == ch)
	    return;

	interpret(victim, argument);
    }

    return;
}

/*
 * Lets the mobile force a group something. Must be mortal level.
 *
 * Syntax: mob gforce [victim] [commands]
 */
void do_mpgforce(CHAR_DATA *ch, const char *argument)
{
    char arg[ MAX_INPUT_LENGTH ];
    CHAR_DATA *victim, *vch, *vch_next;

    argument = one_argument(argument, arg, sizeof(arg));

    if (arg[0] == '\0' || argument[0] == '\0')
    {
	bug("MpGforce - Bad syntax from vnum %d.", 
		IS_NPC(ch) ? ch->pMobIndex->vnum : 0);
	return;
    }

    if ((victim = get_char_room(ch, arg)) == NULL)
	return;

    if (victim == ch)
	return;

    for (vch = victim->in_room->people; vch != NULL; vch = vch_next)
    {
	vch_next = vch->next_in_room;

	if (is_same_group(victim,vch))
        {
	    interpret(vch, argument);
	}
    }
    return;
}

/*
 * Forces all mobiles of certain vnum to do something (except ch)
 *
 * Syntax: mob vforce [vnum] [commands]
 */
void do_mpvforce(CHAR_DATA *ch, const char *argument)
{
    CHAR_DATA *victim, *victim_next;
    char arg[ MAX_INPUT_LENGTH ];
    int vnum;

    argument = one_argument(argument, arg, sizeof(arg));

    if (arg[0] == '\0' || argument[0] == '\0')
    {
	bug("MpVforce - Bad syntax from vnum %d.", 
		IS_NPC(ch) ? ch->pMobIndex->vnum : 0);
	return;
    }

    if (!is_number(arg))
    {
	bug("MpVforce - Non-number argument vnum %d.", 
		IS_NPC(ch) ? ch->pMobIndex->vnum : 0);
	return;
    }

    vnum = atoi(arg);

    for (victim = npc_list; victim; victim = victim_next)
    {
	victim_next = victim->next;
	if (victim->pMobIndex->vnum == vnum
	&&   ch != victim && victim->fighting == NULL)
	    interpret(victim, argument);
    }
    return;
}

/*
 * Lets mob cause unconditional damage to someone. Nasty, use with caution.
 * Also, this is silent, you must show your own damage message...
 *
 * Syntax: mob damage [victim] [min] [max] {kill}
 */
void do_mpdamage(CHAR_DATA *ch, const char *argument)
{
    CHAR_DATA *victim = NULL, *victim_next;
    char target[ MAX_INPUT_LENGTH ],
	 min[ MAX_INPUT_LENGTH ],
	 max[ MAX_INPUT_LENGTH ];
    int low, high;
    bool fAll = FALSE, fKill = FALSE;

    argument = one_argument(argument, target, sizeof(target));
    argument = one_argument(argument, min, sizeof(min));
    argument = one_argument(argument, max, sizeof(max));

    if (target[0] == '\0')
    {
	bug("MpDamage - Bad syntax from vnum %d.", 
		IS_NPC(ch) ? ch->pMobIndex->vnum : 0);
	return;
    }
    if(!str_cmp(target, "all"))
	fAll = TRUE;
    else if((victim = get_char_room(ch, target)) == NULL)
	return;

    if (is_number(min))
	low = atoi(min);
    else
    {
	bug("MpDamage - Bad damage min vnum %d.", 
		IS_NPC(ch) ? ch->pMobIndex->vnum : 0);
	return;
    }
    if (is_number(max))
	high = atoi(max);
    else
    {
	bug("MpDamage - Bad damage max vnum %d.", 
		IS_NPC(ch) ? ch->pMobIndex->vnum : 0);
	return;
    }
    one_argument(argument, target, sizeof(target));

    /*
     * If kill parameter is omitted, this command is "safe" and will not
     * kill the victim.
     */

    if (target[0] != '\0')
	fKill = TRUE;
    if (fAll)
    {
	for(victim = ch->in_room->people; victim; victim = victim_next)
	{
	    victim_next = victim->next_in_room;
	    if (victim != ch)
    		damage(victim, victim, 
		    fKill ? 
		    number_range(low,high) : UMIN(victim->hit,number_range(low,high)),
	        NULL, DAM_NONE, DAMF_NONE);
	}
    }
    else
    	damage(victim, victim, 
	    fKill ? 
	    number_range(low,high) : UMIN(victim->hit,number_range(low,high)),
        NULL, DAM_NONE, DAMF_NONE);
    return;
}

/*
 * Lets the mobile to remember a target. The target can be referred to
 * with $q and $Q codes in MOBprograms. See also "mob forget".
 *
 * Syntax: mob remember [victim]
 */
void do_mpremember(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] != '\0')
		NPC(ch)->mprog_target = get_char_world(ch, arg);
	else {
		bug("do_mpremember: missing argument from vnum %d.", 
		    IS_NPC(ch) ? ch->pMobIndex->vnum : 0);
	}
}

/*
 * Reverse of "mob remember".
 *
 * Syntax: mob forget
 */
void do_mpforget(CHAR_DATA *ch, const char *argument)
{
	NPC(ch)->mprog_target = NULL;
}

/*
 * Sets a delay for MOBprogram execution. When the delay time expires,
 * the mobile is checked for a MObprogram with DELAY trigger, and if
 * one is found, it is executed. Delay is counted in PULSE_MOBILE
 *
 * Syntax: mob delay [pulses]
 */
void do_mpdelay(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];

	one_argument(argument, arg, sizeof(arg));
	if (!is_number(arg)) {
		bug("do_mpdelay: invalid arg from vnum %d.", 
		    IS_NPC(ch) ? ch->pMobIndex->vnum : 0);
		return;
	}
	NPC(ch)->mprog_delay = atoi(arg);
}

/*
 * Reverse of "mob delay", deactivates the timer.
 *
 * Syntax: mob cancel
 */
void do_mpcancel(CHAR_DATA *ch, const char *argument)
{
	NPC(ch)->mprog_delay = -1;
}
/*
 * Lets the mobile to call another MOBprogram withing a MOBprogram.
 * This is a crude way to implement subroutines/functions. Beware of
 * nested loops and unwanted triggerings... Stack usage might be a problem.
 * Characters and objects referred to must be in the same room with the
 * mobile.
 *
 * Syntax: mob call [vnum] [victim|'null'] [object1|'null'] [object2|'null']
 *
 */
void do_mpcall(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *vch;
	OBJ_DATA *obj1, *obj2;
	int vnum;

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		bug("MpCall: missing arguments from vnum %d.", 
			IS_NPC(ch) ? ch->pMobIndex->vnum : 0);
		return;
	}

	vnum = atoi(arg);

	vch = NULL;
	obj1 = obj2 = NULL;
	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] != '\0')
		vch = get_char_room(ch, arg);
	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] != '\0')
		obj1 = get_obj_here(ch, arg);
	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] != '\0')
		obj2 = get_obj_here(ch, arg);
	program_flow(vnum, ch, vch, (void *)obj1, (void *)obj2);
}

/*
 * Forces the mobile to flee.
 *
 * Syntax: mob flee
 *
 */
void do_mpflee(CHAR_DATA *ch, const char *argument)
{
    ROOM_INDEX_DATA *was_in;
    EXIT_DATA *pexit;
    int door, attempt;

    if (ch->fighting != NULL)
	return;

    if ((was_in = ch->in_room) == NULL)
	return;

    for (attempt = 0; attempt < 6; attempt++)
    {
        door = number_door();
        if ((pexit = was_in->exit[door]) == 0
        ||   pexit->to_room.r == NULL
        ||   IS_SET(pexit->exit_info, EX_CLOSED)
        || (IS_NPC(ch)
        &&   IS_SET(pexit->to_room.r->room_flags, ROOM_NOMOB)))
            continue;

        move_char(ch, door, FALSE);
        if (ch->in_room != was_in)
	    return;
    }
}

/*
 * Lets the mobile to transfer an object. The object must be in the same
 * room with the mobile.
 *
 * Syntax: mob otransfer [item name] [location]
 */
void do_mpotransfer(CHAR_DATA *ch, const char *argument)
{
    OBJ_DATA *obj;
    ROOM_INDEX_DATA *location;
    char arg[ MAX_INPUT_LENGTH ];
    char buf[ MAX_INPUT_LENGTH ];

    argument = one_argument(argument, arg, sizeof(arg));
    if (arg[0] == '\0')
    {
	bug("MpOTransfer - Missing argument from vnum %d.", 
		IS_NPC(ch) ? ch->pMobIndex->vnum : 0);
	return;
    }
    one_argument(argument, buf, sizeof(buf));
    if ((location = find_location(ch, buf)) == NULL)
    {
	bug("MpOTransfer - No such location from vnum %d.", 
		IS_NPC(ch) ? ch->pMobIndex->vnum : 0);
	return;
    }
    if ((obj = get_obj_here(ch, arg)) == NULL)
	return;
    if (obj->carried_by == NULL)
	obj_from_room(obj);
    else
    {
	if (obj->wear_loc != WEAR_NONE)
	    unequip_char(ch, obj);
	obj_from_char(obj);
    }
    obj_to_room(obj, location);
}

/*
 * Lets the mobile to strip an object or all objects from the victim.
 * Useful for removing e.g. quest objects from a character.
 *
 * Syntax: mob remove [victim] [object vnum|'all']
 */
void do_mpremove(CHAR_DATA *ch, const char *argument)
{
    CHAR_DATA *victim;
    OBJ_DATA *obj, *obj_next;
    int vnum = 0;
    bool fAll = FALSE;
    char arg[ MAX_INPUT_LENGTH ];

    argument = one_argument(argument, arg, sizeof(arg));
    if ((victim = get_char_room(ch, arg)) == NULL)
	return;

    one_argument(argument, arg, sizeof(arg));
    if (!str_cmp(arg, "all"))
	fAll = TRUE;
    else if (!is_number(arg))
    {
	bug ("MpRemove: Invalid object from vnum %d.", 
		IS_NPC(ch) ? ch->pMobIndex->vnum : 0);
	return;
    }
    else
	vnum = atoi(arg);

    for (obj = victim->carrying; obj; obj = obj_next)
    {
	obj_next = obj->next_content;
	if (fAll || obj->pObjIndex->vnum == vnum)
	{
	     unequip_char(ch, obj);
	     obj_from_char(obj);
	     extract_obj(obj, 0);
	}
    }
}


int lookup_religion_leader (const char *name)
{
   int value;

   for ( value = 0; value < MAX_RELIGION ; value++)
   {
	if (LOWER(name[0]) == LOWER(religion_table[value].leader[0])
	&&  !str_prefix( name,religion_table[value].leader))
	    return value;
   }

   return 0;
} 

void do_mpreligion(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *victim;
	char name[MAX_STRING_LENGTH];
	int chosen = 0;
	bool correct = TRUE;
	int rel;

	argument = one_argument(argument, name, sizeof(name));
	if ((victim = get_char_room(ch, name)) == NULL
	||  IS_NPC(victim))
		return;

	if ((chosen = lookup_religion_leader(argument)) == 0)
		return;

	rel = PC(victim)->religion;
	if (rel > 0 && rel < MAX_RELIGION) {
		act_say(ch, "You are already in the way of $t.",
			religion_table[rel].leader);
		return;
	}

	switch(chosen) {
	case RELIGION_ATUM_RA:
		if (!IS_GOOD(victim) || victim->ethos != ETHOS_LAWFUL)
			correct = FALSE;
		break;
	case RELIGION_ZEUS:
		if (!IS_GOOD(victim) || victim->ethos != ETHOS_NEUTRAL)
			correct = FALSE;
		break;
	case RELIGION_SIEBELE:
		if (!IS_NEUTRAL(victim) || victim->ethos != ETHOS_NEUTRAL)
			correct = FALSE;
		break;
	case RELIGION_AHURAMAZDA:
		if (!IS_GOOD(victim) || victim->ethos != ETHOS_CHAOTIC)
			correct = FALSE;
		break;
	case RELIGION_EHRUMEN:
		if (!IS_EVIL(victim) || victim->ethos != ETHOS_CHAOTIC)
			correct = FALSE;
		break;
	case RELIGION_DEIMOS:
		if (!IS_EVIL(victim) || victim->ethos != ETHOS_LAWFUL)
			correct = FALSE;
		break;
	case RELIGION_PHOBOS:
		if (!IS_EVIL(victim) || victim->ethos != ETHOS_NEUTRAL)
			correct = FALSE;
		break;
	case RELIGION_ODIN:
		if (!IS_NEUTRAL(victim) || victim->ethos != ETHOS_LAWFUL)
			correct = FALSE;
		break;
	case RELIGION_TESHUB:
		if (!IS_NEUTRAL(victim) || victim->ethos != ETHOS_CHAOTIC)
			correct = FALSE;
		break;
	}

	if (!correct) {
		dofun("say", ch,
		      "That religion doesn't match your ethos and alignment.");
		return;
	}

	PC(victim)->religion = chosen;
	act_say(ch, "From now on and forever, you are in the way of $t.",
		religion_table[chosen].leader);
}

void do_mpslay(CHAR_DATA *ch, const char *argument)
{
	dofun("slay", ch, argument);
}

