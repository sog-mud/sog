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
 * $Id: class.h,v 1.19 1999-10-06 09:55:57 fjoe Exp $
 */

#ifndef _CLASS_H_
#define _CLASS_H_

/*--------------------------------------------------------------------
 * class stuff
 */
struct class_t {
	const char *	name;		/* full name */
	const char *	file_name;
	char		who_name[4];	/* three-letter name for 'who' */
	int		attr_prime;	/* primary stat */
	int		weapon;		/* school weapon vnum */
	varr 		guilds;		/* guild room list */
	int		thac0_00;	/* thac0 for level 0 */
	int		thac0_32;	/* thac0 for level 32 */
	int		hp_rate;	/* hp rate (when gaining level) */
	int		mana_rate;	/* mana rate (when gaining level */
	flag32_t	class_flags;	/* class flags */
	int		points;			/* cost in exp */
	int		stats[MAX_STATS];	/* stat modifiers */
	flag32_t	restrict_align;		/* alignment restrictions */
	flag32_t	restrict_sex;		/* sex restrictions */
	flag32_t	restrict_ethos;		/* ethos restrictions */
	const char *	titles[MAX_LEVEL+1][2];	/* titles */
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

extern varr classes;

#define CLASS(i)		((class_t*) VARR_GET(&classes, i))
#define class_lookup(i)		((class_t*) varr_get(&classes, i))

class_t *	class_new(void);
void		class_free(class_t*);
const char *	class_name(CHAR_DATA *ch);
const char *	class_who_name(CHAR_DATA *ch);

int		cn_lookup(const char *name);
const char *	title_lookup(CHAR_DATA *ch);

#endif

