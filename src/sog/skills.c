/*
 * $Id: skills.c,v 1.72 1999-06-29 18:28:42 avn Exp $
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

varr skills = { sizeof(skill_t), 8 };

int	ch_skill_nok	(CHAR_DATA *ch , int sn);

int base_exp(CHAR_DATA *ch)
{
	int expl;
	class_t *cl;
	race_t *r;
	rclass_t *rcl;

	if (IS_NPC(ch)
	||  (cl = class_lookup(ch->class)) == NULL
	||  (r = race_lookup(ch->pcdata->race)) == NULL
	||  !r->pcdata
	||  (rcl = rclass_lookup(r, cl->name)) == NULL)
		return 1500;

	expl = 1000 + r->pcdata->points + cl->points;
	return expl * rcl->mult/100;
}

int exp_for_level(CHAR_DATA *ch, int level)
{
	int i = base_exp(ch) * level;
	return i + i * (level-1) / 20;
}

int exp_to_level(CHAR_DATA *ch)
{ 
	return exp_for_level(ch, ch->level+1) - ch->exp;
}

/* checks for skill improvement */
void check_improve(CHAR_DATA *ch, int sn, bool success, int multiplier)
{
	pcskill_t *ps;
	class_t *cl;
	cskill_t *cs;
	int chance;
	int rating;

	if (IS_NPC(ch)
	||  (cl = class_lookup(ch->class)) == NULL
	||  (ps = pcskill_lookup(ch, sn)) == NULL
	||  ps->percent == 0 || ps->percent == 100
	||  skill_level(ch, sn) > ch->level)
		return;

	if ((cs = cskill_lookup(cl, sn)))
		rating = cs->rating;
	else
		rating = 1;

	/* check to see if the character has a chance to learn */
	chance = 10 * int_app[get_curr_stat(ch,STAT_INT)].learn;
	chance /= (multiplier *	rating * 4);
	chance += ch->level;

	if (number_range(1, 1000) > chance)
		return;

/* now that the character has a CHANCE to learn, see if they really have */	

	if (success) {
		chance = URANGE(5, 100 - ps->percent, 95);
		if (number_percent() < chance) {
			ps->percent++;
			gain_exp(ch, 2 * rating);
			if (ps->percent == 100) {
				act_puts("{gYou mastered {W$t{g!{x",
					 ch, skill_name(sn), NULL,
					 TO_CHAR, POS_DEAD);
			} else {
				act_puts("{gYou have become better at {W$t{g!{x",
					 ch, skill_name(sn), NULL,
					 TO_CHAR, POS_DEAD);
			}
		}
	}
	else {
		chance = URANGE(5, ps->percent / 2, 30);
		if (number_percent() < chance) {
			if ((ps->percent += number_range(1, 3)) > 100)
				ps->percent = 100;
			gain_exp(ch, 2 * rating);
			if (ps->percent == 100) {
				act_puts("{gYou learn from your mistakes and you manage to master {W$t{g!{x",
					 ch, skill_name(sn), NULL,
					 TO_CHAR, POS_DEAD);
			} else {
				act_puts("{gYou learn from your mistakes and your {W$t{g skill improves!{x",
					 ch, skill_name(sn), NULL,
					 TO_CHAR, POS_DEAD);
			}

		}
	}
}

/*
 * simply adds sn to ch's known skills (if skill is not already known).
 */
void set_skill_raw(CHAR_DATA *ch, int sn, int percent, bool replace)
{
	pcskill_t *ps;

	if (sn <= 0)
		return;

	if ((ps = pcskill_lookup(ch, sn))) {
		if (replace || ps->percent < percent)
			ps->percent = percent;
		return;
	}
	ps = varr_enew(&ch->pcdata->learned);
	ps->sn = sn;
	ps->percent = percent;
	varr_qsort(&ch->pcdata->learned, cmpint);
}

