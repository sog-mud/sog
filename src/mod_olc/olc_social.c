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
 * $Id: olc_social.c,v 1.36 2001-09-13 17:54:14 fjoe Exp $
 */

/* I never wanted to be
 * What they told me to be
 * Fulfill my fate and I'll be free
 * God knows how long
 * I tried to change fate...
 */

#include "olc.h"

#define EDIT_SOC(ch, soc)	(soc = (social_t *) ch->desc->pEdit)

DECLARE_OLC_FUN(soced_create		);
DECLARE_OLC_FUN(soced_edit		);
DECLARE_OLC_FUN(soced_save		);
DECLARE_OLC_FUN(soced_touch		);
DECLARE_OLC_FUN(soced_show		);
DECLARE_OLC_FUN(soced_list		);

DECLARE_OLC_FUN(soced_name		);
DECLARE_OLC_FUN(soced_minpos		);
DECLARE_OLC_FUN(soced_found_char	);
DECLARE_OLC_FUN(soced_found_vict	);
DECLARE_OLC_FUN(soced_found_notvict	);
DECLARE_OLC_FUN(soced_noarg_char	);
DECLARE_OLC_FUN(soced_noarg_room	);
DECLARE_OLC_FUN(soced_self_char		);
DECLARE_OLC_FUN(soced_self_room		);
DECLARE_OLC_FUN(soced_notfound_char	);
DECLARE_OLC_FUN(soced_delete		);
DECLARE_OLC_FUN(soced_move		);

static DECLARE_VALIDATE_FUN(validate_soc_name);

olc_cmd_t olc_cmds_soc[] =
{
	{ "create",	soced_create,		NULL,	NULL		},
	{ "edit",	soced_edit,		NULL,	NULL		},
	{ "",		soced_save,		NULL,	NULL		},
	{ "touch",	soced_touch,		NULL,	NULL		},
	{ "show",	soced_show,		NULL,	NULL		},
	{ "list",	soced_list,		NULL,	NULL		},

	{ "name",	soced_name,		validate_soc_name, NULL	},
	{ "minpos",	soced_minpos,		NULL,	position_table	},

	{ "found_char",	soced_found_char,	NULL,	NULL		},
	{ "found_vict",	soced_found_vict,	NULL,	NULL		},
	{ "found_notvict", soced_found_notvict,	NULL,	NULL		},

	{ "noarg_char",	soced_noarg_char,	NULL,	NULL		},
	{ "noarg_room",	soced_noarg_room,	NULL,	NULL		},

	{ "self_char",	soced_self_char,	NULL,	NULL		},
	{ "self_room",	soced_self_room,	NULL,	NULL		},

	{ "notfound_char", soced_notfound_char, NULL,	NULL		},

	{ "move",	soced_move,		NULL,	NULL		},
	{ "delete_so",	olced_spell_out,	NULL,	NULL		},
	{ "delete_soc",	soced_delete,		NULL,	NULL		},
	{ "commands",	show_commands,		NULL,	NULL		},
	{ "version",	show_version,		NULL,	NULL		},

	{ NULL, NULL, NULL, NULL }
};

static void *save_social_cb(void *fp, va_list ap);
static void shadow_dump_cmds(BUFFER *output, const char *name);
static void shadow_print_cmds(CHAR_DATA *ch, const char *name);

#define check_shadow(name)	!!(cmd_search(name))

