/*-
 * Copyright (c) 1999 arborn <avn@org.chem.msu.su>
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
 * $Id: olc_race.c,v 1.7 1999-09-24 04:16:05 avn Exp $
 */

#include "olc.h"

#define EDIT_RACE(ch, race)	(race = (race_t*) ch->desc->pEdit)

DECLARE_OLC_FUN(raceed_create		);
DECLARE_OLC_FUN(raceed_edit		);
DECLARE_OLC_FUN(raceed_save		);
DECLARE_OLC_FUN(raceed_touch		);
DECLARE_OLC_FUN(raceed_show		);
DECLARE_OLC_FUN(raceed_list		);

DECLARE_OLC_FUN(raceed_name		);
DECLARE_OLC_FUN(raceed_filename		);
DECLARE_OLC_FUN(raceed_act		);
DECLARE_OLC_FUN(raceed_affect		);
DECLARE_OLC_FUN(raceed_off		);
DECLARE_OLC_FUN(raceed_imm		);
DECLARE_OLC_FUN(raceed_res		);
DECLARE_OLC_FUN(raceed_vuln		);
DECLARE_OLC_FUN(raceed_form		);
DECLARE_OLC_FUN(raceed_parts		);
DECLARE_OLC_FUN(raceed_flags		);

DECLARE_OLC_FUN(raceed_addpcdata	);
DECLARE_OLC_FUN(raceed_delpcdata	);

DECLARE_OLC_FUN(raceed_whoname		);
DECLARE_OLC_FUN(raceed_points		);
DECLARE_OLC_FUN(raceed_bonusskill	);
DECLARE_OLC_FUN(raceed_stats		);
DECLARE_OLC_FUN(raceed_maxstats		);
DECLARE_OLC_FUN(raceed_size		);
DECLARE_OLC_FUN(raceed_hpbonus		);
DECLARE_OLC_FUN(raceed_manabonus	);
DECLARE_OLC_FUN(raceed_pracbonus	);
DECLARE_OLC_FUN(raceed_slang		);
DECLARE_OLC_FUN(raceed_align		);
DECLARE_OLC_FUN(raceed_ethos		);

DECLARE_OLC_FUN(raceed_addclass		);
DECLARE_OLC_FUN(raceed_delclass		);

DECLARE_OLC_FUN(raceed_addskill		);
DECLARE_OLC_FUN(raceed_delskill		);

DECLARE_OLC_FUN(olc_skill_update	);

static DECLARE_VALIDATE_FUN(validate_name);
static DECLARE_VALIDATE_FUN(validate_whoname);
static DECLARE_VALIDATE_FUN(validate_haspcdata);

olc_cmd_t olc_cmds_race[] =
{
	{ "create",	raceed_create					},
	{ "edit",	raceed_edit					},
	{ "",		raceed_save					},
	{ "touch",	raceed_touch					},
	{ "show",	raceed_show					},
	{ "list",	raceed_list					},

	{ "name",	raceed_name,		validate_name		},
	{ "filename",	raceed_filename,	validate_filename	},
	{ "act",	raceed_act,		act_flags		},
	{ "affect",	raceed_affect,		affect_flags		},
	{ "off",	raceed_off,		off_flags		},
	{ "imm",	raceed_imm,		imm_flags		},
	{ "res",	raceed_res,		res_flags		},
	{ "vuln",	raceed_vuln,		vuln_flags		},
       	{ "form",	raceed_form,		form_flags		},
	{ "parts",	raceed_parts,		part_flags		},
	{ "flags",	raceed_flags,		race_flags		},

	{ "addpcdata",	raceed_addpcdata,	validate_whoname	},
	{ "delpcdata",	raceed_delpcdata				},

	{ "whoname",	raceed_whoname,		validate_whoname	},
	{ "points",	raceed_points,		validate_haspcdata	},
	{ "bonusskill",	raceed_bonusskill,	validate_haspcdata	},
	{ "stats",	raceed_stats,		validate_haspcdata	},
	{ "maxstats",	raceed_maxstats,	validate_haspcdata	},
	{ "size",	raceed_size,		size_table		},
	{ "hpbonus",	raceed_hpbonus,		validate_haspcdata	},
	{ "manabonus",	raceed_manabonus,	validate_haspcdata	},
	{ "pracbonus",	raceed_pracbonus,	validate_haspcdata	},
	{ "slang",	raceed_slang,		slang_table		},
	{ "align",	raceed_align,		ralign_names		},
	{ "ethos",	raceed_ethos,		ethos_table		},

	{ "addskill",	raceed_addskill					},
	{ "delskill",	raceed_delskill					},

	{ "addclass",	raceed_addclass					},
	{ "delclass",	raceed_delclass					},

	{ "update",	olc_skill_update				},
	{ "commands",	show_commands					},
	{ NULL }
};

