/*-
 * Copyright (c) 2002 SoG Development Team
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
 * $Id: fishing.c,v 1.1.2.4 2003-09-30 01:25:20 fjoe Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include "merc.h"

/*
 * salt water fishes
 */
#define	FISH_CODFISH			34432
#define	FISH_HERRING			34433
#define	FISH_SILVERY_SALMON		34434
#define	FISH_BARRACUDA			34435
#define	FISH_MACKEREL			34436
#define	FISH_SWORDFISH			34437
#define	FISH_SKATE			34438

static int sw_fishes[] = {
	FISH_CODFISH,
	FISH_HERRING,
	FISH_SILVERY_SALMON,
	FISH_BARRACUDA,
	FISH_SWORDFISH,
	FISH_SKATE
};
#define SW_FISHES_SZ	(sizeof(sw_fishes) / sizeof(sw_fishes[0]))

/*
 * fresh water fishes
 */
#define	FISH_CRUSIAN			34441
#define	FISH_PERCH			34442
#define	FISH_CARP			34443
#define	FISH_OMUL			34444
#define	FISH_SAZAN			34445
#define	FISH_PIKE			34446
#define	FISH_BREAM			34447
#define	FISH_PIRANHA			34448

static int fw_fishes[] = {
	FISH_CRUSIAN,
	FISH_PERCH,
	FISH_CARP,
	FISH_OMUL,
	FISH_SAZAN,
	FISH_PIKE,
	FISH_BREAM,
	FISH_PIRANHA
};
#define FW_FISHES_SZ	(sizeof(fw_fishes) / sizeof(fw_fishes[0]))

static int fish_gsn_lookup(int vnum);

int fish_vnum_lookup(flag32_t room_flags)
{
	return (room_flags & ROOM_FRESHWATER) ?
	    fw_fishes[number_range(0, FW_FISHES_SZ - 1)] :
	    sw_fishes[number_range(0, SW_FISHES_SZ - 1)];
}

void fish_affect(CHAR_DATA *ch, int fish_vnum)
{
	int gsn, modifier, location;
	AFFECT_DATA af;

	if ((gsn = fish_gsn_lookup(fish_vnum)) < 0) {
		char_puts("Something wrong. Report it to immortals.\n", ch);
		printlog("[*****] BUG: gsn not exists, fish_vnum %d", fish_vnum);
		return;
	}

	if (is_affected(ch, gsn))
		return;

	af.where = TO_AFFECTS;
	af.type	 = gsn;
	af.level = ch->level;
	af.bitvector = 0;

	switch (number_bits(4)) {
	case 0:
		location = APPLY_DEX;
		break;
	case 1:
		location = APPLY_CON;
		break;
	case 2:
		location = APPLY_INT;
		break;
	case 3:
		location = APPLY_WIS;
		break;
	case 4:
		location = APPLY_CHA;
		break;
	case 5:
		location = APPLY_CHA;
		break;
	default:
		location = APPLY_NONE;
		break;
	}

	if (location != APPLY_NONE) {
		af.duration = number_range(5, 15);
		af.location = location;
		while ((modifier = number_range(-3, 3)) == 0)
			;
		af.modifier = - modifier;
		affect_to_char(ch, &af);
	}

	switch (number_bits(3)) {
	case 0:
		location = APPLY_MANA;
		break;
	case 1:
		location = APPLY_HIT;
		break;
	case 2:
	case 3:
		location = APPLY_MOVE;
		break;
	default:
		location = APPLY_NONE;
		break;
	}

	if (location != APPLY_NONE) {
		af.duration = number_range(5, 10);
		af.location = location;
		while ((modifier = number_range(-3, 3)) == 0)
			;
		af.modifier = - modifier * 10;
		affect_to_char(ch, &af);
	}

	switch (number_bits(3)) {
	case 0:
		modifier *= 10;
		location = APPLY_AC;
		break;
	case 1:
		location = APPLY_HITROLL;
		break;
	case 2:
		location = APPLY_DAMROLL;
		break;
	case 3:
		location = APPLY_SAVES;
		break;
	default:
		location = APPLY_NONE;
		break;
	}

	if (location != APPLY_NONE) {
		while ((modifier = number_range(-3, 3)) == 0)
			;
		if (location == APPLY_AC)
			modifier *= 10;
		af.duration = number_range(1, 20);
		af.location = location;
		af.modifier = - modifier;
		affect_to_char(ch, &af);
	}

	switch (number_bits(4)) {
	case 1:
		modifier = -2;
		break;
	case 2:
		modifier = -1;
		break;
	case 3:
		modifier = 1;
		break;
	case 4:
		modifier = 2;
		break;
	default:
		modifier = 0;
		break;
	}

	if (modifier != 0) {
		af.duration = number_range(1, 20);
		af.location = APPLY_LEVEL;
		af.modifier = modifier;
		affect_to_char(ch, &af);
	}
}

struct fish_gsn_t
{
	int	fish_vnum;
	int	*fish_gsn;
};
typedef struct fish_gsn_t fish_gsn_t;

static fish_gsn_t fish_gsn[] =
{
	{ FISH_HERRING,			&gsn_herring		},
	{ FISH_CODFISH,			&gsn_codfish		},
	{ FISH_SILVERY_SALMON,		&gsn_silvery_salmon	},
	{ FISH_BARRACUDA,		&gsn_barracuda		},
	{ FISH_MACKEREL,		&gsn_mackerel		},
	{ FISH_SWORDFISH,		&gsn_swordfish		},
	{ FISH_SKATE,			&gsn_skate		},

	{ FISH_CRUSIAN,			&gsn_crusian		},
	{ FISH_PERCH,			&gsn_perch		},
	{ FISH_CARP,			&gsn_carp		},
	{ FISH_OMUL,			&gsn_omul		},
	{ FISH_SAZAN,			&gsn_sazan		},
	{ FISH_PIKE,			&gsn_pike		},
	{ FISH_BREAM,			&gsn_bream		},
	{ FISH_PIRANHA,			&gsn_piranha		},
};
#define FISH_GSN_SZ	(sizeof(fish_gsn) / sizeof(fish_gsn[0]))

static int fish_gsn_lookup(int vnum)
{
	static int initialized;
	fish_gsn_t *fg;

	if (!initialized) {
		qsort(fish_gsn, FISH_GSN_SZ, sizeof(fish_gsn[0]), cmpint);
		initialized = 1;
	}

	fg = bsearch(&vnum, fish_gsn, FISH_GSN_SZ, sizeof(fish_gsn[0]), cmpint);
	if (fg != NULL)
		return *fg->fish_gsn;

	return -1;
}
