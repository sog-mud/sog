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
 * $Id: lang.c,v 1.27 2001-06-21 16:16:59 avn Exp $
 */

#include <string.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "merc.h"
#include "lang.h"
#include "db.h"

static void str_init(const char **p);
static void str_destroy(const char **p);


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
		strnzncpy(buf2, sizeof(buf2), word, (unsigned)(q-word));

		/*
		 * translate infix, translation must be done
		 * before copying the result to buf[] because buf is
		 * static
		 */
		r = strchr(q+1, '~');
		if (!r)
			r = strchr(q+1, '\0');
		strnzncpy(buf3, sizeof(buf3), q+1,
			(unsigned)(*r ? r-q-1 : r-q));
		strnzcat(buf2, sizeof(buf2),
			 word_form_lookup(l, rcl, buf3, fnum));

		/* translate the rest */
		if (!*r) {
			strnzcpy(buf, sizeof(buf), buf2);
			return buf;
		}

		if (strchr(r+1, '~')) {
			strnzcpy(buf3, sizeof(buf3),
				 word_form_lookup(l, rcl, r+1, fnum));
			q = buf3;
		}
		else
			q = r+1;

		strnzcpy(buf, sizeof(buf), buf2);
		strnzcat(buf, sizeof(buf), q);

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

	strnzcpy(buf, sizeof(buf), word);
	strnzcpy(buf + rule->arg, sizeof(buf) - rule->arg, *p + 1);
	return buf;
}

const char *word_form(const char *word, uint fnum, int lang, int rulecl)
{
	lang_t *l;

	if ((rulecl < 0 || rulecl >= MAX_RULECL)
	||  (l = varr_get(&langs, (unsigned)lang)) == NULL)
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

/*----------------------------------------------------------------------------
 * rule_t functions
 */

#define rulehash(s) hashcasestr(s, 16, MAX_RULE_HASH)

/* reverse order (otherwise word_del will not work) */
static int cmprule(const void *p1, const void *p2)
{
	return -str_cmp(((const rule_t*) p1)->name, ((const rule_t*) p2)->name);
}

static void
str_init(const char **p)
{
	*p = str_empty;
}

static void
str_destroy(const char **p)
{
	free_string(*p);
}

static varrdata_t v_forms =
{
	sizeof(char*), 4,
	(e_init_t) str_init,
	(e_init_t) str_destroy,
	NULL
};

void rule_init(rule_t *r)
{
	r->name = NULL;
	r->arg = 0;
	varr_init(&r->forms, &v_forms);
}

void rule_destroy(rule_t *r)
{
	free_string(r->name);
	r->name = NULL;
	varr_destroy(&r->forms);
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
rule_t *irule_add(rulecl_t *rcl, rule_t *r)
{
	rule_t *rnew = varr_enew(&rcl->impl);
	*rnew = *r;
	return rnew;
}

rule_t *irule_insert(rulecl_t *rcl, size_t num, rule_t *r)
{
	rule_t *rnew;
	if (num > rcl->impl.nused)
		num = rcl->impl.nused;
	rnew = varr_insert(&rcl->impl, num);
	*rnew = *r;
	return rnew;
}

void irule_del(rulecl_t *rcl, rule_t *r)
{
	varr_edelete(&rcl->impl, r);
}

rule_t *irule_lookup(rulecl_t *rcl, const char *num)
{
	char *q;
	size_t i;

	if (IS_NULLSTR(num))
		return NULL;

	i = strtoul(num, &q, 0);
	if (*q || i >= rcl->impl.nused)
		return NULL;

	return VARR_GET(&rcl->impl, i);
}

rule_t *irule_find(rulecl_t *rcl, const char *word)
{
	size_t i;

	for (i = 0; i < rcl->impl.nused; i++) {
		rule_t *r = VARR_GET(&rcl->impl, i);
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
rule_t *erule_add(rulecl_t *rcl, rule_t *r)
{
	rule_t *rnew;
	varr *v;

	if (IS_NULLSTR(r->name))
		return NULL;

	v = rcl->expl + rulehash(r->name);
	if (varr_bsearch(v, r, cmprule))
		return NULL;

	rnew = varr_enew(v);
	*rnew = *r;
	varr_qsort(v, cmprule);
	return varr_bsearch(v, r, cmprule);
}

void erule_del(rulecl_t *rcl, rule_t *r)
{
	varr *v = rcl->expl + rulehash(r->name);
	varr_edelete(v, r);
}

rule_t *erule_lookup(rulecl_t *rcl, const char *name)
{
	if (IS_NULLSTR(name))
		return NULL;
	return varr_bsearch(rcl->expl + rulehash(name), &name, cmprule);
}

/*----------------------------------------------------------------------------
 * rulecl_t functions
 */

static varrdata_t v_rule =
{
	sizeof(rule_t), 4,
	(e_init_t) rule_init,
	(e_destroy_t) rule_destroy,
	NULL
};

static void rulecl_init(lang_t *l, int rulecl)
{
	int i;
	rulecl_t *rcl = l->rules + rulecl;

	rcl->rulecl = rulecl;
	rcl->file_expl = str_empty;
	rcl->file_impl = str_empty;
	for (i = 0; i < MAX_RULE_HASH; i++)
		varr_init(rcl->expl+i, &v_rule);
	varr_init(&rcl->impl, &v_rule);
	rcl->rcl_flags = 0;
}

/*----------------------------------------------------------------------------
 * lang_t functions
 */
varr langs;

void lang_init(lang_t *l)
{
	int i;

	l->name = str_empty;
	l->file_name = str_empty;
	l->lang_flags = 0;
	l->slang_of = -1;

	for (i = 0; i < MAX_RULECL; i++)
		rulecl_init(l, i);
}

int lang_lookup(const char *name)
{
	return lang_nlookup(name, strlen(name));
}

int lang_nlookup(const char *name, size_t len)
{
	size_t lang;

	if (IS_NULLSTR(name))
		return -1;

	for (lang = 0; lang < langs.nused; lang++) {
		lang_t *l = VARR_GET(&langs, lang);
		if (str_ncmp(l->name, name, len) == 0)
			return lang;
	}

	log(LOG_ERROR, "lang_lookup: %s: unknown language", name);
	return -1;
}