static void save_race(CHAR_DATA *ch, race_t *race);

OLC_FUN(raceed_create)
{
	int rn;
	race_t *race;
	char arg[MAX_STRING_LENGTH];

	if (PC(ch)->security < SECURITY_RACE) {
		char_puts("RaceEd: Insufficient security for creating races\n",
			  ch);
		return FALSE;
	}

	first_arg(argument, arg, sizeof(arg), FALSE);
	if (arg[0] == '\0') {
		dofun("help", ch, "'OLC CREATE'");
		return FALSE;
	}

	if ((rn = rn_lookup(arg)) >= 0) {
		char_printf(ch, "RaceEd: %s: already exists.\n",
			    RACE(rn)->name);
		return FALSE;
	}

	race		= race_new();
	race->name	= str_dup(arg);
	race->file_name	= str_printf("race%03d.race", races.nused-1);

	ch->desc->pEdit	= (void *)race;
	OLCED(ch)	= olced_lookup(ED_RACE);
	touch_race(race);
	char_puts("Race created.\n",ch);
	return FALSE;
}

OLC_FUN(raceed_edit)
{
	int rn;

	if (PC(ch)->security < SECURITY_RACE) {
		char_puts("RaceEd: Insufficient security.\n", ch);
		return FALSE;
	}

	if (argument[0] == '\0') {
		dofun("help", ch, "'OLC EDIT'");
		return FALSE;
	}

	if ((rn = rn_lookup(argument)) < 0) {
		char_printf(ch, "RaceEd: %s: No such race.\n", argument);
		return FALSE;
	}

	ch->desc->pEdit	= RACE(rn);
	OLCED(ch)	= olced_lookup(ED_RACE);
	return FALSE;
}

OLC_FUN(raceed_save)
{
	int i;
	FILE *fp;
	bool found = FALSE;

	fp = olc_fopen(RACES_PATH, RACE_LIST, ch, SECURITY_RACE);
	if (fp == NULL)
		return FALSE;

	olc_printf(ch, "Saved races:");

	for (i = 0; i < races.nused; i++) {
		fprintf(fp, "%s\n", RACE(i)->file_name);
		if (IS_SET(RACE(i)->race_flags, RACE_CHANGED)) {
			save_race(ch, RACE(i));
			found = TRUE;
		}
	}

	fprintf(fp, "$\n");
	fclose(fp);

	if (!found)
		olc_printf(ch, "    None.");
	return FALSE;
}

OLC_FUN(raceed_touch)
{
	race_t *race;
	EDIT_RACE(ch, race);
	return touch_race(race);
}

