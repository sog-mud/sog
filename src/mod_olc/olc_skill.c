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
 * $Id: olc_skill.c,v 1.35 2004-06-28 19:21:04 tatyana Exp $
 */

#include "olc.h"
#include "olc_skill.h"

#define EDIT_SKILL(ch, sk)	(sk = (skill_t *) ch->desc->pEdit)

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
DECLARE_OLC_FUN(skilled_minmana		);
DECLARE_OLC_FUN(skilled_beats		);
DECLARE_OLC_FUN(skilled_msgoff		);
DECLARE_OLC_FUN(skilled_msgobj		);
DECLARE_OLC_FUN(skilled_flags		);
DECLARE_OLC_FUN(skilled_group		);
DECLARE_OLC_FUN(skilled_type		);
DECLARE_OLC_FUN(skilled_event		);
DECLARE_OLC_FUN(skilled_rank		);
DECLARE_OLC_FUN(skilled_effect		);

static DECLARE_VALIDATE_FUN(validate_skill_rank);
static DECLARE_VALIDATE_FUN(validate_skname);

olced_strkey_t strkey_skills = { &skills, NULL, NULL };

olc_cmd_t olc_cmds_skill[] =
{
	{ "create",	skilled_create,	NULL,	NULL			},
	{ "edit",	skilled_edit,	NULL,	NULL			},
	{ "",		skilled_save,	NULL,	NULL			},
	{ "touch",	skilled_touch,	NULL,	NULL			},
	{ "show",	skilled_show,	NULL,	NULL			},
	{ "list",	skilled_list,	NULL,	NULL			},

	{ "name",	olced_mlstrkey,	validate_skname, &strkey_skills	},
	{ "gender",	skilled_gender,	NULL,	gender_table		},
	{ "funname",	skilled_funname, validate_funname, NULL		},
	{ "target",	skilled_target, NULL,	skill_targets		},
	{ "minpos",	skilled_minpos, NULL,	position_table		},
	{ "minmana",	skilled_minmana, NULL,	NULL			},
	{ "beats",	skilled_beats,	NULL,	NULL			},
	{ "noun",	skilled_noun,	NULL,	NULL			},
	{ "noungender",	skilled_noungender, NULL, gender_table		},
	{ "msgoff",	skilled_msgoff,	NULL,	NULL			},
	{ "msgobjoff",	skilled_msgobj,	NULL,	NULL			},
	{ "flags",	skilled_flags,	NULL,	skill_flags		},
	{ "group",	skilled_group,	NULL,	skill_groups		},
	{ "type",	skilled_type,	NULL,	skill_types		},
	{ "event",	skilled_event, validate_funname, event_classes	},
	{ "rank",	skilled_rank, validate_skill_rank, NULL		},
	{ "damclass",	skilled_damclass, NULL,	dam_classes		},
	{ "effect",	skilled_effect, NULL,	&effects		},

	{ "delete_skil", olced_spell_out, NULL,	NULL			},
	{ "delete_skill", skilled_delete, NULL,	NULL			},


	{ "commands",	show_commands,	NULL,	NULL			},
	{ "version",	show_version,	NULL,	NULL			},

	{ NULL, NULL, NULL, NULL }
};

