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
 * $Id: olc_clan.c,v 1.35 1999-10-21 12:51:56 fjoe Exp $
 */

#include "olc.h"

#define EDIT_CLAN(ch, clan)	(clan = (clan_t*) ch->desc->pEdit)

DECLARE_OLC_FUN(claned_create		);
DECLARE_OLC_FUN(claned_edit		);
DECLARE_OLC_FUN(claned_save		);
DECLARE_OLC_FUN(claned_touch		);
DECLARE_OLC_FUN(claned_show		);
DECLARE_OLC_FUN(claned_list		);

DECLARE_OLC_FUN(claned_recall		);
DECLARE_OLC_FUN(claned_flags		);
DECLARE_OLC_FUN(claned_item		);
DECLARE_OLC_FUN(claned_mark		);
DECLARE_OLC_FUN(claned_altar		);
DECLARE_OLC_FUN(claned_plist		);
DECLARE_OLC_FUN(claned_skillspec	);

/* also should move clan's plists */
olced_strkey_t strkey_clans = { &clans, CLANS_PATH };

olc_cmd_t olc_cmds_clan[] =
{
	{ "create",	claned_create					},
	{ "edit",	claned_edit					},
	{ "",		claned_save					},
	{ "touch",	claned_touch					},
	{ "show",	claned_show					},
	{ "list",	claned_list					},

	{ "name",	olced_strkey,	NULL,		&strkey_clans 	},
	{ "recall",	claned_recall,	validate_room_vnum		},
	{ "flags",	claned_flags,	NULL,		clan_flags	},
	{ "item",	claned_item					},
	{ "mark",	claned_mark					},
	{ "altar", 	claned_altar					},
	{ "skillspec",	claned_skillspec,validate_skill_spec		},
	{ "plist",	claned_plist					},

	{ "commands",	show_commands					},
	{ NULL }
};

typedef struct _save_clan_t {
	CHAR_DATA *ch;
	bool found;
} _save_clan_t;
static void *save_clan_cb(void *p, void *d);

OLC_FUN(claned_create)
{
	clan_t clan;
	clan_t *cl;
	char arg[MAX_STRING_LENGTH];

	if (PC(ch)->security < SECURITY_CLAN) {
		char_puts("ClanEd: Insufficient security for creating clans\n",
			  ch);
		return FALSE;
	}

	first_arg(argument, arg, sizeof(arg), FALSE);
	if (arg[0] == '\0') {
		dofun("help", ch, "'OLC CREATE'");
		return FALSE;
	}

	/*
	 * olced_busy check is not needed since hash_insert
	 * adds new elements to the end of varr
	 */

	clan_init(&clan);
	clan.name = str_dup(arg);
	cl = hash_insert(&clans, &clan, clan.name);
	clan_destroy(&clan);

	if (cl == NULL) {
		char_printf(ch, "ClanEd: %s: already exists.\n", arg);
		return FALSE;
	}

	ch->desc->pEdit	= cl;
	OLCED(ch)	= olced_lookup(ED_CLAN);
	touch_clan(cl);
	char_puts("Clan created.\n", ch);
	return FALSE;
}

OLC_FUN(claned_edit)
{
	clan_t *clan;
	char arg[MAX_STRING_LENGTH];

	if (PC(ch)->security < SECURITY_CLAN) {
		char_puts("ClanEd: Insufficient security.\n", ch);
		return FALSE;
	}

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		dofun("help", ch, "'OLC EDIT'");
		return FALSE;
	}

	if ((clan = clan_search(arg)) == NULL) {
		char_printf(ch, "ClanEd: %s: No such clan.\n", arg);
		return FALSE;
	}

	ch->desc->pEdit	= clan;
	OLCED(ch)	= olced_lookup(ED_CLAN);
	return FALSE;
}

OLC_FUN(claned_save)
{
	_save_clan_t sc;

	olc_printf(ch, "Saved clans:");
	sc.ch = ch;
	sc.found = FALSE;
	hash_foreach(&clans, save_clan_cb, &sc);
	if (!sc.found)
		olc_printf(ch, "    None.");
	return FALSE;
}

OLC_FUN(claned_touch)
{
	clan_t *clan;
	EDIT_CLAN(ch, clan);
	return touch_clan(clan);
}

OLC_FUN(claned_show)
{
	char arg[MAX_STRING_LENGTH];
	BUFFER *output;
	clan_t *clan;

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		if (IS_EDIT(ch, ED_CLAN))
			EDIT_CLAN(ch, clan);
		else {
			dofun("help", ch, "'OLC ASHOW'");
			return FALSE;
		}
	} else if ((clan = clan_search(arg)) == NULL) {
		char_printf(ch, "ClanEd: %s: No such clan.\n", arg);
		return FALSE;
	}

	output = buf_new(-1);
	buf_printf(output,
		   "Name:        [%s]\n",
		   clan->name);
	if (!IS_NULLSTR(clan->skill_spec))
		buf_printf(output, "SkillSpec:   [%s]\n", clan->skill_spec);
	if (clan->clan_flags)
		buf_printf(output, "Flags:       [%s]\n",
			   flag_string(clan_flags, clan->clan_flags));
	if (clan->recall_vnum)
		buf_printf(output, "Recall:      [%d]\n",
			   clan->recall_vnum);
	if (clan->obj_vnum)
		buf_printf(output, "Item:        [%d]\n",
			   clan->obj_vnum);
	if (clan->mark_vnum) 
		buf_printf(output, "Mark:        [%d]\n",
			   clan->mark_vnum);
	if (clan->altar_vnum)
		buf_printf(output, "Altar:       [%d]\n",
			   clan->altar_vnum);

	page_to_char(buf_string(output), ch);
	buf_free(output);

	return FALSE;
}

