/*-
 * Copyright (c) 2001 SoG Development Team
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
 * $Id: affect.c,v 1.65 2001-09-12 19:43:15 fjoe Exp $
 */

#include <stdio.h>
#include <stdlib.h>	/* atoi in w/a in aff_fread */
#include <string.h>	/* strncmp in w/a in aff_fread */

#include <merc.h>
#include <rwfile.h>

int affect_count;

AFFECT_DATA *
aff_new(int where, const char *sn)
{
	AFFECT_DATA *paf;

	affect_count++;
	paf = mem_alloc(MT_AFFECT, sizeof(*paf));
	paf->next = NULL;
	paf->where = where;
	paf->type = str_dup(sn);
	paf->level = 0;
	paf->duration = 0;
	switch (paf->where) {
	case TO_SKILLS:
	case TO_RACE:
	case TO_FORM:
		paf->location.s = str_empty;
		break;
	default:
		INT(paf->location) = 0;
		break;
	}
	paf->modifier = 0;
	paf->bitvector = 0;
	paf->owner = NULL;
	return paf;
}

AFFECT_DATA *
aff_dup(const AFFECT_DATA *paf)
{
	AFFECT_DATA *naf = aff_new(paf->where, paf->type);
	naf->level	= paf->level;
	naf->duration	= paf->duration;
	switch (paf->where) {
	case TO_SKILLS:
	case TO_RACE:
	case TO_FORM:
		naf->location.s = str_qdup(paf->location.s);
		break;
	default:
		INT(naf->location) = INT(paf->location);
		break;
	}
	naf->modifier	= paf->modifier;
	naf->bitvector	= paf->bitvector;
	naf->owner	= paf->owner;
	return naf;
}

void
aff_free(AFFECT_DATA *af)
{
	switch (af->where) {
	case TO_SKILLS:
	case TO_RACE:
	case TO_FORM:
		free_string(af->location.s);
		break;
	}
	free_string(af->type);
	mem_free(af);
	affect_count--;
}

AFFECT_DATA *
aff_dup_list(AFFECT_DATA *paf, int level)
{
	AFFECT_DATA *rv = NULL;
	AFFECT_DATA **ppaf = &rv;

	while (paf) {
		*ppaf = aff_dup(paf);
		if (level >= 0)
			(*ppaf)->level = level;
		ppaf = &(*ppaf)->next;
		paf = paf->next;
	}

	return rv;
}

void
aff_free_list(AFFECT_DATA *paf)
{
	AFFECT_DATA *paf_next;

	for (; paf != NULL; paf = paf_next) {
		paf_next = paf->next;
		aff_free(paf);
	}
}

void
saff_init(saff_t *sa)
{
	sa->sn = str_empty;
	sa->type = str_empty;
	sa->mod = 0;
	sa->bit = 0;
}

void
saff_destroy(saff_t *sa)
{
	free_string(sa->sn);
	free_string(sa->type);
}

where_t where_table[] =
{
	{ TO_AFFECTS,	apply_flags,	affect_flags,	"modifies {c%s{x by {c%d{x",		"adds '{c%s{x' affect"		},
	{ TO_FORMAFFECTS,apply_flags,	affect_flags,	"modifies {c%s{x by {c%d{x",		"adds '{c%s{x' affect"		},
	{ TO_SKILLS,	NULL,		sk_aff_flags,	str_empty,				str_empty			},
	{ TO_RACE,	NULL,		NULL,		"changes race to '{c%s{x'",		str_empty			},
	{ TO_DETECTS,	apply_flags,	id_flags,	"modifies {c%s{x by {c%d{x",		"adds '{c%s{x' detection"	},
	{ TO_INVIS,	apply_flags,	id_flags,	"modifies {c%s{x by {c%d{x",		"adds '{c%s{x'"			},
	{ TO_RESISTS,	dam_classes,	NULL,		"modifies {c%s resistance{x by {c%d{x",	str_empty			},
	{ TO_FORMRESISTS,dam_classes,	NULL,		"modifies {c%s resistance{x by {c%d{x",	str_empty			},
	{ TO_OBJECT,	apply_flags,	stat_flags,	"modifies {c%s{x by {c%d{x",		"adds '{c%s{x' affect"		},
	{ -1,		NULL,		NULL,		str_empty,				str_empty			}
};

where_t *
where_lookup(flag_t where)
{
	where_t *wd;

	for (wd = where_table; wd->where != -1; wd++)
		if (wd->where == where)
			return wd;
	return NULL;
}

