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
 * $Id: olc_liquid.c,v 1.5 1999-12-15 15:35:39 fjoe Exp $
 */

#include "olc.h"

#define EDIT_LIQ(ch, lq)	(lq = (liquid_t*) ch->desc->pEdit)

DECLARE_OLC_FUN(liqed_create		);
DECLARE_OLC_FUN(liqed_edit		);
DECLARE_OLC_FUN(liqed_save		);
DECLARE_OLC_FUN(liqed_touch		);
DECLARE_OLC_FUN(liqed_show		);
DECLARE_OLC_FUN(liqed_list		);

DECLARE_OLC_FUN(liqed_gender		);
DECLARE_OLC_FUN(liqed_color		);
DECLARE_OLC_FUN(liqed_affect		);
DECLARE_OLC_FUN(liqed_sip		);

DECLARE_OLC_FUN(liqed_delete		);

olced_strkey_t strkey_liquids = { &liquids, NULL };

olc_cmd_t olc_cmds_liq[] =
{
	{ "create",	liqed_create					},
	{ "edit",	liqed_edit					},
	{ "",		liqed_save					},
	{ "touch",	liqed_touch					},
	{ "show",	liqed_show					},
	{ "list",	liqed_list					},

	{ "name",	olced_mlstrkey,	NULL,	&strkey_liquids		},
	{ "gender",	liqed_gender,	NULL,	gender_table		},
	{ "color",	liqed_color					},
	{ "affect",	liqed_affect					},
	{ "sip",	liqed_sip					},

	{ "delete",	liqed_delete					},

	{ "commands",	show_commands					},
	{ NULL }
};

static void *liquid_save_cb(void *p, va_list ap);

OLC_FUN(liqed_create)
{
	liquid_t lq;
	liquid_t *l;

	if (PC(ch)->security < SECURITY_MATERIAL) {
		char_puts("LiqEd: Insufficient security for creating liquids.\n", ch);
		return FALSE;
	}

	if (argument[0] == '\0')
		OLC_ERROR("'OLC CREATE'");

	/*
	 * olced_busy check is not needed since hash_insert
	 * adds new elements to the end of varr
	 */

	liquid_init(&lq);
	mlstr_init(&lq.lq_name, argument);
	l = hash_insert(&liquids, argument, &lq);
	liquid_destroy(&lq);

	if (l == NULL) {
		char_printf(ch, "LiqEd: %s: already exists.\n", argument);
		return FALSE;
	}

	OLCED(ch)	= olced_lookup(ED_LIQUID);
	ch->desc->pEdit = l;
	char_puts("Liquid created.\n",ch);
	SET_BIT(changed_flags, CF_LIQUID);
	return FALSE;
}

OLC_FUN(liqed_edit)
{
	liquid_t *lq;

	if (PC(ch)->security < SECURITY_MATERIAL) {
		char_puts("LiqEd: Insufficient security.\n", ch);
		return FALSE;
	}

	if (argument[0] == '\0')
		OLC_ERROR("'OLC EDIT'");

	if (!(lq = liquid_search(argument))) {
		char_printf(ch, "LiqEd: %s: No such liquid.\n", argument);
		return FALSE;
	}

	ch->desc->pEdit	= lq;
	OLCED(ch)	= olced_lookup(ED_LIQUID);
	return FALSE;
}

static void *liquid_save_cb(void *p, va_list ap)
{
	liquid_t *lq = (liquid_t *)p;

	FILE *fp = va_arg(ap, FILE *);

	int i;

	fprintf(fp, "#LIQUID\n");
	mlstr_fwrite(fp, "Name", &lq->lq_name);
	mlstr_fwrite(fp, "Gender", &lq->lq_gender);
	mlstr_fwrite(fp, "Color", &lq->lq_color);
	fprintf(fp, "Affect");
	for (i = 0; i < MAX_COND; i++)
		fprintf(fp, " %d", lq->affect[i]);
	fprintf(fp, "\n");
	fprintf(fp, "Sip %d\n", lq->sip);
	fprintf(fp, "End\n\n");
	return NULL;
}