OLC_FUN(raceed_show)
{
	int i;
	BUFFER *output;
	race_t *race;
	bool found;

	if (argument[0] == '\0') {
		if (IS_EDIT(ch, ED_RACE))
			EDIT_RACE(ch, race);
		else {
			dofun("help", ch, "'OLC ASHOW'");
			return FALSE;
		}
	}
	else {
		if ((i = rn_lookup(argument)) < 0) {
			char_printf(ch, "RaceEd: %s: No such race.\n", argument);
			return FALSE;
		}
		race = RACE(i);
	}

	output = buf_new(-1);
	buf_printf(output,
		   "Name:          [%s]\n"
		   "Filename:      [%s]\n",
		   race->name,
		   race->file_name);
	if (race->act)
		buf_printf(output, "Act flags:     [%s]\n",
			   flag_string(act_flags, race->act));
	if (race->aff)
		buf_printf(output, "Aff flags:     [%s]\n",
			   flag_string(affect_flags, race->aff));
	if (race->off)
		buf_printf(output, "Off flags:     [%s]\n",
			   flag_string(off_flags, race->off));
	if (race->imm)
		buf_printf(output, "Imm flags:     [%s]\n",
			   flag_string(imm_flags, race->imm));
	if (race->res)
		buf_printf(output, "Res flags:     [%s]\n",
			   flag_string(res_flags, race->res));
	if (race->vuln)
		buf_printf(output, "Vuln flags:    [%s]\n",
			   flag_string(vuln_flags, race->vuln));
	if (race->form)
		buf_printf(output, "Form:          [%s]\n",
			   flag_string(form_flags, race->form));
	if (race->parts)
		buf_printf(output, "Parts:         [%s]\n",
			   flag_string(part_flags, race->parts));
	if (race->race_flags)
		buf_printf(output, "General flags: [%s]\n",
			   flag_string(race_flags, race->race_flags));
	if (!race->race_pcdata) {               
		buf_add(output, "=== No PC race defined ===\n");
		page_to_char(buf_string(output), ch);
		buf_free(output);
		return FALSE;
        }

	buf_add(output, "=== PC race data ===\n");
	if (race->race_pcdata->who_name)
		buf_printf(output, "WHO name:      [%s]\n",
			   race->race_pcdata->who_name);
	if (race->race_pcdata->points)
		buf_printf(output, "Extra exp:     [%d]\n",
			   race->race_pcdata->points);
	if (race->race_pcdata->bonus_skills)
		buf_printf(output, "Bonus skills:  [%s]\n",
			   race->race_pcdata->bonus_skills);
	for (i = 0, found = FALSE; i < MAX_STATS; i++)
		if (race->race_pcdata->stats[i]) found = TRUE;
	if (found) {
		buf_add(output, "Stats mod:     [");
		for (i = 0; i < MAX_STATS; i++)
			buf_printf(output, "%s: %2d ",
				   flag_string(stat_names, i),
				   race->race_pcdata->stats[i]);
		buf_add(output, "]\n");
	}
	for (i = 0, found = FALSE; i < MAX_STATS; i++)
		if (race->race_pcdata->max_stats[i]) found = TRUE;
	if (found) {
		buf_add(output, "Max stats:     [");
		for (i = 0; i < MAX_STATS; i++)
			buf_printf(output, "%s: %2d ",
				   flag_string(stat_names, i),
				   race->race_pcdata->max_stats[i]);
		buf_add(output, "]\n");
	}
	buf_printf(output, "Size:          [%s]\n",
		   flag_string(size_table, race->race_pcdata->size));
	if (race->race_pcdata->hp_bonus)
		buf_printf(output, "HP bonus:      [%d]\n",
			   race->race_pcdata->hp_bonus);
	if (race->race_pcdata->mana_bonus)
		buf_printf(output, "Mana bonus:    [%d]\n",
			   race->race_pcdata->mana_bonus);
	if (race->race_pcdata->prac_bonus)
		buf_printf(output, "Prac bonus:    [%d]\n",
			   race->race_pcdata->prac_bonus);
	buf_printf(output, "Spoken lang:   [%s]\n",
		   flag_string(slang_table, race->race_pcdata->slang));
	if (race->race_pcdata->restrict_align)
		buf_printf(output, "Align restrict:[%s]\n",
			   flag_string(ralign_names, race->race_pcdata->restrict_align));
	if (race->race_pcdata->restrict_ethos)
		buf_printf(output, "Ethos restrict:[%s]\n",
			   flag_string(ethos_table, race->race_pcdata->restrict_ethos));
       	for (i = 0; i < race->race_pcdata->classes.nused; i++) {
		rclass_t *rc = VARR_GET(&race->race_pcdata->classes, i);
		int c;

		if (rc->name == NULL
		||  (c = cn_lookup(rc->name)) == -1)
			continue;
		buf_printf(output, "Class '%s' (exp %d%%)\n",
			   rc->name, rc->mult);
	}
	for (i = 0; i < race->race_pcdata->skills.nused; i++) {
		rskill_t *rs = VARR_GET(&race->race_pcdata->skills, i);
		skill_t *sk;

		if (rs->sn <= 0
		||  (sk = skill_lookup(rs->sn)) == NULL)
			continue;
		buf_printf(output, "Skill '%s' (level %d)\n",
			   sk->name, rs->level);
	}

	page_to_char(buf_string(output), ch);
	buf_free(output);

	return FALSE;
}

