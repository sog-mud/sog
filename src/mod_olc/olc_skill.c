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
 * $Id: olc_skill.c,v 1.11 1999-12-16 11:40:52 fjoe Exp $
 */

#include "olc.h"

#define EDIT_SKILL(ch, sk)	(sk = (skill_t*) ch->desc->pEdit)

DECLARE_OLC_FUN(skilled_create		);
DECLARE_OLC_FUN(skilled_edit		);
DECLARE_OLC_FUN(skilled_save		);
DECLARE_OLC_FUN(skilled_touch		);
DECLARE_OLC_FUN(skilled_show		);
DECLARE_OLC_FUN(skilled_list		);

DECLARE_OLC_FUN(skilled_gender		);
DECLARE_OLC_FUN(skilled_funname		);
DECLARE_OLC_FUN(skilled_target		);
DECLARE_OLC_FUN(skilled_minpos		);
DECLARE_OLC_FUN(skilled_slot		);
DECLARE_OLC_FUN(skilled_minmana		);
DECLARE_OLC_FUN(skilled_beats		);
DECLARE_OLC_FUN(skilled_noun		);
DECLARE_OLC_FUN(skilled_noungender	);
DECLARE_OLC_FUN(skilled_msgoff		);
DECLARE_OLC_FUN(skilled_msgobj		);
DECLARE_OLC_FUN(skilled_flags		);
DECLARE_OLC_FUN(skilled_group		);
DECLARE_OLC_FUN(skilled_type		);
DECLARE_OLC_FUN(skilled_event		);

olced_strkey_t strkey_skills = { &skills, NULL };

olc_cmd_t olc_cmds_skill[] =
{
	{ "create",	skilled_create					},
	{ "edit",	skilled_edit					},
	{ "",		skilled_save					},
	{ "touch",	skilled_touch					},
	{ "show",	skilled_show					},
	{ "list",	skilled_list					},

	{ "name",	olced_mlstrkey,	NULL,	&strkey_skills		},
	{ "gender",	skilled_gender,	NULL,	gender_table		},
	{ "funname",	skilled_funname, validate_funname		},
	{ "target",	skilled_target, NULL,	skill_targets		},
	{ "minpos",	skilled_minpos, NULL,	position_table		},
	{ "slot",	skilled_slot					},
	{ "minmana",	skilled_minmana					},
	{ "beats",	skilled_beats					},
	{ "noun",	skilled_noun					},
	{ "noungender",	skilled_noungender, NULL, gender_table		},
	{ "msgoff",	skilled_msgoff					},
	{ "msgobjoff",	skilled_msgobj					},
	{ "flags",	skilled_flags,	NULL,	skill_flags		},
	{ "group",	skilled_group,	NULL,	skill_groups		},
	{ "type",	skilled_type,	NULL,	skill_types		},
	{ "event",	skilled_event, validate_funname, events_table	},

	{ "commands",	show_commands					},
	{ NULL }
};

OLC_FUN(skilled_create)
{
	skill_t sk;
	skill_t *s;

	if (PC(ch)->security < SECURITY_SKILL) {
		char_puts("SkillEd: Insufficient security for creating skills.\n", ch);
		return FALSE;
	}

	if (argument[0] == '\0')
		OLC_ERROR("'OLC CREATE'");

	/*
	 * olced_busy check is not needed since hash_insert
	 * adds new elements to the end of varr
	 */

	skill_init(&sk);
	mlstr_init(&sk.sk_name.ml, argument);
	s = hash_insert(&skills, argument, &sk);
	skill_destroy(&sk);

	if (s == NULL) {
		char_printf(ch, "SkillEd: %s: already exists.\n", argument);
		return FALSE;
	}

	OLCED(ch)	= olced_lookup(ED_SKILL);
	ch->desc->pEdit = s;
	char_puts("Skill created.\n",ch);
	SET_BIT(changed_flags, CF_SKILL);
	return FALSE;
}

OLC_FUN(skilled_edit)
{
	skill_t *sk;

	if (PC(ch)->security < SECURITY_SKILL) {
		char_puts("SkillEd: Insufficient security.\n", ch);
		return FALSE;
	}

	if (argument[0] == '\0')
		OLC_ERROR("'OLC EDIT'");

	if (!(sk = skill_search(argument))) {
		char_printf(ch, "SkillEd: %s: No such skill.\n", argument);
		return FALSE;
	}

	ch->desc->pEdit	= sk;
	OLCED(ch)	= olced_lookup(ED_SKILL);
	return FALSE;
}

