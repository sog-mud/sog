/*-
 * Copyright (c) 1998 SoG Development Team
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
 * $Id: olc_room.c,v 1.57.2.12 2001-11-24 14:36:03 avn Exp $
 */

#include "olc.h"

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
DECLARE_OLC_FUN(roomed_heal		);
DECLARE_OLC_FUN(roomed_mana		);
DECLARE_OLC_FUN(roomed_clan		);
DECLARE_OLC_FUN(roomed_room		);
DECLARE_OLC_FUN(roomed_sector		);
DECLARE_OLC_FUN(roomed_clone		);

olc_cmd_t olc_cmds_room[] =
{
/*	{ command	function			}, */

	{ "create",	roomed_create			},
	{ "edit",	roomed_edit			},
	{ "",		NULL				},
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
	{ "room",	roomed_room,	room_flags	},
	{ "sector",	roomed_sector,	sector_types	},

	{ "commands",	show_commands			},
	{ "version",	show_version			},

	{ NULL }
};

static bool olced_exit(CHAR_DATA *ch, const char *argument,
		       olc_cmd_t *cmd, int door);

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
		dofun("help", ch, "'OLC CREATE'");
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

	char_from_room(ch);
	char_to_room(ch, pRoom);
	dofun("look", ch, str_empty);

	ch->desc->pEdit		= (void *)pRoom;
	OLCED(ch)		= olced_lookup(ED_ROOM);
	TOUCH_AREA(pArea);
	char_puts("RoomEd: Room created.\n", ch);
	return FALSE;
}

OLC_FUN(roomed_edit)
{
	char arg[MAX_STRING_LENGTH];
	ROOM_INDEX_DATA *pRoom;
	AREA_DATA *pArea;
	bool drop_out = FALSE;

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0')
		pRoom = ch->in_room;
	else if (!is_number(arg)) {
		if (!str_cmp(arg, "dropout")) {
			drop_out = TRUE;
			pRoom = ch->in_room;
		} else {
			dofun("help", ch, "'OLC EDIT'");
			return FALSE;
		}
	} else if ((pRoom = get_room_index(atoi(arg))) == NULL) {
		char_puts("RoomEd: Vnum does not exist.\n", ch);
		return FALSE;
	}

	pArea = area_vnum_lookup(pRoom->vnum);

	if (!IS_BUILDER(ch, pArea)) {
		char_puts("RoomEd: Insufficient security.\n", ch);
		if (drop_out)
			edit_done(ch->desc);
	       	return FALSE;
	}

	if (ch->in_room != pRoom) {
		char_from_room(ch);
		char_to_room(ch, pRoom);
		dofun("look", ch, str_empty);
	}

	ch->desc->pEdit = (void*) pRoom;
	OLCED(ch)	= olced_lookup(ED_ROOM);

	return FALSE;
}

