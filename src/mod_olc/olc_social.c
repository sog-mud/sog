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
 * $Id: olc_social.c,v 1.19 1999-12-19 08:10:30 avn Exp $
 */

#include "olc.h"
#include "socials.h"

#define EDIT_SOC(ch, soc)	(soc = (social_t*) ch->desc->pEdit)

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
	{ "create",	soced_create					},
	{ "edit",	soced_edit					},
	{ "",		soced_save					},
	{ "touch",	soced_touch					},
	{ "show",	soced_show					},
	{ "list",	soced_list					},

	{ "name",	soced_name,	validate_soc_name 		},
	{ "minpos",	soced_minpos,	NULL,		position_table	},

	{ "found_char",	soced_found_char				},
	{ "found_vict",	soced_found_vict				},
	{ "found_notvict",soced_found_notvict				},

	{ "noarg_char",	soced_noarg_char				},
	{ "noarg_room",	soced_noarg_room				},

	{ "self_char",	soced_self_char					},
	{ "self_room",	soced_self_room					},

	{ "notfound_char",soced_notfound_char				},

	{ "move",	soced_move					},
	{ "delete_so",	olced_spell_out					},
	{ "delete_soc",	soced_delete					},
	{ "commands",	show_commands					},
	{ NULL }
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
		char_puts("SocEd: Insufficient security for creating socials\n",
			  ch);
		return FALSE;
	}

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0')
		OLC_ERROR("'OLC CREATE'");

	if ((soc = social_lookup(arg))) {
		char_printf(ch, "SocEd: %s: already exists.\n", soc->name);
		return FALSE;
	}

	shadow_print_cmds(ch, arg);
	soc		= varr_enew(&socials);
	soc->name	= str_dup(arg);

	ch->desc->pEdit	= (void *) soc;
	OLCED(ch)	= olced_lookup(ED_SOCIAL);
	SET_BIT(changed_flags, CF_SOCIAL);
	char_puts("Social created.\n",ch);
	return FALSE;
}

OLC_FUN(soced_edit)
{
	social_t *soc;
	char arg[MAX_STRING_LENGTH];

	if (PC(ch)->security < SECURITY_SOCIALS) {
		char_puts("SocEd: Insufficient security.\n", ch);
		return FALSE;
	}

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0')
		OLC_ERROR("'OLC EDIT'");

	if ((soc = social_lookup(arg)) == NULL) {
		char_printf(ch, "SocEd: %s: No such social.\n", arg);
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

	varr_foreach(&socials, save_social_cb, fp);

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

#define SOC_SHOW(format, s)			\
	if (!IS_NULLSTR(s))			\
		buf_printf(output, format, s);

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
			char_printf(ch, "SocEd: %s: No such social.\n", arg);
			return FALSE;
		}
	}

	output = buf_new(-1);

	buf_printf(output,
		   "Name:          [%s]\n",
		   soc->name);
	buf_printf(output,
		   "Min_pos:       [%s]\n",
		   flag_string(position_table, soc->min_pos));

	SOC_SHOW("Found char   [%s]\n", soc->found_char);
	SOC_SHOW("Found vict   [%s]\n", soc->found_vict);
	SOC_SHOW("Found other  [%s]\n", soc->found_notvict);
	SOC_SHOW("Noarg char   [%s]\n", soc->noarg_char);
	SOC_SHOW("Noarg room   [%s]\n", soc->noarg_room);
	SOC_SHOW("Self  char   [%s]\n", soc->self_char);
	SOC_SHOW("Self  room   [%s]\n", soc->self_room);
	SOC_SHOW("Notfound     [%s]\n", soc->notfound_char);
	shadow_dump_cmds(output, soc->name);

	page_to_char(buf_string(output), ch);
	buf_free(output);

	return FALSE;
}

OLC_FUN(soced_list)
{
	int i;
	int col = 0;
	char arg[MAX_STRING_LENGTH];
	BUFFER *output;

	one_argument(argument, arg, sizeof(arg));
	output = buf_new(-1);

	for (i = 0; i < socials.nused; i++) {
		social_t *soc = (social_t*) VARR_GET(&socials, i);

		if (arg[0] && str_prefix(arg, soc->name))
			continue;

		buf_printf(output, "[%1s%3d] %-12s",
			check_shadow(soc->name) ? "*" : " ", i,	soc->name);
		if (++col % 4 == 0)
			buf_add(output, "\n");
	}

	if (col % 4)
		buf_add(output, "\n");

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
	return olced_str(ch, argument, cmd, &soc->found_char);
}

OLC_FUN(soced_found_vict)
{
	social_t *soc;
	EDIT_SOC(ch, soc);
	return olced_str(ch, argument, cmd, &soc->found_vict);
}

