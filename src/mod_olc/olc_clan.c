/*
 * $Id: olc_clan.c,v 1.1 1998-09-01 18:38:09 fjoe Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include "merc.h"
#include "olc.h"
#include "interp.h"

#define CEDIT(fun)		bool fun(CHAR_DATA *ch, const char *argument)
#define EDIT_CLAN(ch, clan)	(clan = (CLAN_DATA*) ch->desc->pEdit)

DECLARE_OLC_FUN(cedit_create		);
DECLARE_OLC_FUN(cedit_edit		);
DECLARE_OLC_FUN(cedit_show		);
DECLARE_OLC_FUN(cedit_name		);
DECLARE_OLC_FUN(cedit_filename		);
DECLARE_OLC_FUN(cedit_recall		);
DECLARE_OLC_FUN(cedit_msg_prays		);
DECLARE_OLC_FUN(cedit_msg_vanishes	);
DECLARE_OLC_FUN(cedit_flags		);
DECLARE_OLC_FUN(cedit_skadd		);
DECLARE_OLC_FUN(cedit_skdel		);

DECLARE_VALIDATE_FUN(validate_name	);
DECLARE_VALIDATE_FUN(validate_recall	);

OLC_CMD_DATA cedit_table[] =
{
	{ "create",	cedit_create				},
	{ "edit",	cedit_edit				},
	{ "show",	cedit_show				},
	{ "name",	cedit_name,	validate_name	 	},
	{ "filename",	cedit_filename,	validate_filename	},
	{ "recall",	cedit_recall,	validate_recall		},
	{ "msgp",	cedit_msg_prays				},
	{ "msgv",	cedit_msg_vanishes			},
	{ "flags",	cedit_flags,	clan_flags		},
	{ "skadd",	cedit_skadd				},
	{ "skdel",	cedit_skdel				},

	{ "commands",	show_commands				},
	{ NULL }
};

void cedit(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	char command[MAX_INPUT_LENGTH];
	int cmd;

	strnzcpy(arg, argument, MAX_INPUT_LENGTH);
	argument = one_argument(argument, command);

	if (command[0] == '\0') {
		cedit_show(ch, argument);
		return;
	}

	if (!str_cmp(command, "done")) {
		edit_done(ch);
		return;
	}

	for (cmd = 0; cedit_table[cmd].name != NULL; cmd++)
		if (!str_prefix(command, cedit_table[cmd].name)) {
			if (cedit_table[cmd].olc_fun(ch, argument)) {
				CLAN_DATA *clan;
				EDIT_CLAN(ch, clan);
				SET_BIT(clan->flags, CLAN_CHANGED);
			}
			return;
		}

	interpret(ch, arg);
}

void do_cedit(CHAR_DATA *ch, const char *argument)
{
	char command[MAX_INPUT_LENGTH];

	if (IS_NPC(ch)) {
		char_nputs(MSG_HUH, ch);
		return;
	}
		
	if (ch->pcdata->security < SECURITY_CLAN) {
		char_puts("CEdit: Insufficient security for editing clans\n\r", ch);
		return;
	}

	argument = one_argument(argument, command);

	if (!str_cmp(command, "create")) {
		cedit_create(ch, argument);
		return;
	}

	if (!str_cmp(command, "edit")) {
		cedit_edit(ch, argument);
		return;
	}

	do_help(ch, "'OLC CEDIT'");
}

CEDIT(cedit_create)
{
	int cn;
	CLAN_DATA *clan;

	if (argument[0] == '\0') {
		do_help(ch, "'OLC CEDIT'");
		return FALSE;
	}

	if ((cn = cn_lookup(argument)) >= 0) {
		char_printf(ch, "CEdit: %s: already exists.\n\r",
			    CLAN(cn)->name);
		return FALSE;
	}

	clan			= varr_enew(clans);
	clan->name		= str_dup(argument);
	clan->skills		= varr_new(sizeof(CLAN_SKILL), 8);
	clan->file_name		= str_printf("clan%02d.clan", clans->nused-1);

	ch->desc->pEdit		= (void *)clan;
	ch->desc->editor	= ED_CLAN;

	send_to_char("Clan created.\n\r",ch);

	return FALSE;
}

CEDIT(cedit_edit)
{
	int cn;

	if ((cn = cn_lookup(argument)) < 0)
		do_help(ch, "'OLC CEDIT'");
	else {
		ch->desc->pEdit		= CLAN(cn);
		ch->desc->editor	= ED_CLAN;
	}

	return FALSE;
}

CEDIT(cedit_show)
{
	int i;
	BUFFER *output;
	CLAN_DATA *clan;
	EDIT_CLAN(ch, clan);

	output = buf_new(0);
	buf_printf(output,
		   "Name:        [%s]\n\r"
		   "Filename:    [%s]\n\r",
		   clan->name,
		   clan->file_name);
	if (clan->flags)
		buf_printf(output, "Flags:       [%s]\n\r",
			   flag_string(clan_flags, clan->flags));
	if (clan->recall_vnum)
		buf_printf(output, "Recall:      [%d]\n\r",
			   clan->recall_vnum);
	if (!mlstr_null(clan->msg_prays))
		mlstr_dump(output, "MsgPrays:    ", clan->msg_prays);
	if (!mlstr_null(clan->msg_vanishes))
		mlstr_dump(output, "MsgVanishes: ", clan->msg_vanishes);

	for (i = 0; i < clan->skills->nused; i++) {
		CLAN_SKILL *cs = VARR_GET(clan->skills, i);
		SKILL_DATA *skill;

		if (cs->sn <= 0
		||  (skill = skill_lookup(cs->sn)) == NULL)
			continue;
		buf_printf(output, "Skill:       '%s' (level %d)\n\r",
			   skill->name, cs->level);
	}

	page_to_char(buf_string(output), ch);
	buf_free(output);

	return FALSE;
}

CEDIT(cedit_name)
{
	CLAN_DATA *clan;
	EDIT_CLAN(ch, clan);
	return olced_str(ch, argument, cedit_name, &clan->name);
}

CEDIT(cedit_filename)
{
	CLAN_DATA *clan;
	EDIT_CLAN(ch, clan);
	return olced_str(ch, argument, cedit_filename, &clan->file_name);
}

CEDIT(cedit_recall)
{
	CLAN_DATA *clan;
	EDIT_CLAN(ch, clan);
	return olced_number(ch, argument, cedit_recall, &clan->recall_vnum);
}

CEDIT(cedit_msg_prays)
{
	CLAN_DATA *clan;
	EDIT_CLAN(ch, clan);
	return olced_mlstr(ch, argument, cedit_msg_prays, &clan->msg_prays);
}

CEDIT(cedit_msg_vanishes)
{
	CLAN_DATA *clan;
	EDIT_CLAN(ch, clan);
	return olced_mlstr(ch, argument, cedit_msg_vanishes, &clan->msg_vanishes);
}

CEDIT(cedit_flags)
{
	CLAN_DATA *clan;
	EDIT_CLAN(ch, clan);
	return olced_flag(ch, argument, cedit_flags, &clan->flags);
}

CEDIT(cedit_skadd)
{
	int sn;
	CLAN_SKILL *clan_skill;
	char	arg1[MAX_STRING_LENGTH];
	char	arg2[MAX_STRING_LENGTH];
	CLAN_DATA *clan;
	EDIT_CLAN(ch, clan);

	argument = one_argument(argument, arg1);
		   one_argument(argument, arg2);

	if (arg1[0] == '\0' || arg2[0] == '\0') {
		do_help(ch, "'OLC SKADD'");
		return FALSE;
	}

	if ((sn = sn_lookup(arg1)) <= 0) {
		char_printf(ch, "CEdit: %s: unknown skill.\n\r", arg1);
		return FALSE;
	}

	if (!IS_SET(SKILL(sn)->flags, SKILL_CLAN)) {
		char_printf(ch, "CEdit: %s: not a clan skill.\n\r",
			    SKILL(sn)->name);
		return FALSE;
	}

	if ((clan_skill = clan_skill_lookup(clan, sn))) {
		char_printf(ch, "Cedit: %s: already there.\n\r",
			    SKILL(sn)->name);
		return FALSE;
	}

	clan_skill = varr_enew(clan->skills);
	clan_skill->sn = sn;
	clan_skill->level = atoi(arg2);
	varr_qsort(clan->skills, cmpint);

	return TRUE;
}

CEDIT(cedit_skdel)
{
	char	arg[MAX_STRING_LENGTH];
	CLAN_SKILL *clan_skill;
	CLAN_DATA *clan;
	EDIT_CLAN(ch, clan);

	one_argument(argument, arg);
	if ((clan_skill = skill_vlookup(clan->skills, arg)) == NULL) {
		char_printf(ch, "CEdit: %s: not found in clan skill list.\n\r",
			    arg);
		return FALSE;
	}
	clan_skill->sn = 0;
	varr_qsort(clan->skills, cmpint);
	return TRUE;
}

VALIDATOR(validate_name)
{
	int i;
	CLAN_DATA *clan;
	EDIT_CLAN(ch, clan);

	for (i = 0; i < clans->nused; i++)
		if (CLAN(i) != clan
		&&  !str_cmp(CLAN(i)->name, arg)) {
			char_printf(ch, "CEdit: %s: duplicate clan name.\n\r",
				    arg);
			return FALSE;
		}

	return TRUE;
}

VALIDATOR(validate_recall)
{
	int vnum = *(int*) arg;

	if (vnum && get_room_index(vnum) == NULL) {
		char_printf(ch, "CEdit: %d: no such room.\n\r", vnum);
		return FALSE;
	}

	return TRUE;
}