void
aff_fwrite_list(const char *pre, const char *pre_notype, AFFECT_DATA *paf,
		FILE *fp, int w_flags)
{
	REMOVE_BIT(w_flags, AFF_X_NOTYPE);

	for (; paf != NULL; paf = paf->next) {
		int wf_notype = 0;

		/* skip empty affects */
		if (IS_APPLY_AFFECT(paf)
		&&  INT(paf->location) == APPLY_NONE
		&&  !paf->modifier
		&&  !paf->bitvector
		&&  IS_NULLSTR(paf->type))
			continue;

		if (IS_SKILL(paf->type, "doppelganger"))
			continue;

		wf_notype = IS_NULLSTR(paf->type) && !IS_NULLSTR(pre_notype) ?
		    AFF_X_NOTYPE : 0;
		fprintf(fp, "%s ", wf_notype ? pre_notype : pre);
		aff_fwrite(paf, fp, w_flags | wf_notype);
	}
}

void
aff_fwrite(AFFECT_DATA *paf, FILE *fp, int w_flags)
{
	if (!IS_SET(w_flags, AFF_X_NOTYPE))
		fprintf(fp, "'%s' ", paf->type);

	fprintf(fp, "%s ", flag_string(affect_where_types, paf->where));

	switch (paf->where) {
	case TO_SKILLS:
	case TO_RACE:
	case TO_FORM:
		fprintf(fp, "'%s' ", STR(paf->location));
		break;

	case TO_RESISTS:
	case TO_FORMRESISTS:
		fprintf(fp, "%s ",
			flag_string(dam_classes, INT(paf->location)));
		break;

	default:
		fprintf(fp, "%s ",
			flag_string(apply_flags, INT(paf->location)));
		break;
	}

	fprintf(fp, "%d %s",
		paf->modifier, format_flags(paf->bitvector));
	if (!IS_SET(w_flags, AFF_X_NOLD))
		fprintf(fp, " %d %d", paf->level, paf->duration);
	fprintf(fp, "\n");
}

AFFECT_DATA *
aff_fread(rfile_t *fp, int r_flags)
{
	AFFECT_DATA *paf = aff_new(TO_AFFECTS, str_empty);

	if (!IS_SET(r_flags, AFF_X_NOTYPE)) {
		paf->type = c_fread_strkey(
		    fp, &skills, "aff_fread");		// notrans
	}

	paf->where = fread_fword(affect_where_types, fp);
	if (paf->where < 0)
		paf->where = TO_AFFECTS;

	switch (paf->where) {
	case TO_SKILLS:
		paf->location.s = c_fread_strkey(
		    fp, &skills, "aff_fread");			// notrans
		break;
	case TO_RACE:
		paf->location.s = c_fread_strkey(
		    fp, &races, "aff_fread");			// notrans
		break;
	case TO_FORM:
		paf->location.s = c_fread_strkey(
		    fp, &forms, "aff_fread");			// notrans
		break;
	case TO_RESISTS:
	case TO_FORMRESISTS:
		INT(paf->location) = fread_fword(dam_classes, fp);
		break;
	default:
		INT(paf->location) = fread_fword(apply_flags, fp);
		break;
	}

	paf->modifier = fread_number(fp);
	paf->bitvector = fread_flags(fp);

	if (!IS_SET(r_flags, AFF_X_NOLD)) {
		paf->level = fread_number(fp);
		paf->duration = fread_number(fp);
	}

	return paf;
}

AFFECT_DATA *
aff_fread_v5(rfile_t *fp)
{
	const char *name;
	AFFECT_DATA *paf = aff_new(TO_AFFECTS, str_empty);

	paf->type = c_fread_strkey(fp, &skills, "aff_fread"); // notrans

	/*
	 * fixup for areas with 0 < ver <= 5
	 * this w/a can be removed (replaced with fread_fword)
	 * when there will be no areas with such version in translation
	 */
	fread_word(fp);
	name = rfile_tok(fp);

	if (is_number(name))
		paf->where = atoi(name);
	else {
		if (!strncmp(name, "to_", 3))			// notrans
			name += 3;
		paf->where = flag_svalue(affect_where_types, name);
	}

	if (paf->where < 0)
		paf->where = TO_AFFECTS;
	paf->level = fread_number(fp);
	paf->duration = fread_number(fp);
	paf->modifier = fread_number(fp);
	switch (paf->where) {
	case TO_SKILLS:
		paf->location.s = c_fread_strkey(
		    fp, &skills, "aff_fread");			// notrans
		break;
	case TO_RACE:
		paf->location.s = c_fread_strkey(
		    fp, &races, "aff_fread");			// notrans
		break;
	case TO_FORM:
		paf->location.s = c_fread_strkey(
		    fp, &forms, "aff_fread");			// notrans
		break;
	case TO_RESISTS:
	case TO_FORMRESISTS:
		INT(paf->location) = fread_fword(dam_classes, fp);
		break;
	default:
		INT(paf->location) = fread_fword(apply_flags, fp);
		break;
	}
	paf->bitvector = fread_flags(fp);

	return paf;
}
