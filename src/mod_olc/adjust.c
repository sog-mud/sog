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
 * $Id: adjust.c,v 1.1 2001-09-25 11:49:11 kostik Exp $
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "merc.h"
#include "adjust.h"

#define TOO_MUCH (MAX_LEVEL * 666)
void
get_mob_characteristics(const MOB_INDEX_DATA *ind, MOB_CHARACTERISTICS *mc)
{
	int i;
	mc->level	= ind->level;
	mc->hitroll	= ind->hitroll;
	mc->size	= ind->size;

	mc->hit[0]	= ind->hit[0];
	mc->hit[1]	= ind->hit[1];
	mc->hit[2]	= ind->hit[2];

	mc->mana[0]	= ind->mana[0];
	mc->mana[1]	= ind->mana[1];
	mc->mana[2]	= ind->mana[2];

	mc->damage[0]	= ind->damage[0];
	mc->damage[1]	= ind->damage[1];
	mc->damage[2]	= ind->damage[2];

	mc->ac[0]	= ind->ac[0];
	mc->ac[1]	= ind->ac[1];
	mc->ac[2]	= ind->ac[2];
	mc->ac[3]	= ind->ac[3];

	for (i = 0; i < MAX_RESIST; i++)
		mc->resists[i] = ind->resists[i];
}

void
set_mob_characteristics(const MOB_CHARACTERISTICS *mc, MOB_INDEX_DATA *ind)
{
	int i;
	ind->level	= mc->level;
	ind->hitroll	= mc->hitroll;
	ind->size	= mc->size;

	ind->hit[0]	= mc->hit[0];
	ind->hit[1]	= mc->hit[1];
	ind->hit[2]	= mc->hit[2];

	ind->mana[0]	= mc->mana[0];
	ind->mana[1]	= mc->mana[1];
	ind->mana[2]	= mc->mana[2];

	ind->damage[0]	= mc->damage[0];
	ind->damage[1]	= mc->damage[1];
	ind->damage[2]	= mc->damage[2];

	ind->ac[0]	= mc->ac[0];
	ind->ac[1]	= mc->ac[1];
	ind->ac[2]	= mc->ac[2];
	ind->ac[3]	= mc->ac[3];

	for (i = 0; i < MAX_RESIST; i++)
		ind->resists[i] = mc->resists[i];
}

static
int
get_approx_hp(int level)
{
	return ((5 * level + 52) * level + 6) / 3;
}

static int
get_approx_mana(int level)
{
	return ((6 * level + 52) * level + 6) / 3;
}

static int
get_approx_damage(int level)
{
	return ((level + 14) * level + 50) / 25;
}

static int *
get_approx_range(int level, int *range, int (*get_approx)(int))
{
	int approx = get_approx(level);
	range[0] = UMAX(approx * 4 / 5, 1);
	range[1] = approx * 6 / 5;
	return range;
}

#if 0
static int *
get_hp_range(int level, int *range)
{
	return get_approx_range(level, range, get_approx_hp);
}

static int *
get_dam_range(int level, int *range)
{
	return get_approx_range(level, range, get_approx_damage);
}
#endif

static int *
get_range(const int *rdice, int *range)
{
	range[0] = rdice[DICE_NUMBER] + rdice[DICE_BONUS];
	range[1] = rdice[DICE_NUMBER] * rdice[DICE_TYPE] + rdice[DICE_BONUS];
	return range;
}

static bool
in_range(const int *max_range, const int *range)
{
	return (max_range[0] <= range[0]) && (max_range[1] >= range[1]);
}

#if 0
struct adjust_data {
	size_t size;
	int *victims;
	int *ops_per_cycle;
	int *ops_performed;
};
#endif

static int *curr_array;
static int
compare_func(const void *arg1, const void *arg2)
{
	int first = curr_array[*(const int *)arg1];
	int second = curr_array[*(const int *)arg2];

	if (first == 0)
		first = 1;
	else if (first < 0)
		first = -first;

	if (second == 0)
		second = 1;
	else if (second < 0)
		second = -second;

	return second - first;
}

