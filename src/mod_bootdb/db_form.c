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
 * $Id: db_form.c,v 1.17 2001-09-13 16:21:58 fjoe Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include <merc.h>
#include <db.h>
#include <rwfile.h>

DECLARE_DBLOAD_FUN(load_form);

DECLARE_DBINIT_FUN(init_form);

DBFUN dbfun_forms[] =
{
	{ "FORM",	load_form,	NULL	},		// notrans
	{ NULL, NULL, NULL }
};

DBDATA db_forms = { dbfun_forms, init_form, 0 };

DBINIT_FUN(init_form)
{
	if (!DBDATA_VALID(dbdata))
		c_init(&forms, &c_info_forms);
}

DBLOAD_FUN(load_form)
{
	form_index_t *f = NULL;

	for (;;) {
		bool fMatch = FALSE;

		fread_keyword(fp);
		switch (rfile_tokfl(fp)) {
		case 'A':
			CHECK_VAR(f, "Name");

			KEY("Attacks", f->num_attacks, fread_number(fp));
			break;

		case 'D':
			CHECK_VAR(f, "Name");

			KEY("Damtype", f->damtype, fread_strkey(fp, &damtypes));
			if (IS_TOKEN(fp, "Damage")) {
				f->damage[DICE_NUMBER]	= fread_number(fp);
				fread_letter(fp);
				f->damage[DICE_TYPE]	= fread_number(fp);
				fread_letter(fp);
				f->damage[DICE_BONUS]	= fread_number(fp);
				fMatch = TRUE;
				break;
			}
			MLSKEY("Description", f->description);
			break;

		case 'F':
			CHECK_VAR(f, "Name");

			KEY("Flags", f->flags,
			    fread_fstring(shapeform_flags, fp));
			break;

		case 'H':
			CHECK_VAR(f, "Name");

			KEY("Hitroll", f->hitroll, fread_number(fp));
			break;

		case 'E':
			CHECK_VAR(f, "Name");

			if (IS_TOKEN(fp, "End"))
				return;
			break;

		case 'L':
			CHECK_VAR(f, "Name");

			MLSKEY("LongDesc", f->long_desc);
			break;

		case 'N':
			SPKEY("Name", f->name, fread_sword(fp),
			      &forms, f);
			break;

		case 'S':
			CHECK_VAR(f, "Name");

			MLSKEY("ShortDesc", f->short_desc);
			SKEY("SkillSpec", f->skill_spec,
			     fread_strkey(fp, &specs));
			if (IS_TOKEN(fp, "Stats")) {
				int i;
				for (i = 0; i < MAX_STAT; i++)
					f->stats[i] = fread_number(fp);
				fMatch = TRUE;
				break;
			}
			break;

		case 'R':
			CHECK_VAR(f, "Name");

			if (IS_TOKEN(fp, "Resist")) {
				int res = fread_fword(dam_classes, fp);
				if (res < 0 || res == DAM_NONE) {
					log(LOG_ERROR, "load_form: unknown resistance name");
					fread_number(fp);
				} else {
					f->resists[res] = fread_number(fp);
				}
				fMatch = TRUE;
				break;
			}
			break;

		}

		if (!fMatch) {
			log(LOG_ERROR, "%s: %s: Unknown keyword",
			    __FUNCTION__, rfile_tok(fp));
			fread_to_eol(fp);
		}
	}
}
