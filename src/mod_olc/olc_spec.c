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
 * $Id: olc_spec.c,v 1.15 2001-08-26 05:49:14 fjoe Exp $
 */

#include "olc.h"

#define EDIT_SPEC(ch, spec)	(spec = (spec_t *) ch->desc->pEdit)

DECLARE_OLC_FUN(speced_create		);
DECLARE_OLC_FUN(speced_edit		);
DECLARE_OLC_FUN(speced_save		);
DECLARE_OLC_FUN(speced_touch		);
DECLARE_OLC_FUN(speced_show		);
DECLARE_OLC_FUN(speced_list		);

DECLARE_OLC_FUN(speced_class		);
DECLARE_OLC_FUN(speced_skill		);
DECLARE_OLC_FUN(speced_trigger		);
DECLARE_OLC_FUN(speced_flags		);

DECLARE_OLC_FUN(olc_skill_update	);

olced_strkey_t strkey_specs = { &specs, SPEC_PATH, SPEC_EXT };

olc_cmd_t olc_cmds_spec[] =
{
	{ "create",	speced_create,	NULL,		NULL		},
	{ "edit",	speced_edit,	NULL,		NULL		},
	{ "",		speced_save,	NULL,		NULL		},
	{ "touch",	speced_touch,	NULL,		NULL		},
	{ "show",	speced_show,	NULL,		NULL		},
	{ "list",	speced_list,	NULL,		NULL		},

	{ "name",	olced_strkey,	NULL,		&strkey_specs	},
	{ "class",	speced_class,	NULL,		spec_classes	},
	{ "skill",	speced_skill,	NULL,		&skills		},
	{ "trigger",	speced_trigger,	NULL,		NULL		},
	{ "flags",	speced_flags,	NULL,		spec_flags	},

	{ "update",	olc_skill_update, NULL,		NULL		},

	{ "commands",	show_commands,	NULL,		NULL		},
	{ "version",	show_version,	NULL,		NULL		},

	{ NULL, NULL, NULL, NULL }
};

static void *save_spec_cb(void *p, va_list ap);
static void *show_spec_skill_cb(void *p, va_list ap);

