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
 * $Id: olc_damtype.c,v 1.18 2004-06-28 19:21:04 tatyana Exp $
 */

/*
 * Lord of all Noldor
 * A star in the night and a bearer of hope,
 * He rides into his glorious battle alone,
 * Farewell to the valiant warlord.
 *
 * The fate of us all lies deep in the dark,
 * When time stands still at the iron hill.
 */

#include "olc.h"
#include "olc_skill.h"

#define EDIT_DAMT(ch, sk)	(sk = (skill_t *) ch->desc->pEdit)

DECLARE_OLC_FUN(damted_create		);
DECLARE_OLC_FUN(damted_edit		);
DECLARE_OLC_FUN(damted_save		);
DECLARE_OLC_FUN(damted_touch		);
DECLARE_OLC_FUN(damted_show		);
DECLARE_OLC_FUN(damted_list		);

DECLARE_OLC_FUN(damted_name		);

DECLARE_VALIDATE_FUN(validate_damname	);

olc_cmd_t olc_cmds_damt[] =
{
	{ "create",	damted_create,	NULL,	NULL			},
	{ "edit",	damted_edit,	NULL,	NULL			},
	{ "",		damted_save,	NULL,	NULL			},
	{ "touch",	damted_touch,	NULL,	NULL			},
	{ "show",	damted_show,	NULL,	NULL			},
	{ "list",	damted_list,	NULL,	NULL			},

	{ "name",	damted_name,	validate_damname, &strkey_skills },
	{ "noun",	skilled_noun,	NULL,	NULL			},
	{ "noungender",	skilled_noungender, NULL, gender_table		},
	{ "damclass",	skilled_damclass, NULL,	dam_classes		},

	{ "delete_dam",	olced_spell_out, NULL,	NULL			},
	{ "delete_damt",skilled_delete,	NULL,	NULL			},

	{ "commands",	show_commands,	NULL,	NULL			},
	{ "version",	show_version,	NULL,	NULL			},

	{ NULL, NULL, NULL, NULL }
};

OLC_FUN(damted_create)
{
	skill_t *sk;
	char arg[MAX_INPUT_LENGTH];
	const char *dam_name;

	if (PC(ch)->security < SECURITY_DAMT) {
		act_char("DamtEd: Insufficient security for creating damage types.", ch);
		return FALSE;
	}

	if (argument[0] == '\0')
		OLC_ERROR("'OLC CREATE'");

	one_argument(argument, arg + 1, sizeof(arg) - 1);
	if (arg[1] == '+')
		dam_name = arg + 1;
	else {
		arg[0] = '+';
		dam_name = arg;
	}


	if ((sk = c_insert(&skills, dam_name)) == NULL) {
		act_puts("DamtEd: $t: already exists.",
			 ch, dam_name, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	mlstr_init2(&sk->sk_name.ml, argument);
	sk->skill_type	= ST_DAMTYPE;
	OLCED(ch)	= olced_lookup(ED_DAMT);
	ch->desc->pEdit = sk;
	act_char("Damtype created.", ch);
	SET_BIT(changed_flags, CF_DAMT);
	return FALSE;
}

OLC_FUN(damted_edit)
{
	skill_t *sk;

	if (PC(ch)->security < SECURITY_DAMT) {
		act_char("DamtEd: Insufficient security.", ch);
		return FALSE;
	}

	if (argument[0] == '\0')
		OLC_ERROR("'OLC EDIT'");

	if ((sk = skill_search(argument, ST_DAMTYPE)) == NULL) {
		act_puts("DamtEd: $t: No such damtype.",
			 ch, argument, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	ch->desc->pEdit	= sk;
	OLCED(ch)	= olced_lookup(ED_DAMT);
	return FALSE;
}

OLC_FUN(damted_save)
{
	FILE *fp;
	skill_t *sk;

	if (!IS_SET(changed_flags, CF_DAMT)) {
		act_char("Damage types are not changed.", ch);
		return FALSE;
	}
	fp = olc_fopen(ETC_PATH, DAMTYPES_CONF, ch, SECURITY_MATERIAL);
	if (fp == NULL)
		return FALSE;

	C_FOREACH (skill_t *, sk, &skills) {
		if (IS_SET(sk->skill_type, ST_ALL))
			continue;

		fprintf(fp, "#DAMTYPE\n");
		fwrite_string(fp, "Name", gmlstr_mval(&sk->sk_name));
		fprintf(fp, "Type %s\n",
			flag_string(skill_types, sk->skill_type));
		mlstr_fwrite(fp, "NounDamage", &sk->noun_damage.ml);
		mlstr_fwrite(fp, "NounGender", &sk->noun_damage.gender);
		if (sk->dam_class != DAM_NONE) {
			fprintf(fp, "DamClass %s\n",
				flag_string(dam_classes, sk->dam_class));
		}
		fprintf(fp, "End\n\n");
	}

	fprintf(fp, "#$\n");
	fclose(fp);
	olc_printf(ch, "Damage types saved.");
	REMOVE_BIT(changed_flags, CF_DAMT);
	return FALSE;
}

OLC_FUN(damted_touch)
{
	SET_BIT(changed_flags, CF_DAMT);
	return FALSE;
}

OLC_FUN(damted_show)
{
	skill_t *sk;
	BUFFER *buf;

	if (argument[0] == '\0') {
		if (IS_EDIT(ch, ED_DAMT))
			EDIT_DAMT(ch, sk);
		else
			OLC_ERROR("'OLC ASHOW'");
	} else if ((sk = skill_search(argument, ST_DAMTYPE)) == NULL) {
			act_puts("DamtEd: $t: no such damage type.",
				 ch, argument, NULL,
				 TO_CHAR | ACT_NOTRANS, POS_DEAD);
			return FALSE;
	}

	buf = buf_new(0);
	buf_printf(buf, BUF_END,
		   "Name:       [%s]\n", gmlstr_mval(&sk->sk_name));
	mlstr_dump(buf, "NounDamage: ", &sk->noun_damage.ml, DUMP_LEVEL(ch));
	mlstr_dump(buf, "NounGender: ", &sk->noun_damage.gender, DL_NONE);
	buf_printf(buf, BUF_END, "DamClass:   [%s]\n",
		flag_string(dam_classes, sk->dam_class));
	page_to_char(buf_string(buf), ch);
	buf_free(buf);
	return FALSE;
}

OLC_FUN(damted_list)
{
	BUFFER	*buffer;

	buffer = buf_new(0);
	skills_dump(buffer, ST_DAMTYPE);
	page_to_char(buf_string(buffer), ch);
	buf_free(buffer);
	return FALSE;
}

OLC_FUN(damted_name)
{
	if (IS_NULLSTR(argument)) {
		act_puts("Usage: $t <string>",
			 ch, cmd->name, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	return _olced_mlstrkey(ch, "all", argument, cmd);
}

VALIDATE_FUN(validate_damname)
{
	const char *name = (const char *) arg;

	if (name[0] != '+') {
		act_puts("DamtEd: $t: damtype name must start with '+'.",
			 ch, name, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	return TRUE;
}