static void *skill_save_cb(void *p, va_list ap)
{
	skill_t *sk = (skill_t *)p;
	event_fun_t *ev;

	FILE *fp = va_arg(ap, FILE *);

	fprintf(fp, "#SKILL\n");
	mlstr_fwrite(fp, "Name", &sk->sk_name.ml);
	mlstr_fwrite(fp, "Gender", &sk->sk_name.gender);
	fprintf(fp, "Type %s\n", flag_string(skill_types, sk->skill_type));
	fprintf(fp, "Group %s\n", flag_string(skill_groups, sk->group));
	fprintf(fp, "MinPos %s\n", flag_string(position_table, sk->min_pos));
	fprintf(fp, "Target %s\n", flag_string(skill_targets, sk->target));
	if (sk->beats)
		fprintf(fp, "Beats %d\n", sk->beats);
	if (sk->skill_flags)
		fprintf(fp, "Flags %s~\n", flag_string(skill_flags, sk->skill_flags));
	if (sk->min_mana)
		fprintf(fp, "MinMana %d\n", sk->min_mana);
	mlstr_fwrite(fp, "NounDamage", &sk->noun_damage.ml);
	mlstr_fwrite(fp, "NounGender", &sk->noun_damage.gender);
	if (sk->slot)
		fprintf(fp, "Slot %d\n", sk->slot);
	if (!IS_NULLSTR(sk->fun_name))
		fprintf(fp, "SpellFun %s\n", sk->fun_name);
	mlstr_fwrite(fp, "WearOff", &sk->msg_off);
	mlstr_fwrite(fp, "ObjWearOff", &sk->msg_obj);
	for (ev = sk->eventlist; ev; ev = ev->next)
		fprintf(fp, "Event %s %s\n",
			flag_string(events_table, ev->event),
			ev->fun_name);
	fprintf(fp, "End\n\n");
	return NULL;
}

OLC_FUN(skilled_save)
{
	FILE *fp;

	if (!IS_SET(changed_flags, CF_SKILL)) {
		char_puts("Skills are not changed.\n", ch);
		return FALSE;
	}
	fp = olc_fopen(ETC_PATH, SKILLS_CONF, ch, SECURITY_SKILL);
	if (fp == NULL)
		return FALSE;

	hash_foreach(&skills, skill_save_cb, fp);

	fprintf(fp, "#$\n");
	fclose(fp);
	olc_printf(ch, "Skills saved.");
	REMOVE_BIT(changed_flags, CF_SKILL);
	return FALSE;
}

OLC_FUN(skilled_touch)
{
	SET_BIT(changed_flags, CF_SKILL);
	return FALSE;
}

OLC_FUN(skilled_show)
{
	skill_t *sk;
	event_fun_t *ev;
	int i = 1;
	BUFFER *buf;

	if (argument[0] == '\0') {
		if (IS_EDIT(ch, ED_SKILL))
			EDIT_SKILL(ch, sk);
		else
			OLC_ERROR("'OLC ASHOW'");
	} else {
		if (!(sk = skill_search(argument))) {
			char_printf(ch, "SkillEd: %s: no such skill.\n", argument);
			return FALSE;
		}
	}
	
	buf = buf_new(-1);
	mlstr_dump(buf, "Name       ", &sk->sk_name.ml);
	mlstr_dump(buf, "Gender:    ", &sk->sk_name.gender);
	buf_printf(buf, "Type       [%s]     Group       [%s]\n",
			flag_string(skill_types, sk->skill_type),
			flag_string(skill_groups, sk->group));
	buf_printf(buf, "MinPos     [%s]     Target      [%s]\n",
			flag_string(position_table, sk->min_pos),
			flag_string(skill_targets, sk->target));
	if (sk->beats)
		buf_printf(buf, "Beats      [%d]\n", sk->beats);
	if (sk->skill_flags)
		buf_printf(buf, "Flags      [%s]\n",
				flag_string(skill_flags, sk->skill_flags));
	if (sk->min_mana)
		buf_printf(buf, "MinMana    [%d]\n", sk->min_mana);
	mlstr_dump(buf, "NounDamage ", &sk->noun_damage.ml);
	mlstr_dump(buf, "NounGender ", &sk->noun_damage.gender);
	if (sk->slot)
		buf_printf(buf, "Slot       [%d]\n", sk->slot);

	if (!IS_NULLSTR(sk->fun_name))
		buf_printf(buf, "SpellFun   [%s]\n", sk->fun_name);

	mlstr_dump(buf, "WearOff     ", &sk->msg_off);
	mlstr_dump(buf, "ObjWearOff  ", &sk->msg_obj);
	for (ev = sk->eventlist; ev; ev = ev->next, i++) {
		if (i == 1)
			buf_add(buf, "Events:\n");
		buf_printf(buf, "%d) in event   [%s] call fun [%s]\n", i,
			flag_string(events_table, ev->event),
			ev->fun_name);
	}

	page_to_char(buf_string(buf), ch);
	buf_free(buf);
	return FALSE;
}

