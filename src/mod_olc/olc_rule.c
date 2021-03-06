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
 * $Id: olc_rule.c,v 1.47 2004-06-28 19:21:04 tatyana Exp $
 */

#include "olc.h"

#include <lang.h>

#define EDIT_RULE(ch, r)	(r = (rule_t *) ch->desc->pEdit)
#define EDIT_RCL(ch, rcl)	(rcl = (rulecl_t *) ch->desc->pEdit2)
#define EDIT_ROPS(ch, rops)	(rops = (ruleops_t *) cmd->arg1)
#define EDIT_LANG(ch, l)						\
	{								\
		if ((l = varr_get(&langs, GET_LANG(ch))) == NULL) {	\
			act_char("RuleEd: unknown current language. "	\
				 "Use 'lang' command to set "		\
				 "correct language.", ch);		\
			return FALSE;					\
		}							\
	}

typedef struct ruleops_t ruleops_t;
struct ruleops_t {
	rule_t*		(*rule_lookup)(rulecl_t *rcl, const char *name);
	void		(*rule_del)(rulecl_t *rcl, rule_t *r);
	void		(*rcl_save)(CHAR_DATA *ch, lang_t *l, rulecl_t *rcl);
	const char *	id;
	flag_t	bit;
};

static void rcl_save_expl(CHAR_DATA *ch, lang_t *l, rulecl_t *rcl);
static void rcl_save_impl(CHAR_DATA *ch, lang_t *l, rulecl_t *rcl);

ruleops_t rops_expl =
{
	erule_lookup,
	erule_del,
	rcl_save_expl,
	ED_EXPL,
	RULES_EXPL_CHANGED
};

ruleops_t rops_impl =
{
	irule_lookup,
	irule_del,
	rcl_save_impl,
	ED_IMPL,
	RULES_IMPL_CHANGED
};

DECLARE_OLC_FUN(ruleed_create	);
DECLARE_OLC_FUN(ruleed_edit	);
DECLARE_OLC_FUN(ruleed_save	);
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
DECLARE_OLC_FUN(eruleed_auto	);
DECLARE_OLC_FUN(iruleed_name	);

olc_cmd_t olc_cmds_expl[] =
{
	{ "create",	ruleed_create,	NULL,	&rops_expl	},
	{ "edit",	ruleed_edit,	NULL,	&rops_expl	},
	{ "",		ruleed_save,	NULL,	&rops_expl	},
	{ "touch",	ruleed_touch,	NULL,	&rops_expl	},
	{ "show",	ruleed_show,	NULL,	&rops_expl	},
	{ "list",	ruleed_list,	NULL,	&rops_expl	},

	{ "name",	eruleed_name,	NULL,	NULL		},
	{ "auto",	eruleed_auto,	NULL,	NULL		},
	{ "base",	ruleed_base,	NULL,	NULL		},
	{ "add",	ruleed_add,	NULL,	NULL		},
	{ "del",	ruleed_del,	NULL,	NULL		},
	{ "delete_rul",	olced_spell_out, NULL,	NULL		},
	{ "delete_rule",ruleed_delete,	NULL,	&rops_expl	},

	{ "commands",	show_commands,	NULL,	NULL		},
	{ "version",	show_version,	NULL,	NULL		},

	{ NULL, NULL, NULL, NULL }
};

olc_cmd_t olc_cmds_impl[] =
{
	{ "create",	ruleed_create,	NULL,	&rops_impl	},
	{ "edit",	ruleed_edit,	NULL,	&rops_impl	},
	{ "",		ruleed_save,	NULL,	&rops_impl	},
	{ "touch",	ruleed_touch,	NULL,	&rops_impl	},
	{ "show",	ruleed_show,	NULL,	&rops_impl	},
	{ "list",	ruleed_list,	NULL,	&rops_impl	},

	{ "name",	iruleed_name,	NULL,	NULL		},
	{ "arg",	ruleed_arg,	NULL,	NULL		},
	{ "add",	ruleed_add,	NULL,	NULL		},
	{ "del",	ruleed_del,	NULL,	NULL		},
	{ "delete_rul",	olced_spell_out, NULL,	NULL		},
	{ "delete_rule",ruleed_delete,	NULL,	&rops_impl	},

	{ "commands",	show_commands,	NULL,	NULL		},
	{ "version",	show_version,	NULL,	NULL		},

	{ NULL, NULL, NULL, NULL }
};

