/*
 * $Id: skills.c,v 1.75 1999-10-06 09:56:10 fjoe Exp $
 */

/***************************************************************************
 *     ANATOLIA 2.1 is copyright 1996-1997 Serdar BULUT, Ibrahim CANPUNAR  *
 *     ANATOLIA has been brought to you by ANATOLIA consortium		   *
 *	 Serdar BULUT {Chronos}		bulut@rorqual.cc.metu.edu.tr       *
 *	 Ibrahim Canpunar  {Asena}	canpunar@rorqual.cc.metu.edu.tr    *	
 *	 Murat BICER  {KIO}		mbicer@rorqual.cc.metu.edu.tr	   *	
 *	 D.Baris ACAR {Powerman}	dbacar@rorqual.cc.metu.edu.tr	   *	
 *     By using this code, you have agreed to follow the terms of the      *
 *     ANATOLIA license, in the file Anatolia/anatolia.licence             *	
 ***************************************************************************/

/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,	   *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *									   *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael	   *
 *  Chastain, Michael Quan, and Mitchell Tse.				   *
 *									   *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc	   *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.						   *
 *									   *
 *  Much time and thought has gone into this software and you are	   *
 *  benefitting.  We hope that you share your changes too.  What goes	   *
 *  around, comes around.						   *
 ***************************************************************************/
 
/***************************************************************************
*	ROM 2.4 is copyright 1993-1995 Russ Taylor			   *
*	ROM has been brought to you by the ROM consortium		   *
*	    Russ Taylor (rtaylor@pacinfo.com)				   *
*	    Gabrielle Taylor (gtaylor@pacinfo.com)			   *
*	    Brian Moore (rom@rom.efn.org)				   *
*	By using this code, you have agreed to follow the terms of the	   *
*	ROM license, in the file Rom24/doc/rom.license			   *
***************************************************************************/

#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "db.h"

hash_t skills;

static int get_mob_skill(CHAR_DATA *ch, skill_t *sk);

int base_exp(CHAR_DATA *ch)
{
	int expl;
	class_t *cl;
	race_t *r;
	rclass_t *rcl;

	if (IS_NPC(ch)
	||  (cl = class_lookup(ch->class)) == NULL
	||  (r = race_lookup(PC(ch)->race)) == NULL
	||  !r->race_pcdata
	||  (rcl = rclass_lookup(r, cl->name)) == NULL)
		return 1500;

	expl = 1000 + r->race_pcdata->points + cl->points;
	return expl * rcl->mult/100;
}

int exp_for_level(CHAR_DATA *ch, int level)
{
	int i = base_exp(ch) * level;
	return i + i * (level-1) / 20;
}

/*
 * assumes !IS_NPC(ch)
 */
int exp_to_level(CHAR_DATA *ch)
{ 
	return exp_for_level(ch, ch->level+1) - PC(ch)->exp;
}

