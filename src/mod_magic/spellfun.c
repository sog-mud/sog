/*
 * $Id: spellfun.c,v 1.193 1999-12-03 11:57:19 fjoe Exp $
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
#include "chquest.h"
#include "fight.h"

void spell_acid_blast(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	dam = dice(level, 18);
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
		  char_puts("You are already armored.\n",ch);
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
	affect_to_char(victim, &af);
	char_puts("You feel someone protecting you.\n", victim);
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
			REMOVE_BIT(obj->extra_flags,ITEM_EVIL);
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
		affect_to_obj(obj,&af);

		act("$p glows with a holy aura.",ch,obj,NULL,TO_ALL);
		return;
	}

	victim = (CHAR_DATA *) vo;

	if ((ch && victim->position == POS_FIGHTING)
	||  is_affected(victim, sn)) {
		if (victim == ch)
			char_puts("You are already blessed.\n", ch);
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
	INT(af.location)= APPLY_HITROLL;
	af.modifier	= level / 8;
	af.bitvector	= 0;
	affect_to_char(victim, &af);

	INT(af.location)= APPLY_SAVING_SPELL;
	af.modifier	= 0 - level / 8;
	affect_to_char(victim, &af);

	char_puts("You feel righteous.\n", victim);
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
	  char_puts("You failed.\n", ch);
		return;
	}

	af.where     = TO_AFFECTS;
	af.type      = sn;
	af.level     = level;
	INT(af.location) = APPLY_HITROLL;
	af.modifier  = -4;
	af.duration  = 3+level / 15;
	af.bitvector = AFF_BLIND;
	affect_to_char(victim, &af);
	char_puts("You are blinded!\n", victim);
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

void spell_call_lightning(const char *sn, int level,CHAR_DATA *ch,void *vo)
{
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;
	int dam;

	if (!IS_OUTSIDE(ch)) {
		char_puts("You must be out of doors.\n", ch);
		return;
	}

	if (weather_info.sky < SKY_RAINING) {
		char_puts("You need bad weather.\n", ch);
		return;
	}

	dam = dice(level, 9);

	char_puts("Gods' lightning strikes your foes!\n", ch);
	act("$n calls lightning to strike $s foes!", ch, NULL, NULL, TO_ROOM);

	for (vch = char_list; vch; vch = vch_next) {
		vch_next	= vch->next;

		if (vch->in_room == NULL)
			continue;

		if (vch->in_room == ch->in_room) {
			if (is_safe_spell(ch, vch, TRUE))
				continue;
			damage(ch, vch,
			       saves_spell(level, vch, DAM_LIGHTNING) ?
			       dam / 2 : dam,
			       sn, DAM_LIGHTNING, DAMF_SHOW);
			continue;
		}

		if (vch->in_room->area == ch->in_room->area
		&&  IS_OUTSIDE(vch)
		&&  IS_AWAKE(vch))
		    char_puts("Lightning flashes in the sky.\n", vch);
	}
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
	for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
	{
		if (vch->position == POS_FIGHTING)
		{
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
			&&  (IS_SET(vch->imm_flags, IMM_MAGIC) ||
			     IS_SET(vch->pMobIndex->act, ACT_UNDEAD)))
				return;

			if (IS_AFFECTED(vch, AFF_CALM | AFF_BERSERK)
			||  is_affected(vch, "frenzy"))
				return;

			char_puts("A wave of calm passes over you.\n", vch);
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
	{ "change sex",		"$n looks more like $mself again."	},
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
	{ "magic resistance",	NULL					},
	{ "hallucination",	NULL					},
	{ "terrangreal",	NULL					},
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
		char_puts("You feel a brief tingling sensation.\n", victim);
		char_puts("You failed.\n", ch);
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
		char_puts("Ok.\n", ch);
	else
		char_puts("Spell failed.\n",ch);
}

void spell_cancellation(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	/*
	 * unlike dispel magic, the victim gets NO save
	 */
	if (dispel((CHAR_DATA *)vo, level + 2))
		char_puts("Ok.\n", ch);
	else
		char_puts("Spell failed.\n", ch);
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

void spell_chain_lightning(const char *sn, int level,CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	CHAR_DATA *vch,*last_vict,*next_vict;
	bool found;
	int dam;

	/* first strike */

	act("A lightning bolt leaps from $n's hand and arcs to $N.",
		ch,NULL,victim,TO_ROOM);
	act("A lightning bolt leaps from your hand and arcs to $N.",
		ch,NULL,victim,TO_CHAR);
	act("A lightning bolt leaps from $n's hand and hits you!",
		ch,NULL,victim,TO_VICT);

	dam = dice(level, 6);
	if (saves_spell(level, victim, DAM_LIGHTNING))
		dam /= 3;
	damage(ch, victim, dam, sn, DAM_LIGHTNING, DAMF_SHOW);

	last_vict = victim;
	level -= 4;   /* decrement damage */

	while (level > 0) {
		found = FALSE;
		for (vch = ch->in_room->people; vch; vch = next_vict) {
			next_vict = vch->next_in_room;

			if (vch == last_vict)
				continue;

			if (is_safe_spell(ch, vch, TRUE)) {
				act("The bolt passes around $n's body.",
				    ch, NULL, NULL, TO_ROOM);
				act("The bolt passes around your body.",
				    ch, NULL, NULL, TO_CHAR);
				continue;
			}

			found = TRUE;
			last_vict = vch;
			act("The bolt arcs to $n!", vch, NULL, NULL, TO_ROOM);
			act("The bolt hits you!", vch, NULL, NULL, TO_CHAR);
			dam = dice(level,6);

			if (saves_spell(level, vch, DAM_LIGHTNING))
				dam /= 3;
			damage(ch, vch, dam, sn, DAM_LIGHTNING, DAMF_SHOW);
			level -= 4;  /* decrement damage */
		} 

		if (found)
			continue;

/* no victim found, hit the caster */
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
		char_puts("You are struck by your own lightning!\n", ch);
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
		char_puts("This room has already been healed by light.\n",ch);
		return;
	}

	af.where     = TO_ROOM_CONST;
	af.type      = sn;
	af.level     = level;
	af.duration  = level / 25;
	INT(af.location) = APPLY_ROOM_HEAL;
	af.modifier  = level * 3 / 2;
	af.bitvector = 0;
	af.owner     = ch;
	af.events    = 0;
	affect_to_room(ch->in_room, &af);

	af2.where     = TO_AFFECTS;
	af2.type      = sn;
	af2.level	 = level;
	af2.duration  = level / 10;
	af2.modifier  = 0;
	INT(af2.location) = APPLY_NONE;
	af2.bitvector = 0;
	affect_to_char(ch, &af2);
	char_puts("The room starts to be filled with healing light.\n", ch);
	act("The room starts to be filled with $n's healing light.",ch,NULL,NULL,TO_ROOM);
}

