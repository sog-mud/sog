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
 * $Id: olc_damtype.c,v 1.3 2000-10-07 10:58:02 fjoe Exp $
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

#define EDIT_DAMT(ch, d)	(d = (damtype_t*) ch->desc->pEdit)

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

olced_strkey_t strkey_damtypes = { &damtypes, NULL };

olc_cmd_t olc_cmds_damt[] =
{
	{ "create",	damted_create					},
	{ "edit",	damted_edit					},
	{ "",		damted_save					},
	{ "touch",	damted_touch					},
	{ "show",	damted_show					},
	{ "list",	damted_list					},

	{ "name",	olced_strkey,	NULL,	&strkey_damtypes	},
	{ "gender",	damted_gender,	NULL,	gender_table		},
	{ "noundamage",	damted_noun					},
	{ "class",	damted_class,	NULL,	dam_classes		},
	{ "slot",	damted_slot					},

	{ "delete_dam",	olced_spell_out					},
	{ "delete_damt",damted_delete					},

	{ "commands",	show_commands					},
	{ NULL }
};

static void *damt_save_cb(void *p, va_list ap);

OLC_FUN(damted_create)
{
	damtype_t dt;
	damtype_t *d;
	char arg[MAX_INPUT_LENGTH];

	if (PC(ch)->security < SECURITY_DAMT) {
		char_puts("DamtEd: Insufficient security for creating damage types.\n", ch);
		return FALSE;
	}

	if (argument[0] == '\0')
		OLC_ERROR("'OLC CREATE'");

	one_argument(argument, arg, sizeof(arg));

	/*
	 * olced_busy check is not needed since hash_insert
	 * adds new elements to the end of varr
	 */

	damtype_init(&dt);
	dt.dam_name = str_dup(arg);
	d = hash_insert(&damtypes, arg, &dt);
	damtype_destroy(&dt);

	if (d == NULL) {
		char_printf(ch, "DamtEd: %s: already exists.\n", arg);
		return FALSE;
	}

	OLCED(ch)	= olced_lookup(ED_DAMT);
	ch->desc->pEdit = d;
	char_puts("Damtype created.\n",ch);
	SET_BIT(changed_flags, CF_DAMT);
	return FALSE;
}

OLC_FUN(damted_edit)
{
	damtype_t *dt;

	if (PC(ch)->security < SECURITY_DAMT) {
		char_puts("DamtEd: Insufficient security.\n", ch);
		return FALSE;
	}

	if (argument[0] == '\0')
		OLC_ERROR("'OLC EDIT'");

	if ((dt = damtype_search(argument)) == NULL) {
		char_printf(ch, "DamtEd: %s: No such damtype.\n", argument);
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
		char_puts("Damage types are not changed.\n", ch);
		return FALSE;
	}
	fp = olc_fopen(ETC_PATH, DAMTYPE_CONF, ch, SECURITY_MATERIAL);
	if (fp == NULL)
		return FALSE;

	hash_foreach(&damtypes, damt_save_cb, fp);

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
			char_printf(ch, "DamtEd: %s: no such damage type.\n", argument);
			return FALSE;
	}
	
	buf = buf_new(-1);
	buf_printf(buf, BUF_END, "Name:       [%s]\n", dt->dam_name);
	mlstr_dump(buf, "NounDamage: ", &dt->dam_noun.ml);
	mlstr_dump(buf, "Gender:     ", &dt->dam_noun.gender);
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

	buffer = buf_new(-1);
	strkey_printall(&damtypes, buffer);
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

	if (olced_busy(ch, ED_DAMT, NULL, NULL))
		return FALSE;

	EDIT_DAMT(ch, dt);
	hash_delete(&damtypes, dt->dam_name);
	edit_done(ch->desc);
	return TRUE;
}

