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
 * $Id: lang.c,v 1.44 2004-06-28 19:21:10 tatyana Exp $
 */

#include <string.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include <merc.h>
#include <lang.h>

/*----------------------------------------------------------------------------
 * main language support functions
 */

static const char*
word_form_lookup(lang_t *l, rulecl_t *rcl, const char *word, uint fnum)
{
	rule_t *rule;
	char **p;
	char *q;
	static char buf[MAX_STRING_LENGTH];

	if (!fnum || IS_NULLSTR(word))
		return word;

	/*
	 * variable part(s) of word can be specified by tildes
	 * (simple recursion)
	 */
	if ((q = strchr(word, '~'))) {
		char buf2[MAX_STRING_LENGTH];
		char buf3[MAX_STRING_LENGTH];
		char *r;

		/* copy prefix */
		strlncpy(buf2, word, sizeof(buf2), q-word);

		/*
		 * translate infix, translation must be done
		 * before copying the result to buf[] because buf is
		 * static
		 */
		r = strchr(q+1, '~');
		if (!r)
			r = strchr(q+1, '\0');
		strlncpy(buf3, q+1, sizeof(buf3), *r ? r-q-1 : r-q);
		strlcat(buf2, word_form_lookup(l, rcl, buf3, fnum),
		    sizeof(buf2));

		/* translate the rest */
		if (!*r) {
			strlcpy(buf, buf2, sizeof(buf));
			return buf;
		}

		if (strchr(r+1, '~')) {
			strlcpy(buf3, word_form_lookup(l, rcl, r+1, fnum),
			    sizeof(buf3));
			q = buf3;
		}
		else
			q = r+1;

		strlcpy(buf, buf2, sizeof(buf));
		strlcat(buf, q, sizeof(buf));

		return buf;
	}

	/*
	 * explicit rule lookup
	 */
	if ((rule = erule_lookup(rcl, word)) == NULL
	||  (p = varr_get(&rule->forms, fnum)) == NULL
	||  IS_NULLSTR(*p))
		return word;

	if (rule->arg <= 0 || **p != '-')
		return *p;

	strlcpy(buf, word, sizeof(buf));
	strlcpy(buf + rule->arg, *p + 1, sizeof(buf) - rule->arg);
	return buf;
}

const char *
word_form(const char *word, uint fnum, size_t lang, int rulecl)
{
	lang_t *l;

	if ((rulecl < 0 || rulecl >= MAX_RULECL)
	||  (l = varr_get(&langs, lang)) == NULL)
		return word;

	switch (rulecl) {
	case RULES_GENDER:
		/*
		 * SEX_NEUTRAL: 0 -> 2
		 * SEX_MALE:    1 -> 0
		 * SEX_FEMALE:  2 -> 1
		 * SEX_PLURAL:  4 -> 3
		 */
		if (fnum == SEX_PLURAL)
			fnum--;
		else
			fnum = (fnum + 2) % 3;
		break;

	case RULES_QTY:
		fnum %= 100;
		if (fnum > 14)
			fnum %= 10;
	}

	return word_form_lookup(l, l->rules + rulecl, word, fnum);
}

avltree_t msgdb;

avltree_info_t c_info_msgdb =
{
	&avltree_ops,

	(e_init_t) mlstr_init,
	(e_destroy_t) mlstr_destroy,

	MT_MLSTR, sizeof(mlstring), ke_cmp_csmlstr,
};

const char *
GETMSG(const char *msg, size_t lang)
{
	mlstring *ml;

	if (lang == 0 || (ml = msg_lookup(msg)) == NULL)
		return msg;
	return mlstr_val(ml, lang);
}

/*----------------------------------------------------------------------------
 * rule_t functions
 */

static varr_info_t c_info_vforms =
{
	&varr_ops,

	(e_init_t) strkey_init,
	(e_init_t) strkey_destroy,

	sizeof(const char *), 4
};

void
rule_init(rule_t *r)
{
	r->name = NULL;
	r->arg = 0;
	c_init(&r->forms, &c_info_vforms);
}

void
rule_destroy(rule_t *r)
{
	free_string(r->name);
	r->name = NULL;
	c_destroy(&r->forms);
}