/* use for adding/updating all skills available for that ch  */
void update_skills(CHAR_DATA *ch)
{
	int i;
	class_t *cl;
	race_t *r;
	clan_t *clan;
	const char *p;

/* NPCs do not have skills */
	if (IS_NPC(ch)
	||  (cl = class_lookup(ch->class)) == NULL
	||  (r = race_lookup(ch->race)) == NULL
	||  !r->pcdata)
		return;

/* add class skills */
	for (i = 0; i < cl->skills.nused; i++) {
		cskill_t *cs = VARR_GET(&cl->skills, i);
		set_skill_raw(ch, cs->sn, 1, FALSE);
	}

/* add race skills */
	for (i = 0; i < r->pcdata->skills.nused; i++) {
		rskill_t *rs = VARR_GET(&r->pcdata->skills, i);
		set_skill_raw(ch, rs->sn, 100, FALSE);
	}

	if ((p = r->pcdata->bonus_skills))
		for (;;) {
			int sn;
			char name[MAX_STRING_LENGTH];

			p = one_argument(p, name, sizeof(name));
			if (name[0] == '\0')
				break;
		
			sn = sn_lookup(name);
			if (sn < 0)
				continue;

			set_skill_raw(ch, sn, 100, FALSE);
		}

/* add clan skills */
	if ((clan = clan_lookup(ch->clan))) {
		for (i = 0; i < clan->skills.nused; i++) {
			clskill_t *cs = VARR_GET(&clan->skills, i);
			set_skill_raw(ch, cs->sn, cs->percent, FALSE);
		}
	}

/* remove not matched skills */
	for (i = 0; i < ch->pcdata->learned.nused; i++) {
		pcskill_t *ps = VARR_GET(&ch->pcdata->learned, i);
		if (skill_level(ch, ps->sn) > LEVEL_HERO && !IS_IMMORTAL(ch))
			ps->percent = 0;
	}
}

void set_skill(CHAR_DATA *ch, int sn, int percent)
{
	set_skill_raw(ch, sn, percent, TRUE);
}

const char *skill_name(int sn)
{
	skill_t *sk = varr_get(&skills, sn);
	if (sk)
		return sk->name;
	return "none";
}

