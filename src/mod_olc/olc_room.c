/*-
 * Copyright (c) 1998 fjoe <fjoe@iclub.nsu.ru>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id: olc_room.c,v 1.47 1999-05-15 13:01:31 fjoe Exp $
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "olc.h"
#include "db/db.h"

#define EDIT_ROOM(ch, room)	(room = (ROOM_INDEX_DATA*) ch->desc->pEdit)

/*
 * Room Editor Prototypes
 */
DECLARE_OLC_FUN(roomed_create		);
DECLARE_OLC_FUN(roomed_edit		);
DECLARE_OLC_FUN(roomed_touch		);
DECLARE_OLC_FUN(roomed_show		);
DECLARE_OLC_FUN(roomed_list		);

DECLARE_OLC_FUN(roomed_name		);
DECLARE_OLC_FUN(roomed_desc		);
DECLARE_OLC_FUN(roomed_exd		);
DECLARE_OLC_FUN(roomed_north		);
DECLARE_OLC_FUN(roomed_south		);
DECLARE_OLC_FUN(roomed_east		);
DECLARE_OLC_FUN(roomed_west		);
DECLARE_OLC_FUN(roomed_up		);
DECLARE_OLC_FUN(roomed_down		);
DECLARE_OLC_FUN(roomed_mreset		);
DECLARE_OLC_FUN(roomed_oreset		);
DECLARE_OLC_FUN(roomed_heal		);
DECLARE_OLC_FUN(roomed_mana		);
DECLARE_OLC_FUN(roomed_clan		);
DECLARE_OLC_FUN(roomed_owner		);
DECLARE_OLC_FUN(roomed_room		);
DECLARE_OLC_FUN(roomed_sector		);
DECLARE_OLC_FUN(roomed_reset		);
DECLARE_OLC_FUN(roomed_clone		);

olc_cmd_t olc_cmds_room[] =
{
/*	{ command	function			}, */

	{ "create",	roomed_create			},
	{ "edit",	roomed_edit			},
	{ "touch",	roomed_touch			},
	{ "show",	roomed_show			},
	{ "list",	roomed_list			},

	{ "desc",	roomed_desc			},
	{ "exd",	roomed_exd			},
	{ "name",	roomed_name			},
	{ "heal",	roomed_heal			},
	{ "mana",	roomed_mana			},
	{ "clan",	roomed_clan			},
	{ "clone",	roomed_clone			},

	{ "north",	roomed_north			},
	{ "south",	roomed_south			},
	{ "east",	roomed_east			},
	{ "west",	roomed_west			},
	{ "up",		roomed_up			},
	{ "down",	roomed_down			},

/* New reset commands. */
	{ "mreset",	roomed_mreset			},
	{ "oreset",	roomed_oreset			},
	{ "owner",	roomed_owner			},
	{ "room",	roomed_room,	room_flags	},
	{ "sector",	roomed_sector,	sector_types	},
	{ "reset",	roomed_reset			},

	{ "commands",	show_commands			},
	{ "version",	show_version			},

	{ NULL }
};

static bool olced_exit(CHAR_DATA *ch, const char *argument,
		       olc_cmd_t *cmd, int door);
static void add_reset(ROOM_INDEX_DATA *room, RESET_DATA *pReset, int index);

OLC_FUN(roomed_create)
{
	AREA_DATA *pArea;
	ROOM_INDEX_DATA *pRoom;
	int value;
	int iHash;
	char arg[MAX_STRING_LENGTH];
	
	one_argument(argument, arg, sizeof(arg));
	value = atoi(arg);
	if (!value) {
		do_help(ch, "'OLC CREATE'");
		return FALSE;
	}

	pArea = area_vnum_lookup(value);
	if (!pArea) {
		char_puts("RoomEd: Vnum is not assigned an area.\n", ch);
		return FALSE;
	}

	if (!IS_BUILDER(ch, pArea)) {
        	char_puts("RoomEd: Insufficient security.\n", ch);
		return FALSE;
	}

	if (get_room_index(value)) {
		char_puts("RoomEd: Vnum already exists.\n", ch);
		return FALSE;
	}

	pRoom			= new_room_index();
	pRoom->area		= pArea;
	pRoom->vnum		= value;

	if (value > top_vnum_room)
		 top_vnum_room	= value;

	iHash			= value % MAX_KEY_HASH;
	pRoom->next		= room_index_hash[iHash];
	room_index_hash[iHash]	= pRoom;

	ch->desc->pEdit		= (void *)pRoom;
	OLCED(ch)		= olced_lookup(ED_ROOM);
	touch_area(pArea);
	char_puts("RoomEd: Room created.\n", ch);
	return FALSE;
}

OLC_FUN(roomed_edit)
{
	char arg[MAX_STRING_LENGTH];
	ROOM_INDEX_DATA *pRoom;

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0')
		pRoom = ch->in_room;
	else if (!is_number(arg)) {
		do_help(ch, "'OLC EDIT'");
		return FALSE;
	}
	else if ((pRoom = get_room_index(atoi(arg))) == NULL) {
		char_puts("RoomEd: Vnum does not exist.\n", ch);
		return FALSE;
	}

	roomed_edit_room(ch, pRoom, FALSE);
	return FALSE;
}

OLC_FUN(roomed_touch)
{
	ROOM_INDEX_DATA *pRoom;
	EDIT_ROOM(ch, pRoom);
	return touch_vnum(pRoom->vnum);
}