OLC_FUN(roomed_touch)
{
	ROOM_INDEX_DATA *pRoom;
	EDIT_ROOM(ch, pRoom);
	TOUCH_VNUM(pRoom->vnum);
	return FALSE;
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
		dofun("help", ch, OLCED(ch) ? "'OLC EDIT'" : "'OLC ASHOW'");
		return FALSE;
	}
	else if ((pRoom = get_room_index(atoi(arg))) == NULL) {
		char_puts("RoomEd: Vnum does not exist.\n", ch);
		return FALSE;
	}

	output = buf_new(-1);
	
	buf_add(output, "Description:\n");
	mlstr_dump(output, str_empty, &pRoom->description);
	mlstr_dump(output, "Name:       ", &pRoom->name);
	buf_printf(output, "Area:       [%5d] %s\n",
		   pRoom->area->vnum, pRoom->area->name);
	buf_printf(output, "Vnum:       [%5d]\nSector:     [%s]\n",
		   pRoom->vnum, flag_string(sector_types, pRoom->sector_type));

	if (pRoom->clan && (clan = clan_lookup(pRoom->clan))) 
		buf_printf(output, "Clan      : [%s]\n", clan->name);

	buf_printf(output, "Room flags: [%s]\n",
		   flag_string(room_flags, pRoom->room_flags));

	if (pRoom->affected_by)
		buf_printf(output, "Affected  : [%s]\n",
			   flag_string(raffect_flags, pRoom->affected_by));

	if (pRoom->heal_rate != 100 || pRoom->mana_rate != 100)
		buf_printf(output, "Health rec: [%d]\nMana rec  : [%d]\n",
			   pRoom->heal_rate, pRoom->mana_rate);

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
			mlstr_dump(output, str_empty, &pexit->description);
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
				vnum, mlstr_mval(&pRoomIndex->name));
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
		dofun("help", ch, "'OLC ROOM CLONE'");
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

	mlstr_cpy(&room->name, &proto->name);
	mlstr_cpy(&room->description, &proto->description);

	if (fAll) {
		ed_free(room->ed);
		room->ed = ed_dup(proto->ed);

		room->room_flags = proto->room_flags;
		room->sector_type = proto->sector_type;
		room->heal_rate = proto->heal_rate;
		room->mana_rate = proto->mana_rate;
		room->clan = proto->clan;
	}

	return TRUE;
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
			dofun("help", ch, "'OLC EXITS'");
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
		char_from_room(ch);
		char_to_room(ch, pRoom);
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
	ROOM_INDEX_DATA *in_room = ch->in_room;
	RESET_DATA *r;
	RESET_DATA *last_mob = NULL;
	RESET_DATA *last_obj = NULL;
	BUFFER *buf;
	int rnum = 0;
	static char tab[] = "    ";
	static char tab2[] = "        ";
	int d;

	buf = buf_new(0);

	for (r = in_room->reset_first; r; r = r->next) {
		OBJ_INDEX_DATA *obj;
		MOB_INDEX_DATA *mob;
		ROOM_INDEX_DATA *room;

		buf_printf(buf, "[%2d] ", ++rnum);
		switch (r->command) {
		default:
			buf_printf(buf, "Bad reset command: %c.",
				r->command);
			break;

		case 'M':
			if ((mob = get_mob_index(r->arg1)) == NULL) {
				buf_printf(buf, "<bad mob %d>\n",
					r->arg1);
				break;
			}

			last_mob = r;
			buf_printf(buf, "%c[%5d] %-26.26s [%2d-%2d]\n",
				(room = get_room_index(in_room->vnum-1)) &&
				IS_SET(room->room_flags, ROOM_PET_SHOP) ?
					'P' : 'M',
				r->arg1, mlstr_mval(&mob->short_descr),
				r->arg2, r->arg4);
			break;

		case 'O':
			if ((obj = get_obj_index(r->arg1)) == NULL) {
				buf_printf(buf, "<bad obj %d>\n",
					r->arg1);
				break;
			}

			last_obj = r;
			buf_printf(buf, "O[%5d] %-26.26s\n",
				r->arg1, mlstr_mval(&obj->short_descr));
			break;

		case 'P':
			if (last_obj == NULL) {
				buf_printf(buf, "<no prev obj reset>\n");
				break;
			}

			if ((obj = get_obj_index(r->arg1)) == NULL) {
				buf_printf(buf, "<bad obj %d>\n",
					r->arg1);
				break;
			}

			switch (last_obj->command) {
			case 'E':
			case 'G':
				buf_printf(buf, "%s", tab2);
				break;

			case 'O':
				buf_printf(buf, "%s", tab);
				break;
			}

			buf_printf(buf, "P[%5d] %-26.26s <inside> [%2d-%2d]\n",
				r->arg1, mlstr_mval(&obj->short_descr),
				r->arg2, r->arg4);
			break;

		case 'G':
		case 'E':
			if (last_mob == NULL) {
				buf_printf(buf, "<no prev mob reset>\n");
				break;
			}

			if ((mob = get_mob_index(last_mob->arg1)) == NULL) {
				buf_printf(buf, "<bad mob %d>\n",
					last_mob->arg1);
				break;
			}

			if ((obj = get_obj_index(r->arg1)) == NULL) {
				buf_printf(buf, "<bad obj %d>\n",
					r->arg1);
				break;
			}

			last_obj = r;
			buf_printf(buf, "%s%c[%5d] %-26.26s <%s>\n",
				tab,
				r->command == 'G' && mob->pShop ?
					'S' : r->command,
				r->arg1, mlstr_mval(&obj->short_descr),
				flag_string(wear_loc_strings, r->command == 'G' ?  WEAR_NONE : r->arg3));
			break;

		case 'R':
			buf_printf(buf, "R[%5d] Randomized exits [", r->arg1);
			for (d = 0; d < r->arg2 && d < MAX_DIR; d++) {
				if (d != 0)
					buf_printf(buf, " ");
				buf_printf(buf, "%s", dir_name[d]);
			}
			buf_printf(buf, "%s", "]\n");
			break;
		}
			
	}

	page_to_char(buf_string(buf), ch);
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
	ROOM_INDEX_DATA *room = ch->in_room;


	argument = one_argument(argument, arg1, sizeof(arg1));
	argument = one_argument(argument, arg2, sizeof(arg2));
	argument = one_argument(argument, arg3, sizeof(arg3));
	argument = one_argument(argument, arg4, sizeof(arg4));
	argument = one_argument(argument, arg5, sizeof(arg5));
	argument = one_argument(argument, arg6, sizeof(arg6));

	/*
	 * display resets in current room
	 * ------------------------------
	 */
	if (arg1[0] == '\0') {
		if (room->reset_first)
			display_resets(ch);
		else
			char_puts("No resets in this room.\n", ch);

		return;
	}

	if (!IS_BUILDER(ch, room->area)) {
		char_puts("Resets: Invalid security for editing this area.\n", ch);
		return;
	}

	/*
	 * take index number and search for commands
	 * -----------------------------------------
	 */
	if (!str_cmp(arg1, "?")) {
		dofun("help", ch, "'OLC RESETS'");
		return;
	}

	if (!str_cmp(arg1, "delete")) {
		/*
		 * delete a reset
		 * --------------
		 */
		RESET_DATA *reset, *reset_next;
		int deleted = 0;
		const char *dlt;	/* which reset commands depend on it */

		if (!is_number(arg2)) {
			do_resets(ch, "?");
			return;
		}

		if (room->reset_first == NULL) {
			char_puts("No resets in this room.\n", ch);
			return;
		}

		if ((reset = reset_lookup(room, atoi(arg2))) == NULL) {
			act_puts("$t: no resets with such num in this room.",
				ch, arg2, NULL,
				TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
			return;
		}

		switch (reset->command) {
		case 'M':
			dlt = "GEP";
			break;
		case 'O':
		case 'E':
		case 'G':
			dlt = "P";
			break;
		default:
			dlt = "";
			break;
		}

		do {
			reset_next = reset->next;
			reset_del(room, reset);
			reset_free(reset);
			deleted++;
			reset = reset_next;
		} while (reset != NULL && strchr(dlt, reset->command) != NULL);

		TOUCH_AREA(room->area);
		act_puts("Deleted $j reset(s).", ch, (void*)deleted, NULL,
			TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
		return;
	}

	if (!str_prefix(arg1, "delete")) {
		olced_spell_out(ch, str_empty, NULL);
		return;
	}

	if (!str_prefix(arg1, "mob")) {
		/*
		 * mob reset
		 * ---------
		 */
		int mob_vnum;
		RESET_DATA *mob_reset;
		AREA_DATA *pArea;

		if (arg2[0] == '\0') {
			do_resets(ch, "?");
			return;
		}

		mob_vnum = atoi(arg2);
		if (get_mob_index(mob_vnum) == NULL) {
			act_puts("$t: no mob with such vnum.",
				ch, arg2, NULL,
				TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
			return;
		}

		pArea = area_vnum_lookup(mob_vnum);
		if (!IS_BUILDER(ch, pArea)) {
			char_puts("Insufficient security.\n", ch);
			return;
		}

		mob_reset = reset_new();
		mob_reset->command = 'M';
		mob_reset->arg1 = mob_vnum;
		mob_reset->arg2 = is_number(arg3) ? atoi(arg3) : 1;
		mob_reset->arg3 = room->vnum;
		mob_reset->arg4 = is_number(arg4) ? atoi(arg4) : 1;

		reset_add(room, mob_reset, NULL);
		TOUCH_AREA(room->area);
		char_puts("Mob reset added.\n", ch);
		return;
	}

	if (!str_prefix(arg1, "obj")) {
		/*
		 * obj reset
		 * ---------
		 */
		int obj_vnum;
		RESET_DATA *after;
		RESET_DATA *obj_reset;
		AREA_DATA *pArea;

		if (arg2[0] == '\0') {
			do_resets(ch, "?");
			return;
		}

		obj_vnum = atoi(arg2);
		if (get_obj_index(obj_vnum) == NULL) {
			act_puts("$t: no obj with such vnum.",
				ch, arg2, NULL,
				TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
			return;
		}

		pArea = area_vnum_lookup(obj_vnum);
		if (!IS_BUILDER(ch, pArea)) {
			char_puts("Insufficient security.\n", ch);
			return;
		}

		if (!str_prefix(arg3, "inside")) {
			/*
			 * inside another object
			 * ---------------------
			 */
			OBJ_INDEX_DATA *obj_to;

			if (!is_number(arg4)) {
				do_resets(ch, "?");
				return;
			}

			if ((after = reset_lookup(room, atoi(arg4))) == NULL) {
				char_puts("Previous reset not found.\n", ch);
				return;
			}

			if (after->command != 'E'
			&&  after->command != 'G'
			&&  after->command != 'O') {
				char_puts("Previous reset should be 'E', 'G' or 'O'.\n", ch);
				return;
			}

			if ((obj_to = get_obj_index(after->arg1)) == NULL) {
				act_puts("$j: no such obj.",
					ch, (const void *) after->arg1, NULL,
					TO_CHAR, POS_DEAD);
				return;
			}

			if (obj_to->item_type != ITEM_CONTAINER
			&&  obj_to->item_type != ITEM_CORPSE_NPC) {
				act_puts("obj vnum $j is not a container.",
					ch, (const void *) after->arg1, NULL,
					TO_CHAR | ACT_NOUCASE, POS_DEAD);
				return;
			}

			obj_reset = reset_new();
			obj_reset->command = 'P';
			obj_reset->arg2    = is_number(arg5) ? atoi(arg5) : 1;
			obj_reset->arg3    = obj_to->vnum;
			obj_reset->arg4    = is_number(arg6) ? atoi(arg6) : 1;
		} else if (!str_cmp(arg3, "room")) {
			/*
			 * inside the room
			 * ---------------
			 */
			after = NULL;
			obj_reset = reset_new();
			obj_reset->command = 'O';
			obj_reset->arg3 = room->vnum;
		} else {
			/*
			 * Into mob's inventory.
			 * --------------------------
			 */
			int loc = WEAR_NONE;

			if (!is_number(arg4)) {
				do_resets(ch, "?");
				return;
			}

			if ((after = reset_lookup(room, atoi(arg4))) == NULL) {
				char_puts("Previous reset not found.\n", ch);
				return;
			}

			if (after->command != 'M') {
				char_puts("Previous reset should be 'M'.\n", ch);
				return;
			}

			if (str_prefix(arg3, "none")
			&&  str_prefix(arg3, "inventory")
			&&  (loc = flag_value(wear_loc_flags, arg3)) < 0) {
				show_flags(ch, wear_loc_flags);
				return;
			}

			obj_reset = reset_new();
			obj_reset->arg3 = loc;
			if (obj_reset->arg3 == WEAR_NONE)
				obj_reset->command = 'G';
			else
				obj_reset->command = 'E';
		}

		obj_reset->arg1 = obj_vnum;
		reset_add(room, obj_reset, after);
		TOUCH_AREA(room->area);
		char_puts("Reset added.\n", ch);
		return;
	}

	if (!str_prefix(arg1, "random")) {
		int exit_num = atoi(arg2);
		RESET_DATA *rnd_reset;

		if (arg2[0] == '\0') {
			do_resets(ch, "?");
			return;
		}

		if (exit_num < 1 || exit_num > MAX_DIR) {
			act_puts("$t: Invalid argument.",
				ch, arg2, NULL,
				TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
			return;
		}

		rnd_reset = reset_new();
		rnd_reset->command = 'R';
		rnd_reset->arg1 = room->vnum;
		rnd_reset->arg2 = exit_num;

		reset_add(room, rnd_reset, NULL);
		TOUCH_AREA(room->area);
		char_puts("Random exits reset added.\n", ch);
		return;
	}

	if (!str_prefix(arg1, "now")) {
		reset_room(room, RESET_F_NOPCHECK);
		char_puts("Room reset.\n", ch);
		return;
	}

	if (!str_cmp(arg1, "clear")) {
		RESET_DATA *r, *r_next;

		if (room->reset_first == NULL) {
			char_puts("No resets in this room.\n", ch);
			return;
		}

		for (r = room->reset_first; r; r = r_next) {
			r_next = r->next;
			reset_free(r);
		}
		room->reset_first = room->reset_last = NULL;
		TOUCH_AREA(room->area);

		char_puts("Resets in the room cleared.\n", ch);
		return;
	}

	if (!str_prefix(arg1, "clear")) {
		olced_spell_out(ch, str_empty, NULL);
		return;
	}

	do_resets(ch, "?");
}