/* for returning skill information */
int get_skill(CHAR_DATA *ch, int sn)
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
		pcskill_t *ps;

		if ((ps = pcskill_lookup(ch, sn)) == NULL
		||  skill_level(ch, sn) > ch->level)
			skill = 0;
		else
			skill = ps->percent;
	}
	else {
		flag64_t act = ch->pIndexData->act;
		flag64_t off = ch->pIndexData->off_flags;

		/* mobiles */
		if (sk->skill_type == ST_SPELL)
			skill = 40 + 2 * ch->level;
		else if (sn == gsn_track)
			skill = 100;
		else if ((sn == gsn_sneak || sn == gsn_hide || sn == gsn_pick)
		     &&  IS_SET(act, ACT_THIEF))
			skill = ch->level * 2 + 20;
		else if (sn == gsn_backstab
		     &&  (IS_SET(act, ACT_THIEF) ||
			  IS_SET(off, OFF_BACKSTAB)))
			skill = ch->level * 2 + 20;
		else if (sn == gsn_dual_backstab
		     &&  (IS_SET(act, ACT_THIEF) ||
			  IS_SET(off, OFF_BACKSTAB)))
			skill = ch->level + 20;
		else if ((sn == gsn_dodge && IS_SET(off, OFF_DODGE)) ||
 		         (sn == gsn_parry && IS_SET(off, OFF_PARRY)) ||
			 (sn == gsn_dirt && IS_SET(off, OFF_DIRT_KICK)))
			skill = ch->level * 2;
 		else if (sn == gsn_shield_block)
			skill = 10 + 2 * ch->level;
		else if (sn == gsn_second_attack &&
			 (IS_SET(act, ACT_WARRIOR | ACT_THIEF)))
			skill = 10 + 3 * ch->level;
		else if (sn == gsn_third_attack && IS_SET(act, ACT_WARRIOR))
			skill = 4 * ch->level - 40;
		else if (sn == gsn_fourth_attack && IS_SET(act, ACT_WARRIOR))
			skill = 4 * ch->level - 60;
		else if (sn == gsn_hand_to_hand)
			skill = 40 + 2 * ch->level;
 		else if (sn == gsn_trip && IS_SET(off, OFF_TRIP)) 
			skill = 10 + 3 * ch->level;
 		else if ((sn == gsn_bash || sn == gsn_bash_door) &&
			 IS_SET(off, OFF_BASH))
			skill = 10 + 3 * ch->level;
		else if (sn == gsn_kick && IS_SET(off, OFF_KICK))
			skill = 10 + 3 * ch->level;
		else if ((sn == gsn_critical) && IS_SET(act, ACT_WARRIOR))
			skill = ch->level;
		else if (sn == gsn_disarm &&
			 (IS_SET(off, OFF_DISARM) ||
			  IS_SET(act, ACT_WARRIOR | ACT_THIEF)))
			skill = 20 + 3 * ch->level;
		else if (sn == gsn_grip &&
			 (IS_SET(act, ACT_WARRIOR | ACT_THIEF)))
			skill = ch->level;
		else if ((sn == gsn_berserk || sn == gsn_tiger_power) &&
			 IS_SET(off, OFF_BERSERK))
			skill = 3 * ch->level;
		else if (sn == gsn_kick)
			skill = 10 + 3 * ch->level;
		else if (sn == gsn_rescue)
			skill = 40 + ch->level; 
		else if (sn == gsn_sword || sn == gsn_dagger ||
			 sn == gsn_spear || sn == gsn_mace ||
			 sn == gsn_axe || sn == gsn_flail ||
			 sn == gsn_whip || sn == gsn_polearm ||
			 sn == gsn_bow || sn == gsn_arrow || sn == gsn_lance)
			skill = 40 + 5 * ch->level / 2;
		else if (sn == gsn_crush && IS_SET(off, OFF_CRUSH))
			skill = 10 + 3 * ch->level;
		else 
			skill = 0;
	}

	if (ch->daze > 0) {
		if (sk->skill_type == ST_SPELL)
			skill /= 2;
		else
			skill = 2 * skill / 3;
	}

	if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK]  > 10)
		skill = 9 * skill / 10;
	skill = URANGE(0, skill, 100);

	/*
	 * apply class/skill modifiers
	 */
	if (skill != 0 && !IS_NPC(ch)) {
		class_t *cl;
		cskill_t *csk;

		if ((cl = class_lookup(ch->class)) != NULL
		&&  (csk = cskill_lookup(cl, sn)) != NULL)
			skill += csk->mod;
		skill = UMAX(1, skill);
	}

	/*
	 * apply skill affect modifiers
	 */
	for (paf = ch->affected; paf; paf = paf->next) {
		if (paf->where != TO_SKILLS
		||  (!IS_SET(paf->bitvector, SK_AFF_ALL) &&
		     paf->location != -sn)
		||  (IS_SET(paf->bitvector, SK_AFF_NOTCLAN &&
		     IS_SET(skill_lookup(sn)->skill_flags, SKILL_CLAN))))
			continue;

		add += paf->modifier;
		teach |= IS_SET(paf->bitvector, SK_AFF_TEACH);
	}

	return UMAX(0, skill ? (skill+add) : teach ? add : 0);
}

/*
 * Lookup a skill by name.
 */
int sn_lookup(const char *name)
{
	int sn;

	if (IS_NULLSTR(name))
		return -1;

	for (sn = 0; sn < skills.nused; sn++)
		if (!str_prefix(name, SKILL(sn)->name))
			return sn;

	return -1;
}

/*
 * Lookup skill in varr.
 * First field of structure assumed to be sn
 */
void *skill_vlookup(varr *v, const char *name)
{
	int i;

	if (IS_NULLSTR(name))
		return NULL;

	for (i = 0; i < v->nused; i++) {
		skill_t *skill;
		int *psn = (int*) VARR_GET(v, i);

		if ((skill = skill_lookup(*psn))
		&&  !str_prefix(name, skill->name))
			return psn;
	}

	return NULL;
}

