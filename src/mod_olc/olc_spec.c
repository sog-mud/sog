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
 * $Id: olc_spec.c,v 1.23 2003-05-14 20:05:00 fjoe Exp $
 */

#include "olc.h"
#include <mprog.h>

#define EDIT_SPEC(ch, spec)	(spec = (spec_t *) ch->desc->pEdit)

DECLARE_OLC_FUN(speced_create		);
DECLARE_OLC_FUN(speced_edit		);
DECLARE_OLC_FUN(speced_save		);
DECLARE_OLC_FUN(speced_touch		);
DECLARE_OLC_FUN(speced_show		);
DECLARE_OLC_FUN(speced_list		);

DECLARE_OLC_FUN(speced_class		);
DECLARE_OLC_FUN(speced_skill		);
DECLARE_OLC_FUN(speced_check		);
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
	{ "check",	speced_check,	NULL,		NULL		},
	{ "flags",	speced_flags,	NULL,		spec_flags	},

	{ "update",	olc_skill_update, NULL,		NULL		},

	{ "commands",	show_commands,	NULL,		NULL		},
	{ "version",	show_version,	NULL,		NULL		},

	{ NULL, NULL, NULL, NULL }
};

static bool touch_spec(spec_t *spec);

OLC_FUN(speced_create)
{
	spec_t *spec;
	char arg[MAX_INPUT_LENGTH];

	if (PC(ch)->security < SECURITY_SPEC) {
		act_char("SpecEd: Insufficient security for creating specs.", ch);
		return FALSE;
	}

	first_arg(argument, arg, sizeof(arg), FALSE);
	if (arg[0] == '\0')
		OLC_ERROR("'OLC CREATE'");

	if ((spec = c_insert(&specs, arg)) == NULL) {
		act_puts("SpecEd: $t: already exists.",
			 ch, arg, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	spec->spec_name = str_dup(arg);
	ch->desc->pEdit	= spec;
	OLCED(ch) = olced_lookup(ED_SPEC);
	touch_spec(spec);
	act_char("Spec created.", ch);
	return FALSE;
}

OLC_FUN(speced_edit)
{
	spec_t *spec;

	if (PC(ch)->security < SECURITY_SPEC) {
		act_char("SpecEd: Insufficient security.", ch);
		return FALSE;
	}

	if (argument[0] == '\0')
		OLC_ERROR("'OLC EDIT'");

	if ((spec = spec_search(argument)) == 0) {
		act_puts("SpecEd: $t: No such spec.",
			 ch, argument, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	ch->desc->pEdit	= spec;
	OLCED(ch)	= olced_lookup(ED_SPEC);
	return FALSE;
}

OLC_FUN(speced_save)
{
	spec_t *spec;
	bool found = FALSE;

	olc_printf(ch, "Saved specs:");

	C_FOREACH(spec, &specs) {
		FILE *fp;
		const char *filename;
		spec_skill_t *spec_sk;

		if (!IS_SET(spec->spec_flags, SPF_CHANGED))
			continue;

		filename = strkey_filename(spec->spec_name, SPEC_EXT);
		if ((fp = olc_fopen(SPEC_PATH, filename, ch, SECURITY_SPEC)) == NULL)
			continue;

		REMOVE_BIT(spec->spec_flags, SPF_CHANGED);

		fprintf(fp, "#SPEC\n");
		fwrite_string(fp, "Name", spec->spec_name);
		fprintf(fp, "Class %s\n",
			flag_string(spec_classes, spec->spec_class));
		if (spec->mp_trig.trig_type != TRIG_NONE) {
			mprog_t *mp = mprog_lookup(spec->mp_trig.trig_prog);

			if (mp == NULL) {
				olc_printf(ch,
				    "%s: %s: mprog %s: no such mprog",
				    __FUNCTION__, spec->spec_name,
				    spec->mp_trig.trig_prog);
			} else
				fwrite_string(fp, "Check\n", mp->text);
		}

		if (spec->spec_flags) {
			fprintf(fp, "Flags %s~\n",
				flag_string(spec_flags, spec->spec_flags));
		}
		fprintf(fp, "End\n");

		C_FOREACH(spec_sk, &spec->spec_skills) {
			if (IS_NULLSTR(spec_sk->sn))
				continue;

			fprintf(fp, "\n#SKILL\n");
			fprintf(fp, "Skill '%s'\n", spec_sk->sn);
			fprintf(fp, "Level %d\n", spec_sk->level);
			fprintf(fp, "Rating %d\n", spec_sk->rating);
			fprintf(fp, "Min %d\n", spec_sk->min);
			fprintf(fp, "Adept %d\n", spec_sk->adept);
			fprintf(fp, "Max %d\n", spec_sk->max);
			fprintf(fp, "End\n");
		}

		fprintf(fp, "\n#$\n");
		fclose(fp);

		olc_printf(ch, "    %s (%s)", spec->spec_name, filename);
		found = TRUE;
	}

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
	spec_t *spec;

	if (argument[0] == '\0') {
		if (IS_EDIT(ch, ED_SPEC))
			EDIT_SPEC(ch, spec);
		else {
			OLC_ERROR("'OLC ASHOW'");
			return FALSE;
		}
	} else {
		if ((spec = spec_search(argument)) == NULL) {
			act_puts("SpecEd: $t: No such spec.",
				 ch, argument, NULL,
				 TO_CHAR | ACT_NOTRANS, POS_DEAD);
			return FALSE;
		}
	}

	output = buf_new(0);
	buf_printf(output, BUF_END, "Name:          [%s]\n", spec->spec_name);
	buf_printf(output, BUF_END, "Class:         [%s]   Flags: [%s]\n",
			flag_string(spec_classes, spec->spec_class),
			flag_string(spec_flags, spec->spec_flags));
	if (spec->mp_trig.trig_type != TRIG_NONE) {
		buf_printf(output, BUF_END, "Check:         [%s]\n",
			   spec->mp_trig.trig_prog);
	}

	if (c_isempty(&spec->spec_skills))
		buf_printf(output, BUF_END, "No skills defined for this spec.\n");
	else {
		spec_skill_t *spec_sk;

		buf_printf(output, BUF_END, "Num   Skill name          Level  Rate    Min    Adept   Max\n");
		buf_printf(output, BUF_END, "--------------------------------------------------------------\n");

		C_FOREACH(spec_sk, &spec->spec_skills) {
			if (IS_NULLSTR(spec_sk->sn))
				continue;

			buf_printf(output, BUF_END,
			    "(%3d) %-18.17s  [%3d]  [%3d]  [%3d%%]  [%3d%%]  [%3d%%]\n",
			    varr_index(&spec->spec_skills, spec_sk),
			    spec_sk->sn, spec_sk->level, spec_sk->rating,
			    spec_sk->min, spec_sk->adept, spec_sk->max);
		}
	}

	page_to_char(buf_string(output), ch);
	buf_free(output);

	return FALSE;
}

OLC_FUN(speced_list)
{
	BUFFER	*buffer = buf_new(0);
	c_strkey_dump(&specs, buffer);
	page_to_char(buf_string(buffer), ch);
	buf_free(buffer);
	return FALSE;
}

OLC_FUN(speced_class)
{
	spec_t *spec;
	EDIT_SPEC(ch, spec);
	return olced_flag(ch, argument, cmd, &spec->spec_class);
}

OLC_FUN(speced_skill)
{
	spec_t *spec;
	spec_skill_t *spec_sk;
	bool ok, del = FALSE;
	char arg[MAX_INPUT_LENGTH];
	int num;

	EDIT_SPEC(ch, spec);
	if (argument[0] == '\0')
		OLC_ERROR("'OLC SPEC SKILL'");
	argument = one_argument(argument, arg, sizeof(arg));

	if (!str_prefix(arg, "add")) {
		if ((spec_sk = spec_skill_lookup(spec, argument)) != NULL) {
			act_puts("SpecEd: add skill: $t: already there.",
				 ch, spec_sk->sn, NULL,
				 TO_CHAR | ACT_NOTRANS, POS_DEAD);
			return FALSE;
		}
		spec_sk = varr_enew(&spec->spec_skills);
		ok = olced_foreign_mlstrkey(ch, argument, cmd, &spec_sk->sn);
		varr_qsort(&spec->spec_skills, cmpstr);
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
	spec_sk = varr_get(&spec->spec_skills, num);

	if (!spec_sk) {
		act_puts("SpecEd: $j: no skill with such number.",
			 ch, (const void *) num, NULL, TO_CHAR, POS_DEAD);
		return FALSE;
	}

	if (del) {
		varr_edelete(&spec->spec_skills, spec_sk);
		act_char("SpecEd: skill deleted.", ch);
		return TRUE;
	}

	argument = one_argument(argument, arg, sizeof(arg));

	if (!str_prefix(arg, "min"))
		return olced_number(ch, argument, cmd, &spec_sk->min);
	if (!str_prefix(arg, "adept"))
		return olced_number(ch, argument, cmd, &spec_sk->adept);
	if (!str_prefix(arg, "max"))
		return olced_number(ch, argument, cmd, &spec_sk->max);
	if (!str_prefix(arg, "level"))
		return olced_number(ch, argument, cmd, &spec_sk->level);
	if (!str_prefix(arg, "rating"))
		return olced_number(ch, argument, cmd, &spec_sk->rating);

	act_puts("SpecEd: no such field '$t'.",
		 ch, arg, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
	return FALSE;
}

OLC_FUN(speced_check)
{
	char arg[MAX_INPUT_LENGTH];
	int rv;

	spec_t *spec;
	EDIT_SPEC(ch, spec);

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		dofun("help", ch, "'OLC SPEC CHECK'");
		return FALSE;
	}

	if (!str_cmp(arg, "delete")) {
		olced_remove_one_trig(ch, &spec->mp_trig);
		trig_destroy(&spec->mp_trig);
		trig_init(&spec->mp_trig);
		act_char("Ok.", ch);
		return TRUE;
	}

	if ((rv = olced_one_trig(ch, arg, argument, cmd, &spec->mp_trig)) < 0)
		return cmd->olc_fun(ch, "", cmd);
	return rv;
}

OLC_FUN(speced_flags)
{
	spec_t *spec;
	EDIT_SPEC(ch, spec);
	return olced_flag(ch, argument, cmd, &spec->spec_flags);
}

/*--------------------------------------------------------------------
 * static functions
 */

static bool
touch_spec(spec_t *spec)
{
	SET_BIT(spec->spec_flags, SPF_CHANGED);
	return FALSE;
}