OLC_FUN(raceed_list)
{
	int i, col = 0;
	BUFFER	*buffer;

	buffer = buf_new(-1);
	for (i = 0; i < races.nused; i++) {
		buf_printf(buffer, "[%3d] %-18.17s", i, RACE(i)->name);
		if (++col % 3 == 0) buf_add(buffer, "\n");
	}
	if (col % 3) buf_add(buffer, "\n");
	page_to_char(buf_string(buffer), ch);
	buf_free(buffer);
	return FALSE;
}

OLC_FUN(raceed_name)
{
	race_t *race;
	EDIT_RACE(ch, race);
	return olced_str(ch, argument, cmd, &race->name);
}

OLC_FUN(raceed_filename)
{
	race_t *race;
	EDIT_RACE(ch, race);
	return olced_str(ch, argument, cmd, &race->file_name);
}

OLC_FUN(raceed_act)
{
	race_t *race;
	EDIT_RACE(ch, race);
	return olced_flag64(ch, argument, cmd, &race->act);
}

OLC_FUN(raceed_affect)
{
	race_t *race;
	EDIT_RACE(ch, race);
	return olced_flag64(ch, argument, cmd, &race->aff);
}

OLC_FUN(raceed_off)
{
	race_t *race;
	EDIT_RACE(ch, race);
	return olced_flag32(ch, argument, cmd, &race->off);
}

OLC_FUN(raceed_imm)
{
	race_t *race;
	EDIT_RACE(ch, race);
	return olced_flag32(ch, argument, cmd, &race->imm);
}

OLC_FUN(raceed_res)
{
	race_t *race;
	EDIT_RACE(ch, race);
	return olced_flag32(ch, argument, cmd, &race->res);
}

OLC_FUN(raceed_vuln)
{
	race_t *race;
	EDIT_RACE(ch, race);
	return olced_flag32(ch, argument, cmd, &race->vuln);
}

OLC_FUN(raceed_form)
{
	race_t *race;
	EDIT_RACE(ch, race);
	return olced_flag32(ch, argument, cmd, &race->form);
}

OLC_FUN(raceed_parts)
{
	race_t *race;
	EDIT_RACE(ch, race);
	return olced_flag32(ch, argument, cmd, &race->parts);
}

OLC_FUN(raceed_flags)
{
	race_t *race;
	EDIT_RACE(ch, race);
	return olced_flag32(ch, argument, cmd, &race->race_flags);
}

OLC_FUN(raceed_addpcdata)
{
	int i;
	const char *str;
	race_t *race;
	EDIT_RACE(ch, race);
	if (race->race_pcdata) {
		char_puts("RaceEd: Race already has PC race data.\n", ch);
		return FALSE;
	}
	race->race_pcdata = pcrace_new();
	str = str_dup(race->race_pcdata->who_name);
	if (olced_str(ch, argument, cmd, &str)) {
		for (i = 0; i < strlen(str); i++)
			race->race_pcdata->who_name[i] = str[i];
		race->race_pcdata->who_name[i] = '\0';
		char_puts("PC race data created.\n", ch);
		return TRUE;
	}
	pcrace_free(race->race_pcdata);
	race->race_pcdata = NULL;
	return FALSE;
}

