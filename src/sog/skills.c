/*
 * $Id: skills.c,v 1.99 1999-12-17 12:40:36 fjoe Exp $
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
	return base_exp(ch)*level*level*level;
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
void _set_skill(CHAR_DATA *ch, const char *sn, int percent, bool replace)
{
	pc_skill_t *pc_sk;

	if (IS_NULLSTR(sn))
		return;

	STRKEY_CHECK(&skills, sn, "_set_skill");

	if ((pc_sk = pc_skill_lookup(ch, sn)) != NULL) {
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
	_set_skill(ch, sn, percent, TRUE);
}

static void *
apply_sa_cb(void *p, va_list ap)
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
int get_skill_mod(CHAR_DATA *ch, skill_t *sk, int percent)
{
	int mod = 0;
	varr_foreach(&ch->sk_affected, apply_sa_cb, sk, percent, &mod);
	return mod;
}

/* for returning skill information */
int get_skill(CHAR_DATA *ch, const char *sn)
{
	int percent;
	skill_t *sk;

	if ((sk = skill_lookup(sn)) == NULL) 
		return 0;

	if (ch->shapeform) {
		if (!IS_NULLSTR(ch->shapeform->index->skill_spec)) {
			spec_skill_t *sp_sk;
			spec_t * fsp;
			if (!(fsp=spec_lookup(ch->shapeform->index->skill_spec))) {
				bug("get_skill: bad form (%s) spec (%s).\n",
					ch->shapeform->index->name,
					ch->shapeform->index->skill_spec);
				return 0;
			}
			
			if ((sp_sk = spec_skill_lookup(fsp, sn)) != NULL)
				return sp_sk->adept;
			}
		if (sk->skill_type == ST_SKILL 
		&& !IS_SET(sk->skill_flags, SKILL_FORM))
			return 0;

	}

	if (!IS_NPC(ch)) {
		pc_skill_t *pc_sk;

		if ((IS_SET(sk->skill_flags, SKILL_CLAN) 
		&& !clan_item_ok(ch->clan)))
			return 0;

		if ((pc_sk = pc_skill_lookup(ch, sn)) == NULL
		||  skill_level(ch, sn) > ch->level)
			percent = 0;
		else
			percent = pc_sk->percent;
	} else 
		percent = get_mob_skill(ch, sk);

	if (ch->daze > 0) {
		if (sk->skill_type == ST_SPELL
		||  sk->skill_type == ST_PRAYER)
			percent /= 2;
		else
			percent = 2 * percent / 3;
	}

	if (!IS_NPC(ch) && PC(ch)->condition[COND_DRUNK]  > 10)
		percent = 9 * percent / 10;

	return UMAX(0, percent + get_skill_mod(ch, sk, percent));
}

pc_skill_t *
pc_skill_lookup(CHAR_DATA *ch, const char *sn)
{
	return (pc_skill_t*) varr_bsearch(&PC(ch)->learned, &sn, cmpstr);
}

/*
 * Lookup skill by prefix in varr.
 * First field of structure assumed to be sn
 */
void *skill_vsearch(varr *v, const char *psn)
{
	if (IS_NULLSTR(psn))
		return NULL;
	return varr_foreach(v, strkey_search_cb, psn);
}

