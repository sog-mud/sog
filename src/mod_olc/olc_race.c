/*-
 * Copyright (c) 1999 SoG Development Team
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
 * $Id: olc_race.c,v 1.59 2001-09-23 16:24:18 fjoe Exp $
 */

#include "olc.h"

#define EDIT_RACE(ch, race)	(race = (race_t *) ch->desc->pEdit)

DECLARE_OLC_FUN(raceed_create		);
DECLARE_OLC_FUN(raceed_edit		);
DECLARE_OLC_FUN(raceed_save		);
DECLARE_OLC_FUN(raceed_touch		);
DECLARE_OLC_FUN(raceed_show		);
DECLARE_OLC_FUN(raceed_list		);

DECLARE_OLC_FUN(raceed_act		);
DECLARE_OLC_FUN(raceed_affect		);
DECLARE_OLC_FUN(raceed_invis		);
DECLARE_OLC_FUN(raceed_detect		);
DECLARE_OLC_FUN(raceed_off		);
DECLARE_OLC_FUN(raceed_form		);
DECLARE_OLC_FUN(raceed_parts		);
DECLARE_OLC_FUN(raceed_flags		);
DECLARE_OLC_FUN(raceed_resists		);
DECLARE_OLC_FUN(raceed_damtype		);

DECLARE_OLC_FUN(raceed_addpcdata	);
DECLARE_OLC_FUN(raceed_delpcdata	);

DECLARE_OLC_FUN(raceed_whoname		);
DECLARE_OLC_FUN(raceed_points		);
DECLARE_OLC_FUN(raceed_skillspec	);
DECLARE_OLC_FUN(raceed_bonusskill	);
DECLARE_OLC_FUN(raceed_stats		);
DECLARE_OLC_FUN(raceed_maxstats		);
DECLARE_OLC_FUN(raceed_size		);
DECLARE_OLC_FUN(raceed_hpbonus		);
DECLARE_OLC_FUN(raceed_manabonus	);
DECLARE_OLC_FUN(raceed_pracbonus	);
DECLARE_OLC_FUN(raceed_luckbonus	);
DECLARE_OLC_FUN(raceed_slang		);
DECLARE_OLC_FUN(raceed_align		);
DECLARE_OLC_FUN(raceed_ethos		);
DECLARE_OLC_FUN(raceed_class		);
DECLARE_OLC_FUN(raceed_addaffect	);
DECLARE_OLC_FUN(raceed_delaffect	);
DECLARE_OLC_FUN(raceed_hungerrate	);

DECLARE_OLC_FUN(olc_skill_update	);

static DECLARE_VALIDATE_FUN(validate_whoname);
static DECLARE_VALIDATE_FUN(validate_haspc);

olced_strkey_t strkey_races = { &races, RACES_PATH, RACE_EXT };

