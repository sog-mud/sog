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
 * $Id: olc_room.c,v 1.108 2003-05-08 14:00:10 fjoe Exp $
 */

#include "olc.h"
#include <mprog.h>

#define EDIT_ROOM(ch, room)	(room = (ROOM_INDEX_DATA *) ch->desc->pEdit)

DECLARE_DO_FUN(do_resets);

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
DECLARE_OLC_FUN(roomed_room		);
DECLARE_OLC_FUN(roomed_sector		);
DECLARE_OLC_FUN(roomed_clone		);
DECLARE_OLC_FUN(roomed_trig		);
DECLARE_OLC_FUN(roomed_del		);

olc_cmd_t olc_cmds_room[] =
{
/*	{ command	function					}, */

	{ "create",	roomed_create,	NULL,		NULL		},
	{ "edit",	roomed_edit,	NULL,		NULL		},
	{ "",		NULL,		NULL,		NULL		},
	{ "touch",	roomed_touch,	NULL,		NULL		},
	{ "show",	roomed_show,	NULL,		NULL		},
	{ "list",	roomed_list,	NULL,		NULL		},

	{ "desc",	roomed_desc,	NULL,		NULL		},
	{ "exd",	roomed_exd,	NULL,		NULL		},
	{ "name",	roomed_name,	NULL,		NULL		},
	{ "heal",	roomed_heal,	NULL,		NULL		},
	{ "mana",	roomed_mana,	NULL,		NULL		},
	{ "clone",	roomed_clone,	NULL,		NULL		},

	{ "north",	roomed_north,	NULL,		NULL		},
	{ "south",	roomed_south,	NULL,		NULL		},
	{ "east",	roomed_east,	NULL,		NULL		},
	{ "west",	roomed_west,	NULL,		NULL		},
	{ "up",		roomed_up,	NULL,		NULL		},
	{ "down",	roomed_down,	NULL,		NULL		},

	{ "room",	roomed_room,	NULL,		room_flags	},
	{ "sector",	roomed_sector,	NULL,		sector_types	},
	{ "trig",	roomed_trig,	NULL,		NULL		},

	{ "delete_roo",	olced_spell_out, NULL,		NULL		},
	{ "delete_room", roomed_del,	NULL,		NULL		},

	{ "commands",	show_commands,	NULL,		NULL		},
	{ "version",	show_version,	NULL,		NULL		},

	{ NULL, NULL, NULL, NULL }
};

static bool olced_exit(CHAR_DATA *ch, const char *argument,
		       olc_cmd_t *cmd, int door);
static ROOM_INDEX_DATA *roomed_create_room(CHAR_DATA *ch, const char *arg);