OLC_FUN(ruleed_create)
{
	int rulecl;
	rulecl_t *rcl;
	ruleops_t *rops;
	lang_t *l;
	rule_t rnew;
	char arg1[MAX_STRING_LENGTH];
	char arg2[MAX_STRING_LENGTH];
	bool impl;

	if (PC(ch)->security < SECURITY_MSGDB) {
		act_char("RuleEd: Insufficient security.", ch);
		return FALSE;
	}

	EDIT_LANG(ch, l);
	EDIT_ROPS(ch, rops);
	impl = (rops->id == ED_IMPL);

	argument = one_argument(argument, arg1, sizeof(arg1));
	if (impl)
		argument = one_argument(argument, arg2, sizeof(arg2));
	if (argument[0] == '\0' || (impl && !is_number(arg2)))
		OLC_ERROR("'OLC CREATE'");

	if ((rulecl = flag_value(rulecl_names, arg1)) < 0) {
		act_puts("RuleEd: $t: unknown rule class.",
			 ch, arg1, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}
	rcl = l->rules + rulecl;

	if (!impl && erule_lookup(rcl, argument)) {
		char buf[MAX_INPUT_LENGTH];

		act_puts("RuleEd: $t: already exists:",
			 ch, argument, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		snprintf(buf, sizeof(buf), "expl %s %s", arg1, argument);
		dofun("ashow", ch, buf);
		return FALSE;
	}

	if (olced_busy(ch, rops->id, NULL, rcl))
		return FALSE;

	rule_init(&rnew);
	rnew.name	= str_dup(argument);
	rnew.arg	= impl ? 0 : strlen(argument);
	OLCED(ch)	= olced_lookup(rops->id);
	ch->desc->pEdit = impl ? irule_insert(rcl, atoi(arg2), &rnew) :
				 erule_add(rcl, &rnew);
	ch->desc->pEdit2= rcl;
	SET_BIT(rcl->rcl_flags, rops->bit);
	act_char("RuleEd: rule created.", ch);
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

	if (PC(ch)->security < SECURITY_MSGDB) {
		act_char("RuleEd: Insufficient security.", ch);
		return FALSE;
	}

	argument = one_argument(argument, arg, sizeof(arg));
	if (argument[0] == '\0')
		OLC_ERROR("'OLC EDIT'");

	EDIT_LANG(ch, l);
	EDIT_ROPS(ch, rops);

	if ((rulecl = flag_value(rulecl_names, arg)) < 0) {
		act_puts("RuleEd: $t: unknown rule class.",
			 ch, arg, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}
	rcl = l->rules+rulecl;

	if ((r = rops->rule_lookup(rcl, argument)) == NULL) {
		act_puts("RuleEd: $t: not found.",
			 ch, argument, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	OLCED(ch) = olced_lookup(rops->id);
	ch->desc->pEdit	= r;
	ch->desc->pEdit2= rcl;
	return FALSE;
}

OLC_FUN(ruleed_save)
{
	ruleops_t *rops;
	lang_t *l;

	if (!olc_trusted(ch, SECURITY_MSGDB) < 0) {
		olc_printf(ch, "Insufficient security.");
		return FALSE;
	}

	EDIT_ROPS(ch, rops);
	C_FOREACH (lang_t *, l, &langs) {
		int i;

		for (i = 0; i < MAX_RULECL; i++)
			rops->rcl_save(ch, l, l->rules+i);
	}

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
	rule_t *r;
	rulecl_t *rcl;
	ruleops_t *rops;
	lang_t *l;
	const char **p;

	EDIT_LANG(ch, l);
	EDIT_ROPS(ch, rops);

	if (argument[0] == '\0') {
		if (IS_EDIT(ch, ED_IMPL) || IS_EDIT(ch, ED_EXPL)) {
			EDIT_RULE(ch, r);
			EDIT_RCL(ch, rcl);
		}
		else
			OLC_ERROR("'OLC ASHOW'");
	} else {
		int rulecl;
		char arg[MAX_INPUT_LENGTH];

		argument = one_argument(argument, arg, sizeof(arg));
		if (argument[0] == '\0')
			OLC_ERROR("'OLC ASHOW'");

		if ((rulecl = flag_value(rulecl_names, arg)) < 0) {
			act_puts("RuleEd: $t: unknown rule class.",
				 ch, arg, NULL,
				 TO_CHAR | ACT_NOTRANS, POS_DEAD);
			return FALSE;
		}
		rcl = l->rules + rulecl;

		if ((r = rops->rule_lookup(rcl, argument)) == NULL) {
			act_puts("RuleEd: $t: not found.",
				 ch, argument, NULL,
				 TO_CHAR | ACT_NOTRANS, POS_DEAD);
			return FALSE;
		}
	}

	act_puts("Name: [$t]",
		 ch, r->name, NULL, TO_CHAR | ACT_NOTRANS, POS_DEAD);
	act_puts3("Lang: [$t]  Class: [$T]  Type: [$U]",
		  ch, l->name, flag_string(rulecl_names, rcl->rulecl), rops->id,
		  TO_CHAR | ACT_NOTRANS, POS_DEAD);

	if (rops->id == ED_IMPL) {
		act_puts("Arg:  [$j]",
			 ch, (const void *) r->arg, NULL, TO_CHAR, POS_DEAD);
	} else {
		char buf[MAX_STRING_LENGTH];

		strlncpy(buf, r->name, sizeof(buf), r->arg);
		act_puts("Base: [$T] ($j)",
			 ch, (const void *) r->arg, buf,
			 TO_CHAR | ACT_NOTRANS, POS_DEAD);
	}

	C_FOREACH (const char **, p, &r->forms) {
		size_t i = varr_index(&r->forms, p);
		int i2;

		/* gender shift */
		if (rcl->rulecl == RULES_GENDER) {
			if (i == SEX_PLURAL - 1)
				i2 = i + 1;
			else
				i2 = (i + 1) % 3;
		} else
			i2 = i;

		if (!IS_NULLSTR(*p)) {
			if (rops->id == ED_IMPL) {
				act_puts("Form: [$j] [$T]",
					 ch, (const void *) i, *p,
					 TO_CHAR | ACT_NOTRANS, POS_DEAD);
			} else {
				act_puts3("Form: [$j] [$T] $U",
					  ch, (const void *) i, *p,
					  word_form(r->name, i2,
						    varr_index(&langs, l),
						    rcl->rulecl),
					  TO_CHAR | ACT_NOTRANS, POS_DEAD);
			}
		}
	}

	return FALSE;
}

OLC_FUN(ruleed_list)
{
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
		else
			OLC_ERROR("'OLC ALIST'");
	} else {
		int rulecl;
		char arg[MAX_INPUT_LENGTH];

		argument = one_argument(argument, arg, sizeof(arg));
		if (arg[0] == '\0')
			OLC_ERROR("'OLC ALIST'");

		if ((rulecl = flag_value(rulecl_names, arg)) < 0) {
			act_puts("RuleEd: $t: unknown rule class.",
				 ch, arg, NULL,
				 TO_CHAR | ACT_NOTRANS, POS_DEAD);
			return FALSE;
		}
		rcl = l->rules + rulecl;
	}

	if (rops->id == ED_IMPL) {
		if (!c_isempty(&rcl->impl)) {
			rule_t *r;

			output = buf_new(0);

			C_FOREACH (rule_t *, r, &rcl->impl) {
				buf_printf(output, BUF_END,
				    "%3d. %s\n", varr_index(&rcl->impl, r),
				    r->name);
			}
		}
	} else {
		rule_t *r;

		if (argument[0] == '\0')
			OLC_ERROR("'OLC ALIST'");

		C_FOREACH (rule_t *, r, &rcl->expl) {
			if (!!str_prefix(argument, r->name))
				continue;

			if (!output)
				output = buf_new(0);
			buf_printf(output, BUF_END, "%s\n", r->name);
		}
	}

	if (output) {
		page_to_char(buf_string(output), ch);
		buf_free(output);
	} else
		act_char("RuleEd: no rules found.", ch);

	return FALSE;
}

OLC_FUN(eruleed_name)
{
	rule_t *r;
	rule_t *r2;
	rule_t rnew;
	rulecl_t *rcl;

	if (argument[0] == '\0') {
		act_char("Usage: name <word>", ch);
		return FALSE;
	}

	EDIT_RULE(ch, r);
	EDIT_RCL(ch, rcl);
	if (olced_busy(ch, ED_EXPL, NULL, rcl))
		return FALSE;

	if ((r2 = erule_lookup(rcl, argument)) && r2 != r) {
		act_puts("RuleEd: $t: duplicate name.",
			 ch, argument, NULL,
			 TO_CHAR | ACT_NOTRANS, POS_DEAD);
		return FALSE;
	}

	erule_del(rcl, r);
	rule_init(&rnew);
	rnew.name = str_dup(argument);
	ch->desc->pEdit = erule_add(rcl, &rnew);
	return TRUE;
}

OLC_FUN(eruleed_auto)
{
	rule_t *r;
	rulecl_t *rcl;
	rule_t *impl;
	const char **p;

	EDIT_RULE(ch, r);
	EDIT_RCL(ch, rcl);

	/*
	 * lookup implicit rule
	 */
	if ((impl = irule_find(rcl, r->name)) == NULL) {
		act_char("No matching implicit rules found.", ch);
		return FALSE;
	}

	r->arg = strlen(r->name) + impl->arg;
	c_erase(&r->forms);
	C_FOREACH (const char **, p, &impl->forms) {
		const char **q = varr_enew(&r->forms);
		*q = str_qdup(*p);
	}
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
		act_puts("RuleEd: $t: not prefix of name ($T).",
			 ch, arg, r->name, TO_CHAR | ACT_NOTRANS, POS_DEAD);
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

/******************************************************************************
 * local functions
 */

static void
rule_save(FILE *fp, rule_t *r)
{
	const char **p;

	fprintf(fp, "#RULE\n"
		    "Name %s~\n", r->name);
	if (r->arg)
		fprintf(fp, "BaseLen %d\n", r->arg);

	C_FOREACH (const char **, p, &r->forms) {
		if (IS_NULLSTR(*p))
			continue;
		fprintf(fp, "Form %d %s~\n", varr_index(&r->forms, p), *p);
	}

	fprintf(fp, "End\n\n");
}

static void
rcl_save_expl(CHAR_DATA *ch, lang_t *l, rulecl_t *rcl)
{
	FILE *fp;
	rule_t *r;

	if (!IS_SET(rcl->rcl_flags, RULES_EXPL_CHANGED))
		return;

	if (IS_NULLSTR(rcl->file_expl)) {
		printlog(LOG_INFO, "rcl_save_expl: lang %s, rcl %s: NULL file name",
			l->name, flag_string(rulecl_names, rcl->rulecl));
		REMOVE_BIT(rcl->rcl_flags, RULES_EXPL_CHANGED);
		return;
	}

	if ((fp = olc_fopen(LANG_PATH, rcl->file_expl, ch, -1)) == NULL)
		return;

	C_FOREACH (rule_t *, r, &rcl->expl)
		rule_save(fp, r);

	fprintf(fp, "#$\n");
	fclose(fp);

	olc_printf(ch, "Explicit rules (%s%c%s) saved "
		       "(lang '%s', rules type '%s').",
		   LANG_PATH, PATH_SEPARATOR, rcl->file_expl,
		   l->name, flag_string(rulecl_names, rcl->rulecl));
	REMOVE_BIT(rcl->rcl_flags, RULES_EXPL_CHANGED);
}

static void
rcl_save_impl(CHAR_DATA *ch, lang_t *l, rulecl_t *rcl)
{
	rule_t *r;
	FILE *fp;

	if (!IS_SET(rcl->rcl_flags, RULES_IMPL_CHANGED))
		return;

	if (IS_NULLSTR(rcl->file_expl)) {
		printlog(LOG_INFO, "rcl_save_impl: lang %s, rcl %s: NULL file name",
			l->name, flag_string(rulecl_names, rcl->rulecl));
		REMOVE_BIT(rcl->rcl_flags, RULES_IMPL_CHANGED);
		return;
	}

	if ((fp = olc_fopen(LANG_PATH, rcl->file_impl, ch, -1)) == NULL)
		return;

	C_FOREACH (rule_t *, r, &rcl->impl)
		rule_save(fp, r);

	fprintf(fp, "#$\n");
	fclose(fp);

	olc_printf(ch, "Implicit rules (%s%c%s) saved "
		       "(lang '%s', rules type '%s').",
		   LANG_PATH, PATH_SEPARATOR, rcl->file_impl,
		   l->name, flag_string(rulecl_names, rcl->rulecl));
	REMOVE_BIT(rcl->rcl_flags, RULES_IMPL_CHANGED);
}