OLC_FUN(raceed_delpcdata)
{
	race_t *race;
	EDIT_RACE(ch, race);
	if (!race->race_pcdata) {
		char_puts("RaceEd: Race has no PC race data.\n", ch);
		return FALSE;
	}
	pcrace_free(race->race_pcdata);
	race->race_pcdata = NULL;
	char_puts("PC race data deleted.\n", ch);
	return TRUE;
}

OLC_FUN(raceed_whoname)
{
	int i;
	const char *str;
	race_t *race;
	EDIT_RACE(ch, race);

	if (!race->race_pcdata) {
		char_puts("RaceEd: no PC race data.\n", ch);
		return FALSE;
	}
	str = str_dup(race->race_pcdata->who_name);
	if (olced_str(ch, argument, cmd, &str)) {
		for (i = 0; i < strlen(str); i++)
			race->race_pcdata->who_name[i] = str[i];
		race->race_pcdata->who_name[i] = '\0';
		free_string(str);
		return TRUE;
		}
	free_string(str);
	return FALSE;
}

OLC_FUN(raceed_points)
{
	race_t *race;
	EDIT_RACE(ch, race);

	return olced_number(ch, argument, cmd, &race->race_pcdata->points);
}

OLC_FUN(raceed_bonusskill)
{
	race_t *race;
	const char *buff;
	int sn;

	EDIT_RACE(ch, race);
	if ((sn = sn_lookup(argument)) <= 0) {
		char_printf(ch, "RaceEd: %s: no such skill.\n", argument);
		return FALSE;
	}
	buff = str_printf("'%s'", SKILL(sn)->name);
	return olced_name(ch, buff, cmd, &race->race_pcdata->bonus_skills);
}

OLC_FUN(raceed_stats)
{
	race_t *race;
        char arg[MAX_STRING_LENGTH];
	char *endptr;
	int i, val;
	bool st = FALSE;

	EDIT_RACE(ch, race);
	for (i = 0; i < MAX_STATS; i++) {
		argument = one_argument(argument, arg, sizeof(arg));
		if (*arg == '\0') break;
		val = strtol(arg, &endptr, 0);
		if (*arg == '\0' || *endptr != '\0') break;
		race->race_pcdata->stats[i] = val;
		st = TRUE;
	}
	
	if (!st) char_printf(ch, "Syntax: %s attr1 attr2 ...\n", cmd->name);
		else char_puts("Ok.\n", ch);
	return st;
}

OLC_FUN(raceed_maxstats)
{
	race_t *race;
        char arg[MAX_STRING_LENGTH];
	char *endptr;
	int i, val;
	bool st = FALSE;

	EDIT_RACE(ch, race);
	for (i = 0; i < MAX_STATS; i++) {
		argument = one_argument(argument, arg, sizeof(arg));
		if (*arg == '\0') break;
		val = strtol(arg, &endptr, 0);
		if (*arg == '\0' || *endptr != '\0') break;
		race->race_pcdata->max_stats[i] = val;
		st = TRUE;
	}
	
	if (!st) char_printf(ch, "Syntax: %s attr1 attr2 ...\n", cmd->name);
		else char_puts("Ok.\n", ch);
	return st;
}

OLC_FUN(raceed_size)
{
	race_t *race;
	EDIT_RACE(ch, race);

	return olced_flag32(ch, argument, cmd, &race->race_pcdata->size);
}

OLC_FUN(raceed_hpbonus)
{
	race_t *race;
	EDIT_RACE(ch, race);

	return olced_number(ch, argument, cmd, &race->race_pcdata->hp_bonus);
}