OLC_FUN(roomed_show)
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_STRING_LENGTH];
	ROOM_INDEX_DATA	*pRoom;
	BUFFER *	output;
	OBJ_DATA	*obj;
	CHAR_DATA	*rch;
	int		door;
	bool		fcnt;
	clan_t	*clan;
	
	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		if (IS_EDIT(ch, ED_ROOM))
			EDIT_ROOM(ch, pRoom);
		else
			pRoom = ch->in_room;
	}
	else if (!is_number(arg)) {
		do_help(ch, OLCED(ch) ? "'OLC EDIT'" : "'OLC ASHOW'");
		return FALSE;
	}
	else if ((pRoom = get_room_index(atoi(arg))) == NULL) {
		char_puts("RoomEd: Vnum does not exist.\n", ch);
		return FALSE;
	}

	output = buf_new(-1);
	
	buf_add(output, "Description:\n");
	mlstr_dump(output, str_empty, pRoom->description);
	mlstr_dump(output, "Name:       ", pRoom->name);
	buf_printf(output, "Area:       [%5d] %s\n",
		   pRoom->area->vnum, pRoom->area->name);
	buf_printf(output, "Vnum:       [%5d]\nSector:     [%s]\n",
		   pRoom->vnum, flag_string(sector_types, pRoom->sector_type));

	if (pRoom->clan && (clan = clan_lookup(pRoom->clan))) 
		buf_printf(output, "Clan      : [%s]\n", clan->name);

	buf_printf(output, "Room flags: [%s]\n",
		   flag_string(room_flags, pRoom->room_flags));

	if (pRoom->heal_rate != 100 || pRoom->mana_rate != 100)
		buf_printf(output, "Health rec: [%d]\nMana rec  : [%d]\n",
			   pRoom->heal_rate, pRoom->mana_rate);

	if (!IS_NULLSTR(pRoom->owner))
		buf_printf(output, "Owner     : [%s]\n", pRoom->owner);

	if (pRoom->ed) {
		ED_DATA *ed;

		buf_add(output, "Desc Kwds:  [");
		for (ed = pRoom->ed; ed != NULL; ed = ed->next) {
			buf_add(output, ed->keyword);
			if (ed->next != NULL)
				buf_add(output, " ");
		}
		buf_add(output, "]\n");
	}

	buf_add(output, "Characters: [");
	fcnt = FALSE;
	for (rch = pRoom->people; rch; rch = rch->next_in_room) {
		one_argument(rch->name, buf, sizeof(buf));
		buf_add(output, buf);
		if (rch->next_in_room != NULL)
			buf_add(output, " ");
		fcnt = TRUE;
	}

	if (fcnt) 
		buf_add(output, "]\n");
	else
		buf_add(output, "none]\n");

	buf_add(output, "Objects:    [");
	fcnt = FALSE;
	for (obj = pRoom->contents; obj; obj = obj->next_content) {
		one_argument(obj->name, buf, sizeof(buf));
		buf_add(output, buf);
		if (obj->next_content != NULL)
			buf_add(output, " ");
		fcnt = TRUE;
	}

	if (fcnt)
		buf_add(output, "]\n");
	else
		buf_add(output, "none]\n");

	for (door = 0; door < MAX_DIR; door++) {
		EXIT_DATA *pexit;

		if ((pexit = pRoom->exit[door]) != NULL) {
			char word[MAX_INPUT_LENGTH];
			char reset_state[MAX_STRING_LENGTH];
			const char *state;
			int i, length;

			buf_printf(output, "-%-5s to [%5d] Key: [%5d] ",
				   dir_name[door],
				   pexit->to_room.r ?
						pexit->to_room.r->vnum : 0,
				   pexit->key);

			/*
			 * Format up the exit info.
			 * Capitalize all flags that are not part of the reset info.
			 */
			strnzcpy(reset_state, sizeof(reset_state),
				 flag_string(exit_flags, pexit->rs_flags));
			state = flag_string(exit_flags, pexit->exit_info);
			buf_add(output, " Exit flags: [");
			for (; ;) {
				state = one_argument(state, word, sizeof(word));

				if (word[0] == '\0') {
					buf_add(output, "]\n");
					break;
				}

				if (str_infix(word, reset_state)) {
					length = strlen(word);
					for (i = 0; i < length; i++)
						word[i] = UPPER(word[i]);
				}
				buf_add(output, word);
				buf_add(output, " ");
			}

			if (!IS_NULLSTR(pexit->keyword))
				buf_printf(output, "Kwds: [%s]\n",
					   pexit->keyword);
			mlstr_dump(output, str_empty, pexit->description);
		}
	}

	char_puts(buf_string(output), ch);
	buf_free(output);
	return FALSE;
}

OLC_FUN(roomed_list)
{
	ROOM_INDEX_DATA	*pRoomIndex;
	AREA_DATA	*pArea;
	BUFFER		*buffer;
	bool found;
	int vnum;
	int  col = 0;

	if ((pArea = get_edited_area(ch)) == NULL)
		pArea = ch->in_room->area;

	buffer = buf_new(-1);
	found   = FALSE;

	for (vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++) {
		if ((pRoomIndex = get_room_index(vnum))) {
			found = TRUE;
			buf_printf(buffer, "[%5d] %-17.16s",
				vnum, mlstr_mval(pRoomIndex->name));
			if (++col % 3 == 0)
				buf_add(buffer, "\n");
		}
	}

	if (!found) 
		char_puts("RoomEd: No rooms in this area.\n", ch);
	else {
		if (col % 3 != 0)
			buf_add(buffer, "\n");

		page_to_char(buf_string(buffer), ch);
	}

	buf_free(buffer);
	return FALSE;
}

OLC_FUN(roomed_north)
{
	return olced_exit(ch, argument, cmd, DIR_NORTH);
}

OLC_FUN(roomed_south)
{
	return olced_exit(ch, argument, cmd, DIR_SOUTH);
}

OLC_FUN(roomed_east)
{
	return olced_exit(ch, argument, cmd, DIR_EAST);
}

OLC_FUN(roomed_west)
{
	return olced_exit(ch, argument, cmd, DIR_WEST);
}

OLC_FUN(roomed_up)
{
	return olced_exit(ch, argument, cmd, DIR_UP);
}

OLC_FUN(roomed_down)
{
	return olced_exit(ch, argument, cmd, DIR_DOWN);
}

OLC_FUN(roomed_exd)
{
	ROOM_INDEX_DATA *pRoom;
	EDIT_ROOM(ch, pRoom);
	return olced_exd(ch, argument, cmd, &pRoom->ed);
}

OLC_FUN(roomed_name)
{
	ROOM_INDEX_DATA *pRoom;
	EDIT_ROOM(ch, pRoom);
	return olced_mlstr(ch, argument, cmd, &pRoom->name);
}

OLC_FUN(roomed_desc)
{
	ROOM_INDEX_DATA *pRoom;
	EDIT_ROOM(ch, pRoom);
	return olced_mlstr_text(ch, argument, cmd, &pRoom->description);
}

