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
 * $Id: db_material.c,v 1.11 2001-06-24 10:51:02 avn Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include "merc.h"
#include "db.h"

DECLARE_DBLOAD_FUN(load_material);

DECLARE_DBINIT_FUN(init_materials);

DBFUN dbfun_materials[] =
{
	{ "MATERIAL",	load_material,	NULL	},		// notrans
	{ NULL, NULL, NULL }
};

DBDATA db_materials = { dbfun_materials, init_materials, 0 };

static hashdata_t h_materials =
{
	sizeof(material_t), 1,
	(e_init_t) material_init,
	(e_destroy_t) material_destroy,
	(e_cpy_t) material_cpy,

	STRKEY_HASH_SIZE,
	k_hash_str,
	ke_cmp_str
};

DBINIT_FUN(init_materials)
{
	if (!DBDATA_VALID(dbdata))
		hash_init(&materials, &h_materials);
}

DBLOAD_FUN(load_material)
{
	material_t mat;

	material_init(&mat);

	for (;;) {
		bool fMatch = FALSE;

		fread_keyword(fp);
		switch (rfile_tokfl(fp)) {
		case 'D':
			KEY("Damc", mat.dam_class, fread_fword(dam_classes, fp));
			break;
		case 'E':
			if (IS_TOKEN(fp, "End")) {
				if (IS_NULLSTR(mat.name)) {
					log(LOG_ERROR, "load_material: material name undefined");
				} else if (!hash_insert(&materials, mat.name, &mat)) {
					log(LOG_ERROR, "load_material: duplicate material name");
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

		if (!fMatch) {
			log(LOG_ERROR, "load_material: %s: Unknown keyword",
				 rfile_tok(fp));
			fread_to_eol(fp);
		}
	}
}