static void
adjustdata_init(int size, int *values, struct adjust_data *data)
{
	int i;
	data->size		= size;
	data->victims		= malloc(size * sizeof(int));
	data->ops_per_cycle	= malloc(size * sizeof(int));
	data->ops_performed	= calloc(size, sizeof(int));

	for (i = 0; i < size; i++)
		data->victims[i] = i;

	curr_array = values;
	qsort(data->victims, size, sizeof(int), compare_func);

	for (i = 0; i < size; i++) {
		data->ops_per_cycle[i] = values[data->victims[i]];
		if (data->ops_per_cycle[i] == 0)
			data->ops_per_cycle[i] = 1;
		else if (data->ops_per_cycle[i] < 0)
			data->ops_per_cycle[i] = -data->ops_per_cycle[i];
	}
	for (i = 0; i < size; i++)
		data->ops_per_cycle[i] = data->ops_per_cycle[i]
		    / data->ops_per_cycle[size-1];
}

static int
next_victim(struct adjust_data *data)
{
	unsigned i;
	for (i = 0; i < data->size-1; i++) {
		if (data->ops_performed[i] == data->ops_per_cycle[i])
			continue;
		if (data->ops_performed[i] * data->ops_per_cycle[i+1] <=
		    data->ops_performed[i+1] * data->ops_per_cycle[i]) {
			data->ops_performed[i]++;
			return data->victims[i];
		}
	}
	memset(data->ops_performed, 0, data->size * sizeof(int));
	return data->victims[data->size - 1];
}

static void
adjustdata_free(struct adjust_data *data)
{
	free(data->victims);
	free(data->ops_per_cycle);
	free(data->ops_performed);
}

static int
dice_ave(const int *adice)
{
	return adice[DICE_BONUS] + adice[DICE_TYPE] * adice[DICE_NUMBER] / 2;
}

static bool
adjust_dice(const int *old_dice, int old_level, int *new_dice, int new_level,
	   int (*get_approx)(int))
{
	int range[2];
	int tmp_range[2];
	bool changed = FALSE;
	if (get_range(old_dice, range)[0] == 0) {
		/* Completely uninitialized dice */
		changed = TRUE;
		get_approx_range(new_level, range, get_approx);
		new_dice[DICE_BONUS]	= range[0];
		new_dice[DICE_TYPE]	= 1;
		new_dice[DICE_NUMBER]	= 1;
		/* some randomness here */
		while ((dice(new_dice[DICE_NUMBER], new_dice[DICE_TYPE])
		    + new_dice[DICE_BONUS]) < get_approx(new_level) &&
		    in_range(range, get_range(new_dice, tmp_range))) {
			if (number_range(1, 20) > 10)
				new_dice[DICE_NUMBER]++;
			else
				new_dice[DICE_TYPE]++;
		}
	} else if (old_level != new_level) {
		int old_ave = dice_ave(old_dice);
		int new_ave;
		new_ave = old_ave * get_approx(new_level) /
		    get_approx(old_level);
		if (old_level > new_level) {
			struct adjust_data d;
			adjustdata_init(3, new_dice, &d);
			while (new_ave < dice_ave(new_dice)) {
				int vict = next_victim(&d);
				new_dice[vict]--;
			}
			adjustdata_free(&d);
		} else {
			struct adjust_data d;
			adjustdata_init(3, new_dice, &d);
			while (new_ave > dice_ave(new_dice)) {
				new_dice[next_victim(&d)]++;
			}
			adjustdata_free(&d);
		}
	}

	while (!in_range(range, get_range(new_dice, tmp_range))) {
		struct adjust_data ad;
		changed = TRUE;
		/* It's essential here that DAM_BONUS==2 (is last) */
		adjustdata_init(2, new_dice, &ad);
		while (tmp_range[0] < range[0] && tmp_range[1] > range[1]) {
			new_dice[next_victim(&ad)]--;
			get_range(new_dice, tmp_range);
		}
		adjustdata_free(&ad);
		adjustdata_init(3, new_dice, &ad);
		while (tmp_range[0] < range[0]) {
			new_dice[next_victim(&ad)]++;
			get_range(new_dice, tmp_range);
		}

		while (tmp_range[1] > range[1]) {
			new_dice[next_victim(&ad)]--;
			get_range(new_dice, tmp_range);
		}
		adjustdata_free(&ad);
	}

	return changed;
}

bool
adjust_mobile(const MOB_INDEX_DATA *ind, MOB_CHARACTERISTICS *mc,
	      int new_level)
{
	bool changed = FALSE;
	get_mob_characteristics(ind, mc);

	changed = adjust_dice(ind->hit, ind->level, mc->hit, new_level,
	    get_approx_hp);

	changed = adjust_dice(ind->mana, ind->level, mc->mana, new_level,
	    get_approx_mana) || changed;

	changed = adjust_dice(ind->damage, ind->level, mc->damage, new_level,
	    get_approx_damage) || changed;

	return changed;
}

