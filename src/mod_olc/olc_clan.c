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
 * $Id: olc_clan.c,v 1.22 1999-02-15 18:19:44 fjoe Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include "merc.h"
#include "olc.h"

#define EDIT_CLAN(ch, clan)	(clan = (CLAN_DATA*) ch->desc->pEdit)

DECLARE_OLC_FUN(claned_create		);
DECLARE_OLC_FUN(claned_edit		);
DECLARE_OLC_FUN(claned_touch		);
DECLARE_OLC_FUN(claned_show		);
DECLARE_OLC_FUN(claned_list		);

DECLARE_OLC_FUN(claned_name		);
DECLARE_OLC_FUN(claned_filename		);
DECLARE_OLC_FUN(claned_recall		);
DECLARE_OLC_FUN(claned_msg_prays	);
DECLARE_OLC_FUN(claned_msg_vanishes	);
DECLARE_OLC_FUN(claned_flags		);
DECLARE_OLC_FUN(claned_skill		);
DECLARE_OLC_FUN(claned_item		);
DECLARE_OLC_FUN(claned_mark		);
DECLARE_OLC_FUN(claned_altar		);
DECLARE_OLC_FUN(claned_plist		);

DECLARE_OLC_FUN(claned_skill_add	);
DECLARE_OLC_FUN(claned_skill_del	);

static DECLARE_VALIDATE_FUN(validate_name);

OLC_CMD_DATA olc_cmds_clan[] =
{
	{ "create",	claned_create					},
	{ "edit",	claned_edit					},
	{ "touch",	claned_touch					},
	{ "show",	claned_show					},
	{ "list",	claned_list					},

	{ "name",	claned_name,		validate_name	 	},
	{ "filename",	claned_filename,	validate_filename	},
	{ "recall",	claned_recall,		validate_room_vnum	},
	{ "msgp",	claned_msg_prays				},
	{ "msgv",	claned_msg_vanishes				},
	{ "flags",	claned_flags,		clan_flags		},
	{ "skill",	claned_skill					},
	{ "item",	claned_item					},
	{ "mark",	claned_mark					},
	{ "altar", 	claned_altar					},
	{ "plist",	claned_plist					},

	{ "commands",	show_commands					},
	{ NULL }
};

OLC_FUN(claned_create)
{
	int cn;
	CLAN_DATA *clan;
	char arg[MAX_STRING_LENGTH];

	if (ch->pcdata->security < SECURITY_CLAN) {
		char_puts("ClanEd: Insufficient security for creating clans\n",
			  ch);
		return FALSE;
	}

	first_arg(argument, arg, FALSE);
	if (arg[0] == '\0') {
		do_help(ch, "'OLC CREATE'");
		return FALSE;
	}

	if ((cn = cn_lookup(arg)) >= 0) {
		char_printf(ch, "ClanEd: %s: already exists.\n",
			    CLAN(cn)->name);
		return FALSE;
	}

	clan			= clan_new();
	clan->name		= str_dup(arg);
	clan->file_name		= str_printf("clan%02d.clan", clans.nused-1);

	ch->desc->pEdit		= (void *)clan;
	ch->desc->editor	= ED_CLAN;
	touch_clan(clan);
	char_puts("Clan created.\n",ch);
	return FALSE;
}

OLC_FUN(claned_edit)
{
	int cn;
	char arg[MAX_STRING_LENGTH];

	if (ch->pcdata->security < SECURITY_CLAN) {
		char_puts("ClanEd: Insufficient security.\n", ch);
		return FALSE;
	}

	one_argument(argument, arg);
	if (arg[0] == '\0') {
		do_help(ch, "'OLC EDIT'");
		return FALSE;
	}

	if ((cn = cn_lookup(arg)) < 0) {
		char_printf(ch, "ClanEd: %s: No such clan.\n", arg);
		return FALSE;
	}

	ch->desc->pEdit		= CLAN(cn);
	ch->desc->editor	= ED_CLAN;
	return FALSE;
}

OLC_FUN(claned_touch)
{
	CLAN_DATA *clan;
	EDIT_CLAN(ch, clan);
	return touch_clan(clan);
}

