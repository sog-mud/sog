/*-
 * Copyright (c) 1998 SoG Development Team
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
 * $Id: db_class.c,v 1.45 2003-09-30 00:31:03 fjoe Exp $
 */

#include <stdio.h>
#include <string.h>

#include <merc.h>
#include <db.h>
#include <rwfile.h>

DECLARE_DBLOAD_FUN(load_class);
DECLARE_DBLOAD_FUN(load_pose);

DECLARE_DBINIT_FUN(init_class);

DBFUN dbfun_classes[] =
{
	{ "CLASS",	load_class,	NULL	},		// notrans
	{ "POSE",	load_pose,	NULL	},		// notrans
	{ NULL, NULL, NULL }
};

DBDATA db_classes = { dbfun_classes, init_class, 0 };

DBINIT_FUN(init_class)
{
	if (DBDATA_VALID(dbdata))
		db_set_arg(dbdata, "POSE", NULL);
	else
		c_init(&classes, &c_info_classes);
}

DBLOAD_FUN(load_class)
{
	int i;
	class_t *cl = NULL;

	for (;;) {
		bool fMatch = FALSE;

		fread_keyword(fp);
		switch(rfile_tokfl(fp)) {
		case 'A':
			CHECK_VAR(cl, "Name");

			KEY("AddExp", cl->points, fread_number(fp));
			break;

		case 'D':
			CHECK_VAR(cl, "Name");

			KEY("DeathLimit", cl->death_limit,
			    fread_number(fp));
			break;

		case 'E':
			CHECK_VAR(cl, "Name");

			if (IS_TOKEN(fp, "End")) {
				varr_qsort(&cl->guilds, cmpint);
				db_set_arg(dbdata, "POSE", cl);
				return;
			}

		case 'F':
			CHECK_VAR(cl, "Name");

			KEY("Flags", cl->class_flags,
			    fread_fstring(class_flags, fp));
			break;

		case 'G':
			CHECK_VAR(cl, "Name");

			if (IS_TOKEN(fp, "GuildRoom")) {
				int vnum = fread_number(fp);
				int *pvnum = varr_enew(&cl->guilds);
				*pvnum = vnum;
				fMatch = TRUE;
			}
			break;

		case 'H':
			CHECK_VAR(cl, "Name");

			KEY("HPRate", cl->hp_rate, fread_number(fp));
			break;

		case 'L':
			CHECK_VAR(cl, "Name");

			KEY("LuckBonus", cl->luck_bonus, fread_number(fp));
			break;

		case 'M':
			CHECK_VAR(cl, "Name");

			KEY("ManaRate", cl->mana_rate, fread_number(fp));
			break;

		case 'N':
			SPKEY("Name", cl->name, fread_string(fp),
			      &classes, cl);
			break;

		case 'P':
			CHECK_VAR(cl, "Name");

			KEY("PrimeStat", cl->attr_prime,
			    fread_fword(stat_aliases, fp));
			break;

		case 'R':
			CHECK_VAR(cl, "Name");

			KEY("RestrictAlign", cl->restrict_align,
			    fread_fstring(ralign_names, fp));
			SKEY("RestrictSex", cl->restrict_sex,
			     fread_string(fp));
			KEY("RestrictEthos", cl->restrict_ethos,
			    fread_fstring(ethos_table, fp));
			break;

		case 'S':
			CHECK_VAR(cl, "Name");

			KEY("SchoolWeapon", cl->weapon,
			    fread_number(fp));
			SKEY("SkillSpec", cl->skill_spec,
			     fread_strkey(fp, &specs));
			if (IS_TOKEN(fp, "ShortName")) {
				const char *p = fread_string(fp);
				strlcpy(cl->who_name, p, sizeof(cl->who_name));
				free_string(p);
				fMatch = TRUE;
			}
			if (IS_TOKEN(fp, "StatMod")) {
				for (i = 0; i < MAX_STAT; i++)
					cl->mod_stat[i] = fread_number(fp);
				fMatch = TRUE;
			}
			break;

		case 'T':
			CHECK_VAR(cl, "Name");

			KEY("Thac0_00", cl->thac0_00, fread_number(fp));
			KEY("Thac0_32", cl->thac0_32, fread_number(fp));
			break;
		}

		if (!fMatch) {
			printlog(LOG_ERROR, "%s: %s: Unknown keyword",
			    __FUNCTION__, rfile_tok(fp));
			fread_to_eol(fp);
		}
	}
}

DBLOAD_FUN(load_pose)
{
	class_t *class = arg;
	pose_t *pose;

	if (!class) {
		printlog(LOG_ERROR, "load_pose: No #CLASS seen yet");
		return;
	}

	pose = varr_enew(&class->poses);
	for (;;) {
		bool fMatch = FALSE;

		fread_keyword(fp);
		switch(rfile_tokfl(fp)) {
		case 'E':
			if (IS_TOKEN(fp, "End"))
				return;
			break;

		case 'O':
			SKEY("Others", pose->others, fread_string(fp));
			break;

		case 'S':
			SKEY("Self", pose->self, fread_string(fp));
			break;
		}

		if (!fMatch) {
			printlog(LOG_ERROR, "%s: %s: Unknown keyword",
			    __FUNCTION__, rfile_tok(fp));
			fread_to_eol(fp);
		}
	}
}