OLC_FUN(roomed_heal)
{
	ROOM_INDEX_DATA *pRoom;
	EDIT_ROOM(ch, pRoom);
	return olced_number(ch, argument, cmd, &pRoom->heal_rate);
}       

OLC_FUN(roomed_mana)
{
	ROOM_INDEX_DATA *pRoom;
	EDIT_ROOM(ch, pRoom);
	return olced_number(ch, argument, cmd, &pRoom->mana_rate);
}       

OLC_FUN(roomed_clan)
{
	ROOM_INDEX_DATA *pRoom;
	EDIT_ROOM(ch, pRoom);
	return olced_clan(ch, argument, cmd, &pRoom->clan);
}
	  
#define MAX_MOB	1		/* Default maximum number for resetting mobs */

OLC_FUN(roomed_mreset)
{
	ROOM_INDEX_DATA	*pRoom;
	MOB_INDEX_DATA	*pMobIndex;
	AREA_DATA	*pArea;
	CHAR_DATA	*newmob;
	char		arg [ MAX_INPUT_LENGTH ];
	char		arg2 [ MAX_INPUT_LENGTH ];

	RESET_DATA	*pReset;

	EDIT_ROOM(ch, pRoom);

	argument = one_argument(argument, arg, sizeof(arg));
	argument = one_argument(argument, arg2, sizeof(arg2));

	if (arg[0] == '\0' || !is_number(arg)) {
		char_puts ("Syntax:  mreset <vnum> <max #x> <min #x>\n", ch);
		return FALSE;
	}

	if (!(pMobIndex = get_mob_index(atoi(arg)))) {
		char_puts("RoomEd: No mobile has that vnum.\n", ch);
		return FALSE;
	}

	pArea = area_vnum_lookup(pMobIndex->vnum);
	if (pArea != pRoom->area) {
		char_puts("RoomEd: No such mobile in this area.\n", ch);
		return FALSE;
	}

	/*
	 * Create the mobile reset.
	 */
	pReset              = new_reset_data();
	pReset->command	= 'M';
	pReset->arg1	= pMobIndex->vnum;
	pReset->arg2	= is_number(arg2) ? atoi(arg2) : MAX_MOB;
	pReset->arg3	= pRoom->vnum;
	pReset->arg4	= is_number(argument) ? atoi (argument) : 1;
	add_reset(pRoom, pReset, 0/* Last slot*/);

	/*
	 * Create the mobile.
	 */
	newmob = create_mob(pMobIndex);

	char_printf(ch, "%s (%d) has been loaded and added to resets.\n"
		"There will be a maximum of %d loaded to this room.\n",
		mlstr_mval(pMobIndex->short_descr),
		pMobIndex->vnum,
		pReset->arg2);
	act("$n has created $N!", ch, NULL, newmob, TO_ROOM);
	char_to_room(newmob, pRoom);
	return TRUE;
}

struct wear_type
{
	int	wear_loc;
	int	wear_bit;
};

const struct wear_type wear_table[] =
{
	{ WEAR_NONE,		ITEM_TAKE		},
	{ WEAR_LIGHT,		ITEM_LIGHT		},
	{ WEAR_FINGER_L,	ITEM_WEAR_FINGER	},
	{ WEAR_FINGER_R,	ITEM_WEAR_FINGER	},
	{ WEAR_NECK_1,		ITEM_WEAR_NECK		},
	{ WEAR_NECK_2,		ITEM_WEAR_NECK		},
	{ WEAR_BODY,		ITEM_WEAR_BODY		},
	{ WEAR_HEAD,		ITEM_WEAR_HEAD		},
	{ WEAR_LEGS,		ITEM_WEAR_LEGS		},
	{ WEAR_FEET,		ITEM_WEAR_FEET		},
	{ WEAR_HANDS,		ITEM_WEAR_HANDS		},
	{ WEAR_ARMS,		ITEM_WEAR_ARMS		},
	{ WEAR_SHIELD,		ITEM_WEAR_SHIELD	},
	{ WEAR_ABOUT,		ITEM_WEAR_ABOUT		},
	{ WEAR_WAIST,		ITEM_WEAR_WAIST		},
	{ WEAR_WRIST_L,		ITEM_WEAR_WRIST		},
	{ WEAR_WRIST_R,		ITEM_WEAR_WRIST		},
	{ WEAR_WIELD,		ITEM_WIELD		},
	{ WEAR_HOLD,		ITEM_HOLD		},
	{ WEAR_FLOAT,		ITEM_WEAR_FLOAT		},
	{ WEAR_TATTOO,		ITEM_WEAR_TATTOO	},
	{ WEAR_SECOND_WIELD,	ITEM_WIELD		},
	{ WEAR_STUCK_IN,	ITEM_WIELD		},
	{ WEAR_CLANMARK,	ITEM_WEAR_CLANMARK	},
	{ MAX_WEAR }
};

/*****************************************************************************
 Name:		wear_loc
 Purpose:	Returns the location of the bit that matches the count.
 		1 = first match, 2 = second match etc.
 Called by:	objed_reset(olc_act.c).
 ****************************************************************************/
int wear_loc(int bits, int count)
{
	int flag;
 
	for (flag = 0; wear_table[flag].wear_loc != MAX_WEAR; flag++) {
		 if (IS_SET(bits, wear_table[flag].wear_bit) && --count < 1)
		     return wear_table[flag].wear_loc;
	}
 
	return -1;
}

/*****************************************************************************
 Name:		wear_bit
 Purpose:	Converts a wear_loc into a bit.
 Called by:	roomed_oreset(olc_act.c).
 ****************************************************************************/
int wear_bit(int loc)
{
	int flag;
 
	for (flag = 0; wear_table[flag].wear_loc != MAX_WEAR; flag++) {
		if (loc == wear_table[flag].wear_loc)
			return wear_table[flag].wear_bit;
	}
 
	return 0;
}

