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
 * $Id: olc_rule.c,v 1.11 1999-06-10 14:33:36 fjoe Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include "merc.h"
#include "olc.h"
#include "db/lang.h"

#define EDIT_RULE(ch, r)	(r = (rule_t*) ch->desc->pEdit)
#define EDIT_RCL(ch, rcl)	(rcl = (rulecl_t*) ch->desc->pEdit2)
#define EDIT_ROPS(ch, rops)	(rops = (ruleops_t*) cmd->arg1)
#define EDIT_LANG(ch, l)						\
	{								\
		if ((l = varr_get(&langs, ch->lang)) == NULL) {		\
			char_puts("RuleEd: unknown current language. "	\
				  "Use 'lang' command to set "		\
				  "correct language.\n", ch);		\
			return FALSE;					\
		}							\
	}

typedef struct ruleops_t ruleops_t;
struct ruleops_t {
	rule_t*		(*rule_lookup)(rulecl_t *rcl, const char *name);
	void		(*rule_del)(rulecl_t *rcl, rule_t *r);
	const char *	id;
	flag32_t	bit;
};

ruleops_t rops_expl = { erule_lookup, erule_del, ED_EXPL, RULES_EXPL_CHANGED };
ruleops_t rops_impl = { irule_lookup, irule_del, ED_IMPL, RULES_IMPL_CHANGED };

DECLARE_OLC_FUN(ruleed_create	);
DECLARE_OLC_FUN(ruleed_edit	);
DECLARE_OLC_FUN(ruleed_touch	);
DECLARE_OLC_FUN(ruleed_show	);
DECLARE_OLC_FUN(ruleed_list	);

DECLARE_OLC_FUN(ruleed_name	);
DECLARE_OLC_FUN(ruleed_base	);
DECLARE_OLC_FUN(ruleed_arg	);
DECLARE_OLC_FUN(ruleed_add	);
DECLARE_OLC_FUN(ruleed_del	);
DECLARE_OLC_FUN(ruleed_delete	);

DECLARE_OLC_FUN(eruleed_name	);
DECLARE_OLC_FUN(iruleed_name	);
DECLARE_OLC_FUN(eruleed_list	);

olc_cmd_t olc_cmds_expl[] =
{
	{ "create",		ruleed_create,	&rops_expl	},
	{ "edit",		ruleed_edit,	&rops_expl	},
	{ "touch",		ruleed_touch,	&rops_expl	},
	{ "show",		ruleed_show,	&rops_expl	},
	{ "list",		ruleed_list,	&rops_expl	},

	{ "name",		eruleed_name			},
	{ "base",		ruleed_base			},
	{ "add",		ruleed_add			},
	{ "del",		ruleed_del			},
	{ "delete_rul",		olced_spell_out			},
	{ "delete_rule",	ruleed_delete,	&rops_expl	},

	{ "commands",		show_commands			},
	{ NULL }
};

olc_cmd_t olc_cmds_impl[] =
{
	{ "create",		ruleed_create,	&rops_impl	},
	{ "edit",		ruleed_edit,	&rops_impl	},
	{ "touch",		ruleed_touch,	&rops_impl	},
	{ "show",		ruleed_show,	&rops_impl	},
	{ "list",		ruleed_list,	&rops_impl	},

	{ "name",		iruleed_name			},
	{ "arg",		ruleed_arg			},
	{ "add",		ruleed_add			},
	{ "del",		ruleed_del			},
	{ "delete_rul",		olced_spell_out			},
	{ "delete_rule",	ruleed_delete,	&rops_impl	},

	{ "commands",		show_commands			},
	{ NULL }
};

OLC_FUN(ruleed_create)
{
	int rulecl;
	rulecl_t *rcl;
	ruleops_t *rops;
	lang_t *l;
	rule_t rnew;
	char arg[MAX_STRING_LENGTH];
	char arg2[MAX_STRING_LENGTH];
	bool impl;

	if (ch->pcdata->security < SECURITY_MSGDB) {
		char_puts("RuleEd: Insufficient security.\n", ch);
		return FALSE;
	}

	EDIT_LANG(ch, l);
	EDIT_ROPS(ch, rops);
	impl = (rops->id == ED_IMPL);

	argument = one_argument(argument, arg, sizeof(arg));
	if (impl)
		argument = one_argument(argument, arg2, sizeof(arg2));
	if (argument[0] == '\0' || (impl && !is_number(arg2))) {
		do_help(ch, "'OLC CREATE'");
		return FALSE;
	}

	if ((rulecl = flag_value(rulecl_names, arg)) < 0) {
		char_printf(ch, "RuleEd: %s: unknown rule class.\n", arg);
		return FALSE;
	}
	rcl = l->rules + rulecl;

	if (!impl && erule_lookup(rcl, argument)) {
		char_printf(ch, "RuleEd: %s: duplicate name.\n", argument);
		return FALSE;
	}

	if (olced_busy(ch, rops->id, NULL, rcl))
		return FALSE;

	rule_init(&rnew);
	rnew.name = str_dup(argument);
	OLCED(ch)	= olced_lookup(rops->id);
	ch->desc->pEdit = impl ? irule_insert(rcl, atoi(arg2), &rnew) :
				 erule_add(rcl, &rnew);
	ch->desc->pEdit2= rcl; 
	SET_BIT(rcl->rcl_flags, rops->bit);
	char_puts("RuleEd: rule created.\n", ch);
	return FALSE;
}

