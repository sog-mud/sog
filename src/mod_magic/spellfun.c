/*
 * $Id: spellfun.c,v 1.225 2000-11-23 15:46:57 fjoe Exp $
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
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"

#include "quest.h"
#include "fight.h"
#include "effects.h"
#include "_magic.h"

void spell_acid_blast(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	dam = dice(level, 12);
	if (saves_spell(level, victim, DAM_ACID))
		dam /= 2;
	damage(ch, victim, dam, sn, DAM_ACID, DAMF_SHOW);
}

void spell_armor(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (is_affected(victim, sn)) {
		if (victim == ch)
		  act_char("You are already armored.", ch);
		else
		  act("$N is already armored.",ch,NULL,victim,TO_CHAR);
		return;
	}

	af.where	 = TO_AFFECTS;
	af.type      = sn;
	af.level	 = level;
	af.duration  = 7 + level / 6;
	af.modifier  = -1 * UMAX(20,10 + level / 4); /* af.modifier  = -20;*/
	INT(af.location) = APPLY_AC;
	af.bitvector = 0;
	af.owner	= NULL;
	affect_to_char(victim, &af);
	act_char("You feel someone protecting you.", victim);
	if (ch != victim)
		act("$N is protected by your magic.",ch,NULL,victim,TO_CHAR);
}

/*
 * can be called with ch == NULL, vo == ch (e.g.: obj prog of tattoo of venus)
 */