/* checks for skill improvement */
void check_improve(CHAR_DATA *ch, const char *sn, bool success, int multiplier)
{
	pc_skill_t *pc_sk;
	int chance;
	spec_skill_t spec_sk;

	if (IS_NPC(ch)
	||  (pc_sk = pc_skill_lookup(ch, sn)) == NULL
	||  pc_sk->percent <= 0)
		return;

	spec_sk.sn = sn;
	spec_stats(ch, &spec_sk);
	if (spec_sk.level > ch->level
	||  pc_sk->percent >= spec_sk.max)
		return;

	/*
	 * check to see if the character has a chance to learn
	 */
	chance = 10 * int_app[get_curr_stat(ch, STAT_INT)].learn;
	chance /= (multiplier *	spec_sk.rating * 4);
	chance += ch->level;

	if (number_range(1, 1000) > chance)
		return;

/* now that the character has a CHANCE to learn, see if they really have */	

	if (success) {
		chance = URANGE(5, spec_sk.max - pc_sk->percent, 95);
		if (number_percent() < chance) {
			pc_sk->percent++;
			gain_exp(ch, 2 * spec_sk.rating);
			if (pc_sk->percent == spec_sk.max) {
				act_puts("{gYou mastered {W$t{g!{x",
					 ch, sn, NULL,
					 TO_CHAR, POS_DEAD);
			} else {
				act_puts("{gYou have become better at {W$t{g!{x",
					 ch, sn, NULL,
					 TO_CHAR, POS_DEAD);
			}
		}
	} else {
		chance = URANGE(5, pc_sk->percent / 2, 30);
		if (number_percent() < chance) {
			pc_sk->percent += number_range(1, 3);
			pc_sk->percent = UMIN(pc_sk->percent, spec_sk.max);
			gain_exp(ch, 2 * spec_sk.rating);
			if (pc_sk->percent == spec_sk.max) {
				act_puts("{gYou learn from your mistakes and you manage to master {W$t{g!{x",
					 ch, sn, NULL,
					 TO_CHAR, POS_DEAD);
			} else {
				act_puts("{gYou learn from your mistakes and your {W$t{g skill improves!{x",
					 ch, sn, NULL,
					 TO_CHAR, POS_DEAD);
			}

		}
	}
}

/*
 * simply adds sn to ch's known skills (if skill is not already known).
 */
void set_skill_raw(CHAR_DATA *ch, const char *sn, int percent, bool replace)
{
	pc_skill_t *pc_sk;

	if (IS_NULLSTR(sn))
		return;

	NAME_CHECK(&skills, sn, "set_skill_raw");

	if ((pc_sk = pc_skill_lookup(ch, sn))) {
		if (replace || pc_sk->percent < percent)
			pc_sk->percent = percent;
		return;
	}
	pc_sk = varr_enew(&PC(ch)->learned);
	pc_sk->sn = str_dup(sn);
	pc_sk->percent = percent;
	varr_qsort(&PC(ch)->learned, cmpstr);
}

void set_skill(CHAR_DATA *ch, const char *sn, int percent)
{
	set_skill_raw(ch, sn, percent, TRUE);
}

/* for returning skill information */
int get_skill(CHAR_DATA *ch, const char *sn)
{
	int skill;
	skill_t *sk;
	AFFECT_DATA *paf;

	int add = 0;
	bool teach = FALSE;

	if ((sk = skill_lookup(sn)) == NULL
	||  (IS_SET(sk->skill_flags, SKILL_CLAN) && !clan_item_ok(ch->clan)))
		return 0;

	if (!IS_NPC(ch)) {
		pc_skill_t *pc_sk;

		if ((pc_sk = pc_skill_lookup(ch, sn)) == NULL
		||  skill_level(ch, sn) > ch->level)
			skill = 0;
		else
			skill = pc_sk->percent;
	} else 
		skill = get_mob_skill(ch, sk);

	if (ch->daze > 0) {
		if (sk->skill_type == ST_SPELL)
			skill /= 2;
		else
			skill = 2 * skill / 3;
	}

	if (!IS_NPC(ch) && PC(ch)->condition[COND_DRUNK]  > 10)
		skill = 9 * skill / 10;

	/*
	 * apply skill affect modifiers
	 */
	for (paf = ch->affected; paf; paf = paf->next) {
		if (paf->where != TO_SKILLS
		||  (!IS_SET(paf->bitvector, SK_AFF_ALL) &&
		     SKILL_IS(paf->type, sn))
		||  (IS_SET(paf->bitvector, SK_AFF_NOTCLAN &&
		     IS_SET(sk->skill_flags, SKILL_CLAN))))
			continue;

		add += paf->modifier;
		teach |= IS_SET(paf->bitvector, SK_AFF_TEACH);
	}

	return UMAX(0, skill ? (skill+add) : teach ? add : 0);
}

