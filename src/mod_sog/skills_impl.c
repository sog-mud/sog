/*-
 * Copyright (c) 2001 SoG Development Team
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
 * $Id: skills_impl.c,v 1.4 2001-09-13 12:03:05 fjoe Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include <merc.h>

#include "skills_impl.h"

static
FOREACH_CB_FUN(apply_sa_cb, p, ap)
{
	saff_t *sa = (saff_t *) p;

	skill_t *sk = va_arg(ap, skill_t *);
	int percent = va_arg(ap, int);
	int *pmod = va_arg(ap, int *);

	if ((!IS_SET(sa->bit, SK_AFF_ALL) &&
	     !IS_SKILL(sa->sn, gmlstr_mval(&sk->sk_name)))
	||  (IS_SET(sa->bit, SK_AFF_NOTCLAN) &&
	     IS_SET(sk->skill_flags, SKILL_CLAN))
	||  (!IS_SET(sa->bit, SK_AFF_TEACH) &&
	     !percent))
		return NULL;

	(*pmod) += sa->mod;
	return NULL;
}

/*
 * apply skill affect modifiers
 */
int
get_skill_mod(CHAR_DATA *ch, skill_t *sk, int percent)
{
	int mod = 0;
	c_foreach(&ch->sk_affected, apply_sa_cb, sk, percent, &mod);
	return mod;
}

/*-----------------------------------------------------------------------------
 * mob skills stuff
 */

typedef int MOB_SKILL(const CHAR_DATA *);
#define DECLARE_MOB_SKILL(fun) static MOB_SKILL fun;
#define MOB_SKILL(fun)							\
	static int fun(const CHAR_DATA *mob __attribute__((unused)))

typedef struct mob_skill_t mob_skill_t;
struct mob_skill_t {
	const char *sn;
	MOB_SKILL *fun;
};

DECLARE_MOB_SKILL(mob_track);
DECLARE_MOB_SKILL(mob_sneak);
#define mob_hide mob_sneak
#define mob_pick_lock mob_sneak
DECLARE_MOB_SKILL(mob_backstab);
DECLARE_MOB_SKILL(mob_dual_backstab);
DECLARE_MOB_SKILL(mob_dodge);
DECLARE_MOB_SKILL(mob_parry);
DECLARE_MOB_SKILL(mob_dirt_kicking);
DECLARE_MOB_SKILL(mob_shield_block);
DECLARE_MOB_SKILL(mob_second_attack);
DECLARE_MOB_SKILL(mob_third_attack);
DECLARE_MOB_SKILL(mob_fourth_attack);
DECLARE_MOB_SKILL(mob_hand_to_hand);
DECLARE_MOB_SKILL(mob_trip);
DECLARE_MOB_SKILL(mob_bash);
DECLARE_MOB_SKILL(mob_kick);
DECLARE_MOB_SKILL(mob_critical_strike);
DECLARE_MOB_SKILL(mob_disarm);
DECLARE_MOB_SKILL(mob_grip);
DECLARE_MOB_SKILL(mob_berserk);
#define mob_tiger_power mob_berserk
DECLARE_MOB_SKILL(mob_rescue);
DECLARE_MOB_SKILL(mob_crush);
DECLARE_MOB_SKILL(mob_weapon);
DECLARE_MOB_SKILL(mob_distance);
DECLARE_MOB_SKILL(mob_deathblow);
DECLARE_MOB_SKILL(mob_spellbane);
DECLARE_MOB_SKILL(mob_dual_wield);

static size_t mob_skill_count;

