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
 * $Id: olc_material.c,v 1.26 2004-06-28 19:21:04 tatyana Exp $
 */

#include "olc.h"

#define EDIT_MAT(ch, mat)	(mat = (material_t *) ch->desc->pEdit)

DECLARE_OLC_FUN(mated_create		);
DECLARE_OLC_FUN(mated_edit		);
DECLARE_OLC_FUN(mated_save		);
DECLARE_OLC_FUN(mated_touch		);
DECLARE_OLC_FUN(mated_show		);
DECLARE_OLC_FUN(mated_list		);

DECLARE_OLC_FUN(mated_float		);
DECLARE_OLC_FUN(mated_damclass		);
DECLARE_OLC_FUN(mated_material		);

DECLARE_OLC_FUN(mated_delete		);

olced_strkey_t strkey_materials = { &materials, NULL, NULL };

olc_cmd_t olc_cmds_mat[] =
{
	{ "create",	mated_create,	NULL,	NULL			},
	{ "edit",	mated_edit,	NULL,	NULL			},
	{ "",		mated_save,	NULL,	NULL			},
	{ "touch",	mated_touch,	NULL,	NULL			},
	{ "show",	mated_show,	NULL,	NULL			},
	{ "list",	mated_list,	NULL,	NULL			},

	{ "name",	olced_strkey,	NULL,	&strkey_materials	},
	{ "float",	mated_float,	NULL,	NULL			},
	{ "damclass",	mated_damclass,	NULL,	dam_classes		},
	{ "material",	mated_material,	NULL,	material_flags		},

	{ "delete_ma",	olced_spell_out, NULL, NULL			},
	{ "delete_mat", mated_delete,	NULL, NULL			},

	{ "commands",	show_commands,	NULL,	NULL			},
	{ "version",	show_version,	NULL,	NULL			},

	{ NULL, NULL, NULL, NULL }
};

OLC_FUN(mated_create)
{
	material_t *m;
	char arg[MAX_INPUT_LENGTH];

	if (PC(ch)->security < SECURITY_MATERIAL) {
		act_char("MatEd: Insufficient security for creating materials.", ch);
		return FALSE;
	}

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0')
		OLC_ERROR("'OLC CREATE'");

	if ((m = c_insert(&materials, arg)) == NULL) {
		act_puts("MatEd: $t: already exists.",
			 ch, arg, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	m->name		= str_dup(arg);
	OLCED(ch)	= olced_lookup(ED_MATERIAL);
	SET_BIT(changed_flags, CF_MATERIAL);
	ch->desc->pEdit = m;
	act_char("Material created.", ch);
	return FALSE;
}

OLC_FUN(mated_edit)
{
	material_t *mat;
	char arg[MAX_INPUT_LENGTH];

	if (PC(ch)->security < SECURITY_MATERIAL) {
		act_char("MatEd: Insufficient security.", ch);
		return FALSE;
	}

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0')
		OLC_ERROR("'OLC EDIT'");

	if (!(mat = material_search(arg))) {
		act_puts("MatEd: $t: No such material.",
			 ch, arg, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	ch->desc->pEdit	= mat;
	OLCED(ch)	= olced_lookup(ED_MATERIAL);
	return FALSE;
}

OLC_FUN(mated_save)
{
	FILE *fp;
	material_t *mat;

	if (!IS_SET(changed_flags, CF_MATERIAL)) {
		act_char("Materials are not changed.", ch);
		return FALSE;
	}
	fp = olc_fopen(ETC_PATH, MATERIALS_CONF, ch, SECURITY_MATERIAL);
	if (fp == NULL)
		return FALSE;

	C_FOREACH (material_t *, mat, &materials) {
		fprintf(fp, "#MATERIAL\n");
		fprintf(fp, "Name %s~\n", mat->name);
		if (mat->float_time)
			fprintf(fp, "Float %d\n", mat->float_time);
		if (mat->dam_class != DAM_NONE) {
			fprintf(fp, "Damc %s\n",
				flag_string(dam_classes, mat->dam_class));
		}
		if (mat->mat_flags) {
			fprintf(fp, "Flags %s~\n",
				flag_string(material_flags, mat->mat_flags));
		}
		fprintf(fp, "End\n\n");
	}

	fprintf(fp, "#$\n");
	fclose(fp);
	olc_printf(ch, "Materials saved.");
	REMOVE_BIT(changed_flags, CF_MATERIAL);
	return FALSE;
}

OLC_FUN(mated_touch)
{
	SET_BIT(changed_flags, CF_MATERIAL);
	return FALSE;
}

OLC_FUN(mated_show)
{
	material_t *mat;
	char arg[MAX_INPUT_LENGTH];

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0')
		if (IS_EDIT(ch, ED_MATERIAL))
			EDIT_MAT(ch, mat);
		else
			OLC_ERROR("'OLC ASHOW'");
	else
		if (!(mat = material_search(arg))) {
			act_puts("MatEd: $t: no such material.",
				 ch, arg, NULL,
				 TO_CHAR | ACT_NOTRANS, POS_DEAD);
			return FALSE;
		}

	act_puts("Name [$t]",
		 ch, mat->name, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
	if (mat->float_time) {
		act_puts("Float [$j]",
			 ch, (const void *) mat->float_time, NULL,
			 TO_CHAR, POS_DEAD);
	}
	if (mat->dam_class != DAM_NONE) {
		act_puts("Damage class [$t]",
			 ch, flag_string(dam_classes, mat->dam_class),
			 NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
	}
	if (mat->mat_flags) {
		act_puts("Flags [$t]",
			 ch, flag_string(material_flags, mat->mat_flags), NULL,
			 TO_CHAR | ACT_NOTRANS, POS_DEAD);
	}
	return FALSE;
}

OLC_FUN(mated_list)
{
	BUFFER	*buffer;

	buffer = buf_new(0);
	c_strkey_dump(&materials, buffer);
	page_to_char(buf_string(buffer), ch);
	buf_free(buffer);
	return FALSE;
}

OLC_FUN(mated_float)
{
	material_t *mat;

	EDIT_MAT(ch, mat);
	return olced_number(ch, argument, cmd, &mat->float_time);
}

OLC_FUN(mated_damclass)
{
	material_t *mat;

	EDIT_MAT(ch, mat);
	return olced_flag(ch, argument, cmd, &mat->dam_class);
}

OLC_FUN(mated_material)
{
	material_t *mat;

	EDIT_MAT(ch, mat);
	return olced_flag(ch, argument, cmd, &mat->mat_flags);
}

OLC_FUN(mated_delete)
{
	material_t *mat;

	EDIT_MAT(ch, mat);

	if (olced_busy(ch, ED_MATERIAL, mat, NULL))
		return FALSE;

	c_delete(&materials, mat->name);
	edit_done(ch->desc);
	return TRUE;
}
