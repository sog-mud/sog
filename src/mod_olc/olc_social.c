/*-
 * Copyright (c) 1998 fjoe <fjoe@iclub.nsu.ru>
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
 * $Id: olc_social.c,v 1.7 1999-06-24 16:33:12 fjoe Exp $
 */

#include <stdio.h>
#include <string.h>

#include "merc.h"
#include "olc.h"
#include "socials.h"

#define EDIT_SOC(ch, soc)	(soc = (social_t*) ch->desc->pEdit)

DECLARE_OLC_FUN(soced_create		);
DECLARE_OLC_FUN(soced_edit		);
DECLARE_OLC_FUN(soced_show		);
DECLARE_OLC_FUN(soced_list		);

DECLARE_OLC_FUN(soced_name		);
DECLARE_OLC_FUN(soced_min_pos		);
DECLARE_OLC_FUN(soced_found_char	);
DECLARE_OLC_FUN(soced_found_vict	);
DECLARE_OLC_FUN(soced_found_notvict	);
DECLARE_OLC_FUN(soced_noarg_char	);
DECLARE_OLC_FUN(soced_noarg_room	);
DECLARE_OLC_FUN(soced_self_char		);
DECLARE_OLC_FUN(soced_self_room		);
DECLARE_OLC_FUN(soced_notfound_char	);

static DECLARE_VALIDATE_FUN(validate_name);

olc_cmd_t olc_cmds_soc[] =
{
	{ "create",		soced_create				},
	{ "edit",		soced_edit				},
	{ "touch",		olced_dummy				},
	{ "show",		soced_show				},
	{ "list",		soced_list				},

	{ "name",		soced_name,		validate_name 	},
	{ "min_pos",		soced_min_pos,		position_table	},

	{ "found_char", 	soced_found_char			},
	{ "found_vict",		soced_found_vict			},
	{ "found_notvict",	soced_found_notvict			},

	{ "noarg_char",		soced_noarg_char			},
	{ "noarg_room",		soced_noarg_room			},

	{ "self_char",		soced_self_char				},
	{ "self_room",		soced_self_room				},

	{ "notfound_char",	soced_notfound_char			},

	{ "commands",		show_commands				},
	{ NULL }
};

OLC_FUN(soced_create)
{
	social_t *soc;
	char arg[MAX_STRING_LENGTH];

	if (ch->pcdata->security < SECURITY_SOCIALS) {
		char_puts("SocEd: Insufficient security for creating socials\n",
			  ch);
		return FALSE;
	}

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		dofun("help", ch, "'OLC CREATE'");
		return FALSE;
	}

	if ((soc = social_lookup(arg, str_cmp))) {
		char_printf(ch, "SocEd: %s: already exists.\n", soc->name);
		return FALSE;
	}

	soc		= social_new();
	soc->name	= str_dup(arg);

	ch->desc->pEdit	= (void *) soc;
	OLCED(ch)	= olced_lookup(ED_SOC);
	char_puts("Social created.\n",ch);
	return FALSE;
}

OLC_FUN(soced_edit)
{
	social_t *soc;
	char arg[MAX_STRING_LENGTH];

	if (ch->pcdata->security < SECURITY_SOCIALS) {
		char_puts("SocEd: Insufficient security.\n", ch);
		return FALSE;
	}

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		dofun("help", ch, "'OLC EDIT'");
		return FALSE;
	}

	if ((soc = social_lookup(arg, str_cmp)) == NULL) {
		char_printf(ch, "SocEd: %s: No such social.\n", arg);
		return FALSE;
	}

	ch->desc->pEdit	= soc;
	OLCED(ch)	= olced_lookup(ED_SOC);
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
		if (IS_EDIT(ch, ED_SOC))
			EDIT_SOC(ch, soc);
		else {
			dofun("help", ch, "'OLC ASHOW'");
			return FALSE;
		}
	}
	else {
		if ((soc = social_lookup(arg, str_prefix)) == NULL) {
			char_printf(ch, "SocEd: %s: No such social.\n", arg);
			return FALSE;
		}
	}

	output = buf_new(-1);

	buf_printf(output,
		   "name:          [%s]\n",
		   soc->name);
	buf_printf(output,
		   "min_pos:       [%s]\n",
		   flag_string(position_table, soc->min_pos));

	SOC_SHOW("found_char:    [%s]\n", soc->found_char);
	SOC_SHOW("found_vict:    [%s]\n", soc->found_vict);
	SOC_SHOW("found_notvict: [%s]\n", soc->found_notvict);
	SOC_SHOW("noarg_char:    [%s]\n", soc->noarg_char);
	SOC_SHOW("noarg_room:    [%s]\n", soc->noarg_room);
	SOC_SHOW("self_char:     [%s]\n", soc->self_char);
	SOC_SHOW("self_room:     [%s]\n", soc->self_room);
	SOC_SHOW("notfound_char: [%s]\n", soc->notfound_char);

	page_to_char(buf_string(output), ch);
	buf_free(output);

	return FALSE;
}

OLC_FUN(soced_list)
{
	int i;
	int col = 0;
	char arg[MAX_STRING_LENGTH];

	one_argument(argument, arg, sizeof(arg));

	for (i = 0; i < socials.nused; i++) {
		social_t *soc = (social_t*) VARR_GET(&socials, i);

		if (arg[0] && str_prefix(arg, soc->name))
			continue;

		char_printf(ch, "%-12s", soc->name);
		if (++col % 6 == 0)
			char_puts("\n", ch);
	}

	if (col % 6)
		char_puts("\n", ch);

	return FALSE;
}

OLC_FUN(soced_name)
{
	social_t *soc;
	EDIT_SOC(ch, soc);
	return olced_str(ch, argument, cmd, &soc->name);
}

OLC_FUN(soced_min_pos)
{
	social_t *soc;
	EDIT_SOC(ch, soc);
	return olced_flag32(ch, argument, cmd, &soc->min_pos);
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

static VALIDATE_FUN(validate_name)
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

	if ((soc2 = social_lookup(name, str_cmp))
	&&  soc2 != soc) {
		char_printf(ch, "SocEd: %s: duplicate social name.\n", arg);
		return FALSE;
	}

	return TRUE;
}