void spell_bless(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim;
	AFFECT_DATA af;

	/* deal with the object case first */
	if (mem_is(vo, MT_OBJ)) {
		OBJ_DATA *obj = (OBJ_DATA *) vo;
		if (IS_OBJ_STAT(obj,ITEM_BLESS))
		{
		    act("$p is already blessed.",ch,obj,NULL,TO_CHAR);
		    return;
		}

		if (IS_OBJ_STAT(obj,ITEM_EVIL))
		{
		    AFFECT_DATA *paf;

		    paf = affect_find(obj->affected, "curse");
		    if (!saves_dispel(level,paf != NULL ? paf->level : obj->level,0))
		    {
			if (paf != NULL)
			    affect_remove_obj(obj,paf);
			act("$p glows a pale blue.",ch,obj,NULL,TO_ALL);
			REMOVE_OBJ_STAT(obj, ITEM_EVIL);
			return;
		    }
		    else
		    {
			act("The evil of $p is too powerful for you to overcome.",
			    ch,obj,NULL,TO_CHAR);
			return;
		    }
		}

		af.where	= TO_OBJECT;
		af.type		= sn;
		af.level	= level;
		af.duration	= (6 + level / 2);
		INT(af.location)= APPLY_SAVES;
		af.modifier	= -1;
		af.bitvector	= ITEM_BLESS;
		af.owner	= NULL;
		affect_to_obj(obj,&af);

		act("$p glows with a holy aura.",ch,obj,NULL,TO_ALL);
		return;
	}

	victim = (CHAR_DATA *) vo;

	if ((ch && victim->position == POS_FIGHTING)
	||  is_affected(victim, sn)) {
		if (victim == ch)
			act_char("You are already blessed.", ch);
		else if (ch) {
			act("$N already has divine favor.",
			    ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	af.where	= TO_AFFECTS;
	af.type		= sn;
	af.level	= level;
	af.duration	= (6 + level / 2);
	INT(af.location)= APPLY_LUCK;
	af.modifier	= level / 8;
	af.bitvector	= 0;
	af.owner	= NULL;
	affect_to_char(victim, &af);

	act_char("You feel righteous.", victim);
	if (ch && ch != victim) {
		act("You grant $N the favor of your god.",
		    ch, NULL, victim, TO_CHAR);
	}
}

void spell_blindness(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (IS_AFFECTED(victim, AFF_BLIND) || 
	    saves_spell(level,victim,DAM_OTHER))  {
	  act_char("You failed.", ch);
		return;
	}

	af.where     = TO_AFFECTS;
	af.type      = sn;
	af.level     = level;
	INT(af.location) = APPLY_HITROLL;
	af.modifier  = -4;
	af.duration  = 3+level / 15;
	af.bitvector = AFF_BLIND;
	af.owner	= NULL;
	affect_to_char(victim, &af);
	act_char("You are blinded!", victim);
	act("$n appears to be blinded.",victim,NULL,NULL,TO_ROOM);
}

void spell_burning_hands(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	dam = dice(level , 2) + 7;
	if (saves_spell(level, victim,DAM_FIRE))
		dam /= 2;
	damage(ch, victim, dam, sn, DAM_FIRE, DAMF_SHOW);
}

static void *
call_lightning_cb(void *vo, va_list ap)
{
	CHAR_DATA *vch = (CHAR_DATA *) vo;

	const char *sn = va_arg(ap, const char *);
	int level = va_arg(ap, int);
	CHAR_DATA *ch = va_arg(ap, CHAR_DATA *);
	int dam = va_arg(ap, int);

	if (vch->in_room == ch->in_room) {
		if (is_safe_spell(ch, vch, TRUE))
			return NULL;
		damage(ch, vch,
		       saves_spell(level, vch, DAM_LIGHTNING) ?
		       dam / 2 : dam,
		       sn, DAM_LIGHTNING, DAMF_SHOW);
		return NULL;
	}

	if (vch->in_room->area == ch->in_room->area
	&&  IS_OUTSIDE(vch)
	&&  IS_AWAKE(vch))
		act_char("Lightning flashes in the sky.", vch);
	return NULL;
}

void spell_call_lightning(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	int dam;

	if (!IS_OUTSIDE(ch)) {
		act_char("You must be out of doors.", ch);
		return;
	}

	if (weather_info.sky < SKY_RAINING) {
		act_char("You need bad weather.", ch);
		return;
	}

	dam = dice(level, 9);
	act_char("Gods' lightning strikes your foes!", ch);
	act("$n calls lightning to strike $s foes!", ch, NULL, NULL, TO_ROOM);
	vo_foreach(NULL, &iter_char_world, call_lightning_cb,
		   sn, level, ch, dam);
}

/* RT calm spell stops all fighting in the room */
void spell_calm(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *vch;
	int mlevel = 0;
	int count = 0;
	int high_level = 0;
	int chance;
	AFFECT_DATA af;

	/* get sum of all mobile levels in the room */
	for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room) {
		if (vch->position == POS_FIGHTING) {
			count++;
			if (IS_NPC(vch))
				mlevel += LEVEL(vch);
			else
				mlevel += LEVEL(vch)/2;
			high_level = UMAX(high_level, LEVEL(vch));
		}
	}

	/* compute chance of stopping combat */
	chance = 4 * level - high_level + 2 * count;

	if (IS_IMMORTAL(ch)) /* always works */
	  mlevel = 0;

	if (number_range(0, chance) >= mlevel) { /* hard to stop large fights */
		for (vch = ch->in_room->people; vch; vch = vch->next_in_room) {
			if (IS_NPC(vch)
			&&  IS_SET(vch->pMobIndex->act, ACT_UNDEAD))
				return;

			if (IS_AFFECTED(vch, AFF_CALM | AFF_BERSERK)
			||  is_affected(vch, "frenzy"))
				return;

			act_char("A wave of calm passes over you.", vch);
			if (vch->fighting || vch->position == POS_FIGHTING)
				stop_fighting(vch, FALSE);

			af.where = TO_AFFECTS;
			af.type = sn;
			af.level = level;
			af.duration = level/4;
			INT(af.location) = APPLY_HITROLL;
			if (!IS_NPC(vch))
				af.modifier = -5;
			else
				af.modifier = -2;
			af.bitvector = AFF_CALM;
			af.owner	= NULL;
			affect_to_char(vch, &af);

			INT(af.location) = APPLY_DAMROLL;
			affect_to_char(vch, &af);
		}
	}
}

typedef struct spell_dispel_t spell_dispel_t;
struct spell_dispel_t {
	const char *sn;
	const char *msg;
};

static spell_dispel_t dispel_tab[] = {
	{ "armor",		NULL					},
	{ "enhanced armor",	NULL					},
	{ "bless",		NULL					},
	{ "blindness",		"$n is no longer blinded."		},
	{ "calm",		"$n no longer looks so peaceful..."	},
	{ "charm person",	"$n regains $s free will."		},
	{ "chill touch",	"$n looks warmer."			},
	{ "curse",		NULL					},
	{ "detect evil",	NULL					},
	{ "detect good",	NULL					},
	{ "detect hidden",	NULL					},
	{ "detect invis",	NULL					},
	{ "detect magic",	NULL					},
	{ "faerie fire",	"$n's outline fades."			},
	{ "fly",		"$n falls to the ground!"		},
	{ "frenzy",		"$n no longer looks so wild."		},
	{ "giant strength",	"$n no longer looks so mighty."		},
	{ "haste",		"$n is no longer moving so quickly."	},
	{ "infravision",	NULL					},
	{ "invisibility",	"$n fades into existance."		},
	{ "mass invis",		"$n fades into existance."		},
	{ "pass door",		NULL					},
	{ "protection evil",	NULL					},
	{ "protection good",	NULL					},
	{ "sanctuary",		"The white aura around $n's body vanishes." },
	{ "black shroud",	"The black aura around $n's body vanishes." },
	{ "shield",		"The shield protecting $n vanishes."	},
	{ "sleep",		NULL					},
	{ "slow",		"$n is no longer moving so slowly."	},
	{ "stone skin",		"$n's skin regains its normal texture."	},
	{ "weaken",		"$n looks stronger."			},
	{ "shielding",		NULL					},
	{ "fear",		NULL					},
	{ "protection heat",	NULL					},
	{ "protection cold",	NULL					},
	{ "hallucination",	NULL					},
	{ "power word stun",	NULL					},
	{ "corruption",		"$n looks healthier."			},
	{ "web",		"The webs around $n dissolve."		},

	{ NULL }
};

bool dispel(CHAR_DATA *victim, int level)
{
	bool found = FALSE;
	spell_dispel_t *sd;

	for (sd = dispel_tab; sd->sn != NULL; sd++) {
		if (check_dispel(level, victim, sd->sn)) {
			if (sd->msg != NULL)
				act(sd->msg, victim, NULL, NULL, TO_ROOM);
			found = TRUE;
		}
	}
 
	return found;
}

void spell_dispel_magic(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	bool found = FALSE;

	if (saves_spell(level, victim, DAM_OTHER)) {
		act_char("You feel a brief tingling sensation.", victim);
		act_char("You failed.", ch);
		return;
	}

	found = dispel(victim, level);

	if (IS_AFFECTED(victim,AFF_SANCTUARY) 
	&&  !saves_dispel(level, LEVEL(victim), -1)
	&&  !is_affected(victim, "sanctuary")) {
		REMOVE_BIT(victim->affected_by,AFF_SANCTUARY);
		act("The white aura around $n's body vanishes.",
		    victim, NULL, NULL, TO_ROOM);
		found = TRUE;
	}
 
	if (IS_AFFECTED(victim, AFF_BLACK_SHROUD)
	&&  !saves_dispel(level, LEVEL(victim), -1)
	&&  !is_affected(victim, "black shroud")) {
		REMOVE_BIT(victim->affected_by, AFF_BLACK_SHROUD);
		act("The black aura around $n's body vanishes.",
		    victim, NULL, NULL, TO_ROOM);
		found = TRUE;
	}

	if (found)
		act_char("Ok.", ch);
	else
		act_char("Spell failed.", ch);
}

void spell_cancellation(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	/*
	 * unlike dispel magic, the victim gets NO save
	 */
	if (dispel((CHAR_DATA *)vo, level + 2))
		act_char("Ok.", ch);
	else
		act_char("Spell failed.", ch);
}

void spell_cause_light(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	damage(ch, (CHAR_DATA *) vo, dice(1, 8) + level / 3,
	       sn, DAM_HARM, DAMF_SHOW);
}

void spell_cause_critical(const char *sn, int level,CHAR_DATA *ch,void *vo)
{
	damage(ch, (CHAR_DATA *) vo, dice(3, 8) + level - 6,
	       sn, DAM_HARM, DAMF_SHOW);
}

void spell_cause_serious(const char *sn, int level,CHAR_DATA *ch,void *vo)
{
	damage(ch, (CHAR_DATA *) vo, dice(2, 8) + level / 2,
	       sn, DAM_HARM, DAMF_SHOW);
}

static void *
chain_lightning_cb(void *vo, va_list ap)
{
	CHAR_DATA *vch = (CHAR_DATA *) vo;

	CHAR_DATA *ch = va_arg(ap, CHAR_DATA *);
	CHAR_DATA **plast_vict = va_arg(ap, CHAR_DATA **);
	const char *sn;
	bool *pfound;
	int *plevel;
	int dam;

	if (vch == *plast_vict)
		return NULL;

	if (is_safe_spell(ch, vch, TRUE)) {
		act("The bolt passes around $n's body.",
		    ch, NULL, NULL, TO_ROOM);
		act("The bolt passes around your body.",
		    ch, NULL, NULL, TO_CHAR);
		return NULL;
	}

	sn = va_arg(ap, const char *);
	pfound = va_arg(ap, bool *);
	plevel = va_arg(ap, int *);

	*pfound = TRUE;
	*plast_vict = vch;
	act("The bolt arcs to $n!", vch, NULL, NULL, TO_ROOM);
	act("The bolt hits you!", vch, NULL, NULL, TO_CHAR);

	dam = dice(*plevel, 6);
	if (saves_spell(*plevel, vch, DAM_LIGHTNING))
		dam /= 3;
	damage(ch, vch, dam, sn, DAM_LIGHTNING, DAMF_SHOW);
	*plevel -= 4;  /* decrement damage */

	if (*plevel <= 0)
		return vch;

	return NULL;
} 

void spell_chain_lightning(const char *sn, int level,CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	CHAR_DATA *last_vict = victim;
	int dam;

	/*
	 * first strike
	 */
	act("A lightning bolt leaps from $n's hand and arcs to $N.",
	    ch, NULL, victim, TO_ROOM);
	act("A lightning bolt leaps from your hand and arcs to $N.",
	    ch, NULL, victim, TO_CHAR);
	act("A lightning bolt leaps from $n's hand and hits you!",
	    ch, NULL, victim, TO_VICT);

	dam = dice(level, 6);
	if (saves_spell(level, victim, DAM_LIGHTNING))
		dam /= 3;
	damage(ch, victim, dam, sn, DAM_LIGHTNING, DAMF_SHOW);
	level -= 4;   /* decrement damage */

	while (level > 0) {
		bool found = FALSE;
		vo_foreach(ch->in_room, &iter_char_room, chain_lightning_cb,
			   ch, &last_vict, sn, &found, &level);
		if (found)
			continue;

		/*
		 * no victim found, hit the caster
		 */
		if (ch == NULL)
			return;

		if (last_vict == ch) { /* no double hits */
			act("The bolt seems to have fizzled out.",
			    ch, NULL, NULL, TO_ROOM);
			act("The bolt grounds out through your body.",
			    ch, NULL, NULL, TO_CHAR);
			return;
		}

		last_vict = ch;
		act("The bolt arcs to $n...whoops!", ch, NULL, NULL, TO_ROOM);
		act_char("You are struck by your own lightning!", ch);
		dam = dice(level,6);
		if (saves_spell(level, ch, DAM_LIGHTNING))
			dam /= 3;
		damage(ch, ch, dam, sn, DAM_LIGHTNING, DAMF_SHOW);
		level -= 4;  /* decrement damage */
	} /* now go back and find more victims */
}

void spell_healing_light(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	AFFECT_DATA af2;
	AFFECT_DATA af;

	if (is_affected_room(ch->in_room, sn))
	{
		act_char("This room has already been healed by light.", ch);
		return;
	}

	af.where     = TO_ROOM_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = level / 25;
	INT(af.location) = APPLY_ROOM_HEAL;
	af.modifier  = level * 3 / 2;
	af.bitvector = 0;
	af.owner     = ch;
	affect_to_room(ch->in_room, &af);

	af2.where     = TO_AFFECTS;
	af2.type      = sn;
	af2.level	 = level;
	af2.duration  = level / 10;
	af2.modifier  = 0;
	INT(af2.location) = APPLY_NONE;
	af2.bitvector = 0;
	af2.owner	= NULL;
	affect_to_char(ch, &af2);
	act_char("The room starts to be filled with healing light.", ch);
	act("The room starts to be filled with $n's healing light.",ch,NULL,NULL,TO_ROOM);
}

void spell_charm_person(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (count_charmed(ch))
		return;

	if (victim == ch) {
		act_char("You like yourself even better!", ch);
		return;
	}

	if (!IS_NPC(ch)) {
		if (!IS_NPC(victim)) {
			level += get_curr_stat(ch, STAT_CHA) -
				 get_curr_stat(victim, STAT_CHA); 
		}

		if (flag_value(gender_table, mlstr_mval(&ch->gender)) == SEX_FEMALE)
			level += 2;
	}

	if (IS_AFFECTED(victim, AFF_CHARM)
	||  IS_AFFECTED(ch, AFF_CHARM)
	||  !IS_AWAKE(victim)
	||  victim->position == POS_FIGHTING
	||  level < LEVEL(victim)
	||  saves_spell(level, victim, DAM_CHARM) 
	||  (IS_NPC(victim) && victim->pMobIndex->pShop != NULL)
	||  (victim->in_room &&
	     IS_SET(victim->in_room->room_flags, ROOM_BATTLE_ARENA)))
		return;

	if (is_safe(ch, victim))
		return;

	add_follower(victim, ch);
	set_leader(victim, ch);

	af.where	= TO_AFFECTS;
	af.type		= sn;
	af.level	= level;
	af.duration	= number_fuzzy(level / 5);
	INT(af.location)= 0;
	af.modifier	= 0;
	af.bitvector	= AFF_CHARM;
	af.owner	= NULL;
	affect_to_char(victim, &af);
	act("Isn't $n just so nice?", ch, NULL, victim, TO_VICT);
	if (ch != victim)
		act("$N looks at you with adoring eyes.",
		    ch, NULL, victim, TO_CHAR);

	if (IS_NPC(victim) && !IS_NPC(ch)) {
		NPC(victim)->last_fought = ch;
		if (number_percent() < (4 + (LEVEL(victim) - LEVEL(ch))) * 10)
		 	add_mind(victim, ch->name);
		else if (NPC(victim)->in_mind == NULL) {
			NPC(victim)->in_mind =
				str_printf("%d", victim->in_room->vnum);
		}
	}
}

void spell_chill_touch(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	int dam;
	
	dam = dice(level, level / 6 + 1);

	if (!saves_spell(level, victim, DAM_COLD)) {
		act("$n turns blue and shivers.", victim, NULL, NULL, TO_ROOM);
		af.where     = TO_AFFECTS;
		af.type      = sn;
		af.level     = level;
		af.duration  = 6;
		INT(af.location) = APPLY_STR;
		af.modifier  = -1;
		af.bitvector = 0;
		af.owner	= NULL;
		affect_join(victim, &af);
	} else
		dam /= 2;

	damage(ch, victim, dam, sn, DAM_COLD, DAMF_SHOW);
}

void spell_colour_spray(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	dam = dice(level,3) + 13;
	if (saves_spell(level, victim, DAM_LIGHT))
		dam /= 2;
	else
		spellfun_call("blindness", NULL, level/2, ch, (void *) victim);

	damage(ch, victim, dam, sn, DAM_LIGHT, DAMF_SHOW);
}

#define OBJ_VNUM_LIGHT_BALL		21

void spell_continual_light(const char *sn, int level,CHAR_DATA *ch,void *vo)
{
	OBJ_DATA *light;

	if (target_name[0] != '\0')  /* do a glow on some object */
	{
		light = get_obj_carry(ch,target_name);

		if (light == NULL)
		{
		    act_char("You don't see that here.", ch);
		    return;
		}

		if (IS_OBJ_STAT(light,ITEM_GLOW))
		{
		    act("$p is already glowing.",ch,light,NULL,TO_CHAR);
		    return;
		}

		SET_OBJ_STAT(light, ITEM_GLOW);
		act("$p glows with a white light.",ch,light,NULL,TO_ALL);
		return;
	}

	light = create_obj(get_obj_index(OBJ_VNUM_LIGHT_BALL), 0);
	obj_to_room(light, ch->in_room);
	act("$n twiddles $s thumbs and $p appears.",   ch, light, NULL, TO_ROOM);
	act("You twiddle your thumbs and $p appears.", ch, light, NULL, TO_CHAR);
}

void spell_control_weather(const char *sn, int level,CHAR_DATA *ch,void *vo)
{
	if (!str_cmp(target_name, "better"))
		weather_info.change += dice(level / 3, 4);
	else if (!str_cmp(target_name, "worse"))
		weather_info.change -= dice(level / 3, 4);
	else  {
		act_char("Do you want it to get better or worse?", ch);
		return;
	}

	act_char("Ok.", ch);
}

#define OBJ_VNUM_MUSHROOM		20

void spell_create_food(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	OBJ_DATA *mushroom;

	mushroom = create_obj(get_obj_index(OBJ_VNUM_MUSHROOM), 0);
	INT(mushroom->value[0]) = level / 2;
	INT(mushroom->value[1]) = level;
	mushroom->level = level;
	obj_to_room(mushroom, ch->in_room);
	act("$p suddenly appears.", ch, mushroom, NULL, TO_ROOM);
	act("$p suddenly appears.", ch, mushroom, NULL, TO_CHAR);
}

#define OBJ_VNUM_ROSE			1001

void spell_create_rose(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	OBJ_DATA *rose = create_obj(get_obj_index(OBJ_VNUM_ROSE), 0);
	act("$n has created $p.", ch, rose, NULL, TO_ROOM);
	act("You create $p.", ch, rose, NULL, TO_CHAR);
	obj_to_char(rose, ch);
}

#define OBJ_VNUM_SPRING 		22

void spell_create_spring(const char *sn, int level,CHAR_DATA *ch,void *vo)
{
	OBJ_DATA *spring;

	spring = create_obj(get_obj_index(OBJ_VNUM_SPRING), 0);
	spring->timer = level;
	obj_to_room(spring, ch->in_room);
	act("$p flows from the ground.", ch, spring, NULL, TO_ROOM);
	act("$p flows from the ground.", ch, spring, NULL, TO_CHAR);
}

void spell_create_water(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	int water;

	if (obj->item_type != ITEM_DRINK_CON) {
		act_char("It is unable to hold water.", ch);
		return;
	}

	if (str_cmp(STR(obj->value[2]), "water")
	&&  INT(obj->value[1]) != 0) {
		act_char("It contains some other liquid.", ch);
		return;
	}

	water = UMIN(level * (weather_info.sky >= SKY_RAINING ? 4 : 2),
		     INT(obj->value[0]) - INT(obj->value[1]));

	if (water > 0) {
		STR_ASSIGN(obj->value[2], str_dup("water"));
		INT(obj->value[1]) += water;

		if (!IS_OBJ_NAME(obj, "water"))
			label_add(obj, "water");
		act("$p is filled.", ch, obj, NULL, TO_CHAR);
	}
}

void spell_cure_blindness(const char *sn, int level,CHAR_DATA *ch,void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (!is_affected(victim, "blindness")) {
		if (victim == ch)
		  act_char("You aren't blind.", ch);
		else
		  act("$N doesn't appear to be blinded.",ch,NULL,victim,TO_CHAR);
		return;
	}

	if (check_dispel(level, victim, "blindness")) {
		act_char("Your vision returns!", victim);
		act("$n is no longer blinded.",victim,NULL,NULL,TO_ROOM);
	}
	else
		act_char("Spell failed.", ch);
}

void spell_cure_critical(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int heal;

	heal = dice(3, 8) + level / 2 ;
	victim->hit = UMIN(victim->hit + heal, victim->max_hit);
	update_pos(victim);
	act_char("You feel better!", victim);
	if (ch != victim)
		act_char("Ok.", ch);
}

/* RT added to cure plague */
void spell_cure_disease(const char *sn, int level, CHAR_DATA *ch,void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (!is_affected(victim, "plague")) {
		if (victim == ch)
		  act_char("You aren't ill.", ch);
		else
		  act("$N doesn't appear to be diseased.",ch,NULL,victim,TO_CHAR);
		return;
	}

	if (check_dispel(level, victim, "plague"))
		act("$n looks relieved as $s sores vanish.",victim,NULL,NULL,TO_ROOM);
	else
		act_char("Spell failed.", ch);
}

void spell_cure_light(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int heal;

	heal = dice(1, 8) + level / 4 + 5;
	victim->hit = UMIN(victim->hit + heal, victim->max_hit);
	update_pos(victim);
	act_char("You feel better!", victim);
	if (ch != victim)
		act_char("Ok.", ch);
}

void spell_cure_poison(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (!is_affected(victim, "poison")) {
		if (victim == ch)
		  act_char("You aren't poisoned.", ch);
		else
		  act("$N doesn't appear to be poisoned.",ch,NULL,victim,TO_CHAR);
		return;
	}

	if (check_dispel(level, victim, "poison"))
	{
		act_char("A warm feeling runs through your body.", victim);
		act("$n looks much better.",victim,NULL,NULL,TO_ROOM);
	}
	else
		act_char("Spell failed.", ch);
}

void spell_cure_serious(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int heal;

	heal = dice(2, 8) + level / 3 + 10 ;
	victim->hit = UMIN(victim->hit + heal, victim->max_hit);
	update_pos(victim);
	act_char("You feel better!", victim);
	if (ch != victim)
		act_char("Ok.", ch);
}

void spell_curse(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim;
	AFFECT_DATA af;

	/* deal with the object case first */
	if (mem_is(vo, MT_OBJ)) {
		OBJ_DATA *obj = (OBJ_DATA *) vo;
		if (IS_OBJ_STAT(obj, ITEM_EVIL)) {
			act("$p is already filled with evil.",
			    ch, obj, NULL, TO_CHAR);
			return;
		}

		if (IS_OBJ_STAT(obj, ITEM_BLESS)) {
			AFFECT_DATA *paf = affect_find(obj->affected, "bless");
			int spell_level = paf != NULL ? paf->level : obj->level;

			if (!saves_dispel(level, spell_level, 0)) {
				if (paf != NULL)
					affect_remove_obj(obj,paf);
				act("$p glows with a red aura.",
				    ch, obj, NULL, TO_ALL);
				REMOVE_OBJ_STAT(obj, ITEM_BLESS);
				return;
			}

			act("The holy aura of $p is too powerful for you to overcome.", ch, obj, NULL, TO_CHAR);
			return;
		}

		af.where        = TO_OBJECT;
		af.type         = sn;
		af.level        = level;
		af.duration     = (8 + level / 5);
		INT(af.location)= APPLY_SAVES;
		af.modifier     = +1;
		af.bitvector    = ITEM_EVIL;
		af.owner	= NULL;
		affect_to_obj(obj,&af);

		act("$p glows with a malevolent aura.", ch, obj, NULL, TO_ALL);
		return;
	}

	/* character curses */
	victim = (CHAR_DATA *) vo;

	if (IS_AFFECTED(victim, AFF_CURSE)
	||  saves_spell(level, victim, DAM_NEGATIVE))
		return;

	af.where     = TO_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = (8 + level / 10);
	INT(af.location) = APPLY_HITROLL;
	af.modifier  = -1 * (level / 8);
	af.bitvector = AFF_CURSE;
	af.owner	= NULL;
	affect_to_char(victim, &af);

	INT(af.location) = APPLY_SAVING_SPELL;
	af.modifier  = level / 8;
	affect_to_char(victim, &af);

	act_char("You feel unclean.", victim);
	if (ch != victim)
		act("$N looks very uncomfortable.", ch, NULL, victim, TO_CHAR);
}

void spell_anathema(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	AFFECT_DATA af;
	CHAR_DATA* victim = (CHAR_DATA*) vo;
	int strength;

	strength = (ch->alignment - victim->alignment) / 200 * level / 30;
	if (strength < 0) strength = -strength;
	if (strength < 7) {
		act_puts("Oh, no. Your god seems to like $N.",
			 ch, NULL, victim, TO_CHAR, POS_DEAD);
		return;
	}

	if (is_affected(victim, sn)) {
		act_puts("$N is already cursed.",
			 ch, NULL, victim, TO_CHAR, POS_DEAD);
		return;
	}

	level += strength / 6;

	if (saves_spell(level, victim, DAM_HOLY)) {
		act_char("You failed.", ch);
		return;
	}

	af.where 	= TO_AFFECTS;
	af.type  	= sn;
	af.level 	= level;
	af.duration	= (8 + level/10);
	INT(af.location)= APPLY_HITROLL;
	af.modifier	= -strength;
	af.bitvector	= AFF_CURSE;
	af.owner	= NULL;
	affect_to_char(victim, &af);
	
	INT(af.location)= APPLY_SAVING_SPELL;
	af.modifier	= strength;
	affect_to_char(victim, &af);

	INT(af.location)= APPLY_LEVEL;
	af.modifier	= -strength / 7;
	affect_to_char(victim, &af);

	af.where	= TO_SKILLS;
	af.location.s = str_empty;
	af.modifier	= -strength;
	af.bitvector	= SK_AFF_ALL;
	affect_to_char(victim, &af);
	
	act("$n looks very uncomfortable.", victim, NULL, NULL, TO_ROOM);
	act_char("You feel unclean.", victim);
}

/* RT replacement demonfire spell */
void spell_demonfire(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	if (!IS_NPC(ch) && !IS_EVIL(ch))
	{
		victim = ch;
		act_char("The demons turn upon you!", ch);
	}

	if (victim != ch)
	{
		act("$n calls forth the demons of Hell upon $N!",
		    ch,NULL,victim,TO_ROOM);
		act("$n has assailed you with the demons of Hell!",
		    ch,NULL,victim,TO_VICT);
		act_char("You conjure forth the demons of hell!", ch);
	}
	dam = dice(level, 10);
	if (saves_spell(level, victim,DAM_NEGATIVE))
		dam /= 2;
	spellfun_call("curse", NULL, 3 * level / 4, ch, victim);
	damage(ch, victim, dam, sn, DAM_NEGATIVE, DAMF_SHOW);
}

/* added by chronos */
void spell_bluefire(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	if (!IS_NPC(ch) && !IS_NEUTRAL(ch)) {
		victim = ch;
		act_char("Your blue fire turn upon you!", ch);
	}

	if (victim != ch) {
		act("$n calls forth the blue fire of earth $N!",
		    ch,NULL,victim,TO_ROOM);
		act("$n has assailed you with the neutrals of earth!",
		    ch,NULL,victim,TO_VICT);
		act_char("You conjure forth the blue fire!", ch);
	}

	dam = dice(level, 10);
	if (saves_spell(level, victim,DAM_FIRE))
		dam /= 2;
	damage(ch, victim, dam, sn, DAM_FIRE, DAMF_SHOW);
}

void spell_detect_evil(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (HAS_DETECT(victim, ID_EVIL)) {
		if (victim == ch)
			act_char("You can already sense evil.", ch);
		else {
			act("$N can already detect evil.",
			    ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	af.where	= TO_DETECTS;
	af.type		= sn;
	af.level	= level;
	af.duration	= (5 + level / 3);
	af.modifier	= 0;
	INT(af.location)= APPLY_NONE;
	af.bitvector	= ID_EVIL;
	af.owner	= NULL;
	affect_to_char(victim, &af);
	act_char("Your eyes tingle.", victim);
	if (ch != victim)
		act_char("Ok.", ch);
}

void spell_detect_good(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (HAS_DETECT(victim, ID_GOOD)) {
		if (victim == ch)
			act_char("You can already sense good.", ch);
		else {
			act("$N can already detect good.",
			    ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	af.where	= TO_DETECTS;
	af.type		= sn;
	af.level	= level;
	af.duration	= (5 + level / 3);
	af.modifier	= 0;
	INT(af.location)= APPLY_NONE;
	af.bitvector	= ID_GOOD;
	af.owner	= NULL;
	affect_to_char(victim, &af);
	act_char("Your eyes tingle.", victim);
	if (ch != victim)
		act_char("Ok.", ch);
}

void spell_detect_hidden(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (HAS_DETECT(victim, ID_HIDDEN)) {
		if (victim == ch)
			act_char("You are already as alert as you can be.", ch);
		else {
			act("$N can already sense hidden lifeforms.",
			    ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	af.where	= TO_DETECTS;
	af.type		= sn;
	af.level	= level;
	af.duration	= 5 + level / 3;
	INT(af.location)= APPLY_NONE;
	af.modifier	= 0;
	af.bitvector	= ID_HIDDEN;
	af.owner	= NULL;
	affect_to_char(victim, &af);
	act_char("Your awareness improves.", victim);
	if (ch != victim)
		act_char("Ok.", ch);
}

void spell_detect_fade(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (HAS_DETECT(victim, ID_FADE)) {
		if (victim == ch)
			act_char("You are already as alert as you can be.", ch);
		else {
			act("$N can already sense faded lifeforms.",
			    ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	af.where	= TO_DETECTS;
	af.type		= sn;
	af.level	= level;
	af.duration	= 5 + level / 3;
	INT(af.location)= APPLY_NONE;
	af.modifier	= 0;
	af.bitvector	= ID_FADE;
	af.owner	= NULL;
	affect_to_char(victim, &af);
	act_char("Your awareness improves.", victim);
	if (ch != victim)
		act_char("Ok.", ch);
}

void spell_detect_invis(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (HAS_DETECT(victim, ID_INVIS)) {
		if (victim == ch)
			act_char("You can already see invisible.", ch);
		else {
			act("$N can already see invisible things.",
			    ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	af.where	= TO_DETECTS;
	af.type		= sn;
	af.level	= level;
	af.duration	= (5 + level / 3);
	af.modifier	= 0;
	INT(af.location)= APPLY_NONE;
	af.bitvector	= ID_INVIS;
	af.owner	= NULL;
	affect_to_char(victim, &af);
	act_char("Your eyes tingle.", victim);
	if (ch != victim)
		act_char("Ok.", ch);
}

void spell_detect_magic(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (HAS_DETECT(victim, ID_MAGIC)) {
		if (victim == ch)
			act_char("You can already sense magical auras.", ch);
		else {
			act("$N can already detect magic.",
			    ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	af.where	= TO_DETECTS;
	af.type		= sn;
	af.level	= level;
	af.duration	= (5 + level / 3);
	af.modifier	= 0;
	INT(af.location)= APPLY_NONE;
	af.bitvector	= ID_MAGIC;
	af.owner	= NULL;
	affect_to_char(victim, &af);
	act_char("Your eyes tingle.", victim);
	if (ch != victim)
		act_char("Ok.", ch);
}

void spell_true_seeing(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (HAS_DETECT(victim, ID_TRUESEEING)) {
		if (victim == ch)
			act_char("You can already see everything.", ch);
		else {
			act("$N can already see invisible things.",
			    ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	af.where	= TO_DETECTS;
	af.type		= sn;
	af.level	= level;
	af.duration	= (2 + level / 5);
	af.modifier	= 0;
	INT(af.location)= APPLY_NONE;
	af.bitvector	= ID_TRUESEEING;
	af.owner	= NULL;
	affect_to_char(victim, &af);
	act_char("Now you have a god-like sight.", victim);
	if (ch != victim)
		act_char("Ok.", ch);
}

void spell_detect_poison(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;

	if (obj->item_type == ITEM_DRINK_CON || obj->item_type == ITEM_FOOD) {
		if (INT(obj->value[3]) != 0)
			act_char("You smell poisonous fumes.", ch);
		else {
			act_puts("$p looks delicious.",
				 ch, obj, NULL, TO_CHAR, POS_DEAD);
		}
	} else {
		act_puts("$p doesn't look poisoned.",
			 ch, obj, NULL, TO_CHAR, POS_DEAD);
	}
}

void spell_dispel_evil(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	if (!IS_NPC(ch) && IS_EVIL(ch))
		victim = ch;

	if (IS_GOOD(victim)) {
		if (ch == victim)
			act_char("Gods protect you.", ch);
		else
			act("Gods protect $N.", ch, NULL, victim, TO_ROOM);
		return;
	}

	if (IS_NEUTRAL(victim)) {
		if (ch == victim)
			act_char("You do not seem to be affected.", ch);
		else {
			act("$N does not seem to be affected.",
			    ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	if (victim->hit > (level * 4))
		dam = dice(level, 4);
	else
		dam = UMAX(victim->hit, dice(level,4));
	if (saves_spell(level, victim,DAM_HOLY))
		dam /= 2;
	damage(ch, victim, dam, sn, DAM_HOLY, DAMF_SHOW);
}

void spell_dispel_good(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	if (!IS_NPC(ch) && IS_GOOD(ch))
		victim = ch;

	if (IS_EVIL(victim)) {
		if (ch == victim)
			act_char("You are protected by your evil.", ch);
		else {
			act("$N is protected by $S evil.",
			    ch, NULL, victim, TO_ROOM);
		}
		return;
	}

	if (IS_NEUTRAL(victim)) {
		if (ch == victim) 
			act_char("You do not seem to be affected.", ch);
		else {
			act("$N does not seem to be affected.",
			    ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	if (victim->hit > (level * 4))
		dam = dice(level, 4);
	else
		dam = UMAX(victim->hit, dice(level,4));
	if (saves_spell(level, victim,DAM_NEGATIVE))
		dam /= 2;
	damage(ch, victim, dam, sn, DAM_NEGATIVE, DAMF_SHOW);
}

static void *
earthquake_cb(void *vo, va_list ap)
{
	CHAR_DATA *vch = (CHAR_DATA *) vo;

	const char *sn = va_arg(ap, const char *);
	int level = va_arg(ap, int);
	CHAR_DATA *ch = va_arg(ap, CHAR_DATA *);

	if (vch->in_room == ch->in_room) {
		if (is_safe_spell(ch, vch, TRUE))
			return NULL;

		if (IS_AFFECTED(vch, AFF_FLYING))
			damage(ch, vch, 0, sn, DAM_BASH, DAMF_SHOW);
		else
			damage(ch, vch, level + dice(2, 8), sn,
			       DAM_BASH, DAMF_SHOW);
		return NULL;
	}

	if (vch->in_room->area == ch->in_room->area)
		act_char("The earth trembles and shivers.", vch);

	return NULL;
}

void spell_earthquake(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	act_char("The earth trembles beneath your feet!", ch);
	act("$n makes the earth tremble and shiver.", ch, NULL, NULL, TO_ROOM);
	vo_foreach(NULL, &iter_char_world, earthquake_cb, sn, level, ch);
}

void spell_enchant_armor(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	AFFECT_DATA *paf;
	int result, fail;
	int ac_bonus, added;
	bool ac_found = FALSE;
	bool hp_found = FALSE;

	if (obj->item_type != ITEM_ARMOR) {
		act_char("That isn't an armor.", ch);
		return;
	}

	if (obj->wear_loc != -1) {
		act_char("The item must be carried to be enchanted.", ch);
		return;
	}

	/* this means they have no bonus */
	ac_bonus = 0;
	fail = 25;	/* base 25% chance of failure */

	/* find the bonuses */

	if (!IS_OBJ_STAT(obj, ITEM_ENCHANTED))
		for (paf = obj->pObjIndex->affected; 
	 	     paf != NULL; 
		     paf = paf->next) {
			if (paf->where == TO_SKILLS
			||  paf->where == TO_RESIST
			||  paf->where == TO_FORMRESIST
			||  paf->where == TO_RACE)
				continue;

		    	if (INT(paf->location) == APPLY_AC) {
		    		ac_bonus = paf->modifier;
				ac_found = TRUE;
		    		fail += 5 * (ac_bonus * ac_bonus);
 	    		} else if (INT(paf->location) == APPLY_HIT) {
		    		fail += 30;
				hp_found = TRUE;
			} else 
				fail += 15;
		}
 
	for (paf = obj->affected; paf != NULL; paf = paf->next) {
		if (paf->where == TO_SKILLS
		||  paf->where == TO_RESIST
		||  paf->where == TO_FORMRESIST
		||  paf->where == TO_RACE)
			continue;

		if (INT(paf->location) == APPLY_AC) {
			ac_bonus = paf->modifier;
			ac_found = TRUE;
			fail += 5 * (ac_bonus * ac_bonus);
		} else if (INT(paf->location) == APPLY_HIT) {
			fail += 30;
			hp_found = TRUE;
		} else 
			fail += 15;
	}

	/* apply other modifiers */
	fail -= level;

	if (IS_OBJ_STAT(obj, ITEM_BLESS))
		fail -= 15;
	if (IS_OBJ_STAT(obj, ITEM_GLOW))
		fail -= 5;

	fail = URANGE(5,fail,85);

	result = number_percent();

	/* the moment of truth */
	if (result < (fail / 5)) { /* item destroyed */
		act("$p flares blindingly... and evaporates!",
		    ch, obj, NULL, TO_CHAR);
		act("$p flares blindingly... and evaporates!",
		    ch, obj, NULL, TO_ROOM);
		extract_obj(obj, 0);
		return;
	}

	if (result < (fail / 3)) { /* item disenchanted */
		AFFECT_DATA *paf_next;

		act("$p glows brightly, then fades...oops.",
		    ch, obj, NULL, TO_CHAR);
		act("$p glows brightly, then fades.", ch, obj, NULL, TO_ROOM);

		/* remove all affects */
		for (paf = obj->affected; paf != NULL; paf = paf_next) {
			paf_next = paf->next;
			aff_free(paf);
		}
		obj->affected = NULL;

		SET_OBJ_STAT(obj, ITEM_ENCHANTED);
		return;
	}

	if (result <= fail) { /* failed, no bad result */
		act_char("Nothing seemed to happen.", ch);
		return;
	}

	affect_enchant(obj);

	if (result <= (90 - level / 5)) { /* success! */
		act("$p shimmers with a gold aura.", ch, obj, NULL, TO_CHAR);
		act("$p shimmers with a gold aura.", ch, obj, NULL, TO_ROOM);
		SET_OBJ_STAT(obj, ITEM_MAGIC);
		added = -1;
	} else { /* exceptional enchant */
		act("$p glows a brillant gold!", ch, obj, NULL, TO_CHAR);
		act("$p glows a brillant gold!", ch, obj, NULL, TO_ROOM);
		SET_OBJ_STAT(obj, ITEM_MAGIC | ITEM_GLOW);
		added = -2;
	}
			
	/* now add the enchantments */

	if (obj->level < LEVEL_HERO)
		obj->level = UMIN(LEVEL_HERO - 1,obj->level + 1);

	if (ac_found) {
		for (paf = obj->affected; paf != NULL; paf = paf->next) {
			if (paf->where == TO_SKILLS
			||  paf->where == TO_RESIST
			||  paf->where == TO_FORMRESIST
			||  paf->where == TO_RACE)
				continue;

			if (INT(paf->location) == APPLY_AC) {
				free_string(paf->type);
				paf->type = str_dup(sn);
				paf->modifier += added;
				paf->level = UMAX(paf->level, level);
			}
		}
	} else { /* add a new affect */
		paf = aff_new();

		paf->where	= TO_OBJECT;
		paf->type	= str_dup(sn);
		paf->level	= level;
		paf->duration	= -1;
		INT(paf->location) = APPLY_AC;
		paf->modifier	= added;
		paf->bitvector  = 0;
		paf->next	= obj->affected;
		obj->affected	= paf;
	}

	if (hp_found) {
		for (paf = obj->affected; paf != NULL; paf = paf->next) {
			if (paf->where == TO_SKILLS
			||  paf->where == TO_RESIST
			||  paf->where == TO_FORMRESIST
			||  paf->where == TO_RACE)
				continue;

			if (INT(paf->location) == APPLY_HIT) {
				free_string(paf->type);
				paf->type = str_dup(sn);
				paf->modifier -= added;
				paf->level = UMAX(paf->level,level);
			}
		}
	} else {
		paf = aff_new();

		paf->where	= TO_OBJECT;
		paf->type	= str_dup(sn);
		paf->level	= level;
		paf->duration	= -1;
		INT(paf->location) = APPLY_HIT;
		paf->modifier	= -added;
		paf->bitvector  = 0;
		paf->next	= obj->affected;
		obj->affected	= paf;
	}

}

void spell_enchant_weapon(const char *sn, int level,CHAR_DATA *ch, void *vo)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	AFFECT_DATA *paf;
	int result, fail;
	int hit_bonus, dam_bonus, added;
	bool hit_found = FALSE, dam_found = FALSE;

	if (obj->item_type != ITEM_WEAPON) {
		act_char("That isn't a weapon.", ch);
		return;
	}

	if (obj->wear_loc != -1) {
		act_char("The item must be carried to be enchanted.", ch);
		return;
	}

	/* this means they have no bonus */
	hit_bonus = 0;
	dam_bonus = 0;
	fail = 25;	/* base 25% chance of failure */

	/* find the bonuses */

	if (!IS_OBJ_STAT(obj, ITEM_ENCHANTED))
		for (paf = obj->pObjIndex->affected; paf != NULL; paf = paf->next) {
			if (paf->where == TO_SKILLS
			||  paf->where == TO_RESIST
			||  paf->where == TO_FORMRESIST
			||  paf->where == TO_RACE)
				continue;

			if (INT(paf->location) == APPLY_HITROLL) {
				hit_bonus = paf->modifier;
				hit_found = TRUE;
				fail += 2 * (hit_bonus * hit_bonus);
			} else if (INT(paf->location) == APPLY_DAMROLL) {
				dam_bonus = paf->modifier;
				dam_found = TRUE;
				fail += 2 * (dam_bonus * dam_bonus);
			} else  /* things get a little harder */
			fail += 25;
		}
 
	for (paf = obj->affected; paf != NULL; paf = paf->next) {
		if (paf->where == TO_SKILLS
		||  paf->where == TO_RESIST
		||  paf->where == TO_FORMRESIST
		||  paf->where == TO_RACE)
			continue;

		if (INT(paf->location) == APPLY_HITROLL) {
			hit_bonus = paf->modifier;
			hit_found = TRUE;
			fail += 2 * (hit_bonus * hit_bonus);
		} else if (INT(paf->location) == APPLY_DAMROLL) {
			dam_bonus = paf->modifier;
			dam_found = TRUE;
			fail += 2 * (dam_bonus * dam_bonus);
		} else /* things get a little harder */
			fail += 25;
	}

	/* apply other modifiers */
	fail -= 3 * level/2;

	if (IS_OBJ_STAT(obj,ITEM_BLESS))
		fail -= 15;
	if (IS_OBJ_STAT(obj,ITEM_GLOW))
		fail -= 5;

	fail = URANGE(5, fail, 95);

	result = number_percent();

	/* the moment of truth */
	if (result < (fail / 5)) { /* item destroyed */
		act("$p shivers violently and explodes!",
		    ch, obj, NULL, TO_CHAR);
		act("$p shivers violently and explodeds!",
		    ch, obj, NULL, TO_ROOM);
		extract_obj(obj, 0);
		return;
	}

	if (result < (fail / 2)) { /* item disenchanted */
		AFFECT_DATA *paf_next;

		act("$p glows brightly, then fades...oops.",
		    ch, obj, NULL, TO_CHAR);
		act("$p glows brightly, then fades.",
		    ch, obj, NULL, TO_ROOM);

		/* remove all affects */
		for (paf = obj->affected; paf != NULL; paf = paf_next) {
			paf_next = paf->next; 
			aff_free(paf);
		}
		obj->affected = NULL;

		SET_OBJ_STAT(obj, ITEM_ENCHANTED);
		return;
	}

	if (result <= fail) { /* failed, no bad result */
		act_char("Nothing seemed to happen.", ch);
		return;
	}

	affect_enchant(obj);

	if (result <= (100 - level/5)) { /* success! */
		act("$p glows blue.", ch, obj, NULL, TO_CHAR);
		act("$p glows blue.", ch, obj, NULL, TO_ROOM);
		SET_OBJ_STAT(obj, ITEM_MAGIC);
		added = 1;
	} else { /* exceptional enchant */
		act("$p glows a brillant blue!", ch, obj, NULL, TO_CHAR);
		act("$p glows a brillant blue!", ch, obj, NULL, TO_ROOM);
		SET_OBJ_STAT(obj, ITEM_MAGIC | ITEM_GLOW);
		added = 2;
	}
			
	/* now add the enchantments */ 

	if (obj->level < LEVEL_HERO - 1)
		obj->level = UMIN(LEVEL_HERO - 1, obj->level + 1);

	if (dam_found) {
		for (paf = obj->affected; paf != NULL; paf = paf->next) {
			if (paf->where == TO_SKILLS
			||  paf->where == TO_RESIST
			||  paf->where == TO_FORMRESIST
			||  paf->where == TO_RACE)
				continue;

			if (INT(paf->location) == APPLY_DAMROLL) {
				free_string(paf->type);
				paf->type = str_dup(sn);
				paf->modifier += added;
				paf->level = UMAX(paf->level, level);
				if (paf->modifier > 4)
					SET_OBJ_STAT(obj, ITEM_HUM);
			}
		}
	} else { /* add a new affect */
		paf = aff_new();

		paf->where	= TO_OBJECT;
		paf->type	= str_dup(sn);
		paf->level	= level;
		paf->duration	= -1;
		INT(paf->location) = APPLY_DAMROLL;
		paf->modifier	= added;
		paf->bitvector  = 0;
		paf->next	= obj->affected;
		obj->affected	= paf;
	}

	if (hit_found) {
		for (paf = obj->affected; paf != NULL; paf = paf->next) {
			if (INT(paf->location) == APPLY_HITROLL) {
				free_string(paf->type);
				paf->type = str_dup(sn);
				paf->modifier += added;
				paf->level = UMAX(paf->level, level);
				if (paf->modifier > 4)
				SET_OBJ_STAT(obj, ITEM_HUM);
			}
		}
	} else { /* add a new affect */
		paf = aff_new();
 
		paf->type       = str_dup(sn);
		paf->level      = level;
		paf->duration   = -1;
		INT(paf->location) = APPLY_HITROLL;
		paf->modifier   = added;
		paf->bitvector  = 0;
		paf->next       = obj->affected;
		obj->affected   = paf;
	}
}

/*
 * Drain XP, MANA, HP.
 * Caster gains HP.
 */
void spell_energy_drain(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
	int dam;

	if (saves_spell(level, victim, DAM_NEGATIVE)) {
		act_char("You feel a momentary chill.", victim);
		return;
	}

	if (victim->level <= 2) {
		dam		 = ch->hit + 1;
	} else {
		if (!IS_NPC(victim))
			gain_exp(victim, 0 - number_range(level/5, 3*level/5));
		victim->mana	/= 2;
		victim->move	/= 2;
		dam		 = dice(1, level);
		ch->hit		+= dam;
	}

	if (number_percent() < 15) {
		af.where 		= TO_AFFECTS;
		af.type			= sn;
		af.level		= level/2;
		af.duration		= 6+level/12;
		INT(af.location)	= APPLY_LEVEL;
		af.modifier		= -1;
		af.bitvector		= 0;
		af.owner		= NULL;

		affect_join(victim, &af);
	}

	act_char("You feel your life slipping away!", victim);
	act_char("Wow....what a rush!", ch);
	damage(ch, victim, dam, sn, DAM_NEGATIVE, DAMF_SHOW);
}

void spell_mana_drain(const char *sn, int level,CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim=(CHAR_DATA *)vo;
	int manadam;

	act("$n spreads his arms towards $N...", ch, NULL, victim, TO_ROOM);
	act("You spread your arms towards $N...", ch, NULL, victim, TO_CHAR);

	if (saves_spell(level+number_range(1,20), victim, DAM_MENTAL)) {
	    act("   ... but nothing happens.", ch, NULL, victim, TO_ROOM);
	    act("   ... but nothing happens.", ch, NULL, victim, TO_CHAR);
	    return;
	}

	act("Many glowing golden threads link $n's fingers to $N!",
	    ch, NULL, victim, TO_ROOM);
	act("Many glowing golden threads link your fingers to $N!",
	    ch, NULL, victim, TO_CHAR);
	manadam=dice(level,11);
	if (saves_spell(level, victim, DAM_NEGATIVE)) manadam /=3;
	manadam = UMIN(manadam, victim->mana);
	victim->mana -= manadam;
	ch->mana += manadam/2;

}

void spell_draining_touch(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim=(CHAR_DATA *)vo;
	AFFECT_DATA af;
	int dam, gdam;

	gdam = dam = dice (level, 3) + 10;
	if (saves_spell(level, victim, DAM_NEGATIVE)) {
		dam /=3;
		gdam=0;
	}

	ch->hit += gdam;
	act("$n touches $N with $s fingers.", ch, NULL, victim, TO_NOTVICT);
	act("$n touches you with $s fingers.", ch, NULL, victim, TO_VICT);

	damage(ch, victim, dam, sn, DAM_NEGATIVE, DAMF_SHOW);
	if (IS_EXTRACTED(victim))
		return;

	if (!is_affected(victim, sn)
	&&  !saves_spell(level-5, victim, DAM_NEGATIVE)) {
		af.where	= TO_AFFECTS;
		af.type		= sn;
		af.level	= level;
		af.duration	= 1;
		INT(af.location)= APPLY_LEVEL;
		af.modifier	= -1*level/15;
		af.bitvector	= 0;
		af.owner	= NULL;
		affect_to_char(victim, &af);

		act("You feel drained!", victim, NULL, NULL, TO_VICT);
	}
}

void spell_hellfire(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	damage(ch, victim, dice(level, 7), sn, DAM_FIRE, DAMF_SHOW);
}

static void *
iceball_cb(void *vo, va_list ap)
{
	CHAR_DATA *vch = (CHAR_DATA *) vo;

	const char *sn = va_arg(ap, const char *);
	int level = va_arg(ap, int);
	CHAR_DATA *ch = va_arg(ap, CHAR_DATA *);
	int *pdam;
	int movedam;

	if (is_safe_spell(ch, vch, TRUE))
		return NULL;

	pdam = va_arg(ap, int *);
	movedam = va_arg(ap, int);

	if (saves_spell(level, vch, DAM_COLD))
		*pdam /= 2;
	vch->move -= UMIN(vch->move, movedam);
	damage(ch, vch, *pdam, sn, DAM_COLD, DAMF_SHOW);
	return NULL;
}

void spell_iceball(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	int dam = dice(level, 12);
	vo_foreach(ch->in_room, &iter_char_room, iceball_cb,
		   sn, level, ch, &dam, number_range(level, 2 * level));
}

void spell_fireball(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	dam = dice(level, 11);
	if (saves_spell(level, victim, DAM_FIRE))
		dam /= 2;
	damage(ch, victim, dam, sn, DAM_FIRE, DAMF_SHOW);
}

void spell_fireproof(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	AFFECT_DATA af;

	if (IS_OBJ_STAT(obj, ITEM_BURN_PROOF)) {
		act("$p is already protected from burning.",
		    ch, obj, NULL, TO_CHAR);
		return;
	}

	af.where	= TO_OBJECT;
	af.type		= sn;
	af.level	= level;
	af.duration	= number_fuzzy(level / 4);
	INT(af.location)= APPLY_NONE;
	af.modifier	= 0;
	af.bitvector	= ITEM_BURN_PROOF;
	af.owner	= NULL;

	affect_to_obj(obj, &af);

	act("You protect $p from fire.", ch, obj, NULL, TO_CHAR);
	act("$p is surrounded by a protective aura.", ch, obj, NULL, TO_ROOM);
}

void spell_flamestrike(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	dam = dice(level, 10);
	if (saves_spell(level, victim,DAM_FIRE))
		dam /= 2;
	damage(ch, victim, dam, sn, DAM_FIRE, DAMF_SHOW);
}

void spell_faerie_fire(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (IS_AFFECTED(victim, AFF_FAERIE_FIRE))
		return;
	af.where     = TO_AFFECTS;
	af.type      = sn;
	af.level	 = level;
	af.duration  = 10 + level / 5;
	INT(af.location) = APPLY_AC;
	af.modifier  = 2 * level;
	af.bitvector = AFF_FAERIE_FIRE;
	af.owner	= NULL;
	affect_to_char(victim, &af);
	act_char("You are surrounded by a pink outline.", victim);
	act("$n is surrounded by a pink outline.", victim, NULL, NULL, TO_ROOM);
}

void spell_faerie_fog(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *ich;

	act("$n conjures a cloud of purple smoke.", ch, NULL, NULL, TO_ROOM);
	act_char("You conjure a cloud of purple smoke.", ch);

	for (ich = ch->in_room->people; ich != NULL; ich = ich->next_in_room) {
		if (ich->invis_level > 0)
			continue;

		if (ich == ch || saves_spell(level, ich, DAM_OTHER))
			continue;

		affect_bit_strip(ich, TO_INVIS, ID_ALL_INVIS);
		REMOVE_BIT(ich->affected_by, ID_ALL_INVIS);

		act("$n is revealed!", ich, NULL, NULL, TO_ROOM);
		act_char("You are revealed!", ich);
	}
}

#define OBJ_VNUM_DISC			23

void spell_floating_disc(const char *sn, int level,CHAR_DATA *ch,void *vo)
{
	OBJ_DATA *disc, *floating;
	AFFECT_DATA af;

	floating = get_eq_char(ch, WEAR_FLOAT);
	if (floating != NULL && IS_OBJ_STAT(floating, ITEM_NOREMOVE)) {
		act("You can't remove $p.", ch, floating, NULL, TO_CHAR);
		return;
	}

	disc = create_obj(get_obj_index(OBJ_VNUM_DISC), 0);
	INT(disc->value[0]) = level * 10; /* 10 pounds per level capacity */
	INT(disc->value[3]) = level * 5; /* 5 pounds per level max per item */
	disc->timer	= level / 2 - number_range(0, level / 4); 

	af.where	= TO_AFFECTS;
	af.type		= sn;
	af.level	= level;
	af.duration	= disc->timer;
	INT(af.location)= 0;
	af.modifier	= 0;
	af.bitvector	= 0;
	af.owner	= NULL;
	affect_to_char(ch, &af);

	act("$n has created a floating black disc.", ch, NULL, NULL, TO_ROOM);
	act_char("You create a floating disc.", ch);
	obj_to_char(disc, ch);
	equip_char(ch, disc, WEAR_FLOAT);
}

void spell_fly(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (IS_AFFECTED(victim, AFF_FLYING))
	{
		if (victim == ch)
		  act_char("You are already airborne.", ch);
		else
		  act("$N doesn't need your help to fly.",ch,NULL,victim,TO_CHAR);
		return;
	}
	af.where     = TO_AFFECTS;
	af.type      = sn;
	af.level	 = level;
	af.duration  = level + 3;
	INT(af.location) = 0;
	af.modifier  = 0;
	af.bitvector = AFF_FLYING;
	af.owner	= NULL;
	affect_to_char(victim, &af);
	act_char("Your feet rise off the ground.", victim);
	act("$n's feet rise off the ground.", victim, NULL, NULL, TO_ROOM);
}

/* RT clerical berserking spell */
void spell_frenzy(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (is_affected(victim, sn) || IS_AFFECTED(victim, AFF_BERSERK)) {
		if (victim == ch)
			act_char("You are already in a frenzy.", ch);
		else
			act("$N is already in a frenzy.",
			    ch, NULL, victim, TO_CHAR);
		return;
	}

	if (is_affected(victim, "calm")) {
		if (victim == ch)
			act_char("Why don't you just relax for a while?", ch);
		else
			act("$N doesn't look like $e wants to fight anymore.",
			    ch, NULL, victim, TO_CHAR);
		return;
	}

	if ((IS_GOOD(ch) && !IS_GOOD(victim))
	||  (IS_NEUTRAL(ch) && !IS_NEUTRAL(victim))
	||  (IS_EVIL(ch) && !IS_EVIL(victim))) {
		act("Your god doesn't seem to like $N.",
		    ch, NULL, victim, TO_CHAR);
		return;
	}

	af.where	= TO_AFFECTS;
	af.type		= sn;
	af.level	= level;
	af.duration	= level / 3;
	af.modifier	= level / 6;
	af.bitvector	= 0;
	af.owner	= NULL;

	INT(af.location)= APPLY_HITROLL;
	affect_to_char(victim,&af);

	INT(af.location)= APPLY_DAMROLL;
	affect_to_char(victim,&af);

	af.modifier	= 10 * (level / 12);
	INT(af.location)= APPLY_AC;
	affect_to_char(victim,&af);

	act_char("You are filled with holy wrath!", victim);
	act("$n gets a wild look in $s eyes!", victim, NULL, NULL, TO_ROOM);
}

static inline void
gate(CHAR_DATA *ch, CHAR_DATA *victim)
{
	transfer_char(ch, NULL, victim->in_room,
		      "$N steps through a gate and vanishes.",
		      "You step through a gate and vanish.",
		      "$N has arrived through a gate.");
}

void spell_gate(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim;
	CHAR_DATA *pet = NULL;

	if ((victim = get_char_world(ch, target_name)) == NULL
	||  LEVEL(victim) >= level + 3
	||  saves_spell(level, victim, DAM_OTHER)
	||  !can_gate(ch, victim)) {
		act_char("You failed.", ch);
		return;
	}

	pet = GET_PET(ch);
	if (pet && pet->in_room != ch->in_room)
		pet = NULL;

	gate(ch, victim);
	if (pet && !IS_AFFECTED(pet, AFF_SLEEP)) {
		if (pet->position != POS_STANDING)
			dofun("stand", pet, str_empty);
		gate(pet, victim);
	}
}

void spell_giant_strength(const char *sn, int level,CHAR_DATA *ch,void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (is_affected(victim, sn))
	{
		if (victim == ch)
		  act_char("You are already as strong as you can get!", ch);
		else
		  act("$N can't get any stronger.",ch,NULL,victim,TO_CHAR);
		return;
	}

	af.where     = TO_AFFECTS;
	af.type      = sn;
	af.level	 = level;
	af.duration  = (10 + level / 3);
	INT(af.location) = APPLY_STR;
	af.modifier  = UMAX(2,level / 10);
	af.bitvector = 0;
	af.owner	= NULL;
	affect_to_char(victim, &af);
	act_char("Your muscles surge with heightened power!", victim);
	act("$n's muscles surge with heightened power.",victim,NULL,NULL,TO_ROOM);
}

void spell_harm(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	dam = UMAX( 20, victim->hit - dice(1,4));
	if (saves_spell(level, victim,DAM_HARM))
		dam = UMIN(50, dam / 2);
	dam = UMIN(100, dam);
	damage(ch, victim, dam, sn, DAM_HARM, DAMF_SHOW);
}

/* RT haste spell */

void spell_haste(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;
 
	if (is_affected(victim, sn)
	||  IS_AFFECTED(victim,AFF_HASTE)
	||  (IS_NPC(victim) && IS_SET(victim->pMobIndex->off_flags, OFF_FAST))) {
		if (victim == ch)
			act_char("You can't move any faster!", ch);
		else
			act("$N is already moving as fast as $E can.",
			    ch, NULL, victim, TO_CHAR);
		return;
	}

	if (IS_AFFECTED(victim, AFF_SLOW)) {
		if (!check_dispel(level, victim, "slow")) {
			if (victim != ch)
				act_char("Spell failed.", ch);
			act_char("You feel momentarily faster.", victim);
			return;
		}
		act("$n is moving less slowly.", victim, NULL, NULL, TO_ROOM);
		return;
	}

	af.where     = TO_AFFECTS;
	af.type      = sn;
	af.level     = level;
	if (victim == ch)
		af.duration  = level/2;
	else
		af.duration  = level/4;
	INT(af.location) = APPLY_DEX;
	af.modifier  = UMAX(2,level / 12);
	af.bitvector = AFF_HASTE;
	af.owner	= NULL;
	affect_to_char(victim, &af);
	act_char("You feel yourself moving more quickly.", victim);
	act("$n is moving more quickly.", victim, NULL, NULL, TO_ROOM);
	if (ch != victim)
		act_char("Ok.", ch);
}

void spell_heal(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	victim->hit = UMIN(victim->hit + 100 + level / 10, victim->max_hit);
	update_pos(victim);
	act_char("A warm feeling fills your body.", victim);
	if (ch != victim)
		act_char("Ok.", ch);
}

void spell_restoration(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if(is_affected(ch, sn)) {
		act("You do not have enough power.", ch, NULL, NULL, TO_CHAR);
		return;
	}
	victim->hit = victim->max_hit;
	update_pos(victim);
	act("A warm feeling fills your body.", victim, NULL, NULL, TO_CHAR);
	act("A bleeding wounds on $N's body vanish.", NULL, victim, NULL, TO_NOTVICT);

	af.where 	= TO_AFFECTS;
	af.type  	= sn;
	af.level 	= level;
	af.duration	= 5;
	af.bitvector	= 0;
	af.modifier	= 0;
	INT(af.location)= APPLY_NONE;
	af.owner	= NULL;

	affect_to_char(ch, &af);
}

#define OBJ_VNUM_HOLY_HAMMER		18

void spell_holy_hammer(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	OBJ_DATA *hammer;
	AFFECT_DATA af;

	hammer = create_obj(get_obj_index(OBJ_VNUM_HOLY_HAMMER), 0);
	hammer->level = ch->level;
	hammer->timer = level * 3;
	INT(hammer->value[2]) = (level / 10) + 1;

	af.where 	= TO_OBJECT;
	af.type  	= sn;
	af.level 	= level;
	af.duration	= -1;
	af.bitvector	= 0;
	af.modifier	= level/7 +3;
	af.owner	= NULL;

	INT(af.location)= APPLY_HITROLL;
	affect_to_obj(hammer, &af);

	INT(af.location)= APPLY_DAMROLL;
	affect_to_obj(hammer, &af);

	obj_to_char(hammer, ch);

	act ("You create a Holy Hammer.", ch, NULL, NULL, TO_CHAR);
	act ("$n creates a Holy Hammer.", ch, NULL, NULL, TO_ROOM);
}

void spell_hold_person(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA* victim = (CHAR_DATA*) vo;
	AFFECT_DATA af;
	
	if (saves_spell(level + 2, victim, DAM_OTHER)) {
		act_char("You failed.", ch);
		act("$n tries to hold you, but fails.", ch, victim, NULL, TO_VICT);
		return;
	}

	af.where 	= TO_AFFECTS;
	af.type		= sn;
	af.duration	= 1;
	af.level	= level;
	af.bitvector	= 0;
	af.modifier	= -level/12;
	INT(af.location)= APPLY_DEX;
	af.owner	= NULL;
	affect_to_char(victim, &af);
}

void spell_heat_metal(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	OBJ_DATA *obj_lose, *obj_next;
	int dam = 0;

	if (saves_spell(level + 2, victim, DAM_FIRE)
	||  victim->shapeform) {
		act_char("Your spell had no effect.", ch);
		act_char("You feel momentarily warmer.", victim);
		return;
	}

	for (obj_lose = victim->carrying; obj_lose != NULL; obj_lose = obj_next) {
		obj_next = obj_lose->next_content;

		if (number_range(1, 2 * level) < obj_lose->level
		||  saves_spell(level, victim, DAM_FIRE)
		||  !material_is(obj_lose, MATERIAL_METAL)
		||  IS_OBJ_STAT(obj_lose,ITEM_BURN_PROOF))
			continue;

		switch (obj_lose->item_type) {
		case ITEM_ARMOR:
			if (obj_lose->wear_loc == -1) {
				/*
				 * in inventory -- drop it if we can
				 */
				if (can_drop_obj(victim, obj_lose)) {
					act("$n yelps and throws $p to the ground!",
					     victim, obj_lose, NULL, TO_ROOM);
					act("You and drop $p before it burns you.",
					    victim, obj_lose, NULL, TO_CHAR);
					dam += number_range(1,obj_lose->level) / 6;
					obj_from_char(obj_lose);
					obj_to_room(obj_lose, victim->in_room);
				} else { /* cannot drop */
					act("Your skin is seared by $p!",
					    victim, obj_lose, NULL, TO_CHAR);
					dam += number_range(1, obj_lose->level) / 2;
			    }
			    break;
			}

			/*
			 * worn
			 */
			if (can_drop_obj(victim,obj_lose)
			&&  obj_lose->weight / 10 < number_range(1, 2 * get_curr_stat(victim, STAT_DEX))
			&&  remove_obj(victim, obj_lose->wear_loc, TRUE)) {
				act("$n yelps and throws $p to the ground!",
				    victim, obj_lose, NULL, TO_ROOM);
				act("You remove and drop $p before it burns you.",
				    victim, obj_lose, NULL, TO_CHAR);
				dam += number_range(1,obj_lose->level) / 3;
				obj_from_char(obj_lose);
				obj_to_room(obj_lose, victim->in_room);
			} else { /* stuck on the body! ouch! */
				act("Your skin is seared by $p!",
				    victim, obj_lose, NULL, TO_CHAR);
				dam += number_range(1, obj_lose->level);
			}
			break;

		case ITEM_WEAPON:
			if (IS_WEAPON_STAT(obj_lose, WEAPON_FLAMING)
			||  obj_lose->wear_loc == WEAR_STUCK_IN)
				continue;

			if (obj_lose->wear_loc == -1) {
				if (can_drop_obj(victim, obj_lose)) {
					act("$n throws a burning hot $p to the ground!",
					    victim, obj_lose, NULL, TO_ROOM);
					act("You and drop $p before it burns you.",
					    victim, obj_lose, NULL, TO_CHAR);
					dam += number_range(1, obj_lose->level) / 6;
					obj_from_char(obj_lose);
					obj_to_room(obj_lose, victim->in_room);
				} else { /* cannot drop */
					act("Your skin is seared by $p!",
					    victim, obj_lose, NULL, TO_CHAR);
					dam += number_range(1,obj_lose->level) / 2;
				}
			}

			if (can_drop_obj(victim, obj_lose)
			&&  remove_obj(victim, obj_lose->wear_loc, TRUE)) {
				act("$n is burned by $p, and throws it to the ground.",
				    victim, obj_lose, NULL, TO_ROOM);
				act_char("You throw your red-hot weapon to the ground!", victim);
				dam += 1;
				obj_from_char(obj_lose);
				obj_to_room(obj_lose, victim->in_room);
			} else { /* YOWCH! */
				act_char("Your weapon sears your flesh!", victim);
				dam += number_range(1,obj_lose->level);
			}
			break;
		}
	}

	if (!dam) {
		act_char("Your spell had no effect.", ch);
		act_char("You feel momentarily warmer.", victim);
	} else { /* damage! */
		if (saves_spell(level, victim, DAM_FIRE))
			dam = 2 * dam / 3;
		damage(ch, victim, dam, sn, DAM_FIRE, DAMF_SHOW);
	}
}

static void *
holy_word_cb(void *vo, va_list ap)
{
	CHAR_DATA *vch = (CHAR_DATA *) vo;

	const char *sn = va_arg(ap, const char *);
	int level = va_arg(ap, int);
	CHAR_DATA *ch = va_arg(ap, CHAR_DATA *);

	if ((IS_GOOD(ch) && IS_GOOD(vch)) ||
	    (IS_EVIL(ch) && IS_EVIL(vch)) ||
	    (IS_NEUTRAL(ch) && IS_NEUTRAL(vch))) {
		act_char("You feel full more powerful.", vch);
		spellfun_call("frenzy", NULL, level, ch, vch);
		spellfun_call("bless", NULL, level, ch, vch);
		return NULL;
	}

	if (is_safe_spell(ch, vch, TRUE))
		return NULL;

	if ((IS_GOOD(ch) && IS_EVIL(vch))
	||  (IS_EVIL(ch) && IS_GOOD(vch))) {
		spellfun_call("curse", NULL, level, ch, vch);
		act_char("You are struck down!", vch);
		damage(ch, vch, dice(level, 6), sn, DAM_ENERGY, DAMF_SHOW);
		return NULL;
	}

	if (IS_NEUTRAL(ch)) {
		spellfun_call("curse", NULL, level/2, ch, vch);
		act_char("You are struck down!", vch);
		damage(ch, vch, dice(level, 4), sn, DAM_ENERGY, DAMF_SHOW);
		return NULL;
	}

	return NULL;
}

/* RT really nasty high-level attack spell */
void spell_holy_word(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	act("$n utters a word of divine power!", ch, NULL, NULL, TO_ROOM);
	act_char("You utter a word of divine power.", ch);
	vo_foreach(ch->in_room, &iter_char_room, holy_word_cb,
		   sn, level, ch);
	act_puts("You feel drained.", ch, NULL, NULL, TO_CHAR, POS_DEAD);
	if (!IS_NPC(ch))
		gain_exp(ch, -1 * number_range(1, 10) * 5);
	ch->move = ch->move * 3 / 4;
	ch->hit = ch->hit * 3 / 4;
}

void spell_identify(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	BUFFER *output;

	output = buf_new(-1);
	format_obj(output, obj);
	if (!IS_OBJ_STAT(obj, ITEM_ENCHANTED))
		format_obj_affects(output, obj->pObjIndex->affected,
				   FOA_F_NODURATION | FOA_F_NOAFFECTS);
	format_obj_affects(output, obj->affected, FOA_F_NOAFFECTS);
	page_to_char(buf_string(output), ch);
	buf_free(output);
}

void spell_improved_identify(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	BUFFER *output;

	output = buf_new(-1);
	format_obj(output, obj);
	if (!IS_OBJ_STAT(obj, ITEM_ENCHANTED))
		format_obj_affects(output, obj->pObjIndex->affected,
					FOA_F_NODURATION);
	format_obj_affects(output, obj->affected, 0);
	page_to_char(buf_string(output), ch);
	buf_free(output);
}

void spell_infravision(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (HAS_DETECT(victim, ID_INFRARED)) {
		if (victim == ch)
			act_char("You can already see in the dark.", ch);
		else {
			act("$N already has infravision.",
			    ch, NULL, victim,TO_CHAR);
		}
		return;
	}

	act_char("Your eyes glow red.", victim);
	act("$n's eyes glow red.\n", ch, NULL, NULL, TO_ROOM);

	af.where	= TO_DETECTS;
	af.type		= sn;
	af.level	= level;
	af.duration	= 2 * level;
	INT(af.location)= APPLY_NONE;
	af.modifier	= 0;
	af.bitvector	= ID_INFRARED;
	af.owner	= NULL;
	affect_to_char(victim, &af);
}

void spell_invisibility(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim;
	AFFECT_DATA af;

	/* object invisibility */
	if (mem_is(vo, MT_OBJ)) {
		OBJ_DATA *obj = (OBJ_DATA *) vo;

		if (IS_OBJ_STAT(obj, ITEM_INVIS)) {
			act("$p is already invisible.", ch, obj, NULL, TO_CHAR);
			return;
		}

		af.where	= TO_OBJECT;
		af.type		= sn;
		af.level	= level;
		af.duration	= level / 4 + 12;
		INT(af.location)= APPLY_NONE;
		af.modifier	= 0;
		af.bitvector	= ITEM_INVIS;
		af.owner	= NULL;
		affect_to_obj(obj,&af);

		act("$p fades out of sight.", ch, obj, NULL, TO_ALL);
		return;
	}

	/* character invisibility */
	victim = (CHAR_DATA *) vo;

	if (HAS_INVIS(victim, ID_INVIS))
		return;

	act("$n fades out of existence.", victim, NULL, NULL, TO_ROOM);
	act_char("You fade out of existence.", victim);

	af.where	= TO_INVIS;
	af.type		= sn;
	af.level	= level;
	af.duration	= (level / 8 + 10);
	INT(af.location)= APPLY_NONE;
	af.modifier	= 0;
	af.bitvector	= ID_INVIS;
	af.owner	= NULL;
	affect_to_char(victim, &af);
}

void spell_know_alignment(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	char *msg;

		 if (IS_GOOD(victim)) msg = "$N has a pure and good aura.";
	else if (IS_NEUTRAL(victim)) msg = "$N act as no align.";
	else msg = "$N is the embodiment of pure evil!.";

	act(msg, ch, NULL, victim, TO_CHAR);

	if (!IS_NPC(victim)) 
	{
	 if (victim->ethos == 1)		msg = "$N upholds the laws.";
	 else if (victim->ethos == 2) 	msg = "$N seems ambivalent to society.";
	 else if (victim->ethos == 3) 	msg = "$N seems very chaotic.";
	 else msg = "$N doesn't know where they stand on the laws.";
	 act(msg, ch, NULL, victim, TO_CHAR);
	}
}

void spell_lightning_bolt(const char *sn, int level,CHAR_DATA *ch,void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	dam = dice(level,4) + 12;
	if (saves_spell(level, victim,DAM_LIGHTNING))
		dam /= 2;
	damage(ch, victim, dam, sn, DAM_LIGHTNING, DAMF_SHOW);
}

void spell_locate_object(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	BUFFER *buffer = NULL;
	OBJ_DATA *obj;
	OBJ_DATA *in_obj;
	int number = 0, max_found;

	number = 0;
	max_found = IS_IMMORTAL(ch) ? 200 : 2 * level;

	for (obj = object_list; obj != NULL; obj = obj->next) {
		if (!can_see_obj(ch, obj)
		||  !IS_OBJ_NAME(obj, target_name)
		||  OBJ_IS(obj, OBJ_NOLOCATE)
		||  (OBJ_IS(obj, OBJ_CHQUEST) &&
		     chquest_carried_by(obj) == NULL)
		||  number_percent() > 2 * level
		||  LEVEL(ch) < obj->level)
			continue;

		if (buffer == NULL)
			buffer = buf_new(-1);
		number++;

		for (in_obj = obj; in_obj->in_obj != NULL;
						in_obj = in_obj->in_obj)
			;

		if (in_obj->carried_by != NULL
		&&  can_see(ch, in_obj->carried_by)) {
			buf_printf(buffer, BUF_END, "One is carried by %s\n",
				   PERS(in_obj->carried_by, ch));
		} else {
			if (IS_IMMORTAL(ch) && in_obj->in_room != NULL) {
				buf_printf(buffer, BUF_END, "One is in %s [Room %d]\n",
					   mlstr_cval(&in_obj->in_room->name, ch),
					   in_obj->in_room->vnum);
			} else {
				buf_printf(buffer, BUF_END, "One is in %s\n",
					   in_obj->in_room == NULL ?
					   "somewhere" :
					   mlstr_cval(&in_obj->in_room->name, ch));
			}
		}

		if (number >= max_found)
			break;
	}

	if (buffer == NULL)
		act_char("Nothing like that in heaven or earth.", ch);
	else {
		page_to_char(buf_string(buffer),ch);
		buf_free(buffer);
	}
}

void spell_magic_missile(const char *sn, int level, CHAR_DATA *ch,void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	static const int dam_each[] =
	{
		 0,
		 3,  3,  4,  4,  5,	 6,  6,  6,  6,  6,
		 7,  7,  7,  7,  7,	 8,  8,  8,  8,  8,
		 9,  9,  9,  9,  9,	10, 10, 10, 10, 10,
		11, 11, 11, 11, 11,	12, 12, 12, 12, 12,
		13, 13, 13, 13, 13,	14, 14, 14, 14, 14
	};

	int dam;

	if (is_affected(victim, "protective shield"))  {
		const char *text = LEVEL(ch) > 4 ? "missiles" : "missile";

		act("Your magic $t fizzle out near your victim.",
		    ch, NULL, victim, TO_CHAR);
		act("Your shield blocks $N's magic $t.",
		    victim, text, ch, TO_CHAR);
		return;
	}

	level = UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
	level = UMAX(0, level);
	if (LEVEL(ch) > 50)
		dam = level / 4;
	else
		dam = number_range(dam_each[level] / 2, dam_each[level] * 2);

	if (saves_spell(level, victim, DAM_ENERGY))
		dam /= 2;
	damage(ch, victim, dam, sn, DAM_ENERGY, DAMF_SHOW);

	if (LEVEL(ch) > 4)  {
		dam = number_range(dam_each[level] / 2, dam_each[level] * 2);
		if (saves_spell(level, victim, DAM_ENERGY))
			dam /= 2;
		damage(ch, victim, dam, sn, DAM_ENERGY, DAMF_SHOW);
	}
	if (LEVEL(ch) > 8)  {
		dam = number_range(dam_each[level] / 2, dam_each[level] * 2);
		if (saves_spell(level, victim,DAM_ENERGY))
			dam /= 2;
		damage(ch, victim, dam, sn, DAM_ENERGY, DAMF_SHOW);
	}
	if (LEVEL(ch) > 12)  {
		dam = number_range(dam_each[level] / 2, dam_each[level] * 2);
		if (saves_spell(level, victim,DAM_ENERGY))
			dam /= 2;
		damage(ch, victim, dam, sn, DAM_ENERGY, DAMF_SHOW);
	}
	if (LEVEL(ch) > 16)  {
		dam = number_range(dam_each[level] / 2, dam_each[level] * 2);
		if (saves_spell(level, victim,DAM_ENERGY))
			dam /= 2;
		damage(ch, victim, dam, sn, DAM_ENERGY, DAMF_SHOW);
	}
}

void spell_mass_healing(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *gch;

	for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room) {
		if ((IS_NPC(ch) && IS_NPC(gch))
		||  (!IS_NPC(ch) && !IS_NPC(gch))) {
			spellfun_call("heal", NULL, level, ch, gch);
			spellfun_call("refresh", NULL, level, ch, gch);
		}
	}
}

static void *
mass_invis_cb(void *vo, va_list ap)
{
	CHAR_DATA *gch = (CHAR_DATA *) vo;

	CHAR_DATA *ch = va_arg(ap, CHAR_DATA *);
	AFFECT_DATA *paf;

	if (!is_same_group(gch, ch) || HAS_INVIS(gch, ID_INVIS))
		return NULL;

	if (spellbane(gch, ch, 100, dice(2, LEVEL(gch)))) {
		if (IS_EXTRACTED(ch)) 
			return gch;
		return NULL;
	}

	act("$n slowly fades out of existence.", gch, NULL, NULL, TO_ROOM);
	act("You slowly fade out of existence.", gch, NULL, NULL, TO_CHAR);
	paf = va_arg(ap, AFFECT_DATA *);
	affect_to_char(gch, paf);
	return NULL;
}

void spell_mass_invis(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	AFFECT_DATA af;
	af.where	= TO_INVIS;
	af.type		= sn;
	af.level	= level/2;
	af.duration	= 24;
	INT(af.location)= APPLY_NONE;
	af.modifier	= 0;
	af.bitvector	= ID_INVIS;
	af.owner	= NULL;
	vo_foreach(ch->in_room, &iter_char_room, mass_invis_cb, ch, &af);
	act_char("Ok.", ch);
}

void spell_pass_door(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (IS_AFFECTED(victim, AFF_PASS_DOOR)) {
		if (victim == ch)
			act("You are already out of phase.",
			    ch, NULL, NULL, TO_CHAR);
		else
			act("$N is already shifted out of phase.",
			    ch, NULL, victim, TO_CHAR);
		return;
	}

	af.where	= TO_AFFECTS;
	af.type		= sn;
	af.level	= level;
	af.duration	= number_fuzzy(level / 4);
	INT(af.location)= APPLY_NONE;
	af.modifier	= 0;
	af.bitvector	= AFF_PASS_DOOR;
	af.owner	= NULL;
	affect_to_char(victim, &af);
	act("$n turns translucent.", victim, NULL, NULL, TO_ROOM);
	act("You turn translucent.", victim, NULL, NULL, TO_CHAR);
}

/* RT plague spell, very nasty */

void spell_plague(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (saves_spell(level,victim,DAM_DISEASE) ||
		(IS_NPC(victim) && IS_SET(victim->pMobIndex->act, ACT_UNDEAD)))
	{
		if (ch->in_room == victim->in_room)
		  act("$N seems to be unaffected.",ch,NULL,victim,TO_CHAR);
		return;
	}

	af.where	= TO_AFFECTS;
	af.type 	= sn;
	af.level	= level * 3/4;
	af.duration	= (10 + level / 10);
	INT(af.location)= APPLY_STR;
	af.modifier	= -1 * UMAX(1,3 + level / 15); 
	af.bitvector	= AFF_PLAGUE;
	af.owner	= NULL;
	affect_join(victim,&af);

	act_char("You scream in agony as plague sores erupt from your skin.", victim);
	act("$n screams in agony as plague sores erupt from $s skin.",
		victim,NULL,NULL,TO_ROOM);
}

void spell_poison(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim;
	AFFECT_DATA af;

	if (mem_is(vo, MT_OBJ)) {
		OBJ_DATA *obj = (OBJ_DATA *) vo;

		if (obj->item_type == ITEM_FOOD || obj->item_type == ITEM_DRINK_CON)
		{
		    if (IS_OBJ_STAT(obj,ITEM_BLESS) || IS_OBJ_STAT(obj,ITEM_BURN_PROOF))
		    {
			act("Your spell fails to corrupt $p.",ch,obj,NULL,TO_CHAR);
			return;
		    }
		    INT(obj->value[3]) = 1;
		    act("$p is infused with poisonous vapors.",ch,obj,NULL,TO_ALL);
		    return;
		}

		if (obj->item_type == ITEM_WEAPON)
		{
		    if (IS_WEAPON_STAT(obj,WEAPON_FLAMING)
		    ||  IS_WEAPON_STAT(obj,WEAPON_FROST)
		    ||  IS_WEAPON_STAT(obj,WEAPON_VAMPIRIC)
		    ||  IS_WEAPON_STAT(obj,WEAPON_SHARP)
		    ||  IS_WEAPON_STAT(obj,WEAPON_VORPAL)
		    ||  IS_WEAPON_STAT(obj,WEAPON_SHOCKING)
		    ||  IS_WEAPON_STAT(obj,WEAPON_HOLY)
		    ||  IS_OBJ_STAT(obj,ITEM_BLESS) || IS_OBJ_STAT(obj,ITEM_BURN_PROOF))
		    {
			act("You can't seem to envenom $p.",ch,obj,NULL,TO_CHAR);
			return;
		    }

		    if (IS_WEAPON_STAT(obj,WEAPON_POISON))
		    {
			act("$p is already envenomed.",ch,obj,NULL,TO_CHAR);
			return;
		    }

		    af.where	 = TO_WEAPON;
		    af.type	 = sn;
		    af.level	 = level / 2;
		    af.duration	 = level/8;
		    INT(af.location) = 0;
		    af.modifier	 = 0;
		    af.bitvector = WEAPON_POISON;
		    af.owner	= NULL;
		    affect_to_obj(obj,&af);

		    act("$p is coated with deadly venom.",ch,obj,NULL,TO_ALL);
		    return;
		}

		act("You can't poison $p.",ch,obj,NULL,TO_CHAR);
		return;
	}

	victim = (CHAR_DATA *) vo;

	if (saves_spell(level, victim,DAM_POISON))
		return;

	af.where     = TO_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = (10 + level / 10);
	INT(af.location) = APPLY_STR;
	af.modifier  = -2;
	af.bitvector = AFF_POISON;
	af.owner	= NULL;
	affect_join(victim, &af);
	act_char("You feel very sick.", victim);
	act("$n looks very ill.",victim,NULL,NULL,TO_ROOM);
}

void spell_protection_evil(const char *sn, int level,CHAR_DATA *ch,void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (IS_AFFECTED(victim, AFF_PROTECT_EVIL) 
	||   IS_AFFECTED(victim, AFF_PROTECT_GOOD))
	{
		if (victim == ch)
		  act_char("You are already protected.", ch);
		else
		  act("$N is already protected.",ch,NULL,victim,TO_CHAR);
		return;
	}

	af.where     = TO_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = (10 + level / 5);
	INT(af.location) = APPLY_SAVING_SPELL;
	af.modifier  = -(1 + level/10);
	af.bitvector = AFF_PROTECT_EVIL;
	af.owner	= NULL;
	affect_to_char(victim, &af);
	act_char("You feel holy and pure.", victim);
	if (ch != victim)
		act("$N is protected from evil.",ch,NULL,victim,TO_CHAR);
}

void spell_protection_good(const char *sn, int level,CHAR_DATA *ch,void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (IS_AFFECTED(victim, AFF_PROTECT_GOOD)
	||   IS_AFFECTED(victim, AFF_PROTECT_EVIL))
	{
		if (victim == ch)
		  act_char("You are already protected.", ch);
		else
		  act("$N is already protected.",ch,NULL,victim,TO_CHAR);
		return;
	}

	af.where     = TO_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = (10 + level / 5);
	INT(af.location) = APPLY_SAVING_SPELL;
	af.modifier  = -(1+level/10);
	af.bitvector = AFF_PROTECT_GOOD;
	af.owner	= NULL;
	affect_to_char(victim, &af);
	act_char("You feel aligned with darkness.", victim);
	if (ch != victim)
		act("$N is protected from good.",ch,NULL,victim,TO_CHAR);
}

void spell_ray_of_truth (const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam, align;

	if (IS_EVIL(ch))
	{
		victim = ch;
		act_char("The energy explodes inside you!", ch);
	}

	if (victim != ch)
	{
		act("$n raises $s hand, and a blinding ray of light shoots forth!",
		    ch,NULL,NULL,TO_ROOM);
		act_char("You raise your hand and a blinding ray of light shoots forth!",
		   ch);
	}

	if (IS_GOOD(victim))
	{
		act("$n seems unharmed by the light.",victim,NULL,victim,TO_ROOM);
		act_char("The light seems powerless to affect you.", victim);
		return;
	}

	dam = dice(level, 10);
	if (saves_spell(level, victim,DAM_HOLY))
		dam /= 2;

	align = victim->alignment;
	align -= 350;

	if (align < -1000)
		align = -1000 + (align + 1000) / 3;

	dam = (dam * align * align) / 1000000;

	spellfun_call("blindness", NULL, 3 * level / 4, ch, victim);
	damage(ch, victim, dam, sn, DAM_HOLY, DAMF_SHOW);
}

void spell_recharge(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	int chance, percent;

	if (obj->item_type != ITEM_WAND
	&&  obj->item_type != ITEM_STAFF) {
		act_char("That item does not carry charges.", ch);
		return;
	}

	if (INT(obj->value[3]) >= 3 * level / 2) {
		act_char("Your skills are not great enough for that.", ch);
		return;
	}

	if (INT(obj->value[1]) == 0) {
		act_char("That item has already been recharged once.", ch);
		return;
	}

	chance = 40 + 2 * level;

	chance -= INT(obj->value[3]); /* harder to do high-level spells */
	chance -= (INT(obj->value[1]) - INT(obj->value[2])) *
		  (INT(obj->value[1]) - INT(obj->value[2]));

	chance = UMAX(level/2,chance);

	percent = number_percent();

	if (percent < chance / 2) {
		act("$p glows softly.", ch, obj, NULL, TO_ALL);
		INT(obj->value[2]) = UMAX(INT(obj->value[1]),
					      INT(obj->value[2]));
		INT(obj->value[1]) = 0;
		return;
	} else if (percent <= chance) {
		int chargeback,chargemax;

		act("$p glows softly.", ch, obj, NULL, TO_ALL);

		chargemax = INT(obj->value[1]) - INT(obj->value[2]);

		if (chargemax > 0)
		    chargeback = UMAX(1, chargemax * percent / 100);
		else
		    chargeback = 0;

		INT(obj->value[2]) += chargeback;
		INT(obj->value[1]) = 0;
		return;
	} else if (percent <= UMIN(95, 3 * chance / 2)) {
		act_char("Nothing seems to happen.", ch);
		if (INT(obj->value[1]) > 1)
			INT(obj->value[1])--;
		return;
	} else { /* whoops! */
		act("$p glows brightly and explodes!", ch, obj, NULL, TO_ALL);
		extract_obj(obj, 0);
	}
}

void spell_refresh(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	victim->move = UMIN(victim->move + level, victim->max_move);
	if (victim->max_move == victim->move)
		act_char("You feel fully refreshed!", victim);
	else
		act_char("You feel less tired.", victim);
	if (ch != victim)
		act_char("Ok.", ch);
}

void spell_remove_curse(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim;
	bool found = FALSE;
	OBJ_DATA *obj;

	/* do object cases first */
	if (mem_is(vo, MT_OBJ)) {
		obj = (OBJ_DATA *) vo;

		if (IS_OBJ_STAT(obj,ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_NOREMOVE))
		{
		    if (!OBJ_IS(obj,OBJ_NOUNCURSE)
		    &&  !saves_dispel(level + 2,obj->level,0))
		    {
			REMOVE_OBJ_STAT(obj,ITEM_NODROP | ITEM_NOREMOVE);
			act("$p glows blue.",ch,obj,NULL,TO_ALL);
			return;
		    }

		    act("The curse on $p is beyond your power.",ch,obj,NULL,TO_CHAR);
		    return;
		}
		else  {
		  act_char("Nothing happens...", ch);
		  return;
		}
	}

	/* characters */
	victim = (CHAR_DATA *) vo;

	if (check_dispel(level, victim, "curse"))
	{
		act_char("You feel better.", victim);
		act("$n looks more relaxed.",victim,NULL,NULL,TO_ROOM);
	}

   for (obj = victim->carrying; (obj != NULL && !found); obj = obj->next_content)
   {
		if ((IS_OBJ_STAT(obj,ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_NOREMOVE))
		&&  !OBJ_IS(obj,OBJ_NOUNCURSE))
		{   /* attempt to remove curse */
		    if (!saves_dispel(level,obj->level,0))
		    {
			found = TRUE;
			REMOVE_OBJ_STAT(obj, ITEM_NODROP | ITEM_NOREMOVE);
			act("Your $p glows blue.",victim,obj,NULL,TO_CHAR);
			act("$n's $p glows blue.",victim,obj,NULL,TO_ROOM);
		    }
		 }
	}
}

void spell_sanctuary(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (IS_AFFECTED(victim, AFF_SANCTUARY)) {
		if (victim == ch) {
			act_puts("You are already in sanctuary.",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		} else {
			act("$N is already in sanctuary.",
			    ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	if (IS_AFFECTED(victim, AFF_BLACK_SHROUD)) {
		if (victim == ch) {
	 		act_puts("But you are surrounded by black shroud.",
				  ch, NULL, NULL, TO_CHAR, POS_DEAD);
		} else {
			act("But $N is surrounded by black shroud.",
			    ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	if (IS_EVIL(ch)) {
		act_puts("The gods are infuriated!",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		damage(ch, ch, dice(level, IS_EVIL(ch) ? 2 : 1),
		       NULL, DAM_HOLY, DAMF_SHOW);
		return;
	}

	if (IS_EVIL(victim)) {
		act("Your god does not seems to like $N",
			ch, NULL, victim, TO_CHAR);
		return;
	}

	af.where     = TO_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = level / 6;
	INT(af.location) = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_SANCTUARY;
	af.owner     = NULL;
	affect_to_char(victim, &af);
	act("$n is surrounded by a white aura.", victim, NULL, NULL, TO_ROOM);
	act_puts("You are surrounded by a white aura.",
		 victim, NULL, NULL, TO_CHAR, POS_DEAD);
}

void spell_black_shroud(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA*) vo;
	AFFECT_DATA af;

	if (IS_AFFECTED(victim, AFF_BLACK_SHROUD)) {
		if (victim == ch) {
			act_puts("You are already protected.",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		} else {
			act("$N is already protected.",
			    ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	if (IS_AFFECTED(victim, AFF_SANCTUARY)) {
		if (victim == ch) {
			act_puts("But you are in sanctuary.",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		} else {
			act("But $N is in sanctuary.",
			    ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	if (!IS_EVIL(ch)) {
		act_puts("The gods are infuriated!",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		damage(ch, ch, dice(level, IS_GOOD(ch) ? 2 : 1),
		       NULL, DAM_HOLY, DAMF_SHOW);
		return;
	}
	
	if (!IS_EVIL(victim)) {
		act("Your god does not seems to like $N", 
		    ch, NULL, victim, TO_CHAR);
		return;
	}

	af.where     = TO_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = level/6;
	INT(af.location) = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_BLACK_SHROUD;
	af.owner	= NULL;
	affect_to_char(victim, &af);
	act("$n is surrounded by black aura.", victim, NULL, NULL, TO_ROOM);
	act_puts("You are surrounded by black aura.",
		 victim, NULL, NULL, TO_CHAR, POS_DEAD);
}

void spell_shield(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (is_affected(victim, sn))
	{
		if (victim == ch)
		  act_char("You are already shielded from harm.", ch);
		else
		  act("$N is already protected by a shield.",ch,NULL,victim,TO_CHAR);
		return;
	}

	af.where     = TO_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = (8 + level / 3);
	INT(af.location) = APPLY_AC;
	af.modifier  = -1 * UMAX(20,10 + level / 3); /* af.modifier  = -20;*/
	af.bitvector = 0;
	af.owner	= NULL;
	affect_to_char(victim, &af);
	act("$n is surrounded by a force shield.", victim, NULL, NULL, TO_ROOM);
	act_char("You are surrounded by a force shield.", victim);
}

void spell_shocking_grasp(const char *sn, int level,CHAR_DATA *ch,void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	static const int dam_each[] =
	{
		 6,
		 8,  10,  12,  14,  16,	 18, 20, 25, 29, 33,
		36, 39, 39, 39, 40,	40, 41, 41, 42, 42,
		43, 43, 44, 44, 45,	45, 46, 46, 47, 47,
		48, 48, 49, 49, 50,	50, 51, 51, 52, 52,
		53, 53, 54, 54, 55,	55, 56, 56, 57, 57
	};

	int dam;

	level	= UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
	level	= UMAX(0, level);
		if (LEVEL(ch) > 50)
	dam 	= level / 2 ;
		else
	dam		= number_range(dam_each[level] / 2, dam_each[level] * 2);
	if (saves_spell(level, victim,DAM_LIGHTNING))
		dam /= 2;
	damage(ch, victim, dam, sn, DAM_LIGHTNING, DAMF_SHOW);
}

void spell_sleep(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;


	if (is_affected(victim, "free action"))
		level -= 5;
	if (IS_AFFECTED(victim, AFF_SLEEP)
	||  (IS_NPC(victim) && IS_SET(victim->pMobIndex->act, ACT_UNDEAD))
	||  saves_spell(level, victim, DAM_CHARM))
		return;

	af.where     = TO_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = 1 + level/10;
	INT(af.location) = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_SLEEP;
	af.owner	= NULL;
	affect_join(victim, &af);

	if (IS_AWAKE(victim))
	{
		act_char("You feel very sleepy ..... zzzzzz.", victim);
		act("$n goes to sleep.", victim, NULL, NULL, TO_ROOM);
		victim->position = POS_SLEEPING;
	}
}

void spell_slow(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (is_affected(victim, sn) || IS_AFFECTED(victim,AFF_SLOW)) {
		if (victim == ch)
			act_char("You can't move any slower!", ch);
		else {
			act("$N can't get any slower than that.",
			    ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	if (saves_spell(level,victim, DAM_OTHER) && !check_trust(ch, victim))
		return;

	if (IS_AFFECTED(victim, AFF_HASTE)) {
		if (!check_dispel(level, victim, "haste"))
			return;

		act("$n is moving less quickly.", victim, NULL, NULL, TO_ROOM);
		return;
	}

	af.where     = TO_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = (4 + level / 12);
	INT(af.location) = APPLY_DEX;
	af.modifier  = - UMAX(2, level / 12);
	af.bitvector = AFF_SLOW;
	af.owner	= NULL;
	affect_to_char(victim, &af);
	act_char("You feel yourself slowing d o w n...", victim);
	act("$n starts to move in slow motion.", victim, NULL, NULL, TO_ROOM);
}

void spell_stone_skin(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (is_affected(ch, sn))
	{
		if (victim == ch)
		  act_char("Your skin is already as hard as a rock.", ch);
		else
		  act("$N is already as hard as can be.",ch,NULL,victim,TO_CHAR);
		return;
	}

	af.where     = TO_RESIST;
	af.type      = sn;
	af.level     = level;
	af.duration  = (10 + level / 5);
	af.bitvector = 0;
	INT(af.location) = DAM_BASH;
	af.modifier  = level / 7;
	af.owner	= NULL;
	affect_to_char(victim, &af);

	INT(af.location) = DAM_SLASH;
	affect_to_char(victim, &af);

	INT(af.location) = DAM_PIERCE;
	affect_to_char(victim, &af);

	act("$n's skin turns to stone.", victim, NULL, NULL, TO_ROOM);
	act_char("Your skin turns to stone.", victim);
}

void spell_summon(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	bool failed = FALSE;
	CHAR_DATA *victim;

	if ((victim = get_char_world(ch, target_name)) == NULL
	||  victim->in_room == NULL) {
		act_char("You failed.", ch);
		return;
	}

	if (victim == ch
	||  LEVEL(victim) >= level + 3
	||  victim->fighting != NULL
	||  IS_SET(PC(ch)->plr_flags, PLR_GHOST)
	||  !can_see_room(ch, victim->in_room)
	||  IS_SET(ch->in_room->room_flags, ROOM_SAFE | ROOM_NORECALL |
					    ROOM_PEACE | ROOM_NOSUMMON)
	||  IS_SET(victim->in_room->room_flags, ROOM_SAFE | ROOM_NORECALL |
						ROOM_PEACE | ROOM_NOSUMMON)
	||  IS_SET(ch->in_room->area->area_flags, AREA_CLOSED)
	||  room_is_private(ch->in_room)
	||  saves_spell(level, victim, DAM_OTHER)
	||  (victim->in_room->exit[0] == NULL &&
	     victim->in_room->exit[1] == NULL &&
	     victim->in_room->exit[2] == NULL &&
	     victim->in_room->exit[3] == NULL &&
	     victim->in_room->exit[4] == NULL &&
	     victim->in_room->exit[5] == NULL))
		failed = TRUE;
	else if (IS_NPC(victim)) {
		if (victim->pMobIndex->pShop != NULL
		||  IS_SET(victim->pMobIndex->act, ACT_AGGRESSIVE)
		||  IS_SET(ch->in_room->room_flags, ROOM_NOMOB)
		||  IS_SET(victim->pMobIndex->act, ACT_IMMSUMMON)
		||  NPC(victim)->hunter)
			failed = TRUE;
	} else {
		if (victim->level >= LEVEL_HERO
		||  ((!in_PK(ch, victim) ||
		      ch->in_room->area != victim->in_room->area) &&
		     IS_SET(PC(victim)->plr_flags, PLR_NOSUMMON))
		||  !guild_ok(victim, ch->in_room))
			failed = TRUE;
	}

	if (failed) {
		act_char("You failed.", ch);
		return;
	}

	if (IS_NPC(victim) && NPC(victim)->in_mind == NULL) {
		char buf[MAX_INPUT_LENGTH];
		snprintf(buf, sizeof(buf), "%d", victim->in_room->vnum);
		NPC(victim)->in_mind = str_dup(buf);
	}

	transfer_char(victim, ch, ch->in_room,
		      "$N disappears suddenly.",
		      "$n has summoned you!",
		      "$N arrives suddenly.");
}

void spell_teleport(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (victim->in_room == NULL
	||  IS_SET(victim->in_room->room_flags, ROOM_NORECALL)
	||  (!IS_NPC(ch) && victim->fighting != NULL)
	||  (victim != ch
	&&  (saves_spell(level - 5, victim,DAM_OTHER)))) {
		act_char("You failed.", ch);
		return;
	}

	transfer_char(victim, ch, get_random_room(victim, NULL),
		      "$N vanishes!",
		      "You have been teleported!", 
		      "$N slowly fades into existence.");
}

void spell_bamf(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (victim->in_room == NULL
	||  saves_spell(level, victim, DAM_OTHER)
	||  IS_SET(victim->in_room->room_flags, ROOM_PEACE | ROOM_SAFE)) {
		act_char("You failed.", ch);
		return;
	}

	transfer_char(victim, ch,
		      get_random_room(victim, victim->in_room->area),
		      "$N vanishes!",
		      "You have been teleported.",
		      "$N slowly fades into existence.");
}

void spell_ventriloquate(const char *sn, int level, CHAR_DATA *ch,void *vo)
{
	char speaker[MAX_INPUT_LENGTH];
	CHAR_DATA *vch;

	target_name = one_argument(target_name, speaker, sizeof(speaker));

	for (vch = ch->in_room->people; vch; vch = vch->next_in_room) {
		if (is_name(speaker, vch->name))
			continue;

		if (saves_spell(level, vch, DAM_OTHER)) {
			act("Someone makes $t say '{G$T{x'",
			    vch, speaker, target_name, TO_CHAR);
		} else {
			act("$t says '{G$T{x'",
			    vch, speaker, target_name, TO_CHAR);
		}
	}
}

void spell_weaken(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (is_affected(victim, sn) || saves_spell(level, victim,DAM_OTHER))
		return;

	af.where     = TO_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = (4 + level / 12);
	INT(af.location) = APPLY_STR;
	af.modifier  = -1 * (2 + level / 12);
	af.bitvector = AFF_WEAKEN;
	af.owner	= NULL;
	affect_to_char(victim, &af);
	act_char("You feel your strength slip away.", victim);
	act("$n looks tired and weak.",victim,NULL,NULL,TO_ROOM);
}

void spell_word_of_recall(const char *sn, int level, CHAR_DATA *ch,void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	ROOM_INDEX_DATA *location;
	CHAR_DATA *pet;

	if (IS_NPC(victim))
		return;

	if (victim->fighting
	&&  !can_flee(victim)) {
		if (victim == ch)
			act_char("Your honour doesn't let you recall!.", ch);
		else {
			act_puts("You can't cast this spell to a honourable fighting $t!",
				 ch, victim->class, NULL,
				 TO_CHAR | ACT_NOTRANS, POS_DEAD);
		}
		return;
	}

	if (victim->desc && IS_PUMPED(victim)) {
		act_puts("You are too pumped to pray now.",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		return;
	}

	act("$n prays for transportation!", ch, NULL, NULL, TO_ROOM);

	if (IS_SET(victim->in_room->room_flags, ROOM_NORECALL)
	||  IS_AFFECTED(victim, AFF_CURSE)
	||  IS_AFFECTED(victim->in_room, RAFF_CURSE)) {
		act_char("Spell failed.", victim);
		return;
	}

	if (victim->fighting) {
		if (victim == ch && !IS_NPC(ch))
			gain_exp(victim, 0 - (victim->level + 25));
		stop_fighting(victim, TRUE);
	}

	ch->move /= 2;
	pet = GET_PET(victim);
	location = get_recall(victim);
	recall(victim, location);

	if (pet && !IS_AFFECTED(pet, AFF_SLEEP)) {
		if (pet->position != POS_STANDING)
			dofun("stand", pet, str_empty);
		recall(pet, location);
	}
}

/*
 * Draconian spells.
 */
void spell_acid_breath(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam,hp_dam,dice_dam,hpch;

	act("$n spits acid at $N.",ch,NULL,victim,TO_NOTVICT);
	act("$n spits a stream of corrosive acid at you.",ch,NULL,victim,TO_VICT);
	act("You spit acid at $N.",ch,NULL,victim,TO_CHAR);

	hpch = UMAX(12,ch->hit);
	hp_dam = number_range(hpch/11 + 1, hpch/6);
	dice_dam = dice(level,16);

	dam = UMAX(hp_dam + dice_dam/10,dice_dam + hp_dam/10);
	
	if (saves_spell(level, victim, DAM_ACID)) {
		acid_effect(victim, level/2, dam/4);
		damage(ch,victim, dam/2, sn, DAM_ACID, DAMF_SHOW);
	} else {
		acid_effect(victim, level, dam);
		damage(ch, victim, dam, sn, DAM_ACID, DAMF_SHOW);
	}
}

static void *
fire_breath_cb(void *vo, va_list ap)
{
	CHAR_DATA *vch = (CHAR_DATA *) vo;

	const char *sn = va_arg(ap, const char *);
	int level = va_arg(ap, int);
	CHAR_DATA *ch = va_arg(ap, CHAR_DATA *);
	CHAR_DATA *victim = va_arg(ap, CHAR_DATA *);
	int dam = va_arg(ap, int);

	if (is_safe_spell(ch, vch, TRUE)
	||  (IS_NPC(vch) && IS_NPC(ch) &&
	     (ch->fighting != vch /*|| vch->fighting != ch */)))
		return NULL;

	if (vch == victim) { /* full damage */
		if (saves_spell(level, vch, DAM_FIRE)) {
			fire_effect(vch, level/2, dam/4);
			damage(ch, vch, dam/2, sn, DAM_FIRE, DAMF_SHOW);
		} else {
			fire_effect(vch, level, dam);
			damage(ch, vch, dam, sn, DAM_FIRE, DAMF_SHOW);
		}
	} else { /* partial damage */
		if (saves_spell(level - 2, vch, DAM_FIRE)) {
			fire_effect(vch, level/4, dam/8);
			damage(ch, vch, dam/4, sn, DAM_FIRE, DAMF_SHOW);
		} else {
			fire_effect(vch, level/2, dam/4);
			damage(ch, vch, dam/2, sn, DAM_FIRE, DAMF_SHOW);
		}
	}

	return NULL;
}

void spell_fire_breath(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int hpch = UMAX(10, ch->hit);
	int hp_dam  = number_range(hpch/9 + 1, hpch/5);
	int dice_dam = dice(level, 20);
	int dam = UMAX(hp_dam + dice_dam /10, dice_dam + hp_dam / 10);

	act("$n breathes forth a cone of fire.", ch, NULL, victim, TO_NOTVICT);
	act("$n breathes a cone of hot fire over you!",
	    ch, NULL, victim, TO_VICT);
	act("You breath forth a cone of fire.", ch, NULL, NULL, TO_CHAR);

	fire_effect(victim->in_room, level, dam / 2);
	vo_foreach(victim->in_room, &iter_char_room, fire_breath_cb,
		   sn, level, ch, victim, dam);
}

static void *
frost_breath_cb(void *vo, va_list ap)
{
	CHAR_DATA *vch = (CHAR_DATA *) vo;

	const char *sn = va_arg(ap, const char *);
	int level = va_arg(ap, int);
	CHAR_DATA *ch = va_arg(ap, CHAR_DATA *);
	CHAR_DATA *victim = va_arg(ap, CHAR_DATA *);
	int dam = va_arg(ap, int);

	if (is_safe_spell(ch,vch,TRUE)
	||  (IS_NPC(vch) && IS_NPC(ch) &&
	     (ch->fighting != vch /*|| vch->fighting != ch*/)))
		return NULL;

	if (vch == victim) { /* full damage */
		if (saves_spell(level, vch, DAM_COLD)) {
			cold_effect(vch, level/2, dam/4);
			damage(ch, vch, dam/2, sn, DAM_COLD, DAMF_SHOW);
		} else {
			cold_effect(vch, level, dam);
			damage(ch, vch, dam, sn, DAM_COLD, DAMF_SHOW);
		}
	} else {
		if (saves_spell(level - 2, vch, DAM_COLD)) {
			cold_effect(vch, level/4, dam/8);
			damage(ch, vch, dam/4, sn, DAM_COLD, DAMF_SHOW);
		} else {
			cold_effect(vch, level/2, dam/4);
			damage(ch, vch, dam/2, sn, DAM_COLD, DAMF_SHOW);
		}
	}

	return NULL;
}

void spell_frost_breath(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int hpch = UMAX(12, ch->hit);
	int hp_dam = number_range(hpch/11 + 1, hpch/6);
	int dice_dam = dice(level, 16);
	int dam = UMAX(hp_dam + dice_dam/10, dice_dam + hp_dam/10);

	act("$n breathes out a freezing cone of frost!",
	    ch, NULL, victim, TO_NOTVICT);
	act("$n breathes a freezing cone of frost over you!",
	    ch, NULL, victim, TO_VICT);
	act("You breath out a cone of frost.",
	    ch, NULL, NULL, TO_CHAR);

	cold_effect(victim->in_room, level, dam/2); 
	vo_foreach(victim->in_room, &iter_char_room, frost_breath_cb,
		   sn, level, ch, victim, dam);
}

static void *
gas_breath_cb(void *vo, va_list ap)
{
	CHAR_DATA *vch = (CHAR_DATA *) vo;

	const char *sn = va_arg(ap, const char *);
	int level = va_arg(ap, int);
	CHAR_DATA *ch = va_arg(ap, CHAR_DATA *);
	int dam = va_arg(ap, int);

	if (is_safe_spell(ch,vch,TRUE)
	||  (IS_NPC(ch) && IS_NPC(vch) &&
	     (ch->fighting == vch || vch->fighting == ch)))
		return NULL;

	if (saves_spell(level, vch, DAM_POISON)) {
		poison_effect(vch, level/2, dam/4);
		damage(ch, vch, dam/2, sn, DAM_POISON, DAMF_SHOW);
	} else {
		poison_effect(vch, level, dam);
		damage(ch, vch, dam, sn, DAM_POISON, DAMF_SHOW);
	}

	return NULL;
}

void spell_gas_breath(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	int hpch = UMAX(16, ch->hit);
	int hp_dam = number_range(hpch/15 + 1, 8);
	int dice_dam = dice(level, 12);
	int dam = UMAX(hp_dam + dice_dam/10, dice_dam + hp_dam/10);

	act("$n breathes out a cloud of poisonous gas!",
	    ch, NULL, NULL, TO_ROOM);
	act("You breath out a cloud of poisonous gas.",
	    ch, NULL, NULL, TO_CHAR);
	poison_effect(ch->in_room, level, dam);
	vo_foreach(ch->in_room, &iter_char_room, gas_breath_cb,
		   sn, level, ch, dam);
}

void spell_lightning_breath(const char *sn, int level,CHAR_DATA *ch,void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int hpch = UMAX(10, ch->hit);
	int hp_dam = number_range(hpch/9 + 1, hpch/5);
	int dice_dam = dice(level, 20);
	int dam = UMAX(hp_dam + dice_dam/10, dice_dam + hp_dam/10);

	act("$n breathes a bolt of lightning at $N.",
	    ch, NULL, victim, TO_NOTVICT);
	act("$n breathes a bolt of lightning at you!",
	    ch, NULL, victim, TO_VICT);
	act("You breathe a bolt of lightning at $N.",
	    ch, NULL, victim, TO_CHAR);

	if (saves_spell(level, victim, DAM_LIGHTNING)) {
		shock_effect(victim, level/2, dam/4);
		damage(ch, victim, dam/2, sn, DAM_LIGHTNING, DAMF_SHOW);
	} else {
		shock_effect(victim, level, dam);
		damage(ch, victim, dam, sn, DAM_LIGHTNING, DAMF_SHOW); 
	}
}

/*
 * Spells for mega1.are from Glop/Erkenbrand.
 */
void spell_general_purpose(const char *sn, int level,CHAR_DATA *ch,void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;
 
	dam = number_range(25, 100);
	if (saves_spell(level, victim, DAM_PIERCE))
		dam /= 2;
	damage(ch, victim, dam, sn, DAM_PIERCE, DAMF_SHOW);
}

void spell_high_explosive(const char *sn, int level,CHAR_DATA *ch,void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;
 
	dam = number_range(30, 120);
	if (saves_spell(level, victim, DAM_PIERCE))
		dam /= 2;
	damage(ch, victim, dam, sn, DAM_PIERCE, DAMF_SHOW);
}

void spell_find_object(const char *sn, int level, CHAR_DATA *ch, void *vo) 
{
	BUFFER *buffer = NULL;
	OBJ_DATA *obj;
	OBJ_DATA *in_obj;
	int number = 0, max_found;

	number = 0;
	max_found = IS_IMMORTAL(ch) ? 200 : 2 * level;

	for (obj = object_list; obj != NULL; obj = obj->next) {
		if (!can_see_obj(ch, obj)
		||  !IS_OBJ_NAME(obj, target_name)
		||  OBJ_IS(obj, OBJ_NOFIND)
		||  number_percent() > 2 * level
		||  LEVEL(ch) < obj->level
		||  (OBJ_IS(obj, OBJ_CHQUEST) &&
		     chquest_carried_by(obj) == NULL))
			continue;

		if (buffer == NULL)
			buffer = buf_new(-1);
		number++;

		for (in_obj = obj; in_obj->in_obj != NULL;
						in_obj = in_obj->in_obj)
			;

		if (in_obj->carried_by != NULL
		&&  can_see(ch,in_obj->carried_by)) {
			buf_printf(buffer, BUF_END, "One is carried by %s\n",
				   PERS(in_obj->carried_by, ch));
		} else {
			if (IS_IMMORTAL(ch) && in_obj->in_room != NULL)
				buf_printf(buffer, BUF_END, "One is in %s [Room %d]\n",
					mlstr_cval(&in_obj->in_room->name, ch),
					in_obj->in_room->vnum);
			else
				buf_printf(buffer, BUF_END, "One is in %s\n",
					in_obj->in_room == NULL ?
					"somewhere" :
					mlstr_cval(&in_obj->in_room->name, ch));
		}

		if (number >= max_found)
			break;
	}

	if (buffer == NULL)
		act_char("Nothing like that in heaven or earth.", ch);
	else {
		page_to_char(buf_string(buffer),ch);
		buf_free(buffer);
	}
}

void spell_lightning_shield(const char *sn, int level, CHAR_DATA *ch, void *vo) 
{
	AFFECT_DATA af2;
	AFFECT_DATA af;

	if (is_affected_room(ch->in_room, sn)) {
		act_char("This room has already shielded.", ch);
		return;
	}

	if (is_affected(ch,sn)) {
		act_char("This spell is used too recently.", ch);
		return;
	}
   
	af.where	= TO_ROOM_AFFECTS;
	af.type		= sn;
	af.level	= level;
	af.duration	= level / 40;
	INT(af.location)= APPLY_NONE;
	af.modifier	= 0;
	af.bitvector	= 0;
	af.owner	= ch;
	affect_to_room(ch->in_room, &af);

	af2.where	= TO_AFFECTS;
	af2.type	= sn;
	af2.level	= ch->level;
	af2.duration	= level / 10;
	af2.modifier	= 0;
	INT(af2.location)= APPLY_NONE;
	af2.bitvector	= 0;
	af2.owner	= NULL;
	affect_to_char(ch, &af2);

	act_char("The room starts to be filled with lightnings.", ch);
	act("The room starts to be filled with $n's lightnings.",
	    ch, NULL, NULL, TO_ROOM);
}

void spell_shocking_trap(const char *sn, int level, CHAR_DATA *ch, void *vo) 
{
	AFFECT_DATA af2;
	AFFECT_DATA af;

	if (is_affected_room(ch->in_room, sn)) {
		act_char("This room has already trapped with shocks waves.", ch);
		return;
	}

	if (is_affected(ch, sn)) {
		act_char("This spell is used too recently.", ch);
		return;
	}
   
	af.where	= TO_ROOM_AFFECTS;
	af.type		= sn;
	af.level	= level;
	af.duration	= level / 40;
	INT(af.location)= APPLY_NONE;
	af.modifier	= 0;
	af.bitvector	= 0;
	af.owner	= ch;
	affect_to_room(ch->in_room, &af);

	af2.where	= TO_AFFECTS;
	af2.type	= sn;
	af2.level	= level;
	af2.duration	= ch->level / 10;
	af2.modifier	= 0;
	INT(af2.location)= APPLY_NONE;
	af2.bitvector	= 0;
	af2.owner	= NULL;
	affect_to_char(ch, &af2);
	act_char("The room starts to be filled with shock waves.", ch);
	act("The room starts to be filled with $n's shock waves.",
	    ch, NULL, NULL, TO_ROOM);
}

void spell_acid_arrow(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	dam = dice(level, 8);
	if (saves_spell(level, victim, DAM_ACID))
		dam /= 2;
	damage(ch, victim, dam, sn, DAM_ACID, DAMF_SHOW);
}


/* energy spells */
void spell_etheral_fist(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	dam = dice(level, 8);
	if (saves_spell(level, victim, DAM_ENERGY))
		dam /= 2;
	act("A fist of black, otherworldly ether rams into $N, leaving $M looking stunned!",
	    ch, NULL, victim, TO_NOTVICT);
	damage(ch, victim, dam, sn, DAM_ENERGY, DAMF_SHOW);
}

void spell_spectral_furor(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	dam = dice(level, 6);
	if (saves_spell(level, victim, DAM_LIGHT))
		dam /= 2;
	act("The fabric of the cosmos strains in fury about $N!",
	    ch, NULL, victim, TO_NOTVICT);
	damage(ch, victim, dam, sn, DAM_LIGHT, DAMF_SHOW);
}

void spell_disruption(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	dam = dice(level, 7);
	if (saves_spell(level, victim, DAM_NEGATIVE))
		dam /= 2;
	act("A weird energy encompasses $N, causing you to question $S continued existence.",
	    ch, NULL, victim, TO_NOTVICT);
	damage(ch, victim, dam, sn, DAM_NEGATIVE, DAMF_SHOW);
}

void spell_sonic_resonance(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	dam = dice(level, 6);
	if (saves_spell(level, victim, DAM_SOUND))
		dam /= 2;
	act("A cylinder of kinetic energy enshrouds $N causing $S to resonate.",
	    ch, NULL, victim, TO_NOTVICT);
	damage(ch, victim, dam, sn, DAM_SOUND, DAMF_SHOW);
}

/* mental */
void spell_mind_wrack(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	dam = dice(level, 7);
	if (saves_spell(level, victim, DAM_MENTAL))
		dam /= 2;
	act("$n stares intently at $N, causing $N to seem very lethargic.",
	    ch, NULL, victim, TO_NOTVICT);
	damage(ch, victim, dam, sn, DAM_MENTAL, DAMF_SHOW);
}

void spell_mind_wrench(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	dam = dice(level, 9);
	if (saves_spell(level, victim, DAM_MENTAL))
		dam /= 2;
	act("$n stares intently at $N, causing $N to seem very hyperactive.",
	    ch, NULL, victim, TO_NOTVICT);
	damage(ch, victim, dam, sn, DAM_MENTAL, DAMF_SHOW);
}

/* acid */
void spell_sulfurus_spray(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	dam = dice(level, 7);
	if (saves_spell(level, victim, DAM_ACID))
		dam /= 2;
	act("A stinking spray of sulfurous liquid rains down on $N." ,
	    ch, NULL, victim, TO_NOTVICT);
	damage(ch, victim, dam, sn, DAM_ACID, DAMF_SHOW);
}

void spell_caustic_font(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	dam = dice(level, 9);
	if (saves_spell(level, victim, DAM_ACID))
		dam /= 2;
	act("A fountain of caustic liquid forms below $N. The smell of $S degenerating tissues is revolting! ",
	    ch, NULL, victim, TO_NOTVICT);
	damage(ch, victim, dam, sn, DAM_ACID, DAMF_SHOW);
}

void spell_acetum_primus(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	dam = dice(level, 8);
	if (saves_spell(level, victim, DAM_ACID))
		dam /= 2;
	act("A cloak of primal acid enshrouds $N, sparks form as it consumes all it touches. ",
	    ch, NULL, victim, TO_NOTVICT);
	damage(ch, victim, dam, sn, DAM_ACID, DAMF_SHOW);
}

/*  Electrical  */
void spell_galvanic_whip(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	dam = dice(level, 7);
	if (saves_spell(level, victim, DAM_LIGHTNING))
		dam /= 2;
	act("$n conjures a whip of ionized particles, which lashes ferociously at $N.",
	    ch, NULL, victim, TO_NOTVICT);
	damage(ch, victim, dam, sn, DAM_LIGHTNING, DAMF_SHOW);
}

void spell_magnetic_trust(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	dam = dice(level, 8);
	if (saves_spell(level, victim, DAM_LIGHTNING))
		dam /= 2;
	act("An unseen energy moves nearby, causing your hair to stand on end!",
	    ch, NULL, victim, TO_NOTVICT);
	damage(ch, victim, dam, sn, DAM_LIGHTNING, DAMF_SHOW);
}

void spell_quantum_spike(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	dam = dice(level, 9);
	if (saves_spell(level, victim, DAM_LIGHTNING))
		dam /= 2;
	act("$N seems to dissolve into tiny unconnected particles, then is painfully reassembled.",
	    ch, NULL, victim, TO_NOTVICT);
	damage(ch, victim, dam, sn, DAM_LIGHTNING, DAMF_SHOW);
}

/* negative */
void spell_hand_of_undead(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	if (saves_spell(level, victim, DAM_NEGATIVE)) {
		act_char("You feel a momentary chill.", victim);
		return;
	}

	if (IS_NPC(victim) && IS_SET(victim->pMobIndex->act, ACT_UNDEAD)) {
		 act_char("Your victim is unaffected by hand of undead.", ch);
		 return;
	}

	if (victim->level <= 2)
		dam		 = ch->hit + 1;
	else {
		dam = dice(level, 10);
		victim->mana	/= 2;
		victim->move	/= 2;
		ch->hit		+= dam / 2;
	}

	act_char("You feel your life slipping away!", victim);
	act("$N is grasped by an incomprehensible hand of undead!",
	    ch, NULL, victim, TO_NOTVICT);
	damage(ch, victim, dam, sn, DAM_NEGATIVE, DAMF_SHOW);
}

static inline void
astral_walk(CHAR_DATA *ch, CHAR_DATA *victim)
{
	transfer_char(ch, victim, victim->in_room,
		      "$N disappears in a flash of light!",
		      "You travel via astral planes and go to $n.",
		      "$N appears in a flash of light!");
}

/* travel via astral plains */
void spell_astral_walk(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim;
	CHAR_DATA *pet = NULL;

	if ((victim = get_char_world(ch, target_name)) == NULL
	||  LEVEL(victim) >= level + 3
	||  saves_spell(level, victim, DAM_OTHER)
	||  !can_gate(ch, victim)) {
		act_char("You failed.", ch);
		return;
	}

	pet = GET_PET(ch);
	if (pet && pet->in_room != ch->in_room)
		pet = NULL;

	astral_walk(ch, victim);
	if (pet && !IS_AFFECTED(pet, AFF_SLEEP)) {
		if (pet->position != POS_STANDING)
			dofun("stand", pet, str_empty);
		astral_walk(ch, victim);
	}
}

/* vampire version astral walk */
void spell_mist_walk(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim;

	if ((victim = get_char_world(ch, target_name)) == NULL
	||  LEVEL(victim) >= level - 5
	||  saves_spell(level, victim, DAM_OTHER)
	||  !can_gate(ch, victim)) {
		act_char("You failed.", ch);
		return;
	}

	transfer_char(ch, NULL, victim->in_room,
		      "$N dissolves into a cloud of glowing mist, then vanishes!",
		      "You dissolve into a cloud of glowing mist, then flow to your target.",
		      "A cloud of glowing mist engulfs you, then withdraws to unveil $N!");
}

/*  Cleric version of astra_walk  */
void spell_solar_flight(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim;

	if  (time_info.hour > 18 || time_info.hour < 8) {
		 act_char("You need sunlight for solar flight.", ch);
		 return;
	}

	if ((victim = get_char_world(ch, target_name)) == NULL
	||  LEVEL(victim) >= level + 1
	||  saves_spell(level, victim, DAM_OTHER)
	||  !can_gate(ch, victim)) {
		act_char("You failed.", ch);
		return;
	}

	transfer_char(ch, NULL, victim->in_room,
		      "$N disappears in a blinding flash of light!",
		      "You dissolve in a blinding flash of light!",
		      "$N appears in a blinding flash of light!");
}

/* travel via astral plains */
void spell_helical_flow(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim;


	if ((victim = get_char_world(ch, target_name)) == NULL
	||  LEVEL(victim) >= level + 3
	||  saves_spell(level, victim, DAM_OTHER)
	||  !can_gate(ch, victim)) {
		act_char("You failed.", ch);
		return;
	}

	transfer_char(ch, NULL, victim->in_room,
		      "$N coils into an ascending column of colour, vanishing into thin air.",
		      "You coil into an ascending column of colour, vanishing into thin air.",
		      "A coil of colours descends from above, revealing $N as it dissipates.");
}

void spell_corruption(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (IS_AFFECTED(victim, AFF_CORRUPTION)) {
		act("$N is already corrupting.", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (IS_IMMORTAL(victim)
	||  saves_spell(level, victim, DAM_NEGATIVE)
	||  (IS_NPC(victim) && IS_SET(victim->pMobIndex->act, ACT_UNDEAD))) {
		if (ch == victim)
			act_puts("You feel momentarily ill, but it passes.",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		else
			act_puts("$N seems to be unaffected.",
				 ch, NULL, victim, TO_CHAR, POS_DEAD);
		return;
	}

	af.where	= TO_AFFECTS;
	af.type 	= sn;
	af.level	= level * 3 / 4;
	af.duration	= 10 + level / 5;
	INT(af.location)= APPLY_NONE;
	af.modifier	= 0; 
	af.bitvector	= AFF_CORRUPTION;
	af.owner	= NULL;
	affect_join(victim,&af);

	act("You scream in agony as you start to decay into dust.",
	    victim, NULL, NULL, TO_CHAR);
	act("$n screams in agony as $n start to decay into dust.",
	    victim, NULL, NULL, TO_ROOM);
}

static void *
hurricane_cb(void *vo, va_list ap)
{
	CHAR_DATA *vch = (CHAR_DATA *) vo;

	const char *sn = va_arg(ap, const char *);
	int level = va_arg(ap, int);
	CHAR_DATA *ch = va_arg(ap, CHAR_DATA *);
	int *pdam = va_arg(ap, int *);

	if (is_safe_spell(ch, vch, TRUE)
	||  (IS_NPC(ch) && IS_NPC(vch) &&
	     (ch->fighting == vch || vch->fighting == ch)))
		return NULL;

	if (!IS_AFFECTED(vch, AFF_FLYING))
		*pdam /= 2;

	if (vch->size == SIZE_TINY)
		*pdam = *pdam * 3 / 2;
	else if (vch->size == SIZE_SMALL)
		*pdam = *pdam * 13 / 10;
	else if (vch->size == SIZE_MEDIUM)
		;
	else if (vch->size == SIZE_LARGE)
		*pdam = *pdam * 9 / 10;
	else if (vch->size == SIZE_HUGE)
		*pdam = *pdam * 7 / 10;
	else
		*pdam = *pdam / 2;

	if (saves_spell(level, vch, DAM_OTHER))
		damage(ch, vch, *pdam/2, sn, DAM_OTHER, DAMF_SHOW);
	else
		damage(ch, vch, *pdam, sn, DAM_OTHER, DAMF_SHOW);
	return NULL;
}

void spell_hurricane(const char *sn, int level,CHAR_DATA *ch,void *vo)
{
	int hpch = UMAX(16, ch->hit);
	int hp_dam = number_range(hpch/15 + 1, 8);
	int dice_dam = dice(level, 12);
	int dam = UMAX(hp_dam + dice_dam/10, dice_dam + hp_dam/10);

	act("$n prays the gods of the storm for help.",
	    ch, NULL, NULL, TO_NOTVICT);
	act("You pray the gods of the storm to help you.",
	    ch, NULL, NULL, TO_CHAR);
	vo_foreach(ch->in_room, &iter_char_room, hurricane_cb,
		   sn, level, ch, &dam);
}

void spell_detect_undead(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (HAS_DETECT(victim, ID_UNDEAD)) {
		if (victim == ch)
			act_char("You can already sense undead.", ch);
		else {
			act("$N can already detect undead.",
			    ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	af.where	= TO_DETECTS;
	af.type		= sn;
	af.level	= level;
	af.duration	= (5 + level / 3);
	af.modifier	= 0;
	INT(af.location)= APPLY_NONE;
	af.bitvector	= ID_UNDEAD;
	af.owner	= NULL;
	affect_to_char(victim, &af);
	act_char("Your eyes tingle.", victim);
	if (ch != victim)
		act_char("Ok.", ch);
}

void spell_take_revenge(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	OBJ_DATA *obj;
	OBJ_DATA *in_obj;
	ROOM_INDEX_DATA *room = NULL;
	bool found = FALSE;
 
	if (IS_NPC(ch) || !IS_SET(PC(ch)->plr_flags, PLR_GHOST)) {
		act_char("It is too late to take revenge.", ch);
		return;
	}

	for (obj = object_list; obj; obj = obj->next) {
		if (obj->pObjIndex->vnum != OBJ_VNUM_CORPSE_PC
		||  !IS_OWNER(ch, obj))
			continue;

		found = TRUE;
		for (in_obj = obj; in_obj->in_obj; in_obj = in_obj->in_obj)
			;

		if (in_obj->carried_by != NULL)
			room = in_obj->carried_by->in_room;
		else
			room = in_obj->in_room;
		break;
	}

	if (!found || room == NULL)
		act_char("Unluckily your corpse is devoured.", ch);
	else
		transfer_char(ch, NULL, room, NULL, NULL, NULL);
}

