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
 * $Id: db_class.c,v 1.33 2000-08-04 14:12:50 cs Exp $
 */

#include <stdio.h>
#include <string.h>

#include "merc.h"
#include "db.h"

DECLARE_DBLOAD_FUN(load_class);
DECLARE_DBLOAD_FUN(load_pose);

DECLARE_DBINIT_FUN(init_class);

DBFUN dbfun_classes[] =
{
	{ "CLASS",	load_class	},
	{ "POSE",	load_pose	},
	{ NULL }
};

DBDATA db_classes = { dbfun_classes, init_class };

static hashdata_t h_classes =
{
	sizeof(class_t), 1,
	(e_init_t) class_init,
	(e_destroy_t) class_destroy,
	(e_cpy_t) class_cpy,

	STRKEY_HASH_SIZE,
	k_hash_str,
	ke_cmp_str
};

DBINIT_FUN(init_class)
{
	if (DBDATA_VALID(dbdata))
		db_set_arg(dbdata, "POSE", NULL);
	else
		hash_init(&classes, &h_classes);
}

DBLOAD_FUN(load_class)
{
	int i;
	class_t class;

	class_init(&class);

	for (;;) {
		bool fMatch = FALSE;

		fread_keyword(fp);
		switch(rfile_tokfl(fp)) {
		case 'A':
			KEY("AddExp", class.points, fread_number(fp));
			break;
		case 'D':
			KEY("DeathLimit", class.death_limit,
			    fread_number(fp));
			break;
		case 'E':
			if (IS_TOKEN(fp, "End")) {
				class_t *cl;

				if (IS_NULLSTR(class.name)) {
					log(LOG_ERROR, "load_class: class name undefined");
				} else if ((cl = hash_insert(&classes,
						class.name, &class)) == NULL) {
					log(LOG_ERROR, "load_class: duplicate class name");
				} else {
					varr_qsort(&cl->guilds, cmpint);
					db_set_arg(dbdata, "POSE", cl);
				}
				class_destroy(&class);
				return;
			}
		case 'F':
			KEY("Flags", class.class_flags,
			    fread_fstring(class_flags, fp));
			break;
		case 'G':
			if (IS_TOKEN(fp, "GuildRoom")) {
				int vnum = fread_number(fp);
				int *pvnum = varr_enew(&class.guilds);
				*pvnum = vnum;
				fMatch = TRUE;
			}
			break;
		case 'H':
			KEY("HPRate", class.hp_rate, fread_number(fp));
			break;
		case 'L':
			KEY("LuckBonus", class.luck_bonus, fread_number(fp));
			break;
		case 'M':
			KEY("ManaRate", class.mana_rate, fread_number(fp));
			break;
		case 'N':
			SKEY("Name", class.name, fread_string(fp));
			break;
		case 'P':
			KEY("PrimeStat", class.attr_prime,
			    fread_fword(stat_names, fp));
			break;
		case 'R':
			KEY("RestrictAlign", class.restrict_align,
			    fread_fstring(ralign_names, fp));
			SKEY("RestrictSex", class.restrict_sex,
			     fread_string(fp));
			KEY("RestrictEthos", class.restrict_ethos,
			    fread_fstring(ethos_table, fp));
			break;
		case 'S':
			KEY("SchoolWeapon", class.weapon,
			    fread_number(fp));
			SKEY("SkillSpec", class.skill_spec,
			     fread_strkey(fp, &specs, "load_class"));
			if (IS_TOKEN(fp, "ShortName")) {
				const char *p = fread_string(fp);
				strnzcpy(class.who_name,
					 sizeof(class.who_name), p);
				free_string(p);
				fMatch = TRUE;
			}
			if (IS_TOKEN(fp, "StatMod")) {
				for (i = 0; i < MAX_STAT; i++)
					class.mod_stat[i] = fread_number(fp);
				fMatch = TRUE;
			}
			break;
		case 'T':
			KEY("Thac0_00", class.thac0_00, fread_number(fp));
			KEY("Thac0_32", class.thac0_32, fread_number(fp));
			break;
		}

		if (!fMatch) {
			log(LOG_ERROR, "load_class: %s: Unknown keyword",
				 rfile_tok(fp));
			fread_to_eol(fp);
		}
	}
}

DBLOAD_FUN(load_pose)
{
	class_t *class = arg;
	pose_t *pose;

	if (!class) {
		log(LOG_ERROR, "load_pose: No #CLASS seen yet");
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
			log(LOG_ERROR, "load_pose: %s: Unknown keyword",
				 rfile_tok(fp));
			fread_to_eol(fp);
		}
	}
}
