/*-
 * Copyright (c) 1998 kostik <kostik@iclub.nsu.ru>
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
 */

#ifndef _FORMS_H_
#define _FORMS_H_

#define FORM_NOCAST		(A)	/* can't cast spells at all */
#define FORM_CASTSELF		(B)	/* can only cast spells on themself */
#define FORM_NOSPEAK		(C)	/* cannot speak */
#define FORM_NOGET		(D)	/* can't take anything */

struct form_index_t
{
	const char *	name;		/* name of the form		*/
	flag_t		flags;
	mlstring	description;	/* description of the form	*/
	mlstring	short_desc;
	mlstring	long_desc;
	const char * 	damtype;
	int 		damage[3];
	int		hitroll;
	int		damroll;
	int		num_attacks;	/* Number of attacks		*/
	const char *	skill_spec;	/* Skill spec for this form	*/
	int		stats[MAX_STAT];
	int16_t		resists[MAX_RESIST];
};

struct form_t {
	form_index_t * 	index;
	int 		damroll;
	int		hitroll;
	int16_t 	resists[MAX_RESIST];
};
	
extern hash_t forms;

#define form_lookup(fn)		((form_index_t*) strkey_lookup(&forms, (fn)))
#define form_search(fn)		((form_index_t*) strkey_search(&forms, (fn)))

void form_init(form_index_t *f);
form_index_t* form_cpy(form_index_t *dst, form_index_t *src);
void form_destroy(form_index_t *f);

bool shapeshift(CHAR_DATA *ch, const char *form);
bool revert(CHAR_DATA *ch);

#endif
