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
 * $Id: olc_damtype.c,v 1.12 2001-09-14 10:01:08 fjoe Exp $
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

#define EDIT_DAMT(ch, d)	(d = (damtype_t *) ch->desc->pEdit)

DECLARE_OLC_FUN(damted_create		);
DECLARE_OLC_FUN(damted_edit		);
DECLARE_OLC_FUN(damted_save		);
DECLARE_OLC_FUN(damted_touch		);
DECLARE_OLC_FUN(damted_show		);
DECLARE_OLC_FUN(damted_list		);

DECLARE_OLC_FUN(damted_noun		);
DECLARE_OLC_FUN(damted_gender		);
DECLARE_OLC_FUN(damted_class		);
DECLARE_OLC_FUN(damted_slot		);

DECLARE_OLC_FUN(damted_delete		);

olced_strkey_t strkey_damtypes = { &damtypes, NULL, NULL };

olc_cmd_t olc_cmds_damt[] =
{
	{ "create",	damted_create,	NULL,	NULL			},
	{ "edit",	damted_edit,	NULL,	NULL			},
	{ "",		damted_save,	NULL,	NULL			},
	{ "touch",	damted_touch,	NULL,	NULL			},
	{ "show",	damted_show,	NULL,	NULL			},
	{ "list",	damted_list,	NULL,	NULL			},

	{ "name",	olced_strkey,	NULL,	&strkey_damtypes	},
	{ "gender",	damted_gender,	NULL,	gender_table		},
	{ "noundamage",	damted_noun,	NULL,	NULL			},
	{ "class",	damted_class,	NULL,	dam_classes		},
	{ "slot",	damted_slot,	NULL,	NULL			},

	{ "delete_dam",	olced_spell_out, NULL,	NULL			},
	{ "delete_damt",damted_delete,	NULL,	NULL			},

	{ "commands",	show_commands,	NULL,	NULL			},
	{ "version",	show_version,	NULL,	NULL			},

	{ NULL, NULL, NULL, NULL }
};

static void *damt_save_cb(void *p, va_list ap);

OLC_FUN(damted_create)
{
	damtype_t *dt;
	char arg[MAX_INPUT_LENGTH];

	if (PC(ch)->security < SECURITY_DAMT) {
		act_char("DamtEd: Insufficient security for creating damage types.", ch);
		return FALSE;
	}

	if (argument[0] == '\0')
		OLC_ERROR("'OLC CREATE'");

	one_argument(argument, arg, sizeof(arg));

	if ((dt = c_insert(&damtypes, arg)) == NULL) {
		act_puts("DamtEd: $t: already exists.",
			 ch, arg, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	dt->dam_name	= str_dup(arg);
	OLCED(ch)	= olced_lookup(ED_DAMT);
	ch->desc->pEdit = dt;
	act_char("Damtype created.", ch);
	SET_BIT(changed_flags, CF_DAMT);
	return FALSE;
}

OLC_FUN(damted_edit)
{
	damtype_t *dt;

	if (PC(ch)->security < SECURITY_DAMT) {
		act_char("DamtEd: Insufficient security.", ch);
		return FALSE;
	}

	if (argument[0] == '\0')
		OLC_ERROR("'OLC EDIT'");

	if ((dt = damtype_search(argument)) == NULL) {
		act_puts("DamtEd: $t: No such damtype.",
			 ch, argument, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	ch->desc->pEdit	= dt;
	OLCED(ch)	= olced_lookup(ED_DAMT);
	return FALSE;
}

static void *damt_save_cb(void *p, va_list ap)
{
	damtype_t *dt = (damtype_t *)p;

	FILE *fp = va_arg(ap, FILE *);

	fprintf(fp, "#DAMTYPE\n");
	fprintf(fp, "Name %s\n", dt->dam_name);
	mlstr_fwrite(fp, "Noun", &dt->dam_noun.ml);
	mlstr_fwrite(fp, "Gender", &dt->dam_noun.gender);
	fprintf(fp, "Class %s\n",
		flag_string(dam_classes, dt->dam_class));
	if (dt->dam_slot >= 0)
		fprintf(fp, "Slot %d\n", dt->dam_slot);
	fprintf(fp, "End\n\n");

	return NULL;
}

OLC_FUN(damted_save)
{
	FILE *fp;

	if (!IS_SET(changed_flags, CF_DAMT)) {
		act_char("Damage types are not changed.", ch);
		return FALSE;
	}
	fp = olc_fopen(ETC_PATH, DAMTYPE_CONF, ch, SECURITY_MATERIAL);
	if (fp == NULL)
		return FALSE;

	c_foreach(&damtypes, damt_save_cb, fp);

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
	damtype_t *dt;
	BUFFER *buf;

	if (argument[0] == '\0') {
		if (IS_EDIT(ch, ED_DAMT))
			EDIT_DAMT(ch, dt);
		else
			OLC_ERROR("'OLC ASHOW'");
	} else if ((dt = damtype_search(argument)) == NULL) {
			act_puts("DamtEd: $t: no such damage type.",
				 ch, argument, NULL,
				 TO_CHAR | ACT_NOTRANS, POS_DEAD);
			return FALSE;
	}

	buf = buf_new(0);
	buf_printf(buf, BUF_END, "Name:       [%s]\n", dt->dam_name);
	mlstr_dump(buf, "NounDamage: ", &dt->dam_noun.ml, DUMP_LEVEL(ch));
	mlstr_dump(buf, "Gender:     ", &dt->dam_noun.gender, DL_NONE);
	buf_printf(buf, BUF_END, "Damclass:   [%s]\n",
		flag_string(dam_classes, dt->dam_class));
	if (dt->dam_slot)
		buf_printf(buf, BUF_END, "Slot:       [%d]\n", dt->dam_slot);

	page_to_char(buf_string(buf), ch);
	buf_free(buf);
	return FALSE;
}

OLC_FUN(damted_list)
{
	BUFFER	*buffer;

	buffer = buf_new(0);
	c_strkey_dump(&damtypes, buffer);
	page_to_char(buf_string(buffer), ch);
	buf_free(buffer);
	return FALSE;
}

OLC_FUN(damted_gender)
{
	damtype_t *dt;

	EDIT_DAMT(ch, dt);
	return olced_gender(ch, argument, cmd, &dt->dam_noun.gender);
}

OLC_FUN(damted_noun)
{
	damtype_t *dt;

	EDIT_DAMT(ch, dt);
	return olced_mlstr(ch, argument, cmd, &dt->dam_noun.ml);
}

OLC_FUN(damted_class)
{
	damtype_t *dt;

	EDIT_DAMT(ch, dt);
	return olced_flag(ch, argument, cmd, &dt->dam_class);
}

OLC_FUN(damted_slot)
{
	damtype_t *dt;

	EDIT_DAMT(ch, dt);
	return olced_number(ch, argument, cmd, &dt->dam_slot);
}

OLC_FUN(damted_delete)
{
	damtype_t *dt;

	EDIT_DAMT(ch, dt);

	if (olced_busy(ch, ED_DAMT, dt, NULL))
		return FALSE;

	c_delete(&damtypes, dt->dam_name);
	edit_done(ch->desc);
	return TRUE;
}
