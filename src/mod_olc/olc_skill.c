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
 * $Id: olc_skill.c,v 1.27 2001-09-13 12:03:02 fjoe Exp $
 */

#include "olc.h"

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
DECLARE_OLC_FUN(skilled_noun		);
DECLARE_OLC_FUN(skilled_noungender	);
DECLARE_OLC_FUN(skilled_msgoff		);
DECLARE_OLC_FUN(skilled_msgobj		);
DECLARE_OLC_FUN(skilled_flags		);
DECLARE_OLC_FUN(skilled_group		);
DECLARE_OLC_FUN(skilled_type		);
DECLARE_OLC_FUN(skilled_event		);
DECLARE_OLC_FUN(skilled_delete		);
DECLARE_OLC_FUN(skilled_rank		);

static DECLARE_VALIDATE_FUN(validate_skill_rank);

olced_strkey_t strkey_skills = { &skills, NULL, NULL };

olc_cmd_t olc_cmds_skill[] =
{
	{ "create",	skilled_create,	NULL,	NULL			},
	{ "edit",	skilled_edit,	NULL,	NULL			},
	{ "",		skilled_save,	NULL,	NULL			},
	{ "touch",	skilled_touch,	NULL,	NULL			},
	{ "show",	skilled_show,	NULL,	NULL			},
	{ "list",	skilled_list,	NULL,	NULL			},

	{ "name",	olced_mlstrkey,	NULL,	&strkey_skills		},
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
	{ "event",	skilled_event, validate_funname, events_classes	},
	{ "rank",	skilled_rank, validate_skill_rank, NULL		},
	{ "delete_skil", olced_spell_out, NULL,	NULL			},
	{ "delete_skill", skilled_delete, NULL,	NULL			},


	{ "commands",	show_commands,	NULL,	NULL			},
	{ "version",	show_version,	NULL,	NULL			},

	{ NULL, NULL, NULL, NULL }
};

