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
 * $Id: db_form.c,v 1.13 2001-08-02 18:19:57 fjoe Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include <merc.h>
#include <bootdb.h>
#include <rwfile.h>

DECLARE_DBLOAD_FUN(load_form);

DECLARE_DBINIT_FUN(init_form);

DBFUN dbfun_forms[] =
{
	{ "FORM",	load_form,	NULL	},		// notrans
	{ NULL, NULL, NULL }
};

DBDATA db_forms = { dbfun_forms, init_form, 0 };

static hashdata_t h_forms =
{
	sizeof(form_index_t), 1,
	(e_init_t) form_init,
	(e_destroy_t) form_destroy,
	(e_cpy_t) form_cpy,

	STRKEY_HASH_SIZE,
	k_hash_str,
	ke_cmp_str
};

DBINIT_FUN(init_form)
{
	if (!DBDATA_VALID(dbdata))
		hash_init(&forms, &h_forms);
}

DBLOAD_FUN(load_form)
{
	form_index_t f;

	form_init(&f);

	for (;;) {
		bool fMatch = FALSE;

		fread_keyword(fp);
		switch (rfile_tokfl(fp)) {
		case 'A':
			KEY("Attacks", f.num_attacks, fread_number(fp));
			break;
		case 'D':
			KEY("Damtype", f.damtype, fread_strkey(
			    fp, &damtypes, "load_form"));	// notrans
			if (IS_TOKEN(fp, "Damage")) {
				f.damage[DICE_NUMBER]	= fread_number(fp);
				fread_letter(fp);
				f.damage[DICE_TYPE]	= fread_number(fp);
				fread_letter(fp);
				f.damage[DICE_BONUS]	= fread_number(fp);
				fMatch = TRUE;
				break;
			}
			MLSKEY("Description", f.description);
			break;
		case 'F':
			KEY("Flags", f.flags, fread_fstring(shapeform_flags, fp));
			break;
		case 'H':
			KEY("Hitroll", f.hitroll, fread_number(fp));
			break;
		case 'E':
			if (IS_TOKEN(fp, "End")) {
				if (IS_NULLSTR(f.name)) {
					log(LOG_ERROR, "load_form: form name undefined");
				} else if (!hash_insert(&forms, f.name, &f)) {
					log(LOG_ERROR, "load_form: duplicate form name");
				}
				form_destroy(&f);
				return;
			}
			break;
		case 'L':
			MLSKEY("LongDesc", f.long_desc);
			break;
		case 'N':
			KEY("Name", f.name, fread_sword(fp));
			break;
		case 'S':
			MLSKEY("ShortDesc", f.short_desc);
			SKEY("SkillSpec", f.skill_spec, fread_strkey(
			    fp, &specs, "load_form"))		// notrans
			if (IS_TOKEN(fp, "Stats")) {
				int i;
				for (i = 0; i < MAX_STAT; i++)
					f.stats[i] = fread_number(fp);
				fMatch = TRUE;
				break;
			}
			break;
		case 'R':
			if (IS_TOKEN(fp, "Resist")) {
				int res = fread_fword(dam_classes, fp);
				if (res < 0 || res == DAM_NONE) {
					log(LOG_ERROR, "load_form: unknown resistance name");
					fread_number(fp);
				} else {
					f.resists[res] = fread_number(fp);
				}
				fMatch = TRUE;
				break;
			}
			break;

		}

		if (!fMatch) {
			log(LOG_ERROR, "load_form: %s: Unknown keyword",
				 rfile_tok(fp));
			fread_to_eol(fp);
		}
	}
}
