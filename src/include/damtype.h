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
 * $Id: damtype.h,v 1.13 2001-08-19 18:18:40 fjoe Exp $
 */

#ifndef _DAMTYPE_H_
#define _DAMTYPE_H_

/*
 * damage classes
 */
enum {
	/* DAM_WEAPON damtypes */
	DAM_BASH,
	DAM_PIERCE,
	DAM_SLASH,

	/* DAM_MAGIC damtypes */
	DAM_FIRE,
	DAM_COLD,
	DAM_LIGHTNING,
	DAM_ACID,
	DAM_POISON,
	DAM_NEGATIVE,
	DAM_HOLY,
	DAM_ENERGY,
	DAM_MENTAL,
	DAM_DISEASE,
	DAM_LIGHT,
	DAM_CHARM,
	DAM_SOUND,
	DAM_HARM,
	DAM_OTHER,

	/*
	 * material damtypes
	 */
	DAM_IRON,
	DAM_WOOD,
	DAM_SILVER,

	/*
	 * special damtypes (used ONLY as defaults for corresponding
	 * damtype groups)
	 */
	DAM_WEAPON,
	DAM_MAGIC,

	DAM_NONE,	/* should be the last */
};

#define MAX_RESIST		DAM_NONE

#define RES_UNDEF		666

typedef struct damtype_t damtype_t;
struct damtype_t {
	const char *	dam_name;	/* damtype name */
	gmlstr_t	dam_noun;	/* damtype noun */
	int		dam_class;	/* damtype class */
	int		dam_slot;	/* old damtype ROM slot */
};

extern hash_t damtypes;
extern hashdata_t h_damtypes;

void damtype_init(damtype_t *d);
damtype_t *damtype_cpy(damtype_t *dst, const damtype_t *src);
void damtype_destroy(damtype_t *d);

#define damtype_lookup(dn)	((damtype_t*) strkey_lookup(&damtypes, (dn)))
#define damtype_search(dn)	((damtype_t*) strkey_search(&damtypes, (dn)))
const char *	damtype_slot_lookup(int slot);

gmlstr_t *	damtype_noun(const char *dn);
int		damtype_class(const char *dn);

#endif