void spell_charm_person(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (count_charmed(ch))
		return;

	if (victim == ch) {
		char_puts("You like yourself even better!\n", ch);
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
	||  level < LEVEL(victim)
	||  IS_SET(victim->imm_flags, IMM_CHARM)
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

	dam = number_range(1, level);
	if (!saves_spell(level, victim, DAM_COLD)) {
		act("$n turns blue and shivers.", victim, NULL, NULL, TO_ROOM);
		af.where     = TO_AFFECTS;
		af.type      = sn;
		af.level     = level;
		af.duration  = 6;
		INT(af.location) = APPLY_STR;
		af.modifier  = -1;
		af.bitvector = 0;
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

void spell_continual_light(const char *sn, int level,CHAR_DATA *ch,void *vo)
{
	OBJ_DATA *light;

	if (target_name[0] != '\0')  /* do a glow on some object */
	{
		light = get_obj_carry(ch,target_name);

		if (light == NULL)
		{
		    char_puts("You don't see that here.\n",ch);
		    return;
		}

		if (IS_OBJ_STAT(light,ITEM_GLOW))
		{
		    act("$p is already glowing.",ch,light,NULL,TO_CHAR);
		    return;
		}

		SET_BIT(light->extra_flags,ITEM_GLOW);
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
		char_puts ("Do you want it to get better or worse?\n", ch);
		return;
	}

	char_puts("Ok.\n", ch);
}

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

void spell_create_rose(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	OBJ_DATA *rose = create_obj(get_obj_index(OBJ_VNUM_ROSE), 0);
	act("$n has created $p.", ch, rose, NULL, TO_ROOM);
	act("You create $p.", ch, rose, NULL, TO_CHAR);
	obj_to_char(rose, ch);
}

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

	if (obj->pObjIndex->item_type != ITEM_DRINK_CON) {
		char_puts("It is unable to hold water.\n", ch);
		return;
	}

	if (str_cmp(STR(obj->value[2]), "water")
	&&  INT(obj->value[1]) != 0) {
		char_puts("It contains some other liquid.\n", ch);
		return;
	}

	water = UMIN(level * (weather_info.sky >= SKY_RAINING ? 4 : 2),
		     INT(obj->value[0]) - INT(obj->value[1]));

	if (water > 0) {
		STR_ASSIGN(obj->value[2], str_dup("water"));
		INT(obj->value[1]) += water;

		if (!is_name("water", obj->name)) {
			const char *p = obj->name;
			obj->name = str_printf("%s %s", obj->name, "water");
			free_string(p);
		}

		act("$p is filled.", ch, obj, NULL, TO_CHAR);
	}
}

void spell_cure_blindness(const char *sn, int level,CHAR_DATA *ch,void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (!is_affected(victim, "blindness")) {
		if (victim == ch)
		  char_puts("You aren't blind.\n",ch);
		else
		  act("$N doesn't appear to be blinded.",ch,NULL,victim,TO_CHAR);
		return;
	}

	if (check_dispel(level, victim, "blindness")) {
		char_puts("Your vision returns!\n", victim);
		act("$n is no longer blinded.",victim,NULL,NULL,TO_ROOM);
	}
	else
		char_puts("Spell failed.\n",ch);
}

void spell_cure_critical(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int heal;

	heal = dice(3, 8) + level / 2 ;
	victim->hit = UMIN(victim->hit + heal, victim->max_hit);
	update_pos(victim);
	char_puts("You feel better!\n", victim);
	if (ch != victim)
		char_puts("Ok.\n", ch);
}

/* RT added to cure plague */
void spell_cure_disease(const char *sn, int level, CHAR_DATA *ch,void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (!is_affected(victim, "plague")) {
		if (victim == ch)
		  char_puts("You aren't ill.\n",ch);
		else
		  act("$N doesn't appear to be diseased.",ch,NULL,victim,TO_CHAR);
		return;
	}

	if (check_dispel(level, victim, "plague"))
		act("$n looks relieved as $s sores vanish.",victim,NULL,NULL,TO_ROOM);
	else
		char_puts("Spell failed.\n",ch);
}

void spell_cure_light(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int heal;

	heal = dice(1, 8) + level / 4 + 5;
	victim->hit = UMIN(victim->hit + heal, victim->max_hit);
	update_pos(victim);
	char_puts("You feel better!\n", victim);
	if (ch != victim)
		char_puts("Ok.\n", ch);
}

void spell_cure_poison(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;

	if (!is_affected(victim, "poison")) {
		if (victim == ch)
		  char_puts("You aren't poisoned.\n",ch);
		else
		  act("$N doesn't appear to be poisoned.",ch,NULL,victim,TO_CHAR);
		return;
	}

	if (check_dispel(level, victim, "poison"))
	{
		char_puts("A warm feeling runs through your body.\n",victim);
		act("$n looks much better.",victim,NULL,NULL,TO_ROOM);
	}
	else
		char_puts("Spell failed.\n",ch);
}

void spell_cure_serious(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int heal;

	heal = dice(2, 8) + level / 3 + 10 ;
	victim->hit = UMIN(victim->hit + heal, victim->max_hit);
	update_pos(victim);
	char_puts("You feel better!\n", victim);
	if (ch != victim)
		char_puts("Ok.\n", ch);
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
				REMOVE_BIT(obj->extra_flags, ITEM_BLESS);
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
	affect_to_char(victim, &af);

	INT(af.location) = APPLY_SAVING_SPELL;
	af.modifier  = level / 8;
	affect_to_char(victim, &af);

	char_puts("You feel unclean.\n", victim);
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
		char_puts("You failed.\n", ch);
		return;
	}

	af.where 	= TO_AFFECTS;
	af.type  	= sn;
	af.level 	= level;
	af.duration	= (8 + level/10);
	INT(af.location)= APPLY_HITROLL;
	af.modifier	= -strength;
	af.bitvector	= AFF_CURSE;
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
	char_puts("You feel unclean.\n", victim);
}

/* RT replacement demonfire spell */
void spell_demonfire(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	if (!IS_NPC(ch) && !IS_EVIL(ch))
	{
		victim = ch;
		char_puts("The demons turn upon you!\n",ch);
	}

	if (victim != ch)
	{
		act("$n calls forth the demons of Hell upon $N!",
		    ch,NULL,victim,TO_ROOM);
		act("$n has assailed you with the demons of Hell!",
		    ch,NULL,victim,TO_VICT);
		char_puts("You conjure forth the demons of hell!\n",ch);
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

	if (!IS_NPC(ch) && !IS_NEUTRAL(ch))
	{
		victim = ch;
		char_puts("Your blue fire turn upon you!\n",ch);
	}

	if (victim != ch)
	{
		act("$n calls forth the blue fire of earth $N!",
		    ch,NULL,victim,TO_ROOM);
		act("$n has assailed you with the neutrals of earth!",
		    ch,NULL,victim,TO_VICT);
		char_puts("You conjure forth the blue fire!\n",ch);
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

	if (IS_AFFECTED(victim, AFF_DETECT_EVIL))
	{
		if (victim == ch)
		  char_puts("You can already sense evil.\n",ch);
		else
		  act("$N can already detect evil.",ch,NULL,victim,TO_CHAR);
		return;
	}
	af.where     = TO_AFFECTS;
	af.type      = sn;
	af.level	 = level;
	af.duration  = (5 + level / 3);
	af.modifier  = 0;
	INT(af.location) = APPLY_NONE;
	af.bitvector = AFF_DETECT_EVIL;
	affect_to_char(victim, &af);
	char_puts("Your eyes tingle.\n", victim);
	if (ch != victim)
		char_puts("Ok.\n", ch);
}

void spell_detect_good(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (IS_AFFECTED(victim, AFF_DETECT_GOOD))
	{
		if (victim == ch)
		  char_puts("You can already sense good.\n",ch);
		else
		  act("$N can already detect good.",ch,NULL,victim,TO_CHAR);
		return;
	}
	af.where     = TO_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = (5 + level / 3);
	af.modifier  = 0;
	INT(af.location) = APPLY_NONE;
	af.bitvector = AFF_DETECT_GOOD;
	affect_to_char(victim, &af);
	char_puts("Your eyes tingle.\n", victim);
	if (ch != victim)
		char_puts("Ok.\n", ch);
}

void spell_detect_hidden(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (IS_AFFECTED(victim, AFF_DETECT_HIDDEN)) {
		if (victim == ch)
			char_puts("You are already as alert as you can be.\n",
				  ch);
		else
			act("$N can already sense hidden lifeforms.",
			    ch, NULL, victim, TO_CHAR);
		return;
	}

	af.where	= TO_AFFECTS;
	af.type		= sn;
	af.level	= level;
	af.duration	= 5 + level / 3;
	INT(af.location)= APPLY_NONE;
	af.modifier	= 0;
	af.bitvector	= AFF_DETECT_HIDDEN;
	affect_to_char(victim, &af);
	char_puts("Your awareness improves.\n", victim);
	if (ch != victim)
		char_puts("Ok.\n", ch);
}

void spell_detect_fade(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (IS_AFFECTED(victim, AFF_DETECT_FADE)) {
		if (victim == ch)
			char_puts("You are already as alert as you can be.\n",
				  ch);
		else
			act("$N can already sense faded lifeforms.",
			    ch, NULL, victim, TO_CHAR);
		return;
	}

	af.where	= TO_AFFECTS;
	af.type		= sn;
	af.level	= level;
	af.duration	= 5 + level / 3;
	INT(af.location)= APPLY_NONE;
	af.modifier	= 0;
	af.bitvector	= AFF_DETECT_FADE;
	affect_to_char(victim, &af);
	char_puts("Your awareness improves.\n", victim);
	if (ch != victim)
		char_puts("Ok.\n", ch);
}

void spell_detect_invis(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (IS_AFFECTED(victim, AFF_DETECT_INVIS))
	{
		if (victim == ch)
		  char_puts("You can already see invisible.\n",ch);
		else
		  act("$N can already see invisible things.",ch,NULL,victim,TO_CHAR);
		return;
	}

	af.where     = TO_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = (5 + level / 3);
	af.modifier  = 0;
	INT(af.location) = APPLY_NONE;
	af.bitvector = AFF_DETECT_INVIS;
	affect_to_char(victim, &af);
	char_puts("Your eyes tingle.\n", victim);
	if (ch != victim)
		char_puts("Ok.\n", ch);
}

void spell_detect_magic(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (IS_AFFECTED(victim, AFF_DETECT_MAGIC))
	{
		if (victim == ch)
		  char_puts("You can already sense magical auras.\n",ch);
		else
		  act("$N can already detect magic.",ch,NULL,victim,TO_CHAR);
		return;
	}

	af.where     = TO_AFFECTS;
	af.type      = sn;
	af.level	 = level;
	af.duration  = (5 + level / 3);
	af.modifier  = 0;
	INT(af.location) = APPLY_NONE;
	af.bitvector = AFF_DETECT_MAGIC;
	affect_to_char(victim, &af);
	char_puts("Your eyes tingle.\n", victim);
	if (ch != victim)
		char_puts("Ok.\n", ch);
}

void spell_detect_poison(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;

	if (obj->pObjIndex->item_type == ITEM_DRINK_CON || obj->pObjIndex->item_type == ITEM_FOOD)
	{
		if (INT(obj->value[3]) != 0)
		    char_puts("You smell poisonous fumes.\n", ch);
		else
		    char_puts("It looks delicious.\n", ch);
	}
	else
	{
		char_puts("It doesn't look poisoned.\n", ch);
	}
}

void spell_dispel_evil(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	if (!IS_NPC(ch) && IS_EVIL(ch))
		victim = ch;

	if (IS_GOOD(victim))
	{
		act("Gods protects $N.", ch, NULL, victim, TO_ROOM);
		return;
	}

	if (IS_NEUTRAL(victim))
	{
		act("$N does not seem to be affected.", ch, NULL, victim, TO_CHAR);
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

	if (IS_EVIL(victim))
	{
		act("$N is protected by $S evil.", ch, NULL, victim, TO_ROOM);
		return;
	}

	if (IS_NEUTRAL(victim))
	{
		act("$N does not seem to be affected.", ch, NULL, victim, TO_CHAR);
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

void spell_earthquake(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;

	char_puts("The earth trembles beneath your feet!\n", ch);
	act("$n makes the earth tremble and shiver.", ch, NULL, NULL, TO_ROOM);

	for (vch = char_list; vch; vch = vch_next) {
		vch_next	= vch->next;

		if (vch->in_room == NULL)
			continue;

		if (vch->in_room == ch->in_room) {
			if (is_safe_spell(ch, vch, TRUE))
				continue;

			if (IS_AFFECTED(vch,AFF_FLYING))
				damage(ch, vch, 0, sn, DAM_BASH, DAMF_SHOW);
			else
				damage(ch, vch, level + dice(2, 8), sn,
				       DAM_BASH, DAMF_SHOW);
			continue;
		}

		if (vch->in_room->area == ch->in_room->area)
			char_puts("The earth trembles and shivers.\n", vch);
	}
}

void spell_enchant_armor(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	AFFECT_DATA *paf;
	int result, fail;
	int ac_bonus, added;
	bool ac_found = FALSE;
	bool hp_found = FALSE;

	if (obj->pObjIndex->item_type != ITEM_ARMOR) {
		char_puts("That isn't an armor.\n",ch);
		return;
	}

	if (obj->wear_loc != -1) {
		char_puts("The item must be carried to be enchanted.\n",ch);
		return;
	}

	/* this means they have no bonus */
	ac_bonus = 0;
	fail = 25;	/* base 25% chance of failure */

	/* find the bonuses */

	if (!IS_SET(obj->extra_flags, ITEM_ENCHANTED))
		for (paf = obj->pObjIndex->affected; 
	 	     paf != NULL; 
		     paf = paf->next) {
			if (paf->where == TO_SKILLS
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

		SET_BIT(obj->extra_flags, ITEM_ENCHANTED);
		return;
	}

	if (result <= fail) { /* failed, no bad result */
		char_puts("Nothing seemed to happen.\n",ch);
		return;
	}

	affect_enchant(obj);

	if (result <= (90 - level / 5)) { /* success! */
		act("$p shimmers with a gold aura.", ch, obj, NULL, TO_CHAR);
		act("$p shimmers with a gold aura.", ch, obj, NULL, TO_ROOM);
		SET_BIT(obj->extra_flags, ITEM_MAGIC);
		added = -1;
	} else { /* exceptional enchant */
		act("$p glows a brillant gold!", ch, obj, NULL, TO_CHAR);
		act("$p glows a brillant gold!", ch, obj, NULL, TO_ROOM);
		SET_BIT(obj->extra_flags, ITEM_MAGIC | ITEM_GLOW);
		added = -2;
	}
			
	/* now add the enchantments */

	if (obj->level < LEVEL_HERO)
		obj->level = UMIN(LEVEL_HERO - 1,obj->level + 1);

	if (ac_found) {
		for (paf = obj->affected; paf != NULL; paf = paf->next) {
			if (paf->where == TO_SKILLS
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

	if (obj->pObjIndex->item_type != ITEM_WEAPON) {
		char_puts("That isn't a weapon.\n",ch);
		return;
	}

	if (obj->wear_loc != -1) {
		char_puts("The item must be carried to be enchanted.\n", ch);
		return;
	}

	/* this means they have no bonus */
	hit_bonus = 0;
	dam_bonus = 0;
	fail = 25;	/* base 25% chance of failure */

	/* find the bonuses */

	if (!IS_SET(obj->extra_flags, ITEM_ENCHANTED))
		for (paf = obj->pObjIndex->affected; paf != NULL; paf = paf->next) {
			if (paf->where == TO_SKILLS
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

		SET_BIT(obj->extra_flags, ITEM_ENCHANTED);
		return;
	}

	if (result <= fail) { /* failed, no bad result */
		char_puts("Nothing seemed to happen.\n",ch);
		return;
	}

	affect_enchant(obj);

	if (result <= (100 - level/5)) { /* success! */
		act("$p glows blue.", ch, obj, NULL, TO_CHAR);
		act("$p glows blue.", ch, obj, NULL, TO_ROOM);
		SET_BIT(obj->extra_flags, ITEM_MAGIC);
		added = 1;
	} else { /* exceptional enchant */
		act("$p glows a brillant blue!", ch, obj, NULL, TO_CHAR);
		act("$p glows a brillant blue!", ch, obj, NULL, TO_ROOM);
		SET_BIT(obj->extra_flags, ITEM_MAGIC | ITEM_GLOW);
		added = 2;
	}
			
	/* now add the enchantments */ 

	if (obj->level < LEVEL_HERO - 1)
		obj->level = UMIN(LEVEL_HERO - 1, obj->level + 1);

	if (dam_found) {
		for (paf = obj->affected; paf != NULL; paf = paf->next) {
			if (paf->where == TO_SKILLS
			||  paf->where == TO_RACE)
				continue;

			if (INT(paf->location) == APPLY_DAMROLL) {
				free_string(paf->type);
				paf->type = str_dup(sn);
				paf->modifier += added;
				paf->level = UMAX(paf->level, level);
				if (paf->modifier > 4)
					SET_BIT(obj->extra_flags, ITEM_HUM);
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
				SET_BIT(obj->extra_flags, ITEM_HUM);
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
		char_puts("You feel a momentary chill.\n", victim);
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

		affect_join(victim, &af);
	}

	char_puts("You feel your life slipping away!\n", victim);
	char_puts("Wow....what a rush!\n", ch);
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
		affect_to_char(victim, &af);

		act("You feel drained!", victim, NULL, NULL, TO_VICT);
	}
}

void spell_hellfire(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	damage(ch, victim, dice(level, 7), sn, DAM_FIRE, DAMF_SHOW);
}

void spell_iceball(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;
	int dam;
	int movedam;

	dam = dice(level , 12);
	movedam     = number_range(level, 2 * level);

	for (vch = ch->in_room->people; vch; vch = vch_next) {
		vch_next = vch->next_in_room;

		if (is_safe_spell(ch, vch, TRUE))
			continue;

		if (saves_spell(level, vch, DAM_COLD))
			dam /= 2;
		vch->move -= UMIN(vch->move, movedam);
		damage(ch, vch, dam, sn, DAM_COLD, DAMF_SHOW);
	}
}

void spell_fireball(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	dam = dice(level, 15);
	if (saves_spell(level, victim, DAM_FIRE))
		dam /= 2;
	damage(ch, victim, dam, sn, DAM_FIRE, DAMF_SHOW);
}

void spell_fireproof(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	OBJ_DATA *obj = (OBJ_DATA *) vo;
	AFFECT_DATA af;

	if (IS_OBJ_STAT(obj,ITEM_BURN_PROOF))
	{
		act("$p is already protected from burning.",ch,obj,NULL,TO_CHAR);
		return;
	}

	af.where     = TO_OBJECT;
	af.type      = sn;
	af.level     = level;
	af.duration  = number_fuzzy(level / 4);
	INT(af.location) = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = ITEM_BURN_PROOF;

	affect_to_obj(obj,&af);

	act("You protect $p from fire.",ch,obj,NULL,TO_CHAR);
	act("$p is surrounded by a protective aura.",ch,obj,NULL,TO_ROOM);
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
	affect_to_char(victim, &af);
	char_puts("You are surrounded by a pink outline.\n", victim);
	act("$n is surrounded by a pink outline.", victim, NULL, NULL, TO_ROOM);
}

void spell_faerie_fog(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *ich;

	act("$n conjures a cloud of purple smoke.", ch, NULL, NULL, TO_ROOM);
	char_puts("You conjure a cloud of purple smoke.\n", ch);

	for (ich = ch->in_room->people; ich != NULL; ich = ich->next_in_room) {
		if (ich->invis_level > 0)
			continue;

		if (ich == ch || saves_spell(level, ich, DAM_OTHER))
			continue;

		affect_bit_strip(ich, TO_AFFECTS, AFF_INVIS | AFF_IMP_INVIS);
		REMOVE_BIT(ich->affected_by, AFF_HIDE | AFF_FADE |
					     AFF_CAMOUFLAGE | AFF_INVIS |
					     AFF_IMP_INVIS);

		act("$n is revealed!", ich, NULL, NULL, TO_ROOM);
		char_puts("You are revealed!\n", ich);
	}
}

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
	affect_to_char(ch, &af);

	act("$n has created a floating black disc.", ch, NULL, NULL, TO_ROOM);
	char_puts("You create a floating disc.\n", ch);
	obj_to_char(disc, ch);
	wear_obj(ch, disc, TRUE);
}

void spell_fly(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (IS_AFFECTED(victim, AFF_FLYING))
	{
		if (victim == ch)
		  char_puts("You are already airborne.\n",ch);
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
	affect_to_char(victim, &af);
	char_puts("Your feet rise off the ground.\n", victim);
	act("$n's feet rise off the ground.", victim, NULL, NULL, TO_ROOM);
}

/* RT clerical berserking spell */
void spell_frenzy(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (is_affected(victim, sn) || IS_AFFECTED(victim, AFF_BERSERK)) {
		if (victim == ch)
			char_puts("You are already in a frenzy.\n", ch);
		else
			act("$N is already in a frenzy.",
			    ch, NULL, victim, TO_CHAR);
		return;
	}

	if (is_affected(victim, "calm")) {
		if (victim == ch)
			char_puts("Why don't you just relax for a while?\n", ch);
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

	INT(af.location)= APPLY_HITROLL;
	affect_to_char(victim,&af);

	INT(af.location)= APPLY_DAMROLL;
	affect_to_char(victim,&af);

	af.modifier	= 10 * (level / 12);
	INT(af.location)= APPLY_AC;
	affect_to_char(victim,&af);

	char_puts("You are filled with holy wrath!\n", victim);
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
		char_puts("You failed.\n", ch);
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
		  char_puts("You are already as strong as you can get!\n",ch);
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
	affect_to_char(victim, &af);
	char_puts("Your muscles surge with heightened power!\n", victim);
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
			char_puts("You can't move any faster!\n",ch);
		else
			act("$N is already moving as fast as $E can.",
			    ch, NULL, victim, TO_CHAR);
		return;
	}

	if (IS_AFFECTED(victim, AFF_SLOW)) {
		if (!check_dispel(level, victim, "slow")) {
			if (victim != ch)
				char_puts("Spell failed.\n",ch);
			char_puts("You feel momentarily faster.\n", victim);
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
	affect_to_char(victim, &af);
	char_puts("You feel yourself moving more quickly.\n", victim);
	act("$n is moving more quickly.", victim, NULL, NULL, TO_ROOM);
	if (ch != victim)
		char_puts("Ok.\n", ch);
}

void spell_heal(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	victim->hit = UMIN(victim->hit + 100 + level / 10, victim->max_hit);
	update_pos(victim);
	char_puts("A warm feeling fills your body.\n", victim);
	if (ch != victim)
		char_puts("Ok.\n", ch);
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

	affect_to_char(ch, &af);
}

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
		char_puts("You failed.", ch);
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
	affect_to_char(victim, &af);
}

void spell_heat_metal(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	OBJ_DATA *obj_lose, *obj_next;
	int dam = 0;
	bool fail = TRUE;

   if (!saves_spell(level + 2,victim,DAM_FIRE)
   &&  !IS_SET(victim->imm_flags,IMM_FIRE))
   {
		for (obj_lose = victim->carrying;
		      obj_lose != NULL;
		      obj_lose = obj_next)
		{
		    obj_next = obj_lose->next_content;
		    if (number_range(1,2 * level) > obj_lose->level
		    &&   !saves_spell(level,victim,DAM_FIRE)
		    &&   material_is(obj_lose, MATERIAL_METAL)
		    &&   !IS_OBJ_STAT(obj_lose,ITEM_BURN_PROOF))
		    {
			switch (obj_lose->pObjIndex->item_type)
			{
			case ITEM_ARMOR:
			if (obj_lose->wear_loc != -1) /* remove the item */
			{
			    if (can_drop_obj(victim,obj_lose)
			    &&  (obj_lose->weight / 10) <
				number_range(1,2 * get_curr_stat(victim,STAT_DEX))
			    &&  remove_obj(victim, obj_lose->wear_loc, TRUE))
			    {
				act("$n yelps and throws $p to the ground!",
				    victim,obj_lose,NULL,TO_ROOM);
				act("You remove and drop $p before it burns you.",
				    victim,obj_lose,NULL,TO_CHAR);
				dam += (number_range(1,obj_lose->level) / 3);
				obj_from_char(obj_lose);
				obj_to_room(obj_lose, victim->in_room);
				fail = FALSE;
			    }
			    else /* stuck on the body! ouch! */
			    {
				act("Your skin is seared by $p!",
				    victim,obj_lose,NULL,TO_CHAR);
				dam += (number_range(1,obj_lose->level));
				fail = FALSE;
			    }

			}
			else /* drop it if we can */
			{
			    if (can_drop_obj(victim,obj_lose))
			    {
				act("$n yelps and throws $p to the ground!",
				    victim,obj_lose,NULL,TO_ROOM);
				act("You and drop $p before it burns you.",
				    victim,obj_lose,NULL,TO_CHAR);
				dam += (number_range(1,obj_lose->level) / 6);
				obj_from_char(obj_lose);
				obj_to_room(obj_lose, victim->in_room);
				fail = FALSE;
			    }
			    else /* cannot drop */
			    {
				act("Your skin is seared by $p!",
				    victim,obj_lose,NULL,TO_CHAR);
				dam += (number_range(1,obj_lose->level) / 2);
				fail = FALSE;
			    }
			}
			break;
			case ITEM_WEAPON:
			if (obj_lose->wear_loc != -1) /* try to drop it */
			{
			    if (IS_WEAPON_STAT(obj_lose,WEAPON_FLAMING))
				continue;

			    if (can_drop_obj(victim,obj_lose)
			    &&  remove_obj(victim,obj_lose->wear_loc,TRUE))
			    {
				act("$n is burned by $p, and throws it to the ground.",
				    victim,obj_lose,NULL,TO_ROOM);
				char_puts(
				    "You throw your red-hot weapon to the ground!\n",
				    victim);
				dam += 1;
				obj_from_char(obj_lose);
				obj_to_room(obj_lose,victim->in_room);
				fail = FALSE;
			    }
			    else /* YOWCH! */
			    {
				char_puts("Your weapon sears your flesh!\n",
				    victim);
				dam += number_range(1,obj_lose->level);
				fail = FALSE;
			    }
			}
			else /* drop it if we can */
			{
			    if (can_drop_obj(victim,obj_lose))
			    {
				act("$n throws a burning hot $p to the ground!",
				    victim,obj_lose,NULL,TO_ROOM);
				act("You and drop $p before it burns you.",
				    victim,obj_lose,NULL,TO_CHAR);
				dam += (number_range(1,obj_lose->level) / 6);
				obj_from_char(obj_lose);
				obj_to_room(obj_lose, victim->in_room);
				fail = FALSE;
			    }
			    else /* cannot drop */
			    {
				act("Your skin is seared by $p!",
				    victim,obj_lose,NULL,TO_CHAR);
				dam += (number_range(1,obj_lose->level) / 2);
				fail = FALSE;
			    }
			}
			break;
			}
		    }
		}
	}
	if (fail)
	{
		char_puts("Your spell had no effect.\n", ch);
		char_puts("You feel momentarily warmer.\n",victim);
	}
	else /* damage! */
	{
		if (saves_spell(level,victim,DAM_FIRE))
		    dam = 2 * dam / 3;
		damage(ch, victim, dam, sn, DAM_FIRE, DAMF_SHOW);
	}
}

/* RT really nasty high-level attack spell */
void spell_holy_word(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;
	int dam;

	act("$n utters a word of divine power!", ch, NULL, NULL, TO_ROOM);
	char_puts("You utter a word of divine power.\n", ch);

	for (vch = ch->in_room->people; vch != NULL; vch = vch_next) {
		vch_next = vch->next_in_room;

		if ((IS_GOOD(ch) && IS_GOOD(vch)) ||
		    (IS_EVIL(ch) && IS_EVIL(vch)) ||
		    (IS_NEUTRAL(ch) && IS_NEUTRAL(vch))) {
			char_puts("You feel full more powerful.\n", vch);
			spellfun_call("frenzy", NULL, level, ch, vch);
			spellfun_call("bless", NULL, level, ch, vch);
			continue;
		}

		if (is_safe_spell(ch, vch, TRUE))
			continue;

		if ((IS_GOOD(ch) && IS_EVIL(vch))
		||  (IS_EVIL(ch) && IS_GOOD(vch))) {
			spellfun_call("curse", NULL, level, ch, vch);
			char_puts("You are struck down!\n",vch);
			dam = dice(level, 6);
			damage(ch, vch, dam, sn, DAM_ENERGY, DAMF_SHOW);
			continue;
		}

		if (IS_NEUTRAL(ch)) {
			spellfun_call("curse", NULL, level/2, ch, vch);
			char_puts("You are struck down!\n", vch);
			dam = dice(level, 4);
			damage(ch, vch, dam, sn, DAM_ENERGY, DAMF_SHOW);
		}
	}

	char_puts("You feel drained.\n", ch);
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
	if (!IS_SET(obj->extra_flags, ITEM_ENCHANTED))
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
	if (!IS_SET(obj->extra_flags, ITEM_ENCHANTED))
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

	if (IS_AFFECTED(victim, AFF_INFRARED)) {
		if (victim == ch)
			char_puts("You can already see in the dark.\n", ch);
		else
			act("$N already has infravision.\n",
			    ch, NULL, victim,TO_CHAR);
		return;
	}
	act("$n's eyes glow red.\n", ch, NULL, NULL, TO_ROOM);

	af.where	= TO_AFFECTS;
	af.type		= sn;
	af.level	= level;
	af.duration	= 2 * level;
	INT(af.location)= APPLY_NONE;
	af.modifier	= 0;
	af.bitvector	= AFF_INFRARED;
	affect_to_char(victim, &af);
	char_puts("Your eyes glow red.\n", victim);
}

void spell_invisibility(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim;
	AFFECT_DATA af;

	/* object invisibility */
	if (mem_is(vo, MT_OBJ)) {
		OBJ_DATA *obj = (OBJ_DATA *) vo;

		if (IS_OBJ_STAT(obj,ITEM_INVIS))
		{
		    act("$p is already invisible.",ch,obj,NULL,TO_CHAR);
		    return;
		}

		af.where	= TO_OBJECT;
		af.type		= sn;
		af.level	= level;
		af.duration	= level / 4 + 12;
		INT(af.location)= APPLY_NONE;
		af.modifier	= 0;
		af.bitvector	= ITEM_INVIS;
		affect_to_obj(obj,&af);

		act("$p fades out of sight.",ch,obj,NULL,TO_ALL);
		return;
	}

	/* character invisibility */
	victim = (CHAR_DATA *) vo;

	if (IS_AFFECTED(victim, AFF_INVIS))
		return;

	act("$n fades out of existence.", victim, NULL, NULL, TO_ROOM);

	af.where     = TO_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = (level / 8 + 10);
	INT(af.location) = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_INVIS;
	affect_to_char(victim, &af);
	char_puts("You fade out of existence.\n", victim);
}

void spell_know_alignment(const char *sn, int level,CHAR_DATA *ch,void *vo)
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
		if (!can_see_obj(ch, obj) || !is_name(target_name, obj->name)
		||  IS_OBJ_STAT(obj, ITEM_NOLOCATE)
		||  (IS_SET(obj->pObjIndex->extra_flags, ITEM_CHQUEST) &&
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
			buf_printf(buffer, "One is carried by %s\n",
				   PERS(in_obj->carried_by, ch));
		}
		else {
			if (IS_IMMORTAL(ch) && in_obj->in_room != NULL) {
				buf_printf(buffer, "One is in %s [Room %d]\n",
					   mlstr_cval(&in_obj->in_room->name, ch),
					   in_obj->in_room->vnum);
			} else {
				buf_printf(buffer, "One is in %s\n",
					   in_obj->in_room == NULL ?
					   "somewhere" :
					   mlstr_cval(&in_obj->in_room->name, ch));
			}
		}

		if (number >= max_found)
			break;
	}

	if (buffer == NULL)
		char_puts("Nothing like that in heaven or earth.\n", ch);
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

	for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room)
		if ((IS_NPC(ch) && IS_NPC(gch))
		||  (!IS_NPC(ch) && !IS_NPC(gch))) {
			spellfun_call("heal", NULL, level, ch, gch);
			spellfun_call("refresh", NULL, level, ch, gch);
		}
}

void spell_mass_invis(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	AFFECT_DATA af;
	CHAR_DATA *gch;

	for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room)
	{
		if (!is_same_group(gch, ch) || IS_AFFECTED(gch, AFF_INVIS))
		    continue;
		if (spellbane(gch, ch, 100, dice(2, LEVEL(gch)))) {
			if (IS_EXTRACTED(ch)) 
				return;
			else 
				continue;
		}
		act("$n slowly fades out of existence.", gch, NULL, NULL, TO_ROOM);
		char_puts("You slowly fade out of existence.\n", gch);

		af.where     = TO_AFFECTS;
		af.type      = sn;
		af.level     = level/2;
		af.duration  = 24;
		INT(af.location) = APPLY_NONE;
		af.modifier  = 0;
		af.bitvector = AFF_INVIS;
		affect_to_char(gch, &af);
	}
	char_puts("Ok.\n", ch);
}

void spell_pass_door(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (IS_AFFECTED(victim, AFF_PASS_DOOR))
	{
		if (victim == ch)
		  char_puts("You are already out of phase.\n",ch);
		else
		  act("$N is already shifted out of phase.",ch,NULL,victim,TO_CHAR);
		return;
	}

	af.where     = TO_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = number_fuzzy(level / 4);
	INT(af.location) = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_PASS_DOOR;
	affect_to_char(victim, &af);
	act("$n turns translucent.", victim, NULL, NULL, TO_ROOM);
	char_puts("You turn translucent.\n", victim);
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
	af.events	= EVENT_CHAR_UPDATE;
	affect_join(victim,&af);

	char_puts
	  ("You scream in agony as plague sores erupt from your skin.\n",victim);
	act("$n screams in agony as plague sores erupt from $s skin.",
		victim,NULL,NULL,TO_ROOM);
}

void spell_poison(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim;
	AFFECT_DATA af;

	if (mem_is(vo, MT_OBJ)) {
		OBJ_DATA *obj = (OBJ_DATA *) vo;

		if (obj->pObjIndex->item_type == ITEM_FOOD || obj->pObjIndex->item_type == ITEM_DRINK_CON)
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

		if (obj->pObjIndex->item_type == ITEM_WEAPON)
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
	af.events    = EVENT_CHAR_UPDATE;
	affect_join(victim, &af);
	char_puts("You feel very sick.\n", victim);
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
		  char_puts("You are already protected.\n",ch);
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
	affect_to_char(victim, &af);
	char_puts("You feel holy and pure.\n", victim);
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
		  char_puts("You are already protected.\n",ch);
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
	affect_to_char(victim, &af);
	char_puts("You feel aligned with darkness.\n", victim);
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
		char_puts("The energy explodes inside you!\n",ch);
	}

	if (victim != ch)
	{
		act("$n raises $s hand, and a blinding ray of light shoots forth!",
		    ch,NULL,NULL,TO_ROOM);
		char_puts(
		   "You raise your hand and a blinding ray of light shoots forth!\n",
		   ch);
	}

	if (IS_GOOD(victim))
	{
		act("$n seems unharmed by the light.",victim,NULL,victim,TO_ROOM);
		char_puts("The light seems powerless to affect you.\n",victim);
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

	if (obj->pObjIndex->item_type != ITEM_WAND
	&&  obj->pObjIndex->item_type != ITEM_STAFF) {
		char_puts("That item does not carry charges.\n",ch);
		return;
	}

	if (INT(obj->value[3]) >= 3 * level / 2) {
		char_puts("Your skills are not great enough for that.\n", ch);
		return;
	}

	if (INT(obj->value[1]) == 0) {
		char_puts("That item has already been recharged once.\n", ch);
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
		char_puts("Nothing seems to happen.\n", ch);
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
		char_puts("You feel fully refreshed!\n",victim);
	else
		char_puts("You feel less tired.\n", victim);
	if (ch != victim)
		char_puts("Ok.\n", ch);
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
		    if (!IS_OBJ_STAT(obj,ITEM_NOUNCURSE)
		    &&  !saves_dispel(level + 2,obj->level,0))
		    {
			REMOVE_BIT(obj->extra_flags,ITEM_NODROP);
			REMOVE_BIT(obj->extra_flags,ITEM_NOREMOVE);
			act("$p glows blue.",ch,obj,NULL,TO_ALL);
			return;
		    }

		    act("The curse on $p is beyond your power.",ch,obj,NULL,TO_CHAR);
		    return;
		}
		else  {
		  char_puts("Nothing happens...\n", ch);
		  return;
		}
	}

	/* characters */
	victim = (CHAR_DATA *) vo;

	if (check_dispel(level, victim, "curse"))
	{
		char_puts("You feel better.\n",victim);
		act("$n looks more relaxed.",victim,NULL,NULL,TO_ROOM);
	}

   for (obj = victim->carrying; (obj != NULL && !found); obj = obj->next_content)
   {
		if ((IS_OBJ_STAT(obj,ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_NOREMOVE))
		&&  !IS_OBJ_STAT(obj,ITEM_NOUNCURSE))
		{   /* attempt to remove curse */
		    if (!saves_dispel(level,obj->level,0))
		    {
			found = TRUE;
			REMOVE_BIT(obj->extra_flags,ITEM_NODROP);
			REMOVE_BIT(obj->extra_flags,ITEM_NOREMOVE);
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
		if (victim == ch)
			char_puts("You are already in sanctuary.\n", ch);
		else
			act("$N is already in sanctuary.",
			    ch, NULL, victim, TO_CHAR);
		return;
	}

	if (is_affected(victim, "black shroud")) {
		if (victim == ch)
	 		char_puts("But you are surrounded by black shroud.\n",
				  ch);
		else
			act("But $N is surrounded by black shroud.\n",
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
	affect_to_char(victim, &af);
	act("$n is surrounded by a white aura.", victim, NULL, NULL, TO_ROOM);
	char_puts("You are surrounded by a white aura.\n", victim);
}

void spell_black_shroud(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA*) vo;
	AFFECT_DATA af;

	if (IS_AFFECTED(victim, AFF_SANCTUARY)) {
		if (victim==ch)
			char_puts("But you are in sanctuary.\n", ch);
		else
			act("But $N in sanctuary.", ch, NULL, victim,TO_CHAR);
		return;
	}

	if (!IS_EVIL(ch)) {
		char_puts("The gods are infuriated!.\n", ch);
		damage(ch, ch, dice(level, IS_GOOD(ch) ? 2 : 1),
		       NULL, DAM_HOLY, DAMF_SHOW);
		return;
	}
	
	if (!IS_EVIL(victim)) {
		act("Your god does not seems to like $N", 
		    ch, NULL, victim, TO_CHAR);
		return;
	}

	if (is_affected(victim, sn)) {
		if (victim == ch)
			char_puts("You are already protected.\n", ch);
		else
			act("$N is already protected.\n",
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
	affect_to_char(victim, &af);
	act ("$n is surrounded by black aura.", victim, NULL, NULL, TO_ROOM);
	char_puts("You are surrounded by black aura.\n", victim);
}

void spell_shield(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (is_affected(victim, sn))
	{
		if (victim == ch)
		  char_puts("You are already shielded from harm.\n",ch);
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
	affect_to_char(victim, &af);
	act("$n is surrounded by a force shield.", victim, NULL, NULL, TO_ROOM);
	char_puts("You are surrounded by a force shield.\n", victim);
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
	affect_join(victim, &af);

	if (IS_AWAKE(victim))
	{
		char_puts("You feel very sleepy ..... zzzzzz.\n", victim);
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
			char_puts("You can't move any slower!\n",ch);
		else {
			act("$N can't get any slower than that.",
			    ch, NULL, victim, TO_CHAR);
		}
		return;
	}

	if (saves_spell(level,victim, DAM_OTHER)
	||  IS_SET(victim->imm_flags, IMM_MAGIC))
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
	affect_to_char(victim, &af);
	char_puts("You feel yourself slowing d o w n...\n", victim);
	act("$n starts to move in slow motion.", victim, NULL, NULL, TO_ROOM);
}

void spell_stone_skin(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (is_affected(ch, sn))
	{
		if (victim == ch)
		  char_puts("Your skin is already as hard as a rock.\n",ch);
		else
		  act("$N is already as hard as can be.",ch,NULL,victim,TO_CHAR);
		return;
	}

	af.where     = TO_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = (10 + level / 5);
	INT(af.location) = APPLY_AC;
	af.modifier  = -1 * UMAX(40,20 + level / 2);  /*af.modifier=-40;*/ 
	af.bitvector = 0;
	affect_to_char(victim, &af);
	act("$n's skin turns to stone.", victim, NULL, NULL, TO_ROOM);
	char_puts("Your skin turns to stone.\n", victim);
}

void spell_summon(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	bool failed = FALSE;
	CHAR_DATA *victim;

	if ((victim = get_char_world(ch, target_name)) == NULL
	||  victim->in_room == NULL) {
		char_puts("You failed.\n", ch);
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
	||  IS_SET(victim->imm_flags, IMM_SUMMON)
	||  (victim->in_room->exit[0] == NULL &&
	     victim->in_room->exit[1] == NULL &&
	     victim->in_room->exit[2] == NULL &&
	     victim->in_room->exit[3] == NULL &&
	     victim->in_room->exit[4] == NULL &&
	     victim->in_room->exit[5] == NULL))
		failed = TRUE;
	else if (IS_NPC(victim)) {
		if (victim->pMobIndex->pShop != NULL
		||  saves_spell(level, victim, DAM_OTHER)
		||  IS_SET(victim->pMobIndex->act, ACT_AGGRESSIVE)
		||  IS_SET(ch->in_room->room_flags, ROOM_NOMOB)
		||  NPC(victim)->hunter)
			failed = TRUE;
	}
	else {
		if (victim->level >= LEVEL_HERO
		||  ((!in_PK(ch, victim) ||
		      ch->in_room->area != victim->in_room->area) &&
		     IS_SET(PC(victim)->plr_flags, PLR_NOSUMMON))
		||  !guild_ok(victim, ch->in_room))
			failed = TRUE;
	}

	if (failed) {
		char_puts("You failed.\n", ch);
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
	||  (victim != ch && IS_SET(victim->imm_flags,IMM_SUMMON))
	||  (!IS_NPC(ch) && victim->fighting != NULL)
	||  (victim != ch
	&&  (saves_spell(level - 5, victim,DAM_OTHER)))) {
		char_puts("You failed.\n", ch);
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
		send_to_char("You failed.\n",ch);
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
		}
		else {
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
	affect_to_char(victim, &af);
	char_puts("You feel your strength slip away.\n", victim);
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
			char_puts("Your honour doesn't let you recall!.\n", ch);
		else
			char_printf(ch, "You can't cast this spell to a "
					"honourable fighting %s!\n",
				    victim->class);
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
		char_puts("Spell failed.\n", victim);
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

void spell_fire_breath(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	CHAR_DATA *vch, *vch_next;
	int dam,hp_dam,dice_dam;
	int hpch;

	act("$n breathes forth a cone of fire.",ch,NULL,victim,TO_NOTVICT);
	act("$n breathes a cone of hot fire over you!",ch,NULL,victim,TO_VICT);
	act("You breath forth a cone of fire.",ch,NULL,NULL,TO_CHAR);

	hpch = UMAX(10, ch->hit);
	hp_dam  = number_range(hpch/9+1, hpch/5);
	dice_dam = dice(level,20);

	dam = UMAX(hp_dam + dice_dam /10, dice_dam + hp_dam / 10);
	fire_effect(victim->in_room,level,dam/2);

	for (vch = victim->in_room->people; vch != NULL; vch = vch_next)
	{
		vch_next = vch->next_in_room;

		if (is_safe_spell(ch,vch,TRUE)
		||  (IS_NPC(vch) && IS_NPC(ch) 
		&&  (ch->fighting != vch /*|| vch->fighting != ch */)))
		    continue;

		if (vch == victim) /* full damage */
		{
		    if (saves_spell(level, vch, DAM_FIRE)) {
			fire_effect(vch, level/2, dam/4);
			damage(ch, vch, dam/2, sn, DAM_FIRE, DAMF_SHOW);
		    } else {
			fire_effect(vch, level, dam);
			damage(ch, vch, dam, sn, DAM_FIRE, DAMF_SHOW);
		    }
		}
		else /* partial damage */
		{
		    if (saves_spell(level - 2, vch, DAM_FIRE)) {
			fire_effect(vch, level/4, dam/8);
			damage(ch, vch, dam/4, sn, DAM_FIRE, DAMF_SHOW);
		    } else {
			fire_effect(vch, level/2, dam/4);
			damage(ch, vch, dam/2, sn, DAM_FIRE, DAMF_SHOW);
		    }
		}
	}
}

void spell_frost_breath(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	CHAR_DATA *vch, *vch_next;
	int dam,hp_dam,dice_dam, hpch;

	act("$n breathes out a freezing cone of frost!",ch,NULL,victim,TO_NOTVICT);
	act("$n breathes a freezing cone of frost over you!",
		ch,NULL,victim,TO_VICT);
	act("You breath out a cone of frost.",ch,NULL,NULL,TO_CHAR);

	hpch = UMAX(12,ch->hit);
	hp_dam = number_range(hpch/11 + 1, hpch/6);
	dice_dam = dice(level,16);

	dam = UMAX(hp_dam + dice_dam/10,dice_dam + hp_dam/10);
	cold_effect(victim->in_room,level,dam/2); 

	for (vch = victim->in_room->people; vch != NULL; vch = vch_next)
	{
		vch_next = vch->next_in_room;

		if (is_safe_spell(ch,vch,TRUE)
		||  (IS_NPC(vch) && IS_NPC(ch) 
		&&   (ch->fighting != vch /*|| vch->fighting != ch*/)))
		    continue;

		if (vch == victim) /* full damage */
		{
		    if (saves_spell(level, vch, DAM_COLD)) {
			cold_effect(vch, level/2, dam/4);
			damage(ch, vch, dam/2, sn, DAM_COLD, DAMF_SHOW);
		    } else {
			cold_effect(vch, level, dam);
			damage(ch, vch, dam, sn, DAM_COLD, DAMF_SHOW);
		    }
		}
		else
		{
		    if (saves_spell(level - 2, vch, DAM_COLD)) {
			cold_effect(vch, level/4, dam/8);
			damage(ch, vch, dam/4, sn, DAM_COLD, DAMF_SHOW);
		    } else {
			cold_effect(vch, level/2, dam/4);
			damage(ch, vch, dam/2, sn, DAM_COLD, DAMF_SHOW);
		    }
		}
	}
}

	
void spell_gas_breath(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;
	int dam,hp_dam,dice_dam,hpch;

	act("$n breathes out a cloud of poisonous gas!",ch,NULL,NULL,TO_ROOM);
	act("You breath out a cloud of poisonous gas.",ch,NULL,NULL,TO_CHAR);

	hpch = UMAX(16,ch->hit);
	hp_dam = number_range(hpch/15+1,8);
	dice_dam = dice(level,12);

	dam = UMAX(hp_dam + dice_dam/10,dice_dam + hp_dam/10);
	poison_effect(ch->in_room,level,dam);

	for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
	{
		vch_next = vch->next_in_room;

		if (is_safe_spell(ch,vch,TRUE)
		||  (IS_NPC(ch) && IS_NPC(vch) 
		&&   (ch->fighting == vch || vch->fighting == ch)))
		    continue;

		if (saves_spell(level, vch, DAM_POISON)) {
		    poison_effect(vch, level/2, dam/4);
		    damage(ch, vch, dam/2, sn, DAM_POISON, DAMF_SHOW);
		} else {
		    poison_effect(vch, level, dam);
		    damage(ch, vch, dam, sn, DAM_POISON, DAMF_SHOW);
		}
	}
}

void spell_lightning_breath(const char *sn, int level,CHAR_DATA *ch,void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam,hp_dam,dice_dam,hpch;

	act("$n breathes a bolt of lightning at $N.",ch,NULL,victim,TO_NOTVICT);
	act("$n breathes a bolt of lightning at you!",ch,NULL,victim,TO_VICT);
	act("You breathe a bolt of lightning at $N.",ch,NULL,victim,TO_CHAR);

	hpch = UMAX(10,ch->hit);
	hp_dam = number_range(hpch/9+1,hpch/5);
	dice_dam = dice(level,20);

	dam = UMAX(hp_dam + dice_dam/10,dice_dam + hp_dam/10);

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
		if (!can_see_obj(ch, obj) || !is_name(target_name, obj->name)
		||  number_percent() > 2 * level
		||  LEVEL(ch) < obj->level
		||  (IS_SET(obj->pObjIndex->extra_flags, ITEM_CHQUEST) &&
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
			buf_printf(buffer, "One is carried by %s\n",
				   PERS(in_obj->carried_by, ch));
		} else {
			if (IS_IMMORTAL(ch) && in_obj->in_room != NULL)
				buf_printf(buffer, "One is in %s [Room %d]\n",
					mlstr_cval(&in_obj->in_room->name, ch),
					in_obj->in_room->vnum);
			else
				buf_printf(buffer, "One is in %s\n",
					in_obj->in_room == NULL ?
					"somewhere" :
					mlstr_cval(&in_obj->in_room->name, ch));
		}

		if (number >= max_found)
			break;
	}

	if (buffer == NULL)
		char_puts("Nothing like that in heaven or earth.\n", ch);
	else {
		page_to_char(buf_string(buffer),ch);
		buf_free(buffer);
	}
}

void spell_lightning_shield(const char *sn, int level, CHAR_DATA *ch, void *vo) 
{
	AFFECT_DATA af2;
	AFFECT_DATA af;

	if (is_affected_room(ch->in_room, sn))
	{
		char_puts("This room has already shielded.\n",ch);
		return;
	}

	if (is_affected(ch,sn))
	{
		char_puts("This spell is used too recently.\n",ch);
		return;
	}
   
	af.where     = TO_ROOM_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = level / 40;
	INT(af.location) = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = 0;
	af.owner     = ch;
	af.events    = EVENT_ROOM_ENTER | EVENT_ROOM_LEAVE;
	affect_to_room(ch->in_room, &af);

	af2.where     = TO_AFFECTS;
	af2.type      = sn;
	af2.level     = ch->level;
	af2.duration  = level / 10;
	af2.modifier  = 0;
	INT(af2.location) = APPLY_NONE;
	af2.bitvector = 0;
	affect_to_char(ch, &af2);

	char_puts("The room starts to be filled with lightnings.\n", ch);
	act("The room starts to be filled with $n's lightnings.",ch,NULL,NULL,TO_ROOM);
}

void spell_shocking_trap(const char *sn, int level, CHAR_DATA *ch, void *vo) 
{
	AFFECT_DATA af2;
	AFFECT_DATA af;

	if (is_affected_room(ch->in_room, sn))
	{
		char_puts("This room has already trapped with shocks waves.\n",ch);
		return;
	}

	if (is_affected(ch,sn))
	{
		char_puts("This spell is used too recently.\n",ch);
		return;
	}
   
	af.where     = TO_ROOM_AFFECTS;
	af.type      = sn;
	af.level     = level;
	af.duration  = level / 40;
	INT(af.location) = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = 0;
	af.owner     = ch;
	af.events     = EVENT_ROOM_ENTER;
	affect_to_room(ch->in_room, &af);

	af2.where     = TO_AFFECTS;
	af2.type      = sn;
	af2.level     = level;
	af2.duration  = ch->level / 10;
	af2.modifier  = 0;
	INT(af2.location) = APPLY_NONE;
	af2.bitvector = 0;
	affect_to_char(ch, &af2);
	char_puts("The room starts to be filled with shock waves.\n", ch);
	act("The room starts to be filled with $n's shock waves.",ch,NULL,NULL,TO_ROOM);
}

void spell_acid_arrow(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	dam = dice(level, 12);
	if (saves_spell(level, victim, DAM_ACID))
		dam /= 2;
	damage(ch, victim, dam, sn, DAM_ACID, DAMF_SHOW);
}


/* energy spells */
void spell_etheral_fist(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	dam = dice(level, 12);
	if (saves_spell(level, victim, DAM_ENERGY))
		dam /= 2;
	act("A fist of black, otherworldly ether rams into $N, leaving $M looking stunned!",
	    ch,NULL,victim,TO_NOTVICT);
	damage(ch, victim, dam, sn, DAM_ENERGY, DAMF_SHOW);
}

void spell_spectral_furor(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	dam = dice(level, 8);
	if (saves_spell(level, victim, DAM_LIGHT))
		dam /= 2;
	act("The fabric of the cosmos strains in fury about $N!",
			ch,NULL,victim,TO_NOTVICT);
	damage(ch, victim, dam, sn, DAM_LIGHT, DAMF_SHOW);
}

void spell_disruption(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	dam = dice(level, 9);
	if (saves_spell(level, victim, DAM_NEGATIVE))
		dam /= 2;
	act("A weird energy encompasses $N, causing you to question $S continued existence.",
			ch,NULL,victim,TO_NOTVICT);
	damage(ch, victim, dam, sn, DAM_NEGATIVE, DAMF_SHOW);
}

void spell_sonic_resonance(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	dam = dice(level, 7);
	if (saves_spell(level, victim, DAM_SOUND))
		dam /= 2;
	act("A cylinder of kinetic energy enshrouds $N causing $S to resonate.",
			ch,NULL,victim,TO_NOTVICT);
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
			ch,NULL,victim,TO_NOTVICT);
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
			ch,NULL,victim,TO_NOTVICT);
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
			ch,NULL,victim,TO_NOTVICT);
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
			ch,NULL,victim,TO_NOTVICT);
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
			ch,NULL,victim,TO_NOTVICT);
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
			ch,NULL,victim,TO_NOTVICT);
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
			ch,NULL,victim,TO_NOTVICT);
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
			ch,NULL,victim,TO_NOTVICT);
	damage(ch, victim, dam, sn, DAM_LIGHTNING, DAMF_SHOW);
}

/* negative */
void spell_hand_of_undead(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	if (saves_spell(level, victim, DAM_NEGATIVE)) {
		char_puts("You feel a momentary chill.\n",victim);
		return;
	}

	if (IS_NPC(victim) && IS_SET(victim->pMobIndex->act, ACT_UNDEAD)) {
		 char_puts("Your victim is unaffected by hand of undead.\n",ch);
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

	char_puts("You feel your life slipping away!\n",victim);
	act("$N is grasped by an incomprehensible hand of undead!",
			ch,NULL,victim,TO_NOTVICT);
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
		char_puts("You failed.\n", ch);
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
		char_puts("You failed.\n", ch);
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
		 char_puts("You need sunlight for solar flight.\n",ch);
		 return;
	}

	if ((victim = get_char_world(ch, target_name)) == NULL
	||  LEVEL(victim) >= level + 1
	||  saves_spell(level, victim, DAM_OTHER)
	||  !can_gate(ch, victim)) {
		char_puts("You failed.\n", ch);
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
		char_puts("You failed.\n", ch);
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

	af.where     = TO_AFFECTS;
	af.type 	 = sn;
	af.level	 = level * 3/4;
	af.duration  = (10 + level / 5);
	INT(af.location) = APPLY_NONE;
	af.modifier  = 0; 
	af.bitvector = AFF_CORRUPTION;
	affect_join(victim,&af);

	act("You scream in agony as you start to decay into dust.",
	    victim, NULL, NULL, TO_CHAR);
	act("$n screams in agony as $n start to decay into dust.",
	    victim, NULL, NULL, TO_ROOM);
}

void spell_hurricane(const char *sn, int level,CHAR_DATA *ch,void *vo)
{
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;
	int dam,hp_dam,dice_dam,hpch;

	act("$n prays the gods of the storm for help.",ch,NULL,NULL,TO_NOTVICT);
	act("You pray the gods of the storm to help you.",ch,NULL,NULL,TO_CHAR);

	hpch = UMAX(16,ch->hit);
	hp_dam = number_range(hpch/15+1,8);
	dice_dam = dice(level,12);

	dam = UMAX(hp_dam + dice_dam/10,dice_dam + hp_dam/10);

	for (vch = ch->in_room->people; vch != NULL; vch = vch_next) {
		vch_next = vch->next_in_room;

		if (is_safe_spell(ch,vch,TRUE)
		||  (IS_NPC(ch) && IS_NPC(vch) 
		&&   (ch->fighting == vch || vch->fighting == ch)))
			continue;

		if (!IS_AFFECTED(vch,AFF_FLYING)) dam /= 2;

		if (vch->size == SIZE_TINY)  dam *= 1.5;
		else if (vch->size == SIZE_SMALL)  dam *= 1.3;
		else if (vch->size == SIZE_MEDIUM)  dam *= 1;
		else if (vch->size == SIZE_LARGE)  dam *= 0.9;
		else if (vch->size == SIZE_HUGE)  dam *= 0.7;
		else dam *= 0.5;

		if (saves_spell(level, vch, DAM_OTHER))
			damage(ch, vch, dam/2, sn, DAM_OTHER, DAMF_SHOW);
		else
			damage(ch, vch, dam, sn, DAM_OTHER, DAMF_SHOW);
	}
}

void spell_detect_undead(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	AFFECT_DATA af;

	if (IS_AFFECTED(victim, AFF_DETECT_UNDEAD)) {
		if (victim == ch)
			char_puts("You can already sense undead.\n", ch);
		else
			act("$N can already detect undead.",
			    ch, NULL, victim, TO_CHAR);
		return;
	}

	af.where     = TO_AFFECTS;
	af.type      = sn;
	af.level	 = level;
	af.duration  = (5 + level / 3);
	af.modifier  = 0;
	INT(af.location) = APPLY_NONE;
	af.bitvector = AFF_DETECT_UNDEAD;
	affect_to_char(victim, &af);
	char_puts("Your eyes tingle.\n", victim);
	if (ch != victim)
		char_puts("Ok.\n", ch);
}

void spell_take_revenge(const char *sn, int level, CHAR_DATA *ch, void *vo)
{
	OBJ_DATA *obj;
	OBJ_DATA *in_obj;
	ROOM_INDEX_DATA *room = NULL;
	bool found = FALSE;
 
	if (IS_NPC(ch) || !IS_SET(PC(ch)->plr_flags, PLR_GHOST)) {
		char_puts("It is too late to take revenge.\n",ch);
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
		char_puts("Unluckily your corpse is devoured.\n", ch);
	else
		transfer_char(ch, NULL, room, NULL, NULL, NULL);
}