OLC_FUN(liqed_save)
{
	FILE *fp;

	if (!IS_SET(changed_flags, CF_LIQUID)) {
		char_puts("Liquids are not changed.\n", ch);
		return FALSE;
	}
	fp = olc_fopen(ETC_PATH, LIQUIDS_CONF, ch, SECURITY_MATERIAL);
	if (fp == NULL)
		return FALSE;

	hash_foreach(&liquids, liquid_save_cb, fp);

	fprintf(fp, "#$\n");
	fclose(fp);
	olc_printf(ch, "Liquids saved.");
	REMOVE_BIT(changed_flags, CF_LIQUID);
	return FALSE;
}

OLC_FUN(liqed_touch)
{
	SET_BIT(changed_flags, CF_LIQUID);
	return FALSE;
}

OLC_FUN(liqed_show)
{
	liquid_t *lq;
	int i;
	BUFFER *buf;

	if (argument[0] == '\0') {
		if (IS_EDIT(ch, ED_LIQUID))
			EDIT_LIQ(ch, lq);
		else 
			OLC_ERROR("'OLC ASHOW'");
	} else if ((lq = liquid_search(argument)) == NULL) {
			char_printf(ch, "LiqEd: %s: no such liquid.\n", argument);
			return FALSE;
	}
	
	buf = buf_new(-1);
	mlstr_dump(buf, "Name:   ", &lq->lq_name);
	mlstr_dump(buf, "Gender: ", &lq->lq_gender);
	mlstr_dump(buf, "Color:  ", &lq->lq_color);
	if (lq->sip)
		buf_printf(buf, "Sip:    [%d]\n", lq->sip);

	buf_add(buf, "Affects:\n");
	for (i = 0; i < MAX_COND; i++) {
		if (lq->affect[i]) {
			buf_printf(buf, "  %s by [%d]\n",
				    flag_string(cond_table, i), lq->affect[i]);
		}
	}

	page_to_char(buf_string(buf), ch);
	buf_free(buf);
	return FALSE;
}

OLC_FUN(liqed_list)
{
	BUFFER	*buffer;

	buffer = buf_new(-1);
	strkey_printall(&liquids, buffer);
	page_to_char(buf_string(buffer), ch);
	buf_free(buffer);
	return FALSE;
}

OLC_FUN(liqed_gender)
{
	liquid_t *lq;

	EDIT_LIQ(ch, lq);
	return olced_gender(ch, argument, cmd, &lq->lq_gender);
}

OLC_FUN(liqed_color)
{
	liquid_t *lq;

	EDIT_LIQ(ch, lq);
	return olced_mlstr(ch, argument, cmd, &lq->lq_color);
}

OLC_FUN(liqed_sip)
{
	liquid_t *lq;

	EDIT_LIQ(ch, lq);
	return olced_number(ch, argument, cmd, &lq->sip);
}

OLC_FUN(liqed_affect)
{
	liquid_t *lq;
	char arg[MAX_INPUT_LENGTH];
	int i;

	EDIT_LIQ(ch, lq);

	if (IS_NULLSTR(argument)) {
		char_puts("Syntax: affect <condition> <value>\n", ch);
		return FALSE;
	}
	argument = one_argument(argument, arg, sizeof(arg));

	if ((i = flag_value(cond_table, arg)) < 0) {
		show_flags(ch, cond_table);
		return FALSE;
	}

	return olced_number(ch, argument, cmd, &lq->affect[i]);
}

OLC_FUN(liqed_delete)
{
	liquid_t *lq;

	EDIT_LIQ(ch, lq);
	hash_delete(&liquids, lq);
	edit_done(ch->desc);
	return TRUE;
}

