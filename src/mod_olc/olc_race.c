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
 * $Id: olc_race.c,v 1.34 2000-03-28 06:18:31 fjoe Exp $
 */

#include "olc.h"

#define EDIT_RACE(ch, race)	(race = (race_t*) ch->desc->pEdit)

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
DECLARE_OLC_FUN(raceed_slang		);
DECLARE_OLC_FUN(raceed_align		);
DECLARE_OLC_FUN(raceed_ethos		);
DECLARE_OLC_FUN(raceed_class		);
DECLARE_OLC_FUN(raceed_addaffect	);
DECLARE_OLC_FUN(raceed_delaffect	);

DECLARE_OLC_FUN(olc_skill_update	);

static DECLARE_VALIDATE_FUN(validate_whoname);
static DECLARE_VALIDATE_FUN(validate_haspcdata);

olced_strkey_t strkey_races = { &races, RACES_PATH, RACE_EXT };

olc_cmd_t olc_cmds_race[] =
{
	{ "create",	raceed_create					},
	{ "edit",	raceed_edit					},
	{ "",		raceed_save					},
	{ "touch",	raceed_touch					},
	{ "show",	raceed_show					},
	{ "list",	raceed_list					},

	{ "name",	olced_strkey,	NULL,		&strkey_races	},
	{ "act",	raceed_act,	NULL,		act_flags	},
	{ "affect",	raceed_affect,	NULL,		affect_flags	},
	{ "invis",	raceed_invis,	NULL,		id_flags	},
	{ "detect",	raceed_detect,	NULL,		id_flags	},
	{ "off",	raceed_off,	NULL,		off_flags	},
       	{ "form",	raceed_form,	NULL,		form_flags	},
	{ "parts",	raceed_parts,	NULL,		part_flags	},
	{ "flags",	raceed_flags,	NULL,		race_flags	},

	{ "resists",	raceed_resists					},
	{ "damtype", 	raceed_damtype					},

	{ "addpcdata",	raceed_addpcdata,validate_whoname		},
	{ "delpcdata",	raceed_delpcdata				},

	{ "whoname",	raceed_whoname,	validate_whoname		},
	{ "points",	raceed_points,	validate_haspcdata		},
	{ "skillspec",	raceed_skillspec,validate_skill_spec		},
	{ "bonusskill",	raceed_bonusskill,validate_haspcdata		},
	{ "stats",	raceed_stats,	validate_haspcdata		},
	{ "maxstats",	raceed_maxstats,validate_haspcdata		},
	{ "size",	raceed_size,	NULL,		size_table	},
	{ "hpbonus",	raceed_hpbonus,	validate_haspcdata		},
	{ "manabonus",	raceed_manabonus,validate_haspcdata		},
	{ "pracbonus",	raceed_pracbonus,validate_haspcdata		},
	{ "slang",	raceed_slang,	NULL,		slang_table	},
	{ "align",	raceed_align,	NULL,		ralign_names	},
	{ "ethos",	raceed_ethos,	NULL,		ethos_table	},
	{ "class",	raceed_class					},
	{ "addaffect",	raceed_addaffect				},
	{ "delaffect",	raceed_delaffect				},

	{ "update",	olc_skill_update				},
	{ "commands",	show_commands					},
	{ NULL }
};

static void * save_race_cb(void *p, va_list ap);
static void * print_race_cb(void *p, va_list ap);

OLC_FUN(raceed_create)
{
	race_t *r;
	race_t race;
	char arg[MAX_INPUT_LENGTH];

	if (PC(ch)->security < SECURITY_RACE) {
		char_puts("RaceEd: Insufficient security for creating races.\n",
			  ch);
		return FALSE;
	}

	first_arg(argument, arg, sizeof(arg), FALSE);
	if (arg[0] == '\0')
		OLC_ERROR("'OLC CREATE'");

	/*
	 * olced_busy check is not needed since hash_insert
	 * adds new elements to the end of varr
	 */

	race_init(&race);
	race.name = str_dup(arg);
	r = hash_insert(&races, race.name, &race);
	race_destroy(&race);

	if (r == NULL) {
		char_printf(ch, "RaceEd: %s: already exists.\n", arg);
		return FALSE;
	}

	ch->desc->pEdit	= r;
	OLCED(ch) = olced_lookup(ED_RACE);
	touch_race(r);
	char_puts("Race created.\n",ch);
	return FALSE;
}