/* for returning weapon information */
const char* get_weapon_sn(OBJ_DATA *wield)
{
	if (wield == NULL)
		return "hand to hand";

	if (wield->pObjIndex->item_type != ITEM_WEAPON)
		return NULL;

	switch (INT(wield->value[0])) {
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
void say_spell(CHAR_DATA *ch, const skill_t* spell)
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

	if (spell->skill_type == ST_PRAYER) {
		if (!IS_EVIL(ch)) {
			act("$n raises $s hands to the sky in holy prayer.",
				ch, NULL, NULL, TO_ROOM);
		} else {
			act("$n utters the words of unholy power.",
				ch, NULL, NULL, TO_ROOM);
		}
		return;
	}

	buf[0]	= '\0';
	pName = gmlstr_mval(&spell->sk_name);
	for (; *pName != '\0'; pName += length) {
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
			act("$n utters the words, '$v'.",
			    ch, &spell->sk_name, rch, TO_VICT);
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
#ifdef STRKEY_STRICT_CHECKS
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
#ifdef STRKEY_STRICT_CHECKS
		bug("skill_mana: %s: unknown skill", sn);
#endif
		return 0;
	}

	if (IS_NPC(ch))
		return sk->min_mana;

	return UMAX(sk->min_mana, 100 / (2 + UMAX(ch->level - skill_level(ch, sn), 0)));
}

gmlstr_t *
skill_noun(const char *sn)
{
	skill_t *sk;
	STRKEY_CHECK(&skills, sn, "skill_noun");
	sk = skill_lookup(sn);
	if (sk != NULL)
		return &sk->noun_damage;
	else
		return glob_lookup("hit");
}

static void *
skills_dump_cb(void *p, va_list ap)
{
	skill_t *sk = (skill_t *) p;

	BUFFER *output = va_arg(ap, BUFFER *);
	int skill_type = va_arg(ap, int);
	int *pcol = va_arg(ap, int *);

	const char *sn = gmlstr_mval(&sk->sk_name);

	if (!str_cmp(sn, "reserved")
	||  (skill_type >= 0 && sk->skill_type != skill_type))
		return NULL;

	buf_printf(output, "%-19.18s", sn);
	if (++(*pcol) % 4 == 0)
		buf_add(output, "\n");
	return 0;
}

void
skills_dump(BUFFER *output, int skill_type)
{
	int col = 0;
	hash_foreach(&skills, skills_dump_cb, output, skill_type, &col); 
	if (col % 4)
		buf_add(output, "\n");
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
skill_slot_cb(void *p, va_list ap)
{
	skill_t *sk = (skill_t *) p;

	int slot = va_arg(ap, int);

	if (sk->slot == slot)
		return (void*) gmlstr_mval(&sk->sk_name);
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

	sn = hash_foreach(&skills, skill_slot_cb, slot);
	if (IS_NULLSTR(sn))
		db_error("skill_slot_lookup", "unknown slot %d", slot);
	return str_qdup(sn);
}

void skill_init(skill_t *sk)
{
	gmlstr_init(&sk->sk_name);
	sk->fun_name = str_empty;
	sk->fun = NULL;
	sk->target = 0;
	sk->min_pos = 0;
	sk->slot = 0;
	sk->min_mana = 0;
	sk->beats = 0;
	gmlstr_init(&sk->noun_damage);
	mlstr_init(&sk->msg_off, str_empty);
	mlstr_init(&sk->msg_obj, str_empty);
	sk->skill_flags = 0;
	sk->restrict_race = str_empty;
	sk->group = 0;
	sk->skill_type = 0;
	sk->eventlist = NULL;
}

skill_t *skill_cpy(skill_t *dst, const skill_t *src)
{
	event_fun_t *evfs, *evfd;

	gmlstr_cpy(&dst->sk_name, &src->sk_name);
	dst->fun_name = str_qdup(src->fun_name);
	dst->fun = src->fun;
	dst->target = src->target;
	dst->min_pos = src->min_pos;
	dst->slot = src->slot;
	dst->min_mana = src->min_mana;
	dst->beats = src->beats;
	gmlstr_cpy(&dst->noun_damage, &src->noun_damage);
	mlstr_cpy(&dst->msg_off, &src->msg_off);
	mlstr_cpy(&dst->msg_obj, &src->msg_obj);
	dst->skill_flags = src->skill_flags;
	dst->restrict_race = str_qdup(src->restrict_race);
	dst->group = src->group;
	dst->skill_type = src->skill_type;

	for (evfs = src->eventlist; evfs; evfs = evfs->next) {
		evfd = evf_new();
		evfd->event = evfs->event;
		evfd->fun_name = str_qdup(evfs->fun_name);
		evfd->fun = evfs->fun;
		evfd->next = dst->eventlist;
		dst->eventlist = evfd;
	}

	return dst;
}

void skill_destroy(skill_t *sk)
{
	event_fun_t *evf, *evf_next;

	gmlstr_destroy(&sk->sk_name);
	free_string(sk->fun_name);
	gmlstr_destroy(&sk->noun_damage);
	mlstr_destroy(&sk->msg_off);
	mlstr_destroy(&sk->msg_obj);
	free_string(sk->restrict_race);

	for (evf = sk->eventlist; evf; evf = evf_next) {
		evf_next = evf->next;
		evf_free(evf);
	}
}

/*
 *  routine that checks for matching events and calls event function
 */
void check_one_event(CHAR_DATA *ch, AFFECT_DATA *paf, flag_t event)
{
	skill_t *sk;
	event_fun_t *evf;

	if (!IS_SET(paf->events, event))
		return;

	if ((sk = skill_lookup(paf->type)) == NULL) {
		bug("unknown skill");
		return;
	}

	for (evf = sk->eventlist; evf; evf = evf->next)
		if (evf->event == event) break;

	if (evf && evf->fun)
		(evf->fun)(ch, paf);
}

void check_events(CHAR_DATA *ch, AFFECT_DATA *list, flag_t event)
{
	AFFECT_DATA *paf, *paf_next;

	for (paf = list; paf != NULL; paf = paf_next) {

		paf_next = paf->next;
		check_one_event(ch, paf, event);

		if (IS_EXTRACTED(ch))
			break;
	}
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
DECLARE_MOB_SKILL(mob_distance);
DECLARE_MOB_SKILL(mob_deathblow);
DECLARE_MOB_SKILL(mob_spellbane);
DECLARE_MOB_SKILL(mob_dual_wield);

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
	{ "distance",		mob_distance		},
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
	{ "deathblow",		mob_deathblow		},
	{ "spellbane", 		mob_spellbane		},
	{ "disarm",		mob_disarm		},
	{ "grip",		mob_grip		},
	{ "berserk",		mob_berserk		},
	{ "tiger power",	mob_tiger_power		},
	{ "dual wield",		mob_dual_wield		},
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
		STRKEY_CHECK(&skills, mob_skill->sn, "mob_skill_init");
		mob_skill_count++;
	}
	qsort(mob_skill_tab, mob_skill_count, sizeof(mob_skill_t), cmpstr);
}

static int
get_mob_skill(CHAR_DATA *ch, skill_t *sk)
{
	mob_skill_t *mob_skill;
	const char *sn;

	if (sk->skill_type == ST_SPELL
	||  sk->skill_type == ST_PRAYER)
		return 40 + 2 * ch->level;

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
		return mob->level * 2;
	return 0;
}

MOB_SKILL(mob_parry)
{
	if (IS_SET(mob->pMobIndex->off_flags, OFF_PARRY))
		return mob->level * 2;
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