typedef struct flag_cost {
	flag_t	flag;
	int	cost;
} flag_cost;

static flag_cost affect_costs[] = {
	{ AFF_SANCTUARY,	TOO_MUCH	},
	{ AFF_BLACK_SHROUD,	TOO_MUCH	},
	{ AFF_PROTECT_EVIL,	TOO_MUCH	},
	{ AFF_PROTECT_GOOD,	TOO_MUCH	},
	{ AFF_FLYING,		MAX_LEVEL	},
	{ AFF_PASS_DOOR,	MAX_LEVEL	},
	{ AFF_HASTE,		MAX_LEVEL	},
	{ AFF_BERSERK,		MAX_LEVEL	},
	{ AFF_REGENERATION,	MAX_LEVEL	},
	{ 0,			0		}
};

static flag_cost detect_costs[] = {
	{  ID_INVIS,		MAX_LEVEL	},
	{  ID_HIDDEN,		MAX_LEVEL	},
	{  ID_FADE,		MAX_LEVEL	},
	{  ID_IMP_INVIS,	MAX_LEVEL	},
	{  ID_BLEND,		MAX_LEVEL	},
	{  ID_CAMOUFLAGE,	MAX_LEVEL	},
	{  ID_SNEAK,		MAX_LEVEL	},
	{  ID_INFRARED,		MAX_LEVEL / 2	},
	{  ID_UNDEAD,		MAX_LEVEL / 3	},
	{  ID_LIFE,		MAX_LEVEL / 2	},
	{  ID_EVIL,		10		},
	{  ID_MAGIC,		10		},
	{  ID_GOOD,		10		},
	{  ID_TRUESEEING,	TOO_MUCH	},
	{  ID_CHARM,		5		},
	{ 0,			0		}
};

static const flag_cost invis_costs[] = {
	{ ID_INVIS,		MAX_LEVEL	},
	{ ID_HIDDEN,		TOO_MUCH	},
	{ ID_FADE,		TOO_MUCH	},
	{ ID_IMP_INVIS,		MAX_LEVEL	},
	{ ID_BLEND,		TOO_MUCH	},
	{ ID_CAMOUFLAGE,	TOO_MUCH	},
	{ ID_SNEAK,		MAX_LEVEL*3/2	},

	{ 0,			0		}
};

typedef struct apply_cost {
	int apply;
	int cost;
} apply_cost;

static apply_cost apply_costs[] = {
	{ APPLY_STR,		9		},
	{ APPLY_DEX,		9		},
	{ APPLY_INT,		9		},
	{ APPLY_WIS,		9		},
	{ APPLY_CON,		9		},
	{ APPLY_CHA,		9		},
	{ APPLY_LEVEL,		MAX_LEVEL * 2	},
	{ APPLY_MANA,		1		},
	{ APPLY_HIT,		2		},
	{ APPLY_MOVE,		1		},
	{ APPLY_AC,		1		},
	{ APPLY_HITROLL,	6		},
	{ APPLY_DAMROLL,	10		},
	{ APPLY_SAVES,		-10		},
	{ APPLY_SAVING_PARA,	-10		},
	{ APPLY_SAVING_ROD,	-10		},
	{ APPLY_SAVING_PETRI,	-10		},
	{ APPLY_SAVING_BREATH,	-10		},
	{ APPLY_SAVING_SPELL,	-10		},
	{ APPLY_LUCK,		-10		},

	{ APPLY_NONE,		0		}
};

static int
get_flags_cost(flag_t flags, const flag_cost *costs)
{
	const flag_cost *i;
	int cost = 0;

	for (i = costs; i->flag; i++) {
		if (flags & i->flag) {
			if (cost == TOO_MUCH)
				return TOO_MUCH;
			cost += i->cost;
		}
	}

	return cost;
}
static flag_cost wear_costs[] = {
	{ ITEM_WEAR_FINGER,	2 },
	{ ITEM_WEAR_NECK,	3 },
	{ ITEM_WEAR_BODY,	8 },
	{ ITEM_WEAR_HEAD,	4 },
	{ ITEM_WEAR_LEGS,	5 },
	{ ITEM_WEAR_FEET,	3 },
	{ ITEM_WEAR_HANDS,	3 },
	{ ITEM_WEAR_ARMS,	4 },
	{ ITEM_WEAR_SHIELD,	6 },
	{ ITEM_WEAR_ABOUT,	4 },
	{ ITEM_WEAR_WAIST,	4 },
	{ ITEM_WEAR_WRIST,	2 },
	{ ITEM_WIELD,		4 },
	{ ITEM_HOLD,		3 },
	{ ITEM_WEAR_FLOAT,	1 },
	{ ITEM_WEAR_FACE,	2 },

	{ 0,			0 }
};

