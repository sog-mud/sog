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
 * $Id: db_material.c,v 1.17 2003-09-30 00:31:04 fjoe Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include <merc.h>
#include <db.h>
#include <rwfile.h>

DECLARE_DBLOAD_FUN(load_material);

DECLARE_DBINIT_FUN(init_materials);

DBFUN dbfun_materials[] =
{
	{ "MATERIAL",	load_material,	NULL	},		// notrans
	{ NULL, NULL, NULL }
};

DBDATA db_materials = { dbfun_materials, init_materials, 0 };

DBINIT_FUN(init_materials)
{
	if (!DBDATA_VALID(dbdata))
		c_init(&materials, &c_info_materials);
}

DBLOAD_FUN(load_material)
{
	material_t *mat = NULL;

	for (;;) {
		bool fMatch = FALSE;

		fread_keyword(fp);
		switch (rfile_tokfl(fp)) {
		case 'D':
			CHECK_VAR(mat, "Name");

			KEY("Damc", mat->dam_class,
			    fread_fword(dam_classes, fp));
			break;

		case 'E':
			CHECK_VAR(mat, "Name");

			if (IS_TOKEN(fp, "End"))
				return;
			break;

		case 'F':
			CHECK_VAR(mat, "Name");

			KEY("Float", mat->float_time, fread_number(fp));
			KEY("Flags", mat->mat_flags,
			    fread_fstring(material_flags, fp));
			break;

		case 'N':
			SPKEY("Name", mat->name, fread_string(fp),
			      &materials, mat);
			break;
		}

		if (!fMatch) {
			printlog(LOG_ERROR, "%s: %s: Unknown keyword",
			    __FUNCTION__, rfile_tok(fp));
			fread_to_eol(fp);
		}
	}
}