OLC_FUN(raceed_manabonus)
{
	race_t *race;
	EDIT_RACE(ch, race);

	return olced_number(ch, argument, cmd, &race->race_pcdata->mana_bonus);
}

OLC_FUN(raceed_pracbonus)
{
	race_t *race;
	EDIT_RACE(ch, race);

	return olced_number(ch, argument, cmd, &race->race_pcdata->prac_bonus);
}

OLC_FUN(raceed_slang)
{
	race_t *race;
	EDIT_RACE(ch, race);

	return olced_flag32(ch, argument, cmd, &race->race_pcdata->slang);
}

OLC_FUN(raceed_align)
{
	race_t *race;
	EDIT_RACE(ch, race);

	return olced_flag32(ch, argument, cmd, &race->race_pcdata->restrict_align);
}

OLC_FUN(raceed_ethos)
{
	race_t *race;
	EDIT_RACE(ch, race);

	return olced_flag32(ch, argument, cmd, &race->race_pcdata->restrict_ethos);
}

OLC_FUN(raceed_addclass)
{
	int cn;
	rclass_t *rc;
	char	arg1[MAX_STRING_LENGTH];
	char	arg2[MAX_STRING_LENGTH];
	race_t *race;
	EDIT_RACE(ch, race);

	argument = one_argument(argument, arg1, sizeof(arg1));
	           one_argument(argument, arg2, sizeof(arg2));

	if (arg1[0] == '\0' || arg2[0] == '\0') {
		dofun("help", ch, "'OLC RACE CLASS'");
		return FALSE;
	}

	if ((cn = cn_lookup(arg1)) < 0) {
		char_printf(ch, "RaceEd: %s: unknown class.\n", arg1);
		return FALSE;
	}

	if ((rc = rclass_lookup(race, CLASS(cn)->name))) {
		char_printf(ch, "RaceEd: %s: already there.\n",
			    CLASS(cn)->name);
		return FALSE;
	}

	rc = varr_enew(&race->race_pcdata->classes);
	rc->name = CLASS(cn)->name;
	rc->mult = atoi(arg2);
	varr_qsort(&race->race_pcdata->classes, cmpstr);
        char_puts("Ok.\n", ch);
	return TRUE;
}

OLC_FUN(raceed_delclass)
{
	char arg[MAX_STRING_LENGTH];
	rclass_t *rc;
	race_t *race;
	int cn;

	EDIT_RACE(ch, race);

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		dofun("help", ch, "'OLC RACE CLASS'");
		return FALSE;
	}

	if ((cn = cn_lookup(arg)) < 0) {
		char_printf(ch, "RaceEd: %s: unknown class.\n", arg);
		return FALSE;
	}

	if ((rc = rclass_lookup(race, CLASS(cn)->name)) == NULL) {
		char_printf(ch, "RaceEd: %s: not found in race class list.\n",
			    CLASS(cn)->name);
		return FALSE;
	}
	rc->name = str_dup(str_empty);
	varr_qsort(&race->race_pcdata->classes, cmpstr);
        char_puts("Ok.\n", ch);
	return TRUE;
}

OLC_FUN(raceed_addskill)
{
	int sn;
	rskill_t *rs;
	char	arg1[MAX_STRING_LENGTH];
	char	arg2[MAX_STRING_LENGTH];
	race_t *race;
	EDIT_RACE(ch, race);

	argument = one_argument(argument, arg1, sizeof(arg1));
	           one_argument(argument, arg2, sizeof(arg2));

	if (arg1[0] == '\0' || arg2[0] == '\0') {
		dofun("help", ch, "'OLC RACE SKILL'");
		return FALSE;
	}

	if ((sn = sn_lookup(arg1)) <= 0) {
		char_printf(ch, "RaceEd: %s: unknown skill.\n", arg1);
		return FALSE;
	}

	if ((rs = rskill_lookup(race, sn))) {
		char_printf(ch, "RaceEd: %s: already there.\n",
			    SKILL(sn)->name);
		return FALSE;
	}

	rs = varr_enew(&race->race_pcdata->skills);
	rs->sn = sn;
	rs->level = atoi(arg2);
	varr_qsort(&race->race_pcdata->skills, cmpint);
        char_puts("Ok.\n", ch);
	return TRUE;
}