OLC_FUN(ruleed_edit)
{
	int rulecl;
	rulecl_t *rcl;
	ruleops_t *rops;
	lang_t *l;
	rule_t *r;
	char arg[MAX_INPUT_LENGTH];

	if (ch->pcdata->security < SECURITY_MSGDB) {
		char_puts("RuleEd: Insufficient security.\n", ch);
		return FALSE;
	}

	argument = one_argument(argument, arg, sizeof(arg));
	if (argument[0] == '\0') {
		do_help(ch, "'OLC EDIT'");
		return FALSE;
	}

	EDIT_LANG(ch, l);
	EDIT_ROPS(ch, rops);

	if ((rulecl = flag_value(rulecl_names, arg)) < 0) {
		char_printf(ch, "RuleEd: %s: unknown rule class.\n", arg);
		return FALSE;
	}
	rcl = l->rules+rulecl;

	if ((r = rops->rule_lookup(rcl, argument)) == NULL) {
		char_printf(ch, "RuleEd: %s: not found.\n", argument);
		return FALSE;
	}

	ch->desc->olced = olced_lookup(rops->id);
	ch->desc->pEdit	= r;
	ch->desc->pEdit2= rcl;
	return FALSE;
}

OLC_FUN(ruleed_touch)
{
	rulecl_t *rcl;
	ruleops_t *rops;

	EDIT_RCL(ch, rcl);
	EDIT_ROPS(ch, rops);
	SET_BIT(rcl->rcl_flags, rops->bit);
	return FALSE;
}

OLC_FUN(ruleed_show)
{
	int i;
	rule_t *r;
	rulecl_t *rcl;
	ruleops_t *rops;
	lang_t *l;

	EDIT_LANG(ch, l);
	EDIT_ROPS(ch, rops);

	if (argument[0] == '\0') {
		if (IS_EDIT(ch, ED_IMPL) || IS_EDIT(ch, ED_EXPL)) {
			EDIT_RULE(ch, r);
			EDIT_RCL(ch, rcl);
		}
		else {
			do_help(ch, "'OLC ASHOW'");
			return FALSE;
		}
	}
	else {
		int rulecl;
		char arg[MAX_INPUT_LENGTH];

		argument = one_argument(argument, arg, sizeof(arg));
		if (argument[0] == '\0') {
			do_help(ch, "'OLC ASHOW'");
			return FALSE;
		}

		if ((rulecl = flag_value(rulecl_names, arg)) < 0) {
			char_printf(ch, "RuleEd: %s: unknown rule class.\n",
				    arg);
			return FALSE;
		}
		rcl = l->rules + rulecl;

		if ((r = rops->rule_lookup(rcl, argument)) == NULL) {
			char_printf(ch, "RuleEd: %s: not found.\n", argument);
			return FALSE;
		}
	}

	char_printf(ch, "Name: [%s]\n"
			"Lang: [%s]  Class: [%s]  Type: [%s]\n",
		    r->name,
		    l->name, flag_string(rulecl_names, rcl->rulecl),
		    rops->id);

	if (rops->id == ED_IMPL) 
		char_printf(ch, "Arg:  [%d]\n", r->arg);
	else {
		char buf[MAX_STRING_LENGTH];

		strnzncpy(buf, sizeof(buf), r->name, r->arg);
		char_printf(ch, "Base: [%s] (%d)\n", buf, r->arg);
	}

	for (i = 0; i < r->f->v.nused; i++) {
		char **p = VARR_GET(&r->f->v, i);

		if (!IS_NULLSTR(*p))
			char_printf(ch, "Form: [%d] [%s]\n", i, *p);
	}

	return FALSE;
}

