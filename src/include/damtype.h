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
 * $Id: damtype.h,v 1.6 1999-12-16 12:24:44 fjoe Exp $
 */

#ifndef _DAMTYPE_H_
#define _DAMTYPE_H_

/*
 * damage classes
 */
#define DAM_NONE		0
#define DAM_BASH		1
#define DAM_PIERCE		2
#define DAM_SLASH		3
#define DAM_FIRE		4
#define DAM_COLD		5
#define DAM_LIGHTNING		6
#define DAM_ACID		7
#define DAM_POISON		8
#define DAM_NEGATIVE		9
#define DAM_HOLY		10
#define DAM_ENERGY		11
#define DAM_MENTAL		12
#define DAM_DISEASE		13
#define DAM_DROWNING		14
#define DAM_LIGHT		15
#define DAM_OTHER		16
#define DAM_HARM		17
#define DAM_CHARM		18
#define DAM_SOUND		19
#define DAM_THIRST		20
#define DAM_HUNGER		21
#define DAM_LIGHT_V		22
#define DAM_TRAP_ROOM		23
#define DAM_WOOD		24
#define DAM_SILVER		25

typedef struct damtype_t damtype_t;
struct damtype_t {
	const char *	dam_name;	/* damtype name */
	gmlstr_t	dam_noun;	/* damtype noun */
	int		dam_class;	/* damtype class */
	int		dam_slot;	/* old damtype ROM slot */
};

extern hash_t damtypes;

void damtype_init(damtype_t *d);
damtype_t *damtype_cpy(damtype_t *dst, damtype_t *src);
void damtype_destroy(damtype_t *d);

#define damtype_lookup(dn)	((damtype_t*) strkey_lookup(&damtypes, (dn)))
const char *	damtype_slot_lookup(int slot);

gmlstr_t *	damtype_noun(const char *dn);
int		damtype_class(const char *dn);

#endif