void
rule_form_add(rule_t *r, size_t fnum, const char *s)
{
	const char **p = varr_touch(&r->forms, fnum);
	free_string(*p);
	*p = str_dup(s);
}

void
rule_form_del(rule_t *r, size_t fnum)
{
	const char **p = varr_get(&r->forms, fnum);
	if (p == NULL)
		return;
	free_string(*p);
	*p = str_empty;
}

/*----------------------------------------------------------------------------
 * implicit rules operations
 */

rule_t *
irule_add(rulecl_t *rcl, rule_t *r)
{
	rule_t *rnew = varr_enew(&rcl->impl);
	*rnew = *r;
	return rnew;
}

rule_t *
irule_insert(rulecl_t *rcl, size_t num, rule_t *r)
{
	rule_t *rnew;
	if (num > c_size(&rcl->impl))
		num = c_size(&rcl->impl);
	rnew = varr_insert(&rcl->impl, num);
	*rnew = *r;
	return rnew;
}

void
irule_del(rulecl_t *rcl, rule_t *r)
{
	varr_edelete(&rcl->impl, r);
}

rule_t *
irule_lookup(rulecl_t *rcl, const char *num)
{
	char *q;
	size_t i;

	if (IS_NULLSTR(num))
		return NULL;

	i = strtoul(num, &q, 0);
	if (*q || i >= c_size(&rcl->impl))
		return NULL;

	return VARR_GET(&rcl->impl, i);
}

rule_t *
irule_find(rulecl_t *rcl, const char *word)
{
	rule_t *r;

	C_FOREACH (rule_t *, r, &rcl->impl) {
		if (r->name[0] == '-'
		&&  !strchr(word, ' ')
		&&  !str_suffix(r->name+1, word))
			return r;
	}

	return NULL;
}

/*----------------------------------------------------------------------------
 * explicit rules operations
 */

rule_t *
erule_add(rulecl_t *rcl, rule_t *r)
{
	rule_t *rnew;

	rnew = c_insert(&rcl->expl, r->name);
	if (rnew == NULL)
		return NULL;
	*rnew = *r;
	return rnew;
}

void
erule_del(rulecl_t *rcl, rule_t *r)
{
	c_delete(&rcl->expl, r);
}

rule_t *
erule_lookup(rulecl_t *rcl, const char *name)
{
	return c_lookup(&rcl->expl, name);
}

/*----------------------------------------------------------------------------
 * rulecl_t functions
 */

static varr_info_t c_info_rules =
{
	&varr_ops,

	(e_init_t) rule_init,
	(e_destroy_t) rule_destroy,

	sizeof(rule_t), 4
};

static avltree_info_t c_avlinfo_rules =
{
	&avltree_ops,

	(e_init_t) rule_init,
	(e_destroy_t) rule_destroy,

	MT_PVOID, sizeof(rule_t), ke_cmp_str
};

static void
rulecl_init(lang_t *l, size_t rulecl)
{
	rulecl_t *rcl = l->rules + rulecl;

	rcl->rulecl = rulecl;
	rcl->file_expl = str_empty;
	rcl->file_impl = str_empty;
	c_init(&rcl->expl, &c_avlinfo_rules);
	c_init(&rcl->impl, &c_info_rules);
	rcl->rcl_flags = 0;
}

/*----------------------------------------------------------------------------
 * lang_t functions
 */
varr langs;

static void
lang_init(lang_t *l)
{
	size_t i;

	l->name = str_empty;
	l->file_name = str_empty;
	l->lang_flags = 0;
	l->slang_of = NULL;

	for (i = 0; i < MAX_RULECL; i++)
		rulecl_init(l, i);
}

varr_info_t c_info_langs =
{
	&varr_ops,

	(e_init_t) lang_init,
	NULL,

	sizeof(lang_t), 2
};

lang_t *
lang_lookup(const char *name)
{
	return lang_nlookup(name, strlen(name));
}

lang_t *
lang_nlookup(const char *name, size_t len)
{
	lang_t *l;

	if (IS_NULLSTR(name))
		return NULL;

	C_FOREACH (lang_t *, l, &langs) {
		if (str_ncmp(l->name, name, len) == 0)
			return l;
	}

	printlog(LOG_ERROR, "lang_lookup: %s: unknown language", name);
	return NULL;
}