OLC_FUN(skilled_create)
{
	skill_t *sk;
	char arg[MAX_INPUT_LENGTH];

	if (PC(ch)->security < SECURITY_SKILL) {
		act_char("SkillEd: Insufficient security for creating skills.", ch);
		return FALSE;
	}

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0')
		OLC_ERROR("'OLC CREATE'");

	/*
	 * olced_busy check is not needed since c_insert
	 * adds new elements to the end of varr
	 */

	if ((sk = c_insert(&skills, arg)) == NULL) {
		act_puts("SkillEd: $t: already exists.",
			 ch, arg, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	mlstr_init2(&sk->sk_name.ml, arg);
	OLCED(ch)	= olced_lookup(ED_SKILL);
	ch->desc->pEdit = sk;
	act_char("Skill created.", ch);
	SET_BIT(changed_flags, CF_SKILL);
	return FALSE;
}

OLC_FUN(skilled_edit)
{
	skill_t *sk;
	char arg[MAX_INPUT_LENGTH];

	if (PC(ch)->security < SECURITY_SKILL) {
		act_char("SkillEd: Insufficient security.", ch);
		return FALSE;
	}

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0')
		OLC_ERROR("'OLC EDIT'");

	if (!(sk = skill_search(arg))) {
		act_puts("SkillEd: $t: No such skill.",
			 ch, arg, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	ch->desc->pEdit	= sk;
	OLCED(ch)	= olced_lookup(ED_SKILL);
	return FALSE;
}

static void *
event_save_cb(void *p, va_list ap)
{
	evf_t *ev = (evf_t *) p;

	FILE *fp = va_arg(ap, FILE *);

	fprintf(fp, "Event %s %s\n",
		flag_string(events_classes, ev->event),
		ev->fun_name);
	return NULL;
}

static void *skill_save_cb(void *p, va_list ap)
{
	skill_t *sk = (skill_t *) p;

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
	if (sk->rank)
		fprintf(fp, "Rank %d\n", sk->rank);
	if (sk->skill_flags)
		fprintf(fp, "Flags %s~\n", flag_string(skill_flags, sk->skill_flags));
	if (sk->min_mana)
		fprintf(fp, "MinMana %d\n", sk->min_mana);
	mlstr_fwrite(fp, "NounDamage", &sk->noun_damage.ml);
	mlstr_fwrite(fp, "NounGender", &sk->noun_damage.gender);
	if (!IS_NULLSTR(sk->fun_name))
		fprintf(fp, "SpellFun %s\n", sk->fun_name);
	mlstr_fwrite(fp, "WearOff", &sk->msg_off);
	mlstr_fwrite(fp, "ObjWearOff", &sk->msg_obj);
	c_foreach(&sk->events, event_save_cb, fp);
	fprintf(fp, "End\n\n");
	return NULL;
}

OLC_FUN(skilled_save)
{
	FILE *fp;

	if (!IS_SET(changed_flags, CF_SKILL)) {
		act_char("Skills are not changed.", ch);
		return FALSE;
	}
	fp = olc_fopen(ETC_PATH, SKILLS_CONF, ch, SECURITY_SKILL);
	if (fp == NULL)
		return FALSE;

	c_foreach(&skills, skill_save_cb, fp);

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

static void *
event_show_cb(void *p, va_list ap)
{
	evf_t *ev = (evf_t *) p;

	BUFFER *buf = va_arg(ap, BUFFER *);

	buf_printf(buf, BUF_END, "Event: [%s] %s\n",
		   flag_string(events_classes, ev->event),
		   ev->fun_name);
	return NULL;
}

OLC_FUN(skilled_show)
{
	skill_t *sk;
	BUFFER *buf;
	char arg[MAX_INPUT_LENGTH];

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		if (IS_EDIT(ch, ED_SKILL))
			EDIT_SKILL(ch, sk);
		else
			OLC_ERROR("'OLC ASHOW'");
	} else {
		if (!(sk = skill_search(arg))) {
			act_puts("SkillEd: $t: no such skill.",
				 ch, arg, NULL,
				 TO_CHAR | ACT_NOTRANS, POS_DEAD);
			return FALSE;
		}
	}

	buf = buf_new(0);
	mlstr_dump(buf, "Name       ", &sk->sk_name.ml, DL_NONE);
	mlstr_dump(buf, "Gender:    ", &sk->sk_name.gender, DL_NONE);
	buf_printf(buf, BUF_END, "Type       [%s]     Group       [%s]\n",
			flag_string(skill_types, sk->skill_type),
			flag_string(skill_groups, sk->group));
	buf_printf(buf, BUF_END, "MinPos     [%s]     Target      [%s]\n",
			flag_string(position_table, sk->min_pos),
			flag_string(skill_targets, sk->target));
	if (sk->beats)
		buf_printf(buf, BUF_END, "Beats      [%d]\n", sk->beats);
	if (sk->skill_flags)
		buf_printf(buf, BUF_END, "Flags      [%s]\n",
				flag_string(skill_flags, sk->skill_flags));
	if (sk->min_mana)
		buf_printf(buf, BUF_END, "MinMana    [%d]\n", sk->min_mana);
	mlstr_dump(buf, "NounDamage ", &sk->noun_damage.ml, DUMP_LEVEL(ch));
	mlstr_dump(buf, "NounGender ", &sk->noun_damage.gender, DL_NONE);
	buf_printf(buf, BUF_END, "Rank       [%d]\n", sk->rank);

	if (!IS_NULLSTR(sk->fun_name))
		buf_printf(buf, BUF_END, "SpellFun   [%s]\n", sk->fun_name);

	mlstr_dump(buf, "WearOff    ", &sk->msg_off, DUMP_LEVEL(ch));
	mlstr_dump(buf, "ObjWearOff ", &sk->msg_obj, DUMP_LEVEL(ch));
	c_foreach(&sk->events, event_show_cb, buf);

	page_to_char(buf_string(buf), ch);
	buf_free(buf);
	return FALSE;
}

OLC_FUN(skilled_list)
{
	BUFFER	*buffer;

	buffer = buf_new(0);
	c_mlstrkey_dump(&skills, buffer);
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

static VALIDATE_FUN(validate_skill_rank)
{
	int val = *(const int *) arg;

	if (val < 0 || val > 7) {
		act_char("SkillEd: skill rank should be in [0..7].", ch);
		return FALSE;
	}
	return TRUE;
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
		show_flags(ch, events_classes);
		return FALSE;
	}

	if ((event = flag_value(events_classes, arg)) < 0) {
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
				 ch, flag_string(events_classes, event), NULL,
				 TO_CHAR | ACT_NOTRANS, POS_DEAD);
			return FALSE;
		}

		varr_edelete(&sk->events, ev);
		act_puts("SkillEd: $t: event deleted.",
			 ch, flag_string(events_classes, event), NULL,
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
			 ch, flag_string(events_classes, event), NULL,
			 TO_CHAR | ACT_NOTRANS, POS_DEAD);
	}

	free_string(ev->fun_name);
	ev->fun_name = str_dup(arg);
	varr_qsort(&sk->events, cmpint);
	act_char("SkillEd: Ok.", ch);
	return TRUE;
}

OLC_FUN(skilled_delete)
{
	skill_t *sk;

	if (olced_busy(ch, ED_SKILL, NULL, NULL))
		return FALSE;

	EDIT_SKILL(ch, sk);
	c_delete(&skills, gmlstr_mval(&sk->sk_name));
	edit_done(ch->desc);
	return TRUE;
}