OLC_FUN(raceed_delskill)
{
	int sn;
	char arg[MAX_STRING_LENGTH];
	rskill_t *rs;
	race_t *race;
	EDIT_RACE(ch, race);

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		dofun("help", ch, "'OLC RACE SKILL'");
		return FALSE;
	}

	if ((sn = sn_lookup(arg)) <= 0) {
		char_printf(ch, "RaceEd: %s: unknown skill.\n", arg);
		return FALSE;
	}

	if ((rs = rskill_lookup(race, sn)) == NULL) {
		char_printf(ch, "RaceEd: %s: not found in race skill list.\n",
			    arg);
		return FALSE;
	}
	rs->sn = 0;
	varr_qsort(&race->race_pcdata->skills, cmpint);
        char_puts("Ok.\n", ch);
	return TRUE;
}

OLC_FUN(olc_skill_update)
{
	CHAR_DATA *gch;

	for (gch = char_list; gch; gch = gch->next) {
		if (!IS_NPC(gch)) update_skills(gch);
	}
	char_puts("Active players' skills updated.\n", ch);
	return FALSE;
}

bool touch_race(race_t *race)
{
	SET_BIT(race->race_flags, RACE_CHANGED);
	return FALSE;
}

static VALIDATE_FUN(validate_name)
{
	int i;
	race_t *race;
	EDIT_RACE(ch, race);

	for (i = 0; i < races.nused; i++)
		if (RACE(i) != race
		&&  !str_cmp(RACE(i)->name, arg)) {
			char_printf(ch, "RaceEd: %s: duplicate race name.\n",
				    arg);
			return FALSE;
		}

	return TRUE;
}

static VALIDATE_FUN(validate_whoname)
{
	int i;
	race_t *race;
	EDIT_RACE(ch, race);

	if (strlen(arg) > 5 || strlen(arg) < 1) {
		char_puts("RaceEd: whoname should be 1..5 symbols long.\n", ch);
		return FALSE;
	}
	for (i = 0; i < races.nused; i++)
		if (RACE(i) != race
		&&  race->race_pcdata
		&&  !str_cmp(RACE(i)->race_pcdata->who_name, arg)) {
			char_printf(ch, "RaceEd: %s: duplicate race whoname.\n",
				    arg);
			return FALSE;
		}
	return TRUE;
}

static VALIDATE_FUN(validate_haspcdata)
{
	race_t *race;
	EDIT_RACE(ch, race);

	if (race->race_pcdata != NULL) return TRUE;
	char_puts("Add PC race data first.\n", ch);
	return FALSE;
}