OLC_FUN(claned_list)
{
	BUFFER *out = buf_new(-1);
	buf_add(out, "List of defined clans:\n");
	strkey_printall(&clans, out);
	page_to_char(buf_string(out), ch);
	buf_free(out);
	return FALSE;
}

OLC_FUN(claned_recall)
{
	clan_t *clan;
	EDIT_CLAN(ch, clan);
	return olced_number(ch, argument, cmd, &clan->recall_vnum);
}

OLC_FUN(claned_item)
{
	clan_t *clan;
	EDIT_CLAN(ch, clan);
	return olced_number(ch, argument, cmd, &clan->obj_vnum);
}

OLC_FUN(claned_mark)
{
	clan_t *clan;
	EDIT_CLAN(ch, clan);
	return olced_number(ch, argument, cmd, &clan->mark_vnum);
}

OLC_FUN(claned_altar)
{
	clan_t *clan;
	EDIT_CLAN(ch, clan);
	return olced_number(ch, argument, cmd, &clan->altar_vnum);
}

OLC_FUN(claned_skillspec)
{
	clan_t *clan;
	EDIT_CLAN(ch, clan);
	return olced_str(ch, argument, cmd, &clan->skill_spec);
}

OLC_FUN(claned_flags)
{
	clan_t *clan;
	EDIT_CLAN(ch, clan);
	return olced_flag32(ch, argument, cmd, &clan->clan_flags);
}

OLC_FUN(claned_plist)
{
	const char **nl;
	const char *name;
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	clan_t *clan;
	EDIT_CLAN(ch, clan);
	
	if (PC(ch)->security < SECURITY_CLAN_PLIST) {
		char_puts("ClanEd: Insufficient security.\n", ch);
		return FALSE;
	}

	argument = one_argument(argument, arg1, sizeof(arg1));
		   one_argument(argument, arg2, sizeof(arg2));

	if (arg1[0] == '\0') {
		dofun("help", ch, "'OLC CLAN PLIST'");
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
		return claned_plist(ch, str_empty, cmd);

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

bool touch_clan(clan_t *clan)
{
	SET_BIT(clan->clan_flags, CLAN_CHANGED);
	return FALSE;
}

static void *
save_clan_cb(void *p, void *d)
{
	clan_t *clan = (clan_t *) p;
	_save_clan_t *sc = (_save_clan_t *) d;
	
	FILE *fp;
	char buf[PATH_MAX];

	if (!IS_SET(clan->clan_flags, CLAN_CHANGED))
		return NULL;

	snprintf(buf, sizeof(buf), "%s.%s",
		 strkey_filename(clan->name), CLAN_EXT);
	if ((fp = olc_fopen(CLANS_PATH, buf, sc->ch, -1)) == NULL)
		return NULL;
		
	fprintf(fp, "#CLAN\n");

	fwrite_string(fp, "Name", clan->name);
	if (!IS_NULLSTR(clan->skill_spec))
		fprintf(fp, "SkillSpec '%s'\n", clan->skill_spec);
	if (clan->recall_vnum)
		fprintf(fp, "Recall %d\n", clan->recall_vnum);
	if (clan->obj_vnum)
		fprintf(fp, "Item %d\n", clan->obj_vnum);
	if (clan->mark_vnum)
		fprintf(fp, "Mark %d\n", clan->mark_vnum);
	if (clan->altar_vnum)
		fprintf(fp, "Altar %d\n", clan->altar_vnum);

	REMOVE_BIT(clan->clan_flags, CLAN_CHANGED);
	if (clan->clan_flags)
		fprintf(fp, "Flags %s~\n",
			flag_string(clan_flags, clan->clan_flags));

	fprintf(fp, "End\n\n"
		    "#$\n");
	fclose(fp);

/* save plists */
	if ((fp = olc_fopen(PLISTS_PATH, buf, sc->ch, -1)) == NULL)
		return NULL;

	fprintf(fp, "#PLISTS\n");

	fwrite_string(fp, "Leaders", clan->leader_list);
	fwrite_string(fp, "Seconds", clan->second_list);
	fwrite_string(fp, "Members", clan->member_list);

	fprintf(fp, "End\n\n"
		    "#$\n");
	fclose(fp);

	olc_printf(sc->ch, "    %s (%s)", clan->name, buf);
	sc->found = TRUE;
	return NULL;
}