static void *
skill_search_cb(void *p, void *d)
{
	skill_t *sk = (skill_t*) p;
	const char *psn = (const char *) d;
	if (!str_prefix(psn, sk->name))
		return p;
	return NULL;
}

/*
 * skill_search -- lookup skill by prefix
 */
skill_t *skill_search(const char *psn)
{
	if (IS_NULLSTR(psn))
		return NULL;
	return hash_foreach(&skills, skill_search_cb, (void*) psn);
}

/*
 * Lookup skill by prefix in varr.
 * First field of structure assumed to be sn
 */
void *skill_vsearch(varr *v, const char *psn)
{
	if (IS_NULLSTR(psn))
		return NULL;
	return varr_foreach(v, skill_search_cb, (void*) psn);
}

/* for returning weapon information */
const char* get_weapon_sn(OBJ_DATA *wield)
{
	if (wield == NULL)
		return "hand to hand";

	if (wield->pObjIndex->item_type != ITEM_WEAPON)
		return NULL;

	switch (INT_VAL(wield->value[0])) {
	default :               return NULL;
	case(WEAPON_SWORD):     return "sword";
	case(WEAPON_DAGGER):    return "dagger";
	case(WEAPON_SPEAR):     return "spear";
	case(WEAPON_MACE):      return "mace";
	case(WEAPON_AXE):       return "axe";
	case(WEAPON_FLAIL):     return "flail";
	case(WEAPON_WHIP):      return "whip";
	case(WEAPON_POLEARM):   return "polearm";
	case(WEAPON_BOW):	return "bow";
	case(WEAPON_ARROW):	return "arrow";
	case(WEAPON_LANCE):	return "lance";
	case(WEAPON_STAFF):	return "staff";
	}
}

int get_weapon_skill(CHAR_DATA *ch, const char *sn)
{
/* sn == NULL for exotic */
	if (sn == NULL)
		return 3 * ch->level;

	return get_skill(ch, sn);
} 

/*
 * Utter mystical words for an sn.
 */
void say_spell(CHAR_DATA *ch, const char *sn)
{
	char buf  [MAX_STRING_LENGTH];
	CHAR_DATA *rch;
	const char *pName;
	int iSyl;
	int length;
	int skill;

	struct syl_type
	{
		char *	old;
		char *	new;
	};

	static const struct syl_type syl_table[] =
	{
		{ " ",		" "		},
		{ "ar",		"abra"		},
		{ "au",		"kada"		},
		{ "bless",	"fido"		},
		{ "blind",	"nose"		},
		{ "bur",	"mosa"		},
		{ "cu",		"judi"		},
		{ "de",		"oculo"		},
		{ "en",		"unso"		},
		{ "light",	"dies"		},
		{ "lo",		"hi"		},
		{ "mor",	"zak"		},
		{ "move",	"sido"		},
		{ "ness",	"lacri"		},
		{ "ning",	"illa"		},
		{ "per",	"duda"		},
		{ "ra",		"gru"		},
		{ "fresh",	"ima"		},
		{ "re",		"candus"	},
		{ "son",	"sabru"		},
		{ "tect",	"infra"		},
		{ "tri",	"cula"		},
		{ "ven",	"nofo"		},
		{ "a", "a" }, { "b", "b" }, { "c", "q" }, { "d", "e" },
		{ "e", "z" }, { "f", "y" }, { "g", "o" }, { "h", "p" },
		{ "i", "u" }, { "j", "y" }, { "k", "t" }, { "l", "r" },
		{ "m", "w" }, { "n", "i" }, { "o", "a" }, { "p", "s" },
		{ "q", "d" }, { "r", "f" }, { "s", "g" }, { "t", "h" },
		{ "u", "j" }, { "v", "z" }, { "w", "x" }, { "x", "n" },
		{ "y", "l" }, { "z", "k" },
		{ str_empty, str_empty }
	};

	buf[0]	= '\0';
	for (pName = sn; *pName != '\0'; pName += length) {
		for (iSyl = 0; (length = strlen(syl_table[iSyl].old)); iSyl++) {
			if (!str_prefix(syl_table[iSyl].old, pName)) {
				strnzcat(buf, sizeof(buf), syl_table[iSyl].new);
				break;
			}
		}
		if (length == 0)
			length = 1;
	}

	for (rch = ch->in_room->people; rch; rch = rch->next_in_room) {
		if (rch == ch)
			continue;

		skill = get_skill(rch, "spell craft") * 9 / 10;
		if (skill < number_percent()) {
			act("$n utters the words, '$t'.", ch, buf, rch, TO_VICT);
			check_improve(rch, "spell craft", FALSE, 5);
		} else  {
			act("$n utters the words, '$t'.",
			    ch, sn, rch, TO_VICT);
			check_improve(rch, "spell craft", TRUE, 5);
		}
	}
}