static void save_race(CHAR_DATA *ch, race_t *race)
{
	int i;
	FILE *fp;
	bool found;

	if ((fp = olc_fopen(RACES_PATH, race->file_name, ch, -1)) == NULL)
		return;

	fprintf(fp, "#RACE\n");
	fprintf(fp, "Name %s~\n", race->name);

	REMOVE_BIT(race->race_flags, RACE_CHANGED);
	if (race->act)
		fprintf(fp, "Act %s~\n",
			flag_string(act_flags, race->act));
	if (race->aff)
		fprintf(fp, "Aff %s~\n",
			flag_string(affect_flags, race->aff));
	if (race->off)
		fprintf(fp, "Off %s~\n",
			flag_string(off_flags, race->off));
	if (race->imm)
		fprintf(fp, "Imm %s~\n",
			flag_string(imm_flags, race->imm));
	if (race->res)
		fprintf(fp, "Res %s~\n",
			flag_string(res_flags, race->res));
	if (race->vuln)
		fprintf(fp, "Vuln %s~\n",
			flag_string(vuln_flags, race->vuln));
	if (race->form)
		fprintf(fp, "Form %s~\n",
			flag_string(form_flags, race->form));
	if (race->parts)
		fprintf(fp, "Parts %s~\n",
			flag_string(part_flags, race->parts));
	if (race->race_flags)
		fprintf(fp, "Flags %s~\n",
			flag_string(race_flags, race->race_flags));
	fprintf(fp, "End\n\n");

	if (!race->race_pcdata) {
		fprintf(fp, "#$\n");
		fclose(fp);
		olc_printf(ch, "    %s (%s)", race->name, race->file_name);
		return;
	}
	
	fprintf(fp, "#PCRACE\n");
	if (race->race_pcdata->who_name)
		fprintf(fp, "Shortname %s~\n",
			race->race_pcdata->who_name);
	if (race->race_pcdata->points)
		fprintf(fp, "Points %d\n",
			race->race_pcdata->points);
	for (i = 0; i < race->race_pcdata->classes.nused; i++) {
		rclass_t *rc = VARR_GET(&race->race_pcdata->classes, i);
		int c;

		if (rc->name == NULL
		||  (c = cn_lookup(rc->name)) == -1)
			continue;
		fprintf(fp, "Class '%s' %d\n",
			rc->name, rc->mult);
	}
	if (!IS_NULLSTR(race->race_pcdata->bonus_skills))
		fprintf(fp, "BonusSkills %s~\n",
			race->race_pcdata->bonus_skills);
	for (i = 0; i < race->race_pcdata->skills.nused; i++) {
		rskill_t *rs = VARR_GET(&race->race_pcdata->skills, i);
		skill_t *sk;

		if (rs->sn <= 0
		||  (sk = skill_lookup(rs->sn)) == NULL)
			continue;
		fprintf(fp, "Skill '%s' %d\n",
			sk->name, rs->level);
	}
	for (i = 0, found = FALSE; i < MAX_STATS; i++)
		if (race->race_pcdata->stats[i]) found = TRUE;
	if (found) {
		fprintf(fp, "Stats ");
		for (i = 0; i < MAX_STATS; i++)
			fprintf(fp, "%d ", race->race_pcdata->stats[i]);
		fprintf(fp, "\n");
	}
	for (i = 0, found = FALSE; i < MAX_STATS; i++)
		if (race->race_pcdata->max_stats[i]) found = TRUE;
	if (found) {
		fprintf(fp, "MaxStats ");
		for (i = 0; i < MAX_STATS; i++)
			fprintf(fp, "%d ",
				race->race_pcdata->max_stats[i]);
		fprintf(fp, "\n");
	}
	fprintf(fp, "Size %s\n",
		flag_string(size_table, race->race_pcdata->size));
	if (race->race_pcdata->hp_bonus)
		fprintf(fp, "HPBonus %d\n",
			race->race_pcdata->hp_bonus);
	if (race->race_pcdata->mana_bonus)
		fprintf(fp, "ManaBonus %d\n",
			race->race_pcdata->mana_bonus);
	if (race->race_pcdata->prac_bonus)
		fprintf(fp, "PracBonus %d\n",
			race->race_pcdata->prac_bonus);
	if (race->race_pcdata->restrict_align)
		fprintf(fp, "RestrictAlign %s~\n",
			flag_string(ralign_names, race->race_pcdata->restrict_align));
	if (race->race_pcdata->restrict_ethos)
		fprintf(fp, "RestrictEthos %s~\n",
			flag_string(ethos_table, race->race_pcdata->restrict_ethos));
	fprintf(fp, "Slang %s\n",
		flag_string(slang_table, race->race_pcdata->slang));

	fprintf(fp, "End\n\n#$\n");
	fclose(fp);
	olc_printf(ch, "    %s (%s)", race->name, race->file_name);
}