OLC_FUN(skilled_create)
{
	skill_t *sk;

	if (PC(ch)->security < SECURITY_SKILL) {
		act_char("SkillEd: Insufficient security for creating skills.", ch);
		return FALSE;
	}

	if (IS_NULLSTR(argument))
		OLC_ERROR("'OLC CREATE'");

	if ((sk = c_insert(&skills, argument)) == NULL) {
		act_puts("SkillEd: $t: already exists.",
			 ch, argument, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	mlstr_init2(&sk->sk_name.ml, argument);
	OLCED(ch)	= olced_lookup(ED_SKILL);
	ch->desc->pEdit = sk;
	act_char("Skill created.", ch);
	SET_BIT(changed_flags, CF_SKILL);
	return FALSE;
}

OLC_FUN(skilled_edit)
{
	skill_t *sk;

	if (PC(ch)->security < SECURITY_SKILL) {
		act_char("SkillEd: Insufficient security.", ch);
		return FALSE;
	}

	if (IS_NULLSTR(argument))
		OLC_ERROR("'OLC EDIT'");

	if ((sk = skill_search(argument, ST_ALL)) == NULL) {
		act_puts("SkillEd: $t: No such skill.",
			 ch, argument, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	ch->desc->pEdit	= sk;
	OLCED(ch)	= olced_lookup(ED_SKILL);
	return FALSE;
}

OLC_FUN(skilled_save)
{
	FILE *fp;
	skill_t *sk;

	if (!IS_SET(changed_flags, CF_SKILL)) {
		act_char("Skills are not changed.", ch);
		return FALSE;
	}
	fp = olc_fopen(ETC_PATH, SKILLS_CONF, ch, SECURITY_SKILL);
	if (fp == NULL)
		return FALSE;

	C_FOREACH (skill_t *, sk, &skills) {
		evf_t *ev;

		if (!IS_SET(sk->skill_type, ST_ALL))
			continue;

		fprintf(fp, "#SKILL\n");
		mlstr_fwrite(fp, "Name", &sk->sk_name.ml);
		mlstr_fwrite(fp, "Gender", &sk->sk_name.gender);
		fprintf(fp, "Type %s\n",
			flag_string(skill_types, sk->skill_type));
		fprintf(fp, "Group %s\n", flag_string(skill_groups, sk->group));
		fprintf(fp, "MinPos %s\n",
			flag_string(position_table, sk->min_pos));
		fprintf(fp, "Target %s\n",
			flag_string(skill_targets, sk->target));
		if (sk->beats)
			fprintf(fp, "Beats %d\n", sk->beats);
		if (sk->rank)
			fprintf(fp, "Rank %d\n", sk->rank);
		if (sk->skill_flags) {
			fprintf(fp, "Flags %s~\n",
				flag_string(skill_flags, sk->skill_flags));
		}
		if (sk->min_mana)
			fprintf(fp, "MinMana %d\n", sk->min_mana);
		mlstr_fwrite(fp, "NounDamage", &sk->noun_damage.ml);
		mlstr_fwrite(fp, "NounGender", &sk->noun_damage.gender);
		if (sk->dam_class != DAM_NONE) {
			fprintf(fp, "DamClass %s\n",
				flag_string(dam_classes, sk->dam_class));
		}
		if (!IS_NULLSTR(sk->effect))
			fwrite_string(fp, "Effect", sk->effect);
		if (!IS_NULLSTR(sk->fun_name))
			fprintf(fp, "SpellFun %s\n", sk->fun_name);
		mlstr_fwrite(fp, "WearOff", &sk->msg_off);
		mlstr_fwrite(fp, "ObjWearOff", &sk->msg_obj);
		C_FOREACH (evf_t *, ev, &sk->events) {
			fprintf(fp, "Event %s %s\n",
				flag_string(event_classes, ev->event),
				ev->fun_name);
		}
		fprintf(fp, "End\n\n");
	}

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
	evf_t *ev;
	BUFFER *buf;

	if (IS_NULLSTR(argument)) {
		if (IS_EDIT(ch, ED_SKILL))
			EDIT_SKILL(ch, sk);
		else
			OLC_ERROR("'OLC ASHOW'");
	} else {
		if ((sk = skill_search(argument, ST_ALL)) == NULL) {
			act_puts("SkillEd: $t: no such skill.",
				 ch, argument, NULL,
				 TO_CHAR | ACT_NOTRANS, POS_DEAD);
			return FALSE;
		}
	}

	buf = buf_new(0);
	mlstr_dump(buf, "Name:      ", &sk->sk_name.ml, DL_NONE);
	mlstr_dump(buf, "Gender:    ", &sk->sk_name.gender, DL_NONE);
	buf_printf(buf, BUF_END, "Type       [%s]     Group       [%s]\n",
			flag_string(skill_types, sk->skill_type),
			flag_string(skill_groups, sk->group));
	buf_printf(buf, BUF_END, "MinPos     [%s]     Target      [%s]\n",
			flag_string(position_table, sk->min_pos),
			flag_string(skill_targets, sk->target));
	buf_printf(buf, BUF_END, "Beats      [%d]\n", sk->beats);
	if (sk->skill_flags)
		buf_printf(buf, BUF_END, "Flags      [%s]\n",
				flag_string(skill_flags, sk->skill_flags));
	buf_printf(buf, BUF_END, "MinMana    [%d]\n", sk->min_mana);
	mlstr_dump(buf, "NounDamage ", &sk->noun_damage.ml, DUMP_LEVEL(ch));
	mlstr_dump(buf, "NounGender ", &sk->noun_damage.gender, DL_NONE);
	buf_printf(buf, BUF_END, "DamClass:  [%s]\n",
		flag_string(dam_classes, sk->dam_class));
	if (!IS_NULLSTR(sk->effect))
		buf_printf(buf, BUF_END, "Effect:    [%s]\n", sk->effect);
	buf_printf(buf, BUF_END, "Rank       [%d]\n", sk->rank);

	buf_printf(buf, BUF_END, "SpellFun   [%s]\n", sk->fun_name);

	mlstr_dump(buf, "WearOff    ", &sk->msg_off, DUMP_LEVEL(ch));
	mlstr_dump(buf, "ObjWearOff ", &sk->msg_obj, DUMP_LEVEL(ch));
	C_FOREACH (evf_t *, ev, &sk->events) {
		buf_printf(buf, BUF_END, "Event: [%s] %s\n",
		    flag_string(event_classes, ev->event),
		    ev->fun_name);
	}

	page_to_char(buf_string(buf), ch);
	buf_free(buf);
	return FALSE;
}

OLC_FUN(skilled_list)
{
	BUFFER	*buffer;

	buffer = buf_new(0);
	skills_dump(buffer, ST_ALL);
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

OLC_FUN(skilled_rank)
{
	skill_t *sk;
	EDIT_SKILL(ch, sk);

	if (!olced_number(ch, argument, cmd, &sk->rank))
		return FALSE;

	return TRUE;
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
	evf_t *ev;
	flag_t event;
	char arg[MAX_INPUT_LENGTH];

	EDIT_SKILL(ch, sk);
	argument = one_argument(argument, arg, sizeof(arg));

	if (IS_NULLSTR(arg))
		OLC_ERROR("'OLC SKILL EVENT'");

	if (!str_cmp(arg, "?")) {
		act_char("Valid event classes are:", ch);
		show_flags(ch, event_classes);
		return FALSE;
	}

	if ((event = flag_value(event_classes, arg)) < 0) {
		act_puts("SkillEd: $t: unknown event.",
			 ch, arg, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	ev = varr_bsearch(&sk->events, &event, cmpint);
	one_argument(argument, arg, sizeof(arg));
	if (IS_NULLSTR(arg))
		OLC_ERROR("'OLC SKILL EVENT'");

	if (!str_cmp(arg, "none")) {
		/*
		 * delete event
		 */

		if (ev == NULL) {
			act_puts("SkillEd: $t: event not found.",
				 ch, flag_string(event_classes, event), NULL,
				 TO_CHAR | ACT_NOTRANS, POS_DEAD);
			return FALSE;
		}

		varr_edelete(&sk->events, ev);
		act_puts("SkillEd: $t: event deleted.",
			 ch, flag_string(event_classes, event), NULL,
			 TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return TRUE;
	}

	if (!cmd->validator(ch, arg))
		return FALSE;

	/*
	 * add event
	 */
	if (ev == NULL) {
		ev = varr_enew(&sk->events);
		ev->event = event;
	} else {
		act_puts("SkillEd: $t: changing existing event, events module should be reloaded.",
			 ch, flag_string(event_classes, event), NULL,
			 TO_CHAR | ACT_NOTRANS, POS_DEAD);
	}

	free_string(ev->fun_name);
	ev->fun_name = str_dup(arg);
	varr_qsort(&sk->events, cmpint);
	act_char("SkillEd: Ok.", ch);
	return TRUE;
}

OLC_FUN(skilled_effect)
{
	skill_t *sk;
	EDIT_SKILL(ch, sk);
	return olced_foreign_strkey(ch, argument, cmd, &sk->effect);
}

OLC_FUN(skilled_delete)
{
	skill_t *sk;

	EDIT_SKILL(ch, sk);

	if (olced_busy(ch, OLCED(ch)->name, sk, NULL))
		return FALSE;

	c_delete(&skills, gmlstr_mval(&sk->sk_name));
	edit_done(ch->desc);
	return TRUE;
}

OLC_FUN(skilled_damclass)
{
	skill_t *sk;

	EDIT_SKILL(ch, sk);
	return olced_flag(ch, argument, cmd, &sk->dam_class);
}

static VALIDATE_FUN(validate_skill_rank)
{
	int val = *(const int *) arg;

	if (val < 0 || val > 7) {
		act_char("SkillEd: skill rank should be in [0..7].", ch);
		return FALSE;
	}
	return TRUE;
}

static VALIDATE_FUN(validate_skname)
{
	const char *name = (const char *) arg;

	if (name[0] == '+') {
		act_puts("SkillEd: $t: skill name can not start with '+'.",
			 ch, name, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	return TRUE;
}