/*
 * skill_beats -- return skill beats
 */
int skill_beats(const char *sn)
{
	skill_t *sk;

	if ((sk = skill_lookup(sn)) == NULL) {
#ifdef NAME_STRICT_CHECKS
		bug("skill_beats: %s: unknown skill", sn);
#endif
		return 0;
	}

	return sk->beats;
}
/*
 * skill_mana -- return mana cost based on min_mana and ch->level
 */
int skill_mana(CHAR_DATA *ch, const char *sn)
{
	skill_t *sk;

	if ((sk = skill_lookup(sn)) == NULL) {
#ifdef NAME_STRICT_CHECKS
		bug("skill_mana: %s: unknown skill", sn);
#endif
		return 0;
	}

	if (IS_NPC(ch))
		return sk->min_mana;

	return UMAX(sk->min_mana, 100 / (2 + UMAX(ch->level - skill_level(ch, sn), 0)));
}

/*
 * skill_level -- find min level of the skill for char
 */
int skill_level(CHAR_DATA *ch, const char *sn)
{
	spec_skill_t spec_sk;

	if (IS_NPC(ch))
		return ch->level;

	spec_sk.sn = sn;
	spec_stats(ch, &spec_sk);
	return spec_sk.level;
}

static void *
skill_slot_cb(void *p, void *d)
{
	skill_t *sk = (skill_t *) p;
	int slot = *(int *) d;

	if (sk->slot == slot)
		return (void*) sk->name;
	return NULL;
}

/*
 * Lookup a skill by slot number.
 * Used for object loading.
 */
const char *skill_slot_lookup(int slot)
{
	const char *sn;

	if (slot <= 0)
		return NULL;

	sn = hash_foreach(&skills, skill_slot_cb, &slot);
	if (IS_NULLSTR(sn))
		db_error("skill_slot_lookup", "unknown slot %d", slot);
	return str_qdup(sn);
}

void skill_init(skill_t *sk)
{
	sk->name = str_empty;
	sk->fun_name = str_empty;
	sk->fun = NULL;
	sk->target = 0;
	sk->min_pos = 0;
	sk->slot = 0;
	sk->min_mana = 0;
	sk->beats = 0;
	sk->noun_damage = str_empty;
	sk->msg_off = str_empty;
	sk->msg_obj = str_empty;
	sk->skill_flags = 0;
	sk->restrict_race = str_empty;
	sk->group = 0;
	sk->skill_type = 0;
}

skill_t *skill_cpy(skill_t *dst, const skill_t *src)
{
	dst->name = str_qdup(src->name);
	dst->fun_name = str_qdup(src->fun_name);
	dst->fun = src->fun;
	dst->target = src->target;
	dst->min_pos = src->min_pos;
	dst->slot = src->slot;
	dst->min_mana = src->min_mana;
	dst->beats = src->beats;
	dst->noun_damage = str_qdup(src->noun_damage);
	dst->msg_off = str_qdup(src->msg_off);
	dst->msg_obj = str_qdup(src->msg_obj);
	dst->skill_flags = src->skill_flags;
	dst->restrict_race = str_qdup(src->restrict_race);
	dst->group = src->group;
	dst->skill_type = src->skill_type;
	return dst;
}