OLC_FUN(roomed_create)
{
	ROOM_INDEX_DATA *pRoom;
	char arg[MAX_STRING_LENGTH];

	one_argument(argument, arg, sizeof(arg));
	if ((pRoom = roomed_create_room(ch, arg)) == NULL)
		return FALSE;
	char_to_room(ch, pRoom);
	if (IS_EXTRACTED(ch))
		return FALSE;
	dofun("look", ch, str_empty);

	ch->desc->pEdit		= (void *)pRoom;
	OLCED(ch)		= olced_lookup(ED_ROOM);
	act_char("RoomEd: Room created.", ch);
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
		} else
			OLC_ERROR("'OLC EDIT'");
	} else if ((pRoom = get_room_index(atoi(arg))) == NULL) {
		act_char("RoomEd: Vnum does not exist.", ch);
		return FALSE;
	}

	pArea = area_vnum_lookup(pRoom->vnum);

	if (!IS_BUILDER(ch, pArea)) {
		act_char("RoomEd: Insufficient security.", ch);
		if (drop_out)
			edit_done(ch->desc);
		return FALSE;
	}

	if (ch->in_room != pRoom) {
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
	char arg[MAX_STRING_LENGTH];
	ROOM_INDEX_DATA	*pRoom;
	BUFFER *	output;
	int		door;

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		if (IS_EDIT(ch, ED_ROOM))
			EDIT_ROOM(ch, pRoom);
		else
			pRoom = ch->in_room;
	} else if (!is_number(arg))
		OLC_ERROR(OLCED(ch) ? "'OLC EDIT'" : "'OLC ASHOW'");
	else if ((pRoom = get_room_index(atoi(arg))) == NULL) {
		act_char("RoomEd: Vnum does not exist.", ch);
		return FALSE;
	}

	output = buf_new(0);

	buf_printf(output, BUF_END, "Vnum:       [%5d]\n", pRoom->vnum);
	buf_printf(output, BUF_END, "Area:       [%5d] %s\n",
		   pRoom->area->vnum, pRoom->area->name);
	mlstr_dump(output, "Name:       ", &pRoom->name, DUMP_LEVEL(ch));

	if (pRoom->ed) {
		ED_DATA *ed;

		buf_append(output, "Desc Kwds:  ");
		for (ed = pRoom->ed; ed != NULL; ed = ed->next)
			buf_printf(output, BUF_END, "[%s]", ed->keyword);

		buf_append(output, "\n");
	}

	buf_append(output, "Description:\n");
	mlstr_dump(output, str_empty, &pRoom->description, DUMP_LEVEL(ch));

	for (door = 0; door < MAX_DIR; door++) {
		EXIT_DATA *pexit;

		if ((pexit = pRoom->exit[door]) != NULL) {
			char word[MAX_INPUT_LENGTH];
			char reset_state[MAX_STRING_LENGTH];
			const char *state;
			int i, length;

			buf_printf(output, BUF_END, "-%-5s to [%5d] Key: [%5d] ",
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
			if (pexit->size != SIZE_GARGANTUAN)
				buf_printf(output, BUF_END, "Exit size: [%s]\n",
					flag_string(size_table, pexit->size));
			buf_append(output, " Exit flags: [");
			for (; ;) {
				state = one_argument(state, word, sizeof(word));

				if (word[0] == '\0') {
					buf_append(output, "]\n");
					break;
				}

				if (str_infix(word, reset_state)) {
					length = strlen(word);
					for (i = 0; i < length; i++)
						word[i] = UPPER(word[i]);
				}
				buf_append(output, word);
				buf_append(output, " ");
			}

			if (!IS_NULLSTR(pexit->keyword)) {
				buf_printf(output, BUF_END, "Kwds:   [%s]\n",
					   pexit->keyword);
			}

			if (!mlstr_null(&pexit->short_descr.ml)) {
				mlstr_dump(output, "Short:  ",
					   &pexit->short_descr.ml,
					   DUMP_LEVEL(ch));
				mlstr_dump(output, "Gender: ",
					   &pexit->short_descr.gender,
					   DUMP_LEVEL(ch));
			}

			buf_append(output, "Exit Description:\n");
			mlstr_dump(output, str_empty, &pexit->description,
				   DUMP_LEVEL(ch));
		}
	}

	if (IN_TRANS_MODE(ch))
		goto bamfout;

	buf_printf(output, BUF_END,
		   "Sector:     [%s]\n",
		   flag_string(sector_types, pRoom->sector_type));

	buf_printf(output, BUF_END, "Room flags: [%s]\n",
		   flag_string(room_flags, pRoom->room_flags));

	if (pRoom->heal_rate != 100 || pRoom->mana_rate != 100) {
		buf_printf(output, BUF_END,
			   "Health rec: [%d]\n"
			   "Mana rec  : [%d]\n",
			   pRoom->heal_rate, pRoom->mana_rate);
	}

	trig_dump_list(&pRoom->mp_trigs, output);

bamfout:
	send_to_char(buf_string(output), ch);
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

	buffer = buf_new(0);
	found   = FALSE;

	for (vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++) {
		if ((pRoomIndex = get_room_index(vnum))) {
			found = TRUE;
			buf_printf(buffer, BUF_END, "[%5d] %-17.16s",
				vnum, mlstr_mval(&pRoomIndex->name));
			if (++col % 3 == 0)
				buf_append(buffer, "\n");
		}
	}

	if (!found)
		act_char("RoomEd: No rooms in this area.", ch);
	else {
		if (col % 3 != 0)
			buf_append(buffer, "\n");

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
	bool rv;
	ROOM_INDEX_DATA *pRoom;
	EDIT_ROOM(ch, pRoom);
	rv = olced_mlstr_text(ch, argument, cmd, &pRoom->description);
	return rv;
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

OLC_FUN(roomed_room)
{
	ROOM_INDEX_DATA *room;
	EDIT_ROOM(ch, room);
	return olced_flag(ch, argument, cmd, &room->room_flags);
}

OLC_FUN(roomed_sector)
{
	ROOM_INDEX_DATA *room;
	EDIT_ROOM(ch, room);
	return olced_flag(ch, argument, cmd, &room->sector_type);
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

	if (!is_number(arg))
		OLC_ERROR("'OLC CLONE'");

	i = atoi(arg);
	if ((proto = get_room_index(i)) == NULL) {
		act_puts("RoomEd: $j: Vnum does not exist.",
			 ch, (const void *) i, NULL, TO_CHAR, POS_DEAD);
		return FALSE;
	}

	EDIT_ROOM(ch, room);
	if (room == proto) {
		act_char("RoomEd: Huh, cloning from self?", ch);
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
	}

	dofun("look", ch, str_empty);
	return TRUE;
}

OLC_FUN(roomed_trig)
{
	ROOM_INDEX_DATA *pRoom;
	EDIT_ROOM(ch, pRoom);
	return olced_trig(
	    ch, argument, cmd, &pRoom->mp_trigs, MP_T_ROOM, pRoom->vnum, pRoom);
}

OLC_FUN(roomed_del)
{
	ROOM_INDEX_DATA *pRoom;
	ROOM_INDEX_DATA *r;
	AFFECT_DATA *paf, *paf_next;

	EDIT_ROOM(ch, pRoom);

	if (olced_busy(ch, ED_ROOM, pRoom, NULL))
		return FALSE;

	if (pRoom->reset_first != NULL) {
		act_char("RoomEd: delete resets in this room first.", ch);
		return FALSE;
	}

	if (pRoom->people != ch
	||  pRoom->people->next_in_room != NULL) {
		act_char("RoomEd: there are other characters in this room.", ch);
		return FALSE;
	}

	if (pRoom->contents != NULL) {
		act_char("RoomEd: move objects from this room away first.", ch);
		return FALSE;
	}

/* remove affects from this room (slightly suboptimal) */
	for (paf = pRoom->affected; paf != NULL; paf = paf_next) {
		paf_next = paf->next;
		affect_remove_room(pRoom, paf);
	}

/* remove from exits */
	C_FOREACH (r, &rooms) {
		int door;

		for (door = 0; door < MAX_DIR; door++) {
			EXIT_DATA *pExit = r->exit[door];

			if (pExit != NULL && pExit->to_room.r == pRoom)
				pExit->to_room.r = NULL;
		}
	}

	act_char("RoomEd: Room index deleted.", ch);
	edit_done(ch->desc);

	char_to_room(ch, get_room_index(ROOM_VNUM_LIMBO));
	dofun("look", ch, str_empty);

	TOUCH_VNUM(pRoom->vnum);
	c_delete(&rooms, pRoom);
	return FALSE;
}

static bool olced_exit(CHAR_DATA *ch, const char *argument,
		       olc_cmd_t *cmd, int door)
{
	ROOM_INDEX_DATA *pRoom;
	char command[MAX_INPUT_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	int  value;

	EDIT_ROOM(ch, pRoom);

	if (argument[0] == '\0') { /* Move command. */
		move_char(ch, door, 0);                    /* ROM OLC */
		return FALSE;
	}

	argument = one_argument(argument, command, sizeof(command));

	if (!str_prefix(command, "flags")) {
		ROOM_INDEX_DATA *pToRoom;
		int rev;

		if (argument[0] == '\0'
		||  !str_cmp(argument, "?")) {
			BUFFER *buf = buf_new(0);
			buf_printf(buf, BUF_END, "Valid exit flags are:\n");
			show_flags_buf(buf, exit_flags);
			page_to_char(buf_string(buf), ch);
			buf_free(buf);
			return FALSE;
		}

		if ((value = flag_value(exit_flags, argument)) == 0) {
			act_puts("RoomEd: $t: no such exit flags.",
				 ch, argument, NULL,
				 TO_CHAR | ACT_NOTRANS, POS_DEAD);
			return FALSE;
		}

		if (!pRoom->exit[door]) {
			act_char("Exit does not exist.", ch);
			return FALSE;
		}
		/*   pRoom->exit[door] = new_exit(); */

		/*
		 * This room.
		 */
		TOGGLE_BIT(pRoom->exit[door]->rs_flags, value);

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

		act_puts("Exit flag '$t' toggled.",
			 ch, flag_string(exit_flags, value), NULL,
			 TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return TRUE;
	}

	if (!str_cmp(command, "short")) {
		bool ok;

		if (!pRoom->exit[door]) {
			act_char("RoomEd: Exit does not exist.", ch);
			return FALSE;
		}

		ok = olced_mlstr(ch, argument, cmd,
				 &pRoom->exit[door]->short_descr.ml);
		if (ok)
			act_char("Short descr set.", ch);
		return ok;
	}

	if (!str_cmp(command, "gender")) {
		bool ok;

		if (!pRoom->exit[door]) {
			act_char("RoomEd: Exit does not exist.", ch);
			return FALSE;
		}

		cmd->arg1 = gender_table;
		ok = olced_gender(ch, argument, cmd,
				  &pRoom->exit[door]->short_descr.gender);
		if (ok)
			act_char("Gender set.", ch);
		return ok;
	}

	if (!str_cmp(command, "name")) {
		bool ok;

		if (argument[0] == '\0') {
			act_puts("Syntax: $t name [string]",
				 ch, cmd->name, NULL,
				 TO_CHAR | ACT_NOTRANS, POS_DEAD);
			act_puts("        $t name none",
				 ch, cmd->name, NULL,
				 TO_CHAR | ACT_NOTRANS, POS_DEAD);
			return FALSE;
		}

		if (!pRoom->exit[door]) {
			act_char("RoomEd: Exit does not exist.", ch);
			return FALSE;
		}

		ok = olced_name(ch, argument, cmd, &pRoom->exit[door]->keyword);

		if (ok)
			act_char("Exit name set.", ch);
		return ok;
	}

	argument = one_argument(argument, arg, sizeof(arg));

	if (command[0] == '?') {
		dofun("help", ch, "'OLC EXITS'");
		return FALSE;
	}

	if (!str_cmp(command, "delete")) {
		ROOM_INDEX_DATA *pToRoom;
		int rev;                                     /* ROM OLC */

		if (!pRoom->exit[door]) {
			act_char("RoomEd: Cannot delete a null exit.", ch);
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

		act_char("Exit unlinked.", ch);
		return TRUE;
	}

	if (!str_cmp(command, "link")) {
		EXIT_DATA *pExit;
		ROOM_INDEX_DATA *pToRoom;

		if (arg[0] == '\0' || !is_number(arg))
			OLC_ERROR("'OLC EXITS'");

		value = atoi(arg);

		if ((pToRoom = get_room_index(value)) == NULL) {
			act_char("RoomEd: Cannot link to non-existant room.", ch);
			return FALSE;
		}

		if (!IS_BUILDER(ch, pToRoom->area)) {
			act_char("RoomEd: Cannot link to that area.", ch);
			return FALSE;
		}

		if (pToRoom->exit[rev_dir[door]]) {
			act_char("RoomEd: Remote side's exit already exists.", ch);
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

		act_char("Two-way link established.", ch);
		return TRUE;
	}

	if (!str_cmp(command, "dig")) {
		char buf[MAX_INPUT_LENGTH];

		if (arg[0] == '\0' || !is_number(arg)) {
			act_puts("Syntax: $t dig <vnum>",
				 ch, cmd->name, NULL,
				 TO_CHAR | ACT_NOTRANS, POS_DEAD);
			return FALSE;
		}

		if (roomed_create_room(ch, arg) == NULL)
			return FALSE;
		snprintf(buf, sizeof(buf), "link %s", arg);
		olced_exit(ch, buf, cmd, door);
		return TRUE;
	}

	if (!str_cmp(command, "room")) {
		if (arg[0] == '\0' || !is_number(arg)) {
			act_puts("Syntax: $t room [vnum]",
				 ch, cmd->name, NULL,
				 TO_CHAR | ACT_NOTRANS, POS_DEAD);
			return FALSE;
		}

		if (!pRoom->exit[door])
			pRoom->exit[door] = new_exit();

		value = atoi(arg);

		if (!get_room_index(value)) {
			act_char("RoomEd: Cannot link to non-existant room.", ch);
			return FALSE;
		}

		pRoom->exit[door]->to_room.r = get_room_index(value);
		pRoom->exit[door]->orig_door = door;

		act_char("One-way link established.", ch);
		return TRUE;
	}

	if (!str_cmp(command, "key")) {
		OBJ_INDEX_DATA *key;

		if (arg[0] == '\0' || !is_number(arg)) {
			act_puts("Syntax: $t key [vnum]",
				 ch, cmd->name, NULL,
				 TO_CHAR | ACT_NOTRANS, POS_DEAD);
			return FALSE;
		}

		if (!pRoom->exit[door]) {
			act_char("RoomEd: Exit does not exist.", ch);
			return FALSE;
		}

		value = atoi(arg);

		if ((key = get_obj_index(value)) == NULL) {
			act_char("RoomEd: Obj doesn't exist.", ch);
			return FALSE;
		}

		if (key->item_type != ITEM_KEY) {
			act_char("RoomEd: That obj is not key.", ch);
			return FALSE;
		}

		pRoom->exit[door]->key = value;

		act_char("Exit key set.", ch);
		return TRUE;
	}

	if (!str_prefix(command, "description")) {
		if (!pRoom->exit[door]) {
			act_char("RoomEd: Exit does not exist.", ch);
			return FALSE;
		}

		if (!mlstr_append(ch, &pRoom->exit[door]->description, arg)) {
			act_puts("Syntax: $t desc <lang>",
				 ch, cmd->name, NULL,
				 TO_CHAR | ACT_NOTRANS, POS_DEAD);
			return FALSE;
		}
		return TRUE;
	}

	if (!str_prefix(command, "size")) {
		if (!pRoom->exit[door]) {
			act_char("RoomEd: Exit does not exist.", ch);
			return FALSE;
		}

		if (arg[0] == '?') {
			BUFFER *output;

			output = buf_new(0);
			buf_printf(output, BUF_END, "Valid size values are:\n");
			show_flags_buf(output, size_table);
			page_to_char(buf_string(output), ch);
			buf_free(output);
			return FALSE;
		}

		cmd->arg1 = size_table;
		return olced_flag(ch, arg, cmd, &pRoom->exit[door]->size);
	}

	olced_exit(ch, "?", cmd, door);
	return FALSE;
}

static void
display_resets(CHAR_DATA *ch)
{
	ROOM_INDEX_DATA	*in_room = ch->in_room;
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

		buf_printf(buf, BUF_END, "[%2d] ", ++rnum);
		switch (r->command) {
		default:
			buf_printf(buf, BUF_END, "Bad reset command: %c.",
				   r->command);
			break;

		case 'M':
			if ((mob = get_mob_index(r->arg1)) == NULL) {
				buf_printf(buf, BUF_END, "<bad mob %d>\n",
					   r->arg1);
				break;
			}

			last_mob = r;
			buf_printf(buf, BUF_END, "%c[%5d] %-26.26s [%2d-%2d]\n",
				   (room = get_room_index(in_room->vnum-1)) &&
				   IS_SET(room->room_flags, ROOM_PET_SHOP) ?
				   'P' : 'M',
				   r->arg1, mlstr_mval(&mob->short_descr),
				   r->arg2, r->arg4);
			break;

		case 'O':
			if ((obj = get_obj_index(r->arg1)) == NULL) {
				buf_printf(buf, BUF_END, "<bad obj %d>\n",
					   r->arg1);
				break;
			}

			last_obj = r;
			buf_printf(buf, BUF_END, "O[%5d:%3d%%] %-26.26s\n",
				   r->arg1, 100 - r->arg0,
				   mlstr_mval(&obj->short_descr));
			break;

		case 'P':
			if (last_obj == NULL) {
				buf_append(buf, "<no prev obj reset>\n");
				break;
			}

			if ((obj = get_obj_index(r->arg1)) == NULL) {
				buf_printf(buf, BUF_END, "<bad obj %d>\n",
					   r->arg1);
				break;
			}

			switch (last_obj->command) {
			case 'E':
			case 'G':
				buf_append(buf, tab2);
				break;

			case 'O':
				buf_append(buf, tab);
				break;
			}

			buf_printf(buf, BUF_END, "P[%5d:%3d%%] %-26.26s <inside> [%2d-%2d]\n",
				   r->arg1, 100 - r->arg0,
				   mlstr_mval(&obj->short_descr),
				   r->arg2, r->arg4);
			break;

		case 'G':
		case 'E':
			if (last_mob == NULL) {
				buf_append(buf, "<no prev mob reset>\n");
				break;
			}

			if ((mob = get_mob_index(last_mob->arg1)) == NULL) {
				buf_printf(buf, BUF_END, "<bad mob %d>\n",
					   last_mob->arg1);
				break;
			}

			if ((obj = get_obj_index(r->arg1)) == NULL) {
				buf_printf(buf, BUF_END, "<bad obj %d>\n",
					   r->arg1);
				break;
			}

			last_obj = r;
			buf_printf(buf, BUF_END, "%s%c[%5d:%3d%%] %-26.26s <%s>\n",
				   tab,
				   r->command == 'G' && mob->pShop ?
					'S' : r->command,
				   r->arg1, 100 - r->arg0,
				   mlstr_mval(&obj->short_descr),
				   flag_string(wear_loc_strings, r->command == 'G' ?  WEAR_NONE : r->arg3));
			break;

		case 'R':
			buf_printf(buf, BUF_END,
				   "R[%5d] Randomized exits [", r->arg1);
			for (d = 0; d < r->arg2 && d < MAX_DIR; d++) {
				if (d != 0)
					buf_append(buf, " ");
				buf_append(buf, dir_name[d]);
			}
			buf_append(buf, "]\n");
			break;
		}
	}

	page_to_char(buf_string(buf), ch);
	buf_free(buf);
}

DO_FUN(do_resets, ch, argument)
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
			act_char("No resets in this room.", ch);
		return;
	}

	if (!IS_BUILDER(ch, room->area)) {
		act_char("Resets: Insufficient security for editing this room.", ch);
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

	if (!str_prefix(arg1, "where")) {
		int vnum;
		bool show_mob;
		AREA_DATA *pArea;

		if (!str_prefix(arg2, "mob"))
			show_mob = TRUE;
		else if (!str_prefix(arg2, "obj"))
			show_mob = FALSE;
		else {
			do_resets(ch, "?");
			return;
		}

		if (!is_number(arg3)) {
			do_resets(ch, "?");
			return;
		}
		vnum = atoi(arg3);

		pArea = area_vnum_lookup(vnum);
		if (!IS_BUILDER(ch, pArea)) {
			act_char("Insufficient security.", ch);
			return;
		}

		if (show_mob)
			show_resets(ch, vnum, "mob", show_mob_resets);
		else
			show_resets(ch, vnum, "obj", show_obj_resets);
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
			act_char("No resets in this room.", ch);
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
			act_char("Insufficient security.", ch);
			return;
		}

		mob_reset = reset_new();
		mob_reset->command = 'M';
		mob_reset->arg1 = mob_vnum;
		mob_reset->arg2 = is_number(arg3) ? atoi(arg3) : 1; /* Max # */
		mob_reset->arg3 = room->vnum;
		mob_reset->arg4	= is_number(arg4) ? atoi(arg4) : 1; /* Min # */

		reset_add(room, mob_reset, NULL);
		TOUCH_AREA(room->area);
		act_char("Mob reset added.", ch);
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
			act_char("Insufficient security.", ch);
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
				act_char("Previous reset not found.", ch);
				return;
			}

			if (after->command != 'E'
			&&  after->command != 'G'
			&&  after->command != 'O') {
				act_char("Previous reset should be 'E', 'G' or 'O'.", ch);
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
				act_char("Previous reset not found.", ch);
				return;
			}

			if (after->command != 'M') {
				act_char("Previous reset should be 'M'.", ch);
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
		act_char("Reset added.", ch);
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
		act_char("Random exits reset added.", ch);
		return;
	}

	if (!str_prefix(arg1, "probability")) {
		/*
		 * set probability of obj reset
		 * ----------------------------
		 */
		int prob;
		RESET_DATA *reset;

		if (room->reset_first == NULL) {
			act_char("No resets in this room.", ch);
			return;
		}

		if ((reset = reset_lookup(room, atoi(arg2))) == NULL) {
			act_puts("$t: no resets with such num in this room.",
				 ch, arg1, NULL,
				 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
			return;
		}

		if (!is_number(arg3)
		||  (prob = atoi(arg3)) < 0
		||  prob > 100) {
			act_puts("$t: Invalid argument.",
				 ch, arg3, NULL,
				 TO_CHAR | ACT_NOTRANS | ACT_NOUCASE, POS_DEAD);
			return;
		}

		switch (reset->command) {
		case 'P':
		case 'O':
		case 'G':
		case 'E':
			reset->arg0 = 100 - prob;
			TOUCH_AREA(room->area);
			act_char("Reset probability set.", ch);
			break;
		default:
			act_puts("Reset #$t is not an obj reset.",
				 ch, arg2, NULL,
				 TO_CHAR | ACT_NOTRANS, POS_DEAD);
			break;
		}

		return;
	}

	if (!str_prefix(arg1, "now")) {
		reset_room(ch->in_room, RESET_F_NOPCHECK);
		act_char("Room reset.", ch);
		return;
	}

	if (!str_cmp(arg1, "clear")) {
		RESET_DATA *r, *r_next;

		if (room->reset_first == NULL) {
			act_char("No resets in this room.", ch);
			return;
		}

		for (r = room->reset_first; r; r = r_next) {
			r_next = r->next;
			reset_free(r);
		}
		room->reset_first = room->reset_last = NULL;
		TOUCH_AREA(room->area);

		act_char("Resets in the room cleared.", ch);
		return;
	}

	if (!str_prefix(arg1, "clear")) {
		olced_spell_out(ch, str_empty, NULL);
		return;
	}

	do_resets(ch, "?");
}

static ROOM_INDEX_DATA *
roomed_create_room(CHAR_DATA *ch, const char *arg)
{
	AREA_DATA *pArea;
	ROOM_INDEX_DATA *pRoom;
	int value;

	value = atoi(arg);
	if (!value) {
		dofun("help", ch, "'OLC CREATE'");
		return NULL;
	}

	pArea = area_vnum_lookup(value);
	if (!pArea) {
		act_char("RoomEd: Vnum is not assigned an area.", ch);
		return NULL;
	}

	if (!IS_BUILDER(ch, pArea)) {
		act_char("RoomEd: Insufficient security.", ch);
		return NULL;
	}

	if (get_room_index(value)) {
		act_char("RoomEd: Vnum already exists.", ch);
		return NULL;
	}

	pRoom = c_insert(&rooms, &value);
	pRoom->vnum = value;
	if (value > top_vnum_room)
		 top_vnum_room	= value;

	pRoom->area = pArea;

	TOUCH_AREA(pArea);
	return pRoom;
}
