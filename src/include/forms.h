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
 * $Id: forms.h,v 1.11 2001-11-21 14:33:21 kostik Exp $
 */

#ifndef _FORMS_H_
#define _FORMS_H_

#define FORM_NOCAST		(A)	/* can't cast spells at all */
#define FORM_CASTSELF		(B)	/* can only cast spells on themself */
#define FORM_NOSPEAK		(C)	/* cannot speak */
#define FORM_NOGET		(D)	/* can't take anything */
#define FORM_IMMOBILE		(E)	/* can't even move in this form */
#define FORM_NOEMOTE		(F)	/* can't use socials and emote */

struct form_index_t
{
	const char *	name;		/* name of the form		*/
	flag_t		flags;
	mlstring	description;	/* description of the form	*/
	mlstring	short_desc;
	mlstring	long_desc;
	const char *	damtype;
	int		damage[3];
	int		hitroll;
	flag_t		has_invis;
	flag_t		has_detect;
	flag_t		affected_by;
	int		num_attacks;	/* Number of attacks		*/
	const char *	skill_spec;	/* Skill spec for this form	*/
	int		stats[MAX_STAT];
	int16_t		resists[MAX_RESIST];
};

struct form_t {
	form_index_t *	index;
	int		damroll;
	int		hitroll;
	int16_t		res_mod[MAX_RESIST];
};

extern avltree_t forms;
extern avltree_info_t c_info_forms;

#define form_lookup(fn)		((form_index_t*) c_strkey_lookup(&forms, (fn)))
#define form_search(fn)		((form_index_t*) c_strkey_search(&forms, (fn)))

#endif