/* for returning weapon information */
int get_weapon_sn(OBJ_DATA *wield)
{
	int sn;

	if (wield == NULL)
		return gsn_hand_to_hand;

	if (wield->pIndexData->item_type != ITEM_WEAPON)
		return 0;

	switch (wield->value[0]) {
	default :               sn = -1;		break;
	case(WEAPON_SWORD):     sn = gsn_sword;		break;
	case(WEAPON_DAGGER):    sn = gsn_dagger;	break;
	case(WEAPON_SPEAR):     sn = gsn_spear;		break;
	case(WEAPON_MACE):      sn = gsn_mace;		break;
	case(WEAPON_AXE):       sn = gsn_axe;		break;
	case(WEAPON_FLAIL):     sn = gsn_flail;		break;
	case(WEAPON_WHIP):      sn = gsn_whip;		break;
	case(WEAPON_POLEARM):   sn = gsn_polearm;	break;
	case(WEAPON_BOW):	sn = gsn_bow;		break;
	case(WEAPON_ARROW):	sn = gsn_arrow;		break;
	case(WEAPON_LANCE):	sn = gsn_lance;		break;
	case(WEAPON_STAFF):	sn = gsn_staff;		break;
	}
	return sn;
}

int get_weapon_skill(CHAR_DATA *ch, int sn)
{
	 int sk;

/* -1 is exotic */
	if (sn == -1)
		sk = 3 * ch->level;
	else if (!IS_NPC(ch))
		sk = get_skill(ch, sn);
	else if (sn == gsn_hand_to_hand)
		sk = 40 + 2 * ch->level;
	else 
		sk = 40 + 5 * ch->level / 2;

	return URANGE(0, sk, 100);
} 

/*
 * Utter mystical words for an sn.
 */
void say_spell(CHAR_DATA *ch, int sn)
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
	for (pName = skill_name(sn); *pName != '\0'; pName += length) {
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

		skill = (get_skill(rch, gsn_spell_craft) * 9) / 10;
		if (skill < number_percent()) {
			act("$n utters the words, '$t'.", ch, buf, rch, TO_VICT);
			check_improve(rch, gsn_spell_craft, FALSE, 5);
		}
		else  {
			act("$n utters the words, '$t'.",
			    ch, skill_name(sn), rch, TO_VICT);
			check_improve(rch, gsn_spell_craft, TRUE, 5);
		}
	}
}

/* find min level of the skill for char */
int skill_level(CHAR_DATA *ch, int sn)
{
	int slevel = LEVEL_IMMORTAL;
	skill_t *sk;
	clan_t *clan;
	clskill_t *clan_skill;
	class_t *cl;
	cskill_t *class_skill;
	race_t *r;
	rskill_t *race_skill;
	AFFECT_DATA *paf;

/* noone can use ill-defined skills */
/* broken chars can't use any skills */
	if (IS_NPC(ch)
	||  (sk = skill_lookup(sn)) == NULL
	||  (cl = class_lookup(ch->class)) == NULL
	||  (r = race_lookup(ch->race)) == NULL
	||  !r->pcdata)
		return slevel;

	if ((clan = clan_lookup(ch->clan))
	&&  (clan_skill = clskill_lookup(clan, sn)))
		slevel = UMIN(slevel, clan_skill->level);

	if ((class_skill = cskill_lookup(cl, sn))) {
		slevel = UMIN(slevel, class_skill->level);
		if (is_name(sk->name, r->pcdata->bonus_skills))
			slevel = UMIN(slevel, 1);
	}

	if ((race_skill = rskill_lookup(r, sn)))
		slevel = UMIN(slevel, race_skill->level);

	for (paf = ch->affected; paf; paf = paf->next)
	    if (paf->where == TO_SKILLS && paf->location == -sn)
		slevel = 1;
	return slevel;
}

/*
 * assumes !IS_NPC(ch) && ch->level >= skill_level(ch, sn)
 */
int mana_cost(CHAR_DATA *ch, int sn)
{
	skill_t *sk;

	if ((sk = skill_lookup(sn)) == NULL)
		return 0;

	return UMAX(sk->min_mana, 100 / (2 + ch->level - skill_level(ch, sn)));
}