OLC_FUN(speced_create)
{
	spec_t *s;
	spec_t spec;
	char arg[MAX_INPUT_LENGTH];

	if (PC(ch)->security < SECURITY_SPEC) {
		act_char("SpecEd: Insufficient security for creating specs.", ch);
		return FALSE;
	}

	first_arg(argument, arg, sizeof(arg), FALSE);
	if (arg[0] == '\0')
		OLC_ERROR("'OLC CREATE'");

	/*
	 * olced_busy check is not needed since hash_insert
	 * adds new elements to the end of varr
	 */

	spec_init(&spec);
	spec.spec_name = str_dup(arg);
	s = hash_insert(&specs, spec.spec_name, &spec);
	spec_destroy(&spec);

	if (s == NULL) {
		act_puts("SpecEd: $t: already exists.",
			 ch, arg, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	ch->desc->pEdit	= s;
	OLCED(ch) = olced_lookup(ED_SPEC);
	touch_spec(s);
	act_char("Spec created.", ch);
	return FALSE;
}

OLC_FUN(speced_edit)
{
	spec_t *s;

	if (PC(ch)->security < SECURITY_SPEC) {
		act_char("SpecEd: Insufficient security.", ch);
		return FALSE;
	}

	if (argument[0] == '\0')
		OLC_ERROR("'OLC EDIT'");

	if ((s = spec_search(argument)) == 0) {
		act_puts("SpecEd: $t: No such spec.",
			 ch, argument, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	ch->desc->pEdit	= s;
	OLCED(ch)	= olced_lookup(ED_SPEC);
	return FALSE;
}

OLC_FUN(speced_save)
{
	bool found = FALSE;

	olc_printf(ch, "Saved specs:");
	hash_foreach(&specs, save_spec_cb, ch, &found);
	if (!found)
		olc_printf(ch, "    None.");
	return FALSE;
}

OLC_FUN(speced_touch)
{
	spec_t *spec;
	EDIT_SPEC(ch, spec);
	return touch_spec(spec);
}

OLC_FUN(speced_show)
{
	BUFFER *output;
	spec_t *s;

	if (argument[0] == '\0') {
		if (IS_EDIT(ch, ED_SPEC))
			EDIT_SPEC(ch, s);
		else {
			OLC_ERROR("'OLC ASHOW'");
			return FALSE;
		}
	} else {
		if ((s = spec_search(argument)) == NULL) {
			act_puts("SpecEd: $t: No such spec.",
				 ch, argument, NULL,
				 TO_CHAR | ACT_NOTRANS, POS_DEAD);
			return FALSE;
		}
	}

	output = buf_new(0);
	buf_printf(output, BUF_END, "Name:          [%s]\n", s->spec_name);
	buf_printf(output, BUF_END, "Class:         [%s]   Flags: [%s]\n",
			flag_string(spec_classes, s->spec_class),
			flag_string(spec_flags, s->spec_flags));
	if (s->mp_trig.trig_type != TRIG_NONE) {
		buf_printf(output, BUF_END, "Trigger:       [%s]\n",
			   s->mp_trig.trig_prog);
	}

	if (s->spec_skills.nused == 0)
		buf_printf(output, BUF_END, "No skills defined for this spec.\n");
	else {
		buf_printf(output, BUF_END, "Num   Skill name          Level  Rate    Min    Adept   Max\n");
		buf_printf(output, BUF_END, "--------------------------------------------------------------\n");
		varr_foreach(&s->spec_skills, show_spec_skill_cb, output, &s->spec_skills);
	}

	page_to_char(buf_string(output), ch);
	buf_free(output);

	return FALSE;
}

OLC_FUN(speced_list)
{
	BUFFER	*buffer = buf_new(0);
	strkey_printall(&specs, buffer);
	page_to_char(buf_string(buffer), ch);
	buf_free(buffer);
	return FALSE;
}

OLC_FUN(speced_class)
{
	spec_t *s;
	EDIT_SPEC(ch, s);
	return olced_flag(ch, argument, cmd, &s->spec_class);
}

OLC_FUN(speced_skill)
{
	spec_t *s;
	spec_skill_t *ssk;
	bool ok, del = FALSE;
	char arg[MAX_INPUT_LENGTH];
	int num;

	EDIT_SPEC(ch, s);
	if (argument[0] == '\0')
		OLC_ERROR("'OLC SPEC SKILL'");
	argument = one_argument(argument, arg, sizeof(arg));

	if (!str_prefix(arg, "add")) {
		if ((ssk = spec_skill_lookup(s, argument)) != NULL) {
			act_puts("SpecEd: add skill: $t: already there.",
				 ch, ssk->sn, NULL,
				 TO_CHAR | ACT_NOTRANS, POS_DEAD);
			return FALSE;
		}
		ssk = varr_enew(&s->spec_skills);
		ok = olced_foreign_mlstrkey(ch, argument, cmd, &ssk->sn);
		varr_qsort(&s->spec_skills, cmpstr);
		return ok;
	}

	if (!str_prefix(arg, "delete"))
		del = TRUE;

	if (!del && str_prefix(arg, "edit"))
		OLC_ERROR("'OLC SPEC SKILL'");

	argument = one_argument(argument, arg, sizeof(arg));
	if (!is_number(arg))
		OLC_ERROR("'OLC SPEC SKILL'");

	num = atoi(arg);
	ssk = varr_get(&s->spec_skills, num);

	if (!ssk) {
		act_puts("SpecEd: $j: no skill with such number.",
			 ch, (const void *) num, NULL, TO_CHAR, POS_DEAD);
		return FALSE;
	}

	if (del) {
		varr_edelete(&s->spec_skills, ssk);
		act_char("SpecEd: skill deleted.", ch);
		return TRUE;
	}

	argument = one_argument(argument, arg, sizeof(arg));

	if (!str_prefix(arg, "min"))
		return olced_number(ch, argument, cmd, &ssk->min);
	if (!str_prefix(arg, "adept"))
		return olced_number(ch, argument, cmd, &ssk->adept);
	if (!str_prefix(arg, "max"))
		return olced_number(ch, argument, cmd, &ssk->max);
	if (!str_prefix(arg, "level"))
		return olced_number(ch, argument, cmd, &ssk->level);
	if (!str_prefix(arg, "rating"))
		return olced_number(ch, argument, cmd, &ssk->rating);

	act_puts("SpecEd: no such field '$t'.",
		 ch, arg, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
	return FALSE;
}

OLC_FUN(speced_trigger)
{
	spec_t *s;
	EDIT_SPEC(ch, s);
#if 0
	XXX MPC
	return olced_cc_vexpr(ch, argument, cmd, &s->spec_deps, "spec");
#endif
	return FALSE;
}

OLC_FUN(speced_flags)
{
	spec_t *s;
	EDIT_SPEC(ch, s);
	return olced_flag(ch, argument, cmd, &s->spec_flags);
}

/* --------------------------------------------------------------------- */
bool touch_spec(spec_t *spec)
{
	SET_BIT(spec->spec_flags, SPF_CHANGED);
	return FALSE;
}

static void *
save_spec_skill_cb(void *p, va_list ap)
{
	spec_skill_t *ssk = (spec_skill_t *) p;

	FILE *fp = va_arg(ap, FILE *);

	if (!IS_NULLSTR(ssk->sn)) {
		fprintf(fp, "\n#SKILL\n");
		fprintf(fp, "Skill '%s'\n", ssk->sn);
		fprintf(fp, "Level %d\n", ssk->level);
		fprintf(fp, "Rating %d\n", ssk->rating);
		fprintf(fp, "Min %d\n", ssk->min);
		fprintf(fp, "Adept %d\n", ssk->adept);
		fprintf(fp, "Max %d\n", ssk->max);
		fprintf(fp, "End\n");
	}
	return NULL;
}

static void *
save_spec_cb(void *p, va_list ap)
{
	spec_t *s = (spec_t *) p;

	CHAR_DATA *ch = va_arg(ap, CHAR_DATA *);
	bool *pfound = va_arg(ap, bool *);

	FILE *fp;
	const char *filename;

	if (!IS_SET(s->spec_flags, SPF_CHANGED))
		return NULL;

	filename = strkey_filename(s->spec_name, SPEC_EXT);
	if ((fp = olc_fopen(SPEC_PATH, filename, ch, SECURITY_SPEC)) == NULL)
		return NULL;

	REMOVE_BIT(s->spec_flags, SPF_CHANGED);

	fprintf(fp, "#SPEC\n");
	fwrite_string(fp, "Name", s->spec_name);
	fprintf(fp, "Class %s\n", flag_string(spec_classes, s->spec_class));
	fwrite_string(fp, "Trigger", s->mp_trig.trig_prog);
	if (s->spec_flags) {
		fprintf(fp, "Flags %s~\n",
				flag_string(spec_flags, s->spec_flags));
	}
	fprintf(fp, "End\n");

	varr_foreach(&s->spec_skills, save_spec_skill_cb, fp);
	fprintf(fp, "\n#$\n");
	fclose(fp);

	olc_printf(ch, "    %s (%s)", s->spec_name, filename);
	*pfound = TRUE;
	return NULL;
}

static void *
show_spec_skill_cb(void *p, va_list ap)
{
	spec_skill_t *ssk = (spec_skill_t *) p;

	BUFFER *output = va_arg(ap, BUFFER *);
	varr *v = va_arg(ap, varr *);

	if (!IS_NULLSTR(ssk->sn))
		buf_printf(output, BUF_END, "(%3d) %-18.17s  [%3d]  [%3d]  [%3d%%]  [%3d%%]  [%3d%%]\n",
			varr_index(v, p),
			ssk->sn, ssk->level, ssk->rating,
			ssk->min, ssk->adept, ssk->max);
	return NULL;
}
