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
 * $Id: adjust.h,v 1.2 2001-10-21 21:34:01 fjoe Exp $
 */

struct mob_characteristics {
	int	level;
	int	hitroll;
	int	hit[3];
	int	mana[3];
	int	damage[3];
	int	ac[4];
	int	size;
	int16_t	resists[MAX_RESIST];
};

typedef struct mob_characteristics MOB_CHARACTERISTICS;

void
get_mob_characteristics(const MOB_INDEX_DATA *, MOB_CHARACTERISTICS *);

void
set_mob_characteristics(const MOB_CHARACTERISTICS *new_mc, MOB_INDEX_DATA *ind);

bool
adjust_mobile(const MOB_INDEX_DATA *, MOB_CHARACTERISTICS *, int);

struct adjust_data {
	size_t size;
	int *victims;
	int *ops_per_cycle;
	int *ops_performed;
};

int
calc_price(const OBJ_INDEX_DATA *);
