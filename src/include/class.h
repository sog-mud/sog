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
 * $Id: class.h,v 1.25 1999-12-16 12:24:44 fjoe Exp $
 */

#ifndef _CLASS_H_
#define _CLASS_H_

/*--------------------------------------------------------------------
 * class stuff
 */
struct class_t {
	const char *	name;		/* full name */
	char		who_name[4];	/* three-letter name for 'who' */
	int		attr_prime;	/* primary stat */
	int		weapon;		/* school weapon vnum */
	varr 		guilds;		/* guild room list */
	int		thac0_00;	/* thac0 for level 0 */
	int		thac0_32;	/* thac0 for level 32 */
	int		hp_rate;	/* hp rate (when gaining level) */
	int		mana_rate;	/* mana rate (when gaining level */
	flag_t		class_flags;	/* class flags */
	int		points;			/* cost in exp */
	int		mod_stat[MAX_STATS];	/* starting stat mod */
	flag_t		restrict_align;		/* alignment restrictions */
	const char *	restrict_sex;		/* sex restrictions */
	flag_t		restrict_ethos;		/* ethos restrictions */
	varr		poses;			/* varr of class poses */
	int		death_limit;		/* death limit */
	const char *	skill_spec;		/* skills spec for this class */
};

struct pose_t {
	const char *	self;		/* what is seen by char */
	const char *	others;		/* what is seen by others */
};

/* class flags */
#define CLASS_MAGIC		(A)	/* magic user */
#define CLASS_NOCH		(B)	/* can't live in common hometowns */
#define CLASS_CHANGED		(Z)	/* OLC internal flag */

extern hash_t classes;

#define class_lookup(cn)	((class_t*) strkey_lookup(&classes, (cn)))
#define class_search(cn)	((class_t*) strkey_search(&classes, (cn)))

#define IS_CLASS(cl1, cl2)	(!str_cmp((cl1), (cl2)))

void	class_init	(class_t *cl);
class_t *class_cpy	(class_t *dst, class_t *src);
void	class_destroy	(class_t *cl);

const char *	class_who_name(CHAR_DATA *ch);
bool		can_flee(CHAR_DATA *ch);

#endif