static flag_cost weapon_costs[] = {
	{ WEAPON_FLAMING,	11	},
	{ WEAPON_FROST,		7	},
	{ WEAPON_VAMPIRIC,	10	},
	{ WEAPON_SHARP,		15	},
	{ WEAPON_VORPAL,	14	},
	{ WEAPON_TWO_HANDS,	-14	},
	{ WEAPON_SHOCKING,	7	},
	{ WEAPON_POISON,	8	},
	{ WEAPON_HOLY,		3	},
	{ WEAPON_KATANA,	0	},
	{ WEAPON_THROW,		14	},
	{ WEAPON_NO_OFFHAND,	-5	},

	{ 0,			0	}
};


int
calc_price(const OBJ_INDEX_DATA *obj)
{
	const AFFECT_DATA *paf;
	int cost = 0;
	int wear_coeff = 10;

	if (obj->item_type == ITEM_LIGHT) {
		wear_coeff = 1;
	} else {
		const flag_cost *wcp;
		for (wcp = wear_costs; wcp->flag; wcp++)
			if (obj->wear_flags & wcp->flag)
				wear_coeff = UMIN(wear_coeff, wcp->cost);
	}

	/* Calculate affect cost */
	for (paf = obj->affected; paf; paf = paf->next) {
		int int_loc;
		const apply_cost *acp;

		switch (paf->where) {
		case TO_RACE:
			cost += MAX_LEVEL * wear_coeff;
			continue;
		case TO_SKILLS:
			cost += paf->modifier * UMAX(2, wear_coeff - 2);
			continue;
		case TO_RESISTS:
		case TO_FORMRESISTS:
			int_loc = INT(paf->location);
			if (int_loc <= DAM_SLASH)
				cost += paf->modifier * 3 *
				    UMAX(3, wear_coeff - 3);
			else if (int_loc == DAM_WEAPON)
				cost += paf->modifier * 9 *
				    UMAX(3, wear_coeff - 3);
			else if (int_loc == DAM_MAGIC)
				cost += paf->modifier * 15 *
				    UMAX(2, wear_coeff - 2);
			else
				cost += paf->modifier *
				    UMAX(3, wear_coeff - 3);
			continue;

		case TO_AFFECTS:
		case TO_FORMAFFECTS:
			cost += get_flags_cost(paf->bitvector, affect_costs) *
			    wear_coeff;
			break;
		case TO_DETECTS:
			cost += get_flags_cost(paf->bitvector, detect_costs) *
			    wear_coeff;
			break;
		case TO_INVIS:
			cost += get_flags_cost(paf->bitvector, invis_costs) *
			    wear_coeff;
			break;
		case TO_FORM:
			cost += MAX_LEVEL * wear_coeff;
			continue;
		}

		int_loc = INT(paf->location);

		/* Handle damroll/hitroll on weapons differently */
		if ((int_loc == APPLY_HITROLL || int_loc == APPLY_DAMROLL)
		&& obj->item_type == ITEM_WEAPON) {
			cost += paf->modifier;
		} else {
			for (acp = apply_costs; acp->apply; acp++)
				if (acp->apply == int_loc) {
					cost += acp->cost * paf->modifier / 2;
					break;
				}
		}
	}

	if (obj->item_type == ITEM_WEAPON) {
		cost += (GET_WEAPON_AVE(obj) - 6) * 3;
		cost += get_flags_cost(INT(obj->value[4]), weapon_costs);
	}

	/* Decrease cost for limited items */
	if (obj->limit == 1) {
		cost = cost * 4 / 5;
	} else if (obj->limit == 2) {
		cost = cost * 9 / 10;
	} else if (obj->limit == 3) {
		cost = cost * 19 / 20;
	} else if (obj->limit == -1) {
		cost = cost * 3 / 2;
	}

	if (IS_OBJ_STAT(obj, ITEM_ANTI_EVIL))
		cost = cost * 19 / 20;
	if (IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL))
		cost = cost * 19 / 20;
	if (IS_OBJ_STAT(obj, ITEM_ANTI_GOOD))
		cost = cost * 19 / 20;

	return cost * 2 / wear_coeff;
}