olc_cmd_t olc_cmds_race[] =
{
	{ "create",	raceed_create,	NULL,		NULL		},
	{ "edit",	raceed_edit,	NULL,		NULL		},
	{ "",		raceed_save,	NULL,		NULL		},
	{ "touch",	raceed_touch,	NULL,		NULL		},
	{ "show",	raceed_show,	NULL,		NULL		},
	{ "list",	raceed_list,	NULL,		NULL		},

	{ "name",	olced_strkey,	NULL,		&strkey_races	},
	{ "act",	raceed_act,	NULL,		mob_act_flags	},
	{ "affect",	raceed_affect,	NULL,		affect_flags	},
	{ "invis",	raceed_invis,	NULL,		id_flags	},
	{ "detect",	raceed_detect,	NULL,		id_flags	},
	{ "off",	raceed_off,	NULL,		off_flags	},
	{ "form",	raceed_form,	NULL,		form_flags	},
	{ "parts",	raceed_parts,	NULL,		part_flags	},
	{ "flags",	raceed_flags,	NULL,		race_flags	},

	{ "resists",	raceed_resists,	NULL,		NULL		},
	{ "damtype",	raceed_damtype,	NULL,		NULL		},

	{ "addpcdata",	raceed_addpcdata, validate_whoname, NULL	},
	{ "delpcdata",	raceed_delpcdata, NULL,		NULL		},

	{ "whoname",	raceed_whoname,	validate_whoname, NULL		},
	{ "points",	raceed_points,	validate_haspc,	NULL		},
	{ "skillspec",	raceed_skillspec, validate_skill_spec, NULL	},
	{ "bonusskill",	raceed_bonusskill, validate_haspc, NULL		},
	{ "stats",	raceed_stats,	NULL,		NULL		},
	{ "maxstats",	raceed_maxstats, NULL,		NULL		},
	{ "size",	raceed_size,	validate_haspc, size_table	},
	{ "hpbonus",	raceed_hpbonus,	validate_haspc,	NULL		},
	{ "manabonus",	raceed_manabonus, validate_haspc, NULL		},
	{ "pracbonus",	raceed_pracbonus, validate_haspc, NULL		},
	{ "luckbonus",	raceed_luckbonus, NULL,		NULL		},
	{ "slang",	raceed_slang,	validate_haspc,	slang_table	},
	{ "align",	raceed_align,	validate_haspc,	ralign_names	},
	{ "ethos",	raceed_ethos,	validate_haspc,	ethos_table	},
	{ "class",	raceed_class,	NULL,		NULL		},
	{ "addaffect",	raceed_addaffect, NULL,		NULL		},
	{ "delaffect",	raceed_delaffect, NULL,		NULL		},
	{ "hungerrate",	raceed_hungerrate, validate_haspc, NULL		},

	{ "update",	olc_skill_update, NULL,		NULL		},

	{ "commands",	show_commands,	NULL,		NULL		},
	{ "version",	show_version,	NULL,		NULL		},

	{ NULL, NULL, NULL, NULL }
};

static DECLARE_FOREACH_CB_FUN(save_race_cb);
static DECLARE_FOREACH_CB_FUN(dump_race_cb);