OLC_FUN(roomed_oreset)
{
	ROOM_INDEX_DATA	*pRoom;
	OBJ_INDEX_DATA	*pObjIndex;
	AREA_DATA	*pArea;
	OBJ_DATA	*newobj;
	OBJ_DATA	*to_obj;
	CHAR_DATA	*to_mob;
	char		arg1 [ MAX_INPUT_LENGTH ];
	char		arg2 [ MAX_INPUT_LENGTH ];
	int		olevel = 0;

	RESET_DATA	*pReset;

	EDIT_ROOM(ch, pRoom);

	argument = one_argument(argument, arg1, sizeof(arg1));
	argument = one_argument(argument, arg2, sizeof(arg2));

	if (arg1[0] == '\0' || !is_number(arg1)) {
		char_puts ("Syntax:  oreset <vnum> <args>\n", ch);
		char_puts ("        -no_args               = into room\n", ch);
		char_puts ("        -<obj_name>            = into obj\n", ch);
		char_puts ("        -<mob_name> <wear_loc> = into mob\n", ch);
		return FALSE;
	}

	if (!(pObjIndex = get_obj_index(atoi(arg1)))) {
		char_puts("RoomEd: No object has that vnum.\n", ch);
		return FALSE;
	}

	pArea = area_vnum_lookup(pObjIndex->vnum);
	if (pArea != pRoom->area) {
		char_puts("RoomEd: No such object in this area.\n", ch);
		return FALSE;
	}

	/*
	 * Load into room.
	 */
	if (arg2[0] == '\0')
	{
		pReset		= new_reset_data();
		pReset->command	= 'O';
		pReset->arg1	= pObjIndex->vnum;
		pReset->arg2	= 0;
		pReset->arg3	= pRoom->vnum;
		pReset->arg4	= 0;
		add_reset(pRoom, pReset, 0/* Last slot*/);

		newobj = create_obj(pObjIndex, 0);
		obj_to_room(newobj, pRoom);

		char_printf(ch, "%s (%d) has been loaded and added to resets.\n",
			mlstr_mval(pObjIndex->short_descr),
			pObjIndex->vnum);
	}
	else
	/*
	 * Load into object's inventory.
	 */
	if (argument[0] == '\0'
	&& ((to_obj = get_obj_list(ch, arg2, pRoom->contents)) != NULL))
	{
		pReset		= new_reset_data();
		pReset->command	= 'P';
		pReset->arg1	= pObjIndex->vnum;
		pReset->arg2	= 0;
		pReset->arg3	= to_obj->pIndexData->vnum;
		pReset->arg4	= 1;
		add_reset(pRoom, pReset, 0/* Last slot*/);

		newobj = create_obj(pObjIndex, 0);
		newobj->cost = 0;
		obj_to_obj(newobj, to_obj);

		char_printf(ch, "%s (%d) has been loaded into "
			"%s (%d) and added to resets.\n",
			mlstr_mval(newobj->short_descr),
			newobj->pIndexData->vnum,
			mlstr_mval(to_obj->short_descr),
			to_obj->pIndexData->vnum);
	}
	else
	/*
	 * Load into mobile's inventory.
	 */
	if ((to_mob = get_char_room(ch, arg2)) != NULL) {
		int wear_loc;

		if (!str_cmp(argument, "?")) {
			show_flags(ch, wear_loc_flags);
			return FALSE;
		}

		wear_loc = flag_value(wear_loc_flags, argument);

		/*
		 * Disallow loading a sword(WEAR_WIELD) into WEAR_HEAD.
		 */
		if (!IS_SET(pObjIndex->wear_flags, wear_bit(wear_loc))) {
			char_printf(ch, "%s (%d) has wear flags: [%s]\n",
				    mlstr_mval(pObjIndex->short_descr),
				    pObjIndex->vnum,
				    flag_string(wear_flags,
						pObjIndex->wear_flags));
			return FALSE;
		}

		/*
		 * Can't load into same position.
		 */
		if (get_eq_char(to_mob, wear_loc)) {
			char_puts("RoomEd: Object already equipped.\n", ch);
			return FALSE;
		}

		pReset		= new_reset_data();
		pReset->arg1	= pObjIndex->vnum;
		pReset->arg2	= wear_loc;
		if (pReset->arg2 == WEAR_NONE)
			pReset->command = 'G';
		else
			pReset->command = 'E';
		pReset->arg3	= wear_loc;

		add_reset(pRoom, pReset, 0/* Last slot*/);

		olevel  = URANGE(0, to_mob->level - 2, LEVEL_HERO);
		 newobj = create_obj(pObjIndex, 0);

		if (to_mob->pIndexData->pShop) {	/* Shop-keeper? */
			switch (pObjIndex->item_type) {
			default:		olevel = 0;		break;
			case ITEM_PILL:	olevel = number_range( 0, 10);	break;
			case ITEM_POTION:olevel = number_range( 0, 10);	break;
			case ITEM_SCROLL:olevel = number_range( 5, 15);	break;
			case ITEM_WAND:	olevel = number_range(10, 20);	break;
			case ITEM_STAFF:olevel = number_range(15, 25);	break;
			case ITEM_ARMOR:olevel = number_range( 5, 15);	break;
			case ITEM_WEAPON: if (pReset->command == 'G')
					    olevel = number_range(5, 15);
				else
				    olevel = number_fuzzy(olevel);
			break;
			}

			newobj = create_obj(pObjIndex, 0);
			if (pReset->arg2 == WEAR_NONE)
			SET_BIT(newobj->extra_flags, ITEM_INVENTORY);
		}
		else
			newobj = create_obj(pObjIndex, 0);

		obj_to_char(newobj, to_mob);
		if (pReset->command == 'E')
			equip_char(to_mob, newobj, pReset->arg3);

		char_printf(ch, "%s (%d) has been loaded "
			"%s of %s (%d) and added to resets.\n",
			mlstr_mval(pObjIndex->short_descr),
			pObjIndex->vnum,
			flag_string(wear_loc_strings, pReset->arg3),
			mlstr_mval(to_mob->short_descr),
			to_mob->pIndexData->vnum);
	}
	else	/* Display Syntax */
	{
		char_puts("RoomEd: That mobile isn't here.\n", ch);
		return FALSE;
	}

	act("$n has created $p!", ch, newobj, NULL, TO_ROOM);
	return TRUE;
}

OLC_FUN(roomed_room)
{
	ROOM_INDEX_DATA *room;
	EDIT_ROOM(ch, room);
	return olced_flag32(ch, argument, cmd, &room->room_flags);
}

