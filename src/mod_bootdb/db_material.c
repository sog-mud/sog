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
 * $Id: db_material.c,v 1.4 1999-10-25 12:05:30 fjoe Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include "merc.h"
#include "db.h"

DECLARE_DBLOAD_FUN(load_material);

DECLARE_DBINIT_FUN(init_materials);

DBFUN dbfun_materials[] =
{
	{ "MATERIAL",	load_material	},
	{ NULL }
};

DBDATA db_materials = { dbfun_materials, init_materials };

DBINIT_FUN(init_materials)
{
	if (!DBDATA_VALID(dbdata)) {
		hash_init(&materials, STRKEY_HASH_SIZE, sizeof(material_t),
			  (varr_e_init_t) material_init,
			  (varr_e_destroy_t) material_destroy);
		materials.k_hash = strkey_hash;
		materials.ke_cmp = strkey_struct_cmp;
		materials.e_cpy = (hash_e_cpy_t) material_cpy;
	}
}

DBLOAD_FUN(load_material)
{
	material_t mat;

	material_init(&mat);

	for (;;) {
		char *word = rfile_feof(fp) ? "End" : fread_word(fp);
		bool fMatch = FALSE;

		switch (UPPER(word[0])) {
		case 'D':
			KEY("Damc", mat.dam_class, fread_fword(dam_classes, fp));
			break;
		case 'E':
			if (!str_cmp(word, "End")) {
				if (IS_NULLSTR(mat.name)) {
					db_error("load_material",
						 "material name undefined");
				} else if (!hash_insert(&materials, mat.name, &mat)) {
					db_error("load_material",
						 "duplicate material name");
				}
				material_destroy(&mat);
				return;
			}
			break;
		case 'F':
			KEY("Float", mat.float_time, fread_number(fp));
			KEY("Flags", mat.mat_flags,
			    fread_fstring(material_flags, fp));
			break;
		case 'N':
			SKEY("Name", mat.name, fread_string(fp));
			break;
		}

		if (!fMatch)
			db_error("load_material", "%s: Unknown keyword", word);
	}
}