void skill_destroy(skill_t *sk)
{
	free_string(sk->name);
	free_string(sk->fun_name);
	free_string(sk->noun_damage);
	free_string(sk->msg_off);
	free_string(sk->msg_obj);
	free_string(sk->restrict_race);
}

/*-----------------------------------------------------------------------------
 * mob skills stuff
 */

typedef int MOB_SKILL(CHAR_DATA *);
#define DECLARE_MOB_SKILL(fun) static MOB_SKILL fun;
#define MOB_SKILL(fun) static int fun(CHAR_DATA *mob)

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

static size_t mob_skill_count;

static mob_skill_t mob_skill_tab[] =
{
	{ "track",		mob_track		},
	{ "sneak", 		mob_sneak		},
	{ "hide",		mob_hide		},
	{ "pick lock",		mob_pick_lock		},
	{ "backstab",		mob_backstab		},
	{ "dual backstab",	mob_dual_backstab	},
	{ "dodge",		mob_dodge		},
	{ "parry",		mob_parry		},
	{ "dirt kicking",	mob_dirt_kicking	},
	{ "shield block",	mob_shield_block	},
	{ "second attack",	mob_second_attack	},
	{ "third attack",	mob_third_attack	},
	{ "fourth attack",	mob_fourth_attack	},
	{ "hand to hand",	mob_hand_to_hand	},
	{ "trip",		mob_trip		},
	{ "bash",		mob_bash		},
	{ "bash door",		mob_bash		},
	{ "kick",		mob_kick		},
	{ "critical strike",	mob_critical_strike	},
	{ "disarm",		mob_disarm		},
	{ "grip",		mob_grip		},
	{ "berserk",		mob_berserk		},
	{ "tiger power",	mob_tiger_power		},
	{ "rescue",		mob_rescue		},
	{ "crush",		mob_crush		},
	{ "sword",		mob_weapon		},
	{ "dagger",		mob_weapon		},
	{ "spear",		mob_weapon		},
	{ "mace",		mob_weapon		},
	{ "axe",		mob_weapon		},
	{ "flail",		mob_weapon		},
	{ "staff",		mob_weapon		},
	{ "whip",		mob_weapon		},
	{ "polearm",		mob_weapon		},
	{ "bow",		mob_weapon		},
	{ "arrow",		mob_weapon		},
	{ "lance",		mob_weapon		},

	{ NULL }
};

void
mob_skill_init(void)
{
	mob_skill_t *mob_skill;

	for (mob_skill = mob_skill_tab; mob_skill->sn; mob_skill++) {
		NAME_CHECK(&skills, mob_skill->sn, "mob_skill_init");
		mob_skill_count++;
	}
	qsort(mob_skill_tab, mob_skill_count, sizeof(mob_skill_t), cmpstr);
}

static int
get_mob_skill(CHAR_DATA *ch, skill_t *sk)
{
	mob_skill_t *mob_skill;

	if (sk->skill_type == ST_SPELL)
		return 40 + 2 * ch->level;

	if (!mob_skill_count)
		mob_skill_init();

	mob_skill = bsearch(&sk->name, mob_skill_tab, mob_skill_count,
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
		return mob->level * 2;
	return 0;
}

MOB_SKILL(mob_parry)
{
	if (IS_SET(mob->pMobIndex->off_flags, OFF_PARRY))
		return mob->level * 2;
	return 0;
}

MOB_SKILL(mob_dirt_kicking)
{
	if (IS_SET(mob->pMobIndex->off_flags, OFF_DIRT_KICK))
		return mob->level * 2;
	return 0;
}

MOB_SKILL(mob_shield_block)
{
	return 10 + 2 * mob->level;
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