OLC_FUN(roomed_sector)
{
	ROOM_INDEX_DATA *room;
	EDIT_ROOM(ch, room);
	return olced_flag32(ch, argument, cmd, &room->sector_type);
}

OLC_FUN(roomed_owner)
{
	ROOM_INDEX_DATA *pRoom;
	EDIT_ROOM(ch, pRoom);

	if (argument[0] == '\0') {
		char_puts("Syntax:  owner [owner]\n", ch);
		char_puts("         owner none\n", ch);
		return FALSE;
	}

	free_string(pRoom->owner);
	if (!str_cmp(argument, "none"))
		pRoom->owner = str_dup(str_empty);
	else
		pRoom->owner = str_dup(argument);

	char_puts("Owner set.\n", ch);
	return TRUE;
}

OLC_FUN(roomed_reset)
{
	ROOM_INDEX_DATA *pRoom;
	EDIT_ROOM(ch, pRoom);
	reset_room(pRoom, RESET_F_NOPCHECK);
	char_puts("RoomEd: Room reset.\n", ch);
	return FALSE;
}

OLC_FUN(roomed_clone)
{
	ROOM_INDEX_DATA *room;
	ROOM_INDEX_DATA *proto;
	char arg[MAX_INPUT_LENGTH];
	int i;
	bool fAll = FALSE;

	argument = one_argument(argument, arg, sizeof(arg));
	if (!str_cmp(arg, "all")) {
		fAll = TRUE;
		argument = one_argument(argument, arg, sizeof(arg));
	}

	if (!is_number(arg)) {
		do_help(ch, "'OLC ROOM CLONE'");
		return FALSE;
	}

	i = atoi(arg);
	if ((proto = get_room_index(i)) == NULL) {
		char_printf(ch, "RoomEd: %d: Vnum does not exist.\n", i);
		return FALSE;
	}

	EDIT_ROOM(ch, room);
	if (room == proto) {
		char_puts("RoomEd: Huh, cloning from self?\n", ch);
		return FALSE;
	}

	mlstr_free(room->name);
	room->name = mlstr_dup(proto->name);

	mlstr_free(room->description);
	room->description = mlstr_dup(proto->description);

	if (fAll) {
		ed_free(room->ed);
		room->ed = ed_dup(proto->ed);

		free_string(room->owner);
		room->owner = str_dup(proto->owner);

		room->room_flags = proto->room_flags;
		room->sector_type = proto->sector_type;
		room->heal_rate = proto->heal_rate;
		room->mana_rate = proto->mana_rate;
		room->clan = proto->clan;
	}

	return TRUE;
}

void roomed_edit_room(CHAR_DATA *ch, ROOM_INDEX_DATA *pRoom, bool drop_out)
{
	AREA_DATA *pArea = area_vnum_lookup(pRoom->vnum);

	if (!IS_BUILDER(ch, pArea)) {
		char_puts("RoomEd: Insufficient security.\n", ch);
		if (drop_out)
			edit_done(ch->desc);
	       	return;
	}

	ch->desc->pEdit = (void*) pRoom;
	OLCED(ch)	= olced_lookup(ED_ROOM);
}