OLC_FUN(skilled_list)
{
	BUFFER	*buffer;

	buffer = buf_new(-1);
	mlstrkey_printall(&skills, buffer);
	page_to_char(buf_string(buffer), ch);
	buf_free(buffer);
	return FALSE;
}

OLC_FUN(skilled_gender)
{
	skill_t *sk;

	EDIT_SKILL(ch, sk);
	return olced_gender(ch, argument, cmd, &sk->sk_name.gender);
}

OLC_FUN(skilled_funname)
{
	skill_t *sk;

	EDIT_SKILL(ch, sk);
	return olced_str(ch, argument, cmd, &sk->fun_name);
}

OLC_FUN(skilled_target)
{
	skill_t *sk;

	EDIT_SKILL(ch, sk);
	return olced_flag(ch, argument, cmd, &sk->target);
}

OLC_FUN(skilled_minpos)
{
	skill_t *sk;

	EDIT_SKILL(ch, sk);
	return olced_flag(ch, argument, cmd, &sk->min_pos);
}

OLC_FUN(skilled_slot)
{
	skill_t *sk;

	EDIT_SKILL(ch, sk);
	return olced_number(ch, argument, cmd, &sk->slot);
}

OLC_FUN(skilled_minmana)
{
	skill_t *sk;

	EDIT_SKILL(ch, sk);
	return olced_number(ch, argument, cmd, &sk->min_mana);
}

OLC_FUN(skilled_beats)
{
	skill_t *sk;

	EDIT_SKILL(ch, sk);
	return olced_number(ch, argument, cmd, &sk->beats);
}

OLC_FUN(skilled_noun)
{
	skill_t *sk;

	EDIT_SKILL(ch, sk);
	return olced_mlstr(ch, argument, cmd, &sk->noun_damage.ml);
}

OLC_FUN(skilled_noungender)
{
	skill_t *sk;

	EDIT_SKILL(ch, sk);
	return olced_mlstr(ch, argument, cmd, &sk->noun_damage.gender);
}

OLC_FUN(skilled_msgoff)
{
	skill_t *sk;

	EDIT_SKILL(ch, sk);
	return olced_mlstr(ch, argument, cmd, &sk->msg_off);
}

OLC_FUN(skilled_msgobj)
{
	skill_t *sk;

	EDIT_SKILL(ch, sk);
	return olced_mlstr(ch, argument, cmd, &sk->msg_obj);
}

OLC_FUN(skilled_flags)
{
	skill_t *sk;

	EDIT_SKILL(ch, sk);
	return olced_flag(ch, argument, cmd, &sk->skill_flags);
}

OLC_FUN(skilled_group)
{
	skill_t *sk;

	EDIT_SKILL(ch, sk);
	return olced_flag(ch, argument, cmd, &sk->group);
}

OLC_FUN(skilled_type)
{
	skill_t *sk;

	EDIT_SKILL(ch, sk);
	return olced_flag(ch, argument, cmd, &sk->skill_type);
}

OLC_FUN(skilled_event)
{
	skill_t *sk;
	event_fun_t *ev;
	char arg[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];

	EDIT_SKILL(ch, sk);
	argument = one_argument(argument, arg, sizeof(arg));

	if (IS_NULLSTR(argument))
		OLC_ERROR("'OLC SKILL EVENT'");

	if (!str_prefix(arg, "add")) {
		argument = one_argument(argument, arg, sizeof(arg));
			   one_argument(argument, arg2, sizeof(arg2));

		if (IS_NULLSTR(arg))
			OLC_ERROR("'OLC SKILL EVENT'");

		ev = evf_new();
		if (!olced_flag(ch, arg, cmd, &ev->event)) {
			evf_free(ev);
			return FALSE;
		}

		if (IS_NULLSTR(arg2))
			OLC_ERROR("'OLC SKILL EVENT'");

		if (!cmd->validator(ch, arg2))
			return FALSE;

		ev->fun_name = str_dup(arg2);
		ev->next = sk->eventlist;
		sk->eventlist = ev;
		return TRUE;
	}

	if (!str_prefix(arg, "delete")) {
		int i;
		event_fun_t *evp = NULL;

		one_argument(argument, arg, sizeof(arg));
		if (!is_number(arg))
			return skilled_event(ch, str_empty, cmd);

		if ((i = atoi(arg)) < 1) {
			char_puts("SkillEd: event number must be > 0\n", ch);
			return FALSE;
		}

		for (ev = sk->eventlist, i--; i && ev; evp = ev, ev = ev->next, i--);

		if (!ev) {
			char_puts("SkillEd: no such event defined.\n", ch);
			return FALSE;
		}

		if (evp) {
			evp->next = ev->next;
		} else {
			sk->eventlist = ev->next;
		}

		evf_free(ev);
		return TRUE;
	}

	return skilled_event(ch, str_empty, cmd);
}
