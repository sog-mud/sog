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
 * $Id: lang.h,v 1.23 2001-08-02 18:19:52 fjoe Exp $
 */

#ifndef _LANG_H_
#define _LANG_H_

/*
 * language support with creating forms of words
 * (by gender, case or quantity)
 */

const char *	word_form(const char* word, uint fnum, size_t lang, int rulecl);

/*
 * rule_t: rules can be of two types
 *	- explicit rule -- shows how to create word forms explicitly
 *	- implicit rule -- shows how to create explicit rules
 *
 * explicit rules are used in word_form_lookup to build word forms
 * if explicit rules are not found then word_form_lookup
 * tries to use implicit rules to build explicit rules
 */
struct rule_t {
	const char *	name;
	varr		forms;
	int		arg;		/* length of base for explicit rules */
					/* offset (always < 0) from the end  */
					/* of the word for implicit rules    */
};

void	rule_init(rule_t*);
void	rule_destroy(rule_t*);

void	rule_form_add	(rule_t *r, size_t fnum, const char *s);
void	rule_form_del	(rule_t *r, size_t fnum);

#define MAX_RULE_HASH	256

/*
 * rule class - just hash of explicit rules with set of implicit rules
 */
struct rulecl_t {
	int rulecl;			/* rulecl number		*/
	const char *file_expl;		/* explicit rules file		*/
	const char *file_impl;		/* implicit rules file		*/
	varr expl[MAX_RULE_HASH];	/* explicit rules (hashed)	*/
	varr impl;			/* implicit rules		*/
	flag_t rcl_flags;
};

#define RULES_EXPL_CHANGED (Y)	/* explicit rules changed */
#define RULES_IMPL_CHANGED (Z)	/* implicit rules changed */

/*
 * rule operations
 */
rule_t *	irule_add	(rulecl_t *rcl, rule_t *r);
rule_t *	irule_insert	(rulecl_t *rcl, size_t num, rule_t *r);
void		irule_del	(rulecl_t *rcl, rule_t *r);
rule_t *	irule_lookup	(rulecl_t *rcl, const char *num);
rule_t *	irule_find	(rulecl_t *rcl, const char *word);

rule_t *	erule_add	(rulecl_t *rcl, rule_t *r);
void		erule_del	(rulecl_t *rcl, rule_t *r);
rule_t *	erule_lookup	(rulecl_t *rcl, const char *name);

/*
 * at this time three rules classes are supported
 */
enum {
	RULES_CASE,		/* rules for cases forms	*/
	RULES_GENDER,		/* rules for gender forms	*/
	RULES_QTY,		/* rules for quantity forms	*/

	MAX_RULECL
};

struct lang_t {
	const char *	name;
	const char *	file_name;

	flag_t		lang_flags;
	lang_t *	slang_of;

	rulecl_t	rules[MAX_RULECL];
};

#define LANG_HIDDEN		(A)
#define LANG_CHANGED		(Z)

extern varr	langs;

void	lang_init(lang_t *);

lang_t	*lang_lookup(const char*);
lang_t	*lang_nlookup(const char*, size_t len);

extern hash_t msgdb;		/* msgdb */
#define msg_lookup(m)	((mlstring *) strkey_lookup(&msgdb, (m)))

#if !defined(MPC)
const char *GETMSG	(const char *txt, size_t lang);
#else
#define	GETMSG(txt, lang)	(txt)
#endif

#endif