OLC_FUN(soced_create)
{
	social_t *soc;
	char arg[MAX_STRING_LENGTH];

	if (PC(ch)->security < SECURITY_SOCIALS) {
		act_char("SocEd: Insufficient security for creating socials", ch);
		return FALSE;
	}

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0')
		OLC_ERROR("'OLC CREATE'");

	if ((soc = social_lookup(arg))) {
		act_puts("SocEd: $t: already exists.",
			 ch, soc->name, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	shadow_print_cmds(ch, arg);
	soc		= varr_enew(&socials);
	soc->name	= str_dup(arg);

	ch->desc->pEdit	= (void *) soc;
	OLCED(ch)	= olced_lookup(ED_SOCIAL);
	SET_BIT(changed_flags, CF_SOCIAL);
	act_char("Social created.", ch);
	return FALSE;
}

OLC_FUN(soced_edit)
{
	social_t *soc;
	char arg[MAX_STRING_LENGTH];

	if (PC(ch)->security < SECURITY_SOCIALS) {
		act_char("SocEd: Insufficient security.", ch);
		return FALSE;
	}

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0')
		OLC_ERROR("'OLC EDIT'");

	if ((soc = social_search(arg)) == NULL) {
		act_puts("SocEd: $t: No such social.",
			 ch, arg, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	ch->desc->pEdit	= soc;
	OLCED(ch)	= olced_lookup(ED_SOCIAL);
	return FALSE;
}

OLC_FUN(soced_save)
{
	FILE *fp;

	if (!IS_SET(changed_flags, CF_SOCIAL)) {
		olc_printf(ch, "Socials are not changed.");
		return FALSE;
	}

	fp = olc_fopen(ETC_PATH, SOCIALS_CONF, ch, SECURITY_SOCIALS);
	if (fp == NULL)
		return FALSE;

	c_foreach(&socials, save_social_cb, fp);

	fprintf(fp, "#$\n");
	fclose(fp);

	REMOVE_BIT(changed_flags, CF_SOCIAL);
	olc_printf(ch, "Socials saved.");
	return FALSE;
}

OLC_FUN(soced_touch)
{
	SET_BIT(changed_flags, CF_SOCIAL);
	return FALSE;
}

#define SOC_SHOW(preword, mlp)					\
	if (!mlstr_null(mlp))					\
		mlstr_dump(output, preword, mlp, DUMP_LEVEL(ch));

OLC_FUN(soced_show)
{
	char arg[MAX_STRING_LENGTH];
	BUFFER *output;
	social_t *soc;

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		if (IS_EDIT(ch, ED_SOCIAL))
			EDIT_SOC(ch, soc);
		else
			OLC_ERROR("'OLC ASHOW'");
	} else {
		if ((soc = social_search(arg)) == NULL) {
			act_puts("SocEd: $t: No such social.",
				 ch, arg, NULL,
				 TO_CHAR | ACT_NOTRANS, POS_DEAD);
			return FALSE;
		}
	}

	output = buf_new(0);

	buf_printf(output, BUF_END,
		   "[%3d] Name:    [%s]\n",
		   varr_index(&socials, soc), soc->name);
	buf_printf(output, BUF_END,
		   "Min pos:       [%s]\n",
		   flag_string(position_table, soc->min_pos));

	SOC_SHOW("Found char   ", &soc->found_char);
	SOC_SHOW("Found vict   ", &soc->found_vict);
	SOC_SHOW("Found other  ", &soc->found_notvict);
	SOC_SHOW("Noarg char   ", &soc->noarg_char);
	SOC_SHOW("Noarg room   ", &soc->noarg_room);
	SOC_SHOW("Self char    ", &soc->self_char);
	SOC_SHOW("Self room    ", &soc->self_room);
	SOC_SHOW("Not found    ", &soc->notfound_char);
	shadow_dump_cmds(output, soc->name);

	page_to_char(buf_string(output), ch);
	buf_free(output);

	return FALSE;
}

OLC_FUN(soced_list)
{
	size_t i;
	int col = 0;
	char arg[MAX_STRING_LENGTH];
	BUFFER *output;

	one_argument(argument, arg, sizeof(arg));
	output = buf_new(0);

	for (i = 0; i < c_size(&socials); i++) {
		social_t *soc = (social_t*) VARR_GET(&socials, i);

		if (arg[0] && str_prefix(arg, soc->name))
			continue;

		buf_printf(output, BUF_END, "[%1s%3d] %-12s",
			check_shadow(soc->name) ? "*" : " ", i,	soc->name);
		if (++col % 4 == 0)
			buf_append(output, "\n");
	}

	if (col % 4)
		buf_append(output, "\n");

	page_to_char(buf_string(output), ch);
	buf_free(output);

	return FALSE;
}

OLC_FUN(soced_name)
{
	social_t *soc;
	EDIT_SOC(ch, soc);

	shadow_print_cmds(ch, argument);
	return olced_str(ch, argument, cmd, &soc->name);
}

OLC_FUN(soced_minpos)
{
	social_t *soc;
	EDIT_SOC(ch, soc);
	return olced_flag(ch, argument, cmd, &soc->min_pos);
}

OLC_FUN(soced_found_char)
{
	social_t *soc;
	EDIT_SOC(ch, soc);
	return olced_mlstr(ch, argument, cmd, &soc->found_char);
}

OLC_FUN(soced_found_vict)
{
	social_t *soc;
	EDIT_SOC(ch, soc);
	return olced_mlstr(ch, argument, cmd, &soc->found_vict);
}

OLC_FUN(soced_found_notvict)
{
	social_t *soc;
	EDIT_SOC(ch, soc);
	return olced_mlstr(ch, argument, cmd, &soc->found_notvict);
}

OLC_FUN(soced_noarg_char)
{
	social_t *soc;
	EDIT_SOC(ch, soc);
	return olced_mlstr(ch, argument, cmd, &soc->noarg_char);
}

OLC_FUN(soced_noarg_room)
{
	social_t *soc;
	EDIT_SOC(ch, soc);
	return olced_mlstr(ch, argument, cmd, &soc->noarg_room);
}

OLC_FUN(soced_self_char)
{
	social_t *soc;
	EDIT_SOC(ch, soc);
	return olced_mlstr(ch, argument, cmd, &soc->self_char);
}

OLC_FUN(soced_self_room)
{
	social_t *soc;
	EDIT_SOC(ch, soc);
	return olced_mlstr(ch, argument, cmd, &soc->self_room);
}

OLC_FUN(soced_notfound_char)
{
	social_t *soc;
	EDIT_SOC(ch, soc);
	return olced_mlstr(ch, argument, cmd, &soc->notfound_char);
}

OLC_FUN(soced_move)
{
	social_t *soc, nsoc;
	char arg[MAX_INPUT_LENGTH];
	int num, num2;

	EDIT_SOC(ch, soc);

	if (olced_busy(ch, ED_SOCIAL, NULL, NULL))
		return FALSE;

	one_argument(argument, arg, sizeof(arg));
	if (!is_number(arg))
		OLC_ERROR("'OLC SOCIALS'");

	if ((num = atoi(arg)) >= (int) c_size(&socials))
		num = c_size(&socials) - 1;
	if (num < 0) {
		act_char("SocEd: move: num should be >= 0.", ch);
		return FALSE;
	}

	num2 = varr_index(&socials, soc);
	if (num == num2) {
		act_char("SocEd: move: already there", ch);
		return FALSE;
	}

	nsoc = *soc;
	varr_edelete_nd(&socials, soc);
	soc = (social_t *) varr_insert(&socials, num);
	*soc = nsoc;

	ch->desc->pEdit	= soc;
	act_puts("SocEd: '$T' moved to $j position.",
		 ch, (const void *) varr_index(&socials, soc), soc->name,
		 TO_CHAR | ACT_NOTRANS, POS_DEAD);
	return TRUE;
}

OLC_FUN(soced_delete)
{
	social_t *soc;
	EDIT_SOC(ch, soc);

	if (olced_busy(ch, ED_SOCIAL, NULL, NULL))
		return FALSE;

	varr_edelete(&socials, soc);
	edit_done(ch->desc);
	return TRUE;
}

static VALIDATE_FUN(validate_soc_name)
{
	const char *name = (const char*) arg;
	social_t *soc, *soc2;
	EDIT_SOC(ch, soc);

	if (strpbrk(name, " \t")) {
		act_puts("SocEd: $t: illegal character in social name.",
			 ch, name, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	if ((soc2 = social_lookup(name))
	&&  soc2 != soc) {
		act_puts("SocEd: $t: duplicate social name.",
			 ch, name, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	return TRUE;
}

static void *save_social_cb(void *p, va_list ap)
{
	social_t *soc = (social_t *) p;
	FILE *fp = va_arg(ap, FILE *);

	fprintf(fp, "#SOCIAL\n");
	fprintf(fp, "name %s\n", soc->name);
	fprintf(fp, "min_pos %s\n",
		flag_string(position_table, soc->min_pos));
	mlstr_fwrite(fp, "found_char", &soc->found_char);
	mlstr_fwrite(fp, "found_vict", &soc->found_vict);
	mlstr_fwrite(fp, "found_notvict", &soc->found_notvict);
	mlstr_fwrite(fp, "noarg_char", &soc->noarg_char);
	mlstr_fwrite(fp, "noarg_room", &soc->noarg_room);
	mlstr_fwrite(fp, "self_char", &soc->self_char);
	mlstr_fwrite(fp, "self_room", &soc->self_room);
	mlstr_fwrite(fp, "notfound_char", &soc->notfound_char);
	fprintf(fp, "end\n\n");

	return NULL;
}

static void *
shadow_dump_cb(void *p, va_list ap)
{
	cmd_t *cmd = (cmd_t *) p;
	const char *name = va_arg(ap, const char *);
	BUFFER *output = va_arg(ap, BUFFER *);

	if (!str_prefix(name, cmd->name))
		buf_printf(output, BUF_END, "   [%s]\n", cmd->name);

	return NULL;
}

static void
shadow_dump_cmds(BUFFER *output, const char *name)
{
	if (!check_shadow(name))
		return;

	buf_append(output, "[*** SHADOWED BY FOLLOWING COMMANDS ***]\n");
	c_foreach(&commands, shadow_dump_cb, name, output);
}

static void
shadow_print_cmds(CHAR_DATA *ch, const char *name)
{
	BUFFER *output = buf_new(0);
	shadow_dump_cmds(output, name);
	page_to_char(buf_string(output), ch);
	buf_free(output);
}