OLC_FUN(claned_show)
{
	char arg[MAX_STRING_LENGTH];
	int i;
	BUFFER *output;
	CLAN_DATA *clan;

	one_argument(argument, arg);
	if (arg[0] == '\0') {
		if (ch->desc->editor == ED_CLAN)
			EDIT_CLAN(ch, clan);
		else {
			do_help(ch, "'OLC ASHOW'");
			return FALSE;
		}
	}
	else {
		if ((i = cn_lookup(arg)) < 0) {
			char_printf(ch, "ClanEd: %s: No such clan.\n", arg);
			return FALSE;
		}
		clan = CLAN(i);
	}

	output = buf_new(-1);
	buf_printf(output,
		   "Name:        [%s]\n"
		   "Filename:    [%s]\n",
		   clan->name,
		   clan->file_name);
	if (clan->flags)
		buf_printf(output, "Flags:       [%s]\n",
			   flag_string(clan_flags, clan->flags));
	if (clan->recall_vnum)
		buf_printf(output, "Recall:      [%d]\n",
			   clan->recall_vnum);
	if (clan->obj_vnum)
		buf_printf(output, "Item:        [%d]\n",
			   clan->obj_vnum);
	if (clan->mark_vnum) 
		buf_printf(output, "Mark:	 [%d]\n",
			   clan->mark_vnum);
	if (clan->altar_vnum)
		buf_printf(output, "Altar:       [%d]\n",
			   clan->altar_vnum);
	if (!IS_NULLSTR(clan->msg_prays))
		buf_printf(output, "MsgPrays:    [%s]\n", clan->msg_prays);
	if (!IS_NULLSTR(clan->msg_vanishes))
		buf_printf(output, "MsgVanishes: [%s]\n", clan->msg_vanishes);

	for (i = 0; i < clan->skills.nused; i++) {
		CLAN_SKILL *cs = VARR_GET(&clan->skills, i);
		SKILL_DATA *skill;

		if (cs->sn <= 0
		||  (skill = skill_lookup(cs->sn)) == NULL)
			continue;
		buf_printf(output, "Skill:       '%s' (level %d, %d%%)\n",
			   skill->name, cs->level, cs->percent);
	}

	page_to_char(buf_string(output), ch);
	buf_free(output);

	return FALSE;
}

OLC_FUN(claned_list)
{
	int i;

	for (i = 0; i < clans.nused; i++)
		char_printf(ch, "[%d] %s\n", i, CLAN(i)->name);
	return FALSE;
}

OLC_FUN(claned_name)
{
	CLAN_DATA *clan;
	EDIT_CLAN(ch, clan);
	return olced_str(ch, argument, claned_name, &clan->name);
}

OLC_FUN(claned_filename)
{
	CLAN_DATA *clan;
	EDIT_CLAN(ch, clan);
	return olced_str(ch, argument, claned_filename, &clan->file_name);
}

OLC_FUN(claned_recall)
{
	CLAN_DATA *clan;
	EDIT_CLAN(ch, clan);
	return olced_number(ch, argument, claned_recall, &clan->recall_vnum);
}

OLC_FUN(claned_item)
{
	CLAN_DATA *clan;
	EDIT_CLAN(ch, clan);
	return olced_number(ch, argument, claned_item, &clan->obj_vnum);
}

OLC_FUN(claned_mark)
{
	CLAN_DATA *clan;
	EDIT_CLAN(ch, clan);
	return olced_number(ch, argument, claned_mark, &clan->mark_vnum);
}

OLC_FUN(claned_altar)
{
	CLAN_DATA *clan;
	EDIT_CLAN(ch, clan);
	return olced_number(ch, argument, claned_altar, &clan->altar_vnum);
}

OLC_FUN(claned_msg_prays)
{
	CLAN_DATA *clan;
	EDIT_CLAN(ch, clan);
	return olced_str(ch, argument, claned_msg_prays, &clan->msg_prays);
}

OLC_FUN(claned_msg_vanishes)
{
	CLAN_DATA *clan;
	EDIT_CLAN(ch, clan);
	return olced_str(ch, argument, claned_msg_vanishes, &clan->msg_vanishes);
}

OLC_FUN(claned_flags)
{
	CLAN_DATA *clan;
	EDIT_CLAN(ch, clan);
	return olced_flag32(ch, argument, claned_flags, &clan->flags);
}

OLC_FUN(claned_skill)
{
	char arg[MAX_STRING_LENGTH];

	argument = one_argument(argument, arg);
	if (!str_prefix(arg, "add")) 
		return claned_skill_add(ch, argument);
	else if (!str_prefix(arg, "delete"))
		return claned_skill_del(ch, argument);

	do_help(ch, "'OLC CLAN SKILL'");
	return FALSE;
}