OLC_FUN(raceed_create)
{
	race_t *r;
	char arg[MAX_INPUT_LENGTH];

	if (PC(ch)->security < SECURITY_RACE) {
		act_char("RaceEd: Insufficient security for creating races.", ch);
		return FALSE;
	}

	first_arg(argument, arg, sizeof(arg), FALSE);
	if (arg[0] == '\0')
		OLC_ERROR("'OLC CREATE'");

	if ((r = c_insert(&races, arg)) == NULL) {
		act_puts("RaceEd: $t: already exists.",
			 ch, arg, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	r->name = str_dup(arg);
	ch->desc->pEdit	= r;
	OLCED(ch) = olced_lookup(ED_RACE);
	touch_race(r);
	act_char("Race created.", ch);
	return FALSE;
}

OLC_FUN(raceed_edit)
{
	race_t *r;

	if (PC(ch)->security < SECURITY_RACE) {
		act_char("RaceEd: Insufficient security.", ch);
		return FALSE;
	}

	if (argument[0] == '\0')
		OLC_ERROR("'OLC EDIT'");

	if ((r = race_search(argument)) == 0) {
		act_puts("RaceEd: $t: No such race.",
			 ch, argument, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	ch->desc->pEdit	= r;
	OLCED(ch)	= olced_lookup(ED_RACE);
	return FALSE;
}

OLC_FUN(raceed_save)
{
	bool found = FALSE;

	olc_printf(ch, "Saved races:");
	c_foreach(&races, save_race_cb, ch, &found);
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
	size_t i;
	BUFFER *output;
	race_t *r;
	bool found;

	if (argument[0] == '\0') {
		if (IS_EDIT(ch, ED_RACE))
			EDIT_RACE(ch, r);
		else
			OLC_ERROR("'OLC ASHOW'");
	} else {
		if ((r = race_search(argument)) == NULL) {
			act_puts("RaceEd: $t: No such race.",
				 ch, argument, NULL,
				 TO_CHAR | ACT_NOTRANS, POS_DEAD);
			return FALSE;
		}
	}

	output = buf_new(0);
	buf_printf(output, BUF_END, "Name:          [%s]\n", r->name);
	if (r->act)
		buf_printf(output, BUF_END, "Act flags:     [%s]\n",
			   flag_string(mob_act_flags, r->act));
	if (r->aff)
		buf_printf(output, BUF_END, "Aff flags:     [%s]\n",
			   flag_string(affect_flags, r->aff));
	if (r->has_invis)
		buf_printf(output, BUF_END, "Invis flags:   [%s]\n",
			   flag_string(id_flags, r->has_invis));
	if (r->has_detect)
		buf_printf(output, BUF_END, "Detect flags:  [%s]\n",
			   flag_string(id_flags, r->has_detect));
	if (r->off)
		buf_printf(output, BUF_END, "Off flags:     [%s]\n",
			   flag_string(off_flags, r->off));
	if (r->form)
		buf_printf(output, BUF_END, "Form:          [%s]\n",
			   flag_string(form_flags, r->form));
	if (r->parts)
		buf_printf(output, BUF_END, "Parts:         [%s]\n",
			   flag_string(part_flags, r->parts));
	if (r->race_flags)
		buf_printf(output, BUF_END, "General flags: [%s]\n",
			   flag_string(race_flags, r->race_flags));
	if (str_cmp(r->damtype, "punch"))
		buf_printf(output, BUF_END, "Damage type:   [%s]\n", r->damtype);

	buf_printf(output, BUF_END, "Luck bonus:    [%d]\n", r->luck_bonus);

	dump_resists(output, r->resists);
	aff_dump_list(r->affected, output);

	if (!r->race_pcdata) {
		buf_append(output, "=== No PC race defined ===\n");
		page_to_char(buf_string(output), ch);
		buf_free(output);
		return FALSE;
        }

	buf_append(output, "=== race PC data ===\n");
	if (r->race_pcdata->who_name) {
		buf_printf(output, BUF_END, "WHO name:      [%s]\n",
			   r->race_pcdata->who_name);
	}

	if (r->race_pcdata->points) {
		buf_printf(output, BUF_END, "Extra exp:     [%d]\n",
			   r->race_pcdata->points);
	}

	if (!IS_NULLSTR(r->race_pcdata->skill_spec)) {
		buf_printf(output, BUF_END, "SkillSpec:     [%s]\n",
			   r->race_pcdata->skill_spec);
	}

	if (r->race_pcdata->bonus_skills) {
		buf_printf(output, BUF_END, "Bonus skills:  [%s]\n",
			   r->race_pcdata->bonus_skills);
	}

	for (i = 0, found = FALSE; i < MAX_STAT; i++) {
		if (r->race_pcdata->mod_stat[i])
			found = TRUE;
	}

	if (found) {
		buf_append(output, "Stats mod:     [");
		for (i = 0; i < MAX_STAT; i++)
			buf_printf(output, BUF_END, "%s: %2d ",
				   flag_string(stat_aliases, i),
				   r->race_pcdata->mod_stat[i]);
		buf_append(output, "]\n");
	}

	for (i = 0, found = FALSE; i < MAX_STAT; i++) {
		if (r->race_pcdata->max_stat[i])
			found = TRUE;
	}

	if (found) {
		buf_append(output, "Max stats:     [");
		for (i = 0; i < MAX_STAT; i++)
			buf_printf(output, BUF_END, "%s: %2d ",
				   flag_string(stat_aliases, i),
				   r->race_pcdata->max_stat[i]);
		buf_append(output, "]\n");
	}

	buf_printf(output, BUF_END, "Size:          [%s]\n",
		   flag_string(size_table, r->race_pcdata->size));
	if (r->race_pcdata->hp_bonus) {
		buf_printf(output, BUF_END, "HP bonus:      [%d]\n",
			   r->race_pcdata->hp_bonus);
	}

	buf_printf(output, BUF_END, "Hunger Rate:   [%d]%%\n",
		   r->race_pcdata->hunger_rate);

	if (r->race_pcdata->mana_bonus) {
		buf_printf(output, BUF_END, "Mana bonus:    [%d]\n",
			   r->race_pcdata->mana_bonus);
	}

	if (r->race_pcdata->prac_bonus) {
		buf_printf(output, BUF_END, "Prac bonus:    [%d]\n",
			   r->race_pcdata->prac_bonus);
	}

	buf_printf(output, BUF_END, "Spoken lang:   [%s]\n",
		   flag_string(slang_table, r->race_pcdata->slang));

	if (r->race_pcdata->restrict_align) {
		buf_printf(output, BUF_END, "Restrict align:[%s]\n",
			   flag_string(ralign_names, r->race_pcdata->restrict_align));
	}
	if (r->race_pcdata->restrict_ethos) {
		buf_printf(output, BUF_END, "Restrict ethos:[%s]\n",
			   flag_string(ethos_table, r->race_pcdata->restrict_ethos));
	}

	for (i = 0; i < c_size(&r->race_pcdata->classes); i++) {
		rclass_t *rc = VARR_GET(&r->race_pcdata->classes, i);

		if (rc->name == NULL)
			continue;
		buf_printf(output, BUF_END, "Class '%s' (exp %d%%)",
			   rc->name, rc->mult);
		if (class_lookup(rc->name) == NULL)
			buf_append(output, " (UNDEF)");
		buf_append(output, "\n");
	}

	page_to_char(buf_string(output), ch);
	buf_free(output);

	return FALSE;
}

OLC_FUN(raceed_list)
{
	BUFFER *buffer = buf_new(0);
	c_dump(&races, buffer, dump_race_cb);
	page_to_char(buf_string(buffer), ch);
	buf_free(buffer);
	return FALSE;
}

OLC_FUN(raceed_act)
{
	race_t *race;
	EDIT_RACE(ch, race);
	return olced_flag(ch, argument, cmd, &race->act);
}

OLC_FUN(raceed_affect)
{
	race_t *race;
	EDIT_RACE(ch, race);
	return olced_flag(ch, argument, cmd, &race->aff);
}

OLC_FUN(raceed_invis)
{
	race_t *race;
	EDIT_RACE(ch, race);
	return olced_flag(ch, argument, cmd, &race->has_invis);
}

OLC_FUN(raceed_detect)
{
	race_t *race;
	EDIT_RACE(ch, race);
	return olced_flag(ch, argument, cmd, &race->has_detect);
}

OLC_FUN(raceed_off)
{
	race_t *race;
	EDIT_RACE(ch, race);
	return olced_flag(ch, argument, cmd, &race->off);
}

OLC_FUN(raceed_form)
{
	race_t *race;
	EDIT_RACE(ch, race);
	return olced_flag(ch, argument, cmd, &race->form);
}

OLC_FUN(raceed_parts)
{
	race_t *race;
	EDIT_RACE(ch, race);
	return olced_flag(ch, argument, cmd, &race->parts);
}

OLC_FUN(raceed_flags)
{
	race_t *race;
	EDIT_RACE(ch, race);
	return olced_flag(ch, argument, cmd, &race->race_flags);
}

OLC_FUN(raceed_addpcdata)
{
	const char *str;
	race_t *race;

	EDIT_RACE(ch, race);
	if (race->race_pcdata) {
		act_char("RaceEd: Race already has race PC data.", ch);
		return FALSE;
	}

	race->race_pcdata = pcrace_new();
	str = str_dup(race->race_pcdata->who_name);
	if (olced_str(ch, argument, cmd, &str)) {
		strnzcpy(race->race_pcdata->who_name,
			 sizeof(race->race_pcdata->who_name), str);
		free_string(str);
		act_char("race PC data created.", ch);
		return TRUE;
	}

	free_string(str);
	pcrace_free(race->race_pcdata);
	race->race_pcdata = NULL;
	return FALSE;
}

OLC_FUN(raceed_delpcdata)
{
	race_t *race;

	EDIT_RACE(ch, race);
	if (!race->race_pcdata) {
		act_char("RaceEd: no race PC data.", ch);
		return FALSE;
	}

	pcrace_free(race->race_pcdata);
	race->race_pcdata = NULL;
	act_char("race PC data deleted.", ch);
	return TRUE;
}

OLC_FUN(raceed_whoname)
{
	const char *str;
	race_t *race;

	EDIT_RACE(ch, race);
	if (!race->race_pcdata) {
		act_char("RaceEd: no race PC data.", ch);
		return FALSE;
	}

	str = str_qdup(race->race_pcdata->who_name);
	if (olced_str(ch, argument, cmd, &str)) {
		strnzcpy(race->race_pcdata->who_name,
			 sizeof(race->race_pcdata->who_name), str);
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

OLC_FUN(raceed_skillspec)
{
	race_t *race;

	EDIT_RACE(ch, race);
	if (!race->race_pcdata) {
		act_char("RaceEd: no race PC data.", ch);
		return FALSE;
	}

	return olced_str(ch, argument, cmd, &race->race_pcdata->skill_spec);
}

OLC_FUN(raceed_bonusskill)
{
	race_t *race;
	skill_t *sk;

	EDIT_RACE(ch, race);
	if (!race->race_pcdata) {
		act_char("RaceEd: no race PC data.", ch);
		return FALSE;
	}

	if ((sk = skill_lookup(argument)) == NULL) {
		act_puts("RaceEd: $t: no such skill.",
			 ch, argument, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}
	return olced_name(ch, gmlstr_mval(&sk->sk_name), cmd,
			  &race->race_pcdata->bonus_skills);
}

OLC_FUN(raceed_stats)
{
	race_t *race;
        char arg[MAX_INPUT_LENGTH];
	char *endptr;
	int i, val;
	bool st = FALSE;

	EDIT_RACE(ch, race);
	if (!race->race_pcdata) {
		act_char("RaceEd: no race PC data.", ch);
		return FALSE;
	}

	for (i = 0; i < MAX_STAT; i++) {
		argument = one_argument(argument, arg, sizeof(arg));
		if (*arg == '\0')
			break;
		val = strtol(arg, &endptr, 0);
		if (*arg == '\0' || *endptr != '\0')
			break;
		race->race_pcdata->mod_stat[i] = val;
		st = TRUE;
	}

	if (!st) {
		act_puts("Syntax: $t <attr1> <attr2> ...",
			 ch, cmd->name, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
	} else
		act_char("Ok.", ch);
	return st;
}

OLC_FUN(raceed_maxstats)
{
	race_t *race;
        char arg[MAX_INPUT_LENGTH];
	char *endptr;
	int i, val;
	bool st = FALSE;

	EDIT_RACE(ch, race);
	if (!race->race_pcdata) {
		act_char("RaceEd: no race PC data.", ch);
		return FALSE;
	}

	for (i = 0; i < MAX_STAT; i++) {
		argument = one_argument(argument, arg, sizeof(arg));
		if (*arg == '\0')
			break;
		val = strtol(arg, &endptr, 0);
		if (*arg == '\0' || *endptr != '\0')
			break;
		race->race_pcdata->max_stat[i] = val;
		st = TRUE;
	}

	if (!st) {
		act_puts("Syntax: $t <attr1> <attr2> ...",
			 ch, cmd->name, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
	} else
		act_char("Ok.", ch);
	return st;
}

OLC_FUN(raceed_size)
{
	race_t *race;
	EDIT_RACE(ch, race);
	return olced_flag(ch, argument, cmd, &race->race_pcdata->size);
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
	if (!race->race_pcdata) {
		act_char("RaceEd: no race PC data.", ch);
		return FALSE;
	}

	return olced_number(ch, argument, cmd, &race->race_pcdata->mana_bonus);
}

OLC_FUN(raceed_pracbonus)
{
	race_t *race;

	EDIT_RACE(ch, race);
	if (!race->race_pcdata) {
		act_char("RaceEd: no race PC data.", ch);
		return FALSE;
	}

	return olced_number(ch, argument, cmd, &race->race_pcdata->prac_bonus);
}

OLC_FUN(raceed_luckbonus)
{
	race_t *race;
	EDIT_RACE(ch, race);
	return olced_number(ch, argument, cmd, &race->luck_bonus);
}

OLC_FUN(raceed_slang)
{
	race_t *race;
	EDIT_RACE(ch, race);
	return olced_flag(ch, argument, cmd, &race->race_pcdata->slang);
}

OLC_FUN(raceed_align)
{
	race_t *race;
	EDIT_RACE(ch, race);
	return olced_flag(ch, argument, cmd, &race->race_pcdata->restrict_align);
}

OLC_FUN(raceed_ethos)
{
	race_t *race;
	EDIT_RACE(ch, race);
	return olced_flag(ch, argument, cmd, &race->race_pcdata->restrict_ethos);
}

OLC_FUN(raceed_resists)
{
	race_t *race;
	EDIT_RACE(ch, race);
	return olced_resists(ch, argument, cmd, race->resists);
}

OLC_FUN(raceed_damtype)
{
	damtype_t *d;
	race_t *race;
	char arg[MAX_INPUT_LENGTH];
	EDIT_RACE(ch, race);

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		act_char("Syntax: damtype [damage message]", ch);
		act_char("Syntax: damtype ?", ch);
		return FALSE;
	}

	if (!str_cmp(arg, "?")) {
		BUFFER *output = buf_new(0);
		c_strkey_dump(&damtypes, output);
		page_to_char(buf_string(output), ch);
		buf_free(output);
		return FALSE;
	}

	if ((d = damtype_lookup(arg)) == NULL) {
		act_puts("MobEd: $t: unknown damage class.",
			 ch, arg, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	free_string(race->damtype);
	race->damtype = str_qdup(d->dam_name);
	act_char("Damage type set.", ch);
	return TRUE;
}

static
OLC_FUN(raceed_addclass)
{
	class_t *cl;
	rclass_t *rc;
	char	arg1[MAX_INPUT_LENGTH];
	char	arg2[MAX_INPUT_LENGTH];
	race_t *race;

	EDIT_RACE(ch, race);
	if (!race->race_pcdata) {
		act_char("RaceEd: no race PC data.", ch);
		return FALSE;
	}

	argument = one_argument(argument, arg1, sizeof(arg1));
	           one_argument(argument, arg2, sizeof(arg2));

	if (arg1[0] == '\0' || arg2[0] == '\0')
		OLC_ERROR("'OLC RACE CLASS'");

	if ((cl = class_search(arg1)) == NULL) {
		act_puts("RaceEd: $t: unknown class.",
			 ch, arg1, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	if (rclass_lookup(race, cl->name)) {
		act_puts("RaceEd: $t: already there.",
			 ch, cl->name, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	rc = varr_enew(&race->race_pcdata->classes);
	rc->name = str_qdup(cl->name);
	rc->mult = atoi(arg2);
	varr_qsort(&race->race_pcdata->classes, cmpstr);
        act_char("Ok.", ch);
	return TRUE;
}

static
OLC_FUN(raceed_delclass)
{
	char arg[MAX_INPUT_LENGTH];
	rclass_t *rc;
	race_t *race;

	EDIT_RACE(ch, race);
	if (!race->race_pcdata) {
		act_char("RaceEd: no race PC data.", ch);
		return FALSE;
	}

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0')
		OLC_ERROR("'OLC RACE CLASS'");

	if ((rc = rclass_lookup(race, arg)) == NULL) {
		act_puts("RaceEd: $t: not found in race class list.",
			 ch, arg, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	varr_edelete(&race->race_pcdata->classes, rc);
        act_char("Ok.", ch);
	return TRUE;
}

OLC_FUN(raceed_class)
{
	char arg[MAX_INPUT_LENGTH];

	argument = one_argument(argument, arg, sizeof(arg));
	if (!str_prefix(arg, "add"))
		return raceed_addclass(ch, argument, cmd);
	if (!str_prefix(arg, "delete"))
		return raceed_delclass(ch, argument, cmd);
	OLC_ERROR("'OLC RACE CLASS'");
}

OLC_FUN(raceed_addaffect)
{
	race_t *r;
	EDIT_RACE(ch, r);
	return olced_addaffect(ch, argument, cmd, 0, &r->affected);
}

OLC_FUN(raceed_delaffect)
{
	race_t *r;
	EDIT_RACE(ch, r);
	return olced_delaffect(ch, argument, cmd, &r->affected);
}

OLC_FUN(olc_skill_update)
{
	CHAR_DATA *gch;

	for (gch = char_list; gch && !IS_NPC(gch); gch = gch->next)
		spec_update(gch);
	act_char("Active players' skills updated.", ch);
	return FALSE;
}

OLC_FUN(raceed_hungerrate)
{
	race_t *race;
	EDIT_RACE(ch, race);
	return olced_number(ch, argument, cmd, &race->race_pcdata->hunger_rate);
}

bool
touch_race(race_t *race)
{
	SET_BIT(race->race_flags, RACE_CHANGED);
	return FALSE;
}

static void *
search_whoname_cb(void *p, va_list ap)
{
	race_t *r = (race_t *) p;

	const char *arg = va_arg(ap, const char *);

	if (r->race_pcdata
	&&  !str_cmp(r->race_pcdata->who_name, arg))
		return p;
	return NULL;
}

static
VALIDATE_FUN(validate_whoname)
{
	race_t *r;
	race_t *r2;
	EDIT_RACE(ch, r);

	if (strlen(arg) > 5 || strlen(arg) < 1) {
		act_char("RaceEd: whoname should be 1..5 symbols long.", ch);
		return FALSE;
	}

	if ((r2 = c_foreach(&races, search_whoname_cb, arg)) != NULL
	&&  r2 != r) {
		act_puts("RaceEd: $t: duplicate race whoname.",
			 ch, arg, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	return TRUE;
}

static
VALIDATE_FUN(validate_haspc)
{
	race_t *race;
	EDIT_RACE(ch, race);

	if (race->race_pcdata != NULL)
		return TRUE;
	act_char("Add race PC data first.", ch);
	return FALSE;
}

static void
fwrite_rstats(FILE *fp, const char *name, int *stats)
{
	bool found;
	int i;

	for (i = 0, found = FALSE; i < MAX_STAT; i++) {
		if (stats[i])
			found = TRUE;
	}

	if (!found)
		return;

	fprintf(fp, "%s ", name);
	for (i = 0; i < MAX_STAT; i++)
		fprintf(fp, "%d ", stats[i]);
	fprintf(fp, "\n");
}

static
FOREACH_CB_FUN(save_race_class_cb, p, ap)
{
	rclass_t *rcl = (rclass_t *) p;

	FILE *fp = va_arg(ap, FILE *);

	if (!IS_NULLSTR(rcl->name))
		fprintf(fp, "Class '%s' %d\n", rcl->name, rcl->mult);
	return NULL;
}

static void
save_race_pcdata(pcrace_t *pcr, FILE *fp)
{
	fprintf(fp, "#PCRACE\n");
	fwrite_string(fp, "Shortname", pcr->who_name);
	fwrite_number(fp, "Points", pcr->points);
	c_foreach(&pcr->classes, save_race_class_cb, fp);
	fwrite_word(fp, "SkillSpec", pcr->skill_spec);
	fwrite_string(fp, "BonusSkills", pcr->bonus_skills);
	fwrite_rstats(fp, "Stats", pcr->mod_stat);
	fwrite_rstats(fp, "MaxStats", pcr->max_stat);
	fprintf(fp, "Size %s\n", flag_string(size_table, pcr->size));
	fwrite_number(fp, "HPBonus", pcr->hp_bonus);
	fwrite_number(fp, "ManaBonus", pcr->mana_bonus);
	fwrite_number(fp, "PracBonus", pcr->prac_bonus);
	if (pcr->hunger_rate != 100)
		fwrite_number(fp, "HungerRate", pcr->hunger_rate);
	if (pcr->restrict_align) {
		fprintf(fp, "RestrictAlign %s~\n",
			flag_string(ralign_names, pcr->restrict_align));
	}
	if (pcr->restrict_ethos) {
		fprintf(fp, "RestrictEthos %s~\n",
			flag_string(ethos_table, pcr->restrict_ethos));
	}
	fprintf(fp, "Slang %s\n", flag_string(slang_table, pcr->slang));
	fprintf(fp, "End\n\n");
}

static
FOREACH_CB_FUN(save_race_cb, p, ap)
{
	race_t *r = (race_t *) p;
	int i;

	CHAR_DATA *ch = va_arg(ap, CHAR_DATA *);
	bool *pfound = va_arg(ap, bool *);

	FILE *fp;
	const char *filename;

	if (!IS_SET(r->race_flags, RACE_CHANGED))
		return NULL;

	filename = strkey_filename(r->name, RACE_EXT);
	if ((fp = olc_fopen(RACES_PATH, filename, ch, SECURITY_RACE)) == NULL)
		return NULL;

	fprintf(fp, "#RACE\n");
	fprintf(fp, "Name %s~\n", r->name);

	REMOVE_BIT(r->race_flags, RACE_CHANGED);
	if (r->act)
		fprintf(fp, "Act %s~\n", flag_string(mob_act_flags, r->act));
	if (r->aff)
		fprintf(fp, "Aff %s~\n", flag_string(affect_flags, r->aff));
	if (r->has_invis)
		fprintf(fp, "Inv %s~\n", flag_string(id_flags, r->has_invis));
	if (r->has_detect)
		fprintf(fp, "Det %s~\n", flag_string(id_flags, r->has_detect));
	if (r->off)
		fprintf(fp, "Off %s~\n", flag_string(off_flags, r->off));
	if (r->form)
		fprintf(fp, "Form %s~\n", flag_string(form_flags, r->form));
	if (r->parts)
		fprintf(fp, "Parts %s~\n", flag_string(part_flags, r->parts));
	if (r->race_flags)
		fprintf(fp, "Flags %s~\n", flag_string(race_flags, r->race_flags));
	for (i = 0; i < MAX_RESIST; i++) {
		if (r->resists[i] == RES_UNDEF)
			continue;

		fprintf(fp,"Resist %s %d\n",
			flag_string(dam_classes, i), r->resists[i]);
	}

	if (!!strcmp(r->damtype, "punch"))
		fprintf(fp, "Damtype %s\n", r->damtype);

	if (r->luck_bonus)
		fprintf(fp, "LuckBonus %d\n", r->luck_bonus);

	aff_fwrite_list("Affc", NULL, r->affected, fp, AFF_X_NOLD);

	fprintf(fp, "End\n\n");

	if (r->race_pcdata)
		save_race_pcdata(r->race_pcdata, fp);

	fprintf(fp, "#$\n");
	fclose(fp);

	olc_printf(ch, "    %s (%s)", r->name, filename);
	*pfound = TRUE;
	return NULL;
}

static
FOREACH_CB_FUN(dump_race_cb, p, ap)
{
	race_t *r = (race_t *) p;
	char buf[256];
	const char *pbuf = buf;

	snprintf(buf, sizeof(buf), "%s%s",
		 r->race_pcdata ? "*" : " ", r->name);

	return str_dump_cb(&pbuf, ap);
}
