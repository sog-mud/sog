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
 * $Id: affect.c,v 1.56 2001-07-29 23:39:26 fjoe Exp $
 */

#include <stdio.h>

#include <merc.h>
#include <rfile.h>
#include <db.h>

#include "affects.h"	/* XXX temporary w/a */

AFFECT_DATA *
aff_new(void)
{
	AFFECT_DATA *paf;

	top_affect++;
	paf = mem_alloc(MT_AFFECT, sizeof(*paf));
	memset(paf, 0, sizeof(*paf));
	return paf;
}

AFFECT_DATA *
aff_dup(const AFFECT_DATA *paf)
{
	AFFECT_DATA *naf = aff_new();
	naf->where	= paf->where;
	naf->type	= str_dup(paf->type);
	naf->level	= paf->level;
	naf->duration	= paf->duration;
	switch (paf->where) {
	case TO_RACE:
	case TO_SKILLS:
		naf->location.s = str_dup(paf->location.s);
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
	case TO_RACE:
	case TO_SKILLS:
		free_string(af->location.s);
		break;
	}
	free_string(af->type);
	mem_free(af);
	top_affect--;
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
	{ TO_RESIST,	dam_classes,	NULL,		"modifies {c%s resistance{x by {c%d{x",	str_empty			},
	{ TO_FORMRESIST,dam_classes,	NULL,		"modifies {c%s resistance{x by {c%d{x",	str_empty			},
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
aff_fwrite(AFFECT_DATA *paf, FILE *fp)
{
	switch (paf->where) {
	case TO_SKILLS:
	case TO_RACE:
		fprintf(fp, "'%s' %s %d %d %d '%s' %s\n",
			paf->type,
			flag_string(affect_where_types, paf->where),
			paf->level, paf->duration, paf->modifier,
			STR(paf->location), format_flags(paf->bitvector));
		break;
	default:
		fprintf(fp, "'%s' %s %d %d %d %d %s\n",
			paf->type,
			flag_string(affect_where_types, paf->where),
			paf->level, paf->duration, paf->modifier,
			INT(paf->location), format_flags(paf->bitvector));
		break;
	}
}

void
aff_fwrite_list(const char *pre, AFFECT_DATA *paf, FILE *fp)
{
	for (; paf != NULL; paf = paf->next) {
		/* skip empty affects */
		if ((paf->where == TO_AFFECTS || paf->where == TO_INVIS || paf->where == TO_DETECTS)
		&&  INT(paf->location) == APPLY_NONE
		&&  !paf->modifier
		&&  !paf->bitvector
		&&  IS_NULLSTR(paf->type))
			continue;

		if (IS_SKILL(paf->type, "doppelganger"))
			continue;

		fprintf(fp, "%s ", pre);
		aff_fwrite(paf, fp);
	}
}

AFFECT_DATA *
aff_fread(rfile_t *fp)
{
	AFFECT_DATA *paf = aff_new();

	paf->type = fread_strkey(fp, &skills, "aff_fread");	// notrans
	paf->where = fread_fword(affect_where_types, fp);
	if (paf->where < 0)
		paf->where = TO_AFFECTS;
	paf->level = fread_number(fp);
	paf->duration = fread_number(fp);
	paf->modifier = fread_number(fp);
	switch (paf->where) {
	case TO_SKILLS:
		paf->location.s = fread_strkey(
		    fp, &skills, "aff_fread");			// notrans
		break;
	case TO_RACE:
		paf->location.s = fread_strkey(
		    fp, &races, "aff_fread");			// notrans
		break;
	default:
		INT(paf->location) = fread_number(fp);
		break;
	}
	paf->bitvector = fread_flags(fp);

	return paf;
}

/* XXX temporary w/a */
void
affect_to_char2(CHAR_DATA *ch, AFFECT_DATA *paf)
{
	AFFECT_DATA *af = aff_dup(paf);
	affect_to_char(ch, af);
	aff_free(af);
}

void
affect_to_obj2(OBJ_DATA *obj, AFFECT_DATA *paf)
{
	AFFECT_DATA *af = aff_dup(paf);
	affect_to_obj(obj, af);
	aff_free(af);
}

void
affect_to_room2(ROOM_INDEX_DATA *room, AFFECT_DATA *paf)
{
	AFFECT_DATA *af = aff_dup(paf);
	affect_to_room(room, af);
	aff_free(af);
}