OLC_FUN(soced_found_notvict)
{
	social_t *soc;
	EDIT_SOC(ch, soc);
	return olced_str(ch, argument, cmd, &soc->found_notvict);
}

OLC_FUN(soced_noarg_char)
{
	social_t *soc;
	EDIT_SOC(ch, soc);
	return olced_str(ch, argument, cmd, &soc->noarg_char);
}

OLC_FUN(soced_noarg_room)
{
	social_t *soc;
	EDIT_SOC(ch, soc);
	return olced_str(ch, argument, cmd, &soc->noarg_room);
}

OLC_FUN(soced_self_char)
{
	social_t *soc;
	EDIT_SOC(ch, soc);
	return olced_str(ch, argument, cmd, &soc->self_char);
}

OLC_FUN(soced_self_room)
{
	social_t *soc;
	EDIT_SOC(ch, soc);
	return olced_str(ch, argument, cmd, &soc->self_room);
}

OLC_FUN(soced_notfound_char)
{
	social_t *soc;
	EDIT_SOC(ch, soc);
	return olced_str(ch, argument, cmd, &soc->notfound_char);
}

OLC_FUN(soced_move)
{
	social_t *soc, *nsoc;
	char arg[MAX_INPUT_LENGTH];
	int num, num2;

	EDIT_SOC(ch, soc);

	if (olced_busy(ch, ED_SOCIAL, NULL, NULL))
		return FALSE;

	one_argument(argument, arg, sizeof(arg));
	if (!is_number(arg))
		OLC_ERROR("'OLC SOCIALS'");

	if ((num = atoi(arg)) > socials.nused)
		num = socials.nused;

	num2 = varr_index(&socials, soc);

	nsoc = (social_t *)varr_insert(&socials, num);
	if (num <= num2)
		soc = (social_t *)varr_get(&socials, num2 + 1);

	nsoc->name = soc->name;
	nsoc->min_pos = soc->min_pos;
	nsoc->found_char = soc->found_char;
	nsoc->found_vict = soc->found_vict;
	nsoc->found_notvict = soc->found_notvict;
	nsoc->noarg_char = soc->noarg_char;
	nsoc->noarg_room = soc->noarg_room;
	nsoc->self_char = soc->self_char;
	nsoc->self_room = soc->self_room;
	
	varr_edelete(&socials, soc);
	ch->desc->pEdit	= nsoc;
	char_printf(ch, "SocEd: '%s' moved to %d position.\n",
		nsoc->name, varr_index(&socials, nsoc));
	return TRUE;
}

OLC_FUN(soced_delete)
{
	social_t *soc;
	EDIT_SOC(ch, soc);

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
		char_printf(ch,
			    "SocEd: %s: illegal character in social name.\n",
			    arg);
		return FALSE;
	}

	if ((soc2 = social_lookup(name))
	&&  soc2 != soc) {
		char_printf(ch, "SocEd: %s: duplicate social name.\n", arg);
		return FALSE;
	}

	return TRUE;
}

static void *save_social_cb(void *p, va_list ap)
{
	social_t *soc = (social_t *) p;
	FILE *fp = va_arg(ap, FILE *);

	fprintf(fp, "#SOCIAL\n");
	fprintf(fp, "Name %s\n", soc->name);
	fprintf(fp, "Min_pos %s\n",
		flag_string(position_table, soc->min_pos));
	fwrite_string(fp, "Found_char", soc->found_char);
	fwrite_string(fp, "Found_vict", soc->found_vict);
	fwrite_string(fp, "Found_notvict", soc->found_notvict);
	fwrite_string(fp, "Noarg_char", soc->noarg_char);
	fwrite_string(fp, "Noarg_room", soc->noarg_room);
	fwrite_string(fp, "Self_char", soc->self_char);
	fwrite_string(fp, "Self_room", soc->self_room);
	fwrite_string(fp, "Notfound_char", soc->notfound_char);
	fprintf(fp, "End\n\n");

	return NULL;
}

static void *
shadow_dump_cb(void *p, va_list ap)
{
	cmd_t *cmd = (cmd_t *) p;
	const char *name = va_arg(ap, const char *);
	BUFFER *output = va_arg(ap, BUFFER *);

	if (!str_prefix(name, cmd->name))
		buf_printf(output, "   [%s]\n", cmd->name);

	return 0;
}

static void
shadow_dump_cmds(BUFFER *output, const char *name)
{
	if (check_shadow(name))
		return;

	buf_add(output, "[*** SHADOWED BY FOLLOWING COMMANDS ***]\n");
	varr_foreach(&commands, shadow_dump_cb, name, output);
}

static void
shadow_print_cmds(CHAR_DATA *ch, const char *name)
{
	BUFFER *output = buf_new(-1);
	shadow_dump_cmds(output, name);
	page_to_char(buf_string(output), ch);
	buf_free(output);
}