OLC_FUN(claned_plist)
{
	const char **nl;
	const char *name;
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	CLAN_DATA *clan;
	EDIT_CLAN(ch, clan);
	
	if (ch->pcdata->security < SECURITY_CLAN_PLIST) {
		char_puts("ClanEd: Insufficient security.\n", ch);
		return FALSE;
	}

	argument = one_argument(argument, arg1);
		   one_argument(argument, arg2);

	if (arg1[0] == '\0') {
		do_help(ch, "'OLC CLAN PLIST'");
		return FALSE;
	}

	if (!str_prefix(arg1, "member")) {
		nl = &clan->member_list;
		name = "members";
	}
	else if (!str_prefix(arg1, "leader")) {
		nl = &clan->leader_list;
		name = "leaders";
	}
	else if (!str_prefix(arg1, "second")) {
		nl = &clan->second_list;
		name = "secondaries";
	}
	else
		return claned_plist(ch, str_empty);

	if (arg2[0] == '\0') {
		char_printf(ch, "List of %s of %s: [%s]\n",
			    name, clan->name, *nl);
		return FALSE;
	}
			    
	if (!pc_name_ok(arg2)) {
		char_printf(ch, "ClanEd: %s: Illegal name\n", arg2);
		return FALSE;
	}

	name_toggle(nl, arg2, ch, "ClanEd");
	return TRUE;
}

OLC_FUN(claned_skill_add)
{
	int sn;
	int percent;
	CLAN_SKILL *clan_skill;
	char	arg1[MAX_STRING_LENGTH];
	char	arg2[MAX_STRING_LENGTH];
	char	arg3[MAX_STRING_LENGTH];
	CLAN_DATA *clan;
	EDIT_CLAN(ch, clan);

	argument = one_argument(argument, arg1);
	argument = one_argument(argument, arg2);
		   one_argument(argument, arg3);

	if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0') {
		do_help(ch, "'OLC CLAN SKILL'");
		return FALSE;
	}

	if ((sn = sn_lookup(arg1)) <= 0) {
		char_printf(ch, "ClanEd: %s: unknown skill.\n", arg1);
		return FALSE;
	}

	if (!IS_SET(SKILL(sn)->flags, SKILL_CLAN)) {
		char_printf(ch, "ClanEd: %s: not a clan skill.\n",
			    SKILL(sn)->name);
		return FALSE;
	}

	if ((clan_skill = clan_skill_lookup(clan, sn))) {
		char_printf(ch, "ClanEd: %s: already there.\n",
			    SKILL(sn)->name);
		return FALSE;
	}

	percent = atoi(arg3);
	if (percent < 1 || percent > 100) {
		char_puts("ClanEd: percent value must be in range 1..100.\n",
			  ch);
		return FALSE;
	}

	clan_skill = varr_enew(&clan->skills);
	clan_skill->sn = sn;
	clan_skill->level = atoi(arg2);
	clan_skill->percent = percent;
	varr_qsort(&clan->skills, cmpint);

	return TRUE;
}

OLC_FUN(claned_skill_del)
{
	char	arg[MAX_STRING_LENGTH];
	CLAN_SKILL *clan_skill;
	CLAN_DATA *clan;
	EDIT_CLAN(ch, clan);

	one_argument(argument, arg);
	if (arg[0] == '\0') {
		do_help(ch, "'OLC CLAN SKILL'");
		return FALSE;
	}

	if ((clan_skill = skill_vlookup(&clan->skills, arg)) == NULL) {
		char_printf(ch, "ClanEd: %s: not found in clan skill list.\n",
			    arg);
		return FALSE;
	}
	clan_skill->sn = 0;
	varr_qsort(&clan->skills, cmpint);
	return TRUE;
}

bool touch_clan(CLAN_DATA *clan)
{
	SET_BIT(clan->flags, CLAN_CHANGED);
	return FALSE;
}

static VALIDATE_FUN(validate_name)
{
	int i;
	CLAN_DATA *clan;
	EDIT_CLAN(ch, clan);

	for (i = 0; i < clans.nused; i++)
		if (CLAN(i) != clan
		&&  !str_cmp(CLAN(i)->name, arg)) {
			char_printf(ch, "ClanEd: %s: duplicate clan name.\n",
				    arg);
			return FALSE;
		}

	return TRUE;
}