static mob_skill_t mob_skill_tab[] =
{
	{ "track",		mob_track		},	// notrans
	{ "sneak",		mob_sneak		},	// notrans
	{ "hide",		mob_hide		},	// notrans
	{ "pick lock",		mob_pick_lock		},	// notrans
	{ "backstab",		mob_backstab		},	// notrans
	{ "dual backstab",	mob_dual_backstab	},	// notrans
	{ "dodge",		mob_dodge		},	// notrans
	{ "parry",		mob_parry		},	// notrans
	{ "distance",		mob_distance		},	// notrans
	{ "dirt kicking",	mob_dirt_kicking	},	// notrans
	{ "shield block",	mob_shield_block	},	// notrans
	{ "second attack",	mob_second_attack	},	// notrans
	{ "third attack",	mob_third_attack	},	// notrans
	{ "fourth attack",	mob_fourth_attack	},	// notrans
	{ "hand to hand",	mob_hand_to_hand	},	// notrans
	{ "trip",		mob_trip		},	// notrans
	{ "bash",		mob_bash		},	// notrans
	{ "bash door",		mob_bash		},	// notrans
	{ "kick",		mob_kick		},	// notrans
	{ "critical strike",	mob_critical_strike	},	// notrans
	{ "deathblow",		mob_deathblow		},	// notrans
	{ "spellbane",		mob_spellbane		},	// notrans
	{ "disarm",		mob_disarm		},	// notrans
	{ "grip",		mob_grip		},	// notrans
	{ "berserk",		mob_berserk		},	// notrans
	{ "tiger power",	mob_tiger_power		},	// notrans
	{ "dual wield",		mob_dual_wield		},	// notrans
	{ "rescue",		mob_rescue		},	// notrans
	{ "crush",		mob_crush		},	// notrans
	{ "sword",		mob_weapon		},	// notrans
	{ "dagger",		mob_weapon		},	// notrans
	{ "spear",		mob_weapon		},	// notrans
	{ "mace",		mob_weapon		},	// notrans
	{ "axe",		mob_weapon		},	// notrans
	{ "flail",		mob_weapon		},	// notrans
	{ "staff",		mob_weapon		},	// notrans
	{ "whip",		mob_weapon		},	// notrans
	{ "polearm",		mob_weapon		},	// notrans
	{ "bow",		mob_weapon		},	// notrans
	{ "arrow",		mob_weapon		},	// notrans
	{ "lance",		mob_weapon		},	// notrans

	{ NULL, NULL }
};

static void
mob_skill_init(void)
{
	mob_skill_t *mob_skill;

	for (mob_skill = mob_skill_tab; mob_skill->sn; mob_skill++) {
		STRKEY_CHECK(&skills, mob_skill->sn);
		mob_skill_count++;
	}
	qsort(mob_skill_tab, mob_skill_count, sizeof(mob_skill_t), cmpstr);
}

static int
get_minimum_spell_level(int rank)
{
	switch (rank) {
	default:
		return 0;
	case 1:
		return 1;
	case 2:
		return MAX_LEVEL / 10;
	case 3:
		return MAX_LEVEL / 8;
	case 4:
		return MAX_LEVEL / 3;
	case 5:
		return MAX_LEVEL / 2;
	case 6:
		return MAX_LEVEL / 3 * 2;
	case 7:
		return MAX_LEVEL / 4 * 5;
	}
}

int
get_mob_skill(const CHAR_DATA *ch, skill_t *sk)
{
	mob_skill_t *mob_skill;
	const char *sn;

	if (sk->skill_type == ST_SPELL
	||  sk->skill_type == ST_PRAYER) {
		int level_needed = get_minimum_spell_level(sk->rank);
		if (sk->rank == 0)
			return 100;
		return URANGE(0, LEVEL(ch) > level_needed ?
		    75 + (LEVEL(ch) - level_needed) * 2:
		    75 - (LEVEL(ch) - level_needed) * 4, 100);
	}

	if (!mob_skill_count)
		mob_skill_init();

	sn = gmlstr_mval(&sk->sk_name);
	mob_skill = bsearch(&sn, mob_skill_tab, mob_skill_count,
			    sizeof(mob_skill_t), cmpstr);
	if (mob_skill == NULL)
		return 0;
	return mob_skill->fun(ch);
}

MOB_SKILL(mob_track)
{
	return 100;
}

MOB_SKILL(mob_sneak)
{
	if (IS_SET(mob->pMobIndex->act, ACT_THIEF))
		return mob->level * 2 + 20;
	return 0;
}

MOB_SKILL(mob_backstab)
{
	if (IS_SET(mob->pMobIndex->act, ACT_THIEF)
	||  IS_SET(mob->pMobIndex->off_flags, OFF_BACKSTAB))
		return mob->level * 2 + 20;
	return 0;
}