OLC_FUN(raceed_edit)
{
	race_t *r;

	if (PC(ch)->security < SECURITY_RACE) {
		char_puts("RaceEd: Insufficient security.\n", ch);
		return FALSE;
	}

	if (argument[0] == '\0')
		OLC_ERROR("'OLC EDIT'");

	if ((r = race_search(argument)) == 0) {
		char_printf(ch, "RaceEd: %s: No such race.\n", argument);
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
	hash_foreach(&races, save_race_cb, ch, &found);
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
	int i, j;
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
			char_printf(ch, "RaceEd: %s: No such race.\n", argument);
			return FALSE;
		}
	}

	output = buf_new(-1);
	buf_printf(output, "Name:          [%s]\n", r->name);
	if (r->act)
		buf_printf(output, "Act flags:     [%s]\n",
			   flag_string(act_flags, r->act));
	if (r->aff)
		buf_printf(output, "Aff flags:     [%s]\n",
			   flag_string(affect_flags, r->aff));
	if (r->has_invis)
		buf_printf(output, "Invis flags:   [%s]\n",
			   flag_string(id_flags, r->has_invis));
	if (r->has_detect)
		buf_printf(output, "Detect flags:  [%s]\n",
			   flag_string(id_flags, r->has_detect));
	if (r->off)
		buf_printf(output, "Off flags:     [%s]\n",
			   flag_string(off_flags, r->off));
	if (r->form)
		buf_printf(output, "Form:          [%s]\n",
			   flag_string(form_flags, r->form));
	if (r->parts)
		buf_printf(output, "Parts:         [%s]\n",
			   flag_string(part_flags, r->parts));
	if (r->race_flags)
		buf_printf(output, "General flags: [%s]\n",
			   flag_string(race_flags, r->race_flags));
	if (str_cmp(r->damtype, "punch"))
		buf_printf(output, "Damage type:   [%s]\n", r->damtype);

	for (i = 0, j = 0; i < MAX_RESIST; i++) {
		if (r->resists[i]) {
			if (!j)
				buf_add(output, "Resists");
			if (strlen(flag_string(dam_classes, i)) > 7)
				buf_printf(output, "\t%s\t%d%%",
					flag_string(dam_classes, i),
					r->resists[i]);
			else 
				buf_printf(output, "\t%s\t\t%d%%",
					flag_string(dam_classes, i),
					r->resists[i]);
			if (!(++j % 3))
				buf_add(output, "\n");
		}
	}
	if (j)
		buf_add(output, "\n");

	aff_dump_list(r->affected, output);

	if (!r->race_pcdata) {               
		buf_add(output, "=== No PC race defined ===\n");
		page_to_char(buf_string(output), ch);
		buf_free(output);
		return FALSE;
        }

	buf_add(output, "=== PC race data ===\n");
	if (r->race_pcdata->who_name)
		buf_printf(output, "WHO name:      [%s]\n",
			   r->race_pcdata->who_name);
	if (r->race_pcdata->points)
		buf_printf(output, "Extra exp:     [%d]\n",
			   r->race_pcdata->points);
	if (!IS_NULLSTR(r->race_pcdata->skill_spec))
		buf_printf(output, "SkillSpec:     [%s]\n",
			   r->race_pcdata->skill_spec);
	if (r->race_pcdata->bonus_skills)
		buf_printf(output, "Bonus skills:  [%s]\n",
			   r->race_pcdata->bonus_skills);
	for (i = 0, found = FALSE; i < MAX_STATS; i++)
		if (r->race_pcdata->mod_stat[i]) found = TRUE;
	if (found) {
		buf_add(output, "Stats mod:     [");
		for (i = 0; i < MAX_STATS; i++)
			buf_printf(output, "%s: %2d ",
				   flag_string(stat_names, i),
				   r->race_pcdata->mod_stat[i]);
		buf_add(output, "]\n");
	}
	for (i = 0, found = FALSE; i < MAX_STATS; i++)
		if (r->race_pcdata->max_stat[i]) found = TRUE;
	if (found) {
		buf_add(output, "Max stats:     [");
		for (i = 0; i < MAX_STATS; i++)
			buf_printf(output, "%s: %2d ",
				   flag_string(stat_names, i),
				   r->race_pcdata->max_stat[i]);
		buf_add(output, "]\n");
	}
	buf_printf(output, "Size:          [%s]\n",
		   flag_string(size_table, r->race_pcdata->size));
	if (r->race_pcdata->hp_bonus)
		buf_printf(output, "HP bonus:      [%d]\n",
			   r->race_pcdata->hp_bonus);
	if (r->race_pcdata->mana_bonus)
		buf_printf(output, "Mana bonus:    [%d]\n",
			   r->race_pcdata->mana_bonus);
	if (r->race_pcdata->prac_bonus)
		buf_printf(output, "Prac bonus:    [%d]\n",
			   r->race_pcdata->prac_bonus);
	buf_printf(output, "Spoken lang:   [%s]\n",
		   flag_string(slang_table, r->race_pcdata->slang));
	if (r->race_pcdata->restrict_align)
		buf_printf(output, "Align restrict:[%s]\n",
			   flag_string(ralign_names, r->race_pcdata->restrict_align));
	if (r->race_pcdata->restrict_ethos)
		buf_printf(output, "Ethos restrict:[%s]\n",
			   flag_string(ethos_table, r->race_pcdata->restrict_ethos));
       	for (i = 0; i < r->race_pcdata->classes.nused; i++) {
		rclass_t *rc = VARR_GET(&r->race_pcdata->classes, i);

		if (rc->name == NULL)
			continue;
		buf_printf(output, "Class '%s' (exp %d%%)",
			   rc->name, rc->mult);
		if (class_lookup(rc->name) == NULL)
			buf_add(output, " (UNDEF)");
		buf_add(output, "\n");
	}

	page_to_char(buf_string(output), ch);
	buf_free(output);

	return FALSE;
}

