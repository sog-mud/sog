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
 * $Id: olc_material.c,v 1.3 1999-10-18 19:05:36 avn Exp $
 */

#include "olc.h"

#define EDIT_MAT(ch, mat)	(mat = (material_t*) ch->desc->pEdit)

DECLARE_OLC_FUN(mated_create		);
DECLARE_OLC_FUN(mated_edit		);
DECLARE_OLC_FUN(mated_save		);
DECLARE_OLC_FUN(mated_touch		);
DECLARE_OLC_FUN(mated_show		);
DECLARE_OLC_FUN(mated_list		);

DECLARE_OLC_FUN(mated_name		);
DECLARE_OLC_FUN(mated_float		);
DECLARE_OLC_FUN(mated_damclass		);
DECLARE_OLC_FUN(mated_material		);

DECLARE_OLC_FUN(mated_delete		);

static DECLARE_VALIDATE_FUN(validate_name);

olc_cmd_t olc_cmds_mat[] =
{
	{ "create",	mated_create					},
	{ "edit",	mated_edit					},
	{ "",		mated_save					},
	{ "touch",	mated_touch					},
	{ "show",	mated_show					},
	{ "list",	mated_list					},

	{ "name",	mated_name,		validate_name		},
	{ "float",	mated_float					},
	{ "damclass",	mated_damclass,		NULL,	dam_classes	},
	{ "material",	mated_material,		NULL,	material_flags	},

	{ "delete",	mated_delete					},

	{ "commands",	show_commands					},
	{ NULL }
};

static void *material_save_cb(void *p, void *d);

OLC_FUN(mated_create)
{
	material_t mat;

	if (PC(ch)->security < SECURITY_MATERIAL) {
		char_puts("MatEd: Insufficient security for creating materials.\n", ch);
		return FALSE;
	}

	if (argument[0] == '\0') {
		dofun("help", ch, "'OLC CREATE'");
		return FALSE;
	}

	material_init(&mat);
	mat.name = str_dup(argument);

	if (!(ch->desc->pEdit = (void*)hash_insert(&materials, mat.name, &mat))) {
		char_printf(ch, "MatEd: %s: already exists.\n",
			    mat.name);
		return FALSE;
	}

	OLCED(ch)	= olced_lookup(ED_MATERIAL);
	char_puts("Material created.\n",ch);
	return FALSE;
}

OLC_FUN(mated_edit)
{
	material_t *mat;

	if (PC(ch)->security < SECURITY_MATERIAL) {
		char_puts("MatEd: Insufficient security.\n", ch);
		return FALSE;
	}

	if (argument[0] == '\0') {
		dofun("help", ch, "'OLC EDIT'");
		return FALSE;
	}

	if (!(mat = material_search(argument))) {
		char_printf(ch, "MatEd: %s: No such material.\n", argument);
		return FALSE;
	}

	ch->desc->pEdit	= mat;
	OLCED(ch)	= olced_lookup(ED_MATERIAL);
	return FALSE;
}

static void *material_save_cb(void *p, void *d)
{
	material_t *mat = (material_t *)p;
	FILE *fp = (FILE *)d;

	fprintf(fp, "#MATERIAL\n");
	fprintf(fp, "Name %s~\n", mat->name);
	if (mat->float_time)
		fprintf(fp, "Float %d\n", mat->float_time);
	if (mat->dam_class != DAM_NONE)
		fprintf(fp, "Damc %s~\n", flag_string(dam_classes, mat->dam_class));
	if (mat->mat_flags)
		fprintf(fp, "Flags %s~\n", flag_string(material_flags, mat->mat_flags));
	fprintf(fp, "End\n\n");
	return NULL;
}

OLC_FUN(mated_save)
{
	FILE *fp;

	fp = olc_fopen(ETC_PATH, MATERIALS_CONF, ch, SECURITY_MATERIAL);
	if (fp == NULL)
		return FALSE;

	hash_foreach(&materials, material_save_cb, fp);

	fprintf(fp, "#$\n");
	fclose(fp);
	olc_printf(ch, "Materials saved");
	return FALSE;
}

OLC_FUN(mated_touch)
{
	return FALSE;
}

OLC_FUN(mated_show)
{
	material_t *mat;

	if (argument[0] == '\0')
		if (IS_EDIT(ch, ED_MATERIAL))
			EDIT_MAT(ch, mat);
		else {
			dofun("help", ch, "'OLC ASHOW'");
			return FALSE;
		}
	else
		if (!(mat = material_search(argument))) {
			char_printf(ch, "MatEd: %s: no such material.\n", argument);
			return FALSE;
		}
	
	char_printf(ch, "Name [%s]\n", mat->name);
	if (mat->float_time)
		char_printf(ch, "Float [%d]\n", mat->float_time);
	if (mat->dam_class != DAM_NONE)
		char_printf(ch, "Damage class [%s]\n", flag_string(dam_classes, mat->dam_class));
	if (mat->mat_flags)
		char_printf(ch, "Flags [%s]\n", flag_string(material_flags, mat->mat_flags));
	return FALSE;
}

OLC_FUN(mated_list)
{
	BUFFER	*buffer;

	buffer = buf_new(-1);
	hash_print_names(&materials, buffer);
	page_to_char(buf_string(buffer), ch);
	buf_free(buffer);
	return FALSE;
}

OLC_FUN(mated_name)
{
	material_t *mat;

	EDIT_MAT(ch, mat);
	return olced_str(ch, argument, cmd, &mat->name);
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
	return olced_flag32(ch, argument, cmd, &mat->dam_class);
}

OLC_FUN(mated_material)
{
	material_t *mat;

	EDIT_MAT(ch, mat);
	return olced_flag32(ch, argument, cmd, &mat->mat_flags);
}

OLC_FUN(mated_delete)
{
	material_t *mat;

	EDIT_MAT(ch, mat);
	hash_delete(&materials, mat);
	return TRUE;
}

static VALIDATE_FUN(validate_name)
{
	material_t *mat, *m2;
	const char *old;

	EDIT_MAT(ch, mat);
	
	old = mat->name;
	mat->name = str_dup(arg);
	if (!(m2 = hash_insert(&materials, mat->name, mat))) {
		char_printf(ch, "MatEd: %s: duplicate name.\n", arg);
		free_string(mat->name);
		mat->name = old;
		return FALSE;
	}

	free_string(mat->name);
	mat->name = old;
	hash_delete(&materials, mat);
	ch->desc->pEdit = (void*)m2;
	return TRUE;
}