MOB_SKILL(mob_dual_backstab)
{
	if (IS_SET(mob->pMobIndex->act, ACT_THIEF)
	||  IS_SET(mob->pMobIndex->off_flags, OFF_BACKSTAB))
		return mob->level + 20;
	return 0;
}

MOB_SKILL(mob_dodge)
{
	if (IS_SET(mob->pMobIndex->off_flags, OFF_DODGE))
		return UMIN(mob->level, 30);
	return 0;
}

MOB_SKILL(mob_parry)
{
	if (IS_SET(mob->pMobIndex->off_flags, OFF_PARRY))
		return UMIN(mob->level, 35);
	return 0;
}

MOB_SKILL(mob_distance)
{
	if (IS_SET(mob->pMobIndex->off_flags, OFF_DISTANCE))
		return mob->level * 2;
	return 0;
}

MOB_SKILL(mob_deathblow)
{
	if (IS_SET(mob->pMobIndex->off_flags, OFF_DEATHBLOW))
		return 68 + mob->level;
	return 0;
}

MOB_SKILL(mob_spellbane)
{
	if (IS_SET(mob->pMobIndex->off_flags, OFF_SPELLBANE))
		return 65 + mob->level;
	return 0;
}

MOB_SKILL(mob_dual_wield)
{
	return 100;
}

MOB_SKILL(mob_dirt_kicking)
{
	if (IS_SET(mob->pMobIndex->off_flags, OFF_DIRT_KICK))
		return mob->level * 2;
	return 0;
}

MOB_SKILL(mob_shield_block)
{
	return 10;
}

MOB_SKILL(mob_second_attack)
{
	if (IS_SET(mob->pMobIndex->act, ACT_WARRIOR | ACT_THIEF))
		return 10 + 3 * mob->level;
	return 0;
}

MOB_SKILL(mob_third_attack)
{
	if (IS_SET(mob->pMobIndex->act, ACT_WARRIOR))
		return 4 * mob->level - 40;
	return 0;
}

MOB_SKILL(mob_fourth_attack)
{
	if (IS_SET(mob->pMobIndex->act, ACT_WARRIOR))
		return 4 * mob->level - 60;
	return 0;
}

MOB_SKILL(mob_hand_to_hand)
{
	return 40 + 2 * mob->level;
}

MOB_SKILL(mob_trip)
{
	if (IS_SET(mob->pMobIndex->off_flags, OFF_TRIP))
		return 10 + 3 * mob->level;
	return 0;
}

MOB_SKILL(mob_bash)
{
	if (IS_SET(mob->pMobIndex->off_flags, OFF_BASH))
		return 10 + 3 * mob->level;
	return 0;
}

MOB_SKILL(mob_kick)
{
	if (IS_SET(mob->pMobIndex->off_flags, OFF_KICK))
		return 10 + 3 * mob->level;
	return 0;
}

MOB_SKILL(mob_critical_strike)
{
	if (IS_SET(mob->pMobIndex->act, ACT_WARRIOR))
		return mob->level;
	return 0;
}

MOB_SKILL(mob_disarm)
{
	if (IS_SET(mob->pMobIndex->off_flags, OFF_DISARM)
	||  IS_SET(mob->pMobIndex->act, ACT_WARRIOR | ACT_THIEF))
		return 20 + 3 * mob->level;
	return 0;
}

MOB_SKILL(mob_grip)
{
	if (IS_SET(mob->pMobIndex->act, ACT_WARRIOR | ACT_THIEF))
		return mob->level;
	return 0;
}

MOB_SKILL(mob_berserk)
{
	if (IS_SET(mob->pMobIndex->off_flags, OFF_BERSERK))
		return 3 * mob->level;
	return 0;
}

MOB_SKILL(mob_rescue)
{
	return 40 + mob->level;
}

MOB_SKILL(mob_crush)
{
	if (IS_SET(mob->pMobIndex->off_flags, OFF_CRUSH))
		return 10 + 3 * mob->level;
	return 0;
}

MOB_SKILL(mob_weapon)
{
	return 40 + 5 * mob->level / 2;
}