OLC_FUN(raceed_list)
{
	BUFFER	*buffer = buf_new(-1);
	hash_printall(&races, buffer, print_race_cb);
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
		char_puts("RaceEd: Race already has PC race data.\n", ch);
		return FALSE;
	}
	race->race_pcdata = pcrace_new();
	str = str_dup(race->race_pcdata->who_name);
	if (olced_str(ch, argument, cmd, &str)) {
		strnzcpy(race->race_pcdata->who_name,
			 sizeof(race->race_pcdata->who_name), str);
		free_string(str);
		char_puts("PC race data created.\n", ch);
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
	const char *str;
	race_t *race;
	EDIT_RACE(ch, race);

	if (!race->race_pcdata) {
		char_puts("RaceEd: no PC race data.\n", ch);
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
	if (race->race_pcdata == NULL) {
		char_puts("RaceEd: No PCDATA for this race defined.\n", ch);
		return FALSE;
	}
	return olced_str(ch, argument, cmd, &race->race_pcdata->skill_spec);
}

OLC_FUN(raceed_bonusskill)
{
	race_t *race;
	skill_t *sk;

	EDIT_RACE(ch, race);
	if ((sk = skill_lookup(argument)) == NULL) {
		char_printf(ch, "RaceEd: %s: no such skill.\n", argument);
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
	for (i = 0; i < MAX_STATS; i++) {
		argument = one_argument(argument, arg, sizeof(arg));
		if (*arg == '\0')
			break;
		val = strtol(arg, &endptr, 0);
		if (*arg == '\0' || *endptr != '\0')
			break;
		race->race_pcdata->mod_stat[i] = val;
		st = TRUE;
	}
	
	if (!st)
		char_printf(ch, "Syntax: %s <attr1> <attr2> ...\n", cmd->name);
	else
		char_puts("Ok.\n", ch);
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
	for (i = 0; i < MAX_STATS; i++) {
		argument = one_argument(argument, arg, sizeof(arg));
		if (*arg == '\0')
			break;
		val = strtol(arg, &endptr, 0);
		if (*arg == '\0' || *endptr != '\0')
			break;
		race->race_pcdata->max_stat[i] = val;
		st = TRUE;
	}
	
	if (!st)
		char_printf(ch, "Syntax: %s <attr1> <attr2> ...\n", cmd->name);
	else
		char_puts("Ok.\n", ch);
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
		char_puts("Syntax: damtype [damage message]\n", ch);
		char_puts("Syntax: damtype ?\n", ch);
		return FALSE;
	}

	if (!str_cmp(arg, "?")) {
		BUFFER *output = buf_new(-1);
		strkey_printall(&damtypes, output);
		page_to_char(buf_string(output), ch);
		buf_free(output);
		return FALSE;
	}

	if ((d = damtype_lookup(arg)) == NULL) {
		char_printf(ch, "MobEd: %s: unknown damage class.\n", arg);
		return FALSE;
	}

	free_string(race->damtype);
	race->damtype = str_qdup(d->dam_name);
	char_puts("Damage type set.\n", ch);
	return TRUE;
}

OLC_FUN(raceed_addclass)
{
	class_t *cl;
	rclass_t *rc;
	char	arg1[MAX_INPUT_LENGTH];
	char	arg2[MAX_INPUT_LENGTH];
	race_t *race;
	EDIT_RACE(ch, race);

	argument = one_argument(argument, arg1, sizeof(arg1));
	           one_argument(argument, arg2, sizeof(arg2));

	if (arg1[0] == '\0' || arg2[0] == '\0')
		OLC_ERROR("'OLC RACE CLASS'");

	if ((cl = class_search(arg1)) == NULL) {
		char_printf(ch, "RaceEd: %s: unknown class.\n", arg1);
		return FALSE;
	}

	if (rclass_lookup(race, cl->name)) {
		char_printf(ch, "RaceEd: %s: already there.\n", cl->name);
		return FALSE;
	}

	rc = varr_enew(&race->race_pcdata->classes);
	rc->name = str_qdup(cl->name);
	rc->mult = atoi(arg2);
	varr_qsort(&race->race_pcdata->classes, cmpstr);
        char_puts("Ok.\n", ch);
	return TRUE;
}

OLC_FUN(raceed_delclass)
{
	char arg[MAX_INPUT_LENGTH];
	rclass_t *rc;
	race_t *race;

	EDIT_RACE(ch, race);

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0')
		OLC_ERROR("'OLC RACE CLASS'");

	if ((rc = rclass_lookup(race, arg)) == NULL) {
		char_printf(ch, "RaceEd: %s: not found in race class list.\n",
			    arg);
		return FALSE;
	}

	varr_edelete(&race->race_pcdata->classes, rc);
        char_puts("Ok.\n", ch);
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
	char_puts("Active players' skills updated.\n", ch);
	return FALSE;
}

bool touch_race(race_t *race)
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

static VALIDATE_FUN(validate_whoname)
{
	race_t *r;
	race_t *r2;
	EDIT_RACE(ch, r);

	if (strlen(arg) > 5 || strlen(arg) < 1) {
		char_puts("RaceEd: whoname should be 1..5 symbols long.\n", ch);
		return FALSE;
	}

	if ((r2 = hash_foreach(&races, search_whoname_cb, arg)) != NULL
	&&  r2 != r) {
		char_printf(ch, "RaceEd: %s: duplicate race whoname.\n",
			    (const char *) arg);
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

static void
fwrite_rstats(FILE *fp, const char *name, int *stats)
{
	bool found;
	int i;

	for (i = 0, found = FALSE; i < MAX_STATS; i++) {
		if (stats[i])
			found = TRUE;
	}

	if (!found)
		return;

	fprintf(fp, "%s ", name);
	for (i = 0; i < MAX_STATS; i++)
		fprintf(fp, "%d ", stats[i]);
	fprintf(fp, "\n");
}

static void *
save_race_class_cb(void *p, va_list ap)
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
	varr_foreach(&pcr->classes, save_race_class_cb, fp);
	fwrite_word(fp, "SkillSpec", pcr->skill_spec);
	fwrite_string(fp, "BonusSkills", pcr->bonus_skills);
	fwrite_rstats(fp, "Stats", pcr->mod_stat);
	fwrite_rstats(fp, "MaxStats", pcr->max_stat);
	fprintf(fp, "Size %s\n", flag_string(size_table, pcr->size));
	fwrite_number(fp, "HPBonus", pcr->hp_bonus);
	fwrite_number(fp, "ManaBonus", pcr->mana_bonus);
	fwrite_number(fp, "PracBonus", pcr->prac_bonus);
	if (pcr->restrict_align)
		fprintf(fp, "RestrictAlign %s~\n",
			flag_string(ralign_names, pcr->restrict_align));
	if (pcr->restrict_ethos)
		fprintf(fp, "RestrictEthos %s~\n",
			flag_string(ethos_table, pcr->restrict_ethos));
	fprintf(fp, "Slang %s\n", flag_string(slang_table, pcr->slang));
	fprintf(fp, "End\n\n");
}

static void *
save_race_cb(void *p, va_list ap)
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
		fprintf(fp, "Act %s~\n", flag_string(act_flags, r->act));
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
		if (r->resists[i])
			fprintf(fp,"Resist %s %d\n", 
				flag_string(dam_classes, i),
				r->resists[i]);
	}

	if (strcmp(r->damtype, "punch"))
		fprintf(fp, "Damtype %s\n", r->damtype);

	aff_fwrite_list("Affc", r->affected, fp);

	fprintf(fp, "End\n\n");

	if (r->race_pcdata)
		save_race_pcdata(r->race_pcdata, fp);
	
	fprintf(fp, "#$\n");
	fclose(fp);

	olc_printf(ch, "    %s (%s)", r->name, filename);
	*pfound = TRUE;
	return NULL;
}

void *
print_race_cb(void *p, va_list ap)
{
	static char buf[256];
        varr *v = va_arg(ap, varr *);
        const char **q = varr_enew(v);
	snprintf(buf, sizeof(buf), "%s%s",
		((race_t *)p)->race_pcdata ? "*" : " ", *(const char **)p );
        *q = str_dup(buf);
        return NULL;
}