OLC_FUN(ruleed_list)
{
	int i;
	rulecl_t *rcl;
	ruleops_t *rops;
	lang_t *l;
	BUFFER *output = NULL;

	EDIT_LANG(ch, l);
	EDIT_ROPS(ch, rops);

	/*
	 * obtain rule class
	 */
	if (argument[0] == '\0') {
		if (IS_EDIT(ch, ED_IMPL) || IS_EDIT(ch, ED_EXPL))
			EDIT_RCL(ch, rcl);
		else {
			do_help(ch, "'OLC ALIST'");
			return FALSE;
		}
	}
	else {
		int rulecl;
		char arg[MAX_INPUT_LENGTH];

		argument = one_argument(argument, arg, sizeof(arg));
		if (arg[0] == '\0') {
			do_help(ch, "'OLC ALIST'");
			return FALSE;
		}

		if ((rulecl = flag_value(rulecl_names, arg)) < 0) {
			char_printf(ch, "RuleEd: %s: unknown rule class.\n",
				    arg);
			return FALSE;
		}
		rcl = l->rules + rulecl;
	}

	if (rops->id == ED_IMPL) {
		if (rcl->impl.nused) {
			output = buf_new(-1);
			for (i = 0; i < rcl->impl.nused; i++) {
				rule_t *r = VARR_GET(&rcl->impl, i);
				buf_printf(output, "%3d. %s\n", i, r->name);
			}
		}
	}
	else {
		if (argument[0] == '\0') {
			do_help(ch, "'OLC ALIST'");
			return FALSE;
		}
	
		for (i = 0; i < MAX_RULE_HASH; i++) {
			int j;

			for (j = 0; j < rcl->expl[i].nused; j++) {
				rule_t *r = VARR_GET(rcl->expl+i, j);

				if (!str_prefix(argument, r->name)) {
					if (!output)
						output = buf_new(-1);
					buf_printf(output, "%s\n", r->name);
				}
			}
		}
	}

	if (output) {
		page_to_char(buf_string(output), ch);
		buf_free(output);
	}
	else
		char_puts("RuleEd: no rules found.\n", ch);

	return FALSE;
}

OLC_FUN(eruleed_name)
{
	rule_t *r;
	rule_t *r2;
	rule_t rnew;
	rulecl_t *rcl;

	if (argument[0] == '\0') {
		do_help(ch, "'OLC RULE'");
		return FALSE;
	}

	EDIT_RULE(ch, r);
	EDIT_RCL(ch, rcl);
	if (olced_busy(ch, ED_EXPL, NULL, rcl))
		return FALSE;

	if ((r2 = erule_lookup(rcl, argument)) && r2 != r) {
		char_printf(ch, "RuleEd: %s: duplicate name.\n", argument);
		return FALSE;
	}

	erule_del(rcl, r);
	rule_init(&rnew);
	rnew.name = str_dup(argument);
	ch->desc->pEdit = erule_add(rcl, &rnew);
	return TRUE;
}

OLC_FUN(iruleed_name)
{
	rule_t *r;
	EDIT_RULE(ch, r);
	return olced_str(ch, argument, cmd, &r->name);
}

OLC_FUN(ruleed_base)
{
	char arg[MAX_INPUT_LENGTH];
	rule_t *r;
	EDIT_RULE(ch, r);

	one_argument(argument, arg, sizeof(arg));
	if (str_prefix(arg, r->name)) {
		char_printf(ch, "RuleEd: %s: not prefix of name (%s).\n",
			    arg, r->name);
		return FALSE;
	}

	r->arg = strlen(arg);
	return TRUE;
}

OLC_FUN(ruleed_arg)
{
	rule_t *r;
	EDIT_RULE(ch, r);
	return olced_number(ch, argument, cmd, &r->arg);
}

OLC_FUN(ruleed_add)
{
	rule_t *r;
	EDIT_RULE(ch, r);
	return olced_vform_add(ch, argument, cmd, r);
}

OLC_FUN(ruleed_del)
{
	rule_t *r;
	EDIT_RULE(ch, r);
	return olced_vform_del(ch, argument, cmd, r);
}

OLC_FUN(ruleed_delete)
{
	rule_t *r;
	rulecl_t *rcl;
	ruleops_t *rops;

	EDIT_RULE(ch, r);
	EDIT_RCL(ch, rcl);
	EDIT_ROPS(ch, rops);

	if (olced_busy(ch, rops->id, NULL, rcl))
		return FALSE;

	rops->rule_del(rcl, r);
	SET_BIT(rcl->rcl_flags, rops->bit);
	edit_done(ch->desc);

	return FALSE;
}