static bool olced_exit(CHAR_DATA *ch, const char *argument,
		       olc_cmd_t *cmd, int door)
{
	ROOM_INDEX_DATA *pRoom;
	char command[MAX_INPUT_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	int  value;

	EDIT_ROOM(ch, pRoom);

	/*
	 * Set the exit flags, needs full argument.
	 * ----------------------------------------
	 */
	if ((value = flag_value(exit_flags, argument)) != 0) {
		ROOM_INDEX_DATA *pToRoom;
		int rev;                                    /* ROM OLC */

		if (!pRoom->exit[door]) {
		   	char_puts("Exit does not exist.\n",ch);
		   	return FALSE;
		}
		/*   pRoom->exit[door] = new_exit(); */

		/*
		 * This room.
		 */
		TOGGLE_BIT(pRoom->exit[door]->rs_flags,  value);

		/*
		 * Don't toggle exit_info because it can be
		 * changed by players.
		 */
		pRoom->exit[door]->exit_info = pRoom->exit[door]->rs_flags;

		/*
		 * Connected room.
		 */
		rev = rev_dir[door];
		pToRoom = pRoom->exit[door]->to_room.r;     /* ROM OLC */
		if (pToRoom && pToRoom->exit[rev] != NULL) {
			TOGGLE_BIT(pToRoom->exit[rev]->rs_flags,  value);
			TOGGLE_BIT(pToRoom->exit[rev]->exit_info, value);
		}

		char_puts("Exit flag toggled.\n", ch);
		return TRUE;
	}

	/*
	 * Now parse the arguments.
	 */
	argument = one_argument(argument, command, sizeof(command));
	argument = one_argument(argument, arg, sizeof(arg));

	if (command[0] == '\0' && argument[0] == '\0') { /* Move command. */
		move_char(ch, door, TRUE);                    /* ROM OLC */
		return FALSE;
	}

	if (command[0] == '?') {
		BUFFER *output;

		output = buf_new(-1);
		help_show(ch, output, "'OLC EXITS'");
		buf_printf(output, "Valid exit flags are:\n");
		show_flags_buf(output, exit_flags);
		page_to_char(buf_string(output), ch);
		buf_free(output);
		return FALSE;
	}

	if (!str_cmp(command, "delete")) {
		ROOM_INDEX_DATA *pToRoom;
		int rev;                                     /* ROM OLC */
		
		if (!pRoom->exit[door]) {
			char_puts("RoomEd: Cannot delete a null exit.\n", ch);
			return FALSE;
		}

		/*
		 * Remove ToRoom Exit.
		 */
		rev = rev_dir[door];
		pToRoom = pRoom->exit[door]->to_room.r;       /* ROM OLC */
		if (pToRoom && pToRoom->exit[rev]) {
			free_exit(pToRoom->exit[rev]);
			pToRoom->exit[rev] = NULL;
		}

		/*
		 * Remove this exit.
		 */
		free_exit(pRoom->exit[door]);
		pRoom->exit[door] = NULL;

		char_puts("Exit unlinked.\n", ch);
		return TRUE;
	}

	if (!str_cmp(command, "link")) {
		EXIT_DATA *pExit;
		ROOM_INDEX_DATA *pToRoom;

		if (arg[0] == '\0' || !is_number(arg)) {
			do_help(ch, "'OLC EXITS'");
			return FALSE;
		}

		value = atoi(arg);

		if ((pToRoom = get_room_index(value)) == NULL) {
			char_puts("RoomEd: Cannot link to non-existant room.\n", ch);
			return FALSE;
		}

		if (!IS_BUILDER(ch, pToRoom->area)) {
			char_puts("RoomEd: Cannot link to that area.\n", ch);
			return FALSE;
		}

		if (pToRoom->exit[rev_dir[door]]) {
			char_puts("RoomEd: Remote side's exit already exists.\n", ch);
			return FALSE;
		}

		if (!pRoom->exit[door])
			pRoom->exit[door] = new_exit();

		pRoom->exit[door]->to_room.r = pToRoom;
		pRoom->exit[door]->orig_door = door;
		
		door			= rev_dir[door];
		pExit			= new_exit();
		pExit->to_room.r	= pRoom;
		pExit->orig_door	= door;
		pToRoom->exit[door]	= pExit;

		char_puts("Two-way link established.\n", ch);
		return TRUE;
	}
		 
	if (!str_cmp(command, "dig")) {
		char buf[MAX_STRING_LENGTH];
		
		if (arg[0] == '\0' || !is_number(arg)) {
			char_printf(ch, "Syntax: %s dig <vnum>\n",
				    cmd->name);
			return FALSE;
		}
		
		roomed_create(ch, arg, cmd);
		ch->desc->pEdit = pRoom;
		snprintf(buf, sizeof(buf), "link %s", arg);
		olced_exit(ch, buf, cmd, door);
		return TRUE;
	}

	if (!str_cmp(command, "room")) {
		if (arg[0] == '\0' || !is_number(arg)) {
			char_printf(ch, "Syntax: %s room [vnum]\n",
				    cmd->name);
			return FALSE;
		}

		if (!pRoom->exit[door])
			pRoom->exit[door] = new_exit();

		value = atoi(arg);

		if (!get_room_index(value)) {
			char_puts("RoomEd: Cannot link to non-existant room.\n", ch);
			return FALSE;
		}

		pRoom->exit[door]->to_room.r = get_room_index(value);
		pRoom->exit[door]->orig_door = door;

		char_puts("One-way link established.\n", ch);
		return TRUE;
	}

	if (!str_cmp(command, "key")) {
		OBJ_INDEX_DATA *key;

		if (arg[0] == '\0' || !is_number(arg)) {
			char_printf(ch, "Syntax: %s key [vnum]\n",
				    cmd->name);
			return FALSE;
		}

		if (!pRoom->exit[door]) {
			char_puts("RoomEd: Exit does not exist.\n",ch);
			return FALSE;
		}

		value = atoi(arg);

		if ((key = get_obj_index(value)) == NULL) {
			char_puts("RoomEd: Obj doesn't exist.\n", ch);
			return FALSE;
		}

		if (key->item_type != ITEM_KEY) {
			char_puts("RoomEd: That obj is not key.\n", ch);
			return FALSE;
		}

		pRoom->exit[door]->key = value;

		char_puts("Exit key set.\n", ch);
		return TRUE;
	}

	if (!str_cmp(command, "name")) {
		if (arg[0] == '\0') {
			char_printf(ch, "Syntax: %s name [string]\n"
					"        %s name none\n",
				    cmd->name, cmd->name);
			return FALSE;
		}

		if (!pRoom->exit[door]) {
			char_puts("RoomEd: Exit does not exist.\n",ch);
			return FALSE;
		}

		free_string(pRoom->exit[door]->keyword);
		if (!str_cmp(arg, "none"))
			pRoom->exit[door]->keyword = str_dup(str_empty);
		else
			pRoom->exit[door]->keyword = str_dup(arg);

		char_puts("Exit name set.\n", ch);
		return TRUE;
	}

	if (!str_prefix(command, "description")) {
		if (!pRoom->exit[door]) {
			char_puts("RoomEd: Exit does not exist.\n",ch);
			return FALSE;
		}

		if (!mlstr_append(ch, &pRoom->exit[door]->description, arg)) {
			char_printf(ch, "Syntax: %s desc lang\n",
				    cmd->name);
			return FALSE;
		}
		return TRUE;
	}

	return FALSE;
}

void display_resets(CHAR_DATA *ch)
{
    ROOM_INDEX_DATA	*pRoom = ch->in_room;
    RESET_DATA		*pReset;
    MOB_INDEX_DATA	*pMob = NULL;
    BUFFER *		buf;
    int 		iReset = 0;

    buf = buf_new(-1);
    
    buf_add(buf,
  " No.  Loads    Description       Location         Vnum   Mx Mn Description"
  "\n"
  "==== ======== ============= =================== ======== ===== ==========="
  "\n");

    for (pReset = pRoom->reset_first; pReset; pReset = pReset->next) {
	OBJ_INDEX_DATA  *pObj;
	MOB_INDEX_DATA  *pMobIndex;
	OBJ_INDEX_DATA  *pObjIndex;
	OBJ_INDEX_DATA  *pObjToIndex;
	ROOM_INDEX_DATA *pRoomIndex;
	char *start = buf_string(buf);

	buf_printf(buf, "[%2d] ", ++iReset);

	switch (pReset->command)
	{
	default:
	    buf_printf(buf, "Bad reset command: %c.", pReset->command);
	    break;

	case 'M':
	    if (!(pMobIndex = get_mob_index(pReset->arg1))) {
                buf_printf(buf, "Load Mobile - Bad Mob %d\n", pReset->arg1);
                continue;
	    }

	    if (!(pRoomIndex = get_room_index(pReset->arg3))) {
                buf_printf(buf, "Load Mobile - Bad Room %d\n", pReset->arg3);
                continue;
	    }

            pMob = pMobIndex;
            buf_printf(buf, "M[%5d] %-13.13s in room             R[%5d] %2d-%2d %-15.15s\n",
                       pReset->arg1, mlstr_mval(pMob->short_descr),
			pReset->arg3, pReset->arg2, pReset->arg4,
			mlstr_mval(pRoomIndex->name));

	    /*
	     * Check for pet shop.
	     * -------------------
	     */
	    {
		ROOM_INDEX_DATA *pRoomIndexPrev;

		pRoomIndexPrev = get_room_index(pRoomIndex->vnum - 1);
		if (pRoomIndexPrev
		    && IS_SET(pRoomIndexPrev->room_flags, ROOM_PET_SHOP))
                    start[5] = 'P';
	    }

	    break;

	case 'O':
	    if (!(pObjIndex = get_obj_index(pReset->arg1))) {
                buf_printf(buf, "Load Object - Bad Object %d\n",
		    pReset->arg1);
                continue;
	    }

            pObj       = pObjIndex;

	    if (!(pRoomIndex = get_room_index(pReset->arg3))) {
                buf_printf(buf, "Load Object - Bad Room %d\n", pReset->arg3);
                continue;
	    }

            buf_printf(buf, "O[%5d] %-13.13s in room             "
                          "R[%5d]       %-15.15s\n",
                          pReset->arg1, mlstr_mval(pObj->short_descr),
                          pReset->arg3, mlstr_mval(pRoomIndex->name));

	    break;

	case 'P':
	    if (!(pObjIndex = get_obj_index(pReset->arg1))) {
                buf_printf(buf, "Put Object - Bad Object %d\n",
                    pReset->arg1);
                continue;
	    }

            pObj       = pObjIndex;

	    if (!(pObjToIndex = get_obj_index(pReset->arg3))) {
                buf_printf(buf, "Put Object - Bad To Object %d\n",
                    pReset->arg3);
                continue;
	    }

	    buf_printf(buf,
		"O[%5d] %-13.13s inside              O[%5d] %2d-%2d %-15.15s\n",
		pReset->arg1,
		mlstr_mval(pObj->short_descr),
		pReset->arg3,
		pReset->arg2,
		pReset->arg4,
		mlstr_mval(pObjToIndex->short_descr));

	    break;

	case 'G':
	case 'E':
	    if (!(pObjIndex = get_obj_index(pReset->arg1))) {
                buf_printf(buf, "Give/Equip Object - Bad Object %d\n",
                    pReset->arg1);
                continue;
	    }

            pObj       = pObjIndex;

	    if (!pMob) {
                buf_printf(buf, "Give/Equip Object - No Previous Mobile\n");
                break;
	    }

	    if (pMob->pShop) {
	        buf_printf(buf,
		"O[%5d] %-13.13s in the inventory of S[%5d]       %-15.15s\n",
		pReset->arg1,
		mlstr_mval(pObj->short_descr),                           
		pMob->vnum,
		mlstr_mval(pMob->short_descr));
	    }
	    else
	    buf_printf(buf,
		"O[%5d] %-13.13s %-19.19s M[%5d]       %-15.15s\n",
		pReset->arg1,
		mlstr_mval(pObj->short_descr),
		(pReset->command == 'G') ?
		    flag_string(wear_loc_strings, WEAR_NONE)
		  : flag_string(wear_loc_strings, pReset->arg3),
		  pMob->vnum,
		  mlstr_mval(pMob->short_descr));

	    break;

	/*
	 * Doors are set in rs_flags don't need to be displayed.
	 * If you want to display them then uncomment the new_reset
	 * line in the case 'D' in load_resets in db.c and here.
	 */
	case 'D':
	    pRoomIndex = get_room_index(pReset->arg1);
	    buf_printf(buf, "R[%5d] %s door of %-19.19s reset to %s\n",
		pReset->arg1,
		dir_name[pReset->arg2],
		mlstr_mval(pRoomIndex->name),
		flag_string(door_resets, pReset->arg3));

	    break;
	/*
	 * End Doors Comment.
	 */
	case 'R':
	    if (!(pRoomIndex = get_room_index(pReset->arg1))) {
		buf_printf(buf, "Randomize Exits - Bad Room %d\n",
		    pReset->arg1);
		continue;
	    }

	    buf_printf(buf, "R[%5d] Exits are randomized in %s\n",
		pReset->arg1, mlstr_mval(pRoomIndex->name));

	    break;
	}
    }
    char_puts(buf_string(buf), ch);
    buf_free(buf);
}

void do_resets(CHAR_DATA *ch, const char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    char arg4[MAX_INPUT_LENGTH];
    char arg5[MAX_INPUT_LENGTH];
    char arg6[MAX_INPUT_LENGTH];
    char arg7[MAX_INPUT_LENGTH];
    RESET_DATA *pReset = NULL;
	ROOM_INDEX_DATA *pRoom = ch->in_room;


    argument = one_argument(argument, arg1, sizeof(arg1));
    argument = one_argument(argument, arg2, sizeof(arg2));
    argument = one_argument(argument, arg3, sizeof(arg3));
    argument = one_argument(argument, arg4, sizeof(arg4));
    argument = one_argument(argument, arg5, sizeof(arg5));
    argument = one_argument(argument, arg6, sizeof(arg6));
    argument = one_argument(argument, arg7, sizeof(arg7));

    if (!IS_BUILDER(ch, pRoom->area))
    {
	char_puts("Resets: Invalid security for editing this area.\n",
                      ch);
	return;
    }

    /*
     * Display resets in current room.
     * -------------------------------
     */
    if (arg1[0] == '\0')
    {
	if (pRoom->reset_first)
	{
	    char_puts(
		"Resets: M = mobile, R = room, O = object, "
		"P = pet, S = shopkeeper\n", ch);
	    display_resets(ch);
	}
	else
	    char_puts("No resets in this room.\n", ch);
    }


    /*
     * Take index number and search for commands.
     * ------------------------------------------
     */
    if (is_number(arg1))
    {
	/*
	 * Delete a reset.
	 * ---------------
	 */
	if (!str_cmp(arg2, "delete"))
	{
	    int insert_loc = atoi(arg1);

	    if (!pRoom->reset_first)
	    {
		char_puts("No resets in this area.\n", ch);
		return;
	    }

	    if (insert_loc-1 <= 0)
	    {
		pReset = pRoom->reset_first;
		pRoom->reset_first = pRoom->reset_first->next;
		if (!pRoom->reset_first)
		    pRoom->reset_last = NULL;
	    }
	    else
	    {
		int iReset = 0;
		RESET_DATA *prev = NULL;

		for (pReset = pRoom->reset_first;
		  pReset;
		  pReset = pReset->next)
		{
		    if (++iReset == insert_loc)
			break;
		    prev = pReset;
		}

		if (!pReset)
		{
		    char_puts("Reset not found.\n", ch);
		    return;
		}

		if (prev)
		    prev->next = prev->next->next;
		else
		    pRoom->reset_first = pRoom->reset_first->next;

		for (pRoom->reset_last = pRoom->reset_first;
		  pRoom->reset_last->next;
		  pRoom->reset_last = pRoom->reset_last->next);
	    }

	    free_reset_data(pReset);
	    touch_area(pRoom->area);
	    char_puts("Reset deleted.\n", ch);
	}
	else
	/*
	 * Add a reset.
	 * ------------
	 */
	if ((!str_cmp(arg2, "mob") && is_number(arg3))
	  || (!str_cmp(arg2, "obj") && is_number(arg3)))
	{
	    /*
	     * Check for Mobile reset.
	     * -----------------------
	     */
	    if (!str_cmp(arg2, "mob"))
	    {
		if (get_mob_index(is_number(arg3) ? atoi(arg3) : 1) == NULL)
		  {
		    char_puts("Mob no existe.\n",ch);
		    return;
		  }
		pReset = new_reset_data();
		pReset->command = 'M';
		pReset->arg1    = atoi(arg3);
		pReset->arg2    = is_number(arg4) ? atoi(arg4) : 1; /* Max # */
		pReset->arg3    = pRoom->vnum;
		pReset->arg4	= is_number(arg5) ? atoi(arg5) : 1; /* Min # */
	    }
	    else
	    /*
	     * Check for Object reset.
	     * -----------------------
	     */
	    if (!str_cmp(arg2, "obj"))
	    {
		pReset = new_reset_data();
		pReset->arg1    = atoi(arg3);
		/*
		 * Inside another object.
		 * ----------------------
		 */
		if (!str_prefix(arg4, "inside"))
		{
		    OBJ_INDEX_DATA *temp;

		    temp = get_obj_index(is_number(arg5) ? atoi(arg5) : 1);
		    if ((temp->item_type != ITEM_CONTAINER) &&
		         (temp->item_type != ITEM_CORPSE_NPC))
		     {
		       char_puts("Objeto 2 no es container.\n", ch);
		       return;
		     }
		    pReset->command = 'P';
		    pReset->arg2    = is_number(arg6) ? atoi(arg6) : 1;
		    pReset->arg3    = is_number(arg5) ? atoi(arg5) : 1;
		    pReset->arg4    = is_number(arg7) ? atoi(arg7) : 1;
		}
		else
		/*
		 * Inside the room.
		 * ----------------
		 */
		if (!str_cmp(arg4, "room"))
		{
		    if (get_obj_index(atoi(arg3)) == NULL)
		      {
		         char_puts("Vnum no existe.\n",ch);
		         return;
		      }
		    pReset->command  = 'O';
		    pReset->arg2     = 0;
		    pReset->arg3     = pRoom->vnum;
		    pReset->arg4     = 0;
		}
		else {
		/*
		 * Into a Mobile's inventory.
		 * --------------------------
		 */
		    int loc = WEAR_NONE;
		    int vnum;

		    if (str_prefix(arg4, "none")
		    &&  str_prefix(arg4, "inventory")
		    &&  (loc = flag_value(wear_loc_flags, arg4)) < 0) {
				show_flags(ch, wear_loc_flags);
				return;
		    }

		    if (get_obj_index(vnum = atoi(arg3)) == NULL) {
		         char_puts("Vnum no existe.\n",ch);
		         return;
		    }
		    pReset->arg1 = vnum;
		    pReset->arg3 = loc;
		    if (pReset->arg3 == WEAR_NONE)
			pReset->command = 'G';
		    else
			pReset->command = 'E';
		}
	    }
	    add_reset(pRoom, pReset, atoi(arg1));
	    touch_area(pRoom->area);
	    char_puts("Reset added.\n", ch);
	}
	else
	if (!str_cmp(arg2, "random") && is_number(arg3))
	{
		if (atoi(arg3) < 1 || atoi(arg3) > 6)
			{
				char_puts("Invalid argument.\n", ch);
				return;
			}
		pReset = new_reset_data ();
		pReset->command = 'R';
		pReset->arg1 = pRoom->vnum;
		pReset->arg2 = atoi(arg3);
		add_reset(pRoom, pReset, atoi(arg1));
		touch_area(pRoom->area);
		char_puts("Random exits reset added.\n", ch);
	}
	else
	{
	char_puts("Syntax: RESET <number> OBJ <vnum> <wear_loc>\n", ch);
	char_puts("        RESET <number> OBJ <vnum> inside <vnum> [limit] [count]\n", ch);
	char_puts("        RESET <number> OBJ <vnum> room\n", ch);
	char_puts("        RESET <number> MOB <vnum> [max #x area] [max #x room]\n", ch);
	char_puts("        RESET <number> DELETE\n", ch);
	char_puts("        RESET <number> RANDOM [#x exits]\n", ch);
	}
    }
}

/* Static functions */

/*****************************************************************************
 Name:		add_reset
 Purpose:	Inserts a new reset in the given index slot.
 Called by:	do_resets(olc.c).
 ****************************************************************************/
static void add_reset(ROOM_INDEX_DATA *room, RESET_DATA *pReset, int index)
{
    RESET_DATA *reset;
    int iReset = 0;

    if (!room->reset_first)
    {
	room->reset_first	= pReset;
	room->reset_last	= pReset;
	pReset->next		= NULL;
	return;
    }

    index--;

    if (index == 0)	/* First slot (1) selected. */
    {
	pReset->next = room->reset_first;
	room->reset_first = pReset;
	return;
    }

    /*
     * If negative slot(<= 0 selected) then this will find the last.
     */
    for (reset = room->reset_first; reset->next; reset = reset->next)
    {
	if (++iReset == index)
	    break;
    }

    pReset->next	= reset->next;
    reset->next		= pReset;
    if (!pReset->next)
	room->reset_last = pReset;
    return;
}
